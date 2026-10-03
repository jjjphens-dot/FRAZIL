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
        sf.write(source,np.zeros((100,2)),48000,subtype="FLOAT")
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


if __name__ == "__main__":
    run_main(main)
