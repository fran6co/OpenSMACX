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


def oracle_safety(header: Path) -> dict[str, bool]:
    """{DLL_Name: may the differential oracle call this twice?}

    "Implemented" and "safe to compare" are DIFFERENT questions and conflating
    them produces false failures, which are worse than skips because they look
    like lowering bugs. The differential oracle calls the original and the
    lifted side separately; an import whose second call cannot agree with its
    first - GetTickCount, HeapAlloc, TlsAlloc, ReadFile - makes the two sides
    differ for a reason that has nothing to do with the lift. Such an import
    must keep blocking its callers even though it works perfectly.

    So every implemented shim has to answer `safe` or `unsafe` next to its
    body, and this refuses to run if one does not. That is the same discipline
    as opensmacx_host()'s mandatory size: a new shim cannot be written without
    the author stating whether calling it twice is meaningful.
    """
    pattern = re.compile(
        r"^#define\s+OPENSMACX_ORACLE_([A-Za-z0-9_]+)\s+(safe|unsafe)\b", re.M)
    return {key: verdict == "safe"
            for key, verdict in pattern.findall(
                header.read_text(encoding="utf-8"))}


def collect(exe: Path, nm: Path, libs: Path, header: Path) -> list[dict]:
    pe = pefile.PE(str(exe), fast_load=True)
    pe.parse_data_directories(
        directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
    available = implemented_imports(header)
    safety = oracle_safety(header)
    unstated = sorted(set(available) - set(safety))
    if unstated:
        raise SystemExit(
            "these shims do not say whether the differential oracle may call "
            "them twice; add `#define OPENSMACX_ORACLE_<key> safe|unsafe` "
            "beside each body:\n  " + "\n  ".join(unstated))
    orphaned = sorted(set(safety) - set(available))
    if orphaned:
        raise SystemExit(
            "these OPENSMACX_ORACLE_ lines name no implemented shim:\n  "
            + "\n  ".join(orphaned))
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
                # Kept apart from the display name so the run-time binder can
                # ask for the export without re-parsing "DLL!Name".
                "export_name": (entry.name.decode("ascii")
                                if entry.name else None),
                "ordinal": entry.ordinal or 0,
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
                # An unimplemented import can never be oracle-safe: its shim
                # traps, so the lifted side stops where the original ran on.
                "oracle_safe": (key in available and arg_bytes is not None
                                and safety.get(key, False)),
            })
    entries.sort(key=lambda item: item["iat"])
    for index, entry in enumerate(entries):
        entry["index"] = index
        entry["synthetic"] = IMPORT_BASE + index * IMPORT_STRIDE
    return entries


