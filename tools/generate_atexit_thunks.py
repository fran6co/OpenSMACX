#!/usr/bin/env python3
"""Generate recoveries for the ??__F atexit destructor thunk family.

MSVC emits one `??__F<global>` thunk per dynamically-initialized global: the
counterpart of the `??__E` initializer, registered with atexit, whose whole
body is "tear down the object at this fixed address". The initializers stay
parked on the CRT exit-registration boundary, but nothing parks the thunks -
the atexit call lives in the `??__E`, and the `??__F` body is a bare
`mov ecx, <global> / call <teardown>`.

The family is zero-variance, which is why this is a generator rather than a
work order: every accepted body is proven by the forwarder acceptor
(tools/analyze_delegates.py) to be exactly that shape, and everything emitted
here differs only in a name, an address, and which teardown it calls.
Anything whose proven target is not one of the modelled ones and does not
match the opaque template below is skipped and named, never emitted.

Two spellings come out of that. A thunk whose class the source models keeps
the typed spelling, `g_ALPHAMENU_WAVE->close()`. A thunk whose class has no
modelled type - three dozen of them, BaseWin through Strings - is emitted over
OPAQUE STORAGE instead: the address is a literal and the teardown goes through
a rebindable function-pointer seam defaulting to the original destructor.
Nothing about those bytes is harder; what was missing was a C++ type, and the
opaque spelling needs none, so no class header is written or touched.

The recovery also names the globals. Each thunk's mangled name carries the
original global's name (`??__Fg_ALPHAMENU_WAVE@@YAXXZ`), so the emitted seam
pointers - `Wave *g_ALPHAMENU_WAVE = (Wave *)0x006A7090;` - put the original
symbol names on ~two hundred fixed-address game objects. Those are rebindable
data seams and stay unclassified, per the standing data-seam policy.

Emitted, deterministically ordered by address:
  src/atexit_thunks.h / src/atexit_thunks.cpp   committed source
  <scratch>/atexit-thunk-tests.cpp              fragment for the leaf suite
  <scratch>/atexit-wire.txt                     address/symbol redirect list

Like the other bulk generators, this compresses authoring, not verification:
the emitted bodies still go through the leaf suite, mutation, and the batch
gate exactly as hand-written recoveries do.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import analyze_delegates as delegates  # noqa: E402
import disasm  # noqa: E402
from generator_support import (LICENSE,  # noqa: E402
                               identifier_of_global, read_bytes,
                               scan_seam_bindings)
from generator_support import seam_name as support_seam_name  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent

# The four teardowns the family dispatches to. A thunk whose proven target is
# anything else is reported and left alone.
DOMAINS = {
    "?close@Sprite@@QAEXXZ": ("Sprite", "sprite.h", "->close()"),
    "?close@CaviarData@@QAEXXZ": ("CaviarData", "caviar.h", "->close()"),
    "??1Texture@@QAE@XZ": ("Texture", "texture.h", "->~Texture()"),
    "??1Wave@@QAE@XZ": ("Wave", "wave.h", None),   # via WaveOriginalDestructor
    # The binary's ??1Buffer is source-owned as Buffer::destroy - the trivial
    # ~Buffer() stays trivial so embedding classes keep their implicit
    # destruction, which is why the call is a named method.
    "??1Buffer@@QAE@XZ": ("Buffer", "buffer.h", "->destroy()"),
    "?close@ButtonGroup@@QAEXXZ": ("ButtonGroup", "buttongroup.h", "->close()"),
    "??1BattleWin@@QAE@XZ": ("BattleWin", "battlewin.h", "->~BattleWin()"),
    "??1FX@@QAE@XZ": ("FX", "fx.h", "->~FX()"),
    "??1FontQueue@@QAE@XZ": ("FontQueue", "font.h", "->~FontQueue()"),
    "??1Font@@QAE@XZ": ("Font", "font.h", "->~Font()"),
    # The AMBIENCE thunk reaches its destructor through the 5-byte jump
    # thunk at 0x004483C0; the proven target keeps that spelling.
    "j_??1Ambience@@QAE@XZ": ("Ambience", "ambience.h", "->~Ambience()"),
}

# The CRT's vector destructor iterator: walk an array, calling one teardown
# per element. Thunks over global ARRAYS forward here with four literals -
# array, element size, count, per-element teardown - and the per-element
# teardowns map the array to its element type.
VECTOR_DTOR_NAME = "??_M@YGXPAXIHP6EX0@Z@Z"
ELEMENT_TEARDOWNS = {
    "??1Wave@@QAE@XZ": ("Wave", "wave.h", "WaveElementTeardown"),
    "??1Sprite@@QAE@XZ": ("Sprite", "sprite.h", "SpriteElementTeardown"),
    "??1CaviarData@@QAE@XZ":
        ("CaviarData", "caviar.h", "CaviarDataElementTeardown"),
    "??1Texture@@QAE@XZ": ("Texture", "texture.h", "TextureElementTeardown"),
    "??1Buffer@@QAE@XZ": ("Buffer", "buffer.h", "BufferElementTeardown"),
    "??1Font@@QAE@XZ": ("Font", "font.h", "FontElementTeardown"),
    "??1TextIndex@@QAE@XZ":
        ("TextIndex", "textindex.h", "TextIndexElementTeardown"),
    "??1FactionArt@@QAE@XZ":
        ("FactionArt", "faction.h", "FactionArtElementTeardown"),
}

# This family binds destructors and constructors and named methods, and never
# a scalar-deleting destructor - those belong to the deleting-thunk generator,
# and a name outside this set is a row to skip.
SEAM_KINDS = frozenset({"dtor", "ctor", "method"})

THUNK_NAME_RE = re.compile(r"^\?\?__F(\w+)@@YAXXZ$")

# Globals whose original name would shadow their own type get a distinct
# variable name. faction.cpp separately binds 0x0078E978 as FactionsArt, but
# that file is not linked into the leaf suite, so the binding is emitted here
# too rather than reused - the same one-address-two-names arrangement
# NetDaemonNet already documents.
REUSED_GLOBALS = {
    "FactionArt": "FactionArtGlobal",
    # The leaf suite holds a rebindable double named TxtIndex, and
    # textindex.cpp binds the interior pointer under the same name.
    "TxtIndex": "TxtIndexGlobal",
}


def variable_of(row) -> str:
    return REUSED_GLOBALS.get(row["global_name"], row["global_name"])


# --------------------------------------------------------------------------
# Opaque storage
#
# Most of these thunks name a class the source models. About three dozen name
# one it does not - BaseWin, DiploWin, Palette, Wave_In_Device, Strings - and
# the blocker was never the bytes: they are the same two instructions as every
# accepted thunk. It was that the typed spelling needs a C++ type, and writing
# a class header for a class nothing else in the tree uses is how a scripted
# batch once overwrote src/dialogs.h.
#
# The opaque spelling needs no type at all:
#
#     DiploWinDtorTarget(reinterpret_cast<void *>(0x0073ACD8));
#
# The storage stays a literal - there is no typed global to declare, so there
# is nothing to keep in sync with a header - and the teardown goes through a
# rebindable function-pointer seam defaulting to the original destructor's
# fixed address, exactly like every other original-dependency seam here. The
# hybrid redirects that address to a recovered body at run time; the leaf
# suite rebinds the seam to a recorder and checks the address it is handed.
# --------------------------------------------------------------------------


def adjustor_module():
    """tools/generate_adjustor_thunks, imported on demand.

    That module imports this one at module scope for LICENSE, so importing it
    back at module scope here would close the cycle: the partially initialised
    module would not have `entry_extent` bound yet at the moment this body
    looked it up. A deferred import is the whole fix.
    """
    import generate_adjustor_thunks  # noqa: PLC0415  (see the docstring)
    return generate_adjustor_thunks


def entry_extent(row):
    """generate_adjustor_thunks.entry_extent, through the deferred import.

    NOT `size`: that column sums every span in `body_ranges`, and 416
    catalogued functions are split, so decoding `size` bytes from the entry of
    one of those runs into whatever follows it.
    """
    return adjustor_module().entry_extent(row)


# `extern <type> *<Name>;   // 0xADDR` - how every seam in this tree records
# the original address it defaults to. Seams dedupe on that ADDRESS and never
# on the name: a second name for one address is a duplicate definition, and
# the link error lands a long way from its cause.
# Typedef names that spell exactly `void(__thiscall)(void *)`, so a seam
# already bound under one of them carries an opaque nullary teardown or
# constructor unchanged.
OPAQUE_NULLARY = "func_thiscall_teardown"
NULLARY_TYPEDEFS = frozenset({
    OPAQUE_NULLARY, "func_deleting_dtor", "func_adjustor_v"})

# The headers a generated thunk file may pull in to pick up a reused seam.
# A seam bound anywhere else is bound next to a real class declaration, and
# including that from here is how these files would acquire a dependency on a
# hand-written header - so that case refuses rather than reusing, and refuses
# rather than defining a second name for the same address.
SEAM_HEADERS = frozenset({
    "adjustor_thunks.h", "atexit_thunks.h", "deleting_thunks.h",
    "init_thunks.h", "vector_teardown.h"})


def load_bindings(source_dir: Path, exclude=()):
    """(by address, by name) over every seam binding declared in src/*.h.

    `exclude` drops the generator's own output, which would otherwise report
    last run's seams as prior art and make a rerun depend on what is already
    on disk.
    """
    by_address: dict[int, tuple[str, str, str]] = {}
    by_name: dict[str, tuple[int, str]] = {}
    for header_name, name, typedef, address in scan_seam_bindings(
            source_dir, exclude):
        previous = by_address.get(address)
        if previous and previous[0] != name:
            raise SystemExit(
                f"0x{address:08X} is bound as {previous[0]} in "
                f"{previous[2]} and as {name} in {header_name}; "
                f"refusing to guess which one an opaque seam may reuse")
        by_address[address] = (name, typedef, header_name)
        by_name[name] = (address, header_name)
    return by_address, by_name


def merge_bindings(bindings, extra):
    """Fold computed seam bindings into a scanned (by address, by name) pair.

    The scan only sees `extern T *N;   // 0xADDR`. Seams declared without that
    trailing comment - the per-element teardowns, WaveOriginalDestructor - are
    just as real, and a second definition of one of their addresses is the same
    link error. `extra` is {address: (name, typedef, header)}; the scan wins
    where both describe an address, and a disagreement refuses.
    """
    by_address = dict(bindings[0])
    by_name = dict(bindings[1])
    for address, entry in sorted(extra.items()):
        existing = by_address.get(address)
        if existing is None:
            by_address[address] = entry
            by_name.setdefault(entry[0], (address, entry[2]))
        elif existing[0] != entry[0]:
            raise SystemExit(
                f"0x{address:08X} is bound as {existing[0]} in {existing[2]} "
                f"and as {entry[0]} in {entry[2]}")
    return by_address, by_name


def side_bindings(pe, functions, source_dir):
    """{address: (name, typedef, header)} for the seams atexit_thunks.h owns.

    Computed rather than read back, so the init side can dedupe against this
    file's seams whether or not the copy on disk is current.
    """
    rows, _ = collect(pe, functions)
    bindings = load_bindings(
        source_dir, exclude=("atexit_thunks.h", "init_thunks.h"))
    seams, _declare, _includes = resolve_seams(opaque_needs(rows), bindings)
    table = {address: (name, OPAQUE_NULLARY, "atexit_thunks.h")
             for address, name in seams.items()}
    for row in rows:
        if row["target_name"] == VECTOR_DTOR_NAME:
            table[row["teardown_address"]] = (
                ELEMENT_TEARDOWNS[row["teardown_name"]][2], OPAQUE_NULLARY,
                "atexit_thunks.h")
    if WAVE_DESTRUCTOR_ADDRESS:
        table.setdefault(WAVE_DESTRUCTOR_ADDRESS,
                         ("WaveOriginalDestructor", "func_wave_destructor",
                          "atexit_thunks.h"))
    return table


def seam_name(mangled: str) -> str:
    """A seam name for a ctor/dtor/method target, from its own mangled name.

    Deliberately the spelling tools/generate_adjustor_thunks.target_symbol
    uses, because that is what the seams already sitting in deleting_thunks.h
    are named after: a target bound there resolves to the same name here and
    is reused rather than redefined.
    """
    return support_seam_name(mangled, SEAM_KINDS)


def resolve_seams(needs, bindings):
    """Assign one seam per distinct ADDRESS, reusing what src/*.h already has.

    `needs` maps address -> (mangled target name, required typedef). Returns
    (address -> seam name, [(address, name, typedef)] to declare here, sorted
    header names to include for the reused ones). Every refusal is a SystemExit
    naming both sides, because the alternative is a second definition of one
    address under two names.
    """
    by_address, by_name = bindings
    seams: dict[int, str] = {}
    declare: list[tuple[int, str, str]] = []
    declared: dict[str, int] = {}
    includes: set[str] = set()
    for address in sorted(needs):
        mangled, typedef = needs[address]
        preferred = seam_name(mangled)
        if not preferred:
            raise SystemExit(f"cannot derive a seam name for {mangled} "
                             f"at 0x{address:08X}")
        bound = by_address.get(address)
        if bound is not None:
            name, bound_typedef, header = bound
            if header not in SEAM_HEADERS:
                raise SystemExit(
                    f"0x{address:08X} ({mangled}) is already bound as {name} "
                    f"in {header}, which is not a generated seam header; "
                    f"reusing it would pull that header in here and defining "
                    f"a second name would be a duplicate definition")
            compatible = (bound_typedef in NULLARY_TYPEDEFS
                          if typedef == OPAQUE_NULLARY
                          else bound_typedef == typedef)
            if not compatible:
                raise SystemExit(
                    f"0x{address:08X} ({mangled}) is bound as {name} of type "
                    f"{bound_typedef} in {header}, but this needs {typedef}")
            seams[address] = name
            includes.add(header)
            continue
        clash = by_name.get(preferred)
        if clash is not None:
            raise SystemExit(
                f"seam {preferred} for 0x{address:08X} ({mangled}) is already "
                f"bound to 0x{clash[0]:08X} in {clash[1]}")
        if preferred in declared:
            raise SystemExit(
                f"seam {preferred} would be defined for both "
                f"0x{declared[preferred]:08X} and 0x{address:08X}")
        declared[preferred] = address
        declare.append((address, preferred, typedef))
        seams[address] = preferred
    return seams, declare, sorted(includes)


def wrapped_call(indent: str, callee: str, arguments) -> list[str]:
    """`callee(a, b, c);` wrapped under the open paren at 79 columns."""
    one_line = f"{indent}{callee}({', '.join(arguments)});"
    if len(one_line) <= 79:
        return [one_line]
    continuation = " " * (len(indent) + len(callee) + 1)
    lines = [f"{indent}{callee}("]
    for index, argument in enumerate(arguments):
        piece = argument + (", " if index + 1 < len(arguments) else ");")
        if lines[-1].endswith("(") or len(lines[-1]) + len(piece) <= 79:
            lines[-1] += piece
        else:
            lines.append(continuation + piece)
    return [line.rstrip() for line in lines]


def storage_literal(address: int) -> str:
    return f"reinterpret_cast<void *>(0x{address:08X})"


def decode_opaque_teardown_raw(data: bytes, address: int):
    """(storage, target) read straight off the encoding, or None.

    The capstone path renders every operand to text before anything reads it,
    which puts a text renderer between the bytes and an address that ends up
    in committed source. This reads the same two instructions out of the
    opcode and immediate bytes instead, and collect() requires the two to
    agree before either is emitted.

        B9 <imm32>   mov ecx, <storage>
        E9 <rel32>   jmp <teardown>          tail call, or
        E8 <rel32>   call <teardown> / C3    call then return
    """
    if len(data) < 10 or data[0] != 0xB9:
        return None
    storage = int.from_bytes(data[1:5], "little")
    opcode = data[5]
    if opcode not in (0xE8, 0xE9):
        return None
    relative = int.from_bytes(data[6:10], "little", signed=True)
    target = (address + 10 + relative) & 0xFFFFFFFF
    rest = data[10:]
    while rest and rest[-1] in (0x90, 0xCC):     # alignment padding
        rest = rest[:-1]
    if rest != (b"\xc3" if opcode == 0xE8 else b""):
        return None
    return storage, target


def opaque_teardown_row(pe, functions, row, forward):
    """An opaque-storage row for a ??__F whose class is not modelled.

    Returns the row, or a string saying why the body was left alone. The
    template is the same two instructions every accepted thunk has; what is
    checked here is that the target really is a nullary `__thiscall` - the
    only signature `void(__thiscall)(void *)` can carry - proven twice, from
    the mangled name and from the callee-pop byte count the body itself ends
    with. A destructor that popped arguments would corrupt the caller's stack.
    """
    target_row = functions.get(forward.target)
    if target_row is None:
        return f"target 0x{forward.target:08X} not catalogued"
    target_name = target_row["name"]
    receiver = forward.receiver or ""
    if forward.arguments or not receiver.startswith("global:0x"):
        return f"target {target_name}: unexpected contract"
    if not seam_name(target_name):
        return f"cannot name a seam for target {target_name}"
    if delegates.name_convention_and_arity(target_name) != ("thiscall", 0):
        return f"target {target_name} is not a nullary thiscall"
    pop = adjustor_module().callee_pop(pe, functions, forward.target)
    if pop != 0:
        return (f"target {target_name} pops {pop}"
                if pop is not None
                else f"target {target_name} callee-pop not determinable")

    address = int(row["address"], 16)
    raw = decode_opaque_teardown_raw(
        disasm.read_range(pe, address, entry_extent(row)), address)
    if raw is None:
        return f"target {target_name}: bytes are not the opaque template"
    decoded = (int(forward.receiver.split("global:")[1], 16), forward.target)
    if raw != decoded:
        raise SystemExit(
            f"{row['name']}: capstone reads "
            f"(0x{decoded[0]:08X}, 0x{decoded[1]:08X}), the encoding reads "
            f"(0x{raw[0]:08X}, 0x{raw[1]:08X}); refusing to emit either")
    return {
        "address": address,
        "global_address": raw[0],
        "target_name": target_name,
        "target_address": raw[1],
        "opaque": True,
    }


def collect(pe, functions):
    """(domain rows, leftovers): every unrecovered ??__F, proven and sorted."""
    rows = []
    leftovers = []
    # All states, not just unrecovered: regeneration must keep re-emitting the
    # thunks recovered by earlier runs, or rerunning the generator after a new
    # hub recovery would silently drop them from the committed files.
    for row in delegates.load_rows(None, None):
        if not row["name"].startswith("??__F"):
            continue
        match = THUNK_NAME_RE.match(row["name"])
        if not match:
            leftovers.append((row["address"], row["name"], "unparsable name"))
            continue
        try:
            forward = delegates.analyze(pe, row, functions)
        except delegates.Reject as reason:
            leftovers.append((row["address"], row["name"], str(reason)))
            continue
        target_row = functions.get(forward.target)
        target_name = target_row["name"] if target_row else ""
        if target_name == VECTOR_DTOR_NAME:
            if (len(forward.arguments) != 4
                    or any(a.kind != "imm" for a in forward.arguments)):
                leftovers.append((row["address"], row["name"],
                                  "array contract not four literals"))
                continue
            array, element_size, count, teardown = (
                a.value for a in forward.arguments)
            teardown_row = functions.get(teardown, {})
            teardown_name = teardown_row.get("name", "")
            if teardown_name not in ELEMENT_TEARDOWNS:
                leftovers.append((row["address"], row["name"],
                                  f"element teardown {teardown_name or hex(teardown)}"))
                continue
            rows.append({
                "address": forward.address,
                "global_name": match.group(1),
                "global_address": array,
                "target_name": VECTOR_DTOR_NAME,
                "element_size": element_size,
                "count": count,
                "teardown_name": teardown_name,
                "teardown_address": teardown,
            })
            continue
        if target_name not in DOMAINS:
            opaque = opaque_teardown_row(pe, functions, row, forward)
            if isinstance(opaque, str):
                leftovers.append((row["address"], row["name"], opaque))
                continue
            opaque["global_name"] = match.group(1)
            rows.append(opaque)
            continue
        if forward.arguments or not (forward.receiver or "").startswith("global:0x"):
            leftovers.append((row["address"], row["name"], "unexpected contract"))
            continue
        rows.append({
            "address": forward.address,
            "global_name": match.group(1),
            "global_address": int(forward.receiver.split("global:")[1], 16),
            "target_name": target_name,
        })
    rows.sort(key=lambda entry: entry["address"])
    return rows, leftovers


def typed(rows):
    """The rows that name a modelled class, in address order."""
    return [row for row in rows if not row.get("opaque")]


def opaque(rows):
    """The rows emitted over opaque storage, in address order."""
    return [row for row in rows if row.get("opaque")]


def opaque_needs(rows):
    """{target address: (mangled name, typedef)} for the opaque rows."""
    return {row["target_address"]: (row["target_name"], OPAQUE_NULLARY)
            for row in opaque(rows)}


def kind_of(row):
    if row.get("opaque"):
        return None
    if row["target_name"] == VECTOR_DTOR_NAME:
        return ELEMENT_TEARDOWNS[row["teardown_name"]][0]
    return DOMAINS[row["target_name"]][0]


def render_seam_block(declare, comment) -> list[str]:
    """`extern T *Name;   // 0xADDR` per new seam, in address order."""
    lines = list(comment)
    for address, name, typedef in declare:
        lines.append(f"extern {typedef} *{name};   // 0x{address:08X}")
    return lines


def render_header(rows, declare=(), includes=()) -> str:
    lines = [LICENSE, "#pragma once", ""]
    # Only pointers cross this header, so the element types stay forward
    # declarations; faction.h in particular does not stand alone, and nothing
    # here needs a complete type. The bodies include what they call.
    kinds = sorted({kind_of(row) for row in typed(rows)})
    for kind in kinds:
        keyword = "struct" if kind == "FactionArt" else "class"
        lines.append(f"{keyword} {kind};")
    lines.append("")
    lines.append("""/*
 * Atexit teardown thunks
 *
 * One ??__F thunk per dynamically-initialized global: MSVC registers each with
 * atexit from the matching ??__E initializer, and its whole body is "tear down
 * the object at this fixed address". The pointers below are those objects,
 * carrying the original global names out of the thunks' mangled names; they
 * are rebindable data seams and stay unclassified. The Wave teardown is the
 * one target not yet source-owned, so it is reached through a classified
 * dependency instead of a direct call.
 */""")
    lines.append("")
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic push")
    lines.append('#pragma GCC diagnostic ignored "-Wattributes"')
    lines.append("#endif")
    lines.append("typedef void(__thiscall func_wave_destructor)(Wave *wave);")
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic pop")
    lines.append("#endif")
    lines.append("extern func_wave_destructor *WaveOriginalDestructor;")
    lines.append("")
    lines.append('#include "vector_teardown.h"')
    teardowns = sorted({row["teardown_name"] for row in rows
                        if row["target_name"] == VECTOR_DTOR_NAME})
    for name in teardowns:
        seam = ELEMENT_TEARDOWNS[name][2]
        lines.append(f"extern func_thiscall_teardown *{seam};")
    lines.append("")
    if includes or declare:
        lines.extend(f'#include "{header}"' for header in includes)
        if includes and declare:
            lines.append("")
        lines.extend(render_seam_block(declare, [
            "// Opaque-storage teardown seams. These globals' classes are not",
            "// modelled in source, so the thunk hands the storage address",
            "// straight to the original teardown through a seam rather than",
            "// naming a type. Seams are deduped on the ADDRESS they default",
            "// to: one already bound at the same address elsewhere is reused",
            "// from the header above, never redefined here.",
        ]))
        lines.append("")
    for row in typed(rows):
        lines.append(f"extern {kind_of(row)} *{variable_of(row)};")
    lines.append("")
    for row in rows:
        lines.append(
            f"DLLEXPORT void __cdecl destroy_{identifier_of_global(row['global_name'])}();")
    lines.append("")
    return "\n".join(lines)


WAVE_DESTRUCTOR_ADDRESS = 0
VECTOR_DTOR_ADDRESS = 0


def render_source(rows, declare=()) -> str:
    lines = [LICENSE, '#include "stdafx.h"', '#include "atexit_thunks.h"']
    # Complete types are needed only where a method is called; array bodies
    # pass pointers through the iterator and stay on the forward declarations.
    for include in sorted({spec[1] for spec in DOMAINS.values()}):
        lines.append(f'#include "{include}"')
    lines.append("")
    lines.append("func_wave_destructor *WaveOriginalDestructor =")
    lines.append(f"    (func_wave_destructor *)0x{WAVE_DESTRUCTOR_ADDRESS:08X};")
    lines.append("")
    teardown_addresses = {}
    for row in rows:
        if row["target_name"] == VECTOR_DTOR_NAME:
            teardown_addresses[row["teardown_name"]] = row["teardown_address"]
    lines.append("")
    lines.append("// Per-element teardowns the array walks dispatch to. The Wave one binds")
    lines.append("// the same address as WaveOriginalDestructor deliberately: it is the same")
    lines.append("// original body, held separately so the array seam rebinds alone.")
    for name in sorted(teardown_addresses):
        seam = ELEMENT_TEARDOWNS[name][2]
        lines.append(f"func_thiscall_teardown *{seam} =")
        lines.append(f"    (func_thiscall_teardown *)0x{teardown_addresses[name]:08X};")
    if declare:
        lines.append("")
        lines.append("// The opaque-storage teardowns, each defaulting to the")
        lines.append("// original destructor the thunk tail jumps to; the")
        lines.append("// hybrid redirects it to a recovered body at run time.")
        for address, name, typedef in declare:
            lines.append(f"{typedef} *{name} =")
            lines.append(f"    ({typedef} *)0x{address:08X};")
    lines.append("")
    for row in typed(rows):
        lines.append(f"{kind_of(row)} *{variable_of(row)} = "
                     f"({kind_of(row)} *)0x{row['global_address']:08X};")
    lines.append("")
    for row in rows:
        if row.get("opaque"):
            body = wrapped_call("    ", row["seam"],
                                [storage_literal(row["global_address"])])
        elif row["target_name"] == VECTOR_DTOR_NAME:
            seam = ELEMENT_TEARDOWNS[row["teardown_name"]][2]
            body = [f"    VectorDtorIterator({variable_of(row)}, "
                    f"0x{row['element_size']:X}, {row['count']}, {seam});"]
        else:
            call = DOMAINS[row["target_name"]][2]
            body = [f"    WaveOriginalDestructor({variable_of(row)});"
                    if call is None else f"    {variable_of(row)}{call};"]
        lines.append("/*")
        lines.append(f"Purpose: Atexit teardown thunk for {row['global_name']}.")
        lines.append(f"Original Offset: {row['address']:08X}")
        lines.append("Return Value: n/a")
        lines.append("Status: Complete")
        lines.append("*/")
        lines.append(f"void __cdecl destroy_{identifier_of_global(row['global_name'])}() {{")
        lines.extend(body)
        lines.append("}")
        lines.append("")
    return "\n".join(lines)


def render_tests(rows) -> str:
    """A table-driven fragment: one case row per thunk, one loop per domain."""
    def cases(target_name):
        return [row for row in rows if row["target_name"] == target_name]

    out = []
    out.append("namespace {")
    out.append("struct AtexitThunkCase {")
    out.append("    void(__cdecl *thunk)();")
    out.append("    void *slot;   // the global seam to rebind")
    out.append("};")
    for target_name, table in (
            ("?close@Sprite@@QAEXXZ", "g_atexit_sprite_cases"),
            ("?close@CaviarData@@QAEXXZ", "g_atexit_caviar_cases"),
            ("??1Texture@@QAE@XZ", "g_atexit_texture_cases"),
            ("??1Wave@@QAE@XZ", "g_atexit_wave_cases"),
            ("??1Buffer@@QAE@XZ", "g_atexit_buffer_cases"),
            ("?close@ButtonGroup@@QAEXXZ", "g_atexit_group_cases"),
            ("??1BattleWin@@QAE@XZ", "g_atexit_battlewin_cases"),
            ("??1FX@@QAE@XZ", "g_atexit_fx_cases"),
            ("??1FontQueue@@QAE@XZ", "g_atexit_fontqueue_cases"),
            ("??1Font@@QAE@XZ", "g_atexit_font_cases")):
        out.append(f"const AtexitThunkCase {table}[] = {{")
        for row in cases(target_name):
            out.append(f"    {{&destroy_{identifier_of_global(row['global_name'])}, "
                       f"&{variable_of(row)}}},")
        out.append("};")
    out.append("struct AtexitArrayCase {")
    out.append("    void(__cdecl *thunk)();")
    out.append("    void *slot;")
    out.append("    uint32_t element_size;")
    out.append("    int count;")
    out.append("    func_thiscall_teardown **teardown_slot;")
    out.append("};")
    out.append("const AtexitArrayCase g_atexit_array_cases[] = {")
    for row in rows:
        if row["target_name"] != VECTOR_DTOR_NAME:
            continue
        seam = ELEMENT_TEARDOWNS[row["teardown_name"]][2]
        out.append(f"    {{&destroy_{identifier_of_global(row['global_name'])}, "
                   f"&{variable_of(row)}, 0x{row['element_size']:X}, "
                   f"{row['count']}, &{seam}}},")
    out.append("};")
    if opaque(rows):
        out.append("// The opaque-storage thunks carry no typed global to")
        out.append("// rebind, so the storage address itself is the")
        out.append("// expectation: rebind the teardown seam and check the")
        out.append("// pointer it is handed. Two thunks sharing a seam")
        out.append("// (NetMsg, Palette) each still assert their own address.")
        out.append("struct AtexitOpaqueCase {")
        out.append("    void(__cdecl *thunk)();")
        out.append("    func_thiscall_teardown **slot;")
        out.append("    void *storage;")
        out.append("};")
        out.append("const AtexitOpaqueCase g_atexit_opaque_cases[] = {")
        for row in opaque(rows):
            entry = (f"    {{&destroy_{identifier_of_global(row['global_name'])}, "
                     f"&{row['seam']},")
            out.append(entry)
            out.append(f"     {storage_literal(row['global_address'])}}},")
        out.append("};")
    out.append("""
