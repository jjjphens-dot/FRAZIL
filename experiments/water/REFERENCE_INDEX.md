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
