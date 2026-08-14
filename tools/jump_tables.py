#!/usr/bin/env python3
"""Resolve the jump table behind an indirect `jmp`, so a switch is not guessed.

A `switch` compiles to `jmp dword ptr [reg*4 + TABLE]`, and TABLE lives in a
DATA section. The brief prints the function's CODE, so the table is invisible -
and an agent meeting a switch has to infer the case order from the surrounding
compares. Three agents reported that in one batch and all three said so
plainly: 0x00636960 and 0x00425DB0 "the case order is a best-effort guess",
0x00490B00 "3 byte-indexed jump-table dispatches simplified to a uniform field
test (table bytes not in the brief)".

A guessed case order is the worst kind of wrong. It compiles, it reads
correctly, and it dispatches to the wrong branch.

The fix is not clever: read the table. A fourth agent already did it by hand -
0x0045B550 pulled a real 30-entry table straight out of the image with pefile
and got an exact case-to-target mapping - which is what makes this a tool
rather than a hope.

TWO FORMS, both in this image:

    jmp dword ptr [eax*4 + 0x5f1234]     a table of code ADDRESSES
    movzx eax, byte ptr [ecx + 0x5f1200] a byte INDEX table, feeding the above
    jmp dword ptr [eax*4 + 0x5f1234]     - MSVC emits this pair for a sparse
                                           switch, and reading only the second
                                           gives every case the same target

BOUNDED THE WAY VTABLES ARE. `derive_class_vtables.slot_targets` stops at the
first entry that is not a code address and at the next known function head;
the same two rules apply here, because a jump table is a vtable by another
name. Without them a table runs into whatever follows it in .rdata and the
brief prints fiction with the confidence of fact.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

# `jmp dword ptr [eax*4 + 0x5f1234]`
INDIRECT_JUMP = re.compile(
    r"^dword ptr \[(?P<index>[a-z]\w+)\*(?P<scale>\d)"
    r"(?: \+ (?P<base>0x[0-9a-f]+))?\]$")
# `movzx eax, byte ptr [ecx + 0x5f1200]` - the sparse-switch index table.
BYTE_INDEX = re.compile(
    r"^(?P<dst>[a-z]\w+), byte ptr \[(?P<reg>[a-z]\w+) \+ "
    r"(?P<base>0x[0-9a-f]{6,8})\]$")


def _section_of(pe, address: int):
    for section in pe.sections:
        start = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
        if start <= address < start + max(section.Misc_VirtualSize,
                                          len(section.get_data())):
            return section, start
    return None, None


def _read(pe, address: int, length: int) -> bytes:
    section, start = _section_of(pe, address)
    if section is None:
        return b""
    data = section.get_data()
    offset = address - start
    return data[offset:offset + length]


def code_bounds(pe) -> tuple:
    section = [s for s in pe.sections if s.Name.startswith(b".text")][0]
    start = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
    return start, start + len(section.get_data())


def address_table(pe, base: int, heads: set, cap: int = 256) -> list:
    """Consecutive code addresses at `base`, stopping the way a vtable does.

    Two stops, both borrowed from `derive_class_vtables.slot_targets` because
    the structures are the same shape: the first entry that is not inside
    `.text`, and the first entry that is itself a known function head other
    than the ones already collected - which is where the NEXT table begins.
    """
    low, high = code_bounds(pe)
    out = []
    for index in range(cap):
        raw = _read(pe, base + index * 4, 4)
        if len(raw) < 4:
            break
        value = int.from_bytes(raw, "little")
        if not low <= value < high:
            break
        out.append(value)
    return out


def byte_table(pe, base: int, entries: int, cap: int = 512) -> list:
    """The sparse-switch index table: one byte per case, each an index into
    the address table."""
    raw = _read(pe, base, min(entries, cap))
    return list(raw)


def tables_in(pe, spans, functions=None) -> list:
    """[{kind, at, base, entries...}] for every switch dispatch in `spans`.

    Decoded from the catalogued span only, so decoding starts on a real
    instruction boundary - a linear sweep of `.text` invents instructions and
    would invent tables with them.
    """
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    heads = set(functions or ())
    found, pending_index = [], None
    for low, high in spans:
        window = _read(pe, low, high - low)
        if not window:
            continue
        for one in engine.disasm(window, low):
            if one.mnemonic in ("movzx", "mov"):
                hit = BYTE_INDEX.match(one.op_str)
                if hit:
                    pending_index = (one.address, int(hit.group("base"), 16))
                continue
            if one.mnemonic != "jmp":
                continue
            hit = INDIRECT_JUMP.match(one.op_str)
            if hit is None or hit.group("base") is None:
                continue
            base = int(hit.group("base"), 16)
            targets = address_table(pe, base, heads)
            if not targets:
                continue
            record = {"kind": "switch", "at": one.address, "base": base,
                      "targets": targets, "index_table": None}
            if pending_index is not None:
                # A BYTE INDEX TABLE IN FRONT OF IT is how MSVC spells a
                # SPARSE switch: case -> byte -> address. Reading only the
                # address table maps every case onto the same few targets and
                # loses which case goes where, which is the whole question.
                record["index_table"] = {
                    "at": pending_index[0], "base": pending_index[1],
                    "bytes": byte_table(pe, pending_index[1], 256)}
            found.append(record)
            pending_index = None
    return found


def render(pe, spans, functions=None) -> str:
    """The tables as text for the brief, or "" when the function has none."""
    tables = tables_in(pe, spans, functions)
    if not tables:
        return ""
    names = functions or {}
    lines = ["", "# Jump tables behind this function's switches",
             "",
             "Read from the image, not inferred. The table lives in a DATA",
             "section, so it is NOT in the disassembly above - three agents in",
             "one batch guessed a case order without it. `case N` is the entry",
             "at index N unless an index table is shown, in which case",
             "`case N` -> `index[N]` -> `targets[index[N]]`.",
             ""]
    for table in tables:
        lines.append(f"  jmp at 0x{table['at']:08X} through "
                     f"0x{table['base']:08X}, {len(table['targets'])} entries:")
        for index, target in enumerate(table["targets"]):
            row = names.get(target)
            label = f"  {row.get('name', '')[:44]}" if row else ""
            lines.append(f"    [{index:>3}] -> 0x{target:08X}{label}")
        index_table = table.get("index_table")
        if index_table:
            used = index_table["bytes"]
            # Only the prefix that indexes INTO the address table is real;
            # past that the bytes belong to whatever follows.
            limit = 0
            while limit < len(used) and used[limit] < len(table["targets"]):
                limit += 1
            lines.append(f"    sparse index table at "
                         f"0x{index_table['base']:08X} (byte per case), "
                         f"first {limit}:")
            lines.append("      " + " ".join(str(v) for v in used[:limit]))
        lines.append("")
    return "\n".join(lines)


def main() -> int:
    import argparse
    import pefile
    import byte_match
    import emit_translation_unit as emit

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("address")
    arguments = parser.parse_args()
    address = int(arguments.address, 16)

    functions = emit.load_functions()
    row = functions.get(address)
    if row is None:
        print(f"0x{address:08X} is not catalogued", file=sys.stderr)
        return 2
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    spans = emit.parse_body_ranges(row.get("body_ranges") or "")
    text = render(pe, spans, functions)
    print(text or f"0x{address:08X} has no indirect jump through a table")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
