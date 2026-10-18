"""Actual-source controls for the out-of-line Buffer copy boundary."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
BUFFER_CPP = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
BUFFER_H = (ROOT / "src/buffer.h").read_text(encoding="utf-8")
FLIC_CPP = (ROOT / "src/flic.cpp").read_text(encoding="utf-8")
FLIC_CODE = re.sub(r"/\*.*?\*/|//[^\n]*", " ", FLIC_CPP, flags=re.S)


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated source body: {signature}")


COPY = body(BUFFER_CPP, "int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,")


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>
using std::memcpy;
using std::memmove;
typedef unsigned char uint8_t;
struct RECT { int left, top, right, bottom; };
struct Header { int biWidth, biHeight; };
struct Dib { Header bmiHeader; };
static int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
  out->left = a->left > b->left ? a->left : b->left;
  out->top = a->top > b->top ? a->top : b->top;
  out->right = a->right < b->right ? a->right : b->right;
  out->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
  return out->left < out->right && out->top < out->bottom;
}
class Buffer {
 public:
  Dib dib_;
  RECT rect1_;
  void *dib_bits_, *locked_bits_;
  void *surface_;
  unsigned int stride_;
  int fail_get_call=0, get_calls=0, free_calls=0, locks=0;
  int get_data() {
    ++get_calls;
    if (get_calls == fail_get_call) return 0;
    locked_bits_ = dib_bits_;
    if (!locked_bits_) return 0;
    ++locks;
    return 1;
  }
  void free_data(int) {
    ++free_calls;
    if (--locks == 0) locked_bits_ = 0;
  }
  int copy(Buffer *, int, int, int, int, int, int);
};
// ACTUAL_COPY

static Buffer make(void *bits, int width, int height, int stride) {
  Buffer b{}; b.dib_.bmiHeader.biWidth=width;
  b.dib_.bmiHeader.biHeight=-height; b.rect1_={0,0,width,height};
  b.dib_bits_=bits; b.stride_=stride; return b;
}
static void self_case(int dx, int dy, int width) {
  unsigned char pixels[24*12];
  for (int i=0; i<24*12; ++i) pixels[i]=(unsigned char)(i+1);
  unsigned char expected[24*12];
  for (int i=0; i<24*12; ++i) expected[i]=pixels[i];
  const int sx=5, sy=3, tx=sx+dx, ty=sy+dy, height=4, stride=24;
  const bool reverse_x=tx>=sx, reverse_y=ty>=sy;
  for (int ri=0; ri<height; ++ri) {
    const int row=reverse_y ? height-1-ri : ri;
    if (!reverse_x) {
      for (int column=0; column<width; ++column) {
        expected[(ty+row)*stride+tx+column] =
            expected[(sy+row)*stride+sx+column];
      }
    } else {
      const int x_offset=width<4 ? width-1 : width-4;
      const int groups=width>>2, remainder=width&3;
      int destination_x=tx+x_offset+remainder;
      int source_x=sx+x_offset+remainder;
      for (int group=0; group<groups; ++group) {
        unsigned char word[4];
        for (int byte=0; byte<4; ++byte)
          word[byte]=expected[(sy+row)*stride+source_x+byte];
        for (int byte=0; byte<4; ++byte)
          expected[(ty+row)*stride+destination_x+byte]=word[byte];
        destination_x-=4; source_x-=4;
      }
      for (int byte=0; byte<remainder; ++byte) {
        expected[(ty+row)*stride+destination_x]=
            expected[(sy+row)*stride+source_x];
        --destination_x; --source_x;
      }
    }
  }
  Buffer same=make(pixels,20,10,stride);
  assert(same.copy(&same,sx,sy,tx,ty,width,height)==0);
  for (int i=0; i<24*12; ++i) assert(pixels[i]==expected[i]);
  assert(same.get_calls==2 && same.free_calls==2 && same.locks==0);
}
static void clipped_self_case() {
  unsigned char pixels[32*12];
  for (int i=0; i<32*12; ++i) pixels[i]=(unsigned char)(i+3);
  unsigned char expected[32*12];
  for (int i=0; i<32*12; ++i) expected[i]=pixels[i];
  Buffer same=make(pixels,28,10,32);
  same.rect1_={8,0,27,10};
  // IntersectRect moves the destination left edge from 6 to 8 and moves
  // the source x from 7 to 9; direction selection uses those adjusted args.
  const int source_x=9, source_y=3, destination_x=8, destination_y=4;
  const int width=6, height=3, stride=32;
  for (int row=height-1; row>=0; --row) {
    for (int column=0; column<width; ++column) {
      expected[(destination_y+row)*stride+destination_x+column]=
          expected[(source_y+row)*stride+source_x+column];
    }
  }
  assert(same.copy(&same,7,3,6,4,8,3)==0);
  for (int i=0; i<32*12; ++i) assert(pixels[i]==expected[i]);
  assert(same.get_calls==2 && same.free_calls==2 && same.locks==0);
}
static void reverse_boundary_case() {
  unsigned char pixels[32*4];
  for (int i=0; i<32*4; ++i) pixels[i]=(unsigned char)(i+11);
  unsigned char expected[32*4];
  for (int i=0; i<32*4; ++i) expected[i]=pixels[i];
  Buffer same=make(pixels,10,2,32);
  same.rect1_={0,0,20,2};
  // The original >=4 branch validates x+width-4 (8), not x+width-1
  // (11). Its remainder-adjusted dword/tail writes stay inside this larger
  // backing allocation while crossing the nominal ten-pixel DIB width.
  for (int byte=0; byte<4; ++byte)
    expected[9+byte]=expected[32+6+byte];
  expected[5]=expected[32+2];
  assert(same.copy(&same,4,1,7,0,5,1)==0);
  for (int i=0; i<32*4; ++i) assert(pixels[i]==expected[i]);
  assert(same.get_calls==2 && same.free_calls==2 && same.locks==0);
}
int main() {
  unsigned char src[18]={0,1,2,3,90,90, 4,5,6,7,90,90,
                        8,9,10,11,90,90};
  unsigned char dst[21]={};
  Buffer source=make(src,4,3,6), destination=make(dst,4,3,7);
  destination.rect1_={1,1,4,3};
  assert(source.copy(&destination,0,0,0,0,4,3)==0);
  assert(dst[8]==5 && dst[9]==6 && dst[10]==7);
  assert(dst[15]==9 && dst[16]==10 && dst[17]==11);
  assert(source.locked_bits_==0 && destination.locked_bits_==0);

  Buffer no_storage=make(0,4,3,4);
  assert(no_storage.copy(0,0,0,0,0,1,1)==7);
  Buffer live=make(src,4,3,6);
  assert(live.copy(0,0,0,0,0,1,1)==3);
  assert(live.copy(&destination,8,0,0,0,1,1)==0);

  // All four self-copy direction arms preserve the pinned row order,
  // clipped pointer origins and the original four-byte reverse-copy split.
  for (int dx : {-1,1}) for (int dy : {-1,1}) {
    for (int width : {3,4,5,6,7,8}) self_case(dx,dy,width);
  }
  clipped_self_case();
  reverse_boundary_case();

  // Distinct wrappers over aliased storage still take the forward-only
  // branch: a four-byte transfer reads/writes one dword-sized group, while
  // a three-byte tail proceeds byte by byte.
  unsigned char aliased[8]={0,1,2,3,4,5,6,7};
  Buffer alias_source=make(aliased,8,1,8);
  Buffer alias_destination=make(aliased,8,1,8);
  assert(alias_source.copy(&alias_destination,0,0,1,0,4,1)==0);
  assert(aliased[0]==0 && aliased[1]==0 && aliased[2]==1 &&
         aliased[3]==2 && aliased[4]==3);
  assert(alias_source.get_calls==1 && alias_destination.get_calls==1);
  assert(alias_source.free_calls==1 && alias_destination.free_calls==1);
  unsigned char mixed_alias[8]={0,1,2,3,4,5,6,7};
  Buffer mixed_source=make(mixed_alias,8,1,8);
  Buffer mixed_destination=make(mixed_alias,8,1,8);
  assert(mixed_source.copy(&mixed_destination,0,0,1,0,5,1)==0);
  assert(mixed_alias[0]==0 && mixed_alias[1]==0 && mixed_alias[2]==1 &&
         mixed_alias[3]==2 && mixed_alias[4]==3 && mixed_alias[5]==3);
  unsigned char byte_alias[8]={0,1,2,3,4,5,6,7};
  Buffer byte_source=make(byte_alias,8,1,8);
  Buffer byte_destination=make(byte_alias,8,1,8);
  assert(byte_source.copy(&byte_destination,0,0,1,0,3,1)==0);
  assert(byte_alias[0]==0 && byte_alias[1]==0 && byte_alias[2]==0 &&
         byte_alias[3]==0);
  unsigned char row_alias[12]={0,1,2,3,4,5,6,7,8,9,10,11};
  Buffer row_source=make(row_alias,4,3,4);
  Buffer row_destination=make(row_alias,4,3,4);
  assert(row_source.copy(&row_destination,0,0,0,1,4,2)==0);
  assert(row_alias[4]==0 && row_alias[5]==1 && row_alias[6]==2 && row_alias[7]==3);
  assert(row_alias[8]==0 && row_alias[9]==1 && row_alias[10]==2 && row_alias[11]==3);

  // Clipping moves the read origin by the clipped write overhang.
  unsigned char clipped_src[8]={0,1,2,3,4,5,6,7};
  unsigned char clipped_dst[8]={};
  Buffer clip_source=make(clipped_src,4,2,4), clip_destination=make(clipped_dst,4,2,4);
  assert(clip_source.copy(&clip_destination,-1,0,0,0,4,1)==0);
  assert(clipped_dst[0]==0 && clipped_dst[1]==0 && clipped_dst[2]==1 &&
         clipped_dst[3]==2);

  // The first (destination) acquisition failure has nothing to release;
  // source failure after destination acquisition releases exactly once.
  unsigned char fail_a[8]={}, fail_b[8]={};
  Buffer fail_source=make(fail_a,4,2,4), fail_destination=make(fail_b,4,2,4);
  fail_destination.fail_get_call=1;
  assert(fail_source.copy(&fail_destination,0,0,0,0,2,1)==3);
  assert(fail_destination.get_calls==1 && fail_source.get_calls==0);
  assert(fail_destination.free_calls==0 && fail_source.free_calls==0);
  fail_source=make(fail_a,4,2,4); fail_destination=make(fail_b,4,2,4);
  fail_source.fail_get_call=1;
  assert(fail_source.copy(&fail_destination,0,0,0,0,2,1)==3);
  assert(fail_destination.get_calls==1 && fail_source.get_calls==1);
  assert(fail_destination.free_calls==1 && fail_destination.locks==0);

  // Self-copy second acquisition failure unwinds only the first reference.
  unsigned char self_fail_bits[8]={};
  Buffer self_fail=make(self_fail_bits,4,2,4);
  self_fail.fail_get_call=2;
  assert(self_fail.copy(&self_fail,0,0,0,0,2,1)==3);
  assert(self_fail.get_calls==2 && self_fail.free_calls==1 && self_fail.locks==0);
}
'''


def compile_and_run(tmp_path: Path, compiler: str, implementation: str) -> subprocess.CompletedProcess:
    source = tmp_path / "copy_boundary.cpp"
    binary = tmp_path / "copy_boundary"
    source.write_text(HARNESS.replace("// ACTUAL_COPY", implementation), encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_definition_is_out_of_line_and_header_retains_only_api():
    marker = BUFFER_CPP.index("// ORIGINAL: 0x005DFF00 ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z")
    mapping = BUFFER_CPP[marker:BUFFER_CPP.index(
        "int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,")]
    assert "// body      src/buffer.h" not in mapping
    assert "MEASURED int copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,\n                    int width, int height);" in BUFFER_H
    assert "MEASURED int copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,\n                    int width, int height) {" not in BUFFER_H
    assert BUFFER_CPP.count("int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,") == 1


def test_seven_copy_calls_and_flic_getter_calls_keep_direct_boundaries():
    assert "return copy(buffer, xCoord, yCoord, xCoord, yCoord, width, height);" in BUFFER_CPP
    assert "return copy(buffer, left, top, left, top," in BUFFER_CPP
    assert "source->copy(this, 0, 0, colX, rowY, tileWidth," in BUFFER_CPP
    assert "buffer->copy(&buffer_, origin_x_, origin_y_, 0, 0, decoded_width_," in FLIC_CPP
    assert "buffer_.copy(buffer2_, 0, 0, origin_x_, origin_y_," in FLIC_CPP
    assert FLIC_CPP.count("buffer_.copy(buffer2_, 0, 0, origin_x_, origin_y_,") == 2
    assert "buffer2_->copy(&buffer_, origin_x_, origin_y_, 0, 0, decoded_width_," in FLIC_CPP
    assert "int (Buffer::*copy)" not in BUFFER_CPP
    assert "int (Buffer::*copy)" not in FLIC_CPP
    assert FLIC_CODE.count("buffer2_->get_data()") == 3
    assert FLIC_CODE.count("buffer_.get_data()") == 3
    assert "Buffer::*get_data" not in FLIC_CODE
    assert "(this->*&Flic::close)()" not in FLIC_CODE


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_copy_body_runs_and_rejects_storage_mutant(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    assert compile_and_run(tmp_path, compiler, COPY).returncode == 0
    mutant = COPY.replace("return 7;", "return 0;", 1)
    assert mutant != COPY
    failed = compile_and_run(tmp_path, compiler, mutant)
    assert failed.returncode != 0
