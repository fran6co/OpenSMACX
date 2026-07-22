#!/usr/bin/env python3
"""Disassemble a canonical address range from the pinned executable.

Every recovery in this repository is settled by disassembly rather than by
decompiler output: the decompiler drops the `this` pointer on `__thiscall`
functions, invents parameter counts, and silently normalises the callee-pop
byte count that is the only ground truth for arity. Reading the bytes is how a
candidate gets rejected before it compiles into something plausible and wrong.

Until now that step meant an ad-hoc script each time. This exposes it as one
command over the same hash-pinned executable and the same capstone
configuration the island extractor already uses, so what this prints and what
the extractor consumes cannot drift apart.

Output is disassembly of proprietary bytes: local analysis only, never
committed.
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"


def load_functions() -> dict[int, dict[str, str]]:
    if not FUNCTIONS_CSV.is_file():
        return {}
    table = {}
    with FUNCTIONS_CSV.open() as handle:
        for row in csv.DictReader(handle):
            try:
                table[int(row["address"], 16)] = row
            except (KeyError, ValueError):
                continue
    return table


def resolve(text: str, functions: dict[int, dict[str, str]]) -> tuple[int, int | None]:
    """Accept a hex address or a mangled name; return (address, size)."""
    try:
        address = int(text, 16)
    except ValueError:
        for candidate, row in functions.items():
            if row.get("name") == text:
                address = candidate
                break
        else:
            raise ValueError(f"{text} is neither a hex address nor a known name")
    row = functions.get(address)
    size = int(row["size"]) if row and row.get("size") else None
    return address, size


def read_range(pe: pefile.PE, start: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    rva = start - base
    for section in pe.sections:
        begin = section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= rva < end:
            offset = section.PointerToRawData + (rva - begin)
            return pe.__data__[offset:offset + length]
    raise ValueError(f"0x{start:08X} is outside every section")


def annotate(mnemonic: str, operand: str,
             functions: dict[int, dict[str, str]]) -> str:
    """Name a call/jump target so the reader does not have to look it up."""
    if mnemonic not in ("call", "jmp") or not operand.startswith("0x"):
        return ""
    try:
        row = functions.get(int(operand, 16))
    except ValueError:
        return ""
    return f"  ; {row['name']}" if row and row.get("name") else ""


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("target", help="hex address, or a mangled function name")
    parser.add_argument("--length", type=lambda v: int(v, 0),
                        help="bytes to disassemble; defaults to the catalogued "
                             "function size, then 64")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--bytes", action="store_true",
                        help="show raw opcode bytes alongside the mnemonics")
    args = parser.parse_args()

    functions = load_functions()
    try:
        address, catalogued = resolve(args.target, functions)
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    length = args.length or catalogued or 64
    pe = pefile.PE(str(args.exe), fast_load=True)
    try:
        code = read_range(pe, address, length)
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    row = functions.get(address)
    if row and row.get("name"):
        print(f"; {row['name']}  ({length} bytes at 0x{address:08X})")

    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    for instruction in engine.disasm(code, address):
        raw = ""
        if args.bytes:
            raw = " ".join(f"{b:02x}" for b in instruction.bytes).ljust(24)
        note = annotate(instruction.mnemonic, instruction.op_str, functions)
        operand = f" {instruction.op_str}" if instruction.op_str else ""
        print(f"0x{instruction.address:08X}  {raw}"
              f"{instruction.mnemonic}{operand}{note}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
