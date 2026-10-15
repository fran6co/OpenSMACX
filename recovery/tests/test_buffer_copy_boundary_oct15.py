"""Actual-source controls for the out-of-line Buffer copy boundary."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
BUFFER_CPP = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
BUFFER_H = (ROOT / "src/buffer.h").read_text(encoding="utf-8")
FLIC_CPP = (ROOT / "src/flic.cpp").read_text(encoding="utf-8")


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
  int get_data() { locked_bits_ = dib_bits_; return locked_bits_ != 0; }
  void free_data(int) { locked_bits_ = 0; }
  int copy(Buffer *, int, int, int, int, int, int);
};
// ACTUAL_COPY

static Buffer make(void *bits, int width, int height, int stride) {
  Buffer b{}; b.dib_.bmiHeader.biWidth=width;
  b.dib_.bmiHeader.biHeight=-height; b.rect1_={0,0,width,height};
  b.dib_bits_=bits; b.stride_=stride; return b;
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


def test_only_the_seven_copy_pmf_sites_are_restored_to_direct_calls():
    assert "return copy(buffer, xCoord, yCoord, xCoord, yCoord, width, height);" in BUFFER_CPP
    assert "return copy(buffer, left, top, left, top," in BUFFER_CPP
    assert "source->copy(this, 0, 0, colX, rowY, tileWidth," in BUFFER_CPP
    assert "buffer->copy(&buffer_, origin_x_, origin_y_, 0, 0, decoded_width_," in FLIC_CPP
    assert "buffer_.copy(buffer2_, 0, 0, origin_x_, origin_y_," in FLIC_CPP
    assert FLIC_CPP.count("buffer_.copy(buffer2_, 0, 0, origin_x_, origin_y_,") == 2
    assert "buffer2_->copy(&buffer_, origin_x_, origin_y_, 0, 0, decoded_width_," in FLIC_CPP
    assert "int (Buffer::*copy)" not in BUFFER_CPP
    assert "int (Buffer::*copy)" not in FLIC_CPP
    # Unrelated measured PMFs remain in place.
    assert "void *(Buffer::*get_data)() = &Buffer::get_data;" in FLIC_CPP
    assert "(this->*&Flic::close)()" in FLIC_CPP


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
