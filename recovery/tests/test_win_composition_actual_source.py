"""Actual-source Win controls with deterministic positive/mutant host probes.

C++11 is used only for host fixture convenience. These tests do not establish
VC6 ABI, linked preservation, byte equality or full game acceptance.
"""

import pathlib
import re
import shutil
import subprocess
import tempfile

from match_recovery.source import Mode, read_file


ROOT = next(parent for parent in pathlib.Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file())
WIN_CPP = ROOT / "src" / "win.cpp"
WIN_IDENTITIES = {
    0x5EF640: (774, "draw_rect_edges", "?draw_rect_edges@Win@@QAEXPAUtagRECT@@PAX1H@Z"),
    0x5EFBA0: (341, "fill_palette_color_rect", "?fill_palette_color_rect@Win@@QAEXHHHHHH@Z"),
    0x5F37C0: (947, "copy_edges_to_buffer", "?copy_edges_to_buffer@Win@@QAEXPAVBuffer@@HH@Z"),
    0x5F3B80: (3879, "copy_border_buffers_to_window", "?copy_border_buffers_to_window@Win@@QAEXPAV1@PAUtagRECT@@H@Z"),
}
EMITTED_SYMBOLS = {
    0x5EF640: "?draw_rect_edges@Win@@QAEXPAUtagRECT@@PAX1H@Z",
    0x5EFBA0: "?fill_palette_color_rect@Win@@QAEXHHHHHH@Z",
    0x5F37C0: "?copy_edges_to_buffer@Win@@QAEXPAVBuffer@@HH@Z",
    0x5F3B80: "?copy_border_buffers_to_window@Win@@QAEXPAV1@PAUtagRECT@@H@Z",
}


def _original_rows():
    rows = {}
    for row in read_file(WIN_CPP):
        if row.mode is Mode.FILE and row.address in WIN_IDENTITIES:
            rows[row.address] = row
    return rows


def _definition(text, qualified_name):
    match = re.search(r"\b(?:void|int)\s+" + re.escape(qualified_name) +
                      r"\s*\([^;]*?\)\s*\{", text, re.S)
    assert match is not None, "missing product definition: " + qualified_name
    opening = text.index("{", match.start())
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():index + 1]
    raise AssertionError("unterminated product definition: " + qualified_name)


def _mutate_once(body, old, new):
    assert old in body, "mutant anchor missing: " + old
    changed = body.replace(old, new, 1)
    assert changed != body
    return changed


def _product_bodies():
    rows = _original_rows()
    assert set(rows) == set(WIN_IDENTITIES), "mapped Win identity set changed"
    for address, (length, _, symbol) in WIN_IDENTITIES.items():
        row = rows[address]
        assert row.name == "sub_" + format(address, "x")
        assert row.symbol == EMITTED_SYMBOLS[address]
        assert row.path.resolve() == WIN_CPP.resolve()
        assert row.image_spans == ((address, address + length),)
        assert row.symbol == symbol
    text = WIN_CPP.read_text()
    bodies = {}
    for address, (_, method, _) in WIN_IDENTITIES.items():
        bodies[address] = _definition(text, "Win::" + method)
    return bodies


def _fixture():
    return r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

typedef void *HDC;
typedef void *HGDIOBJ;
typedef void *HBRUSH;
typedef unsigned int COLORREF;
typedef void *HWND;
struct RECT { int left, top, right, bottom; };
struct Header { int biWidth, biHeight; };
struct Dib { Header bmiHeader; };
struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };
#define RGB(r,g,b) ((COLORREF)((r) | ((g) << 8) | ((b) << 16)))

struct CopyCall {
  const void *source;
  const void *destination;
  int sx, sy, dx, dy, width, height;
};
static std::vector<std::string> events;
static std::vector<CopyCall> copies;
static HBRUSH brush_result = reinterpret_cast<HBRUSH>(0x44);
static RECT last_fill;
static COLORREF last_color;
static RECT window_rect = {0, 0, 640, 480};
static int release_count;
static int get_calls;
static int failed_get_call;
static HWND HandleMain = reinterpret_cast<HWND>(0x1234);

static int IsRectEmpty(const RECT *r) {
  return r->left >= r->right || r->top >= r->bottom;
}
static int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
  RECT r;
  r.left = a->left > b->left ? a->left : b->left;
  r.top = a->top > b->top ? a->top : b->top;
  r.right = a->right < b->right ? a->right : b->right;
  r.bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
  *out = r;
  return !IsRectEmpty(out);
}
static int GetWindowRect(HWND, RECT *r) { *r = window_rect; return 1; }

