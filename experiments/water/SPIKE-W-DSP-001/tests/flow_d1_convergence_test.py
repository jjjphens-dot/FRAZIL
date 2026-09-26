"""Independent fixed-character and policy checks; no listening acceptance."""

import sys
import argparse
import csv
import subprocess
import tempfile
import unittest
from pathlib import Path
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "render"))
from flow_d1_convergence_models import FixedConditioner, registry
from flow_d1_latency_models import GuardKernel, Conditioner
from flow_d1_latency_study import nonlinear_filter_probe
from flow_d1_convergence_cross_rate import analytic_policy_pairs

parser = argparse.ArgumentParser(add_help=False)
parser.add_argument("--probe", type=Path)
options, remaining = parser.parse_known_args()
sys.argv = [sys.argv[0]] + remaining
from flow_d1_convergence_study import select_policies, kernel_authority


class ConvergenceTests(unittest.TestCase):
    def test_registry_and_96_raw_priority(self):
        self.assertEqual(
            [len(list(registry(r))) for r in (44100, 48000, 96000)], [11, 11, 2]
        )
        self.assertEqual(
            [c.name for c in registry(96000)], ["raw-control", "butter4-fc20000"]
        )

    def test_butterworth_against_closed_form(self):
        for rate in (44100, 48000, 96000):
            for order in (4, 6):
                c = FixedConditioner(rate, f"butter{order}", 20000)
                f = np.array([0.0, 1000.0, 16000.0, 18000.0, 20000.0])
                ratio = np.tan(np.pi * f / rate) / np.tan(np.pi * 20000 / rate)
                np.testing.assert_allclose(
                    abs(c.response(f)) ** 2,
                    1 / (1 + ratio ** (2 * order)),
                    rtol=1e-12,
                    atol=1e-14,
                )

    def test_bessel_against_analog_prototype(self):
        # Independent reverse-Bessel polynomial: s^4+10s^3+45s^2+105s+105.
        from scipy.optimize import brentq

        polynomial = np.array([1.0, 10.0, 45.0, 105.0, 105.0])
        mag = lambda w: abs(105 / np.polyval(polynomial, 1j * w))
        w3 = brentq(lambda w: mag(w) - 1 / np.sqrt(2), 0, 10)
        for rate in (44100, 48000):
            c = FixedConditioner(rate, "bessel4", 18000)
            f = np.array([0.0, 1000.0, 16000.0, 18000.0, 20000.0])
            analog = w3 * np.tan(np.pi * f / rate) / np.tan(np.pi * 18000 / rate)
            np.testing.assert_allclose(
                c.response(f), 105 / np.polyval(polynomial, 1j * analog), atol=2e-13
            )

    def test_alignment_and_no_source_change(self):
        x = np.zeros((512, 2))
        x[200] = [1.0, -0.5]
        for c in registry(48000):
            y = c.apply(x)
            self.assertTrue(np.all(y[:200] == 0))
            aligned = c.aligned(x)
            np.testing.assert_allclose(
                y[c.latency :], aligned[: len(x) - c.latency], atol=0
            )
            if c.fir is not None:
                np.testing.assert_allclose(c.fir, c.fir[::-1], atol=1e-16)
                self.assertAlmostEqual(sum(c.fir), 1.0, places=13)
        np.testing.assert_array_equal(FixedConditioner(48000).aligned(x), x)

    def test_policy_preserves_filter_rejection(self):
        rows = [
            dict(
                rate=r,
                conditioner=n,
                kernel="hann-g32",
                numerical_pass=True,
                core_magnitude_pass=not (r == 96000 and n != "raw-control"),
                total_latency_samples=32,
            )
            for r in (44100, 48000, 96000)
            for n in ("raw-control", "butter4-fc20000")
        ]
        policies = select_policies(rows)
        self.assertEqual({r["policy"] for r in policies}, {"S0", "S1-butter4", "S2"})
        rejected = next(
            r for r in policies if r["policy"] == "S2" and r["rate"] == 96000
        )
        self.assertFalse(rejected["qualified"])
        self.assertEqual(rejected["kernel"], "hann-g32")

    def test_zero_path_shared_conditioner(self):
        x = np.random.default_rng(42).normal(size=(1024, 2))
        for c in registry(48000):
            off = c.aligned(x)
            for family in ("lagrange", "hann", "kaiser"):
                on = GuardKernel(family, 16).apply(off, np.zeros(len(x)))
                np.testing.assert_array_equal(on, off)

    @unittest.skipUnless(options.probe, "native source probe not supplied")
    def test_native_event_provenance(self):
        root = Path(__file__).resolve().parents[4]
        with tempfile.TemporaryDirectory(
            prefix="convergence-events-", dir=root / "build"
        ) as temporary:
            output = Path(temporary) / "sources"
            subprocess.run(
                [str(options.probe.resolve()), str(output)],
                check=True,
                capture_output=True,
                timeout=120,
            )
            files = list(output.glob("*-events.csv"))
            self.assertEqual(len(files), 12)
            for path in files:
                with path.open() as stream:
                    rows = list(csv.DictReader(stream))
                for source in ("A1", "B1"):
                    events = [r for r in rows if r["source"] == source]
                    self.assertTrue(events)
                    self.assertEqual(
                        [int(r["id"]) for r in events], list(range(1, len(events) + 1))
                    )
                    times = [float(r["time_s"]) for r in events]
                    self.assertEqual(times, sorted(times))
                    for event in events:
                        self.assertTrue(0 <= float(event["time_s"]) < 1)
                        self.assertTrue(float(event["radius_m"]) > 0)
                        self.assertTrue(float(event["frequency_hz"]) > 0)
                        self.assertIn(event["admitted"], ("0", "1"))
                        if source == "B1":
                            self.assertGreaterEqual(
                                float(event["due_time_s"]), float(event["time_s"])
                            )
                        else:
                            self.assertEqual(event["due_time_s"], "")

    def test_nonlinear_probe_preserves_historical_values(self):
        root = Path(__file__).resolve().parents[4]
        c = Conditioner(96000, 18000, 24000)
        with (
            root / "docs/evidence/WATER_FLOW_D1_LATENCY_NONLINEAR.csv"
        ).open() as stream:
            old = {
                r["nonlinearity"]: r
                for r in csv.DictReader(stream)
                if r["conditioner"] == c.name
            }
        self.assertEqual(len(old), 2)
        for row in nonlinear_filter_probe(c):
            for key in (
                "audible_raw_energy",
                "audible_conditioned_energy",
                "audible_raw_alias_energy",
                "audible_conditioned_alias_energy",
            ):
                expected = float(old[row["nonlinearity"]][key])
                self.assertAlmostEqual(
                    row[key], expected, delta=max(1.0, abs(expected)) * 1e-12
                )

    def test_mixed_kernel_cross_rate_against_ideal(self):
        cells = [
            dict(policy="control", rate=rate, conditioner="raw-control", kernel=kernel)
            for rate, kernel in (
                (44100, "kaiser-g32"),
                (48000, "kaiser-g16"),
                (96000, "kaiser-g64"),
            )
        ]
        rows = analytic_policy_pairs(cells)
        self.assertEqual(len(rows), 16)
        core = [r for r in rows if r["frequency_hz"] <= 16000]
        self.assertEqual(len(core), 12)
        self.assertTrue(all(r["tier_a_pair_pass"] for r in core))
        for r in rows:
            # Identity conditioning: entire chain difference equals kernel error.
            self.assertAlmostEqual(
                r["kernel_pair_error_rms"], r["whole_chain_difference_rms"], places=13
            )

    def test_frozen_kernel_coverage(self):
        root = Path(__file__).resolve().parents[4] / "docs/evidence"
        allowed = kernel_authority(root)
        self.assertNotIn((44100, "lagrange-g0"), allowed)
        self.assertIn((96000, "hann-g32"), allowed)


if __name__ == "__main__":
    unittest.main()
