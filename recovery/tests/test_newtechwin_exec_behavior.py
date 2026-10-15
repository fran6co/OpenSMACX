from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/newtechwin.cpp"


def source_body(signature: str) -> str:
    text = SOURCE.read_text()
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start : pos + 1]
    raise AssertionError(f"unterminated method: {signature}")


BODY = source_body("void NewTechWin::exec(int technology_id, int other_faction_id)")
CTOR = source_body("NewTechWin::NewTechWin()")
DTOR = source_body("NewTechWin::~NewTechWin()")
LINK_HOOK = source_body("int NewTechWin::on_link_clicked(int, char *link)")
INFO_CALLBACK = source_body("static void __cdecl newtech_info(char *text)")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdlib>
#define __cdecl
typedef long LONG;
char *_itoa(int value, char *buffer, int radix) { std::sprintf(buffer, radix == 10 ? "%d" : "%x", value); return buffer; }
struct RECT { LONG left, top, right, bottom; };
struct Font { int id; };
struct BoxStyle { int id; };
struct Sprite { int id; };
struct Trace {
    int draw_calls, modal_calls, hide_calls, close_calls, release_calls;
    int help_calls, parse_calls, datalink_calls, wave_unloads, wave_plays;
    int wave_fades, wave_loads, last_volume, initialized_boxes;
    int mutate_multiplayer_on_help, reset_multiplayer_on_help;
    int mutate_human_mask_on_help;
    std::string wave_path;
    std::vector<int> help_style;
    std::vector<std::string> appended[4];
} trace = {};
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    uint32_t game_more_preferences_;
    GameDataLockMaskView lock_masks_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage = {};
