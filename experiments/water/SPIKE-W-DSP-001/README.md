# Water DSP objective feasibility — SPIKE-W-DSP-001

Research-only A/B/D/C mechanisms; local Debug/Release/ASAN 16/16 each, 138 core + 4 supplemental smoke renders
and 80 corpus renders PASS. Exact-head Hosted CI is separately recorded in PR #30.
No production WaterProcessor or perceptual acceptance. Source and limitations: [REVALIDATION.md](REVALIDATION.md).

Those counts describe the historical SPIKE revision. Current Protect follow-up results and limitations are
separately recorded in [PROTECT-EXP-001](../../../docs/planning/WATER_PROTECT_EXECUTION.md).

The [listening-UI follow-up](../../../docs/evidence/WATER_LISTENING_UI_EXECUTION.md) records the
subsequent review fixes and research-tool changes. The preview now groups completed mouse gestures
and debounced wheel/key edits into a 50-entry runtime operation history, separate from state revisions
and excluded from every serialized format. Prepare-required drags stop once; live Protect remains live.

## Protect follow-up — PROTECT-EXP-001

The user authorized sequential objective engineering waves/self-review and one final upload after Wave 1.
This is separate from the closed SPIKE scope and does not satisfy formal EXP-W-002 or human/product gates.
The original A/B/D/C generators are reused unchanged; Protect owns no generator, random stream or tail.

| Addition | API / ownership | Validation |
|---|---|---|
| `dsp/ProtectDetector.h` | prepare/reset/process; own linked fast/slow follower; D0 difference and D1 positive log ratio with slow floor | Level/floor/cap/rate/reset/stereo tests |
| `dsp/ResidualProtect.h` | prepare/reset/setDepth/processSource; score-to-dB computer, own gain envelope; apply whole residual | Bounds, exact OFF/continuation, retarget, lifecycle and partition tests |
| `dsp/FluidProtect.h` | Pure F1 whole / F2 Droplet exempt / F3 half-weight composition | Exact unity and cancellation counterexample |

Single processing owner: lifecycle/targets and process must not race. Config is prepare-only except depth,
which can be retargeted at sample boundaries by the research caller; no plugin transport exists. Follower
and generator advance during OFF. Positive depth uses dB attack/release; OFF uses a finite dB ramp then exact
unity. Repeated OFF does not restart; reset clears envelope state while retaining the prepared depth/config.
The step-bound test is not a human click-free claim. F2/F3 can increase the summed residual through cancellation.

The existing JSON config now optionally accepts a numeric `protect` object (omitted = exact OFF):

| Field | Default | Valid experimental range / units |
|---|---|---|
| depth | 0 | 0..1; not a Host default |
| detector / topology | 1 / 1 | detector 0=D0, 1=D1; topology 1=F1, 2=F2, 3=F3 (C always whole residual) |
| floor / epsilon | 1e-4 / 1e-8 | floor 1e-8..0.1; 0 < epsilon <= floor; amplitude |
| thresholdLow / thresholdHigh | 1 / 9 | 0 <= low < high <= 100 dB (D1), <=1 amplitude (D0); D0 examples use .01/.12 |
| capDb | 9 | 0..12 dB attenuation; search cases 3/6/9/12 |
| depthExponent / scoreExponent | 1 / 1 | .1..8; finite |
| attackSeconds / releaseSeconds | .001 / .08 | .00025.. .002 / .04.. .2 seconds |
| offSeconds | .01 | .001.. .1 seconds; finite transition duration |

All Protect config semantics are validated before output, including OFF/baseline. No log/linear smoothing
comparison, live Decay, macro mapping or optimized production settings are claimed. The renderer accepts
an optional final `NEW-protect-trace.csv` after tail seconds. It records frame/D0/D1/GR outside DSP and timing,
refuses output/source collisions and existing trace files, and fails on write errors. No makeup/limiter is used.

