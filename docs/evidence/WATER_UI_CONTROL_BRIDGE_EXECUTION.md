# Water UI control bridge execution

## Goal and acceptance

Implement the user-reviewed staged research UI plan: shared Sound Lead/Engineering session state,
explicit lifecycle/ownership, strict adaptive ms/s input, experiment-only Decay, existing Protect
integration/diagnostics and reproducible session workflow. No production parameter adoption,
algorithm redesign, inferred macro curves or perceptual acceptance.

## Status

Phases 0–8 implemented and self-reviewed locally, ready for GitHub review. The user explicitly
requested autonomous progression after each self-review, then a single GitHub publication.
Owner: Engineering implementation; Sound Lead retains human workflow/perceptual acceptance.
Independent review and acceptance remain pending. No delegated workers.

## Live baseline

- Main: `fc20370ddcf7cce97b950522b9aeaf9d605e945d`.
- Preview: `234056e82fe52c67ab288d4be7f99c0535548c19`.
- Protect: `a883097b4ddcd6fe5ede3bd0fcf49ac88bfa4063`.
- PR #37: open, mergeable, no formal review decision; Windows Debug CI successful,
  [run 35314572729](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35314572729).
- Integration branch: `codex/feat/water-ui-control-bridge`, based on live main, then Preview and
  Protect. Source branches are retained, without history rewriting or main merge.

These identities were fetched for this task; re-fetch before later integration or publishing.

## Phase 0 inventory / next action

Four content conflicts: `.github/workflows/ci.yml`, `docs/DEVELOPER_SOUND_TOOLS.md`,
`docs/MODULE_INDEX.md`, `experiments/water/SPIKE-W-DSP-001/render/ReadConfig.h`.
Preserve Protect's explicit Python interpreter/requirements, listening regression and evidence;
retain Preview's opt-in build and shared in-memory strict config parser.

Auto-merged hotspots requiring review: research CMake/README, Coding Plan, Environment,
Project Status and Testing. Protect DSP, render and listening sources must remain unchanged.

All four conflicts resolved. CI retains Protect's pip/CMake/CTest interpreter identity and enables
both research targets. Both documentation sections are retained. The shared parser accepts an
optional Protect destination; the existing Preview call rejects Protect fields until Phase 5 wires
that module. Renderer reference-overload compatibility is retained. Added regression coverage for
in-memory Protect retention, rejection without a destination and duplicate fields.

Phase 0 self-review: no changes to Protect DSP, renderer main or listening implementation against
the Protect source ref; no production target dependency on research. CMake test registration and
ASAN support retain both suites. No new macro mapping or audio behavior is introduced.

Combined baseline: Windows Debug safe build and 20/20 CTests passed. Markdown links, portability
and staged/unstaged whitespace checks passed. The added parser regressions also passed in the
20/20 Debug rerun. Phase 0 is committed as `b173418`; independent review and human acceptance remain
pending. Exact integrated file inventory relative to Preview can be reproduced with
`git diff --name-only 234056e b173418`. The four conflict-resolution paths above, the Preview parser
regressions and this execution record are the integration-specific edits; other changes are retained
Protect work, not newly authored algorithms.

## Phase tracking

| Phase | Scope | Status |
|---|---|---|
| 0 | Baseline, contracts, integration, documentation inventory | Implemented; self-review and Debug pass |
| 1 | Isolated strict time formatter/parser and tests | Implemented; self-review and three presets pass |
| 2 | Typed descriptors for existing controls | Implemented; self-review and Debug pass |
| 3 | Shared ResearchSessionModel and dual views | Implemented; self-review, final Debug and docs pass |
| 4 | Experiment-only Decay state/workflow | Implemented; self-review, Debug and docs pass |
| 5 | Existing Protect DSP integration and calibration memory | Implemented; self-review, three presets and docs pass |
| 6 | Bounded Protect numerical diagnostics | Implemented; self-review and Debug pass |
| 7 | Module/session imports, exports, A/B and reset | Implemented at `045826e`; self-review, Debug and docs pass |
| 8 | GUI usability and human handoff | Implemented; self-review, three presets and Windows GUI checks pass within limits below; human acceptance pending |

