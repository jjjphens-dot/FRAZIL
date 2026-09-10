#!/usr/bin/env python3
"""Generate FRAZIL's deterministic DSP diagnostic signal corpus."""

from __future__ import annotations

import argparse
from pathlib import Path

try:
    # Keep the historical helper imports available to test and analysis scripts.
    from testdata_generation.manifest import create_manifest
    from testdata_generation.renderers import render
    from testdata_generation.specs import (
        BASE_SEED,
        CORPUS,
        HF_FREQUENCY_SAMPLE_RATE_RATIO,
        IMPULSE_LEVEL_DBFS,
        NOISE_TARGET_RMS_DBFS,
        SAMPLE_RATE,
        SUPPORTED_SAMPLE_RATES,
        SignalSpec,
        dbfs_to_linear,
        frames_for_duration,
        stable_seed,
    )
    from testdata_generation.wav_io import write_wav
except ImportError:
    # Also support importing this facade as tools.generate_testdata.
    from .testdata_generation.manifest import create_manifest
    from .testdata_generation.renderers import render
    from .testdata_generation.specs import (
        BASE_SEED,
        CORPUS,
        HF_FREQUENCY_SAMPLE_RATE_RATIO,
        IMPULSE_LEVEL_DBFS,
        NOISE_TARGET_RMS_DBFS,
        SAMPLE_RATE,
        SUPPORTED_SAMPLE_RATES,
        SignalSpec,
        dbfs_to_linear,
        frames_for_duration,
        stable_seed,
    )
    from .testdata_generation.wav_io import write_wav

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_DIR = ROOT / "testdata" / "input"
DEFAULT_MANIFEST = ROOT / "testdata" / "manifest.json"


def generate_corpus(
    input_dir: Path,
    manifest_path: Path,
    manifest_root: Path,
    sample_rate: int = SAMPLE_RATE,
) -> None:
    if sample_rate not in SUPPORTED_SAMPLE_RATES:
        raise ValueError(f"sample_rate must be one of {SUPPORTED_SAMPLE_RATES}")
    input_dir = input_dir.resolve()
    manifest_path = manifest_path.resolve()
    manifest_root = manifest_root.resolve()
    input_dir.mkdir(parents=True, exist_ok=True)
    for spec in CORPUS:
        write_wav(input_dir / spec.filename, render(spec.id, sample_rate), sample_rate)
    create_manifest(input_dir, manifest_path, manifest_root, sample_rate)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_INPUT_DIR)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument(
        "--manifest-root",
        type=Path,
        default=ROOT,
        help="Root used to write repository-relative paths into the manifest.",
    )
    parser.add_argument(
        "--sample-rate",
        type=int,
        choices=SUPPORTED_SAMPLE_RATES,
        default=SAMPLE_RATE,
        help="Sample rate for this generated corpus; committed fixtures use 48000.",
    )
    args = parser.parse_args()

    generate_corpus(args.input_dir, args.manifest, args.manifest_root, args.sample_rate)
    print(
        f"Generated {len(CORPUS)} deterministic DSP diagnostic WAV fixtures "
        f"at {args.sample_rate} Hz in {args.input_dir}"
    )
    print(f"Wrote manifest to {args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
