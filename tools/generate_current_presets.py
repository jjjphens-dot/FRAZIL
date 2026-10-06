"""Generate CURRENT module presets from the single module registry; --check never writes."""
import argparse
import json
from pathlib import Path

from current_modules import ROOT, load_modules

BASES = ("windows-debug", "windows-release", "windows-asan", "ci-windows-debug")


def generate(data, modules=None):
    modules = load_modules() if modules is None else modules
    # Generated entries carry a vendor marker; hand-maintained historical presets stay intact.
    for section in ("configurePresets", "buildPresets", "testPresets"):
        data[section] = [p for p in data[section] if "frazil/current" not in p.get("vendor", {})]
    configs = {p["name"]: p for p in data["configurePresets"]}
    for base in BASES:
        configs[base]["cacheVariables"].update(FRAZIL_TEST_PROFILE="CURRENT",
            FRAZIL_TEST_PURPOSE="memory-safety" if base == "windows-asan" else "correctness",
            FRAZIL_BUILD_WATER_EXPERIMENT="ON", FRAZIL_BUILD_WATER_PREVIEW="OFF")
        for suffix, profile, water, preview in (("all", "ALL", "ON", "ON"),
                                                ("historical", "HISTORICAL", "ON", "ON"),
                                                ("host", "CORE", "OFF", "OFF")):
            data["configurePresets"].append(dict(name=f"{base}-{suffix}", inherits=base,
                cacheVariables=dict(FRAZIL_TEST_PROFILE=profile, FRAZIL_TEST_PURPOSE="correctness",
                                    FRAZIL_BUILD_WATER_EXPERIMENT=water, FRAZIL_BUILD_WATER_PREVIEW=preview),
                vendor={"frazil/current": {}}))
        for preset in data["buildPresets"]:
            if preset["name"] == base:
                preset["targets"] = ["FRAZIL_All"]
            elif preset["name"].startswith(base + "-"):
                preset.pop("inherits", None)
                preset["configurePreset"] = base + ("-host" if preset["name"] == base + "-core" else "-all")
                preset["jobs"] = 6
        for preset in data["testPresets"]:
            if preset["name"] == base:
                preset["filter"] = {"include": {"label": "^current-.*-correctness$"}}
            elif preset["name"].startswith(base + "-"):
                # Keep old selectors, but they require an explicit archive/core configure tree.
                preset.pop("inherits", None)
                preset.setdefault("output", {"outputOnFailure": True})
                preset["configurePreset"] = base + ("-host" if preset["name"] == base + "-core" else "-all")

        def add(name, targets, label=None, configure=base):
            data["buildPresets"].append(dict(name=name, configurePreset=configure, jobs=6,
                                             targets=targets, vendor={"frazil/current": {}}))
            if label is not None:
                data["testPresets"].append(dict(name=name, configurePreset=configure,
                    output={"outputOnFailure": True}, execution={"noTestsAction": "error", "jobs": 1},
                    filter={"include": {"label": label}}, vendor={"frazil/current": {}}))
        add(base + "-build", ["FRAZIL_All"])
        add(base + "-current-tests", ["frazil_test_current"], "^current-.*-correctness$")
        for module in modules:
            add(f"{base}-{module}-test", [f"frazil_test_{module}_current"], f"^current-{module}-correctness$")
        add(base + "-historical-tests", ["frazil_full_tests"], "historical|core", base + "-historical")

    data["configurePresets"].append(dict(name="windows-release-performance", inherits="windows-release",
        cacheVariables={"FRAZIL_TEST_PURPOSE": "performance"}, vendor={"frazil/current": {}}))
    data["buildPresets"].extend([
        dict(name="windows-asan-current-memory", configurePreset="windows-asan", jobs=6,
             targets=["frazil_test_current"], vendor={"frazil/current": {}}),
        dict(name="windows-release-current-performance", configurePreset="windows-release-performance", jobs=6,
             targets=["frazil_test_current_performance"], vendor={"frazil/current": {}})])
    for name, config, label in (("windows-asan-current-memory", "windows-asan", "^current-.*-memory$"),
                               ("windows-release-current-performance", "windows-release-performance", "^current-.*-performance$")):
        data["testPresets"].append(dict(name=name, configurePreset=config,
            output={"outputOnFailure": True}, execution={"noTestsAction": "error", "jobs": 1},
            filter={"include": {"label": label}}, vendor={"frazil/current": {}}))
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    path = ROOT / "CMakePresets.json"
    original = path.read_text(encoding="utf-8")
    expected = json.dumps(generate(json.loads(original)), indent=2) + "\n"
    if args.check:
        if original != expected:
            raise SystemExit("CURRENT presets are stale; run tools/generate_current_presets.py")
        print("CURRENT generated presets: PASS")
    else:
        path.write_text(expected, encoding="utf-8")


if __name__ == "__main__":
    main()
