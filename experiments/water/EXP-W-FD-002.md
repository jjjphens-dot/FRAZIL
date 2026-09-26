# EXP-W-FD-002 — D1 guard latency and audible-band study

Status: completed historical preregistered screen; original gates/results retained.
Current convergence policy is [EXP-W-FD-003](EXP-W-FD-003.md), which supersedes
the broad candidate search and defines sample-rate-aware ultrasonic management.
Authority: user's 2026-09-26 physical-model/low-latency plan. Physical authority
remains [EXP-W-FD-001](EXP-W-FD-001.md); proposed processing policy is
[ADR-0007](../../docs/adr/0007-minimum-practical-processing-latency.md).
No runtime replacement, production adoption, final bandwidth mapping or human PASS.

## Independent bandwidth contract

This contract is frozen for this study before kernel PASS/FAIL is inspected.
The user's protected core is 0–16 kHz; 16–20 kHz is explicitly evaluated; >20 kHz
was a candidate attenuation region for this historical screen. Current FD-003 policy
is reduced-perceptual-priority / ultrasonic-management, without mandatory removal. These are research/product choices, not water
absorption or universal human hearing limits. Conditioner design and qualification
must be independent of the D1 kernel. No source-model or trajectory retuning.

ENGINEERING screening thresholds: <=0.1 dB maximum passband magnitude deviation
through each declared passband (16 or 18 kHz), >=60 dB attenuation from declared
stopband to Nyquist, no more than 0.1 dB gain anywhere. These conservative study
values are not psychoacoustic thresholds or constants from a paper. Report 16/18/20
kHz explicitly even when inside a transition/stopband. A 16 kHz passband with an
18/19/20 kHz stopband is an unaccepted extended-band tradeoff, never proof that the
removed content was inaudible. Human review must choose the bandwidth before adoption.

Compare Butterworth SOS IIR and linear-phase Kaiser FIR at all valid combinations
of passband 16/18 kHz and stopband 18/19/20 kHz/Nyquist-safe (strictly above pass).
Nyquist-safe stop = min(24 kHz,0.98*Nyquist), explicitly ENGINEERING. FIR lengths
17/33/65/129 are bounded short-filter probes; do not secretly increase length.
IIR design uses the independently fixed 0.1/60 dB specifications; record actual
order, poles/stability, coefficient count and measured response, not just design
claims. Include unfiltered control. No gain matching in numerical comparisons.

Filter-only evidence: magnitude, phase/group delay, impulse, energy before/after
main impulse peak, peak/energy/onset changes in actual A1/B1/overlapping AB, spectra
and ringing duration. IIR causal pre-onset energy is zero; energy before its impulse
peak is NOT acausal pre-ringing. FIR pre-ringing is reported relative to its declared
linear-phase delay. Numerical screening does not accept B1 attack preservation.
Nonlinearity audit uses labelled synthetic square/cubic stress probes and reports
audible difference/alias energy, not a model of the unimplemented Ice processor.

## Fractional-delay qualification

Guards 0/8/16/32/64 samples; rates 44100/48000/96000 Hz. Keep s in[0,.05] m,
c=1484 m/s and existing trajectory. Compare historical Lagrange3 with centred
Lagrange and windowed sinc; higher order is not an accuracy claim. Farrow/allpass
are optional later families, not obligatory independent evidence.

Static ideal includes guard: exp(-j*w*(Lg+tau*fs)). De-embed exactly known Lg for
physical error reporting. Candidate support may not read samples after processing
time. Offline aligned output at n compares candidate evaluated with available
future support n+Lg to H(E)[n]; do not evaluate the physical trajectory at n+Lg.
This prevents guard from shifting the physical path/control clock.

Retain the previous dt=.05*3.2/1484^2 engineering scale (not calibrated physics).
Core gate through16 kHz: |GD error|<=dt, |phase error|<=2*pi*f*dt,
complex error<=eps16=2*sin(pi*16000*dt), magnitude error<=-20log10(1-eps16).
Extended16–20 kHz: separately report the same dt-derived gates using eps20;
do not relax them after results. Conditioner passband defines the joint static
qualification interval; full20 kHz raw-kernel status is always reported separately.
Moving/actual conditioned-source NRMS and peak-normalized error<=eps16; independent
reference convergence<=eps16/10. Full-band raw-source stress remains reported even
when it fails. Reject unqualified references; no self-comparison or hidden scaling.

Use analytic fixed1/2/4/8/12/16/18/20 kHz, dense frequency/path grids and integer
stencil boundaries. Cross-rate analytic tests share physical paths and exact common
times, with pair error<=2*eps16. Actual native A1, B1 and simultaneous AB must pass;
report overlap sample count/energy, not merely nonzero isolated sources. Preserve
historical source fixtures and use additional overlap excitation without retuning
the A1/B1 model. Independently converged zero-padded Fourier reconstruction is the
oracle; existing numerical utilities may be reused and checked analytically.

## Composition and comb audit

