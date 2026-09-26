"""Streaming realization versus untabulated model, including path clock and reset."""
import sys
import tempfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'render'))
from flow_d1_latency_models import Conditioner, GuardKernel
from flow_d1_latency_native_study import run_native


def main():
    executable = Path(sys.argv[1]).resolve()
    root = Path(__file__).resolve().parents[4]/'build'
    root.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='latency-native-', dir=root) as temporary:
        for rate in (44100, 48000, 96000):
            n = np.arange(4096)
            audio = np.column_stack((np.sin(2*np.pi*12000*n/rate),
                                     .3*np.cos(2*np.pi*18000*n/rate)))
            audio[:256] = audio[-256:] = 0
            maximum = .05/1484*rate
            # Exercise moving boundaries and exact identity without retuning sources.
            delay = maximum*(.5+.5*np.sin(n*.023))
            delay[1024:1536] = 0
            for taps in (None, 65):
                conditioner = Conditioner(rate, 16000, min(24000, .49*rate), taps)
                for guard in (8, 16, 32, 64):
                    kernel = GuardKernel('hann' if rate == 96000 and guard == 8 else 'kaiser', guard)
                    directory = Path(temporary)/f'{rate}-{taps}-{guard}'
                    values = audio
                    if rate == 96000 and taps == 65 and guard == 64:
                        values = audio*(np.finfo(np.float32).max/4)
                    run_native(executable, directory, conditioner, kernel, values, delay, quick=True)
    print('24 native IIR/FIR/guard/rate cases: model, reset, partition and allocation PASS')


if __name__ == '__main__':
    main()
