"""Actual packed shape walker; bounded controls, not complete renderer proof."""
from pathlib import Path
import os
import shutil
import subprocess
from collections import Counter

import pytest

from match_recovery.source import read, read_text
from test_caviar_draw_callbacks_actual_source import extract
from test_texture_raster_actual_source import _continuity_fields
from test_function_span_tails_oct11 import expected_extended_record
from test_verified_tail_claims_oct11 import expected_recorded_tail

ROOT = Path(__file__).resolve().parents[2]
BASE = "ba1ed66be9324bdccdf8c2b75db88be2dd1b50d9"


def test_original_identity_claim_and_symbol_continuity():
    path = ROOT / "src/caviar.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"],
                                  cwd=ROOT, text=True)
    before = read_text(old, path)
    assert_caviar_owner_continuity(before)


def assert_caviar_owner_continuity(before):
    """Retain historical Caviar identities across their current source owners."""
    from test_caviar_record_api_typing_oct12 import assert_api_symbol_transition
    old_addresses = {record.address for record in before}
    by_address = {}
    for record in read(ROOT / "src"):
        if record.address in old_addresses:
            by_address.setdefault(record.address, []).append(record)
    assert set(by_address) == old_addresses
    assert all(len(records) == 1 for records in by_address.values())

    def lessons(row):
        return Counter((item.kind, item.key, item.prose,
                        tuple(text for _, text in item.lines))
                       for item in (row.origin or ()))

    def retained_history(old, new):
        for field in ("levers", "ruled_out", "unrecoverable", "deferred"):
            before_rows = Counter(getattr(old, field))
            after_rows = Counter(getattr(new, field))
            if old.address == 0x616DA0 and field == "levers":
                old_rows = [row for row in before_rows if row[0] == "0"]
                new_rows = [row for row in after_rows if row[0] == "0"]
                assert len(old_rows) == len(new_rows) == 1
                old_correction, new_correction = old_rows[0], new_rows[0]
                assert "FOLDED into the implicit array construction" in old_correction[1]
                assert "VoxInstance" in new_correction[1]
                before_rows[old_correction] -= 1
                after_rows[new_correction] -= 1
                before_rows += Counter()
                after_rows += Counter()
            assert before_rows <= after_rows
        before_lessons, after_lessons = lessons(old), lessons(new)
        if old.address == 0x616DA0:
            old_rows = [row for row in before_lessons if row[:2] == ("lever", "0")]
            new_rows = [row for row in after_lessons if row[:2] == ("lever", "0")]
            assert len(old_rows) == len(new_rows) == 1
            old_correction, new_correction = old_rows[0], new_rows[0]
            assert "FOLDED into the implicit array construction" in old_correction[2]
            assert "VoxInstance" in new_correction[2]
            before_lessons[old_correction] -= 1
            after_lessons[new_correction] -= 1
            before_lessons += Counter()
            after_lessons += Counter()
        assert before_lessons <= after_lessons

    for record in before:
        record = expected_recorded_tail(expected_extended_record(record))
        # Preserve the separately validated complete RET 4 instruction spans.
        prior_extents = {
            0x618560: (((0x618560, 0x61859F),), ((0x618560, 0x6185A0),)),
            0x6183D0: (((0x6183D0, 0x618424),), ((0x6183D0, 0x618426),)),
        }
        if record.address in prior_extents:
            from dataclasses import replace
            previous, complete = prior_extents[record.address]
            assert record.image_spans in (previous, complete)
            record = replace(record, image_spans=complete)
        current = by_address[record.address][0]
        assert record.function_fragment == current.function_fragment
        if record.address == 0x63E860:
            from match_recovery.source.model import Mode
            assert record.mode is Mode.BODY and current.mode is Mode.FILE
            assert record.address == current.address
            assert record.name == current.name
            assert record.image_spans == current.image_spans
            assert record.kind == current.kind and record.exclusion == current.exclusion
            assert record.source_role == current.source_role
            assert record.source_role_reason == current.source_role_reason
            assert not record.byte_exact or current.byte_exact
            assert not record.semantic or current.semantic or current.byte_exact
            retained_history(record, current)
            assert current.symbol == "?vox_detect_cpu@@YAXXZ"
        else:
            assert record.address == current.address
            assert record.mode == current.mode
            assert record.name == current.name
            assert record.image_spans == current.image_spans
            assert record.kind == current.kind and record.exclusion == current.exclusion
            assert record.source_role == current.source_role
            assert record.source_role_reason == current.source_role_reason
            assert not record.byte_exact or current.byte_exact
            assert not record.semantic or current.semantic or current.byte_exact
            retained_history(record, current)
            assert_api_symbol_transition(record.address, record.symbol, current.symbol, current)


def test_pinned_original_unordered_gates_and_relative_link():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(),
                    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x6385D0, 0x638611)), 0x6385D0))
    assert instructions[-1].mnemonic == "ret"
    assert instructions[-1].address + instructions[-1].size == 0x638611
    anchors = {item.address: (item.mnemonic, item.op_str) for item in instructions}
    assert anchors[0x6385D4] == ("test", "byte ptr [edx], 1")
    assert anchors[0x6385E6] == ("fnstsw", "ax")
    assert anchors[0x6385E8] == ("test", "ah, 1")
    assert anchors[0x6385EB] == ("je", "0x63860e")
    assert anchors[0x6385ED] == ("mov", "ecx, dword ptr [edx + 0xc]")
    assert anchors[0x6385F0] == ("add", "ecx, edx")
    assert anchors[0x6385FF] == ("fnstsw", "ax")
    assert anchors[0x638601] == ("test", "ah, 0x41")
    assert anchors[0x638604] == ("je", "0x63860e")


