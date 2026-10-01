"""Host checks over the actual player-turn orchestration body."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body_for(signature):
    source = (ROOT / "src/console.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


HUMAN_TURN_DECLS = r'''#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
using std::string; using std::vector;
static vector<string> calls;
static void record(const char *s) { calls.emplace_back(s); }
static void require(bool b, const char *s) { if (!b) { std::fprintf(stderr,"FAIL %s\n",s); std::exit(1); } }
struct Veh { uint32_t state=0; int order=0, x=0, y=0, faction_id=0; };
Veh Vehs[16]; int VehCurrentCount=0;
struct ConsoleState { int cursor_active_=0, faction_id_=0, cursor_history_index_=0; int cursor_history_x_[32]={}, cursor_history_y_[32]={}; };
extern ConsoleState ConsoleGlobal;
struct Player { int is_leader_female=0, noun_gender=0, is_noun_plural=0; const char *title_leader="", *name_leader="", *noun_faction=""; } Players[8];
struct WorldWin { int viewport_outline_enabled_=0; void set_world_map(){record("world-map");} } WorldClimateWorldWin;
static int *live_selected_vehicle=nullptr;
static bool mutate_on_map_cursor=false;
struct MapWin { void draw_map(int){record("map-draw");} void draw_cursor(){record("map-cursor");if(mutate_on_map_cursor&&live_selected_vehicle)*live_selected_vehicle=0;} void main_caption(){record("caption");} } PrimaryMap;
MapWin *MapWinTable[8]={&PrimaryMap};
struct Info { void reset(){record("info-reset");} void change(int){record("info-change");} } ConsoleInfoWin;
struct Status { void reset(){record("status-reset");} void redraw(){record("status-redraw");} } ConsoleStatusWin;
struct Win { int visible=0; int is_visible(){record("visible"); return visible;} } TutWinBaseWindow, ReportWinGlobal;
struct Tutorial { void tour(){record("tour");} } TutorialWindow;
int TutWinTourPending=0;
struct Effect { bool playing=false; int is_playing(){record("fx-playing");return playing;} int load(){record("fx-load");return 0;} int set_reverb_mix(float f){require(f==0.0f,"reverb arg");record("fx-reverb");return 0;} int play(){record("fx-play");playing=true;return 0;} };
struct FX { Effect effects_[97]; Effect &effect(int i){return effects_[i];} Effect &get(int i){return effects_[i];} int play(int i){(void)i;record("fx-other");return 0;} Effect &operator[](int i){return effects_[i];} } g_FX;
// The source API is an inline Effect& accessor; keep the same call spelling.
int GameState=0, GameRules=0, GamePreferences=0, ControlTurnPhase=0;
int IsMultiplayerNet=0, IsMultiplayerPBEM=0, TurnCurrentNum=0, ExitTurnLoop=0;
int MapBaseDisplayOverride=0, GenderDefault=0, PluralityDefault=0;
int PREF_BSC_PAUSE_END_TURN=1, RULES_VICTORY_CONQUEST=2;
enum { STATE_UNK_400=0x400, VSTATE_UNK_8000000=0x08000000 };
int game_year(int y){return y+2100;}
static vector<int> parsed_ids; static int mutate_faction_after_parse=-1;
static int ready_calls=0; static int mutate_selection_on_ready_call=-1;
static int cursor_x=-1, cursor_y=-1;
static int veh_turn_calls=0;
int parse_says(int slot,const char *value,int,int){(void)slot;(void)value;parsed_ids.push_back(ConsoleGlobal.faction_id_);record("parse");if(mutate_faction_after_parse>0&&(int)parsed_ids.size()>=mutate_faction_after_parse)++ConsoleGlobal.faction_id_;return 0;}
int parse_num(int,int){record("parse-num");return 0;}
int X_pop(const char *,void *){record("popup");return 1;}
struct Popup { void pop(const char *,int,int,void *){record("net-pop");} } PopupObject;
Popup *NetMsgPopup=&PopupObject;
int end_calls=0; int end_after=999;
int end_of_game(int){++end_calls;return end_calls>=end_after;}
int wait_calls=0; bool toggle_cursor_on_wait=false;
void wait_loop(){++wait_calls;record("wait");if(toggle_cursor_on_wait)ConsoleGlobal.cursor_active_=1;if(wait_calls>1)ExitTurnLoop=1;}
void ambience(int){record("ambience");}
void draw_cursor(){record("draw-cursor");}
void flush_input(){record("flush-input");}
void go_reset(){record("go-reset");}
int veh_ready(int){++ready_calls;record("veh-ready");if(ready_calls==mutate_selection_on_ready_call&&live_selected_vehicle)*live_selected_vehicle=1;return 1;}
int veh_selectable(int){++ready_calls;record("veh-selectable");if(ready_calls==mutate_selection_on_ready_call&&live_selected_vehicle)*live_selected_vehicle=1;return 1;}
struct Alpha { void check(int){record("alpha-check");} };
struct Console {
  int selected_vehicle_=0, faction_id_=0; uint32_t cursor_active_=0, field_23BF8_=0, field_23BFC_=0,
    field_23BE4_=0, field_23BE8_=0, field_23BEC_=0, field_23BF0_=0, field_23BF4_=0,
    survey_mode_=0; int field_23D1C_=0, field_23D24_=0; Alpha alpha_menu_;
  int focus_x=-1,focus_y=-1,focus_faction=-1; int lock_result=1, turn_result=0, edit_state=0;
  void human_turn(); void set_move(int){record("set-move");} void ready(){record("ready");}
  int edit_lock(){edit_state=GameState;record("edit-lock");return lock_result;} void turn_hints(){record("hints");}
  int veh_turn(){++veh_turn_calls;record("veh-turn");if(turn_result<0)selected_vehicle_=-1;else if(turn_result==0&&veh_turn_calls>1)ExitTurnLoop=1;return turn_result;}
  int focus(int x,int y,int f){focus_x=x;focus_y=y;focus_faction=f;record("focus");return 0;}
  void cursor_next(int x,int y){cursor_x=x;cursor_y=y;record("cursor-next");}
};
ConsoleState ConsoleGlobal;
'''

HUMAN_TURN_TESTS = r'''
static void reset(Console &c){
 calls.clear(); parsed_ids.clear(); wait_calls=0; end_calls=0; end_after=999;
 ready_calls=0; veh_turn_calls=0; mutate_selection_on_ready_call=-1; cursor_x=cursor_y=-1; live_selected_vehicle=&c.selected_vehicle_;
 ControlTurnPhase=0; IsMultiplayerNet=0; IsMultiplayerPBEM=0; GameState=0; GameRules=0;
 GamePreferences=0; ExitTurnLoop=0; VehCurrentCount=0; ConsoleGlobal.cursor_active_=0;
 ConsoleGlobal.faction_id_=0; ConsoleGlobal.cursor_history_index_=0;
 toggle_cursor_on_wait=false; mutate_on_map_cursor=false;
 c=Console{}; c.lock_result=1; g_FX.get(57).playing=true;
 TutWinBaseWindow.visible=0; ReportWinGlobal.visible=0; mutate_faction_after_parse=-1;
}
int main(){
 Console c;
 live_selected_vehicle=&c.selected_vehicle_;
 // Native's first cursor-active guard routes nonzero to set_move(1); zero
 // takes the redraw path. This is the inverse of the tempting zero-test.
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=1; ExitTurnLoop=1;
 c.human_turn();
 require(std::count(calls.begin(),calls.end(),"set-move")==1,"nonzero initial cursor guard calls set_move");
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=0; ExitTurnLoop=1;
 c.human_turn();
 require(std::count(calls.begin(),calls.end(),"set-move")==0,"zero initial cursor guard redraws instead of set_move");

 // Vehicle-state clear is unconditional even when ControlTurnPhase starts zero.
 reset(c); ControlTurnPhase=0; VehCurrentCount=1; Vehs[0].state=0x08000001;
 c.selected_vehicle_=0; ExitTurnLoop=1; c.human_turn();
 require((Vehs[0].state&0x08000000)==0,"unconditional vehicle state clear");

 // Negative selection uses the owned effect and sets warning bit 4 from the
 // two real visibility checks (second call is short-circuited on first true).
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=-1; c.faction_id_=2; g_FX.get(57).playing=false;
 ConsoleGlobal.faction_id_=5; ConsoleGlobal.cursor_history_x_[0]=17; ConsoleGlobal.cursor_history_y_[0]=23;
 TutWinBaseWindow.visible=1; ReportWinGlobal.visible=1; ExitTurnLoop=1;
 c.lock_result=1; c.human_turn();
 auto fx_playing=std::find(calls.begin(),calls.end(),"fx-playing");
 require(fx_playing!=calls.end(),"effect 57 is queried");
 auto fx_load=std::find(calls.begin(),calls.end(),"fx-load");
 require(fx_load!=calls.end() && std::find(fx_load,calls.end(),"fx-reverb")!=calls.end() &&
         std::find(fx_load,calls.end(),"fx-play")!=calls.end(),"effect 57 load/reverb/play lifecycle");
 require(std::count(calls.begin(),calls.end(),"visible")==1,"visibility short-circuit");
 require((c.edit_state&4)!=0,"visible window sets end-turn warning bit before edit lock");
 require(c.focus_x==17 && c.focus_y==23 && c.focus_faction==2,
         "focus uses receiver faction, not ConsoleGlobal faction");
 auto hints=std::find(calls.begin(),calls.end(),"hints");
 auto fx=std::find(calls.begin(),calls.end(),"fx-playing");
 require(hints!=calls.end() && fx!=calls.end() && hints<fx,
         "FX effect query is after turn hints in negative-selection wait branch");

 // Eligibility can dispatch callbacks that change the current selection.
 // Native reloads before reading cursor coordinates.
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=0; ConsoleGlobal.cursor_active_=1;
 VehCurrentCount=2; Vehs[0].x=10; Vehs[0].y=20; Vehs[1].x=51; Vehs[1].y=61;
 mutate_selection_on_ready_call=2; ExitTurnLoop=0;
 c.human_turn();
 require(cursor_x==51 && cursor_y==61,"cursor coords use selection reloaded after eligibility callback");

 // Selected-vehicle cleanup clears bit 2 only; it must preserve conquest bit 1.
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=0; VehCurrentCount=1;
 GameState=3; ExitTurnLoop=0;
 // Run human_turn with veh_turn's normal exit behavior; the state transition
 // occurs before the method returns through ExitTurnLoop.
 c.human_turn();
 require((GameState&1)!=0,"selected-vehicle path preserves conquest bit 1");
 require((GameState&2)==0,"selected-vehicle path clears only bit 2");

 // The wait predicate is the live global cursor bit, not the already-cleared
 // ControlTurnPhase. One wait flips it; later endgame calls terminate outer loop.
 reset(c); c.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=0;
 toggle_cursor_on_wait=true; end_after=3; c.human_turn();
 require(wait_calls==1,"wait loop rechecks live cursor-active global");

 // An early edit-lock/manual-stop exit precedes turn_hints and the FX57 wait
 // branch, so it must not query or play that effect.
 reset(c); c.lock_result=0; GamePreferences=0;
 c.turn_result=-1; c.selected_vehicle_=0; ControlTurnPhase=1; VehCurrentCount=1;
 ExitTurnLoop=0;
 c.human_turn();
 require(std::find(calls.begin(),calls.end(),"fx-playing")==calls.end(),
         "early manual stop exits before FX57 wait-branch side effect");

 // Parser calls can change the global faction; native reloads it between
 // parse_says calls rather than carrying a cached faction local.
 reset(c); c.selected_vehicle_=-1; GameState=0x2000; GameRules=RULES_VICTORY_CONQUEST;
 ConsoleGlobal.faction_id_=1; mutate_faction_after_parse=1; ExitTurnLoop=1;
 c.human_turn();
 require(parsed_ids.size()>=3 && parsed_ids[0]==1 && parsed_ids[1]==2 && parsed_ids[2]==3,
         "conquest parser reloads ConsoleGlobal faction after each callback");
 reset(c); ControlTurnPhase=1; c.selected_vehicle_=0; ConsoleGlobal.cursor_active_=1;
 VehCurrentCount=2; Vehs[0].x=10; Vehs[0].y=20; Vehs[1].x=51; Vehs[1].y=61;
 mutate_selection_on_ready_call=2; mutate_on_map_cursor=true;
 c.human_turn();
 require(cursor_x==51 && cursor_y==61,"tile captured before map redraw callbacks");
 return 0;
}

'''

@pytest.mark.parametrize("damage", [None, "cursor_guard", "conquest_flag", "fx_order"])
def test_player_turn_native_guards_and_live_callback_state(tmp_path, damage):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = body_for("void Console::human_turn()")
    changes = {
        "cursor_guard": ("else if (ConsoleGlobal.cursor_active_ != 0)", "else if (ConsoleGlobal.cursor_active_ == 0)"),
        "conquest_flag": ("GameState &= ~2u;", "GameState &= ~3u;"),
        "fx_order": ("turn_hints();", "g_FX.effect(57).is_playing(); turn_hints();"),
    }
    if damage is not None:
        old, new = changes[damage]
        assert old in body
        body = body.replace(old, new, 1)
    unit = tmp_path / "human_turn.cpp"
    unit.write_text(HUMAN_TURN_DECLS + "\n" + body + "\n" + HUMAN_TURN_TESTS)
    executable = tmp_path / "human_turn"
    subprocess.run([compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr
    if damage is not None:
        assert "FAIL " in result.stderr, result.stdout + result.stderr
