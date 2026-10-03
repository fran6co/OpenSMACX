"""Private pytest proposal for the PlanWin + battle-plans body packet.

This proposal intentionally reads only tracked product source. After the
coordinator installs four recovered definitions in src/planwin.cpp and the
typed battle_plans body in src/faction.cpp, it extracts those definitions and
compiles them against narrow typed behavioral fixtures. It has no dependency
on ignored candidate fragments or saved binaries. Missing declarations or
owned data are named below as source-prerequisite witnesses, not fabricated
inside either fixture.
"""

from __future__ import annotations

import shutil
import re
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import read_text as read_source_records
from match_recovery.binding_source import parse as parse_source_bindings


ROOT = Path(__file__).resolve().parents[2]
PLANWIN_CPP = ROOT / "src/planwin.cpp"
PLANWIN_H = ROOT / "src/planwin.h"
MAPWIN_H = ROOT / "src/mapwin.h"
WIN_H = ROOT / "src/win.h"
PENDING_CPP = ROOT / "src/pending_bodies.cpp"
FACTION_H = ROOT / "src/faction.h"
FACTION_CPP = ROOT / "src/faction.cpp"
BASE_H = ROOT / "src/base.h"

# Review map for interfaces and owned storage that this packet consumes.
# These checks deliberately look at the real owners; the behavior fixture
# below supplies no address aliases or synthetic production declarations.
SOURCE_PREREQUISITE_WITNESSES = {
    "PlanWin line storage and real inheritance": (PLANWIN_H, MAPWIN_H, WIN_H),
    "canonical attack_from declaration": (BASE_H,),
    "battle faction state declaration/definition": (FACTION_H, FACTION_CPP),
    "battle labels declaration/definition": (FACTION_H, FACTION_CPP),
    "PlayerData target and region members": (FACTION_H,),
    "Popup, Dialogs, and Scroll lifecycle": (
        ROOT / "src/popup.h", ROOT / "src/popup.cpp",
        ROOT / "src/basepop.h", ROOT / "src/scroll.cpp",
    ),
    "base/map/vehicle storage": (
        BASE_H, ROOT / "src/map.h", ROOT / "src/veh.h",
    ),
}
NATIVE_OBJECT_BINDING_WITNESSES = {
    "PlanBattleFactionA": (0x0093F7BC, None),
    "PlanBattleFactionB": (0x0093F810, None),
    "BattlePlansNoActionLabel": (0x0068DD20, None),
    "BattlePlansMapSharingLabel": (0x0068DD34, None),
    "BattlePlansInvalidPlanLabel": (0x0068DD44, None),
    "BattlePlansLabel": (0x0068DD54, None),
    "BattlePlanLabel": (0x0068DD60, None),
    "BattlePlansRegionRejectedLabel": (0x0068DD6C, None),
    "BattlePlansDistanceRejectedLabel": (0x0068DD78, None),
    "BattlePlansSelectionLabel": (0x0068DD84, None),
}

PROTECTED_ORIGINAL_IDENTITIES = {
    # Seed claims/identity must remain bound when central source ownership
    # retires the seed file; compiling extracted bodies must not rewrite them.
    "PlanWin::draw_line_base": (0x0048BAB0, "no BYTE_EXACT claim"),
    "battle_routine": (0x0054B140, "BYTE_EXACT"),
    "sub_54dc90": (0x0054DC90, "BYTE_EXACT", "C linkage retained"),
    "PlanWin::on_redraw": (0x0048AF30, "original QA identity; emitted UA override measured separately"),
    "battle_plans": (
        0x0054B1C0, "0x0054B1C0-0x0054BEBE", "0x0065F662-0x0065F8FC FILE", "no BYTE_EXACT claim"
    ),
}


def _definition(source: str, start: str) -> str:
    """Return one C++ definition using brace depth, preserving its text."""
    at = source.find(start)
    assert at >= 0, f"actual source definition missing: {start}"
    opening = source.find("{", at)
    assert opening >= 0, f"definition has no body: {start}"
    depth = 0
    quote: str | None = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 1
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and following == "/":
            line_comment = True
            index += 1
        elif char == "/" and following == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[at : index + 1]
        index += 1
    raise AssertionError(f"unterminated C++ definition: {start}")


def _production_definitions() -> tuple[str, str, str, str, str]:
    source = PLANWIN_CPP.read_text(encoding="utf-8")
    known_base = _definition(source, "static bool planwin_base_known_to_faction(")
    known_tile = _definition(source, "static bool planwin_tile_has_known_base(")
    return (
        _definition(source, "void PlanWin::draw_line_base("),
        _definition(source, "void __cdecl battle_routine("),
        _definition(source, 'extern "C" void __cdecl sub_54dc90('),
        known_base,
        known_tile + "\n" + _definition(source, "void PlanWin::on_redraw()"),
    )


def _battle_plans_definitions() -> tuple[str, str, str]:
    source = FACTION_CPP.read_text(encoding="utf-8")
    return (
        _definition(source, "static int battle_plans_selected_region_distance_result("),
        _definition(source, "static void battle_plans_apply_selected_target("),
        _definition(source, "void __cdecl battle_plans("),
    )


