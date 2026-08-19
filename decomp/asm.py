"""The assembly behind a record: the original image's, VC6's, and the verdict.

One call answers the recovery's central question for one record:

    from pathlib import Path
    from decomp import read_file
    from decomp.asm import compare_record

    records = read_file(Path("src/buffer.cpp"))
    record = next(r for r in records if r.address == 0x005DAC70)
    from decomp.asm import build_command
    command = build_command(Path("build/compile_commands.json"),
                            record.path)
    compare_record(record,
                   Path(".opensmacx/game/terranx_original.exe"),
                   command, FLAG_SETS).verdict

and the two sides are still available separately, as `original_asm` and
`compiled_asm`.

THE ORIGINAL SIDE reads the pinned executable's bytes at the record's span
and disassembles them. THE COMPILED SIDE compiles the record's file with a
command the CALLER supplies - `build_command` reads one out of the build's
own database - plus the optimisation flags the caller is asking about, and
disassembles the function pulled out of the resulting object. The compiler the entry names is the build's own wrapper -
it knows where VC6 lives and how to run it under wine, so this module never
asks. Both return a `Listing`: lines shaped like tools/disasm.py's output,
carrying the bytes and the relocation mask behind them.

THREE THINGS A CORRECT COMPARISON NEEDS, each of which this module got
wrong until it was measured against the tree. Four bodies carrying a
BYTE_EXACT claim in `src/` all reported MISMATCH:

  * THE FLAG SET IS PER FUNCTION - see `compare_record`, which takes as
    many as the caller cares to name. Asking one answers half the image.
  * A RELOCATED BYTE IS NOT A WRONG ANSWER. The object holds zeros where
    the image holds linked addresses; `compare_asm` discounts both sides.
  * THE CATALOGUE'S NAME IS NOT ALWAYS THE COMPILER'S. What the image
    calls a piece and what this tree emits for it are two facts, and the
    annotation carries both - `name` and `symbol`. The lookup reads them;
    it does not guess, and a missing fact is an error that names itself.

SEVEN TIERS, and the last three are not failures. NO_COMPILE is a body
that did not build, returned as a verdict because getting one from there to
MISMATCH is the most valuable single move in a recovery pass. SHARED_TAIL
and REFUSED are spans no verdict is DEFINED on - a `/Gy`-folded tail
belonging to several functions, a span that rewrites itself at run time -
and keeping them apart from the misses is what lets a reader tell a wall
from a body nobody has written. See `model.Tier`.

WHAT IS STILL ELSEWHERE: the EH funclet classification, jump-table
clipping, batching, caching and the ledger.

CAPSTONE is imported like anything else. It is a declared dependency, so a
lazy import bought nothing but a failure that arrives halfway through a
measurement instead of at import; the readers that must not need it live in
other modules, and `python -m decomp` checks that split. The PE and COFF
readers below are small struct walks instead of dependencies, because all
they need is the section table, the symbol table and two relocation
directories.
"""

from __future__ import annotations

import difflib
import functools
import json
import re
import shlex
import struct
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs, x86

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
    """

    code: bytes
    base: int                        # the address the code is disassembled at
    mask: frozenset[int] = frozenset()   # offsets into `code`, not addresses
    flags: str = ""                  # the invocation that produced it, "" for
                                     # the image, which was not invoked

    @functools.cached_property
    def instructions(self) -> list:
        """The decoded instructions, operands and all."""
        return _decode(self.code, self.base)

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
    """CL produced no object. Carries what CL said about it.

    Its own type because a loop treats it as a RESULT - `Tier.NO_COMPILE` -
    while a stale or missing `symbol` fact is a defect in the annotation and
    stays a `ValueError` the caller must deal with. Both used to be one
    exception, so a body that would not build and an annotation that lied
    were indistinguishable to everything above.
    """


def _diagnostic(stdout: str, stderr: str) -> str:
    """The first line CL called an error, or the last thing it said."""
    for stream in (stdout or "", stderr or ""):
        for line in stream.splitlines():
            if "error" in line.lower():
                return line.strip()
    tail = (stdout or stderr or "").strip().splitlines()
    return tail[-1].strip() if tail else "the compile produced no object"


# SECTION CHARACTERISTICS. A section that is both executable and writable is
# code that can rewrite itself - which is what the copy protection in this
# image does, and why nothing a compiler emits can equal a body that is not
# the body at rest.
IMAGE_SCN_MEM_EXECUTE = 0x20000000
IMAGE_SCN_MEM_WRITE = 0x80000000


# -------------------------------------------------------------- the original


def _pe_bytes(exe: Path, address: int, size: int) -> bytes:
    """The image's bytes at `address`, mapped through the section table."""
    data = exe.read_bytes()
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
    data = exe.read_bytes()
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
    return _base_relocations(exe.read_bytes())


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


