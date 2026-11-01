"""CheckBox state controls on x86 hosts, with separate shift-sanitizer controls.

The actual-source run preserves negative and >=32 shift counts. At -O0 on the
explicit x86 host domain, we observe the emitted hardware count wrapping with
only shift instrumentation disabled; all address/other undefined sanitizers
remain enabled. This is neither portable C++ proof nor VC6 target evidence.
Strict native function byte/reference checks supply the latter separately.

A clearly labeled host-only masked projection retains the shift-sanitizer damage
control: removing its explicit mask must diagnose the raw out-of-range shift.
The projection never substitutes for executing the actual product bodies.
"""

from pathlib import Path
import os
import platform
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/checkbox.cpp"


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
                index += 1
                continue
            if char == "'":
                state = "char"
                index += 1
                continue
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        index += 1
    raise AssertionError("unterminated product definition: " + signature)


def actual_methods():
    text = SOURCE.read_text(encoding="utf-8")
    return "\n\n".join((
        definition(text, "void CheckBox::UNK1(int pos)"),
        definition(text, "int CheckBox::get_state_id(int id)"),
    ))


FIXTURE = r'''
#include <assert.h>
#include <stdint.h>

struct Entry {
    int id;
    Entry *next;
};

struct StringStruct {
    int entry_count_;
    Entry *current_;
    int current_position_;
    Entry *head_;
};

struct GraphicWin {
    virtual ~GraphicWin() {}
};

struct Dialog {
    StringStruct item_list_;
    int selected_position_;
};

class CheckBox : public virtual GraphicWin, public virtual Dialog {
public:
    int wrong_selected_position_;
    void UNK1(int pos);
    int get_state_id(int id);
};

BODIES

static void link_entries(Entry *entries, int count) {
    for (int index = 0; index < count; ++index) {
        entries[index].id = index;
        entries[index].next = index + 1 < count ? &entries[index + 1] : 0;
    }
}

int main() {
    CheckBox box;
    Dialog &dialog = box;
    StringStruct &items = dialog.item_list_;
    box.wrong_selected_position_ = 0;

    // UNK1 retains the original raw x86 shift and toggles the real int member.
    dialog.selected_position_ = 0xA5;
    box.UNK1(2);
    assert(dialog.selected_position_ == 0xA1);
    box.UNK1(2);
    assert(dialog.selected_position_ == 0xA5);
    box.UNK1(31);
    assert(static_cast<uint32_t>(dialog.selected_position_) == 0x800000A5U);
    box.UNK1(31);
    assert(dialog.selected_position_ == 0xA5);
    assert(box.wrong_selected_position_ == 0);

    Entry short_entries[3];
    link_entries(short_entries, 3);
    items.entry_count_ = 3;
    items.head_ = &short_entries[0];
    items.current_ = &short_entries[2];
    items.current_position_ = 29;
    dialog.selected_position_ = 0x80000002U;
    assert(box.get_state_id(1) == 2);
    assert(items.current_position_ == 1);
    assert(items.current_ == &short_entries[1]);

    // A miss advances through the complete list and returns the full mask
    // selected at the resulting ordinal.
    dialog.selected_position_ = 8;
    items.current_position_ = 19;
    items.current_ = &short_entries[0];
    assert(box.get_state_id(99) == 8);
    assert(items.current_position_ == 3);
    assert(items.current_ == 0);

    // A null head preserves the stale cursor, even with a positive count;
    // the x86 host run observes SHL count wrapping, not defined portable C++.
    items.head_ = 0;
    items.entry_count_ = 1;
    items.current_ = &short_entries[2];
    items.current_position_ = -1;
    dialog.selected_position_ = static_cast<int>(0x80000000U);
    assert(static_cast<uint32_t>(box.get_state_id(123)) == 0x80000000U);
    assert(items.current_position_ == -1);
    assert(items.current_ == &short_entries[2]);

    // A nonnull head resets the cursor even when the signed count is zero.
    items.entry_count_ = 0;
    items.head_ = &short_entries[0];
    items.current_ = &short_entries[2];
    items.current_position_ = 2;
    dialog.selected_position_ = static_cast<int>(0x80000000U);
    assert(box.get_state_id(123) == 0);
    assert(items.current_position_ == 0);
    assert(items.current_ == &short_entries[0]);

    // Large but valid lists exercise x86's wrapped shift count at 32 and 63.
    Entry many_entries[64];
    link_entries(many_entries, 64);
    items.entry_count_ = 64;
    items.head_ = &many_entries[0];
    dialog.selected_position_ = 1;
    assert(box.get_state_id(32) == 1);
    assert(items.current_position_ == 32);
    assert(items.current_ == &many_entries[32]);
    dialog.selected_position_ = static_cast<int>(0xC0000000U);
    assert(static_cast<uint32_t>(box.get_state_id(63)) == 0x80000000U);
    assert(items.current_position_ == 63);
    assert(items.current_ == &many_entries[63]);
}
'''


