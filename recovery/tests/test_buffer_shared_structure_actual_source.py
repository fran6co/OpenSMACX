"""Bounded actual flying-height operations with callback-controlled dependencies.

The host doubles test pinned read phases and shared height effects. They do not
establish actual CRT reentrancy, Win32 layout, signed-overflow behavior or game
acceptance; the native pinned function checks measure the production compiler.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_secret_project_rules_owner_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]

HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
struct Font { int unk_1_, height_, line_height_; };
Font font={-1,4,9};
Font *FontDefault=&font;
int BufferFlyingWidth, BufferStrHeight, BufferFlyingY;
const char BufferMeasureSpace[]=" ";
struct Buffer {
    Font *fonts_[1];
    int field_14_,field_18_;
    int text_line_height();
    int wrap_height_flying(char *);
    char *find_line_break_l(char *,int *,int);
    int text_width(char *,int);
};
static Buffer *active;
static int calls,finds,expected_y;
static std::vector<char> events;
static size_t counted_strlen(const char *text) {
    events.push_back('S');
    ++calls;
    if (calls==1) {
        assert(BufferFlyingWidth==100 && BufferStrHeight==20);
        BufferFlyingWidth=80; BufferStrHeight=7;
    } else if(calls==2) {
        assert(BufferFlyingY==expected_y && BufferStrHeight==0);
        assert(BufferFlyingWidth==75 && active->field_14_==0);
        BufferFlyingWidth=70; BufferStrHeight=9;
    } else {
        assert(calls==3 && text==BufferMeasureSpace);
    }
    return std::strlen(text);
}
#define strlen counted_strlen
char *Buffer::find_line_break_l(char *text,int *width,int length) {
    events.push_back('F'); ++finds;
    if(finds==1) {
        assert(*width==73 && length==3 && calls==1);
        assert(field_18_==0);
        fonts_[0]=0;
        return text+1;
    }
    assert(finds==2 && *width==61 && length==2 && calls==2);
    assert(field_18_==1 && fonts_[0]==FontDefault);
    return 0;
}
int Buffer::text_width(char *text,int length) {
    events.push_back('W');
    assert(text==BufferMeasureSpace && length==1 && calls==3);
    return 4;
}
ACTUAL_HEIGHT
ACTUAL_FLYING
#undef strlen
static void scenario(int override_value,int result_y) {
    font.unk_1_=override_value;
    Buffer b={{&font},5,0}; active=&b;
    BufferFlyingWidth=100; BufferStrHeight=20; BufferFlyingY=11;
    calls=finds=0; events.clear(); expected_y=result_y;
    char input[]="abc";
    assert(b.wrap_height_flying(input)==result_y);
    assert(BufferFlyingY==result_y && BufferFlyingWidth==70);
    assert(BufferStrHeight==13 && b.field_18_==1 && b.field_14_==0);
    assert((events==std::vector<char>{'S','F','S','F','S','W'}));
}
int main() {
    scenario(-1,20); // negative override selects line_height 9
    scenario(3,18);  // nonnegative override selects height4 + override3
    Buffer b={{0},5,6}; active=&b; calls=finds=0; events.clear();
    BufferFlyingWidth=0; BufferFlyingY=17; BufferStrHeight=23;
    char input[]="abc";
    assert(b.wrap_height_flying(input)==0);
    assert(calls==0 && b.fonts_[0]==0 && b.field_18_==6);
    assert(BufferFlyingY==17 && BufferStrHeight==23 && b.field_14_==5);
    BufferFlyingWidth=1;
    assert(b.wrap_height_flying(0)==17 && b.fonts_[0]==0 && calls==0);
    char empty[]="";
    assert(b.wrap_height_flying(empty)==17 && b.fonts_[0]==FontDefault);
    assert(calls==0 && events.empty() && b.field_14_==5 && b.field_18_==6);
    assert(BufferStrHeight==23 && BufferFlyingWidth==1);
}
'''


def actual_operations():
    source = (ROOT / "src/buffer.cpp").read_text()
    return (
        extract_function(source, "int Buffer::text_line_height()"),
        extract_function(source, "int Buffer::wrap_height_flying(char *input_text)"),
    )


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("damage", ["", "cached_width", "retain_running_width", "retain_carry"])
def test_flying_height_live_reads_and_shared_height_effects(tmp_path, compiler, damage):
    height, flying = actual_operations()
    if damage == "cached_width":
        old = ("int length = static_cast<int>(strlen(text));\n"
               "                int remaining = BufferFlyingWidth - BufferStrHeight;")
        new = ("int remaining = BufferFlyingWidth - BufferStrHeight;\n"
               "                int length = static_cast<int>(strlen(text));")
        assert flying.count(old) == 1
        flying = flying.replace(old, new, 1)
    elif damage:
        old = "BufferStrHeight = 0;" if damage == "retain_running_width" else "field_14_ = 0;"
        assert flying.count(old) == 1
        flying = flying.replace(old, "", 1)
    unit = HARNESS.replace("ACTUAL_HEIGHT", height).replace("ACTUAL_FLYING", flying)
    source, binary = tmp_path / "flying.cpp", tmp_path / "flying"
    source.write_text(unit)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if damage:
        assert ran.returncode != 0 and "Assertion" in ran.stderr, ran.stderr
    else:
        assert ran.returncode == 0, ran.stderr


RECT_HARNESS = r'''#include <cassert>
#include <cstring>
typedef char *LPSTR;
struct RECT { int left,top,right,bottom; };
struct Font {
    void *font_obj_; int height_;
    bool is_initialized() const { return font_obj_!=0; }
};
Font initial_font={&initial_font,10}, default_font={&default_font,4};
Font *FontDefault=&default_font;
struct Buffer {
    Font *fonts_[1];
    int text_height();
    int write_l(LPSTR,RECT *,int);
    int write_multi_font_raw_l(LPSTR,int,int,int);
};
static Buffer *active;
static RECT *caller_rect;
static int lengths,writes;
static size_t observed_strlen(const char *text) {
    ++lengths;
    if(lengths==4) {
        caller_rect->left=11; caller_rect->top=20;
        caller_rect->right=50; caller_rect->bottom=80;
        active->fonts_[0]=0;
    }
    return std::strlen(text);
}
#define strlen observed_strlen
ACTUAL_MIN
ACTUAL_TEXT_HEIGHT
ACTUAL_RECT_WRITER
#undef strlen
int Buffer::write_multi_font_raw_l(LPSTR text,int x,int y,int length) {
    ++writes;
    assert(text[0]=='a' && x==11 && y==48 && length==4);
    assert(lengths==4 && fonts_[0]==FontDefault);
    return 17;
}
int main() {
    Buffer b={{&initial_font}}; active=&b;
    RECT area={3,5,30,40}; caller_rect=&area;
    char text[]="abcd";
    assert(b.write_l(text,&area,6)==17 && writes==1);
    assert(area.left==11 && area.top==20 && area.right==50 && area.bottom==80);
    assert(b.write_l(0,&area,6)==0);
    assert(b.write_l(text,0,6)==0);
    b.fonts_[0]=0;
    assert(b.write_l(text,&area,6)==0 && b.fonts_[0]==0);
    assert(lengths==4 && writes==1);
}
'''


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("early_snapshot", [False, True])
def test_rect_writer_snapshot_after_length_calls_and_default_height(
        tmp_path, compiler, early_snapshot):
    source = (ROOT / "src/buffer.cpp").read_text()
    height = extract_function(source, "int Buffer::text_height()")
    writer = extract_function(source, "int Buffer::write_l(LPSTR text, RECT *rect, int len)")
    if early_snapshot:
        old = "const RECT area = *rect;"
        assert writer.count(old) == 1 and writer.count("int limit;") == 1
        writer = writer.replace(old, "", 1).replace("int limit;", old + "\n    int limit;", 1)
    macro, = [line for line in source.splitlines() if line.startswith("#define BUFFER_MIN(")]
    unit = (RECT_HARNESS.replace("ACTUAL_MIN", macro)
            .replace("ACTUAL_TEXT_HEIGHT", height)
            .replace("ACTUAL_RECT_WRITER", writer))
    path, binary = tmp_path / "rect.cpp", tmp_path / "rect"
    path.write_text(unit)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(path), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if early_snapshot:
        assert ran.returncode != 0 and "Assertion" in ran.stderr, ran.stderr
    else:
        assert ran.returncode == 0, ran.stderr
