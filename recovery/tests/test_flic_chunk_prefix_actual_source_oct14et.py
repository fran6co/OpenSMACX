"""Bounded source/layout controls for the recovered FLIC chunk prefix."""
from pathlib import Path
import re
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
    unkn = _braced_declaration(source, "int Flic::UNK3(")
    assert "FlicFrameChunk *chunks = &chunk_;" in unkn
    assert "fread(chunks, 0x10, 1, file_);" in unkn
    assert "chunk_.header_.size_ + frame_pos" in unkn
    assert "chunk_.header_.count_" in unkn
    assert "decode_chunk(&chunks->header_);" in unkn
    # The scan's declared object extent is not established by its six-byte
    # read. Retain the pre-migration sixteen-byte storage without initializing
    # it; only the recursive decoder's eight-byte local is independently known.
    assert "FlicFrameChunk sub;" in unkn
    assert "fread(&sub, 6, 1, file_);" in unkn
    assert "sub.header_.type_ == 7 || sub.header_.type_ == 0xc" in unkn
    assert "FlicFrameChunk sub{};" not in unkn
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
