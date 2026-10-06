"""Reject incomplete, malformed or nonfinite observations without timing thresholds."""
import unittest
from performance_observation import expected_cases, validate


def fixture(kind):
    keys, cases = expected_cases(kind)
    columns = [*keys, "mean_us", "p95_us", "p99_us", "worst_us", "output_sum"]
    rows = [",".join(columns)]
    rows += [",".join((*case, "1000000", "2000000", "3000000", "4000000", "0"))
             for case in sorted(cases)]
    return "\n".join(rows)


class ObservationTests(unittest.TestCase):
    def test_complete_observations_without_budget(self):
        for kind in ("legacy", "a1", "b1", "d1", "d1-current"):
            validate(kind, fixture(kind))

    def test_missing_duplicate_nonfinite_and_schema(self):
        data = fixture("d1")
        mutations = [data.rsplit("\n", 1)[0], data + "\n" + data.splitlines()[-1],
                     data.replace("1000000", "nan", 1), data.replace("mean_us", "missing", 1),
                     data + ",extra", data.rsplit(",", 1)[0]]
        for broken in mutations:
            with self.subTest(broken=broken[-80:]), self.assertRaises(ValueError):
                validate("d1", broken)

    def test_current_b2_completeness_and_finite_values(self):
        data = "\n".join(f"B2 diagnostic rate={rate} fixture={fixture} eligible=1 mean_callback_us=2 max_callback_us=3 callback_budget_us=100"
                         for rate in (44100, 48000, 96000) for fixture in (0, 1))
        validate("b2", data)
        for bad in (data.rsplit("\n", 1)[0], data + "\n" + data.splitlines()[0], data.replace("mean_callback_us=2", "mean_callback_us=nan")):
            with self.assertRaises(ValueError):
                validate("b2", bad)

    def test_current_d1_excludes_b1(self):
        data = fixture("d1-current")
        self.assertNotIn("A1+B1+D1", data)
        self.assertEqual(len(data.splitlines()), 19)
        with self.assertRaises(ValueError):
            validate("d1-current", fixture("d1"))

    def test_product_scenarios(self):
        data = ""
        for name in ("steady-state", "parameter-retarget"):
            data += f"scenario={name}\nfinite_output_status=PASS\nstatus=PASS\nmeasured_blocks=100\n"
            for field in ("mean_callback_us", "p95_callback_us", "p99_callback_us", "worst_callback_us", "wall_seconds"):
                data += f"{field}=1\n"
        data += "denormal_finite_output_status=PASS\n"
        validate("product", data)
        for broken in (data.replace("parameter-retarget", "steady-state"),
                       data.replace("finite_output_status=PASS", "finite_output_status=FAIL"),
                       data.replace("measured_blocks=100", "measured_blocks=0")):
            with self.assertRaises(ValueError):
                validate("product", broken)


if __name__ == "__main__":
    unittest.main()
