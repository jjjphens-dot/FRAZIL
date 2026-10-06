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
        sf.write(source, np.zeros((100, 2)), 48000, subtype="FLOAT")
        invalid = ['{"dropletB1":{}}', '{"dropletB1":{"version":2}}',
                   '{"dropletB1":{"version":1,"frequencyHz":1000}}',
                   '{"dropletB1":{"version":1,"voiceCapacity":33}}',
                   '{"dropletB1":{"version":1,"riseXi":0.07}}',
                   '{"dropletB1":{"version":1,"version":1}}',
                   '{"dropletB1":{"version":1}} trailing',
                   '{"dropletB1":{"version":1,"equivalentBubbleRadiusMm":NaN}}']
        for p in descriptor["parameters"]:
            if p["writable"]:
                invalid += [json.dumps({"dropletB1":{"version":1,p["name"]:p["minimum"]-1}}),
                            json.dumps({"dropletB1":{"version":1,p["name"]:p["maximum"]+1}})]
        for i, text in enumerate(invalid):
            config.write_text(text)
            render(exe, source, root/f"bad-{i}.wav", config, ok=False)
        config.write_text('{"dropletB1":{"version":1}}')
        for mode in ("baseline","b","bd","abd","a1","c"):
            render(exe, source, root/f"wrong-{mode}.wav", config, mode=mode, ok=False)
        config.write_text('{"dropletB1":{"version":1},"protect":{"depth":0.1}}')
        render(exe, source, root/"protect.wav", config, ok=False)


if __name__ == "__main__":
    run_main(main)
