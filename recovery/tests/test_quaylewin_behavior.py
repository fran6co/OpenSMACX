"""Host controls for the actual QuayleWin close and callback bodies."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/quaylewin.cpp"
GENERAL = ROOT / "src/general.cpp"


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


HARNESS = r"""
#include <cassert>
#include <cstdint>
struct LockState { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; };
struct NetDaemon { PrimaryState primary_state_; LockState channels_[2]; } NetDaemonNet;
struct { int faction_id_; } ConsoleGlobal;
struct Spot {
  int result, selected, checks;
  int check(int, int, int *position, int *kind) {
    ++checks; *position = selected; *kind = 0; return result;
  }
};
struct ListBox { int closes; ListBox() : closes(0) {} uint32_t close() { ++closes; return 0; } };
struct GraphicWin {
  int redraws, closes, modal_releases;
  GraphicWin() : redraws(0), closes(0), modal_releases(0) {}
  void redraw() { ++redraws; }
  uint32_t close() { ++closes; return 0; }
  void release_modal() { ++modal_releases; }
};
struct QuayleWin : GraphicWin {
  int field_2144_;
  Spot spot_;
  ListBox list_box_;
  void close();
  void on_left_click(int, int);
  void on_selected(int, int);
  void on_button_clicked(int);
  int on_key_click(int, int);
};
BODY_CLOSE
BODY_LEFT
BODY_SELECTED
BODY_BUTTON
BODY_KEY
int main() {
  QuayleWin win{};
  ConsoleGlobal.faction_id_ = 2;

  NetDaemonNet.primary_state_.trailing_words_[7] = 1u << 2;
  win.on_left_click(20, 30);
  assert(win.spot_.checks == 0 && win.redraws == 0);
  NetDaemonNet.primary_state_.trailing_words_[7] = 0;
  NetDaemonNet.channels_[1].payload_.flags = 1;
  win.on_left_click(20, 30);
  assert(win.spot_.checks == 0 && win.redraws == 0);
  NetDaemonNet.channels_[1].payload_.flags = 0;
  NetDaemonNet.primary_state_.leading_words_[2] = 1;
  win.on_left_click(20, 30);
  assert(win.spot_.checks == 0 && win.redraws == 0);
  NetDaemonNet.primary_state_.leading_words_[2] = 0;

  win.spot_.result = 0; win.spot_.selected = 7;
  win.on_left_click(20, 30);
  assert(win.spot_.checks == 1 && win.field_2144_ == 7 && win.redraws == 1);
  win.on_selected(4, 123);
  assert(win.field_2144_ == 4 && win.redraws == 2);
  win.on_button_clicked(12);
  assert(win.field_2144_ == 4 && win.redraws == 2 && win.closes == 0);
  win.on_button_clicked(999);
  assert(win.field_2144_ == -1 && win.redraws == 3 && win.closes == 0);

  win.field_2144_ = 1;
  assert(win.on_key_click(0, 0) == 1);
  assert(win.field_2144_ == -1 && win.redraws == 4 && win.closes == 0);
  win.field_2144_ = -1;
  assert(win.on_key_click(0, 0) == 1);
  assert(win.list_box_.closes == 1 && win.modal_releases == 1 && win.closes == 1);
}
"""


def test_quaylewin_actual_click_and_close_bodies(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    bodies = (
        extract_function(source, "void QuayleWin::close()"),
        extract_function(source, "void QuayleWin::on_left_click(int x, int y)"),
        extract_function(source, "void QuayleWin::on_selected(int selection, int)"),
        extract_function(source, "void QuayleWin::on_button_clicked(int button_id)"),
        extract_function(source, "int QuayleWin::on_key_click(int, int)"),
    )
    harness = HARNESS
    for token, body in zip(
        ("BODY_CLOSE", "BODY_LEFT", "BODY_SELECTED", "BODY_BUTTON", "BODY_KEY"),
        bodies,
    ):
        harness = harness.replace(token, body)
    executable = tmp_path / "quaylewin-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode()


def test_parse_it_dispatches_filename_section_and_clears_destination(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    production = extract_function(GENERAL.read_text(), "void __cdecl parse_it(char *file_name")
    harness = r"""
