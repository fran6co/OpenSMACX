"""Exercise the actual SpriteBox list/input bodies with host boundary doubles.

This checks source behavior, not VC6 layout, original-byte matching, or game
runtime acceptance. The body text is compiled directly from spritebox.cpp.
"""

from pathlib import Path
import shutil
import subprocess
import os

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature + " {")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated body: {signature}")


def test_right_callback_members_keep_the_pinned_slots_and_lifecycle_resets():
    header = (ROOT / "src/spritebox.h").read_text()
    source = (ROOT / "src/spritebox.cpp").read_text()
    for declaration in (
        "ClickCallback right_click_callback_;  // 0x74",
        "ClickCallback right_down_callback_;  // 0x78",
        "ClickCallback right_up_callback_;  // 0x7C",
        "ClickCallback right_double_click_callback_;  // 0x84",
    ):
        assert declaration in header
    constructor = _body(source, "SpriteBox::SpriteBox()")
    close = _body(source, "void SpriteBox::close()")
    for member in (
        "right_click_callback_", "right_down_callback_", "right_up_callback_",
        "right_double_click_callback_",
    ):
        assert f"{member} = 0;" in constructor
        assert f"{member} = 0;" in close


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_spritebox_input_and_list_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/spritebox.cpp").read_text()
    signatures = (
        "uint32_t SpriteBox::id_to_pos(int id)",
        "void SpriteBox::on_right_down(int x_coord, int y_coord)",
        "void SpriteBox::on_right_click(int x_coord, int y_coord)",
        "void SpriteBox::on_right_up(int x_coord, int y_coord)",
        "void SpriteBox::on_right_double_click(int x_coord, int y_coord)",
    )
    bodies = "\n".join(_body(source, signature) for signature in signatures)
    harness = r'''
#include <cassert>
#include <cstdint>

enum { WM_KEYDOWN = 0x100, VK_RETURN = 0x0d };
static int HandleMain = 77;
static int events[8], event_count, posted;
static int expected_x, expected_y, output_y, check_result;
static int callback_result, callback_argument, callback_calls;

static void event(int value) { events[event_count++] = value; }

struct StringStructEntry {
    int unused;
    int id;
    void *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
};
struct SpriteList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
};
struct Spot {
    int check(int x, int y, int *out, int flags) {
        assert(x == expected_x && y == expected_y && flags == 0);
        event(1);
        *out = output_y;
        return check_result;
    }
};
static int callback(int value) {
    event(2);
    ++callback_calls;
    callback_argument = value;
    return callback_result;
}
static int PostMessageA(int window, int message, int key, int extra) {
    assert(window == HandleMain && message == WM_KEYDOWN);
    assert(key == VK_RETURN && extra == 0);
    event(3);
    ++posted;
    return 1;
}

struct SpriteBox {
    typedef int (*ClickCallback)(int);
    SpriteList list_;
    Spot spots_;
    ClickCallback right_click_callback_;
    ClickCallback right_down_callback_;
    ClickCallback right_up_callback_;
    ClickCallback right_double_click_callback_;
    uint32_t id_to_pos(int id);
    void on_right_down(int x_coord, int y_coord);
    void on_right_click(int x_coord, int y_coord);
    void on_right_up(int x_coord, int y_coord);
    void on_right_double_click(int x_coord, int y_coord);
};
''' + bodies + r'''
static void reset_input(int result, int response) {
    event_count = posted = callback_calls = 0;
    callback_argument = -999;
    check_result = result;
    callback_result = response;
    expected_x = 19;
    expected_y = 27;
    output_y = 41;
}
static void assert_no_dispatch() {
    assert(callback_calls == 0 && posted == 0);
    assert(event_count == 1 && events[0] == 1);
}
static void assert_dispatch(int response) {
    assert(callback_calls == 1 && callback_argument == output_y);
    assert(posted == (response != 0));
    assert(event_count == (response ? 3 : 2));
    assert(events[0] == 1 && events[1] == 2);
    if (response) assert(events[2] == 3);
}
int main() {
    SpriteBox box = {};
    box.right_click_callback_ = callback;
    box.right_down_callback_ = callback;
    box.right_up_callback_ = callback;
    box.right_double_click_callback_ = callback;
    typedef void (SpriteBox::*Handler)(int, int);
    Handler handlers[] = {&SpriteBox::on_right_down, &SpriteBox::on_right_click,
                          &SpriteBox::on_right_up, &SpriteBox::on_right_double_click};
    for (Handler handler : handlers) {
        reset_input(-1, 1);
        (box.*handler)(expected_x, expected_y);
        assert_no_dispatch();
        reset_input(3, 0);
        (box.*handler)(expected_x, expected_y);
        assert_dispatch(0);
        reset_input(3, 1);
        (box.*handler)(expected_x, expected_y);
        assert_dispatch(1);
    }
    box.right_click_callback_ = 0;
    reset_input(3, 1);
    box.on_right_click(expected_x, expected_y);
    assert_no_dispatch();

    StringStructEntry a = {0, 10, 0, 0, 0};
    StringStructEntry b = {0, 20, 0, 0, 0};
    StringStructEntry c = {0, 30, 0, 0, 0};
    a.next = &b; b.next = &c; c.next = &a;
    a.previous = &c; b.previous = &a; c.previous = &b;
    box.list_ = {0, &c, 0, 8};
    assert(box.id_to_pos(20) == 8); // null head leaves all list state untouched
    assert(box.list_.current_ == &c && box.list_.current_position_ == 8);
    box.list_ = {&a, &c, 0, 9};
    assert(box.id_to_pos(20) == 0); // headed but nonpositive count resets both
    assert(box.list_.current_ == &a && box.list_.current_position_ == 0);
    box.list_ = {&a, &c, 3, 9};
    assert(box.id_to_pos(10) == 0);
    assert(box.list_.current_ == &a && box.list_.current_position_ == 0);
    box.list_ = {&a, &c, 3, 9};
    assert(box.id_to_pos(30) == 2);
    assert(box.list_.current_ == &c && box.list_.current_position_ == 2);
    box.list_ = {&a, &c, 3, 9};
    assert(box.id_to_pos(99) == 3); // missing ID traverses count and cursor wraps
    assert(box.list_.current_ == &a && box.list_.current_position_ == 3);
    box.list_ = {&a, &c, -2, 9};
    assert(box.id_to_pos(20) == 0); // negative count follows same original branch
    assert(box.list_.current_ == &a && box.list_.current_position_ == 0);
}
'''
    path = tmp_path / "spritebox_members.cpp"
    binary = tmp_path / "spritebox_members"
    path.write_text(harness)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(path), "-o", str(binary)],
        check=True,
    )
    run_environment = os.environ.copy()
    run_environment["ASAN_OPTIONS"] = "detect_leaks=0"
    subprocess.run([str(binary)], check=True, env=run_environment)

    mutants = (
        bodies.replace("if (list_.head_ == 0)", "if (list_.head_ != 0)", 1),
        bodies.replace("handler(y_coord) != 0", "handler(y_coord) == 0", 1),
        bodies.replace("handler(y_coord)", "handler(x_coord)", 1),
    )
    for index, mutant in enumerate(mutants):
        mutant_path = tmp_path / f"spritebox_mutant_{index}.cpp"
        mutant_binary = tmp_path / f"spritebox_mutant_{index}"
        mutant_path.write_text(harness.replace(bodies, mutant, 1))
        subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
             str(mutant_path), "-o", str(mutant_binary)],
            check=True,
        )
        result = subprocess.run([str(mutant_binary)], env=run_environment,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        assert result.returncode != 0, f"mutant {index} survived the controls"
