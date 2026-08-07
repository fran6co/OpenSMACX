#!/usr/bin/env python3
"""Structural facts a recovery turned up, recorded where they can be acted on.

Recovering a function means reading its disassembly closely enough to
reproduce it byte for byte, and that produces knowledge the FUNCTION does not
need: a field at `+0x6C` that holds a flag byte, a table of 512 three-word
records at `+0xA20`, a receiver that is three floats and therefore a vector, a
vtable slot that takes an `int` where the emitter guessed nullary, a callee
reached by a tail `jmp` that the call graph never recorded.

None of that was going anywhere. The agent reported it in prose, the prose
went into a findings file or a commit message, and the next agent to touch the
same class rediscovered it. Meanwhile the repository already has a family of
member-evidence catalogues - `idb-members.csv`, `pracx-members.csv`,
`thinker-members.csv`, `access-lower-bounds.csv`, `vtables.csv` - all of them
fed from EXTERNAL sources, and none from the one source that has actually
proved something about the bytes.

This is that catalogue. It is deliberately an OBSERVATION log, not a layout:
each row says what was seen, where, and what proves it, and carries a status
so that acting on it is a separate, reviewable step. A row here is evidence,
not a decision - promoting one into `hypothesis_layouts.h`, into
`catalogue_corrections.py`, or into an emitter fix is done by a person looking
at it, which is the same rule every other evidence catalogue here follows.

    tools/structure_observations.py --list
    tools/structure_observations.py --list --kind vtable-slot
    tools/structure_observations.py --check
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
OBSERVATIONS = REPO_ROOT / "docs" / "recovery" / "agent-structure-observations.csv"

FIELDS = ("address", "kind", "subject", "offset", "detail", "evidence", "status")

# What an observation can be about. Kept short on purpose: a vocabulary that
# grows per-observation stops being groupable, and grouping is the whole point
# of writing them down rather than leaving them in prose.
KINDS = {
    "member":       "a field exists at an offset in a class",
    "member-type":  "a field's width or signedness is visible",
    "class":        "a type exists that the tree does not declare",
    "embedded":     "one class contains another at an offset",
    "vtable-slot":  "a virtual slot index, and its arity when a call shows it",
    "table":        "a repeated record: stride, count and element shape",
    "callgraph":    "an edge the call graph does not carry",
    "emitter":      "the scaffolding is wrong or insufficient, with the symptom",
}

# `open` is the default. `promoted` means it has been acted on and where.
# `rejected` means it was looked at and is not true; keeping it stops the same
# observation being re-filed by the next agent that sees the same bytes.
STATUSES = {"open", "promoted", "rejected"}


class Invalid(Exception):
    """A row that cannot be acted on later is not worth recording now."""


def load() -> list:
    if not OBSERVATIONS.is_file():
        return []
    with OBSERVATIONS.open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def validate(rows: list) -> None:
    """Every row names a kind, a subject and its evidence, or it is refused.

    EVIDENCE IS THE REQUIRED FIELD. An observation without it is a guess with
    a citation format, and this catalogue sits next to ones fed from external
    analysis whose whole value is that a reader can tell the difference.
    """
    for index, row in enumerate(rows, start=2):
        where = f"row {index} ({row.get('address', '?')})"
        if set(row) - set(FIELDS):
            raise Invalid(f"{where}: unexpected column(s) "
                          f"{sorted(set(row) - set(FIELDS))}")
        if row.get("kind") not in KINDS:
            raise Invalid(f"{where}: kind {row.get('kind')!r} is not one of "
                          f"{sorted(KINDS)}")
        if row.get("status") not in STATUSES:
            raise Invalid(f"{where}: status {row.get('status')!r} is not one "
                          f"of {sorted(STATUSES)}")
        if not (row.get("subject") or "").strip():
            raise Invalid(f"{where}: no subject")
        if not (row.get("evidence") or "").strip():
            raise Invalid(f"{where}: no evidence. An observation without it "
                          f"is a guess in citation format.")
        address = (row.get("address") or "").strip()
        if address and not address.lower().startswith("0x"):
            raise Invalid(f"{where}: address {address!r} is not hex")
        # `-` means "not applicable" and is what a person writes in a table.
        # Refusing it made three separate agents' rows fail validation for a
        # reason that has nothing to do with whether the observation is sound.
        offset = (row.get("offset") or "").strip()
        if offset == "-":
            offset = ""
        # A NEGATIVE offset is allowed and is the most informative kind there
        # is: `[ecx-0x1c]` means the declared class is a SUBOBJECT inside a
        # larger one, which is inheritance the declaration does not model.
        # Rejecting them, as this first did, throws away the evidence that
        # matters most.
        if offset and not offset.lstrip("-").lower().startswith("0x"):
            raise Invalid(f"{where}: offset {offset!r} is not hex")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--kind", help="only this kind")
    parser.add_argument("--status", help="only this status")
    parser.add_argument("--check", action="store_true",
                        help="validate and say nothing when the file is sound")
    arguments = parser.parse_args(argv)

    rows = load()
    try:
        validate(rows)
    except Invalid as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    if arguments.check:
        return 0

    shown = [r for r in rows
             if (not arguments.kind or r["kind"] == arguments.kind)
             and (not arguments.status or r["status"] == arguments.status)]
    for row in shown:
        head = f"{row['address'] or '-':10} {row['kind']:12} {row['subject']}"
        if row["offset"]:
            head += f" @{row['offset']}"
        print(f"{head}\n    {row['detail']}\n    evidence: {row['evidence']}"
              f"   [{row['status']}]")
    print(f"\n{len(shown)} of {len(rows)} observation(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
