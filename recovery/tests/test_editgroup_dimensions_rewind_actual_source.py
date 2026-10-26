"""Bounded host control for EditGroup's actual dimensions body and list rewind.

Buffer/font methods are deterministic service doubles. This does not execute
on_redraw's separately documented null-payload dereference.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
REWIND = "dialog.item_list_.seek_pos(-1);"


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unterminated function: {signature}")


HARNESS = r"""
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#define __cdecl
typedef int (__cdecl *AbsFnT)(int);

struct StringStructPayload { char *text; };
struct StringStructEntry {
  StringStructPayload *payload;
  StringStructEntry *next;
  StringStructEntry *previous;
  StringStructPayload *string_payload() { return payload; }
};
class StringStruct {
 public:
  int entry_count_;
  StringStructEntry *head_;
  StringStructEntry *current_;
  int current_position_;
  int seek_pos(int a1);
};
class StringList : public StringStruct {};

HELPER

struct Font {};
class Dialog {
 public:
  StringList item_list_;
  int row_count_;
  int field_50_;
  int field_64_;
  int field_68_;
  int field_20_;
  int field_2C_;
  unsigned int field_34_;
  int field_40_;
  unsigned int field_44_;
  int field_4C_;
  int column_count_;
  int field_30_;
  int field_60_;
  int field_5C_;
  unsigned int text_color_a_, text_color_b_, text_color_c_, text_color_d_;
  unsigned int text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_;
  unsigned int text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_;
  static Font *font1_;
  static Font *font2_;
  static Font *font3_;
  Dialog() : row_count_(0), field_50_(0), field_64_(0), field_68_(0),
      field_20_(0), field_2C_(0), field_34_(0), field_40_(0), field_44_(0),
      field_4C_(1), column_count_(0), field_30_(0), field_60_(0), field_5C_(0),
      text_color_a_(1), text_color_b_(2), text_color_c_(3), text_color_d_(4),
      text_color_2a_(5), text_color_2b_(6), text_color_2c_(7), text_color_2d_(8),
      text_color_3a_(9), text_color_3b_(10), text_color_3c_(11), text_color_3d_(12) {}
};
Font *Dialog::font1_ = 0;
Font *Dialog::font2_ = 0;
Font *Dialog::font3_ = 0;

struct Buffer {
  int style_calls;
  int font_calls;
  int width_calls;
  Buffer() : style_calls(0), font_calls(0), width_calls(0) {}
  void set_text_color(unsigned int, unsigned int, unsigned int, unsigned int) { ++style_calls; }
  void set_text_color2(unsigned int, unsigned int, unsigned int, unsigned int) { ++style_calls; }
  void set_text_color3(unsigned int, unsigned int, unsigned int, unsigned int) { ++style_calls; }
  int set_font(Font *, Font *, Font *, int) { ++font_calls; return 0; }
  int text_width(char *text) { ++width_calls; return text ? static_cast<int>(std::strlen(text)) : 0; }
  int text_line_height() { return 13; }
};
struct GraphicWin : Buffer {};
struct EditGroup : virtual GraphicWin, virtual Dialog {
  char *text_buffers_[10];
  int label_width_;
  int edit_width_;
  int calculate_dimensions();
  EditGroup() : label_width_(0), edit_width_(0) {
    for (int i = 0; i < 10; ++i) text_buffers_[i] = 0;
  }
};

CALCULATE_DIMENSIONS

static void require(bool condition, const char *message) {
  if (!condition) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::abort();
  }
}

static void init_list(StringList &list, StringStructEntry *head,
                      StringStructEntry *current, int count, int position) {
  list.head_ = head;
  list.current_ = current;
  list.entry_count_ = count;
  list.current_position_ = position;
}

