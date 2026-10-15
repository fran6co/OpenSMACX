"""Compile the actual CheckBox state methods with typed host list members."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/checkbox.cpp"


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, "actual product definition missing: " + signature
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated product definition: " + signature)


def test_actual_checkbox_state_methods_preserve_distinct_searches(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")

    source = SOURCE.read_text()
    methods = "\n\n".join(
        extract_definition(source, signature)
        for signature in (
            "void CheckBox::set_state_id(int id, int value)",
            "void CheckBox::flip_state_id(int id)",
            "int CheckBox::get_state_id(int id)",
        )
    )
    flip = extract_definition(source, "void CheckBox::flip_state_id(int id)")
    # Plain typed members cannot observe how many times the original loop
    # reads nodes; this is only a structural source assertion, not runtime
    # proof of the two passes. Runtime controls below verify both outcomes.
    assert flip.count("if (items.head_ != 0)") == 3
    assert "seek_id(" not in methods
    assert "^=" not in flip

    setter = extract_definition(source, "void CheckBox::set_state_id(int id, int value)")
    getter = extract_definition(source, "int CheckBox::get_state_id(int id)")

    prefix = r'''#include <cassert>
#include <cstdint>
struct StringStructEntry {
    int id;
    StringStructEntry *next;
};
struct StringStruct {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
};
struct Dialog {
    StringStruct item_list_;
    uint32_t selected_position_;
};
struct CheckBox : Dialog {
    void set_state_id(int id, int value);
    void flip_state_id(int id);
    int get_state_id(int id);
};
'''
    main = r'''
int main() {
    StringStructEntry nodes[32];
    for (int i = 0; i < 32; ++i) {
        nodes[i].id = 10 + i;
        nodes[i].next = i == 31 ? 0 : &nodes[i + 1];
    }
    CheckBox box = {};
    box.item_list_.head_ = nodes;
    box.item_list_.current_ = nodes;
    box.item_list_.entry_count_ = 32;
    box.item_list_.current_position_ = 0;

    // Found lookup updates both cursor members; getter returns the complete
    // bit mask rather than a normalized bool, and setter clears/sets it.
    box.set_state_id(11, 1);
    assert(box.item_list_.current_ == &nodes[1]);
    assert(box.item_list_.current_position_ == 1);
    assert(box.selected_position_ == 2u);
    assert(box.get_state_id(11) == 2);
    box.set_state_id(11, 0);
    assert(box.selected_position_ == 0u);

    // A high ordinal keeps the original low-five-bit shift behavior and the
    // int getter's full high-bit result.
    box.set_state_id(41, 1);
    assert(box.item_list_.current_ == &nodes[31]);
    assert(box.item_list_.current_position_ == 31);
    assert(box.selected_position_ == 0x80000000u);
    assert(static_cast<uint32_t>(box.get_state_id(41)) == 0x80000000u);

    // Missing IDs walk to null/count. Flip clears or sets from the first
    // pass's bit, then leaves the cursor from its second pass.
    box.selected_position_ = 1u;
    box.flip_state_id(999);
    assert(box.selected_position_ == 0u);
    assert(box.item_list_.current_ == 0 && box.item_list_.current_position_ == 32);
    box.flip_state_id(999);
    assert(box.selected_position_ == (1u << 0));
    assert(box.item_list_.current_ == 0 && box.item_list_.current_position_ == 32);

    // The setter's head guard leaves a stale cursor when head is null; its
    // bit update still uses the stale position. Getter instead exits on a
    // zero count without touching that cursor.
    box.item_list_.head_ = 0;
    box.item_list_.current_ = &nodes[2];
    box.item_list_.current_position_ = 2;
    box.item_list_.entry_count_ = 0;
    box.selected_position_ = 0;
    box.set_state_id(999, 1);
    assert(box.selected_position_ == (1u << 2));
    assert(box.item_list_.current_ == &nodes[2] && box.item_list_.current_position_ == 2);
    box.selected_position_ = 1u << 2;
    assert(box.get_state_id(999) == (1 << 2));
    assert(box.item_list_.current_ == &nodes[2] && box.item_list_.current_position_ == 2);

    // With a non-null head and zero count, setter/flip reset to position 0;
    // getter's count-zero path preserves a separately seeded position.
    box.item_list_.head_ = nodes;
    box.item_list_.current_ = &nodes[4];
    box.item_list_.current_position_ = 4;
    box.selected_position_ = 0;
    box.set_state_id(999, 1);
    assert(box.item_list_.current_ == nodes && box.item_list_.current_position_ == 0);
    assert(box.selected_position_ == 1u);
    box.item_list_.current_ = &nodes[4];
    box.item_list_.current_position_ = 4;
    box.selected_position_ = 1u << 4;
    assert(box.get_state_id(999) == (1 << 4));
    assert(box.item_list_.current_ == &nodes[4] && box.item_list_.current_position_ == 4);
    box.selected_position_ = 1u;
    box.flip_state_id(999);
    assert(box.selected_position_ == 0u);
    assert(box.item_list_.current_ == nodes && box.item_list_.current_position_ == 0);

    // Negative counts preserve the distinct guards too: set skips when the
    // head is null, while get resets on any nonzero count before its signed
    // loop test. Flip's two head-gated passes leave the seeded cursor alone.
    box.item_list_.head_ = 0;
    box.item_list_.current_ = &nodes[5];
    box.item_list_.current_position_ = 5;
    box.item_list_.entry_count_ = -1;
    box.selected_position_ = 0;
    box.set_state_id(999, 1);
    assert(box.selected_position_ == (1u << 5));
    assert(box.item_list_.current_ == &nodes[5] && box.item_list_.current_position_ == 5);
    box.item_list_.current_ = &nodes[6];
    box.item_list_.current_position_ = 6;
    box.selected_position_ = 1u;
    assert(box.get_state_id(999) == 1);
    assert(box.item_list_.current_ == 0 && box.item_list_.current_position_ == 0);
    box.item_list_.current_ = &nodes[5];
    box.item_list_.current_position_ = 5;
    box.selected_position_ = 1u << 5;
    box.flip_state_id(999);
    assert(box.selected_position_ == 0u);
    assert(box.item_list_.current_ == &nodes[5] && box.item_list_.current_position_ == 5);

    // Mutating the list between entries makes each method search the current
    // chain rather than trusting a previous cursor or ID placement.
    box.item_list_.entry_count_ = 3;
    box.item_list_.head_ = nodes;
    nodes[0].next = &nodes[2];
    nodes[2].next = &nodes[3];
    nodes[0].id = 70;
    nodes[2].id = 71;
    nodes[3].id = 72;
    box.set_state_id(71, 1);
    assert(box.item_list_.current_ == &nodes[2] && box.item_list_.current_position_ == 1);
    assert((box.selected_position_ & (1u << 1)) != 0);
    box.set_state_id(72, 1);
    assert(box.get_state_id(72) == (1 << 2));
    assert(box.item_list_.current_ == &nodes[3] && box.item_list_.current_position_ == 2);
}
'''
    def compile_and_run(label, candidate_methods, expected_success):
        cpp = tmp_path / ("checkbox-state-" + label + ".cpp")
        exe = tmp_path / ("checkbox-state-" + label)
        cpp.write_text(prefix + candidate_methods + main)
        subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)], check=True, capture_output=True,
                       text=True, timeout=30)
        result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        assert (result.returncode == 0) == expected_success, label + ": " + result.stderr

    compile_and_run("actual", methods, True)

    # Deliberate source regressions: each must be caught by the actual-state
    # controls, not merely by checking the spelling of the production body.
    wrong_get_guard = methods.replace(
        "if (items.entry_count_ != 0)", "if (items.head_ != 0)", 1)
    assert wrong_get_guard != methods
    compile_and_run("mutant-getter-head-guard", wrong_get_guard, False)

    narrow_getter = methods.replace(
        "return static_cast<int>(Dialog::selected_position_ & mask);",
        "return (Dialog::selected_position_ & mask) != 0;", 1)
    assert narrow_getter != methods
    compile_and_run("mutant-getter-bool", narrow_getter, False)

    wrong_set_guard = methods.replace(
        "if (items.head_ != 0)", "if (items.entry_count_ != 0)", 1)
    assert wrong_set_guard != methods
    compile_and_run("mutant-setter-count-guard", wrong_set_guard, False)
