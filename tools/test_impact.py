#!/usr/bin/env python3
"""Route changes to CURRENT owners without scheduling tests during local builds.

Unknown executable infrastructure is conservative (all modules). Research-only
scripts still select their owner; full numerical/listening studies require the
explicit validation-full dispatch, rather than running on every pull request.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess

from current_modules import load_modules

ROOT = Path(__file__).resolve().parents[1]
SPIKE = Path("experiments/water/SPIKE-W-DSP-001")
MODULES = tuple(load_modules())
# Only tools with dedicated lightweight regressions may bypass native builds.
TOOLING = frozenset(f"tools/{name}.py" for name in (
    "test_impact", "test_test_impact", "check_test_paths", "test_check_test_paths",
    "python_test_ab", "test_python_test_ab", "plan_validation", "test_plan_validation",
    "test_validation_workflow", "current_modules", "generate_current_presets",
    "check_current_tests", "test_current_modules", "run_current_tests",
))
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


def tooling_doc_diff(diff: str) -> bool:
    """Recognize documentation commands exclusively about allowlisted tooling."""
    commands = [line for line in diff.splitlines() if executable_doc_diff(line)]
    for line in commands:
        normalized = line.replace("\\", "/")
        paths = re.findall(r"tools/[\w/-]+\.py", normalized)
        if not paths or any(path not in TOOLING for path in paths):
            return False
        # A tooling command alongside a native build command is still broad.
        if re.search(r"\b(cmake|ctest|ninja)\s+[-\w]|\b(?:FRAZIL|CMAKE)_[A-Z_]+", normalized):
            return False
    return bool(commands)


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
          executable_docs: set[str] | None = None, tooling_docs: set[str] | None = None) -> dict:
    """Current source ownership, independent from archived A0/B1/common test suites."""
    root = root.resolve()
    structural, executable_docs, tooling_docs = structural or set(), executable_docs or set(), tooling_docs or set()
    registry = load_modules()
    owners: dict[str, set[str]] = {}
    for module, entry in registry.items():
        for path in dependencies([SPIKE / source for source in entry.get("impact_seeds", []) + entry["native"]["sources"]] +
                                 [Path("src/dsp/primitives/RandomSource.cpp")], root):
            owners.setdefault(path, set()).add(module)
        if "cli" in entry:
            owners.setdefault((SPIKE.parent / "contracts" / entry["cli"]["contract"]).as_posix(), set()).add(module)
    renderer = dependencies([SPIKE / "render/render_main.cpp"], root)
    selected, reasons = set(), []
    build_required = core_required = tooling_required = False
    for raw in paths:
        path = raw.replace("\\", "/")
        if documentation(path) and path not in executable_docs:
            continue
        if path not in structural and (path in TOOLING or path in tooling_docs):
            tooling_required = True
            continue
        if path.startswith(("src/plugin/", "src/app/", "src/ui/", "src/dsp/")):
            build_required = core_required = True
        if path in structural:
            selected.update(registry)
            build_required = True
            reasons.append("new/deleted executable infrastructure: " + path)
        elif path in owners:
            selected.update(owners[path])
            build_required = True
        elif path in renderer or path == (SPIKE / "tests/current_cli_test.py").as_posix():
            selected.update(name for name, entry in registry.items() if "cli" in entry)
            build_required = True
        elif path.startswith(("src/plugin/", "src/app/", "src/ui/", "src/dsp/", "tests/unit/", "tests/integration/",
                              "tests/property/", "tests/smoke/", "tests/render/", "tests/latency/")):
            build_required = core_required = True
        elif path.startswith(str(SPIKE / "tests").replace("\\", "/") + "/") and (root / path).is_file():
            reasons.append("archived test source; explicit historical validation only: " + path)
        elif path.startswith("testdata/") or path in {"tools/generate_testdata.py", "tools/test_testdata.py"}:
            reasons.append("corpus/research maintenance requires explicit validation: " + path)
            tooling_required = True
        else:
            build_required = True
            selected.update(registry)
            reasons.append("unclassified infrastructure; conservative CURRENT scope: " + path)
    active = [name for name in registry if name in selected]
    return dict(active_modules=active, modules=active, water=bool(active), build_required=build_required,
                native_build_required=build_required, core_required=core_required,
                tooling_required=tooling_required, testdata=False, research_dependencies=False, reason=reasons)

def changed_impact(base: str, head: str = "HEAD") -> dict:
    """Read real Git changes; missing/invalid revisions fail instead of inventing paths."""
    def names(*filters: str) -> list[str]:
        output = subprocess.check_output(["git", "diff", "--name-only", "--no-renames", "-z",
                                          *filters, base, head], cwd=ROOT)
        return [p.decode("utf-8") for p in output.split(b"\0") if p]

    paths = names()
    executable_docs, tooling_docs = set(), set()
    for path in paths:
        if documentation(path):
            diff = subprocess.check_output(["git", "diff", "--unified=0", base, head,
                                            "--", path], cwd=ROOT).decode("utf-8")
            if executable_doc_diff(diff):
                executable_docs.add(path)
                if tooling_doc_diff(diff):
                    tooling_docs.add(path)
    return route(paths, structural=set(names("--diff-filter=AD")),
                 executable_docs=executable_docs, tooling_docs=tooling_docs)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("--github-output", type=Path)
    parser.add_argument("paths", nargs="*")
    args = parser.parse_args()
    result = changed_impact(args.base, args.head) if args.base else route(args.paths)
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as stream:
            for key, value in result.items():
                stream.write(f"{key}={json.dumps(value, separators=(',', ':'))}\n")


if __name__ == "__main__":
    main()
