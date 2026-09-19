# EXP-W-RN-001 — C3 bounded modal normalization

Status: ENGINEERING VALIDATED; explicit research candidate, not preview default or product adoption.
Baseline: `24247c6`, following the reviewed [excitation comparison](EXP-W-RX-001.md).
This work preserves C0 and the [unmodified C1/C2 failure record](../../docs/evidence/WATER_LISTENING_REMEDIATION.md).
Engineering owns implementation/proof; human audibility and acceptance remain separate.

## Contract and scope

C3 is valid only with an EXP-W-RX-001 bounded conditioner, `|e[channel]|<=1`. Raw excitation plus
C3 is rejected before preparation; a finite float source is not assumed to be within full scale.
The normal preview, module JSON, session v3 and v0.2 mapper still use raw/C0. C3 is an explicit
offline/typed prepare option. No clipping/saturation occurs inside the modal recurrence, and no
finite-input guarantee is weakened. Production DSP, Host parameters/state, routing and Protect
remain unchanged. Decay remains 30/120/480 ms for this mapped-grid comparison.

## Derivation from the actual Cartesian recurrence

For the actual rounded pole coefficients `a+ic`, define `r=hypot(a,c)` and `theta=atan2(c,a)`.
The real excitation coefficient is b. Starting at zero, the imaginary-state impulse response is
`h[n]=b*r^n*sin(n*theta)` for n>=0; h[0]=0. All supported poles satisfy 0<r<1.

Define the positive, cancellation-resistant expression

```text
S(q,theta) = sum(q^n*sin^2(n*theta), n>=0)
           = q*(1+q)*sin^2(theta)
             / ((1-q)*((1-q)^2 + 4*q*sin^2(theta)))
```

Single-mode unit-b impulse energy is `S(r*r,theta)`. Cauchy-Schwarz gives a conservative induced
L1 bound, including the infinite tail:

```text
B(r,theta) = sqrt((r/(1-r))*S(r,theta))
sum(r^n*abs(sin(n*theta)), n>=1) <= B(r,theta)
||h||1 <= abs(b)*B(r,theta)
```

The six-mode bank uses `g/6`, with g<=.3. Current time-varying input weights are positive and
bounded by `Wmax=1.35/.65`; no assumption about cancellation or stochastic average is required.
For arbitrary bounded driver sequences and arbitrary legal weight trajectories:

```text
abs(E[channel,n]) <= Emax * (g/6) * Wmax * sum(abs(b_i)*B_i)
```

The same bound covers static Motion=0. L/R channels have isolated modal states and a linked
conditioner gain. The original dry source is not conditioned.

## Energy target versus safety cap

Research residual budget H=4 (+12.04 dBFS) is an explicit worst-case ceiling, not a target level,
normalizer or hidden limiter. With ordinary |x|<=1, Reference E Trim=0 and Monitor=-18 dB,
the conservative combined monitor bound is `(1+4)*10^(-18/20)<.63`. Focus boost can still exceed
full scale; existing over-range diagnostics remain necessary. No production gain budget is adopted.

Using each current frequency and a common 480 ms anchor, compute anchor unit-energy coefficients
`u_anchor_i=1/sqrt(S(r_anchor_i^2,theta_i))` and its worst bank bound with g=.3 and Wmax.
Set `alpha=.99*H/anchorBound`. For the actual Decay:

```text
b_candidate_i = alpha/sqrt(S(r_i*r_i,theta_i))
candidateBound = (.3/6)*Wmax*sum(b_candidate_i*B_i)
safetyScale = min(1, .99*H/candidateBound)
b_final_i = b_candidate_i*safetyScale
          = min(b_candidate_i, b_safe_i)
b_safe_i = b_candidate_i*(.99*H/candidateBound)
```

The common anchor keeps one energy target across 30/120/480 ms instead of applying an unrelated
gain change at each Decay value. Motion and the actual residual gain knob do not change alpha.
Longer legal raw Decay values up to 1 s can engage the common safety cap; this is reported, not
disguised as a perceptual normalization claim. At configured g<=.3, Emax=1 gives an analytical
residual ceiling <=3.96. The 1% margin exceeds observed floating arithmetic / tiny-tail-floor
effects; numerical adversarial tests supplement the mathematical bound rather than replacing it.
No overflow recovery, internal clamp or state saturation is added.

