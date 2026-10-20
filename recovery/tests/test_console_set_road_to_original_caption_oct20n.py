"""Actual-source and pinned controls for Console::set_road_to."""
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
SIGNATURE = "void Console::set_road_to(int veh_id, int x_coord, int y_coord, int order)"
MESSAGE_ARRAYS = (
    "RoadToNeedTerraformers",
    "RoadToNotInTriad",
    "RoadToNotInOcean",
    "SetRoadToNotInOcean",
)


def actual_definition() -> str:
    source = console_method_source(SIGNATURE)
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual Console::set_road_to definition")


def message_array_declarations() -> str:
    source = (ROOT / "src/console_commands.cpp").read_text(encoding="utf-8")
    declarations = []
    for name in MESSAGE_ARRAYS:
        match = re.search(rf"static char {name}\[\]\s*=\s*\"[^\"]*\";", source)
        assert match is not None, f"missing actual declaration for {name}"
        declarations.append(match.group(0))
    return "\n".join(declarations) + "\n"


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct Event { std::string name; int a, b, c, d; };
static std::vector<Event> events;
static void record(const char *name, int a = -999, int b = -999,
                   int c = -999, int d = -999) {
    events.push_back(Event{name, a, b, c, d});
}
static bool is(const Event &event, const char *name, int a, int b, int c, int d) {
    return event.name == name && event.a == a && event.b == b &&
           event.c == c && event.d == d;
}

struct Veh {
    int16_t x, y;
    int8_t order;
    int16_t waypoint_x[4], waypoint_y[4];
};
struct Map { uint8_t climate, contour, val2, region; };
static Veh Vehs[16];
static Map MapTiles[1024];
static int MapLatitudeBounds = 64;
static int MapLongitudeBounds = 64;
static int MapLongitude = 32;
static int MapIsFlat = 1;
static Map *map_tiles() { return MapTiles; }

struct Popup { void pop(const char *label, int timeout, int flags, int) {
    last_label = label;
    record("pop", timeout, flags);
    // Keep the actual caption observable rather than reducing it to a boolean.
    captions.push_back(label);
} static const char *last_label; static std::vector<std::string> captions; };
const char *Popup::last_label = nullptr;
std::vector<std::string> Popup::captions;
static Popup NetMsgPrimary;

static int lock_result;
static int retarget_vehicle = -1;
struct NetDaemonDouble {
    int lock_veh(int *vehicle_id, int flags, int timeout, int wait, int extra) {
        record("lock", *vehicle_id, flags, timeout, wait);
        assert(extra == 0);
        if (retarget_vehicle >= 0) *vehicle_id = retarget_vehicle;
        return lock_result;
    }
    void await_synch() { record("await"); }
    void unlock_veh() { record("unlock"); }
} NetDaemonNet;
static void synch_veh(int vehicle_id) { record("synch", vehicle_id); }

class Console { public:
    void set_road_to(int veh_id, int x_coord, int y_coord, int order);
};
'''


def harness_source(definition: str) -> str:
    return HARNESS + message_array_declarations() + definition + r'''
static void reset() {
    events.clear();
    Popup::last_label = nullptr;
    Popup::captions.clear();
    lock_result = 0;
    retarget_vehicle = -1;
    MapIsFlat = 1;
    for (int i = 0; i < 16; ++i) Vehs[i] = Veh{};
    for (int i = 0; i < 1024; ++i) MapTiles[i] = Map{};
    Vehs[3].x = 4; Vehs[3].y = 5; Vehs[3].order = 2;
    Vehs[3].waypoint_x[0] = 101; Vehs[3].waypoint_y[0] = 102;
    Vehs[8].order = 6;
    Vehs[8].waypoint_x[0] = 201; Vehs[8].waypoint_y[0] = 202;
    // origin index 32*5+2=162; target index 32*17+(12>>1)=550.
    MapTiles[162].region = 7;
    MapTiles[550].region = 7;
    MapTiles[550].climate = 0x60;
}
static void expect_one(const char *name, int a = -999, int b = -999,
                       int c = -999, int d = -999) {
    assert(events.size() == 1 && is(events[0], name, a, b, c, d));
}

