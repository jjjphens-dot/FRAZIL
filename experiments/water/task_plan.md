# EXP-W-001 execution plan — latest revision

> Active basis: latest user-supplied Water Dual-Mode Perceptual/Product Brief plan (2026-09-16).
> Existing branch codex/exp-w-001-water-identity; earlier uncommitted intake work retained.
> Implementation DRI: Sound & Host Lead. Engineering Lead: feasibility review after Human Review.

## Current priority — v0.1 engineering handoff

User requested an initial deliverable now because engineering is waiting. Pause supplementary questions.
Six primary paragraph groups have answers; only Q16/Q17 bass follow-ups are unanswered, and Decay/Size-time
definition remains with Engineering Lead. These do not block a draft handoff.
Canonical brief now opens with v0.1 engineering summary, evidence IDs, open-item impact and concrete review
actions. No parallel contract created, no message dispatched, no implied whole-contract ACCEPT.
Next step is engineer review of this draft and return of Decay/findings; ask only questions needed to resolve
those findings or actual listening ambiguity. Accepted-instance prerequisite for candidate DSP remains intact.

## Mission / scope

Human Water Intent + references → Agent first-pass → Human calibration → accepted canonical brief.
No DSP implementation, candidate selection, sweep, ablation, production parameters/state or ADR adoption.
Canonical deliverable: [EXP-W-001_PERCEPTUAL_BRIEF.md](EXP-W-001_PERCEPTUAL_BRIEF.md).
Do not create parallel EXP-W-001/ or empty downstream stage folders.

## Latest-plan changes

The old workflow required humans to classify references before Agent interpretation and centered on
candidate scores/anonymous comparisons. It is superseded for this definition stage.
Now preserve AGENT INITIAL before HUMAN CALIBRATED; do not overwrite initial judgments.
Use OBSERVATION / INTERPRETATION / ENGINEERING QUESTION with confidence and rationale.
Numerical candidate gates, fixed two-round vocabulary freeze, two-person blind listening packs,
Golden/holdout and candidate evidence are not EXP-W-001 Exit prerequisites.
Earlier intake remains valid evidence; it does not become completed listening or calibration.

## Round sequence

| Round | One semantic question | Current status |
|---|---|---|
| 0 | Human Intent Seed: input-driven material, equal modes, moving Fluid, stable Resonant, preservation, no Foley | Documented from explicit latest intent |
| 1 | What might belong to Common Water across 2 pure + 2 musical references? | Four-reference numerical/visual first-pass preserved; auditory layer unavailable; human calibration pending |
| 2 | Fluid responsibility | Pending Round 1 calibration; choose 4–6 informative references |
| 3 | Resonant responsibility | Pending calibration; no C implementation |
| 4 | Common versus mode-specific versus mixed/uncertain | Pending; no winner/distance score |
| 5 | Negative / anti-example and reject meaning | Pending actual examples; no negative DSP generation |
| 6 | Size: larger/deeper material scale; not loudness/Amount/Motion | Draft intent; calibration pending |
| 7 | Motion: temporal activity; common direction, subtler Resonant | Draft intent; calibration pending |

One primary semantic question and at most 4–6 references per round. Do not analyze the full library
in one semantic batch. Choose material using the current round's gaps; do not assign perceptual labels
from filenames. Mixed/Ambiguous/Uncertain are legitimate. Retain every initial and calibrated version.
Only promote a semantic dimension after multiple-reference support and repeated human retention.

## Current batch phases

- [x] Contract Review: latest plan versus existing brief, framework/template and current repository stage.
- [x] Implementation: four-reference first-pass record, calibration form and scoped brief revisions.
- [x] Functional Validation: existing analyzer generates measurements and waveform/PSD/spectrogram for four IDs.
- [x] Code Quality Review: bounded plots option, existing analyzer reuse, path/development guards.
- [x] Comment & Documentation Pass: active plan, round evidence and candidate/acceptance status.
- [x] Final Validation: boundary tests, output integrity, links, portability and diff.
- [ ] Sound Lead calibration and contract acceptance.
- [ ] Independent Engineering Lead feasibility review.

