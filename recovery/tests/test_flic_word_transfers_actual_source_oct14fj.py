"""Bounded actual-source FLIC word-transfer controls, not full decoder proof."""
from __future__ import annotations

import shutil
import subprocess
import os
import re
from pathlib import Path
import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated method: {signature}")


def _require_byte_transfers(delta, trans):
    def compact(body):
        masked = COMMENTS_OR_STRINGS.sub(lambda m: " " * len(m[0]), body)
        return re.sub(r"\s+", "", masked)
    delta, trans = compact(delta), compact(trans)
    assert not re.search(r"reinterpret_cast<[^>]*uint16_t[^>]*>", delta + trans), \
        "word alignment assumption reintroduced"
    assert delta.count("std::memcpy(dst,&word,sizeof(word));") == 1
    assert trans.count("std::memcpy(out,&word,sizeof(word));") == 1
    assert trans.count("uint16_tpixel;std::memcpy(&pixel,src,sizeof(pixel));"
                       "std::memcpy(out,&pixel,sizeof(pixel));") == 1, \
        "paired transfer must load before store"


def test_actual_byte_transfer_representation_and_damage_controls():
    source = (SRC / "flic.cpp").read_text()
    delta = _function(source, "void Flic::decode_delta_flc() {")
    trans = _function(source, "void Flic::decode_delta_flc_trans() {")
    _require_byte_transfers(delta, trans)
    with pytest.raises(AssertionError, match="word alignment assumption"):
        _require_byte_transfers(delta.replace(
            "std::memcpy(dst, &word, sizeof(word));",
            "*reinterpret_cast<uint16_t *>(dst) = word;", 1), trans)
    with pytest.raises(AssertionError, match="word alignment assumption"):
        _require_byte_transfers(delta, trans.replace(
            "std::memcpy(&pixel, src, sizeof(pixel));",
            "pixel = *reinterpret_cast<uint16_t *>(src);", 1))
    pair = ("std::memcpy(&pixel, src, sizeof(pixel));\n"
            "                                    std::memcpy(out, &pixel, sizeof(pixel));")
    assert trans.count(pair) == 1
    with pytest.raises(AssertionError, match="load before store"):
        _require_byte_transfers(delta, trans.replace(pair,
            "std::memcpy(out, &pixel, sizeof(pixel));\n"
            "                                    std::memcpy(&pixel, src, sizeof(pixel));", 1))