class Palette {
 public:
  PALETTEENTRY values[256];
  const PALETTEENTRY *entries() const { return values; }
};
static Palette palette;
Palette *PaletteActive = &palette;

class Buffer {
 public:
  RECT rect2_;
  Dib dib_;
  Buffer() {
    rect2_ = RECT{0, 0, 20, 20};
    dib_.bmiHeader.biWidth = 20;
    dib_.bmiHeader.biHeight = -20;
  }
  int copy(Buffer *destination, int sx, int sy, int dx, int dy,
           int width, int height) {
    copies.push_back(CopyCall{this, destination, sx, sy, dx, dy, width, height});
    return 0;
  }
};
class GraphicWin;
class Win {
 public:
  unsigned int iFlags_;
  unsigned char iSomeFlag_;
  RECT client_rect_;
  Buffer *buffer1_, *buffer2_, *buffer3_, *buffer4_;
  GraphicWin *graphic_;
  Win() : iFlags_(0), iSomeFlag_(0), buffer1_(0), buffer2_(0),
          buffer3_(0), buffer4_(0), graphic_(0) {
    client_rect_ = RECT{0, 0, 0, 0};
  }
  static HDC get_hdc() {
    events.push_back("get");
    ++get_calls;
    if (get_calls == failed_get_call) return 0;
    return reinterpret_cast<HDC>(0x11);
  }
  static void release_hdc() { events.push_back("release"); ++release_count; }
  void sync_palette() { events.push_back("sync"); }
  virtual GraphicWin *get_graphic_win() { return graphic_; }
  void draw_rect_edges(RECT *, HGDIOBJ, HGDIOBJ, int);
  void fill_palette_color_rect(int, int, int, int, int, int);
  void copy_edges_to_buffer(Buffer *, int, int);
  void copy_border_buffers_to_window(Win *, RECT *, int);
};
class GraphicWin : public Win, public Buffer {
 public:
  Buffer *owned_;
  GraphicWin() : owned_(0) {}
  Buffer *owned_buffer() const { return owned_; }
  GraphicWin *get_graphic_win() { return Win::get_graphic_win(); }
};

static HGDIOBJ SelectObject(HDC, HGDIOBJ object) {
  events.push_back(object == reinterpret_cast<HGDIOBJ>(0x21) ? "select-light" :
                   object == reinterpret_cast<HGDIOBJ>(0x22) ? "select-dark" :
                   "select-old");
  return reinterpret_cast<HGDIOBJ>(0x33);
}
static int MoveToEx(HDC, int x, int y, void *) {
  events.push_back("move:" + std::to_string(x) + ":" + std::to_string(y));
  return 1;
}
static int LineTo(HDC, int x, int y) {
  events.push_back("line:" + std::to_string(x) + ":" + std::to_string(y));
  return 1;
}
static HBRUSH CreateSolidBrush(COLORREF color) {
  events.push_back("brush"); last_color = color; return brush_result;
}
static int FillRect(HDC, const RECT *r, HBRUSH) {
  events.push_back("fill"); last_fill = *r; return 1;
}
static int DeleteObject(HGDIOBJ) { events.push_back("delete"); return 1; }
'''


def _compile_and_run(bodies, main, expect_success=True):
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest

        pytest.skip("host g++ is unavailable")
    with tempfile.TemporaryDirectory(prefix="osmx-product-win-ui-") as temp:
        source = pathlib.Path(temp) / "candidate.cpp"
        executable = pathlib.Path(temp) / "candidate"
        source.write_text(_fixture() + "\n".join(bodies.values()) + main)
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", str(source), "-o", str(executable)],
            capture_output=True,
            text=True, timeout=30,
        )
        assert compiled.returncode == 0, compiled.stderr
        ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        if expect_success:
            assert ran.returncode == 0, ran.stderr
        else:
            assert ran.returncode != 0
            assert "assert" in ran.stderr.lower(), ran.stderr


def test_source_api_binds_all_original_spans_and_product_methods():
    bodies = _product_bodies()
    assert set(bodies) == set(WIN_IDENTITIES)


def test_product_methods_preserve_native_geometry_order_and_lifetimes():
    bodies = _product_bodies()
    main = r'''
