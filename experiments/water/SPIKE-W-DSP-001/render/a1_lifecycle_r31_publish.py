"""Publish canonical R3.1 CSVs from one completed local study; never copy audio/paths.

Expected layout: reference/{MEASUREMENTS,L1_MEASUREMENTS,BANDS,PRESERVATION}.csv,
reference/source-*/L1_BANDS.csv, preservation/PRESERVATION.csv,
performance-{before,l0,l1,l1-trace}.csv, STATUS.json and PROVENANCE.json.
The provenance file stays local; its two renderer digests are the plan's explicit exception.
"""

import argparse
import csv
import io
import hashlib
import json
from pathlib import Path
import re
import subprocess


def read(path):
    with path.open(encoding="utf-8", newline="") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise ValueError(f"Empty evidence: {path.name}")
    return rows


def encode(rows):
    columns = list(dict.fromkeys(key for row in rows for key in row))
    stream = io.StringIO(newline="")
    writer = csv.DictWriter(stream, columns, lineterminator="\n")
    writer.writeheader()
    for row in rows:
        clean = {key: "N/A" if row.get(key) in (None, "") else str(row[key]) for key in columns}
        if any(re.search(r"[A-Za-z]:[\\/]|\\\\|^/", value) for value in clean.values()):
            raise ValueError("Absolute/private path in publishable evidence")
        writer.writerow(clean)
    return stream.getvalue()


BASELINE_SHA = "1092ba01007d70a66ac5204c7d0d7cb070421972"
RATES = (44100, 48000, 96000)
SOURCE_RATES = {f"source-{i:02}": rate for i, rate in enumerate(
    (44100, 48000, 48000, 44100, 48000, 48000), 1)}


def require_keys(rows, fields, expected, label):
    keys = [tuple(str(row[field]) for field in fields) for row in rows]
    if len(keys) != len(expected) or set(keys) != expected:
        raise ValueError(f"Incomplete/duplicate/unexpected {label}")


def verify_provenance(root, expected_baseline_sha, expected_current_sha,
                      baseline_binary, current_binary):
    """Bind both recorded renderer digests to clean live Git checkouts; only hash these binaries."""
    if expected_baseline_sha != BASELINE_SHA:
        raise ValueError("Wrong expected baseline SHA")
    provenance = json.loads((root / "PROVENANCE.json").read_text())
    for role, expected, binary in (("baseline", expected_baseline_sha, baseline_binary),
                                   ("current", expected_current_sha, current_binary)):
        entry = provenance[role]
        if (entry["working_tree"] != "clean" or
                not re.fullmatch(r"[0-9a-f]{40}", expected) or entry["source_sha"] != expected or
                not re.fullmatch(r"[0-9a-f]{64}", entry["binary_sha256"])):
            raise ValueError(f"Invalid clean exact-source provenance: {role}")
        binary = Path(binary).resolve(strict=True)
        def git(*args):
            return subprocess.check_output(["git", "-C", str(binary.parent), *args],
                                           text=True).strip()
        if git("rev-parse", "HEAD") != expected or git("status", "--porcelain"):
            raise ValueError(f"Live Git HEAD/tree mismatch: {role}")
        with binary.open("rb") as stream:
            digest = hashlib.file_digest(stream, "sha256").hexdigest()
        if digest != entry["binary_sha256"]:
            raise ValueError(f"Binary digest mismatch: {role}")


