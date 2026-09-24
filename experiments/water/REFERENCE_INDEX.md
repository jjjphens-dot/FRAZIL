# EXP-W-001 reference index v0.1

> Provisional local inventory, 2026-09-16. 12 PURE + 8 MUSICAL WAVs from three songs.
> Source: user-supplied Sample_Packs and Sample_Songs. Author/pack edition/license/redistribution terms are UNVERIFIED.
> Audio stays in the external library; no recording is committed, uploaded or accepted as LISTENING-001.

## Registry and extension

[REFERENCE_INDEX.csv](REFERENCE_INDEX.csv) owns stable IDs, relative paths, filename hints,
song groups, excerpt numbers and split assignment. Metadata below is an OBSERVED header snapshot,
not a perceptual conclusion or content-integrity certification. No hashes were computed.
Current six development references are W-P001/002/003/009 and W-M001/007; all others are unassigned.
Header-only intake does not certify a holdout. All Flux and Woelvinquesh excerpts are in development
song groups once one excerpt is analyzed; remaining excerpts cannot become independent unseen-song holdout.

To add material, append the next unused P or M ID without renumbering/deleting old rows. Record source,
license, song/recording group and intended question; mark perceptual fields PENDING/UNREVIEWED until heard.
If a recording is replaced, assign a new ID and document its relationship; same filename/metadata does not
prove identical content. Choose development/holdout by source group before content use, then inspect <=6 IDs.
Keep original author/title strings; filename hints do not prove water type or processing method.
Use song excerpt numbers only as identifiers; crop positions in the original tracks remain unknown.

## Reproduce locally

Use the existing requirements-dsp.txt environment. Set FRAZIL_REFERENCE_ROOT to your local library root
in the current shell or ignored local configuration. No personal path belongs in tracked files.

```powershell
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-P001 W-P002 W-P003 W-P004 W-P005 W-P006 --output testdata/rendered/exp-w-001/intake-01.json
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-P007 W-P008 W-P009 W-P010 W-P011 W-P012 --output testdata/rendered/exp-w-001/intake-02.json
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-M001 W-M002 W-M003 W-M004 W-M005 W-M006 --output testdata/rendered/exp-w-001/intake-03.json
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-M007 W-M008 --output testdata/rendered/exp-w-001/intake-04.json
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-P001 W-P002 W-P003 W-P009 W-M001 W-M007 --analyze --output testdata/rendered/exp-w-001/qa-01.json
```

No resampling, trimming, audio normalization or source separation occurs. Analysis reuses
[analyze_testdata.py](../../tools/analyze_testdata.py). Large references stay header-only until selected.

## Observed metadata

