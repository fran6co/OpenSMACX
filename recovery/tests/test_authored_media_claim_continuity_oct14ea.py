"""Four native promotions; implementations and recovery history unchanged."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "7a20029edfe4fb109e048bdcdce23f0e44cc0117"
FLIC_STRUCTURE_BASE = "a7ad5cb7066e77b2648b086cb7bdd0d0cf99245d"
SELECTED = {0x629E50, 0x637310, 0x63E7F0, 0x63B2D0}


@pytest.mark.parametrize("relative", ["src/flic.cpp", "src/video.cpp", "src/cursor.cpp"])
def test_native_publication_preserves_complete_source(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    before = read_text(old.decode(), path)
    candidate = path.read_text()
    candidate_projected = (_project_flic_cpp_to_structure_baseline(candidate)
                           if relative == "src/flic.cpp" else candidate)
    after = read_text(candidate_projected, path)
    promoted = [replace(row, byte_exact=True, semantic=False)
                for row in before if row.address in SELECTED]
    assert promoted
    assert all(not row.byte_exact for row in before if row.address in SELECTED)
    assert write_bytes(old, promoted) == candidate_projected.encode()
    assert len(before) == len(after)
    for original, recorded in zip(before, after):
        expected = (replace(original, byte_exact=True, semantic=False)
                    if original.address in SELECTED else original)
        for field in fields(original):
            if field.name != "region":
                assert getattr(expected, field.name) == getattr(recorded, field.name)

def _replace_once(source, before, after):
    assert source.count(before) == 1, f"expected one structural fragment: {before!r}"
    return source.replace(before, after, 1)


def _project_flic_cpp_to_structure_baseline(source):
    replacements = (
        ("decode_chunk(&chunk_.header_);", "decode_chunk(&chunk_);", 2),
        ("fseek(file_, chunk_.header_.size_, SEEK_CUR);",
         "fseek(file_, chunk_.size_, SEEK_CUR);", 1),
        ("FlicFrameChunk *chunks = &chunk_;", "DataChunk *chunks = &chunk_;", 1),
        ("chunk_.header_.size_ + frame_pos", "chunk_.size_ + frame_pos", 1),
        ("chunk_.header_.count_", "chunk_.count_", 1),
        ("decode_chunk(&chunks->header_);", "decode_chunk(chunks);", 1),
        ("FlicFrameChunk sub;", "DataChunk sub;", 1),
        ("sub.header_.type_", "sub.type_", 2),
        ("""            DataChunk sub;
            fread(&sub, 6, 1, file_);
            decode_chunk(&sub);""",
         """            struct {
                uint32_t size_;
                uint16_t type_;
                uint16_t count_;
            } sub;
            fread(&sub, 6, 1, file_);
            decode_chunk(reinterpret_cast<DataChunk *>(&sub));""", 1),
    )
    for before, after, count in replacements:
        assert source.count(before) == count, f"unexpected source occurrences: {before!r}"
        source = source.replace(before, after, count)
    return source


def _project_flic_header_to_structure_baseline(source):
    current_docs = """ * DataChunk is the 8-byte decoder prefix: a 4-byte size that counts the 6
 * on-disk header bytes themselves, a 2-byte type, and the word at +6 that
 * a type-0xF1FA frame ring uses as its subchunk count. The recursive decoder
 * also receives a six-byte subchunk read in an eight-byte stack slot; on
 * that path count_ is deliberately left untouched by fread."""
    old_docs = """ * DataChunk is a chunk header: a 4-byte size that counts the 6 header
 * bytes themselves, a 2-byte type, and the 2-byte subchunk count a frame
 * ring (type 0xF1FA) carries. The struct is 16 bytes rather than 6
 * because frame headers are fetched 0x10 at a time (0x0062A070,
 * 0x0062A2C0); the tail is slack those fetches fill and nothing reads."""
    source = _replace_once(source, current_docs, old_docs)
    current_types = """struct DataChunk {
    uint32_t size_;             // +0x00  chunk size including the 6-byte header
    uint16_t type_;             // +0x04  0xF1FA frame ring, 7 delta, 0xD black,
                                //       0xF byte run, 0x10 copy
    uint16_t count_;            // +0x06  subchunk count when type_ is 0xF1FA
};

