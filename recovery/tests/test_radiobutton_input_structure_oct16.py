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
typedef ItemList StringStruct;
struct Parent { int notifications; void on_button_clicked(int) { ++notifications; } };
struct Dialog {
    ItemList item_list_;
    int selected_position_;
    uint32_t field_F0_;
    uint32_t field_20_;
    int field_68_, field_44_, field_64_, row_count_;
    int field_4C_, field_2C_, field_40_;
    int get_selected_id() { return selected_position_ + 100; }
};
struct Buffer { static int text_line_height() { return 10; } };
static int callback_values[8];
static int callback_count;
static int double_click_values[8];
static int double_click_count;
static int event_log[32];
static int event_count;
static bool mutate_selection;
struct RadioButton;
static RadioButton *active_button;
static void selection_callback(int ordinal);
static void double_click_callback(int value);
static int drawn[16];
static int draw_count;
static int updates;
static Parent *WinCallbackWindow;
struct RadioButton : Dialog {
    WinNotificationCallback selection_callback_;
    WinNotificationCallback double_click_callback_;
    Parent *win_parent_;
    void draw_item(int position, int state) {
        drawn[draw_count++] = position * 2 + state;
        event_log[event_count++] = 1000 + position * 2 + state;
    }
    void soft_update() { ++updates; event_log[event_count++] = 3000; }
    void redraw() { ++updates; }
    void on_left_down(int x, int y);
    void on_mouse_move(int x, int y);
    int on_key_down(int key);
    void on_left_double_click(int x, int y);
};
static void selection_callback(int ordinal) {
    callback_values[callback_count++] = ordinal;
    event_log[event_count++] = 2000;
    if (mutate_selection) active_button->selected_position_ = 1;
}
static void double_click_callback(int value) {
    double_click_values[double_click_count++] = value;
    event_log[event_count++] = 2100;
}
/* ACTUAL_LEFT_DOWN */
/* ACTUAL_MOUSE_MOVE */
/* ACTUAL_KEY_DOWN */
/* ACTUAL_DOUBLE_CLICK */
static void reset(ItemList &items, RadioButton &button) {
    items.current_ = 0;
    items.head_ = 0;
    items.current_position_ = 0;
    items.entry_count_ = 3;
    button.field_68_ = 20;
    button.field_44_ = 4;
    button.field_64_ = 16;
    button.row_count_ = 2;
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

    if (argc > 1 && std::strcmp(argv[1], "--key") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.selected_position_ = 0;
        active_button = &button;
        assert(button.on_key_down(0x25) == 1);
        assert(button.selected_position_ == 2);
        assert(callback_count == 1 && callback_values[0] == 2);
        assert(double_click_count == 0);
        assert(draw_count == 2 && drawn[0] == 0 && drawn[1] == 5);
        assert(event_count == 4 && event_log[0] == 1000 &&
               event_log[1] == 2000 && event_log[2] == 1005 &&
               event_log[3] == 3000);
        assert(WinCallbackWindow == &parent);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--key-mutation") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.selected_position_ = 0;
        active_button = &button;
        mutate_selection = true;
        assert(button.on_key_down(0x25) == 1);
        assert(callback_count == 1 && callback_values[0] == 2);
        assert(double_click_count == 0);
        assert(button.selected_position_ == 1);
        assert(draw_count == 2 && drawn[0] == 0 && drawn[1] == 3);
        assert(event_count == 4 && event_log[0] == 1000 &&
               event_log[1] == 2000 && event_log[2] == 1003 &&
               event_log[3] == 3000);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--key-null") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.selected_position_ = 0;
        button.selection_callback_ = 0;
        assert(button.on_key_down(0x25) == 1);
        assert(button.selected_position_ == 2);
        assert(callback_count == 0 && double_click_count == 0);
        assert(draw_count == 2 && drawn[0] == 0 && drawn[1] == 5);
        assert(event_count == 3 && event_log[0] == 1000 &&
               event_log[1] == 1005 && event_log[2] == 3000);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--key-default") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.selected_position_ = 1;
        assert(button.on_key_down(0x41) == 0);
        assert(button.selected_position_ == 1);
        assert(callback_count == 0 && double_click_count == 0);
        assert(draw_count == 0 && updates == 0 && event_count == 0);
        assert(WinCallbackWindow == &parent);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--double-click") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.selected_position_ = 2;
        active_button = &button;
        button.on_left_double_click(20, 22);
        assert(callback_count == 0);
        assert(double_click_count == 1 && double_click_values[0] == 102);
        assert(event_count == 1 && event_log[0] == 2100);
        assert(WinCallbackWindow == &parent);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--double-null") == 0) {
        Parent parent = {};
        button.win_parent_ = &parent;
        button.double_click_callback_ = 0;
        button.on_left_double_click(20, 22);
        assert(callback_count == 0 && double_click_count == 0);
        assert(event_count == 0 && WinCallbackWindow == &parent);
        return 0;
    }

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
        assert(button.field_F0_ == 2 && updates == 3);
        return 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "--hover-matrix") == 0) {
        updates = 0;
        button.selected_position_ = 2;
        button.field_F0_ = static_cast<uint32_t>(-1);
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == static_cast<uint32_t>(-1) && updates == 0);

        button.field_F0_ = 2;
        button.on_mouse_move(20, 22);
        // Original60E467..60E472 clears selected-item hover, then compares -1.
        assert(button.field_F0_ == static_cast<uint32_t>(-1) && updates == 1);
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == static_cast<uint32_t>(-1) && updates == 1);

        button.selected_position_ = 0;
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == 2 && updates == 2);
        button.on_mouse_move(20, 22);
        assert(button.field_F0_ == 2 && updates == 3);
        // Original60E3A3 jumps directly to the return when traversal runs out.
        button.field_F0_ = 77;
        button.on_mouse_move(20, 1000);
        assert(button.field_F0_ == 77 && updates == 3);
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


