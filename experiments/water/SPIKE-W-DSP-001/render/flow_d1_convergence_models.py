"""Fixed-character offline registry for EXP-W-FD-003; no production consumer."""

import numpy as np
from scipy import signal
from flow_d1_latency_models import Conditioner, RATES


class FixedConditioner(Conditioner):
    """Reuse causal/aligned filter behavior; replace only the design specification.

    IIR labels are -3 dB digital cutoffs; FIR labels are half-amplitude cutoffs.
    Bessel uses bilinear mapping and does not promise analog high-frequency phase.
    """

    def __init__(self, rate, family="raw", cutoff=0):
        if rate not in RATES or family not in (
            "raw",
            "butter4",
            "butter6",
            "bessel4",
            "fir33",
            "fir65",
        ):
            raise ValueError("Outside fixed-character registry")
        if (family == "raw" and cutoff != 0) or (
            family != "raw" and cutoff not in (18000, 20000)
        ):
            raise ValueError("Outside cutoff shortlist")
        self.rate, self.family, self.cutoff = rate, family, cutoff
        self.sos = self.fir = None
        self.latency = 0
        self.name = "raw-control" if family == "raw" else f"{family}-fc{cutoff}"
        if family == "raw":
            self.fir, self.order = np.array([1.0]), 0
        elif family.startswith("fir"):
            taps = int(family[3:])
            self.fir = signal.firwin(
                taps,
                cutoff,
                fs=rate,
                window=("kaiser", signal.kaiser_beta(60)),
                scale=True,
            )
            self.order, self.latency = taps - 1, (taps - 1) // 2
        else:
            self.order = int(family[-1])
            design = signal.bessel if family.startswith("bessel") else signal.butter
            kwargs = {"norm": "mag"} if family.startswith("bessel") else {}
            self.sos = design(self.order, cutoff, fs=rate, output="sos", **kwargs)


def registry(rate):
    yield FixedConditioner(rate)
    if rate == 96000:
        yield FixedConditioner(rate, "butter4", 20000)
    else:
        for family in ("butter4", "butter6", "bessel4", "fir33", "fir65"):
            for cutoff in (18000, 20000):
                yield FixedConditioner(rate, family, cutoff)
