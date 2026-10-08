"""Strict actual-source controls for Console::editor_set_brush."""
from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _block(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[opening : pos + 1]
    raise AssertionError("unterminated C++ block")


def _declaration(pattern: str, source: str, description: str) -> str:
    match = re.search(pattern, source, re.MULTILINE)
    assert match, f"could not find source declaration: {description}"
    return match.group(0)


def _installed() -> tuple[str, str, str, str, str]:
    console = (ROOT / "src/console.cpp").read_text()
    marker = "void Console::editor_set_brush()"
    body = _block(console, console.index(marker))
    assert "EditorBrushButtonGroup" not in body
    assert "EditorBrushButtonGroupSelection" not in body
    assert "MainInterfaceGlobal.button_group(3)" in body
    assert ".field_84_" in body and ".set(" in body
    assert "x_pop_value_flags" in body

    bg = (ROOT / "src/buttongroup.h").read_text()
    field = _declaration(r"uint32_t field_84_;", bg, "ButtonGroup::field_84_")
    set_decl = _declaration(r"int set\(int button_id, int notify\);", bg, "ButtonGroup::set")

    main = (ROOT / "src/maininterface.h").read_text()
    accessor_start = main.index("ButtonGroup &button_group(int index)")
    accessor = "ButtonGroup &button_group(int index) " + _block(main, accessor_start)
    group_field = _declaration(r"ButtonGroup button_group_\[4\];", main, "MainInterface::button_group_")
    return field, set_decl, accessor, group_field, f"void Console::editor_set_brush() {body}"


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
using std::uint32_t;
typedef int (*NullCallback)(void);

static std::vector<int> events;
static int MapEditorBrushId = 4;
static int WinScreenWidth = 1024;
static int PopupModalState = 0;
static int popup_answer = -1;
static int popup_value_seen = -999;
static int popup_flags_seen = -999;
static int popup_calls = 0;
static int redraw_brush_mutation = -1;
static int draw_pane_calls = 0;
static int set_calls = 0;
static int set_id = -999;
static int set_notify = -999;
static bool observe_button_group_access = false;
static const char StrBrush[] = "STR_BRUSH";

class ButtonGroup {
public:
    FIELD_DECL
    SET_DECL
};
static_assert(std::is_same<decltype(std::declval<ButtonGroup>().field_84_), uint32_t>::value,
              "selection field must retain the declared ButtonGroup type");
static_assert(std::is_same<decltype(&ButtonGroup::set), int (ButtonGroup::*)(int, int)>::value,
              "ButtonGroup::set ABI/signature drift");

int ButtonGroup::set(int button_id, int notify) {
    ++set_calls; set_id = button_id; set_notify = notify;
    field_84_ = static_cast<uint32_t>(button_id);
    events.push_back(6);
    return 0;
}

class MainInterface {
private:
    GROUP_FIELD
public:
    ACCESSOR
    void draw_edit_pane() {
        assert(PopupModalState == 0);
        ++draw_pane_calls;
        events.push_back(5);
    }
};
MainInterface MainInterfaceGlobal;

class InfoWin {
public:
    int phase = -1;
    void set_redraw_phase(int value) {
        assert(PopupModalState == 0 && value == 7);
        phase = value;
        events.push_back(3);
    }
    void on_redraw() {
        assert(PopupModalState == 0 && phase == 7);
        events.push_back(4);
        if (redraw_brush_mutation >= 0) MapEditorBrushId = redraw_brush_mutation;
    }
};
InfoWin ConsoleInfoWin;

int x_pop_value_flags(const char *label, int value, int flags, int (*)()) {
    assert(PopupModalState == 6);
    assert(label == StrBrush);
    ++popup_calls; popup_value_seen = value; popup_flags_seen = flags;
    events.push_back(2);
    return popup_answer;
}

class Console { public: void editor_set_brush(); };
void Console::editor_set_brush() BODY

static void reset(int answer, int current_brush, uint32_t selection, int width) {
    events.clear();
    MapEditorBrushId = current_brush;
    popup_answer = answer;
    popup_value_seen = -999; popup_flags_seen = -999; popup_calls = 0;
    redraw_brush_mutation = -1;
    draw_pane_calls = set_calls = 0;
    set_id = set_notify = -999;
    WinScreenWidth = width;
    PopupModalState = 0;
    ConsoleInfoWin.phase = -1;
    for (int i = 0; i != 4; ++i) {
        MainInterfaceGlobal.button_group(i).field_84_ = 700u + static_cast<uint32_t>(i);
    }
    MainInterfaceGlobal.button_group(3).field_84_ = selection;
}

static void check_common(int original_popup_value) {
    assert(events.size() == 4 || events.size() == 5);
    assert(events[0] == 2 && events[1] == 3 && events[2] == 4 && events[3] == 5);
    assert(PopupModalState == 0);
    assert(popup_calls == 1 && popup_value_seen == original_popup_value && popup_flags_seen == 0x40);
    assert(ConsoleInfoWin.phase == 7 && draw_pane_calls == 1);
}

static void test_negative_answer_preserves_brush_and_different_selection() {
    reset(-7, 2, 0, 1024);
    Console c; observe_button_group_access = true; c.editor_set_brush(); observe_button_group_access = false;
    check_common(2);
    assert(MapEditorBrushId == 2);
    assert(set_calls == 1 && set_id == 0x40E && set_notify == 1);
    assert(MainInterfaceGlobal.button_group(3).field_84_ == 0x40E);
    for (int i = 0; i != 3; ++i) assert(MainInterfaceGlobal.button_group(i).field_84_ == 700u + uint32_t(i));
    assert(events.size() == 5 && events[4] == 6);
}

static void test_positive_answer_and_equal_selection_are_noop() {
    reset(6, 1, 0x412, 1024);
    Console c; observe_button_group_access = true; c.editor_set_brush(); observe_button_group_access = false;
    check_common(1);
    assert(MapEditorBrushId == 6);
    assert(set_calls == 0 && MainInterfaceGlobal.button_group(3).field_84_ == 0x412);
    assert(events.size() == 4);
    assert(MainInterfaceGlobal.button_group(0).field_84_ == 700u);
}

static void test_800_pixel_width_skips_group_even_when_different() {
    reset(3, 1, 0, 0x320);
    Console c; observe_button_group_access = true; c.editor_set_brush(); observe_button_group_access = false;
    check_common(1);
    assert(MapEditorBrushId == 3);
    assert(set_calls == 0 && MainInterfaceGlobal.button_group(3).field_84_ == 0);
    assert(events.size() == 4);
}

static void test_group_id_uses_brush_after_redraw_callback() {
    reset(3, 1, 0, 1024);
    redraw_brush_mutation = 9;
    Console c; observe_button_group_access = true; c.editor_set_brush(); observe_button_group_access = false;
    check_common(1);
    assert(MapEditorBrushId == 9);
    assert(set_calls == 1 && set_id == 9 + 0x40C && set_notify == 1);
    assert(MainInterfaceGlobal.button_group(3).field_84_ == 9 + 0x40C);
}

int main() {
    test_negative_answer_preserves_brush_and_different_selection();
    test_positive_answer_and_equal_selection_are_noop();
    test_800_pixel_width_skips_group_even_when_different();
    test_group_id_uses_brush_after_redraw_callback();
}
'''


def test_actual_source_body_gcc_clang_asan_ubsan(tmp_path: Path) -> None:
    field, set_decl, accessor, group_field, method = _installed()
    harness = (HARNESS.replace("FIELD_DECL", field)
               .replace("SET_DECL", set_decl)
               .replace("ACCESSOR", accessor.replace(
                   "{ return button_group_[index]; }",
                   "{ if (observe_button_group_access) assert(index == 3); return button_group_[index]; }"))
               .replace("GROUP_FIELD", group_field)
               .replace("BODY", method[len("void Console::editor_set_brush() "):]))
    source = tmp_path / "editor_set_brush.cpp"
    source.write_text(harness)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), f"required host compilers unavailable: {compilers}"
    for compiler in compilers:
        exe = tmp_path / (Path(compiler).name + "-controls")
        subprocess.run(
            [compiler, "-std=c++11", "-O1", "-g", "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(source), "-o", str(exe)],
            check=True,
        )
        subprocess.run([str(exe)], check=True,
                      env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
