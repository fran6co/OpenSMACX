"""Pinned switch-table and actual-source controls for SpriteBox key navigation."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


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


def test_pinned_remap_and_jump_table_select_the_four_native_actions():
    if not IMAGE.is_file():
        pytest.skip("pinned executable unavailable")
    image = PeImage(IMAGE.read_bytes(), IMAGE_SHA256)
    table = image.read(Span(0x611580, 0x611594))
    assert struct.unpack("<5I", table) == (
        0x611494, 0x611424, 0x611513, 0x61145A, 0x611578
    )
    remap = image.read(Span(0x611594, 0x6115D8))
    assert {key: remap[key - 0x25] for key in (
        0x25, 0x26, 0x27, 0x28, 0x62, 0x64, 0x66, 0x68
    )} == {
        0x25: 0, 0x26: 1, 0x27: 2, 0x28: 3,
        0x62: 3, 0x64: 0, 0x66: 2, 0x68: 1,
    }
    assert all(remap[key - 0x25] == 4 for key in (0x29, 0x41, 0x67))
    # Key dispatch: subtract-one/row-step guards and wrapped row limit are
    # pinned separately from table bytes; default and guarded paths return 1/0.
    instructions = {
        ins.address: (ins.mnemonic, ins.op_str)
        for ins in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
            image.read(Span(0x611400, 0x611580)), 0x611400
        )
    }
    assert instructions[0x6114AA] == ("cmp", "ecx, 1")
    assert instructions[0x6114C2] == ("jge", "0x6114cf")
    assert instructions[0x611538] == ("imul", "ebx, edx")
    assert instructions[0x611540] == ("jl", "0x61154d")
    assert instructions[0x61154F] == ("mov", "dword ptr [eax + 0x60], edx")
    assert instructions[0x611563] == ("cmp", "ebx, edx")
    assert instructions[0x611565] == ("jle", "0x6114d6")
    assert instructions[0x61156C] == ("mov", "dword ptr [eax], ecx")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_key_navigation_and_mutation_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/spritebox.cpp").read_text(encoding="utf-8")
    body = _body(source, "int SpriteBox::on_key_down(int key)")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <initializer_list>

enum { VK_LEFT=0x25, VK_UP=0x26, VK_RIGHT=0x27, VK_DOWN=0x28,
       VK_NUMPAD2=0x62, VK_NUMPAD4=0x64, VK_NUMPAD6=0x66, VK_NUMPAD8=0x68 };
struct Dialog {
    int column_count_;
    int row_count_;
    int selected_position_;
};
struct SpriteList { int entry_count_; };
struct SpriteBox : Dialog {
    SpriteList list_;
    int draws[4], draw_selection[4], draw_count;
    int updates, update_selection;
    int mutate_first_draw;
    int on_key_down(int key);
    void draw_item(int position) {
        assert(draw_count < 4);
        draws[draw_count] = position;
        draw_selection[draw_count] = selected_position_;
        ++draw_count;
        if (draw_count == 1 && mutate_first_draw != 0)
            selected_position_ = mutate_first_draw;
    }
    void on_update() { ++updates; update_selection = selected_position_; }
};
''' + body + r'''

static void reset(SpriteBox &box, uint32_t rows, uint32_t columns,
                  int count, int position) {
    box.column_count_ = rows;
    box.row_count_ = columns;
    box.list_.entry_count_ = count;
    box.selected_position_ = position;
    box.draw_count = box.updates = 0;
    box.update_selection = -999;
    box.mutate_first_draw = 0;
    for (int i=0; i<4; ++i) box.draws[i] = box.draw_selection[i] = -999;
}
static void moved(SpriteBox &box, int key, int old_pos, int new_pos) {
    assert(box.on_key_down(key) == 1);
    assert(box.selected_position_ == new_pos);
    assert(box.draw_count == 2 && box.updates == 1);
    assert(box.draws[0] == old_pos && box.draws[1] == new_pos);
    assert(box.draw_selection[0] == new_pos && box.draw_selection[1] == new_pos);
    assert(box.update_selection == new_pos);
}
static void stopped(SpriteBox &box, int key, uint32_t rows, uint32_t columns,
                    int count, int position) {
    reset(box, rows, columns, count, position);
    assert(box.on_key_down(key) == 1);
    assert(box.selected_position_ == position);
    assert(box.draw_count == 0 && box.updates == 0);
}
int main() {
    SpriteBox box = {};
    reset(box, 3, 4, 12, 5);
    moved(box, VK_LEFT, 5, 1);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_NUMPAD4, 5, 1);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_UP, 5, 4);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_NUMPAD8, 5, 4);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_RIGHT, 5, 9);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_NUMPAD6, 5, 9);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_DOWN, 5, 6);
    reset(box, 3, 4, 12, 5);
    moved(box, VK_NUMPAD2, 5, 6);

    // Exact original boundary guards: row count/column range for horizontal
    // moves, zero and entry_count-1 for single-step moves.
    stopped(box, VK_LEFT, 1, 4, 12, 5);
    stopped(box, VK_LEFT, 3, 4, 12, 3);
    reset(box, 3, 4, 12, 4); moved(box, VK_LEFT, 4, 0);
    stopped(box, VK_RIGHT, 1, 4, 12, 0);
    stopped(box, VK_RIGHT, 3, 4, 12, 8);
    reset(box, 3, 4, 12, 7); moved(box, VK_RIGHT, 7, 11);
    // The right handler also checks the actual list end after publishing the
    // column step: a partial final row can reject a move that passed the
    // geometric last-row-start guard, restoring the original selection.
    stopped(box, VK_RIGHT, 3, 4, 10, 7);
    reset(box, 3, 4, 10, 5); moved(box, VK_RIGHT, 5, 9);
    stopped(box, VK_UP, 3, 4, 12, 0);
    reset(box, 3, 4, 12, 1); moved(box, VK_UP, 1, 0);
    stopped(box, VK_DOWN, 3, 4, 12, 11);
    reset(box, 3, 4, 12, 10); moved(box, VK_DOWN, 10, 11);

    // The native handler does not reject the whole function for empty lists;
    // it still follows the per-key branch, including these odd states.
    reset(box, 3, 4, 0, 2); moved(box, VK_UP, 2, 1);
    stopped(box, VK_DOWN, 3, 4, 0, -1);
    reset(box, 3, 4, 0, 0); moved(box, VK_DOWN, 0, 1);
    reset(box, 3, 4, 12, -1); moved(box, VK_UP, -1, -2);
    reset(box, 3, 4, 12, -1); moved(box, VK_RIGHT, -1, 3);
    reset(box, 2, 0, 12, 0); moved(box, VK_LEFT, 0, 0); // zero step still redraws
    reset(box, 0x80000000u, 2, 12, 0);
    stopped(box, VK_RIGHT, 0x80000000u, 2, 12, 0); // imul wrap => signed -2

    // A recognized path publishes first, draws old then rereads the shared
    // selection for draw two, and only then calls the virtual update.
    reset(box, 3, 4, 12, 5);
    box.mutate_first_draw = 77;
    assert(box.on_key_down(VK_LEFT) == 1);
    assert(box.draw_count == 2 && box.draws[0] == 5 && box.draws[1] == 77);
    assert(box.draw_selection[0] == 1 && box.draw_selection[1] == 77);
    assert(box.updates == 1 && box.update_selection == 77);

    for (int key : {0x24, 0x29, 0x41, 0x69, 0xFFFF}) {
        reset(box, 3, 4, 12, 5);
        assert(box.on_key_down(key) == 0);
        assert(box.selected_position_ == 5 && box.draw_count == 0 && box.updates == 0);
    }
}
'''

    def build_and_run(candidate_body: str, stem: str, expect_success: bool):
        candidate_harness = harness.replace(body, candidate_body, 1)
        cpp = tmp_path / f"{stem}.cpp"
        exe = tmp_path / stem
        cpp.write_text(candidate_harness, encoding="utf-8")
        subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            check=True,
        )
        result = subprocess.run([str(exe)], env=os.environ.copy(), check=False)
        assert (result.returncode == 0) == expect_success, (
            f"{stem}: exit {result.returncode}, expected success={expect_success}"
        )

    build_and_run(body, f"spritebox_key_{compiler_name.replace('+', 'p')}", True)
    cached_selection = body.replace(
        "draw_item(Dialog::selected_position_);", "draw_item(next_position);", 1,
    )
    assert cached_selection != body
    build_and_run(cached_selection, f"spritebox_reload_mutant_{compiler_name.replace('+', 'p')}", False)
    mapping_mutant = body.replace(
        "next_position -= Dialog::row_count_;",
        "--next_position;", 1,
    )
    assert mapping_mutant != body
    build_and_run(mapping_mutant, f"spritebox_map_mutant_{compiler_name.replace('+', 'p')}", False)
    ordering_mutant = body.replace(
        "Dialog::selected_position_ = next_position;\n    draw_item(old_position);",
        "draw_item(old_position);\n    Dialog::selected_position_ = next_position;", 1,
    )
    assert ordering_mutant != body
    build_and_run(ordering_mutant, f"spritebox_order_mutant_{compiler_name.replace('+', 'p')}", False)
    empty_guard_mutant = body.replace(
        "const int count = list_.entry_count_;",
        "const int count = list_.entry_count_;\n    if (count <= 0) return 0;", 1,
    )
    assert empty_guard_mutant != body
    build_and_run(empty_guard_mutant, f"spritebox_empty_mutant_{compiler_name.replace('+', 'p')}", False)
    missing_right_end_guard = body.replace(
        "        Dialog::selected_position_ = next_position;\n"
        "        if (Dialog::selected_position_ > count - 1) {\n"
        "            Dialog::selected_position_ = old_position;\n"
        "            return 1;\n"
        "        }\n",
        "        Dialog::selected_position_ = next_position;\n",
        1,
    )
    assert missing_right_end_guard != body
    build_and_run(
        missing_right_end_guard,
        f"spritebox_right_end_mutant_{compiler_name.replace('+', 'p')}",
        False,
    )
