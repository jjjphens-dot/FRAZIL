# Bubble A1 execution record

## Status

ENGINEERING COMPLETE — local offline A1 implementation, validation and handoff prepared.
HUMAN ACCEPTANCE NOT RUN. UI integration remains explicitly deferred.

## Goal and authority

Implement the user-requested bounded Shared Excitation + Bubble A1 research reset.
The 2026-09-24 task attachment specifies physics, ranges, regressions, offline
ablations and performance evidence. The accompanying user instruction defers UI
integration: preview controls, session migration and UI transport remain deferred.
Production, B/D/C, Protect, Ice and routing are outside the write scope.

## Baseline and ownership

- Verified repository: `jjjphens-dot/FRAZIL`.
- Fetched main: `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`.
- PR #40 remains OPEN at `1444b4466a4bab8e1e60b59c399174d40f4f6491`.
- Review branch `codex/experiment/water-bubble-a1-review` starts at that PR head to
  preserve current B/D/C and research renderer behavior. This is a stacked
  research change, not a claim that PR #40 is merged/accepted.
- Original checkout's compatibility-matrix and unit-test edits are preserved.
- Engineering implementation/self-review: current agent. Human listening and
  acceptance: user/Sound Lead and independent reviewer. The user's follow-up
  authorizes a new branch and GitHub push for review; merge remains unauthorized.

## Success criteria

- Source-linked stereo A1; 128 radius bins, coherent physical frequency/damping,
  power-law population, bounded independent voices and selective pitch rise.
- A0 and exact B/D/BD/C regressions retained; legacy configs stay A0.
- Scientific provenance distinguishes physical model, empirical approximation,
  engineering assumptions and unimplemented coupled/geometry mechanisms.
- Serial safe Debug/Release/ASAN validation, partition/stereo/capacity properties,
  offline ablations, timing matrix and blank human-review handoff.
- Code Quality Review, Comment & Documentation Pass and Final Validation recorded.

## Completed

- Read task attachment; verified live main/PR heads and protected existing edits.
- Completed relevant canonical audit and recorded the A0 gap matrix before coding in
  [EXP-W-BA-001](../../experiments/water/EXP-W-BA-001.md).
- Original author-hosted van den Doel manuscript verified; modern original research
  distinguishes independent oscillators from geometry/transfer/coupling models.
- Unmodified PR #40 Release configure/safe build and CTest 25/25 passed (32.36 s).
  Baseline renderer retained locally before renderer source changes (binary timestamp
  21:53:19; first renderer/config edit 21:56:01). No checksum operation was used.
- Added separate A1 analyzer, physical table, voice pool and population coordinator;
  explicit offline renderer modes/config, property tests and listening/performance harness.
- First Debug safe build passed. A1 properties passed; overall suite 25/26 passed.
  Legacy C renderer failed with access violation (below). This is retained evidence.

## Next action / checkpoint

Two independent humans can review `build/bubble-a1/study-v2` using its blank forms.
Calibration/adoption requires their decisions; UI integration awaits a later user instruction.

## Blockers and limitations

Human listening is NOT RUN. UI work is explicitly deferred.
Original Minnaert DOI retrieval failed on first web attempt; pursue an original
paper/official source and record access limits rather than inventing verification.

### Preserved Debug renderer fault

2026-09-24 first Debug suite: legacy `c-residual`, 48 kHz, block128, seed42,
motionDepth0, motionIntervalSeconds.02 returned 0xc0000005. Windows event offset
0x422dc maps via LLVM symbolizer and the preserved matching PDB to
`WaterExcitationFeatures.h:56` (legacy follower lambda coefficient access), matching
the location category previously recorded by PR #40. A1 was not selected. Thirty
isolated exact-input repetitions passed; this is non-reproduction, not resolution.
Original binary/PDB, event, suite log and reproduction script/results remain under
ignored `build/bubble-a1/crash`. No legacy DSP workaround or acceptance claim.


## Offline render validation (2026-09-24)

Historical first implementation: the following v1 pack/table predate the final linked
AR-energy carrier correction. They remain reproducible comparison evidence, not the
final A1 listening pack. Final carrier results are recorded separately below.

Five inputs: four previously authorized local sampling-pack files and the existing
six-second generated engineering pad. No external music was downloaded. The pad is
not representative musical-pad acceptance. Local pack: `build/bubble-a1/study-v1`.

