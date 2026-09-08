#!/usr/bin/env python3
"""Run a bounded CMake build without flooding the terminal or exhausting memory."""

from __future__ import annotations

import argparse
import ctypes
import os
import subprocess
import sys
from collections.abc import Mapping
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAX_BUILD_JOBS = 8
DEFAULT_BUILD_JOBS = 6
BASE_AVAILABLE_MEMORY_BYTES = 2 * 1024 * 1024 * 1024
MEMORY_PER_JOB_BYTES = 512 * 1024 * 1024
SUPPORTED_PRESETS = (
    "windows-debug",
    "windows-release",
    "windows-asan",
    "ci-windows-debug",
)


class MemoryStatusEx(ctypes.Structure):
    _fields_ = (
        ("dwLength", ctypes.c_ulong),
        ("dwMemoryLoad", ctypes.c_ulong),
        ("ullTotalPhys", ctypes.c_ulonglong),
        ("ullAvailPhys", ctypes.c_ulonglong),
        ("ullTotalPageFile", ctypes.c_ulonglong),
        ("ullAvailPageFile", ctypes.c_ulonglong),
        ("ullTotalVirtual", ctypes.c_ulonglong),
        ("ullAvailVirtual", ctypes.c_ulonglong),
        ("ullAvailExtendedVirtual", ctypes.c_ulonglong),
    )


def validate_job_count(jobs: int) -> int:
    if jobs < 1 or jobs > MAX_BUILD_JOBS:
        raise ValueError(
            f"refusing jobs={jobs}; local builds are limited to "
            f"{MAX_BUILD_JOBS} jobs"
        )
    return jobs


def validate_environment(
    environment: Mapping[str, str] | None = None,
) -> int | None:
    values = os.environ if environment is None else environment
    value = values.get("CMAKE_BUILD_PARALLEL_LEVEL")
    if value is None:
        return None
    try:
        environment_jobs = int(value)
    except (TypeError, ValueError) as error:
        raise ValueError("CMAKE_BUILD_PARALLEL_LEVEL must be an integer") from error
    return validate_job_count(environment_jobs)


def required_memory_bytes(jobs: int) -> int:
    validate_job_count(jobs)
    return max(BASE_AVAILABLE_MEMORY_BYTES, jobs * MEMORY_PER_JOB_BYTES)


def validate_memory(available_bytes: int | None, jobs: int) -> None:
    required_memory = required_memory_bytes(jobs)
    if available_bytes is None:
        raise ValueError(
            "physical memory availability could not be determined; refusing build"
        )
    if available_bytes >= required_memory:
        return
    available_gib = available_bytes / (1024**3)
    raise ValueError(
        f"only {available_gib:.2f} GiB physical memory is available; "
        f"at least {required_memory / (1024**3):.0f} GiB is required for {jobs} jobs."
    )


def available_physical_memory() -> int | None:
    if os.name != "nt":
        return None
    status = MemoryStatusEx()
    status.dwLength = ctypes.sizeof(MemoryStatusEx)
    if not ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
        return None
    return int(status.ullAvailPhys)


def build_command(preset: str, jobs: int) -> list[str]:
    return [
        "cmake",
        "--build",
        "--preset",
        preset,
        "--parallel",
        str(validate_job_count(jobs)),
    ]


def tail_log(log_path: Path, line_count: int = 80) -> list[str]:
    try:
        lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError as error:
        return [f"Unable to read build log: {error}"]
    return lines[-line_count:]


def run_build(command: list[str], log_path: Path) -> int:
    with log_path.open("w", encoding="utf-8", errors="replace") as log_file:
        result = subprocess.run(
            command,
            cwd=ROOT,
            stdout=log_file,
            stderr=subprocess.STDOUT,
            check=False,
        )
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", choices=SUPPORTED_PRESETS, required=True)
    parser.add_argument("--jobs", type=int, default=DEFAULT_BUILD_JOBS)
    parser.add_argument("--check-only", action="store_true")
    args = parser.parse_args()

    try:
        jobs = validate_job_count(args.jobs)
        validate_environment()
        validate_memory(available_physical_memory(), jobs)
    except ValueError as error:
        print(f"Build safety check: REFUSED: {error}", file=sys.stderr)
        return 2

    if args.check_only:
        print("Build safety preflight: PASS")
        return 0

    log_path = ROOT / "build" / "safe-build" / f"{args.preset}.log"
    log_path.parent.mkdir(parents=True, exist_ok=True)
    command = build_command(args.preset, jobs)
    print(
        f"Build safety check: PASS (preset={args.preset}, jobs={jobs}, "
        f"log={log_path.relative_to(ROOT)})"
    )

    try:
        result = run_build(command, log_path)
    except OSError as error:
        print(f"Build safety check: FAILED to start build: {error}", file=sys.stderr)
        return 1

    if result != 0:
        print(f"Build failed with exit code {result}. Last log lines:")
        for line in tail_log(log_path):
            print(line)
        return result if 0 < result < 256 else 1

    print("Build completed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