M0=AB; M1=ideal H(AB); M2=historical numerical H(AB); M3=AB+ideal H(AB);
M4=AB+numerical H(AB). Actual D0 is a separate historical control: it processes
the input carrier, not AB. Also label analytic g*(H-1) as a frozen-D0 mechanism
probe, never as actual modulated D0 output. Pure delay has unit magnitude; equal
direct+delayed notches=(2k+1)/(2*tau), spacing=1/tau. Test physical range and
.05/.1/.25/.5/1/2/5 ms; larger values are comb engineering probes, not D1 paths.
Save response, notch positions, ripple, group delay (undefined at zeros), impulse,
actual-source difference spectra and waveform evidence. Separate source resonance
from transfer artifacts; metallic/hollow/phasey/chorus/flanger/ringing are human labels.

D-OFF and D-ON share conditioner and guard. U=0 or A=0 must equal that aligned
baseline, not historical undelayed raw AB. Composition is H(AB), never AB+alpha*H(AB).
Audit renderer full/residual/debug paths and record unimplemented future routing/
dry-wet as NOT IMPLEMENTED, not passed. Carrier enters once and would need the same
engineering alignment in any future full-output candidate.

## Implementation traceability

| Responsibility | Classification | Current owner | Independent check |
| --- | --- | --- | --- |
| Shared emission sum | REDUCED_PHYSICAL_MODEL | existing FluidComponents/native_cluster_signal; native probe mirrors renderer float boundary | actual renderer versus separate source/transfer probe |
| Path generation | ENGINEERING kinematic surrogate for reduced advection | unchanged FlowD1Trajectory | previous path/speed tests plus exported actual path |
| tau=s/c | PHYSICAL under stated freshwater/reduced-path assumptions | unchanged FlowD1Model | independent SI expression in study |
| Audible band choice | PRODUCT_MAPPING, unaccepted research | this contract | separately reported16/18/20 kHz and future human review |
| IIR/FIR realization | ENGINEERING | Conditioner in flow_d1_latency_models.py | analytic Butterworth power, symmetry/latency, response/transient audit |
| Guard and interpolation | ENGINEERING ONLY | GuardKernel in flow_d1_latency_models.py | polynomial product, analytic sine, causal support and path-clock tests |
| Comb/bandwidth audit | ENGINEERING test/analysis | flow_d1_latency_study.py | closed-form notches and independent Fourier reference |
| Production latency alignment/reporting | ENGINEERING, DEFERRED | ADR-0007 proposal; no WaterLatencyAligner implemented | future routing/Host acceptance |

FIR's declared fixed delay is also removed from the physical trajectory clock in
the aligned offline representation. Final causal availability is shifted by
filterLatency+guardLatency; IIR's frequency-dependent phase is measured separately
and cannot be removed by pretending it is one fixed delay.

## Selection and handoff criteria

Report filter-only, latency-only, kernel-only and combination separately. Select
only the smallest passing guard for each qualified conditioner, then compare total
filter+guard latency; do not trade bandwidth solely to reduce taps. No final product
bandwidth choice without human evidence. Native resource benchmarking follows
numerical qualification. Runtime replacement additionally requires full regressions,
clean full ASAN, independent review and listening. If no bounded combination passes,
record failure and return to model review; do not enlarge guard/path or relax gates.
musical-pad and human review remain deferred per user instruction.

## Native resource realization (test only)

After the offline screen, `tests/LatencyCandidate.h` realizes the same SOS/FIR
conditioning and centred kernel as a bounded streaming prototype. A prepare-owned
4097-position double-precision coefficient table adds a measured numerical approximation; it is not
an optimized production design. Audio history is 136 stereo frames, with separate
physical-path history and prepared SOS states. Fixed table/state memory, allocations,
mean/P95/P99/peak block times, reset/reprepare and partition invariance are reported.

`flow_d1_latency_native_study.py` measures every minimum-guard combination surviving
the offline screen, on all four actual AB fixtures. It compares C++ with the original
untabulated model (relative RMS and peak <=1e-6) and the independent converged oracle
using the unchanged eps16 gate. Coefficient import and file I/O occur only outside
the measured processing loop. The existing isolated allocation observer is shared
between executables; it observes replaceable C++ allocation entry points, not every
possible system allocator. Process code is also reviewed for direct allocation/I/O.

CPU scope is conditioner plus interpolation only, excluding A1/B1 source generation,
Host and routing. Seven block sizes (32/64/128/256/257/512/1024) are measured; full
resource rows use 100 warmup +500 measured blocks on overlap-reference. Other fixtures
and CTest use explicitly labelled 3+5 smoke timings, not performance evidence. This
prototype never links into the renderer, Preview or production targets.

The optional CI `flow_d1_latency` dispatch reproduces this study on an independent
Windows Release runner using generated source fixtures only. Record its code commit,
interpreter/packages and machine metadata separately; do not pool partial local
attempts with hosted results or infer a local-environment repair from remote success.
