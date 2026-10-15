"""Behavior controls extracted from the current CouncWin production bodies.

These host tests check UI call order and the state read after owned callbacks;
they do not claim original ABI, renderer behavior, or game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/councwin.cpp"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated production function: {signature}")


def adapt_body(body: str) -> str:
    """Bind current typed owner expressions to the fixture's scalar doubles."""
    return (body
        .replace("GameDataStateStorage.fields_.lock_masks_", "LockMasks")
        .replace("GameDataStateStorage.fields_.governor_faction_", "GovernorFaction")
        .replace("GameDataStateStorage.fields_.turn_current_num_", "TurnCurrentNum")
        .replace("GameDataStateStorage.fields_.game_more_preferences_", "GameMorePreferences")
        .replace("GameDataStateStorage.fields_.council_last_resolution_turn_[proposal]", "CouncilLastResolutionTurn[proposal]"))


def adapt_harness(harness: str) -> str:
    """Use the production mask union in host fixtures while retaining their scalar alias."""
    combined = "uint32_t LockEnableMask = 0, GameMorePreferences = 0;"
    if combined in harness:
        return harness.replace(
            combined,
            "struct GameDataLockMaskBytes { uint8_t human_mask_, alive_mask_, unused_[2]; };\n"
            "union GameDataLockMaskView { uint32_t enable_mask_; GameDataLockMaskBytes bytes_; };\n"
            "GameDataLockMaskView LockMasks{};\n"
            "#define LockEnableMask LockMasks.enable_mask_\n"
            "uint32_t GameMorePreferences = 0;",
        )
    declaration = r"uint32_t LockEnableMask(?:\s*=\s*([^;]+))?;"
    matches = list(re.finditer(declaration, harness))
    if not matches:
        return harness
    assert len(matches) == 1, f"expected one LockEnableMask declaration, got {len(matches)}"
    initial = matches[0].group(1)
    initializer = f" = {{ {initial} }}" if initial is not None else "{}"
    owner = (
        "struct GameDataLockMaskBytes { uint8_t human_mask_, alive_mask_, unused_[2]; };\n"
        "union GameDataLockMaskView { uint32_t enable_mask_; GameDataLockMaskBytes bytes_; };\n"
        f"GameDataLockMaskView LockMasks{initializer};\n"
        "#define LockEnableMask LockMasks.enable_mask_"
    )
    harness = harness[:matches[0].start()] + owner + harness[matches[0].end():]
    return harness


REDRAW_HARNESS = r"""
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using LONG = int32_t;
struct RECT { LONG left, top, right, bottom; };
struct BoxSpriteParams {};
struct Font {};
struct DibHeader { int biWidth, biHeight; };
struct Dib { DibHeader bmiHeader; };
std::vector<std::string> events;
struct Buffer {
  Dib dib_{{640, -480}};
  void restore_clip() { events.push_back("clip"); }
  int box_sprite(RECT *, BoxSpriteParams *) { events.push_back("box"); return 0; }
  void set_text_color(int color, int, int, int) { events.push_back("color:" + std::to_string(color)); }
  void set_font(Font *, int, int, int) { events.push_back("font"); }
  int wrap_cent(char *text, RECT *) { events.push_back(std::string("wrap:") + text); return 0; }
};
struct MainInterface : Buffer {
  RECT panel{10, 20, 310, 220};
  const RECT &info_frame_rect() const { return panel; }
  void restore_back(int) { events.push_back("restore-back"); }
} MainInterfaceGlobal;
struct Player { int is_leader_female; char name_leader[24]; } Players[8];
int ChatItemIdTable[8] = {249, 111, 252, 255, 251, 65, 102, 187};
int GovernorFaction = -1;
int GenderDefault = 0;
int PluralityDefault = 0;
const int MaxPlayerNum = 8;
char StringTemp[1032];
BoxSpriteParams InterfaceBoxStyles[51];
const char *label_get(int index) {
  switch (index) {
  case 505: return "VETO";
  case 504: return "RESULT";
  case 513: return "PROPOSAL";
  case 900: return "AGENDA";
  case 509: return "label";
  case 127: return "TITLE";
  default: return "LABEL";
  }
}
char *uppercase(char *text) {
  for (char *p = text; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
  events.push_back("uppercase");
  return text;
}
char *(*string_upper_slot())(char *) { return &uppercase; }
struct CouncWin : Buffer {
  uint32_t field_A1C_ = 0, field_A20_ = 0, field_A24_ = 0, field_A28_ = 0;
  uint32_t field_BA4_ = 1, field_BA8_ = 2, field_BAC_ = 3, field_BB0_ = 4;
  Font font6_;
  void draw_info() { events.push_back("info"); }
  void draw_leader(int faction) { events.push_back("leader:" + std::to_string(faction)); }
  void on_redraw();
};
BODY
int main() {
  CouncWin win;
  std::strcpy(Players[2].name_leader, "leader");
  Players[2].is_leader_female = 1;
  win.field_A1C_ = 0;
  win.field_A20_ = 2;
  win.field_A24_ = 1;
  win.field_A28_ = 0;
  win.on_redraw();
  assert(events[0] == "clip");
  assert(events[1] == "clip");
  assert(events[2] == "restore-back");
  assert(events[3] == "box" && events[4] == "box" && events[5] == "box");
  assert(events[6] == "info");
  for (int i = 0; i < 7; ++i) assert(events[7 + i] == "leader:" + std::to_string(i + 1));
  assert(events[14] == "box");
  assert(GenderDefault == 1 && PluralityDefault == 0);
  assert(events.back() == "wrap:LEADER LABEL");
}
"""


def test_on_redraw_uses_live_typed_owner_state_and_preserves_draw_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "void CouncWin::on_redraw()"))
    body = body.replace("CharUpperA", "uppercase")
    harness = adapt_harness(REDRAW_HARNESS.replace("BODY", body))
    executable = tmp_path / "councwin-redraw"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


