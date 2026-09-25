"""A1 actual renderer properties; all temporary files stay in the repository build tree."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

import numpy as np
import soundfile as sf


def run(renderer, source, output, config, block=128, mode="a1-residual", ok=True):
    result = subprocess.run([str(renderer), str(source), str(output), mode, str(block), "42",
                             str(config), "1"], capture_output=True, text=True)
    if ok:
        assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
        audio, rate = sf.read(output, always_2d=True)
        assert np.isfinite(audio).all()
        return audio, result.stdout
    assert result.returncode != 0 and not output.exists(), (result.returncode, result.stdout)


def main():
    renderer = Path(sys.argv[1]).resolve()
    descriptor = json.loads(subprocess.run([str(renderer), "--describe-bubble-a1"],
                                          check=True, capture_output=True, text=True).stdout)
    snapshot = Path(__file__).resolve().parents[2] / "contracts" / "bubble-a1-v2.json"
    assert descriptor == json.loads(snapshot.read_text(encoding="utf-8"))
    assert descriptor["modelVersion"] == descriptor["configVersion"] == 2
    assert len(descriptor["parameters"]) == 22
    assert len({p["name"] for p in descriptor["parameters"]}) == 22
    build = Path(__file__).resolve().parents[4] / "build" / "bubble-a1"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=build, prefix="cli-") as directory:
        root = Path(directory)
        config = root / "config.json"
        for rate in (44100, 48000, 96000):
            time = np.arange(int(rate * .25) + 3) / rate
            left = .7 * np.sin(2 * np.pi * 197 * time)
            source = root / "source.wav"
            sf.write(source, np.column_stack((left, -.3 * left)), rate, subtype="FLOAT")
            config.write_text('{"bubbleA1": {"version":2}}', encoding="utf-8")
            reference, stats = run(renderer, source, root / f"{rate}-base.wav", config)
            config.write_text(json.dumps({"bubbleA1": {"version": 2, **{
                p["name"]: p["default"] for p in descriptor["parameters"]}}}), encoding="utf-8")
            explicit, _ = run(renderer, source, root / f"{rate}-explicit-defaults.wav", config)
            assert np.array_equal(reference, explicit), "descriptor and typed defaults disagree"
            assert "bubble_model=A1" in stats and "radius_hist=" in stats
            fields = dict(token.split("=", 1) for token in stats.split() if "=" in token)
            assert int(fields["bubble_events"]) == int(fields["started"]) > 0
            assert int(fields["bubble_first_frame"]) == int(fields["a1_started_frame"]) >= 0
            assert 0 <= int(fields["a1_requested_frame"]) <= int(fields["a1_started_frame"])
            assert int(fields["a1_start_on_zero_current_frame"]) == int(fields["bubble_silent_events"])
            assert int(fields["a1_source_window_active"]) > 0
            assert np.square(reference).sum() > 0
            for mode, components in (("a1b", ("a1", "b")), ("a1d", ("a1", "d")),
                                     ("a1bd", ("a1", "b", "d"))):
                combined, _ = run(renderer, source, root / f"{rate}-{mode}.wav", config, mode=mode+"-residual")
                expected = reference.copy()
                empty = root / "empty.json"
                empty.write_text("{}", encoding="utf-8")
                for component in components[1:]:
                    part, _ = run(renderer, source, root / f"{rate}-{mode}-{component}.wav",
                                  empty, mode=component+"-residual")
                    expected += part
                assert np.max(np.abs(combined-expected)) < 1e-7
            for block in (1, 7, 32, 64, 256, 257, 512, 1024):
                actual, _ = run(renderer, source, root / f"{rate}-{block}.wav", config, block)
                assert np.array_equal(reference, actual)
            sf.write(source, np.column_stack((-.3 * left, left)), rate, subtype="FLOAT")
            swapped, _ = run(renderer, source, root / f"{rate}-swap.wav", config)
            assert np.array_equal(reference[:, ::-1], swapped)
            for stereo in ("left", "mono", "anti"):
                right = np.zeros_like(left) if stereo == "left" else left if stereo == "mono" else -left
                sf.write(source, np.column_stack((left, right)), rate, subtype="FLOAT")
                actual, _ = run(renderer, source, root / f"{rate}-{stereo}.wav", config)
                expected = np.zeros(len(actual)) if stereo == "left" else actual[:, 0] * (1 if stereo == "mono" else -1)
                assert np.array_equal(actual[:, 1], expected)
            # File-level asymmetric/phase/decorrelated/transient coverage, in addition to
            # the C++ independent shared-frame oracle and per-request trajectory checks.
            rng = np.random.default_rng(719)
            for label, pair in (
                ("right", np.column_stack((np.zeros_like(left), left))),
                ("quadrature", np.column_stack((left, .7*np.cos(2*np.pi*197*time)))),
                ("decorrelated", np.column_stack((left, rng.uniform(-.5,.5,len(left))))),
                ("transients", np.column_stack((np.where(np.arange(len(left))%113==0,.9,.1*left),
                                                  np.where(np.arange(len(left))%127==0,-.6,-.2*left))))):
                sf.write(source, pair, rate, subtype="FLOAT")
                actual, trace = run(renderer, source, root / f"{rate}-{label}.wav", config)
                sf.write(source, pair[:, ::-1], rate, subtype="FLOAT")
                reverse, reverse_trace = run(renderer, source, root / f"{rate}-{label}-swap.wav", config)
                assert np.array_equal(actual[:, ::-1], reverse)
                if label == "right":
                    assert not np.any(actual[:,0]) and np.any(actual[:,1])
                def counters(text):
                    return {k:v for k,v in (t.split("=",1) for t in text.split() if "=" in t)
                            if k in ("requested","started","radius_hist","lifetime_hist","rising")}
                assert counters(trace) == counters(reverse_trace)
            config.write_text('{"bubbleA1":{"version":2,"motionFactor":0}}', encoding="utf-8")
            silent, stats = run(renderer, source, root / f"{rate}-zero.wav", config)
            assert not np.any(silent) and "requested=0 " in stats
            # Zero current frames coexist with an active 2 ms window; these starts
            # are source-linked, not spontaneous events. Keep the distinction executable.
            pulses = np.zeros((len(left), 2))
            pulses[100::16, 0] = .9
            sf.write(source, pulses, rate, subtype="FLOAT")
            config.write_text('{"bubbleA1":{"version":2,"maxEventRateHz":10000}}', encoding="utf-8")
            _, stats = run(renderer, source, root / f"{rate}-window.wav", config)
            fields = dict(token.split("=", 1) for token in stats.split() if "=" in token)
            assert int(fields["a1_start_on_zero_current_frame"]) > 0
            assert int(fields["a1_source_window_active"]) > 0
            assert int(fields["a1_requested_frame"]) >= 100
        invalid = [
            '{"bubbleA1":{}}',
            '{"bubbleA1":{"version":1}}',
            '{"bubbleA1":{"version":2,"riseFactor":0.1}}',
            '{"bubbleA1":{"version":2,"riseModel":0.5}}',
            '{"bubbleA1":{"version":2,"riseModel":2}}',
            '{"bubbleA1":{"version":2,"radiusMinMm":2,"radiusMaxMm":2}}',
            '{"bubbleA1":{"version":2,"voiceCapacity":65}}',
            '{"bubbleA1":{"version":2,"voiceCapacity":64.5}}',
            '{"bubbleA1":{"version":2,"maxEventRateHz":10001}}',
            '{"bubbleA1":{"version":2,"sourceEnergyAmplitude":2}}',
            '{"bubbleA1":{"version":2,"radiusMinMm":NaN}}',
            '{"bubbleA1":{"version":2,"radiusMinMm":1,"radiusMinMm":2}}',
            '{"bubbleA1":{"version":2,"fastAttackMs":0}}',
            '{"bubbleA1":{"version":2,"unknown":1}}',
            '{"bubbleA1":{"version":2,"motionFactor":true}}',
            '{"bubbleA1":{"version":2}} trailing',
            '{"bubbleA1":{"version":2},"protect":{"depth":1}}',
        ]
        # Descriptor bounds are executable: parser/DSP must reject each field outside them.
        for parameter in descriptor["parameters"]:
            for value in (parameter["minimum"] - 1, parameter["maximum"] + 1):
                invalid.append(json.dumps({"bubbleA1": {"version": 2, parameter["name"]: value}}))
        for i, text in enumerate(invalid):
            config.write_text(text, encoding="utf-8")
            run(renderer, source, root / f"bad-{i}.wav", config, ok=False)
        config.write_text('{"bubbleA1":{"version":2}}', encoding="utf-8")
        run(renderer, source, root / "implicit-upgrade.wav", config, mode="a-residual", ok=False)
    print("A1 renderer partition/stereo/silence/strict-config PASS")


if __name__ == "__main__":
    main()
