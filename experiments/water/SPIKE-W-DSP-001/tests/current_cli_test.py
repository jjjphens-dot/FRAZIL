"""Small CURRENT renderer contracts using standard-library PCM/float WAV decoding.

One 48 kHz fixture checks real schema, determinism and odd-block rendering; one
96 kHz fixture checks rate support. Historical mode and Cartesian sweeps stay archived.
"""
import argparse
import json
import math
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools"))
from current_modules import load_modules


def read_float_wave(path):
    data = path.read_bytes()
    assert data[:4] == b"RIFF" and data[8:12] == b"WAVE", "expected RIFF WAV"
    offset, fmt, audio = 12, None, None
    while offset + 8 <= len(data):
        name, size = struct.unpack_from("<4sI", data, offset)
        chunk = data[offset + 8:offset + 8 + size]
        assert len(chunk) == size, "truncated WAV"
        if name == b"fmt ":
            fmt = struct.unpack_from("<HHIIHH", chunk)
            code = fmt[0] if fmt[0] != 0xfffe else struct.unpack_from("<H", chunk, 24)[0]
            assert code == 3 and fmt[5] == 32, "expected uncompressed float32"
        elif name == b"data":
            audio = chunk
        offset += 8 + size + size % 2
    assert fmt and audio and len(audio) % (4 * fmt[1]) == 0
    samples = tuple(value[0] for value in struct.iter_unpack("<f", audio))
    assert all(math.isfinite(x) for x in samples), "nonfinite renderer output"
    return fmt[1], fmt[2], samples


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    modules = load_modules()
    parser.add_argument("--module", choices=modules, required=True)
    parser.add_argument("--renderer", type=Path, required=True)
    args = parser.parse_args()
    module = modules[args.module]
    renderer = args.renderer.resolve()
    def invoke(*arguments, expected=0):
        result = subprocess.run([str(renderer), *map(str, arguments)], capture_output=True,
                                text=True, timeout=20)
        assert result.returncode == expected, (result.returncode, result.stdout, result.stderr)
        return result.stdout

    descriptor = json.loads(invoke(module["descriptor"]))
    contract = ROOT / "experiments/water/contracts" / module["contract"]
    assert descriptor == json.loads(contract.read_text(encoding="utf-8-sig"))
    parent = ROOT / "build/current-cli"
    parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=args.module + "-", dir=parent) as folder:
        folder = Path(folder)
        sequence = 0
        source, config = folder / "input.wav", folder / "config.json"
        def render(values, block=128, expected=0):
            nonlocal sequence
            sequence += 1
            output = folder / f"{sequence}.wav"
            config.write_text(json.dumps({module["config_key"]: values}), encoding="utf-8")
            invoke(source, output, module["mode"], block, 42, config, 0, expected=expected)
            if expected:
                assert not output.exists(), "rejected config must not publish audio"
                return None
            return read_float_wave(output)

        for rate in (48000, 96000):
            frames = int(rate * .08)
            samples = [int(14000 * math.sin(i * .17)) if i % 960 < 128 else 0 for i in range(frames)]
            with wave.open(str(source), "wb") as stream:
                stream.setparams((2, 2, rate, frames, "NONE", "not compressed"))
                stream.writeframes(b"".join(struct.pack("<hh", x, -x // 2) for x in samples))
            baseline = render({"version": module["version"]})
            assert baseline[:2] == (2, rate) and len(baseline[2]) == frames * 2
            assert any(baseline[2]), "non-vacuous current render"
            if rate == 48000:
                assert render({"version": module["version"]}, 257) == baseline, "partition identity"
                assert render({"version": module["version"]}) == baseline, "fixed-seed repeat"
        writable = [p for p in descriptor["parameters"] if p.get("writable")]
        first = writable[0]
        for parameter in writable:
            for invalid in (parameter["minimum"] - 1, parameter["maximum"] + 1):
                render({"version": module["version"], parameter["name"]: invalid}, expected=2)
        for values in ({"version": -1}, {"version": module["version"], "unknown": 1},
                       {"version": module["version"], first["name"]: True}):
            render(values, expected=2)
    print(f"CURRENT {args.module}: schema/rate/finite/stereo/repeat/partition PASS")


if __name__ == "__main__":
    main()
