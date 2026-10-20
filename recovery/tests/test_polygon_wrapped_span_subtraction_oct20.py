"""The two actual polygon span writers preserve the original signed SUB branches.

The pinned routines use SUB/JLE both before and after clipping. JLE follows
signed operand ordering (including SUB overflow); the pixel loop decrements
the 32-bit SUB result until it reaches zero. The host fixture extracts the
complete production translation unit and invokes those real span bodies with
safe pixel storage and a bounded clip; it does not claim coverage of enormous
pixel ranges or native x87/ABI equivalence.
"""

import os
import shutil
import subprocess

import pytest

from test_polygon import actual_source


HARNESS = r"""
#include <cstdint>
#include <cstddef>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#define __cdecl
void require_at(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(89);}}
#define require(x) require_at((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
ACTUAL_VERTEX
unsigned char pixels[32],lookup[256];
const unsigned _MCW_EM=0x003f,_MCW_PC=0x0300,_MCW_RC=0x0c00,_MCW_IC=0x1000;
const unsigned _PC_64=0x0300,_PC_24=0,_RC_NEAR=0,_IC_PROJECTIVE=0,_RC_CHOP=0x0c00;
void _fpreset(){}
unsigned _control87(unsigned,unsigned){return 0;}
struct Buffer { ACTUAL_FIELDS void *get_data(){return pixels;} void free_data(int){} };
ACTUAL_SOURCE
void reset(){
    for(unsigned i=0;i<sizeof(pixels);++i)pixels[i]=(unsigned char)(i+3);
    for(unsigned i=0;i<256;++i)lookup[i]=(unsigned char)(i^0x55);
    PolygonPixels=pixels;PolygonY.coordinate=0;PolygonPitch=8;
    PolygonClipLeft=0;PolygonClipRight=8;PolygonColor=0xA7;PolygonRemap=lookup;
}
int main(){
    reset();
    unsigned char unchanged[sizeof(pixels)];std::memcpy(unchanged,pixels,sizeof(pixels));
    draw_solid_polygon_span(INT_MIN,INT_MAX);
    for(int x=0;x<8;++x)require(pixels[x]==0xA7);
    for(int x=8;x<32;++x)require(pixels[x]==unchanged[x]);
    std::memcpy(pixels,unchanged,sizeof(pixels));
    draw_remapped_polygon_span(INT_MIN,INT_MAX);
    for(int x=0;x<8;++x)require(pixels[x]==lookup[unchanged[x]]);
    for(int x=8;x<32;++x)require(pixels[x]==unchanged[x]);

    reset();std::memcpy(unchanged,pixels,sizeof(pixels));
    draw_solid_polygon_span(INT_MAX,INT_MIN);
    require(std::memcmp(pixels,unchanged,sizeof(pixels))==0);
    draw_remapped_polygon_span(INT_MAX,INT_MIN);
    require(std::memcmp(pixels,unchanged,sizeof(pixels))==0);

    reset();draw_solid_polygon_span(-2,5);
    for(int x=0;x<8;++x)require(pixels[x]==(x<5 ? 0xA7 : (unsigned char)(x+3)));

    reset();draw_remapped_polygon_span(-2,4);
    for(int x=0;x<8;++x)require(pixels[x]==(x<4 ? lookup[x+3] : (unsigned char)(x+3)));
    return 0;
}
"""


def compile_and_run(tmp_path, actual, fields, vertex):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    source = (HARNESS.replace("ACTUAL_FIELDS", fields)
             .replace("ACTUAL_VERTEX", vertex)
             .replace("ACTUAL_SOURCE", actual))
    cpp = tmp_path / "polygon_span.cpp"
    cpp.write_text(source)
    binary = tmp_path / "polygon_span"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_span_writers_keep_signed_sub_branches(tmp_path):
    actual, fields, vertex = actual_source()
    assert actual.count("right <= left") == 2
    assert actual.count(
        "unsigned int count = static_cast<unsigned int>(right)\n"
        "            - static_cast<unsigned int>(left);"
    ) == 2
    result = compile_and_run(tmp_path, actual, fields, vertex)
    assert result.returncode == 0, result.stderr


def test_wrapped_result_sign_guard_is_rejected(tmp_path):
    actual, fields, vertex = actual_source()
    before = "right <= left"
    assert actual.count(before) == 2
    damaged = actual.replace(
        before,
        "static_cast<int>(static_cast<unsigned int>(right) - "
        "static_cast<unsigned int>(left)) <= 0",
    )
    result = compile_and_run(tmp_path, damaged, fields, vertex)
    assert result.returncode == 89, result.stderr
