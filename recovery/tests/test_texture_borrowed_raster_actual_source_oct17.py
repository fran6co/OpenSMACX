"""Actual-source controls for Texture::draw_trans2.

It extracts the current production method and helpers; it does not carry an
alternate renderer or infer correctness from byte matching.
Host controls cover bounded valid borrowed textures, not x87 sticky flags,
native ABI, the unverified historical return contract, all edge refresh paths,
or renderer acceptance.
"""
from __future__ import annotations

from pathlib import Path
from fractions import Fraction
import re
import random
import shutil
import subprocess
import sys
import hashlib
import os

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
sys.path.insert(0, str(ROOT / "recovery/tests"))
from test_texture_sub61f0d0_actual_source import extract_method  # noqa: E402
from texture_direct_storage_host import direct_storage_prelude  # noqa: E402

SOURCE = ROOT / "src/texture.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/texture_sub61f0d0_actual_host.cpp"
DRAW2_INSERT = "// ACTUAL_DRAW_TRANS2_INSERTION"
HELPER_INSERT = "// ACTUAL_TRANS3_HELPER_INSERTION"
ORACLE_INSERT = "// X87_STEP_ORACLE_ASSERTIONS"
SIGNATURE = "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)"


def test_pinned_draw_trans2_fp_staging_and_opaque_loop():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import X86_OP_MEM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == pin
    image_bytes = path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == pin
    image = PeImage(image_bytes, pin)
    raw = image.read(Span(0x619D80, 0x61A23F))
    assert hashlib.sha256(raw).hexdigest() == "b4831c137261abe19f3fe217ffb21cb2b4f848bca8697d228160b74fef9caba9"
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(raw, 0x619D80))
    cursor = 0x619D80
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == 0x61A23F
    by_address = {instruction.address: instruction for instruction in instructions}
    assert by_address[0x619D96].mnemonic == "call"
    assert by_address[0x619D96].operands[0].imm == 0x5E3373
    assert by_address[0x619DA1].mnemonic == "fninit"
    assert by_address[0x619F2A].mnemonic == "and"
    assert by_address[0x619F2A].operands[1].imm == 0xFCFF
    assert by_address[0x619F2E].mnemonic == "or"
    assert by_address[0x619F2E].operands[1].imm == 0xC00
    assert by_address[0x619F36].mnemonic == "fldcw"
    assert by_address[0x619F39].mnemonic == "fld1"
    assert by_address[0x619F4B].mnemonic == "jle"
    assert by_address[0x619F4B].operands[0].imm == 0x619F6E
    assert by_address[0x61A079].mnemonic == "jle"
    assert by_address[0x61A079].operands[0].imm == 0x61A097
    for address, cell in ((0x61A085, 0x9BB4E4), (0x61A091, 0x9BB4C8)):
        store = by_address[address]
        assert store.mnemonic == "fistp"
        assert store.operands[0].type == X86_OP_MEM
        assert store.operands[0].size == 4 and store.operands[0].mem.disp == cell
    assert by_address[0x61A0A7].mnemonic == "jle"
    assert by_address[0x61A0A7].operands[0].imm == 0x61A0CA
    for address in (0x619F58, 0x619F66, 0x619F69, 0x61A0B4, 0x61A0C2, 0x61A0C5):
        assert by_address[address].mnemonic == "fild"
    assert by_address[0x619F6C].mnemonic == by_address[0x61A0C8].mnemonic == "fdivr"
    assert by_address[0x61A1DA].mnemonic == "and"
    assert by_address[0x61A1DA].op_str == "ecx, 0xffff"
    expected = ((0x61A1E0, "mov", "al, byte ptr [ebx]"),
                (0x61A1E2, "add", "edx, esi"),
                (0x61A1E4, "mov", "byte ptr [edi], al"),
                (0x61A1E8, "adc", "bl, dl"),
                (0x61A1EA, "add", "eax, ebp"),
                (0x61A1EC, "adc", "bh, dh"),
                (0x61A1EE, "inc", "edi"),
                (0x61A1EF, "dec", "ecx"))
    for address, mnemonic, operands in expected:
        instruction = by_address[address]
        assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands)
    assert by_address[0x61A1F0].mnemonic == "jns"
    assert by_address[0x61A1F0].operands[0].imm == 0x61A1E0
    assert by_address[0x61A22E].mnemonic == "call"
    assert by_address[0x61A22E].operands[0].imm == 0x5E34A3
    assert by_address[0x61A234].mnemonic == "fninit"
    # No historical return-value interpretation is inferred from the epilogue.


