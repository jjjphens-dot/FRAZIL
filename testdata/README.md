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

Verify WAV metadata, provenance fields, license references, storage policy and
the required SHA-256 content hashes:

```powershell
python tools/verify_testdata.py
python tools/test_testdata.py
```

The manifest is the machine-readable source for the input paths and their
integrity evidence. Hashes are for the eight required input WAV files only;
rendered outputs are not reference inputs and are never committed.
