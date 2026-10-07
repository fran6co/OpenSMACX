"""Guard and execute the live ordered 005D2037 packer implementation."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())
SOURCE = ROOT / "src/general.cpp"


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
    index_read = definition(text, "static int movie_blend_read_index(")
    word_read = definition(text, "static DWORD movie_blend_read_word(")
    low_blend = definition(text, "static DWORD movie_blend_low_half_ordered(")
    high_blend = definition(text, "static DWORD movie_blend_high_half_ordered(")
    body = definition(text, "void __cdecl sub_5d2037(int *data_row,")
    assert "const volatile int *" in index_read
    assert "const volatile DWORD *" in word_read
    order = (
        "source_index_0", "selector_index_0", "selector_index_1",
        "source_value_0", "selector_value_0", "selector_value_1",
        "low_half", "source_index_1", "source_value_1", "high_half",
        "memcpy(&packed_output[index]",
    )
    positions = [body.index(token) for token in order]
    assert positions == sorted(positions)
    assert "MovieBlendTableMid" in low_blend and "MovieBlendTableLow" in low_blend
    assert low_blend.index("const DWORD high") < low_blend.index("const DWORD middle") < low_blend.index("const DWORD low")
    assert high_blend.index("const DWORD high") < high_blend.index("const DWORD low") < high_blend.index("const DWORD middle")
    return "\n".join((index_read, word_read, low_blend, high_blend, body))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_5d2037_access_order_and_values(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = live_source()
    fixture = r'''#include <cassert>
#include <stdint.h>
#include <string.h>
typedef uint32_t DWORD;
#define __cdecl
static DWORD data_values[512], quant_values[512], quant2_values[512];
static DWORD high_values[512], middle_values[512], low_values[512];
static DWORD *MovieBlendTableData=data_values;
static DWORD *MovieBlendTableQuant=quant_values;
static DWORD *MovieBlendTableQuant2=quant2_values;
static DWORD *MovieBlendTableHi=high_values;
static DWORD *MovieBlendTableMid=middle_values;
static DWORD *MovieBlendTableLow=low_values;
''' + bodies + r'''
int main() {
  int data[16], selectors[0x48]; unsigned int output[10];
  for(int i=0;i<512;++i){
    data_values[i]=i*17u+3u; quant_values[i]=i*29u+5u;
    quant2_values[i]=i*37u+11u; high_values[i]=i*0x10001u;
    middle_values[i]=i*0x101u; low_values[i]=i*0x10001u+7u;
  }
  for(int i=0;i<16;++i)data[i]=(i*7)%32;
  for(int i=0;i<0x48;++i)selectors[i]=(i*11)%32;
  for(int i=0;i<10;++i)output[i]=0xdeadbeefu;
  sub_5d2037(data,selectors,output+1);
  assert(output[0]==0xdeadbeefu&&output[9]==0xdeadbeefu);
  for(int i=0;i<8;++i){
    DWORD left=(data_values[data[2*i]]+quant2_values[selectors[0x40+i]])+
               quant_values[selectors[i]];
    DWORD right=(data_values[data[2*i+1]]+quant2_values[selectors[0x40+i]])+
                quant_values[selectors[i]];
    DWORD expected=(high_values[right>>23]|low_values[right&0x1ffu]|
                    middle_values[(right&0xff800u)>>11])&0xffff0000u;
    expected|=(high_values[left>>23]|middle_values[(left&0xff800u)>>11]|
               low_values[left&0x1ffu])&0xffffu;
    assert(output[i+1]==expected);
  }
}
'''
    with tempfile.TemporaryDirectory(prefix="movie-live-5d2037-") as temp:
        source = Path(temp) / "control.cpp"
        binary = Path(temp) / "control"
        source.write_text(fixture)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
             str(source), "-o", str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True)
        assert ran.returncode == 0, ran.stdout + ran.stderr