int IsMultiplayerNet = 0;
const int MPREF_AV_VOICEOVER_TECH_FAC = 0x4000;
const int MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP = 0x20000;
int CurrentTechnologyBoxIndex = -1;
int PendingTechnologyLink = -1;
char StringTemp[1032] = {};
char datalink_blurb_file[] = "BLURBSX";
char *DatalinkBlurbFile = datalink_blurb_file;
struct Console { int faction_id_; } ConsoleGlobal = {};
struct MainInterface { Font primary; Font *primary_font() { return &primary; } } MainInterfaceGlobal = {};
Font font_pool[64] = {};
Font *find_font(int size, int style) { font_pool[size * 4 + style].id = size * 4 + style; return &font_pool[size * 4 + style]; }
struct BoxStyle InterfaceBoxStyles[51] = {};
// Borrower identity fixture, not a model of production guarded lifetimes.
struct NewTechSpritesFixtureOwner {
    Sprite elements_[6];
    Sprite &at(int index) { return elements_[index]; }
} NewTechSpritesOwner = {};
char *label_get(int) { static char label[] = "close"; return label; }
struct Buffer {
    Font *fonts[4]; int colors[4];
    void set_font(Font *a, Font *b, Font *c, Font *d) { fonts[0]=a; fonts[1]=b; fonts[2]=c; fonts[3]=d; }
    void set_text_color(int a, int b, int c, int d) { colors[0]=a; colors[1]=b; colors[2]=c; colors[3]=d; }
};
struct StringList {
    std::string face, contents; int clear_existing, flags;
    int load(char *font, char *text, int clear, int options) {
        face=font; contents=text; clear_existing=clear; flags=options; return 1;
    }
};
struct StringBox : Buffer {
    RECT rect; int init_count, show_count, special_count; StringList list;
    StringBox() : rect{}, init_count(0), show_count(0), special_count(0) {}
    void init(int x,int y,int w,int h,void *,StringList *,int) {
        ++init_count; rect.left=x; rect.top=y; rect.right=x+w; rect.bottom=y+h; ++trace.initialized_boxes;
    }
    StringList &strings() { return list; }
    int add_special(char *text) { ++special_count; trace.appended[CurrentTechnologyBoxIndex].push_back(text); return 1; }
    void add_fixup() {}
    void show(int) { ++show_count; }
};
struct FlatButton : Buffer {
    char *label; int args[6], init_count, redraw_count; BoxStyle *styles[3]; Sprite *icons[3];
    FlatButton() : label(0), args{}, init_count(0), redraw_count(0), styles{}, icons{} {}
    void init(char *name,int id,int x,int y,int w,int h,void *,int) {
        label=name; args[0]=id; args[1]=x; args[2]=y; args[3]=w; args[4]=h; ++init_count;
    }
    void set_border_styles(BoxStyle *a,BoxStyle *b,BoxStyle *c) { styles[0]=a; styles[1]=b; styles[2]=c; }
    void set_report_icons(Sprite *a,Sprite *b,Sprite *c) { icons[0]=a; icons[1]=b; icons[2]=c; }
    void redraw() { ++redraw_count; }
};
struct ButtonGroup {
    FlatButton *members[2]; int add_count, selected;
    void init(int,int) { add_count=0; }
    void add(FlatButton *b) { members[add_count++]=b; }
    void set(int,int value) { selected=value; }
};
struct Datalink {
    uint32_t parsed; int remainder;
    void parse_id(int id,uint32_t *out,int *tail) { ++trace.parse_calls; *out=static_cast<uint32_t>(id/10000); *tail=id%10000; }
    void exec(uint32_t id,int tail) { ++trace.datalink_calls; parsed=id; remainder=tail; }
    int trace_parse_calls() const { return 0; }
} DatalinkMain;
struct Wave {
    void unload() { ++trace.wave_unloads; }
    int load(const char *path) { ++trace.wave_loads; trace.wave_path=path; return 1; }
    void set_volume(int value) { trace.last_volume=value; }
    int play() { ++trace.wave_plays; return 1; }
    void fade(unsigned long) { ++trace.wave_fades; }
} g_WAVE_GENERAL;
struct WaveDevice { int get_group_volume(unsigned int) { return 73; } } WaveDeviceGlobal;
int modal_event_pending = 0;
struct NewTechWin {
    uint32_t field_A14_,field_A18_,field_A1C_,field_A20_,field_A24_,field_A28_,field_A2C_,field_A30_;
    uint32_t field_A34_,field_A38_,field_A3C_,field_A40_,field_A44_,field_A48_,field_A4C_,field_A50_;
    uint32_t field_A54_,field_A58_,field_A5C_,field_A60_,field_A64_,field_A68_,field_A6C_,field_A70_;
    uint32_t field_A74_,field_A78_,field_A7C_,field_A80_,field_A84_,field_A88_,field_A8C_,field_A90_;
    uint32_t field_A94_,field_A98_,field_A9C_,field_AA0_,field_AA4_,field_AA8_,field_AAC_,field_AB0_;
    uint32_t field_AB4_,field_AB8_,field_ABC_,field_AC0_,field_AC4_,field_AC8_,field_ACC_,field_AD0_;
    uint32_t field_AD4_,field_AD8_,field_ADC_,field_AE0_,field_AE4_,field_AE8_,field_AEC_,field_AF0_;
    uint32_t field_AF4_,field_AF8_,field_AFC_,field_B00_,field_B04_,field_B08_,field_B0C_,field_B10_;
    uint32_t field_B14_,field_B18_,field_B1C_,field_B20_,field_B24_,field_B28_,field_16C_;
    FlatButton flatButtons_[3]; ButtonGroup buttonGroup_; StringBox stringBoxes_[4]; StringBox stringBox2_;
    int init_count,fill_color,center_count,modal_count,hide_count,close_count,release_count;
    NewTechWin();
    ~NewTechWin();
    int init(RECT *r,char *,int,void *,void *,void *) { ++init_count; field_A14_=r->left; field_A18_=r->top; field_A1C_=r->right; field_A20_=r->bottom; return 1; }
    int fill(int color) { fill_color=color; return 1; }
    int center() { ++center_count; return 1; }
    void show(int) {}
    int set_modal(int,int (*)(),void *) { ++modal_count; ++trace.modal_calls; if(modal_event_pending) { modal_event_pending=0; on_link_clicked(0,const_cast<char *>("52007")); } return 1; }
    void hide() { ++hide_count; ++trace.hide_calls; }
    uint32_t close() { ++close_count; ++trace.close_calls; return 0; }
    void release_modal() { ++release_count; ++trace.release_calls; }
    int on_link_clicked(int,char *link);
    void exec(int technology_id,int other_faction_id);
} NewTechWindow;
void do_all_draws() { ++trace.draw_calls; }
void help_tech_info(void (__cdecl *append)(char *),int,int,int box_index) {
    ++trace.help_calls; char text[20]; std::sprintf(text,"box%d",box_index); append(text);
    if(trace.mutate_multiplayer_on_help==box_index) IsMultiplayerNet=1;
    if(trace.reset_multiplayer_on_help==box_index) IsMultiplayerNet=0;
    if(trace.mutate_human_mask_on_help==box_index) GameDataStateStorage.fields_.lock_masks_.enable_mask_=1u<<ConsoleGlobal.faction_id_;
}
'''


def compile_and_run() -> None:
    program = HARNESS + "\n" + CTOR + "\n" + DTOR + "\n" + LINK_HOOK + "\n" + INFO_CALLBACK + "\n" + BODY + r'''
