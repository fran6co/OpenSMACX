#!/usr/bin/env python3
"""Canonical original address -> the bytes OUR compiler produced for it.

The SMT equivalence spike needs both sides of a comparison it can trust: the
original's machine code, which `tools/extract_legacy_leaves.py` already reads
out of the pinned executable, and the RECOVERED body's machine code, which
nothing reads today. This is that second half.

WHY NOT JUST READ THE SOURCE. A proof about C++ is a proof about what the
compiler was supposed to do. The obligation that matters is about what it did:
the bytes that get patched over the original at 0x00402F10 are the bytes in the
release object, so those are the bytes to compare.

HOW AN ADDRESS FINDS ITS BYTES, and both steps can fail loudly rather than
quietly:

  1. `src/dllmain.cpp`'s redirect table maps each canonical address to the
     symbol installed there. That table is authoritative - it is what the
     runtime actually patches - so it is consulted first.
  2. The symbol is located in the release objects by DEMANGLED name. A name
     that appears in two objects is REFUSED, never resolved to whichever was
     scanned first: this tool exists to feed a prover, and a prover fed the
     wrong body proves the wrong thing while looking exactly as convincing.

SECTION-RELATIVE, ALWAYS. `objdump -t` reports each symbol's `(sec N)` and an
offset INSIDE that section. Methods frequently land in their own COMDAT rather
than in `.text`, so reading at a file offset computed against `.text` yields
another function's bytes - a wrong answer, not a missing one. The section is
therefore read from the symbol, never assumed.

THE REDIRECT MAY BE A WRAPPER. Many entries name a `*_redirect` thunk that
adapts the calling convention and calls the recovered method:

    int __fastcall alpha_menu_requested_height_redirect(AlphaMenu *self, void *)
    { return self->requested_height(); }

Both are reported, and which one a caller wants depends on the obligation being
discharged - the wrapper is what is installed at the original address, the
method is what holds the recovered logic. Collapsing them would hide the
distinction, so `kind` says which was found.
"""

from __future__ import annotations

import argparse
import csv
import re
import struct
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DLLMAIN = REPO_ROOT / "src" / "dllmain.cpp"
DEFAULT_OBJECTS = (REPO_ROOT / "build" / "mingw-i686-release" / "CMakeFiles"
                   / "OpenSMACX.dir" / "src")
OBJDUMP = "i686-w64-mingw32-objdump"

# {  0x00402F10,  reinterpret_cast<uintptr_t>(&construct_alphamenu_wave),
REDIRECT_ENTRY = re.compile(
    r"\{\s*(0x[0-9A-Fa-f]{8})\s*,\s*reinterpret_cast<uintptr_t>\(\s*&\s*"
    r"([A-Za-z_][\w:]*)\s*\)", re.S)

# [2623](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x000007c0 __ZN9Alpha...
SYMBOL_LINE = re.compile(
    r"^\[\s*\d+\]\(sec\s+(-?\d+)\)\(fl\s+\S+\)\(ty\s+(\S+)\)\(scl\s+(\d+)\)"
    r"\s+\(nx\s+\d+\)\s+(0x[0-9A-Fa-f]+)\s+(\S+)\s*$")


@dataclass(frozen=True)
class Symbol:
    name: str            # as the object file spells it
    demangled: str       # what a human wrote
    object_path: Path
    section: int         # 1-based, as objdump reports it
    offset: int          # INSIDE the section, not the file


@dataclass
class Body:
    address: int
    symbol: Symbol
    code: bytes
    kind: str            # "wrapper" or "recovered"


def redirect_targets(path: Path = DEFAULT_DLLMAIN) -> dict[int, str]:
    """canonical address -> the symbol the runtime patches over it."""
    text = path.read_text(encoding="utf-8")
    found: dict[int, str] = {}
    for address, symbol in REDIRECT_ENTRY.findall(text):
        found[int(address, 16)] = symbol
    return found


DECORATION = re.compile(r"^(@)?(.*?)(?:@\d+)?$")


def undecorate(name: str) -> str:
    """Undo i686 calling-convention decoration before demangling.

    mingw spells a __stdcall function `__ZN...@8` and a __fastcall one
    `@_ZN...@8` - and the `@` on the fastcall form REPLACES the leading
    underscore rather than joining it. So the underscore has to be put back,
    not merely stripped: this cross-c++filt demangles `__ZN9AlphaMenu...` and
    refuses `_ZN9AlphaMenu...`, because it removes the target's own leading
    underscore itself.

    Getting this backwards left every __fastcall wrapper unresolved - measured
    at 1,136 of 2,048 - which reads as a coverage limit rather than as the
    parsing bug it is.
    """
    found = DECORATION.match(name)
    if not found:
        return name
    at, body = found.groups()
    return ("_" + body) if at else body


