"""Execute complete free dispatchers with service doubles, not native GUI proof."""
from dataclasses import fields
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "4d8865ed757e17348bf60873e1591e3d08cfa27b"
SUBJECTS = {0x46B190, 0x46AEF0}


def test_complete_unit_identity_and_outside_source_continuity():
    path = ROOT / "src/mapwin.cpp"
    current = path.read_text()
    old = subprocess.check_output(["git", "show", BASE + ":src/mapwin.cpp"], cwd=ROOT).decode()
    rows = read_text(current, path)
    previous = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(previous)
    assert write(current, rows) == current
    restored = current
    for signature in ("void __cdecl draw_map(int draw_type)",
                      "void __cdecl draw_radius(int x_coord, int y_coord, int a3, int draw_type)"):
        restored = restored.replace(extract_definition(current, signature),
                                    extract_definition(old, signature), 1)
    assert restored == old
    for row in rows:
        baseline = previous[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(baseline, field.name)
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(baseline)


def control(kind):
    header = (ROOT / "src/mapwin.h").read_text()
    array = re.search(r"(?m)^typedef MapWin \*MapWinSlots\[8\];", header)[0]
    bound = re.search(r"(?m)^static const size_t MapWinTableSlots = 8;", header)[0]
    activity = re.search(r"(?m)^  uint32_t field_1DD74_;", header)[0]
    signature = ("void __cdecl draw_map(int draw_type)" if kind == "map" else
                 "void __cdecl draw_radius(int x_coord, int y_coord, int a3, int draw_type)")
    body = extract_definition((ROOT / "src/mapwin.cpp").read_text(), signature)
    assert "reinterpret_cast" not in body
    assert "static_cast<const volatile MapWin *>(window)->field_1DD74_" in body
    call = "draw_map(expected_type);" if kind == "map" else "draw_radius(-17, 23, -4, expected_type);"
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#define __cdecl
struct MapWin {
    int id;
ACTIVITY
    void draw_map(int);
    void draw_radius(int,int,int,int);
};
ARRAY
BOUND
MapWinSlots MapWinTable;
static MapWin windows[8],replacement{8,1};
static std::vector<int> calls;
static bool mutate;
static int expected_type;
static void callback(MapWin *receiver) {
    calls.push_back(receiver->id);
    if(mutate && receiver->id==0){
        MapWinTable[0]=&replacement;
        MapWinTable[3]=&replacement;
        windows[4].field_1DD74_=0;
        windows[5].field_1DD74_=0xFFFFFFFFu;
        MapWinTable[6]=nullptr;
    }
}
void MapWin::draw_map(int n){assert(n==expected_type);callback(this);}
void MapWin::draw_radius(int x,int y,int r,int n){
    assert(x==-17 && y==23 && r==-4 && n==expected_type);callback(this);
}
BODY
int main(){
    for(int mode=0;mode!=4;++mode){
        for(int type: {-2147483647-1,0,1,2147483647}){
            calls.clear();mutate=mode==0;expected_type=type;
            for(int i=0;i<8;++i){windows[i]={i,mode==2?0u:1u};MapWinTable[i]=&windows[i];}
            if(mode==0){windows[0].field_1DD74_=0;MapWinTable[1]=nullptr;
                windows[2].field_1DD74_=0;windows[5].field_1DD74_=0;}
            if(mode==1){MapWinTable[0]=nullptr;windows[2].field_1DD74_=0x80000000u;}
            if(mode==3)for(auto &slot:MapWinTable)slot=nullptr;
            CALL
            if(mode==0)assert((calls==std::vector<int>{0,8,5,7}));
            if(mode==1)assert((calls==std::vector<int>{1,2,3,4,5,6,7}));
            if(mode==2)assert((calls==std::vector<int>{0}));
            if(mode==3)assert(calls.empty());
        }
    }
}
'''.replace("ARRAY", array).replace("BOUND", bound).replace("ACTIVITY", activity).replace("BODY", body).replace("CALL", call)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("kind", ("map", "radius"))
def test_actual_dispatchers_with_mutating_callbacks_and_damage(tmp_path, compiler, kind):
    program = control(kind)
    run_control(tmp_path, compiler, program, None)
    for damage in (
        ("slot == 0 ||", "false ||"),
        ("MapWin *const window = MapWinTable[slot];", "MapWin *const window = MapWinTable[0];"),
        ("++slot)", "slot += 2)"),
        ("field_1DD74_ != 0", "field_1DD74_ == 1"),
    ):
        run_control(tmp_path, compiler, program, damage)
    damage = (("window->draw_map(draw_type);", "window->draw_map(2);") if kind == "map" else
              ("window->draw_radius(x_coord, y_coord, a3, draw_type);",
               "window->draw_radius(y_coord, x_coord, a3, draw_type);"))
    run_control(tmp_path, compiler, program, damage)
