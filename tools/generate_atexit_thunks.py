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
here differs only in a name, an address, and which of four teardowns it calls.
Anything whose proven target is not one of those four is skipped and named,
never emitted.

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

REPO_ROOT = Path(__file__).resolve().parent.parent

LICENSE = """/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
"""

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


def snake(name: str) -> str:
    text = name[2:] if name.startswith("g_") else name
    return re.sub(r"__+", "_", text.lower())


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
            leftovers.append(
                (row["address"], row["name"], f"target {target_name or hex(forward.target)}"))
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


def kind_of(row):
    if row["target_name"] == VECTOR_DTOR_NAME:
        return ELEMENT_TEARDOWNS[row["teardown_name"]][0]
    return DOMAINS[row["target_name"]][0]


def render_header(rows) -> str:
    lines = [LICENSE, "#pragma once", ""]
    # Only pointers cross this header, so the element types stay forward
    # declarations; faction.h in particular does not stand alone, and nothing
    # here needs a complete type. The bodies include what they call.
    kinds = sorted({kind_of(row) for row in rows})
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
    for row in rows:
        lines.append(f"extern {kind_of(row)} *{variable_of(row)};")
    lines.append("")
    for row in rows:
        lines.append(
            f"DLLEXPORT void __cdecl destroy_{snake(row['global_name'])}();")
    lines.append("")
    return "\n".join(lines)


WAVE_DESTRUCTOR_ADDRESS = 0
VECTOR_DTOR_ADDRESS = 0


def render_source(rows) -> str:
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
    lines.append("")
    for row in rows:
        lines.append(f"{kind_of(row)} *{variable_of(row)} = "
                     f"({kind_of(row)} *)0x{row['global_address']:08X};")
    lines.append("")
    for row in rows:
        if row["target_name"] == VECTOR_DTOR_NAME:
            seam = ELEMENT_TEARDOWNS[row["teardown_name"]][2]
            body = (f"    VectorDtorIterator({variable_of(row)}, "
                    f"0x{row['element_size']:X}, {row['count']}, {seam});")
        else:
            call = DOMAINS[row["target_name"]][2]
            body = (f"    WaveOriginalDestructor({variable_of(row)});"
                    if call is None else f"    {variable_of(row)}{call};")
        lines.append("/*")
        lines.append(f"Purpose: Atexit teardown thunk for {row['global_name']}.")
        lines.append(f"Original Offset: {row['address']:08X}")
        lines.append("Return Value: n/a")
        lines.append("Status: Complete")
        lines.append("*/")
        lines.append(f"void __cdecl destroy_{snake(row['global_name'])}() {{")
        lines.append(body)
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
            out.append(f"    {{&destroy_{snake(row['global_name'])}, "
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
        out.append(f"    {{&destroy_{snake(row['global_name'])}, "
                   f"&{variable_of(row)}, 0x{row['element_size']:X}, "
                   f"{row['count']}, &{seam}}},")
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
int g_vector_sentinel;
}  // namespace

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

    SpriteFree = saved_sprite_free;
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

    by_domain = {}
    for row in rows:
        label = kind_of(row)
        if row["target_name"] == VECTOR_DTOR_NAME:
            label += "[]"
        by_domain.setdefault(label, []).append(row)
    print(f"thunks accepted: {len(rows)}")
    for domain, entries in sorted(by_domain.items(), key=lambda i: -len(i[1])):
        print(f"  {domain:12} {len(entries):4}")
    if leftovers:
        print(f"left alone: {len(leftovers)}")
        for address, name, reason in leftovers[:10]:
            print(f"  {address} {name[:44]:46} {reason[:40]}")

    names = [row["global_name"] for row in rows]
    if len(set(names)) != len(names):
        raise SystemExit("duplicate global names; refusing to emit")

    if args.dry_run:
        return 0

    (REPO_ROOT / "src" / "atexit_thunks.h").write_text(render_header(rows))
    (REPO_ROOT / "src" / "atexit_thunks.cpp").write_text(render_source(rows))
    args.scratch_dir.mkdir(parents=True, exist_ok=True)
    (args.scratch_dir / "atexit-thunk-tests.cpp").write_text(render_tests(rows))
    wire = "\n".join(
        f"0x{row['address']:08X} destroy_{snake(row['global_name'])}"
        for row in rows) + "\n"
    (args.scratch_dir / "atexit-wire.txt").write_text(wire)
    print("emitted src/atexit_thunks.{h,cpp}, test fragment, wire list")
    return 0


if __name__ == "__main__":
    sys.exit(main())
