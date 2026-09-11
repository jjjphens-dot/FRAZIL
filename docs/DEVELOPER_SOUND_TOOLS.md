# FRAZIL Developer Sound Tools

> Status: PLANNED development-enablement contract; no Developer Control Surface or diagnostics bridge is implemented by this document.

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
  RENDER-001 offline smoke and the manual performance harness.
- **PLANNED**: `DEV-UI-001`, a minimal diagnostics snapshot/bridge, realtime A/B workflow, experiment-config export,
  richer Offline Sound Lab review packs and reproducible debug bundles.
- **CANDIDATE**: exact developer-build isolation, diagnostic transport, UI layout, A/B storage lifetime, exported JSON
  schema and Water experimental-control mapping.
- **DEFERRED**: Production UI, production presets, generic modulation, a large logging framework and Ice tooling until
  the Water-first method is stable.

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

The first version should provide:

- sample rate, block size and channel count;
- current routing and current parameter snapshot;
- input/output peak and RMS plus finite status;
- current Water experiment mode;
- Dry/Processed comparison, two temporary A/B states and reset-to-current-experiment/default actions;
- export of the current experiment configuration for the Offline Sound Lab.

Water-specific event/voice counts, tail energy, transition state/peak and diagnostic overflow are PLANNED
extensions once a concrete experiment needs them. Temporary A/B states are development state, not release presets.

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

A standard review pack is PLANNED to contain dry, baseline and candidate WAVs, a manifest, per-candidate analysis,
waveform/spectrum/spectrogram plots and `LISTENING_REVIEW.md`. Objective measurements are proxies; no scalar
"quality score" may replace the per-dimension engineering, source-preservation, perceptual and decision record.

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

A future reproducible debug bundle may contain input/output audio, experiment parameters, engine state, diagnostics,
analysis, performance observations, plots, build provenance and a short README. Raw machine-specific paths and
generated audio remain ignored/local or artifact-hosted according to repository storage rules.

## 6. Build isolation

A Developer Build and Release Build should be distinguishable so developer controls and diagnostics cannot be
shipped accidentally. The mechanism is CANDIDATE: a build option, dedicated preset or another bounded approach may
be selected by the Engineering Lead in the `DEV-UI-001` implementation issue. This document intentionally does not
freeze a macro name or preset.

## 7. Ownership and acceptance

- Engineering Lead owns JUCE developer controls, safe parameter binding, diagnostics infrastructure and
  experiment-config export implementation.
- Sound & Host Lead owns workflow/product-usability acceptance: parameter findability, realtime comparison, A/B,
  reset, useful diagnostics and successful handoff to offline experiments.
- Joint Gate decides whether any experimental control becomes a production Host parameter.

`HOST-001` must still use DAW parameter enumeration, automation lanes, state restore and save/reopen. Developer UI
success cannot be cited as Host evidence; interactive success cannot be cited as deterministic offline evidence.