#include <cassert>
#include <cstring>
#define __cdecl
char StringTemp[128];
char TextBufferGetPtr[128];
int open_result, opened, fetched, parsed;
char seen_file[32], seen_section[32], seen_input[128];
int X_text_open(char *file, const char *section) {
  ++opened; std::strcpy(seen_file, file); std::strcpy(seen_section, section);
  return open_result;
}
char *text_get() { ++fetched; std::strcpy(TextBufferGetPtr, "record"); return TextBufferGetPtr; }
int parse_string(char *input, char *output) {
  ++parsed; std::strcpy(seen_input, input); std::strcpy(output, input); return 0;
}
BODY
int main() {
  std::strcpy(StringTemp, "stale"); open_result = 1;
  parse_it((char*)"Holobook", "BOOKTITLE");
  assert(!StringTemp[0] && opened == 1 && fetched == 0 && parsed == 0);
  open_result = 0; parse_it((char*)"Holobook", "BOOKLIST3");
  assert(!std::strcmp(seen_file, "Holobook") && !std::strcmp(seen_section, "BOOKLIST3"));
  assert(fetched == 1 && parsed == 1 && !std::strcmp(seen_input, "record"));
  assert(!std::strcmp(StringTemp, "record"));
}
""".replace("BODY", production)
    executable = tmp_path / "parse-it-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(executable)],
        input=harness, text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode()


REDRAW_HARNESS = r"""
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef int BOOL;
struct RECT { long left, top, right, bottom; };
struct Heap {} owner_heap;
Heap *StringAllocationHeap;
char StringTemp[512];
int GenderDefault, PluralityDefault, Language;
struct Font { int id; explicit Font(int value = 0) : id(value) {} };
Font title_font(11), body_font(22), list_font(33), heading_font(44);
struct { Font *field_1E1AC_, *field_1E1B0_, *field_1E1A8_, *field_1E1B8_; } ReportIfGlobal = {
  &title_font, &body_font, &list_font, &heading_font
};
struct { int faction_id_; } ConsoleGlobal;
struct { int unk_107; } PlayersData[8];
struct Player { int is_leader_female; char title_leader[32]; char name_leader[32]; } Players[8];
struct StringStruct { void remove_all() {} };
std::vector<std::string> sections, speech_inputs, list_items, events, wrapped_texts;
std::vector<int> speech_ids, speech_gender, speech_plural;
std::vector<int> spot_pages, spot_tops, wrap_fonts, wrapped_pages;
int parse_file_count, rating_score, rating_book, list_close_count, list_attach_flags;
int list_report_color, list_selected, list_show_calls, spot_kills, clip_calls;
int copies, art_loads, art_draws, write_lines, centered_lines, centered_wraps;
RECT last_dialog_rect, last_write_rect, last_center_rect;
std::string last_written, last_centered, art_filename;

