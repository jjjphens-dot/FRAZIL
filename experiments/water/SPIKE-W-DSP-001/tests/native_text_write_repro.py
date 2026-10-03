"""Manual, bounded Python/NumPy text-write crash isolation; no DSP/native child.

Run with the same interpreter as the failing CTest. A passing run does not clear the
original fault. Output must be a new directory in the repository's ignored build tree.
"""

import argparse
import faulthandler
import json
from pathlib import Path
import platform
import sys
import time

faulthandler.enable()
import numpy as np


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--iterations", type=int, choices=range(1, 9), default=4)
    args = parser.parse_args()
    root = args.output.resolve()
    if not root.is_relative_to(Path(__file__).resolve().parents[4] / "build"):
        parser.error("Output must stay under build/")
    root.mkdir(parents=True, exist_ok=False)
    metadata = dict(python=sys.version, numpy=np.__version__, platform=platform.platform(),
                    native_child=False, dsp=False, values_per_iteration=4097 * 129,
                    iterations=args.iterations)
    (root / "context.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    # Same scalar write shape/format as the coefficient exporter, without its calculations.
    values = np.linspace(-1., 1., metadata["values_per_iteration"])
    with (root / "fault.log").open("w", encoding="utf-8") as fault:
        faulthandler.enable(file=fault)
        for i in range(args.iterations):
            start = time.monotonic()
            print(f"iteration={i} stage=start", flush=True)
            with (root / f"values-{i}.txt").open("w", encoding="utf-8") as stream:
                np.savetxt(stream, values, fmt="%.17g")
            print(f"iteration={i} stage=finish seconds={time.monotonic() - start}", flush=True)
        faulthandler.enable()


if __name__ == "__main__":
    main()
