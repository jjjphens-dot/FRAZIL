"""Source-aware D1 numerical research; no candidate changes realtime DSP.

Run with a new output directory and the actual C++ source-probe output. Analytic
complex exponentials and continuous-time signals are independent ideal oracles.
Oversampled Fourier reconstruction is an offline reference. The centred Hann sinc
is a noncausal comparison control, never an eligible realtime candidate.
"""
import argparse
import csv
import json
from pathlib import Path
import time

import numpy as np

RATES = (44100, 48000, 96000)
FIXED_HZ = (1000, 2000, 4000, 8000, 12000, 16000, 18000)


def csv_write(path, rows):
    with path.open('w', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def source_envelope(authority):
    """Independent SI equation; typed limits/constants come from the executable."""
    factor = np.sqrt(3 * authority['gamma'] * authority['pressure_pa'] /
                     authority['density_kg_m3']) / (2 * np.pi)
    f0 = [factor / (authority[f'{source}_min_radius_mm'] * .001)
          for source in ('a1', 'b1')]
    for source, frequency in zip(('a1', 'b1'), f0):
        if not np.isclose(frequency, authority[f'{source}_f0_hz'], rtol=1e-13):
            raise ValueError('Independent Minnaert equation disagrees with source probe')
    # Voice cap audited in BubbleA1VoicePool and DropletB1BubbleVoice. This is an
    # oscillator support envelope, not a hard spectral bandlimit for finite events.
    return max(f0) * np.sqrt(2)


class Candidate:
    """Offline kernels with explicit tap positions; negative taps require lookahead."""
    def __init__(self, name, taps, rate, upper_hz):
        self.name, self.count = name, taps
        self.table = self.polynomial = None
        if name.startswith(('vfd', 'farrow')):
            # Real least-squares frequency fit to the complex ideal, uniform in Hz.
            # Eliminate h[0] to enforce exact DC, rather than normalize a fitted EQ.
            w = 2 * np.pi * np.linspace(0, upper_hz / rate, 2049)
            basis = np.exp(-1j * w[:, None] * np.arange(1, taps)) - 1
            matrix = np.vstack((basis.real, basis.imag))
            mu = np.linspace(0, 1, 513)
            desired = np.exp(-1j * w[:, None] * mu) - 1
            solution = np.linalg.lstsq(matrix, np.vstack((desired.real, desired.imag)),
                                       rcond=1e-12)[0].T
            self.table = np.column_stack((1 - solution.sum(axis=1), solution))
            self.table[0] = np.eye(1, taps, 0)[0]
            self.table[-1] = np.eye(1, taps, 1)[0]
            if name.startswith('farrow'):
                # Polynomial coefficients are fixed FIR branches. Horner evaluation
                # below is the Farrow structure, not an extra filter on source audio.
                self.polynomial = np.polynomial.polynomial.polyfit(2 * mu - 1,
                                                                   self.table, 7)
                self.table = None

    def coefficients(self, delays):
        d = np.asarray(delays, dtype=float).reshape(-1)
        if self.name.startswith('sinc'):
            half = self.count // 2
            taps = np.floor(d).astype(int)[:, None] + np.arange(-half, half + 1)
            distance = taps - d[:, None]
            weight = np.sinc(distance) * np.where(np.abs(distance) <= half,
                                                  .5 + .5 * np.cos(np.pi * distance / half), 0)
            weight /= weight.sum(axis=1)[:, None]
        elif self.name.startswith('lagrange'):
            base = np.maximum(0, np.floor(d).astype(int) - (self.count // 2 - 1))
            taps = base[:, None] + np.arange(self.count)
            weight = np.ones(taps.shape)
            local = d - base
            for k in range(self.count):
                for j in range(self.count):
                    if k != j:
                        weight[:, k] *= (local - j) / (k - j)
        else:
            base = np.floor(d).astype(int)
            mu = d - base
            taps = base[:, None] + np.arange(self.count)
            if self.table is not None:
                position = mu * (len(self.table) - 1)
                lo = position.astype(int)
                fraction = position - lo
                weight = ((1 - fraction[:, None]) * self.table[lo] +
                          fraction[:, None] * self.table[np.minimum(lo + 1, len(self.table)-1)])
            else:
                weight = np.zeros((len(d), self.count))
                for coefficient in self.polynomial[::-1]:
                    weight = weight * (2 * mu[:, None] - 1) + coefficient
            # Mandatory integer identity; a polynomial endpoint mismatch immediately
            # next to the integer is measured separately by the static grid.
            weight[mu == 0] = np.eye(1, self.count, 0)[0]
        return taps, weight

    def apply(self, signal, delays):
        x = np.asarray(signal)
        if x.ndim == 1:
            x = x[:, None]
        out = np.zeros_like(x, dtype=float)
        for start in range(0, len(x), 256):
            stop = min(start + 256, len(x))
            taps, weights = self.coefficients(delays[start:stop])
            index = np.arange(start, stop)[:, None] - taps
            valid = (index >= 0) & (index < len(x))
            samples = x[np.clip(index, 0, len(x)-1)]
            out[start:stop] = np.einsum('nt,ntc->nc', weights * valid, samples)
        return out

    def response(self, delay, frequencies, rate):
        taps, weights = self.coefficients([delay])
        basis = np.exp(-2j * np.pi * frequencies[:, None] / rate * taps)
        response = basis @ weights[0]
        derivative = (basis * (-1j * taps)) @ weights[0]
        group_seconds = -np.imag(derivative / response) / rate
        ideal = np.exp(-2j * np.pi * frequencies / rate * delay)
        return (20 * np.log10(np.maximum(np.abs(response), 1e-300)),
                np.angle(response / ideal), group_seconds - delay / rate,
                np.abs(response - ideal))


def metrics(output, reference):
    error = output - reference
    norm = max(float(np.linalg.norm(reference)), 1e-30)
    peak = max(float(np.max(np.abs(reference))), 1e-30)
    spectrum = np.fft.rfft(output, axis=0)
    ref_spectrum = np.fft.rfft(reference, axis=0)
    return {'relative_rms_error': float(np.linalg.norm(error) / norm),
            'peak_normalized_error': float(np.max(np.abs(error)) / peak),
            'spectral_magnitude_relative_error': float(np.linalg.norm(
                np.abs(spectrum)-np.abs(ref_spectrum)) / max(np.linalg.norm(ref_spectrum), 1e-30)),
            'finite': bool(np.isfinite(output).all())}


def native_cluster_signal(a1, b1):
    """Match FluidComponents::sum: double sum including zero D0, then float output."""
    return (a1.astype(np.float64) + b1.astype(np.float64) + 0.).astype(np.float32).astype(np.float64)


def fourier_reference(signal, delays, padding=4, oversampling=8):
    """Offline bandlimited Fourier oracle with zero-padded periodic extension.

    Interpolate the oversampled result on a centred 8-point stencil. Its bandwidth
    is <=1/(2*oversampling), unlike a direct causal audio-rate stencil. Compare
    both doubled period and doubled oversampling to expose each approximation.
    """
    x = np.asarray(signal)
    if x.ndim == 1:
        x = x[:,None]
    length = 1 << int(np.ceil(np.log2(padding*len(x))))
    offset = len(x)
    padded = np.zeros((length,x.shape[1]))
    padded[offset:offset+len(x)] = x
    spectrum = np.fft.rfft(padded,axis=0)
    # The former real Nyquist bin becomes a conjugate pair after zero-padding.
    spectrum[-1] *= .5
    dense = np.fft.irfft(spectrum,n=length*oversampling,axis=0)*oversampling
    position = (offset+np.arange(len(x))-delays)*oversampling
    base = np.floor(position).astype(int)-3
    local = position-base
    result = np.zeros_like(x,dtype=float)
    for k in range(8):
        coefficient = np.ones(len(x))
        for j in range(8):
            if j != k:
                coefficient *= (local-j)/(k-j)
        result += coefficient[:,None]*dense[base+k]
    return result


def continuous_path(t):
    """One rate-independent seeded smooth physical path, bounded by U=1 m/s.

    Fixed .1 s segments and 5 cm extent imply speed <=.75 m/s. This isolates
    numerical sample-rate error; it is not a replacement D1 trajectory.
    """
    targets = np.r_[0., np.random.default_rng(42).uniform(0, .05, 32)]
    segment = np.floor(t / .1).astype(int)
    p = t / .1 - segment
    return targets[segment] + (targets[segment+1]-targets[segment]) * p*p*(3-2*p)


def fixed_path_comparison(authority):
    """Same physical path and frequency, including signed errors at every rate."""
    rows = []
    for rate in RATES:
        upper = min(source_envelope(authority),.45*rate)
        for name, taps in [('lagrange3',4),('lagrange7',8),('lagrange15',16),
                           ('farrow7-fir32',32),('vfd-ls32',32),('vfd-ls64',64),
                           ('sinc129-noncausal-control',129)]:
            candidate = Candidate(name,taps,rate,upper)
            frequencies = np.array([4000.,8000.,12000.,16000.])
            for path in (.005,.015,.03,.05):
                values = candidate.response(path/authority['sound_speed_mps']*rate,frequencies,rate)
                for i,frequency in enumerate(frequencies):
                    rows.append({'rate':rate,'candidate':name,'path_m':path,'frequency_hz':frequency,
                                 'magnitude_error_db':values[0][i],'phase_error_rad':values[1][i],
                                 'group_delay_error_ns':values[2][i]*1e9,'complex_error':values[3][i]})
    return rows


def run(args):
    args.output.mkdir(parents=True, exist_ok=False)
    authority = json.loads((args.sources / 'authority.json').read_text())
    relevant = source_envelope(authority)
    c, maximum_path = authority['sound_speed_mps'], authority['maximum_path_m']
    dt = maximum_path * 3.2 / c**2
    static, points, probes, actual, resources, cross = [], [], [], [], [], []
    moving_outputs = {}
    for rate in RATES:
        upper = min(relevant, .45*rate)
        epsilon = 2*np.sin(np.pi*upper*dt)
        maximum_delay = maximum_path / c * rate
        frequencies = np.unique(np.r_[np.linspace(0, upper, 1801), FIXED_HZ,
                                      rate*np.array([.1, .2, .3, .4, .45])])
        band = frequencies <= upper
        delays = np.unique(np.r_[np.linspace(0, maximum_delay, 201),
                                [v for k in range(4) for v in (k-1e-8, k, k+1e-8)
                                 if 0 <= v <= maximum_delay]])
        candidates = [Candidate(f'lagrange{order}', order+1, rate, upper) for order in (3, 7, 15)]
        candidates += [Candidate('farrow7-fir32', 32, rate, upper),
                       Candidate('vfd-ls32', 32, rate, upper),
                       Candidate('vfd-ls64', 64, rate, upper)]
        # This lookahead control measures a centred alternative, but cannot
        # be selected for the immediate causal process API.
        candidates += [Candidate('sinc129-noncausal-control', 129, rate, upper)]
        t = np.arange(rate // 2) / rate
        path = continuous_path(t)
        moving_delay = path / c * rate
        fixtures = {'impulse': (np.arange(len(t)) == int(.2*rate)).astype(float),
                    'broadband': np.random.default_rng(42).uniform(-.5, .5, len(t)),
                    'transient': np.sin(2*np.pi*8000*t) * ((t >= .2) & (t < .22)),
                    'multitone': sum(np.sin(2*np.pi*f*t)/4 for f in (4000,8000,12000,16000))}
        fixtures.update({f'sine{f}': np.sin(2*np.pi*f*t) for f in (4000,8000,12000,16000)})
        oracle = {}
        for name, signal in fixtures.items():
            if name.startswith('sine'):
                ideal = np.sin(2*np.pi*int(name[4:])*(t-path/c))[:, None]
                convergence = 0.
            elif name == 'multitone':
                ideal = sum(np.sin(2*np.pi*f*(t-path/c))/4 for f in (4000,8000,12000,16000))[:, None]
                convergence = 0.
            else:
                ideal = fourier_reference(signal,moving_delay,8,16)
                convergence = metrics(fourier_reference(signal,moving_delay,4,8), ideal)['relative_rms_error']
            oracle[name] = ideal, convergence
        for candidate in candidates:
            maxima = np.zeros((4, len(frequencies)))
            coefficient_l1 = 0.
            for delay in delays:
                values = np.array(candidate.response(delay, frequencies, rate))
                maxima = np.maximum(maxima, np.abs(values))
                coefficient_l1 = max(coefficient_l1, np.abs(candidate.coefficients([delay])[1]).sum())
            limits = np.array([np.full(band.sum(), -20*np.log10(1-epsilon)),
                               2*np.pi*frequencies[band]*dt + 1e-12,
                               np.full(band.sum(), dt), np.full(band.sum(), epsilon)])
            passed = bool(np.all(maxima[:, band] <= limits))
            static.append({'rate': rate, 'candidate': candidate.name, 'upper_hz': upper,
                           'magnitude_error_db': float(maxima[0, band].max()),
                           'phase_error_rad': float(maxima[1, band].max()),
                           'group_delay_error_ns': float(maxima[2, band].max()*1e9),
                           'complex_error': float(maxima[3, band].max()), 'epsilon': epsilon,
                           'group_delay_budget_ns': dt*1e9, 'static_pass': passed,
                           'causal': not candidate.name.startswith('sinc'), 'max_coefficient_l1': coefficient_l1})
            for f in np.unique(np.r_[FIXED_HZ, rate*np.array([.1,.2,.3,.4,.45])]):
                i = np.flatnonzero(frequencies == f)[0]
                points.append({'rate':rate, 'candidate':candidate.name, 'frequency_hz':float(f),
                               'in_source_band': bool(f <= upper), 'magnitude_error_db':maxima[0,i],
                               'phase_error_rad':maxima[1,i], 'group_delay_error_ns':maxima[2,i]*1e9})
            for name, signal in fixtures.items():
                start = time.perf_counter()
                output = candidate.apply(signal, moving_delay)
                elapsed = time.perf_counter()-start
                ideal, convergence = oracle[name]
                # Exclude startup/end support for perpetual analytic signals only.
                cut = slice(1024, -1024) if name.startswith('sine') or name == 'multitone' else slice(None)
                row = metrics(output[cut], ideal[cut])
                probes.append({'rate':rate, 'candidate':candidate.name, 'fixture':name,
                               **row, 'reference_convergence_relative_rms':convergence,
                               'reference_qualified': bool(convergence <= epsilon/10),
                               'python_wall_seconds':elapsed})
                if name.startswith('sine'):
                    # Store values/errors on a common physical-time grid (1 ms).
                    # Multiples of 1/300 s lie on all three sample lattices, so no
                    # resampler or nearest-sample time mismatch enters comparison.
                    common = np.arange(9,141) / 300
                    index = np.rint(common*rate).astype(int)
                    moving_outputs[rate,candidate.name,name] = (output-ideal)[index,0]
                    np.savez(args.output/f'{rate}-{candidate.name}-{name}.npz',
                             frequency_hz=np.fft.rfftfreq(len(t),1/rate),
                             output_spectrum=np.fft.rfft(output[:,0]),
                             ideal_spectrum=np.fft.rfft(ideal[:,0]), output=output[:,0],ideal=ideal[:,0])
            resources.append({'rate':rate, 'candidate':candidate.name, 'taps':candidate.count,
                              'stereo_float_history_bytes':2*4*(candidate.count+int(np.ceil(maximum_delay))),
                              'coefficient_bytes':0 if candidate.table is None and candidate.polynomial is None else
                                  (candidate.table.nbytes if candidate.table is not None else candidate.polynomial.nbytes),
                              'native_callback_timing':'NOT RUN - offline candidate research'})
        for profile in ('reference','edge'):
            raw = np.loadtxt(args.sources/f'{rate}-{profile}.csv', delimiter=',', skiprows=1)
            delay = raw[:,0]/c*rate
            for name, signal in [('A1',raw[:,1:3]),('B1',raw[:,3:5]),
                                 ('AB',native_cluster_signal(raw[:,1:3],raw[:,3:5]))]:
                ideal = fourier_reference(signal,delay,8,16)
                convergence = metrics(fourier_reference(signal,delay,4,8),ideal)['relative_rms_error']
                spectrum = np.fft.rfft(signal,axis=0)
                outside = float(np.sum(np.abs(spectrum[np.fft.rfftfreq(len(signal),1/rate)>upper])**2) /
                                max(np.sum(np.abs(spectrum)**2),1e-30))
                for candidate in candidates:
                    row = metrics(candidate.apply(signal,delay),ideal)
                    actual.append({'rate':rate, 'profile':profile,'source':name,'candidate':candidate.name,
                                   **row,'outside_band_energy_fraction':outside,
                                   'reference_convergence_relative_rms':convergence,
                                   'reference_qualified':bool(convergence<=epsilon/10),
                                   'error_budget_pass':bool(row['relative_rms_error']<=epsilon and
                                                            row['peak_normalized_error']<=epsilon)})
        print(rate, 'Hz source envelope', upper, 'completed', flush=True)
    for name in [r['candidate'] for r in resources if r['rate']==RATES[0]]:
        for fixture in ('sine4000','sine8000','sine12000','sine16000'):
            for rate in RATES[:2]:
                delta = moving_outputs[rate,name,fixture]-moving_outputs[96000,name,fixture]
                cross.append({'candidate':name,'fixture':fixture,'rate':rate,'reference_rate':96000,
                              'common_time_error_difference_rms':float(np.sqrt(np.mean(delta**2))),
                              'scope':'identical physical times on all sample lattices, 1/300 s grid'})
    for name, rows in [('STATIC',static),('POINTS',points),('PROBES',probes),('ACTUAL',actual),
                       ('RESOURCES',resources),('CROSS_RATE',cross)]:
        csv_write(args.output/f'{name}.csv',rows)
    csv_write(args.output/'FIXED_PATH.csv',fixed_path_comparison(authority))
    (args.output/'report.json').write_text(json.dumps({'authority':authority,'relevant_max_hz':relevant,
        'group_delay_budget_seconds':dt,'delay_grid_count_at_least':201,'frequency_grid_count_at_least':1801,
        'selection':'NONE unless all eligibility gates independently pass; see CSVs',
        'reference':'zero-padded Fourier extension; period >=4N/8N, oversampling 8/16, centred order7 interpolation; convergence reported',
        'scope':'offline engineering study; no native or listening acceptance'},indent=2),encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sources',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    run(parser.parse_args())
