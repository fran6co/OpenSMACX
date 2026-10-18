"""Actual-source controls for BasePop's shared button-bevel copy.

The host fixture checks bounded C++ value and store behavior. It does not
establish VC6 ABI, palette/rendering behavior, or runtime behavior in-game.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXEC_SPAN = Span(0x00602600, 0x006037F5)
HELPER_SIG = "void apply_button_bevel(FlatButton *button)"
EXEC_SIG = "int BasePop::exec(int flags,"
HELPER_MARKER = "/* ACTUAL_BASEPOP_BEVEL_HELPER */"
CALLS_MARKER = "/* ACTUAL_BASEPOP_BEVEL_CALLS */"


HARNESS = r'''#include <assert.h>
#include <stdint.h>

static int writes[18];
static int write_count = 0;

struct ObservedWord {
    uint32_t value;
    int slot;
    ObservedWord(uint32_t initial, int id) : value(initial), slot(id) {}
    ObservedWord &operator=(uint32_t next) {
        value = next;
        writes[write_count++] = slot;
        return *this;
    }
};

struct BaseButton {
    ObservedWord color_;
    ObservedWord bevel_upper_;
    ObservedWord bevel_lower_;
    ObservedWord bevel_thickness_;
    ObservedWord field_A94_;
    ObservedWord field_A98_;
    BaseButton() : color_(0, 0), bevel_upper_(0, 1), bevel_lower_(0, 2),
        bevel_thickness_(0, 3), field_A94_(0, 4), field_A98_(0, 5) {}
};

struct FlatButton : BaseButton {};

struct BasePop {
    uint8_t field_31A0_;
    uint32_t field_31A4_;
    uint32_t field_31A8_;
    uint32_t field_31AC_;
    uint32_t field_31B0_;
    uint32_t field_31B4_;
    FlatButton flat_button1_;
    FlatButton flat_button2_;

    BasePop() : field_31A0_(0), field_31A4_(0x11111111),
        field_31A8_(0x22222222), field_31AC_(0x33333333),
        field_31B0_(0x44444444), field_31B4_(0x55555555) {}

/* ACTUAL_BASEPOP_BEVEL_HELPER */

    void actual_call_shapes(FlatButton *button) {
/* ACTUAL_BASEPOP_BEVEL_CALLS */
    }
};

static void check(uint8_t byte_value) {
    BasePop popup;
    FlatButton item_button;
    popup.field_31A0_ = byte_value;
    popup.flat_button1_.color_.value = 0xDEAD0001;
    item_button.color_.value = 0xDEAD0002;
    popup.flat_button2_.color_.value = 0xDEAD0003;
    write_count = 0;
    popup.actual_call_shapes(&item_button);

    assert(write_count == 18);
    for (int index = 0; index < write_count; ++index)
        assert(writes[index] == index % 6);

    const uint32_t expected[] = {
        static_cast<uint32_t>(byte_value), 0x11111111, 0x22222222,
        0x33333333, 0x44444444, 0x55555555
    };
    const ObservedWord *destinations[] = {
        &popup.flat_button1_.color_, &popup.flat_button1_.bevel_upper_,
        &popup.flat_button1_.bevel_lower_, &popup.flat_button1_.bevel_thickness_,
        &popup.flat_button1_.field_A94_, &popup.flat_button1_.field_A98_,
        &item_button.color_, &item_button.bevel_upper_, &item_button.bevel_lower_,
        &item_button.bevel_thickness_, &item_button.field_A94_, &item_button.field_A98_,
        &popup.flat_button2_.color_, &popup.flat_button2_.bevel_upper_,
        &popup.flat_button2_.bevel_lower_, &popup.flat_button2_.bevel_thickness_,
        &popup.flat_button2_.field_A94_, &popup.flat_button2_.field_A98_
    };
    for (int index = 0; index < 18; ++index)
        assert(destinations[index]->value == expected[index % 6]);
}

int main() {
    const uint8_t values[] = {0, 9, 127, 128, 247, 255};
    for (int index = 0; index < 6; ++index)
        check(values[index]);
}
'''


def _closing_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif ((state == "string" and char == '"')
                  or (state == "char" and char == "'")):
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line"
            index += 1
        elif char == "/" and next_char == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    raise AssertionError("unterminated C++ definition")


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    return text[start:_closing_brace(text, opening) + 1]


def _actual_helper() -> str:
    header = (ROOT / "src/basepop.h").read_text(encoding="utf-8")
    helper = _definition(header, HELPER_SIG)
    assert helper.count("button->") == 6
    return helper


def _actual_call_shapes() -> list[str]:
    source = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    body = _definition(source, EXEC_SIG)
    calls = [
        "apply_button_bevel(&flat_button1_);",
        "apply_button_bevel(button);",
        "apply_button_bevel(&flat_button2_);",
    ]
    positions = []
    for call in calls:
        assert body.count(call) == 1, (call, body.count(call))
        positions.append(body.index(call))
    assert positions == sorted(positions)
    return calls


def _fixture(helper: str, calls: list[str]) -> str:
    source = HARNESS.replace(HELPER_MARKER, helper)
    return source.replace(CALLS_MARKER, "\n".join("        " + call for call in calls))


def _compilers() -> list[str]:
    result = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
              if compiler]
    if not result:
        pytest.skip("host C++ compiler unavailable")
    return result


def _compile_run(tmp_path: Path, source: str, compiler: str, should_pass: bool) -> None:
    cpp = tmp_path / "basepop_bevel.cpp"
    exe = tmp_path / "basepop_bevel"
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
def test_actual_helper_and_three_call_shapes_zero_extend_source_byte_and_keep_order(
    tmp_path: Path, compiler: str,
) -> None:
    calls = _actual_call_shapes()
    _compile_run(tmp_path, _fixture(_actual_helper(), calls), compiler, should_pass=True)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("damage", ("zero", "default-247", "sign-extend"))
def test_compiling_wrong_color_provider_mutants_fail(
    tmp_path: Path, compiler: str, damage: str,
) -> None:
    helper = _actual_helper()
    old = "button->color_ = field_31A0_;"
    assert helper.count(old) == 1
    if damage == "zero":
        replacement = "button->color_ = 0;"
    elif damage == "default-247":
        replacement = "button->color_ = 247;"
    else:
        replacement = (
            "button->color_ = static_cast<uint32_t>("
            "static_cast<int8_t>(field_31A0_));"
        )
    helper = helper.replace(old, replacement, 1)
    _compile_run(tmp_path, _fixture(helper, _actual_call_shapes()), compiler,
                 should_pass=False)


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


def test_pinned_all_three_bevel_sites_read_popup_byte_then_store_color_word() -> None:
    instructions = _decode_original()
    expected = {
        0x00602F4A: ("xor", "eax, eax"),
        0x00602F4E: ("mov", "al, byte ptr [esi + 0x31a0]"),
        0x00602F56: ("mov", "dword ptr [edi + 0xa84], eax"),
        0x006032D7: ("xor", "eax, eax"),
        0x006032D9: ("mov", "al, byte ptr [esi + 0x31a0]"),
        0x006032E1: ("mov", "dword ptr [ecx + 0xa84], eax"),
        0x00603627: ("xor", "eax, eax"),
        0x0060362B: ("mov", "al, byte ptr [esi + 0x31a0]"),
        0x00603631: ("mov", "dword ptr [esi + 0x202c], eax"),
    }
    for address, wanted in expected.items():
        actual = instructions[address]
        assert (actual.mnemonic, actual.op_str) == wanted

    # Confirm the five adjacent original values are loaded from this popup
    # and stored into their corresponding BaseButton slots at each site.
    adjacent = {
        0x00602F5C: ("mov", "eax, dword ptr [esi + 0x31a4]"),
        0x00602F62: ("mov", "dword ptr [edi + 0xa88], eax"),
        0x00602F68: ("mov", "eax, dword ptr [esi + 0x31a8]"),
        0x00602F6E: ("mov", "dword ptr [edi + 0xa8c], eax"),
        0x00602F74: ("mov", "eax, dword ptr [esi + 0x31ac]"),
        0x00602F7A: ("mov", "dword ptr [edi + 0xa90], eax"),
        0x00602F80: ("mov", "eax, dword ptr [esi + 0x31b0]"),
        0x00602F86: ("mov", "dword ptr [edi + 0xa94], eax"),
        0x00602F8C: ("mov", "eax, dword ptr [esi + 0x31b4]"),
        0x00602F92: ("mov", "dword ptr [edi + 0xa98], eax"),
        0x006032E9: ("mov", "eax, dword ptr [esi + 0x31a4]"),
        0x006032EF: ("mov", "dword ptr [edx + 0xa88], eax"),
        0x006032F7: ("mov", "edx, dword ptr [esi + 0x31a8]"),
        0x006032FD: ("mov", "dword ptr [ecx + 0xa8c], edx"),
        0x00603305: ("mov", "ecx, dword ptr [esi + 0x31ac]"),
        0x0060330B: ("mov", "dword ptr [eax + 0xa90], ecx"),
        0x00603313: ("mov", "eax, dword ptr [esi + 0x31b0]"),
        0x00603319: ("mov", "dword ptr [edx + 0xa94], eax"),
        0x00603321: ("mov", "edx, dword ptr [esi + 0x31b4]"),
        0x00603327: ("mov", "dword ptr [ecx + 0xa98], edx"),
        0x00603637: ("mov", "ecx, dword ptr [esi + 0x31a4]"),
        0x0060363D: ("mov", "dword ptr [esi + 0x2030], ecx"),
        0x00603643: ("mov", "edx, dword ptr [esi + 0x31a8]"),
        0x00603649: ("mov", "dword ptr [esi + 0x2034], edx"),
        0x0060364F: ("mov", "eax, dword ptr [esi + 0x31ac]"),
        0x00603655: ("mov", "dword ptr [esi + 0x2038], eax"),
        0x0060365B: ("mov", "ecx, dword ptr [esi + 0x31b0]"),
        0x00603661: ("mov", "dword ptr [esi + 0x203c], ecx"),
        0x00603667: ("mov", "edx, dword ptr [esi + 0x31b4]"),
        0x0060366D: ("mov", "dword ptr [esi + 0x2040], edx"),
    }
    for address, wanted in adjacent.items():
        actual = instructions[address]
        assert (actual.mnemonic, actual.op_str) == wanted
