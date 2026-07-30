#!/usr/bin/env python3
"""Drive the lifted-image boot and report whether the stop address MOVED.

Task #32's loop is: boot, read where it stopped, build that, boot again. The
failure mode of running that by hand is not tedium, it is a no-op read as
progress - this project has been caught twice by a change that measured
identically to no change, and both times the tell was a number that did not move
when it should have. So the one thing this tool insists on saying is whether the
stop address is the SAME as last time.

Two modes, and the difference is the strength of the evidence:

  default    The boot stops at its first trap. `BOOT-STOPPED-AT <addr>` is the
             next work item, and it is a real statement about how far the image
             gets.
  --survey   A named-but-unimplemented CRT routine returns 0 to its caller
             instead of trapping, so one run enumerates the whole reachable
             frontier as `SURVEY-REACHED` lines. That is a WORK QUEUE and not a
             boot: every skipped routine returned a lie, so execution past the
             first skip is not the program's. The two verdicts are spelled apart
             for exactly this reason and this tool never totals them together.

Writes the frontier to a TSV so the queue survives the run, and prints nothing
that could be mistaken for a claim the game booted.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_BOOT = REPO_ROOT / "build" / "lifted" / "lifted_boot.exe"
DEFAULT_FRONTIER = REPO_ROOT / "build" / "lifted" / "boot-frontier.tsv"
DEFAULT_STATE = REPO_ROOT / "build" / "lifted" / "boot-loop-state.json"

STOPPED = re.compile(r"^BOOT-STOPPED-AT\s+(0x[0-9A-Fa-f]+)", re.M)
SURVEYED = re.compile(r"^SURVEY-REACHED\s+(0x[0-9A-Fa-f]+)\s+(\S+)", re.M)
RANGE = re.compile(
    r"^boot: \.CRT\$(\S+) \[(0x[0-9A-Fa-f]+),(0x[0-9A-Fa-f]+)\) ran (\d+), "
    r"(\d+) catalogued", re.M)


def run_boot(executable: Path, wine: str | None, survey: bool,
             extra: list[str], timeout: float) -> str:
    command: list[str] = []
    if wine:
        command.append(wine)
    command.append(str(executable))
    command += ["--winmain", "--init"]
    if survey:
        command.append("--survey")
    command += extra
    try:
        result = subprocess.run(command, capture_output=True, text=True,
                                timeout=timeout)
    except subprocess.TimeoutExpired as expired:
        # A hang is a result: it means the boot got somewhere it could loop,
        # which is different from stopping and must not be reported as a stop.
        captured = expired.stdout or b""
        text = captured.decode("utf-8", "replace") if isinstance(
            captured, bytes) else str(captured)
        return text + "\nBOOT-TIMED-OUT\n"
    return result.stdout + result.stderr


def parse(text: str) -> dict:
    stopped = STOPPED.search(text)
    return {
        "stopped_at": stopped.group(1).lower() if stopped else None,
        "timed_out": "BOOT-TIMED-OUT" in text,
        "surveyed": [(address.lower(), name)
                     for address, name in SURVEYED.findall(text)],
        "ranges": [{"section": section, "begin": begin, "end": end,
                    "ran": int(ran), "catalogued": int(catalogued)}
                   for section, begin, end, ran, catalogued
                   in RANGE.findall(text)],
    }


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--boot", type=Path, default=DEFAULT_BOOT)
    parser.add_argument("--wine", default="wine")
    parser.add_argument("--survey", action="store_true")
    parser.add_argument("--frontier", type=Path, default=DEFAULT_FRONTIER)
    parser.add_argument("--state", type=Path, default=DEFAULT_STATE,
                        help="where the previous stop address is remembered")
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--log", type=Path)
    parser.add_argument("extra", nargs="*",
                        help="further arguments passed to the boot")
    args = parser.parse_args(argv)

    if not args.boot.is_file():
        print(f"error: {args.boot} is missing; run build/lifted/build.sh, which "
              f"builds it beside lifted_probe.exe", file=sys.stderr)
        return 2

    text = run_boot(args.boot, args.wine, args.survey, args.extra, args.timeout)
    if args.log:
        args.log.write_text(text, encoding="utf-8")
    found = parse(text)

    for entry in found["ranges"]:
        print(f"CRT${entry['section']}: ran {entry['ran']}, "
              f"{entry['catalogued']} catalogued ??__E")

    if found["surveyed"]:
        args.frontier.parent.mkdir(parents=True, exist_ok=True)
        with args.frontier.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
            writer.writerow(["address", "name"])
            writer.writerows(sorted(set(found["surveyed"])))
        distinct = len(set(found["surveyed"]))
        print(f"SURVEY: {distinct} distinct CRT routine(s) on the boot path "
              f"-> {args.frontier}")
        print("SURVEY is a WORK QUEUE, not a boot: every one returned 0, so "
              "execution past the first is not the program's.")

    previous = {}
    if args.state.is_file():
        previous = json.loads(args.state.read_text(encoding="utf-8"))

    if found["timed_out"]:
        print("BOOT TIMED OUT - it reached something that loops, which is not "
              "the same as stopping")
    elif found["stopped_at"] is None:
        print("no stop address was reported; the boot neither stopped nor timed "
              "out, so the run says nothing")
    else:
        print(f"STOPPED AT {found['stopped_at']}")
        # THE POINT OF THIS TOOL. A change that leaves the stop address where it
        # was has bought nothing, and saying so is the only way that reads as a
        # non-result rather than as a run that happened.
        if previous.get("stopped_at") == found["stopped_at"]:
            print(f"UNCHANGED from the previous run - whatever was built since "
                  f"did not move the boot. This is a NO-OP, not progress.")
        elif previous.get("stopped_at"):
            print(f"MOVED from {previous['stopped_at']}")

    args.state.parent.mkdir(parents=True, exist_ok=True)
    args.state.write_text(json.dumps(found, indent=2, sort_keys=True) + "\n",
                          encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
