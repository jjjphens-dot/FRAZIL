"""Measure a test-only C++ realization of already screened offline candidates.

Coefficient tables are prepare-owned and linearly interpolated on a fixed 4096-cell
grid. This extra approximation is compared with the un-tabulated offline model and
the separately converged Fourier oracle. No runtime candidate is selected here.
"""
import argparse
import csv
import subprocess
from pathlib import Path

import numpy as np

from flow_d1_latency_models import EPSILON, Conditioner, GuardKernel, conditioners
from flow_d1_latency_study import qualified_reference, sources
from flow_d1_remediation_study import csv_write, metrics


def run_native(executable, directory, conditioner, kernel, audio, delays, quick=False):
    directory.mkdir(parents=True, exist_ok=False)
    positions = np.linspace(0, 1, 4097)
    positions[-1] = 1-1e-12
    _, table = kernel.coefficients(positions)
    table[-1] = 0
    table[-1, kernel.guard+1] = 1
    fir = [] if conditioner.fir is None else conditioner.fir
    sos = [] if conditioner.sos is None else conditioner.sos
    with (directory/'coefficients.txt').open('w', encoding='utf-8') as stream:
        stream.write(f'{kernel.guard} {conditioner.latency} {len(fir)} {len(sos)}\n')
        for values in (fir, sos, table):
            np.savetxt(stream, np.asarray(values).reshape(-1), fmt='%.17g')
    np.savetxt(directory/'input.txt', np.column_stack((delays, audio)), fmt='%.17g')
    result = subprocess.run([str(executable), str(directory/'coefficients.txt'),
                             str(directory/'input.txt'), str(directory/'output.csv'),
                             str(directory/'resource.csv')]+(['--quick'] if quick else []), capture_output=True, text=True,
                            timeout=240)
    (directory/'native.log').write_text(result.stdout+result.stderr, encoding='utf-8')
    if result.returncode:
        raise RuntimeError(f'native candidate failed ({result.returncode}): {directory}\n'
                           f'{(result.stdout+result.stderr)[-4000:]}')
    output = np.loadtxt(directory/'output.csv', delimiter=',')
    latency = conditioner.latency+kernel.guard
    aligned = output[latency:]
    # Continue filter state into zero input before cropping. In particular, FIR
    # pre-ringing/guard support must not see a prematurely truncated filter tail.
    extended = conditioner.aligned(np.pad(audio, ((0, 128), (0, 0))))
    expected = kernel.apply(extended, np.pad(delays, (0, 128)))[:len(audio)]
    values = metrics(aligned, expected)
    if not values['finite'] or max(values['relative_rms_error'], values['peak_normalized_error']) > 1e-6:
        raise AssertionError(f'Native/model mismatch: {directory}: {values}')
    return aligned, values, list(csv.DictReader((directory/'resource.csv').open()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native', type=Path, required=True)
    parser.add_argument('--sources', type=Path, required=True)
    parser.add_argument('--selection', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    rows, resources = [], []
    with args.selection.open() as stream:
        selected = [r for r in csv.DictReader(stream) if r['minimum_passing_guard'] == 'True']
    # Every minimum-guard numerical combination, no latency/CPU-based filter choice.
    reference_key, references = None, {}
    for row in selected:
        rate = int(row['rate'])
        key = (rate, row['conditioner'])
        if key != reference_key:
            reference_key, references = key, {}
        if row['conditioner'] == 'raw-control':
            # Exactly [1], used only to transport the unfiltered control through
            # the same native harness. This is not a new filter-screen candidate.
            conditioner = Conditioner(rate, 16000, 18000, 1)
            conditioner.name = 'raw-control'
        else:
            conditioner = next(c for c in conditioners(rate) if c.name == row['conditioner'])
        family, guard = row['kernel'].split('-g')
        kernel = GuardKernel(family, int(guard))
        for profile, source, audio, delay in sources(args.sources, rate):
            # Native realization is linear stereo; test actual simultaneous AB in
            # all four profiles. Offline screen already checks isolated A1 and B1.
            if source != 'AB':
                continue
            label = f'{rate}-{conditioner.name}-{kernel.name}-{profile}'
            y, implementation, measurements = run_native(args.native.resolve(), args.output/label,
                                                         conditioner, kernel, audio, delay,
                                                         quick=profile != 'overlap-reference')
            if profile not in references:
                references[profile] = qualified_reference(conditioner.aligned(audio), delay)
            reference, convergence = references[profile]
            error = metrics(y, reference)
            passed = (convergence <= EPSILON/10 and error['finite'] and
                      max(error['relative_rms_error'], error['peak_normalized_error']) <= EPSILON)
            rows.append(dict(rate=rate, conditioner=conditioner.name, kernel=kernel.name, profile=profile,
                             **error, reference_convergence=convergence,
                             implementation_nrms=implementation['relative_rms_error'],
                             implementation_peak_error=implementation['peak_normalized_error'],
                             native_numerical_pass=passed))
            for measurement in measurements:
                resources.append(dict(rate=rate, conditioner=conditioner.name, kernel=kernel.name,
                                      profile=profile, **measurement,
                                      total_latency_samples=conditioner.latency+kernel.guard,
                                      scope='candidate only; not complete A1+B1/plugin callback'))
            csv_write(args.output/'NATIVE.csv', rows)
            csv_write(args.output/'RESOURCES.csv', resources)
        print('native', label.rsplit('-', 1)[0], 'complete', flush=True)
    if not rows or not all(r['native_numerical_pass'] for r in rows):
        raise SystemExit('One or more native candidates failed; no adoption')


if __name__ == '__main__':
    main()
