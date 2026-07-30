#!/usr/bin/env python3
"""Recover the case targets behind `jmp dword ptr [reg*4 + TABLE]`.

A switch compiled by MSVC leaves no trace in the instruction stream of where
its arms are: the jump reads a dword out of a table that sits in the data gaps
between functions, and the table's LENGTH is nowhere in the encoding. Lowering
the jump without recovering that table is not a missing mnemonic - the
instruction lowers fine - it is a silent control-flow hole. `jmp [eax*4 + T]`
becomes `opensmacx_dispatch(opensmacx_mem32(...))`, and every value in T is an
address INTERIOR to the function while the dispatch table holds function STARTS
only, so the dispatch traps on all 283 of them the instant a switch is reached.

This module is pure analysis: it answers "which addresses can this jump reach",
and emits nothing. The emission side needs one thing from here - a list of
targets in index order - and gets the index register from the capstone operand
it already has.

THE STOPPING RULE, AND WHY THE OBVIOUS ONE IS WRONG
---------------------------------------------------
Walk consecutive dwords from the table base and keep those that land inside the
function's own `body_spans`. That much is sound and it is what makes every
recovered target provably an address in the function being lifted, with no
heuristic about what a switch "looks like".

Stopping at the first dword that does NOT land inside the function is the
obvious rule and it OVER-READS, because it can only see the end of a table when
the bytes after it stop looking like the function. When MSVC emits two switches
in one function it lays their tables out back to back, and the second table's
entries point into the very same function - so the walk runs straight through
it. `alt_get_ocean_detail` is the clean demonstration: four four-entry tables
at 0x46269c, 0x4626ac, 0x4626bc, 0x4626cc, each with its own `cmp ecx, 3` in
front of its jump, and the naive walk from the first returns all sixteen
entries. Across the image that rule returns 3,942 targets where 2,526 are real:
1,416 of them - 36% - are the next switch's arms.

The extra entries are worse than useless. An index past the true end used to
reach the dispatch and trap; with a bogus case it silently jumps into a
different switch's first arm, which is a wrong answer that runs.

So the walk is CLAMPED at the next table base referenced by any indexed jump in
the same function. That is structural rather than heuristic - it uses only the
jump instructions already decoded - and it is what the two-pass shape below is
for. Clamping against every table base in the whole image was measured and
gives an identical result, so the cheaper local clamp is the one kept.

WHY THE CLAMPED RULE IS BELIEVED
--------------------------------
The compiler emits its own range check in front of the jump, and that check is
an INDEPENDENT statement of the table's length that this module never consults.
`bound_check` reconstructs it for cross-checking only. Of the 283 sites, 276
have a recoverable bound and all 276 agree with the clamped walk exactly; the
naive walk disagreed on 68 of them. The remaining 7 bound their index with a
register, an `and` mask or an FPU compare rather than an immediate, so no
static length exists to compare against.
"""

from __future__ import annotations

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone import x86
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from generator_support import read_bytes

# A table longer than this is not a switch, it is a walk that has escaped. No
# real site comes near it - the largest in the image has 101 arms - so this
# only bounds the damage if a future image breaks an assumption above.
MAX_ENTRIES = 4096

# The 8-bit destination MSVC writes when a switch is dense enough to index
# through a byte table first (`mov cl, byte ptr [edi + BYTES]`). Only used by
# the cross-check, which has to know that CL narrows ECX.
LOW8_OWNER = {
    x86.X86_REG_AL: x86.X86_REG_EAX, x86.X86_REG_BL: x86.X86_REG_EBX,
    x86.X86_REG_CL: x86.X86_REG_ECX, x86.X86_REG_DL: x86.X86_REG_EDX,
}


def inside(spans, address: int) -> bool:
    return any(low <= address < high for low, high in spans)


def indexed_jump(instruction):
    """(table base, index register) if this is a switch dispatch, else None.

    The shape is exact on purpose: scale 4, an index register, and NO base
    register, so the displacement is the whole table address. All 283 sites in
    the image are this form. A base register would mean the table address is
    computed at runtime and the displacement alone would name the wrong table -
    returning None leaves such a jump lowering as it does today, which traps,
    rather than inventing a table for it.
    """
    if instruction.mnemonic != "jmp":
        return None
    operands = instruction.operands
    if len(operands) != 1 or operands[0].type != X86_OP_MEM:
        return None
    memory = operands[0].mem
    if memory.scale != 4 or memory.index == x86.X86_REG_INVALID:
        return None
    if memory.base != x86.X86_REG_INVALID:
        return None
    if memory.segment != x86.X86_REG_INVALID:
        return None
    return memory.disp & 0xFFFFFFFF, memory.index