FIXTURE = r"""
#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstring>

#ifndef __cdecl
#define __cdecl
#endif

struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former, population;
    uint8_t assimilation, staple, unknown, visibility;
};
struct MapTile { uint8_t climate, contour, owner, region, visibility; };
struct PlayerData { uint32_t flags; };
struct DibHeader { int biWidth, biHeight; };
struct Dib { DibHeader bmiHeader; };
struct DrawnLine { int x1,y1,x2,y2,c1,c2,on,off,phase; };
struct Buffer {
    int copies, lines;
    DrawnLine last;
    Buffer() : copies(0), lines(0) { std::memset(&last, 0, sizeof(last)); }
    void copy(Buffer *, int, int, int, int) { ++copies; }
    void line(int x1,int y1,int x2,int y2,int c1,int c2,int on,int off,int phase) {
        last.x1=x1; last.y1=y1; last.x2=x2; last.y2=y2;
        last.c1=c1; last.c2=c2; last.on=on; last.off=off; last.phase=phase;
        ++lines;
    }
};
static int event_count, update_event;
struct Win { virtual void on_redraw() {} };
struct GraphicWin : Win, Buffer {
    Dib dib_;
    int updates;
    GraphicWin() : dib_(), updates(0) {
        dib_.bmiHeader.biWidth=320; dib_.bmiHeader.biHeight=-200;
    }
    void on_update() { ++updates; update_event = event_count; }
};
struct MapWin : virtual GraphicWin {
    int field_1DD98_, field_1DDBC_, field_1DDC0_;
    MapWin() : GraphicWin(), field_1DD98_(-7), field_1DDBC_(0), field_1DDC0_(0) {}
    void tile_to_pixel(int x,int y,int *px,int *py) {
        *px=x*8+5; *py=y*6+3;
    }
};
struct PlanLineRecord { int32_t x1,y1,x2,y2; };
struct PlanWin : MapWin {
    int field_21A68_, field_21A6C_;
    Buffer buffer_;
    int line_count_, line_colors_[4];
    PlanLineRecord line_records_[4];
    int line_count, clears, centers;
    int center_x, center_y, center_force;
    struct StoredLine { int x1,y1,x2,y2,color; } stored[4];
    PlanWin() : MapWin(), field_21A68_(1), field_21A6C_(1), buffer_(),
        line_count_(0), line_count(0), clears(0), centers(0), center_x(0),
        center_y(0), center_force(0) {
        std::memset(line_colors_, 0, sizeof(line_colors_));
        std::memset(line_records_, 0, sizeof(line_records_));
        std::memset(stored, 0, sizeof(stored));
    }
    void draw_line_base(int,int,int);
    void clear_lines() { line_count=0; ++clears; }
    void add_line(int,int,int);
    void monocenter(int x,int y,int force) {
        center_x=x; center_y=y; center_force=force; ++centers;
    }
    virtual void on_redraw();
};

enum { STATE_OMNISCIENT_VIEW = 0x80, PFLAG_MAP_REVEALED = 0x200 };
static Base Bases[8];
static MapTile MapTiles[64];
static PlayerData PlayersData[8];
static int BaseCurrentCount;
static uint32_t GameState;
static uint32_t MapLongitude = 8;
static int PlanLineCounter;
static int PlanBattleFactionA, PlanBattleFactionB;
static int InfoFactionTextColors[8];
static int base_draw_count, fake_count, vehicle_draw_count;
static int drawn_base_ids[8], drawn_base_flags[8];
static int vehicle_x_history[8], vehicle_y_history[8];
static int __cdecl base_draw(Buffer *,int id,int,int,int,int flags) {
    drawn_base_ids[base_draw_count]=id;
    drawn_base_flags[base_draw_count++]=flags; ++event_count; return 0;
}
static int __cdecl veh_fake(int,int) { ++fake_count; ++event_count; return 2048; }
static int __cdecl veh_draw(Buffer *,int,int x,int y,int,int,int) {
    vehicle_x_history[vehicle_draw_count]=x;
    vehicle_y_history[vehicle_draw_count]=y;
    ++vehicle_draw_count; ++event_count; return 0;
}
struct ConsoleState { int faction_id_; ConsoleState() : faction_id_(1) {} };
static ConsoleState ConsoleGlobal;
static PlanWin PlanWindow;
static void set_base(int id, int x, int y, int faction, int visibility) {
    Bases[id].x=static_cast<int16_t>(x); Bases[id].y=static_cast<int16_t>(y);
    Bases[id].faction_id_current=static_cast<uint8_t>(faction);
    Bases[id].visibility=static_cast<uint8_t>(visibility);
}
static int attack_from(int base_id,int faction_id) {
    return (base_id + faction_id + 1) % 8;
}
static void PlanWin_add_line(int x1,int y1,int x2,int y2,int color) {
    PlanWin::StoredLine &line=PlanWindow.stored[PlanWindow.line_count++];
    line.x1=x1; line.y1=y1; line.x2=x2; line.y2=y2; line.color=color;
}
void PlanWin::add_line(int a,int b,int color) {
    const Base &first=Bases[a], &second=Bases[b];
    PlanWin_add_line(first.x,first.y,second.x,second.y,color);
}
"""


