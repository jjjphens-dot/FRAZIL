# Water A1 Convergence Round 03

Status: **P0 PRESERVATION FAILED — downstream studies stopped. NO CANDIDATE SELECTED.**
Implementation/self-review: engineering agent. Perceptual selection: Sound Lead.

## Live baseline and scope

Fetched origin from jjjphens-dot/FRAZIL. Remote R2 branch
`codex/experiment/water-a1-listening-convergence-r2` and local HEAD both
`d01defa91363cf7b3606cdb09a5156c55d06b14c`; clean, ahead/behind 0/0.
GitHub queries returned no open PR and no workflow runs for that branch.
R3 work branch: `codex/experiment/water-a1-lifecycle-convergence-r3`.
The existing in-boundary checkout is reused; unrelated main-checkout edits preserved.
R2 recorded Debug/Release/ASAN 40/40; these are historical, not R3 validation.

Goal: distinguish physical/reduced/product/engineering mechanisms; establish
sample-exact explicit pre-start culling before lifecycle/depth/rise/Persistence
ablations. Preserve Minnaert/damping/oscillator, source excitation, populationGamma,
Motion, alpha default, B2/D1, Host/state and production. No hard band caps or solvers.

## Ordered checkpoints

1. Contract/primary-source review; inspect full lifecycle including pending replacement.
2. P0 explicit cull versus exact R2 baseline at three rates; stop if any sample differs.
3. Only if step 2 passes: state semantics, L0/L1/L2, beta, depth gamma, selective rise,
   P0/P1 Persistence, scheduler audit, performance, small listening pack.
4. Separate code-quality review, comment/documentation pass and final validation.
5. Human coverage/selection gates; no A1+B2 acceptance or D1 changes before them.

Known constraints: missing piano/pad inputs; human judgments cannot be inferred from
numeric events. The R2 1808/2772 below-floor starts are a numeric/lifecycle finding,
not 65% measured inaudibility. Initial gate must also examine pool saturation:
silent requests may influence stealing despite emitting zero themselves.

The P0 counterexample below blocks the remaining checkpoints. Runtime stays R2.
Preview defaults to Legacy, with explicitly selectable Reworked Fluid; no live Preview
session was inspected. A1 reference config remains v2, optional depth-amplitude mapping
v3. B2 remains independent; D1 retains Historical Lagrange3 and pending C6/C7.

## P0 result: zero output does not imply zero resource effect

`BubbleA1Voice` initializes real=1, imag=0. Its first output is zero; after envelope
decay, the absolute floor retires an initially below-floor event. However, the pool
has already allocated it or selected a sounding victim and begun its steal release.
Skipping the request removes that release, changing the victim's output.

The new [native probe](../../experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_cull_gate_tests.cpp)
reuses the unchanged model and pool, comparing historical trigger versus skipped
below-floor trigger. A third unchanged pool provides the exact-zero control.
Default model, capacity64, 10mm bin, unchanged physical frequency/damping/pole/tau;
full case starts64 events with L/R amplitudes .01/-.005, then processes32 frames.
The next request has lifecycle amplitude 1e-5/-5e-6, below the unchanged -80dB floor.
v2 uses the same render amplitude; v3-role probe uses .01/-.005 render amplitude.
An isolated voice independently confirms zero first output and immediate retirement.
One second of pool output is compared at each rate. No RNG or scheduler is involved.

| Rate Hz | Empty-pool max delta | Full-pool max delta | First difference frame | Historical/cull steals |
| --- | --- | --- | --- | --- |
| 44100 | 0 | 0.0095480084419250488 | 33 | 1/0 |
| 48000 | 0 | 0.0095490813255310059 | 33 | 1/0 |
| 96000 | 0 | 0.0095496177673339844 | 33 | 1/0 |

Both amplitude roles produce these values; unchanged controls are all exactly zero.
Frames are zero-based. [12-case numeric CSV](WATER_A1_R3_CULL_GATE.csv).
CTest PASS means the counterexample reproduces; CSV `preservation_gate=FAIL` means
the proposed cleanup fails sample identity. Six full-pool rows fail preservation.

This synthetic pool-level counterexample is not a musical render or evidence of how
often actual music saturates the pool. Manual simultaneous fill isolates resource
behavior; it does not simulate A1's one-request-per-frame scheduler.
Another code-derived limitation, not a measured count: an event initially just above
floor can cross below it on its first decay update, also dying before nonzero output.

## Stop decision and remaining scope

Task section8 explicitly requires stopping on nonzero delta. Runtime culling is
therefore NOT installed. L0/L1/L2, beta/gamma/rise/Persistence, scheduler audit,
performance sweep and new listening packs are NOT RUN. Canonical-source renders and
the nine-mode/three-rate preservation matrix are not claimed: the prerequisite has
a counterexample. B2/D1 and production/Host/state code have no changes.

There is no new runtime `firstNonZero` counter. Existing `started` includes events
with zero emitted samples; `completed` includes steal retirement. Trace-payload /
render-event splitting is deferred at this gate, not claimed unnecessary or complete.

Next decision requested: keep historical rendering as L0 and explicitly permit
pre-start culling as a sound-changing research candidate before lifecycle ablations;
alternatively keep historical resource behavior and limit the next change to honest
event statistics. Do not hide ghost voices to call rejection sample preserving.

## F1–F9 answers