static void set_buffer(Buffer &b, int l, int t, int r, int bot,
                       int width, int height) {
  b.rect2_ = RECT{l, t, r, bot};
  b.dib_.bmiHeader.biWidth = width;
  b.dib_.bmiHeader.biHeight = -height;
}
int main() {
  Win win;
  RECT r = {2, 3, 10, 20};
  win.draw_rect_edges(&r, reinterpret_cast<HGDIOBJ>(0x21),
                      reinterpret_cast<HGDIOBJ>(0x22), 777);
  assert(events.size() == 28);
  assert(events[0] == "get" && events[1] == "sync" &&
         events[3] == "move:3:3" && events[4] == "line:11:3");
  events.clear(); get_calls = 0; release_count = 0; failed_get_call = 2;
  win.draw_rect_edges(&r, reinterpret_cast<HGDIOBJ>(0x21),
                      reinterpret_cast<HGDIOBJ>(0x22), 777);
  assert(get_calls == 4 && release_count == 3);  // later borders still run

  events.clear(); failed_get_call = 0; release_count = 0;
  palette.values[7].peRed = 1; palette.values[7].peGreen = 2;
  palette.values[7].peBlue = 3; brush_result = 0;
  win.fill_palette_color_rect(4, 5, 6, 7, 7, 999);
  assert((events == std::vector<std::string>{"get", "sync", "brush"}));
  assert(release_count == 0);  // retain the native brush-failure HDC leak
  events.clear(); brush_result = reinterpret_cast<HBRUSH>(0x44);
  win.fill_palette_color_rect(4, 5, 6, 7, 7, 999);
  assert(last_color == 0x030201u && last_fill.right == 10 &&
         last_fill.bottom == 12 && release_count == 1);

  copies.clear();
  Buffer top, left, bottom, right, target;
  set_buffer(top, 0, 0, 100, 5, 100, 5);
  set_buffer(bottom, 0, 0, 100, 5, 100, 5);
  set_buffer(right, 0, 0, 5, 100, 5, 100);
  set_buffer(left, 0, 0, 5, 100, 5, 100);
  set_buffer(target, 0, 0, 200, 200, 200, 200);
  Win source;
  source.iSomeFlag_ = 2; source.client_rect_ = RECT{10, 20, 110, 120};
  source.buffer1_ = &top; source.buffer2_ = &left;
  source.buffer3_ = &bottom; source.buffer4_ = &right;
  source.copy_edges_to_buffer(&target, 10, 20);
  assert(copies.size() == 4);
  assert(copies[0].source == &top && copies[0].destination == &target);
  assert(copies[0].sx == 0 && copies[0].sy == 0 && copies[0].dx == 0 &&
         copies[0].dy == 0 && copies[0].width == 100 && copies[0].height == 5);
  assert(copies[1].source == &bottom && copies[2].source == &right &&
         copies[3].source == &left);

  copies.clear();
  GraphicWin current_source; GraphicWin unrelated_getter_result;
  current_source.graphic_ = &unrelated_getter_result;
  current_source.copy_edges_to_buffer(&target, 10, 20);
  assert(copies.size() == 1);
  assert(copies[0].source == static_cast<Buffer *>(&current_source) &&
         copies[0].destination == &target);

  copies.clear();
  Win destination;
  destination.iSomeFlag_ = 2; destination.iFlags_ = 0x10000000;
  destination.client_rect_ = RECT{10, 20, 110, 120};
  destination.buffer1_ = &top; destination.buffer2_ = &left;
  destination.buffer3_ = &bottom; destination.buffer4_ = &right;
  Buffer own_top, own_left, own_bottom, own_right;
  set_buffer(own_top, 0, 0, 100, 5, 100, 5);
  set_buffer(own_bottom, 0, 0, 100, 5, 100, 5);
  set_buffer(own_right, 0, 0, 5, 100, 5, 100);
  set_buffer(own_left, 0, 0, 5, 100, 5, 100);
  Win painter;
  painter.iSomeFlag_ = 2; painter.client_rect_ = RECT{10, 20, 110, 120};
  painter.buffer1_ = &own_top; painter.buffer2_ = &own_left;
  painter.buffer3_ = &own_bottom; painter.buffer4_ = &own_right;
  painter.copy_border_buffers_to_window(&destination, 0, 0);
  assert(!copies.empty());
  assert(copies[0].source == &own_top && copies[0].destination == &top);

  copies.clear();
  GraphicWin target_graphic; Buffer owned_screen;
  set_buffer(owned_screen, 0, 0, 640, 480, 640, 480);
  target_graphic.owned_ = &owned_screen;
  destination.graphic_ = &target_graphic;
  destination.iFlags_ |= 0x40000000;
  GraphicWin current_graphic;
  current_graphic.iSomeFlag_ = 2;
  current_graphic.client_rect_ = RECT{10, 20, 110, 120};
  GraphicWin other_current_graphic;
  current_graphic.graphic_ = &other_current_graphic;
  current_graphic.buffer1_ = &own_top; current_graphic.buffer2_ = &own_left;
  current_graphic.buffer3_ = &own_bottom; current_graphic.buffer4_ = &own_right;
  RECT clip = {0, 0, 640, 480};
  current_graphic.copy_border_buffers_to_window(&destination, &clip, 0);
  bool owned_is_target = false, inherited_is_source = false;
  for (std::size_t i = 0; i < copies.size(); ++i) {
        if (copies[i].destination == &owned_screen) {
          owned_is_target = true;
        }
    if (copies[i].source == static_cast<Buffer *>(&current_graphic))
      inherited_is_source = true;
  }
  assert(owned_is_target && inherited_is_source);
}
'''
    _compile_and_run(bodies, main)


def test_product_source_mutants_are_caught():
    bodies = _product_bodies()
    bad_hdc_order = dict(bodies)
    # Native code obtains each side independently: a failed second acquisition
    # does not skip the left and right edge attempts.  A shared early-return
    # refactor violates that ordering and leaks/omits later side effects.
    second_acquire = "    hdc = Win::get_hdc();\n    if (hdc != 0) {"
    first = bad_hdc_order[0x5EF640].find(second_acquire)
    second = first
    assert second >= 0
    bad_hdc_order[0x5EF640] = (
        bad_hdc_order[0x5EF640][:second]
        + "    hdc = Win::get_hdc();\n    if (hdc == 0) return;\n    if (hdc != 0) {"
        + bad_hdc_order[0x5EF640][second + len(second_acquire):]
    )
    hdc_control = r'''
