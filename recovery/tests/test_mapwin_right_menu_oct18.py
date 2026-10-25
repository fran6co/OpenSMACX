"""Host controls for the actual MapWin right-menu owner and helpers.

The fixture is intentionally a typed service boundary, not a second menu
algorithm. It splices the live three helpers and complete right_menu body.
Unsafe original indices are kept in a valid bounded test domain only.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/mapwin.cpp"


def _definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        depth += (text[pos] == "{") - (text[pos] == "}")
        if depth == 0:
            return text[start:pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _actual_sources(source_text=None):
    text = SOURCE.read_text(encoding="utf-8") if source_text is None else source_text
    indent_signature = "void mapwin_add_indented_label("
    indent_name = "mapwin_add_indented_label"
    helpers = [
        _definition(text, "void mapwin_add_context_label("),
        _definition(text, indent_signature),
        _definition(text, "void mapwin_add_base_production("),
    ]
    right_menu = _definition(
        text, "void MapWin::right_menu(int tile_x, int tile_y, int screen_x, int screen_y)")
    return helpers, right_menu, indent_name


FIXTURE = r'''#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static std::vector<std::string> trace;
static std::vector<int> LastMenuIds;
static std::vector<std::string> LastMenuText;
static void event(const std::string &s) { trace.push_back(s); }
static void (*PullDownShowHook)();
static void (*SetMoveHook)();
static void require(bool ok, const char *label) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}
#define CHECK(x, label) require((x), (label))
typedef char *LPSTR;
static const int STATE_OMNISCIENT_VIEW=0x80,
                 TRIAD_AIR=2, ABL_DROP_POD=0x10, ORDER_MOVE_TO=0x18,
                 MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS=0x400;

struct PullDown {
    struct Item { int id; std::string text; };
    std::vector<Item> items;
    static int selected_id;
    static bool force_index;
    PullDown() { event("pull+"); LastMenuIds.clear(); LastMenuText.clear(); }
    ~PullDown() {
        event("pull-"); LastMenuIds.clear(); LastMenuText.clear();
        for (std::size_t i=0;i<items.size();++i) {
            LastMenuIds.push_back(items[i].id); LastMenuText.push_back(items[i].text);
        }
    }
    void init(int) { event("init"); }
    int add_item(int id, char *text) {
        items.push_back(Item{id, text ? text : ""});
        event("add:" + std::to_string(id)); return 0;
    }
    int add_separator(int id) {
        items.push_back(Item{id, "<separator>"});
        event("sep:" + std::to_string(id)); return 0;
    }
    int disable_item(int id) { event("disable:" + std::to_string(id)); return 0; }
    int show(int x, int y, int (*)()) {
        event("show:" + std::to_string(x) + "," + std::to_string(y));
        if (!force_index) {
            // Bind selection to the menu that the actual source constructed.
            // The no-action test service uses the native upper-bound path,
            // never a negative index into host C++ storage.
            selected_index=64;
            if (selected_id >= 0) {
                for (std::size_t i=0;i<items.size();++i) {
                    if (items[i].id==selected_id) { selected_index=static_cast<int>(i); break; }
                }
                CHECK(selected_index<64,"requested action must be offered");
            }
        }
        if (PullDownShowHook) PullDownShowHook();
        return 0;
    }
    static int selected_index;
    int get_selected() { event("selected"); return selected_index; }
    int item_id(int index) const {
        CHECK(index >= 0 && index < static_cast<int>(items.size()), "valid item index domain");
        return items[index].id;
    }
};
int PullDown::selected_id = -1;
int PullDown::selected_index = 0;
bool PullDown::force_index = false;
struct MenuEntry { PullDown *pull_down; };
struct Menu {
    MenuEntry entries_[15];
    int id_to_index(int id) { return id == 3 ? 0 : (id == 9 ? 1 : -1); }
};
static PullDown *Submenus[2];

struct Base { int queue_production_id[8]; int faction_id_current; };
static Base Bases[4];
struct FacilityRecord { int name; };
static FacilityRecord Facility[8];
struct Prototype { char *veh_name; int chassis_id; int weapon_id; int plan; };
typedef Prototype VehPrototype;
static Prototype VehPrototypes[16];
struct ChassisRecord { int triad; };
static ChassisRecord Chassis[8];
struct WeaponRecord { int offense_rating; };
static WeaponRecord Weapon[8];
struct Veh {
    int x, y, proto_id, order;
    unsigned state;
    int16_t waypoint_x[16], waypoint_y[16];
};
static Veh Vehs[32];
struct Player { unsigned flags; unsigned diplo_treaties[4]; };
static Player PlayersData[4];
struct RulesRecord { int artillery_max_rng; };
static RulesRecord Rules;
struct StringTableMock { char *get(int id); };
static StringTableMock StringTable;
static char facility_name[] = "FACILITY", vehicle_name[] = "VEHICLE";
char *StringTableMock::get(int id) { (void)id; return facility_name; }

static int BaseAtValue=-1, VehAtValue=-1, BaseAtCalls, VehAtCalls;
static int mutate_selected_on_third_veh_read;
static void (*VehAtHook)();
static int stack_result, stack_result_for_selected, stack_selected;
static int label_null_offset=-1;
static int label_calls[4096];
static int edit_lock_value, spying_value, find_landmark_value=-1;
static int stack_calls;
static LPSTR label_get(int index) {
    ++label_calls[index]; if (index == label_null_offset) return 0;
    static char labels[4096][24];
    std::snprintf(labels[index], sizeof(labels[index]), "L%d", index);
    return labels[index];
}
static int base_at(int, int) { ++BaseAtCalls; event("base"); return BaseAtValue; }
static int veh_at(int, int) {
    ++VehAtCalls; event("veh");
    if (mutate_selected_on_third_veh_read && VehAtCalls == 3 && VehAtHook) VehAtHook();
    return VehAtValue;
}
static void client_to_screen(int *, int *) { event("client"); }
static int stack_check(int vehicle, int mode, int, int, int) {
    ++stack_calls; event("stack:" + std::to_string(vehicle) + ":" + std::to_string(mode));
    return vehicle == stack_selected ? stack_result_for_selected : stack_result;
}
static int spying(int) { return spying_value; }
static int HasAbilResult, CanArtyResult;
static void (*CanArtyHook)();
static int has_abil(int, int) { return HasAbilResult; }
static int can_arty(int, int) { if (CanArtyHook) CanArtyHook(); return CanArtyResult; }
static int find_landmark(int, int, int) { return find_landmark_value; }
static int pop_expire() { return 0; }
static int X_pop(const char *, void *) { event("X_pop"); return 1; }
static void synch_veh(int id) { event("synch:" + std::to_string(id)); }
static void action_arty(int v, int x, int y) {
    event("action_arty:" + std::to_string(v) + ":" + std::to_string(x) + ":" + std::to_string(y));
}
static void action_patrol(int, int, int) { event("action_patrol"); }
static void help_any() { event("help"); }

static struct { struct { unsigned game_state_, game_more_preferences_; } fields_; }
    GameDataStateStorage;
static int IsMultiplayerNet, ConsoleControlTurnActive;
static unsigned MapIsFlat, MapLongitude=8, MapLongitudeBounds=32;
struct Channel { struct { unsigned flags; } payload_; };
struct NetState { unsigned leading_words_[4]; int session_phase_; unsigned trailing_words_[9]; };
struct NetDaemonMock {
    Channel channels_[2]; NetState primary_state_;
    int lock_result;
    int lock_veh(int *vehicle, int a, int b, int c, int d) {
        CHECK(b==-1 && c==-1, "lock original sentinel arguments");
        event("lock:" + std::to_string(*vehicle) + ":" + std::to_string(a) +
              ":" + std::to_string(d));
        if (lock_result == 0) *vehicle = lock_vehicle_result;
        return lock_result;
    }
    void unlock_veh() { event("unlock"); }
    void veh_drop(int v, int x, int y) {
        event("net_veh_drop:" + std::to_string(v) + ":" +
              std::to_string(x) + ":" + std::to_string(y));
    }
    int lock_vehicle_result;
};
static NetDaemonMock NetDaemonNet;
struct MenuButton { void set(int id, int state) { event("button:" + std::to_string(id) + ":" + std::to_string(state)); } };
struct MainInterfaceMock { MenuButton button_group(int) { return MenuButton(); } };
static MainInterfaceMock MainInterfaceGlobal;
struct WorldMock {
    void set_center(int, int) { event("world_center"); }
    void set_world_map() { event("world_map"); }
};
static WorldMock WorldClimateWorldWin;
struct ConsoleStatusMock { void set_loc(int,int) { event("status_loc"); } void redraw(){event("status_redraw");} };
struct ConsoleInfoMock { void set_state(int,int) { event("info_state"); } };
static ConsoleStatusMock ConsoleStatusWin;
static ConsoleInfoMock ConsoleInfoWin;
struct TutMock {
    void zoom(int base,int flag) { CHECK(base==1 && flag==0,"base zoom arguments"); event("zoom"); }
    void soft_zoom(int base) { CHECK(base==1,"base soft zoom argument"); event("soft_zoom"); }
    void hurry() { event("hurry"); }
    void gov_on(unsigned mask) { event("gov_on:"+std::to_string(mask)); }
    void production(int a, int b) { event("production:" + std::to_string(a) + ":" + std::to_string(b)); }
};
static TutMock TutWinBaseWindow;
struct ConsoleMock {
    int selected_vehicle_, cursor_active_, faction_id_, field_23BE4_, field_23BE8_;
    Menu alpha_menu_;
    ConsoleMock() : selected_vehicle_(-1), cursor_active_(0), faction_id_(0),
        field_23BE4_(0), field_23BE8_(0) {}
    int edit_lock() { event("edit_lock"); return edit_lock_value; }
    void get_mouse_pos(int *x, int *y) { event("mouse"); *x=31; *y=41; }
    void set_view(int) { event("set_view"); }
    void set_cursor(int,int) { event("set_cursor"); }
    void set_center(int,int,int) { event("set_center"); }
    void set_move(int) { event("set_move"); if (SetMoveHook) SetMoveHook(); }
    void activate(int,int,int) { event("activate"); }
    void name_landmark(int,int) { event("name_landmark"); }
    void erase_landmark(int,int) { event("erase_landmark"); }
    void go_to(int,int,int) { event("go_to"); }
    void go_home(int id) { event("go_home:"+std::to_string(id)); }
    void destroy(int id) { event("destroy:"+std::to_string(id)); }
    void disband(int id) { event("disband:"+std::to_string(id)); }
    void editor_edit_veh(int id) { event("edit_veh:"+std::to_string(id)); }
    void editor_kill_veh(int id) { event("kill_veh:"+std::to_string(id)); }
    void set_bombing_run(int,int,int) { event("bombing_run"); }
};
static ConsoleMock ConsoleGlobal;
static void mutate_base_on_show() { BaseAtValue=2; }
static void mutate_selected_on_show() { ConsoleGlobal.selected_vehicle_=11; }
static void mutate_selected_on_third_vehicle_read() { ConsoleGlobal.selected_vehicle_=9; }
static void mutate_selected_on_set_move() { ConsoleGlobal.selected_vehicle_=-1; }
static void move_selected_vehicle_far() { Vehs[3].x=25; Vehs[3].y=25; }
struct MapWin {
    int map_view_index_, zoom_offset_;
    void right_menu(int tile_x, int tile_y, int screen_x, int screen_y);
    void set_center(int,int,int) { event("map_center"); }
    void on_redraw() { event("map_redraw"); }
};
static MapWin *MapWinTable[2];
static char StringTemp[1032];

HELPERS

RIGHT_MENU

static void reset_fixture() {
    trace.clear(); BaseAtValue=-1; VehAtValue=-1; BaseAtCalls=VehAtCalls=0;
    mutate_selected_on_third_veh_read=0; stack_result=0; stack_result_for_selected=0;
    stack_selected=-1; label_null_offset=-1; std::memset(label_calls,0,sizeof(label_calls));
    edit_lock_value=spying_value=0; find_landmark_value=-1;
    GameDataStateStorage.fields_.game_state_=0;
    GameDataStateStorage.fields_.game_more_preferences_=0;
    IsMultiplayerNet=ConsoleControlTurnActive=0;
    ConsoleGlobal.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=0;
    ConsoleGlobal.faction_id_=0; ConsoleGlobal.field_23BE4_=ConsoleGlobal.field_23BE8_=0;
    PullDown::force_index=false; PullDown::selected_index=0; PullDown::selected_id=-1;
    PullDownShowHook=0; VehAtHook=0; SetMoveHook=0;
    CanArtyHook=0; HasAbilResult=CanArtyResult=0; Submenus[0]=Submenus[1]=0;
    MapWinTable[0]=0; MapWinTable[1]=0; BaseAtCalls=VehAtCalls=stack_calls=0;
    NetDaemonNet.channels_[1].payload_.flags=0;
    NetDaemonNet.primary_state_.leading_words_[2]=0;
    NetDaemonNet.primary_state_.trailing_words_[6]=0;
    NetDaemonNet.lock_result=0; NetDaemonNet.lock_vehicle_result=4;
    for(int i=0;i<32;++i) Vehs[i]=Veh{};
    for(int i=0;i<4;++i) { Bases[i]=Base{}; PlayersData[i]=Player{}; }
    for(int i=0;i<15;++i) ConsoleGlobal.alpha_menu_.entries_[i].pull_down=0;
    for(int i=0;i<16;++i) VehPrototypes[i]=Prototype{vehicle_name,0,0,0};
    for(int i=0;i<8;++i) { Chassis[i]=ChassisRecord{}; Weapon[i]=WeaponRecord{}; }
    Rules.artillery_max_rng=8;
}
static void simple_menu_case() {
    reset_fixture(); MapWin w={0,0}; ConsoleGlobal.cursor_active_=0;
    /* Valid no-base/no-vehicle domain; the service returns a no-action ID.
       A negative slot is deliberately NOT exercised as bounded host C++. */
    w.right_menu(2,3,10,20);
    CHECK(LastMenuIds==std::vector<int>({2,0x63,3,8}),"simple menu item order");
    CHECK(trace.front()=="pull+" && trace[1]=="client" && trace[2]=="base" &&
          trace[3]=="veh" && trace[4]=="init" && trace.back()=="pull-",
          "menu construction/lifetime trace");
}
static bool has_id(int id) {
    for (std::size_t i=0;i<LastMenuIds.size();++i) if (LastMenuIds[i]==id) return true;
    return false;
}
static void selected_air_eligibility_case() {
    reset_fixture(); MapWin w={0,0};
    ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{8,8,2,0,0,{0},{0}};
    VehPrototypes[2]=Prototype{vehicle_name,1,1,0}; Chassis[1].triad=TRIAD_AIR;
    BaseAtValue=-1; VehAtValue=7; stack_result=0; PullDown::selected_id=-1;
    w.right_menu(2,3,10,20);
    CHECK(has_id(0x6E),"selected air unit offers home independently of clicked stack");

    reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{8,8,2,0,0,{0},{0}};
    VehPrototypes[2]=Prototype{vehicle_name,1,1,0}; Chassis[1].triad=0;
    BaseAtValue=-1; VehAtValue=7; stack_result=1; stack_selected=7;
    w.right_menu(2,3,10,20);
    CHECK(!has_id(0x6E),"clicked stack cannot substitute for selected air unit");
}
static void advanced_preference_gate_case() {
    MapWin w={0,0};
    const unsigned prefs[]={0,4,0x400};
    for (int i=0;i<3;++i) {
        reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
        Vehs[3]=Veh{8,8,2,0,0,{0},{0}};
        VehPrototypes[2]=Prototype{vehicle_name,1,1,0};
        Chassis[1].triad=TRIAD_AIR; Weapon[1].offense_rating=1;
        GameDataStateStorage.fields_.game_more_preferences_=prefs[i];
        w.right_menu(2,3,10,20);
        CHECK(has_id(0x69)==(i==2),"advanced preference gates patrol item");
        CHECK(has_id(0x6A)==(i==2),"advanced preference gates air offense item");
    }
}
static void shared_label_and_action_case() {
    reset_fixture(); MapWin w={0,0};
    BaseAtValue=1; VehAtValue=-1; Bases[1].faction_id_current=0;
    Bases[1].queue_production_id[0]=-2; Facility[2].name=3;
    PullDown::selected_id=0x33;
    w.right_menu(2,3,10,20);
    if (!has_id(0x33) || !has_id(0x34) || !has_id(0x38)) {
        std::fprintf(stderr,"captured ids:");
        for(std::size_t i=0;i<LastMenuIds.size();++i) std::fprintf(stderr," %x",LastMenuIds[i]);
        std::fprintf(stderr,"\n");
    }
    CHECK(has_id(0x33) && has_id(0x34) && has_id(0x38),"owned base menu item range");
    CHECK(LastMenuIds==std::vector<int>({2,0x63,0x33,0x34,0x35,0x36,0x37,0x38,
                                         0x63,3,8}),
          "owned-base item and separator order");
    bool label33=false,label34=false;
    for(std::size_t i=0;i<LastMenuIds.size();++i) {
        if(LastMenuIds[i]==0x33) label33=LastMenuText[i]==" L550";
        if(LastMenuIds[i]==0x34) label34=LastMenuText[i]==" L551 FACILITY";
    }
    CHECK(label33 && label34,"menu captures shared scratch labels and production concatenation");
    CHECK(trace.size()>=3 && trace[trace.size()-3]=="soft_zoom" &&
          trace[trace.size()-2]=="production:0:0" && trace.back()=="pull-",
          "item 0x33 action ordering and arguments");
}
static void revalidation_and_live_callback_cases() {
    reset_fixture(); MapWin w={0,0};
    BaseAtValue=1; VehAtValue=-1; Bases[1].faction_id_current=0;
    PullDown::selected_id=0x33; PullDownShowHook=mutate_base_on_show;
    w.right_menu(2,3,10,20);
    CHECK(std::find(trace.begin(),trace.end(),"soft_zoom")==trace.end(),
          "base identity is revalidated after modal callback");

    reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3].proto_id=2; VehPrototypes[2].chassis_id=1; Chassis[1].triad=TRIAD_AIR;
    PullDown::selected_id=0x6E;
    PullDownShowHook=mutate_selected_on_show;
    w.right_menu(2,3,10,20);
    CHECK(std::find_if(trace.begin(),trace.end(),[](const std::string &s) {
              return s.find("go_home:")==0;
          })==trace.end(),"selected vehicle identity is revalidated after modal callback");

    reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{8,8,2,0,0,{0},{0}}; VehPrototypes[2]=Prototype{vehicle_name,1,1,0};
    Chassis[1].triad=TRIAD_AIR; Weapon[1].offense_rating=1;
    PullDown::selected_id=0x68; CanArtyResult=1; IsMultiplayerNet=1;
    mutate_selected_on_third_veh_read=1; VehAtHook=mutate_selected_on_third_vehicle_read;
    w.right_menu(2,3,10,20);
    CHECK(std::find(trace.begin(),trace.end(),"action_arty:9:2:3")!=trace.end(),
          "multiplayer refresh uses selected vehicle changed by live callback");

    reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{2,2,2,0,0,{0},{0}}; VehPrototypes[2]=Prototype{vehicle_name,1,1,0};
    Chassis[1].triad=TRIAD_AIR; Weapon[1].offense_rating=1;
    CanArtyResult=1; CanArtyHook=move_selected_vehicle_far;
    PullDown::selected_id=-1;
    w.right_menu(4,2,10,20);
    CHECK(!has_id(0x68) && !has_id(0x65),
          "artillery callback mutations precede artillery and close-range geometry");
}
static void item8_live_selection_case() {
    reset_fixture(); MapWin w={0,0};
    ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{8,8,2,0,0,{0},{0}}; VehPrototypes[2]=Prototype{vehicle_name,1,1,0};
    Chassis[1].triad=TRIAD_AIR; PullDown::selected_id=8;
    SetMoveHook=mutate_selected_on_set_move;
    w.right_menu(2,3,10,20);
    CHECK(std::find(trace.begin(),trace.end(),"set_move")!=trace.end(),"item 8 enters move mode");
    CHECK(std::find(trace.begin(),trace.end(),"X_pop")==trace.end(),
          "item 8 re-reads selected vehicle after set_move callback");
}
static void drop_pod_dispatch_case() {
    reset_fixture(); MapWin w={0,0};
    ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3]=Veh{8,8,2,0,0,{0},{0}}; VehPrototypes[2]=Prototype{vehicle_name,1,1,0};
    Chassis[1].triad=TRIAD_AIR; HasAbilResult=1; PullDown::selected_id=0x67;
    w.right_menu(5,6,10,20);
    CHECK(std::find(trace.begin(),trace.end(),"net_veh_drop:3:5:6")!=trace.end(),
          "item 0x67 dispatches selected vehicle and clicked coordinates");
}
static void helper_capture_cases() {
    reset_fixture(); PullDown p; mapwin_add_context_label(p, 0xC04, 0x320);
    CHECK(p.items.size()==1 && p.items[0].id==0x320 && p.items[0].text=="L769","context label capture");
    INDENT_HELPER(p, 0x898, 0x33);
    CHECK(p.items.size()==2 && p.items[1].text==" L550","shared indented label captured");
    std::strcpy(StringTemp,"mutated after add");
    CHECK(p.items[1].text==" L550","PullDown captures shared label before scratch reuse");
    INDENT_HELPER(p,0x8A0,0x35);
    CHECK(p.items.size()==3 && p.items[2].text==" L552","second indented label capture");
    Base b={}; b.queue_production_id[0]=-2; Facility[2].name=3;
    mapwin_add_base_production(p,b,0x34);
    CHECK(p.items.size()==4 && p.items[3].text==" L551 FACILITY","production label uses shared buffer and label 0x89C");
    b.queue_production_id[0]=2;
    mapwin_add_base_production(p,b,0x34);
    CHECK(p.items.back().text==" L551 VEHICLE","prototype production label and name");
}
static void distance_and_plan_cases() {
    MapWin w={0,0};
    for(int flat=0;flat<2;++flat) {
        reset_fixture(); MapIsFlat=flat; MapLongitude=16; MapLongitudeBounds=32;
        ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
        Vehs[3].x=31; Vehs[3].y=2; Vehs[3].proto_id=2;
        VehPrototypes[2].plan=9; CanArtyResult=1; Rules.artillery_max_rng=2;
        w.right_menu(1,2,10,20);
        CHECK(has_id(0x68)==!flat && has_id(0x65)==!flat && has_id(0x66)==!flat,
              "cylindrical wrap and plan-nine close-range entries");
    }
    reset_fixture(); ConsoleGlobal.cursor_active_=1; ConsoleGlobal.selected_vehicle_=3;
    Vehs[3].x=2; Vehs[3].y=2; Vehs[3].proto_id=2;
    stack_selected=3; stack_result_for_selected=9; VehPrototypes[2].plan=0;
    w.right_menu(2,2,10,20);
    CHECK(has_id(0x65) && !has_id(0x66),"stack count nine is not prototype plan nine");
    CHECK(has_id(4) && !has_id(3),"active cursor offers recenter without edit-lock call");
    CHECK(std::find(trace.begin(),trace.end(),"edit_lock")==trace.end(),"active cursor bypasses edit lock");
}
static void quick_map_and_slot_cases() {
    MapWin w={1,0}, primary={0,0};
    for(int id : {0x320,0x321,0x322,0x323,0x324,0x325}) {
        reset_fixture(); MapWinTable[0]=&primary; PullDown::selected_id=id; w.zoom_offset_=-6;
        w.right_menu(2,3,10,20);
        CHECK(LastMenuIds==std::vector<int>({0x320,0x63,0x323,0x324,0x325,0x63,0x321,0x322}),
              "quick-map indexed item order");
        if(id==0x321 || id==0x322 || id==0x325) {
            CHECK(w.zoom_offset_==(id==0x325?-4:-8),"signed quick-map zoom action");
            CHECK(std::count(trace.begin(),trace.end(),"map_redraw")==1,"changed zoom redraws once");
        }
        if(id==0x320) CHECK(std::find(trace.begin(),trace.end(),"world_map")!=trace.end(),"quick world action");
        if(id==0x323 || id==0x324) CHECK(std::count(trace.begin(),trace.end(),"map_center")==1,"quick center action");
    }
    for(int view : {0,1}) {
        reset_fixture(); w.map_view_index_=view; PullDown::selected_id=2;
        PullDown::force_index=true; PullDown::selected_index=64;
        w.right_menu(2,3,10,20);
        CHECK(std::find(trace.begin(),trace.end(),"set_view")==trace.end(),"slot upper bound suppresses action");
    }
}
static bool action_event(const std::string &s) {
    const char *prefixes[]={"lock:","set_move","X_pop","name_landmark","erase_landmark",
        "soft_zoom","production:","hurry","gov_on:","go_to","net_veh_drop:",
        "action_arty:","action_patrol","bombing_run","go_home:","destroy:",
        "disband:","edit_veh:","kill_veh:"};
    for(const char *prefix:prefixes) if(s.find(prefix)==0) return true;
    return false;
}
static void multiplayer_action_gates_and_lock_case() {
    MapWin w={0,0};
    for(int mask : {0,1}) for(int id : {1,3,8,9,0xB,0x33,0x34,0x35,0x36,
                                      0x37,0x38,0x39,0x67,0x68,0x69,0x6A,
                                      0x6E,0xC7,0xC8,0x29A,0x29B}) {
        reset_fixture(); BaseAtValue=1; VehAtValue=7; ConsoleGlobal.selected_vehicle_=3;
        ConsoleGlobal.cursor_active_=id==3?0:1;
        Vehs[3].x=1; Vehs[3].y=3; Vehs[3].proto_id=2;
        VehPrototypes[2].chassis_id=1; VehPrototypes[2].weapon_id=1;
        VehPrototypes[2].plan=9; Chassis[1].triad=TRIAD_AIR; Weapon[1].offense_rating=1;
        HasAbilResult=CanArtyResult=1; find_landmark_value=0; stack_result=1;
        GameDataStateStorage.fields_.game_more_preferences_=MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS;
        GameDataStateStorage.fields_.game_state_=0x4000;
        IsMultiplayerNet=1; PullDown::selected_id=id;
        if(mask) NetDaemonNet.primary_state_.trailing_words_[6]=1;
        else NetDaemonNet.channels_[1].payload_.flags=1;
        w.right_menu(2,3,10,20);
        CHECK(std::find_if(trace.begin(),trace.end(),action_event)==trace.end(),
              "every modifying context action honors multiplayer gates");
    }
    reset_fixture(); ConsoleGlobal.cursor_active_=1;
    ConsoleGlobal.selected_vehicle_=3; PullDown::selected_id=1;
    Vehs[4].state=0x12350000U;
    w.right_menu(2,3,10,20);
    CHECK(Vehs[4].order==ORDER_MOVE_TO && Vehs[4].waypoint_x[0]==2 &&
          Vehs[4].waypoint_y[0]==3 && Vehs[4].state==0x12340000U,
          "lock-updated vehicle receives original move order and state");
    CHECK(trace[trace.size()-4]=="lock:3:0:0" && trace[trace.size()-3]=="synch:4" &&
          trace[trace.size()-2]=="unlock" && trace.back()=="pull-",
          "vehicle lock write sync unlock ordering");
}
int main() {
    helper_capture_cases(); simple_menu_case(); selected_air_eligibility_case();
    advanced_preference_gate_case(); shared_label_and_action_case();
    revalidation_and_live_callback_cases(); item8_live_selection_case();
    drop_pod_dispatch_case();
    distance_and_plan_cases(); quick_map_and_slot_cases();
    multiplayer_action_gates_and_lock_case();
}
'''


def _fixture_sources(source_text=None):
    helpers, menu, indent_name = _actual_sources(source_text)
    return (FIXTURE.replace("HELPERS", "\n".join(helpers)).replace("RIGHT_MENU", menu)
            .replace("INDENT_HELPER", indent_name))


def _compile_run(tmp_path, compiler, source, name):
    cpp=tmp_path / (name + ".cpp"); exe=tmp_path / name
    cpp.write_text(source,encoding="utf-8")
    build=subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror",
                          str(cpp),"-o",str(exe)],text=True,capture_output=True,timeout=30)
    assert build.returncode==0, f"{name} must compile before behavior assertions:\n{build.stderr}"
    return subprocess.run([str(exe)],text=True,capture_output=True,timeout=30)


@pytest.mark.parametrize("compiler_name",["g++","clang++"])
def test_actual_right_menu_and_helpers_fixture(compiler_name,tmp_path):
    compiler=shutil.which(compiler_name)
    if not compiler: pytest.skip(f"{compiler_name} unavailable")
    result=_compile_run(tmp_path,compiler,_fixture_sources(),"right_menu_actual")
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("compiler_name",["g++","clang++"])
@pytest.mark.parametrize(("name","old","new","witness"),[
    ("clicked-unit-air-test", "Vehs[selected_vehicle].proto_id",
     "Vehs[vehicle_id].proto_id", "selected air unit offers home"),
    ("item8-cached-selection", "ConsoleGlobal.selected_vehicle_ >= 0) {\n                if (X_pop",
     "selected_vehicle >= 0) {\n                if (X_pop",
     "item 8 re-reads selected vehicle after set_move callback"),
    ("stale-selected-revalidation", "ConsoleGlobal.selected_vehicle_ != selected_vehicle",
     "false", "selected vehicle identity is revalidated"),
    ("wrong-production-arguments", "TutWinBaseWindow.production(0, 0);",
     "TutWinBaseWindow.production(1, 0);", "item 0x33 action ordering"),
    ("flat-wrap-polarity", "(MapIsFlat & 1U) == 0", "(MapIsFlat & 1U) != 0",
     "cylindrical wrap and plan-nine close-range entries"),
    ("wrong-plan-field", "VehPrototypes[vehicle.proto_id].plan == 9",
     "VehPrototypes[vehicle.proto_id].weapon_id == 9",
     "cylindrical wrap and plan-nine close-range entries"),
    ("missing-erase-gate", "case 0xB:\n        if (edi || esi) return;",
     "case 0xB:", "every modifying context action honors multiplayer gates"),
])
def test_compiling_live_source_mutants_fail_behavior(compiler_name,tmp_path,name,old,new,witness):
    compiler=shutil.which(compiler_name)
    if not compiler: pytest.skip(f"{compiler_name} unavailable")
    original=SOURCE.read_text(encoding="utf-8")
    assert old in original, f"mutation anchor disappeared: {name}"
    if name=="stale-selected-revalidation":
        # The second occurrence belongs to the full context menu; the first is
        # the quick-map route and is outside this mutation's witness.
        head, sep, tail=original.rpartition(old)
        assert sep
        damaged=head+new+tail
    else:
        damaged=original.replace(old,new,1)
    result=_compile_run(tmp_path,compiler,_fixture_sources(damaged),name)
    assert result.returncode != 0 and f"FAIL: {witness}" in result.stderr, (
        f"{name} must compile and fail its named behavior witness; "
        f"rc={result.returncode}, stderr={result.stderr!r}")