```powershell
# Existing research-enabled Release build, using the safe wrapper as documented below.
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
python experiments/water/SPIKE-W-DSP-001/analysis/protect_review.py --renderer $renderer --suite diagnostics --output build/protect-diagnostics
python experiments/water/SPIKE-W-DSP-001/analysis/protect_review.py --renderer $renderer --suite interaction --output build/protect-interaction
python experiments/water/SPIKE-W-DSP-001/analysis/protect_review.py --renderer $renderer --suite sweep --output build/protect-sweep
```

All output directories must be new. Reports contain raw metrics/config/seed, source commit+dirty state,
per-sample traces and selected diagnostic plots. Canonical fixture onset annotations use 5 ms pre/50 ms post
windows, union pooling, 1 dB duty threshold, nearest-rank P95 and a fixed 1 ms rectified-peak analysis envelope.
These are declared engineering observations, not psychoacoustic timing constants or detector-derived labels.
Silent denominator/zero residual are explicit N/A/negative-infinity reasons, never fake finite scores.
Interaction uses static engineering activity/persistence proxies: 12 Fluid cases and 6 unique C cases; C
Motion is N/A because the existing resonator has no such destination. No accepted macro curve is inferred.

Listening handoff (user supplies audio/permission later):

```powershell
python -m pip install -r requirements-dsp.txt
python experiments/water/SPIKE-W-DSP-001/analysis/prepare_protect_listening.py --renderer $renderer --source <authorized-input.wav> --source-metadata <local-metadata.json> --mode abd --dsp-seed 42 --randomization-seed 42 --appended-tail-seconds 3 --output build/protect-listening-fluid-v2
```

Use `--mode c` for Resonant; add `--diagnostic` for engineering fixtures. The metadata JSON requires nonempty
`source`, `author`, `license`, `permission` and `storage_policy` strings. Describe the actual source/version and
permission for local copies/derived renders; a filled field does not itself establish permission. Sources must
be PCM/float WAV at 44.1/48/96 kHz, mono/stereo. Keep metadata, source copies and packs in ignored local storage.
For example, a self-created fixture's metadata can identify its generator/version, author, applicable license,
local-test permission and temporary retention policy. Do not copy those claims onto third-party music.

Each run creates two independent randomized packs, each containing **21 Fluid or 13 C trials**:

- `fixed_source/`: primary attack/source-preservation evidence. Every condition uses one common gain
  `min(1, .9 / maximum_raw_peak)`, including dry/OFF and lower-residual controls. There is no per-condition
  normalization, so the carrier coefficient is identical. The common gain and every playback gain are recorded.
- `rms_matched/`: **preference-supporting evidence only**. Full comparison-window RMS (including the appended
  tail) is matched with condition-specific gains and .9 peak headroom. This is neither LUFS nor proven equal
  perceived loudness, and cannot establish attack/source preservation. It has separate scores and conclusions;
  never copy/pool conclusions between the two packs.

Both include explicit D0/D1 OFF/mild/medium/strong pairs and surviving Fluid topology pairs. D0 uses .01/.12
amplitude thresholds; D1 uses 1/9 dB. All Protect fields are written explicitly to renderer configs. These are
bounded configurations, not equally tuned detector families. OFF, D0-medium and D1-medium have hidden repeats
with identical config/seed/gain/audio. Fixed-source randomization uses the supplied seed; RMS uses seed + 1.
DSP seed is independent. Trial WAVs alone are the blind handoff; the coordinator retains keys and `reviewer/`
raw/config/source files until scoring ends. Both scorecards start blank.

Manifest schema 2 records source name/description, original `source_frames`/`source_duration_seconds`, channels,
subtype/bit-depth, source/author/license/permission/storage policy, `dsp_seed`, actual `randomization_seed`,
`comparison_frames`/duration and separate `appended_tail_seconds`. The original byte snapshot is retained as
`reviewer/source.wav` relative to the run root; renderers use that snapshot. No source/artifact hashes are
computed, per the user's superseding instruction. Names/metadata/copies distinguish sources, without a
cryptographic identity claim. `code_commit`/`code_dirty` identify repository code, not source audio.

