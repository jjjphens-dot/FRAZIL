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
| Research session export | v5, 28 explicit engineering targets |
| Production contract | Nine Host parameters, `schemaVersion=1`, unchanged |

The [mapping specification](../../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md)
and [debug control guide](../DEV_UI_WATER_DEBUG_GUIDE.md) describe current controls. v1 and
v2/v0.1 imports retain raw engineering values and mark macros CUSTOM; adopting the new mapping
requires explicit Adopt/Return. Session version and mapping revision are separate identities.

Motion=0 schedules no new Bubble or Droplet events; active tails may decay and Flow continues.
It is **event-silent, not an all-output mute**. C0 modal normalization (`b=1-r`) is unchanged.
Resonant Motion audibility and Decay level/persistence tradeoffs remain unresolved. The preview default has no bounded
excitation conditioner, C3 normalization or new temporal-motion candidate. Explicit Engineering comparison options now make Hard/Feature+C3+structured Motion available
after Apply; their presence is not adoption. Carrier comparisons and actual-driver monitors are tracked in
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
renders and `report.json`. Those baseline files are fixed-source comparisons. The current generator also emits separately
labelled attenuation-only RMS support; never pool the two interpretations.
Generated WAVs, source audio, reports and screenshots remain ignored locally.

## Listening and reference boundaries

Two independent reviewers should record separate ACCEPT / REVISE / REJECT / NOT ASSESSED
decisions with input, character, macro settings and monitoring conditions. Current status for
both reviewers is **NOT ASSESSED** for the next-stage candidates; historical review must not be
relabelled as acceptance of an unimplemented candidate.

1. Audibility: use Water Only / Focus18 first; Focus36 is an extreme diagnostic comparison.
2. Semantics: compare 0/.5/1 at fixed source level. Separate RMS-matched comparisons support character/preference judgments and cannot prove source preservation.
3. Preservation: compare Reference and Full `x+E` for recognizable source, rhythm, bass and
   masking. Assess Water identity, input recognizability, Motion fluidity, musical usefulness
   and artifact severity separately; do not replace human judgment with an aggregate score.

Reference recordings and collaborator feedback remain in the
[Protect listening intake](../planning/WATER_PROTECT_EXECUTION.md). The interim v0.2 report has
no reference render entries. Historical reference-file observations do not establish acceptance
of processing musical inputs. Protect remains OFF for the next baseline comparisons.

Missing: a representative musical sustained pad (deferred by the user), bounded-C
adoption and human review, reliable Resonant macro audibility, Fluid balance review, two independent
human decisions and subsequent Protect re-evaluation. None is implied by the 72-case regression.