def render(entries: list[dict], exe: Path) -> str:
    parts = [f'''#include "lifted_imports.h"

#include <cstdint>
#include <cstring>

#include <windows.h>

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

    def row(entry: dict) -> str:
        export = ("nullptr" if entry["export_name"] is None
                  else '"%s"' % entry["export_name"])
        return (f'    {{{entry["iat"]:#010x}U,'
                f' &opensmacx_import_{entry["index"]:03d},'
                f' "{entry["dll"]}!{entry["name"]}",'
                f' {entry["argument_bytes"] or 0}U,'
                f' "{entry["dll"]}", {export}, {entry["ordinal"]}U,'
                f' {"true" if entry["oracle_safe"] else "false"}}},')

    rows = "\n".join(row(entry) for entry in entries)

    parts.append(f'''struct Slot {{
    uint32_t iat;
    OpensmacxLiftedFunction shim;
    const char *name;
    uint32_t argument_bytes;
    // The DLL and export are kept SEPARATELY from the display name because
    // opensmacx_bind_real_imports() has to pass them to GetProcAddress, and
    // splitting "DLL!Name" back apart at run time would put a parser in the
    // one place that must not have one. `export_name` is null for an ordinal
    // import, and then `ordinal` is the thing to ask for.
    const char *dll;
    const char *export_name;
    uint32_t ordinal;
    bool oracle_safe;
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

bool opensmacx_import_oracle_safe(uint32_t address) {{
    const Slot *const slot = slot_for(address);
    return slot != nullptr && slot->oracle_safe;
}}

// The span of the image the IAT occupies, as a half-open [low, high).
//
// The differential oracle compares the two sides' images byte for byte, and
// once each side's IAT is bound to something appropriate to that side - real
// Windows addresses on the original's, synthetic ones on the lifted's - the
// two DELIBERATELY differ there. Without this the very first case would report
// a memory difference at the first IAT slot, for every function in the image,
// and the report would be all noise. Derived from the same import directory as
// everything else, so it cannot drift from the slots actually written.
uint32_t opensmacx_import_iat_low() {{ return {min(one["iat"] for one in entries):#010x}U; }}
uint32_t opensmacx_import_iat_high() {{ return {max(one["iat"] for one in entries) + 4:#010x}U; }}

// Fill an image's IAT with the addresses of the REAL Windows functions.
//
// This is the other half of the boundary, and it exists for the differential
// oracle. opensmacx_bind_imports() prepares the LIFTED side, whose `call [iat]`
// goes through opensmacx_dispatch to a shim. The ORIGINAL side is real machine
// code executed by the CPU, so its `call [iat]` needs a real code address in
// the slot - the loader normally supplies that, and the oracle, which maps
// sections by hand, does not. Until it is called, the original's IAT holds the
// raw file bytes, which are RVAs into the hint/name table: the original then
// calls into the middle of a string table and faults. That is why a function
// reaching an import is not merely uncomparable but uncallable, and it is the
// reason the oracle skips 2,983 of them.
//
// ONLY THE ORACLE-SAFE SLOTS ARE BOUND, and that restriction is not
// conservatism, it is the correct set. A function the oracle agrees to run can
// only reach oracle-safe imports - that is what the planner checks - so every
// other slot is unreachable in a legitimate run, and leaving it pointing at
// the hint/name table means a function that should have been SKIPPED faults
// loudly instead of quietly doing something to the machine.
//
// Binding everything was measured to be much worse than useless: the full
// table names DDRAW, DSOUND and DPLAYX, and merely LoadLibrary-ing those under
// Wine starts the display and audio drivers. A single function that had
// previously been decided in 0.1 s did not finish in 120 s. Nothing in the
// oracle wants DirectDraw initialised; the imports it needs - the kernel32,
// user32 and gdi32 ones - are already in the process because the harness
// itself links against them, so GetModuleHandleA finds them and no library is
// loaded at all.
//
// `image` is the host address of the guest's byte 0, i.e. of guest
// OpensmacxImageBase. Returns the number of slots filled; a slot whose DLL or
// export cannot be resolved is left alone and not counted, so the caller can
// tell "bound everything" from "bound most of it" instead of discovering the
// difference as a fault.
uint32_t opensmacx_bind_real_imports(unsigned char *image) {{
    uint32_t bound = 0;
    for (uint32_t index = 0; index < SlotCount; ++index) {{
        const Slot &slot = Slots[index];
        if (!slot.oracle_safe) {{
            continue;
        }}
        // GetModuleHandleA only. A DLL that is not already loaded cannot hold
        // an import the oracle is allowed to reach, and loading it to find out
        // is the expensive mistake described above.
        HMODULE module = GetModuleHandleA(slot.dll);
        if (module == nullptr) {{
            continue;
        }}
        // An ordinal import has no name to ask for, so it is asked for by
        // ordinal - MAKEINTRESOURCE-style, the low word in the pointer.
        const char *const request =
            slot.export_name != nullptr
                ? slot.export_name
                : reinterpret_cast<const char *>(
                      static_cast<uintptr_t>(slot.ordinal));
        FARPROC target = GetProcAddress(module, request);
        if (target == nullptr) {{
            continue;
        }}
        const uintptr_t value = reinterpret_cast<uintptr_t>(target);
        // This target is 32-bit, so a code address always fits; the check is
        // here so that building it anywhere else fails loudly rather than
        // truncating a pointer into a wild jump.
        if (value > 0xFFFFFFFFULL) {{
            continue;
        }}
        const uint32_t address = static_cast<uint32_t>(value);
        std::memcpy(image + (slot.iat - OpensmacxImageBase), &address, 4);
        ++bound;
    }}
    return bound;
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
        "oracle_safe": sum(1 for one in entries if one["oracle_safe"]),
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
             "implemented": one["implemented"],
             "oracle_safe": one["oracle_safe"]}
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
    # Not the same number as `implemented`, and the gap is the point: an
    # import can work perfectly and still be uncomparable, because the
    # differential oracle calls it twice and the second call has to agree.
    print(f"oracle-safe          {manifest['oracle_safe']}"
          f"  (of {manifest['implemented']} implemented; the rest work but"
          f" cannot agree with themselves)")
    if unresolved:
        print("\nargument size UNDETERMINED (these trap):")
        for line in unresolved:
            print(f"  {line}")
    print(f"\nwrote {args.out / 'lifted_imports.cpp'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
