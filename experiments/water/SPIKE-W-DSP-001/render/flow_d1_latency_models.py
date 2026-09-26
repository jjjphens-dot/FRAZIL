"""Offline ENGINEERING candidates; no production or realtime dependency.

Authority: EXP-W-FD-002. Guard shifts availability, never physical path/time.
Candidate.apply returns guard-aligned output; causal_output exposes actual latency.
"""
import math

import numpy as np
from scipy import signal

from flow_d1_remediation_study import Candidate

RATES = (44100, 48000, 96000)
GUARDS = (0, 8, 16, 32, 64)
FIXED_HZ = np.array([1000., 2000., 4000., 8000., 12000., 16000., 18000., 20000.])
TIME_ERROR = .05 * 3.2 / 1484**2
EPSILON = 2 * np.sin(np.pi * 16000 * TIME_ERROR)


class GuardKernel(Candidate):
    """Centred reconstruction at physical time, delayed by guard in a causal host.

    For floor(d)=q, offsets q-g..q+g need at most g future samples. Adding the
    guard makes the earliest history offset q>=0. No time-varying recursive state.
    """
    def __init__(self, family, guard):
        if guard not in GUARDS or family not in ('lagrange', 'hann', 'kaiser'):
            raise ValueError('Unsupported preregistered kernel')
        self.guard = guard
        self.family = family
        self.name = f'{family}-g{guard}'
        self.count = 4 if guard == 0 else 2 * guard + 1
        self.table = self.polynomial = None
        self.control = Candidate('lagrange3', 4, 48000, 16000) if guard == 0 else None
        self.barycentric = np.array([(-1.)**k * math.comb(self.count-1, k)
                                     for k in range(self.count)])
        self.barycentric /= max(abs(self.barycentric))

    def coefficients(self, delays):
        if self.control is not None:
            return self.control.coefficients(delays)
        d = np.asarray(delays, dtype=float).reshape(-1)
        q = np.floor(d).astype(int)
        local = d - q + self.guard
        taps = q[:, None] + np.arange(-self.guard, self.guard + 1)
        if self.family == 'lagrange':
            difference = local[:, None] - np.arange(self.count)
            # Barycentric Lagrange cardinal functions, with exact integer override.
            weights = self.barycentric / np.where(difference == 0, 1., difference)
            weights /= weights.sum(axis=1)[:, None]
        else:
            distance = taps - d[:, None]
            u = distance / self.guard
            inside = abs(u) <= 1
            if self.family == 'hann':
                window = np.where(inside, .5 + .5*np.cos(np.pi*u), 0.)
            else:
                window = np.where(inside, np.i0(8*np.sqrt(np.maximum(0, 1-u*u))) / np.i0(8), 0.)
            weights = np.sinc(distance) * window
            weights /= weights.sum(axis=1)[:, None]  # Explicit DC constraint only.
        integer = d == q
        weights[integer] = 0
        weights[integer, self.guard] = 1
        return taps, weights

    def causal_output(self, x, physical_delays):
        """Availability model: y[n+g]=aligned[n], including tail; path clock is n."""
        aligned = self.apply(x, physical_delays)
        return np.pad(aligned, ((self.guard, 0), (0, 0)))


class Conditioner:
    """Independent fixed band candidate; filter state belongs only to this run."""
    def __init__(self, rate, pass_hz, stop_hz, taps=None):
        if not 0 < pass_hz < stop_hz < rate/2:
            raise ValueError('Invalid physical frequency band')
        self.rate, self.pass_hz, self.stop_hz = rate, pass_hz, stop_hz
        self.sos = self.fir = None
        self.latency = 0
        if taps is None:
            self.order, critical = signal.buttord(pass_hz, stop_hz, .1, 60, fs=rate)
            self.sos = signal.butter(self.order, critical, fs=rate, output='sos')
            self.name = f'iir-p{pass_hz}-s{stop_hz:g}'
        else:
            self.order = taps-1
            self.latency = (taps-1)//2
            self.fir = signal.firwin(taps, (pass_hz+stop_hz)/2,
                                    window=('kaiser', signal.kaiser_beta(60)), fs=rate,
                                    scale=True)  # Explicit unity-DC FIR design, no output matching.
            self.name = f'fir{taps}-p{pass_hz}-s{stop_hz:g}'

    def response(self, frequencies):
        if self.sos is not None:
            return signal.sosfreqz(self.sos, worN=frequencies, fs=self.rate)[1]
        return signal.freqz(self.fir, worN=frequencies, fs=self.rate)[1]

    def apply(self, x):
        """Causal filter-only output; no delay removal or gain normalization."""
        if self.sos is not None:
            return signal.sosfilt(self.sos, x, axis=0)
        return signal.lfilter(self.fir, [1.], x, axis=0)

    def aligned(self, x):
        """Remove declared FIR delay from the physical clock, not from availability.

        A caller must delay final output by latency+guard. IIR dispersion has no
        single compensating delay and remains explicitly measured filter behavior.
        """
        if not self.latency:
            return self.apply(x)
        padding = [(0, self.latency)]+[(0, 0)]*(np.asarray(x).ndim-1)
        return self.apply(np.pad(x, padding))[self.latency:]


def conditioners(rate):
    for pass_hz in (16000, 18000):
        for stop_hz in sorted(set((18000, 19000, 20000, min(24000, .49*rate)))):
            if stop_hz <= pass_hz:
                continue
            yield Conditioner(rate, pass_hz, stop_hz)
            for taps in (17, 33, 65, 129):
                yield Conditioner(rate, pass_hz, stop_hz, taps)


def static_errors(kernel, rate, frequencies, delays):
    """Analytic frequency derivative of FIR, vectorized across physical delays."""
    taps, weights = kernel.coefficients(delays)
    # Integer stencil base contributes exact phase/delay, leaving a small fixed grid.
    base = taps[:, 0]
    offsets = taps[0] - base[0]
    w = 2*np.pi*frequencies/rate
    basis = np.exp(-1j*w[:, None]*offsets)
    response = weights @ basis.T
    derivative = weights @ (basis*(-1j*offsets)).T
    local = np.asarray(delays)-base
    ideal = np.exp(-1j*local[:, None]*w)
    return np.array([np.max(abs(20*np.log10(np.maximum(abs(response), 1e-300))), axis=0),
                     np.max(abs(np.angle(response/ideal)), axis=0),
                     np.max(abs((-np.imag(derivative/response)-local[:, None])/rate), axis=0),
                     np.max(abs(response-ideal), axis=0)])


def static_pass(errors, frequencies, upper):
    mask = frequencies <= upper
    epsilon = 2*np.sin(np.pi*upper*TIME_ERROR)
    limits = np.array([np.full(mask.sum(), -20*np.log10(1-epsilon)),
                       2*np.pi*frequencies[mask]*TIME_ERROR+1e-12,
                       np.full(mask.sum(), TIME_ERROR), np.full(mask.sum(), epsilon)])
    return bool(np.all(errors[:, mask] <= limits))
