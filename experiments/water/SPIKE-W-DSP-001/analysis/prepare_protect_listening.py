"""Independent fixed-source and RMS-matched D0/D1 blinded listening packs.

Human listening and detector selection remain pending. Sources are distinguished by
name, description and audio metadata; no source or artifact checksums are computed.
"""
import argparse
import csv
import io
import json
from pathlib import Path
import random
import subprocess

import numpy as np
import soundfile as sf

DIMENSIONS = ("attack_clarity", "source_recognizability", "water_identity", "continuity",
              "droplet_identity", "tail_preservation", "pumping", "post_attack_holes",
              "stereo_stability", "variable_depth_usefulness", "other_artifacts")
RIGHTS_FIELDS = ("source", "author", "license", "permission", "storage_policy")
BIT_DEPTHS = {"PCM_U8": 8, "PCM_16": 16, "PCM_24": 24, "PCM_32": 32,
              "FLOAT": 32, "DOUBLE": 64}
ROOT = Path(__file__).resolve().parents[4]
BLOCK_SIZE = 128


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def read_source(path, metadata_path):
    """Identify/decode one snapshot so later source edits cannot change the rendered input."""
    rights = json.loads(metadata_path.read_text(encoding="utf-8"))
    if not isinstance(rights, dict) or any(
            not isinstance(rights.get(key), str) or not rights[key].strip() for key in RIGHTS_FIELDS):
        raise ValueError("Source metadata requires nonempty source/author/license/permission/storage_policy")
    source_bytes = path.read_bytes()
    info = sf.info(io.BytesIO(source_bytes))
    dry, rate = sf.read(io.BytesIO(source_bytes), always_2d=True)
    if (info.format not in ("WAV", "WAVEX") or info.subtype not in BIT_DEPTHS
            or rate not in (44100, 48000, 96000) or dry.shape[1] not in (1, 2)
            or len(dry) == 0 or not np.isfinite(dry).all() or np.max(np.abs(dry)) > 1):
        raise ValueError("Need finite full-scale PCM/float WAV, mono/stereo, at 44.1/48/96 kHz")
    provenance = {key: rights[key] for key in RIGHTS_FIELDS}
    provenance.update(source_name=path.name, source_snapshot="reviewer/source.wav",
                      source_frames=info.frames, source_duration_seconds=info.frames / rate,
                      sample_rate=rate, channels=info.channels, format=info.format,
                      subtype=info.subtype, bit_depth=BIT_DEPTHS[info.subtype])
    return source_bytes, dry, provenance


def protect_config(detector, depth, topology):
    """Pin thresholds and every Protect field; no D1-by-omission in listening conditions."""
    if detector not in (0, 1):
        raise ValueError("Detector must be explicitly D0 or D1")
    low, high = (.01, .12) if detector == 0 else (1., 9.)
    return {"protect": {"detector": detector, "depth": depth, "topology": topology,
                        "floor": 1e-4, "epsilon": 1e-8, "thresholdLow": low,
                        "thresholdHigh": high, "capDb": 9., "depthExponent": 1.,
                        "scoreExponent": 1., "attackSeconds": .001,
                        "releaseSeconds": .08, "offSeconds": .01}}


def render_cases(args, source_path, dry, reviewer):
    cases = {"dry": dry}
    conditions = {"dry": {"detector": "N/A", "config": None, "residual_scale": 0.}}
    settings = [("off", 0., 1), ("mild", .25, 1), ("medium", .5, 1), ("strong", 1., 1)]
    if args.mode == "abd":
        settings += [("droplet_exempt_medium", .5, 2), ("droplet_exempt_strong", 1., 2),
                     ("droplet_half_medium", .5, 3), ("droplet_half_strong", 1., 3)]
    for detector in (0, 1):
        for suffix, depth, topology in settings:
            name = f"d{detector}_{suffix}"
            config = protect_config(detector, depth, topology)
            config_path = reviewer / (name + ".json")
            write_json(config_path, config)
            path = reviewer / (name + "_raw.wav")
            subprocess.run([str(args.renderer.resolve()), str(source_path.resolve()), str(path.resolve()),
                            args.mode, str(BLOCK_SIZE), str(args.dsp_seed), str(config_path.resolve()),
                            str(args.appended_tail_seconds)], check=True, capture_output=True, text=True)
            audio, rate = sf.read(path, always_2d=True)
            if rate != args.sample_rate or audio.shape != dry.shape or not np.isfinite(audio).all():
                raise ValueError(f"Invalid renderer output for {name}")
            cases[name] = audio
            conditions[name] = {"detector": f"D{detector}", "config": config, "residual_scale": 1.}
    if not np.array_equal(cases["d0_off"], cases["d1_off"]):
        raise ValueError("D0/D1 OFF must share the exact baseline")
    # Offline residual-depth control, not a production Amount/routing mapping.
    cases["lower_residual_control"] = dry + .5 * (cases["d0_off"] - dry)
    conditions["lower_residual_control"] = {
        "detector": "N/A (OFF-derived)", "config": None, "residual_scale": .5,
        "derived_from": "d0_off", "formula": "source + 0.5 * (d0_off - source)"}
    for name in ("dry", "lower_residual_control"):
        sf.write(reviewer / (name + "_raw.wav"), cases[name], args.sample_rate, subtype="FLOAT")
    return cases, conditions


