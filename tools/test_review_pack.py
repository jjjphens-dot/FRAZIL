#!/usr/bin/env python3
"""Exercise review-pack assembly, failure clarity and evidence integrity; no sound judgments."""

from __future__ import annotations

import copy
import json
import math
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import numpy as np
import soundfile as sf

import build_review_pack as pack


ROOT = Path(__file__).resolve().parents[1]


class ReviewPackTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # All generated test audio/plots stay under this explicitly bounded ignored directory.
        parent = (ROOT / "build" / "review-pack-tests").resolve()
        parent.mkdir(parents=True, exist_ok=True)
        cls.temporary = tempfile.TemporaryDirectory(prefix="suite-", dir=parent)
        cls.root = Path(cls.temporary.name).resolve()
        assert cls.root.is_relative_to(parent)
        cls.addClassCleanup(cls.temporary.cleanup)
        time = np.arange(2048) / 8000
        cls.samples = np.column_stack((0.125 * np.sin(2 * np.pi * 125 * time),
                                       0.125 * np.cos(2 * np.pi * 125 * time)))
        for name, samples in (("dry", cls.samples), ("double", 2 * cls.samples), ("zero", cls.samples * 0)):
            sf.write(cls.root / f"{name}.wav", samples, 8000, subtype="FLOAT")
        cls.provenance = {"sourceCommit": "123abcd", "sourceState": "unknown", "buildType": "fixture",
                          "platform": "generated test signal"}
        cls.spec = {"schema": pack.SPEC_SCHEMA, "schemaVersion": 1,
                    "experiment": {"id": "test-001", "purpose": "Infrastructure test"},
                    "provenance": cls.provenance, "input": {"id": "dry-fixture", "file": "dry.wav"},
                    "candidates": [{"id": "double", "label": "Double amplitude", "file": "double.wav",
                                    "config": {"gain": 2, "blockSize": 128}, "seed": 42,
                                    "provenance": cls.provenance}]}
        cls.spec_path = cls.root / "spec.json"
        pack.write_json(cls.spec_path, cls.spec)
        cls.reference = cls.root / "reference"
        cls.manifest = pack.build_pack(cls.spec_path, cls.reference)

    def setUp(self):
        self.case = Path(tempfile.mkdtemp(prefix="case-", dir=self.root))

    def write_spec(self, spec):
        # Input audio paths are resolved against this file; no reliance on caller cwd.
        path = self.root / (self.case.name + ".json")
        path.write_text(json.dumps(spec, allow_nan=True), encoding="utf-8")
        return path

    def build(self, spec):
        return pack.build_pack(self.write_spec(spec), self.case / "pack")

    def rejected_spec(self, spec):
        with self.assertRaises((pack.PackError, OSError, ValueError)):
            self.build(spec)
        self.assertFalse((self.case / "pack").exists(), "Input errors must precede output creation")

    def copied_pack(self):
        result = self.case / "pack"
        shutil.copytree(self.reference, result)
        return result

    def tamper(self, change):
        root = self.copied_pack()
        manifest = pack.read_json(root / "manifest.json")
        change(manifest)
        # Test corrupt external JSON independently of the writer's own guard.
        (root / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaises(pack.PackError):
            pack.validate_pack(root)

    def test_minimal_pack_real_analysis_plots_and_copy(self):
        manifest = pack.validate_pack(self.reference)
        self.assertEqual(manifest["baseline"], None)
        self.assertEqual(len(manifest["artifacts"]["plots"]), 6)
        for name, entry in (("dry", manifest["dry"]), ("double", manifest["candidates"][0])):
            self.assertEqual((self.root / f"{name}.wav").read_bytes(), (self.reference / entry["file"]).read_bytes())
            analysis = pack.read_json(self.reference / entry["analysisFile"])
            self.assertEqual(analysis["path"], entry["file"])
            self.assertGreater(analysis["rms"], 0)
            self.assertGreater(analysis["spectrogram"]["timeBins"], 0)
        self.assertAlmostEqual(manifest["comparisons"][0]["value"], 20 * math.log10(2), places=10)
        self.assertFalse((self.reference / ".incomplete").exists())

    def test_baseline_and_multiple_candidates_have_independent_identity(self):
        spec = copy.deepcopy(self.spec)
        spec["baseline"] = {key: value for key, value in spec["candidates"][0].items() if key != "id"}
        spec["baseline"].update(label="Same as dry (explicit control)", file="dry.wav", config={"gain": 1})
        candidate = copy.deepcopy(spec["candidates"][0])
        candidate.update(id="zero", label="Zero control", file="zero.wav", config={"gain": 0}, seed=43)
        candidate["provenance"]["sourceCommit"] = "456abcd"
        spec["candidates"].append(candidate)
        manifest = self.build(spec)
        self.assertEqual([item["file"] for item in pack.audio_entries(manifest)],
                         ["audio/00-dry.wav", "audio/01-baseline.wav", "audio/02-double.wav", "audio/03-zero.wav"])
        self.assertEqual(len(manifest["comparisons"]), 4)
        self.assertEqual(manifest["candidates"][1]["seed"], 43)
        self.assertEqual(manifest["candidates"][1]["provenance"]["sourceCommit"], "456abcd")
        self.assertEqual(manifest["comparisons"][-1]["state"], "candidate_zero")
        self.assertIsNone(manifest["comparisons"][-1]["value"])

    def test_dry_baseline_single_candidate(self):
        spec = copy.deepcopy(self.spec)
        spec["baseline"] = {key: value for key, value in spec["candidates"][0].items() if key != "id"}
        spec["baseline"].update(file="dry.wav", label="Dry baseline", config={})
        result = self.build(spec)
        self.assertEqual(len(result["comparisons"]), 2)
        self.assertEqual(result["comparisons"][0]["value"], result["comparisons"][1]["value"])

    def test_all_zero_and_reference_zero(self):
        spec = copy.deepcopy(self.spec)
        spec["input"]["file"] = "zero.wav"
        candidate = copy.deepcopy(spec["candidates"][0])
        candidate.update(id="zero", file="zero.wav", config={"gain": 0})
        spec["candidates"].append(candidate)
        result = self.build(spec)
        self.assertEqual([row["state"] for row in result["comparisons"]], ["reference_zero", "both_zero"])
        self.assertTrue(all(row["value"] is None for row in result["comparisons"]))
        self.assertNotIn("Infinity", (self.case / "pack" / "manifest.json").read_text(encoding="utf-8"))

    def test_ratio_extremes_stay_finite_without_audio_normalization(self):
        self.assertEqual(pack.level_comparison("c", "r", 1e300, 1e-300)["value"], 12000.0)
        self.assertEqual(pack.level_comparison("c", "r", 1e-300, 1e300)["value"], -12000.0)
        for invalid in (-1, float("nan"), float("inf"), True):
            with self.subTest(value=invalid), self.assertRaises(pack.PackError):
                pack.level_comparison("c", "r", invalid, 1)

    def test_repeatable_portable_manifest_and_analysis(self):
        manifest = self.build(copy.deepcopy(self.spec))
        self.assertEqual(manifest, self.manifest)
        self.assertEqual((self.reference / "manifest.json").read_bytes(), (self.case / "pack" / "manifest.json").read_bytes())
        for item in pack.audio_entries(manifest):
            self.assertEqual((self.reference / item["analysisFile"]).read_bytes(),
                             (self.case / "pack" / item["analysisFile"]).read_bytes())
        self.assertNotIn(str(self.root), (self.case / "pack" / "manifest.json").read_text(encoding="utf-8"))

    def test_generic_template_escapes_labels_and_carries_metadata(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["experiment"]["metadata"] = {"clause": "example", "question": "Preserve source?"}
        manifest["candidates"][0]["label"] = "[link](outside) | <script>\n# heading"
        pack.write_review_files(self.case, manifest)
        review = (self.case / "LISTENING_REVIEW.md").read_text(encoding="utf-8")
        self.assertIn("Reviewer A", review)
        self.assertIn("Reviewer B", review)
        self.assertIn("Joint Decision", review)
        self.assertIn("Decision: ______", review)
        self.assertIn("Proxy only. Not perceptual truth.", review)
        self.assertIn("Input: dry", review)
        self.assertIn("Declared experiment source: 123abcd (unknown)", review)
        self.assertIn("Preserve source?", review)
        self.assertIn("&lt;script&gt;", review)
        self.assertNotIn("\n# heading", review)
        for specialized in ("Water Identity", "Fluid", "Resonant", "Bubble", "Droplet"):
            self.assertNotIn(specialized, review)

    def test_human_notes_can_be_edited_without_changing_evidence(self):
        root = self.copied_pack()
        with (root / "LISTENING_REVIEW.md").open("a", encoding="utf-8") as stream:
            stream.write("\nHuman notes: review pending.\n")
        self.assertEqual(pack.validate_pack(root), self.manifest)

    def test_missing_required_spec_fields(self):
        for field in ("input", "candidates", "provenance", "experiment"):
            with self.subTest(field=field):
                spec = copy.deepcopy(self.spec)
                del spec[field]
                self.rejected_spec(spec)

    def test_missing_candidate_identity_fields(self):
        for field in ("id", "label", "file", "config", "seed", "provenance"):
            with self.subTest(field=field):
                spec = copy.deepcopy(self.spec)
                del spec["candidates"][0][field]
                self.rejected_spec(spec)

    def test_invalid_config_seed_and_provenance(self):
        for field, value in (("config", []), ("config", "defaults.json"), ("config", None),
                             ("seed", None), ("seed", True), ("seed", -1), ("seed", 2**64),
                             ("seed", 42.0), ("provenance", {})):
            with self.subTest(field=field, value=value):
                spec = copy.deepcopy(self.spec)
                spec["candidates"][0][field] = value
                self.rejected_spec(spec)
        for field, value in (("sourceCommit", "unknown"), ("sourceState", "verified"), ("buildType", "")):
            with self.subTest(field=field):
                spec = copy.deepcopy(self.spec)
                spec["candidates"][0]["provenance"][field] = value
                self.rejected_spec(spec)

    def test_duplicate_reserved_and_unsafe_ids(self):
        for value in ("dry", "baseline", "input", "../escape", "foo/bar", "Upper", "con", "lpt1", ""):
            with self.subTest(value=value):
                spec = copy.deepcopy(self.spec)
                spec["candidates"][0]["id"] = value
                self.rejected_spec(spec)
        spec = copy.deepcopy(self.spec)
        spec["candidates"].append(copy.deepcopy(spec["candidates"][0]))
        self.rejected_spec(spec)

    def test_bad_json_duplicate_keys_nonfinite_and_unknown_fields(self):
        valid = json.dumps(self.spec)
        for content in (valid + " {}", '{"schema":1,"schema":2}', valid.replace('"gain": 2', '"gain": NaN'),
                        valid.replace('"gain": 2', '"gain": 1e999')):
            with self.subTest(content=content[:45]):
                path = self.root / "invalid.json"
                path.write_text(content, encoding="utf-8")
                with self.assertRaises(pack.PackError):
                    pack.build_pack(path, self.case / "pack")
                self.assertFalse((self.case / "pack").exists())
        spec = copy.deepcopy(self.spec)
        spec["unknown"] = 1
        self.rejected_spec(spec)
        for field in pack.FORBIDDEN_KEYS:
            spec = copy.deepcopy(self.spec)
            spec["experiment"]["metadata"] = {field: 1}
            self.rejected_spec(spec)

    def test_missing_and_unreadable_files(self):
        for target in ("input", "candidate"):
            spec = copy.deepcopy(self.spec)
            entry = spec["input"] if target == "input" else spec["candidates"][0]
            entry["file"] = "missing.wav"
            self.rejected_spec(spec)
        (self.root / "not-audio.wav").write_text("Not a WAV", encoding="utf-8")
        spec["candidates"][0]["file"] = "not-audio.wav"
        self.rejected_spec(spec)

    def test_nonfinite_audio_rejected_before_output(self):
        for value in (float("nan"), float("inf"), -float("inf")):
            samples = self.samples.copy()
            samples[50, 1] = value
            sf.write(self.root / "invalid.wav", samples, 8000, subtype="FLOAT")
            spec = copy.deepcopy(self.spec)
            spec["candidates"][0]["file"] = "invalid.wav"
            self.rejected_spec(spec)

    def test_empty_short_wrong_format_and_dimensions(self):
        cases = ((np.zeros((0, 2)), 8000, "WAV"), (np.zeros((1, 2)), 8000, "WAV"),
                 (self.samples, 16000, "WAV"), (self.samples[:, :1], 8000, "WAV"),
                 (self.samples[:-1], 8000, "WAV"), (self.samples, 8000, "AIFF"))
        for samples, rate, fmt in cases:
            with self.subTest(shape=samples.shape, rate=rate, format=fmt):
                sf.write(self.root / "invalid.wav", samples, rate, format=fmt, subtype="PCM_16")
                spec = copy.deepcopy(self.spec)
                spec["candidates"][0]["file"] = "invalid.wav"
                self.rejected_spec(spec)

    def test_existing_directory_and_notes_are_untouched(self):
        output = self.case / "pack"
        output.mkdir()
        (output / "LISTENING_REVIEW.md").write_text("Human work", encoding="utf-8")
        with self.assertRaises(FileExistsError):
            pack.build_pack(self.spec_path, output)
        self.assertEqual((output / "LISTENING_REVIEW.md").read_text(encoding="utf-8"), "Human work")
        self.assertEqual(len(list(output.iterdir())), 1)

    def test_missing_dependencies_do_not_create_output(self):
        with mock.patch.object(pack, "dependencies", side_effect=pack.PackError("dependencies missing")):
            with self.assertRaisesRegex(pack.PackError, "dependencies missing"):
                pack.build_pack(self.spec_path, self.case / "pack")
        self.assertFalse((self.case / "pack").exists())

    def test_finite_audio_with_overflowing_analysis_cannot_complete(self):
        sf.write(self.root / "huge.wav", np.full((2048, 2), 1e200), 8000, subtype="DOUBLE")
        spec = copy.deepcopy(self.spec)
        spec["candidates"][0]["file"] = "huge.wav"
        with np.errstate(over="ignore", invalid="ignore"), self.assertRaisesRegex(pack.PackError, "non-finite"):
            self.build(spec)
        self.assertTrue((self.case / "pack" / ".incomplete").exists())

    def test_tool_git_unavailable_is_unknown_not_clean(self):
        with mock.patch.object(pack.subprocess, "run", side_effect=FileNotFoundError("Git unavailable")):
            self.assertEqual(pack.tool_source(), {"sourceCommit": None, "sourceState": "unknown"})

    def test_symlink_guard_without_platform_privileges(self):
        root = self.copied_pack()
        original = Path.is_symlink

        def simulated_symlink(path):
            return path == root / "audio/02-double.wav" or original(path)

        with mock.patch.object(Path, "is_symlink", simulated_symlink):
            with self.assertRaisesRegex(pack.PackError, "Symlinked artifact"):
                pack.validate_pack(root)

    def test_analysis_failure_retains_incomplete_marker(self):
        with mock.patch.object(pack.analyze_testdata, "analyze_audio", side_effect=RuntimeError("injected failure")):
            with self.assertRaisesRegex(pack.PackError, "injected failure"):
                pack.build_pack(self.spec_path, self.case / "pack")
        self.assertTrue((self.case / "pack" / ".incomplete").exists())
        with self.assertRaisesRegex(pack.PackError, "incomplete"):
            pack.validate_pack(self.case / "pack")

    def test_silently_missing_plots_prevent_completion(self):
        with mock.patch.object(pack.analyze_testdata, "write_plots"):
            with self.assertRaisesRegex(pack.PackError, "Missing/empty artifact"):
                pack.build_pack(self.spec_path, self.case / "pack")
        self.assertTrue((self.case / "pack" / ".incomplete").exists())

    def test_missing_review_prevents_completion(self):
        with mock.patch.object(pack, "write_review_files"):
            with self.assertRaisesRegex(pack.PackError, "Missing/empty artifact"):
                pack.build_pack(self.spec_path, self.case / "pack")
        self.assertTrue((self.case / "pack" / ".incomplete").exists())

    def test_required_artifacts_are_checked(self):
        for index, filename in enumerate(("manifest.json", "README.md", "LISTENING_REVIEW.md",
                                          "audio/02-double.wav", "analysis/double.json", "plots/double/waveform.png")):
            with self.subTest(filename=filename):
                root = self.case / str(index)
                shutil.copytree(self.reference, root)
                (root / filename).unlink()
                with self.assertRaises(pack.PackError):
                    pack.validate_pack(root)

    def test_changed_audio_rejected_by_integrity_check(self):
        root = self.copied_pack()
        sf.write(root / "audio/02-double.wav", self.samples, 8000, subtype="FLOAT")
        with self.assertRaisesRegex(pack.PackError, "digest mismatch"):
            pack.validate_pack(root)

    def test_analysis_and_comparison_joint_tampering_is_rejected(self):
        root = self.copied_pack()
        analysis = pack.read_json(root / "analysis/double.json")
        analysis["rms"] = 123.0
        pack.write_json(root / "analysis/double.json", analysis)
        manifest = pack.read_json(root / "manifest.json")
        manifest["comparisons"][0] = pack.level_comparison("double", "dry", 123., manifest["comparisons"][0]["referenceRms"])
        pack.write_json(root / "manifest.json", manifest)
        with self.assertRaisesRegex(pack.PackError, "analysis.rms"):
            pack.validate_pack(root)

    def test_invalid_comparison_state(self):
        self.tamper(lambda m: m["comparisons"][0].update(state="reference_zero", value=None))

    def test_invalid_comparison_value(self):
        self.tamper(lambda m: m["comparisons"][0].update(value=123.0))

    def test_manifest_identity_and_required_fields(self):
        changes = (
            lambda m: m["candidates"].append(copy.deepcopy(m["candidates"][0])),
            lambda m: m["candidates"][0].pop("provenance"),
            lambda m: m["candidates"][0].pop("seed"),
            lambda m: m["candidates"][0].update(config=[]),
            lambda m: m.update(schemaVersion=True),
            lambda m: m["dry"].update(frames=2048.0),
            lambda m: m["input"].update(file="audio/02-double.wav"),
            lambda m: m["artifacts"]["plots"].pop(),
        )
        for index, change in enumerate(changes):
            with self.subTest(change=index):
                self.case = Path(tempfile.mkdtemp(prefix="tamper-", dir=self.root))
                self.tamper(change)

    def test_noncanonical_path_is_rejected(self):
        self.tamper(lambda m: m["candidates"][0].update(file="../outside.wav"))

    def test_invalid_png_signature_is_rejected(self):
        root = self.copied_pack()
        (root / "plots/double/waveform.png").write_bytes(b"not a png image")
        with self.assertRaisesRegex(pack.PackError, "invalid/empty PNG"):
            pack.validate_pack(root)

    def test_symlinked_artifact_is_rejected(self):
        root = self.copied_pack()
        artifact = root / "audio/02-double.wav"
        artifact.unlink()
        try:
            artifact.symlink_to(self.root / "double.wav")
        except OSError as error:
            self.skipTest(f"Symlink creation unavailable on this host: {error}")
        with self.assertRaisesRegex(pack.PackError, "Symlinked artifact"):
            pack.validate_pack(root)

    def test_cli_success_validation_and_exit_two_on_conflict(self):
        cli = [sys.executable, str(ROOT / "tools/build_review_pack.py")]
        result = subprocess.run(cli + ["--validate", str(self.reference)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run(cli + ["--spec", str(self.spec_path), "--output", str(self.reference)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn("FAILED", result.stderr)
        result = subprocess.run(cli + ["--spec", str(self.spec_path)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