| ID | Relative source path | Hz | Channels | Encoding | Frames | Seconds |
|---|---|---:|---:|---|---:|---:|
| W-P001 | `Sample_Packs/BubbleSurfaceLarge_SFXB.4_1.wav` | 96000 | 2 | PCM_24 | 161792 | 1.685333 |
| W-P002 | `Sample_Packs/CaveWaterFlow_BW.53725.wav` | 48000 | 2 | FLOAT | 1176000 | 24.500000 |
| W-P003 | `Sample_Packs/ChlorinePourInto_SFXB.4785.wav` | 96000 | 2 | PCM_24 | 1022976 | 10.656000 |
| W-P004 | `Sample_Packs/DripsBodyStandUp_S08WR.37.wav` | 96000 | 2 | PCM_24 | 1126912 | 11.738667 |
| W-P005 | `Sample_Packs/ForestStream_BW.53693.wav` | 96000 | 2 | PCM_24 | 10959872 | 114.165333 |
| W-P006 | `Sample_Packs/FountainRun_BW.62841.wav` | 96000 | 2 | PCM_24 | 14018048 | 146.021333 |
| W-P007 | `Sample_Packs/LakeDockWavesSplash_SFXB._1.wav` | 48000 | 2 | FLOAT | 366657 | 7.638687 |
| W-P008 | `Sample_Packs/ObjectSurface_S08WR.71.wav` | 96000 | 2 | PCM_24 | 329216 | 3.429333 |
| W-P009 | `Sample_Packs/PebbleDropSplash_SFXB.4803.wav` | 96000 | 2 | PCM_24 | 32256 | 0.336000 |
| W-P010 | `Sample_Packs/SubmergeContainer_SFXB.4860.wav` | 96000 | 2 | PCM_24 | 1342976 | 13.989333 |
| W-P011 | `Sample_Packs/UnderwaterBubbles_SFXB.4876.wav` | 96000 | 2 | PCM_24 | 301568 | 3.141333 |
| W-P012 | `Sample_Packs/WaterWalkWadeFast_SFXB.4909.wav` | 96000 | 2 | PCM_24 | 2720256 | 28.336000 |
| W-M001 | `Sample_Songs/Aut0,ILusMin,Shizu - Flux (feat. Shizu)1.wav` | 48000 | 2 | FLOAT | 1656000 | 34.500000 |
| W-M002 | `Sample_Songs/Aut0,ILusMin,Shizu - Flux (feat. Shizu)2.wav` | 48000 | 2 | FLOAT | 2736000 | 57.000000 |
| W-M003 | `Sample_Songs/Aut0,ILusMin,Shizu - Flux (feat. Shizu)3.wav` | 48000 | 2 | FLOAT | 2472000 | 51.500000 |
| W-M004 | `Sample_Songs/ILusMin - Diphylleia (山荷叶)1.wav` | 48000 | 2 | FLOAT | 33000 | 0.687500 |
| W-M005 | `Sample_Songs/ILusMin - Diphylleia (山荷叶)2.wav` | 48000 | 2 | FLOAT | 90000 | 1.875000 |
| W-M006 | `Sample_Songs/ILusMin - Diphylleia (山荷叶)3.wav` | 48000 | 2 | FLOAT | 7042176 | 146.712000 |
| W-M007 | `Sample_Songs/削除 - Woelvinquesh1.wav` | 48000 | 2 | FLOAT | 576000 | 12.000000 |
| W-M008 | `Sample_Songs/削除 - Woelvinquesh2.wav` | 48000 | 2 | FLOAT | 1420756 | 29.599083 |

## First six: technical observations only

Full-file analysis, NumPy 2.5.3 / SciPy 1.18.1 / soundfile 0.14.0. Levels are sample-domain
full-scale values, not LUFS or true peak. RMS pools all channels; DC is the signed pooled mean and
can hide channel cancellation. Correlation describes the first two channels only. A value of 1
can also be the analyzer's zero-variance fallback; it alone cannot prove dual-mono.

| ID | finite | Sample peak dBFS | RMS dBFS | DC linear | Crest dB | L/R correlation |
|---|---|---:|---:|---:|---:|---:|
| W-P001 | True | -0.499 | -21.816 | -0.00000594 | 21.317 | 1.000000 |
| W-P002 | True | -12.745 | -34.867 | 0.00000011 | 22.122 | 0.324542 |
| W-P003 | True | -0.501 | -31.912 | 0.00000029 | 31.411 | 1.000000 |
| W-P009 | True | -4.498 | -23.195 | 0.00000006 | 18.697 | 1.000000 |
| W-M001 | True | -0.000 | -11.281 | 0.00178840 | 11.281 | 0.723850 |
| W-M007 | True | -0.264 | -13.928 | 0.00001029 | 13.664 | 0.295652 |

- OBSERVED (measurement): six decoded files are finite, with sample peaks below 1. This is not
  a perceptual PASS, true-peak clearance or proof that the source recordings were never clipped.
- OBSERVED (measurement): W-M001 sample peak is near full scale; gain increases need headroom checks.
  Song versus pure-water RMS differs substantially; raw playback cannot claim loudness-controlled comparison.
- OBSERVED (metadata): W-P009 is a very short event. Whole-file averages across different durations
  and silent margins are not directly comparable trait evidence.
- HYPOTHESIZED: flow/pour/bubble/impact filename coverage may help separate continuous movement
  from input-excited material language. Human observations are needed before trait extraction.
- NOT RUN: listening, LUFS/true peak, perceptual fusion classification, Level 2/3 analysis,
  negative-anchor validation, source separation, A/B matching, holdout validation and license verification.

## Human calibration update — 2026-09-17

User feedback on W-P002/W-P003/W-M001/W-M007 and Human Review ACCEPT for brief b616533 are recorded in
[LISTENING_LOG.md](LISTENING_LOG.md). [Round 01 Agent first-pass](ROUND_01_COMMON_WATER.md) remains the
original numerical/visual record; agent auditory interpretation is still incomplete. Cave reverberation and
the musical bell/arrangement are explicit confounds. No exact time windows/playback levels, Fused/Layered
classification, Decay sweep or engineering acceptance were supplied in that listening-feedback round. The metadata/QA above remains historical;
this index is provenance, not a perceptual label key.