def test_source_has_typed_planwin_override_and_current_definitions() -> None:
    planwin_h = PLANWIN_H.read_text(encoding="utf-8")
    mapwin_h = MAPWIN_H.read_text(encoding="utf-8")
    win_h = WIN_H.read_text(encoding="utf-8")
    pending = PENDING_CPP.read_text(encoding="utf-8")
    faction_h = FACTION_H.read_text(encoding="utf-8")
    faction_cpp = FACTION_CPP.read_text(encoding="utf-8")
    base_h = BASE_H.read_text(encoding="utf-8")
    expected_mappings = {
        0x0048BAB0: ("src/planwin.cpp", "?draw_line_base@PlanWin@@QAEXHHH@Z", False,
                    ((0x0048BAB0, 0x0048BB72),)),
        0x0054B140: ("src/planwin.cpp", "?battle_routine@@YAXH@Z", True,
                    ((0x0054B140, 0x0054B1B6),)),
        0x0054DC90: ("src/planwin.cpp", "sub_54dc90", True,
                    ((0x0054DC90, 0x0054DCE4),)),
        0x0048AF30: ("src/planwin.cpp", "?on_redraw@PlanWin@@QAEXXZ", False,
                    ((0x0048AF30, 0x0048B3B7),)),
        0x0054B1C0: ("src/faction.cpp", "?battle_plans@@YAXHH@Z", False,
                    ((0x0054B1C0, 0x0054BEBE), (0x65F662, 0x65F8FC))),
    }
    parsed = []
    for owner_path in ("src/planwin.cpp", "src/faction.cpp"):
        owner_text = (ROOT / owner_path).read_text(encoding="utf-8")
        rows = read_source_records(owner_text, ROOT / owner_path)
        parsed.extend((owner_path, owner_text, row) for row in rows)
    bindings_by_owner = {
        owner_path: parse_source_bindings(
            owner_text, [row for path, owner_text, row in parsed if path == owner_path])
        for owner_path, owner_text in (("src/planwin.cpp", PLANWIN_CPP.read_text(encoding="utf-8")),
                                       ("src/faction.cpp", FACTION_CPP.read_text(encoding="utf-8")))
    }
    for address, (owner, name, byte_exact, spans) in expected_mappings.items():
        matches = [(path, text, row) for path, text, row in parsed
                   if row.address == address]
        assert len(matches) == 1 and matches[0][0] == owner
        _path, owner_text, record = matches[0]
        assert record.mode.value == "body" and record.name == name
        assert record.byte_exact is byte_exact and record.image_spans == spans
        assert record.line in bindings_by_owner[owner]
    redraw = next(row for _path, _text, row in parsed if row.address == 0x0048AF30)
    assert redraw.symbol == "?on_redraw@PlanWin@@UAEXXZ"
    seeds = ("src/unrecovered/0048bab0.cpp", "src/recovered/units/0054b140.cpp",
             "src/recovered/0054dc90.cpp", "src/recovered/units/0048af30.cpp",
             "src/unrecovered/0054b1c0.cpp")
    for seed in seeds:
        seed_path = ROOT / seed
        assert not seed_path.exists(), f"superseded seed remains: {seed}"

    for witness, paths in SOURCE_PREREQUISITE_WITNESSES.items():
        assert all(path.is_file() for path in paths), witness
    faction_h = FACTION_H.read_text(encoding="utf-8")
    faction_cpp = FACTION_CPP.read_text(encoding="utf-8")

    assert "class PlanWin : public MapWin" in planwin_h
    assert "class MapWin : public virtual GraphicWin" in mapwin_h
    assert "virtual void on_redraw()" in win_h
    assert re.search(r"\b(?:virtual\s+)?void\s+on_redraw\s*\(\s*\)\s*;", planwin_h)
    assert "PENDING_BODY(0x0048AF30" not in pending
    assert "void __cdecl battle_routine(int base_id);" in planwin_h
    assert 'extern "C" void __cdecl sub_54dc90(int base_id);' in planwin_h
    assert "extern int PlanBattleFactionA;" in faction_h
    assert "extern int PlanBattleFactionB;" in faction_h
    assert re.search(r"\bint\s+PlanBattleFactionA\s*;", faction_cpp)
    assert re.search(r"\bint\s+PlanBattleFactionB\s*;", faction_cpp)
    assert re.search(
        r"\bint\s+__cdecl\s+attack_from\s*\(\s*int\s+base_id_to_atk\s*,\s*int\s+faction_id\s*\)\s*;",
        base_h,
    )
    assert re.search(r"\bvoid\s+draw_line_base\s*\(", planwin_h)
    for name in ("PlanBattleFactionA", "PlanBattleFactionB"):
        assert re.search(r"\bextern\s+int\s+" + name + r"\s*;", faction_h)
        assert re.search(r"\bint\s+" + name + r"\s*;", faction_cpp)
    for name, (start, end) in NATIVE_OBJECT_BINDING_WITNESSES.items():
        annotation = re.compile(r"// RECOVERY_OBJECT: (v[12]) "
                                r"(0x[0-9A-Fa-f]+)(?: (0x[0-9A-Fa-f]+) \S+ 0x[0-9A-Fa-f]+)? \S+")
        matches = [line for line in faction_cpp.splitlines()
                   if name in line and "RECOVERY_OBJECT:" in line]
        assert len(matches) == 1, (name, matches)
        match = annotation.fullmatch(matches[0])
        assert match and int(match.group(2), 16) == start, (name, matches[0])
        if end is None:
            assert match.group(1) == "v1" and match.group(3) is None
            if name.startswith("PlanBattleFaction"):
                assert re.search(r"\bint\s+" + name + r"\s*;", faction_cpp)
                assert re.search(r"\bextern\s+int\s+" + name + r"\s*;", faction_h)
            else:
                assert re.search(r"\bchar\s+" + name + r"\[\]", faction_cpp)
                assert re.search(r"\bextern\s+char\s+" + name + r"\[\]", faction_h)
        else:
            assert match.group(1) == "v2" and int(match.group(3), 16) == end
    assert re.search(r"\bvoid\s+__cdecl\s+battle_plans\s*\(", faction_h)
    assert "region_base_plan[128]" in faction_h
    assert "region_total_bases[128]" in faction_h
    assert "int x_target;" in faction_h and "int y_target;" in faction_h
    assert "base_id_atk_target" in faction_h
    popup_h = (ROOT / "src/popup.h").read_text(encoding="utf-8")
    popup_cpp = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    basepop_h = (ROOT / "src/basepop.h").read_text(encoding="utf-8")
    scroll_cpp = (ROOT / "src/scroll.cpp").read_text(encoding="utf-8")
    assert "class Popup : public BasePop" in popup_h
    assert "Scroll scroll_;" in popup_h
    assert "Popup::~Popup()" in popup_cpp
    assert "void Popup::close()" in popup_cpp
    assert "Dialogs" in basepop_h
    assert "Scroll::~Scroll()" in scroll_cpp
    bodies = _production_definitions()
    assert all("PENDING_BODY" not in body for body in bodies)
    assert "PlanWindow.clear_lines()" in bodies[1]
    assert "InfoFactionTextColors[PlanBattleFactionA]" in bodies[1]
    assert 'extern "C" void __cdecl sub_54dc90' in bodies[2]
    assert "PlanWindow.monocenter(base.x, base.y, 0)" in bodies[2]
    assert "surface->on_update()" in bodies[4]
    battle_bodies = _battle_plans_definitions()
    assert len(battle_bodies) == 3
    assert all("PENDING_BODY" not in body for body in battle_bodies)
    assert "Popup popup;" in battle_bodies[2]


