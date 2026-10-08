"""Actual expression controls, not complete terrain/checksum/clip-body proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated source body")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_source_owned_stride_and_clip_arithmetic(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/mapwin.cpp").read_text()
    stride = re.findall(r"iVar5 = ([^;\n]*MapLongitude[^;\n]*);", source)[0]
    clip = braced(source, "int MapWin::compute_clip(")
    statement, = re.findall(r"    rect.top = [^;\n]*field_1DDC0_[^;\n]*;", clip)
    bottom, = re.findall(r"        rect.bottom = [^;\n]*dib_[^;\n]*;", clip)
    helpers = "\n".join(braced(source, signature) for signature in
                        ("int map_dest_signed(uint32_t value) {",
                         "int mapwin_dest_sub(int left, int right) {"))
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <climits>
#include <type_traits>
HELPERS
struct Rect { int top, bottom; };
struct { struct { int biHeight; } bmiHeader; } dib_;
int main() {
    static_assert(sizeof(int)==4, "target dword width");
    for (uint32_t MapLongitude : {uint32_t(4),uint32_t(-4)}) {
        for (int a8 : {-3,-1,0,1,3}) {
            int local_14=1;
            auto index = STRIDE;
            static_assert(std::is_same<decltype(index),int>::value,
                          "preserve signed intermediate indexing");
            int expected = (MapLongitude==4 ? 4 : -4)*a8+1;
            assert(index==expected);
        }
    }
    for (int top : {0,1,-1,INT_MIN,INT_MAX}) {
        for (uint32_t field_1DDC0_ : {uint32_t(0),uint32_t(1),
             uint32_t(0xffffffff),uint32_t(0x80000000),uint32_t(0x7fffffff)}) {
            Rect rect={top,0};
STATEMENT
            assert(static_cast<uint32_t>(rect.top)==
                   static_cast<uint32_t>(top)-field_1DDC0_);
            dib_.bmiHeader.biHeight=top;
BOTTOM
            assert(static_cast<uint32_t>(rect.bottom)==
                   uint32_t(0)-static_cast<uint32_t>(top));
        }
    }
}
'''
    fixture = "#include <initializer_list>\n" + fixture
    path = tmp_path / "owned.cpp"
    path.write_text(fixture.replace("HELPERS", helpers).replace("STRIDE", stride)
                    .replace("STATEMENT", statement).replace("BOTTOM", bottom))
    binary = tmp_path / "owned"
    subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined",
                    "-fno-sanitize-recover=all", str(path), "-o", str(binary)],
                   check=True, capture_output=True)
    subprocess.run([str(binary)], check=True, capture_output=True)
