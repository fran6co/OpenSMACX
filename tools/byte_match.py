#!/usr/bin/env python3
"""Recompile a body with the ORIGINAL compiler and compare, for any function.

`tools/msvc6_byte_match.py` asked whether VC6 12.00.8168 reproduces the shipped
instruction sequence. It does. But it asked over five subjects hard-coded in a
Python list, which made the list the interface - so the answer could not be had
for a sixth function without editing the tool. This is the mechanism, extracted,
and it takes an address and a source file.

WHAT IS COMPARED, and why it is not mnemonics.

The original tool discarded ALL operands and compared capstone mnemonic names.
That was justified as conservative - "the original's addresses do not exist in
an object file" - and it is true of RELOCATED operands and false of everything
else. Register numbers, ModRM displacements, immediates, branch displacements
and the `ret` pop count are all fully determined by the compiler and reproduce
exactly: measured, all four matching cases are byte-identical over their whole
bodies, not merely mnemonic-identical.

Discarding operands is therefore not conservative. It is permissive in exactly
the direction this repository's authoring style diverges:

  * `call dword ptr [SeamPtr]` and `call rel32` both spell `call`;
  * `call dword ptr [eax+0x64]` and `call dword ptr [eax+0x68]` - a WRONG
    VTABLE SLOT - are indistinguishable.

With 5,159 unresolved indirect call sites in this image, that second one is the
most valuable thing an operand-aware comparator buys.

So the verdict is tiered, and every tier is reported every run:

  BYTE_EXACT     every byte equal, or unknowable on both sides
  SHAPE_EXACT    same count, same length, same (mnemonic, size, operand kind);
                 displacement and immediate dropped only where that instruction
                 carries a relocation. "Same choices, different constant."
  MNEMONIC_ONLY  the old verdict, plus the `ret` pop count. The right tier for a
                 body under active authoring.
  MISMATCH       with the first divergence index.

The current recovery workflow advances only on BYTE_EXACT. SHAPE_EXACT and
MNEMONIC_ONLY remain diagnostic tiers for authoring: a body at SHAPE_EXACT but
not BYTE_EXACT has a wrong constant, which is a different piece of work from a
body at MISMATCH, but it is not a recovery success.

MASKING. A relocated site holds a linked value on one side and a placeholder on
the other, so it must be masked - and WHICH SIDE carries the relocation is not
symmetric, which is the thing this got wrong twice.

  * An ABSOLUTE address (`mov eax, [0x9BBFEC]`) carries a HIGHLOW base
    relocation in the IMAGE, because rebasing must fix it up. An object file
    compiled from `static int *const G = (int *)0x9BBFEC` emits the literal
    with NO relocation at all. Masked on the original side only.
  * A RELATIVE branch (`call rel32`, `jmp rel32`) carries NO base relocation in
    the image - a relative displacement is unaffected by rebasing - but an
    object file calling a separately-compiled function MUST emit an
    IMAGE_REL_I386_REL32. Masked on the OBJECT side only.

So the first rule tried here - "agrees if the bytes are equal, or BOTH sides are
masked" - made BYTE_EXACT structurally unreachable for every function
containing a call. Measured: 83% of the remaining population contains a rel32
branch, and the census's 266 matches were all call-free bodies, which is what
that rule was selecting for rather than measuring.

The rule is therefore: index i agrees if the bytes are equal, OR EITHER side is
relocated there. A relocation is a declaration that the value is not determined
by the compiler, and that is true whichever side declares it.

What that gives up, stated exactly. A relocation covers the DISPLACEMENT, not
the opcode: for `E8 xx xx xx xx` the record points at the four displacement
bytes and the `E8` stays unmasked. So a call placed at the wrong offset, or an
`E8` where the original has something else, is still caught by ordinary byte
comparison - position and instruction selection are not given up.

What IS given up is the callee's IDENTITY: `call A` and `call B` compare equal,
because neither side's four bytes are compared. Recovering that needs the
emitter to spell callees stably enough to match a relocation's target symbol
against the catalogue name at the original's resolved branch target. That is NOT
done here, and `masked_bytes`/`compared_bytes` are reported on every verdict so
a body that is mostly relocation cannot read as a strongly evidenced match.

The worry that motivated the strict rule - a relocated `call rel32` masking into
agreement with an absolute `call dword ptr [X]` - survives: those encode as
`E8 xx xx xx xx` and `FF 15 xx xx xx xx`, different lengths and different opcode
bytes, and opcode bytes are never masked.

THE FLAGS: `/c /O2 /Gy /GR- /Oy- /GX`.

`/Oy-` because `/O2` implies `/Oy` and the shipped image keeps its frame
pointers - that correction took the original experiment from 0/5 to 3/5. `/GX`
because 387 bodies in this image register a `__CxxFrameHandler` frame, and
measured, `/GX` reproduces it exactly - the `.text$x` COMDAT holding
`lea ecx,[ebp-N]; jmp <dtor>` funclets and the `mov eax,<FuncInfo>; jmp
__CxxFrameHandler` thunk - while changing nothing on the five non-EH cases.

Output is disassembly of proprietary bytes as MNEMONIC NAMES and counts only.
No opcode byte leaves this tool, and nothing it prints is committed.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import shutil
import struct
import concurrent.futures
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

from generator_support import parse_body_ranges, read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

VC6_ROOT = Path(os.environ.get("VC6_ROOT", Path.home() / "opt" / "vc6"))
VC6_CL = VC6_ROOT / "BIN" / "CL.EXE"
VC6_PREFIX = VC6_ROOT / ".wineprefix"

# Kept verbatim so the pre-registered run stays reproducible, alongside the
# flags this project then measured the shipped build to have used.
PREREGISTERED_FLAGS = "/c /O2 /Gy /GR- /Oy-"
MEASURED_FLAGS = "/c /O2 /Gy /GR- /Oy- /GX"
# `/O2` implies `/Oy`, which OMITS the frame pointer.
FRAMELESS_FLAGS = "/c /O2 /Gy /GR- /GX"

# THE FLAG SET IS NOT ONE-SIZE-FITS-ALL, and assuming it was is the single
# biggest thing this tool got wrong.
#
# `/Oy-` was added because the five pre-registered cases all diverged at
# instruction #0, `push` against `mov`, and the shipped image kept its frame
# pointers. That correction took the original experiment 0/5 -> 3/5 and is
# right about those five. It was then generalised to the whole image, which the
# image does not support. Measured over the 3,062 remaining targets whose
# prologue is readable:
#
#     1,518 open `push ebp; mov ebp, esp`   -> /Oy- is correct
#     1,544 do NOT                          -> /Oy- is WRONG, and forces a
#                                              frame the original has not
#
# So a fixed `/Oy-` is right for 49% of the population and guarantees a
# divergence at instruction #0 for the other 51%. Three of six pilot functions
# failed in exactly that way - `original 'mov' vs rebuilt 'push'` - which is
# the original 0/5 symptom with the sign flipped.
#
# Both are tried and the best verdict is kept. One compile is 44 ms, so this
# costs less than any rule for choosing between them, and it cannot be wrong
# the way a rule read off the prologue could be.
#
# `/O1` IS ALSO IN THE IMAGE. The pre-registered run recorded `/O1` as
# "decisively not it" on five functions, and that is true of those five. It is
# not true of the image: a fan-out agent reproduced `_malloc` exactly under
# `/O1`, and `sub_515430` clears one stack argument with `pop` where `/O2`
# emits `add esp, 4` - the size-optimisation idiom. So `/O1` bodies exist and
# are refused by an `/O2`-only tool.
#
# Four sets, two optimisation levels crossed with the frame pointer. At 44 ms a
# compile the whole cross costs under 200 ms, which is far cheaper than any
# rule for choosing between them.
SIZE_FLAGS = "/c /O1 /Gy /GR- /Oy- /GX"
SIZE_FRAMELESS_FLAGS = "/c /O1 /Gy /GR- /GX"
FLAG_SETS = (MEASURED_FLAGS, FRAMELESS_FLAGS, SIZE_FLAGS,
             SIZE_FRAMELESS_FLAGS)

# FOUR IS THE WHOLE SEARCH, AND THAT IS MEASURED RATHER THAN ASSUMED. The
# obvious next move when a body will not match is to suspect the flags, so it
# was swept exhaustively once and the answer written down here, beside the
# constant, rather than left for each agent to re-derive:
#
#   * 288 flag combinations over `sub_5e3630` - the /O1 and /O2 bundles taken
#     apart into `/Og /Oi /Os` x `/Ot`, `/Oy` against `/Oy-`, `/Ob0/1/2`, `/Oa`
#     against `/Ow`. NOT ONE reached BYTE_EXACT, or even SHAPE_EXACT. The cost
#     of finding that out was one command; the cost of not knowing it was every
#     future agent spending attempts on "it might be the flags".
#   * `/Op` and `/O1` both cancel the `sqrt` INTRINSIC, replacing an inline
#     `fsqrt` with a `call`. So neither can ever explain a body that contains
#     one, which removes two more flags from any search that starts there.
#   * VC6 /O2 spills an FP temp into a DEAD PARAMETER'S HOME SLOT before it
#     grows the frame. "Two memory temps in a `push ecx` frame" is therefore one
#     local plus an argument home, not two locals - a reading that otherwise
#     sends you looking for a frame size the compiler never emits.
#   * `/G3 /G4 /G5 /G6 /GB` emit identical bytes on the cases tried, so the
#     processor target is not a lever either. That was measured on INTEGER
#     bodies; re-tested 2026-08-12 on an x87 body (0x006280E0) crossed with
#     twelve optimisation flags, it holds there too - same FP sequence, zero
#     `fxch`, every time. The obvious "but surely /G5 changes the FPU schedule"
#     is answered.
#   * `register` storage is ignored at /O2, and compilation-unit context does
#     not move the allocator: a body compiled alone and beside a sibling
#     produces byte-identical output, because VC6's allocator is
#     intra-procedural.
#
# So a MISMATCH is a fact about SOURCE FORM, not about the invocation. The
# search space that remains is the spelling, which is what `--dir` is for.

# Best first; `match_function` keeps the best verdict across flag sets.
TIER_ORDER = ("BYTE_EXACT", "SHAPE_EXACT", "MNEMONIC_ONLY", "MISMATCH",
              "NO_COMPILE", "SHARED_TAIL", "REFUSED")


def recovery_exit_code(tier: str) -> int:
    """Only byte equality is a successful recovery-verification result."""
    return 0 if tier == "BYTE_EXACT" else 1


# `__CxxFrameHandler`. Every EH registration in this image names it; the fifteen
# frames that do not are CRT (docs/EXCLUSIONS.md), and a prologue naming
# anything else means you are reading library code.
CXX_FRAME_HANDLER = 0x00644FD6

# The self-modifying copy-protection section. Refuse rather than compare.
SELFMOD_RANGE = (0x00664000, 0x00669000)

# VC6 has neither <cstdint> nor <stdint.h>. Hand-typedef instead of including.
STDINT = ("typedef int int32_t;\n"
          "typedef unsigned int uint32_t;\n"
          "typedef short int16_t;\n"
          "typedef unsigned short uint16_t;\n"
          "typedef signed char int8_t;\n"
          "typedef unsigned char uint8_t;\n")

# COFF constants.
IMAGE_SYM_CLASS_EXTERNAL = 2
IMAGE_SYM_CLASS_LABEL = 6
IMAGE_REL_I386_DIR32 = 6
IMAGE_REL_I386_REL32 = 20
# PE base relocation types.
IMAGE_REL_BASED_ABSOLUTE = 0
IMAGE_REL_BASED_HIGHLOW = 3


# ------------------------------------------------------------ the catalogue

@dataclass
class BodyLayout:
    """A function's spans, classified. NEVER concatenated.

    Concatenating them - which `msvc6_byte_match.original_bytes` does - creates
    a fake instruction boundary: the last byte of one span abuts the first of
    the next in the buffer but not in the image, so capstone can decode a
    straddling instruction that exists nowhere.

    Measured over all 416 split rows (448 extra spans): every one decodes fully
    as CODE and none is a jump table. 387 end in the ten-byte
    `mov eax, imm32; jmp rel32` shape whose target is `__CxxFrameHandler` - the
    exact count docs/EXCLUSIONS.md already carries - so the split is MSVC's C++
    EH funclets, gathered by the linker into 0x0065xxxx-0x0066xxxx.
    """
    primary: list = field(default_factory=list)
    eh: list = field(default_factory=list)
    shared: list = field(default_factory=list)
    selfmod: list = field(default_factory=list)

    @property
    def refusal(self) -> str:
        if self.selfmod:
            return "body has a span in _SELFMOD (copy protection)"
        if not self.primary:
            return "no primary span"
        return ""


def _looks_like_eh_funclet(pe, low: int, high: int) -> bool:
    """Does this span end in the `__ehhandler` thunk?

    `mov eax, <FuncInfo>; jmp __CxxFrameHandler` is ten bytes: B8 imm32 E9 rel32.
    The jump is relative to the address after it, so the target is resolved
    rather than pattern-matched - a rel32 that happens to look right but lands
    somewhere else is not this shape.
    """
    data = read_bytes(pe, max(low, high - 10), min(10, high - low))
    if len(data) != 10 or data[0] != 0xB8 or data[5] != 0xE9:
        return False
    displacement = struct.unpack_from("<i", data, 6)[0]
    return (high + displacement) == CXX_FRAME_HANDLER


def classify_body(pe, row: dict, shared_spans: set) -> BodyLayout:
    """Split `body_ranges` into primary / EH / shared / selfmod."""
    layout = BodyLayout()
    spans = parse_body_ranges(row.get("body_ranges") or "")
    if not spans:
        address = int(row["address"], 16)
        spans = [(address, address + int(row["size"] or 0))]
    entry = int(row["address"], 16)
    for low, high in spans:
        if SELFMOD_RANGE[0] <= low < SELFMOD_RANGE[1]:
            layout.selfmod.append((low, high))
        elif (low, high) in shared_spans and low != entry:
            layout.shared.append((low, high))
        elif low != entry and _looks_like_eh_funclet(pe, low, high):
            layout.eh.append((low, high))
        elif low == entry:
            layout.primary.append((low, high))
        else:
            # Reached only from the frame registration but not ending in the
            # thunk: still EH-side code, and not part of the primary body.
            layout.eh.append((low, high))
    return layout


def load_rows() -> dict:
    """The catalogue, with `catalogue_corrections` applied.

    `emit_translation_unit.load_functions` corrects the mangled names the
    bytes prove wrong, and this loader used to read the CSV raw. So the
    emitter built a unit against the CORRECTED signature while the comparison
    looked the ORIGINAL name up - two loaders with two answers about one row,
    and the disagreement was invisible, because the tier came out of a
    comparison neither side thought it was making.

    Found on `Caviar::vx_read`: the `QAA` -> `SAA` correction reached the
    scaffolding but not here, so every parameter read stayed off by exactly
    the width of a `this` the emitter had already taken out.

    The store is `src/` now, so both loaders resolve to the same annotations
    rather than to the same CSV, which closes that disagreement by construction
    instead of by remembering to correct in two places.
    """
    import emit_translation_unit
    return emit_translation_unit.load_functions()


def shared_span_index(rows: dict) -> set:
    """Spans claimed by more than one catalogued function.

    13 spans have multiple claimants and one is claimed by 13 functions. That is
    `/Gy` COMDAT identical folding - independent evidence the shipped build used
    `/Gy` - and a folded tail belongs to no single function, so its byte-match
    verdict is not well defined. SHARED_TAIL says so instead of inventing one.
    """
    seen, shared = {}, set()
    for address, row in rows.items():
        for span in parse_body_ranges(row.get("body_ranges") or ""):
            if span in seen and seen[span] != address:
                shared.add(span)
            seen.setdefault(span, address)
    return shared


# --------------------------------------------------------- the original side

def original_relocation_mask(pe, low: int, high: int) -> set:
    """Byte addresses in [low, high) the base relocations make unknowable.

    Only HIGHLOW (type 3) marks four bytes. Type 0 is ABSOLUTE padding and marks
    nothing - this image carries 362 of them, and counting those would mask real
    code bytes at offset zero of a block.
    """
    masked = set()
    if not hasattr(pe, "DIRECTORY_ENTRY_BASERELOC"):
        pe.parse_data_directories(directories=[
            pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_BASERELOC"]])
    base = pe.OPTIONAL_HEADER.ImageBase
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []) or []:
        for entry in block.entries:
            if entry.type != IMAGE_REL_BASED_HIGHLOW:
                continue
            address = base + entry.rva
            if low <= address < high or low <= address + 3 < high:
                for offset in range(4):
                    if low <= address + offset < high:
                        masked.add(address + offset)
    return masked


def original_span_bytes(pe, low: int, high: int) -> bytes:
    """The span's bytes, with trailing alignment padding removed.

    THE STRIP MUST BE SYMMETRIC. `object_code` strips trailing `0x90`/`0xCC`
    from the rebuilt side, because VC6 pads a COMDAT to alignment. Doing it on
    one side only makes the lengths differ whenever the CATALOGUED span
    includes padding too, and then no source can ever match.

    Found by a fan-out agent on `0x0050F630`: the real body is
    `mov ecx, imm32; call; ret 4` - 13 bytes - inside a catalogued 16-byte span
    whose last three bytes are `nop` alignment before the next 16-byte-aligned
    thunk. The body was correct and the tool refused it, which is the worst
    kind of defect this comparator can have: a false MISMATCH reads as the
    author's fault.

    `nop` and `int3` are stripped only from the END, so a body that genuinely
    executes a `nop` in its interior is untouched.
    """
    data = read_bytes(pe, low, high - low)
    if len(data) != high - low:
        raise ValueError(f"short read at 0x{low:08X}")
    return data.rstrip(b"\xcc").rstrip(b"\x90")


# ----------------------------------------------------------- the object side

@dataclass
class CoffSection:
    name: str
    raw_size: int
    raw_pointer: int
    relocations: list       # [(offset, symbol index, type)]


@dataclass
class CoffSymbol:
    name: str
    value: int
    section: int
    storage: int


def parse_coff(data: bytes):
    """Sections, symbols and relocations, walked directly.

    Not through dumpbin: capstone has to see the same bytes the comparison
    claims to be about, and a text parse of another disassembler's output is a
    second decoder to keep honest.

    Auxiliary symbol records are SKIPPED rather than decoded as symbols. The
    version this replaces iterated all 18-byte records blindly, which can read
    an aux record's bytes as a name and a storage class.
    """
    machine, nsections, _, sym_ptr, nsyms, opt_size, _ = struct.unpack_from(
        "<HHIIIHH", data, 0)
    if machine != 0x14C:
        raise ValueError(f"not an i386 object (machine 0x{machine:04X})")

    sections = []
    offset = 20 + opt_size
    for _ in range(nsections):
        name = data[offset:offset + 8].rstrip(b"\0").decode("ascii", "replace")
        raw_size, raw_pointer, reloc_pointer = struct.unpack_from(
            "<III", data, offset + 16)
        nrelocs = struct.unpack_from("<H", data, offset + 32)[0]
        relocations = []
        for index in range(nrelocs):
            at = reloc_pointer + index * 10
            va, symbol, kind = struct.unpack_from("<IIH", data, at)
            relocations.append((va, symbol, kind))
        sections.append(CoffSection(name, raw_size, raw_pointer, relocations))
        offset += 40

    strings = sym_ptr + nsyms * 18
    symbols, index = [], 0
    while index < nsyms:
        entry = data[sym_ptr + index * 18:sym_ptr + index * 18 + 18]
        if entry[:4] == b"\0\0\0\0":
            at = struct.unpack_from("<I", entry, 4)[0] + strings
            name = data[at:data.index(b"\0", at)].decode("ascii", "replace")
        else:
            name = entry[:8].rstrip(b"\0").decode("ascii", "replace")
        value, section, _, storage, aux = struct.unpack_from("<IhHBB", entry, 8)
        symbols.append(CoffSymbol(name, value, section, storage))
        for _ in range(aux):
            symbols.append(CoffSymbol("", 0, 0, -1))
        index += 1 + aux
    return sections, symbols


def index_table_span(section: CoffSection, data: bytes, entries: list,
                     end: int):
    """`entries[-1] + 4` when a byte-index table fills the rest, else None.

    VC6 compresses a sparse switch into two tables: a dword table of label
    addresses, which carries the DIR32 relocations, and a dense byte table of
    indices into it, which carries none:

        mov dl, byte ptr [eax + BYTES]
        jmp dword ptr [edx*4 + DWORDS]

    The byte table sits last and owns the COMDAT's final byte, so the
    relocation run never reaches the end and the trailing-table test rejects
    the whole thing - leaving BOTH tables inside the compared span. 24 bytes of
    data then get compared against nothing on the original side, which reports
    as a length mismatch on a function whose code is byte-identical.

    The tail is verified, not assumed: every byte of an index table indexes the
    dword table, so no byte may be >= the number of dword entries. That is a
    strong test - ordinary code has bytes all over the range - and it is what
    keeps this from inventing a boundary, which is exactly what the original
    version declined to do.
    """
    dwords = entries[-1] + 4
    if dwords >= end:
        return None
    if any(dwords <= va < end
           for va, _symbol, _kind in section.relocations):
        return None                      # a relocation here means it is code
    base = section.raw_pointer
    tail = data[base + dwords:base + end]
    if not tail:
        return None
    # Trailing COMDAT padding is not part of the table and is not an index.
    tail = tail.rstrip(b"\xcc").rstrip(b"\x90")
    if not tail:
        return None
    count = len(entries)
    if any(byte >= count for byte in tail):
        return None
    return dwords


def clip_jump_table(section: CoffSection, symbols: list, data: bytes,
                    start: int, end: int) -> int:
    """Where the code stops and MSVC's switch table begins.

    Measured: under `/Gy` VC6 puts the table in the SAME `.text` COMDAT,
    immediately after the last instruction, with no separating padding. The
    ORIGINAL side excludes it - jump tables sit in the data gaps between
    functions and are inside no function's `body_ranges` - so a comparison that
    keeps it on the object side is comparing code against data. In the measured
    case 28 table bytes decoded as 14 phantom `add byte ptr [eax], al`.

    The table is identified by STRUCTURE, not by guessing: it is the run of
    DIR32 relocations at four-byte stride, ending at the COMDAT's end, whose
    targets are LABEL symbols in this same section. Walking backwards from the
    end means multiple adjacent tables collapse into one run, which is what
    `alt_get_ocean_detail`'s four tables need.

    Returns `end` unchanged when there is no such run. A dense byte-index table,
    which carries no relocations at all, is NOT detected here - the caller's
    decode-coverage check is what catches that, rather than this inventing a
    boundary it cannot see.
    """
    labels = {index for index, symbol in enumerate(symbols)
              if symbol.storage == IMAGE_SYM_CLASS_LABEL}
    entries = sorted(
        va for va, symbol, kind in section.relocations
        if kind == IMAGE_REL_I386_DIR32 and symbol in labels
        and start <= va < end)
    if not entries:
        return end
    # The run has to reach the COMDAT's end, or it is not a trailing table -
    # EXCEPT when a dense byte-index table follows it, which is what VC6 emits
    # for a sparse switch: `mov dl,[eax+BYTES]; jmp [edx*4+DWORDS]`. There the
    # dword table carries the relocations and the byte table, which carries
    # none, sits after it and owns the COMDAT's last byte.
    #
    # The tail is confirmed rather than assumed, and the test is exact: every
    # byte of an index table must be a valid index INTO the dword table. Code
    # bytes essentially never satisfy that, and the check costs one pass.
    if entries[-1] + 4 != end:
        tail = index_table_span(section, data, entries, end)
        if tail is None:
            return end
        end = tail
    clip = entries[-1]
    for va in reversed(entries[:-1]):
        if va + 4 != clip:
            break
        clip = va
    return clip


def object_code(data: bytes, want_eh: bool = False):
    """(bytes, relocation mask) for the compiled subject.

    `want_eh` selects the `.text$x` COMDAT that `/GX` emits the unwind funclets
    and the `__ehhandler` thunk into. That section carries no external symbol,
    so it is found by name rather than by symbol lookup.
    """
    sections, symbols = parse_coff(data)

    if want_eh:
        for section in sections:
            if section.name.startswith(".text$x"):
                body = data[section.raw_pointer:
                            section.raw_pointer + section.raw_size]
                mask = _object_mask(section, 0, section.raw_size)
                return body, mask
        return b"", set()

    found = []
    for index, symbol in enumerate(symbols):
        if symbol.storage != IMAGE_SYM_CLASS_EXTERNAL:
            continue
        if symbol.section <= 0 or symbol.section > len(sections):
            continue
        name = sections[symbol.section - 1].name
        if not name.startswith(".text") or name.startswith(".text$x"):
            continue
        found.append((symbol, sections[symbol.section - 1]))

    if len(found) != 1:
        names = [s.name for s, _ in found]
        raise ValueError(
            f"expected one external .text symbol, found {len(found)}: {names}. "
            f"A translation unit must define the SUBJECT and nothing else - a "
            f"helper defined alongside it gets inlined, and the original did "
            f"not inline it.")
    symbol, section = found[0]

    start, end = symbol.value, section.raw_size
    end = clip_jump_table(section, symbols, data, start, end)
    body = data[section.raw_pointer + start:section.raw_pointer + end]
    # VC6 pads COMDATs with 0x90; the original's catalogued span does not
    # include padding, so strip it. `int3` is stripped for the same reason.
    stripped = body.rstrip(b"\xcc").rstrip(b"\x90")
    mask = _object_mask(section, start, start + len(stripped))
    return stripped, mask


def _object_mask(section: CoffSection, start: int, end: int) -> set:
    """Offsets (relative to `start`) an object relocation makes unknowable."""
    masked = set()
    for va, _, kind in section.relocations:
        if kind not in (IMAGE_REL_I386_DIR32, IMAGE_REL_I386_REL32):
            continue
        for offset in range(4):
            if start <= va + offset < end:
                masked.add(va + offset - start)
    return masked


# ------------------------------------------------------------ the comparison

def decode(code: bytes, base: int = 0):
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    return list(engine.disasm(code, base))


# `ret 8` and `ret` are the same mnemonic to capstone, so the callee-pop count -
# the only ground truth for arity - never reached the old verdict. It does now.
RET_MNEMONICS = ("ret", "retf", "iret", "iretd")


def mnemonic_key(instruction) -> str:
    if instruction.mnemonic in RET_MNEMONICS and instruction.op_str:
        return f"{instruction.mnemonic} {instruction.op_str}"
    return instruction.mnemonic


def mnemonics(code: bytes, base: int = 0) -> list:
    return [mnemonic_key(i) for i in decode(code, base)]


def operand_kind(instruction):
    """The compiler's STRUCTURAL choices: registers and addressing form only.

    Immediates and displacements are dropped wholesale, and that is the point
    rather than a concession. The first cut of this kept every non-relocated
    immediate, which made the tier nearly redundant with BYTE_EXACT - if the
    mnemonics, the sizes and every constant agree, the bytes agree too, so the
    tier could almost never fire. A control written to exercise it is what
    surfaced that.

    Dropping them makes SHAPE_EXACT mean something an agent can act on: the
    instruction selection and register allocation are RIGHT and a value is
    WRONG. That is the wrong-field-offset, wrong-vtable-slot,
    wrong-loop-bound class - the most actionable failure this comparator can
    report, and the one mnemonic-only comparison is blind to.
    """
    from capstone import x86
    kinds = []
    for operand in instruction.operands:
        if operand.type == x86.X86_OP_REG:
            kinds.append(("reg", operand.reg))
        elif operand.type == x86.X86_OP_IMM:
            kinds.append(("imm",))
        elif operand.type == x86.X86_OP_MEM:
            memory = operand.mem
            kinds.append(("mem", memory.base, memory.index, memory.scale))
    return tuple(kinds)


def differing_constants(left: list, right: list) -> list:
    """Which operand values disagree, for a SHAPE_EXACT pair.

    Reported as instruction indices and both values, because "your constant is
    wrong at instruction 12" is a repair instruction and "SHAPE_EXACT" alone is
    not. Values that a relocation determines are skipped: an object file's
    placeholder is not a wrong answer.
    """
    from capstone import x86
    out = []
    for index, (a, b) in enumerate(zip(left, right)):
        for one, two in zip(a.operands, b.operands):
            if one.type != two.type:
                continue
            if one.type == x86.X86_OP_IMM and one.imm != two.imm:
                out.append((index, a.mnemonic, hex(one.imm), hex(two.imm)))
            elif one.type == x86.X86_OP_MEM and one.mem.disp != two.mem.disp:
                out.append((index, a.mnemonic,
                            hex(one.mem.disp), hex(two.mem.disp)))
    return out


def first_divergence(left: list, right: list):
    for index in range(min(len(left), len(right))):
        if left[index] != right[index]:
            return index
    return None if len(left) == len(right) else min(len(left), len(right))


def compare(original: bytes, original_mask: set, original_base: int,
            rebuilt: bytes, rebuilt_mask: set) -> dict:
    """The tiered verdict, with every tier reported.

    `original_mask` holds absolute addresses; `rebuilt_mask` holds offsets. Both
    are normalised to offsets here so the one masking rule reads the same on
    both sides.
    """
    left_mask = {a - original_base for a in original_mask}
    result = {
        "tier": "MISMATCH",
        "original_bytes": len(original),
        "rebuilt_bytes": len(rebuilt),
        "note": "",
        "mask_disagreements": 0,
    }

    # Tier 0. Equal bytes, or relocated on EITHER side - see the module
    # docstring for why "both" is wrong and what it silently selected for.
    unknowable = left_mask | rebuilt_mask
    result["masked_bytes"] = len(unknowable)
    if len(original) == len(rebuilt):
        divergence = None
        for index in range(len(original)):
            if original[index] == rebuilt[index] or index in unknowable:
                continue
            divergence = index
            break
        if divergence is None:
            # A body that is mostly relocation is mostly uncompared. Say so
            # rather than let it read as a strong match.
            result["compared_bytes"] = len(original) - len(unknowable)
            result["tier"] = "BYTE_EXACT"
            return result

    left, right = decode(original, original_base), decode(rebuilt)
    left_keys = [mnemonic_key(i) for i in left]
    right_keys = [mnemonic_key(i) for i in right]
    result["original_mnemonics"] = len(left)
    result["rebuilt_mnemonics"] = len(right)

    # Tier 1. The instruction selection and register allocation are right and a
    # VALUE is wrong. Sizes are compared per instruction rather than in total,
    # so a shorter encoding of a smaller constant is caught here rather than
    # passing as structurally identical.
    if len(left) == len(right):
        shapes_left = [(mnemonic_key(i), i.size, operand_kind(i)) for i in left]
        shapes_right = [(mnemonic_key(i), i.size, operand_kind(i)) for i in right]
        if shapes_left == shapes_right:
            result["tier"] = "SHAPE_EXACT"
            result["differing_constants"] = differing_constants(left, right)
            return result

    # Tier 2. Control flow and arithmetic shape, plus the pop count.
    index = first_divergence(left_keys, right_keys)
    if index is None:
        result["tier"] = "MNEMONIC_ONLY"
        return result

    # HOW MUCH of the body agrees, not just where it first stops agreeing.
    #
    # First-divergence alone is actively misleading on large bodies. A 1,187 B
    # function whose every call, constant, branch and store matched the
    # original reported `MISMATCH` at index 2, because one prologue `push` was
    # absent and everything downstream shifted by one. That reads identically
    # to a body that is entirely wrong, and it under-reports real progress in
    # exactly the population where progress is hardest to come by.
    #
    # A similarity ratio and the edit summary separate the two. This does NOT
    # move the tier - a body that differs anywhere is not a match, and
    # softening that would be the "honest SKIP into a flattering PASS" move
    # this project already refuses. It is reported alongside.
    import difflib
    matcher = difflib.SequenceMatcher(None, left_keys, right_keys,
                                      autojunk=False)
    common = sum(block.size for block in matcher.get_matching_blocks())
    result["mnemonic_similarity"] = round(matcher.ratio(), 4)
    result["mnemonics_in_common"] = common
    edits = [op for op in matcher.get_opcodes() if op[0] != "equal"]
    result["edit_count"] = len(edits)
    result["edits"] = [
        {"op": op, "at": i1, "original": left_keys[i1:i2][:4],
         "rebuilt": right_keys[j1:j2][:4]}
        for op, i1, i2, j1, j2 in edits[:6]]

    def at(sequence, position):
        return sequence[position] if position < len(sequence) else "<end>"

    result["note"] = (f"#{index}: original {at(left_keys, index)!r} vs "
                      f"rebuilt {at(right_keys, index)!r}")
    result["first_divergence"] = index
    # The window either side of the divergence. An index alone tells an author
    # THAT it diverged; the surrounding sequence is what tells them what the
    # original did instead, which is the whole repair signal.
    low = max(0, index - 6)
    result["context"] = {
        "at": index,
        "original": left_keys[low:index + 7],
        "rebuilt": right_keys[low:index + 7],
        "window_starts_at": low,
    }
    return result


# ---------------------------------------------------------------- compiling

def compile_unit(text: str, work: Path, stem: str, flags: str) -> bytes:
    source = work / f"{stem}.cpp"
    obj = work / f"{stem}.obj"
    source.write_text(text)
    command = ["wine", str(VC6_CL), "/nologo", *flags.split(),
               f"/Fo{obj.name}", source.name]
    result = subprocess.run(command, cwd=work, env=wine_environment(),
                            capture_output=True, text=True)
    if not obj.is_file():
        raise ValueError(f"CL produced no object: {result.stdout}{result.stderr}")
    return obj.read_bytes()


def compile_batch(units: dict, work: Path, flags: str) -> dict:
    """Compile many units in ONE invocation. 21.6x, measured.

    44 ms per isolated `wine CL`; 2 ms per file through a response file (200
    files, 0.407 s). The objects are byte-identical to isolated ones except COFF
    byte 4, the timestamp, and a unit with a syntax error does not abort the
    batch - CL reports it and carries on - so a missing `.obj` is the
    compile-failure signal rather than a reason to fall back to one-at-a-time.
    """
    for stem, text in units.items():
        (work / f"{stem}.cpp").write_text(text)
    response = work / "cl.rsp"
    response.write_text(" ".join(flags.split()) + "\n"
                        + "\n".join(f"{stem}.cpp" for stem in units) + "\n")
    done = subprocess.run(["wine", str(VC6_CL), "/nologo", "@cl.rsp"],
                          cwd=work, env=wine_environment(),
                          capture_output=True, text=True)
    # RETURNED, not stashed on the function. This used to be
    # `compile_batch.diagnostics = ...`, a single slot on a module-level
    # object, which is exactly one shared mutable cell - fine while the only
    # caller was a serial loop and a race the moment batches run at once.
    diagnostics = batch_diagnostics(done.stdout + done.stderr, units)
    out = {}
    for stem in units:
        obj = work / f"{stem}.obj"
        out[stem] = obj.read_bytes() if obj.is_file() else None
    return out, diagnostics


def compile_batches(chunks: list, flags: str, workers: int = 0) -> list:
    """Compile several batches CONCURRENTLY, one directory each.

    `compile_batch` already buys 21.6x by putting many units through a single
    response file. What it does not do is use the other fifteen cores: the
    census compiled ~13 chunks per flag set strictly one after another, which
    is most of why a full census took 12 to 19 minutes.

    EACH BATCH NEEDS ITS OWN DIRECTORY. cl writes `<stem>.obj` and reads
    `cl.rsp` from the working directory, so two batches sharing one would
    overwrite each other's response file and read each other's objects. That
    is the whole reason this is not simply a thread pool over the old loop.

    Nothing about a verdict changes. The objects are byte-identical to the
    serial ones - `compile_batch`'s own docstring records that they differ
    from isolated compiles only in COFF byte 4, the timestamp - and no batch
    can see another's files.
    """
    workers = workers or min(8, (os.cpu_count() or 2))

    def one(chunk):
        with tempfile.TemporaryDirectory() as tmp:
            return compile_batch(chunk, Path(tmp), flags)

    if len(chunks) < 2 or workers < 2:
        return [one(chunk) for chunk in chunks]
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(one, chunks))


def batch_diagnostics(output: str, units: dict) -> dict:
    """{stem: what CL printed about it}.

    CL announces each source file on a line of its own and then reports that
    file's errors, so the last announcement seen attributes everything after
    it.

    This exists because the output was being thrown away: `compile_batch` ran
    with `capture_output=True` and read neither stream. That is why 1,988
    NO_COMPILE rows carry no error text - the census recorded "CL emitted no
    object" for every one of them, which says only that the compile failed,
    and 923 hand-written failures have been unreadable ever since.
    """
    stems = {f"{stem}.cpp": stem for stem in units}
    found: dict = {}
    current = None
    for line in output.splitlines():
        stripped = line.strip()
        if stripped in stems:
            current = stems[stripped]
            continue
        if current and stripped:
            found.setdefault(current, []).append(stripped)
    return {stem: "\n".join(text) for stem, text in found.items()}


def wine_environment() -> dict:
    include = VC6_ROOT / "INCLUDE"
    library = VC6_ROOT / "LIB"
    return dict(os.environ,
                WINEPREFIX=str(VC6_PREFIX),
                WINEDEBUG="-all",
                INCLUDE=f"Z:{include}".replace("/", "\\"),
                LIB=f"Z:{library}".replace("/", "\\"))


def available() -> str:
    if not VC6_CL.is_file():
        return f"{VC6_CL} is absent"
    if shutil.which("wine") is None:
        return "wine is not on PATH"
    if not DEFAULT_EXE.is_file():
        return f"{DEFAULT_EXE} is absent"
    return ""


# ------------------------------------------------------------------ driving

def _better(candidate: dict, incumbent: dict) -> bool:
    """Is `candidate` a better result than `incumbent`?

    Tier first, then HOW CLOSE. Ranking on tier alone means that when every
    flag set scores MISMATCH the first one tried wins by default, and the first
    one tried is `/Oy-` - so a frameless function was reported against the
    framed build even though the frameless attempt was visibly closer.
    Measured on `?vx_malloc@Caviar@@QAAXK@Z`: the kept result inserted a whole
    prologue and epilogue the original does not have.

    Similarity, then a later first divergence, then fewer bytes of difference.
    """
    left = TIER_ORDER.index(candidate["tier"])
    right = TIER_ORDER.index(incumbent["tier"])
    if left != right:
        return left < right
    for key in ("mnemonic_similarity", "first_divergence"):
        mine, theirs = candidate.get(key), incumbent.get(key)
        if mine is not None and theirs is not None and mine != theirs:
            return mine > theirs
        if (mine is None) != (theirs is None):
            return theirs is None
    mine = abs(candidate.get("original_bytes", 0)
               - candidate.get("rebuilt_bytes", 0))
    theirs = abs(incumbent.get("original_bytes", 0)
                 - incumbent.get("rebuilt_bytes", 0))
    return mine < theirs


def match_functions(pe, rows: dict, shared: set, subjects: list,
                    flags: str, chunk: int = 120) -> dict:
    """`match_function` for MANY units at once. {address: outcome}.

    Identical arithmetic to the singular form and deliberately so - it is the
    ratchet's measurement, and a faster answer that is a different answer is
    worthless. What changes is only WHEN the compiler is invoked: one process
    per unit becomes one per batch, and batches run concurrently.

    `collect` compiled 1,210 units one at a time and this file's own caller
    describes that pass as "over an hour", which is the comment's words for
    how a ratchet stops being checked. The census had the same shape and went
    from 12-19 minutes to 58 seconds on this change.

    THE FLAG SETS STAY SEQUENTIAL, and that is not incidental. `match_function`
    tries them in order and STOPS at the first BYTE_EXACT; running them
    together would compile every unit four times to learn nothing new. Each
    pass here compiles only what is still unsettled, which reproduces the
    early exit exactly.
    """
    outcomes, pending, layouts, bases = {}, {}, {}, {}
    for address, source, _stem in subjects:
        row = rows.get(address)
        if row is None:
            outcomes[address] = {
                "address": address, "tier": "REFUSED",
                "refusal_reason": "address is not a catalogued function"}
            continue
        layout = classify_body(pe, row, shared)
        base = {
            "address": address, "name": row.get("name", ""),
            "size": int(row.get("size") or 0),
            "span_classes": {
                "primary": len(layout.primary), "eh": len(layout.eh),
                "shared": len(layout.shared), "selfmod": len(layout.selfmod)},
        }
        if layout.refusal:
            outcomes[address] = {**base, "tier": "REFUSED",
                                 "refusal_reason": layout.refusal}
            continue
        if layout.shared:
            outcomes[address] = {
                **base, "tier": "SHARED_TAIL",
                "refusal_reason": (
                    f"{len(layout.shared)} span(s) are COMDAT-folded and "
                    f"claimed by another function; no per-function verdict "
                    f"is well defined")}
            continue
        layouts[address], bases[address], pending[address] = layout, base, source

    attempts = [flags] if flags and flags not in FLAG_SETS else list(FLAG_SETS)
    best = {}
    for attempt in attempts:
        if not pending:
            break
        stems = {f"u{address:08x}": address for address in sorted(pending)}
        groups = [dict(list(stems.items())[at:at + chunk])
                  for at in range(0, len(stems), chunk)]
        batches = compile_batches(
            [{stem: pending[stems[stem]] for stem in group} for group in groups],
            attempt)
        for objects, diagnostics in batches:
            for stem, data in objects.items():
                address = stems[stem]
                layout = layouts[address]
                low, high = layout.primary[0]
                if data is None:
                    candidate = {"tier": "NO_COMPILE",
                                 "refusal_reason": diagnostics.get(
                                     stem, "CL emitted no object")}
                else:
                    try:
                        rebuilt, rebuilt_mask = object_code(data)
                        candidate = compare(
                            original_span_bytes(pe, low, high),
                            original_relocation_mask(pe, low, high),
                            low, rebuilt, rebuilt_mask)
                    except ValueError as error:
                        candidate = {"tier": "NO_COMPILE",
                                     "refusal_reason": str(error)}
                candidate["flags"] = attempt
                if address not in best or _better(candidate, best[address]):
                    best[address] = candidate
        for address in [a for a in pending
                        if best.get(a, {}).get("tier") == "BYTE_EXACT"]:
            del pending[address]

    for address, base in bases.items():
        outcome = dict(base)
        outcome.update(best.get(address, {"tier": "NO_COMPILE",
                                          "refusal_reason": "not compiled"}))
        outcome["eh_spans_uncompared"] = sum(
            high - low for low, high in layouts[address].eh)
        outcomes[address] = outcome
    return outcomes


def match_function(pe, rows: dict, shared: set, address: int, source: str,
                   work: Path, flags: str, stem: str = "") -> dict:
    """One function, end to end. The unit of the fan-out."""
    row = rows.get(address)
    if row is None:
        return {"address": address, "tier": "REFUSED",
                "refusal_reason": "address is not a catalogued function"}

    layout = classify_body(pe, row, shared)
    outcome = {
        "address": address,
        "name": row.get("name", ""),
        "size": int(row.get("size") or 0),
        "span_classes": {
            "primary": len(layout.primary), "eh": len(layout.eh),
            "shared": len(layout.shared), "selfmod": len(layout.selfmod)},
    }
    if layout.refusal:
        outcome.update(tier="REFUSED", refusal_reason=layout.refusal)
        return outcome
    if layout.shared:
        outcome.update(
            tier="SHARED_TAIL",
            refusal_reason=(f"{len(layout.shared)} span(s) are COMDAT-folded and "
                            f"claimed by another function; no per-function "
                            f"verdict is well defined"))
        return outcome

    low, high = layout.primary[0]
    original = original_span_bytes(pe, low, high)
    original_mask = original_relocation_mask(pe, low, high)

    # Try each flag set and keep the BEST verdict. Half this image keeps its
    # frame pointer and half omits it, and no single `/Oy` setting can match
    # both - see FLAG_SETS.
    attempts = [flags] if flags else list(FLAG_SETS)
    if flags and flags in FLAG_SETS:
        attempts = list(FLAG_SETS)
    best = None
    for index, attempt in enumerate(attempts):
        try:
            data = compile_unit(source, work,
                                f"{stem or f'f{address:08x}'}_{index}", attempt)
            rebuilt, rebuilt_mask = object_code(data)
        except ValueError as error:
            candidate = {"tier": "NO_COMPILE", "refusal_reason": str(error)}
        else:
            candidate = compare(original, original_mask, low, rebuilt,
                                rebuilt_mask)
        candidate["flags"] = attempt
        if best is None or _better(candidate, best):
            best = candidate
        if best["tier"] == "BYTE_EXACT":
            break

    outcome.update(best)
    outcome["eh_spans_uncompared"] = sum(h - l for l, h in layout.eh)
    return outcome


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address", help="hex address of the catalogued function")
    parser.add_argument("--source", type=Path, required=True,
                        help="a complete translation unit defining the subject "
                             "and nothing else")
    parser.add_argument("--flags", default=MEASURED_FLAGS)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--keep", type=Path)
    arguments = parser.parse_args()

    reason = available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine and the pinned executable; set VC6_ROOT to relocate it.")
        return 0

    import pefile
    pe = pefile.PE(str(DEFAULT_EXE))
    rows = load_rows()
    shared = shared_span_index(rows)

    holder = None
    if arguments.keep:
        arguments.keep.mkdir(parents=True, exist_ok=True)
        work = arguments.keep
    else:
        holder = tempfile.TemporaryDirectory()
        work = Path(holder.name)
    try:
        outcome = match_function(pe, rows, shared, int(arguments.address, 16),
                                 arguments.source.read_text(), work,
                                 arguments.flags)
    finally:
        if holder:
            holder.cleanup()

    if arguments.json:
        print(json.dumps(outcome, indent=2))
    else:
        print(f"0x{outcome['address']:08X}  {outcome.get('name', '')}")
        print(f"    {outcome['tier']}"
              + (f"    {outcome['note']}" if outcome.get("note") else "")
              + (f"    {outcome['refusal_reason']}"
                 if outcome.get("refusal_reason") else ""))
        if "original_bytes" in outcome:
            print(f"    original {outcome['original_bytes']:4d} B    "
                  f"rebuilt {outcome['rebuilt_bytes']:4d} B")
        if "mnemonic_similarity" in outcome:
            print(f"    {outcome['mnemonics_in_common']} of "
                  f"{outcome.get('original_mnemonics', '?')} mnemonics agree "
                  f"({outcome['mnemonic_similarity']:.1%}), "
                  f"{outcome['edit_count']} edit(s)")
            for edit in outcome.get("edits", [])[:4]:
                print(f"      {edit['op']:8s} at #{edit['at']}: "
                      f"original {edit['original']} vs yours {edit['rebuilt']}")
        for entry in outcome.get("differing_constants", [])[:12]:
            index, mnemonic, was, now = entry
            print(f"    constant #{index} {mnemonic}: original {was}, "
                  f"yours {now}")
        context = outcome.get("context")
        if context:
            print(f"    around #{context['at']} "
                  f"(window from #{context['window_starts_at']}):")
            print(f"      original: {' '.join(context['original'])}")
            print(f"      yours   : {' '.join(context['rebuilt'])}")
    return recovery_exit_code(outcome["tier"])


if __name__ == "__main__":
    sys.exit(main())
