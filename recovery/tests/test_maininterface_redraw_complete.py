"""Exercise the actual C++ body against original 45C3A0 branch evidence.

Observers model Flic callbacks changing network readiness. This is a bounded
control of branch/call sequencing, not a claim that the application works.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


def product_body():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/maininterface.cpp").read_text()
    start = source.index("void MainInterface::redraw_complete() {")
    return source[start:source.index("\n}\n", start) + 3]


def harness(body):
    return r'''
#include <cstring>
typedef unsigned int uint32_t;
struct RECT {};
int IfaceIdToSlotStorage[30], ActiveIfaceIdStorage;
int IsMultiplayerNet;
struct GameDataStateFields { uint32_t game_state_, net_turn_flags_; int net_turn_faction_; };
struct GameDataStateWindow { GameDataStateFields fields_; };
GameDataStateWindow GameDataStateStorage;
int MainInterfaceEndTurnAnimation, MainInterfaceRedrawCount;
struct Lock { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct Daemon { Lock lock_state_[24]; } NetDaemonNet;
struct Console { int faction_id_, field_23BE4_; } ConsoleGlobal;
int selected_label, rewinds, decodes, updates;
uint32_t callback_ready;
const char *label_get(int id) { selected_label=id; return "turn"; }
void CharUpperA(char *) {}
struct Font {};
struct Buffer {
    void set_text_color(int,int,int,int) {}
    void set_font(Font *,int,int,int) {}
    void write_cent_l(const char *,RECT *,unsigned int) {}
};
struct Spot { void get_rect(int,int,RECT *) {} };
struct Flic {
    void rewind() { ++rewinds; }
    void decode_stored_frame() {
        ++decodes;
        NetDaemonNet.lock_state_[0].trailing_words_[7]=callback_ready;
    }
};
struct MainInterface : Buffer {
    int field_4_;
    Spot spot_;
    Font font1_;
    Flic flic2_;
    void soft_update(RECT *) { ++updates; }
    void redraw_complete();
} MainInterfaceGlobal;
''' + body + r'''
int main() {
    const uint32_t readiness[]={0,255,4,8};
    ConsoleGlobal.faction_id_=2;
    for(int mode=0;mode<2;++mode)
    for(int front=0;front<2;++front)
    for(int game=0;game<2;++game)
    for(int net=0;net<2;++net)
    for(int simultaneous=0;simultaneous<2;++simultaneous)
    for(int same=0;same<2;++same)
    for(int console_ready=0;console_ready<2;++console_ready)
    for(int before=0;before<4;++before)
    for(int after=0;after<4;++after) {
        IfaceIdToSlotStorage[0]=mode;
        MainInterfaceGlobal.field_4_=front ? 10 : 0;
        GameDataStateStorage.fields_.game_state_=game ? 2u : 0u;
        IsMultiplayerNet=net;
        GameDataStateStorage.fields_.net_turn_flags_=simultaneous ? 16u : 0u;
        GameDataStateStorage.fields_.net_turn_faction_=same ? 2 : 3;
        ConsoleGlobal.field_23BE4_=console_ready;
        NetDaemonNet.lock_state_[0].trailing_words_[7]=readiness[before];
        callback_ready=readiness[after];
        MainInterfaceRedrawCount=0;
        MainInterfaceEndTurnAnimation=-1;
        selected_label=-1;
        rewinds=decodes=updates=0;
        MainInterfaceGlobal.redraw_complete();
        if(mode && !front) {
            if(rewinds || decodes || updates || MainInterfaceRedrawCount) return 1;
            continue;
        }
        // Original45C457 admits flag-clear; flag-set requires same faction.
        bool animate=game && (!net || !simultaneous || same)
            && (console_ready || (net && readiness[before]!=0 && readiness[before]!=255));
        if(rewinds!=(animate ? 0 : 1) || decodes!=1 || updates!=1) return 2;
        if(MainInterfaceEndTurnAnimation!=(animate ? 1 : 0)) return 3;
        // Original45C4EC reloads readiness AFTER Flic callback calls.
        int expected=(!net || !game || simultaneous || readiness[after]==255
            || !(readiness[after] & 4)) ? 0x68/4 : 0x3FC/4;
        if(selected_label!=expected || MainInterfaceRedrawCount!=1) return 4;
    }
    return 0;
}
'''


@pytest.mark.parametrize("damage", [None, "flag", "stale_readiness", "game", "state"])
def test_product_maininterface_redraw_complete(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = product_body()
    mutations = {
        "flag": ("|| !(GameDataStateStorage.fields_.net_turn_flags_ & 0x10)",
                 "|| (GameDataStateStorage.fields_.net_turn_flags_ & 0x10)"),
        "stale_readiness": (
            "const uint32_t label_ready = NetDaemonNet.lock_state_[0].trailing_words_[7];",
            "const uint32_t label_ready = ready;"),
        "game": ("(GameDataStateStorage.fields_.game_state_ & 2) &&", "true &&"),
        "state": ("MainInterfaceEndTurnAnimation = 1;", "MainInterfaceEndTurnAnimation = 0;"),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    source = tmp_path / "redraw_complete.cpp"
    source.write_text(harness(body))
    executable = tmp_path / "redraw_complete"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None)
