#!/usr/bin/env python3
"""Run a bounded CMake build without flooding the terminal or exhausting memory."""

from __future__ import annotations

import argparse
import ctypes
import os
import subprocess
import sys
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


def validate_environment() -> None:
    value = os.environ.get("CMAKE_BUILD_PARALLEL_LEVEL")
    if value is None:
        return
    try:
        environment_jobs = int(value)
    except ValueError as error:
        raise ValueError("CMAKE_BUILD_PARALLEL_LEVEL must be an integer") from error
    validate_job_count(environment_jobs)


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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", choices=SUPPORTED_PRESETS, required=True)
    parser.add_argument("--jobs", type=int, default=DEFAULT_BUILD_JOBS)
    parser.add_argument("--check-only", action="store_true")
    args = parser.parse_args()

    try:
        jobs = validate_job_count(args.jobs)
        validate_environment()
    except ValueError as error:
        print(f"Build safety check: REFUSED: {error}", file=sys.stderr)
        return 2

    required_memory = max(BASE_AVAILABLE_MEMORY_BYTES, jobs * MEMORY_PER_JOB_BYTES)
    available_memory = available_physical_memory()
    if (
        available_memory is not None
        and available_memory < required_memory
    ):
        available_gib = available_memory / (1024**3)
        print(
            "Build safety check: REFUSED: only "
            f"{available_gib:.2f} GiB physical memory is available; "
            f"at least {required_memory / (1024**3):.0f} GiB is required for {jobs} jobs.",
            file=sys.stderr,
        )
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
        with log_path.open("w", encoding="utf-8", errors="replace") as log_file:
            result = subprocess.run(
                command,
                cwd=ROOT,
                stdout=log_file,
                stderr=subprocess.STDOUT,
                check=False,
            )
    except OSError as error:
        print(f"Build safety check: FAILED to start build: {error}", file=sys.stderr)
        return 1

    if result.returncode != 0:
        print(f"Build failed with exit code {result.returncode}. Last log lines:")
        for line in tail_log(log_path):
            print(line)
        return result.returncode if 0 < result.returncode < 256 else 1

    print("Build completed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
