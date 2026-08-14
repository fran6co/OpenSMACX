#!/usr/bin/env python3
"""Fail when a mangled name and its prototype disagree about returning a value.

Every catalogued row carries TWO independent readings of the same function: a
mangled name, whose return type is a single letter, and an IDA prototype
derived separately. The image has no symbols, so both are analysis and either
can be wrong - but when they disagree about whether anything comes back at
all, one of them IS wrong, and the emitter has to pick. It picks the name,
because the name has to outrank the prototype's SPELLING (see
`catalogue_corrections`, which explains why at length), so a name that spells
`X` makes the scaffold declare `void` and every caller in the tree loses the
ability to test the result.

That is not a byte-ordering divergence, it is a control-flow one: a caller
that cannot capture a failure code cannot early-return on it.

MEASURED 2026-08-14 over 2,984 rows carrying both readings. Twelve disagreed.
Five were the name being wrong and are now corrected - `GraphicWin::init`,
whose 47 callers inherited `void`, and four `Time` methods that end by
computing 1 or 2 into EAX. Three are the PROTOTYPE being wrong and are pinned
below. The remaining four were already corrected before this check existed.

WHY A PINNED SET RATHER THAN A CLEAN ZERO. A disagreement does not say which
side is wrong, and this tree has both directions in it. Resolving every one
towards the name would have renamed three functions that really do return
nothing. So the check demands ADJUDICATION, not agreement: every disagreement
is either corrected in `catalogue_corrections` or named here with the bytes
that settle it. An unadjudicated one fails.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402

# address: why the NAME is right and the prototype's `int` is IDA's guess.
# Each was measured the same way the corrections were: does the body write EAX
# on the way out, and does any caller test the result?
SETTLED_IN_FAVOUR_OF_THE_NAME = {
    0x0042A040: (
        "?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z",
        "the tail is `mov dword ptr [eax], ecx; pop ebp; ret 0xc` - EAX holds "
        "the out-parameter being written THROUGH, loaded earlier by `mov eax, "
        "dword ptr [ebp + 0x10]`, and is not a returned value. No caller tests "
        "the result"),
    0x00481DA0: (
        "?fill_player@NetWin@@QAEXPAUPlayerData@@H@Z",
        "the body writes EAX nowhere on the way out and no caller tests the "
        "result"),
    0x0062ACD0: (
        "?decode_chunk@Flic@@QAEXPAUDataChunk@@@Z",
        "the body writes EAX nowhere on the way out and no caller tests the "
        "result"),
}

PROTOTYPE_RETURN = re.compile(r"\s*([A-Za-z_][\w \*]*?)\s*\(")


def readings(row: dict):
    """(name's return type, prototype's return type), or None if not scorable."""
    name, prototype = row.get("name") or "", row.get("prototype") or ""
    if not name or not prototype:
        return None
    try:
        returns, _ = emit.decode_signature(name)
    except Exception:                      # noqa: BLE001 - an undecodable name
        return None                        # is not this check's business
    hit = PROTOTYPE_RETURN.match(prototype)
    if not returns or not hit:
        return None
    return returns.strip(), hit.group(1).strip()


def faults(rows: dict = None) -> tuple:
    """(unadjudicated disagreements, number of rows actually scored)."""
    rows = emit.load_functions() if rows is None else rows
    found, scored = [], 0
    for address, row in sorted(rows.items()):
        pair = readings(row)
        if pair is None:
            continue
        scored += 1
        from_name, from_prototype = pair
        if (from_name == "void") == (from_prototype == "void"):
            continue
        if address in SETTLED_IN_FAVOUR_OF_THE_NAME:
            continue
        found.append(
            f"0x{address:08X} {row.get('name', '')}: the name returns "
            f"`{from_name}` and the prototype returns `{from_prototype}`. One "
            f"is wrong and the emitter follows the name - correct it in "
            f"catalogue_corrections, or pin it here with the bytes that show "
            f"the name is right")
    return found, scored


def main(argv=None) -> int:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    # `--src` EXISTS SO THE DAMAGE CASE RUNS THIS EXACT TOOL, for the reason
    # `verify_span_termination` gives: a proof that a check can fail is worth
    # nothing if it fails a different invocation than the gate makes.
    parser.add_argument("--src", help="annotation tree to read instead of src/")
    arguments = parser.parse_args(argv)
    rows = None
    if arguments.src:
        import project_catalogue
        rows = project_catalogue.from_source(Path(arguments.src))
    found, scored = faults(rows)
    for line in found:
        print(f"  {line}")
    # THE POPULATION IS REPORTED, so a reader that quietly matches nothing
    # cannot present an empty sweep as a clean one. This file exists in a tree
    # that has published a vacuous proof before, and the first version of this
    # very measurement returned "0 disagreements" because it read the decoder's
    # return value as an object when the decoder returns a tuple - it had
    # skipped all 2,984 rows.
    print(f"return-agreement: {len(found)} unadjudicated disagreement(s) "
          f"over {scored} row(s) carrying both readings, "
          f"{len(SETTLED_IN_FAVOUR_OF_THE_NAME)} pinned")
    # THE FLOOR IS FOR THE GATE'S OWN INVOCATION ONLY. A damaged tree is one
    # file by construction, so applying the floor to `--src` would make every
    # damage case "fail" on the population rather than on the defect it
    # planted - a positive control that proves the floor works and says
    # nothing about the check. This repository has published one of those.
    if arguments.src is None and scored < 2000:
        print(f"REFUSED: only {scored} rows carried both a name and a "
              f"prototype, below the pinned 2000; the reader has lost most of "
              f"its population and a clean sweep would mean nothing")
        return 1
    return 1 if found else 0


if __name__ == "__main__":
    raise SystemExit(main())
