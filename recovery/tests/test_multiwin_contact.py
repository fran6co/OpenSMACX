"""Actual-source C++98 contact/diplomacy controls; not ABI or game proof."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = r'''// Durable, self-contained host control for the complete five-body candidate
// set. The default build embeds the actual candidates in this file; no ignored
// source fragment or generated intermediate is read at compile/run time.
#include <cassert>
#include <cstddef>
#include <cstring>

#define __cdecl
typedef unsigned int uint32_t;
typedef unsigned int DWORD;
typedef int BOOL;
typedef const char *LPCSTR;
enum { RFLAG_ALIEN = 0x80 };
struct RECT { int left, top, right, bottom; };

struct Player {
    BOOL is_leader_female;
    char filename[24], search_key[24], name_leader[24], title_leader[24];
    char adj_leader[128], adj_insult_leader[128], adj_faction[128];
    char adj_insult_faction[128], padding1[128], noun_faction[24];
    int noun_gender;
    BOOL is_noun_plural;
    char adj_name_faction[128], formal_name_faction[40], insult_leader[24];
    char desc_name_faction[24], assistant_name[24], scientist_name[24];
    char assistant_city[24], padding2[176];
    int pre_rule_flags[13];
    int rule_flags;
    char tail[0x90];
};
typedef char Player_size_is_native[(sizeof(Player) == 0x59C) ? 1 : -1];

struct PlayerData {
    uint32_t flags, ranking;
    int diff_level;
    uint32_t base_name_offset, base_sea_name_offset;
    int last_turn_new_base;
    uint32_t diplo_treaties[8];
    char pad_to_integrity[0xEC];
    int integrity_blemishes;
    char tail[0x20CC - 0x128];
};
typedef char PlayerData_stride_is_native[(sizeof(PlayerData) == 0x20CC) ? 1 : -1];
typedef char PlayerData_integrity_is_native[(offsetof(PlayerData, integrity_blemishes) == 0x124) ? 1 : -1];
typedef char PlayerData_treaties_are_native[(offsetof(PlayerData, diplo_treaties) == 0x18) ? 1 : -1];

struct Sprite {
    int width_, height_, extracts;
    Sprite() : width_(0), height_(0), extracts(0) {}
    int width() const { return width_; }
    int height() const { return height_; }
    int extract(void *, int, int, int, int width, int height, int) {
        ++extracts; width_ = width; height_ = height; return 0;
    }
};
struct Buffer { void load_pcx(const char *, int, int, int) {} };
struct Win {
    int move_x, move_y, shown, screen_x, screen_y;
    Win() : move_x(0), move_y(0), shown(-1), screen_x(0), screen_y(0) {}
    void move(int x, int y) { move_x = x; move_y = y; }
    void show(int value) { shown = value; }
    void client_to_screen(int *x, int *y) {
        *x += 100; *y += 200; screen_x = *x; screen_y = *y;
    }
};
struct DIB { struct Header { int biWidth, biHeight; } bmiHeader; };
struct GraphicWin : Win {
    DIB dib_;
    int init_calls, fill_color;
    GraphicWin() : init_calls(0), fill_color(-1) {}
    int init(RECT *r, int, int, Win *, int, int) {
        ++init_calls; dib_.bmiHeader.biWidth = r->right-r->left;
        dib_.bmiHeader.biHeight = -(r->bottom-r->top); return 0;
    }
    void fill(int color) { fill_color = color; }
};
struct Font { int height; Font() : height(0) {} int init(const char *, int h, int) { height=h; return 0; } };
struct Spot {
    int rc, faction, kind, count;
    Spot() : rc(-1), faction(0), kind(0), count(0) {}
    void init(int n) { count=n; }
    int check(int, int, int *f, int *k) { *f=faction; *k=kind; return rc; }
};
struct FlatButton : Win {
    int init_calls, id, virtual_calls, redraw_calls;
    Sprite *icons[3];
    FlatButton() : init_calls(0), id(-1), virtual_calls(0), redraw_calls(0) {
        icons[0]=icons[1]=icons[2]=0;
    }
    int init(const char *, int button_id, int, int, int, int, Win *, int) {
        ++init_calls; id=button_id; return 0;
    }
    int vslot_59() { ++virtual_calls; return 0; }
    void set_report_icons(Sprite *a, Sprite *b, Sprite *c) { icons[0]=a; icons[1]=b; icons[2]=c; }
    void set_slice_colours(int, int, int) {}
    void redraw() { ++redraw_calls; }
};
struct LockStateEntry {
    uint32_t leading_words_[4];
    union { uint32_t flags; void *pointer; } payload_;
    uint32_t trailing_words_[10];
};

struct NetDaemon {
    LockStateEntry lock_state_[24];
    int add_calls, add_stop_after, await_calls, awaited_code, unlock_calls;
    int open_calls, open_result;
    NetDaemon() : add_calls(0), add_stop_after(0), await_calls(0),
        awaited_code(-1), unlock_calls(0), open_calls(0), open_result(0) {}
    int add_global() { ++add_calls; return add_calls <= add_stop_after; }
    void await_diplo(int code) { ++await_calls; awaited_code=code; }
    void unlock() { ++unlock_calls; }
    int open_channel(int, int) { ++open_calls; return open_result; }
};
struct NetMsg {
    int calls, duration;
    const char *label, *target;
    NetMsg() : calls(0), duration(0), label(0), target(0) {}
    void pop(const char *l, int d, int, const char *t) {
        ++calls; label=l; duration=d; target=t;
    }
};
struct InfoWin { int resets; InfoWin() : resets(0) {} void reset() { ++resets; } };
struct Console {
    int faction_id_, time_controls_calls, bonus_calls, chat_calls, chat_target;
    Console() : faction_id_(0), time_controls_calls(0), bonus_calls(0),
        chat_calls(0), chat_target(-1) {}
    void time_controls() { ++time_controls_calls; }
    void use_time_bonus() { ++bonus_calls; }
    void chat(int f) { ++chat_calls; chat_target=f; }
};
struct PullDown {
    static int ids[64], count, selected, forced_id, shown_x, shown_y;
    static int show_calls;
    void init(int) { count=0; }
    int add_item(int id, char *) { ids[count++]=id; return 0; }
    int add_separator(int id) { ids[count++]=-id; return 0; }
    int show(int x, int y, int (*)()) { ++show_calls; shown_x=x; shown_y=y; return 0; }
    int get_selected() { return selected; }
    int item_id(int i) { return forced_id >= 0 ? forced_id : ids[i]; }
};
int PullDown::ids[64]; int PullDown::count; int PullDown::selected;
int PullDown::forced_id=-1; int PullDown::shown_x; int PullDown::shown_y;
int PullDown::show_calls;
struct Wave {
    int init_calls, pan, volume; const char *file;
    Wave() : init_calls(0), pan(0), volume(0), file(0) {}
    void init(char *f, unsigned long) { ++init_calls; file=f; }
    void set_pan(int n) { pan=n; }
    void set_volume(int n) { volume=n; }
};

Player Players[8] = {};
PlayerData PlayersData[8] = {};
NetDaemon NetDaemonNet;
NetMsg NetMsgPrimary;
Console ConsoleGlobal;
InfoWin ConsoleInfoWin, ConsoleStatusWin;
Wave g_MULTIWIN_WAVE;
Sprite *FactionDiplomacyPortraits[8] = {};
uint32_t LockEnableMask, GameState;
int PopupDialogFactionID, ExitTurnLoop, IsMultiplayerNet, IsMultiplayerPBEM;
int SunspotDuration, GenderDefault, PluralityDefault, ExpansionEnabled;
int StatusFlags, WinScreenWidth=800, WinScreenHeight=600;
int MultiWindowContactStatusMask;
int CommlinkPending, CommlinkFaction;
int timeout_pop_result, popup_mutate_network_to=-1, parser_mutates_actor;
int commlink_gate_result, has_tech_4e, has_tech_4;
int message_calls, message_ops[4], message_args[4][5];
int treaty_off_calls, treaty_off_args[3], pact_ends_calls, pact_ends_args[2];
int communicate_calls, diplo_calls, treaty_on_calls, call_council_calls;
int demand_calls, report_calls, parse_calls[4], parse_gender[4];
int check_calls, xpop_calls, xpops_calls, diplomacy_check_calls, diplomacy_caption_calls;
int log_calls, parse_num_value, time_tick_index, time_tick_count;
uint32_t time_ticks[30010];
int alive_mask_for_ui;
char StringTemp[1032];
const char *DefaultFontFace="Arial";
char *DiploLabelGettingDIPLOMATICLOCK=const_cast<char *>("Getting DIPLOMATIC LOCK");
char *DiploLabelNEEDGLOBAL=const_cast<char *>("NEEDGLOBAL");
char *DiploLabelSendingTREATYOFF=const_cast<char *>("Sending TREATY OFF");
char *DiploLabelSendingPACTENDS=const_cast<char *>("Sending PACT ENDS");
char *DiploLabelReleasingDIPLOMATICLOCK=const_cast<char *>("Releasing DIPLOMATIC LOCK");
char *DiploLabelSendingTREATYON=const_cast<char *>("Sending TREATY ON");

int parse_says(int id, char *, int, int) {
    if (id>=0 && id<4) { ++parse_calls[id]; parse_gender[id]=GenderDefault; }
    if (parser_mutates_actor && id==0) Players[2].is_leader_female=1;
    if (parser_mutates_actor && id==1) Players[2].is_leader_female=0;
    return 0;
}
int parse_num(int, int value) { parse_num_value=value; return 0; }
char *get_pact(int) { return const_cast<char *>("PACT"); }
int commlink_attempter(int, int) { return commlink_gate_result; }
int has_tech(int tech, int) { return tech==0x4E ? has_tech_4e : has_tech_4; }
int X_pop(const char *, int (*)()) { return timeout_pop_result; }
int x_pops_minimal(const char *, Sprite *, int (*)()) {
    ++xpops_calls;
    if (popup_mutate_network_to>=0) IsMultiplayerNet=popup_mutate_network_to;
    return 0;
}
int pop_wait() { return 0; }
int pop_expire() { return 0; }
void check_spock() { ++check_calls; }
void log_say(char *, int, int, int) { ++log_calls; }
void message_data(int opcode, int a, int b, int c, int d, int e) {
    message_ops[message_calls]=opcode;
    message_args[message_calls][0]=a; message_args[message_calls][1]=b;
    message_args[message_calls][2]=c; message_args[message_calls][3]=d;
    message_args[message_calls][4]=e; ++message_calls;
}
void treaty_off(int a, int b, int c) {
    ++treaty_off_calls; treaty_off_args[0]=a; treaty_off_args[1]=b; treaty_off_args[2]=c;
}
void pact_ends(int a, int b) {
    ++pact_ends_calls; pact_ends_args[0]=a; pact_ends_args[1]=b;
}
void diplomacy_check(int, int, int) { ++diplomacy_check_calls; }
void diplomacy_caption(int, int) { ++diplomacy_caption_calls; }
void communicate(int, int, int) { ++communicate_calls; }
void diplo(int, int) { ++diplo_calls; }
void treaty_on(int, int, int) { ++treaty_on_calls; }
void call_council(int) { ++call_council_calls; }
int demand_args[2];
void demands_withdrawal(int a, int b) { ++demand_calls; demand_args[0]=a; demand_args[1]=b; }
void report_intel(int) { ++report_calls; }
void draw_stub(int) {}
uint32_t timeGetTime() {
    if (time_tick_index<time_tick_count) return time_ticks[time_tick_index++];
    return time_ticks[time_tick_count-1];
}
int diplo_add(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a)+static_cast<uint32_t>(b)); }
bool is_alive(uint32_t faction) { return (alive_mask_for_ui & (1u<<faction))!=0; }
BOOL is_human(uint32_t faction) { return (LockEnableMask & (1u<<faction))!=0; }
char *label_get(int) { return const_cast<char *>("label"); }
unsigned char WinFillColour=9;
struct MainInterfaceFixture : Win {} MainInterfaceGlobal;

struct MultiWin : GraphicWin {
    uint32_t field_A4C_, field_A50_, field_A48_, field_A24_;
    Font font1_, font2_; Spot spot_;
    Sprite sprites1_[3], sprites2_[3], sprite3_, sprite4_, sprite5_, sprite6_, sprite7_;
    FlatButton flat_button1_, flat_button2_;
    MultiWin() : field_A4C_(1), field_A50_(1), field_A48_(1), field_A24_(1) {}
    void draw(int) {}
    void on_left_click(int, int);
    void on_right_click(int, int);
    void init();
};

void __cdecl commlink_attempt(int target_faction);
void __cdecl renounce_pact(int faction_id, int faction_id_with);

// BEGIN actual MultiWin::on_left_click candidate
// @@ACTUAL_OWNER_0@@

// BEGIN actual MultiWin::on_right_click candidate
// @@ACTUAL_OWNER_1@@

// BEGIN actual MultiWin::init candidate
// @@ACTUAL_OWNER_2@@

// BEGIN actual commlink_attempt candidate; the only v4 delta from v3 is the
// defined byte-1 extraction below. The 93A95C test uses IsMultiplayerPBEM.
// @@ACTUAL_OWNER_3@@

// BEGIN actual renounce_pact candidate. diplo_add is defined below as the
// same modulo-32 helper already owned by faction.cpp.
// @@ACTUAL_OWNER_4@@

static void reset_all() {
    std::memset(Players,0,sizeof(Players)); std::memset(PlayersData,0,sizeof(PlayersData));
    LockEnableMask=GameState=0; PopupDialogFactionID=ExitTurnLoop=0;
    IsMultiplayerNet=IsMultiplayerPBEM=SunspotDuration=0;
    GenderDefault=PluralityDefault=ExpansionEnabled=StatusFlags=0;
    NetMsgPrimary=NetMsg(); ConsoleGlobal=Console();
    ConsoleInfoWin.resets=ConsoleStatusWin.resets=0;
    NetDaemonNet=NetDaemon(); MultiWindowContactStatusMask=CommlinkPending=CommlinkFaction=0;
    std::memset(NetDaemonNet.lock_state_,0,sizeof(NetDaemonNet.lock_state_));
    g_MULTIWIN_WAVE=Wave(); timeout_pop_result=commlink_gate_result=0;
    has_tech_4e=has_tech_4=0; message_calls=treaty_off_calls=pact_ends_calls=0;
    communicate_calls=diplo_calls=treaty_on_calls=call_council_calls=demand_calls=report_calls=0;
    demand_args[0]=demand_args[1]=-1;
    check_calls=xpop_calls=xpops_calls=diplomacy_check_calls=diplomacy_caption_calls=log_calls=0;
    time_tick_index=0; time_tick_count=1; time_ticks[0]=100;
    parser_mutates_actor=0; popup_mutate_network_to=-1; parse_num_value=0; alive_mask_for_ui=0;
    std::memset(parse_calls,0,sizeof(parse_calls)); std::memset(parse_gender,0,sizeof(parse_gender));
    PullDown::count=PullDown::show_calls=0; PullDown::forced_id=-1;
    StringTemp[0]=0; WinScreenWidth=800; WinScreenHeight=600;
}
static void timer_constant(uint32_t t) {
    time_tick_index=0; time_tick_count=30000;
    for (int i=0;i<time_tick_count;++i) time_ticks[i]=t;
}

int main() {
    MultiWin win;
    reset_all(); win.spot_.rc=-1; win.on_left_click(0,0);
    assert(ConsoleInfoWin.resets==1&&ConsoleStatusWin.resets==1);

    reset_all(); win.spot_.rc=0; win.spot_.faction=3; win.spot_.kind=0;
    alive_mask_for_ui=1<<3; IsMultiplayerNet=1; MultiWindowContactStatusMask=1<<3;
    win.on_left_click(0,0);
    assert(MultiWindowContactStatusMask==0&&NetDaemonNet.lock_state_[2].trailing_words_[6]==(1u<<3));

    reset_all(); win.spot_.rc=0; win.spot_.faction=3; win.spot_.kind=1;
    alive_mask_for_ui=1<<3; LockEnableMask=1u<<(8+3); IsMultiplayerNet=0;
    win.on_left_click(0,0);
    assert(communicate_calls==1&&treaty_on_calls==1);

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0;
    PlayersData[0].diplo_treaties[2]=1; PlayersData[2].diplo_treaties[0]=2;
    PullDown::selected=3; win.on_right_click(0,0);
    assert(PullDown::ids[3]==8);

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0;
    PlayersData[2].diplo_treaties[0]=1; PlayersData[0].diplo_treaties[2]=2;
    PullDown::selected=3; win.on_right_click(0,0);
    assert(PullDown::ids[3]==9);

    // Native clicked IDs 8..11 are the four entries in the pinned jump table.
    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0; PullDown::selected=0;
    PullDown::forced_id=8; win.on_right_click(0,0);
    assert(PlayersData[0].integrity_blemishes==1&&treaty_off_calls==1);
    assert(treaty_off_args[0]==2&&treaty_off_args[1]==0);

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0; PullDown::selected=0;
    PullDown::forced_id=9; win.on_right_click(0,0);
    assert(demand_calls==1&&demand_args[0]==0&&demand_args[1]==2);

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0; PullDown::selected=0;
    PullDown::forced_id=10; MultiWindowContactStatusMask=1u<<2; win.on_right_click(0,0);
    assert(MultiWindowContactStatusMask==0&&NetDaemonNet.lock_state_[2].trailing_words_[6]==(1u<<2));

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0; PullDown::selected=0;
    PullDown::forced_id=11; win.on_right_click(0,0);
    assert(ConsoleGlobal.chat_calls==1&&ConsoleGlobal.chat_target==2);

    reset_all(); win.spot_.rc=0; win.spot_.faction=2; win.spot_.kind=1;
    alive_mask_for_ui=1<<2; ConsoleGlobal.faction_id_=0; PullDown::selected=0;
    PullDown::forced_id=8; win.on_right_click(0,0);
    assert(PlayersData[0].integrity_blemishes==1&&treaty_off_calls==1&&pact_ends_calls==1);
    assert(treaty_off_args[0]==2&&treaty_off_args[1]==0&&treaty_off_args[2]==1);

    reset_all(); win=MultiWin(); WinScreenWidth=800; win.init();
    assert(win.init_calls==1&&win.fill_color==9&&win.sprite3_.height()==6&&win.move_x==800-0x8A);
    reset_all(); win=MultiWin(); IsMultiplayerNet=1; WinScreenWidth=1024; WinScreenHeight=768;
    ExpansionEnabled=1; ConsoleGlobal.faction_id_=2; Players[2].rule_flags=RFLAG_ALIEN; win.init();
    assert(win.flat_button2_.init_calls==1&&win.flat_button1_.init_calls==0&&win.shown==0);

    reset_all(); ConsoleGlobal.faction_id_=2; LockEnableMask=1u<<11;
    commlink_attempt(3); assert(NetMsgPrimary.calls==0);
    reset_all(); ConsoleGlobal.faction_id_=2; LockEnableMask=1u<<3;
    commlink_attempt(3); assert(NetMsgPrimary.calls==0&&NetDaemonNet.open_calls==0);
    reset_all(); ConsoleGlobal.faction_id_=2; SunspotDuration=1; LockEnableMask=1u<<11;
    commlink_attempt(3); assert(std::strcmp(NetMsgPrimary.label,"SUNSPOTSNO")==0);
    reset_all(); ConsoleGlobal.faction_id_=2; IsMultiplayerPBEM=1;
    LockEnableMask=(1u<<11)|(1u<<3); commlink_attempt(3);
    assert(diplo_calls==1&&communicate_calls==0);
    reset_all(); ConsoleGlobal.faction_id_=2; IsMultiplayerNet=1;
    LockEnableMask=1u<<11; NetDaemonNet.open_result=1; commlink_attempt(3);
    assert(std::strcmp(NetMsgPrimary.label,"NOCHANNEL")==0&&communicate_calls==0);

    reset_all(); timer_constant(100); NetDaemonNet.add_stop_after=1; timeout_pop_result=1;
    time_ticks[1]=10100; renounce_pact(2,3);
    assert(PlayersData[2].integrity_blemishes==0&&NetDaemonNet.unlock_calls==0);

    reset_all(); timer_constant(0xFFFFFFF0u); NetDaemonNet.add_stop_after=1; timeout_pop_result=1;
    time_ticks[1]=0x2710; renounce_pact(2,3);
    assert(check_calls==1&&NetDaemonNet.unlock_calls==0);

    reset_all(); timer_constant(100); IsMultiplayerNet=1;
    NetDaemonNet.add_stop_after=0; parser_mutates_actor=1; popup_mutate_network_to=1;
    renounce_pact(2,3);
    assert(message_calls==2&&message_ops[0]==0x2442&&message_ops[1]==0x244D&&pact_ends_calls==0);
    assert(diplomacy_check_calls==1&&xpops_calls==1&&PlayersData[2].integrity_blemishes==1);
    return 0;
}
'''

def _actual_bodies():
    ui = (ROOT / "src/multiwin.cpp").read_text()
    faction = (ROOT / "src/faction.cpp").read_text()
    names = ("MultiWin::on_left_click", "MultiWin::on_right_click",
             "MultiWin::init", "commlink_attempt", "renounce_pact")
    result = []
    for index, name in enumerate(names):
        signature = (r"void\s+" + (r"__cdecl\s+" if index > 2 else "")
                     + re.escape(name) + r"\([^;{}]*\)(?=\s*\{)")
        result.append(definition(ui if index < 3 else faction, signature))
    return result

def _replace_once(text, old, new):
    assert text.count(old) == 1, old
    return text.replace(old, new, 1)

@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_five_actual_owners_and_native_dispatch_damage(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    actual = _actual_bodies()
    variants = [actual]
    shifted = list(actual)
    for item in (8, 9, 10, 11):
        assert len(re.findall(r"case " + str(item) + ":", shifted[1])) == 1
    shifted[1] = re.sub(r"case (8|9|10|11):",
                       lambda m: "case " + str(int(m[1]) - 1) + ":", shifted[1])
    variants.append(shifted)
    for index, old, new in (
        (1, "demands_withdrawal(ConsoleGlobal.faction_id_, faction);",
            "demands_withdrawal(faction, ConsoleGlobal.faction_id_);"),
        (1, "PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction]",
            "PlayersData[faction].diplo_treaties[ConsoleGlobal.faction_id_]"),
        (3, "(LockEnableMask >> 8) & 0xFFu", "LockEnableMask & 0xFFu"),
        (4, "treaty_off(faction_id_with, faction_id, 1);",
            "treaty_off(faction_id, faction_id_with, 1);"),
    ):
        damaged = list(actual)
        damaged[index] = _replace_once(damaged[index], old, new)
        variants.append(damaged)
    with tempfile.TemporaryDirectory(prefix="osmx-multiwin-contact-") as temporary:
        source, binary = Path(temporary) / "control.cpp", Path(temporary) / "control"
        for variant_index, bodies in enumerate(variants):
            text = FIXTURE
            for index, body in enumerate(bodies):
                text = _replace_once(text, "// @@ACTUAL_OWNER_" + str(index) + "@@", body)
            source.write_text(text)
            built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                    "-Werror", "-pedantic", str(source), "-o", str(binary)],
                                   capture_output=True, text=True, timeout=45)
            assert built.returncode == 0, built.stdout + built.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (variant_index == 0), (
                compiler_name, variant_index, run.returncode, run.stdout, run.stderr)

