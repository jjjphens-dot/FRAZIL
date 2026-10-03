"""Real renderer, strict B1 schema/descriptor and decoded audio invariants."""
import json
from pathlib import Path
import subprocess
import sys
import numpy as np
import soundfile as sf


from b1_cli_support import render
from cli_support import workspace


from cli_support import run_main

def main():
    exe = Path(sys.argv[1]).resolve()
    experiment = Path(__file__).resolve().parents[1]
    descriptor = json.loads(subprocess.check_output([str(exe), "--describe-droplet-b1"], text=True))
    assert descriptor == json.loads((experiment.parent / "contracts/droplet-b1-v1.json").read_text())
    with workspace() as root:
        config = root / "config.json"
        source = root / "source.wav"
        rate = 48000
        t = np.arange(int(rate * .1)) / rate
        x = np.where(t < .002, .7, 0)
        sf.write(source, np.column_stack((x, x * 0)), rate, subtype="FLOAT")
        config.write_text('{"dropletB1":{"version":1}}')
        reference, stats = render(exe, source, root / "reference.wav", config)
        odd, other = render(exe, source, root / "odd.wav", config, block=257)
        assert reference.shape == (len(x) + 2 * rate, 2)
        assert np.any(reference[:, 0]) and not np.any(reference[:, 1])
        assert np.array_equal(reference, odd) and stats["eligible"] == other["eligible"]


if __name__ == "__main__":
    run_main(main)