int main() {
  Win win; RECT r = {0,0,10,10}; failed_get_call = 2;
  win.draw_rect_edges(&r, reinterpret_cast<HGDIOBJ>(0x21),
                      reinterpret_cast<HGDIOBJ>(0x22), 0);
  assert(get_calls == 4 && release_count == 3);
}
'''
    _compile_and_run(bodies, hdc_control)
    _compile_and_run(bad_hdc_order, hdc_control, expect_success=False)

    bad_geometry = dict(bodies)
    bad_geometry[0x5F37C0] = _mutate_once(
        bad_geometry[0x5F37C0],
        "damage.bottom = y - target->dib_.bmiHeader.biHeight;",
        "damage.bottom = y + target->dib_.bmiHeader.biHeight;",
    )
    geometry_control = r'''
int main() {
  Buffer target; target.dib_.bmiHeader.biWidth = 20;
  target.dib_.bmiHeader.biHeight = -30;
  GraphicWin win, getter_result; win.graphic_ = &getter_result;
  win.copy_edges_to_buffer(&target, 4, 7);
  // Bad sign makes the native damage RECT empty and suppresses copy calls.
  assert(!copies.empty());
}
'''
    _compile_and_run(bodies, geometry_control)
    _compile_and_run(bad_geometry, geometry_control, expect_success=False)

    bad_copy_roles = dict(bodies)
    bad_copy_roles[0x5F3B80] = _mutate_once(
        bad_copy_roles[0x5F3B80],
        "(destination)->copy(target->buffer1_,",
        "target->buffer1_->copy(destination,",
    )
    role_control = r'''