SET_VOTE_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <vector>
const int MaxPlayerNum = 8;
uint32_t LockEnableMask = 0;
int IsMultiplayerNet = 1;
int IsMultiplayerPBEM = 0;
struct LockState { uint32_t dword_[8]; uint32_t leading_words_[2]{}; };
struct NetDaemon { LockState channels_[2]; struct Primary { uint32_t leading_words_[3], flags_, trailing_words_[10]; } primary_state_; } NetDaemonNet;
struct { int faction_id_; } ConsoleGlobal;
std::vector<int> events;
int council_get_vote(int faction, int proposal, int target) {
  assert(proposal == 1 && target == 3);
  events.push_back(100 + faction);
  if (events.size() == 1) LockEnableMask |= (1u << 1);
  return -2;
}
uint32_t message_data(int, int, int, int, int, int) { events.push_back(700); return 0; }
struct GraphicWin { void redraw() { events.push_back(600); } };
struct Button {
  int id;
  void hide() { events.push_back(800 + id); }
  void show(int mode) { events.push_back(900 + id + mode); }
};
struct CouncWin : GraphicWin {
  uint32_t field_A1C_ = 1, field_A20_ = 3, field_A24_ = 0;
  uint32_t leader_label_kind_[8]{};
  uint32_t leader_tech_[8]{};
  Button buttons_[6]{{0},{1},{2},{3},{4},{5}};
  int resolutions = 0;
  int make_resolution(int mode) { events.push_back(500 + mode); ++resolutions; return 0; }
  void set_vote(int faction, int vote);
};
BODY
int main() {
  CouncWin win;
  win.field_A24_ = 1;
  win.set_vote(0, -1);
  assert(win.leader_label_kind_[0] == 0 && events.empty());

  win.field_A24_ = 0;
  LockEnableMask = (1u << 8) | (1u << 9); // factions 0 and 1 alive; initially no humans
  win.set_vote(2, -1);
  assert(win.leader_label_kind_[2] == static_cast<uint32_t>(-1));
  assert(events.size() == 4);
  assert(events[0] == 100 && events[1] == 100 && events[2] == 100);
  assert(events[3] == 600); // redraw after the three live-mask rounds
  assert(win.resolutions == 0);
}
"""


def test_set_vote_rereads_lock_masks_after_ai_vote_callbacks(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "void CouncWin::set_vote(int faction_id, int vote)"))
    harness = adapt_harness(SET_VOTE_HARNESS.replace("BODY", body))
    executable = tmp_path / "councwin-set-vote"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


KEY_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
const int MaxPlayerNum = 8;
uint32_t LockEnableMask = 0;
uint32_t GameMorePreferences = 0;
int IsMultiplayerPBEM = 0, IsMultiplayerNet = 0;
int TurnRedrawPending = 1, CouncilDialogActive = 1, CouncilPbemGate = 0;
struct { int faction_id_; } ConsoleGlobal;
std::vector<std::string> events;
int council_get_vote(int faction, int proposal, int target) {
  assert(proposal == 3 && target == 5);
  events.push_back("vote:" + std::to_string(faction));
  if (faction == 1) LockEnableMask |= 1u << 3;
  return -2;
}
void council_action(int proposal, int faction) {
  assert(proposal == 3 && faction == 5);
  events.push_back("action");
}
void do_all_draws() { events.push_back("draws"); }
struct Font { int id; void close() { events.push_back("font:" + std::to_string(id)); } };
struct Button { int id; void hide() { events.push_back("button:" + std::to_string(id)); } };
struct CouncWin {
  uint32_t field_A1C_ = 3, field_A20_ = 5, field_A24_ = 0;
  uint32_t leader_label_kind_[8]{};
  Button buttons_[6]{{0},{1},{2},{3},{4},{5}};
  Font font1_{1}, font2_{2}, font3_{3}, font4_{4}, font5_{5};
  void release_modal() { events.push_back("release"); }
  int on_key_click(int, int);
};
BODY
int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_ = 0;
  LockEnableMask = (1u << 0) | (1u << 2);
  assert(win.on_key_click(0, 13) == 1);
  assert(events.size() == 5);
  assert(events[0] == "vote:1" && events[1] == "vote:4" &&
         events[2] == "vote:5" && events[3] == "vote:6" && events[4] == "vote:7");
  assert(win.leader_label_kind_[1] == static_cast<uint32_t>(-2));
  assert(win.leader_label_kind_[3] == 0); // callback changed the live human mask
  assert(TurnRedrawPending == 1 && CouncilDialogActive == 1);

  events.clear();
  win.field_A24_ = 1;
  assert(win.on_key_click(0, 27) == 1);
  assert(events.size() == 14);
  assert(events[0] == "release" && events[1] == "button:0" &&
         events[6] == "button:5" && events[7] == "font:1" &&
         events[12] == "draws" && events[13] == "action");
  assert(TurnRedrawPending == 0 && CouncilDialogActive == 0);
}
"""


def test_on_key_click_fills_from_live_lock_state_and_closes_in_native_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "int CouncWin::on_key_click(int, int key)"))
    harness = adapt_harness(KEY_HARNESS.replace("BODY", body))
    executable = tmp_path / "councwin-key-click"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


