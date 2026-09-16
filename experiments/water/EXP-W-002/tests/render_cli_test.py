"""Decoded PCM assertions for the research baseline; no hashes or listening claims."""

import pathlib
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
