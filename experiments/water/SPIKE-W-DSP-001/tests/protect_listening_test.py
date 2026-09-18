"""Decoded-audio regression for the real Protect listening-pack CLI/renderer."""
import csv
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import numpy as np
import soundfile as sf

RENDERER = Path(sys.argv.pop(1)).resolve()
SCRIPT = Path(__file__).resolve().parents[1] / "analysis" / "prepare_protect_listening.py"


class ListeningPackTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="frazil-protect-listening-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.root = Path(cls.temp.name)
        cls.rights = {"source": "generated loud/quiet burst regression, version 1",
                      "author": "FRAZIL regression generator", "license": "repository test fixture",
                      "permission": "generated locally for automated testing; no third-party material",
                      "storage_policy": "temporary local source/copies/renders; delete after test"}
        cls.metadata = cls.root / "source-metadata.json"
        cls.metadata.write_text(json.dumps(cls.rights), encoding="utf-8")
        cls.sources, cls.packs = {}, {}
        rate, frames = 48000, 9600
        time = np.arange(frames) / rate
        amplitude = np.where(time < .08, .65, .06) * (np.mod(time, .04) < .012)
        signal = amplitude * np.sin(2 * np.pi * 220 * time)
        for mode, subtype in (("abd", "PCM_24"), ("c", "PCM_16")):
            source = cls.root / f"{mode}-loud-quiet.wav"
            audio = np.column_stack((signal, .37 * signal)) if mode == "abd" else signal
            sf.write(source, audio, rate, subtype=subtype)
            cls.sources[mode] = source
            cls.packs[mode] = []
            for run, seed in enumerate((17, 17, 19) if mode == "c" else (17, 17)):
                output = cls.root / f"{mode}-{run}"
                cls.run_cli(source, output, mode, seed)
                cls.packs[mode].append(output)

    @classmethod
    def run_cli(cls, source, output, mode, seed, metadata=None, check=True):
        result = subprocess.run(
            [sys.executable, str(SCRIPT), "--renderer", str(RENDERER), "--source", str(source),
             "--source-metadata", str(metadata or cls.metadata), "--mode", mode, "--output", str(output),
             "--diagnostic", "--dsp-seed", "123", "--randomization-seed", str(seed),
             "--appended-tail-seconds", "1"], capture_output=True, text=True)
        if check and result.returncode:
            raise AssertionError(result.stdout + result.stderr)
        return result

    @staticmethod
    def manifest(pack, kind):
        return json.loads((pack / kind / "REVIEWER_KEY.json").read_text(encoding="utf-8"))

    @staticmethod
    def audio(path):
        return sf.read(path, always_2d=True)[0]

    def test_fixed_source_carrier_gain_is_common_in_decoded_audio(self):
        for mode, packs in self.packs.items():
            pack = packs[0]
            manifest = self.manifest(pack, "fixed_source")
            gain = manifest["fixed_source_gain"]
            self.assertGreater(gain, 0)
            self.assertLessEqual(gain, 1)
            source = self.audio(pack / "reviewer" / "dry_raw.wav")
            for key in manifest["keys"]:
                with self.subTest(mode=mode, condition=key["condition"]):
                    self.assertEqual(key["source_carrier_gain"], gain)
                    self.assertEqual(key["playback_gain"], gain)
                    raw = self.audio(pack / "reviewer" / (key["condition"] + "_raw.wav"))
                    trial = self.audio(pack / "fixed_source" / (key["trial"] + ".wav"))
                    # Remove the scaled residual and recover the same scaled source in every candidate.
                    np.testing.assert_allclose(trial - gain * (raw - source), gain * source,
                                               rtol=0, atol=6e-8)

    def test_detector_pairs_are_explicit_and_match_actual_renderer_output(self):
        for mode, packs in self.packs.items():
            pack = packs[0]
            keys = self.manifest(pack, "fixed_source")["keys"]
            configs = {key["condition"]: key for key in keys if key["config"] is not None}
            for name, key in configs.items():
                detector = int(name[1])
                config = key["config"]["protect"]
                self.assertEqual(key["detector"], f"D{detector}")
                self.assertEqual(config["detector"], detector)
                self.assertEqual((config["thresholdLow"], config["thresholdHigh"]),
                                 (.01, .12) if detector == 0 else (1, 9))
                self.assertEqual(configs[f"d{1-detector}" + name[2:]]["config"]["protect"]["depth"],
                                 config["depth"])
                self.assertEqual(json.loads((pack / "reviewer" / (name + ".json")).read_text()), key["config"])
            # Independently invoke the renderer with explicit detector values; catches a dropped config argument.
            for detector in (0, 1):
                name = f"d{detector}_strong"
                config = configs[name]["config"]
                path = self.root / f"{mode}-expected-{detector}.json"
                path.write_text(json.dumps(config), encoding="utf-8")
                output = self.root / f"{mode}-expected-{detector}.wav"
                subprocess.run([str(RENDERER), str(self.sources[mode]), str(output), mode, "128", "123",
                                str(path), "1"], check=True, capture_output=True)
                np.testing.assert_array_equal(self.audio(output), self.audio(pack / "reviewer" / (name + "_raw.wav")))
            self.assertGreater(np.max(np.abs(self.audio(pack / "reviewer" / "d0_strong_raw.wav") -
                                             self.audio(pack / "reviewer" / "d1_strong_raw.wav"))), 1e-7)
            np.testing.assert_array_equal(self.audio(pack / "reviewer" / "d0_off_raw.wav"),
                                          self.audio(pack / "reviewer" / "d1_off_raw.wav"))

    def test_original_source_metadata_and_tail_are_unambiguous(self):
        for mode, packs in self.packs.items():
            source = self.sources[mode]
            info = sf.info(source)
            self.assertEqual(source.read_bytes(), (packs[0] / "reviewer" / "source.wav").read_bytes())
            for kind, seed in (("fixed_source", 17), ("rms_matched", 18)):
                manifest = self.manifest(packs[0], kind)
                for key, value in self.rights.items():
                    self.assertEqual(manifest[key], value)
                self.assertEqual(manifest["source_name"], source.name)
                self.assertEqual(manifest["source_frames"], 9600)
                self.assertEqual(manifest["source_duration_seconds"], .2)
                self.assertEqual(manifest["comparison_frames"], 57600)
                self.assertEqual(manifest["comparison_duration_seconds"], 1.2)
                self.assertEqual(manifest["channels"], info.channels)
                self.assertEqual(manifest["subtype"], info.subtype)
                self.assertEqual(manifest["bit_depth"], 24 if mode == "abd" else 16)
                self.assertEqual(manifest["sample_rate"], 48000)
                self.assertEqual(manifest["appended_tail_seconds"], 1)
                self.assertEqual(manifest["dsp_seed"], 123)
                self.assertEqual(manifest["randomization_seed"], seed)
                self.assertNotIn("frames", manifest)
                self.assertNotIn("source_sha256", manifest)
                self.assertTrue(manifest["code_commit"])
                for key in manifest["keys"]:
                    self.assertEqual(sf.info(packs[0] / kind / (key["trial"] + ".wav")).frames, 57600)

    def test_hidden_repeats_have_identical_conditions_and_samples(self):
        for packs in self.packs.values():
            for kind in ("fixed_source", "rms_matched"):
                seen, repeated = {}, []
                for key in self.manifest(packs[0], kind)["keys"]:
                    condition = key["condition"]
                    if condition in seen:
                        first = seen[condition]
                        self.assertEqual({k: v for k, v in key.items() if k != "trial"},
                                         {k: v for k, v in first.items() if k != "trial"})
                        np.testing.assert_array_equal(
                            self.audio(packs[0] / kind / (key["trial"] + ".wav")),
                            self.audio(packs[0] / kind / (first["trial"] + ".wav")))
                        repeated.append(condition)
                    seen[condition] = key
                self.assertCountEqual(repeated, ["d0_off", "d0_medium", "d1_medium"])

    def test_randomization_and_audio_are_reproducible_with_separate_seeds(self):
        for packs in self.packs.values():
            for kind in ("fixed_source", "rms_matched"):
                first = self.manifest(packs[0], kind)
                second = self.manifest(packs[1], kind)
                self.assertEqual(first, second)
                for key in first["keys"]:
                    np.testing.assert_array_equal(self.audio(packs[0] / kind / (key["trial"] + ".wav")),
                                                  self.audio(packs[1] / kind / (key["trial"] + ".wav")))
        packs = self.packs["c"]
        for kind in ("fixed_source", "rms_matched"):
            self.assertNotEqual([k["condition"] for k in self.manifest(packs[0], kind)["keys"]],
                                [k["condition"] for k in self.manifest(packs[2], kind)["keys"]])
        for raw in (packs[0] / "reviewer").glob("*_raw.wav"):
            np.testing.assert_array_equal(self.audio(raw), self.audio(packs[2] / "reviewer" / raw.name))

    def test_evidence_purposes_are_separate_and_selection_is_blocked(self):
        for mode, packs in self.packs.items():
            fixed = self.manifest(packs[0], "fixed_source")
            matched = self.manifest(packs[0], "rms_matched")
            self.assertTrue(fixed["evidence_purpose"].startswith("PRIMARY"))
            self.assertTrue(matched["evidence_purpose"].startswith("PREFERENCE-SUPPORTING ONLY"))
            self.assertIsNone(fixed["target_rms"])
            self.assertIsNone(matched["fixed_source_gain"])
            self.assertGreater(len({key["source_carrier_gain"] for key in matched["keys"]}), 1)
            for kind, manifest in (("fixed_source", fixed), ("rms_matched", matched)):
                self.assertEqual(manifest["detector_selection"], "UNRESOLVED")
                self.assertTrue(manifest["wave7_product_decision"].startswith("BLOCKED"))
                self.assertEqual(len(manifest["keys"]), 21 if mode == "abd" else 13)
                with (packs[0] / kind / "SCORES.csv").open(newline="", encoding="utf-8") as stream:
                    rows = list(csv.reader(stream))
                self.assertEqual(len(rows), len(manifest["keys"]) + 1)
                self.assertTrue(all(not cell for row in rows[1:] for cell in row[1:]))
                self.assertIn("Do not copy or pool", (packs[0] / kind / "LISTENING_REVIEW.md").read_text(encoding="utf-8"))
            for key in matched["keys"]:
                audio = self.audio(packs[0] / "rms_matched" / (key["trial"] + ".wav"))
                self.assertAlmostEqual(float(np.sqrt(np.mean(audio*audio))), matched["target_rms"], places=7)

    def test_missing_source_rights_are_rejected_before_pack_creation(self):
        for field in self.rights:
            invalid = dict(self.rights)
            del invalid[field]
            metadata = self.root / f"missing-{field}.json"
            metadata.write_text(json.dumps(invalid), encoding="utf-8")
            output = self.root / f"invalid-{field}"
            result = self.run_cli(self.sources["c"], output, "c", 17, metadata, check=False)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Source metadata requires", result.stderr)
            self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
