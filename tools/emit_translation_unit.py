#!/usr/bin/env python3
"""Emit a compilable standalone unit for one catalogued function.

An agent asked to byte-match a function should spend its turns on the BODY, not
on reconstructing the declarations the body needs. This emits everything else:
the signature, the class the method hangs off, declarations for every callee,
and a binding for every fixed global the original touches.

THE PROPERTY THAT MAKES THIS TRACTABLE: the oracle compiles with `/c` and never
links. So every callee, every global, every base class and every CRT helper can
be a bare declaration with no definition anywhere. Unresolved externals are not
an error because nothing ever resolves them.

THREE DECISIONS THAT ARE BYTE-VISIBLE, and are made here rather than left to
the agent, because each was measured and each has exactly one right answer:

1. CLASSES ARE OPAQUE SHELLS. The emitted class declares the one method and no
   members; the body reaches fields by offset. Measured, the same function
   written with real members and with offset casts compiles to BYTE-IDENTICAL
   objects. So no class layout is ever needed - which matters, because 84 of the
   106 classes owning the remaining work have no pinned layout - and `/Zp`
   cannot affect codegen, since the compiler never computes an offset.

2. FIXED GLOBALS ARE `static T *const g = (T *)0xADDR;`. Measured, three
   spellings of the same two-global store:

       extern int *g;  *g = x;   the repo's seam style   +1 `mov` per access
       extern int g;    g = x;                           needs a relocation
       static int *const g = (int *)0x9BBFEC; *g = x;    the original's bytes

   Only the third reproduces the original's encoding INCLUDING the address. The
   first cannot match at any tier.

   BUT THE THIRD CANNOT DO READ-MODIFY-WRITE, and that is not a small corner.
   The original writes `dec dword ptr [0x93F664]`; through the const-pointer
   spelling VC6 emits load/dec/store on every flag set, for every source form
   tried - `(*g)--`, `--(*g)`, `g[0]--`, `*g -= 1`, a named `int *p` local,
   and volatile variants - including in an isolated one-line probe. The
   second spelling emits the single in-place `dec`, and 0x00592DE0 is
   BYTE_EXACT with it.

   The reason the second was set aside - "needs a relocation" - is not
   disqualifying, because the comparison MASKS relocation operands. It costs
   the address bytes, which are exactly the bytes that are not compared. So
   the rule is by ACCESS SHAPE, not one spelling for everything: a plain load
   or store takes the const pointer, and a read-modify-write takes
   `extern T name;`. The second is also the more readable of the two, since
   it carries a name instead of an address.

3. `__thiscall` IS A MEMBER FUNCTION, never a free function with a keyword. VC6
   rejects the keyword outright - `error C4234: '__thiscall' keyword reserved
   for future use` - and 1,708 of the 3,063 remaining functions are `__thiscall`,
   so this is the majority path rather than a corner.

WHAT THIS EMITS IS A VERIFICATION ARTIFACT, NOT PRODUCT SOURCE. Decisions 1 and
2 are deliberately at odds with the authoring style `AGENTS.md` mandates for
`src/`, which is written to BEHAVE and uses rebindable seams and `volatile`
field access - both of which are byte-visible and cannot match. Reconciling the
two spellings is a later phase's problem; pretending they are the same file is
how that phase would get skipped.

Output is derived from proprietary bytes - it names addresses the original
touches. Local analysis only, never committed.
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

import pefile  # noqa: E402

import catalogue_corrections  # noqa: E402
import class_layouts  # noqa: E402
import recovery_symbols  # noqa: E402
from generator_support import (absolute_operands, parse_body_ranges,  # noqa: E402
                               read_bytes)
from declfix import CRT_SIGNATURES, decode_signature  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DERIVED_CSV = REPO_ROOT / "docs" / "recovery" / "derived-prototypes.csv"
CALLGRAPH = REPO_ROOT / "docs" / "recovery" / "callgraph.json"

# VC6 has neither <cstdint> nor <stdint.h>.
PRELUDE = """typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// WHAT THE BODY NEEDS, not only what the signature reaches. The unit used to
// declare exactly the types the DECODED SIGNATURE mentioned, which is correct
// for the definition head and wrong for everything inside it. Measured over
// every NO_COMPILE row in the map on 2026-08-14: 1,544 implemented pieces do
// not compile, and 899 DISTINCT undeclared identifiers cause it - led by
// `NULL` at 50 bodies, which is one line.
//
// Every name below is already defined somewhere in src/*.h. The scaffold is a
// standalone unit and cannot include those headers - they pull in the whole
// project - so the cheap, layout-free half is restated here. Constants and
// typedefs only: no class, no global address, nothing that could disagree with
// a layout the emitter computes elsewhere.
#ifndef NULL
#define NULL 0
#endif

