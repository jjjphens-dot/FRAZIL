# Water Protect execution — PROTECT-EXP-001

Submission: [Issue #38](https://github.com/jjjphens-dot/FRAZIL/issues/38), combined
[PR #37](https://github.com/jjjphens-dot/FRAZIL/pull/37); follows historical documentation issue #36.

## Authority and scope

The user's follow-up on 2026-09-17 explicitly authorizes sequential local waves after agent self-review,
followed by one overall review and upload. This supersedes the Wave 1 proposal's per-wave external-review/
merge wait for this task. It does not fabricate accepted EXP-W-001/Decay Revision B, Developer readiness,
human listening, independent GitHub approval or production adoption. Those statuses remain pending.

Engineering Lead executes objective Protect research and prepares listening handoff in the existing opt-in
Water research tree; no formal EXP-W-002 completion or closed SPIKE scope expansion is claimed. This is a
separately identified user-authorized follow-up, not retrospective authorization of the old spike. No push,
PR-body update or new GitHub issue until the final combined submission. Wave 1 was already uploaded as #37.

Allowed: experiment-only detector/gain/application, existing renderer/config/analyzer/performance integration,
research tests/CMake and directly affected documentation. Forbidden: production src, Host/APVTS/schema,
plugin/UI, Routing/Ice, vendor edits, perceptual scores invented by an agent, production adoption or merge.

## Goal and success criteria

Complete available numerical engineering work through Waves 2–5, create Wave 6 reproducible comparison packs
and an honest Wave 7 disposition. Each wave receives functional validation, a separate code-quality review,
comment/documentation pass and local checkpoint. Preserve generator/state/RNG and exact static OFF, gain
bounds, causal zero-lookahead processing, and bounded dynamic OFF completion. Validate serial Debug/Release/
ASAN pipelines using build_safe. Upload once after final review of the combined change.

## Status

PR #37 engineering remediation and self-review complete. Historical runs below remain source-specific evidence;
Wave 6 human listening is NOT RUN and Wave 7 product decision is BLOCKED pending detector selection and listening.

## Completed

- Wave 1: `08df5f4`, theory and proposal uploaded as PR #37; live base `fc20370`.
- Recovery: clean worktree; original worktree unrelated changes retained; Issue #17 still awaiting acceptance.
- Wave 1 self-review: theory contraction/cancellation bounds and gate labels reviewed; no algorithm adoption.

## Owners and blockers

Engineering agent owns implementation/self-review. Sound & Host owns musical material and human judgments.
No representative audio exists in main's listening directory. The user confirmed that audio and conclusions
will be provided later, and directed the agent to proceed with the other work. Wave 6 human listening and
Wave 7 product adoption therefore remain awaiting that input; numerical experiments and pack preparation
continue. No independent approval will be claimed from self-review.

## Next action / checkpoint

Upload the reviewed remediation to the existing PR #37 branch. Resume musical listening and explicit detector
selection when the user supplies material and judgments; independent PR approval/merge remain unclaimed.

## Wave 2 review notes

Implementation reuses `WaterExcitationFeatures` as a member of `ProtectDetector`; the generator's instances
are untouched. D0 uses amplitude difference; D1 uses positive log ratio gated by slow amplitude. Separate
floor/epsilon validation and failed-prepare inert behavior are implemented. Tests cover three rates,
silence/weak input, reset, stereo-linked polarity, input saturation, invalid config and sub-cap level scaling.
Review found no global state, process allocation/lock/I/O, hidden dependency or changed generator lifecycle.
D1's slow-floor onset miss and D0's level dependence remain known behavior, not reasons to choose a winner.
Functional validation: `cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON`,
`python tools/build_safe.py --preset windows-debug`, `ctest --preset windows-debug --output-on-failure`:
PASS, 17/17 (29.72 s). `clang-format` and diff checks passed for the detector. Wave 2 self-review: engineering
ACCEPT for comparison infrastructure; retain both candidates without perceptual ranking. Comment/documentation
pass records units, independent ownership and floor/cap limitations. Next Wave 3 authorized by user sequence.

## Wave 3 review notes

Implemented pure score-to-dB mapping and independent detector/gain state with finite 10 ms default OFF ramp.
Repeated OFF does not restart it; reactivation retargets from current gain. Static unity bypasses residual
multiplication. No generator is owned or reset by Protect. Tests cover exact full-stream generator continuation,
Fluid/C contraction, OFF completion, invalid/rapid depth targets and partition invariance at three rates.
Debug build and new tests PASS. The existing renderer CLI's Python process once exited `0xc0000409`; Windows
Application events identified `python312.dll` (Python 3.12.4), with no test assertion. An unchanged focused
rerun passed in 17.47 s. Root cause is not established; final preset runs must cover this test again.
Self-review: ACCEPT for objective whole-residual experiments; no click-audibility or perceptual benefit claim.
Units, failure behavior, state ownership and OFF policy documented in code. Wave 4 may proceed.

## Wave 4 review notes

Added pure F1/F2/F3 residual composition and a numeric `protect` object in the existing strict offline config
reader; default depth is zero. Renderer optionally emits source-detector/GR CSV, entirely outside DSP/timing.
Invalid enums/ranges/types fail before output; trace paths refuse collisions. Existing renderer regression
plus decoded OFF/contraction/partition/trace tests PASS: safe Debug build, 18/18 CTest in 23.55 s.
Cancellation counterexample tested; all branches retain processing/state. Self-review: ACCEPT for comparison
infrastructure, no topology winner. Comment/documentation pass distinguishes partial-branch bounds from F1.

## Wave 5 review notes

Safe Release configure/build/CTest PASS, 18/18 in 17.43 s. Existing renderer reused for 84 diagnostic renders,
18 unique static interaction renders and 22 one-factor sweep renders: all PASS finite/output/config/GR checks.
Post-render analysis verifies same-generator event counts and F1 residual energy contraction with explicit
float reconstruction tolerance. Eight plots generated; impulse plot aggregation corrected during self-review
to preserve single-sample peaks instead of dropping them by stride. Reports were reanalyzed without rerendering.

Commands: `python experiments/water/SPIKE-W-DSP-001/analysis/protect_review.py --renderer <Release renderer>
--suite <diagnostics|interaction|sweep> --output <new ignored directory>` and the existing Release
`frazil_water_performance.exe`. Local artifacts: `build/protect-diagnostics`, `build/protect-interaction`,
`build/protect-sweep`, `build/protect-performance.csv`; generated files are not committed.

Motion/Decay interaction is explicitly **static engineering proxies**, not accepted macro mapping or live
Decay automation: Fluid uses 2 activity x 2 persistence x 3 Protect levels (12); C uses 2 persistence x 3
Protect levels (6). C has no Motion destination; the other six requested conceptual cells are N/A rather
than duplicated. Fluid activity alters event-rate/refractory/Flow-interval only; persistence alters A/B/C
decay only. Voice steals/active counts unsupported by existing interfaces are N/A. Product separability and
live/event-latched Decay remain outside this bounded follow-up.

Timing: Windows/MSVC Release, Intel Core i9-14900HX, 48 kHz/128 stereo, 2000 warmup + 20000 measured blocks,
same gated workload. Full raw rows retained locally. Fluid reference mean 8.8983 us, C 2.92287 us. Active
Protect increments across D0/D1 and P=.5/1 were Fluid 7.78109..8.33939 us, C 7.32370..8.51186 us. OFF still
advances the follower (about 2.25..3.08 us increment). Both scores are computed for comparison infrastructure;
these are not optimized detector-only costs, process CPU percent, formal budget/provenance or a benchmark
stability claim. Maximum callbacks include scheduler noise; no real-time deadline guarantee is inferred.

Self-review: ACCEPT numerical infrastructure; retain candidates, no perceptual winner. D1 level/floor/cap
limits and component cancellation remain. Denominator floors, full-file appended-tail duration, source-only
GR and annotated onset windows are recorded separately; no favorable window selection or hidden normalization.
Comment/documentation pass completed; next is a reproducible listening handoff, with human input pending.

Concrete detector finding: on the canonical high-then-low gated sine at P=1, default D1's 50 ms onset-window
maximum GR is 9.000 dB for the first onset but about 0.003 dB for the second. D0 gives 9.000 / 0.207 dB.
The slow envelope retains the louder preceding event; approximate scale invariance does not imply recognition
of a weaker event in a different history. This is a missed/weak numerical response, not a human importance or
audibility judgment. Do not select D1 merely for isolated-level invariance. Any detector revision needs a
bounded follow-up comparing this case with bass ripple/noise false positives; no subjective tuning occurred.
For reference, Fluid input-duration duty above 1 dB is 11.62% (D1) versus 28.43% (D0) on this fixture; full-file
duty also includes the appended 3 s silence and is separately labelled. In the static interaction matrix,
changing decay preserves A/B event counts (low activity 23/1, high 62/1) while tail energy changes; this is
not proof of perceptual macro separability or live-decay behavior.

## Historical Wave 6 preparation / Wave 7 engineering disposition (`8eb0061`)

**Superseded listening design:** the 12/8-trial RMS-only preparation below cannot establish attack/source
preservation. Its initial tooling self-review did not catch that confound or the implicit D1 default. The
remediation section below replaces this handoff; old generated files are not relabeled as new evidence.

The existing renderer now feeds `prepare_protect_listening.py`: dry/OFF/mild/medium/strong, surviving Fluid
placements and a fixed lower-residual control, hidden repeated OFF/medium trials, randomized order, raw
counterparts, normalization keys, blank eleven-dimension scorecard and review instructions. Local diagnostic
dry runs produced 12 Fluid and 8 C trials; float readback, common RMS and peak checks PASS. No human scores
are filled. RMS matching is explicitly not equal perceived loudness; reviewers can inspect the raw files.
Musical audio/permission/context and human conclusions will be provided later as instructed by the user.

Wave 6 self-review: ACCEPT handoff tooling, **human listening NOT RUN**. Wave 7 engineering disposition:
**REVISE / HOLD for product adoption**, retain this as research. Numerical mechanics work within declared
tests, but useful variable depth, preserved Droplet/Water identity, pumping tolerance and superiority over
simply reducing material remain unproven. Reject / Internal safeguard / User macro remains PENDING human
evidence; no fifth Host macro, production DSP, ADR acceptance or schema change is authorized by self-review.

## Historical combined validation and documentation review (`8eb0061`)

Final serial runs, MSVC 19.43 / Windows 11 10.0.22631, default 6-job safe wrapper:

| Preset | Configure / safe build | CTest | Duration |
|---|---|---|---|
| windows-asan | PASS | 18/18 PASS | 56.16 s |
| windows-debug | PASS | 18/18 PASS | 27.30 s |
| windows-release | PASS | 18/18 PASS | 14.33 s |

Commands from an MSVC developer environment (research remains opt-in):

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
python tools/check_markdown_links.py
python tools/check_portability.py
git diff --cached --check
```

`clang-format --dry-run --Werror` on all eight affected C++ headers/sources PASS after fixing comment wrapping.
Python compilation checks PASS. Scope assertions compared production src/tools/root tests/CMake/AGENTS and
all original A/B/D/C/features headers unchanged against `fc20370`; existing Host registry and M1 Exit,
Decay Revision B and M2 Exit sections are identical. New local anchors resolve. No hashes calculated.
Final listening-tool dry runs at `build/protect-listening-verified-fluid` and `build/protect-listening-verified-c`
PASS with 12/8 trials and recorded source commit/dirty state. Earlier runs are not substituted for this check.

Source provenance: runs used `08df5f4` plus this uncommitted PROTECT-EXP-001 working change, then submitted in
the same PR. Reports retain that dirty status; final formatting/includes/tests and documentation do not
retroactively turn measurements into clean-commit/formal provenance. Generated WAV/CSV/PNG/JSON/logs remain
ignored. The initial Python process crash and the detector's weak-onset finding are retained above; later
passing tests do not establish a cause for the Python crash.

Separate final Code Quality Review: no blocking implementation findings for this bounded research scope.
Reviewed cohesion/coupling, naming, includes, units/ranges, failed-prepare behavior, instance/lifetime ownership,
constant bounds, RNG continuity, finite OFF completion, no new globals/macros/dead paths or generic framework,
and no new process allocation/lock/I/O. Allocation instrumentation was NOT RUN; code review plus ASAN is not
a proof of production realtime suitability. Human artifacts/identity are not inferred from numerical bounds.

Comment & Documentation Pass and full synchronization review:

- Changed: Core Implementation Guide, Coding Plan, Perceptual Contract, Testing, Developer Sound Tools,
  Proposed ADR-0006, Project Status, Module Index, historical Wave 1 banner, this execution record,
  experiments index and research README. The earlier Parameters proposal remains accurate without another
  edit: no fifth accepted macro, registration, freeze or persisted field.
- Reviewed unchanged: Architecture v0.3, AGENTS, Code Standards, Documentation Governance, Collaboration
  Roles, GitHub Workflow, CONTRIBUTING, Accepted ADR-0001/0002/0003/0005, production module READMEs and root
  README. Approved production boundaries/behavior and milestone claims are unchanged. Perceptual template
  is sufficient; Decay revision gate is retained. Original SPIKE plan/EVIDENCE/REVALIDATION remain historical
  source-specific records; this follow-up is separately identified, not backdated into them.
- Cross-document result: objective implementation/evidence is distinguished from historical Wave 1 scope,
  formal EXP-W-002 readiness, human review and production adoption. v1.4/four-macro baseline, nine Host IDs,
  schema 1, existing M1/Decay/M2 gates and no production Protect/UI remain consistent.
- Not executed: new pluginval/DAW validation (no plugin change), musical listening, live Decay/product-macro
  automation, runtime allocation instrumentation, independent product acceptance or merge. Existing Host/
  latency CTest regression does not prove Protect has been integrated into a Host.

## Historical fourteen-field completion index (`8eb0061`)

| Required field | Evidence / disposition |
|---|---|
| 1. Reviewed sources | Wave 1 audit plus unchanged generator/features/config/renderer/test/performance code and task-relevant contracts; theory reference access limits retained. |
| 2. Baseline | `fc20370` main; Wave 1 `08df5f4`; v1.4, nine Host IDs/schema 1. |
| 3. Scope | Objective Waves 2–5, Wave 6 tooling, Wave 7 engineering disposition; human work deferred by user. |
| 4. Files | Three DSP headers, two property tests, two analysis/handoff scripts, existing research renderer/config/performance/CLI/CMake and synchronized docs; no production source. |
| 5. Decisions | Independent state, causal residual-only gain, exact static OFF and finite dB OFF ramp; retain D0/D1/F1/F2/F3; no winner/adoption. |
| 6. Tests | Serial safe three-preset builds/CTest, decoded renderer/property tests, 124 renders, analysis/event checks, listening-pack readback, formatting/document/scope checks. |
| 7. Results | Final 18/18 for each preset; 84+18+22 renders PASS; 12+8 handoff trials PASS; detailed exceptions/limitations above. |
| 8. Render/analysis | Ignored directories/commands above; source-annotated windows, raw metrics, traces, eight plots; no human scores. |
| 9. Realtime/performance | Bounded code-path review, ASAN and preliminary same-run wall-time increments; no formal CPU/deadline budget claim. |
| 10. Documentation | Full changed/unchanged/consistency review above. |
| 11. Limitations | Weak onset after loud history; partial-branch cancellation; C Motion N/A; static Decay only; dirty research provenance; Python transient crash retained. |
| 12. Decision | Engineering infrastructure ACCEPT for review; product hypothesis REVISE/HOLD, no independent approval or listening acceptance. |
| 13. Next step | Combined GitHub submission; human audio/conclusions and scoped detector follow-up before selection/adoption. |
| 14. Non-goals | Production DSP/Host/schema/UI/Routing/Ice, macro freeze, unlicensed material, independent approval or merge. |

## PR #37 remediation (2026-09-18)

The user's six review findings reopen Wave 6 tooling review. Their subsequent explicit instruction removes
SHA-256 from findings 3/6(c): use source names/descriptions and audio metadata, with no full or shortened hash.
No source/artifact hashes were computed in this remediation. Existing Git commit IDs remain code references.

Contract Review -> Implementation -> Functional Validation -> Code Quality Review -> Comment & Documentation
Pass -> Final Validation are tracked separately here. Scope is listening preparation, persistent-state comments,
regression/CTest/CI integration and affected documentation; no research DSP behavior or production code change.

| Finding | Remediation / evidence |
|---|---|
| Fixed-source primary listening | One common gain across dry, OFF, both detectors, topology/depth and lower-residual controls; decoded-audio regression recovers the same source coefficient after removing each scaled residual. |
| RMS evidence confound | Separate `fixed_source` and `rms_matched` directories, manifests, scorecards and review conclusions. RMS evidence is preference-supporting only; no shared/pooled conclusion or attack-preservation claim. Historical 12/8-trial packs are superseded. |
| Unresolved detector | Explicit D0/D1 blinded pairs, including all depth/topology conditions; D0 .01/.12 amplitude and D1 1/9 dB thresholds, all Protect fields pinned. These are bounded settings, not equally tuned families. Independent re-renders test actual selection, not only labels. |
| Selection gate | `DETECTOR_SELECTION.md` requests fixed-source trial/config evidence, reviewer, repeat consistency, quiet-after-loud/recovery/identity tradeoffs and rationale, including neither/revise. D1 is not selected by default. Wave 7 product decision is BLOCKED pending detector selection and human listening. |
| Provenance / frame semantics | Required source/author/license/permission/storage-policy JSON; original source name, frames/duration, channels, subtype/bit-depth, source snapshot, code revision/dirty state, independent DSP/randomization seeds, comparison frames/duration and separately declared appended tail. No ambiguous `frames`/`seed` fields or hash claims. |
| Persistent state | Detector/follower, latest detection, prepared depth/settings, attack/release coefficients, gainDb/offStartDb, OFF duration/countdown and readiness now explain purpose, units and reset/prepare behavior; DSP expressions unchanged. |
| Automated regressions | New real-renderer `protect_listening_test.py`: seven tests covering common carrier gain, D0/D1 configs/behavior, metadata/tail lengths, exact repeat config/gain/audio, deterministic order/audio and independent seeds, distinct evidence/blank scores, missing-rights rejection. CTest and hosted CI run it; CI installs existing `requirements-dsp.txt`. |

Each new pack has 21 Fluid / 13 Resonant trials, including hidden repeats of D0-OFF, D0-medium and D1-medium.
DSP seed is separate from randomization; the RMS pack uses randomization seed + 1 and records that actual value.
The original source bytes are copied locally once, and every renderer call uses the copy; storage permission
must cover the copy and derived audio. Names/metadata are identifying descriptions, not cryptographic identity.
Human-provided permission remains a claim to be reviewed by the material owner, not automatic authorization.

### Remediation validation and review

Contract Review and Implementation complete against the task-specific findings, Code Standards, testing and
build/CI contracts. Functional Validation used the real renderer; no mocked DSP or invented listening scores.

| Preset | Configure / safe build | CTest | Duration |
|---|---|---|---|
| windows-debug | PASS (6 jobs) | 19/19 PASS | 31.30 s |
| windows-release initial | PASS (6 jobs) | 18/19; existing renderer CLI Python process crashed | 14.46 s |
| windows-release focused rerun | Existing successful build | renderer CLI 1/1 PASS | 8.02 s |
| windows-release full rerun | Existing successful build | 19/19 PASS | 13.91 s |
| windows-asan | PASS (6 jobs) | 19/19 PASS | 53.14 s |

The new listening test passed in every preset, including the failed initial Release suite. Windows Application
Error identified `python.exe` / `python312.dll` 3.12.4 access violation `0xc0000005` at offset `0x12fd30` in the
initial existing renderer CLI test (2026-09-18 13:23 local time). This is not a sanitizer report against DSP.
The unchanged focused/full reruns used `PYTHONFAULTHANDLER=1` and passed; no root cause or fix is claimed.
The earlier historical Python failure above remains a separate event. Do not summarize the initial run as PASS.

Commands: the same configure/build_safe/CTest sequence shown in the historical section, with the added
`python -m pip install -r requirements-dsp.txt` prerequisite. The local environment already had these packages.
Also ran the seven-test script directly against Release (7/7), Python compilation, C++ clang-format check,
Markdown links, portability and Git diff whitespace checks. New CTest executes all seven assertions groups;
production/unit/Host/state/latency regressions remain in the 19-test suite.

Retained diagnostic packs: `build/protect-listening-v2-fluid` and `build/protect-listening-v2-c`, using
`testdata/input/envelope_response__gated_sine.wav`, source metadata in ignored
`build/protect-listening-source-v2.json`, DSP seed 42, randomization seeds 42/43 and 3 s appended tail.
Both packs passed readback: 21+21 Fluid and 13+13 C trials. Source is 96000 frames / 2 s / 48 kHz / stereo /
PCM24; comparison is 240000 frames / 5 s. Both fixed-source packs use carrier gain 1.0 for every condition.
These runs record `8eb0061` plus dirty remediation working state, not clean-commit measurement provenance.
Only blank scorecards were produced; human listening is NOT RUN. All audio/metadata/keys/logs remain ignored.

**Separate Code Quality Review:** no blocking finding in the bounded remediation. Reviewed common-gain
algebra, explicit detector thresholds/config forwarding, source snapshot consistency, manifest lengths/units,
randomization independence, hidden-repeat equality, input validation/no overwrite, dependency scope and missing
metadata rejection. Python preparation remains offline; C++ processing expressions and generator/state/RNG
behavior are unchanged. Comments distinguish history cleared by reset from cached configuration retained.
No new production abstraction, macro, global DSP state or runtime operation was introduced.

**Comment & Documentation Pass / Full Gate (testing and CI contract):**

- Changed: research README, TESTING, ENVIRONMENT, Coding Plan, Project Status, Module Index and this execution
  record. The README documents the new required metadata JSON and supersedes `--source-note`; there is no
  silent compatibility fallback to incomplete provenance or default D1 listening conditions.
- Reviewed unchanged: PARAMETERS (nine Host IDs/schema 1 and Protect proposal only), CODE_STANDARDS (comments
  now comply), DOCUMENT_GOVERNANCE (actual affected Full Gate rows), GITHUB_WORKFLOW and root README (portable
  build/review policy and production defaults unchanged), DEVELOPER_SOUND_TOOLS (offline-only boundary).
  Architecture/Accepted ADR impact is N/A: no production algorithm/interface/routing/latency/state change.
- Cross-document consistency: source versus padded frames, evidence separation, D0/D1 unresolved selection,
  Wave 7 BLOCKED, no production/Host control and opt-in research dependency are consistent. Historical
  measurements are preserved with historical labels. Documentation Review: PASS.

**Final Validation:** local checks above PASS, with the initial Release exception retained. Hosted CI must be
read on the uploaded exact PR head; local runs do not establish hosted success. Self-review is engineering
remediation only, not formal independent approval. Not run: musical listening, human detector selection,
pluginval/DAW acceptance, new CPU measurement or Waves 2–5 diagnostic sweeps (processing behavior unchanged),
runtime allocation instrumentation, product adoption or merge. Wave 7 remains BLOCKED.
