# Water UI control bridge execution

## Goal and acceptance

Implement the user-reviewed staged research UI plan: shared Sound Lead/Engineering session state,
explicit lifecycle/ownership, strict adaptive ms/s input, experiment-only Decay, existing Protect
integration/diagnostics and reproducible session workflow. No production parameter adoption,
algorithm redesign, inferred macro curves or perceptual acceptance.

## Status

Running, Phase 1 preparation. Current delivery slice: Phase 0 and isolated Phase 1, with separate
self-review checkpoints; Phases 2–8 remain pending. Owner: Engineering implementation;
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
20/20 Debug rerun. Phase 0 is ready for its local integration checkpoint; independent review and
human acceptance remain pending.

## Phase tracking

| Phase | Scope | Status |
|---|---|---|
| 0 | Baseline, contracts, integration, documentation inventory | Implemented; self-review and Debug pass |
| 1 | Isolated strict time formatter/parser and tests | Pending |
| 2 | Typed descriptors for existing controls | Pending |
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
