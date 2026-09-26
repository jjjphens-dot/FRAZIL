# Water D1 latency, bandwidth and comb research

Status: numerical study COMPLETE; native resources/preservation IN PROGRESS.
No runtime replacement or acceptance.
Baseline: c0188182d86a991b58addfe29e817141d8683bed, branch
`codex/experiment/water-flow-d1`; `git fetch --all --prune` found no divergence.
Authority: [EXP-W-FD-002](../../experiments/water/EXP-W-FD-002.md) and proposed
[ADR-0007](../adr/0007-minimum-practical-processing-latency.md).
Predecessor: [source-aware remediation](WATER_FLOW_D1_REMEDIATION.md), retained.

## Completed numerical screen

Corrected `study-v2` exited 0. It contains 105 filter candidates, 117 static-band rows,
312 moving-tone rows,208 cross-rate rows and4152 actual-source joint comparisons.
335 combinations pass the complete numerical screen; 55 retain the smallest passing
guard for their conditioner. These are numerical candidates, not 55 product choices.

The core tolerance is eps16=0.00730383689879. Worst independent reference convergence
is 2.06737522110e-5, below eps16/10. Historical native transfer versus the independent
polynomial evaluation differs by at most 1.11022302463e-16. Mandatory default-overlap
counts are 787/731/2918 frames at 44.1/48/96 kHz; overlap energy is reported separately.

Unfiltered actual-source control fails at 44.1 and 48 kHz throughout this bounded
search. At 96 kHz its minimum passing guard is 32 (Hann). No zero-guard candidate
qualifies. Conditioned minimum guards range 16–32 at44.1 kHz, 8–16 at48 kHz and 8 at
96 kHz. Adding FIR latency changes the total; no common instance/Host latency is
chosen in this research.

Illustrative 18 kHz-pass /Nyquist-safe-stop alternatives (all minimum guards):

| Rate Hz | Conditioner | Order | Guard | Total samples / ms | 20 kHz gain dB |
| --- | --- | --- | --- | --- | --- |
| 44100 | iir-p18000-s21609 | 4 | 32 | 32 / 0.7256 | -8.694 |
| 44100 | fir65-p18000-s21609 | 64 | 16 | 48 / 1.0884 | -8.947 |
| 44100 | fir129-p18000-s21609 | 128 | 16 | 80 / 1.8141 | -12.787 |
| 48000 | iir-p18000-s23520 | 4 | 16 | 16 / 0.3333 | -2.454 |
| 48000 | fir65-p18000-s23520 | 64 | 16 | 48 / 1.0000 | -0.780 |
| 48000 | fir129-p18000-s23520 | 128 | 16 | 80 / 1.6667 | 0.009 |
| 96000 | iir-p18000-s24000 | 22 | 8 | 8 / 0.0833 | -10.515 |
| 96000 | fir129-p18000-s24000 | 128 | 8 | 72 / 0.7500 | -0.267 |

All measured choices, including 16 kHz-pass tradeoffs and failed candidates, are
preserved in the CSVs below. This table does not accept high-frequency attenuation
or transient changes. Filter cross-rate phase/magnitude remain diagnostic, not a
passed perceptual or common-timbre contract.

- Comb mechanisms: [COMB](WATER_FLOW_D1_LATENCY_COMB.csv), [NOTCHES](WATER_FLOW_D1_LATENCY_NOTCHES.csv).
- Filter-only/actual/nonlinear/cross-rate: [FILTERS](WATER_FLOW_D1_LATENCY_FILTERS.csv), [FILTER_ACTUAL](WATER_FLOW_D1_LATENCY_FILTER_ACTUAL.csv), [NONLINEAR](WATER_FLOW_D1_LATENCY_NONLINEAR.csv), [FILTER_CROSS_RATE](WATER_FLOW_D1_LATENCY_FILTER_CROSS_RATE.csv).
- Kernel static/fixed/moving/cross-rate: [KERNELS](WATER_FLOW_D1_LATENCY_KERNELS.csv), [FIXED_HZ](WATER_FLOW_D1_LATENCY_FIXED_HZ.csv), [MOVING](WATER_FLOW_D1_LATENCY_MOVING.csv), [CROSS_RATE](WATER_FLOW_D1_LATENCY_CROSS_RATE.csv).
- Joint/coverage/composition/selection: [JOINT](WATER_FLOW_D1_LATENCY_JOINT.csv), [OVERLAP](WATER_FLOW_D1_LATENCY_OVERLAP.csv), [LEAKAGE](WATER_FLOW_D1_LATENCY_LEAKAGE.csv), [SELECTION](WATER_FLOW_D1_LATENCY_SELECTION.csv).


