"""Publish canonical R3.1 CSVs from one completed local study; never copy audio/paths.

Expected layout: reference/{MEASUREMENTS,L1_MEASUREMENTS,BANDS,PRESERVATION}.csv,
reference/source-*/L1_BANDS.csv, preservation/PRESERVATION.csv,
performance-{before,l0,l1,l1-trace}.csv, STATUS.json and PROVENANCE.json.
The provenance file stays local; its two renderer digests are the plan's explicit exception.
"""

import argparse
import csv
import io
import json
from pathlib import Path
import re


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


def collect(root):
    if json.loads((root / "STATUS.json").read_text())["status"] != "ENGINEERING COMPLETE":
        raise ValueError("Study is incomplete")
    provenance = json.loads((root / "PROVENANCE.json").read_text())
    for role in ("baseline", "current"):
        entry = provenance[role]
        if (entry["working_tree"] != "clean" or
                not re.fullmatch(r"[0-9a-f]{40}", entry["source_sha"]) or
                not re.fullmatch(r"[0-9a-f]{64}", entry["binary_sha256"])):
            raise ValueError(f"Missing clean exact-source binary provenance: {role}")
    reference = root / "reference"
    l0 = read(reference / "MEASUREMENTS.csv")
    l1 = read(reference / "L1_MEASUREMENTS.csv")
    ids = [r["source_id"] for r in l0]
    if ids != [r["source_id"] for r in l1] or len(set(ids)) != len(ids):
        raise ValueError("L0/L1 source identities differ")
    if any(not re.fullmatch(r"source-\d{2}", sid) for sid in ids):
        raise ValueError("Only anonymized source IDs may be published")
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
    if len(bands) != len(ids) * 14:
        raise ValueError("Incomplete seven-band policy evidence")
    preservation = read(root / "preservation" / "PRESERVATION.csv") + read(reference / "PRESERVATION.csv")
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
    args = parser.parse_args()
    outputs = collect(args.study.resolve())  # Validate everything before writing any destination.
    args.output.mkdir(parents=True, exist_ok=True)
    for name, content in outputs.items():
        (args.output / name).write_text(content, encoding="utf-8", newline="")
    print("Published four deterministic canonical CSVs; no audio/private paths copied")


if __name__ == "__main__":
    main()
