# FRAZIL Perceptual Contract

> Framework status: Approval Candidate in this v1.3 revision; CURRENT/CONTROLLED after approval and merge.<br>
> Water instance status: `EXP-W-001` Water Perceptual Contract is PLANNED until produced and accepted.<br>
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

## 5. Lifecycle and agent rules

The authoritative lifecycle distinguishes definition from downstream consumption:

```text
Human Water Intent
  -> EXP-W-001 perceptual-definition work
  -> Water Perceptual Contract
  -> EXP-W-002+ downstream DSP experiment/refinement
  -> Evidence -> Review -> ADR -> Production
```

Perceptual-definition work creates the contract and does not require a pre-existing instance of that same contract.
An agent performing downstream subjective DSP experiment/refinement must find the applicable accepted contract and
check its positive, negative, preserve and reject sections before proposing sound changes. Experiment controls remain
outside the production Host registry until evidence, Joint Gate and the applicable ADR/state compatibility work
approve adoption.
