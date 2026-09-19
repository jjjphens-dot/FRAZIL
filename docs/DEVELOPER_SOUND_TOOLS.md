# FRAZIL Developer Sound Tools

> Document status: CURRENT/CONTROLLED; first established by v1.3 / [PR #23](https://github.com/jjjphens-dot/FRAZIL/pull/23). Decay candidate revision follows the [v1.4 activation rule](CODING_PLAN.md).<br>
> Capability implementation status: tracked individually as CURRENT / PLANNED / CANDIDATE / DEFERRED below.<br>
> The Debug/ASAN Developer Control Surface implementation is merged on `main` and is engineering-ready for Sound &
> Host Lead usability acceptance; final workflow acceptance and Offline Sound Lab handoff remain in progress.

## 1. Purpose and boundaries

FRAZIL uses three distinct development and validation entry points:

| Entry point | Purpose | Primary evidence | Must not replace |
|---|---|---|---|
| Host Test (`HOST-001`) | Verify the DAW/VST3 contract | DAW enumeration, automation, state restore, save/reopen, render | Developer UI or offline tests |
| Developer Control Surface (`DEV-UI-001`) | Realtime sound exploration, parameter control and diagnostics | Interactive usability review and bounded diagnostic observations | Host acceptance or deterministic evidence |
| Offline Sound Lab | Reproducible render, sweep, analysis and review-pack generation | Fixed input/config/seed artifacts and analysis | Human listening or Host acceptance |

The Developer Control Surface is not the M5 Production UI, a release surface, a preset system or final product
interaction design. It may be information-dense, use engineering terminology and change quickly. Production UI
continues to require product hierarchy, accessibility, visual design and release acceptance.

`DEV-UI-001` is a Water M2 effective-development-readiness prerequisite before large-scale `EXP-W-002`
experimentation. It is not an M1 architecture-correctness exit gate, and `HOST-001` may proceed independently.

## 2. Status model

- **CURRENT**: the nine static Host parameters, M1 parameter/snapshot/engine path, TESTDATA-001 diagnostic corpus,
  RENDER-001 offline smoke, manual performance harness, and `tools/analyze_testdata.py` using the
  `requirements-dsp.txt` Python environment. The analyzer already provides waveform diagnostics, FFT, Welch PSD,
  RMS, DC, stereo correlation and STFT/spectrogram analysis.
- **CURRENT**: the Perceptual Contract framework, template and Agent usage rules first established by the approved v1.3 baseline; current plan authority follows `CODING_PLAN.md`.
- **CURRENT implementation / acceptance pending**: a Debug/ASAN-only `DEV-UI-001` surface in
  `src/plugin/PluginEditor.*` binds the nine current Host parameters, keeps Water Model/Size/Motion
  experiment-only, detaches APVTS attachments while a temporary non-APVTS override is active, and provides an
  explicit Return Host path. It provides Dry/Processed comparison, bounded A/B/reset slots, complete draft
  experiment-state export, and coherent prepared/latest block diagnostics. It is not the Production UI and has
  not received workflow usability acceptance.
- **PLANNED**: final `DEV-UI-001` acceptance, richer Offline Sound Lab review packs, reproducible debug bundles,
  automated review-pack generation, LUFS/true peak, spectral flux, onset, pitch/harmonic-retention and extended
  tail analysis.
- **ACCEPTED perceptual definition**: [EXP-W-001 Revision B](../experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md),
  with Human ACCEPT, recorded engineering preliminary PASS and user-reported Engineering Lead oral acceptance.
  This does not accept Developer workflow usability, runtime Decay support or candidate listening results.
  Bounded local reference intake reuses the analyzer; it is not an automated listening pack.
- **CANDIDATE / PLANNED**: Sound & Host Lead workflow usability and UI layout acceptance, richer diagnostic/debug
  bundle evidence, and Water experimental-control mapping. The current bounded A/B storage is temporary editor
  state and is not a persistent preset or plugin state.
- **DEFERRED**: Production UI, production presets, generic modulation and a large logging framework remain at their
  existing later-plan gates. Ice tooling remains deferred until `M2 Exit + Explicit Joint Gate` confirms that the
  Water workflow is reusable for Ice.

## 3. DEV-UI-001 minimum scope

The first usable surface must control the current Host parameters through the existing narrow parameter interface:

```text
water.enabled      ice.enabled          routing.mode
parallel.balance   water.amount         ice.amount
input.gain         global.mix           output.gain
```

During Water experiments the candidate vocabulary is Water Model, Water Size, Water Motion and Water Decay.
The original plugin Developer surface exposes the first three; its Decay is a PLANNED extension under
[DOC-W-DECAY-001](planning/WATER_DECAY_CANDIDATE_REVISION.md), not an implemented control.
Before explicit parameter adoption, these controls are not Host parameters, do not enter
`ParameterLayout`, do not change `schemaVersion`, and create no automation or compatibility promise. Their exact
internal transport is a `DEV-UI-001` implementation decision and must preserve the repository dependency and
realtime boundaries.

When the developer override is inactive, normal Host controls use APVTS attachments. While the override is active,
those attachments are detached so the visible controls remain authoritative for the effective developer state;
Return Host clears the override, reattaches the APVTS controls and restores Host/APVTS as effective state. The currently
implemented Water Model/Size/Motion controls remain outside the Host registry, automation and plugin state.
The planned Decay experiment control must remain outside those production contracts as well; its UI/snapshot/export
extension is not implemented. Dry/Processed comparison reuses the
preallocated dry reference and does not change `global.mix`; the candidate still requires workflow usability
acceptance and does not provide the full debug-bundle workflow required for final DEV-UI-001 acceptance.

The editor also reconciles an external Host/APVTS state restore: if the Processor clears a Developer override while
attachments are detached, the next bounded editor reconciliation reattaches Host controls, syncs restored values and
updates the Dry/Processed selection from the Processor comparison mode. Editing an already active override captures an
active-publication token and commits conditionally; if Host restore or another control transaction wins first, the stale
edit is discarded and the visible state is reconciled without reactivating it. The temporary override transport keeps
its audio read/apply path lock-free and bounded; a primed reader may use the last coherent snapshot only within the same
active session, while clear/session epoch invalidation permits Host values at the normal block boundary. Non-realtime
set/clear/conditional-commit operations are serialized because Editor edits and Host state restore may arrive on
different threads.

The first version should provide:

- sample rate, block size and channel count;
- current routing and current parameter snapshot;
- input/output peak and RMS plus finite status;
- current Water experiment mode;
- Dry/Processed comparison, two temporary A/B states and reset-to-current-experiment/default actions;
- export of the current experiment configuration for the Offline Sound Lab.

Water-specific event/voice counts, tail energy, transition state/peak and diagnostic overflow are PLANNED
extensions once a concrete experiment needs them.

Temporary Developer A/B state is not a Production preset, Host saved state or Host automation state. Developer
Dry/Processed, A/B and Reset actions must not silently write DAW automation or redefine production plugin state.
Developer-only comparison state may enter reproducible experiment evidence only through an explicit export/apply
handoff. The exact internal storage and transaction design remains a `DEV-UI-001` implementation decision.

### Planned Decay experiment-control follow-up

A separately scoped implementation may extend `DeveloperWaterExperimentSnapshot` with `decay`, the visible
Water Decay control, A/B capture/apply and experiment reset. A provisional `0.5` normalized experiment UI default
is permitted; it is not a future production default. Motion means temporal activity; Decay means input-excited
response persistence, Short/Tight to Long/Lingering. Do not label the low end Dry or imply whole-effect duration.
The control must not call DSP `prepare()` on a live parameter change or imply that prepare-time SPIKE supports
realtime automation. No product ID enters the primitive, APVTS, ParameterLayout or plugin state.

Required follow-up evidence: default value, A/B retain Decay, experiment reset, explicit export contains Decay,
consumer round-trip where a parser exists, unchanged nine Host parameters/plugin schema, Release Host enumeration
and Developer-control exclusion. GUI/build/plugin validation follows `TESTING.md`; none is claimed by this revision.

### Standalone Water engineering preview

The opt-in `frazil_water_preview` application connects a WAV source and explicit engineering-unit
controls to the existing SPIKE A/B/D/C DSP, with the same value-only diagnostic meters. This is a
separate research target, not a modification of the FRAZIL plugin or its nine Host parameters.
It may use the Release compiler configuration for research timing; that executable is not a
FRAZIL release product/artifact. The plugin Developer/Release isolation invariant below is unchanged.
The application supports applied/draft config, temporary A/B, Dry/Processed/Residual monitoring,
fixed-seed restart and renderer-compatible module JSON export. Source loading/config preparation
runs with the callback detached; algorithm values are prepare-time only. Monitoring gain and
source/residual crossfade use 10 ms smoothing; no production automation/model transition is claimed.

Sound Lead and Engineering views now share a message-thread `ResearchSessionModel`; Model maps only
Fluid/ABD and Resonant/C. The listening-ready follow-up adds research-only Size/Fluid Motion/Decay
curves with per-macro ownership; legacy sessions retain CUSTOM/unmapped values until adoption. Engineering
edits retain inactive values, record origin/revision and require Apply. A/B captures complete applied
experiment/engineering/monitor values. No reverse mapping is inferred from manual engineering edits.
The [control-bridge record](evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md) identifies staged validation;
the standalone preview carries provisional Decay `0.5` in both views, A/B/reset and separate
`frazil.water-research-session` v2 exports with conservative v1 import. See the
[research mapping](../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md) for candidate formulas/limits. Copy/Export Session
uses applied values; Import Session validates syntax/schema/config before replacing state. The
original `DeveloperWaterExperimentSnapshot` and `frazil.dev-experiment` export are unchanged.
Session source metadata is filename/rate/channels/frames only; configure-time Git/build information
is retained on import alongside current build context. It is not a content-identity check. Module
imports preserve macros/monitor/source and use renderer defaults for omitted configuration fields.
Protect research controls now reuse the existing residual-only processor. Depth/Enable publish one
lock-free target consumed at a callback boundary; detector/topology/timing changes require Apply.
D0/D1 calibration memory and Fluid topology are retained separately, with C restricted to Whole.
Protect time fields use strict ms/s exact entry. Fast/Slow/D0/D1/GR now show last-sample values and
peaks over blocks consumed by each UI poll. A fixed SPSC queue drops/counts new summaries when full;
peaks are not co-timed and GR does not imply output-level reduction. The original 21 controls also
use exact entry, adaptive time display and fine gestures; engineering modules are collapsible and
retain inactive values. Draft details and optional audio diagnostics share the scrollable layout.
Actual Windows evidence and limits are recorded in the control-bridge record; no rolling trace is implemented.

The original Debug UI still stores Water Model/Size/Motion locally without DSP mapping; its wet
path remains M1 pass-through. The standalone preview uses engineering quantities, not inferred
Size/Motion/Decay product mappings. Accepted EXP-W-001, subjective EXP-W-002 and production adoption
gates are unchanged. See the [button and debugging guide](DEV_UI_WATER_DEBUG_GUIDE.md) and
[validation record](evidence/WATER_PREVIEW_VALIDATION.md).

### Proposed Protect research control

[DOC-W-PROTECT-001](planning/WATER_PROTECT_CANDIDATE_REVISION.md) is the historical Wave 1 proposal. The
user-authorized [PROTECT-EXP-001](planning/WATER_PROTECT_EXECUTION.md) established offline research. The
separate Water control-bridge scope now connects it to the standalone research preview and its own
session/module exports; the plugin Developer snapshot/editor is unchanged and DEV-UI-001 acceptance
is not extended. Current Water
plugin experiment controls remain Model/Size/Motion; the plugin Decay follow-up above remains planned. A future Protect
experiment-control change needs its own accepted scope/readiness, bounded state handoff and config tests;
it cannot register an APVTS/Host parameter or alter the production schema for convenience.

## 4. Realtime-to-offline handoff

The intended workflow is:

```text
Realtime exploration
  -> export experiment config
  -> deterministic offline render
  -> objective proxies and plots
  -> review pack
  -> human listening decision
```

The exported configuration must identify every experiment input needed for reproduction without silently adopting
new Host parameters. Its exact schema remains CANDIDATE until the Offline Sound Lab and DEV-UI implementation issues
agree on one boundary.

The initial export uses `schema=frazil.dev-experiment`, `schemaVersion=1`, and `source=DEV-UI-001`. This is a local
draft handoff format, not the plugin state schema or a frozen Offline Sound Lab contract.

The planned extension adds `waterExperiment.decay` to explicit experiment export. Current export contains only
`model`, `size`, `motion`; no Decay export has been implemented here. Before changing the format, inventory consumers
and their missing/unknown-field policies, choose a compatible additive field or explicit experiment-schema revision,
and record old/new fixture behavior. Do not change plugin `StateModel::schemaVersion`, APVTS or Host presets.
The SPIKE renderer currently accepts engineering module configs and rejects unknown product fields; this export
cannot be passed directly to it. A reviewed mapping/handoff remains necessary after the accepted brief.

A standard review pack is PLANNED to contain dry, baseline and candidate WAVs, a manifest, per-candidate analysis,
waveform/spectrum/spectrogram plots and `LISTENING_REVIEW.md`. Objective measurements are proxies; no scalar
"quality score" may replace the per-dimension engineering, source-preservation, perceptual and decision record.
New analysis requirements must extend or reuse `tools/analyze_testdata.py` unless a reviewed, concrete limitation
justifies another tool; do not create a parallel analyzer by default.

## 5. Diagnostics and realtime safety

Diagnostics must follow this direction:

```text
Audio thread
  -> atomic counters or a bounded preallocated transport
  -> immutable/bounded DiagnosticsSnapshot
  -> non-realtime consumer
  -> Developer UI, local log or debug bundle
```

The audio thread must not perform disk/network/console I/O, string formatting, dynamic diagnostic allocation,
blocking locks or unbounded queue growth. A complex logging framework is explicitly out of scope.

The candidate uses `DeveloperDiagnostics` as a bounded latest-block transport. Debug/ASAN builds publish input/output
peak and RMS, finite status, prepared maximum block size and latest callback block size; a bounded two-slot
sequence-check keeps the editor from accepting a torn snapshot and falls back to the previous valid snapshot if a
read is not coherent. The editor polls the snapshot on the message thread. Release builds select the non-developer
placeholder and compile out the callback diagnostics publication path.

### Graphical diagnostics presentation candidate

The diagnostics GUI follow-up is an **implementation candidate / human usability acceptance pending**.
`PluginEditor` passes the existing diagnostics snapshot and effective routing text to
`src/ui/DeveloperDiagnosticsView`, which owns two value-only `DeveloperLevelMeter` components. These
components have no Processor, APVTS, engine or transport ownership. They are included only in Debug/ASAN targets; Release retains its existing placeholder.

- INPUT and OUTPUT are aggregate channel-combined latest-block metrics. Peak is the maximum absolute sample;
  RMS combines the channel/sample population. Input is measured before input gain; output is measured after
  the engine, including output gain. The GUI does not change either measurement point.
- RMS is the filled bar, Peak is the instantaneous line, and both retain one-decimal dBFS numeric values.
  The graphical scale is -60 to 0 dBFS. Both readouts include `dBFS`, including zero as `-inf dBFS`;
  quiet numeric values remain below -60 dBFS.
  Values above full scale retain positive dBFS numbers and show an immediate `OVER 0 dBFS` indication while
  the bar saturates. Invalid amplitudes display `Peak INVALID` / `RMS INVALID` without a unit.
- Runtime text preserves sample rate, latest/prepared maximum block sizes in samples, channel count and
  `Route:` for the effective routing, including an active developer override.
  `FINITE OK` and warning-coloured `FINITE NO` reflect the existing snapshot flag.
- The message-thread timer remains 10 Hz. There is no visual smoothing, decay, peak hold or accumulated history;
  peaks between UI observations may be missed. The audio timing and diagnostics transport are unchanged.
- Diagnostics occupies a reserved area below the left parameter grid. Water Size/Motion retains its baseline;
  workflow buttons use a modestly taller 4x3 grid. Control meaning, A/B/export behavior and the 820x680 minimum,
  1000x720 default and 1440x960 maximum editor size contract are unchanged.

This candidate does not provide independent L/R metering, true peak, LUFS, waveform/FFT/spectrum analysis or
production-grade metering. It does not establish a Production UI direction or final Sound & Host acceptance.
Validation observations are recorded in [Project Status](PROJECT_STATUS.md#29-diagnostics-gui-candidate).

A future reproducible debug bundle may contain input/output audio, experiment parameters, engine state, diagnostics,
analysis, performance observations, plots, build provenance and a short README. Raw machine-specific paths and
generated audio remain ignored/local or artifact-hosted according to repository storage rules.

## 6. Build isolation

A Developer Build and Release Build should be distinguishable so developer controls and diagnostics cannot be
shipped accidentally. The initial implementation selects the single-config CMake build type as the isolation
mechanism: Debug (including the Debug-configured ASAN preset) defines `FRAZIL_ENABLE_DEVELOPER_UI=1`, while Release
defines it as `0`. The mechanism remains an implementation choice rather than a new production contract and can be
replaced before final DEV-UI-001 acceptance.

The result invariant is CONTROLLED even while the mechanism remains CANDIDATE:

- a Developer build makes the Developer Control Surface available;
- a Release artifact must not expose the Developer Control Surface, experiment-only controls, debug-only
  diagnostics UI or temporary Developer A/B state;
- the Release Host parameter registry must remain identical to the approved production registry.

`DEV-UI-001` acceptance must verify those Developer/Release outcomes without freezing the option, macro, preset or
internal implementation used to achieve them.

## 7. Ownership and acceptance

- Engineering Lead owns JUCE developer controls, safe parameter binding, diagnostics infrastructure and
  experiment-config export implementation.
- Sound & Host Lead owns workflow/product-usability acceptance: parameter findability, realtime comparison, A/B,
  reset, useful diagnostics and successful handoff to offline experiments.
- Joint Gate decides whether any experimental control becomes a production Host parameter.

`HOST-001` must still use DAW parameter enumeration, automation lanes, state restore and save/reopen. Developer UI
success cannot be cited as Host evidence; interactive success cannot be cited as deterministic offline evidence.

Sound Lead Auto Audition defaults ON: one completed macro gesture prepares/applies/restarts once. Engineering remains manual. Actual mapped targets and a bounded 50-operation history are visible. Monitor controls are above Protect; Reference/Focus and live Protect changes never restart DSP.

A [current v0.2 supplied-input handoff](evidence/WATER_LISTENING_HANDOFF_V02.md) now provides reproducible fixed-source macro comparisons and a human review sheet. Numerically different low/high renders do not establish clear audibility; low-level Resonant Motion remains explicitly unresolved.

PR #40 research-preview remediation defers Stop until an actual value mutation; untouched clicks
do not prepare/restart or enter history. Its separate monitor-over-range latch retains short
output peaks across UI polls and displays a 3 s warning without modifying samples. This does not
change the plugin diagnostics behavior described above. See the
[scoped remediation and normalization review](evidence/WATER_LISTENING_REMEDIATION.md).

The research-only Engineering excitation audition exposes the actual common modal-bank driver,
with Monitor Output but no E Trim or Protect. It is temporary, not serialized or recorded in
history, and cannot alter DSP composition. Offline bounded-carrier candidates remain separate
from preview defaults; see [EXP-W-RX-001](../experiments/water/EXP-W-RX-001.md).
