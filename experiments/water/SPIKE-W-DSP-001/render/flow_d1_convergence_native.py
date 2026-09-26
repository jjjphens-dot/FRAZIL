"""Measure only unique FD-003 policy shortlist cells with the existing native harness."""

import argparse
from pathlib import Path
from flow_d1_convergence_models import registry
from flow_d1_convergence_study import records
from flow_d1_latency_models import EPSILON, GuardKernel
from flow_d1_latency_study import sources, qualified_reference
from flow_d1_latency_native_study import run_native
from flow_d1_remediation_study import metrics, csv_write


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--native", type=Path, required=True)
    p.add_argument("--sources", type=Path, required=True)
    p.add_argument("--study", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    # Rejected S0/S2 cells with a qualified numerical kernel remain explicit controls.
    cells = {
        (int(r["rate"]), r["conditioner"], r["kernel"])
        for r in records(a.study / "POLICIES.csv")
        if r["kernel"]
    }
    results = []
    resources = []
    for rate, name, kernel in sorted(cells):
        c = next(c for c in registry(rate) if c.name == name)
        family, g = kernel.split("-g")
        k = GuardKernel(family, int(g))
        for profile, source, audio, delay in sources(a.sources, rate):
            if source != "AB":
                continue
            label = f"{rate}-{name}-{kernel}-{profile}"
            y, implementation, measurements = run_native(
                a.native.resolve(),
                a.output / label,
                c,
                k,
                audio,
                delay,
                quick=profile != "overlap-reference",
            )
            ideal, conv = qualified_reference(c.aligned(audio), delay)
            error = metrics(y, ideal)
            passed = (
                error["finite"]
                and max(error["relative_rms_error"], error["peak_normalized_error"])
                <= EPSILON
                and conv <= EPSILON / 10
            )
            results.append(
                dict(
                    rate=rate,
                    conditioner=name,
                    kernel=kernel,
                    profile=profile,
                    **error,
                    reference_convergence=conv,
                    implementation_nrms=implementation["relative_rms_error"],
                    implementation_peak_error=implementation["peak_normalized_error"],
                    native_numerical_pass=bool(passed),
                )
            )
            resources.extend(
                dict(
                    rate=rate,
                    conditioner=name,
                    kernel=kernel,
                    profile=profile,
                    **m,
                    total_latency_samples=c.latency + k.guard,
                    scope="candidate and harness only; not plugin budget",
                )
                for m in measurements
            )
            csv_write(a.output / "NATIVE.csv", results)
            csv_write(a.output / "RESOURCES.csv", resources)
        print("native shortlist", rate, name, kernel, "complete", flush=True)
    if not results or not all(r["native_numerical_pass"] for r in results):
        raise SystemExit("Native shortlist failed; no adoption")


if __name__ == "__main__":
    main()
