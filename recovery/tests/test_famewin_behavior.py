"""Host controls for the actual FameWin record and click bodies."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_body(signature: str) -> str:
    source = (ROOT / "src/famewin.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated body: {signature}")


LEFT_CLICK = "void FameWin::on_left_click(int x, int y) {"
DEMOTE = "void FameWin::demote(int rank) {"
ADD_RECORD = "void FameWin::try_to_add_me() {"
INIT = "int FameWin::init() {"
REDRAW = "void FameWin::on_redraw() {"


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#define FALSE 0
struct HallOfFameRecord {
  uint8_t present, award_flags, unknown_02[2];
  int year, leader_one, leader_two, rating, field_14, field_18, field_1C;
  uint8_t field_20, field_21;
  char leader_title[24], leader_name[24], faction_noun[24];
  uint8_t unknown_6A[0x22];
};
HallOfFameRecord HallOfFameRecords[35];
char StringTemp[1032];
int total_buffer_loads, total_buffer_writes;
char *_itoa(int value, char *out, int radix) {
  assert(radix == 10); std::sprintf(out, "%d", value); return out;
}
struct Buffer {
  int loads, writes;
  char loaded[64], written[64];
  Buffer() : loads(0), writes(0) { loaded[0] = written[0] = 0; }
  int load_pcx(const char *name, void *, int, int) {
    ++loads; ++total_buffer_loads;
    std::strncpy(loaded, name, sizeof(loaded)); return 0;
  }
  int write_pcx(char *name) {
    ++writes; ++total_buffer_writes;
    std::strncpy(written, name, sizeof(written)); return 0;
  }
};
struct Spot {
  int result, out_x, out_y, calls;
  int check(int x, int y, int *out_x_arg, int *out_y_arg) {
    ++calls; assert(x == 20 && y == 30);
    *out_x_arg = out_x; *out_y_arg = out_y; return result;
  }
};
struct LockState { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct NetDaemon { LockState lock_state_[24]; } NetDaemonNet;
struct { int faction_id_; } ConsoleGlobal;
struct Player {
  int is_leader_female;
  char title_leader[24], name_leader[24];
  int noun_gender, is_noun_plural;
  char noun_faction[24];
} Players[8];
struct PlayerData {
  int unk_107, energy_reserves, score_items_[9], diff_level, unk_109;
} PlayersData[8];
struct GameDataState {
  struct Fields { uint32_t game_preferences_, game_rules_, turn_current_num_; } fields_;
} GameDataStateStorage;
uint32_t MapNativeLifeForms;
int compute_calls, last_gender, last_plural;
void compute_score(int faction, int *, int *, int) {
  ++compute_calls; PlayersData[faction].unk_107 = 45;
}
int get_rating(int, int score) { return score + 30; }
int game_year(int turn) { return 2100 + turn; }
void parse_set(int gender, int plural) { last_gender = gender; last_plural = plural; }
struct GraphicWin : Buffer {};
struct WorldWin : GraphicWin {} WorldClimateWorldWin;
struct FameWin {
  uint32_t factionID_, selectedRecord_;
  Spot spot_;
  int redraws;
  FameWin() : factionID_(0), selectedRecord_(0), redraws(0) {}
  void redraw() { ++redraws; }
  void demote(int rank);
  void on_left_click(int x, int y);
  void try_to_add_me();
};
BODY_LEFT
BODY_DEMOTE
BODY_ADD
static void reset() {
  std::memset(HallOfFameRecords, 0, sizeof HallOfFameRecords);
  std::memset(Players, 0, sizeof Players);
  std::memset(PlayersData, 0, sizeof PlayersData);
  std::memset(&NetDaemonNet, 0, sizeof NetDaemonNet);
  GameDataStateStorage.fields_.game_preferences_ = 0;
  GameDataStateStorage.fields_.game_rules_ = 0;
  GameDataStateStorage.fields_.turn_current_num_ = 0;
  MapNativeLifeForms = 0;
  compute_calls = last_gender = last_plural = 0;
  total_buffer_loads = total_buffer_writes = 0;
  StringTemp[0] = 0;
  WorldClimateWorldWin.loads = WorldClimateWorldWin.writes = 0;
  WorldClimateWorldWin.loaded[0] = WorldClimateWorldWin.written[0] = 0;
  ConsoleGlobal.faction_id_ = 1;
}
int main() {
  reset();
  FameWin click;
  click.spot_.result = 4; click.spot_.out_x = 8; click.spot_.out_y = 3;
  click.on_left_click(20, 30);
  assert(click.spot_.calls == 1 && click.selectedRecord_ == 8 && click.redraws == 1);
  NetDaemonNet.lock_state_[2].payload_.flags = 1;
  click.on_left_click(20, 30);
  assert(click.spot_.calls == 1 && click.redraws == 1);

  reset();
  HallOfFameRecords[1].unknown_6A[0] = 0xA5;
  HallOfFameRecords[1].unknown_6A[1] = 0x5A;
  HallOfFameRecords[1].unknown_6A[2] = 0x11;
  HallOfFameRecords[0].unknown_6A[2] = 0xC3;
  HallOfFameRecords[0].present = 1; HallOfFameRecords[0].award_flags = 0x55;
  HallOfFameRecords[0].year = 2140; HallOfFameRecords[0].leader_one = 11;
  HallOfFameRecords[0].leader_two = 12; HallOfFameRecords[0].rating = 13;
  HallOfFameRecords[0].field_14 = 14; HallOfFameRecords[0].field_18 = 18;
  HallOfFameRecords[0].field_1C = 19; HallOfFameRecords[0].field_20 = 20;
  HallOfFameRecords[0].field_21 = 21;
  std::strcpy(HallOfFameRecords[0].leader_title, "title");
  FameWin demotion; demotion.factionID_ = 1; demotion.demote(0);
  assert(!std::strcmp(StringTemp, "hof_1_2.pcx"));
  assert(total_buffer_loads == 1 && total_buffer_writes == 1);
  assert(HallOfFameRecords[1].rating == 13 &&
         HallOfFameRecords[1].leader_title[0] == 't');
  assert(HallOfFameRecords[1].unknown_6A[0] == 0xA5 &&
         HallOfFameRecords[1].unknown_6A[1] == 0x5A &&
         HallOfFameRecords[1].unknown_6A[2] == 0xC3);

  reset();
  FameWin entry;
  Players[1].is_leader_female = 1; Players[1].noun_gender = 2;
  Players[1].is_noun_plural = 1;
  std::strcpy(Players[1].title_leader, "Lady");
  std::strcpy(Players[1].name_leader, "Leader");
  std::strcpy(Players[1].noun_faction, "Faction");
  PlayersData[1].energy_reserves = 900;
  PlayersData[1].diff_level = 4; PlayersData[1].score_items_[0] = 10;
  PlayersData[1].score_items_[2] = 20; PlayersData[1].score_items_[3] = 30;
  PlayersData[1].score_items_[4] = 40; PlayersData[1].score_items_[5] = 1;
  PlayersData[1].unk_109 = 1;
  GameDataStateStorage.fields_.turn_current_num_ = 7; MapNativeLifeForms = 1;
  entry.try_to_add_me();
  const HallOfFameRecord &added = HallOfFameRecords[0];
  assert(compute_calls == 1 && entry.factionID_ == 1 && entry.selectedRecord_ == 0);
  assert(added.present == 5 && added.rating == 75 && added.leader_one == 45);
  assert(added.leader_two == 900 && added.year == 2107 && added.field_1C == 50);
  assert(added.field_14 == 40 && added.field_18 == 10 && added.field_20 == 4);
  assert(added.field_21 == 2 && !std::strcmp(added.leader_title, "Lady"));
  assert(!std::strcmp(added.leader_name, "Leader") &&
         !std::strcmp(added.faction_noun, "Faction"));
  assert(last_gender == 2 && last_plural == 1);
  assert(total_buffer_loads == 4 && total_buffer_writes == 5);
  assert(WorldClimateWorldWin.writes == 1 &&
         !std::strcmp(WorldClimateWorldWin.written, "hof_1_1.pcx"));
}
"""


