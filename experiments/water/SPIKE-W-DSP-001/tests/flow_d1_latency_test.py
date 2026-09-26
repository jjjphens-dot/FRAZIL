"""Independent analytic checks for guard, conditioning and interference evidence."""
import sys
import unittest
from pathlib import Path

import numpy as np
from scipy import signal

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'render'))
from flow_d1_latency_models import Conditioner, GuardKernel, static_errors


class LatencyEvidenceTests(unittest.TestCase):
    def test_guard_availability_and_integer_identity(self):
        rng = np.random.default_rng(4)
        x = rng.normal(size=(512, 2))
        for guard in (8, 16, 32, 64):
            for family in ('lagrange', 'hann', 'kaiser'):
                kernel = GuardKernel(family, guard)
                np.testing.assert_array_equal(kernel.apply(x, np.zeros(len(x))), x)
                causal = kernel.causal_output(x, np.zeros(len(x)))
                np.testing.assert_array_equal(causal[guard:], x)
                self.assertTrue(np.all(causal[:guard] == 0))
                taps, _ = kernel.coefficients([0, .5, 1., 3.23])
                self.assertTrue(np.all(taps+guard >= 0))

    def test_barycentric_against_product_formula(self):
        kernel = GuardKernel('lagrange', 8)
        taps, weights = kernel.coefficients([.5, 1.01])
        for row, delay in enumerate((.5, 1.01)):
            for k in range(17):
                exact = np.prod([(delay-taps[row, j])/(taps[row, k]-taps[row, j])
                                 for j in range(17) if j != k])
                self.assertAlmostEqual(weights[row, k], exact, places=13)

    def test_frequency_oracle_integer(self):
        for family in ('lagrange', 'hann', 'kaiser'):
            error = static_errors(GuardKernel(family, 16), 48000,
                                  np.array([1000., 16000., 20000.]), np.array([0., 1., 2.]))
            np.testing.assert_allclose(error, 0, atol=1e-12)

    def test_no_future_input_leak(self):
        x = np.random.default_rng(3).normal(size=(512, 2))
        other = x.copy()
        other[250:] *= 13
        delay = np.linspace(0, 1.6, len(x))
        for family in ('lagrange', 'hann', 'kaiser'):
            k = GuardKernel(family, 16)
            np.testing.assert_array_equal(k.causal_output(x, delay)[:250],
                                          k.causal_output(other, delay)[:250])

    def test_physical_clock_not_shifted_by_guard(self):
        rate = 48000
        t = np.arange(4096)/rate
        d = .5+.4*np.sin(2*np.pi*100*t)
        x = np.sin(2*np.pi*4000*t)
        k = GuardKernel('kaiser', 32)
        y = k.causal_output(x, d)[32:, 0]
        expected = np.sin(2*np.pi*4000*(t-d/rate))
        self.assertLess(max(abs(y[128:-128]-expected[128:-128])), 1e-4)
        wrong = np.sin(2*np.pi*4000*(t-np.roll(d, -32)/rate))
        self.assertGreater(max(abs(y[128:-128]-wrong[128:-128])), .03)

    def test_pure_delay_and_comb_are_distinct(self):
        tau = .0001
        frequencies = np.array([0., 5000., 10000., 15000.])
        h = np.exp(-2j*np.pi*frequencies*tau)
        np.testing.assert_allclose(abs(h), 1, atol=1e-15)
        np.testing.assert_allclose(abs(1+h), [2, 0, 2, 0], atol=2e-15)
        np.testing.assert_allclose(abs(.1*(h-1)), [0, .2, 0, .2], atol=2e-15)

    def test_fir_impulse_latency_and_symmetry(self):
        c = Conditioner(48000, 18000, 20000, 129)
        impulse = np.r_[1., np.zeros(255)]
        y = c.apply(impulse)
        self.assertEqual(c.latency, 64)
        self.assertEqual(np.argmax(abs(y)), 64)
        np.testing.assert_allclose(y[:129], y[:129][::-1], atol=1e-15)
        self.assertGreater(np.sum(y[:64]**2), 0)

    def test_cubic_ultrasonic_foldback_closed_form(self):
        rate = 96000
        t = np.arange(rate)/rate
        x = .25*np.sin(2*np.pi*37000*t)
        # sin^3(wt)=(3sin(wt)-sin(3wt))/4;111 kHz folds to15 kHz.
        amplitude = 2*abs(np.fft.rfft(x**3)[15000])/rate
        self.assertAlmostEqual(amplitude, .25**3/4, places=13)

    def test_iir_analytic_butterworth_power(self):
        c = Conditioner(48000, 16000, 20000)
        _, critical = signal.buttord(16000, 20000, .1, 60, fs=48000)
        f = np.array([1000., 16000., 19000., 20000.])
        warped = np.tan(np.pi*f/48000)/np.tan(np.pi*critical/48000)
        exact = 1/(1+warped**(2*c.order))
        np.testing.assert_allclose(abs(c.response(f))**2, exact, atol=1e-13)
        impulse = np.zeros(4096)
        impulse[1024] = 1
        self.assertTrue(np.all(c.apply(impulse)[:1024] == 0))

    def test_conditioned_identity_baseline(self):
        x = np.random.default_rng(1).normal(size=(4096, 2))
        for c in (Conditioner(48000, 16000, 20000), Conditioner(48000, 18000, 20000, 129)):
            conditioned = c.aligned(x)
            k = GuardKernel('kaiser', 16)
            off = np.pad(conditioned, ((16, 0), (0, 0)))
            on = k.causal_output(conditioned, np.zeros(len(x)))
            np.testing.assert_array_equal(off, on)

    def test_fir_delay_does_not_retime_physical_trajectory(self):
        rate = 48000
        t = np.arange(4096)/rate
        x = np.sin(2*np.pi*4000*t)
        c = Conditioner(rate, 18000, 20000, 129)
        k = GuardKernel('kaiser', 32)
        delay = .5+.4*np.sin(2*np.pi*100*t)
        conditioned = c.aligned(x)
        aligned = k.apply(conditioned, delay)[:, 0]
        # FIR is symmetric: after removing its64-sample phase, this perpetual sine
        # has only the measured real amplitude response, no hidden trajectory shift.
        amplitude = (c.response(np.array([4000.]))[0] *
                     np.exp(2j*np.pi*4000*c.latency/rate)).real
        oracle = amplitude*np.sin(2*np.pi*4000*(t-delay/rate))
        self.assertLess(max(abs(aligned[256:-256]-oracle[256:-256])), 1e-4)
        causal = np.pad(aligned, (c.latency+k.guard, 0))
        self.assertEqual(len(causal)-len(x), 96)

    def test_combined_filter_guard_causality(self):
        x = np.random.default_rng(9).normal(size=(1024, 2))
        changed = x.copy()
        changed[512:] += 17
        delay = np.linspace(0, 1.6, len(x))
        for c in (Conditioner(48000, 18000, 20000), Conditioner(48000, 18000, 20000, 129)):
            k = GuardKernel('kaiser', 16)
            outputs = [np.pad(k.apply(c.aligned(s), delay), ((c.latency+k.guard, 0), (0, 0)))
                       for s in (x, changed)]
            np.testing.assert_array_equal(outputs[0][:512], outputs[1][:512])


if __name__ == '__main__':
    unittest.main()