## Constraints and unresolved items

DSP/config/renderer retain seconds; unitless exact input means ms. Unsupported suffixes, partial
parse, nonfinite and out-of-range values must fail atomically without clamping. Model maps only
Fluid↔ABD and Resonant↔C; Size/Motion/Decay remain unmapped pending evidence. Flow has no Decay
destination. Protect is residual-only research, depth zero is OFF, D0/D1 calibration domains differ,
and F2/F3 do not promise summed-energy contraction. Nine Host parameters and state schema stay intact.

Windows automated GUI checks are recorded below. They do not establish formal human workflow,
perceptual, Host or production acceptance.

## Phase 1 implementation and review

- Baseline: Phase 0 integration commit `b173418`; source refs above are retained as ancestors.
- New `preview/TimeValue.h`: narrow stateless C++ tooling functions; no framework or dependency.
  Formatting selects units using the unrounded seconds value. Parsing consumes the entire input,
  validates finite values and inclusive seconds bounds, then commits only a successful value.
- New `tests/preview_time_tests.cpp`: runs in the existing Preview executable; CMake and its main
  add that test entry. Valid/invalid examples, boundary conversion, suffixes, whitespace,
  scientific notation, repeated signs, overflow/underflow and failure atomicity are covered.
- Behavior: isolated helpers only; no widgets, draft/apply, serialization or DSP behavior changes.
- Code Quality Review: explicit seconds units, no mutable global state, no hidden ownership,
  locale-independent standard conversions, no exception-driven parsing or silent clamping.
  Formatting allocates a string and is explicitly UI/tooling-only; the audio call graph is unchanged.
- Comment & Documentation Pass: public helper semantics and error contract documented; Module
  Index, Testing, Project Status and research README synchronized with this checkpoint.
- Reviewed, no update required: Architecture, Parameters, Perceptual Contract, ADRs 0002/0003/0006,
  Coding Plan, Developer Sound Tools, Preview guide, `src/ui/README.md`, Code Standards and
  Document Governance. This slice introduces no Host/state, production ownership, perceptual
  mapping, public GUI behavior, realtime, latency, random or formal performance contract change.
- Phase 0 preserved historical validation records; this record identifies new combined evidence
  without replacing earlier failures or treating PR #37 CI as validation of this branch.
- Human/UI evidence: not performed for this slice; no interactive behavior was changed.
  Pluginval/DAW, audio-device audition and subjective listening were not run. Three-preset
  automated integration checks are recorded below when complete.
- Next phase: typed descriptors for the existing 21 controls, preserving applied DSP config.
  Protect/Decay/session/UI work remains explicitly pending and needs its own later checkpoints.

## Phase 1 commands and three-preset validation

Run from the integration repository root in the discovered MSVC developer environment. The ignored
local command wrapper only sequences the existing tools and redirects full logs beneath
`build/control-bridge/`; build safety remains enforced by `tools/build_safe.py` at six jobs.
Each preset is fully finished before the next starts. For each of `windows-debug`,
`windows-release` and `windows-asan`:

```powershell
$bridgePython = python -c "import sys; print(sys.executable)"
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON "-DPython3_EXECUTABLE:FILEPATH=$bridgePython"
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
```

`<preset>` is a documentation placeholder, not a literal shell argument. Python dependency imports
(`numpy`, `scipy`, `matplotlib`) passed. Debug CMakeCache and CTest JSON commands were checked against
the discovered interpreter after normalizing path separators; interpreter identity passed.

