# Offline render smoke

`offline_render.cpp` is the small RENDER-001 command-line harness. It reads a
WAV file outside the realtime plugin path, processes bounded blocks through the
current `AudioEngine`, rejects non-finite output, and writes a WAV artifact.

`tools/render_testdata.py` runs the renderer twice with the same input, block
size, seed and parameters. The current configuration includes both enable
values, routing, stage amounts, parallel balance, and gain/mix values. It
requires identical output bytes, records the current commit/build metadata
plus input and output WAV metadata and SHA-256 values, and writes a render
manifest. The default manual output is under ignored `testdata/rendered/`.

The renderer accepts `--water-enabled 0|1`, `--ice-enabled 0|1`,
`--routing parallel|water-into-ice|ice-into-water`,
`--parallel-balance 0..1`, `--water-amount 0..1`, and `--ice-amount 0..1`, in
addition to the existing block, seed, gain, and global mix options. The seed is
recorded as test metadata only: the current M1 pass-through `AudioEngine` has
no stochastic DSP and therefore does not consume it.

Build and run the harness through the repository presets:

```powershell
python tools/build_safe.py --preset windows-debug
python tools/render_testdata.py --renderer build/windows-debug/frazil_render_artefacts/Debug/frazil_render.exe
```

This is an offline determinism smoke for the current M1 pass-through engine. It
does not claim Water/Ice DSP, full render matrices, listening acceptance, DAW
validation or performance coverage. CTest passes explicit output and manifest
paths under the preset build tree to avoid polluting `testdata/rendered/`.
