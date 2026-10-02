"""Actual Console owner bodies with host doubles; not native layout/runtime proof."""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def body(source, signature):
    assert source.count(signature) == 1
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated owner body")


def actual():
    path = ROOT / "src/console.cpp"
    records = {r.address: r for r in read_file(path)}
    assert records[0x51D890].image_spans == ((0x51D890, 0x51D9CF),)
    assert records[0x51D760].image_spans == ((0x51D760, 0x51D7B5),)
    source = path.read_text()
    return body(source, "void Console::init(int)") + "\n" + body(source, "void Console::set_screen_geometry()")


HARNESS = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define __cdecl
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;

struct RECT { int32_t left, top, right, bottom; };
struct Win {
    static Win *focus;
    static void set_def_focus(Win *value) { focus = value; trace[trace_count++] = 3; }
    static int trace[16];
    static int trace_count;
};
Win *Win::focus = 0;
int Win::trace[16] = {};
int Win::trace_count = 0;

struct Menu;
typedef void (*MenuProc)(int);
struct GraphicWin : Win {
    int rect_calls, coord_calls, width, height, flags;
    Win *parent;
    Menu *menu;
    LPCSTR caption;
    RECT *bounds;
    GraphicWin() : rect_calls(0), coord_calls(0), width(0), height(0),
                   flags(0), parent(0), menu(0), caption(0), bounds(0) {}
    int init(RECT *r, char *title, int style, Win *owner, Menu *attached,
             void *border) {
        assert(border == 0);
        ++rect_calls;
        bounds = r; caption = title; flags = style;
        parent = owner; menu = attached;
        width = r->right - r->left;
        height = r->bottom - r->top;
        trace[trace_count++] = 1;
        return 0;
    }
    int init(int x, int y, int cx, int cy, LPSTR title, int style,
             Win *owner, Menu *attached, void *border) {
        assert(x == 0 && y == 0 && title == 0 && attached == 0 && border == 0);
        ++coord_calls;
        width = cx; height = cy; flags = style; parent = owner;
        trace[trace_count++] = 2;
        return 0;
    }
    int pixel_width() const { return width; }
    int pixel_height() const { return height; }
};
struct Menu : GraphicWin {
    MenuProc proc;
    Menu() : proc(0) {}
    void set_menu_proc(MenuProc callback) { proc = callback; trace[trace_count++] = 5; }
};
struct AlphaMenu : Menu {
    int menu_init_calls;
    AlphaMenu() : menu_init_calls(0) {}
    int menu_init() { ++menu_init_calls; trace[trace_count++] = 4; return 17; }
};
struct MainInterface {
    int height;
    int panel_height() const { return height; }
} MainInterfaceGlobal;

static uint32_t StatusFlags;
static int WinScreenWidth, WinScreenHeight;
static int IfaceScreenWidth, IfaceScreenHeight;
static RECT ConsoleMenuBounds;
static Win *BasePopDefaultField322C;
static int cursor_load_calls;
static int close_calls;
static char Caption[] = "Console menu";
static char *label_get(int index) { assert(index == 0x70); return Caption; }
static void load_cursors() { ++cursor_load_calls; Win::trace[Win::trace_count++] = 6; }

struct ConsolePrefix { int unrelated_storage; };
struct Console : ConsolePrefix, virtual GraphicWin {
    GraphicWin graphic_win_;
    AlphaMenu alpha_menu_;
    int focus_vehicle_;
    uint32_t field_23BEC_, field_23BCC_, field_23BD0_;
    Console() : focus_vehicle_(0), field_23BEC_(99), field_23BCC_(0),
                field_23BD0_(0) {}
    void close() { ++close_calls; trace[trace_count++] = 0; }
    static void __cdecl menu_daemon(int);
    void init(int mode);
    void set_screen_geometry();
};
void __cdecl Console::menu_daemon(int) {}

// ACTUAL_SOURCE_BODIES

