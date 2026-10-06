"""Registry, generated selectors and leak detection; no native test execution."""
import copy
import json
import unittest
from unittest.mock import patch
from current_modules import ROOT, load_modules, select_modules, selection_label, validate_modules, expected_tests, targets, registered_build_targets
from generate_current_presets import generate
from check_current_tests import validate_inventory
from plan_validation import plan


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

    @staticmethod
    def native_only_registry():
        # Synthetic fixture reuses a real source path; never written to the project registry.
        data = json.loads((ROOT / "tests/current_modules.json").read_text())
        data["modules"].append({"id": "c", "native": {
            "target": "frazil_fixture_c_tests", "sources": ["tests/flow_d1_tests.cpp"]}})
        return validate_modules(data)

    def test_native_only_module_inventory_and_plans(self):
        registry = self.native_only_registry()
        self.assertEqual(expected_tests(["c"], registry=registry), {"frazil_current_c"})
        self.assertEqual(expected_tests(["c"], "memory-safety", registry), {"frazil_current_c"})
        self.assertEqual(len(expected_tests(registry, registry=registry)), 7)
        self.assertEqual(len(expected_tests(registry, "performance", registry)), 3)
        with patch("current_modules.load_modules", return_value=registry):
            result = plan(purpose="targeted", module="c")
            self.assertEqual(result["build_targets"], ["frazil_test_c_current"])
            self.assertEqual(plan(purpose="memory-safety", module="c", configuration="asan")["active_modules"], ["c"])
            for module in ("c", "a1,c"):
                with self.assertRaisesRegex(ValueError, "c has no registered performance capability"):
                    plan(purpose="performance", module=module, configuration="release")
            self.assertEqual(plan(purpose="performance", configuration="release")["active_modules"], ["a1", "b2", "d1"])
            self.assertEqual(targets(["c"]), ["frazil_test_c_current"])
        with patch("current_modules.load_modules", return_value={"c": registry["c"]}):
            with self.assertRaisesRegex(ValueError, "no registered performance capability"):
                plan(purpose="performance", configuration="release")
        from test_impact import route, SPIKE
        with patch("test_impact.load_modules", return_value=registry):
            self.assertEqual(route([str(SPIKE / "render/render_main.cpp")])["active_modules"], ["a1", "b2", "d1"])
        allowed = registered_build_targets(registry)
        self.assertIn("frazil_test_c_current", allowed)
        self.assertNotIn("frazil_test_c_performance", allowed)
        self.assertNotIn("frazil_fixture_c_tests", allowed)
        presets = generate(json.loads((ROOT / "CMakePresets.json").read_text()), registry)
        self.assertTrue(any(p["name"] == "windows-debug-c-test" for p in presets["testPresets"]))
        native = {"name": "frazil_current_c", "properties": [{"name": "LABELS", "value": ["current"]}]}
        validate_inventory({"tests": [native]}, ["c"], registry=registry)
        with self.assertRaises(ValueError):
            validate_inventory({"tests": []}, ["c"], "performance", registry)

    def test_optional_capabilities_must_be_complete(self):
        original = json.loads((ROOT / "tests/current_modules.json").read_text())
        for capability in ("cli", "performance"):
            data = copy.deepcopy(original)
            data["modules"][0][capability] = {}
            with self.assertRaisesRegex(ValueError, "incomplete"):
                validate_modules(data)
        for bad in ({}, {"target": "frazil_empty", "sources": []}):
            data = copy.deepcopy(original)
            data["modules"][0]["native"] = bad
            with self.assertRaisesRegex(ValueError, "requires native"):
                validate_modules(data)

    def test_optional_capabilities_can_be_added_independently(self):
        registry = self.native_only_registry()
        original = load_modules()["a1"]
        registry["c"]["cli"] = copy.deepcopy(original["cli"])
        self.assertEqual(expected_tests(["c"], registry=registry), {"frazil_current_c", "frazil_current_c_cli"})
        registry["c"].pop("cli")
        registry["c"]["performance"] = copy.deepcopy(original["performance"])
        registry["c"]["performance"]["target"] = "frazil_fixture_c_performance"
        validated = validate_modules({"schema_version": 2, "modules": list(registry.values())})
        with patch("current_modules.load_modules", return_value=validated):
            self.assertEqual(targets(["c"], "performance"), ["frazil_test_c_performance"])
            self.assertEqual(expected_tests(["c"]), {"frazil_current_c"})

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
