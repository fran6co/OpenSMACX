"""Legacy relocation-masked i386 listing comparison over source mappings.

Compiler commands, option semantics and process management are supplied by the
caller through a required compiler callable (the framework Compiler client also
implements it). This module never starts a compiler or locates a project.

This historical comparer discounts relocation bytes. Its BYTE_EXACT tier does
not establish reference identity, C++ structure, lifetime or full acceptance;
use the strict reference-aware checks for those independent requirements.
"""

from __future__ import annotations

import difflib
import functools
import hashlib
import re
import shlex
import struct
from dataclasses import dataclass, field, replace
from enum import Enum
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, Cs, x86

from ..application_binary import verify_records
from ..binary import RelocationKind
from ..associated_source import parse_associated
from ..measurement import Measurement
from ..model import Result, Span
from ..pe_coff import Coff, CoffRecords, PeImage
from ..source_inventory import image_pin
from .eh_regions import bind_eh_regions, eh_action_semantic_difference, original_eh_action_count
from .grammar import is_assembler_source
from .image_patch import (PREFIX as IMAGE_PATCH_PREFIX, covering_run, parse_image_patches,
                          patches_for_spans)
from .model import (DecompilationState, FRAGMENT_EH_CLEANUP, FRAGMENT_TERMINAL, Mode, Tier,
                    fragment_fact_kind)

@dataclass(frozen=True)
class Listing:
    """One disassembled body: its bytes, where they sit, and what the linker
    has still to fill in.

    THE LINES ARE DERIVED, and that is the point of the shape. A listing is a
    rendering of bytes; storing both invites them to disagree, and the only
    thing that can be compared honestly is the bytes. `lines` renders on
    demand and caches, so a reader still gets text without the text ever
    being the record of what this is.

    THE MASK IS WHY THIS TYPE EXISTS AT ALL. An object file's `push <global>`
    is `68 00 00 00 00` with a relocation on the immediate; the image has the
    linked address in those four bytes. Comparing the rendering calls that a
    divergence, which is how bodies that reproduce the shipped bytes exactly
    were reported as MISMATCH. A byte a relocation determines is not a wrong
    answer - but a comparison can only discount it if it is told which bytes
    those are, and rendered text cannot say.

    NOT ALL OF IT IS CODE. Under `/Gy` a `switch`'s jump table is laid in
    the same COMDAT as the function it serves, so the object hands back
    code followed by data with no marker between them - and decoding that
    table as instructions desynchronises the whole comparison, because the
    object holds zeros where the linker has still to write addresses and
    the image holds the addresses, so the two sides do not even agree on
    where one instruction ends. `code_bytes` says where the instructions
    stop; everything past it is `data`, compared as bytes.
    """

    code: bytes
    base: int                        # the address the code is disassembled at
    mask: frozenset[int] = frozenset()   # offsets into `code`, not addresses
    flags: str = ""                  # the invocation that produced it, "" for
                                     # the image, which was not invoked
    code_bytes: int | None = None    # how much of `code` decodes as
                                     # instructions; None means all of it
    references: tuple = ()           # compiled side only: one
                                     # `CodeReference` per DIR32/REL32 field,
                                     # so a masked field can still be asked
                                     # where it points - see `reference_mismatches`
    table_start: int | None = None   # compiled side only: the first byte
                                     # of in-section data (a `switch` table)
                                     # by the object's own relocations - see
                                     # `_object_table_start`; None when none
    bounded: bool = False            # compiled side only: longer than its
                                     # original span, which must not be read
                                     # past - a local function's neighbor is
                                     # another body, never its switch table

    @property
    def instruction_bytes(self) -> int:
        """How many of `code`'s bytes decode as instructions."""
        return len(self.code) if self.code_bytes is None else self.code_bytes

    @property
    def data(self) -> bytes:
        """The bytes past the code - a jump table, or nothing."""
        return self.code[self.instruction_bytes:]

    @functools.cached_property
    def instructions(self) -> list:
        """The decoded instructions, operands and all."""
        return _decode(self.code[:self.instruction_bytes], self.base)

    @functools.cached_property
    def lines(self) -> list[str]:
        """`0xADDR  bytes  mnemonic operands`, one per instruction."""
        return [_render(instruction) for instruction in self.instructions]

    @functools.cached_property
    def mnemonics(self) -> tuple[str, ...]:
        """The instruction sequence, for similarity and for MNEMONIC_ONLY.

        `ret` keeps its operand: `ret` and `ret 4` are different
        instructions, and the pop count is exactly what a
        calling-convention slip changes.
        """
        return tuple(_mnemonic_key(i) for i in self.instructions)

    @functools.cached_property
    def shapes(self) -> tuple:
        """Instruction selection AND register allocation, constants dropped.

        What SHAPE_EXACT compares. Immediates and displacements go wholesale
        - keeping them would make the tier nearly redundant with BYTE_EXACT,
        because if the mnemonics, the registers and every constant agree
        then the bytes agree too. Dropping them makes the tier say something
        a reader can act on: the instructions are RIGHT and a value is
        WRONG, which is the wrong-field-offset, wrong-vtable-slot,
        wrong-loop-bound class.
        """
        return tuple((_mnemonic_key(i), _operand_kinds(i))
                     for i in self.instructions)


# ------------------------------------------------------------------ capstone

_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True                    # `shapes` needs the operands

# `ret 8` and `ret` are one mnemonic to capstone, so the callee-pop count -
# the only ground truth for arity - would never reach a verdict without this.
_RET = ("ret", "retf", "iret", "iretd")


def _decode(code: bytes, base: int) -> list:
    return list(_ENGINE.disasm(code, base))


def _render(instruction) -> str:
    return (f"0x{instruction.address:08X}  "
            f"{' '.join(f'{b:02X}' for b in instruction.bytes):<24}  "
            f"{instruction.mnemonic} {instruction.op_str}").rstrip()


def _mnemonic_key(instruction) -> str:
    if instruction.mnemonic in _RET and instruction.op_str:
        return f"{instruction.mnemonic} {instruction.op_str}"
    return instruction.mnemonic


def _operand_kinds(instruction) -> tuple:
    """Registers and addressing form; every constant discarded."""
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


def call_symbols(obj: bytes, symbol: str, emitted: str = "") -> list[str]:
    """The symbols a body's REL32 relocations name - what it CALLS.

    The object side of a call is a relocation, not an address: the linker
    writes the target. So "which helper is this body calling" is answerable
    only from the relocation's symbol, and that is exactly the question when
    the image makes fewer calls than this tree does.

    Names are returned as the compiler emits them, in the order the call sites
    appear. A duplicate means the body calls it more than once.
    """
    _machine, n_sections, _ts, sym_ptr, n_syms = \
        struct.unpack_from("<HHIII", obj, 0)
    str_ptr = sym_ptr + n_syms * 18

    def name_of(index: int) -> str:
        off = sym_ptr + index * 18
        field = obj[off:off + 8]
        if field[:4] == b"\x00\x00\x00\x00":
            (at,) = struct.unpack("<I", field[4:])
            end = obj.index(b"\x00", str_ptr + at)
            return obj[str_ptr + at:end].decode()
        return field.rstrip(b"\x00").decode()

    wanted = emitted or symbol
    section = None
    for entry, value, index in text_symbols(obj):
        if entry in (wanted, "_" + wanted):
            section, start = index, value
            break
    if section is None:
        return []
    off = 20 + section * 40
    raw_size, _raw_ptr = struct.unpack_from("<II", obj, off + 16)
    reloc_ptr, = struct.unpack_from("<I", obj, off + 24)
    n_relocs, = struct.unpack_from("<H", obj, off + 32)
    out = []
    for index in range(n_relocs):
        at = reloc_ptr + index * 10
        va, symbol_index, kind = struct.unpack_from("<IIH", obj, at)
        if kind != 0x14:                 # REL32: a call or a near jump
            continue
        if start <= va < raw_size:
            out.append(name_of(symbol_index))
    return out


def allocation_only(original: Listing, compiled: Listing) -> str:
    """"" if the two differ ONLY in which registers hold what, else why not.

    THE TEST A SEMANTIC CLAIM RESTS ON, and it has to be stricter than any
    tier here. SHAPE_EXACT says "the instructions are right and a VALUE is
    wrong" - a wrong field offset, a wrong loop bound - which is precisely a
    semantic difference. MNEMONIC_ONLY allows both that and a different
    addressing form. Neither supports "the same program, allocated
    differently".

    So this compares the mnemonic, the operand TYPES, and every immediate and
    displacement, and permits a difference only in register numbers. What
    survives is a body that computes the same values from the same places and
    keeps them somewhere else.

    A displacement is compared as it is WRITTEN, so `[eax-2]` and `[eax+4]`
    differ even when the two bases make them the same address. That is
    deliberate: proving those equal needs to know what the base holds, this
    does not, and a claim that guessed would be worth nothing.
    """
    def relocated(instruction, listing) -> bool:
        """Whether a relocation owns any of this instruction's bytes.

        Those bytes are the linker's, so the operand VALUE on the object side
        is a placeholder - `mov eax, [0]` against the image's
        `mov eax, [0x68faf0]` is the same instruction. The byte comparison
        already discounts them; this has to as well or every reference to a
        global reads as a difference.
        """
        start = instruction.address - listing.base
        return any(offset in listing.mask
                   for offset in range(start, start + instruction.size))

    left, right = original.instructions, compiled.instructions
    if len(left) != len(right):
        return (f"{len(right)} instructions against {len(left)} - a different "
                f"program, not a different allocation")
    # A BRANCH IS COMPARED BY WHERE IT LANDS, NOT BY ITS ADDRESS. One extra
    # byte anywhere shifts every later target, so comparing the immediates
    # reports every jump in the body as a difference and hides the one real
    # cause. The same jump to the same INSTRUCTION is the same control flow.
    index_of = ({i.address: n for n, i in enumerate(left)},
                {i.address: n for n, i in enumerate(right)})
    # A BASE REGISTER MAY HOLD A DIFFERENT VALUE, and then every displacement
    # off it differs by the SAME delta - `init_goals` bases its loop on
    # `&goal.priority` where this tree bases it on `&goal.x`, so every offset
    # is two apart and every effective address agrees. One delta per base is
    # provable; two deltas off one base is a real value difference, which is
    # what SHAPE_EXACT means and is not an allocation.
    deltas: dict = {}
    for a, b in zip(left, right):
        if _mnemonic_key(a) != _mnemonic_key(b):
            continue
        for one, two in zip(a.operands, b.operands):
            if one.type != two.type or one.type != x86.X86_OP_MEM:
                continue
            if one.mem.disp == two.mem.disp:
                continue
            if relocated(a, original) or relocated(b, compiled):
                continue
            deltas.setdefault(one.mem.base, set()).add(
                two.mem.disp - one.mem.disp)
    shifted = {base for base, seen in deltas.items() if len(seen) == 1}

    for index, (a, b) in enumerate(zip(left, right)):
        if _mnemonic_key(a) != _mnemonic_key(b):
            return (f"instruction {index}: {_mnemonic_key(a)} against "
                    f"{_mnemonic_key(b)}")
        if len(a.operands) != len(b.operands):
            return f"instruction {index}: {a.mnemonic} takes a different form"
        masked = relocated(a, original) or relocated(b, compiled)
        for one, two in zip(a.operands, b.operands):
            if one.type != two.type:
                return (f"instruction {index}: {a.mnemonic} operand is a "
                        f"different KIND")
            if masked:
                continue
            if masked:
                continue
            if one.type == x86.X86_OP_IMM and one.imm != two.imm:
                inside = (index_of[0].get(one.imm), index_of[1].get(two.imm))
                if inside[0] is not None and inside[0] == inside[1]:
                    continue        # same target instruction, shifted address
                return (f"instruction {index}: {a.mnemonic} immediate "
                        f"0x{one.imm:x} against 0x{two.imm:x}")
            if one.type == x86.X86_OP_MEM:
                if (one.mem.disp != two.mem.disp
                        and one.mem.base not in shifted):
                    return (f"instruction {index}: {a.mnemonic} displacement "
                            f"0x{one.mem.disp:x} against 0x{two.mem.disp:x}")
                if one.mem.scale != two.mem.scale:
                    return (f"instruction {index}: {a.mnemonic} index scale "
                            f"differs")
    return ""


def differing_constants(original: Listing, compiled: Listing) -> list:
    """Which operand values disagree in a SHAPE_EXACT pair.

    "Your constant is wrong at instruction 12, 0x838 against 0x83C" is a
    repair instruction; "SHAPE_EXACT" alone is not.
    """
    out = []
    for index, (a, b) in enumerate(zip(original.instructions,
                                       compiled.instructions)):
        for one, two in zip(a.operands, b.operands):
            if one.type != two.type:
                continue
            if one.type == x86.X86_OP_IMM and one.imm != two.imm:
                out.append((index, a.mnemonic, hex(one.imm), hex(two.imm)))
            elif one.type == x86.X86_OP_MEM and one.mem.disp != two.mem.disp:
                out.append((index, a.mnemonic,
                            hex(one.mem.disp), hex(two.mem.disp)))
    return out


class CompileFailed(Exception):
    """The supplied compiler failed. Carries its diagnostic.

    Its own type because a loop treats it as a RESULT - `Tier.NO_COMPILE` -
    while a stale or missing `symbol` fact is a defect in the annotation and
    stays a `ValueError` the caller must deal with. Both used to be one
    exception, so a body that would not build and an annotation that lied
    were indistinguishable to everything above.
    """


class SpanComparisonRefused(ValueError):
    """The legacy whole-function comparison cannot cover every mapped span."""


# SECTION CHARACTERISTICS. A section that is both executable and writable is
# code that can rewrite itself - which is what the copy protection in this
# image does, and why nothing a compiler emits can equal a body that is not
# the body at rest.
IMAGE_SCN_MEM_EXECUTE = 0x20000000
IMAGE_SCN_MEM_WRITE = 0x80000000


# -------------------------------------------------------------- the original


@functools.lru_cache(maxsize=4)
def _image_bytes(exe: Path) -> bytes:
    """The image, read once. `_pe_bytes` is called per record and per flag
    set, and re-reading two and a half megabytes each time is a cost the
    sweep pays thousands of times over for nothing. `_sections` and
    `_image_relocations` beside it are cached for the same reason."""
    return exe.read_bytes()


def _pe_bytes(exe: Path, address: int, size: int) -> bytes:
    """The image's bytes at `address`, mapped through the section table."""
    data = _image_bytes(exe)
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    if data[e_lfanew:e_lfanew + 4] != b"PE\x00\x00":
        raise ValueError(f"{exe.name}: not a PE image")
    _machine, n_sections, _ts, _sp, _ns, opt_size, _ch = \
        struct.unpack_from("<HHIIIHH", data, e_lfanew + 4)
    (image_base,) = struct.unpack_from("<I", data, e_lfanew + 24 + 28)
    table = e_lfanew + 24 + opt_size
    for i in range(n_sections):
        off = table + i * 40
        va, raw_size, raw_ptr = struct.unpack_from("<III", data, off + 12)
        begin = image_base + va
        if begin <= address < begin + raw_size:
            delta = address - begin
            if delta + size > raw_size:
                raise ValueError(
                    f"{exe.name}: 0x{address:08X}+{size} runs past the "
                    f"section's raw data")
            return data[raw_ptr + delta:raw_ptr + delta + size]
    raise ValueError(f"{exe.name}: 0x{address:08X} is in no section")


def _base_relocations(data: bytes) -> frozenset[int]:
    """Every address the loader rewrites when the image is rebased.

    Only HIGHLOW (type 3) marks anything: it names four bytes holding an
    address the linker filled in, and those are exactly the bytes an object
    file leaves at zero for a relocation to fix. Type 0 is ABSOLUTE, a
    block-alignment pad that names no byte at all - this image carries 362 of
    them, and counting those would mask real code.
    """
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    optional = e_lfanew + 24
    (image_base,) = struct.unpack_from("<I", data, optional + 28)
    # PE32 lays its data directories at optional-header offset 96, eight bytes
    # each; the base relocation table is the sixth.
    directory_rva, directory_size = struct.unpack_from(
        "<II", data, optional + 96 + 5 * 8)
    if not directory_rva or not directory_size:
        return frozenset()

    n_sections, = struct.unpack_from("<H", data, e_lfanew + 6)
    opt_size, = struct.unpack_from("<H", data, e_lfanew + 20)
    table = e_lfanew + 24 + opt_size

    def file_offset(rva: int) -> int | None:
        for index in range(n_sections):
            off = table + index * 40
            va, raw_size, raw_ptr = struct.unpack_from("<III", data, off + 12)
            if va <= rva < va + raw_size:
                return raw_ptr + (rva - va)
        return None

    start = file_offset(directory_rva)
    if start is None:
        return frozenset()
    addresses, cursor, end = [], start, start + directory_size
    while cursor + 8 <= end:
        page, block = struct.unpack_from("<II", data, cursor)
        if block < 8:
            break
        for entry_at in range(cursor + 8, min(cursor + block, end), 2):
            (entry,) = struct.unpack_from("<H", data, entry_at)
            if entry >> 12 == 3:                       # HIGHLOW
                addresses.append(image_base + page + (entry & 0xFFF))
        cursor += block
    return frozenset(addresses)


