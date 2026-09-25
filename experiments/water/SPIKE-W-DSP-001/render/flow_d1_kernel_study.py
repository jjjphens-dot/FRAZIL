"""Independent polynomial delay study; no renderer or physical validation claim."""
import argparse
import json
from pathlib import Path
import time
import numpy as np


def weights(delay, cubic):
    base = max(0, int(np.floor(delay)) - (1 if cubic else 0))
    x = delay - base
    nodes = np.arange(4 if cubic else 2)
    w = np.ones(len(nodes))
    for k in nodes:
        for j in nodes:
            if j != k:
                w[k] *= (x-j)/(k-j)
    return base+nodes, w


def apply(signal, delays, cubic):
    out = np.zeros(len(signal))
    for n, delay in enumerate(delays):
        taps, w = weights(delay, cubic)
        for tap, coefficient in zip(taps, w):
            if n >= tap:
                out[n] += coefficient * signal[n-tap]
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    a = parser.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    rows = []
    for rate in (44100, 48000, 96000):
        max_delay = .05/1484*rate
        for cubic in (False, True):
            freq = np.linspace(0, .45, 1801)
            omega = 2*np.pi*freq
            band = freq <= .2
            magnitude, phase, group = [], [], []
            stress = []
            for delay in np.linspace(0, max_delay, 201):
                taps, w = weights(delay, cubic)
                h = np.exp(-1j*omega[:, None]*taps) @ w
                db = 20*np.log10(np.maximum(np.abs(h), 1e-15))
                pe = np.unwrap(np.angle(h))+omega*delay
                gd = -np.gradient(np.unwrap(np.angle(h)), omega)-delay
                magnitude.append(float(np.max(np.abs(db[band]))))
                phase.append(float(np.max(np.abs(pe[band]))))
                group.append(float(np.max(np.abs(gd[band]))))
                stress.append(float(np.max(np.abs(db))))
            n = np.arange(4096)
            # Bounded smooth movement for numerical sideband stress, not stochastic D1.
            delay = max_delay*(.5-.5*np.cos(2*np.pi*n/4096))
            signals = {'impulse': (n==64).astype(float),
                       'stepped_sine': np.sin(2*np.pi*np.cumsum(np.select(
                           [n<1024,n<2048,n<3072],[.02,.10,.20],default=.40))),
                       'amplitude_step_sine': np.sin(2*np.pi*.1*n)*np.where(n<2048,.1,.5),
                       'high_frequency_sine': np.sin(2*np.pi*.22*n),
                       'two_tone': .25*(np.sin(2*np.pi*.11*n)+np.sin(2*np.pi*.17*n)),
                       'broadband': np.random.default_rng(42).uniform(-.5,.5,len(n))}
            probes = {}
            for name, signal in signals.items():
                start = time.perf_counter()
                output = apply(signal, delay, cubic)
                elapsed = time.perf_counter()-start
                probes[name] = {'finite': bool(np.isfinite(output).all()),
                    'input_rms': float(np.sqrt(np.mean(signal**2))),
                    'output_rms': float(np.sqrt(np.mean(output**2))),
                    'python_wall_seconds': elapsed}
                if name == 'high_frequency_sine':
                    ideal = np.sin(2*np.pi*.22*(n-delay))
                    probes[name]['moving_ideal_error_rms'] = float(np.sqrt(np.mean((output[8:]-ideal[8:])**2)))
                    np.savetxt(a.output/f'{rate}-{cubic}-moving-spectrum.csv',
                        np.column_stack((np.fft.rfftfreq(len(n)),np.abs(np.fft.rfft(output)),np.abs(np.fft.rfft(ideal)))),
                        delimiter=',',header='cycles_per_sample,numeric_amplitude,ideal_timewarp_amplitude')
            rows.append({'rate':rate, 'kernel':'lagrange3' if cubic else 'linear',
                'band_magnitude_error_db':max(magnitude),'band_phase_error_rad':max(phase),
                'band_group_delay_error_samples':max(group),'stress_magnitude_error_db':max(stress),
                'target_pass':max(magnitude)<=1 and max(phase)<=.1,'probes':probes})
    (a.output/'report.json').write_text(json.dumps({'scope':'numerical independent oracle; Python timing is not C++ CPU',
        'target_band_cycles_per_sample':[0,.2],'stress_upper_cycles_per_sample':.45,'rows':rows},indent=2),encoding='utf-8')
    for r in rows:
        print(r['rate'],r['kernel'],r['band_magnitude_error_db'],r['band_phase_error_rad'],r['target_pass'])

if __name__ == '__main__':
    main()
