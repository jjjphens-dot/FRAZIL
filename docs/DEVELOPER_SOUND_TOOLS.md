# FRAZIL Developer Sound Tools

> Document status: Approval Candidate in v1.3; CURRENT/CONTROLLED after approval and merge.<br>
> Capability implementation status: tracked individually as CURRENT / PLANNED / CANDIDATE / DEFERRED below.<br>
> A Debug/ASAN Developer Control Surface candidate exists on the follow-up feature branch and is engineering-ready
> for Sound & Host Lead usability acceptance; the current development baseline remains PLANNED until review, merge
> and acceptance. Offline Sound Lab handoff remains in progress.

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
- **CURRENT after v1.3 approval/merge**: the Perceptual Contract framework, template and Agent usage rules.
- **CANDIDATE (follow-up feature branch, engineering-ready)**: a Debug/ASAN-only `DEV-UI-001` surface in
  `src/plugin/PluginEditor.*` binds the nine current Host parameters, keeps Water Model/Size/Motion
  experiment-only, detaches APVTS attachments while a temporary non-APVTS override is active, and provides an
  explicit Return Host path. It provides Dry/Processed comparison, bounded A/B/reset slots, complete draft
  experiment-state export, and coherent prepared/latest block diagnostics. It is not the Production UI and has
  not received workflow usability acceptance; this candidate is not the current main baseline.
- **PLANNED**: final `DEV-UI-001` acceptance, richer Offline Sound Lab review packs, reproducible debug bundles,
  automated review-pack generation, LUFS/true peak, spectral flux, onset, pitch/harmonic-retention and extended
  tail analysis. The `EXP-W-001` Water Perceptual Contract instance remains PLANNED until produced and accepted.
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

During Water experiments it may also expose Developer/Experiment controls named Water Model, Water Size and Water
Motion. Before explicit parameter adoption, these controls are not Host parameters, do not enter
`ParameterLayout`, do not change `schemaVersion`, and create no automation or compatibility promise. Their exact
internal transport is a `DEV-UI-001` implementation decision and must preserve the repository dependency and
realtime boundaries.

When the developer override is inactive, normal Host controls use APVTS attachments. While the override is active,
those attachments are detached so the visible controls remain authoritative for the effective developer state;
Return Host clears the override, reattaches the APVTS controls and restores Host/APVTS as effective state. Water
Model/Size/Motion remain outside the Host registry, automation and plugin state. Dry/Processed comparison reuses the
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