// Windows typedefs. The brief used to tell agents these were "a fact about the
// unit, not about the body - do not rewrite the body to chase it", which is a
// scaffold gap described accurately and then accepted. 432 bodies stop on
// C2061 for want of these ten lines.
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef void *HANDLE;
typedef void *HWND;
typedef void *HDC;
typedef unsigned int UINT;
"""

_CONSTANTS_CACHE = None
_SEAM_CACHE = None
SEAM_HEADER = REPO_ROOT / "src" / "original_seam.h"
TEARDOWN_HEADER = REPO_ROOT / "src" / "vector_teardown.h"


def _spliced(path: Path) -> str:
    """A layout-free header, minus the lines that mean nothing in a fragment."""
    return "\n".join(
        line for line in path.read_text().splitlines()
        if line.strip() != "#pragma once"
        and not line.strip().startswith("#include")).strip()


def teardown_header() -> str:
    """`src/vector_teardown.h`, spliced for the same reason as the seam.

    Declarations only - two iterator seams, a teardown pointer-to-member and
    the game's `operator new` - so it emits no code either. It includes
    `original_seam.h`, so whatever splices this must splice that first.
    """
    return ("\n// Spliced verbatim from src/vector_teardown.h. See "
            "seam_header().\n" + _spliced(TEARDOWN_HEADER) + "\n")


def seam_header() -> str:
    """`src/original_seam.h`, spliced rather than restated.

    Fifty NO_COMPILE bodies stop on `original_slot` and nine more on
    `original_address`: both are defined in this header, every recovered
    source that calls into the original image includes it, and the scaffold -
    which is a standalone unit and includes nothing from src/ - did not. So
    these bodies compile in the build and fail only under measurement, which
    makes it a harness defect reported as a body defect.

    SPLICED, NOT COPIED INTO `PRELUDE`, and the distinction is the whole
    point: a copy is a second definition of the seam that nothing keeps equal
    to the first, and the failure mode is a unit that measures a body against
    a seam the build does not use. The header is uniquely suited to it - it
    includes nothing, declares no layout, and emits no code: one incomplete
    class, three templates and a macro. Nothing here can change a byte the
    subject compiles to, which is what makes splicing it wholesale safe.
    """
    global _SEAM_CACHE
    if _SEAM_CACHE is not None:
        return _SEAM_CACHE
    _SEAM_CACHE = ("\n// Spliced verbatim from src/original_seam.h so the unit "
                   "calls into the\n// original image exactly as the build "
                   "does. See seam_header().\n" + _spliced(SEAM_HEADER) + "\n")
    return _SEAM_CACHE


CONSTANT_IN_ENUM = re.compile(
    r"^\s*([A-Z][A-Z0-9_]{2,})\s*=\s*(-?(?:0[xX][0-9a-fA-F]+|\d+))\s*,?\s*(?://.*)?$")
# ANY INTEGRAL TYPE, not just `int`. The pattern read `constexpr int` and
# `src/` writes `constexpr size_t` for every offset and slot constant it has -
# DialogsDestructorAdjustment, ListBoxDestructorAdjustment,
# StringStructDerivedCloseAdjustment, BufferSurfaceLockSlot, MapWinActiveOffset
# and the rest - so a body using one got `C2065: undeclared identifier` for a
# value sitting in a header two directories away. Restating it is the whole
# point of this function; it was just spelled too narrowly to find them.
CONSTANT_CONSTEXPR = re.compile(
    r"^\s*constexpr\s+(?:unsigned\s+)?"
    r"(?:int|size_t|long|short|char|uint\d+_t|int\d+_t)\s+(\w+)\s*=\s*"
    r"(-?(?:0[xX][0-9a-fA-F]+|\d+))\s*;")


def game_constants() -> str:
    """The integer constants src/*.h already defines, restated for the unit.

    THE SCAFFOLD DECLARES WHAT THE SIGNATURE REACHES, NOT WHAT THE BODY NEEDS,
    and that is the largest blocker in the tree: measured 2026-08-14, 1,544
    implemented pieces are NO_COMPILE and 899 distinct undeclared identifiers
    cause it - `TRIAD_AIR`, `MaxPlayerNum`, `BIT_FUNGUS`, `MaxVehProtoFactionNum`
    and hundreds more, every one of them already defined in a header this unit
    cannot include, because including it would pull in the whole project.

    DERIVED, NOT LISTED. A hand-written set of "the constants that matter" is
    the defect shape this repository names as its highest-yield, and it would
    go stale the first time a header gained a value. This reads the headers.

    Deliberately narrow: only enum members and `constexpr int` with a LITERAL
    integer value. No expressions (they can reference things this unit does not
    have), no globals (they carry addresses the emitter computes elsewhere and
    a second spelling could disagree), no types. A constant is a value and
    cannot disagree with a layout.
    """
    global _CONSTANTS_CACHE
    if _CONSTANTS_CACHE is not None:
        return _CONSTANTS_CACHE
    found: dict = {}
    src = REPO_ROOT / "src"
    if src.is_dir():
        for header in sorted(src.glob("*.h")):
            try:
                text = header.read_text(errors="replace")
            except OSError:
                continue
            for line in text.splitlines():
                for pattern in (CONSTANT_CONSTEXPR, CONSTANT_IN_ENUM):
                    hit = pattern.match(line)
                    if not hit:
                        continue
                    name, value = hit.group(1), hit.group(2)
                    # First definition wins, and a name defined twice with
                    # DIFFERENT values is dropped: the unit must not pick a
                    # side the headers do not agree on.
                    if name in found and found[name] != value:
                        found[name] = None
                    elif name not in found:
                        found[name] = value
                    break
    lines = [f"const int {name} = {value};"
             for name, value in sorted(found.items()) if value is not None]
    lines.extend(constant_tables())
    _CONSTANTS_CACHE = ("\n// Integer constants restated from src/*.h, which "
                        "this standalone unit cannot include.\n"
                        + "\n".join(lines) + "\n") if lines else ""
    return _CONSTANTS_CACHE


# `const int RadiusOffsetX[] = { 1, 0, -1, ... };` - a whole TABLE of literals.
# The same rule as a scalar constant, one dimension up.
CONSTANT_TABLE = re.compile(
    r"^(?:static\s+|DLLEXPORT\s+)*const\s+"
    r"(?:int|short|char|int8_t|uint8_t|int16_t|uint16_t|int32_t|uint32_t)\s+"
    r"(?P<name>\w+)\s*\[\s*\w*\s*\]\s*=\s*\{(?P<values>[^}]*)\}\s*;",
    re.M)
# Nothing but integer literals, signs, separators and whitespace. A table with
# a NAME in it - `{ MaxPlayerNum, 0 }` - is not restated: the name may be one
# this unit does have, and the moment it is not the table becomes a syntax
# error in the prelude, which costs every body rather than the one that reads
# the table.
LITERAL_VALUES = re.compile(r"[\s,\-+0-9xXa-fA-F]*")


def constant_tables() -> list:
    """`const int NAME[] = {...};` for the literal tables src/*.h defines.

    A CONSTANT ARRAY CANNOT BE REACHED BY DECLARATION. `const` at namespace
    scope has internal linkage in C++, so `extern const int RadiusOffsetX[];`
    does not name the header's array, and an unbounded extern array is
    `C2036: unknown size` the moment it is subscripted. The values are the
    only way in, and they are literals sitting in `src/map.h`.

    137 bodies were blocked on five of these - the radius-offset walk that
    every base-radius loop in the game uses.
    """
    out, found = [], {}
    src = REPO_ROOT / "src"
    if not src.is_dir():
        return out
    for header in sorted(src.glob("*.h")):
        try:
            text = header.read_text(errors="replace")
        except OSError:
            continue
        for hit in CONSTANT_TABLE.finditer(text):
            name, values = hit.group("name"), hit.group("values")
            if not LITERAL_VALUES.fullmatch(values):
                continue
            spelled = " ".join(values.split())
            if name in found and found[name] != spelled:
                found[name] = None      # two headers disagree: emit neither
            elif name not in found:
                found[name] = spelled
    for name, values in sorted(found.items()):
        if values is not None:
            out.append(f"const int {name}[] = {{{values}}};")
    return out


BUILTIN = {
    "void", "char", "signed", "unsigned", "short", "int", "long", "float",
    "double", "bool", "const", "volatile", "struct", "class", "union",
    "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
    "__int64",
    # The catalogue spells fixed-width types WITHOUT the `_t`: measured over
    # the remaining targets' prototypes, `int8` appears 222 times, `int16`
    # twice, `int32` once. Omitting them here made the emitter forward-declare
    # `struct int8;` and then use it as a type, which agents were patching by
    # hand on every affected function.
    "int8", "uint8", "int16", "uint16", "int32", "uint32",
    # CL declares `size_t` itself with no header included, so forward-
    # declaring it is `error C2371: redefinition; different basic types`.
    "size_t",
    # `this` is a keyword, never a type. `struct this;` is a syntax error and
    # was being emitted for every __thiscall method whose receiver parameter
    # reached the type scan.
    "this",
    # Neither is a calling convention. A function-pointer parameter carries one
    # inside its type text, the type scan read it as an identifier, and
    # `struct __cdecl;` is a syntax error that took 179 units with it - every
    # declaration after it in the file.
    "__cdecl", "__stdcall", "__thiscall", "__fastcall", "__declspec",
}

# IDA writes an anonymous type it could not name as `#120`. There is no C++
# spelling for it, and emitting one puts a `#` mid-line where CL expects a
# preprocessor directive (`error C2014`), which cost 26 units.
ANONYMOUS_TYPE = re.compile(r"#\d+")

# Win32 types that are NOT structs. Forward-declaring one as `struct HRESULT;`
# and then using it by value is `error C2027: use of undefined type`, or
# `C2526: C linkage function cannot return C++ class 'HRESULT'` when it is a
# return type - 17 units, all of them a DirectDraw or CRT import.
#
# The spellings are the Win32 headers' own, so they keep both their size and
# their mangling. Safe to choose here because NONE of these names appears
# inside a catalogued mangled name: counted over all 6,000, the only Windows
# types that do are the real structs - `RECT` 92, `FILE` 9, `_GUID` 2,
# `PALETTEENTRY` 2, `WINDOWPOS` 2, `RGBQUAD` 1, `DPNAME` 1 - and those stay
# forward-declared, which is what their `PAURECT@@` asks for.
NOT_A_STRUCT = {
    "BOOL": "int", "CHAR": "char", "WCHAR": "unsigned short",
    "BYTE": "unsigned char", "WORD": "unsigned short",
    "UINT": "unsigned int", "DWORD": "unsigned long", "LONG": "long",
    "ULONG": "unsigned long", "LCID": "unsigned long",
    "LPARAM": "long", "WPARAM": "unsigned int", "LRESULT": "long",
    "UINT_PTR": "unsigned int", "ULONG_PTR": "unsigned long",
    "SIZE_T": "unsigned int", "HRESULT": "long",
    "LPSTR": "char *", "LPCSTR": "const char *",
    "PVOID": "void *", "LPVOID": "void *", "LPCVOID": "const void *",
    "LPBYTE": "unsigned char *", "LPWORD": "unsigned short *",
    "LPGUID": "struct _GUID *",
    "PEXCEPTION_RECORD": "struct _EXCEPTION_RECORD *",
    "FARPROC": "int (__stdcall *)()",
    # Handles are opaque pointers. `void *` rather than the headers'
    # `struct HWND__ *` because nothing here needs them distinct and the
    # mangling reaches no comparison.
    # `HCURSOR` is deliberately ABSENT: `?set_cursor@Win@@QAEHPAUHCURSOR@@@Z`
    # mangles it as a struct, so a typedef here emits `PAPAX` and loses the
    # one row that uses it. Checked the same way for every name in this table
    # rather than by inspection - it is the only clash, and it was found by
    # the pairing count dropping by one.
    "HANDLE": "void *", "HWND": "void *", "HINSTANCE": "void *",
    "HIC": "void *", "LPDIRECTDRAW": "void *",
    # HDC was in class_layouts.WINDOWS_TYPEDEF and missing HERE, and the two
    # lists are kept in step BY HAND. Nothing noticed while Buffer was
    # unreadable, because `HDC hdc2_;` never reached a unit; the moment Buffer
    # became extractable the emitter forward-declared `struct HDC;` beside a
    # by-value member and 72 byte-exact bodies went NO_COMPILE with
    # `C2079: 'hdc2_' uses undefined struct 'HDC'`. test_class_layouts now
    # requires the two sets to agree, so the next omission cannot wait for a
    # cascade to reveal it.
    "HDC": "void *",
    # GDI object handles, added on the same evidence and checked the same way:
    # each name was counted against `PA[UV]<name>@@` across all 6,000
    # catalogued mangled names and each scores ZERO, where HCURSOR scores one.
    # Their absence refused Font (HFONT) and Buffer (HRGN, HBITMAP) outright,
    # and Buffer refuses GraphicWin, which refuses eight more.
    "HFONT": "void *", "HRGN": "void *", "HBITMAP": "void *",
    "HPALETTE": "void *", "HBRUSH": "void *", "HPEN": "void *",
    "HMENU": "void *",
}


def class_keys(functions: dict) -> dict:
    """{class name: 'struct' | 'class'} - a class if it has methods.

    MSVC encodes a struct `U` and a class `V`, and they mangle differently, so
    the two objects have to agree on the key. The catalogue cannot settle it:
    six classes disagree with THEMSELVES there (`Buffer` is `U` 49 times and
    `V` 17), and neither can the image, which carries no RTTI and no embedded
    mangled strings. A tally was tried and only bought the majority of each
    argument; the minority stayed unpairable.

    It does not have to be settled, because BOTH objects are ours. The target
    object's symbol is a computed choice already - the 1,179 disassembler
    labels get a decoration nothing in the image ever carried - so the key
    only has to be the SAME on both sides, and `recovery_symbols
    .canonicalise_class_keys` writes whatever this returns into the target.

    Which leaves it a naming convention, so it follows the ordinary one: a
    type with methods is a class, a type that is only data is a struct. 152
    catalogued types have methods; `RECT`, `_GUID` and the rest of the plain
    data stay structs.
    """
    keys = {}
    for row in functions.values():
        name = row.get("name") or ""
        if not name.startswith("?"):
            continue
        for pattern in (MANGLED_CTOR, MANGLED_DTOR, MANGLED_METHOD):
            found = pattern.match(name)
            if found:
                keys[found.group("cls")] = "class"
                break
        else:
            # `??_G<Class>@@`, `??4<Class>@@` and the rest of the operator and
            # compiler-generated family, whose qualifier chain the three
            # patterns above do not describe. The class is still the second
            # link in the chain when there is one.
            split = name.find("@@")
            chain = [part for part in name[1:split].split("@") if part] \
                if split != -1 else []
            if len(chain) >= 2:
                keys[chain[1]] = "class"
    # Everything referenced only as a type, and never as a receiver, is data.
    # Naming every one of them matters as much as the choice does: a type
    # missing from this map is declared `struct` by the emitter and left
    # however the catalogue spelled it in the target, which is the same
    # disagreement in a quieter form.
    for row in functions.values():
        for subject in recovery_symbols.class_key_uses(row.get("name") or ""):
            keys.setdefault(subject, "struct")
    keys.pop("", None)
    return keys


@functools.lru_cache(maxsize=1)
def renamed_for_collision() -> frozenset:
    """Addresses whose catalogued name is ALSO another row's symbol.

    `?get@NetFifo@@QAEHPAXPAIPAHPAI@Z` at 0x00633F70 compresses to
    `?get@NetFifo@@QAEHPAXPAIPAH1@Z`, which is what 0x00633D90 is catalogued
    as outright - two different functions, 173 bytes and 229, under one
    symbol. A linked image cannot hold that, so one of the two names is
    wrong, and there is nothing here that can say which.

    Every tool resolves a subject BY symbol, so the loser would be handed the
    winner's context, target object and assembly and the agent would work
    against another function's bytes. Renaming it is the same device the
    emitter already uses for the 908 names that are not identifiers: the
    symbol is ours, it only has to be unique and agreed on both sides. The
    lowest address keeps the catalogued spelling, so the choice is stable
    across runs.
    """
    keys = catalogue_class_keys()
    owners = {}
    for address, row in load_functions().items():
        name = (row.get("name") or "").strip()
        if not name.startswith("?"):
            continue
        symbol = recovery_symbols.compress_backrefs(
            recovery_symbols.canonicalise_class_keys(
                recovery_symbols.empty_destructor_arguments(name), keys))
        owners.setdefault(symbol, []).append(address)
    return frozenset(address for group in owners.values() if len(group) > 1
                     for address in sorted(group)[1:])


@functools.lru_cache(maxsize=1)
def catalogue_class_keys() -> dict:
    """`class_keys` over the real catalogue, read once.

    One table for the whole process. Two callers computing their own from
    different inputs is exactly the failure this map exists to prevent.
    """
    return class_keys(load_functions())


# `int (__thiscall ?add@X@@QAEHH@Z)(X* this, int)`
PROTOTYPE_RE = re.compile(
    r"^\s*(?P<ret>.+?)\s*\(\s*(?P<conv>__\w+)\s+(?P<name>\S+)\s*\)\s*"
    r"\((?P<params>.*)\)\s*$")

MANGLED_METHOD = re.compile(r"^\?(?P<method>[\w_]+)@(?P<cls>[\w_]+)@@")
MANGLED_CTOR = re.compile(r"^\?\?0(?P<cls>[\w_]+)@@")
MANGLED_DTOR = re.compile(r"^\?\?1(?P<cls>[\w_]+)@@")
MANGLED_FREE = re.compile(r"^\?(?P<name>[\w_]+)@@")


class Unsettled(Exception):
    """This row cannot be emitted, and says why rather than guessing."""


# ------------------------------------------------------------------ metadata

def load_functions() -> dict:
    """The catalogue, with the names the bytes prove wrong corrected.

    The image carries no symbols, so every catalogued mangled name is IDA's
    analysis and 62 are contradicted by the bodies they name - see
    `catalogue_corrections`. Correcting HERE rather than at each use is what
    keeps the declaration, the decoded signature and the emitted symbol
    agreeing with one another: `byte_match.load_rows` and
    `project_catalogue.catalogue` both read through this function, and both say
    so, because two loaders with two answers about one row is how the `QAA` ->
    `SAA` correction on `Caviar::vx_read` once reached the emitter but not the
    comparison.

    SRC/ IS THE ONLY STORE. `docs/recovery/functions.csv` was deleted (185dd977)
    and the export fallback that used to stand here - it took the CSV's rows for
    addresses `src/` did not carry, on the measured grounds that 28 rows were
    `source_complete` with no annotation anywhere - guarded a file that cannot
    exist. Measured against the deleted file itself, restored out of git: those
    28 have since been annotated, so replaying the fallback over today's `src/`
    ADDS 0 ROWS, and the CSV's address set and `from_source`'s are the same
    6,000 with no row differing in content. Deleting it loses nothing.

    THE APPLY IS NOT PART OF THE FALLBACK, it was only nested inside it, and it
    was the whole tree's only call into `catalogue_corrections` - so the
    corrections went silent the day the CSV went away, while three docstrings
    kept promising them. It is hoisted out unconditionally rather than dropped;
    what it restores is the tripwire - `apply` raises `Stale` rather than
    correcting when a row's name is neither the catalogued spelling nor the
    corrected one, which is the check that was dead, not the rewrite.

    THE REWRITE IS NO LONGER A NO-OP, and that is worth stating because the
    argument for hoisting it rested on the opposite. When the call was restored
    it rewrote 0 of the 6,000 names - `src/` already spelled all 15 corrections
    the corrected way - so no live measurement could distinguish a working call
    from a deleted one. Since 2026-08-13 it rewrites 47: the
    `??3<Class>@@SAXPAXI@Z` rows whose bodies are this-adjusting thunks are
    still annotated with the wrong name in `src/deleting_thunks.cpp`, and this
    is where they become right for every consumer. A regression in this call is
    now visible in the catalogue itself, not only in the unit test.
    """
    import project_catalogue

    return catalogue_corrections.apply(project_catalogue.from_source())


def load_derived() -> dict:
    if not DERIVED_CSV.is_file():
        return {}
    with DERIVED_CSV.open() as handle:
        return {int(r["address"], 16): r for r in csv.DictReader(handle)}


def load_callees() -> dict:
    """{caller address: [callee addresses]} from the edge list.

    `callgraph.json` holds `edges` as a LIST of {source, target, call_sites},
    not an address-keyed map. Reading it as a map is how a seam filter once
    admitted every row it was meant to filter, so the shape is asserted here
    rather than assumed.

    THE EDGES ARE IN `src/` NOW, one `// calls` line per annotation, and the
    JSON is the fallback. Flipping `load_functions` without this one left the
    emitter declaring NO callees at all, so every unit failed on the first CRT
    name it used - `error C2065: 'abs' : undeclared identifier` - and 241
    BYTE_EXACT claims went NO_COMPILE at once. A catalogue is two halves and
    moving one is not moving it.
    """
    if not CALLGRAPH.is_file():
        import project_catalogue
        return {address: sorted(row.get("_calls") or ())
                for address, row in project_catalogue.from_source().items()}
    data = json.loads(CALLGRAPH.read_text())
    edges = data.get("edges")
    if not isinstance(edges, list):
        raise Unsettled("callgraph.json 'edges' is not a list")
    out = {}
    for edge in edges:
        out.setdefault(int(edge["source"], 16), set()).add(
            int(edge["target"], 16))
    return {key: sorted(value) for key, value in out.items()}


# ----------------------------------------------------------------- signatures

def strip_receiver_token(text: str) -> str:
    """Remove a bare `this` from a parameter's TYPE text.

    The catalogue writes a receiver as `Caviar* this`, and emitting that into a
    parameter list gives `Caviar* this a1` - a keyword where a name belongs.

    Dropping the whole parameter is right for a `__thiscall` receiver and WRONG
    for anything else: 93 units carry a parameter named `this` on a `__cdecl`
    or `__stdcall` function, where it is an ordinary argument that happens to
    be spelled with a keyword. So the token is removed and the parameter kept.
    """
    return re.sub(r"\bthis\b", "", text).strip()


def parameter_type(text: str) -> str:
    """A parameter's TYPE, with the name IDA recorded beside it removed.

    The catalogue writes `int8* lpString`, and the emitter appends its own
    `a2`, so the declaration reads `int8* lpString a2` - two identifiers where
    one belongs (`error C2146`, 35 units). A trailing word is a name only when
    it is not itself part of the type, so `unsigned int` and `char *` survive.

    A function-pointer type is left whole: its trailing token is `)`, and any
    identifier inside it belongs to the pointee's own parameter list.

    `__thiscall` is stripped from one, because VC6 rejects the keyword on a
    free function pointer outright (`error C4234`, 16 units) - the same
    restriction the VCall shim exists to work around. The pointer is still one
    stack slot and still mangles the same, so nothing byte-visible moves.
    """
    text = ANONYMOUS_TYPE.sub("void", text).strip()
    text = re.sub(r"\b__thiscall\s*", "", text)
    if text.endswith(")") or "(" in text:
        return text.strip()
    words = text.split()
    if len(words) > 1 and re.fullmatch(r"[A-Za-z_]\w*", words[-1]) \
            and words[-1] not in BUILTIN:
        return " ".join(words[:-1])
    return text


def split_params(text: str) -> list:
    """Top-level comma split, so `X<a,b>` and function pointers survive."""
    out, depth, current = [], 0, ""
    for character in text:
        if character in "(<[":
            depth += 1
        elif character in ")>]":
            depth -= 1
        if character == "," and depth == 0:
            out.append(current.strip())
            current = ""
        else:
            current += character
    if current.strip():
        out.append(current.strip())
    return out


def named_types(text: str) -> set:
    """Identifiers in a type expression that are not builtins."""
    return {word for word in re.findall(r"[A-Za-z_]\w*", text)
            if word not in BUILTIN}


# `int (__cdecl *)()`, `void (__cdecl *)(int8*)`: the pointer declarator a
# parameter NAME has to go inside, not after.
POINTER_DECLARATOR = re.compile(r"(\(\s*(?:__\w+\s+)?\*(?:\s*\*)*)\s*\)")


def named_parameter(text: str, name: str) -> str:
    """`int (__cdecl *)()` and `a2` -> `int (__cdecl *a2)()`.

    C declarator syntax puts the name INSIDE the parentheses for a function
    pointer. Appending it - `int (__cdecl *)() a2` - is
    `error C2146: syntax error : missing ')' before identifier 'a2'`, and 45
    of the 154 units that would not compile were exactly that, one per
    function-pointer parameter. Everything else just takes the name after.
    """
    if POINTER_DECLARATOR.search(text):
        return POINTER_DECLARATOR.sub(rf"\1{name})", text, count=1)
    return f"{text} {name}"


def prototype_from_name(mangled: str) -> str:
    """The prototype the MANGLED NAME states, in the catalogue's own grammar.

    The linker wrote this name from the real declaration, so it is the best
    evidence there is for a row the catalogue has no prototype for - better
    than the body's `ret`, which gives an argument COUNT and nothing else.

    Empty when the name is not mangled or carries a type this cannot decode;
    the caller then falls back to the `ret`.
    """
    decoded = decode_signature(mangled)
    convention = recovery_symbols.convention_of(mangled)
    if decoded is None or not convention:
        return ""
    returns, params = decoded
    # `this` is deliberately absent: it is not in the mangled argument list
    # either, and `Signature` reads membership off the access code.
    return f"{returns} ({convention} {mangled})({', '.join(params)})"


def prototype_from_ret(pe, row: dict) -> str:
    """A signature read out of the body's own `ret N`, for rows with no other.

    679 targets have no prototype in `functions.csv` and none in
    `derived-prototypes.csv`. `callee-purge.csv` looked like the answer and is
    not - measured, it covers exactly ONE of them, because it catalogues a
    different population. But the purge byte is IN THE BODY, and reading it
    directly covers 638 of the 679 (213,111 bytes).

    `tools/disasm.py` calls the callee-pop count "the only ground truth for
    arity", and that is what this reads. What it does NOT settle is the
    convention: `ret 8` is `__stdcall(int, int)` and `__thiscall(this, int)`
    alike, and those emit DIFFERENT code because one passes a receiver in ECX.
    So this returns the `__stdcall` reading and the caller is told to expect
    the other to be tried - the oracle answers in 44 ms, which is a cheaper way
    to decide than any inference here would be.

    A bare `ret` is returned as a zero-argument `__cdecl`, which is a FLOOR
    rather than a reading: caller-pop conventions purge nothing, so the byte
    carries no argument count at all. Those rows are marked so the count is
    never mistaken for evidence.
    """
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    spans = parse_body_ranges(row.get("body_ranges") or "")
    if not spans:
        return ""
    low, high = spans[0]
    data = read_bytes(pe, low, high - low)
    if not data:
        return ""
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    purges = {one.op_str for one in engine.disasm(data, low)
              if one.mnemonic == "ret"}
    if len(purges) > 1:
        # Two different purges in one body is not an arity, it is a
        # contradiction - usually a span covering more than one function.
        # Picking either would be a guess that changes emitted bytes.
        return ""
    if not purges:
        # 76 bodies leave through a tail `jmp` and never execute a `ret` at
        # all, so there is no purge byte to read. That is the same state of
        # knowledge a bare `ret` leaves - caller-pop, arity unknowable - and
        # they were the entire remaining "no scaffolding at all" population.
        return "int (__cdecl X)()"
    text = purges.pop()
    if not text:
        return "int (__cdecl X)()"          # floor: arity unknowable from bytes
    try:
        count = int(text, 0) // 4
    except ValueError:
        return ""
    return f"int (__stdcall X)({', '.join(['int'] * count) or ''})"


VTABLE_SLOT_CAP = 128


def vtable_slots(pe, spans, cap: int = VTABLE_SLOT_CAP):
    """Slot indices this body dispatches through, from `call [reg+disp]`.

    827 of the remaining targets contain such a call and they hold **33% of the
    remaining mass** (720,586 bytes), so this is not a corner case.

    A call through a function-pointer FIELD looks identical, and what tells
    them apart is where the base register came from. A vtable pointer is read
    by dereferencing an OBJECT; a function pointer held in a local or an
    argument is read off the stack frame:

        mov eax, [ecx]          mov eax, [esp+0xc]     <- off the stack
        call [eax + 0xf8]       call [eax + 0x24]
        a vtable slot           a function-pointer field at +0x24

    Both were previously read as slots. 0x00644910 is the measured case: the
    emitter produced a ten-slot shim for a body with no vtable at all, and the
    agent had to notice and ignore it.

    "Dereferenced at offset zero" was tried first as the test and is WRONG: a
    virtual base reads its vtable through a computed address,
    `mov eax, [edx + ecx - 0x1c]`, and 0x0060FB90 stopped being detected at
    all. Excluding esp/ebp-relative loads keeps that case and still rejects
    the field.

    The cap stays as a backstop. Measured over the 806 readable bodies before
    any of this: median highest slot 37, p99 93, largest 6,286 - a 25 KB
    "vtable" and certainly a field.
    """
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from capstone.x86 import (X86_OP_MEM, X86_OP_REG, X86_REG_ESP,
                              X86_REG_EBP)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    FRAME = (X86_REG_ESP, X86_REG_EBP)
    found = set()
    for low, high in spans:
        data = read_bytes(pe, low, high - low)
        if not data:
            continue
        # Registers holding something read out of an OBJECT rather than off
        # the stack frame - which is what a vtable pointer load leaves behind.
        object_regs = set()
        for one in engine.disasm(data, low):
            if one.mnemonic == "mov" and len(one.operands) == 2:
                destination, source = one.operands
                if destination.type == X86_OP_REG:
                    # A FIXED-ADDRESS SINGLETON is an object too. `mov edx,
                    # ds:[0x7B0CB8]` has no base register at all, so the
                    # `base != 0` test dropped it and the `call [edx+N]` that
                    # followed was not read as a slot - the emitter produced no
                    # shim for a body with two catalogued indirect calls, and
                    # the agent hand-wrote one. Reported against 0x004E0FD0.
                    #
                    # Gated on the displacement looking like an IMAGE ADDRESS,
                    # which is what keeps `[esp+8]` and friends out; those have
                    # a base register and are already excluded, but an absolute
                    # load with a small displacement would be something else
                    # entirely and is not admitted on a guess.
                    absolute = (source.type == X86_OP_MEM
                                and source.mem.base == 0
                                and source.mem.index == 0
                                and source.mem.disp >= 0x400000)
                    if (source.type == X86_OP_MEM and source.mem.base != 0
                            and source.mem.base not in FRAME) or absolute:
                        object_regs.add(destination.reg)
                    else:
                        object_regs.discard(destination.reg)
            elif one.mnemonic == "call":
                for operand in one.operands:
                    if (operand.type == X86_OP_MEM and operand.mem.base != 0
                            and operand.mem.disp >= 0
                            and operand.mem.base in object_regs):
                        found.add(operand.mem.disp // 4)
    return sorted(slot for slot in found if slot <= cap), sorted(
        slot for slot in found if slot > cap)


def com_slots(pe, spans, cap: int = VTABLE_SLOT_CAP) -> list:
    """Slots dispatched COM-style: `This` PUSHED, not passed in ecx.

    `vtable_shim` models a C++ virtual call, because that is what a vtable
    dispatch is in this codebase. A COM interface calls the other way round -
    the interface pointer is argument ZERO on the stack:

        mov  eax, [0x009BE600]      ; the interface
        mov  ecx, [eax]             ; its vtable
        push eax                    ; `This`, as an explicit argument
        call [ecx + 0x10]

    Through the `VCall` class VC6 emits a receiver in ecx and NO push, so the
    call shape is wrong and no body written against it can ever match. An
    agent recovering `?create_session@Net@@` found this the hard way and
    hand-wrote `__stdcall` function-pointer casts instead; two others in the
    same batch hand-split the shim for unrelated reasons.

    THE TELL IS THE LAST PUSH BEFORE THE CALL being `<obj>` itself - the very
    register the vtable was loaded FROM. Nothing softer works: an ordinary
    thiscall dispatch also pushes arguments, and only the last push being the
    OBJECT distinguishes the two.

    "The instruction immediately before the call" was the first rule and it
    has a false negative, reported by an agent recovering 0x0062E540 that
    needed a slot this did not offer. The two orderings are both real:

        mov  ecx, [eax]     push eax
        push eax            mov  ecx, [eax]
        call [ecx + 0x10]   call [ecx + 0x7c]

    The vtable load may sit between the push and the call, so the search
    skips non-pushes - and stops at the previous CALL, because a push from an
    earlier call's argument list says nothing about this one.

    Measured over the whole image: 301 such sites in 125 catalogued
    functions, 30 of them still unrecovered. DirectPlay, DirectDraw and
    DirectSound are COM, and `docs/EXCLUSIONS.md` puts that surface at 48.4%
    of catalogued bytes.
    """
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from capstone.x86 import (X86_OP_MEM, X86_OP_REG, X86_REG_ESP,
                              X86_REG_EBP)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    FRAME = (X86_REG_ESP, X86_REG_EBP)
    found = set()
    for low, high in spans:
        data = read_bytes(pe, low, high - low)
        if not data:
            continue
        # {vtable register: the object register it was read out of}. Only a
        # load at displacement ZERO counts: a vtable pointer lives at the top
        # of the object, and `mov eax, [esi+0x40]` is a field.
        # `last_push` is the most recent `push <reg>` since the previous CALL,
        # or None where the last push was of something that is not a register.
        vtable_of, last_push = {}, None
        for one in engine.disasm(data, low):
            if one.mnemonic == "push":
                last_push = (one.operands[0].reg
                             if one.operands
                             and one.operands[0].type == X86_OP_REG else None)
                continue
            if one.mnemonic == "mov" and len(one.operands) == 2:
                destination, source = one.operands
                if destination.type != X86_OP_REG:
                    pass
                elif (source.type == X86_OP_MEM and source.mem.base
                        and source.mem.base not in FRAME
                        and source.mem.index == 0 and source.mem.disp == 0):
                    vtable_of[destination.reg] = source.mem.base
                else:
                    vtable_of.pop(destination.reg, None)
            elif one.mnemonic == "call":
                if one.operands and one.operands[0].type == X86_OP_MEM:
                    operand = one.operands[0]
                    obj = vtable_of.get(operand.mem.base)
                    if (obj is not None and last_push == obj
                            and operand.mem.disp >= 0):
                        found.add(operand.mem.disp // 4)
                # A push belonging to THIS call's argument list says nothing
                # about the next one.
                last_push = None
    return sorted(slot for slot in found if slot <= cap)


DECLARED_TYPE = re.compile(r"^\s*(?:class|struct)\s+(\w+)", re.M)


def declared_before(lines: list) -> set:
    """Type names this unit has already declared, at this point in the text.

    An EMBEDDED member needs a COMPLETE type, so it can only be declared when
    the unit has already defined that class - and it has to be already, not
    eventually, because a member cannot name a class declared below it.
    """
    return set(DECLARED_TYPE.findall("\n".join(lines)))


def proved_member_declaration(name: str, declared: set = frozenset()) -> list:
    """Member lines for a class only BYTE-EXACT bodies have described, or [].

    Read from `agent-structure-observations.csv` through
    `emit_hypothesis_layouts.proved_members`, so the emitter and the layout
    header agree about what has been proved. Gaps between known offsets become
    padding, exactly as they do for a Thinker-only layout - a member nobody has
    touched is storage, not an invention.

    Deliberately NOT routed through `class_layouts.pinned_layouts()`. That gate
    means "this layout compiles to the real class size", which is a stronger
    and different claim; these offsets are proved individually and the size is
    still unknown. Keeping them apart is what stops a partial layout being read
    downstream as a verified one.
    """
    import emit_hypothesis_layouts as layouts
    proved = layouts.proved_members().get(name) or {}
    subobjects = layouts.proved_subobjects().get(name) or {}
    if not proved and not subobjects:
        return []
    lines, cursor = [], 0
    for offset in sorted(proved):
        member, size = proved[offset]
        if offset < cursor:
            continue
        if offset > cursor:
            lines.append(f"    uint8_t pad_{cursor:x}_[0x{offset - cursor:X}];")
        lines.append(f"    uint32_t {member};" if size == 4
                     else f"    uint8_t {member};")
        cursor = offset + size
    # An embedded sub-object goes LAST and closes the class. Its size is the
    # one thing not known - that is what makes it a sub-object observation
    # rather than a member - so nothing may be declared after it without
    # inventing an offset. Only the lowest one can be placed, for the same
    # reason: a second would have to sit a known distance past the first.
    #
    # This is what takes the reinterpret_cast out. The body writes
    # `sub_interface_.release_iface_mode()` instead of
    # `((SubInterface *)((char *)this + 0xA14))->release_iface_mode()`, and
    # both compile to `lea ecx,[esi+0xa14]; call ...` - the byte comparison
    # cannot tell them apart, which is exactly why the readable one is free.
    #
    # It is emitted ONLY when this unit has already declared the type. An
    # embedded member needs a COMPLETE type, and the scaffolding declares a
    # class only when the subject reaches it - so `SubInterface` is present in
    # the units of the five bodies that call into it and absent from every
    # other method of the same class.
    #
    # Emitting it unconditionally cost 50 proved functions: BaseWin,
    # CouncWin, Datalink and SocialWin methods that never touch the
    # sub-object got a member of a type their unit never names, and VC6
    # reports an unknown type as `C2146: syntax error : missing ';'`, which
    # reads like a broken body rather than a broken declaration. The census
    # caught it and the ratchet refused to move; nothing else would have.
    for offset in sorted(subobjects)[:1]:
        member, type_name = subobjects[offset]
        if type_name not in declared:
            continue
        if offset < cursor:
            continue
        if offset > cursor:
            lines.append(f"    uint8_t pad_{cursor:x}_[0x{offset - cursor:X}];")
        lines.append(f"    {type_name} {member};")
    return lines


def vtable_shim(slots: list) -> str:
    """A class whose Nth virtual gives ordinary codegen for `call [reg+N*4]`.

    VC6 rejects a free `__thiscall` function pointer outright - `error C4234:
    '__thiscall' keyword reserved for future use` - so an indirect virtual call
    cannot be spelled directly. Declaring a class with enough virtual members
    and calling the right one makes the COMPILER emit
    `mov eax, [ptr]; call [eax + N*4]`, which is the original's encoding.

    The filler slots exist only to place the real ones at the right byte
    offset: a vtable index is decided by DECLARATION ORDER, not by signature,
    so an agent may freely change the signature of a slot it calls without
    moving it.

    This was invented by a fan-out agent that needed slots 61 and 88, and is
    generated here rather than left in a prompt so that 827 agents do not each
    rediscover it and hand-write 90 lines of filler.
    """
    if not slots:
        return ""
    highest = max(slots)
    lines = [
        "// Vtable shim. VC6 rejects a free `__thiscall` function pointer",
        "// (C4234), so an indirect virtual call is spelled by calling the Nth",
        "// virtual of a class that is never defined and never instantiated.",
        "// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of",
        "// a slot are yours to set and setting them does not move it: write",
        "// `virtual int slot074(int, int);` if that is the call you need.",
        "// The slots below are spelled nullary because the emitter reads the",
        "// vtable OFFSET from the body and not the argument list.",
        f"// This body dispatches through slot(s): "
        f"{', '.join(str(s) for s in slots)}",
        "class VCall { public:",
    ]
    for index in range(highest + 1):
        mark = "  // <-- used" if index in slots else ""
        lines.append(f"    virtual void slot{index:03d}();{mark}")
    lines.append("};")
    return "\n".join(lines)


def com_shim(slots: list) -> str:
    """Typedefs for slots that push `This` instead of passing it in ecx.

    ADDITIVE, deliberately. These slots stay in the `VCall` class above as
    well, because bodies already written against it must keep compiling - a
    slot disappearing from that class is a recovery that stops building. What
    this adds is the shape that can actually MATCH, beside the one that
    cannot, with the difference stated.

    Spelled as function pointers rather than as a second class because that
    is the one thing VC6 allows here: `__thiscall` on a function pointer is
    C4234, which is the whole reason `VCall` exists, but `__stdcall` on one
    is fine.
    """
    if not slots:
        return ""
    lines = [
        "// COM-STYLE DISPATCH, which the VCall class above CANNOT express.",
        "// These slots push the interface as an explicit first argument",
        "// instead of passing it in ecx:",
        "//",
        "//     mov  eax, [0x009BE600]   ; the interface",
        "//     mov  ecx, [eax]          ; its vtable",
        "//     push eax                 ; `This`, as argument 0",
        "//     call [ecx + 0x10]",
        "//",
        "// Through VCall, VC6 emits a receiver in ecx and NO push, so no body",
        "// written that way can match. DirectPlay, DirectDraw and DirectSound",
        "// are COM. Set the parameters and return type to what the call needs;",
        "// the interface is the first parameter and does not move. Spell a",
        "// call as:",
        "//",
        "//     ComSlot004 fn = (ComSlot004)(*(void ***)obj)[4];",
        "//     int result = fn(obj, arg);",
        "//",
        f"// This body dispatches COM-style through slot(s): "
        f"{', '.join(str(s) for s in slots)}",
    ]
    for index in slots:
        lines.append(f"typedef int (__stdcall *ComSlot{index:03d})"
                     f"(void *self);")
    return "\n".join(lines)


class Signature:
    def __init__(self, row: dict, derived: dict, pe=None, keys: dict = None):
        # `keys` is {class: 'struct'|'class'}, and it has to be the SAME map
        # the unit is emitted from - it decides half the mangled symbol. The
        # default reads the catalogue, so a caller that forgets to pass one
        # still lands on the same answer the emitter will.
        self._keys = catalogue_class_keys() if keys is None else keys
        prototype = (row.get("prototype") or "").strip()
        self.inferred = False
        if not prototype:
            entry = derived.get(int(row["address"], 16))
            prototype = (entry or {}).get("prototype", "").strip()
        if not prototype:
            # THE NAME BEFORE THE RET. 48 rows have no prototype anywhere and
            # a mangled name that decodes completely, and the name is direct
            # evidence where the `ret` is an inference: it gives the return
            # type, every argument type and the convention outright, while
            # `prototype_from_ret` can only offer N ints and cannot tell
            # __stdcall from __thiscall at all, because the two write the
            # purge byte identically.
            prototype = prototype_from_name(row.get("name") or "")
            self.inferred = bool(prototype)
        if not prototype and pe is not None:
            prototype = prototype_from_ret(pe, row)
            self.inferred = bool(prototype)
        if not prototype:
            raise Unsettled(
                "no prototype in functions.csv or derived-prototypes.csv, and "
                "no single consistent `ret` in the body to read arity from")
        match = PROTOTYPE_RE.match(prototype)
        if not match:
            raise Unsettled(f"prototype not parseable: {prototype!r}")

        self.mangled = row["name"]
        self.returns = parameter_type(match.group("ret"))
        self.convention = match.group("conv")
        # NOT stripped yet: the receiver check below reads the `this` token,
        # so removing it first makes every __thiscall prototype look like it
        # has no receiver. That mistake dropped the emitter from 3,021 units
        # to 1,306 before the count caught it.
        self.params = split_params(match.group("params"))
        self.is_method = self.convention == "__thiscall"
        if not self.is_method:
            # `QAA` is a PUBLIC __cdecl MEMBER function and `QAG` a __stdcall
            # one; 88 of them are catalogued. Keying "is this a method" off
            # __thiscall alone sent every one down the free-function path,
            # where the name `?fill_func@AlphaMenu@@QAAH...` is not an
            # identifier, so it was renamed `fn_<address>` and emitted as a
            # free function - a symbol no target object holds.
            #
            # THE NAME DECIDES, not the prototype: the access code says
            # non-static member outright, and 8 prototypes for these - the
            # `Win` window-procedure family - simply leave the receiver out.
            # A prototype that DOES carry `this` still counts, for the rows
            # whose access code this cannot read; 93 free functions take a
            # parameter honestly named `this`, and MANGLED_METHOD rejects
            # them because they have no class in the name.
            self.is_method = recovery_symbols.is_nonstatic_member(self.mangled) \
                or bool(MANGLED_METHOD.match(self.mangled) and self.params
                        and "this" in self.params[0])
        self.klass = ""
        self.method = ""
        self.kind = "free"

        if self.is_method and self.convention == "__thiscall" \
                and len(self.params) == 1 \
                and "this" not in self.params[0] \
                and not (MANGLED_CTOR.match(self.mangled)
                         or MANGLED_DTOR.match(self.mangled)
                         or MANGLED_METHOD.match(self.mangled)):
            # ONE ARGUMENT IN ECX AND NOTHING ON THE STACK, which is what IDA
            # means by `int (__thiscall sub_5CB050)(LPSTR pszFileName)` for a
            # function with no class: the single argument is the register one.
            # VC6 rejects `__thiscall` on a free function (C4234), and
            # `__fastcall` with one argument puts it in the same register and
            # cleans the same empty stack. Measured, not assumed - a
            # __thiscall member and a __fastcall free function over the same
            # body compile to the identical 22 bytes,
            # `8a1133c084d2740d0fbed203c28a51014184d275f3c3`, while __cdecl
            # loads from the stack instead.
            #
            # Only for EXACTLY one parameter. With two, __thiscall passes the
            # second on the stack and __fastcall passes it in EDX, and the two
            # stop being the same function.
            self.convention = recovery_symbols.FASTCALL
            self.is_method = False

        if self.is_method:
            # The CLASS COMES FROM THE MANGLED NAME, not from the receiver's
            # declared type. They disagree on 50 rows - the prototype for
            # `?on_mouse_move@BaseWin@@QAEXHH@Z` says `Win* this`, because the
            # receiver is spelled as the base it is used through - and the
            # class is half of the symbol, so the name is the only side of that
            # disagreement that can be right.
            found = (MANGLED_CTOR.match(self.mangled)
                     or MANGLED_DTOR.match(self.mangled)
                     or MANGLED_METHOD.match(self.mangled))
            receiver = bool(self.params) and "this" in self.params[0]
            if found:
                self.klass = found.group("cls")
            elif receiver:
                self.klass = self.params[0].replace("*", " ").split()[0]
            else:
                # `sub_5cb050`: a __thiscall prototype with no `this` and no
                # class in the name, so there is nothing to name the class
                # after but this tool's imagination - and an invented class is
                # half of a symbol no target object can hold.
                raise Unsettled("__thiscall prototype has no `this` parameter "
                                "and no class in the name")
            # Drop EVERY parameter naming the receiver, not only the first.
            # Four catalogued prototypes carry `this` beyond position 0, and
            # leaving one in emits `Caviar* this a1` - a literal keyword in the
            # parameter list, which agents were deleting by hand. A prototype
            # that never mentions the receiver keeps all of its parameters:
            # the mangled argument list does not count `this` either.
            self.params = [parameter_type(strip_receiver_token(p))
                           for p in (self.params[1:] if receiver
                                     else self.params)]
            if MANGLED_CTOR.match(self.mangled):
                self.kind, self.method = "ctor", self.klass
            elif MANGLED_DTOR.match(self.mangled):
                self.kind, self.method = "dtor", f"~{self.klass}"
                # A destructor takes no arguments, whatever the catalogue
                # recorded. `??1StringStruct@@QAE@H@Z` carries one, and C++
                # cannot express that: CL writes `@XZ` and the target object
                # holds `@H@Z`, so the two never pair.
                self.params = []
            else:
                found = MANGLED_METHOD.match(self.mangled)
                # Same reasoning as the free-function fallback below: the
                # spelling never reaches a comparison. `??_Gthunk1...` and the
                # other compiler-generated method names are not identifiers.
                self.kind = "method"
                self.method = (found.group("method") if found
                               else f"m_{int(row['address'], 16):08x}")
                if int(row["address"], 16) in renamed_for_collision():
                    # Another row is catalogued under the symbol this name
                    # compresses to. Both sides take the synthesised name, so
                    # the two objects still agree and the subject stops
                    # resolving to somebody else.
                    self.method = f"m_{int(row['address'], 16):08x}"
        else:
            # A `__cdecl`/`__stdcall` parameter may still be NAMED `this`; it
            # is a real argument there, so keep it and drop only the keyword.
            self.params = [parameter_type(strip_receiver_token(p))
                           for p in self.params]
            found = MANGLED_FREE.match(self.mangled)
            self.method = found.group("name") if found else self.mangled
            if not re.match(r"^[A-Za-z_]\w*$", self.method):
                # THE NAME CANNOT AFFECT THE BYTES. Nothing is ever linked -
                # the oracle compiles with `/c` - so the emitted symbol's
                # spelling reaches no comparison; only the convention, the
                # parameters and the return type do. Refusing here cost 60+
                # rows in the first census run, all of them `??__E` dynamic
                # initialisers and `??__F` atexit thunks whose catalogued name
                # is simply not a C identifier.
                self.method = f"fn_{int(row['address'], 16):08x}"

        # THE MANGLED NAME OUTRANKS THE PROTOTYPE'S SPELLING OF A TYPE. The
        # derived prototypes are written in IDA's alphabet on purpose
        # (`derive_prototypes_from_names.SCALAR_SPELLING`, so derived rows parse
        # like recorded ones), and that alphabet collapses types MSVC keeps
        # apart: `char` and `signed char` both become `int8`, `long` and `int`
        # both become `int`. Those mangle differently - D vs C, J vs H - so a
        # body compiled from the IDA spelling emits `PAC` where the target
        # holds `PAD`, and the two objects share no symbol at all. 108 of the
        # 264 unpairable `?`-mangled rows were exactly this and nothing else.
        #
        # Only when the count agrees: `decode_signature` reads scalars and
        # pointers to them, and returns None on a user-defined type, so a
        # disagreement here means it read a different signature than the
        # prototype describes rather than a better spelling of the same one.
        decoded = decode_signature(self.mangled)
        if decoded is not None and len(decoded[1]) == len(self.params):
            self.returns, self.params = decoded[0], list(decoded[1])

        # C or C++ linkage, and with it the symbol both objects must carry.
        # A catalogued name that is not `?`-mangled was never a C++ symbol:
        # emitting it with C++ linkage makes CL invent `?sub_5e3650@@YGHH@Z`,
        # which no target object holds. See tools/recovery_symbols.py.
        constraint = recovery_symbols.spelling(self.mangled)
        self.linkage = "c++" if (self.is_method or constraint.linkage == "c++") \
            else "c"
        if self.linkage == "c++" and constraint.linkage == "c":
            # A `__thiscall` prototype under a name the linker never carried:
            # the unit would be emitted as a class method, and CL would mangle
            # the class and method THIS TOOL invented into a symbol no target
            # object can hold. One row is in this state (`j_??1Ambience...`).
            # Refusing says so; emitting would produce a prompt that cannot
            # match however good the body is.
            raise Unsettled(
                f"__thiscall under the undecorated name {self.mangled!r}: the "
                "emitted C++ symbol would name a synthesised class")
        if self.linkage == "c":
            self.method = constraint.identifier or self.method
            # The decoration outranks the inferred convention: `_WinMain@16`
            # says __stdcall outright, while `prototype_from_ret` only guesses
            # it from a purge byte that __thiscall writes identically.
            if constraint.convention:
                self.convention = constraint.convention
        self.symbol = recovery_symbols.symbol_for(
            self.mangled, int(row["address"], 16), self.convention,
            self.params, self.method) if self.linkage == "c" else \
            recovery_symbols.compress_backrefs(
                # Before compression, so a written-out `PAVBuffer@@` is still
                # there to be re-keyed; what compression leaves behind is a
                # digit that carries no key of its own.
                recovery_symbols.canonicalise_class_keys(
                    recovery_symbols.substitute_name(
                        recovery_symbols.empty_destructor_arguments(
                            self.mangled),
                        self.method.lstrip("~")),
                    self._keys))

    def member_convention(self) -> str:
        """The convention a MEMBER declaration has to spell, or ''.

        `__thiscall` is the default for a member and VC6 rejects the keyword
        outright (C4234), so it is never written. `__cdecl` on a member is not
        the default and must be written, or CL mangles `QAE` where the target
        holds `QAA`.
        """
        return "" if self.convention == "__thiscall" \
            else f"{self.convention} "

    def argument_list(self) -> str:
        return ", ".join(named_parameter(text, f"a{index}")
                         for index, text in enumerate(self.params, start=1)) \
            or "void" if self.params else ""

    def referenced_types(self) -> set:
        out = set()
        for text in [self.returns] + self.params:
            out |= named_types(text)
        out.discard(self.klass)
        return out


# ------------------------------------------------------------------ emission

def unsettled_identifier(row: dict) -> str:
    """What a callee whose signature will not settle is called in the source.

    Shared with `emit_target_object`, which has to write the same name into the
    relocation - a declaration and a relocation that spell the same function
    differently is the failure this whole path exists to avoid.
    """
    return recovery_symbols.fallback_identifier(
        row.get("name") or "", int(row["address"], 16))


def crt_declaration(method: str) -> str:
    """`CRT_SIGNATURES` for this callee, spelled with the name it is CALLED by.

    The table is keyed by the DECORATED name - `_free`, `_read` - because that
    is what the image's import carries. The emitter prints the undecorated one,
    `free`, so a lookup on the printed name misses every entry that has a
    leading underscore. That is not a near-miss: `free` then stays declared
    nullary, and a body calling it properly fails `C2660` while a body
    declaring it fails `C2733`. An agent hit exactly this on
    `StringList::kill_entry` and worked around it by casting `&free` through a
    function-pointer type - a correct answer to the wrong problem.

    The name in the returned text is rewritten to whatever the emitter is
    actually printing, because a declaration of `_free` does not declare the
    `free` the call site names.
    """
    for key in (method, f"_{method}"):
        text = CRT_SIGNATURES.get(key)
        if text is None:
            continue
        head, _, tail = text.partition("(")
        return f"{head.rsplit(key, 1)[0]}{method}({tail}"
    return ""


def declare_callee(row: dict, derived: dict, pe=None,
                   keys: dict = None) -> str:
    """A callee as a bare declaration. Never a definition.

    A definition would be INLINED, and the original did not inline it, so the
    emitted unit must never contain one. `byte_match.object_code` refuses a unit
    holding two external `.text` symbols for the same reason.
    """
    try:
        signature = Signature(row, derived, pe, keys)
    except Unsettled:
        # 30% of callee targets have no prototype. A declaration with unknown
        # arity is still better than none: the body will not compile without
        # something to call, and a wrong arity shows up as a mismatch rather
        # than as a silent pass.
        name = unsettled_identifier(row)
        if name in recovery_symbols.COMPILER_DECLARED:
            return ""
        # A CRT routine whose arity IS known, just not from the catalogue.
        # These are not `?`-mangled and appear in no prototype source, so
        # `Signature` cannot decode them and they fell to the nullary
        # declaration below - which does not merely lose information, it makes
        # every caller uncompilable: `C2660: does not take 3 parameters` if the
        # body calls it properly, and `C2733: second C linkage` if the body
        # tries to declare it instead. `Caviar::vx_read` and `vx_write` are
        # thin forwarders onto `_read`/`_write` and were blocked on nothing
        # else.
        crt = CRT_SIGNATURES.get(name)
        if crt:
            return f'extern "C" {crt};'
        return f"extern \"C\" int {name}();  // arity unknown"
    if signature.is_method:
        return ""      # emitted with its class
    if signature.linkage == "c" and \
            signature.method in recovery_symbols.COMPILER_DECLARED:
        # CL declares this one itself. A second C-linkage declaration is
        # `error C2733`, and it took the whole unit down with it.
        return ""
    # `extern "C"` is what makes the call site reference `_sub_5e3650@8` rather
    # than a C++ mangling of it, which is the name the target object's
    # relocation carries. Without it every call to an undecorated function
    # reads as a diff in a body that is otherwise byte-exact.
    # A CRT routine the catalogue SETTLED as nullary. `__read` carries no
    # prototype at all, so `Signature` reads its mangled-looking name, finds
    # no parameters to decode, and emits `_read()` - settled, and wrong. That
    # is worse than the unsettled case, which at least says `arity unknown`:
    # a caller then fails `C2660: does not take 3 parameters`, and declaring
    # it in the body instead fails `C2733: second C linkage`, so the function
    # is unreachable for a reason that has nothing to do with its own bytes.
    #
    # Only an EMPTY parameter list is overridden. A catalogue that has decoded
    # real parameters knows something this table does not, and outranks it.
    crt = crt_declaration(signature.method)
    if crt and not signature.params:
        return f'extern "C" {crt};'
    linkage = "extern \"C\" " if signature.linkage == "c" else ""
    return (f"{linkage}{signature.returns} {signature.convention} "
            f"{signature.method}({', '.join(signature.params) or ''});")


# `Buffer buffer_;` / `Spot spot_[4];` - a declarator whose type is a bare
# identifier. Leading `unsigned`/`const` and the primitives are not classes
# and impose no ordering, so an initial keyword disqualifies the line.
BY_VALUE_MEMBER = re.compile(
    r"^(?!return|const|static|unsigned|signed|struct|class|enum|typedef)"
    r"([A-Z]\w*)\s+\w+\s*(?:\[[^\]]*\])?\s*;$")


def layout_dependencies(name) -> list:
    """By-value member types in the text that will actually be emitted.

    Module level rather than nested inside `by_value_first`, because the
    Module level rather than nested inside `by_value_first`, so the same
    edges can be read by anything that needs them - `src_declarations` emits
    layouts of its own and has to declare what they name.
    """
    # A BASE MUST BE COMPLETE at the derived definition, exactly as a
    # by-value member must, and once the shell carries a base clause the
    # base's members are no longer inlined into it - so the edge that used
    # to arrive through the flattened member list has to arrive here
    # instead, or the derived class is emitted above its own base.
    found = list(class_layouts.layout_bases(name))
    bases = set(found)
    for line in (class_layouts.own_declaration_for(name) if bases
                 else class_layouts.declaration_for(name)) or ():
        stripped = line.strip()
        # `Type name_;` and `Type name_[N];` need Type COMPLETE.
        # `Type *name_;` does not - the forward declaration above is
        # enough - and a function-pointer member is not a member type.
        if "*" in stripped or "(" in stripped:
            continue
        match = BY_VALUE_MEMBER.match(stripped)
        if match:
            found.append(match.group(1))
    for type_, _, _ in class_layouts.pinned_layouts().get(name, ()):
        if "*" not in type_:
            found.append(type_.replace("const", "").strip())
    return found

def by_value_first(names) -> list:
    """Order class definitions so a by-value member's type comes first.

    `class Dialog { Heap heap_; }` needs `Heap` COMPLETE, and the classes
    were emitted alphabetically - Dialog at line 72, Heap at 143 - so 77
    units failed with `error C2079: uses undefined class`. A pointer member
    needs only the forward declaration above and imposes no order.

    Stable: ties keep alphabetical order, so a unit does not reshuffle
    between runs. A cycle cannot happen - a class cannot contain itself by
    value - but is broken defensively rather than recursed into.

    THE DEPENDENCIES ARE READ OFF WHAT WILL BE EMITTED, not off
    `pinned_layouts()`. Those are two different sources - the body emitted
    below comes from `class_layouts.declaration_for()` - and a member present
    in the first and absent from the second is a dependency this sort cannot
    see. It then ordered the classes confidently and wrongly: 27% of staged
    units still failed with `C2079: uses undefined class` (Spot 21, Buffer 19,
    Heap 15, Time 6 in a 150-unit sample) against the one function whose whole
    job is preventing that. A sort and an emitter that disagree about what a
    class contains cannot be reconciled by making the sort cleverer; they have
    to read the same text.
    """
    ordered, placed = [], set()
    dependencies = layout_dependencies

    def place(name, seen):
        if name in placed or name in seen:
            return
        for dependency in dependencies(name):
            if dependency in names:
                place(dependency, seen | {name})
        if name not in placed:
            placed.add(name)
            ordered.append(name)

    for name in sorted(names):
        place(name, frozenset())
    return ordered


# `static int *const g_0096cac4 = (int *)0x0096CAC4;` - the scaffold's line
# for one fixed global.
SCAFFOLD_GLOBAL = re.compile(r"^static \w+ \*const (g_[0-9a-f]{8}) = .*;$",
                             re.M)


def without_globals_the_body_declares(scaffolding: str, body: str) -> str:
    """Drop scaffold globals the BODY already declares for itself.

    THE BODY'S SPELLING WINS, because the body's spelling is what was
    measured. `src/recovered/00539510.cpp` says `extern int g_0096cac4[];` on
    purpose - the address does index work, and an array declaration is how
    this tree writes that - and it scored BYTE_EXACT with that line. The
    scaffold's own form is a const pointer, so emitting both is
    `C2373: redefinition; different type modifiers` and the unit dies.

    This became reachable on 2026-08-14 when `absolute_operands` learned to
    read `[ecx + 0x94CABC]`, which is the register-indexed global the array
    spelling exists for: three claimed-BYTE_EXACT bodies stopped compiling in
    the same commit that fixed the emitter's blindness to them. Dropping the
    scaffold's line rather than the body's keeps the measurement honest -
    nothing the body says is rewritten, and a body that does NOT declare the
    global still gets the scaffold's.

    A USE IS NOT A DECLARATION. The pattern requires `extern` or `static` on
    the same statement, so a body that merely reads `g_0096cac4` keeps the
    declaration it depends on.
    """
    declared = {name for name in SCAFFOLD_GLOBAL.findall(scaffolding)
                if re.search(rf"^[^/\n]*\b(?:extern|static)\b[^;\n]*"
                             rf"\b{name}\b", body, re.M)}
    if not declared:
        return scaffolding
    return "\n".join(
        line for line in scaffolding.splitlines()
        if not (SCAFFOLD_GLOBAL.match(line)
                and SCAFFOLD_GLOBAL.match(line).group(1) in declared)) + "\n"


def static_for(entry) -> str:
    """`static ` for a CALLEE whose mangling says it takes no receiver.

    `QAA` is a public member with `__cdecl` - no `this` in ecx, every argument
    on the stack - so the call site is indistinguishable from a static one and
    that is how a body has to spell it: `Palette::set_active_window(...)`.
    Declared non-static, that spelling is `C2352: illegal call of non-static
    member function`, and there is no legal alternative because there is no
    object to call it on.

    THREE AGENTS REPORTED THIS ACROSS TWO BATCHES, each working around it by
    redeclaring the method as a free function under a different name -
    `Win::init_class`, `Cursor::init_cursor_class`, `Palette::set_active_window`
    and every other `*::init_class()`. One of them said it plainly: fix it once
    in the emitter rather than per body.

    ONLY FOR A CALLEE. `static` changes the mangling from `QAA` to `SA`, and
    for the SUBJECT that is the symbol the comparison looks up - the reason
    `member_convention` spells `__cdecl` at all. A callee is reached by a
    relocation the comparison masks, so its mangling reaches nothing.
    """
    return "static " if entry.is_method and entry.convention == "__cdecl" \
        else ""


# `class Foo {` / `struct Foo : Bar {` at column zero - a DEFINITION, not a
# forward declaration.
TOP_LEVEL_DEFINITION = re.compile(
    r"^(?:class|struct)\s+(\w+)\s*(?::[^{;]*)?\{", re.M)


def classes_defined_in(text: str) -> set:
    """Class names `text` DEFINES, ignoring comments and string literals."""
    stripped = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    stripped = re.sub(r"//[^\n]*", " ", stripped)
    return set(TOP_LEVEL_DEFINITION.findall(stripped))


def without_classes_the_body_defines(scaffolding: str, body: str) -> str:
    """Drop scaffold class DEFINITIONS the body defines for itself.

    A BODY THAT DECLARES ITS OWN BASE IS DOING THE RIGHT THING. 0x0062C010 is
    BYTE_EXACT precisely because it writes `class BaseButton { public:
    ~BaseButton(); };` and calls through it - a declared, never-defined
    destructor becomes the `jmp` the original has. When the scaffold also
    emits BaseButton, that is `C2011: 'BaseButton' : 'class' type
    redefinition` and the whole unit dies.

    Measured 2026-08-14: this is the mechanism behind the base-clause
    experiment that broke 112 claimed-byte-exact bodies, which I had recorded
    as COMDAT folding on the strength of an error string rather than a
    compile. It is not about destructors or folding. It is two declarations of
    one name.

    THE DEFINITION GOES, THE NAME STAYS. A forward declaration is left behind
    so scaffold code above that only points at the type still compiles; code
    needing it COMPLETE will fail - but it fails today too, with the whole
    unit, so this is strictly better and never worse.
    """
    defined = classes_defined_in(body) & classes_defined_in(scaffolding)
    if not defined:
        return scaffolding
    out, skipping = [], None
    for line in scaffolding.splitlines():
        if skipping is None:
            hit = TOP_LEVEL_DEFINITION.match(line)
            if hit and hit.group(1) in defined:
                skipping = hit.group(1)
                keyword = line.split()[0]
                out.append(f"{keyword} {skipping};   "
                           f"// defined by the body below")
                continue
            out.append(line)
        elif line.startswith("};"):
            skipping = None
    return "\n".join(out) + "\n"


def imported_methods(name: str, catalogued: set) -> list:
    """`src/`'s declarations for `name`'s methods, minus the catalogued ones.

    A CATALOGUED METHOD OUTRANKS THE HEADER'S, always and by name rather than
    by signature. The catalogue's version is decoded from the mangled symbol
    the linker wrote, so it is what the image really has; the header is what
    this project has reconstructed, and the two disagree often enough to
    matter. Emitting both is `C2556` - overloads differing only in return type
    - which takes the whole unit down to cure a single missing name.

    Skipping by NAME rather than by full signature also covers the overload
    case in the safe direction: the catalogue's set is complete for any name
    it knows at all, so nothing is lost, and a header overload of a
    catalogued name can never contradict one that is already emitted.
    """
    return [text for text, method, _ in class_layouts.methods_of(name)
            if method not in catalogued]



# The CRT string routines the shipped image CALLS rather than inlines, with
# the count of direct `E8` sites behind each. `/O2` implies `/Oi`, which
# expands all four inline; the image has none of those expansions.
STRING_ROUTINES = ("strcat", "strcpy", "strlen", "strcmp")


def string_routine_pragma(declarations: list) -> list:
    """`#pragma function(...)` for the string routines this unit declares.

    WHY THE IMAGE NEEDS IT. `_strcat` has 4,332 direct call sites in .text,
    `_strlen` 875, `_strcpy` 339, `_strcmp` 155 - a program compiled with
    `/Oi` throughout would have almost none. But 19 BYTE_EXACT bodies carry a
    `rep stosd` or `rep movsd` that ONLY `/Oi` produces, and 34 functions call
    `strcat` and inline a block op in the SAME body. No per-unit compiler flag
    can produce that; a pragma naming the four string routines and leaving the
    block ones alone produces exactly it. `/O2 /Oi-` was tried as two extra
    flag sets first and reverted - see byte_match.FLAG_SETS.

    AFTER THE DECLARATIONS, WHICH IS THE ENTIRE TRICK. `#pragma function` on a
    name nothing has declared yet is SILENTLY IGNORED - no C4163, no warning
    at any level, identical bytes. Measured on 0x00634E80
    (`FileBox::set_def_ext`, which calls strcat and strlen): the pragma at the
    top of the unit leaves it MISMATCH at 0.4872 similarity, and the same
    pragma one line below the `extern "C"` declarations makes it BYTE_EXACT.

    ONLY WHAT THIS UNIT DECLARES, so a scaffold that never mentions `strcmp`
    does not name it - `#pragma function` on a routine with no declaration in
    scope is the ignored case above, and emitting four names where one is
    used makes the working ones look like the ignored ones.
    """
    named = [routine for routine in STRING_ROUTINES
             if any(f" {routine}(" in text or f"*{routine}(" in text
                    for text in declarations if text)]
    if not named:
        return []
    return [f"#pragma function({', '.join(named)})", ""]


def emit(address: int, functions: dict, derived: dict, callees: dict,
         pe, scaffolding_only: bool = False, body: str = "") -> str:
    """The unit. With `scaffolding_only`, everything EXCEPT the subject's
    definition, so a caller can append a body it already has - which is what
    the census does with the 2,518 bodies already committed under `src/`."""
    row = functions.get(address)
    if row is None:
        raise Unsettled(f"0x{address:08X} is not a catalogued function")
    keys = class_keys(functions)
    # Classes the BODY defines for itself, so this scaffold neither defines
    # nor inherits from them. Empty for a bare scaffold.
    body_defines = classes_defined_in(body) if body else set()

    def declare(name: str, opening: bool) -> str:
        # `class X { public:` and `struct X {` differ only in default access,
        # and the key has to agree between the forward declaration and the
        # definition or VC6 warns C4099 - but it also has to agree with the
        # TARGET object, because `U` and `V` mangle differently. Both sides
        # read `class_keys`, which is what makes that true by construction.
        #
        # THE BASE CLAUSE IS EMITTED when `src/` declares one and every base
        # in the chain can be supplied. Flattening is still the fallback and
        # still produces the same layout; what it cannot produce is a body
        # that NAMES a base - `SubInterface::set_iface_mode()` in
        # SocialWin::show - because in a flat class there is no such base.
        # `public` is spelled explicitly: the original's access specifier
        # changes no offset, and the emitted unit needs the conversion to be
        # legal so a body may pass `this` where a base pointer is expected.
        key = keys.get(name, "struct")
        if not opening:
            return f"{key} {name};"
        bases = emitted_bases(name)
        clause = (" : " + ", ".join(f"public {base}" for base in bases)) \
            if bases else ""
        return f"{key} {name}{clause} {{ public:" if key == "class" else \
            f"{key} {name}{clause} {{"

    def emitted_bases(name: str) -> list:
        """The bases to spell for `name`, or [] to emit it flat.

        A class that derives from the SUBJECT is emitted FLAT, because the
        subject's own definition is written last - its methods go with it - so
        a derived shell above it would name a base that is not defined yet:
        `C2504: 'Win' : base class undefined`, on 41 rows the first time
        inheritance was emitted. The layout is identical either way, and the
        base clause only matters to a body that NAMES the base - which, when
        the subject is the base, is the subject's own body and needs nothing
        from the derived shell.
        """
        if name != signature.klass and \
                signature.klass in class_layouts.base_chain(name):
            return []
        bases = class_layouts.layout_bases(name)
        # A BASE THE BODY DEFINES IS NOT A BASE THIS SCAFFOLD MAY NAME.
        # `without_classes_the_body_defines` removes the colliding DEFINITION
        # and leaves a forward declaration, which turns `C2011: type
        # redefinition` into `C2504: base class undefined` for any shell that
        # inherits it - reproduced on 0x0062C010 with `sizeof(PushButton)`
        # pinned. The flat form carries the base's members, so the layout is
        # unchanged and only the spelling is.
        if bases and body_defines & set(bases):
            return []
        return bases

    def embeds_the_subject(name: str) -> bool:
        """Does `name` hold the SUBJECT's class by value?

        The subject's own class is emitted LAST, because its methods go with
        its definition. `emitted_bases` already handles a class that DERIVES
        from the subject - it emits it flat, since a base clause would name a
        type not yet defined. A by-value MEMBER needs the identical treatment
        and was not getting it, so the shell was emitted with a member of an
        incomplete type: `C2079`, before the body is even reached, and no
        spelling inside the body can route around it.

        MEASURED 2026-08-14. `Scroll` holds `FlatButton flat_button_left_` and
        `flat_button_right_` by value, so every unit whose subject is a
        `FlatButton` method emits `Scroll` above the still-undefined
        `FlatButton`. Reproduced at 0x00607CF0 (`??0FlatButton@@QAE@XZ`) and
        0x00608660 (`?on_mouse_leave@FlatButton@@QAEXHH@Z`), in both cases
        with a TRIVIAL EMPTY BODY, which is what proves it is the scaffold and
        not the recovery. Two agents hit it independently; one moved the class
        by hand and the address then went BYTE_EXACT, so this was costing
        recoveries outright rather than merely wasting time.

        `by_value_first` cannot fix it: it orders the classes it is GIVEN, and
        the subject's class is deliberately excluded from that set.
        """
        if name == signature.klass:
            return False
        for line in (class_layouts.declaration_for(name) or ()):
            stripped = line.strip()
            if "*" in stripped or "(" in stripped:
                continue          # a pointer or function-pointer member is fine
            match = BY_VALUE_MEMBER.match(stripped)
            if match and match.group(1) == signature.klass:
                return True
        return False

    def members_of_shell(name: str) -> list:
        """The member lines to put inside `name`'s shell.

        OWN members when the shell carries a base clause, because the base
        supplies its own; the flattened chain otherwise. Reading these two
        from different places is how a class ends up missing sizeof(base)
        bytes at the front while still compiling.

        A shell that would hold the SUBJECT by value goes back to being opaque:
        the subject is defined below it, so the member cannot be complete here.
        Emitting nothing loses only the field NAMES - a body reaches those
        fields by offset, exactly as it does for every other opaque class - and
        it is the difference between a unit that compiles and one that cannot.
        """
        if embeds_the_subject(name):
            return []
        if emitted_bases(name):
            return class_layouts.own_declaration_for(name)
        flat = class_layouts.declaration_for(name)
        # FLAT AND SHADOWED IS NOT EMITTABLE. `Buffer buffer_` inherited from
        # GraphicWin beside Scroll's own `Buffer *buffer_` is
        # `C2040: differs in levels of indirection`, and PullDown's `menu_`
        # over Win's is `C2086: redefinition`. Under a base clause they are in
        # different scopes and both compile; without one the class has to go
        # back to an opaque shell, which is exactly where it was before
        # inheritance could be emitted at all.
        spelled = [line.rsplit(" ", 1)[-1] for line in flat]
        if len(spelled) != len(set(spelled)):
            return []
        return flat

    signature = Signature(row, derived, pe, keys)
    refusal = recovery_symbols.UNDEFINABLE.get(signature.method) \
        if signature.linkage == "c" else None
    if refusal:
        raise Unsettled(f"{signature.method} cannot be defined: {refusal}")

    spans = parse_body_ranges(row.get("body_ranges") or "")
    globals_touched = absolute_operands(pe, spans) if spans else {}

    lines = [
        "// GENERATED SKELETON - tools/emit_translation_unit.py",
        f"// subject: {row['name']}  at 0x{address:08X}  "
        f"({row.get('size', '?')} bytes)",
        "//",
        "// A VERIFICATION ARTIFACT, not product source: classes are opaque and",
        "// globals are bound to fixed addresses, because both are byte-visible",
        "// and both differ from the style src/ is written in.",
        "//",
        "// The VC6 dialect limits and the source-form rules used to live here.",
        "// They are knowledge, not scaffolding, so they now live in the agent",
        "// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),",
        "// where they can be edited without regenerating anything and are in",
        "// context from the first token rather than behind a file read. This",
        "// emitter computes declarations; it does not carry lessons.",
        "",
        PRELUDE.rstrip(),
        seam_header().rstrip(),
        game_constants().rstrip(),
        "",
    ]

    # Callee declarations.
    declarations, methods_by_class, free_callees = [], {}, []
    for target in callees.get(address, []):
        callee = functions.get(target)
        if callee is None or target == address:
            continue
        try:
            callee_signature = Signature(callee, derived, pe, keys)
        except Unsettled:
            declarations.append(declare_callee(callee, derived, pe, keys))
            continue
        if callee_signature.is_method:
            methods_by_class.setdefault(callee_signature.klass, []).append(
                callee_signature)
        else:
            free_callees.append(callee_signature)
            declarations.append(declare_callee(callee, derived, pe, keys))

    # Forward declarations for every non-builtin type named ANYWHERE in this
    # unit, not just in the subject's own signature. Scoping this to the
    # subject was worth 38% compile rate: a callee taking `RECT *` is declared
    # before anything declares `RECT`, and CL stops at the first one.
    wanted = set(signature.referenced_types())
    if signature.is_method:
        # `referenced_types` drops the subject's own class, which is right for
        # deciding what to DECLARE and wrong for deciding what to forward-
        # declare: a zero-argument method like `?update_zorder@Win@@QAAXXZ`
        # never mentions `Win` in its own signature, while a callee declaration
        # above it does.
        wanted.add(signature.klass)
    for entries in methods_by_class.values():
        for entry in entries:
            wanted |= entry.referenced_types()
            wanted.add(entry.klass)
    # A FREE callee's types too. This loop had the method callees and the
    # subject and not these, so `int in_box(int, int, RECT *);` was emitted
    # above anything that declares `RECT` - 103 of the 154 units that would
    # not compile, all of them a C2061 or a C2065 on the first such line. The
    # comment above already claimed "anywhere in this unit"; now it is true.
    for entry in free_callees:
        wanted |= entry.referenced_types()
    # A supplied layout names types of its own - `StringStructEntry *head_` -
    # and they have to be declared before the class that holds them. To a
    # FIXPOINT: a layout supplied for a type that is itself only pointed at
    # brings its own references, and stopping after one round left
    # `StringStructEntry` undeclared inside `StringList`'s layout - nine
    # units, and the sweep is the only thing that saw it.
    for klass in [signature.klass] + list(methods_by_class):
        wanted |= class_layouts.referenced_types(klass)
    while True:
        grown = set(wanted)
        for name in list(wanted):
            grown |= class_layouts.referenced_types(name)
            # A method IMPORTED from `src/` names types too, and it is emitted
            # inside the shell - above every class that follows it. `HDC` in
            # `Buffer::get_hdc` is the shape: without this the import cures a
            # C2039 and buys a C2061 one line higher up.
            grown |= class_layouts.method_types(name)
        grown -= BUILTIN
        if grown == wanted:
            break
        wanted = grown
    # EVERYTHING referenced is forward-declared, including classes defined
    # further down. `struct X;` before `struct X { ... };` is legal and is what
    # keeps a callee declaration that takes `Win *` from preceding the
    # definition of `Win` - which it does whenever the SUBJECT owns that class,
    # and which used to be a C2065 the moment `Win`'s methods started being
    # recognised as methods. Excluding them was a guard against C4099, the
    # class/struct-key mismatch warning, and that cannot fire now that both the
    # forward declaration and the definition say `struct`.
    for name in sorted(wanted):
        spelling = NOT_A_STRUCT.get(name)
        lines.append(f"typedef {named_parameter(spelling, name)};" if spelling
                     else declare(name, opening=False))
    if wanted:
        lines.append("")

    # A type a supplied layout POINTS AT gets its layout too, when `src/`
    # proves one. Without this the agent has `StringStructEntry *current_`
    # and an incomplete type behind it, so it can reach the field only as
    # `reinterpret_cast<int *>(current_)[1]` - which is what the first
    # recovery of the run actually wrote, next to a perfectly good `head_`.
    #
    # ONE ORDERED PASS, NOT TWO. This used to emit layout-only classes first
    # and classes with called methods afterwards, under a comment asserting
    # that "layouts never hold a member by value, so a forward declaration is
    # enough". That stopped being true. A class holding `Spot spot_` by value
    # got emitted in the first pass while `Spot` - which also had a method
    # called, so it was deferred - was still only forward-declared, and the
    # unit failed with `C2079: uses undefined class` before the body was even
    # reached. 27% of staged units were unrecoverable for this reason, none of
    # it visible to an agent, who sees a scaffold that cannot compile and no
    # way to fix it from inside the body.
    #
    # Splitting by "does it have called methods" was never a dependency
    # property, so no ordering within the two passes could fix it. Every class
    # is now emitted exactly once, with its layout AND its called methods, in
    # by_value_first order across the whole set.
    defined_later = {signature.klass}
    definable = (wanted | set(methods_by_class)) - defined_later
    if declarations or methods_by_class:
        lines.append("// ---- callees, declared and never defined "
                     "(a definition would be inlined) ----")
        # WHY TWO THINGS BELOW LOOK WRONG AND ARE NOT. An agent recovering
        # 0x0048C650 "fixed" both of them, and one of the fixes would have
        # broken the pairing it was trying to restore. They are explained
        # HERE because this is what an agent reads; the emitter's docstrings
        # are not in the brief.
        lines.append("//")
        lines.append("// `static` ON A CALLEE IS DELIBERATE. A method whose")
        lines.append("// mangled infix is `QAA` or `QAG` takes NO `this` -")
        lines.append("// every argument is on the stack - so the call site is")
        lines.append("// `Class::method(...)` with no object, and declared")
        lines.append("// non-static that spelling is `C2352: illegal call of")
        lines.append("// non-static member function`. It does change the")
        lines.append("// mangling from QAA to SA, which matters only for the")
        lines.append("// SUBJECT; a callee is reached by a relocation the")
        lines.append("// comparison masks, so its mangling reaches nothing.")
        lines.append("//")
        lines.append("// THE class/struct KEY IS NOT A GUESS EITHER, and must")
        lines.append("// not be `corrected` against the catalogue. MSVC")
        lines.append("// mangles struct `U` and class `V`, six classes")
        lines.append("// disagree with THEMSELVES in the catalogue, and the")
        lines.append("// image carries no RTTI to settle it. Both objects are")
        lines.append("// ours: `recovery_symbols.canonicalise_class_keys`")
        lines.append("// rewrites the TARGET object with the same map this")
        lines.append("// unit uses, so they agree by construction. Changing")
        lines.append("// one side alone is what breaks it.")
    for name in by_value_first(definable):
        layout = members_of_shell(name)
        body = []
        for entry in methods_by_class.get(name, ()):
            if entry.kind == "ctor":
                body.append(f"    {name}({', '.join(entry.params)});")
            elif entry.kind == "dtor":
                body.append(f"    ~{name}();")
            else:
                body.append(f"    {static_for(entry)}{entry.returns} "
                            f"{entry.member_convention()}"
                            f"{entry.method}({', '.join(entry.params)});")
        if not layout and not body:
            # STILL OPAQUE, and it must stay that way: a class emitted with
            # nothing but imported methods has `sizeof` 1, and a body holding
            # one by value would then compile against a wrong size instead of
            # failing. An incomplete type fails loudly, which is correct.
            continue
        body.extend(imported_methods(name, {e.method for e in
                                            methods_by_class.get(name, ())}))
        lines.append(declare(name, opening=True))
        lines.extend(layout or [])
        lines.extend(sorted(set(body)))
        lines.append("};")
        lines.append("")
    for text in sorted(set(d for d in declarations if d)):
        lines.append(text)
    if declarations or methods_by_class:
        lines.append("")
    lines.extend(string_routine_pragma(declarations))

    # A vtable shim, where the body dispatches indirectly.
    slots, over_cap = vtable_slots(pe, spans) if spans else ([], [])
    if slots:
        lines.append(vtable_shim(slots))
        lines.append("")
    # AND THE OTHER SHAPE, where the interface is pushed rather than passed in
    # ecx. Emitted beside `VCall` rather than instead of it: a slot vanishing
    # from that class is a body that stops compiling.
    pushed = com_slots(pe, spans) if spans else []
    if pushed:
        lines.append(com_shim(pushed))
        lines.append("")
    if over_cap:
        # Named, never silently dropped: a slot index this large is almost
        # certainly a function-pointer FIELD rather than a vtable.
        lines.append(f"// NOTE: this body also dispatches through offset(s) "
                     f"{', '.join(hex(s * 4) for s in over_cap)}, which are "
                     f"past the vtable cap")
        lines.append("// and are probably function-pointer FIELDS, not virtual "
                     "calls. Spell those yourself.")
        lines.append("")

    # Fixed globals the original touches.
    if globals_touched:
        lines.append("// ---- fixed globals this body references ----")
        lines.append("// The const-pointer spelling reproduces the original's")
        lines.append("// encoding including the address; `extern T *g` does not.")
        for value in sorted(globals_touched):
            lines.append(f"static int *const g_{value:08x} = "
                         f"(int *)0x{value:08X};")
        lines.append("")

    # The subject.
    own = methods_by_class.get(signature.klass, [])
    if signature.is_method:
        # `struct`, not `class`: MSVC encodes the two differently in a mangled
        # name - `PAUSprite@@` against `PAVSprite@@` - and the catalogue holds
        # `U` over `V` by 503 to 61. Emitting `class` made every function
        # taking one of these as a parameter mangle to a name no target object
        # holds. It also settles the C4099 the forward declarations above
        # warned about, since those already say `struct`.
        lines.append(declare(signature.klass, opening=True))
        # THE REAL MEMBERS, when `src/` declares a layout that has been proved
        # to compile to the real class's size (tools/verify_class_layouts.py).
        # Without them the class is an opaque shell, and an agent that needs a
        # field reaches it by offset - or declares a shadow struct beside the
        # function, which the writeback cannot carry into `src/` because it
        # splices the definition alone. Measured on the first real run: 4 of
        # 10 writebacks lost that way, and the ones that landed wrote
        # `self[2]` into a file that already said `head_`.
        layout = members_of_shell(signature.klass)
        if not layout:
            # NO PINNED LAYOUT, but a body that was proved byte-identical may
            # already have reached into this class, and where it did the
            # offset is not a hypothesis - the comparison would have caught it.
            #
            # The pinned gate exists so a WRONG offset cannot reach an agent.
            # A proved one cannot be wrong, which is why it is admitted here
            # and nowhere else: this is still not a proved SIZE, so it does not
            # go near `pinned_layouts()` or `verified-layouts.txt`.
            #
            # What it buys is the difference between
            #     *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2144) = a1;
            # and
            #     field_2144_ = a1;
            # in every future recovery on the class. The casts in recovered
            # code were never a style problem; they are this file's gaps.
            layout = proved_member_declaration(
                signature.klass, declared_before(lines))
        if layout:
            lines.extend(layout)
            lines.append("")
        seen = set()
        for entry in own:
            # Skip the SUBJECT, whose declaration is written below with its
            # definition - but skip only the subject. Comparing bare names
            # dropped every same-name OVERLOAD too, and an overload is a
            # different function: 98 (class, method) pairs in the catalogue
            # have more than one, covering 252 functions, and each of them
            # disappeared from the class body whenever a sibling was the
            # subject. A body calling that sibling then cannot compile, and
            # the failure looks like a bad body rather than a missing
            # declaration - `Buffer::wrap_height_flying` delegates to its own
            # one-argument overload and had to route around it through an
            # unrelated same-layout class.
            if (entry.method == signature.method
                    and entry.params == signature.params
                    and entry.kind == signature.kind):
                continue
            text = (f"    {static_for(entry)}{entry.returns} "
                    f"{entry.member_convention()}"
                    f"{entry.method}({', '.join(entry.params)});")
            if entry.kind == "ctor":
                text = f"    {signature.klass}({', '.join(entry.params)});"
            elif entry.kind == "dtor":
                text = f"    ~{signature.klass}();"
            if text not in seen:
                seen.add(text)
                lines.append(text)
        lines.extend(imported_methods(
            signature.klass,
            {e.method for e in own} | {signature.method}))
        if signature.kind == "ctor":
            lines.append(f"    {signature.klass}({', '.join(signature.params)});")
        elif signature.kind == "dtor":
            lines.append(f"    ~{signature.klass}();")
        else:
            lines.append(f"    {signature.returns} "
                         f"{signature.member_convention()}{signature.method}"
                         f"({', '.join(signature.params)});")
        lines.append("};")
        lines.append("")
        if scaffolding_only:
            return "\n".join(lines)
        arguments = signature.argument_list()
        if signature.kind == "ctor":
            head = f"{signature.klass}::{signature.klass}({arguments})"
        elif signature.kind == "dtor":
            head = f"{signature.klass}::~{signature.klass}()"
        else:
            head = (f"{signature.returns} {signature.member_convention()}"
                    f"{signature.klass}::{signature.method}({arguments})")
    else:
        if scaffolding_only:
            return "\n".join(lines)
        arguments = signature.argument_list()
        # `extern "C"` on a name the disassembler invented is not cosmetic: it
        # is what makes CL emit the symbol the target object carries. Dropping
        # it leaves the two objects with no name in common and objdiff reports
        # a missing symbol rather than a diff. See tools/recovery_symbols.py.
        linkage = "extern \"C\" " if signature.linkage == "c" else ""
        head = (f"{linkage}{signature.returns} {signature.convention} "
                f"{signature.method}({arguments})")

    # A placeholder return, so the SKELETON COMPILES BEFORE THE BODY EXISTS.
    # Without it VC6 raises C4716 "must return a value" on every non-void
    # subject - 11 of the first 48 failures measured - and "did it compile"
    # stops being a usable signal for whether the scaffolding is right, which
    # is the only question this tool is asked.
    # The `*` is stripped only to recognise a scalar BY NAME below. Testing
    # for void on the stripped text made `void *` read as `void`, so the 70
    # scalar deleting destructors - `??_GAlphaMovie@@UAEPAXI@Z` returns one -
    # got no return and no scaffold that compiled.
    returns = signature.returns.replace("*", "").strip()
    if signature.kind in ("ctor", "dtor") or signature.returns.strip() == "void":
        placeholder = []
    elif "*" in signature.returns:
        placeholder = ["    return 0;  // PLACEHOLDER - replace with the body"]
    elif returns in ("float", "double"):
        placeholder = ["    return 0;  // PLACEHOLDER - replace with the body"]
    else:
        placeholder = [f"    return ({signature.returns})0;"
                       f"  // PLACEHOLDER - replace with the body"]

    lines += [
        head + " {",
        "    // BODY GOES HERE.",
        "    //",
        "    // Reach fields by offset - the class is deliberately empty:",
        "    //     char *self = reinterpret_cast<char *>(this);",
        "    //     int v = *reinterpret_cast<int *>(self + 0x24);",
        "",
    ] + placeholder + [
        "}",
        "",
    ]
    return "\n".join(lines)


# CL names the type it could not resolve directly in these two.
UNDECLARED = (
    re.compile(r"error C2061: syntax error : identifier '(\w+)'"),
    re.compile(r"error C2065: '(\w+)' : undeclared identifier"),
)

# C2146 names the VARIABLE, not the type - `missing ';' before identifier 'bar'`
# for `Foo *bar`. Guessing which token on the line was meant to be a type would
# be inference, but CL also gives the LINE, so the file itself can be asked
# instead. Everything on that line which is not a builtin, not the named
# variable, and not already declared is a candidate. That is still mechanical:
# a wrong guess forward-declares an unused struct, which changes no emitted
# byte, while a missed one leaves the unit uncompilable and visible.
#
# The punctuation it names is NOT always ';'. In a parameter list it is ','
# ("missing ',' before identifier 'a1'"), and a regex pinned to ';' matched
# nothing at all - the repair silently did nothing and the compile rate came
# back byte-identical to the run before it. Identical output is what exposed
# it; treating that as "the change did not help" would have buried a dead code
# path as a measurement.
MISSING_SEMICOLON = re.compile(
    r"\((\d+)\) : error C2146: syntax error : missing '[;,]' "
    r"before identifier '(\w+)'")

KEYWORDS = {"struct", "class", "extern", "public", "private", "static",
            "const", "return", "typedef", "this", "operator", "C", "union",
            "enum", "inline", "virtual", "namespace", "using"}


def repair(text: str, compile_once, rounds: int = 4, keys: dict = None):
    """Add forward declarations until CL stops asking, or give up and say so.

    Predicting which identifiers in a signature are TYPES cannot be done
    reliably from the catalogue: `RECT` is a Windows type, `Console` is a game
    class, and neither is distinguishable from a parameter name by inspection.
    So the compiler is asked instead. This is mechanical and has no blind spot
    that a hand-maintained list of known types would have.

    Returns (text, [names added], reason-it-stopped). A unit that still does not
    compile is returned WITH its diagnostic rather than silently: the census
    counts NO_COMPILE, and a skeleton that never compiled is the emitter's
    defect to fix, not the agent's.
    """
    added = []
    for _ in range(rounds):
        output = compile_once(text)
        if output is None:
            return text, added, ""
        names = set()
        for pattern in UNDECLARED:
            names |= set(pattern.findall(output))
        source = text.splitlines()
        for number, variable in MISSING_SEMICOLON.findall(output):
            index = int(number) - 1
            if not (0 <= index < len(source)):
                continue
            # The unresolved type is the token immediately BEFORE the named
            # variable, not "some identifier on this line". Taking the whole
            # line forward-declares non-types too, which trades a C2146 for a
            # C2059 and moves the failure without fixing it - measured, C2059
            # went 4 -> 11 while the total stayed put.
            words = re.findall(r"[A-Za-z_]\w*", source[index])
            if variable in words:
                position = words.index(variable)
                if position:
                    names.add(words[position - 1])
        names -= set(added) | BUILTIN
        names -= KEYWORDS
        if not names:
            return text, added, output
        added.extend(sorted(names))
        # The key has to agree with `class_keys`, or a type this loop
        # discovers is declared `struct` while the target object holds
        # the `V` the source would have emitted for a class.
        table = catalogue_class_keys() if keys is None else keys
        insert = "\n".join(f"{table.get(name, 'struct')} {name};"
                           for name in sorted(names))
        marker = "typedef unsigned char uint8_t;\n"
        text = text.replace(marker, marker + "\n" + insert + "\n", 1)
    return text, added, "still undeclared after repair rounds"


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    functions = load_functions()
    derived = load_derived()
    callees = load_callees()
    pe = pefile.PE(str(arguments.exe), fast_load=True)
    try:
        text = emit(int(arguments.address, 16), functions, derived, callees, pe)
    except Unsettled as error:
        print(f"REFUSED: {error}", file=sys.stderr)
        return 2

    if arguments.output:
        arguments.output.write_text(text)
        print(f"wrote {arguments.output}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
