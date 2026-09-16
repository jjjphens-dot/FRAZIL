# EXP-W-001 Engineering Handoff

> Status: DRAFT engineering companion / Phase 1; independent review pending.<br>
> Work item: `EXP-W-001-ENG-HANDOFF`, supporting [EXP-W-001 / #17](https://github.com/jjjphens-dot/FRAZIL/issues/17).<br>
> Implementation: Engineering Lead. Sound & Host Lead confirms perceptual interpretation; Engineering Lead records feasibility review.<br>
> This is not a second Perceptual Contract, an accepted brief, or permission to begin subjective refinement.

## 1. Scope and verified baseline

This companion makes the handoff repeatable while Sound & Host Lead develops the single Water instance,
`experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md`. It supplies a clause record, evidence classes, question matrix
and existing observations. Its preparation does not close #17 or change the accepted lifecycle in
[Perceptual Contract](../../docs/PERCEPTUAL_CONTRACT.md) and [Coding Plan](../../docs/CODING_PLAN.md).

Repository inspection on 2026-09-16 used `origin/main` at `c7e68ce`:

- M1 Exit is approved; Developer Control Surface implementation is merged, with usability/offline handoff
  acceptance outstanding. See [Project Status](../../docs/PROJECT_STATUS.md).
- [SPIKE-W-DSP-001](SPIKE-W-DSP-001/README.md) contains objective A/B/D/C feasibility mechanisms, fixed-config
  ablation, render scripts and engineering evidence. It does not implement production Water or close EXP-W-002.
- The Water instance is absent from this main snapshot. The [remote draft at fcecc7d](https://github.com/jjjphens-dot/FRAZIL/blob/fcecc7d76d35353fa9ac7d9ba75f908541a00356/experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md)
  is explicitly Draft for joint Contract Review. It is useful input, not accepted authority. Refresh the exact
  brief revision and review decision before doing the real clause review.
- The existing [analyzer](../../tools/analyze_testdata.py) supplies diagnostics and plots. Generic review-pack
  assembly remains planned; the spike's smoke/corpus scripts are not a completed generic Sound Lab.

Allowed writes for this phase are this companion and its link in [experiments/README.md](../README.md).
No perceptual brief edits, DSP/config changes, new analyzer, render, Host/state changes or production work occur.
Architecture, parameters/automation, state, realtime, routing and formal performance contract impact: **N/A**.
No ownership transfer, new Joint Gate or ADR acceptance is introduced.

Sound & Host Lead owns Common Water, Fluid, Resonant, Size/Motion meaning, positive/negative behavior,
must-preserve/reject conditions, reference material and human perceptual decisions. Engineering owns feasibility,
question/fixture/proxy mapping, implementation limitations and reproducibility. This follows
[Collaboration Roles](../../docs/COLLABORATION_ROLES.md); engineering readiness is never perceptual acceptance.

## 2. One instance, four review layers

| Layer | Sound-owned definition | Engineering responsibility |
|---|---|---|
| W-COMMON | Source-driven material transformation, Water-family identity, recognizability, musical usefulness, anti-Foley and artifact boundaries | Separate numerical/source checks from identity/usefulness judgments |
| W-FLUID | Continuous, irregular, liquid articulation and transient-related movement; non-mechanical character | Ask whether existing A/B/D mechanisms can answer the clause; do not redefine Fluid around A+B+D |
| W-RESONANT | Stable, cohesive liquid resonance, tonal compatibility and predictable response, with less temporal activity than Fluid | Evaluate C as a research mechanism; do not call it production Resonant or a lesser-quality mode |
| W-MACRO | Size: Fine/Small/Bright to Large/Deep; Motion: Calm/Stable to Active/Flowing, with consistent meaning across modes | Identify available degrees of freedom and gaps; do not choose mappings, ranges, defaults, compensation or transitions |

These are review groupings inside one instance, not four contracts. Formal clause IDs and precise language
belong to Sound & Host Lead. The matrix below uses provisional local IDs solely to demonstrate the handoff.
It does not establish accepted positive/negative/preserve/reject clauses or freeze 1/3/5 listening anchors.

## 3. Evidence and engineering review vocabulary

| Class | Question answered | Evidence boundary |
|---|---|---|
| E1 — Engineering Validity | Finite/stable output, fixed-seed determinism, prepare/reset/reprepare, tail, rate/partition behavior, bounded state and RNG isolation | Tests and code-path review support only their declared scope; no claim of Water identity, sound quality or musical usefulness |
| E2 — Source Preservation | Carrier ownership, transient/pitch/harmonic/stereo/dynamic preservation | Exact carrier composition is an engineering invariant; audible recognizability still needs E4. A retained carrier can be masked by its residual |
| E3 — Objective Perceptual Proxy | RMS/level difference, residual energy, spectra, temporal observations and measured tail behavior | **Proxy only. Not perceptual truth.** No Water/Fluidity/quality score or candidate ranking |
| E4 — Human Perceptual Evidence | Water identity, mode responsibility, recognizability, usefulness, acceptable artifacts and product preference | Independent listening records and explicit human ACCEPT / REVISE / REJECT, under the applicable listening protocol |

The same measurement can support more than one question; label its purpose. Finite output is E1; RMS used to
contextualize perceived activity is E3. No favorable E1/E2/E3 result implies Water acceptance. E4 cannot override
a failed mandatory engineering gate. Reuse [TESTING](../../docs/TESTING.md) for listening and safety requirements.

| Review status | Meaning | Does not mean |
|---|---|---|
| ENGINEERING-READY | Clause is clear enough to formulate a question and plan evidence | Evidence passed, brief accepted, or experiment execution authorized |
| NEEDS-CLARIFICATION | Meaning, scope, preservation or reject boundary is ambiguous/conflicting | Permission for Engineering to choose a sound target |
| NEEDS-MATERIAL | Question is clear but its perceptual review needs representative/reference material | TESTDATA-001 should become a musical acceptance corpus |
| OUT-OF-SCOPE | Request specifies algorithm adoption, production mapping/UI or another downstream responsibility | A negative judgment of the sound idea |

Use only these four statuses for a clause review; never Water PASS/FAIL, Good/Bad, Best or Winner.
Record reviewer, date, brief revision and rationale separately. Document DRAFT status and historical test PASS
are different facts. Where multiple gaps exist, record the first blocking status and list the remaining gaps.

## 4. Reusable clause record

Copy one record per Sound-owned clause, preserving its wording and source. If its revision changes, revisit the
interpretation and evidence rather than carrying a previous ENGINEERING-READY label forward automatically.

```text
Clause ID / layer:
Brief revision / section / acceptance evidence (or explicitly not accepted):
Perceptual intent (verbatim clause or attributed paraphrase):
Positive / negative / must-preserve / reject references:
Engineering question (observable question, not a proposed tuning answer):
Evidence classes and which conclusion requires human listening:
Engineering fixture (TESTDATA ID or declared algorithm probe):
Listening material requirement (LISTENING-001 source and question, or missing):
Possible objective proxy / measurement window / comparison reference:
Proxy limitation: Proxy only. Not perceptual truth.
Current SPIKE mechanism / config / seed / output kind (residual or processed):
Existing observation (OBS ID and source, or no evidence):
Engineering gap / downstream work / stop condition:
Review status / rationale:
Reviewer / date:
```

Workflow: Sound supplies the clause and references -> Engineering fills the record -> Sound confirms the
interpretation -> Engineering records readiness and unresolved gaps. Actual EXP-W-002 execution still requires
the accepted instance and applicable development-readiness gates. All IDs in the following tables are provisional;
their statuses are **illustrative triage of the supplied direction**, not a completed review of Sound's new brief.

## 5. Fixture and material key

Use the existing [TESTDATA-001 corpus](../../testdata/README.md), without regenerating or altering it for this phase.
All filenames below are relative to `testdata/input/`.

| Key | Filename | Engineering question |
|---|---|---|
| S0 | `zero_input__silence.wav` | Does a reset, never-excited candidate remain silent? |
| I0 | `zero_state_response__impulse.wav` | Does excitation yield a bounded response and decay? No-event behavior is possible |
| SW | `frequency_response__log_sweep.wav` | What frequency-dependent coloration is present? |
| LV | `harmonic_response__stepped_sine_1khz.wav` | How do level changes affect response and thresholds? |
| IM | `intermodulation_response__two_tone.wav` | What sidebands/intermodulation require further investigation? |
| NZ | `broadband_response__white_noise.wav` | What spectral distribution and temporal variation are visible? |
| GT | `envelope_response__gated_sine.wav` | How do onset, quiet/strong gates and post-excitation gaps differ? |
| TR | `transient_response__pitch_decay.wav` | How do transient timing and pitched decay interact with the mechanism? |
| HF | `aliasing_response__high_frequency_sine.wav` | Which HF features need rate-aware investigation? Plots alone do not establish alias rejection |
| ST | `stereo_isolation__channel_probe.wav` | Is response consistent with declared channel ownership, including tails from earlier excitation? |

`LM` below means licensed, representative [LISTENING-001](../../testdata/listening/README.md) material selected
by Sound & Host Lead: drums, vocal, piano/guitar, pad and bass as relevant to the question. Reference/anti-example
selection and licensing remain that work item's responsibility. Its readiness is required for formal EXP-W-003;
engineering questions using TESTDATA-001 need not wait. Fresh-silence checks must not mistake an intentional
post-excitation tail for an autonomous generator. Strict stereo isolation describes the current spike, not a new
universal ban on future reviewed spatial behavior.

## 6. Provisional engineering handoff matrix

The two tables form one matrix joined by Clause ID. Together they cover all fields in the reusable record;
the first identifies the question/evidence, the second records implementation observations and unresolved work.
Intent text is a preparation example from the requested direction, not authored perceptual authority.

| Clause ID | Perceptual intent | Engineering question | Evidence | Engineering fixture | Listening material requirement | Possible objective proxy | Proxy limitation |
|---|---|---|---|---|---|---|---|
| W-COMMON-01 | Source-driven / anti-Foley | Are events tied to excitation; does fresh silence remain zero while prior excitation decays? | E1/E2/E4 | S0, I0, GT | LM drums/pad for causal relation | Zero-source event counts, output/residual energy, tail | Gating cannot prove perceived causal relation or anti-Foley identity |
| W-COMMON-02 | Material transformation | Is output source plus one declared residual; is change audible as transformation? | E1/E2/E3/E4 | I0, NZ, SW | LM vocal/piano | Carrier error, residual RMS, waveform/PSD | Nonzero residual does not imply material identity |
| W-COMMON-03 | Water-family identity | Do both modes satisfy the same human Water anchors across relevant sources? | E3/E4 | NZ, SW only for context | LM plus positive/negative reference anchors | Spectral context, level difference | No objective Water classifier; E4 alone judges identity |
| W-COMMON-04 | Input recognizability | What timing, pitch, dynamics and stereo information survives, including a Water-only processed comparison? | E2/E3/E4 | TR, LV, IM, ST | LM drums/vocal/bass/piano | Waveform, FFT/PSD, RMS, stereo correlation | Carrier retention/FFT peak cannot prove recognizability; current plots downmix channels |
| W-COMMON-05 | Musical usefulness | In which declared uses is transformation useful without relying on a louder output? | E3/E4 | LV, GT for level context | LM and intended musical use cases | RMS difference against dry/declared baseline | RMS is not perceptual loudness matching or utility |
| W-COMMON-06 | Artifact boundaries | Which numerical defects fail engineering checks, and which audible artifacts cause rejection? | E1/E3/E4 | All ten | LM pitched/sustained/transient cases | Finite/peak/DC, plots, tail | Finite/bounded does not prove click-free or acceptable sound |
| W-FLUID-01 | Continuous irregular flow / non-mechanical | Is variation source-related and continuous; does a human hear unwanted periodic modulation? | E1/E3/E4 | NZ, GT, SW | LM sustained pad and repeated phrases | Spectrogram, RMS context; future justified temporal proxy | Randomness and visible modulation do not prove Fluidity |
| W-FLUID-02 | Bubble/liquid articulation | Which source classes require liquid events, including isolated impulses? | E1/E3/E4 | I0, GT, NZ, TR | LM percussion plus reference examples | A event counts and residual energy | More events is not better Water; no-event I0 is not automatically a bug |
| W-FLUID-03 | Droplet-sensitive transient response | Which transient strengths/types should produce a perceptibly related response? | E1/E2/E3/E4 | GT, TR, I0, LV | LM quiet/strong percussion | B event timing/counts, waveform | Threshold crossings do not define desired sensitivity or audibility |
| W-RESONANT-01 | Stable/cohesive liquid resonance | Does the response decay predictably and satisfy liquid-resonance anchors? | E1/E3/E4 | I0, NZ, SW | LM pad/piano with anchors | Residual decay windows, PSD, RMS | Stable resonance can still sound generic or metallic |
| W-RESONANT-02 | Tonal compatibility / predictable response | Does the residual preserve usable pitch and articulation across tonal sources? | E2/E3/E4 | IM, TR, HF | LM piano/guitar/vocal/bass | FFT/PSD, waveform and level context | Fixed spectral peaks do not establish harmonic compatibility |
| W-MACRO-SIZE | Fine/Small/Bright to Large/Deep | Are there degrees of freedom to investigate consistent scale direction in both modes without primarily changing level? | E2/E3/E4 | SW, NZ, TR; future scoped probes | LM plus semantic anchors | Future reviewed mapping comparisons, level/spectral context | Existing frequency/decay controls do not prove a valid Size mapping |
| W-MACRO-MOTION | Calm/Stable to Active/Flowing | Can temporal behavior vary consistently across modes without mainly becoming gain, with Resonant subtler than Fluid? | E2/E3/E4 | GT, NZ; future scoped probes | LM sustained/repeated phrases | Level difference; future justified activity proxy | No Motion transport/mapping or cross-mode semantic evidence exists |

| Clause ID | Current SPIKE mechanism | Existing observation | Engineering gap | Illustrative review status |
|---|---|---|---|---|
| W-COMMON-01 | A/B gated events, D activity control, C excitation/decay | Existing silence and no-zero-source-event checks; see evidence below | E4 causal/anti-Foley review still required | ENGINEERING-READY |
| W-COMMON-02 | Renderer adds source once to A/B/D/C residual | Recorded carrier error <= 2.9802322387695312e-08 in typical-signal smoke | Audible material transformation unvalidated | ENGINEERING-READY |
| W-COMMON-03 | ABD and C as observations, not identity definitions | No Water identity acceptance | Sound-owned anchors and mode-specific positive/negative/reject clauses | NEEDS-CLARIFICATION |
| W-COMMON-04 | Current channel-isolated processed paths | Carrier/partition/isolation evidence; D coloration observation | Representative E4; no implemented onset/pitch-retention analyzer | NEEDS-MATERIAL |
| W-COMMON-05 | Fixed-config offline paths | No musical utility evidence | Intended use cases and representative LM | NEEDS-MATERIAL |
| W-COMMON-06 | A/B voice pools, D delay, C resonator | E1 coverage; hard stealing and spectral/timbral risks remain | Sound must distinguish tolerated character from reject artifacts | NEEDS-CLARIFICATION |
| W-FLUID-01 | D; ABD with A/B/D ablation | OBS-W-D-001; reproducible temporal state | Human non-mechanical/continuous-flow evidence absent | NEEDS-MATERIAL |
| W-FLUID-02 | A within ABD | OBS-W-A-001 | Clarify whether this transient class belongs to A's perceptual responsibility | NEEDS-CLARIFICATION |
| W-FLUID-03 | B within ABD | OBS-W-B-001 | Define low-energy responsibility before any threshold changes | NEEDS-CLARIFICATION |
| W-RESONANT-01 | C alone | OBS-W-C-001 | Clarify audible liquid/cohesive anchors; numerical stability already has evidence | NEEDS-CLARIFICATION |
| W-RESONANT-02 | C fixed mode family | Decay, finite/stereo evidence; no tonal listening conclusion | LM pitch/harmonic acceptance | NEEDS-MATERIAL |
| W-MACRO-SIZE | A/B frequency/decay; C root/decay; D delay degrees of freedom | Research configuration exists; no semantic mapping evidence | Accepted semantics before mapping experiments; no calibrated radius or size model | NEEDS-CLARIFICATION |
| W-MACRO-MOTION | A rate; B threshold/refractory; D interval/depth; C fixed response | Prepare-time controls only; C has no declared Motion mapping | Cross-mode feasibility unresolved; dynamic controls/transition design deferred | NEEDS-CLARIFICATION |

Macro inventory describes possible degrees of freedom only. It does not equate Size with frequency or Motion
with event rate, select a curve, freeze a range/default, or authorize changing any config. A request to freeze
such values, register Host controls or implement transitions is OUT-OF-SCOPE for this handoff.

## 7. Existing Engineering Observations

Source: [REVALIDATION.md](SPIKE-W-DSP-001/REVALIDATION.md), especially “Typical-signal smoke”, “Engineering
observations and listening risks” and the final JSON parser repair revalidation. These are existing measurements,
not reruns for this document. Defaults, seed 42, 48 kHz and three appended seconds of silence apply to the cited
smoke observations; core processed cases used blocks 7/128/1024, residual/control comparisons block 128.
The final repaired renderer source is `06443b4`; that report records equivalent smoke results after repair.
The earlier [EVIDENCE.md](SPIKE-W-DSP-001/EVIDENCE.md) is explicitly historical, not current acceptance.

| Observation ID | Mechanism and existing observation | Question for Sound-owned brief | Disposition |
|---|---|---|---|
| OBS-W-A-001 | A: no event on the single-sample impulse at the cited defaults/seed; gated high/low sections have 25/5 events | Must Fluid's Bubble contribution respond to this impulse class, or is another input-driven response sufficient? | No tuning authorized; not automatically a production bug |
| OBS-W-B-001 | B: gated high/low counts 1/0; no one-sample impulse event; four pitch-decay transient events | Which quiet/transient classes must have a Droplet response, and what input articulation must survive? | No threshold/refractory tuning authorized |
| OBS-W-D-001 | D: PSD ripple/low-level off-main-track sweep features; processed-vs-dry RMS delta about -0.697 dB HF and -0.836 dB sweep | Which generic chorus/flanger cues are negative or rejecting, and under which source/use conditions? | Audibility and alias attribution unmeasured; no Flow/interpolation/compensation change |
| OBS-W-C-001 | C: default impulse residual peak about 1.25e-5 and decaying response; generic/metallic character is a listening risk | What audible anchors distinguish stable/cohesive liquid resonance from generic ringing? | No gain/modal/excitation tuning; small residual is not a quality verdict |

E1 evidence additionally covers fixed-seed reset/reprepare, bounded capacity, RNG domains, ablation, tested
44.1/48/96 kHz rates and block partitions. Exact repeatability is within the tested build/platform, not a promise
of cross-compiler bit identity. Preliminary callback timing is not an accepted performance budget; formal
performance provenance remains NOT RUN in the spike. No allocation instrumentation or new runtime result is
claimed by this handoff. Consult the source report for coverage and limitations rather than generalizing PASS.

## 8. Worked handoff and current brief gaps

Worked example (preparation only):

```text
Clause: W-COMMON-01 (provisional, not accepted)
Intent: response must be input-driven; silence must not create unrelated foreground events
Positive/negative/preserve/reject references: final Sound-owned clause references pending
Question Q-COMMON-01: after reset, is S0 exactly silent; after GT excitation, do new events
  cease on exactly zero input while the existing state follows its declared bounded decay?
Evidence: E1 exact silence/event/decay checks; E2 carrier ownership; E4 audible causal relation
Fixture: S0 + GT; LM percussion/pad needed for E4
Mechanism/config/seed: A/B/D/ABD/C; checked-in spike defaults; seed 42
Output/reference: processed y and separate residual E; dry x with identical appended silence
Observation: existing silence/gating checks and OBS-W-A-001 / OBS-W-B-001
Proxy: output/residual RMS and event counts; identical sample rate, channels and time windows
Limitation: Proxy only. Not perceptual truth. No event-rate or Water-quality target
Status: ENGINEERING-READY for question/evidence planning only
Reviewer/date/accepted brief revision: not recorded; real clause review pending
Next action: Sound confirms interpretation; accepted brief gates downstream subjective experiments
```

Reviewing the remote draft snapshot found these handoff gaps; this companion does not repair them on behalf of
Sound & Host Lead or declare a final feasibility decision:

| Gap | Current evidence | Required handoff |
|---|---|---|
| Dual-mode responsibility | Old draft has general Water attributes and Motion/Fluidity; no dedicated Resonant responsibility or shared Size/Motion section | Sound supplies Common/Fluid/Resonant/Macro clauses, negative/preserve/reject conditions and cross-mode semantics under current #17 scope |
| Candidate-local settings versus product meanings | Draft's “depth” wording does not identify Size, Motion or stage/global mix responsibility | Clarify intended perceptual dimension without selecting DSP mapping |
| Review artifact naming | Old draft names `LISTENING_NOTES.md`; current TESTING and Developer Sound Tools name `LISTENING_REVIEW.md` | Align the revised draft to canonical naming; Phase 2 must also reconcile planned root audio paths with proposed `audio/` staging |
| Listening anchors and decisions | Old draft includes proposed 1/3/5 anchors; no accepted instance/review decision is established | Treat as Sound-owned draft, not frozen rubric or completed E4; do not prefill scores/conclusions |
| Material and loudness evidence | TESTDATA and existing RMS comparisons are engineering evidence; representative musical acceptance remains absent | LISTENING-001 plus the actual matched-loudness procedure is required for formal listening; raw RP-v0 level reporting cannot claim it |

## 9. Track B boundary and next phase

The engineering lane can prepare the generic `SOUNDLAB-RP-001` contract without waiting for final sound taste.
It should have its own bounded issue/PR lifecycle; #17 remains the perceptual brief work item. Neither a new
GitHub issue nor the Review-Pack builder is created by this Phase 1 deliverable.

Phase 2 should resolve directory/manifest versioning, candidate identity, independent audio/config/seed/source
provenance, output-conflict/failure behavior, validation and level-comparison windows. In particular:

- Reuse `analyze_audio` / `write_plots` from `tools/analyze_testdata.py`. Spectra/waveform plots use channel mean;
  aggregate RMS uses all channel samples. Do not infer per-channel preservation from downmixed plots.
- Distinguish RMS(y), RMS(x) and RMS(E). Candidate-versus-reference level difference is based on processed
  audio over comparable durations including the same tail treatment, not residual-versus-dry energy.
- Keep `finite`, `reference_zero`, `candidate_zero`, `both_zero` semantic states explicit; freeze the exact
  numeric/null representation in Phase 2, not here. RMS delta is not loudness matching; audio stays unmodified.
- The supplied RP proposal requests artifact hashes. Limit any later approved implementation to its required
  staged pack artifacts under [AGENTS section 0](../../AGENTS.md); do not broaden this into routine source,
  dependency, build or worktree hashing. This documentation phase calculates none and adds no hash manifest.
- Water clause/question IDs, evidence rationale and proxy limitations are metadata for the handoff; a generic
  builder must not encode Water/Fluid/Resonant judgments. No score, ranking, automatic decision or conclusion.

This phase defines the question/evidence handoff only. It does not freeze RP-v0 schema or implement phases 2–8.
The real per-clause integration in Phase 9 waits for Sound's revised brief. DSP subjective changes, macro mapping,
Host parameter/schema changes, production Water, routing, Ice, Production UI and automatic loudness matching
remain outside this task. If such a need appears, record a downstream proposal rather than extending this scope.

## 10. Phase 1 completion and documentation impact

- Four layers, four evidence classes, four review statuses, a reusable record, a joined matrix and a worked
  example are present. Existing A/B/D/C observations retain their source, conditions and uncertainty.
- Sound can supply a clause using section 4; Engineering can formulate a question without choosing its sound.
  Demonstrating the format is not acceptance of a real clause or closure of EXP-W-001.
- Changed: this companion and the experiments index. Targeted Documentation Impact Check applies: no accepted
  contract, production behavior, module existence/responsibility, support or milestone claim changes.
- Reviewed without updates: Coding Plan, Perceptual Contract/template, Collaboration Roles, Developer Sound
  Tools, TESTING, Project Status, Module Index, testdata and listening READMEs, and SPIKE README/REVALIDATION.
  Their lifecycle, evidence boundaries and implementation status remain applicable. The remote draft gaps in
  section 8 stay explicit; cross-document consistency is not a claim that the old draft is ready for acceptance.
- Validation on 2026-09-16: `python tools/check_markdown_links.py`, `python tools/check_portability.py` and
  `git diff --check` PASS. The new untracked companion was also passed directly to both scanners; a separate
  matrix/path/whitespace check confirmed 13 unique paired clause rows and all ten existing fixture paths.
  Post-draft quality/documentation review checked provisional IDs/statuses, evidence attribution, ownership,
  absent-brief links and downstream boundaries. Phase 1 consistency PASS; real brief acceptance remains pending.
- Runtime builds/tests, renders, performance, pluginval/DAW and listening: **NOT RUN** for this docs-only phase.
  Independent review and Sound confirmation remain pending; branch publication does not satisfy either gate.

## 11. Phase 9 preliminary review of the available draft

Engineering self-review, 2026-09-16, exact remote brief revision `fcecc7d76d35353fa9ac7d9ba75f908541a00356`.
Issue #17 is still open. This is a real review of that available **draft**, separate from the provisional
matrix above. It is not Sound confirmation, a formal GitHub review or acceptance of the updated dual-mode
instance. Section numbers below refer to that pinned remote document; no new Sound-owned clauses are invented.

| Draft section / question ID | Engineering question and evidence | Fixture / listening material | SPIKE mapping and gap | Review status |
|---|---|---|---|---|
| 1 / Q-DRAFT-01 | Does source-driven residual composition preserve an audible source role? E1/E2/E4 | S0/GT/ST; LM vocal/drums/bass | A/B/D/C carrier evidence exists; recognizability is not proved by composition | NEEDS-MATERIAL |
| 2.1 / Q-DRAFT-02 | Is continuous motion heard without obvious periodic chorus/flanger/pitch wobble? E3/E4 | NZ/SW; LM pad/repeated phrases | D/ABD; OBS-W-D-001 supplies spectral context only | NEEDS-MATERIAL |
| 2.2 / Q-DRAFT-03 | Which low-energy/isolated transients must yield a perceptibly related response, and may components share that responsibility? E1/E2/E3/E4 | GT/I0/TR; LM percussion | A/B/ABD; OBS-W-A-001 and OBS-W-B-001 need interpretation before threshold/event changes | NEEDS-CLARIFICATION |
| 2.3 / Q-DRAFT-04 | Is identity stable across phrases/sources while fixed-seed technical behavior remains reproducible? E1/E3/E4 | GT/ST; LM drums/voice/pad/bass | RNG/reset/partition observations exist; no cross-source Water identity evidence | NEEDS-MATERIAL |
| 2.4 / Q-DRAFT-05 | Does “depth” refer to Size, Motion, stage Amount or another perceptual dimension; what defines normal use? E2/E3/E4 | LV/TR; LM pitched/transient sources | Research controls exist, but no production macro map; do not silently equate depth with gain or delay | NEEDS-CLARIFICATION |
| 2.5 / Q-DRAFT-06 | Can each verbal anchor be paired with a positive/negative listening question without requiring its physical synthesis mechanism? E3/E4 | SW/NZ for context; LM chosen for each anchor | A/B/D/C are possible mechanisms, not definitions; references need not be literal water recordings | ENGINEERING-READY |
| 3 / Q-DRAFT-07 | Which artifact is numerically invalid and which is audible but context-dependent rejection? E1/E3/E4 | All ten; LM with declared normal-use regions | A stealing, D coloration and C generic/metallic risks remain; separate zero-input events from legitimate tails | NEEDS-CLARIFICATION |
| 4.1 / Q-DRAFT-08 | Do all ten diagnostic questions preserve their distinction from musical evidence and declared channel behavior? E1/E2/E3 | S0/I0/SW/LV/IM/NZ/GT/TR/HF/ST; E4 separately | Existing tests/corpus can supply scoped evidence; current strict channel isolation does not forbid future reviewed spatial designs | ENGINEERING-READY |
| 4.2 / Q-DRAFT-09 | Are representative inputs licensed and suitable for the named questions? E4 | LISTENING-001, not diagnostic substitutes | No musical source/redistribution acceptance supplied by the SPIKE | NEEDS-MATERIAL |
| 5.1 / Q-DRAFT-10 | Is the baseline identity/purpose declared before comparison, including dry-equivalent controls? E1/E3/E4 | Same input/window for every path; LM for E4 | RP-v0 records explicit baseline config/seed/provenance; no baseline algorithm chosen | ENGINEERING-READY |
| 5.2 / Q-DRAFT-11 | What perceptual use regions must candidate-local low/normal/extreme settings represent? E2/E3/E4 | LV/GT plus LM | Actual values belong to accepted-brief downstream experiments, not this handoff | NEEDS-CLARIFICATION |
| 5.3–5.4 / Q-DRAFT-12 | Can raw source/config/seed evidence be kept distinct from later matched/blinded listening records? E1/E3/E4 | Matched-duration diagnostics now; LM and listening procedure later | RP-v0 preserves raw artifacts and level context; matching/blinding/listening still required by the formal stage | ENGINEERING-READY |
| 6–7 / Q-DRAFT-13 | Do the rubric and completion gate address current Common/Fluid/Resonant/Macro responsibilities? E4 | Sound-owned clauses and LM | Old Motion/Fluidity-led rubric and four attributes do not supply the missing Resonant/shared macro clauses | NEEDS-CLARIFICATION |

Phase 9 can advance to a full revised-brief handoff when Sound supplies explicit Resonant responsibilities,
shared Size/Motion meaning and positive/negative/preserve/reject references. Re-review the changed clauses at
their new exact revision; the accepted instance is required before executing subjective EXP-W-002 questions.
No tuning is performed to answer this preliminary review. Any request to implement normalization, adopt an
algorithm or freeze production mapping during this work is OUT-OF-SCOPE.

Phases 2–8 now have a separate [RP-v0 implementation candidate](../../docs/SOUNDLAB_REVIEW_PACK.md) and
[validation record](../../docs/evidence/SOUNDLAB-RP-001_VALIDATION.md). The historical Phase 1 scope/results
above remain attributed to that phase; this continuation does not retroactively claim a completed brief.
