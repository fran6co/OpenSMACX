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
import re
import shlex
import struct
from dataclasses import dataclass, field, replace
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs, x86

from ..pe_coff import Coff
from .model import DecompilationState, Tier

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

    out, i = [], 0
    while i < n_syms:
        off = sym_ptr + i * 18
        name = sym_name(obj[off:off + 8])
        value, section_no, _type, storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if 0 < section_no <= len(names) and (
                storage == 3 or storage == 2 and names[section_no - 1] == ".text"):
            out.append((name, value, section_no - 1, storage))
        i += 1 + n_aux
    return out


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
    # The legacy external comparer can extend the original read for switch
    # tails. A local mapping must not borrow a neighboring original body.
    if original_extent is not None and len(code) > original_extent:
        raise ValueError("local function exceeds its explicit original image span")
    masked = set()
    for relocation in relocations:
        if not 0 <= relocation.offset <= len(code) - 4:
            raise ValueError("local function relocation crosses its code boundary")
        field = set(range(relocation.offset, relocation.offset + 4))
        if field & masked:
            raise ValueError("local function has overlapping relocations")
        masked.update(field)
    return code, frozenset(masked)


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
    _name, value, sec, _storage = matches[0]
    _sec_name, raw_size, raw_ptr, reloc_ptr, n_relocs = sections[sec]
    code = obj[raw_ptr + value:raw_ptr + raw_size]
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


def compiled_asm(record: DecompilationState, command: list[str],
                 flags: str | tuple[str, ...] | list[str],
                 source: Path | str | None = None, *, compiler) -> Listing:
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
                flags: str = "") -> Listing:
    """The record's own function, pulled out of an object already compiled.

    The half of `compiled_asm` that is not the compile, so that one object
    can answer for every subject its file defines.
    """
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    code, mask = _coff_function_masked(obj, record.name, record.symbol, spans=record.image_spans)
    return Listing(code=code, base=record.address, mask=mask, flags=flags)


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
    diagnostic: str = ""            # NO_COMPILE only: what the compiler said
    flags: str = ""                 # the flag set that produced `compiled`


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
    at = 0
    for instruction in compiled.instructions:
        at += instruction.size
        if at < extent:
            continue
        if at > extent:                     # not on an instruction boundary
            return compiled
        if not instruction.mnemonic.startswith(_TERMINATORS):
            return compiled
        return replace(compiled, code_bytes=extent)
    return compiled


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
    similarity = difflib.SequenceMatcher(
        None, original.mnemonics, compiled.mnemonics).ratio()

    # THE TAIL IS PART OF THE ANSWER. A body whose instructions all agree
    # and whose jump table does not has not reproduced the function.
    data_divergence = _data_divergence(original, compiled)

    constants: list = []
    if n == m and matching == n and data_divergence is None:
        verdict = Tier.BYTE_EXACT
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
        flags=compiled.flags)


def compare_subject(record: DecompilationState, exe: Path | str,
                    compiled: Listing) -> AsmComparison:
    """`compare_asm` with the image side aligned to the object's extent.

    THE OBJECT DECIDES HOW MUCH IMAGE TO READ, because only it knows how
    long the COMDAT is - the catalogue's span is the CODE, and a `switch`'s
    jump table sits past it. Every caller holding a compiled `Listing`
    wants this rather than `compare_asm` directly; `compare_asm` stays the
    pure comparison of two listings, which is what makes it testable
    without an image.
    """
    original = original_asm(record, exe)
    if len(compiled.code) > len(original.code):
        try:
            original = original_asm(record, exe, extent=len(compiled.code))
        except ValueError:
            # The tail runs past the section's raw data, so the image has no
            # copy of it to compare against. The comparison stays what it
            # was, and the extra instructions read as the mismatch they are.
            pass
    return compare_asm(original, compiled)


def span_refusal(record: DecompilationState, exe: Path | str,
                 shared: frozenset = frozenset()) -> Tier | None:
    """The tier for a record no verdict is defined on, or None.

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
        return Tier.REFUSED
    low, high = record.image_spans[0]
    if rewrites_itself(exe, low):
        return Tier.REFUSED
    if (low, high) in shared:
        return Tier.SHARED_TAIL
    return None


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
                   retry_compile=None) -> AsmComparison:
    """How far VC6 reproduces the shipped bytes for the body IN THE TREE.

    `compare_source` with the record's own file - which is the question
    `check` and `record` ask, and the only one that can be written down.
    """
    return compare_source(record, exe, record.path, command, flags, shared,
                          compiler=compiler, retry_compile=retry_compile)


@dataclass(frozen=True)
class UnitComparison:
    """Disposable measurements in selection order; never source claims.

    A string result is unresolved original/symbol evidence, not a compilation
    verdict. Objects and failed attempts are reused only inside this call.
    """
    results: tuple[tuple[DecompilationState, AsmComparison | str], ...]
    compile_attempts: int
    reused_attempts: int


def compare_unit(records, exe: Path | str, source: Path | str,
                 command: list[str], flags: tuple | str,
                 shared: frozenset = frozenset(), *, compiler,
                 retry_compile=None) -> UnitComparison:
    """Measure one TU's selected mappings with one compile per flag trial.

    Use the same per-function selection, span refusals, missing-symbol retries
    and similarity ranking as compare_source, rather than another scoring loop.
    Different subjects may need different trials. One unresolved subject never
    discards measured siblings. The caller owns whole-project input freshness
    and publication; this function also refuses changed candidate/marker files.
    No objects, scores or failures are cached across calls.
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
    inputs = {path: path.read_bytes() for path in {source, *(item.path for item in records)}}
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

    results = []
    for record in records:
        try:
            result = compare_source(record, exe, source, command, attempts, shared,
                                    compiler=shared_compile, retry_compile=retry_compile)
        except (OSError, ValueError) as error:
            result = str(error)
        results.append((record, result))
    current()
    return UnitComparison(tuple(results), count, reused)


def compare_source(record: DecompilationState, exe: Path | str,
                   source: Path | str, command: list[str],
                   flags: tuple | str,
                   shared: frozenset = frozenset(), *, compiler,
                   retry_compile=None) -> AsmComparison:
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
    attempts = (flags,) if isinstance(flags, str) else tuple(flags)
    if not attempts:
        raise ValueError("no flag sets to try")
    refusal = span_refusal(record, exe, shared)
    if refusal is not None:
        return AsmComparison(verdict=refusal, flags="")
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
        result = compare_subject(record, exe, compiled)
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
