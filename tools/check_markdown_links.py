#!/usr/bin/env python3
"""Fail when tracked Markdown links point to missing repository files."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED_PARTS = {".git", ".venv", "build", "external"}
MARKDOWN_LINK = re.compile(
    r"\[[^\]]*\]\(\s*(?:<([^>\r\n]+)>|([^\s)]+))"
)


def tracked_markdown_paths(root: Path = ROOT) -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=root,
        check=True,
        capture_output=True,
    )
    paths = []
    for item in result.stdout.decode("utf-8").split(chr(0)):
        if not item:
            continue
        path = Path(item)
        if path.suffix.lower() not in {".md", ".markdown"}:
            continue
        if any(part in EXCLUDED_PARTS for part in path.parts):
            continue
        paths.append(path)
    return paths


def is_external_link(target: str) -> bool:
    parsed = urlsplit(target)
    return bool(parsed.scheme or parsed.netloc) or target.startswith("//")


def scan_markdown_text(
    root: Path, relative_path: Path, text: str
) -> list[tuple[str, int, str, str]]:
    findings: list[tuple[str, int, str, str]] = []
    source_path = root / relative_path
    root_path = root.resolve()
    in_fence = False
    in_math_block = False
    for line_number, line in enumerate(text.splitlines(), start=1):
        stripped = line.strip()
        if stripped.startswith("```") or stripped.startswith("~~~"):
            in_fence = not in_fence
            continue
        if stripped == r"\[":
            in_math_block = True
            continue
        if stripped == r"\]":
            in_math_block = False
            continue
        if in_fence or in_math_block:
            continue
        for match in MARKDOWN_LINK.finditer(line):
            target = match.group(1) or match.group(2) or ""
            target = unquote(target)
            target_without_fragment = target.split("#", 1)[0]
            if not target_without_fragment or is_external_link(target_without_fragment):
                continue
            candidate = (source_path.parent / target_without_fragment).resolve()
            try:
                candidate.relative_to(root_path)
            except ValueError:
                findings.append(
                    (
                        relative_path.as_posix(),
                        line_number,
                        target,
                        "link resolves outside repository",
                    )
                )
                continue
            if not candidate.exists():
                findings.append(
                    (
                        relative_path.as_posix(),
                        line_number,
                        target,
                        "target does not exist",
                    )
                )
    return findings


def scan_markdown_file(root: Path, relative_path: Path) -> list[tuple[str, int, str, str]]:
    path = root / relative_path
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as error:
        return [(relative_path.as_posix(), 0, "", f"cannot read file: {error}")]
    return scan_markdown_text(root, relative_path, text)


def main() -> int:
    findings: list[tuple[str, int, str, str]] = []
    for relative_path in tracked_markdown_paths():
        findings.extend(scan_markdown_file(ROOT, relative_path))

    if findings:
        print("Markdown internal-link scan: FAIL")
        for path, line_number, target, reason in findings:
            print(f"{path}:{line_number}: {reason}: {target}")
        return 1

    print("Markdown internal-link scan: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())