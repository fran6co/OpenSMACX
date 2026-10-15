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


def product(damage=None):
    source = clean((ROOT / "src/sprite.cpp").read_text())
    if damage is not None:
        name, old, new = damage
        match = re.search(r"^(?:void|int\s*\*)\s*Sprite::" + name + r"\([^;{]*\)\s*\{", source, re.M)
        assert match
        body = braced(source, match.start())
        assert body.count(old) == 1
        source = source[:match.start()] + body.replace(old, new, 1) + source[match.start() + len(body):]
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
    matches = re.finditer(r"^(?:void|int \*)\s*Sprite::(?:fill_scaling_table|get_clipped_[xy]_table_(?:expand|shrink))"
        r"\([^;{]*\)\s*\{", source, re.M)
    bodies = [braced(source, m.start()) for m in matches]
    assert len(bodies) == 5
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
// Win32 RECT coordinates are signed 32-bit LONGs; int* output alias cases
// below are therefore type-correct for this host harness.
struct RECT { int left,top,right,bottom; };
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
void reversed_shrink(bool y) {
    RECT full={0,0,100,100};
    RECT rect=y?RECT{0,2,1,-1}:RECT{2,0,-1,1};
    for(int i=0;i<800;++i) SpriteClippedXSteps[i]=SpriteClippedYSteps[i]=-91;
    int a=-11,b=-12,c=-13,d=-14;
    int *result=clip(y,false,&full,&rect,&a,&b,&c,&d);
    int *table=y?SpriteClippedYSteps:SpriteClippedXSteps;
    check(result==table);
    check(a==2 && b==SpriteScalingSteps[0]+SpriteScalingSteps[1]);
    check(c==-3 && d==0);
    for(int i=0;i<800;++i) check(table[i]==-91);
    guards();
}
void reversed_partial_head_expand(bool y) {
    fill(2,1,{2});
    RECT full={0,0,100,100};
    RECT rect=y?RECT{0,1,1,0}:RECT{1,0,0,1};
    for(int i=0;i<800;++i) SpriteClippedXSteps[i]=SpriteClippedYSteps[i]=-91;
    int source_skip=-11,destination_skip=-12,source_count=-13,destination_count=-14;
    int *result=clip(y,true,&full,&rect,&source_skip,&destination_skip,
                     &source_count,&destination_count);
    int *table=y?SpriteClippedYSteps:SpriteClippedXSteps;
    check(result==table);
    check(source_skip==0 && destination_skip==1);
    check(source_count==1 && destination_count==-1);
    // This original partial-head path's output[-1] lands on table[0]. Avoid
    // skip=0 with negative extent: that path can address before the table.
    check(table[0]==-1);
    for(int i=1;i<800;++i) check(table[i]==-91);
    guards();
}
void zero_extent_alias(bool y,bool expand) {
    RECT full={0,0,100,100};
    RECT rect=y?RECT{0,4,1,4}:RECT{4,0,4,1};
    int *table=y?SpriteClippedYSteps:SpriteClippedXSteps;
    table[0]=-73;
    int *result=clip(y,expand,&full,&rect,&table[0],&table[0],&table[0],&table[0]);
    check(result==0 && table[0]==-73);
    guards();
}
void rect_output_alias_reads_before_writes() {
    fill(5,2,{2,3});
    RECT full={0,0,100,100}, rect={1,0,4,1};
    int destination_skip=-12,source_count=-13,destination_count=-14;
    int *result=sprite.get_clipped_x_table_expand(&full,&rect,&rect.left,
        &destination_skip,&source_count,&destination_count);
    check(result==SpriteClippedXSteps);
    // Fetch both rectangle-derived inputs before source_skip overwrites left.
    check(rect.left==0 && destination_skip==1 && source_count==2 && destination_count==3);
    check(SpriteClippedXSteps[0]==1 && SpriteClippedXSteps[1]==2);
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
    fill(5,2,{2,3});
    for(int y=0;y<2;++y) {
        reversed_shrink(y);
        reversed_partial_head_expand(y);
        zero_extent_alias(y,true);
        zero_extent_alias(y,false);
    }
    rect_output_alias_reads_before_writes();
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


@pytest.mark.parametrize("method,old,new", [
    ("fill_scaling_table", "static_cast<uint32_t>(larger));", "static_cast<uint32_t>(smaller));"),
    ("fill_scaling_table", "if (error > 0)", "if (error >= 0)"),
    ("fill_scaling_table", "i < SpriteScalingTableCount", "i < SpriteScalingTableCount - 1"),
    ("get_clipped_x_table_expand", "--index;", "index -= 0;"),
    ("get_clipped_x_table_expand", "if (extent < 0) output[-1] += extent;", "if (extent < 0) output[-1] -= extent;"),
    ("get_clipped_x_table_shrink", "*source_skip += SpriteScalingSteps[index];", "*source_skip += 1;"),
    ("get_clipped_x_table_shrink", "SpriteClippedXSteps[i] = step;\n        *source_count += step;", "*source_count += step;\n        SpriteClippedXSteps[i] = step;"),
    ("get_clipped_y_table_shrink", "return SpriteClippedYSteps;", "return SpriteClippedXSteps;"),
])
def test_scaling_damage(tmp_path,method,old,new):
    body=product((method,old,new))
    result=run(tmp_path,body)
    assert result.returncode == 91, result.stdout+result.stderr


def test_signed_fill_overflow_damage(tmp_path):
    body = product()
    old = "twice_divisor = static_cast<int>(static_cast<uint32_t>(smaller) * 2u);"
    assert body.count(old) == 1
    result = run(tmp_path, body.replace(old, "twice_divisor = smaller * 2;"))
    # Here sanitizer rejection is specifically the property under test. The
    # intact body passes the same literal large-input expectations above.
    assert result.returncode != 0 and "signed integer overflow" in result.stderr

def test_nonpositive_extent_safety_mutation(tmp_path):
    old = "if (extent == 0) return 0;"
    for method in ("get_clipped_x_table_expand", "get_clipped_x_table_shrink"):
        body = product((method, old, "if (extent <= 0) return 0;"))
        # Negative shrink and partial-head negative expansion retain the
        # original behavior; reject this safety-normalization mutant.
        result = run(tmp_path, body)
        assert result.returncode == 91, result.stdout+result.stderr

def test_zero_extent_write_mutation(tmp_path):
    old = "if (extent == 0) return 0;"
    changed = "if (extent == 0) { *destination_count = extent; return 0; }"
    for method in ("get_clipped_x_table_expand", "get_clipped_x_table_shrink"):
        body = product((method, old, changed))
        result = run(tmp_path, body)
        assert result.returncode == 91, result.stdout+result.stderr
