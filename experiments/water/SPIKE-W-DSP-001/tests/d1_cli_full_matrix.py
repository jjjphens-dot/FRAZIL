"""Actual D1 renderer integration, strict schema and old-mode isolation."""
import json
from pathlib import Path
import subprocess
import sys
import numpy as np
import soundfile as sf
from bubble_a1_cli_test import run


from cli_support import workspace


from cli_support import run_main

def main():
    renderer = Path(sys.argv[1]).resolve()
    descriptor = json.loads(subprocess.run([str(renderer),'--describe-flow-d1'],check=True,capture_output=True,text=True).stdout)
    assert descriptor == json.loads((Path(__file__).resolve().parents[2]/'contracts/flow-d1-v1.json').read_text(encoding='utf-8'))
    with workspace() as root:
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


if __name__ == "__main__":
    run_main(main)
