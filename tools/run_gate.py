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

    failed = [lane.preset for lane in lanes if lane.returncode != 0]
    mode = "serially" if (arguments.serial or len(lanes) == 1) else "concurrently"
    total = sum(lane.elapsed for lane in lanes)
    print(f"\n{len(lanes)} lane(s) {mode}: wall {wall:.2f} s, "
          f"lane time {total:.2f} s")
    if failed:
        # Named, not counted: "1 of 2 failed" sends the reader to the wrong log.
        print(f"GATE FAILED: {', '.join(failed)}")
        return 1
    print(f"GATE PASSED: {', '.join(lane.preset for lane in lanes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