def program():
    source = (ROOT / "src/caviar.cpp").read_text()
    header = (ROOT / "src/caviar.h").read_text()
    shape = extract(header, "struct CvrShape {") + ";"
    walker = extract(source, 'extern "C" CvrShape *__cdecl vox_walk_scaled_chain(')
    return r'''#include <cstdint>
#include <cstddef>
#include <cassert>
#include <limits>
#define __cdecl
#pragma pack(push,1)
SHAPE
#pragma pack(pop)
static const float vox_sqrt_two = 1.414213562f;
WALKER
int main() {
    static_assert(sizeof(CvrShape)==43,"accessed prefix, not allocation size");
    static_assert(offsetof(CvrShape,next_shape_offset_)==12,"relative link");
    static_assert(offsetof(CvrShape,field_10_)==16,"unresolved word");
    static_assert(offsetof(CvrShape,item_stream_)==42,"variable stream");
    CvrShape nodes[3]={};
    for (int i=0;i<3;++i) {
        nodes[i].field_0_=1;
        nodes[i].scale_=0.5f;
        nodes[i].next_shape_offset_=sizeof(CvrShape);
    }
    nodes[2].field_0_=0;
    assert(vox_walk_scaled_chain(nodes,1)==nodes+2);
    // Only flag bit zero controls traversal; the other prefix bits survive.
    nodes[0].field_0_=0x80000002u;
    nodes[0].next_shape_offset_=0x7fffffff;
    assert(vox_walk_scaled_chain(nodes,1)==nodes);
    nodes[0].field_0_=0x80000003u;
    nodes[0].next_shape_offset_=sizeof(CvrShape);
    assert(vox_walk_scaled_chain(nodes,1)==nodes+2);
    // First gate is strict, second gate inclusive, before any next flags read.
    nodes[0].scale_=vox_sqrt_two;
    nodes[0].next_shape_offset_=0x7fffffff;
    assert(vox_walk_scaled_chain(nodes,1)==nodes);
    nodes[0].next_shape_offset_=sizeof(CvrShape);
    nodes[0].scale_=0.5f;
    nodes[1].scale_=vox_sqrt_two;
    assert(vox_walk_scaled_chain(nodes,1)==nodes+1);
    nodes[1].scale_=2;
    assert(vox_walk_scaled_chain(nodes,1)==nodes);
    // Negative byte links are not unsigned offsets or C++ element indices.
    nodes[1].scale_=0.5f;
    nodes[1].next_shape_offset_=-static_cast<int32_t>(sizeof(CvrShape));
    nodes[0].field_0_=0;
    assert(vox_walk_scaled_chain(nodes+1,1)==nodes);
    nodes[0].field_0_=1;
    nodes[1].next_shape_offset_=sizeof(CvrShape);
    const float nan=std::numeric_limits<float>::quiet_NaN();
    nodes[0].scale_=nan;
    assert(vox_walk_scaled_chain(nodes,1)==nodes+2);
    nodes[0].scale_=0.5f;
    nodes[1].scale_=nan;
    assert(vox_walk_scaled_chain(nodes,1)==nodes+2);
    nodes[1].scale_=0.5f;
    assert(vox_walk_scaled_chain(nodes,nan)==nodes+2);
    nodes[0].scale_=std::numeric_limits<float>::infinity();
    assert(vox_walk_scaled_chain(nodes,0)==nodes+2);
    assert(vox_walk_scaled_chain(nodes,1)==nodes);
    nodes[0].scale_=-std::numeric_limits<float>::infinity();
    assert(vox_walk_scaled_chain(nodes,1)==nodes+2);
}
'''.replace("SHAPE", shape).replace("WALKER", walker)


def run(tmp_path, text, succeeds):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "actual-source host compiler required"
    cpp = tmp_path / "shape-chain.cpp"
    cpp.write_text(text)
    for compiler in filter(None, compilers):
        exe = tmp_path / Path(compiler).name.replace("+", "p")
        subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined",
                        "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
                       check=True, capture_output=True, timeout=60)
        # Keep sanitizer checks and nonrecovering failures enabled, but do not
        # let symbolization of an intentionally faulting mutant outlive it.
        # Diagnostic decoration is not part of this behavioral control.
        environment = {**os.environ, "UBSAN_OPTIONS":
                       os.environ.get("UBSAN_OPTIONS", "") + ":symbolize=0"}
        result = subprocess.run([str(exe)], capture_output=True, timeout=30,
                                env=environment)
        assert (result.returncode == 0) == succeeds, result.stderr.decode()


def test_actual_shape_layout_and_original_traversal(tmp_path):
    run(tmp_path, program(), True)


@pytest.mark.parametrize("old,new", (
    ("extent * scale >= vox_sqrt_two", "!(extent * scale < vox_sqrt_two)"),
    ("next_extent * scale > vox_sqrt_two", "!(next_extent * scale <= vox_sqrt_two)"),
    ("extent * scale >= vox_sqrt_two", "extent * scale > vox_sqrt_two"),
    ("next_extent * scale > vox_sqrt_two", "next_extent * scale >= vox_sqrt_two"),
    ("while (flags & 1)", "while (flags != 0)"),
))
def test_unordered_threshold_and_flag_damage_rejected(tmp_path, old, new):
    text = program()
    assert old in text
    run(tmp_path, text.replace(old, new, 1), False)
