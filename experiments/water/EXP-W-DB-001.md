# EXP-W-DB-001 — Droplet / Impact B1

Implementation status: IMPLEMENTED. MODULE-LEVEL ENGINEERING COMPLETE / RESEARCH ONLY.
Engineering validation: PASS; see the execution record for exact scope.
Human acceptance: NOT ASSESSED. Authority: user B1 task attachment, accepted
[Water definition](EXP-W-001_PERCEPTUAL_BRIEF.md) and
[physical-model governance](../../docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md).
Starting source: A1 review `95be0de109c66be6ab218a9ac65384eabde3815e`, descendant of PR40.
Engineering implementation/self-review: current task agent; independent engineering
review and Sound Lead listening remain separate. No production adoption or merge.

## Contract Review

Preserve source rhythm, important attacks, bass/low-mid character and source-fused
liquid response. Detached Foley, intrusive fixed pitch, clutter and severe attack
damage remain reject conditions. Metrics cannot decide these listening questions.
Size means material scale, Decay persistence, Motion temporal activity. The accepted
brief leaves genuinely new onsets at minimum Motion unresolved. B1 core therefore
accepts explicit admission probability, never a Motion macro. M0-A probability0 and
M0-B onset-only probability0.25 are offline comparison hypotheses, not adopted curves.

## Primary-source provenance