def compile_and_run(tmp_path, compiler, methods, stem, *, check_shifts=False):
    source = tmp_path / (stem + ".cpp")
    executable = tmp_path / stem
    source.write_text(FIXTURE.replace("BODIES", methods))
    flags = ["-O0", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    if not check_shifts:
        # Intentional original x86 effect; do not suppress other UBSan checks.
        flags.append("-fno-sanitize=shift")
    compiled = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", *flags,
         str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run(
        [str(executable)], capture_output=True, text=True,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
                 UBSAN_OPTIONS="halt_on_error=1"),
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_checkbox_selected_state_and_compiled_damage(tmp_path, compiler_name):
    if platform.machine().lower() not in ("i386", "i486", "i586", "i686", "x86", "x86_64", "amd64"):
        pytest.skip("raw out-of-range shift observation requires an x86 host")
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")

    methods = actual_methods()
    result = compile_and_run(tmp_path, compiler, methods, "checkbox-actual")
    assert result.returncode == 0, result.stdout + result.stderr

    wrong_member = methods.replace(
        "int *const selected = &selected_position_;",
        "int *const selected = &wrong_selected_position_;", 1,
    )
    assert wrong_member != methods
    result = compile_and_run(tmp_path, compiler, wrong_member,
                             "checkbox-wrong-member")
    assert result.returncode != 0, "wrong-member mutant escaped the behavior control"

    assert "Assertion" in result.stderr or "Assertion" in result.stdout, result.stderr

    raw_shift = "1U << static_cast<uint32_t>(Dialog::item_list_.current_position_)"
    assert methods.count(raw_shift) == 1
    guarded_shift = methods.replace(
        raw_shift,
        "(Dialog::item_list_.current_position_ >= 0 && "
        "Dialog::item_list_.current_position_ < 32 ? " + raw_shift + " : 0U)", 1,
    )
    result = compile_and_run(tmp_path, compiler, guarded_shift, "checkbox-guarded-shift")
    assert result.returncode != 0, "rejecting wrapped ordinals escaped boundary assertions"
    assert "Assertion" in result.stderr or "Assertion" in result.stdout, result.stderr

    # Portable host-only control. The product still uses the raw expression;
    # this projection establishes that the separate shift detector is live.
    masked_shift = "1U << (static_cast<uint32_t>(Dialog::item_list_.current_position_) & 31U)"
    masked_projection = methods.replace(raw_shift, masked_shift, 1)
    assert masked_projection != methods
    result = compile_and_run(tmp_path, compiler, masked_projection,
                             "checkbox-masked-projection", check_shifts=True)
    assert result.returncode == 0, result.stdout + result.stderr
    undefined_shift = masked_projection.replace(masked_shift, raw_shift, 1)
    assert undefined_shift != masked_projection
    result = compile_and_run(tmp_path, compiler, undefined_shift,
                             "checkbox-undefined-shift", check_shifts=True)
    assert result.returncode != 0, "unmasked out-of-range shift escaped UBSan"
    assert "shift exponent" in result.stderr, result.stderr
