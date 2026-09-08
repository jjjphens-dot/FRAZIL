#!/usr/bin/env python3
"""Run a deterministic offline render smoke and write its audit manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT = ROOT / "testdata" / "input" / "impulse.wav"
DEFAULT_OUTPUT = ROOT / "testdata" / "rendered" / "impulse.wav"
DEFAULT_MANIFEST = ROOT / "testdata" / "rendered" / "impulse.manifest.json"
DEFAULT_SEED = 20260908
DEFAULT_BUILD_TYPE = "Debug"
ROUTING_MODES = ("parallel", "water-into-ice", "ice-into-water")


def parse_bool(value: str) -> bool:
    if value not in {"0", "1"}:
        raise argparse.ArgumentTypeError("expected 0 or 1")
    return value == "1"


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def stable_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(ROOT).as_posix()
    except ValueError:
        return resolved.name


def wav_metadata(path: Path) -> dict[str, int | float | str]:
    data = path.read_bytes()
    if len(data) < 12 or data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError(f"not a RIFF/WAVE file: {path}")

    format_tag = None
    channels = None
    sample_rate = None
    block_align = None
    bit_depth = None
    data_size = None
    offset = 12
    while offset + 8 <= len(data):
        chunk_id = data[offset : offset + 4]
        chunk_size = struct.unpack_from("<I", data, offset + 4)[0]
        chunk_start = offset + 8
        chunk_end = chunk_start + chunk_size
        if chunk_end > len(data):
            raise ValueError(f"truncated WAV chunk in {path}")
        if chunk_id == b"fmt " and chunk_size >= 16:
            (
                format_tag,
                channels,
                sample_rate,
                _byte_rate,
                block_align,
                bit_depth,
            ) = struct.unpack_from("<HHIIHH", data, chunk_start)
            if format_tag == 0xFFFE and chunk_size >= 40:
                format_tag = struct.unpack_from("<I", data, chunk_start + 24)[0]
        elif chunk_id == b"data":
            data_size = chunk_size
        offset = chunk_end + (chunk_size & 1)

    if (
        format_tag is None
        or channels is None
        or sample_rate is None
        or block_align is None
        or bit_depth is None
        or data_size is None
        or channels == 0
        or sample_rate == 0
        or block_align == 0
    ):
        raise ValueError(f"WAV is missing required fmt/data metadata: {path}")

    if format_tag == 1:
        format_name = f"PCM_S{bit_depth}LE"
    elif format_tag == 3:
        format_name = f"IEEE_FLOAT{bit_depth}LE"
    else:
        format_name = "unknown"
    frames = data_size // block_align
    return {
        "sampleRate": sample_rate,
        "bitDepth": bit_depth,
        "channels": channels,
        "frames": frames,
        "durationSeconds": frames / sample_rate,
        "format": format_name,
    }


def current_commit() -> str:
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout.strip()


def run_renderer(
    renderer: Path,
    input_path: Path,
    output_path: Path,
    block_size: int,
    seed: int,
    water_enabled: bool,
    ice_enabled: bool,
    routing: str,
    parallel_balance: float,
    water_amount: float,
    ice_amount: float,
    input_gain_db: float,
    global_mix: float,
    output_gain_db: float,
) -> None:
    command = [
        str(renderer),
        "--input",
        str(input_path),
        "--output",
        str(output_path),
        "--block-size",
        str(block_size),
        "--seed",
        str(seed),
        "--water-enabled",
        str(int(water_enabled)),
        "--ice-enabled",
        str(int(ice_enabled)),
        "--routing",
        routing,
        "--parallel-balance",
        str(parallel_balance),
        "--water-amount",
        str(water_amount),
        "--ice-amount",
        str(ice_amount),
        "--input-gain-db",
        str(input_gain_db),
        "--global-mix",
        str(global_mix),
        "--output-gain-db",
        str(output_gain_db),
    ]
    subprocess.run(command, cwd=ROOT, check=True)


def create_manifest(
    input_path: Path,
    output_path: Path,
    block_size: int,
    seed: int,
    commit_sha: str,
    build_type: str,
    water_enabled: bool,
    ice_enabled: bool,
    routing: str,
    parallel_balance: float,
    water_amount: float,
    ice_amount: float,
    input_gain_db: float,
    global_mix: float,
    output_gain_db: float,
) -> dict[str, object]:
    input_metadata = wav_metadata(input_path)
    output_metadata = wav_metadata(output_path)
    return {
        "schemaVersion": 1,
        "purpose": "Deterministic offline render smoke for RENDER-001.",
        "input": {
            "path": stable_path(input_path),
            "sha256": file_sha256(input_path),
            **input_metadata,
        },
        "render": {
            "tool": "frazil_render",
            "commit": commit_sha,
            "buildType": build_type,
            "algorithm": "M1 AudioEngine pass-through",
            "seed": seed,
            "random": {
                "testSeed": seed,
                "appliedToEngine": False,
                "reason": "M1 pass-through AudioEngine currently contains no stochastic DSP.",
            },
            "blockSize": block_size,
            "parameters": {
                "waterEnabled": water_enabled,
                "iceEnabled": ice_enabled,
                "routing": routing,
                "parallelBalance": parallel_balance,
                "waterAmount": water_amount,
                "iceAmount": ice_amount,
                "inputGainDb": input_gain_db,
                "globalMix": global_mix,
                "outputGainDb": output_gain_db,
            },
        },
        "output": {
            "path": stable_path(output_path),
            "sha256": file_sha256(output_path),
            **output_metadata,
        },
    }


def run(options: argparse.Namespace) -> None:
    renderer = options.renderer.resolve()
    input_path = options.input.resolve()
    output_path = options.output.resolve()
    manifest_path = options.manifest.resolve()
    if not renderer.is_file():
        raise FileNotFoundError(f"renderer does not exist: {renderer}")
    if not input_path.is_file():
        raise FileNotFoundError(f"input WAV does not exist: {input_path}")
    commit_sha = options.commit or current_commit()

    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        first_output = temporary_root / "first.wav"
        repeat_output = temporary_root / "repeat.wav"
        renderer_arguments = {
            "block_size": options.block_size,
            "seed": options.seed,
            "water_enabled": options.water_enabled,
            "ice_enabled": options.ice_enabled,
            "routing": options.routing,
            "parallel_balance": options.parallel_balance,
            "water_amount": options.water_amount,
            "ice_amount": options.ice_amount,
            "input_gain_db": options.input_gain_db,
            "global_mix": options.global_mix,
            "output_gain_db": options.output_gain_db,
        }
        manifest_arguments = {
            **renderer_arguments,
            "commit_sha": commit_sha,
            "build_type": options.build_type,
        }
        run_renderer(renderer, input_path, first_output, **renderer_arguments)
        run_renderer(renderer, input_path, repeat_output, **renderer_arguments)

        first_manifest = create_manifest(input_path, first_output, **manifest_arguments)
        repeat_manifest = create_manifest(input_path, repeat_output, **manifest_arguments)
        if first_output.read_bytes() != repeat_output.read_bytes():
            raise RuntimeError("deterministic offline render differs between identical runs")
        if first_manifest["input"] != repeat_manifest["input"]:
            raise RuntimeError("input metadata changed between identical offline render runs")
        if first_manifest["render"] != repeat_manifest["render"]:
            raise RuntimeError("render configuration changed between identical offline render runs")
        repeat_output_metadata = repeat_manifest["output"].copy()
        first_output_metadata = first_manifest["output"].copy()
        repeat_output_metadata["path"] = first_output_metadata["path"]
        if first_output_metadata != repeat_output_metadata:
            raise RuntimeError("output manifest changed between identical offline render runs")

        output_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(first_output, output_path)

    manifest = create_manifest(input_path, output_path, **manifest_arguments)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"RENDER-001 offline render smoke: PASS ({stable_path(output_path)})")
    print(f"Wrote render manifest to {stable_path(manifest_path)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--block-size", type=int, default=128)
    parser.add_argument("--seed", type=int, default=DEFAULT_SEED)
    parser.add_argument("--commit", type=str, default=None)
    parser.add_argument("--build-type", type=str, default=DEFAULT_BUILD_TYPE)
    parser.add_argument("--water-enabled", type=parse_bool, default=True)
    parser.add_argument("--ice-enabled", type=parse_bool, default=True)
    parser.add_argument("--routing", choices=ROUTING_MODES, default="parallel")
    parser.add_argument("--parallel-balance", type=float, default=0.5)
    parser.add_argument("--water-amount", type=float, default=1.0)
    parser.add_argument("--ice-amount", type=float, default=1.0)
    parser.add_argument("--input-gain-db", type=float, default=0.0)
    parser.add_argument("--global-mix", type=float, default=1.0)
    parser.add_argument("--output-gain-db", type=float, default=0.0)
    options = parser.parse_args()
    if not 1 <= options.block_size <= 8192:
        parser.error("--block-size must be between 1 and 8192")
    if not 0 <= options.seed <= 0xFFFFFFFF:
        parser.error("--seed must be between 0 and 4294967295")
    if not 0.0 <= options.parallel_balance <= 1.0:
        parser.error("--parallel-balance must be between 0 and 1")
    if not 0.0 <= options.water_amount <= 1.0:
        parser.error("--water-amount must be between 0 and 1")
    if not 0.0 <= options.ice_amount <= 1.0:
        parser.error("--ice-amount must be between 0 and 1")
    if not -24.0 <= options.input_gain_db <= 24.0:
        parser.error("--input-gain-db must be between -24 and 24")
    if not 0.0 <= options.global_mix <= 1.0:
        parser.error("--global-mix must be between 0 and 1")
    if not -24.0 <= options.output_gain_db <= 24.0:
        parser.error("--output-gain-db must be between -24 and 24")
    try:
        run(options)
    except (
        FileNotFoundError,
        RuntimeError,
        ValueError,
        struct.error,
        subprocess.CalledProcessError,
    ) as error:
        parser.exit(1, f"RENDER-001 offline render smoke: FAIL: {error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
