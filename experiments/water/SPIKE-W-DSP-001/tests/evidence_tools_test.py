"""Evidence tooling regressions: missing data, private paths and retained child failures."""

import csv
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "render"))
from a1_lifecycle_r31_publish import collect, encode
from native_case_evidence import run_case


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
        # Deliberately synthetic metadata for parser tests; no content hash is computed.
        (self.root / "PROVENANCE.json").write_text(json.dumps({
            role: dict(source_sha="a" * 40, binary_sha256="b" * 64, working_tree="clean")
            for role in ("baseline", "current")}))
        row = dict(source_id="source-01", requested=3, started=2, firstNonZero=1,
                   completedWithoutNonZero=1, causedStealButNeverNonZero=0, steals=0,
                   causedSteal=0, riseEnabledAndFirstNonZero=0)
        self.write("reference/MEASUREMENTS.csv", [dict(policy="L0", **row)])
        self.write("reference/L1_MEASUREMENTS.csv", [dict(policy="L1", **row)])
        self.write("reference/BANDS.csv", [dict(source_id="source-01", band=i) for i in range(7)])
        self.write("reference/source-01/L1_BANDS.csv", [dict(band=i) for i in range(7)])
        for name in ("reference", "preservation"):
            self.write(f"{name}/PRESERVATION.csv", [dict(max_delta=0)])
        perf = [dict(rate=r, capacity=c, profile=p, worst_us=1)
                for r in (44100, 48000, 96000) for c in (64, 128, 256, 512, 1024)
                for p in ("physical-reference", "dense-stress")]
        for name in ("before", "l0", "l1", "l1-trace"):
            self.write(f"performance-{name}.csv", perf)

    def test_deterministic_normalization_and_missing_evidence(self):
        self.fixture()
        first = collect(self.root)
        self.assertEqual(first, collect(self.root))
        rows = list(csv.DictReader(first["WATER_A1_R31_REFERENCE.csv"].splitlines()))
        self.assertEqual(rows[0]["firstNonZero_per_requested"], str(1 / 3))
        self.assertEqual(rows[0]["causedStealButNeverNonZero_per_steals"], "N/A")
        self.assertEqual(encode([dict(a="", b=None)]), "a,b\nN/A,N/A\n")
        (self.root / "performance-l1.csv").unlink()
        with self.assertRaises(FileNotFoundError):
            collect(self.root)

    def test_reject_failed_preservation_dirty_source_and_private_paths(self):
        self.fixture()
        self.write("preservation/PRESERVATION.csv", [dict(max_delta=.01)])
        with self.assertRaisesRegex(ValueError, "preservation failed"):
            collect(self.root)
        self.write("preservation/PRESERVATION.csv", [dict(max_delta=0)])
        path = self.root / "PROVENANCE.json"
        data = json.loads(path.read_text())
        data["current"]["working_tree"] = "dirty"
        path.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, "exact-source"):
            collect(self.root)
        for private in ("X:/private/input.wav", "X:\\private\\input.wav", "/private/input.wav"):
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


if __name__ == "__main__":
    unittest.main()
