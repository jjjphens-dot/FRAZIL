# PERF-BASE-001 Reference Baseline

- Status: measured Release engineering baseline; no formal CPU percentage threshold is defined.
- Implementation commit: `6d4be8fe68214cd2f9ccd8204f0ca6cec81edfc2`
- Configure/runtime provenance from the clean Release run:
  `configured_commit=6d4be8fe68214cd2f9ccd8204f0ca6cec81edfc2`,
  `configured_source_state=clean`, `runtime_commit=6d4be8fe68214cd2f9ccd8204f0ca6cec81edfc2`,
  `runtime_source_state=clean`, `formal_provenance_status=PASS`
- Measurement date: 2026-09-11
- Reference machine: Windows 11 Home China 23H2, build 22631, x64; Intel Core i9-14900HX,
  32 logical CPUs, 16003 MiB RAM
- Toolchain: MSVC `_MSC_VER=1943`, JUCE 9.0.1
- Build type: Release
- Effective compiler flags: `/DWIN32 /D_WINDOWS /EHsc /O2 /Ob2 /DNDEBUG`

## Scope and execution model

The canonical implementation is the manual `frazil_performance` benchmark at
`tests/performance/performance_main.cpp`. It measures only the current M1 `AudioEngine`
pass-through/gain skeleton, without Water, Ice, Routing, UI, a plugin editor, or a running DAW.
It is not registered as a CTest pass/fail gate.

```text
sample rate: 48000 Hz
block size: 128 samples
channels: 2
warm-up: 2000 blocks per scenario
measurement window: 20000 blocks per scenario
input: deterministic 440 Hz stereo reference oscillator with continuous phase across blocks,
  generated in the benchmark; the oscillator state is continuous across warm-up and measurement
Reference DAW: N/A; headless AudioEngine benchmark
measurement tool: std::chrono::steady_clock around AudioEngine::process
thread configuration: one benchmark process thread
instance configuration: one AudioEngine instance per scenario
statistical method: arithmetic mean, nearest-rank P95/P99, maximum
```

Run from a clean tree with a fresh configure:

```powershell
cmake --fresh --preset windows-release
python tools/build_safe.py --preset windows-release
ctest --preset windows-release
.\build\windows-release\frazil_performance.exe
```

## Benchmark results

| Scenario | Mean callback (us) | P95 (us) | P99 (us) | Worst (us) | Deadline (us) | Mean deadline use | Worst deadline use | Harness process CPU observation | Measured callback `operator new` | Finite output |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| steady-state | 0.807 | 0.900 | 0.900 | 60.400 | 2666.667 | 0.030% | 2.265% | 29.507% | 0 | PASS |
| parameter-retarget | 0.849 | 1.000 | 1.600 | 51.800 | 2666.667 | 0.032% | 1.943% | 59.382% | 0 | PASS |

`harness_process_cpu_percent` is process-wide CPU time divided by wall time for the complete
benchmark measurement window. The window includes reference signal generation, parameter-retarget
setup, timing calls, result bookkeeping, and finite-output scanning around the separately timed
`AudioEngine::process` call. It is not an `AudioEngine::process`-only CPU utilization metric and
is not a formal performance budget.
Working-set observations were 3.957 MiB before and 4.102 MiB after/peak for steady-state, and
3.953 MiB before and 4.102 MiB after/peak for parameter-retarget.

`configured_commit` and `configured_source_state` are captured by CMake at configure time.
`runtime_commit` and `runtime_source_state` are read by the executable before formal measurement.
`formal_provenance_status=PASS` requires matching commits and `clean` for both source states;
dirty, unknown, or mismatched provenance is `NOT RUN` and does not force the benchmark process to
fail.

## Denormal and finite-output observation

The benchmark runs a separate prepared `AudioEngine` probe with 256 subnormal input samples
(`std::numeric_limits<float>::denorm_min()` across 2 x 128 samples):

```text
input_subnormal_samples: 256
output_subnormal_samples: 256
output_nonfinite_samples: 0
denormal_probe_status: OBSERVED
denormal_finite_output_status: PASS
```

The probe records observed handling; `denormal_finite_output_status=PASS` means only that the
processed output remained finite. It does not claim a platform-independent flush-to-zero mode.
Both benchmark scenarios also reported finite output, and the measured callback allocation
observer recorded zero `operator new` calls for the selected `AudioEngine::process` workload; it
is not complete `FRAZILAudioProcessor::processBlock` allocation-free evidence.

## Validation boundary

- Release fresh configure, safe build and CTest: **7/7 PASS**. The manual benchmark is separate.
- ASAN fresh configure, safe build and CTest: **7/7 PASS**.
- Provenance regression cases: clean matching configure/runtime Git reported `PASS`; a tracked
  source mutation after configure reported `runtime_source_state=dirty` and `NOT RUN`; execution
  from a different clean Git repository reported a runtime commit mismatch and `NOT RUN`; and
  execution with Git unavailable reported `runtime_commit=unknown`,
  `runtime_source_state=unknown`, and `NOT RUN`. In each negative case the benchmark itself still
  returned exit code 0 with finite output.
- This evidence does not claim pluginval, real DAW, listening, offline render coverage beyond the
  existing RENDER-001 smoke, or a formal M1 Joint Exit.
- Dirty or unknown `configured_source_state`/`runtime_source_state`, a runtime commit mismatch, or
  unavailable Git is visibly reported by the executable and is not formal baseline evidence; only
  a clean fresh-configure run with `formal_provenance_status=PASS` is recorded above.
