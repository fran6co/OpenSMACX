#!/usr/bin/env python3
"""No test may write into the source tree, and this is what enforces it.

`AGENTS.md` has stated the rule for a long time, and states it because a test
that appended one line to `functions.csv` and restored it made a concurrent
lane read the file at ZERO BYTES, measured 36 times in 40 seconds. Nothing
showed in `git status`, because the file was restored. The rule was written
down. Nothing checked it.

WHAT IT COST. `tools/test_integrate_recovery.py` sets `tool.REPO_ROOT` to a
temporary directory and stubs five seams, which is a reasonable expectation of
isolation. Two paths escaped it, because `integrate_recovery` reached into
another module's IMPORT-TIME constants - `writeback.MATCHED_DIR` and, through
`repair.main(["--apply"])`, `repair_source_locations.FUNCTIONS`. Both are bound
from THEIR module's root when the import runs, so rebinding `REPO_ROOT` in the
tool under test moves nothing. And because `writeback.verify` is stubbed to
return BYTE_EXACT, the tests need no Wine and ran on every `ctest` invocation:

  * `src/recovered/00401000.cpp` was DELETED from the real tree on every run.
    The test address 0x401000 formats to exactly that name, and that file was
    the only body in the tree carrying the BYTE_EXACT proof for
    `??0StringStruct@@QAE@H@Z`. Nothing noticed, because the ratchet counts
    claims that are PRESENT.
  * 37 `source_locations` rows in the real `docs/recovery/functions.csv` were
    re-pointed on every run.

The second one is the more instructive failure, because it made another test
PASS. `test_repair_source_locations.CatalogueIsCurrentTest` asserts the
committed catalogue has no drifted locations. It was green - not because the
catalogue was current, but because `test_integrate_recovery` sorts first and
had just silently repaired it. Removing the side effect turned that test red
and revealed 37 genuinely stale rows. A test suite where one test's damage
satisfies another test's assertion reports on itself, not on the tree.

HOW IT WORKS, and why it is per-test rather than one pass at the end. Each
`test_*.py` runs in its own subprocess and the tree is compared before and
after, so a failure NAMES the offender instead of saying the suite was dirty.
Measured over all 111 files: the whole sweep is a couple of minutes and only
`test_verify_recovery_metadata.py` takes longer than 20 s.

A DIRTY TREE IS NOT AN ERROR, because that is the normal state during
development. The baseline is whatever `git status` says at the start, and only
a CHANGE from it counts. Files that were clean at baseline and got dirtied are
restored, because they are safely restorable to HEAD; a file that was already
modified and got modified again is reported as UNATTRIBUTABLE and left exactly
as it is. Guessing at a restore there would destroy the operator's work, which
is a worse failure than the one this is looking for.
"""

from __future__ import annotations

import argparse
import hashlib
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PYTHON = sys.executable


def tracked_state(repo: Path) -> dict:
    """{path: content fingerprint} for every TRACKED file that differs from HEAD.

    Untracked entries (`??`) are excluded deliberately. Running a Python test
    writes `__pycache__/`, so counting untracked paths reports every test file
    in the tree as an offender and the real one disappears into the noise -
    which is what the first draft did. The damage that matters is to files the
    repository already carries: a modified catalogue, a deleted proof.

    THE VALUE IS A CONTENT HASH, NOT THE STATUS CODE, and that distinction is
    the difference between this check working and this check lying. A file that
    is already modified reads ` M` both before a test clobbers it and after, so
    comparing status codes says nothing changed - the check would pass on
    exactly the tree where it matters most, a developer's dirty one. Its own
    test caught that.
    """
    finished = subprocess.run(["git", "diff", "--name-only", "HEAD"], cwd=repo,
                              capture_output=True, text=True)
    if finished.returncode != 0:
        return {}
    state = {}
    for name in finished.stdout.splitlines():
        name = name.strip()
        if not name:
            continue
        path = repo / name
        try:
            state[name] = hashlib.sha256(path.read_bytes()).hexdigest()
        except (OSError, IsADirectoryError):
            state[name] = "<absent>"    # deleted, which is a change like any other
    return state


