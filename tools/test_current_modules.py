"""Registry, generated selectors and leak detection; no native test execution."""
import copy
import json
import unittest
from current_modules import ROOT, load_modules, select_modules, selection_label
from generate_current_presets import generate
from check_current_tests import validate_inventory


class CurrentTests(unittest.TestCase):
    def test_registry_has_real_current_modules_and_no_placeholder(self):
        modules = load_modules()
        self.assertEqual(list(modules), ["a1", "b2", "d1"])
        with self.assertRaises(ValueError):
            select_modules(["c"])
        self.assertEqual(select_modules(["d1", "a1", "d1"]), ["a1", "d1"])

    def test_extension_and_union_use_registry(self):
        registry = {"a1": {}, "b2": {}, "d1": {}, "c1": {}}
        self.assertEqual(select_modules(["c1", "a1", "c1"], registry), ["a1", "c1"])
        self.assertEqual(selection_label(["d1", "b2"], "memory-safety"), "^current-(b2|d1)-memory$")

    def test_presets_idempotent_and_production_only(self):
        data = json.loads((ROOT / "CMakePresets.json").read_text())
        self.assertEqual(generate(copy.deepcopy(data)), data)
        for preset in data["buildPresets"]:
            if preset["name"].endswith("-build"):
                self.assertEqual(preset["targets"], ["FRAZIL_All"])
        for module in load_modules():
            self.assertTrue(any(p["name"] == f"windows-debug-{module}-test" for p in data["testPresets"]))

    def test_inventory_rejects_leaks_empty_duplicate_and_timing(self):
        tests = [{"name": "frazil_current_a1" + suffix,
                  "properties": [{"name": "LABELS", "value": ["current"]}]} for suffix in ("", "_cli")]
        validate_inventory({"tests": tests}, ["a1"])
        for bad in ([], tests + tests[:1], tests + [{"name": "historical"}],
                    [{**tests[0], "properties": [{"name": "LABELS", "value": ["current", "performance"]}]}, tests[1]]):
            with self.assertRaises(ValueError):
                validate_inventory({"tests": bad}, ["a1"])


if __name__ == "__main__":
    unittest.main()
