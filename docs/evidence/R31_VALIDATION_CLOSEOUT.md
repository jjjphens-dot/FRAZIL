# R3.1 validation closeout — Phase 0

Status: PHASE 0 OPEN / STOP BEFORE PHASE 1. Bounded fixes and reproducible evidence delivered; crash root causes and realtime margin are unresolved. HUMAN NOT ASSESSED.

## Authority and recovery state

### Phase 0 follow-up intake

Latest authority: [current-status plan](../research/water-current-status/FRAZIL_Water_Current_Status_and_Next_Agent_Plan.md), upstream `9b027ecce53429aee61ea050da8d86232dd8f8d7`. Intake HEAD/remote `2c20de25d10471290802ea9a766e6e341ced9359`, clean, ahead/behind 0/0; PR #43 remains draft/open. Its Hosted Windows Debug run 37102955436 completed SUCCESS. Original local failures below remain valid.

Follow-up status: PHASE 0 OPEN. Owner: engineering agent; no delegated workers. Publisher completeness/live binding, renderer case capture, controlled parent-runtime A/B and three complete timing rounds are executed below. Frozen-head full-suite execution status/results are maintained in the linked PR #43 validation table so recording the result does not move the validated commit. Next engineering checkpoint: unresolved failure/cost diagnosis. No sound/lifecycle policy changes.

CONFLICT FOUND: older status sections and the Perceptual Contract's illustrative Decay paragraph retain pre-acceptance wording although the accepted brief and current framework header record acceptance. This follow-up uses the current accepted brief for perceptual authority and treats dated execution sections as historical; it does not rewrite controlled contracts or infer algorithm acceptance from implemented code.

Follow-up implementation checkpoint: publisher now checks exact fixed reference/band/preservation/performance keys and live Git HEAD/clean status plus the two binary digests. Evidence regressions: six tests PASS, including matrix mutations, wrong provenance, strict expected CLI exits and environment construction. A1 CLI now captures 183 individually identified children; focused Release and ASAN PASS against the retained cb9ddab binaries. The first harness trial failed with an obsolete `check` keyword; this test-adapter error was repaired and its log retained.

Controlled parent-PATH diagnostic (two preregistered A/B rounds, same retained ASAN native binary, Python 3.12.4, NumPy 2.3.4 and original 24-case workload): A PASS / access violation; B PASS / PASS. A inherits the CTest MSVC runtime path; B removes only that path from the parent and restores it for native children. The second A interrupted 48000 / IIR / Kaiser guard64 in `np.savetxt`, before native launch. Stack again points to NumPy `write_normal` and `savetxt`. Environment interaction is implicated, not proven; one failure in two A trials and none in two B trials do not establish a repair. Default CTest PATH and all native timeouts remain unchanged. Raw evidence stays in `build/r31-closeout/parent-environment-ab/`.

Separate code-quality review: no DSP/header changes; native evidence remains offline; admission/order/RNG unchanged. CLI config-rejection cases accept only exit 2, never an arbitrary crash. Workload counters use a separate untimed benchmark loop, with no per-voice counting branch in timing mode. The next step is a clean Release build and three-round resource/provenance evidence, followed by a final frozen-head validation.

- User plan: [B2/D1 convergence](../research/water-b2-d1-convergence/FRAZIL_B2_D1_Convergence_Agent_Plan.md), upstream `be3fdf606f49e5e5139d4847cc1deb9008036042`.
- Research intake: `e0b37e0e06d7b4fc8c2c81e05ce05556fd66ddd4`, clean, origin synchronized (0 ahead / 0 behind); PR #42 draft; Hosted Debug PASS 41/41.
- Work branch: `codex/fix/water-r31-validation-closeout`. The plan branch is based on older code; only its single plan commit was imported, without reverting research implementation.
- Implementation owner: engineering agent. Sound acceptance: Sound Lead; D1 C7 remains a Joint Gate.
- Current step: Contract Review, Implementation, Functional Validation, separate Code Quality Review, Comment & Documentation Pass and Final Validation executed. Phase 0 acceptance did not pass.
- Success criteria: request-local typed outcomes, consistent snapshots, clean exact-R3 rebuild provenance, sample-exact L0, explained native failures, full Debug/Release/ASAN on one implementation commit, measured observation overhead.
- Next checkpoint: independent review of captured Python text-write access violation, renderer GS fail-fast and observability cost. No B2 work before these Phase 0 findings are explained and the gate is explicitly passed.
- Blockers retained: previous Debug native provenance timeout, ASAN native latency SegFault and 96 kHz/1024 dense timing overruns. No later passing run erases these observations.
- Active workers: one engineering agent; no delegated work.