Wave *g_atexit_wave_seen;
int g_atexit_wave_calls;
void __thiscall observe_wave_destructor(Wave *wave) {
    g_atexit_wave_seen = wave;
    ++g_atexit_wave_calls;
}
void *g_vector_array_seen;
uint32_t g_vector_size_seen;
int g_vector_count_seen;
func_thiscall_teardown *g_vector_teardown_seen;
int g_vector_calls;
void __stdcall observe_vector_dtor(void *array, unsigned int element_size,
                                   int count,
                                   func_thiscall_teardown *teardown) {
    g_vector_array_seen = array;
    g_vector_size_seen = element_size;
    g_vector_count_seen = count;
    g_vector_teardown_seen = teardown;
    ++g_vector_calls;
}
int g_vector_sentinel;""")
    if opaque(rows):
        out.append("""void *g_atexit_opaque_seen;
int g_atexit_opaque_calls;
void __thiscall observe_opaque_teardown(void *object) {
    g_atexit_opaque_seen = object;
    ++g_atexit_opaque_calls;
}""")
    out.append("""}  // namespace

void test_atexit_teardown_thunks() {
    // Every thunk is "tear down the object at this fixed address". Rebinding
    // the per-global seam to a local object and watching the teardown's own
    // observable - the free seam it calls, or the destructor dependency -
    // proves each thunk reaches its own global and the right teardown.
    auto *const saved_sprite_free = SpriteFree;
    int *const saved_sprite_memory = SpriteMemoryUsed;
    auto *const saved_caviar_free = CaviarDataFreeRecord;
    auto *const saved_texture_free = TextureFree;
    auto *const saved_wave_dtor = WaveOriginalDestructor;
    SpriteFree = &observe_texture_free;   // same shape; shared observer
    TextureFree = &observe_texture_free;
    CaviarDataFreeRecord = &observe_caviar_free_record;
    WaveOriginalDestructor = &observe_wave_destructor;
    int memory_used = 0;
    SpriteMemoryUsed = &memory_used;

    auto *const saved_iterator = VectorDtorIterator;
    for (const AtexitThunkCase &entry : g_atexit_sprite_cases) {
        alignas(4) uint8_t fake[sizeof(Sprite)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // second free branch only
        auto **slot = static_cast<Sprite **>(entry.slot);
        Sprite *const saved = *slot;
        *slot = reinterpret_cast<Sprite *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);         // its own global, freed once
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_caviar_cases) {
        alignas(4) uint8_t fake[sizeof(CaviarData)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x8, &pointer, 4);
        auto **slot = static_cast<CaviarData **>(entry.slot);
        CaviarData *const saved = *slot;
        *slot = reinterpret_cast<CaviarData *>(fake);
        g_caviar_free_calls = 0;
        g_caviar_freed = nullptr;
        entry.thunk();
        expect(g_caviar_free_calls == 1);
        expect(g_caviar_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_texture_cases) {
        alignas(4) uint8_t fake[sizeof(Texture)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // ours: borrowed_ stays 0
        auto **slot = static_cast<Texture **>(entry.slot);
        Texture *const saved = *slot;
        *slot = reinterpret_cast<Texture *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_wave_cases) {
        alignas(4) uint8_t fake[sizeof(Wave)] = {};
        auto **slot = static_cast<Wave **>(entry.slot);
        Wave *const saved = *slot;
        *slot = reinterpret_cast<Wave *>(fake);
        g_atexit_wave_calls = 0;
        g_atexit_wave_seen = nullptr;
        entry.thunk();
        expect(g_atexit_wave_calls == 1);
        expect(g_atexit_wave_seen == reinterpret_cast<Wave *>(fake));
        *slot = saved;
    }

    // Array thunks hand the whole walk to the iterator seam: the observed
    // call must carry the rebound array, the exact element size and count,
    // and the rebound per-element teardown - proving the body reads all four
    // seams rather than any baked literal.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitArrayCase &entry : g_atexit_array_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_teardown = *entry.teardown_slot;
        *slot = fake;
        *entry.teardown_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        expect(g_vector_size_seen == entry.element_size);
        expect(g_vector_count_seen == entry.count);
        expect(g_vector_teardown_seen ==
               reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
        *slot = saved_slot;
        *entry.teardown_slot = saved_teardown;
    }
    VectorDtorIterator = saved_iterator;

    // Buffer and ButtonGroup teardowns are already source-owned, so each
    // thunk is checked against ground truth directly: run the real teardown
    // on an identical twin and require the bytes to agree.
    //
    // Buffer::close reads *BufferResetValue520, whose default target is an
    // address in the original image. Nothing maps that address in the
    // standalone leaf process, so it has to be repointed at a local slot
    // first. This suite got away without it for a long time only because the
    // address happened to fall inside the test binary's own image; growing
    // the binary moved it outside and turned it into a page fault in the
    // release build while debug still passed.
    uint32_t *const saved_reset_520 = BufferResetValue520;
    uint32_t reset_520_slot = 0;
    BufferResetValue520 = &reset_520_slot;
    for (const AtexitThunkCase &entry : g_atexit_buffer_cases) {
        alignas(4) uint8_t fake[sizeof(Buffer)] = {};
        alignas(4) uint8_t twin[sizeof(Buffer)] = {};
        auto **slot = static_cast<Buffer **>(entry.slot);
        Buffer *const saved = *slot;
        *slot = reinterpret_cast<Buffer *>(fake);
        entry.thunk();
        reinterpret_cast<Buffer *>(twin)->destroy();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }
    BufferResetValue520 = saved_reset_520;
    for (const AtexitThunkCase &entry : g_atexit_group_cases) {
        alignas(4) uint8_t fake[sizeof(ButtonGroup)];
        alignas(4) uint8_t twin[sizeof(ButtonGroup)];
        seed_storage(fake, twin, sizeof(fake));
        auto **slot = static_cast<ButtonGroup **>(entry.slot);
        ButtonGroup *const saved = *slot;
        *slot = reinterpret_cast<ButtonGroup *>(fake);
        entry.thunk();
        reinterpret_cast<ButtonGroup *>(twin)->close();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }

    // FX and FontQueue teardowns hand their member-array walk to the vector
    // iterator; the observed array base must be the rebound global. Sizes and
    // counts are pinned by the destructors' own suites.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitThunkCase &entry : g_atexit_fx_cases) {
        alignas(4) uint8_t fake[sizeof(FX)];
        auto **slot = static_cast<FX **>(entry.slot);
        FX *const saved = *slot;
        *slot = reinterpret_cast<FX *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    for (const AtexitThunkCase &entry : g_atexit_fontqueue_cases) {
        alignas(4) uint8_t fake[sizeof(FontQueue)];
        auto **slot = static_cast<FontQueue **>(entry.slot);
        FontQueue *const saved = *slot;
        *slot = reinterpret_cast<FontQueue *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    VectorDtorIterator = saved_iterator;

    // The Font teardown runs the suite's Font::close double, whose writes to
    // the rebound object are themselves the observation.
    for (const AtexitThunkCase &entry : g_atexit_font_cases) {
        alignas(4) uint8_t fake[sizeof(Font)];
        uint8_t untouched[sizeof(Font)];
        seed_storage(fake, untouched, sizeof(fake));
        auto **slot = static_cast<Font **>(entry.slot);
        Font *const saved = *slot;
        *slot = reinterpret_cast<Font *>(fake);
        entry.thunk();
        int32_t first = 0;
        std::memcpy(&first, fake, 4);
        expect(first == -1);               // the double's signature write
        *slot = saved;
    }

    // The BattleWin teardown is the Time member at +8, observed through the
    // suite's Time::close double recording its receiver.
    for (const AtexitThunkCase &entry : g_atexit_battlewin_cases) {
        alignas(4) uint8_t fake[sizeof(BattleWin)] = {};
        auto **slot = static_cast<BattleWin **>(entry.slot);
        BattleWin *const saved = *slot;
        *slot = reinterpret_cast<BattleWin *>(fake);
        time_close_calls = 0;
        entry.thunk();
        expect(time_close_calls == 1);
        expect(time_close_targets[0] == reinterpret_cast<Time *>(fake + 8));
        *slot = saved;
    }
""")
    if opaque(rows):
        out.append(
            """    // Opaque-storage thunks: the whole body is to hand this
    // address to that teardown, so the rebound seam receiving exactly
    // the recorded address, exactly once, is the whole contract. The
    // address is a literal in the body rather than a rebindable global,
    // which is what lets one seam serve two thunks over different storage.
    for (const AtexitOpaqueCase &entry : g_atexit_opaque_cases) {
        func_thiscall_teardown *const saved = *entry.slot;
        *entry.slot = &observe_opaque_teardown;
        g_atexit_opaque_calls = 0;
        g_atexit_opaque_seen = nullptr;
        entry.thunk();
        expect(g_atexit_opaque_calls == 1);
        expect(g_atexit_opaque_seen == entry.storage);
        *entry.slot = saved;
    }
""")
    out.append("""    SpriteFree = saved_sprite_free;
    SpriteMemoryUsed = saved_sprite_memory;
    CaviarDataFreeRecord = saved_caviar_free;
    TextureFree = saved_texture_free;
    WaveOriginalDestructor = saved_wave_dtor;
}
""")
    return "\n".join(out)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=disasm.DEFAULT_EXE)
    parser.add_argument("--scratch-dir", type=Path, required=True)
    parser.add_argument("--source-dir", type=Path,
                        default=REPO_ROOT / "src",
                        help="where the committed source pair is written; "
                             "point it at a scratch tree to diff a change "
                             "without touching src/")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    functions = {int(row["address"], 16): row
                 for row in delegates.load_rows(None, None)}
    global WAVE_DESTRUCTOR_ADDRESS
    wave_dtors = [address for address, row in functions.items()
                  if row["name"] == "??1Wave@@QAE@XZ"]
    if len(wave_dtors) != 1:
        raise SystemExit("expected exactly one ??1Wave in the catalog")
    WAVE_DESTRUCTOR_ADDRESS = wave_dtors[0]
    global VECTOR_DTOR_ADDRESS
    vector_dtors = [address for address, row in functions.items()
                    if row["name"] == VECTOR_DTOR_NAME]
    if len(vector_dtors) != 1:
        raise SystemExit("expected exactly one vector destructor iterator")
    VECTOR_DTOR_ADDRESS = vector_dtors[0]
    rows, leftovers = collect(pe, functions)
    # Seams dedupe on address against what src/*.h already binds. This file's
    # own output is excluded so a rerun does not read last run's seams back as
    # prior art, and init_thunks.h is excluded because nothing here includes
    # it - reusing from there would invert the dependency between the pair.
    bindings = load_bindings(REPO_ROOT / "src",
                             exclude=("atexit_thunks.h", "init_thunks.h"))
    seams, declare, includes = resolve_seams(opaque_needs(rows), bindings)
    for row in opaque(rows):
        row["seam"] = seams[row["target_address"]]

    by_domain = {}
    for row in rows:
        label = kind_of(row)
        if label is None:
            label = "opaque"
        elif row["target_name"] == VECTOR_DTOR_NAME:
            label += "[]"
        by_domain.setdefault(label, []).append(row)
    print(f"thunks accepted: {len(rows)}")
    for domain, entries in sorted(by_domain.items(), key=lambda i: -len(i[1])):
        print(f"  {domain:12} {len(entries):4}")
    print(f"opaque seams: {len(declare)} defined here, "
          f"{len(seams) - len(declare)} reused from {', '.join(includes)}"
          if includes else f"opaque seams: {len(declare)} defined here")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers[:10]:
            print(f"  {address} {name[:44]:46} {reason[:40]}")

    names = [row["global_name"] for row in rows]
    if len(set(names)) != len(names):
        raise SystemExit("duplicate global names; refusing to emit")
    symbols = [identifier_of_global(row["global_name"]) for row in rows]
    if len(set(symbols)) != len(symbols):
        raise SystemExit("duplicate emitted symbols; refusing to emit")

    if args.dry_run:
        return 0

    source_dir = Path(args.source_dir)
    source_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "atexit_thunks.h").write_text(
        render_header(rows, declare, includes))
    (source_dir / "atexit_thunks.cpp").write_text(render_source(rows, declare))
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (args.scratch_dir / "atexit-thunk-tests.cpp").write_text(render_tests(rows))
    wire = "\n".join(
        f"0x{row['address']:08X} destroy_{identifier_of_global(row['global_name'])}"
        for row in rows) + "\n"
    (args.scratch_dir / "atexit-wire.txt").write_text(wire)
    print(f"emitted {source_dir}/atexit_thunks.{{h,cpp}}, test fragment, "
          "wire list")
    return 0


if __name__ == "__main__":
    sys.exit(main())