The [v0.1 historical appendix](#historical-v01-handoff) and
[old UI execution record](WATER_LISTENING_UI_EXECUTION.md) preserve v0.1/session-v2 history,
including the former 30/s minimum and earlier measured levels; they are not current instructions.

C3 now has [separate bounded engineering evidence](../../experiments/water/EXP-W-RN-001.md)
and an explicit offline renderer option. This does not change the raw/C0 preview default or count
as listening acceptance. See the [phase ledger](WATER_DSP_LISTENING_EXECUTION.md) for continuation.

Phase 3 support files are locally available in `build/listening-ui/decay-c3-v1`: five input folders,
three Decay values each, `fixed-Full-Reference-output-18`,
`fixed-WaterOnly-Focus18-output-18`, actual excitation and separate `RMS-matched-support-WaterOnly`.
They use explicit hard/C3, seed 42 and Protect OFF; the preview default remains raw/C0.
Matching is source-window RMS attenuation after rendering, not LUFS or an adopted normalizer.
Sub Bass and the generated engineering pad lose about 11.3/12.0 dB of source-window C level
from short to long. Tail extension alone therefore does not establish Decay audibility.
Both human reviews are NOT ASSESSED. The initial resource refusal was cleared. One subsequent
Debug access violation was not reproduced in ten isolated runs or three full-suite repetitions;
ASAN passed. This is an unresolved observation, not a claimed fix; see the execution ledger.
The user authorized the current sample pack and deferred representative musical-input testing.

Further engineering comparison packs (no adopted defaults):

- [R-M1 Motion](../../experiments/water/EXP-W-RM-001.md): `build/listening-ui/motion-rm1-v1`,
  60 hard/feature, legacy/structured, low/mid/high combinations with fixed and RMS support files.
- [Fluid balance](../../experiments/water/EXP-W-LCF-001.md): `build/listening-ui/fluid-balance-v1`,
  45 LC-F0/1/2 and Size combinations. Compare complete ABD before using component solos to diagnose.

Neither pack supplies human approval. Quiet Resonant outputs, driven-source Decay level loss,
possible Fluid masking and the unreproduced renderer failures remain review limitations.

Session v4 adds the independent Engineering Onset probability control (default 1); older sessions
fill 1 without remapping. v0.2 Motion does not automatically adopt the separate continuous-activity
curve. See the current debug guide for explicit probability and exporter candidate use.

## Phase10 current staged comparison packs

Two separately labelled packs use five inputs x eighteen cases each. Both retain original source
samples, v0.2 macro targets, LC-F0 calibration, fixed seed42, Protect OFF and three seconds of tail.
The first four inputs are the supplied sampling pack. The fifth is the six-second generated
engineering pad from EXP-W-RX-001, explicitly labelled in every relevant report row. The user
approved use of the current sampling pack and will supply representative musical listening later.
Neither pack selects a winner or changes the preview default.

| Local directory | Explicit Resonant policy | Droplet policy |
| --- | --- | --- |
| `build/listening-ui/handoff-hard-c3-v1` | hard / C3 / structured | exporter candidate `clamp(4*m*m,0,1)` |
| `build/listening-ui/handoff-feature-c3-v1` | feature / C3 / structured | same explicit candidate |

The continuous Droplet policy is an **offline candidate**, not silently applied by the UI macro.
To reproduce an exact case in the GUI, Import Module Config from the case JSON, then Apply/Play;
the imported engineering values remain explicit/CUSTOM. Subsequent UI macro motion does not
recalculate probability. For manual isolation, edit Onset probability explicitly. Hard/Feature C
comparison can instead be selected in Engineering, followed by Apply/Play. Component Solo is
for diagnosis, not evidence of full-ABD macro semantics.

### File interpretation and listening stages

| File suffix | Equation after static monitor settling | Use |
| --- | --- | --- |
| `Source-output-18.wav` | source x at -18 dB | source reference |
| `*-E.wav` | E | Stage A Water Only / Focus18 / output -18 |
| `*-WaterOnly-Focus36-output-18.wav` | E at +18 dB | extreme Stage A diagnostic only |
| `*-Full-Focus18-output-18.wav` | x at -18 dB + E | boosted diagnostic context |
| `*-Full-Reference-output-18.wav` | (x+E) at -18 dB | Stage C preservation/context |
| `*-RMSmatched-WaterOnly.wav` | E times recorded attenuation | separate Stage B support only |
| `c-*-excitation.wav` | actual common Modal driver | engineering inspection; no implied monitor attenuation |

Stage A asks clearly/barely/inaudible only. Do not judge naturalness or quality. If low/high cannot
be distinguished at Focus36, record REVISE MECHANISM; do not increase boost. Stage B compares
0/.5/1 with the other macros fixed at .5: Size Fine/Bright to Large/Deep; Fluid Motion Calm to
Active/Flowing, Resonant Stable to subtly more active; Decay Tight to Lingering. Stage C restores
Reference/Full and compares the source, rhythm, major transient timing, bass weight and masking.

Matched support uses minimum source-window residual RMS within each input/model/macro triplet;
only post-render attenuation is applied. Gains and target are in `extreme_comparisons`. This is
RMS matching, **not LUFS or perceptual loudness equality**. A zero target is flagged unassessable.
It can support character/direction/preference, never source or transient preservation. Keep
fixed-source and matched-support decisions separate. No hidden limiter/normalizer modifies DSP.

Reports include actual C excitation peak/RMS, first100ms C RMS, source-window C RMS, tail energy/
centroid, low/high Motion difference RMS, and spectral-energy fractions in0-250/250-1000/
1000-4000/4000-Nyquist Hz. Early100ms is relative to file start, not a detected onset; these are
engineering proxies. All output audio remains local/ignored; do not commit the sampling pack.

### Portable regeneration

After the documented Release configure/safe build, use Python with numpy/soundfile. Set
`FRAZIL_SAMPLE_ROOT` to this machine's library root and `FRAZIL_ENGINEERING_PAD` to the local
engineering fixture (or replace it with a real pad and omit the engineering label). From repo root:

```powershell
$research = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
$inputArgs = @()
Get-ChildItem "$env:FRAZIL_SAMPLE_ROOT/Sample_Input" -Filter *.wav -Recurse |
  Sort-Object FullName | ForEach-Object { $inputArgs += @('--input', $_.FullName) }
$inputArgs += @('--input', $env:FRAZIL_ENGINEERING_PAD, '--engineering-input', $env:FRAZIL_ENGINEERING_PAD)
python experiments/water/SPIKE-W-DSP-001/render/listening_handoff.py `
  --cases-executable "$research/frazil_water_research_cases_artefacts/Release/frazil_water_research_cases.exe" `
  --renderer "$research/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe" `
  @inputArgs --continuous-droplet --resonant-profile hard-c3 `
  --output build/listening-ui/handoff-hard-c3-new
```

Repeat serially with `feature-c3` and another new output directory. Omit both candidate flags
for the legacy policy. The generator refuses an existing output directory and bounds the batch.
Input1..5 order is recorded by source name in each report; use the report rather than assuming
which instrument is in a numbered folder. The current record is SubBass, Fill, Partisan, Axusr, pad.

### Phase10 engineering observations (not listening decisions)

Both packs passed90/90 finite/repeat/128-vs-257/channel-isolation cases; all five Fluid Motion0
cases scheduled zero new A/B events. Decoded matched and Full-Reference equations were separately
checked for every case. Maximum float-rounding error in the latter was1.95e-8. Attenuation gains
were0..1; source-window RMS spread within every matched triplet was below1e-8 amplitude.

| Observation | Hard/C3 | Feature/C3 |
| --- | --- | --- |
| C source-window RMS across45 cases | -76.65 to -49.76 dBFS | -67.58 to -43.70 dBFS |
| C low/high Motion difference RMS across5 inputs | -97.90 to -79.35 dBFS | -86.83 to -76.20 dBFS |
| SubBass C long-minus-short source-window RMS | -11.25 dB | -10.37 dB |
| Engineering pad C long-minus-short source-window RMS | -12.03 dB | -11.83 dB |

These low levels and driven-source Decay level losses remain unresolved listening risks; neither
numerical difference nor longer tails establishes macro acceptance. Maximum Focus36 output peak
across either pack is+0.765 dBFS (Fluid case), retained as float over-range, without limiting.
No human audibility, musical usefulness, source preservation or Joint Gate readiness is claimed.


The [independent review guide](WATER_CANDIDATE_LISTENING_REVIEW.md) explains the two blank CSV
forms now generated per pack, rubric anchors, decision vocabulary and conditional Protect handoff.

## Historical v0.1 handoff

Merged from WATER_LISTENING_HANDOFF.md during A1 review remediation. Baseline4142db1,
mappingv0.1/sessionv2; superseded instructions are not current defaults. The original
command uses the same listening_handoff.py/four sources and three reference selections
as the current guide, but must be run at that historical revision for v0.1 reproduction.
Historical files: E; Full-Focus18-output-18 = x at-18dB +E;
WaterOnly-Focus36-output-18 = E at+18dB; repeat/block257/left-only checks.
Original review decisions remained PENDING; no new human result is recorded by this merge.
References/selected ranges remain in WATER_PROTECT_EXECUTION; current reviewer workflow is above.

## Objective results and candidate limitations

All 72 cases: finite output, decoded-sample repeat identity, block 128 vs 257 exact equality,
left-only input produces exactly zero right residual. All 24 low/high comparisons differ
numerically. Identical center settings from three macro families produce identical decoded output.
Source rates are retained (44.1/48 kHz); no original was trimmed or replaced for these checks.

Partisan drum loop (48 kHz stereo, 5.647 s), endpoints 0 -> 1:

| Model / macro | Observation | Interpretation limit |
|---|---|---|
| Fluid Size | Whole E spectral energy centroid 7193.1 -> 6939.7 Hz; isolated A 1332.7 -> 331.8 Hz, B 4161.4 -> 1034.1 Hz | Direction is measurable; D can mask the small combined change |
| Resonant Size | E spectral energy centroid 1035.9 -> 346.3 Hz | Energy centroid is not perceived pitch |
| Fluid Motion | A events 8 -> 148; B events 44 -> 50; Flow input delay travel 12.743 -> 237.225 samples | B is source-dependent; counts do not establish natural activity |
| Fluid Decay | Tail energy time centroid .00266 -> .12162 s; A/B counts fixed 44/54 | No event rescheduling from Decay |
| Resonant Decay | Tail energy time centroid .01523 -> .23932 s | Increased persistence also lowers output level under existing normalization |
| Resonant Motion | Low/high difference RMS -102.9 dBFS before audition boost | Numerically different, potentially too subtle for reliable listening |

Tail time centroid means energy-weighted time **after input end**, not an RT60 or accepted perceptual
decay measure. Three-second silence is sufficient for these candidate bounds; no 30-second GUI-tail
identity is claimed. The input-window Flow range/travel in `report.json` measures actual delay
trajectory; its final value after silence returns to base and is not useful Motion evidence.

| Original input | Source RMS dBFS | Fluid E/source range dB | Resonant E/source range dB |
|---|---:|---:|---:|
| Partisan loop | -29.20 | -21.76..-20.82 | -61.95..-50.13 |
| Sub Bass | -21.68 | -19.28..-18.89 | -72.02..-43.12 |
| Dunamis fill | -14.44 | -20.94..-20.40 | -59.04..-45.36 |
| Axusr razor bass | -8.41 | -21.53..-21.00 | -66.41..-48.73 |

Across the four inputs, E/source RMS ranges: Fluid -21.76..-18.89 dB; Resonant -72.02..-43.12 dB.
Partisan Resonant Motion endpoints have E RMS about -84.9/-85.0 dBFS; with Focus +36 and output -18
that is still about -66.9/-67.0 dBFS, and the low/high difference about -84.9 dBFS.
**Candidate mechanism/audibility risk:** no claim of clearly audible Resonant Motion is made.
Do not alter the requested mapping or disguise it with per-macro gain. If Sound Lead confirms
A/B/C remain inaudible at Focus +36 with a suitable output level, stop mapping tuning and open a
separate **Water Audibility DSP Remediation** task.

The largest observed Fluid Focus +36 / output -18 peak is +0.76 dBFS. Keep the initial +18 Focus
and lower monitor output before higher boosts; float offline files preserve over-range peaks.
The preview has no hidden limiter. Flow remains delayed-minus-source and may sound like chorus or
flanging; this is an unresolved candidate limitation, not accepted Water Motion. Minimum Fluid
Motion still allows a 30/s maximum A rate; HI-08 final product-minimum compliance is not claimed.
