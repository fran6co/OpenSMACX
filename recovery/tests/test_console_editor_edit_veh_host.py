"""Compile the product editor body itself against controlled owner doubles.

This checks branch effects, live prototype reloads and lexical Popup cleanup;
it is a host behavior fixture, not proof of the game UI or native ABI.
"""
from pathlib import Path
from match_recovery.source import read_file
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError("unterminated actual-source definition")


def actual_source():
    record, = [r for r in read_file(SOURCE) if r.address == 0x4DDA50]
    assert record.name == "?editor_edit_veh@Console@@QAEXH@Z"
    text = SOURCE.read_text()
    helpers = "\n".join(braced(text, signature) for signature in (
        "void append_decimal(", "void update_then_redraw_vehicle_editor_tile("))
    body = braced(text, "void Console::editor_edit_veh(int vehicle_id)")
    return helpers + body


HARNESS = r'''
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#define __cdecl
using std::string;
using std::vector;
typedef int BOOL;
typedef char *LPSTR;
static void require(bool ok, int line) { if (!ok) { std::fprintf(stderr,"assertion %d\n",line); std::exit(92); } }
#define CHECK(x) require((x), __LINE__)

struct Char256 { char str[256]; };
struct Veh { int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id; uint16_t unk_1; uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order; uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4]; uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended; int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id; int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; uint8_t obsolete_factions; int8_t combat_factions,icon_offset,padding; uint16_t flags; int16_t preq_tech; };
struct RulesWeapon { LPSTR name,name_short; int8_t offense_rating,icon; uint8_t mode,cost; int16_t preq_tech,padding; };
struct RulesChassis { uint8_t range; };
struct RulesMorale { LPSTR name,name_lifecycle; };
struct Player { int pad[185]; char noun_faction[24]; int noun_gender; BOOL is_noun_plural; };
struct Base { int16_t x,y; uint8_t faction_id_current; char rest[0x131]; char name_string[25]; };
static_assert(sizeof(Veh)==0x34,"fixture Veh size");
static_assert(sizeof(VehPrototype)==0x34,"fixture prototype size");

enum { VFLAG_IS_OBJECTIVE=0x20,VFLAG_LURKER=0x40,VFLAG_INVISIBLE=0x400,VSTATE_MONOLITH_UPGRADED=0x100 };
static const int MaxMoraleNum=7,MaxPlayerNum=8;
Veh Vehs[20]; VehPrototype VehPrototypes[20]; RulesWeapon Weapon[20]; RulesChassis Chassis[20]; RulesMorale Morale[7]; Player Players[8]; Base Bases[16];
int BaseCurrentCount,BaseAtKeyPollFlag,PopupWriteCheckState,GenderDefault,PluralityDefault;
Char256 ParseStrBuffer[10]; char StringTemp[1032];
char PopupStartCaption[256];
struct Strings { char *get(int) { static char value[]="STR"; return value; } } StringTable;
char *label_get(int) { static char value[]="NONE"; return value; }

vector<string> effects; int xpop_result=0,exec_result=0,vehicle_at_result=0,pick_result=0;
vector<vector<int>> tile_draws; int update_mutates_tile=0,updated_x=0,updated_y=0;
int owner_x=777,owner_y=777,owner_faction=777,selected_popup_value=-999;
int morale_value=0,alien_value=0,parse_mutate_proto=-1,exec_mutate_proto=-1,mutate_veh=0;
int parse_three_calls=0;
const char *last_popup_label="";
struct Dialogs { int item(char *,int index) { effects.push_back("item:"+std::to_string(index)); return 0; } };
struct Popup {
    Dialogs dialogs_;
    Popup() { effects.push_back("ctor"); }
    void close() {
        effects.push_back("popup.close");
        effects.push_back("scroll.close"); effects.push_back("basepop.close");
    }
    ~Popup() {
        // Mirrors Popup::~Popup -> one Popup::close plus Scroll/BasePop
        // destructors, which each perform their own close.
        close();
        effects.push_back("scroll.close"); effects.push_back("basepop.close");
    }
    int start(char *,const char *label,int,char *,int,void *) { last_popup_label=label; return 1; }
    int exec(int,void *) {
        effects.push_back("exec");
        if(std::strncmp(last_popup_label,"EDITVEH2",8)==0) std::strcpy(ParseStrBuffer[3].str,"5");
        if(exec_mutate_proto>=0) Vehs[mutate_veh].proto_id=static_cast<int16_t>(exec_mutate_proto);
        return exec_result;
    }
    void set_value(int value) { selected_popup_value=value; }
};
int _itoa(int n,char *out,int radix) { std::sprintf(out,radix==10?"%d":"%x",n); return 0; }
void auto_undo() { effects.push_back("undo"); }
int vehicle_at_x,vehicle_at_y;
int veh_at(int x,int y) { vehicle_at_x=x;vehicle_at_y=y;return vehicle_at_result; }
int veh_pick(int,const char *,int) { return pick_result; }
int x_pop_short(const char *,int,int(*)()) { effects.push_back("xpop"); return xpop_result; }
void owner_set(int x,int y,int f) { owner_x=x; owner_y=y; owner_faction=f; effects.push_back("owner"); }
void draw_tile(int x,int y,int type) { tile_draws.push_back({x,y,type}); effects.push_back("draw"); }
int morale_veh(int,int,int) { effects.push_back("morale"); return morale_value; }
uint32_t morale_alien(int,int) { return static_cast<uint32_t>(alien_value); }
int parse_says(int id,const char *input,int,int) {
    if(id==3) ++parse_three_calls;
    std::strncpy(ParseStrBuffer[id].str,input,sizeof(ParseStrBuffer[id].str)-1);
    if(id==3 && parse_mutate_proto>=0) Vehs[mutate_veh].proto_id=static_cast<int16_t>(parse_mutate_proto);
    return 0;
}
struct Console { int cursor_active_=0,selected_vehicle_=-1,cursor_history_index_=0,cursor_history_x_[4]={0},cursor_history_y_[4]={0}; void update_data(int){effects.push_back("update");if(update_mutates_tile){Vehs[0].x=updated_x;Vehs[0].y=updated_y;}} void editor_edit_veh(int); };
Console ConsoleGlobal;
struct ConsoleRef { void update_data(int){effects.push_back("update");} };
ConsoleRef ConsoleObject;

__ACTUAL_BODY_SLOT__

void reset(){
    parse_three_calls=0;
    ConsoleGlobal=Console();vehicle_at_x=vehicle_at_y=-999;
    std::memset(Vehs,0,sizeof(Vehs)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Weapon,0,sizeof(Weapon)); std::memset(Players,0,sizeof(Players)); std::memset(Bases,0,sizeof(Bases));
    std::memset(Chassis,0,sizeof(Chassis));
    for(int i=0;i<20;i++){ Vehs[i].proto_id=0; Vehs[i].faction_id=1; Vehs[i].x=5; Vehs[i].y=7; VehPrototypes[i].veh_name[0]='V'; VehPrototypes[i].veh_name[1]=0; }
    for(int i=0;i<7;i++){Morale[i].name=Morale[i].name_lifecycle=(LPSTR)"M";}
    for(int i=0;i<8;i++){Players[i].noun_faction[0]='F';Players[i].noun_faction[1]=0;}
    Vehs[0].home_base_id=-1; BaseCurrentCount=0; BaseAtKeyPollFlag=0; PopupWriteCheckState=0;
    GenderDefault=PluralityDefault=0; xpop_result=exec_result=0; vehicle_at_result=0; pick_result=0;
    owner_x=owner_y=owner_faction=777; selected_popup_value=-999; morale_value=alien_value=0;
    parse_mutate_proto=exec_mutate_proto=-1; mutate_veh=0; last_popup_label=""; effects.clear();
    tile_draws.clear(); update_mutates_tile=0; updated_x=updated_y=0;
    for(int i=0;i<10;i++) ParseStrBuffer[i].str[0]=0;
    std::strcpy(ParseStrBuffer[3].str,"5");
}
void lifecycle(){
    int p=0,s=0,b=0;for(const auto &x:effects){p+=x=="popup.close";s+=x=="scroll.close";b+=x=="basepop.close";}
    CHECK(p==1 && s==2 && b==2);
}
void test_returns(){
    reset();Console c;c.editor_edit_veh(-1);lifecycle(); // no vehicle at cursor
    reset();vehicle_at_result=1;pick_result=-1;Console c2;c2.editor_edit_veh(-1);lifecycle();
    reset();xpop_result=-1;Console c3;c3.editor_edit_veh(0);lifecycle();
    for(int mode=1;mode<=4;++mode){reset();xpop_result=0;BaseAtKeyPollFlag=mode;exec_result=-1;Console c;c.editor_edit_veh(0);lifecycle();}
    reset();xpop_result=0;Console c4;c4.editor_edit_veh(0);lifecycle();
    for(int mode=1;mode<=4;++mode){reset();xpop_result=0;BaseAtKeyPollFlag=mode;exec_result=mode==1?3:mode==3?2:mode==4?9999:0;Console c;c.editor_edit_veh(0);lifecycle();}
}
void test_mode2_live_range(){
    // Native prototype+0x24 selects Chassis; Chassis+0x4A is unsigned range.
    // Deliberately disagree with Weapon to catch the wrong table owner.
    reset();xpop_result=0;BaseAtKeyPollFlag=2;Chassis[0].range=255;Weapon[0].offense_rating=0;
    exec_result=0;Console c;c.editor_edit_veh(0);
    CHECK(std::strcmp(last_popup_label,"EDITVEH2A")==0);
    CHECK(parse_three_calls==1);
    CHECK(Vehs[0].terraforming_turns==5);lifecycle();

    // parse_says(3) can change the active prototype; the dialog choice must
    // reread the currently selected prototype after the parser call.
    reset();xpop_result=0;BaseAtKeyPollFlag=2;Chassis[0].range=2;Chassis[1].range=0;VehPrototypes[1].chassis_id=1;
    parse_mutate_proto=1;Console c2;c2.editor_edit_veh(0);
    CHECK(std::strcmp(last_popup_label,"EDITVEH2")==0);lifecycle();

    // Popup event processing can change it again; the final write gate must
    // reload after exec rather than use the pre-popup cached range.
    reset();xpop_result=0;BaseAtKeyPollFlag=2;Chassis[0].range=0;Chassis[1].range=2;VehPrototypes[1].chassis_id=1;
    exec_mutate_proto=1;Console c3;c3.editor_edit_veh(0);
    CHECK(Vehs[0].terraforming_turns==5);lifecycle();
}
void test_global_cursor_owner(){
    reset();BaseAtKeyPollFlag=3;exec_result=5;
    ConsoleGlobal.cursor_active_=1;ConsoleGlobal.selected_vehicle_=3;
    Console c;c.cursor_active_=1;c.selected_vehicle_=1;c.editor_edit_veh(-1);
    CHECK(Vehs[3].faction_id==5 && Vehs[1].faction_id==1);lifecycle();
    reset();ConsoleGlobal.cursor_history_index_=2;
    ConsoleGlobal.cursor_history_x_[2]=11;ConsoleGlobal.cursor_history_y_[2]=13;
    vehicle_at_result=-1;Console c2;c2.cursor_history_x_[0]=4;c2.cursor_history_y_[0]=6;
    c2.editor_edit_veh(-1);CHECK(vehicle_at_x==11 && vehicle_at_y==13);lifecycle();
}
void test_defaults_and_order(){
    reset();xpop_result=0;BaseAtKeyPollFlag=1;morale_value=2;exec_result=4;Console c;c.editor_edit_veh(0);
    CHECK(selected_popup_value==2);CHECK(Vehs[0].morale==2);lifecycle();
    reset();xpop_result=0;BaseAtKeyPollFlag=3;Vehs[0].faction_id=4;exec_result=5;Console c2;c2.editor_edit_veh(0);
    CHECK(selected_popup_value==4 && Vehs[0].faction_id==5);lifecycle();
    reset();xpop_result=0;BaseAtKeyPollFlag=4;Vehs[0].faction_id=3;Vehs[0].home_base_id=6;exec_result=9999;Console c3;c3.editor_edit_veh(0);
    CHECK(selected_popup_value==6 && Vehs[0].home_base_id==-1);lifecycle();
}
void test_flags_and_owner(){
    reset();xpop_result=0;Vehs[0].flags=0x7fe0;PopupWriteCheckState=0x55;Vehs[0].state=0x100;
    Console c;c.editor_edit_veh(0);
    CHECK(owner_x==5 && owner_y==7 && owner_faction==-1);
    CHECK((Vehs[0].flags&0x400)!=0 && (Vehs[0].state&0x100)!=0);lifecycle();
}
void test_draw_tile_native_argument_order_and_liveness(){
    reset();xpop_result=0;update_mutates_tile=1;updated_x=12;updated_y=14;
    Console c;c.editor_edit_veh(0);
    CHECK(tile_draws.size()==1 && tile_draws[0][0]==12 && tile_draws[0][1]==14 && tile_draws[0][2]==1);
    reset();xpop_result=0;BaseAtKeyPollFlag=3;exec_result=5;
    Console c2;c2.editor_edit_veh(0);
    CHECK(tile_draws.size()==2 && tile_draws[0][0]==5 && tile_draws[0][1]==7 && tile_draws[0][2]==1 && tile_draws[1][0]==5 && tile_draws[1][1]==7 && tile_draws[1][2]==1);
    reset();xpop_result=0;BaseAtKeyPollFlag=4;exec_result=9999;
    Console c3;c3.editor_edit_veh(0);
    CHECK(tile_draws.size()==2 && tile_draws[0][0]==5 && tile_draws[0][1]==7 && tile_draws[0][2]==1 && tile_draws[1][0]==5 && tile_draws[1][1]==7 && tile_draws[1][2]==1);
}
int main(){test_returns();test_mode2_live_range();test_global_cursor_owner();test_defaults_and_order();test_flags_and_owner();test_draw_tile_native_argument_order_and_liveness();}
'''


