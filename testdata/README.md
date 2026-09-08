# FRAZIL reference input corpus

This directory contains the small, synthetic `TESTDATA-001` corpus shared by
future Water, Ice, render, and property tests. The eight inputs are generated
by [`tools/generate_testdata.py`](../tools/generate_testdata.py) with a fixed
seed; no third-party recording is included. The fixtures are covered by the
repository [MIT license](../LICENSE).

The tracked corpus is intentionally small enough for ordinary Git storage. The
ignored `testdata/rendered/` directory is reserved for generated render output;
the input corpus does not use Git LFS.

Regenerate the fixtures and manifest from the repository root:

```powershell
python tools/generate_testdata.py
```

Verify the machine-readable provenance, purpose, author, redistribution terms,
license, storage policy, WAV metadata and required SHA-256 content hashes:

```powershell
python tools/verify_testdata.py
python tools/test_testdata.py
```

The regression test invokes the generator's complete corpus path in a temporary
directory, including both `input/*.wav` and `manifest.json`. It compares parsed
manifest content semantically, then compares every generated WAV byte-for-byte
and by SHA-256 against the committed corpus; tracked fixtures are never
overwritten by the regression test.

The manifest records `id`, `filename`, `purpose`, machine-readable
`sourceType: synthetic`, descriptive `source`, `author`, license/redistribution
terms, `sampleRate`, `bitDepth`, `channels`,
`durationSeconds`, `sha256`, and repository/artifact/LFS storage policy for each
input. It explicitly identifies a generated synthetic reference corpus with no
third-party audio. Hashes are for the eight required input WAV files only;
rendered outputs are not reference inputs and are never committed.

This engineering corpus intentionally contains only the canonical 48 kHz,
stereo, one-second PCM inputs. Future property/render tests may derive mono,
44.1 kHz, 96 kHz or other supported configurations into temporary/generated
output. Real-world listening material belongs in a separate future
`testdata/listening/` corpus and must not be added here.
