#!/usr/bin/env python3
"""Plan scoped validation without configuring, building or executing test bodies.

Phase A supports existing Fast, explicit one-config Full, isolated D1 studies and
failure-specific Python diagnostics. Dedicated correctness/memory/performance
purposes remain unavailable until their coverage audit and selectors are ready.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from test_impact import MODULES, changed_impact, route

PURPOSES = ("auto", "targeted", "full", "diagnostic", "research")
FAILURES = ("none", "python-process", "interpreter", "ctest-environment")
CHECKS = ["test_test_impact", "test_plan_validation", "test_python_test_ab",
          "test_validation_workflow", "test_check_test_paths", "test_build_safe",
          "check_portability", "check_markdown_links"]


def diagnostic_eligible(failure: str, *, requested: bool, cancelled: bool = False) -> bool:
    return requested and not cancelled and failure in FAILURES[1:]


def plan(*, context: str = "local", purpose: str = "auto", impact: dict | None = None,
         module: str = "core", configuration: str = "debug", diagnostic_test: str = "none",
         failure: str = "none", hypothesis: str = "", timeout_seconds: int = 120,
         study: str = "none", cancelled: bool = False) -> dict:
    if context not in {"local", "pr", "dispatch"} or purpose not in PURPOSES:
        raise ValueError("unsupported context or purpose")
    if configuration not in {"debug", "release", "asan"} or module not in {"core", "all", *MODULES}:
        raise ValueError("unsupported configuration or module")
    if cancelled:
        raise ValueError("cancelled request must not schedule new work")
    if purpose == "auto" and context == "dispatch":
        raise ValueError("dispatch requires an explicit purpose")
    if purpose != "auto" and context == "pr":
        raise ValueError("PR changes cannot request heavy/manual purposes")
    if purpose != "diagnostic" and (diagnostic_test != "none" or failure != "none" or hypothesis):
        raise ValueError("diagnostic inputs require diagnostic purpose")
    if study != "none" and purpose != "research":
        raise ValueError("study input requires research purpose")
    result = dict(purpose=purpose, context=context, modules=[], water=False,
                  core_required=False, tooling_required=False, native_build_required=False,
                  testdata=False, research_dependencies=False, performance_required=False,
                  diagnostics_required=False, checks=[], configure_preset="", build_preset="",
                  build_target="", test_preset="", test_label="", reason=[])
    if purpose == "auto":
        if impact is None:
            raise ValueError("automatic planning requires actual changed-file impact")
        result.update(impact)
        result["checks"] = CHECKS
        result["purpose"] = ("targeted" if impact["native_build_required"] else
                             "tooling" if impact["tooling_required"] else "docs-only")
        result["reason"] = ["changed-file impact; no automatic Full, timing or diagnostics"]
        return result

    base = f"windows-{configuration}"
    result.update(configure_preset=base, reason=[f"explicit {purpose}: {module}/{configuration}"])
    if purpose == "targeted":
        label = "water-preview" if module == "preview" else module
        target = ("frazil_fast_tests" if module == "all" else "frazil_core_tests" if module == "core"
                  else f"frazil_water_{module.removeprefix('water-')}_test_group")
        result.update(build_preset=base + "-fast", build_target=target, test_preset=base + "-fast",
                      test_label="^fast$" if module == "all" else f"^fast-{label}$",
                      native_build_required=True, core_required=module in {"core", "all"},
                      modules=list(MODULES) if module == "all" else [] if module == "core" else [module],
                      water=module != "core", research_dependencies=module in {"all", "water-b1", "water-d1"})
    elif purpose == "full":
        if module != "all":
            raise ValueError("Full requires module=all; it is one explicitly selected configuration")
        result.update(build_preset=base + "-full", test_preset=base + "-full",
                      native_build_required=True, core_required=True, modules=list(MODULES),
                      water=True, testdata=True, research_dependencies=True, performance_required=True)
    elif purpose == "diagnostic":
        if not diagnostic_eligible(failure, requested=True) or not hypothesis.strip():
            raise ValueError("diagnostics require a Python failure class and a concrete hypothesis")
        if diagnostic_test not in {"testdata", "render_cli"} or not 1 <= timeout_seconds <= 600:
            raise ValueError("select one diagnostic test and a timeout in 1..600 seconds")
        if module != ("core" if diagnostic_test == "testdata" else "water-common"):
            raise ValueError("diagnostic module must match the selected test")
        render = diagnostic_test == "render_cli"
        result.update(diagnostics_required=True, diagnostic_test=diagnostic_test,
                      failure=failure, hypothesis=hypothesis.strip(), timeout_seconds=timeout_seconds,
                      case_limit=4, test_preset=base + "-full", water=render,
                      native_build_required=render, research_dependencies=render,
                      build_preset=base + "-fast" if render else "",
                      build_target="frazil_water_experiment_render" if render else "")
    else:
        if module != "water-d1" or configuration != "release" or study not in {"latency", "convergence"}:
            raise ValueError("Phase A research dispatch requires D1/Release and one explicit study")
        result.update(study=study, water=True, modules=[module], native_build_required=True,
                      research_dependencies=True, build_preset=base + "-fast",
                      build_target="frazil_water_d1_research_tests")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", choices=("local", "pr", "dispatch"), default="local")
    parser.add_argument("--purpose", choices=PURPOSES, default="auto")
    parser.add_argument("--base")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("--module", choices=("core", *MODULES, "all"), default="core")
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
        parser.error("provide --base/--head or changed paths; missing input is not docs-only")
    impact = None
    if args.purpose == "auto":
        impact = changed_impact(args.base, args.head) if args.base else route(args.paths)
    try:
        result = plan(context=args.context, purpose=args.purpose, impact=impact,
                      module=args.module, configuration=args.configuration,
                      diagnostic_test=args.diagnostic_test, failure=args.failure,
                      hypothesis=args.hypothesis, timeout_seconds=args.timeout_seconds, study=args.study)
    except ValueError as error:
        parser.error(str(error))
    result.update(base=args.base, head=args.head)
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as stream:
            for key, value in result.items():
                # Strings are raw workflow values; lists/bools are JSON, never shell code.
                if isinstance(value, str) and ("\n" in value or "\r" in value):
                    continue  # Free-text hypothesis stays in the printed plan only.
                stream.write(f"{key}={value if isinstance(value, str) else json.dumps(value)}\n")


if __name__ == "__main__":
    main()