| Item | Finding / evidence limit |
| --- | --- |
| F1 silent starts | Zero phase plus post-process absolute-floor retirement. R2 default alpha1.5:1808/2772 initial starts at/below floor,65.22%; a lifecycle lower bound, not audibility. Sample-preserving cleanup failed under saturation; not installed. |
| F2 A05 | Count/emission mismatch is established. Event-rate versus effective-density contributions to human A05 are not quantified. The pool finding adds an indirect effect on sounding victims; no perceptual percentage is claimed. |
| F3 beta | beta10 suitability remains unknown. Literature provenance does not choose FRAZIL tuning; R3 sweep stopped. |
| F4 depth gamma | Larger render amplitude can still die under the old lifecycle rules, as the separate-role probe confirms. No replacement-policy density gain demonstrated. |
| F5 rise | Historical R2:10 rising starts/2772 starts,0.36%. This is not the fraction among nonzero-emitting events. No R3 fixed-sine improvement or chirp/chorus judgment. |
| F6 Persistence | R2 excluded direct initial-amplitude multiplication. Tail energy, overlap, P0/P1 trajectory, lifecycle and stealing contributions remain unresolved. No R3 P0 recommendation. |
| F7 classification | New code is ENGINEERING verification only; no physical/reduced/product equation changes. |
| F8 commercial precedent | Layered controls, variation and bounded complexity only; no physical validation. |
| F9 candidate | NO CANDIDATE SELECTED. A1+B2 final acceptance gated; B2 trigger inflation OPEN; D1 C6/C7 pending. |

A01–A07 partial human observations remain distinct from root-cause hypotheses.
A08–A22 are not reclassified as confirmed failures. Missing piano/pad inputs and
Sound Lead R3 listening remain gaps; synthetic tests cannot close musical coverage.

## Classification and primary references

- PHYSICAL: unchanged Minnaert-type resonance and radius-dependent empirical damping;
  the damping fit is not first-principles derivation.
- REDUCED_PHYSICAL_MODEL: independent bubble population, adjustable alpha, depth proxy,
  selective physical-damping rise, source excitation proxy and occupancy scheduling.
  Audio power is not actual fluid velocity.
- PRODUCT_MAPPING: Motion, Persistence, depth-amplitude gamma, P1 rise and presentation.
- ENGINEERING: normalization, floor, capacity, stealing, transport and the new numeric probe.

[van den Doel](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf) supports
damped single bubbles, stochastic populations and selective rise.
[Phillips et al. 2018](https://www.nature.com/articles/s41598-018-27913-0) supports
trapped-bubble resonance, with distinct underwater/airborne radiation.
R3 retrieval of the former timed out; the latter's PMC mirror was access-blocked.
These retain prior R2 provenance, not a new full-text verification claim.
Rechecked author pages for [Langlois et al. 2016](https://graphics.stanford.edu/~djames/publication/toward-animating-water-with-complex-acoustic-bubbles/)
and [Xue et al. 2023](https://graphics.stanford.edu/papers/coupledbubbles/): omitted
geometry/topology/transfer and coupling delimit the model; they authorize no new solver.

Rechecked commercial primary sources:
[Chromaphone 3 manual](https://www.applied-acoustics.com/chromaphone-3/manual/) separates
resonator/material/excitation controls and documents mode-count/polyphony CPU cost;
[Kaivo](https://www.madronalabs.com/products/kaivo) describes varying initial conditions
and separate spatial/pickup controls. These support engineering/product layering, not
FRAZIL bubble physics or adding coupled/spatial solvers.

## Development phases and documentation review

1. Contract Review: carried forward canonical R2 review at the unchanged baseline;
   rechecked lifecycle, deferred replacement and accepted perceptual/physical boundaries.
2. Implementation: one offline characterization target using existing CMake/CTest/ASAN
   registration; no parallel DSP or runtime policy.
3. Functional Validation: three-rate counterexample and exact unchanged controls.
   Initial compilation used named members on StereoFrame; corrected to array indexing.
4. Code Quality Review: separately checked ownership, causal isolation, finite/exact
   comparisons, v3 lifecycle selection, bounded loops and test/gate status distinction.
   Pool allocation is offline setup to avoid large stack copies. No callback/RNG/I/O change.
5. Comment & Documentation Pass: synchronized this evidence, A1 contract/execution,
   Testing, Project Status, debug guide, Module Index and SPIKE README. R1/R2 evidence
   remains relevant and is retained; no obsolete evidence was silently removed.
6. Final Validation: all three safe builds and focused A1 suites passed; this validates
   the characterization, not the failed preservation gate.

Reviewed without changes: Architecture, Parameters, Coding Plan, accepted brief,
physical governance, B2 contract/execution, Developer Sound Tools, Round01/02 evidence,
Code Standards and Document Governance. Contract/default/Host behavior and acceptance
gates stay intact. Production/realtime/performance contract impact: N/A.
No ADR or Joint Gate change is required for this isolated characterization.

## Validation and publication

Serial `cmake --preset <preset>` and `python tools/build_safe.py --preset <preset>`
completed for windows-debug, windows-release and windows-asan with the existing research
opt-ins and six-job safety wrapper. Then, for each preset:

```powershell
ctest --preset <preset> -R frazil_water_bubble_a1 --output-on-failure
```

Each focused suite passed 3/3: existing native A1, new cull characterization, existing
A1 CLI. Debug elapsed12.68s; Release3.95s; ASAN28.11s. Release probe output is the tracked CSV;
Debug produced identical rows. Configure/build/test logs remain under ignored
`build/safe-build/r3-*`. The initial compile error and subsequent correction are recorded
above; only the final successful builds support these results.

`python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/check_vscode_tasks.py`, changed-C++ `clang-format --dry-run --Werror`
and `git diff --check`: PASS. Cross-document status consistently says gate failed,
runtime unchanged, downstream not executed. Source diff review confirms no changes to
production, DSP, Preview or renderer implementation.

Full preset suites, new timing sweep, pluginval/DAW, human R3
listening, final A01–A06 reevaluation and candidate selection are NOT RUN.
Publish only test, portable numeric CSV and synchronized evidence; local audio/logs
remain ignored. No merge or production adoption is implied.
