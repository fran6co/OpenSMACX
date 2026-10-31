"""Original callback read timing through actual Console method bodies.

Service doubles and bounded rows establish effects, not target layout/provider ABI.
"""
from pathlib import Path
import shutil
import re
import subprocess
import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
ROOT = Path(__file__).resolve().parents[2]


def definition(signature):
    text = (ROOT / 'src/console.cpp').read_text()
    masked = COMMENTS_OR_STRINGS.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)
    start = masked.index(signature)
    opening = masked.index('{', start)
    depth = 0
    for at in range(opening, len(masked)):
        depth += (masked[at] == '{') - (masked[at] == '}')
        if depth == 0:
            return text[start:at + 1]
    raise AssertionError('unterminated actual definition')


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
static const int MaxPlayerNum=8;
static const uint32_t PREF_BSC_TUTORIAL_MSGS=0x20, DTREATY_COMMLINK=8, DTREATY_VENDETTA=0x10;
struct GameFields { uint32_t game_preferences_, game_rules_; int turn_current_num_;
 struct { uint32_t enable_mask_; } lock_masks_; };
static struct { GameFields fields_; } GameDataStateStorage;
struct PlayerData { int current_num_bases, pop_total; unsigned char proto_id_active[1], proto_id_queue[1];
 int last_turn_new_base; uint32_t diplo_treaties[8]; };
static PlayerData PlayersData[8];
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char title_leader[32],name_leader[32],noun_faction[32]; };
static Player Players[8];
struct Base { int x,y; char name_string[16]; };
static Base Bases[2];
static struct { int proto_id; } Vehs[1];
static struct { const char *veh_name; } VehPrototypes[1];
struct Console { int faction_id_, selected_vehicle_, cursor_history_index_;
 int cursor_history_x_[8],cursor_history_y_[8];
 void turn_hints(); void show_scen_objectives(); void chat(int); void focus(int,int,int); };
static Console ConsoleGlobal;
struct TutWin { bool blocked; bool tutorial_blocked() { return blocked; }
 void set_tour_stage(int stage) { assert(stage==0); }
 void tut_map(const char *,int,int,int,void *,int,int,int); };
static TutWin TutorialWindow;
struct NetDaemon { struct Channel { uint32_t leading_words_[2],trailing_words_[7]; } channels_[2];
 struct { int session_phase_; uint32_t trailing_words_[4]; } primary_state_; };