`DETECTOR_SELECTION.md` requires independent fixed-source observations, repeat consistency, attack/identity,
quiet-after-loud response and recovery tradeoffs, exact condition references, reviewer and rationale (including
neither/revise). **D1 is not selected by default; Wave 7 remains BLOCKED until detector selection and human
listening evidence exist.** Product adoption still needs Joint Gate/ADR. Historical 12/8 RMS-only packs are
superseded preparation artifacts and cannot support attack/source-preservation conclusions.

CTest `frazil_water_protect_listening` invokes the real renderer and pack CLI, checks decoded carrier gains,
explicit detector behavior/configs, source metadata/tail separation, hidden repeats, deterministic order/audio,
independent evidence labels/blank scores and missing-rights rejection. Hosted CI installs `requirements-dsp.txt`.

## Scope and execution state

The optional pre-EXP-W-002 work item is defined in [Coding Plan](../../../docs/CODING_PLAN.md),
[Perceptual Contract section 6](../../../docs/PERCEPTUAL_CONTRACT.md#6-optional-objective-feasibility-before-the-water-instance)
and [Issue #29](https://github.com/jjjphens-dot/FRAZIL/issues/29). The
[implementation plan](../SPIKE-W-DSP-001_IMPLEMENTATION_PLAN.md) bounds objective numerical,
realtime, lifecycle, isolation, render and performance work. Engineering Lead implements;
Sound & Host Lead independently reviews scope/evidence.

Formal EXP-W-002 still requires accepted EXP-W-001 (#17). Subjective tuning/selection, Water
identity acceptance, macro mapping and Fluid/Resonant quality rankings are forbidden before that
brief. This spike does not close EXP-W-002, accept ADR-W-001 or authorize production integration.
Future EXP-W-002 reuses/revises these results against the accepted brief without duplicating DSP.
LOCAL-WDSP-00..06 are engineering checkpoints; LOCAL-WDSP-07 remains deferred. No listening
finding justifies additional synthesis complexity in this remediation.

Current remediation evidence is recorded in [REVALIDATION.md](REVALIDATION.md); the earlier
[EVIDENCE.md](EVIDENCE.md) remains explicitly historical and applies only to its original source.

## Module map and output contract

All DSP lives here and is excluded from production plugin targets. State belongs to the calling
processing owner; prepare/reset/process must not execute concurrently. A/B/D/C generator controls are fixed
by prepare; they have no runtime parameter transport, automation smoothing or mode transitions. The separate
Protect depth retarget/OFF envelope is described above; it does not implement generator macro automation.

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
names. Config files use UTF-8 (an initial UTF-8 BOM is tolerated) and must contain exactly one
complete root object, whose module objects contain numeric fields. The offline syntax gate checks
[RFC 8259](https://www.rfc-editor.org/rfc/rfc8259) object/string/number grammar before JUCE decodes
values: concatenated roots, trailing garbage, malformed escapes, leading-zero numbers, incomplete
fractions/exponents and unescaped control bytes fail. Raw file bytes are checked before string
conversion so a NUL cannot silently truncate the document. Invalid configs fail before output creation,
including baseline and inactive-module cases. This bounded schema check adds no general JSON framework.
Duplicate decoded keys are rejected at either level, including escaped aliases, so JUCE's overwrite
behavior cannot hide an unknown field or nonfinite value from the global checks.
Omitted JSON fields retain C++ defaults. Unknown fields, nonnumeric/nonfinite values and
invalid voice-count representation (negative, fractional or outside size_t) fail globally, even
in unused modules. Integer literals outside JUCE’s signed int64 parser representation are rejected
before parsing to prevent wraparound; decimal/exponent values must still be finite and voice counts
representable as size_t. Type-valid semantic errors (e.g. voices=0/17 or negative decay) fail only when
that DSP is enabled. Only the selected baseline/C/Fluid subset prepares; disabled components are
reset and never processed or advanced. Active DSP remains strict. `water.model/size/motion` are
not accepted. Flags are fixed at prepare, not live bypass/transition controls.

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

## Standalone engineering preview

The optional `frazil_water_preview` GUI reuses these mechanisms for source-driven engineering
inspection. It has a WAV transport, explicit-unit draft/apply controls, temporary applied-config
A/B, Dry/Processed/Residual monitoring and JSON export understood by this renderer. Every algorithm
change stops playback and requires Apply + Play; no DSP prepare runs in the audio callback. This
adds no product macro mapping, perceptual acceptance or production plugin dependency.

Enable `FRAZIL_BUILD_WATER_EXPERIMENT=ON` and `FRAZIL_BUILD_WATER_PREVIEW=ON` using the existing safe
presets. The preview remains a separate executable even in Release. See the
[Sound Lead debugging guide](../../../docs/DEV_UI_WATER_DEBUG_GUIDE.md) for buttons, raw controls,
source/device limits and reproducibility, and [validation](../../../docs/evidence/WATER_PREVIEW_VALIDATION.md).

## Build, tests, render and measurement

From an initialized MSVC developer environment at repository root:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

Repeat serially with `windows-release` and `windows-asan`. The opt-in option defaults OFF. Existing
safe presets build research through `frazil_smoke` dependencies; no research code is linked into
FRAZIL. CTest includes baseline/features/modal/bubble/flow/droplet/fluid/event-pool plus decoded renderer tests.
ASAN tests receive the compiler runtime path, and executables receive the runtime DLL. Hosted CI
explicitly enables this option; no Hosted CI result is claimed from the local runs.

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
& $renderer testdata/input/zero_state_response__impulse.wav build/water-c.wav c 128 42 experiments/water/SPIKE-W-DSP-001/configs/defaults.json 3
python tools/analyze_testdata.py build/water-c.wav --json-out build/water-c.analysis.json
python experiments/water/SPIKE-W-DSP-001/analysis/review_smoke.py --renderer $renderer --output build/water-review-smoke
python experiments/water/SPIKE-W-DSP-001/analysis/render_corpus.py --renderer $renderer --output build/water-corpus
& 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_performance.exe'
```

Renderer arguments: input WAV, **new** output WAV, mode, block (1..8192), uint32 seed, optional
JSON path (or `-` for defaults), optional integer tail seconds (0..30), then optional new Protect trace CSV.
Modes: `a`, `b`, `d`, `ab`,
`ad`, `bd`, `abd`, `c`; append `-residual` for E only. `baseline` is pass-through PCM24; `residual`
is its zero residual. Sonic renders are float32 WAV, retaining peaks above 1 for analysis. Input
must be finite mono/stereo within full scale. Existing outputs are refused; failed renders are
not valid evidence and may leave a partial new file. Use a new ignored output directory each run.
Offline diagnostics report A/B event counts, first event frames (-1 for none), and events on exactly
zero source frames. They do not alter DSP state, output or the separate timed callback benchmark.

The single `review_smoke.py` command runs 138 core renders and four supplemental controls.
`observations.json` retains the 32 core signal/mode records. `supplemental_controls.json` records
A/B high/low gate event counts derived from a canonical-input prefix, and Flow-D
processed-vs-dry RMS level delta and dominant frequencies. Gate boundaries come from TESTDATA-001's manifest; the
prefix preserves initial detector/RNG history and matches the corresponding full-render samples.
Two dry HF/sweep baselines use the same three-second appended silence as processed/residual runs.
Existing analyzer PSD/spectrogram plots support inspection of coloration/ripple, without automatic
alias attribution or subjective audibility claims. All derived inputs and outputs stay ignored.
The script checks that all 18 expected dry/processed/residual plot files exist; it does not inspect pixels.

RMS metrics use the full render, including appended silence, and remain distinct:

| Supplemental JSON field | Meaning |
|---|---|
| `processed_rms` | Processed signal RMS(y), linear amplitude |
| `baseline.rms` | Dry/source RMS(x), linear amplitude |
| `processed_vs_dry_rms_delta_db` | Processed-vs-dry RMS level delta: 20 log10(RMS(y) / RMS(x)), dB |
| `residual.rms` | RMS(E) for E = y - x, linear amplitude; a separate measurement |

Hosted CI runs repository policy/tool checks, TESTDATA-001 verification and research-enabled
configure/build/CTest; Water smoke, 80-render corpus and research timing are local evidence.

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

## Validation checkpoints

The [control-bridge execution record](../../../docs/evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md)
tracks staged Preview/Protect integration. Phase 1 adds isolated UI-thread `formatTimeValue` and
`parseTimeValue` helpers in `preview/TimeValue.h`, covered by the existing Preview test executable.
Seconds remain the internal/config unit. Display uses ms through exactly one second, then s;
the locale-independent formatter preserves fractional values. Exact entry accepts decimal or
scientific numbers, ASCII whitespace and case-insensitive ms/s only, defaulting to ms. Bounds are
inclusive in seconds; invalid input returns an error without overwriting the prior value or clamping.
The formatter rejects negative/nonfinite values. Neither helper is called from the audio path or
connected to current widgets yet; current GUI instructions remain unchanged.

Phase 2 moves the existing 21-control metadata to `preview/ControlDescriptor.h`, adding typed IDs,
groups, internal units, display policy, research baseline provenance and prepare-required lifecycle.
The original values, UI ranges/steps, labels and four-module export schema are retained. Descriptor
tests check complete unique IDs, typed DSP defaults, pre-refactor ranges and serialization coverage.

Phase 3 adds a single message-thread `ResearchSessionModel` with command-based edits, provenance and
draft/applied/A/B values. `ResearchViews.h` supplies Sound Lead/Engineering representations; the
`PreviewPanel` coordinator stops playback for draft edits and validates through `PreviewController`.
Model/composition mapping is limited to Fluid/ABD and Resonant/C. Size/Motion remain UNMAPPED; manual
raw edits do not reverse-map them. Inactive controls retain values and show their inactive status.
No view owns duplicate parameter state or holds DSP objects. Session tests verify both-view observation,
one notification per change, no-op feedback suppression, composition/active-module truth tables,
provenance, invalid commands, applied isolation and complete temporary A/B restore.

Phase 4 adds normalized Decay with the DOC-W-DECAY-001 provisional baseline `0.5`; it is UNMAPPED
and cannot affect Flow or any DSP parameter. Four macros participate in A/B/reset and separate
`frazil.water-research-session` manifests (v2 exports, conservative v1 imports). `SessionCodec.h` covers config/composition,
fixed seed 42, monitor and provenance, including source/build context without audio bytes or absolute paths.
Module imports reuse renderer defaults and validation, preserve session macros/source/monitor, and
mark engineering values CUSTOM. Configure-time Git/build provenance requires reconfigure to refresh.
All engineering/Protect exact-entry widgets use adaptive ms/s display, strict parsing, baseline reset
and Shift fine drag. Engineering module cards retain inactive settings; draft details and audio
diagnostics are optional expansions. The viewport follows the current tab and expansion heights.
`SessionJsonSyntax.h` remains necessary because the renderer gate
only supports two numeric object levels; the bounded session syntax additionally supports strings,
booleans and nested provenance, rejects duplicate keys/full-input violations, and never runs in
the callback. Import decodes a candidate, then validates existing DSP config before replacement.
Source/build provenance and Protect state are included in the completed workflow; this research
format has no public preset compatibility promise. Renderer module JSON remains unchanged.

Phase 5 wires the existing `ResidualProtect` and `applyFluidProtect` into `PreviewEngine`, preserving
exact Depth-zero baseline and generator progression. `PreviewController` transfers one lock-free
double Depth target at callback boundaries; all other Protect config still requires stopped Apply.
Source is added once by monitoring after residual processing. D0/D1 threshold memories, last nonzero
Depth and Fluid topology belong to UI session state, not DSP/renderer config. Initial enable recall
is a documented convenience `0.5`; the actual research baseline remains OFF/Depth zero. C uses only
Whole and restores the remembered Fluid topology when returning. Session/renderer exports include
existing Protect module fields; no schema key was added to the renderer. The shared exact-entry
widget validates before slider snapping/clamping, supports adaptive ms/s and flags invalid text.
Protect DSP sources remain identical to their source branch; samplewise integration tests compare
both detector domains/topologies and exact OFF recovery at all three supported validation rates.

Phase 6 adds `ProtectDiagnostics.h`: a preallocated 256-entry SPSC queue of callback summaries,
carrying Fast/Slow linear amplitude, D0 amplitude, D1 dB ratio and GR dB attenuation. The callback
keeps per-block peaks and publishes once; the 10 Hz UI drains at most 256 entries, displays the
latest sample plus interval peaks and cumulative dropped summaries. Full queues drop new data,
never wait/overwrite; Stop clears the queue after callback detach. Peaks are not co-timed values
or output-level reduction. No rolling trace, callback strings or dynamically growing history is
introduced. Concurrent transport/overflow tests complement samplewise detector-value comparisons.

LOCAL-WDSP-00..06 cover baseline, features, C, A, D, B and Fluid integration respectively.
Historical measurements are retained in [EVIDENCE.md](EVIDENCE.md). Current source, three-preset
regression, isolation/capacity fixes, typical-signal smoke, 80 renders and preliminary timing are
separately identified in [REVALIDATION.md](REVALIDATION.md). No checkpoint is sound acceptance.

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

Documentation synchronization covers the controlled optional-spike scope, Agent/Code Standards
cross-references, Coding Plan, Perceptual Contract, experiment plan/index/README, module/testing
entry points, implementation guide, Environment paths and merge-stable project status. Architecture,
Parameters and Accepted ADRs retain their production contracts; see the detailed documentation
review in [REVALIDATION.md](REVALIDATION.md).

## Primary sources and inference boundaries

- [Smith: pole radius and bandwidth](https://ccrma.stanford.edu/~jos/filters/Relating_Pole_Radius_Bandwidth.html): exponential pole mapping informs decay stability; local normalization still requires tests.
- [Smith: delay-line interpolation](https://ccrma.stanford.edu/~jos/pasp/Delay_Line_Signal_Interpolation.html): linear interpolation is inexpensive but has frequency-dependent error; it does not guarantee a Flow percept.
- [Pumphrey et al., DTU, 1989](https://orbit.dtu.dk/en/publications/underwater-sound-produced-by-individual-drop-impacts-and-rainfall/): impact emission and entrained-bubble ringing motivate separate A/B mechanisms.
- [van den Doel, UBC](https://www.cs.ubc.ca/labs/lci/lci-forum/03/vandendoel-040312.html): isolated bubble models and stochastic populations support the approximation strategy, not our exact event law.
- [RSC: acoustic interaction between cubic bubbles](https://pubs.rsc.org/en/content/articlehtml/2020/sm/c9sm02423a): discusses the inverse-radius Minnaert reference and deviations through interaction.
- [Xue et al., Stanford, 2023](https://graphics.stanford.edu/papers/coupledbubbles/): coupling affects low-frequency emissions; independent oscillators omit that behavior.

Smith primary references are Julius O. Smith III, *Introduction to Digital Filters with Audio
Applications* (W3K Publishing, 2007), “Relating Pole Radius to Bandwidth,” and *Physical Audio Signal
Processing* (W3K Publishing, 2010), “Delay-Line and Signal Interpolation,” hosted by Stanford CCRMA.
The institutional pages restrict automated retrieval; book provenance was cross-checked against
[Smith’s DAFx-09 keynote references 2–3](https://www.dafx.de/paper-archive/2009/tutorials/DAFx09-knp-jos.pdf) and the
[Stanford 2010 book notice](https://cm-mail.stanford.edu/pipermail/planetccrma/2010-September/017292.html).
Mirror hosting is not the primary authority. Citation correction changes no algorithm.

References were reviewed on 2026-09-16. FRAZIL's residual composition, Fluid/Resonant names, gains,
frequency families and scheduling are engineering hypotheses, not formulas endorsed by these papers.

The listening-ready follow-up exports research session v2 and accepts v1 without changing raw
engineering values. Legacy macro states are CUSTOM / legacy-unmapped. Runtime operation history
never enters either schema; see the debug guide for DSP/context dirty and checkpoint semantics.
