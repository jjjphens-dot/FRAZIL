"""Bounded FD-003 convergence; consumes frozen FD-002 kernel qualification tables.

No retuning, production selection, listening score or relaxed Tier A tolerance.
"""

import argparse
import csv
import json
from pathlib import Path
import numpy as np
from flow_d1_convergence_models import registry
from flow_d1_latency_models import EPSILON, GUARDS, RATES, GuardKernel
from flow_d1_latency_study import sources, qualified_reference
from flow_d1_remediation_study import csv_write, metrics, continuous_path


def records(path):
    with path.open(encoding="utf-8-sig") as stream:
        return list(csv.DictReader(stream))


def kernel_authority(root):
    tables = {
        name: records(root / f"WATER_FLOW_D1_LATENCY_{name}.csv")
        for name in ("KERNELS", "MOVING", "CROSS_RATE")
    }
    expected = {"KERNELS": 117, "MOVING": 312, "CROSS_RATE": 208}
    if any(len(tables[k]) != n for k, n in expected.items()):
        raise ValueError("Incomplete frozen kernel evidence")
    names = [
        GuardKernel(family, g).name
        for g in GUARDS
        for family in (("lagrange",) if g == 0 else ("lagrange", "hann", "kaiser"))
    ]
    for table, rates, column, values in (
        ("KERNELS", RATES, "band_hz", (16000, 18000, 20000)),
        (
            "MOVING",
            RATES,
            "frequency_hz",
            (1000, 2000, 4000, 8000, 12000, 16000, 18000, 20000),
        ),
        (
            "CROSS_RATE",
            RATES[:2],
            "frequency_hz",
            (1000, 2000, 4000, 8000, 12000, 16000, 18000, 20000),
        ),
    ):
        actual = {
            (int(r["rate"]), r["kernel"], float(r[column])) for r in tables[table]
        }
        expected_keys = {
            (rate, name, value) for rate in rates for name in names for value in values
        }
        if actual != expected_keys or len(actual) != len(tables[table]):
            raise ValueError("Missing/duplicate frozen kernel grid")
    eligible = set()
    for r in tables["KERNELS"]:
        if r["band_hz"] != "16000" or r["static_pass"] != "True":
            continue
        rate, name = int(r["rate"]), r["kernel"]
        moving = [
            x
            for x in tables["MOVING"]
            if int(x["rate"]) == rate
            and x["kernel"] == name
            and float(x["frequency_hz"]) <= 16000
        ]
        cross = [
            x
            for x in tables["CROSS_RATE"]
            if x["kernel"] == name
            and float(x["frequency_hz"]) <= 16000
            and (rate == 96000 or int(x["rate"]) == rate)
        ]
        if len(moving) != 6 or len(cross) != (12 if rate == 96000 else 6):
            raise ValueError("Missing core kernel coverage")
        if all(x["core_pass"] == "True" for x in moving) and all(
            x["pair_budget_pass"] == "True" for x in cross
        ):
            eligible.add((rate, name))
    return eligible


