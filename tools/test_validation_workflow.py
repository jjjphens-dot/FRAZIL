"""Guard workflow entrypoints and cancellation boundaries without running CI."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]


class WorkflowTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fast = (ROOT / ".github/workflows/ci.yml").read_text(encoding="utf-8")
        cls.manual = (ROOT / ".github/workflows/validation.yml").read_text(encoding="utf-8")

    def test_daily_has_no_dispatch_full_or_diagnostics(self):
        self.assertNotIn("workflow_dispatch:", self.fast)
        self.assertNotIn("python tools/python_test_ab.py", self.fast)
        self.assertNotIn("validation-full:", self.fast)
        self.assertNotIn("CMakeLists.txt testdata/manifest.json", self.fast)
        self.assertIn("tools/plan_validation.py --context pr", self.fast)

    def test_independent_concurrency(self):
        self.assertIn("cancel-in-progress: true", self.fast)
        self.assertIn("group: frazil-fast-", self.fast)
        concurrency = self.manual.split("\nconcurrency:\n", 1)[1].split("\njobs:", 1)[0]
        self.assertIn("group: frazil-explicit-${{ github.ref }}", concurrency)
        self.assertNotIn("github.run_id", concurrency)
        self.assertNotIn("github.sha", concurrency)
        self.assertIn("cancel-in-progress: false", self.manual)
        self.assertNotIn("needs: ci-core", self.manual)
        self.assertNotIn("matrix:", self.manual)
        self.assertNotIn("  pull_request:", self.manual)
        self.assertNotIn("  push:", self.manual)
        for job in ("selected-validation", "d1-study"):
            header = self.manual.split(f"  {job}:", 1)[1].split("    steps:", 1)[0]
            self.assertIn("success() && !cancelled()", header)

    def test_diagnostic_only_explicit_success_not_cancelled(self):
        step = self.manual.split("      - name: Diagnose only", 1)[1].split("      - name:", 1)[0]
        self.assertIn("success() && !cancelled()", step)
        self.assertIn("needs.plan.outputs.purpose == 'diagnostic'", step)
        self.assertIn("--max-cases 4", step)
        self.assertNotIn("always()", step)
        self.assertEqual(self.manual.count("python tools/python_test_ab.py"), 1)

    def test_user_strings_are_environment_values_not_script_interpolation(self):
        for document in (self.fast, self.manual):
            for block in re.split(r"      - ", document):
                if "run:" in block:
                    script = block.split("run:", 1)[1]
                    self.assertNotIn("${{ inputs.hypothesis }}", script)
                    self.assertNotIn("${{ inputs.timeout_seconds }}", script)

    def test_study_no_longer_prepends_full_tests(self):
        study = self.manual.split("  d1-study:", 1)[1]
        self.assertIn("--target frazil_water_d1_research_tests", study)
        self.assertNotIn("ctest --preset windows-release", study)
        self.assertNotIn("full_validation", self.manual)

    def test_regressions_are_in_daily_policy(self):
        for name in ("test_plan_validation", "test_python_test_ab", "test_validation_workflow"):
            self.assertIn(name, self.fast)


if __name__ == "__main__":
    unittest.main()