def demangle(names: list[str]) -> dict[str, str]:
    """Mangled -> demangled, in one pass; c++filt per name is far too slow."""
    if not names:
        return {}
    stripped = [undecorate(name) for name in names]
    done = subprocess.run([f"{OBJDUMP.rsplit('-', 1)[0]}-c++filt"],
                          input="\n".join(stripped), capture_output=True,
                          text=True, check=False)
    if done.returncode != 0:
        return {}
    lines = done.stdout.splitlines()
    if len(lines) != len(names):
        return {}
    return dict(zip(names, lines))


def object_symbols(directory: Path = DEFAULT_OBJECTS) -> dict[str, list[Symbol]]:
    """Every defined function symbol in the release objects, by demangled name.

    A list per name on purpose. One name in two objects is an ambiguity the
    caller must refuse, and returning the first would make that undetectable.
    """
    table: dict[str, list[Symbol]] = {}
    for obj in sorted(directory.glob("*.obj")):
        done = subprocess.run([OBJDUMP, "-t", str(obj)], capture_output=True,
                              text=True, check=False)
        if done.returncode != 0:
            continue
        raw: list[tuple[int, int, str]] = []
        for line in done.stdout.splitlines():
            found = SYMBOL_LINE.match(line.strip())
            if not found:
                continue
            section, kind, storage, offset, name = found.groups()
            # ty 20 is DT_FUNCTION; scl 2 is external, 3 is static. Both are
            # real definitions; anything in section 0 is undefined.
            if kind != "20" or int(section) < 1:
                continue
            if storage not in ("2", "3"):
                continue
            raw.append((int(section), int(offset, 16), name))
        pretty = demangle([name for _, _, name in raw])
        for section, offset, name in raw:
            symbol = Symbol(name, pretty.get(name, name), obj, section, offset)
            for key in index_keys(symbol):
                table.setdefault(key, []).append(symbol)
    return table


def qualified_name(demangled: str) -> str:
    """`construct_basewin()` -> `construct_basewin`, keeping `A::b`.

    The redirect table names a function the way a human writes it, with no
    parameter list, while a demangled symbol always carries one. Without this
    the exact lookup misses every free function and falls through to a
    substring search, which then matches `construct_basewin_wave()` as well and
    reports an ambiguity that does not exist. That was 47 false ambiguities -
    a coverage hole that looked exactly like the tool being careful.
    """
    depth = 0
    for index in range(len(demangled) - 1, -1, -1):
        char = demangled[index]
        if char == ")":
            depth += 1
        elif char == "(":
            depth -= 1
            if depth == 0:
                return demangled[:index].strip()
    return demangled.strip()


def index_keys(symbol: Symbol) -> set[str]:
    """Every spelling a caller might reasonably use for this symbol."""
    keys = {symbol.name, symbol.demangled, undecorate(symbol.name)}
    bare = qualified_name(symbol.demangled)
    if bare:
        keys.add(bare)
        # A C symbol demangles to itself, so `_construct_basewin` also has to
        # be findable as `construct_basewin`.
        keys.add(bare.lstrip("_"))
    return {key for key in keys if key}


def section_data(obj: Path) -> dict[int, tuple[bytes, int]]:
    """1-based section index -> (raw bytes, size).

    The COFF header is read directly rather than shelled out to, because the
    offsets that matter are section-relative and a second tool's idea of where
    a section starts is one more thing that can disagree.
    """
    blob = obj.read_bytes()
    count, = struct.unpack_from("<H", blob, 2)
    optional, = struct.unpack_from("<H", blob, 16)
    table = 20 + optional
    sections: dict[int, tuple[bytes, int]] = {}
    for index in range(count):
        base = table + index * 40
        size, pointer = struct.unpack_from("<II", blob, base + 16)
        data = blob[pointer:pointer + size] if pointer else b""
        sections[index + 1] = (data, size)
    return sections


# Inter-function padding. 0x90 is GCC's nop and 0xCC is an int3 trap; both are
# emitted only to align the NEXT symbol and are never reached from this one.
PADDING = (0x90, 0xCC)


