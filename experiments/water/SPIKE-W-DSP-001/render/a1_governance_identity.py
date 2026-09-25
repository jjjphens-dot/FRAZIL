"""Exact decoded regression using preserved A1/B1 executables and existing study configs.

Offline only; no new synthesis path, hashes, normalization or human decisions.
Successful temporary WAVs are removed; a failed pair and its logs remain for inspection.
"""
import argparse
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("renderer", "a1-baseline", "b1-baseline", "a1-study", "b1-study", "output"):
        parser.add_argument("--" + name, required=True, type=Path)
    parser.add_argument("--input", required=True, action="append", type=Path)
    parser.add_argument("--a1-reference", default="95be0de109c66be6ab218a9ac65384eabde3815e")
    parser.add_argument("--b1-reference", default="8f3b7ca06073439d9d47742f2a3b92a361029c1f")
    parser.add_argument("--matrix", action="store_true", help="Also compare all closeout rates/partitions/modes")
    parser.add_argument("--matrix-only", action="store_true", help="Run only the additional matrix")
    args = parser.parse_args()
    if args.output.exists():
        parser.error("Output must be a new directory; preserve previous evidence")
    sources = {p.name: p.resolve(strict=True) for p in args.input}
    if len(sources) != len(args.input) or not 1 <= len(sources) <= 6:
        parser.error("Use 1..6 explicitly authorized sources with distinct filenames")
    a1 = json.loads((args.a1_study / "report.json").read_text(encoding="utf-8"))
    b1 = json.loads((args.b1_study / "report.json").read_text(encoding="utf-8"))
    if not {r["source"] for r in a1["renders"] + b1["renders"]} <= sources.keys():
        parser.error("Every study source must be explicitly supplied")
    args.output.mkdir(parents=True)
    rows = []
    report = {"a1_reference": args.a1_reference,
              "b1_reference": args.b1_reference,
              "seed": 42, "block": 128, "tail_seconds": 3, "comparisons": rows}

    def compare(label, source, mode, config, baseline, block=128, tail=3):
        config_path = args.output / "config.json"
        config_path.write_text(json.dumps(config), encoding="utf-8")
        decoded = []
        paths = []
        for tag, executable in (("before", baseline), ("after", args.renderer)):
            target = args.output / (tag + ".wav")
            paths.append(target)
            result = subprocess.run([str(executable.resolve()), str(source), str(target.resolve()),
                                     mode, str(block), "42", str(config_path.resolve()), str(tail)],
                                    capture_output=True, text=True)
            (args.output / (tag + ".log")).write_text(result.stdout + result.stderr, encoding="utf-8")
            if result.returncode:
                raise RuntimeError((label, source.name, mode, tag, result.returncode))
            audio, rate = sf.read(target, always_2d=True, dtype="float32")
            if not np.isfinite(audio).all():
                raise AssertionError((label, "nonfinite", tag))
            decoded.append((audio, rate))
        if decoded[0][1] != decoded[1][1] or not np.array_equal(decoded[0][0], decoded[1][0]):
            raise AssertionError((label, source.name, mode, "decoded identity FAIL; pair retained"))
        rows.append({"group": label, "source": source.name, "mode": mode, "config": config,
                     "frames": len(decoded[0][0]), "rate": decoded[0][1], "block": block,
                     "tail_seconds": tail, "decoded_exact": True})
        for target in paths:
            target.unlink()  # Only the two files created in this new output directory.
        (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    if not args.matrix_only:
        # Every historical A1 condition/source, in every requested full/residual composition.
        for row in a1["renders"]:
            if row["case"] == "A0":
                continue
            for mode in ("a1", "a1-residual", "a1b", "a1b-residual", "a1d", "a1d-residual",
                         "a1bd", "a1bd-residual"):
                compare("A1:" + row["case"], sources[row["source"]], mode, row["config"], args.a1_baseline)
            print("PASS A1", row["source"], row["case"], flush=True)
        for source in sources.values():
            for mode in ("a", "b", "d", "ab", "ad", "bd", "c", "abd"):
                for suffix in ("", "-residual"):
                    compare("legacy", source, mode + suffix, {}, args.a1_baseline)
            print("PASS legacy", source.name, flush=True)
        cases = {c["name"]: c for c in b1["cases"]}
        for row in b1["renders"]:
            case = cases[row["case"]]
            for suffix in ("", "-residual"):
                compare("B1:" + case["name"], sources[row["source"]], case["mode"] + suffix,
                        case["config"], args.b1_baseline)
            print("PASS B1", row["source"], row["case"], flush=True)
    if args.matrix or args.matrix_only:
        # A single deterministic fixture sequences the requested stereo conditions.
        # This is engineering input, not musical listening evidence.
        for rate in (44100, 48000, 96000):
            n = np.arange(int(rate * .04))
            phase = 2 * np.pi * 173 * n / rate
            left = np.where(n < rate * .008, .7 * np.cos(phase), 0)
            right = np.where(n < rate * .008, .7 * np.sin(phase), 0)
            asymmetric = left.copy()
            asymmetric[int(rate * .02)] = .9
            fixture = np.concatenate([np.column_stack(pair) for pair in (
                (left, left), (left, left * 0), (left * 0, left), (left, -left),
                (left, right), (right, left), (left, left * .03), (asymmetric, left * .001))])
            source = args.output / f"stereo-matrix-{rate}.wav"
            sf.write(source, fixture, rate, subtype="FLOAT")
            for block in (1, 7, 32, 64, 128, 256, 257, 512, 1024):
                for mode in ("a", "b", "d", "ab", "ad", "bd", "abd", "c", "a1", "a1-residual",
                             "a1b", "a1d", "a1bd", "b1", "b1-residual", "a1b1", "a1b1d"):
                    baseline = args.b1_baseline if "b1" in mode else args.a1_baseline
                    compare("rate-partition-stereo", source, mode, {}, baseline, block, 1)
            print("PASS matrix", rate, flush=True)
    print(f"PASS {len(rows)} exact decoded pairs; no perceptual acceptance", flush=True)


if __name__ == "__main__":
    main()