def run_host_control(tmp_path, bodies, *, should_pass: bool) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = HARNESS
    for token, body in zip(("BODY_LEFT", "BODY_DEMOTE", "BODY_ADD"), bodies):
        harness = harness.replace(token, body)
    executable = tmp_path / "famewin-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) is should_pass, result.stderr.decode()


def test_famewin_actual_record_and_click_bodies(tmp_path):
    run_host_control(
        tmp_path,
        (source_body(LEFT_CLICK), source_body(DEMOTE), source_body(ADD_RECORD)),
        should_pass=True,
    )


def test_famewin_damage_control_detects_click_gate_change(tmp_path):
    left_click = source_body(LEFT_CLICK)
    before = "NetDaemonNet.lock_state_[2].payload_.flags == 0"
    assert before in left_click
    mutant = left_click.replace(before, "NetDaemonNet.lock_state_[2].payload_.flags != 0")
    run_host_control(
        tmp_path,
        (mutant, source_body(DEMOTE), source_body(ADD_RECORD)),
        should_pass=False,
    )


def test_famewin_damage_control_detects_lost_demote_tail_copy(tmp_path):
    demote = source_body(DEMOTE)
    copy = "memcpy(destination.unknown_6A + 2, source.unknown_6A + 2, 0x20);"
    assert copy in demote
    mutant = demote.replace(copy, "")
    run_host_control(
        tmp_path,
        (source_body(LEFT_CLICK), mutant, source_body(ADD_RECORD)),
        should_pass=False,
    )


