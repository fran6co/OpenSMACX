"""Actual-source controls for SocialWin category-header/model RECT ownership."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def block(text: str, start_anchor: str, end_anchor: str) -> str:
    start = text.index(start_anchor)
    end = text.index(end_anchor, start)
    return text[start:end]


def compile_run(tmp_path, compiler, text):
    source = tmp_path / "socialwin-category-rectangles.cpp"
    executable = tmp_path / "socialwin-category-rectangles"
    source.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if build.returncode != 0:
        return build, None
    return build, subprocess.run([str(executable)], capture_output=True,
                                 text=True, timeout=10)


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


def initialization_fragment():
    text = SOURCE.read_text()
    init = body(text, "void SocialWin::init(")
    return block(init, "    categoryHeaders_[0].left =", "    tooltipFactionPower_.left =")


def draw_header_and_tail(text):
    draw = body(text, "void SocialWin::draw_social(int a1)")
    start = draw.index("    RECT *headerSlot = &categoryHeaders_[a1];")
    clip = draw.index("    buffer->box_sprite(&rect1, &InterfaceBoxStyles[3]);", start)
    tail_at = draw.index("    soft_update(headerSlot);", clip)
    tail_end = draw.index("\n}", tail_at)
    selection = draw[start:clip]
    tail = draw[tail_at:tail_end]
    assert selection.index("RECT *headerSlot") < selection.index("RECT rect1 = *headerSlot;")
    assert selection.index("RECT rect1 = *headerSlot;") < selection.index("buffer->set_clip(&rect1);")
    assert tail.index("soft_update(headerSlot);") < tail.index("do_sound();")
    return selection, tail


INIT_HARNESS = r'''#include <cassert>
struct RECT { int left, top, right, bottom; };
struct SocialWin {
  RECT categoryHeaders_[4];
  RECT modelRects_[16];
  void init_rectangles() {
BLOCK
  }
};
static bool same(const RECT &a, const RECT &b) {
  return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
int main() {
  SocialWin win;
  win.init_rectangles();
  const RECT headers[4]={{0x1a,4,0x26e,0x4a},{0x1a,0x4d,0x26e,0x93},
                         {0x1a,0x96,0x26e,0xdc},{0x1a,0xdf,0x26e,0x125}};
  const RECT cells[16]={{0x1d,0x1d,0xaf,0x31},{0xb1,0x1d,0x143,0x31},
    {0x145,0x1d,0x1d7,0x31},{0x1d9,0x1d,0x26b,0x31},
    {0x1d,0x66,0xaf,0x7a},{0xb1,0x66,0x143,0x7a},
    {0x145,0x66,0x1d7,0x7a},{0x1d9,0x66,0x26b,0x7a},
    {0x1d,0xaf,0xaf,0xc3},{0xb1,0xaf,0x143,0xc3},
    {0x145,0xaf,0x1d7,0xc3},{0x1d9,0xaf,0x26b,0xc3},
    {0x1d,0xf8,0xaf,0x10c},{0xb1,0xf8,0x143,0x10c},
    {0x145,0xf8,0x1d7,0x10c},{0x1d9,0xf8,0x26b,0x10c}};
  for(int i=0;i<4;++i) assert(same(win.categoryHeaders_[i],headers[i]));
  for(int i=0;i<16;++i) assert(same(win.modelRects_[i],cells[i]));
}
'''


def mouse_harness(mouse_body: str) -> str:
    return r'''#include <cassert>
#include <cstdint>
struct RECT { int left, top, right, bottom; };
struct Win { static int clears; static void clear_bubble_text(){++clears;} };
int Win::clears=0;
struct SocialWin;
SocialWin *current=0;
int availability[4][4];
int availability_calls=0, in_box_calls=0;
int hit_first=1, hit_later=1, hit_power=0;
int first_seen_top=-1, later_seen_top=-1, redraws=0;
struct SocialWin : Win {
  RECT categoryHeaders_[4], modelRects_[16], tooltipFactionPower_;
  int factionID_=3;
  uint32_t field_A2C_=0xffffffffu;
  void on_mouse_move(int,int);
  void draw_mouseover(){++redraws;}
};
int society_avail(int category,int row,int faction) {
  assert(faction==3); ++availability_calls;
  current->categoryHeaders_[category].top=70+availability_calls;
  return availability[category][row];
}
int in_box(int,int,RECT *rect) {
  ++in_box_calls;
  if(rect==&current->tooltipFactionPower_) return hit_power;
  if(rect->left==320) {first_seen_top=rect->top; return hit_first;}
  if(rect->left==520) {later_seen_top=rect->top; return hit_later;}
  return 0;
}
BODY
int main() {
  SocialWin win; current=&win;
  for(int cat=0;cat<4;++cat) {
    win.categoryHeaders_[cat]={100+cat,0,200+cat,10};
    for(int row=0;row<4;++row) {
      int index=cat*4+row;
      win.modelRects_[index]={300+20*index,20,310+20*index,30};
      availability[cat][row]=0;
    }
  }
  availability[0][1]=1; availability[2][3]=1;
  win.on_mouse_move(1,2);
  assert(availability_calls==14 && in_box_calls==3);
  assert(first_seen_top==72 && later_seen_top==80);
  assert(win.field_A2C_==11 && redraws==1 && Win::clears==2);
  hit_first=0; hit_later=0; hit_power=1;
  win.on_mouse_move(1,2);
  assert(win.field_A2C_==static_cast<uint32_t>(-2) && redraws==2);
  return 0;
}
'''.replace("BODY", mouse_body)


def right_click_harness(click_body: str) -> str:
    return r'''#include <cassert>
#include <cstring>
struct RECT { int left, top, right, bottom; };
struct Win {
  static int resets, bubble_calls, last_label;
  static RECT bubble_rect; static char bubble_text[32];
  static void clear_bubble_text() {}
  void set_bubble_text(char *text, RECT *rect) {
    ++bubble_calls; bubble_rect=*rect;
    std::strncpy(bubble_text,text,sizeof(bubble_text)-1);
  }
};
int Win::resets=0; int Win::bubble_calls=0; int Win::last_label=-1;
RECT Win::bubble_rect; char Win::bubble_text[32];
struct Tutorial { void reset(){++Win::resets;} } TutorialWindow;
struct SocialWin : Win {
  RECT categoryHeaders_[4], tooltipFactionPower_;
  void on_right_click(int,int);
};
SocialWin *current=0;
int target_header=-1, hit_power=0, box_calls=0, screen_calls=0;
int in_box(int,int,RECT *rect) {
  ++box_calls;
  for(int i=0;i<4;++i) if(rect==&current->categoryHeaders_[i])
    return i==target_header;
  return rect==&current->tooltipFactionPower_ ? hit_power : 0;
}
char *label_get(int id) {
  Win::last_label=id;
  return const_cast<char *>(id==828 ? "category" : "faction");
}
void client_to_screen(int *x,int *y){++screen_calls;*x+=10;*y+=20;}
BODY
int main() {
  SocialWin win; current=&win;
  for(int i=0;i<4;++i) win.categoryHeaders_[i]={i,0,i+1,1};
  win.tooltipFactionPower_={9,0,10,1};
  for(int i=0;i<4;++i) {
    target_header=i; hit_power=0; box_calls=screen_calls=0;
    Win::bubble_calls=0; Win::last_label=-1;
    win.on_right_click(3,4);
    assert(Win::resets==i+1 && Win::last_label==828);
    assert(Win::bubble_calls==1 && screen_calls==1);
    assert(Win::bubble_rect.left==13 && Win::bubble_rect.top==24 &&
           Win::bubble_rect.right==13 && Win::bubble_rect.bottom==24);
    assert(std::strcmp(Win::bubble_text,"category")==0);
    assert(box_calls==i+1);
  }
  target_header=-1; hit_power=1; box_calls=screen_calls=0;
  Win::bubble_calls=0; Win::last_label=-1;
  win.on_right_click(5,6);
  assert(Win::resets==5 && Win::last_label==840 && Win::bubble_calls==1);
  assert(std::strcmp(Win::bubble_text,"faction")==0);
  target_header=2; hit_power=1; Win::last_label=-1; box_calls=0;
  win.on_right_click(5,6);
  assert(Win::last_label==828 && box_calls==3);
  target_header=-1; hit_power=0; Win::last_label=-1;
  Win::bubble_calls=0; win.on_right_click(5,6);
  assert(Win::last_label==-1 && Win::bubble_calls==0);
  return 0;
}
'''.replace("BODY", click_body)


def draw_header_harness(selection: str, tail: str) -> str:
    return r'''#include <cassert>
#include <vector>
struct RECT { int left, top, right, bottom; };
static std::vector<int> calls;
struct Buffer { RECT clipped; void set_clip(RECT *r) { clipped=*r; calls.push_back(1); } };
struct SocialWin {
  RECT categoryHeaders_[4]; Buffer b;
  int selected=-1; int sounds=0;
  void soft_update(RECT *r) { assert(r==&categoryHeaders_[selected]); calls.push_back(2); }
  void do_sound() { ++sounds; calls.push_back(3); }
  void run(int a1) { selected=a1; Buffer *buffer=&b;
SELECTION
TAIL
  }
};
int main() {
  SocialWin win;
  for(int i=0;i<4;++i) win.categoryHeaders_[i]={10+i,20+i,30+i,40+i};
  for(int i=0;i<4;++i) {
    calls.clear(); win.run(i);
    assert(win.b.clipped.left==10+i && win.b.clipped.top==20+i);
    assert(calls.size()==3 && calls[0]==1 && calls[1]==2 && calls[2]==3);
    assert(win.sounds==i+1);
  }
}
'''.replace("SELECTION", selection).replace("TAIL", tail)


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_initialized_four_headers_and_sixteen_cells(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness = INIT_HARNESS.replace("BLOCK", initialization_fragment())
    build, result = compile_run(tmp_path, compiler, harness)
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_hover_read_timing_skips_and_later_category_wins(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    mouse = body(source, "void SocialWin::on_mouse_move(int a1, int a2)")
    harness = mouse_harness(mouse)
    build, result = compile_run(tmp_path, compiler, harness)
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"

    early = mouse.replace(
        "            if (society_avail(category, row, factionID_) == 0) {",
        "            RECT cached_header = categoryHeaders_[category];\n"
        "            if (society_avail(category, row, factionID_) == 0) {",
        1,
    ).replace("            RECT rect = categoryHeaders_[category];",
              "            RECT rect = cached_header;", 1)
    skip = mouse.replace("society_avail(category, row, factionID_) == 0",
                         "society_avail(category, row, factionID_) != 0", 1)
    for name, mutant in (("early-header-read", early), ("inverted-availability", skip)):
        assert mutant != mouse
        build, result = compile_run(tmp_path, compiler, mouse_harness(mutant))
        assert build.returncode == 0, build.stderr
        assert result is not None and result.returncode != 0, name


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_right_click_header_and_faction_tooltips(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    click = body(source, "void SocialWin::on_right_click(int x, int y)")
    build, result = compile_run(tmp_path, compiler, right_click_harness(click))
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_draw_social_retains_selected_header_for_tail_update(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    selection, tail = draw_header_and_tail(SOURCE.read_text())
    build, result = compile_run(tmp_path, compiler, draw_header_harness(selection, tail))
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"

    wrong_tail = tail.replace("soft_update(headerSlot);", "soft_update(&rect1);")
    assert wrong_tail != tail
    build, result = compile_run(tmp_path, compiler, draw_header_harness(selection, wrong_tail))
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode != 0
