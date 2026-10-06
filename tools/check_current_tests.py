"""Read-only CURRENT registration and production/test build-closure validation."""
import argparse
import json
from pathlib import Path
import re
import subprocess

from current_modules import ROOT, load_modules, selection_label
from check_test_paths import validate_build_closure


def validate_inventory(inventory, modules, purpose="correctness"):
    suffixes = ("_performance",) if purpose == "performance" else ("", "_cli")
    expected = {f"frazil_current_{m}{s}" for m in modules for s in suffixes}
    names = [test["name"] for test in inventory["tests"]]
    if set(names) != expected or len(names) != len(expected):
        raise ValueError(f"CURRENT inventory differs: expected={sorted(expected)}, got={names}")
    for test in inventory["tests"]:
        props = {p["name"]: p["value"] for p in test["properties"]}
        labels = set(props.get("LABELS", []))
        if "current" not in labels or labels.intersection({"historical", "research", "listening", "evidence"}):
            raise ValueError(f"non-current workload: {test['name']}")
        if purpose != "performance" and "performance" in labels:
            raise ValueError("timing loop in CURRENT correctness/memory")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", default="windows-debug")
    args = parser.parse_args()
    modules = load_modules()
    build = ROOT / "build" / args.preset
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    def cache_value(name):
        return re.search(r"^" + name + r":[^=]+=(.+)$", cache, re.M).group(1).strip()
    if cache_value("FRAZIL_TEST_PROFILE") != "CURRENT":
        raise SystemExit("configure CURRENT before checking its inventory")
    purpose = cache_value("FRAZIL_TEST_PURPOSE")
    # Query the unfiltered configured tree, so default registration leaks cannot hide behind presets.
    inventory = json.loads(subprocess.check_output(["ctest", "--test-dir", str(build), "--show-only=json-v1"], text=True))
    validate_inventory(inventory, modules, purpose)
    ninja = cache_value("CMAKE_MAKE_PROGRAM")
    def graph(*targets):
        return subprocess.check_output([ninja, "-C", str(build), "-t", "graph", *targets], text=True)
    production = graph("FRAZIL_All")
    if re.search(r"frazil_(?:water|test|smoke|render|performance)", production):
        raise SystemExit("production aggregate contains test/research dependencies")
    target = "frazil_test_current_performance" if purpose == "performance" else "frazil_test_current"
    findings = validate_build_closure(inventory, graph(target), build)
    if findings:
        raise SystemExit("\n".join(findings))
    if purpose != "performance" and re.search(r"performance.*\.exe|source_probe.*\.exe|latency_native.*\.exe", graph(target)):
        raise SystemExit("CURRENT build includes historical study or timing executable")
    for module in modules:
        selected = json.loads(subprocess.check_output(["ctest", "--test-dir", str(build), "-L",
            selection_label([module], purpose), "--show-only=json-v1"], text=True))
        validate_inventory(selected, [module], purpose)
    print(f"CURRENT {purpose}: {len(inventory['tests'])} registrations; module selectors and build closures PASS")


if __name__ == "__main__":
    main()
