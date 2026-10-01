"""Actual-source controls for the native social-window entry sequence."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body():
    text = (ROOT / "src/socialwin.cpp").read_text()
    start = text.index("void __cdecl social_select(int faction_id) {")
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated social_select")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
enum { STATE_SCENARIO_EDITOR=0x40, STATE_OMNISCIENT_VIEW=0x80 };
uint32_t GameState, NetTurnFlags;
int IsMultiplayerNet;
std::vector<int> events;
int chosen_faction, editor_result, editor_calls, sync_multiplayer=-1;
struct Console {
    int editor_who(int faction, int mode) {
        assert(faction==chosen_faction && mode==1);
        ++editor_calls; events.push_back(1);
        GameState=0; // The original retains its entry-state byte.
        return editor_result;
    }
} ConsoleGlobal;
struct SocialWin {
    void init(int faction) { chosen_faction=faction; events.push_back(2); }
    void close() { events.push_back(3); }
} TutWinSocWindow;
void synch_soc(int faction) {
    assert(faction==chosen_faction); events.push_back(4);
    if (sync_multiplayer!=-1) IsMultiplayerNet=sync_multiplayer;
}
void social_set(uint32_t faction) {
    assert(faction==static_cast<uint32_t>(chosen_faction)); events.push_back(5);
}
__BODY__
void reset(uint32_t state, uint32_t turn, int multiplayer) {
    GameState=state; NetTurnFlags=turn; IsMultiplayerNet=multiplayer;
    events.clear(); editor_calls=0; editor_result=6;
    chosen_faction=3; sync_multiplayer=-1;
}
int main() {
    reset(0,8,0); social_select(3); assert(events.empty());
    reset(0x10000,8,0); social_select(3); assert(events.empty());
    reset(0,0x800,0); social_select(3);
    assert((events==std::vector<int>{2,3,4,5}));
    reset(0x40,8,0); social_select(3);
    assert(editor_calls==0 && (events==std::vector<int>{2,3,4,5}));
    reset(0x80,8,0); social_select(3);
    assert(chosen_faction==6 && (events==std::vector<int>{1,2,3,4,5}));
    reset(0xC0,8,0); editor_result=-1; social_select(3);
    assert((events==std::vector<int>{1}));
    reset(0,0,1); social_select(3);
    assert((events==std::vector<int>{2,3,4}));
    reset(0,0,0); sync_multiplayer=1; social_select(3);
    assert((events==std::vector<int>{2,3,4}));
    reset(0,0,1); sync_multiplayer=0; social_select(3);
    assert((events==std::vector<int>{2,3,4,5}));
    reset(0,0,0); social_select(-1);
    assert(chosen_faction==-1 && (events==std::vector<int>{2,3,4,5}));
}
"""


def test_social_select_native_guards_and_live_application_order(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "social_select.cpp"
    executable = tmp_path / "social_select"
    source.write_text(HARNESS.replace("__BODY__", body()))
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)