@pytest.mark.parametrize("damage", [None, "extra_close", "draw_order", "wrong_faction", "signed_range", "wrong_range_owner", "wrong_cursor_receiver"])
def test_actual_editor_body_and_popup_lifecycle(tmp_path, damage):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "editor.cpp"
    binary = tmp_path / "editor"
    actual = actual_source()
    if damage == "extra_close":
        actual = actual.replace("Popup popup;", "Popup popup; popup.close();", 1)
    elif damage == "draw_order":
        actual = actual.replace("draw_tile(vehicle.x, vehicle.y, 1)",
                                "draw_tile(1, vehicle.x, vehicle.y)")
    elif damage == "wrong_faction":
        actual = actual.replace("vehicle.faction_id", "vehicle.year_end_lurking")
    elif damage == "signed_range":
        actual = actual.replace(
            "const uint8_t input_fields_range =",
            "const int8_t input_fields_range =", 1)
    elif damage == "wrong_range_owner":
        actual = actual.replace("Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range",
                                "Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating")
    elif damage == "wrong_cursor_receiver":
        actual = actual.replace("ConsoleGlobal.cursor_", "this->cursor_").replace(
            "ConsoleGlobal.selected_vehicle_", "this->selected_vehicle_")
    if damage is not None:
        assert actual != actual_source(), damage
    source.write_text(HARNESS.replace("__ACTUAL_BODY_SLOT__", actual))
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert (result.returncode == 0) == (damage is None), result.stderr
