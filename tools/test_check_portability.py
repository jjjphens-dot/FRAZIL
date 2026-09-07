#!/usr/bin/env python3
"""Lightweight regression tests for the repository portability scanner."""

from __future__ import annotations

from pathlib import Path

from check_portability import classify, scan_text


def windows_backslash_path() -> str:
    separator = chr(92)
    return "C:" + separator + "Users" + separator + "Alice" + separator + "project"


def windows_slash_path() -> str:
    return "D" + ":/local/tool"


def linux_user_path() -> str:
    return "/" + "home" + "/alice/project"


def macos_user_path() -> str:
    return "/" + "Users" + "/alice/project"


def unc_path() -> str:
    separator = chr(92)
    return separator * 2 + "server" + separator + "share"


def assert_category(line: str, expected: str) -> None:
    categories = [category for category, _ in classify(line)]
    assert any(category.startswith(expected) for category in categories), (
        f"expected {expected!r} in {categories!r} for {line!r}"
    )


def main() -> int:
    bad_cases = (
        (windows_backslash_path(), "Windows absolute path"),
        (windows_slash_path(), "Windows absolute path"),
        (linux_user_path(), "Linux user path"),
        (macos_user_path(), "macOS user path"),
        (unc_path(), "UNC path"),
        (
            "](" + windows_slash_path() + "/file.md)",
            "Windows absolute path / absolute Markdown link",
        ),
    )
    for line, category in bad_cases:
        assert_category(line, category)

    good_cases = (
        "$" + "{sourceDir}/tools/bin",
        "$" + "{workspaceFolder}",
        "<repo-root>/build",
        "$env{VCToolsInstallDir}",
        "tools/bin/pluginval.exe",
    )
    for line in good_cases:
        assert not classify(line), f"unexpected finding for {line!r}"

    marker = "PORTABILITY_" + "ALLOW"
    bypass_attempt = windows_backslash_path() + " " + marker + " reference evidence"
    findings = scan_text(Path("tools/example-runtime.txt"), bypass_attempt)
    assert findings, "absolute path with a marker must remain rejected"

    print("Scanner regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
