"""Actual satellite editor and deletion log objects; bounded host controls only."""

from pathlib import Path
import re

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def satellite_control():
    source = (ROOT / "src/reportwin.cpp").read_text()
    body = extract(source, "void __cdecl sat_edit(")
    header = (ROOT / "src/faction.h").read_text()
    declarations = []
    for name in ("sat_odp_deployed", "satellites_"):
        matches = re.findall(r"^\s*(uint32_t " + name + r"(?:\[4\])?;)", header, re.M)
        assert len(matches) == 1
        declarations.append(matches[0])
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl
struct PlayerData {
    unsigned char prefix[0x364];
    DEPLOYED
    unsigned char middle[0x808-0x368];
    SATELLITES
    unsigned char tail[0x20CC-0x818];
};
static_assert(offsetof(PlayerData,satellites_)==0x808,"counter address");
static_assert(offsetof(PlayerData,sat_odp_deployed)==0x364,"deployed address");
static_assert(sizeof(PlayerData)==0x20CC,"faction stride");
static PlayerData PlayersData[2];
static std::vector<int> calls;
static int key;
static int GetAsyncKeyState(int code){assert(code==0x10);calls.push_back(1);return key;}
static void see_map_check(){calls.push_back(2);}
struct ReportWin{void redraw(){calls.push_back(3);}};
static ReportWin ReportWinGlobal;
BODY
int main(){
    const uint32_t values[]={0,1,7,0x7fffffffu,0x80000000u,0xffffffffu};
    for(int faction=0;faction!=2;++faction)for(int type=0;type!=6;++type)
    for(int enabled: {0,1,-2})for(int shift: {0,1,0x100,0x8000})
    for(uint32_t start:values)for(uint32_t deployed:values){
        std::memset(PlayersData,0xA5,sizeof PlayersData);
        for(int i=0;i!=4;++i)PlayersData[faction].satellites_[i]=start;
        PlayersData[faction].sat_odp_deployed=deployed;
        PlayerData expected[2];std::memcpy(expected,PlayersData,sizeof expected);
        calls.clear();key=shift;
        std::vector<int> events;
        if(type<4 && enabled){
            events={1,2,3};uint32_t &count=expected[faction].satellites_[type];
            if((shift&0xff00)==0)++count;
            else {
                if(count)--count;
                // Independent signed interpretation of the wrapped dword.
                const uint32_t difference=expected[faction].satellites_[3]-deployed;
                const int64_t signed_difference=difference<=0x7fffffffu
                    ? int64_t(difference):int64_t(difference)-0x100000000LL;
                if(type==3 && signed_difference<0)
                    expected[faction].sat_odp_deployed=expected[faction].satellites_[3];
            }
        }
        sat_edit(enabled,faction,type);
        assert(calls==events);
        assert(!std::memcmp(PlayersData,expected,sizeof expected));
    }
    // Negative indices remain admitted by the original guard. Their invalid
    // indexing is a target-compiler obligation, not an ISO host-runtime proof.
    calls.clear();sat_edit(0,0,-1);assert(calls.empty());
}
'''.replace("DEPLOYED", declarations[0]).replace("SATELLITES", declarations[1]).replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("report_faction_id != 0", "report_faction_id == 0"),
    ("++PlayersData[target_faction_id].satellites_[satellite_type];", "++PlayersData[0].satellites_[satellite_type];"),
    ("satellite_type < 4", "satellite_type < 3"),
    ("shift_high == 0", "shift_high != 0"),
    ("++PlayersData[target_faction_id].satellites_[satellite_type];", "PlayersData[target_faction_id].satellites_[satellite_type] += 2;"),
    ("*field = *field - 1;", "*field = *field + 1;"),
    ("& 0x80000000U", "& 0x40000000U"),
    ("PlayersData[target_faction_id].sat_odp_deployed =", "PlayersData[target_faction_id].satellites_[0] =")))
def test_actual_satellite_members_and_mutants(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, satellite_control(), damage)


def test_original_unbounded_guard_and_source_log_storage():
    source = (ROOT / "src/stringstruct.cpp").read_text()
    body = extract((ROOT / "src/reportwin.cpp").read_text(), "void __cdecl sat_edit(")
    assert "if (satellite_type < 4 && report_faction_id != 0)" in body
    assert "satellite_type >= 0" not in body and "reinterpret_cast" not in body
    for name, text in (("StrDeletingVehicle", "Deleting vehicle"),
                       ("StrDelConvertedIndex", "DelConverted index")):
        assert f'static const char {name}[] = "{text}";' in source
    assert "(char *)0x0068C134" not in source and "(char *)0x0068C120" not in source
    enqueue = extract(source, "int DeletionList::add_deletion(")
    convert = extract(source, "int DeletionList::convert(")
    assert "log_say(StrDeletingVehicle, veh_id, Vehs[veh_id].x, Vehs[veh_id].y);" in enqueue
    assert "log_say(StrDelConvertedIndex, original_id, result, 0);" in convert

