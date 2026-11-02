"""Host controls for the actual QuayleWin close and callback bodies."""
from pathlib import Path
import shutil
import subprocess

import pytest
from owner_transfer_test_support import current_record_by_address


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/quaylewin.cpp"


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
struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; int session_phase_; uint32_t trailing_words_[9]; };
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

  NetDaemonNet.primary_state_.trailing_words_[6] = 1u << 2;
  win.on_left_click(20, 30);
  assert(win.spot_.checks == 0 && win.redraws == 0);
  NetDaemonNet.primary_state_.trailing_words_[6] = 0;
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
    record = current_record_by_address(ROOT, 0x0048C060)
    production = extract_function(record.region, "void __cdecl parse_it(char *file_name")
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


# This host fixture copies the allocation-base lifetime bodies and stubs list
# storage/removal; it checks owner restoration, not the production ABI or runtime.
REDRAW_HARNESS = r"""
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#define __cdecl
typedef int BOOL;
struct RECT { long left, top, right, bottom; };
struct Heap {} owner_heap;
Heap *StringAllocationHeap;
int allocation_base_constructions, allocation_base_destructions, string_list_removals;
struct StringAllocationBase {
  Heap *allocation_owner_;
  StringAllocationBase() {
    allocation_owner_ = StringAllocationHeap;
    StringAllocationHeap = 0;
    ++allocation_base_constructions;
  }
  virtual ~StringAllocationBase() {
    StringAllocationHeap = allocation_owner_;
    ++allocation_base_destructions;
  }
};
struct StringStruct : virtual StringAllocationBase {
  int current_position_;
  StringStruct() : current_position_(0) {}
  virtual ~StringStruct() { remove_all(); current_position_ = 0; }
  void remove_all() { ++string_list_removals; }
};
struct StringList : StringStruct {
  ~StringList() { remove_all(); current_position_ = 0; }
};
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
int chapter_append_calls;
// Host append-seam probe: perturb gender between title and name to expose
// hoisted reads. This does not claim the native CRT mutates Player records.
char *observe_chapter_strcat(char *destination, const char *text) {
  const bool title = text == Players[3].title_leader;
  const bool name = text == Players[3].name_leader;
  if (!title && !name) return std::strcat(destination, text);
  assert(GenderDefault == Players[3].is_leader_female && PluralityDefault == 0);
  ++chapter_append_calls;
  char *result = std::strcat(destination, text);
  if (title) {
    Players[3].is_leader_female = -0x10003;
    GenderDefault = -77; PluralityDefault = -97;
  }
  return result;
}
std::vector<std::string> sections, speech_inputs, list_items, events, wrapped_texts;
std::vector<int> speech_ids, speech_gender, speech_plural;
std::vector<int> spot_pages, spot_tops, wrap_fonts, wrapped_pages;
int parse_file_count, rating_score, rating_book, list_close_count, list_attach_flags;
int list_report_color, list_selected, list_show_calls, spot_kills, clip_calls;
bool throw_during_parse;
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
  assert(StringAllocationHeap == 0);
  if (throw_during_parse) throw std::runtime_error("host redraw unwind control");
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
#define strcat observe_chapter_strcat
BODY_REDRAW
#undef strcat

void reset_observations() {
  chapter_append_calls = 0;
  sections.clear(); speech_inputs.clear(); list_items.clear(); events.clear(); wrapped_texts.clear();
  speech_ids.clear(); speech_gender.clear(); speech_plural.clear(); spot_pages.clear();
  spot_tops.clear(); wrap_fonts.clear(); wrapped_pages.clear(); parse_file_count = 0;
  rating_book = rating_score = list_close_count = list_attach_flags = 0;
  list_report_color = list_selected = list_show_calls = spot_kills = clip_calls = 0;
  copies = art_loads = art_draws = write_lines = centered_lines = centered_wraps = 0;
  last_written.clear(); last_centered.clear(); art_filename.clear();
  throw_during_parse = false;
  allocation_base_constructions = allocation_base_destructions = 0;
  string_list_removals = 0;
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
  assert(allocation_base_constructions == 1 && allocation_base_destructions == 1);
  assert(string_list_removals == 3); // explicit body, StringList dtor, StringStruct dtor
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
  QuayleWin win; initialize(win, chapter, 0);
  Players[3].is_leader_female = 0x10003;
  GenderDefault = -77; PluralityDefault = -97;
  win.on_redraw();
  assert(chapter_append_calls == 2);
  assert(GenderDefault == -0x10003 && PluralityDefault == 0);
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
void test_owner_restored_on_unwind() {
  QuayleWin win; initialize(win, -1, 0);
  throw_during_parse = true;
  bool caught = false;
  try { win.on_redraw(); }
  catch (const std::runtime_error &) { caught = true; }
  assert(caught);
  assert(allocation_base_constructions == 1 && allocation_base_destructions == 1);
  assert(string_list_removals == 2); // both list destructors run during unwind
  assert_heap_restored();
}
void legacy_manual_owner_exchange_unwind() {
  Heap *saved_string_owner = StringAllocationHeap;
  StringAllocationHeap = 0;
  StringStruct strings;
  throw std::runtime_error("legacy manual restore is skipped during unwind");
  strings.remove_all();
  StringAllocationHeap = saved_string_owner;
}
void test_legacy_manual_restore_damage_control() {
  StringAllocationHeap = &owner_heap;
  try { legacy_manual_owner_exchange_unwind(); }
  catch (const std::runtime_error &) {}
  assert(StringAllocationHeap == 0);
  StringAllocationHeap = &owner_heap;
}
int main() {
  test_english_list(); test_multilingual_list();
  test_chapter(1, &title_font, RECT{0x15B, 0x1F6, 0x2E2, 0x21C}, 0x45, false);
  test_chapter(2, &heading_font, RECT{0x136, 0x1A5, 0x301, 0x1D7}, 0x45, false);
  test_chapter(8, &heading_font, RECT{0x13C, 0x1DF, 0x2FD, 0x233}, 0x5E, true);
  test_owner_restored_on_unwind();
  test_legacy_manual_restore_damage_control();
}
"""