@functools.lru_cache(maxsize=4)
def _sections(exe: Path) -> tuple:
    """(name, low, high) for each section, from the image's own table."""
    data = _image_bytes(exe)
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    _machine, count, _ts, _sp, _ns, opt_size, _ch = \
        struct.unpack_from("<HHIIIHH", data, e_lfanew + 4)
    (image_base,) = struct.unpack_from("<I", data, e_lfanew + 24 + 28)
    table = e_lfanew + 24 + opt_size
    out = []
    for index in range(count):
        off = table + index * 40
        name = data[off:off + 8].rstrip(b"\x00").decode(errors="replace")
        va, raw_size, _raw_ptr = struct.unpack_from("<III", data, off + 12)
        (characteristics,) = struct.unpack_from("<I", data, off + 36)
        out.append((name, image_base + va, image_base + va + raw_size,
                    characteristics))
    return tuple(out)


def section_of(exe: Path | str, address: int) -> str:
    """The name of the section holding `address`, or "" if none does."""
    for name, low, high, _characteristics in _sections(Path(exe)):
        if low <= address < high:
            return name
    return ""


def rewrites_itself(exe: Path | str, address: int) -> bool:
    """Is `address` in a section that is executable AND writable?

    THE IMAGE SAYS SO ITSELF, in the characteristics of its own section
    table, and that is the whole reason this is a question rather than a
    constant. It began as `SELFMOD_RANGE = (0x00664000, 0x00669000)` copied
    out of tools/byte_match.py - the bounds of one section of one image,
    transcribed into source where nothing could keep them true. Reading the
    bounds instead of copying them was better; reading the PROPERTY is
    better again, because the name `_SELFMOD` is this image's label for it
    and the flags are what actually make it so. In this image exactly one
    section carries both: `.text` is 0x60000020, `_SELFMOD` is 0xE0000020.
    """
    wanted = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE
    for _name, low, high, characteristics in _sections(Path(exe)):
        if low <= address < high:
            return characteristics & wanted == wanted
    return False


@functools.lru_cache(maxsize=4)
def _image_relocations(exe: Path) -> frozenset[int]:
    """`_base_relocations` for a file, read once per image."""
    return _base_relocations(_image_bytes(exe))


def _image_mask(exe: Path, low: int, high: int) -> frozenset[int]:
    """Offsets in `[low, high)`, relative to `low`, a base relocation owns.

    A HIGHLOW entry names the FIRST of four bytes, so each one masks four.
    """
    relocated = _image_relocations(exe)
    masked = set()
    # Walked over the SPAN rather than over the relocation table: the table
    # holds 107,760 entries and a body is tens to thousands of bytes, so
    # asking the span is orders of magnitude less work per record.
    for address in range(low - 3, high):
        if address in relocated:
            for byte in range(4):
                if low <= address + byte < high:
                    masked.add(address + byte - low)
    return frozenset(masked)


def original_asm(record: DecompilationState, exe: Path | str,
                 extent: int | None = None) -> Listing:
    """The shipped image's assembly for `record` - the bytes at its
    primary span in `exe`, disassembled. A record can carry no spans - a
    fresh annotation the catalogue has not stamped yet - and then nothing
    locates it in the image, so the call says so.

    `extent` READS PAST THE SPAN, and the span still says where the code
    stops. A `switch`'s jump table sits immediately after the function it
    serves and is not in the catalogue - the span is the CODE, which is
    what 1,010 live claims already assume - but the object hands back both,
    so a comparison that wants to prove the table too has to be able to see
    the image's copy of it. The caller passes the object section's length;
    what comes back carries the code and the data with the boundary marked.
    """
    if not record.image_spans:
        raise ValueError(
            f"{record.address_hex}: the record carries no spans - nothing "
            f"locates it in the image")
    exe = Path(exe)
    if not exe.is_file():
        raise ValueError(f"no executable at {exe}")
    low, high = record.image_spans[0]
    code = _pe_bytes(exe, low, high - low)
    # SYMMETRY WITH THE OBJECT SIDE. A COMDAT is padded to alignment with
    # `int3`/`nop` and the catalogued span can include that padding, so both
    # sides are stripped or neither is: `0x0050F630` is a thirteen-byte body
    # in a sixteen-byte span.
    end = len(code)
    while end > 0 and code[end - 1] in (0x90, 0xCC):
        end -= 1
    code = code[:end]
    if extent is not None and extent > len(code):
        code = _pe_bytes(exe, low, extent)
        return Listing(code=code, base=low, code_bytes=end,
                       mask=_image_mask(exe, low, low + extent))
    return Listing(code=code, base=low,
                   mask=_image_mask(exe, low, low + len(code)))


# --------------------------------------------------------------- the compile


def text_symbols(obj: bytes) -> list[tuple[str, int, int]]:
    """Every external `.text` symbol in a COFF object: (name, value, section).

    PUBLIC BECAUSE THE LOOKUP'S FAILURE NEEDS AN ANSWER. When a record's
    catalogued name is not among these, `_coff_function_masked` refuses and
    says the fix is a `symbol` fact - and writing that fact means knowing
    what the object DOES define. This is that list, and nothing here decides
    which entry is right: guessing belongs in the migration that writes the
    fact, never in what measures.
    """
    return [(name, value, section) for name, value, section, storage
            in _subject_candidates(obj) if storage == 2]


def _subject_candidates(obj: bytes) -> list[tuple[str, int, int, int]]:
    """Legacy external .text candidates plus local definitions to validate.

    A local definition is not yet a function or an independently bounded body.
    Keep its storage class so selection cannot prefer a same-named external.
    """
    machine, n_sections, _ts, sym_ptr, n_syms, _opt, _ch = \
        struct.unpack_from("<HHIIIHH", obj, 0)
    if machine != 0x14C:
        raise ValueError("not an i386 COFF object")
    str_ptr = sym_ptr + n_syms * 18
    names = []
    for index in range(n_sections):
        off = 20 + index * 40
        names.append(obj[off:off + 8].rstrip(b"\x00").decode())

    def sym_name(field: bytes) -> str:
        if field[:4] == b"\x00\x00\x00\x00":
            (off,) = struct.unpack("<I", field[4:])
            end = obj.index(b"\x00", str_ptr + off)
            return obj[str_ptr + off:end].decode()
        return field.rstrip(b"\x00").decode()

    code = [_is_code_section(obj, index, names[index]) for index in range(n_sections)]
    out, i = [], 0
    while i < n_syms:
        off = sym_ptr + i * 18
        name = sym_name(obj[off:off + 8])
        value, section_no, _type, storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if 0 < section_no <= len(names) and (
                storage == 3 or storage == 2 and code[section_no - 1]):
            out.append((name, value, section_no - 1, storage))
        i += 1 + n_aux
    return out


IMAGE_SCN_CNT_CODE = 0x00000020


def _is_code_section(obj: bytes, index: int, name: str) -> bool:
    """A compiler's `.text`, or an assembler segment declared as code.

    VC6 names every code section `.text`. A MASM module can name its own
    code segment - the original's `_SELFMOD` is one - and its external
    procedures are functions exactly as a `.text` one is. Executable code
    contents, not the spelling, decide it; a data section never qualifies.
    """
    if name == ".text":
        return True
    (characteristics,) = struct.unpack_from("<I", obj, 20 + index * 40 + 36)
    wanted = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE
    return characteristics & wanted == wanted


def _next_function_offset(obj: bytes, section: int, value: int) -> int | None:
    """The next function symbol after `value` in the same object section.

    `/Gy` puts each compiled function in its own COMDAT, so the section end
    is the function end. An assembler module puts several procedures in one
    segment; each one ends where the next declared procedure (COFF function
    type 0x20) begins. Labels and jump-table symbols are not functions and
    never cut a body short.
    """
    _machine, _n_sections, _ts, sym_ptr, n_syms, _opt, _ch = \
        struct.unpack_from("<HHIIIHH", obj, 0)
    following, i = [], 0
    while i < n_syms:
        off = sym_ptr + i * 18
        other, section_no, kind, _storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if section_no - 1 == section and kind == 0x20 and other > value:
            following.append(other)
        i += 1 + n_aux
    return min(following, default=None)


def _local_function_masked(raw: bytes, name: str, *,
                           original_extent: int | None = None) -> tuple[bytes, frozenset[int]]:
    """One explicitly named TU-local function, never a slice between labels.

    Reuse the bounded COFF reader for the additional supported domain. Plain
    shared code, missing section ownership and associated code stay unsupported;
    accepting STATIC in the legacy section-tail extractor would be unsafe.
    This is still masked-byte measurement, not reference or link verification.
    """
    obj = Coff(raw)
    definitions_named = [entry for entry in obj.symbols.values()
                         if entry.name == name and entry.section > 0]
    if len(definitions_named) != 1:
        raise ValueError(f"{name} is ambiguous in the object")
    symbol = obj.function(name)
    section = obj.sections[symbol.section - 1]
    definitions = [entry for entry in obj.symbols.values()
                   if entry.section == symbol.section and entry.storage == 3
                   and entry.type == 0 and entry.name == section.name]
    auxiliary = obj.auxiliary.get(definitions[0].index, b"") if len(definitions) == 1 else b""
    if (len(auxiliary) != 18 or struct.unpack_from("<H", auxiliary, 12)[0] != 0
            or symbol.section in obj.associations or len(obj.code_sections(symbol)) != 1):
        raise ValueError("local function requires one independently defined code COMDAT without associated code")
    code, relocations, _ = obj.subject(symbol)
    end = len(code)
    while end and code[end - 1] in (0x90, 0xCC):
        end -= 1
    code = code[:end]
    if not code:
        raise ValueError("local function has no code after alignment padding")
    masked = set()
    for relocation in relocations:
        if not 0 <= relocation.offset <= len(code) - 4:
            raise ValueError("local function relocation crosses its code boundary")
        field = set(range(relocation.offset, relocation.offset + 4))
        if field & masked:
            raise ValueError("local function has overlapping relocations")
        masked.update(field)
    # The legacy external comparer can extend the original read for switch
    # tails. A local mapping must not borrow a neighboring original body, so
    # a longer candidate is measured against its own span only: a mismatch.
    if original_extent is not None and len(code) > original_extent:
        raise LocalExtentExceeded(code, frozenset(masked))
    return code, frozenset(masked)


class LocalExtentExceeded(ValueError):
    """A local candidate longer than its original span; compare unextended."""

    def __init__(self, code: bytes, mask: frozenset[int]):
        super().__init__("local function exceeds its explicit original image span")
        self.code, self.mask = code, mask


def _coff_function_masked(obj: bytes, symbol: str, emitted: str = "",
                          *, spans=None) -> tuple[bytes, frozenset[int]]:
    """The symbol's code out of a COFF object - the legacy external `.text`
    path, or a separately validated exclusive local code COMDAT.

    The external path takes the section named
    by the symbol, from the symbol's value to the section's end, trailing
    NOP/INT3 padding stripped. With `/Gy` each function is its own COMDAT
    section, so the section end is the function end."""
    _machine, n_sections = struct.unpack_from("<HH", obj, 0)
    sections = []
    for i in range(n_sections):
        off = 20 + i * 40
        name = obj[off:off + 8].rstrip(b"\x00").decode()
        raw_size, raw_ptr = struct.unpack_from("<II", obj, off + 16)
        reloc_ptr, = struct.unpack_from("<I", obj, off + 24)
        n_relocs, = struct.unpack_from("<H", obj, off + 32)
        sections.append((name, raw_size, raw_ptr, reloc_ptr, n_relocs))

    candidates = _subject_candidates(obj)

    # ONE RULE, AND IT READS A FACT. `emitted` is what the annotation says
    # this tree's compiler produces for the piece; `name` is what the IMAGE
    # calls it, and the two differ whenever the source re-expresses a
    # compiler-generated construct - an adjustor thunk, an atexit destructor,
    # a scalar deleting destructor - under a name of its own.
    #
    # THERE IS DELIBERATELY NO FALLBACK. Earlier versions guessed: a symbol
    # sharing the mangled head, then an identifier scraped out of the
    # region's definition head. Both existed to paper over a catalogued name
    # that was wrong or absent, and a lookup that guesses returns the
    # assembly of a NEIGHBOURING function, which reads as a mismatch nobody
    # can explain. The guessing lives in the migration that writes the fact
    # (`reader.region_identifiers`); what measures reads it.
    wanted = emitted or symbol
    matches = [e for e in candidates
               if e[0] == wanted or (not emitted and e[3] == 2 and e[0] == "_" + symbol)]
    if any(storage == 3 for _, _, _, storage in matches):
        if len(matches) != 1:
            raise ValueError(f"{wanted} is ambiguous in the object")
        if spans is not None and len(spans) != 1:
            raise ValueError("local function measurement requires one original image span")
        extent = spans[0][1] - spans[0][0] if spans is not None else None
        return _local_function_masked(obj, wanted, original_extent=extent)
    if not matches:
        # TWO DIFFERENT FAULTS, SAID DIFFERENTLY. A recorded symbol that is
        # absent is a STALE fact - the annotation made a claim and the
        # compiler disagrees, so the fix is in the annotation. No recorded
        # symbol is a MISSING fact, and the fix is to write one.
        raise ValueError(
            f"{symbol}: the annotation records the emitted symbol "
            f"{emitted!r}, which is not among the object's .text symbols "
            f"- the fact is stale"
            if emitted else
            f"{symbol} not found among the object's .text symbols, and the "
            f"annotation records no `symbol` fact saying what this tree "
            f"emits for it")
    if len({(value, sec) for _n, value, sec, _storage in matches}) > 1:
        raise ValueError(f"{wanted} is ambiguous in the object")
    if spans is not None and len(spans) != 1:
        raise SpanComparisonRefused(
            "complete function measurement requires one original image span")
    _name, value, sec, _storage = matches[0]
    _sec_name, raw_size, raw_ptr, reloc_ptr, n_relocs = sections[sec]
    end_offset = _next_function_offset(obj, sec, value)
    code = obj[raw_ptr + value:raw_ptr + (raw_size if end_offset is None else min(raw_size, end_offset))]
    end = len(code)
    while end > 0 and code[end - 1] in (0x90, 0xCC):
        end -= 1
    code = code[:end]

    # THE OBJECT'S OWN RELOCATIONS, in the same shape the image side returns:
    # offsets into `code`. DIR32 is an absolute address the linker writes,
    # REL32 a displacement it computes; both leave four bytes the object
    # cannot know and the image does.
    masked = set()
    for index in range(n_relocs):
        at = reloc_ptr + index * 10
        va, _symbol_index, kind = struct.unpack_from("<IIH", obj, at)
        if kind not in (0x0006, 0x0014):               # DIR32, REL32
            continue
        for byte in range(4):
            offset = va + byte - value
            if 0 <= offset < len(code):
                masked.add(offset)
    return code, frozenset(masked)


@dataclass(frozen=True)
class CodeReference:
    """One relocated field of a compiled body: where, how, and to what.

    `offset` is into the listing's `code`; `addend` is the object's own field,
    which is what the linker adds to the target's address. `literal` holds a
    string-literal target's bytes from `addend` through its terminator - see
    `string_literal_bytes` - and is None for every other target.
    """
    offset: int
    relative: bool                   # REL32 rather than DIR32
    symbol: str
    storage: int                     # COFF storage class: 2 external, 3 static
    addend: int
    literal: bytes | None = None


def string_literal_bytes(obj, symbol, addend: int) -> bytes | None:
    """A string literal's bytes in `obj` from `addend` through its terminator.

    VC6 names an unpooled literal `$SG<n>` (static, in `.data`) and a pooled
    one `??_C@...` (a `/GF` COMDAT; `??_C@_1` is wide, terminated by an
    aligned NUL pair). Any other symbol, an addend outside the section, or a
    missing terminator answers None and the field stays masked. The bytes are
    the object's own section contents, never the image's.
    """
    if not symbol.name.startswith(("$SG", "??_C@")) or not 0 < symbol.section <= len(obj.sections):
        return None
    data = obj.sections[symbol.section - 1].data
    start = symbol.value + addend
    if not 0 <= start < len(data):
        return None
    width = 2 if symbol.name.startswith("??_C@_1") else 1
    for end in range(start, len(data) - width + 1, width):
        if data[end:end + width] == bytes(width):
            return data[start:end + width]
    return None


def _render_literal(raw: bytes) -> str:
    text = raw.rstrip(b"\0").decode("latin-1")
    shown = repr(text)[1:-1]
    return "'" + (shown if len(shown) <= 60 else shown[:57] + "...") + "'"


@dataclass(frozen=True)
class ReferenceTargets:
    """Original addresses for emitted symbols, from source bindings only.

    `external` names globally scoped symbols; `local` is keyed by the resolved
    marker host and names TU-local (storage class 3) definitions. Anything not
    named here - section symbols, labels, literals, imports, ambiguous names -
    stays a masked field, exactly as before. A target must never come from the
    operand under test, so every address here is a source mapping's own.
    """
    external: dict = field(default_factory=dict)
    local: dict = field(default_factory=dict)

    def address(self, reference: CodeReference, host: Path) -> int | None:
        if reference.storage == 2:
            return self.external.get(reference.symbol)
        if reference.storage == 3:
            return self.local.get((Path(host).resolve(), reference.symbol))
        return None


