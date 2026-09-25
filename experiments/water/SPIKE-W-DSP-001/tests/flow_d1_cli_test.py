"""Actual D1 renderer integration, strict schema and old-mode isolation."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import numpy as np
import soundfile as sf
from bubble_a1_cli_test import run


def main():
    renderer = Path(sys.argv[1]).resolve()
    descriptor = json.loads(subprocess.run([str(renderer),'--describe-flow-d1'],check=True,capture_output=True,text=True).stdout)
    assert descriptor == json.loads((Path(__file__).resolve().parents[2]/'contracts/flow-d1-v1.json').read_text(encoding='utf-8'))
    build = Path(__file__).resolve().parents[4]/'build/flow-d1'
    build.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(dir=build,prefix='cli-') as temp:
        root = Path(temp)
        config = root/'config.json'
        source = root/'input.wav'
        counter = 0
        def render(mode, values, block=128, ok=True):
            nonlocal counter
            counter += 1
            config.write_text(json.dumps(values),encoding='utf-8')
            return run(renderer,source,root/f'{counter}.wav',config,block,mode,ok)
        for rate in (44100,48000,96000):
            n=np.arange(int(.18*rate))
            x=.6*np.sin(2*np.pi*197*n/rate)*(n%int(rate*.04)<int(rate*.015))
            sf.write(source,np.column_stack((x,np.zeros(len(x)))),rate,subtype='FLOAT')
            for base,mode in [('a1','a1d1'),('b1','b1d1'),('a1b1','a1b1d1')]:
                old,_=render(base+'-residual',{})
                for off in [{'velocityScaleMps':0},{'maxExcessPathMeters':0}]:
                    identity,_=render(mode+'-residual',{'flowD1':{'version':1,**off}})
                    assert np.array_equal(old,identity)
                reference,stats=render(mode+'-residual',{})
                assert not np.array_equal(old,reference), mode
                assert np.all(reference[:,1]==0)
                fields=dict(t.split('=',1) for t in stats.split() if '=' in t)
                assert fields['flow_model']=='D1'
                assert 0 <= float(fields['d1_max_path_m']) <= .015
                assert float(fields['d1_max_speed_mps']) <= .2+1e-10
                explicit,_=render(mode+'-residual',{'flowD1':{'version':1,**{p['name']:p['default'] for p in descriptor['parameters']}}})
                assert np.array_equal(reference,explicit)
                for block in (32,64,256,257,512,1024):
                    other,_=render(mode+'-residual',{},block)
                    assert np.array_equal(reference,other)
                full,_=render(mode,{})
                padded=np.zeros_like(full); padded[:len(x),0]=x.astype(np.float32)
                expected=(padded.astype(np.float32)+reference.astype(np.float32)).astype(np.float32)
                assert np.array_equal(full,expected)
            sf.write(source,np.zeros((100,2)),rate,subtype='FLOAT')
            silent,_=render('a1b1d1-residual',{})
            assert np.all(silent==0)
        for legacy in ('baseline','a','b','d','ab','ad','bd','abd','c','a1','b1','a1b1','a1b1d'):
            render(legacy,{'flowD1':{'version':1}},ok=False)
        for bad in ({},{'version':0},{'version':2},{'version':1,'delaySeconds':.001},
                    {'version':1,'velocityScaleMps':True},{'version':1,'velocityScaleMps':'0.2'}):
            render('a1b1d1',{'flowD1':bad},ok=False)
        for p in descriptor['parameters']:
            for value in (p['minimum']-.001,p['maximum']+.001,float('nan'),float('inf')):
                render('a1b1d1',{'flowD1':{'version':1,p['name']:value}},ok=False)
        for badmode in ('d1','d1-residual','ad1','bd1'):
            render(badmode,{},ok=False)
        render('a1b1d1',{'protect':{'depth':.5}},ok=False)
    print('D1 actual renderer schema, composition, rate/partition and legacy isolation PASS')

if __name__=='__main__':
    main()