DRAW_INFO_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
using LONG = int32_t;
struct RECT { LONG left, top, right, bottom; };
struct BoxSpriteParams {};
struct Font { int unk_1_ = 0, is_fot_set_ = 0, font_obj_ = 0, line_height_ = 12, height_ = 10; int line_advance() const { return line_height_; } };
std::vector<std::string> events;
char *CharUpperA(char *text) { for (char *p=text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A'; return text; }
struct Buffer {
  void set_clip(RECT *) { events.push_back("clip"); }
  int box_sprite(RECT *, BoxSpriteParams *) { events.push_back("box"); return 0; }
  void set_font(Font *, int, int, int) { events.push_back("font"); }
  void set_text_color(int color, int, int, int) { events.push_back("color:" + std::to_string(color)); }
  int write_cent_l(char *s, RECT *, int) { events.push_back(std::string("center:") + s); return 0; }
  int wrap_height(char *, int) { return 14; }
  int wrap_cent(char *s, RECT *) { events.push_back(std::string("wrap:") + s); return 0; }
  int write_l(char *s, RECT *, int) { events.push_back(std::string("left:") + s); return 0; }
  int write_right_l(char *s, RECT *, int) { events.push_back(std::string("right:") + s); return 0; }
};
struct GraphicWin { void soft_update(RECT *) { events.push_back("update"); } };
struct MainInterface : Buffer, GraphicWin {
  RECT clip{0,0,640,480};
  RECT *buffer_working_rect() { return &clip; }
} MainInterfaceGlobal;
struct Player { int is_leader_female = 0; char name_leader[24]{}; int rule_flags = 0; } Players[8];
int ChatItemIdTable[8] = {0, 11, 22, 33, 44, 55, 66, 77};
int GenderDefault = 0, PluralityDefault = 0, WinScreenWidth = 1024;
const int MaxPlayerNum = 8, RFLAG_ALIEN = 0x80;
char StringTemp[1032];
BoxSpriteParams InterfaceBoxStyles[51];
struct RulesProposal { char *name; char *description; int preq_tech; };
RulesProposal Proposal[11];
struct Strings { char *get(int handle) { return handle == 1 ? const_cast<char *>("PROPOSAL") : const_cast<char *>("DESCRIPTION"); } char *get(char *value) { return value == reinterpret_cast<char *>(1) ? const_cast<char *>("PROPOSAL") : const_cast<char *>("DESCRIPTION"); } } StringTable;
const char *label_get(int index) {
  if (index == 498) return "COUNCIL";
  if (index == 495) return "FOR";
  if (index == 496) return "AGAINST";
  if (index == 900) return "OFFICE";
  return "LABEL";
}
char *upper(char *s) { return s; }
char *(*string_upper_slot())(char *) { return &upper; }
int eligible(int faction) { return faction == 1 || faction == 2; }
void say_num(int n) { strcat(StringTemp, std::to_string(n).c_str()); }
struct CouncWin {
  uint32_t field_A1C_ = 2, field_A20_ = 1;
  uint32_t leader_label_kind_[8]{};
  int leader_tech_[8]{};
  uint32_t field_BB4_ = 2, field_BB8_ = 3, field_BBC_ = 302, field_BC0_ = 203;
  Font font2_;
  void draw_info();
};
BODY
int main() {
  CouncWin win;
  Proposal[2].name = reinterpret_cast<char *>(1);
  Proposal[2].description = reinterpret_cast<char *>(2);
  Players[3].rule_flags = RFLAG_ALIEN;
  win.leader_label_kind_[1] = static_cast<uint32_t>(-1);
  win.leader_label_kind_[2] = static_cast<uint32_t>(-2);
  win.leader_label_kind_[3] = static_cast<uint32_t>(-1);
  win.leader_label_kind_[4] = static_cast<uint32_t>(-1);
  win.draw_info();
  bool saw_for = false, saw_against = false;
  for (const std::string &event : events) {
    if (event == "left:FOR: 2") saw_for = true;
    if (event == "right:AGAINST: 1") saw_against = true;
  }
  assert(saw_for && saw_against);
  assert(events.front() == "clip" && events.back() == "update");

  events.clear();
  win.field_A1C_ = 0;
  std::strcpy(Players[1].name_leader, "ONE");
  std::strcpy(Players[2].name_leader, "TWO");
  for (int faction = 1; faction < 8; ++faction) win.leader_tech_[faction] = 1;
  win.leader_label_kind_[4] = 1;
  win.leader_label_kind_[5] = 2;
  win.draw_info();
  bool saw_one = false, saw_two = false;
  int clips = 0;
  for (const std::string &event : events) {
    if (event == "left:ONE: 1") saw_one = true;
    if (event == "right:TWO: 1") saw_two = true;
    if (event == "clip") ++clips;
  }
  assert(saw_one && saw_two && clips == 2);
}
"""


def test_draw_info_counts_votes_from_live_typed_faction_records(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "void CouncWin::draw_info()"))
    harness = adapt_harness(DRAW_INFO_HARNESS.replace("BODY", body))
    executable = tmp_path / "councwin-draw-info"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


RESOLUTION_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
const int MaxPlayerNum = 8, RFLAG_ALIEN = 0x80;
uint32_t LockEnableMask = 0;
int IsMultiplayerPBEM = 0, IsMultiplayerNet = 0;
int GovernorFaction = 1, TurnCurrentNum = 9;
int CouncilPbemGate = 1, CouncilLastResolutionTurn[11]{};
int CouncilNotifyFormSlots[8]{}, CouncilNotifyPluralSlots[8]{};
struct { int faction_id_; } ConsoleGlobal;
std::vector<std::string> events;
struct Player { int rule_flags = 0; } Players[8];
struct PlayerData { int unk_29[11]{}; int unk_30[11]{}; } PlayersData[8];
void council_action(int proposal, int faction) {
  assert(proposal == 0 && faction == 2);
  events.push_back("action");
}
struct GraphicWin { void redraw() { events.push_back("redraw"); } };
struct Button {
  int id;
  void hide() { events.push_back("hide:" + std::to_string(id)); }
  void show(int mode) { assert(mode == 0); events.push_back("show:" + std::to_string(id)); }
};
struct CouncWin : GraphicWin {
  uint32_t field_A1C_ = 0, field_A20_ = 3, field_A24_ = 0;
  uint32_t leader_label_kind_[8]{};
  int leader_tech_[8]{};
  Button buttons_[6]{{0},{1},{2},{3},{4},{5}};
  int make_resolution(int);
};
BODY
int main() {
  CouncWin win;
  for (int i = 1; i < 8; ++i) {
    win.leader_label_kind_[i] = 2;
    win.leader_tech_[i] = i == 2 ? 8 : 1;
    PlayersData[i].unk_29[0] = 44;
    PlayersData[i].unk_30[0] = 55;
  }
  LockEnableMask = 0x0000ff00u | (1u << 2) | (1u << 3);
  ConsoleGlobal.faction_id_ = 2;
  IsMultiplayerPBEM = 1;
  IsMultiplayerNet = 1;
  assert(win.make_resolution(0) == 1);
  assert(events.empty() && CouncilPbemGate == 1); // tally-only call has no side effects

  assert(win.make_resolution(1) == 1);
  assert(win.field_A20_ == 2 && win.field_A24_ == 1);
  assert(GovernorFaction == 2 && CouncilPbemGate == 0);
  assert(CouncilLastResolutionTurn[0] == 9);
  assert(CouncilNotifyFormSlots[2] == 0 && CouncilNotifyPluralSlots[2] == 1);
  for (int i = 1; i < 8; ++i) {
    assert(PlayersData[i].unk_29[0] == 0 && PlayersData[i].unk_30[0] == 0);
  }
  assert(events.size() == 15);
  assert(events[0] == "redraw" && events[1] == "hide:0" && events[2] == "show:5");
  assert(events[11] == "hide:5" && events[12] == "show:5");
  assert(events[13] == "redraw" && events[14] == "action");
}
"""


def test_make_resolution_retains_tally_only_and_resolution_side_effect_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "int CouncWin::make_resolution(int resolve)"))
    harness = adapt_harness(RESOLUTION_HARNESS.replace("BODY", body))
    executable = tmp_path / "councwin-resolution"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


INIT_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using LONG = int32_t;
struct RECT { LONG left, top, right, bottom; };
std::vector<std::string> events;
struct Font { int unk_1_ = 0; void close() { events.push_back("font-close"); }
  int init(const char *, int height, int) { events.push_back("font:" + std::to_string(height)); return 1; } };