def build_reference_targets(functions, objects=()) -> ReferenceTargets:
    """Unambiguous emitted-symbol addresses from function and object bindings.

    `functions` yields (host, address, emitted names, local); `objects` yields
    (emitted symbol, address) for direct object bindings. A name bound to more
    than one address is dropped rather than guessed, so it stays masked.
    """
    external, local = {}, {}
    for host, address, names, is_local in functions:
        for name in names:
            if is_local:
                local.setdefault((Path(host).resolve(), name), set()).add(address)
            else:
                external.setdefault(name, set()).add(address)
    for name, address in objects:
        external.setdefault(name, set()).add(address)
    return ReferenceTargets(
        external={name: next(iter(values)) for name, values in external.items() if len(values) == 1},
        local={key: next(iter(values)) for key, values in local.items() if len(values) == 1})


def _selected_subject(obj: bytes, record: DecompilationState):
    """(records, symbol) for the record's selected body, or None if unclear.

    Mirrors the selection in `_coff_function_masked`; when that cannot be
    reproduced unambiguously the caller learns nothing from the object.
    """
    try:
        parsed = CoffRecords(obj)
    except ValueError:
        return None
    wanted = record.symbol or record.name
    found = [item for item in parsed.symbols.values()
             if item.section > 0 and item.storage in (2, 3)
             and (item.name == wanted
                  or (not record.symbol and item.storage == 2 and item.name == "_" + record.name))]
    if len({(item.value, item.section) for item in found}) != 1:
        return None
    return parsed, found[0]


def _object_table_start(obj: bytes, record: DecompilationState,
                        code_size: int) -> int | None:
    """Where the selected body's in-section data begins, by the object's own
    word, or None when the object shows none.

    A `switch` addresses its tables FORWARD: `jmp [reg*4 + $L]` and
    `mov reg8, [reg + $L]` carry a DIR32 to a label of the body's own section
    lying after the instruction, while the table's entries point BACK at case
    labels. Code reaches code through REL32 displacements, never through an
    absolute self-section address, so the lowest forward self-section DIR32
    target is the first table byte. This is read from the compiled object, not
    from the original's extent, so a body whose code is a few bytes longer or
    shorter than the shipped one still has its tables accounted as data.
    """
    selected = _selected_subject(obj, record)
    if selected is None:
        return None
    parsed, subject = selected
    section = parsed.sections[subject.section - 1]
    starts = []
    for va, index, kind in section.relocations:
        target = parsed.symbols.get(index)
        if kind != 0x0006 or target is None or target.section != subject.section:
            continue
        field = va - subject.value
        if not 0 <= field <= code_size - 4:
            continue
        addend = int.from_bytes(section.data[va:va + 4], "little", signed=True)
        at = target.value + addend - subject.value
        if field < at < code_size:
            starts.append(at)
    return min(starts, default=None)


def _subject_references(obj: bytes, record: DecompilationState,
                        code_size: int) -> tuple[CodeReference, ...]:
    """The relocated fields of the record's selected body, or () if unclear.

    Mirrors the selection in `_coff_function_masked`; when that cannot be
    reproduced unambiguously, no reference is reported and the comparison is
    the masked one it always was.
    """
    selected = _selected_subject(obj, record)
    if selected is None:
        return ()
    parsed, subject = selected
    section = parsed.sections[subject.section - 1]
    rows = []
    for va, index, kind in section.relocations:
        if kind not in (0x0006, 0x0014):
            continue
        offset = va - subject.value
        target = parsed.symbols.get(index)
        if target is None or not 0 <= offset <= code_size - 4:
            continue
        addend = int.from_bytes(section.data[va:va + 4], "little", signed=True)
        rows.append(CodeReference(offset, kind == 0x0014, target.name, target.storage, addend,
                                  string_literal_bytes(parsed, target, addend)))
    return tuple(rows)


def reference_mismatches(original: Listing, compiled: Listing,
                         targets: ReferenceTargets | None, host: Path,
                         image=None) -> list[str]:
    """Relocated fields whose bound target plus addend is not the image's.

    THE MASK HIDES TWO FACTS, not one: which symbol a field names, and how far
    into it. Masking is right for the unknown linked address; it is wrong for
    the addend, which the object does carry. `VehDrawFacetNormals1[index]`
    and `VehDrawFacetNormals1[index - 400]` differ only in that addend, and
    both measured BYTE_EXACT against an image that reads the latter. Only
    fields whose symbol has an independent source binding are asked; the rest
    remain masked. Offsets are positional, so this is meaningful only once
    the masked comparison already agrees over the whole extent.

    A STRING LITERAL HAS NO BINDING BUT CARRIES ITS VALUE. `push "NOTINTRIAD"`
    and `push "TERRASEA"` differ only in a masked field, yet the literal's
    bytes are in the object. When `image(address, size)` reads the original
    image, an absolute field naming a literal is asked whether the image's
    bytes at its address (the same addend included) are the literal's,
    through the terminator. Content, not address: a pooled and an unpooled
    copy of the same text both pass.
    """
    out = []
    for reference in compiled.references:
        if reference.offset + 4 > len(original.code):
            continue
        address = targets.address(reference, host) if targets is not None else None
        if address is None:
            if reference.literal is not None and image is not None and not reference.relative:
                wrong = _literal_mismatch(original, reference, image)
                if wrong:
                    out.append(wrong)
            continue
        value = address + reference.addend
        if reference.relative:
            value -= original.base + reference.offset + 4
        observed = int.from_bytes(original.code[reference.offset:reference.offset + 4], "little")
        if value & 0xFFFFFFFF != observed:
            actual = (observed + original.base + reference.offset + 4) & 0xFFFFFFFF \
                if reference.relative else observed
            expected = (address + reference.addend) & 0xFFFFFFFF
            out.append(f"+0x{reference.offset:X} {reference.symbol}"
                       f"{reference.addend:+#x}: source 0x{expected:08X}, image 0x{actual:08X}")
    return out


def _literal_mismatch(original: Listing, reference: CodeReference, image) -> str | None:
    """A diagnostic when the image's bytes at the field's address are not the literal."""
    observed = int.from_bytes(original.code[reference.offset:reference.offset + 4], "little")
    expected = reference.literal
    try:
        found = image(observed, len(expected))
    except ValueError:
        found = None
    if found == expected:
        return None
    label = f"+0x{reference.offset:X} {reference.symbol}{reference.addend:+#x}: literal {_render_literal(expected)}"
    if found is None:
        return f"{label} vs image 0x{observed:08X} unreadable"
    width = 2 if reference.symbol.startswith("??_C@_1") else 1
    shown = b""
    while len(shown) < 256:          # the image's own text, for the reader
        try:
            unit = image(observed + len(shown), width)
        except ValueError:
            break
        if unit == bytes(width):
            break
        shown += unit
    return f"{label} vs image {_render_literal(shown)} at 0x{observed:08X}"


def image_reader(exe: Path | str):
    """`image(address, size)` over the original image, for `reference_mismatches`."""
    exe = Path(exe)
    return lambda address, size: _pe_bytes(exe, address, size)


@dataclass(frozen=True)
class RegionCandidate:
    """Unmeasured emitted regions; ownership still needs original-image proof."""
    object_bytes: bytes = field(repr=False)
    emitted_symbol: str
    flags: str
    mapping_identity: tuple


def _mapping_identity(record: DecompilationState) -> tuple:
    return record.path.resolve(), record.line, record.address, record.name, record.symbol, record.image_spans


def compiled_asm(record: DecompilationState, command: list[str],
                 flags: str | tuple[str, ...] | list[str],
                 source: Path | str | None = None, *, compiler) -> Listing | RegionCandidate:
    """Compile through the caller's environment and select a mapped COFF body.

    `compiler(source, arguments, flags)` receives tuples of argument tokens,
    returns nonempty object bytes, and raises OSError/ValueError on failure.
    The framework Compiler client implements this same boundary. The function
    is pulled out of the object by the
    record's mangled name, disassembled at the record's address so the
    listing lines up with `original_asm`.

    `source` COMPILES SOMETHING ELSE IN ITS PLACE - a candidate spelling of
    the same function, living outside the tree. See `compare_source`. The
    subject is still selected by the RECORD's name, so a candidate that does
    not define it is a `ValueError` naming the symbol it looked for, which
    is the honest answer: the file compiled and did not contain the piece.
    """
    if record.mode is Mode.UNBOUND:
        raise ValueError("UNBOUND mapping has no emitted body to compile")
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    if isinstance(flags, str):
        arguments, label = tuple(shlex.split(flags)), flags
    elif (isinstance(flags, (tuple, list))
          and all(isinstance(word, str) and "\0" not in word for word in flags)):
        arguments = tuple(flags)
        label = shlex.join(arguments)  # Display only; never reparsed to compile.
    else:
        raise ValueError("compiler flag trial must be a string or an array of argument tokens")
    try:
        obj = compiler(Path(source) if source else record.path,
                       tuple(command), arguments)
    except (OSError, ValueError) as error:
        raise CompileFailed(str(error)) from error
    if not isinstance(obj, bytes) or not obj:
        raise CompileFailed("compiler returned no object bytes")
    return subject_asm(obj, record, label)


def subject_asm(obj: bytes, record: DecompilationState,
                flags: str = "") -> Listing | RegionCandidate:
    """The record's own function, pulled out of an object already compiled.

    The half of `compiled_asm` that is not the compile, so that one object
    can answer for every subject its file defines.
    """
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    if len(record.image_spans) > 1:
        # This is only candidate selection, not a binding or a match. The
        # original-image EH graph must independently join every region before
        # compare_subject can compare any of them.
        wanted = record.symbol or record.name
        parsed = Coff(obj)
        definitions = [item for item in parsed.symbols.values()
                       if item.name == wanted and item.section > 0 and item.storage in (2, 3)]
        if not definitions and not record.symbol:
            wanted = "_" + record.name
            definitions = [item for item in parsed.symbols.values()
                           if item.name == wanted and item.section > 0 and item.storage in (2, 3)]
        if len(definitions) != 1:
            raise ValueError(f"{wanted}: expected one defined mapped symbol; "
                             "the annotation is absent or ambiguous in this object")
        try:
            root = parsed.function(wanted)
            # One code section: the candidate emits no cleanup region, which
            # compare_subject measures once the original pair is proved.
            if len(record.image_spans) != 2 or len(parsed.code_sections(root)) not in (1, 2):
                raise ValueError("no unique root/associated-code pair")
        except ValueError as error:
            raise SpanComparisonRefused(
                "complete function measurement requires one original image span "
                "or a supported independently rooted EH region pair: " + str(error)) from error
        return RegionCandidate(obj, wanted, flags, _mapping_identity(record))
    bounded = False
    try:
        code, mask = _coff_function_masked(obj, record.name, record.symbol, spans=record.image_spans)
    except LocalExtentExceeded as longer:
        code, mask, bounded = longer.code, longer.mask, True
    return Listing(code=code, base=record.address, mask=mask, flags=flags, bounded=bounded,
                   references=_subject_references(obj, record, len(code)),
                   table_start=_object_table_start(obj, record, len(code)))


# ------------------------------------------------------------------ disasm


# -------------------------------------------------------------- comparison


@dataclass
class AsmComparison:
    """How far one assembly listing reproduces another."""
    verdict: Tier                   # see model.Tier

    # WHAT THE VERDICT RESTS ON: bytes, and how many of them a relocation
    # took out of the question. These two decide BYTE_EXACT.
    compared_bytes: int = 0
    masked_bytes: int = 0

    # WHAT A READER LOOKS AT. Counted over INSTRUCTIONS, not lines, and the
    # distinction is not pedantry - these were named `*_count` and
    # `matching_lines` when a listing WAS a list of lines and the comparison
    # was over text. It is not; a listing renders on demand. An instruction
    # count is a fact about the code, and a line count would be a fact about
    # the rendering.
    original_instructions: int = 0
    compiled_instructions: int = 0
    matching_instructions: int = 0  # positionally identical, once relocated
                                    # bytes are discounted on both sides
    mnemonic_similarity: float = 0.0  # 0..1 across the instruction sequences
    first_divergence: int | None = None   # first differing instruction
    context: tuple[list[str], list[str]] = \
        field(default_factory=lambda: ([], []))
                                    # (original, compiled) lines around the
                                    # divergence, rendered as they REALLY are
    differing_constants: tuple = ()  # SHAPE_EXACT only: (index, mnemonic,
                                    # original, compiled) per wrong value
    allocation_only: str = "?"      # "" when the two listings differ ONLY in
                                    # which registers hold what, otherwise why
                                    # not. See `allocation_only()`; this is
                                    # what a SEMANTIC claim rests on, and it
                                    # is stricter than every tier above.

    # THE DATA TAIL - a `/Gy` COMDAT's jump table, which belongs to the
    # function and is not in the catalogue's span. Zero for the bodies that
    # have none, which is most of them.
    data_bytes: int = 0
    data_divergence: int | None = None    # first differing offset into
                                          # `code`, relocations discounted
    diagnostic: str = ""            # compiler, refusal or comparison context
    flags: str = ""                 # the flag set that produced `compiled`
    # NON-AUTHORITATIVE. Only set on a REFUSED/unscored (or strictly failed
    # associated) verdict when the caller asked for diagnostics: a legacy
    # relocation-masked comparison of whatever compiled body and original
    # span(s) could be obtained. It never changes `verdict`, is never
    # claimable, and recording never requests it. See `diagnostic_comparison`.
    diagnostic_comparison: dict | None = None
    # DECLARED POST-LINK IMAGE PATCHES that this comparison applied: one dict
    # per `RECOVERY_IMAGE_PATCH` span (see `apply_image_patches`). Never empty
    # silently: a BYTE_EXACT verdict with entries here means "exact outside
    # the declared spans, image bytes inside them exactly the declared patch
    # form"; the compiled bytes each entry lists were NOT compared with the
    # image, because the image no longer holds what the compiler emitted.
    image_patches: tuple = ()


# ------------------------------------------------- declared image patches

IMAGE_PATCH_NOTE = ("exact outside declared post-link image patch span(s) {spans}; the image holds "
                    "exactly the declared patch bytes there and the compiled instructions they "
                    "replaced are not verified against the image")


def record_image_patches(record: DecompilationState) -> tuple:
    """The `RECOVERY_IMAGE_PATCH` declarations inside `record`'s mapped spans.

    Read from the marker host, never from a candidate source: a variant deck
    cannot add, move or drop a declaration. A malformed declaration anywhere
    in the host, or one that crosses this record's span boundary, refuses.
    """
    try:
        raw = record.path.read_bytes()
    except OSError:
        return ()
    if IMAGE_PATCH_PREFIX.encode() not in raw:
        return ()
    try:
        return patches_for_spans(parse_image_patches(raw.decode("utf-8", errors="replace")),
                                 record.image_spans)
    except ValueError as error:
        raise SpanComparisonRefused(f"declared image patch refused: {error}") from error


def apply_image_patches(patches, original: Listing, compiled: Listing) -> tuple[Listing, tuple, tuple]:
    """`compiled` as the declared post-link patches would have left it.

    A patch is admitted for this pair only when it lies wholly inside the
    original listing's instruction bytes. Three things are then checked:

    - THE IMAGE holds exactly the declared bytes over the span. Anything
      else refuses the comparison: the declaration does not describe the
      pinned image.
    - THE COMPILED BODY decodes to complete instructions that begin at the
      span's first byte and end at its last, so the patch replaces whole
      instructions and nothing straddles an edge, and its absolute (DIR32)
      fields inside the span sit exactly where the image's relocation table
      still has entries for the span (see below). If not, the patch is NOT
      applied and the comparison runs on the compiled bytes as they are -
      which cannot be exact, since the image holds the patch there.
    - Everything outside the span is compared as usual by the caller.

    Applying writes the declared bytes over the compiled span and drops the
    relocations and references inside it, which is what patching the linked
    file did. Returns the listing, one report dict per applied patch, and a
    note per declared patch that could not be applied.
    """
    applied, notes = [], []
    low, high = original.base, original.base + original.instruction_bytes
    for patch in patches:
        if not (low <= patch.start and patch.end <= high):
            continue
        if compiled.base != original.base:
            raise SpanComparisonRefused("declared image patch requires listings at one base address")
        start, end = patch.start - low, patch.end - low
        if original.code[start:end] != patch.expected:
            raise SpanComparisonRefused(
                f"declared image patch {patch.describe()} does not describe the image: it holds "
                f"{original.code[start:end].hex()} there, the declaration states {patch.expected.hex()}")
        if end > compiled.instruction_bytes:
            notes.append(f"declared image patch {patch.describe()} not applied: the compiled body "
                         "ends before the span does")
            continue
        run = covering_run(((instruction.address - compiled.base, instruction.size)
                            for instruction in compiled.instructions), start, end)
        inside = [compiled.instructions[index] for index in run or ()]
        if run is None:
            notes.append(f"declared image patch {patch.describe()} not applied: no run of complete "
                         "compiled instructions covers exactly that span")
            continue
        if any(offset < start < offset + 4 or offset < end < offset + 4
               for offset in (reference.offset for reference in compiled.references)):
            notes.append(f"declared image patch {patch.describe()} not applied: a compiled "
                         "relocation field crosses the span edge")
            continue
        # STALE BASE RELOCATIONS ARE EVIDENCE. Patching the linked file leaves
        # the image's relocation table alone, so an absolute field the
        # overwritten code held still has its HIGHLOW entry. The compiled
        # span must put a DIR32 field at exactly those places and nowhere
        # else: the one part of the replaced code the image still records.
        stale = frozenset(offset for offset in original.mask if start <= offset < end)
        absolute = frozenset(reference.offset + byte for reference in compiled.references
                             if not reference.relative and start <= reference.offset < end
                             for byte in range(4))
        if stale != absolute:
            notes.append(
                f"declared image patch {patch.describe()} not applied: the image keeps base "
                "relocations for bytes "
                + (", ".join(f"0x{low + offset:08X}" for offset in sorted(stale)) or "none")
                + " of the span and the compiled span has absolute fields at "
                + (", ".join(f"0x{low + offset:08X}" for offset in sorted(absolute)) or "none"))
            continue
        applied.append({
            "start": f"0x{patch.start:08X}", "end": f"0x{patch.end:08X}", "form": patch.form,
            **({"jmp_address": f"0x{patch.jmp_address:08X}", "jmp_target": f"0x{patch.jmp_target:08X}"}
               if patch.jmp_address is not None else {}),
            "declaration_line": patch.line,
            "image_bytes": patch.expected.hex(),
            "image_stale_base_relocation_bytes": [f"0x{low + offset:08X}" for offset in sorted(stale)],
            "compiled_bytes_not_compared": compiled.code[start:end].hex(),
            "compiled_instructions_not_compared": [_render(instruction) for instruction in inside],
            "compiled_relocations_not_compared": sorted(
                reference.symbol for reference in compiled.references
                if start <= reference.offset < end)})
        compiled = replace(
            compiled,
            code=compiled.code[:start] + patch.expected + compiled.code[end:],
            mask=frozenset(offset for offset in compiled.mask if not start <= offset < end),
            references=tuple(reference for reference in compiled.references
                             if not start <= reference.offset < end))
    return compiled, tuple(applied), tuple(notes)


