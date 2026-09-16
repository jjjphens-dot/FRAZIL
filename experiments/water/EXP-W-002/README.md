# Water DSP research — EXP-W-002 bounded spike

Status: LOCAL-WDSP-00..06 implemented and engineering-validated locally in Debug/Release/ASAN; 80 diagnostic renders passed. **Research candidates only; no production WaterProcessor or sound acceptance.**

## Scope, authorization and execution state

The Engineering Lead requested the supplied [execution proposal](../EXP-W-002_DSP_IMPLEMENTATION_PLAN.md)
and explicitly authorized algorithm research on 2026-09-16 before accepted EXP-W-001, deferring
brief integration and listening to later work with Sound Lead. This session scope decision does
not rewrite canonical contracts, accept the draft brief, close EXP-W-002 or approve production.
Issue #17 remains the brief work item. Formal closure still needs the accepted brief and listening.

- Base: `origin/main` `3438593`; original unrelated worktree changes were preserved.
- Review branch: `codex/exp-w-002-research`, created/published at the user's request.
- Implementation owner: Engineering Lead/current agent. No delegated workers.
- Current checkpoint: LOCAL-WDSP-00..06 engineering checkpoint complete; formal perceptual acceptance deferred.
- Next action: independent review of the research branch, then joint brief/listening work before any justified refinement or production adoption.
- LOCAL-WDSP-07: deferred; no listening finding yet justifies pitch drift, interpolation upgrades,
  coupling compensation or additional synthesis complexity.
- Human boundary: EXP-W-001 reconciliation, Water identity, tonal recognizability, artifact
  acceptability and musical usefulness remain for Engineering + Sound Lead collaboration.

## Module map and output contract

All DSP lives here and is excluded from production plugin targets. State belongs to the calling
processing owner; prepare/reset/process must not execute concurrently. Controls are fixed by
prepare; no runtime parameter transport, automation smoothing or mode transitions are claimed.

| Module | Responsibility | State/reset/tail |
|---|---|---|
| `WaterDspConfig.h` | Sample-rate/seed values; stable A/B/D seed domains | No Host/state registration |
| `WaterExcitationFeatures.h` | Linked max(abs(L),abs(R)), fast/slow envelopes, positive difference | Reset zero; control magnitude capped at 1; source audio unchanged |
| `LiquidModalResonator.h` | Independent Resonant C; six fixed complex-pole modes | Separate stereo quadratures; reset zero; exponential tail, floor 1e-25 |
| `BubbleEnsemble.h` | A; input/envelope-gated stochastic events | Own PRNG and feature state; reset reseeds and clears pool |
| `DropletImpactExciter.h` | B; transient threshold, hysteresis and refractory gate | Own PRNG chooses frequency family; no autonomous event timing |
| `FlowModulator.h` | D; source-activity-scaled smooth random fractional delay | Prepare-only allocation; reset clears both channel buffers/index/trajectory; tail <=20 ms |
| `FluidCandidate.h` | A+B+D residual sum and fixed-config ablation | Each component has independent state; disabled components are not advanced |
| `detail/DampedResonator.h` | Cartesian complex pole update | No imaginary direct feedthrough from current real excitation |
| `detail/EventVoicePool.h` | Fixed 16-slot stereo impulse-excited pool | Inactive-first, otherwise oldest-age steal; lowest-index tie break; expiry at 24 decay constants |
| `ResearchBaseline.h` | Zero-residual infrastructure control | No audio state or tail |

Every sonic module returns **E**, not x+E. The renderer adds the source exactly once. Flow returns
`gain*(xd-x)`. Modal weights sum to its residual gain, and its `(1-r)` excitation bounds the
absolute impulse sum by that gain. Event voices receive bounded signed source impulses, not sample
playback or added noise; total pool weighting bounds residual amplitude by the configured gain.
There is no limiter, compressor, automatic makeup or hidden normalization after composition.
Event gain normalization is an explicit design bound, not perceptual loudness matching.

Stereo timing/control may be linked; audio and resonator/delay states remain isolated. Each RNG
is derived from the base seed with a stable module ID. Reset/reprepare restarts the same stream.
Repeatability is tested within the same build/platform; cross-compiler bit identity is not promised.
Feature detection is implemented once as a reusable type; A/B/D keep local instances to preserve
independent lifecycle and ablation. No speculative shared production primitive is added.

## Engineering configs (not product macros)