int _itoa(int value, char *output, int radix) {
  if (radix == 10) std::sprintf(output, "%d", value);
  return 0;
}
const char *label_get(int) { return "Rating"; }
int get_rating(int book, int score) { rating_book = book; rating_score = score; return 17; }
void parse_says(int id, const char *input, int, int) {
  speech_ids.push_back(id); speech_inputs.push_back(input);
  speech_gender.push_back(GenderDefault); speech_plural.push_back(PluralityDefault);
}
void parse_it(char *, const char *section) {
  sections.push_back(section); ++parse_file_count;
  if (!std::strcmp(section, "BOOKTITLE")) std::strcpy(StringTemp, "Title");
  else if (!std::strcmp(section, "BOOKINTRO")) std::strcpy(StringTemp, "Intro");
  else if (!std::strncmp(section, "BOOKLIST", 8)) std::strcpy(StringTemp, section + 8);
  else std::strcpy(StringTemp, section);
}
struct Buffer {
  int font_id, colors[4];
  RECT active_clip;
  Buffer() : font_id(0) { std::memset(colors, 0, sizeof(colors)); }
  void restore_clip() { ++clip_calls; }
  int set_clip(RECT *r) { ++clip_calls; active_clip = *r; return 0; }
  void set_font(Font *a, Font *, Font *, int) { font_id = a ? a->id : 0; }
  void set_text_color(int a, int, int, int) { colors[0] = a; }
  void set_text_color2(int a, int, int, int) { colors[1] = a; }
  void set_text_color3(int a, int, int, int) { colors[2] = a; }
  void copy(Buffer *, int, int, int, int, int, int) { ++copies; }
  int wrap_cent(char *s, int, int, int) { last_centered = s; wrap_fonts.push_back(font_id); return 0; }
  int wrap_cent(char *s, RECT *r) { ++centered_wraps; last_centered = s; last_center_rect = *r; wrap_fonts.push_back(font_id); return 0; }
  int wrap_height(char *, int) { return 12; }
  int wrap(char *s, RECT *r) { wrapped_pages.push_back(std::atoi(s)); wrapped_texts.push_back(s); last_center_rect = *r; return 0; }
  int write_l(char *s, RECT *r, unsigned int) { ++write_lines; last_written = s; last_write_rect = *r; return 0; }
  int write_cent_l(char *s, RECT *r, unsigned int) { ++centered_lines; last_centered = s; last_center_rect = *r; wrap_fonts.push_back(font_id); return 0; }
  int load_pcx(char *s, int, int, int) { ++art_loads; art_filename = s; return 0; }
  int draw(Buffer *, int, int, int) { ++art_draws; return 0; }
};
struct Button { int hides, shows; Button() : hides(0), shows(0) {} void hide() { ++hides; } void show(int) { ++shows; } };
struct Spot {
  void kill_type(int) { ++spot_kills; }
  int add(int page, int, int, int top, int, int) { spot_pages.push_back(page); spot_tops.push_back(top); return 0; }
};
struct Dialog {
  int field_B4_;
  void init(RECT *r, int) { last_dialog_rect = *r; events.push_back("dialog.init"); }
  void set_dialog_font(Font *, Font *, Font *) { events.push_back("dialog.font"); }
  void set_dialog_text_color(int color, int, int, int) { events.push_back(color == 0x63 ? "dialog.color63" : "dialog.color"); }
  void attach_at_stored_position(void *, int flags) { list_attach_flags = flags; events.push_back("dialog.attach"); }
};
struct ListBox : Dialog {
  int field_8_;
  void close() { ++list_close_count; events.push_back("list.close"); }
  void set_report_text_color(int value) { list_report_color = value; events.push_back("list.palette"); }
  void item(char *s, int page) { list_items.push_back(std::string(s) + ":" + std::to_string(page)); events.push_back("list.item"); }
  void reset_report_selection() { events.push_back("list.reset"); }
  void set_selected_pos(int, int) { ++list_selected; events.push_back("list.select"); }
  void show(int) { ++list_show_calls; events.push_back("list.show"); }
};
struct QuayleWin : Buffer {
  RECT window_rect_, chapter_list_rect_, illustration_rect_, navigation_button_rect_;
  RECT title_rect_, introduction_rect_, rating_rect_, text_scratch_rect_;
  unsigned int field_A14_, field_A18_, field_A1C_, field_A20_;
  unsigned int line_advance_, monitor_mode_;
  int field_2144_, field_2148_;
  Button chapter_button_, pages_button_;
  Spot spot_;
  ListBox list_box_;
  Buffer buffer1_, buffer2_;
  void on_redraw();
};
BODY_REDRAW