def test_pinned_word_transfer_widths_and_load_store_order():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_REG_EBX, X86_REG_EDI, X86_REG_ESI
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image_path.is_file():
        pytest.skip("pinned image unavailable")
    image = PeImage(image_path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for address, base in ((0x62AA24, X86_REG_EDI), (0x62B52C, X86_REG_ESI)):
        instruction = next(decoder.disasm(image.read(Span(address, address + 16)), address))
        assert instruction.mnemonic == "mov"
        memory, value = instruction.operands
        assert memory.type == X86_OP_MEM and memory.size == 2
        assert memory.mem.base == base and memory.mem.disp == 0 and memory.mem.index == 0
        assert value.type == X86_OP_REG and value.size == 2
    pair = {i.address: i for i in decoder.disasm(
        image.read(Span(0x62B5CD, 0x62B5D6)), 0x62B5CD)}
    load, store = pair[0x62B5CD], pair[0x62B5D3]
    assert load.mnemonic == store.mnemonic == "mov"
    register, source = load.operands
    target, stored = store.operands
    assert source.type == target.type == X86_OP_MEM
    assert source.size == target.size == 2
    assert source.mem.base == X86_REG_EBX and target.mem.base == X86_REG_ESI
    assert register.type == stored.type == X86_OP_REG
    assert register.size == stored.size == 2 and register.reg == stored.reg


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_flic_word_transfers_unaligned_palette_and_alias_order(
        tmp_path: Path, compiler_name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (SRC / "flic.cpp").read_text()
    delta = _function(source, "void Flic::decode_delta_flc() {")
    trans = _function(source, "void Flic::decode_delta_flc_trans() {")
    assert delta.count("std::memcpy(dst, &word, sizeof(word));") == 1
    assert trans.count("std::memcpy(&pixel, src, sizeof(pixel));") == 1
    assert trans.count("std::memcpy(out, &pixel, sizeof(pixel));") == 1
    assert trans.count("std::memcpy(out, &word, sizeof(word));") == 1
    unit = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct Stream { const uint8_t *bytes; size_t size; size_t position; };
static Stream *active_stream;
static size_t flic_test_fread(void *out, size_t size, size_t count, void *file) {
  if (file != active_stream || size * count > active_stream->size - active_stream->position)
    return 0;
  std::memcpy(out, active_stream->bytes + active_stream->position, size * count);
  active_stream->position += size * count;
  return count;
}
static void *mem_get(size_t size) { return std::malloc(size); }
struct Buffer {
  uint32_t stride_;
  uint8_t *bytes_;
  uint8_t *get_data(int, int) { return bytes_; }
  void *get_data() { return bytes_; }
};
struct Rect { int left, top, right, bottom; };
struct Flic {
  uint32_t field_5A4_;
  uint32_t decoded_width_;
  uint32_t origin_x_, origin_y_;
  uint8_t field_0_;
  Buffer *buffer2_;
  Buffer buffer_;
  void *file_;
  Rect update_rect_;
  void decode_delta_flc();
  void decode_delta_flc_trans();
};
#define fread flic_test_fread
''' + delta + "\n" + trans + r'''
#undef fread
static int fail(const char *message) {
  std::fprintf(stderr, "FAIL: %s\n", message);
  return 97;
}
static bool bytes_equal(const uint8_t *actual, int at,
                        const uint8_t *expected, size_t count) {
  return std::memcmp(actual + at, expected, count) == 0;
}
static int run_trans(uint8_t transparent, uint16_t raw_word,
                     const uint8_t *source_initial, const uint8_t *expected,
                     const char *label, bool overlap) {
  alignas(16) uint8_t shared[64] = {0, 0x31, 0x32, 0x34, 0x35, 0x36, 0x37};
  alignas(16) uint8_t separate_source[64] = {};
  alignas(16) uint8_t separate_output[64];
  std::memset(separate_output, 0xa5, sizeof(separate_output));
  if (source_initial) std::memcpy(separate_source, source_initial, 8);
  Buffer destination{16, overlap ? shared + 1 : separate_output};
  Flic f = {};
  f.field_5A4_ = 0x10;
  f.field_0_ = transparent;
  f.buffer2_ = &destination;
  f.buffer_.stride_ = 16;
  f.buffer_.bytes_ = overlap ? shared : separate_source;
  const uint8_t stream_bytes[] = {
      1, 0, 1, 0, 1, 0xfe,
      static_cast<uint8_t>(raw_word), static_cast<uint8_t>(raw_word >> 8)};
  Stream stream{stream_bytes, sizeof(stream_bytes), 0};
  active_stream = &stream;
  f.file_ = &stream;
  f.decode_delta_flc_trans();
  const uint8_t *actual = overlap ? shared : separate_output;
  const int output_at = overlap ? 2 : 1;
  if (!bytes_equal(actual, output_at, expected, 4)) return fail(label);
  return 0;
}
int main() {
  // The non-transparent delta run writes the same biased word to an odd
  // destination address; the expected bytes also pin host byte order.
  alignas(16) uint8_t delta_output[64];
  std::memset(delta_output, 0xa5, sizeof(delta_output));
  Buffer delta_buffer{16, delta_output};
  Flic delta = {};
  delta.field_5A4_ = 0x10;
  delta.buffer2_ = &delta_buffer;
  const uint8_t delta_stream_bytes[] = {1, 0, 1, 0, 1, 0xfe, 0xc0, 0xb0};
  Stream delta_stream{delta_stream_bytes, sizeof(delta_stream_bytes), 0};
  active_stream = &delta_stream;
  delta.file_ = &delta_stream;
  delta.decode_delta_flc();
  const uint8_t filled[] = {0xd0, 0xc0, 0xd0, 0xc0};
  if (!bytes_equal(delta_output, 1, filled, sizeof(filled)))
    return fail("delta repeated-word byte order/palette bias");

  const uint8_t copied_source[] = {0, 0x12, 0x34, 0x56, 0x78, 0, 0, 0};
  const uint8_t copied[] = {0x12, 0x34, 0x56, 0x78};
  const uint8_t low_source[] = {0, 0x11, 0x22, 0x33, 0x44, 0, 0, 0};
  const uint8_t low_result[] = {0xa1, 0x22, 0xa1, 0x44};
  const uint8_t high_result[] = {0x11, 0xb2, 0x33, 0xb2};
  const uint8_t opaque_result[] = {0xb2, 0xa1, 0xb2, 0xa1};
  if (run_trans(0x77, 0x6767, copied_source, copied,
                "transparent paired copy", false)) return 97;
  if (run_trans(0x77, 0x9167, low_source, low_result,
                "low-byte transparent branch", false)) return 97;
  if (run_trans(0x77, 0x67a2, low_source, high_result,
                "high-byte transparent branch", false)) return 97;
  if (run_trans(0x77, 0x91a2, low_source, opaque_result,
                "opaque repeated-word branch", false)) return 97;

  // Output begins one byte after source. Each word is loaded before its
  // corresponding store, so the first store changes the next source word.
  const uint8_t overlap_result[] = {0x31, 0x32, 0x32, 0x35};
  if (run_trans(0x77, 0x6767, nullptr, overlap_result,
                "overlapping paired-copy order", true)) return 97;
  return 0;
}
'''
    unit_path = tmp_path / "flic_word_transfers.cpp"
    unit_path.write_text(unit)
    binary = tmp_path / "flic_word_transfers"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", str(unit_path), "-o", str(binary)], check=True)
    baseline = subprocess.run([str(binary)], capture_output=True, text=True)
    assert baseline.returncode == 0, baseline.stderr

    # Compile-success semantic mutants fail through the same explicit oracle.
    mutants = (
        ("std::memcpy(dst, &word, sizeof(word));",
         "std::memcpy(dst, &line_hdr, sizeof(word));",
         "delta repeated-word byte order/palette bias"),
        ("std::memcpy(&pixel, src, sizeof(pixel));",
         "std::memcpy(&pixel, out, sizeof(pixel));",
         "transparent paired copy"),
        ("std::memcpy(out, &word, sizeof(word));",
         "std::memcpy(out, &opcode, sizeof(word));",
         "opaque repeated-word branch"),
    )
    for index, (before, after, diagnostic) in enumerate(mutants):
        assert unit.count(before) == 1
        mutant_path = tmp_path / f"flic_word_mutant_{index}.cpp"
        mutant_path.write_text(unit.replace(before, after, 1))
        mutant_binary = tmp_path / f"flic_word_mutant_{index}"
        subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        "-pedantic", str(mutant_path), "-o", str(mutant_binary)],
                       check=True)
        result = subprocess.run([str(mutant_binary)], capture_output=True, text=True)
        assert result.returncode == 97
        assert result.stderr == f"FAIL: {diagnostic}\n"
