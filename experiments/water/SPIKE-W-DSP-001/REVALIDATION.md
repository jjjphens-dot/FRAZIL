# SPIKE-W-DSP-001 review revalidation

Date: 2026-09-16. Work item: [#29](https://github.com/jjjphens-dot/FRAZIL/issues/29).
Engineering feasibility only. No perceptual acceptance, formal EXP-W-002 closure or production adoption.

## Source, provenance and environment

- Repaired executable/config/test/analysis source: `6b3b8f8e590acfc1164fe8dfe7f6d10108a8f041`.
- Reviewed predecessor: `646a8bc9cfe951333ef2a4ca02a88dc3a6622761`; main base: `3438593`.
- Debug/Release builds initially compiled the identical source contents before the repaired-source
  commit; ASAN and all subsequent smoke/corpus/timing ran with that committed source. The final
  evidence commit changes Markdown only. The final submitted branch HEAD and exact-head Hosted CI
  run are recorded in the associated PR body, avoiding a self-referential commit ID in this file.
  Reviewers can verify `git diff 6b3b8f8..HEAD -- experiments ':!*.md'` has no executable differences.
- Windows 11 10.0.22631; Intel Core i9-14900HX; MSVC 19.43.34808; pinned JUCE 9.0.1;
  CMake 4.3.2; Python 3.12.4. No reference DAW: standalone offline/benchmark programs.
- Local safety wrapper unchanged, six jobs, one configure/build/test pipeline at a time.
  No generated WAV/log/plot or personal absolute path is committed. No new content hashes.
- Formal performance provenance: **NOT RUN**, as explicitly emitted by the research executable.
  This report identifies research source; it does not promote timings into PERF-BASE-001 evidence.

## Contract Review and Implementation

- P1-1: `SPIKE-W-DSP-001` is an optional objective feasibility work item before accepted EXP-W-001
  and M1 Joint Exit. Controlled plan/framework, Agent and Code Standards cross-references agree.
  The accepted-brief prerequisite for formal EXP-W-002 remains. Future experiments reuse/revise
  the spike rather than duplicate it. The prior broad execution proposal was consolidated into
  the bounded spike plan; Git history retains the original proposal.
- P1-2: Fluid resets previous state and prepares only enabled components; the renderer prepares
  only baseline, C, or the selected Fluid subset. Disabled semantic errors do not block other
  mechanisms. Structural JSON/type/integer representation errors remain global; active DSP
  retains semantic range validation. Tests include A-only, D-only, C-only, baseline/residual,
  active-invalid, unknown fields, invalid integer representation and recovery.
- P2-1: EventVoicePool owns the 1..16 capacity invariant. Unprepared/failed pools are inactive;
  zero, 17 and SIZE_MAX fail without clamping. Trigger/process/reset/activeVoices remain safe;
  valid 1/16 preparation recovers. Bubble and Droplet propagate failure.
- P2-2: PROJECT_STATUS describes capability and outstanding acceptance, not branch topology.
- P2-3: Smith references now identify Stanford CCRMA/original books; provenance is supported by
  Smith's DAFx keynote references and Stanford's book notice. Automated access restrictions are
  disclosed. No DSP design changed in response to citation corrections.
- Regression-discovered parser defect: JUCE wraps an oversized integer literal such as 2^64.
  The offline reader now rejects literals outside signed int64 before JUCE parsing and guards
  floating-point-to-size_t conversion. This is a representation fix, not a DSP range relaxation.

## Functional and Final Validation

From an initialized MSVC developer shell, serially for each preset:

```powershell
chcp 65001
cmake --fresh --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
```

| Preset | Build | Full CTest | Sanitizer |
|---|---|---|---|
| windows-debug | PASS | 16/16 PASS | N/A |
| windows-release | PASS | 16/16 PASS | N/A |
| windows-asan | PASS | 16/16 PASS | No ASAN finding |

The suites include existing production regressions and baseline/features/modal/bubble/flow/droplet/
Fluid/event-pool/decoded-renderer checks. Rates: 44.1/48/96 kHz. Partition coverage includes
1/7/32/64/128/256/512/1024 and odd final spans. JSON isolation, fixed-seed reset/reprepare,
ablation, finite extremes, mono/stereo and zero-gain carrier ownership passed.

Validation history is not suppressed: the new oversized-integer regression first failed. A stale
local CMake dependency cache then missed the header edit: localized `/showIncludes` had zero parsed
header dependencies. UTF-8 code page + fresh configure/rebuild corrected this; the renderer object
records 406 dependencies including ReadConfig, FluidCandidate and EventVoicePool. One subsequent
Debug Python CLI test process terminated with a reported SegFault, without diagnostic output.
The standalone `python -X faulthandler .../tests/render_cli_test.py <debug-renderer>` rerun and the
complete Debug rerun passed; Release and ASAN also passed. That isolated runner termination was
not reproduced and its cause remains unconfirmed; no DSP fault is inferred or hidden.

Additional checks: research C++ `clang-format --dry-run --Werror`, Python `py_compile`,
`python tools/check_portability.py`, `python tools/check_markdown_links.py`, and `git diff --check`
PASS. Production `src/`, existing production tests, Host registry and state have no changes.

Hosted CI: **pending at evidence-writing time**. The associated final PR records the actual final
HEAD and completed run URL/result; do not infer Hosted CI success from these local results.

## Typical-signal smoke

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
python experiments/water/SPIKE-W-DSP-001/analysis/review_smoke.py --renderer $renderer --output build/water-review-smoke-6b3b8f8
```

**32 signal/mode cases; 138 renders PASS**: each case has processed blocks 7/128/1024 and residual
block 128, plus ten entirely-unexcited-channel probes derived from the canonical stereo fixture.
Defaults, seed 42, 48 kHz, three appended seconds of silence. Decoded samples match exactly across
partitions. Max carrier error `2.9802322387695312e-08`; max final-100-ms residual peak `3.15009e-15`.
Every output is finite; no Bubble/Droplet event was scheduled on any exactly zero source frame.
The opposite channel of both one-channel-only probes remains exactly zero in A/B/D/ABD/C.

All silence cases have output/residual peak, RMS, DC, event counts and tail equal to zero.
The table uses complete-render metrics (including appended silence); DC includes source DC.
A/B counts identify actual events, not a subjective activity score.

| Signal | Mode | Peak | RMS | DC | Residual RMS | A/B events |
|---|---|---:|---:|---:|---:|---:|
| silence | A | 0 | 0 | 0 | 0 | 0/0 |
| silence | B | 0 | 0 | 0 | 0 | 0/0 |
| silence | D | 0 | 0 | 0 | 0 | 0/0 |
| silence | ABD | 0 | 0 | 0 | 0 | 0/0 |
| silence | C | 0 | 0 | 0 | 0 | 0/0 |
| impulse | C | 0.501187 | 0.00086463 | 1.49235e-06 | 4.18683e-07 | 0/0 |
| impulse | A | 0.501187 | 0.00086463 | 1.49163e-06 | 0 | 0/0 |
| impulse | B | 0.501187 | 0.00086463 | 1.49163e-06 | 0 | 0/0 |
| impulse | ABD | 0.451068 | 0.00078283 | 1.49163e-06 | 0.00012147 | 0/0 |
| gated | A | 0.520501 | 0.112979 | 4.82099e-07 | 0.00149349 | 30/0 |
| gated | B | 0.503602 | 0.112954 | 7.61239e-08 | 9.03067e-05 | 0/1 |
| gated | D | 0.457294 | 0.098645 | -2.00119e-08 | 0.0183409 | 0/0 |
| gated | ABD | 0.467165 | 0.0986734 | 5.38208e-07 | 0.0183945 | 30/1 |
| transient | B | 0.493093 | 0.0433456 | 0.000128025 | 0.000114354 | 0/4 |
| transient | A | 0.49535 | 0.043348 | 0.000127633 | 0.000230924 | 21/0 |
| transient | ABD | 0.443678 | 0.0372253 | 0.000127692 | 0.00740457 | 21/4 |
| transient | C | 0.495621 | 0.0433683 | 0.000127977 | 2.51612e-05 | 0/0 |
| noise | A | 0.221138 | 0.0795851 | -0.000506274 | 0.000634014 | 50/0 |
| noise | D | 0.216948 | 0.0719352 | -0.000505576 | 0.0102573 | 0/0 |
| noise | ABD | 0.218968 | 0.0719377 | -0.000506348 | 0.0102792 | 50/1 |
| noise | C | 0.21815 | 0.079583 | -0.000505806 | 3.78164e-05 | 0/0 |
| sweep | D | 0.125891 | 0.0625592 | 0.000100961 | 0.0094602 | 0/0 |
| sweep | C | 0.126327 | 0.0688921 | 0.00010101 | 9.15036e-05 | 0/0 |
| sweep | ABD | 0.127272 | 0.0625661 | 0.000101728 | 0.00947065 | 76/27 |
| HF sine | D | 0.125641 | 0.0519624 | 1.64479e-10 | 0.00654359 | 0/0 |
| HF sine | ABD | 0.127164 | 0.0519642 | -4.36718e-07 | 0.00655777 | 38/1 |
| HF sine | C | 0.125645 | 0.0563008 | 2.44906e-12 | 1.38963e-07 | 0/0 |
| stereo | A | 0.256731 | 0.0585656 | -3.60378e-07 | 0.00054002 | 30/0 |
| stereo | B | 0.254949 | 0.0585589 | 4.79172e-08 | 9.4917e-05 | 0/2 |
| stereo | D | 0.229101 | 0.0525869 | -3.39452e-09 | 0.00858677 | 0/0 |
| stereo | ABD | 0.233539 | 0.0525945 | -3.15852e-07 | 0.00860184 | 30/2 |
| stereo | C | 0.251337 | 0.0585719 | 5.72447e-12 | 1.8497e-05 | 0/0 |

### Engineering observations and listening risks

PASS below means behavior consistent with the current engineering mechanism, not “sounds good”
or “Water identity accepted.” None is a quality ranking.

| Module | Signals checked | Objective behavior | Potential listening risk / deferred finding | Engineering result |
|---|---|---|---|---|
| A Bubble | silence, impulse, gated, transient, noise, stereo | Silence creates no events; gated has 25 high-section + 5 low-section events; transient 21; noise 50; independent RNG/reset | No event on one-sample impulse at seed 42; isolated bubbles omit coupling/pitch rise; hard stealing may click; thin/high-frequency impression untested | PASS |
| B Droplet | silence, impulse, gated, transient, stereo | Gated high onset triggers at frame 9607 (0.146 ms after its start); low gate does not trigger; four transient events, first at frame 4837 (0.771 ms after start); four localized decays visible; no zero-source events | Threshold sensitivity: misses low gated segment and one-sample impulse; event-like timbre untested | PASS; sensitivity observation retained |
| D Flow | gated, noise, sweep, HF sine, stereo | Bounded delayed-minus-source residual; silence clears; HF total RMS -0.697 dB relative to carrier, sweep -0.836 dB; source frequency trajectory retained | F-D-01: visible frequency coloration/PSD ripple and low-level off-main-track sweep features versus dry control; audibility and alias attribution unmeasured; chorus/flanger risk retained | PASS; spectral finding for later isolated review |
| C Resonant | silence, impulse, transient, noise, sweep, HF, stereo | Decaying impulse residual; low-frequency resonance distribution in noise/sweep; finite and isolated; no duplicated carrier | Default impulse residual peak only about 1.25e-5; subtle/generic/metallic character possible; no physical-water mode claim | PASS |
| ABD Fluid | silence, impulse, gated, transient, noise, sweep, HF, stereo | A/B counts match isolated components; exact partition repeatability; residual composition agrees within rounding; no autonomous output | Relative balance and source recognizability need musical listening; Flow spectral observation remains | PASS |

C impulse residual RMS in 50 ms windows at 0, 0.2, 0.5 and 1.0 s after excitation:
`3.72970e-6`, `6.91864e-7`, `5.73418e-8`, `8.86181e-10`. C processed = source + C residual within
float tolerance (the measured impulse case is exact). A/B no-event impulse response is consistent
with the existing source-envelope/transient threshold mechanism; it is not proof of useful transient response.

Noise C residual PSD maximum is 1089.84 Hz and sweep C is 257.812 Hz at the analyzer's frequency
resolution, near the fixed family endpoints 1084.2/260 Hz. These are synthesis choices, not measured
water resonances. HF source is 10560 Hz; its main spectral peak remains dominant. Dry-versus-D
plots show low-level ripple/sideband-like content and frequency-dependent coloration; visual
spectrogram scales are not calibrated alias rejection measurements. No interpolation upgrade,
compensation, drift or new excitation was introduced.

Two appended-silence baseline controls (HF/sweep) and two first-gate prefix probes were generated
separately in `build/water-review-spectrum-controls-6b3b8f8/`. Reproduce the baseline with the
renderer `baseline 128 42 - 3`; reuse `analyze_audio`/`write_plots` from `tools/analyze_testdata.py`.
For the prefix, copy the canonical gated sine's first 33600 frames at 48 kHz to an ignored PCM24
file, then render A/B with `128 42 - 3`. This retains initial detector/RNG history and isolates the
first gate; full-minus-prefix gives the second-section A count. No new analyzer is introduced.

## Full corpus

```powershell
python experiments/water/SPIKE-W-DSP-001/analysis/render_corpus.py --renderer $renderer --output build/water-review-corpus-6b3b8f8
```

Ten TESTDATA-001 signals x A/B/D/AB/AD/BD/ABD/C = **80 PASS**. Seed 42, defaults, block 128,
three-second tail. Finite/default-tail/silence and combination residual consistency <2e-7 passed.
The complementary smoke/CLI checks establish carrier ownership, fixed-seed partitions and actual
unexcited-channel isolation; canonical stereo alternates excitation, so previous-channel tails
are intentional. The following maxima are observations, not universal acceptance thresholds.

| Mode | Maximum peak | Maximum absolute DC | Largest final-100-ms tail peak |
|---|---:|---:|---:|
| A | 0.520501 | 0.000506274 | 0 |
| B | 0.505593 | 0.000505622 | 0 |
| D | 0.501183 | 0.000505576 | 0 |
| AB | 0.520501 | 0.000506333 | 0 |
| AD | 0.508509 | 0.000506288 | 0 |
| BD | 0.504196 | 0.000505636 | 0 |
| ABD | 0.508509 | 0.000506348 | 0 |
| C | 0.501483 | 0.000505806 | 3.15009e-15 |

## Preliminary same-run Release timing

```powershell
& 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_performance.exe'
```

Raw output: ignored `build/water-review-performance-6b3b8f8.txt`. One process/thread, 48 kHz,
128 samples, stereo, base seed 20260916, identical gated workload, 2000 warmup + 20000 measured
blocks per case; input copying outside timing, steady-clock wall durations, nearest-rank percentiles.
A/B/AB/AD/BD/ABD labels in raw output include the M1 engine and are expanded below for clarity.
Callback deadline is 2666.67 us; no formal utilization threshold is imposed.

| Case | Mean us | P95 us | P99 us | Worst observed us | Mean increment over same-run M1 us |
|---|---:|---:|---:|---:|---:|
| M1 | 1.136 | 2.000 | 7.300 | 40.000 | 0.000 |
| M1+C | 2.619 | 3.000 | 5.600 | 150.500 | 1.483 |
| M1+D | 4.129 | 4.300 | 5.200 | 130.800 | 2.993 |
| M1+A | 3.103 | 3.900 | 4.200 | 84.900 | 1.968 |
| M1+B | 2.626 | 3.000 | 4.700 | 221.800 | 1.490 |
| M1+AB | 5.013 | 5.900 | 10.400 | 94.700 | 3.877 |
| M1+AD | 7.136 | 8.000 | 10.900 | 304.700 | 6.001 |
| M1+BD | 6.782 | 7.300 | 9.200 | 238.400 | 5.646 |
| M1+ABD | 8.618 | 9.600 | 12.600 | 155.800 | 7.483 |

Preliminary timing only; formal provenance, process CPU percent, memory/allocation instrumentation,
confidence intervals, stress maxima and production performance budget are NOT RUN/not claimed.
Worst observed includes scheduling noise and is not a worst-case callback guarantee. Compare
only with the same-run M1 row; the historical report measured another source/environment moment.

## Code Quality Review

Separate post-functional review checked cohesion/coupling, ownership/lifetime, naming/scopes,
constants/macros/includes, mutable globals, dead code and realtime call paths. DSP processing
retains fixed loops/pools, local state and RNG domains; Flow allocation/coefficient generation
remains prepare-only. There is no process-path I/O, blocking, UI/APVTS or hidden initialization.
Renderer diagnostic counters and JSON parsing are offline-only, outside DSP and timed callbacks.
This is code-path evidence plus bounds/ASAN tests, not runtime allocation/lock instrumentation.
No algorithm redesign or speculative LOCAL-WDSP-07 work was introduced.

## Comment & Documentation Pass / consistency

Changed: AGENTS and Code Standards scope cross-references; Coding Plan and Perceptual Contract
optional work-item lifecycle; experiment index, bounded implementation plan, module README and
historical/current evidence; PROJECT_STATUS capability wording; MODULE_INDEX paths/status;
CORE_IMPLEMENTATION_GUIDE scope; TESTING isolation/capacity/smoke gates; ENVIRONMENT research
paths and the observed local dependency-encoding diagnostic. Comments describe capacity failure,
disabled-state reset, parser representation and offline diagnostics.

Reviewed, no update required:

- Architecture: product dual-mode direction and production module boundaries are unchanged.
- PARAMETERS / ADR-0002: nine Host parameters and schemaVersion=1 unchanged; no macro/state adoption.
- ADR-0001 / ADR-0003 / ADR-0005: routing, realtime boundaries and Host latency unchanged.
- Proposed ADR-0006: remains Proposed; no algorithm adoption or acceptance.
- DOCUMENT_GOVERNANCE / GITHUB_WORKFLOW: existing controlled issue/review, Full Gate and identity
  rules are applied, not changed. Issue #29 supplies the controlled scope and future handoff.
- COLLABORATION_ROLES / DEVELOPER_SOUND_TOOLS: Engineering implements, Sound/Host independently
  reviews; no production DRI transfer or Developer UI/Host/offline boundary change.
- Production module READMEs: no production source/API/dependency change. Root README build
  workflow and existing safety wrapper/presets remain applicable.

Consistency: plan/framework/Agent rules all distinguish objective spike from accepted-brief formal
EXP-W-002; module index/README/CMake/source agree on paths and isolation; TESTING matches current
results; PROJECT_STATUS makes no production/M2/listening claim. No obsolete executable path remains.
All necessary synchronization is in this single PR. Human review remains pending, not self-approved.

## Unexecuted / deferred acceptance

Human listening and loudness-matched musical acceptance: **NOT RUN**. Accepted EXP-W-001
reconciliation, subjective tuning/selection, macro mapping and Sound Lead judgments remain future
work. Pluginval/DAW, production WaterProcessor/AudioEngine Water path, routing/Ice, Host/state
changes, production transitions, formal CPU budget and release compatibility are not this spike.
WaterProcessor is still not production-implemented. EXP-W-002 is not formally closed; ADR-W-001
is not Accepted. Stop at independent Engineering + Sound/Host PR review; no automatic merge.
