# EXP-W-BA-001 — Bubble A1 independent population

Implementation status: IMPLEMENTED (offline research). Human acceptance: NOT ASSESSED.
Product adoption: NOT ADOPTED. This is the canonical model/parameter/module contract;
commands, measurements, failures and review history belong to the
[execution record](../../docs/evidence/WATER_BUBBLE_A1_EXECUTION.md).
The accepted [Revision B definition](EXP-W-001_PERCEPTUAL_BRIEF.md) supplies the
positive/negative/must-preserve/reject conditions, not DSP approval. Follow
[physical-model governance](../../docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md).
ADR-0006 stays Proposed; Legacy Preview/session v5 remains A0/B0/D0.
The separately authorized [Preview bridge](../../docs/evidence/WATER_PREVIEW_A1_B1_D1_BRIDGE.md)
consumes typed defaults in runtime Reworked mode; no new mapping or raw parameter UI. No Host/state/production adoption.

## Scope and model limits

A1 is an independent, approximately spherical, small-oscillation bubble population.
Reference pressure101325 Pa, water density998 kg/m3 and gas exponent1.4 are fixed.
No surface-tension frequency correction, hydrostatic depth, measured inward fluid
velocity, nonlinear radial motion, geometry, boundary transfer, physical entrainment,
splitting/merging/popping, coupled-cloud modes or airborne radiation is reconstructed.
Large radii remain research hypotheses, not proof that real large bubbles stay spherical.
Relative oscillator samples are neither calibrated Pa/SPL nor a microphone prediction.
B1 is a [separate impact model](EXP-W-DB-001.md); sharing reference equations does
not merge A1 population scheduling with B1 onset/admission/queue responsibilities.

## Primary-source audit and limits

Historical primary audit: 2026-09-24. The 2026-09-25 governance audit revisited the
2009/2016/2023 author abstracts and Phillips publisher article. Current van den Doel
PDF retrieval timed out; prior full-text verification below is retained as historical
evidence, not claimed as a fresh successful download. No new coefficients are inferred.


- [van den Doel, Physically-based Models for Liquid Sounds, author manuscript](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf),
  sections 3–5: isolated volume oscillation, inverse-radius frequency, radius-dependent
  damping, formation amplitude scaling, stochastic log-radius populations and selective
  rise. The 50-bin example is not a requirement for 128 bins. Depth is a lumped
  attenuation/formation proxy, not a geometric depth reconstruction. This is a hybrid
  physical/empirical model, not a fluid solver.
