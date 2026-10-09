import pathlib
import shutil
import subprocess
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "alphasave.cpp"


def extract_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for cursor in range(brace, len(text)):
        if text[cursor] == "{":
            depth += 1
        elif text[cursor] == "}":
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError("unbalanced method body")


def test_alphasave_init_actual_body_geometry_and_controls(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    text = SOURCE.read_text()
    helper = extract_body(text, "static void alpha_save_store_rect(")
    body = extract_body(text, "void AlphaSave::init()")
    harness = r'''
#include <assert.h>
#include <string.h>
#define __cdecl
struct RECT { int left, top, right, bottom; };
struct Font {};
struct BoxSpriteParams { int tag; };
struct Buffer { Buffer() {} ~Buffer() {} };
static BoxSpriteParams InterfaceBoxStyles[51];
static BoxSpriteParams InterfaceStandardPopupBoxStyle = { 91 };
static int TextColourTable[195];
static int WinScreenWidth, WinScreenHeight;
static int dialog_font_calls, dialog_colour_calls, list_colour_calls;
static int list_reset_calls, draw_callback_calls, dialog_redraw_calls;
static int edit_font_calls, edit_text_calls, edit_focus_calls, edit_redraw_calls;
static int button2_font_calls, button2_redraw_calls, button1_font_calls, button1_redraw_calls;
static int filewin_init_calls, box_calls, fill_calls;
static int last_x, last_y, last_width, last_height;
struct Win { int tag; };
struct ConsoleMock : Win { int visible; int is_visible() const { return visible; } };
static ConsoleMock ConsoleGlobal;
struct MainInterfaceMock {
  int panel_height() const { return 77; }
  Font *secondary_font() { static Font f; return &f; }
  Font *primary_font() { static Font f; return &f; }
} MainInterfaceGlobal;
static RECT WorkingRect = { 10, 20, 300, 220 };
''' + helper + r'''
struct WinParent {};
struct GraphicWin : Win {
  RECT *buffer_working_rect() { return &WorkingRect; }
  void init(int x, int y, int w, int h, char *, int, Win *, void *, void *) {
    ++fill_calls; last_x=x; last_y=y; last_width=w; last_height=h;
  }
  void fill(int) { ++fill_calls; }
  int box_sprite(RECT *r, BoxSpriteParams *p) {
    assert(box_calls < 3);
    rects[box_calls] = *r; styles[box_calls] = p; ++box_calls; return 0;
  }
  static RECT rects[3];
  static BoxSpriteParams *styles[3];
};
RECT GraphicWin::rects[3]; BoxSpriteParams *GraphicWin::styles[3];
struct Dialog {
  unsigned field_20_, field_48_, field_4C_, field_B4_;
  void set_dialog_font(Font *, int, int) { ++dialog_font_calls; }
  void set_dialog_text_color(int, int, int, int) { ++dialog_colour_calls; }
  void on_redraw() { ++dialog_redraw_calls; }
};
static void old_draw(GraphicWin *, int, int, int, int, int) {}
static int old_write(GraphicWin *, int, int, RECT *, char *) { return 17; }
static int old_fill(GraphicWin *, int, int, RECT *) { return 19; }
struct ListBox : Dialog {
  void (*draw_)(GraphicWin *, int, int, int, int, int);
  int (*write_)(GraphicWin *, int, int, RECT *, char *);
  int (*fill_)(GraphicWin *, int, int, RECT *);
  ListBox() : draw_(old_draw), write_(old_write), fill_(old_fill) {}
  void set_report_text_color(int) { ++list_colour_calls; }
  void reset_report_selection() { ++list_reset_calls; }
  void set_draw_callback(void (*draw)(GraphicWin *, int, int, int, int, int)) {
    draw_ = draw;
    ++draw_callback_calls;
  }
};
struct EditBox {
  unsigned field_B1C_, field_B20_, field_B24_, field_B28_, field_B2C_;
  void set_font(Font *, int, int, int) { ++edit_font_calls; }
  void set_text_color(int, int, int, int) { ++edit_text_calls; }
  void on_dialog_focus(int) { ++edit_focus_calls; }
  void redraw() { ++edit_redraw_calls; }
};
struct FlatButton {
  void set_font(Font *, int, int, int) { ++button2_font_calls; }
  void redraw() { ++button2_redraw_calls; }
};
struct FlatButton1 {
  void set_font(Font *, int, int, int) { ++button1_font_calls; }
  void redraw() { ++button1_redraw_calls; }
};
struct FileWinConstruct {
  int field_00_; RECT list_box_bounds_, edit_box_bounds_, button2_bounds_, button1_bounds_; int field_44_;
};
struct FileWin {
  ListBox list_box_; EditBox edit_box_; FlatButton flat_button2_; FlatButton1 flat_button1_;
  FileWinConstruct captured_;
  ListBox &list_box() { return list_box_; }
  EditBox &edit_box() { return edit_box_; }
  FlatButton1 &cancel_button() { return flat_button1_; }
  FlatButton &ok_button() { return flat_button2_; }
  int init(int, GraphicWin *, FileWinConstruct *p) { captured_=*p; ++filewin_init_calls; return 0; }
};
struct AlphaSave : GraphicWin { FileWin file_win_; void init(); };
static void alpha_save_filebox_draw(GraphicWin *, int, int, int, int, int) {}
''' + body + r'''
int main() {
  AlphaSave a;
  ConsoleGlobal.visible = 1; WinScreenWidth = 801; WinScreenHeight = 700;
  memset(TextColourTable, 0, sizeof(TextColourTable));
  int (*preserved_write)(GraphicWin *, int, int, RECT *, char *) = a.file_win_.list_box().write_;
  int (*preserved_fill)(GraphicWin *, int, int, RECT *) = a.file_win_.list_box().fill_;
  a.init();
  assert(a.file_win_.list_box().draw_ == alpha_save_filebox_draw);
  assert(a.file_win_.list_box().write_ == preserved_write);
  assert(a.file_win_.list_box().fill_ == preserved_fill);
  assert(last_x == 219 && last_y == 222 && last_width == 0x18a && last_height == 0x191);
  assert(filewin_init_calls == 1 && box_calls == 2 && fill_calls == 2);
  assert(GraphicWin::styles[0] == &InterfaceBoxStyles[9]);
  assert(GraphicWin::styles[1] == &InterfaceBoxStyles[0]);
  const FileWinConstruct &v = a.file_win_.captured_;
  assert(v.field_00_ == 0 && v.field_44_ == 31);
  assert(v.list_box_bounds_.left == 28 && v.list_box_bounds_.top == 32);
  assert(v.list_box_bounds_.right == 0x136 + 23 && v.list_box_bounds_.bottom == 0x140);
  assert(v.edit_box_bounds_.left == 31 && v.edit_box_bounds_.top == 0x142);
  assert(v.button2_bounds_.left == 33 && v.button1_bounds_.left == 0xA2 + 23);
  assert(GraphicWin::rects[0].left == 33 && GraphicWin::rects[0].right == 245);
  assert(GraphicWin::rects[0].bottom == 230);
  assert(GraphicWin::rects[1].left == 36 && GraphicWin::rects[1].top == 23);
  assert(GraphicWin::rects[1].right == 242 && GraphicWin::rects[1].bottom == 227);
  assert(dialog_font_calls == 1 && dialog_colour_calls == 1 && list_colour_calls == 1);
  assert(list_reset_calls == 1 && draw_callback_calls == 1 && dialog_redraw_calls == 1);
  assert(edit_font_calls == 1 && edit_text_calls == 1 && edit_focus_calls == 1 && edit_redraw_calls == 1);
  assert(button2_font_calls == 1 && button2_redraw_calls == 2);
  assert(button1_font_calls == 1 && button1_redraw_calls == 2);

  box_calls=fill_calls=filewin_init_calls=0;
  ConsoleGlobal.visible = 0; WinScreenWidth = 315; WinScreenHeight = 700;
  a.init();
  assert(last_x == 0 && last_y == 221 && last_width == 0x14e && last_height == 0x189);
  assert(filewin_init_calls == 1 && box_calls == 2 && fill_calls == 2);
  assert(GraphicWin::styles[0] == &InterfaceStandardPopupBoxStyle);
  assert(GraphicWin::styles[1] == &InterfaceBoxStyles[0]);
  const FileWinConstruct &h = a.file_win_.captured_;
  assert((unsigned)h.field_00_ == 0x80020u && h.field_44_ == 8);
  assert(h.list_box_bounds_.left == 0x0e && h.list_box_bounds_.top == 0x2a);
  assert(h.list_box_bounds_.right == 0x13f && h.list_box_bounds_.bottom == 0x14a);
  assert(h.edit_box_bounds_.left == 0x11 && h.edit_box_bounds_.top == 0x14c);
  assert(h.button2_bounds_.left == 0x13 && h.button1_bounds_.left == 0xab);
  assert(GraphicWin::rects[0].left == 10 && GraphicWin::rects[0].top == 20);
  assert(GraphicWin::rects[1].left == 22 && GraphicWin::rects[1].top == 32);
  assert(GraphicWin::rects[1].right == 288 && GraphicWin::rects[1].bottom == 208);
}
'''
    source = tmp_path / "alpha_save_init_control.cpp"
    source.write_text(harness)
    for compiler in compilers:
        binary = tmp_path / ("alpha-save-init-" + pathlib.Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