def _with_image_patches(result: AsmComparison, applied: tuple, notes: tuple) -> AsmComparison:
    """Attach the applied patches and any unapplied-declaration notes."""
    if not applied and not notes:
        return result
    parts = [result.diagnostic] if result.diagnostic else []
    if applied and result.verdict is Tier.BYTE_EXACT:
        parts.append(IMAGE_PATCH_NOTE.format(
            spans=", ".join(f"{item['start']}-{item['end']}" for item in applied)))
    parts.extend(notes)
    return replace(result, image_patches=tuple(applied), diagnostic="; ".join(parts))


def _discounted(instruction, mask: frozenset[int], base: int) -> bytes:
    """An instruction's bytes with the ones a relocation owns blanked."""
    offset = instruction.address - base
    return bytes(0 if offset + index in mask else byte
                 for index, byte in enumerate(instruction.bytes))


def _agree(original: Listing, compiled: Listing, index: int) -> bool:
    """Do the two instructions at `index` say the same thing?

    PER INSTRUCTION, AND THAT IS THE POINT. This compared whole bodies -
    zeroing both buffers at the union of their masks and re-disassembling -
    which needed the two to be the same LENGTH, because a mask is a set of
    offsets and offsets only name the same bytes in bodies laid out alike.
    One instruction of a different size broke that for everything after it,
    so the guard refused to discount anything at all.

    Measured on `_WinMain@16`: 459 bytes against 458, one `mov` three bytes
    where the other is two, and the whole-body rule reported 35 of 141
    instructions agreeing. 138 of 141 actually do; the other three are the
    real divergence and its knock-on. A reader given the first number
    chases instruction 3, which is a relocated address and not a difference
    at all.

    An instruction's offsets are its own, so drift earlier in the body
    cannot corrupt them - and nothing is re-disassembled, so a mask can no
    longer split instructions that were never there.

    THE MASKS ARE UNIONED, not taken side by side. An image `call` carries
    no base relocation, because a REL32 displacement does not move when the
    image does; the object's does. Discounting only what each side itself
    relocates would call every one of those a divergence.
    """
    a = original.instructions[index]
    b = compiled.instructions[index]
    if a.size != b.size:
        return False
    a_offset, b_offset = a.address - original.base, b.address - compiled.base
    for step in range(a.size):
        if a_offset + step in original.mask or b_offset + step in compiled.mask:
            continue                       # one side or the other relocates it
        if a.bytes[step] != b.bytes[step]:
            return False
    return True


# The instructions a function can end on. A conditional jump does not end
# one, and none of the `j<cc>` mnemonics begins with `jmp`.
_TERMINATORS = ("ret", "jmp")


def _split_at(compiled: Listing, extent: int) -> Listing:
    """`compiled` with its code/data boundary marked at `extent`, when what
    follows the shipped function's extent is provably not part of it.

    THE JUMP TABLE RIDES THE COMDAT. Under `/Gy` VC6 lays a `switch`'s table
    in the same section as the code it serves, and the object marks no
    boundary between them - so `_coff_function_masked`, which takes the
    symbol's value to the section's end, hands back the table decoded as
    instructions. Three of this tree's ratchet regressions are exactly that:
    23 of 23, 39 of 39 and 35 of 35 shipped instructions agree, and the
    compiled listing then runs 42, 19 and 18 instructions past the function.

    TWO CONDITIONS, AND BOTH ARE THE GUARD. The split is taken only when the
    decode lands EXACTLY on `extent` at an instruction boundary and the
    instruction ending there is a `ret` or an unconditional `jmp`. A body
    that is genuinely longer than the shipped one rarely satisfies either;
    one that satisfies both has reproduced the shipped function over its
    whole extent and ended it, so what follows is data or is unreachable.
    Splitting without the guard would call a body that merely STARTS the
    same way a match, which is the failure this ratchet exists to prevent.

    NOTHING IS DISCARDED. The tail becomes `data` and is still compared,
    byte for byte against the image's own copy - see `compare_asm`. Cutting
    it off instead would stop checking the jump table, and a table whose
    cases were reordered is a real defect in a real body.
    """
    if len(compiled.code) <= extent or compiled.code_bytes is not None:
        return compiled
    end = _body_end(compiled, extent, padding=False)
    if end is not None:
        return replace(compiled, code_bytes=end)
    # THE OBJECT'S OWN BOUNDARY, when the original's extent is not it. A
    # register or slot tie that moves the compiled code by a few bytes made
    # the extent miss, and the tables were then decoded as junk
    # instructions. The object says where its tables begin (`table_start`,
    # from its relocations); that boundary is taken under the same guard,
    # with the alignment `npad` VC6 lays between the terminator and the
    # table moved to the data side - where the catalogued span, which ends
    # on the terminator, leaves the image's own `npad`. Only the accounting
    # changes: a body of a different length cannot be BYTE_EXACT, because
    # its whole extent and its data still have to agree byte for byte.
    start = compiled.table_start
    if start is not None and 0 < start < len(compiled.code):
        end = _body_end(compiled, start, padding=True)
        if end is not None:
            return replace(compiled, code_bytes=end)
    return compiled


# VC6's `npad` fill before a 4-aligned `switch` table: the one- to
# three-byte forms. Only these may stand between a terminator and a table.
_NPAD = (b"\x90", b"\x8b\xff", b"\x8d\x49\x00")


def _body_end(compiled: Listing, boundary: int, *, padding: bool) -> int | None:
    """Where the code ends if decoding `compiled` up to `boundary` lands
    exactly on it and the last instruction - past at most three bytes of
    `npad` when `padding` - is a `ret` or an unconditional `jmp`; else None.
    """
    instructions = _decode(compiled.code[:boundary], compiled.base)
    if sum(instruction.size for instruction in instructions) != boundary:
        return None                         # not on an instruction boundary
    end, fill = boundary, 0
    while (padding and instructions and bytes(instructions[-1].bytes) in _NPAD
           and fill + instructions[-1].size <= 3):
        fill += instructions[-1].size
        end -= instructions.pop().size
    if instructions and instructions[-1].mnemonic.startswith(_TERMINATORS):
        return end
    return None


def _data_divergence(original: Listing, compiled: Listing) -> int | None:
    """The first offset into the tails that disagrees, or None.

    Offsets are into `code`, so they can be read against the masks
    directly, and a relocated byte is discounted on either side exactly as
    it is between instructions. A jump table is four-byte addresses the
    linker writes, so in the object it is zeros and every one of them is
    relocated; the byte-index table beside it is not, and that is the half
    a wrong `switch` would break.
    """
    if len(original.data) != len(compiled.data):
        return min(original.instruction_bytes, compiled.instruction_bytes)
    start = original.instruction_bytes
    for step in range(len(original.data)):
        offset = start + step
        if offset in original.mask or offset in compiled.mask:
            continue
        if original.data[step] != compiled.data[step]:
            return offset
    return None


def compare_asm(original: Listing, compiled: Listing) -> AsmComparison:
    """How far the compiled listing reproduces the original one.

    FOUR TIERS, EACH A WEAKER STATEMENT THAN THE LAST. BYTE_EXACT: every
    compared byte agrees. SHAPE_EXACT: the instruction selection AND the
    register allocation agree and a CONSTANT does not - the most actionable
    miss there is, because it names a wrong field offset or vtable slot
    rather than a wrong idea. MNEMONIC_ONLY: the instruction sequence
    agrees but the registers or the addressing do not. MISMATCH: neither.

    The report says how many instructions match outright, how similar the
    sequences are, where the first divergence is with a window of both
    sides around it, and - for a SHAPE_EXACT pair - which constants differ.

    RELOCATED BYTES ARE DISCOUNTED, instruction by instruction - see
    `_agree`. An object file's `push <global>` is `68 00 00 00 00` and the
    image's is `68` plus the linked address; the four bytes disagree
    because one side has been linked and the other has not, which is not a
    fact about the source. `context` still shows the REAL lines, because a
    reader chasing a divergence wants the address the image actually holds.

    BYTE_EXACT compares the complete equal-length extent directly. Decoding
    relocated table words can produce different instruction boundaries, and
    an undecodable suffix must not disappear from the byte verdict. The
    instruction counts and context remain diagnostics, not code/data proof.
    """
    compiled = _split_at(compiled, original.instruction_bytes)
    n, m = len(original.instructions), len(compiled.instructions)
    matching = 0
    first_divergence = None
    context: tuple = ([], [])
    for index in range(min(n, m)):
        if _agree(original, compiled, index):
            matching += 1
        elif first_divergence is None:
            first_divergence = index
            lo = max(0, index - 3)
            # The window shows what each side REALLY holds, relocated bytes
            # included: a reader chasing a divergence wants the address the
            # image carries, not the zero it was discounted to.
            context = (original.lines[lo:index + 4],
                       compiled.lines[lo:index + 4])
    if first_divergence is None and n != m:
        first_divergence = min(n, m)
        lo = max(0, min(n, m) - 3)
        context = (original.lines[lo:], compiled.lines[lo:])

    # DECODED FROM THE BYTES, not parsed back out of the rendering: the
    # sequences are a fact about the code, and the text is a view of it.
    # NO AUTOJUNK: difflib's default heuristic discards every element that
    # is more than 1% of a 200+ item sequence - `mov`, `push`, `call` - so a
    # large body scored near zero however close it was (0x004986D0: 0.07
    # with the heuristic, 0.79 without, on the same listings).
    similarity = difflib.SequenceMatcher(
        None, original.mnemonics, compiled.mnemonics, autojunk=False).ratio()

    # THE TAIL IS PART OF THE ANSWER. A body whose instructions all agree
    # and whose jump table does not has not reproduced the function.
    data_divergence = _data_divergence(original, compiled)

    mask = original.mask | compiled.mask
    whole_extent_equal = (
        len(original.code) == len(compiled.code)
        and all(left == right or offset in mask
                for offset, (left, right) in enumerate(zip(original.code, compiled.code)))
    )
    constants: list = []
    diagnostic = ""
    if whole_extent_equal and data_divergence is None:
        verdict = Tier.BYTE_EXACT
        if n != m or matching != n:
            diagnostic = (
                "Complete relocation-masked extent agrees; instruction decoding "
                "differs within masked bytes. No code/data boundary or target "
                "agreement is inferred."
            )
    elif original.shapes == compiled.shapes:
        verdict = Tier.SHAPE_EXACT
        constants = differing_constants(original, compiled)
    elif original.mnemonics == compiled.mnemonics:
        verdict = Tier.MNEMONIC_ONLY
    else:
        verdict = Tier.MISMATCH
    # COUNTED OVER WHAT WAS ACTUALLY COMPARED. When the split declines, the
    # image's tail was read but never asked about, and counting it here
    # would let a body that reproduced only its code report the byte total
    # of one that reproduced its table too.
    extent = (len(original.code) if len(original.data) == len(compiled.data)
              else original.instruction_bytes)
    masked = len(original.mask | compiled.mask)
    return AsmComparison(
        verdict=verdict,
        compared_bytes=max(0, extent - masked),
        masked_bytes=masked,
        data_bytes=len(original.data) if extent == len(original.code) else 0,
        data_divergence=data_divergence,
        original_instructions=n, compiled_instructions=m,
        matching_instructions=matching, mnemonic_similarity=similarity,
        first_divergence=first_divergence, context=context,
        differing_constants=tuple(constants),
        allocation_only=("" if verdict is Tier.BYTE_EXACT
                         else allocation_only(original, compiled)),
        diagnostic=diagnostic,
        flags=compiled.flags)


def compare_regions(pairs: tuple[tuple[Listing, Listing], ...], *,
                    alignments: tuple[int, ...] | None = None) -> AsmComparison:
    """Compare every region; one missed or undecodable region cannot pass."""
    if not pairs:
        raise ValueError("region comparison requires at least one region")
    alignments = (1,) * len(pairs) if alignments is None else alignments
    if (len(alignments) != len(pairs)
            or any(type(value) is not int or value < 1 or value & (value - 1)
                   for value in alignments)):
        raise ValueError("each region requires a positive power-of-two COFF alignment")
    flags = {candidate.flags for _, candidate in pairs}
    if len(flags) != 1:
        raise ValueError("regions must come from the same compiler flag trial")
    previous_end = None
    for original, candidate in pairs:
        if previous_end is not None and original.base < previous_end:
            raise ValueError("original regions must be ordered and non-overlapping")
        previous_end = original.base + len(original.code)
        # The candidate's boundary may differ from the span's: a body of
        # another length split at its object's own table start keeps its
        # tables as data (see `eh_region_listings`), and `compare_asm`
        # reports the differing tail. An image tail is never read without
        # a candidate tail of the same extent to compare it against.
        if (original.code_bytes is not None
                and (candidate.code_bytes is None or len(original.code) != len(candidate.code))):
            raise ValueError("region comparison requires matching code/data boundaries")
        for listing in (original, candidate):
            if not listing.code:
                raise ValueError("region comparison requires nonempty complete code regions")
            if any(offset < 0 or offset >= len(listing.code) for offset in listing.mask):
                raise ValueError("region relocation mask exceeds its bytes")
            if sum(instruction.size for instruction in listing.instructions) != listing.instruction_bytes:
                return AsmComparison(verdict=Tier.REFUSED, flags=candidate.flags,
                                     diagnostic=f"region at 0x{original.base:08X} is not fully decoded")
    measured_pairs, fills = [], []
    for (original, candidate), alignment in zip(pairs, alignments):
        size = len(original.code)
        tail = candidate.code[size:]
        # The mapped image span can itself include padding after the final
        # instruction (for example, one NOP after RET). Preserve and compare
        # every such mapped byte; only the *additional* physical COMDAT
        # envelope may be omitted. Require a terminating instruction before
        # the mapped boundary and padding only from its end through the
        # candidate's aligned end, so a NOP at the mapped boundary is not
        # mistaken for evidence that the function ended there.
        executable_end = None
        for instruction in candidate.instructions:
            end = instruction.address + instruction.size
            if (end <= candidate.base + size
                    and instruction.mnemonic in ("ret", "retf", "jmp")):
                executable_end = end - candidate.base
        suffix_is_fill = (executable_end is not None
                          and all(byte in (0x90, 0xCC)
                                  for byte in candidate.code[executable_end:]))
        omitted_start = candidate.base + size
        omitted_end = candidate.base + len(candidate.code)
        branch_into_fill = any(
            (instruction.group(CS_GRP_CALL) or instruction.group(CS_GRP_JUMP))
            and instruction.operands
            and instruction.operands[0].type == x86.X86_OP_IMM
            and omitted_start <= (instruction.operands[0].imm & 0xFFFFFFFF) < omitted_end
            for instruction in candidate.instructions)
        # Only the exact physical COMDAT alignment envelope can be fill, and
        # only after a terminated body with no relocation field in the tail
        # and no decoded direct control-flow edge into the omitted bytes.
        fill = (len(tail) if tail and len(candidate.code) == size + (-size % alignment)
                and all(byte in (0x90, 0xCC) for byte in tail)
                and not any(offset >= size for offset in candidate.mask)
                and suffix_is_fill and not branch_into_fill else 0)
        if fill:
            candidate = replace(candidate, code=candidate.code[:size])
        measured_pairs.append((original, candidate))
        fills.append(fill)
    results = tuple(compare_asm(original, candidate) for original, candidate in measured_pairs)
    rank = {tier: index for index, tier in enumerate((Tier.BYTE_EXACT, Tier.SHAPE_EXACT,
                                                     Tier.MNEMONIC_ONLY, Tier.MISMATCH))}
    verdict = max((row.verdict for row in results), key=rank.__getitem__)
    failed = next((index for index, row in enumerate(results) if row.verdict != Tier.BYTE_EXACT), None)
    offset = sum(row.original_instructions for row in results[:failed]) if failed is not None else 0
    first = results[failed] if failed is not None else None
    weights = [max(row.original_instructions, row.compiled_instructions) for row in results]
    constants, preceding, preceding_bytes, data_divergence = [], 0, 0, None
    for region_index, row in enumerate(results):
        constants.extend((index + preceding, *values) for index, *values in row.differing_constants)
        preceding += row.original_instructions
        if data_divergence is None and row.data_divergence is not None:
            data_divergence = preceding_bytes + row.data_divergence
        preceding_bytes += len(pairs[region_index][0].code)
    allocation = [f"region 0x{pairs[index][0].base:08X}: {row.allocation_only}"
                  for index, row in enumerate(results)
                  if row.verdict != Tier.BYTE_EXACT and row.allocation_only]
    return AsmComparison(
        verdict=verdict,
        compared_bytes=sum(row.compared_bytes for row in results),
        masked_bytes=sum(row.masked_bytes for row in results),
        original_instructions=sum(row.original_instructions for row in results),
        compiled_instructions=sum(row.compiled_instructions for row in results),
        matching_instructions=sum(row.matching_instructions for row in results),
        mnemonic_similarity=sum(row.mnemonic_similarity * weight for row, weight in zip(results, weights)) / sum(weights),
        first_divergence=(offset + first.first_divergence
                          if first is not None and first.first_divergence is not None else None),
        context=first.context if first is not None else ([], []),
        differing_constants=tuple(constants),
        data_bytes=sum(row.data_bytes for row in results),
        data_divergence=data_divergence,
        allocation_only="; ".join(allocation),
        flags=next(iter(flags)),
        diagnostic="; ".join(f"region 0x{original.base:08X}: {row.verdict}"
                             + (f" ({fill} bytes of bounded COFF alignment fill)" if fill else "")
                             for (original, _), row, fill in zip(pairs, results, fills)),
    )


