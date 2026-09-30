# EXP-W-DB-002 — Droplet / Impact B2

Status: RESEARCH-CANDIDATE; engineering implementation in progress; human acceptance
NOT ASSESSED; NOT PRODUCT DEFAULT / NOT ADOPTED. Engineering agent implements and
self-reviews; Sound Lead owns ACCEPT / REVISE / REJECT. Authority: user Round 01
task, [accepted Water intent](EXP-W-001_PERCEPTUAL_BRIEF.md) and
[physical-model governance](../../docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md).
B1 remains the preserved fixed-radius baseline. No production, Host, session v5,
Ice, routing or macro mapping changes are authorized by this contract.

## Contract review and primary sources

Preserve source identity, principal attacks/rhythm and bass/low-mid character.
Intrusive fixed pitch, detached Foley, clutter, double attack, metallic/hollow,
chorus/flanger, roughness and seasick pitch remain listening rejection questions.
Objective metrics are proxies, never a total quality score.

| Source / current access | SOURCE SUPPORTS | SOURCE DOES NOT SUPPORT |
| --- | --- | --- |
| [Phillips et al. 2018](https://doi.org/10.1038/s41598-018-27913-0), Cambridge repository indexed manuscript; direct retrieval currently failed | Entrapped bubble resonance is central to the typical dripping plink; radius relates to natural frequency | Constant 2 mm on every musical attack, audio amplitude as radius, stereo detune |
| [Pumphrey & Elmore 1990](https://doi.org/10.1017/S0022112090003378), publisher abstract retrieved | Distinct regular, irregular, large-bubble and Mesler entrainment regimes | This bounded log-radius distribution or its percentages |
| [Oguz & Prosperetti 1990](https://pages.jh.edu/aprospe1/publications/PapersPublished/AirEntrainment/DropImpactJfm.pdf), author manuscript indexed | Impact/cavity dynamics and fluid state govern entrainment | An audio onset is a measured impact velocity; this implementation is CFD |
| [van den Doel 2005](https://doi.org/10.1145/1101530.1101554), prior project equation audit retained; current author PDF retrieval failed | Isolated bubble synthesis and stochastic populations for complex liquid sound | A1 gamma/Rmin are optimal or the B2 radius distribution is measured |
| [Bello et al. 2005](https://doi.org/10.1109/TSA.2005.851998), university manuscript indexed; direct retrieval failed | Energy, spectral and phase features offer different onset definitions | Our slope threshold detects physical drops or is optimal for every source |
| [Laakso et al. 1996](https://doi.org/10.1109/79.482137), author university bibliographic record / existing FD-003 audit | Fractional delay is an interpolation problem with FIR/allpass tradeoffs | Automatic adoption of any FD-003 backend |
| [Ross et al. 2014](https://journals.physiology.org/doi/abs/10.1152/jn.00224.2014), publisher indexed results | Frequency differences can alter binaural motion/beat perception, with carrier-frequency limits (response at 400 Hz but not 3200 Hz in that experiment) | Natural water stereo physics, guaranteed width at bubble frequencies, an optimal half-cent or 2 Hz bound |

Current retrieval limits are not represented as fresh full-text equation verification.
Unchanged physical equations reuse audited BubblePhysics; new numeric choices are
explicit project hypotheses.

## Model / ownership / independent oracle

| ID | Contract, units and bounds | Primary classification | Implementation owner | Independent oracle |
| --- | --- | --- | --- | --- |
| B2-PHY-01 | f0=sqrt(3*kappa*P/rho)/(2*pi*R); R in meters; same fixed reference constants as B1 | PHYSICAL | BubblePhysics | Analytic inverse-radius ratios / B1 baseline |
| B2-PHY-02 | d=.13/R+.0072/R^1.5, 1/s; empirical fit, small isolated spherical bubble approximation | PHYSICAL | BubblePhysics | Analytic formula / monotonicity |
| B2-RED-01 | R=clamp(Rcenter*exp(u*log(1+spread/100)),.2,7) mm, u uniform [-1,1]; one draw per admitted event | REDUCED_PHYSICAL_MODEL | B2 radius generator | Endpoints, bounds, reset and unrelated-domain invariance |
| B2-ENG-01 | Ratio or positive fast-power log derivative (dB/ms), source floor, spacing, hysteresis/rearm | ENGINEERING | B2 detector | Synthetic 8/10/20/40/80 ms transients, silence and steady signals |
| B2-PROD-01 | E_mapped=clamp(E_source,0,1)^gamma; gamma=1 identity | PRODUCT_MAPPING | B2 impact capture | Monotonicity, eligible/admission identity, fixed radius baseline |
| B2-PROD-02 | fL=fcenter*2^(-s*delta/1200), fR=fcenter*2^(s*delta/1200); one event polarity s | PRODUCT_MAPPING | B2 spatial voice | Geometric center, zero detune identity, mono/stereo proxies |
| B2-ENG-02 | Maximum L/R difference 2 Hz, no periodic/per-sample random modulation | ENGINEERING | B2 spatial renderer | All legal radii and rise trajectories satisfy difference bound |
| B2-ENG-03 | Queue16, voice256 storage, bounded release replacement; no process allocation/I/O/lock | ENGINEERING | B2 queue/pool | Allocation observer, saturation/reset/partition/tail tests |

Radius varies between events, never within an event. Missing physical inputs include
drop velocity/geometry, cavity state, depth and measured forcing. Source excitation
is a dimensionless surrogate. Neither channel represents a second physical bubble.
Center emission retains the B1 small-oscillation/relative volume-acceleration relation;
detuned rendering is a product presentation, not pressure/SPL prediction.

Stereo cents are fixed at event start using the maximum frequency of the unchanged
center rise trajectory. This conservative bound keeps the L/R difference below the
configured cap throughout the event without adding dynamic detune or an LFO.

Research initial values: radius spread 2.5%, source gamma .75, hybrid detector,
half-cent per-channel detune, maximum beat 2 Hz. These are NOT PRODUCT DEFAULTS.
Retain 0/1/2.5/5% radius, 1/.75/.5 gamma and 0/.25/.5/1 cent comparisons, raw and
normalized amplitude policies. Ratio-only + zero spread + gamma1 + zero detune is
the B1-like ablation; exact compatibility must be measured, not inferred.

## Integration and evidence gates

Independent `dropletB2` version1 schema and b2/b2-residual/a1b2/b2d1/a1b2d1 modes.
Legacy b/bd/abd and explicit B1 modes retain their meaning. Preview revision/config
participates in Draft, Applied, A/B, history and reset, never session v5 or Host state.
Radius/spatial RNG domains are appended; legacy domains and draws stay unchanged.

Listening: B1, B2-R, B2-D, B2-C, B2-S, B2-RS and B2-FULL, with same source/rate/seed,
monitor mode/output and E Trim. Save config and event traces. Separate fixed-scale
preservation and RMS-matched preference. Require headphone, speaker and mono review;
L/R correlation, M/S energy and mono delta do not decide width quality.
Execution and unresolved findings: [B2 execution](../../docs/evidence/WATER_DROPLET_B2_EXECUTION.md).
