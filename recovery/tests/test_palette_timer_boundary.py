"""Palette stop_animation keeps the original typed Time::stop boundary."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/palette.cpp"
HEADER = ROOT / "src/jackal/palette.h"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
SIGNATURE = "void Palette::stop_animation(int key)"
START = 0x005FE900
CALL_PC = 0x005FE939
TIME_STOP = 0x00616730


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def test_palette_stop_animation_uses_real_time_member_boundary():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    assert "stop_animation_stop_call" not in source
    assert "internal_[i].time->stop();" in _body(source, SIGNATURE)
    assert "void stop_animation(int key);" in header
    assert "MEASURED void stop();" in (ROOT / "src/time.h").read_text(
        encoding="utf-8")


def test_pinned_palette_stop_animation_calls_mapped_time_stop():
    if not IMAGE.is_file():
        pytest.skip("pinned original image unavailable")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import (
        X86_OP_IMM, X86_OP_MEM, X86_REG_EAX, X86_REG_ECX,
    )

    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
    rows = [row for row in read_file(SOURCE) if row.address == START]
    assert len(rows) == 1
    owner = rows[0]
    assert len(owner.image_spans) == 1
    low, high = owner.image_spans[0]
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    assert (low, high) == (START, 0x005FE941)
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
        cursor += instruction.size
    assert cursor == high
    by_address = {instruction.address: instruction for instruction in instructions}
    load = by_address[0x005FE933]
    assert load.mnemonic == "mov"
    assert load.operands[0].reg == X86_REG_ECX
    assert load.operands[1].type == X86_OP_MEM
    assert load.operands[1].mem.base == X86_REG_EAX
    assert load.operands[1].mem.disp == 0x408
    assert by_address[CALL_PC].mnemonic == "call"
    call = by_address[CALL_PC]
    assert call.operands[0].type == X86_OP_IMM
    assert call.operands[0].imm == TIME_STOP
    assert by_address[0x005FE939].size == 5


def test_actual_stop_animation_body_matches_slots_and_first_duplicate(tmp_path):
    """Compile extracted C++17 source behavior; not a VC6 ABI test."""
    source = SOURCE.read_text(encoding="utf-8")
    body = _body(source, SIGNATURE)
    program = r'''#include <cassert>
#include <cstdint>
#define __cdecl
#define __thiscall
struct Time {
  int id;
  int *calls;
  void stop() { ++*calls; }
};
struct PaletteInternal { uint32_t key; Time *time; uint8_t first, count, a, b; void *colours; };
class Palette {
 public:
  void stop_animation(int key);
  PaletteInternal internal_[5];
};
void Palette::stop_animation(int key) {
BODY
}
int main() {
  int calls[5] = {0, 0, 0, 0, 0};
  Time timers[5] = {{0,&calls[0]}, {1,&calls[1]}, {2,&calls[2]},
                    {3,&calls[3]}, {4,&calls[4]}};
  Palette p = {};
  for (int i = 0; i < 5; ++i) {
    p.internal_[i].key = 100u + (unsigned)i;
    p.internal_[i].time = &timers[i];
  }
  for (int selected = 0; selected < 5; ++selected) {
    for (int i = 0; i < 5; ++i) calls[i] = 0;
    p.stop_animation(100 + selected);
    for (int i = 0; i < 5; ++i) assert(calls[i] == (i == selected));
  }

  for (int i = 0; i < 5; ++i) calls[i] = 0;
  p.internal_[0].key = 10;
  p.internal_[1].key = 11;
  p.internal_[2].key = 0xffffffffu; // first free slot terminates search
  p.internal_[3].key = 12;
  p.internal_[4].key = 13;
  p.stop_animation(12); // free slot's key does not match; no later search
  for (int i = 0; i < 5; ++i) assert(calls[i] == 0);

  for (int i = 0; i < 5; ++i) calls[i] = 0;
  p.internal_[0].key = 20;
  p.internal_[1].key = 21;
  p.internal_[2].key = 22;
  p.internal_[3].key = 22;
  p.internal_[4].key = 24;
  p.stop_animation(22);
  assert(calls[0] == 0 && calls[1] == 0 && calls[2] == 1
         && calls[3] == 0 && calls[4] == 0);
}
'''.replace("BODY", body)
    cpp = tmp_path / "palette_stop_animation.cpp"
    cpp.write_text(program, encoding="utf-8")
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for index, compiler in enumerate(compilers):
        executable = tmp_path / f"palette_stop_animation_{index}"
        subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(executable)], check=True,
                       capture_output=True, text=True)
        subprocess.run([str(executable)], check=True, capture_output=True,
                       text=True)
