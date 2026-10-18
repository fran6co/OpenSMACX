"""Actual-source controls for BasePop self-owner position calculations.

The selected current position block is compiled with typed host stand-ins and
tested over its owner/flag/location branches. This is not VC6 ABI, Win32,
or game-runtime evidence.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
import pytest

from test_basepop_dialog_attach_oct17 import _function_body


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXEC_SPAN = Span(0x00602600, 0x006037F5)
POSITION_MARKER = "// THE POPUP'S POSITION"
ON_REDRAW = "    // DRAW IT."
BLOCK_MARKER = "/* ACTUAL_BASEPOP_POSITION_BLOCK */"


HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct RECT { int32_t left, top, right, bottom; };
struct BitmapInfoHeader { int32_t biWidth, biHeight; };
struct DIB { BitmapInfoHeader bmiHeader; };
struct GraphicWin {
    DIB dib_;
    RECT client_rect_;
};

static int WinScreenWidth = 1024;
static int WinScreenHeight = 768;

struct BasePop : GraphicWin {
    uint32_t iSomeFlag_;
    uint32_t loc_a_, loc_b_;
    uint32_t field_30BC_, field_30C0_;
    GraphicWin *owner_window_;
    int32_t field_30B4_, field_30B8_;

    BasePop() : iSomeFlag_(0), loc_a_(0), loc_b_(0), field_30BC_(622),
        field_30C0_(211), owner_window_(0), field_30B4_(0), field_30B8_(0) {
        dib_.bmiHeader.biWidth = 622;
        dib_.bmiHeader.biHeight = -211;
        client_rect_.left = 20; client_rect_.top = 30;
        client_rect_.right = 820; client_rect_.bottom = 500;
    }

    void position() {
/* ACTUAL_BASEPOP_POSITION_BLOCK */
    }
};

static void expect(BasePop &popup, GraphicWin *owner, uint32_t flag,
                   uint32_t x, uint32_t y, int expected_x, int expected_y) {
    popup.owner_window_ = owner;
    popup.iSomeFlag_ = flag;
    popup.loc_a_ = x;
    popup.loc_b_ = y;
    popup.field_30BC_ = 622;
    popup.field_30C0_ = 211;
    popup.position();
    if (popup.field_30B4_ != expected_x || popup.field_30B8_ != expected_y)
        fprintf(stderr, "position (%08x,%08x) flag=%u got (%d,%d), expected (%d,%d)\\n",
                x, y, flag, popup.field_30B4_, popup.field_30B8_, expected_x, expected_y);
    assert(popup.field_30B4_ == expected_x);
    assert(popup.field_30B8_ == expected_y);
}

int main() {
    BasePop self;
    // Normal self-owned DIB path. Negative DIB height is intentional: preserve
    // its sign in the sentinel and negative-y formulas.
    expect(self, &self, 0, 0x1000, 0x1000, 201, 278);
    expect(self, &self, 0, static_cast<uint32_t>(-5), static_cast<uint32_t>(-1), 397, 556);
    expect(self, &self, 0, 0, 123, 0, 123);

    // The original uses arithmetic SAR for all six centered arms. These
    // odd-negative intermediates distinguish floor-style SAR from C++98's
    // truncation-toward-zero signed division.
    self.dib_.bmiHeader.biWidth = 1025;
    self.dib_.bmiHeader.biHeight = -769;
    expect(self, &self, 0, 0x1000, 0x1000, -1, -1);

    // Special client-rectangle self-owner arm remains distinct and unchanged.
    expect(self, &self, 2, 0x1000, 0x1000, 112, 149);
    expect(self, &self, 2, static_cast<uint32_t>(-5), static_cast<uint32_t>(-1), 219, 297);
    expect(self, &self, 2, 0, 123, 0, 123);
    self.client_rect_.right = 1045;
    self.client_rect_.bottom = 799;
    expect(self, &self, 2, 0x1000, 0x1000, -1, -1);

    // An external owner uses the owner's DIB and popup dimensions, not the
    // self-owned screen/DIB formulas above.
    GraphicWin external;
    external.dib_.bmiHeader.biWidth = 800;
    external.dib_.bmiHeader.biHeight = 600;
    expect(self, &external, 0, 0x1000, 0x1000, 89, -406);
    expect(self, &external, 0, static_cast<uint32_t>(-5), static_cast<uint32_t>(-1), 173, -812);
    expect(self, &external, 0, 0, 123, 0, 123);
    external.dib_.bmiHeader.biWidth = 621;
    external.dib_.bmiHeader.biHeight = -210;
    expect(self, &external, 0, 0x1000, 0x1000, -1, -1);
}
'''


def _position_block() -> str:
    source = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    body = _function_body(source, "int BasePop::exec(int flags,")
    marker = body.index(POSITION_MARKER)
    start = body.index("if (owner_window_ == this)", marker)
    end = body.index(ON_REDRAW, start)
    block = body[start:end].rstrip()
    assert block.count("owner_window_ == this") == 1
    return block


def _compilers() -> list[str]:
    result = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
              if compiler]
    if not result:
        pytest.skip("host C++ compiler unavailable")
    return result


def _compile_run(tmp_path: Path, block: str, compiler: str, should_pass: bool) -> None:
    source = HARNESS.replace(BLOCK_MARKER, block)
    cpp = tmp_path / "popup_position.cpp"
    exe = tmp_path / "popup_position"
    cpp.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=40,
    )
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert run.returncode == 0, run.stderr
    else:
        assert run.returncode != 0, "behavioral damage control unexpectedly passed"


@pytest.mark.parametrize("compiler", _compilers())
def test_actual_self_and_external_owner_position_branches(
    tmp_path: Path, compiler: str,
) -> None:
    _compile_run(tmp_path, _position_block(), compiler, should_pass=True)


def _replace_once(source: str, old: str, new: str) -> str:
    assert source.count(old) == 1, (old, source.count(old))
    return source.replace(old, new, 1)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("damage", (
    "extra-half", "zero-takes-negative-arm", "sar-to-division",
))
def test_compiling_position_damage_controls_fail(
    tmp_path: Path, compiler: str, damage: str,
) -> None:
    block = _position_block()
    if damage == "extra-half":
        old = (
            "field_30B4_ = static_cast<int>(loc_a_) -\n"
            "                               static_cast<int>(dib_.bmiHeader.biWidth) +\n"
            "                               WinScreenWidth;"
        )
        new = (
            "field_30B4_ = (static_cast<int>(loc_a_) -\n"
            "                               static_cast<int>(dib_.bmiHeader.biWidth) +\n"
            "                               WinScreenWidth) / 2;"
        )
        block = _replace_once(block, old, new)
    elif damage == "zero-takes-negative-arm":
        old = "static_cast<int>(loc_a_) < 0"
        assert block.count(old) == 3
        # Change only the self-owned normal-DIB signed guard; the special
        # client and external-owner branches remain untouched in this mutant.
        normal_arm = block.index("if (loc_a_ == 0x1000)", block.index("} else {"))
        index = block.index(old, normal_arm)
        block = block[:index] + "static_cast<int>(loc_a_) <= 0" + block[index + len(old):]
    else:
        block, substitutions = re.subn(r">>\s*1", "/ 2", block)
        assert substitutions == 6
    _compile_run(tmp_path, block, compiler, should_pass=False)


def _decode_original() -> dict[int, object]:
    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else []) + [
        ROOT / ".opensmacx/game/terranx_original.exe",
        ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
    ]
    image_path = next((candidate for candidate in candidates if candidate.is_file()), candidates[0])
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded: dict[int, object] = {}
    cursor = EXEC_SPAN.start
    for instruction in decoder.disasm(image.read(EXEC_SPAN), EXEC_SPAN.start):
        assert instruction.address == cursor, f"undecoded original byte at {cursor:08X}"
        decoded[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == EXEC_SPAN.end
    return decoded


def test_pinned_original_owner_flag_sentinel_and_negative_x_sequence() -> None:
    instructions = _decode_original()
    expected = {
        0x0060295A: ("mov", "ecx, dword ptr [esi + 0xa3c]"),
        0x00602960: ("cmp", "ecx, esi"),
        0x00602962: ("jne", "0x602ab9"),
        0x00602998: ("sar", "edx, 1"),
        0x006029F4: ("sar", "ecx, 1"),
        0x00602968: ("test", "byte ptr [ecx + 0x9c], 2"),
        0x00602975: ("je", "0x602a31"),
        0x00602A31: ("cmp", "eax, 0x1000"),
        0x00602A36: ("jne", "0x602a49"),
        0x00602A38: ("mov", "ecx, dword ptr [ecx + 0x4c4]"),
        0x00602A3E: ("mov", "eax, dword ptr [0x9b7b1c]"),
        0x00602A43: ("sub", "eax, ecx"),
        0x00602A45: ("sar", "eax, 1"),
        0x00602A84: ("sar", "edx, 1"),
        0x00602A49: ("test", "eax, eax"),
        0x00602A4B: ("jge", "0x602a5d"),
        0x00602A4D: ("mov", "edx, dword ptr [ecx + 0x4c4]"),
        0x00602A53: ("mov", "ecx, dword ptr [0x9b7b1c]"),
        0x00602A59: ("sub", "eax, edx"),
        0x00602A5B: ("add", "eax, ecx"),
        0x00602A5D: ("mov", "dword ptr [esi + 0x30b4], eax"),
        0x00602AD4: ("sar", "ecx, 1"),
        0x00602B21: ("sar", "ecx, 1"),
    }
    for address, expected_instruction in expected.items():
        actual = instructions[address]
        assert (actual.mnemonic, actual.op_str) == expected_instruction
    negative_arm = [instructions[address].mnemonic for address in (
        0x00602A49, 0x00602A4B, 0x00602A4D, 0x00602A53,
        0x00602A59, 0x00602A5B, 0x00602A5D,
    )]
    assert negative_arm == ["test", "jge", "mov", "mov", "sub", "add", "mov"]
    assert not any(instruction.mnemonic == "sar" for address, instruction in instructions.items()
                   if 0x00602A49 <= address < 0x00602A5D)
