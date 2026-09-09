# M1 Engineering Evidence

- Tested code commit: `94f302ff63008f8f23008b2cb3daa1ea4476008e`
- Scope: TEST-002, PERF-BASE-001, ARCH-LAT-001, and bounded documentation/build wiring.
- Status: engineering evidence established; M1 Joint Exit is not claimed.

## Results

- `TEST-002`: `frazil_processor_property` passed 70 representative cases. The harness covers representative nominal values 32/64/128/256/512/1024 (not a public maximum), the full 44.1/48/96 kHz and mono/stereo matrix, actual callback sizes 0/1/7/31/1024 after nominal 1024 prepare, parameter extremes/routing/enable states, silence finite/DC/max-magnitude checks, impulse/noise/extreme finite input, standard release/reprepare and other lifecycle cases, finite-output checks, and M1 neutral/deterministic-path repeatability.
- `PERF-BASE-001`: Debug, Release, and ASAN reports passed with zero observed `operator new` calls during measured callbacks. The report field is `configuredCommit`, meaning Git HEAD captured during CMake configure; formal evidence requires fresh configure. The measured values and method are recorded in [PERF-BASE-001.md](PERF-BASE-001.md).
- `ARCH-LAT-001`: the real `FRAZILAudioProcessor` reported 0 samples latency in the neutral/dry fixture (`input.gain=0 dB`, `output.gain=0 dB`, `global.mix=0`); the canonical impulse peak remained at sample 12000 with maximum dry error 0. The zero-tail assertion is explicitly current M1 skeleton evidence, not a permanent Water/Ice tail contract.
- Debug, Release, and ASAN each passed all 8 CTest entries: smoke, unit, plugin integration, processor property, latency contract, performance baseline, render, and render CLI.
- Portability, Markdown-link, and VS Code task scanners plus their regression tests passed.

## Review boundary

The new targets are test executables only. No Water, Ice, Routing, UI, EditHistoryManager, parameter ID/range/default, or production DSP algorithm was added. The CMake source-set cleanup only centralizes existing source lists; it does not create a new production dependency boundary. The measured allocation observation covers the selected PluginProcessor callback path; it is not a substitute for a complete realtime audit or a future Water/Ice performance claim.

Current gaps are real Host/DAW validation, full render regression coverage, listening acceptance, pluginval on the current artifact, formal parameter freeze/compatibility evidence, and M1 Joint Exit review. `tools/bin/pluginval.exe` was not present on this machine, so current pluginval validation is `NOT RUN`; historical pluginval records remain historical only.
