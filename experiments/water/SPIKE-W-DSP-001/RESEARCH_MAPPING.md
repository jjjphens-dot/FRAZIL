# Research Water Mapping v0.2

Flow D1 has no product macro mapping. Its independent offline version1 accepts
virtual velocity, structure length and excess path only; Size/Decay and the existing
v0.2/sessionv5 destinations stay unchanged. U0 is exact identity, not a decision
about A1/B1 minimum-Motion admission. See [D1 contract](../EXP-W-FD-001.md).

B1 uses separate `droplet-b1-offline-v1` Size/Decay candidates documented in [EXP-W-DB-001](../EXP-W-DB-001.md). Its core accepts radius/persistence/admission, not product IDs. M0-A strict calm and M0-B onset-preserving calm remain unaccepted comparisons; no Motion curve is adopted. This v0.2 mapping and session v5 remain unchanged.

[Bubble A1](../EXP-W-BA-001.md) has a separate explicit `bubble-a1-offline-v2` exporter
candidate. It does not replace these curves or migrate session v5. UI integration is deferred.

Status: research listening candidate; NOT PRODUCT FROZEN. No production parameter, Host state,
algorithm adoption or perceptual acceptance is implied. Implementation lives only in the opt-in
research target. See the [current handoff](../../../docs/evidence/WATER_LISTENING_HANDOFF_V02.md) and
[v0.2 remediation evidence](../../../docs/evidence/WATER_LISTENING_REMEDIATION.md).

`ResearchWaterMacroMapper` takes a plain `WaterExperimentState` and returns numeric Fluid/Resonant
targets without JUCE, allocation, UI or DSP objects. `ResearchMappingAdapter` owns the explicit
engineering destination table. Session commands apply that table on the message thread.

| Macro | Candidate destinations and formula (normalized s/m/d) |
|---|---|
| Size | scale = 2^(1-2s); Bubble min/max = 250/2800 * scale Hz; Droplet = 600/4500 * scale Hz; Modal root = 260 * scale Hz |
| Fluid Motion | Bubble max rate = 480*m*m /s; Droplet new events enabled iff m>0; Droplet threshold = .015 * 4^(1-2m); refractory = .020 * 4^(1-2m) s; Flow interval = .250 * 4^(1-2m) s; depth = .0005+.001m s |
| Decay | Bubble = .070 * 3.5^(2d-1) s; Droplet = .012 * 3^(2d-1) s; Modal = .120 * 4^(2d-1) s |

Macro movement in a v0.2 session reclaims only its own targets. Manual raw edits mark the owning
macro CUSTOM without inventing a reverse value. Return Size/Motion/Decay overwrites only that
macro's destinations, including inactive module values; Return All reclaims the three macro
families but preserves separate Model/composition, gains, voice counts, Bubble sensitivity,
Flow base delay and Protect. Baseline Bubble sensitivity remains .0001 and Flow base remains 4 ms;
raw CUSTOM changes to these unowned controls remain subject to the existing coupled DSP validator.

v1 sessions remain `legacy-unmapped`: moving macros changes research context only. The explicit
Adopt Research Mapping v0.2 button maps all three macros, or a per-macro Return explicitly adopts
that family. Other CUSTOM families remain unchanged. v5 checks mapped target consistency as well
as known revision, typed values, source rate and retained Protect state. No history enters JSON.

v2 / v0.1 imports preserve raw values, append the legacy Droplet gate default ON, and become
`legacy-research-v0.1` / CUSTOM. v3 exports carry the 24th explicit scheduling target; unknown
revisions reject. The renderer still defaults omitted `droplet.eventsEnabled` to 1.

Motion=0 schedules no new A/B events. The Droplet audio-owner gate preserves active voices;
macro gestures still follow the stopped prepare/restart audition workflow.
Known limits: Flow is delayed-minus-source and can sound
like comb filtering/chorus/flanging. Mapping direction and audibility require Sound Lead review;
objective monotonicity is not perceptual acceptance. Resonant temporal motion uses depth .35m and interval .7 * 2.8^(1-2m) s
to redistribute excitation over six fixed modes. Both destinations belong to Motion; changing
them manually marks Motion CUSTOM. Typed default depth=0 preserves legacy renderer behavior.

## Independent listening calibration

New research sessions and Reset Baseline use **Research Listening Calibration v0.1**:
A/B/D/C residual gain = .26/.24/.06/.30. This is a listening starting point to reduce D dominance;
it is not product balance or a loudness claim. Typed renderer defaults remain unchanged.

Gains have calibration ownership, outside Size/Motion/Decay. Raw gain edits mark calibration
CUSTOM without changing macro states; macro movement and Return All leave those gains intact.
Engineering's Restore Listening Calibration changes only these four gains in one operation.
Session v5 stores calibration status and rejects a mapped claim whose gains disagree; legacy v2
also carries calibration provenance. v1/module
imports preserve their raw gains and mark calibration CUSTOM.

## Current readiness boundary

C0 modal normalization (`b = 1-r`) remains unchanged. Unmodified C1/C2 failed the finite-float
boundary study and were not adopted. Resonant is **not fully listening-ready**; current Motion
and Decay measurements do not establish clearly audible macro semantics. The next bounded
excitation experiment is tracked separately in the [phase record](../../../docs/evidence/WATER_DSP_LISTENING_EXECUTION.md).

## Separate continuous Droplet activity candidate

Session v4 adds unowned `droplet.eventActivity` (default 1), preserving v0.2 macro destinations.
v1/v2/v3 import fills the legacy probability 1. The pure mapper's explicit candidate helper
returns clamp(4*m*m,0,1); the cases exporter applies it only with `--continuous-droplet` and labels
that candidate in its manifest. The normal preview mapper does not adopt it. Probability affects
otherwise-valid event scheduling only; frequency/Decay/gain and threshold eligibility are retained.

## Explicit C comparison options (session v5)

Three additional unowned fields select excitation, normalization and Motion model. Default
Raw/C0/independent remains. Engineering Hard/C3/structured and Feature/C3/structured require
Apply; they retain the v0.2 Size/Motion/Decay targets and independent calibration. v1-v4 imports
fill default C options, preserving previous engineering values. Diagnostic monitor selection
is temporary and never becomes a mapped field or saved session target.

## Bubble A1 baseline identity

A1-PHYS-REF is raw .2..10mm, Motion factor1, persistence1.
A1-MACRO-NEUTRAL is Size/Motion/Decay=.5/.5/.5 -> .632455532..10mm,
Motion factor.25, persistence1. They are not interchangeable defaults. A1 config
requires version2, uses P1 effective-damping rise by default, and permits explicit
P0 offline comparison. Legacyv1 exact reproduction stays at1bc6947.
See EXP-W-BA-001 for the canonical provenance/ranges; Preview mappingv0.2 remains A0.
