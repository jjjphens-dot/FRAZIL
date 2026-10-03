#!/usr/bin/env python3
"""Fixed direct/CTest diagnostic matrix for the two retained Python failures.

Each invocation runs once with the configured interpreter, test working directory
and CTest environment modifications. Logs supplement, never replace, Full evidence.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
TESTS = ("frazil_testdata_regeneration", "frazil_water_experiment_render_cli")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("diagnostic output must remain in repository build/")
    output.mkdir(parents=True, exist_ok=False)  # Never overwrite an earlier diagnostic.
    inventory = json.loads(subprocess.check_output(
        ["ctest", "--preset", args.preset, "--show-only=json-v1"], cwd=ROOT, text=True))
    selected = {t["name"]: t for t in inventory["tests"] if t["name"] in TESTS}
    if set(selected) != set(TESTS):
        raise ValueError("diagnostic requires both registered Python tests")
    results = []
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    interpreter = selected[TESTS[0]]["command"][0]
    version = subprocess.check_output([interpreter, "--version"], text=True).strip()
    for no_user_site in (False, True):
        variant = "no-user-site" if no_user_site else "configured"
        for name in TESTS:
            test = selected[name]
            properties = {p["name"]: p["value"] for p in test["properties"]}
            parent = dict(os.environ, PYTHONFAULTHANDLER="1")
            if no_user_site:
                parent["PYTHONNOUSERSITE"] = "1"
            direct = dict(parent)
            for entry in properties.get("ENVIRONMENT", []):
                key, value = entry.split("=", 1)
                direct[key] = value
            for entry in properties.get("ENVIRONMENT_MODIFICATION", []):
                key, operation = entry.split("=", 1)
                action, value = operation.split(":", 1)
                if action == "set":
                    direct[key] = value
                elif action == "path_list_prepend":
                    direct[key] = value + (os.pathsep + direct[key] if direct.get(key) else "")
                else:
                    raise ValueError(f"unhandled environment operation: {entry}")
            for invocation in ("direct", "ctest"):
                command = test["command"] if invocation == "direct" else [
                    "ctest", "--preset", args.preset, "-R", f"^{name}$", "--output-on-failure"]
                env = direct if invocation == "direct" else parent
                cwd = properties["WORKING_DIRECTORY"] if invocation == "direct" else ROOT
                label = f"{variant}-{name}-{invocation}"
                start = time.monotonic()
                with (output / (label + ".log")).open("w", encoding="utf-8") as stream:
                    result = subprocess.run(command, cwd=cwd, env=env, stdout=stream, stderr=subprocess.STDOUT)
                row = dict(case=label, exit=result.returncode, seconds=round(time.monotonic()-start, 3),
                           command=command, cwd=str(cwd), modifications=properties.get("ENVIRONMENT_MODIFICATION", []))
                results.append(row)
                (output / "results.json").write_text(json.dumps({"head": head, "preset": args.preset,
                    "python": test["command"][0], "python_version": version,
                    "results": results}, indent=2), encoding="utf-8")
                print(f"{label}: exit={row['exit']} seconds={row['seconds']}", flush=True)
    return int(any(row["exit"] for row in results))


if __name__ == "__main__":
    raise SystemExit(main())