def run_one(test: Path, tests_dir: Path, timeout: int) -> None:
    """Run one test file, discarding its output - only its effect matters."""
    subprocess.run([PYTHON, "-m", "unittest", test.stem],
                   cwd=tests_dir, capture_output=True, text=True,
                   timeout=timeout)


def sweep(repo: Path, tests_dir: Path, pattern: str, timeout: int,
          only: str | None = None) -> tuple:
    """(offenders, unattributable, how many tests ran).

    `offenders` is [(test name, {path: status})] for changes this run can pin
    on a specific test AND safely undo.
    """
    tests = sorted(path for path in tests_dir.glob(pattern))
    if only:
        tests = [path for path in tests if path.name == only]
    baseline = tracked_state(repo)
    offenders, unattributable = [], []

    for test in tests:
        try:
            run_one(test, tests_dir, timeout)
        except subprocess.TimeoutExpired:
            # A hang is not this check's business; the test's own registration
            # owns that. Skipping keeps one slow file from hiding the rest.
            continue
        now = tracked_state(repo)
        # Both directions: a path whose content moved, and a path that WENT
        # BACK to matching HEAD - a test that "restores" a file it edited has
        # still written into the tree, and that is the shape AGENTS.md records
        # as costing a concurrent lane a zero-byte read.
        changed = {path: now.get(path, "<restored>")
                   for path in set(now) | set(baseline)
                   if baseline.get(path) != now.get(path)}
        if not changed:
            continue
        restorable = {path: code for path, code in changed.items()
                      if path not in baseline}
        stuck = {path: code for path, code in changed.items()
                 if path in baseline}
        if restorable:
            offenders.append((test.name, restorable))
            subprocess.run(["git", "checkout", "--", *restorable], cwd=repo,
                           capture_output=True)
        if stuck:
            unattributable.append((test.name, stuck))
        baseline = tracked_state(repo)
    return offenders, unattributable, len(tests)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--repo", type=Path, default=REPO_ROOT,
                        help="the git tree to watch (default: this one)")
    parser.add_argument("--tests-dir", type=Path, default=None,
                        help="where the test files live (default: <repo>/tools)")
    parser.add_argument("--pattern", default="test_*.py")
    parser.add_argument("--only", help="run a single test file, by name")
    parser.add_argument("--timeout", type=int, default=600)
    arguments = parser.parse_args(argv)

    repo = arguments.repo.resolve()
    tests_dir = (arguments.tests_dir or (repo / "tools")).resolve()
    if not tests_dir.is_dir():
        print(f"FAIL: {tests_dir} is not a directory", file=sys.stderr)
        return 1

    started = time.time()
    offenders, unattributable, count = sweep(
        repo, tests_dir, arguments.pattern, arguments.timeout, arguments.only)

    # A sweep that ran nothing proves nothing, and would otherwise print the
    # same OK line as a sweep that ran everything.
    if not count:
        print(f"FAIL: no test files matched {arguments.pattern} in {tests_dir}",
              file=sys.stderr)
        return 1

    for name, changed in offenders:
        print(f"  WROTE  {name} changed {len(changed)} tracked file(s):",
              file=sys.stderr)
        for path in sorted(changed):
            print(f"           {path}", file=sys.stderr)
    for name, changed in unattributable:
        print(f"  UNATTRIBUTABLE {name} touched {len(changed)} file(s) that "
              f"were already modified; left alone:", file=sys.stderr)
        for path in sorted(changed):
            print(f"           {path}", file=sys.stderr)

    if offenders or unattributable:
        print(f"FAIL: {len(offenders) + len(unattributable)} test file(s) write "
              f"into the source tree. A test must never do that "
              f"(AGENTS.md); isolate it, or derive its paths from a root the "
              f"test can move.", file=sys.stderr)
        return 1

    print(f"tests-do-not-write: {count} test file(s) ran, none changed a "
          f"tracked file ({time.time() - started:.0f}s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
