"""Real renderer, strict B1 schema/descriptor and decoded audio invariants."""
import subprocess
import numpy as np
import soundfile as sf


def render(exe, source, output, config, mode="b1-residual", block=128, ok=True):
    p = subprocess.run([str(exe), str(source), str(output), mode, str(block), "42", str(config), "2"], capture_output=True, text=True)
    if not ok:
        assert p.returncode == 2 and not output.exists(), (p.returncode, p.stdout, p.stderr)
        return None
    assert p.returncode == 0, (p.returncode, p.stdout, p.stderr)
    audio, _ = sf.read(output, always_2d=True)
    assert np.isfinite(audio).all()
    stats = dict(t.split("=", 1) for t in p.stdout.split() if "=" in t)
    return audio, stats
