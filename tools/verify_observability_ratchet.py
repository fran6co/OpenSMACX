#!/usr/bin/env python3
"""A new recovery must prove something observes it. The old ones need not.

141 of 193 recovered functions of >=200 B are observed by nothing - no assertion
distinguishes the body from a wrecked one - and 131 of those have no oracle and
no proof either, 96,550 B in total. That is a real number and it is not a reason
to stop: writing 131 retroactive fixtures would halt the objective, and the
project's own decision is that proof is opportunistic and never a gate.

What is NOT acceptable is the number growing quietly, which is exactly what has
been happening. So this is a ratchet, the same shape as BASELINE in
audit_export_signedness.py:

  * every `source_complete` function of >=200 B must appear in the census;
  * a function ABSENT from it fails - that is what a new recovery hits, and the
    failure text says how to measure it;
  * the UNOBSERVED total may fall and must never rise.

WHY THIS COSTS NOTHING TO RUN. It compares two committed files and builds
nothing. The measurement it rests on is expensive - one build per function,
about 30 minutes over the corpus - so it is recorded rather than recomputed. A
recovery only has to measure ITS OWN function, which is one build.

THE HONEST LIMIT. The census is a committed artifact, so it states what someone
measured, not what is true now. Editing a verdict from UNOBSERVED to OBSERVED
would satisfy this check while proving nothing. It is a ratchet against drift,
not a proof of coverage - the proof is `tools/measure_observability.py`, and the
census is reviewable precisely so a hand-edited verdict shows up in a diff.
"""
import argparse
import csv
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CENSUS = REPO_ROOT / "docs" / "recovery" / "observability.json"
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"

UNOBSERVED = "UNOBSERVED"
NOT_MEASURED = "NOT-MEASURED"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--census", type=Path, default=DEFAULT_CENSUS)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--floor", type=int, default=150,
                        help="refuse to report clean over fewer recorded "
                             "functions than this")
    arguments = parser.parse_args()

    if not arguments.census.is_file():
        print(f"observability-ratchet: {arguments.census} is absent, so this "
              f"check verified NOTHING", file=sys.stderr)
        return 1
    census = json.loads(arguments.census.read_text(encoding="utf-8"))
    recorded = census.get("functions", {})
    minimum = int(census.get("min_size", 200))
    baseline = int(census.get("baseline_unobserved", 0))

    if len(recorded) < arguments.floor:
        print(f"observability-ratchet: the census holds {len(recorded)} "
              f"functions, below the floor of {arguments.floor}. A truncated "
              f"census would let every recovery through.", file=sys.stderr)
        return 1

    with arguments.functions.open(newline="", encoding="utf-8-sig") as handle:
        live = {row["address"]: row for row in csv.DictReader(handle)
                if row["recovery_state"] == "source_complete"
                and row["binary_kind"] == "game"
                and int(row["size"]) >= minimum}

    missing = sorted(set(live) - set(recorded),
                     key=lambda a: -int(live[a]["size"]))
    if missing:
        print(f"observability-ratchet: {len(missing)} recovered function(s) of "
              f">={minimum} B are not in the census, so nothing here shows "
              f"anything observes them:", file=sys.stderr)
        for address in missing[:10]:
            print(f"    {address}  {live[address]['size']:>6} B  "
                  f"{live[address]['name']}", file=sys.stderr)
        print(f"  Measure yours with:\n"
              f"    tools/measure_observability.py --sources <its .cpp> "
              f"--min-size {minimum}\n"
              f"  then add the verdict to {arguments.census.name}. A new "
              f"recovery is expected to come back OBSERVED; today's three do, "
              f"at 93-94% of mutants killed.", file=sys.stderr)
        return 1

    # NOT-MEASURED is ratcheted too, and that is not pedantry. 25 of the 26 are
    # in sources compiled into more than one test executable, so the prober
    # cannot pick a target and correctly refuses to guess - those functions may
    # well be observed by the leaf suite. Recording them as UNOBSERVED would be
    # a lie; leaving them unratcheted would make "put it in a multi-target file"
    # the way to land a recovery nothing checks.
    unmeasured_baseline = int(census.get("baseline_unmeasured", 0))
    for verdict, seen, floor, name in (
            (UNOBSERVED,
             [a for a, e in recorded.items() if e.get("verdict") == UNOBSERVED],
             baseline, "baseline_unobserved"),
            (NOT_MEASURED,
             [a for a, e in recorded.items() if e.get("verdict") == NOT_MEASURED],
             unmeasured_baseline, "baseline_unmeasured")):
        if len(seen) > floor:
            print(f"observability-ratchet: {verdict} recoveries rose from "
                  f"{floor} to {len(seen)}. The historical debt may be paid "
                  f"down; it must not grow.", file=sys.stderr)
            return 1
        if len(seen) < floor:
            print(f"observability-ratchet: {verdict} fell {floor} -> "
                  f"{len(seen)}. Lower {name} in {arguments.census.name} to "
                  f"lock the gain in.", file=sys.stderr)
            return 1

    observed = sum(1 for e in recorded.values()
                   if e.get("verdict") == "OBSERVED")
    print(f"observability-ratchet: {len(recorded)} recorded, {observed} "
          f"observed, {baseline} unobserved, {unmeasured_baseline} "
          f"unmeasured; {len(live)} live functions all present")
    return 0


if __name__ == "__main__":
    sys.exit(main())
