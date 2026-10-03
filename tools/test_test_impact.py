"""Regression checks for transitive dependency routing and conservative fallbacks."""
import unittest
from test_impact import MODULES, SPIKE, dependencies, route


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


if __name__ == "__main__":
    unittest.main()
