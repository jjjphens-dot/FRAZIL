"""Inspect explicit batches of local Water references using the existing analyzer.

This offline helper never copies audio or infers perceptual labels. The registry uses
stable IDs and paths relative to an external library; adding files cannot renumber IDs.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from analyze_testdata import analyze_audio, write_plots


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--ids", nargs="+", required=True)
    parser.add_argument("--analyze", action="store_true")
    parser.add_argument("--plots", action="store_true",
                        help="With --analyze, write existing analyzer plots beside the JSON")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.plots and not args.analyze:
        parser.error("--plots requires --analyze and development references")
    if not 1 <= len(args.ids) <= 6 or len(set(args.ids)) != len(args.ids):
        parser.error("Select 1-6 unique reference IDs per intake batch")

    import soundfile as sf

    registry_path = Path(__file__).with_name("REFERENCE_INDEX.csv")
    with registry_path.open(encoding="utf-8", newline="") as stream:
        rows = list(csv.DictReader(stream))
    registry = {row["id"]: row for row in rows}
    if len(registry) != len(rows):
        parser.error("Duplicate reference IDs in registry")
    if any(ref_id not in registry for ref_id in args.ids):
        parser.error("Unknown reference ID")
    root = args.root.resolve(strict=True)
    output = args.output.resolve()
    # Measurements contain local-only evidence and must stay in the ignored tree.
    if output.suffix.lower() != ".json" or not output.is_relative_to(
        ROOT / "testdata" / "rendered"
    ):
        parser.error("Output must be a JSON file inside repository testdata/rendered/")
    results = []
    for ref_id in args.ids:
        row = registry[ref_id]
        path = (root / row["relative_path"]).resolve(strict=True)
        if not path.is_relative_to(root):
            parser.error("Reference path escapes the supplied library root")
        if args.analyze and row["split"] != "development":
            parser.error("Analysis requires an explicitly assigned development reference")
        info = sf.info(path)
        record = {
            **row,
            "bytes": path.stat().st_size,
            "sample_rate": info.samplerate,
            "channels": info.channels,
            "frames": info.frames,
            "duration_seconds": info.duration,
            "subtype": info.subtype,
            "format": info.format,
        }
        if args.analyze:
            record["qa"] = analyze_audio(path, include_spectrogram=False)
            record["qa"]["path"] = row["relative_path"]
        results.append(record)
    # Validate every reference before creating plots. Stable IDs avoid title-based
    # output paths; figures are observations, never automatic perceptual labels.
    if args.plots:
        import matplotlib
        from matplotlib import font_manager

        available_fonts = {font.name for font in font_manager.fontManager.ttflist}
        title_fonts = [name for name in ("Microsoft YaHei", "Noto Sans CJK SC")
                       if name in available_fonts]
        # Scope font preferences to these plots; do not change analyzer/global defaults.
        with matplotlib.rc_context({"font.family": title_fonts + ["DejaVu Sans"]}):
            for record in results:
                write_plots(root / record["relative_path"], output.parent / record["id"])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(results, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
    )
    print(f"Inspected {len(results)} references; analysis={args.analyze}")


if __name__ == "__main__":
    main()
