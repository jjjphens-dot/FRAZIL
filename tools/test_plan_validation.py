"""Regression checks for bounded, explicit validation planning (no test workloads)."""
import unittest

from build_safe import build_command
from plan_validation import diagnostic_eligible, plan
from test_impact import MODULES, SPIKE, route


class PlanTests(unittest.TestCase):
    def test_docs_and_tooling_never_invalidate_performance(self):
        for paths, expected in ((["README.md"], "docs-only"),
                                (["tools/test_impact.py", "tools/test_test_impact.py"], "tooling")):
            result = plan(impact=route(paths))
            self.assertEqual(result["purpose"], expected)
            for flag in ("native_build_required", "performance_required", "diagnostics_required"):
                self.assertFalse(result[flag])
            self.assertIn("test_test_impact", result["checks"])

    def test_diagnostic_trigger_is_never_automatic(self):
        for failure in ("none", "full-pass", "d1-timeout", "python-process"):
            self.assertFalse(diagnostic_eligible(failure, requested=False))
        self.assertFalse(diagnostic_eligible("d1-timeout", requested=True))
        self.assertTrue(diagnostic_eligible("python-process", requested=True))
        self.assertFalse(diagnostic_eligible("python-process", requested=True, cancelled=True))
        self.assertFalse(plan(context="dispatch", purpose="full", module="all")["diagnostics_required"])

    def test_one_d1_asan_fast_path(self):
        result = plan(context="dispatch", purpose="targeted", module="water-d1", configuration="asan")
        self.assertEqual(result["modules"], ["water-d1"])
        self.assertEqual(result["test_label"], "^fast-water-d1$")
        self.assertEqual(result["test_preset"], "windows-asan-fast")
        self.assertFalse(result["performance_required"])
        self.assertFalse(result["testdata"])
        self.assertEqual(result["build_target"], "frazil_water_d1_test_group")
        command = build_command(result["build_preset"], 6, result["build_target"])
        self.assertEqual(command[-2:], ["--target", "frazil_water_d1_test_group"])
        self.assertIn("--parallel", command)

    def test_explicit_full_one_configuration(self):
        result = plan(context="dispatch", purpose="full", module="all", configuration="release")
        self.assertEqual(result["test_preset"], "windows-release-full")
        self.assertEqual(result["modules"], list(MODULES))
        self.assertTrue(result["performance_required"])
        self.assertTrue(result["testdata"])
        self.assertFalse(result["diagnostics_required"])

    def test_single_diagnostic(self):
        result = plan(context="dispatch", purpose="diagnostic", module="core", configuration="release",
                      diagnostic_test="testdata", failure="python-process", hypothesis="user-site imports differ")
        self.assertEqual(result["case_limit"], 4)
        self.assertEqual(result["test_preset"], "windows-release-full")
        self.assertFalse(result["native_build_required"])
        self.assertEqual(result["build_preset"], "")
        self.assertFalse(result["performance_required"])

    def test_renderer_builds_renderer_only(self):
        result = plan(purpose="diagnostic", module="water-common", diagnostic_test="render_cli",
                      failure="interpreter", hypothesis="test configured site versus no-user-site")
        self.assertEqual(result["build_target"], "frazil_water_experiment_render")
        self.assertEqual(result["case_limit"], 4)

    def test_invalid_or_unimplemented_requests_fail_closed(self):
        for request in (
            dict(context="dispatch"), dict(context="pr", purpose="full", module="all"),
            dict(purpose="full", module="water-d1"), dict(purpose="targeted", cancelled=True),
            dict(purpose="performance"), dict(purpose="memory-safety"),
            dict(purpose="targeted", diagnostic_test="testdata"),
            dict(purpose="research", module="water-d1", configuration="asan", study="latency"),
            dict(purpose="diagnostic", diagnostic_test="testdata", failure="python-process", hypothesis=""),
            dict(purpose="diagnostic", diagnostic_test="testdata", failure="d1-timeout", hypothesis="x"),
            dict(purpose="diagnostic", diagnostic_test="testdata", failure="python-process", hypothesis="x", timeout_seconds=601),
        ):
            with self.subTest(request=request), self.assertRaises(ValueError):
                plan(**request)

    def test_unknown_infrastructure_remains_broad_but_not_full(self):
        result = plan(impact=route(["tools/unknown.py"]))
        self.assertEqual(result["modules"], list(MODULES))
        self.assertFalse(result["performance_required"])
        self.assertFalse(result["diagnostics_required"])

    def test_b1_owned_test_selects_b1(self):
        result = plan(impact=route([str(SPIKE / "tests/b1_cli_contract.py")]))
        self.assertEqual(result["modules"], ["water-b1"])
        self.assertFalse(result["performance_required"])

    def test_d1_probe_does_not_invalidate_a1_performance(self):
        result = plan(impact=route([str(SPIKE / "tests/flow_d1_source_probe.cpp")]))
        self.assertEqual(result["modules"], ["water-d1"])
        self.assertFalse(result["performance_required"])

    def test_research_does_not_schedule_daily_or_full(self):
        result = plan(purpose="research", module="water-d1", configuration="release", study="latency")
        self.assertEqual(result["build_target"], "frazil_water_d1_research_tests")
        self.assertEqual(result["test_preset"], "")
        self.assertFalse(result["performance_required"])
        self.assertFalse(result["core_required"])

    def test_scoped_build_still_refuses_unsafe_jobs_or_unknown_target(self):
        with self.assertRaises(ValueError):
            build_command("windows-asan-fast", 9, "frazil_water_d1_test_group")
        with self.assertRaises(ValueError):
            build_command("windows-asan-fast", 6, "arbitrary-target")


if __name__ == "__main__":
    unittest.main()
