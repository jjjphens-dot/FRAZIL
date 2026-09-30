# Water Listening Remediation Round 01

Status: RUNNING; research only; Human ACCEPT / REVISE / REJECT remains with Sound Lead.
Baseline: `3426c4fd9495767ead6dc7fd462660cb4f215dbd` (fetched unchanged).
Branch: `codex/experiment/water-listening-remediation-b2`.
Implementation/self-review owner: Engineering agent. Independent human review pending.

## Human observations supplied for this round

- B1 is weak on percussion / dense transients / large dynamics. In Bb Up Stroke
  120bpm, B is more prominent than A, with a near-fixed-pitch ping after each stroke.
  The current 2 mm equivalent radius predicts approximately 1643 Hz.
- Plucky Bass E 120bpm: B timing follows the pluck well, without an obvious double
  attack. Existing bass examples do not expose the default 20 ms spacing behavior.
  B stereo conclusions on mono material are unreliable until Preview routing is fixed.
- A1 default above 6 kHz sounds like noise / boiling oil / frying. Spectral modal
  peaks have narrow surrounding spikes; perceived event loudness varies too much.
  These observations are not dismissed as an FFT display issue.
- Preview loads 44.1 kHz successfully but Play reports that the default output must
  support stereo at the WAV rate. This is a playback/device failure, not decoder failure.

Reported local source metadata (not independently reproduced yet):

| Source | Rate/channels | Frames / duration | RMS | B1 eligible / minimum gap |
| --- | --- | --- | --- | --- |
| Bb Up Stroke 120bpm.wav | 48 kHz / mono | 199355 / about 4.153 s | about -19.8 dBFS | about 9 / 118.08 ms |
| Plucky Bass E 120bpm.wav | 48 kHz / mono | 194520 / about 4.053 s | about -19.1 dBFS | about 9 / 224 ms |

No WAV permission or current location is inferred. Generated audio stays local.
Synthetic 8/10/20/40/80 ms transient fixtures are required for spacing validation.

## Execution state

- Goal: monitor-rate/mono correction, persistent diagnostics, bounded A1 amplitude
  study, historical D1 transparency/C6 handoff, independent B2 research candidate.
- Success: the attachment's engineering checklist plus honest separation of local
  simulation, device tests and human listening; no production or Host/state changes.
- Current phase: Implementation / targeted Functional Validation. Existing source/DSP-rate coupling and mono routing
  defects confirmed in PreviewController. Preview rate/mono, JSONL/trace, A1 amplitude mapping and isolated B2 implementation are present.
- Next action: implement and test monitor boundary and canonical stereo; then logging,
  A1 v3 candidate, B2 isolated contract/code/integration and offline evidence.
- Next checkpoint: focused Preview functional validation, independent self-review,
  comments/docs, followed by serial Debug/Release/ASAN validation.
- Blockers: authorized musical source paths/intake needed for complete FD-003 C6 pack;
  human listening and real-device validation cannot be inferred from simulation.
- D1 stop gate: historical backend/trajectory stays unchanged. C6/C7 acceptance is
  pending; no numerical prototype promotion in this task.

## Documentation impact and boundaries

Full synchronization applies to changed research module responsibilities, Preview
behavior, random domains and realtime transport. Update affected research contracts,
Preview guide, Developer Sound Tools, Module Index, Project Status, Testing, Water
and spike READMEs and Research Mapping as implementation is verified.
Architecture, Parameters, accepted Water intent, physical-model governance and
Coding Plan decisions remain authoritative and unchanged. B1 historical evidence,
production WaterProcessor, Ice, routing, APVTS and session v5 are outside write scope.

Targeted Debug A1/B1/B2/D1 passed; Preview passed after correcting new-module expectations. Full suites and listening studies remain pending. No implementation, acoustic improvement, physical accuracy,
perceptual acceptance or production adoption is claimed by this intake record.

User follow-up authorizes documentation cleanup/formatting and GitHub branch push after final validation; no merge authorization. Six local WAVs located: two specified mono files, two loop/fill files and two other bass files. Pad coverage is still missing.
