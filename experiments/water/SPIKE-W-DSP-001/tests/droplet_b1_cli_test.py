"""Real renderer, strict B1 schema/descriptor and decoded audio invariants."""
import json
import csv
from pathlib import Path
import subprocess
import sys
import tempfile
import numpy as np
import soundfile as sf


def render(exe, source, output, config, mode="b1-residual", block=128, ok=True):
    p = subprocess.run([str(exe), str(source), str(output), mode, str(block), "42", str(config), "2"], capture_output=True, text=True)
    if not ok:
        assert p.returncode != 0 and not output.exists(), (p.stdout, p.stderr)
        return None
    assert p.returncode == 0, (p.returncode, p.stdout, p.stderr)
    audio, rate = sf.read(output, always_2d=True)
    assert np.isfinite(audio).all()
    stats = dict(t.split("=", 1) for t in p.stdout.split() if "=" in t)
    return audio, stats


def main():
    exe = Path(sys.argv[1]).resolve()
    experiment = Path(__file__).resolve().parents[1]
    descriptor = json.loads(subprocess.check_output([str(exe), "--describe-droplet-b1"], text=True))
    assert descriptor == json.loads((experiment.parent / "contracts/droplet-b1-v1.json").read_text())
    build = experiment.parents[2] / "build/droplet-b1"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=build, prefix="cli-") as directory:
        root = Path(directory)
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
            config.write_text('{"dropletB1":{"version":1,"entrainmentProbability":0}}')
            zero, stats = render(exe, source, root/f"{rate}-p0.wav", config)
            assert not np.any(zero) and int(stats["eligible"]) > 0 and stats["admitted"] == "0"
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
        # Exercise the real study builder. Using this executable as both versions only
        # tests pack mechanics; historical identity requires the separate baseline run.
        sf.write(source, np.zeros((4410, 2)), 44100, subtype="FLOAT")
        pack = root / "pack"
        subprocess.run([sys.executable, str(experiment/"render/droplet_b1_study.py"),
                        "--renderer", str(exe), "--baseline-renderer", str(exe),
                        "--input", str(source), "--engineering-input", str(source),
                        "--output", str(pack)], check=True, capture_output=True, text=True)
        report = json.loads((pack/"report.json").read_text())
        assert len(report["cases"]) == 26 and len(report["legacy_regressions"]) == 10
        assert all(r["repeat_partition_exact"] for r in report["renders"])
        assert all(not r["assessable"] for r in report["matching"])
        for reviewer in (1, 2):
            with (pack/f"reviewer-{reviewer}.csv").open(encoding="utf-8-sig") as f:
                rows = list(csv.DictReader(f))
            assert len(rows) == 30 and all(r["decision"] == "NOT ASSESSED" and not r["reviewer"] for r in rows)
    print("B1 CLI descriptor/config/stereo/rate/partition/composition PASS")


if __name__ == "__main__":
    main()
