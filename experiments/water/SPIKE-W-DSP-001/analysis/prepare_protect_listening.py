"""Prepare randomized RMS-matched comparisons using the existing Protect renderer.

Engineering preparation only: RMS matching is not equal perceived loudness or a listening result.
User supplies material/provenance; generated audio, keys and blank scorecards remain local.
"""
import argparse
import csv
import json
from pathlib import Path
import random
import subprocess

import numpy as np
import soundfile as sf

DIMENSIONS = ("attack_clarity", "source_recognizability", "water_identity", "continuity",
              "droplet_identity", "tail_preservation", "pumping", "post_attack_holes",
              "stereo_stability", "variable_depth_usefulness", "other_artifacts")
ROOT = Path(__file__).resolve().parents[4]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--source-note", required=True, help="Source/author/license or local-use permission")
    parser.add_argument("--mode", choices=("abd", "c"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--diagnostic", action="store_true", help="Mark a technical dry run, not musical material")
    args = parser.parse_args()
    dry, rate = sf.read(args.source, always_2d=True)
    if (rate not in (44100, 48000, 96000) or dry.shape[1] not in (1, 2) or len(dry) == 0
            or not np.isfinite(dry).all() or np.max(np.abs(dry)) > 1):
        raise ValueError("Need finite full-scale mono/stereo input at 44.1/48/96 kHz")
    args.output.mkdir(parents=True, exist_ok=False)
    dry = np.pad(dry, ((0, 3*rate), (0, 0)))
    cases = {"dry": dry}
    configs = {}
    for name, depth, topology in [("off", 0, 1), ("mild", .25, 1), ("medium", .5, 1), ("strong", 1, 1)] + (
            [("droplet_exempt_medium", .5, 2), ("droplet_exempt_strong", 1, 2),
             ("droplet_half_medium", .5, 3), ("droplet_half_strong", 1, 3)] if args.mode == "abd" else []):
        config = {"protect": {"depth": depth, "topology": topology}}
        config_path = args.output / (name + ".json")
        config_path.write_text(json.dumps(config, indent=2), encoding="utf-8")
        path = args.output / (name + "_raw.wav")
        subprocess.run([str(args.renderer.resolve()), str(args.source.resolve()), str(path.resolve()),
                        args.mode, "128", "42", str(config_path.resolve()), "3"], check=True,
                       capture_output=True, text=True)
        audio, actual_rate = sf.read(path, always_2d=True)
        assert actual_rate == rate and audio.shape == dry.shape and np.isfinite(audio).all()
        cases[name], configs[name] = audio, config
    cases["lower_residual_control"] = dry + .5*(cases["off"]-dry)
    # A fixed residual-depth comparison, not production routing/Amount mapping.
    stats = {name: (float(np.sqrt(np.mean(x*x))), float(np.max(np.abs(x)))) for name, x in cases.items()}
    if any(rms <= 1e-12 for rms, _ in stats.values()):
        raise ValueError("Silent/near-silent comparison cannot be RMS matched")
    target = min(.1, *(0.9*rms/peak for rms, peak in stats.values()))
    trials = list(cases) + ["off", "medium"] # Hidden repeats to support consistency observations.
    random.Random(42).shuffle(trials)
    keys = []
    for i, name in enumerate(trials):
        label = f"trial_{i+1:02d}"
        gain = target/stats[name][0]
        path = args.output / (label+".wav")
        sf.write(path, cases[name]*gain, rate, subtype="FLOAT")
        decoded, _ = sf.read(path, always_2d=True)
        assert abs(float(np.sqrt(np.mean(decoded*decoded)))-target) <= 1e-7
        assert np.max(np.abs(decoded)) <= .900001
        keys.append({"trial":label, "condition":name, "normalization_gain":gain,
                     "raw_rms":stats[name][0], "raw_peak":stats[name][1], "config":configs.get(name)})
    for name in ("dry", "lower_residual_control"):
        sf.write(args.output/(name+"_raw.wav"), cases[name], rate, subtype="FLOAT")
    manifest = {"status":"NOT LISTENED / human conclusions pending", "diagnostic_only":args.diagnostic,
                "source_commit":subprocess.check_output(["git","rev-parse","HEAD"],cwd=ROOT,text=True).strip(),
                "source_dirty":bool(subprocess.check_output(["git","status","--porcelain"],cwd=ROOT)),
                "source_name":args.source.name, "source_note":args.source_note, "mode":args.mode,
                "rate":rate, "frames":len(dry), "seed":42, "block":128,
                "matching":"full common-window RMS, including 3 s tail; not LUFS/perceptual matching",
                "target_rms":target, "keys":keys}
    (args.output/"REVIEWER_KEY.json").write_text(json.dumps(manifest, indent=2, allow_nan=False), encoding="utf-8")
    with (args.output/"SCORES.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["trial", *DIMENSIONS, "observations", "decision"])
        for key in keys:
            writer.writerow([key["trial"], *([""]*(len(DIMENSIONS)+2))])
    (args.output/"LISTENING_REVIEW.md").write_text(
        "# Protect listening review — NOT RUN\n\n"
        "Diagnostic dry run: " + str(args.diagnostic) + ". No musical acceptance is implied.\n\n"
        "Reviewer / exact source revision / material / monitoring level / environment: PENDING.\n"
        "Define 1/3/5 anchors for each score dimension before listening. Use N/A for unsupported dimensions.\n"
        "Listen to trial WAVs before opening REVIEWER_KEY.json; it contains normalization gains and repeats.\n"
        "Raw WAVs remain available for level/artifact checks. RMS matching does not guarantee equal loudness.\n"
        "Record per-dimension observations and uncertainty; do not compute one quality winner.\n"
        "Compare temporary yielding with lower_residual_control, then decide whether variable depth is useful.\n"
        "Decision: PENDING (ACCEPT / REVISE / REJECT). Product: PENDING (Reject / Internal safeguard / User macro).\n",
        encoding="utf-8")
    print(f"PASS: {len(keys)} trials; RMS/peak/readback verified; human listening NOT RUN")


if __name__ == "__main__":
    main()
