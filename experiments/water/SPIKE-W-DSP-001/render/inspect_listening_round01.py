"""Verify completed local packs and derive histograms; never score human quality."""
import argparse
import csv
import json
import subprocess
from pathlib import Path

import numpy as np
import soundfile as sf


def inspect(root, playback_gain_db):
    context = json.loads((root / "CONTEXT.json").read_text(encoding="utf-8"))
    peaks = {"fixed": [0, ""], "preference": [0, ""]}
    over = {"fixed": 0, "preference": 0}
    files = list(root.rglob("*.wav"))
    if not files:
        raise ValueError("Empty pack")
    for path in files:
        audio, rate = sf.read(path, always_2d=True)
        if not len(audio) or audio.shape[1] != 2 or not np.isfinite(audio).all():
            raise ValueError(f"Invalid listening WAV: {path.name}")
        if rate not in (44100, 48000, 96000):
            raise ValueError("Unexpected source/DSP rate")
        peak = float(np.max(np.abs(audio)))
        group = "preference" if any("rms-preference" in p for p in path.parts) else "fixed"
        over[group] += int(peak > 1)
        if peak > peaks[group][0]:
            peaks[group] = [peak, path.relative_to(root).as_posix()]
    post_gain_peak = max(v[0] for v in peaks.values()) * 10**(playback_gain_db / 20)
    if post_gain_peak > 1:
        raise ValueError("Declared common playback attenuation is insufficient")
    result = dict(section=context["section"], wav_count=len(files), finite=True,
                  peak_by_group=peaks, above_full_scale=over, playback_gain_db=playback_gain_db,
                  maximum_post_playback_peak=post_gain_peak,
                  inspector_revision=subprocess.check_output(
                      ["git", "rev-parse", "HEAD"], cwd=Path(__file__).resolve().parents[4],
                      text=True).strip(), human_acceptance="NOT ASSESSED")
    (root / "INSPECTION.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    note = (f"\nVerified playback headroom: route ALL labels through a common {playback_gain_db:g} dB "
            "playback bus before device conversion. Keep fixed-scale and RMS-preference groups separate. "
            "Float files retain original peaks; no per-file normalization or limiter is added. "
            "See INSPECTION.json.\n")
    readme = root / "READ_ME.txt"
    text = readme.read_text(encoding="utf-8").split("\nVerified playback headroom:")[0]
    readme.write_text(text + note, encoding="utf-8")
    if context["section"] == "a1":
        rows = []
        for path in sorted(root.glob("source-*-bins*.csv")):
            events = np.atleast_1d(np.genfromtxt(path, delimiter=",", names=True))
            for bin_number in np.unique(events["bin"]):
                selected = events[events["bin"] == bin_number]
                rows.append(dict(case=path.stem, bin=int(bin_number),
                                 radius_mm=float(selected["radius_mm"][0]),
                                 frequency_hz=float(selected["frequency_hz"][0]), requests=len(selected)))
        if not rows:
            raise ValueError("Missing A1 event histograms")
        with (root / "FREQUENCY_HISTOGRAM.csv").open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
    print(json.dumps(result))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packs", type=Path, nargs="+")
    parser.add_argument("--playback-gain-db", type=float, default=-18)
    args = parser.parse_args()
    if not np.isfinite(args.playback_gain_db) or not -60 <= args.playback_gain_db <= 0:
        parser.error("Playback gain must be finite in [-60,0] dB")
    for pack in args.packs:
        inspect(pack, args.playback_gain_db)
