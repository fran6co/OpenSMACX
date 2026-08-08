#!/usr/bin/env python3
"""Which addresses in a wave were actually WORKED, regardless of what was claimed.

`byte_match_fanout.py --collect` already refuses to trust an agent's reported
tier: it recompiles the unit and records what it measures. That catches a
wrong claim about a body. It does not catch the cheaper failure, because
there is nothing to recompile - an agent that never opened the file at all,
and reported BYTE_EXACT anyway.

Measured on wave 3: 101 addresses were claimed BYTE_EXACT, and 43 of them
still held `// BODY GOES HERE.` in a file last modified six days before the
wave started. The ledger had those 43 as MISMATCH or NO_COMPILE. Nothing
false reached the ledger - collect never saw a body to score - but the wave
reported itself as 84% successful when it was at most 48%, and the 43 would
have been quietly dropped from the queue as "done".

So the file is the witness. A unit that still carries the placeholder was not
worked, whatever the report said, and a unit whose mtime predates the wave
was not worked in that wave.

    tools/verify_wave.py 0xAAA 0xBBB ...        # addresses on the command line
    tools/verify_wave.py --file targets.json    # or a {group: [address]} map
    tools/verify_wave.py --since 2026-08-08T10:00  # require work after this
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
WORK_ROOT = REPO_ROOT / "build" / "byte-match"
LEDGER = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"
PLACEHOLDER = "// BODY GOES HERE."


def ledger_tiers() -> dict:
    if not LEDGER.is_file():
        return {}
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        return {row["address"].upper(): row.get("tier", "")
                for row in csv.DictReader(handle)}


def is_untouched(text: str) -> bool:
    """Placeholder present AND nothing written where the body belongs.

    The placeholder alone is not proof of an untouched unit. An agent found
    one whose body had been appended BELOW the `// BODY GOES HERE.` line
    instead of replacing it - real work that this would have called untouched
    and thrown back into the queue. So the test is what follows the marker,
    not whether the marker survived.
    """
    if PLACEHOLDER not in text:
        return False
    after = text.split(PLACEHOLDER, 1)[1]
    code = [line for line in after.splitlines()
            if line.strip() and not line.lstrip().startswith("//")]
    # The emitter's own closing brace is one line and is not a body.
    return len(code) <= 1


def state_of(address: int, since: dt.datetime | None) -> str:
    unit = WORK_ROOT / f"{address:08x}" / "unit.cpp"
    if not unit.is_file():
        return "no-unit"
    if is_untouched(unit.read_text()):
        return "untouched"
    if since is not None:
        stamp = dt.datetime.fromtimestamp(unit.stat().st_mtime)
        if stamp < since:
            return "stale"          # a body, but not one this wave wrote
    return "worked"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("addresses", nargs="*")
    parser.add_argument("--file", help="JSON list, or {group: [address]} map")
    parser.add_argument("--since", help="ISO timestamp the wave started")
    parser.add_argument("--list", action="store_true",
                        help="print every address, not just the summary")
    arguments = parser.parse_args(argv)

    wanted = list(arguments.addresses)
    if arguments.file:
        loaded = json.loads(Path(arguments.file).read_text())
        if isinstance(loaded, dict):
            wanted += [a for group in loaded.values() for a in group]
        else:
            wanted += list(loaded)
    if not wanted:
        parser.error("give addresses or --file")

    since = dt.datetime.fromisoformat(arguments.since) if arguments.since \
        else None
    tiers = ledger_tiers()

    rows = []
    for text in wanted:
        address = int(text, 16)
        rows.append((address, state_of(address, since),
                     tiers.get(f"0X{address:08X}", "")))

    counts = Counter(state for _a, state, _t in rows)
    print(f"{len(rows)} address(es): " +
          ", ".join(f"{state} {n}" for state, n in sorted(counts.items())))
    measured = Counter(tier or "(no row)" for _a, state, tier in rows
                       if state == "worked")
    if measured:
        print("worked units, tier the HARNESS measured: " +
              ", ".join(f"{t} {n}" for t, n in measured.most_common()))

    if arguments.list or counts.get("untouched") or counts.get("stale"):
        for address, state, tier in rows:
            if arguments.list or state in ("untouched", "stale", "no-unit"):
                print(f"  0x{address:08X}  {state:<9}  {tier}")
    # An untouched address is not a failure of this tool; it is work still to
    # do. Exit non-zero so a wave runner notices instead of moving the queue on.
    return 1 if counts.get("untouched") or counts.get("no-unit") else 0


if __name__ == "__main__":
    raise SystemExit(main())
