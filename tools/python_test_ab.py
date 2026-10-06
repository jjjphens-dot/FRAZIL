#!/usr/bin/env python3
"""Explicit, bounded direct/CTest diagnosis for one observed Python failure.

Logs supplement first-failure evidence. No retries, automatic Full post-step,
interpreter changes or unrelated second test are scheduled.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import time

from plan_validation import FAILURES, diagnostic_eligible

ROOT = Path(__file__).resolve().parents[1]
TESTS = {"testdata": "frazil_testdata_regeneration", "render_cli": "frazil_water_experiment_render_cli"}
PRESETS = tuple(f"windows-{config}-full" for config in ("debug", "release", "asan"))


def case_ids(test: str, limit: int) -> list[tuple[str, str, str]]:
    if test not in TESTS or not 1 <= limit <= 4:
        raise ValueError("select one known test and a case limit in 1..4")
    return [(f"{site}-{TESTS[test]}-{mode}", site, mode)
            for site in ("configured", "no-user-site") for mode in ("direct", "ctest")][:limit]


def direct_environment(parent: dict, properties: dict) -> dict:
    result = dict(parent)
    for entry in properties.get("ENVIRONMENT", []):
        key, value = entry.split("=", 1)
        result[key] = value
    for entry in properties.get("ENVIRONMENT_MODIFICATION", []):
        key, operation = entry.split("=", 1)
        action, value = operation.split(":", 1)
        if action == "set":
            result[key] = value
        elif action == "path_list_prepend":
            result[key] = value + (os.pathsep + result[key] if result.get(key) else "")
        else:
            raise ValueError(f"unhandled environment operation: {entry}")
    return result


def stop_tree(process: subprocess.Popen) -> None:
    """Timeout/cancellation includes CTest's Python and native descendants."""
    if os.name == "nt":
        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=10, check=True)
    else:
        os.killpg(process.pid, signal.SIGKILL)
    process.wait(timeout=10)


def run_case(command: list[str], *, cwd: Path, env: dict, log: Path, timeout: int) -> int:
    with log.open("w", encoding="utf-8") as stream:
        process = subprocess.Popen(command, cwd=cwd, env=env, stdout=stream, stderr=subprocess.STDOUT,
                                   start_new_session=os.name != "nt",
                                   creationflags=subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0)
        try:
            return process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            stop_tree(process)
            return 124
        except KeyboardInterrupt:
            stop_tree(process)
            raise  # Never continue with another case after cancellation.


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", choices=PRESETS, required=True)
    parser.add_argument("--test", choices=TESTS, required=True)
    parser.add_argument("--failure", choices=FAILURES[1:], required=True)
    parser.add_argument("--hypothesis", required=True)
    parser.add_argument("--timeout-seconds", type=int, default=120)
    parser.add_argument("--max-cases", type=int, default=4)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not diagnostic_eligible(args.failure, requested=True) or not args.hypothesis.strip():
        parser.error("a concrete failure and hypothesis are required")
    if not 1 <= args.timeout_seconds <= 600:
        parser.error("per-case timeout must be in 1..600 seconds")
    try:
        cases = case_ids(args.test, args.max_cases)
    except ValueError as error:
        parser.error(str(error))
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("diagnostic output must remain in repository build/")
    output.mkdir(parents=True, exist_ok=False)
    inventory = json.loads(subprocess.check_output(
        ["ctest", "--preset", args.preset, "--show-only=json-v1"], cwd=ROOT, text=True, timeout=30))
    selected = [t for t in inventory["tests"] if t["name"] == TESTS[args.test]]
    if len(selected) != 1:
        raise ValueError("diagnostic requires exactly one matching registered test")
    test = selected[0]
    properties = {p["name"]: p["value"] for p in test["properties"]}
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True, timeout=10).strip()
    version = subprocess.check_output([test["command"][0], "--version"], text=True, timeout=10).strip()
    report = dict(head=head, preset=args.preset, test=args.test, failure=args.failure,
                  hypothesis=args.hypothesis, timeout_seconds=args.timeout_seconds, case_limit=args.max_cases,
                  changed_variable="invocation mode and user-site environment (paired comparisons)",
                  control="configured/direct", interpretation="pending; no retry replaces first failure",
                  next_hypothesis="not selected", python=test["command"][0], python_version=version, results=[])
    for label, site, invocation in cases:
        parent = dict(os.environ, PYTHONFAULTHANDLER="1")
        if site == "no-user-site":
            parent["PYTHONNOUSERSITE"] = "1"
        command = test["command"] if invocation == "direct" else [
            "ctest", "--preset", args.preset, "-R", f"^{test['name']}$", "--output-on-failure",
            "--timeout", str(args.timeout_seconds)]
        env = direct_environment(parent, properties) if invocation == "direct" else parent
        cwd = Path(properties["WORKING_DIRECTORY"]) if invocation == "direct" else ROOT
        start = time.monotonic()
        try:
            code = run_case(command, cwd=cwd, env=env, log=output / (label + ".log"), timeout=args.timeout_seconds)
        except KeyboardInterrupt:
            code = 130
        report["results"].append(dict(case=label, exit=code, seconds=round(time.monotonic()-start, 3),
                                      command=command, cwd=str(cwd),
                                      modifications=properties.get("ENVIRONMENT_MODIFICATION", [])))
        (output / "results.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(f"{label}: exit={code}", flush=True)
        if code in (124, 130):
            return code  # A timeout/cancel is a stop, not a reason for more diagnostic work.
    return int(any(row["exit"] for row in report["results"]))


if __name__ == "__main__":
    def cancel(signum, frame):
        raise KeyboardInterrupt

    signal.signal(signal.SIGTERM, cancel)
    raise SystemExit(main())
