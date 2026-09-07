#!/usr/bin/env python3
"""Regression tests for the lightweight Markdown internal-link checker."""

from __future__ import annotations

import tempfile
from pathlib import Path

from check_markdown_links import scan_markdown_file


def main() -> int:
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = Path(temporary_directory)
        (root / "docs").mkdir()
        (root / "docs" / "guide.md").write_text("# Guide\n", encoding="utf-8")
        (root / "README.md").write_text(
            "[valid](docs/guide.md#intro)\n"
            "[external](https://example.com/missing.md)\n"
            "[mail](mailto:test@example.com)\n"
            "\\[\n"
            "f[n]=h[n](1+mu)\n"
            "\\]\n"
            "```\n"
            "[code](missing)\n"
            "```\n",
            encoding="utf-8",
        )
        assert scan_markdown_file(root, Path("README.md")) == []

        (root / "README.md").write_text(
            "[missing](docs/missing.md)\n"
            "[outside](../outside.md)\n",
            encoding="utf-8",
        )
        findings = scan_markdown_file(root, Path("README.md"))
        assert len(findings) == 2
        assert findings[0][3] == "target does not exist"
        assert findings[1][3] == "link resolves outside repository"

    print("Markdown internal-link regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())