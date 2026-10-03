#!/usr/bin/env python3
"""Route changed paths to fast test owners using local C++ and Python dependencies.

Unknown executable infrastructure is conservative (all modules). Research-only
scripts still select their owner; full numerical/listening studies require the
explicit validation-full dispatch, rather than running on every pull request.
"""
from __future__ import annotations

import argparse
import ast
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SPIKE = Path("experiments/water/SPIKE-W-DSP-001")
MODULES = ("water-common", "water-a1", "water-b1", "water-b2", "water-d1", "water-protect", "preview")
COMMON = "baseline features modal excitation normalization motion bubble flow droplet activity fluid event_pool mapping".split()
SOURCES = {
    "water-common": [*(f"tests/{s}_tests.cpp" for s in COMMON), "tests/droplet_b1_allocation_tests.cpp", "render/render_main.cpp"],
    "water-a1": ["tests/bubble_a1_tests.cpp"],
    "water-b1": [*(f"tests/droplet_b1{s}_tests.cpp" for s in ("", "_physics", "_onset", "_allocation")), "render/render_main.cpp"],
    "water-b2": ["tests/droplet_b2_tests.cpp"],
    "water-d1": ["tests/flow_d1_tests.cpp", "tests/droplet_b1_allocation_tests.cpp", "render/render_main.cpp"],
    "water-protect": ["tests/protect_tests.cpp", "tests/protect_detector_tests.cpp", "render/render_main.cpp"],
    "preview": ["preview/PreviewController.cpp"],
}
PYTHON_SOURCES = {
    "water-common": ["render_cli_smoke", "render_cli_contract", "render_cli_full_matrix", "render_cli_test", "evidence_tools_test"],
    "water-a1": ["bubble_a1_cli_test"],
    "water-b1": ["b1_cli_smoke", "b1_cli_contract", "b1_cli_full_matrix", "b1_listening_pack_validation", "droplet_b1_cli_test"],
    "water-b2": ["droplet_b2_cli_test"],
    "water-d1": ["d1_cli_smoke", "d1_cli_contract", "d1_cli_full_matrix", "d1_native_oracle_validation", "flow_d1_cli_test", "flow_d1_latency_test", "flow_d1_latency_native_test", "flow_d1_remediation_test", "flow_d1_convergence_test"],
    "water-protect": ["render_cli_contract", "render_cli_full_matrix", "protect_listening_test"],
}


def python_dependencies(seeds: list[Path], root: Path) -> set[str]:
    """Follow local imports, including the research render helpers added to sys.path."""
    pending = [root / p for p in seeds]
    seen: set[str] = set()
    while pending:
        path = pending.pop().resolve()
        if not path.is_relative_to(root) or not path.is_file():
            continue
        relative = path.relative_to(root).as_posix()
        if relative in seen:
            continue
        seen.add(relative)
        tree = ast.parse(path.read_text(encoding="utf-8"))
        imports = []
        for node in ast.walk(tree):
            if isinstance(node, ast.Import):
                imports.extend(alias.name for alias in node.names)
            elif isinstance(node, ast.ImportFrom) and node.module:
                imports.append(node.module)
        for name in imports:
            for parent in (path.parent, root / SPIKE / "tests", root / SPIKE / "render"):
                candidate = parent / (name.replace(".", "/") + ".py")
                if candidate.is_file():
                    pending.append(candidate)
                    break
    return seen


def documentation(path: str) -> bool:
    return Path(path).suffix.lower() in {".md", ".rst"} or path.startswith(
        (".github/ISSUE_TEMPLATE/", ".github/PULL_REQUEST_TEMPLATE/"))


def executable_doc_diff(diff: str) -> bool:
    """Command/preset changes in prose require engineering checks; wording does not."""
    return any(re.search(r"\b(cmake|ctest|python(?:3)?|ninja)\s+[-\w]|"
                         r"CMakePresets\.json|\.github/workflows/|(?i:tools/[\w/-]+\.(py|ps1|cmd))|"
                         r"\b(?:FRAZIL|CMAKE)_[A-Z_]+", line[1:].replace("\\", "/"))
               for line in diff.splitlines()
               if line.startswith(("+", "-")) and not line.startswith(("+++", "---")))


