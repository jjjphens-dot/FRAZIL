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
        rate = 48000
        n = np.arange(int(.1 * rate))
        x = .6 * np.sin(2 * np.pi * 197 * n / rate) * (n < 720)
        sf.write(source, np.column_stack((x, x * 0)), rate, subtype="FLOAT")
        reference, stats = render('a1b1d1-residual', {})
        odd, _ = render('a1b1d1-residual', {}, 257)
        # The shared A1/D1 renderer helper requests one second of tail.
        assert len(reference) == len(x) + rate
        assert np.any(reference[:, 0]) and np.all(reference[:, 1] == 0)
        assert np.array_equal(reference, odd) and 'flow_model=D1' in stats


if __name__ == "__main__":
    run_main(main)
