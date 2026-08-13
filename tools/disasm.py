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
import functools
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile

import recovery_symbols  # noqa: E402
from generator_support import read_bytes  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402


def _catalogue_rows():
    """Every catalogued row, from `src/`.

    `docs/recovery/functions.csv` is deleted: every `ORIGINAL:` annotation
    carries its own name, size, spans, prototype, kind, flags and call
    edges, and `emit.load_functions()` reads them back. This tool opened
    the CSV directly, so it broke the moment the store moved - which is
    how five layout gates went red at once.
    """
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
SYMBOL_MAP = REPO_ROOT / "build" / "target-objects" / "symbol-map.json"
OBJECT_ADDRESS = re.compile(r"([0-9a-f]{8})\.obj$")


def load_functions() -> dict[int, dict[str, str]]:
    table = {}
    # `src/` is the catalogue's store; the export is deleted.
    for row in _catalogue_rows():
        try:
            table[int(row["address"], 16)] = row
        except (KeyError, ValueError):
            continue
    return table


@functools.lru_cache(maxsize=1)
def symbol_map() -> dict:
    """{symbol: address} from `build/target-objects/symbol-map.json`.

    The map is {symbol: object path} and every object is named for the
    address it holds, so the address comes out of the filename.
    """
    if not SYMBOL_MAP.is_file():
        return {}
    out = {}
    for symbol, path in json.loads(SYMBOL_MAP.read_text()).items():
        found = OBJECT_ADDRESS.search(path)
        if found:
            out[symbol] = int(found.group(1), 16)
    return out


def resolve(text: str, functions: dict[int, dict[str, str]]) -> tuple[int, int | None]:
    """Accept a hex address, a catalogued name, or a recovery symbol.

    The third case came in with Mizuchi, retired since: `{{functionName}}`
    carries the symbol both objects are made to share, which for the 1,179
    functions a disassembler named is a decoration of the label rather than
    the label itself. `_sub_5e3650@4` has to find the same row `sub_5e3650`
    does, or the context emitter refuses every prompt the change enables.

    A C++ symbol needs the same treatment and cannot be undecorated back:
    `?POP2@@YAHPBD0H@Z` is the catalogued `?POP2@@YAHPBDPBDH@Z` with its
    back-references compressed, and `?init@AlphaMenu@@QAEHPAVWin@@@Z` is the
    same name with the struct key the source declares. 221 of the 2,783
    unrecovered rows are in that state. Rather than invert the transforms,
    this reads the symbol map `emit_target_object` writes - the same file
    `mizuchi-integrator.mjs` resolves through, so both directions agree by
    construction. Missing or stale, it just falls through to the name.
    """
    try:
        address = int(text, 16)
    except ValueError:
        # IN THIS ORDER. The exact name and the undecorated one used to be
        # tried together as a set, and whichever row came first in the
        # catalogue won: `__exit` is a real catalogued row AND the decoration
        # of the row named `_exit` (CL decorates source `exit` to `_exit` and
        # source `_exit` to `__exit`), so the symbol for one resolved to the
        # other. An exact name beats a reconstructed one.
        by_name = {row.get("name"): candidate
                   for candidate, row in functions.items()}
        address = by_name.get(text)
        if address is None:
            address = symbol_map().get(text)
        if address is None:
            address = by_name.get(recovery_symbols.undecorate(text))
        if address is None:
            address = recovery_symbols.address_in(text)
        if address is None or address not in functions:
            raise ValueError(
                f"{text} is neither a hex address nor a known name")
    row = functions.get(address)
    size = int(row["size"]) if row and row.get("size") else None
    return address, size


def read_range(pe: pefile.PE, start: int, length: int) -> bytes:
    """Bytes at a virtual address, raising rather than inventing them.

    The shared reader does the section walk. The local copy this replaces was
    UNCLAMPED: it computed a file offset and sliced `length` bytes without
    checking the read stayed inside the section's raw data, so a read starting
    near a section end - or anywhere in the 3.25 MB of .data that has no bytes
    on disk - returned whatever followed in the file. This tool is the project's
    ground truth for arity, so silently decoding .rsrc content as a function
    body is the worst place in the tree for that.

    The raise is kept and now covers the uninitialised case too: read_bytes
    returns b"" both for an address outside every section and for one inside a
    section but past its raw data, and neither is something to disassemble.
    """
    data = read_bytes(pe, start, length)
    if not data:
        raise ValueError(
            f"0x{start:08X} is outside every section, or inside one that has "
            f"no bytes on disk there")
    return data


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
