"""Regression checks for transitive dependency routing and conservative fallbacks."""
import unittest
from test_impact import MODULES, SPIKE, TOOLING, dependencies, executable_doc_diff, route, tooling_doc_diff


class ImpactTests(unittest.TestCase):
    def test_ownership_and_dependencies(self):
        self.assertIn((SPIKE / "dsp/DropletB1.h").as_posix(), dependencies([SPIKE / "tests/droplet_b2_tests.cpp"]))
        self.assertIn("b2", route([str(SPIKE / "dsp/DropletB1.h")])["active_modules"])
        self.assertEqual(route([str(SPIKE / "tests/bubble_a1_tests.cpp")])["active_modules"], ["a1"])
        self.assertEqual(route([str(SPIKE / "tests/flow_d1_source_probe.cpp")])["active_modules"], ["d1"])
        self.assertEqual(route([str(SPIKE / "render/render_main.cpp")])["active_modules"], list(MODULES))

    def test_archives_do_not_enter_current(self):
        for name in ("b1_cli_contract.py", "preview_session_tests.cpp", "flow_d1_convergence_test.py"):
            self.assertEqual(route([str(SPIKE / "tests" / name)])["active_modules"], [])

    def test_unknown_and_structural_fail_closed(self):
        for path in ("CMakeLists.txt", "tools/new.py", "CMakePresets.json"):
            self.assertEqual(route([path])["active_modules"], list(MODULES))
        for path in TOOLING:
            self.assertFalse(route([path])["build_required"])
            self.assertEqual(route([path], structural={path})["active_modules"], list(MODULES))

    def test_core_is_independent(self):
        result = route(["src/plugin/PluginProcessor.cpp", "tests/unit/test_main.cpp"])
        self.assertTrue(result["core_required"])
        self.assertEqual(result["active_modules"], [])

    def test_document_commands_and_tooling(self):
        self.assertFalse(route(["docs/TESTING.md"])["build_required"])
        self.assertFalse(executable_doc_diff("+Clarify the testing policy"))
        self.assertTrue(executable_doc_diff("+ctest --preset windows-debug-current-tests"))
        self.assertTrue(tooling_doc_diff("+python tools/test_test_impact.py"))
        self.assertFalse(tooling_doc_diff("+python tools/test_test_impact.py; cmake --build build"))
        result = route(["docs/TESTING.md"], executable_docs={"docs/TESTING.md"})
        self.assertEqual(result["active_modules"], list(MODULES))

    def test_shared_random_dependency(self):
        self.assertEqual(route(["src/dsp/primitives/RandomSource.cpp"])["active_modules"], list(MODULES))


if __name__ == "__main__":
    unittest.main()
