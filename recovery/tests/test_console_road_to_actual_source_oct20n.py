"""Actual-source controls for Console::road_to's typed prototype lookup."""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from console_sources import console_method_source


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "void Console::road_to(int veh_id, int cursor_mode)"


def actual_definition() -> str:
    source = console_method_source(SIGNATURE)
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual Console::road_to definition")


def header_declaration(path: Path, start_text: str) -> str:
    source = path.read_text(encoding="utf-8")
    start = source.index(start_text)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            finish = end + 2 if end + 1 < len(source) and source[end + 1] == ";" else end + 1
            return source[start:finish]
    raise AssertionError(f"unterminated declaration {start_text!r}")


def terraforming_declarations() -> str:
    header = ROOT / "src/terraforming.h"
    enum = header_declaration(header, "enum TerraformingAction")
    rules = header_declaration(header, "struct RulesTerraforming")
    max_count = re.search(r"static const int MaxTerrainNum\s*=\s*20\s*;", header.read_text(encoding="utf-8"))
    assert max_count is not None
    return enum + "\n" + rules + "\n" + max_count.group(0) + "\n"


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

struct Event { std::string name; int a, b, c; };
static std::vector<Event> events;
static void record(const char *name, int a = -999, int b = -999, int c = -999) {
    events.push_back(Event{name, a, b, c});
}
static bool is(const Event &event, const char *name, int a, int b, int c) {
    return event.name == name && event.a == a && event.b == b && event.c == c;
}

struct Veh { int16_t x, y, proto_id; uint8_t faction_id; };
struct VehPrototype { uint8_t chassis_id, plan; };
struct RulesChassis { int triad; };
struct Map { uint8_t climate; };
using LPSTR = char *;
static Veh Vehs[16];
static VehPrototype VehPrototypes[16];
static RulesChassis Chassis[16];
static Map MapTiles[64];
static int MapLongitude = 8;
enum { TRIAD_LAND = 0, TRIAD_SEA = 1 };
static Map *map_tiles() { return MapTiles; }

struct Popup { void pop(const char *label, int timeout, int flags, int) {
    record(label, timeout, flags);
} };
static Popup NetMsgPrimary;
static int tech_result;
static int has_tech_calls;
static int has_tech(int tech, int faction) {
    ++has_tech_calls;
    record("has_tech", tech, faction);
    return tech_result;
}
struct ConsoleState {
    void cursor_on(int mode, int range) { record("cursor_on", mode, range); }
};
static ConsoleState ConsoleGlobal;

class Console { public: void road_to(int veh_id, int cursor_mode); };
'''


def harness_source(definition: str) -> str:
    return HARNESS + terraforming_declarations() + r'''
static RulesTerraforming Terraforming[MaxTerrainNum];
''' + definition + r'''
static void reset() {
    events.clear();
    has_tech_calls = 0;
    tech_result = 0;
    for (int i = 0; i < 16; ++i) {
        Vehs[i] = Veh{};
        VehPrototypes[i] = VehPrototype{};
        Chassis[i] = RulesChassis{};
    }
    for (int i = 0; i < 64; ++i) MapTiles[i] = Map{};
    Vehs[2] = Veh{6, 2, 7, 4};
    // Deliberately disagree with slot 2: this owner must follow proto_id 7.
    VehPrototypes[2] = VehPrototype{2, 9};
    VehPrototypes[7] = VehPrototype{5, 8};
    Chassis[2].triad = TRIAD_LAND;
    Chassis[5].triad = TRIAD_SEA;
    Terraforming[TERRA_FARM].preq_tech = 19;
    Terraforming[TERRA_MAGTUBE].preq_tech = 37;
}
static void expect_one(const char *name, int a, int b, int c) {
    assert(events.size() == 1 && is(events[0], name, a, b, c));
}

