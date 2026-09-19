"""LC-F0/1/2 balance comparison through actual Fluid renderer; no default selection."""
from __future__ import annotations
import argparse
import copy
import json
from pathlib import Path
import subprocess
import numpy as np
import soundfile as sf
from listening_handoff import inspect, measures, rms, db

BALANCES = {'LC-F0':(.26,.24,.06),'LC-F1':(.28,.26,.04),'LC-F2':(.30,.27,.03)}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cases-executable',type=Path,required=True)
    parser.add_argument('--renderer',type=Path,required=True)
    parser.add_argument('--input',type=Path,action='append',required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    if args.output.exists() or not 1 <= len(args.input) <= 5: parser.error('New output; one to five inputs')
    inputs=[(p.resolve(),*inspect(p)) for p in args.input]
    if sum(x.size for _,x,_ in inputs)>8_000_000: parser.error('Split source batch')
    manifest=json.loads(subprocess.run([str(args.cases_executable.resolve())],check=True,capture_output=True,text=True).stdout)
    cases=[c for c in manifest['cases'] if c['mode']=='abd' and c['macro']=='size']
    assert len(cases)==3 and manifest['seed']==42
    output=args.output.resolve();output.mkdir(parents=True)
    (output/'mapped-cases.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    rows=[]
    for index,(path,source,rate) in enumerate(inputs,1):
        folder=output/f'input-{index}';folder.mkdir()
        for case in cases:
            triplet=[]
            for name,gains in BALANCES.items():
                key=f"{name}-size-{case['value']:g}"
                config=copy.deepcopy(case['config'])
                for module,gain in zip(('bubble','droplet','flow'),gains): config[module]['residualGain']=gain
                config_path=folder/f'{key}.json';config_path.write_text(json.dumps(config)+'\n',encoding='utf-8')
                components={}
                for mode in ('a','b','d','abd'):
                    destination=folder/f'{key}-{mode}-E.wav'
                    subprocess.run([str(args.renderer.resolve()),str(path),str(destination),mode+'-residual','128','42',str(config_path),'3'],check=True,capture_output=True)
                    c,actual_rate=sf.read(destination,dtype='float32',always_2d=True)
                    assert actual_rate==rate and c.shape==(len(source)+3*rate,source.shape[1]) and np.isfinite(c).all()
                    components[mode]=c
                c=components['abd']
                expected=sum(components[m].astype(np.float64) for m in ('a','b','d'))
                assert np.max(abs(expected-c))<1e-7
                x=np.zeros_like(c);x[:len(source)]=source
                sf.write(folder/f'{key}-fixed-Full-Reference-output-18.wav',(x.astype(np.float64)+c)*10**(-18/20),rate,subtype='FLOAT')
                row={'source':path.name,'size':case['value'],'balance':name,'gains':gains,
                     'components':{m:measures(data,rate,len(source),rms(source)) for m,data in components.items()},
                     'finite':True,'component_sum_matches':True,'human_review':'NOT ASSESSED'}
                triplet.append((key,c,row))
            target=min(rms(c[:len(source)]) for _,c,_ in triplet)
            for key,c,row in triplet:
                gain=target/max(rms(c[:len(source)]),1e-30)
                sf.write(folder/f'{key}-RMS-matched-support-ABD.wav',c.astype(np.float64)*gain,rate,subtype='FLOAT')
                row['matched_gain_db']=db(gain);rows.append(row)
    report={'phase':5,'mapping_revision':manifest['mappingRevision'],'seed':42,'protect':'OFF',
            'selection':'NONE; default LC0 unchanged','human_review':'NOT ASSESSED',
            'matching':'post-render attenuation-only RMS within same source/Size balance triplet',
            'rows':rows}
    (output/'report.json').write_text(json.dumps(report,indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(f'PASS: {len(rows)} balance/Size rows, actual ABD and component sum identity')


if __name__=='__main__': main()