int main() {
  Buffer src, dst; src.rect2_ = RECT{0,0,20,5};
  dst.rect2_ = RECT{0,0,20,5};
  Win target; target.iSomeFlag_ = 2; target.iFlags_ = 0x10000000;
  target.client_rect_ = RECT{0,0,20,5}; target.buffer1_ = &dst;
  target.buffer2_ = target.buffer3_ = target.buffer4_ = &dst;
  Win painter; painter.iSomeFlag_ = 2; painter.buffer1_ = &src;
  painter.buffer2_ = painter.buffer3_ = painter.buffer4_ = &src;
  painter.copy_border_buffers_to_window(&target, 0, 0);
  assert(copies[0].source == &src && copies[0].destination == &dst);
}
'''
    _compile_and_run(bodies, role_control)
    _compile_and_run(bad_copy_roles, role_control, expect_success=False)

    bad_owned_role = dict(bodies)
    bad_owned_role[0x5F3B80] = _mutate_once(
        bad_owned_role[0x5F3B80],
        "target_graphic->owned_buffer()", "target_graphic",
    )
    owned_control = r'''
int main() {
  Buffer src, dst, owned; GraphicWin target_graphic;
  target_graphic.owned_ = &owned;
  Win target; target.iSomeFlag_ = 2;
  target.iFlags_ = 0x10000000 | 0x40000000;
  target.client_rect_ = RECT{0,0,20,5}; target.buffer1_ = &dst;
  target.buffer2_ = target.buffer3_ = target.buffer4_ = &dst;
  target.graphic_ = &target_graphic;
  Win painter; painter.iSomeFlag_ = 2; painter.buffer1_ = &src;
  painter.buffer2_ = painter.buffer3_ = painter.buffer4_ = &src;
  RECT clip = {0,0,640,480};
  painter.copy_border_buffers_to_window(&target, &clip, 0);
  bool saw = false;
  for (std::size_t i=0; i<copies.size(); ++i)
    if (copies[i].destination == &owned) saw = true;
  assert(saw);
}
'''
    _compile_and_run(bodies, owned_control)
    _compile_and_run(bad_owned_role, owned_control, expect_success=False)

    bad_getter_role = dict(bodies)
    bad_getter_role[0x5F37C0] = _mutate_once(
        bad_getter_role[0x5F37C0],
        "static_cast<GraphicWin *>(this)->Buffer::copy(",
        "graphic->Buffer::copy(",
    )
    getter_control = r'''
int main() {
  Buffer target; target.dib_.bmiHeader.biWidth = 20;
  target.dib_.bmiHeader.biHeight = -30;
  GraphicWin current, other; current.graphic_ = &other;
  current.copy_edges_to_buffer(&target, 4, 7);
  assert(copies.size() == 1);
  assert(copies[0].source == static_cast<Buffer *>(&current));
}
'''
    _compile_and_run(bodies, getter_control)
    _compile_and_run(bad_getter_role, getter_control, expect_success=False)

    bad_border_getter = _mutate_once(
        bodies[0x5F3B80],
        "COPY_EDGE_TO_TARGET(static_cast<Buffer *>(static_cast<GraphicWin *>(this)), screen_rect);",
        "COPY_EDGE_TO_TARGET(static_cast<Buffer *>(graphic), screen_rect);",
    )
    bad_border_bodies = dict(bodies)
    bad_border_bodies[0x5F3B80] = bad_border_getter
    border_getter_control = r'''
int main() {
  Buffer top, left, bottom, right, target_top, target_left;
  Buffer target_bottom, target_right, owned;
  GraphicWin target_graphic, current, other;
  target_graphic.owned_ = &owned;
  Win target; target.iSomeFlag_ = 2;
  target.iFlags_ = 0x10000000 | 0x40000000;
  target.client_rect_ = RECT{10,20,110,120};
  target.buffer1_ = &target_top; target.buffer2_ = &target_left;
  target.buffer3_ = &target_bottom; target.buffer4_ = &target_right;
  target.graphic_ = &target_graphic;
  current.iSomeFlag_ = 2; current.client_rect_ = RECT{10,20,110,120};
  current.buffer1_ = &top; current.buffer2_ = &left;
  current.buffer3_ = &bottom; current.buffer4_ = &right;
  current.graphic_ = &other;
  RECT clip = {0,0,640,480};
  current.copy_border_buffers_to_window(&target, &clip, 0);
  bool found = false, saw_current_source = false;
  for (std::size_t i=0; i<copies.size(); ++i)
    if (copies[i].destination == &owned) {
      found = true;
      if (copies[i].source == static_cast<Buffer *>(&current))
        saw_current_source = true;
    }
  assert(found && saw_current_source);
}
'''
    _compile_and_run(bodies, border_getter_control)
    _compile_and_run(bad_border_bodies, border_getter_control,
                     expect_success=False)
