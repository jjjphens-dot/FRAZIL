#!/usr/bin/env python3
"""Regression checks for the frazil_render command-line contract."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run_case(renderer: Path, arguments: list[str], expected_error: str) -> None:
    result = subprocess.run(
        [str(renderer), "--input", "unused.wav", "--output", "unused.wav", *arguments],
        capture_output=True,
        text=True,
    )
    if result.returncode == 0:
        raise RuntimeError(f"expected failure for {' '.join(arguments)}")
    if expected_error not in result.stderr:
        raise RuntimeError(
            f"missing {expected_error!r} in stderr for {' '.join(arguments)}: "
            f"{result.stderr.strip()}"
        )


def check_non_default_manifest(renderer: Path) -> None:
    expected_parameters = {
        "waterEnabled": False,
        "iceEnabled": True,
        "routing": "ice-into-water",
        "parallelBalance": 0.23,
        "waterAmount": 0.37,
        "iceAmount": 0.81,
        "inputGainDb": -3.0,
        "globalMix": 0.64,
        "outputGainDb": 2.0,
    }
    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        output_path = temporary_root / "non-default.wav"
        manifest_path = temporary_root / "non-default.manifest.json"
        result = subprocess.run(
            [
                sys.executable,
                str(ROOT / "tools" / "render_testdata.py"),
                "--renderer",
                str(renderer),
                "--output",
                str(output_path),
                "--manifest",
                str(manifest_path),
                "--water-enabled",
                "0",
                "--ice-enabled",
                "1",
                "--routing",
                "ice-into-water",
                "--parallel-balance",
                "0.23",
                "--water-amount",
                "0.37",
                "--ice-amount",
                "0.81",
                "--input-gain-db",
                "-3.0",
                "--global-mix",
                "0.64",
                "--output-gain-db",
                "2.0",
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            raise RuntimeError(
                "non-default render configuration failed: "
                f"{result.stderr.strip()}"
            )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        actual_parameters = manifest["render"]["parameters"]
        if actual_parameters != expected_parameters:
            raise RuntimeError(
                f"non-default render configuration was not preserved: {actual_parameters}"
            )


def check_repeated_output_path(renderer: Path) -> None:
    input_path = ROOT / "testdata" / "input" / "impulse.wav"
    with tempfile.TemporaryDirectory() as temporary:
        output_path = Path(temporary) / "repeated.wav"
        command = [
            str(renderer),
            "--input",
            str(input_path),
            "--output",
            str(output_path),
            "--block-size",
            "128",
            "--seed",
            "20260908",
            "--water-enabled",
            "1",
            "--ice-enabled",
            "1",
            "--routing",
            "parallel",
            "--parallel-balance",
            "0.5",
            "--water-amount",
            "1.0",
            "--ice-amount",
            "1.0",
            "--input-gain-db",
            "0.0",
            "--global-mix",
            "1.0",
            "--output-gain-db",
            "0.0",
        ]
        first_result = subprocess.run(command, capture_output=True, text=True)
        if first_result.returncode != 0:
            raise RuntimeError(
                f"first render to shared output path failed: {first_result.stderr.strip()}"
            )
        first_bytes = output_path.read_bytes()
        second_result = subprocess.run(command, capture_output=True, text=True)
        if second_result.returncode != 0:
            raise RuntimeError(
                f"second render to shared output path failed: {second_result.stderr.strip()}"
            )
        if first_bytes != output_path.read_bytes():
            raise RuntimeError("same output path produced different bytes on repeated render")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    options = parser.parse_args()
    renderer = options.renderer.resolve()
    if not renderer.is_file():
        parser.error(f"renderer does not exist: {renderer}")

    help_result = subprocess.run([str(renderer), "--help"], capture_output=True, text=True)
    if help_result.returncode != 0 or "Usage:" not in help_result.stdout:
        raise RuntimeError("--help must print usage and return zero")

    invalid_cases = (
        (["--water-enabled", "2"], "--water-enabled must be 0 or 1"),
        (["--ice-enabled", "yes"], "--ice-enabled must be 0 or 1"),
        (["--routing", "invalid"], "--routing must be parallel"),
        (["--parallel-balance", "-0.1"], "--parallel-balance must be between 0 and 1"),
        (["--parallel-balance", "1.1"], "--parallel-balance must be between 0 and 1"),
        (["--water-amount", "-0.1"], "--water-amount must be between 0 and 1"),
        (["--water-amount", "1.1"], "--water-amount must be between 0 and 1"),
        (["--ice-amount", "-0.1"], "--ice-amount must be between 0 and 1"),
        (["--ice-amount", "1.1"], "--ice-amount must be between 0 and 1"),
    )
    for arguments, expected_error in invalid_cases:
        run_case(renderer, arguments, expected_error)
    check_non_default_manifest(renderer)
    check_repeated_output_path(renderer)

    print(
        "frazil_render CLI validation: PASS "
        f"({len(invalid_cases)} invalid cases, --help, non-default manifest, "
        "and repeated output path)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
