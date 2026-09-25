# EXP-W-FD-001 — Flow D1 reduced acoustic transfer

Implementation status: IMPLEMENTED (offline research candidate).
Contract established before sonic implementation; validation is separately recorded.
Human acceptance: NOT ASSESSED. Product adoption: NOT ADOPTED.
This canonical document owns the model, parameters and boundaries. Execution,
measurements, failures and branch lineage belong to the
[execution record](../../docs/evidence/WATER_FLOW_D1_EXECUTION.md).
Authority: user Flow D1 plan of 2026-09-26, [physical governance](../../docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md)
and [accepted Water definition](EXP-W-001_PERCEPTUAL_BRIEF.md). Definition acceptance
is not algorithm acceptance. Engineering implementation/self-review belongs to this
research task; independent engineering review and Sound Lead decisions remain separate.

## Contract Review and physical reduction

Fluid must retain input identity, rhythm, major attacks and pitch centre. Continuous
source-related evolution is desirable; F01/F02 chorus/flanger, F03 periodic motion,
F04 pitch wobble, F09 distraction, F10 smear, F13 source loss, F14 inaudibility and
F16 unnatural motion require independent listening. Numerical difference is not benefit.

The forward reference is a moving acoustic source with position xb, velocity vb,
dxb/dt=vb, and geometry/frequency-dependent transfer G(listener;xb,omega,t).
D1 replaces missing 3D positions, fluid velocity, boundaries and listener geometry
with ONE virtual projected excess path s in [0,A]. Zero is an implicit reference
path, not a colocated microphone. This is reduced acoustic-source advection plus
reduced time-varying acoustic transfer, not bulk-current propagation or a fluid solver.

Let E_AB=E_A1+E_B1 (with the absent source zero in ablations). With c_ref=1484 m/s,
tau=s/c_ref and H(E_AB)(t)=E_AB(t-tau(t)). D1 correction is H(E_AB)-E_AB;
the Fluid residual is H(E_AB), and full output is x+H(E_AB). The carrier x enters
only once, outside D1. The correction is not an independent emitter and its RMS
cannot rank D1 against bubble-source loudness. There is no standalone D1 listening mode.

A changing delay necessarily time-warps the residual; its instantaneous sinusoidal
frequency ratio is 1-dtau/dt. This is an intrinsic consequence, not a separately
calibrated Doppler solver. No correction gain, feedback or physical amplitude-scintillation model;
interpolation error can still introduce numerical level variation.

## Primary evidence and limits