| Check | Result |
|---|---|
| Windows Debug configure / safe build / CTest | PASS; 20/20 |
| Windows Release configure / safe build / CTest | PASS; 20/20 |
| Windows ASAN configure / safe build / CTest | PASS; 20/20 |
| `python tools/check_markdown_links.py` | PASS |
| `python tools/check_portability.py` | PASS |
| `git diff --check` | PASS |
| Protect DSP diff against source Protect ref | Empty |
| Production source code diff against main | Empty; inherited `src/ui/README.md` docs only |

The six development stages are recorded separately: Contract Review (Phase 0), Implementation
(parser compatibility and isolated time tooling), Functional Validation (Debug), Code Quality Review
(above), Comment & Documentation Pass (above), and Final Validation (this table). New Hosted CI,
independent approval and main merge are not part of this local checkpoint.

## Phase 2 implementation and review

1. Baseline: Phase 1 `b8f085d`; integration branch remains `codex/feat/water-ui-control-bridge`.
2. Scope: migrate exactly the original 21 controls to typed metadata, without changing applied config.
3. Files: new `preview/ControlDescriptor.h` and `tests/preview_descriptor_tests.cpp`; updated
   `preview/PreviewSettings.h`, `tests/preview_tests.cpp`, research CMake/README, Module Index,
   Testing and this record.
4. Behavior: metadata only; original module/key/label/range/step/default and export order preserved.
5. Contracts: DSP structs and renderer schema unchanged; every existing control remains APPLY.
6. Tests: Debug configure, safe build and CTest; descriptor regressions run inside Preview tests.
7. Results: Debug 20/20, markdown links, portability and diff checks PASS. The Phase 1 three-preset
   table is not a Phase 2 Release/ASAN claim; those are repeated at the final integrated checkpoint.
8. Human/UI evidence: no interactive behavior changed; no GUI/DAW/listening run for this phase.
9. Realtime: descriptors remain outside processing; standard-library metadata only, no DSP dependency.
10. Documentation: README/Module Index/Testing synchronized. Architecture, Parameters, ADRs, Developer
    Sound Tools, Preview guide and production UI README reviewed, no update required for metadata.
11. Self-review: compile-time ID ordering, runtime unique field/ID coverage, typed defaults, preserved
    ranges, unit/group/lifecycle checks. No generic reflection framework or changed DSP authority.
12. Next: shared ResearchSessionModel and two views; no guessed macro mappings.

## Phase 3 implementation and review

1. Baseline: Phase 2 `033cf73`, same integration branch.
2. Scope: shared state/commands, two macro representations, engineering view, inactive controls,
   draft/applied, origin/revision and Model-only mapping infrastructure.
3. Files: new `preview/ResearchSessionModel.h`, `preview/ResearchViews.h`, `preview/PreviewPanel.h`,
   `tests/preview_session_tests.cpp`; `PreviewMain.cpp` now only owns application/window setup;
   CMake/test entry and directly affected docs updated.
4. Behavior: Sound Lead and Engineering read one model; original flat UI is replaced by two tabs.
   Source/Full/Water-only monitoring keeps existing audio semantics. Engineering edits stop playback;
   inactive values remain editable/retained. A/B now includes applied macro state as well.
5. Contracts: Model maps only Fluid/ABD and Resonant/C; ablations mark custom composition. The mapper
   reports Size/Motion UNMAPPED. No arbitrary curve or reverse mapping, no Host/production DSP change.
6. Tests: existing Debug configure/safe build/CTest including new session regressions.
7. Results: functional and final post-review Debug 20/20 PASS; markdown links, portability and diff
   checks PASS. Later integrated validation repeats Release/ASAN.
8. Human/UI: no new GUI or listening acceptance yet; actual Windows interaction is Phase 8.
9. Realtime: model/views/commands stay on message thread, validation/preparation uses the existing
   detached-callback controller; audio processing is unchanged.
10. Documentation: Developer Sound Tools, Debug Guide, Module Index, Project Status, Testing, research
    README and execution record updated. Architecture/Proposed ADR-0006 reviewed: this isolated
    research application ownership introduces no production mapping or dependency decision, so no
    ADR change required. Parameters, Coding Plan, Code Standards and production UI README remain valid.
