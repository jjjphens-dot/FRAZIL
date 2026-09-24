# DSP Physical Model Governance

Level: CONTROLLED. Task: EXP-W-DB-001, user-authorized B1 physical modular refactor.
This rule is established before B1 sonic implementation. Independent repository review
is still required; this document does not approve a DSP candidate or close a milestone.
Architecture, Parameters, Code Standards, Document Governance and the Perceptual
Contract retain their authority. No existing acceptance or realtime requirement is weakened.

## Model classification

Every significant equation, algorithm and parameter has exactly one primary category:

| Category | Meaning | Required limitation |
| --- | --- | --- |
| PHYSICAL | A theory, approximation or measured relation supported by physical literature | State assumptions, units, validity range and approximation error; empirical physical fits remain identified as fits |
| REDUCED_PHYSICAL_MODEL | Represents a real mechanism with unavailable or omitted physical state | Name the missing state and surrogate; do not describe source audio as measured fluid forcing |
| PRODUCT_MAPPING | Maps accepted product intent or an explicitly unaccepted candidate to DSP quantities | A mapping is not a physical law; acceptance requires perceptual evidence |
| ENGINEERING | Numerical, resource, lifecycle, signal-analysis or calibration mechanism | Constants are implementation choices, not natural acoustic constants |

Mixed expressions must be decomposed: radius scaling is PHYSICAL; substitution of
audio excitation for unknown fluid velocity is REDUCED_PHYSICAL_MODEL; normalization
is ENGINEERING or PRODUCT_MAPPING according to its purpose. Do not assign multiple
categories to one indivisible row. Secondary context does not change the primary category.

## Traceability and evidence

Each module's canonical research contract owns an ID / contract / classification /
source / code owner / independent test owner / status matrix. Implementation status
uses only IMPLEMENTED, RESEARCH-CANDIDATE or DEFERRED. Acceptance and validation
are separate fields; IMPLEMENTED does not mean physically exact or perceptually accepted.
Unimplemented mechanisms appear only as DEFERRED, never as current behavior.

For each equation record units, reference constants, domain, output meaning, and an
independent analytic or experimental oracle. A function tested against itself is not
an oracle. Distinguish external measured data, our derived prediction, and synthetic
test output. Record primary-source access limits; commercial claims are not validation.
Parameter specifications should generate a runtime descriptor checked against a tracked
versioned snapshot. Unknown versions/fields reject; no silent research model migration.

## Review and adoption

Before sonic code: read the applicable accepted perceptual definition and canonical
contracts, review primary literature, identify the reduced/product/engineering boundary,
and create the module contract. Then implement, functionally validate, independently
self-review code quality, complete comments/docs, and run final validation.
An independent human review remains separate from agent self-review.

Changes to shared physics require preserved evaluation order and before/after decoded
render identity for existing consumers, plus their property/CLI regressions. If exact
preservation fails, leave the existing consumer intact and record deferred consolidation.
No model may infer real drop size, impact velocity, cavity/bubble depth, temperature,
surface geometry, absolute pressure/SPL or microphone propagation from arbitrary audio
without the corresponding simulation or measurement. Relative source emission must be
labelled relative; a radial-displacement proxy is not microphone pressure.

This policy applies to new physical-model work in B1 and future A1/D1/C1/Ice revisions;
it does not retroactively rewrite historical evidence or authorize deferred modules.
Production adoption still follows [Perceptual Contract](PERCEPTUAL_CONTRACT.md),
[Coding Plan](CODING_PLAN.md), Joint Gate and applicable ADR/compatibility requirements.
Governance changes use the [Documentation Gate](DOCUMENT_GOVERNANCE.md#5-documentation-synchronization-gate).