def _section_table_start(parsed: Coff, section: int, code: bytes,
                         relocations) -> int | None:
    """`_object_table_start` for one whole code section of a multi-span body
    (an EH root or its cold region).

    The code is the section's raw data, so symbol values and relocation
    offsets are already section offsets: the lowest FORWARD DIR32 target in
    the section's own section is its first table byte.
    """
    starts = []
    for item in relocations:
        if item.kind is not RelocationKind.ABSOLUTE32:
            continue
        target = parsed.symbols.get(int(item.target))
        if target is None or target.section != section:
            continue
        addend = int.from_bytes(code[item.offset:item.offset + 4], "little", signed=True)
        at = target.value + addend
        if item.offset < at < len(code):
            starts.append(at)
    return min(starts, default=None)


def eh_region_listings(compiled: RegionCandidate, bound, image: PeImage) -> list[tuple[Listing, Listing]]:
    """(original, compiled) listings of each bound EH region, relocations masked on both sides."""
    pairs = []
    parsed = Coff(compiled.object_bytes)
    symbols = parsed.symbols
    for region in bound:
        low = region.original.start
        original = Listing(image.read(region.original), low,
                           mask=frozenset(offset + byte
                               for offset in image.absolute_sites(region.original)
                               for byte in range(4)))
        references = tuple(
            CodeReference(item.offset, item.kind == RelocationKind.RELATIVE32,
                          symbols[int(item.target)].name, symbols[int(item.target)].storage,
                          addend, string_literal_bytes(parsed, symbols[int(item.target)], addend))
            for item in region.relocations
            for addend in (int.from_bytes(region.code[item.offset:item.offset + 4],
                                          "little", signed=True),))
        candidate = Listing(region.code, low,
                            mask=frozenset(item.offset + byte
                                for item in region.relocations for byte in range(4)),
                            flags=compiled.flags, references=references,
                            table_start=_section_table_start(parsed, region.section,
                                                              region.code, region.relocations))
        # A switch's jump tables ride the COMDAT past the mapped code span,
        # as in the single-span path (`_split_at`): when the candidate's
        # instructions end with a terminator - at the span, or failing that
        # at the object's own table start - and the tail carries
        # relocations, compare that tail as data against the image's own
        # bytes after the span. A body of another length keeps its tables
        # on the data side too, so its instruction count is the body's; it
        # still cannot be BYTE_EXACT. Pure alignment fill keeps the region
        # comparison's own envelope rule.
        span_size = region.original.end - region.original.start
        split = _split_at(candidate, span_size)
        if (split.code_bytes is not None
                and any(offset >= split.code_bytes for offset in candidate.mask)):
            if len(region.code) > span_size:
                extended = Span(low, low + len(region.code))
                original = Listing(image.read(extended), low, code_bytes=span_size,
                                   mask=frozenset(offset + byte
                                       for offset in image.absolute_sites(extended)
                                       for byte in range(4)))
            candidate = split
        pairs.append((original, candidate))
    return pairs


def compare_subject(record: DecompilationState, exe: Path | str,
                    compiled: Listing | RegionCandidate, *,
                    targets: ReferenceTargets | None = None) -> AsmComparison:
    """`compare_asm` with the image side aligned to the object's extent.

    THE OBJECT DECIDES HOW MUCH IMAGE TO READ, because only it knows how
    long the COMDAT is - the catalogue's span is the CODE, and a `switch`'s
    jump table sits past it. Every caller holding a compiled `Listing`
    wants this rather than `compare_asm` directly; `compare_asm` stays the
    pure comparison of two listings, which is what makes it testable
    without an image.
    """
    if isinstance(compiled, RegionCandidate):
        if compiled.mapping_identity != _mapping_identity(record):
            raise SpanComparisonRefused("region candidate belongs to a different original mapping")
        raw_image = _image_bytes(Path(exe))
        # The native caller independently enforces its configured image pin.
        # This parsed view reuses exactly the legacy comparison's image bytes;
        # the digest here validates the parsed view, not an extra source claim.
        image = PeImage(raw_image, hashlib.sha256(raw_image).hexdigest())
        parsed = Coff(compiled.object_bytes)
        if len(parsed.code_sections(parsed.function(compiled.emitted_symbol))) == 1:
            return _missing_eh_region(record, exe, compiled, image)
        try:
            bound = bind_eh_regions(compiled.object_bytes, compiled.emitted_symbol,
                                    image, record.image_spans)
            ownership = bound[0].ownership
            # A valid but different EH topology or action is a MISMATCH. The
            # raw region comparison still runs, so the row keeps its byte
            # similarity; it can never become BYTE_EXACT.
            structural_diagnostic = None
            if ownership.original_predecessors != ownership.compiled_predecessors:
                structural_diagnostic = (
                    "valid EH unwind predecessor topology differs: "
                    f"original={ownership.original_predecessors!r}, "
                    f"compiled={ownership.compiled_predecessors!r}")
            elif ownership.original_null_states != ownership.compiled_null_states:
                structural_diagnostic = (
                    "valid EH null-cleanup states differ: "
                    f"original={ownership.original_null_states!r}, "
                    f"compiled={ownership.compiled_null_states!r}")
            elif len(ownership.original_actions) != len(ownership.compiled_actions):
                structural_diagnostic = "valid EH cleanup action counts differ"
            frame_semantic_diagnostic = None
            if structural_diagnostic is None:
                for index, (original_action, compiled_action) in enumerate(zip(
                        ownership.original_actions, ownership.compiled_actions)):
                    difference = eh_action_semantic_difference(
                        original_action, compiled_action, index)
                    if difference is None:
                        continue
                    category, diagnostic = difference
                    if category == "frame-transfer":
                        frame_semantic_diagnostic = diagnostic
                    else:
                        structural_diagnostic = diagnostic
                        break
            pairs = eh_region_listings(compiled, bound, image)
            patches, patch_applied, patch_notes = record_image_patches(record), [], []
            if patches:
                patched_pairs = []
                for original, candidate in pairs:
                    candidate, applied, notes = apply_image_patches(patches, original, candidate)
                    patched_pairs.append((original, candidate))
                    patch_applied.extend(applied)
                    patch_notes.extend(notes)
                pairs = patched_pairs
                if len(patch_applied) + len(patch_notes) != len(patches):
                    raise SpanComparisonRefused(
                        "declared image patch lies in no compared original region")
            comparison = _with_image_patches(
                compare_regions(tuple(pairs), alignments=tuple(region.alignment for region in bound)),
                tuple(patch_applied), tuple(patch_notes))
            if structural_diagnostic is not None:
                return replace(comparison, verdict=Tier.MISMATCH, flags=compiled.flags,
                               diagnostic=(structural_diagnostic + "; raw region comparison: "
                                           + comparison.diagnostic))
            if comparison.verdict is Tier.BYTE_EXACT:
                wrong = [item for original, candidate in pairs
                         for item in reference_mismatches(original, candidate, targets, record.path,
                                                          image_reader(exe))]
                if wrong:
                    reason = "relocated field differs from its bound target: " + "; ".join(wrong)
                    comparison = replace(comparison, verdict=Tier.SHAPE_EXACT,
                                         allocation_only=reason, diagnostic=reason)
            if frame_semantic_diagnostic is not None:
                return replace(
                    comparison,
                    verdict=Tier.MISMATCH,
                    diagnostic=(frame_semantic_diagnostic + "; raw region comparison: "
                                + comparison.diagnostic),
                )
            return comparison
        except ValueError as error:
            raise SpanComparisonRefused("complete EH-region measurement refused: " + str(error)) from error
    if len(record.image_spans) != 1:
        raise SpanComparisonRefused(
            "complete function comparison requires one original image span")
    original = original_asm(record, exe)
    if len(compiled.code) > len(original.code) and not compiled.bounded:
        try:
            original = original_asm(record, exe, extent=len(compiled.code))
        except ValueError:
            # The tail runs past the section's raw data, so the image has no
            # copy of it to compare against. The comparison stays what it
            # was, and the extra instructions read as the mismatch they are.
            pass
    patches = record_image_patches(record)
    compiled, patch_applied, patch_notes = apply_image_patches(patches, original, compiled)
    if len(patch_applied) + len(patch_notes) != len(patches):
        raise SpanComparisonRefused("declared image patch lies outside the compared original span")
    result = _with_image_patches(compare_asm(original, compiled), patch_applied, patch_notes)
    if result.verdict is Tier.BYTE_EXACT:
        wrong = reference_mismatches(original, compiled, targets, record.path, image_reader(exe))
        if wrong:
            # The instructions and every unrelocated byte agree; a bound
            # target or its offset does not. That is a wrong constant.
            reason = "relocated field differs from its bound target: " + "; ".join(wrong)
            result = replace(result, verdict=Tier.SHAPE_EXACT, allocation_only=reason,
                             diagnostic=reason)
    return result


def _missing_eh_region(record, exe, compiled: RegionCandidate, image) -> AsmComparison:
    """The original has a proved EH cleanup region and the candidate none.

    That is the candidate's difference - a local with a destructor the
    source does not declare, or one it does not keep live - so it is a
    measured MISMATCH, never a match. Only the primary spans are compared,
    for a similarity a reader and the permute scorer can use.
    """
    try:
        actions = original_eh_action_count(image, record.image_spans)
    except ValueError as error:
        raise SpanComparisonRefused("complete EH-region measurement refused: " + str(error)) from error
    code, mask = _coff_function_masked(compiled.object_bytes, record.name, record.symbol)
    candidate = Listing(code=code, base=record.address, mask=mask, flags=compiled.flags)
    primary = compare_asm(original_asm(record, exe), candidate)
    cold_low, cold_high = record.image_spans[1]
    return replace(primary, verdict=Tier.MISMATCH, flags=compiled.flags, diagnostic=(
        f"candidate emits no EH cleanup region; the original's cold region "
        f"{cold_low:#010x}-{cold_high:#010x} has {actions} cleanup action(s); "
        f"primary-span comparison only: {primary.diagnostic}"))


def span_refusal_reason(record: DecompilationState, exe: Path | str,
                        shared: frozenset = frozenset()) -> tuple[Tier, str] | None:
    """The tier for a record no verdict is defined on, and why; or None.

    NOT A FAILURE, A REFUSAL, and the difference is what lets a reader tell
    a wall from a body nobody has written. A self-modifying span cannot be
    reproduced by a compiler at all; a span `/Gy` folded onto another
    function's belongs to no single body, so asking which one it reproduces
    is not a question; and a record with no primary span names nothing to
    compare.

    `shared` comes from `shared_spans` over the whole record set, because
    whether a span has a second claimant is not knowable from one record.
    Everything else is read out of the image - see `rewrites_itself`.
    """
    if not record.image_spans:
        return Tier.REFUSED, "mapping declares no original image span"
    spans = sorted(record.image_spans)
    if (any(high <= low for low, high in spans)
            or any(left[1] > right[0] for left, right in zip(spans, spans[1:]))):
        return Tier.REFUSED, "original image spans are empty, reversed or overlapping"
    # A cold cleanup span is no less part of the function than its entry.
    # Inspect every byte range, including a range that enters a writable
    # executable section after starting in an ordinary section. A refusal
    # takes precedence over a shared-tail diagnostic in another span.
    writable_code = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE
    sections = _sections(Path(exe))
    # AN ASSEMBLER CAN. The refusal is about compilers: no C++ spelling
    # reproduces code that patches its own immediates. A MASM module states
    # those initial bytes and the labels its patchers store through, so a
    # mapping hosted (and assembled) in an assembler source is compared like
    # any other body; the image's file bytes are the pre-patch state.
    assembled = is_assembler_source(record.path) and (
        not record.context or is_assembler_source(record.context))
    selfmod = [(low, high, name) for low, high in spans
               for name, section_low, section_high, characteristics in sections
               if low < section_high and section_low < high
               and characteristics & writable_code == writable_code]
    if not assembled and selfmod:
        low, high, name = selfmod[0]
        return Tier.REFUSED, (f"span {low:#010x}-{high:#010x} lies in writable executable section {name}: "
                              "only an assembler-hosted mapping can reproduce self-modifying code")
    shared_here = [span for span in spans if span in shared]
    if shared_here:
        low, high = shared_here[0]
        return Tier.SHARED_TAIL, f"span {low:#010x}-{high:#010x} is also claimed by another mapping"
    return None


def span_refusal(record: DecompilationState, exe: Path | str,
                 shared: frozenset = frozenset()) -> Tier | None:
    found = span_refusal_reason(record, exe, shared)
    return None if found is None else found[0]


def _span_refusal_detail(record, exe, shared) -> str:
    # Diagnostic text only: span_refusal already decided the tier.
    try:
        found = span_refusal_reason(record, exe, shared)
    except (OSError, ValueError, struct.error):
        return ""
    return "" if found is None else found[1]


def shared_spans(records) -> frozenset:
    """Spans more than one record claims - `/Gy` identical COMDAT folding.

    Independent evidence the shipped build used `/Gy`, and the reason
    SHARED_TAIL exists: a folded tail is one body serving several
    catalogue entries, so a per-function verdict on it is not merely
    unknown, it is undefined.
    """
    seen: dict = {}
    shared = set()
    for record in records:
        for span in record.image_spans:
            if span in seen and seen[span] != record.address:
                shared.add(span)
            seen.setdefault(span, record.address)
    return frozenset(shared)


class _FragmentAdmission(Enum):
    RESOLVED = 1


_FRAGMENT_ADMISSION = _FragmentAdmission.RESOLVED


@dataclass(frozen=True)
class ResolvedFragmentPair:
    """Opaque pair resolved from one captured source inventory."""
    child: object
    parent: object
    _admission: object = field(repr=False, compare=False)

    def __iter__(self):
        yield self.child
        yield self.parent

    @property
    def members(self):
        return (self.child, self.parent)


@dataclass(frozen=True)
class ResolvedEHFragmentGroup:
    """Opaque EH cleanup group resolved from one captured source inventory.

    `children` are the parent's generated cleanup actions in address order;
    they tile the front of its cold region, which ends with the handler stub
    the parent's own cold span declares. Deliberately not iterable as a pair.
    """
    parent: object
    children: tuple
    _admission: object = field(repr=False, compare=False)

    @property
    def members(self):
        return (self.parent, *self.children)

    def measured_parent(self):
        """The parent over (primary, combined cold); measurement only.

        The source span is not rewritten: the combined cold extent starts
        at the first cleanup child and ends with the parent's handler stub.
        """
        primary, cold = self.parent.image_spans
        return replace(self.parent, image_spans=(
            tuple(primary), (self.children[0].image_spans[0][0], cold[1])))


