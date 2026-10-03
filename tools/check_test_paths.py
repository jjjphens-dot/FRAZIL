#!/usr/bin/env python3
"""Check configured CTest selectors against orthogonal module/tier labels.

Run after configure. This only queries registration metadata; it never executes
test bodies, changes their environment or turns an empty module into a pass.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
MODULES = (
    "water-common", "water-a1", "water-b1", "water-b2", "water-d1",
    "water-protect", "water-preview",
)
FORBIDDEN_FAST = frozenset({"slow", "research", "listening", "performance", "native", "evidence"})
KINDS = frozenset({
    "unit", "integration", "property", "cli", "render", "native", "smoke",
    "research", "listening", "evidence", "performance",
})


def inventory_tests(inventory: dict) -> dict[str, set[str]]:
    tests = {}
    for test in inventory["tests"]:
        name = test["name"]
        if name in tests:
            raise ValueError(f"duplicate CTest registration: {name}")
        properties = {item["name"]: item["value"] for item in test["properties"]}
        tests[name] = set(properties.get("LABELS", []))
    return tests


def validate_inventory(tests: dict[str, set[str]]) -> list[str]:
    findings = []
    if not tests:
        findings.append("Full inventory is empty")
    for name, labels in tests.items():
        if not labels.intersection({"core", *MODULES}):
            findings.append(f"{name}: missing module")
        if len(labels.intersection({"fast", "slow"})) != 1:
            findings.append(f"{name}: expected exactly one tier")
        if not labels.intersection(KINDS):
            findings.append(f"{name}: missing kind")
        if "fast" in labels and labels.intersection(FORBIDDEN_FAST):
            findings.append(f"{name}: Fast contains slow/research workload")
        expected = {f"fast-{module}" for module in MODULES
                    if module in labels and "fast" in labels}
        actual = {label for label in labels if label.startswith("fast-water-")}
        if actual != expected:
            findings.append(f"{name}: derived module-fast labels disagree with module/tier")
    return findings


def validate_selection(
    tests: dict[str, set[str]], selected: set[str], module: str | None = None,
) -> list[str]:
    expected = {name for name, labels in tests.items()
                if "fast" in labels and (module is None or module in labels)}
    findings = []
    if selected != expected:
        findings.append(f"{module or 'fast'}: missing={sorted(expected - selected)}, "
                        f"unexpected={sorted(selected - expected)}")
    if not expected and (module is None or any(module in labels for labels in tests.values())):
        findings.append(f"{module or 'fast'}: configured domain has no Fast coverage")
    return findings


def query(preset: str, *filters: str) -> dict:
    result = subprocess.run(
        ["ctest", "--preset", preset, *filters, "--show-only=json-v1"],
        cwd=ROOT, text=True, encoding="utf-8", capture_output=True, check=True,
    )
    return json.loads(result.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", default="windows-debug", choices=("windows-debug",))
    args = parser.parse_args()
    try:
        full = inventory_tests(query(args.preset + "-full"))
        findings = validate_inventory(full)
        for module in (None, "core", *MODULES):
            suffix = "fast" if module is None else "preview" if module == "water-preview" else module
            preset = args.preset + "-" + suffix
            selected = set(inventory_tests(query(preset)))
            findings += validate_selection(full, selected, module)
            filters = ["-L", "^fast$"]
            if module:
                filters += ["-L", f"^{module}$"]
            intersection = set(inventory_tests(query(args.preset + "-full", *filters)))
            if selected != intersection:
                findings.append(f"{preset}: selection differs from CLI label intersection")
            state = "disabled" if module and not any(module in ls for ls in full.values()) else "enabled"
            print(f"{preset}: {len(selected)} ({state}) {', '.join(sorted(selected))}")
    except (OSError, subprocess.CalledProcessError, ValueError, KeyError) as error:
        print(f"Test path check: ERROR: {error}", file=sys.stderr)
        return 1
    for finding in findings:
        print(finding, file=sys.stderr)
    print(f"Test path check: {'FAIL' if findings else 'PASS'}")
    return int(bool(findings))


if __name__ == "__main__":
    raise SystemExit(main())