struct Sprite {};
struct FactionArt { Sprite council_logo[2], council_small, council_big, diplomacy; };
const int MaxPlayerNum = 8;
const int RFLAG_ALIEN = 0x80;
int WinScreenWidth = 800, WinScreenHeight = 600;
int IsMultiplayerNet = 0, IsMultiplayerPBEM = 0;
int TurnCurrentNum = 0, DiploFriction = 0;
int DiploScratchFA60 = 0, DiploScratchF800 = -1, DiploScratchF80C = -1;
uint32_t LockEnableMask = (1u << 1) | (1u << 8) | (1u << 9);
int GovernorFaction = 3, TurnRedrawPending = 0, CouncilDialogActive = 0;
int CouncilPbemGate = 0;
int GenderDefault = 0, PluralityDefault = 0;
int eligible_candidate = -1, eligible_calls = 0, ai_vote_override = -1;
int mutate_alive_on_eligible_faction = -1;
uint8_t alive_mask_after_eligible = 0;
int split_ai_votes = 0, vote_weight_override = -1;
int popup_result = 1, mutate_scratch_after_popup = 0;
int friction_after_diplomacy_check = -1, invalid_tech_result = 0;
int eligible(int faction) {
  events.push_back("eligible:" + std::to_string(faction));
  ++eligible_calls;
  if (faction == mutate_alive_on_eligible_faction) {
    LockEnableMask = (LockEnableMask & ~0x0000ff00u) |
                     (static_cast<uint32_t>(alive_mask_after_eligible) << 8);
    mutate_alive_on_eligible_faction = -1;
  }
  return eligible_calls == 1 ? 0 : faction == eligible_candidate;
}
void diplomacy_check(int faction, int other, int update) {
  events.push_back("diplomacy:" + std::to_string(faction) + ":" + std::to_string(other) + ":" + std::to_string(update));
  if (friction_after_diplomacy_check >= 0) DiploFriction = friction_after_diplomacy_check;
}
void diplomacy_caption(int faction, int other) {
  events.push_back("caption:" + std::to_string(faction) + ":" + std::to_string(other));
}
void tech_analysis(int faction, int other) {
  events.push_back("tech-analysis:" + std::to_string(faction) + ":" + std::to_string(other));
  DiploScratchF800 = invalid_tech_result ? -1 : 21;
  DiploScratchF80C = 22;
}
void tech_achieved(int faction, int tech, int other, int freebie) {
  events.push_back("tech:" + std::to_string(faction) + ":" + std::to_string(tech) + ":" + std::to_string(other) + ":" + std::to_string(freebie));
  if (tech == 31) DiploScratchF80C = 33;
}
void parse_num(int id, int value) { events.push_back("number:" + std::to_string(id) + ":" + std::to_string(value)); }
void parse_says(int id, const char *text, int, int) {
  events.push_back("parse:" + std::to_string(id) + ":" + text);
}
void say_tech(char *output, int tech, int category) {
  events.push_back("say-tech:" + std::to_string(tech) + ":" + std::to_string(category));
  std::strcpy(output, tech == 21 ? "TECH21" : "TECH22");
}
void clock_wait(int milliseconds) { events.push_back("wait:" + std::to_string(milliseconds)); }
int X_pops(const char *label, int value, Sprite *, int (*)()) {
  events.push_back(std::string("xpop:") + label + ":" + std::to_string(value));
  if (mutate_scratch_after_popup) {
    DiploScratchF800 = 31;
    DiploScratchF80C = 32;
    mutate_scratch_after_popup = 0;
  }
  return popup_result;
}
int council_votes(int faction) {
  events.push_back("votes:" + std::to_string(faction));
  return vote_weight_override >= 0 ? vote_weight_override : faction + 10;
}
int council_get_vote(int faction, int proposal, int target) {
  (void)proposal;
  (void)target;
  events.push_back("vote:" + std::to_string(faction));
  if (split_ai_votes) return faction <= 3 ? 6 : 5;
  return ai_vote_override >= 0 ? ai_vote_override : faction - 1;
}
int bit_count(int bits) { int n = 0; for (; bits; bits &= bits - 1) ++n; return n; }
uint32_t message_data(int, int, int, int, int, int) { events.push_back("message"); return 0; }
unsigned long timeGetTime() { return 1234; }
const char *DefaultLargeFontFace = "font";
char StringTemp[1032];
char *label_get(int id) { return const_cast<char *>(id == 495 ? "vote" : id == 496 ? "other" : id == 497 ? "third" : id == 101 ? "back" : "label"); }
char *CharUpperA(char *p) { for (char *c=p; *c; ++c) if (*c >= 'a' && *c <= 'z') *c -= 'a' - 'A'; return p; }
struct Win { virtual ~Win() {} };
struct Buffer {
  int set_font(Font *, Font *, Font *, Font *) { events.push_back("local-font"); return 1; }
};
struct GraphicWin : Win {
  int init(int x, int y, int w, int h, char *, int, Win *parent, void *, void *) {
    assert(x == 10 && y == 105 && w == 0x32c && h == 0x18b && parent != nullptr);
    events.push_back("window-init"); return 1;
  }
  void fill(int color) { assert(color == 9); events.push_back("fill"); }
  void redraw() { events.push_back("button-redraw"); }
};
struct MainInterface : GraphicWin, Buffer {
  uint32_t field_D7C_ = 100;
  RECT info{10, 20, 410, 320};
  const RECT &info_frame_rect() const { return info; }
} MainInterfaceGlobal;
struct MenuWindow {
  bool visible = true;
  int is_visible() const { return visible; }
  void hide() { visible = false; events.push_back("alpha-hide"); }
  void show() { visible = true; events.push_back("alpha-show"); }
};
struct Console : Win { int faction_id_ = 2; MenuWindow alpha_menu_; } ConsoleGlobal;
struct MultiWin : Win {
  bool visible = true;
  int is_visible() const { return visible; }
  void hide() { visible = false; events.push_back("multi-hide"); }
  void show() { visible = true; events.push_back("multi-show"); }
} MultiWindow;
struct SubInterface {
  protected: uint32_t field_4_ = 0;
  public: void release_iface_mode() { events.push_back("release-iface"); }
};
struct Spot { void init(int count) { assert(count == 8); events.push_back("spot"); } };
struct Button : GraphicWin, Buffer {
  int id = -1, x = 0, y = 0, w = 0, h = 0;
  std::string name;
  int init(const char *caption, int button_id, int left, int top, int width,
           int height, Win *parent, int) {
    assert(parent == static_cast<Win *>(&MainInterfaceGlobal));
    id=button_id; x=left; y=top; w=width; h=height; name=caption;
    events.push_back("button:" + std::to_string(id) + ":" + name); return 1;
  }
  void close() { events.push_back("button-close"); }
  void hide() { events.push_back("button-hide"); }
};
struct Player {
  uint32_t rule_flags = 0;
  int is_leader_female = 0;
  char title_leader[24] = {}, name_leader[24] = {};
} Players[8];
struct PlayerData {
  uint32_t flags = 0, diplo_treaties[8]{};
  int diff_level = 0, energy_reserves = 0;
  int16_t proto_id_lost[512]{};
} PlayersData[8];
FactionArt FactionsArt[8];
Sprite *FactionDiplomacyPortraits[8];
int pop_wait() { events.push_back("wait"); return 1; }
struct CouncWin : GraphicWin, SubInterface {
  uint32_t field_A1C_=0,field_A20_=0,field_A24_=0,field_A28_=0,field_A38_=0,field_A3C_=0,field_A40_=0,field_A44_=0;
  uint32_t field_BA4_=0,field_BA8_=0,field_BAC_=0,field_BB0_=0,field_BB4_=0,field_BB8_=0,field_BBC_=0,field_BC0_=0;
  uint32_t field_BC4_=0,field_BC8_=0,field_BCC_=0,field_BD0_=0;
  RECT leader_rects_[20]{};
  Sprite *leader_backing_[8][2]{}, *leader_portraits_[8][2]{};
  int leader_tech_[8]{}; uint32_t leader_label_kind_[8]{};
  Font font1_,font2_,font3_,font4_,font5_;
  Spot spot_; Button buttons_[6];
  int set_modal(int flag, int (*callback)(), Win *owner) {
    assert(flag == 0 && callback == pop_wait && owner == static_cast<Win *>(&MainInterfaceGlobal));
    events.push_back("modal"); return 1;
  }
  void set_vote(int faction, int vote) { events.push_back("set-vote:" + std::to_string(faction) + ":" + std::to_string(vote)); }
  void show(int flag) { assert(flag == 0); events.push_back("council-show"); }
  void init(int proposal, int faction_id);
};
BODY
int main() {
  CouncWin win;
  win.init(1, 4);
  assert(win.field_A1C_ == 1 && win.field_A20_ == 4 && win.field_A28_ == 3);
  assert(win.field_A44_ == ~uint32_t(1u << 1));
  assert(win.field_A38_ == 1234 && win.field_A40_ == 1 && win.field_A3C_ == 60000);
  assert(CouncilDialogActive == 1 && win.field_A24_ == 0 && win.font3_.unk_1_ == 5);
  assert(win.leader_tech_[4] == 14 && win.leader_label_kind_[2] == static_cast<uint32_t>(-5));
  assert(win.leader_backing_[4][0] == &FactionsArt[4].council_logo[0]);
  assert(win.leader_backing_[4][1] == &FactionsArt[4].council_logo[1]);
  assert(win.leader_portraits_[4][0] == &FactionsArt[4].council_small);
  assert(win.leader_portraits_[4][1] == &FactionsArt[4].council_big);
  assert(win.buttons_[1].id == 1 && win.buttons_[1].name == "VOTE");
  assert(win.buttons_[2].id == 2 && win.buttons_[2].name == "OTHER");
  assert(win.buttons_[3].id == 3 && win.buttons_[3].name == "THIRD");
  assert(win.buttons_[5].id == 5 && win.buttons_[5].name == "BACK");
  assert(win.buttons_[1].x == win.leader_rects_[16].left);
  assert(MultiWindow.visible && ConsoleGlobal.alpha_menu_.visible);
  assert(events[0] == "button-close" && events[6] == "font-close" && events[11] == "spot");
  assert(events[17] == "local-font");
  assert(events[events.size()-4] == "modal" && events[events.size()-3] == "release-iface" &&
         events[events.size()-2] == "alpha-show" && events.back() == "multi-show");
  int vote_calls=0;
  for (const std::string &event : events) if (event.find("vote:") == 0) ++vote_calls;
  assert(vote_calls == 24);

  events.clear();
  IsMultiplayerPBEM = 1;
  IsMultiplayerNet = 1;
  CouncilPbemGate = 2;
  CouncWin pbem;
  pbem.init(0, 2);
  assert(pbem.field_A44_ == 0xffu && CouncilPbemGate == 2);
  for (int faction = 0; faction < MaxPlayerNum; ++faction) {
    assert(pbem.leader_tech_[faction] == 0 && pbem.leader_label_kind_[faction] == 0);
  }
  bool sent_message = false, chose_proposal_button = false;
  for (const std::string &event : events) {
    if (event == "message") sent_message = true;
    if (event == "button:4:LABEL") chose_proposal_button = true;
  }
  assert(sent_message && chose_proposal_button);

  events.clear();
  IsMultiplayerPBEM = 0;
  IsMultiplayerNet = 0;
  LockEnableMask = (0xffu << 8) | (1u << 1);
  eligible_candidate = 6;
  eligible_calls = 0;
  PlayersData[6].diff_level = 1;
  for (int faction = 0; faction < MaxPlayerNum; ++faction) {
    FactionDiplomacyPortraits[faction] = &FactionsArt[faction].diplomacy;
  }
  std::strcpy(Players[6].title_leader, "candidate-title");
  std::strcpy(Players[6].name_leader, "candidate-name");
  DiploScratchFA60 = 0;
  DiploFriction = 0;
  popup_result = 1;
  mutate_scratch_after_popup = 1;
  split_ai_votes = 1; vote_weight_override = 1;
  Players[7].rule_flags = RFLAG_ALIEN;
  ConsoleGlobal.faction_id_ = 6;
  PlayersData[6].diff_level = 1;
  CouncWin ai;
  ai.init(0, 6);
  bool checked_selected_ai = false, offered_tech = false;
  bool granted_live_first = false, granted_live_second = false, voted_for_ai = false;
  bool parsed_candidate_title = false, parsed_candidate_name = false;
  for (const std::string &event : events) {
    if (event == "diplomacy:6:6:0") checked_selected_ai = true;
    if (event == "xpop:VOTEFORMETECH:0") offered_tech = true;
    if (event == "tech:6:31:6:0") granted_live_first = true;
    if (event == "tech:6:33:6:0") granted_live_second = true;
    if (event == "set-vote:6:6") voted_for_ai = true;
    if (event == "parse:0:candidate-title" && offered_tech == false) parsed_candidate_title = true;
    if (event == "parse:1:candidate-name") parsed_candidate_name = true;
  }
  if (!(checked_selected_ai && offered_tech && granted_live_first &&
        granted_live_second && voted_for_ai && parsed_candidate_title &&
        parsed_candidate_name)) {
    std::cerr << "init callback diagnostics: " << checked_selected_ai << offered_tech
              << granted_live_first << granted_live_second << voted_for_ai
              << parsed_candidate_title << parsed_candidate_name << "\n";
    for (const auto &event : events) std::cerr << event << "\n";
  }
  assert(checked_selected_ai && offered_tech && granted_live_first &&
         granted_live_second && voted_for_ai && parsed_candidate_title &&
         parsed_candidate_name);
  assert(events[events.size()-4] == "modal" &&
         events[events.size()-3] == "release-iface" &&
         events[events.size()-2] == "alpha-show" && events.back() == "multi-show");

  events.clear();
  eligible_calls = 0;
  PlayersData[6].flags = 100;
  PlayersData[6].proto_id_lost[0xf2] = 25;
  PlayersData[6].energy_reserves = 200;
  PlayersData[4].diff_level = 1;
  PlayersData[4].energy_reserves = 3;
  ConsoleGlobal.faction_id_ = 4;
  split_ai_votes = 1; vote_weight_override = 1;
  Players[7].rule_flags = RFLAG_ALIEN;
  std::strcpy(Players[4].title_leader, "local-title");
  std::strcpy(Players[4].name_leader, "local-name");
  CouncWin money;
  money.init(0, 4);
  bool offered_money = false, transferred_money = false;
  bool parsed_local_title = false, parsed_local_name = false;
  bool parsed_target_title = false, parsed_target_name = false;
  for (const std::string &event : events) {
    if (event == "xpop:VOTEFORME:0") offered_money = true;
    if (event == "set-vote:4:6") transferred_money = true;
    if (event == "parse:0:local-title") parsed_local_title = true;
    if (event == "parse:1:local-name") parsed_local_name = true;
    if (event == "parse:2:candidate-title") parsed_target_title = true;
    if (event == "parse:3:candidate-name") parsed_target_name = true;
    assert(event != "tech-analysis:6:6");
  }
  if (!(offered_money && transferred_money && parsed_local_title &&
        parsed_local_name && parsed_target_title && parsed_target_name)) {
    std::cerr << "money callback diagnostics: " << offered_money << transferred_money
              << parsed_local_title << parsed_local_name << parsed_target_title
              << parsed_target_name << "\n";
    for (const auto &event : events) std::cerr << event << "\n";
  }
  assert(offered_money && transferred_money && parsed_local_title &&
         parsed_local_name && parsed_target_title && parsed_target_name);
  assert(PlayersData[6].energy_reserves == 125 && PlayersData[4].energy_reserves == 78);

  events.clear();
  eligible_calls = 0;
  PlayersData[6].energy_reserves = 200;
  PlayersData[4].energy_reserves = 3;
  ConsoleGlobal.faction_id_ = 4;
  popup_result = 0;
  CouncWin refused_money;
  refused_money.init(0, 4);
  bool showed_money_without_transfer = false;
  for (const std::string &event : events) {
    if (event == "xpop:VOTEFORME:0") showed_money_without_transfer = true;
    assert(event != "set-vote:4:6");
  }
  assert(showed_money_without_transfer && PlayersData[6].energy_reserves == 200 &&
         PlayersData[4].energy_reserves == 3);

  events.clear();
  eligible_calls = 0;
  friction_after_diplomacy_check = 15;
  ConsoleGlobal.faction_id_ = 4;
  CouncWin friction_blocked;
  friction_blocked.init(0, 4);
  bool checked_then_blocked = false;
  for (const std::string &event : events) {
    if (event == "diplomacy:4:6:0") checked_then_blocked = true;
    assert(event != "caption:4:6" && event.find("xpop:") != 0 &&
           event != "tech-analysis:4:6");
  }
  if (!checked_then_blocked) for (const auto &event : events) std::cerr << event << "\n";
  assert(checked_then_blocked);

  events.clear();
  eligible_calls = 0;
  friction_after_diplomacy_check = -1;
  DiploFriction = 0;
  PlayersData[6].flags = 0;
  PlayersData[6].proto_id_lost[0xf2] = 0;
  invalid_tech_result = 1;
  ConsoleGlobal.faction_id_ = 4;
  CouncWin missing_tech;
  missing_tech.init(0, 4);
  bool analyzed_without_offer = false;
  for (const std::string &event : events) {
    if (event == "tech-analysis:4:6") analyzed_without_offer = true;
    assert(event.find("xpop:") != 0);
  }
  assert(analyzed_without_offer);

  // Candidate eligibility is a live stage: candidate 1's callback clears
  // candidate 2's alive bit, which the next native byte load must observe.
  events.clear();
  eligible_calls = 0;
  eligible_candidate = 2;
  mutate_alive_on_eligible_faction = 1;
  alive_mask_after_eligible = 1u << 1;
  IsMultiplayerNet = 0;
  IsMultiplayerPBEM = 0;
  CouncilPbemGate = 0;
  ConsoleGlobal.faction_id_ = 0;
  PlayersData[0].diff_level = 1;
  LockEnableMask = (1u << 1) | (1u << 2) | (1u << 9) | (1u << 10);
  CouncWin live_byte;
  live_byte.init(0, 0);
  bool checked_candidate_one = false;
  bool checked_candidate_two = false;
  for (const std::string &event : events) {
    checked_candidate_one |= event == "eligible:1";
    checked_candidate_two |= event == "eligible:2";
  }
  assert(checked_candidate_one && !checked_candidate_two);
  assert(mutate_alive_on_eligible_faction == -1);
}
"""


def test_init_builds_typed_council_controls_and_restores_shared_windows(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    body = adapt_body(extract_function(source, "void CouncWin::init(int proposal, int faction_id)"))
    helper = extract_function(source, "static void set_council_rect(")
    harness = adapt_harness(INIT_HARNESS.replace("BODY", helper + "\n" + body))
    executable = tmp_path / "councwin-init"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


BUTTON_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
const int MaxPlayerNum = 8, RFLAG_ALIEN = 0x80;
uint32_t LockEnableMask = 0, GameMorePreferences = 0;
int IsMultiplayerPBEM = 0, IsMultiplayerNet = 0;
int TurnRedrawPending = 1, CouncilDialogActive = 1, CouncilPbemGate = 0;
char PopupStartCaption[256];
struct { int faction_id_; } ConsoleGlobal;
std::vector<std::string> events;
int result_from_popup = -1, last_message = 0, last_payload = 0;
int sprite_init_result = 0;
bool expect_council_picture = true;
char StringTemp[1032];
struct Player { int rule_flags = 0; char name_leader[24]{}; } Players[8];
int eligible(int faction) { return faction == 1 || faction == 2; }
int council_get_vote(int faction, int, int) { events.push_back("vote:" + std::to_string(faction)); return -2; }
void council_action(int, int) { events.push_back("action"); }
uint32_t message_data(int opcode, int, int, int payload, int, int) {
  last_message = opcode; last_payload = payload; events.push_back("message-data"); return 0;
}
char *label_get(int) { return const_cast<char *>("OPTIONS"); }
void do_all_draws() { events.push_back("draws"); }
struct NetMsg { void pop(const char *text, int duration, int mode, const char *) {
  assert(duration == 5000 && mode == 1); events.push_back(std::string("toast:") + text);
} };
NetMsg net_message;
NetMsg *NetMsgPopup = &net_message;
struct Buffer { static int get_pcx_dimensions(const char *, int *w, int *h) { *w=16; *h=8; events.push_back("dimensions"); return 0; } };
struct Sprite { int init(char *, int, int) { events.push_back("sprite"); return sprite_init_result; } };
struct Popup {
  Sprite picture_;
  Sprite *caller_picture_ = nullptr;
  Popup() { events.push_back("popup-ctor"); }
  ~Popup() { events.push_back("popup-dtor"); }
  void start(char *, const char *label, int, char *, int flags, void *) { assert(label == std::string("GOVVOTE") && flags == 0x200040); events.push_back("popup-start"); }
  Sprite &owned_picture() { return picture_; }
  void select_owned_picture() { caller_picture_ = &picture_; events.push_back("select-picture"); }
  int exec(int, int (*)()) { assert((caller_picture_ == &picture_) == expect_council_picture); events.push_back("popup-exec"); return result_from_popup; }
  void close() { events.push_back("popup-close"); }
};
struct Dialogs { void item(char *text, int id) { events.push_back(std::string("item:") + text + ":" + std::to_string(id)); } };
int pop_wait() { events.push_back("wait"); return 1; }
struct Font { void close() { events.push_back("font-close"); } };
struct Button { int id; void hide() { events.push_back("hide:" + std::to_string(id)); } void show(int) {} };
struct CouncWin {
  uint32_t field_A1C_ = 2, field_A20_ = 3, field_A24_ = 0, field_A44_ = 0;
  uint32_t leader_label_kind_[8]{};
  Button buttons_[6]{{0},{1},{2},{3},{4},{5}};
  Font font1_,font2_,font3_,font4_,font5_;
  int votes[8]{};
  void set_vote(int faction, int vote) { votes[faction] = vote; events.push_back("set:" + std::to_string(vote)); }
  void release_modal() { events.push_back("release"); }
  int vslot_25(int, int key) { assert(key == 13); events.push_back("keypress"); return 1; }
  void on_button_clicked(int);
};
BODY
int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_ = 0;
  IsMultiplayerNet = 1;
  LockEnableMask = 0x00000300u | 0x00000003u;
  win.field_A44_ = 1; // one of two human factions has arrived
  win.on_button_clicked(1);
  assert(last_message == 0 && events[1] == "toast:VOTING");

  events.clear();
  win.field_A44_ = 3;
  win.on_button_clicked(2);
  assert(last_message == 0x2601 && last_payload == -2);
  assert(events[1] == "message-data" && events.back() == "popup-dtor");

  events.clear();
  result_from_popup = 0;
  sprite_init_result = 0;
  expect_council_picture = true;
  win.on_button_clicked(4);
  assert(events[1] == "popup-start" && events[2] == "dimensions" &&
         events[3] == "sprite" && events[4] == "select-picture" &&
         events[5] == "item:OPTIONS:0");
  assert(events[6] == "item::1" || events[6] == "item:ONE:1");
  assert(last_payload == -3 && events[events.size()-2] == "popup-close");

  events.clear();
  sprite_init_result = 1;
  expect_council_picture = false;
  win.on_button_clicked(4);
  assert(events[2] == "dimensions" && events[3] == "sprite" &&
         events[4] == "item:OPTIONS:0");
  assert(events[4] != "select-picture");

  events.clear();
  win.on_button_clicked(5);
  assert(events[1] == "keypress" && events.back() == "popup-dtor");
}
"""