## Boundaries

All changes are ENGINEERING diagnostics/test/evidence tooling. No A1 tuning, physical equation, B2 detector, D1 DSP/kernel/filter/policy, production, Host, state or latency change is authorized in Phase 0.
Do not start Phase 1/B2 sonic work while Phase 0 has unexplained failures.

The new plan explicitly requires binary SHA-256 for the rebuilt historical/current renderer pair. This is a narrow provenance exception to the default no-hash rule; no source, audio, log or dependency hashes are required.

## Validation ledger

Historical failures remain in [R3.1 evidence](WATER_A1_LIFECYCLE_R31.md).

- First Release safe build PASS; A1 focused CTests 3/3 PASS (5.60 s); evidence-tool regressions 3/3 PASS.
- Clean exact-R3 Release rebuild PASS. The requested baseline worktree and all outputs are inside the configured workspace boundary; the existing pinned JUCE dependency is shared read-only through a checked in-boundary junction.
- First instrumented Python/native reproduction against the retained R3.1 ASAN executable: 24/24 cases PASS. This is diagnostic evidence, not full-suite closure.
- Original Windows Application event at 2026-10-03 04:37 local: faulting application `python.exe` 3.12.4150.1013, module `python312.dll`, exception `0xc0000005`, offset `0x36ae7`. It identifies the parent process; it does not establish the corruption source. WER report survives, but its referenced temporary dump is no longer present. The original residual directory later localized the interrupted case to 48000 / FIR65 / Kaiser guard64, before native launch. Its full backtrace is unavailable: ROOT CAUSE UNRESOLVED.
- Native case tooling retains command, timestamps, exit code, stdout/stderr and progress files in ignored build directories; source-probe whole-run timeout remains 120 s, native-child timeout remains 240 s. Python faulthandler records parent stacks on future faults. The native process uses Microsoft's documented [ASAN dump hook](https://learn.microsoft.com/en-us/cpp/sanitizers/asan) for future sanitizer-detected faults.

Code Quality Review: no new RNG draws, admission/voice/order changes, heap/lock/I/O on DSP paths or event-state growth. New capacityDropped records use the existing bounded queue and overflow accounting. Pending cancellation bookkeeping precedes the snapshot. Offline logging remains outside allocation/CPU measurement loops. No filter/kernel registry or D1 source tuning changes.

Comment & Documentation Pass: trace v4 outcome semantics, retained native evidence, publisher layout and research-only status documented. Architecture, Parameters, Coding Plan and physical governance reviewed unchanged. Initial repository checks passed before full validation. Final portability scanning after tracking the new tests caught synthetic absolute-path literals in the publisher rejection test. They were rewritten as constructed strings following the existing scanner-test convention, preserving the exact runtime values; no scanner exception was added.

## Documentation impact

Targeted research diagnostics, native test behavior and evidence status. Update this report, affected module/testing/status documentation and R3.1 navigation as implementation settles. Architecture, Parameters, Coding Plan and physical contracts remain unchanged because no corresponding contract changes.


## Exact source and binary provenance

All three full preset runs below used implementation commit `cb9ddab1d22853b7a43773dd8565965cc0bda5d9`; configure/build began with a clean tree. Subsequent additions are evidence/documentation, the independently exercised manual text-write reproducer and equivalent construction of synthetic path strings in the evidence test. DSP and compiled test sources are unchanged. No passing final-artifact ASAN claim is made.

| Role | Source SHA | Tree at build | Renderer SHA-256 (explicit plan requirement) |
| --- | --- | --- | --- |
| Rebuilt R3 | `1092ba01007d70a66ac5204c7d0d7cb070421972` | clean, detached dedicated worktree | `07bab8eb8a2ed20697a8d81530e31fe40704aee0e8a0b5e6382604b3e760e2cc` |
| Current L0/L1 | `cb9ddab1d22853b7a43773dd8565965cc0bda5d9` | clean | `23f0f8bf318a36a5242250d16e82a28227b616ed34103dd91aadd188e0ccd23a` |