- 70/70 finite cases, decoded repeat/block128-versus257 identity PASS.
- 30/30 decoded exact comparisons to the preserved pre-edit PR #40 Release renderer:
  A0, B, D, BD, C and ABD for each source. No legacy DSP file changed.
- Independent file-level verification of Full and RMS support equations PASS;
  maximum Full float-rounding error 2.07e-8. All matching gains in [0,1].
- Two independent CSV forms: 60 rows each, identity/ratings blank, decisions all
  NOT ASSESSED, every referenced audio file exists. Human listening NOT RUN.
- Maximum raw residual peak across the pack -18.13 dBFS. No capacity drops/steals.
- Default A1-4 source-window residual RMS spans -87.85..-50.97 dBFS. Some material
  remains very quiet. This is not an audibility or improvement PASS.
- Default observed median radii .264.. .281 mm; rise fractions 0..0.006. The requested
  gamma2/depth10/cutoff.9 defaults favor small/short and mostly non-rising events.
- Increasing Size changes both frequency and duration/energy: low-to-high residual
  RMS rises roughly 9 dB in this pack. Distribution normalization preserves expected
  initial squared amplitude, not duration-integrated energy or perceived loudness.
- Motion0 matched support is unassessable (zero target); fixed-source silence remains
  valid. Fixed-source/context and matched character comparisons stay separate.

These observations identify listening/calibration risks, not grounds to secretly
change requested physical defaults. Human choices about water identity, artifacts,
source preservation and preferred calibration remain open. Independent oscillators
cannot supply coupled-cloud low-frequency behavior by increasing voice capacity.

## Preliminary performance matrix

Windows 11 10.0.22631, Intel Core i9-14900HX, MSVC 19.43.34809.0, Release.
Source constant stereo +.7/-.7, seed42, block128. Warmup500, measured3000 blocks;
nearest-rank percentiles. Default profile uses requested defaults. Dense profile:
radii10..50 mm, persistence4, rate10000, depth exponent1. Actual event rates are
occupancy counts, not the configured rate. Active values below are block-end means.
No other build/test pipeline ran during this measurement. Ordinary OS scheduling
was not controlled; outliers are retained. CSV: `build/bubble-a1/performance.csv`.

| Hz | Capacity | Profile | Mean us | P95 us | P99 us | Worst us | Mean active | Events/s |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 44100 | 64 | default | 4.7224 | 5.7 | 7.1 | 243.1 | 1.04533 | 694.345 |
| 44100 | 64 | dense | 42.2569 | 68.8 | 96.6 | 470.9 | 63.999 | 6539.2 |
| 44100 | 128 | default | 4.8127 | 5.8 | 6.6 | 88.5 | 1.04533 | 694.345 |
| 44100 | 128 | dense | 65.4106 | 82.6 | 141.8 | 329.5 | 127.999 | 6539.2 |
| 44100 | 256 | default | 4.51797 | 5.4 | 5.9 | 75.5 | 1.04533 | 694.345 |
| 44100 | 256 | dense | 134.67 | 174.8 | 259.9 | 431.9 | 255.999 | 6539.2 |
| 44100 | 512 | default | 4.55827 | 5.6 | 9.6 | 51.5 | 1.04533 | 694.345 |
| 44100 | 512 | dense | 275.15 | 378.7 | 494.4 | 688.9 | 511.999 | 6539.2 |
| 44100 | 1024 | default | 4.6652 | 5.4 | 6.4 | 76.5 | 1.04533 | 694.345 |
| 44100 | 1024 | dense | 587.172 | 778.2 | 942.3 | 1326.3 | 1024 | 6539.2 |
| 48000 | 64 | default | 4.39397 | 5.3 | 8.7 | 52.9 | 1.05067 | 697.75 |
| 48000 | 64 | dense | 37.3912 | 45.5 | 79.6 | 249.8 | 63.9983 | 6580.88 |
| 48000 | 128 | default | 4.7998 | 5.8 | 6.7 | 95.9 | 1.05067 | 697.75 |
| 48000 | 128 | dense | 62.233 | 86.7 | 130.9 | 445.1 | 127.998 | 6580.88 |
| 48000 | 256 | default | 4.49893 | 5.4 | 7 | 80.9 | 1.05067 | 697.75 |
| 48000 | 256 | dense | 133.358 | 179.2 | 262 | 511.6 | 255.998 | 6580.88 |
| 48000 | 512 | default | 4.54487 | 5.4 | 7.7 | 63.7 | 1.05067 | 697.75 |
| 48000 | 512 | dense | 274.952 | 380.7 | 498.6 | 689.4 | 511.998 | 6580.88 |
| 48000 | 1024 | default | 4.7308 | 5.5 | 6.3 | 120.7 | 1.05067 | 697.75 |
| 48000 | 1024 | dense | 588.82 | 780.2 | 972.1 | 1269.2 | 1024 | 6580.88 |
| 96000 | 64 | default | 4.5087 | 5.2 | 10.1 | 115.2 | 1.138 | 707 |
| 96000 | 64 | dense | 33.7816 | 39.6 | 79.2 | 185.8 | 63.998 | 6800.25 |
| 96000 | 128 | default | 4.70907 | 5.4 | 7.5 | 130.1 | 1.138 | 707 |
| 96000 | 128 | dense | 70.1336 | 150 | 254.6 | 592.8 | 127.998 | 6800.25 |
| 96000 | 256 | default | 4.47333 | 5.7 | 8.5 | 100.4 | 1.138 | 707 |
| 96000 | 256 | dense | 178.266 | 449.6 | 577.5 | 805.7 | 255.998 | 6800.25 |
| 96000 | 512 | default | 5.39783 | 10.3 | 12 | 124.4 | 1.138 | 707 |
| 96000 | 512 | dense | 354.318 | 661.1 | 996.8 | 1895.9 | 511.998 | 6800.25 |
| 96000 | 1024 | default | 5.73253 | 10.9 | 25.8 | 239.8 | 1.138 | 707 |
| 96000 | 1024 | dense | 736.751 | 1375.2 | 2089.2 | 2635.1 | 1024 | 6800.25 |