def test_on_button_clicked_uses_typed_popup_vote_and_key_dispatch(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "void CouncWin::on_button_clicked(int button_id)"))
    fixture = BUTTON_HARNESS.replace(
        "NetMsg *NetMsgPopup = &net_message;",
        "NetMsg *NetMsgPopup = &net_message; NetMsg NetMsgPrimary;",
    )
    fixture = fixture.replace(
        "struct Popup {",
        "struct Dialogs { void item(char *text, int id) { events.push_back(std::string(\"item:\") + text + \":\" + std::to_string(id)); } };\n"
        "struct Popup { Dialogs dialogs_; Dialogs &dialogs() { return dialogs_; }",
    )
    fixture = fixture.replace(
        "struct Dialogs { void item(char *text, int id) { events.push_back(std::string(\"item:\") + text + \":\" + std::to_string(id)); } };\nint pop_wait()",
        "int pop_wait()",
    )
    fixture = fixture.replace(
        "int vslot_25(int, int key) { assert(key == 13); events.push_back(\"keypress\"); return 1; }",
        "int vslot_25(int, int key) { assert(key == 13); events.push_back(\"keypress\"); return 1; }\n"
        "int on_key_click(int, int key) { assert(key == 13); events.push_back(\"keypress\"); return 1; }",
    )
    harness = adapt_harness(fixture.replace("BODY", body))
    executable = tmp_path / "councwin-button-click"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