static_assert(sizeof(DataChunk) == 0x8,
              "the recursive decoder's stack prefix occupies eight bytes");

// Frame headers are read as sixteen bytes. Keep their eight-byte decoder
// prefix distinct from the unexamined trailing bytes of that file read.
struct FlicFrameChunk {
    DataChunk header_;          // +0x00  fields consumed by decode_chunk
    uint8_t slack_[8];          // +0x08  slack the 0x10-byte frame fetches fill
};

static_assert(sizeof(FlicFrameChunk) == 0x10,
              "frame headers are fetched sixteen bytes at a time");"""
    old_types = """struct DataChunk {
    uint32_t size_;             // +0x00  chunk size including the 6-byte header
    uint16_t type_;             // +0x04  0xF1FA frame ring, 7 delta, 0xD black,
                                //       0xF byte run, 0x10 copy
    uint16_t count_;            // +0x06  subchunk count when type_ is 0xF1FA
    uint8_t slack_[8];          // +0x08  slack the 0x10-byte frame fetches fill
};

static_assert(sizeof(DataChunk) == 0x10,
              "frame headers are fetched sixteen bytes at a time");"""
    source = _replace_once(source, current_types, old_types)
    for current, old in (
        ("FlicFrameChunk chunk_;          // 0xAB4  frame bytes in flight",
         "DataChunk chunk_;          // 0xAB4  header of the frame chunk in flight"),
        ("FlicFrameChunk first_frame_;    // 0xAC4  saved first-frame bytes",
         "DataChunk first_frame_;    // 0xAC4  saved first-frame chunk header"),
        ("FlicFrameChunk second_frame_;   // 0xAD4  saved loop-point bytes",
         "DataChunk second_frame_;   // 0xAD4  saved loop-point chunk header"),
    ):
        source = _replace_once(source, current, old)
    return source


def test_flic_structural_projection_preserves_the_fixed_task_baseline():
    cpp_old = subprocess.check_output(
        ["git", "show", FLIC_STRUCTURE_BASE + ":src/flic.cpp"], cwd=ROOT)
    cpp_old = cpp_old.decode()
    cpp_projected = _project_flic_cpp_to_structure_baseline(
        (ROOT / "src/flic.cpp").read_text())
    assert cpp_projected == cpp_old

    header_old = subprocess.check_output(
        ["git", "show", FLIC_STRUCTURE_BASE + ":src/flic.h"], cwd=ROOT).decode()
    header_projected = _project_flic_header_to_structure_baseline(
        (ROOT / "src/flic.h").read_text())
    assert header_projected == header_old


@pytest.mark.parametrize(("relative", "mutation"), [
    ("src/flic.cpp", lambda s: s.replace("DataChunk sub;", "DataChunk damaged;", 1)),
    ("src/flic.cpp", lambda s: s.replace("decode_chunk(&chunks->header_);",
                                         "decode_chunk(chunks);", 1)),
    ("src/flic.cpp", lambda s: s.replace("// The scan head", "// altered scan head", 1)),
    ("src/flic.h", lambda s: s.replace("FlicFrameChunk second_frame_;",
                                        "DataChunk second_frame_;", 1)),
    ("src/flic.h", lambda s: s.replace("uint8_t slack_[8];",
                                        "uint8_t slack_[7];", 1)),
    ("src/flic.h", lambda s: s.replace(" * Flic class", " * changed Flic class", 1)),
])
def test_flic_projection_refuses_unadmitted_source_changes(relative, mutation):
    current = (ROOT / relative).read_text()
    damaged = mutation(current)
    assert damaged != current
    if relative.endswith(".cpp"):
        project = _project_flic_cpp_to_structure_baseline
        baseline = subprocess.check_output(
            ["git", "show", FLIC_STRUCTURE_BASE + ":src/flic.cpp"], cwd=ROOT).decode()
    else:
        project = _project_flic_header_to_structure_baseline
        baseline = subprocess.check_output(
            ["git", "show", FLIC_STRUCTURE_BASE + ":src/flic.h"], cwd=ROOT).decode()
    try:
        projected = project(damaged)
    except AssertionError:
        return
    assert projected != baseline
