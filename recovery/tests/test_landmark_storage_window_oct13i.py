"""Bounded controls for the chosen landmark byte-storage window.

These tests inspect the actual source declarations, original-object bindings,
and renderer/producer bodies. Host-compiled controls extract the actual window,
initializer and byte helper, substituting only synthetic include contents;
they are not native ABI or full-runtime proof. The optional pinned-image
control uses ignored generated includes and compares the parent bytes in
memory; it publishes no proprietary bytes.
"""
from hashlib import sha256
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

import pytest

from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from test_object_data import data_image


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PARENT = "?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A"
WINDOW_FIELDS = (
    ("mount_planet", 0x000, 98),
    ("gap_mount_mesa", 0x062, 2),
    ("sunny_mesa", 0x064, 98),
    ("gap_mesa_point_light", 0x0C6, 2),
    ("alt_light_bit2", 0x0C8, 113),
    ("gap_point_light_crater", 0x139, 3),
    ("garland_crater", 0x13C, 98),
    ("gap_crater_ocean", 0x19E, 0x72),
    ("ocean_corner_uvs", 0x210, 128),
)
OBJECT_IDENTITIES = (
    (0x685A98, 0x685AFA, "_MountPlanetTable", "mount_planet"),
    (0x685AFC, 0x685B5E, "_SunnyMesaTable", "sunny_mesa"),
    (0x685B60, None, "_AltLightBit2", "alt_light_bit2"),
    (0x685BD4, 0x685C36, "_GarlandCraterTable", "garland_crater"),
    (0x685CA8, 0x685D28, "_OceanCornerUVs", "ocean_corner_uvs"),
)
GAPS = (
    ("terrain_landmark_gap_mount_mesa.inc", 0x685AFA, 2),
    ("terrain_landmark_gap_mesa_point_light.inc", 0x685B5E, 2),
    ("terrain_landmark_gap_point_light_crater.inc", 0x685BD1, 3),
    ("terrain_landmark_gap_crater_ocean.inc", 0x685C36, 0x72),
)


