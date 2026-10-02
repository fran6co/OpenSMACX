"""Actual Council voting bodies with typed host doubles and damage controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body_for(signature):
    source = (ROOT / "src/councwin.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


VOTING_HARNESS = r'''#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct Call { std::string name; int a, b, c, d, e, f; };
static std::vector<Call> calls;
static void record(const char *name, int a=-999, int b=-999, int c=-999,
                   int d=-999, int e=-999, int f=-999) {
    calls.push_back(Call{name,a,b,c,d,e,f});
}
static void check(bool ok, const char *message) {
    if (!ok) { std::cerr << "FAIL: " << message << "\n"; std::exit(1); }
}
static void clear_calls() { calls.clear(); }

enum { MaxPlayerNum=8, RFLAG_ALIEN=0x80 };
struct Player { uint32_t rule_flags=0; };
struct PlayerData { int unk_29[32]={}; int unk_30[32]={}; };
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static uint32_t LockEnableMask;
static int GovernorFaction;
static int TurnCurrentNum;
static int IsMultiplayerNet;
static int IsMultiplayerPBEM;
static int CouncilPbemGate;
static int CouncilNotifyFormSlots[8];
static int CouncilNotifyPluralSlots[8];
static int CouncilLastResolutionTurn[32];
struct ConsoleGlobalDouble { int faction_id_=0; };
static ConsoleGlobalDouble ConsoleGlobal;
struct NetState { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct NetDaemonDouble { NetState lock_state_[24]; };
static NetDaemonDouble NetDaemonNet;

static int ai_votes[MaxPlayerNum];
static int ai_call_count;
static int ai_mutate_after_first;
static uint32_t ai_mask_after_first;
static int message_data_count;
static int message_data_result;
static uint32_t message_data(int opcode,int target,int p10,int p14,int p18,int p1c) {
    record("message_data",opcode,target,p10,p14,p18,p1c);
    ++message_data_count;
    return static_cast<uint32_t>(message_data_result);
}
static int council_get_vote(int faction,int proposal,int choice) {
    record("council_get_vote",faction,proposal,choice);
    ++ai_call_count;
    if (ai_mutate_after_first && ai_call_count==1) LockEnableMask=ai_mask_after_first;
    return ai_votes[faction];
}
static void council_action(int proposal,int choice) { record("council_action",proposal,choice); }

class GraphicWin {
public:
    virtual void redraw() { record("redraw"); }
};
struct Button {
    int id=0;
    void hide() { record("hide",id); }
    void show(int value) { record("show",id,value); }
};
class CouncWin : public GraphicWin {
public:
    uint32_t field_A1C_=0, field_A20_=0, field_A24_=0;
    uint32_t leader_label_kind_[8]={};
    int leader_tech_[8]={};
    Button buttons_[6];
    int make_resolution(int resolve);
    void set_vote(int faction_id,int vote);
};

static void reset(CouncWin &c) {
    clear_calls();
    LockEnableMask=0; GovernorFaction=1; TurnCurrentNum=1;
    IsMultiplayerNet=0; IsMultiplayerPBEM=0; CouncilPbemGate=99;
    ConsoleGlobal.faction_id_=1;
    NetDaemonNet=NetDaemonDouble{};
    ai_call_count=0; ai_mutate_after_first=0; ai_mask_after_first=0;
    message_data_count=0; message_data_result=0;
    for(int i=0;i<8;++i) {
        Players[i]=Player{}; PlayersData[i]=PlayerData{};
        c.leader_label_kind_[i]=0; c.leader_tech_[i]=1;
        ai_votes[i]=-1;
    }
    for(int i=0;i<6;++i) c.buttons_[i].id=i;
    for(int i=0;i<8;++i) { CouncilNotifyFormSlots[i]=0; CouncilNotifyPluralSlots[i]=0; }
    for(int i=0;i<32;++i) CouncilLastResolutionTurn[i]=0;
    c.field_A1C_=2; c.field_A20_=0; c.field_A24_=0;
}
static void set_status(uint8_t humans,uint8_t alive) {
    LockEnableMask=static_cast<uint32_t>(humans) |
                   (static_cast<uint32_t>(alive)<<8);
}
static void make_all_votes_nonzero(CouncWin &c) {
    for(int f=1;f<MaxPlayerNum;++f) c.leader_label_kind_[f]=static_cast<uint32_t>(-1);
}
static int count_calls(const char *name) {
    int count=0; for(const Call &c:calls) if(c.name==name) ++count; return count;
}

static void test_unresolved_alien_live_and_weights() {
    CouncWin c; reset(c); make_all_votes_nonzero(c); set_status(0,0xff);
    c.leader_label_kind_[2]=0;
    check(c.make_resolution(1)==0 && calls.empty(),"zero vote blocks even before live filtering");

    reset(c); make_all_votes_nonzero(c); set_status(0,0xff);
    Players[2].rule_flags=RFLAG_ALIEN; c.leader_label_kind_[2]=0;
    check(c.make_resolution(1)==1 && c.field_A24_==1,"alien zero vote is excluded");

    reset(c); make_all_votes_nonzero(c); set_status(0,1u<<1);
    c.leader_label_kind_[2]=0;
    check(c.make_resolution(1)==0,"dead non-alien zero vote still blocks resolution");

    reset(c); set_status(0,(1u<<1)|(1u<<2));
    for(int f=3;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    c.field_A1C_=1; c.leader_label_kind_[1]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[2]=static_cast<uint32_t>(-2);
    c.leader_tech_[1]=3; c.leader_tech_[2]=1;
    check(c.make_resolution(1)==1 && c.field_A24_==1,
          "proposal one uses tech weights and three-quarter threshold");

    reset(c); set_status(0,(1u<<1)|(1u<<2));
    for(int f=3;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    c.field_A1C_=2; c.leader_label_kind_[1]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[2]=static_cast<uint32_t>(-2);
    c.leader_tech_[1]=3; c.leader_tech_[2]=1;
    check(c.make_resolution(1)==1 && c.field_A24_==static_cast<uint32_t>(-1),
          "proposal two uses one-person votes, not tech weights");

    reset(c); set_status(0,(1u<<1)|(1u<<2));
    for(int f=3;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    c.leader_label_kind_[1]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[2]=static_cast<uint32_t>(-3);
    check(c.make_resolution(1)==1 && c.field_A24_==1,
          "out-of-range negative vote is ignored rather than counted against");
}

static void test_governor_objection_and_ties() {
    CouncWin c; reset(c); set_status(0,(1u<<1)|(1u<<2)|(1u<<3)|(1u<<4));
    for(int f=5;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    GovernorFaction=2;
    c.leader_label_kind_[1]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[2]=static_cast<uint32_t>(-2);
    c.leader_label_kind_[3]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[4]=static_cast<uint32_t>(-1);
    check(c.make_resolution(1)==1 && c.field_A24_==static_cast<uint32_t>(-2),
          "governor objection vetoes otherwise successful resolution");
    check(count_calls("council_action")==0,"objection does not execute council action");

    reset(c); c.field_A1C_=0; TurnCurrentNum=1; set_status(0,(1u<<1)|(1u<<2));
    for(int f=3;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    GovernorFaction=1;
    c.leader_label_kind_[1]=1; c.leader_label_kind_[2]=2;
    check(c.make_resolution(1)==1 && c.field_A20_==1 && c.field_A24_==1,
          "proposal-option tie preserves governor");

    reset(c); c.field_A1C_=0; TurnCurrentNum=1; set_status(0,(1u<<2)|(1u<<3));
    Players[1].rule_flags=RFLAG_ALIEN;
    for(int f=4;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    GovernorFaction=1;
    c.leader_label_kind_[2]=2; c.leader_label_kind_[3]=3;
    check(c.make_resolution(1)==1 && c.field_A20_==0 &&
          c.field_A24_==static_cast<uint32_t>(-1),"non-governor option tie is unresolved");
}

static void test_pbem_and_ui_completion() {
    CouncWin c; reset(c); c.field_A1C_=2; make_all_votes_nonzero(c);
    set_status((1u<<1)|(1u<<2),(1u<<1)|(1u<<2));
    for(int f=3;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    c.leader_label_kind_[1]=static_cast<uint32_t>(-1);
    c.leader_label_kind_[2]=static_cast<uint32_t>(-1);
    IsMultiplayerPBEM=1; ConsoleGlobal.faction_id_=1;
    check(c.make_resolution(1)==1 && c.field_A24_==1,"PBEM majority resolves");
    check(CouncilPbemGate==0,"resolution clears PBEM arrival gate");
    check(CouncilNotifyFormSlots[0]==0 && CouncilNotifyPluralSlots[0]==0,
          "local human faction receives no PBEM notice");
    check(CouncilNotifyFormSlots[1]==2 && CouncilNotifyPluralSlots[1]==1,
          "other human receives proposal and resolution notice");
    check(CouncilLastResolutionTurn[2]==TurnCurrentNum,"resolution turn recorded");
    for(int f=1;f<8;++f) check(PlayersData[f].unk_29[2]==0 && PlayersData[f].unk_30[2]==0,
                               "resolution clears per-faction proposal state");
    check(count_calls("redraw")==1 && count_calls("hide")==6 &&
          count_calls("show")==6,"resolution redraws and completes all six buttons");
    check(count_calls("council_action")==0,"offline resolution has no network council action");
}

static void test_set_vote_ai_rounds_and_human_guard() {
    CouncWin c; reset(c); set_status(1u<<1,(1u<<2)|(1u<<3));
    c.field_A1C_=4; c.field_A20_=6;
    for(int f=1;f<8;++f) Players[f].rule_flags=RFLAG_ALIEN;
    Players[2].rule_flags=0; Players[3].rule_flags=0;
    ai_votes[2]=-1; ai_votes[3]=-2;
    IsMultiplayerNet=1; // lock-state gate stays closed: no resolution after the rounds
    c.set_vote(1,0);
    check(ai_call_count==6,"non-human live AI voters get three rounds");
    for(const Call &call:calls) if(call.name=="council_get_vote")
        check((call.a==2 || call.a==3) && call.b==4 && call.c==6,
              "AI vote argument roles and live-voter filtering");

    reset(c); set_status((1u<<1)|(1u<<2),(1u<<1)|(1u<<2));
    Players[2].rule_flags=0; ai_votes[2]=-1;
    c.set_vote(1,-1);
    check(ai_call_count==0,"one human vote suppresses automatic AI rounds");

    reset(c); c.field_A24_=1; c.set_vote(1,-1);
    check(calls.empty() && c.leader_label_kind_[1]==0,"completed council ignores later vote");
}

static void test_live_ai_mask_refresh_and_network_boundary() {
    CouncWin c; reset(c); set_status(1u<<1,1u<<2);
    Players[2].rule_flags=0; Players[3].rule_flags=0;
    ai_votes[2]=-1; ai_votes[3]=-2;
    ai_mutate_after_first=1;
    ai_mask_after_first=(1u<<1)|(((1u<<2)|(1u<<3))<<8);
    IsMultiplayerNet=1;
    c.set_vote(1,0);
    check(ai_call_count==6,"live AI mask is reloaded after callback and across rounds");
    check(count_calls("council_get_vote")==6,"AI mask refresh call trace");

    reset(c); make_all_votes_nonzero(c); set_status(1u<<1,1u<<1);
    IsMultiplayerNet=1; NetDaemonNet.lock_state_[0].leading_words_[1]=0;
    c.set_vote(1,-1);
    check(message_data_count==0 && count_calls("hide")==0,
          "network without resolution boundary does not resolve or notify");

    reset(c); make_all_votes_nonzero(c); set_status(1u<<1,1u<<1);
    IsMultiplayerNet=1; NetDaemonNet.lock_state_[0].leading_words_[1]=1;
    c.set_vote(1,-1);
    check(message_data_count==1 && calls.back().name=="message_data" &&
          calls.back().a==0x2603,"network resolution boundary emits opcode 0x2603");

    reset(c); make_all_votes_nonzero(c); set_status(1u<<1,1u<<1);
    c.leader_label_kind_[2]=0; IsMultiplayerNet=1;
    NetDaemonNet.lock_state_[0].leading_words_[1]=1;
    c.set_vote(1,-1);
    check(message_data_count==0,"incomplete network council does not announce resolution");
}

static void test_set_vote_pbem_button_notices() {
    CouncWin c; reset(c); IsMultiplayerPBEM=1; ConsoleGlobal.faction_id_=1;
    set_status(1u<<1,1u<<1);
    // Other factions remain non-alien with no vote, so the attempted resolution
    // stays incomplete and only set_vote's PBEM-specific button sequence runs.
    IsMultiplayerNet=1;
    c.set_vote(1,-1);
    check(count_calls("hide")==6 && count_calls("show")==1,
          "local PBEM vote hides six controls and reopens passover");
    check(calls.back().name=="show" && calls.back().a==5 && calls.back().b==0,
          "PBEM passover button receives zero mode");
}

int main() {
    test_unresolved_alien_live_and_weights();
    test_governor_objection_and_ties();
    test_pbem_and_ui_completion();
    test_set_vote_ai_rounds_and_human_guard();
    test_live_ai_mask_refresh_and_network_boundary();
    test_set_vote_pbem_button_notices();
    std::cout << "CouncWin voting fixture passed\n";
}
'''


@pytest.mark.parametrize("damage", [None, "vote_bound", "ai_rounds"])
def test_council_voting_native_branches_and_callback_order(tmp_path, damage):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    methods = body_for("int CouncWin::make_resolution(") + "\n" + body_for("void CouncWin::set_vote(")
    if damage == "vote_bound":
        assert "vote >= -2" in methods
        methods = methods.replace("vote >= -2", "vote >= -3", 1)
    elif damage == "ai_rounds":
        assert "int rounds = 3;" in methods
        methods = methods.replace("int rounds = 3;", "int rounds = 2;", 1)
    unit = tmp_path / "council_voting.cpp"
    unit.write_text(VOTING_HARNESS + "\n" + methods)
    executable = tmp_path / "council_voting"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    # LeakSanitizer cannot inspect threads under the managed ptrace sandbox;
    # retain address/undefined checks, disabling only leak enumeration.
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            env=environment)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr
    if damage is not None:
        assert "FAIL:" in result.stderr, result.stdout + result.stderr
