"""Guard and execute the live ordered 005CF5C7/005CF852 source bodies."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/movie_media_decoder.cpp").is_file())
SOURCE = ROOT / "src/movie_media_decoder.cpp"


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated live source definition: " + signature)


def live_source():
    text = SOURCE.read_text()
    helper = definition(text, "static DWORD movie_pixel_read_dword_address(")
    offset_helper = definition(text, "static DWORD movie_pixel_read_dword_at(")
    row = definition(text, "void __cdecl OutputPixelRow(MediaConversionTableStorage &tables,")
    row16 = definition(text, "void __cdecl OutputPixelRow16(MediaConversionTableStorage &tables,")
    assert "const volatile DWORD *const word" in helper
    assert "const volatile DWORD *" in offset_helper or "movie_pixel_read_dword_address(address)" in offset_helper
    for body in (row, row16):
        assert body.count("if (pair == 0 || pair == 4)") == 1
        assert "discarded_destination" in body
        assert body.index("discarded_destination") < body.index("component_index_0")
        assert "selector_zero = tables.packed_component_words + 256" in body
        assert "selector_one = tables.packed_component_words + 128" in body
        ordered = (
            "component_index_0", "component_index_1", "selector_index_0",
            "selector_index_1", "component_0", "component_1",
            "selector_0", "selector_1", "pixel_0", "pixel_1",
        )
        positions = [body.index(name) for name in ordered]
        assert positions == sorted(positions)
    assert "movie_pixel_read_dword_at(" in row and "destination" in row
    assert "movie_pixel_read_dword_address(destination_address)" in row16
    return "\n".join((helper, offset_helper, row, row16))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None, "swapped_banks", "missing_odd_word"])
def test_live_pixel_row_order_and_values(compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = live_source()
    if mutation == "swapped_banks":
        bodies = bodies.replace("packed_component_words + 256", "packed_component_words + BANK")
        bodies = bodies.replace("packed_component_words + 128", "packed_component_words + 256")
        bodies = bodies.replace("packed_component_words + BANK", "packed_component_words + 128")
    elif mutation == "missing_odd_word":
        old = "memcpy(&destination[pair * 2 + 1], &pixel_1, sizeof(pixel_1));"
        left, right = bodies.rsplit(old, 1)
        bodies = left + "(void)pixel_1;" + right
    fixture = r'''#include <cassert>
#include <stdint.h>
#include <string.h>
typedef uint32_t DWORD;
typedef unsigned char BYTE;
#define __cdecl
struct MediaConversionTableStorage { DWORD packed_component_words[384]; };
''' + bodies + r'''
int main() {
  MediaConversionTableStorage tables = {};
  uint32_t component[16], chroma[0x48];
  int live32[20]; short live16[20];
  // Nonlinear contents distinguish swapped banks: a single affine pattern
  // makes their sum invariant under swapping the two selector sources.
  for (DWORD i=0;i<384;++i) tables.packed_component_words[i]=
      (i*0x9e3779b9u) ^ (i*i*37u);
  for (int i=0;i<16;++i) component[i]=(i*7u%128u)<<17;
  for (int i=0;i<0x48;++i) chroma[i]=(i*11u%128u)<<17;
  for (int i=0;i<20;++i) { live32[i]=0x13579bdf; live16[i]=(short)0x2468; }
  OutputPixelRow(tables,live32+1,component,chroma);
  OutputPixelRow16(tables,live16+1,component,chroma);
  assert(live32[0]==0x13579bdf && live32[17]==0x13579bdf && live32[19]==0x13579bdf);
  assert(live16[0]==(short)0x2468 && live16[17]==(short)0x2468 && live16[19]==(short)0x2468);
  for (int pair=0;pair<8;++pair) {
    DWORD c0=component[pair*2]>>17, c1=component[pair*2+1]>>17;
    DWORD selector0=chroma[pair]>>17, selector1=chroma[0x40+pair]>>17;
    DWORD v0=tables.packed_component_words[c0]+tables.packed_component_words[256+selector0]+tables.packed_component_words[128+selector1];
    DWORD v1=tables.packed_component_words[c1]+tables.packed_component_words[256+selector0]+tables.packed_component_words[128+selector1];
    DWORD actual0, actual1; unsigned short actual16, actual16_second;
    memcpy(&actual0,&live32[1+pair*2],4); memcpy(&actual1,&live32[2+pair*2],4);
    memcpy(&actual16,&live16[1+pair*2],2);
    memcpy(&actual16_second,&live16[2+pair*2],2);
    assert(actual0==v0 && actual1==v1 && actual16==(unsigned short)v0 &&
           actual16_second==(unsigned short)v1);
  }
}
'''
    with tempfile.TemporaryDirectory(prefix="movie-live-pixel-row-") as temp:
        source = Path(temp) / "control.cpp"
        binary = Path(temp) / "control"
        source.write_text(fixture)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
             str(source), "-o", str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        if mutation is None:
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "incorrect pixel-row mutant escaped the oracle"
