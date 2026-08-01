#!/usr/bin/env python3

"""Run the recovery gate in both presets at once.

`verify-recovery-batch` has to pass in the Debug and the Release preset, and
running them one after the other costs the sum of the two. It does not have to:
every output of that target graph is under the preset's own
CMAKE_CURRENT_BINARY_DIR - build tree, staged game, hybrid image, oracle
manifests, Wine prefix - and `.opensmacx/game` is read only during a batch, so
the two lanes share no writable state. Process kills are safe by construction
too: tools/runtime_process.py stages a `secrets.token_hex(16)`-named hard link
and matches on that absolute path, so one lane's reaper cannot see the other's
game.

Measured on this tree, both trees warm:

    serial      debug 184.47 s then release 147.97 s   wall 332.44 s
    concurrent  debug 190.11 s and  release 154.69 s   wall 190.11 s   1.75x

Per-lane cost of sharing the machine: 1.5% debug, 4.7% release. Verdicts are
identical either way in both presets - the same 13 oracle suites, byte-identical
result files, and the same 118 generated verdicts split {PASS 50,
INCONCLUSIVE-no-effect 45, INCONCLUSIVE-original-faulted 22,
INCONCLUSIVE-original-unstable 1}.

The only genuinely shared resource is DISPLAY, and one X display carries two
Wine sessions.

What this script deliberately does NOT do is run `promote-recovery-metadata`.
That target writes `docs/recovery/` in the source tree, which is exactly the
kind of shared writable state the concurrency argument above depends on not
existing. Promote first, serially, then gate.

    tools/run_gate.py                     # both presets, concurrently
    tools/run_gate.py --preset mingw-i686-debug     # one alone
    tools/run_gate.py --serial            # both, one after the other
"""

import argparse
from pathlib import Path
import subprocess
import sys
import time


PRESETS = ("mingw-i686-debug", "mingw-i686-release")
TARGET = "verify-recovery-batch"
# A gate lane runs for minutes; a fifth of a second of poll granularity is
# below the noise and costs nothing.
POLL_SECONDS = 0.2
# Lines worth echoing from a lane's log when it finishes. A gate log is
# thousands of lines; these are the ones that carry a verdict.
INTERESTING = (
    "tests passed",
    "tests failed",
    "The following tests FAILED",
    "verdicts",
    "SMOKE",
)


class Lane:
    def __init__(self, preset, target, log_directory, repository):
        self.preset = preset
        self.target = target
        self.log = Path(log_directory) / f"gate-{preset}.log"
        self.command = ["cmake", "--build", "--preset", preset,
                        "--target", target]
        self.repository = Path(repository)
        self.process = None
        self.handle = None
        self.started = 0.0
        self.elapsed = 0.0
        self.returncode = None

    def start(self):
        self.log.parent.mkdir(parents=True, exist_ok=True)
        self.handle = self.log.open("w", encoding="utf-8", errors="replace")
        self.started = time.monotonic()
        self.process = subprocess.Popen(
            self.command, cwd=self.repository, stdout=self.handle,
            stderr=subprocess.STDOUT)

    def _finish(self, returncode):
        self.returncode = returncode
        self.elapsed = time.monotonic() - self.started
        self.handle.close()
        return returncode

    def wait(self):
        return self._finish(self.process.wait())

    def poll(self):
        """None while the lane is running, its returncode once it is not.

        Polling rather than waiting on each lane in turn, because
        `debug.wait(); release.wait()` timestamps the release lane when the
        DEBUG one finished - both lanes then report the same elapsed and the
        per-lane cost of concurrency cannot be seen at all.
        """
        if self.returncode is not None:
            return self.returncode
        returncode = self.process.poll()
        if returncode is None:
            return None
        return self._finish(returncode)

    def verdict_lines(self):
        try:
            text = self.log.read_text(encoding="utf-8", errors="replace")
        except OSError:
            return []
        return [line.rstrip() for line in text.splitlines()
                if any(needle in line for needle in INTERESTING)]


def report(lane):
    status = "PASSED" if lane.returncode == 0 else "FAILED"
    print(f"\n=== {lane.preset}: {status} in {lane.elapsed:.2f} s "
          f"({lane.log})")
    for line in lane.verdict_lines():
        print(f"    {line}")
    if lane.returncode != 0:
        # The verdict lines are a summary, and a build failure produces none of
        # them at all. Show the tail so a failing lane is never silent.
        tail = lane.log.read_text(encoding="utf-8", errors="replace")
        for line in tail.splitlines()[-25:]:
            print(f"  | {line.rstrip()}")


# docs/recovery/ is the metadata every lane READS while the other lane runs. A
# test that writes there - even one that puts the file back - can be read
# mid-write by the other lane.
METADATA_PREFIX = "docs/recovery/"


