#!/usr/bin/env python3
"""Fail when a hand-typed count in AGENTS.md disagrees with the generated data.

AGENTS.md carries a "Current recovery state" line listing per-state function
counts. It is typed by hand, so it rots, and the line itself admits as much:
"These counts drift ... every figure on this line was wrong until 2026-07-29,
five of them by more than 60%." Writing the warning down did not stop it - on
2026-08-01 the line said 2,552 `source_complete` against an actual 2,573, and
2,808 `unrecovered` against 2,787.

A note telling the reader the number may be wrong is not a substitute for the
number being right. This compares the documented figures against
docs/recovery/functions.csv, which is generated, and names every disagreement.

Deliberately NOT solved by deleting the counts. They are load-bearing context -
a reader sizing the remaining work needs them in the file they are already
reading - so they stay, and drift becomes a failing test instead of a caveat.
"""
import argparse
import csv
import re
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DOC = REPO_ROOT / "AGENTS.md"
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# "2,552 `source_complete`" / "28 `thunk`"
COUNT_RE = re.compile(r"([\d,]+)\s+`([a-z_]+)`")
STATES = {"source_complete", "original_dependency", "source_in_progress",
          "external_library", "thunk", "unrecovered"}


def documented_counts(doc_path):
    """The figures on the 'Current recovery state' line, by state."""
    text = doc_path.read_text(encoding="utf-8")
    start = text.find("Current recovery state:")
    if start < 0:
        return None
    # The claim runs to the end of the sentence that introduces it.
    block = text[start:start + 600]
    found = {}
    for number, state in COUNT_RE.findall(block):
        if state in STATES and state not in found:
            found[state] = int(number.replace(",", ""))
    return found


def actual_counts(functions_path):
    counts = Counter()
    with functions_path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            counts[row["recovery_state"]] += 1
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--doc", type=Path, default=DEFAULT_DOC)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    arguments = parser.parse_args()

    if not arguments.functions.is_file():
        print(f"documented-counts: {arguments.functions} is absent, so this "
              "check verified NOTHING", file=sys.stderr)
        return 1

    documented = documented_counts(arguments.doc)
    if not documented:
        print("documented-counts: no 'Current recovery state:' line found in "
              f"{arguments.doc}. If the line was removed deliberately, remove "
              "this check with it; otherwise it has been renamed and this "
              "check is now watching nothing.", file=sys.stderr)
        return 1
    # A parser that silently matched two of six states would pass forever.
    missing = sorted(STATES - set(documented))
    if missing:
        print(f"documented-counts: only found {len(documented)} of "
              f"{len(STATES)} states on that line; missing "
              f"{', '.join(missing)}. Not comparing a subset and calling it "
              "clean.", file=sys.stderr)
        return 1

    actual = actual_counts(arguments.functions)
    wrong = [(state, documented[state], actual[state])
             for state in sorted(documented) if documented[state] != actual[state]]
    if wrong:
        print("documented-counts: AGENTS.md disagrees with the generated "
              "docs/recovery/functions.csv:", file=sys.stderr)
        for state, said, is_ in wrong:
            print(f"    {state:<22} says {said:>6,}   actually {is_:>6,}",
                  file=sys.stderr)
        print("  Update the line, or make it say something that cannot rot.",
              file=sys.stderr)
        return 1

    print(f"documented-counts: {len(documented)} states agree with "
          f"{sum(actual.values()):,} catalogued functions")
    return 0


if __name__ == "__main__":
    sys.exit(main())
