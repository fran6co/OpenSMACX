#!/usr/bin/env python3
"""Record the shape of every recovered function's ORIGINAL body.

`unproven_recovered` is published as one number - 2,469 functions, 188,443 bytes
- and read as a debt that could go to zero. It cannot, and the reason is a
property of the bodies rather than of anyone's effort. This writes that property
down so the figure can be published split by WHY, and so a plan can be aimed at
the bucket it can actually move.

The shapes, in the order a function is tested against them:

  seam_forwarding  The recovered body hands the ORIGINAL address onward through a
                   shared seam - the `??__E` initialiser and `??__F` atexit
                   families do - so a differential runs the original on both
                   sides and its agreement says nothing about the recovery. This
                   is the measured FLOOR: no differential mechanism can reach it,
                   at any effort.
  x87              The body computes on the x87 stack. SMT-LIB has no 80-bit
                   extended sort, and `tools/lifted_x87.h` keeps true 80 bits
                   deliberately, so a symbolic route must refuse rather than
                   approximate: a Float64 abstraction could prove a WRONG
                   recovery equal.
  loop             The body contains a backward branch. A per-function symbolic
                   proof needs bounded unrolling or an invariant; a runtime
                   differential handles these fine, so this bucket is a limit on
                   ONE route, not on all of them.
  straight_line    Loop-free and x87-free: the population a symbolic equivalence
                   proof could discharge as a single verification condition.

Why this is a committed catalogue rather than computed on demand:
`tools/recovery_metrics.py` is deliberately runnable with no IDB, no Wine and no
game, which is what lets the published figures be regenerated anywhere. Deciding
these shapes needs the hash-pinned executable, so the decision is made here, once,
and the answer is committed - exactly the arrangement `docs/recovery/proven.csv`
already uses for the same reason.

Reads proprietary bytes. Writes only addresses and a shape name; no original
bytes, disassembly text or instruction counts reach the output.
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_GRP_JUMP, X86_OP_IMM  # noqa: E402

from generator_support import read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "recovery" / "recovered-shapes.csv"

# The generated families whose emitted body forwards the original address on.
# Named by the file the recovery lives in, because that is what the inventory
# records; the property is a fact about those generators' output, not a guess.
SEAM_SOURCES = ("src/atexit_thunks.cpp", "src/init_thunks.cpp")

# Mnemonics beginning with `f` that are NOT x87. Capstone spells the x87 set
# f*, and these are the collisions.
NOT_X87 = frozenset({"far", "fs"})


def parse_ranges(value: str):
    """The body's spans, which is NOT `end_address - address`.

    416 catalogued functions are split, and 402 carry a second cold span, so a
    contiguous read from the entry runs past the body and into whatever follows
    it - up to 2,102 bytes of another function in the worst measured case.
    """
    out = []
    for item in (value or "").split(";"):
        if not item:
            continue
        start, end = item.split("-", 1)
        out.append((int(start, 0), int(end, 0)))
    return sorted(out)


def body_shape(pe, md, row) -> str:
    """The shape of one function's original body."""
    if any(source in (row["source_locations"] or "")
           for source in SEAM_SOURCES):
        return "seam_forwarding"
    decoded = 0
    has_x87 = False
    has_loop = False
    for start, end in parse_ranges(row["body_ranges"]):
        code = read_bytes(pe, start, end - start)
        if not code:
            continue
        for instruction in md.disasm(code, start):
            decoded += 1
            mnemonic = instruction.mnemonic
            if mnemonic.startswith("f") and mnemonic not in NOT_X87:
                has_x87 = True
            if mnemonic.startswith("loop"):
                has_loop = True
            elif X86_GRP_JUMP in instruction.groups:
                for operand in instruction.operands:
                    # A branch to an address at or below its own is backward,
                    # which is the only definition of a loop available without
                    # building the whole control-flow graph - and it is the one
                    # that matters, because it is what makes a single symbolic
                    # verification condition impossible.
                    if (operand.type == X86_OP_IMM
                            and operand.imm <= instruction.address):
                        has_loop = True
    if decoded == 0:
        # Nothing to say about a body no decoder could read. Reported rather
        # than folded into a shape it might not have.
        return "undecodable"
    if has_x87:
        return "x87"
    if has_loop:
        return "loop"
    return "straight_line"


def classify(exe: Path, functions: Path):
    pe = pefile.PE(str(exe), fast_load=True)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    with functions.open(newline="", encoding="utf-8-sig") as handle:
        rows = [row for row in csv.DictReader(handle)
                if row["recovery_state"] == "source_complete"]
    out = []
    for row in sorted(rows, key=lambda r: int(r["address"], 16)):
        out.append({
            "address": row["address"],
            "name": row["name"],
            "shape": body_shape(pe, md, row),
        })
    return out


def render(rows) -> str:
    """The artifact's text, in ONE place.

    `write()` and `--check` rendered this separately until 2026-08-13, so the
    byte comparison held only while the two copies happened to agree. Change the
    column set in one and the check compares the new format against a file
    written in the old one and calls it stale - forever, and with a message
    pointing at the file rather than at the divergence.
    """
    import io
    buffer = io.StringIO()
    # LF, explicitly: csv defaults to CRLF and every other committed catalogue
    # here is LF, so the default would make this the odd one out and make a byte
    # comparison against it fail for no reason.
    writer = csv.DictWriter(buffer, fieldnames=("address", "name", "shape"),
                            lineterminator="\n")
    writer.writeheader()
    writer.writerows(rows)
    return buffer.getvalue()


def write(rows, output: Path) -> None:
    output.write_text(render(rows), encoding="utf-8")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed catalogue is stale")
    args = parser.parse_args(argv)

    rows = classify(args.exe, args.functions)
    counts: dict[str, int] = {}
    for row in rows:
        counts[row["shape"]] = counts.get(row["shape"], 0) + 1

    if args.check:
        if not args.output.is_file():
            print(f"error: {args.output} is missing", file=sys.stderr)
            return 1
        with args.output.open(newline="", encoding="utf-8") as handle:
            committed = handle.read()
        if render(rows) != committed:
            print(f"error: {args.output} is stale; regenerate it",
                  file=sys.stderr)
            return 1
        print(f"recovered shapes: {len(rows)} (up to date)")
        return 0

    write(rows, args.output)
    print(f"recovered shapes: {len(rows)} -> {args.output}")
    for shape, count in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"  {shape:16} {count}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
