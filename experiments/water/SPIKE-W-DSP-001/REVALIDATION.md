# SPIKE-W-DSP-001 review revalidation

Date: 2026-09-16. Work item: [#29](https://github.com/jjjphens-dot/FRAZIL/issues/29).
Engineering feasibility only. No perceptual acceptance, formal EXP-W-002 closure or production adoption.

The sections below preserve pre-review evidence through `d3bae57`. The subsequent independent-review
JSON parser repair and its fresh executable validation are recorded in the final section. Older
measurements keep their original source attribution; they are not silently relabelled as new runs.

## Pre-review source, provenance and environment (through d3bae57)

- Repaired executable/config/C++ test source: `6b3b8f8e590acfc1164fe8dfe7f6d10108a8f041`.
- Initial supplemental smoke automation source: `f9a0e6ce69452b21d4ff23cb739f187a6f94e562`;
  the subsequent `e842e50` changed README/REVALIDATION only. The final terminology cleanup changes
  only the smoke field name, adds plot-existence checks and synchronizes these two documents.
  Its automation source is `d3bae57916cc62dfa12564651d052b19fbb999e5`. That smoke rerun
  used identical Python contents immediately before that cleanup commit; the calculation is unchanged.
- Reviewed predecessor: `646a8bc9cfe951333ef2a4ca02a88dc3a6622761`; main base: `3438593`.
- Debug/Release builds initially compiled the identical source contents before the repaired-source
  commit; ASAN and the original smoke/corpus/timing ran at that committed source. The final smoke
  rerun uses its unchanged Release renderer with the updated Python automation identified above.
  Final submitted HEAD and exact-head Hosted CI are recorded in the associated PR body, avoiding
  a self-referential commit ID in this file.
  Reviewers can verify `git diff 6b3b8f8..d3bae57 -- experiments ':!*.md' ':!*/analysis/review_smoke.py'`
  has no compiled-executable/config differences. Local three-preset, 80-render corpus and timing
  evidence below remains attached to that unchanged DSP source; it was not rerun for Python-only work.
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

Hosted CI is a separate evidence class: repository portability/policy/tool checks, TESTDATA-001
verification, research-enabled configure/safe build and Windows Debug CTest. It does **not** run
the local Release/ASAN presets, Water smoke, 80-render Water corpus or research timing.
The prior exact-head [run 35062526836](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35062526836)
passed 16/16 CTest on `daea4cb`; that is historical CI, not final-head validation for this change.
PR #30 records the actual final HEAD and its completed Hosted run URL/result. No CI workflow changed.

## Typical-signal smoke

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
python experiments/water/SPIKE-W-DSP-001/analysis/review_smoke.py --renderer $renderer --output build/water-review-smoke-final
```

**32 signal/mode cases; 138 renders PASS**: each case has processed blocks 7/128/1024 and residual
block 128, plus ten entirely-unexcited-channel probes derived from the canonical stereo fixture.
The final terminology-cleanup rerun includes **four supplemental controls, 142 total PASS** using the unchanged
Release renderer from `6b3b8f8`. Its complete 32-record `observations.json` exactly matches the
prior `build/water-review-smoke-final-controls/observations.json` (and the original `6b3b8f8` run);
the table below remains applicable.
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
| D Flow | gated, noise, sweep, HF sine, stereo | Bounded delayed-minus-source residual; silence clears; processed-vs-dry RMS level delta: HF -0.697 dB, sweep -0.836 dB; source frequency trajectory retained | F-D-01: visible frequency coloration/PSD ripple and low-level off-main-track sweep features versus dry control; audibility and alias attribution unmeasured; chorus/flanger risk retained | PASS; spectral finding for later isolated review |
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

The same smoke command now generates two appended-silence baselines (HF/sweep) and two first-gate
prefix probes automatically. It reads the canonical manifest's high gate end (33600 frames at
48 kHz), copies from frame zero to retain detector/RNG history, and checks prefix PCM24 identity
and processed-prefix equality with the full render. It verifies silence outside the two gates and
zero-source event counts before subtracting prefix events from full events. Results: A high/low
**25/5**, B **1/0**. Counts are observations, not perceptual thresholds.

`build/water-review-smoke-final/supplemental_controls.json` stores the counts, defaults,
seed 42, block 128, three-second tail, baseline metrics, Flow processed/residual metrics and plot
paths. Comparable dry/Flow durations and rates, PCM24 carrier tolerance and zero dry tail are checked.
Using the existing `analyze_audio`/`write_plots`, full-render processed-vs-dry RMS level delta is **-0.696507 dB**
for HF and **-0.835653 dB** for the sweep. HF dry and Flow FFT peaks both remain **10560 Hz**;
their Welch maxima are both **10558.59375 Hz**. A single global sweep peak is not trajectory proof;
the spectrogram supplies that inspection. All 18 dry/processed/residual comparison PNGs exist,
now checked by the smoke script without image-content analysis.
Visual review reproduces F-D-01: HF PSD ripple and faint sweep off-main-track features. Automatic
plot scales do not establish calibrated alias rejection or audibility. No new analyzer or DSP was added.
The earlier manual controls remain historical in `build/water-review-spectrum-controls-6b3b8f8/`;
manual WAV construction is no longer needed for these observations.

The current JSON names this metric `processed_vs_dry_rms_delta_db` and computes
`20 * log10(processed_rms / baseline.rms)`. Here `processed_rms` is RMS(y), `baseline.rms` is
RMS(x), and the separate `residual.rms` is RMS(E), E = y - x. The first two and residual RMS use
linear amplitude; the level delta uses dB. This terminology correction does not change the
measurement. Comparison with the prior supplemental report confirms identical numeric results
and gate counts after accounting for the renamed field. Historical local outputs remain unchanged.

Final automation validation: `python -m py_compile experiments/water/SPIKE-W-DSP-001/analysis/review_smoke.py`,
the smoke command above, JSON/result/plot consistency checks, `python tools/check_portability.py`,
`python tools/check_markdown_links.py` and `git diff --check` PASS. No new local C++ build, full-corpus
or timing run was needed: DSP, renderer, configs, C++ tests and build/CI wiring did not change.

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

Final supplemental-control follow-up uses a targeted Documentation Impact Check: only the smoke
script, its README and this evidence report change. Code Quality Review checked the separate offline
helper, reuse of the existing analyzer, manifest-derived bounds, prefix history, comparable durations,
generated-file isolation and unchanged core assertions. Comment & Documentation Pass explains those
invariants and separates local versus Hosted evidence. TESTING, bounded implementation plan,
MODULE_INDEX, PROJECT_STATUS, Coding Plan and Perceptual Contract were reviewed without further
edits: testing contracts, module/status claims and accepted-brief lifecycle remain accurate. Production
contracts/ADRs and historical EVIDENCE.md are unchanged. Commands, counts, paths and provenance agree.
PR #30 requests `jjjphens-dot` to independently review objective scope, disclosed risks, reproducibility,
absence of production adoption and the accepted EXP-W-001 prerequisite for future EXP-W-002.
The live request/decision status belongs in the PR; self-validation is not independent acceptance.
The terminology cleanup follows the same six phases and targeted scope. Its separate Code Quality
Review confirms the ratio expression, render settings and core assertions are unchanged; the optional
plot assertions check file existence only. Comment & Documentation Pass aligns the JSON field/formula,
README metric definitions, this report and PR text. No testing contract, milestone claim or DSP changes.

## Unexecuted / deferred acceptance

Human listening and loudness-matched musical acceptance: **NOT RUN**. Accepted EXP-W-001
reconciliation, subjective tuning/selection, macro mapping and Sound Lead judgments remain future
work. Pluginval/DAW, production WaterProcessor/AudioEngine Water path, routing/Ice, Host/state
changes, production transitions, formal CPU budget and release compatibility are not this spike.
WaterProcessor is still not production-implemented. EXP-W-002 is not formally closed; ADR-W-001
is not Accepted. Stop at independent Engineering + Sound/Host PR review; no automatic merge.

## Independent-review JSON parser repair (current)

Finding: [P2 / discussion 4023469841](https://github.com/jjjphens-dot/FRAZIL/pull/30#discussion_r4023469841),
reviewed at `d3bae57`. JUCE accepted concatenated/trailing input and non-JSON number/escape forms,
contradicting the global strict-config contract. The new raw-text regression first failed against
the old Debug renderer on a concatenated document in baseline mode (exit 0 and output created).
Separate probes also reproduced duplicate-key overwrite hiding unknown/nonfinite fields.

Repair source: **`06443b474c3e4c9c8d1fe0317718e737f96e0373`**. Debug/Release/ASAN tested identical
source contents before that commit; smoke, corpus and timing below ran after it. The subsequent
evidence commit changes this Markdown report only; final submitted HEAD/Hosted CI are in PR #30.
The offline `ReadConfig.h` executable path and CLI regression changed. DSP algorithms, defaults,
renderer processing loop, benchmark implementation, production source, Host/state/routing/Ice
and CMake/CI wiring did not change. Do not call this a Python/docs-only follow-up.

### Contract Review / Implementation / Code Quality Review

- A bounded two-level numeric-config syntax gate checks [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259)
  object/string/number grammar and complete input before JUCE decodes values. No new dependency,
  general JSON framework, unbounded recursion, realtime parsing or DSP refinement is introduced.
- Raw UTF-8 file bytes are validated before string conversion (initial UTF-8 BOM tolerated), so NUL
  or invalid encoding cannot truncate/repair the input silently. Unknown keys, finite numeric types,
  int64 integer-literal and size_t voice representation checks remain global. Only enabled DSP
  enforces semantic ranges or prepares.
- Compare lexical versus decoded member counts in this fixed two-level schema to reject duplicate
  decoded keys, including escaped aliases. This prevents JUCE overwrite from hiding fields before
  global representation/unknown-field checks. No duplicate-key precedence is silently selected.
- Independent post-functional code review checked cursor progress/bounds, at-most-two-level stack
  depth, borrowed-buffer lifetime, exact byte-length conversion, integer overflow, key decoding,
  failure before output creation, globals/includes/naming and unchanged processing ownership.

### Functional and final validation

Serial commands from the initialized MSVC shell, for each preset:

```powershell
chcp 65001
cmake --fresh --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset <preset> --jobs 4
ctest --preset <preset> --output-on-failure
```

The final small header correction used safe incremental Debug/Release rebuilds after their fresh
configure; ASAN was freshly configured afterward. Local safety checks passed; four jobs were chosen
for available memory, without bypassing the wrapper. Compiler: MSVC 19.43.34809.0; Python 3.12.4.

| Preset | Safe build | Final full CTest | Time | Sanitizer |
|---|---|---|---:|---|
| windows-debug | PASS | 16/16 PASS | 18.92 s | N/A |
| windows-release | PASS | 16/16 PASS | 9.37 s | N/A |
| windows-asan | PASS | 16/16 PASS | 38.42 s | No ASAN finding |

Within the existing renderer CLI CTest: **33 rejected raw inputs x 6 modes = 198 rejection probes**,
each requiring exit 2, an invalid-config diagnostic and no output creation. Modes: baseline,
residual, A, C, D, ABD. Cases cover the five review examples, lexical/container/control/encoding
edges, duplicate keys/escaped aliases, oversized integers and nonfinite values. Four accepted raw
configurations cover whitespace, exponent forms, escaped known keys and UTF-8 BOM; their zero-gain
Flow residual is verified by decoded samples. Existing active-invalid/unused-invalid tests still pass.

One final Debug CTest attempt exited `0xc0000409` in the Python CLI runner without test output.
Windows Application error identified `python.exe` / `python312.dll`, not a renderer crash report.
The same full CLI test with `python -X faulthandler -u` passed; complete Debug/Release/ASAN runs with
`PYTHONFAULTHANDLER=1` then passed. Root cause remains unconfirmed. The failed attempt and successful
diagnostic runs are retained in ignored `build/water-strict-json-*.log`; this does not erase the
earlier pre-review Python runner incident or claim it cannot recur.

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
python -X faulthandler experiments/water/SPIKE-W-DSP-001/analysis/review_smoke.py --renderer $renderer --output build/water-json-review-smoke
python -X faulthandler experiments/water/SPIKE-W-DSP-001/analysis/render_corpus.py --renderer $renderer --output build/water-json-review-corpus
& 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_performance.exe'
```

**142 smoke renders PASS; 80 corpus renders PASS.** The smoke's 32 observations and supplemental
JSON exactly equal the `d3bae57` results, including A high/low 25/5, B 1/0 and the processed-vs-dry
RMS level deltas. Decoded samples/rates for all 142 renders + 3 derived inputs exactly match the
prior smoke. All 80 corpus decoded renders and metric rows match the `6b3b8f8` corpus. Thus the
earlier numerical tables remain applicable to valid defaults; invalid-input acceptance changed.
No content hashes were calculated. Existing plots/metrics remain objective, not audibility evidence.

Fresh same-run Release timing, identical 48 kHz/128 stereo, seed 20260916, 2000 warmup/20000 measured
blocks, nearest-rank wall-duration method; raw output: `build/water-json-review-performance.txt`:

| Case | Mean us | P95 us | P99 us | Worst observed us | Mean increment over same-run M1 us |
|---|---:|---:|---:|---:|---:|
| M1 | 0.558245 | 0.6 | 0.6 | 11.5 | 0 |
| M1+C | 2.35619 | 2.7 | 2.9 | 93.8 | 1.79794 |
| M1+D | 3.98146 | 4.1 | 5.2 | 101.7 | 3.42321 |
| M1+A | 2.93094 | 3.6 | 3.9 | 74.7 | 2.37269 |
| M1+B | 2.4315 | 2.8 | 3.2 | 156.1 | 1.87325 |
| M1+AB | 4.28841 | 5.2 | 9.2 | 142.3 | 3.73016 |
| M1+AD | 6.59437 | 7.3 | 9.8 | 241.0 | 6.03613 |
| M1+BD | 6.24103 | 6.6 | 8.1 | 105.3 | 5.68278 |
| M1+ABD | 7.8923 | 8.7 | 11.0 | 264.6 | 7.33405 |

Formal performance provenance remains **NOT RUN**. The parser is outside the timed callback;
differences from the historical table are not evidence of a DSP optimization or a CPU-budget pass.

### Comment & Documentation Pass / final handoff

Changed: README config syntax/encoding/duplicate-key behavior, TESTING raw-input rejection coverage,
this source-separated evidence, and comments explaining the bounded syntax gate/member-count check.
Reviewed without edits: implementation plan, MODULE_INDEX, PROJECT_STATUS, Coding Plan, Perceptual
Contract, production contracts/ADRs, DOCUMENT_GOVERNANCE and GITHUB_WORKFLOW. The strict-global,
active-only-semantic contract is repaired rather than relaxed; no module/milestone/production claim
changes. Historical EVIDENCE.md is preserved. Cross-document terminology, paths, scope and evidence agree.

`py_compile`, `clang-format --dry-run --Werror`, `check_portability.py`, `check_markdown_links.py`
and `git diff --check` PASS. Hosted CI remains separate from these local runs; PR #30 records its
exact final source checkout and CTest result. The independent REQUEST_CHANGES decision requires
reviewer revalidation; author self-checks do not resolve/approve the review. No merge performed.
Human listening, pluginval/DAW, formal allocation instrumentation and production adoption remain NOT RUN.

## Historical implementation scope and checkpoints

Consolidated from SPIKE-W-DSP-001_IMPLEMENTATION_PLAN.md. The following is the closed
Issue29 scope and then-current review plan, not an active plan for A1/Protect.
Later bounded follow-ups have their own explicit authorization and records.

## Authority and purpose

Stable work item: [Issue #29](https://github.com/jjjphens-dot/FRAZIL/issues/29).
Canonical scope: [Coding Plan](../../../docs/CODING_PLAN.md) and
[Perceptual Contract section 6](../../../docs/PERCEPTUAL_CONTRACT.md#6-optional-objective-feasibility-before-the-water-instance).
This historical implementation plan explained that scope; it cannot override controlled contracts.
It replaces the former EXP-W-002-labelled execution proposal; Git history retains that proposal.
The current task is review remediation, not a new algorithm round.

Implementation DRI: Engineering Lead. Acceptance DRI: Sound & Host Lead for independent scope
and evidence review; Engineering Lead supplies reproducible engineering checks. No DRI transfer.
Allowed paths: the standalone spike, research CMake/test wiring and directly affected docs.
Forbidden: production src, Host registry/state, routing, Ice, UI integration and vendor changes.

## Lifecycle and non-goals

The optional spike may precede accepted EXP-W-001 and M1 Joint Exit. It only establishes numerical,
realtime, deterministic, residual/carrier, finite/reset/tail/state, RNG isolation, sample-rate/block,
ablation, offline rendering and preliminary performance feasibility.
Formal EXP-W-001 -> accepted brief -> EXP-W-002 -> EXP-W-003 -> ADR-W-001 -> production remains.
EXP-W-002 must consume/revise the spike under accepted positive/negative/preserve/reject conditions,
not duplicate it. No subjective tuning/selection, macro mapping, Fluid/Resonant quality ranking,
Water acceptance, EXP-W-002 closure, ADR acceptance or production adoption is authorized here.

## Existing mechanisms and bounded checkpoints

| Checkpoint | Mechanism / engineering question |
|---|---|
| LOCAL-WDSP-00 | Zero residual baseline; carrier added once; seed/config/render plumbing |
| LOCAL-WDSP-01 | Linked fast/slow envelopes; input-driven excitation and reset |
| LOCAL-WDSP-02 | C: six fixed normalized complex-pole resonators; independent stereo state |
| LOCAL-WDSP-03 | A: source/envelope-gated stochastic event pool; bounded signed excitation |
| LOCAL-WDSP-04 | D: smooth random fractional delay; E = gain * (delayed - source) |
| LOCAL-WDSP-05 | B: hysteretic transient/refractory events with independent frequency RNG |
| LOCAL-WDSP-06 | Fluid A+B+D; independent ablation, deterministic partitions and render evidence |

All components return residual; the offline caller owns the source carrier. Each A/B/D mechanism
has a stable RNG domain and independent detector/state. Controls are fixed at prepare; no live
transition or production smoothing claim. Exact ranges/defaults and references live in the
[module README](README.md), C++ value types and checked-in config.
LOCAL-WDSP-07 is deferred. Do not add coupling, pitch rise, interpolation upgrades, modal drift,
new excitation layers, limiter/compressor or automatic makeup without a separately justified task.

## Review remediation

1. Replace session-level governance with the explicit controlled objective-spike work item.
2. Prepare only enabled Fluid components and only the renderer-selected candidate. Reset old state;
   disabled processing never advances RNG. JSON syntax/types/integer representation remain globally
   strict; active DSP owns semantic bounds. Test unused-invalid and active-invalid configurations.
3. EventVoicePool rejects capacity 0 or >16 itself, returns failure and stays safely inactive until
   a valid prepare; callers propagate failure. Direct tests cover invalid/unprepared/recovery paths.
4. Keep status capability-based, with exact source/branch identity confined to evidence/PR metadata.
5. Use Stanford CCRMA/book references as primary Smith authority; do not redesign algorithms.

## Validation and exit to review

Execute Contract Review -> Implementation -> Functional Validation -> Code Quality Review ->
Comment & Documentation Pass -> Final Validation. Build/test Debug, Release and ASAN serially using
`tools/build_safe.py`; never bypass resource refusal. Full CTest includes existing production
regressions and spike property/CLI tests. Use new ignored output directories for every evidence run.

- Typical-signal smoke: eight specified TESTDATA-001 inputs, selected A/B/D/ABD/C cases, seed 42,
  checked-in defaults, blocks 7/128/1024, three-second tail; observe events, finite/peak/RMS/DC,
  spectrum, residual/carrier ownership, decay and completely unexcited-channel isolation.
- Full corpus: ten existing signals x eight modes, 80 processed renders; reuse existing analyzer.
- Performance: same-run M1 and M1+C/D/A/B/AB/AD/BD/ABD; mean/P95/P99/worst/increment, no product budget.
- Preserve historical provenance; current source, test counts, smoke/corpus and timing belong in
  [revalidation evidence](REVALIDATION.md). Exact final PR HEAD/Hosted CI is recorded
  in the PR, with an explicit distinction if the final commit changes evidence text only.
- Full Documentation Synchronization Gate: scope/plan, module/paths, config semantics, status,
  test/evidence entry points and authority links agree; production contracts remain unchanged.

After all local gates, open one final PR and verify exact-head Hosted CI. Stop development for
independent Engineering + Sound/Host review. Human listening, accepted brief reconciliation,
algorithm adoption and any later production work remain separate gates.