int main() {
    ConsoleGlobal.faction_id_=2; GameDataStateStorage.fields_.lock_masks_.enable_mask_=0; IsMultiplayerNet=0;
    GameDataStateStorage.fields_.game_more_preferences_=MPREF_AV_VOICEOVER_TECH_FAC|MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP;
    trace.mutate_multiplayer_on_help=0; trace.reset_multiplayer_on_help=3;
    trace.mutate_human_mask_on_help=0; modal_event_pending=1;
    NewTechWindow.exec(37,5);
    assert(NewTechWindow.field_B24_==37 && NewTechWindow.field_B28_==5);
    assert(NewTechWindow.init_count==1 && NewTechWindow.fill_color==9);
    assert(trace.initialized_boxes==5 && trace.help_calls==4);
    for(int i=0;i<4;++i) {
        assert(NewTechWindow.stringBoxes_[i].special_count==1);
        assert(trace.appended[i].size()==1 && trace.appended[i][0]==std::string("box")+char('0'+i));
    }
    const RECT expected_box_rects[4] = {
        {280,265,531,277}, {280,288,531,356},
        {19,469,531,551}, {19,390,531,458}
    };
    for(int i=0;i<4;++i) {
        const RECT &actual=NewTechWindow.stringBoxes_[i].rect;
        assert(actual.left==expected_box_rects[i].left);
        assert(actual.top==expected_box_rects[i].top);
        assert(actual.right==expected_box_rects[i].right);
        assert(actual.bottom==expected_box_rects[i].bottom);
    }
    assert(NewTechWindow.field_B24_==37 && NewTechWindow.field_B28_==5);
    assert(NewTechWindow.field_A5C_==273 && NewTechWindow.field_A64_==276);
    assert(NewTechWindow.field_AC4_==276 && NewTechWindow.field_A84_==15);
    assert(NewTechWindow.stringBoxes_[0].fonts[0]->id==56);
    assert(NewTechWindow.stringBoxes_[0].fonts[3]->id==59);
    assert(NewTechWindow.stringBoxes_[1].fonts[3]->id==58);
    assert(NewTechWindow.stringBox2_.strings().contents=="TECH37");
    assert(NewTechWindow.stringBox2_.strings().face=="BLURBSX");
    assert(NewTechWindow.flatButtons_[0].styles[0]==&InterfaceBoxStyles[27]);
    assert(NewTechWindow.flatButtons_[0].args[1]==15);
    assert(NewTechWindow.flatButtons_[0].args[2]==565);
    assert(NewTechWindow.flatButtons_[0].args[3]==520);
    assert(NewTechWindow.flatButtons_[0].args[4]==20);
    assert(NewTechWindow.flatButtons_[1].args[1]==473);
    assert(NewTechWindow.flatButtons_[1].args[2]==518);
    assert(NewTechWindow.flatButtons_[1].args[3]==28);
    assert(NewTechWindow.flatButtons_[1].args[4]==34);
    assert(NewTechWindow.flatButtons_[2].icons[2]==&NewTechSpritesOwner.at(5));
    assert(NewTechWindow.buttonGroup_.add_count==2 && NewTechWindow.buttonGroup_.selected==1);
    assert(trace.wave_path=="voices\\tech37.mp3" && trace.wave_loads==1);
    assert(trace.wave_unloads==1 && trace.wave_plays==1 && trace.last_volume==73);
    assert(trace.parse_calls==1 && trace.datalink_calls==1);
    assert(DatalinkMain.parsed==5 && DatalinkMain.remainder==2007);
    assert(trace.release_calls==1 && trace.close_calls==1 && trace.wave_fades==1);
    assert(trace.modal_calls==2 && trace.hide_calls==2 && trace.draw_calls==4);
    const int releases_before=trace.release_calls;
    IsMultiplayerNet=1;
    assert(NewTechWindow.on_link_clicked(0,const_cast<char *>("84"))==1);
    assert(PendingTechnologyLink==-1 && trace.release_calls==releases_before);
    IsMultiplayerNet=0;
    assert(NewTechWindow.on_link_clicked(0,0)==1);
    assert(NewTechWindow.on_link_clicked(0,const_cast<char *>("84"))==0);
    assert(PendingTechnologyLink==84 && trace.release_calls==releases_before+1);
    PendingTechnologyLink=-1;
    trace.mutate_human_mask_on_help=-1;
    GameDataStateStorage.fields_.game_more_preferences_=MPREF_AV_VOICEOVER_TECH_FAC; GameDataStateStorage.fields_.lock_masks_.enable_mask_=1u<<1;
    const int loads_before=trace.wave_loads;
    NewTechWindow.exec(4,7);
    assert(trace.wave_loads==loads_before);
    // Native 484741 tests AL against only the low byte of the faction mask.
    ConsoleGlobal.faction_id_=8; GameDataStateStorage.fields_.lock_masks_.enable_mask_=1u<<8;
    NewTechWindow.exec(4,7);
    assert(trace.wave_loads==loads_before);
    // x86 masks the shift count to five bits, including unusual caller IDs.
    ConsoleGlobal.faction_id_=33; GameDataStateStorage.fields_.lock_masks_.enable_mask_=1u<<1;
    NewTechWindow.exec(4,7);
    assert(trace.wave_loads==loads_before+1);
    return 0;
}
'''
    with tempfile.TemporaryDirectory() as tmp:
        cpp=Path(tmp)/"newtechwin_exec.cpp"; exe=Path(tmp)/"newtechwin_exec"
        cpp.write_text(program)
        subprocess.run(["g++","-std=c++11","-Wall","-Wextra","-Werror","-Wno-unused-parameter",str(cpp),"-o",str(exe)],check=True)
        subprocess.run([str(exe)],check=True)


def test_exec_constructs_popup_and_dispatches_live_link_and_voice_state():
    compile_and_run()