def _body(text, name, return_type="void"):
    masked = _mask(text)
    signature = return_type + " RadioButton::" + name
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
    key = _body(text, "on_key_down", "int")
    double_click = _body(text, "on_left_double_click")
    if validate:
        assert "Dialog::field_68_ + Dialog::field_44_" in left
        assert "Dialog::field_68_ + Dialog::field_44_" in move
        assert "Dialog::field_64_ + Dialog::field_44_" in left
        assert "Dialog::field_64_ + Dialog::field_44_" in move
        assert "Dialog::row_count_" in left and "Dialog::row_count_" in move
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
        assert "selection_callback_(static_cast<int>(selected_position_))" in key
        assert "double_click_callback_" not in key
        key_ordered = ("draw_item(previous, 0)", "selection_callback_(",
                       "draw_item(static_cast<int>(selected_position_), 1)",
                       "soft_update()")
        assert [key.index(token) for token in key_ordered] == sorted(
            key.index(token) for token in key_ordered)
        assert "double_click_callback_(Dialog::get_selected_id())" in double_click
        assert "selection_callback_" not in double_click
    return left, move, key, double_click


def _compile(tmp_path, compiler, source_text=None, *, validate=True):
    left, move, key, double_click = _actual(source_text, validate=validate)
    # The host stub uses unnamed parameters because the product body does not
    # consume coordinates; only the extracted source body is under test here.
    double_click = double_click.replace(
        "RadioButton::on_left_double_click(int x_coord, int y_coord)",
        "RadioButton::on_left_double_click(int, int)", 1)
    fixture = HARNESS_SOURCE
    for marker in ("/* ACTUAL_LEFT_DOWN */", "/* ACTUAL_MOUSE_MOVE */",
                   "/* ACTUAL_KEY_DOWN */", "/* ACTUAL_DOUBLE_CLICK */"):
        assert fixture.count(marker) == 1
    fixture = fixture.replace("/* ACTUAL_LEFT_DOWN */", left)
    fixture = fixture.replace("/* ACTUAL_MOUSE_MOVE */", move)
    fixture = fixture.replace("/* ACTUAL_KEY_DOWN */", key)
    fixture = fixture.replace("/* ACTUAL_DOUBLE_CLICK */", double_click)
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
    _execute(exe, mode="--hover-matrix")
    _execute(exe, mode="--null-current")
    for mode in ("--key", "--key-mutation", "--key-null", "--key-default",
                 "--double-click", "--double-null"):
        _execute(exe, mode=mode)
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
    text = SOURCE.read_text().replace("Dialog::row_count_", "Dialog::field_40_")
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


def test_keyboard_double_click_callback_confusion_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text()
    key = _body(text, "on_key_down", "int")
    damaged_key = key.replace("selection_callback_", "double_click_callback_")
    assert damaged_key != key
    text = text.replace(key, damaged_key, 1)
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe), "--key"], text=True, capture_output=True,
                            timeout=20, env={**__import__("os").environ,
                                             "ASAN_OPTIONS": "detect_leaks=0",
                                             "UBSAN_OPTIONS": "halt_on_error=1"})
    assert result.returncode != 0


def test_mutated_repeated_hover_suppression_is_caught(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text()
    old = """    Dialog::field_F0_ = static_cast<uint32_t>(position);
    redraw();"""
    guarded = """    if (Dialog::field_F0_ != static_cast<uint32_t>(position)) {
        Dialog::field_F0_ = static_cast<uint32_t>(position);
        redraw();
    }"""
    assert text.count(old) == 1
    text = text.replace(old, guarded, 1)
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe), "--hover-matrix"], text=True, capture_output=True,
                            timeout=20, env={**__import__("os").environ,
                                             "ASAN_OPTIONS": "detect_leaks=0",
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


@pytest.mark.parametrize("damage", ("selected-hover", "exhausted-list"))
def test_original_hover_transition_damage_is_caught(damage, tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    text = SOURCE.read_text()
    if damage == "selected-hover":
        old = "        position = -1;"
        new = "        /* damaged: selected ordinal survives */"
    else:
        old = """                          Dialog::item_list_.entry_count_ - 1) {
            return;
        }"""
        # Restrict the mutation to the actual mouse-move body.
        start = text.index("void RadioButton::on_mouse_move(")
        prefix, text = text[:start], text[start:]
        new = old.replace("return;", "break;")
    assert text.count(old) == 1
    text = text.replace(old, new, 1)
    if damage == "exhausted-list":
        text = prefix + text
    built, exe = _compile(tmp_path, compiler, text, validate=False)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe), "--hover-matrix"], text=True,
                            capture_output=True, timeout=20)
    assert result.returncode != 0
