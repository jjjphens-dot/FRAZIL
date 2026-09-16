# ADR-0006: Water Dual-Mode Architecture

- Status: Proposed
- Date: 2026-09-11
- Work item: `ADR-W-001`

## Modification Policy

This ADR records the proposed technical closure for the decided dual-mode Water product direction. It is not
Accepted and does not authorize production DSP, Host parameter registration, state-schema changes, or final
numeric mappings. Acceptance requires the Water Joint Gate and the evidence listed below. After acceptance,
changes must follow the repository ADR supersession policy.

## Context

The previous M2 plan treated Bubble, Droplet, Flow, Liquid Resonator, and related mechanisms as candidates from
which one Water vertical slice would be selected. The product direction is now a dual-mode material processor:

- `Fluid` combines A — input-driven Bubble Ensemble, B — input-driven Droplet/Impact Exciter, and D — Flow
  Modulator;
- `Resonant` uses C — input-driven Liquid/Modal Resonator.

Both modes must transform musical input rather than act as independent Water Foley generators. The current
accepted routing, static-parameter, state, realtime, and zero-sample Host-processing-latency contracts remain in
force. Ice is outside this Water revision.

## Proposed Decision

### Product and signal responsibility

WaterProcessor owns a dual-mode, input-driven Water transformation. Conceptually:

```text
W(x) = x + E_water(x)
E_fluid = E_bubble + E_droplet + E_flow
E_resonant = modal/resonant material residual
```

This composition is an FRAZIL engineering/product inference, not a formula attributed to the cited acoustics
literature. Every sub-engine must explicitly declare whether its output is a material residual or a complete
processed signal. Residual-oriented composition is preferred. A resonator output that already contains direct
feedthrough must not be added to `x` again.

WaterProcessor does not own Parallel/Serial routing, `water.amount`, `parallel.balance`, or `global.mix`. A public
`water.dryWet` parameter is explicitly not proposed.

### Product controls

