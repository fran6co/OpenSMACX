"""The assembly behind a record: the original image's, and VC6's.

Two functions, one per side of the recovery's central question:

    from pathlib import Path
    from decomp import read_file
    from decomp.asm import compiled_asm, original_asm

    records = read_file(Path("src/buffer.cpp"))
    record = next(r for r in records if r.address == 0x005D7210)
    # what the shipped image contains:
    original_asm(record, Path(".opensmacx/game/terranx_original.exe"))
    # what VC6 makes of the record's code, compiled with the flags the
    # ratchet measures with:
    compiled_asm(record, Path("build/compile_commands.json"),
                 "/c /O2 /Gy /GR- /Oy- /GX")

THE ORIGINAL SIDE reads the pinned executable's bytes at the record's span
and disassembles them. THE COMPILED SIDE takes the build's own compile
command for the record's file from compile_commands.json, keeps its
compiler and its include and define flags, swaps in the caller's
optimisation flags, and disassembles the function pulled out of the
resulting object by the record's mangled name. The compiler the entry
names is the build's own wrapper - it knows where VC6 lives and how to
run it under wine, so this module never asks. Both functions return lines
shaped like tools/disasm.py's output: address, bytes, mnemonic.

CAPSTONE is imported lazily and is the one thing here that is not standard
library; `uv sync` installs it. The PE and COFF readers below are small
struct walks instead of dependencies, because all they need is the section
table and the symbol table. What this module does NOT do is the ratchet's
comparison machinery - relocation masking, the verdict tiers, batching, the
ledger stay in tools/byte_match.py. These two functions return assembly;
what is done with the pair is the caller's question.
"""

from __future__ import annotations

from collections.abc import Iterable

import difflib
import functools
import json
import re
import shlex
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from .model import DecompilationState, Tier

class Listing(list):
    """Disassembly lines, plus the bytes and the mask behind them.

    A `list[str]` first, so everything that treats a listing as lines of text
    keeps working unchanged. The attributes carry what a listing CANNOT: the
    code itself, the address it was disassembled at, and the offsets a
    relocation makes unknowable.

    THE MASK IS WHY THIS EXISTS. An object file's `push <global>` is
    `68 00 00 00 00` with a relocation on the immediate; the image has the
    linked address in those four bytes. Comparing the rendered text calls that
    a divergence, which is how a body that reproduces the shipped bytes
    exactly was reported as MISMATCH. A byte a relocation determines is not a
    wrong answer, and `compare_asm` discounts it - but only if it is told
    which bytes those are.
    """

    def __init__(self, lines: Iterable[str], *, code: bytes = b"",
                 base: int = 0, mask: frozenset[int] = frozenset(),
                 flags: str = "") -> None:
        super().__init__(lines)
        self.code = code
        self.base = base
        self.mask = mask        # offsets into `code`, not addresses
        self.flags = flags      # the flag set that produced it, "" for the image


# WHAT THE COMPARISON ACCEPTS. `original_asm` and `compiled_asm` return
# `Listing`s and a caller may hand in bare lines; the difference is not
# cosmetic, because only a `Listing` carries the mask that lets a relocated
# byte be discounted. Spelling the union out is the honest signature: a
# `list[str]` alone would say the mask branch cannot be reached, and a bare
# `Listing` would say plain lines are refused when they are not.
Disassembly = Listing | list[str]


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
    return Listing(_disasm(code, low), code=code, base=low,
                   mask=_image_mask(exe, low, low + len(code)))


# --------------------------------------------------------------- the compile


def _entry_for(source: Path, compile_commands: Path) -> dict[str, str]:
    """The compile_commands entry describing how the build compiles
    `source`."""
    if not compile_commands.is_file():
        raise ValueError(
            f"no {compile_commands} - configure the build first")
    table = json.loads(compile_commands.read_text())
    resolved = source.resolve()
    for candidate in table:
        file = Path(candidate["file"])
        if not file.is_absolute():
            file = Path(candidate.get("directory", ".")) / file
        if file.resolve() == resolved:
            return candidate
    raise ValueError(
        f"{source.name} is not a build input in {compile_commands.name}")


def _compile_command(entry: dict[str, str], flags: str) -> list[str]:
    """The build's own compile command with its optimisation flags swapped
    for `flags`.

    The leading tokens up to the first flag are the compiler the build
    invokes - this tree's wrapper, which knows where VC6 lives and how to
    run it under wine, and translates POSIX paths on its own. The include
    and define flags stay exactly as the build parses them; its PCH flags
    are dropped, because they point at a precompiled header built for the
    build's own object directory. `/Fo` and `/Fd` are replaced by the
    caller.
    """
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
    return [*compiler, "/nologo", *flags.split(), *kept]


