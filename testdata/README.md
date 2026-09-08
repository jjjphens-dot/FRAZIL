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

The manifest records `id`, `filename`, `purpose`, synthetic `source`, `author`,
license/redistribution terms, `sampleRate`, `bitDepth`, `channels`,
`durationSeconds`, `sha256`, and repository/artifact/LFS storage policy for each
input. It explicitly identifies a generated synthetic reference corpus with no
third-party audio. Hashes are for the eight required input WAV files only;
rendered outputs are not reference inputs and are never committed.
