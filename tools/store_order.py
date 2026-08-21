#!/usr/bin/env -S uv run python
"""The order a body writes its fields, in the image and in this tree.

WHY. The commonest reason a body lands SHAPE_EXACT with every wrong constant a
member offset is that the source assigns fields in DECLARATION order and the
image assigns them in another. PlayerLock::clear, both Time::init overloads and
clear_faction were all this, in one day.

Finding it by eye means reading two disassemblies and matching displacements.
This prints just the displacements, in order, side by side - so the permutation
IS the answer, and nothing needs to be known about the struct's layout to read
it: the compiled column is this source's assignment order, the image column is
the order to write them in.

    uv run tools/store_order.py 0x00585FE0

A store is any `mov`/`and`/`or` whose destination is memory. Reads are ignored.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import capstone.x86 as x86

from decomp import read
from decomp.asm import (build_command, compile_unit, original_asm,
                        shared_spans, span_refusal, subject_asm)

REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
COMPILE_COMMANDS = Path(os.environ.get(
    "OPENSMACX_COMPILE_COMMANDS", REPO_ROOT / "build" / "compile_commands.json"))
BORROW = REPO_ROOT / "src" / "buffer.cpp"
FLAGS = ("/c /O2 /Gy /GR- /Oy- /GX", "/c /O2 /Gy /GR- /GX",
         "/c /O1 /Gy /GR- /Oy- /GX", "/c /O1 /Gy /GR- /GX")


def stores(listing) -> list[str]:
    """`base+disp` for each memory WRITE, in the order the code makes them."""
    out = []
    for instruction in listing.instructions:
        operands = instruction.operands
        if not operands or operands[0].type != x86.X86_OP_MEM:
            continue
        if instruction.mnemonic not in ("mov", "and", "or", "xor", "add",
                                        "sub"):
            continue
        memory = operands[0].mem
        base = instruction.reg_name(memory.base) if memory.base else "abs"
        index = (f"+{instruction.reg_name(memory.index)}*{memory.scale}"
                 if memory.index else "")
        disp = memory.disp
        sign = "-" if disp < 0 else "+"
        out.append(f"{instruction.mnemonic} [{base}{index}{sign}"
                   f"0x{abs(disp):x}]")
    return out


def _relative(rows: list[str]) -> list[int]:
    """Each store's displacement relative to the FIRST store's."""
    out = []
    for row in rows:
        text = row[row.rfind("0x"):].rstrip("]")
        value = int(text, 16) if text.startswith("0x") else 0
        out.append(-value if "-0x" in row else value)
    return [value - out[0] for value in out] if out else []


if __name__ == "__main__":
    target = int(sys.argv[1], 16)
    records = read(REPO_ROOT / "src")
    record = next((r for r in records if r.address == target), None)
    if record is None:
        raise SystemExit(f"{sys.argv[1]}: no annotation")
    if span_refusal(record, IMAGE, shared_spans(records)):
        raise SystemExit(f"{sys.argv[1]}: no bytes to read")

    try:
        command = build_command(COMPILE_COMMANDS, record.path)
    except ValueError:
        command = build_command(COMPILE_COMMANDS, BORROW)

    image = stores(original_asm(record, IMAGE))
    best: list[str] = []
    for flags in FLAGS:
        try:
            here = stores(subject_asm(compile_unit(record.path, command,
                                                   flags), record, flags))
        except Exception:                       # noqa: BLE001 - try the next
            continue
        if len(here) == len(image):
            best = here
            break
        best = best or here
    # NORMALISED TO THE FIRST STORE, because the two sides routinely base
    # their addressing on different points inside the same struct - the image
    # took `&goal.priority` where this tree took `&goal.x` - and the raw
    # displacements then differ everywhere while the ORDER is the only real
    # question.
    image_at = _relative(image)
    here_at = _relative(best)
    print(f"{record.address_hex}  {record.name}")
    print(f"{'IMAGE':30s} {'':6s}  {'THIS TREE':30s}")
    for index in range(max(len(image), len(best))):
        left = image[index] if index < len(image) else ""
        right = best[index] if index < len(best) else ""
        mark = " " if image_at[index:index + 1] == here_at[index:index + 1] \
            else "*"
        lo = f"@{image_at[index]:+d}" if index < len(image_at) else ""
        ro = f"@{here_at[index]:+d}" if index < len(here_at) else ""
        print(f"{mark} {left:30s} {lo:6s}  {right:30s} {ro}")
    print(f"{len(image)} store(s) in the image, {len(best)} here")
