"""Installed-source typed-owner controls; host models do not prove native ABI."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.layout_source import parse_members
from match_recovery.source import read_file
from match_recovery.source.asm import Listing


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/setupwin.cpp"
HEADER = ROOT / "src/setupwin.h"
GRAPHIC = ROOT / "src/graphicwin.h"
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


def test_typed_fields_preserve_pinned_signed_modular_arithmetic() -> None:
    cpp = SOURCE.read_text()
    header = HEADER.read_text()
    records = {record.address: record for record in read_file(SOURCE)}
    assert records[0x004ADA80].image_spans == ((0x004ADA80, 0x004ADA9D),)
    assert records[0x004ADAA0].image_spans == ((0x004ADAA0, 0x004ADAC4),)
    assert records[0x004ADA80].byte_exact and records[0x004ADAA0].byte_exact

    graphic = GRAPHIC.read_text()
    assert "static_assert(sizeof(GraphicWin) == 0xA14," in graphic
    assert "class SetupWin : public GraphicWin" in header
    assert "uint32_t field_A14_;  // 0xA14" in header
    assert "uint32_t field_A18_;  // 0xA18" in header
    assert header.index("uint32_t field_A14_;  // 0xA14") < header.index("uint32_t field_A18_;  // 0xA18")
    candidate_header = header
    assert "RECOVERY_MEMBER: v1 SetupWin field_A14_ 0xA14 0x004ADA80" in candidate_header
    assert "RECOVERY_MEMBER: v1 SetupWin field_A18_ 0xA18 0x004ADAA0" in candidate_header
    pin_lines = [line.split() for line in (ROOT / "src/IMAGE_PIN").read_text().splitlines()
                 if line.strip().startswith("sha256 ")]
    assert len(pin_lines) == 1 and len(pin_lines[0]) == 2
    members = parse_members(candidate_header, "src/setupwin.h", pin_lines[0][1],
                             address_space="pe:va")
    selected = {(member.member, member.offset, member.evidence.address)
                for member in members if member.class_name == "SetupWin"}
    assert ("field_A14_", 0xA14, 0x004ADA80) in selected
    assert ("field_A18_", 0xA18, 0x004ADAA0) in selected

    pe = PeImage(IMAGE.read_bytes(), pin_lines[0][1])
    reads = {
        0x004ADA80: Listing(pe.read(Span(0x004ADA80, 0x004ADA9D)), 0x004ADA80),
        0x004ADAA0: Listing(pe.read(Span(0x004ADAA0, 0x004ADAC4)), 0x004ADAA0),
    }
    first = {insn.address: insn for insn in reads[0x004ADA80].instructions}
    second = {insn.address: insn for insn in reads[0x004ADAA0].instructions}
    assert first[0x004ADA83].op_str == "eax, dword ptr [ecx + 0xa14]"
    assert second[0x004ADAA3].op_str == "ecx, dword ptr [ecx + 0xa18]"
    assert first[0x004ADA9A].mnemonic == "ret" and bytes(first[0x004ADA9A].bytes) == b"\xc2\x04\x00"
    assert second[0x004ADAC1].mnemonic == "ret" and bytes(second[0x004ADAC1].bytes) == b"\xc2\x04\x00"

    candidate = cpp
    assert "0x004ADA80 ?UNK1@SetupWin@@QAEHH@Z 0x004ADA80-0x004ADA9D BYTE_EXACT" in candidate
    assert "0x004ADAA0 ?UNK2@SetupWin@@QAEHH@Z 0x004ADAA0-0x004ADAC4 BYTE_EXACT" in candidate
    method_1 = extract_definition(candidate, "int SetupWin::UNK1(int pos)")
    method_2 = extract_definition(candidate, "int SetupWin::UNK2(int pos)")
    assert "reinterpret_cast" not in method_1 + method_2

    compiler = shutil.which("c++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler available for the isolated control")
    harness = r"""
#include <cassert>
#include <cstdint>
class SetupWin {
public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    int UNK1(int pos);
    int UNK2(int pos);
};
""" + method_1 + "\n" + method_2 + r"""
static int32_t expected(uint32_t scale_bits, int pos, int divisor) {
    const uint32_t product_bits = scale_bits * static_cast<uint32_t>(pos);
    return static_cast<int32_t>(product_bits) / divisor;
}
int main() {
    const uint32_t scales[] = {0U, 1U, 0xffffffffU, 0x80000000U,
                               0x7fffffffU, 0xa0000000U, 0x12345678U};
    const int positions[] = {0, 1, -1, 1024, -1024,
                             (-2147483647 - 1), 0x40000001, 0x7fffffff};
    SetupWin object{};
    for (uint32_t scale : scales) {
        object.field_A14_ = scale;
        object.field_A18_ = scale;
        for (int pos : positions) {
            assert(object.UNK1(pos) == expected(scale, pos, 1024));
            assert(object.UNK2(pos) == expected(scale, pos, 768));
        }
    }
}
"""
    with tempfile.TemporaryDirectory(prefix="opensmacx-setupwin-control-") as directory:
        source = Path(directory) / "control.cpp"
        binary = Path(directory) / "control"
        source.write_text(harness)
        subprocess.run([compiler, "-std=c++11", "-Wno-unused-parameter",
                        str(source), "-o", str(binary)], check=True,
                       capture_output=True, text=True)
        subprocess.run([str(binary)], check=True, capture_output=True, text=True)
