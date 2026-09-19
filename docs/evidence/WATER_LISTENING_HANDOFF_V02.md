# Water listening handoff — current v0.2 baseline

Status: **RESEARCH ONLY; Resonant NOT FULLY LISTENING-READY**. This is the current entry point
for PR #40 listening work. It does not grant product mapping, Host/state adoption, EXP-W-002
acceptance or a human listening decision. Engineering owns implementation; Sound Lead owns
perceptual acceptance. The [next-stage phase record](WATER_DSP_LISTENING_EXECUTION.md) tracks
the bounded continuation and its stop conditions.

## Verified baseline

Reconciled on 2026-09-19 before documentation changes:

| Item | Identity / state |
| --- | --- |
| PR #40 implementation | `64f082c84041c9de588b6840ce4e797f23660680`, OPEN |
| Main | `3c95e47212a03d43ccf06a2484d8f3861a4f6b33` |
| Protect PR #37 | `1a1fb410238c274488bdd2ca0709c6831cc15859`, OPEN |
| Implementation CI | [35422887316](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35422887316), SUCCESS; historical evidence for the implementation commit, not subsequent documentation commits |
| Current mapping | `research-water-mapping-v0.2` |
| Research session export | v3, 24 explicit engineering targets |
| Production contract | Nine Host parameters, `schemaVersion=1`, unchanged |

The [mapping specification](../../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md)
and [debug control guide](../DEV_UI_WATER_DEBUG_GUIDE.md) describe current controls. v1 and
v2/v0.1 imports retain raw engineering values and mark macros CUSTOM; adopting the new mapping
requires explicit Adopt/Return. Session version and mapping revision are separate identities.

Motion=0 schedules no new Bubble or Droplet events; active tails may decay and Flow continues.
It is **event-silent, not an all-output mute**. C0 modal normalization (`b=1-r`) is unchanged.
Resonant Motion audibility and Decay level/persistence tradeoffs remain unresolved. The preview default has no bounded
excitation conditioner, C3 normalization or new temporal-motion candidate. Separate offline
carrier comparisons and an actual-driver monitor are tracked in
[EXP-W-RX-001](../../experiments/water/EXP-W-RX-001.md); they do not replace this mapping baseline.

## Existing v0.2 objective evidence

The interim report `build/listening-ui/remediation-v02-interim/report.json` was generated during
the [prior remediation](WATER_LISTENING_REMEDIATION.md); its contents were re-read in Phase 0.
This phase does not claim a new audio render or new native GUI run.

All 72 cases (four original inputs x 18 configurations) report finite output, decoded repeat
identity, exact block-128/block-257 equality and zero right residual for left-only input.
At Fluid Motion=0 all four inputs report Bubble=0 and Droplet=0 new events:

| Original input | Sample rate | Bubble / Droplet events |
| --- | --- | --- |
| ABL2_Loops_42_Partisan_BPM170.wav | 48 kHz | 0 / 0 |
| -_Sub Bass.wav | 44.1 kHz | 0 / 0 |
| ABL2_Fill_32_Dunamis_BPM191.wav | 48 kHz | 0 / 0 |
| Axusr_razor Bass 01 C.wav | 44.1 kHz | 0 / 0 |

Protect was OFF, seed 42, tail 3 s, calibration A/B/D/C=.26/.24/.06/.30. The maximum observed
Water Only / Focus36 / output -18 peak was +0.76475 dBFS. Float output preserves this over-range;
the GUI warning does not limit or normalize audio. Begin with Focus18 and monitor output -18.

The C0/C1/C2 study retains the finite-float counterexamples for unmodified C1/C2. Those candidates
were not adopted. The [study record](WATER_LISTENING_REMEDIATION.md#phase-3-normalization-experiment-stop-a-review)
contains the recurrence, assumptions and results. This failure does not rule out a separately
proved bounded-excitation design. It does prohibit presenting the current C0 pack as corrected C.

## Reproduce the current interim pack

Use the recorded implementation revision for baseline reproduction. Configure the selected
preset with `FRAZIL_BUILD_WATER_EXPERIMENT=ON` and `FRAZIL_BUILD_WATER_PREVIEW=ON`, then use
`python tools/build_safe.py --preset windows-release`. See [environment setup](../ENVIRONMENT.md)
for compiler discovery. Set `FRAZIL_SAMPLE_ROOT` locally to the supplied library root; no personal
path belongs in tracked files. Run from the repository root, choosing a new output directory:

```powershell
$research = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
python experiments/water/SPIKE-W-DSP-001/render/listening_handoff.py `
  --cases-executable "$research/frazil_water_research_cases_artefacts/Release/frazil_water_research_cases.exe" `
  --renderer "$research/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/ABL2_Loops_42_Partisan_BPM170.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/-_Sub Bass.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/ABL2_Fill_32_Dunamis_BPM191.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/Axusr_razor Bass 01 C.wav" `
  --output build/listening-ui/v02-baseline-reproduction
```

The existing driver requires numpy/soundfile and emits mapped case JSON, `*-E.wav`,
`*-Full-Focus18-output-18.wav`, `*-WaterOnly-Focus36-output-18.wav`, repeat/partition/isolation
renders and `report.json`. These are fixed-source comparisons, **not RMS-matched evidence**.
Generated WAVs, source audio, reports and screenshots remain ignored locally.

## Listening and reference boundaries

Two independent reviewers should record separate ACCEPT / REVISE / REJECT / NOT ASSESSED
decisions with input, character, macro settings and monitoring conditions. Current status for
both reviewers is **NOT ASSESSED** for the next-stage candidates; historical review must not be
relabelled as acceptance of an unimplemented candidate.

1. Audibility: use Water Only / Focus18 first; Focus36 is an extreme diagnostic comparison.
2. Semantics: compare 0/.5/1 at fixed source level. Separate RMS-matched comparisons, when
   generated, support character/preference judgments and cannot prove source preservation.
3. Preservation: compare Reference and Full `x+E` for recognizable source, rhythm, bass and
   masking. Assess Water identity, input recognizability, Motion fluidity, musical usefulness
   and artifact severity separately; do not replace human judgment with an aggregate score.

Reference recordings and collaborator feedback remain in the
[Protect listening intake](../planning/WATER_PROTECT_EXECUTION.md). The interim v0.2 report has
no reference render entries. Historical reference-file observations do not establish acceptance
of processing musical inputs. Protect remains OFF for the next baseline comparisons.

Missing: a representative sustained pad and the requested five-input/90-case pack, bounded-C
proof and measurements, reliable Resonant macro audibility, Fluid balance review, two independent
human decisions and subsequent Protect re-evaluation. None is implied by the 72-case regression.

The [old handoff](WATER_LISTENING_HANDOFF.md) and
[old UI execution record](WATER_LISTENING_UI_EXECUTION.md) preserve v0.1/session-v2 history,
including the former 30/s minimum and earlier measured levels; they are not current instructions.