def test_extracted_actual_bodies_under_gcc_and_clang(tmp_path: Path) -> None:
    bodies = _production_definitions()
    # Pull only actual declarations with the bodies. Typedef aliases in this
    # narrow fixture intentionally preserve the measured source-facing types.
    source = FIXTURE + "\n" + "\n".join(bodies) + r"""
int main() {
    PlanWin plan;
    plan.field_1DDBC_=-4; plan.field_1DDC0_=11;
    set_base(0,2,3,0,2);
    set_base(1,8,7,2,2);
    set_base(3,6,5,0,0);
    PlanLineCounter=7;
    plan.draw_line_base(0,1,0x5a);
    assert(plan.lines==1 && plan.last.c1==0 && plan.last.c2==0x5a);
    assert(plan.last.x1==17 && plan.last.y1==32);
    assert(plan.last.x2==65 && plan.last.y2==56);
    assert(plan.last.on==4 && plan.last.off==4 && plan.last.phase==7);

    PlanBattleFactionA=1; PlanBattleFactionB=2;
    InfoFactionTextColors[1]=101; InfoFactionTextColors[2]=202;
    set_base(4,4,6,0,0); set_base(5,7,5,0,0); set_base(6,9,2,0,0);
    battle_routine(3);
    assert(PlanWindow.clears==1 && PlanWindow.line_count==2);
    assert(PlanWindow.stored[0].color==101 && PlanWindow.stored[1].color==202);
    assert(static_cast<GraphicWin *>(&PlanWindow)->updates==1);

    set_base(2,9,3,0,0);
    sub_54dc90(2);
    assert(PlanWindow.centers==1 && PlanWindow.center_x==9);
    assert(PlanWindow.center_y==3 && PlanWindow.center_force==0);
    assert(PlanWindow.stored[0].x1==9 && PlanWindow.stored[0].y1==3);

    plan.line_count_=1; plan.line_colors_[0]=0x5a;
    plan.line_records_[0].x1=2; plan.line_records_[0].y1=3;
    plan.line_records_[0].x2=6; plan.line_records_[0].y2=5;
    GraphicWin *surface=static_cast<GraphicWin *>(&plan);
    surface->on_redraw();
    assert(plan.buffer_.copies==1 && surface->updates==1);
    BaseCurrentCount=2;
    MapTiles[3*MapLongitude+1].visibility=2;
    MapTiles[7*MapLongitude+4].visibility=2;
    set_base(0,2,3,0,2); set_base(1,8,7,2,2);
    const int fake_before=fake_count;
    const int vehicles_before=vehicle_draw_count;
    plan.on_redraw();
    assert(plan.buffer_.copies==2 && base_draw_count==2);
    assert(fake_count==fake_before+1 && vehicle_draw_count==vehicles_before+1);
    assert(vehicle_x_history[vehicles_before]==53
           && vehicle_y_history[vehicles_before]==22);
    assert(plan.lines==3);
    assert(plan.last.x1==17 && plan.last.y1==32);
    assert(plan.last.x2==49 && plan.last.y2==44);
    assert(plan.last.c1==0 && plan.last.c2==0x5a);
    assert(plan.last.on==4 && plan.last.off==4 && plan.last.phase==7);
    assert(surface->updates==2 && update_event==event_count);

    // Native v4 iterates endpoint two, then endpoint one. Both endpoints are
    // deliberately absent from known bases so both calls are observable.
    BaseCurrentCount=0;
    plan.line_count_=1;
    plan.line_records_[0].x1=20; plan.line_records_[0].y1=4;
    plan.line_records_[0].x2=24; plan.line_records_[0].y2=5;
    vehicle_draw_count=0; fake_count=0;
    plan.on_redraw();
    assert(vehicle_draw_count==2 && fake_count==2);
    assert(vehicle_x_history[0]==197 && vehicle_y_history[0]==22);
    assert(vehicle_x_history[1]==165 && vehicle_y_history[1]==16);
    std::puts("actual-source PlanWin extraction controls passed");
}
"""

    cpp = tmp_path / "planwin_actual_source.cpp"
    cpp.write_text(source, encoding="utf-8")
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "need at least one configured host C++ compiler"
    for compiler in compilers:
        exe = tmp_path / f"planwin-{compiler.replace('+', 'p')}"
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-pedantic", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], text=True, capture_output=True, check=False)
        assert ran.returncode == 0, ran.stdout + ran.stderr
        assert "actual-source PlanWin extraction controls passed" in ran.stdout

        wrong_endpoint_order = bodies[4].replace(
            "{record.x2, record.y2}, {record.x1, record.y1}",
            "{record.x1, record.y1}, {record.x2, record.y2}",
            1,
        )
        assert wrong_endpoint_order != bodies[4]
        damaged_source = FIXTURE + "\n" + "\n".join(
            bodies[:4] + (wrong_endpoint_order,)
        ) + source[source.rfind("int main()") :]
        damaged_cpp = tmp_path / f"planwin-endpoint-damage-{compiler.replace('+', 'p')}.cpp"
        damaged_cpp.write_text(damaged_source, encoding="utf-8")
        damaged_exe = tmp_path / f"planwin-endpoint-damage-{compiler.replace('+', 'p')}"
        damaged_built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-pedantic", str(damaged_cpp), "-o", str(damaged_exe)],
            text=True, capture_output=True, check=False,
        )
        assert damaged_built.returncode == 0, damaged_built.stdout + damaged_built.stderr
        damaged_run = subprocess.run(
            [str(damaged_exe)], text=True, capture_output=True, check=False
        )
        assert damaged_run.returncode != 0, "endpoint-order damage escaped fixture"