Both use `windows-release`, MSVC 19.43.34809.0, CMake 4.3.2 and the repository-pinned JUCE commit `e18f7f506c0b96f2c738a0bcd7fe6467a5005ad8` with its existing compatibility patch. Machine: Windows 11 build 22631, Intel Core i9-14900HX, 24 cores / 32 logical processors. Python 3.12.4; NumPy 2.3.4. Only the two required renderer binaries were hashed. Local paths and raw logs remain ignored.

## Full validation and first failures retained

| Preset at cb9ddab | Safe build | Full CTest | Seconds |
| --- | --- | --- | --- |
| Release | PASS | 42/42 PASS | 69.42 |
| Debug | PASS | 42/42 PASS | 242.06 |
| ASAN | PASS | 40/42 FAIL | 568.45 |

The extra CTest is the evidence-tool regression: deterministic publication, missing/dirty/private evidence rejection, and retained child failure/timeout streams. The ASAN failures are `frazil_water_flow_d1_latency_native` (parent SegFault, 5.38 s) and `frazil_water_bubble_a1_cli` (renderer fail-fast). All runs used serial six-job safe builds; no timeout was raised and no full failure was replaced by a focused PASS.

### Python/native-test failure localization

Original R3.1 failure: retained directory ends at 48000 / FIR65 / Kaiser guard64. Only `coefficients.txt` exists in that case: 359933 lines versus 528579 required. Its preceding guard32 native case completed reset/partition/allocation checks. No input file or native launch had been reached for the interrupted guard64 case.

The new full ASAN run reproduced the same Windows fault offset `python312.dll+0x36ae7`, now in `RATE_44100_IIR_kaiser-g32_moving-zero-path`. The previous guard16 child exited 0, last frame 4111. The interrupted guard32 case contains 121237 of 266318 coefficient lines and no native launch. `parent-fault.log` records:

```text
Windows fatal exception: access violation
numpy/lib/_npyio_impl.py:1548 write_normal
numpy/lib/_npyio_impl.py:1636 savetxt
render/flow_d1_latency_native_study.py:35 run_native
```

This identifies a parent Python text-write failure before the affected native case, not a detected D1 ASAN violation. It does not prove whether CPython, an extension, runtime state or the machine caused memory corruption. There is no native ASAN report/dump for a child that had not launched.

The new `tests/native_text_write_repro.py` exercises the same scalar text format without DSP or a native child: four 528513-value writes passed both under LLDB and directly. This bounded negative result does not clear the full-suite crash. An attempted LLDB attachment to a source probe found that it had already exited; that test subsequently passed and supplied no hang backtrace. Download/extraction of optional official Python symbols was rejected by automatic approval review (`blocked by policy`, no more specific reason); it was not executed. ROOT CAUSE UNRESOLVED.

### Debug timeout and separate renderer failure

The original 120 s Debug source-probe timeout has no recovered per-case history. Current retained 12-case runs completed in 2.984 s Release, 18.657 s Debug and 87.078 s ASAN; every case reached its final frame and retains A1/B1 event counts. The original timeout was not reproduced; no evidence justifies attributing it to I/O, CRT, a hang or A1 observation cost. ROOT CAUSE UNRESOLVED.

ASAN also failed in the existing A1 CLI component-sum loop while invoking a B/D residual component: child code `3221226505` / `0xc0000409`, empty stdout/stderr. Windows event offset `0x64cbdd` resolves with the built renderer PDB to MSVC `__report_gsfailure` (`gs_report.c:220`). No caller stack or exact rate/component was retained by the historical CLI helper, so this is a separate OPEN finding; the fault site alone does not establish a root cause. No algorithm or sanitizer was disabled to make it pass.

## Rebuilt-baseline numerical evidence

[Preservation CSV](r31-closeout/WATER_A1_R31_PRESERVATION.csv): 57/57 max delta 0, comprising 27 old/new mode/rate cases, 15 trace/partition cases, three dense A1 cases, six real-source old/new cases and six L1 Full equation checks. Thus 51 L0 comparisons are sample exact. Rates: 44.1/48/96 kHz for synthetic regressions; the six music sources retain native rate. Synthetic signals are not musical listening coverage.

[Reference CSV](r31-closeout/WATER_A1_R31_REFERENCE.csv) and [bands](r31-closeout/WATER_A1_R31_BANDS.csv) reproduce the original totals:

| Policy | Requested | Started | First nonzero | Pre-start culls | Completed without nonzero | Steals/drops |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| L0 | 2772 | 2772 | 959 | 0 | 1813 | 0/0 |
| L1 | 2772 | 964 | 959 | 1808 | 5 | 0/0 |

No new sound benefit or baseline selection follows. A1 remains Historical L0. Source gain, radius/depth/rise, seed42 and other configs are unchanged. L0/L1 reference pairs remain exact; real pad/piano coverage and human decisions are still missing.

## Performance finding retained

[Canonical timing matrix](r31-closeout/WATER_A1_R31_PERFORMANCE.csv): 120 rows, all rates and 64/128/256/512/1024 capacities, reference/dense, before/L0/L1/L1+trace. Additional 30-row [observer-only](r31-closeout/WATER_A1_R31_OBSERVER_ONLY.csv) and [L0 trace](r31-closeout/WATER_A1_R31_TRACE_L0.csv) probes separate callback payload work from queue production. One fixed serial sequence, no retries to obtain a convenient worst time; all trace loss counters are zero.

96 kHz / 1024 dense / block128 (deadline 1333.33 us):

| Variant | Mean us | P95 us | P99 us | Worst us |
| --- | ---: | ---: | ---: | ---: |
| Exact R3 rebuilt | 445.030 | 517.6 | 659.9 | 1043.6 |
| L0 observed, no consumer | 518.930 | 616.7 | 840.4 | 1598.0 |
| L1 | 499.146 | 595.6 | 703.3 | 1122.7 |
| L1 + queue | 499.757 | 605.4 | 753.8 | 1387.3 |
| L0 + count-only observer | 519.829 | 614.6 | 748.1 | 1278.7 |
| L0 + queue | 514.326 | 604.3 | 751.1 | 1393.0 |

The no-consumer L0 mean exceeds rebuilt R3 by about 16.6% in this cell. The count-only callback increment is small in this run; the queued mean is slightly lower than no consumer, illustrating measurement variance rather than negative queue cost. Sidecar access, branch/cache effects and compiler layout are not independently isolated by this experiment. Historical 1430/1335.6/1439.3 us overruns remain valid earlier observations. Neither the new L1 worst below deadline nor a single wall-time matrix grants realtime acceptance. Performance closeout remains OPEN.

## Reproduction and publication

1. Rebuild exact R3 in a clean worktree and current code with the same `windows-release` preset and pinned toolchain/dependency; all builds use `python tools/build_safe.py --preset <preset>`.
2. Run `render/a1_lifecycle_r31.py --baseline <rebuilt-renderer> --renderer <current-renderer> --compare-l1 --input <source> ... --output build/<new-study>`.
3. Run the baseline performance executable and current executable with no selector, `--l1`, and `--l1 --trace`; save the four `performance-{before,l0,l1,l1-trace}.csv` files in that study. Additional `--observer-only` and `--trace` runs are separate diagnostics.
4. Record `PROVENANCE.json` with baseline/current `source_sha`, `working_tree=clean`, `binary_sha256`, toolchain and preset; raw paths stay local.
5. Run `render/a1_lifecycle_r31_publish.py build/<study> --output <output> --expected-baseline-sha 1092ba01007d70a66ac5204c7d0d7cb070421972 --expected-current-sha <study-build-head> --baseline-binary <rebuilt-renderer> --current-binary <current-renderer>`. The follow-up requires both live binary checkouts at those exact clean revisions and matches the two supplied binary digests against provenance. Historical studies cannot be relabelled as the current build. It validates complete keyed matrices before writing, derives ratios/deadlines and normalizes empty cells to `N/A`. The original publisher produced identical bytes in two publications. No manual spreadsheet merge or input audio publication.

The versioned closeout folder is the canonical evidence for this run. Original R3.1 CSVs remain historical snapshots and are not silently overwritten. Private audio and the local derived pack remain ignored. Exact command/log details live under `build/r31-closeout/`; native failures also retain uniquely named per-run directories under build.

## F1–F12 / next phase

