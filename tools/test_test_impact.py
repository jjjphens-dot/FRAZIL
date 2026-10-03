"""Regression checks for transitive dependency routing and conservative fallbacks."""
import unittest
from test_impact import MODULES, SPIKE, dependencies, executable_doc_diff, route


class ImpactTests(unittest.TestCase):
    def test_real_transitive_dependencies(self):
        closure = dependencies([SPIKE / "tests/droplet_b2_tests.cpp"])
        self.assertIn((SPIKE / "dsp/DropletB1.h").as_posix(), closure)
        self.assertIn("src/dsp/primitives/RandomSource.h", closure)
        self.assertIn("water-b2", route([str(SPIKE / "dsp/DropletB1.h")])["modules"])

    def test_shared_renderer_consumers(self):
        self.assertEqual(route([str(SPIKE / "render/render_main.cpp")])["modules"],
                         ["water-common", "water-b1", "water-d1", "water-protect"])

    def test_private_tests(self):
        self.assertEqual(route([str(SPIKE / "tests/bubble_a1_tests.cpp")])["modules"], ["water-a1"])
        result = route([str(SPIKE / "tests/preview_session_tests.cpp")])
        self.assertEqual(result["modules"], ["preview"])
        self.assertFalse(result["research_dependencies"])

    def test_unknown_deleted_or_infrastructure(self):
        for path in ("CMakeLists.txt", "tools/new_tool.py", str(SPIKE / "dsp/Deleted.h"),
                     "requirements-dsp.txt", ".github/workflows/ci.yml"):
            self.assertEqual(route([path])["modules"], list(MODULES))

    def test_document_and_core(self):
        self.assertEqual(route(["docs/TESTING.md", "tests/unit/test_main.cpp", "src/plugin/PluginProcessor.cpp"])["modules"], [])
        self.assertFalse(route(["docs/TESTING.md"])["testdata"])

    def test_linked_translation_units(self):
        self.assertEqual(route(["src/dsp/primitives/RandomSource.cpp"])["modules"], list(MODULES))
        self.assertEqual(route(["src/dsp/primitives/LinearSmoother.cpp"])["modules"], ["preview"])

    def test_testdata(self):
        self.assertTrue(route(["testdata/input/input.wav"])["testdata"])
        self.assertTrue(route(["tools/generate_testdata.py"])["testdata"])

    def test_docs_only_no_windows(self):
        for path in ("docs/TESTING.md", "README.md", "AGENTS.md", "docs/DOCUMENT_GOVERNANCE.md",
                     ".github/ISSUE_TEMPLATE/bug.yml", ".github/pull_request_template.md"):
            self.assertFalse(route([path])["core_required"], path)
        self.assertFalse(executable_doc_diff("-Daily checks are useful\n+Daily checks are required"))

    def test_executable_contract_document(self):
        self.assertTrue(executable_doc_diff("-ctest --preset windows-debug-core\n+ctest --preset windows-debug-full"))
        result = route(["docs/TESTING.md"], executable_docs={"docs/TESTING.md"})
        self.assertTrue(result["core_required"])
        self.assertEqual(result["modules"], list(MODULES))

    def test_core_executable_changes(self):
        for path in ("CMakePresets.json", ".github/workflows/ci.yml", "tools/build_safe.py",
                     "tests/unit/test_main.cpp", "src/plugin/PluginProcessor.cpp", "docs/new_tool.py"):
            self.assertTrue(route([path])["core_required"], path)

    def test_python_owners(self):
        for name, expected in {
            "b1_cli_smoke": ["water-b1"], "b1_cli_contract": ["water-b1"],
            "b1_cli_support": ["water-b1"], "b1_listening_pack_validation": ["water-b1"],
            "d1_cli_smoke": ["water-d1"], "d1_cli_contract": ["water-d1"],
            "d1_native_oracle_validation": ["water-d1"],
            "render_cli_smoke": ["water-common"],
            "render_cli_contract": ["water-common", "water-protect"],
            "render_cli_full_matrix": ["water-common", "water-protect"],
            "cli_support": ["water-common", "water-b1", "water-d1", "water-protect"],
        }.items():
            self.assertEqual(route([str(SPIKE / "tests" / (name + ".py"))])["modules"], expected, name)

    def test_preview_helper(self):
        self.assertEqual(route([str(SPIKE / "preview/TimeValue.h")])["modules"], ["preview"])

    def test_python_new_deleted_fail_closed(self):
        self.assertEqual(route([str(SPIKE / "tests/unknown_new.py")])["modules"], list(MODULES))
        path = str(SPIKE / "tests/b1_cli_smoke.py").replace("\\", "/")
        # Even a recognized path is broad on addition/deletion, not just on content edits.
        self.assertEqual(route([path], structural={path})["modules"], list(MODULES))


if __name__ == "__main__":
    unittest.main()
