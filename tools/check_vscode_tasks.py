#!/usr/bin/env python3
"""Validate VS Code launch preLaunchTask references against tasks.json labels."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def check_vscode_tasks(root: Path = ROOT) -> list[str]:
    vscode = root / ".vscode"
    try:
        tasks = json.loads((vscode / "tasks.json").read_text(encoding="utf-8"))
        launch = json.loads((vscode / "launch.json").read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"unable to read VS Code configuration: {error}"]

    labels = {
        task.get("label")
        for task in tasks.get("tasks", [])
        if isinstance(task, dict) and isinstance(task.get("label"), str)
    }
    findings = []
    for configuration in launch.get("configurations", []):
        if not isinstance(configuration, dict):
            continue
        task_name = configuration.get("preLaunchTask")
        if task_name is None:
            continue
        if not isinstance(task_name, str) or task_name not in labels:
            configuration_name = configuration.get("name", "<unnamed>")
            findings.append(
                f"{configuration_name}: preLaunchTask {task_name!r} is not defined"
            )
    return findings


def main() -> int:
    findings = check_vscode_tasks()
    if findings:
        print("VS Code task reference check: FAIL")
        for finding in findings:
            print(finding)
        return 1
    print("VS Code task reference check: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())