"""Dedicated Test Stage: build the selected target union once, execute each CTest once.

Configuration and the production Build Gate are separate caller-owned stages.
Without --execute this command only prints/validates registration metadata.
"""
import argparse
import json
import subprocess
import sys

from current_modules import ROOT, expected_tests
from plan_validation import plan


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", choices=("windows-debug", "windows-release", "windows-asan", "ci-windows-debug"), required=True)
    parser.add_argument("--modules", default="all")
    parser.add_argument("--purpose", choices=("correctness", "memory-safety", "performance"), default="correctness")
    parser.add_argument("--execute", action="store_true")
    args = parser.parse_args()
    configuration = args.preset.removeprefix("ci-").removeprefix("windows-")
    request = plan(context="pr" if args.preset.startswith("ci-") else "local",
                   purpose="targeted" if args.purpose == "correctness" else args.purpose,
                   module=args.modules, configuration=configuration)
    print("VALIDATION PLAN\n" + json.dumps(request, indent=2), flush=True)
    selection = ["ctest", "--preset", request["test_preset"], "-L", request["test_label"]]
    inventory = json.loads(subprocess.check_output(selection + ["--show-only=json-v1"], cwd=ROOT, text=True))
    names = [t["name"] for t in inventory["tests"]]
    expected = expected_tests(request["active_modules"], args.purpose)
    if set(names) != expected or len(names) != len(expected):
        raise SystemExit("configured CURRENT selection is empty, stale or has duplicate/unrelated tests")
    print("Selected once: " + ", ".join(names), flush=True)
    if args.execute:
        subprocess.run([sys.executable, str(ROOT / "tools/build_safe.py"), "--preset", request["test_build_preset"],
                        "--target", *request["build_targets"]], cwd=ROOT, check=True)
        return subprocess.run(selection + ["--no-tests=error", "--output-junit", "current-selected.xml"], cwd=ROOT).returncode
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