def run_redraw_host(tmp_path, body, should_pass=True):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = REDRAW_HARNESS.replace("BODY_REDRAW", body)
    executable = tmp_path / "quaylewin-redraw-host-control"
    (tmp_path / "quaylewin-redraw-host-control.cpp").write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-x", "c++", "-", "-o", str(executable)],
        input=harness, text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) is should_pass, result.stderr.decode()
    return result


def test_quayle_redraw_production_branches_and_observable_effects(tmp_path):
    body = extract_function(SOURCE.read_text(), "void QuayleWin::on_redraw()")
    run_redraw_host(tmp_path, body)


@pytest.mark.parametrize("chapter, phase", [(1, "title"), (1, "name"),
    (2, "title"), (2, "name"), (8, "title"), (8, "name"), (8, "width")])
def test_quayle_chapter_append_grammar_damage_is_detected(tmp_path, chapter, phase):
    body = extract_function(SOURCE.read_text(), "void QuayleWin::on_redraw()")
    start = body.index(f"if (chapter == {chapter}) {{")
    end = body.index("      canvas->set_font", start)
    section = body[start:end]
    field = "title_leader" if phase == "title" else "name_leader"
    gender = "      GenderDefault = Players[book].is_leader_female;"
    plural = "      PluralityDefault = 0;"
    stores = [gender, plural] if phase == "title" and chapter != 8 else [plural, gender]
    needle = "\n".join(stores + [f"      strcat(StringTemp, Players[book].{field});"])
    assert section.count(needle) == 1
    replacement = (needle.replace("= Players[book]", "= !!Players[book]")
                   if phase == "width" else needle.splitlines()[-1])
    changed_section = section.replace(needle, replacement, 1)
    changed = body[:start] + changed_section + body[end:]
    assert changed != body
    result = run_redraw_host(tmp_path, changed, should_pass=False)
    assert b"Assertion" in result.stderr


