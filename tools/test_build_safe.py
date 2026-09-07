#!/usr/bin/env python3
"""Regression tests for the bounded CMake build wrapper."""

from __future__ import annotations

import sys
import tempfile
from pathlib import Path
from unittest.mock import patch

import build_safe
from build_safe import (
    MAX_BUILD_JOBS,
    build_command,
    required_memory_bytes,
    tail_log,
    validate_environment,
    validate_job_count,
    validate_memory,
)


def expect_refusal(jobs: int) -> None:
    try:
        validate_job_count(jobs)
    except ValueError:
        return
    raise AssertionError(f"jobs={jobs} should have been refused")


def expect_environment_refusal(value: str) -> None:
    try:
        validate_environment({"CMAKE_BUILD_PARALLEL_LEVEL": value})
    except ValueError:
        return
    raise AssertionError(f"CMAKE_BUILD_PARALLEL_LEVEL={value!r} should have been refused")


def expect_memory_refusal(available_bytes: int | None, jobs: int) -> None:
    try:
        validate_memory(available_bytes, jobs)
    except ValueError:
        return
    raise AssertionError(
        f"available_bytes={available_bytes} should have been refused for jobs={jobs}"
    )


def assert_check_only_does_not_run_build() -> None:
    calls: list[tuple[list[str], Path]] = []

    def unexpected_build(command: list[str], log_path: Path) -> int:
        calls.append((command, log_path))
        raise AssertionError("check-only must not call the build runner")

    original_args = sys.argv
    original_runner = build_safe.run_build
    original_memory = build_safe.available_physical_memory
    try:
        build_safe.run_build = unexpected_build
        build_safe.available_physical_memory = (
            lambda: required_memory_bytes(6)
        )
        sys.argv = [
            "build_safe.py",
            "--preset",
            "windows-debug",
            "--jobs",
            "6",
            "--check-only",
        ]
        with patch.dict(
            build_safe.os.environ,
            {"CMAKE_BUILD_PARALLEL_LEVEL": "6"},
        ):
            assert build_safe.main() == 0
    finally:
        sys.argv = original_args
        build_safe.run_build = original_runner
        build_safe.available_physical_memory = original_memory

    assert calls == []


def assert_memory_failure_refuses_build() -> None:
    calls: list[tuple[list[str], Path]] = []

    def unexpected_build(command: list[str], log_path: Path) -> int:
        calls.append((command, log_path))
        raise AssertionError("unknown memory status must refuse before build")

    original_args = sys.argv
    original_runner = build_safe.run_build
    original_memory = build_safe.available_physical_memory
    try:
        build_safe.run_build = unexpected_build
        build_safe.available_physical_memory = lambda: None
        sys.argv = [
            "build_safe.py",
            "--preset",
            "windows-debug",
            "--jobs",
            "6",
        ]
        with patch.dict(
            build_safe.os.environ,
            {"CMAKE_BUILD_PARALLEL_LEVEL": "6"},
        ):
            assert build_safe.main() == 2
    finally:
        sys.argv = original_args
        build_safe.run_build = original_runner
        build_safe.available_physical_memory = original_memory

    assert calls == []


def assert_tail_is_bounded() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        log_path = Path(temporary_directory) / "build.log"
        log_path.write_text(
            "\n".join(f"line-{index}" for index in range(200)),
            encoding="utf-8",
        )
        lines = tail_log(log_path)
    assert len(lines) == 80
    assert lines[0] == "line-120"
    assert lines[-1] == "line-199"


def main() -> int:
    assert validate_job_count(1) == 1
    assert validate_job_count(6) == 6
    assert validate_job_count(8) == 8
    assert validate_job_count(MAX_BUILD_JOBS) == MAX_BUILD_JOBS
    expect_refusal(0)
    expect_refusal(MAX_BUILD_JOBS + 1)

    assert validate_environment({}) is None
    assert validate_environment({"CMAKE_BUILD_PARALLEL_LEVEL": "6"}) == 6
    assert validate_environment({"CMAKE_BUILD_PARALLEL_LEVEL": "8"}) == 8
    expect_environment_refusal("32")
    expect_environment_refusal("abc")

    required_memory = required_memory_bytes(6)
    expect_memory_refusal(required_memory - 1, 6)
    validate_memory(required_memory, 6)
    expect_memory_refusal(None, 6)

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

    assert_check_only_does_not_run_build()
    assert_memory_failure_refuses_build()
    assert_tail_is_bounded()
    print("Build safety regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