- [Zheng and James 2009, author manuscript](https://research.cs.cornell.edu/HarmonicFluids/harmonicfluids-lowres.pdf):
  sections3.3/4/5 describe buoyant particle motion and geometry-dependent Helmholtz
  transfer. Eq19 has propagation phase and distance spreading. D1 retains only a
  virtual excess travel-time term; it omits their particle dynamics, boundary solver,
  geometric attenuation and air radiation. This is not reproduction of that paper.
- [Check and Watson 2023](https://link.springer.com/article/10.1007/s40799-023-00627-3):
  their 20°C/28kHz experiment reports 1484 ±3.2 m/s from a measured wavelength.
  We use1484 as fixed freshwater reference, not a universal material constant or
  a temperature estimate from audio. The article contains inconsistent percentage
  uncertainty wording; we use its dimensional observation, not that percentage.
- [Kumar and Verma, Taylor hypothesis study](https://arxiv.org/abs/1512.00959):
  applicability depends on a suitable mean transport field. Our Tc=Lc/U is a
  reduced timescale analogy, not measured turbulence or guaranteed frozen eddies.
- [Andreeva and Durgin2005](https://digitalcommons.calpoly.edu/provost_schol/36/):
  author abstract links travel-time statistics to path, mean flow and turbulence.
  It supplies no numeric calibration for D1. For a uniform current along a fixed
  path, t=L/(c+u), hence delta t approximately -L*u/c² for |u| much smaller than c.
  This dimensional sanity check is distinct from changing geometric path s/c.
  [Xu et al.2023, underwater moving tomography](https://www.frontiersin.org/journals/marine-science/articles/10.3389/fmars.2023.1111176/full),
  section3.1, explicitly separates path, sound speed, current and transceiver motion;
  it motivates the small-current expansion, not D1 virtual calibration. For example,
  our fixed-path approximation at L=.05m,u=1m/s is about22.7ns, versus33.7us for
  an excess geometric path of.05m. These are our derived values, not measured D1 data.
- [Laakso et al.1996, university publication record](https://research.aalto.fi/en/publications/splitting-the-unit-delay-tools-for-fractional-delay-filter-design/):
  fractional-delay FIR/allpass methods are numerical tools, not physical validation.
  Original DOI retrieval failed; do not claim its full text was retrieved. The
  interpolation study uses independently derived polynomial interpolation equations.
  [Julius O. Smith, Physical Audio Signal Processing](https://www.dsprelated.com/freebooks/pasp/Delay_Line_Signal_Interpolation.html)
  supplies the explicit Lagrange product formula and explains interpolation as a
  numerical delay approximation. Our causal near-zero stencil is not the centred
  passband-optimal stencil; its measured overshoot/error are retained.
- [van den Doel author manuscript](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf)
  and [Phillips et al.2018](https://www.nature.com/articles/s41598-018-27913-0)
  supply the retained A1/B1 source context audited in their contracts. They do not
  validate our virtual trajectory, source-audio substitution or D1 parameter ranges.

## Parameter authority and units

The three writable fields are prepare-only REDUCED_PHYSICAL_MODEL values. Ranges
are engineering research envelopes, not measured natural populations or product macros.
`FlowD1Config.h` owns typed specs, consumed by validation/parser/descriptor.

| Field | Unit | Minimum | Maximum | Default |
| --- | --- | --- | --- | --- |
| velocityScaleMps | m/s | 0 | 1 | .20 |
| virtualStructureLengthMeters | m | .005 | .20 | .03 |
| maxExcessPathMeters | m | 0 | .05 | .015 |

Config root flowD1 requires numeric version1; unknown fields/versions, nonfinite,
invalid type or out-of-range input reject without clamping. Derived c, delay, path
and trajectory rates are not writable. Descriptor model flow-d1, modelVersion1,
configVersion1, snapshot contracts/flow-d1-v1.json. No UI/session/Host registry.

## Trajectory and numerical contract

Uniform seeded waypoints lie in[0,A]. Smoothstep q(p)=3p²-2p³ connects endpoints
with zero endpoint velocity. Segment duration is at least max(Lc,1.5*|delta s|)/U,
so |ds/dt|<=U. Phase advances by U/(fs*max(Lc,1.5*|delta s|)), with the final step
shortened to land exactly on the target. At most one waypoint transition/sample.
This avoids converting arbitrarily long segment durations to integer counters.
Tiny positive speeds may produce zero phase increments or stagnate at floating-point
resolution; they never wrap a counter or acquire a minimum hidden motion rate.
U=0 or A=0 means s=0 and exact identity, with zero correction.
RNG domain10 is appended; domains1..9 and existing draw order stay unchanged.
One trajectory is shared across channels; audio memory is separate. No crossfeed.

The kernel study starts with linear interpolation against causal third-order
Lagrange on four available past samples. Before measurement, declare an engineering
comparison band0..0.2*fs: maximum magnitude error1dB and phase error0.1rad relative
to ideal delay. This is a numerical study target, not a perceptual threshold or
proof over the whole A1/B1 bandwidth. Also report0.45*fs stress because source
oscillators can extend there; do not hide out-of-band attenuation/amplification.
Use impulses, stepped/HF sines, two-tone and broadband, static/moving delay,
three rates, group delay, finite extremes and measured cost. Keep linear only if
it meets that declared target; otherwise justify the higher-order choice from data.
Selected third-order Lagrange after the predeclared sampled comparison
(201 delays and1801 frequency points per rate): linear exceeds1dB,
while cubic stays below0.457dB and0.078rad in the comparison band. This band is
8.82/9.6/19.2kHz at the tested rates. Up to0.45*fs, cubic magnitude error reaches
12.656dB: HIGH-FREQUENCY ACCURACY REMAINS LIMITED, especially for small A1/B1
bubbles at lower rates. The implementation is a bounded approximation, not an
accurate all-band propagation solver. No additional filtering/retuning is hidden.

Four taps use integer base=max(0,floor(delaySamples)-1), fractional coordinate
D=delaySamples-base and polynomial weights product over j!=k of(D-j)/(k-j).
All taps are causal. Maximum delay is3.235 samples at96kHz and5cm; eight fixed
history samples/channel safely cover support. Conservative drain is ceil(maxDelay)+3
samples, at most7. No memory growth or allocation occurs even in prepare.
For local coordinate D in[0,2], the sum of absolute polynomial coefficients is
conservatively below11, so even float extrema remain far within double range.
Input samples are float, result fields double; interpolation can amplify amplitude,
so neither unity instantaneous gain nor samplewise contraction is claimed.

Zero-source means zero input history gives exact zero. After nonzero history,
causal interpolation retains a bounded memory tail; it must drain after the oldest
permitted tap has passed. Do not gate current-frame silence or cut old source tails.
A transfer correction can exceed float range for opposite-sign finite float samples.
The D1 result therefore uses double precision for correction and transferred values;
finite-float input remains finite without clipping. Renderer conversion to float
must check representability. Compose via the returned H value rather than subtract
and re-add a rounded correction. Inactive failure returns zero; reset cannot enable
an unsuccessfully prepared object. Storage is fixed/prepare-owned, process noexcept.

## Traceability matrix

Paths are below SPIKE-W-DSP-001. DEFERRED means not yet implemented, not rejected.
Each row has one primary class; independent tests must not call the same model as oracle.

| ID | Contract | Primary class | Source | Code owner | Independent test owner | Status |
| --- | --- | --- | --- | --- | --- | --- |
| D1-PHY-001 | tau=s/c reference relation | PHYSICAL | acoustic travel time | dsp/FlowD1Model.h | SI values/monotonicity | IMPLEMENTED |
| D1-RED-001 | virtual s replaces absent geometry | REDUCED_PHYSICAL_MODEL | project reduction of Harmonic Fluids | dsp/FlowD1Trajectory.h | units/path-delay bounds | IMPLEMENTED |
| D1-RED-002 | Tc=Lc/U transport analogy | REDUCED_PHYSICAL_MODEL | Taylor applicability | dsp/FlowD1Model.h | independent scale ratios | IMPLEMENTED |
| D1-ENG-001 | seeded smooth bounded waypoints | ENGINEERING | project discretization | dsp/FlowD1Trajectory.h | analytic endpoint/speed/seed tests | IMPLEMENTED |
| D1-ENG-002 | causal fractional interpolation | ENGINEERING | polynomial interpolation | dsp/FlowD1FractionalDelay.h | impulse/frequency/moving study | IMPLEMENTED |
| D1-ENG-005 | H-E correction, direct H composition | ENGINEERING | residual ownership algebra | dsp/FlowD1.h | composition/isolation/extreme tests | IMPLEMENTED |
| D1-ENG-004 | strict versioned config | ENGINEERING | offline compatibility | render/ReadConfig.h | actual CLI snapshot/rejection | IMPLEMENTED |
| D1-RED-003 | full Green-function transfer replaced by variable excess delay | REDUCED_PHYSICAL_MODEL | Harmonic Fluids reduction | dsp/FlowD1.h | independent moving interpolation oracle | IMPLEMENTED |
| D1-ENG-003 | domain10, fixed resources, wide output | ENGINEERING | project finite/resource policy | WaterDspConfig / FlowD1FractionalDelay | domains1..10/extremes/allocation | IMPLEMENTED |
| D1-PROD-001 | Motion to D1 kinematics | PRODUCT_MAPPING | future accepted FRAZIL mapping | none | human listening | DEFERRED |
| D1-GAP-001 | actual CFD velocity field | PHYSICAL | fluid mechanics | none | none | DEFERRED |
| D1-GAP-002 | actual 3D source trajectory | PHYSICAL | Harmonic Fluids | none | none | DEFERRED |
| D1-GAP-003 | Helmholtz/boundary/air radiation | PHYSICAL | Harmonic Fluids, Phillips | none | none | DEFERRED |
| D1-GAP-004 | amplitude spreading / absorption | PHYSICAL | acoustics | none | none | DEFERRED |
| D1-GAP-005 | turbulent scattering / scintillation | PHYSICAL | flow acoustics | none | none | DEFERRED |
| D1-GAP-006 | physical stereo listener geometry | PHYSICAL | spatial acoustics | none | none | DEFERRED |

## Forbidden claims and compatibility

No actual water velocity/turbulence/Reynolds number/eddy spectrum, bubble position,
source-microphone distance/depth, surface geometry, temperature/salinity/pressure,
absolute Pa/SPL, measured attenuation or accurate water-air transfer may be inferred.
No CFD, Helmholtz solve,1/r gain, absorption, scattering, container resonance,
spatial widening, per-voice position or turbulence-noise source is implemented.

Legacy FlowModulator/FluidCandidate remains D0, an ENGINEERING carrier delay-minus-
input mechanism. Explicit new modes a1d1/b1d1/a1b1d1 and their -residual variants
alone enable D1. Old a/b/d/ab/ad/bd/abd/c and A1/B1 combinations remain exact.
Preview/sessionv5 retains A0/B0/D0; no mapping adapter, Size/Decay destination,
Motion curve, UI, Host/schema, production or Ice work. Protect remains off here.
This independently authorized task does not rewrite historical PR40 conditional
Flow decisions. No human approval, EXP-W-002 completion or M2 exit is implied.