| Item | Disposition |
| --- | --- |
| F1 pending snapshot | Fixed; direct and deferred drop totals independently tested. |
| F2 request outcome | Trace v4 exposes started/pendingReplacement/preStartCulled/capacityDropped for every prepared request. Deferred start/cancellation is not a second admission. Transport loss remains explicit. |
| F3 R3 binary provenance | Clean exact-SHA rebuild completed; required two binary digests and toolchain recorded above. |
| F4 timeout root cause | UNRESOLVED; current full Debug passes with original 120 s timeout. |
| F5 SegFault root cause | Parent text-write stage/cases and Python stack captured; underlying cause UNRESOLVED. Renderer GS failure separately OPEN. |
| F6 B2 extra-event origin | NOT RUN; Phase 1 gated. |
| F7 HYBRID_EPISODE | NOT IMPLEMENTED; Phase 2 gated. |
| F8 detector human decisions | NOT ASSESSED. |
| F9 combined A1+B2 baseline | GATED; Historical L0 retained. |
| F10 D1 C6 differences | NOT ASSESSED; missing musical coverage not substituted. |
| F11 D1 selection | NO CANDIDATE SELECTED. |
| F12 production latency/PDC gate | NO; C7 incomplete. |

Phase 0 STOP conditions encountered: reproduced unexplained full-suite parent crash, separate renderer fail-fast, unresolved original timeout and inadequate realtime margin. DO NOT START NEXT PHASE. Next authorized work remains bounded failure diagnosis and independent review; no B2 sonic work, D1 algorithm search, product mapping or production adoption.

## Final documentation review

Changed: this execution/root-cause/provenance report, versioned numeric tables, R3.1 follow-up link, Testing, Project Status, Module Index, debug guide and SPIKE README. The new manual reproducer is scoped to the observed Python fault.

Reviewed unchanged: Architecture, Parameters, Coding Plan, DSP physical governance, A1/B2/D1 acoustic contracts and Developer Sound Tools. These changes are ENGINEERING only and do not revise stable physical, parameter/state or product contracts. Commercial/physics material is not used to justify a debugging workaround. Historical evidence remains available.

Human listening, independent acceptance, pluginval/DAW, B2 phases 1–5, D1 C6/C7 and production work are NOT RUN. Hosted CI and draft PR are recorded as live publication evidence on GitHub; a Hosted pass cannot resolve the retained local ASAN failures.

Final staged-file validation: evidence-tool regression 3/3 PASS (0.581 s); repository portability, Markdown internal links, VS Code task references, changed Python AST, changed C++ clang-format and `git diff --cached --check` PASS. Cross-document state consistently retains Phase 0 OPEN and Historical L0. Full preset results above remain tied to cb9ddab; they were not rerun merely to replace failures.

## Phase 0 follow-up measurement and validation handoff

Measurement code HEAD: `9f8a295a5f6914ec77d3ea169fa3cfdd566b93e0`, clean at Release configure/safe-build and both publication checks. Exact R3 baseline remains `1092ba01007d70a66ac5204c7d0d7cb070421972`. Both renderer digests remain the pair recorded above: this follow-up changes offline/test tooling, not renderer DSP. The automatic publisher independently checked both live Git HEADs, clean trees and binary digests; a stale checkout or changed binary rejects. It does not infer a rebuild from a filename. The safe-build record supplies build provenance.

New versioned evidence: [12 reference rows](r31-followup/WATER_A1_R31_REFERENCE.csv), [84 band rows](r31-followup/WATER_A1_R31_BANDS.csv), [57 exact comparisons](r31-followup/WATER_A1_R31_PRESERVATION.csv), and [120 first-round timing rows](r31-followup/WATER_A1_R31_PERFORMANCE.csv). All six sources retain the previous source IDs/rates, seed/config/gain. L0/L1 totals are unchanged (959 firstNonZero each); no new musical benefit or selection. Two live-bound publications into separate ignored directories were byte-identical. Only the explicitly authorized renderer pair was hashed; no audio/log/source hashes.

[All 540 timing rows](r31-followup/PERFORMANCE_RUNS.csv) retain all three fixed-order runs, not a selected best run. [180 median/min/max rows](r31-followup/PERFORMANCE_SUMMARY.csv) cover every variant/rate/capacity/profile. At 96 kHz / 1024 / dense / block128:

| Variant | Mean median us | Mean range us | Worst median us | Worst range us |
| --- | ---: | --- | ---: | --- |
| rebuilt R3 | 452.092 | 427.371–456.404 | 1191.7 | 1009.0–1191.9 |
| L0 | 514.951 | 505.563–523.126 | 1254.9 | 1220.8–1384.1 |
| L1 | 515.008 | 503.408–526.897 | 1271.4 | 1221.7–1503.1 |
| L1 + trace | 527.831 | 490.779–543.463 | 1363.1 | 1289.5–1373.5 |
| L0 count observer | 516.810 | 511.273–529.595 | 1270.0 | 1259.9–1276.4 |
| L0 + trace | 512.255 | 509.266–516.169 | 1312.8 | 1178.8–1488.5 |

L0 mean median exceeds rebuilt R3 by 13.9%. Callback/queue variants overlap the L0 timing range; payload/queue cost alone does not explain that gap. The prior single-run +16.6% and deadline misses are not erased. There is no optimization/adoption claim.

Separate [L0](r31-followup/PERFORMANCE_l0-WORKLOAD.csv) and [L1](r31-followup/PERFORMANCE_l1-WORKLOAD.csv) audits preserve output sums and event counters for all 30 cells per policy. Timing fields are N/A. Current measured sizes: voice512 bytes, observation sidecar2 bytes, pool546136 bytes. Dense96k/1024 visits393215509 voice-samples over3000 callbacks (~131072/callback), first-nonzero branch taken27176 times and completion taken27203 times; requested/callback9.067. The pool checks first-nonzero/completion once per visited voice; the first-nonzero predicate succeeds on about0.0069% of visits. `sidecar_logical_touches` counts the per-visit lookup plus start/reset write, not hardware memory transactions or cache misses. The per-voice observation path is a plausible major contributor; branch prediction, cache/compiler layout and bookkeeping are not separately causally isolated. **Overhead explanation and realtime acceptance remain OPEN.**

[Parent environment A/B](r31-followup/PARENT_ENVIRONMENT_AB.csv) retains A PASS/FAIL and B PASS/PASS. Completed coefficients/input pairs compare byte-equal for24 first-round and11 second-round cases. The interrupted coefficient file is an exact prefix of B's complete file. This strengthens the controlled input comparison, not a root-cause or environment-fix claim. No default environment change was made. Historical source-probe120s timeout and renderer GS failure remain open; per-case evidence now identifies future failures.

First frozen validation attempt at `7fb32e9a6155062a71ecf0d793f85bbe597c0645`: Debug full41/42,205.56s. `flow_d1_cli` exposed a new test-adapter API regression: it imports A1's `run()` with a Path, but the case-capture change assumed RendererCases. This is an explained tooling failure, not a D1 DSP finding. The Release build already starting next was interrupted, ASAN not run, and obsolete Hosted run37109368840 cancelled; none is a PASS. All first-attempt logs remain under `build/r31-followup/final-validation/`.

Correction restores the imported Path-call contract while retaining A1's explicit case adapter. A dedicated compatibility regression and actual Debug D1 CLI both pass (2/2 CTests,35.14s; evidence-tool unittest now7 cases). D1 code/tests are unchanged. Repository search confirmed D1 CLI is the external importer. The code-quality review now explicitly includes this cross-test API dependency.

Final validation protocol after that concrete repair: freeze a new publication commit, then run configure, six-job safe build and full CTest for Debug, Release and ASAN serially, plus Hosted Windows Debug on that same exact HEAD. Do not amend the validated commit or replace an unexplained failure with a retry. The exact HEAD and final results are recorded in the [PR #43 validation table](https://github.com/jjjphens-dot/FRAZIL/pull/43) and its linked Actions run; new logs use `build/r31-followup/final-validation-2/`. This publication-time document does not predeclare those results. It remains PHASE 0 OPEN regardless of subsequent green tests because the retained root-cause and performance findings are unresolved.

Comment/documentation consistency: Testing, Module Index, Project Status and SPIKE README describe the implemented tools and the same open gate. Stable Architecture, Parameters, Coding Plan, physical governance and acoustic contracts remain unchanged. All follow-up changes classify as ENGINEERING. No A1/B2/D1 acoustic code, production, Host/state, defaults, physical equations, lifecycle/RNG/order or timeout was changed. Human listening, independent acceptance, B2 Phase1+, D1 C6/C7 and pluginval/DAW remain NOT RUN. Next authorized work is only Phase0 failure/cost diagnosis.