def resolve_fragment_pairs(records, selected, *, component_keys=frozenset(),
                           is_seed=lambda _record: False, bind=lambda row: row):
    """Resolve selected fragment/parent rows against one captured inventory.

    Returns ``(pairs, refused)``. A pair is ``(child, parent)``; the parent
    is support only unless it is itself selected. Ambiguity is explicit and
    fail-closed: callers must report every refused selected row UNKNOWN.
    ``component_keys`` uses the source record identity tuple
    ``(resolved_path, address, line)``.
    """
    records, selected = tuple(records), tuple(selected)

    def key(record):
        return record.path.resolve(), record.address, record.line

    def emitted(record):
        return record.symbol or record.name

    def relation(record, kind=FRAGMENT_TERMINAL):
        value = record.function_fragment
        if fragment_fact_kind(value) != kind:
            return None
        if isinstance(value, dict):
            try:
                return value["parent_address"], value["parent_symbol"]
            except KeyError:
                return None
        return value.parent_address, value.parent_symbol

    selected_keys = {key(row) for row in selected}
    pairs, refused, owners = [], set(), {}
    # An unknown relation kind is never support for anything.
    for row in records:
        if row.function_fragment is not None and fragment_fact_kind(row.function_fragment) is None:
            if key(row) in selected_keys:
                refused.add(key(row))
    children = [row for row in records if relation(row) is not None]
    for child in children:
        parent_address, parent_symbol = relation(child)
        address_candidates = [row for row in records if row.address == parent_address]
        parents = [row for row in address_candidates if emitted(row) == parent_symbol]
        # A seed or another source host is still a competing identity, not a
        # reason to prefer the convenient product-file candidate.
        valid = (len(address_candidates) == 1 and len(parents) == 1
                 and not is_seed(parents[0])
                 and parents[0].path.resolve() == child.path.resolve()
                 # Fragment endpoints must use the same explicit context
                 # metadata. Do not infer equality from suffixes or from an
                 # explicit path that happens to resemble the marker host.
                 and child.context == parents[0].context
                 and parents[0].function_fragment is None
                 and len(parents[0].image_spans) == 1
                 and len(child.image_spans) == 1
                 and child.address == child.image_spans[0][0]
                 and child.image_spans[0][1] > child.image_spans[0][0]
                 and parents[0].image_spans[0][1] == child.image_spans[0][0]
                 and parents[0].image_spans[0][0] == parent_address
                 and key(child) not in component_keys
                 and (len(parents) != 1 or key(parents[0]) not in component_keys))
        if not valid:
            if key(child) in selected_keys:
                refused.add(key(child))
            for parent in address_candidates:
                if key(parent) in selected_keys:
                    refused.add(key(parent))
            continue
        parent = parents[0]
        # This legacy path deliberately supports one adjacent fragment only.
        if key(parent) in owners:
            refused.update((key(child), key(parent), key(owners[key(parent)])))
            continue
        owners[key(parent)] = child
        if key(child) in selected_keys or key(parent) in selected_keys:
            pairs.append(ResolvedFragmentPair(bind(child), bind(parent),
                                              _FRAGMENT_ADMISSION))
    groups = _resolve_eh_fragment_groups(
        records, selected_keys, refused, key=key, emitted=emitted,
        relation=lambda row: relation(row, FRAGMENT_EH_CLEANUP),
        component_keys=component_keys, is_seed=is_seed, bind=bind)
    # One mapping cannot be both a terminal-pair endpoint and an EH group
    # member: refuse both interpretations rather than pick one.
    pair_keys = {key(member) for pair in pairs for member in pair.members}
    for group in groups:
        overlap = {key(member) for member in group.members} & pair_keys
        if overlap:
            refused.update(selected_keys & ({key(member) for member in group.members} | overlap))
    pairs = [pair for pair in pairs
             if key(pair.child) not in refused and key(pair.parent) not in refused]
    groups = [group for group in groups
              if not any(key(member) in refused for member in group.members)]
    return tuple(pairs) + tuple(groups), frozenset(refused)


def _resolve_eh_fragment_groups(records, selected_keys, refused, *, key, emitted, relation,
                                component_keys, is_seed, bind):
    """Resolve v2 EH cleanup children into whole parent groups, fail closed.

    Every eh_cleanup child naming one parent address is one group. The group
    is admitted only when the parent is the sole mapping at that address, is
    the named emitted symbol in the same marker host and context, has a
    primary and a later cold span, is not a seed/component/fragment, and the
    ordered children tile contiguously up to its cold span. Otherwise every
    selected member and address candidate is refused.
    """
    by_parent = {}
    for row in records:
        found = relation(row)
        if found is not None:
            by_parent.setdefault(found[0], []).append(row)
    groups = []
    for parent_address, children in by_parent.items():
        address_candidates = [row for row in records if row.address == parent_address]
        symbols = {relation(child)[1] for child in children}
        parents = [row for row in address_candidates if emitted(row) in symbols]
        valid = len(symbols) == 1 and len(address_candidates) == 1 and len(parents) == 1
        if valid:
            parent = parents[0]
            spans = tuple(tuple(span) for span in parent.image_spans)
            valid = (not is_seed(parent)
                     and parent.function_fragment is None
                     and str(parent.mode) != Mode.UNBOUND.value
                     and str(getattr(parent, "state", "")) != "excluded"
                     and key(parent) not in component_keys
                     and len(spans) == 2 and spans[0][0] == parent_address
                     and spans[0][0] < spans[0][1] <= spans[1][0] < spans[1][1]
                     and all(child.path.resolve() == parent.path.resolve()
                             and child.context == parent.context
                             and key(child) not in component_keys
                             and len(child.image_spans) == 1
                             and child.image_spans[0][0] == child.address
                             and child.image_spans[0][1] > child.image_spans[0][0]
                             for child in children))
        if valid:
            ordered = sorted(children, key=lambda child: child.image_spans[0][0])
            cursor = ordered[0].image_spans[0][0]
            valid = cursor >= spans[0][1]
            for child in ordered:
                valid = valid and child.image_spans[0][0] == cursor
                cursor = child.image_spans[0][1]
            valid = valid and cursor == spans[1][0]
        if not valid:
            refused.update(selected_keys & {key(row) for row in (*children, *address_candidates)})
            continue
        if any(key(row) in selected_keys for row in (parent, *ordered)):
            groups.append(ResolvedEHFragmentGroup(
                bind(parent), tuple(bind(child) for child in ordered), _FRAGMENT_ADMISSION))
    return groups


def _better(candidate: AsmComparison, incumbent: AsmComparison) -> bool:
    """Is `candidate` the more useful of two measurements of one record?

    SIMILARITY BEFORE POSITIONAL AGREEMENT. `matching_instructions` counts
    instruction i against instruction i, so a candidate one instruction
    short of the image scores near zero however right the rest of it is -
    every later instruction is compared with the wrong one. Ranking on it
    picked the answer that happened to keep its length over the answer that
    was closer. `mnemonic_similarity` is an alignment, so it survives a
    length difference; agreement still breaks ties, where it is the sharper
    of the two.
    """
    return ((candidate.verdict.rank, -candidate.mnemonic_similarity,
             -candidate.matching_instructions)
            < (incumbent.verdict.rank, -incumbent.mnemonic_similarity,
               -incumbent.matching_instructions))


def compare_record(record: DecompilationState, exe: Path | str,
                   command: list[str], flags: tuple | str,
                   shared: frozenset = frozenset(), *, compiler,
                   retry_compile=None, targets: ReferenceTargets | None = None) -> AsmComparison:
    """How far VC6 reproduces the shipped bytes for the body IN THE TREE.

    `compare_source` with the record's own file - which is the question
    `check` and `record` ask, and the only one that can be written down.
    """
    return compare_source(record, exe, record.path, command, flags, shared,
                          compiler=compiler, retry_compile=retry_compile, targets=targets)


DIAGNOSTIC_BASIS = (
    "non-authoritative relocation-masked comparison of the compiled body against "
    "the original span(s); the strict verdict and its refusal stand, nothing here "
    "is claimable, and references, associated data, EH metadata and extents are "
    "not verified")


def diagnostic_payload(comparison: AsmComparison, *, basis: str = DIAGNOSTIC_BASIS,
                       regions: int = 1, notes: tuple[str, ...] = ()) -> dict:
    """The JSON shape of a diagnostic-only comparison; never a verdict."""
    return {
        "authoritative": False, "claimable": False, "basis": basis,
        "masked_verdict": str(comparison.verdict),
        "similarity": comparison.mnemonic_similarity,
        "original_instructions": comparison.original_instructions,
        "compiled_instructions": comparison.compiled_instructions,
        "matching_instructions": comparison.matching_instructions,
        "first_divergence": comparison.first_divergence,
        "context": comparison.context,
        "compared_bytes": comparison.compared_bytes,
        "masked_bytes": comparison.masked_bytes,
        "data_bytes": comparison.data_bytes,
        "data_divergence": comparison.data_divergence,
        "differing_constants": comparison.differing_constants,
        "regions": regions, "flags": comparison.flags,
        "notes": [*notes, *([comparison.diagnostic] if comparison.diagnostic else [])],
    }


def _combine_diagnostic_regions(results: list[AsmComparison]) -> AsmComparison:
    weights = [max(row.original_instructions, row.compiled_instructions, 1) for row in results]
    rank = {tier: index for index, tier in enumerate((Tier.BYTE_EXACT, Tier.SHAPE_EXACT,
                                                     Tier.MNEMONIC_ONLY, Tier.MISMATCH))}
    failed = next((index for index, row in enumerate(results) if row.verdict != Tier.BYTE_EXACT), None)
    offset = sum(row.original_instructions for row in results[:failed]) if failed is not None else 0
    first = results[failed] if failed is not None else None
    return AsmComparison(
        verdict=max((row.verdict for row in results), key=lambda tier: rank.get(tier, len(rank))),
        compared_bytes=sum(row.compared_bytes for row in results),
        masked_bytes=sum(row.masked_bytes for row in results),
        original_instructions=sum(row.original_instructions for row in results),
        compiled_instructions=sum(row.compiled_instructions for row in results),
        matching_instructions=sum(row.matching_instructions for row in results),
        mnemonic_similarity=sum(row.mnemonic_similarity * weight
                                for row, weight in zip(results, weights)) / sum(weights),
        first_divergence=(offset + first.first_divergence
                          if first is not None and first.first_divergence is not None else None),
        context=first.context if first is not None else ([], []),
        data_bytes=sum(row.data_bytes for row in results),
        allocation_only="", flags=results[0].flags)


def _diagnostic_listings(obj: bytes, record: DecompilationState, exe: Path,
                         label: str) -> tuple[list[tuple[Listing, Listing]], tuple[str, ...]]:
    """Best-effort (original, compiled) region pairs; no EH/extent proof."""
    spans = sorted(record.image_spans)
    if not spans:
        raise ValueError("the record carries no original spans")
    if len(spans) == 1:
        compiled = subject_asm(obj, record, label)
        original = original_asm(record, exe)
        if len(compiled.code) > len(original.code) and not compiled.bounded:
            try:
                original = original_asm(record, exe, extent=len(compiled.code))
            except ValueError:
                pass
        return [(original, compiled)], ()
    parsed = Coff(obj)
    wanted = record.symbol or record.name
    try:
        root = parsed.function(wanted)
    except ValueError:
        if record.symbol:
            raise
        root = parsed.function("_" + record.name)
    sections = parsed.code_sections(root)
    notes = []
    if len(sections) != len(spans):
        notes.append(f"compiled code sections ({len(sections)}) and original spans ({len(spans)}) "
                     "differ in number; only paired regions in order are compared")
    pairs = []
    for (low, high), number in zip(spans, sections):
        code, relocations, _alignment = parsed.section_subject(number)
        mask = frozenset(item.offset + byte for item in relocations for byte in range(4)
                         if item.offset + byte < len(code))
        compiled = Listing(code=code, base=low, mask=mask, flags=label,
                           table_start=_section_table_start(parsed, number, code, relocations))
        end = len(compiled.code)
        while end > 0 and compiled.code[end - 1] in (0x90, 0xCC):
            end -= 1
        compiled = replace(compiled, code=compiled.code[:end],
                           mask=frozenset(item for item in mask if item < end))
        image_code = _pe_bytes(exe, low, high - low)
        cut = len(image_code)
        while cut > 0 and image_code[cut - 1] in (0x90, 0xCC):
            cut -= 1
        original = Listing(code=image_code[:cut], base=low, mask=_image_mask(exe, low, low + cut))
        # As in the single-span branch: a region whose COMDAT carries its
        # switch tables past the span reads the image's copy of them, and
        # `compare_asm` splits the candidate at the span or, for a body of
        # another length, at its own table start - so the tables are
        # compared as data, never counted as instructions.
        if len(compiled.code) > cut:
            try:
                original = Listing(code=_pe_bytes(exe, low, len(compiled.code)), base=low,
                                   code_bytes=cut,
                                   mask=_image_mask(exe, low, low + len(compiled.code)))
            except ValueError:
                pass
        pairs.append((original, compiled))
    return pairs, tuple(notes)


def diagnostic_comparison(record: DecompilationState, exe: Path | str, source: Path | str,
                          command, attempts, *, compiler) -> dict:
    """Best-of-trials masked comparison for a record the strict path refused.

    DIAGNOSTIC ONLY. A refusal says the claim cannot be decided; it does not
    say how close the body is, and a recovery loop that sees only REFUSED has
    nothing to work toward. This reuses the legacy masked comparer on the
    compiled COMDAT(s) and the original span(s) without any of the strict
    preconditions (association cells, EH action graphs, FPO extents,
    CodeView labels). The result is reported beside, never instead of, the
    strict verdict.
    """
    best, best_notes, best_regions, errors = _masked_region_comparison(
        record, exe, source, command, attempts, compiler=compiler)
    if best is None:
        return {"authoritative": False, "claimable": False, "available": False,
                "reason": errors[0] if errors else "no compiler trial produced a comparable body"}
    return {"available": True, **diagnostic_payload(best, regions=best_regions, notes=best_notes)}


def _masked_region_comparison(record: DecompilationState, exe: Path | str, source: Path | str,
                              command, attempts, *, compiler):
    """Best-of-trials masked comparison over the record's region pairs.

    Returns (best AsmComparison or None, notes, region count, errors). This
    proves nothing: callers either label it diagnostic, or use it only to
    rank a body whose strict verdict is already a conclusive FAIL.
    """
    exe = Path(exe)
    best, best_notes, best_regions, errors = None, (), 1, []
    for attempt in attempts:
        if isinstance(attempt, str):
            arguments, label = tuple(shlex.split(attempt)), attempt
        else:
            arguments = tuple(attempt)
            label = shlex.join(arguments)
        try:
            obj = compiler(Path(source), tuple(command), arguments)
            pairs, notes = _diagnostic_listings(obj, record, exe, label)
            if not pairs:
                raise ValueError("no compiled region pairs with an original span")
            results = [compare_asm(original, compiled) for original, compiled in pairs]
            result = results[0] if len(results) == 1 else _combine_diagnostic_regions(results)
        except (OSError, ValueError, CompileFailed, struct.error) as error:
            errors.append(str(error))
            continue
        if best is None or _better(result, best):
            best, best_notes, best_regions = result, notes, len(pairs)
        if best.verdict is Tier.BYTE_EXACT:
            break
    return best, best_notes, best_regions, errors


def _ranked_strict_failure(record: DecompilationState, diagnostic: str, exe, source, command,
                           attempts, *, compiler, diagnostics: bool) -> AsmComparison:
    """An ordinary MISMATCH for a strict FAIL, ranked like any other miss.

    The strict proof already decided the verdict: a conclusive FAIL (for
    example a compiled extent that differs from the recorded original
    envelope) can never become a claim, so the masked comparison over the
    same region pairs is used here only for the distance fields every other
    MISMATCH carries. The verdict stays MISMATCH whatever the masked bytes
    say, `allocation_only` stays undecided, and an unavailable comparison
    leaves the zero distance. Strict UNKNOWN outcomes never come here; they
    remain REFUSED.
    """
    best, notes, regions, errors = _masked_region_comparison(
        record, exe, source, command, attempts, compiler=compiler)
    if best is None:
        return AsmComparison(
            verdict=Tier.MISMATCH, diagnostic=diagnostic,
            diagnostic_comparison=({"authoritative": False, "claimable": False, "available": False,
                                    "reason": errors[0] if errors else
                                    "no compiler trial produced a comparable body"}
                                   if diagnostics else None))
    return AsmComparison(
        verdict=Tier.MISMATCH, diagnostic=diagnostic, flags=best.flags,
        compared_bytes=best.compared_bytes, masked_bytes=best.masked_bytes,
        original_instructions=best.original_instructions,
        compiled_instructions=best.compiled_instructions,
        matching_instructions=best.matching_instructions,
        mnemonic_similarity=best.mnemonic_similarity,
        first_divergence=best.first_divergence, context=best.context,
        data_bytes=best.data_bytes, data_divergence=best.data_divergence,
        diagnostic_comparison=({"available": True,
                                **diagnostic_payload(best, regions=regions, notes=notes)}
                               if diagnostics else None))


@dataclass(frozen=True)
class UnitComparison:
    """Disposable measurements in selection order; never source claims.

    A string result is unresolved original/symbol evidence, not a compilation
    verdict. Objects and failed attempts are reused only inside this call.
    """
    results: tuple[tuple[DecompilationState, AsmComparison | str], ...]
    compile_attempts: int
    reused_attempts: int


