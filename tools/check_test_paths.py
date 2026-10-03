#!/usr/bin/env python3
"""Check configured CTest selectors against orthogonal module/tier labels.

Run after configure. This only queries registration metadata; it never executes
test bodies, changes their environment or turns an empty module into a pass.
"""

from __future__ import annotations

import argparse
import json
import re
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
PERFORMANCE = {
    "core": "frazil_product_performance",
    "water-common": "frazil_water_legacy_performance",
    "water-a1": "frazil_water_a1_performance",
    "water-b1": "frazil_water_b1_performance",
    "water-b2": "frazil_water_droplet_b2_performance",
    "water-d1": "frazil_water_d1_performance",
    "water-preview": "frazil_water_preview_performance",
}


def validate_performance(tests: dict[str, set[str]]) -> list[str]:
    findings = []
    modules = set().union(*tests.values()) if tests else set()
    for module, name in PERFORMANCE.items():
        if module in modules and not {module, "slow", "performance"}.issubset(tests.get(name, set())):
            findings.append(f"Full missing canonical performance execution: {name}")
    return findings


def validate_build_closure(inventory: dict, graph: str, build: Path) -> list[str]:
    """Check executable arguments (including Python's native helpers) against Ninja's graph."""
    built = {label.replace("\\", "/").lower() for label in re.findall(r'\[label="([^"]+)"', graph)}
    findings = []
    labels = inventory_tests(inventory)
    if labels and all("fast" in value for value in labels.values()):
        forbidden = ("performance", "source_probe", "latency_native", "research_cases")
        for executable in sorted(built):
            if executable.endswith(".exe") and any(word in executable for word in forbidden):
                findings.append(f"Fast build contains slow helper: {executable}")
            if set(labels) == {"frazil_smoke"} and executable.endswith(".exe") and "frazil_water" in executable:
                findings.append(f"Smoke build contains Water helper: {executable}")
    for test in inventory["tests"]:
        for argument in test.get("command", []):
            path = Path(argument)
            if path.suffix.lower() == ".exe" and path.is_absolute() and path.is_relative_to(build):
                relative = path.relative_to(build).as_posix().lower()
                if relative not in built:
                    findings.append(f"{test['name']}: helper absent from build closure: {relative}")
    return findings


def check_build(preset: str, inventory: dict) -> list[str]:
    presets = json.loads((ROOT / "CMakePresets.json").read_text(encoding="utf-8"))
    builds = {p["name"]: p for p in presets["buildPresets"]}
    resolved = dict(builds[preset])
    parent = resolved.get("inherits")
    while parent:
        inherited = builds[parent]
        resolved = {**inherited, **resolved}
        parent = inherited.get("inherits")
    build = ROOT / "build" / resolved["configurePreset"]
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    ninja = re.search(r"^CMAKE_MAKE_PROGRAM:[^=]+=(.+)$", cache, re.M).group(1).strip()
    graph = subprocess.check_output([ninja, "-C", str(build), "-t", "graph", *resolved["targets"]],
                                    text=True, encoding="utf-8")
    return validate_build_closure(inventory, graph, build)


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
    parser.add_argument("--preset", default="windows-debug", choices=("windows-debug", "ci-windows-debug"))
    parser.add_argument("--build-closure", action="store_true", help="Also inspect read-only Ninja target graphs")
    args = parser.parse_args()
    try:
        full_inventory = query(args.preset + "-full")
        full = inventory_tests(full_inventory)
        findings = validate_inventory(full)
        findings += validate_performance(full)
        if args.build_closure:
            findings += check_build(args.preset + "-full", full_inventory)
        smoke = set(inventory_tests(query(args.preset + "-smoke")))
        if smoke != {"frazil_smoke"}:
            findings.append(f"Smoke build/test path contains unrelated tests: {sorted(smoke)}")
        if args.build_closure:
            findings += check_build(args.preset + "-smoke", query(args.preset + "-smoke"))
        for module in (None, "core", *MODULES):
            suffix = "fast" if module is None else "preview" if module == "water-preview" else module
            preset = args.preset + "-" + suffix
            selection = query(preset)
            selected = set(inventory_tests(selection))
            if args.build_closure and selected:
                findings += check_build(preset, selection)
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
