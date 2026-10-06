"""Build/Test separation and explicit expensive-purpose contracts."""
import unittest
from build_safe import build_command
from plan_validation import diagnostic_eligible, plan
from test_impact import route, SPIKE


class PlanTests(unittest.TestCase):
    def test_build_first_even_when_native_changes(self):
        result = plan(impact=route([str(SPIKE / "tests/droplet_b2_tests.cpp")]))
        self.assertTrue(result["build_required"])
        self.assertFalse(result["test_required"])
        self.assertEqual(result["active_modules"], ["b2"])
        self.assertEqual(result["test_preset"], "")
        for flag in ("memory_safety_required", "performance_required", "research_required", "historical_required"):
            self.assertFalse(result[flag])

    def test_explicit_union_deduplicates(self):
        result = plan(purpose="targeted", module="b2,a1,b2")
        self.assertEqual(result["active_modules"], ["a1", "b2"])
        self.assertEqual(result["test_label"], "^current-(a1|b2)-correctness$")
        command = build_command(result["test_build_preset"], 6, result["build_targets"])
        self.assertEqual(command[-3:], ["--target", "frazil_test_a1_current", "frazil_test_b2_current"])

    def test_ci_test_stage_only_affected_current(self):
        result = plan(context="pr", stage="test", impact=route([str(SPIKE / "tests/flow_d1_tests.cpp")]))
        self.assertTrue(result["test_required"])
        self.assertEqual(result["active_modules"], ["d1"])
        self.assertEqual(result["test_preset"], "ci-windows-debug-current-tests")
        self.assertFalse(plan(context="pr", stage="test", impact=route(["README.md"]))["test_required"])

    def test_auto_host_selection_has_executable_presets(self):
        result = plan(stage="test", impact=route(["src/plugin/PluginProcessor.cpp"]))
        self.assertTrue(result["core_required"])
        self.assertEqual(result["test_preset"], "windows-debug-core")
        self.assertEqual(result["configure_preset"], "windows-debug-host")

    def test_memory_and_performance_are_explicit(self):
        memory = plan(purpose="memory-safety", configuration="asan")
        perf = plan(purpose="performance", configuration="release")
        self.assertTrue(memory["memory_safety_required"])
        self.assertFalse(memory["performance_required"])
        self.assertEqual(perf["configure_preset"], "windows-release-performance")
        self.assertTrue(perf["performance_required"])
        self.assertFalse(perf["historical_required"])

    def test_reject_invalid_or_heavy_pr_requests(self):
        for request in (dict(purpose="targeted", module="c"), dict(purpose="memory-safety"),
                        dict(purpose="performance"), dict(context="pr", purpose="full"),
                        dict(context="dispatch"), dict(purpose="targeted", cancelled=True),
                        dict(purpose="diagnostic", failure="none", hypothesis="x")):
            with self.subTest(request=request), self.assertRaises(ValueError):
                plan(**request)
        with self.assertRaises(ValueError):
            build_command("windows-debug-build", 9)
        with self.assertRaises(ValueError):
            build_command("windows-debug-build", 6, ["FRAZIL_All", "unknown"])

    def test_archive_and_diagnostics_never_fallback(self):
        self.assertTrue(plan(purpose="historical")["historical_required"])
        self.assertEqual(plan(purpose="full")["configure_preset"], "windows-debug-all")
        self.assertTrue(plan(purpose="research", module="d1", configuration="release", study="latency")["research_required"])
        self.assertFalse(diagnostic_eligible("d1-timeout", requested=True))
        self.assertFalse(diagnostic_eligible("python-process", requested=True, cancelled=True))
        result = plan(purpose="diagnostic", diagnostic_test="render_cli", failure="python-process", hypothesis="import path")
        self.assertEqual(result["build_targets"], ["frazil_water_experiment_render"])
        self.assertFalse(result["test_required"])


if __name__ == "__main__":
    unittest.main()