BATTLE_FIXTURE = r"""
#include <assert.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
enum { MaxRegionLandNum = 64, PLAN_NAVAL_TRANSPORT = 7 };
enum { VSTATE_UNK_20000 = 0x00020000u, VSTATE_UNK_40000 = 0x00040000u };
struct Sprite {};
struct Map { uint8_t climate, contour, value, region; };
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former, population_size;
    uint8_t assimilation_turns_left, nerve_staple_turns_left, unknown;
    uint8_t visibility;
    uint8_t intel[8];
    char name_string[25];
};
struct PlayerData {
    uint32_t diplo_treaties[8];
    int x_target, y_target, base_id_atk_target;
    int unk_71, unk_72;
    uint8_t region_total_bases[128];
    uint8_t region_base_plan[128];
};
struct Player { int is_leader_female; char title_leader[32], name_leader[32]; };
struct Veh { uint32_t state; uint8_t faction_id; };
struct Dialogs { void sprite(Sprite *, int, int); void item(char *, int); };
struct PlanWindowStub {
    int init_count, close_count, redraw_count, item_count, show_count;
    int view[8], add_count, lines_now, calls[8][3];
    int clear_count;
    PlanWindowStub() : init_count(0), close_count(0), redraw_count(0),
        item_count(0), show_count(0), add_count(0), lines_now(0), clear_count(0) {
        int i; for (i = 0; i < 8; ++i) view[i] = -1;
    }
    void init(char *) { ++init_count; }
    void find_view(int a, int b, int c, int d, int e, int f, int g, int h) {
        view[0]=a; view[1]=b; view[2]=c; view[3]=d;
        view[4]=e; view[5]=f; view[6]=g; view[7]=h;
    }
    void back_redraw() {}
    void add_line(int a, int b, int c) {
        assert(add_count < 8);
        calls[add_count][0]=a; calls[add_count][1]=b; calls[add_count][2]=c;
        ++add_count; ++lines_now;
    }
    void add_line(int, int, int, int, int) { ++lines_now; }
    void clear_lines() { lines_now=0; ++clear_count; }
    void show(int) { ++show_count; }
    void on_redraw() { ++redraw_count; }
    void close() { ++close_count; }
    void monocenter(int, int, int) {}
};
struct NetDaemonStub { int awaited; NetDaemonStub() : awaited(0) {}
    void await_diplo(int value) { awaited=value; } };
struct Popup {
    Dialogs dialogs_;
    Popup();
    ~Popup();
    void start(char *, const char *, int, char *, int, void *);
    int exec(int, int (*)());
    Dialogs &dialogs() { return dialogs_; }
};

static Map Tiles[256];
// Padding makes the intentionally damaged guard safe to execute in this
// fixture; the genuine source guard returns before accessing an invalid plan.
static Base BaseStorage[33];
static Base *Bases = BaseStorage + 1;
static PlayerData PlayersData[8];
static Player Players[8];
static Sprite Portraits[8];
static Sprite *FactionDiplomacyPortraits[8];
static int InfoFactionTextColors[8];
static int VehCurrentCount;
static Veh Vehs[8];
static int BaseCurrentCount;
static int Longitude;
static int BaseFindDist, SelectedDistance, PlannedDistance;
static int DiploScratchFA2C, DiploScratchFA48, DiploScratchFA60, DiploFriction;
static int PopupDialogFactionID, IsMultiplayerNet;
static int PlanBattleFactionA, PlanBattleFactionB;
static int GenderDefault, PluralityDefault;
static char PopupStartCaption[256];
static char DiploLabelSendingTRADEMAPS[] = "Sending TRADE MAPS";
static char BattlePlansNoActionLabel[] = "NEVERBATTLEPLANS";
static char BattlePlansMapSharingLabel[] = "MAPBATTLEPLANS";
static char BattlePlansInvalidPlanLabel[] = "NOBATTLEPLANS";
static char BattlePlansLabel[] = "BATTLEPLANS";
static char BattlePlanLabel[] = "BATTLEPLAN";
static char BattlePlansRegionRejectedLabel[] = "BATTLENO";
static char BattlePlansDistanceRejectedLabel[] = "BATTLENO";
static char BattlePlansSelectionLabel[] = "BATTLEYES";
static PlanWindowStub PlanWindow;
static NetDaemonStub NetDaemonNet;
static int PlanResults[2], PopResults[4], PopResultCount, PopResultIndex;
static int PopupSelection, PopupConstructed, PopupDestroyed;
static int TradeMapsCalls, MessageCalls, LogCalls, ParseCalls, SpotCalls;
static const char *ParsedInputs[8];
static int AttackCalls, AttackTrace[16];
static const char *LastPopupLabel;
static int LastPopupValue;
static int ThrowFromSuggest;
static int ForcedInvalidPlanAttack;

static const Map *map_loc(int x, int y) { return &Tiles[(x >> 1) + y * Longitude]; }
static int base_find(int x, int y, int) {
    BaseFindDist = (x == Bases[3].x && y == Bases[3].y)
                       ? SelectedDistance : PlannedDistance;
    return -1;
}
static int suggest_plan(int faction, int) {
    if (ThrowFromSuggest) throw 17;
    return PlanResults[faction == 1 ? 0 : 1];
}
static int attack_from(int base_id, int faction) {
    ++AttackCalls; AttackTrace[AttackCalls - 1] = base_id * 10 + faction;
    if (ForcedInvalidPlanAttack && base_id == -1 && faction == 2) return 8;
    if (base_id == 5) return 9;
    if (base_id == 6) return 10;
    if (base_id == 3) return faction == 1 ? 11 : 12;
    return -1;
}
static int pop_wait() { return 0; }
static int x_pops_short(const char *label, int value, Sprite *, int (*)()) {
    LastPopupLabel=label; LastPopupValue=value;
    return PopResultIndex < PopResultCount ? PopResults[PopResultIndex++] : 0;
}
static void trade_maps(int, int) { ++TradeMapsCalls; }
static void log_say(const char *, int, int, int) { ++LogCalls; }
static int message_data(int, int, int, int, int, int) { ++MessageCalls; return 0; }
static int parse_says(int, const char *input, int, int) {
    ParsedInputs[ParseCalls++]=input; return 0;
}
static void spot_loc(int, int, int) { ++SpotCalls; }
void Dialogs::sprite(Sprite *, int, int) {}
void Dialogs::item(char *, int) { ++PlanWindow.item_count; }
Popup::Popup() { ++PopupConstructed; }
Popup::~Popup() { ++PopupDestroyed; }
void Popup::start(char *, const char *label, int, char *, int, void *) {
    LastPopupLabel=label;
}
int Popup::exec(int, int (*)()) { return PopupSelection; }
"""


