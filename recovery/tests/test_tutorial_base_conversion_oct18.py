"""Actual-source host controls for the four TutWin MapWin conversions.

These tests compile the complete current wrapper definitions with small host
stand-ins for the surrounding windows. They check C++ source effects only;
they do not establish VC6 virtual-base ABI or GUI behavior.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/tutwin.cpp"
HEADER = ROOT / "src/mapwin.h"
METHODS = ("do_base", "do_iface", "do_soc", "do_des")
RECEIVER_EXPRESSIONS = {
    "do_base": "TutWinBaseWindow.client_to_screen(&x, &y);",
    "do_iface": "MainInterfaceGlobal.client_to_screen(&x, &y);",
    "do_soc": "TutWinSocWindow.client_to_screen(&x, &y);",
    "do_des": "TutWinDesWindow.client_to_screen(&x, &y);",
}


def _method_body(source: str, name: str) -> str:
    signature = f"int TutWin::{name}(RECT *rect, const char *text, int flag)"
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    index = opening
    state = "code"
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == '"':
                state = "code"
        elif state == "character":
            if char == "\\":
                index += 1
            elif char == "'":
                state = "code"
        elif char == "/" and following == "/":
            state = "line_comment"
            index += 1
        elif char == "/" and following == "*":
            state = "block_comment"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "character"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
        index += 1
    raise AssertionError(f"unterminated actual source method {name}")


def _actual_methods(source: str) -> dict[str, str]:
    return {name: _method_body(source, name) for name in METHODS}


HARNESS = r'''#include <cassert>
#include <cstring>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct GraphicWin { int identity; };
struct Prefix { int prefix; };
struct MapWin : Prefix, public virtual GraphicWin { };
MapWin *MapWinTable[1];

static std::vector<int> events;
struct ScreenWindow {
    int id, dx, dy;
    MapWin *published;
    bool publish;
    void client_to_screen(int *x, int *y) {
        events.push_back(id);
        *x += dx;
        *y += dy;
        if (publish) MapWinTable[0] = published;
    }
};
ScreenWindow TutWinBaseWindow = {1, 1, 2, 0, false};
ScreenWindow MainInterfaceGlobal = {2, 3, 4, 0, false};
ScreenWindow TutWinSocWindow = {3, 5, 6, 0, false};
ScreenWindow TutWinDesWindow = {4, 7, 8, 0, false};

struct TutWin {
    GraphicWin *owner;
    const char *text;
    int x, y, flag, calls, result;
    void *sprite;
    int tail_x, tail_y;
    TutWin() : owner(0), text(0), x(0), y(0), flag(0), calls(0),
               result(73), sprite(0), tail_x(0), tail_y(0) { }
    int tut_win(GraphicWin *value, const char *label, int xpos, int ypos,
                void *picture, int mode, int tail_x_value, int tail_y_value) {
        ++calls;
        owner = value;
        text = label;
        x = xpos;
        y = ypos;
        sprite = picture;
        flag = mode;
        tail_x = tail_x_value;
        tail_y = tail_y_value;
        return result;
    }
    int do_base(RECT *, const char *, int);
    int do_iface(RECT *, const char *, int);
    int do_soc(RECT *, const char *, int);
    int do_des(RECT *, const char *, int);
};

static void check_result(TutWin &tutorial, int receiver, int expected_x,
                         int expected_y, GraphicWin *expected_owner,
                         const char *label, int flag, int expected_return) {
    assert(events.size() == 1);
    assert(events[0] == receiver);
    assert(tutorial.calls == 1);
    assert(tutorial.owner == expected_owner);
    assert(tutorial.text == label);
    assert(tutorial.x == expected_x);
    assert(tutorial.y == expected_y);
    assert(tutorial.flag == flag);
    assert(tutorial.sprite == 0);
    assert(tutorial.tail_x == -1 && tutorial.tail_y == -1);
    assert(expected_return == 73);
}

static int run_checks() {
    TutWin tutorial;
    MapWin first, published;
    const char label[] = "negative-odd-center";
    // The x and y differences are both -5; C++ signed division truncates
    // toward zero, giving centers (8, -7) before each screen translation.
    RECT rect = {10, -5, 5, -10};

    TutWinBaseWindow.publish = true;
    TutWinBaseWindow.published = &published;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_base(&rect, label, 17) == 73);
    check_result(tutorial, 1, 9, -5,
                 static_cast<GraphicWin *>(&published), label, 17, 73);

    MainInterfaceGlobal.publish = true;
    MainInterfaceGlobal.published = &published;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_iface(&rect, label, -23) == 73);
    check_result(tutorial, 2, 11, -3,
                 static_cast<GraphicWin *>(&published), label, -23, 73);

    MainInterfaceGlobal.publish = true;
    MainInterfaceGlobal.published = 0;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_iface(&rect, label, -24) == 73);
    check_result(tutorial, 2, 11, -3, 0, label, -24, 73);

    TutWinSocWindow.publish = true;
    TutWinSocWindow.published = &published;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_soc(&rect, label, 0) == 73);
    check_result(tutorial, 3, 13, -1,
                 static_cast<GraphicWin *>(&published), label, 0, 73);

    TutWinSocWindow.publish = true;
    TutWinSocWindow.published = 0;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_soc(&rect, label, -5) == 73);
    check_result(tutorial, 3, 13, -1, 0, label, -5, 73);

    TutWinDesWindow.publish = true;
    TutWinDesWindow.published = &published;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_des(&rect, label, 4) == 73);
    check_result(tutorial, 4, 15, 1,
                 static_cast<GraphicWin *>(&published), label, 4, 73);

    TutWinDesWindow.publish = true;
    TutWinDesWindow.published = 0;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_des(&rect, label, 6) == 73);
    check_result(tutorial, 4, 15, 1, 0, label, 6, 73);

    // Publish null during conversion: direct virtual-base conversion must
    // still deliver null to tut_win, and must not skip the call.
    TutWinBaseWindow.publish = true;
    TutWinBaseWindow.published = 0;
    MapWinTable[0] = &first;
    events.clear(); tutorial.calls = 0;
    assert(tutorial.do_base(&rect, label, 9) == 73);
    check_result(tutorial, 1, 9, -5, 0, label, 9, 73);
    return 0;
}

int main() { return run_checks(); }
'''


def _fixture_source(methods: dict[str, str]) -> str:
    definitions = "\n\n".join(methods[name] for name in METHODS)
    return HARNESS.replace("int main() { return run_checks(); }",
                           definitions + "\n\nint main() { return run_checks(); }")


def _mutate(methods: dict[str, str], mutation: str) -> dict[str, str]:
    changed = dict(methods)
    if mutation == "wrong_receiver":
        old = RECEIVER_EXPRESSIONS["do_iface"]
        assert old in changed["do_iface"]
        changed["do_iface"] = changed["do_iface"].replace(
            old, RECEIVER_EXPRESSIONS["do_base"], 1
        )
    elif mutation == "conversion_bypass":
        old = "GraphicWin *const window = primary;"
        assert old in changed["do_base"]
        changed["do_base"] = changed["do_base"].replace(
            old, "GraphicWin *const window = reinterpret_cast<GraphicWin *>(primary);", 1
        )
    elif mutation == "read_map_before_callback":
        old = (
            "TutWinBaseWindow.client_to_screen(&x, &y);\n\n"
            "    MapWin *const primary = MapWinTable[0];\n"
            "    GraphicWin *const window = primary;"
        )
        new = (
            "MapWin *const primary = MapWinTable[0];\n"
            "    TutWinBaseWindow.client_to_screen(&x, &y);\n\n"
            "    GraphicWin *const window = primary;"
        )
        assert old in changed["do_base"]
        changed["do_base"] = changed["do_base"].replace(old, new, 1)
    elif mutation == "skip_null_call":
        old = "return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);"
        assert old in changed["do_base"]
        changed["do_base"] = changed["do_base"].replace(
            old, "if (window == 0) return 0;\n    " + old, 1
        )
    else:
        raise AssertionError(mutation)
    return changed


def _compiler_paths():
    return [path for name in ("g++", "clang++")
            if (path := shutil.which(name)) is not None]


@pytest.fixture(scope="module")
def live_methods():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    assert "class MapWin : public virtual GraphicWin" in header
    methods = _actual_methods(source)
    for name, receiver in RECEIVER_EXPRESSIONS.items():
        assert receiver in methods[name]
        assert "GraphicWin *const window = primary;" in methods[name]
        assert "return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);" in methods[name]
    return methods


def _compile(tmp_path: Path, compiler: str, methods: dict[str, str]) -> Path:
    source_path = tmp_path / "fixture.cpp"
    binary_path = tmp_path / "fixture"
    source_path.write_text(_fixture_source(methods), encoding="utf-8")
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source_path),
         "-o", str(binary_path)],
        text=True, capture_output=True, check=False, timeout=30,
    )
    assert result.returncode == 0, result.stderr
    return binary_path


@pytest.mark.parametrize("compiler", _compiler_paths())
def test_all_actual_wrappers_preserve_pointer_and_call_behavior(
    tmp_path, live_methods, compiler
):
    binary = _compile(tmp_path, compiler, live_methods)
    result = subprocess.run([str(binary)], text=True, capture_output=True,
                            check=False, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", (
    "wrong_receiver", "conversion_bypass", "read_map_before_callback",
    "skip_null_call",
))
@pytest.mark.parametrize("compiler", _compiler_paths())
def test_compiling_damage_controls_fail_behavior(
    tmp_path, live_methods, compiler, mutation
):
    binary = _compile(tmp_path, compiler, _mutate(live_methods, mutation))
    result = subprocess.run([str(binary)], text=True, capture_output=True,
                            check=False, timeout=10)
    assert result.returncode != 0, mutation