## Reproduce and distinguish evidence

The existing renderer accepts a final `c0|c3` option after the explicit conditioner and optional
excitation output (`-` omits it):

```text
renderer input.wav NEW-C.wav c-residual 128 42 config.json 3 - hard - c3
```

Omission remains C0. `modal_bound`, energy/safety scales and six actual b coefficients are emitted
by the actual prepared bank. They are numerical provenance, not a human score.

Run `render/bounded_normalization_study.py --renderer <built-renderer> --output <new-directory>`.
It renders actual source and impulse outputs at 44.1/48/96 kHz, root 130/260/520 Hz and Decay
.03/.12/.48. C0 and C3 remain separate. Source input, seed and Protect OFF remain fixed within
comparisons. Record early impulse peak, source-window RMS, total/tail energy, energy-time centroid,
sampled impulse L1, worst analytical gain, coefficients and partition identity. Analytical
reconstruction is checked against captured impulse output; it is not presented as a measured WAV.

`frazil_water_normalization` additionally drives the actual bank using FLT_MAX times the reversed
sign of its impulse response, then checks every output and the final induced response. Tests cover
all mapped triples, raw frequency/Decay corners, legal maximum Motion, all bounded conditioners,
stereo relationships, reset and blocks 32/64/128/256/257/512/1024. Raw+C3 is rejected.

`frazil_water_performance --normalization-study` measures the four bounded conditioners across
Motion 0/.5/1 and Decay .03/.12/.48 using the existing 48 kHz/stereo/block128 timing harness.
M1 and raw/C0 controls remain separate; timing is local engineering evidence, not a formal budget.

## Validation / review

Debug 23/23 PASS (40.82 s), Release 23/23 PASS (19.89 s); ASAN 23/23 PASS (74.15 s). Updated Debug renderer CLI checks also PASS (31.15 s).
The actual C0/C3 grid (`build/listening-ui/normalization-c3-v1`) has 54 rows, each with source,
impulse and block257 verification. C3 maximum analytical bound is 3.96 (roundoff at the last
printed decimal), sampled impulse L1 <=.62808; C0 sampled impulse L1 <=.06997. C3's common
mapped-grid safety scale is 1 within floating precision; no Decay-dependent cap is active there.

At 48 kHz/root260, Decay .03/.12/.48: C3 source-window RMS is -52.03/-47.87/-43.92 dBFS,
versus C0 -59.06/-60.93/-63.00 dBFS on the same gated engineering source. C3 impulse energy
is 1.32835e-5 / 1.31575e-5 / 1.31495e-5; energy-time centroid is .0147/.0599/.2400 s.
This demonstrates persistence with no normalization-driven energy collapse on the measured grid.
It does not establish clear perceived Decay or source preservation on musical material.

Timing has 38 rows (36 C3 combinations plus M1/raw-C0 controls). Maxima across rows:
mean 6.31 us, P95 6.9 us, P99 12.5 us, maximum 1684.3 us. The observed worst wall-time sample
is retained, not averaged away; it is below the 2666.7 us block period for this measurement,
which is not a formal CPU/Host acceptance result. Full row data remain local.
No new native GUI behavior was added in Phase 2; Phase 1's actual-driver monitor still uses raw/C0.
No human listening, new Protect conclusion, normalization adoption or readiness acceptance is
claimed. Next phase compares Decay persistence and fixed-source versus matched support material.

Independent code review checked the pole-energy derivation, conservative weight bound, fixed-size
callback state, failed-prepare behavior, enum validation, raw/C3 rejection, renderer output collision
checks and preservation of the C0 recurrence. No callback allocation, lock, I/O or saturation was
introduced. The new helper is driven by the requested whole-bank proof and preparation readout;
the new test and render script exercise that actual implementation rather than a parallel DSP.
Documentation impact: experiment record, module README/index and testing inventory updated.
Architecture, Parameters, production state, accepted brief, ADR, Protect and formal performance
contract reviewed without changes. No production acceptance or new UI behavior is implied.
