"""Host controls compile and execute the current RadioButton input bodies."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.layout_source import parse_members


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/radiobutton.cpp"
HEADER = ROOT / "src/jackal/radiobutton.h"
HARNESS_SOURCE = r'''#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
typedef void (*WinNotificationCallback)(int);
struct StringStructPayload { int field_8_; };
struct StringStructEntry {
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
};
struct ItemList {
    StringStructEntry *current_;
    int current_position_;
    int entry_count_;
    int seek_pos(int position) {
        if (position > entry_count_ - 1) return 0;
        current_ = head_;
        if (position < 0 && -position > entry_count_) return 0;
        current_position_ = position + entry_count_;
        for (int i = 0; i < -position; ++i) current_ = current_->previous;
        return 1;
    }
    void next_entry() {
        if (!head_) return;
        current_ = current_->next;
        ++current_position_;
        if (current_position_ == entry_count_) current_position_ = 0;
    }
    StringStructEntry *head_;
};
struct Parent { int notifications; void on_button_clicked(int) { ++notifications; } };
struct Dialog {
    ItemList item_list_;
    int selected_position_;
    uint32_t field_F0_;
    uint32_t field_20_;
    int field_68_, field_44_, field_64_, field_58_;
    int field_4C_, field_2C_, field_40_;
};
struct Buffer { static int text_line_height() { return 10; } };
static int callback_values[8];
static int callback_count;
static void selection_callback(int ordinal) { callback_values[callback_count++] = ordinal; }
static int double_click_count;
static void double_click_callback(int) { ++double_click_count; }
static int drawn[16];
static int draw_count;
static int updates;
static Parent *WinCallbackWindow;
struct RadioButton : Dialog {
    WinNotificationCallback selection_callback_;
    WinNotificationCallback double_click_callback_;
    Parent *win_parent_;
    void draw_item(int position, int state) { drawn[draw_count++] = position * 2 + state; }
    void soft_update() { ++updates; }
    void redraw() { ++updates; }
    void on_left_down(int x, int y);
    void on_mouse_move(int x, int y);
};
/* ACTUAL_LEFT_DOWN */
/* ACTUAL_MOUSE_MOVE */
static void reset(ItemList &items, RadioButton &button) {
    items.current_ = 0;
    items.head_ = 0;
    items.current_position_ = 0;
    items.entry_count_ = 3;
    button.field_68_ = 20;
    button.field_44_ = 4;
    button.field_64_ = 16;
    button.field_58_ = 2;
    button.field_4C_ = 2;
    button.field_2C_ = 3;
    button.field_40_ = 3;
    button.selected_position_ = 0;
    button.field_F0_ = static_cast<uint32_t>(-1);
    button.field_20_ = 0;
    button.selection_callback_ = selection_callback;
    button.double_click_callback_ = double_click_callback;
    button.win_parent_ = 0;
}
int main(int argc, char **argv) {
    ItemList items = {};
    RadioButton button = {};
    reset(items, button);
    button.item_list_ = items;
    StringStructPayload payloads[3] = {{1}, {1}, {1}};
    StringStructEntry entries[3] = {};
    for (int i = 0; i < 3; ++i) {
        entries[i].payload = &payloads[i];
        entries[i].next = &entries[(i + 1) % 3];
        entries[i].previous = &entries[(i + 2) % 3];
    }
    button.item_list_.head_ = &entries[0];
    button.item_list_.current_ = &entries[2];
    button.on_left_down(20, 22);
    assert(button.selected_position_ == 2);
    assert(callback_count == 1 && callback_values[0] == 2);
    assert(double_click_count == 0);
    assert(draw_count == 2 && drawn[0] == 0 && drawn[1] == 5);
    if (argc > 1 && std::strcmp(argv[1], "--hover") == 0) {
        button.selected_position_ = 0;
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == 2 && updates == 2);
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == 2 && updates == 2);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--null-current") == 0) {
        button.item_list_.head_ = 0;
        button.item_list_.entry_count_ = 0;
        button.item_list_.current_ = &entries[0];
        button.item_list_.current_position_ = 2;
        button.selected_position_ = 0;
        button.on_mouse_move(0, 1);
        assert(button.item_list_.current_ == 0);
        assert(button.item_list_.current_position_ == 2);
        assert(button.field_F0_ == 2);
        return 0;
    }
    if (argc > 1 && (std::strcmp(argv[1], "--head-fault-left") == 0 ||
                     std::strcmp(argv[1], "--head-fault-move") == 0)) {
        button.item_list_.head_ = 0;
        button.item_list_.entry_count_ = -1;
        button.item_list_.current_ = &entries[0];
        button.item_list_.current_position_ = 0;
        if (std::strcmp(argv[1], "--head-fault-left") == 0)
            button.on_left_down(0, 1);
        else
            button.on_mouse_move(0, 1);
        return 4;
    }
    if (argc > 1 && (std::strcmp(argv[1], "--payload-fault-left") == 0 ||
                     std::strcmp(argv[1], "--payload-fault-move") == 0)) {
        button.item_list_.current_ = &entries[0];
        entries[0].payload = 0;
        if (std::strcmp(argv[1], "--payload-fault-left") == 0)
            button.on_left_down(0, 1);
        else
            button.on_mouse_move(0, 1);
        return 4;
    }
    std::puts("RadioButton input actual-source controls passed");
}
'''


def _mask(text):
    return re.sub(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'",
                  lambda match: re.sub(r"[^\n]", " ", match.group()), text,
                  flags=re.S)


def _body(text, name):
    masked = _mask(text)
    signature = "void RadioButton::" + name
    matches = list(re.finditer(re.escape(signature) + r"\s*\(", masked))
    found = []
    for match in matches:
        opening = masked.find("{", masked.find(")", match.end()) + 1)
        if opening < 0:
            continue
        depth = 0
        for pos in range(opening, len(masked)):
            depth += (masked[pos] == "{") - (masked[pos] == "}")
            if depth == 0:
                found.append(text[match.start():pos + 1])
                break
    assert len(found) == 1
    assert "BODY GOES HERE" not in found[0]
    return found[0]


def _actual(text=None, *, validate=True):
    text = SOURCE.read_text() if text is None else text
    left = _body(text, "on_left_down")
    move = _body(text, "on_mouse_move")
    if validate:
        assert "Dialog::field_68_ + Dialog::field_44_" in left
        assert "Dialog::field_68_ + Dialog::field_44_" in move
        assert "Dialog::field_64_ + Dialog::field_44_" in left
        assert "Dialog::field_64_ + Dialog::field_44_" in move
        assert "Dialog::field_58_" in left and "Dialog::field_58_" in move
        assert "current_ != 0" in left and "current_ != 0" in move
        assert "head_ == 0" in left and "head_ == 0" in move
        assert "payload->field_8_" in left and "payload->field_8_" in move
        assert "selection_callback_(position)" in left
        assert "double_click_callback_(position)" not in left
        assert "seek_pos(-1)" in left
        assert "entry_count_ <= 0" not in left and "entry_count_ <= 0" not in move
        ordered = ("draw_item(previous, 0)", "Dialog::selected_position_ = position",
                   "selection_callback_(position)", "draw_item(position, 1)",
                   "Dialog::field_F0_ =", "soft_update()")
        assert [left.index(token) for token in ordered] == sorted(
            left.index(token) for token in ordered)
    return left, move


def _compile(tmp_path, compiler, source_text=None, *, validate=True):
    left, move = _actual(source_text, validate=validate)
    fixture = HARNESS_SOURCE
    assert fixture.count("/* ACTUAL_LEFT_DOWN */") == 1
    assert fixture.count("/* ACTUAL_MOUSE_MOVE */") == 1
    fixture = fixture.replace("/* ACTUAL_LEFT_DOWN */", left)
    fixture = fixture.replace("/* ACTUAL_MOUSE_MOVE */", move)
    cpp = tmp_path / "radiobutton_input_actual.cpp"
    exe = tmp_path / "radiobutton_input_actual"
    cpp.write_text(fixture)
    built = subprocess.run(
        [compiler, "-std=c++11", "-fwrapv", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
        text=True, capture_output=True,
    )
    return built, exe


def _execute(exe, *, mode=None, fault=False):
    args = [str(exe), mode or "--fault"] if mode is not None or fault else [str(exe)]
    result = subprocess.run(args, text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ,
                                 "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    if fault:
        _assert_expected_fault(result, mode)
    else:
        assert result.returncode == 0, result.stdout + result.stderr
        if mode is None:
            assert "RadioButton input actual-source controls passed" in result.stdout


def _assert_expected_fault(result, mode):
    assert result.returncode not in (0, 4), mode + " returned safely"
    assert ("runtime error: member access within null pointer" in result.stderr
            or "AddressSanitizer: SEGV" in result.stderr), (
                mode + " failed without evidence of the expected null-member read: "
                + result.stdout + result.stderr)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_radiobutton_input_bodies(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, exe = _compile(tmp_path, compiler)
    assert built.returncode == 0, built.stdout + built.stderr
    _execute(exe)
    _execute(exe, mode="--hover")
    _execute(exe, mode="--null-current")
    for mode in ("--head-fault-left", "--head-fault-move",
                 "--payload-fault-left", "--payload-fault-move"):
        _execute(exe, mode=mode, fault=True)


def test_mutated_geometry_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace("Dialog::field_64_ + Dialog::field_44_",
                                     "Dialog::field_4C_ + Dialog::field_2C_")
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode != 0


def test_mutated_minimum_height_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace(
        "const int minimum = static_cast<int>(Dialog::field_68_ + Dialog::field_44_);",
        "const int minimum = 0;")
    assert text != SOURCE.read_text()
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode != 0


def test_mutated_row_count_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace("Dialog::field_58_", "Dialog::field_40_")
    assert text != SOURCE.read_text()
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode != 0


def test_mutated_callback_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace("selection_callback_(position)",
                                     "double_click_callback_(position)")
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode != 0


def test_mutated_null_payload_guard_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace(
        "lines = static_cast<int>(payload->field_8_);",
        "if (payload != 0) lines = static_cast<int>(payload->field_8_);")
    assert text != SOURCE.read_text()
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe), "--payload-fault-left"], text=True, capture_output=True, timeout=20,
                            env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
                                 "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode == 4, result.stdout + result.stderr


def test_mutated_positive_count_requirement_is_caught(tmp_path):
    """A positive-count guard must not erase the original bad-state fault."""
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text().replace(
        "void RadioButton::on_left_down(int x, int y) {\n",
        "void RadioButton::on_left_down(int x, int y) {\n"
        "    if (Dialog::item_list_.entry_count_ <= 0) return;\n", 1)
    assert text != SOURCE.read_text()
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe), "--head-fault-left"], text=True, capture_output=True,
                            timeout=20, env={**__import__("os").environ,
                                             "ASAN_OPTIONS": "detect_leaks=0",
                                             "UBSAN_OPTIONS": "halt_on_error=1"})
    with pytest.raises(AssertionError, match="returned safely"):
        _assert_expected_fault(result, "positive-count guard mutant")


def test_callback_member_has_typed_ordinal_identity():
    header = HEADER.read_text()
    assert "RECOVERY_MEMBER: v1 RadioButton selection_callback_ 0xC 0x0060DFAD" in header
    assert "WinNotificationCallback selection_callback_;" in header
    members = parse_members(header, "src/jackal/radiobutton.h",
                            "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
                            address_space="pe:va")
    member = next(row for row in members if row.member == "selection_callback_")
    assert (member.class_name, member.offset, member.evidence.address) == (
        "RadioButton", 0xC, 0x0060DFAD)
    body = SOURCE.read_text()
    assert body.count("selection_callback_ = 0;") == 2
    assert "double_click_callback_ = 0;" in body
