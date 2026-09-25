"""D1 source-transfer comparison using the existing renderer; no listening decisions."""
import argparse
import csv
import json
from pathlib import Path
import subprocess
import numpy as np
import soundfile as sf
from listening_handoff import inspect, rms, measures


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--renderer',type=Path,required=True)
    p.add_argument('--input',type=Path,action='append',required=True)
    p.add_argument('--engineering-input',type=Path,action='append',default=[])
    p.add_argument('--diagnostics',action='store_true')
    p.add_argument('--output',type=Path,required=True)
    args=p.parse_args()
    if args.output.exists() or not 1<=len(args.input)<=6: p.error('NEW output and 1..6 authorized inputs required')
    renderer=args.renderer.resolve()
    inputs=[(x.resolve(),*inspect(x)) for x in args.input]
    if sum(a.size for _,a,_ in inputs)>8_000_000: p.error('Split bounded batch')
    engineering={x.resolve() for x in args.engineering_input}
    if not engineering.issubset({x for x,_,_ in inputs}): p.error('Engineering labels must name supplied inputs')
    repo=Path(__file__).resolve().parents[4]
    if args.diagnostics:
        inputs += [(x.resolve(),*inspect(x)) for x in sorted((repo/'testdata/input').glob('*.wav'))]
    output=args.output.resolve();output.mkdir(parents=True)
    defaults={}
    for name,root in [('bubble-a1','bubbleA1'),('droplet-b1','dropletB1'),('flow-d1','flowD1')]:
        d=json.loads(subprocess.check_output([str(renderer),'--describe-'+name],text=True))
        defaults[root]={'version':d['configVersion'],**{x['name']:x['default'] for x in d['parameters'] if x['writable']}}
    cases=[('AB','a1b1',None),('AB-D0','a1b1d',None)]
    for u in (0,.05,.20,.50):
        for a in (.005,.015,.030): cases.append((f'A-U{u:g}-A{a:g}','a1b1d1',(u,.03,a)))
    for l in (.01,.03,.10): cases.append((f'B-L{l:g}','a1b1d1',(.20,l,.015)))
    configs={}
    for name,mode,values in cases:
        c={k:v.copy() for k,v in defaults.items() if k!='flowD1'}
        if values is not None:
            c['flowD1']=dict(zip(('velocityScaleMps','virtualStructureLengthMeters','maxExcessPathMeters'),values),version=1)
        path=output/f'{name}.json';path.write_text(json.dumps(c,indent=2),encoding='utf-8');configs[name]=path
    report={'status':'RESEARCH ONLY / HUMAN NOT ASSESSED','seed':42,'block':128,'tail_seconds':3,
            'study_B_selection':'predeclared middle engineering probe; not human-selected usable condition',
            'cases':[{'name':n,'mode':m,'config':json.loads(configs[n].read_text())} for n,m,_ in cases],
            'renders':[],'sources':[]}
    forms=[]
    for index,(source,audio,rate) in enumerate(inputs,1):
        diagnostic=source.parent==repo/'testdata/input'
        selected=cases if not diagnostic else [cases[0],cases[1],next(c for c in cases if c[0]=='B-L0.03')]
        folder=output/f'input-{index}';folder.mkdir()
        role='canonical engineering diagnostic' if diagnostic else 'engineering pad' if source in engineering else 'user supplied musical sample'
        report['sources'].append({'name':source.name,'role':role,'rate':rate,'source_frames':len(audio),
          'channels':audio.shape[1],'redistribution':'not authorized; local use only' if not diagnostic else 'repository diagnostic license',
          'permission':'user authorized local evaluation','sample_policy':'original samples; no source normalization/trim'})
        padded=np.pad(audio,((0,3*rate),(0,0)))
        results={}
        for name,mode,_ in selected:
            audios=[];stats={}
            for block in (128,257):
                target=folder/f'{name}-{block}-E.wav'
                cmd=[str(renderer),str(source),str(target),mode+'-residual',str(block),'42',str(configs[name]),'3']
                proc=subprocess.run(cmd,capture_output=True,text=True)
                target.with_suffix('.log').write_text(proc.stdout+proc.stderr,encoding='utf-8')
                if proc.returncode: raise RuntimeError(f'Render {name} failed {proc.returncode}; logs retained')
                y,actual_rate=sf.read(target,always_2d=True)
                assert actual_rate==rate and y.shape==padded.shape and np.isfinite(y).all()
                audios.append(y)
                stats=dict(t.split('=',1) for t in proc.stdout.split() if '=' in t)
            assert np.array_equal(*audios)
            y=audios[0];results[name]=y
            baseline=results.get('AB',y)
            difference=y-baseline
            report['renders'].append({'source':source.name,'role':role,'case':name,'mode':mode,
                'file':(folder/f'{name}-128-E.wav').relative_to(output).as_posix(),'finite':True,'partition_exact':True,
                'metrics':{k:v for k,v in measures(y,rate,len(audio),rms(audio)).items() if not k.startswith('focus')},
                'difference_from_AB_rms':rms(difference),'dc_per_channel':np.mean(y,axis=0).tolist(),
                'peak_linear':float(np.max(np.abs(y))),'rms_linear':rms(y),'diagnostics':stats})
            if mode=='a1b1d1':
                c=json.loads(configs[name].read_text())['flowD1']
                assert float(stats['d1_max_path_m'])<=c['maxExcessPathMeters']+1e-12
                assert float(stats['d1_max_speed_mps'])<=c['velocityScaleMps']+1e-9
                if c['velocityScaleMps']==0: assert np.array_equal(y,baseline)
        maximum=max(np.max(np.abs(padded+y)) for y in results.values())
        gain=min(1.,.9/maximum) if maximum else 1.
        report['sources'][-1]['common_monitor_gain']=float(gain)
        sf.write(folder/'Source.wav',padded*gain,rate,subtype='FLOAT')
        for name,y in results.items(): sf.write(folder/f'{name}-Full.wav',(padded+y)*gain,rate,subtype='FLOAT')
        if not diagnostic:
            forms.append({'reviewer':'','date':'','environment':'','monitor_level':'','source':source.name,'role':role,
              'evidence':'fixed-source Full primary','files':' | '.join((folder/f'{n}-Full.wav').relative_to(output).as_posix() for n in results),
              'continuity':'','source_rhythm_attacks':'','pitch_centre':'','fluid_fusion':'',
              **{k:'' for k in ('F01','F02','F03','F04','F09','F10','F13','F14','F16')},'decision':'NOT ASSESSED','notes':''})
        (output/'report.json').write_text(json.dumps(report,indent=2,allow_nan=False),encoding='utf-8')
    for reviewer in (1,2):
        with (output/f'REVIEWER_{reviewer}.csv').open('x',newline='',encoding='utf-8') as f:
            w=csv.DictWriter(f,fieldnames=forms[0].keys());w.writeheader();w.writerows(forms)
    print('D1 rows',len(report['renders']),'finite/exact partition PASS; human NOT ASSESSED')

if __name__=='__main__':main()
