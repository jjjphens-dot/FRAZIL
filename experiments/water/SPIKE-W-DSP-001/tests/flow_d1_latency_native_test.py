"""Streaming realization versus untabulated model, including path clock and reset."""
import sys
import argparse
import os
import faulthandler
import tempfile
from pathlib import Path

faulthandler.enable()
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'render'))
from flow_d1_latency_models import Conditioner, GuardKernel
from flow_d1_latency_native_study import run_native
from native_case_evidence import record


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('--child-runtime-dir', type=Path)
    parser.add_argument('--evidence-root', type=Path)
    args = parser.parse_args()
    executable = args.executable.resolve()
    root = Path(__file__).resolve().parents[4]/'build'
    root.mkdir(exist_ok=True)
    child_environment = None
    if args.child_runtime_dir:
        child_environment = os.environ.copy()
        child_environment['PATH'] = str(args.child_runtime_dir.resolve()) + os.pathsep + os.environ['PATH']
    # Keep successful and failing runs; a parent crash must not erase case provenance.
    if args.evidence_root:
        if not args.evidence_root.resolve().is_relative_to(root.resolve()):
            parser.error('Evidence must stay under build/')
        args.evidence_root.mkdir(parents=True, exist_ok=False)
        temporary = str(args.evidence_root.resolve())
    else:
        temporary = tempfile.mkdtemp(prefix='latency-native-', dir=root)
    with (Path(temporary)/'parent-fault.log').open('w', encoding='utf-8') as fault_log:
        faulthandler.enable(file=fault_log)
        record(temporary, 'parent-start', executable=str(executable), python=sys.executable)
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
                    fixture = 'float-limit' if rate == 96000 and taps == 65 and guard == 64 else 'moving-zero-path'
                    directory = Path(temporary)/f'RATE_{rate}_{"IIR" if taps is None else "FIR65"}_{kernel.name}_{fixture}'
                    record(temporary, 'case-start', case_id=directory.name)
                    values = audio
                    if rate == 96000 and taps == 65 and guard == 64:
                        values = audio*(np.finfo(np.float32).max/4)
                    run_native(executable, directory, conditioner, kernel, values, delay, quick=True,
                               child_environment=child_environment)
                    record(temporary, 'case-finish', case_id=directory.name)
        faulthandler.enable()
    print('24 native IIR/FIR/guard/rate cases: model, reset, partition and allocation PASS')


if __name__ == '__main__':
    main()
