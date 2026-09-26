# Flow D1 physical/numerical remediation

Status: **D1-NUM-001 OPEN / BLOCKED; no replacement selected**. This is a
reviewable research finding, not a completed kernel fix. The six causal candidates
tested do not satisfy the source-aware gates together. No evidence here proves
that every possible causal fractional-delay design must fail.

## Contract Review — R0/R1

Starting commit: 3c95fe9662e0ac12e91ecce846e2a1c301c285c0 on
`codex/experiment/water-flow-d1`. `git fetch --all --prune` confirmed no newer
descendant before implementation. Origin was verified as the FRAZIL repository.
Its [original hosted CI](https://github.com/jjjphens-dot/FRAZIL/actions/runs/36172602404)
was freshly checked: SUCCESS on that exact head. This is baseline evidence only.
The delta from A1/B1 governance baseline 30186b8 is the original isolated D1 work;
the default checkout's unrelated user changes were not touched.

The required canonical contracts, A1/B1/D1 execution records and numeric tables,
source implementations and tests were read. Review confirmed the topology
H(E_A1+E_B1), carrier-once composition, shared stereo trajectory and absence of
actual source/listener/CFD geometry. The [canonical contract](../../experiments/water/EXP-W-FD-001.md)
was amended and self-reviewed **before** candidate research: receive-time path,
low-Mach limit, co-located cluster reduction, source-derived frequency support,
fixed-Hz comparisons and coupled magnitude/phase/group-delay criteria.

The original 0..0.2*fs selection is superseded. Its data and code remain historical
evidence; no threshold was relaxed after seeing the new results. The engineering
time-error comparison scale is 72.6528 ns, derived from maximum path and the
documented sound-speed reference uncertainty. It is not a physical calibration
or listening threshold. The resulting complex-error ceilings are .009059/.009860/
.010608 at 44.1/48/96 kHz. Candidate failure is assessed against the ideal transfer,
not against how similar a candidate sounds to the old kernel.

### Literature audit

- [Zheng and James, author project](https://www.cs.cornell.edu/projects/HarmonicFluids/)
  confirms particle advection and individual bubble-to-ear Helmholtz transfer.
  Those mechanisms support the forward-model direction. Missing geometry is why
  D1 reduces all emitters to one virtual path; the paper does not calibrate our
  waypoint process. The current PDF fetch failed; the earlier equation audit in
  the retained D1 record is not presented as a newly retrieved full text.
- [van den Doel, author manuscript](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf)
  supports bubble oscillators/populations as liquid-sound sources. A1/B1 retain
  that responsibility; it supplies no D1 path or stochastic-speed calibration.
- [Phillips, Agarwal and Jordan, full text](https://pmc.ncbi.nlm.nih.gov/articles/PMC6014985/)
  ties the tap plink to entrained bubble resonance and discusses surface radiation.
  This supports source identity, not substitution of musical audio for drop
  kinematics or delay-only water-to-air transfer.
- [Laakso et al., university record](https://research.aalto.fi/en/publications/splitting-the-unit-delay-tools-for-fractional-delay-filter-design/)
  verifies the FIR/allpass review. Full text remained unavailable; no reproduced
  coefficients or full-paper review is claimed. The independently derived fits
  are project numerical experiments.
- [Vesma and Saramaki, author-hosted notes](https://homepages.tuni.fi/tapio.saramaki/part3multi.pdf)
  define sample interpolation and polynomial FIR/Farrow structures. Two-sided
  support explains the lookahead of centred methods; it does not validate our
  one-sided stencil. These notes are primary author material, not a substitute
  citation claiming the inaccessible 1996 paper was read.
- [Kumar and Verma](https://arxiv.org/abs/1512.00959) identify a suitable mean
  circulation for Taylor's hypothesis. D1 has no measured convection field;
  L/U remains a reduced timescale analogy.
- [Check and Watson](https://link.springer.com/article/10.1007/s40799-023-00627-3)
  report 1484 +/-3.2 m/s at 20 C. The dimensional observation motivates a fixed
  engineering error scale; it neither measures FRAZIL nor fixes universal water
  sound speed. Temperature, pressure, salinity and absolute path remain absent.

## Implementation — R2

Only offline research infrastructure and documentation changed. No source model,
runtime interpolation, trajectory, RNG domain, config version, renderer mode,
Preview, Host, UI or production algorithm changed.

New files are driven by two missing evidence capabilities:

- `tests/flow_d1_source_probe.cpp` exports typed source radii/physics constants and
  actual A1/B1 output with the existing D1 trajectory. Six one-second fixtures:
  three rates, reference settings and minimum-radius/high-rise edge settings.
  The same gated 997 Hz physical source is sampled at each rate, seed42, signed
  stereo ratio -0.5. These are engineering stimuli, not musical recordings.
  Edge A1 uses minimum radius endpoints, maximum population exponent and rise;
  B1 uses minimum radius and maximum legal rise. No source implementation changes.
- `render/flow_d1_remediation_study.py` compares independent analytic transfer and
  time-warp oracles, actual residuals and offline references. Its dedicated CTest
  verifies the oracle machinery and actual exporter, rather than asserting that
  the current D1 is physically accurate.

The source probe reports f0_max=16432.6580 Hz; the retained rise cap gives
f_relevant_max=23239.2878 Hz. Therefore the physical test limits are **19845,
21600 and 23239.2878 Hz**, not three unrelated normalized acceptance bands.

Candidates: causal Lagrange orders3/7/15; degree7 Farrow with eight fixed length32 FIR polynomial branches; and
32/64-tap variable FIR coefficient tables. Farrow and
table32 share the same uniform-frequency least-squares design, so their similar
results are expected, not independent-method corroboration. Table coefficients
use 513 fractional-delay nodes with linear coefficient interpolation. The fit
enforces DC algebraically, uses a 2049-point frequency design grid and SVD cutoff
1e-12. There is no source EQ, clipping, gain correction or hidden low-pass.
Centred 129-tap Hann sinc divides its coefficient row by its sum for exact DC;
this declared numerical definition is an offline noncausal comparison control,
not source-level normalization or an eligible runtime fix.

Each static evaluation uses at least 201 delays across the complete legal path,
integer-boundary neighbours and 1801 in-band frequency points plus required
fixed/normalized frequencies. Group delay uses the analytic frequency derivative
of each candidate response, not a coarse numerical gradient. Dense-grid evidence
does not prove the continuous supremum; future passing candidates need refinement.

Moving analytic probes use one continuous seeded physical path sampled at all
three rates; its maximum speed is <=.75 m/s. Four sines, multitone, impulse,
broadband and a gated 8 kHz transient are included. Cross-rate error comparisons
use exact common sample times spaced 1/300 s; there is no resampling in that test.
Actual A1/B1/AB comparisons instead use the unchanged native D1 trajectory. Actual
populations differ across sample rates, so those outputs are compared to their
own ideal transfer, not falsely asserted to be identical acoustic sources.
AB composition follows the renderer's double intermediate sum (zero D0) and final
float conversion before the D1 input. These six gated fixtures contain no frames
where A1 and B1 are both nonzero: AB evidence covers their temporal mixture, not
simultaneously active populations. An overlapping actual-source fixture remains
necessary before any future kernel acceptance; the synthetic multitone does not
replace that coverage. The present rejection does not depend on claiming it.

### Independent reference qualification

The first local attempt used 513/1025-tap Hann sinc. Its actual-output convergence
error reached .0092605, above epsilon/10, so most cases were INCONCLUSIVE. It is
retained at `build/flow-d1-remediation/study-v1`; it is not accepted reference evidence.
Its initial nearest-grid cross-rate comparison is also superseded by exact common
sample times. No runtime candidate was selected from it.

Final study-v2 uses a zero-padded Fourier extension with periods at least 4N/8N,
oversampling8/16 and centred order7 interpolation on the oversampled grid. The
former real Nyquist bin is correctly split when upsampling. This is an offline
bandlimited reconstruction assumption, not missing analog ground truth or a new
realtime path. Both period and oversampling are doubled for convergence checks.
An independent sinc-impulse test checks the reference including integer identity.
Maximum measured convergence: 2.0674e-5 relative RMS on actual outputs and
3.4044e-5 on synthetic finite-signal probes, below epsilon/10 in every case.
Analytic sine/multitone references use x(t-tau(t)) directly, without reconstruction.

## Functional Validation — R2/R3 finding

Machine-readable evidence is intentionally separate from the original CSVs:

| Evidence | Rows | Purpose |
| --- | ---: | --- |
| [STATIC](WATER_FLOW_D1_REMEDIATION_STATIC.csv) | 21 | Source-aware maxima and necessary gate results |
| [POINTS](WATER_FLOW_D1_REMEDIATION_POINTS.csv) | 252 | Fixed-Hz and supplemental normalized-frequency maxima |
| [FIXED_PATH](WATER_FLOW_D1_REMEDIATION_FIXED_PATH.csv) | 336 | Same four physical paths, four physical tones, three rates |
| [CROSS_RATE](WATER_FLOW_D1_REMEDIATION_CROSS_RATE.csv) | 56 | Moving analytic errors at identical physical times |
| [PROBES](WATER_FLOW_D1_REMEDIATION_PROBES.csv) | 168 | Moving signals, spectra/error, reference qualification and prototype timing |
| [ACTUAL](WATER_FLOW_D1_REMEDIATION_ACTUAL.csv) | 126 | Actual A1/B1/AB transfer, spectral/peak error and reference convergence |
| [RESOURCES](WATER_FLOW_D1_REMEDIATION_RESOURCES.csv) | 21 | Candidate history/coefficient storage estimates; native timing status |

All six causal candidates fail the combined gates. Representative results:

| Candidate | Max magnitude error at 44.1 kHz source band | Max actual relative RMS error across rates/profiles | Decision |
| --- | ---: | ---: | --- |
| Lagrange3 (current) | 12.5811 dB | .555288 | Reject |
| Lagrange7 | 15.2645 dB | 2.15065 | Reject |
| Lagrange15 | 52.3863 dB | 130.641 | Reject |
| Farrow7/FIR32 | 2.01265 dB | 810.271 | Reject |
| VFD LS32 | 2.01270 dB | 810.267 | Reject |
| VFD LS64 | 1.23804 dB | 586.453 | Reject |
| Centred sinc129 control | .00256216 dB | .0400500 | Ineligible: lookahead, also fails combined gates |

At 16 kHz the current kernel's worst magnitude error is 4.57142/3.25413/.247655 dB
at 44.1/48/96 kHz respectively. Thus the former normalized-band PASS excluded
important physical frequencies at the lower rates. New evidence rejects it.

At 96 kHz the optimized candidates pass the sampled static physical-band gates,
but their coefficient L1 sums reach about 3.4e4-3.6e4. Actual sources contain up
to .060647 of spectral energy outside the oscillator-support band in this small
fixture set. Their finite attacks are not bandlimited steady sinusoids, and the
one-sided predictive fits severely amplify that energy. This is numerical error,
not turbulence, absorption or physical amplitude modulation. A1/B1 must not be
filtered or recalibrated to conceal it. Actual spectral-magnitude and peak-error
metrics complement the static phase/group-delay measures; a time-varying FFT
ratio is not falsely reported as a stationary group delay.

R3: **NONE SELECTED**. R4 runtime replacement: **NOT PERFORMED**, following the
review plan's instruction to submit a finding when fidelity is unsupported.
Near-zero delays and immediate causal output are consequential constraints.
The experiment does not justify adding fixed latency, shifting the carrier,
changing the physical path or selecting a cheap failed kernel. Further research
may examine constrained worst-case/out-of-band FIR optimization and additional
representations; this study is not an exhaustive search or impossibility proof.
Allpass candidates were not studied: time-varying state/stability/phase analysis
would be required before treating that family as a solution.
If constrained causal filtering still cannot meet the gates, another research
direction is evaluation of the analytic emitter state at delayed time, rather
than reconstruction from emission samples alone. That would require a separately
reviewed source-interface/representation contract; it is not implemented here.
Likewise, centred filtering would require an explicit lookahead/latency decision.
Neither change may be silently introduced by a fractional-delay kernel swap.

Prototype wall times and memory are reported for all candidates. Farrow coefficient
storage is 2048 bytes; the table32/table64 designs need 131328/262656 bytes before
history. These are offline representation counts, not measured C++ object sizes.
Native candidate and integrated-new-kernel CPU/allocation timing is **NOT RUN**:
none passed fidelity to enter realtime implementation. No cost argument selected
or rejected a candidate, and no native budget PASS is inferred from Python timing.
The local prototype used Python3.12.4/NumPy2.3.4 on Windows, Intel Core i9-14900HX
(24 cores / 32 logical processors), with OPENBLAS_NUM_THREADS=1. These timings
include Python/NumPy execution and temporary arrays; they are not hard-RT bounds.

## Code Quality Review

Self-review checked dependency direction, immutable numerical setup, bounds,
source authority, SI units, exact sample-grid alignment, separate stereo history,
integer identity, analytic group-delay sign/units and Nyquist scaling. The evidence
test deliberately detects gain changes and rejects mismatched source equations.
No callback contains the new exporter, NumPy, filesystem, allocation or FFT work.
The existing runtime kernel remains unchanged and explicitly unaccepted.
Independent reviewer approval is still separate from this self-review.

## Comment & Documentation Pass — R6/R8

Changed: EXP-W-FD-001; original D1 execution header; this new finding and seven
numeric tables; Core Guide; Testing; Module Index; Project Status; Water/spike
READMEs and Research Mapping. The outdated blanket Flow redesign deferral now
distinguishes implemented D1 from further D1.1+ work. Historical data is retained.

Reviewed, no update required: Parameters and Architecture (no Host/state/latency
contract changed); Coding Plan and accepted brief (no milestone or perceptual
decision); Physical Governance, Code Standards and Document Governance (existing
rules followed); A1/B1 contracts and code (unchanged source physics/config);
Developer Sound Tools/Preview boundaries (no adapter or UI integration).
Equations, classification, source ownership, test ownership and open status agree.
Documentation consistency is checked again with final validation below.

Version policy: v1 remains research-unfrozen; repository inspection finds no
production/Preview consumer. No sonic change was selected, so model/config version
and numerical revision remain the original Lagrange3 at 3c95fe9. A future selected
replacement must record before/after revisions under canonical option A, or revisit
compatibility if a frozen external dependency is identified.

R7: **DEFERRED by user instruction**. Representative musical pad and human review
will be addressed together later. No remediated kernel exists for a new Sound Lead
pack; the original pack remains PRE-REMEDIATION. No human scores are inferred.
A1 dense96k stress, A1-GOV-ENV-001, B1 bass audibility/Size loudness and integration
findings all remain OPEN. Product mapping/adoption, UI, Host, physical calibration,
independent human acceptance and M2 Exit are not established.

## Reproduction

Use the documented MSVC environment and existing NumPy dependency. Configure,
safe-build and CTest serially for Debug, Release and ASAN; never run concurrent
heavy pipelines. Preserve a pre-edit renderer from 3c95fe9 before building.

```powershell
cmake --preset windows-release -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-release
ctest --preset windows-release --output-on-failure
$base = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
& "$base/frazil_water_flow_d1_source_probe.exe" build/flow-d1-remediation/sources-new
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_remediation_study.py `
  --sources build/flow-d1-remediation/sources-new `
  --output build/flow-d1-remediation/study-new
```

Create the parent directory first; source/study output directories must be new.
Local final data uses sources-v1 and study-v2; the FIXED_PATH table was added with
the study's `fixed_path_comparison` function after the main v2 run. The published
entry point generates all seven tables in one run. No audio, executables, expanded
spectra, local absolute paths or hashes are committed.

## Final Validation — R5

All three safe builds passed with six jobs, serially. Debug CTest passed35/35 in
143.08 s; Release passed35/35 in40.27 s. The initial Release34/34 run (42.48 s)
preceded registration of the new test and is not the final suite. The later
closed-form two-tap LS/Farrow check was additionally run through the focused
numerical CTest for each preset. Final numerical CTest contains ten checks after
the native cluster-rounding regression described below.

**D1-VAL-001 OPEN:** full ASAN CTest passed34/35 and failed the D1 renderer CLI
in285.25 s. ASAN reported an access violation while reading inside
`SharedExcitationAnalyzer::process`'s follower lambda (line59), through
`BubbleA1::process` and the offline renderer. The failing CLI call was its old
source-residual baseline render, before the compared D1 transfer. The new numerical
export/oracle CTest passed in that same full run. Raw failure is retained in
`build/flow-d1-remediation/ctest-windows-asan-first-failure.log`.

Git diff confirms no changes to the affected source code or renderer.
Code-path review found follower offsets0/2 with an added0/1, and the192-frame
window bounded by validated rates and its modulo cursor. That inspection does
not exclude corruption elsewhere. No root cause or environmental attribution is
established. A1-GOV-ENV-001 remains a separate historical finding; similar symptoms
do not prove common cause. No source rewrite, disabled assertion or sanitizer
suppression was used. Targeted repeats are reported separately below and cannot
erase the failed full run. Local sanitizer closeout is therefore **NOT CLEAN**.
Without code changes, targeted ASAN repetitions passed three CLI runs and three
new numerical-test runs (153.88 s total). Final focused numerical tests also passed
in Debug (5.32 s) and Release (1.48 s). Those bounded non-reproductions do not
establish a repair or replace the full-suite34/35 result.

**D1-VAL-002 OPEN:** the first preservation invocation terminated after1152
successfully decoded pairs (partial report retained in identity-v1). Windows
Application event1000 identifies Python3.12.4150.1013 / VCRUNTIME140.dll
14.38.33126.1, exception0xc0000005. No mismatched audio pair was reported, but this
is not a completed1707-pair PASS. A complete repeat enables PYTHONFAULTHANDLER=1
and unbuffered output in a new directory. The historical A1-GOV-ENV-001 Python
failure class has recurred; its root cause and any relationship to D1-VAL-001 are
unproven. Original logs and partial JSON are retained, not overwritten.
The complete identity-v2 repeat exited0 and passed **1707/1707 decoded pairs**:
840 A1,96 legacy,312 B1 and459 rate/partition/stereo cases against the preserved
3c95fe9 Release renderer. It used the existing `a1_governance_identity.py --matrix`
and the same six authorized sources/studies documented in the original D1 record.
This confirms that repeat's samples, not recovery of the interpreter or sanitizer
fault. No assertions or source configurations were changed for the repeat.
The [new preservation summary](WATER_FLOW_D1_REMEDIATION_PRESERVATION.csv) records
that complete repeat separately from the original30186b8-based evidence.

After the native failures, an independent full regeneration used sources-v2 and
study-v3 with faulthandler enabled. All six actual source traces reproduced exactly.
Six numerical tables (644 rows) matched study-v2 exactly after excluding prototype
wall-clock timings. The336-row FIXED_PATH supplement was originally generated in
a separate process without the main run's thread setting. Its maximum differences
are3.39e-11 dB,6.84e-12 rad,1.16e-6 ns and3.82e-12 complex error; these do not change
the finding. Regenerating that supplement with the matching thread setting exactly
reproduced all336 study-v3 rows. Both full study processes exited0. Published tables
remain study-v2; no content hashes were used.

A final renderer audit added the explicit float output conversion to the Python
AB sum and a regression that distinguishes float from double rounding. Study-v4
reran the complete study using the same native source traces. All non-timing data
matches study-v3: the current gated fixtures have no simultaneous A1/B1 samples,
so this correction does not change their values or any candidate decision. The
closed-form rounding test protects the formerly uncovered superposition case.
Final ten-check numerical CTests passed Debug/Release/ASAN in4.86/1.27/25.03 s.
These are focused checks, not a replacement full ASAN pass.

Final quality checks: Python AST parsing, C++ clang-format dry-run, Git whitespace
checks, Markdown internal links, portability and both scanner regression suites
passed. Runtime/production/config diff is empty. Documentation consistency: PASS
for reporting the actual implementation, numerical rejection and unresolved native
failures; this is not engineering/physical acceptance of D1.

Publication continues `codex/experiment/water-flow-d1` with ordinary commits/pushes;
old branches and the3c95fe9 review baseline remain available. No merge, forced update
or remote deletion. Hosted CI on the published commit is separate evidence and
cannot erase either local native failure.

Phase disposition: R0/R1 completed; R2 bounded offline comparison completed with
native candidate timing explicitly not performed; R3 rejected all tested causal
candidates; R4 not performed; R5 regression below; R6 new evidence retained alongside
old evidence; R7 deferred; R8 synchronized finding. This does not satisfy the full
D1-NUM-001 closeout checklist or authorize subsequent product/production work.
