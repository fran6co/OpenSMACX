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

import json
import shlex
import struct
import subprocess
import tempfile
from pathlib import Path

from .model import DecompilationState


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


def original_asm(record: DecompilationState, exe: Path | str) -> list:
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
    return _disasm(_pe_bytes(exe, low, high - low), low)


# --------------------------------------------------------------- the compile


def _entry_for(source: Path, compile_commands: Path) -> dict:
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


def _compile_command(entry: dict, flags: str) -> list:
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


def _coff_function(obj: bytes, symbol: str) -> bytes:
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
        sections.append((name, raw_size, raw_ptr))

    matches = []
    i = 0
    while i < n_syms:
        off = sym_ptr + i * 18
        name = sym_name(obj[off:off + 8])
        value, section_no, _type, storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if storage == 2 and 0 < section_no <= len(sections):
            sec = sections[section_no - 1]
            if sec[0] == ".text" and name in (symbol, "_" + symbol):
                matches.append((name, value, section_no - 1))
        i += 1 + n_aux

    if not matches:
        raise ValueError(
            f"{symbol} not found among the object's .text symbols")
    if len(matches) > 1:
        raise ValueError(f"{symbol} is ambiguous in the object")
    _name, value, sec = matches[0]
    _sec_name, raw_size, raw_ptr = sections[sec]
    code = obj[raw_ptr + value:raw_ptr + raw_size]
    end = len(code)
    while end > 0 and code[end - 1] in (0x90, 0xCC):
        end -= 1
    return code[:end]


def compiled_asm(record: DecompilationState, compile_commands: Path | str,
                 flags: str) -> list:
    """The assembly VC6 produces for the record's code: the record's file
    compiled by the build's own compile command with its optimisation flags
    swapped for `flags`, the function pulled out of the object by the
    record's mangled name, disassembled at the record's address so the
    listing lines up with `original_asm`."""
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    obj = _compile(record.path, flags, Path(compile_commands))
    return _disasm(_coff_function(obj, record.name), record.address)


# ------------------------------------------------------------------ disasm


def _disasm(data: bytes, base: int) -> list:
    """Lines of `0xADDR  bytes  mnemonic operands`, capstone under the hood."""
    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        raise ImportError(
            "capstone is not installed - `uv sync` installs it") from None
    return [f"0x{ins.address:08X}  "
            f"{' '.join(f'{b:02X}' for b in ins.bytes):<24}  "
            f"{ins.mnemonic} {ins.op_str}".rstrip()
            for ins in Cs(CS_ARCH_X86, CS_MODE_32).disasm(data, base)]