11. Self-review: no widget-to-widget propagation; refresh uses notification-free assignments. A/B
    restore publishes once, equal-value edits do not increment revisions, invalid commands preserve
    state. Per-target edit provenance, retained inactive values and macro non-mutation are tested.
    The coordinator owns command lifecycle, the session owns values, views own presentation.
12. Next: add provisional Decay experiment value with complete state/workflow coverage; then Protect.

## Phase 4 implementation and review

1. Baseline: Phase 3 `2182672`, same integration branch.
2. Scope: provisional normalized Decay in both views, shared state, applied A/B/reset and a separate
   session file/clipboard workflow. Session infrastructure is introduced here because Decay must
   survive export/import; Phase 7 adds module import and complete source/build/Protect provenance.
3. Files: model/views/panel/window and existing test entry/CMake updated; new `SessionCodec.h`,
   `SessionJsonSyntax.h`, `tests/preview_session_codec_tests.cpp`; directly affected docs synchronized.
4. Behavior: Decay `0.5` provisional baseline is visible, editable and saved with Model/Size/Motion.
   Copy/Export Session saves applied values; strict Import Session prepares a candidate and validates
   with the existing controller before replacing state. Invalid imports retain the current session.
5. Contracts: no Decay DSP mapping, no Flow decay destination, no Host/APVTS/schema change. The old
   plugin `frazil.dev-experiment` v1 and DeveloperWaterExperimentSnapshot remain unchanged. New
   `frazil.water-research-session` v1 is a separate unreleased research format, not renderer config.
6. Tests: Debug safe build/20 CTests with baseline, Decay A/B/reset, roundtrip, provenance and atomic
   failure tests; malformed numeric tokens, duplicate/escaped duplicate keys, unknown fields and
   wrong version/seed are rejected.
7. Results: Debug 20/20 PASS; final markdown links, portability and diff checks PASS.
8. Human/UI evidence: compilation/model/codec tests only; Windows interaction remains Phase 8.
9. Realtime: JSON/file/string work stays on message thread; no callback or Protect DSP change.
10. Documentation: guide, Developer Sound Tools, Module Index, Project Status, Testing, research
    README and this record updated. Decay Revision C consumer inventory: only the new standalone
    session consumes the new four-macro format; original plugin consumers do not change. Architecture,
    Parameters, Coding Plan, ADRs and production UI README need no contract change for this scope.
11. Self-review: bounded 1 MiB/eight-level syntax gate is necessary because module config grammar
    cannot validate nested/string session metadata. Full consumption, decoded duplicate detection,
    enum/range checks and candidate-only assignment precede controller validation. No generic preset
    framework, ownership duplication, reverse mapping or production default claim was introduced.
12. Next: consume the latest Protect source, add residual-only controls and D0/D1 calibration memory.

## Phase 5 implementation and review

1. Baseline: Phase 4 `fbdd139`; a fresh fetch confirmed main `fc20370` and Protect `a883097` unchanged.
   PR #37 remains open without formal review decision; no source-branch/main merge was performed.
2. Scope: existing Protect processor integration, main/advanced controls, live Depth/Enable,
   per-detector calibration and retained Fluid topology. No detector/envelope algorithm rewrite.
3. Files: `ProtectControls.h`, `ProtectView.h`, `ExactValueControl.h`, `preview_protect_tests.cpp` added;
   settings/session/codec/controller/engine/panel/window/CMake/test entry updated, plus related docs.
4. Behavior: only residual is attenuated. Depth target is live; other fields stop playback and require
   Apply. C rejects F2/F3; Fluid remembers topology. D0/D1 restore their own threshold domain. Initial
   enable recall is convenience `0.5`, while the actual baseline remains Depth zero. Session/A/B now
   retain full Protect settings and calibration; renderer exports use its existing Protect schema.