BATTLE_MAIN = r"""
static void set_base(int id, int x, int y, int owner, int visibility,
                     const char *name) {
    Bases[id].x=(int16_t)x; Bases[id].y=(int16_t)y;
    Bases[id].faction_id_current=(uint8_t)owner;
    Bases[id].visibility=(uint8_t)visibility;
    strcpy(Bases[id].name_string, name);
}
static void reset_fixture() {
    int i, j;
    memset(Tiles,0,sizeof(Tiles)); memset(BaseStorage,0,sizeof(BaseStorage));
    memset(PlayersData,0,sizeof(PlayersData)); memset(Players,0,sizeof(Players));
    memset(Vehs,0,sizeof(Vehs));
    for(i=0;i<8;++i) {
        FactionDiplomacyPortraits[i]=&Portraits[i];
        InfoFactionTextColors[i]=100+i;
        for(j=0;j<128;++j) PlayersData[i].region_base_plan[j]=0;
    }
    Longitude=10; BaseCurrentCount=11; VehCurrentCount=0; BaseFindDist=0;
    SelectedDistance=3; PlannedDistance=6;
    DiploScratchFA2C=0; DiploScratchFA48=1; DiploScratchFA60=0; DiploFriction=0;
    PopupDialogFactionID=-1; IsMultiplayerNet=0; PlanBattleFactionA=-1;
    PlanBattleFactionB=-1; GenderDefault=-1; PluralityDefault=-1;
    PlanWindow=PlanWindowStub(); NetDaemonNet.awaited=0;
    PlanResults[0]=5; PlanResults[1]=6; PopResults[0]=1;
    PopResults[1]=0; PopResults[2]=0; PopResults[3]=0;
    PopResultCount=1; PopResultIndex=0; PopupSelection=3;
    PopupConstructed=0; PopupDestroyed=0; ThrowFromSuggest=0;
    ForcedInvalidPlanAttack=0;
    TradeMapsCalls=MessageCalls=LogCalls=ParseCalls=SpotCalls=AttackCalls=0;
    for(i=0;i<8;++i) ParsedInputs[i]=0;
    for(i=0;i<16;++i) AttackTrace[i]=-1;
    LastPopupLabel=0; LastPopupValue=0;
    set_base(3,2,1,3,4,"candidate");
    set_base(5,0,0,0,0,"plan-a"); set_base(6,4,2,0,0,"plan-b");
    set_base(9,2,3,0,0,"attack-a"); set_base(10,8,4,0,0,"attack-b");
    set_base(11,10,5,0,0,"candidate-attack-a");
    set_base(12,12,6,0,0,"candidate-attack-b");
    Tiles[(2>>1)+1*Longitude].region=5;
    Tiles[(2>>1)+1*Longitude].climate=2;
    Tiles[0].climate=1;
    PlayersData[2].region_total_bases[5]=1;
    PlayersData[2].x_target=0; PlayersData[2].y_target=0;
    PlayersData[2].unk_71=17; PlayersData[2].unk_72=19;
    PlayersData[1].diplo_treaties[3]=0x10;
    PlayersData[3].diplo_treaties[2]=0x10;
}
static void accepted_plan_checks() {
    reset_fixture(); PopResults[0]=1; PopResultCount=1;
    battle_plans(1,2);
    assert(PopupConstructed==1 && PopupDestroyed==1);
    assert(PlanWindow.view[0]==0 && PlanWindow.view[1]==0);
    assert(PlanWindow.view[2]==4 && PlanWindow.view[3]==2);
    assert(PlanWindow.view[4]==2 && PlanWindow.view[5]==3);
    assert(PlanWindow.view[6]==8 && PlanWindow.view[7]==4);
    assert(PlanWindow.add_count==4);
    assert(PlanWindow.calls[0][0]==9 && PlanWindow.calls[0][1]==5);
    assert(PlanWindow.calls[0][2]==101);
    assert(PlanWindow.calls[1][0]==10 && PlanWindow.calls[1][1]==6);
    assert(PlanWindow.calls[1][2]==102);
    assert(PlanWindow.calls[2][0]==11 && PlanWindow.calls[2][1]==3);
    assert(PlanWindow.calls[2][2]==101);
    assert(PlanWindow.calls[3][0]==12 && PlanWindow.calls[3][1]==3);
    assert(PlanWindow.calls[3][2]==102);
    assert(ParseCalls==5);
    assert(strcmp(ParsedInputs[0],Bases[6].name_string)==0);
    assert(strcmp(ParsedInputs[1],Bases[5].name_string)==0);
    assert(strcmp(ParsedInputs[4],Bases[3].name_string)==0);
    assert(PlanWindow.redraw_count==1 && PlanWindow.item_count==1);
    assert(PlayersData[2].base_id_atk_target==3);
    assert(PlayersData[2].x_target==-1 && PlayersData[2].y_target==-1);
    assert(PlayersData[2].unk_71==0 && PlayersData[2].unk_72==0);
    assert(strcmp(LastPopupLabel,"BATTLEYES")==0);
}
int main() {
    accepted_plan_checks();

    reset_fixture(); PlayersData[1].diplo_treaties[2]=0;
    PlayersData[2].diplo_treaties[1]=0x20; battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"NEVERBATTLEPLANS")==0);
    assert(PopupDestroyed==1 && PlanWindow.init_count==0);

    reset_fixture(); DiploScratchFA48=0; PopResults[0]=0;
    battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"MAPBATTLEPLANS")==0 && PopupDestroyed==1);

    reset_fixture(); IsMultiplayerNet=1; battle_plans(1,2);
    assert(LogCalls==1 && MessageCalls==1 && NetDaemonNet.awaited==0x448);

    reset_fixture(); PlanResults[0]=-1; battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"NOBATTLEPLANS")==0 && PopupDestroyed==1);
    reset_fixture(); PlanResults[1]=-1; ForcedInvalidPlanAttack=1;
    battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"NOBATTLEPLANS")==0 && PopupDestroyed==1);

    reset_fixture(); PlayersData[2].region_total_bases[5]=0;
    PlayersData[2].region_total_bases[7]=1;
    PlayersData[2].region_base_plan[7]=PLAN_NAVAL_TRANSPORT;
    battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"BATTLENO")==0);
    reset_fixture(); PlannedDistance=5; battle_plans(1,2);
    assert(strcmp(LastPopupLabel,"BATTLENO")==0);

    reset_fixture(); ThrowFromSuggest=1;
    try { battle_plans(1,2); assert(0); }
    catch(int value) { assert(value==17 && PopupDestroyed==1); }
    return 0;
}
"""