## Capability and safe continuation

Paragraph-based user calibration: ① preservation/fusion, ② Fluid, ③ Resonant, ④ Size, ⑤ Motion,
⑥ use/reject boundaries. HI-02 records strong-material allowance, transient priority and audible yet
performance-related liquid events. Next clarify transient timing/shape/secondary-attack tolerance within ①.
Keep Amount's existing Serial dry/wet contract separate from the user's strong-character goal.

HI-03 resolves paragraph ① wording: limited attack softening, permitted liquid tail, bounded secondary
events with desirable low/mid impact, and strong-character attack changes while retaining main onsets/rhythm.
Proceed to paragraph ② Fluid questions; no requirement to reconfirm paragraph ①. User-requested future Time
tail-length control is recorded as a requirement for later scope/parameter/tail review, not current adoption.

HI-04 completes the current paragraph ② intent questions: pad = continuous flow with discernible liquid events;
sustained input retains evolution, calm/activity left to Motion/user automation. Keep transient limits and
candidate-control boundaries. Next question block: paragraph ③ Resonant material quality and intrusive tonality.

HI-05 records paragraph ③: both integrated material and audible resonance; transient pitch and stable
non-masking resonance permitted; avoid metal/bell edges, tolerate only a small residual without promoting
it as a target. Next question block: paragraph ④ Size perceptual direction and its separation from Time.

HI-06: Size direction accepted; strict Time-to-overall-tail requirement recorded. Size-to-individual-event
duration coupling remains undecided. HI-07 supersedes the earlier plan to resolve this before proceeding:
the user has handed the definition to Engineering Lead; future formal name is Decay. Mark PENDING ENGINEERING
DEFINITION and resume after the engineering definition returns. No numerical/Host/tail contract adopted.
Proceed now to paragraph ⑤ Motion without deciding Size/time coupling or the proposed hard deadline.

HI-08 records minimum Motion behavior concretely. HI-09 resolves both follow-up questions: original Q13
speed > event frequency > irregularity > depth, with the first two most important; minimum behavior applies
to both modes. Next: paragraph ⑥ musical use and reject boundaries. No numeric mapping chosen.
Decay discussion remains deferred to engineering.

HI-10 completes initial paragraph ⑥ input: bass > drums/percussion > pad/atmosphere > creative piano/guitar
> special-purpose vocal; exclude full-mix bus use from target applications. Perceptual baseline = clearly
Water without excessive strength; weak identity acceptable only at lower effect amounts. Six paragraph
groups now have initial conversational coverage, not reference acceptance. Next focus: concrete bass use
and low-end preservation; Decay and Size/time coupling remain with engineering.

User requested a complete Agent synthesis before further human calibration. OVERVIEW-01 is now inside
the canonical brief, with stable C/F/R/S/M discussion IDs, concrete musical examples and evidence boundaries.
This is an overall interpretation draft from existing intent, not another audio round or accepted contract.

HI-01 wording clarification is recorded: the user accepts performance-related response as common intent
and allows relatively stable Resonant. This clarifies Round 0 intent; it does not complete the four-reference
Round 1 calibration, auditory first-pass or final contract review. Preserve the original vague proposal and
the user's correction in LISTENING_LOG. No further approval of these two intentions is needed.

Current Agent can inspect numerical data and plots but cannot reliably directly hear the local WAVs.
Record Water Salience NOT ASSESSED and Character Uncertain rather than invent hearing.
This batch is partial first-pass, not satisfaction of the latest plan's full auditory interpretation requirement.
No semantic promotion or later semantic round until human feedback is recorded. All independent preparation
continues to a concrete four-reference handoff; human decisions are not replaced by elapsed time.

## Downstream boundary

Holdout/source-class generalization, parameter regions, candidate matching and performance are future work.
Existing six analyzed development references remain used; do not relabel them unseen.
Reference IDs and source grouping persist for later additions; 20 metadata records do not prove a full corpus.
