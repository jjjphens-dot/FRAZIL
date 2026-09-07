#!/usr/bin/env python3
"""Fail when tracked repository text contains unapproved machine-specific paths."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED_PARTS = {
    ".git",
    ".venv",
    "build",
    "external",
    "generated",
    "rendered",
}
EXCLUDED_PREFIXES = (
    "tools/bin/",
    "tools/downloads/",
    "testdata/rendered/",
)
BINARY_SUFFIXES = {
    ".7z",
    ".a",
    ".bin",
    ".dll",
    ".dylib",
    ".exe",
    ".lib",
    ".o",
    ".obj",
    ".pdb",
    ".so",
    ".wav",
    ".zip",
}

backslash = chr(92)
escaped_backslash = re.escape(backslash)
whitespace = backslash + "s"
windows_absolute = re.compile(
    "(?<![A-Za-z0-9])[A-Za-z]:" + "[" + escaped_backslash + "/]"
)
path_component = "[^/" + whitespace + escaped_backslash + "]"
linux_user_path = re.compile("/home/" + path_component + "+/")
macos_user_path = re.compile("/Users/" + path_component + "+/")
unc_path = re.compile(
    escaped_backslash * 2
    + "[^"
    + whitespace
    + escaped_backslash
    + "/]+"
    + escaped_backslash
)
markdown_link = re.compile(re.escape("](") + whitespace + "*")


def tracked_paths() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        capture_output=True,
    )
    return [Path(item) for item in result.stdout.decode("utf-8").split(chr(0)) if item]


def is_scannable(path: Path) -> bool:
    normalized = path.as_posix()
    if normalized.startswith(EXCLUDED_PREFIXES):
        return False
    if any(part in EXCLUDED_PARTS for part in path.parts):
        return False
    if path.suffix.lower() in BINARY_SUFFIXES:
        return False
    return True


def classify(line: str) -> list[tuple[str, int]]:
    patterns = (
        ("Windows absolute path", windows_absolute),
        ("Linux user path", linux_user_path),
        ("macOS user path", macos_user_path),
        ("UNC path", unc_path),
    )
    matches: list[tuple[str, int]] = []
    for label, pattern in patterns:
        match = pattern.search(line)
        if match:
            suffix = " / absolute Markdown link" if markdown_link.search(line) else ""
            matches.append((label + suffix, match.start()))
    return matches


def scan_text(relative_path: Path, text: str) -> list[tuple[str, int, str, str]]:
    findings: list[tuple[str, int, str, str]] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        for category, _ in classify(line):
            findings.append((relative_path.as_posix(), line_number, category, line.strip()))
    return findings


def main() -> int:
    findings: list[tuple[str, int, str, str]] = []
    for relative_path in tracked_paths():
        if not is_scannable(relative_path):
            continue
        path = ROOT / relative_path
        try:
            raw = path.read_bytes()
        except OSError as error:
            print(f"ERROR: cannot read {relative_path}: {error}", file=sys.stderr)
            return 2
        if bytes([0]) in raw:
            continue
        text = raw.decode("utf-8", errors="replace")
        findings.extend(scan_text(relative_path, text))

    if findings:
        print("Repository portability scan: FAIL")
        for path, line_number, category, line in findings:
            print(f"{path}:{line_number}: {category}: {line}")
        print(
            "Use repo-relative paths, environment/tool discovery, or ignored local "
            "configuration. Tracked reference evidence must not contain raw machine paths."
        )
        return 1

    print("Repository portability scan: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
