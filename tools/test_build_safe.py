#!/usr/bin/env python3
"""Regression tests for the bounded CMake build wrapper."""

from __future__ import annotations

from build_safe import MAX_BUILD_JOBS, build_command, validate_job_count


def expect_refusal(jobs: int) -> None:
    try:
        validate_job_count(jobs)
    except ValueError:
        return
    raise AssertionError(f"jobs={jobs} should have been refused")


def main() -> int:
    assert validate_job_count(1) == 1
    assert validate_job_count(MAX_BUILD_JOBS) == MAX_BUILD_JOBS
    expect_refusal(0)
    expect_refusal(MAX_BUILD_JOBS + 1)

    command = build_command("windows-debug", MAX_BUILD_JOBS)
    assert command == [
        "cmake",
        "--build",
        "--preset",
        "windows-debug",
        "--parallel",
        str(MAX_BUILD_JOBS),
    ]
    assert command[-1] != ""
    print("Build safety regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