M2 evaluates these experiment-only candidate controls for possible later Host adoption:
Decay revision: [DOC-W-DECAY-001 / #32](https://github.com/jjjphens-dot/FRAZIL/issues/32), pending review/merge.

- `water.model`: working choice order `Fluid`, `Resonant`;
- `water.size`: `Fine / Small / Bright <-> Large / Deep`;
- `water.motion`: temporal activity, `Calm / Stable <-> Active / Flowing`;
- `water.decay`: Water response persistence, `Short / Tight <-> Long / Lingering` (not Dry).

Size, Motion and Decay retain the same high-level meaning and UI position in both modes. The planned chain is
Host / Developer Control -> ParameterSnapshot -> ParameterMapper -> WaterProductValues { model, size, motion, decay }
-> WaterMacroMapper (Water domain) -> FluidTargets / ResonantTargets -> DSP components.
ParameterMapper owns raw interpretation, finite fallback, clamp, choice-to-enum, dB-to-linear and normalized
product/domain value preparation. It does not know component configs, decay seconds, event probabilities,
trajectory intervals, modal coefficients or voice lifetimes. WaterMacroMapper alone owns normalized Water
values -> mode-specific bounded DSP targets: pure C++, deterministic, allocation-free, unit-testable and
independent of JUCE/APVTS/UI/DSP state. Primitives consume engineering quantities, not product parameter IDs.
This is a future boundary, not an implemented Developer/Host path or a new generic mapping framework.
Detailed ranges, defaults, nonlinear curves, transition duration, smoothing constants, and state evolution
are not decided here.

Responsibility orthogonality + perceptual separability + bounded interaction: Motion does not directly drive
decay targets; Decay does not directly drive event/trajectory-rate targets. High Motion + Long Decay may increase
overlap, voices, tail energy and steals; measure bounds and listening usability, not strict acoustic independence.
Droplet refractory is an implementation quantity, not a frozen Motion destination. Water remains a continuous
input-driven transform; no whole-effect duration/hold/restart/envelope is introduced. Prepare-time SPIKE decay
is not realtime automation and does not authorize callback reprepare/allocation/blocking.

### Mode transition candidate

If both engines expose residuals, M2 may evaluate:

```text
W = x + (1 - c) * E_fluid + c * E_resonant
```

where `c` is bounded and click-free. This is not an accepted topology or duration.

### Internal modulation boundary

Per-instance random sources, probabilistic events, smoothed random processes, internal low-frequency motion, and
subtle resonator drift are permitted implementation details when bounded and realtime-safe. A general
user-programmable LFO/modulation matrix is deferred. A constrained future `Motion Mod` may be investigated only
with user evidence and a separate parameter/state review.

## Open Decisions Required for Acceptance

- exact residual extraction and resonator feedthrough topology;
- final Bubble/Droplet/Flow/Resonant structures and ablation results;
- Size, Motion and Decay engine destinations, ranges, defaults, nonlinear curves, and energy compensation;
- validate Decay perceptual separability and cross-mode persistence direction against the accepted brief;
- Bubble/Droplet response-decay destinations and Resonant damping curves; no forced Flow decay;
- normalized per-mechanism curves (equal normalized values need not mean equal seconds);
- live damping versus event-latched policy, existing-state response, coefficient/excitation updates and automation lag;
- voice lifetime/stealing, overlap, tail termination, energy buildup, smoothing/rapid automation and state compatibility;
- whether both engines run during a mode transition;
- engine state, tail, random-state progression, rapid-automation, reset/prepare, and restore behavior;
- final transition duration and CPU upper bound;
- production seed source, instance decorrelation, offline determinism, and persistence semantics;
- intentional effect tail behavior within ADR-0005's zero-sample Host processing-latency contract;
- parameter adoption, stable choice order, and state-schema evolution/compatibility strategy;
- final Fluid/Resonant performance budget and failure/degradation behavior.

## Consequences

- `EXP-W-001` defines common/Fluid/Resonant identity, Size/Motion/Decay semantics, anti-examples, and source
  recognizability. It must not be accepted/closed before the mandatory
  [Decay Revision B completion gate](../CODING_PLAN.md#decay-revision-b-completion-gate).
- `EXP-W-002` measures Bubble, Droplet/Impact, Flow, and Resonant components separately and in integration with
  fixed test seeds where randomness is involved. Formal work requires M1 Exit, applicable Developer readiness
  and accepted EXP-W-001 including Revision B; neither Revision A nor the old three-macro brief suffices.
- `EXP-W-003` validates and refines the dual-mode direction instead of selecting only one Water slice.
- `water.model`, `water.size`, `water.motion`, and `water.decay` remain outside the current nine-parameter registry and
  `schemaVersion=1` until explicit adoption and compatibility work.
- High-fidelity coupled-bubble/full-fluid/FDTD methods inform approximation risk but are not v1 realtime
  requirements.
- Ice design, parameters, work items, and tests remain unchanged.

## Verification Required Before Accepted

- loudness-matched evidence that evaluates Fluid and Resonant independently against their mode-specific
  responsibilities, plus qualitative confirmation that both are intentional Water models rather than a good/bad
  switch; no perceptual-distance metric, classification threshold, or mode-separation score is required;
- source recognizability at normal settings, including representative `global.mix=100%` Water-only evaluation;
- Size, Motion and Decay semantic consistency across both modes, with Motion = activity and Decay = persistence; neither primarily gain/Amount;
- per-mode Motion x Decay 2x2, held-macro destination isolation, dynamic-state-policy and tail/overlap evidence;
- fixed-seed scheduling/RNG ownership review when changing Decay; explained architecture-dependent interactions;
- loudness-matched separability, source rhythm/transient preservation and masking/ringing/stealing/energy review;
- component ablation and fixed-seed engineering evidence;
- finite output, extreme parameters, DC/peak/tail, reset/prepare, 44.1/48/96 kHz, representative block-size,
  rapid mode automation, click/zipper, and instance-isolation evidence;
- performance increments relative to `PERF-BASE-001`, including mode-transition peak cost;
- confirmation that Host-reported processing latency remains 0 samples;
- parameter/state compatibility plan and fixtures before any new Host registration;
- Engineering and Sound/Host Joint Gate review evidence.

## References

The scientific and official product references, their supported inferences, and their limitations are maintained
in [`CORE_IMPLEMENTATION_GUIDE.md`](../CORE_IMPLEMENTATION_GUIDE.md#59-参考文献与官方资料).
