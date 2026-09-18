# Water research mapping: Sound Lead handoff

Status: engineering comparison pack generated; **human mapping/audibility decision PENDING**.
This is research tooling, not product Water adoption, EXP-W-002 acceptance, Host validation or a
claim that every macro is clearly audible. Implementation: Engineering Lead; acceptance: Sound Lead.
Phase baseline `4142db1`; formulas and independent calibration are defined in
[RESEARCH_MAPPING](../../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md).

## Reproduce the local pack

Build both opt-in research targets using the repository safe wrapper. The new
`frazil_water_research_cases` console target exports 18 configurations from the **same C++ mapper,
destination adapter and calibration used by the UI**. It performs no audio processing. The Python
handoff driver calls the existing renderer; it contains no copied macro curves or synthesis code.

From the repository root, with `FRAZIL_SAMPLE_ROOT` set locally to the supplied library root:

```powershell
$research = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
python experiments/water/SPIKE-W-DSP-001/render/listening_handoff.py `
  --cases-executable "$research/frazil_water_research_cases_artefacts/Release/frazil_water_research_cases.exe" `
  --renderer "$research/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/ABL2_Loops_42_Partisan_BPM170.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/-_Sub Bass.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/ABL2_Fill_32_Dunamis_BPM191.wav" `
  --input "$env:FRAZIL_SAMPLE_ROOT/Sample_Input/Axusr_razor Bass 01 C.wav" `
  --reference "$env:FRAZIL_SAMPLE_ROOT/Sample_Packs/ForestStream_BW.53693.wav" `
  --reference "$env:FRAZIL_SAMPLE_ROOT/Sample_Packs/StreamWater_S08AM.60.wav" `
  --reference "$env:FRAZIL_SAMPLE_ROOT/Sample_Packs/SubmergeContainer_SFXB.4860.wav" `
  --output build/listening-ui/handoff-new
```

Requires existing Python development dependencies `numpy` and `soundfile`. Output must be a new
directory. The bounded batch allows 1–6 explicit inputs, at most 120 s each and 8 million total
input channel-samples. No resampling, normalization, content hashes or source modification.
Generated WAV/JSON files stay ignored locally; no supplied or derived audio is committed/uploaded.

Each input folder contains Fluid (`abd`) and Resonant (`c`), each macro at 0/.5/1 with the other two
at .5. Protect is OFF; seed 42; tail 3 s; calibration A/B/D/C=.26/.24/.06/.30. Files:

- `*-E.wav`: original residual; also the settled Water Only / Focus +18 / output -18 equation.
- `*-Full-Focus18-output-18.wav`: source at -18 dB plus E.
- `*-WaterOnly-Focus36-output-18.wav`: E at net +18 dB; no clipping/limiter is hidden in float WAV.
- `*-repeat.wav`, `*-block257.wav`, `*-left.wav`: repeat, partition and isolation verification.
- Case JSON and `mapped-cases.json`: exact renderer targets and mapping/calibration revision.
- `report.json`: sanitized filenames, measurements, event/trajectory observations and test results.

Offline monitor files use static gain equations; GUI transitions are separately tested 10 ms ramps.
These are fixed-source comparisons, **not loudness-matched preference evidence**. The first input
also receives isolated A/B Size/Decay endpoints to reveal components masked in combined Fluid E.

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

## Reference and human review sheet

Use the latest [Sound Lead intake](../planning/WATER_PROTECT_EXECUTION.md), keeping reference
recordings separate from processed musical input:

| Reference | Selected range | User-specified listening purpose |
|---|---|---|
| ForestStream_BW.53693.wav | Whole 8.75 s | Clear/high-frequency/transparent water; exclude birdsong from identity |
| StreamWater_S08AM.60.wav | 0–30 s of 122.453 s | Spring/stream clarity at somewhat lower perceived frequency; avoid noise-like masking |
| SubmergeContainer_SFXB.4860.wav | Whole 13.989 s | Large Size, medium-high Motion, low-frequency bubbles and prominent transients |

Reference intake checked readable finite excerpts and recorded rate/channels/RMS locally. These
measurements do not classify the references or imply that the synthesized result matches them.

1. Start Protect OFF, output -18, Water Only + Focus18. Compare 0/.5/1 one macro at a time.
2. Check Size scale, Motion activity/movement and Decay persistence separately in both models.
3. Use Reference to check actual E/source balance; Full must retain recognizable Partisan rhythm.
4. Inspect isolated A/B and Engineering C where a combined result hides the effect. Increase E
   trim only as an explicit monitor setting, recording it with the decision.
5. Capture A/B or export v2 Session to preserve targets, source descriptors and listening context.
   The [button guide](../DEV_UI_WATER_DEBUG_GUIDE.md) explains Auto Audition and CUSTOM ownership.

| Review item | Decision | Evidence to record |
|---|---|---|
| Fluid Size / Motion / Decay | PENDING | ACCEPT / REVISE / REJECT per macro; input, endpoints, monitor settings, reason |
| Resonant Size / Motion / Decay | PENDING | Same; explicitly assess low-level Motion difference |
| Water layer audibility | PENDING | Water Only +18/+36, output level and whether components remain inaudible |
| Rhythm/source preservation | PENDING | Full mode Partisan; distinguish fixed-source preservation from preference |
| Reference character and Flow limitation | PENDING | Clear water vs noise masking/chorus/flanger; no automatic ranking |

No human listening, independent approval, pluginval/DAW, other-DPI or multi-monitor validation is
claimed by this handoff. See [phase evidence](WATER_LISTENING_UI_EXECUTION.md) for native GUI and
Debug/Release/ASAN results. Engineering handoff completion does not close perceptual acceptance.