def _definition(source: str, name: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    matches = list(re.finditer(r"\b" + re.escape(name) + r"\s*\(", masked))
    definitions = []
    for match in matches:
        depth = 1
        end = match.end()
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        while end < len(masked) and masked[end].isspace():
            end += 1
        if depth == 0 and end < len(masked) and masked[end] == "{":
            definitions.append((match.start(), end))
    assert len(definitions) == 1, (name, len(definitions))
    _, opening = definitions[0]
    start = masked.rfind("\n", 0, definitions[0][0]) + 1
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated helper {name}")


def _replace_function(source: str, signature: str, replacement: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert masked.count(signature) == 1, signature
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[:start] + replacement + source[end + 1:]
    raise AssertionError(f"unterminated function {signature}")


def _edge_stub(source: str) -> str:
    old = """int Texture::setup_edge(EdgeScan *, int) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    return 0;
}"""
    new = """static EdgeScan planned_left, planned_right;
static int planned_left_result, planned_right_result;
static int refresh_mode, left_refreshes, right_refreshes;
int Texture::setup_edge(EdgeScan *edge, int index) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    const int side = edge->f0;
    if (refresh_mode && setup_edge_calls > 2) {
        assert(index == 3);
        // Refresh changes the live endpoints; the saved current row and
        // previously staged slopes must survive even when refresh fails.
        edge->accumA += 0x00070000;
        edge->accumB += 0x00090000;
        edge->counter = 4;
        if (side < 0) {
            ++left_refreshes;
            return refresh_mode == 1;
        }
        ++right_refreshes;
        return 0;
    }
    *edge = side < 0 ? planned_left : planned_right;
    edge->f0 = side;
    return side < 0 ? planned_left_result : planned_right_result;
}"""
    assert source.count(old) == 1
    return source.replace(old, new, 1)


def _harness_tail() -> str:
    return r'''
#include <cstdio>
static void *texture_mapping;
static unsigned char *aligned_texels;
static const unsigned int FPU_PC24_CHOP = 3;

static void reset_case() {
    setup_calls = setup_index = setup_selected_uv = 0;
    setup_edge_calls = forward_calls = reset_calls = 0;
    refresh_mode = left_refreshes = right_refreshes = 0;
    current_fpu = release_fpu = control_calls = 0;
    setup_edge_result = 0;
    buffer.events_.clear();
    active_buffer = &buffer;
    buffer.stride_ = 8;
    buffer.rect1_.left = 0;
    buffer.rect1_.top = 0;
    buffer.rect1_.right = 6;
    buffer.rect1_.bottom = 2;
    buffer.rect2_ = buffer.rect1_;
    buffer.data_ = pixels;
    std::memset(pixels, 0xCC, sizeof(pixels));
    texture.pixels_ = aligned_texels;
    texture.iWidth_ = 256;
    texture.iHeight_ = 256;
    texture.borrowed_ = 1;
    std::memset(texture.corners_, 0, sizeof(texture.corners_));
    std::memset(texture.cornersAndMids_, 0, sizeof(texture.cornersAndMids_));
    for (int i = 0; i < 8; ++i) {
        input_xy[i].x = (i & 1) ? 4 : 1;
        input_xy[i].y = (i < 2) ? 0 : 3;
    }
    planned_left = EdgeScan();
    planned_right = EdgeScan();
    planned_left_result = planned_right_result = 1;
    planned_left.counter = planned_right.counter = 8;
    planned_left.id = planned_right.id = 3;
    planned_left.accumA = 0x00FE8000;
    planned_left.accumB = 0x00FF8000;
    planned_left.accumC = 1;
    planned_left.deltaA = 0x00010000;
    planned_left.deltaB = 0x00010000;
    planned_left.deltaC = 1;
    planned_right.accumA = planned_left.accumA + 0x00030000;
    planned_right.accumB = planned_left.accumB + 0x00018000;
    planned_right.accumC = 4;
    planned_right.deltaA = 0x00030000;
    planned_right.deltaB = 0x00028000;
    planned_right.deltaC = 1;
    cell_slot_4e4 = 0x13572468;
    cell_slot_4c8 = 0x24681357;
    cell_pixel_base = 0;
    cell_texture_pixels = 0;
    cell_clip_bottom = cell_clip_left = cell_clip_right = cell_clip_top = 0;
    cell_edge_a = cell_edge_b = 0;
    cell_texture_width = cell_texture_height = cell_buffer_stride = 0;
    for (unsigned int i = 0; i < 65536; ++i)
        aligned_texels[i] = static_cast<unsigned char>((i * 37u + (i >> 8) * 11u) & 255u);
}

static unsigned int texel_index(unsigned int u, unsigned int v) {
    return (((v >> 16) & 255u) << 8) | ((u >> 16) & 255u);
}

static void assert_row(int row, unsigned int u, unsigned int v,
                       unsigned int du, unsigned int dv, int x0, int x1) {
    for (int x = x0; x <= x1; ++x) {
        const unsigned char expected = aligned_texels[texel_index(u, v)];
        if (pixels[row * buffer.stride_ + x] != expected)
            std::fprintf(stderr, "row=%d x=%d actual=%u expected=%u u=%08x v=%08x du=%08x dv=%08x\\n",
                         row, x, pixels[row * buffer.stride_ + x], expected, u, v, du, dv);
        assert(pixels[row * buffer.stride_ + x] == expected);
        u += du;
        v += dv;
    }
}

static void test_staged_uv_wrap_inclusive_right_and_release_order() {
    reset_case();
    assert((reinterpret_cast<uintptr_t>(aligned_texels) & 0xffffu) == 0);
    cell_light_threshold = 0xA5;
    aligned_texels[0xFFFE] = 0xA5; // exact key byte must still be copied
    assert(texture.draw_trans2(&buffer, input_xy, 0) == 0);
    // Row 0 consumes the initial edge deltas; row 1 consumes deltas staged
    // from the advanced edge snapshots. The distinct rows detect stale or
    // same-row-cached slope values; UV fractions cross both byte-index wraps.
    const unsigned int row0_du = ROW0_DU;
    const unsigned int row0_dv = ROW0_DV;
    const unsigned int row1_du = ROW1_DU;
    const unsigned int row1_dv = ROW1_DV;
    assert_row(0, 0x00FE8000u, 0x00FF8000u, row0_du, row0_dv, 1, 4);
    assert_row(1, 0x00FF8000u, 0x01008000u,
               row1_du, row1_dv, 2, 5);
    assert(pixels[1] == 0xA5); // original has no transparent-key skip
    assert(pixels[0] == 0xCC && pixels[5] == 0xCC);
    assert(control_calls == 1 && release_fpu == FPU_PC24_CHOP);
    assert(reset_calls == 2 && current_fpu == 0);
    assert(buffer.events_.front() == 1 && buffer.events_.back() == 4);
    assert(buffer.events_[1] == 4); // entry reset follows get_data
    assert(buffer.events_[buffer.events_.size() - 2] == 3); // release precedes reset
}

static void test_clip_right_caps_inclusively() {
    reset_case();
    buffer.rect1_.right = 3;
    assert(texture.draw_trans2(&buffer, input_xy, 0) == 0);
    assert_row(0, 0x00FE8000u, 0x00FF8000u,
               ROW0_DU, ROW0_DV, 1, 3);
    assert(pixels[4] == 0xCC);
}

static void test_preskip_advances_both_fixed_point_cursors() {
    reset_case();
    buffer.rect1_.left = 2; // one pixel lies left of clip; original presteps once
    assert(texture.draw_trans2(&buffer, input_xy, 0) == 0);
    const unsigned int du = ROW0_DU;
    const unsigned int dv = ROW0_DV;
    assert_row(0, 0x00FE8000u + du, 0x00FF8000u + dv, du, dv, 2, 4);
    assert(pixels[1] == 0xCC);
}

static void test_nonpositive_wrapped_span_keeps_previous_slope_cells() {
    reset_case();
    planned_right.accumC = 0x8001;
    planned_left.accumC = 1; // dx<<16 wraps to signed 0x80000000
    cell_slot_4e4 = 0x13572468;
    cell_slot_4c8 = 0x24681357;
    buffer.rect1_.bottom = 1;
    assert(texture.draw_trans2(&buffer, input_xy, 0) == 0);
    assert(cell_slot_4e4 == 0x13572468 && cell_slot_4c8 == 0x24681357);
    assert(control_calls == 1 && release_fpu == FPU_PC24_CHOP);
    assert(buffer.events_.front() == 1 && buffer.events_[1] == 4);
    assert(buffer.events_[buffer.events_.size() - 2] == 3 && buffer.events_.back() == 4);
}

static void test_setup_failure_releases_then_resets_without_fp_mode_change() {
    reset_case();
    planned_left_result = 0;
    assert(texture.draw_trans2(&buffer, input_xy, 0) == 0);
    assert(control_calls == 0 && release_fpu == 0);
    assert(reset_calls == 2 && current_fpu == 0);
    assert(buffer.events_.front() == 1 && buffer.events_[1] == 4);
    assert(buffer.events_[buffer.events_.size() - 2] == 3 && buffer.events_.back() == 4);
}

static void assert_no_pixels_written() {
    for (unsigned int i = 0; i < sizeof(pixels); ++i)
        assert(pixels[i] == 0xCC);
}

static void test_zero_span_preserves_slopes_without_sampling() {
    reset_case();
    planned_right.accumC = planned_left.accumC;
    buffer.rect1_.bottom = 1;
    texture.draw_trans2(&buffer, input_xy, 0);
    assert_no_pixels_written();
    assert(cell_slot_4e4 == 0x13572468 && cell_slot_4c8 == 0x24681357);
    assert(control_calls == 1 && release_fpu == FPU_PC24_CHOP);
    assert(reset_calls == 2 && current_fpu == 0);
}

static void test_nonborrowed_gate_and_right_initial_failure() {
    for (int failure = 0; failure < 2; ++failure) {
        reset_case();
        if (failure == 0) texture.borrowed_ = 0;
        else planned_right_result = 0;
        texture.draw_trans2(&buffer, input_xy, 0);
        assert(setup_edge_calls == 2);
        assert_no_pixels_written();
        assert(cell_slot_4e4 == 0x13572468 && cell_slot_4c8 == 0x24681357);
        assert(control_calls == 0 && release_fpu == 0);
        assert(reset_calls == 2 && current_fpu == 0);
        assert(buffer.events_[buffer.events_.size() - 2] == 3);
        assert(buffer.events_.back() == 4);
    }
}

static void test_endpoint_refresh_preserves_last_row_and_short_circuits_right() {
    for (int mode = 1; mode <= 2; ++mode) {
        reset_case();
        refresh_mode = mode;
        planned_left.counter = planned_right.counter = 1;
        texture.draw_trans2(&buffer, input_xy, 0);
        assert(left_refreshes == 1);
        assert(right_refreshes == (mode == 1 ? 1 : 0));
        assert(setup_edge_calls == (mode == 1 ? 4 : 3));
        assert_row(0, 0x00FE8000u, 0x00FF8000u, ROW0_DU, ROW0_DV, 1, 4);
        assert(cell_slot_4e4 == ROW0_DU && cell_slot_4c8 == ROW0_DV);
        for (unsigned int i = buffer.stride_; i < sizeof(pixels); ++i)
            assert(pixels[i] == 0xCC);
        assert(control_calls == 1 && release_fpu == FPU_PC24_CHOP);
        assert(reset_calls == 2 && current_fpu == 0);
        assert(buffer.events_[buffer.events_.size() - 2] == 3);
        assert(buffer.events_.back() == 4);
    }
}

static void test_texture_span_step_24bit_oracle() {
    // Independent Fraction-derived expectations are substituted here by the
    // Python driver. Large numerators distinguish PC24/RZ from exact integer
    // division; signed and boundary cases cover FISTP conversion behavior.
    // X87_STEP_ORACLE_ASSERTIONS
}

int main() {
    int map_flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    map_flags |= MAP_32BIT;
#endif
    texture_mapping = mmap(0, 0x30000, PROT_READ | PROT_WRITE,
                           map_flags, -1, 0);
    assert(texture_mapping != MAP_FAILED);
    uintptr_t aligned = (reinterpret_cast<uintptr_t>(texture_mapping) + 0xffffu) &
                        ~static_cast<uintptr_t>(0xffffu);
    aligned_texels = reinterpret_cast<unsigned char *>(aligned);
    assert(aligned + 65536u <= reinterpret_cast<uintptr_t>(texture_mapping) + 0x30000u);
    test_staged_uv_wrap_inclusive_right_and_release_order();
    test_clip_right_caps_inclusively();
    test_preskip_advances_both_fixed_point_cursors();
    test_nonpositive_wrapped_span_keeps_previous_slope_cells();
    test_setup_failure_releases_then_resets_without_fp_mode_change();
    test_zero_span_preserves_slopes_without_sampling();
    test_nonborrowed_gate_and_right_initial_failure();
    test_endpoint_refresh_preserves_last_row_and_short_circuits_right();
    test_texture_span_step_24bit_oracle();
    assert(munmap(texture_mapping, 0x30000) == 0);
}
'''


def translation_unit(method: str | None = None,
                     span_helper: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    method = method or extract_method(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    span_helper = span_helper or _definition(source, "texture_span_step")
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(DRAW2_INSERT) == fixture.count(HELPER_INSERT) == 1
    assert fixture.count("static void reset_case()") == 1
    # Keep the existing source-owned storage extraction, EdgeScan/Buffer/Texture
    # test doubles and compiler setup, but replace only the no-op edge and main
    # scenario tail with deterministic renderer fixtures.
    fixture = fixture.replace("// TEXTURE_DIRECT_STORAGE_PRELUDE",
                              direct_storage_prelude(source), 1)
    fixture = fixture.replace(
        HELPER_INSERT,
        _definition(source, "texture_difference") + "\n\n" +
        _definition(source, "texture_fixed") + "\n\n" + span_helper, 1)
    fixture = fixture.replace(DRAW2_INSERT, method, 1)
    fixture = _edge_stub(fixture)
    fixture = fixture.replace("static unsigned char texels[256];",
                              "static unsigned char texels[65536];", 1)
    tail = fixture.index("static void reset_case()")
    fixture = fixture[:tail] + _harness_tail()
    fixture = fixture.replace(ORACLE_INSERT, _oracle_assertions(source), 1)
    for marker, value in {
        "ROW0_DU": _x87_step_oracle(0x30000, 3),
        "ROW0_DV": _x87_step_oracle(0x18000, 3),
        "ROW1_DU": _x87_step_oracle(0x50000, 3),
        "ROW1_DV": _x87_step_oracle(0x30000, 3),
    }.items():
        fixture = fixture.replace(marker, str(value) + "u")
    # The fixture's reset stub records entry/exit order and the FPU mode seen by
    # Buffer::free_data; the actual production method still comes from source.
    old_reset = "static void texture_reset_fpu() { ++reset_calls; current_fpu = 0; }"
    new_reset = "static void texture_reset_fpu() { ++reset_calls; current_fpu = 0; " \
                "active_buffer->events_.push_back(4); }"
    assert fixture.count(old_reset) == 1
    fixture = fixture.replace(old_reset, new_reset, 1)
    return fixture


def _quantize_binary_chop(value: Fraction, precision: int = 24) -> Fraction:
    """Round an exact rational toward zero to x87 PC=24 significand bits."""
    if value == 0:
        return value
    sign = -1 if value < 0 else 1
    value = abs(value)
    numerator, denominator = value.numerator, value.denominator
    exponent = numerator.bit_length() - denominator.bit_length()
    if exponent >= 0:
        if numerator < (denominator << exponent):
            exponent -= 1
    elif (numerator << -exponent) < denominator:
        exponent -= 1
    shift = precision - 1 - exponent
    if shift >= 0:
        significand = (numerator << shift) // denominator
    else:
        significand = numerator // (denominator << -shift)
    if exponent >= precision - 1:
        rounded = Fraction(significand << (exponent - (precision - 1)), 1)
    else:
        rounded = Fraction(significand, 1 << ((precision - 1) - exponent))
    return rounded * sign


def _fistp_signed32(value: Fraction) -> int:
    magnitude = abs(value.numerator) // value.denominator
    result = -magnitude if value < 0 else magnitude
    if result < -(1 << 31) or result > (1 << 31) - 1:
        return -(1 << 31)  # masked x87 invalid conversion returns integer indefinite
    return result


def _x87_step_oracle(delta: int, width: int) -> int:
    assert width > 0
    span = width << 16
    reciprocal = _quantize_binary_chop(Fraction(1, span))
    product = _quantize_binary_chop(reciprocal * delta)
    return _fistp_signed32(product * 65536)


def _oracle_assertions(source: str) -> str:
    helper = _definition(source, "texture_span_step")
    assert "reciprocal_shift" in helper and "product >>= 1" in helper
    cases = [(0x50000, 3), (-0x30000, 5), (0x7fffffff, 3),
             (-0x80000000, 1), (0x12345678, 32767), (-17, 1),
             (196608, 3)]
    deltas = (-(1 << 31), -(1 << 31) + 1, -65537, -65536, -1, 0,
              1, 65535, 65536, 65537, (1 << 31) - 1)
    widths = (1, 2, 3, 4, 7, 255, 256, 32766, 32767)
    cases.extend((delta, width) for delta in deltas for width in widths)
    rng = random.Random(0x619D80)
    for _ in range(128):
        cases.append((rng.randrange(-(1 << 31), 1 << 31),
                      rng.randrange(1, 32768)))
    statements = []
    for delta, width in cases:
        expected = _x87_step_oracle(delta, width)
        statements.append(
            f"assert(texture_span_step({delta}, {width}) == {expected});")
    assert _x87_step_oracle(0x7fffffff, 3) != 0x7fffffff // 3, (
        "chosen precision witness no longer distinguishes PC24 from exact division")
    assert _x87_step_oracle(196608, 3) == 65535
    return "\n    ".join(statements)


def compile_and_run(source: str, compiler: str, tmp_path: Path) -> None:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"texture_raster_{compiler}.cpp"
    binary = tmp_path / f"texture_raster_{compiler}"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-Wno-unused-variable", "-Wno-unused-function", "-no-pie",
         str(cpp), "-o", str(binary)],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_draw_trans2_borrowed_raster_controls(
    compiler: str, tmp_path: Path,
) -> None:
    # Compile the complete installed method in the controlled owner shell.
    # This is not a linked-application or unrestricted rendering proof.
    body = extract_method(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    # Binding checks prevent a stale local approximation from silently being
    # tested as if it contained the recovered FP stage and direct pixel path.
    required = (
        "texture_reset_fpu()",
        "TextureRasterScratch4E4.dword",
        "TextureRasterScratch4C8.dword",
        "texture_span_step(",
        "TextureRasterUAccumulator.dword",
        "TextureRasterVAccumulator.dword",
        "a1->free_data(1)",
    )
    assert all(token in body for token in required)
    assert body.index("a1->get_data()") < body.index("texture_reset_fpu()")
    assert body.rindex("a1->free_data(1)") < body.rindex("texture_reset_fpu()")
    assert body.index("TextureRasterUAccumulator.dword = leftEdge.accumA") < \
           body.index("TextureRasterScratch4E4.dword =", body.index("TextureRasterUAccumulator.dword = leftEdge.accumA"))
    compile_and_run(translation_unit(body), compiler, tmp_path)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("no_entry_reset", "no_exit_reset"))
def test_fpu_reset_mutants_fail_runtime_controls(
    compiler: str, mutation: str, tmp_path: Path,
) -> None:
    body = extract_method(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    reset_positions = [match.start() for match in re.finditer(r"texture_reset_fpu\(\);", body)]
    assert len(reset_positions) >= 2
    position = reset_positions[0] if mutation == "no_entry_reset" else reset_positions[-1]
    mutant = body[:position] + "/* reset removed */" + body[position + len("texture_reset_fpu();"):]
    method = translation_unit(mutant)
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"texture_raster_{mutation}_{compiler}.cpp"
    binary = tmp_path / f"texture_raster_{mutation}_{compiler}"
    cpp.write_text(method, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-Wno-unused-variable", "-Wno-unused-function", "-no-pie",
         str(cpp), "-o", str(binary)],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode != 0, f"{mutation} unexpectedly passed"


def _expect_runtime_failure(source: str, compiler: str, tmp_path: Path,
                            stem: str) -> None:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"texture_raster_{stem}_{compiler}.cpp"
    binary = tmp_path / f"texture_raster_{stem}_{compiler}"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-Wno-unused-variable", "-Wno-unused-function", "-no-pie",
         str(cpp), "-o", str(binary)],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode != 0, f"{stem} unexpectedly passed"


def test_slope_pixel_and_count_mutants_fail_runtime_controls(tmp_path: Path) -> None:
    method = extract_method(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    publish = """if (stagedSpan > 0) {
                                    TextureRasterScratch4E4.dword = texture_span_step(stagedU, stagedSpan >> 16);
                                    TextureRasterScratch4C8.dword = texture_span_step(stagedV, stagedSpan >> 16);
                                }
                                stagedSpan = texture_fixed(texture_difference(
                                    rightEdge.accumC, leftEdge.accumC));
                                if (stagedSpan > 0) {
                                    stagedU = texture_difference(rightEdge.accumA, leftEdge.accumA);
                                    stagedV = texture_difference(rightEdge.accumB, leftEdge.accumB);
                                }"""
    stage_then_publish = """stagedSpan = texture_fixed(texture_difference(
                                    rightEdge.accumC, leftEdge.accumC));
                                if (stagedSpan > 0) {
                                    stagedU = texture_difference(rightEdge.accumA, leftEdge.accumA);
                                    stagedV = texture_difference(rightEdge.accumB, leftEdge.accumB);
                                }
                                if (stagedSpan > 0) {
                                    TextureRasterScratch4E4.dword = texture_span_step(stagedU, stagedSpan >> 16);
                                    TextureRasterScratch4C8.dword = texture_span_step(stagedV, stagedSpan >> 16);
                                }"""
    assert method.count(publish) == 1
    mutants = {
        "missing_staged_store": method.replace(
            "TextureRasterScratch4E4.dword = texture_span_step(stagedU, stagedSpan >> 16);",
            "/* omitted dU publication */", 1),
        "one_row_early_slope": method.replace(publish, stage_then_publish, 1),
        "swapped_uv_steps": method.replace(
            "const unsigned int du = static_cast<unsigned int>(TextureRasterScratch4E4.dword);\n"
            "                                    const unsigned int dv = static_cast<unsigned int>(TextureRasterScratch4C8.dword);",
            "const unsigned int du = static_cast<unsigned int>(TextureRasterScratch4C8.dword);\n"
            "                                    const unsigned int dv = static_cast<unsigned int>(TextureRasterScratch4E4.dword);", 1),
        "transparent_key_skip": method.replace(
            "*destPtr = sourcePixels[((v >> 16) & 0xffu) * 256u + ((u >> 16) & 0xffu)];",
            "{ const unsigned char texel = sourcePixels[((v >> 16) & 0xffu) * 256u + ((u >> 16) & 0xffu)]; "
            "if (texel != static_cast<unsigned char>(TextureRasterLightThreshold)) *destPtr = texel; }", 1),
        "exclusive_pixel_count": method.replace(
            "unsigned int remaining = static_cast<unsigned short>(spanCount) + 1u;",
            "unsigned int remaining = static_cast<unsigned short>(spanCount);", 1),
    }
    for name, mutant in mutants.items():
        assert mutant != method, f"candidate mutation anchor absent: {name}"
        for compiler in ("g++", "clang++"):
            _expect_runtime_failure(translation_unit(mutant), compiler, tmp_path, name)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_pc24_truncating_precision_mutant_fails_oracle(
    compiler: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _definition(source, "texture_span_step")
    mutant_helper = _replace_function(
        helper, "static int texture_span_step(int delta, int width)",
        "static int texture_span_step(int delta, int width) {\n"
        "    return delta / width; // deliberately removes PC24/RZ stages\n}")
    _expect_runtime_failure(translation_unit(span_helper=mutant_helper),
                            compiler, tmp_path, "exact_integer_division")
