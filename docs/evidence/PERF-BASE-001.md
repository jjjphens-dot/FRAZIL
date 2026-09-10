# PERF-BASE-001 Reference Baseline

- Status: measured Release engineering baseline; no formal CPU percentage threshold is defined.
- Implementation commit: `e56664c8775f614e77004ac891d63a5c2fd2fa7a`
- Configure provenance: `configured_commit=e56664c8775f614e77004ac891d63a5c2fd2fa7a`,
  `source_state=clean`
- Measurement date: 2026-09-10
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
input: deterministic 440 Hz stereo reference block, generated in the benchmark
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

| Scenario | Mean callback (us) | P95 (us) | P99 (us) | Worst (us) | Deadline (us) | Mean deadline use | Worst deadline use | Process CPU observation | Measured callback `operator new` | Finite output |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| steady-state | 1.058 | 1.300 | 1.400 | 37.300 | 2666.667 | 0.040% | 1.399% | 0.000% | 0 | PASS |
| parameter-retarget | 0.970 | 1.000 | 1.700 | 218.100 | 2666.667 | 0.036% | 8.179% | 0.000% | 0 | PASS |

The process CPU observation is process-wide CPU time divided by wall time for each measurement
window. The observed value rounded to `0.000%` because the Windows CPU-time clock resolution was
coarser than these short windows; it is retained as an observation and is not a performance gate.
Working-set observations were 3.938 MiB before and 4.094 MiB after/peak for steady-state, and
4.094 MiB before and 4.094 MiB after/peak for parameter-retarget.

## Denormal and finite-output observation

The benchmark runs a separate prepared `AudioEngine` probe with 256 subnormal input samples
(`std::numeric_limits<float>::denorm_min()` across 2 x 128 samples):

```text
input_subnormal_samples: 256
output_subnormal_samples: 256
output_nonfinite_samples: 0
denormal_probe_status: PASS
```

The probe records observed handling; it does not claim a platform-independent flush-to-zero mode.
Both benchmark scenarios also reported finite output, and the measured callback allocation
observer recorded zero `operator new` calls.

## Validation boundary

- Release fresh configure, safe build and CTest: **7/7 PASS**. The manual benchmark is separate.
- ASAN fresh configure, safe build and CTest: **7/7 PASS**.
- This evidence does not claim pluginval, real DAW, listening, offline render coverage beyond the
  existing RENDER-001 smoke, or a formal M1 Joint Exit.
- Dirty or unknown `source_state` is visibly reported by the executable and is not formal baseline
  evidence; only this clean fresh-configure run is recorded above.
