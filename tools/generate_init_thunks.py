#!/usr/bin/env python3
"""Generate recoveries for the ??__E dynamic-initializer thunk family.

MSVC emits one `??__E<global>` thunk per dynamically-initialized global: the
counterpart of the `??__F` teardown thunks src/atexit_thunks.cpp already
recovers. Each body is "construct the object at this fixed address, then
register its ??__F teardown with the CRT's atexit":

    mov ecx, <global> / call <ctor>
    push <??__F thunk> / call _atexit / pop ecx / ret

or, for global arrays, the vector constructor iterator with five literals:

    push <dtor> <ctor> <count> <size> <array> / call ??_L
    push <??__F thunk> / call _atexit / pop ecx / ret

The family is zero-variance: every accepted body is proven by the forwarder
acceptor (tools/analyze_delegates.py) to be exactly one of those shapes, and
everything emitted differs only in a name, a global address, which recovered
constructor runs, and which ??__F address is registered. Two invariants are
checked per thunk rather than assumed: the second call really is the one
catalogued _atexit, and the registered callback really is the catalogued
`??__F` thunk of the SAME global name. Anything else is skipped and named.

A thunk whose class the source models keeps the typed spelling, a placement
new on a typed global. A thunk whose class has no modelled type - three dozen
of them, BaseWin through MultiDebug - is emitted over OPAQUE STORAGE instead:

    DiploWinCtorTarget(reinterpret_cast<void *>(0x0073ACD8));

The address stays a literal, so there is no typed global to declare and no
header to write; the constructor goes through a rebindable function-pointer
seam defaulting to the original's fixed address. Constructors that take
arguments ride the same path, because their pushed literals are as provable as
the storage - the mangled name declares the parameter bytes and the callee-pop
byte count the constructor ends with confirms them.

The atexit registration passes the ORIGINAL ??__F address through one shared
rebindable GameAtexit seam: the game CRT's exit list must own the
registration so teardown order stays inside its LIFO walk, and the hybrid
redirects that address to the recovered destroy_* twin at run time.

Emitted, deterministically ordered by address:
  src/init_thunks.h / src/init_thunks.cpp      committed source
  <scratch>/init-thunk-tests.cpp               fragment for the leaf suite
  <scratch>/init-wire.txt                      address/symbol redirect list

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
import generate_atexit_thunks as atexit_gen  # noqa: E402
# entry_extent, not `size`: that column sums every span in `body_ranges` and
# 416 catalogued functions are split, so decoding `size` bytes from the entry
# of one of those runs past the body and into the next function.
from generate_adjustor_thunks import (  # noqa: E402
    callee_pop, declaration, entry_extent)

REPO_ROOT = Path(__file__).resolve().parent.parent
LICENSE = atexit_gen.LICENSE
OPAQUE_NULLARY = atexit_gen.OPAQUE_NULLARY

# The constructors the family dispatches to, keyed by catalogued name. The
# spelling is how the recovered source runs that constructor on the fixed
# global: a placement-new for the real recovered constructors, the construct()
# method for Buffer (whose trivial Buffer() stays trivial so embedding classes
# keep their implicit construction), and destroy() for the one initializer
# that resets a Buffer global through its teardown. A thunk whose proven
# target is anything else is reported and left alone.
# (kind, header, seam name, trampoline body over `object`). The thunk bodies
# construct through a per-kind rebindable seam whose DEFAULT is a source-owned
# trampoline running the recovered constructor, the WaveControlGroupOriginalCtor
# arrangement: the hybrid still executes recovered code, while leaf tests
# rebind the seam to a recorder - the recovered constructors legitimately reach
# their own deeper seams (Wave's SoundSetType and the like), whose defaults are
# unmapped outside the game process, so the generated suite must never execute
# them.
INIT_DOMAINS = {
    "??0Sprite@@QAE@XZ":
        ("Sprite", "sprite.h", "SpriteInitCtor",
         "new (object) Sprite();"),
    "??0CaviarData@@QAE@XZ":
        ("CaviarData", "caviar.h", "CaviarDataInitCtor",
         "new (object) CaviarData();"),
    "??0Wave@@QAE@XZ":
        ("Wave", "wave.h", "WaveInitCtor", "new (object) Wave();"),
    "??0Buffer@@QAE@XZ":
        ("Buffer", "buffer.h", "BufferInitCtor",
         "static_cast<Buffer *>(object)->construct();"),
    "??0Texture@@QAE@XZ":
        ("Texture", "texture.h", "TextureInitCtor",
         "new (object) Texture();"),
    "??0Font@@QAE@XZ":
        ("Font", "font.h", "FontInitCtor", "new (object) Font();"),
    "??0ButtonGroup@@QAE@XZ":
        ("ButtonGroup", "buttongroup.h", "ButtonGroupInitCtor",
         "new (object) ButtonGroup();"),
    "??0Caviar@@QAE@XZ":
        ("Caviar", "caviar.h", "CaviarInitCtor", "new (object) Caviar();"),
    "??0Wave_Device@@QAE@XZ":
        ("Wave_Device", "wave_device.h", "WaveDeviceInitCtor",
         "new (object) Wave_Device();"),
    "??0Time@@QAE@XZ":
        ("Time", "time.h", "TimeInitCtor", "new (object) Time();"),
    "??0Strings@@QAE@XZ":
        ("Strings", "strings.h", "StringsInitCtor",
         "new (object) Strings();"),
    "??1Buffer@@QAE@XZ":
        ("Buffer", "buffer.h", "BufferInitReset",
         "static_cast<Buffer *>(object)->destroy();"),
}

# Per-element constructors for the vector-iterator arrays, keyed by catalogued
# name: the emitted body hands the ORIGINAL address on through a rebindable
# seam, exactly as the ??__F side hands its element teardowns. The element
# type must agree with the array's registered element teardown.
ELEMENT_CTORS = {
    "??0Sprite@@QAE@XZ": ("Sprite", "SpriteElementCtor"),
    "??0CaviarData@@QAE@XZ": ("CaviarData", "CaviarDataElementCtor"),
    "??0Texture@@QAE@XZ": ("Texture", "TextureElementCtor"),
    "??0Wave@@QAE@XZ": ("Wave", "WaveElementCtor"),
    "??0Buffer@@QAE@XZ": ("Buffer", "BufferElementCtor"),
    "??0Font@@QAE@XZ": ("Font", "FontElementCtor"),
    "??0TextIndex@@QAE@XZ": ("TextIndex", "TextIndexElementCtor"),
}

# The matching per-element teardown seams atexit_thunks.h already owns, keyed
# by the ??_L call's dtor argument name. An array whose dtor kind has no seam
# there yet gains one here.
ELEMENT_DTORS = {
    "??1Sprite@@QAE@XZ": "SpriteElementTeardown",
    "??1CaviarData@@QAE@XZ": "CaviarDataElementTeardown",
    "??1Texture@@QAE@XZ": "TextureElementTeardown",
    "??1Wave@@QAE@XZ": "WaveElementTeardown",
    "??1Buffer@@QAE@XZ": "BufferElementTeardown",
    "??1Font@@QAE@XZ": "FontElementTeardown",
    "??1TextIndex@@QAE@XZ": "TextIndexElementTeardown",
}

VECTOR_CTOR_NAME = "??_L@YGXPAXIHP6EX0@Z1@Z"
ATEXIT_NAME = "_atexit"
THUNK_NAME_RE = re.compile(r"^\?\?__E(\w+)@@YAXXZ$")

# The constructor signatures the opaque path can carry: nullary, or all-`int`.
# Both are spelled straight out of the mangled name - `H` is int - so the
# emitted seam's parameter list is read from the binary's own declaration
# rather than inferred from how many literals happened to be pushed. Anything
# else (a pointer, a float, a struct) is left alone and named: the pushed
# literal alone would not say what type the slot is.
OPAQUE_CTOR_RE = re.compile(r"^\?\?0(\w+)@@QAE@(?:XZ|(H+)@Z)$")


def opaque_ctor_arity(mangled: str) -> int | None:
    """How many `int` parameters an opaque-carryable constructor takes."""
    match = OPAQUE_CTOR_RE.match(mangled)
    if not match:
        return None
    return len(match.group(2) or "")


def opaque_ctor_typedef(arity: int) -> str:
    return OPAQUE_NULLARY if not arity else f"func_opaque_ctor_{'i' * arity}"


def render_ctor_typedefs(arities) -> list[str]:
    """One `typedef void(__thiscall func_opaque_ctor_i...)(...)` per arity."""
    lines = []
    for arity in sorted(a for a in arities if a):
        name = opaque_ctor_typedef(arity)
        parameters = ["void *object"] + [f"int a{i}" for i in range(arity)]
        lines.extend(declaration(f"typedef void(__thiscall {name})",
                                 parameters, ";"))
    return lines


def _read_push(data: bytes, index: int):
    """(value, next index) for `6A ib` or `68 id`, else (None, index)."""
    if index < len(data) and data[index] == 0x6A:
        return int.from_bytes(data[index + 1:index + 2], "little",
                              signed=True), index + 2
    if index < len(data) and data[index] == 0x68:
        return int.from_bytes(data[index + 1:index + 5], "little"), index + 5
    return None, index


def decode_opaque_init_raw(data: bytes, address: int):
    """(arguments, storage, target, registered, atexit) off the encoding.

    The capstone path renders every operand to text before anything reads it,
    which puts a text renderer between the bytes and the addresses and literals
    that end up in committed source. This walks the same grammar out of the
    opcode and immediate bytes, and collect() requires the two to agree.

        [6A ib | 68 id]*    push the constructor arguments, right to left
        B9 <imm32>          mov ecx, <storage>          (scalar form only)
        E8 <rel32>          call <constructor>
        68 <imm32>          push <??__F thunk>
        E8 <rel32>          call <atexit>
        59 C3               pop ecx / ret

    Returns None when the bytes are not that shape, and lets the capstone path
    report why.
    """
    index = 0
    pushed = []
    while True:
        value, following = _read_push(data, index)
        if value is None:
            break
        pushed.append(value)
        index = following
    storage = None
    if index < len(data) and data[index] == 0xB9:
        storage = int.from_bytes(data[index + 1:index + 5], "little")
        index += 5
    if index >= len(data) or data[index] != 0xE8:
        return None
    index += 5
    target = (address + index
              + int.from_bytes(data[index - 4:index], "little",
                               signed=True)) & 0xFFFFFFFF
    if index >= len(data) or data[index] != 0x68:
        return None
    registered, index = _read_push(data, index)
    if index >= len(data) or data[index] != 0xE8:
        return None
    index += 5
    registrar = (address + index
                 + int.from_bytes(data[index - 4:index], "little",
                                  signed=True)) & 0xFFFFFFFF
    if data[index:index + 2] != b"\x59\xc3":
        return None
    rest = data[index + 2:]
    while rest and rest[-1] in (0x90, 0xCC):        # alignment padding
        rest = rest[:-1]
    if rest:
        return None
    pushed.reverse()                    # right-to-left pushes -> source order
    return pushed, storage, target, registered, registrar


def second_call_target(pe, row) -> int | None:
    """The atexit call the acceptor does not itself verify."""
    instructions = delegates.trailing_padding(
        delegates.decode(pe, int(row["address"], 16), entry_extent(row)))
    calls = [int(i.op_str, 16) for i in instructions
             if i.mnemonic == "call" and i.op_str.startswith("0x")]
    return calls[1] if len(calls) >= 2 else None


def check_raw(row, pe, forward, registered, decoded):
    """Cross-check the capstone reading against the encoding, or say why not.

    Returns None when the two agree, a leftover reason when the bytes are not
    the template at all, and refuses outright when they are the template but
    disagree - a rendering quirk must never become a wrong address or a wrong
    literal in committed source.
    """
    address = int(row["address"], 16)
    raw = decode_opaque_init_raw(
        disasm.read_range(pe, address, entry_extent(row)), address)
    if raw is None:
        return "bytes are not the opaque template"
    arguments, storage, target, raw_registered, _registrar = raw
    read = (arguments, storage, target, raw_registered)
    if read != decoded:
        raise SystemExit(
            f"{row['name']}: capstone reads {decoded}, the encoding reads "
            f"{read}; refusing to emit either")
    return None


def check_nullary(pe, functions, address, label):
    """A per-element ctor/dtor must be a nullary thiscall, proven twice."""
    target_row = functions.get(address)
    if target_row is None:
        return f"{label} 0x{address:08X} not catalogued"
    name = target_row["name"]
    if not atexit_gen.seam_name(name):
        return f"cannot name a seam for {label} {name}"
    if delegates.name_convention_and_arity(name) != ("thiscall", 0):
        return f"{label} {name} is not a nullary thiscall"
    pop = callee_pop(pe, functions, address)
    if pop != 0:
        return (f"{label} {name} pops {pop}" if pop is not None
                else f"{label} {name} callee-pop not determinable")
    return None


def opaque_init_row(pe, functions, row, forward, registered):
    """An opaque-storage row for a ??__E whose class is not modelled.

    Returns the row, or a string saying why the body was left alone. The
    arithmetic that would corrupt a caller's stack if guessed - how many
    argument bytes the constructor takes - is proven twice here: from the
    mangled name's declaration and from the callee-pop count the constructor
    itself ends with. Both must also equal what the body pushed.
    """
    target_row = functions.get(forward.target)
    if target_row is None:
        return f"target 0x{forward.target:08X} not catalogued"
    target_name = target_row["name"]
    if not (forward.receiver or "").startswith("global:0x"):
        return f"target {target_name}: no fixed-address receiver"
    if any(argument.kind != "imm" for argument in forward.arguments):
        return f"target {target_name}: arguments are not all literals"
    arity = opaque_ctor_arity(target_name)
    if arity is None:
        return f"target {target_name} is not a nullary or all-int constructor"
    if arity != len(forward.arguments):
        return (f"target {target_name} declares {arity} arguments but "
                f"{len(forward.arguments)} are pushed")
    if delegates.name_convention_and_arity(target_name) != ("thiscall",
                                                            4 * arity):
        return f"target {target_name} does not declare {4 * arity} bytes"
    pop = callee_pop(pe, functions, forward.target)
    if pop != 4 * arity:
        return (f"target {target_name} pops {pop}, signature wants "
                f"{4 * arity}")
    storage = int(forward.receiver.split("global:")[1], 16)
    reason = check_raw(row, pe, forward, registered,
                       ([argument.value for argument in forward.arguments],
                        storage, forward.target, registered))
    if reason:
        return f"target {target_name}: {reason}"
    return {
        "address": forward.address,
        "global_address": storage,
        "target_name": target_name,
        "target_address": forward.target,
        "arity": arity,
        "arguments": [argument.value for argument in forward.arguments],
        "registered": registered,
        "opaque": True,
    }


def opaque_array_row(pe, functions, row, forward, registered):
    """An opaque-storage row for a ??_L array whose element class is not
    modelled. The five literals are the whole contract; the element ctor and
    dtor become address seams, so neither needs a C++ type."""
    array, element_size, count, ctor, dtor = (
        argument.value for argument in forward.arguments)
    for label, address in (("element ctor", ctor), ("element dtor", dtor)):
        reason = check_nullary(pe, functions, address, label)
        if reason:
            return reason
    reason = check_raw(row, pe, forward, registered,
                       ([array, element_size, count, ctor, dtor], None,
                        forward.target, registered))
    if reason:
        return reason
    return {
        "address": forward.address,
        "global_address": array,
        "target_name": VECTOR_CTOR_NAME,
        "element_size": element_size,
        "count": count,
        "ctor_name": functions[ctor]["name"],
        "ctor_address": ctor,
        "dtor_name": functions[dtor]["name"],
        "dtor_address": dtor,
        "registered": registered,
        "opaque": True,
    }


def collect(pe, functions):
    """(rows, leftovers): every catalogued ??__E, proven, verified, sorted."""
    names_by_address = {address: row["name"]
                        for address, row in functions.items()}
    atexit_addresses = {address for address, name in names_by_address.items()
                        if name == ATEXIT_NAME}
    if len(atexit_addresses) != 1:
        raise SystemExit("expected exactly one catalogued _atexit")
    atexit_address = next(iter(atexit_addresses))

    rows = []
    leftovers = []
    # All states, not just unrecovered: regeneration must keep re-emitting
    # thunks recovered by earlier runs.
    for row in delegates.load_rows(None, None):
        if not row["name"].startswith("??__E"):
            continue
        match = THUNK_NAME_RE.match(row["name"])
        if not match:
            leftovers.append((row["address"], row["name"], "unparsable name"))
            continue
        global_name = match.group(1)
        try:
            forward = delegates.analyze(pe, row, functions)
        except delegates.Reject as reason:
            leftovers.append((row["address"], row["name"], str(reason)))
            continue
        if not forward.vtable_writes:
            leftovers.append((row["address"], row["name"],
                              "registers no teardown"))
            continue
        registered = forward.vtable_writes[-1][1]
        registered_name = names_by_address.get(registered, "")
        if registered_name != f"??__F{global_name}@@YAXXZ":
            leftovers.append((row["address"], row["name"],
                              f"registered {registered_name or hex(registered)}"
                              " is not this global's ??__F"))
            continue
        if second_call_target(pe, row) != atexit_address:
            leftovers.append((row["address"], row["name"],
                              "second call is not the catalogued _atexit"))
            continue

        target_name = names_by_address.get(forward.target, "")
        if target_name == VECTOR_CTOR_NAME:
            if (len(forward.arguments) != 5
                    or any(a.kind != "imm" for a in forward.arguments)):
                leftovers.append((row["address"], row["name"],
                                  "array contract not five literals"))
                continue
            array, element_size, count, ctor, dtor = (
                a.value for a in forward.arguments)
            ctor_name = names_by_address.get(ctor, "")
            dtor_name = names_by_address.get(dtor, "")
            if ctor_name in ELEMENT_CTORS and dtor_name in ELEMENT_DTORS:
                # Both element kinds are modelled: the typed spelling, where
                # the two kinds must agree or the pair is not something to
                # guess about. The opaque path below has no kinds to disagree,
                # so this check stays on the typed side rather than moving.
                if ELEMENT_CTORS[ctor_name][0] not in dtor_name:
                    leftovers.append((row["address"], row["name"],
                                      "element ctor and dtor kinds disagree"))
                    continue
                rows.append({
                    "address": forward.address,
                    "global_name": global_name,
                    "global_address": array,
                    "target_name": VECTOR_CTOR_NAME,
                    "element_size": element_size,
                    "count": count,
                    "ctor_name": ctor_name,
                    "ctor_address": ctor,
                    "dtor_name": dtor_name,
                    "dtor_address": dtor,
                    "registered": registered,
                })
                continue
            entry = opaque_array_row(pe, functions, row, forward, registered)
            if isinstance(entry, str):
                leftovers.append((row["address"], row["name"], entry))
                continue
            entry["global_name"] = global_name
            rows.append(entry)
            continue
        if target_name not in INIT_DOMAINS:
            entry = opaque_init_row(pe, functions, row, forward, registered)
            if isinstance(entry, str):
                leftovers.append((row["address"], row["name"], entry))
                continue
            entry["global_name"] = global_name
            rows.append(entry)
            continue
        if forward.arguments or not (forward.receiver or "").startswith("global:0x"):
            leftovers.append((row["address"], row["name"],
                              "unexpected contract"))
            continue
        rows.append({
            "address": forward.address,
            "global_name": global_name,
            "global_address": int(forward.receiver.split("global:")[1], 16),
            "target_name": target_name,
            "registered": registered,
        })
    rows.sort(key=lambda entry: entry["address"])
    return rows, leftovers


def typed(rows):
    """The rows that name a modelled class, in address order."""
    return [row for row in rows if not row.get("opaque")]


def opaque(rows):
    """The rows emitted over opaque storage, in address order."""
    return [row for row in rows if row.get("opaque")]


def kind_of(row) -> str | None:
    if row.get("opaque"):
        return None
    if row["target_name"] == VECTOR_CTOR_NAME:
        return ELEMENT_CTORS[row["ctor_name"]][0]
    return INIT_DOMAINS[row["target_name"]][0]


def opaque_needs(rows):
    """{address: (mangled name, typedef)} for every seam the opaque rows want.

    An opaque array wants two - a per-element constructor and a per-element
    destructor - and either may already be bound: the FactionArt teardown, for
    one, is atexit_thunks.h's FactionArtElementTeardown. Feeding both through
    the same address-keyed resolver is what makes that a reuse instead of a
    second definition of one address.
    """
    needs = {}
    for row in opaque(rows):
        if row["target_name"] == VECTOR_CTOR_NAME:
            needs[row["ctor_address"]] = (row["ctor_name"], OPAQUE_NULLARY)
            needs[row["dtor_address"]] = (row["dtor_name"], OPAQUE_NULLARY)
        else:
            needs[row["target_address"]] = (
                row["target_name"], opaque_ctor_typedef(row["arity"]))
    return needs


def table_bindings(rows, f_rows):
    """{address: (name, typedef, header)} for the seams the two tables own.

    These are declared without the `// 0xADDR` comment the scanner reads, so
    they have to be handed over explicitly - a per-element seam redefined
    under a second name is exactly the duplicate definition this dedupe is for.
    """
    table = {}
    for row in typed(rows):
        if row["target_name"] != VECTOR_CTOR_NAME:
            continue
        table[row["ctor_address"]] = (
            ELEMENT_CTORS[row["ctor_name"]][1], OPAQUE_NULLARY,
            "init_thunks.h")
        table[row["dtor_address"]] = (
            ELEMENT_DTORS[row["dtor_name"]], OPAQUE_NULLARY,
            "atexit_thunks.h")
    for row in f_rows:
        if row["target_name"] == atexit_gen.VECTOR_DTOR_NAME:
            table.setdefault(row["teardown_address"], (
                atexit_gen.ELEMENT_TEARDOWNS[row["teardown_name"]][2],
                OPAQUE_NULLARY, "atexit_thunks.h"))
    return table


def atexit_side_globals(pe, functions) -> dict[str, int]:
    """global name -> address for every global atexit_thunks.h already binds.

    Opaque rows are skipped: they declare no global at all, so nothing on this
    side may treat their name as already bound over there.
    """
    f_rows, _ = atexit_gen.collect(pe, functions)
    return {atexit_gen.variable_of(row): row["global_address"]
            for row in atexit_gen.typed(f_rows)}


def render_header(rows, shared_globals, declare=(), includes=()) -> str:
    lines = [LICENSE, "#pragma once", ""]
    kinds = sorted({kind_of(row) for row in typed(rows)})
    for kind in kinds:
        lines.append(f"class {kind};")
    lines.append("")
    lines.append("""/*
 * Dynamic-initializer thunks
 *
 * One ??__E thunk per dynamically-initialized global: construct the object at
 * its fixed address through the recovered constructor, then register its
 * catalogued ??__F teardown with the game CRT's atexit - through the
 * GameAtexit seam, because the game CRT's exit list must own the registration
 * so teardown order stays inside its LIFO walk. The registered value is the
 * ORIGINAL ??__F address, which the hybrid redirects to the recovered
 * destroy_* twin at run time. Globals shared with the teardown side stay
 * declared in atexit_thunks.h; only the ones the teardown side does not bind
 * appear here.
 */""")
    lines.append("")
    lines.append("typedef void(__cdecl func_atexit_callback)();")
    lines.append("typedef int(__cdecl func_game_atexit)("
                 "func_atexit_callback *callback);")
    lines.append("extern func_game_atexit *GameAtexit;   // 0x00645398")
    lines.append("")
    lines.append('#include "vector_teardown.h"')
    ctor_seams = sorted({ELEMENT_CTORS[row["ctor_name"]][1]
                         for row in typed(rows)
                         if row["target_name"] == VECTOR_CTOR_NAME})
    for seam in ctor_seams:
        lines.append(f"extern func_thiscall_teardown *{seam};")
    lines.append("")
    lines.append("// Per-kind scalar construction seams, defaulting to")
    lines.append("// source-owned trampolines over the recovered")
    lines.append("// constructors. Rebindable because those constructors")
    lines.append("// reach their own deeper seams, whose defaults are")
    lines.append("// unmapped outside the game process.")
    scalar_seams = sorted({INIT_DOMAINS[row["target_name"]][2]
                           for row in typed(rows)
                           if row["target_name"] != VECTOR_CTOR_NAME})
    for seam in scalar_seams:
        lines.append(f"extern func_thiscall_teardown *{seam};")
    lines.append("")
    if includes or declare:
        lines.extend(f'#include "{header}"' for header in includes)
        if includes:
            lines.append("")
        typedefs = render_ctor_typedefs(
            {row["arity"] for row in opaque(rows)
             if row["target_name"] != VECTOR_CTOR_NAME})
        if typedefs:
            lines.append("#if defined(__GNUC__)")
            lines.append("#pragma GCC diagnostic push")
            lines.append('#pragma GCC diagnostic ignored "-Wattributes"')
            lines.append("#endif")
            lines.extend(typedefs)
            lines.append("#if defined(__GNUC__)")
            lines.append("#pragma GCC diagnostic pop")
            lines.append("#endif")
            lines.append("")
        lines.extend(atexit_gen.render_seam_block(declare, [
            "// Opaque-storage construction seams. These globals' classes",
            "// are not modelled in source, so the thunk hands the storage",
            "// address straight to the original constructor through a seam",
            "// rather than naming a type. Seams are deduped on the ADDRESS",
            "// they default to; one already bound at the same address",
            "// elsewhere is reused, never redefined here.",
        ]))
        lines.append("")
    for row in typed(rows):
        variable = atexit_gen.variable_of(row)
        if variable in shared_globals:
            continue
        lines.append(f"extern {kind_of(row)} *{variable};")
    lines.append("")
    for row in rows:
        lines.append(
            f"DLLEXPORT void __cdecl "
            f"construct_{atexit_gen.snake(row['global_name'])}();")
    lines.append("")
    return "\n".join(lines)


def render_source(rows, shared_globals, atexit_address, declare=()) -> str:
    lines = [LICENSE, '#include "stdafx.h"', '#include <new>',
             '#include "init_thunks.h"', '#include "atexit_thunks.h"']
    scalar_specs = sorted(
        {INIT_DOMAINS[row["target_name"]] for row in typed(rows)
         if row["target_name"] != VECTOR_CTOR_NAME},
        key=lambda spec: spec[2])
    includes = {spec[1] for name, spec in INIT_DOMAINS.items()
                if any(row["target_name"] == name for row in typed(rows))}
    for row in typed(rows):
        if row["target_name"] == VECTOR_CTOR_NAME:
            includes.add("sprite.h")   # element kinds are pointer-only; the
            break                      # scalar includes below cover the types
    for include in sorted(includes):
        lines.append(f'#include "{include}"')
    lines.append("")
    lines.append("// The game CRT's atexit: the exit list the original")
    lines.append("// registrations live in, so recovered initializers must")
    lines.append("// register there too or teardown order would split across")
    lines.append("// two CRTs. Rebindable so tests can observe registration.")
    lines.append("func_game_atexit *GameAtexit =")
    lines.append(f"    (func_game_atexit *)0x{atexit_address:08X};")
    lines.append("")
    ctor_addresses = {}
    for row in typed(rows):
        if row["target_name"] == VECTOR_CTOR_NAME:
            ctor_addresses[row["ctor_name"]] = row["ctor_address"]
    if ctor_addresses:
        lines.append("// Per-element constructors the array walks dispatch")
        lines.append("// to, bound at the original addresses the ??_L call")
        lines.append("// sites pass; the hybrid redirects each to its")
        lines.append("// recovered body at run time.")
        for name in sorted(ctor_addresses):
            seam = ELEMENT_CTORS[name][1]
            lines.append(f"func_thiscall_teardown *{seam} =")
            lines.append(
                f"    (func_thiscall_teardown *)0x{ctor_addresses[name]:08X};")
        lines.append("")
    lines.append("namespace {")
    lines.append("")
    lines.append("// Source-owned trampolines the scalar seams default to:")
    lines.append("// each runs the recovered construction on the object the")
    lines.append("// thunk names. The hybrid executes these; leaf tests")
    lines.append("// rebind the seams to recorders instead.")
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic push")
    lines.append('#pragma GCC diagnostic ignored "-Wattributes"')
    lines.append("#endif")
    for kind, _include, seam, body in scalar_specs:
        lines.append(f"void __thiscall {atexit_gen.snake(seam)}_default("
                     "void *object) {")
        lines.append(f"    {body}")
        lines.append("}")
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic pop")
    lines.append("#endif")
    lines.append("")
    lines.append("}  // namespace")
    lines.append("")
    for _kind, _include, seam, _body in scalar_specs:
        lines.append(f"func_thiscall_teardown *{seam} = "
                     f"&{atexit_gen.snake(seam)}_default;")
    if declare:
        lines.append("")
        lines.append("// The opaque-storage constructors, each defaulting to")
        lines.append("// the original the thunk calls; the hybrid redirects")
        lines.append("// it to a recovered body at run time.")
        for address, name, typedef in declare:
            lines.append(f"{typedef} *{name} =")
            lines.append(f"    ({typedef} *)0x{address:08X};")
    lines.append("")
    for row in typed(rows):
        variable = atexit_gen.variable_of(row)
        if variable in shared_globals:
            continue
        lines.append(f"{kind_of(row)} *{variable} = "
                     f"({kind_of(row)} *)0x{row['global_address']:08X};")
    lines.append("")
    for row in rows:
        variable = atexit_gen.variable_of(row)
        if row["target_name"] == VECTOR_CTOR_NAME and row.get("opaque"):
            body = atexit_gen.wrapped_call("    ", "VectorCtorIterator", [
                atexit_gen.storage_literal(row["global_address"]),
                f"0x{row['element_size']:X}", str(row["count"]),
                row["ctor_seam"], row["dtor_seam"]])
        elif row["target_name"] == VECTOR_CTOR_NAME:
            ctor_seam = ELEMENT_CTORS[row["ctor_name"]][1]
            dtor_seam = ELEMENT_DTORS[row["dtor_name"]]
            body = [f"    VectorCtorIterator({variable}, "
                    f"0x{row['element_size']:X}, {row['count']}, "
                    f"{ctor_seam}, {dtor_seam});"]
        elif row.get("opaque"):
            body = atexit_gen.wrapped_call(
                "    ", row["seam"],
                [atexit_gen.storage_literal(row["global_address"])]
                + [str(value) for value in row["arguments"]])
        else:
            seam = INIT_DOMAINS[row["target_name"]][2]
            body = [f"    {seam}({variable});"]
        lines.append("/*")
        lines.append(f"Purpose: Dynamic initializer for {row['global_name']}; "
                     "registers its ??__F teardown.")
        lines.append(f"Original Offset: {row['address']:08X}")
        lines.append("Return Value: n/a")
        lines.append("Status: Complete")
        lines.append("*/")
        lines.append(
            f"void __cdecl construct_{atexit_gen.snake(row['global_name'])}() {{")
        lines.extend(body)
        lines.append(f"    GameAtexit(reinterpret_cast<func_atexit_callback *>("
                     f"0x{row['registered']:08X}));")
        lines.append("}")
        lines.append("")
    return "\n".join(lines)


def opaque_scalars(rows):
    """The opaque scalar rows, grouped by arity in address order."""
    grouped = {}
    for row in opaque(rows):
        if row["target_name"] != VECTOR_CTOR_NAME:
            grouped.setdefault(row["arity"], []).append(row)
    return dict(sorted(grouped.items()))


def render_opaque_tables(rows) -> list[str]:
    """One case table per opaque arity, plus one for the opaque arrays.

    An opaque thunk carries no typed global to rebind, so the storage address
    itself is the expectation: rebind the construction seam and check the
    pointer and literals it is handed. That is also what proves two thunks
    sharing one seam - NetMsg, Palette, FontQueue - carry different addresses.
    """
    out = []
    for arity, entries in opaque_scalars(rows).items():
        suffix = f"_{'i' * arity}" if arity else ""
        out.append(f"struct InitOpaqueCase{suffix} {{")
        out.append("    void(__cdecl *thunk)();")
        out.append(f"    {opaque_ctor_typedef(arity)} **ctor_slot;")
        out.append("    void *storage;")
        for index in range(arity):
            out.append(f"    int a{index};")
        out.append("    func_atexit_callback *callback;")
        out.append("};")
        out.append(f"const InitOpaqueCase{suffix} "
                   f"g_init_opaque_cases{suffix}[] = {{")
        for row in entries:
            values = [atexit_gen.storage_literal(row["global_address"])]
            values += [str(value) for value in row["arguments"]]
            out.append(
                f"    {{&construct_{atexit_gen.snake(row['global_name'])}, "
                f"&{row['seam']},")
            out.append("     " + ", ".join(values) + ",")
            out.append(f"     reinterpret_cast<func_atexit_callback *>("
                       f"0x{row['registered']:08X}U)}},")
        out.append("};")
    arrays = [row for row in opaque(rows)
              if row["target_name"] == VECTOR_CTOR_NAME]
    if arrays:
        out.append("struct InitOpaqueArrayCase {")
        out.append("    void(__cdecl *thunk)();")
        out.append("    void *storage;")
        out.append("    uint32_t element_size;")
        out.append("    int count;")
        out.append("    func_thiscall_teardown **ctor_slot;")
        out.append("    func_thiscall_teardown **dtor_slot;")
        out.append("    func_atexit_callback *callback;")
        out.append("};")
        out.append("const InitOpaqueArrayCase g_init_opaque_array_cases[] = {")
        for row in arrays:
            out.append(
                f"    {{&construct_{atexit_gen.snake(row['global_name'])},")
            out.append(
                f"     {atexit_gen.storage_literal(row['global_address'])}, "
                f"0x{row['element_size']:X}, {row['count']},")
            out.append(f"     &{row['ctor_seam']}, &{row['dtor_seam']},")
            out.append(f"     reinterpret_cast<func_atexit_callback *>("
                       f"0x{row['registered']:08X}U)}},")
        out.append("};")
    return out


def render_opaque_observers(rows) -> list[str]:
    """One recorder per opaque arity; the nullary one reuses observe_init_ctor
    only for its shape, and keeps its own counters so an arity mismatch cannot
    pass by accident."""
    arities = [arity for arity in opaque_scalars(rows) if arity]
    if not arities:
        return []
    out = ["int g_init_opaque_args[4];",
           "#if defined(__GNUC__)",
           "#pragma GCC diagnostic push",
           '#pragma GCC diagnostic ignored "-Wattributes"',
           "#endif"]
    for arity in arities:
        parameters = ["void *object"] + [f"int a{i}" for i in range(arity)]
        out.extend(declaration(
            f"void __thiscall observe_opaque_ctor_{'i' * arity}",
            parameters, " {"))
        out.append("    g_init_constructed = object;")
        out.append("    ++g_init_ctor_calls;")
        for index in range(arity):
            out.append(f"    g_init_opaque_args[{index}] = a{index};")
        out.append("    g_init_atexit_calls_at_ctor = g_init_atexit_calls;")
        out.append("}")
    out.extend(["#if defined(__GNUC__)",
                "#pragma GCC diagnostic pop",
                "#endif"])
    return out


def render_opaque_loops(rows) -> list[str]:
    """The opaque halves of test_init_thunks: seam in, address and literals
    out, then exactly one registration of exactly the catalogued ??__F."""
    out = []
    for arity, _entries in opaque_scalars(rows).items():
        suffix = f"_{'i' * arity}" if arity else ""
        observer = (f"&observe_opaque_ctor_{'i' * arity}" if arity
                    else "&observe_init_ctor")
        out.append(f"    for (const InitOpaqueCase{suffix} &entry :")
        out.append(f"             g_init_opaque_cases{suffix}) {{")
        out.append(f"        {opaque_ctor_typedef(arity)} *const saved =")
        out.append("            *entry.ctor_slot;")
        out.append(f"        *entry.ctor_slot = {observer};")
        out.append("        g_init_ctor_calls = 0;")
        out.append("        g_init_constructed = nullptr;")
        out.append("        g_init_atexit_calls = 0;")
        out.append("        g_init_registered = nullptr;")
        out.append("        g_init_atexit_calls_at_ctor = -1;")
        out.append("        entry.thunk();")
        out.append("        expect(g_init_ctor_calls == 1);")
        out.append("        expect(g_init_constructed == entry.storage);")
        for index in range(arity):
            out.append(f"        expect(g_init_opaque_args[{index}] "
                       f"== entry.a{index});")
        out.append("        expect(g_init_atexit_calls == 1);")
        out.append("        expect(g_init_registered == entry.callback);")
        out.append("        expect(g_init_atexit_calls_at_ctor == 0);")
        out.append("        *entry.ctor_slot = saved;")
        out.append("    }")
    if any(row["target_name"] == VECTOR_CTOR_NAME for row in opaque(rows)):
        out.append("    for (const InitOpaqueArrayCase &entry :")
        out.append("             g_init_opaque_array_cases) {")
        out.append("        func_thiscall_teardown *const saved_ctor =")
        out.append("            *entry.ctor_slot;")
        out.append("        func_thiscall_teardown *const saved_dtor =")
        out.append("            *entry.dtor_slot;")
        out.append("        *entry.ctor_slot =")
        out.append("            reinterpret_cast<func_thiscall_teardown *>(")
        out.append("                &g_init_dtor_sentinel);")
        out.append("        *entry.dtor_slot =")
        out.append("            reinterpret_cast<func_thiscall_teardown *>(")
        out.append("                &g_init_dtor_sentinel) + 1;")
        out.append("        g_init_vector_calls = 0;")
        out.append("        g_init_atexit_calls = 0;")
        out.append("        g_init_registered = nullptr;")
        out.append("        g_init_atexit_calls_at_ctor = -1;")
        out.append("        entry.thunk();")
        out.append("        expect(g_init_vector_calls == 1);")
        out.append("        expect(g_init_vector_array == entry.storage);")
        out.append("        expect(g_init_vector_size == entry.element_size);")
        out.append("        expect(g_init_vector_count == entry.count);")
        cast = "               reinterpret_cast<func_thiscall_teardown *>("
        out.append("        expect(g_init_vector_ctor ==")
        out.append(cast)
        out.append("                   &g_init_dtor_sentinel));")
        out.append("        expect(g_init_vector_dtor ==")
        out.append(cast)
        out.append("                   &g_init_dtor_sentinel) + 1);")
        out.append("        expect(g_init_atexit_calls == 1);")
        out.append("        expect(g_init_registered == entry.callback);")
        out.append("        expect(g_init_atexit_calls_at_ctor == 0);")
        out.append("        *entry.ctor_slot = saved_ctor;")
        out.append("        *entry.dtor_slot = saved_dtor;")
        out.append("    }")
    return out


def render_tests(rows) -> str:
    """Table-driven fragment: observed construction seam + registration."""
    scalar_specs = sorted(
        {INIT_DOMAINS[row["target_name"]] for row in typed(rows)
         if row["target_name"] != VECTOR_CTOR_NAME},
        key=lambda spec: spec[2])
    out = []
    out.append("namespace {")
    out.append("struct InitThunkCase {")
    out.append("    void(__cdecl *thunk)();")
    out.append("    void *slot;                     // the global seam")
    out.append("    func_thiscall_teardown **ctor_slot;")
    out.append("    func_atexit_callback *callback; // the ??__F it registers")
    out.append("};")
    out.append("const InitThunkCase g_init_scalar_cases[] = {")
    for row in typed(rows):
        if row["target_name"] == VECTOR_CTOR_NAME:
            continue
        seam = INIT_DOMAINS[row["target_name"]][2]
        out.append(
            f"    {{&construct_{atexit_gen.snake(row['global_name'])}, "
            f"&{atexit_gen.variable_of(row)}, &{seam}, "
            f"reinterpret_cast<func_atexit_callback *>("
            f"0x{row['registered']:08X}U)}},")
    out.append("};")
    out.append("struct InitArrayCase {")
    out.append("    void(__cdecl *thunk)();")
    out.append("    void *slot;")
    out.append("    uint32_t element_size;")
    out.append("    int count;")
    out.append("    func_thiscall_teardown **ctor_slot;")
    out.append("    func_thiscall_teardown **dtor_slot;")
    out.append("    func_atexit_callback *callback;")
    out.append("};")
    out.append("const InitArrayCase g_init_array_cases[] = {")
    for row in typed(rows):
        if row["target_name"] != VECTOR_CTOR_NAME:
            continue
        ctor_seam = ELEMENT_CTORS[row["ctor_name"]][1]
        dtor_seam = ELEMENT_DTORS[row["dtor_name"]]
        out.append(
            f"    {{&construct_{atexit_gen.snake(row['global_name'])}, "
            f"&{atexit_gen.variable_of(row)}, 0x{row['element_size']:X}, "
            f"{row['count']}, &{ctor_seam}, &{dtor_seam}, "
            f"reinterpret_cast<func_atexit_callback *>("
            f"0x{row['registered']:08X}U)}},")
    out.append("};")
    out.extend(render_opaque_tables(rows))
    out.append("""
