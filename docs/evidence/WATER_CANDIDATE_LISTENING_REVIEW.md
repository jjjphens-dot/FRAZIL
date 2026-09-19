# Water candidate listening review — independent human handoff

Status: **NOT ASSESSED by both reviewers**. Engineering preparation does not establish
RESONANT AUDIBILITY READY, accepted mapping, Joint Gate readiness or production adoption.
The user authorized testing the existing sampling pack and deferred representative musical
material. The fifth engineering pad is explicitly a fixture, not musical-pad acceptance.

Use the [current pack and reproduction guide](WATER_LISTENING_HANDOFF_V02.md#phase10-current-staged-comparison-packs),
[button guide](../DEV_UI_WATER_DEBUG_GUIDE.md) and accepted
[Water perceptual definition](../../experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md).
The latter defines the intent; it does not accept these DSP candidates.

## Independent recording

Each of the Hard/C3 and Feature/C3 pack directories contains `reviewer-1.csv` and
`reviewer-2.csv`. Each has120 rows: five inputs x two characters x three macros x four
stage/evidence combinations. Sound Lead and collaborator take separate files, fill their name,
date, device/output listening level and observations independently, and discuss only after both
records are saved. Keep the original fixed-source and matched-support judgments separate.
Both CSV files initially contain NOT ASSESSED and blank scores/reasons. Generation refuses to
overwrite an existing form. A blank field is missing evidence, never a zero score or agreement.

Every row already identifies source role, candidate profile, model, macro0/.5/1, other macros.5,
seed42, Protect OFF, monitor/E Trim/output setting and the three relative WAV paths. Verify the
profile in `report.json`; record any changed listening setting in the row. Record timestamps and
failure tags, not just a preference. Do not pool Fluid/Resonant or rank them as high/low quality.

## Three stages

1. **A — audibility:** Water Only, Focus18, output-18. Record clearly audible / barely audible /
   inaudible; leave quality scores blank here. Focus36 is an extreme diagnostic fallback and must
   be recorded if used. If low/high remain indistinguishable, record REVISE with reason
   REVISE MECHANISM; do not increase boost or proceed as though semantics passed.
2. **B — semantics:** only after the mechanism is audible. Compare0/.5/1 at fixed source level,
   then separately inspect the RMS-matched support triplet. Size: Fine/Bright to Large/Deep.
   Fluid Motion: Calm to Active/Flowing; Resonant Motion: Stable to subtly more active.
   Decay: Tight to Lingering, without being primarily loud to quiet. Record perceived direction
   and separability. RMS support is not LUFS/perceptual loudness matching and cannot establish
   source/transient preservation. Fixed Motion with changing Decay should not create more events.
3. **C — context:** Reference E0, Full x+E, output-18. Compare `Source-output-18.wav` with the
   three Full-Reference files. Assess source recognizability, rhythm, major attack timing, pitch/
   harmony, bass weight and masking. Strong material transformation may alter attack shape, but
   must preserve recognizable source and performance. Audible liquid events are allowed when
   tied to the performance; detached repeated Foley is not. Use full ABD for Fluid semantics;
   component Solo can diagnose a failure but cannot pass the full mixture.

Use ACCEPT / REVISE / REJECT / NOT ASSESSED only in the decision column. Do not turn objective
tail/centroid/event/difference measurements into perceptual decisions. Both reviewers must
independently confirm Motion direction; long tails alone cannot pass Decay. Missing representative
musical material must remain explicit even if the sample-pack comparison is useful.

## Rubric anchors for this review

Use the five dimensions required by [TESTING](../TESTING.md), with2/4 as intermediate judgments.
These working listening anchors do not replace the accepted definition or create numerical DSP
acceptance thresholds. Leave a dimension blank when the current stage cannot assess it.

| Dimension | 1 | 3 | 5 |
| --- | --- | --- | --- |
| Water Identity | No clear Water material | Clear on some inputs, inconsistent elsewhere | Strong, stable Water identity with recognizable source |
| Input Recognizability | Source/performance difficult to follow | Recognizable with material losses | Source, rhythm and musical relations remain clear |
| Motion / Fluidity | Incoherent or generic modulation | Some intended activity, inconsistent | Coherent mode-appropriate motion tied to input |
| Musical Usefulness | Cannot use without undermining source | Useful in limited contexts | Consistently useful across evaluated contexts |
| Artifact Severity, lower is better | None or negligible | Noticeable and sometimes distracting | Dominant/disruptive artifacts |

No aggregate score, automatic winner or Fluid-versus-Resonant ranking is allowed. Explain every
decision, including artifact severity. For stillness at Motion0, do not demand Fluid-like activity
from Resonant; the intended macro direction and both mode identities matter.

Failure vocabulary: CHORUS-LIKE, FLANGER-LIKE, PERIODIC-LFO, SEASICK-PITCH, METALLIC, TONAL-LOCK,
EXCESS-RINGING, DETACHED-FOLEY, RANDOM-DISTRACTION, TRANSIENT-SMEAR, MUDDY, OVER-BRIGHT,
SOURCE-LOSS, TOO-SUBTLE, GENERIC-FX, UNNATURAL-MOTION, OVER-PROCESSED.

## Decision handoff

| Required human result | Current state | Follow-up |
| --- | --- | --- |
| Two independent StageA/B/C records | NOT ASSESSED | Fill separate CSVs before discussion |
| Fluid Size/Motion/Decay | NOT ASSESSED | Full ABD; separate fixed and matched findings |
| Resonant Size/Motion/Decay | NOT ASSESSED | Retain weak-output and driven-source Decay-level risks |
| Representative musical pad/context | NOT ASSESSED, user deferred | Replace fixture with representative input later |
| Baseline direction for both characters | NOT ASSESSED | Record ACCEPT/REVISE/REJECT and exact candidate identity |

Phase12 Protect listening is **NOT RUN / trigger pending**. Only after both Fluid and Resonant
baseline acceptance/revision records exist, re-evaluate D0/D1, F1/F2/F3 and Depth/Attack/Release
on the selected new residual. C currently permits Whole/F1 only; do not imply unsupported F2/F3
operation for C. Preserve the unprotected comparison, record component cancellation/topology
changes, and do not relabel old Protect conclusions as current. Existing automated Protect tests
are numerical regressions, not musical acceptance. Conditional Flow D1/D2 or Droplet B2 work
also requires the plan's actual human failure finding; neither has been implemented speculatively.
