# FD-003 listening handoff protocol

Status: first local Round 01 pack GENERATED with incomplete source-class coverage;
human assessment NOT ASSESSED.
Authority: [FD-003](EXP-W-FD-003.md) and the accepted Water perceptual brief.
This handoff is not evidence of listening or a request to adopt a filter.

## Scope and materials

Keep at most five strategy combinations: S0 raw control, up to three qualified S1
character variants, and S2 common-band comparison. A numerically rejected cell must
be labelled in the private engineering key and cannot become a production candidate.
S0 at a rate with no qualifying kernel provides raw/historical references only.
S2 at 96 kHz remains a comparison if its conditioner fails the core magnitude gate.

Required source classes: authorized bass, drums, representative musical pad, and
guitar/piano creative material. Track source permission, duration, channels and
rate/conversion method in an ignored local manifest. Do not commit private paths or
material. Representative musical-pad and guitar/piano coverage still needs confirmed intake;
human review is pending. Engineering synthetic fixtures are not substitutes.

For each rate (44.1/48/96 kHz), present Source, raw AB, conditioned AB, historical
AB+D, and qualified candidate AB+D. Shared references need not be duplicated for
every variant. Conditioned D OFF and candidate D ON share conditioning and fixed
engineering latency; preserve intentional physical transfer. Historical D1 has its
own explicit legacy processing description. Keep direct-carrier/full-output renders
separate from Fluid residual comparisons; do not introduce AB+alpha*H(AB).

## Blinding and level handling

The listener-facing pack uses neutral A/B/C labels and contains no filter, order,
kernel or latency names. Keep a separate engineering key. Use a recorded shuffle
seed, with one mapping across sample rates in each comparison set. No claimed
blind study until that separation is actually implemented and reviewed.

Primary preservation comparisons retain fixed source/output scale. A separately
identified RMS-matched preference set may be generated; never pool its results
with fixed-source attack/recognizability judgments. No hidden peak normalization,
compression or correction EQ. Silence/invalid-level cases must be explicit.

## Review record

Record reviewer, date, source ID, comparison labels/rate, monitoring chain, context,
repeatability and Accept/Reject/Uncertain plus notes for each relevant question:

- Water identity; flow continuity; bubble clarity; droplet attack.
- Metallic, hollow, phasey, chorus-like, flanger-like, pre-ringing, high-frequency dullness.
- Sample-rate timbre consistency and source recognizability.

No aggregate quality score. Separate conditioner-only observations from D movement,
legacy interpolation artifacts and baseline stochastic source-rate differences.
Compare the same physical/config preset; same seed does not imply the same A1 event
stream across rates. Record numerical/event evidence beside, not in place of, listening.

## Handoff gate

Before audio generation, freeze the final code/candidate manifest and verify all
source permissions. Before runtime adoption, complete Sound Lead + Engineering
review of bandwidth/filter/guard/total latency, cross-rate behavior, open validation
findings and ADR-0007. Current plugin/Host/UI remain unchanged until that gate.

## Round 01 execution

Six user-provided local sources, three rates and the existing five-policy shortlist
produced 90 engineering cells. The successful `c6-headroom` pack uses explicitly
recorded common -6 dB input gain after input-rate conversion, before all DSP. Original
0 dB conversion of one fill exceeded full scale and was rejected; the partial first
attempt is retained, not presented as a completed pack. Fixed and RMS-preference
files are separate. Use the inspected common -18 dB playback bus before conversion
to device samples. Source preservation conclusions stay within a fixed-gain set.

Listener directories have neutral labels; the shuffle key and rejected-cell flags
are separate. No scores were populated. C6 human completion and C7 remain pending.
Reproduction, numeric tables and validation findings: [Round 01](../../docs/evidence/WATER_LISTENING_ROUND_01.md).