PAINTER_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#define FALSE 0
typedef long LONG;
enum { MaxPlayerNum = 8, MaxDiffNum = 6, FAC_ASCENT_TO_TRANSCENDENCE = 0 };
int WinScreenWidth = 1024, WinScreenHeight = 768, WinFillColour = 0;
struct RECT { LONG left, top, right, bottom; };
struct Font { int line_advance() const { return 12; } } font1, font2, font3;
int clip_calls, font_calls, fill_calls, wrap_calls, center_calls, spot_adds;
int sprite_extracts, sprite_draws, button_inits, button_redraws;
int wrapped_label_974, wrapped_label_976;
int stale_clip_copies, copy_calls, text_width_calls;
int uppercase_difficulty_writes, lowercase_difficulty_writes;
int pcx_loads;
int init_x, init_y, init_w, init_h, init_result;
char init_caption[8];
struct Buffer {
  int load_pcx(const char *, int, int, int) { ++pcx_loads; return 0; }
  bool clip_ready;
  Buffer() : clip_ready(false) {}
  void copy(Buffer *target, int, int, int, int, int, int) {
    ++copy_calls;
    if (copy_calls <= 2 && !target->clip_ready) ++stale_clip_copies;
  }
  void restore_clip() { clip_ready=true; }
  void set_clip(RECT *) { ++clip_calls; clip_ready=false; }
  void set_font(Font *, int, int, int) { ++font_calls; }
  void set_text_color(int, int, int, int) {}
  void write_l(char *, int, int, int) {}
  void write_right_l(char *text, int, int, int, int) {
    if (!std::strcmp(text,"VALUE")) ++uppercase_difficulty_writes;
    if (!std::strcmp(text,"value")) ++lowercase_difficulty_writes;
  }
  void write_cent_l(char *, int, int, int, int) { ++center_calls; }
  void wrap_cent(char *text, int, int, int) {
    ++wrap_calls;
    if (std::strstr(text,"label974")) ++wrapped_label_974;
    if (std::strstr(text,"label976")) ++wrapped_label_976;
  }
  int wrap_height(char *, int) { return 12; }
  int text_width(char *text) {
    ++text_width_calls;
    return static_cast<int>(std::strlen(text) * 5);
  }
};
struct Win {};
struct GraphicWin : Win, Buffer {
  RECT working_rect;
  int init(int x, int y, int w, int h, char *caption,
           int, int, int, int) {
    init_x=x; init_y=y; init_w=w; init_h=h;
    std::strncpy(init_caption, caption, sizeof(init_caption)-1);
    return init_result;
  }
  RECT *buffer_working_rect() { return &working_rect; }
  void fill(int) {}
  void fill(int, int, int, int, int) { ++fill_calls; }
};
struct Sprite {
  int transparent_index() const { return 0; }
  int extract(Buffer *, int, int, int, int, int, int) {
    ++sprite_extracts; return 0;
  }
  void draw(Buffer *, int, int, int) { ++sprite_draws; }
};
struct BoxSpriteParams {};
struct FlatButton : Buffer {
  int page;
  FlatButton() : page(-99) {}
  int init(int, int, int, int, int, int, Win *, int) {
    ++button_inits; return 0;
  }
  int init(const char *, int, int, int, int, int, Win *, int) {
    ++button_inits; return 0;
  }
  void set_report_icons(Sprite *, Sprite *, Sprite *) {}
  void set_border_styles(BoxSpriteParams *, BoxSpriteParams *, BoxSpriteParams *) {}
  void set_bubble_text(const char *) {}
  void set_report_page_state(int state) { page=state; }
  void hide() {}
  void redraw() { ++button_redraws; }
};
struct ButtonGroup {
  uint32_t field_84_;
  void init(int, int) {}
  void add(FlatButton *) {}
  void set(int id, int) { field_84_=static_cast<uint32_t>(id); }
};
struct Spot {
  void init(int) {}
  int ids[5];
  void add(int id, int, int, int, int, int) {
    if (spot_adds < 5) ids[spot_adds] = id;
    ++spot_adds;
  }
  void kill_type(int) {}
};
struct HallOfFameRecord {
  uint8_t present, award_flags, unknown_02[2];
  int year, leader_one, leader_two, rating, field_14, field_18, field_1C;
  uint8_t field_20, field_21;
  char leader_title[24], leader_name[24], faction_noun[24];
  uint8_t unknown_6A[0x22];
};
HallOfFameRecord HallOfFameRecords[35];
struct Player { char title_leader[32], name_leader[32], noun_faction[32]; } Players[8];
struct Art { Sprite report_logo[2], diplomacy_landscape; } FactionsArt[8];
struct FacilityEntry { uint32_t name; } Facility[1];
uint32_t Difficulty[MaxDiffNum];
struct StringService { char *get(int) { static char text[]="value"; std::strcpy(text,"value"); return text; } } StringTable;
struct ReportState { int field_1E230_; Font *field_1E190_, *field_1E19C_, *field_1E1B4_; };
ReportState ReportIfGlobal = {2, &font1, &font2, &font3};
struct Tutorial { Font font1_; } TutWinBaseWindow;
BoxSpriteParams InterfaceBoxStyles[51];
char StringTemp[1032];
char FameWinStandardCaption[4], FameWinWideCaption[4];
char *label_get(int id) { static char label[32]; std::sprintf(label,"label%d",id); return label; }
char *upper(char *s) { for (char *p=s; *p; ++p) *p=static_cast<char>(std::toupper(*p)); return s; }
char *CharUpperA(char *s) { return upper(s); }
char *_itoa(int value, char *out, int radix) { assert(radix==10); std::sprintf(out,"%d",value); return out; }
struct FameWin : GraphicWin {
  uint32_t field_A14_,field_A18_,field_A1C_,field_A20_,field_A24_,field_A28_,field_A2C_,field_A30_;
  uint32_t field_A34_,field_A38_,field_A3C_,field_A40_,field_A44_,field_A48_,factionID_;
  Spot spot_; FlatButton flatButtons1_[7]; ButtonGroup buttonGroup_;
  FlatButton flatButtons2_[6], flatButton3_; uint32_t selectedRecord_;
  Buffer buffer1_,buffer2_;
  uint32_t field_B42C_,field_B430_,field_B434_,field_B438_,field_B43C_,field_B440_,field_B444_,field_B448_;
  uint32_t field_B44C_,field_B450_,field_B454_,field_B458_,field_B45C_,field_B460_,field_B464_,field_B468_;
  uint32_t field_B46C_,field_B470_,field_B474_,field_B478_,field_B47C_,field_B480_,field_B484_,field_B488_;
  Sprite sprites_[12];
  int init(); void on_redraw();
};
static void famewin_clear_text() { StringTemp[0]='\0'; }
static void famewin_append_label(int index) { std::strcat(StringTemp,label_get(index)); }
static void famewin_append_number(int value) { char number[32]; std::sprintf(number,"%d",value); std::strcat(StringTemp,number); }
static void famewin_write_left(Buffer *b,int x,int y) { int n=std::strlen(StringTemp); if(n) b->write_l(StringTemp,x,y,n); }
static void famewin_write_right(Buffer *b,int x,int y,int w) { int n=std::strlen(StringTemp); if(n) b->write_right_l(StringTemp,x,y,w,n); }
static void famewin_write_wrapped(Buffer *b,int x,int y,int w) { b->wrap_cent(StringTemp,x,y,w); }
BODY_INIT
BODY_REDRAW
"""


def run_painter_host(tmp_path, bodies, *, should_pass: bool, absent=False) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = PAINTER_HARNESS
    for token, body in zip(("BODY_INIT", "BODY_REDRAW"), bodies):
        harness = harness.replace(token, body)
    if absent:
        harness = harness.replace("int main() {", "#define ABSENT_RECORD_FIXTURE 1\nint main() {")
    harness += r"""