def dependencies(seeds: list[Path], root: Path = ROOT) -> set[str]:
    """Follow quoted project includes; vendor/system includes stay outside routing."""
    pending = [root / p for p in seeds]
    seen: set[str] = set()
    while pending:
        path = pending.pop().resolve()
        if not path.is_relative_to(root) or not path.is_file():
            continue
        relative = path.relative_to(root).as_posix()
        if relative in seen:
            continue
        seen.add(relative)
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', path.read_text(encoding="utf-8"), re.M):
            for parent in (path.parent, root / SPIKE, root / "src"):
                candidate = parent / include
                if candidate.is_file():
                    pending.append(candidate)
                    break
    return seen


def route(paths: list[str], root: Path = ROOT, *, structural: set[str] | None = None,
          executable_docs: set[str] | None = None) -> dict:
    root = root.resolve()
    structural = structural or set()
    executable_docs = executable_docs or set()
    owners: dict[str, set[str]] = {}
    for module, sources in SOURCES.items():
        # All native Water targets link frazil_water_research/RandomSource.cpp.
        seeds = [SPIKE / s for s in sources] + [Path("src/dsp/primitives/RandomSource.cpp")]
        if module == "preview":
            seeds += [p.relative_to(root) for p in (root / SPIKE / "tests").glob("preview*_tests.cpp")]
            seeds.append(Path("src/dsp/primitives/LinearSmoother.cpp"))
        for path in dependencies(seeds, root):
            owners.setdefault(path, set()).add(module)
    for module, scripts in PYTHON_SOURCES.items():
        for path in python_dependencies([SPIKE / "tests" / (s + ".py") for s in scripts], root):
            owners.setdefault(path, set()).add(module)
    selected: set[str] = set()
    testdata = False
    core_required = False
    for raw in paths:
        path = raw.replace("\\", "/")
        testdata |= path.startswith("testdata/") or path in {
            "tools/generate_testdata.py", "tools/verify_testdata.py", "tools/test_testdata.py",
        }
        if documentation(path) and path not in executable_docs:
            continue
        core_required = True
        if path in structural or path in executable_docs:
            selected.update(MODULES)
        elif path in owners:
            selected.update(owners[path])
        elif path.startswith(("testdata/", "tests/", "src/")):
            continue  # Production changes select ci-core, with no unrelated Water tests.
        else:
            # New/deleted files, unparsed includes, scripts/config/contracts, build,
            # CI and tooling affect integration. Fail closed, including renames.
            selected.update(MODULES)
    modules = [m for m in MODULES if m in selected]
    return {"modules": modules, "water": bool(modules), "testdata": testdata,
            "core_required": core_required,
            "research_dependencies": bool(selected.intersection({"water-b1", "water-d1"}))}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("--github-output", type=Path)
    parser.add_argument("paths", nargs="*")
    args = parser.parse_args()
    paths = args.paths
    structural: set[str] = set()
    executable_docs: set[str] = set()
    if args.base:
        # Invalid/missing base fails the job rather than silently selecting zero tests.
        output = subprocess.check_output(["git", "diff", "--name-only", "--no-renames", "-z",
                                          args.base, args.head], cwd=ROOT)
        paths = [p.decode("utf-8") for p in output.split(b"\0") if p]
        output = subprocess.check_output(["git", "diff", "--name-only", "--no-renames", "-z",
                                          "--diff-filter=AD", args.base, args.head], cwd=ROOT)
        structural = {p.decode("utf-8") for p in output.split(b"\0") if p}
        for path in paths:
            if documentation(path):
                diff = subprocess.check_output(["git", "diff", "--unified=0", args.base,
                                                args.head, "--", path], cwd=ROOT).decode("utf-8")
                if executable_doc_diff(diff):
                    executable_docs.add(path)
    result = route(paths, structural=structural, executable_docs=executable_docs)
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as stream:
            for key, value in result.items():
                stream.write(f"{key}={json.dumps(value, separators=(',', ':'))}\n")


if __name__ == "__main__":
    main()
