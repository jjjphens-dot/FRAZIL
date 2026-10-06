"""Decoded PCM assertions for the research baseline; no hashes or listening claims."""

import pathlib
import json
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
        for index, bad in enumerate(({"depth":-1}, {"depth":2}, {"detector":.5},
                {"topology":4}, {"capDb":13}, {"epsilon":0}, {"offSeconds":0},
                {"thresholdLow":10}, {"attackSeconds":0}, {"unknown":0},
                {"depth":True}, {"depth":"0"})):
            config = root / "protect-invalid.json"
            config.write_text(json.dumps({"protect":bad}))
            output = root / f"protect-invalid-{index}.wav"
            result = subprocess.run([str(renderer), str(source), str(output), "baseline",
                                     "128", "42", str(config)], capture_output=True)
            assert result.returncode == 2 and not output.exists()
        # Trace creation also refuses collisions and preserves the source payload.
        output = root / "trace-collision.wav"
        result = subprocess.run([str(renderer), str(source), str(output), "abd", "128",
                                 "42", "-", "0", str(source)], capture_output=True)
        assert result.returncode == 2 and not output.exists()
        # Representation is globally strict; type-valid unused DSP ranges are independent.
        invalid_modules = {
            "bubble": {"voices": 0}, "droplet": {"voices": 17},
            "flow": {"depthSeconds": -1}, "modal": {"decaySeconds": -1},
        }
        for mode, active in (("a", ("bubble",)), ("b", ("droplet",)),
                             ("d", ("flow",)), ("c", ("modal",)),
                             ("baseline", ()), ("residual", ())):
            config = root / "independent.json"
            config.write_text(json.dumps({k: v for k, v in invalid_modules.items() if k not in active}))
            output = root / f"independent-{mode}.wav"
            clean = root / f"clean-{mode}.wav"
            for path, config_path in ((output, str(config)), (clean, "-")):
                subprocess.run([str(renderer), str(source), str(path), mode, "128", "42", config_path], check=True, capture_output=True)
            if active:
                assert read_float(output) == read_float(clean)
            else:
                with wave.open(str(clean), "rb") as reader:
                    check_frames(output, reader.readframes(reader.getnframes()))
        for mode, module in (("a", "bubble"), ("b", "droplet"), ("d", "flow"), ("c", "modal")):
            config = root / "active-invalid.json"
            config.write_text(json.dumps({module: invalid_modules[module]}))
            assert subprocess.run([str(renderer), str(source), str(root/"active-bad.wav"), mode, "128", "42", str(config)], capture_output=True).returncode != 0
        for bad in ({"unknown": {}}, {"flow": {"unknown": 1}},
                    {"bubble": {"voices": -1}}, {"bubble": {"voices": 1.5}},
                    {"bubble": {"voices": 2**64}}, {"bubble": {"voices": True}},
                    {"bubble": {"voices": "1"}}, {"droplet": []},
                    {"flow": {"residualGain": None}}):
            config = root / "structural-invalid.json"
            config.write_text(json.dumps(bad))
            for mode in ("baseline", "c", "a"):
                assert subprocess.run([str(renderer), str(source), str(root/"structural-bad.wav"), mode, "128", "42", str(config)], capture_output=True).returncode != 0
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


if __name__ == "__main__":
    run_main(main)
