"""Decay support material using exported mapper targets and actual bounded C3 renders."""
from __future__ import annotations
import argparse
import copy
import json
from pathlib import Path
import subprocess
import numpy as np
import soundfile as sf
from listening_handoff import db, inspect, measures, rms


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cases-executable', type=Path, required=True)
    parser.add_argument('--renderer', type=Path, required=True)
    parser.add_argument('--input', type=Path, action='append', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or not 1 <= len(args.input) <= 5:
        parser.error('Use a new output directory and one to five explicit inputs')
    inputs = [(p.resolve(), *inspect(p)) for p in args.input]
    if sum(x.size for _, x, _ in inputs) > 8_000_000:
        parser.error('Split inputs: maximum eight million channel samples')
    manifest = json.loads(subprocess.run([str(args.cases_executable.resolve())],
        check=True, capture_output=True, text=True).stdout)
    cases = [c for c in manifest['cases'] if c['mode'] == 'c' and c['macro'] == 'decay']
    assert [c['config']['modal']['decaySeconds'] for c in cases] == [.03, .12, .48]
    # Check actual mapper ownership rather than reproducing its formulas in this script.
    invariant = []
    for c in cases:
        config = copy.deepcopy(c['config'])
        for module in ('bubble', 'droplet', 'modal'):
            del config[module]['decaySeconds']
        invariant.append(config)
    assert invariant[0] == invariant[1] == invariant[2]
    assert manifest['seed'] == 42
    output = args.output.resolve()
    output.mkdir(parents=True)
    (output/'mapped-cases.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    rows = []
    for index, (path, source, rate) in enumerate(inputs, 1):
        folder = output/f'input-{index}'
        folder.mkdir()
        renders = []
        for case in cases:
            key = f"decay-{case['value']:g}"
            config = folder/f'{key}.json'
            config.write_text(json.dumps(case['config'], indent=2)+'\n', encoding='utf-8')
            driver_path = folder/f'{key}-excitation.wav'
            c_path = folder/f'{key}-fixed-WaterOnly-Focus18-output-18.wav'
            result = subprocess.run([str(args.renderer.resolve()), str(path), str(c_path),
                'c-residual', '128', '42', str(config), '3', '-', 'hard', str(driver_path), 'c3'],
                check=True, capture_output=True, text=True)
            c, actual_rate = sf.read(c_path, dtype='float32', always_2d=True)
            driver, _ = sf.read(driver_path, dtype='float32', always_2d=True)
            assert actual_rate == rate and c.shape == (len(source)+3*rate, source.shape[1])
            assert np.isfinite(c).all() and np.array_equal(driver[:len(source)], source)
            assert not np.any(driver[len(source):])
            fields = dict(v.split('=', 1) for v in result.stdout.split() if '=' in v)
            x = np.zeros_like(c)
            x[:len(source)] = source
            monitor = (x.astype(np.float64)+c)*10**(-18/20)
            sf.write(folder/f'{key}-fixed-Full-Reference-output-18.wav', monitor, rate, subtype='FLOAT')
            metrics = measures(c, rate, len(source), rms(source))
            row = {'source': path.name, 'macro_value': case['value'], 'decay_seconds': case['config']['modal']['decaySeconds'],
                'rate': rate, 'C': metrics, 'driver_peak': float(np.max(np.abs(driver))),
                'driver_source_rms_dbfs': db(rms(driver[:len(source)])),
                'full_reference_peak_dbfs': db(np.max(np.abs(monitor))),
                'normalization_bound': float(fields['modal_bound']), 'finite': True,
                'reviewer_1': 'NOT ASSESSED', 'reviewer_2': 'NOT ASSESSED'}
            renders.append((key, c, row))
        # Post-render attenuation only, common source-window RMS target for this source triplet.
        target = min(rms(c[:len(source)]) for _, c, _ in renders)
        for key, c, row in renders:
            gain = target/max(rms(c[:len(source)]), 1e-30)
            matched = c.astype(np.float64)*gain
            sf.write(folder/f'{key}-RMS-matched-support-WaterOnly.wav', matched, rate, subtype='FLOAT')
            row.update({'matched_gain_db': db(gain), 'matched_source_rms_dbfs': db(rms(matched[:len(source)]))})
            rows.append(row)
        assert max(r['matched_source_rms_dbfs'] for _, _, r in renders)-min(r['matched_source_rms_dbfs'] for _, _, r in renders) < 1e-5
    report = {'phase': 3, 'mapping_revision': manifest['mappingRevision'], 'normalization': 'C3',
        'conditioner': 'hard; identity on these full-scale-bounded sources, not selected default',
        'seed': 42, 'protect': 'OFF', 'mapper_non_decay_destinations_unchanged': True,
        'matching': 'attenuation only after rendering, source-window RMS within source triplet; not LUFS',
        'interpretation': 'Fixed Full for masking/preservation; matched Water Only for persistence support only',
        'human_review': 'NOT ASSESSED', 'rows': rows}
    (output/'report.json').write_text(json.dumps(report, indent=2, allow_nan=False)+'\n', encoding='utf-8')
    print(f'PASS: {len(rows)} Decay renders; mapper isolation, driver identity, separate RMS support')


if __name__ == '__main__':
    main()