func_atexit_callback *g_init_registered;
int g_init_atexit_calls;
int g_init_atexit_calls_at_ctor = -1;
int __cdecl observe_game_atexit(func_atexit_callback *callback) {
    g_init_registered = callback;
    ++g_init_atexit_calls;
    return 0;
}
void *g_init_constructed;
int g_init_ctor_calls;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall observe_init_ctor(void *object) {
    g_init_constructed = object;
    ++g_init_ctor_calls;
    // Sampled so the order assertion below proves construction precedes
    // registration, the original's push-after-call order.
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
void *g_init_vector_array;
uint32_t g_init_vector_size;
int g_init_vector_count;
func_thiscall_teardown *g_init_vector_ctor;
func_thiscall_teardown *g_init_vector_dtor;
int g_init_vector_calls;
void __stdcall observe_vector_ctor(void *array, unsigned int element_size,
                                   int count, func_thiscall_teardown *ctor,
                                   func_thiscall_teardown *dtor) {
    g_init_vector_array = array;
    g_init_vector_size = element_size;
    g_init_vector_count = count;
    g_init_vector_ctor = ctor;
    g_init_vector_dtor = dtor;
    ++g_init_vector_calls;
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}
int g_init_dtor_sentinel;""")
    out.extend(render_opaque_observers(rows))
    out.append("""}  // namespace