# This projection exercises the existing Win virtual contract and the actual
# two entry bodies. The real global object's C++ dynamic type may allow direct
# and base-reference expressions to be observationally equivalent. Only the
# pinned/captured target instructions establish VC6's actual dispatch lowering.
ENTRY_HARNESS = r"""
#include <cstdio>
#include <cstdlib>
#include <climits>
#define __cdecl
int IsMultiplayerNet, init_status, init_calls, modal_calls, rating_calls;
int rating_value, rating_multiplayer, init_multiplayer, expected_page;
int expected_faction, expected_score;
void require(bool condition, const char *phase) {
  if (!condition) { std::fprintf(stderr, "failed:%s\n", phase); std::abort(); }
}
struct Win {
  virtual int set_modal(int, int (*)(), Win *) { return -5; }
};
struct QuayleWin : Win {
  int field_2144_, field_2148_;
  int init() {
    ++init_calls;
    field_2144_ = -801; field_2148_ = -802;
    if (init_multiplayer >= 0) IsMultiplayerNet = init_multiplayer;
    return init_status;
  }
  SET_CHAPTER
  SET_PAGE
  int set_modal(int flag, int (*poll)(), Win *owner) override;
} QuayleWinGlobal;
int QuayleWin::set_modal(int flag, int (*poll)(), Win *owner) {
  require(this == &QuayleWinGlobal, "modal.receiver");
  require(flag == 0 && poll == nullptr && owner == nullptr, "modal.arguments");
  require(field_2148_ == expected_page, "modal.page");
  require(field_2144_ == -1, "modal.chapter");
  ++modal_calls;
  return 37;
}
struct PlayerData { int unk_107; } PlayersData[8];
int get_rating(int faction, int score) {
  require(faction == expected_faction && score == expected_score, "rating.arguments");
  ++rating_calls;
  if (rating_multiplayer >= 0) IsMultiplayerNet = rating_multiplayer;
  return rating_value;
}
BODY_DIRECT
BODY_RATED
void reset() {
  IsMultiplayerNet = init_status = init_calls = modal_calls = rating_calls = 0;
  rating_multiplayer = init_multiplayer = -1;
  QuayleWinGlobal.field_2144_ = -701;
  QuayleWinGlobal.field_2148_ = -702;
  expected_faction = 2; expected_score = 123;
  PlayersData[2].unk_107 = expected_score;
  expected_page = 0;
}
int original_page(int page) { return page >= 25 ? 24 : page < -1 ? -1 : page; }
int original_rank(int rating) {
  // Independent thresholds from the pin's signed square/2 scan at4966A2..B8.
  const int thresholds[] = {2,4,8,12,18,24,32,40,50,60,72,84,
    98,112,128,144,162,180,200,220,242,264,288,312};
  int rank = 0;
  for (int threshold : thresholds) if (rating >= threshold) ++rank;
  return rank;
}
int main() {
  for (int multiplayer : {1, 256, INT_MIN}) {
    reset(); IsMultiplayerNet = multiplayer;
    sub_496610(0);
    require(init_calls == 0 && modal_calls == 0, "guard.multiplayer");
    require(QuayleWinGlobal.field_2144_ == -701 && QuayleWinGlobal.field_2148_ == -702,
      "guard.multiplayer.fields");
  }
  for (int page : {INT_MIN, -2, -1, 0, 24, 25, INT_MAX}) {
    reset(); init_status = 1;
    sub_496610(page);
    require(init_calls == 1 && modal_calls == 0, "guard.init");
    require(QuayleWinGlobal.field_2144_ == -801 && QuayleWinGlobal.field_2148_ == -802,
      "guard.init.fields");
    reset(); expected_page = original_page(page);
    sub_496610(page);
    require(init_calls == 1 && modal_calls == 1, "modal.dispatch");
  }
  reset(); init_multiplayer = 256; expected_page = 7;
  sub_496610(7);
  require(modal_calls == 1, "init.no-second-multiplayer-guard");
  const int ratings[] = {-1,0,1,2,3,4,7,8,11,12,17,18,23,24,31,32,
    39,40,49,50,59,60,71,72,83,84,97,98,111,112,127,128,143,144,
    161,162,179,180,199,200,219,220,241,242,263,264,287,288,311,312,313,INT_MAX};
  for (int rating : ratings) {
    reset(); rating_value = rating; expected_page = original_rank(rating);
    quayle(2);
    require(rating_calls == 1 && init_calls == 1 && modal_calls == 1, "rated.dispatch");
  }
  reset(); rating_value = 24; rating_multiplayer = 256;
  quayle(2);
  require(rating_calls == 1 && init_calls == 0 && modal_calls == 0, "rated.live-multiplayer");
  require(QuayleWinGlobal.field_2144_ == -701 && QuayleWinGlobal.field_2148_ == -702,
    "rated.guard.fields");
  reset(); rating_value = 24; init_status = -1;
  quayle(2);
  require(rating_calls == 1 && init_calls == 1 && modal_calls == 0, "rated.init-failure");
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage, failure", [
    ("", ""),
    ("old_global_syntax", ""),
    ("qualified_base", "modal.dispatch"),
    ("wrong_high_clamp", "modal.page"),
    ("omit_chapter", "modal.chapter"),
    ("skip_multiplayer", "guard.multiplayer"),
])
def test_actual_quayle_entry_modal_contract(tmp_path, compiler_name, damage, failure):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text()
    direct = extract_function(source, 'extern "C" void __cdecl sub_496610(int page)')
    rated = extract_function(source, "void __cdecl quayle(int faction_id)")
    bodies = direct + "\n" + rated
    if damage == "old_global_syntax":
        assert bodies.count("Win &modal_window = QuayleWinGlobal;") == 2
        bodies = bodies.replace("Win &modal_window = QuayleWinGlobal;", "")
        bodies = bodies.replace("modal_window.set_modal", "QuayleWinGlobal.set_modal")
    elif damage == "qualified_base":
        assert bodies.count("modal_window.set_modal") == 2
        bodies = bodies.replace("modal_window.set_modal", "modal_window.Win::set_modal")
    elif damage == "wrong_high_clamp":
        assert "page = 0x18;" in bodies
        bodies = bodies.replace("page = 0x18;", "page = 0x17;", 1)
    elif damage == "omit_chapter":
        assert bodies.count("QuayleWinGlobal.set_chapter(-1);") == 2
        bodies = bodies.replace("QuayleWinGlobal.set_chapter(-1);", "", 1)
    elif damage == "skip_multiplayer":
        assert "if (IsMultiplayerNet == 0)" in bodies
        bodies = bodies.replace("if (IsMultiplayerNet == 0)", "if (true)", 1)
    header = (ROOT / "src/quaylewin.h").read_text()
    chapter = extract_function(header, "void set_chapter(int chapter)")
    page = extract_function(header, "void set_page(int page)")
    harness = ENTRY_HARNESS.replace("SET_CHAPTER", chapter).replace("SET_PAGE", page)
    harness = harness.replace("BODY_DIRECT", bodies).replace("BODY_RATED", "")
    harness = harness.replace("#include <climits>", "#include <climits>\n#include <initializer_list>")
    cpp = tmp_path / "entry-contract.cpp"
    cpp.write_text(harness)
    executable = tmp_path / "entry-contract"
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
        str(cpp), "-o", str(executable)], text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    if failure:
        assert ran.returncode < 0 and f"failed:{failure}" in ran.stderr, ran.stderr
    else:
        assert ran.returncode == 0, ran.stderr
