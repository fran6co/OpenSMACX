"""Actual-body controls for aggression and network drop; not game acceptance.

Service doubles cover directional treaty lookup and reentrant wait state. They
do not prove multiplayer transport, missing helper behavior, or native layout.
"""

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_air_defense_popup_uses_original_alien_air_picture():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instruction = next(decoder.disasm(image.read(Span(0x4CC9CD, 0x4CC9D2)), 0x4CC9CD))
    assert instruction.mnemonic == "push" and instruction.op_str == "0x687f48"
    assert image.read(Span(0x687F48, 0x687F55)) == b"alair_sm.pcx\0"
    candidate = body("src/veh.cpp", "int __cdecl action_airdrop(")
    assert 'pop("AIRDEFENSES", 0xFFFFEC78, 0, "alair_sm.pcx")' in candidate


def body(path, signature):
    source = (ROOT / path).read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


def test_aggression_direction_and_network_reentrant_state(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
typedef uint32_t DWORD;
std::vector<int> calls;
struct Player { uint32_t diplo_treaties[8]; } PlayersData[8];
void double_cross(int a,int b,int c) {
    assert(a==2 && b==3 && c==-1); calls.push_back(10);
}
void treaty_on(int a,int b,int c) {
    assert(a==2 && b==3 && c==0x8010); calls.push_back(11);
}
AGGRESSION
struct Entry {uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10];};
struct Locks {void check_global() {calls.push_back(4);}};
struct NetDaemon {
    Entry lock_state_[24]{};
    Locks locks_;
    int veh_drop(int,int,int);
    int lock_veh(int *id,int flag,int x,int y,int mode);
    int wait(const char *,int *ack,int,int);
    void receive();
    void check_spock() {calls.push_back(5);}
} NetDaemonNet;
struct Console {int faction_id_;} ConsoleGlobal;
struct Vehicle {uint8_t faction_id,order;uint32_t state;int moves_expended;} Vehs[8];
struct Base {uint8_t faction_id_current;} Bases[8];
struct Map {uint8_t bit,val2;} tile;
struct Popup {
    int shown=0;
    void pop(const char *,int,int,void *) {++shown;}
    void close() {--shown;}
} popup;
Popup *NetMsgPopup=&popup;
int IsMultiplayerNet,NetBusyCount,ExitTurnLoop,NetDaemonWaitTicks;
int action_result,action_calls,lock_result,found_base,speed_result,action_mode;
bool network_stays;
DWORD timeGetTime() {return 100;}
void log_say(const char *,int,int,int) {}
void message_data(int,int,int,int,int,int) {calls.push_back(6);}
void wait_task() {calls.push_back(2);}
int base_at(int x,int y) {assert(x==5&&y==7);return found_base;}
Map *map_loc(int,int) {return &tile;}
int add_global() {return 1;}
void synch_veh(int) {}
int speed(int id,int mode) {assert(id==1&&mode==0);return speed_result;}
int action_airdrop(int id,int x,int y,int mode) {
    assert(id==1&&x==5&&y==7); ++action_calls; action_mode=mode;
    NetDaemonNet.lock_state_[1].leading_words_[3]=mode;
    if(!network_stays) IsMultiplayerNet=0;
    return action_result;
}
int NetDaemon::lock_veh(int *id,int flag,int x,int y,int mode) {
    assert(*id==1&&flag==4&&x==5&&y==7&&mode==4);return lock_result;
}
int NetDaemon::wait(const char *,int *ack,int,int) {*ack=0;return 7;}
void NetDaemon::receive() {
    calls.push_back(3);
    lock_state_[2].leading_words_[3]=0;
    NetDaemonNet.lock_state_[1].leading_words_[3]=99;
    ExitTurnLoop=27;
}
DROP
void reset() {
    NetDaemonNet=NetDaemon{};calls.clear();IsMultiplayerNet=1;
    NetBusyCount=0;ExitTurnLoop=0;NetDaemonWaitTicks=0;
    action_calls=0;action_result=1;lock_result=0;found_base=-1;
    speed_result=5;network_stays=true;popup.shown=0;
    Vehs[1].faction_id=2;Vehs[1].moves_expended=1;
}
int main() {
    int followed_vehicle = 17;
    PlayersData[2].diplo_treaties[3]=7;
    assert(act_of_aggression(2,3)==0);assert((calls==std::vector<int>{11}));
    calls.clear();PlayersData[3].diplo_treaties[2]=4;
    assert(act_of_aggression(2,3)==1);assert((calls==std::vector<int>{10,11}));
    reset();IsMultiplayerNet=0;action_result=9;
    assert(NetDaemonNet.veh_drop(1,5,7)==9&&action_mode==3);
    reset();NetDaemonNet.lock_state_[2].leading_words_[2]=1;
    NetDaemonNet.lock_state_[2].trailing_words_[3]=1;
    NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow=&followed_vehicle;
    assert(NetDaemonNet.veh_drop(1,5,7)==0);
    assert(NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow==&followed_vehicle&&action_calls==0);
    reset();lock_result=1;NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow=&followed_vehicle;
    assert(NetDaemonNet.veh_drop(1,5,7)==0);
    assert(NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow==&followed_vehicle&&action_calls==0);
    reset();network_stays=false;NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow=&followed_vehicle;
    assert(NetDaemonNet.veh_drop(1,5,7)==1);
    assert(NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow==&followed_vehicle);
    reset();NetDaemon other;
    other.lock_state_[2].leading_words_[3]=1;
    assert(other.veh_drop(1,5,7)==1);
    assert(NetDaemonNet.lock_state_[1].leading_words_[3]==1);
    assert(ExitTurnLoop==27&&NetBusyCount==0);
    assert((calls==std::vector<int>{6,2,3,4,5,6}));
}
'''.replace("AGGRESSION", body("src/faction.cpp", "int __cdecl act_of_aggression("))
    harness = harness.replace("DROP", body("src/netdaemon.cpp", "int NetDaemon::veh_drop("))
    unit = tmp_path / "network.cpp"
    unit.write_text(harness)
    executable = tmp_path / "network"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
