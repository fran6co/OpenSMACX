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
}

THUNK_NAME_RE = re.compile(r"^\?\?__F(\w+)@@YAXXZ$")


def snake(name: str) -> str:
    text = name[2:] if name.startswith("g_") else name
    return re.sub(r"__+", "_", text.lower())


def collect(pe, functions):
    """(domain rows, leftovers): every unrecovered ??__F, proven and sorted."""
    rows = []
    leftovers = []
    for row in delegates.load_rows(None, "unrecovered"):
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


def render_header(rows) -> str:
    lines = [LICENSE, "#pragma once", ""]
    includes = sorted({DOMAINS[row["target_name"]][1] for row in rows})
    for include in includes:
        lines.append(f'#include "{include}"')
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
    for row in rows:
        kind = DOMAINS[row["target_name"]][0]
        lines.append(f"extern {kind} *{row['global_name']};")
    lines.append("")
    for row in rows:
        lines.append(
            f"DLLEXPORT void __cdecl destroy_{snake(row['global_name'])}();")
    lines.append("")
    return "\n".join(lines)


WAVE_DESTRUCTOR_ADDRESS = 0


def render_source(rows) -> str:
    lines = [LICENSE, '#include "stdafx.h"', '#include "atexit_thunks.h"', ""]
    lines.append("func_wave_destructor *WaveOriginalDestructor =")
    lines.append(f"    (func_wave_destructor *)0x{WAVE_DESTRUCTOR_ADDRESS:08X};")
    lines.append("")
    for row in rows:
        kind = DOMAINS[row["target_name"]][0]
        lines.append(f"{kind} *{row['global_name']} = "
                     f"({kind} *)0x{row['global_address']:08X};")
    lines.append("")
    for row in rows:
        call = DOMAINS[row["target_name"]][2]
        body = (f"    WaveOriginalDestructor({row['global_name']});"
                if call is None else f"    {row['global_name']}{call};")
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
            ("??1Wave@@QAE@XZ", "g_atexit_wave_cases")):
        out.append(f"const AtexitThunkCase {table}[] = {{")
        for row in cases(target_name):
            out.append(f"    {{&destroy_{snake(row['global_name'])}, "
                       f"&{row['global_name']}}},")
        out.append("};")
    out.append("""
Wave *g_atexit_wave_seen;
int g_atexit_wave_calls;
void __thiscall observe_wave_destructor(Wave *wave) {
    g_atexit_wave_seen = wave;
    ++g_atexit_wave_calls;
}
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
    rows, leftovers = collect(pe, functions)

    by_domain = {}
    for row in rows:
        by_domain.setdefault(DOMAINS[row["target_name"]][0], []).append(row)
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