## Contract Review

The physical source/virtual-cluster/path/low-Mach reduction remains unchanged.
Guard support and filter implementation are ENGINEERING. Audible-band choices are
unaccepted PRODUCT_MAPPING research. No implied velocity, temperature, distance,
3D listener or water absorption is derived from an engineering sample delay/filter.
ADR-0005's Accepted Decision is immutable; ADR-0007 proposes its supersession and
retains independent Joint Gate as NOT RECORDED. User authorization permits this
offline study now, not a fabricated production latency approval.

The previous source-frequency envelope (up to 23239.2878 Hz) remains measured, but
the new study protects core 0–16 kHz, evaluates 16–20 kHz and separately studies
ultrasonic attenuation. Gates were written before candidate results. Attenuation
inside an explicitly declared transition remains a product tradeoff; it cannot
be renamed numerical fidelity or imperceptibility.

## Sources: support, limitations and access

All numeric study thresholds and guard/cutoff choices below are project engineering
decisions. No cited work calibrates them or proves a FRAZIL candidate accurate.

| Source | SOURCE SUPPORTS | SOURCE DOES NOT SUPPORT | Access in this revision |
| --- | --- | --- | --- |
| [Zheng & James 2009, Harmonic Fluids](https://www.cs.cornell.edu/projects/HarmonicFluids/) | Moving bubble oscillators with individual Helmholtz Green-function transfer | Virtual path calibration, shared-cluster equivalence, guard/filter values | Author project abstract retrieved; linked PDF fetch failed; earlier equation audit remains historical |
| [van den Doel 2005 author manuscript](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf) | Bubble acoustic emission as a liquid-sound primitive | D1 kernel, cutoff or Host latency | Fresh fetch timed out; reuse explicitly identified prior A1/B1 source audit |
| [Laakso et al. 1996 university record](https://research.aalto.fi/en/publications/splitting-the-unit-delay-tools-for-fractional-delay-filter-design/) | Fractional-delay FIR/allpass design families | Accuracy of our coefficients or a physical flow model | Bibliography/keywords retrieved; full text unavailable, not claimed read |
| [Smith, Lagrange interpolation](https://www.dsprelated.com/freebooks/pasp/Lagrange_Interpolation.html) | Polynomial interpolation and centred-delay design mathematics | Near-Nyquist fidelity without measurement | Author textbook page retrieved; product and barycentric implementations cross-checked analytically |
| [Vesma/Saramaki author notes](https://homepages.tuni.fi/tapio.saramaki/part3multi.pdf) | Two-sided interpolation and Farrow structure | A guaranteed accurate short kernel | Fresh fetch timed out; prior remediation access identified separately |
| [Wang et al. 2021, extended-high-frequency audiometry](https://pmc.ncbi.nlm.nih.gov/articles/PMC8394048/) | In this 162-person study, under-30 group responded through 16 kHz; 52.2% responded at 20 kHz; age/frequency dependence | Universal16/20 kHz cutoff, our 0.1 dB gate or music audibility for every listener | Indexed primary full-text result retrieved; direct page challenge prevented fresh full-page access |
| [Hearing-aid delay study 2022](https://pubmed.ncbi.nlm.nih.gov/35297723/) | Direct+processed sound interference; coloration-pitch discrimination 0.3–1 ms depending on condition | Pure delay being metallic, a universal 0.1 ms FRAZIL threshold | Primary abstract retrieved; publisher full page403. Prior detection thresholds cited inside that paper are not treated as this experiment's result |
| [Stuart et al. 2019 AES study](https://secure.aes.org/forum/pubs/journal/?ID=971) | Ultrasonic IMD can be measured and matters in some threshold experiments | Universal audible harm from high-resolution music or a justified FRAZIL cutoff | Publisher abstract retrieved; it explicitly limits relevance to ordinary recordings |
| [FabFilter processing modes](https://prod.fabfilter.com/help/pro-q/using/processingmode) and [bypass](https://www.fabfilter.com/help/pro-q/using/output) | Commercial latency/phase/pre-ringing and bypass-compensation precedent | Water physics or our guard value | Official documentation retrieved |
| [iZotope RX delay compensation](https://downloads.izotope.com/docs/rx6/58-rx-plug-ins/index.html) | Plugin reports delay to Host for compensation | Current FRAZIL Host/PDC validation | Official historical RX 6 documentation indexed; engineering precedent only |
| [SciPy buttord](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.buttord.html), [firwin](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.firwin.html) | Numerical design API semantics | Physical or perceptual acceptance | Official docs retrieved; local SciPy 1.17.1, online documentation newer |

The attachment's approximate 0.1 ms hearing-aid statement is not promoted to a
general threshold. The retrieved 2022 abstract reports discrimination 0.3–1 ms;
detection versus discrimination, insertion gain, spectrum and hearing status differ.
The comb equations themselves are independently verified without a perceptual claim.

## Implementation and reproducibility

Paths below `experiments/water/SPIKE-W-DSP-001/`:

- `tests/flow_d1_source_probe.cpp`: existing native sources/trajectory, plus actual
  current D1 and D0 outputs. Historical reference/edge fixtures unchanged. Additional
  overlap-reference uses seed 42, 997 Hz, .9-amplitude 40 ms pulses over a .4 bed during
  .05–.85 s. No A1/B1 config changes. Sustained-edge uses that excitation with the
  previously defined extreme-source config, but does not guarantee overlap.
- `render/flow_d1_latency_models.py`: separately owned conditioner and numerical
  guard kernel candidates. No production dependency, Host registration or parser.
- `render/flow_d1_latency_study.py`: separate comb/filter/kernel/combination audits;
  fresh output directories; independent analytic and Fourier references.
- `tests/flow_d1_latency_test.py`: analytic verification of causal support, path clock,
  polynomial coefficients, integer identity, Butterworth power, FIR timing and comb.

Initial fixture attempts showed zero edge overlap at one rate; failed generated
directories are retained. Input excitation was adjusted before candidate research;
no gate was weakened. Default overlap is mandatory at all rates; actual counts are
reported. Native helper values preserve the renderer's double-sum-to-float boundary.

Commands (repository root; generated data stays ignored):

```powershell
python tools/build_safe.py --preset windows-release
& build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_flow_d1_source_probe.exe build/flow-d1-latency/sources-v5
$env:OPENBLAS_NUM_THREADS='1'
$env:PYTHONFAULTHANDLER='1'
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_latency_study.py --sources build/flow-d1-latency/sources-v5 --output build/flow-d1-latency/study-v2
python experiments/water/SPIKE-W-DSP-001/tests/flow_d1_latency_test.py
```

## Functional and final validation

All local configure/build operations used the existing MSVC environment and
`python tools/build_safe.py --preset <preset>` with six jobs, serially. Full suites:

| Preset | First complete run | Complete repeat | Final affected-target validation |
| --- | --- | --- | --- |
| windows-release |36/37, Python export access violation |37/37,63.28 s |2/2 after final namespace/constants/input-bound edits |
| windows-debug |37/37,216.86 s |Not needed |2/2 after those same edits |
| windows-asan |36/37, existing CLI Python exception |37/37,485.28 s |Full repeat uses final C++ implementation |

Affected-target commands rebuild through the same safe wrapper, then run
`ctest --preset <preset> --output-on-failure -R "^frazil_water_(flow_d1_latency_native|droplet_b1_allocation)$"`.
Full commands use `ctest --preset <preset> --output-on-failure` with the experiment
and Preview enabled. Twelve independent analytical Python checks also passed.
Each native CTest executes24 IIR/FIR/rate/guard cases, including moving/integer
boundaries, float-range stress, seven block sizes, reset/reprepare and allocation.

The complete native resource matrix is pending the independent Hosted run after
local export failures; current preservation remains pending at this checkpoint. Musical-pad/listening, pluginval, DAW, Host PDC, production integration,
final bandwidth mapping and merge are NOT RUN. No subjective or Host result is
inferred from CTest. Exact-head hosted CI is a separate publication-time check.

D1-NUM-001 remainsOPEN/BLOCKED for runtime replacement: bounded offline solutions
now exist, but no final source bandwidth/filter/kernel is adopted. D1-VAL-001 has a
clean full ASAN repeat in this revision; historical native-fault root cause remains
unresolved. D1-VAL-002 recurred during Python data export and remainsOPEN. The new
Python generator exception is separately retained below; a clean repeat is not a
root-cause repair or evidence that these failures share a cause.

## Documentation Review

Changed: ADR0007/proposed supersession and ADR index/cross-links; Architecture,
Coding Plan, Parameters, Testing, Core Implementation Guide; D1 canonical contract
and preregistered EXP-W-FD-002; Physical Model Governance classification example;
Module Index, Project Status, Water/spike READMEs and Research Mapping; Environment
and the explicitly dispatched independent-machine CI study. Previous
execution/remediation records gain forward links only; their measured data remains.

Reviewed, no update required: Code Standards and Document Governance (existing
quality/classification/ADR rules applied); Collaboration Roles (existing independent
Joint Gate retained); accepted Water perceptual brief (no subjective acceptance or
macro changes); A1/B1 canonical contracts and typed descriptors (no source retune,
parameter/state or config changes); Developer Sound Tools/Preview (no consumer or UI
change).

Consistency checks keep proposed policy distinct from current zero-latency
artifact and failed historical kernel. Markdown/portability and scanner regressions
passed during implementation; final repeat follows measured-result synchronization.

Earlier attempts retained: an ASAN invocation was interrupted after33/36 tests,
and numerical study-v1 was deliberately stopped to correct FIR-delay/physical-clock
separation. Neither incomplete attempt establishes a PASS. Complete study-v2
supersedes the interim numerical observations; its gates were not relaxed.

Native validation addition: a test-only streaming realization imports prepared
coefficients and uses bounded stereo/path history. Its table interpolation is checked
against the untabulated model, then independently against the Fourier oracle. The
shared AllocationObserver extraction is required by two isolated test executables;
no instrumentation enters the renderer or plugin. Numerical gates are unchanged.

Boundary audit: all twelve native source fixtures end with at least 128 exact-zero
AB frames. Across all candidate conditioners, the largest next 128-frame filtered
tail relative to the filtered peak was 3.40e-80 / 1.13e-76 / 1.64e-77 at 44.1/48/96 kHz.
Finite-file closure therefore does not affect the stated numerical gates. The native
model comparison nevertheless continues filter state into 128 zero-input frames
before cropping, so it does not depend on premature filter-tail truncation.

## Interpretation boundaries

The ideal pure transfer M1 has unit magnitude. Equal direct plus transferred AB
(M3) has notches at (2k+1)/(2*tau); at the 5 cm /1484 m/s physical-range endpoint,
the first is 14840 Hz. This demonstrates the consequence of an extra direct AB path,
not evidence that a pure delay itself creates metallic sound. The strengthened
native/renderer check tests that composition error independently. Source resonances,
current interpolation error, intentional interference and filter ringing remain
separate mechanisms; none receives a human timbral label from these plots/numbers.

IIR's zero fixed lookahead is not zero phase delay. For example, the independently
screened 18 kHz-pass /Nyquist-safe-stop IIR has four poles at 44.1/48 kHz but 22 at 96 kHz;
its 20 kHz attenuation also differs materially. FIR alternatives retain different
extended-band content and add deterministic delay/pre-ringing. These differences
must remain visible in filter and cross-rate tables before any bandwidth choice.

## New validation failure retained

The first Release full suite passed 36/37; the native test driver crashed in
`numpy.savetxt` during coefficient export for 96 kHz/FIR65/guard8, before that case's
C++ child launch. Windows Application event 1000 records Python 3.12.4,
VCRUNTIME140 14.38.33126.1, exception 0xc0000005, offset 0x113de. NumPy 2.3.4 and
SciPy 1.17.1 were loaded. The incomplete fixture and failure log remain ignored local
evidence. A targeted complete 24-case repeat and 50 isolated same-case coefficient
exports (no native child) passed. No deterministic cause or code repair is established;
D1-VAL-002 remains OPEN/recurred. A later successful run must not erase this failure.

## Code Quality Review

Independent post-functional inspection checked cohesion, dependencies, ownership,
naming/constants, globals, includes and realtime operations. The streaming resource
prototype is isolated in a test namespace, with named table/history limits and
float-range input validation. Its filter, path FIFO and audio history belong to one
prepared instance. `process` performs bounded arithmetic/history access only; reset
fills existing state. The allocation observer is the existing replaceable-new
instrumentation extracted for two test executables, with no production linkage.
The model driver now continues filtered tails before cropping. Native coefficient
lookup error is measured separately from the original kernel approximation.

No mutable runtime globals, parser/Host state changes, hidden normalization or
source retuning were introduced. Benchmark file I/O, coefficient design/import,
allocation and clock sampling stay outside the sample-processing loop. This audit
supports the tested prototype path; it does not certify future renderer integration.

Reference-machine metadata for this revision: Windows 11 build 22631; Intel Core
 i9-14900HX (24 cores/32 logical processors); MSVC 19.43.34809.0; CMake 4.3.2.
Native resource results use the Release preset and this machine only. They do not
establish a portable hard budget. Local paths, binaries and generated audio are not
tracked; Git commit identity supplies repository provenance without content hashes.

The first complete ASAN attempt also encountered a separate existing CLI-driver
exception at `render_cli_test.py:145`: `TypeError: tuple indices must be integers
or slices, not NoneType`, while `i` is produced by `range` in a nested generator.
This is not a reported DSP sample mismatch or an ASAN memory diagnostic. It keeps
that full run non-clean; its cause and relationship to the native Python crash are
unproven. No assertion or tolerance was weakened to bypass the failure.

Source-band boundary review: existing A1/B1 voices cap instantaneous rise frequency
at min(sqrt(2)*f0,0.45*sampleRate). Their finite onsets, releases and voice stealing
still are not strictly bandlimited. This study does not claim to remove any alias
already present in generated source audio. The synthetic downstream square/cubic
audit separates its own IMD from foldback; it is not a complete source anti-alias
certification, playback-chain measurement or implementation of Ice.

Further isolation: the first native matrix terminated after59 completed, passing AB
rows, again during NumPy text export. This event identifies Python3.12.4's
python312.dll, offset0x15974, exception0xc0000005. An ignored isolated Python3.12.14
venv with the same NumPy2.3.4/SciPy1.17.1 also failed in the24-case native driver.
A reduced coefficient-generation/export loop with no C++ child, filtering or error
metrics failed after four complete24-export cycles. A trial standard-Python writer
passed exact-value round trips but then failed with0xc0000409 after five cycles.
That unproven workaround was withdrawn. These observations do not isolate the
underlying NumPy, interpreter, runtime or machine cause; no dependency or hardware
repair is claimed. Failed logs and partial results remain separate, never pooled
into a complete matrix. The independent Hosted Windows study is now prepared to
complete and cross-check evidence outside this local environment.
