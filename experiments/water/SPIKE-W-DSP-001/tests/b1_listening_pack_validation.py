"""Real renderer, strict B1 schema/descriptor and decoded audio invariants."""
import json
import csv
from pathlib import Path
import subprocess
import sys
import numpy as np
import soundfile as sf


from cli_support import workspace


from cli_support import run_main

def main():
    exe = Path(sys.argv[1]).resolve()
    experiment = Path(__file__).resolve().parents[1]
    descriptor = json.loads(subprocess.check_output([str(exe), "--describe-droplet-b1"], text=True))
    assert descriptor == json.loads((experiment.parent / "contracts/droplet-b1-v1.json").read_text())
    with workspace() as root:
        config = root / "config.json"
        source = root / "source.wav"
        # Exercise the real study builder. Using this executable as both versions only
        # tests pack mechanics; historical identity requires the separate baseline run.
        sf.write(source, np.zeros((4410, 2)), 44100, subtype="FLOAT")
        pack = root / "pack"
        subprocess.run([sys.executable, str(experiment/"render/droplet_b1_study.py"),
                        "--renderer", str(exe), "--baseline-renderer", str(exe),
                        "--input", str(source), "--engineering-input", str(source),
                        "--output", str(pack)], check=True, capture_output=True, text=True)
        report = json.loads((pack/"report.json").read_text())
        assert len(report["cases"]) == 26 and len(report["legacy_regressions"]) == 10
        assert all(r["repeat_partition_exact"] for r in report["renders"])
        assert all(not r["assessable"] for r in report["matching"])
        for reviewer in (1, 2):
            with (pack/f"reviewer-{reviewer}.csv").open(encoding="utf-8-sig") as f:
                rows = list(csv.DictReader(f))
            assert len(rows) == 30 and all(r["decision"] == "NOT ASSESSED" and not r["reviewer"] for r in rows)


if __name__ == "__main__":
    run_main(main)
