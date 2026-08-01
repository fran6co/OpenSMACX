#!/usr/bin/env python3
"""Fail when AGENTS.md restates per-state recovery counts.

AGENTS.md used to list the six per-state function counts by hand. They rotted
twice: every figure was stale before 2026-07-29, and on 2026-08-01 the line
said 2,552 `source_complete` against an actual 2,573 and 2,808 `unrecovered`
against 2,787. The line's own defence was a sentence warning the reader it
might be wrong, which is not a defence.

The first version of this check compared the figures and demanded they match.
That was worse than it looked: EVERY recovery changes at least one of those
counts, so pinning them would have forced every recovery to edit AGENTS.md - a
file all of them share - which is precisely the coordination that worktree-
isolated parallel batches exist to remove. Trading a rotting number for a new
merge point is not a fix.

So the counts are gone from AGENTS.md, and this check keeps them gone. The
numbers live in docs/recovery/summary.json, which is generated and already
gate-checked, and in tools/recovery_metrics.py.
"""
import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DOC = REPO_ROOT / "AGENTS.md"

# "2,552 `source_complete`" / "28 `thunk`"
# A leading digit is required: `source_complete`, `external_library` in prose
# otherwise parses the separating comma as the number.
COUNT_RE = re.compile(r"(\d[\d,]*)\s+`([a-z_]+)`")
STATES = {"source_complete", "original_dependency", "source_in_progress",
          "external_library", "thunk", "unrecovered"}


def restated_counts(doc_path):
    """[(state, number)] for every per-state count restated in the document.

    A count is a number immediately followed by a backticked state name - the
    shape the removed line used. Prose may name a state, and may quote a
    number; only the two adjacent are a restatement of generated data.
    """
    text = doc_path.read_text(encoding="utf-8")
    found = []
    for number, state in COUNT_RE.findall(text):
        if state in STATES:
            found.append((state, int(number.replace(",", ""))))
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--doc", type=Path, default=DEFAULT_DOC)
    arguments = parser.parse_args()

    if not arguments.doc.is_file():
        print(f"documented-counts: {arguments.doc} is absent, so this check "
              "verified NOTHING", file=sys.stderr)
        return 1

    restated = restated_counts(arguments.doc)
    if restated:
        print(f"documented-counts: {arguments.doc.name} restates "
              f"{len(restated)} per-state recovery count(s). Do not copy "
              "generated numbers here:\n  every recovery changes at least one "
              "of them, so this file would have to be edited by\n  every "
              "recovery - and it is shared, so that is a merge point for every "
              "parallel\n  batch. It also rotted twice when it was here "
              "before.\n  Use tools/recovery_metrics.py or "
              "docs/recovery/summary.json.", file=sys.stderr)
        for state, number in restated:
            print(f"    {number:,} `{state}`", file=sys.stderr)
        return 1

    # Not a vacuous pass: the document must exist and be substantial, or a
    # truncated file would report clean forever.
    size = arguments.doc.stat().st_size
    if size < 1000:
        print(f"documented-counts: {arguments.doc} is only {size} B; refusing "
              "to report clean over a file that may be truncated.",
              file=sys.stderr)
        return 1

    print(f"documented-counts: {arguments.doc.name} restates no generated "
          f"per-state count ({size:,} B scanned)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
