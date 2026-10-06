"""Exercise diagnostic limits with mocks and a harmless short-lived child."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

import python_test_ab as ab


class DiagnosticTests(unittest.TestCase):
    def test_one_test_and_bounded_case_ids(self):
        cases = ab.case_ids("testdata", 4)
        self.assertEqual(len(cases), 4)
        self.assertEqual(len({case[0] for case in cases}), 4)
        self.assertTrue(all("render" not in case[0] for case in cases))
        self.assertEqual(len(ab.case_ids("render_cli", 2)), 2)
        for count in (0, 5):
            with self.assertRaises(ValueError):
                ab.case_ids("testdata", count)

    def test_registered_environment_preserved(self):
        parent = {"PATH": "old", "PYTHONNOUSERSITE": "1"}
        env = ab.direct_environment(parent, {"ENVIRONMENT": ["A=B"],
            "ENVIRONMENT_MODIFICATION": ["TEMP=set:local-temp", "PATH=path_list_prepend:runtime"]})
        self.assertEqual(parent["PATH"], "old")
        self.assertEqual(env["PATH"], "runtime" + os.pathsep + "old")
        self.assertEqual(env["TEMP"], "local-temp")
        self.assertEqual(env["PYTHONNOUSERSITE"], "1")
        with self.assertRaises(ValueError):
            ab.direct_environment({}, {"ENVIRONMENT_MODIFICATION": ["A=unknown:B"]})

    def test_timeout_and_interrupt_stop_process_tree(self):
        for error in (subprocess.TimeoutExpired("fake", 1), KeyboardInterrupt()):
            process = Mock()
            process.wait.side_effect = error
            with tempfile.TemporaryDirectory() as folder, patch.object(ab.subprocess, "Popen", return_value=process), \
                    patch.object(ab, "stop_tree") as stop:
                kwargs = dict(cwd=Path(folder), env={}, log=Path(folder) / "log", timeout=1)
                if isinstance(error, KeyboardInterrupt):
                    with self.assertRaises(KeyboardInterrupt):
                        ab.run_case(["fake"], **kwargs)
                else:
                    self.assertEqual(ab.run_case(["fake"], **kwargs), 124)
                stop.assert_called_once_with(process)

    def test_real_process_timeout(self):
        with tempfile.TemporaryDirectory() as folder:
            result = ab.run_case([sys.executable, "-c", "import time; time.sleep(20)"],
                                 cwd=Path(folder), env=dict(os.environ), log=Path(folder) / "log", timeout=1)
            self.assertEqual(result, 124)

    def test_cancel_or_timeout_does_not_start_next_case(self):
        test = dict(name=ab.TESTS["testdata"], command=[sys.executable, "fake.py"],
                    properties=[dict(name="WORKING_DIRECTORY", value=str(ab.ROOT))])
        for outcome in (124, KeyboardInterrupt()):
            with tempfile.TemporaryDirectory(dir=ab.ROOT / "build") as folder:
                args = ["diagnostic", "--preset", "windows-debug-full", "--test", "testdata",
                        "--failure", "python-process", "--hypothesis", "site paths differ",
                        "--output", str(Path(folder) / "results")]
                with patch.object(sys, "argv", args), \
                        patch.object(ab.subprocess, "check_output", side_effect=[
                            __import__("json").dumps({"tests": [test]}), "test-head", "test-python"]), \
                        patch.object(ab, "run_case", side_effect=[outcome]) as runner:
                    self.assertEqual(ab.main(), 130 if isinstance(outcome, KeyboardInterrupt) else 124)
                    self.assertEqual(runner.call_count, 1)


if __name__ == "__main__":
    (ab.ROOT / "build").mkdir(exist_ok=True)
    unittest.main()
