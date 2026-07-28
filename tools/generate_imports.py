#!/usr/bin/env python3
"""Derive the import shim table from the PE import directory.

The lift's 5,655 internal functions need no calling convention: each one is
`void f(State &)` and the argument passing is whatever the original
instructions did to the guest stack. The 221 imports are the only place where
a real convention has to be right, and the failure mode is the worst one in
the project - a stdcall callee that pops the wrong number of bytes corrupts
its CALLER's stack, so the crash lands in an unrelated function some
arbitrary time later.

Two facts are needed per import and neither can be invented:

  WHERE  - the IAT slot address, so something dispatchable can be put in it.
           pefile gives this exactly.
  HOW MANY BYTES TO POP - the argument size. pefile does NOT give this: the
           import directory carries a name and an ordinal and nothing about
           the signature. It is recovered instead from the TOOLCHAIN's own
           import libraries, where every stdcall export is a decorated symbol
           `_Name@N` and N is precisely the byte count. That is a derivation
           from an authority (the same libraries the host links against),
           not a guess, and it stays right if the import table changes.

That covers 217 of 221. The four that remain are named in the manifest and
trap, because a plausible-looking number is worse than no number here. Both
figures are printed by this script on every run; do not take them from here.

Hand-writing 221 shims would also be the wrong shape for a second reason: the
argument count would then live next to each body, and a typo in it would be
silent. Here the count exists TWICE on purpose - once as the derived @N and
once as the prototype's own OPENSMACX_ARGS_* in tools/lifted_imports.h - and
the generated shim static_asserts they agree. Disagreement is a build error
naming the import. Nothing else in this layer can catch a wrong cleanup
before it corrupts something.

Output lands in build/lifted/, which is git-ignored, and carries only the
SHAPE of the import table - names, slots, argument sizes - never any byte of
the original image.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_OUT = REPO_ROOT / "build" / "lifted"
DEFAULT_NM = Path("/opt/homebrew/bin/i686-w64-mingw32-nm")
DEFAULT_LIBS = Path(
    "/opt/homebrew/opt/mingw-w64/toolchain-i686/i686-w64-mingw32/lib")

# Must match tools/lifted_imports.h. Duplicated here rather than parsed out of
# the header because the generator has to emit the addresses as literals and a
# silent divergence would put the IAT outside the range the dispatcher tests;
# the generated file asserts the two agree, so the duplication cannot rot.
IMPORT_BASE = 0x0F000000
IMPORT_STRIDE = 16

# `_Name@N` is stdcall with N bytes of argument. `_Name` with no suffix is
# cdecl: the CALLER cleans, so the shim pops only the return address. Both
# forms appear; the second is rare in these DLLs but real (CRT-style entry
# points exported from system libraries).
DECORATED = re.compile(r"^_(?P<name>[A-Za-z_][A-Za-z0-9_]*)@(?P<bytes>\d+)$")
UNDECORATED = re.compile(r"^_(?P<name>[A-Za-z_][A-Za-z0-9_]*)$")


def sanitize(text: str) -> str:
    """A C identifier fragment: uppercase, .dll dropped, non-alnum to _."""
    stem = re.sub(r"\.dll$", "", text, flags=re.IGNORECASE)
    return re.sub(r"[^A-Za-z0-9]", "_", stem).upper()


def library_for(libs: Path, dll: str) -> Path:
    return libs / f"lib{sanitize(dll).lower()}.a"


def argument_bytes(nm: Path, library: Path) -> dict[str, int | None]:
    """{export name: stdcall argument bytes, or None if cdecl}.

    Both the `T` thunks and the `I` __imp_ pointers are read. Several
    kernel32 exports - the whole Interlocked family - have no thunk at all
    because mingw supplies them as compiler intrinsics, and appear ONLY as
    `__imp__InterlockedIncrement@4`. Reading just the thunks loses them and
    they are exactly the functions a lift most needs.
    """
    if not library.exists():
        return {}
    output = subprocess.run(
        [str(nm), "--defined-only", str(library)],
        capture_output=True, text=True, check=False).stdout
    found: dict[str, int | None] = {}
    for line in output.splitlines():
        parts = line.split()
        if len(parts) != 3 or parts[1] not in ("T", "I"):
            continue
        symbol = parts[2]
        if symbol.startswith("__imp_"):
            symbol = symbol[len("__imp_"):]
        match = DECORATED.match(symbol)
        if match:
            found.setdefault(match.group("name"), int(match.group("bytes")))
            continue
        match = UNDECORATED.match(symbol)
        if match:
            found.setdefault(match.group("name"), None)
    return found


def implemented_imports(header: Path) -> dict[str, int]:
    """{DLL_Name: declared argument count} from the hand-written header.

    Read out of `#define OPENSMACX_ARGS_<key> <count>` rather than kept as a
    list here, so adding an implementation is a one-file change and this
    generator never has to learn about it.
    """
    pattern = re.compile(
        r"^#define\s+OPENSMACX_ARGS_([A-Za-z0-9_]+)\s+(\d+)\s*$", re.M)
    text = header.read_text(encoding="utf-8")
    return {key: int(count) for key, count in pattern.findall(text)}


def collect(exe: Path, nm: Path, libs: Path, header: Path) -> list[dict]:
    pe = pefile.PE(str(exe), fast_load=True)
    pe.parse_data_directories(
        directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
    available = implemented_imports(header)
    entries: list[dict] = []
    caches: dict[str, dict[str, int | None]] = {}
    for descriptor in pe.DIRECTORY_ENTRY_IMPORT:
        dll = descriptor.dll.decode("ascii")
        if dll not in caches:
            caches[dll] = argument_bytes(nm, library_for(libs, dll))
        table = caches[dll]
        for entry in descriptor.imports:
            name = (entry.name.decode("ascii") if entry.name
                    else f"ORD_{entry.ordinal}")
            key = f"{sanitize(dll)}_{name}"
            # An ordinal import carries no name, so the import library - which
            # is indexed by name - cannot be consulted at all. mingw's members
            # are full object files rather than short import records, so they
            # do not carry the ordinal either. Undeterminable, and said so.
            if entry.name is None:
                convention, arg_bytes = "unknown", None
                reason = "imported by ordinal; no name to resolve"
            elif name not in table:
                convention, arg_bytes = "unknown", None
                reason = "not exported by the toolchain's import library"
            elif table[name] is None:
                convention, arg_bytes, reason = "cdecl", 0, ""
            else:
                convention, arg_bytes, reason = "stdcall", table[name], ""
            entries.append({
                "dll": dll,
                "name": name,
                "key": key,
                "iat": entry.address,
                "convention": convention,
                "argument_bytes": arg_bytes,
                "unresolved_reason": reason,
                "declared_arguments": available.get(key),
                # An import is implemented only if BOTH halves are present:
                # a body to call and a byte count to clean up with. A body
                # with no derived count would have to guess the cleanup, and
                # guessing is the thing this whole file exists to avoid.
                "implemented": key in available and arg_bytes is not None,
            })
    entries.sort(key=lambda item: item["iat"])
    for index, entry in enumerate(entries):
        entry["index"] = index
        entry["synthetic"] = IMPORT_BASE + index * IMPORT_STRIDE
    return entries


def render(entries: list[dict], exe: Path) -> str:
    parts = [f'''#include "lifted_imports.h"

#include <cstdint>

// Generated by tools/generate_imports.py from the import directory of the
// user's own executable. Edit that tool, not this file.
//
// {len(entries)} imports. Only names, IAT slot addresses and argument sizes
// appear here - no byte of the original image.

static_assert(OpensmacxImportBase == {IMPORT_BASE:#010x}U,
              "the generator and lifted_imports.h disagree on the range");
static_assert(OpensmacxImportStride == {IMPORT_STRIDE}U,
              "the generator and lifted_imports.h disagree on the stride");
static_assert(OpensmacxImportBase + {len(entries)}U * OpensmacxImportStride
                  <= OpensmacxImportLimit,
              "the reserved import range is too small for this image");

namespace {{

''']

    for entry in entries:
        symbol = f"opensmacx_import_{entry['index']:03d}"
        label = f"{entry['dll']}!{entry['name']}"
        if entry["implemented"]:
            parts.append(f'''// {label}  IAT {entry['iat']:#010x}  \
{entry['convention']} @{entry['argument_bytes']}
void {symbol}(OpensmacxStaticRecompileState &s) {{
    // The prototype in lifted_imports.h and the @N in the toolchain's import
    // library are independent answers to the same question. If they differ
    // the cleanup below is wrong and the CALLER is the one that breaks, so
    // the disagreement has to stop the build.
    static_assert(OPENSMACX_ARGS_{entry['key']} * 4U
                      == {entry['argument_bytes']}U,
                  "{label}: the shim's argument count and the import "
                  "library's @N disagree");
    const uint32_t result = opensmacx_shim_{entry['key']}(s);
    s.eax = result;
    // stdcall: the callee pops the return address AND the arguments. For a
    // cdecl import argument_bytes is 0 and only the return address goes.
    s.esp += 4U + {entry['argument_bytes']}U;
}}

''')
        else:
            parts.append(f'''// {label}  IAT {entry['iat']:#010x}  \
{entry['convention']}\
{"" if entry['argument_bytes'] is None else " @%d" % entry['argument_bytes']}
void {symbol}(OpensmacxStaticRecompileState &s) {{
    (void)s;
    opensmacx_trap({entry['synthetic']:#010x}U,
                   "unimplemented import {label}");
}}

''')

    rows = "\n".join(
        f'    {{{entry["iat"]:#010x}U, &opensmacx_import_{entry["index"]:03d},'
        f' "{entry["dll"]}!{entry["name"]}",'
        f' {entry["argument_bytes"] if entry["argument_bytes"] is not None else 0}U}},'
        for entry in entries)

    parts.append(f'''struct Slot {{
    uint32_t iat;
    OpensmacxLiftedFunction shim;
    const char *name;
    uint32_t argument_bytes;
}};

// Dense and in synthetic-address order, so the dispatcher indexes rather than
// searches: the synthetic address IS the index, scaled.
const Slot Slots[] = {{
{rows}
}};

constexpr uint32_t SlotCount = sizeof(Slots) / sizeof(Slots[0]);

const Slot *slot_for(uint32_t address) {{
    if (!opensmacx_is_import(address)) {{
        return nullptr;
    }}
    const uint32_t offset = address - OpensmacxImportBase;
    if (offset % OpensmacxImportStride != 0U) {{
        return nullptr;
    }}
    const uint32_t index = offset / OpensmacxImportStride;
    return index < SlotCount ? &Slots[index] : nullptr;
}}

}}  // namespace

OpensmacxLiftedFunction opensmacx_import_dispatch(uint32_t address) {{
    const Slot *const slot = slot_for(address);
    if (slot == nullptr) {{
        // Inside the reserved range but not a real import: the IAT was
        // overwritten, or a synthetic address was computed rather than
        // loaded. Either way it is a real bug and not a missing shim.
        opensmacx_trap(address, "synthetic import address with no import");
    }}
    return slot->shim;
}}

// Writes each import's synthetic address into its IAT slot. Must run before
// any lifted code, because until it does every slot holds zero and the first
// `call [iat]` dispatches to address 0.
void opensmacx_bind_imports() {{
    for (uint32_t index = 0; index < SlotCount; ++index) {{
        opensmacx_store32(Slots[index].iat,
                          OpensmacxImportBase + index * OpensmacxImportStride);
    }}
}}

const char *opensmacx_import_name(uint32_t address) {{
    const Slot *const slot = slot_for(address);
    return slot == nullptr ? nullptr : slot->name;
}}

uint32_t opensmacx_import_arg_bytes(uint32_t address) {{
    const Slot *const slot = slot_for(address);
    return slot == nullptr ? 0U : slot->argument_bytes;
}}

// The IAT slot this import was bound INTO. Exposed because without it the
// binding direction is structurally untestable: a probe can only find a slot
// by searching the image for a synthetic value, which finds whichever slot
// holds it and therefore cannot tell a correct binding from a reversed one.
// With it, a probe can assert (slot -> name) pairs read straight out of the
// PE import directory.
uint32_t opensmacx_import_iat(uint32_t address) {{
    const Slot *const slot = slot_for(address);
    return slot == nullptr ? 0U : slot->iat;
}}

uint32_t opensmacx_import_count() {{ return SlotCount; }}

uint32_t opensmacx_import_address(uint32_t index) {{
    return index < SlotCount
        ? OpensmacxImportBase + index * OpensmacxImportStride : 0U;
}}
''')
    return "".join(parts)


PROBE_SCRIPT = """\
#!/bin/sh
# Build and run the ABI probe. Generated by tools/generate_imports.py.
#
# -lgdi32 and -lwinmm are both required and neither is in the default specs;
# without them the link fails naming _imp__GdiFlush@0 and _imp__timeGetTime@0.
# The same two have to be added to build/lifted/build.sh for the whole image.
set -e
CXX="${CXX:-%(cxx)s}"
cd "$(dirname "$0")"
"$CXX" -std=c++17 -O2 -I%(tools)s -I. -c lifted_imports.cpp -o imports.o
"$CXX" -std=c++17 -O2 -I%(tools)s -I. -c %(probe)s -o probe.o
"$CXX" -std=c++17 -O2 -static -o imports_probe.exe probe.o imports.o \\
    -lgdi32 -lwinmm