void test_init_thunks() {
    func_game_atexit *const saved_atexit = GameAtexit;
    GameAtexit = &observe_game_atexit;

    // Scalar thunks: the construction seam must be handed exactly the
    // rebound global, once, before exactly one registration of exactly the
    // catalogued ??__F address.
    for (const InitThunkCase &entry : g_init_scalar_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_ctor = *entry.ctor_slot;
        *slot = fake;
        *entry.ctor_slot = &observe_init_ctor;
        g_init_ctor_calls = 0;
        g_init_constructed = nullptr;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_ctor_calls == 1);
        expect(g_init_constructed == fake);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved_ctor;
        *slot = saved_slot;
    }

    // Array thunks hand the walk to the iterator seam: the observed call
    // must carry the rebound array, the exact size and count, and both
    // rebound per-element seams - proving the body reads every seam rather
    // than any baked literal - before the one registration.
    auto *const saved_iterator = VectorCtorIterator;
    VectorCtorIterator = &observe_vector_ctor;
    for (const InitArrayCase &entry : g_init_array_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_ctor = *entry.ctor_slot;
        func_thiscall_teardown *const saved_dtor = *entry.dtor_slot;
        *slot = fake;
        *entry.ctor_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_init_dtor_sentinel);
        *entry.dtor_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_init_dtor_sentinel) + 1;
        g_init_vector_calls = 0;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_vector_calls == 1);
        expect(g_init_vector_array == fake);
        expect(g_init_vector_size == entry.element_size);
        expect(g_init_vector_count == entry.count);
        expect(g_init_vector_ctor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel));
        expect(g_init_vector_dtor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel) + 1);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved_ctor;
        *entry.dtor_slot = saved_dtor;
        *slot = saved_slot;
    }""")
    out.extend(render_opaque_loops(rows))
    out.append("""    VectorCtorIterator = saved_iterator;
    GameAtexit = saved_atexit;
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
    atexit_addresses = [address for address, row in functions.items()
                        if row["name"] == ATEXIT_NAME]
    if len(atexit_addresses) != 1:
        raise SystemExit("expected exactly one catalogued _atexit")

    rows, leftovers = collect(pe, functions)
    shared = atexit_side_globals(pe, functions)
    for row in typed(rows):
        variable = atexit_gen.variable_of(row)
        if variable in shared and shared[variable] != row["global_address"]:
            raise SystemExit(
                f"{variable}: init side 0x{row['global_address']:08X} vs "
                f"teardown side 0x{shared[variable]:08X}")

    # Seams dedupe on address against src/*.h, against the seams the two
    # element tables own (which carry no address comment for the scanner to
    # find), and against the ones atexit_thunks.h computes for its own opaque
    # rows. This file's output is excluded so a rerun does not read last run's
    # seams back as prior art.
    f_rows, _ = atexit_gen.collect(pe, functions)
    bindings = atexit_gen.merge_bindings(
        atexit_gen.load_bindings(REPO_ROOT / "src",
                                 exclude=("init_thunks.h",)),
        {**atexit_gen.side_bindings(pe, functions, REPO_ROOT / "src"),
         **table_bindings(rows, f_rows)})
    seams, declare, includes = atexit_gen.resolve_seams(
        opaque_needs(rows), bindings)
    # init_thunks.cpp already includes atexit_thunks.h, and a header may not
    # include itself; everything else a reused seam lives in is emitted.
    includes = [header for header in includes
                if header not in ("init_thunks.h", "atexit_thunks.h")]
    for row in opaque(rows):
        if row["target_name"] == VECTOR_CTOR_NAME:
            row["ctor_seam"] = seams[row["ctor_address"]]
            row["dtor_seam"] = seams[row["dtor_address"]]
        else:
            row["seam"] = seams[row["target_address"]]

    by_domain = {}
    for row in rows:
        label = kind_of(row)
        if label is None:
            label = "opaque"
        elif row["target_name"] == VECTOR_CTOR_NAME:
            label += "[]"
        if row.get("opaque") and row["target_name"] == VECTOR_CTOR_NAME:
            label = "opaque[]"
        by_domain.setdefault(label, []).append(row)
    print(f"thunks accepted: {len(rows)}")
    for domain, entries in sorted(by_domain.items(), key=lambda i: -len(i[1])):
        print(f"  {domain:14} {len(entries):4}")
    print(f"opaque seams: {len(declare)} defined here, "
          f"{len(seams) - len(declare)} reused")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers[:12]:
            print(f"  {address} {name[:44]:46} {reason[:44]}")

    names = [row["global_name"] for row in rows]
    if len(set(names)) != len(names):
        raise SystemExit("duplicate global names; refusing to emit")
    symbols = [atexit_gen.snake(row["global_name"]) for row in rows]
    if len(set(symbols)) != len(symbols):
        raise SystemExit("duplicate emitted symbols; refusing to emit")

    if args.dry_run:
        return 0

    source_dir = Path(args.source_dir)
    source_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "init_thunks.h").write_text(
        render_header(rows, shared, declare, includes))
    (source_dir / "init_thunks.cpp").write_text(
        render_source(rows, shared, atexit_addresses[0], declare))
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (args.scratch_dir / "init-thunk-tests.cpp").write_text(render_tests(rows))
    wire = "\n".join(
        f"0x{row['address']:08X} construct_{atexit_gen.snake(row['global_name'])}"
        for row in rows) + "\n"
    (args.scratch_dir / "init-wire.txt").write_text(wire)
    print(f"emitted {source_dir}/init_thunks.{{h,cpp}}, test fragment, "
          "wire list")
    return 0


if __name__ == "__main__":
    sys.exit(main())