def original_asm(record: DecompilationState, exe: Path | str) -> Listing:
    """The shipped image's assembly for `record` - the bytes at its
    primary span in `exe`, disassembled. A record can carry no spans - a
    fresh annotation the catalogue has not stamped yet - and then nothing
    locates it in the image, so the call says so."""
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
    return Listing(code=code, base=low,
                   mask=_image_mask(exe, low, low + len(code)))


# --------------------------------------------------------------- the compile


def build_command(compile_commands: Path | str,
                  source: Path | str) -> list[str]:
    """The compiler invocation this build uses for `source`, without its
    optimisation flags.

    The compiler and the include and define flags - what a translation unit
    needs to parse - and nothing about how it is optimised, because that is
    a property of the FUNCTION being matched and belongs to the caller. See
    `compare_record`.

    RAISES IF THE BUILD DOES NOT NAME `source`, and does not go looking for
    something close. A build entry is a command, not a permission: this
    tree's `src/recovered/` and `src/unrecovered/` hold 3,315 annotated
    records CMake has no reason to build, 2,475 of them FILE-mode, where the
    file already IS the translation unit and the only thing missing is an
    invocation. Those are perfectly compilable with a command borrowed from
    anywhere in the same tree - and WHICH command is a judgement about which
    include path a unit needs, which the caller is holding and this module
    is not. Borrow deliberately:

        command = build_command(cc, record.path if in_build else a_sibling)

    The leading tokens up to the first flag are the compiler the build
    invokes - this tree's wrapper, which knows where VC6 lives and how to
    run it under wine. PCH flags are dropped, because they point at a
    precompiled header built for the build's own object directory.
    """
    compile_commands = Path(compile_commands)
    if not compile_commands.is_file():
        raise ValueError(
            f"no {compile_commands} - configure the build first")
    table = json.loads(compile_commands.read_text())
    resolved = Path(source).resolve()
    for candidate in table:
        file = Path(candidate["file"])
        if not file.is_absolute():
            file = Path(candidate.get("directory", ".")) / file
        if file.resolve() == resolved:
            return _invocation(candidate)
    raise ValueError(
        f"{Path(source).name} is not a build input in "
        f"{compile_commands.name}; pass the command of a file that is")


def _invocation(entry: dict[str, str]) -> list[str]:
    """An entry's compiler and its include and define flags."""
    words = shlex.split(entry.get("command")
                        or " ".join(entry.get("arguments", [])))
    split = 0
    for index, word in enumerate(words):
        if index and word.startswith(("-", "/", "@")):
            split = index
            break
    compiler, kept = words[:split], []
    for word in words[split:]:
        if "cmake_pch" in word or word.startswith(("/Fo", "/Fd", "/Fp")):
            continue
        if word.startswith(("-I", "-D", "-FI")):
            word = "/" + word[1:]
        if (word.startswith(("/I", "/D")) and len(word) > 2) or \
                (word.startswith("/FI") and len(word) > 3):
            kept.append(word)
    return [*compiler, "/nologo", *kept]


def compile_unit(source: Path, command: list[str], flags: str) -> bytes:
    """Compile `source` with `command` plus `flags`; return the object.

    PUBLIC BECAUSE ONE OBJECT ANSWERS FOR MANY SUBJECTS. `compiled_asm`
    compiles per record, which is right for one question and wrong for a
    sweep: this tree's 626 reachable claims live in 96 files, so asking per
    record is 2,504 compiles where asking per file is 384. A caller doing
    the second calls this once and `subject_asm` many times.
    """
    with tempfile.TemporaryDirectory() as work:
        result = subprocess.run([*command, *flags.split(), "/Founit.obj",
                                 str(source)],
                                cwd=work, capture_output=True, text=True,
                                timeout=120)
        obj = Path(work) / "unit.obj"
        if not obj.is_file():
            raise CompileFailed(_diagnostic(result.stdout, result.stderr))
        return obj.read_bytes()