int main() {
    Console console;
    assert(static_cast<void *>(static_cast<Win *>(&console)) != static_cast<void *>(&console));
    StatusFlags = 0;
    WinScreenWidth = 800;
    WinScreenHeight = 600;
    MainInterfaceGlobal.height = 100;
    Win::trace_count = 0;
    console.init(0);

    assert(close_calls == 1);
    assert(console.focus_vehicle_ == -1 && console.field_23BEC_ == 0);
    assert(console.field_23BCC_ == 800 && console.field_23BD0_ == 500);
    assert(IfaceScreenWidth == 800 && IfaceScreenHeight == 500);
    assert(ConsoleMenuBounds.left == 0 && ConsoleMenuBounds.top == 0);
    assert(ConsoleMenuBounds.right == 800 && ConsoleMenuBounds.bottom == 500);
    assert((StatusFlags & 1u) != 0);
    assert(console.rect_calls == 1 && console.bounds == &ConsoleMenuBounds);
    assert(console.caption == Caption && console.flags == 0x02400000);
    assert(console.parent == 0 && console.menu == &console.alpha_menu_);
    assert(console.graphic_win_.coord_calls == 1);
    assert(console.graphic_win_.width == 800 && console.graphic_win_.height == 500);
    assert(console.graphic_win_.flags == 0x01000022);
    assert(console.graphic_win_.parent == static_cast<Win *>(&console));
    assert(Win::focus == static_cast<Win *>(&console));
    assert(console.alpha_menu_.menu_init_calls == 1);
    assert(console.alpha_menu_.proc == &Console::menu_daemon);
    assert(cursor_load_calls == 1);
    assert(BasePopDefaultField322C == static_cast<Win *>(&console));
    const int first_count = Win::trace_count;

    // The once bit makes the global screen metrics and RECT persistent across
    // later Console::init calls even if the host metrics change.
    WinScreenWidth = 1024;
    WinScreenHeight = 768;
    MainInterfaceGlobal.height = 180;
    console.init(1);
    assert(console.field_23BCC_ == 800 && console.field_23BD0_ == 500);
    assert(ConsoleMenuBounds.right == 800 && ConsoleMenuBounds.bottom == 500);
    assert(console.alpha_menu_.menu_init_calls == 2);
    assert(cursor_load_calls == 2 && Win::trace_count > first_count);
    console.set_screen_geometry();
    assert(console.field_23BCC_ == 1024 && console.field_23BD0_ == 588);
    assert(IfaceScreenWidth == 1024 && IfaceScreenHeight == 588);
    assert(ConsoleMenuBounds.left == 0 && ConsoleMenuBounds.top == 0);
    assert(ConsoleMenuBounds.right == 1024 && ConsoleMenuBounds.bottom == 588);
    assert(StatusFlags & 1u);
    return 0;
}


'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["none", "ignore_once_bit", "wrong_panel_height", "missing_callback", "raw_default_parent", "geometry_ignores_current_metrics"])
def test_console_init_and_geometry_actual_bodies(tmp_path, compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = actual()
    replacements = {
        "ignore_once_bit": ("if ((StatusFlags & 1u) == 0)", "if (true)"),
        "wrong_panel_height": ("WinScreenHeight - MainInterfaceGlobal.panel_height()", "WinScreenHeight"),
        "missing_callback": ("alpha_menu_.set_menu_proc(&Console::menu_daemon);", "alpha_menu_.set_menu_proc(0);"),
        "raw_default_parent": ("BasePopDefaultField322C = static_cast<Win *>(this);", "BasePopDefaultField322C = static_cast<Win *>(static_cast<void *>(this));"),
        "geometry_ignores_current_metrics": ("const int width = WinScreenWidth;", "const int width = 800;"),
    }
    if mutation != "none":
        old, new = replacements[mutation]
        assert old in source, "live mutation anchor changed"
        source = source.replace(old, new)
    source = HARNESS.replace("// ACTUAL_SOURCE_BODIES", source)
    exe = tmp_path / "console-init-control"
    result = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(exe)],
                            input=source, text=True, capture_output=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=30)
    if mutation == "none":
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, f"{mutation} escaped the actual-body controls"
