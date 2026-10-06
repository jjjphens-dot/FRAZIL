"""Canonical CURRENT module registry shared by planning, builds and test selection."""
from __future__ import annotations

import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SPIKE = Path("experiments/water/SPIKE-W-DSP-001")
REGISTRY = ROOT / "tests/current_modules.json"


def load_modules(path: Path = REGISTRY) -> dict[str, dict]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1 or not data.get("modules"):
        raise ValueError("CURRENT registry must contain real modules")
    result = {}
    for module in data["modules"]:
        name = module["id"]
        if not re.fullmatch(r"[a-z][a-z0-9]*", name) or name in result:
            raise ValueError(f"duplicate/invalid CURRENT module: {name}")
        for key in ("native_target", "performance_target"):
            if not re.fullmatch(r"frazil_[a-z0-9_]+", module[key]):
                raise ValueError(f"invalid target: {module[key]}")
        for source in [*module["sources"], *module["impact_seeds"], module["performance_source"]]:
            resolved = (ROOT / SPIKE / source).resolve()
            if not resolved.is_relative_to(ROOT / SPIKE) or not resolved.is_file():
                raise ValueError(f"missing/unsafe registered source: {source}")
        if not (ROOT / SPIKE.parent / "contracts" / module["contract"]).is_file():
            raise ValueError(f"missing descriptor contract: {name}")
        result[name] = module
    return result


def select_modules(names: list[str], registry: dict[str, dict] | None = None) -> list[str]:
    registry = load_modules() if registry is None else registry
    if not names or names == ["all"]:
        return list(registry)
    if "all" in names or any(name not in registry for name in names):
        raise ValueError("select registered CURRENT modules, or all")
    return [name for name in registry if name in names]


def targets(modules: list[str], purpose: str = "correctness") -> list[str]:
    suffix = "performance" if purpose == "performance" else "current"
    return [f"frazil_test_{name}_{suffix}" for name in select_modules(modules)]


def selection_label(modules: list[str], purpose: str = "correctness") -> str:
    # Derived intersection labels avoid CTest's OR semantics across separate -L options.
    domain = "memory" if purpose == "memory-safety" else purpose
    return "^current-(" + "|".join(select_modules(modules)) + ")-" + domain + "$"