void reset_observations() {
  sections.clear(); speech_inputs.clear(); list_items.clear(); events.clear(); wrapped_texts.clear();
  speech_ids.clear(); speech_gender.clear(); speech_plural.clear(); spot_pages.clear();
  spot_tops.clear(); wrap_fonts.clear(); wrapped_pages.clear(); parse_file_count = 0;
  rating_book = rating_score = list_close_count = list_attach_flags = 0;
  list_report_color = list_selected = list_show_calls = spot_kills = clip_calls = 0;
  copies = art_loads = art_draws = write_lines = centered_lines = centered_wraps = 0;
  last_written.clear(); last_centered.clear(); art_filename.clear();
}
void initialize(QuayleWin &win, int chapter, int language) {
  reset_observations();
  win = QuayleWin();
  Language = language; ConsoleGlobal.faction_id_ = 3;
  std::strcpy(Players[3].title_leader, "Leader");
  std::strcpy(Players[3].name_leader, "LeaderName");
  Players[3].is_leader_female = 1; PlayersData[3].unk_107 = 91;
  StringAllocationHeap = &owner_heap;
  win.monitor_mode_ = 1; win.field_2144_ = chapter; win.field_2148_ = 2;
  win.window_rect_ = RECT{0, 0, 800, 600};
  win.chapter_list_rect_ = RECT{10, 20, 310, 250};
  win.illustration_rect_ = RECT{100, 30, 700, 500};
  win.navigation_button_rect_ = RECT{600, 500, 700, 530};
  win.title_rect_ = RECT{20, 20, 300, 80};
  win.introduction_rect_ = RECT{20, 100, 300, 350};
  win.rating_rect_ = RECT{350, 500, 620, 530};
}
void assert_heap_restored() { assert(StringAllocationHeap == &owner_heap); }
void test_english_list() {
  QuayleWin win; initialize(win, -1, 0); win.on_redraw();
  assert_heap_restored();
  assert((sections == std::vector<std::string>{"BOOKTITLE", "BOOKINTRO", "BOOKLIST0", "BOOKLIST1", "BOOKLIST2"}));
  assert(spot_kills == 1 && (spot_pages == std::vector<int>{0, 1, 2}));
  assert((spot_tops == std::vector<int>{238, 220, 202}));
  assert(win.text_scratch_rect_.left == 10 && win.text_scratch_rect_.top == 202);
  assert(win.text_scratch_rect_.right == 310 && win.text_scratch_rect_.bottom == 214);
  assert(wrapped_texts.size() == 3 && wrapped_texts[2] == "[2]");
  assert((speech_ids == std::vector<int>{0, 1, 0, 1, 0, 1}));
  assert((speech_gender == std::vector<int>{1, 1, 1, 1, 1, 1}));
  assert((speech_plural == std::vector<int>{0, 0, 0, 0, 0, 0}));
  assert(rating_book == 3 && rating_score == 91 && last_written == "Rating:  17%");
  assert(last_write_rect.left == 350 && win.field_2144_ == -1);
  assert(win.chapter_button_.shows == 1 && win.pages_button_.shows == 0);
}
void test_multilingual_list() {
  QuayleWin win; initialize(win, -1, 1); win.on_redraw();
  assert_heap_restored();
  assert((sections == std::vector<std::string>{"BOOKTITLE", "BOOKINTRO", "BOOKLIST2", "BOOKLIST1", "BOOKLIST0"}));
  assert(list_close_count == 1 && last_dialog_rect.left == 10 && last_dialog_rect.right == 310);
  assert((list_items == std::vector<std::string>{"2:2", "1:1", "0:0"}));
  assert(list_report_color == 0x9B && list_attach_flags == 0x80020);
  assert(list_selected == 1 && list_show_calls == 1);
  assert(win.list_box_.field_8_ == 1 && win.list_box_.field_B4_ == 0);
  assert((events == std::vector<std::string>{"list.close", "dialog.init", "dialog.font",
      "dialog.color63", "list.palette", "list.item", "list.item", "list.item",
      "list.reset", "dialog.attach", "list.select", "list.show"}));
  assert((speech_ids == std::vector<int>{0, 1, 0, 1, 0, 1}));
  assert((speech_plural == std::vector<int>{0, 0, 0, 0, 0, 0}));
  assert(rating_book == 3 && rating_score == 91 && win.field_2144_ == -1);
}
void test_chapter(int chapter, Font *expected_font, RECT expected, int expected_color, bool wrap) {
  QuayleWin win; initialize(win, chapter, 0); win.on_redraw();
  assert_heap_restored();
  assert((sections == std::vector<std::string>{"BOOKTITLE", "BOOKINTRO"}));
  assert(art_filename == std::string("artbox") + std::to_string(chapter) + ".pcx");
  assert(art_loads == 1 && art_draws == 1);
  assert(win.text_scratch_rect_.left == expected.left && win.text_scratch_rect_.top == expected.top);
  assert(win.text_scratch_rect_.right == expected.right && win.text_scratch_rect_.bottom == expected.bottom);
  if (wrap) {
    assert(centered_wraps == 1 && last_center_rect.left == expected.left && last_center_rect.bottom == expected.bottom);
  } else {
    assert(centered_lines == 1 && last_center_rect.left == expected.left && last_center_rect.bottom == expected.bottom);
  }
  assert(!wrap_fonts.empty() && wrap_fonts.back() == expected_font->id);
  assert(win.colors[0] == expected_color);
  assert(last_centered == "Leader LeaderName");
  assert(win.pages_button_.shows == 1 && win.chapter_button_.shows == 0);
  assert(win.illustration_rect_.left == 100);
}
int main() {
  test_english_list(); test_multilingual_list();
  test_chapter(1, &title_font, RECT{0x15B, 0x1F6, 0x2E2, 0x21C}, 0x45, false);
  test_chapter(2, &heading_font, RECT{0x136, 0x1A5, 0x301, 0x1D7}, 0x45, false);
  test_chapter(8, &heading_font, RECT{0x13C, 0x1DF, 0x2FD, 0x233}, 0x5E, true);
}
"""


def test_quayle_redraw_production_branches_and_observable_effects(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = extract_function(SOURCE.read_text(), "void QuayleWin::on_redraw()")
    harness = REDRAW_HARNESS.replace("BODY_REDRAW", body)
    executable = tmp_path / "quaylewin-redraw-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-x", "c++", "-", "-o", str(executable)],
        input=harness, text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode()
