"""Actual-source controls for Buffer's right-justified RECT overloads."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"
HEADER = ROOT / "src/buffer.h"


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


HARNESS = r'''#include <cassert>
#include <cstring>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Font { int font_obj_, unk_1_, height_, line_height_; };
Font default_font={1,-1,5,12};
Font *FontDefault=&default_font;
int BufferStrHeight=99;
int find_calls=0;
bool split_next=true;
std::vector<int> widths, xs, ys, lengths, active_secondary;
std::vector<Font *> active_fonts;
char *find_line_break_l(char *text, int *width, int) {
  widths.push_back(*width);
  ++find_calls;
  return split_next && find_calls==1 ? text+3 : 0;
}
struct Buffer {
  Font *font1_, *font2_, *font3_;
  int field_10_, field_14_, field_18_;
  int wrap_right(Font *,char *,RECT *);
  int wrap_right(char *,RECT *);
  int write_right_l(char *,int,int,int);
};
int Buffer::write_right_l(char *,int x,int y,int len) {
  xs.push_back(x); ys.push_back(y); lengths.push_back(len);
  active_fonts.push_back(font1_);
  active_secondary.push_back((font2_!=0)+(font3_!=0));
  return y;
}
FONT_WRAPPER
RECT_BODY
int main() {
  Buffer b={&default_font,0,0,2,2,0};
  RECT area={10,20,40,50};
  char multiline[]="ab/cd";
  int y=b.wrap_right(multiline,&area);
  assert(y==44 && find_calls==2 && widths.size()==2);
  assert(widths[0]==30 && widths[1]==28);
  assert(xs.size()==2 && xs[0]==10 && xs[1]==10);
  assert(ys[0]==20 && ys[1]==32);
  assert(lengths[0]==3 && lengths[1]==2);
  assert(active_fonts[0]==&default_font && active_fonts[1]==&default_font);
  assert(active_secondary[0]==0 && active_secondary[1]==0);
  assert(b.field_18_==2 && b.field_14_==0 && BufferStrHeight==0);

  // Null text returns the original RECT top without resetting or drawing.
  const int writes=static_cast<int>(ys.size());
  b.field_18_=7; b.field_14_=9; BufferStrHeight=13;
  assert(b.wrap_right(0,&area)==20);
  assert(static_cast<int>(ys.size())==writes && b.field_18_==7 &&
         b.field_14_==9 && BufferStrHeight==13);

  // The Font wrapper overrides all three slots for the call, then restores
  // the prior triple after the integer y/height result returns.
  Font supplied={1,0,6,99};
  Font old_second={1,0,1,1}, old_third={1,0,1,1};
  b.font1_=&default_font; b.font2_=&old_second; b.font3_=&old_third;
  b.field_10_=0; b.field_14_=0; b.field_18_=0; find_calls=0;
  split_next=false;
  char single[]="x";
  RECT one_line={3,8,15,30};
  assert(b.wrap_right(&supplied,single,&one_line)==14);
  assert(active_fonts.back()==&supplied && active_secondary.back()==0);
  assert(b.font1_==&default_font && b.font2_==&old_second &&
         b.font3_==&old_third);
  return 0;
}
'''


def source_harness(mutant=None):
    text = SOURCE.read_text()
    wrapper = body(text, "int Buffer::wrap_right(Font *font, char *text, RECT *area)")
    rect_body = body(text, "int Buffer::wrap_right(char *text, RECT *area)")
    if mutant == "left_from_top":
        rect_body = rect_body.replace("int start = area->left;",
                                      "int start = area->top;", 1)
    elif mutant == "top_from_left":
        rect_body = rect_body.replace("int total = area->top;",
                                      "int total = area->left;", 1)
    assert "wrap_right(text, area)" in wrapper
    assert "area->right" in rect_body
    return HARNESS.replace("FONT_WRAPPER", wrapper).replace("RECT_BODY", rect_body)


def compile_run(tmp_path, compiler, source):
    cpp = tmp_path / "buffer-wrap-right-rect.cpp"
    exe = tmp_path / "buffer-wrap-right-rect"
    cpp.write_text(source)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    if build.returncode:
        return build, None
    return build, subprocess.run([str(exe)], capture_output=True,
                                 text=True, timeout=10)


def test_rectangle_signatures_are_shared_by_both_overloads():
    header = HEADER.read_text()
    assert "int wrap_right(Font *font, char *text, RECT *area);" in header
    assert "int wrap_right(char *text, RECT *area);" in header
    # The distinct, still-undefined scalar-coordinate declaration is untouched.
    assert "int wrap_right(Font *font, char *text, int x_coord, int len);" in header


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_source_multiline_null_text_and_font_restore(compiler, tmp_path):
    build, run = compile_run(tmp_path, compiler, source_harness())
    assert build.returncode == 0, build.stderr
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutant", ["left_from_top", "top_from_left"])
def test_rectangle_field_mutants_compile_then_fail(compiler, tmp_path, mutant):
    build, run = compile_run(tmp_path, compiler, source_harness(mutant))
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0
