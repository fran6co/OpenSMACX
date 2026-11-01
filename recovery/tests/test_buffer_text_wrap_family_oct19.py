"""Execute the mapped centered and scalar right wrap bodies with host doubles."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import (
    ROOT,
    SOURCE,
    extract_buffer_min_macro,
    extract_definition,
)


FUNCTIONS = {
    "write_cent_l": (
        "0x005DD250",
        "int Buffer::write_cent_l(char *text, int x_coord, int y_coord, int len)",
    ),
    "write_cent_l_span": (
        "0x005DD020",
        "int Buffer::write_cent_l(LPSTR text, int x_coord, int y_coord, int width,",
    ),
    "wrap_cent": (
        "0x005DDA00",
        "int Buffer::wrap_cent(char *text, int x_coord, int y_coord, int width,",
    ),
    "write_right_l": (
        "0x005DD3B0",
        "int Buffer::write_right_l(char *text, int x_coord, int y_coord, int len)",
    ),
    "wrap_right": (
        "0x005DDBB0",
        "int Buffer::wrap_right(char *text, int x_coord, int y_coord, int max_width)",
    ),
}


HARNESS = r'''#include <cassert>
#include <cstring>
#include <vector>

typedef char *LPSTR;
struct Font {
  int font_obj_, unk_1_, height_, line_height_;
  bool is_initialized() const { return font_obj_ != 0; }
};
Font default_font={1,0,4,90};
Font primary_font={1,-1,5,9};
Font *FontDefault=&default_font;
int BufferStrHeight=0;
static int test_strlen_calls=0;
static size_t counted_strlen(const char *text) {
  ++test_strlen_calls;
  return ::strlen(text);
}
#define strlen(text) counted_strlen(text)

static std::vector<char> events;
static std::vector<int> find_widths, find_lengths, find_strlen_counts;
static std::vector<int> measured_lengths, measure_strlen_counts;
static std::vector<int> write_xs, write_ys, write_lengths, write_strlen_counts;
static std::vector<Font *> active_fonts;
static int split_at[8]={0};
static int find_calls=0;
static int measure_width=0;
static int clear_font_on_write_index=-1;
static int write_calls=0;

struct Buffer {
  Font *fonts_[4];
  int field_10_, field_14_, field_18_;
  int wrap_cent(char *,int,int,int,int);
  int wrap_right(char *,int,int,int);
  int write_cent_l(char *,int,int,int);
  int write_cent_l(char *,int,int,int,int);
  int write_right_l(char *,int,int,int);
  char *find_line_break_l(char *,int *,int);
  int text_width(char *,unsigned int);
  int write_multi_font_raw_l(char *,int,int,unsigned int);
};

char *Buffer::find_line_break_l(char *text,int *width,int length) {
  events.push_back('F');
  find_widths.push_back(*width);
  find_lengths.push_back(length);
  find_strlen_counts.push_back(test_strlen_calls);
  const int split=split_at[find_calls++];
  *width-=1; // A local width edit must not alter the next line's input width.
  return split>0 && length>split ? text+split : 0;
}

int Buffer::text_width(char *,unsigned int length) {
  events.push_back('M');
  measured_lengths.push_back(static_cast<int>(length));
  measure_strlen_counts.push_back(test_strlen_calls);
  return measure_width;
}

int Buffer::write_multi_font_raw_l(char *,int x,int y,unsigned int length) {
  events.push_back('W');
  write_xs.push_back(x);
  write_ys.push_back(y);
  write_lengths.push_back(static_cast<int>(length));
  write_strlen_counts.push_back(test_strlen_calls);
  active_fonts.push_back(fonts_[0]);
  if (write_calls++==clear_font_on_write_index) fonts_[0]=0;
  return y;
}

'''

MAIN = r'''
static void reset_observations() {
  events.clear(); find_widths.clear(); find_lengths.clear();
  find_strlen_counts.clear(); measured_lengths.clear();
  measure_strlen_counts.clear(); write_xs.clear(); write_ys.clear();
  write_lengths.clear(); write_strlen_counts.clear(); active_fonts.clear();
  find_calls=0; write_calls=0; test_strlen_calls=0;
  clear_font_on_write_index=-1;
  for (int i=0;i<8;++i) split_at[i]=0;
}

int main() {
  // Null input leaves the original scalar-wrap state untouched.
  Buffer scalar_null={{&primary_font,0,0,0},3,8,9};
  BufferStrHeight=31; reset_observations();
  assert(scalar_null.wrap_right(0,40,100,20)==100);
  assert(events.empty() && test_strlen_calls==0 && BufferStrHeight==31);
  assert(scalar_null.field_18_==9 && scalar_null.field_14_==8);

  // A split followed by an unsplit tail exercises fresh per-line width,
  // fresh no-break strlen, one-time field_14 subtraction, and default reload
  // after the first writer callback clears fonts_[0].
  Buffer scalar={{&primary_font,0,0,0},3,3,7};
  char right_text[]="abcd";
  split_at[0]=2; split_at[1]=0; measure_width=4;
  clear_font_on_write_index=0; BufferStrHeight=17;
  assert(scalar.wrap_right(right_text,50,100,20)==108);
  assert(events.size()==6 && events[0]=='F' && events[1]=='M' && events[2]=='W' &&
         events[3]=='F' && events[4]=='M' && events[5]=='W');
  assert(find_widths.size()==2 && find_widths[0]==20 && find_widths[1]==17);
  assert(find_lengths[0]==4 && find_lengths[1]==2);
  assert(find_strlen_counts[0]==1 && find_strlen_counts[1]==4);
  assert(measured_lengths[0]==2 && measured_lengths[1]==2);
  assert(measure_strlen_counts[0]==3 && measure_strlen_counts[1]==7);
  assert(write_strlen_counts[0]==3 && write_strlen_counts[1]==7);
  assert(write_xs[0]==46 && write_xs[1]==46);
  assert(write_ys[0]==100 && write_ys[1]==104);
  assert(write_lengths[0]==2 && write_lengths[1]==2);
  assert(active_fonts[0]==&primary_font && active_fonts[1]==&default_font);
  assert(test_strlen_calls==7 && scalar.field_18_==2 && scalar.field_14_==0);
  assert(BufferStrHeight==0);

  // The empty but non-null string still takes one loop iteration and advances
  // by the default font's line height, while both writers return before draw.
  Buffer empty={{&primary_font,0,0,0},5,5,4};
  char empty_text[]=""; BufferStrHeight=23; reset_observations();
  assert(empty.wrap_right(empty_text,21,30,9)==39);
  assert(find_calls==1 && find_lengths[0]==0 && find_widths[0]==9);
  assert(events.size()==1 && events[0]=='F' && write_calls==0);
  assert(test_strlen_calls==4 && empty.field_18_==1 && empty.field_14_==0);
  assert(empty.fonts_[0]==&primary_font && BufferStrHeight==0);

  // Centered wrapping uses fixed caller-supplied line advance, leaves field_14
  // alone, resets StringHeight, and makes a fresh no-break strlen call.
  Buffer centered={{&primary_font,0,0,0},0,11,6};
  char center_text[]="abcd";
  measure_width=13; BufferStrHeight=29; reset_observations();
  split_at[0]=2; split_at[1]=0;
  assert(centered.wrap_cent(center_text,40,10,30,7)==24);
  assert(events.size()==6 && events[0]=='F' && events[1]=='M' && events[2]=='W' &&
         events[3]=='F' && events[4]=='M' && events[5]=='W');
  assert(find_widths.size()==2 && find_widths[0]==30 && find_widths[1]==30);
  assert(find_lengths[0]==4 && find_lengths[1]==2);
  assert(find_strlen_counts[0]==1 && find_strlen_counts[1]==4);
  assert(measure_strlen_counts[0]==3 && measure_strlen_counts[1]==7);
  assert(write_xs[0]==48 && write_xs[1]==48);
  assert(write_ys[0]==10 && write_ys[1]==17);
  assert(write_lengths[0]==2 && write_lengths[1]==2);
  assert(test_strlen_calls==7 && centered.field_18_==2 && centered.field_14_==11);
  assert(BufferStrHeight==0);

  // Null centered text returns the input y before touching its counters/state.
  reset_observations(); BufferStrHeight=43; centered.field_18_=9;
  assert(centered.wrap_cent(0,40,51,30,7)==51);
  assert(events.empty() && test_strlen_calls==0 && BufferStrHeight==43 &&
         centered.field_18_==9);
  return 0;
}
'''


def source_program(mutation: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    definitions = {
        name: extract_definition(source, *identity)
        for name, identity in FUNCTIONS.items()
    }
    if mutation == "cached_no_break_strlen":
        definitions["wrap_cent"] = definitions["wrap_cent"].replace(
            "static_cast<int>(strlen(current))", "static_cast<int>(length)", 1)
        definitions["wrap_right"] = definitions["wrap_right"].replace(
            "static_cast<int>(strlen(text))", "static_cast<int>(length)", 1)
    elif mutation == "wrong_line_width":
        for name in ("wrap_cent", "wrap_right"):
            assignments = (
                ("int line_width = width;", "int line_width = width - 1;"),
                ("y_coord = max_width;", "y_coord = max_width - 1;"),
            )
            matches = [(old, new) for old, new in assignments
                       if old in definitions[name]]
            assert len(matches) == 1, name
            old, new = matches[0]
            definitions[name] = definitions[name].replace(old, new, 1)
    elif mutation == "wrong_center_advance":
        old = "y_coord += line_advance;"
        assert old in definitions["wrap_cent"]
        definitions["wrap_cent"] = definitions["wrap_cent"].replace(
            old, "y_coord += line_advance + 1;", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return (HARNESS + extract_buffer_min_macro(source) + "\n" +
            "\n".join(definitions.values()) + MAIN)


COMPILERS = [compiler for compiler in
             (shutil.which("g++"), shutil.which("clang++")) if compiler]


def compile_run(tmp_path: Path, compiler: str, program: str,
                label: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"buffer_wrap_family_{Path(compiler).name}_{label}.cpp"
    executable = tmp_path / f"buffer_wrap_family_{Path(compiler).name}_{label}"
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if built.returncode:
        return built
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_center_and_scalar_wrap_bodies_preserve_paths_and_state(
        compiler: str, tmp_path: Path) -> None:
    result = compile_run(tmp_path, compiler, source_program(), "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutation", [
    "cached_no_break_strlen", "wrong_line_width", "wrong_center_advance",
])
def test_wrap_family_controls_reject_fresh_length_width_and_coordinate_mutants(
        compiler: str, mutation: str, tmp_path: Path) -> None:
    result = compile_run(tmp_path, compiler, source_program(mutation), mutation)
    assert result.returncode != 0, f"{compiler} accepted {mutation} mutation"
