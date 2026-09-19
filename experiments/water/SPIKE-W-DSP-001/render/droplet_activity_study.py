"""Droplet onset-probability isolation; actual B/ABD, legacy-one reference, no curve adoption."""
from __future__ import annotations
import argparse,copy,json,subprocess
from pathlib import Path
import numpy as np
import soundfile as sf
from listening_handoff import inspect,measures,rms


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cases-executable',type=Path,required=True)
    p.add_argument('--renderer',type=Path,required=True)
    p.add_argument('--input',type=Path,action='append',required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--legacy-balance-pack',type=Path)
    args=p.parse_args()
    if args.output.exists() or not 1<=len(args.input)<=5:p.error('New output and one to five inputs')
    inputs=[(path.resolve(),*inspect(path)) for path in args.input]
    if sum(audio.size for _,audio,_ in inputs)>8_000_000:p.error('Split source batch')
    manifest=json.loads(subprocess.run([str(args.cases_executable.resolve())],check=True,capture_output=True,text=True).stdout)
    case=next(c for c in manifest['cases'] if c['mode']=='abd' and c['macro']=='motion' and c['value']==.5)
    output=args.output.resolve();output.mkdir(parents=True)
    rows=[]
    for index,(path,source,rate) in enumerate(inputs,1):
        folder=output/f'input-{index}';folder.mkdir()
        previous=-1
        for probability in (0,.25,.5,1):
            config=copy.deepcopy(case['config']);config['droplet']['eventActivity']=probability
            config_path=folder/f'activity-{probability:g}.json';config_path.write_text(json.dumps(config)+'\n',encoding='utf-8')
            components={};fields={}
            for mode in ('b','abd'):
                reference=None
                for block in (128,257):
                    destination=folder/f'activity-{probability:g}-{mode}-block{block}.wav'
                    result=subprocess.run([str(args.renderer.resolve()),str(path),str(destination),mode+'-residual',str(block),'42',str(config_path),'3'],check=True,capture_output=True,text=True)
                    audio,actual_rate=sf.read(destination,dtype='float32',always_2d=True)
                    assert actual_rate==rate and np.isfinite(audio).all()
                    if reference is not None:assert np.array_equal(audio,reference)
                    reference=audio
                    fields=dict(item.split('=',1) for item in result.stdout.split() if '=' in item)
                components[mode]=reference
            count=int(fields['droplet_events'])
            assert count>=previous
            previous=count
            if probability==0:assert count==0 and not np.any(components['b'])
            legacy_verified=False
            if probability==1 and args.legacy_balance_pack:
                # Names/rates are checked before comparing prior actual legacy output.
                old_report=json.loads((args.legacy_balance_pack/'report.json').read_text(encoding='utf-8-sig'))
                assert list(dict.fromkeys(r['source'] for r in old_report['rows']))[index-1] == path.name
                for mode in ('b','abd'):
                    old,old_rate=sf.read(args.legacy_balance_pack/f'input-{index}'/f'LC-F0-size-0.5-{mode}-E.wav',dtype='float32',always_2d=True)
                    assert old_rate==rate and np.array_equal(old,components[mode])
                legacy_verified=True
            rows.append({'source':path.name,'probability':probability,'droplet_events':count,
                         'components':{m:measures(a,rate,len(source),rms(source)) for m,a in components.items()},
                         'finite':True,'partition_exact':True,'legacy_one_verified':legacy_verified})
    (output/'report.json').write_text(json.dumps({'phase':6,'seed':42,'protect':'OFF','macro_context':'fixed center; probability isolation only',
        'curve':'separate C++ candidate clamp(4*m*m,0,1); not adopted by v0.2','human_review':'NOT ASSESSED','rows':rows},indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(f'PASS: {len(rows)} activity rows; zero events, partition and optional legacy identity')


if __name__=='__main__':main()
