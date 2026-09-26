# EXP-W-FD-003 — Sample-rate-aware D1 convergence

Status: preregistered C1/C2 contract, before new fixed-character candidate results.
Authority: user's next-stage convergence plan, reviewed baseline `7723c29`.
C0 closure: [FD-002 measurements](../../docs/evidence/WATER_FLOW_D1_LATENCY_STUDY.md)
now include independent Hosted completion; local validation fault causes remain open.
Physical authority remains [FD-001](EXP-W-FD-001.md). [ADR-0007](../../docs/adr/0007-minimum-practical-processing-latency.md)
remains Proposed. No production, renderer, Host, UI, routing or source retuning.
Implementation: engineering research; acceptance: Sound Lead + Engineering Joint Gate.

## Frequency and model authority

Retain E_AB=E_A+E_B and H_D(E_AB), tau=s/c, the shared-cluster reduction, source
parameters, seed and trajectory. Physical propagation and engineering guard remain
separate. D OFF and ON use the same conditioner and total engineering alignment.
No duplicate direct AB. Kernel qualifications are relative to the conditioned
ideal transfer; conditioner coloration relative to raw is separately reported.

- Tier A, 0–16 kHz: retain FD-002 static magnitude/phase/group/complex, moving,
  cross-rate and actual-source limits, eps16=2*sin(pi*16000*dt), dt=.05*3.2/1484^2.
  dt is an ENGINEERING screening scale, not physical measurement uncertainty.
- Tier B, 16–20 kHz: report fully, with no automatic deletion or listening PASS.
- Tier C, >20 kHz: reduced-perceptual-priority / ultrasonic-management region.
  Require finite/bounded behavior, stability, <=0.1 dB sampled transfer gain, and
  no excess core-band contamination in declared stress probes. This is not a
  universal anti-alias guarantee for all input signals or downstream nonlinearities.
The 0.1 dB gain ceiling applies independently to conditioner and kernel in this
conservative screen; a borderline failure is not evidence of audible harm or a
physical necessity for longer guard.

Conditioner core magnitude retains <=0.1 dB and maximum gain <=0.1 dB; phase,
frequency-dependent group delay and source/transient differences remain explicit
unaccepted product effects. Record whole-chain error against raw ideal H as well:
a passing kernel must not mask conditioner changes. Neither an IIR's zero fixed
latency nor a linear-phase FIR establishes perceptual transparency. The historical
0.1/60 dB design remains FD-002 stress evidence, not a product filter definition.
No gate is weakened after measurements. Filter core rejection stays visible even
if its downstream kernel passes. Tier A perceptual/whole-chain acceptance is pending.

## C2 bounded registry and numerical definitions

At 44.1/48 kHz: raw plus Butterworth4/6, Bessel4, FIR33/65 at 18/20 kHz (11 each).
At 96 kHz: raw primary, Butterworth4 at 20 kHz comparison only (2).
No Bessel6/FIR129 or additional families in this first round. No all-rate LPF mandate.

Butterworth/Bessel labels use their digital -3 dB cutoff (Bessel `norm=mag`).
The bilinear Bessel implementation is not phase-preserving near Nyquist. FIR uses
Kaiser beta from the previous 60 dB window choice, unity DC and the labelled `firwin`
half-amplitude (-6 dB) cutoff; this does NOT impose a 60 dB stopband. These different
cutoff meanings are always exposed, not presented as matched filters. Measure actual
16/18/20 kHz/Nyquist magnitude, phase/group delay, poles, impulse/ringing and B1
transients. Nominal 24/36 dB/oct is an analog asymptote, not a constant digital
near-Nyquist slope. Reuse existing causal/aligned conditioner and native prototype.

## C3 minimum guard qualification

Search 0,8,16,32,64 in order, the established centred Lagrange/Hann/Kaiser families
(guard0 historical Lagrange3 only). Stop after the first passing guard for that
conditioner/rate. At equal guard choose the smallest worst actual-source error;
family name only breaks an exact tie. This is numerical implementation selection,
not a product quality score. Keep failures. Do not enlarge the search if it fails.

Reuse frozen FD-002 analytic kernel tables (same implementation/commit/gates) for
static/moving/cross-rate qualifications through Tier A; verify their coverage before
use. Tier B results remain reported separately. Actual reference uses independently
converged 4N/8x versus 8N/16x Fourier reconstruction, convergence <=eps16/10.
All four unchanged profiles, A1/B1/AB, must pass eps16 NRMS/peak; retain this stricter
full-signal diagnostic too, without turning Tier C into a new product fidelity law.
Zero delay must equal the conditioned aligned baseline. Full-band sampled gain and
ultrasonic-tone core-error probes supplement finite/stability checks. They do not
certify source anti-aliasing or future Ice. Total latency=filter fixed delay+guard;
FIR=(N-1)/2, IIR=0 fixed lookahead with dispersion separately measured.

## C4–C6 convergence, resources and cross-rate evidence

