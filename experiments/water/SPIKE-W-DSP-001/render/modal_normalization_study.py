"""Offline C0/C1/C2 normalization study for the actual imaginary-output recurrence.

This does not change DSP or select a perceptual winner. Run with an ignored --output directory.
C1 normalizes single-mode impulse energy to one; C2 normalizes its peak to one.
Both are candidate formulas, not safe defaults. The finite-float counterexample is mandatory.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import numpy as np

RATIOS = (1., 1.41, 1.93, 2.57, 3.31, 4.17)

def study(rate, root, decay):
    r = np.exp(-1 / (rate * decay))
    n = np.arange(int(np.ceil(30 * decay * rate)))
    modes = []
    coefficients = []
    for ratio in RATIOS:
        theta = 2 * np.pi * root * ratio / rate
        unit = np.power(r, n) * np.sin(theta * n)
        # h[n]=b*r^n*sin(n*theta), because DampedResonator returns its imaginary state.
        # Sum q^n*sin^2(n*theta) = 1/(2(1-q)) - Re(1/(1-q*exp(2i*theta)))/2.
        q = r * r
        energy = .5 / (1 - q) - .5 * (1 - q * np.cos(2 * theta)) / (1 - 2*q*np.cos(2*theta) + q*q)
        if not np.isclose(np.dot(unit, unit), energy, rtol=1e-8):
            raise AssertionError("Closed-form energy disagrees with sampled impulse")
        # Continuous upper peak for exp(-t/tau)*sin(omega*t), first lobe.
        # Sampling can only reduce this peak; this is a conservative peak normalization.
        t = np.arctan(theta / -np.log(r)) / theta
        peak_bound = np.exp(t * np.log(r)) * np.sin(theta * t)
        coefficients.append((1-r, 1/np.sqrt(energy), 1/peak_bound))
        modes.append(unit)
    rows = []
    for candidate in range(3):
        h = sum(modes[i] * coefficients[i][candidate] for i in range(6)) * (.3 / 6)
        peak = float(np.max(np.abs(h)))
        l1 = float(np.sum(np.abs(h)))
        # x[N-k] = FLT_MAX*sign(h[k]) is a finite, legal float input. At N,
        # y[N] / FLT_MAX = sum(abs(h)); >1 proves float output cannot remain finite.
        rows.append(dict(candidate=f"C{candidate}", rate=rate, root=root, decay=decay,
                         excitation=[float(c[candidate]) for c in coefficients],
                         peak=peak, rms=float(np.sqrt(np.mean(h*h))),
                         early_20ms_peak=float(np.max(np.abs(h[:int(.02*rate)]))),
                         energy=float(np.dot(h,h)), tail_energy=float(np.dot(h[int(.02*rate):],h[int(.02*rate):])),
                         tail_last_peak=float(np.max(np.abs(h[-int(.01*rate):]))),
                         l1_gain=l1, finite_normalized_input=bool(np.isfinite(h).all()),
                         finite_float_counterexample=l1 > 1,
                         worst_float_multiple=l1))
    return rows

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args=parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    rows=[row for rate in (44100,48000,96000) for root in (130,260,520)
          for decay in (.03,.12,.48) for row in study(rate,root,decay)]
    report={"status":"EXPERIMENT ONLY; no DSP adoption", "motion":0, "gain":.3,
            "tail_seconds":"30*decay", "rows":rows,
            "counterexample":"Finite float input FLT_MAX * reversed sign of modal impulse response",
            "warning":"Single impulse peak/energy normalization does not guarantee float-range safety"}
    (args.output/"report.json").write_text(json.dumps(report,indent=2,allow_nan=False)+"\n", encoding="utf-8")
    for c in ("C0","C1","C2"):
        values=[v for v in rows if v["candidate"]==c]
        print(c, "max_l1", max(v["l1_gain"] for v in values),
              "float_counterexamples", sum(v["finite_float_counterexample"] for v in values), "/",len(values))

if __name__ == "__main__":
    main()
