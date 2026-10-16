"""Check Scroll::set_pos's void source contract and preserved state behavior."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_set_pos_contract_and_behavior(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    source = (ROOT / "src/scroll.cpp").read_text()
    header = (ROOT / "src/scroll.h").read_text()
    assert "void set_pos(int position);" in header
    assert "uint32_t set_pos(int position);" not in header
    body = definition(source, "void Scroll::set_pos(int position)")
    assert "uint32_t Scroll::set_pos" not in source
    helper = definition(source, "inline LONG long_from_bits(uint32_t bits)")

    fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef int32_t LONG;
struct Win { int identity; };
Win *WinCallbackWindow;
static int redraw_count;
static Win *redraw_receiver;
struct Scroll;
static Scroll *redraw_owner;
static int redraw_position;
struct Scroll {
    Win *win_parent_;
    int range_minimum_, range_maximum_, position_;
    uint32_t range_reversed_;
    Scroll() : win_parent_(0), range_minimum_(0), range_maximum_(0),
               position_(0), range_reversed_(0) {}
    virtual void redraw() {
        ++redraw_count;
        redraw_owner = this;
        redraw_position = position_;
        redraw_receiver = win_parent_;
        assert(WinCallbackWindow == win_parent_);
    }
    void set_pos(int position);
};
''' + helper + "\n" + body + r'''
static void check(int argument, int expected) {
    Scroll value;
    Win parent = { 17 };
    value.win_parent_ = &parent;
    value.range_minimum_ = 3;
    value.range_maximum_ = 8;
    WinCallbackWindow = 0;
    redraw_receiver = 0;
    redraw_count = 0;
    value.set_pos(argument);
    assert(value.position_ == expected);
    assert(redraw_count == 1 && redraw_receiver == &parent);
    assert(redraw_owner == &value && redraw_position == expected);
    assert(WinCallbackWindow == &parent);
}
int main() {
    Win previous = { 9 };
    Scroll absent;
    absent.range_minimum_ = -4; absent.range_maximum_ = 12;
    absent.position_ = 6; absent.range_reversed_ = 1;
    WinCallbackWindow = &previous; redraw_count = 0; redraw_receiver = 0;
    absent.set_pos(100);
    assert(absent.position_ == 6 && absent.range_minimum_ == -4 &&
           absent.range_maximum_ == 12 && absent.range_reversed_ == 1);
    assert(WinCallbackWindow == &previous && redraw_count == 0 && !redraw_receiver);

    check(-20, 3);
    check(6, 6);
    check(20, 8);

    Scroll reversed;
    Win parent = { 21 };
    reversed.win_parent_ = &parent;
    reversed.range_minimum_ = 3; reversed.range_maximum_ = 8;
    reversed.range_reversed_ = 1; WinCallbackWindow = 0; redraw_count = 0;
    reversed.set_pos(-20);
    assert(reversed.position_ == 8 && redraw_count == 1 && redraw_receiver == &parent);
    assert(redraw_owner == &reversed && redraw_position == 8);
    redraw_count = 0; reversed.set_pos(6);
    assert(reversed.position_ == 5 && redraw_count == 1);
    assert(redraw_position == 5);
    redraw_count = 0; reversed.set_pos(20);
    assert(reversed.position_ == 3 && redraw_count == 1);
    assert(redraw_position == 3);

    // The reversed expression is 32-bit modulo arithmetic even when its
    // intermediate signed interpretation would overflow: 0 - INT_MIN + INT_MIN.
    reversed.range_minimum_ = INT32_MIN;
    reversed.range_maximum_ = 0;
    redraw_count = 0; reversed.set_pos(INT32_MIN);
    assert(reversed.position_ == 0 && redraw_count == 1);
    assert(redraw_position == 0);
}
'''
    generated = tmp_path / "scroll_set_pos.cpp"
    generated.write_text(fixture)
    executable = tmp_path / f"scroll-set-pos-{compiler_name}"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror=return-type",
         str(generated), "-o", str(executable)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_pinned_set_pos_control_flow_and_callback_store():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    rows = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x605D20, 0x605D8A)), 0x605D20))
    decoded = [(row.address, row.mnemonic, row.op_str) for row in rows]
    cursor = 0x605D20
    for row in rows:
        assert row.address == cursor
        cursor += row.size
    assert cursor == 0x605D8A
    assert decoded[0:3] == [
        (0x605D20, "mov", "eax, dword ptr [ecx + 0xc4]"),
        (0x605D26, "test", "eax, eax"),
        (0x605D28, "je", "0x605d87"),
    ]
    assert (0x605D2B, "mov", "dword ptr [0x9b7ab8], eax") in decoded
    assert (0x605D3C, "jge", "0x605d46") in decoded
    assert (0x605D4E, "jle", "0x605d58") in decoded
    assert (0x605D80, "call", "dword ptr [edx + 0xf8]") in decoded
    assert decoded[-1] == (0x605D87, "ret", "4")
    assert all("eax" not in operands.lower() for address, _, operands in decoded
               if address > 0x605D80)