def _compile(source: Path, flags: str, compile_commands: Path) -> bytes:
    """Compile `source` with the build's own command; return the object's
    bytes."""
    command = _compile_command(_entry_for(source, compile_commands), flags)
    with tempfile.TemporaryDirectory() as work:
        result = subprocess.run([*command, "/Founit.obj", str(source)],
                                cwd=work, capture_output=True, text=True,
                                timeout=120)
        obj = Path(work) / "unit.obj"
        if not obj.is_file():
            raise ValueError("the compile produced no object:\n"
                             f"{result.stdout}\n{result.stderr}")
        return obj.read_bytes()


def _coff_function_masked(obj: bytes, symbol: str,
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

    matches = [e for e in externals if e[0] in (symbol, "_" + symbol)]
    if not matches:
        raise ValueError(
            f"{symbol} not found among the object's .text symbols")
    if len({(value, sec) for _n, value, sec in matches}) > 1:
        raise ValueError(f"{symbol} is ambiguous in the object")
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


def _coff_function(obj: bytes, symbol: str) -> bytes:
    """The symbol's code alone, without the relocation mask beside it."""
    return _coff_function_masked(obj, symbol)[0]


def compiled_asm(record: DecompilationState, compile_commands: Path | str,
                 flags: str) -> Listing:
    """The assembly VC6 produces for the record's code: the record's file
    compiled by the build's own compile command with its optimisation flags
    swapped for `flags`, the function pulled out of the object by the
    record's mangled name, disassembled at the record's address so the
    listing lines up with `original_asm`."""
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    obj = _compile(record.path, flags, Path(compile_commands))
    code, mask = _coff_function_masked(obj, record.name)
    return Listing(_disasm(code, record.address), code=code,
                   base=record.address, mask=mask, flags=flags)


# ------------------------------------------------------------------ disasm


def _disasm(data: bytes, base: int) -> list[str]:
    """Lines of `0xADDR  bytes  mnemonic operands`, capstone under the hood."""
    return [f"0x{ins.address:08X}  "
            f"{' '.join(f'{b:02X}' for b in ins.bytes):<24}  "
            f"{ins.mnemonic} {ins.op_str}".rstrip()
            for ins in Cs(CS_ARCH_X86, CS_MODE_32).disasm(data, base)]


# -------------------------------------------------------------- comparison


@dataclass
class AsmComparison:
    """How far one assembly listing reproduces another."""
    verdict: str                    # BYTE_EXACT | MNEMONIC_ONLY | MISMATCH
    original_count: int
    compiled_count: int
    matching_lines: int             # positionally identical listing lines
    mnemonic_similarity: float      # 0..1 across the instruction sequences
    first_divergence: int | None    # first differing instruction, or None
    context: tuple                  # (original, compiled) lines around it


def _mnemonic_sequence(listing: list) -> list:
    """The instruction sequence of a listing, for similarity.

    `ret` keeps its operand: `ret` and `ret 4` are different instructions,
    and the pop count is exactly what a calling-convention slip changes.
    """
    out = []
    for line in listing:
        parts = re.split(r"\s{2,}", line.strip(), maxsplit=2)
        if len(parts) < 3:
            out.append(line.strip())
            continue
        instruction = parts[2].strip()
        out.append(instruction
                   if instruction.split()[0].startswith("ret")
                   else instruction.split()[0])
    return out


def compare_asm(original: list, compiled: list) -> AsmComparison:
    """How far the compiled listing reproduces the original one.

    The verdicts mirror tools/byte_match.py at the listing level:
    BYTE_EXACT when every instruction agrees byte for byte, MNEMONIC_ONLY
    when the instruction sequence agrees but constants or encodings differ,
    MISMATCH otherwise. The report says how many lines match outright, how
    similar the instruction sequences are, and where the first divergence
    is, with a window of both listings around it.
    """
    n, m = len(original), len(compiled)
    matching = 0
    first_divergence = None
    context: tuple = ([], [])
    for index, (a, b) in enumerate(zip(original, compiled)):
        if a == b:
            matching += 1
        elif first_divergence is None:
            first_divergence = index
            lo = max(0, index - 3)
            context = (original[lo:index + 4], compiled[lo:index + 4])
    if first_divergence is None and n != m:
        first_divergence = min(n, m)
        lo = max(0, min(n, m) - 3)
        context = (original[lo:], compiled[lo:])

    seq_original = _mnemonic_sequence(original)
    seq_compiled = _mnemonic_sequence(compiled)
    similarity = difflib.SequenceMatcher(
        None, seq_original, seq_compiled).ratio()

    if n == m and matching == n:
        verdict = "BYTE_EXACT"
    elif seq_original == seq_compiled:
        verdict = "MNEMONIC_ONLY"
    else:
        verdict = "MISMATCH"
    return AsmComparison(verdict, n, m, matching, similarity,
                         first_divergence, context)
