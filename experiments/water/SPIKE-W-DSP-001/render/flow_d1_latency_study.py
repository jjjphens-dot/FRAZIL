"""Reproducible offline EXP-W-FD-002 evidence; fresh output directory required.

Run with OPENBLAS_NUM_THREADS=1. Native sources are exported separately. No
candidate selection mutates runtime, source configurations or the frozen gates.
"""
import argparse
import json
from pathlib import Path

import numpy as np
from scipy import signal

from flow_d1_latency_models import (EPSILON, FIXED_HZ, GUARDS, RATES, Conditioner,
                                   GuardKernel, conditioners, static_errors, static_pass)
from flow_d1_remediation_study import (Candidate, continuous_path, csv_write,
                                      fourier_reference, metrics, native_cluster_signal,
                                      source_envelope)

PROFILES = ('reference', 'edge', 'overlap-reference', 'sustained-edge')


def sources(root, rate):
    for profile in PROFILES:
        raw = np.loadtxt(root/f'{rate}-{profile}.csv', delimiter=',', skiprows=1)
        for name, audio in [('A1', raw[:, 1:3]), ('B1', raw[:, 3:5]),
                            ('AB', native_cluster_signal(raw[:, 1:3], raw[:, 3:5]))]:
            yield profile, name, audio, raw[:, 0]/1484*rate


def qualified_reference(x, delay):
    fine = fourier_reference(x, delay, 8, 16)
    coarse = fourier_reference(x, delay, 4, 8)
    error = metrics(coarse, fine)
    convergence = max(error['relative_rms_error'], error['peak_normalized_error'])
    return fine, convergence


def comb_study(root, output):
    rows, notches = [], []
    for rate in RATES:
        raw = np.loadtxt(root/f'{rate}-overlap-reference.csv', delimiter=',', skiprows=1)
        ab = native_cluster_signal(raw[:, 1:3], raw[:, 3:5])
        native = np.loadtxt(root/f'{rate}-overlap-reference-audit.csv', delimiter=',', skiprows=1)
        historical = Candidate('lagrange3', 4, rate, 20000)
        for seconds in (.005/1484, .015/1484, .03/1484, .05/1484,
                        .00005, .0001, .00025, .0005, .001, .002, .005):
            delay = seconds*rate
            f = np.linspace(0, 20000, 8001)
            taps, weights = historical.coefficients([delay])
            basis = np.exp(-2j*np.pi*f[:, None]/rate*taps)
            h = basis @ weights[0]
            hd = (basis*(-1j*taps)) @ weights[0]
            ideal = np.exp(-2j*np.pi*f*seconds)
            ideal_d = -1j*delay*ideal
            impulse = np.zeros(4096)
            impulse[1024] = 1
            impulse_delay = np.full(len(impulse), delay)
            ideal_audio, convergence = qualified_reference(ab, np.full(len(ab), delay))
            numeric_audio = historical.apply(ab, np.full(len(ab), delay))
            ideal_impulse = fourier_reference(impulse, impulse_delay, 8, 16)[:, 0]
            numeric_impulse = historical.apply(impulse, impulse_delay)[:, 0]
            responses = {'M0': (np.ones_like(h), np.zeros_like(h), ab, impulse),
                         'M1': (ideal, ideal_d, ideal_audio, ideal_impulse),
                         'M2': (h, hd, numeric_audio, numeric_impulse),
                         'M3': (1+ideal, ideal_d, ab+ideal_audio, impulse+ideal_impulse),
                         'M4': (1+h, hd, ab+numeric_audio, impulse+numeric_impulse),
                         'D0-frozen-mechanism': (.1*(ideal-1), .1*ideal_d,
                                                  .1*(ideal_audio-ab), .1*(ideal_impulse-impulse))}
            for name, (response, derivative, audio, response_impulse) in responses.items():
                magnitude = 20*np.log10(np.maximum(abs(response), 1e-15))
                valid = abs(response) > 1e-8
                group = np.full(len(f), np.nan)
                group[valid] = -np.imag(derivative[valid]/response[valid])/rate
                rows.append(dict(rate=rate, delay_ms=seconds*1000, mechanism=name,
                                 physical_range=seconds <= .05/1484,
                                 magnitude_min_db=float(magnitude.min()),
                                 magnitude_max_db=float(magnitude.max()),
                                 ripple_db=float(np.ptp(magnitude)),
                                 valid_group_min_ms=float(np.nanmin(group)*1000),
                                 valid_group_max_ms=float(np.nanmax(group)*1000),
                                 reference_convergence=convergence))
                np.savez_compressed(output/f'comb-{rate}-{seconds:.9f}-{name}.npz',
                                    hz=f, response=response, group_seconds=group,
                                    impulse=response_impulse, audio=audio,
                                    difference_spectrum=np.fft.rfft(audio-ab, axis=0))
            for k in range(int(20000*seconds)+1):
                notch = (2*k+1)/(2*seconds)
                if notch <= 20000:
                    notches.append(dict(rate=rate, delay_ms=seconds*1000, index=k,
                                        ideal_notch_hz=notch, spacing_hz=1/seconds,
                                        ideal_magnitude=abs(1+np.exp(-2j*np.pi*notch*seconds))))
        # Actual historical control owns input, activity and trajectory. It is not
        # the frozen analytical g*(H-1) above, and is never substituted for AB.
        np.savez_compressed(output/f'd0-native-{rate}.npz', input=native[:, :2],
                            residual=native[:, 2:4], ab=ab, current_d1=native[:, 4:6])
        print('comb', rate, 'complete', flush=True)
    csv_write(output/'COMB.csv', rows)
    csv_write(output/'NOTCHES.csv', notches)