5. Contracts: all Protect DSP sources are byte-for-byte unchanged by Git diff against `a883097` (no
   content hashes computed). Nine Host parameters, schema 1, source path and generator/RNG progression
   are unchanged. Size/Motion/Decay remain UNMAPPED. F2/F3 contraction is not claimed.
6. Tests: serial Debug/Release safe builds + CTest; added exact-reference integration comparisons at
   44.1/48/96 kHz for both detectors, three Fluid topologies and C/Whole. Tests cover OFF recovery,
   reset, finite/stereo behavior, live state, enable recall, retained calibration/topology and A/B/session.
7. Results: Debug/Release/ASAN each 20/20 PASS. Existing Protect/listening tests remain. The portability
   scan initially mistook escaped JSON negative fixtures for UNC paths after those files became
   tracked; equivalent raw-string fixtures fixed the false positive without relaxing the scanner.
   Staged new files are now included in the final portability/link/diff checks, all PASS; a Debug
   rerun verifies the equivalent fixtures. Earlier Phase 4 scans covered tracked files at that time.
8. Human/UI: no new physical-output or human listening result; Phase 8 remains the GUI evidence gate.
9. Realtime review: the new callback work is existing prepared Protect processing plus one atomic
   double load per callback and sample-boundary retarget. Lock-free static assertion, no new callback
   allocation/string/JSON/I/O/lock. Processing consumes typed config; preparation stays detached.
10. Documentation: guide, Developer Sound Tools, Module Index, Project Status, Testing and research
    README updated. Architecture/Parameters/ADRs/production UI README/Coding Plan reviewed: isolated
    research integration changes no production contract or adoption decision.
11. Self-review: strict exact-entry widget validates before slider clamping; session imports check both
    calibration domains and config/memory consistency. Apply rejects invalid retained calibration.
    Source DSP validation stays authoritative. Module JSON uses 17 decimal places to keep session
    calibration/config roundtrips consistent. Current large layout and original 21-control input
    migration remain Phase 8; module-specific Apply feedback and full provenance remain Phase 7.
12. Next: bounded numerical Fast/Slow/D0/D1/GR diagnostics; optional trace only if realtime-safe.

## Phase 6 implementation and review

1. Baseline: Phase 5 `3f9ae54`, same integration branch.
2. Scope: actual Fast/Slow/D0/D1/GR values and consumed-block peaks, bounded transport and loss count.
3. Files: new `ProtectDiagnostics.h`, `tests/preview_diagnostics_tests.cpp`; engine/controller/Protect
   view/panel/window/CMake/test entry and related documentation updated.
4. Behavior: 10 Hz UI shows latest sample and interval peaks, plus block/drop counts. Stop clears
   after callback detach. Peaks are explicitly not co-timed; no output attenuation equivalence claim.
5. Contracts: no Protect DSP source change, no mappings/Host/state/production dependency change.
6. Tests: Debug safe build/CTest; real producer/reader, full/empty/reset/drop/coherence and bounded
   work tests; integrated readouts compared to original detector/envelope at every tested sample.
7. Results: Debug 20/20, markdown links, staged-file portability and diff checks PASS.
   Final integrated Release/ASAN still follow.
8. Human/UI: graphical interaction/readability remains Phase 8; numerical model evidence only here.
9. Realtime: 256 preallocated summaries, one producer/one consumer, release/acquire ownership transfer,
   lock-free integer atomics, no waits/retry growth/formatting/I/O. Callback computes five maxima per
   sample and copies one summary per block; UI drains at most capacity. Drop rather than overwrite.
10. Documentation: guide, Developer Sound Tools, Module Index, Testing, research README and this record
    describe values/units/window/loss. Architecture, Parameters, Accepted ADRs, Coding Plan and
    production UI README reviewed, no production contract change. No new formal performance claim.