def collect(root, *, expected_baseline_sha, expected_current_sha, baseline_binary, current_binary):
    if json.loads((root / "STATUS.json").read_text())["status"] != "ENGINEERING COMPLETE":
        raise ValueError("Study is incomplete")
    verify_provenance(root, expected_baseline_sha, expected_current_sha, baseline_binary, current_binary)
    reference = root / "reference"
    l0 = read(reference / "MEASUREMENTS.csv")
    l1 = read(reference / "L1_MEASUREMENTS.csv")
    ids = list(SOURCE_RATES)
    for policy, values in (("L0", l0), ("L1", l1)):
        require_keys(values, ("policy", "source_id", "rate"),
                     {(policy, sid, str(rate)) for sid, rate in SOURCE_RATES.items()},
                     f"{policy} reference sources/rates")
    rows = l0 + l1
    for row in rows:
        for numerator, denominator in (
                ("firstNonZero", "requested"), ("firstNonZero", "started"),
                ("completedWithoutNonZero", "started"),
                ("causedStealButNeverNonZero", "steals"),
                ("causedStealButNeverNonZero", "causedSteal"),
                ("riseEnabledAndFirstNonZero", "firstNonZero")):
            row[f"{numerator}_per_{denominator}"] = (
                int(row[numerator]) / int(row[denominator]) if int(row[denominator]) else "N/A")
    bands = [dict(policy="L0", **row) for row in read(reference / "BANDS.csv")]
    for sid in ids:
        bands.extend(dict(policy="L1", source_id=sid, **row)
                     for row in read(reference / sid / "L1_BANDS.csv"))
    require_keys(bands, ("policy", "source_id", "band"),
                 {(policy, sid, str(band)) for policy in ("L0", "L1")
                  for sid in ids for band in range(7)}, "seven-band policy evidence")
    preservation = read(root / "preservation" / "PRESERVATION.csv") + read(reference / "PRESERVATION.csv")
    expected = {(str(rate), mode, "old-new") for rate in RATES
                for mode in ("a1", "a1b1", "a1b2", "a1b1d1", "a1b2d1", "b1", "b2", "abd", "c")}
    expected |= {(str(rate), mode, "trace-partition") for rate in RATES
                 for mode in ("a1", "a1b1", "a1b2", "a1b1d1", "a1b2d1")}
    expected |= {(str(rate), "a1-dense64", "old-new-trace") for rate in RATES}
    expected |= {(str(rate), sid + "-a1", comparison) for sid, rate in SOURCE_RATES.items()
                 for comparison in ("old-new-real-source", "l1-full-equation")}
    require_keys(preservation, ("rate", "mode", "comparison"), expected, "preservation matrix")
    if any(float(row["max_delta"]) != 0 for row in preservation):
        raise ValueError("STOP: preservation failed")
    performance = []
    for variant in ("before", "l0", "l1", "l1-trace"):
        measurements = read(root / f"performance-{variant}.csv")
        keys = {(int(r["rate"]), int(r["capacity"]), r["profile"]) for r in measurements}
        expected = {(r, c, p) for r in (44100, 48000, 96000) for c in (64, 128, 256, 512, 1024)
                    for p in ("physical-reference", "dense-stress")}
        if keys != expected or len(measurements) != len(expected):
            raise ValueError(f"Incomplete performance matrix: {variant}")
        for row in measurements:
            deadline = 128 / int(row["rate"]) * 1e6
            performance.append(dict(variant=variant, **row, deadline_us=deadline,
                                    deadline_exceeded=float(row["worst_us"]) > deadline))
    return {name: encode(data) for name, data in (
        ("WATER_A1_R31_REFERENCE.csv", rows), ("WATER_A1_R31_BANDS.csv", bands),
        ("WATER_A1_R31_PRESERVATION.csv", preservation), ("WATER_A1_R31_PERFORMANCE.csv", performance))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("study", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--expected-baseline-sha", required=True)
    parser.add_argument("--expected-current-sha", required=True)
    parser.add_argument("--baseline-binary", type=Path, required=True)
    parser.add_argument("--current-binary", type=Path, required=True)
    args = parser.parse_args()
    outputs = collect(args.study.resolve(), expected_baseline_sha=args.expected_baseline_sha,
                      expected_current_sha=args.expected_current_sha,
                      baseline_binary=args.baseline_binary, current_binary=args.current_binary)  # Validate everything before writing any destination.
    args.output.mkdir(parents=True, exist_ok=True)
    for name, content in outputs.items():
        (args.output / name).write_text(content, encoding="utf-8", newline="")
    print("Published four deterministic canonical CSVs; no audio/private paths copied")


if __name__ == "__main__":
    main()