int main() {
    Console console;
    reset();
    console.road_to(2, 0x1b);
    expect_one("NEEDTERRAFORMERS", 5000, 0, -999);
    assert(has_tech_calls == 0);

    reset();
    VehPrototypes[7].plan = 9;
    console.road_to(2, 0x1b);
    expect_one("NOTINTRIAD", 5000, 0, -999);
    assert(has_tech_calls == 0);

    reset();
    VehPrototypes[7].plan = 9;
    Chassis[5].triad = TRIAD_LAND;
    // y*8 + (x>>1) is 19; neighbors remain invalid ocean sentinels.
    MapTiles[19].climate = 0x40;
    console.road_to(2, 0x1b);
    expect_one("NOTINOCEAN", 5000, 0, -999);
    assert(has_tech_calls == 0);

    reset();
    VehPrototypes[7].plan = 9;
    Chassis[5].triad = TRIAD_LAND;
    MapTiles[19].climate = 0x60;
    console.road_to(2, 0x1b);
    expect_one("cursor_on", 2, -1, -999);
    assert(has_tech_calls == 0);

    reset();
    VehPrototypes[7].plan = 9;
    Chassis[5].triad = TRIAD_LAND;
    MapTiles[19].climate = 0x60;
    console.road_to(2, 0x1c);
    assert(events.size() == 1 && is(events[0], "has_tech", 37, 4, -999));
    assert(has_tech_calls == 1); // tech unavailable: no cursor notification

    reset();
    VehPrototypes[7].plan = 9;
    Chassis[5].triad = TRIAD_LAND;
    MapTiles[19].climate = 0x60;
    tech_result = 1;
    console.road_to(2, 0x1c);
    assert(events.size() == 2 && is(events[0], "has_tech", 37, 4, -999) &&
           is(events[1], "cursor_on", 3, -1, -999));
    assert(has_tech_calls == 1);
}
'''


def run_fixture(compiler: str, tmp_path: Path, definition: str, label: str) -> subprocess.CompletedProcess:
    unit = tmp_path / (label + ".cpp")
    executable = tmp_path / label
    unit.write_text(harness_source(definition), encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=60)
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_road_to_preserves_typed_lookup_and_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    definition = actual_definition()
    assert definition.count("Vehs[veh_id].proto_id") == 1
    assert definition.index("Vehs[veh_id].proto_id") < definition.index(".plan")
    assert definition.index(".plan") < definition.index(".chassis_id")
    assert definition.index(".chassis_id") < definition.index("map_tiles()")
    assert definition.index("map_tiles()") < definition.index("has_tech(")
    assert "VehPrototypes[prototype_id].plan" in definition
    assert "VehPrototypes[prototype_id].chassis_id" in definition
    assert run_fixture(compiler, tmp_path, definition, "road_to_actual").returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", ("wrong_prototype", "wrong_chassis", "wrong_terraforming_row", "tech_before_cursor_gate",
                                     "missing_plan_return"))
def test_road_to_actual_body_controls_reject_wrong_owner_or_order(tmp_path, compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = actual_definition()
    if mutant == "wrong_prototype":
        damaged = source.replace("VehPrototypes[prototype_id].plan",
                                 "VehPrototypes[veh_id].plan", 1)
    elif mutant == "wrong_chassis":
        damaged = source.replace("VehPrototypes[prototype_id].chassis_id",
                                 "VehPrototypes[veh_id].chassis_id", 1)
    elif mutant == "wrong_terraforming_row":
        damaged = source.replace("Terraforming[TERRA_MAGTUBE].preq_tech",
                                 "Terraforming[TERRA_FARM].preq_tech", 1)
    elif mutant == "tech_before_cursor_gate":
        damaged = source.replace("cursor_mode != 0x1C ||\n        has_tech(",
                                 "has_tech(", 1).replace(
            "vehicle.faction_id)) {", "vehicle.faction_id) || cursor_mode != 0x1C) {", 1)
    else:
        damaged = source.replace("        return;\n    }\n    if (Chassis[",
                                 "    }\n    if (Chassis[", 1)
    assert damaged != source
    assert run_fixture(compiler, tmp_path, damaged, "road_to_" + mutant).returncode != 0


def test_pinned_road_to_span_supports_typed_access_and_call_order():
    if not IMAGE.is_file():
        pytest.skip("pinned original image unavailable")
    from match_recovery.model import Span
    from match_recovery.object_source import parse_objects
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    raw = IMAGE.read_bytes()
    image = PeImage(raw, IMAGE_SHA256)
    lines = Listing(image.read(Span(0x004D7BE0, 0x004D7CED)), 0x004D7BE0).lines
    indexed = {int(line.split()[0], 16): line for line in lines}
    expected = {
        0x004D7BF0: "0x952832",  # Veh::proto_id, after the 0x34 stride calculation
        0x004D7C00: "0x9ab892",  # VehPrototype::plan
        0x004D7C28: "0x9ab88c",  # VehPrototype::chassis_id, only after plan succeeds
        0x004D7C36: "0x94a379",  # RulesChassis::triad
        0x004D7C5D: "0x95282a",  # live Veh y, then x for the map index
        0x004D7C74: "0x94a30c",  # MapTiles load before the climate gate
        0x004D7CAF: "0x1c",      # cursor mode gate precedes technology lookup
        0x004D7CBC: "0x691940",  # Terraforming[6].preq_tech
        0x004D7CC4: "0x5b9f20", # has_tech call
        0x004D7CE3: "0x510490", # cursor_on call
    }
    for address, operand in expected.items():
        assert address in indexed and operand in indexed[address]
    assert list(expected) == sorted(expected)
    header = (ROOT / "src/terraforming.h").read_text(encoding="utf-8")
    alpha = (ROOT / "src/alpha.cpp").read_text(encoding="utf-8")
    enum = terraforming_declarations()
    assert "TERRA_MAGTUBE = 6" in enum
    assert "RulesTerraforming Terraforming[MaxTerrainNum]" in alpha
    binding, = [row for row in parse_objects(
        alpha, "src/alpha.cpp", IMAGE_SHA256, address_space="pe:va"
    ) if row.symbol == "?Terraforming@@3PAURulesTerraforming@@A"]
    assert binding.span is not None
    array_base, array_end = binding.span.start, binding.span.end
    record_stride = (array_end - array_base) // 20
    assert array_base == 0x691878
    assert record_stride == 0x20
    # Original PE32 LPSTR fields are four bytes; the two preceding fields put
    # preq_tech at +8, as verified against the actual source declaration.
    assert re.search(r"struct RulesTerraforming\s*\{\s*LPSTR name;\s*LPSTR name_sea;\s*int preq_tech;", header)
    assert array_base + 6 * record_stride + 8 == 0x691940
