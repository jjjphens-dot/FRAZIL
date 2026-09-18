# Water UI control bridge execution

## Goal and acceptance

Implement the user-reviewed staged research UI plan: shared Sound Lead/Engineering session state,
explicit lifecycle/ownership, strict adaptive ms/s input, experiment-only Decay, existing Protect
integration/diagnostics and reproducible session workflow. No production parameter adoption,
algorithm redesign, inferred macro curves or perceptual acceptance.

## Status

Running, Phase 3 preparation. The user explicitly requested autonomous progression through
all phases after each self-review, then a single GitHub publication of the completed work. Separate
local commits/checkpoints remain required; Phases 2–8 are next. Owner: Engineering implementation;
Sound Lead retains human workflow/perceptual acceptance. No delegated workers.

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
| 3 | Shared ResearchSessionModel and dual views | Pending |
| 4 | Experiment-only Decay state/workflow | Pending |
| 5 | Existing Protect DSP integration and calibration memory | Pending |
| 6 | Bounded Protect numerical diagnostics | Pending |
| 7 | Module/session imports, exports, A/B and reset | Pending |
| 8 | GUI usability and human handoff | Pending |

## Constraints and unresolved items

DSP/config/renderer retain seconds; unitless exact input means ms. Unsupported suffixes, partial
parse, nonfinite and out-of-range values must fail atomically without clamping. Model maps only
Fluid↔ABD and Resonant↔C; Size/Motion/Decay remain unmapped pending evidence. Flow has no Decay
destination. Protect is residual-only research, depth zero is OFF, D0/D1 calibration domains differ,
and F2/F3 do not promise summed-energy contraction. Nine Host parameters and state schema stay intact.

No human/UI acceptance has been performed for this integration.

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

## Commands and final validation

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
