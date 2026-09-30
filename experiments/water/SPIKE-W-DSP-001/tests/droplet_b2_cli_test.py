"""Independent schema, actual WAV routing and new-mode regression tests."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

import numpy as np
import soundfile as sf

renderer = Path(sys.argv[1]).resolve()
contracts = Path(__file__).resolve().parents[2] / "contracts"
for argument, snapshot in (("--describe-droplet-b2", "droplet-b2-v1.json"),
                           ("--describe-bubble-a1-v3", "bubble-a1-v3.json")):
    actual = json.loads(subprocess.check_output([str(renderer), argument], text=True))
    assert actual == json.loads((contracts / snapshot).read_text(encoding="utf-8-sig"))

with tempfile.TemporaryDirectory(prefix="b2-cli-", dir=Path.cwd()) as temporary:
    root = Path(temporary)
    def render(source, name, mode, config, accepted=True):
        cfg, output = root / (name + ".json"), root / (name + ".wav")
        cfg.write_text(json.dumps(config), encoding="utf-8")
        result = subprocess.run([str(renderer), str(source), str(output), mode, "127", "42", str(cfg), "2"],
                                capture_output=True, text=True)
        assert (result.returncode == 0) == accepted, (mode, config, result.stdout, result.stderr)
        if accepted:
            audio, rate = sf.read(output, always_2d=True)
            assert np.isfinite(audio).all()
            return audio, rate

    for rate in (44100, 48000, 96000):
        mono = np.zeros(int(rate * .08), dtype=np.float32)
        mono[:int(rate * .002)] = .5
        source = root / f"mono-{rate}.wav"
        sf.write(source, mono, rate, subtype="FLOAT")
        for mode in ("b2", "b2-residual", "a1b2", "b2d1", "a1b2d1"):
            audio, actual_rate = render(source, f"{mode}-{rate}", mode, {"dropletB2": {"version": 1}})
            assert audio.shape[1] == 2 and actual_rate == rate
            assert np.any(audio[:, 0] != audio[:, 1]), "mono source must retain B2 channel difference"
    source = root / "mono-48000.wav"
    for index, config in enumerate(({"dropletB2": {"version": 2}},
                                    {"dropletB2": {"version": 1, "unknown": 1}},
                                    {"dropletB2": {"version": 1, "eventRadiusSpreadPct": 6}},
                                    {"dropletB1": {"version": 1}})):
        render(source, f"reject-{index}", "b2", config, False)
    render(source, "reject-legacy", "b", {"dropletB2": {"version": 1}}, False)
    render(source, "v2-reject-gamma", "a1", {"bubbleA1": {"version": 2, "depthAmplitudeGamma": 1}}, False)
    a, _ = render(source, "a1-v2", "a1", {"bubbleA1": {"version": 2}})
    b, _ = render(source, "a1-v3", "a1", {"bubbleA1": {"version": 3, "depthAmplitudeGamma": 1}})
    assert np.array_equal(a, b), "A1 gamma=1 exact versioned baseline"
print("B2 CLI: schemas, modes, mono spatial output and A1 identity PASS")
