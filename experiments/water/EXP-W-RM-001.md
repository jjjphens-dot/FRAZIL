# EXP-W-RM-001 — Structured Resonant Motion comparison

> Historical scope and source-specific evidence below are retained unchanged. Current Water
> definition acceptance, A1 versus A0/Preview and deferred work are indexed in [Water research](README.md).

Status: ENGINEERING COMPARISON PREPARED, research candidate only. No default selection or human acceptance.
Baseline: `64ebf4d`, following bounded C3 and the Decay comparison preparation.

## Mechanism and limits

R-M1 uses one seeded stochastic latent q in [-1,1], smoothly interpolated between random
endpoints over the existing mapped interval. The six positions span -1 to +1 in modal order.
Weights are `exp(depth*q*position)` normalized to sum six. With depth <=.35 their conservative
maximum is exp(.7), below the existing 1.35/.65 weight bound used in C3. The whole-bank bound
therefore remains valid without changing its residual budget, energy target or pole coefficients.

Motion changes neither root frequency, Decay, residual gain nor C3 normalization coefficients.
At Motion=0 the bank uses its unchanged static path exactly. The existing mapping remains
interval 1.96/.7/.25 s and depth 0/.175/.35. R-M1 is optional at typed prepare and as the last
renderer argument `independent|structured`; omission preserves the historical independent weights.
The preview/session/module JSON still uses the historical default. No invisible preview adoption.

This controls a coherent spectral tilt instead of independent per-mode targets. Constant weight
sum is not a loudness guarantee: source spectra and phase cancellation can still affect level.
Human reviewers must distinguish temporal redistribution from mere level changes. No frequency
drift, vibrato or chorus candidate is introduced; optional drift requires prior human evidence.

## Reproduction and evidence

`render/motion_listening_study.py` accepts the existing cases exporter, renderer, one to five
explicit inputs and a new output directory. It compares hard and feature carriers with C3,
old and structured Motion at 0/.5/1, fixed Size/Decay/source/seed and Protect OFF. Actual rendered
outputs at blocks 128/257 must match. Motion=0 must match across old/new models exactly.

The pack preserves fixed-source Full Reference at Monitor -18 dB and separate Water Only RMS
support. RMS matching is attenuation-only after rendering within each source/carrier/model triplet;
it is not LUFS matching, a DSP normalizer, or evidence of source preservation. Reported residual
RMS difference, best scalar fit and remaining difference, spectral energy fraction above 600 Hz,
and tail metrics are objective diagnostics, not perceptual scores or a selection rule.

The new `frazil_water_motion` test covers 44.1/48/96 kHz, three depth/Decay values, FLT_MAX signs,
stereo relationship, positive normalized coherent weights, zero-depth identity, seed behavior,
reset, silence and blocks 32/64/128/256/257/512/1024. Existing tests retain C0 historical arithmetic.
`frazil_water_performance --motion-study` measures 36 bounded-carrier/Motion/Decay cases plus
M1 and legacy-C0 controls, using the existing 48 kHz/stereo/block128 timing procedure.

Independent code review checked fixed-array ownership, existing seed domain, bounded std::exp
arguments, no callback allocation/I/O/locks, explicit policy validation and untouched legacy branch.
Six bounded exponentials per moving sample are a deliberate candidate cost; the performance run
must report this cost rather than assume it is negligible. Renderer parsing rejects other modes
and unsupported policy text; the candidate is not exported as a production or session parameter.

Both independent human judgments are NOT ASSESSED. Prior Decay driven-source level losses and
the unreproduced Phase 3 Debug access violation remain visible in the execution record.

## Measured comparison

Debug 24/24 PASS (45.82 s), Release 24/24 PASS (24.10 s); final ASAN 24/24 PASS (74.77 s).
Local `build/listening-ui/motion-rm1-v1` contains 60 comparisons and 38 timing rows.
All actual outputs are finite and block128/257 identical; stable Motion matches both policies.
For structured Motion=1 the residual after best scalar fitting to Motion=0 is 5.77%-12.16%
of output RMS across these supplied/engineering source and carrier combinations. This rules out
an exact scalar-only difference for those captures; it is not a perceptual sensitivity threshold.
Feature/C3 source-window RMS at Motion=1 is -59.71 dBFS (Sub Bass), -48.77 (Fill), -60.92
(Partisan), -49.37 (Axusr) and -58.23 (engineering pad). Quiet candidates remain an audibility risk.
Maxima across timing rows: mean 9.43807 us, P95 12.7 us, P99 16.9 us, worst 425.6 us.
These are separate row maxima, local standalone measurements, not formal Host/CPU acceptance.

One preceding ASAN suite returned exit1 from an old AD renderer case (48 kHz/block1024).
The test had hidden captured stderr; the exception handler now prints captured diagnostics.
With diagnostics enabled, the renderer suite passed (44.22 s); 100 exact AD ASAN reproductions
passed, followed by the full 24/24 result above. No root cause or fix is claimed for that event
or the earlier native Debug access violation. Both raw failure logs remain local and recurrence
must be investigated before adoption. Repository/link/portability/scanner/task/format/whitespace
checks PASS. Production contracts and human acceptance remain unchanged.
