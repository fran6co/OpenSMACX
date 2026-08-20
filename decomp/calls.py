"""What one function calls, in the order the image calls it.

THE ANNOTATION ALREADY CARRIES A `calls` FACT AND IT IS NOT THIS. That fact
is a SET, sorted by address, produced by a disassembler pass that no longer
runs - so it cannot say what happens first, it cannot say a target is
reached twice, and nothing re-derives it when a body changes. Reading the
shipped bytes answers all three, and answers them about the image rather
than about a CSV.

WHAT COUNTS AS A CALL. A direct `E8 rel32`, and a `jmp rel32` that leaves
the function's own span - which is a tail call, and reads as one edge in
the graph even though the encoding is a jump. An `FF 15 [slot]` is a call
through a memory slot: if the slot is in the import address table it is a
DLL entry point, and if it is not it is this image's own indirection. An
`FF /2 <register>` cannot be resolved from one function's bytes at all,
and is reported as what it is rather than dropped.

WHAT IS WORTH READING. The catalogue's `kind` fact separates Alpha
Centauri's own code (5,575 pieces) from the statically linked CRT and zlib
(331) and the thunks (35). A call graph that lists `_memset` and
`__ftol` beside `Console::editor_polar` buries the thing the reader came
for, so the caller filters on `kind` - see `tools/osmx.py calls`. This
module classifies; it does not decide what to hide.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path

from capstone import x86

from .asm import Listing


@dataclass(frozen=True)
class CallSite:
    """One outgoing edge, at the instruction that makes it."""
    index: int                   # position in the caller's instruction list
    at: int                      # address of the calling instruction
    target: int | None           # callee address, or None if unresolvable
    form: str                    # direct | tail | indirect | dynamic
    slot: int | None = None      # the memory slot an indirect call reads


def call_sites(listing: Listing) -> tuple[CallSite, ...]:
    """Every outgoing edge in `listing`, in the order the code makes them.

    A `jmp` counts only when it leaves the listing - inside it, it is
    control flow, and every loop would otherwise read as a self-call.
    """
    low = listing.base
    high = listing.base + listing.instruction_bytes
    found = []
    for index, instruction in enumerate(listing.instructions):
        mnemonic = instruction.mnemonic
        if mnemonic not in ("call", "jmp"):
            continue
        operands = instruction.operands
        if not operands:
            continue
        operand = operands[0]
        if operand.type == x86.X86_OP_IMM:
            target = operand.imm
            if mnemonic == "jmp" and low <= target < high:
                continue                     # a branch, not an edge
            found.append(CallSite(
                index=index, at=instruction.address, target=target,
                form="direct" if mnemonic == "call" else "tail"))
        elif operand.type == x86.X86_OP_MEM and not operand.mem.base \
                and not operand.mem.index:
            found.append(CallSite(
                index=index, at=instruction.address, target=None,
                form="indirect", slot=operand.mem.disp & 0xFFFFFFFF))
        else:
            # Through a register or a computed address - a vtable dispatch,
            # a bound function pointer. One function's bytes cannot say
            # where it goes, and guessing would invent an edge.
            found.append(CallSite(index=index, at=instruction.address,
                                  target=None, form="dynamic"))
    return tuple(found)


def imported_names(exe: Path | str) -> dict[int, str]:
    """Every import address table slot in `exe`, by the address it sits at.

    `{0x00669058: "GDI32.dll!DeleteObject"}`. An `FF 15` through one of
    these leaves the program, so it is never a recovery target - and saying
    WHICH one it leaves through is most of what a reader wants from an
    indirect call.
    """
    data = Path(exe).read_bytes()
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    if data[e_lfanew:e_lfanew + 4] != b"PE\x00\x00":
        raise ValueError(f"{Path(exe).name}: not a PE image")
    _machine, n_sections, _ts, _sp, _ns, opt_size, _ch = \
        struct.unpack_from("<HHIIIHH", data, e_lfanew + 4)
    optional = e_lfanew + 24
    (image_base,) = struct.unpack_from("<I", data, optional + 28)
    # DataDirectory starts at optional+96; [0] is the export table and
    # [1] is the import table, so the import descriptors are at +104.
    import_rva, _size = struct.unpack_from("<II", data, optional + 104)
    if not import_rva:
        return {}

    sections = []
    table = optional + opt_size
    for index in range(n_sections):
        off = table + index * 40
        va, raw_size, raw_ptr = struct.unpack_from("<III", data, off + 12)
        sections.append((va, raw_size, raw_ptr))

    def at(rva: int) -> int | None:
        for va, raw_size, raw_ptr in sections:
            if va <= rva < va + raw_size:
                return raw_ptr + (rva - va)
        return None

    def string(rva: int) -> str:
        start = at(rva)
        if start is None:
            return ""
        return data[start:data.index(b"\x00", start)].decode(errors="replace")

    found: dict[int, str] = {}
    descriptor = at(import_rva)
    if descriptor is None:
        return {}
    while True:
        int_rva, _ts, _fc, name_rva, iat_rva = \
            struct.unpack_from("<IIIII", data, descriptor)
        if not (int_rva or name_rva or iat_rva):
            break
        library = string(name_rva)
        # THE INT IS THE ONE THAT KEEPS THE NAMES. In a linked image the
        # IAT holds the resolved addresses, so reading it back gives no
        # names at all; the import name table beside it still points at
        # the hint/name pairs. Where the INT is absent - some linkers omit
        # it - the IAT is still the unbound original on disk.
        walk = at(int_rva or iat_rva)
        slot = image_base + iat_rva
        while walk is not None:
            (entry,) = struct.unpack_from("<I", data, walk)
            if not entry:
                break
            if entry & 0x80000000:
                found[slot] = f"{library}!#{entry & 0xFFFF}"
            else:
                found[slot] = f"{library}!{string(entry + 2)}"
            walk += 4
            slot += 4
        descriptor += 20
    return found
