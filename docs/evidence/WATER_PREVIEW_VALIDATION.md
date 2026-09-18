# Water research preview validation

Implementation candidate based on main `fc20370`; local validation of the working changes below.
User-authorized scope: independent engineering preview, no production Water integration or product mapping.

## Validation scope and reference machine

Local working-tree validation on 2026-09-17, Windows x64, Intel Core i9-14900HX,
MSVC 19.43.34809, CMake 4.3.2, pinned JUCE 9.0.1. These results apply to this implementation
candidate, not a published release or a remote CI run. No production DSP algorithm was changed.

## Commands and results

From an initialized MSVC shell, for each of `windows-debug`, `windows-release`, `windows-asan`:

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
```

All three configure/safe builds passed with six jobs. Final CTest runs were serial:

| Preset | Result | CTest elapsed |
|---|---|---|
| Debug | 17/17 PASS | 24.02 s |
| Release | 17/17 PASS | 12.13 s |
| ASAN | 17/17 PASS | 51.35 s |

The new device-free `frazil_water_preview` regression covers nine compositions at 44.1/48/96 kHz,
sample equality with the existing research DSP, fixed-seed/reset repeatability, stereo isolation,
finite output, changed engineering-frequency response, invalid-config rejection and module JSON
round-trip through the renderer's parser. Existing production and research regressions also passed.
No new tests require a sound device on hosted CI.

`clang-format --dry-run --Werror` passed for the seven changed/new C++ files. `git diff --check`,
`python tools/check_markdown_links.py`, `python tools/check_portability.py` and
`python tools/check_vscode_tasks.py` passed. The modified CI YAML parsed successfully.
Existing JUCE/vendor C4819 source-code-page warnings remain; no vendor workaround was introduced.

## Windows GUI observations

Debug application, default system stereo output, 48 kHz / 480-frame callback. The local 90-second
GUI fixture repeats the repository's two-second `envelope_response__gated_sine.wav` 45 times,
preserving its original encoding, rate and channels. This is an engineering signal, not listening
acceptance material. Initial checks also loaded the original two-second source.

| Action | Observed result |
|---|---|
| Startup / CLI source argument | Source loaded without autoplay; ABD / Processed / -12 dB, ready. Fixed an asynchronous-default notification that initially marked the draft dirty. |
| Native Load WAV picker | Selected WAV loaded; source description retained 48 kHz / 2 ch / 90 s, cursor reset to zero. |
| Play, Stop, restart | Real device callback and position advance; Stop clears runtime meters; restart begins at the source start. |
| ABD and C compositions | Apply selected the intended route; input/output meters updated, FINITE remained OK. |
| Dry / Processed / Residual | Applied monitor label changed to x / x+E / E during playback. Dry output reflected -12 dB monitor gain. Nonzero ABD and C residuals were observed, including response during zero-input portions. |
| Engineering slider | C root changed from 260 to 1151 Hz; playback stopped, draft flagged, Play disabled until Apply. |
| Capture A/B, Apply A/B | A restored C root 260 Hz; B restored 1151 Hz; source retained and playback stayed stopped. |
| Copy config | Application reported copying APPLIED module config; clipboard bytes were not independently inspected. |
| Export config | Native save dialog wrote module JSON; file readback contained C root 1151 Hz. Renderer accepted this actual file. |
| Reset config | Returned to ABD / Processed / -12 dB and research defaults; loaded WAV and A/B slots retained. |
| Viewport | Default window and vertical scrolling exposed all engineering controls, meters and lower status text. Minimum-size and mixed-DPI matrix were not completed. |

GUI checks used the actual Windows application through Computer Use, not a mock UI.
The ASAN GUI also loaded the original two-second source and played ABD through the physical
48 kHz / 480-frame output callback. It stopped automatically at 32.0 seconds with
`Finished source and 30 s tail`; the captured ASAN stderr log contained no diagnostics.

## Offline handoff and timing

The actual GUI-exported module JSON was supplied to the existing Release renderer:

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
& $renderer testdata/input/envelope_response__gated_sine.wav build/water-preview/gui-c-render.wav c 128 42 build/water-preview/gui-export-c.json 30
```

PASS: 1,536,000 frames, 48 kHz, seed 42, peak 0.501254, RMS 0.044654,
residual RMS 0.00000525329. This establishes nonzero research output and GUI-to-renderer config
handoff, not perceptual quality. Generated config/WAV and raw build logs remain ignored.
The offline output excludes monitor gain and monitor fades; no claim of bit equality with captured
sound-device output is made.

Release preview regression's preliminary ABD core observation was 13.7317 microseconds per 128
stereo frames (256,000 frames, fixed workload, arithmetic mean). It excludes device, UI and meter
overhead and is neither a worst-callback measurement nor formal performance-budget evidence.

## Code Quality Review / Comment & Documentation Pass

- Reviewed lifecycle and callback paths separately from functional testing: WAV decode/JSON/prepare
  run with callback detached; source/DSP state have one audio owner; callback uses preallocated state
  and lock-free scalar publications. Monitor smoothing is 10 ms; Stop is a transport command.
- New settings/engine/controller/UI boundaries serve config export, DSP ownership, device lifecycle
  and presentation respectively. UI has no DSP object access. No mutable globals, new production
  parameter IDs, state schema, hidden algorithm mapping or duplicated production path was added.
- Updated the guide, research README, Developer Sound Tools, Module Index, Testing, Environment,
  Project Status, Coding Plan tooling note, UI README and root README entry. UI/build/module/status
  documentation impact rows were checked together; feature implementation is a candidate, while
  production Water/macros and human workflow acceptance remain pending.
- Reviewed Architecture, PARAMETERS, Accepted ADR-0003, CODE_STANDARDS and GITHUB_WORKFLOW without
  changes: production dependency, parameter/state/routing, realtime and GitHub gate contracts hold.
  `src/ui` Release wording now explicitly refers to FRAZIL plugin targets; the separate optimized
  research executable is not a FRAZIL release artifact.

## Limitations / handoff

No pluginval or new DAW matrix was run: the new artifact is a standalone research executable;
production plugin sources and Host/state contracts are unchanged. No subjective listening acceptance,
production Water adoption, Size/Motion/Decay mapping, formal callback budget, recording/looping,
resampling, input-device routing or complete accessibility/DPI/device-switch matrix is claimed.
Hosted CI, independent collaborator review and merge are pending; the configured CI job now opts
into the research preview build and device-free test.

The Sound Lead's next step is the [debugging guide](../DEV_UI_WATER_DEBUG_GUIDE.md): compare applied
engineering configurations, retain source/config/mode/seed/monitor context and report observed
behavior. Product/perceptual decisions keep their existing separate gates.