int main() {
    Console console;
    assert(&RoadToNeedTerraformers[0] != &RoadToNotInTriad[0]);
    assert(&RoadToNotInTriad[0] != &RoadToNotInOcean[0]);
    assert(&RoadToNotInOcean[0] != &SetRoadToNotInOcean[0]);
    assert(std::strcmp(RoadToNeedTerraformers, "NEEDTERRAFORMERS") == 0);
    assert(std::strcmp(RoadToNotInTriad, "NOTINTRIAD") == 0);
    assert(std::strcmp(RoadToNotInOcean, "NOTINOCEAN") == 0);
    assert(std::strcmp(SetRoadToNotInOcean, "NOTINOCEAN") == 0);
    reset();
    console.set_road_to(3, 12, -1, 7);
    assert(events.empty() && Popup::captions.empty()); // bounds: no message or lock

    reset();
    console.set_road_to(3, 5, 5, 7); // Manhattan distance 1 is rejected before reads/calls.
    assert(events.empty());

    reset();
    MapTiles[550].climate = 0x40;
    console.set_road_to(3, 12, 17, 7);
    expect_one("pop", 5000, 0);
    assert(Popup::captions.size() == 1 && Popup::captions[0] == "NOTINOCEAN");
    assert(Popup::last_label == SetRoadToNotInOcean);

    reset();
    MapTiles[550].region = 8;
    console.set_road_to(3, 12, 17, 7);
    assert(events.empty()); // different regions return silently before locking.

    reset();
    lock_result = 1;
    console.set_road_to(3, 12, 17, 7);
    expect_one("lock", 3, 0, -1, -1);
    assert(Vehs[3].order == 2 && Vehs[3].waypoint_x[0] == 101 &&
           Vehs[3].waypoint_y[0] == 102 && Vehs[8].order == 6);

    reset();
    retarget_vehicle = 8;
    console.set_road_to(3, 12, 17, 7);
    assert(events.size() == 4 &&
           is(events[0], "lock", 3, 0, -1, -1) &&
           is(events[1], "synch", 8, -999, -999, -999) &&
           is(events[2], "await", -999, -999, -999, -999) &&
           is(events[3], "unlock", -999, -999, -999, -999));
    assert(Vehs[3].order == 2 && Vehs[3].waypoint_x[0] == 101 &&
           Vehs[3].waypoint_y[0] == 102);
    assert(Vehs[8].order == 7 && Vehs[8].waypoint_x[0] == 12 &&
           Vehs[8].waypoint_y[0] == 17);
}
'''


def run_fixture(compiler: str, tmp_path: Path, definition: str, name: str) -> subprocess.CompletedProcess:
    unit = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    unit.write_text(harness_source(definition), encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=60)
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_set_road_to_preserves_caption_gates_and_lock_retarget(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    definition = actual_definition()
    assert "NetMsgPrimary.pop(SetRoadToNotInOcean, 5000, 0, 0)" in definition
    assert definition.index("MapLatitudeBounds") < definition.index("MapLongitudeBounds")
    assert definition.index("NetMsgPrimary.pop(SetRoadToNotInOcean") < definition.index("lock_veh(&veh_id")
    assert definition.index("Vehs[veh_id].order") < definition.index("synch_veh(veh_id)")
    assert definition.index("synch_veh(veh_id)") < definition.index("await_synch()")
    assert definition.index("await_synch()") < definition.index("unlock_veh()")
    assert run_fixture(compiler, tmp_path, definition, "set_road_to_actual").returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", ("wrong_caption", "wrong_duplicate_identity",
                                     "missing_popup_return", "wrong_region_index"))
def test_set_road_to_actual_mutants_fail(tmp_path, compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = actual_definition()
    if mutant == "wrong_caption":
        damaged = source.replace("NetMsgPrimary.pop(SetRoadToNotInOcean,",
                                 'NetMsgPrimary.pop("NOTHERE",', 1)
    elif mutant == "wrong_duplicate_identity":
        damaged = source.replace("NetMsgPrimary.pop(SetRoadToNotInOcean",
                                 "NetMsgPrimary.pop(RoadToNotInOcean", 1)
    elif mutant == "missing_popup_return":
        damaged = source.replace("NetMsgPrimary.pop(SetRoadToNotInOcean, 5000, 0, 0);\n        return;",
                                 "NetMsgPrimary.pop(SetRoadToNotInOcean, 5000, 0, 0);", 1)
    else:
        damaged = source.replace("map_tiles()[target_index].region",
                                 "map_tiles()[origin_index].region", 1)
    assert damaged != source
    assert run_fixture(compiler, tmp_path, damaged, "set_road_to_" + mutant).returncode != 0


def test_pinned_set_road_to_string_operand_and_effect_order():
    if not IMAGE.is_file():
        pytest.skip("pinned original image unavailable")
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    raw = IMAGE.read_bytes()
    image = PeImage(raw, IMAGE_SHA256)
    lines = Listing(image.read(Span(0x004D8DC0, 0x004D8F37)), 0x004D8DC0).lines
    indexed = {int(line.split()[0], 16): line for line in lines}
    assert "0x688ec8" in indexed[0x004D8E8C]  # target-ocean message operand
    assert indexed[0x004D8E7B].startswith("0x004D8E7B")
    assert "0xffffffe0" in indexed[0x004D8E7B]
    assert "0x60" in indexed[0x004D8E7E]
    assert "0x531020" in indexed[0x004D8EE3]  # lock before target writes
    assert "0x952839" in indexed[0x004D8EFC]  # order byte
    assert "0x95283c" in indexed[0x004D8F06]  # waypoint x
    assert "0x952844" in indexed[0x004D8F0D]  # waypoint y
    assert "0x593220" in indexed[0x004D8F14]  # synch
    assert "0x531480" in indexed[0x004D8F21]  # await synch
    assert "0x5310f0" in indexed[0x004D8F2B]  # unlock
    for start, end, text in (
        (0x00688EC8, 0x00688ED3, b"NOTINOCEAN\0"),
    ):
        assert image.read(Span(start, end)) == text
