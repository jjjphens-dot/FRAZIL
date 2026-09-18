# Research Water Mapping v0.1

Status: research listening candidate; NOT PRODUCT FROZEN. No production parameter, Host state,
algorithm adoption or perceptual acceptance is implied. Implementation lives only in the opt-in
research target. See [execution evidence](../../../docs/evidence/WATER_LISTENING_UI_EXECUTION.md).

`ResearchWaterMacroMapper` takes a plain `WaterExperimentState` and returns numeric Fluid/Resonant
targets without JUCE, allocation, UI or DSP objects. `ResearchMappingAdapter` owns the explicit
engineering destination table. Session commands apply that table on the message thread.

| Macro | Candidate destinations and formula (normalized s/m/d) |
|---|---|
| Size | scale = 2^(1-2s); Bubble min/max = 250/2800 * scale Hz; Droplet = 600/4500 * scale Hz; Modal root = 260 * scale Hz |
| Fluid Motion | Bubble max rate = 120 * 4^(2m-1) /s; Droplet threshold = .015 * 4^(1-2m); refractory = .020 * 4^(1-2m) s; Flow interval = .250 * 4^(1-2m) s; depth = .0005+.001m s |
| Decay | Bubble = .070 * 3.5^(2d-1) s; Droplet = .012 * 3^(2d-1) s; Modal = .120 * 4^(2d-1) s |

Macro movement in a v0.1 session reclaims only its own targets. Manual raw edits mark the owning
macro CUSTOM without inventing a reverse value. Return Size/Motion/Decay overwrites only that
macro's destinations, including inactive module values; Return All reclaims the three macro
families but preserves separate Model/composition, gains, voice counts, Bubble sensitivity,
Flow base delay and Protect. Baseline Bubble sensitivity remains .0001 and Flow base remains 4 ms;
raw CUSTOM changes to these unowned controls remain subject to the existing coupled DSP validator.

v1 sessions remain `legacy-unmapped`: moving macros changes research context only. The explicit
Adopt Research Mapping v0.1 button maps all three macros, or a per-macro Return explicitly adopts
that family. Other CUSTOM families remain unchanged. v2 checks mapped target consistency as well
as known revision, typed values, source rate and retained Protect state. No history enters JSON.

Known limits: minimum Fluid Motion still permits a 30/s maximum Bubble rate, so this candidate
does not establish HI-08 product-minimum compliance. Flow is delayed-minus-source and can sound
like comb filtering/chorus/flanging. Mapping direction and audibility require Sound Lead review;
objective monotonicity is not perceptual acceptance. Resonant temporal motion uses depth .35m and interval .7 * 2.8^(1-2m) s
to redistribute excitation over six fixed modes. Both destinations belong to Motion; changing
them manually marks Motion CUSTOM. Typed default depth=0 preserves legacy renderer behavior.
