"""Actual-source dispatcher/reset controls with named service doubles, not game runtime proof."""
from pathlib import Path
import subprocess
import tempfile

def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unclosed source body")

def compile_run(program):
    with tempfile.TemporaryDirectory(prefix="console-menu-host-") as directory:
        binary = str(Path(directory) / "control")
        result = subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                                 "-x", "c++", "-", "-o", binary],
                                input=program, text=True, capture_output=True, timeout=30)
        assert result.returncode == 0, result.stderr
        ran = subprocess.run([binary], text=True, capture_output=True, timeout=30)
        assert ran.returncode == 0, ran.stdout + ran.stderr

def test_console_menu_dispatch():
    source = (Path(__file__).resolve().parents[2] / "src/console.cpp").read_text()
    methods = "\n".join(body(source, signature) for signature in [
        "void Console::reset_turn_view()", "void Console::menu_proc(int",
        "void __cdecl Console::menu_daemon(int", "void Console::editor_kill_vehicles()"])
    storage = []
    for declaration in (
        "char ConsoleVersionReleaseText[", "char ConsoleVersionBuildDateText[",
        "char *ConsoleVersionRelease =", "char *ConsoleVersionBuildDate =",
        "char ConsoleStrAcLogoSm[", "char ConsoleStrVersionControlX[",
        "typedef int (__cdecl *NullCallback)",
    ):
        matches = [line for line in source.splitlines() if line.startswith(declaration)]
        assert len(matches) == 1, "missing/ambiguous actual-source fixture input: " + declaration
        storage.append(matches[0])
    prefix = r"""#include <cstddef>
#include <cassert>
#include <string>
#include <vector>
std::vector<std::string> Trace;
int PopupResponse=0;
int EditLock=0;
int LastVehicle=-1, LastOrder=-1;
int TileX=-1, TileY=-1;
int count(const char *name){int n=0;for(std::size_t i=0;i<Trace.size();++i)if(Trace[i]==name)++n;return n;}
#include <cstdint>
typedef unsigned int uint32_t;
typedef int BOOL;
#define __cdecl

struct NetMsg { void pop(const char *, int, int, const char *) {} };
struct Sprite {};
struct PlayerData { int ai_growth, ai_tech, ai_wealth, ai_power; };
struct Veh { int proto_id; };
struct VehPrototype { int chassis_id, weapon_id; };
struct RulesChassis { int triad; };
struct RulesWeapon { int mode; };
struct LockStateEntry {
  struct Payload { uint32_t flags; } payload_;
  uint32_t leading_words_[4];
};
struct NetDaemon { LockStateEntry channels_[2]; struct Primary { uint32_t leading_words_[3], flags_, trailing_words_[10]; } primary_state_; void hang_up() {Trace.push_back("hang_up");} void reset() {Trace.push_back("daemon_reset");} };
struct AlphaMenu {
  void check(int) {Trace.push_back("menu_check");}
  void uncheck_menu_item(...) {}
  void check_menu_item(...) {}
};
struct GraphicWin { void redraw(){Trace.push_back("world_redraw");} };
struct MapWin : GraphicWin {
  Sprite *cursor_sprite_;
  uint32_t cursor_resource_id_, field_1DD74_, field_1DD70_;
  void set_cursor(...) {Trace.push_back("set_cursor");}
  void draw_map(...) {Trace.push_back("member_map");}
  void draw_base_dest(...) {Trace.push_back("base_dest");}
  void draw_cursor(...) {Trace.push_back("member_cursor");}
  void set_center(...) {}
  void focus(...) {}
  void main_caption(...) {}
  void editor(...) {}
  void redraw(...) {}
};
struct StatusWin { void reset() {} void redraw(...) {} void set_loc(...) {} };
struct InfoWin { void change(...) {} };
struct TutWin { int reset() {Trace.push_back("tutorial_reset");return 0;} void tour() {} void set_tour_stage(int) {} };
extern TutWin TutorialWindow;
struct FlatButton { void hide() {} void show(int) {} };
struct MainInterface { FlatButton flat_button_[42]; FlatButton &flat_button(int i) { return flat_button_[i]; } };
struct MultiWin {
  unsigned int panel_open_;
  MultiWin() : panel_open_(0) {}
  unsigned int panel_open() const { return panel_open_; }
  void show() { panel_open_=1; }
  void hide() { panel_open_=0; }
  void on_button_clicked(...) {}
};
struct Console : MapWin {
  void menu_proc(int);
  static void __cdecl menu_daemon(int);
  void editor_kill_vehicles();
  void automate(int vehicle,int order){LastVehicle=vehicle;LastOrder=order;Trace.push_back("automate");}
  void reset_turn_view();
  uint32_t field_23D10_, field_23BF8_, field_23BFC_, survey_mode_;
  int selected_vehicle_, cursor_history_index_;
  int faction_id_, cursor_active_;
  int cursor_history_x_[32], cursor_history_y_[32];
  AlphaMenu alpha_menu_;
#define M(name) void name(...) {Trace.push_back(#name);}
  M(terraform); M(set_move); M(zoom); M(set_tech); M(survey);
  M(corner_the_market); M(show_scen_objectives);  M(next_cursor);
  M(new_base); M(oblit);  M(arty); M(convoy); M(use_ability);
  M(go_to); M(destroy); M(disband); M(activate); M(hold); M(go_home); M(assemble);
  M(name_landmark); M(erase_landmark);
  M(explore); M(wait); M(unload); M(upgrade); M(give_veh); M(designate); M(sentry);
  M(skip); M(set_patrol); M(editor_on); M(editor_veh); M(editor_edit_veh);
  M(editor_tech); M(editor_view); M(editor_diff); M(editor_energy); M(editor_date);
  M(editor_eliminate); M(editor_reload);  M(show_movies);
  M(editor_reset_tech); M(editor_reset_faction); M(editor_diplomacy);
  M(editor_personality); M(editor_strategy); M(editor_rules); M(editor_scen_rules);
  M(editor_scen_param); M(editor_scen_victory); M(editor_undo); M(editor_redo);
  M(load_proc); M(chat); M(custom_my_faction); M(time_controls);
  M(zoom_to_upkeep_base); M(use_time_bonus); M(end_my_turn); M(road_to);
  M(set_preferences); M(set_base_preferences); M(set_adv_preferences);
  M(set_auto_preferences); M(set_audiovisual); M(set_map_display); M(retire_game);
  M(editor_load); M(editor_paints); M(editor_terrain); M(editor_natural);
  M(editor_minor); M(editor_set_brush); M(editor_climate); M(editor_generate);
  M(editor_fast); M(editor_fungus); M(editor_rockiness); M(editor_randomize);
  M(editor_clear_random); M(editor_flat); M(editor_clear); M(editor_polar);
  M(editor_clear_vehicles); M(editor_clear_terrain); M(editor_lock2); M(editor_save);
#undef M
  int edit_lock() {return EditLock;}
};

int ConsoleNoMapOverrideLatch;
int ConsoleControlTurnActive, IsMultiplayerNet, IsMultiplayerPBEM;
uint32_t MapBaseDisplayOverride, ExitTurnLoop, GameRestartQueued;
uint32_t GameRules, GameState, GamePreferences, GameMorePreferences;
uint32_t StatusFlags, LockEnableMask;
// Named-storage API double. References keep the control inputs/assertions
// on the same words read and written by the extracted current owner bodies.
struct HostLockMasks { uint32_t &enable_mask_; };
struct HostGameFields {
  uint32_t &game_rules_, &game_state_, &game_preferences_, &game_more_preferences_;
  HostLockMasks lock_masks_;
};
struct HostGameStorage { HostGameFields fields_; } GameDataStateStorage = {
  {GameRules, GameState, GamePreferences, GameMorePreferences, {LockEnableMask}}
};
int PopupWriteCheckState;
char *ScriptPopupFile;
PlayerData PlayersData[8];
Veh Vehs[256];
VehPrototype VehPrototypes[256];
RulesChassis Chassis[16];
RulesWeapon Weapon[64];
NetDaemon NetDaemonNet;
AlphaMenu AlphaMenuGlobal;
MapWin *MapWinTable[8];
const std::size_t MapWinTableSlots=8;
MultiWin MultiWindow;
InfoWin ConsoleInfoWin;
StatusWin ConsoleStatusWin;
MainInterface MainInterfaceGlobal;
MapWin WorldClimateWorldWin;
Console ConsoleGlobal;
NetMsg NetMsgPrimary;
struct AlphaPref { uint32_t preferences, more_preferences; } AlphaIniPrefs;
int __cdecl pop_wait(){Trace.push_back("pop_wait");return 0;}
int bit_count(unsigned int n){int c=0;while(n){c+=n&1;n>>=1;}return c;}
int __cdecl x_pop_short(const char *label, int flags, int (__cdecl *callback)()){assert(std::string(label)=="TECHRANDOM" && flags==1 && callback==0);Trace.push_back("tech_popup");return PopupResponse;}
int __cdecl X_pop(const char *, int (__cdecl *)()){Trace.push_back("popup");return PopupResponse;}
int __cdecl desktop_init(int){Trace.push_back("desktop_init");return 0;}
int __cdecl tech_advance(int){Trace.push_back("tech_advance");return 0;}
int __cdecl save_game(int){Trace.push_back("save_game");return 0;}
void __cdecl auto_save(){Trace.push_back("auto_save");}
void __cdecl design_new_veh(int, int){Trace.push_back("design_new_veh");}
void __cdecl go_reset(){Trace.push_back("go_reset");}
void __cdecl ambience(int){Trace.push_back("ambience");}
void __cdecl prefs_save(int){Trace.push_back("prefs_save");}
void __cdecl synch_ai(int){Trace.push_back("synch_ai");}
void __cdecl social_select(...){Trace.push_back("social_select");}
void __cdecl report_labs(...){Trace.push_back("report_labs");} void __cdecl report_energy(...){Trace.push_back("report_energy");}
void __cdecl report_ops(...){Trace.push_back("report_ops");} void __cdecl report_projects(...){Trace.push_back("report_projects");}
void __cdecl report_sat(...){Trace.push_back("report_sat");} void __cdecl report_security(...){Trace.push_back("report_security");}
void __cdecl report_score(...){Trace.push_back("report_score");} void __cdecl monument(...){Trace.push_back("monument");}
void __cdecl hall_of_fame(...){Trace.push_back("hall_of_fame");} void __cdecl show_credits(...){Trace.push_back("show_credits");}
void __cdecl show_replay(...){Trace.push_back("show_replay");}
void __cdecl log_say_hex(...){Trace.push_back("log_say_hex");} void __cdecl log_say(...){Trace.push_back("log_say");}
void __cdecl help_topic(...){Trace.push_back("help_topic");} void __cdecl parse_says(...){Trace.push_back("parse_says");}
int __cdecl popp(const char *, const char *, int, const char *, int (__cdecl *)()){Trace.push_back("popp");return 0;}
void __cdecl draw_cursor(...){Trace.push_back("draw_cursor");} void __cdecl desktop_update(...){Trace.push_back("desktop_update");}
void __cdecl do_non_input(...){Trace.push_back("do_non_input");}
const int TRIAD_AIR=2;
void __cdecl ConsoleResetMenuDaemon(){Trace.push_back("ConsoleResetMenuDaemon");}


void draw_map(int mode){assert(mode==1);Trace.push_back("free_map");}
void auto_undo(){Trace.push_back("undo");}
int veh_at(int x,int y){TileX=x;TileY=y;Trace.push_back("veh_at");return 17;}
void stack_kill(int vehicle){assert(vehicle==17);Trace.push_back("stack_kill");}
"""
    tests = r"""
MapWin map;
  TutWin TutorialWindow;
NetMsg &message = NetMsgPrimary;
void reset_state(){
    ConsoleGlobal=Console();
    map=MapWin(); NetDaemonNet=NetDaemon();
    MapBaseDisplayOverride=1; ExitTurnLoop=0; ConsoleNoMapOverrideLatch=0;
    IsMultiplayerNet=0; IsMultiplayerPBEM=0; ConsoleControlTurnActive=0;
    GameState=0; GameRules=0; GamePreferences=0; GameMorePreferences=0;
    EditLock=0; MultiWindow=MultiWin(); PopupResponse=0; GameRestartQueued=0;
    LockEnableMask=0; PopupWriteCheckState=0; StatusFlags=0;
    map.cursor_resource_id_=0x7F00;
    for(int i=0;i!=8;++i) MapWinTable[i]=0;
    MapWinTable[0]=&map;
    ConsoleGlobal.selected_vehicle_=3;
    Trace.clear();
}
int main(){
    reset_state(); assert(MultiWindow.panel_open()==0);
    ConsoleGlobal.menu_proc(0x113); assert(MultiWindow.panel_open()==1);
    ConsoleGlobal.menu_proc(0x113); assert(MultiWindow.panel_open()==0);
    reset_state(); MultiWindow.show();
    ConsoleGlobal.menu_proc(0x113); assert(MultiWindow.panel_open()==0);
    reset_state(); ConsoleGlobal.menu_proc(0x751);
    assert(count("parse_says")==2 && count("popp")==1);
    reset_state(); MapBaseDisplayOverride=0;
    ConsoleGlobal.menu_proc(0x59);
    assert(ConsoleNoMapOverrideLatch==1 && Trace.empty());
    reset_state(); ExitTurnLoop=1;
    ConsoleGlobal.menu_proc(0x59); assert(Trace.empty());

    reset_state(); IsMultiplayerNet=1; ConsoleControlTurnActive=1;
    ConsoleGlobal.menu_proc(0x304); assert(count("automate")==0);
    reset_state(); ConsoleGlobal.cursor_active_=1; EditLock=1;
    ConsoleGlobal.menu_proc(0x304);
    assert(count("automate")==0 && count("free_map")==0);
    assert(count("draw_cursor")==1 && count("member_cursor")==0);

    reset_state(); GameMorePreferences=0;
    ConsoleGlobal.menu_proc(0x209);
    assert(GameMorePreferences==0x10000 && AlphaIniPrefs.more_preferences==0x10000);
    assert(count("free_map")==1 && count("member_map")==0 && count("prefs_save")==1);

    reset_state(); Vehs[3].proto_id=2; VehPrototypes[2].chassis_id=1;
    VehPrototypes[2].weapon_id=1; Chassis[1].triad=TRIAD_AIR; Weapon[1].mode=1;
    ConsoleGlobal.menu_proc(0x304); assert(LastVehicle==3 && LastOrder==12);
    reset_state(); Weapon[1].mode=0;
    ConsoleGlobal.menu_proc(0x304); assert(LastVehicle==3 && LastOrder==3);

    reset_state(); ConsoleGlobal.faction_id_=2; GameRules=0x200;
    PlayersData[2].ai_growth=2; PlayersData[2].ai_tech=0;
    PlayersData[2].ai_wealth=4; PlayersData[2].ai_power=0;
    ConsoleGlobal.menu_proc(0x106);
    assert(PopupWriteCheckState==5 && count("tech_popup")==1 && count("synch_ai")==1);
    assert(PlayersData[2].ai_growth==1 && PlayersData[2].ai_wealth==1);

    reset_state(); Console::menu_daemon(0x59); assert(count("save_game")==1);
    reset_state(); IsMultiplayerNet=1; PopupResponse=1;
    ConsoleGlobal.menu_proc(0x5D);
    assert(ExitTurnLoop==1 && IsMultiplayerNet==0);
    assert(count("hang_up")==1 && count("daemon_reset")==1 && count("log_say")==1);
    reset_state(); PopupResponse=0; ConsoleGlobal.menu_proc(0x5D);
    assert(ExitTurnLoop==0 && count("log_say")==0);

    reset_state(); GameState=0x40;
    ConsoleGlobal.menu_proc(0x601);
    assert((GameState&0x80)!=0 && count("free_map")==1 && count("member_map")==0);
    assert(count("world_redraw")==1 && count("desktop_update")==1);

    reset_state(); ConsoleGlobal.cursor_history_index_=2;
    ConsoleGlobal.cursor_history_x_[2]=31; ConsoleGlobal.cursor_history_y_[2]=47;
    ConsoleGlobal.editor_kill_vehicles();
    assert(TileX==31 && TileY==47);
    assert(Trace.size()==3 && Trace[0]=="undo" && Trace[1]=="veh_at" && Trace[2]=="stack_kill");
}
"""
    compile_run(prefix + "\n" + "\n".join(storage) + "\n" + methods + "\n" + tests)