echo "built imports_probe.exe"
"""


def write_probe_script(out: Path, tools: Path, cxx: str) -> None:
    path = out / "build_imports_probe.sh"
    path.write_text(
        PROBE_SCRIPT % {"tools": tools, "cxx": cxx,
                        "probe": tools / "lifted_imports_probe.cpp"},
        encoding="utf-8")
    path.chmod(0o755)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--nm", type=Path, default=DEFAULT_NM)
    parser.add_argument("--libs", type=Path, default=DEFAULT_LIBS)
    parser.add_argument(
        "--header", type=Path,
        default=Path(__file__).resolve().parent / "lifted_imports.h")
    args = parser.parse_args()

    entries = collect(args.exe, args.nm, args.libs, args.header)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "lifted_imports.cpp").write_text(
        render(entries, args.exe), encoding="utf-8")
    write_probe_script(args.out, Path(__file__).resolve().parent,
                       str(args.nm).replace("-nm", "-g++"))

    per_dll: dict[str, dict] = {}
    for entry in entries:
        bucket = per_dll.setdefault(
            entry["dll"], {"imports": 0, "resolved": 0, "implemented": 0})
        bucket["imports"] += 1
        bucket["resolved"] += entry["argument_bytes"] is not None
        bucket["implemented"] += entry["implemented"]

    unresolved = [f"{one['dll']}!{one['name']}: {one['unresolved_reason']}"
                  for one in entries if one["argument_bytes"] is None]
    manifest = {
        "imports": len(entries),
        "dlls": len(per_dll),
        "per_dll": per_dll,
        "resolved_argument_bytes": sum(
            1 for one in entries if one["argument_bytes"] is not None),
        "implemented": sum(1 for one in entries if one["implemented"]),
        "trapping": sum(1 for one in entries if not one["implemented"]),
        "unresolved": unresolved,
        "conventions": dict(Counter(one["convention"] for one in entries)),
        "import_base": f"{IMPORT_BASE:#010x}",
        "import_stride": IMPORT_STRIDE,
        "slots": [
            {"index": one["index"], "dll": one["dll"], "name": one["name"],
             "iat": f"{one['iat']:#010x}",
             "synthetic": f"{one['synthetic']:#010x}",
             "argument_bytes": one["argument_bytes"],
             "convention": one["convention"],
             "implemented": one["implemented"]}
            for one in entries],
    }
    (args.out / "imports_manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"imports              {len(entries)} across {len(per_dll)} DLLs")
    for dll in sorted(per_dll, key=lambda name: -per_dll[name]["imports"]):
        counts = per_dll[dll]
        print(f"  {dll:<16} {counts['imports']:>3} imports"
              f"  {counts['resolved']:>3} sized"
              f"  {counts['implemented']:>3} implemented")
    print(f"argument sizes known {manifest['resolved_argument_bytes']}"
          f" of {len(entries)}")
    print(f"implemented          {manifest['implemented']}")
    print(f"trapping             {manifest['trapping']}")
    if unresolved:
        print("\nargument size UNDETERMINED (these trap):")
        for line in unresolved:
            print(f"  {line}")
    print(f"\nwrote {args.out / 'lifted_imports.cpp'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