def _coff_function_masked(obj: bytes, symbol: str, emitted: str = "",
                          ) -> tuple[bytes, frozenset[int]]:
    """The symbol's code out of a COFF object - the `.text` section named
    by the symbol, from the symbol's value to the section's end, trailing
    NOP/INT3 padding stripped. With `/Gy` each function is its own COMDAT
    section, so the section end is the function end."""
    machine, n_sections, _ts, sym_ptr, n_syms, _opt, _ch = \
        struct.unpack_from("<HHIIIHH", obj, 0)
    if machine != 0x14C:
        raise ValueError("not an i386 COFF object")
    str_ptr = sym_ptr + n_syms * 18

    def sym_name(field: bytes) -> str:
        if field[:4] == b"\x00\x00\x00\x00":
            (off,) = struct.unpack("<I", field[4:])
            end = obj.index(b"\x00", str_ptr + off)
            return obj[str_ptr + off:end].decode()
        return field.rstrip(b"\x00").decode()

    sections = []
    for i in range(n_sections):
        off = 20 + i * 40
        name = obj[off:off + 8].rstrip(b"\x00").decode()
        raw_size, raw_ptr = struct.unpack_from("<II", obj, off + 16)
        reloc_ptr, = struct.unpack_from("<I", obj, off + 24)
        n_relocs, = struct.unpack_from("<H", obj, off + 32)
        sections.append((name, raw_size, raw_ptr, reloc_ptr, n_relocs))

    externals = []
    i = 0
    while i < n_syms:
        off = sym_ptr + i * 18
        name = sym_name(obj[off:off + 8])
        value, section_no, _type, storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if storage == 2 and 0 < section_no <= len(sections) \
                and sections[section_no - 1][0] == ".text":
            externals.append((name, value, section_no - 1))
        i += 1 + n_aux

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
    matches = [e for e in externals
               if e[0] == wanted or (not emitted and e[0] == "_" + symbol)]
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
    if len({(value, sec) for _n, value, sec in matches}) > 1:
        raise ValueError(f"{wanted} is ambiguous in the object")
    _name, value, sec = matches[0]
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
                 flags: str, source: Path | str | None = None) -> Listing:
    """The assembly VC6 produces for the record's code: the record's file
    compiled by the build's own compile command with its optimisation flags
    swapped for `flags`, the function pulled out of the object by the
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
    obj = compile_unit(Path(source) if source else record.path,
                       command, flags)
    return subject_asm(obj, record, flags)


def subject_asm(obj: bytes, record: DecompilationState,
                flags: str = "") -> Listing:
    """The record's own function, pulled out of an object already compiled.

    The half of `compiled_asm` that is not the compile, so that one object
    can answer for every subject its file defines.
    """
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    code, mask = _coff_function_masked(obj, record.name, record.symbol)
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

    constants: list = []
    if n == m and matching == n:
        verdict = Tier.BYTE_EXACT
    elif original.shapes == compiled.shapes:
        verdict = Tier.SHAPE_EXACT
        constants = differing_constants(original, compiled)
    elif original.mnemonics == compiled.mnemonics:
        verdict = Tier.MNEMONIC_ONLY
    else:
        verdict = Tier.MISMATCH
    masked = len(original.mask | compiled.mask)
    return AsmComparison(
        verdict=verdict,
        compared_bytes=max(0, len(original.code) - masked),
        masked_bytes=masked,
        original_instructions=n, compiled_instructions=m,
        matching_instructions=matching, mnemonic_similarity=similarity,
        first_divergence=first_divergence, context=context,
        differing_constants=tuple(constants), flags=compiled.flags)


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
    """Is `candidate` the more useful of two measurements of one record?"""
    return ((candidate.verdict.rank, -candidate.matching_instructions)
            < (incumbent.verdict.rank, -incumbent.matching_instructions))


def compare_record(record: DecompilationState, exe: Path | str,
                   command: list[str], flags: tuple | str,
                   shared: frozenset = frozenset()) -> AsmComparison:
    """How far VC6 reproduces the shipped bytes for the body IN THE TREE.

    `compare_source` with the record's own file - which is the question
    `check` and `record` ask, and the only one that can be written down.
    """
    return compare_source(record, exe, record.path, command, flags, shared)


def compare_source(record: DecompilationState, exe: Path | str,
                   source: Path | str, command: list[str],
                   flags: tuple | str,
                   shared: frozenset = frozenset()) -> AsmComparison:
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

    `flags` is a flag string, or several. THERE IS NO DEFAULT SET HERE, and
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
    returned with the compiler's own first error line, because getting a
    body from there to MISMATCH is the most valuable single move in a
    recovery pass and a loop has to be able to rank it. A stale or missing
    `symbol` fact stays a `ValueError`: that is a defect in the annotation,
    not a fact about the body, and swallowing it would report a body nobody
    compiled as one that did not match.

    SPANS THAT CANNOT BE SCORED come back before any of that - see
    `span_refusal`.
    """
    attempts = (flags,) if isinstance(flags, str) else tuple(flags)
    if not attempts:
        raise ValueError("no flag sets to try")
    refusal = span_refusal(record, exe, shared)
    if refusal is not None:
        return AsmComparison(verdict=refusal, flags="")
    original = original_asm(record, exe)
    best, diagnostic = None, ""
    for attempt in attempts:
        try:
            compiled = compiled_asm(record, command, attempt, source)
        except CompileFailed as failed:
            diagnostic = diagnostic or str(failed)
            continue
        result = compare_asm(original, compiled)
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
        return AsmComparison(verdict=Tier.NO_COMPILE, diagnostic=diagnostic)
    return best