def _battle_program(bodies: tuple[str, str, str]) -> str:
    return BATTLE_FIXTURE + "\n" + "\n".join(bodies) + "\n" + BATTLE_MAIN


def test_extracted_battle_plans_actual_source_and_native_order_damage_cases(
    tmp_path: Path,
) -> None:
    bodies = _battle_plans_definitions()
    assert len(bodies) == 3
    assert "Popup popup;" in bodies[2]
    assert "PlanWindow.find_view(" in bodies[2]
    assert "parse_says(0, Bases[plan_b].name_string" in bodies[2]
    assert "if (plan_a < 0 || plan_b < 0)" in bodies[2]
    assert "if (plan_a < 0 || attack_b < 0)" not in bodies[2]

    wrong_view = bodies[2].replace(
        "Bases[plan_a].x, Bases[plan_a].y,\n                         Bases[plan_b].x, Bases[plan_b].y,",
        "Bases[plan_b].x, Bases[plan_b].y,\n                         Bases[plan_a].x, Bases[plan_a].y,",
        1,
    )
    wrong_line = bodies[2].replace(
        "PlanWindow.add_line(attack_a, plan_a,",
        "PlanWindow.add_line(plan_a, attack_a,",
        1,
    )
    wrong_parse = bodies[2].replace(
        "parse_says(0, Bases[plan_b].name_string",
        "parse_says(0, Bases[plan_a].name_string",
        1,
    )
    wrong_plan_guard = bodies[2].replace(
        "if (plan_a < 0 || plan_b < 0)",
        "if (plan_a < 0 || attack_b < 0)",
        1,
    )
    assert wrong_view != bodies[2] and wrong_line != bodies[2]
    assert wrong_parse != bodies[2]
    assert wrong_plan_guard != bodies[2]

    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "need at least one configured host C++ compiler"
    for compiler in compilers:
        def compile_run(program: str, suffix: str) -> subprocess.CompletedProcess[str]:
            cpp = tmp_path / ("battle-" + suffix + ".cpp")
            cpp.write_text(program, encoding="utf-8")
            exe = tmp_path / ("battle-" + suffix + "-" + compiler.replace("+", "p"))
            built = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                 "-pedantic", str(cpp), "-o", str(exe)],
                text=True, capture_output=True, check=False,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            return subprocess.run([str(exe)], text=True, capture_output=True,
                                  check=False)

        good = compile_run(_battle_program(bodies), "actual")
        assert good.returncode == 0, good.stdout + good.stderr
        for label, damaged in (
            ("view-order", (bodies[0], bodies[1], wrong_view)),
            ("line-direction", (bodies[0], bodies[1], wrong_line)),
            ("parse-order", (bodies[0], bodies[1], wrong_parse)),
            ("plan-result-guard", (bodies[0], bodies[1], wrong_plan_guard)),
        ):
            bad = compile_run(_battle_program(damaged), label)
            assert bad.returncode != 0, "damage case escaped fixture: " + label