def test_netdaemon_reset_state_and_close_order():
    source = (Path(__file__).resolve().parents[2] / "src/netdaemon.cpp").read_text()
    method = body(source, "void NetDaemon::reset()")
    program = r"""#include <cassert>
#include <string>
#include <vector>
std::vector<std::string> events;
#include <cstdint>
#include <cstring>
typedef unsigned int uint32_t;
struct Lock { bool cleared; Lock() : cleared(false) {} void clear() { cleared = true; events.push_back("clear"); } };
struct AlphaNet { bool closed; AlphaNet() : closed(false) {} void close() { closed = true; events.push_back("close"); } };
struct LockStateEntry {
    uint32_t leading_words_[4];
    union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_;
    uint32_t trailing_words_[10];
};
struct DeletionList {
    uint8_t records_[0x5A0];
    int current_owner_;
    uint32_t sequence_bits() const {
        uint32_t bits;
        std::memcpy(&bits, &current_owner_, sizeof bits);
        return bits;
    }
};
struct SavedProcessSlot { uint8_t bytes_[0x19C]; };
struct NetDaemon : AlphaNet {
    Lock locks_;
    DeletionList deletion_list_;
    LockStateEntry channels_[2];
    struct Primary { uint32_t leading_words_[3], flags_, trailing_words_[10]; } primary_state_;
    void *file_index_records_;
    uint32_t field_1BE4_;
    SavedProcessSlot saved_process_slots_[8];
    NetDaemon() : file_index_records_(reinterpret_cast<void *>(0x1234)), field_1BE4_(0xA5A5A5A5u) {
        std::memset(&deletion_list_, 0x6B, sizeof(deletion_list_));
        std::memset(channels_, 0x7F, sizeof(channels_));
        std::memset(&primary_state_, 0x7F, sizeof(primary_state_));
        std::memset(saved_process_slots_, 0x3C, sizeof(saved_process_slots_));
        deletion_list_.current_owner_ = 0x12345678;
    }
    void reset();
};
// SOURCE_BODY

int main() {
    NetDaemon daemon;
    uint8_t deletion_records_snapshot[sizeof(daemon.deletion_list_.records_)];
    std::memcpy(deletion_records_snapshot, daemon.deletion_list_.records_,
                sizeof deletion_records_snapshot);
    uint8_t saved_process_snapshot[sizeof(daemon.saved_process_slots_)];
    std::memcpy(saved_process_snapshot, daemon.saved_process_slots_, sizeof saved_process_snapshot);
    const uint32_t sequence_before = daemon.deletion_list_.sequence_bits();
    void *const file_index_before = daemon.file_index_records_;
    const uint32_t file_index_adjacent_before = daemon.field_1BE4_;
    daemon.reset();
    assert(daemon.deletion_list_.current_owner_ == 0x12345678);
    assert(daemon.deletion_list_.sequence_bits() == sequence_before);
    assert(std::memcmp(deletion_records_snapshot, daemon.deletion_list_.records_,
                       sizeof deletion_records_snapshot) == 0);
    assert(daemon.file_index_records_ == file_index_before);
    assert(daemon.field_1BE4_ == file_index_adjacent_before);
    assert(std::memcmp(saved_process_snapshot, daemon.saved_process_slots_,
                       sizeof saved_process_snapshot) == 0);
    assert(daemon.primary_state_.leading_words_[0] == 0);
    assert(daemon.primary_state_.leading_words_[1] == 0);
    assert(daemon.primary_state_.leading_words_[2] == 0);
    assert(daemon.primary_state_.flags_ == 0);
    for (int i = 0; i < 7; ++i) assert(daemon.primary_state_.trailing_words_[i] == 0);
    assert(daemon.primary_state_.trailing_words_[7] == 0);
    assert(daemon.primary_state_.trailing_words_[8] == 0);
    assert(daemon.primary_state_.trailing_words_[9] == 0x7F7F7F7Fu);
    assert(daemon.channels_[0].leading_words_[0] == 0);
    assert(daemon.channels_[0].leading_words_[1] == 0);
    assert(daemon.channels_[0].leading_words_[2] == 0x7F7F7F7Fu);
    assert(daemon.channels_[0].leading_words_[3] == 0x7F7F7F7Fu);
    assert(daemon.channels_[0].payload_.vehicle_selection_follow == nullptr);
    for (int i = 0; i < 8; ++i) assert(daemon.channels_[0].trailing_words_[i] == 0);
    assert(daemon.channels_[0].trailing_words_[8] == 0);
    assert(daemon.channels_[0].trailing_words_[9] == 0);
    assert(daemon.channels_[1].leading_words_[0] == 0);
    assert(daemon.channels_[1].leading_words_[1] == 0);
    assert(daemon.channels_[1].leading_words_[2] == 0);
    assert(daemon.channels_[1].leading_words_[3] == 0);
    assert(daemon.channels_[1].payload_.flags == 0);
    assert(daemon.channels_[1].trailing_words_[0] == 0x7F7F7F7Fu);
    assert(daemon.channels_[1].trailing_words_[1] == 0x7F7F7F7Fu);
    assert(daemon.channels_[1].trailing_words_[2] == 0);
    assert(daemon.channels_[1].trailing_words_[3] == 0);
    assert(daemon.channels_[1].trailing_words_[4] == 0);
    assert(daemon.channels_[1].trailing_words_[5] == 0);
    assert(daemon.channels_[1].trailing_words_[6] == 0xFF);
    assert(daemon.channels_[1].trailing_words_[7] == 0);
    assert(daemon.channels_[1].trailing_words_[8] == 0);
    assert(daemon.channels_[1].trailing_words_[9] == 0x7F7F7F7Fu);
    assert(daemon.locks_.cleared);
    assert(daemon.closed);
    assert(events.size()==2 && events[0]=="clear" && events[1]=="close");
}
"""
    compile_run(program.replace("// SOURCE_BODY", method))
