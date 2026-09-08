#!/usr/bin/env python3
"""Regression checks for the frazil_render command-line contract."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


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

    print(f"frazil_render CLI validation: PASS ({len(invalid_cases)} invalid cases plus --help)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
