# Water DSP / listening readiness — phase execution

Authority: user-supplied next-stage review plan, sections 20 and 115–120, plus the later direct
user instruction to continue automatically after each phase passes self-review, then upload
after all work completes. That instruction supersedes the per-phase pause, while preserving
separate implementation, validation, review and documentation gates and required human decisions.
This record begins with Phase 0, before any DSP experiment implementation.

## Phase 0 — Contract Review and documentation reconciliation

1. **Exact baseline HEAD:** `64f082c84041c9de588b6840ce4e797f23660680`; main
   `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`, Protect PR #37
   `1a1fb410238c274488bdd2ca0709c6831cc15859`. Live fetch found no new upstream changes;
   active research worktree was clean. The separate original checkout's unrelated edits remain untouched.
2. **Scope:** current v0.2 handoff and historical-evidence labelling only. No DSP, mapping curves,
   session codec, GUI controls or production code changes.
3. **Files changed:** new `WATER_LISTENING_HANDOFF_V02.md` and this execution record, historical
   banners in `WATER_LISTENING_HANDOFF.md` / `WATER_LISTENING_UI_EXECUTION.md`, current links and
   session wording in `DEV_UI_WATER_DEBUG_GUIDE.md`, `RESEARCH_MAPPING.md`,
   `DEVELOPER_SOUND_TOOLS.md`, and a continuation link in `WATER_LISTENING_REMEDIATION.md`.
   The two new documents serve distinct needs: current operator handoff and phase audit trail.
4. **Theory/source used:** actual mapper, session codec and unchanged imaginary-state modal
   recurrence; accepted EXP-W-001 definition, testing and developer-tool contracts; previous
   remediation's C0/C1/C2 study. No paper-derived coefficient or external product behavior is
   adopted in this documentation phase. Detailed mechanism-source review belongs before Phase 1.
5. **Implementation behavior:** unchanged. Current statements identify mapping v0.2, session v3,
   24 targets, zero new A/B events at Motion=0, retained Flow/tails and unchanged C0.
6. **Contract impact:** N/A for architecture, public API, nine Host parameters, production schema,
   routing, latency, random persistence, performance budget and ownership. Accepted definition
   does not grant downstream experiment/production acceptance. ADR-0006 remains Proposed.
7. **Tests:** serial configure / safe build / CTest for windows-debug, windows-release and
   windows-asan, plus Markdown links, portability, both scanner regressions, VS Code tasks and
   whitespace. Results are recorded below; clang-format is N/A because no C++ changed.
8. **Results:** all three preset suites and static checks PASS (details below). The prior exact-implementation CI is linked in the
   handoff and is explicitly separate from validation of this documentation update.
9. **Objective audio observations:** re-read the existing v0.2 interim JSON: 72/72 finite,
   repeat/partition/isolation PASS; all four Motion=0 cases A/B=0. No new render or listening run.
10. **Realtime review:** N/A for new callback behavior; no executable code changed. This phase
    makes no new allocation, lock, performance or arbitrary-finite DSP safety claim.
11. **Documentation:** targeted synchronization of current guide, mapping, handoff and evidence;
    no change to documented runtime behavior or milestone/support claims. See reconciliation
    and independent review below.
12. **Known limitations:** C0 remains; Resonant is not fully listening-ready. Pad/90-case pack,
    new normalized C, Motion/Decay listening and human acceptance remain pending. No new native
    GUI, pluginval, DAW, CPU, RMS-matched pack or product validation in this phase.
13. **Stop-condition check:** old unmodified C1/C2 remain rejected by Stop A; no workaround,
    internal saturation or weakened finite-input contract. Phase 0 gate passed; the later user instruction authorizes direct continuation to Phase 1.
14. **Next phase:** EXP-W-RX-001, compare raw R-E0 against bounded-source R-E1 and bounded
    feature-modulated R-E2. Establish silence, finite-extreme, stereo, partition and performance
    evidence before later C3 normalization. No candidate is selected or implemented in Phase 0.

## CODE / DOC DRIFT reconciliation

| Finding | Resolution / authority |
| --- | --- |
| Guide says current export v2 while codec exports v3 | Correct current guide; retain explicit legacy v1/v2 import semantics |
| Historical handoff shows v0.1 Motion minimum 30/s | Preserve measurements and add superseded banner pointing to v0.2 handoff |
| Old execution record describes former gesture workflow and v0.1 | Preserve historical chronology/failures; label current scope as superseded |
| Mapping document's calibration paragraph says session v2 | State current v3 and retained legacy-v2 provenance |
| Current links lead to an unlabelled historical handoff | Direct operator/mapping links to current handoff; retain old evidence links where historical |
| AGENTS baseline uses conditional EXP-W-001 PLANNED wording | Read actual accepted brief: v0.2 Decay Revision B is ACCEPTED for perceptual definition only; conditional pre-acceptance wording does not override it |
| ADR-0006 appears among plan references | Its actual status is Proposed; reference use does not imply adoption or Accepted authority |

## Code Quality Review / Comment & Documentation Pass

Reviewed the final diff independently from successful tests: no historical measurements rewritten,
no new production readiness claim, current session versus mapping identities distinguished,
no personal absolute paths or generated audio added. The handoff distinguishes event silence from
output mute and prior CI/audio/GUI evidence from new validation. New files avoid duplicating DSP
formulas or implementing a second renderer.

Reviewed, no update required: Architecture, Coding Plan, Parameters, Code Standards, Document
Governance, Perceptual Contract and accepted Water brief (no controlled contract changes);
TESTING (no testing-contract change); MODULE_INDEX (no module responsibility/API change);
PROJECT_STATUS (existing historical-pack and unfinished-remediation statements remain true);
experiment README and Protect execution (no algorithm/intake/ownership change). Historical
cross-links remain available. Current handoff, mapping and debug guide agree on v0.2/v3, legacy adoption, Motion=0 and C0 readiness limits; targeted consistency review PASS.

## Final Validation

Completed on 2026-09-19, serially through the ignored local helper:
`cmd /c build\control-bridge\validate.cmd windows-debug`, then `windows-release`, then
`windows-asan`. Each invocation discovers the MSVC/Python environment, configures both research
opt-ins, runs `python tools/build_safe.py --preset <preset>` at 6 jobs, and
`ctest --preset <preset> --output-on-failure`. Every resource safety preflight passed.

| Preset | Configure / safe build | CTest |
| --- | --- | --- |
| windows-debug | PASS | 21/21 PASS, 35.15 s |
| windows-release | PASS | 21/21 PASS, 19.78 s |
| windows-asan | PASS | 21/21 PASS, 61.82 s |

Executed `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_markdown_links.py`, `python tools/test_check_portability.py`,
`python tools/check_vscode_tasks.py`, and `git diff --cached --check`: all PASS. New handoff and
phase record were staged before repository scanners. No C++ changed, so clang-format is N/A.
Build/CTest logs remain under ignored `build/safe-build` and `build/control-bridge`.

Phase 0 is COMPLETE through Contract Review, Implementation, Functional Validation, independent
Code Quality Review, Comment & Documentation Pass and Final Validation. No new audio rendering,
native GUI, listening, pluginval/DAW or CPU evidence is claimed. Phase 1 has not started;
no push or merge was performed for this phase.
