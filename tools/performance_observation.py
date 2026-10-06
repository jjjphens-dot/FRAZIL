#!/usr/bin/env python3
"""Run existing canonical benchmarks and validate observations, not timing budgets."""
from __future__ import annotations

import argparse
import csv
import io
import itertools
import math
import re
import subprocess
import sys


def expected_cases(kind: str) -> tuple[tuple[str, ...], set[tuple[str, ...]]]:
    rates = ("44100", "48000", "96000")
    if kind == "a1":
        return ("rate", "capacity", "profile"), set(itertools.product(
            rates, ("64", "128", "256", "512", "1024"), ("physical-reference", "dense-stress")))
    if kind == "b1":
        return ("rate", "capacity", "radius_mm", "persistence", "profile"), set(itertools.product(
            rates, ("16", "32", "64", "128", "256"), ("0.2", "7"), ("0.25", "4"),
            ("normal-onsets", "stress-onsets")))
    if kind in {"d1", "d1-current"}:
        return ("rate", "block", "profile"), set(itertools.product(
            rates, ("32", "128", "1024"), ("D1", "A1+B2+D1") if kind == "d1-current" else
            ("linear-kernel", "lagrange3-kernel", "D1", "A1+B1+D1")))
    if kind == "legacy":
        names = ["M1", "M1+C", "M1+D", "A", "B", "AB", "AD", "BD", "ABD"]
        names += [f"C_motion_{depth}" for depth in ("0.000000", "0.175000", "0.350000")]
        for mode in ("F", "C"):
            names.append(f"protect_{mode}_reference")
            names += [f"protect_{mode}_{score}_{depth}" for score in ("D0", "D1")
                      for depth in ("0.000000", "0.500000", "1.000000")]
        return ("case",), {(name,) for name in names}
    raise ValueError(f"unknown observation: {kind}")


def validate(kind: str, output: str) -> None:
    if kind == "b2":
        seen = set()
        for line in output.splitlines():
            if not line.startswith("B2 diagnostic "):
                continue
            fields = dict(re.findall(r"(\w+)=([^ ]+)", line))
            identity = (fields["rate"], fields["fixture"])
            if identity in seen:
                raise ValueError("duplicate B2 observation")
            seen.add(identity)
            for key in ("eligible", "mean_callback_us", "max_callback_us", "callback_budget_us"):
                if not math.isfinite(float(fields[key])) or float(fields[key]) < 0:
                    raise ValueError(f"invalid B2 observation: {key}")
        if seen != set(itertools.product(("44100", "48000", "96000"), ("0", "1"))):
            raise ValueError("incomplete B2 observations")
        return
    if kind == "product":
        scenarios = output.split("scenario=")[1:]
        if len(scenarios) != 2 or {s.splitlines()[0] for s in scenarios} != {"steady-state", "parameter-retarget"}:
            raise ValueError("missing/duplicate product scenarios")
        for scenario in scenarios:
            fields = dict(line.split("=", 1) for line in scenario.splitlines()[1:] if "=" in line)
            if fields.get("finite_output_status") != "PASS" or fields.get("status") != "PASS":
                raise ValueError("product finite output failed")
            for field in ("mean_callback_us", "p95_callback_us", "p99_callback_us", "worst_callback_us",
                          "wall_seconds", "measured_blocks"):
                if not math.isfinite(float(fields[field])) or float(fields[field]) < 0:
                    raise ValueError(f"invalid product measurement: {field}")
            if float(fields["measured_blocks"]) <= 0:
                raise ValueError("empty product measurement")
        if "denormal_finite_output_status=PASS" not in output:
            raise ValueError("missing denormal finite output result")
        return
    keys, expected = expected_cases(kind)
    lines = output.splitlines()
    # The legacy program emits two provenance lines before its CSV header.
    start = next((i for i, line in enumerate(lines) if line.startswith(keys[0] + ",")), None)
    if start is None:
        raise ValueError("missing CSV header")
    reader = csv.DictReader(io.StringIO("\n".join(lines[start:])))
    if len(reader.fieldnames or ()) != len(set(reader.fieldnames or ())):
        raise ValueError("duplicate CSV columns")
    required = {*keys, "mean_us", "p95_us", "p99_us", "worst_us", "output_sum"}
    if not required.issubset(reader.fieldnames or ()):
        raise ValueError("missing measurement columns")
    seen = set()
    for row in reader:
        if None in row or any(value is None for value in row.values()):
            raise ValueError("malformed CSV row")
        identity = tuple(row[key] for key in keys)
        if identity in seen or identity not in expected:
            raise ValueError(f"duplicate/unexpected measurement: {identity}")
        seen.add(identity)
        for key, value in row.items():
            if key in {"case", "profile", "policy", "measurement_kind"}:
                continue
            if not math.isfinite(float(value)):
                raise ValueError(f"nonfinite observation: {identity} {key}")
            if key in {"mean_us", "p95_us", "p99_us", "worst_us"} and float(value) < 0:
                raise ValueError("negative elapsed measurement")
    if seen != expected:
        raise ValueError(f"missing {len(expected - seen)} measurements")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("product", "legacy", "a1", "b1", "b2", "d1", "d1-current"))
    parser.add_argument("executable")
    args = parser.parse_args()
    command = [args.executable] + (["--current"] if args.kind == "d1-current" else [])
    result = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=180)
    # Keep native streams even on failure; never retry a measurement inside this adapter.
    print(result.stdout, end="")
    print(result.stderr, end="", file=sys.stderr)
    if result.returncode:
        print(f"Native observation exit: {result.returncode}", file=sys.stderr)
        return 1
    try:
        validate(args.kind, result.stdout)
    except (ValueError, KeyError) as error:
        print(f"Observation validation failed: {error}", file=sys.stderr)
        return 1
    print("Observation schema/cases/finite values: PASS; timing budget: NOT EVALUATED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