int main() {
  StringStructPayload payloads[3] = {{const_cast<char *>("a")},
      {const_cast<char *>("longest")}, {const_cast<char *>("mid")}};
  StringStructEntry entries[3];
  for (int i = 0; i < 3; ++i) entries[i].payload = &payloads[i];
  entries[0].next = &entries[1]; entries[0].previous = &entries[2];
  entries[1].next = &entries[2]; entries[1].previous = &entries[0];
  entries[2].next = &entries[0]; entries[2].previous = &entries[1];

  EditGroup positive;
  init_list(positive.item_list_, &entries[0], &entries[1], 3, 77);
  positive.text_buffers_[0] = const_cast<char *>("x");
  positive.text_buffers_[1] = const_cast<char *>("longedit");
  positive.text_buffers_[2] = const_cast<char *>("ed");
  positive.field_40_ = 2;
  positive.field_44_ = 2;
  positive.field_34_ = 4;
  positive.field_50_ = 8;
  positive.field_4C_ = 2;
  positive.field_2C_ = 0;
  positive.field_30_ = 0;
  require(positive.calculate_dimensions() == 0, "positive count returns zero");
  require(positive.item_list_.current_ == &entries[2] &&
          positive.item_list_.current_position_ == 2,
          "positive rewind starts at tail and iteration visits each row");
  require(positive.label_width_ == 7 && positive.edit_width_ == 12,
          "labels and edit buffers determine measured widths");
  require(positive.field_64_ == 31 && positive.field_5C_ == 62 &&
          positive.column_count_ == 2 && positive.row_count_ == 3 &&
          positive.field_68_ == 15 && positive.field_60_ == 47,
          "dimension arithmetic and grid outputs remain intact");
  require(static_cast<Buffer &>(positive).style_calls == 3 &&
          static_cast<Buffer &>(positive).font_calls == 1 &&
          static_cast<Buffer &>(positive).width_calls == 6,
          "real body performs the expected service calls");

  EditGroup empty;
  init_list(empty.item_list_, &entries[0], &entries[1], 0, 19);
  require(empty.calculate_dimensions() == 7, "zero count returns seven");
  require(empty.item_list_.current_ == &entries[1] &&
          empty.item_list_.current_position_ == 19,
          "zero count leaves list cursor and position untouched");
  require(static_cast<Buffer &>(empty).style_calls == 0,
          "zero count returns before buffer services");

  EditGroup negative;
  init_list(negative.item_list_, &entries[0], &entries[1], -1, 42);
  negative.field_4C_ = 1;
  require(negative.calculate_dimensions() == 0, "negative count follows nonempty path");
  require(negative.item_list_.current_ == &entries[1] &&
          negative.item_list_.current_position_ == 42,
          "negative rewind preserves prior cursor and position");
  require(negative.row_count_ == -1,
          "negative row count is not normalized");
}
"""


def _program(compiler: str, body: str, helper: str) -> str:
    del compiler
    return HARNESS.replace("HELPER", helper).replace("CALCULATE_DIMENSIONS", body)


def _compile_and_run(program: str, compiler: str, tmp_path: Path, name: str):
    binary = tmp_path / name
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(binary)],
        input=program,
        text=True,
        capture_output=True,
        timeout=30,
    )
    return built, subprocess.run(
        [str(binary)], text=True, capture_output=True, timeout=10
    ) if built.returncode == 0 else None


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_calculate_dimensions_uses_native_rewind_effects(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} is unavailable")
    source = (ROOT / "src/editgroup.cpp").read_text()
    header = (ROOT / "src/stringstruct.h").read_text()
    body = _function(source, "int EditGroup::calculate_dimensions()")
    helper = _function(header, "MEASURED inline int StringStruct::seek_pos(int a1)")
    helper = helper.replace("MEASURED inline ", "inline ", 1)
    assert REWIND in body, "expected coordinator-published helper call in actual source"

    built, ran = _compile_and_run(_program(compiler, body, helper), compiler,
                                  tmp_path, f"dimensions_{compiler}")
    assert built.returncode == 0, built.stderr
    assert ran is not None and ran.returncode == 0, (ran.stdout, ran.stderr)

    old_rewind = r"""dialog.item_list_.current_ = dialog.item_list_.head_;
    if (dialog.item_list_.entry_count_ > 0) {
        int step = abs(-1);
        if (step <= dialog.item_list_.entry_count_) {
            step = abs(-1);
            while (step > 0) {
                --step;
                dialog.item_list_.current_ = dialog.item_list_.current_->previous;
            }
            dialog.item_list_.current_position_ = dialog.item_list_.entry_count_ - 1;
        }
    }"""
    mutant = body.replace(REWIND, old_rewind, 1)
    assert mutant != body, "old manual rewind mutation did not apply"
    mutant_built, mutant_run = _compile_and_run(
        _program(compiler, mutant, helper), compiler, tmp_path,
        f"dimensions_old_rewind_{compiler}",
    )
    assert mutant_built.returncode == 0, mutant_built.stderr
    assert mutant_run is not None and mutant_run.returncode != 0
    assert "negative rewind preserves prior cursor and position" in mutant_run.stderr
