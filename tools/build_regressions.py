#!/usr/bin/env python3
"""Bodies that gave up byte-exactness so the tree would compile.

A recovered body has two jobs and they can conflict. It must reproduce the
original's bytes, and it must compile in `src/` against the project's real
headers. Most of the time one body does both. Sometimes making it compile
means changing the shape that matched.

THAT TRADE IS ALLOWED. A proof is re-derivable - the verifier can re-measure
any body at any time - and a tree that does not build is worth less than one
that does. What is NOT allowed is making the trade invisibly.

Without this file the only way to land such a change is to lower the ratchet
floor, which is a number moving down in a source file with no record of WHICH
functions regressed, or why, or whether anyone meant to. That is precisely how
122 proofs came to exist only as gitignored build artefacts while the ledger
went on counting them: not a lie, just nobody's job to notice.

So a regression is recorded here, per address, with the tier it held, the tier
it holds now, and the reason. `byte_match_fanout.py --check` reads it, so the
floor can stay where it is and the debt stays legible - and `--list` prints
exactly what is owed.

    tools/build_regressions.py --list
    tools/build_regressions.py --check
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
LEDGER = REPO_ROOT / "docs" / "recovery" / "build-regressions.csv"

FIELDS = ("address", "name", "was_tier", "now_tier", "reason", "recorded_by")

# A body may only be traded DOWN from one of these. Anything else was never a
# proof, so giving it up costs nothing and does not belong here.
PROVEN = ("BYTE_EXACT", "SHAPE_EXACT")


class Invalid(Exception):
    """A row that does not describe a real, recoverable debt."""


def load() -> list:
    if not LEDGER.is_file():
        return []
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def validate(rows: list) -> None:
    """Every row names what was lost and why, or it is refused.

    THE REASON IS THE REQUIRED FIELD. "it did not compile" is not a reason -
    the whole file is about things that did not compile. What has to be here
    is what specifically could not be reconciled, so that whoever picks the
    function back up starts where the last person stopped instead of
    rediscovering it.
    """
    for index, row in enumerate(rows, start=2):
        where = f"row {index} ({row.get('address', '?')})"
        if set(row) - set(FIELDS):
            raise Invalid(f"{where}: unexpected column(s) "
                          f"{sorted(set(row) - set(FIELDS))}")
        address = (row.get("address") or "").strip()
        if not address.lower().startswith("0x"):
            raise Invalid(f"{where}: address {address!r} is not hex")
        if row.get("was_tier") not in PROVEN:
            raise Invalid(f"{where}: was_tier {row.get('was_tier')!r} was never "
                          f"a proof; only {list(PROVEN)} can be traded away")
        if row.get("now_tier") in PROVEN:
            raise Invalid(f"{where}: now_tier {row.get('now_tier')!r} is still "
                          f"a proof, so nothing was lost and nothing is owed")
        reason = (row.get("reason") or "").strip()
        if len(reason) < 20:
            raise Invalid(
                f"{where}: the reason is missing or too short. Say what could "
                f"not be reconciled, not that it did not compile.")


def owed() -> tuple:
    """(functions, bytes) the ratchet should forgive, and the rows."""
    rows = load()
    validate(rows)
    return len(rows), rows


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--check", action="store_true",
                        help="validate and say nothing when the file is sound")
    arguments = parser.parse_args(argv)

    try:
        rows = load()
        validate(rows)
    except Invalid as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    if arguments.check:
        return 0
    if not rows:
        print("no body has been traded down for the build")
        return 0
    print(f"{len(rows)} body(ies) gave up a proof so the tree would compile:\n")
    for row in rows:
        print(f"  {row['address']}  {row['was_tier']} -> {row['now_tier']}"
              f"  {row['name']}")
        print(f"      {row['reason']}")
    print(f"\nEach is re-derivable. `verify_recovered_function.py <addr>` "
          f"re-measures one.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
