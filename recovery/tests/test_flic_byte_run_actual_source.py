"""Actual-source FLIC byte-run lengths; host doubles are not native ABI proof.

The original byte-stores abs(signed opcode) at 62A63A and masks the dword
length at 62A655 before fread. Opcode 0x80 therefore requests 128 literal bytes.
"""
from hashlib import sha256
import os
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void Flic::decode_byte_run()"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def actual_body():
    source = (ROOT / "src/flic.cpp").read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if not depth:
            return source[start:index + 1]
    raise AssertionError("unterminated byte-run decoder")


def actual_rectangle_operation():
    source = (ROOT / "src/rectangle_operations.h").read_text()
    start = source.index("static inline RECT *__cdecl set_rectangle_dimensions(")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if not depth:
            # Host controls project the actual operation, dropping only the
            # VC6 calling-convention keyword; they do not establish its ABI.
            return source[start:index + 1].replace("__cdecl ", "")
    raise AssertionError("unterminated actual rectangle operation")


HEAD = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
static std::vector<uint8_t> bytes;
static size_t position;
static bool oversized;
static size_t read_bytes(void *out,size_t size,size_t count,void*) {
 if (size*count>bytes.size()-position) { oversized=true; return 0; }
 memcpy(out,bytes.data()+position,size*count);position+=size*count;return count;
}
#define fread read_bytes
static void *mem_get(size_t size) { return malloc(size); }
struct RECT {int left,top,right,bottom;};
struct Buffer {uint32_t stride_;uint8_t pixels[512];uint8_t*get_data(uint32_t,uint32_t){return pixels;}};
struct Flic {
 Buffer *destination_buffer_; uint32_t palette_offset_,origin_x_,origin_y_,decoded_width_,decoded_height_;
 void *file_;RECT update_rect_;void decode_byte_run();
};
"""

TAIL = r"""
int main() {
 for(unsigned offset: {0u,29u,251u}) {
  for(int length=1;length<=128;++length) {
   Buffer destination{};destination.stride_=160;
   Flic f{&destination,offset,11,17,static_cast<uint32_t>(length),1,nullptr,{}};
   bytes={0,static_cast<uint8_t>(256-length)};
   for(int i=0;i<length;++i)bytes.push_back(static_cast<uint8_t>(i*3+200));
   position=0;oversized=false;f.decode_byte_run();
   if(oversized)return 23;
   assert(position==bytes.size());
   for(int i=0;i<length;++i)assert(destination.pixels[i]==static_cast<uint8_t>(i*3+200+offset));
   assert(f.update_rect_.left==11&&f.update_rect_.top==17);
   assert(f.update_rect_.right==11+length&&f.update_rect_.bottom==18);
  }
  for(int length=1;length<=127;++length) {
   Buffer destination{};destination.stride_=160;
   Flic f{&destination,offset,11,17,static_cast<uint32_t>(length),1,nullptr,{}};
   bytes={0,static_cast<uint8_t>(length),218};position=0;oversized=false;f.decode_byte_run();
   assert(!oversized&&position==bytes.size());
   for(int i=0;i<length;++i)assert(destination.pixels[i]==static_cast<uint8_t>(218+offset));
   assert(f.update_rect_.left==11&&f.update_rect_.top==17);
   assert(f.update_rect_.right==11+length&&f.update_rect_.bottom==18);
  }
  Buffer destination{};destination.stride_=160;
  Flic f{&destination,offset,11,17,3,1,nullptr,{}};
  bytes={0,0,3,218};position=0;oversized=false;f.decode_byte_run();
  assert(!oversized&&position==bytes.size());
  for(int i=0;i<3;++i)assert(destination.pixels[i]==static_cast<uint8_t>(218+offset));
  assert(f.update_rect_.left==11&&f.update_rect_.top==17);
  assert(f.update_rect_.right==14&&f.update_rect_.bottom==18);
 }
}
"""


def run_control(tmp_path, compiler_name, body, rectangle_operation=None):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / "byte_run.cpp"
    executable = tmp_path / "byte_run"
    if rectangle_operation is None:
        rectangle_operation = actual_rectangle_operation()
    cpp.write_text(HEAD + rectangle_operation + "\n" + body + TAIL)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)], capture_output=True, text=True,
        timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_all_byte_run_lengths_offsets_and_zero_continuation(tmp_path, compiler_name):
    result = run_control(tmp_path, compiler_name, actual_body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("old_signed_length", "unsigned_opcode_guard", "wrong_rectangle_sum"))
def test_compiling_old_source_and_signed_guard_damage_are_rejected(tmp_path, compiler_name, damage):
    body = actual_body()
    if damage == "old_signed_length":
        assert body.count("uint8_t count;") == 1
        body = body.replace("uint8_t count;", "int8_t count;")
        current = "count = static_cast<uint8_t>(abs(static_cast<int8_t>(count)));"
        assert body.count(current) == 1
        body = body.replace(current, "count = static_cast<int8_t>(abs(count));")
    elif damage == "unsigned_opcode_guard":
        guard = "if (static_cast<int8_t>(count) > 0)"
        assert body.count(guard) == 1
        body = body.replace(guard, "if (count > 0)")
    operation = actual_rectangle_operation()
    if damage == "wrong_rectangle_sum":
        anchor = "rect->right = x + width;"
        assert operation.count(anchor) == 1
        operation = operation.replace(anchor, "rect->right = width;", 1)
    result = run_control(tmp_path, compiler_name, body, operation)
    if damage == "old_signed_length":
        assert result.returncode == 23, result.stderr
    else:
        assert result.returncode != 0, "compiling damage escaped the host control"


def test_pinned_opcode_abs_byte_store_and_masked_literal_length():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
    from capstone.x86_const import X86_REG_AL, X86_REG_EDX
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(image.read(Span(0x62A631, 0x62A665)), 0x62A631))
    rows = {instruction.address: instruction for instruction in decoded}
    assert sum(instruction.size for instruction in decoded) == 0x34
    assert rows[0x62A631].mnemonic == "movsx"
    assert rows[0x62A631].operands[1].reg == X86_REG_AL
    assert rows[0x62A635].mnemonic == "call"
    assert rows[0x62A635].operands[0].imm == 0x644F3A
    store = rows[0x62A63A]
    assert store.mnemonic == "mov"
    assert store.operands[0].type == X86_OP_MEM and store.operands[0].size == 1
    assert store.operands[1].reg == X86_REG_AL
    mask = rows[0x62A655]
    assert mask.mnemonic == "and"
    assert mask.operands[0].type == X86_OP_REG and mask.operands[0].reg == X86_REG_EDX
    assert mask.operands[1].type == X86_OP_IMM and mask.operands[1].imm == 0xff
    assert rows[0x62A65C].mnemonic == "push" and rows[0x62A65C].operands[0].reg == X86_REG_EDX
    assert rows[0x62A660].mnemonic == "call"
    assert rows[0x62A660].operands[0].imm == 0x646178
