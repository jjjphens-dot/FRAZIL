"""FD-003 cross-rate diagnostics; no automatic perceptual similarity score/gate."""

import argparse
from pathlib import Path
import numpy as np
from flow_d1_convergence_models import registry
from flow_d1_convergence_study import records
from flow_d1_latency_models import GuardKernel, RATES, EPSILON
from flow_d1_latency_study import sources, PROFILES, nonlinear_filter_probe
from flow_d1_remediation_study import csv_write, metrics, continuous_path


def envelope(x, rate):
    edges = np.ceil(np.arange(1001) * rate / 1000).astype(int)
    return np.array(
        [np.sqrt(np.mean(x[a:b] ** 2)) for a, b in zip(edges[:-1], edges[1:])]
    )


def compare(x, rate, reference):
    a = abs(np.fft.rfft(x[:, 0])) / rate
    b = abs(np.fft.rfft(reference[:, 0])) / 96000
    values = {}
    for lo, hi in ((0, 16000), (16000, 20000)):
        # All original fixtures last exactly one second: identical integer-Hz bins.
        start = lo + 1 if lo else 0
        left = a[start : hi + 1]
        right = b[start : hi + 1]
        values[f"band_{lo}_{hi}_spectral_nrms"] = float(
            np.linalg.norm(left - right) / max(np.linalg.norm(right), 1e-300)
        )
        values[f"band_{lo}_{hi}_energy_ratio_db"] = float(
            20
            * np.log10(
                max(np.linalg.norm(left), 1e-300) / max(np.linalg.norm(right), 1e-300)
            )
        )
    e, en = envelope(x, rate), envelope(reference, 96000)
    values["envelope_nrms"] = float(
        np.linalg.norm(e - en) / max(np.linalg.norm(en), 1e-300)
    )
    values["peak_envelope_time_delta_ms"] = int(np.argmax(e)) - int(np.argmax(en))
    return values


def event_comparison(root):
    rows = []
    for profile in PROFILES:
        reference = records(root / f"96000-{profile}-events.csv")
        for rate in RATES[:2]:
            events = records(root / f"{rate}-{profile}-events.csv")
            for source in ("A1", "B1"):
                left = [r for r in events if r["source"] == source]
                right = [r for r in reference if r["source"] == source]
                paired = list(zip(left, right))
                rows.append(
                    dict(
                        rate=rate,
                        reference_rate=96000,
                        profile=profile,
                        source=source,
                        event_count=len(left),
                        reference_event_count=len(right),
                        ordinal_pairs=len(paired),
                        ordinal_identity_token_matches=sum(
                            a["identity_token"] == b["identity_token"]
                            for a, b in paired
                        ),
                        ordinal_radius_matches=sum(
                            a["radius_m"] == b["radius_m"] for a, b in paired
                        ),
                        max_ordinal_time_delta_ms=max(
                            (
                                abs(float(a["time_s"]) - float(b["time_s"])) * 1000
                                for a, b in paired
                            ),
                            default=0.0,
                        ),
                        admitted_count=sum(r["admitted"] == "1" for r in left),
                        reference_admitted_count=sum(
                            r["admitted"] == "1" for r in right
                        ),
                        scope="unchanged source event streams; ordinal pairing is diagnostic, not same-event proof",
                    )
                )
    return rows


def analytic_policy_pairs(policies):
    """Compare different per-rate kernels on the same continuous path/clock.

    A steady sinusoid is an independent closed-form reference. Conditioner phase
    remains in whole-chain differences; intrinsic kernel error is reported apart.
    No stochastic source events enter this controlled propagation comparison.
    """
    rows = []
    times = np.arange(30, 121) / 300
    path_seconds = continuous_path(times) / 1484
    for policy in sorted({r["policy"] for r in policies}):
        cells = {int(r["rate"]): r for r in policies if r["policy"] == policy}
        for frequency in (1000, 2000, 4000, 8000, 12000, 16000, 18000, 20000):
            values = {}
            ideal = np.exp(-2j * np.pi * frequency * path_seconds)
            for rate, cell in cells.items():
                if not cell["kernel"]:
                    continue
                c = next(c for c in registry(rate) if c.name == cell["conditioner"])
                family, g = cell["kernel"].split("-g")
                taps, weights = GuardKernel(family, int(g)).coefficients(
                    path_seconds * rate
                )
                numeric = np.sum(
                    weights * np.exp(-2j * np.pi * frequency / rate * taps), axis=1
                )
                conditioner = c.response(np.array([float(frequency)]))[0] * np.exp(
                    2j * np.pi * frequency * c.latency / rate
                )
                values[rate] = (numeric - ideal, conditioner * numeric)
            if 96000 not in values:
                continue
            for rate in RATES[:2]:
                if rate not in values:
                    continue
                error = values[rate][0] - values[96000][0]
                chain = values[rate][1] - values[96000][1]
                finite = bool(np.isfinite(error).all() and np.isfinite(chain).all())
                rows.append(
                    dict(
                        policy=policy,
                        rate=rate,
                        reference_rate=96000,
                        frequency_hz=frequency,
                        kernel_pair_error_rms=float(np.sqrt(np.mean(abs(error) ** 2))),
                        kernel_pair_error_peak=float(max(abs(error))),
                        whole_chain_difference_rms=float(
                            np.sqrt(np.mean(abs(chain) ** 2))
                        ),
                        tier_a_pair_pass=(
                            bool(finite and max(abs(error)) <= 2 * EPSILON)
                            if frequency <= 16000
                            else ""
                        ),
                        scope="common clock analytic transfer; whole-chain filter change not accepted",
                    )
                )
    return rows


