"""Bounded source/layout controls for the recovered FLIC chunk prefix."""
from pathlib import Path
import re
import os
from hashlib import sha256
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "CMakeLists.txt").is_file())
HEADER = ROOT / "src/flic.h"
SOURCE = ROOT / "src/flic.cpp"


def _braced_declaration(source, marker):
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    start = masked.find(marker)
    assert start >= 0, f"missing actual source declaration: {marker}"
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated declaration: {marker}")


def _actual_schema_fragment():
    header = HEADER.read_text()
    data = _braced_declaration(header, "struct DataChunk {")
    frame = _braced_declaration(header, "struct FlicFrameChunk {")
    return "\n".join((data + ";", frame + ";"))


def _compile_layout(fragment, compiler):
    program = """#include <cstddef>
#include <cstdint>
""" + fragment + """
static_assert(sizeof(DataChunk) == 8, "DataChunk extent");
static_assert(offsetof(DataChunk, size_) == 0, "size offset");
static_assert(offsetof(DataChunk, type_) == 4, "type offset");
static_assert(offsetof(DataChunk, count_) == 6, "count offset");
static_assert(sizeof(FlicFrameChunk) == 16, "frame extent");
static_assert(offsetof(FlicFrameChunk, header_) == 0, "header offset");
static_assert(offsetof(FlicFrameChunk, slack_) == 8, "slack offset");
int main() { return 0; }
"""
    return subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-fsyntax-only", "-"],
        input=program, text=True, capture_output=True)


def test_current_actual_prefix_and_frame_declarations_compile():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("host GCC/Clang unavailable")
    fragment = _actual_schema_fragment()
    assert "uint32_t size_;" in fragment
    assert "uint16_t type_;" in fragment
    assert "uint16_t count_;" in fragment
    assert "DataChunk header_;" in fragment
    assert "uint8_t slack_[8];" in fragment
    for compiler in compilers:
        result = _compile_layout(fragment, compiler)
        assert result.returncode == 0, result.stderr


@pytest.mark.parametrize(("old", "new", "diagnostic"), [
    ("uint16_t count_;", "uint32_t count_;", "DataChunk extent"),
    ("uint8_t slack_[8];", "uint8_t slack_[9];", "frame extent"),
])
def test_wrong_actual_field_extent_fails_the_intended_layout_assertion(
        old, new, diagnostic):
    fragment = _actual_schema_fragment()
    assert fragment.count(old) == 1
    mutated = fragment.replace(old, new, 1)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host GCC/Clang unavailable")
    result = _compile_layout(mutated, compiler)
    assert result.returncode != 0
    assert diagnostic in result.stderr


def test_frame_owners_and_decoder_use_actual_named_prefix_members():
    header = HEADER.read_text()
    for declaration in (
        "FlicFrameChunk chunk_;          // 0xAB4",
        "FlicFrameChunk first_frame_;    // 0xAC4",
        "FlicFrameChunk second_frame_;   // 0xAD4",
    ):
        assert declaration in header

    source = SOURCE.read_text()
    assert source.count("decode_chunk(&chunk_.header_);") == 2
    assert "fseek(file_, chunk_.header_.size_, SEEK_CUR);" in source
    unkn = _braced_declaration(source, "int Flic::advance_to_frame(")
    assert "FlicFrameChunk *chunks = &chunk_;" in unkn
    assert "fread(chunks, 0x10, 1, file_);" in unkn
    assert "chunks->header_.size_ + frame_pos" in unkn
    assert "chunks->header_.count_" in unkn
    assert "decode_chunk(&chunks->header_);" in unkn
    # Full original prologue/scan/cleanup evidence below bounds this scan
    # storage to eight bytes before the return address. The six-byte read
    # alone would not establish that extent. Use the existing prefix type;
    # leave its unread count tail and frame_pos uninitialized.
    assert "DataChunk sub;" in unkn
    assert "fread(&sub, 6, 1, file_);" in unkn
    assert "sub.type_ == 7 || sub.type_ == 0xc" in unkn
    assert "FlicFrameChunk sub{};" not in unkn
    assert "long frame_pos;" in unkn
    assert "long frame_pos =" not in unkn
    assert "sub.count_" not in unkn
    assert "DataChunk sub{};" not in unkn

    decoder = _braced_declaration(source, "void Flic::decode_chunk(")
    assert "DataChunk sub;" in decoder
    assert "fread(&sub, 6, 1, file_);" in decoder
    assert "decode_chunk(&sub);" in decoder
    assert not re.search(r"\b(?:memset|zero|initialize)\s*\([^;]*\bsub\b",
                         decoder)


# These are declaration and selected-source-operation controls, not execution
# of the proprietary decoder, complete grammar proof, native ABI validation,
# or semantic assignments to untouched/trailing bytes.


def test_pinned_scan_storage_uses_the_complete_original_stack_frame():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_ESP
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == pin
    image = PeImage(raw, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(Span(0x62A2C0, 0x62A4FA)), 0x62A2C0))
    by_address = {instruction.address: instruction for instruction in instructions}
    allocation = by_address[0x62A2C4]
    assert allocation.mnemonic == "sub" and allocation.operands[0].reg == X86_REG_ESP
    assert allocation.operands[1].imm == 0x18
    assert [by_address[address].mnemonic for address in
            (0x62A2C8, 0x62A2C9, 0x62A2CA, 0x62A2CF)] == ["push"] * 4
    # Track every explicit stack adjustment up to the scan's destination LEA;
    # all intervening original calls have balanced caller cleanup here.
    delta = 0
    for instruction in instructions:
        if instruction.address == 0x62A387:
            break
        if instruction.mnemonic == "push": delta -= 4
        elif instruction.mnemonic == "pop": delta += 4
        elif instruction.mnemonic in ("sub", "add"):
            left, right = instruction.operands
            if left.type == X86_OP_REG and left.reg == X86_REG_ESP:
                assert right.type == X86_OP_IMM
                delta += right.imm * (-1 if instruction.mnemonic == "sub" else 1)
    assert delta == -0x28
    scan = by_address[0x62A387]
    assert scan.mnemonic == "lea"
    memory = scan.operands[1]
    assert memory.type == X86_OP_MEM and memory.mem.base == X86_REG_ESP
    assert memory.mem.disp == 0x20 and memory.mem.index == 0
    assert -(delta + memory.mem.disp) == 8  # slot ends before the return address
    count = by_address[0x62A38E]
    assert count.mnemonic == "push" and count.operands[0].imm == 6
    call = by_address[0x62A391]
    assert call.mnemonic == "call" and call.operands[0].imm == 0x646178
    for cleanup, terminal in ((0x62A4D2, 0x62A4D5), (0x62A4F4, 0x62A4F7)):
        add, ret = by_address[cleanup], by_address[terminal]
        assert add.mnemonic == "add" and add.operands[0].reg == X86_REG_ESP
        assert add.operands[1].imm == 0x18
        assert ret.mnemonic == "ret" and ret.operands[0].imm == 4