At 96 kHz, a128-frame period is1333.33 us. Dense1024 P95/P99/worst exceed that period;
dense512 worst also exceeds it. These are unsupported real-time stress conditions,
not a passing device/Host budget. Default active mean stays around1.05..1.14 despite
capacity changes, supporting active-list scaling. Raw sums are not limited; increasing
physical density can raise level as well as cost. No formal production CPU claim.

## Independent Code Quality Review

Reviewed separately from test execution: cohesion, dependency direction, config validation,
units, event/voice lifetime, active/free storage bounds, RNG ownership, stereo symmetry,
phase integration, overflow/downshift, counters, raw-sum ownership and legacy preservation.
Found and corrected missing normalized-range rejection in the offline macro helper;
also removed the full A1 population include from the shared configuration reader.
Added parameter-corner and actual A1+B/D composition assertions during this pass.
Final diagnostic review also found the renderer's generic Bubble event/first-frame
fields still read the disabled A0 object in A1 mode. Routed those fields to actual
A1 starts and added CLI assertions. This changes observation only, not audio; final
serial suites and a fresh listening pack verify the corrected output.
The final source-energy audit identified that the first carrier used short-window RMS
directly for level. It now uses stereo direction from that window and common amplitude
from linked fast AR power. A sustained100Hz test checks phase-robust drive. The first
updated test run exposed an old fixture that assumed full amplitude after only2ms;
that fixture needs detector settling before its zero-crossing assertion. This is a
test precondition correction, not a weakened DSP requirement or changed attack time.

No mutable global state, macros replacing abstractions, production dependencies,
callback I/O, locks, growing containers, hidden first-use allocation, or per-channel
event/RNG ownership were found in the new process paths. A1 fixed storage is allocated
once by the offline renderer before processing. Prepare/reset clear fixed arrays.
This allocation/lock assessment is a code-path review, not runtime allocation instrumentation.
Test and performance executable allocations occur outside the measured processing loop.
Full-pool victim selection is O(active) at event time; ordinary processing visits only
active slots. Raising idle capacity does not add a1024-slot per-sample scan.

The retained legacy Debug fault has no demonstrated root cause. Successful later runs
and ASAN cannot be presented as its repair. A1 source-level boundaries and exact legacy
output comparisons support isolation, not a blanket proof about native process stability.

## Comment & Documentation Pass

Updated experiment record (sources, A0 matrix, equations, controls, assumptions, CLI,
mapping and human gates), this execution record, experiment README/mapping, TESTING,
MODULE_INDEX, PROJECT_STATUS and the debug guide's explicit A1-not-connected notice.
Comments state SI units, source energy/polarity proxy, bounded domains, acoustic versus
resource state, occupancy approximation, queued stealing and process ownership.

