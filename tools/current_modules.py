"""Canonical CURRENT module registry shared by planning, builds and test selection."""
from __future__ import annotations

import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SPIKE = Path("experiments/water/SPIKE-W-DSP-001")
REGISTRY = ROOT / "tests/current_modules.json"


def validate_modules(data: dict, root: Path = ROOT) -> dict[str, dict]:
    """Native is required; supplied optional capabilities must be complete and real."""
    if data.get("schema_version") != 2 or not data.get("modules"):
        raise ValueError("CURRENT schema v2 must contain real modules")
    result, used_targets = {}, set()
    def source_exists(source, parent):
        resolved = (parent / source).resolve()
        if not resolved.is_relative_to(parent.resolve()) or not resolved.is_file():
            raise ValueError(f"missing/unsafe registered source: {source}")
    for module in data["modules"]:
        name = module.get("id", "")
        if not re.fullmatch(r"[a-z][a-z0-9]*", name) or name in result:
            raise ValueError(f"duplicate/invalid CURRENT module: {name}")
        if set(module) - {"id", "impact_seeds", "native", "cli", "performance"}:
            raise ValueError(f"unknown capability or field: {name}")
        native = module.get("native")
        if not isinstance(native, dict) or set(native) != {"target", "sources"} or not native["sources"]:
            raise ValueError(f"{name} requires native target and nonempty sources")
        for capability, fields in (("cli", {"mode", "config_key", "version", "descriptor", "contract"}),
                                   ("performance", {"target", "source", "kind"})):
            if capability in module and (not isinstance(module[capability], dict) or set(module[capability]) != fields):
                raise ValueError(f"incomplete {capability} capability: {name}")
        for capability in (native, *([module["performance"]] if "performance" in module else [])):
            target = capability["target"]
            if not re.fullmatch(r"frazil_[a-z0-9_]+", target) or target in used_targets:
                raise ValueError(f"duplicate/invalid target: {target}")
            used_targets.add(target)
        sources = native["sources"] + module.get("impact_seeds", [])
        if "performance" in module:
            sources += [module["performance"]["source"]]
            if not isinstance(module["performance"]["kind"], str) or not module["performance"]["kind"]:
                raise ValueError(f"missing performance validator: {name}")
        for source in sources:
            source_exists(source, root / SPIKE)
        if "cli" in module:
            cli = module["cli"]
            if type(cli["version"]) is not int or any(not isinstance(cli[k], str) or not cli[k]
                    for k in ("mode", "config_key", "descriptor", "contract")):
                raise ValueError(f"invalid CLI capability: {name}")
            source_exists(cli["contract"], root / SPIKE.parent / "contracts")
        result[name] = module
    return result


def load_modules(path: Path = REGISTRY) -> dict[str, dict]:
    return validate_modules(json.loads(path.read_text(encoding="utf-8")))


def select_modules(names: list[str], registry: dict[str, dict] | None = None) -> list[str]:
    registry = load_modules() if registry is None else registry
    if not names or names == ["all"]:
        return list(registry)
    if "all" in names or any(name not in registry for name in names):
        raise ValueError("select registered CURRENT modules, or all")
    return [name for name in registry if name in names]


def select_for_purpose(names: list[str], purpose="correctness", registry=None) -> list[str]:
    registry = load_modules() if registry is None else registry
    selected = select_modules(names, registry)
    if purpose == "performance":
        missing = [name for name in selected if "performance" not in registry[name]]
        if missing and names and names != ["all"]:
            raise ValueError(", ".join(missing) + " has no registered performance capability")
        selected = [name for name in selected if name not in missing]
        if not selected:
            raise ValueError("no registered performance capability in selection")
    return selected


def expected_tests(modules, purpose="correctness", registry=None) -> set[str]:
    registry = load_modules() if registry is None else registry
    expected = set()
    for name in select_modules(list(modules), registry):
        entry = registry[name]
        if purpose == "performance":
            if "performance" in entry:
                expected.add(f"frazil_current_{name}_performance")
        else:
            expected.add(f"frazil_current_{name}")
            if "cli" in entry:
                expected.add(f"frazil_current_{name}_cli")
    return expected


def targets(modules: list[str], purpose: str = "correctness") -> list[str]:
    suffix = "performance" if purpose == "performance" else "current"
    return [f"frazil_test_{name}_{suffix}" for name in select_for_purpose(modules, purpose)]


def selection_label(modules: list[str], purpose: str = "correctness") -> str:
    # Intersection aliases avoid CTest's OR semantics across separate -L options.
    domain = "memory" if purpose == "memory-safety" else purpose
    return "^current-(" + "|".join(select_for_purpose(modules, purpose)) + ")-" + domain + "$"


def registered_build_targets(registry=None) -> tuple[str, ...]:
    """Closed safe-wrapper additions: never invent unavailable capability targets."""
    registry = load_modules() if registry is None else registry
    result = [f"frazil_test_{name}_current" for name in registry]
    for name, entry in registry.items():
        if "performance" in entry:
            result.extend((f"frazil_test_{name}_performance", entry["performance"]["target"]))
    return tuple(result)