## Definition closeout — 2026-09-18

Engineering preliminary PASS and user-reported final oral acceptance are recorded in
[brief sections 11–12](EXP-W-001_PERCEPTUAL_BRIEF.md#11-engineering-feasibility-review-human-review-后).
No new reference was heard or licensed by that acceptance record. Further negative/Decay calibration,
source-preservation listening and licensed shared-corpus preparation remain LISTENING-001 / EXP-W-003 work.

## Historical intake and closeout validation

Consolidated from notes.md and task_plan.md, which are no longer active plans.
Current definition authority is brief sections11-12; HI-01..HI-10 user wording and
corrections remain in LISTENING_LOG. Agent initial interpretations remain in ROUND_01.
Q16/Q17 are optional follow-ups; negative/Decay/reference coverage remains assigned
to LISTENING-001/EXP-W-003, not silently completed.

## Actual baseline and scope

2026-09-16: clean worktree at `2508d2b`, origin verified as FRAZIL, new local branch
`codex/exp-w-001-water-identity`. Local `main` is older; no reset/rebase or remote mutation performed.
`gh issue view 17 --json title,body,state` confirmed OPEN and the canonical perceptual-definition scope.
Existing historical `experiment/music-water-brief` draft was read for prior source-preservation,
diagnostic mapping and loudness evidence ideas; it predates the current dual-mode wording and was not
cherry-picked or treated as accepted. No current production implementation was duplicated.

## Evidence and limitations

- 20 external WAVs: 12 real-water, eight excerpts of three songs. Four metadata batches: 6/6/6/2.
- Six development references decoded/analyzed using the existing analyzer; all finite, sample peaks below 1.
- Metadata and sanitized numeric summary: [REFERENCE_INDEX.md](REFERENCE_INDEX.md).
  Full local JSON stays under ignored `testdata/rendered/exp-w-001/`.
- Whole-file RMS differences and near-full-scale W-M001 mean raw playback is not a controlled loudness comparison.
  Very short W-P009 is an event reference, not evidence of sustained motion.
- Filename hints and user provenance identify research questions; no hearing was performed by the agent.
  Fusion, Water Identity, source preservation and modal/physical interpretation remain unverified.
- No holds on local read/analysis are needed for missing redistribution terms. The files stay local;
  author/pack edition/license verification remains required before formal shared corpus adoption.

## Validation record

Executed using the existing project .venv Python with NumPy 2.5.3, SciPy 1.18.1 and soundfile 0.14.0:
the five commands in REFERENCE_INDEX (local root supplied at runtime) completed successfully.
Final checks:

- `python tools/check_markdown_links.py`: PASS (tracked files).
- `python tools/check_portability.py`: PASS (tracked files).
- `git diff --check`: PASS; Git reports only normal LF/CRLF conversion warnings.
- Explicit calls to the existing scan helpers on new untracked Water .md/.csv/.py files:
  links, portability and trailing whitespace PASS. No staging needed to include new files in review.
- Subprocess boundary checks: seven IDs, duplicate ID, unknown ID, analysis of an unassigned reference,
  output outside the ignored tree and non-JSON output all rejected with exit 2 and no output creation.
- Registry check: 20 unique IDs; all relative Unicode source paths resolve to real local files.
- Code Quality Review: narrow offline adapter; no production coupling; existing analysis reused;
  output constrained to JSON; no audio mutation; no mutable runtime global, macro or new DSP state.
  Resource limitation: existing analyzer reads one full selected file at a time; it is not a streaming analyzer.
- Comment & Documentation Pass: explains stable IDs, external paths, local-only output, proxy limitations,
  provisional status, reviewer ownership and unchanged stage/Host boundaries. Consistency result: PASS
  for this branch's bounded preparation; human contract acceptance remains PENDING.

NOT RUN: human hearing/independent reviewer acceptance, anonymous audio-pack generation, loudness matching,
LUFS/true peak, acoustic Level 2/3, candidate render/ablation/Golden/holdout, license verification.
Build/CTest/ASAN/pluginval/DAW/performance are N/A: no product code, DSP or build configuration changed.
No commit, push, PR creation, merge or issue mutation performed.


### Plot extension provenance

2026-09-16 reference_intake --plots reused analyzer write_plots, with rc_context and
discovered CJK fonts; corrected a Chinese-title glyph warning without changing metrics.
Four finite records and twelve PNGs; four waveforms/four spectrograms inspected, PSD
not used. Seven rejection cases passed: missing analysis, unassigned ID, duplicate,
unknown, seven-ID batch, outside ignored output, non-JSON output. Full command remains
in ROUND_01. No audio changed; one-file-at-a-time memory limitation retained.
HI-06 consulted Ando et al.2009 (PMC2731495) to distinguish frequency/damping from
translation; the coupled-bubbles PDF fetch then failed. This was not mapping adoption.
The subsequent HI-07 engineering handoff and Revision B supersede the old Time proposal.

## Revision B validation and review record — 2026-09-16

Scope: documentation changes after synchronization commit `04fb8ab`; six existing Markdown files only.
The older brief/reference-intake work predates Revision B; no executable is changed by this revision.
GitHub Issue #17 body synchronized via `gh issue edit 17 --body-file`; read-back matches the prepared body
line-for-line after newline normalization. Issue remains OPEN; unchecked acceptance boxes remain unchecked.

- Contract Review: approved PR #35 / main fc20370, Parameters, Coding Plan Revision B gate, Perceptual
  Contract, Testing, Proposed ADR-0006, collaboration and governance requirements checked.
- Implementation: four macro perceptual/UX clauses, Decay history reconciliation, listening scorecard and
  downstream engineering questions; update six existing docs and Issue #17, no new production abstraction.
- Functional Validation: Markdown links and portability checks PASS; remote issue read-back PASS.
- Code Quality Review: independent read-only Codex review task returned PASS / READY FOR DOCUMENT HANDOFF,
  no P0/P1/P2 findings. Reviewer task id `01a0a90d-89fd-7801-8464-5c21d44595f5`; reviewed the six-file working
  tree after 04fb8ab and live Issue #17. This is an independent agent document review, NOT a formal GitHub
  APPROVE, real second-developer review, Engineering Lead sign-off or human perceptual acceptance.
- Comment & Documentation Pass: optional P3 noted an isolated historical “future Time” phrase in Fluid text;
  clarified its historical status and explicit section-5 precedence. Prior HI records remain intact.
- Final Validation: `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
  `git diff 04fb8ab --check` and six-file/production-boundary assertions PASS. Both author and reviewer ran
  the first three checks independently before the final P3 clarification; author reran after clarification.

Documentation Review: changed canonical brief, LISTENING_LOG, task_plan, experiments README,
PERCEPTUAL_CONTRACT candidate-status note and PROJECT_STATUS draft fact. Reviewed without updates:
PARAMETERS, CODING_PLAN, TESTING and Proposed ADR-0006 already supply the applicable four-macro/gate contract;
DEVELOPER_SOUND_TOOLS and MODULE_INDEX retain current runtime/Planned production distinctions;
COLLABORATION_ROLES and DOCUMENT_GOVERNANCE preserve real independent acceptance requirements.
Consistency PASS for Issue/brief/plan and candidate versus implemented/accepted status. No Host/state,
production algorithm, routing, realtime, formal performance or release contract change; impact N/A.

NOT RUN: Debug/Release/ASAN builds, CTest, render/property/performance, pluginval/DAW, actual reference
listening/calibration and Developer usability. Documentation-only revision supplies no runtime evidence.
No push, merge to main, Issue closure or production acceptance performed. Branch-local merge of approved
main for preparation is separate from a future reviewed merge of this brief into main.

## Human review recording — 2026-09-17

Source: user supplied feedback for the four Round 01 references, explicitly passed the brief Human Review,
and requested pushing the review results. Reviewed product revision: b61653399a5104efaaf441bfe8add5455264e089.
This update records those statements and synchronizes current-status summaries; it does not redesign macros.
Original Agent first-pass and HI records remain historical. Device/level/time windows and engineering findings
are NOT PROVIDED. User's earlier willingness to perform engineering review is not a completed decision.

Contract Review -> Implementation -> Functional Validation -> Code Quality Review -> Comment & Documentation
Pass -> Final Validation: reviewed the product/engineering gate distinction; preserved user statements in
LISTENING_LOG, updated brief and related index/status/plan; checked no invented mode/classification or Ice/DSP
adoption; validated Markdown links, portability and diff. Results are recorded by the push follow-up on Issue #17.
Documentation changes: LISTENING_LOG, brief, reference index, experiments README, PERCEPTUAL_CONTRACT status
note, PROJECT_STATUS and this task plan. Reviewed without update: PARAMETERS, CODING_PLAN, TESTING,
Proposed ADR-0006, COLLABORATION_ROLES and DOCUMENT_GOVERNANCE; existing scope and independent gates unchanged.
Runtime/build/DAW/CPU/automated audio validation NOT RUN (documentation/evidence recording only).
Next action: review the pushed recording revision, supply actual Engineering feasibility findings/decision,
and satisfy independent acceptance before Issue #17 closure or formal EXP-W-002. No merge/closure authorized here.

## Closeout execution — 2026-09-18

This entry supersedes the historical pending states above. User reports Engineering Lead oral acceptance
and explicitly directs completion without another confirmation. See brief sections 11–12 for the exact
attribution, reviewed-revision limits and remaining-evidence disposition. No formal GitHub APPROVE is claimed.
Existing engineering preliminary PASS: a5a0d99, Issue #17 comment 5701188940; Human ACCEPT: b616533.

- Contract Review: Coding Plan Revision B gate, Parameters, Perceptual Contract, Testing,
  collaboration/governance/workflow, Proposed ADR-0006 and applicable architecture/implementation boundaries.
  No new product semantics, algorithm choice or Joint Gate trigger; existing macro controls remain candidates.
- Implementation: record supplied acceptance, backfill existing engineering review, resolve checklist ownership,
  synchronize status/entry points and index the existing offline intake helper. No production or helper code change.
- Functional Validation: Markdown links, portability and both scanner regression suites PASS. Existing helper
  additionally passes 9 synthetic CLI cases: metadata; analysis/plots; duplicate, unknown and >6 IDs;
  unassigned-reference analysis; plots without analysis; outside output; non-JSON output. Synthetic files are
  local ignored artifacts, not references or listening evidence. Default Python lacked soundfile; the existing
  developer DSP virtual environment supplied requirements-dsp.txt dependencies without a repository change.
- Code Quality Review: a separate post-validation read of the complete branch's 96-line helper checked path
  containment, explicit batch/development guards, analyzer reuse, scoped plotting configuration and no runtime
  dependency on production targets. No blocking finding; original reference audio and previous measurements
  were not reproduced. This is agent self-review, distinct from the earlier engineering-side review.
- Comment & Documentation Pass: preserve historical Human Intent and analysis; make current versus historical
  status explicit; separate accepted intent, user-reported oral evidence, formal review and unperformed listening.
- Final Validation: links/portability/scanner tests and diff checks PASS; product sections 3–5 unchanged versus
  a5a0d99. Repository/PR completion and actual CI results are recorded on Issue #17 and its closing PR.

Actual commands: `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_markdown_links.py`, `python tools/test_check_portability.py`, `git diff --check`;
developer DSP Python ran ignored `build/exp-w-001-closeout/smoke_intake.py` (9/9 PASS).

Documentation Review — changed for closeout: brief, LISTENING_LOG, REFERENCE_INDEX.md, task_plan, notes,
experiments README, PERCEPTUAL_CONTRACT status, DEVELOPER_SOUND_TOOLS status, PROJECT_STATUS and MODULE_INDEX.
Reviewed without updates: CODING_PLAN/PARAMETERS/TESTING/Proposed ADR-0006 (gates and candidate contracts unchanged),
Architecture/CORE_IMPLEMENTATION_GUIDE/src DSP and UI READMEs (no production/module behavior change),
AGENTS/CODE_STANDARDS/DOCUMENT_GOVERNANCE/GITHUB_WORKFLOW/COLLABORATION_ROLES (no governance/ownership change;
AGENTS' conditional planned-until-accepted rule remains valid). Reference CSV and ROUND_01 preserve historical evidence.
Consistency: PASS — accepted definition versus pending DSP/tool readiness; evidence sources and limitations;
Issue/brief Revision B scope; module index/helper dependency; no Host/state/routing/realtime/latency/random,
performance-budget or release impact (N/A). No new abstraction or framework introduced.

NOT RUN locally: C++ Debug/Release/ASAN, CTest, DSP/property/render/performance, pluginval/DAW,
new human or agent listening, license verification, Developer workflow usability or runtime Decay validation.
Hosted CI is handled as repository PR validation; it cannot upgrade these perceptual/runtime claims.