def nonlinear_filter_probe(conditioner):
    """Shared 96 kHz synthetic nonlinear stress; not a source or Ice model."""
    rate = conditioner.rate
    if rate != 96000:
        raise ValueError('This probe requires 96 kHz')
    nonlinear = []
    # Deliberate stress input, not a predicted playback-chain distortion.
    t = np.arange(rate)/rate
    tones = np.array([22000., 25000., 37000.])
    x = sum(.25*np.sin(2*np.pi*f*t) for f in tones)
    y = conditioner.apply(x)
    cut = slice(rate//4, 3*rate//4)
    dense_t = np.arange(8*rate)/(8*rate)
    gains = conditioner.response(tones)
    dense_x = sum(.25*np.sin(2*np.pi*f*dense_t) for f in tones)
    dense_y = sum(.25*np.imag(g*np.exp(2j*np.pi*f*dense_t))
                  for f, g in zip(tones, gains))
    for order in (2, 3):
        raw_spec = np.fft.rfft(x[cut]**order)
        conditioned_spec = np.fft.rfft(y[cut]**order)
        hz = np.fft.rfftfreq(len(x[cut]), 1/rate)
        band = (hz > 0) & (hz <= 20000)
        # Periodic spectral truncation of8x analytic steady state:
        # nonlinear bandwidth<=111 kHz <384 kHz Nyquist. This separates
        # true audible IMD from audio-rate foldback, without Ice claims.
        raw_oracle = np.fft.irfft(np.fft.rfft(dense_x**order)[:rate//2+1], n=rate)/8
        conditioned_oracle = np.fft.irfft(np.fft.rfft(dense_y**order)[:rate//2+1], n=rate)/8
        raw_alias = np.fft.rfft(x[cut]**order-raw_oracle[cut])
        conditioned_alias = np.fft.rfft(y[cut]**order-conditioned_oracle[cut])
        nonlinear.append(dict(rate=rate, conditioner=conditioner.name, nonlinearity=f'x^{order}',
                              audible_raw_energy=float(np.sum(abs(raw_spec[band])**2)),
                              audible_conditioned_energy=float(np.sum(abs(conditioned_spec[band])**2)),
                              audible_raw_alias_energy=float(np.sum(abs(raw_alias[band])**2)),
                              audible_conditioned_alias_energy=float(np.sum(abs(conditioned_alias[band])**2)),
                              scope='synthetic IMD/alias stress; not implemented Ice or measured playback'))
    return nonlinear


def filter_study(root, output):
    rows, actual, nonlinear, selected = [], [], [], {}
    for rate in RATES:
        selected[rate] = []
        fixtures = list(sources(root, rate))
        for conditioner in conditioners(rate):
            f = np.unique(np.r_[np.linspace(0, rate/2, 16385), FIXED_HZ,
                                conditioner.pass_hz, conditioner.stop_hz])
            h = conditioner.response(f)
            mag = 20*np.log10(np.maximum(abs(h), 1e-300))
            pass_error = max(abs(mag[f <= conditioner.pass_hz]))
            stop_gain = max(mag[f >= conditioner.stop_hz])
            poles = [] if conditioner.sos is None else np.concatenate(
                [np.roots(section[3:]) for section in conditioner.sos])
            stable = not len(poles) or max(abs(poles)) < 1
            passed = (np.isfinite(h).all() and stable and pass_error <= .1+1e-9 and
                      stop_gain <= -60+1e-9 and max(mag) <= .1)
            if passed:
                selected[rate].append(conditioner)
            impulse = np.zeros(rate//4)
            impulse[rate//16] = 1
            response = conditioner.apply(impulse)
            onset = rate//16
            peak = int(np.argmax(abs(response)))
            energy = float(response@response)
            centre = onset+conditioner.latency if conditioner.fir is not None else peak
            significant = np.flatnonzero(abs(response) > max(abs(response))*1e-4)
            # Differentiated unwrapped phase only inside passband, away from zeros.
            group = -np.gradient(np.unwrap(np.angle(h)), 2*np.pi*f/rate)/rate
            row = dict(rate=rate, conditioner=conditioner.name, order=conditioner.order,
                       pass_hz=conditioner.pass_hz, stop_hz=conditioner.stop_hz,
                       latency_samples=conditioner.latency, latency_ms=conditioner.latency/rate*1000,
                       pass_error_db=float(pass_error), stop_max_db=float(stop_gain),
                       response16_db=float(mag[np.flatnonzero(f == 16000)[0]]),
                       response18_db=float(mag[np.flatnonzero(f == 18000)[0]]),
                       response20_db=float(mag[np.flatnonzero(f == 20000)[0]]),
                       group_min_ms=float(group[f <= conditioner.pass_hz].min()*1000),
                       group_max_ms=float(group[f <= conditioner.pass_hz].max()*1000),
                       before_main_energy_fraction=float(response[:centre]@response[:centre]/energy),
                       after_main_energy_fraction=float(response[centre+1:]@response[centre+1:]/energy),
                       before_input_energy=float(response[:onset]@response[:onset]),
                       ringing_span_ms=float((significant[-1]-significant[0])/rate*1000),
                       max_pole_radius=float(max(abs(poles))) if len(poles) else 0.,
                       filter_screen_pass=bool(passed))
            rows.append(row)
            np.savez_compressed(output/f'filter-{rate}-{conditioner.name}.npz',
                                hz=f, response=h, impulse=response,
                                coefficients=conditioner.sos if conditioner.sos is not None else conditioner.fir)
            for profile, name, audio, _ in fixtures:
                y = conditioner.apply(audio)
                aligned = y[conditioner.latency:] if conditioner.latency else y
                original = audio[:len(aligned)]
                first_in = int(np.flatnonzero(np.any(audio != 0, axis=1))[0])
                first_out = int(np.flatnonzero(np.any(y != 0, axis=1))[0])
                actual.append(dict(rate=rate, conditioner=conditioner.name, profile=profile, source=name,
                                   **metrics(aligned, original), peak_ratio=float(abs(y).max()/abs(audio).max()),
                                   first_nonzero_shift_samples=first_out-first_in,
                                   engineering_latency_removed=conditioner.latency))
            if rate == 96000:
                nonlinear.extend(nonlinear_filter_probe(conditioner))
        print('filters', rate, 'qualified', len(selected[rate]), flush=True)
    csv_write(output/'FILTERS.csv', rows)
    csv_write(output/'FILTER_ACTUAL.csv', actual)
    csv_write(output/'NONLINEAR.csv', nonlinear)
    cross = []
    for rate in RATES[:2]:
        for c in conditioners(rate):
            nyquist_relative = c.stop_hz == min(24000, .49*rate)
            other_stop = 24000 if nyquist_relative else c.stop_hz
            reference = Conditioner(96000, c.pass_hz, other_stop,
                                    None if c.fir is None else len(c.fir))
            h = c.response(FIXED_HZ)*np.exp(2j*np.pi*FIXED_HZ*c.latency/rate)
            href = reference.response(FIXED_HZ)*np.exp(2j*np.pi*FIXED_HZ*reference.latency/96000)
            for i, frequency in enumerate(FIXED_HZ):
                cross.append(dict(rate=rate, reference_rate=96000, conditioner=c.name,
                                  reference_conditioner=reference.name, frequency_hz=frequency,
                                  magnitude_difference_db=float(20*np.log10(max(abs(h[i]), 1e-300)/max(abs(href[i]), 1e-300))),
                                  phase_difference_rad=float(np.angle(h[i]/href[i])),
                                  engineering_latency_removed=True,
                                  acceptance='diagnostic; no cross-rate product/filter acceptance'))
    csv_write(output/'FILTER_CROSS_RATE.csv', cross)
    return selected


def kernel_study(output):
    rows, points, moving, cross, eligible = [], [], [], [], {}
    common_outputs = {}
    for rate in RATES:
        eligible[rate] = []
        f = np.unique(np.r_[np.linspace(0, 20000, 2001), FIXED_HZ])
        maximum = .05/1484*rate
        d = np.unique(np.r_[np.linspace(0, maximum, 201),
                            [v for k in range(5) for v in (k-1e-8, k, k+1e-8) if 0 <= v <= maximum]])
        t = np.arange(rate//2)/rate
        path = continuous_path(t)
        delays = path/1484*rate
        for guard in GUARDS:
            for family in (('lagrange',) if guard == 0 else ('lagrange', 'hann', 'kaiser')):
                kernel = GuardKernel(family, guard)
                errors = static_errors(kernel, rate, f, d)
                passes = {upper: static_pass(errors, f, upper) for upper in (16000, 18000, 20000)}
                if passes[16000]:
                    eligible[rate].append((kernel, passes))
                for upper in passes:
                    band = f <= upper
                    maximum_errors = errors[:, band].max(axis=1)
                    rows.append(dict(rate=rate, kernel=kernel.name, guard_samples=guard,
                                     guard_ms=guard/rate*1000, band_hz=upper,
                                     magnitude_error_db=maximum_errors[0], phase_error_rad=maximum_errors[1],
                                     group_delay_error_ns=maximum_errors[2]*1e9, complex_error=maximum_errors[3],
                                     static_pass=passes[upper]))
                for frequency in FIXED_HZ:
                    i = np.flatnonzero(f == frequency)[0]
                    points.append(dict(rate=rate, kernel=kernel.name, frequency_hz=frequency,
                                       magnitude_error_db=errors[0, i], phase_error_rad=errors[1, i],
                                       group_delay_error_ns=errors[2, i]*1e9, complex_error=errors[3, i]))
                    x = np.sin(2*np.pi*frequency*t)
                    ideal = np.sin(2*np.pi*frequency*(t-path/1484))[:, None]
                    y = kernel.apply(x, delays)
                    error = metrics(y[256:-256], ideal[256:-256])
                    moving.append(dict(rate=rate, kernel=kernel.name, frequency_hz=frequency,
                                       **error, core_pass=bool(error['relative_rms_error'] <= EPSILON and
                                                               error['peak_normalized_error'] <= EPSILON)))
                    for upper in passes:
                        if frequency <= upper:
                            passes[upper] &= (error['relative_rms_error'] <= EPSILON and
                                              error['peak_normalized_error'] <= EPSILON)
                    index = np.rint(np.arange(9, 141)/300*rate).astype(int)
                    common_outputs[rate, kernel.name, frequency] = (y-ideal)[index, 0]
        print('kernels', rate, 'complete', flush=True)
    for (rate, name, frequency), values in common_outputs.items():
        if rate == 96000:
            continue
        difference = values-common_outputs[96000, name, frequency]
        error = float(np.sqrt(np.mean(difference*difference)))
        cross.append(dict(rate=rate, reference_rate=96000, kernel=name, frequency_hz=frequency,
                          common_time_error_rms=error, pair_budget_pass=bool(error <= 2*EPSILON)))
        if error > 2*EPSILON:
            for tested_rate in (rate, 96000):
                for kernel, passes in eligible[tested_rate]:
                    if kernel.name == name:
                        for upper in passes:
                            if frequency <= upper:
                                passes[upper] = False
    for name, data in [('KERNELS', rows), ('FIXED_HZ', points), ('MOVING', moving), ('CROSS_RATE', cross)]:
        csv_write(output/f'{name}.csv', data)
    return eligible


def joint_study(root, output, filters, kernels):
    rows, coverage, leakage, selection = [], [], [], []
    for rate in RATES:
        fixtures = list(sources(root, rate))
        for profile in PROFILES:
            raw = np.loadtxt(root/f'{rate}-{profile}.csv', delimiter=',', skiprows=1)
            simultaneous = (raw[:, 1] != 0) & (raw[:, 3] != 0)
            ab = native_cluster_signal(raw[:, 1:3], raw[:, 3:5])
            coverage.append(dict(rate=rate, profile=profile, overlap_frames=int(simultaneous.sum()),
                                 overlap_energy=float(np.sum(ab[simultaneous]**2)),
                                 total_energy=float(np.sum(ab**2))))
            native = np.loadtxt(root/f'{rate}-{profile}-audit.csv', delimiter=',', skiprows=1)
            old = Candidate('lagrange3', 4, rate, 20000).apply(ab, raw[:, 0]/1484*rate)
            # Actual C++ transfer versus independent polynomial expansion on the bus.
            error = float(abs(old-native[:, 4:6]).max())
            leakage.append(dict(rate=rate, profile=profile, native_max_error=error,
                                duplicate_ab_counterexample_error=float(abs(old+ab-native[:, 4:6]).max()),
                                passed=bool(error < 1e-10)))
        for conditioner in [None]+filters[rate]:
            name = 'raw-control' if conditioner is None else conditioner.name
            upper = 20000 if conditioner is None else conditioner.pass_hz
            candidates = [(k, p) for k, p in kernels[rate] if p[upper]]
            # Every static-passing guard remains tested; no selecting from a single fixture.
            passing = {k.name: True for k, _ in candidates}
            for profile, source, audio, delay in fixtures:
                conditioned = audio if conditioner is None else conditioner.aligned(audio)
                ideal, convergence = qualified_reference(conditioned, delay)
                for kernel, _ in candidates:
                    values = metrics(kernel.apply(conditioned, delay), ideal)
                    passed = (convergence <= EPSILON/10 and values['finite'] and
                              values['relative_rms_error'] <= EPSILON and
                              values['peak_normalized_error'] <= EPSILON)
                    passing[kernel.name] &= passed
                    rows.append(dict(rate=rate, conditioner=name, kernel=kernel.name,
                                     profile=profile, source=source, **values,
                                     reference_convergence=convergence, joint_pass=bool(passed)))
            qualified = [k for k, _ in candidates if passing[k.name]]
            least = min((k.guard for k in qualified), default=-1)
            for k in qualified:
                selection.append(dict(rate=rate, conditioner=name, kernel=k.name,
                                      guard_samples=k.guard, filter_latency_samples=0 if conditioner is None else conditioner.latency,
                                      total_latency_samples=k.guard+(0 if conditioner is None else conditioner.latency),
                                      minimum_passing_guard=k.guard == least,
                                      acceptance='numerical fixture screen only; native/human review required'))
            print('joint', rate, name, 'minimum guard', least, flush=True)
            csv_write(output/'JOINT.partial.csv', rows)
    for name, data in [('JOINT', rows), ('OVERLAP', coverage), ('LEAKAGE', leakage)]:
        csv_write(output/f'{name}.csv', data)
    if selection:
        csv_write(output/'SELECTION.csv', selection)
    return selection


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sources', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    authority = json.loads((args.sources/'authority.json').read_text())
    source_envelope(authority)
    if authority['sound_speed_mps'] != 1484 or authority['maximum_path_m'] != .05:
        raise ValueError('Physical authority drift: review contract before study')
    comb_study(args.sources, args.output)
    filters = filter_study(args.sources, args.output)
    kernels = kernel_study(args.output)
    selection = joint_study(args.sources, args.output, filters, kernels)
    (args.output/'report.json').write_text(json.dumps(dict(
        contract='EXP-W-FD-002', authority=authority, epsilon_core=EPSILON,
        qualified_combinations=len(selection), runtime_replaced=False,
        human_acceptance='NOT ASSESSED', native_resources='NOT RUN'), indent=2), encoding='utf-8')


if __name__ == '__main__':
    main()
