#!/usr/bin/env -S uv run python
"""Every place a body's listing differs from the image's, not just the first.

`osmx measure` reports the FIRST divergence, which is the right summary and the
wrong tool when the first one is a jump whose target moved because of something
200 instructions later. This prints the whole aligned diff, so the real cause is
visible rather than inferred.

    uv run tools/listing_diff.py <addr> [--flags '<flag set>']

Operands that either side relocates are masked before comparing, the same way
the ratchet masks them, so a `call dword ptr [0]` against a real address is not
reported as a difference.
"""

from __future__ import annotations

import difflib
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_command, compiled_asm, original_asm

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
COMPILE_COMMANDS = Path(os.environ.get(
    "OPENSMACX_COMPILE_COMMANDS", REPO_ROOT / "build" / "compile_commands.json"))


# `Listing.shapes` is the library's own relocation-masked form - an operand
# becomes ('imm',) with no value, so a `call rel32` compares equal whatever it
# targets. The first draft of this tool compared LISTING TEXT and claimed in
# its docstring to mask; it did not, and every call and every jump whose target
# had shifted read as a difference, which buries the real ones.
def aligned(listing):
    """(comparable shape, printable text) per instruction."""
    return list(zip([repr(s) for s in listing.shapes],
                    [l.split("  ", 2)[-1].strip() for l in listing.lines]))


if __name__ == "__main__":
    argv = sys.argv[1:]
    flags = "/c /O2 /Gy /GR- /GX"
    if "--flags" in argv:
        flags = argv[argv.index("--flags") + 1]
        del argv[argv.index("--flags"):argv.index("--flags") + 2]
    if not argv:
        raise SystemExit("usage: listing_diff.py <addr> [--flags '<set>']")

    address = int(argv[0], 16)
    record = next((r for r in read(REPO_ROOT / "src") if r.address == address), None)
    if record is None:
        raise SystemExit(f"0x{address:08X} is not in the catalogue")

    theirs = aligned(original_asm(record, IMAGE))
    command = build_command(COMPILE_COMMANDS, record.path)
    mine = aligned(compiled_asm(record, command, flags))

    print(f"0x{address:08X}  {record.name}")
    print(f"  flags {flags}")
    print(f"  image {len(theirs)} instruction(s), this tree {len(mine)}\n")

    runs = 0
    matcher = difflib.SequenceMatcher(None, [k for k, _ in theirs],
                                      [k for k, _ in mine])
    for group in matcher.get_opcodes():
        tag, i1, i2, j1, j2 = group
        if tag == "equal":
            continue
        runs += 1
        print(f"  at image instruction {i1}:")
        for _, line in theirs[i1:i2]:
            print(f"    IMAGE  {line}")
        for _, line in mine[j1:j2]:
            print(f"    OURS   {line}")
        print()
    print(f"{runs} differing run(s)")
