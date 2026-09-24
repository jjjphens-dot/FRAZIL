# EXP-W-BA-001 — Bubble A1 independent population

Status: REMEDIATION LOCALLY VALIDATED / RESEARCH ONLY / HUMAN NOT ASSESSED. UI integration is deferred by
the current user instruction. Baseline: PR #40 `1444b4466a4bab8e1e60b59c399174d40f4f6491`.
The accepted Revision B perceptual definition supplies rejection criteria, not DSP approval.
ADR-0006 remains Proposed. No production or Host/state contract changes are authorized.

## Phase 0: A0 gap matrix (recorded before implementation)

Paths below are relative to `SPIKE-W-DSP-001/`.

| Concern | Actual A0 evidence | A1 requirement |
| --- | --- | --- |
| Radius | `dsp/BubbleEnsemble.h`, Hz-only config | Explicit meters internally; mm at offline boundary |
| Frequency | `detail/EventVoicePool.h`, 16 log frequency families | 128 log radius bins; Minnaert frequency |
| Damping | One `decaySeconds` shared by all families | Radius-dependent physical damping and separate persistence scale |
| Amplitude | `trigger(input, family)` clips trigger PCM | Short-window source energy, radius factor and depth proxy |
| Population | `nextUInt() % 16` | Normalized power-law probability per log-radius bin |
| Scheduler | `probability_ * feature.slow`; instantaneous threshold | Linked power activity; exponential occupancy probability |
| Pitch | Fixed complex poles | Optional selective, capped integrated-frequency rise |
| Capacity | Fixed 16 slots; configured 1..16 | Fixed 1024 storage; five discrete ceilings |
| Gain | `residualGain / capacity_` | Distribution normalization independent of voice capacity |
| Stereo | Shared events; separate resonator channel states | Keep event symmetry; remove trigger-zero dependence |
| Termination | Hard `ceil(24*tau*fs)` | Envelope floor and independent bounded lifetime guard |
| Stealing | Oldest voice hard reset | Deterministic least-audible release before replacement |
| Analysis | `WaterExcitationFeatures.h`, max-absolute envelope | Separate A1 linked-power analyzer; preserve legacy class |
| Size | Preview v0.2 maps Hz | Separate offline radius mapping candidate |
| Motion | Preview v0.2 maps rate | Offline activity multiplier; zero closes new events |
| Decay | Preview maps one time constant | Offline multiplier on physical lifetime only |

Legacy `FluidCandidate`, A0, B/D/C, Protect, preview/session v5 and mapping v0.2
remain reference paths. A1 is explicitly selected offline; omission never upgrades A0.
Do not translate legacy Hz fields into radius or relabel old listening evidence.

## Primary-source audit and limits

Reviewed 2026-09-24:

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

## Reproducible model specification

For radius R in meters, constants are pressure 101325 Pa, liquid density 998 kg/m3,
gas polytropic exponent 1.4. Frequency is `sqrt(3*kappa*P/rho)/(2*pi*R)`.
Physical damping is `0.13/R + 0.0072/pow(R,1.5)` per second; effective damping divides
this by persistence. This empirical fit is used only above its 0.15 mm lower limit.
Surface tension, pressure changes with depth, nonlinear oscillation and propagation
are omitted. Persistence other than one is an explicit nonphysical research scaling.
The rendered signal is a relative-amplitude oscillator proxy, not measured pressure in Pa or
an absolute bubble-to-microphone transfer prediction. Large-radius bins still assume
spherical independent oscillation; their availability does not prove real large bubbles
remain spherical/stable. No measured geometry, depth, temperature or fluid velocity is
reconstructed from music. These are material limits on the phrase physically coherent.