The checked-in [defaults](configs/defaults.json) reproduce the v0 candidate. Units are in field
names. Omitted JSON fields retain C++ defaults; unknown fields, nonnumeric values, fractional
voice counts and invalid ranges fail before rendering. `water.model/size/motion` are not accepted.

| Config | Default | Valid research range |
|---|---|---|
| Sample rate | 48000 Hz | 44100..96000; tested 44100/48000/96000 |
| Features | fast attack/release .001/.03 s; slow .03/.2 s | Each .0001..2 s in C++ config; renderer uses defaults |
| A frequency/decay | 250..2800 Hz / .07 s | 40 Hz..0.45*fs, ordered; .002..0.5 s |
| A rate/threshold/gain/voices | 120/s / .0001 / .2 / 16 | 0..2000/s / 0..1 / 0..0.3 / 1..16 |
| B frequency/decay | 600..4500 Hz / .012 s | 40 Hz..0.45*fs, ordered; .002..0.1 s |
| B threshold/refractory/gain/voices | .015 / .02 s / .15 / 8 | .0001..1 / .001..1 s / 0..0.3 / 1..16 |
| D base/depth | .004/.001 s | base-depth >=1 sample; base+depth <=.02 s; depth >=0 |
| D target interval/gain | .25 s / .1 | .02..10 s / 0..0.15 |
| C root/decay/gain | 260 Hz / .12 s / .18 | root >=40 Hz; root*4.17 <=.45*fs; .002..1 s / 0..0.3 |

C mode ratios 1/1.41/1.93/2.57/3.31/4.17 and A/B's 16-frequency log-spaced families are engineering
choices. Bubble radius/frequency direction is inspired by isolated-bubble acoustics; the code
uses frequency controls and does not claim a calibrated physical radius model. B v0 has deterministic
threshold timing; stochastic timing is optional in the proposal and is not implemented.

## Build, tests, render and measurement

From an initialized MSVC developer environment at repository root:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

Repeat serially with `windows-release` and `windows-asan`. The opt-in option defaults OFF. Existing
safe presets build research through `frazil_smoke` dependencies; no research code is linked into
FRAZIL. CTest includes baseline/features/modal/bubble/flow/droplet/fluid plus decoded renderer tests.
ASAN tests receive the compiler runtime path, and executables receive the runtime DLL. Hosted CI
explicitly enables this option; no Hosted CI result is claimed from the local runs.

```powershell
$renderer = 'build/windows-release/experiments/water/EXP-W-002/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
& $renderer testdata/input/zero_state_response__impulse.wav build/water-c.wav c 128 42 experiments/water/EXP-W-002/configs/defaults.json 3
python tools/analyze_testdata.py build/water-c.wav --json-out build/water-c.analysis.json
python experiments/water/EXP-W-002/analysis/render_corpus.py --renderer $renderer --output build/water-corpus
& 'build/windows-release/experiments/water/EXP-W-002/frazil_water_performance.exe'
```

Renderer arguments: input WAV, **new** output WAV, mode, block (1..8192), uint32 seed, optional
JSON path (or `-` for defaults), optional integer tail seconds (0..30). Modes: `a`, `b`, `d`, `ab`,
`ad`, `bd`, `abd`, `c`; append `-residual` for E only. `baseline` is pass-through PCM24; `residual`
is its zero residual. Sonic renders are float32 WAV, retaining peaks above 1 for analysis. Input
must be finite mono/stereo within full scale. Existing outputs are refused; failed renders are
not valid evidence and may leave a partial new file. Use a new ignored output directory each run.

The corpus script reuses all ten TESTDATA-001 fixtures and the existing analyzer: 80 processed
renders, default-config tail checks, finite metrics and A/B/D ablation error checks. It does not
replace licensed musical fixtures or loudness-matched listening. Float/PCM quantization is
accounted for in render comparisons; in-memory deterministic tests require exact equality.

The performance executable measures M1 alone and M1 plus each candidate/ablation on the same
48 kHz/128 stereo gated workload. It uses 2000 warmup/20000 measured blocks, steady-clock wall
callback duration, mean/nearest-rank P95/P99/worst and mean increment from the same-run baseline.
Input copying is outside timing; the periodic gate exercises repeated Droplet onsets after warmup.
This is preliminary research timing, not process CPU percent, formal provenance/budget or a
production AudioEngine integration claim. Production sources and baseline harness are unchanged.

## Checkpoints and validation