- [Phillips, Agarwal & Jordan 2018](https://www.nature.com/articles/s41598-018-27913-0):
  Results, Frequency Comparison and eqs2–3 reviewed in full text. The measured example
  has a4mm drop, a0.71mm stabilized bubble diameter and8.66kHz dominant frequency.
  Wave-packet onset aligns with pinch-off; preventing entrainment suppresses the plink.
  First three tests approximately agree with Minnaert within10%; smaller cases differ.
  Surface-driven airborne radiation is distinct from simple underwater transmission.
- [Oguz & Prosperetti 1990](https://doi.org/10.1017/S0022112090002890): publisher abstract
  and author-hosted manuscript entry reviewed; direct PDF download denied. Boundary-element
  fluid dynamics include geometry and surface tension; entrainment depends on actual
  drop radius/velocity. B1 supplies none of those measured states and implements no CFD.
- [Pumphrey & Elmore 1990](https://doi.org/10.1017/S0022112090003378): publisher abstract
  reviewed; regular, irregular, large-bubble and Mesler entrainment are distinguished;
  the last is weakly audible. This supports separate admission, not our numeric probability.
- [van den Doel 2005](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf):
  original manuscript/reference equations reviewed in the A1 source audit; current
  retrieval was intermittent and direct download denied. Selected damping fit applies
  above approximately0.15mm. Radius-amplitude scaling assumes unknown inward fluid velocity;
  optional rise and population statistics form a physical/empirical hybrid. Its19-subject
  single-bubble study motivates testing2–7mm, not a measured entrainment distribution.
- [Bello et al. 2005](https://doi.org/10.1109/TSA.2005.851998): original manuscript,
  [university-hosted copy](https://www.iro.umontreal.ca/~pift6080/H09/documents/papers/bello_onset_tutorial.pdf),
  signal-feature and threshold sections reviewed. Onset detection is signal analysis;
  B1's fast/slow relative-power detector is our bounded candidate, not a reproduced
  paper algorithm or a physical impact measurement.

## Module and equation contract

Implemented mechanisms below have executable engineering evidence. Product macro
curves remain RESEARCH-CANDIDATE and require human acceptance; no status implies adoption.
All paths below are under `SPIKE-W-DSP-001/`. Each row has one primary category.

| ID | Contract / equation | Classification | Source | Code owner | Independent test owner | Status |
| --- | --- | --- | --- | --- | --- | --- |
| B1-PHY-001 | f0=sqrt(3*1.4*101325/998)/(2*pi*R), SI radius | PHYSICAL | Phillips eq2 | physics/BubblePhysics | physics fixed-radius oracle | IMPLEMENTED |
| B1-PHY-002 | d=.13/R+.0072/R^1.5; natural tau=1/d | PHYSICAL | van den Doel damping fit | physics/BubblePhysics | physics formula/monotonicity | IMPLEMENTED |
| B1-PHY-003 | physicalAmplitudeScale=(R/.002)^1.5 | PHYSICAL | simplified formation scaling, relative reference | DropletB1Model | radius-scale oracle | IMPLEMENTED |
| B1-PHY-004 | x=A*exp(-d*t)*sin(phase), phase'=omega | PHYSICAL | isolated small radial perturbation model | DropletB1BubbleVoice | analytic early-sample oracle | IMPLEMENTED |
| B1-PHY-005 | relative emission proportional to4*pi*R^2*x'' | PHYSICAL | Phillips eq3, linearized spherical volume | DropletB1AcousticEmission | analytic volume-acceleration oracle | IMPLEMENTED |
| B1-RED-001 | eligible musical onset to captured VirtualImpact | REDUCED_PHYSICAL_MODEL | project source coupling | DropletB1SourceCoupler | captured stereo/source tests | IMPLEMENTED |
| B1-RED-002 | independent admission probability; delayed single bubble | REDUCED_PHYSICAL_MODEL | entrainment literature, project surrogate | DropletB1EntrainmentModel | RNG/timing/endpoint oracle | IMPLEMENTED |
| B1-RED-003 | optional f=f0*(1+xi*dPhysical*t), capped | REDUCED_PHYSICAL_MODEL | van den Doel empirical rise cue | DropletB1BubbleVoice | integrated chirp/cap oracle | IMPLEMENTED |
| B1-PROD-001 | dEffective=dPhysical/persistence | PRODUCT_MAPPING | explicit nonphysical persistence | DropletB1Model | isolation/lifetime tests | IMPLEMENTED |
| B1-PROD-002 | R=.2*35^Size mm; persistence=4^(2*Decay-1) | PRODUCT_MAPPING | offline-v1 candidate, no Motion mapping | offline mapper | endpoints/isolation | RESEARCH-CANDIDATE |
| B1-ENG-001 | 10log10((Pfast+eps)/(Pslow+eps)), floor/hysteresis/spacing | ENGINEERING | Bello context; project detector | DropletB1OnsetDetector | synthetic source fixtures | IMPLEMENTED |
| B1-ENG-002 | fixed queue16; voices256 with runtime16/32/64/128/256 | ENGINEERING | realtime bounds | queue / pool | capacity/lifecycle tests | IMPLEMENTED |
| B1-ENG-003 | recurrence, bandwidth cap, tail floor, deterministic release | ENGINEERING | numerical resource policy | voice / pool | recurrence/cap/retirement | IMPLEMENTED |
| B1-ENG-004 | fixed emission reference and explicit amplitude normalization | ENGINEERING | calibration, not physical conservation | emission / model | raw/normalized/capacity tests | IMPLEMENTED |
| B1-GAP-001 | CavityGeometryModel / pinch-off CFD | PHYSICAL | Oguz & Prosperetti | none | none | DEFERRED |
| B1-GAP-002 | SurfaceRadiationModel / AbsolutePressureModel / MicrophoneDistanceModel | PHYSICAL | Phillips | none | none | DEFERRED |
| B1-GAP-003 | SyntheticImpactPulseModel, multiple entrainment classes and coupling | REDUCED_PHYSICAL_MODEL | future separately reviewed model | none | none | DEFERRED |

## Mathematical and resource decisions

Equivalent radius is an acoustic latent, not real drop size. Reference range0.2–7mm
is a research interval, not a measured population. Expected f0 at0.2/.355/2/7mm is
approximately16.43/9.26/1.64/.47kHz. The9.26kHz prediction is not forced to equal
Phillips'8.66kHz measurement. Reference constants are not UI controls.

The reference is an isolated, approximately spherical air bubble with small radial
perturbation, fixed reference pressure 101325 Pa, water density 998 kg/m3 and gas
exponent 1.4. It omits surface-tension frequency corrections, hydrostatic depth,
nonlinear radial motion and bubble-bubble interaction. The damping relation is a
physical empirical fit, not a first-principles thermal/viscous/radiative solver.
The Phillips datum provides a scale check at one radius; it does not experimentally
validate this entire radius range, source coupling, rise law or musical mapping.

Capture one same-frame signed stereo direction and linked fast-RMS excitation at
eligible onset. Never resample after delay. Candidate identity and admission consume
one draw each per eligible onset, including p0/p1; dedicated domains7/8, future jitter9
reserved. Fixed radius remains study-controlled; random rank is diagnostic identity,
not an invented radius distribution. A1's domain6 and all legacy streams stay intact.

For x=A*e^(-dEffective*t)*sin(phi), compute x'' analytically:
`A*e^(-dEffective*t)*[(dEffective^2-omega^2)*sin(phi)
+(omegaPrime-2*dEffective*omega)*cos(phi)]`.
Relative emission divides4*pi*R^2*x'' by the fixed reference
`4*pi*(.002)^2*(2*pi*f0(.002))^2`; this is ENGINEERING calibration, not Pa.
No noisy finite differences. An explicit displacement ablation is separate.
Raw formation scaling and normalized musical amplitude are explicit alternatives;
normalization divides out the radius-amplitude factor, not the physical equation.
Source excitation is dimensionless and bounded, never measured fluid velocity.

Rise defaults0; optional.05/.1 uses physical damping slope, independent of persistence.
Frequency is capped at min(sqrt(2)*f0,.45*fs); cap is a numerical/stylized boundary,
not a geometric free surface. The analytic derivative uses zero omegaPrime after cap.
Product parameters are prepare-only, except audio-owner admission retarget preserving
already queued events and tails. This is not realtime macro automation acceptance.

Tail floor is relative envelope amplitude. Maximum natural tau occurs at7mm;
maximum lifetime is `-ln(10^(-100/20))*4/d(.007)`, approximately1.5s.
A2s guard strictly exceeds this plus one sample; independent tests must verify it.
The queue captures at most ceil(40/8)+1=6 pending events under legal scheduling;
16 slots leave margin. Voice ceilings do not divide amplitude. On overflow choose
least estimated audible envelope with stable slot tie, release linearly, then start
the captured replacement. If every slot is already releasing, drop the new request.
Stealing can add up to `ceil(fs*releaseMs/1000)` samples beyond the pinch-off due
time (at most 4 ms plus sampling quantization), and is recorded explicitly.

## Lifecycle and diagnostics

`eligible` counts detector transitions; `admitted` passes the independent draw;
`rejectedByAdmission=eligible-admitted`. `queued` counts successful inserts, so
`admitted=queued+droppedByPendingCapacity`. Due events are removed exactly once.
`steals` counts accepted replacement reservations, not voice starts. `started`
counts coefficient initialization: immediate free-slot starts emit that sample;
replacement starts initialize after the final zero-contribution release sample
and emit the next sample. `completed` counts each natural or stolen outgoing voice
once. `droppedByVoiceCapacity` counts due requests refused while every slot already
has a replacement reserved. A reset clears every counter and captured event.
`active` includes a replacement initialized at the end of the current sample;
timing reports sample block-end occupancy, not a voice-time integral.

The renderer reports physical and render amplitude scales separately, plus the last
started event's captured `sourceExcitation` and `renderAmplitude` (scale times source
excitation times residual gain, before signed stereo direction and emission).
These last-event diagnostics are zero if no voice started. They are neither peak
audio amplitude nor pressure. Tail-floor retirement is gain independent; it is not
an absolute audibility claim. The bounded priority estimate used for stealing is
an engineering heuristic, not a perceptual loudness detector.

## Research parameter authority

[Machine-readable snapshot](contracts/droplet-b1-v1.json) is generated by
`frazil_water_experiment_render --describe-droplet-b1`; `DropletB1Config.h` owns
all writable numeric defaults/ranges. The parser consumes that same specification.
Derived frequency, damping and source excitation reject writes. Config root
`dropletB1` requires `version: 1` and is accepted only by explicit B1 modes.
Each parameter has a primary classification and unit. Full per-render configs
are saved by the study, including defaults; an omitted root retains legacy behavior.

## Forbidden Physical Claims

Do not claim reconstruction of real drop diameter, impact velocity, cavity depth,
bubble depth, free-surface geometry, surface tension, water temperature, physical
fluid velocity from audio amplitude, absolute microphone/underwater pressure, SPL,
source-to-microphone propagation or accurate water-air transmission. No misleading
dropDiameterMm, impactVelocityMps, cavityDepthMm, waterVelocityMps or pressurePa fields.

## Compatibility and handoff

Explicit b1/b1-residual/a1b1/a1b1d modes only. Legacy b/bd/abd remain B0;
A1/B0/D0/C and Preview/sessionv5, Host/state and production are unchanged.
BubblePhysics now also supplies the unchanged A1 reference equations. Consolidation
requires exact decoded A1/B1 identity; see the A1 governance execution record for
validation status. No A1/B1 tuning is permitted.
Descriptor version1 and tracked JSON snapshot own the numeric parameter contract.
Study must preserve separate fixed-source and RMS-matched questions and two blank
human forms. Engineering results and regeneration commands belong to
[B1 execution](../../docs/evidence/WATER_DROPLET_B1_EXECUTION.md).
