"""R-M1 versus legacy Motion using actual exported targets and C3; no adoption decision."""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import subprocess
import numpy as np
import soundfile as sf
from listening_handoff import inspect, measures, rms, db


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cases-executable', type=Path, required=True)
    parser.add_argument('--renderer', type=Path, required=True)
    parser.add_argument('--input', type=Path, action='append', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or not 1 <= len(args.input) <= 5:
        parser.error('Use new output and at most five inputs')
    inputs = [(p.resolve(), *inspect(p)) for p in args.input]
    if sum(x.size for _, x, _ in inputs) > 8_000_000:
        parser.error('Split source batch')
    manifest = json.loads(subprocess.run([str(args.cases_executable.resolve())], check=True,
        capture_output=True, text=True).stdout)
    cases = [c for c in manifest['cases'] if c['mode'] == 'c' and c['macro'] == 'motion']
    assert [c['value'] for c in cases] == [0, .5, 1] and manifest['seed'] == 42
    assert all(c['config']['modal']['rootFrequencyHz'] == 260 and
               c['config']['modal']['decaySeconds'] == .12 and
               c['config']['modal']['residualGain'] == .3 for c in cases)
    output = args.output.resolve()
    output.mkdir(parents=True)
    (output/'mapped-cases.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    rows = []
    for index, (path, source, rate) in enumerate(inputs, 1):
        folder = output/f'input-{index}'
        folder.mkdir()
        for carrier in ('hard', 'feature'):
            stable = None
            for policy in ('independent', 'structured'):
                triplet = []
                for case in cases:
                    key = f"{carrier}-{policy}-motion-{case['value']:g}"
                    config = folder/f'{key}.json'
                    config.write_text(json.dumps(case['config'])+'\n', encoding='utf-8')
                    c = None
                    for block in (128,257):
                        destination = folder/f'{key}-block{block}-E.wav'
                        subprocess.run([str(args.renderer.resolve()), str(path), str(destination),
                            'c-residual', str(block), '42', str(config), '3', '-', carrier, '-', 'c3', policy],
                            check=True, capture_output=True, text=True)
                        actual, actual_rate = sf.read(destination, dtype='float32', always_2d=True)
                        assert actual_rate == rate and actual.shape == (len(source)+3*rate, source.shape[1])
                        assert np.isfinite(actual).all()
                        if c is not None: assert np.array_equal(c,actual)
                        c = actual
                    if case['value'] == 0:
                        if stable is not None: assert np.array_equal(stable,c)
                        stable = c
                    x = np.zeros_like(c)
                    x[:len(source)] = source
                    sf.write(folder/f'{key}-fixed-Full-Reference-output-18.wav',
                             (x.astype(np.float64)+c)*10**(-18/20), rate, subtype='FLOAT')
                    a = stable[:len(source)].astype(np.float64).reshape(-1)
                    b = c[:len(source)].astype(np.float64).reshape(-1)
                    scale = float(np.dot(a,b)/max(np.dot(a,a),1e-30))
                    spectrum = np.sum(abs(np.fft.rfft(c[:len(source)], axis=0))**2,axis=1)
                    frequencies = np.fft.rfftfreq(len(source),1/rate)
                    row = {'source':path.name,'carrier':carrier,'policy':policy,'motion':case['value'],
                           'C':measures(c,rate,len(source),rms(source)),
                           'difference_from_motion0_rms_dbfs':db(rms(c-stable)),
                           'best_scalar_fit_to_motion0':scale,
                           'remaining_difference_after_scalar_fit_ratio':rms(b-scale*a)/max(rms(b),1e-30),
                           'energy_fraction_above_600Hz':float(spectrum[frequencies>=600].sum()/max(spectrum.sum(),1e-30)),
                           'finite':True,'partition_exact':True,'human_review':'NOT ASSESSED'}
                    triplet.append((key,c,row))
                target = min(rms(c[:len(source)]) for _,c,_ in triplet)
                for key,c,row in triplet:
                    gain = target/max(rms(c[:len(source)]),1e-30)
                    sf.write(folder/f'{key}-RMS-matched-support-WaterOnly.wav', c.astype(np.float64)*gain, rate, subtype='FLOAT')
                    row['matched_gain_db'] = db(gain)
                    rows.append(row)
    report = {'phase':4,'normalization':'C3','mapping_revision':manifest['mappingRevision'],
              'seed':42,'protect':'OFF','selection':'NONE','human_review':'NOT ASSESSED',
              'matching':'attenuation-only source-window RMS within carrier/policy/source triplet',
              'rows':rows}
    (output/'report.json').write_text(json.dumps(report,indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(f'PASS: {len(rows)} Motion rows, partition and stable legacy equality')


if __name__ == '__main__':
    main()