def trim_padding(code: bytes) -> bytes:
    """Drop alignment bytes the linker put after the body.

    A 3-byte function can occupy 16 bytes of section, and counting the other 13
    as recovered code would overstate every byte figure this feeds. Only
    TRAILING padding goes: a 0x90 in the middle of a body is an instruction
    that executes.
    """
    end = len(code)
    while end and code[end - 1] in PADDING:
        end -= 1
    return code[:end]


def body_of(symbol: Symbol, sections: dict[int, tuple[bytes, int]],
            others: list[Symbol]) -> bytes:
    """The symbol's bytes: from its offset to the next symbol in its section."""
    data, size = sections.get(symbol.section, (b"", 0))
    if not data:
        return b""
    following = [other.offset for other in others
                 if other.section == symbol.section
                 and other.offset > symbol.offset]
    end = min(following) if following else size
    return trim_padding(data[symbol.offset:end])


def extract(addresses, dllmain: Path, objects: Path) -> tuple[list[Body], dict]:
    targets = redirect_targets(dllmain)
    table = object_symbols(objects)
    per_object: dict[Path, list[Symbol]] = {}
    for symbols in table.values():
        for symbol in symbols:
            per_object.setdefault(symbol.object_path, []).append(symbol)
    cache: dict[Path, dict[int, tuple[bytes, int]]] = {}

    bodies: list[Body] = []
    tally = {"no_redirect": 0, "unresolved": 0, "ambiguous": 0, "empty": 0}
    for address in addresses:
        name = targets.get(address)
        if not name:
            tally["no_redirect"] += 1
            continue
        # Exact keys only. A substring fallback was tried and removed: it
        # manufactured 47 ambiguities by matching `construct_basewin_wave()`
        # for `construct_basewin`, so the tool refused bodies it could have
        # resolved while appearing to be careful about them.
        found = table.get(name) or []
        if not found:
            tally["unresolved"] += 1
            continue
        if len({(s.object_path, s.offset, s.section) for s in found}) > 1:
            tally["ambiguous"] += 1
            continue
        symbol = found[0]
        if symbol.object_path not in cache:
            cache[symbol.object_path] = section_data(symbol.object_path)
        code = body_of(symbol, cache[symbol.object_path],
                       per_object.get(symbol.object_path, []))
        if not code:
            tally["empty"] += 1
            continue
        bodies.append(Body(address, symbol, code,
                           "wrapper" if name.endswith("_redirect") else "recovered"))
    return bodies, tally


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("addresses", nargs="*",
                        help="canonical addresses; default is every redirect")
    parser.add_argument("--dllmain", type=Path, default=DEFAULT_DLLMAIN)
    parser.add_argument("--objects", type=Path, default=DEFAULT_OBJECTS)
    parser.add_argument("--functions", type=Path,
                        default=REPO_ROOT / "docs" / "recovery" / "functions.csv")
    args = parser.parse_args(argv)

    if not args.objects.is_dir():
        print(f"error: {args.objects} is missing; build the release preset "
              f"first", file=sys.stderr)
        return 2

    targets = redirect_targets(args.dllmain)
    wanted = ([int(a, 16) for a in args.addresses] if args.addresses
              else sorted(targets))
    bodies, tally = extract(wanted, args.dllmain, args.objects)

    sizes = {}
    if args.functions.is_file():
        with args.functions.open(newline="", encoding="utf-8-sig") as handle:
            sizes = {int(row["address"], 16): int(row["size"] or 0)
                     for row in csv.DictReader(handle)}

    print(f"redirect table: {len(targets)} entries")
    print(f"asked for {len(wanted)}, extracted {len(bodies)} body(ies), "
          f"{sum(len(b.code) for b in bodies)} compiled bytes")
    for key, count in tally.items():
        if count:
            print(f"  {count:5} {key.replace('_', ' ')}")
    kinds = {"wrapper": 0, "recovered": 0}
    for body in bodies:
        kinds[body.kind] += 1
    print(f"  {kinds['recovered']:5} resolved to a recovered body directly")
    print(f"  {kinds['wrapper']:5} resolved to a *_redirect wrapper")
    if args.addresses:
        for body in bodies:
            original = sizes.get(body.address)
            print(f"\n{body.address:#010x}  {body.symbol.demangled}")
            print(f"  {body.symbol.object_path.name} sec {body.symbol.section} "
                  f"+{body.symbol.offset:#x}, {len(body.code)} compiled bytes"
                  + (f", original is {original} B" if original else ""))
            print(f"  {body.code[:32].hex(' ')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