def boundary_audit(fixtures):
    rows = []
    for rate in RATES:
        for c in registry(rate):
            for (profile, source), (audio, _) in fixtures[rate].items():
                y = c.aligned(np.pad(audio, ((0, 128), (0, 0))))
                ratio = float(abs(y[len(audio) :]).max() / max(abs(y).max(), 1e-300))
                rows.append(
                    dict(
                        rate=rate,
                        conditioner=c.name,
                        profile=profile,
                        source=source,
                        next128_tail_peak_ratio=ratio,
                        finite_window_pass=bool(
                            np.isfinite(y).all() and ratio <= 1e-12
                        ),
                    )
                )
    return rows


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sources", type=Path, required=True)
    p.add_argument("--study", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    policies = records(a.study / "POLICIES.csv")
    rows = []
    movement = []
    fixtures = {
        rate: {
            (profile, source): (audio, delay)
            for profile, source, audio, delay in sources(a.sources, rate)
        }
        for rate in RATES
    }
    analytic = analytic_policy_pairs(policies)
    csv_write(a.output / "ANALYTIC_CROSS_RATE.csv", analytic)
    if not analytic or any(r["tier_a_pair_pass"] is False for r in analytic):
        raise ValueError("Different-kernel Tier A cross-rate gate failed")
    boundary = boundary_audit(fixtures)
    csv_write(a.output / "BOUNDARY.csv", boundary)
    if not all(r["finite_window_pass"] for r in boundary):
        raise ValueError(
            "Finite-window closure failed; extend the reference, do not ignore tails"
        )
    for policy in sorted({r["policy"] for r in policies}):
        cells = {int(r["rate"]): r for r in policies if r["policy"] == policy}
        for profile in PROFILES:
            for source in ("A1", "B1", "AB"):
                outputs = {}
                for rate in RATES:
                    row = cells[rate]
                    audio, delay = fixtures[rate][profile, source]
                    c = next(c for c in registry(rate) if c.name == row["conditioner"])
                    conditioned = c.aligned(audio)
                    outputs[rate, "raw"] = audio
                    outputs[rate, "D-OFF"] = conditioned
                    if row["kernel"]:
                        family, g = row["kernel"].split("-g")
                        k = GuardKernel(family, int(g))
                        y = k.apply(conditioned, delay)
                        outputs[rate, "D-ON"] = y
                        movement.append(
                            dict(
                                policy=policy,
                                rate=rate,
                                profile=profile,
                                source=source,
                                **metrics(y, conditioned),
                                scope="physical effect plus numerical residual; not listening score",
                            )
                        )
                for rate in RATES[:2]:
                    for stage in ("raw", "D-OFF", "D-ON"):
                        if (rate, stage) not in outputs or (
                            96000,
                            stage,
                        ) not in outputs:
                            continue
                        rows.append(
                            dict(
                                policy=policy,
                                rate=rate,
                                reference_rate=96000,
                                profile=profile,
                                source=source,
                                stage=stage,
                                **compare(
                                    outputs[rate, stage], rate, outputs[96000, stage]
                                ),
                                acceptance="cross-rate listening pending; baseline source variation retained",
                            )
                        )
        print("cross-rate", policy, "complete", flush=True)
    paths = []
    for profile in PROFILES:
        _, d96 = fixtures[96000][profile, "AB"]
        for rate in RATES[:2]:
            _, d = fixtures[rate][profile, "AB"]
            t = np.arange(300) / 300
            left = d[np.rint(t * rate).astype(int)] / rate
            right = d96[np.rint(t * 96000).astype(int)] / 96000
            paths.append(
                dict(
                    rate=rate,
                    reference_rate=96000,
                    profile=profile,
                    max_path_time_difference_s=float(max(abs(left - right))),
                )
            )
    nonlinear = [r for c in registry(96000) for r in nonlinear_filter_probe(c)]
    csv_write(a.output / "NONLINEAR.csv", nonlinear)
    csv_write(a.output / "CROSS_RATE.csv", rows)
    csv_write(a.output / "MOVEMENT.csv", movement)
    csv_write(a.output / "EVENTS.csv", event_comparison(a.sources))
    csv_write(a.output / "PATHS.csv", paths)


if __name__ == "__main__":
    main()
