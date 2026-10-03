"""Three complete fixed-order observation timing rounds; retain every row and range.

The separate workload pass is not timing evidence. No automatic winner or gate decision.
"""

import argparse
import csv
import json
from pathlib import Path
import statistics

from a1_lifecycle_r31_publish import encode, read
from native_case_evidence import run_case


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--current", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = args.output.resolve()
    if not root.is_relative_to(Path(__file__).resolve().parents[4] / "build"):
        parser.error("Output must stay under build/")
    root.mkdir(parents=True, exist_ok=False)
    variants = [("before", args.baseline, []), ("l0", args.current, []),
                ("l1", args.current, ["--l1"]), ("l1-trace", args.current, ["--l1", "--trace"]),
                ("l0-observer", args.current, ["--observer-only"]),
                ("l0-trace", args.current, ["--trace"])]
    (root / "protocol.json").write_text(json.dumps(dict(rounds=3, block=128, warmup=500,
        measured=3000, order=[v[0] for v in variants]), indent=2), encoding="utf-8")
    combined = []
    expected = {(str(r), str(c), p) for r in (44100, 48000, 96000)
                for c in (64, 128, 256, 512, 1024) for p in ("physical-reference", "dense-stress")}
    for iteration in range(1, 4):
        for label, executable, flags in variants:
            case = f"round-{iteration}-{label}"
            directory = root / case
            run_case([str(executable.resolve()), *flags], directory, case, None)
            rows = read(directory / "stdout.log")
            if len(rows) != 30 or {(r["rate"], r["capacity"], r["profile"]) for r in rows} != expected:
                raise ValueError(f"Incomplete matrix: {case}")
            combined.extend(dict(round=iteration, variant=label, **row) for row in rows)
            (root / "RUNS.csv").write_text(encode(combined), encoding="utf-8", newline="")
    summaries = []
    for label, _, _ in variants:
        for rate, capacity, profile in sorted(expected):
            rows = [r for r in combined if (r["variant"], r["rate"], r["capacity"], r["profile"])
                    == (label, rate, capacity, profile)]
            result = dict(variant=label, rate=rate, capacity=capacity, profile=profile)
            for metric in ("mean_us", "p95_us", "p99_us", "worst_us"):
                values = [float(r[metric]) for r in rows]
                result.update({metric + "_median": statistics.median(values),
                               metric + "_min": min(values), metric + "_max": max(values)})
            summaries.append(result)
    (root / "SUMMARY.csv").write_text(encode(summaries), encoding="utf-8", newline="")
    for label, flags in (("l0", []), ("l1", ["--l1"])):
        directory = root / (label + "-workload")
        run_case([str(args.current.resolve()), *flags, "--workload"], directory, directory.name, None)
        rows = read(directory / "stdout.log")
        # Numeric output and scheduling must be unaffected by audit-only counting.
        timing = [r for r in combined if r["round"] == 1 and r["variant"] == label]
        assert len(rows) == len(timing) == 30
        for audit, timed in zip(rows, timing):
            for field in ("rate", "capacity", "profile", "output_sum", "events_per_second",
                          "starts_per_second", "firstNonZero_per_second", "steals", "drops"):
                if audit[field] != str(timed[field]):
                    raise ValueError(f"Workload changed {field}: {label}")
        (root / (label + "-WORKLOAD.csv")).write_text(encode(rows), encoding="utf-8", newline="")


if __name__ == "__main__":
    main()