def strict_row_reason(row: dict, *, limit: int = 4) -> str | None:
    """Describe a non-passing strict function row from its own proof fields.

    Only the row's (or its non-passing flag variants') boundary, byte and
    reference components are consulted. Returns None when they say nothing,
    so a caller can fall back to explicitly labelled evidence-check reasons.
    """
    def addresses(variant, offsets):
        start = (variant.get("comparison_span") or {}).get("start")
        shown = ", ".join(f"+{offset:#x}" + (f" ({start + offset:#010x})" if isinstance(start, int) else "")
                          for offset in offsets[:limit])
        return shown + (f" and {len(offsets) - limit} more" if len(offsets) > limit else "")

    variants = [variant for variant in row.get("variants", ())
                if isinstance(variant, dict) and variant.get("result") != Result.PASS]
    unknown = None
    for outer in (*variants, row):
        # A multi-span body (for example code plus its `.text$x` unwind
        # funclets) states its component results per span.
        for variant in (outer, *(span for span in outer.get("spans", ()) or ()
                                 if isinstance(span, dict) and span.get("result") != Result.PASS)):
            parts = []
            if variant.get("boundary") == Result.FAIL:
                parts.append("compiled boundary differs")
            offsets = variant.get("differing_offsets") or []
            if offsets:
                parts.append(f"{len(offsets)} non-relocation byte(s) differ at " + addresses(variant, offsets))
            elif variant.get("bytes") == Result.FAIL:
                parts.append("body bytes differ")
            for field, label in (("missing_absolute_relocations", "original absolute relocation(s) missing at"),
                                 ("extra_absolute_relocations", "extra compiled absolute relocation(s) at"),
                                 ("unexpected_constant_base_relocations", "rebased constant(s) at")):
                sites = variant.get(field) or []
                if sites:
                    parts.append(f"{label} " + addresses(variant, sites))
            wrong = [item for item in variant.get("relocations", ()) or ()
                     if isinstance(item, dict) and item.get("result") == Result.FAIL]
            if wrong:
                parts.append("relocated target(s) differ: " + ", ".join(
                    f"+{item.get('offset', 0):#x} {item.get('symbol', item.get('target'))}"
                    + (f" ({item['reason']})" if item.get("reason") else "")
                    for item in wrong[:limit]) + (f" and {len(wrong) - limit} more" if len(wrong) > limit else ""))
                if any(re.fullmatch(r"\$SG[0-9]+", str(item.get("symbol", ""))) for item in wrong):
                    # Positional VC6 string labels renumber when a header changes.
                    parts.append("if a $SG literal binding went stale, run `recover rebind-literals --unit <this TU>`")
            if not parts and variant.get("references") == Result.FAIL:
                parts.append("references differ")
            if parts:
                return "strict body proof failed: " + "; ".join(parts)
            artifacts = [item for item in variant.get("relocations", ()) or ()
                         if isinstance(item, dict) and item.get("result") == Result.UNKNOWN
                         and (item.get("binding") or {}).get("kind") == "compiler_artifact"]
            if artifacts and unknown is None:
                # An unplaced compiler-emitted target (typically an unbound
                # `$SG` string label) leaves the references unknown, never
                # failed; say which instead of a bare "unknown".
                unknown = ("strict body proof unknown: unbound compiler artifact relocation(s): " + ", ".join(
                    f"+{item.get('offset', 0):#x} {item.get('symbol', item.get('target'))}"
                    for item in artifacts[:limit]) + (f" and {len(artifacts) - limit} more"
                                                      if len(artifacts) > limit else ""))
                if any(re.fullmatch(r"\$SG[0-9]+", str(item.get("symbol", ""))) for item in artifacts):
                    unknown += ("; bind each literal with `RECOVERY_LITERAL` from the image's operand, or run "
                                "`recover rebind-literals --unit <this TU>` if an existing binding went stale")
    return unknown


def strict_evidence_reason(strict: dict, *, limit: int = 4) -> str | None:
    """Name the first evidence-table row that keeps a strict selection from passing.

    The selection's object, literal, generated-table and shared-function
    proofs decide its result alongside the function rows, but only the
    explicit evidence checks carry a `reason`. A table row whose own content
    is unknown or failed (typically an object whose initialiser points at a
    stale `$SG` literal binding in another unit) is described from its proof
    fields here, so the refusal says which binding to repair. Failed rows are
    reported before unknown ones. An unusable link observation is appended
    (or reported alone): it only matters for weak `??_E` selections.
    """
    found = {}
    for table, field in (("object_definitions", "definition"), ("object_definitions", "content"),
                         ("literal_definitions", "content"), ("generated_tables", "content"),
                         ("shared_functions", "result")):
        for row in strict.get(table, ()) or ():
            if not isinstance(row, dict) or field not in row:
                continue
            state = row[field]
            if state == Result.PASS or state in found:
                continue
            proof = row.get("proof") if isinstance(row.get("proof"), dict) else {}
            unresolved = [str(item.get("symbol", item.get("target")))
                          for item in proof.get("relocations", ()) or ()
                          if isinstance(item, dict) and item.get("result") != Result.PASS]
            text = (f"selected evidence {row.get('symbol')} in {row.get('unit')} "
                    f"({table[:-1].replace('_', ' ')} {field}) is {state}")
            if row.get("reason"):
                text += f": {row['reason']}"
            elif unresolved:
                text += ": unproved relocation target(s) " + ", ".join(unresolved[:limit]) + (
                    f" and {len(unresolved) - limit} more" if len(unresolved) > limit else "")
                if any(re.fullmatch(r"\$SG[0-9]+", name) for name in unresolved):
                    text += f"; run `recover rebind-literals --unit {row.get('unit')}` if a literal binding went stale"
            found[state] = text
    observation = strict.get("weak_link_observation")
    link = (f"observed link is {observation.get('result')}: {observation['reason']}"
            if isinstance(observation, dict) and observation.get("result") != Result.PASS
            and observation.get("reason") else None)
    text = found.get(Result.FAIL) or next(iter(found.values()), None)
    if text and link:
        # A weak `??_E` slot is only selectable through a usable observation.
        return f"{text}; {link}"
    return text or link


