# FRAZIL Perceptual Contract

> Framework status: CURRENT/CONTROLLED; first established by v1.3 / [PR #23](https://github.com/jjjphens-dot/FRAZIL/pull/23). Decay candidate revision follows the [v1.4 activation rule](CODING_PLAN.md).<br>
> Water instance status: [EXP-W-001 Revision B](../experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md) is ACCEPTED for perceptual definition.<br>
> Evidence: Human ACCEPT at b616533, engineering preliminary PASS at a5a0d99, and user-reported Engineering Lead oral acceptance recorded on 2026-09-18. Manual evidence is not formal GitHub APPROVE; remaining listening evidence and downstream gates are preserved in brief sections 11–12.<br>
> This framework does not accept a DSP algorithm or register parameters.

## 1. Purpose

A Perceptual Contract translates human and musical intent into reviewable engineering questions without translating
subjective adjectives directly into a fixed DSP implementation:

```text
Human perception
  -> Perceptual Contract
  -> engineering interpretation
  -> DSP experiment
  -> evidence
  -> human decision
```

For example, "more natural" must not silently become "lower the low-pass cutoff." The contract first records the
desired behavior, prohibited behavior, source properties that must survive and conditions that cause rejection.
Engineering then proposes candidates inside `experiments/`.

## 2. Required content

Each contract records:

- ID and human description;
- intent and positive behavior;
- negative behavior and anti-examples;
- properties that must be preserved;
- engineering questions, not predetermined answers;
- possible objective proxies;
- representative test material;
- reject conditions and human review.

Use [`templates/PERCEPTUAL_CONTRACT_TEMPLATE.md`](templates/PERCEPTUAL_CONTRACT_TEMPLATE.md) when an experiment
stage begins. Do not create empty stage trees in advance.

## 3. Objective proxy, not objective truth

Peak, RMS, DC, finite status, crest factor, LUFS, true peak, spectral features, onset preservation, pitch/harmonic
retention, tail decay and stereo correlation may support a question. None proves musical quality or perceptual
identity by itself. For example, spectral flux may show changed temporal activity; it does not prove Fluidity.

Do not collapse candidate evidence into one quality score. Keep an evidence vector with engineering validity,
source preservation, perceptual dimensions, objective observations and an explicit `ACCEPT`, `REVISE` or `REJECT`
decision. Human review remains authoritative for product value.

## 4. Water application

`EXP-W-001` owns the first Water contracts. A Fluid Motion contract may express:

- intent: sustained, non-mechanical temporal movement;
- positive: continuous, irregular, flow-like variation;
- negative: fixed-period LFO, mechanical tremolo, simple gain increase or independent random Foley;
- preserve: source rhythm, major transient timing and source identity.

Stochastic scheduling, flow modulation, micro-delay and event activity are engineering candidate questions, not
contract clauses. Fluid and Resonant are assessed against their own responsibilities; no extra metric is required
solely to prove that the two models are perceptually distant.

The planned Water Decay dimension expresses **Water response persistence**, separately from Motion's temporal
activity. It is an application of this framework, not an accepted EXP-W-001 instance or a predetermined DSP answer:

- intent: independently control how long an input-excited Water event/resonant response persists;
- positive: Short/Tight -> Long/Lingering; longer responses/tails may naturally overlap more, shorter responses
  may become more articulated; both Fluid and Resonant retain that high-level direction;
- negative: mainly gain, Water Amount, global mix, parallel balance, event rate, Motion/Flow speed, generic reverb
  wetness/size, source-envelope release, whole-effect duration or independent Foley playback length;
- preserve: source rhythm, major transient timing, recognizability and Size/Motion meaning;
- question: can a listener distinguish activity from persistence while their natural interaction remains useful?
- proxies: tail/overlap/energy observations support review; do not require mathematically constant RMS or decide
  compensation without evidence;
- reject/revise: persistence cannot be distinguished from Amount/activity, source identity is masked, or prolonged
  responses produce unacceptable ringing/artifacts; human listening determines musical acceptance.

Use responsibility orthogonality + perceptual separability + bounded interaction, not strict independence of every
acoustic outcome. Exact decay seconds, coefficients, voice lifetime, mapping curves and live/event-latched policies
remain engineering questions. The Water instance owner must incorporate and review this dimension through
EXP-W-001; this example does not create or accept that deliverable.

## 5. Lifecycle and agent rules

The authoritative lifecycle distinguishes definition from downstream consumption:

```text
Human Water Intent
  -> EXP-W-001 perceptual-definition work
  -> accepted Water Perceptual Contract instance
  -> EXP-W-002+ downstream DSP experiment/refinement
  -> Evidence -> Review -> ADR -> Production
```

Perceptual-definition work creates the project-specific contract instance and does not require a pre-existing
instance of that same contract. Downstream subjective DSP experiment/refinement must both:

1. follow the framework and agent rules in this document; and
2. read the applicable accepted project-specific instance and check its positive, negative, preserve and reject
   sections before proposing sound changes.

For current Water work, that instance is `experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md`. Reading only this
framework document does not satisfy the Perceptual Contract prerequisite. Experiment controls remain outside the
production Host registry until evidence, Joint Gate and the applicable ADR/state compatibility work approve adoption.

For the Decay candidate revision, the [Coding Plan Revision B gate](CODING_PLAN.md#decay-revision-b-completion-gate)
is mandatory before EXP-W-001 acceptance/closure: accepted and merged DOC-W-DECAY-001, owner-synchronized
Issue #17 and four-macro brief, Decay perceptual/UX content and recorded Engineering feasibility review.
Formal EXP-W-002 requires M1 Exit, applicable Developer readiness and the accepted instance including Revision B;
Revision A documentation or the old three-macro brief cannot satisfy this prerequisite. Existing prepare-time
SPIKE decay evidence remains objective feasibility only.

## 6. Optional objective feasibility before the Water instance

[SPIKE-W-DSP-001 / #29](https://github.com/jjjphens-dot/FRAZIL/issues/29) is a bounded,
optional pre-EXP-W-002 engineering work item. It may run alongside perceptual-definition work,
before the Water instance or M1 Joint Exit, inside `experiments/water/SPIKE-W-DSP-001/` only.
It investigates numerical implementation, realtime safety, fixed-seed determinism, residual/carrier
ownership, finite output, reset/tail/state, random isolation, sample-rate/block behavior, ablation,
offline engineering renders and preliminary performance. These are feasibility hypotheses, not
subjective Water candidates evaluated against an invented brief.

Before the accepted instance, this spike must not perform perceptual acceptance, subjective
selection/tuning (including “more natural/watery”), product macro mapping or Fluid/Resonant quality
ranking. It cannot close EXP-W-002, adopt an algorithm, accept ADR-W-001 or integrate production
WaterProcessor. It adds no M1 exit gate and grants no production exception.

The formal lifecycle in section 5 remains unchanged: accepted EXP-W-001 -> EXP-W-002 -> EXP-W-003
listening/refinement decision -> ADR-W-001 -> production. EXP-W-002 must reuse/revise the spike
under the accepted positive/negative/preserve/reject conditions rather than duplicate it or
retroactively label feasibility evidence as perceptual acceptance. The controlled issue/PR review,
not a session authorization note, governs this scope.