11. Self-review: non-atomic payload slots are exclusively transferred, consumer publishes tail only
    after reads, producer never touches unread slots. `clear` requires detached producer and sole
    UI consumer. Tests exercise actual concurrent observation; ASAN is not claimed as race detection.
    Optional rolling trace is omitted; numerical diagnostics meet this phase's required scope.
12. Next: complete module/session workflow, source/build provenance, atomic imports and Apply feedback.

## Phase 7 implementation and review

1. Baseline: Phase 6 `f24bd08`, same isolated integration branch.
2. Scope: renderer module import, portable source/build context, complete session/A/B/reset workflow,
   module-specific validation feedback and transaction regression coverage.
3. Files: new build-info template, SessionMetadata and workflow tests; preview CMake/settings/model/
   codec/controller/panel updated. No production source or Protect algorithm edit.
4. Behavior: imports parse into a candidate, use authoritative prepare validation and commit only
   after success. Omitted module fields use renderer defaults; macros/source/monitor are retained.
   Session/A/B preserve source filename/rate/channels/frames; mismatch disables Play until explicit
   source selection. Reset retains source. Current/imported configure-time builds remain distinguishable.
5. Contracts: renderer schema unchanged; new unreleased research manifest v1 expanded. No Host schema,
   parameter, accepted mapping, content hash, personal path or production dependency introduced.
6. Tests: `cmd /c build\control-bridge\validate.cmd windows-debug`; workflow tests cover roundtrips,
   defaults, coupled constraints, atomic rejection and live-edit provenance.
7. Results: Debug safe build and 20/20 CTest PASS; staged links/portability/diff checks PASS.
8. Human/UI: functional model/codec evidence; actual Windows workflow remains Phase 8.
9. Realtime: all new parsing, source metadata, error probes and build strings remain on message thread;
   callback unchanged. Validation probes allocate only while preparing failed candidate diagnostics.
10. Documentation: debug guide, Developer Sound Tools, Module Index, Project Status, Testing, research
    README and execution record synchronized. Architecture, Parameters, Coding Plan, Accepted ADRs,
    Environment and production UI README reviewed; no contract change required.
11. Self-review: separate source descriptors are not content identity; imported audio is not embedded
    or automatically loaded. Invalid imports preserve both applied and draft values. Build provenance
    is configure-time, clearly documented; module fields never receive macro values or new mappings.
12. Next: strict inputs for all controls, collapsible modules, responsive layout and Windows GUI checks.

## Phase 8 implementation and review

1. Baseline: Phase 7 `045826e83929a2cfaa4e6b6f746debccf573d393`, plus this phase's working changes.
   A final fetch reconfirmed main `fc20370`, Preview `234056e` and Protect `a883097`.
2. Scope: all 21 engineering controls use strict exact entry; larger macro controls, collapsible
   A/B/D/C cards, Advanced Protect, responsive scroll layout, draft detail and optional audio diagnostics.
3. Files: ExactValueControl, ResearchViews, ProtectView, PreviewPanel, PreviewMain and workflow tests;
   new `DraftSummary.h` formats applied-to-draft differences without adding state ownership.
4. Behavior: exact entry retains precision and rejects invalid input before model mutation; Escape
   restores the applied widget value. Shift-drag provides fine adjustment. Explicit reset/import/A/B
   discards pending invalid text even when the recalled numeric value is identical. Inactive modules
   retain their values; monitor labels distinguish Source, Full and Water-only residual.
5. Contracts: production `src/plugin`, `src/app` and `src/dsp` have no diff against main. Protect DSP,
   renderer main and analysis have no diff against `a883097`. No macro mapping or Host/schema change.
6. Functional Validation and Final Validation: serial
   `cmd /c build\control-bridge\validate.cmd windows-debug`, `windows-release` and `windows-asan`.
   The ignored helper runs the MSVC environment setup, explicit Python configure binding,
   `python tools/build_safe.py --preset <preset>` and `ctest --preset <preset> --output-on-failure`.
   Both research options are ON. CMake/Python/CTest interpreter identity was checked.
