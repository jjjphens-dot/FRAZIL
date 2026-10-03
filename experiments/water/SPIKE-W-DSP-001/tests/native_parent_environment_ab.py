"""Manual Phase 0 diagnostic: only the Python parent's MSVC runtime PATH entry differs.

Runs the unchanged 24-case numerical workload in fixed A/B order. Both native children
receive the same PATH. All attempts survive, including failures; no retry-until-PASS.
"""

import argparse
import json
import os
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "render"))
from native_case_evidence import run_case


def environments(environment, runtime):
    normal = environment.copy()
    normal["PATH"] = os.pathsep.join(part for part in environment["PATH"].split(os.pathsep)
                                  if Path(part).resolve() != runtime)
    inherited = normal.copy()
    inherited["PATH"] = str(runtime) + os.pathsep + normal["PATH"]
    return normal, inherited


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--runtime-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--rounds", type=int, choices=range(1, 4), default=2)
    args = parser.parse_args()
    root, runtime = args.output.resolve(), args.runtime_dir.resolve(strict=True)
    build = Path(__file__).resolve().parents[4] / "build"
    if not root.is_relative_to(build.resolve()):
        parser.error("Output must stay under build/")
    root.mkdir(parents=True, exist_ok=False)
    normal, inherited = environments(os.environ, runtime)
    (root / "environment.json").write_text(json.dumps(dict(
        python=sys.executable, native=str(args.native.resolve()), runtime=str(runtime),
        normal_path=normal["PATH"], inherited_path=inherited["PATH"],
        rounds=args.rounds, order=["A-parent-inherited", "B-child-only"],
        boundary="Only the specified runtime PATH entry is isolated; no DSP or library changes"),
        indent=2), encoding="utf-8")
    rows = []
    for iteration in range(1, args.rounds + 1):
        for label, env in (("A-parent-inherited", inherited), ("B-child-only", normal)):
            case = f"round-{iteration}-{label}"
            directory = root / case
            command = [sys.executable, str(Path(__file__).with_name("flow_d1_latency_native_test.py")),
                       str(args.native.resolve()), "--evidence-root", str(directory / "cases")]
            if label == "B-child-only":
                command += ["--child-runtime-dir", str(runtime)]
            try:
                run_case(command, directory, case, None, environment=env)
                result = "PASS"
            except RuntimeError as error:
                # Continue the preregistered matrix, preserving each first failure.
                result = str(error)
            rows.append(dict(round=iteration, variant=label, result=result))
            (root / "RESULTS.json").write_text(json.dumps(rows, indent=2), encoding="utf-8")
    print(json.dumps(rows, indent=2))
    print("Both stable means NOT REPRODUCED, not fixed; differences alone do not prove root cause.")


if __name__ == "__main__":
    main()