- LOCAL-WDSP-00: zero residual, carrier ownership, seed plumbing and baseline renderer; full Debug 9/9 PASS.
  First build was safely refused at 2.99 GiB available memory; after user memory recovery the unchanged wrapper passed.
- LOCAL-WDSP-01: analytic attack/release, stereo-linked control, reset/reprepare, long-silence convergence; PASS.
- LOCAL-WDSP-02: C impulse decay, finite extremes, no instantaneous carrier, deterministic lifecycle; PASS.
  Initial impulse render finite; provisional Debug M1+C mean 37.373 us versus M1 9.807 us.
- LOCAL-WDSP-03: A silence gating, source drive, capacity/stealing, repeatability, tail expiry; PASS; A gated-sine render generated.
- LOCAL-WDSP-04: D delay bounds, isolation, reset, finite extreme inputs; PASS; sweep/HF renders generated.
  Provisional Debug M1+D mean 24.725 us versus M1 9.216 us on the earlier steady workload.
- LOCAL-WDSP-05: B transient-correlated timing, signed isolated excitation, independent stream, reset/tail; PASS.
- LOCAL-WDSP-06: fixed-seed component ablation, zero-gain carrier, odd/empty callbacks, sample-rate/partition
  and decoded float rendering; Debug/Release/ASAN each 15/15 PASS; 80 corpus renders and preliminary Release timings recorded.

Engineering commands and results are recorded in [EVIDENCE.md](EVIDENCE.md). These checkpoints do not
assert perceptual Water identity or source recognizability. LOCAL-WDSP-07 has not started.

## Code quality, documentation and limitations

Code-path review: fixed bounded loops and pools, prepare-only Flow allocation/coefficient generation,
no processing I/O/locks/UI/APVTS, no mutable global state, explicit reset/seed/tail, no production
source edits. ASAN complements bounds tests; allocation/lock absence is a code-path review, not a
runtime allocation-instrumentation result. The complex-pole realization was chosen over the proposed
direct-form recurrence for bounded quadrature state and clear excitation normalization.

Known limitations: isolated bubbles omit coupling/geometry/pitch-rise; hard stealing can click;
linear interpolation can color high frequencies; fixed modal ratios may sound generic or metallic;
source-linked smooth random delay can still sound chorus-like. There is no evidence yet to justify
refinements, macro mappings or production adoption. The modal normalization may be too subtle on
some material; objective stability is not a Water-identity judgment. No claim of correct tonal
recognizability is made from the source-carrier arithmetic alone.

Documentation synchronization covers this README/proposal, experiment index, module index, testing
entry point, implementation guide, CI/Environment configuration and branch-local project status. Architecture, Parameters,
Coding Plan, Perceptual Contract, Code Standards and ADR-0003/0005/0006 are reviewed without changing
contracts. Same toolchain/JUCE/safety wrapper; no Host registry, schema, routing, latency reporting,
random persistence, performance budget, Ice or production UI changes. No pluginval/DAW/listening
validation is claimed for this standalone experiment.

## Primary sources and inference boundaries

- [Smith: pole radius and bandwidth](https://www.dsprelated.com/freebooks/filters/Relating_Pole_Radius_Bandwidth.html): exponential pole mapping informs decay stability; local normalization still requires tests.
- [Smith: delay-line interpolation](https://www.dsprelated.com/freebooks/pasp/Delay_Line_Signal_Interpolation.html): linear interpolation is inexpensive but has frequency-dependent error; it does not guarantee a Flow percept.
- [Pumphrey et al., DTU, 1989](https://orbit.dtu.dk/en/publications/underwater-sound-produced-by-individual-drop-impacts-and-rainfall/): impact emission and entrained-bubble ringing motivate separate A/B mechanisms.
- [van den Doel, UBC](https://www.cs.ubc.ca/labs/lci/lci-forum/03/vandendoel-040312.html): isolated bubble models and stochastic populations support the approximation strategy, not our exact event law.
- [RSC: acoustic interaction between cubic bubbles](https://pubs.rsc.org/en/content/articlehtml/2020/sm/c9sm02423a): discusses the inverse-radius Minnaert reference and deviations through interaction.
- [Xue et al., Stanford, 2023](https://graphics.stanford.edu/papers/coupledbubbles/): coupling affects low-frequency emissions; independent oscillators omit that behavior.

Sources were checked on 2026-09-16. FRAZIL's residual composition, Fluid/Resonant names, gains,
frequency families and scheduling are engineering hypotheses, not formulas endorsed by these papers.