7. Results: safe builds PASS; Debug 20/20 (37.95 s), Release 20/20 (22.57 s), ASAN 20/20 (67.90 s).
   Final `python tools/check_markdown_links.py`, `python tools/check_portability.py` (including staged
   new files) and `git diff --cached --check` PASS. No generated evidence or personal paths staged.
   Widget callback tests exercise strict time/integer entry, invalid suffix/NaN/range rejection,
   Escape/external restore, same-value pending-text discard and synthesized Shift-drag fine movement.
   Draft delta formatting is covered. These tests complement the earlier session/DSP/transport suites.
8. Windows GUI evidence: native automation exercised default, minimum and maximized windows at the
   reference machine's 150% DPI profile; scrolling, Advanced folding, inactive module readability,
   invalid time feedback/Escape, draft/apply, A/B recall, Copy Session and live Protect diagnostics.
   Exact-entry width increased to 190 px and font 13 after inspection; source-string separators use
   ASCII to avoid the observed MSVC encoding artifact. Source/Full/Water-only labels are readable.
   Task-only screenshots are in ignored `build/control-bridge/gui/`, including
   `minimum-advanced.png`, `large-advanced.png`, `minimum-engineering.png`, `large-engineering.png`
   and `48000-protect-diagnostics.png`. They are local evidence, not attached human acceptance.
9. Audio/realtime evidence: native playback at 48 kHz, stereo, 480-frame blocks reported finite
   nonzero input/output and actual Protect values with zero observed diagnostic drops. Stop clears
   diagnostics. 44.1/96 kHz sources loaded but the default device rejected those rates with the
   documented no-resampling error; automated DSP/reference tests cover all three rates. GUI fixtures
   reuse canonical impulse, gated sine and transient-pitch generators with silence; no subjective
   quality or formal callback performance conclusion is inferred.
10. Comment & Documentation Pass: debug guide, Developer Sound Tools, Testing, Module Index,
    Project Status, research README and this record synchronized. Architecture, Parameters, Coding
    Plan, Accepted ADRs, Environment and production UI README reviewed without further changes:
    this phase changes research workflow only. ADR 0006 stays Proposed. Units, lifecycle, source/build
    provenance and bounded diagnostic ownership are documented; personal paths remain ignored.
11. Code Quality Review: one message-thread model owns state; views issue typed commands. Strict
    parsing precedes mutation, draft formatting is stateless, and no algorithm logic moved into UI.
    Callback review confirms prepared DSP, one live target load and bounded preallocated SPSC
    diagnostics; no UI/JSON/file/lock/allocation added there. Ownership, naming, includes, dead code,
    constants and dependency direction reviewed. No independent approval is claimed.
12. Known limits / handoff: other DPI factors, multi-monitor transitions, pluginval/DAW and formal
    human listening acceptance were not executed. Production is untouched. Sound Lead receives the
    guide/session workflow for acceptance; independent GitHub review remains required. All requested
    implementation phases are complete; publication follows final documentation checks.

### Corrections retained in the evidence trail

- The first draft-summary test selected Bubble decay's existing 70 ms baseline, so its expected
  delta was invalid. The fixture now selects 81 ms; the final three presets pass.
- A Debug link attempt failed with LNK1168 while the user was interacting with the running preview.
  The user's revision 563 session was saved in ignored `gui/manual-session-before-rebuild.json`
  before closing the app; the normal safe build then passed. No build guard was bypassed.
  Native Import Session restored that saved session in the final Debug binary with the matching
  48 kHz fixture; the UI reported APPLIED, zero pending changes and Session Load provenance.
- GUI checks exposed narrow exact-entry text and a garbled separator; both were corrected and
  visually rechecked. Final pending-text discard also has direct widget regression coverage.
- Contract Review preceded implementation; Functional Validation, separate Code Quality Review,
  Comment & Documentation Pass and Final Validation are recorded above, completing the required
  engineering sequence without promoting research results to production or perceptual acceptance.
