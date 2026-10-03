"""Offline child-process evidence; retained failures, unchanged caller timeouts.

Never imported by a realtime target. Paths/commands stay in ignored local evidence.
"""

from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import time


def record(directory, stage, **fields):
    row = dict(timestamp=datetime.now(timezone.utc).isoformat(), stage=stage, **fields)
    with (Path(directory) / "progress.jsonl").open("a", encoding="utf-8") as stream:
        stream.write(json.dumps(row, sort_keys=True) + "\n")
    print(json.dumps(row, sort_keys=True), flush=True)


def run_case(command, directory, case_id, timeout):
    """Write streams directly to disk so timeout/crash cannot discard partial output."""
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    command = [str(value) for value in command]
    env = os.environ.copy()
    # Microsoft ASAN's documented dump hook. It only writes on a detected error.
    env["ASAN_SAVE_DUMPS"] = str((directory / "asan.dmp").resolve())
    (directory / "command.json").write_text(
        json.dumps(dict(case_id=case_id, command=command, timeout_seconds=timeout,
                        asan_dump=env["ASAN_SAVE_DUMPS"]), indent=2), encoding="utf-8")
    record(directory, "child-start", case_id=case_id)
    start = time.monotonic()
    with (directory / "stdout.log").open("wb") as stdout, (directory / "stderr.log").open("wb") as stderr:
        try:
            result = subprocess.run(command, stdout=stdout, stderr=stderr, timeout=timeout, env=env)
        except subprocess.TimeoutExpired:
            record(directory, "child-timeout", case_id=case_id,
                   elapsed_seconds=time.monotonic() - start, exit_code="TIMEOUT")
            raise
    record(directory, "child-finish", case_id=case_id, elapsed_seconds=time.monotonic() - start,
           exit_code=result.returncode)
    if result.returncode:
        tail = (directory / "stderr.log").read_text(encoding="utf-8", errors="replace")[-6000:]
        raise RuntimeError(f"{case_id}: child exit {result.returncode}; evidence {directory}\n{tail}")
    return result