def tracked_manifest(repository):
    """path -> (mtime_ns, size) for every git-tracked file.

    mtime, not content: the defect this catches restores what it wrote, so a
    content hash sees nothing. On 2026-08-01 a unit test appended one line to
    docs/recovery/functions.csv and put it back; `write_bytes` truncates first,
    so the OTHER preset lane read the file at 0 BYTES - measured 36 times in
    40 s - and died with "analysis correlation and canonical inventory counts
    differ". Nothing showed in `git status`, `git diff` or `git bisect`, and it
    cost three separate diagnoses.

    Known gap, stated rather than assumed away: a restore via shutil.copy2
    preserves mtime and would evade this.
    """
    try:
        listing = subprocess.run(["git", "ls-files", "-z"], cwd=repository,
                                 capture_output=True, text=True, check=True)
    except (OSError, subprocess.CalledProcessError):
        # Degrade loudly. Returning {} silently would compare nothing against
        # nothing and report a clean tree on every run, which is the failure
        # this whole file exists to stop.
        print("note: cannot list tracked files here, so the gate CANNOT tell "
              "whether\n      committed metadata was written while it ran.")
        return None
    manifest = {}
    for name in listing.stdout.split("\0"):
        if not name:
            continue
        try:
            info = (repository / name).stat()
        except OSError:
            continue
        manifest[name] = (info.st_mtime_ns, info.st_size)
    return manifest


def report_tree_writes(before, after):
    """Split verdict: metadata is a failure, everything else is a warning.

    Another agent legitimately edits src/ and tests/ while a gate runs here, so
    failing on those would make the gate flaky - and a flaky gate is worse than
    no check. Nothing should be touching docs/recovery/ mid-gate.
    """
    if before is None or after is None:
        return 0
    touched = sorted(name for name, stamp in after.items()
                     if name in before and before[name] != stamp)
    if not touched:
        return 0
    metadata = [name for name in touched if name.startswith(METADATA_PREFIX)]
    other = [name for name in touched if not name.startswith(METADATA_PREFIX)]
    if other:
        print(f"\nnote: {len(other)} tracked file(s) changed during the run "
              f"(another agent editing is normal):")
        for name in other[:5]:
            print(f"    {name}")
        if len(other) > 5:
            print(f"    ... and {len(other) - 5} more")
    if metadata:
        print(f"\nSOURCE TREE WRITTEN DURING THE GATE: {len(metadata)} file(s) "
              "under docs/recovery/ changed\nwhile the lanes were reading them. "
              "A lane can read one of these mid-write - at\nzero bytes - and "
              "fail with an error that names metadata and points nowhere.\n"
              "Restoring the file afterwards does not help: this compares "
              "mtime, and\n`git status` will look clean.")
        for name in metadata:
            print(f"    {name}")
        return 1
    return 0


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--preset", action="append", default=[],
                        help=f"repeatable; defaults to {' and '.join(PRESETS)}")
    parser.add_argument("--target", default=TARGET)
    parser.add_argument("--serial", action="store_true",
                        help="run the presets one after the other, for a "
                             "machine that cannot afford two lanes")
    parser.add_argument("--repository", default=str(Path(__file__).resolve().parent.parent))
    parser.add_argument("--log-dir", default=None,
                        help="defaults to <repository>/build")
    arguments = parser.parse_args()

    presets = arguments.preset or list(PRESETS)
    repository = Path(arguments.repository).resolve()
    log_directory = Path(arguments.log_dir) if arguments.log_dir \
        else repository / "build"

    lanes = [Lane(preset, arguments.target, log_directory, repository)
             for preset in presets]
    before = tracked_manifest(repository)
    started = time.monotonic()
    if arguments.serial or len(lanes) == 1:
        for lane in lanes:
            lane.start()
            lane.wait()
            report(lane)
    else:
        for lane in lanes:
            lane.start()
        pending = list(lanes)
        while pending:
            for lane in list(pending):
                if lane.poll() is not None:
                    report(lane)
                    pending.remove(lane)
            if pending:
                time.sleep(POLL_SECONDS)
    wall = time.monotonic() - started

    wrote_metadata = report_tree_writes(before, tracked_manifest(repository))

    failed = [lane.preset for lane in lanes if lane.returncode != 0]
    mode = "serially" if (arguments.serial or len(lanes) == 1) else "concurrently"
    total = sum(lane.elapsed for lane in lanes)
    print(f"\n{len(lanes)} lane(s) {mode}: wall {wall:.2f} s, "
          f"lane time {total:.2f} s")
    if wrote_metadata and not failed:
        # Green lanes over a tree that moved underneath them is exactly the
        # combination that reads as success and is not.
        print("GATE FAILED: the lanes passed, but committed metadata was "
              "written while they ran")
        return 1
    if failed:
        # Named, not counted: "1 of 2 failed" sends the reader to the wrong log.
        print(f"GATE FAILED: {', '.join(failed)}")
        return 1
    print(f"GATE PASSED: {', '.join(lane.preset for lane in lanes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