def compare_unit(records, exe: Path | str, source: Path | str,
                 command: list[str], flags: tuple | str,
                 shared: frozenset = frozenset(), *, compiler,
                 retry_compile=None, fragment_pairs=(), root: Path | None = None,
                 strict_command_for=None, associated_bindings=None,
                 targets: ReferenceTargets | None = None,
                 diagnostics: bool = False) -> UnitComparison:
    """Measure one TU's selected mappings with one compile per flag trial.

    Use the same per-function selection, span refusals, missing-symbol retries
    and similarity ranking as compare_source, rather than another scoring loop.
    Different subjects may need different trials. One unresolved subject never
    discards measured siblings. The caller owns whole-project input freshness
    and publication; this function also refuses changed candidate/marker files.
    No objects, scores or failures are cached across calls.

    `diagnostics=True` (measure/diff only, never recording) attaches a
    non-authoritative `diagnostic_comparison` to refused, unscored or
    strictly failed associated results; verdicts are unchanged.
    """
    records = tuple(records)
    if not records:
        raise ValueError("no source mappings to measure")
    keys = {(item.path.resolve(), item.address, item.line) for item in records}
    if len(keys) != len(records):
        raise ValueError("the same source mapping was selected more than once")
    if len({item.path.resolve() for item in records}) != 1:
        raise ValueError("one measurement unit requires one source mapping host")
    attempts = (flags,) if isinstance(flags, str) else tuple(flags)
    if not attempts:
        raise ValueError("no flag sets to try")
    source = Path(source)
    fragment_pairs = tuple(fragment_pairs)
    eh_groups = tuple(item for item in fragment_pairs if isinstance(item, ResolvedEHFragmentGroup))
    fragment_pairs = tuple(item for item in fragment_pairs
                           if not isinstance(item, ResolvedEHFragmentGroup))
    inputs = {path: path.read_bytes() for path in {
        source, *(item.path for item in records),
        *(item.path for pair in fragment_pairs for item in pair),
        *(item.path for group in eh_groups for item in group.members)}}
    objects, failures = {}, {}
    count, reused = 0, 0

    def current():
        if any(path.read_bytes() != data for path, data in inputs.items()):
            raise ValueError("source changed during unit measurement")

    def shared_compile(path, arguments, trial):
        nonlocal count, reused
        current()
        key = (path.resolve(), arguments, trial)
        if key in objects:
            reused += 1
            return objects[key]
        if key in failures:
            reused += 1
            raise CompileFailed(failures[key])
        count += 1
        try:
            data = compiler(path, arguments, trial)
            if not isinstance(data, bytes) or not data:
                raise CompileFailed("compiler returned no object bytes")
        except (OSError, ValueError, CompileFailed) as error:
            failures[key] = str(error)
            raise CompileFailed(str(error)) from error
        finally:
            current()
        objects[key] = data
        return data

    def key(record):
        return record.path.resolve(), record.address, record.line

    # Associated code/data tails cannot use the legacy section-tail matcher:
    # it may borrow unrelated bytes after the ordinary source function. Route
    # only explicitly associated owners through the shared strict verifier.
    associated_results = {}
    marker = b"RECOVERY_ASSOCIATED:"
    selected_source_has_associations = marker in inputs[next(iter(records)).path]
    if root is None:
        if selected_source_has_associations:
            raise ValueError("root context is required to verify selected associated owners")
        if associated_bindings is not None:
            if any(binding.owner.address == record.address
                   for binding in associated_bindings for record in records):
                raise ValueError("root context is required to verify selected associated owners")
    if root is not None and selected_source_has_associations:
        root = Path(root).resolve()
        pin = image_pin(root)
        settings = Measurement.read(root)
        owner_path = next(iter(records)).path.resolve()
        relative = owner_path.relative_to(root).as_posix()
        current_associated = parse_associated(
            inputs[next(iter(records)).path].decode("utf-8"), relative, pin,
            address_space=settings.source.address_space)
        if (associated_bindings is not None
                and tuple(binding for binding in associated_bindings if binding.unit == relative)
                != current_associated):
            raise ValueError("associated bindings differ from current configured source")
        associated_bindings = current_associated
        relative_by_record = {
            key(record): record.path.resolve().relative_to(root).as_posix()
            for record in records
        }
        associated_keys = {
            key(record) for record in records
            if any(binding.owner.address == record.address
                   and binding.owner.image_sha256 == pin
                   and binding.unit == relative_by_record[key(record)]
                   for binding in associated_bindings)
        }
        if associated_keys:
            selected_associated = [record for record in records if key(record) in associated_keys]
            if strict_command_for is None:
                strict_command_for = lambda path: tuple(command) if Path(path).resolve() == source.resolve() else ()
            strict_attempts = []
            for attempt in attempts:
                if isinstance(attempt, str):
                    strict_attempts.append(tuple(shlex.split(attempt)))
                elif (isinstance(attempt, (tuple, list))
                      and all(isinstance(word, str) and "\0" not in word for word in attempt)):
                    strict_attempts.append(tuple(attempt))
                else:
                    raise ValueError("compiler flag trial must be a string or an array of argument tokens")

            def strict_for(selection):
                return verify_records(
                    root, selection, Path(exe), compiler=shared_compile,
                    command_for=strict_command_for,
                    flag_sets=tuple(strict_attempts),
                )

            # A selection's evidence closure is the union of its roots'
            # closures and its checks are not attributed to roots. A passing
            # union therefore covers every member, but a non-passing one says
            # nothing about a member whose own closure is complete. Re-verify
            # each owner alone then, so its verdict never depends on which
            # file-mates the caller happened to select.
            grouped = strict_for(selected_associated)
            strict_by_record = {}
            for record in selected_associated:
                strict_by_record[key(record)] = (
                    grouped if grouped.get("result") == Result.PASS or len(selected_associated) == 1
                    else strict_for([record]))
            for record in selected_associated:
                strict = strict_by_record[key(record)]
                row = next((candidate for candidate in strict.get("functions", [])
                            if int(candidate.get("address", "0"), 16) == record.address), None)
                result = row.get("result") if row else None
                diagnostic = row.get("reason") if row else None
                if not diagnostic and row:
                    # A per-flag variant carries the strict refusal reason
                    # when the aggregated function row does not.
                    diagnostic = next((variant.get("reason") for variant in row.get("variants", ())
                                       if isinstance(variant, dict)
                                       and variant.get("result") != Result.PASS
                                       and variant.get("reason")), None)
                if not diagnostic and row:
                    # A failed body proof states its own component results;
                    # never let an unrelated evidence check stand in for it.
                    diagnostic = strict_row_reason(row)
                body_unknown = None
                if diagnostic and diagnostic.startswith("strict body proof unknown"):
                    # An unknown body reference yields to a refused evidence
                    # check, which is reported first when both are present.
                    body_unknown, diagnostic = diagnostic, None
                if not diagnostic:
                    checks = strict.get("selected_evidence", {}).get("checks", [])
                    diagnostic = next((f"selected evidence {item.get('symbol')} in {item.get('unit')} "
                                       f"is {item.get('result')}: {item['reason']}"
                                       for item in checks
                                       if item.get("reason") and item.get("result") != Result.PASS), None)
                diagnostic = diagnostic or body_unknown
                if not diagnostic and row:
                    reference_report = row.get("diagnostic")
                    if isinstance(reference_report, dict):
                        unresolved = [item.get("symbol") for item in reference_report.get("emitted_references", [])
                                      if item.get("binding_address") is None]
                        if unresolved:
                            diagnostic = "selected code has unresolved references: " + ", ".join(unresolved)
                    elif isinstance(reference_report, str):
                        diagnostic = reference_report
                if not diagnostic and strict.get("result") != Result.PASS:
                    diagnostic = strict_evidence_reason(strict)
                if not diagnostic and strict.get("result") != Result.PASS:
                    diagnostic = f"strict selected evidence result: {strict.get('result', Result.UNKNOWN)}"
                if result == Result.PASS and strict.get("result") == Result.PASS:
                    observed = AsmComparison(verdict=Tier.BYTE_EXACT)
                elif result == Result.FAIL or strict.get("result") == Result.FAIL:
                    observed = _ranked_strict_failure(
                        record, diagnostic or "strict associated owner or selected dependency check failed",
                        exe, source, command, attempts, compiler=shared_compile, diagnostics=diagnostics)
                else:
                    observed = AsmComparison(
                        verdict=Tier.REFUSED,
                        diagnostic=diagnostic or "strict associated owner or selected dependency is unknown or refused",
                    )
                associated_results[key(record)] = observed

    selected_by_key = {key(record): record for record in records}
    selected = set(selected_by_key)
    parent_to_pair, child_to_parent, pair_errors = {}, {}, {}
    parent_counts, child_counts = {}, {}
    for candidate_pair in fragment_pairs:
        if not isinstance(candidate_pair, ResolvedFragmentPair):
            continue
        try:
            candidate_child, candidate_parent = candidate_pair
            candidate_ck, candidate_pk = key(candidate_child), key(candidate_parent)
        except (AttributeError, TypeError):
            continue
        parent_counts[candidate_pk] = parent_counts.get(candidate_pk, 0) + 1
        child_counts[candidate_ck] = child_counts.get(candidate_ck, 0) + 1
    for candidate_pair in fragment_pairs:
        if (not isinstance(candidate_pair, ResolvedFragmentPair)
                or candidate_pair._admission is not _FRAGMENT_ADMISSION):
            try:
                endpoints = tuple(candidate_pair)
            except TypeError:
                endpoints = ()
            for endpoint in endpoints:
                try:
                    endpoint_key = key(endpoint)
                except AttributeError:
                    continue
                if endpoint_key in selected_by_key:
                    pair_errors[endpoint_key] = (
                        "fragment support was not resolved by the package source-view binder")
            for record in records:
                if record.function_fragment is not None:
                    pair_errors[key(record)] = (
                        "fragment support was not resolved by the package source-view binder")
            continue
        child, parent = candidate_pair
        ck = pk = None
        try:
            ck, pk = key(child), key(parent)
            if parent_counts.get(pk, 0) > 1 or child_counts.get(ck, 0) > 1:
                raise ValueError("duplicate fragment relation is ambiguous")
            if child.function_fragment is None:
                raise ValueError("fragment support row has no fragment relation")
            relation = child.function_fragment
            if (parent.function_fragment is not None
                    or parent.path.resolve() != child.path.resolve()
                    or child.path.resolve() != source.resolve()
                    or parent.address != relation.parent_address
                    or (parent.symbol or parent.name) != relation.parent_symbol
                    or len(parent.image_spans) != 1 or len(child.image_spans) != 1
                    or parent.image_spans[0][0] != parent.address
                    or child.image_spans[0][0] != child.address
                    or parent.image_spans[0][1] != child.image_spans[0][0]
                    or child.image_spans[0][1] <= child.image_spans[0][0]):
                raise ValueError("fragment parent identity or contiguous extent is unsupported")
            if ((ck in selected and child != selected_by_key[ck])
                    or (pk in selected and parent != selected_by_key[pk])):
                raise ValueError("fragment support differs from the selected source mapping")
            parent_to_pair[pk] = (child, parent)
            child_to_parent[ck] = pk
        except (AttributeError, IndexError, TypeError, ValueError) as error:
            if ck is not None and ck in selected:
                pair_errors[ck] = str(error)
            if pk is not None and pk in selected:
                pair_errors[pk] = str(error)

    pair_results = {}
    for child, parent in parent_to_pair.values():
        ck, pk = key(child), key(parent)
        refused = parent if span_refusal(parent, exe, shared) is not None else child
        refusal = span_refusal(refused, exe, shared)
        if refusal is not None:
            pair_results[pk] = pair_results[ck] = AsmComparison(
                verdict=refusal, diagnostic=_span_refusal_detail(refused, exe, shared))
            continue
        best_parent, best_child, diagnostic, missing = None, None, "", None
        for attempt in attempts:
            try:
                label = attempt if isinstance(attempt, str) else shlex.join(tuple(attempt))
                arguments = tuple(shlex.split(attempt)) if isinstance(attempt, str) else tuple(attempt)
                raw = shared_compile(source, tuple(command), arguments)
                obj = Coff(raw)
                symbol = obj.function(parent.symbol or parent.name)
                if (symbol.section in obj.associations
                        or obj.code_sections(symbol) != (symbol.section,)):
                    raise ValueError("fragment parent is not one exclusive code COMDAT")
                section = obj.sections[symbol.section - 1]
                if any(kind not in (0x0006, 0x0014)
                       for _offset, _symbol, kind in section.relocations):
                    raise ValueError("unsupported COFF relocation kind in fragment parent")
                code, relocations, alignment = obj.subject(symbol)
                child_offset = child.image_spans[0][0] - parent.image_spans[0][0]
                child_size = child.image_spans[0][1] - child.image_spans[0][0]
                child_end = child_offset + child_size
                parent_size = child_end
                if len(code) < parent_size:
                    raise ValueError("fragment parent COMDAT extent differs from the captured contiguous span")
                if alignment <= 0:
                    raise ValueError("fragment parent COMDAT has invalid section alignment")
                aligned_extent = ((parent_size + alignment - 1) // alignment) * alignment
                if len(code) > aligned_extent:
                    raise ValueError("fragment parent COMDAT exceeds its bounded section-alignment padding")
                if any(byte not in (0x90, 0xCC) for byte in code[parent_size:]):
                    raise ValueError("fragment parent COMDAT has non-padding bytes beyond the captured span")
                if any(relocation.offset >= parent_size
                       or relocation.offset + 4 > parent_size
                       for relocation in relocations):
                    raise ValueError("COFF relocation lies outside the captured fragment parent span")
                if any(relocation.offset < boundary < relocation.offset + 4
                       for relocation in relocations
                       for boundary in (child_offset, child_end)):
                    raise ValueError("COFF relocation crosses a fragment slice boundary")
                compiled_mask = frozenset(
                    relocation.offset + byte
                    for relocation in relocations for byte in range(4)
                    if relocation.offset + byte < parent_size)
                references = tuple(
                    CodeReference(relocation.offset, relocation.kind == RelocationKind.RELATIVE32,
                                  obj.symbols[int(relocation.target)].name,
                                  obj.symbols[int(relocation.target)].storage,
                                  addend, string_literal_bytes(obj, obj.symbols[int(relocation.target)], addend))
                    for relocation in relocations
                    for addend in (int.from_bytes(code[relocation.offset:relocation.offset + 4],
                                                  "little", signed=True),))
                compiled = Listing(code=code[:parent_size], base=parent.address,
                                   mask=compiled_mask, flags=label, references=references)
                image_code = _pe_bytes(Path(exe), parent.address, parent_size)
                original_parent = Listing(
                    code=image_code, base=parent.address,
                    mask=_image_mask(Path(exe), parent.address,
                                     parent.address + parent_size))
                parent_result = compare_asm(original_parent, compiled)
                wrong = (reference_mismatches(original_parent, compiled, targets, parent.path,
                                              image_reader(exe))
                         if parent_result.verdict is Tier.BYTE_EXACT else [])
                if wrong:
                    reason = "relocated field differs from its bound target: " + "; ".join(wrong)
                    parent_result = replace(parent_result, verdict=Tier.SHAPE_EXACT,
                                            allocation_only=reason, diagnostic=reason)
                child_listing = Listing(
                    code=compiled.code[child_offset:child_end], base=child.address,
                    mask=frozenset(offset - child_offset for offset in compiled.mask
                                   if child_offset <= offset < child_end),
                    flags=compiled.flags)
                child_original = Listing(
                    code=image_code[child_offset:child_end], base=child.address,
                    mask=frozenset(offset - child_offset
                                   for offset in original_parent.mask
                                   if child_offset <= offset < child_end))
                child_result = compare_asm(child_original, child_listing)
                if parent_result.verdict is not Tier.BYTE_EXACT:
                    child_result = AsmComparison(
                        verdict=Tier.REFUSED,
                        diagnostic="fragment slice is not claimable until the complete parent extent matches",
                        flags=label,
                        diagnostic_comparison=(diagnostic_payload(
                            child_result, regions=1,
                            notes=("fragment slice compared within the compiled parent COMDAT; "
                                   f"parent masked verdict {parent_result.verdict}, "
                                   f"similarity {parent_result.mnemonic_similarity:.3f}",))
                            if diagnostics else None))
                if best_parent is None or _better(parent_result, best_parent):
                    best_parent, best_child = parent_result, child_result
                if parent_result.verdict is Tier.BYTE_EXACT:
                    best_parent, best_child = parent_result, child_result
                    break
            except CompileFailed as error:
                diagnostic = diagnostic or str(error)
                if retry_compile is None or retry_compile(str(error)):
                    continue
                break
            except ValueError as error:
                missing = missing or error
                continue
        if best_parent is None:
            outcome = (str(missing) if missing is not None and not diagnostic else
                       AsmComparison(verdict=Tier.NO_COMPILE, diagnostic=diagnostic))
            pair_results[pk] = pair_results[ck] = outcome
        else:
            pair_results[pk], pair_results[ck] = best_parent, best_child

    # EH cleanup groups: the parent is measured once over its primary span
    # and the combined cold extent; each child's verdict derives from it.
    eh_parents, measured_parents = set(), {}
    member_counts = {}
    for group in eh_groups:
        for member in getattr(group, "members", ()):
            try:
                member_key = key(member)
            except AttributeError:
                continue
            member_counts[member_key] = member_counts.get(member_key, 0) + 1
    for group in eh_groups:
        try:
            members = group.members
            member_keys = [key(member) for member in members]
        except (AttributeError, TypeError):
            continue
        try:
            if group._admission is not _FRAGMENT_ADMISSION:
                raise ValueError("fragment support was not resolved by the package source-view binder")
            if any(member_counts[item] > 1 or item in parent_to_pair or item in child_to_parent
                   for item in member_keys):
                raise ValueError("duplicate fragment relation is ambiguous")
            parent, children = group.parent, tuple(group.children)
            spans = tuple(tuple(span) for span in parent.image_spans)
            emitted_parent = parent.symbol or parent.name
            if (not children or parent.function_fragment is not None
                    or len(spans) != 2 or spans[0][0] != parent.address
                    or any(member.path.resolve() != parent.path.resolve() for member in members)):
                raise ValueError("EH cleanup fragment parent identity or extent is unsupported")
            cursor = children[0].image_spans[0][0]
            if cursor < spans[0][1]:
                raise ValueError("EH cleanup fragments overlap the parent's primary span")
            for child in children:
                relation = child.function_fragment
                if (fragment_fact_kind(relation) != FRAGMENT_EH_CLEANUP
                        or relation.parent_address != parent.address
                        or relation.parent_symbol != emitted_parent
                        or child.context != parent.context
                        or len(child.image_spans) != 1
                        or child.image_spans[0][0] != child.address
                        or child.image_spans[0][0] != cursor
                        or child.image_spans[0][1] <= cursor):
                    raise ValueError("EH cleanup fragments do not tile the parent's cold region")
                cursor = child.image_spans[0][1]
            if cursor != spans[1][0]:
                raise ValueError("EH cleanup fragments do not end at the parent's cold span")
            if any(item in selected and member != selected_by_key[item]
                   for item, member in zip(member_keys, members)):
                raise ValueError("fragment support differs from the selected source mapping")
        except (AttributeError, IndexError, TypeError, ValueError) as error:
            for item in member_keys:
                if item in selected:
                    pair_errors[item] = str(error)
            continue
        if not any(item in selected for item in member_keys):
            continue
        measured = group.measured_parent()
        pk = key(parent)
        eh_parents.add(pk)
        measured_parents[pk] = measured
        refused = next((member for member in members
                        if span_refusal(member, exe, shared) is not None), None)
        if refused is not None:
            outcome = AsmComparison(verdict=span_refusal(refused, exe, shared),
                                    diagnostic=_span_refusal_detail(refused, exe, shared))
        else:
            try:
                outcome = compare_source(measured, exe, source, command, attempts, shared,
                                         compiler=shared_compile, retry_compile=retry_compile,
                                         targets=targets)
            except (OSError, ValueError) as error:
                outcome = str(error)
        pair_results[pk] = outcome
        for child in children:
            pair_results[key(child)] = _eh_fragment_result(child, parent, outcome)

    results = []
    for record in records:
        record_key = key(record)
        if record_key in pair_errors:
            result = pair_errors[record_key]
        elif record.function_fragment is not None:
            result = pair_results.get(record_key, "fragment has no resolved same-TU parent support")
        elif record_key in parent_to_pair or record_key in eh_parents:
            result = pair_results[record_key]
        elif record_key in associated_results:
            result = associated_results[record_key]
        else:
            try:
                result = compare_source(record, exe, source, command, attempts, shared,
                                        compiler=shared_compile, retry_compile=retry_compile,
                                        targets=targets)
            except (OSError, ValueError) as error:
                result = str(error)
        if (diagnostics and isinstance(result, AsmComparison)
                and result.diagnostic_comparison is None
                and record.function_fragment is None and record.mode is not Mode.UNBOUND
                and (result.verdict in (Tier.REFUSED, Tier.SHARED_TAIL)
                     or (record_key in associated_results
                         and result.verdict is not Tier.BYTE_EXACT))):
            result = replace(result, diagnostic_comparison=diagnostic_comparison(
                measured_parents.get(record_key, record), exe, source, command, attempts,
                compiler=shared_compile))
        results.append((record, result))
    current()
    return UnitComparison(tuple(results), count, reused)


def _eh_fragment_result(child, parent, outcome):
    """A cleanup child's verdict, derived only from its parent's whole measurement.

    The combined comparison covers every byte of every child slice, so a
    BYTE_EXACT parent implies each slice. Anything else is never a
    standalone match: a measured difference is a MISMATCH, and an unknown
    parent outcome stays unknown.
    """
    symbol = parent.symbol or parent.name
    prefix = (f"EH cleanup fragment of {symbol} (0x{parent.address:08X}); measured only "
              "within the parent's combined EH extent: ")
    if isinstance(outcome, str):
        return prefix + "parent measurement unresolved: " + outcome
    low, high = child.image_spans[0]
    if outcome.verdict is Tier.BYTE_EXACT:
        return AsmComparison(verdict=Tier.BYTE_EXACT, compared_bytes=high - low,
                             mnemonic_similarity=1.0, flags=outcome.flags,
                             diagnostic=prefix + "parent is BYTE_EXACT")
    if outcome.verdict in (Tier.NO_COMPILE, Tier.REFUSED, Tier.SHARED_TAIL):
        return AsmComparison(verdict=outcome.verdict, flags=outcome.flags,
                             diagnostic=prefix + f"parent is {outcome.verdict}"
                             + (f" ({outcome.diagnostic})" if outcome.diagnostic else ""))
    return AsmComparison(verdict=Tier.MISMATCH, flags=outcome.flags,
                         diagnostic=prefix + f"parent is {outcome.verdict}")


def compare_source(record: DecompilationState, exe: Path | str,
                   source: Path | str, command: list[str],
                   flags: tuple | str,
                   shared: frozenset = frozenset(), *, compiler,
                   retry_compile=None, targets: ReferenceTargets | None = None) -> AsmComparison:
    """How far VC6 reproduces the shipped bytes for `record` when the piece
    is spelled the way `source` spells it, best of `flags`.

    THE CANDIDATE IS THE PARAMETER, and that is what makes an edit-measure
    loop possible. Matching decompilation is a search over SOURCE FORM - a
    ternary against an `if`, a condition's polarity, a temp that changes an
    addressing mode - and there is no reasoning to the answer; the compiler
    has to be asked. Asking it about the tree alone means editing the tree to
    pose the question, so a failed experiment is a dirty checkout, only one
    spelling can be in flight, and an agent with no write access cannot take
    part at all. With `source` the useful question - "which of these nine" -
    is one call per candidate against an unchanged tree.

    `source` need only DEFINE the record's subject under the name the
    annotation records; it is compiled with the record's own invocation, so
    it sees the same include path the real unit does.

    `flags` is a legacy flag string, or a sequence of trials. Each trial may
    instead be a tuple/list of argument tokens, as in native measurement
    configuration; those tokens reach the compiler without shell splitting.
    THERE IS NO DEFAULT SET HERE, and
    that is the point: which invocation built a given function is not
    something this module can know, and a constant sitting in it would be
    read as the answer. It is not even a property of the BUILD - `/Oy-`
    appears nowhere in this tree's CMake configuration. It is a property of
    the FUNCTION: `/O2` implies `/Oy`, which omits the frame pointer, and the
    shipped image is mixed, so either spelling is wrong for roughly half of
    it. A caller that asks about one invocation has asked half a question,
    and the caller is who knows which half it meant.

    THE SEARCH CANNOT LIVE IN `compiled_asm`. Keeping the best result means
    comparing each candidate against the image, and the compiled side alone
    has never seen the image; this is the one function that holds both, so it
    is the one that can choose.

    A COMPILE THAT FAILS IS A RESULT, not an error. `Tier.NO_COMPILE` is
    returned with the compiler's diagnostic, because getting a
    body from there to MISMATCH is the most valuable single move in a
    recovery pass and a loop has to be able to rank it. A stale or missing
    `symbol` fact stays a `ValueError`: that is a defect in the annotation,
    not a fact about the body, and swallowing it would report a body nobody
    compiled as one that did not match.

    SPANS THAT CANNOT BE SCORED come back before any of that - see
    `span_refusal`.

    Compiler failures try the remaining flag sets unless a caller-supplied
    `retry_compile(diagnostic)` says they cannot help. No compiler error codes
    or process-management policy live in this comparer.
    """
    if record.mode is Mode.UNBOUND:
        return AsmComparison(verdict=Tier.REFUSED,
                             diagnostic="UNBOUND mapping has no emitted body or compiler recipe")
    if record.function_fragment is not None:
        raise ValueError("non-callable function fragment requires compare_unit parent support")
    attempts = (flags,) if isinstance(flags, str) else tuple(flags)
    if not attempts:
        raise ValueError("no flag sets to try")
    refusal = span_refusal(record, exe, shared)
    if refusal is not None:
        return AsmComparison(verdict=refusal, flags="", diagnostic=_span_refusal_detail(record, exe, shared))
    original_asm(record, exe)            # raises early if nothing locates it
    best, diagnostic = None, ""
    missing = None
    for attempt in attempts:
        try:
            compiled = compiled_asm(record, command, attempt, source, compiler=compiler)
        except CompileFailed as failed:
            diagnostic = diagnostic or str(failed)
            if retry_compile is None or retry_compile(str(failed)):
                continue
            break
        except SpanComparisonRefused as refused:
            result = AsmComparison(verdict=Tier.REFUSED, diagnostic=str(refused),
                                   flags=attempt if isinstance(attempt, str) else shlex.join(attempt))
            if len(record.image_spans) != 2:
                return result
            # An unsupported candidate layout is a property of this flag
            # trial, not proof that every compiler trial lacks the regions.
            if best is None or _better(result, best):
                best = result
            continue
        except ValueError as absent:
            # A SUBJECT THAT ONE INVOCATION DOES NOT EMIT. An `inline`
            # function is folded into its callers under /O2 and emitted as
            # its own COMDAT under /Ob0, so which flag sets can answer for
            # it is a property of the invocation, not of the annotation.
            # Raising here aborted the whole search on the first set:
            # `say_num` is exactly this shape, and the image has it BOTH
            # ways - a standalone body with three callers, and folded into
            # `prefs_get`. Kept and re-raised only if NO set finds it,
            # because that is the stale-fact case this must still catch.
            missing = missing or absent
            continue
        try:
            result = compare_subject(record, exe, compiled, targets=targets)
        except SpanComparisonRefused as refused:
            # Another trial may emit a supported EH layout. Never use a
            # primary-only comparison as fallback when a region is unknown.
            result = AsmComparison(verdict=Tier.REFUSED, flags=compiled.flags,
                                   diagnostic=str(refused))
        # TIED ON TIER, DECIDED ON AGREEMENT. Every candidate is measured
        # against the same original, so the instruction counts compare
        # directly - and keeping the first of four MISMATCHes reports a
        # diagnostic nobody can act on. 0x00612B80 gave 0 of 35 under the
        # frame-pointer set and 35 of 35 without it, both MISMATCH; the
        # first was what came back, and it points at the prologue instead
        # of the tail that actually differs.
        if best is None or _better(result, best):
            best = result
        if best.verdict is Tier.BYTE_EXACT:
            break                    # nothing beats byte equality
    if best is None:
        if missing is not None:
            raise missing            # no invocation emitted it: a bad fact
        return AsmComparison(verdict=Tier.NO_COMPILE, diagnostic=diagnostic)
    return best