def balanced_body(text, signature):
    start = text.index(signature)
    first = text.index("{", start)
    depth = 0
    for end in range(first, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body for {signature}")


def read_sources():
    return {
        "mapwin": (ROOT / "src/mapwin.cpp").read_text(),
        "map": (ROOT / "src/map.cpp").read_text(),
        "console": (ROOT / "src/console.cpp").read_text(),
    }


def expected_bindings(text):
    rows = parse_objects(text, "src/mapwin.cpp", PIN, address_space="pe:va")
    selected = [row for row in rows if row.symbol in {item[2] for item in OBJECT_IDENTITIES}]
    assert len(selected) == len(OBJECT_IDENTITIES)
    by_symbol = {row.symbol: row for row in selected}
    assert len(by_symbol) == len(OBJECT_IDENTITIES)
    for start, end, symbol, member in OBJECT_IDENTITIES:
        row = by_symbol[symbol]
        assert row.origin.address == start
        if end is None:
            assert row.span is None
        else:
            assert row.span is not None and row.span.start == start and row.span.end == end
        assert row.parent_symbol == PARENT and row.member_path == (member,)
        assert row.target_symbol is None
    return by_symbol


def validate_source_contract(sources):
    cpp = sources["mapwin"]
    map_cpp = sources["map"]
    window = re.search(r"struct TerrainLandmarkStorageWindow\s*\{(?P<body>.*?)\};", cpp, re.S)
    assert window, "actual aggregate declaration missing"
    fields = []
    for declaration in window.group("body").split(";"):
        match = re.search(r"\b([A-Za-z_][A-Za-z_0-9]*)\s*\[(0x[0-9A-Fa-f]+|[0-9]+)\]\s*$", declaration.strip())
        if match:
            fields.append((match.group(1), int(match.group(2), 0)))
    expected_counts = (("mount_planet", 49), ("gap_mount_mesa", 2),
                       ("sunny_mesa", 49), ("gap_mesa_point_light", 2),
                       ("alt_light_bit2", 113), ("gap_point_light_crater", 3),
                       ("garland_crater", 49), ("gap_crater_ocean", 0x72),
                       ("ocean_corner_uvs", 16))
    assert tuple(fields) == expected_counts
    assert "offsetof(TerrainLandmarkStorageWindow, mount_planet) == 0x000" in cpp
    assert "offsetof(TerrainLandmarkStorageWindow, sunny_mesa) == 0x064" in cpp
    assert "offsetof(TerrainLandmarkStorageWindow, alt_light_bit2) == 0x0C8" in cpp
    assert "offsetof(TerrainLandmarkStorageWindow, garland_crater) == 0x13C" in cpp
    assert "offsetof(TerrainLandmarkStorageWindow, ocean_corner_uvs) == 0x210" in cpp
    assert "sizeof(TerrainLandmarkStorageWindow) == 0x290" in cpp

    expected_bindings(cpp)
    assert "TerrainLandmarkStorageWindow TerrainLandmarkStorageWindowData = {" in cpp
    for filename, _, _ in GAPS:
        assert f'#include "{filename}"' in cpp

    # Renderer uses the aggregate's complete object representation, not a
    # pointer stepped beyond one of its array subobjects.
    body = balanced_body(cpp, "int __cdecl gen_terrain_poly(")
    assert "MountPlanetTable[" not in body
    assert "SunnyMesaTable[" not in body
    assert "GarlandCraterTable[" not in body
    assert "local_94 < 49" not in body and "iVar5 < 49" not in body
    branch_specs = (("mount_planet", "local_94"),
                    ("sunny_mesa", "iVar5"),
                    ("garland_crater", "iVar5"))
    for name, variable in branch_specs:
        offset = (f"offsetof(TerrainLandmarkStorageWindow, {name}) +\n"
                  f"                static_cast<size_t>({variable}) * sizeof(LandmarkTextureChoice)")
        assert offset in body
    for name, variable, pair_name in (("mount_planet", "local_94", "mount"),
                                      ("sunny_mesa", "iVar5", "mesa"),
                                      ("garland_crater", "iVar5", "crater")):
        texture_read = (f"terrain_window_signed_byte(\n"
                        f"                offsetof(TerrainLandmarkStorageWindow, {name}) +\n"
                        f"                static_cast<size_t>({variable}) * sizeof(LandmarkTextureChoice))")
        rotation_read = (f"terrain_window_signed_byte(\n"
                         f"                offsetof(TerrainLandmarkStorageWindow, {name}) +\n"
                         f"                static_cast<size_t>({variable}) * sizeof(LandmarkTextureChoice) + 1)")
        texture_at = body.index(texture_read)
        rotation_at = body.index(rotation_read, texture_at + len(texture_read))
        assert body.index(f"-1 < {variable}") < texture_at
        assert body.index("!= -1", texture_at, rotation_at) >= texture_at
    signed_byte = balanced_body(cpp, "static inline int terrain_window_signed_byte(size_t offset) {")
    assert "reinterpret_cast<const unsigned char *>(" in signed_byte
    assert "&TerrainLandmarkStorageWindowData" in signed_byte
    assert "bytes[offset]" in signed_byte
    assert "value < 0x80 ? value : static_cast<int>(value) - 0x100" in signed_byte
    assert "&TerrainLandmarkStorageWindowData.mount_planet" not in signed_byte
    assert body.count("terrain_window_signed_byte(") >= 9

    # The mapped producer preserves existing feature bits, then writes the
    # signed code into only the high byte. Monsoon publishes 0..120, including
    # the aliased renderer row 119; this is source-flow evidence, not proof
    # that every path executes in a full game.
    bit2 = balanced_body(map_cpp, "void __cdecl bit2_set(int x, int y, int bit2, BOOL set) {")
    code = balanced_body(map_cpp, "void __cdecl code_set(int x, int y, int code) {")
    monsoon = balanced_body(map_cpp, "void __cdecl world_monsoon(int x, int y) {")
    assert "*field |= bit2;" in bit2 and "*field &= ~bit2;" in bit2
    assert "(*field & 0xFFFFFF) | (code << 24)" in code
    assert re.search(r"for \(int i = 0; i < RadiusRange\[5\]; i\+\+\).*?\n\s*new_landmark", monsoon, re.S)
    assert "const int RadiusRange[9] = { 1, 9, 25, 49, 81, 121, 169, 225, 289 };" in map_cpp
    assert "BIT2_JUNGLE, true" in monsoon and "code_set(x_radius, y_radius, i)" in monsoon
    assert monsoon.index("bit2_set(x_radius, y_radius, BIT2_JUNGLE, true)") < monsoon.index("code_set(x_radius, y_radius, i)")
    assert "i < 21 && is_ocean(x_radius, y_radius)" in monsoon


def test_actual_storage_layout_bindings_renderer_and_producer_contract():
    validate_source_contract(read_sources())


@pytest.mark.parametrize("damage", [
    ("gap_mount_mesa", "gap_mount_mesa[2]", "gap_mount_mesa[3]"),
    ("wrong_member", "member ?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A sunny_mesa",
     "member ?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A garland_crater"),
    ("missing_member", "// RECOVERY_OBJECT: v3 0x00685CA8 0x00685D28 _OceanCornerUVs 0x004632D0 member ?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A ocean_corner_uvs\n", ""),
    ("wrong_alias_base", "offsetof(TerrainLandmarkStorageWindow, sunny_mesa)",
     "offsetof(TerrainLandmarkStorageWindow, garland_crater)"),
    ("upper_guard", "static_cast<size_t>(local_94) * sizeof(LandmarkTextureChoice)",
     "static_cast<size_t>(local_94) * sizeof(LandmarkTextureChoice); if (local_94 < 49)"),
    ("subobject_view", "&TerrainLandmarkStorageWindowData);\n    const unsigned char value = bytes[offset];",
     "&TerrainLandmarkStorageWindowData.mount_planet);\n    const unsigned char value = bytes[offset];"),
])
def test_contract_rejects_layout_alias_guard_and_subobject_damage(damage):
    sources = read_sources()
    name, before, after = damage
    cpp = sources["mapwin"]
    assert before in cpp, name
    sources["mapwin"] = cpp.replace(before, after, 1)
    with pytest.raises(AssertionError):
        validate_source_contract(sources)


def test_signed_code_window_and_row119_aliases_are_bounded_by_full_backing():
    offsets = {name: start for name, start, _ in WINDOW_FIELDS}
    assert offsets == {
        "mount_planet": 0, "gap_mount_mesa": 0x62, "sunny_mesa": 0x64,
        "gap_mesa_point_light": 0xC6, "alt_light_bit2": 0xC8,
        "gap_point_light_crater": 0x139, "garland_crater": 0x13C,
        "gap_crater_ocean": 0x19E, "ocean_corner_uvs": 0x210,
    }
    storage = bytes((i * 29 + 7) & 0xFF for i in range(0x290))
    for name in ("mount_planet", "sunny_mesa", "garland_crater"):
        base = offsets[name]
        for index in range(128):
            pair = base + 2 * index
            assert pair + 1 < len(storage)
            signed = tuple(value if value < 0x80 else value - 0x100
                           for value in storage[pair:pair + 2])
            assert all(-128 <= value <= 127 for value in signed)
    # Native code-byte 0x80..0xFF is negative and skipped before table access;
    # 0..127 is the entire unsigned-positive lookup domain. Texture -1 alone
    # is the sentinel; rotations remain signed data, not sentinel tests.
    assert all(not (-1 < index) for index in (-128, -1))
    assert all(-1 < index for index in range(128))
    signed_texture_ff = 0xFF if 0xFF < 0x80 else 0xFF - 0x100
    signed_texture_zero = 0 if 0 < 0x80 else 0 - 0x100
    assert signed_texture_ff == -1 and signed_texture_zero == 0
    assert signed_texture_zero != -1  # zero remains drawable; only -1 suppresses.
    # The pinned aliases are structural addresses, with only the mapped
    # 49-row windows asserted; bytes are not duplicated into source.
    assert (0x685A98 + 2 * 119) == 0x685B86  # Mount -> AltLightBit2 + 38
    assert (0x685AFC + 2 * 119) == 0x685BEA  # Mesa -> Crater + 11
    assert (0x685BD4 + 2 * 119) == 0x685CC2  # Crater -> Ocean UV bytes + 3
    assert 0x685B86 - 0x685B60 == 38
    assert 0x685BEA - 0x685BD4 == 22
    assert 0x685CC2 - 0x685CA8 == 26


def _signed_byte(value):
    return value if value < 0x80 else value - 0x100


def _synthetic_initializer_includes():
    tables = {}
    for name, seed in (("terrain_mount_planet.inc", 17),
                       ("terrain_sunny_mesa.inc", 51),
                       ("terrain_garland_crater.inc", 93)):
        values = [((index * 37 + seed) & 0xFF) for index in range(98)]
        values[14] = 0xFF  # Texture -1 sentinel in each original 49-row table.
        tables[name] = values
    # Make the three cross-owner index-119 reads distinct and drawable:
    # Mount reads AltLightBit2[38], Mesa reads Crater[11].texture, and Crater
    # reads byte 2 of OceanCornerUVs[3].x.
    point_light = [((index * 23 + 9) & 0xFF) for index in range(113)]
    point_light[38] = 0x81
    crater = tables["terrain_garland_crater.inc"]
    crater[22] = 0x32
    gaps = {
        "terrain_landmark_gap_mount_mesa.inc": [0xA1, 0x27],
        "terrain_landmark_gap_mesa_point_light.inc": [0x18, 0xF2],
        "terrain_landmark_gap_point_light_crater.inc": [0x51, 0x82, 0x14],
        "terrain_landmark_gap_crater_ocean.inc": [((index * 11 + 5) & 0xFF)
                                                    for index in range(0x72)],
    }
    lines = {}
    for filename, values in tables.items():
        pairs = ["{%d, %d}," % (_signed_byte(values[i]), _signed_byte(values[i + 1]))
                 for i in range(0, len(values), 2)]
        lines[filename] = "\n".join(pairs) + "\n"
    lines["terrain_point_light_bit2.inc"] = ", ".join(
        str(_signed_byte(value)) for value in point_light) + ",\n"
    for filename, values in gaps.items():
        lines[filename] = ", ".join(f"0x{value:02x}" for value in values) + ",\n"
    return lines


def _extract_parent_cpp(mapwin_text, tmp_path, *, signed_decode_damage=False,
                        subobject_damage=False, layout_damage=False,
                        condition_damage=None, expected_window=None):
    choice_start = mapwin_text.index("struct LandmarkTextureChoice {")
    helper = balanced_body(mapwin_text,
                           "static inline int terrain_window_signed_byte(size_t offset) {")
    helper_start = mapwin_text.index(helper, choice_start)
    snippet = mapwin_text[choice_start:helper_start + len(helper)]
    if layout_damage:
        snippet = snippet.replace("gap_mount_mesa[2]", "gap_mount_mesa[3]", 1)
    if signed_decode_damage:
        snippet = snippet.replace(
            "return value < 0x80 ? value : static_cast<int>(value) - 0x100;",
            "return static_cast<int>(value);", 1)
    if subobject_damage:
        snippet = snippet.replace(
            "&TerrainLandmarkStorageWindowData);",
            "&TerrainLandmarkStorageWindowData.mount_planet[0]);", 1)

    renderer = balanced_body(mapwin_text, "int __cdecl gen_terrain_poly(")
    condition_markers = {
        "mount_planet": "offsetof(TerrainLandmarkStorageWindow, mount_planet)",
        "sunny_mesa": "offsetof(TerrainLandmarkStorageWindow, sunny_mesa)",
        "garland_crater": "offsetof(TerrainLandmarkStorageWindow, garland_crater)",
    }
    conditions = {}
    for family, marker in condition_markers.items():
        marker_at = renderer.index(marker)
        if_at = renderer.rfind("if (", 0, marker_at)
        opening = renderer.index("(", if_at)
        depth = 0
        for end in range(opening, len(renderer)):
            depth += (renderer[end] == "(") - (renderer[end] == ")")
            if depth == 0:
                conditions[family] = renderer[opening + 1:end]
                break
        else:
            raise AssertionError(f"unterminated actual {family} gate")
    if condition_damage:
        family, damaged = condition_damage
        conditions[family] = damaged

    extracted_helper = balanced_body(
        snippet, "static inline int terrain_window_signed_byte(size_t offset) {")
    pointer = re.search(r"reinterpret_cast<const unsigned char \*>\((.*?)\);",
                        extracted_helper, re.S)
    assert pointer, "actual helper's parent object representation view missing"
    pointer_expression = pointer.group(1).strip()
    gate_functions = []
    for family, index_name, mask in (("mount_planet", "local_94", 2),
                                     ("sunny_mesa", "iVar5", 0x100),
                                     ("garland_crater", "iVar5", 1)):
        gate_functions.append(f"""
bool {family}_gate(int index, int flags, int hidden) {{
    const int {index_name} = index;
    const int local_70 = hidden;
    const int local_34 = flags;
    return ({conditions[family]});
}}
""")

    expected_window_check = ""
    if expected_window is not None:
        assert len(expected_window) == 0x290
        expected_values = ", ".join(f"0x{value:02x}" for value in expected_window)
        expected_window_check = f"""
    const unsigned char expected_window[0x290] = {{{expected_values}}};
    for (size_t offset = 0; offset < sizeof(expected_window); ++offset)
        check(raw[offset] == expected_window[offset], "pinned parent initialization bytes");
"""

    return f"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <stddef.h>
#include "{(ROOT / 'src/vert.h').as_posix()}"
{snippet}
{''.join(gate_functions)}

static int expected_signed(unsigned char value) {{
    return value < 0x80 ? static_cast<int>(value)
                        : static_cast<int>(value) - 0x100;
}}
static void check(bool value, const char *label) {{
    if (!value) {{ std::fprintf(stderr, "CHECK FAILED: %s\\n", label); std::exit(92); }}
}}
int main() {{
    static_assert(sizeof(TerrainLandmarkStorageWindow) == 656, "full window extent");
    static_assert(std::is_pod<TerrainLandmarkStorageWindow>::value,
                  "actual table window remains a static-initializable POD aggregate");
    static_assert(std::is_trivial<TerrainLandmarkStorageWindow>::value,
                  "table window has no dynamic constructor or destructor");
    static_assert(offsetof(TerrainLandmarkStorageWindow, mount_planet) == 0, "mount offset");
    static_assert(offsetof(TerrainLandmarkStorageWindow, sunny_mesa) == 100, "mesa offset");
    static_assert(offsetof(TerrainLandmarkStorageWindow, alt_light_bit2) == 200, "point-light offset");
    static_assert(offsetof(TerrainLandmarkStorageWindow, garland_crater) == 316, "crater offset");
    static_assert(offsetof(TerrainLandmarkStorageWindow, ocean_corner_uvs) == 528, "ocean offset");
    const unsigned char *const raw = reinterpret_cast<const unsigned char *>(
        &TerrainLandmarkStorageWindowData);
    const unsigned char *const actual_helper_view = reinterpret_cast<const unsigned char *>(
        {pointer_expression});
    const char *const actual_helper_pointer_expression = "{pointer_expression}";
    check(std::strcmp(actual_helper_pointer_expression,
                      "&TerrainLandmarkStorageWindowData") == 0,
          "helper reads through complete parent object, not an array subobject");
    check(__builtin_object_size(actual_helper_view, 0) == sizeof(TerrainLandmarkStorageWindow),
          "helper views complete parent object");
    volatile uintptr_t mount119 = reinterpret_cast<uintptr_t>(raw + 238);
    volatile uintptr_t point_light38 = reinterpret_cast<uintptr_t>(
        raw + offsetof(TerrainLandmarkStorageWindow, alt_light_bit2) + 38);
    volatile uintptr_t mesa119 = reinterpret_cast<uintptr_t>(raw + 100 + 238);
    volatile uintptr_t crater11 = reinterpret_cast<uintptr_t>(
        raw + offsetof(TerrainLandmarkStorageWindow, garland_crater) + 22);
    volatile uintptr_t crater119 = reinterpret_cast<uintptr_t>(raw + 316 + 238);
    volatile uintptr_t ocean_uv3_x_byte2 = reinterpret_cast<uintptr_t>(
        raw + offsetof(TerrainLandmarkStorageWindow, ocean_corner_uvs) + 26);
    check(mount119 == point_light38,
          "mount row 119 aliases point-light byte 38");
    check(mesa119 == crater11,
          "mesa row 119 aliases crater row 11");
    check(crater119 == ocean_uv3_x_byte2,
          "crater row 119 aliases ocean UV byte");
    check(terrain_window_signed_byte(238) == expected_signed(raw[238]),
          "mount row 119 signed decode");
    check(terrain_window_signed_byte(100 + 238) == expected_signed(raw[100 + 238]),
          "mesa row 119 signed decode");
    check(terrain_window_signed_byte(316 + 238) == expected_signed(raw[316 + 238]),
          "crater row 119 signed decode");

    const size_t bases[3] = {{0, 100, 316}};
    const int masks[3] = {{2, 0x100, 1}};
    bool (*gates[3])(int, int, int) = {{mount_planet_gate, sunny_mesa_gate,
                                      garland_crater_gate}};
    for (int family = 0; family < 3; ++family) {{
        for (int index = -1; index <= 127; ++index) {{
            if (index < 0) {{
                check(!gates[family](index, masks[family], 0), "negative index short-circuit");
                continue;
            }}
            const size_t offset = bases[family] + static_cast<size_t>(index) * 2;
            check(offset + 1 < sizeof(TerrainLandmarkStorageWindow), "full-parent index bound");
            const int texture = expected_signed(raw[offset]);
            const int rotation = expected_signed(raw[offset + 1]);
            check(terrain_window_signed_byte(offset) == texture, "texture signed-byte read");
            check(terrain_window_signed_byte(offset + 1) == rotation, "rotation signed-byte read");
            const bool expected_gate = texture != -1;
            check(gates[family](index, masks[family], 0) == expected_gate,
                  "actual caller gate preserves index and sentinel protocol");
            check(!gates[family](index, masks[family], 1), "hidden caller gate");
        }}
    }}
    check(mount_planet_gate(119, 2, 0), "high index mount gate");
    check(sunny_mesa_gate(119, 0x100, 0), "high index mesa gate");
    check(garland_crater_gate(119, 1, 0), "high index crater gate");
    check(!mount_planet_gate(7, 0, 0), "missing feature bit");
    check(!sunny_mesa_gate(7, 0, 0), "missing mesa feature bit");
    check(!garland_crater_gate(7, 0, 0), "missing crater feature bit");
    {expected_window_check}
    return 0;
}}
"""


def _write_host_fixture(tmp_path, source, includes):
    for filename, contents in includes.items():
        (tmp_path / filename).write_text(contents)
    source_path = tmp_path / "landmark_window_actual_source.cpp"
    source_path.write_text(source)
    return source_path


def _available_host_compilers():
    return [(name, shutil.which(name)) for name in ("g++", "clang++")
            if shutil.which(name)]


def _compile_run_host(tmp_path, compiler, source_path):
    binary = tmp_path / ("landmark-" + Path(compiler).name)
    command = [compiler, "-std=c++11", "-O1", "-g", "-Wall", "-Wextra",
               "-Werror", "-Wno-missing-braces",
               "-fsanitize=address,undefined,bounds,object-size",
               "-fno-sanitize-recover=all", str(source_path), "-o", str(binary)]
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=45)
    return compiled, (subprocess.run([str(binary)], capture_output=True, text=True,
                                     timeout=30, env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
                      if compiled.returncode == 0 else None)


@pytest.mark.parametrize("compiler_name,compiler", _available_host_compilers())
def test_actual_extracted_cpp_window_and_lookup_run_0_to_127(tmp_path, compiler_name, compiler):
    cpp = (ROOT / "src/mapwin.cpp").read_text()
    fixture = _extract_parent_cpp(cpp, tmp_path)
    source = _write_host_fixture(tmp_path, fixture, _synthetic_initializer_includes())
    compiled, ran = _compile_run_host(tmp_path, compiler, source)
    assert compiled.returncode == 0, (compiler_name, compiled.stderr)
    assert ran.returncode == 0, (compiler_name, ran.stdout, ran.stderr)


@pytest.mark.parametrize("compiler_name,compiler", _available_host_compilers())
@pytest.mark.parametrize("damage", ["wrong_layout", "wrong_subobject", "wrong_signed_decode", "upper49_guard"])
def test_actual_cpp_control_rejects_compilable_or_layout_mutants(
        tmp_path, compiler_name, compiler, damage):
    cpp = (ROOT / "src/mapwin.cpp").read_text()
    options = {}
    if damage == "wrong_layout":
        options["layout_damage"] = True
    elif damage == "wrong_subobject":
        options["subobject_damage"] = True
    elif damage == "wrong_signed_decode":
        options["signed_decode_damage"] = True
    else:
        renderer = balanced_body(cpp, "int __cdecl gen_terrain_poly(")
        marker = "offsetof(TerrainLandmarkStorageWindow, mount_planet)"
        offset = renderer.index(marker)
        if_at = renderer.rfind("if (", 0, offset)
        opening = renderer.index("(", if_at)
        depth = 0
        for end in range(opening, len(renderer)):
            depth += (renderer[end] == "(") - (renderer[end] == ")")
            if depth == 0:
                condition = renderer[opening + 1:end]
                break
        damaged = condition.replace("-1 < local_94", "-1 < local_94 && local_94 < 49", 1)
        assert damaged != condition
        options["condition_damage"] = ("mount_planet", damaged)
    fixture = _extract_parent_cpp(cpp, tmp_path, **options)
    source = _write_host_fixture(tmp_path, fixture, _synthetic_initializer_includes())
    compiled, ran = _compile_run_host(tmp_path, compiler, source)
    if damage == "wrong_layout":
        assert compiled.returncode != 0, (compiler_name, "layout damage compiled")
        assert "landmark storage window member offsets" in compiled.stderr
    else:
        assert compiled.returncode == 0, (compiler_name, compiled.stderr)
        assert ran is not None and ran.returncode != 0, (compiler_name, damage, ran)
        if damage == "wrong_signed_decode":
            assert "mount row 119 signed decode" in ran.stderr
        elif damage == "upper49_guard":
            assert "actual caller gate preserves index and sentinel protocol" in ran.stderr
        else:
            assert "complete parent object" in ran.stderr or "runtime error" in ran.stderr


def _gap_hook():
    spec = importlib.util.spec_from_file_location(
        "terrain_data_under_test", ROOT / "recovery/terrain_data.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _wide_data_image(*, raw_size=0x2000, section_flags=0xC0000040,
                     relocation_site=None):
    """A synthetic, raw-backed data section covering the fixed gap VAs."""
    raw = bytearray(data_image(bytes(4)))
    section = 0x178
    payload_offset = 512
    base_va = 0x685000
    section_rva = 0x285000  # image base 0x400000 => VA 0x685000
    payload_size = 0x2000
    raw.extend(bytes(payload_offset + payload_size - len(raw)))
    struct.pack_into("<II", raw, section + 8, payload_size, section_rva)
    struct.pack_into("<I", raw, section + 16, raw_size)
    struct.pack_into("<I", raw, section + 20, payload_offset)
    raw[section + 36:section + 40] = struct.pack("<I", section_flags)
    for _, offset, size in GAPS:
        relative = offset - 0x685000
        raw[payload_offset + relative:payload_offset + relative + size] = bytes(
            ((relative + i) * 17 + 3) & 0xFF for i in range(size))
    if relocation_site is not None:
        relocation_rva = section_rva + 0x1F00
        relocation_file = payload_offset + 0x1F00
        relocation_delta = relocation_site - base_va
        struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, relocation_rva, 12)
        struct.pack_into("<IIHH", raw, relocation_file, section_rva, 12,
                         0x3000 | relocation_delta, 0)
    return bytes(raw)


def _synthetic_project(tmp_path, image_bytes):
    root = tmp_path / "project"
    (root / "src").mkdir(parents=True)
    (root / "recovery").mkdir()
    (root / ".gitignore").write_text("/build/\n")
    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    (root / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
                   "seed_roots": ["src/recovered", "src/unrecovered"],
                   "symbol_format": "msvc"},
    }))
    image = tmp_path / "synthetic.exe"
    image.write_bytes(image_bytes)
    (root / "src/IMAGE_PIN").write_text(f"sha256 {sha256(image_bytes).hexdigest()}\n")
    parent = PARENT
    rows = []
    for start, end, symbol, member in OBJECT_IDENTITIES:
        if end is None:
            rows.append(f"// RECOVERY_OBJECT: v3 0x{start:08X} {symbol} member {parent} {member}")
        else:
            rows.append(f"// RECOVERY_OBJECT: v3 0x{start:08X} 0x{end:08X} {symbol} 0x004632D0 member {parent} {member}")
    (root / "src/mapwin.cpp").write_text("\n".join(rows) + "\n")
    return root, image


@pytest.mark.parametrize("filename,start,size", GAPS)
def test_gap_initializer_uses_pinned_file_backed_bytes_and_refuses_pin_damage(
        tmp_path, filename, start, size):
    hook = _gap_hook()
    raw = _wide_data_image()
    root, original = _synthetic_project(tmp_path, raw)
    output = root / "build/recovery/preparation/generated" / filename
    hook.terrain_landmark_gap_initializer(root, original, output)
    actual = bytes(int(token, 16) for token in
                   re.findall(r"0x([0-9a-fA-F]{2})", output.read_text()))
    pin = sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    assert actual == image.read(__import__("match_recovery.model", fromlist=["Span"]).Span(start, start + size))
    prior = output.read_bytes()
    (root / "src/IMAGE_PIN").write_text("sha256 " + "0" * 64 + "\n")
    with pytest.raises(ValueError):
        hook.terrain_landmark_gap_initializer(root, original, output)
    assert output.read_bytes() == prior


def test_gap_generator_rejects_unbacked_bytes(tmp_path):
    hook = _gap_hook()
    raw = bytearray(_wide_data_image())
    root, original = _synthetic_project(tmp_path, bytes(raw))
    output = root / "build/recovery/preparation/generated/terrain_landmark_gap_mount_mesa.inc"
    # Removing file-backed coverage for the first gap makes the exact-byte
    # request invalid, rather than materializing a virtual zero tail.
    struct.pack_into("<I", raw, 0x178 + 16, 0x100)
    original.write_bytes(raw)
    (root / "src/IMAGE_PIN").write_text(f"sha256 {sha256(raw).hexdigest()}\n")
    with pytest.raises(ValueError):
        hook.terrain_landmark_gap_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.parametrize("damage", ["executable", "relocated"])
def test_gap_generator_rejects_executable_or_relocated_bytes(tmp_path, damage):
    hook = _gap_hook()
    raw = _wide_data_image(
        section_flags=0x60000020 if damage == "executable" else 0xC0000040,
        relocation_site=0x685AFA if damage == "relocated" else None)
    root, original = _synthetic_project(tmp_path, raw)
    output = root / "build/recovery/preparation/generated/terrain_landmark_gap_mount_mesa.inc"
    with pytest.raises(ValueError):
        hook.terrain_landmark_gap_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.skipif(not (os.environ.get("OPENSMACX_IMAGE") or
                         (ROOT / ".opensmacx/game/terranx_original.exe").is_file()),
                    reason="pinned original image is not configured")
def test_optional_pinned_gap_windows_are_file_backed_and_relocation_free(tmp_path):
    original = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    assert image_pin(ROOT) == PIN
    image = PeImage(original.read_bytes(), PIN)
    span_type = __import__("match_recovery.model", fromlist=["Span"]).Span
    for _, start, size in GAPS:
        span = span_type(start, start + size)
        assert image.read_initial(span) == image.read(span)
        assert not image.absolute_sites(span)

    hook = _gap_hook()
    generated = ROOT / "build/recovery/preparation/generated"
    hook.terrain_initializers(ROOT, original, generated)
    hook.point_light_initializer(
        ROOT, original, generated / "terrain_point_light_bit2.inc")
    include_names = [filename for filename, _, _ in GAPS]
    include_names.extend(hook._LANDMARK_TABLES)
    include_names.append("terrain_point_light_bit2.inc")
    includes = {name: (generated / name).read_text() for name in include_names}
    expected = image.read(span_type(0x685A98, 0x685D28))
    fixture = _extract_parent_cpp((ROOT / "src/mapwin.cpp").read_text(), tmp_path,
                                  expected_window=expected)
    source = _write_host_fixture(tmp_path, fixture, includes)
    for compiler_name, compiler in _available_host_compilers():
        compiled, ran = _compile_run_host(tmp_path, compiler, source)
        assert compiled.returncode == 0, (compiler_name, compiled.stderr)
        assert ran is not None and ran.returncode == 0, (
            compiler_name, ran.stdout if ran else "", ran.stderr if ran else "")