S0 raw is a control at every rate, including failures. S1 is primary architecture:
managed 44.1/48, raw 96. S2 is common-intention Butterworth4 at20 kHz at every rate;
it is a comparison even if core tests fail, not an automatic fallback to adoption.
Shortlist up to three S1 character variants from Butterworth4, Butterworth6, FIR65,
using20 kHz first and18 only if20 fails mandatory numerical/core-magnitude criteria.
Each must qualify at both low rates; raw96 must qualify. This caps the final policy
comparisons at five including S0/S2. Do not native-benchmark every passing registry
row. Bessel/FIR33 stay documented comparisons; revise the contract explicitly before
substituting them if the bounded shortlist is empty. S1 primary + S0 raw reference
is an architectural research recommendation only; no final product filter chosen.

Native shortlist cases reuse the existing C++ prototype: <=1e-6 error against the
untabulated model, unchanged Fourier-reference gate, zero observed allocations,
reset/reprepare/partition checks, state bytes and P95/P99/peak timings. Record code,
machine and smoke versus500-block measurement separately. No formal CPU budget claim.

Cross-rate uses the same native physical/config preset and seed at each rate, all
four profiles, A1/B1/AB/D OFF/D ON. Report 0–16/16–20 kHz band/spectral differences,
common-time transient envelopes, event identities and path/movement differences.
Separate baseline source-rate variation from added conditioning/transfer variation.
Same seed is NOT assumed to produce the same event stream across different rates:
the existing A1 scheduler draws once per sample. Export actual event identities;
never repair this by retuning the sources. Event `identity_token` is the A1 radius
bin or B1 random rank, not a cross-rate identity guarantee. A1 IDs label requests
and admission means pool acceptance; B1 IDs label eligible onsets with admission
and requested due time. A1 due time is blank because stealing may defer a start. No objective cross-rate timbre PASS is
invented; Sound Lead must review the differences.

Listening generation follows shortlist only, with Source/raw AB/conditioned AB/
historical D1/candidate D1 at each rate, blind public labels and a separate key.
Bass/drums/musical pad/guitar or piano require authorized source material. Musical-pad
and human assessment remain deferred by the user. A synthetic regression pad does
not fulfill this requirement. Score Water identity, continuity, bubble clarity,
droplet attack, metallic/hollow/phasey/chorus/flanger/pre-ringing, dullness,
cross-rate timbre and source recognizability. Missing material means pack incomplete,
not permission to synthesize substitute listening acceptance.

## Sources and boundaries

- [Zheng & James, Harmonic Fluids](https://www.cs.cornell.edu/projects/HarmonicFluids/): bubble dynamics/advection and acoustic transfer; not our cutoff/guard or shared-cluster calibration. Author abstract retrieved.
- [van den Doel manuscript](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf): bubble emission as a liquid-sound basis; fresh manuscript retrieved. Does not specify D1 conditioning.
- [Smith fractional-delay text](https://www.dsprelated.com/freebooks/pasp/Lagrange_Interpolation.html): interpolation mathematics, not automatic near-Nyquist accuracy. Author text retrieved.
- [SciPy Bessel documentation](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.bessel.html): bilinear phase limitation above roughly fs/4, `norm=mag` definition. Links Thomson1949; original Thomson paper not claimed read.
- [Pro-Q modes](https://prod.fabfilter.com/help/pro-q/using/processingmode): near-Nyquist/phase/pre-ringing engineering tradeoffs; no FRAZIL algorithm or numeric requirement.
- [Kirchhoff author explanation](https://blog.threebodytech.com/127.html): Nyquist-matched linear modeling precedent, not water physics.
- [NOVA GE manual](https://docs.tokyodawn.net/nova-ge-manual/): internal processing quality/anti-alias management; internal bandwidth is distinct from product bandwidth. No imported internal-rate target.
- [Pro-L oversampling](https://www.fabfilter.com/help/pro-l/using/oversampling) and [Saturn release](https://prod.fabfilter.com/press/1589878800/fabfilter-releases-fabfilter-saturn-2-distortion-and-saturation-plug-in): nonlinear stages manage their own aliasing; no8x/32x requirement for linear D1.

Future ICE-ANTI-ALIAS-001 is DEFERRED until Ice is authorized: no implementation now.
D1 conditioner is not global anti-alias infrastructure. Before runtime replacement:
complete numerical/native/preservation evidence, independent machine, Debug/Release/
ASAN, reviewed cross-rate/perceptual evidence and ADR-0007 Joint Gate are all required.

Finite-window validation additionally measures 128 post-fixture zero-input frames
for every conditioner/source/profile; tail peak relative to complete filtered peak
must be <=1e-12. This engineering closure check is not an audibility criterion.
Failure requires extending the reference; do not truncate a meaningful filter tail.

Different per-rate kernels are additionally compared on exact common times with
closed-form sinusoidal transfer, using the unchanged2*eps16 core pair budget.
Whole-chain conditioner differences remain in a separate column; do not subtract
them away and call the complete chain transparent. Tier B is reported, not accepted.
