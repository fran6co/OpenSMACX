"""Actual palette blending with bounded signed inputs and an unsigned mutant.

No overflow, zero divisor or invalid indices are executed. Original machine
overflow effects and runtime input bounds remain recovery obligations.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", (ROOT / "src/jackal/palette.cpp").read_text(), flags=re.S)
    signature = "int Palette::make_blend_table("
    assert text.count(signature) == 1
    start = text.index(signature)
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated product definition")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
struct PALETTEENTRY { uint8_t peRed,peGreen,peBlue,peFlags; };
static int initialized;
static void *PaletteInitialized=&initialized;
struct RGB { int r,g,b; };
std::vector<RGB> queries;
struct Palette {
    PALETTEENTRY entries_[256];
    int make_blend_table(const Palette *,uint8_t *,int,int,int,int);
    int make_blend_table_unsigned(const Palette *,uint8_t *,int,int,int,int);
    int get_nearest_palette_index(uint8_t r,uint8_t g,uint8_t b,int skip) {
        assert(skip==0); queries.push_back({r,g,b}); return 300;
    }
};
// BODY
// MUTANT
int main() {
    Palette a={},other={};
    for (int i=0;i<256;++i) {
        a.entries_[i]={10,30,80,123}; other.entries_[i]={40,90,20,234};
    }
    const int weights[][2]={{1,1},{2,-1},{-2,1},{3,2}};
    for (const auto &pair : weights) {
        const int wa=pair[0],wb=pair[1],divisor=wa+wb;
        uint8_t table[256]; std::memset(table,0xCC,sizeof table); queries.clear();
        assert(a.make_blend_table(&other,table,3,2,wa,wb)==0);
        assert(queries.size()==2);
        // Independent wide signed arithmetic stays in the 32-bit defined domain.
        const int r=uint8_t((40LL*wb+10LL*wa)/divisor);
        const int g=uint8_t((90LL*wb+30LL*wa)/divisor);
        const int b=uint8_t((20LL*wb+80LL*wa)/divisor);
        for (const RGB &q : queries) assert(q.r==r && q.g==g && q.b==b);
        for (int i=0;i<256;++i) assert(table[i]==(i==3||i==4 ? uint8_t(300) : uint8_t(i)));
        // Only private entry copies may change; both source palettes survive.
        for (int i=0;i<256;++i) {
            assert(a.entries_[i].peRed==10 && a.entries_[i].peGreen==30 && a.entries_[i].peBlue==80 && a.entries_[i].peFlags==123);
            assert(other.entries_[i].peRed==40 && other.entries_[i].peGreen==90 && other.entries_[i].peBlue==20 && other.entries_[i].peFlags==234);
        }
    }
    uint8_t table[256]; queries.clear();
    assert(a.make_blend_table(&other,table,3,1,-2,1)==0);
    RGB signed_result=queries.back(); queries.clear();
    assert(a.make_blend_table_unsigned(&other,table,3,1,-2,1)==0);
    assert(queries.back().r!=signed_result.r && queries.back().g!=signed_result.g);
    // Original guard order: other, initialized, then destination table.
    queries.clear(); std::memset(table,0xCC,sizeof table);
    PaletteInitialized=0;
    assert(a.make_blend_table(0,table,3,2,1,1)==0x10);
    assert(a.make_blend_table(&other,0,3,2,1,1)==7);
    PaletteInitialized=&initialized;
    assert(a.make_blend_table(&other,0,3,2,1,1)==0x10);
    assert(queries.empty());
    for (uint8_t byte : table) assert(byte==0xCC);
    // An empty blend range fills identities without invoking the search.
    assert(a.make_blend_table(&other,table,3,0,1,1)==0 && queries.empty());
    for (int i=0;i<256;++i) assert(table[i]==uint8_t(i));
}
"""


@pytest.mark.parametrize("compiler", tuple(filter(None, (shutil.which("g++"), shutil.which("clang++")))) or (None,))
def test_actual_palette_signed_blend_and_unsigned_mutant(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = actual_body()
    mutant, count = re.subn(r"((?:otherCopy|selfCopy)\[i\]\.pe(?:Red|Green|Blue))(?= \*)",
                            r"static_cast<unsigned int>(\1)", body)
    assert count == 6 and mutant != body
    mutant = mutant.replace("Palette::make_blend_table(", "Palette::make_blend_table_unsigned(", 1)
    source = tmp_path / "palette.cpp"
    binary = tmp_path / "palette"
    source.write_text(HARNESS.replace("// BODY", body).replace("// MUTANT", mutant))
    subprocess.run([compiler, "-std=c++11", "-O2", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=30)
    subprocess.run([str(binary)], check=True, capture_output=True, timeout=10)
