"""Live Sprite raster bodies with bounded pixel/API doubles and UBSan.

Pinned mono5E3E00/5E3E50, draw5E4B9A and dest5E5833 determine the
expectations. Scaling callees are explicit boundary doubles with independently
chosen small tables; shared count/pointer/pitch cells are extracted from source
and checked across successful, partial, and stale-state paths. This remains a
bounded three-renderer host control, not proof for the other five kernels,
native ABI, x87 precision, rendering-device behavior, shared-scratch
reentrancy, or whole-game correctness.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def definitions(source, name):
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    out = []
    for match in re.finditer(r"\bint Sprite::" + name + r"\([^;{}]*\)\s*\{", source):
        opening = source.index("{", match.start())
        depth = 0
        for end in range(opening, len(source)):
            depth += (source[end] == "{") - (source[end] == "}")
            if depth == 0:
                out.append(source[match.start():end + 1])
                break
    return out


def live_bodies():
    source = (ROOT / "src/sprite.cpp").read_text()
    mono = definitions(source, "draw_mono")
    plain = [body for body in definitions(source, "draw")
             if body[:body.index("{")].count(",") == 3]
    dest = [body for body in definitions(source, "draw_dest")
            if body[:body.index("{")].count(",") == 4]
    assert len(mono) == 2 and len(plain) == len(dest) == 1
    raster_cells = []
    for name in ("SpriteRasterDestinationColumnCount", "SpriteRasterSourceColumnCount",
                 "SpriteRasterDestinationRowCount", "SpriteRasterSourceRowCount",
                 "SpriteRasterXTable", "SpriteRasterYTable",
                 "SpriteRasterDestinationPitch", "SpriteRasterSourcePitch"):
        match, = re.findall(r"^(?:int\s+|int\s*\*\s*)" + name + r";\s*$", source, re.M)
        raster_cells.append(match)
    remap_union, = re.findall(r"(?s)^union SpriteRasterRemapPointer \{.*?^\};\s*$", source, re.M)
    remap_global, = re.findall(r"^SpriteRasterRemapPointer SpriteRasterRemapTable;\s*$", source, re.M)
    raster_cells.extend((remap_union, remap_global))
    return "\n".join(raster_cells + mono + plain + dest)


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
void check_at(bool c,int line) { if (!c) { std::fprintf(stderr,"check line %d\\n",line); std::exit(91); } }
#define check(c) check_at((c),__LINE__)
struct RECT { int32_t left, top, right, bottom; };
bool same(RECT a, RECT b) { return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom; }
int SpriteScaleNumerator=1, SpriteScaleDenominator=1;
int SpriteCachedScaleNumerator=-1, SpriteCachedScaleDenominator=-1;
float SpriteScalingRatio=0;
int SpriteMinimumScaledWidth=0, SpriteMinimumScaledHeight=0;
std::string events;
int which, scenario, lock_result=1, load_result=0, clip_calls=0, helper_failure=0;
int expected_scale_n, expected_scale_d, expected_key, expected_x, expected_y, expected_color;
bool change_clip=false;
RECT original_expected, clipped_expected;
unsigned char input[4096], output[1024], expected[1024], remap[256];
struct Buffer {
    RECT rect1_; uint32_t stride_; void *locked_bits_;
    void *get_data() { events+='L'; return lock_result ? locked_bits_ : nullptr; }
    void free_data(int n) { check(n==1); events+='U'; }
    int load_pcx(const char *name, void *palette, int start, int count);
    void close() { events+='C'; }
    int draw_mono(Buffer*,int,int,int,int,int,int);
    int draw(Buffer*,int,int,int,int,int);
} destination, g_BUFFER;
struct Sprite {
    char *ppszFileName_; unsigned char *pcBits_; unsigned char cTransparentIndex_;
    int iSpriteWidth2_,iSpriteWidth_,iSpriteHeight_,iWidth_,iHeight_,iLeftOffset_,iTopOffset_;
    int draw_mono(Buffer*,int,int,int,int);
    int draw_mono(Buffer*,int,int,int,int,int,int);
    int draw(Buffer*,int,int,int);
    int draw_dest(Buffer*,int,int,int,unsigned char*);
    void fill_scaling_table() { events+='F'; }
    int *get_clipped_x_table_shrink(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_y_table_shrink(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_x_table_expand(RECT*,RECT*,int*,int*,int*,int*);
    int *get_clipped_y_table_expand(RECT*,RECT*,int*,int*,int*,int*);
} sprite;
struct Axis { int source_skip,destination_skip,source_count,destination_count,steps[8]; } axis[2];
int *axis_call(int a, bool expand, RECT *bounds,RECT *clip,int *one,int *two,int *three,int *four) {
    events += a ? 'Y' : 'X';
    check(same(*bounds,original_expected) && same(*clip,clipped_expected));
    check(expand == (SpriteScaleNumerator>=SpriteScaleDenominator));
    if (helper_failure==a+1) return nullptr;
    Axis &v=axis[a];
    *one=expand?v.source_skip:v.destination_skip;
    *two=expand?v.destination_skip:v.source_skip;
    *three=expand?v.source_count:v.destination_count;
    *four=expand?v.destination_count:v.source_count;
    return v.steps;
}
#define AXIS_METHOD(NAME,A,E) int *Sprite::NAME(RECT *o,RECT*c,int*a,int*b,int*d,int*e) { check(this==&sprite); return axis_call(A,E,o,c,a,b,d,e); }
AXIS_METHOD(get_clipped_x_table_shrink,0,false)
AXIS_METHOD(get_clipped_y_table_shrink,1,false)
AXIS_METHOD(get_clipped_x_table_expand,0,true)
AXIS_METHOD(get_clipped_y_table_expand,1,true)
int IntersectRect(RECT *out,const RECT *a,const RECT *b) {
    events+='I'; ++clip_calls;
    check(b==&destination.rect1_ && same(*a,original_expected));
    check((clip_calls==1)==(out==a));
    RECT result={std::max(a->left,b->left),std::max(a->top,b->top),std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
    if(result.left>=result.right || result.top>=result.bottom) { *out={0,0,0,0}; return 0; }
    *out=result;
    if(change_clip && clip_calls==1) destination.rect1_=clipped_expected;
    return 1;
}
int Buffer::load_pcx(const char *name,void *palette,int first,int count) {
    check(this==&g_BUFFER && !std::strcmp(name,"owned.pcx") && !palette && first==10 && count==236);
    events+='P'; SpriteScaleNumerator=3; SpriteScaleDenominator=2; return load_result;
}
int delegate(Buffer *self,Buffer *to,int key,int x,int y,int color,int n,int d) {
    check(self==&g_BUFFER && to==nullptr && key==expected_key && x==expected_x && y==expected_y);
    check(color==expected_color && n==3 && d==2); events+='D'; return -17;
}
int Buffer::draw_mono(Buffer *to,int k,int x,int y,int c,int n,int d) { return delegate(this,to,k,x,y,c,n,d); }
int Buffer::draw(Buffer *to,int k,int x,int y,int n,int d) { return delegate(this,to,k,x,y,expected_color,n,d); }
// LIVE_BODIES
int paint(Buffer *to,int key,int x,int y,int color) {
    if(which==0) return sprite.draw_mono(to,key,x,y,color);
    if(which==1) return sprite.draw(to,key,x,y);
    return sprite.draw_dest(to,key,x,y,remap);
}
void one(int x,int y,unsigned char sample,int key,int color) {
    if(sample==static_cast<unsigned char>(key)) return;
    unsigned char &v=expected[y*16+x];
    v=which==0?static_cast<unsigned char>(color):which==1?sample:sample==0?remap[v]:sample;
}
void block(int x,int y,int w,int h,unsigned char sample,int key,int color) {
    if(sample==static_cast<unsigned char>(key)) return;
    // Original expansion resolves zero's remap ONCE for the entire run,
    // using its first destination byte; all pixels intentionally differ.
    unsigned char value=which==0?static_cast<unsigned char>(color):which==1?sample:sample==0?remap[expected[y*16+x]]:sample;
    for(int j=0;j<h;++j) for(int i=0;i<w;++i) expected[(y+j)*16+x+i]=value;
}
int main(int argc,char **argv) {
    check(argc==3); which=std::atoi(argv[1]); scenario=std::atoi(argv[2]);
    for(int i=0;i<1024;++i) { input[i]=static_cast<unsigned char>((i*7+3)%251); output[i]=expected[i]=static_cast<unsigned char>((i*11+17)%251); }
    for(int i=0;i<256;++i) remap[i]=static_cast<unsigned char>(255-i);
    sprite={}; sprite.pcBits_=input; sprite.cTransparentIndex_=66;
    sprite.iSpriteWidth_=3; sprite.iSpriteHeight_=3; sprite.iSpriteWidth2_=5;
    destination.rect1_={3,2,5,4}; destination.stride_=16; destination.locked_bits_=output;
    int stale_x[8]={},stale_y[8]={};
    SpriteRasterDestinationColumnCount=-31; SpriteRasterSourceColumnCount=-32;
    SpriteRasterDestinationRowCount=-33; SpriteRasterSourceRowCount=-34;
    SpriteRasterXTable=stale_x; SpriteRasterYTable=stale_y;
    SpriteRasterDestinationPitch=-41; SpriteRasterSourcePitch=-42;
    SpriteRasterRemapTable.single=input;
    int key=0x109,color=0x17B,x=2,y=1;
    Buffer *to=&destination;
    std::string want="LIU"; int result=0;
    original_expected={2,1,5,4}; clipped_expected=destination.rect1_;
    if(scenario==0) {
        if(which==2) sprite.ppszFileName_=const_cast<char*>("ignored-by-dest");
        input[4]=0; input[5]=9; input[6]=9; input[7]=0;
        int base=which==0?4:6;
        for(int row=0;row<2;++row) for(int col=0;col<2;++col) one(3+col,2+row,input[base+row*5+col],key,color);
    } else if((scenario>=1 && scenario<=4) || scenario==8 || scenario==20) {
        int repeats=scenario==8 || scenario==20?2:scenario;
        int ratio=std::max(2,repeats);
        SpriteScaleNumerator=ratio; SpriteScaleDenominator=1;
        sprite.iSpriteWidth_=3; sprite.iSpriteHeight_=1; sprite.iLeftOffset_=2; sprite.iTopOffset_=2;
        input[0]=0;input[1]=9;input[2]=37; x=-1;y=-2;
        int left=-1+2*ratio,top=-2+2*ratio;
        original_expected={left,top,left+3*ratio,top+ratio};
        destination.rect1_=clipped_expected={left,top,left+3*ratio,top+repeats};
        axis[0]={0,0,3,3*ratio,{ratio,ratio,ratio}};
        axis[1]={0,0,1,repeats,{repeats}};
        want="LFI IXYU"; want.erase(std::remove(want.begin(),want.end(),' '),want.end());
        if(scenario==8) { SpriteScaleNumerator=5; SpriteCachedScaleNumerator=5;SpriteCachedScaleDenominator=1;SpriteScalingRatio=2;want="LIIXYU"; }
        if(scenario==20) key=0;
        block(left,top,ratio,repeats,0,key,color); block(left+ratio,top,ratio,repeats,9,key,color); block(left+ratio*2,top,ratio,repeats,37,key,color);
    } else if(scenario==5 || scenario==6 || scenario==19) {
        SpriteScaleNumerator=2;SpriteScaleDenominator=3;
        sprite.iSpriteWidth_=6;sprite.iSpriteHeight_=6;sprite.iSpriteWidth2_=8;
        original_expected={2,1,6,5};destination.rect1_={2,1,6,5};
        clipped_expected={3,2,5,4};
        if(scenario!=19) destination.rect1_=clipped_expected; else change_clip=true;
        axis[0]={2,1,3,2,{1,2}};axis[1]={2,1,3,2,{1,2}};
        if(scenario==6) {
            axis[1].steps[0]=257;
            // The full-int mutant remains inside real backing storage and
            // reaches wrong pixels, rather than earning credit for a crash.
            input[18+257*8]=input[19+257*8]=9;
        }
        input[18]=0;input[19]=9;input[26]=11;input[27]=19;
        one(3,2,0,key,color);one(4,2,9,key,color);one(3,3,11,key,color);one(4,3,19,key,color);
        want="LFIIXYU";
    } else if(scenario==7) {
        SpriteScaleNumerator=5;want="LU";
    } else if(scenario==9 || scenario==10) {
        check(which!=2); sprite.ppszFileName_=const_cast<char*>("owned.pcx");
        to=nullptr; load_result=scenario==9?6:0;result=scenario==9?6:-17;
        expected_key=key;expected_x=x;expected_y=y;expected_color=color;want=scenario==9?"P":"PDC";
    } else if(scenario==11) { sprite.pcBits_=nullptr;result=3;want="";
    } else if(scenario==12) { to=nullptr;result=3;want="";
    } else if(scenario==13) { lock_result=0;result=3;want="L";
    } else if(scenario==14) { destination.rect1_={50,50,60,60};
    } else if(scenario==15) {
        check(which==0);lock_result=0;SpriteScaleNumerator=7;SpriteScaleDenominator=8;
        check(sprite.draw_mono(&destination,key,x,y,color,2,3)==3);
        check(SpriteScaleNumerator==7 && SpriteScaleDenominator==8 && events=="L");
        check(SpriteRasterDestinationColumnCount==-31 && SpriteRasterSourceColumnCount==-32);
        check(SpriteRasterDestinationRowCount==-33 && SpriteRasterSourceRowCount==-34);
        check(SpriteRasterXTable==stale_x && SpriteRasterYTable==stale_y);
        check(SpriteRasterDestinationPitch==-41 && SpriteRasterSourcePitch==-42);return 0;
    } else if(scenario==16 || scenario==17 || scenario==18) {
        SpriteScaleNumerator=2;SpriteScaleDenominator=3;
        original_expected={2,1,4,3};destination.rect1_=clipped_expected=original_expected;
        axis[0]={0,0,3,2,{2,1}};axis[1]={0,0,3,2,{2,1}};
        if(scenario==16) { SpriteMinimumScaledHeight=3;want="LFIU"; }
        else { helper_failure=scenario==17?1:2;want=scenario==17?"LFIIXU":"LFIIXYU"; }
    } else if(scenario==21 || scenario==22) {
        SpriteScaleNumerator=2; sprite.iSpriteWidth_=3;sprite.iSpriteHeight_=2;
        x=2;y=1; original_expected={2,1,8,5};destination.rect1_=clipped_expected={3,2,7,5};
        axis[0]={0,1,3,4,{1,2,1}};axis[1]={0,1,2,3,{1,2}};
        input[0]=0;input[1]=9;input[2]=37;input[5]=11;input[6]=0;input[7]=23;
        if(scenario==21) {
            block(3,2,1,1,0,key,color);block(4,2,2,1,9,key,color);block(6,2,1,1,37,key,color);
            block(3,3,1,2,11,key,color);block(4,3,2,2,0,key,color);block(6,3,1,2,23,key,color);
        } else {
            SpriteScaleNumerator=3;SpriteScaleDenominator=2;sprite.iSpriteWidth_=2;
            sprite.iLeftOffset_=sprite.iTopOffset_=1;x=y=-2;
            original_expected=destination.rect1_=clipped_expected={0,0,3,3};
            axis[0]={0,0,2,3,{2,1}};axis[1]={0,0,2,3,{2,1}};
            block(0,0,2,2,0,key,color);block(2,0,1,2,9,key,color);
            block(0,2,2,1,11,key,color);block(2,2,1,1,0,key,color);
        }
        want="LFIIXYU";
    } else if(scenario==23) {
        SpriteScaleNumerator=2;sprite.iSpriteWidth_=2;sprite.iSpriteHeight_=1;sprite.iSpriteWidth2_=5;
        x=2;y=2; original_expected={2,2,6,4};destination.rect1_=clipped_expected={2,2,6,3};
        axis[0]={0,0,2,4,{2,2}};axis[1]={0,0,1,1,{1}};
        sprite.pcBits_=output+34;
        output[34]=expected[34]=0;output[35]=expected[35]=9;
        // First expanded source pixel overwrites the next source byte;
        // the next source read must observe it, not a whole-row snapshot.
        unsigned char value=which==0?123:which==1?0:255;
        for(int column=0;column<4;++column) expected[34+column]=value;
        want="LFIIXYU";
    } else check(false);
    unsigned char source_before[4096];std::memcpy(source_before,input,sizeof input);
    check(paint(to,key,x,y,color)==result);
    if(events!=want) {std::fprintf(stderr,"events=%s expected=%s\n",events.c_str(),want.c_str());std::exit(91);}
    for(int i=0;i<1024;++i) if(output[i]!=expected[i]) {std::fprintf(stderr,"pixel%d=%d expected%d\n",i,output[i],expected[i]);std::exit(91);}
    check(!std::memcmp(source_before,input,sizeof input));
    if(scenario==7) check(SpriteCachedScaleNumerator==-1 && SpriteCachedScaleDenominator==-1 && SpriteScalingRatio==5);
    if(events.find('F')!=std::string::npos) check(SpriteCachedScaleNumerator==SpriteScaleNumerator && SpriteCachedScaleDenominator==SpriteScaleDenominator);
    if(scenario==0) {
        check(SpriteRasterDestinationColumnCount==2 && SpriteRasterDestinationRowCount==(which==2?0:2));
        check(SpriteRasterSourceColumnCount==-32 && SpriteRasterSourceRowCount==-34);
        check(SpriteRasterXTable==stale_x && SpriteRasterYTable==stale_y);
        check(SpriteRasterDestinationPitch==16 && SpriteRasterSourcePitch==5);
        check(SpriteRasterRemapTable.single==input);
    } else if(scenario==14 || scenario==7 || scenario==16) {
        check(SpriteRasterDestinationColumnCount==-31 && SpriteRasterSourceColumnCount==-32);
        check(SpriteRasterDestinationRowCount==-33 && SpriteRasterSourceRowCount==-34);
        check(SpriteRasterXTable==stale_x && SpriteRasterYTable==stale_y);
        check(SpriteRasterDestinationPitch==(scenario==7? -41:16));
        check(SpriteRasterSourcePitch==(scenario==7? -42:5));
        check(SpriteRasterRemapTable.single==input);
    } else if(scenario==17) {
        check(SpriteRasterXTable==nullptr && SpriteRasterYTable==stale_y);
        check(SpriteRasterDestinationColumnCount==-31 && SpriteRasterSourceColumnCount==-32);
        check(SpriteRasterDestinationRowCount==-33 && SpriteRasterSourceRowCount==-34);
        check(SpriteRasterDestinationPitch==16 && SpriteRasterSourcePitch==5);
        check(SpriteRasterRemapTable.single==input);
    } else if(scenario==18) {
        check(SpriteRasterXTable==axis[0].steps && SpriteRasterYTable==nullptr);
        check(SpriteRasterDestinationColumnCount==axis[0].destination_count);
        check(SpriteRasterSourceColumnCount==axis[0].source_count);
        check(SpriteRasterDestinationRowCount==-33 && SpriteRasterSourceRowCount==-34);
        check(SpriteRasterDestinationPitch==16 && SpriteRasterSourcePitch==5);
        check(SpriteRasterRemapTable.single==input);
    } else if((scenario>=1 && scenario<=4) || scenario==8 || scenario==20 ||
              scenario==21 || scenario==22 || scenario==5 || scenario==6 || scenario==19) {
        const bool expanding=SpriteScaleNumerator>=SpriteScaleDenominator;
        check(SpriteRasterXTable==axis[0].steps);
        check(SpriteRasterYTable==axis[1].steps +
              (expanding ? axis[1].source_count : axis[1].destination_count));
        check(SpriteRasterDestinationColumnCount==axis[0].destination_count);
        check(SpriteRasterSourceColumnCount==axis[0].source_count);
        check(SpriteRasterDestinationRowCount==(expanding ? axis[1].destination_count : 0));
        check(SpriteRasterSourceRowCount==(expanding ? 0 : axis[1].source_count));
        check(SpriteRasterDestinationPitch==16 && SpriteRasterSourcePitch==((scenario==5 || scenario==6 || scenario==19)?8:5));
        check(SpriteRasterRemapTable.single==(which==2 && expanding?remap:input));
    }
    return 0;
}
'''


def compile_host(tmp_path, bodies):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source = tmp_path / "blitters.cpp"
    binary = tmp_path / "blitters"
    source.write_text(HARNESS.replace("// LIVE_BODIES", bodies))
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
        "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return binary


@pytest.fixture(scope="module")
def executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("sprite-blitters"), live_bodies())


CASES = [(kind, scenario) for kind in range(3) for scenario in range(24)
         if not (kind == 2 and scenario in (9, 10)) and not (kind != 0 and scenario == 15)]


@pytest.mark.parametrize("kind,scenario", CASES)
def test_live_blitter_paths(executable, kind, scenario):
    result = subprocess.run([str(executable), str(kind), str(scenario)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after,kind,scenario", [
    ("destination->free_data(1);", "/* dropped release */", 0, 0),
    ("SpriteScalingRatio > 4.0", "SpriteScalingRatio >= 4.0", 0, 4),
    ("(clipped.top - original.top) * iSpriteWidth_", "(clipped.top - original.top) * source_pitch", 0, 0),
    ("const unsigned char row_selector =\n                static_cast<unsigned char>(*SpriteRasterYTable++);",
     "const int row_selector = *SpriteRasterYTable++;", 0, 6),
    ("source_row[column] != key", "true", 0, 2),
    ("saved_denominator;", "saved_numerator;", 0, 15),
    ("g_BUFFER.close();", "/* missing close */", 0, 10),
])
def test_behavior_damage(tmp_path, before, after, kind, scenario):
    source = live_bodies()
    assert before in source
    binary = compile_host(tmp_path, source.replace(before, after))
    result = subprocess.run([str(binary), str(kind), str(scenario)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 91, result.stdout + result.stderr
