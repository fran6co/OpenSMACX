"""Actual-source controls for the owned first-colony monument record."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_monument_records_behavior import body


ROOT = Path(__file__).resolve().parents[2]


def test_colony_foundation_native_flow_and_name_storage(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    header = (ROOT / "src/game.h").read_text()
    layout = header[header.index("struct MonumentRecord {"):
                    header.index("extern MonumentFaction FactionMonuments")]
    implementation = body("src/game.cpp", "void __cdecl mon_colony_founded(int faction_id, char *base_name) {")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#define __cdecl
enum { MaxPlayerNum=8, RFLAG_ALIEN=0x80 };
__LAYOUT__
MonumentFaction FactionMonuments[8];
struct Player { int rule_flags; } Players[8];
struct Console { int faction_id_; } ConsoleGlobal;
int TurnCurrentNum=21, year_calls, popup_calls, change_local=-1;
uint32_t game_year(int turn) {
    assert(turn==21); ++year_calls;
    if (change_local!=-1) ConsoleGlobal.faction_id_=change_local;
    return 2150;
}
void monument(int from_load) { assert(from_load==1); ++popup_calls; }
__BODY__
void reset() {
    memset(FactionMonuments,0,sizeof FactionMonuments);
    memset(Players,0,sizeof Players);
    year_calls=popup_calls=0; change_local=-1; ConsoleGlobal.faction_id_=3;
}
int main() {
    static_assert(sizeof(MonumentFaction)==0x4F4,"native faction stride");
    static_assert(offsetof(MonumentFaction,founding_base_name_)==0x4AC,"native name");
    char name[]="First Base";
    reset(); memset(FactionMonuments[3].founding_base_name_,0x7F,24);
    FactionMonuments[3].records_[1].year_=123;
    mon_colony_founded(3,name);
    const MonumentRecord &r=FactionMonuments[3].records_[0];
    assert(r.year_==2150 && r.achieved_==1 && r.word_14_==1);
    assert(r.owner_faction_==3 && popup_calls==1 && year_calls==1);
    assert(FactionMonuments[3].latest_type_==0);
    assert(FactionMonuments[3].records_[1].year_==123);
    assert(strcmp(FactionMonuments[3].founding_base_name_,name)==0);
    for (int i=10;i<24;++i) assert(FactionMonuments[3].founding_base_name_[i]==0);
    MonumentFaction saved=FactionMonuments[3];
    mon_colony_founded(3,name);
    assert(year_calls==2 && popup_calls==1);
    assert(memcmp(&saved,&FactionMonuments[3],sizeof saved)==0);
    reset(); Players[3].rule_flags=RFLAG_ALIEN;
    saved=FactionMonuments[3]; mon_colony_founded(3,name);
    assert(year_calls==1 && popup_calls==0);
    assert(memcmp(&saved,&FactionMonuments[3],sizeof saved)==0);
    reset();
    FactionMonuments[1].records_[0].achieved_=1;
    FactionMonuments[1].records_[0].word_14_=1;
    FactionMonuments[7].records_[0].achieved_=-1;
    FactionMonuments[7].records_[0].word_14_=-2;
    Players[7].rule_flags=RFLAG_ALIEN; // Native does not filter peer flags.
    mon_colony_founded(3,name);
    assert(FactionMonuments[3].records_[0].owner_faction_==7);
    assert(FactionMonuments[3].records_[0].word_14_==0);
    reset(); FactionMonuments[1].records_[0].achieved_=1;
    mon_colony_founded(3,name);
    assert(FactionMonuments[3].records_[0].owner_faction_==3);
    assert(FactionMonuments[3].records_[0].word_14_==0);
    reset(); ConsoleGlobal.faction_id_=1; change_local=3;
    char long_name[]="123456789012345678901234EXTRA";
    FactionMonuments[3].opaque_tail_suffix_[0]=0x5A;
    mon_colony_founded(3,long_name);
    assert(popup_calls==1);
    assert(memcmp(FactionMonuments[3].founding_base_name_,long_name,24)==0);
    assert(FactionMonuments[3].founding_base_name_[23]=='4');
    assert(FactionMonuments[3].opaque_tail_suffix_[0]==0x5A);
    reset(); ConsoleGlobal.faction_id_=1; mon_colony_founded(3,name);
    assert(popup_calls==0);
}
'''
    source = tmp_path / "colony.cpp"
    executable = tmp_path / "colony"
    source.write_text(harness.replace("__LAYOUT__", layout).replace("__BODY__", implementation))
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True, timeout=10)


def test_founding_action_uses_owned_dependencies_and_faction_count():
    implementation = body("src/leaf_recoveries.cpp", "int __cdecl action_build(int veh_id, char *base_name) {")
    assert "PlayersData[owner].current_num_bases" in implementation
    assert "base_init(owner, x, y)" in implementation
    assert "mon_colony_founded(owner, Bases[base_id].name_string)" in implementation
    for retired_address in ("0x004E4B80", "0x00476B70", "FactionRelationSlot"):
        assert retired_address not in implementation


def test_founding_focus_reloads_vehicle_owner_after_callbacks(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    implementation = body("src/leaf_recoveries.cpp", "int __cdecl action_build(int veh_id, char *base_name) {")
    harness = r'''
#include <cassert>
#include <cstring>
#define __cdecl
struct Veh { int x,y; unsigned char faction_id; unsigned flags,visibility,state; short home_base_id; } Vehs[1];
struct PlayerData { int current_num_bases; unsigned diplo_treaties[8]; int last_turn_new_base; } PlayersData[8];
struct Base { unsigned governor; char name_string[32]; int faction_id_current; unsigned visibility; } Bases[1];
int focus_calls, captured_owner, new_owner, skip_calls, TurnCurrentNum, GameState;
enum { STATE_OMNISCIENT_VIEW=0x80 };
struct Console {
    int faction_id_;
    void focus(int x,int y,int owner) {
        assert(x==6 && y==8); ++focus_calls; captured_owner=owner;
    }
} ConsoleGlobal;
int BuildLogCaption[1], BuildLogTurnCaption[1];
void log_say(char*,int,int,int) { Vehs[0].faction_id=new_owner; }
int rocky_at(int,int) { return 0; }
void rocky_set(int,int,int) {}
int whose_territory(int,int,int,int,int) { return -1; }
int is_human(int) { return 0; }
void treaty_on(int,int,int) {}
void double_cross(int,int,int) {}
int base_init(int owner,int x,int y) {
    assert(owner==1 && x==6 && y==8); return -1;
}
void make_base_unique(int) {}
void veh_kill(int) {}
void draw_map(int) {}
void del_site(int,int,int,int,int) {}
void mon_colony_founded(int,char*) {}
void veh_skip(int) { ++skip_calls; }
__BODY__
int main() {
    ConsoleGlobal.faction_id_=3;
    Vehs[0].x=6; Vehs[0].y=8; Vehs[0].faction_id=1;
    new_owner=3;
    assert(action_build(0,0)==-1);
    assert(focus_calls==1 && captured_owner==1 && skip_calls==1);
    // The original's second test reloads the vehicle faction, but still
    // supplies the owner captured at entry to focus and base_init.
    Vehs[0].faction_id=1; Vehs[0].flags=0x440; focus_calls=0;
    assert(action_build(0,0)==-1 && focus_calls==0);
    Vehs[0].faction_id=1; Vehs[0].flags=0; new_owner=2;
    assert(action_build(0,0)==-1 && focus_calls==0);
}
'''
    source = tmp_path / "founding.cpp"
    executable = tmp_path / "founding"
    source.write_text(harness.replace("__BODY__", implementation))
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True, timeout=10)
