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

M2 evaluates these candidate Host-visible controls:

- `water.model`: working choice order `Fluid`, `Resonant`;
- `water.size`: `Fine / Small / Bright <-> Large / Deep / Full`;
- `water.motion`: `Calm / Stable <-> Active / Flowing`.

Size and Motion retain the same high-level meaning and UI position in both modes. ParameterMapper owns the
normalized-product-to-mode-specific-engine mapping. Detailed ranges, defaults, nonlinear curves, transition
duration, smoothing constants, and state evolution are not decided here.

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
- Size and Motion engine destinations, ranges, defaults, nonlinear curves, and energy compensation;
- whether both engines run during a mode transition;
- engine state, tail, random-state progression, rapid-automation, reset/prepare, and restore behavior;
- final transition duration and CPU upper bound;
- production seed source, instance decorrelation, offline determinism, and persistence semantics;
- intentional effect tail behavior within ADR-0005's zero-sample Host processing-latency contract;
- parameter adoption, stable choice order, and state-schema evolution/compatibility strategy;
- final Fluid/Resonant performance budget and failure/degradation behavior.

## Consequences

- `EXP-W-001` defines common/Fluid/Resonant identity, Size/Motion semantics, anti-examples, and source
  recognizability.
- `EXP-W-002` measures Bubble, Droplet/Impact, Flow, and Resonant components separately and in integration with
  fixed test seeds where randomness is involved.
- `EXP-W-003` validates and refines the dual-mode direction instead of selecting only one Water slice.
- `water.model`, `water.size`, and `water.motion` remain outside the current nine-parameter registry and
  `schemaVersion=1` until explicit adoption and compatibility work.
- High-fidelity coupled-bubble/full-fluid/FDTD methods inform approximation risk but are not v1 realtime
  requirements.
- Ice design, parameters, work items, and tests remain unchanged.

## Verification Required Before Accepted

- loudness-matched evidence that evaluates Fluid and Resonant independently against their mode-specific
  responsibilities, plus qualitative confirmation that both are intentional Water models rather than a good/bad
  switch; no perceptual-distance metric, classification threshold, or mode-separation score is required;
- source recognizability at normal settings, including representative `global.mix=100%` Water-only evaluation;
- Size and Motion semantic consistency across both modes, with Motion shown not to act primarily as gain;
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
