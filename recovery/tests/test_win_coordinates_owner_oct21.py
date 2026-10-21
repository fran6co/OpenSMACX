"""C++98 host controls for the relocated Win coordinate-owner methods.

These compile the six actual moved bodies together with the two actual XY
helpers retained in win.cpp. The harness models only Win fields, RECT/POINT,
and GetCursorPos; it does not claim VC6 instruction or full runtime fidelity.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source.reader import function_line, _region_end


ROOT = Path(__file__).resolve().parents[2]
COORDINATES = ROOT / "src/win_coordinates.cpp"
WIN = ROOT / "src/win.cpp"
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


def _actual_definition(text, qualified, *, after=0, before=None):
    lines = text.splitlines()
    if before is None:
        before = len(lines) + 1
    start = function_line(text, qualified, after_line=after, before_line=before)
    end = _region_end(lines, start - 1, qualified)
    assert end is not None, "native source reader could not bound " + qualified
    return "".join(text.splitlines(keepends=True)[start - 1:end + 1])


def _actual_bodies():
    moved_text = COORDINATES.read_text()
    win_text = WIN.read_text()
    bodies = {
        "screen_rect": _actual_definition(moved_text, "Win::screen_to_client"),
        "get_mouse": _actual_definition(moved_text, "Win::get_mouse_pos"),
        "nonscreen_rect": _actual_definition(moved_text, "Win::nonscreen_to_client"),
        "screen_nonclient": _actual_definition(moved_text, "Win::screen_to_nonclient"),
        "client_rect": _actual_definition(moved_text, "Win::client_to_screen"),
        "nonclient_rect": _actual_definition(moved_text, "Win::nonclient_to_screen"),
        # These remain defined in the original owner TU; the relocated callers
        # must exercise the real source implementation rather than a test stub.
        "client_xy": _actual_definition(win_text, "Win::client_to_screen",
                                        after=0, before=500),
        "screen_xy": _actual_definition(win_text, "Win::screen_to_client",
                                        after=0, before=500),
    }
    for key in ("screen_rect", "get_mouse", "nonscreen_rect", "screen_nonclient",
                "client_rect", "nonclient_rect", "client_xy", "screen_xy"):
        assert bodies[key].startswith("void Win::"), key
    return bodies


HARNESS = r'''#include <stdlib.h>

struct RECT { int left, top, right, bottom; };
struct POINT { int x, y; };

static int cursor_x;
static int cursor_y;
static int cursor_calls;
static int GetCursorPos(POINT *point) {
    ++cursor_calls;
    point->x = cursor_x;
    point->y = cursor_y;
    return 1;
}

struct Win {
    RECT client_rect_;
    RECT outer_rect_;
    int iFlags_;
    Win *win_parent_;
    Win() : iFlags_(0), win_parent_(0) {
        client_rect_.left = client_rect_.top = 0;
        client_rect_.right = client_rect_.bottom = 0;
        outer_rect_.left = outer_rect_.top = 0;
        outer_rect_.right = outer_rect_.bottom = 0;
    }
    void client_to_screen(int *, int *);
    void screen_to_client(int *, int *);
    void screen_to_client(RECT *);
    void get_mouse_pos(int *, int *);
    void nonscreen_to_client(RECT *);
    void screen_to_nonclient(int *, int *);
    void client_to_screen(RECT *);
    void nonclient_to_screen(RECT *);
};

#define CHECK(code, condition) do { if (!(condition)) return (code); } while (0)
static void set_win(Win *w, int cl, int ct, int ol, int ot,
                    int flags, Win *parent) {
    w->client_rect_.left = cl;
    w->client_rect_.top = ct;
    w->outer_rect_.left = ol;
    w->outer_rect_.top = ot;
    w->iFlags_ = flags;
    w->win_parent_ = parent;
}

static void set_rect(RECT *r, int l, int t, int rr, int b) {
    r->left = l; r->top = t; r->right = rr; r->bottom = b;
}

int main() {
    Win grand, parent, child;
    set_win(&grand, 1, 2, 10, 20, 0, 0);
    set_win(&parent, 3, 4, 30, 40, 0x20 | 0x8000, &grand);
    set_win(&child, 5, 6, 50, 60, 0x20 | 0x8000, &parent);

    // The retained XY helpers walk nested parents, and the two 0x8000
    // adjustments cancel their respective parent's outer origins.
    int x = 100, y = 200;
    child.client_to_screen(&x, &y);
    CHECK(1, x == 159 && y == 272);
    child.screen_to_client(&x, &y);
    CHECK(2, x == 100 && y == 200);

    // RECT screen/client conversion delegates to the same real helpers.
    RECT r;
    set_rect(&r, 148, 250, 158, 260);
    child.screen_to_client(&r);
    CHECK(3, r.left == 89 && r.top == 178 && r.right == 99 && r.bottom == 188);

    // Nonscreen conversion performs its final own-outer addition after the
    // optional nested parent walk.
    set_rect(&r, 148, 250, 158, 260);
    child.nonscreen_to_client(&r);
    CHECK(4, r.left == 139 && r.top == 238 && r.right == 149 && r.bottom == 248);

    // Client-to-screen RECT adds the nested screen-space offset.
    set_rect(&r, 1, 2, 11, 12);
    child.client_to_screen(&r);
    CHECK(5, r.left == 60 && r.top == 74 && r.right == 70 && r.bottom == 84);

    // Nonclient-to-screen cancels the child's own outer origin at the end.
    set_rect(&r, 1, 2, 11, 12);
    child.nonclient_to_screen(&r);
    CHECK(6, r.left == 10 && r.top == 14 && r.right == 20 && r.bottom == 24);

    // XY nonclient output performs its own outer addition last.
    x = 148; y = 250;
    child.screen_to_nonclient(&x, &y);
    CHECK(7, x == 139 && y == 238);

    // GetCursorPos is called once, then the child and parent transforms run.
    cursor_x = 148; cursor_y = 250; cursor_calls = 0;
    x = y = -1;
    child.get_mouse_pos(&x, &y);
    CHECK(8, cursor_calls == 1 && x == 89 && y == 178);

    // The original RECT overloads and GetMousePos guard null arguments.
    cursor_calls = 0; y = 77;
    child.screen_to_client(0);
    child.nonscreen_to_client(0);
    child.client_to_screen(0);
    child.nonclient_to_screen(0);
    child.get_mouse_pos(0, &y);
    child.get_mouse_pos(&x, 0);
    CHECK(9, cursor_calls == 0 && y == 77);

    // 0x8000 without 0x20 does not enter a parent walk in nonscreen_to_client.
    Win no_walk, distant;
    set_win(&distant, 100, 200, 300, 400, 0, 0);
    set_win(&no_walk, 3, 4, 30, 40, 0x8000, &distant);
    set_rect(&r, 100, 200, 110, 210);
    no_walk.nonscreen_to_client(&r);
    CHECK(10, r.left == 97 && r.top == 196 && r.right == 107 && r.bottom == 206);

    // Output aliasing preserves sequential stores/subtractions. GetMousePos
    // writes pt.y over pt.x before applying both axis offsets to the same int.
    Win alias;
    set_win(&alias, 3, 7, 5, 11, 0, 0);
    cursor_x = 500; cursor_y = 600; cursor_calls = 0;
    int same = -1;
    alias.get_mouse_pos(&same, &same);
    CHECK(11, cursor_calls == 1 && same == 574);

    // The XY methods and screen_to_nonclient also preserve X-then-Y order
    // when the two output pointers alias.
    same = 100;
    alias.screen_to_client(&same, &same);
    CHECK(12, same == 74);
    same = 100;
    alias.client_to_screen(&same, &same);
    CHECK(13, same == 126);
    same = 100;
    alias.screen_to_nonclient(&same, &same);
    CHECK(14, same == 90);
    return 0;
}
'''


def _program(bodies):
    order = ("client_xy", "screen_xy", "screen_rect", "get_mouse",
             "nonscreen_rect", "screen_nonclient", "client_rect", "nonclient_rect")
    return HARNESS + "\n" + "\n".join(bodies[key] for key in order) + "\n"


def _compile_and_run(tmp_path, compiler, source, label, *, should_pass):
    cpp = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode > 0, "compile-success mutant unexpectedly passed"


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_win_coordinate_owner_bodies_and_compile_success_mutants(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    bodies = _actual_bodies()
    _compile_and_run(tmp_path, compiler, _program(bodies), "win_coordinates_actual",
                     should_pass=True)

    # Historical wrong-receiver mutation compiles but must fail the nested
    # GetMousePos case: this->screen_to_client repeats the child's translation.
    wrong_receiver = dict(bodies)
    old = "win_parent_->screen_to_client(x, y);"
    new = "screen_to_client(x, y);"
    assert wrong_receiver["get_mouse"].count(old) == 1
    wrong_receiver["get_mouse"] = wrong_receiver["get_mouse"].replace(old, new, 1)
    _compile_and_run(tmp_path, compiler, _program(wrong_receiver),
                     "win_coordinates_wrong_receiver", should_pass=False)

    # Moving the 0x8000 arm outside the 0x20 parent gate changes the original
    # no-parent-walk case while remaining valid C++98.
    wrong_gate = dict(bodies)
    old = "if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {"
    new = "if ((iFlags_ & 0x8000) != 0 && win_parent_ != 0) {"
    assert wrong_gate["nonscreen_rect"].count(old) == 1
    wrong_gate["nonscreen_rect"] = wrong_gate["nonscreen_rect"].replace(old, new, 1)
    _compile_and_run(tmp_path, compiler, _program(wrong_gate),
                     "win_coordinates_wrong_parent_gate", should_pass=False)