static NetDaemon NetDaemonNet;
static struct NetMsg { void pop(const char *,int,int,void *) {} } NetMsgPrimary;
static int IsMultiplayerNet, ConsoleControlTurnActive, CommlinkPending,CommlinkFaction,TurnRedrawPending;
static int GenderDefault,PluralityDefault;
static struct { int objective_req_victory,objectives_sudden_death_victory,ending_mission_year; } ScenarioSave;
static char StringTemp[128],ConsoleChatInput[1];
static struct { char str[128]; } ParseStrBuffer[1];
static const char *TutorialPopupCaption="TUTOR", *StrScenarioObjectivesTitle="TITLE", *StrScenarioObjectivesPopup="OBJECTIVES";
static int mode,base_calls,tutorial_calls,objectives_faction;
static std::vector<int> recipients;
static std::vector<std::string> publications;
static void history(int index,int x,int y) {
 ConsoleGlobal.cursor_history_index_=index;
 ConsoleGlobal.cursor_history_x_[index]=x; ConsoleGlobal.cursor_history_y_[index]=y;
}
static void parse_num(int slot,int value) {
 if(mode==10) {
  if(slot==0) ConsoleGlobal.faction_id_=2;
  if(slot==1) ConsoleGlobal.faction_id_=3;
  if(slot==2) { assert(value==703); ConsoleGlobal.faction_id_=4; }
  if(slot==3) ConsoleGlobal.faction_id_=5;
 }
}
static int num_objectives(int faction,uint32_t) { objectives_faction=faction; return 700+faction; }
static void parse_says(int slot,const char *text,int,int) {
 publications.push_back(text);
 if(mode==10) {
  if(slot==1) { assert(std::string(text)=="title5" && GenderDefault==55 && PluralityDefault==0); ConsoleGlobal.faction_id_=6; }
  if(slot==2) { assert(std::string(text)=="name6" && GenderDefault==66 && PluralityDefault==0); ConsoleGlobal.faction_id_=7; }
  if(slot==3) { assert(std::string(text)=="noun7" && GenderDefault==707 && PluralityDefault==1); ConsoleGlobal.faction_id_=0; }
 }
 if(mode==1) { if(slot==0) history(2,220,221); if(slot==1) history(3,330,331); }
 if(mode==5 && slot==0) { Bases[0].x=50; Bases[0].y=60; }
}
static void say_year(char *out) { std::strcpy(out,"YEAR"); }
static int X_pop(const char *,const char *,void *) { return 0; }
static int tut_check(int mask) {
 if(mask==1) {
  if(mode==1) { history(1,110,111); return 1; }
  if(mode==2) { GameDataStateStorage.fields_.turn_current_num_=30; ConsoleGlobal.faction_id_=4; history(4,440,441); return 0; }
 }
 if(mask==0x100000) {
  if(mode==4) { GameDataStateStorage.fields_.turn_current_num_=40; PlayersData[1].current_num_bases=2; history(5,550,551); return 0; }
  if(mode==3 || mode==5) return 1;
 }
 return 0;
}
static int base_find(int x,int y,int faction) {
 ++base_calls; assert(faction==1);
 if(mode==1) assert(x==110 && y==111);
 if(mode==2) assert(x==440 && y==441);
 if(mode==4) assert(x==550 && y==551);
 if(mode==3) { PlayersData[1].current_num_bases=2; return -1; }
 return 0;
}
void Console::focus(int x,int y,int faction) {
 assert(faction==1 && x==10 && y==20);
 if(mode==5) { Bases[0].x=30; Bases[0].y=40; }
}
void TutWin::tut_map(const char *key,int x,int y,int,void *,int,int,int) {
 ++tutorial_calls;
 if(mode==1) assert(std::string(key)=="LANDFALL" && x==330 && y==331);
 if(mode==2 || mode==4) assert(std::string(key)=="BUILDMORE" && x==10 && y==20);
 if(mode==5) { assert(std::string(key)=="SECONDBASE" && x==50 && y==60); GameDataStateStorage.fields_.turn_current_num_=99; }
}
static int pop_wait() { return 0; }
static int X_pop_ask(const char *,int,const char *,int(*)(),int) { return 0; }
static void change_recipients() { GameDataStateStorage.fields_.lock_masks_.enable_mask_=0x200|2|8; }
static void message_chat(int kind,int faction,char *) {
 assert(kind==0x1e00); recipients.push_back(faction);
 if(mode==20 && faction==1) change_recipients();
}
static void log_say(const char *,char *,int faction,int,int) {
 if(mode==21 && faction==1) change_recipients();
}
__ACTUAL_BODIES__
static void reset(int chosen) {
 mode=chosen; base_calls=tutorial_calls=objectives_faction=0;
 ConsoleGlobal=Console{}; ConsoleGlobal.faction_id_=1; ConsoleGlobal.selected_vehicle_=0;
 GameDataStateStorage.fields_=GameFields{};
 GameDataStateStorage.fields_.game_preferences_=PREF_BSC_TUTORIAL_MSGS;
 for(int i=0;i<8;++i) {
  PlayersData[i]=PlayerData{}; PlayersData[i].current_num_bases=2;
  Players[i].is_leader_female=i*11; Players[i].noun_gender=700+i; Players[i].is_noun_plural=i&1;
  std::snprintf(Players[i].title_leader,32,"title%d",i);
  std::snprintf(Players[i].name_leader,32,"name%d",i);
  std::snprintf(Players[i].noun_faction,32,"noun%d",i);
 }
 Bases[0].x=10; Bases[0].y=20; std::strcpy(Bases[0].name_string,"BASE");
 VehPrototypes[0].veh_name="VEHICLE";
 TutorialWindow.blocked=false; IsMultiplayerNet=ConsoleControlTurnActive=0;
 CommlinkPending=CommlinkFaction=0; TurnRedrawPending=1;
 NetDaemonNet=NetDaemon{}; publications.clear();recipients.clear();
 std::strcpy(ParseStrBuffer[0].str,"hello");
}
int main() {
 reset(10); ConsoleGlobal.show_scen_objectives();
 assert(objectives_faction==3);
 assert((publications==std::vector<std::string>{"title5","name6","noun7","YEAR","YEAR"}));
 for(int m=20;m<=21;++m) {
  reset(m); IsMultiplayerNet=1; NetDaemonNet.primary_state_.session_phase_=2;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0x200|2|4;
  ConsoleGlobal.chat(0); assert((recipients==std::vector<int>{1,3}));
 }
 reset(1);GameDataStateStorage.fields_.turn_current_num_=1;ConsoleGlobal.turn_hints();
 assert(base_calls==1 && tutorial_calls==1);
 reset(2);GameDataStateStorage.fields_.turn_current_num_=1;ConsoleGlobal.turn_hints();
 assert(base_calls==1 && tutorial_calls==1);
 reset(3);GameDataStateStorage.fields_.turn_current_num_=30;PlayersData[1].current_num_bases=1;PlayersData[1].pop_total=2;
 ConsoleGlobal.turn_hints(); assert(base_calls==1 && tutorial_calls==0);
 reset(4);GameDataStateStorage.fields_.turn_current_num_=30;PlayersData[1].current_num_bases=1;PlayersData[1].pop_total=2;PlayersData[1].last_turn_new_base=15;
 ConsoleGlobal.turn_hints(); assert(base_calls==1 && tutorial_calls==1);
 reset(5);GameDataStateStorage.fields_.turn_current_num_=30;PlayersData[1].current_num_bases=1;PlayersData[1].pop_total=2;
 ConsoleGlobal.turn_hints();assert(base_calls==1 && tutorial_calls==1 && PlayersData[1].last_turn_new_base==99);
}
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'scenario_faction', 'chat_mask', 'find_history', 'landfall_history', 'first_turn', 'second_turn', 'failed_find', 'base_coordinates'])
def test_actual_console_callback_read_order(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f'{compiler_name} unavailable')
    scenario = definition('void Console::show_scen_objectives()')
    chat = definition('void Console::chat(int target_faction)')
    turn = definition('void Console::turn_hints()')
    if damage == 'scenario_faction':
        assert scenario.count('Players[ConsoleGlobal.faction_id_]') == 7
        scenario = scenario.replace('{', '{\n const int cached_faction=ConsoleGlobal.faction_id_;', 1)
        scenario = scenario.replace('Players[ConsoleGlobal.faction_id_]', 'Players[cached_faction]')
        scenario = scenario.replace('num_objectives(ConsoleGlobal.faction_id_,', 'num_objectives(cached_faction,')
    elif damage == 'chat_mask':
        pattern = r'\*reinterpret_cast<const unsigned char \*>\(\s*&GameDataStateStorage\.fields_\.lock_masks_\.enable_mask_\)'
        matches = list(re.finditer(pattern, chat))
        assert len(matches) == 1
        match = matches[0]
        chat = chat[:match.start()] + 'enabled' + chat[match.end():]
    elif damage == 'find_history':
        anchor = 'const int history = ConsoleGlobal.cursor_history_index_;'
        assert turn.count(anchor) == 3
        turn = turn.replace('{', '{\n const int old_history=ConsoleGlobal.cursor_history_index_;', 1)
        turn = turn.replace(anchor, 'const int history = old_history;')
    elif damage == 'landfall_history':
        anchor = 'const int tutorial_history = ConsoleGlobal.cursor_history_index_;'
        assert turn.count(anchor) == 1
        turn = turn.replace(anchor, 'const int tutorial_history = history;', 1)
    elif damage in ('first_turn', 'second_turn'):
        anchor = 'turn = GameDataStateStorage.fields_.turn_current_num_;'
        # One initialization plus the two post-check callback refreshes.
        assert turn.count(anchor) == 3
        at = turn.index(anchor, turn.index(anchor) + len(anchor))
        if damage == 'second_turn':
            at = turn.index(anchor, at + len(anchor))
        turn = turn[:at] + '(void)0;' + turn[at+len(anchor):]
    elif damage == 'failed_find':
        anchor = 'second_base_tutorial_shown = true;'
        assert turn.count(anchor) == 1
        turn = turn.replace(anchor, 'second_base_tutorial_shown = false;', 1)
    elif damage == 'base_coordinates':
        anchor = '"SECONDBASE", Bases[base_id].x, Bases[base_id].y,'
        assert turn.count(anchor) == 1
        turn = turn.replace(anchor, '"SECONDBASE", x, y,', 1)
    cpp, exe = tmp_path/'console.cpp', tmp_path/'console'
    cpp.write_text(HARNESS.replace('__ACTUAL_BODIES__', scenario+'\n'+chat+'\n'+turn))
    built = subprocess.run([compiler, '-std=c++11', '-O1', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',str(cpp),'-o',str(exe)],
                           capture_output=True,text=True)
    assert built.returncode == 0, built.stdout+built.stderr
    result = subprocess.run([str(exe)], capture_output=True,text=True)
    assert (result.returncode == 0) == (damage is None), result.stdout+result.stderr
