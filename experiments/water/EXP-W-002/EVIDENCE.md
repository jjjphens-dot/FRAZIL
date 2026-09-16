# EXP-W-002 v0 engineering evidence

Date: 2026-09-16. Scope: bounded algorithm research authorized by Engineering Lead; not accepted
Water sound design, production DSP, M2 Exit or formal EXP-W-002 closure.

## Source and environment

- Source: candidate code/config changeset `f17cbf5`, based on `3438593`; measurements ran against
  the same DSP/config working contents before that commit. This does not imply clean-tree formal performance provenance.
- Formal performance provenance: **NOT RUN**. The standalone harness intentionally reports this;
  these are preliminary research observations, not a replacement for PERF-BASE-001 provenance.
- OS: Windows 11, 10.0.22631; CPU: Intel Core i9-14900HX.
- Compiler: MSVC 19.43.34808; JUCE: existing pinned 9.0.1 dependency.
- Local builds used the unchanged safety wrapper, six jobs, one pipeline at a time.
- Original build preflight refused at 2.99 GiB free; user released memory and the same checks passed.
  No bypass or content hash calculation was used.

## Executed checks

For each preset `windows-debug`, `windows-release`, `windows-asan`, serially:

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
```

All three builds passed; each complete CTest suite passed **15/15**. Supplemental final Debug/ASAN
builds also passed after the standalone benchmark's reporting-only cleanup; candidate DSP sources
were unchanged from their passing tests. Research tests cover baseline, analytic envelope behavior,
modal decay/extremes, event gating/capacity/stealing, Flow bounds, transient timing, independent
streams, ablation, fixed-seed reset/reprepare, mono/stereo isolation, finite extremes, rates
44.1/48/96 kHz, and callback partitions including 1/7/32/64/128/256/512/1024 samples.

Renderer tests decode samples and validate output length, no hidden carrier duplication at zero gain,
strict config rejection, overwrite refusal and cross-partition equality. Independent A/B/D outputs
agree with combined residuals within float rounding. Existing production regression tests passed;
no production source or Host/state registry was changed.

Formatting (`clang-format --dry-run --Werror` for research C++), portability, Markdown links,
explicit scans of newly added files, Python syntax checks and `git diff --check` passed.
Hosted CI is configured to enable these experiments but **has not run for this branch**.

## Diagnostic corpus

```powershell
python experiments/water/EXP-W-002/analysis/render_corpus.py --renderer <release-renderer> --output build/water-corpus-v0
```

Ten existing TESTDATA-001 inputs x eight modes A/B/D/AB/AD/BD/ABD/C = **80 PASS**. Each used the
checked-in defaults, seed 42, 128-sample blocks and three appended seconds of silence. The script
reused `tools/analyze_testdata.py`. It verified finite outputs, exact zero for initial silence,
default-config tail decay and combination consistency. Tail limits here are for defaults only;
longer allowed decay settings have separate bounds and are not claimed to expire within three seconds.

Maxima below are over the ten fixtures, including the source carrier; they are not acceptance
thresholds or perceptual scores. DC includes any DC already present in the input.

| Mode | Maximum peak | Maximum absolute DC | Largest final-100-ms tail peak |
|---|---:|---:|---:|
| A | 0.520501 | 0.000506274 | 0 |
| B | 0.505593 | 0.000505622 | 0 |
| D | 0.501183 | 0.000505576 | 0 |
| AB | 0.520501 | 0.000506333 | 0 |
| AD | 0.508509 | 0.000506288 | 0 |
| BD | 0.504196 | 0.000505636 | 0 |
| ABD | 0.508509 | 0.000506348 | 0 |
| C | 0.501483 | 0.000505806 | 3.15e-15 |

The renderer records residual RMS and event counts per file. Full WAVs, per-file analysis JSON,
`metrics.csv` and raw build/performance logs remain in ignored `build/`, not in Git. Re-run into a
new output directory. No licensed musical/listening corpus or loudness matching was claimed.

## Preliminary Release callback timing

```powershell
& 'build/windows-release/experiments/water/EXP-W-002/frazil_water_performance.exe'
```

48 kHz, 128 samples, stereo; base seed 20260916; 2000 warmup / 20000 measured blocks per case.
Same-run M1 baseline and M1 plus candidate use identical gated source blocks, with copying outside
timing. The gate exercises repeated Droplet onsets after warmup. Percentiles use nearest rank.
These wall durations include the M1 engine and candidate application loop; they are not process
CPU percent, a formal performance budget, a production integration benchmark or a stress maximum.

| Case | Mean us | P95 us | P99 us | Worst us | Mean increment over M1 us |
|---|---:|---:|---:|---:|---:|
| M1 | 0.613 | 0.8 | 0.8 | 1.7 | 0 |
| M1+C | 2.466 | 2.6 | 3.2 | 154.1 | 1.853 |
| M1+D | 3.941 | 4.1 | 4.9 | 98.4 | 3.329 |
| M1+A | 3.146 | 4.1 | 5.3 | 33.6 | 2.533 |
| M1+B | 2.519 | 3.4 | 3.9 | 82.6 | 1.907 |
| M1+AB | 4.967 | 6.1 | 6.9 | 54.4 | 4.354 |
| M1+AD | 6.603 | 7.3 | 7.8 | 174.1 | 5.990 |
| M1+BD | 6.352 | 6.5 | 9.1 | 239.9 | 5.739 |
| M1+ABD | 8.048 | 8.8 | 11.5 | 257.8 | 7.435 |

Scheduling noise and workload dependence remain; there is no claim of confidence intervals,
worst-case scheduling guarantees or meeting a future product CPU budget.

## Code Quality Review and Comment & Documentation Pass

- Reviewed all new DSP call paths: fixed bounded storage/loops; coefficient generation and dynamic
  delay storage only in prepare; no locks/I/O/APVTS/UI/hidden initialization or mutable globals.
  This is code-path evidence, not runtime allocation instrumentation.
- State ownership, signed channel excitation, random domains, reset/reprepare and tail semantics
  are explicit. Shared details have two actual consumers; no new production primitive is created.
- Complex-pole C realization avoids direct-form near-DC state amplification and has explicit
  excitation normalization. No hidden carrier, post limiter or auto makeup masks energy behavior.
- Experimental numeric ranges/defaults live in code/config/README; no product macro/Host/state
  mapping is frozen. Voice-stealing clicks, delay coloration and generic modal timbre remain
  listening risks. No speculative LOCAL-WDSP-07 refinement was added.
- Documentation changed: experiment index, proposal/checkpoint/evidence, module index, testing,
  implementation guide, Environment/CI entry point and branch-local project status.
- Reviewed without contract changes: Architecture, Parameters, Coding Plan, Perceptual Contract,
  Code Standards, Document Governance and ADR-0003/0005/0006. Cross-document scope/status consistency PASS.

## Outstanding work

Independent review; accepted EXP-W-001 reconciliation; licensed/loudness-matched musical listening;
Sound Lead accept/revise/reject decisions; any resulting individually justified refinement.
Pluginval/DAW, production Water integration, Host controls/state changes, Ice, production mode
transitions, formal CPU budget and release compatibility are outside this spike and NOT RUN.
No PR, merge or algorithm-adoption decision is implied by publishing the review branch.