def write_pack(output, pack_type, cases, conditions, provenance, randomization_seed):
    """Separate evidence purposes, scores and decisions, even for the same raw renders."""
    output.mkdir()
    stats = {name: (float(np.sqrt(np.mean(x*x))), float(np.max(np.abs(x))))
             for name, x in cases.items()}
    if any(rms <= 1e-12 for rms, _ in stats.values()):
        raise ValueError("Silent/near-silent comparison cannot be RMS matched")
    fixed = pack_type == "fixed_source"
    # One attenuator across the primary pack preserves the same source coefficient in g*(x+E).
    common_gain = min(1., .9 / max(peak for _, peak in stats.values()))
    target_rms = min(.1, *(.9 * rms / peak for rms, peak in stats.values()))
    trials = list(cases) + ["d0_off", "d0_medium", "d1_medium"]
    random.Random(randomization_seed).shuffle(trials)
    keys = []
    for index, name in enumerate(trials, 1):
        label = f"trial_{index:02d}"
        gain = common_gain if fixed else target_rms / stats[name][0]
        expected = cases[name] * gain
        sf.write(output / (label + ".wav"), expected, provenance["sample_rate"], subtype="FLOAT")
        decoded, rate = sf.read(output / (label + ".wav"), always_2d=True)
        if (rate != provenance["sample_rate"] or decoded.shape != expected.shape
                or not np.allclose(decoded, expected, atol=3e-8, rtol=0)
                or np.max(np.abs(decoded)) > .900001):
            raise ValueError(f"Playback readback failed: {label}")
        if not fixed and abs(float(np.sqrt(np.mean(decoded*decoded))) - target_rms) > 1e-7:
            raise ValueError(f"RMS readback failed: {label}")
        keys.append({"trial": label, "condition": name, **conditions[name],
                     "dsp_seed": provenance["dsp_seed"], "playback_gain": gain,
                     "source_carrier_gain": gain, "raw_rms": stats[name][0], "raw_peak": stats[name][1]})
    purpose = ("PRIMARY: attack/source-preservation and bounded D0/D1 comparison" if fixed else
               "PREFERENCE-SUPPORTING ONLY: RMS matched; cannot establish attack/source preservation")
    manifest = {**provenance, "pack_type": pack_type, "evidence_purpose": purpose,
                "randomization_seed": randomization_seed,
                "matching": ("one common source/carrier gain; no per-condition normalization" if fixed else
                             "full comparison-window RMS including appended tail; not LUFS/perceptual matching"),
                "fixed_source_gain": common_gain if fixed else None,
                "target_rms": None if fixed else target_rms, "keys": keys}
    write_json(output / "REVIEWER_KEY.json", manifest)
    with (output / "SCORES.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["trial", *DIMENSIONS, "observations", "decision"])
        for key in keys:
            writer.writerow([key["trial"], *([""] * (len(DIMENSIONS) + 2))])
    (output / "LISTENING_REVIEW.md").write_text(
        f"# {pack_type} listening review — NOT RUN\n\n{purpose}\n\n"
        f"Diagnostic dry run: {provenance['diagnostic_only']}. No musical acceptance implied.\n\n"
        "Reviewer / material / monitoring level / environment / 1-3-5 anchors: PENDING.\n"
        "Listen only to trial WAVs before opening REVIEWER_KEY.json or the sibling reviewer directory.\n"
        "The key reveals conditions/repeats; keep it with the coordinator until scoring ends.\n"
        "Record independent dimensions, repeat consistency and uncertainty; no total winner.\n"
        "Use N/A for unsupported dimensions. Compare lower-residual control before claiming depth utility.\n"
        "This pack has its own scores and conclusion. Do not copy or pool conclusions between packs.\n"
        "RMS-matched observations support preference only, even if attack dimensions are recorded.\n"
        "Pack conclusion (ACCEPT / REVISE / REJECT with evidence and uncertainty): PENDING.\n"
        "Detector selection: UNRESOLVED. Wave 7 product decision: BLOCKED.\n", encoding="utf-8")
    return len(keys)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--source-metadata", type=Path, required=True,
                        help="JSON: source, author, license, permission, storage_policy (local copies included)")
    parser.add_argument("--mode", choices=("abd", "c"), required=True)
    parser.add_argument("--output", type=Path, required=True, help="New ignored local directory")
    parser.add_argument("--diagnostic", action="store_true")
    parser.add_argument("--dsp-seed", type=int, default=42)
    parser.add_argument("--randomization-seed", type=int, default=42,
                        help="Fixed-source seed; RMS pack uses this seed + 1")
    parser.add_argument("--appended-tail-seconds", type=int, choices=range(31), default=3)
    args = parser.parse_args()
    if not 0 <= args.dsp_seed <= 0xffffffff:
        parser.error("dsp-seed must fit an unsigned 32-bit integer")
    source_bytes, dry, provenance = read_source(args.source, args.source_metadata)
    args.sample_rate = provenance["sample_rate"]
    dry = np.pad(dry, ((0, args.appended_tail_seconds * args.sample_rate), (0, 0)))
    provenance.update(schema_version=2, mode=args.mode, diagnostic_only=args.diagnostic,
                      status="NOT LISTENED / human conclusions pending", detector_selection="UNRESOLVED",
                      wave7_product_decision="BLOCKED: detector selection and human listening required",
                      code_commit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                      code_dirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                      comparison_frames=len(dry), comparison_duration_seconds=len(dry) / args.sample_rate,
                      appended_tail_seconds=args.appended_tail_seconds, dsp_seed=args.dsp_seed,
                      block_size=BLOCK_SIZE, output_subtype="FLOAT")
    args.output.mkdir(parents=True, exist_ok=False)
    reviewer = args.output / "reviewer"
    reviewer.mkdir()
    # Frozen original bytes remain local and subject to the declared source storage permission.
    source_path = reviewer / "source.wav"
    source_path.write_bytes(source_bytes)
    write_json(reviewer / "SOURCE_PROVENANCE.json", provenance)
    cases, conditions = render_cases(args, source_path, dry, reviewer)
    for offset, pack_type in enumerate(("fixed_source", "rms_matched")):
        count = write_pack(args.output / pack_type, pack_type, cases, conditions, provenance,
                           args.randomization_seed + offset)
        print(f"PASS: {pack_type}: {count} trials; readback verified; human listening NOT RUN")
    (args.output / "DETECTOR_SELECTION.md").write_text(
        "# Bounded D0/D1 selection — UNRESOLVED\n\n"
        "Wave 7 product decision: BLOCKED until detector selection and human listening evidence exist.\n"
        "Both detectors use explicit thresholds (D0 .01/.12 amplitude; D1 1/9 dB), cap 9 dB and the same\n"
        "depth/topology/seed/source. These are bounded configurations, not equally tuned detector families.\n"
        "Complete fixed_source blind scores before unblinding. Record repeat consistency, source/attack\n"
        "preservation, sustained suppression, quiet-after-loud response, recovery holes and identity tradeoffs.\n"
        "Preference evidence stays in rms_matched; never pool its conclusions with fixed_source.\n"
        "Record reviewer, source name/description, code revision, exact trial/config references, chosen detector or\n"
        "neither, rejected alternatives, uncertainty and rationale. If neither works, retain the block and\n"
        "open bounded threshold/follower follow-up. No D1 default is a selection decision.\n\n"
        "Selection evidence / reviewer / decision: PENDING. Product adoption still requires Joint Gate/ADR.\n",
        encoding="utf-8")


if __name__ == "__main__":
    main()
