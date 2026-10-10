"""Actual move-end body against pinned byte positions, not a game/runtime proof."""

from pathlib import Path

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract


ROOT = Path(__file__).resolve().parents[2]


def control():
    body = extract((ROOT / "src/veh.cpp").read_text(), "void __cdecl veh_move_end(")
    record = extract((ROOT / "src/veh.h").read_text(), "struct Veh {") + ";"
    assert "reinterpret_cast" not in body
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#define __cdecl
RECORD
static_assert(sizeof(Veh)==0x34,"pinned stride");
static_assert(offsetof(Veh,state)==4,"state dword");
static_assert(offsetof(Veh,order)==0x11,"order byte");
static_assert(offsetof(Veh,waypoint_count)==0x12,"waypoint count byte");
static_assert(offsetof(Veh,patrol_current_point)==0x13,"patrol iterator byte");
static Veh Vehs[2049];
static unsigned char expected[sizeof Vehs];
BODY
// The oracle uses independently decoded original byte positions. It does not
// use the production member names or its clear masks; unrelated bytes and
// adjacent vehicles are part of each whole-array comparison.
static void oracle(unsigned char *row) {
    uint32_t state;
    std::memcpy(&state,row+4,4);
    const bool patrol=(state&(1u<<25)) && (state&(1u<<9));
    if(patrol && row[0x13]<row[0x12]) {
        state &= ~(1u<<25);
        state &= ~(1u<<9);
    }
    const bool second=(state&(1u<<24)) && (state&(1u<<9));
    if(second) {
        state &= ~(1u<<24);
        state &= ~(1u<<9);
    }
    state &= ~(1u<<14);
    std::memcpy(row+4,&state,4);
    if(row[0x11]==24)row[0x11]=0;
}
int main() {
    const uint32_t states[]={0,2,0x200,0x1000000,0x2000000,0x1000200,
        0x2000200,0x3000200,0x4000,0x3004200,0xffffffff,0xfafffdff};
    const unsigned values[]={0,1,23,24,25,127,128,254,255};
    for(int index: {0,1,2048})for(uint32_t state:states)
    for(unsigned count:values)for(unsigned patrol:values)for(unsigned order:values) {
        std::memset(Vehs,0xa5,sizeof Vehs);
        Veh &vehicle=Vehs[index];
        vehicle.state=state;
        vehicle.waypoint_count=(uint8_t)count;
        vehicle.patrol_current_point=(uint8_t)patrol;
        vehicle.order=(int8_t)order;
        // Deliberately different from the actual counters/order: the old
        // state-relative translation used these three neighboring fields.
        vehicle.unk_1=0x1807;
        vehicle.faction_id=255;
        vehicle.year_end_lurking=0;
        std::memcpy(expected,Vehs,sizeof Vehs);
        oracle(expected+index*0x34);
        veh_move_end(index);
        assert(std::memcmp(expected,Vehs,sizeof Vehs)==0);
    }
}
'''.replace("RECORD", record).replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("vehicle.patrol_current_point < vehicle.waypoint_count",
     "vehicle.year_end_lurking < vehicle.faction_id"),
    ("vehicle.patrol_current_point < vehicle.waypoint_count",
     "int8_t(vehicle.patrol_current_point) < int8_t(vehicle.waypoint_count)"),
    ("vehicle.order == 0x18", "(vehicle.unk_1 >> 8) == 0x18"),
    ("vehicle.order = 0;", "vehicle.faction_id = 0;"),
    ("vehicle.state &= 0xffffbfff;", "vehicle.state &= 0xffffffff;")))
def test_actual_move_end_members(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)
