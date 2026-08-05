#!/usr/bin/env python3
"""Emit COFF target objects for Mizuchi from the pinned executable.

Mizuchi's objdiff stage compares two relocatable objects, but this project
has no original objects: only the linked image. So for each catalogued
function this synthesises the object the original compiler would have
produced, close enough for objdiff to pair instructions:

  * `.text` holds the function's bytes, read from the hash-pinned
    executable over the catalogued body ranges.
  * The function symbol carries the name the RECOVERED SOURCE will make CL
    emit, which is the catalogued name for the 4,821 `?`-mangled functions
    and a computed C decoration for the 1,179 that were only ever labelled
    by a disassembler. `tools/recovery_symbols.py` decides, once, for both
    sides: a target object holding `sub_5e3650` pairs with nothing, because
    no MSVC declaration emits an undecorated name.
  * Every `call`/`jmp` whose target is catalogued becomes an
    IMAGE_REL_I386_REL32 relocation against an undefined external symbol
    with the target's mangled name, and the rel32 field is zeroed - the
    same encoding VC6 emits for a call to an external symbol. objdiff
    then displays both sides as `call <name>` and pairs them by symbol.
  * Absolute references (the `mov eax, [0x0093CD90]` family) are left
    unrelocated on purpose: a recovered unit spells them as fixed-address
    constants, so VC6 bakes the identical immediate with no relocation,
    and both sides display the same raw address.

Calls to uncatalogued targets keep their original bytes; they show as raw
branch destinations on this side and as symbols on the recovered side, so
a diff there is expected noise until the target gets catalogued.

The output feeds Mizuchi's per-prompt `targetObjectPath`, and the optional
`--symbol-map` JSON (symbol -> object path) feeds its `symbolMapPath`. That
file is also how `mizuchi-integrator.mjs` gets from a symbol back to an
address, since each object is named for the address it holds.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

import recovery_symbols  # noqa: E402
from disasm import DEFAULT_EXE, load_functions, read_range  # noqa: E402
from emit_translation_unit import (Signature, Unsettled, load_derived,  # noqa: E402
                                   unsettled_identifier)
from generator_support import parse_body_ranges  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT = REPO_ROOT / "build" / "target-objects"

IMAGE_REL_I386_REL32 = 0x14
SYM_FUNCTION = 0x20
SYM_CLASS_EXTERNAL = 2


def read_spans(pe: pefile.PE, body_ranges: str) -> bytes:
    """Concatenate the catalogued body spans of one function."""
    out = bytearray()
    for lo, hi in parse_body_ranges(body_ranges):
        out += read_range(pe, lo, hi - lo)
    return bytes(out)


class SymbolResolver:
    """The symbol each catalogued function will carry, resolved once.

    Reading a prototype out of a `ret` disassembles the body, and a whole-
    catalogue run asks for the same popular callee hundreds of times, so the
    answers are memoised by address.

    Without a resolver this falls back to the catalogued name, which is what
    the unit tests and any caller with no executable to hand get.
    """

    def __init__(self, derived: dict, pe=None):
        self._derived, self._pe, self._cache = derived, pe, {}

    def of(self, address: int, row: dict[str, str]) -> str:
        if address not in self._cache:
            self._cache[address] = self._resolve(address, row)
        return self._cache[address]

    def _resolve(self, address: int, row: dict[str, str]) -> str:
        name = (row.get("name") or "").strip()
        try:
            # The Signature decides for EVERY row, `?`-mangled included. It
            # used to short-circuit here and return the catalogued name, which
            # is what kept the source side's back-reference compression and
            # its `fn_<address>` substitution from ever reaching this side.
            return Signature(row, self._derived, self._pe).symbol
        except Unsettled:
            if name.startswith("?"):
                # No signature to compute from, so normalise what there is.
                return recovery_symbols.compress_backrefs(
                    recovery_symbols.empty_destructor_arguments(name))
            # The same shape `emit_translation_unit.declare_callee` falls back
            # to when a signature will not settle: `extern "C" int name();`,
            # which is __cdecl and so decorates without an argument count.
            return recovery_symbols.decorate(unsettled_identifier(row),
                                             recovery_symbols.CDECL)


def call_relocations(text: bytearray, address: int,
                     functions: dict[int, dict[str, str]],
                     symbols: SymbolResolver | None = None):
    """REL32 relocations for catalogued call/jmp targets.

    Zeroes each rel32 field in place so objdiff's implicit addend reads 0,
    matching VC6's encoding of a fresh external call.
    """
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    relocs = []
    externals = set()
    for insn in engine.disasm(bytes(text), address):
        if insn.mnemonic not in ("call", "jmp") or not insn.op_str.startswith("0x"):
            continue
        target = int(insn.op_str, 16)
        if target == address:
            continue
        row = functions.get(target)
        if not row or not row.get("name"):
            continue
        name = symbols.of(target, row) if symbols else row["name"]
        off = insn.address - address + 1
        relocs.append((off, name, IMAGE_REL_I386_REL32))
        externals.add(name)
        text[off:off + 4] = b"\x00\x00\x00\x00"
    return relocs, sorted(externals)


def build_coff(text: bytes, fn_name: str, relocs, externals) -> bytes:
    """A minimal i386 COFF object: one .text section, no line numbers."""
    strings = bytearray(b"\x00\x00\x00\x00")

    def sym_name(name: str) -> bytes:
        nonlocal strings
        if len(name) <= 8:
            return name.encode().ljust(8, b"\x00")
        off = len(strings)
        strings += name.encode() + b"\x00"
        return struct.pack("<II", 0, off)

    sym_index = {fn_name: 0}
    for i, name in enumerate(externals, start=1):
        sym_index[name] = i

    symtab = bytearray()
    symtab += sym_name(fn_name) + struct.pack("<IhHBB", 0, 1, SYM_FUNCTION,
                                              SYM_CLASS_EXTERNAL, 0)
    for name in externals:
        symtab += sym_name(name) + struct.pack("<IhHBB", 0, 0, SYM_FUNCTION,
                                               SYM_CLASS_EXTERNAL, 0)
    strings[0:4] = struct.pack("<I", len(strings))

    reloc_blob = bytearray()
    for off, name, typ in sorted(relocs):
        reloc_blob += struct.pack("<IIH", off, sym_index[name], typ)

    text_ptr = 20 + 40
    reloc_ptr = text_ptr + len(text)
    symtab_ptr = reloc_ptr + len(reloc_blob)

    header = struct.pack("<HHIIIHH", 0x014C, 1, 0, symtab_ptr,
                         1 + len(externals), 0, 0)
    # CODE | MEM_EXECUTE | MEM_READ | ALIGN_16BYTES
    section = struct.pack("<8sIIIIIIHHI", b".text", 0, 0, len(text),
                          text_ptr, reloc_ptr, 0, len(relocs), 0, 0x60500020)
    return bytes(header + section + text + reloc_blob + symtab + strings)


def emit_one(pe: pefile.PE, address: int, row: dict[str, str],
             functions: dict[int, dict[str, str]], out_dir: Path,
             symbols: SymbolResolver | None = None) -> tuple[Path, str]:
    name = symbols.of(address, row) if symbols else row["name"]
    text = bytearray(read_spans(pe, row["body_ranges"]))
    relocs, externals = call_relocations(text, address, functions, symbols)
    obj_path = out_dir / f"{address:08x}.obj"
    obj_path.write_bytes(build_coff(bytes(text), name, relocs, externals))
    return obj_path, name


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Emit COFF target objects for Mizuchi from the pinned executable")
    parser.add_argument("subjects", nargs="*",
                        help="hex addresses or mangled names (default: whole catalog)")
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT,
                        help=f"output directory (default: {DEFAULT_OUT})")
    parser.add_argument("--symbol-map", type=Path, default=None,
                        help="also write a Mizuchi symbolMap JSON here")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE,
                        help="executable to read (default: the pinned original)")
    args = parser.parse_args()

    functions = load_functions()
    if not functions:
        print("no catalog: docs/recovery/functions.csv is missing", file=sys.stderr)
        return 1

    if args.subjects:
        addresses = []
        for text in args.subjects:
            try:
                address = int(text, 16)
            except ValueError:
                for candidate, row in functions.items():
                    if row.get("name") == text:
                        address = candidate
                        break
                else:
                    print(f"{text} is neither a hex address nor a known name",
                          file=sys.stderr)
                    return 1
            addresses.append(address)
    else:
        addresses = sorted(functions)

    pe = pefile.PE(str(args.exe), fast_load=True)
    args.out.mkdir(parents=True, exist_ok=True)
    symbols = SymbolResolver(load_derived(), pe)

    symbol_map = {}
    disagreements = []
    collisions = []
    emitted = skipped = 0
    for address in addresses:
        row = functions.get(address)
        if not row or not row.get("name") or not row.get("body_ranges"):
            skipped += 1
            continue
        try:
            obj_path, symbol = emit_one(pe, address, row, functions, args.out,
                                        symbols)
        except ValueError as exc:
            print(f"skip 0x{address:08X} {row.get('name')}: {exc}", file=sys.stderr)
            skipped += 1
            continue
        try:
            mapped = str(obj_path.relative_to(REPO_ROOT))
        except ValueError:
            mapped = str(obj_path)
        if symbol in symbol_map:
            # Two catalogued rows claiming one symbol. A linked image cannot
            # contain that, so one of the two names is wrong in the catalogue -
            # and the symbol map is keyed by symbol, so the second row silently
            # replaces the first and the integrator resolves that symbol to
            # whichever address won. Saying so beats a count that quietly
            # disagrees with the object count.
            collisions.append(f"0x{address:08X} {row['name']} and "
                              f"{symbol_map[symbol]} both claim {symbol}")
        symbol_map[symbol] = mapped
        note = recovery_symbols.disagreement(row["name"], symbol)
        if note:
            disagreements.append(f"0x{address:08X} {note}")
        emitted += 1

    if args.symbol_map:
        args.symbol_map.parent.mkdir(parents=True, exist_ok=True)
        args.symbol_map.write_text(json.dumps(symbol_map, indent=2, sort_keys=True) + "\n")
        print(f"wrote {args.symbol_map} ({len(symbol_map)} entries)")

    print(f"emitted {emitted} object(s) into {args.out}, skipped {skipped}")
    for line in collisions:
        print(f"    COLLISION: {line}")
    if disagreements:
        # Both objects still carry the SAME symbol, so nothing fails to pair.
        # What each of these says is that the recovered signature contradicts
        # the decoration the binary itself carries - a wrong arity or a wrong
        # convention - which is worth reading rather than swallowing.
        print(f"{len(disagreements)} recovered signature(s) contradict the "
              f"catalogued decoration:")
        for line in disagreements[:20]:
            print(f"    {line}")
        if len(disagreements) > 20:
            print(f"    ... and {len(disagreements) - 20} more")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
