"""Evidence tooling regressions: missing data, private paths and retained child failures."""

import csv
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "render"))
from a1_lifecycle_r31_publish import BASELINE_SHA, collect, encode
from native_case_evidence import run_case
from native_parent_environment_ab import environments


class EvidenceTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            prefix="evidence-tools-", dir=Path(__file__).resolve().parents[4] / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def write(self, name, rows):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(encode(rows), encoding="utf-8")

    def fixture(self):
        (self.root / "STATUS.json").write_text(json.dumps(dict(status="ENGINEERING COMPLETE")))
        # Empty stand-ins exercise only the explicitly required two-binary binding contract.
        self.binary = self.root / "renderer.exe"
        self.binary.write_bytes(b"")
        self.current = "a" * 40
        self.binding = dict(expected_baseline_sha=BASELINE_SHA, expected_current_sha=self.current,
                            baseline_binary=self.binary, current_binary=self.binary)
        self.provenance = {role: dict(source_sha=sha, working_tree="clean",
            binary_sha256="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")
            for role, sha in (("baseline", BASELINE_SHA), ("current", self.current))}
        self.save_provenance()
        # Recorded canonical matrix is the independent completeness oracle, not publisher logic.
        canonical = Path(__file__).resolve().parents[4] / "docs/evidence/r31-closeout"
        def rows(name):
            with (canonical / ("WATER_A1_R31_" + name + ".csv")).open() as stream:
                return list(csv.DictReader(stream))
        for policy, name in (("L0", "MEASUREMENTS"), ("L1", "L1_MEASUREMENTS")):
            self.write(f"reference/{name}.csv", [r for r in rows("REFERENCE") if r["policy"] == policy])
        bands = rows("BANDS")
        self.write("reference/BANDS.csv", [{k:v for k,v in r.items() if k != "policy"}
                                          for r in bands if r["policy"] == "L0"])
        for i in range(1, 7):
            sid = f"source-{i:02}"
            self.write(f"reference/{sid}/L1_BANDS.csv", [
                {k:v for k,v in r.items() if k not in ("policy", "source_id")}
                for r in bands if r["policy"] == "L1" and r["source_id"] == sid])
        preservation = rows("PRESERVATION")
        self.write("preservation/PRESERVATION.csv", preservation[:45])
        self.write("reference/PRESERVATION.csv", preservation[45:])
        for variant in ("before", "l0", "l1", "l1-trace"):
            self.write(f"performance-{variant}.csv", [
                {k:v for k,v in r.items() if k not in ("variant", "deadline_us", "deadline_exceeded")}
                for r in rows("PERFORMANCE") if r["variant"] == variant])

    def save_provenance(self):
        (self.root / "PROVENANCE.json").write_text(json.dumps(self.provenance))

    def collect(self, git_results=None):
        results = git_results if git_results is not None else [BASELINE_SHA, "", self.current, ""]
        with patch("a1_lifecycle_r31_publish.subprocess.check_output", side_effect=results):
            return collect(self.root, **self.binding)

    def test_complete_deterministic_publication(self):
        self.fixture()
        first = self.collect()
        self.assertEqual(first, self.collect())
        for name, count in (("REFERENCE", 12), ("BANDS", 84), ("PRESERVATION", 57), ("PERFORMANCE", 120)):
            self.assertEqual(len(list(csv.DictReader(first[f"WATER_A1_R31_{name}.csv"].splitlines()))), count)
        self.assertEqual(encode([dict(a="", b=None)]), "a,b\nN/A,N/A\n")
        (self.root / "performance-l1.csv").unlink()
        with self.assertRaises(FileNotFoundError):
            self.collect()

    def test_incomplete_duplicate_and_wrong_matrix_cells(self):
        cases = [(file, mutation) for file in (
            "reference/MEASUREMENTS.csv", "reference/L1_MEASUREMENTS.csv",
            "reference/BANDS.csv", "reference/source-06/L1_BANDS.csv",
            "preservation/PRESERVATION.csv", "reference/PRESERVATION.csv",
            "performance-l1.csv") for mutation in ("missing", "duplicate", "extra")]
        cases += [("preservation/PRESERVATION.csv", field) for field in ("mode", "rate", "comparison")]
        cases += [("reference/MEASUREMENTS.csv", "source_id"), ("reference/BANDS.csv", "band")]
        for name, mutation in cases:
            with self.subTest(name=name, mutation=mutation):
                self.fixture()
                with (self.root / name).open() as stream:
                    rows = list(csv.DictReader(stream))
                if mutation == "missing":
                    rows.pop()
                elif mutation == "duplicate":
                    rows[-1] = rows[0].copy()  # Same count must still reject.
                elif mutation == "extra":
                    rows.append(rows[0].copy())
                else:
                    rows[0][mutation] = "unexpected"
                self.write(name, rows)
                with self.assertRaisesRegex(ValueError, "Incomplete"):
                    self.collect()

    def test_provenance_and_preservation_rejection(self):
        for field, value in (("working_tree", "dirty"), ("source_sha", "c" * 40),
                             ("binary_sha256", "b" * 64)):
            with self.subTest(field=field):
                self.fixture()
                self.provenance["baseline"][field] = value
                self.save_provenance()
                with self.assertRaises(ValueError):
                    self.collect()
        self.fixture()
        self.binding["expected_baseline_sha"] = "c" * 40
        with self.assertRaisesRegex(ValueError, "expected baseline"):
            self.collect()
        self.fixture()
        for results in (["c" * 40], [BASELINE_SHA, " M changed.py"]):
            with self.assertRaisesRegex(ValueError, "Live Git"):
                self.collect(results)
        with (self.root / "preservation/PRESERVATION.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        for delta in ("0.01", "nan", "inf"):
            rows[0]["max_delta"] = delta
            self.write("preservation/PRESERVATION.csv", rows)
            with self.assertRaisesRegex(ValueError, "preservation failed"):
                self.collect()
        separator = chr(92)
        for private in ("X" + ":/private/input.wav",
                        "X:" + separator + "private" + separator + "input.wav", "/private/input.wav"):
            with self.assertRaisesRegex(ValueError, "private path"):
                encode([dict(source=private)])

    def test_child_failure_and_timeout_keep_partial_streams(self):
        directory = self.root / "failed"
        with self.assertRaisesRegex(RuntimeError, "child exit 7"):
            run_case([sys.executable, "-c", "import sys; print('marker', file=sys.stderr); sys.exit(7)"],
                     directory, "failed", 10)
        self.assertIn("marker", (directory / "stderr.log").read_text())
        directory = self.root / "timeout"
        with self.assertRaises(subprocess.TimeoutExpired):
            run_case([sys.executable, "-c", "import time; print('started', flush=True); time.sleep(5)"],
                     directory, "timeout", .5)
        self.assertIn("started", (directory / "stdout.log").read_text())
        self.assertIn("child-timeout", (directory / "progress.jsonl").read_text())

    def test_expected_rejection_is_not_any_native_failure(self):
        directory = self.root / "rejected"
        run_case([sys.executable, "-c", "raise SystemExit(2)"], directory, "reject", 10,
                 expected_exit_codes=(2,), metadata=dict(rate=48000, purpose="invalid-config"))
        self.assertEqual(json.loads((directory / "command.json").read_text())["metadata"]["rate"], 48000)
        with self.assertRaisesRegex(RuntimeError, "child exit 3"):
            run_case([sys.executable, "-c", "raise SystemExit(3)"], self.root / "unexpected",
                     "unexpected", 10, expected_exit_codes=(2,))

    def test_parent_isolation_changes_only_runtime_path(self):
        import os
        runtime = (self.root / "runtime").resolve()
        other = str(self.root / "python")
        normal, inherited = environments(dict(PATH=str(runtime) + os.pathsep + other,
                                             SENTINEL="unchanged"), runtime)
        self.assertEqual(normal, dict(PATH=other, SENTINEL="unchanged"))
        self.assertEqual(inherited["PATH"], str(runtime) + os.pathsep + normal["PATH"])
        self.assertEqual(inherited["SENTINEL"], normal["SENTINEL"])


if __name__ == "__main__":
    unittest.main()
