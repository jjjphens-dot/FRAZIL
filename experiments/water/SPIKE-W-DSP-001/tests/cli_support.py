"""Fixture lifetime and WAV decoding shared by scoped renderer tests."""
from contextlib import contextmanager
from pathlib import Path
import faulthandler
import struct
import subprocess
import tempfile
import wave

faulthandler.enable()

@contextmanager
def workspace():
    build = Path(__file__).resolve().parents[4] / "build" / "cli-tests"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=build, prefix="case-") as directory:
        yield Path(directory)

def pcm_source(path):
    with wave.open(str(path), "wb") as writer:
        writer.setparams((1, 2, 48000, 0, "NONE", "not compressed"))
        writer.writeframes(struct.pack("<" + "h" * 1031, *([5000] + [0] * 1030)))

def read_float(path):
    data = path.read_bytes()
    position = 12
    payload = None
    while position + 8 <= len(data):
        name, size = struct.unpack_from("<4sI", data, position)
        position += 8
        if name == b"data":
            payload = data[position:position + size]
        position += size + size % 2
    assert payload is not None and len(payload) % 4 == 0
    return struct.unpack("<" + "f" * (len(payload) // 4), payload)

def check_frames(path, expected_frames):
    with wave.open(str(path), "rb") as reader:
        assert reader.readframes(reader.getnframes()) == expected_frames


def run_main(main):
    """Do not hide captured child diagnostics when a scoped script fails."""
    try:
        main()
    except subprocess.CalledProcessError as error:
        for stream in (error.stdout, error.stderr):
            if stream:
                print(stream.decode("utf-8", errors="replace") if isinstance(stream, bytes) else stream)
        raise
