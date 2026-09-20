"""Actual scaling bodies with bounded host arithmetic/alias controls.

The receiver is an explicit inert test double: original instructions never
access Sprite fields. This is not Sprite lifetime/layout or full blitter proof.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin

ROOT = Path(__file__).resolve().parents[2]


def clean(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unclosed product body")


def product():
    source = clean((ROOT / "src/sprite.cpp").read_text())
    header = clean((ROOT / "src/sprite.h").read_text())
    enum, = re.findall(r"enum\s*\{\s*SpriteScalingTableCount\s*=\s*\d+\s*\};", header)
    globals_ = re.findall(r"^(?:int|float) Sprite(?:ScaleNumerator|ScaleDenominator|CachedScaleNumerator|"
        r"CachedScaleDenominator|ScalingRatio|MinimumScaledWidth|MinimumScaledHeight|"
        r"ScalingSteps|ClippedXSteps|ClippedYSteps)[^;]*;", source, re.M)
    assert len(globals_) == 10
    names = ["fill_scaling_table", "get_clipped_x_table_expand", "get_clipped_y_table_expand",
             "get_clipped_x_table_shrink", "get_clipped_y_table_shrink"]
    declarations = []
    for name in names:
        match, = re.findall(r"(?:void|int\s*\*)\s*" + name + r"\([^;{}]*\);", header)
        declarations.append(match)
    matches = re.finditer(r"^(?:static int \*sprite_clip_(?:expand|shrink)|"
        r"(?:void|int \*)\s*Sprite::(?:fill_scaling_table|get_clipped_[xy]_table_(?:expand|shrink)))"
        r"\([^;{]*\)\s*\{", source, re.M)
    bodies = [braced(source, m.start()) for m in matches]
    assert len(bodies) == 7
    return enum + "\n" + "\n".join(globals_) + "\nclass Sprite { public: unsigned char guard[44];\n" + \
        "\n".join(declarations) + "\n};\n" + "\n".join(bodies)


def test_original_scaling_and_table_evidence():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    def decode(a, b):
        return [(i.mnemonic, i.op_str) for i in decoder.disasm(image.read(Span(a, b)), a)]
    assert ("mov", "ecx, 0x320") in decode(0x5EB25F, 0x5EB270)
    assert ("mov", "edi, 0x9b3ad8") in decode(0x5EB25F, 0x5EB270)
    assert ("cmp", "ecx, 0x9b4758") in decode(0x5EB285, 0x5EB290)
    assert decode(0x5EB23C,0x5EB249) == [("mov","esi, edx"),
        ("lea","edx, [ecx + ecx]"),("shl","esi, 1"),("mov","eax, esi"),
        ("sar","eax, 1"),("sub","eax, edx")]
    assert decode(0x5EB258,0x5EB25A) == [("add","eax, edi")]
    assert decode(0x5EB22B,0x5EB230) == [("cmp","ecx, 1"),("jle","0x5eb24b")]
    for lo, hi, address in [(0x5EAB0F,0x5EABB7,0x9B53EC), (0x5EABBF,0x5EAC69,0x9B4760),
                            (0x5EAC6F,0x5EACFB,0x9B53EC), (0x5EACFF,0x5EAD8D,0x9B4760)]:
        instructions = decode(lo, hi)
        assert instructions[-1] == ("ret", "0x18")
        assert ("mov", "eax, " + hex(address)) in instructions[-5:]
    # These independent next-object references exclude a guessed array starting
    # at the fill's end, or a clipped table that swallowed neighboring storage.
    assert decode(0x5E52EB,0x5E52F1) == [("mov", "dword ptr [0x9b4758], edx")]
    assert decode(0x5E3F80,0x5E3F86) == [("mov", "dword ptr [0x9b53e0], edx")]
    assert decode(0x5E503C,0x5E5041) == [("mov", "dword ptr [0x9b606c], eax")]
    for lo, hi in [(0x9B3AD8,0x9B4758),(0x9B4760,0x9B53E0),(0x9B53EC,0x9B606C)]:
        assert hi-lo == 800*4 and image.read_initial(Span(lo,hi)) == bytes(800*4)
    assert image.read_initial(Span(0x696D18,0x696D28)) == bytes.fromhex("0100000001000000ffffffffffffffff")
    assert image.read_initial(Span(0x9B661C,0x9B6628)) == bytes(12)
    # Stored float ratio is N / D, not independent X/Y positioning.
    assert decode(0x5E4C69,0x5E4C7B) == [("fild","dword ptr [0x696d18]"),
        ("fidiv","dword ptr [0x696d1c]"),("fstp","dword ptr [0x9b6624]")]


HARNESS = r"""
#include <cstring>
#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>
struct RECT { long left,top,right,bottom; };
void check_at(bool condition,int line) {
    if(!condition) { std::fprintf(stderr,"behavior assertion line %d\n",line); std::exit(91); }
}
#define check(x) check_at((x),__LINE__)
// PRODUCT
Sprite sprite;
void guards() { for(unsigned i=0;i<sizeof(sprite.guard);++i) check(sprite.guard[i]==0xA7); }
void fill(int numerator,int denominator,const std::vector<int> &pattern,int prefix=0) {
    SpriteScaleNumerator=numerator; SpriteScaleDenominator=denominator;
    SpriteCachedScaleNumerator=71; SpriteCachedScaleDenominator=72;
    SpriteScalingRatio=0.375f; SpriteMinimumScaledWidth=41; SpriteMinimumScaledHeight=42;
    for(int i=0;i<800;++i) SpriteClippedXSteps[i]=SpriteClippedYSteps[i]=-91;
    sprite.fill_scaling_table();
    for(int i=0;i<800;++i) {
        check(SpriteScalingSteps[i]==(i<prefix?4:pattern[(i-prefix)%pattern.size()]));
        check(SpriteClippedXSteps[i]==-91 && SpriteClippedYSteps[i]==-91);
    }
    check(SpriteCachedScaleNumerator==71 && SpriteCachedScaleDenominator==72);
    check(SpriteScaleNumerator==numerator && SpriteScaleDenominator==denominator);
    check(SpriteScalingRatio==0.375f && SpriteMinimumScaledWidth==41 && SpriteMinimumScaledHeight==42);
    guards();
}
int *clip(bool y,bool expand,RECT *full,RECT *clipped,int *a,int *b,int *c,int *d) {
    if(y) return expand?sprite.get_clipped_y_table_expand(full,clipped,a,b,c,d):
        sprite.get_clipped_y_table_shrink(full,clipped,a,b,c,d);
    return expand?sprite.get_clipped_x_table_expand(full,clipped,a,b,c,d):
        sprite.get_clipped_x_table_shrink(full,clipped,a,b,c,d);
}
void clipping(bool y,bool expand,int skip,int length,bool alias=false) {
    RECT full={-17,-23,2000,2000};
    RECT rect=y?RECT{100,full.top+skip,103,full.top+skip+length}:
                RECT{full.left+skip,100,full.left+skip+length,103};
    for(int i=0;i<800;++i) SpriteClippedXSteps[i]=SpriteClippedYSteps[i]=-91;
    int a=-11,b=-12,c=-13,d=-14,all=-15;
    int *result=clip(y,expand,&full,&rect,alias?&all:&a,alias?&all:&b,alias?&all:&c,alias?&all:&d);
    if(length==0) {
        check(result==0 && a==-11 && b==-12 && c==-13 && d==-14 && all==-15);
        return;
    }
    int *table=y?SpriteClippedYSteps:SpriteClippedXSteps;
    int *other=y?SpriteClippedXSteps:SpriteClippedYSteps;
    check(result==table);
    std::vector<int> expected;
    if(expand) {
        // Independent pixel ownership oracle: expand whole source pixels,
        // slice the destination range, then count surviving equal owners.
        std::vector<int> owners;
        for(int i=0;i<800;++i) for(int j=0;j<SpriteScalingSteps[i];++j) owners.push_back(i);
        check(skip+length<=int(owners.size()));
        int previous=-1;
        for(int i=skip;i<skip+length;++i) {
            if(owners[i]!=previous) { expected.push_back(0); previous=owners[i]; }
            ++expected.back();
        }
        if(alias) check(all==int(expected.size()));
        else check(a==owners[skip] && b==skip && c==int(expected.size()) && d==length);
    } else {
        for(int i=skip;i<skip+length;++i) expected.push_back(SpriteScalingSteps[i]);
        int prefix=std::accumulate(SpriteScalingSteps,SpriteScalingSteps+skip,0);
        int consumed=std::accumulate(expected.begin(),expected.end(),0);
        if(alias) check(all==consumed);
        else check(a==skip && b==prefix && c==length && d==consumed);
    }
    for(unsigned i=0;i<expected.size();++i) check(table[i]==expected[i]);
    for(unsigned i=expected.size();i<800;++i) check(table[i]==-91);
    for(int i=0;i<800;++i) check(other[i]==-91);
    guards();
}
int main() {
    std::memset(sprite.guard,0xA7,sizeof(sprite.guard));
    check(SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
    check(SpriteCachedScaleNumerator==-1 && SpriteCachedScaleDenominator==-1);
    check(SpriteScalingRatio==0 && SpriteMinimumScaledWidth==0 && SpriteMinimumScaledHeight==0);
    fill(1,1,{1}); fill(1,4,{4}); fill(4,1,{4});
    fill(3,2,{2,1}); fill(2,3,{2,1}); fill(5,2,{2,3});
    fill(4,3,{2,1,1}); fill(8,3,{2,3,3}); fill(7,2,{3,4},3);
    fill(0,0,{0}); fill(0,2,{2}); fill(0,3,{4});
    // Literal consequences of original 32-bit ADD/SUB/SHL, not a second
    // implementation of the candidate's error recurrence. For MAX:(MAX-1),
    // doubled divisor is -4 and the odd-quotient adjustment wraps error to
    // -2147483644. All 800 following +2 steps remain negative: every entry 1.
    fill(INT_MAX,INT_MAX-1,{1}); fill(INT_MAX-1,INT_MAX,{1});
    // The smaller<=1 path does no division. Odd positive MAX keeps error
    // positive and its increment wraps the stored step to MIN on every pass.
    fill(INT_MAX,0,{INT_MIN}); fill(0,INT_MAX,{INT_MIN});
    fill(INT_MAX,INT_MIN,{INT_MIN});
    fill(-1,-3,{-1}); fill(-3,-2,{-2}); fill(-1,3,{4});
    fill(-1,1,{2}); fill(INT_MIN,INT_MIN,{INT_MIN});
    for(int y=0;y<2;++y) for(int expand=0;expand<2;++expand) {
        fill(3,2,{2,1});
        for(int skip=0;skip<9;++skip) for(int length=0;length<12;++length)
            clipping(y,expand,skip,length);
        clipping(y,expand,2,7,true);
        fill(1,1,{1}); clipping(y,expand,0,800); clipping(y,expand,799,1);
    }
    // Output accumulator aliases a clipped-table entry: every += must reload
    // after the table write, not use a cached sum.
    fill(5,2,{2,3});
    RECT full={0,0,100,100}, rect={0,0,3,3}; int a,b,c;
    sprite.get_clipped_x_table_shrink(&full,&rect,&a,&b,&c,&SpriteClippedXSteps[0]);
    check(SpriteClippedXSteps[0]==9 && SpriteClippedXSteps[1]==3 && SpriteClippedXSteps[2]==2);
}
"""


def run(tmp_path, body):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source, exe = tmp_path / "scaling.cpp", tmp_path / "scaling"
    source.write_text(HARNESS.replace("// PRODUCT", body))
    built = subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined",
        "-fno-sanitize-recover=undefined",str(source),"-o",str(exe)],capture_output=True,text=True,timeout=30)
    assert built.returncode == 0, built.stdout+built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)


def test_actual_scaling_bodies(tmp_path):
    result=run(tmp_path,product())
    assert result.returncode == 0, result.stdout+result.stderr


@pytest.mark.parametrize("old,new", [
    ("static_cast<uint32_t>(larger));", "static_cast<uint32_t>(smaller));"),
    ("if (error > 0)", "if (error >= 0)"),
    ("i < SpriteScalingTableCount", "i < SpriteScalingTableCount - 1"),
    ("--index;", "index -= 0;"),
    ("if (extent < 0) output[-1] += extent;", "if (extent < 0) output[-1] -= extent;"),
    ("*source_skip += SpriteScalingSteps[index];", "*source_skip += 1;"),
    ("table[i] = step;\n        *source_count += step;", "*source_count += step;\n        table[i] = step;"),
    ("destination_skip, source_skip, destination_count, source_count, SpriteClippedYSteps);",
     "destination_skip, source_skip, destination_count, source_count, SpriteClippedXSteps);"),
])
def test_scaling_damage(tmp_path,old,new):
    body=product(); assert body.count(old)==1
    result=run(tmp_path,body.replace(old,new))
    assert result.returncode == 91, result.stdout+result.stderr


def test_signed_fill_overflow_damage(tmp_path):
    body = product()
    old = "twice_divisor = static_cast<int>(static_cast<uint32_t>(smaller) * 2u);"
    assert body.count(old) == 1
    result = run(tmp_path, body.replace(old, "twice_divisor = smaller * 2;"))
    # Here sanitizer rejection is specifically the property under test. The
    # intact body passes the same literal large-input expectations above.
    assert result.returncode != 0 and "signed integer overflow" in result.stderr
