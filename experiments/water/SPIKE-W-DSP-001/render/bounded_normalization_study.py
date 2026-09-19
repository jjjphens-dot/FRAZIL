"""C0/C3 grid through the actual C++ bank; analytical bounds are reported separately."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf

from listening_handoff import measures, rms


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    rows = []
    for rate in (44100, 48000, 96000):
        t = np.arange(rate * 2) / rate
        rng = np.random.default_rng(42)
        carrier = (.12 * np.sin(2*np.pi*260*t) + .08*np.sin(2*np.pi*55*t) +
                   .05*np.sin(2*np.pi*600*t) + rng.uniform(-.03, .03, len(t)))
        carrier *= (t % .4 < .2)
        source = output / f"source-{rate}.wav"
        impulse = output / f"impulse-{rate}.wav"
        audio = np.column_stack((carrier, -.5*carrier)).astype(np.float32)
        sf.write(source, audio, rate, subtype="FLOAT")
        sf.write(impulse, np.array([[1., 0.]], dtype=np.float32), rate, subtype="FLOAT")
        for root in (130, 260, 520):
            for decay in (.03, .12, .48):
                for normalization in ("c0", "c3"):
                    name = f"{normalization}-{rate}-{root}-{decay}"
                    config = output / f"{name}.json"
                    config.write_text(json.dumps({"modal": {"rootFrequencyHz": root,
                                      "decaySeconds": decay, "residualGain": .3,
                                      "motionDepth": 0}}) + "\n")
                    def render(input_file, suffix, block):
                        destination = output / f"{name}-{suffix}.wav"
                        result = subprocess.run([str(args.renderer.resolve()), str(input_file),
                            str(destination), "c-residual", str(block), "42", str(config), "3",
                            "-", "hard", "-", normalization], check=True,
                            capture_output=True, text=True)
                        data, actual_rate = sf.read(destination, dtype="float32", always_2d=True)
                        assert actual_rate == rate and np.isfinite(data).all()
                        fields = dict(item.split("=", 1) for item in result.stdout.split() if "=" in item)
                        return data, fields
                    c, fields = render(source, "source", 128)
                    repeat, _ = render(source, "source-block257", 257)
                    assert np.array_equal(c, repeat)
                    h_stereo, _ = render(impulse, "impulse", 128)
                    assert np.count_nonzero(h_stereo[:, 1]) == 0
                    h = h_stereo[:, 0].astype(np.float64)
                    n = np.arange(len(h))
                    coefficients = [float(v) for v in fields["modal_excitation"].split(",")]
                    assert len(coefficients) == 6
                    radius = np.exp(-1/(rate*decay))
                    analytical_h = sum(b * radius**n * np.sin(2*np.pi*root*ratio*n/rate)
                                       for b, ratio in zip(coefficients, (1, 1.41, 1.93, 2.57, 3.31, 4.17))) * .05
                    assert np.max(np.abs(h-analytical_h)) < 1e-7
                    bound = float(fields["modal_bound"])
                    assert np.sum(np.abs(h)) <= bound and np.max(np.abs(c)) <= bound
                    if normalization == "c3":
                        assert bound <= 3.9600001
                    energy = h*h
                    early = min(len(h), int(.02*rate))
                    rows.append({"normalization": normalization, "rate": rate, "root": root,
                        "decay": decay, "C": measures(c, rate, len(audio), rms(audio)),
                        "impulse_early_20ms_peak": float(np.max(np.abs(h[:early]))),
                        "impulse_total_energy": float(energy.sum()),
                        "impulse_tail_energy_after_20ms": float(energy[early:].sum()),
                        "impulse_energy_centroid_seconds": float(np.dot(n/rate, energy)/energy.sum()),
                        "sampled_impulse_l1": float(np.abs(h).sum()),
                        "worst_analytical_gain": bound, "excitation_coefficients": coefficients,
                        "energy_scale": float(fields["modal_energy_scale"]),
                        "safety_scale": float(fields["modal_safety_scale"]),
                        "finite": True, "partition_exact": True})
    report = {"status": "research comparison; no default adoption", "Emax": 1,
              "conditioner": "hard (identity on these normal inputs)", "C3_residual_budget": 4,
              "numerical_margin": .99, "motion": 0, "weight_bound_covers": "all legal Motion",
              "seed": 42, "protect": "OFF", "human_review": "NOT ASSESSED", "rows": rows}
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(f"PASS: {len(rows)} C0/C3 grid rows, source and impulse renders, bounded/partition checks")


if __name__ == "__main__":
    main()