Reviewed without changes: Architecture, Coding Plan, Parameters, Code Standards,
Document Governance, Core Implementation Guide, Perceptual Contract, accepted EXP-W-001
brief, Developer Sound Tools, ADR-0006, existing listening execution/handoff and legacy
module contracts. Production interfaces, nine Host parameters/schema1, routing, latency,
random persistence, performance budgets, acceptance ownership and prior evidence do not
change. Full Gate impact rows for research API/testing/status are synchronized; other
contract rows are N/A. UI/session runtime and source DSP files have no diff. The new
status note does not claim milestone closure or Host/device support.

Cross-document consistency: A1 is opt-in offline research; v0.2/session v5 remain
legacy; no physical constants are claimed as measured FRAZIL acoustics; human review
and production acceptance remain pending. New files each serve the requested physics,
analysis, bounded pool, orchestration, tests, offline evidence or auditable handoff need.

## User-requested sixteen-item delivery index

1. **Exact repository/base:** `jjjphens-dot/FRAZIL`; review branch
   `codex/experiment/water-bubble-a1-review`, based on PR #40
   `1444b4466a4bab8e1e60b59c399174d40f4f6491`, plus this Bubble A1 change.
   Main reference `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`.
   Branch publication is user-authorized; no merge is authorized.
2. **Documents reviewed:** canonical list and status discrepancies are in
   [EXP-W-BA-001](../../experiments/water/EXP-W-BA-001.md#documentation-audit).
3. **Scientific sources:** original van den Doel manuscript; author-hosted
   Harmonic Fluids, Complex Acoustic Bubbles and Coupled Bubbles; original 2018
   Phillips/Agarwal/Jordan experiment. Source access limits and model omissions
   are explicit; Minnaert1933 full text and FOAM official page were unavailable.
4. **A0 gap matrix:** recorded before implementation in EXP-W-BA-001 Phase0.
5. **Architecture:** linked energy analysis -> single scheduler ->128 radius bins
   -> coherent independent stereo-linked voices -> bounded population residual.
6. **Files changed:** four new DSP headers; new property/CLI/performance tests and
   study driver; extended existing renderer/config/exporter/CMake; experiment and
   execution records plus six existing documentation entry points. No `src/`,
   legacy DSP primitive, preview runtime or session code edits.
7. **Formulas/assumptions:** SI Minnaert constants, empirical damping, reciprocal
   lifetime, normalized radius amplitude, depth proxy, occupancy law and integrated
   capped rise are specified in EXP-W-BA-001; no absolute-pressure/geometry claim.
8. **Controls:** full range/default table in EXP-W-BA-001. Strict numeric config,
   explicit offline mode; old configurations remain A0.
9. **A0/A1 differences:** physical state replaces independent Hz/tau families,
   window energy replaces trigger PCM, weighted population replaces uniform16,
   capacity no longer attenuates output, bounded release replaces hard stealing.
10. **Tests:** serial safe Debug/Release/ASAN results below. Historical first-run
    Debug fault retained; later success is not a root-cause fix.
11. **Rendering/listening:**70 comparison cases,30 exact legacy references, fixed
    source and RMS support separated, two independent60-row blank reviewer forms.
12. **Capacity/performance:** complete30-row table above covers64/128/256/512/1024
    at44.1/48/96kHz, actual occupancy, mean/P95/P99/worst and stated overload limits.
13. **Stereo:** exact swap, isolation/mirror, dual mono, anti-phase noncollapse and
    partition/reset properties pass. This does not establish spatial listening quality.
14. **Known risks:** legacy intermittent access violation; quiet/default sparse A1;
    Size duration/level tradeoff; very rare default rise; high-density deadline misses;
    independent spherical model omits fluid geometry, splitting, coupling and radiation.
15. **Documentation:** affected records synchronized, unchanged contracts reviewed
    with reasons above. Repository links/portability/format checks recorded below.
16. **Human decisions:** Sound Lead and independent reviewer must assess water
    identity, source preservation, artifacts, calibration and macro semantics. No
    perceptual ACCEPT, production adoption or permission for deferred UI is inferred.


## Final linked-AR carrier evidence — current delivery

Current pack: `build/bubble-a1/study-v2`; current timing:
`build/bubble-a1/performance-v2.csv`. Earlier v1 audio/measurements remain historical.
Five sources in the same order as v1 (input5 is the generated engineering pad).
70/70 finite/repeat/block128-vs257 cases and30/30 exact A0/B/D/BD/C/ABD comparisons
PASS. Independent decoded Full/RMS equations and both60-row blank forms PASS;
maximum Full float-rounding error2.07e-8. No pack capacity drops or steals.
A1 generic event/first-frame diagnostics now report actual A1 starts.

Source-window residual RMS, dBFS (measurements, not listening scores):

| Input | A0 | A1-1 | A1-2 | A1-3 | A1-4 |
| --- | --- | --- | --- | --- | --- |
| 1 | -61.92 | -44.62 | -65.22 | -65.77 | -65.77 |
| 2 | -57.75 | -37.73 | -57.61 | -52.99 | -52.99 |
| 3 | -74.95 | -45.65 | -67.21 | -81.64 | -81.64 |
| 4 | -46.38 | -38.09 | -55.48 | -48.97 | -48.97 |
| 5 | -67.50 | -42.04 | -59.91 | -67.54 | -67.54 |

Final A1-4 RMS range is-81.64..-48.97dBFS.
It remains weak on some inputs; neither the AR correction nor physical consistency
establishes perceptual improvement. Radius distributions and rare default rise remain
as reported above. Size increases lower actual residual spectral centroids substantially,
not merely change gain; level and duration still change and require separate listening.
No human decisions were filled in by the agent.

Current performance (same machine, profiles and measurement protocol as above):

| Hz | Capacity | Profile | Mean us | P95 us | P99 us | Worst us | Mean active | Events/s |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 44100 | 64 | default | 4.71573 | 5.8 | 7.3 | 72.7 | 1.04533 | 694.345 |
| 44100 | 64 | dense | 39.8559 | 49.4 | 84.5 | 333.6 | 63.999 | 6539.2 |
| 44100 | 128 | default | 5.1321 | 6 | 9.3 | 129.2 | 1.04533 | 694.345 |
| 44100 | 128 | dense | 67.7211 | 84.2 | 139.7 | 306.8 | 127.999 | 6539.2 |
| 44100 | 256 | default | 4.85867 | 5.7 | 6.3 | 14.3 | 1.04533 | 694.345 |
| 44100 | 256 | dense | 142.354 | 182.7 | 286.6 | 450 | 255.999 | 6539.2 |
| 44100 | 512 | default | 4.84323 | 5.7 | 6.5 | 152 | 1.04533 | 694.345 |
| 44100 | 512 | dense | 285.255 | 373.9 | 511.5 | 1063 | 511.999 | 6539.2 |
| 44100 | 1024 | default | 4.70157 | 5.5 | 8 | 84.2 | 1.04533 | 694.345 |
| 44100 | 1024 | dense | 622.321 | 815.5 | 1035.3 | 1381.3 | 1024 | 6539.2 |
| 48000 | 64 | default | 5.05067 | 6 | 7.1 | 76.4 | 1.05067 | 697.75 |
| 48000 | 64 | dense | 39.736 | 48 | 80.7 | 198.7 | 63.9983 | 6580.88 |
| 48000 | 128 | default | 5.17817 | 6.7 | 8.9 | 75.9 | 1.05067 | 697.75 |
| 48000 | 128 | dense | 69.922 | 89 | 142 | 345.5 | 127.998 | 6580.88 |
| 48000 | 256 | default | 4.87963 | 5.7 | 8 | 65.2 | 1.05067 | 697.75 |
| 48000 | 256 | dense | 142.696 | 177 | 257.2 | 666.3 | 255.998 | 6580.88 |
| 48000 | 512 | default | 5.22557 | 8.2 | 10.2 | 75.4 | 1.05067 | 697.75 |
| 48000 | 512 | dense | 285.025 | 374 | 498.5 | 706.1 | 511.998 | 6580.88 |
| 48000 | 1024 | default | 4.82273 | 5.7 | 10.8 | 84.1 | 1.05067 | 697.75 |
| 48000 | 1024 | dense | 602.16 | 800.8 | 980.2 | 1292.6 | 1024 | 6580.88 |
| 96000 | 64 | default | 5.26737 | 6.9 | 12.3 | 169.6 | 1.138 | 707 |
| 96000 | 64 | dense | 34.8763 | 41.4 | 76.9 | 273.8 | 63.998 | 6800.25 |
| 96000 | 128 | default | 4.6338 | 5.4 | 6.3 | 157 | 1.138 | 707 |
| 96000 | 128 | dense | 60.9095 | 73.4 | 131.2 | 368.5 | 127.998 | 6800.25 |
| 96000 | 256 | default | 4.8176 | 5.5 | 6.5 | 137.8 | 1.138 | 707 |
| 96000 | 256 | dense | 131.192 | 169.5 | 253.8 | 791.8 | 255.998 | 6800.25 |
| 96000 | 512 | default | 4.72273 | 5.5 | 7 | 182.4 | 1.138 | 707 |
| 96000 | 512 | dense | 262.134 | 361.7 | 465.9 | 680.3 | 511.998 | 6800.25 |
| 96000 | 1024 | default | 4.942 | 5.6 | 7.4 | 154.1 | 1.138 | 707 |
| 96000 | 1024 | dense | 590.922 | 804.5 | 986.3 | 1421.9 | 1024 | 6800.25 |

Current96kHz/dense1024 worst1421.9us exceeds the1333.33us block period; its current
P99 is986.3us. Earlier v1 had larger tail outliers, retained above. OS variation is
not a DSP repair. These measurements do not authorize a production realtime budget.

## Final Validation / completion boundary

All six required stages were performed: Contract Review -> Implementation ->
Functional Validation -> independent Code Quality Review -> Comment & Documentation
Pass -> Final Validation. Attachment phases0-5/7-9 are engineering-complete; phase6
is offline config/diagnostics only because the user deferred UI/session integration;
phase10 handoff is prepared, with human decisions pending.

Executed serially after the final AR-energy change and settled-fixture correction:

| Preset | Configure / safe build (6 jobs) | CTest |
| --- | --- | --- |
| windows-debug | PASS |27/27 PASS,69.10s |
| windows-release | PASS |27/27 PASS,27.35s |
| windows-asan | PASS |27/27 PASS,120.54s |

Commands: `cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
-DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE:FILEPATH=<discovered-python>`,
`python tools/build_safe.py --preset <preset>`, then
`ctest --preset <preset> --output-on-failure`. The ignored helper
`build/bubble-a1/final-validation.cmd` runs Debug, Release and ASAN in that order,
stopping on error. Final validation redirected TEMP/TMP to ignored workspace-local
`build/bubble-a1/tmp`. Python package interpreter, CMake cache and CTest interpreter
were verified to agree. All safety preflights passed; none was bypassed.

Executed the committed `render/bubble_a1_study.py` on five explicit local sources,
the Release `frazil_water_bubble_a1_performance` executable, and independent local
`build/bubble-a1/verify_final_artifacts.py`. Logs and reports remain ignored.
The first legacy Debug crash and the AR fixture failure remain separately preserved;
subsequent success is not retrospective erasure or a claimed legacy crash fix.

Not performed: new Hosted CI, formal independent GitHub approval, human listening,
representative musical-pad acceptance, UI integration/native GUI acceptance,
pluginval/DAW/audio-device callback validation, production adoption or merge.
These are not supplied by local tests or historical PR #40 results. Original checkout
edits and the PR #40 checkout remain untouched. No source audio or generated binaries
are included in the deliverable. The subsequent user request authorizes committing
this reviewed deliverable to the new review branch and pushing it to GitHub.


Final repository quality commands all PASS: `python tools/check_markdown_links.py`,
`python tools/check_portability.py`, both `tools/test_check_*.py` scanner regression
scripts, `python tools/check_vscode_tasks.py`, `clang-format --dry-run --Werror` on
changed C++ headers/sources, Python syntax compilation and `git diff --cached --check`.
No unstaged implementation diff remains after staging the reviewed deliverable.

## Complete changed-file inventory

- `docs/DEV_UI_WATER_DEBUG_GUIDE.md`
- `docs/MODULE_INDEX.md`
- `docs/PROJECT_STATUS.md`
- `docs/TESTING.md`
- `docs/evidence/WATER_BUBBLE_A1_EXECUTION.md`
- `experiments/water/EXP-W-BA-001.md`
- `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt`
- `experiments/water/SPIKE-W-DSP-001/README.md`
- `experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md`
- `experiments/water/SPIKE-W-DSP-001/dsp/BubbleA1.h`
- `experiments/water/SPIKE-W-DSP-001/dsp/BubbleA1Model.h`
- `experiments/water/SPIKE-W-DSP-001/dsp/BubbleA1VoicePool.h`
- `experiments/water/SPIKE-W-DSP-001/dsp/SharedExcitationAnalyzer.h`
- `experiments/water/SPIKE-W-DSP-001/render/ReadConfig.h`
- `experiments/water/SPIKE-W-DSP-001/render/bubble_a1_study.py`
- `experiments/water/SPIKE-W-DSP-001/render/render_main.cpp`
- `experiments/water/SPIKE-W-DSP-001/render/research_cases.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_cli_test.py`
- `experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_performance.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_tests.cpp`