def filter_metrics(c):
    f = np.unique(np.r_[np.linspace(0, c.rate / 2, 16385), 16000, 18000, 20000])
    h = c.response(f)
    aligned = h * np.exp(2j * np.pi * f * c.latency / c.rate)
    mag = 20 * np.log10(np.maximum(abs(h), 1e-300))
    core = f <= 16000
    phase = np.unwrap(np.angle(aligned[core]))
    group = -np.gradient(phase, 2 * np.pi * f[core])
    poles = [] if c.sos is None else np.concatenate([np.roots(s[3:]) for s in c.sos])
    stable = not len(poles) or max(abs(poles)) < 1
    impulse = np.zeros(c.rate // 4)
    onset = c.rate // 16
    impulse[onset] = 1
    y = c.apply(impulse)
    peak = int(np.argmax(abs(y)))
    centre = onset + c.latency if c.fir is not None else peak
    energy = float(y @ y)
    significant = np.flatnonzero(abs(y) >= max(abs(y)) * 1e-4)
    row = dict(
        rate=c.rate,
        conditioner=c.name,
        family=c.family,
        cutoff_hz=c.cutoff,
        cutoff_definition=(
            "identity"
            if c.family == "raw"
            else ("half amplitude" if c.fir is not None else "half power")
        ),
        order=c.order,
        filter_latency_samples=c.latency,
        core_max_magnitude_change_db=float(max(abs(mag[core]))),
        core_max_phase_change_rad=float(max(abs(phase))),
        core_group_min_us=float(min(group) * 1e6),
        core_group_max_us=float(max(group) * 1e6),
        core_complex_change=float(max(abs(aligned[core] - 1))),
        max_gain_db=float(max(mag)),
        stable=bool(stable),
        max_pole_radius=float(max(abs(poles))) if len(poles) else 0.0,
        before_main_energy_fraction=float(y[:centre] @ y[:centre] / energy),
        before_input_energy=float(y[:onset] @ y[:onset]),
        ringing_span_ms=float((significant[-1] - significant[0]) / c.rate * 1000),
    )
    for hz in (16000, 18000, 20000, c.rate / 2):
        row[f'gain_{"nyquist" if hz==c.rate/2 else int(hz)}_db'] = float(
            mag[np.flatnonzero(f == hz)[0]]
        )
    row["core_magnitude_pass"] = bool(max(abs(mag[core])) <= 0.1 + 1e-9)
    row["bounded_filter_pass"] = bool(
        stable and np.isfinite(h).all() and max(mag) <= 0.1 + 1e-9
    )
    return row, (f, h, y)


def ultrasonic_probe(rate, c, kernel):
    # Difference from ideal Doppler transfer, not rejection of physical sidebands.
    maximum = 0.05 / 1484 * rate
    delays_grid = np.linspace(0, maximum, 129)
    taps, weights = kernel.coefficients(delays_grid)
    base = taps[:, 0]
    offsets = taps[0] - base[0]
    f = np.linspace(0, rate / 2, 1025)
    h = weights @ np.exp(-2j * np.pi * f[:, None] / rate * offsets).T
    gain_db = float(20 * np.log10(max(abs(h).max(), 1e-300)))
    t = np.arange(rate // 2) / rate
    delay = continuous_path(t) / 1484 * rate
    worst, convergence = 0.0, 0.0
    finite = bool(np.isfinite(h).all())
    for frequency in (21000.0, 0.47 * rate):
        x = c.aligned(np.sin(2 * np.pi * frequency * t)[:, None])
        ideal, conv = qualified_reference(x, delay)
        y = kernel.apply(x, delay)
        cut = slice(256, -256)
        error = (y - ideal)[cut, 0]
        window = np.hanning(len(error))
        spectrum = np.fft.rfft(error * window)
        hz = np.fft.rfftfreq(len(error), 1 / rate)
        # Parseval with real-FFT interior weights; normalize by conditioned RMS.
        power = (
            2 * np.sum(abs(spectrum[(hz > 0) & (hz <= 16000)]) ** 2)
            + abs(spectrum[0]) ** 2
        )
        rms = np.sqrt(power / (len(error) * np.sum(window**2)))
        normalized = float(rms / max(np.sqrt(np.mean(x[cut] ** 2)), 1e-300))
        finite &= bool(
            np.isfinite(x).all()
            and np.isfinite(y).all()
            and np.isfinite(ideal).all()
            and np.isfinite(normalized)
            and np.isfinite(conv)
        )
        worst, convergence = max(worst, normalized), max(convergence, conv)
    return dict(
        kernel_max_gain_db=gain_db,
        ultrasonic_core_error=worst,
        ultrasonic_reference_convergence=convergence,
        tier_c_probe_pass=bool(
            finite
            and gain_db <= 0.1 + 1e-9
            and worst <= EPSILON
            and convergence <= EPSILON / 10
        ),
    )


def select_policies(selections):
    by = {
        (int(r["rate"]), r["conditioner"]): r
        for r in selections
        if r["numerical_pass"] and r["core_magnitude_pass"]
    }
    policies = []

    def add(name, names, role):
        for rate, conditioner in zip(RATES, names):
            r = next(
                (
                    x
                    for x in selections
                    if int(x["rate"]) == rate
                    and x["conditioner"] == conditioner
                    and x["numerical_pass"]
                ),
                None,
            )
            policies.append(
                dict(
                    policy=name,
                    role=role,
                    rate=rate,
                    conditioner=conditioner,
                    kernel="" if r is None else r["kernel"],
                    qualified=(rate, conditioner) in by,
                    total_latency_samples=(
                        "" if r is None else r["total_latency_samples"]
                    ),
                )
            )

    add("S0", ["raw-control"] * 3, "raw numerical control; failures retained")
    for family in ("butter4", "butter6", "fir65"):
        chosen = []
        for rate in RATES[:2]:
            options = [f"{family}-fc{fc}" for fc in (20000, 18000)]
            chosen.append(next((n for n in options if (rate, n) in by), ""))
        if all(chosen) and (96000, "raw-control") in by:
            add(
                f"S1-{family}",
                chosen + ["raw-control"],
                "primary architecture variant; human review pending",
            )
    add(
        "S2",
        ["butter4-fc20000"] * 3,
        "common intention comparison; core rejection retained",
    )
    return policies


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sources", type=Path, required=True)
    p.add_argument("--evidence", type=Path, default=Path("docs/evidence"))
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    eligible = kernel_authority(a.evidence)
    authority = json.loads((a.sources / "authority.json").read_text())
    if authority["sound_speed_mps"] != 1484 or authority["maximum_path_m"] != 0.05:
        raise ValueError("Physical authority drift")
    filters = []
    attempts = []
    joints = []
    selections = []
    actual = []
    for rate in RATES:
        fixtures = list(sources(a.sources, rate))
        raw_refs = {
            (profile, name): qualified_reference(audio, delay)
            for profile, name, audio, delay in fixtures
        }
        for c in registry(rate):
            row, arrays = filter_metrics(c)
            filters.append(row)
            np.savez_compressed(
                a.output / f"filter-{rate}-{c.name}.npz",
                hz=arrays[0],
                response=arrays[1],
                impulse=arrays[2],
            )
            prepared = []
            for profile, name, audio, delay in fixtures:
                conditioned = c.aligned(audio)
                ideal, convergence = (
                    raw_refs[profile, name]
                    if c.family == "raw"
                    else qualified_reference(conditioned, delay)
                )
                prepared.append(
                    (profile, name, audio, delay, conditioned, ideal, convergence)
                )
                actual.append(
                    dict(
                        rate=rate,
                        conditioner=c.name,
                        profile=profile,
                        source=name,
                        **metrics(conditioned, audio),
                        peak_ratio=float(
                            abs(conditioned).max() / max(abs(audio).max(), 1e-300)
                        ),
                    )
                )
            chosen = None
            for guard in GUARDS:
                passing = []
                for family in (
                    ("lagrange",) if guard == 0 else ("lagrange", "hann", "kaiser")
                ):
                    k = GuardKernel(family, guard)
                    if (rate, k.name) not in eligible:
                        attempts.append(
                            dict(
                                rate=rate,
                                conditioner=c.name,
                                kernel=k.name,
                                stage="frozen Tier A kernel rejection",
                                passed=False,
                                worst_error="",
                                kernel_max_gain_db="",
                                ultrasonic_core_error="",
                                ultrasonic_reference_convergence="",
                                tier_c_probe_pass=False,
                            )
                        )
                        continue
                    tier_c = ultrasonic_probe(rate, c, k)
                    passed = row["bounded_filter_pass"] and tier_c["tier_c_probe_pass"]
                    worst = 0.0
                    for (
                        profile,
                        name,
                        audio,
                        delay,
                        conditioned,
                        ideal,
                        convergence,
                    ) in prepared:
                        y = k.apply(conditioned, delay)
                        values = metrics(y, ideal)
                        error = max(
                            values["relative_rms_error"],
                            values["peak_normalized_error"],
                        )
                        worst = max(worst, error)
                        ok = (
                            values["finite"]
                            and convergence <= EPSILON / 10
                            and error <= EPSILON
                        )
                        passed = passed and ok
                        raw_ideal, raw_convergence = raw_refs[profile, name]
                        whole = metrics(y, raw_ideal)
                        joints.append(
                            dict(
                                rate=rate,
                                conditioner=c.name,
                                kernel=k.name,
                                profile=profile,
                                source=name,
                                **values,
                                reference_convergence=convergence,
                                joint_pass=bool(ok),
                                whole_chain_raw_nrms=whole["relative_rms_error"],
                                whole_chain_raw_peak=whole["peak_normalized_error"],
                                raw_reference_convergence=raw_convergence,
                            )
                        )
                    attempts.append(
                        dict(
                            rate=rate,
                            conditioner=c.name,
                            kernel=k.name,
                            stage="actual and Tier C",
                            passed=bool(passed),
                            worst_error=worst,
                            **tier_c,
                        )
                    )
                    if passed:
                        passing.append((worst, k, tier_c))
                if passing:
                    _, chosen, tier_c = min(passing, key=lambda x: (x[0], x[1].name))
                    break
            result = dict(
                rate=rate,
                conditioner=c.name,
                core_magnitude_pass=row["core_magnitude_pass"],
                numerical_pass=chosen is not None,
                kernel="" if chosen is None else chosen.name,
                guard_samples="" if chosen is None else chosen.guard,
                filter_latency_samples=c.latency,
                total_latency_samples=(
                    "" if chosen is None else c.latency + chosen.guard
                ),
                acceptance="engineering screen only; filter phase/transient/human acceptance pending",
            )
            if chosen is not None:
                result.update(tier_c)
            else:
                result.update(
                    dict(
                        kernel_max_gain_db="",
                        ultrasonic_core_error="",
                        ultrasonic_reference_convergence="",
                        tier_c_probe_pass=False,
                    )
                )
            selections.append(result)
            for label, data in [
                ("FILTERS", filters),
                ("ACTUAL", actual),
                ("ATTEMPTS", attempts),
                ("JOINT", joints),
                ("SELECTION", selections),
            ]:
                csv_write(a.output / f"{label}.csv", data)
            print(
                "convergence",
                rate,
                c.name,
                "guard",
                result["guard_samples"],
                "core filter",
                row["core_magnitude_pass"],
                flush=True,
            )
    policies = select_policies(selections)
    csv_write(a.output / "POLICIES.csv", policies)
    (a.output / "report.json").write_text(
        json.dumps(
            dict(
                contract="EXP-W-FD-003",
                conditioners=len(filters),
                numerical_pass=sum(r["numerical_pass"] for r in selections),
                policy_rows=len(policies),
                runtime_replaced=False,
                human_acceptance="NOT ASSESSED",
                source_authority=authority,
            ),
            indent=2,
        ),
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
