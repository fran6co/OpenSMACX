"""Installed-source typed-owner controls; host models do not prove native ABI."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source.asm import Listing


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/diplopop.cpp"
IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"



def extract_definition(text: str, signature: str) -> str:
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


def test_pinned_calls_and_actual_source_candidate() -> None:
    source = SOURCE.read_text()
    records = {record.address: record for record in read_file(SOURCE)}
    assert records[0x0043F170].image_spans == ((0x0043F170, 0x0043F185),)
    assert records[0x0043F170].byte_exact
    assert records[0x004406D0].image_spans == ((0x004406D0, 0x004406EC),)
    assert records[0x004406D0].byte_exact
    # This historical FILE span includes padding and the next prologue; do not
    # silently turn its current FILE state into a byte-exact claim.
    assert records[0x004406F0].image_spans == ((0x004406F0, 0x00440707),)
    assert not records[0x004406F0].byte_exact

    owner = (ROOT / "src/diplopop.h").read_text()
    graphic = (ROOT / "src/graphicwin.h").read_text()
    win = (ROOT / "src/win.h").read_text()
    assert "class DiploPop : public GraphicWin, public SubInterface" in owner
    assert "RECOVERY_BASE: v1 DiploPop SubInterface 0xA14 0x0043EFF0" in owner
    assert "class GraphicWin : public Win, public Buffer" in graphic
    assert "virtual void hide();  // slot 2  0x005EDCD0" in win
    assert "virtual int set_modal(int flag, int (*poll)(), Win *owner);  // slot 57  0x005EE190" in win
    assert "virtual void release_modal();  // slot 58  0x005EE280" in win

    pin_lines = [line.split() for line in (ROOT / "src/IMAGE_PIN").read_text().splitlines()
                 if line.strip().startswith("sha256 ")]
    assert len(pin_lines) == 1 and len(pin_lines[0]) == 2
    pe = PeImage(IMAGE.read_bytes(), pin_lines[0][1])
    pinned = {
        0x0043F170: Listing(pe.read(Span(0x0043F170, 0x0043F185)), 0x0043F170),
        0x004406D0: Listing(pe.read(Span(0x004406D0, 0x004406EC)), 0x004406D0),
        0x004406F0: Listing(pe.read(Span(0x004406F0, 0x00440707)), 0x004406F0),
    }
    by_address = {address: {instruction.address: instruction
                            for instruction in listing.instructions}
                  for address, listing in pinned.items()}
    # The originals use thiscall ECX as the primary Win receiver; neither the
    # calls nor the tail-jump apply a secondary-base adjustment.
    hide = by_address[0x0043F170]
    assert hide[0x0043F171].mnemonic == "mov" and hide[0x0043F171].op_str == "esi, ecx"
    assert hide[0x0043F173].mnemonic == "call" and hide[0x0043F173].op_str == "0x5f7e90"
    assert hide[0x0043F17C].op_str == "ecx, esi"
    assert hide[0x0043F17E].mnemonic == "call" and hide[0x0043F17E].op_str == "0x5edcd0"
    modal = by_address[0x004406D0]
    assert modal[0x004406DE].mnemonic == "or" and modal[0x004406DE].op_str == "edx, 1"
    assert modal[0x004406E3].mnemonic == "call" and modal[0x004406E3].op_str == "0x5ee190"
    assert modal[0x004406E9].mnemonic == "ret" and bytes(modal[0x004406E9].bytes) == b"\xc2\x0c\x00"
    release = by_address[0x004406F0]
    assert release[0x004406F0].mnemonic == "jmp" and release[0x004406F0].op_str == "0x5ee280"

    candidate = source
    assert "0x0043F170 ?hide@DiploPop@@QAEXXZ 0x0043F170-0x0043F185 BYTE_EXACT" in candidate
    assert "0x004406D0 ?set_modal@DiploPop@@QAEHHP6AHXZPAUWin@@@Z 0x004406D0-0x004406EC FILE BYTE_EXACT" in candidate
    assert "0x004406F0 ?release_modal@DiploPop@@QAEXXZ 0x004406F0-0x00440707 FILE" in candidate
    hide_body = extract_definition(candidate, "void DiploPop::hide()")
    modal_body = extract_definition(candidate, "int DiploPop::set_modal(")
    release_body = extract_definition(candidate, "void DiploPop::release_modal()")
    assert "reinterpret_cast" not in hide_body + modal_body + release_body

    compiler = shutil.which("c++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler available for the isolated control")
    harness = r"""
#include <cassert>
#define __cdecl
class Win {
public:
    int visible = 0;
    int base_hide_count = 0;
    int base_release_count = 0;
    int modal_flag = 0;
    int (*modal_poll)() = nullptr;
    Win *modal_owner = nullptr;
    virtual int is_visible() { return visible; }
    virtual void hide() { ++base_hide_count; }
    virtual int set_modal(int flag, int (*poll)(), Win *owner) {
        modal_flag = flag; modal_poll = poll; modal_owner = owner; return 23;
    }
    virtual void release_modal() { ++base_release_count; }
};
class GraphicWin : public Win {};
class SubInterface { public: virtual ~SubInterface() = default; };
class DiploPop : public GraphicWin, public SubInterface {
public:
    void hide();
    int set_modal(int flag, int (*poll)(), Win *owner);
    void release_modal();
};
static Win MainInterfaceGlobal;
""" + hide_body + "\n" + modal_body + "\n" + release_body + r"""
static int poll_callback() { return 7; }
int main() {
    DiploPop popup{};
    popup.visible = 1;
    popup.hide();
    assert(popup.base_hide_count == 1);
    popup.visible = 0;
    popup.hide();
    assert(popup.base_hide_count == 1);
    assert(popup.set_modal(4, poll_callback, nullptr) == 23);
    assert(popup.modal_flag == 5);
    assert(popup.modal_poll == poll_callback);
    assert(popup.modal_owner == &MainInterfaceGlobal);
    popup.release_modal();
    assert(popup.base_release_count == 1);
}
"""
    with tempfile.TemporaryDirectory(prefix="opensmacx-diplopop-control-") as directory:
        source_file = Path(directory) / "control.cpp"
        binary = Path(directory) / "control"
        source_file.write_text(harness)
        subprocess.run([compiler, "-std=c++11", "-Wno-unused-parameter",
                        str(source_file), "-o", str(binary)], check=True,
                       capture_output=True, text=True)
        subprocess.run([str(binary)], check=True, capture_output=True, text=True)