- [Minnaert 1933, original DOI](https://doi.org/10.1080/14786443309462277):
  original full text could not be retrieved; do not claim it was read. The frequency
  relation is cross-checked through the above original synthesis paper and the
  author-hosted modern bubble literature, with constants explicitly stated below.
- [Zheng and James 2009, Harmonic Fluids](https://www.cs.cornell.edu/projects/HarmonicFluids/):
  oscillator superposition also needs fluid animation and acoustic transfer for its
  geometric result. A1 implements neither geometry nor bubble-to-ear transfer.
- [Langlois, Zheng and James 2016](https://graphics.stanford.edu/~djames/publication/toward-animating-water-with-complex-acoustic-bubbles/):
  shape/proximity, entrainment, splitting, merging and popping require additional
  mechanisms. Their presence in this paper does not make them present in A1.
- [Xue et al. 2023, Coupled Bubbles](https://graphics.stanford.edu/papers/coupledbubbles/):
  collective coupled dynamics are outside this independent population approximation;
  low-frequency bubble-cloud behavior cannot be claimed from A1.
- NOMN FOAM: official site retrieval failed. Product listings are not physical evidence;
  no advertised feature or voice count is used as validation of this implementation.
- [Phillips, Agarwal and Jordan 2018, Scientific Reports 8:9515](https://www.nature.com/articles/s41598-018-27913-0):
  original high-speed-video/acoustic experiment, not a synthesis product. The described
  example has a final bubble diameter0.71 mm; the recorded dominant frequency is8.66 kHz.
  A1's stated free-field constants predict about9.26 kHz at radius0.355 mm (our inference,
  about7% higher, not an exact experimental fit). This supports the frequency scale and
  mechanism, not calibrated reproduction. The paper also finds airborne radiation is
  not explained by simply transmitting the underwater field through the interface;
  its proposed surface-motion mechanism is not implemented by A1. Published measurements
  remain external evidence, distinct from FRAZIL's synthetic property tests.

## Equation → code → independent test ownership

Each row has exactly one primary class. All code paths are relative to
`SPIKE-W-DSP-001/`; test functions/sections are in `tests/bubble_a1_tests.cpp` unless
specified. IMPLEMENTED describes mechanism availability only; RESEARCH-CANDIDATE
marks unaccepted product curves; DEFERRED means absent. Neither implies acceptance.

| ID / mechanism | Equation / boundary | Primary classification | Source | Code owner | Independent test | Status |
| --- | --- | --- | --- | --- | --- | --- |
| A1-PHY-001 frequency | sqrt(3*kappa*P/rho)/(2*pi*R), SI radius | PHYSICAL | Minnaert via van den Doel / Phillips eq2 | physics/BubblePhysics; BubbleA1Model | fixed SI values, radius monotonicity | IMPLEMENTED |
| A1-PHY-002 damping | .13/R+.0072/R^1.5, per second; empirical fit above .15mm | PHYSICAL | van den Doel eq3 | physics/BubblePhysics | independent damping numeric oracle | IMPLEMENTED |
| A1-PHY-003 reference amplitude exponent | R^1.5 under radius-independent inward velocity assumption | PHYSICAL | van den Doel eq4/6 | BubbleA1Model, alpha=1.5 reference | adjacent-radius amplitude ratios | IMPLEMENTED |
| A1-RED-001 adjustable exponent | (R/Rref)^alpha, alpha .75..2.25 | REDUCED_PHYSICAL_MODEL | project extension of reference scaling | BubbleA1Model | alpha corners, global amplitude bound | IMPLEMENTED |
| A1-ENG-001 reference radius | Rref=radiusMin; dimensional reference cancels in normalization | ENGINEERING | project relative calibration | BubbleA1Model | endpoint / amplitude-ratio oracle | IMPLEMENTED |
| A1-ENG-002 amplitude normalization | divide by sqrt(sum(p_i*a_i^2)) | ENGINEERING | project second-moment calibration | BubbleA1Model | weighted initial squared moment=1 | IMPLEMENTED |
| A1-RED-002 population | p_i proportional R_i^-gamma per log bin; gamma2 is not measured | REDUCED_PHYSICAL_MODEL | van den Doel population hypothesis | BubbleA1Model | flat/power-law histograms, six-sigma | IMPLEMENTED |
| A1-ENG-003 discretization | 128 log-radius bins, binary inverse CDF | ENGINEERING | project resolution; paper example50 | BubbleA1Model | log spacing, endpoints, CDF | IMPLEMENTED |
| A1-RED-003 stochastic arrivals | audio-driven independent event population concept | REDUCED_PHYSICAL_MODEL | van den Doel, project source substitution | BubbleA1 | source gating / shared request identity | IMPLEMENTED |
| A1-ENG-004 discrete scheduler | Bernoulli p=-expm1(-lambda/fs); mean fs*p, at most one request/sample | ENGINEERING | occupancy approximation | BubbleA1 | stationary mean six-sigma | IMPLEMENTED |
| A1-RED-004 formation/depth proxy | D=U^beta; beta10 not measured; not meters | REDUCED_PHYSICAL_MODEL | van den Doel lumped proxy | BubbleA1 | captured proxy / seed / stereo identity | IMPLEMENTED |
| A1-RED-005 P0 rise | f=f0*(1+xi*dPhysical*t), selected when D>cutoff | REDUCED_PHYSICAL_MODEL | van den Doel selective-rise cue | BubbleA1Voice | analytic integrated chirp, persistence .25/1/4 | IMPLEMENTED |
| A1-PROD-001 P1 rise | f=f0*(1+xi*dEffective*t), unchanged default | PRODUCT_MAPPING | project persistence-relative stylization | BubbleA1Voice | P1 rise-per-lifetime=xi, analytic chirp | IMPLEMENTED |
| A1-RED-006 surface cue cap | sqrt(2)*f0 | REDUCED_PHYSICAL_MODEL | simplified surface-limit cue | BubbleA1Voice | capped analytic trajectory | IMPLEMENTED |
| A1-ENG-005 bandwidth guard | .45*fs cap | ENGINEERING | numeric margin, no alias-free claim | BubbleA1Voice | supported-bin bounds / rate matrix | IMPLEMENTED |
| A1-ENG-006 analysis | linked power, AR, 2ms window, same-frame joint peak direction | ENGINEERING | project signal analysis/calibration | SharedExcitationAnalyzer | independent eight-scenario frame oracle | IMPLEMENTED |
| A1-RED-007 source substitution | signed direction times sqrt(fastPower), or .25 ablation | REDUCED_PHYSICAL_MODEL | audio proxy for unknown forcing | BubbleA1 / analyzer | carrier oracle / zero-crossing / isolation | IMPLEMENTED |
| A1-PROD-002 persistence | tau=persistenceScale/dPhysical | PRODUCT_MAPPING | explicit nonphysical scaling | BubbleA1Model | reciprocal damping and lifetime tests | IMPLEMENTED |
| A1-PROD-003 offline macros | Rmin=.2*10^Size; Rmax=2*25^Size mm; Motion=m^2; persistence=4^(2*d-1) | PRODUCT_MAPPING | project curves, not physics | mapBubbleA1 / research_cases | macro endpoints / invalid values | RESEARCH-CANDIDATE |
| A1-ENG-007 output gain | residualGain; no capacity divisor / limiter | ENGINEERING | relative calibration | BubbleA1 | exact capacity-independent single-event waveform | IMPLEMENTED |
| A1-ENG-008 oscillator | complex recurrence, midpoint phase integration | ENGINEERING | discretization of damped oscillator | BubbleA1Voice | analytic waveform / cap oracle | IMPLEMENTED |
| A1-ENG-009 resources | ceilings64/128/256/512/1024; active/free arrays | ENGINEERING | bounded storage/work | BubbleA1VoicePool | fill/downshift/extremes/reset | IMPLEMENTED |
| A1-ENG-010 retirement | absolute envelope floor; unchanged30s backstop | ENGINEERING | resource lifecycle, not physical extinction | BubbleA1Voice | independent global bound and natural retirement | IMPLEMENTED |
| A1-ENG-011 stealing | least envelope-weighted amplitude, lowest-slot tie; linear release then replacement | ENGINEERING | bounded deterministic policy | BubbleA1VoicePool | release/start timing and capacity drops | IMPLEMENTED |
| A1-ENG-012 random identity | named bubbleA1 domain6, unchanged draw order | ENGINEERING | deterministic research isolation | WaterDspConfig / BubbleA1 | all nine numeric domains / seed equality | IMPLEMENTED |
| A1-ENG-013 descriptors | strict numeric config v2; 22 writable fields | ENGINEERING | research tooling contract | ConfigSpec / ReadConfig / Descriptor | actual CLI snapshot and invalid bounds | IMPLEMENTED |
| A1-GAP-001 geometry/topology/transfer | no shape, splitting, merging, popping or bubble-to-ear solver | PHYSICAL | Harmonic Fluids / Complex Bubbles | none | none | DEFERRED |
| A1-GAP-002 coupling | no collective cloud dynamics | PHYSICAL | Coupled Bubbles | none | none | DEFERRED |
| A1-GAP-003 radiation | no surface-driven airborne field | PHYSICAL | Phillips2018 | none | none | DEFERRED |

The second-moment normalization controls expected **initial squared amplitude** only.
It is not physical energy conservation, duration-integrated energy equality,
pressure calibration or perceptual loudness matching. Stochastic gamma/beta choices,
source substitution and P1 remain hypotheses even with exact numerical regressions.

## Shared excitation and lifecycle contract

One linked 2ms window clamps analysis channels to [-1,1]; nonfinite analysis becomes
zero. The offline renderer rejects nonfinite/over-full-scale inputs. Audio itself is
not clipped by this analysis. Window power is mean((L²+R²)/2). Fast/slow power
followers use `next=power+exp(-1000/(fs*timeMs))*(old-power)`, choosing attack or
release from the comparison with old power; values below1e-25 become zero.
Let `floorPower=10^(activityFloorDbFS/10)` and
`kneePower=floorPower*10^(activityKneeDb/10)`. Activity is zero when window power
is at or below the floor; otherwise it is `clamp((fastPower-floorPower)/(kneePower-floorPower),0,1)
*sqrt(slowPower)`. Requested `lambda=maxEventRateHz*motionFactor*activity`.
The follower/gate are ENGINEERING; their audio-to-arrival interpretation is REDUCED,
and motionFactor is PRODUCT_MAPPING. These coefficients are not fluid measurements.
Select ONE window frame maximizing L²+R²; ties retain
lowest physical ring index. Divide its signed vector by sqrt((L²+R²)/2), with a
1e-12 norm floor. Multiply by sqrt(linked fast AR power), or .25 in the fixed-level
ablation. No independent L/R event, phase or RNG; no promise of preserved perceived
width or wideband correlation. Shared analysis has no A1 model dependency.

Activity is zero when the source window is below its floor. It may remain active
briefly after the current input sample becomes zero. Source linkage therefore must
not be judged by the current sample alone. One scheduler draw runs every sample;
radius and formation draws run on requests only. Motion0 stops new requests;
already accepted replacement events and existing tails finish. No callback reprepare.

The pool accepts immediately into a free slot or reserves a releasing replacement.
All-releasing requests drop. Capacity downshift preserves existing tails and can
cancel pending starts that cannot fit. Accepted and later dropped counters are
transitions, not mutually exclusive totals. Completed-lifetime bins include stolen
tails, and report approximate lower edges. No UI transport for these histograms or growing event log.

Offline diagnostic definitions:

- `a1_requested_frame`: first requested sample index, -1 if absent.
- `a1_started_frame`: first voice initialization sample index, -1 if absent. A stolen
  replacement initializes after its outgoing voice step and emits next sample.
- `a1_start_on_zero_current_frame`: number of starts while the current stereo frame
  equals zero; it does **not** count source-window-independent events.
- `a1_source_window_active`: number of processed frames with positive source activity,
  not a boolean or an event count. Delayed replacement starts use captured excitation.
- `bubble_silent_events` and `bubble_first_frame`: retained deprecated aliases for
  zero-current-frame starts and first start in A1 modes; no spontaneous-event inference.

### Independent30s guard bound

For every legal radius range, Rmax/Rmin<=250 and alpha<=2.25. Unnormalized amplitude
is at least1, so its probability-weighted second-moment normalization is at least1
for every legal gamma. The signed linked carrier has magnitude<=sqrt(2); gain and
formation proxy are<=1. Thus per-channel initial envelope A<=sqrt(2)*250^2.25.
Physical damping decreases with R, so tau<=4/(.13/.05+.0072/.05^1.5).
The lowest absolute floor is10^(-100/20)=1e-5. Consequently
`t <= tauMax*ln(Amax/1e-5) = 29.9418977911 seconds`.
Two samples at44.1kHz still leave over58ms before30s. Lower gains, larger floors,
smaller persistence and stealing only shorten lifetime. Numerical recurrence is
separately checked with the conservative adversarial envelope at all three rates;
this is a single-voice retirement bound, not a bound on summed output loudness.
The guard stays30s; it is not a sound-design decay control.

## Typed research parameter authority

`dsp/BubbleA1ConfigSpec.h` owns A1 specs; `dsp/SharedExcitationConfig.h` owns the six
shared analysis specs independently of A1. Immutable `ResearchParameterSpec` values
supply defaults, range/choice validation, parser field names and the offline descriptor.
`--describe-bubble-a1` reports model `bubble-a1`, modelVersion2/configVersion2.
The [tracked snapshot](contracts/bubble-a1-v2.json) is checked against the running
renderer in CTest. This table is a review mirror, not an independent numeric authority.
Bounds are research limits; a PHYSICAL radius quantity does not make its chosen
interval a measured population. Adjustable alpha is REDUCED; only alpha1.5 has the
reference physical scaling interpretation.

| Key | Unit | Primary classification | Range / choices | Default |
| --- | --- | --- | --- | --- |
| `radiusMinMm` | mm | PHYSICAL | 0.2..10 | 0.2 |
| `radiusMaxMm` | mm | PHYSICAL | 2..50 | 10 |
| `populationGamma` | dimensionless | REDUCED_PHYSICAL_MODEL | 0..6 | 2 |
| `amplitudeRadiusExponent` | dimensionless | REDUCED_PHYSICAL_MODEL | 0.75..2.25 | 1.5 |
| `depthExponent` | dimensionless | REDUCED_PHYSICAL_MODEL | 1..16 | 10 |
| `persistenceScale` | dimensionless | PRODUCT_MAPPING | 0.25..4 | 1 |
| `maxEventRateHz` | Hz | REDUCED_PHYSICAL_MODEL | 0..10000 | 1000 |
| `motionFactor` | dimensionless | PRODUCT_MAPPING | 0..1 | 1 |
| `riseXi` | dimensionless | REDUCED_PHYSICAL_MODEL | 0..0.2 | 0.1 |
| `riseCutoff` | dimensionless proxy | REDUCED_PHYSICAL_MODEL | 0.8..1 | 0.9 |
| `tailFloorDb` | dB relative amplitude | ENGINEERING | -100..-60 | -80 |
| `stealReleaseMs` | ms | ENGINEERING | 0.5..4 | 1.5 |
| `residualGain` | linear gain | ENGINEERING | 0..1 | 0.2 |
| `voiceCapacity` | voices | ENGINEERING | 64, 128, 256, 512, 1024 | 256 |
| `riseModel` | choice | PRODUCT_MAPPING | 0, 1 | 1 |
| `sourceEnergyAmplitude` | choice | REDUCED_PHYSICAL_MODEL | 0, 1 | 1 |
| `fastAttackMs` | ms | ENGINEERING | 0.5..5 | 1 |
| `fastReleaseMs` | ms | ENGINEERING | 10..80 | 30 |
| `slowAttackMs` | ms | ENGINEERING | 10..80 | 30 |
| `slowReleaseMs` | ms | ENGINEERING | 80..500 | 200 |
| `activityFloorDbFS` | dBFS | ENGINEERING | -80..-40 | -60 |
| `activityKneeDb` | dB | ENGINEERING | 3..12 | 6 |

Coupled constraint: radiusMinMm < radiusMaxMm. Supplied `bubbleA1` objects require
numeric `version:2` (a schema marker, not a writable DSP parameter). Missing/v1
versions, old `riseFactor`, unknown/derived fields and invalid types/ranges reject.
P0 is the corrected shared-frame variant, not reconstruction of the old stereo defect.
Exact historical v1 belongs to `1bc69476251851363189817a42735e62e8987916`.

## Baseline identity and integration boundaries

| Name | Radius mm | Motion | Persistence | Meaning |
| --- | --- | --- | --- | --- |
| A1-PHYS-REF | .2..10 | 1 | 1 | Historical raw reference; gamma2, alpha1.5, beta10, xi.1, cutoff.9, rate1000, gain.2, cap256, P1 |
| A1-MACRO-NEUTRAL | .632455532..10 | .25 | 1 | Offline Size/Motion/Decay=.5/.5/.5; other reference fields retained |

**A1-PHYS-REF is a historical name, not a wholly first-principles pipeline.** It
includes unmeasured gamma/beta, audio forcing substitution,128-bin discretization,
second-moment normalization, Bernoulli scheduling, finite resources, relative gain
and the product-mapped P1 slope. Do not rename or retune it during governance work.
The macro neutral is intentionally different. Neither is an accepted product default.

Explicit a1-residual returns E; a1 returns x+E; a1b/a1d/a1bd replace only A in legacy
composition. B1 combinations are separate explicit modes. A1 rejects enabled Protect;
there is no hidden normalization/limiter. Other modes/Preview reject bubbleA1 fields.
Omitted configuration never upgrades legacy a to A1. Nine Host parameters, schema1,
Preview sessionv5, legacy DSP and production paths remain outside this contract.

Engineering validation, current timing CSV, exact before/after comparisons, historical
A0 gap analysis and regeneration commands live only in the execution record. Human
Water identity, source preservation, stereo width, audibility and useful mappings
remain NOT ASSESSED. No voice count proves coupling or low-frequency cloud behavior.
Stop at governance handoff; no A2/D1/C1/UI work follows without separate instruction.