int main() {
  FameWin win;
  assert(win.init() == 0);
  assert(init_x == 0 && init_y == 0 && init_w == 0x400 && init_h == 0x300);
  assert(sprite_extracts == 12 && button_inits == 14);
  win.buttonGroup_.field_84_ = 1;
  win.selectedRecord_ = 0;
  win.field_B42C_=30; win.field_B430_=270; win.field_B434_=291; win.field_B438_=327;
  win.field_B43C_=303; win.field_B440_=269; win.field_B444_=541; win.field_B448_=387;
  win.field_B44C_=552; win.field_B450_=269; win.field_B454_=762; win.field_B458_=543;
  win.field_B45C_=21; win.field_B460_=123; win.field_B464_=771; win.field_B468_=237;
  win.field_B46C_=76; win.field_B470_=39; win.field_B474_=724; win.field_B478_=119;
  win.field_B47C_=296; win.field_B480_=391; win.field_B484_=548; win.field_B488_=584;
  win.field_A14_=119; win.field_A18_=80; win.field_A1C_=800; win.field_A20_=600;
  win.field_A24_=119; win.field_A28_=80; win.field_A2C_=919; win.field_A30_=680;
  win.field_A44_=12; win.field_A48_=1;
  win.clip_ready=false;
  HallOfFameRecords[0].present=1; HallOfFameRecords[0].award_flags=5;
  HallOfFameRecords[2].present=1;
  HallOfFameRecords[4].present=1;
#ifdef ABSENT_RECORD_FIXTURE
  HallOfFameRecords[0].present=0;
#endif
  HallOfFameRecords[0].field_20=2; HallOfFameRecords[0].field_21=2;
  HallOfFameRecords[0].field_14=14; HallOfFameRecords[0].field_1C=19;
  HallOfFameRecords[0].leader_two=12; HallOfFameRecords[0].field_18=18;
  HallOfFameRecords[0].leader_one=11; HallOfFameRecords[0].rating=75;
  HallOfFameRecords[0].year=2100;
  const int loads_before_redraw=pcx_loads;
  win.on_redraw();
  assert(pcx_loads == loads_before_redraw +
#ifdef ABSENT_RECORD_FIXTURE
         0
#else
         1
#endif
  );
#ifdef ABSENT_RECORD_FIXTURE
  assert(win.flatButtons2_[0].page == -1 && win.flatButtons2_[1].page == -1 &&
         win.flatButtons2_[2].page == -1);
  assert(wrap_calls == 0 && fill_calls == 0 && sprite_draws == 0);
  assert(copy_calls == 2 && stale_clip_copies == 0 && text_width_calls == 0);
  assert(uppercase_difficulty_writes == 0 && lowercase_difficulty_writes == 0);
  assert(spot_adds == 2 && win.spot_.ids[0] == 2 && win.spot_.ids[1] == 4);
  assert(!win.clip_ready);
#else
  assert(win.flatButtons2_[0].page == 2 && win.flatButtons2_[1].page == -1 &&
         win.flatButtons2_[2].page == 2);
  assert(wrap_calls == 2 && wrapped_label_974 == 1 && wrapped_label_976 == 1);
  assert(fill_calls == 1 && sprite_draws == 1);
  assert(copy_calls == 3 && stale_clip_copies == 0 && text_width_calls == 2);
  assert(uppercase_difficulty_writes == 1 && lowercase_difficulty_writes == 0);
  assert(win.clip_ready);
  assert(spot_adds == 3 && win.spot_.ids[0] == 0 &&
         win.spot_.ids[1] == 2 && win.spot_.ids[2] == 4 && center_calls >= 8);
#endif
  return 0;
}
"""
    executable = tmp_path / "famewin-painter-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) is should_pass, result.stderr.decode()


def test_famewin_actual_initializer_and_painter_paths(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    # Adapt only pointer-to-32-bit-int conversions for the 64-bit host harness;
    # the original 32-bit source ABI and real string reads remain unchanged.
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace(
        "reinterpret_cast<int>(Difficulty[record->field_20])", "0"
    )
    run_painter_host(tmp_path, (init, redraw), should_pass=True)


def test_famewin_absent_record_skips_details_and_preserves_margin_clip(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    run_painter_host(tmp_path, (init, redraw), should_pass=True, absent=True)


def test_famewin_damage_control_detects_repeated_award_banner(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    banner_break = "      break;\n    }\n    canvas->set_font(ReportIfGlobal.field_1E190_"
    assert banner_break in redraw
    mutant = redraw.replace(banner_break, "    }\n    canvas->set_font(ReportIfGlobal.field_1E190_", 1)
    run_painter_host(tmp_path, (init, mutant), should_pass=False)


def test_famewin_damage_control_detects_blit_with_stale_clip(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    initial_restore = "  Buffer *canvas = static_cast<Buffer *>(this);\n  canvas->restore_clip();"
    assert initial_restore in redraw
    mutant = redraw.replace(initial_restore,
                            "  Buffer *canvas = static_cast<Buffer *>(this);", 1)
    run_painter_host(tmp_path, (init, mutant), should_pass=False)


def test_famewin_damage_control_detects_lost_text_width_call(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    fill_extent = "const int fill_extent = canvas->text_width(StringTemp) + 0x14;"
    assert fill_extent in redraw
    mutant = redraw.replace(fill_extent, "const int fill_extent = 0x41;", 1)
    run_painter_host(tmp_path, (init, mutant), should_pass=False)


def test_famewin_damage_control_detects_lost_difficulty_uppercase(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    difficulty_uppercase = (
        "strcat(StringTemp, StringTable.get(\n"
        "        reinterpret_cast<int>(Difficulty[record->field_20])));\n"
        "    CharUpperA(StringTemp);"
    )
    assert difficulty_uppercase in redraw
    mutant = redraw.replace(difficulty_uppercase,
                            difficulty_uppercase.rsplit("\n", 1)[0], 1)
    mutant = mutant.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    run_painter_host(tmp_path, (init, mutant), should_pass=False)


def test_famewin_damage_control_detects_blank_row_hit_targets(tmp_path):
    init = source_body(INIT)
    redraw = source_body(REDRAW)
    redraw = redraw.replace(
        "reinterpret_cast<int>(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name)", "0"
    ).replace("reinterpret_cast<int>(Difficulty[record->field_20])", "0")
    guarded_add = (
        "      if ((entry.present & 1) != 0) {\n"
        "        spot_.add(rank, 0, row.left, row.top, width, row.bottom - row.top);\n"
        "      }"
    )
    assert guarded_add in redraw
    # Pinned 0x448d90 disassembly tests the present bit at 0x44A20D and
    # branches around the row paint/hit-target path when it is clear. The loop
    # still advances rank and the row's record pointer at 0x44A46E-0x44A481.
    mutant = redraw.replace(guarded_add,
                            "      spot_.add(rank, 0, row.left, row.top, width, row.bottom - row.top);", 1)
    run_painter_host(tmp_path, (init, mutant), should_pass=False)