Engineering choices: 128 bins; probability proportional to `R^-gamma` per log bin;
size amplitude `(R/Rref)^alpha` normalized by its probability-weighted second moment.
This removes distribution-dependent expected initial squared amplitude, not duration,
perceived loudness, instantaneous peaks or event-rate loudness. No capacity divisor.
Excitation proxy `D=U^beta` is not geometric depth; rise only above cutoff.
P1 (current default) integrates `f0*(1+xi*dEffective*t)`, where
`dEffective=dPhysical/persistenceScale=1/tauEffective`. P0 explicitly uses
`f0*(1+xi*dPhysical*t)` for historical slope comparison. Both cap at `sqrt(2)*f0`
and `0.45*fs`. P1 keeps rise over one effective lifetime equal to xi; P0 makes it
xi times persistence. Both are approximate/stylized surface-rise models, not a
simulation of vertical velocity, position, hydrostatic pressure or a moving interface.

The requested per-sample probability `1-exp(-lambda/fs)` represents Poisson interval
occupancy, with at most one event per sample. Actual mean is `fs*(1-exp(-lambda/fs))`,
not lambda at high rates. This bias must be reported, not described as exact Poisson
counts. Source activity/energy mapping, calibration, stereo carrier, persistence,
capacity and stealing are engineering choices requiring tests and human evaluation.

## Documentation audit

Reviewed Architecture, Coding Plan, Parameters, Core Implementation Guide, Testing,
Code Standards, Document Governance, Module Index, Project Status, Perceptual Contract,
accepted EXP-W-001 brief, Developer Sound Tools, debug guide, ADR-0006, spike README,
mapping, listening execution and v0.2 handoff. Old conditional EXP-W-001 PLANNED text
in AGENTS does not override the accepted brief; older phase ledgers are historical.
Current repository M1 status supersedes the user's older pasted baseline. No unrelated
controlled status/acceptance record is rewritten as part of this research experiment.

## Validation and handoff

Historical pre-review Debug/Release/ASAN27/27 and70-case study-v2 results are preserved;
remediation measurements are recorded separately in the execution record. Human listening, two independent reviewer decisions
and production adoption are NOT RUN. Actual commands/results and the new legacy-path
fault are in the [execution record](../../docs/evidence/WATER_BUBBLE_A1_EXECUTION.md).

## Implementation and raw controls

`SharedExcitationAnalyzer.h`: fixed 2 ms stereo window, one linked power detector,
linear-power knee times slow RMS, empty-window gate. Analysis bounds finite source
channels to [-1,1]; neither source nor residual audio is clipped. Nonfinite analysis
samples become zero; offline input rejects nonfinite/over-full-scale audio.
Event carriers select ONE 2 ms frame `i*=argmax(L[i]^2+R[i]^2)`; equal energies
retain the lowest physical ring-buffer index. Its signed vector v is divided by
`sqrt((vL*vL+vR*vR)/2)`; norm <=1e-12 yields zero. The common amplitude is
`sqrt(linkedFastPower)` (fixed .25 for the amplitude ablation). Channel magnitudes
and signs therefore come from the same instant, including quadrature/transients.
No independent channel peak, phase, event scheduler or RNG is used. This preserves
swap/dual-mono/anti-phase/isolation; it does not promise original wideband stereo
correlation or perceived width. `source_energy_mean` reports this fast-power proxy. It is not spatial acoustic propagation or
the original waveform. Events may continue for 2 ms after input ends, not through
the detector's whole release. No delayed dry path or Host latency is added.

`BubbleA1Model.h` prepares physics/probabilities; `BubbleA1VoicePool.h` owns fixed
storage and active/free indices; `BubbleA1.h` coordinates one scheduler/RNG stream.
Shared acoustic trajectories carry separate signed channel amplitudes. Complex
rotation integrates frequency with midpoint steps; rotation increments use another
recurrence. Transcendental setup occurs at prepare/event start or one-time cap entry,
not per active voice per sample.

