"""Run the actual BasePop constructor dialog-tail against a small host owner."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body_after(source: str, anchor: str) -> str:
    start = source.index(anchor)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated source block after {anchor!r}")


def test_actual_constructor_dialog_tail_null_nonnull_and_stores(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source = (ROOT / "src/basepop.cpp").read_text()
    constructor = source[source.index("BasePop::BasePop()") :]
    tail = _body_after(constructor, "if (external_window_ != 0)")
    assert "dialogs_.Dialog::field_2C_ = dialog_width_;" in tail
    assert "height = (height * 3) / 2;" in tail
    assert "if (external_window_ == 0" in tail
    assert "dialogs_.Dialog::field_20_ |= 3;" in tail

    fixture = r'''
#include <cassert>
#include <cstdint>
using uint32_t = std::uint32_t;
struct ListBox {
    uint32_t field_2C_ = 0x11111111u;
    uint32_t field_30_ = 0x22222222u;
};
struct Dialog {
    uint32_t field_2C_ = 0x33333333u;
    uint32_t field_30_ = 0x44444444u;
    uint32_t field_20_ = 0;
};
struct Dialogs : ListBox, Dialog {};
extern int WinScreenWidth;
struct BasePopTail {
    void *external_window_ = nullptr;
    uint32_t dialog_width_ = 0, dialog_height_ = 0;
    uint32_t default_loc_a_ = 0, default_loc_b_ = 0;
    uint32_t loc_a_ = 71, loc_b_ = 72;
    uint32_t field_30A8_ = 0;
    Dialogs dialogs_;
        void apply_tail() TAIL
};
int WinScreenWidth = 1024;
int main() {
    BasePopTail null_window;
    null_window.dialog_width_ = 640;
    null_window.dialog_height_ = 320;
    null_window.default_loc_a_ = 11;
    null_window.default_loc_b_ = 12;
    null_window.dialogs_.Dialog::field_2C_ = 13;
    null_window.dialogs_.Dialog::field_30_ = 14;
    null_window.dialogs_.Dialog::field_20_ = 0x80000004u;
    null_window.apply_tail();
    assert(null_window.dialogs_.Dialog::field_2C_ == 13);
    assert(null_window.dialogs_.Dialog::field_30_ == 14);
    assert(null_window.dialogs_.Dialog::field_20_ == 0x80000004u);
    assert(null_window.loc_a_ == 71 && null_window.loc_b_ == 72);
    assert(null_window.dialogs_.ListBox::field_2C_ == 0x11111111u);
    assert(null_window.dialogs_.ListBox::field_30_ == 0x22222222u);

    BasePopTail attached;
    attached.external_window_ = reinterpret_cast<void *>(1);
    attached.dialog_width_ = 640;
    attached.dialog_height_ = 321;
    attached.default_loc_a_ = 0x2000;
    attached.default_loc_b_ = 44;
    attached.loc_a_ = 91;
    attached.loc_b_ = 92;
    attached.field_30A8_ = 0xA000u;
    attached.dialogs_.Dialog::field_20_ = 0x80000004u;
    attached.apply_tail();
    assert(attached.dialogs_.Dialog::field_2C_ == 640);
    // The inner null-window recheck remains false under this non-null outer
    // guard, so the pinned constructor stores the unscaled signed height.
    assert(attached.dialogs_.Dialog::field_30_ == 321);
    assert(attached.loc_a_ == 91 && attached.loc_b_ == 44);
    assert(attached.dialogs_.Dialog::field_20_ == 0x80000007u);
    assert(attached.dialogs_.Dialog::field_2C_ == 640);
    assert(attached.dialogs_.Dialog::field_30_ == 321);
    assert(attached.dialogs_.ListBox::field_2C_ == 0x11111111u);
    assert(attached.dialogs_.ListBox::field_30_ == 0x22222222u);

    attached.default_loc_a_ = 13;
    attached.default_loc_b_ = 0x2000;
    attached.field_30A8_ = 0xA400u;
    attached.dialog_width_ = 0xFFFFFFEFu;
    attached.dialog_height_ = 0xFFFFFFEDu;
    attached.dialogs_.Dialog::field_20_ = 0x40000008u;
    attached.apply_tail();
    assert(attached.loc_a_ == 13 && attached.loc_b_ == 44);
    assert(attached.dialogs_.Dialog::field_20_ == 0x4000000Bu);
    assert(attached.dialogs_.Dialog::field_2C_ == 0xFFFFFFEFu);
    assert(attached.dialogs_.Dialog::field_30_ == 0xFFFFFFEDu);
    assert(attached.dialogs_.ListBox::field_2C_ == 0x11111111u);
    assert(attached.dialogs_.ListBox::field_30_ == 0x22222222u);
}
'''.replace("TAIL", "{\n" + tail + "\n}")
    cpp_file = tmp_path / "basepop_dialog_tail.cpp"
    binary = tmp_path / "basepop_dialog_tail"
    cpp_file.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp_file), "-o", str(binary)],
        check=True,
        timeout=30,
    )
    subprocess.run([str(binary)], check=True, timeout=30)

    for label, statement in (
        ("width", "dialogs_.Dialog::field_2C_ = dialog_width_;"),
        ("style", "dialogs_.Dialog::field_20_ |= 3;"),
    ):
        assert fixture.count(statement) == 1
        mutant_file = tmp_path / f"basepop_dialog_tail_missing_{label}.cpp"
        mutant_binary = tmp_path / f"basepop_dialog_tail_missing_{label}"
        mutant_file.write_text(fixture.replace(statement, "/* omitted by damage control */"))
        subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(mutant_file), "-o", str(mutant_binary)],
            check=True,
            timeout=30,
        )
        damaged = subprocess.run([str(mutant_binary)], capture_output=True, timeout=30)
        assert damaged.returncode != 0, f"missing {label} write escaped the host control"
