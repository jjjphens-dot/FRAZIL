"""Independent checks of the offline numerical evidence machinery."""
import sys
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'render'))
from flow_d1_remediation_study import Candidate, continuous_path, fourier_reference, metrics, native_cluster_signal, source_envelope

PROBE = sys.argv.pop(1) if len(sys.argv) > 1 else None


class NumericalEvidenceTests(unittest.TestCase):
    def test_cluster_uses_native_float_rounding(self):
        a = np.array([[1.,-1.]])
        b = np.array([[2.**-25,-2.**-25]])
        # The contribution is below half a float ULP, but survives double addition.
        np.testing.assert_array_equal(native_cluster_signal(a,b),a)
        self.assertFalse(np.array_equal(a+b,a))

    @unittest.skipUnless(PROBE, 'source probe executable supplied by CTest')
    def test_actual_source_probe(self):
        local = Path(__file__).resolve().parents[4] / 'build' / 'flow-d1-latency'
        local.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=local, prefix='numerical-') as temp:
            root = Path(temp) / 'sources'
            subprocess.run([PROBE, str(root)],check=True,capture_output=True,text=True)
            authority = json.loads((root/'authority.json').read_text())
            self.assertGreater(source_envelope(authority),23000)
            for rate in (44100,48000,96000):
                for profile in ('reference','edge','overlap-reference','sustained-edge'):
                    data = np.loadtxt(root/f'{rate}-{profile}.csv',delimiter=',',skiprows=1)
                    self.assertEqual(data.shape,(rate,5))
                    self.assertTrue(np.isfinite(data).all())
                    self.assertGreater(np.linalg.norm(data[:,1]),0)
                    self.assertGreater(np.linalg.norm(data[:,3]),0)
                    if profile.startswith('overlap'):
                        self.assertGreater(np.count_nonzero((data[:,1] != 0) & (data[:,3] != 0)),0)
                    self.assertLessEqual(max(abs(np.diff(data[:,0])))*rate,1+1e-9)
                    np.testing.assert_array_equal(data[:,2],-.5*data[:,1])
                    np.testing.assert_array_equal(data[:,4],-.5*data[:,3])

    def test_known_half_sample_response(self):
        kernel = Candidate('lagrange3', 4, 48000, 21600)
        taps, weights = kernel.coefficients([.5])
        np.testing.assert_array_equal(taps, [[0, 1, 2, 3]])
        np.testing.assert_array_equal(weights, [[.3125, .9375, -.3125, .0625]])
        # At Nyquist the response is -1 for this stencil, unlike the ideal -j.
        magnitude, phase, _, error = kernel.response(.5, np.array([24000.]), 48000)
        self.assertAlmostEqual(magnitude[0], 0.)
        self.assertAlmostEqual(abs(phase[0]), np.pi/2)
        self.assertAlmostEqual(error[0], np.sqrt(2))

    def test_optimized_representations_against_symmetric_two_tap_solution(self):
        # For half-sample delay, a full-band two-tap DC-constrained LS fit has
        # equal coefficients by symmetry. This checks the optimization and Farrow
        # representations against a closed-form solution, not against each other.
        for name in ('vfd-ls2','farrow7-fir2'):
            kernel = Candidate(name,2,48000,24000)
            taps,weights = kernel.coefficients([.5])
            np.testing.assert_array_equal(taps,[[0,1]])
            np.testing.assert_allclose(weights,[[.5,.5]],atol=1e-13)
            frequencies = np.array([4000.,12000.,18000.])
            mag,phase,group,_ = kernel.response(.5,frequencies,48000)
            np.testing.assert_allclose(mag,20*np.log10(np.cos(np.pi*frequencies/48000)),atol=1e-12)
            np.testing.assert_allclose(phase,0,atol=1e-13)
            np.testing.assert_allclose(group,0,atol=1e-13)

    def test_ideal_integer_delay_and_group_delay(self):
        for name, taps in [('lagrange3',4),('lagrange15',16),('sinc129',129)]:
            kernel = Candidate(name,taps,48000,21600)
            values = kernel.response(2.,np.array([1000.,8000.,18000.]),48000)
            for value in values:
                np.testing.assert_allclose(value,0,atol=2e-13)
            impulse = np.zeros(32)
            impulse[4] = 1
            result = kernel.apply(impulse,np.full(32,2.))[:,0]
            self.assertAlmostEqual(result[6],1.)
            np.testing.assert_allclose(np.delete(result,6),0,atol=1e-15)

    def test_sinc_against_analytic_fractional_delay(self):
        kernel = Candidate('sinc1025',1025,44100,19845)
        mag,phase,group,error = kernel.response(.5,np.array([4000.,16000.,19845.]),44100)
        self.assertLess(max(error),1e-6)
        self.assertLess(max(abs(group)),1e-9)

    def test_source_equation_detects_drift(self):
        spec = dict(gamma=1.4,pressure_pa=101325,density_kg_m3=998,
                    a1_min_radius_mm=.2,b1_min_radius_mm=.2,a1_f0_hz=1,b1_f0_hz=1)
        with self.assertRaises(ValueError):
            source_envelope(spec)

    def test_cross_rate_path_and_speed(self):
        common = np.arange(9,141)/300
        reference = continuous_path(common)
        for rate in (44100,48000,96000):
            index = np.rint(common*rate).astype(int)
            np.testing.assert_allclose(index/rate,common,atol=1e-16)
            np.testing.assert_allclose(continuous_path(index/rate),reference,atol=1e-15)
        t = np.arange(96000)/96000
        path = continuous_path(t)
        self.assertLessEqual(np.max(abs(np.diff(path)))*96000,1.)
        self.assertGreaterEqual(path.min(),0.)
        self.assertLessEqual(path.max(),.05)

    def test_metrics_do_not_hide_gain_error(self):
        signal = np.array([[1.,-.5],[0.,0.],[-.5,.25]])
        values = metrics(2*signal,signal)
        self.assertAlmostEqual(values['relative_rms_error'],1.)
        self.assertAlmostEqual(values['peak_normalized_error'],1.)
        self.assertAlmostEqual(values['spectral_magnitude_relative_error'],1.)

    def test_fourier_reference_against_sinc_impulse(self):
        signal = np.zeros(4096)
        signal[2048] = 1
        delay = np.full(len(signal),.5)
        result = fourier_reference(signal,delay,8,16)[:,0]
        expected = np.sinc(np.arange(len(signal))-2048-.5)
        np.testing.assert_allclose(result[2000:2100],expected[2000:2100],atol=6e-8)
        integer = fourier_reference(signal,np.full(len(signal),2.),8,16)[:,0]
        self.assertAlmostEqual(integer[2050],1.)


if __name__ == '__main__':
    unittest.main()
