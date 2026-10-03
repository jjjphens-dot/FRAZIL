# R3.1 validation closeout — Phase 0

Status: IN PROGRESS. Engineering only; no sonic candidate or human acceptance.

## Authority and recovery state

- User plan: [B2/D1 convergence](../research/water-b2-d1-convergence/FRAZIL_B2_D1_Convergence_Agent_Plan.md), upstream `be3fdf606f49e5e5139d4847cc1deb9008036042`.
- Research intake: `e0b37e0e06d7b4fc8c2c81e05ce05556fd66ddd4`, clean, origin synchronized (0 ahead / 0 behind); PR #42 draft; Hosted Debug PASS 41/41.
- Work branch: `codex/fix/water-r31-validation-closeout`. The plan branch is based on older code; only its single plan commit was imported, without reverting research implementation.
- Implementation owner: engineering agent. Sound acceptance: Sound Lead; D1 C7 remains a Joint Gate.
- Current step: implementation and focused regressions complete; separate Code Quality Review and Comment & Documentation Pass performed; preparing exact-commit full validation.
- Success criteria: request-local typed outcomes, consistent snapshots, clean exact-R3 rebuild provenance, sample-exact L0, explained native failures, full Debug/Release/ASAN on one implementation commit, measured observation overhead.
- Next checkpoint: focused diagnostics and native reproduction; then independent code/comment review, serial full validation and publication.
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
- Original Windows Application event at 2026-10-03 04:37 local: faulting application `python.exe` 3.12.4150.1013, module `python312.dll`, exception `0xc0000005`, offset `0x36ae7`. It identifies the parent process; it does not establish the corruption source. WER report survives, but its referenced temporary dump is no longer present. Exact original case/backtrace remains unavailable: ROOT CAUSE UNRESOLVED.
- Native case tooling retains command, timestamps, exit code, stdout/stderr and progress files in ignored build directories; source-probe whole-run timeout remains 120 s, native-child timeout remains 240 s. Python faulthandler records parent stacks on future faults. The native process uses Microsoft's documented [ASAN dump hook](https://learn.microsoft.com/en-us/cpp/sanitizers/asan) for future sanitizer-detected faults.

Code Quality Review: no new RNG draws, admission/voice/order changes, heap/lock/I/O on DSP paths or event-state growth. New capacityDropped records use the existing bounded queue and overflow accounting. Pending cancellation bookkeeping precedes the snapshot. Offline logging remains outside allocation/CPU measurement loops. No filter/kernel registry or D1 source tuning changes.

Comment & Documentation Pass: trace v4 outcome semantics, retained native evidence, publisher layout and research-only status documented. Architecture, Parameters, Coding Plan and physical governance reviewed unchanged. Markdown links, portability, VS Code tasks, changed Python AST and diff checks PASS before full validation.

## Documentation impact

Targeted research diagnostics, native test behavior and evidence status. Update this report, affected module/testing/status documentation and R3.1 navigation as implementation settles. Architecture, Parameters, Coding Plan and physical contracts remain unchanged because no corresponding contract changes.
