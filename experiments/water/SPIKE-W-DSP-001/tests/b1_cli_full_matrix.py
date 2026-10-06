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
        for rate in (44100, 48000, 96000):
            t = np.arange(int(rate * .8)) / rate
            left = np.where(t % .25 < .02, .7 * np.cos(2*np.pi*173*t), 0)
            pair = np.column_stack((left, -.3*left))
            sf.write(source, pair, rate, subtype="FLOAT")
            config.write_text('{"dropletB1":{"version":1}}')
            ref, stats = render(exe, source, root/f"{rate}-ref.wav", config)
            assert int(stats["eligible"]) > 1 and int(stats["started"]) == int(stats["admitted"])
            assert np.any(ref) and float(stats["frequency_hz"]) > 1600
            for block in (1, 7, 32, 64, 256, 257, 512, 1024):
                actual, other = render(exe, source, root/f"{rate}-{block}.wav", config, block=block)
                assert np.array_equal(ref, actual) and stats["eligible"] == other["eligible"]
            full, _ = render(exe, source, root/f"{rate}-full.wav", config, mode="b1")
            padded = np.pad(sf.read(source, always_2d=True)[0], ((0, 2*rate), (0, 0)))
            assert np.max(np.abs(full-(padded+ref))) < 1e-7
            empty = root/"empty.json"; empty.write_text("{}")
            a1, _ = render(exe, source, root/f"{rate}-a1.wav", empty, mode="a1-residual")
            d0, _ = render(exe, source, root/f"{rate}-d0.wav", empty, mode="d-residual")
            for mode, expected in (("a1b1", a1+ref), ("a1b1d", a1+ref+d0)):
                actual, _ = render(exe, source, root/f"{rate}-{mode}.wav", config, mode=mode+"-residual")
                assert np.max(np.abs(actual-expected)) < 1e-7
            for label, pair in (
                ("mono", np.column_stack((left,left))),
                ("left", np.column_stack((left,left*0))),
                ("right", np.column_stack((left*0,left))),
                ("anti", np.column_stack((left,-left))),
                ("unequal", np.column_stack((left,left*.03))),
                ("quad", np.column_stack((left,np.where(t%.25<.02,.7*np.sin(2*np.pi*173*t),0)))),
                ("left-transient", np.column_stack((left,left*.001))),
                ("right-transient", np.column_stack((left*.001,left))),
            ):
                sf.write(source, pair, rate, subtype="FLOAT")
                y, ys = render(exe, source, root/f"{rate}-{label}.wav", config)
                sf.write(source, pair[:,::-1], rate, subtype="FLOAT")
                z, zs = render(exe, source, root/f"{rate}-{label}-swap.wav", config)
                assert np.array_equal(y[:,::-1], z) and ys["eligible"] == zs["eligible"]
                if label == "left": assert not np.any(y[:,1])
                if label == "right": assert not np.any(y[:,0])
                if label == "mono": assert np.array_equal(y[:,0],y[:,1])
                if label == "anti": assert np.array_equal(y[:,0],-y[:,1])
            # A short source pulse ends well before the captured 40ms pinch-off time.
            pulse = np.zeros((int(rate * .1), 2))
            pulse[:int(rate * .002)] = [.9, -.3]
            sf.write(source, pulse, rate, subtype="FLOAT")
            config.write_text('{"dropletB1":{"version":1,"pinchOffDelayMs":40}}')
            _, timing = render(exe, source, root/f"{rate}-delay.wav", config)
            onset = int(timing["b1_first_source_onset_frame"])
            due = int(timing["b1_first_due_frame"])
            started = int(timing["b1_first_started_frame"])
            assert onset == int(timing["b1_first_eligible_frame"]) < due
            assert due == onset + int(np.ceil(rate * .04)) == started
            assert started >= len(pulse[:int(rate * .002)]) and int(timing["b1_start_on_zero_current_frame"]) > 0
            assert int(timing["b1_first_started_eligible_id"]) == 1
            assert timing["droplet_first_frame"] == timing["b1_first_started_frame"]
            assert timing["droplet_silent_events"] == timing["b1_start_on_zero_current_frame"]
            assert timing["physicalAmplitudeScale"] == timing["relativeFormationAmplitudeScale"]
            assert float(timing["lastStartedSourceExcitation"]) > 0
            config.write_text('{"dropletB1":{"version":1,"entrainmentProbability":0}}')
            zero, stats = render(exe, source, root/f"{rate}-p0.wav", config)
            assert not np.any(zero) and int(stats["eligible"]) > 0 and stats["admitted"] == "0"


if __name__ == "__main__":
    run_main(main)