def walk(pe, spans, base: int, limit: int) -> list[int]:
    """Consecutive in-function dwords from `base`, stopping before `limit`."""
    targets: list[int] = []
    address = base
    while address < limit and len(targets) < MAX_ENTRIES:
        raw = read_bytes(pe, address, 4)
        if len(raw) < 4:
            break
        value = int.from_bytes(raw, "little")
        if not inside(spans, value):
            break
        targets.append(value)
        address += 4
    return targets


def resolve_decoded(pe, spans, instructions) -> dict[int, list[int]]:
    """{jmp address: [target in index order]} for one already-decoded function.

    Two passes, and the split is the whole point: pass one collects every table
    base in the function so pass two can clamp each walk at the next one. A
    single pass cannot do it - the table that ends this one has not been seen
    yet when the walk needs to stop.
    """
    sites = []
    for instruction in instructions:
        found = indexed_jump(instruction)
        if found:
            sites.append((instruction.address, found[0]))
    if not sites:
        return {}

    bases = sorted({base for _, base in sites})
    tables: dict[int, list[int]] = {}
    for address, base in sites:
        following = [other for other in bases if other > base]
        limit = following[0] if following else 1 << 32
        targets = walk(pe, spans, base, limit)
        # A base whose first dword already leaves the function is not a table
        # this function owns. Emitting an empty switch would be legal C++ and a
        # lie; leaving the site out keeps today's dispatch, which traps.
        if targets:
            tables[address] = targets
    return tables


def resolve(pe, spans, decoder: Cs | None = None) -> dict[int, list[int]]:
    """{jmp address: [target]} for the function occupying `spans`.

    The self-contained entry point. A caller that has already decoded the body
    should hand the instructions to `resolve_decoded` instead and skip the
    second decode.
    """
    if decoder is None:
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
    instructions = []
    for low, high in spans:
        instructions.extend(decoder.disasm(read_bytes(pe, low, high - low), low))
    return resolve_decoded(pe, spans, instructions)


def bound_check(pe, instructions, position: int):
    """(entries, shape) the COMPILER says this switch has, or None.

    Not used to recover anything - `resolve` never calls this. It exists so the
    stopping rule can be checked against a second, independent source, which is
    the only reason to believe a table length that is nowhere in the encoding.
    Two shapes carry a static length:

    `direct`     `cmp idx, N` / `ja default` / `jmp [idx*4 + T]`, so N + 1 arms.
                 MSVC schedules unrelated instructions between the compare and
                 the branch, so the compare is searched for rather than assumed
                 adjacent - and rejected if anything in between rewrites idx.
    `byte-table` a dense switch indexes a byte table first: `cmp r, N` / `ja` /
                 `mov idx8, byte [r + B]` / `jmp [idx*4 + T]`. The dword table
                 then has as many arms as the byte table has distinct values,
                 so its length is max(B[0..N]) + 1.
    """
    jump = instructions[position]
    found = indexed_jump(jump)
    if not found:
        return None
    index = found[1]

    for step in range(position - 1, max(-1, position - 5), -1):
        loader = instructions[step]
        operands = loader.operands
        if loader.mnemonic not in ("mov", "movzx") or len(operands) != 2:
            continue
        if operands[1].type != X86_OP_MEM or operands[1].size != 1:
            continue
        if operands[0].type != X86_OP_REG:
            continue
        written = LOW8_OWNER.get(operands[0].reg, operands[0].reg)
        if written != index:
            continue
        memory = operands[1].mem
        if memory.base == x86.X86_REG_INVALID or not memory.disp:
            continue
        table = memory.disp & 0xFFFFFFFF
        for back in range(step - 1, max(-1, step - 9), -1):
            compare = instructions[back]
            if compare.mnemonic != "cmp" or len(compare.operands) != 2:
                continue
            if (compare.operands[0].type != X86_OP_REG
                    or compare.operands[0].reg != memory.base
                    or compare.operands[1].type != X86_OP_IMM):
                continue
            if not any(instructions[gap].mnemonic in ("ja", "jnbe")
                       for gap in range(back + 1, step)):
                break
            count = compare.operands[1].imm + 1
            raw = read_bytes(pe, table, count)
            if len(raw) == count:
                return max(raw) + 1, "byte-table"
            break
        break

    for step in range(position - 1, max(-1, position - 9), -1):
        compare = instructions[step]
        if compare.mnemonic != "cmp" or len(compare.operands) != 2:
            continue
        if (compare.operands[0].type != X86_OP_REG
                or compare.operands[0].reg != index
                or compare.operands[1].type != X86_OP_IMM):
            continue
        if not any(instructions[gap].mnemonic in ("ja", "jnbe")
                   for gap in range(step + 1, position)):
            return None
        for gap in range(step + 1, position):
            if index in instructions[gap].regs_access()[1]:
                return None
        return compare.operands[1].imm + 1, "direct"
    return None
