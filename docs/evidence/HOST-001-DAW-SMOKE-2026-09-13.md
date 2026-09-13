# HOST-001 DAW smoke evidence — 2026-09-13

本记录把用户在本任务中确认的 DAW 观察拆成独立 case。它是可追溯的用户观察记录，
不是完整 HOST-001 验收报告；没有观察到或没有记录的字段保持 `not captured`，不推导为通过。

## Traceability

- Source: [HOST-001 PR #26](https://github.com/jjjphens-dot/FRAZIL/pull/26) and the user-confirmed observations in that task.
- Observation owner: Sound & Host Lead / user; exact observation time and independent reviewer were not captured.
- Environment: Windows x64; exact OS build, sample rate, block size and channel configuration were not captured.
- Artifact: FRAZIL 0.1.0 Debug VST3 built from `main@b595a47`; the machine-specific scan path is intentionally omitted.
- Scope boundary: Water/Ice processing was not judged as a sonic product feature. No parameter, state, routing or realtime contract changed.

## Per-case observations

| Case | Host / version | Narrow observation recorded | Status |
|---|---|---|---|
| Enumeration | Ableton Live 12 Suite 12.4.2 | FRAZIL loaded and the Host displayed the nine current parameters in contract order. | `Verified` — user-observed, narrow scope |
| Automation | Ableton Live 12 Suite 12.4.2 | The nine-parameter test recording, editing and playback showed no abnormal behavior. | `Verified` — user-observed, narrow scope |
| Save/reopen | Ableton Live 12 Suite 12.4.2 | The project was saved, reopened and reported as having no abnormal behavior. | `Verified` — user-observed, narrow scope |
| DAW render | Ableton Live 12 Suite 12.4.2 | DAW render completed with no abnormal behavior reported. | `Verified` — render-completion smoke only |
| Enumeration | FL Studio 2025 25.1.4.4951 | FRAZIL was scanned/loaded and the nine-parameter interaction was reported as working normally. | `Verified` — user-observed, narrow scope |
| Automation | FL Studio 2025 25.1.4.4951 | No separate automation-lane observation was recorded. | `Not run` |
| Save/reopen | FL Studio 2025 25.1.4.4951 | The project was saved, reopened and reported as having no abnormal behavior. | `Verified` — user-observed, narrow scope |
| DAW render | FL Studio 2025 25.1.4.4951 | DAW render completed with no abnormal behavior reported. | `Verified` — render-completion smoke only |

## Evidence not captured

The following HOST-001 acceptance fields remain open and must not be inferred from the observations above:

- Complete rescan/verify output, mono and stereo instance load, editor open/close, bus/lifecycle checks,
  display metadata, range/default/choice text and automation visibility.
- Explicit fast/slow automation ramps, final-value and inactive-value retention checks for every contract case;
  Ableton's record/edit/playback observation is narrower than that matrix, and FL Studio automation was not
  separately captured.
- Save/reopen per-parameter/state assertions and automation-lane restoration evidence for both hosts.
- Render sample rate, block size, channel count, output length and finite-output observation; no durable bounce
  artifact or deterministic comparison was recorded. Deterministic render remains the responsibility of
  `RENDER-001`.
- REAPER validation and any official-support or `Development Validated` classification.

These limitations intentionally keep the case status at `Verified` rather than `Passed` until the full
HOST-001 protocol has durable, reviewable evidence.
