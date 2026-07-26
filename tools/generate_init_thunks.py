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

REPO_ROOT = Path(__file__).resolve().parent.parent
LICENSE = atexit_gen.LICENSE

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


def second_call_target(pe, row) -> int | None:
    """The atexit call the acceptor does not itself verify."""
    instructions = delegates.trailing_padding(
        delegates.decode(pe, int(row["address"], 16), int(row["size"])))
    calls = [int(i.op_str, 16) for i in instructions
             if i.mnemonic == "call" and i.op_str.startswith("0x")]
    return calls[1] if len(calls) >= 2 else None


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
            if ctor_name not in ELEMENT_CTORS:
                leftovers.append((row["address"], row["name"],
                                  f"element ctor {ctor_name or hex(ctor)}"))
                continue
            if dtor_name not in ELEMENT_DTORS:
                leftovers.append((row["address"], row["name"],
                                  f"element dtor {dtor_name or hex(dtor)}"))
                continue
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
        if target_name not in INIT_DOMAINS:
            leftovers.append((row["address"], row["name"],
                              f"target {target_name or hex(forward.target)}"))
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


def kind_of(row) -> str:
    if row["target_name"] == VECTOR_CTOR_NAME:
        return ELEMENT_CTORS[row["ctor_name"]][0]
    return INIT_DOMAINS[row["target_name"]][0]


def atexit_side_globals(pe, functions) -> dict[str, int]:
    """global name -> address for every global atexit_thunks.h already binds."""
    f_rows, _ = atexit_gen.collect(pe, functions)
    return {atexit_gen.variable_of(row): row["global_address"]
            for row in f_rows}


def render_header(rows, shared_globals) -> str:
    lines = [LICENSE, "#pragma once", ""]
    kinds = sorted({kind_of(row) for row in rows})
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
    ctor_seams = sorted({ELEMENT_CTORS[row["ctor_name"]][1] for row in rows
                         if row["target_name"] == VECTOR_CTOR_NAME})
    for seam in ctor_seams:
        lines.append(f"extern func_thiscall_teardown *{seam};")
    lines.append("")
    lines.append("// Per-kind scalar construction seams, defaulting to")
    lines.append("// source-owned trampolines over the recovered")
    lines.append("// constructors. Rebindable because those constructors")
    lines.append("// reach their own deeper seams, whose defaults are")
    lines.append("// unmapped outside the game process.")
    scalar_seams = sorted({INIT_DOMAINS[row["target_name"]][2] for row in rows
                           if row["target_name"] != VECTOR_CTOR_NAME})
    for seam in scalar_seams:
        lines.append(f"extern func_thiscall_teardown *{seam};")
    lines.append("")
    for row in rows:
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


def render_source(rows, shared_globals, atexit_address) -> str:
    lines = [LICENSE, '#include "stdafx.h"', '#include <new>',
             '#include "init_thunks.h"', '#include "atexit_thunks.h"']
    scalar_specs = sorted(
        {INIT_DOMAINS[row["target_name"]] for row in rows
         if row["target_name"] != VECTOR_CTOR_NAME},
        key=lambda spec: spec[2])
    includes = {spec[1] for name, spec in INIT_DOMAINS.items()
                if any(row["target_name"] == name for row in rows)}
    for row in rows:
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
    for row in rows:
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
    lines.append("")
    for row in rows:
        variable = atexit_gen.variable_of(row)
        if variable in shared_globals:
            continue
        lines.append(f"{kind_of(row)} *{variable} = "
                     f"({kind_of(row)} *)0x{row['global_address']:08X};")
    lines.append("")
    for row in rows:
        variable = atexit_gen.variable_of(row)
        if row["target_name"] == VECTOR_CTOR_NAME:
            ctor_seam = ELEMENT_CTORS[row["ctor_name"]][1]
            dtor_seam = ELEMENT_DTORS[row["dtor_name"]]
            body = (f"    VectorCtorIterator({variable}, "
                    f"0x{row['element_size']:X}, {row['count']}, "
                    f"{ctor_seam}, {dtor_seam});")
        else:
            seam = INIT_DOMAINS[row["target_name"]][2]
            body = f"    {seam}({variable});"
        lines.append("/*")
        lines.append(f"Purpose: Dynamic initializer for {row['global_name']}; "
                     "registers its ??__F teardown.")
        lines.append(f"Original Offset: {row['address']:08X}")
        lines.append("Return Value: n/a")
        lines.append("Status: Complete")
        lines.append("*/")
        lines.append(
            f"void __cdecl construct_{atexit_gen.snake(row['global_name'])}() {{")
        lines.append(body)
        lines.append(f"    GameAtexit(reinterpret_cast<func_atexit_callback *>("
                     f"0x{row['registered']:08X}));")
        lines.append("}")
        lines.append("")
    return "\n".join(lines)


def render_tests(rows) -> str:
    """Table-driven fragment: observed construction seam + registration."""
    scalar_specs = sorted(
        {INIT_DOMAINS[row["target_name"]] for row in rows
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
    for row in rows:
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
    for row in rows:
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
int g_init_dtor_sentinel;
}  // namespace

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
    }
    VectorCtorIterator = saved_iterator;
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
    for row in rows:
        variable = atexit_gen.variable_of(row)
        if variable in shared and shared[variable] != row["global_address"]:
            raise SystemExit(
                f"{variable}: init side 0x{row['global_address']:08X} vs "
                f"teardown side 0x{shared[variable]:08X}")

    by_domain = {}
    for row in rows:
        label = kind_of(row)
        if row["target_name"] == VECTOR_CTOR_NAME:
            label += "[]"
        by_domain.setdefault(label, []).append(row)
    print(f"thunks accepted: {len(rows)}")
    for domain, entries in sorted(by_domain.items(), key=lambda i: -len(i[1])):
        print(f"  {domain:14} {len(entries):4}")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers[:12]:
            print(f"  {address} {name[:44]:46} {reason[:44]}")

    names = [row["global_name"] for row in rows]
    if len(set(names)) != len(names):
        raise SystemExit("duplicate global names; refusing to emit")

    if args.dry_run:
        return 0

    (REPO_ROOT / "src" / "init_thunks.h").write_text(
        render_header(rows, shared))
    (REPO_ROOT / "src" / "init_thunks.cpp").write_text(
        render_source(rows, shared, atexit_addresses[0]))
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (args.scratch_dir / "init-thunk-tests.cpp").write_text(render_tests(rows))
    wire = "\n".join(
        f"0x{row['address']:08X} construct_{atexit_gen.snake(row['global_name'])}"
        for row in rows) + "\n"
    (args.scratch_dir / "init-wire.txt").write_text(wire)
    print("emitted src/init_thunks.{h,cpp}, test fragment, wire list")
    return 0


if __name__ == "__main__":
    sys.exit(main())
