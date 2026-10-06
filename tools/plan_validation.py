#!/usr/bin/env python3
"""Build first; enter an explicit CURRENT test stage without escalating its scope."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from current_modules import select_for_purpose, selection_label, targets
from test_impact import changed_impact, route

PURPOSES = ("auto", "build", "targeted", "memory-safety", "performance", "core",
            "historical", "full", "diagnostic", "research")
FAILURES = ("none", "python-process", "interpreter", "ctest-environment")


def diagnostic_eligible(failure: str, *, requested: bool, cancelled: bool = False) -> bool:
    return requested and not cancelled and failure in FAILURES[1:]


def plan(*, context="local", purpose="auto", impact=None, module="all", configuration="debug",
         stage="build", diagnostic_test="none", failure="none", hypothesis="", timeout_seconds=120,
         study="none", cancelled=False):
    if context not in {"local", "pr", "dispatch"} or purpose not in PURPOSES or stage not in {"build", "test"}:
        raise ValueError("unsupported context, purpose or stage")
    if configuration not in {"debug", "release", "asan"} or cancelled:
        raise ValueError("unsupported configuration or cancelled request")
    if context == "dispatch" and purpose == "auto":
        raise ValueError("dispatch requires an explicit purpose")
    if context == "pr" and purpose not in {"auto", "build", "targeted", "core"}:
        raise ValueError("PR cannot escalate to memory/performance/research/historical/diagnostic")
    if purpose != "diagnostic" and (diagnostic_test != "none" or failure != "none" or hypothesis):
        raise ValueError("diagnostic inputs require diagnostic purpose")
    if study != "none" and purpose != "research":
        raise ValueError("study requires explicit research purpose")
    base = "ci-windows-debug" if context == "pr" and configuration == "debug" else f"windows-{configuration}"
    result = dict(purpose=purpose, context=context, stage=stage, active_modules=[],
        build_required=False, test_required=False, core_required=False, tooling_required=False,
        native_build_required=False, memory_safety_required=False, performance_required=False,
        research_required=False, historical_required=False, diagnostics_required=False,
        configure_preset=base, build_preset=base + "-build", test_build_preset="",
        build_targets=[], test_preset="", test_label="", reason=[])
    if purpose == "auto":
        if impact is None:
            raise ValueError("automatic planning requires real changed-file impact")
        result.update({key: impact[key] for key in ("active_modules", "build_required", "core_required", "tooling_required")})
        result["reason"] = impact["reason"] + ["Build and Test Stage are independent; no implicit heavy validation"]
        result["test_required"] = stage == "test" and bool(result["active_modules"] or result["core_required"])
    elif purpose == "build":
        result["build_required"] = True
        result["reason"] = ["project compile/link only; no CTest"]
    elif purpose in {"targeted", "memory-safety", "performance"}:
        if purpose == "memory-safety" and configuration != "asan":
            raise ValueError("memory-safety requires ASAN")
        if purpose == "performance" and configuration != "release":
            raise ValueError("performance requires Release")
        result.update(active_modules=select_for_purpose(module.split(","), purpose), build_required=True,
                      test_required=True, stage="test", reason=["explicit CURRENT " + purpose])
    elif purpose == "core":
        result.update(build_required=True, test_required=True, core_required=True, stage="test",
                      configure_preset=base + "-host", build_preset=base + "-core", test_build_preset=base + "-core",
                      test_preset=base + "-core", reason=["explicit Host/core contracts"])
    elif purpose in {"historical", "full"}:
        if module != "all":
            raise ValueError("archive profiles require module=all")
        result.update(build_required=True, test_required=True, stage="test", historical_required=True,
                      configure_preset=base + ("-all" if purpose == "full" else "-historical"),
                      test_build_preset=base + ("-full" if purpose == "full" else "-historical-tests"),
                      test_preset=base + ("-full" if purpose == "full" else "-historical-tests"),
                      reason=["explicit archival/forensic assets, never normal Final Validation"])
    elif purpose == "diagnostic":
        if not diagnostic_eligible(failure, requested=True) or not hypothesis.strip():
            raise ValueError("diagnostic requires observed Python failure and hypothesis")
        if diagnostic_test not in {"testdata", "render_cli"} or not 1 <= timeout_seconds <= 600:
            raise ValueError("select one diagnostic test and timeout 1..600")
        result.update(configure_preset=base + "-all", diagnostics_required=True,
                      diagnostic_test=diagnostic_test, failure=failure, hypothesis=hypothesis,
                      timeout_seconds=timeout_seconds, case_limit=4, test_preset=base + "-full",
                      test_build_preset=base + "-full" if diagnostic_test == "render_cli" else "",
                      build_targets=["frazil_water_experiment_render"] if diagnostic_test == "render_cli" else [],
                      reason=["explicit failure investigation only"])
    else:
        if configuration != "release" or module != "d1" or study not in {"latency", "convergence"}:
            raise ValueError("research requires D1/Release and one named study")
        result.update(configure_preset=base + "-historical", research_required=True, study=study,
                      test_build_preset=base + "-historical-tests", build_targets=["frazil_water_d1_research_tests"],
                      reason=["explicit historical D1 study; CURRENT remains unaffected"])
    if purpose == "auto" and result["test_required"] and result["core_required"] and not result["active_modules"]:
        result.update(configure_preset=base + "-host", build_preset=base + "-core",
                      test_build_preset=base + "-core", test_preset=base + "-core")
    if result["active_modules"] and result["test_required"]:
        selected_purpose = purpose if purpose in {"memory-safety", "performance"} else "correctness"
        result["build_targets"] = targets(result["active_modules"], selected_purpose)
        result["test_label"] = selection_label(result["active_modules"], selected_purpose)
        result["test_build_preset"] = base + "-current-tests"
        result["test_preset"] = base + "-current-tests"
        if purpose == "memory-safety":
            result.update(memory_safety_required=True, test_build_preset="windows-asan-current-memory",
                          test_preset="windows-asan-current-memory")
        elif purpose == "performance":
            result.update(performance_required=True, configure_preset="windows-release-performance",
                          test_build_preset="windows-release-current-performance",
                          test_preset="windows-release-current-performance")
    result["native_build_required"] = result["build_required"]
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", choices=("local", "pr", "dispatch"), default="local")
    parser.add_argument("--purpose", choices=PURPOSES, default="auto")
    parser.add_argument("--stage", choices=("build", "test"), default="build")
    parser.add_argument("--base")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("--module", default="all", help="registered IDs separated by commas, or all")
    parser.add_argument("--configuration", choices=("debug", "release", "asan"), default="debug")
    parser.add_argument("--diagnostic-test", default="none")
    parser.add_argument("--failure", default="none")
    parser.add_argument("--hypothesis", default="")
    parser.add_argument("--timeout-seconds", type=int, default=120)
    parser.add_argument("--study", default="none")
    parser.add_argument("--github-output", type=Path)
    parser.add_argument("paths", nargs="*")
    args = parser.parse_args()
    if args.purpose == "auto" and not args.base and not args.paths:
        parser.error("provide a real comparison base or changed paths")
    impact = None
    if args.purpose == "auto":
        impact = changed_impact(args.base, args.head) if args.base else route(args.paths)
    try:
        result = plan(context=args.context, purpose=args.purpose, impact=impact, stage=args.stage,
            module=args.module, configuration=args.configuration, diagnostic_test=args.diagnostic_test,
            failure=args.failure, hypothesis=args.hypothesis, timeout_seconds=args.timeout_seconds, study=args.study)
    except ValueError as error:
        parser.error(str(error))
    result.update(base=args.base, head=args.head)
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as stream:
            for key, value in result.items():
                if isinstance(value, str) and ("\n" in value or "\r" in value):
                    continue
                stream.write(f"{key}={value if isinstance(value, str) else json.dumps(value)}\n")


if __name__ == "__main__":
    main()
