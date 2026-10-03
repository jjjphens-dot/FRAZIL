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
        probe = Path(sys.argv[2]).resolve()
        native = root/'native'
        subprocess.run([str(probe), str(native)], check=True, capture_output=True, text=True)
        for rate in (44100,48000,96000):
            audit=np.loadtxt(native/f'{rate}-overlap-reference-audit.csv',delimiter=',',skiprows=1)
            raw=np.loadtxt(native/f'{rate}-overlap-reference.csv',delimiter=',',skiprows=1)
            assert np.any((raw[:,1]!=0)&(raw[:,3]!=0))
            sf.write(source,audit[:,:2],rate,subtype='FLOAT')
            values={'flowD1':{'version':1,'velocityScaleMps':1,
                              'virtualStructureLengthMeters':.005,'maxExcessPathMeters':.05}}
            residual,_=render('a1b1d1-residual',values)
            expected=audit[:,4:6].astype(np.float32)
            # Independent native source/transfer harness, not another renderer mode:
            # duplicate direct AB in both full/residual must fail this comparison.
            assert np.array_equal(residual[:rate],expected)
            full,_=render('a1b1d1',values)
            combined=(audit[:,:2].astype(np.float32)+expected).astype(np.float32)
            assert np.array_equal(full[:rate],combined)


if __name__ == "__main__":
    run_main(main)
