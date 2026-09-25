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
    report = {"a1_reference": "95be0de109c66be6ab218a9ac65384eabde3815e",
              "b1_reference": "8f3b7ca06073439d9d47742f2a3b92a361029c1f",
              "seed": 42, "block": 128, "tail_seconds": 3, "comparisons": rows}

    def compare(label, source, mode, config, baseline):
        config_path = args.output / "config.json"
        config_path.write_text(json.dumps(config), encoding="utf-8")
        decoded = []
        paths = []
        for tag, executable in (("before", baseline), ("after", args.renderer)):
            target = args.output / (tag + ".wav")
            paths.append(target)
            result = subprocess.run([str(executable.resolve()), str(source), str(target.resolve()),
                                     mode, "128", "42", str(config_path.resolve()), "3"],
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
                     "frames": len(decoded[0][0]), "rate": decoded[0][1], "decoded_exact": True})
        for target in paths:
            target.unlink()  # Only the two files created in this new output directory.
        (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    # Every historical A1 condition/source, in every requested full/residual composition.
    for row in a1["renders"]:
        if row["case"] == "A0":
            continue
        for mode in ("a1", "a1-residual", "a1b", "a1b-residual", "a1d", "a1d-residual",
                     "a1bd", "a1bd-residual"):
            compare("A1:" + row["case"], sources[row["source"]], mode, row["config"], args.a1_baseline)
        print("PASS A1", row["source"], row["case"], flush=True)
    for source in sources.values():
        for mode in ("a", "b", "d", "bd", "c", "abd"):
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
    print(f"PASS {len(rows)} exact decoded pairs; no perceptual acceptance", flush=True)


if __name__ == "__main__":
    main()