LEFT_CLICK_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
const int MaxPlayerNum = 8;
uint32_t LockEnableMask = 0;
int IsMultiplayerNet = 0;
int CommlinkPending = 0, CommlinkFaction = 0;
struct { int faction_id_; } ConsoleGlobal;
int GenderDefault = 0;
int PluralityDefault = 0;
char StringTemp[1032];
char PopupStartCaption[256];
std::vector<std::string> events;
int prompt_result = 0;
int mutate_after_prompt = 0;
int mutate_alive_after_prompt = 0;
uint8_t alive_mask_after_prompt = 0;
int mutate_alive_after_eligible_faction = -1;
uint8_t alive_mask_after_eligible = 0;
int mutate_local_after_prompt = -1;
int mutate_local_on_channel = -1;
int mutate_local_on_buy = -1;
int last_buy_actor = -1, last_message_actor = -1;
int last_channel_actor = -1, last_diplomacy_actor = -1;
uint32_t *arrived_mask = nullptr;
struct Player {
  int is_leader_female = 0;
  char title_leader[24]{};
  char name_leader[24]{};
  char noun_faction[24]{};
  int noun_gender = 0;
  int is_noun_plural = 0;
  char adj_name_faction[128]{};
} Players[8];
struct PlayerData {
  uint32_t diplo_treaties[8]{};
  int unk_29[11]{};
  int unk_30[11]{};
} PlayersData[8];
struct Spot {
  int result = 0, faction = 0, option = 0;
  int check(int, int, int *out_faction, int *out_option) {
    *out_faction = faction;
    *out_option = option;
    return result;
  }
};
struct NetMsg { void pop(const char *label, int, int, const char *) { events.push_back(std::string("pop:") + label); } } net_msg;
NetMsg *NetMsgPopup = &net_msg;
struct NetDaemon {
  int open_channel(int actor, int) {
    last_channel_actor = actor;
    events.push_back("channel");
    if (mutate_local_on_channel >= 0) {
      ConsoleGlobal.faction_id_ = mutate_local_on_channel;
      mutate_local_on_channel = -1;
    }
    return 0;
  }
} NetDaemonNet;
int eligible(int faction) {
  if (faction == mutate_alive_after_eligible_faction) {
    LockEnableMask = (LockEnableMask & ~0x0000ff00u) |
                     (static_cast<uint32_t>(alive_mask_after_eligible) << 8);
    mutate_alive_after_eligible_faction = -1;
  }
  return 1;
}
int pop_wait() { return 0; }
int X_pop(const char *label, int (*)()) {
  events.push_back(std::string("xpop:") + label);
  if (mutate_after_prompt) {
    LockEnableMask |= 1u << 2;
    if (arrived_mask) *arrived_mask = 0x6;
    mutate_after_prompt = 0;
  }
  if (mutate_alive_after_prompt) {
    LockEnableMask = (LockEnableMask & ~0x0000ff00u) |
                     (static_cast<uint32_t>(alive_mask_after_prompt) << 8);
    mutate_alive_after_prompt = 0;
  }
  if (mutate_local_after_prompt >= 0) {
    ConsoleGlobal.faction_id_ = mutate_local_after_prompt;
    mutate_local_after_prompt = -1;
  }
  return prompt_result;
}
void parse_says(int id, const char *, int, int) { events.push_back("parse:" + std::to_string(id)); }
void parse_num(int, int) { events.push_back("number"); }
void message_data(int, int, int, int actor, int, int) {
  last_message_actor = actor;
  events.push_back("message");
}
void buy_council_vote(int actor, int, int, int selection) {
  last_buy_actor = actor;
  events.push_back("buy:" + std::to_string(selection));
  if (mutate_local_on_buy >= 0) {
    ConsoleGlobal.faction_id_ = mutate_local_on_buy;
    mutate_local_on_buy = -1;
  }
}
void diplomacy_ends(int actor, int) {
  last_diplomacy_actor = actor;
  events.push_back("diplomacy-end");
}
std::vector<int> government_option_ids;
struct Dialogs { void item(char *, int id) { events.push_back("item"); government_option_ids.push_back(id); } };
struct Popup {
  Dialogs options;
  void close() { events.push_back("close"); }
  void start(char *, const char *, int, char *, int, void *) { events.push_back("start"); }
  Dialogs &dialogs() { return options; }
  int exec(int, int (*)()) { events.push_back("exec"); return 0x270f; }
};
struct CouncWin {
  Spot spot_;
  uint32_t field_A1C_ = 0, field_A20_ = 0, field_A24_ = 0, field_A40_ = 0, field_A44_ = 0;
  void draw_leader(int faction) { events.push_back("leader:" + std::to_string(faction)); }
  void redraw() { events.push_back("redraw"); }
  void on_left_click(int, int);
};
BODY
int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_ = 1;
  win.spot_.result = -1;
  win.on_left_click(0, 0);
  assert((events == std::vector<std::string>{"close"}));

  events.clear();
  win.spot_.result = 0;
  win.spot_.faction = 1;
  win.on_left_click(0, 0);
  assert((events == std::vector<std::string>{"leader:1", "close"}));

  events.clear();
  win.spot_.faction = 2;
  LockEnableMask = (1u << 2); // human-held target is only highlighted
  win.on_left_click(0, 0);
  assert((events == std::vector<std::string>{"leader:2", "close"}));

  events.clear();
  LockEnableMask = (1u << 8) | (1u << 1); // faction 0 alive, local is human
  win.spot_.faction = 0;
  PlayersData[0].diplo_treaties[1] = 0x10;
  win.on_left_click(0, 0);
  assert(events.size() == 4 && events[0] == "parse:0" &&
         events[1] == "parse:1" && events[2] == "xpop:BUYNOTALK" &&
         events[3] == "close");

  events.clear();
  PlayersData[0].diplo_treaties[1] = 0;
  PlayersData[0].unk_29[0] = 1;
  PlayersData[0].unk_30[0] = 1;
  win.on_left_click(0, 0);
  assert((events == std::vector<std::string>{"close"}));

  events.clear();
  win.spot_.faction = 2;
  LockEnableMask = (1u << 1) | (1u << 8) | (1u << 10);
  PlayersData[2].unk_29[0] = 0;
  prompt_result = 1;
  ConsoleGlobal.faction_id_ = 1;
  mutate_local_after_prompt = -1;
  win.on_left_click(0, 0);
  assert(events.size() == 5 && events[0] == "xpop:WHICHVOTE" &&
         events[1] == "buy:-2" && events[2] == "redraw" &&
         events[3] == "leader:2" && events[4] == "close");
  assert(last_buy_actor == 1); // unchanged local faction stays the native actor

  events.clear();
  ConsoleGlobal.faction_id_ = 1;
  mutate_local_after_prompt = 4;
  win.on_left_click(0, 0);
  assert(last_buy_actor == 4); // reload occurs after the prompt callback

  events.clear();
  IsMultiplayerNet = 1;
  ConsoleGlobal.faction_id_ = 1;
  LockEnableMask = (1u << 1) | (1u << 9) | (1u << 10);
  win.field_A44_ = 0;
  arrived_mask = &win.field_A44_;
  mutate_after_prompt = 1;
  mutate_local_after_prompt = 3;
  prompt_result = 0;
  win.on_left_click(0, 0);
  assert(events.size() == 8 && events[0] == "xpop:WHICHVOTE" &&
         events[1] == "message" && events[2] == "parse:0" &&
         events[3] == "parse:1" && events[4] == "number" &&
         events[5] == "pop:DIPLOCOMM" && events[6] == "leader:2" &&
         events[7] == "close");
  assert(CommlinkPending == 1 && CommlinkFaction == 2 && last_message_actor == 3);

  events.clear();
  ConsoleGlobal.faction_id_ = 1;
  LockEnableMask = (1u << 1) | (1u << 9) | (1u << 10);
  win.field_A44_ = 1u << 1;
  arrived_mask = &win.field_A44_;
  prompt_result = 1;
  mutate_local_after_prompt = 3;
  mutate_local_on_channel = 4;
  mutate_local_on_buy = 5;
  win.on_left_click(0, 0);
  assert(last_channel_actor == 3); // reload after vote-selection prompt
  assert(last_buy_actor == 4); // reload after open_channel callback
  assert(last_diplomacy_actor == 5); // reload after purchase callback

  // A target alive at the early gate can be eliminated by the vote prompt.
  // The later native byte read at 0x425723 must use that changed state.
  events.clear();
  IsMultiplayerNet = 0;
  ConsoleGlobal.faction_id_ = 1;
  LockEnableMask = (1u << 1) | (1u << 10);
  win.spot_.faction = 2;
  PlayersData[2].unk_29[0] = 0;
  prompt_result = 1;
  mutate_alive_after_prompt = 1;
  alive_mask_after_prompt = 0;
  win.on_left_click(0, 0);
  bool eliminated_notice = false;
  bool bought_vote = false;
  for (const std::string &event : events) {
    eliminated_notice |= event == "pop:ELIMINATED";
    bought_vote |= event.find("buy:") == 0;
  }
  assert(eliminated_notice && !bought_vote);
}
"""


def test_on_left_click_routes_hit_treaty_duplicate_and_vote_paths(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = adapt_body(extract_function(SOURCE.read_text(), "void CouncWin::on_left_click(int x, int y)"))
    fixture = LEFT_CLICK_HARNESS.replace(
        "NetMsg *NetMsgPopup = &net_msg;",
        "NetMsg *NetMsgPopup = &net_msg; NetMsg NetMsgPrimary;",
    )
    harness = adapt_harness(fixture.replace("BODY", body))
    executable = tmp_path / "councwin-left-click"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr
