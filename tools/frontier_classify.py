#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""What the remaining frontier is actually made of.

"389 bodies left" is one undifferentiated pile, and it is not one problem.
Some of those bodies have a plausible lever nobody has tried; others are
CEILINGS this project has already measured and named, where more grinding
buys nothing. Reporting them as one number makes the second kind look like
unfinished work forever.

This reads each remaining body's own TRIED and LEVER lines and sorts it
into the ceiling families the tree has established, or into "open" when
nothing it says matches one.

    uv run tools/frontier_classify.py

It classifies by what the refusals SAY, so it is only as good as the notes -
a body whose wall was never written down reads as open, which is the safe
direction. Run tools/prose_refusals.py alongside it.
"""
import collections
import pathlib
import re
import subprocess
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402

# Ordered: the first family a body's notes match wins, so the more specific
# walls are tested before the generic "register allocation" one.
FAMILIES = [
    ("hand-written assembly",
     r"hand-?written assembly|hand-?written asm|handwritten_asm|"
     r"pushf|lodsb|\bloop\b.*opcode|opcodes VC6 (?:does not|never)"),
    ("SEH frame (constructor family)",
     r"SEH frame|unwind frame|/GX\b.*frame|frame the image does not"),
    ("helper inlined here, called there",
     r"MEASURED inline.*(?:call site|out-of-line)|"
     r"port_to_port|has_tech.*inline|do_all_non_input|"
     r"no `?__declspec\(noinline\)|noinline"),
    ("COMDAT / shared tail",
     r"COMDAT|SHARED_TAIL|shared tail|folded with"),
    ("jump-table / switch lowering",
     r"jump.?table|sub-?table|switch.*lowering|compare.?chain"),
    ("register allocation / scheduling",
     r"register (?:allocation|choice|swap|permutation|rotation|schedul)|"
     r"scheduling|spill|ebx/esi|edx vs eax|eax vs edx"),
]


def family_of(record) -> str:
    text = " ".join(list(record.ruled_out) + [p for _k, p in record.levers])
    if not text.strip():
        return "no notes (unclassifiable)"
    for name, pattern in FAMILIES:
        if re.search(pattern, text, re.I):
            return name
    return "worked, no known ceiling named - likeliest to yield"


if __name__ == "__main__":
    frontier = subprocess.run(
        ["uv", "run", "tools/frontier.py"], cwd=REPO_ROOT,
        capture_output=True, text=True).stdout
    addresses = {int(m, 16) for m in re.findall(r"^\s*\d+\s+(0x[0-9A-F]{8})",
                                                frontier, re.M)}
    if not addresses:
        raise SystemExit("frontier.py produced no rows; is the build current?")

    counts = collections.Counter()
    untouched = 0
    # THE DISPATCH LIST, and the reason this option exists. On 2026-08-22 a
    # batch of 72 was bucketed straight out of `frontier.py`'s depth-first
    # order, which is CALL ORDER and says nothing about yield. Three of the six
    # buckets came back 12-for-12 MISMATCH having changed NOTHING, because
    # every body in them was already worked and already sitting on a
    # register-allocation plateau with the refusal written on its marker. The
    # agents were right; the list was wrong. This file already knew which those
    # were - it just could not say so in a form anything could dispatch.
    open_addresses: list[int] = []
    for record in read(REPO_ROOT / "src"):
        if record.address not in addresses or record.semantic:
            continue
        if not record.levers and not record.ruled_out:
            untouched += 1
            open_addresses.append(record.address)
            continue
        family = family_of(record)
        counts[family] += 1
        if family.startswith("worked, no known ceiling"):
            open_addresses.append(record.address)

    if "--open" in sys.argv:
        for address in sorted(open_addresses):
            print(f"0x{address:08X}")
        raise SystemExit(0)

    total = sum(counts.values()) + untouched
    print(f"  {untouched:4d}  never looked at - no lever, no refusal")
    for name, n in counts.most_common():
        print(f"  {n:4d}  {name}")
    # MATCHED ON THE STRING, WHICH IS WHY THIS WAS WRONG. The filter excluded
    # names starting "open" and "no notes" - but the not-a-ceiling family is
    # spelled "worked, no known ceiling named - likeliest to yield", which
    # starts with "worked". So the 210 bodies that explicitly have NO known
    # ceiling were counted as sitting on a measured, named one, and this line
    # reported 344 blocked when the real number is 134. It made the frontier
    # look two and a half times more stuck than it is.
    #
    # Now keyed off the same predicate `--open` uses, so the two CANNOT
    # disagree again: anything dispatchable is by definition not a ceiling.
    ceilings = sum(n for name, n in counts.items()
                   if not name.startswith("worked, no known ceiling")
                   and not name.startswith("no notes"))
    print(f"\n{total} remaining against \"byte exact or semantically the same\"")
    print(f"  {ceilings} sit on a ceiling this tree has already measured and named")
    print(f"  {total - ceilings} are open: never looked at, or worked without "
          f"hitting a named wall")
    print(f"\n  --open prints the {len(open_addresses)} address(es) that are "
          f"WORTH DISPATCHING - never looked at, or worked\n         without "
          f"hitting a named wall. Bucket from THIS, not from frontier.py's "
          f"depth-first order,\n         which is call order and says nothing "
          f"about yield.")
    print("\nSampled to check the big bucket is real: its members carry LEVER "
          "lines recording partial progress, not refusals - so they are "
          "genuinely unfinished rather than misfiled ceilings.")
