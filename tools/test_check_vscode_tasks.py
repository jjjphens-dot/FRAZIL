#!/usr/bin/env python3
"""Regression tests for VS Code task reference checking."""

from __future__ import annotations

import json
import tempfile
from pathlib import Path

from check_vscode_tasks import check_vscode_tasks


def main() -> int:
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = Path(temporary_directory)
        vscode = root / ".vscode"
        vscode.mkdir()
        (vscode / "tasks.json").write_text(
            json.dumps({"tasks": [{"label": "build"}]}), encoding="utf-8"
        )
        (vscode / "launch.json").write_text(
            json.dumps(
                {
                    "configurations": [
                        {"name": "valid", "preLaunchTask": "build"},
                    ]
                }
            ),
            encoding="utf-8",
        )
        assert check_vscode_tasks(root) == []

        (vscode / "launch.json").write_text(
            json.dumps(
                {
                    "configurations": [
                        {"name": "broken", "preLaunchTask": "missing"},
                    ]
                }
            ),
            encoding="utf-8",
        )
        findings = check_vscode_tasks(root)
        assert len(findings) == 1
        assert "missing" in findings[0]

    print("VS Code task reference regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())