| Offline `bubbleA1` key | Range | Default |
| --- | --- | --- |
| radiusMinMm / radiusMaxMm | .2..10 / 2..50, min < max | .2 / 10 |
| populationGamma | 0..6 | 2 |
| amplitudeRadiusExponent | .75..2.25 | 1.5 |
| depthExponent | 1..16 | 10 |
| persistenceScale | .25..4 | 1 |
| maxEventRateHz / motionFactor | 0..10000 / 0..1 | 1000 / 1 |
| riseXi / riseCutoff | 0...2 / .8..1 | .1 / .9 |
| voiceCapacity | 64,128,256,512,1024 | 256 |
| tailFloorDb / stealReleaseMs | -100..-60 / .5..4 | -80 / 1.5 |
| residualGain | 0..1 | .2 |
| fastAttackMs / fastReleaseMs | .5..5 / 10..80 | 1 / 30 |
| slowAttackMs / slowReleaseMs | 10..80 / 80..500 | 30 / 200 |
| activityFloorDbFS / activityKneeDb | -80..-40 / 3..12 | -60 / 6 |
| version | required numeric 2 for supplied bubbleA1 objects | 2 |
| riseModel | numeric 0=P0 physical slope, 1=P1 effective slope | 1 |
| sourceEnergyAmplitude | numeric 0 or 1; ablation only | 1 |

Overflow chooses least envelope-weighted amplitude (lowest slot tie), releases it
linearly, then starts its stored replacement. All-releasing pools drop requests.
A capacity downshift lets existing voices finish and cancels starts that cannot fit.
Counters distinguish requested, accepted/queued, started, capacity drops, initiated
steals and completed. A later cancelled request is both historically accepted and
dropped: these describe transitions, not mutually exclusive totals. Retirement uses
maximum channel amplitude times envelope <= absolute tail floor, plus a 30 s guard.
Completed-lifetime histograms include stolen tails. Fixed histograms/counters support
offline percentiles, rates, utilization, rise fraction, source activity/energy and
residual peak/RMS. Lifetime percentiles are approximate lower-bin edges. No UI queue.
Motion0 closes new event admission; previously accepted replacement events can finish
their bounded release/start transition (at most4ms), and existing tails continue.
A fresh prepare at Motion0 is exactly silent. This is not an instantaneous output mute.

## Offline workflow

Existing renderer modes `a1-residual` (E), `a1` (x+E), `a1b`, `a1d`, `a1bd` explicitly
select A1. The latter replace only A in legacy composition. A1 rejects enabled Protect;
its sum has no limiter/normalizer. Other modes and preview imports reject `bubbleA1`
fields. Legacy A modes and omitted config remain A0. Example:

```json
{"bubbleA1":{"version":2,"radiusMinMm":0.2,"radiusMaxMm":10,"voiceCapacity":256}}
```

`frazil_water_research_cases --bubble-a1` exports `bubble-a1-offline-v2`: Size moves
radius endpoints `.2*10^s` and `2*25^s` mm, Motion sets `m*m`, Decay sets persistence
`4^(2*d-1)`. Other fields retain defaults. No import/overwrite of v0.2/session v5.

`render/bubble_a1_study.py` reuses the renderer/exporter/listening metrics. A0 is the
historical default; A1-1 uses physical radius/damping, flat bins, fixed linked RMS .25
and no rise; A1-2 adds gamma2; A1-3 adds source energy; A1-4 adds selective rise.
Fixed-amplitude ablations still retain source gating and stereo direction.
Per source: A0, four ablations, two named baselines, six P0/P1 persistence cases and
nine macro cases (22 total), repeat/block257 checks, raw residual,
and `(x+E)*10^(-18/20)` references. Post-render RMS matching only attenuates to the
quietest source-window RMS, with gains recorded. Motion0 makes that matched triplet
unassessable. Matching is not LUFS/perceptual equality or preservation evidence.
Two blank reviewer forms request identity, recognizability, motion, usefulness,
artifacts and timestamped decisions; no aggregate score chooses a winner.

After serial safe builds with the existing research opt-ins enabled:

```powershell
$research = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
python experiments/water/SPIKE-W-DSP-001/render/bubble_a1_study.py `
  --renderer "$research/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe" `
  --baseline-renderer build/bubble-a1/pr40-baseline-render.exe `
  --cases-executable "$research/frazil_water_research_cases_artefacts/Release/frazil_water_research_cases.exe" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/-_Sub Bass.wav" `
  --output build/bubble-a1/new-study
& "$research/frazil_water_bubble_a1_performance.exe"
```

Build the baseline executable from the recorded unmodified PR head before edits.
The batch compares decoded A0/B/D/BD/C/ABD exactly. Audio, source names, logs and machine
paths remain local/ignored. Timing: 44.1/48/96 kHz, five capacities, default/dense
profiles, block128, 500 warmup and 3000 measured calls, nearest-rank P95/P99. Active
mean/peak are sampled at block ends. This is research wall time, not device callback,
Host or formal budget acceptance. Event-time setup and detector math are timed.

## Version and baseline identity

Supplied `bubbleA1` objects MUST contain `version:2`. Missing/v1 versions and the old
`riseFactor` key reject before output creation. `riseXi` is dimensionless; the typed
`depthExcitationProxy` is dimensionless D, and `depthExponent` remains its empirical
shape parameter. No meters are inferred. `riseModel:0` chooses P0 with the corrected
shared-frame carrier; it does NOT recreate the old stereo defect. Reproduce exact
v1 audio at commit `1bc69476251851363189817a42735e62e8987916` with its v1 configs.
No public Host/state/session compatibility is changed. An explicit a1 mode with no
config uses current P1 physical-reference defaults; legacy a modes remain A0.

| Baseline name | Radius mm | Motion factor | Persistence | Meaning |
| --- | --- | --- | --- | --- |
| A1-PHYS-REF | .2..10 | 1 | 1 | Raw physical-reference configuration; gamma2, alpha1.5, beta10, xi.1, cutoff.9, rate1000, gain.2, cap256 |
| A1-MACRO-NEUTRAL | .632455532..10 | .25 | 1 | Offline Size/Motion/Decay=.5/.5/.5; other physical-reference fields retained |

These are deliberately different configurations. Macro neutral is not the raw
physical baseline. Neither is a perceptually accepted product default. The v2
exporter is authoritative for the macro curves; Python does not duplicate them.

## Physics Provenance Matrix

Source keys refer to the primary-source links above. Categories distinguish physical
approximations, empirical hypotheses, source coupling, product mapping and engineering.

| Element | Category / source | Implemented meaning and limit |
| --- | --- | --- |
| Minnaert frequency | Physical approximation; Minnaert via van den Doel and Phillips eq2 | SI radius, pressure/density/kappa; isolated spherical small oscillations in unbounded liquid |
| Radius-dependent damping | Empirical physical approximation; van den Doel eq3 | Normal water/air fit above .15mm; no separately solved viscous/thermal/radiation losses |
| Relative R^1.5 amplitude | Physical scaling assumption; van den Doel eq4/6 | Radius-independent initial inward velocity assumption; normalized relative amplitude, not calibrated Pa/SPL |
| D=U^beta and cutoff | Empirical population/formation proxy; van den Doel eq7 | Lumped attenuation/excitation, not metric depth; beta10/cutoff.9 are retained research choices |
| R^-gamma log-bin probabilities | Empirical statistical population; van den Doel eq8 | Adjustable relative rates; gamma2 is not a measured FRAZIL water distribution |
| 128 log-radius bins | Engineering discretization | Paper demonstrates50;128 is not a literature-required threshold or fidelity guarantee |
| One exponential occupancy trial/sample | Engineering approximation to paper Poisson populations | Mean fs*(1-exp(-lambda/fs)); one-event ceiling, not exact Poisson counts |
| P1/P0 pitch rise | Empirical acoustic cue; van den Doel section4 | P1 xi times effective damping; P0 historical physical-damping slope; both integrated/capped, neither geometry solver |
| sqrt(2) and .45fs caps | Approximate surface limit plus engineering bandwidth guard | No interface crossing/popping dynamics or proof of alias-free modulation |
| Shared power/activity and carrier | FRAZIL source-coupling engineering | One observed stereo frame, linked AR power, source gate; music is not fluid forcing data |
| Size endpoints / Motion squared | Product research mapping | Offline candidate curves, not physics or accepted Host macros |
| Persistence scale | Nonphysical research control | Multiplies tau; P1 keeps relative rise consistent, does not turn persistence into a physical fluid property |
| Gain .2 / moment normalization | Engineering calibration | Capacity independent initial-energy normalization; no loudness or integrated-energy equivalence |
| 64/128/256/512/1024 capacities | Engineering resource controls | Default256;512 stress,1024 dense96k high-cost stress; commercial FOAM is no physical validation |
| Active list / retirement / stealing | Engineering bounded lifecycle | Tail floor,30s guard, release/replacement/drop counters; no claim of physical bubble extinction |
| Geometry / boundary transfer | Not implemented; Harmonic Fluids / Complex Bubbles | No shape, moving surface/solid boundary, cavity transfer or bubble-to-ear model |
| Entrainment / splitting / merging / popping | Not implemented; Complex Bubbles | Stochastic arrivals replace fluid topology/forcing; no physical creation/extinction process |
| Coupled bubble clouds | Not implemented; Coupled Bubbles | More independent voices cannot reproduce collective low-frequency modes |
| Underwater-to-air radiation | Not implemented; Phillips2018 | Surface-driven airborne sound is distinct from simply transmitting underwater pressure |

The cited synthesis work is a physical/empirical hybrid. Literature provenance does
not make source coupling, product curves or finite resource policies physical laws.
Primary audit on remediation: van den Doel full manuscript sections3-6 and Phillips
full Results/Frequency/Propagation sections verified; 2009/2016/2023 author abstracts
verified, linked PDF retrieval failed. Original1933 full text and FOAM official page
remain inaccessible. Commercial marketing is excluded from quantitative validation.

## Remaining physical gaps and layered maturity

| Layer / gap | Status | Evidence boundary / next prerequisite |
| --- | --- | --- |
| Single-bubble core frequency/damping | PASS within declared approximation | Analytic SI/oscillator tests; no measured FRAZIL acoustic fit |
| Population synthesis | PARTIAL | Independent empirical log-bin distribution, no measured entrainment statistics |
| Arrival process | APPROXIMATE | Discrete occupancy bias retained and tested; no exact-count claim |
| Source coupling / stereo | ENGINEERING VALIDATED within tests | Shared-frame/event trajectory, finite/partition/swap properties; human width/identity NOT ASSESSED |
| Baseline identity | EXPLICIT | PHYS-REF and MACRO-NEUTRAL named separately; no silent v1 adoption |
| Capacity and CPU | MEASURED, NOT universally realtime-safe | All five capacities retained; per-device stress observations cannot establish Host deadline guarantees |
| Geometry / fluid topology | NOT IMPLEMENTED | Needs independently scoped fluid/shape/creation model |
| Propagation / radiation | NOT IMPLEMENTED | Needs transfer and surface-radiation model, not gain tuning |
| Bubble coupling / collective low end | NOT IMPLEMENTED | Needs coupled dynamics; voice count alone is insufficient |
| Full water/fluid simulation | NOT ACHIEVED | No claim that A1 reproduces the whole physical water process |
| Perceptual / production acceptance | NOT ASSESSED / NOT ADOPTED | Sound Lead and independent listening, later formal gates |

Do not tune gamma, gain, radii, beta, rate or cutoff merely to improve numeric metrics.
Quiet microbubble-dominant defaults and rare rise remain listening risks. A1 review
stops here; Droplet/Flow/Resonant redesign, A2 coupling and UI require later scope.
