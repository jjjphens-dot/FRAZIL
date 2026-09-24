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
            config.write_text('{"bubbleA1": {}}', encoding="utf-8")
            reference, stats = run(renderer, source, root / f"{rate}-base.wav", config)
            assert "bubble_model=A1" in stats and "radius_hist=" in stats
            fields = dict(token.split("=", 1) for token in stats.split() if "=" in token)
            assert int(fields["bubble_events"]) == int(fields["started"]) > 0
            assert int(fields["bubble_first_frame"]) >= 0
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
            config.write_text('{"bubbleA1":{"motionFactor":0}}', encoding="utf-8")
            silent, stats = run(renderer, source, root / f"{rate}-zero.wav", config)
            assert not np.any(silent) and "requested=0 " in stats
        invalid = [
            '{"bubbleA1":{"radiusMinMm":2,"radiusMaxMm":2}}',
            '{"bubbleA1":{"voiceCapacity":65}}',
            '{"bubbleA1":{"voiceCapacity":64.5}}',
            '{"bubbleA1":{"maxEventRateHz":10001}}',
            '{"bubbleA1":{"sourceEnergyAmplitude":2}}',
            '{"bubbleA1":{"radiusMinMm":NaN}}',
            '{"bubbleA1":{"radiusMinMm":1,"radiusMinMm":2}}',
            '{"bubbleA1":{"fastAttackMs":0}}',
            '{"bubbleA1":{"unknown":1}}',
            '{"bubbleA1":{"motionFactor":true}}',
            '{"bubbleA1":{}} trailing',
            '{"bubbleA1":{},"protect":{"depth":1}}',
        ]
        for i, text in enumerate(invalid):
            config.write_text(text, encoding="utf-8")
            run(renderer, source, root / f"bad-{i}.wav", config, ok=False)
        config.write_text('{"bubbleA1":{}}', encoding="utf-8")
        run(renderer, source, root / "implicit-upgrade.wav", config, mode="a-residual", ok=False)
    print("A1 renderer partition/stereo/silence/strict-config PASS")


if __name__ == "__main__":
    main()
