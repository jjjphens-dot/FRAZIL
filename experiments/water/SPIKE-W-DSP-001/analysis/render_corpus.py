"""Render all existing diagnostic WAVs through independent research modes; no hashes.

Requires the existing requirements-dsp.txt dependencies. Output directory must be new.
All generated WAV/JSON/CSV artifacts belong in ignored build/ or testdata/rendered/.
"""
import argparse
import csv
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools"))
from analyze_testdata import analyze_audio
import numpy as np
import soundfile as sf


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    config = ROOT / "experiments/water/SPIKE-W-DSP-001/configs/defaults.json"
    rows = []
    for source in sorted((ROOT / "testdata/input").glob("*.wav")):
        dry, rate = sf.read(source, always_2d=True)
        modes = {}
        for mode in ("a", "b", "d", "ab", "ad", "bd", "abd", "c"):
            output = args.output / f"{source.stem}__{mode}.wav"
            result = subprocess.run([str(args.renderer.resolve()), str(source), str(output.resolve()),
                                     mode, "128", "42", str(config), "3"], check=True, capture_output=True, text=True)
            metrics = analyze_audio(output, include_spectrogram=False)
            processed, actual_rate = sf.read(output, always_2d=True)
            assert actual_rate == rate and processed.shape[0] == dry.shape[0] + 3 * rate
            residual = processed.copy()
            residual[:len(dry)] -= dry
            modes[mode] = residual
            tail = processed[len(dry):]
            row = {"fixture":source.stem, "mode":mode, "seed":42, "sample_rate":rate,
                   "finite":metrics["finite"], "peak":metrics["peak"], "rms":metrics["rms"],
                   "dc":metrics["dc"], "residual_rms":float(np.sqrt(np.mean(residual**2))),
                   "last_100ms_peak":float(np.max(np.abs(tail[-rate//10:])))}
            assert row["finite"]
            if "zero_input" in source.stem: assert row["peak"] == 0
            assert row["last_100ms_peak"] < 1e-8  # v0 defaults, not max-decay configurations.
            if "stereo_isolation" in source.stem:
                # A tail in a previously excited channel is intentional; the dedicated CLI
                # isolation tests use an entirely unexcited channel to establish no crossfeed.
                row["isolation_note"] = "see unexcited-channel automated test"
            metrics["path"] = output.name
            metrics["renderer"] = result.stdout.strip()
            output.with_suffix(".json").write_text(json.dumps(metrics, indent=2), encoding="utf-8")
            rows.append(row)
        for mode in ("ab", "ad", "bd", "abd"):
            expected = sum(modes[component] for component in mode)
            assert np.max(np.abs(modes[mode] - expected)) < 2e-7
        print(f"{source.name}: 8 modes, finite/tail/ablation PASS", flush=True)
    with (args.output / "metrics.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0])+["isolation_note"], extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)
    print(f"PASS: {len(rows)} renders; objective diagnostics only, no listening acceptance")


if __name__ == "__main__":
    main()
