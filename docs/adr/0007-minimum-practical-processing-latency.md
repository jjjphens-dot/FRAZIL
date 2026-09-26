# ADR-0007: Minimum practical declared processing latency

- Status: Proposed — user-authorized policy revision; independent Joint Gate pending.
- Date: 2026-09-26
- Work item: ARCH-LAT-002 / EXP-W-FD-001 low-latency research.
- Supersedes: [ADR-0005](0005-zero-sample-processing-latency.md) **upon acceptance**.
- Implementation DRI: Engineering Lead; acceptance: Engineering + Sound/Host Joint Gate.

## Context

The current plugin and its accepted M1 evidence report zero samples. D1's immediate
causal interpolation fails source-aware numerical fidelity. The user explicitly
authorizes studying deterministic guard latency and preparing a replacement policy.
Changing an Accepted Decision requires a new ADR; this proposal does not fabricate
independent approval or make the present plugin non-zero-latency.

## Proposed decision

Physical/numerical fidelity takes precedence over absolute zero latency. Select the
smallest measured processing latency that satisfies all fidelity and stability gates.
For the first D1 study, guards are limited to 0/8/16/32/64 samples at 44.1/48/96 kHz;
report samples and milliseconds at each rate. Failure at 64 returns to model review.
Filter latency must be counted separately and included in the total comparison.

Physical excess delay is tau=s/c. Guard Lg is ENGINEERING ONLY numerical support,
in samples; never convert Lg into an asserted source/listener distance. Audible-band
choice is PRODUCT_MAPPING research; its filter implementation is ENGINEERING.
Neither may retune source radius, trajectory, velocity, Size, Motion or Decay.

Future production adoption requires:

- Deterministic, explicitly measured processing latency, preferably instance-fixed;
  no sample-wise changes or changes from normal parameter automation.
- DSP stages declare their processing latency without depending on Host objects.
  AudioEngine/routing owns alignment; PluginProcessor alone reports total latency.
- Serial stage processing latencies add. Parallel latency is the maximum branch
  latency after branch compensation. Global latency covers the longest permitted
  active topology, not merely the maximum individual stage. A fixed instance budget
  can pad shorter topologies; its value must follow measured accepted algorithms.
- Dry, carrier, wet, stage mix, bypass and parallel branches share an engineering
  time reference. Physical transfer/tail remains in the wet signal, uncompensated.
- ROUTE-011 prohibits undeclared/uncompensated infrastructure latency; zero is a
  current-artifact baseline, not the future universal acceptance condition.
- Prepare owns fixed-capacity history. Process remains allocation/lock/I/O free.
  Reset, drain, mode transitions and failure states have explicit timing contracts.
- Host metadata, DAW PDC, bypass, offline render and topology changes must be tested
  on the actual adopted artifact before any support claim.

## Compatibility and migration

No Host ID/order/range/default, schemaVersion=1 or production code changes here.
The present plugin retains ADR-0005 tests until an accepted implementation replaces
them with declared-latency alignment tests. Old sessions currently have zero delay;
an adopted non-zero algorithm changes render timing and requires explicit release/
session compatibility review, version identification and before/after evidence.
Research flow-d1-v1 is unfrozen but must not be silently replaced: record old/new
kernel, numericalRevision, latencyRevision and model/config compatibility analysis.
No new writable latency parameter is authorized.

## Consequences and alternatives

Guard support permits centred interpolation but introduces latency, finite impulse
support and possible transient modification. Linear-phase conditioning adds its own
delay/pre-ringing; minimum-phase IIR adds dispersion without lookahead. Neither is
automatically preferable. No filter may hide failure inside the protected band.
Zero-guard candidates remain measured controls. Analytic per-emitter retarded-state
evaluation is a future alternative requiring a separate source-interface contract.

## Verification and activation

The [preregistered D1 study](../../experiments/water/EXP-W-FD-002.md) supplies offline
evidence. Numerical success alone cannot activate this ADR or replace runtime:
native resource evidence, full Debug/Release/ASAN, historical exact preservation,
independent review and human listening remain required. The current native faults
D1-VAL-001/002 remain separate findings until evidence supports their disposition.
Independent Joint Gate reviewer/date/decision: **NOT RECORDED**.
