"""Decoded PCM assertions for the research baseline; no hashes or listening claims."""

import pathlib
import math
import subprocess
import sys
import wave


from cli_support import workspace, pcm_source, read_float, check_frames


from cli_support import run_main

def main():
    renderer = pathlib.Path(sys.argv[1]).resolve()
    with workspace() as root:
        source = root / "input.wav"
        pcm_source(source)
        for mode in ("baseline", "a-residual"):
            output = root / (mode + ".wav")
            command = [str(renderer), str(source), str(output), mode, "128", "42"]
            subprocess.run(command, check=True, capture_output=True)
            if mode == "baseline":
                with wave.open(str(output), "rb") as reader:
                    assert reader.getnframes() == 1031
                    payload = reader.readframes(1031)
                assert subprocess.run(command, capture_output=True).returncode == 2
                check_frames(output, payload)
            else:
                values = read_float(output)
                assert len(values) == 1031 and all(math.isfinite(v) for v in values)
        assert subprocess.run([str(renderer), str(source), str(root / "bad.wav"),
                               "invalid", "128", "42"], capture_output=True).returncode == 2


if __name__ == "__main__":
    run_main(main)
