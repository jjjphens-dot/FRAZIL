"""Decoded PCM assertions for the research baseline; no hashes or listening claims."""

import pathlib
import math
import json
import struct
import subprocess
import sys
import tempfile
import wave


def main():
    renderer = pathlib.Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="frazil-water-") as directory:
        root = pathlib.Path(directory)
        source = root / "input.wav"
        # Odd length and alternating stereo activity catch truncated blocks and crossfeed.
        samples = [(12000 if i % 7 == 0 else -3000, 0) for i in range(1031)]
        for rate in (44100, 48000, 96000):
            with wave.open(str(source), "wb") as writer:
                writer.setparams((2, 2, rate, 0, "NONE", "not compressed"))
                writer.writeframes(b"".join(struct.pack("<hh", *pair) for pair in samples))
            for block in (1, 7, 32, 64, 128, 256, 512, 1024):
                for mode in ("baseline", "residual"):
                    output = root / f"{rate}-{block}-{mode}.wav"
                    command = [str(renderer), str(source), str(output), mode, str(block), "42"]
                    subprocess.run(command, check=True, capture_output=True, text=True)
                    with wave.open(str(output), "rb") as reader:
                        assert reader.getnframes() == len(samples)
                        assert reader.getnchannels() == 2
                        assert reader.getframerate() == rate
                        assert reader.getsampwidth() == 3
                        raw = reader.readframes(len(samples))
                    decoded = [int.from_bytes(raw[i:i+3], "little", signed=True)
                               for i in range(0, len(raw), 3)]
                    expected = [v * 256 if mode == "baseline" else 0
                                for pair in samples for v in pair]
                    assert all(abs(a - b) <= 1 for a, b in zip(decoded, expected))
                    assert len(decoded) == len(expected)
                    # Refuse overwrites rather than risk source loss or appended WAV data.
                    assert subprocess.run(command, capture_output=True).returncode != 0
                    check_frames(output, raw)
        # Research WAVs are IEEE float: decode samples, rather than compare container bytes.
        def read_float(path):
            data = path.read_bytes()
            position = 12
            payload = None
            while position + 8 <= len(data):
                name, size = struct.unpack_from("<4sI", data, position)
                position += 8
                if name == b"data": payload = data[position:position+size]
                position += size + size % 2
            assert payload is not None and len(payload) % 4 == 0
            return struct.unpack("<" + "f" * (len(payload)//4), payload)

        for rate in (44100, 48000, 96000):
            values = [int(10000 * math.sin(i * .17)) if i % 8192 < 4096 else 0 for i in range(32771)]
            with wave.open(str(source), "wb") as writer:
                writer.setparams((2, 2, rate, 0, "NONE", "not compressed"))
                writer.writeframes(b"".join(struct.pack("<hh", value, 0) for value in values))
            renders = {}
            for mode in ("a", "b", "d", "ab", "ad", "bd", "abd", "c"):
                reference = None
                for block in (7, 128, 1024):
                    output = root / f"sonic-{rate}-{mode}-{block}.wav"
                    subprocess.run([str(renderer), str(source), str(output), mode+"-residual", str(block), "42", "-", "2"], check=True, capture_output=True)
                    actual = read_float(output)
                    assert len(actual) == (len(values)+2*rate)*2
                    assert all(math.isfinite(value) for value in actual)
                    assert all(value == 0 for value in actual[1::2])
                    if reference is not None: assert actual == reference
                    reference = actual
                renders[mode] = reference
            for mode, parts in (("ab", "ab"), ("ad", "ad"), ("bd", "bd"), ("abd", "abd")):
                assert all(abs(renders[mode][i] - sum(renders[part][i] for part in parts)) < 1e-7 for i in range(len(renders[mode])))
            assert any(abs(value) > 1e-8 for value in renders["a"])
            assert any(abs(value) > 1e-8 for value in renders["b"])
            config = root / "zero.json"
            config.write_text(json.dumps({name: {"residualGain": 0} for name in ("bubble", "droplet", "flow", "modal")}))
            output = root / f"zero-{rate}.wav"
            subprocess.run([str(renderer), str(source), str(output), "abd", "128", "42", str(config)], check=True, capture_output=True)
            actual = read_float(output)
            assert all(actual[2*i] == value/32768 for i,value in enumerate(values))
        with wave.open(str(source), "wb") as writer:
            writer.setparams((1, 2, 48000, 0, "NONE", "not compressed"))
            writer.writeframes(struct.pack("<"+"h"*1031, *([5000]+[0]*1030)))
        output = root / "mono.wav"
        subprocess.run([str(renderer), str(source), str(output), "c", "7", "0"], check=True, capture_output=True)
        assert len(read_float(output)) == 1031
        for bad in ({"water.size": 1}, {"bubble":{"voices":1.5}}, {"flow":{"depthSeconds":-1}}, {"modal":{"residualGain":"0"}}):
            config = root / "invalid.json"
            config.write_text(json.dumps(bad))
            assert subprocess.run([str(renderer), str(source), str(root/"bad.wav"), "abd", "128", "42", str(config)], capture_output=True).returncode != 0
        for mode, block, seed in (("fluid", "128", "42"), ("baseline", "0", "42"),
                                 ("baseline", "128", "-1")):
            result = subprocess.run([str(renderer), str(source), str(root / "invalid.wav"),
                                     mode, block, seed], capture_output=True)
            assert result.returncode != 0
        assert subprocess.run([str(renderer), str(source), str(source), "baseline", "128", "42"],
                              capture_output=True).returncode != 0
    print("Water baseline decoded PCM / partition / CLI checks: PASS")


def check_frames(path, expected_frames):
    # Verify frame payload after refusal without relying on header bytes or a checksum.
    with wave.open(str(path), "rb") as reader:
        assert reader.readframes(reader.getnframes()) == expected_frames


if __name__ == "__main__":
    main()
