"""Actual five-argument Sprite::draw, original 5E9D44..5EAAFF.

Independent pixel oracles deliberately differ from draw_dest: unit/shrink map
non-key zero; expansion skips zero, and its default four-row kernel maps the
first destination pixel. Scaling/clipping/acquisition are service doubles, not
native ABI, DirectDraw, x87 precision/status, real helper integration or shared
raster-scratch reentrancy proof. The renderer-level fixture seeds and checks
source-owned count, pointer and pitch cells; it does not claim coverage of the
other seven mapped renderers. A separate integration case extracts the real
fill and all four clipping methods as well as this renderer body.
Shrink low-byte Y steps are nonzero; zero invokes an original huge wrap-loop.
Expansion selectors above four are explicit worker-boundary stress controls,
not claimed outputs of the ordinary capped scaling helper. Nonuniform tables
and high-byte X advances likewise test the worker boundary; their consumed-X
sums preserve the helpers' count contract, not full real-helper integration.
LSan is disabled
for ptrace; address and undefined-behavior sanitizers remain enabled.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def one(pattern, text):
    result, = re.findall(pattern, text, re.M)
    return result


def actual(with_real_clippers=False):
    source = (ROOT / "src/sprite.cpp").read_text()
    header = (ROOT / "src/sprite.h").read_text()
    buffer = (ROOT / "src/buffer.h").read_text()
    signature = one(r"int Sprite::draw\(Buffer \*destination, int transparent, int x, int y,\s*unsigned char \*table\)\s*\{", source)
    start = source.index(signature)
    depth = 0
    for end in range(source.index("{", start), len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    multi_signature = one(r"int Sprite::draw_multi_table_dest\(Buffer \*destination, int transparent, int x,\s*int y, ColorTable \*multi_table\)\s*\{", source)
    multi_start = source.index(multi_signature)
    multi_depth = 0
    for multi_end in range(source.index("{", multi_start), len(source)):
        multi_depth += (source[multi_end] == "{") - (source[multi_end] == "}")
        if multi_depth == 0:
            multi_body = source[multi_start:multi_end + 1]
            break
    fields = "\n".join(one(r"^  (?:char \*|unsigned char \*|int |unsigned char )" + name + r";", header) for name in
        ("ppszFileName_", "pcBits_", "cTransparentIndex_", "iSpriteWidth2_", "iSpriteWidth_", "iSpriteHeight_",
         "iWidth_", "iHeight_", "iLeftOffset_", "iTopOffset_"))
    methods = one(r"  int draw\(Buffer \*destination, int transparent, int x, int y,\s*unsigned char \*table\);", header)
    for name in ("fill_scaling_table", "get_clipped_x_table_shrink", "get_clipped_y_table_shrink",
                 "get_clipped_x_table_expand", "get_clipped_y_table_expand"):
        methods += "\n" + one(r"  (?:void|int \*)\s*" + name + r"\([^;]*\);", header)
    globals_ = "\n".join(one(r"^(?:int|float) " + name + r"(?: = -?\d+)?;", source) for name in
        ("SpriteScaleNumerator", "SpriteScaleDenominator", "SpriteCachedScaleNumerator", "SpriteCachedScaleDenominator",
         "SpriteScalingRatio", "SpriteMinimumScaledWidth", "SpriteMinimumScaledHeight"))
    raster_cells = "\n".join(one(r"^(?:int\s+|int\s*\*\s*)" + name + r";", source) for name in
        ("SpriteRasterDestinationColumnCount", "SpriteRasterSourceColumnCount",
         "SpriteRasterDestinationRowCount", "SpriteRasterSourceRowCount",
         "SpriteRasterXTable", "SpriteRasterYTable",
         "SpriteRasterDestinationPitch", "SpriteRasterSourcePitch"))
    remap_union = one(r"(?s)^union SpriteRasterRemapPointer \{.*?^\};\s*$", source)
    remap_global = one(r"^SpriteRasterRemapPointer SpriteRasterRemapTable;\s*$", source)
    step_arrays = "\n".join(one(r"^int " + name + r"\[SpriteScalingTableCount\];", source) for name in
        ("SpriteScalingSteps", "SpriteClippedYSteps", "SpriteClippedXSteps"))
    table_count = one(r"^enum \{ SpriteScalingTableCount = \d+ \};$", header)
    buffer_fields = "\n".join(one(r"^  (?:RECT |uint32_t |LPVOID )" + name + r";", buffer)
                              for name in ("rect1_", "stride_", "locked_bits_"))
    if with_real_clippers:
        clippers = []
        for name, result in (("fill_scaling_table", "void"),
                             ("get_clipped_x_table_expand", r"int\s*\*\s*"),
                             ("get_clipped_y_table_expand", r"int\s*\*\s*"),
                             ("get_clipped_x_table_shrink", r"int\s*\*\s*"),
                             ("get_clipped_y_table_shrink", r"int\s*\*\s*")):
            signature = one(result + r"\s*Sprite::" + name + r"\([^;{}]*\)\s*\{", source)
            start = source.index(signature)
            opening = source.index("{", start)
            depth = 0
            for end in range(opening, len(source)):
                depth += (source[end] == "{") - (source[end] == "}")
                if depth == 0:
                    clippers.append(source[start:end + 1])
                    break
        clipping = "\n".join(clippers)
    else:
        clipping = CLIPPING_STUBS
    return {"BODY": body, "MULTIBODY": multi_body, "FIELDS": fields, "METHODS": methods,
            "GLOBALS": globals_ + "\n" + table_count + "\n" + raster_cells + "\n" + remap_union + "\n" + remap_global + "\n" + step_arrays,
            "BUFFER_FIELDS": buffer_fields, "CLIPPERS": clipping}


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <functional>
using LPVOID=void*;
typedef unsigned char ColorTable[256];
void require_at(bool value,int line){if(!value){fprintf(stderr,"assertion line %d\n",line);std::_Exit(99);}}
#define require(x) require_at((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
struct Buffer {BUFFER_FIELDS void *get_data();void free_data(int);};
struct Sprite {FIELDS METHODS int draw_multi_table_dest(Buffer*,int,int,int,ColorTable*);};
GLOBALS
unsigned char memory[131072],expected[131072];
const int SOURCE=0,ALTSOURCE=16384,TABLE=32768,OUTPUT=65536,ALTOUTPUT=98304;
Sprite sprite;Buffer buffer;std::string events;int locks,unlocks,clip_calls,fail_axis;
int stale_x[8],stale_y[8];
std::function<void(char)> callback;
struct Axis {int source_skip,destination_skip,source_count,destination_count,steps[16];} axes[2];
RECT expected_original,expected_clipped;bool check_original;
bool equal(RECT a,RECT b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
void event(char c){events+=c;if(callback)callback(c);}
void *Buffer::get_data(){require(this==&buffer);event('L');buffer.locked_bits_=locks?memory+OUTPUT:nullptr;return buffer.locked_bits_;}
void Buffer::free_data(int n){require(this==&buffer&&n==1);++unlocks;event('U');}
int IntersectRect(RECT*out,const RECT*a,const RECT*b){
 require(b==&buffer.rect1_);if(check_original)require(equal(*a,expected_original));
 ++clip_calls;require((clip_calls==1)==(out==a));
 RECT r={std::max(a->left,b->left),std::max(a->top,b->top),std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
 bool valid=r.left<r.right&&r.top<r.bottom;*out=valid?r:RECT{0,0,0,0};event(clip_calls==1?'I':'J');return valid;
}
int* axis(int index,bool expand,RECT*o,RECT*c,int*a,int*b,int*d,int*e){
 require(equal(*o,expected_original)&&equal(*c,expected_clipped));event(index?'Y':'X');if(fail_axis==index+1)return nullptr;
 Axis&v=axes[index];*a=expand?v.source_skip:v.destination_skip;*b=expand?v.destination_skip:v.source_skip;
 *d=expand?v.source_count:v.destination_count;*e=expand?v.destination_count:v.source_count;return v.steps;
}
// CLIPPING_IMPLEMENTATIONS
BODY
MULTIBODY
void reset(){
 for(int i=0;i<131072;++i)memory[i]=expected[i]=static_cast<unsigned char>((i*17+23)%251);
 for(int i=0;i<256;++i)memory[TABLE+i]=expected[TABLE+i]=static_cast<unsigned char>((i*37+7)%256);
 sprite={};sprite.pcBits_=memory+SOURCE;sprite.ppszFileName_=const_cast<char*>("must-not-load.pcx");
 sprite.iSpriteWidth2_=12;sprite.iSpriteWidth_=5;sprite.iSpriteHeight_=3;
 buffer={};buffer.rect1_={0,0,64,64};buffer.stride_=64;buffer.locked_bits_=memory+OUTPUT;
 SpriteScaleNumerator=SpriteScaleDenominator=1;SpriteCachedScaleNumerator=SpriteCachedScaleDenominator=-1;
 SpriteScalingRatio=0;SpriteMinimumScaledWidth=SpriteMinimumScaledHeight=0;
 SpriteRasterDestinationColumnCount=-31;SpriteRasterSourceColumnCount=-32;
 SpriteRasterDestinationRowCount=-33;SpriteRasterSourceRowCount=-34;
 SpriteRasterXTable=stale_x;SpriteRasterYTable=stale_y;
 SpriteRasterDestinationPitch=-41;SpriteRasterSourcePitch=-42;
 SpriteRasterRemapTable.single=memory+ALTSOURCE;
 for(int i=0;i<SpriteScalingTableCount;++i)SpriteScalingSteps[i]=SpriteClippedXSteps[i]=SpriteClippedYSteps[i]=0;
 events.clear();locks=1;unlocks=clip_calls=fail_axis=0;callback={};check_original=true;
 axes[0]={};axes[1]={};
}
void stale_cells(){
 require(SpriteRasterDestinationColumnCount==-31&&SpriteRasterSourceColumnCount==-32);
 require(SpriteRasterDestinationRowCount==-33&&SpriteRasterSourceRowCount==-34);
 require(SpriteRasterXTable==stale_x&&SpriteRasterYTable==stale_y);
 require(SpriteRasterRemapTable.single==memory+ALTSOURCE);
}
void check_pitches(int source,int destination){
 require(SpriteRasterSourcePitch==source&&SpriteRasterDestinationPitch==destination);
}
void put(int index,int value){memory[index]=expected[index]=static_cast<unsigned char>(value);}
void pixels(){for(int i=0;i<131072;++i)if(memory[i]!=expected[i]){fprintf(stderr,"pixel %d actual%d expected%d\n",i,memory[i],expected[i]);require(false);}}
void done(int result,const char*trace,int code=0){if(result!=code||events!=trace)fprintf(stderr,"done got result%d events%s expected%d %s\\n",result,events.c_str(),code,trace);require(result==code&&events==trace);require(unlocks==(strchr(trace,'U')?1:0));pixels();}
// Independent sample coordinates: unit/shrink prefix-sum advances; every LUT
// access is against the evolving expected arena, preserving all three aliases.
void sample(int source,int destination,int table,int key){unsigned char value=expected[source];
 if(value!=static_cast<unsigned char>(key))expected[destination]=expected[table+value];}
void unit(int source,int destination,int columns,int rows,int source_pitch,int destination_pitch,int table,int key){
 for(int y=0;y<rows;++y)for(int x=0;x<columns;++x)sample(source+y*source_pitch+x,destination+y*destination_pitch+x,table,key);}
void shrink(int source,int destination,int source_pitch,int destination_pitch,int table,int key){
 int sy=0;for(int y=0;y<axes[1].destination_count;++y){int sx=0;
  for(int x=0;x<axes[0].destination_count;++x){sample(source+sy*source_pitch+sx,destination+y*destination_pitch+x,table,key);sx+=axes[0].steps[x];}
  sy+=axes[1].steps[y]%256;}}
// Whole source-pixel rectangles, independently scheduled as original blocks.
// Distinct destination contents make four/default's remap operand observable.
void expand(int source,int destination,int source_pitch,int destination_pitch,int table,int key){
 int y=0;for(int row=0;row<axes[1].source_count;++row){int selector=axes[1].steps[row]&255;
  int height=selector>=1&&selector<=3?selector:4;int x=0;
  for(int col=0;col<axes[0].source_count;++col){int width=axes[0].steps[col];unsigned char value=expected[source+row*source_pitch+col];
   if(value!=static_cast<unsigned char>(key)&&value!=0){int index=height==4?expected[destination+y*destination_pitch+x]:value;
    unsigned char mapped=expected[table+index];for(int yy=y;yy<y+height;++yy)for(int xx=x;xx<x+width;++xx)
     expected[destination+yy*destination_pitch+xx]=mapped;}
   x+=width;}y+=height;}}
int paint(int key=9,int x=2,int y=3,int table=TABLE){
 expected_clipped={std::max(expected_original.left,buffer.rect1_.left),std::max(expected_original.top,buffer.rect1_.top),
  std::min(expected_original.right,buffer.rect1_.right),std::min(expected_original.bottom,buffer.rect1_.bottom)};
 return sprite.draw(&buffer,key,x,y,memory+table);}
int actual_clipper_integration(){
 // Real fill and all four real clipping methods execute inside this live
 // renderer. Independent fixtures verify pixels plus terminal shared cells.
 reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;
 sprite.iSpriteWidth_=sprite.iSpriteHeight_=6;expected_original={2,3,6,7};
 buffer.rect1_={3,4,6,7};expected_clipped=buffer.rect1_;
 axes[0]={2,1,4,3,{1,2,1}};axes[1]={2,1,4,3,{1,2,1}};
 for(int row=0;row<6;++row)for(int col=0;col<6;++col)put(row*12+col,1+(row+col)%7);
 shrink(26,OUTPUT+4*64+3,12,64,TABLE,9);
 done(paint(9,2,3,TABLE),"LIJU");
 require(SpriteRasterDestinationColumnCount==3&&SpriteRasterSourceColumnCount==4);
 require(SpriteRasterDestinationRowCount==0&&SpriteRasterSourceRowCount==4);
 require(SpriteRasterXTable==SpriteClippedXSteps);
 require(SpriteRasterYTable==SpriteClippedYSteps+3);check_pitches(12,64);
 require(SpriteRasterRemapTable.single==memory+ALTSOURCE);

 reset();SpriteScaleNumerator=3;SpriteScaleDenominator=2;
 sprite.iSpriteWidth_=sprite.iSpriteHeight_=2;expected_original={0,0,3,3};
 buffer.rect1_=expected_clipped=expected_original;
 axes[0]={0,0,2,3,{2,1}};axes[1]={0,0,2,3,{2,1}};
 put(0,1);put(1,2);put(12,3);put(13,4);
 expand(0,OUTPUT,12,64,TABLE,9);
 done(paint(9,0,0,TABLE),"LIJU");
 require(SpriteRasterDestinationColumnCount==3&&SpriteRasterSourceColumnCount==2);
 require(SpriteRasterDestinationRowCount==3&&SpriteRasterSourceRowCount==0);
 require(SpriteRasterXTable==SpriteClippedXSteps);
 require(SpriteRasterYTable==SpriteClippedYSteps+2);check_pitches(12,64);
 require(SpriteRasterRemapTable.single==memory+TABLE);
 return 0;
}
void setup_expand(int count,int selector){
 SpriteScaleNumerator=2;sprite.iSpriteWidth_=count;sprite.iSpriteHeight_=2;
 expected_original={2,3,2+count*2,7};buffer.rect1_=expected_original;
 axes[0]={0,0,count,0,{}};for(int i=0;i<count;++i){axes[0].steps[i]=i%3+1;axes[0].destination_count+=i%3+1;}
 axes[1]={0,0,2,4,{selector,2}};
 for(int i=0;i<count;++i){put(SOURCE+i,i%3==0?0:i%3==1?9:47+i);put(SOURCE+12+i,31+i);}
}
int actual_multi_table_runtime(){
 reset();SpriteScaleNumerator=2;SpriteScaleDenominator=1;
 sprite.iSpriteWidth_=sprite.iSpriteHeight_=2;expected_original={0,0,4,4};
 buffer.rect1_=expected_clipped=expected_original;
 axes[0]={0,0,2,4,{2,2}};axes[1]={0,0,2,4,{2,2}};
 unsigned char multi[256][256];
 for(int value=0;value<256;++value)for(int target=0;target<256;++target)
  multi[value][target]=static_cast<unsigned char>(value+target);
 put(0,1);put(1,2);put(12,3);put(13,4);
 for(int sy=0;sy<2;++sy)for(int sx=0;sx<2;++sx){
  int value=memory[sy*12+sx];
  for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx){
   int at=OUTPUT+(sy*2+dy)*64+sx*2+dx;
   expected[at]=multi[value][expected[at]];
  }
 }
 int result=sprite.draw_multi_table_dest(&buffer,9,0,0,multi);
 done(result,"LFIJXYU",1);
 require(SpriteRasterRemapTable.multi==multi);
 require(SpriteRasterSourceRowCount==0);
 return 0;
}
int main(){
 if(std::getenv("SPRITE_MULTI_RUNTIME")) return actual_multi_table_runtime();
 if(std::getenv("SPRITE_REAL_CLIPPERS")) return actual_clipper_integration();
 // Unit widths cover every unrolled tail; unequal display/source/pitch values.
 for(int width=1;width<=9;++width)for(int key : {0,9,0x109,-1})for(int clipped=0;clipped<2;++clipped){
  reset();sprite.iSpriteWidth_=width;sprite.iSpriteHeight_=3;sprite.iWidth_=77;sprite.iHeight_=88;
  sprite.iLeftOffset_=1;sprite.iTopOffset_=2;expected_original={3,5,3+width,8};
  if(clipped&&width>2)buffer.rect1_={4,6,2+width,8};else buffer.rect1_=expected_original;
  for(int y=0;y<3;++y)for(int x=0;x<width;++x)put(y*12+x,(x+y)%4==0?0:(x+y)%4==1?9:(x+y)%4==2?255:47);
  put(TABLE+47,9);put(TABLE+255,0); // mapped key/zero still overwrite destination
  RECT clipped_rect=buffer.rect1_;unit((clipped_rect.top-5)*12+clipped_rect.left-3,OUTPUT+clipped_rect.top*64+clipped_rect.left,clipped_rect.right-clipped_rect.left,clipped_rect.bottom-clipped_rect.top,12,64,TABLE,key);
  done(paint(key),"LIU");
  RECT c=buffer.rect1_;
  require(SpriteRasterDestinationColumnCount==c.right-c.left && SpriteRasterDestinationRowCount==0);
  require(SpriteRasterSourceColumnCount==-32 && SpriteRasterSourceRowCount==-34);
  require(SpriteRasterXTable==stale_x && SpriteRasterYTable==stale_y);
  require(SpriteRasterRemapTable.single==memory+ALTSOURCE);check_pitches(12,64);}
 // Shrink source-width6 differs from pitch12; X high byte matters, Y doesn't.
 for(int high=0;high<2;++high)for(int key : {0,9,0x109}){
  reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;sprite.iSpriteWidth_=6;sprite.iSpriteHeight_=6;
  expected_original={2,3,6,7};buffer.rect1_={3,4,5,6};
  axes[0]={2,1,high?258:3,2,{high?257:2,1}};axes[1]={1,1,3,2,{258,1}};
  for(int y=0;y<6;++y){put(y*12+2,0);put(y*12+2+(high?257:2),47+y);}
  shrink(14,OUTPUT+4*64+3,12,64,TABLE,key);done(paint(key),"LFIJXYU");
  require(SpriteRasterXTable==axes[0].steps && SpriteRasterYTable==axes[1].steps+2);
  require(SpriteRasterDestinationColumnCount==2 && SpriteRasterSourceColumnCount==axes[0].source_count);
  require(SpriteRasterDestinationRowCount==0 && SpriteRasterSourceRowCount==3);
  require(SpriteRasterRemapTable.single==memory+ALTSOURCE);check_pitches(12,64);}
 for(int selector : {0,1,2,3,4,5,255,257,258,259,260,261})for(int count=1;count<=7;++count){
  reset();setup_expand(count,selector);expand(0,OUTPUT+3*64+2,12,64,TABLE,9);done(paint(),"LFIJXYU");
  require(SpriteRasterXTable==axes[0].steps && SpriteRasterYTable==axes[1].steps+axes[1].source_count);
  require(SpriteRasterDestinationColumnCount==axes[0].destination_count);
  require(SpriteRasterSourceColumnCount==count);
  require(SpriteRasterDestinationRowCount==4 && SpriteRasterSourceRowCount==0);
  require(SpriteRasterRemapTable.single==memory+TABLE);check_pitches(12,64);}
 // Aliases are not snapshots. Unit table writes can change later table reads.
 for(int mode=0;mode<3;++mode){reset();sprite.iSpriteWidth_=4;sprite.iSpriteHeight_=1;expected_original={2,3,6,4};
  int table=TABLE,source=SOURCE,dst=OUTPUT+3*64+2;
  if(mode==0){table=dst;put(SOURCE,1);put(SOURCE+1,0);put(dst+1,72);}
  if(mode==1){source=dst-1;sprite.pcBits_=memory+source;put(source,47);}
  if(mode==2){table=SOURCE;put(SOURCE,2);put(SOURCE+1,1);put(SOURCE+2,91);}
  unit(source,dst,4,1,12,64,table,9);done(paint(9,2,3,table),"LIU");}
 for(int alias=0;alias<3;++alias){
  reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;sprite.iSpriteWidth_=6;sprite.iSpriteHeight_=6;
  expected_original={2,3,6,7};buffer.rect1_={2,3,4,5};
  axes[0]={0,0,3,2,{1,2}};axes[1]={0,0,3,2,{1,2}};
  int source=SOURCE,table=TABLE,dst=OUTPUT+3*64+2;put(SOURCE,1);put(SOURCE+1,0);
  if(alias==0){source=dst-1;sprite.pcBits_=memory+source;put(source,47);}
  if(alias==1){table=dst;put(dst,2);put(dst+1,71);}
  if(alias==2){table=SOURCE;put(SOURCE+1,2);put(SOURCE+2,91);}
  shrink(source,dst,12,64,table,9);done(paint(9,2,3,table),"LFIJXYU");}
 for(int repeats : {1,2,3,4,5})for(int alias=0;alias<3;++alias){
  reset();setup_expand(3,repeats);int table=TABLE,source=SOURCE,dst=OUTPUT+3*64+2;
  // Force non-key values in every block, with mapping changing later input.
  put(SOURCE,1);put(SOURCE+1,2);put(SOURCE+2,3);
  if(alias==0){source=dst-1;sprite.pcBits_=memory+source;put(source,47);}
  if(alias==1){table=dst;put(dst,2);put(dst+1,71);put(dst+2,99);}
  if(alias==2){table=SOURCE;put(SOURCE+1,27);put(SOURCE+2,91);}
  expand(source,dst,12,64,table,9);done(paint(9,2,3,table),"LFIJXYU");}
 // Real clipped expansion: partial runs with both nonzero source and
 // destination skips, not a coordinate-normalized worker-only example.
 reset();SpriteScaleNumerator=2;sprite.iSpriteWidth_=4;sprite.iSpriteHeight_=3;
 expected_original={2,3,10,9};buffer.rect1_={5,6,9,9};
 axes[0]={1,3,3,4,{1,2,1}};axes[1]={1,3,2,3,{1,2}};
 expand(13,OUTPUT+6*64+5,12,64,TABLE,9);done(paint(),"LFIJXYU");
 // Guard/return matrix: filename is always nonnull yet no file API exists.
 reset();sprite.pcBits_=nullptr;done(paint(),"",3);stale_cells();
 reset();require(sprite.draw(nullptr,9,2,3,memory+TABLE)==3&&events.empty());pixels();stale_cells();
 reset();locks=0;done(paint(),"L",3);stale_cells();
 reset();expected_original={2,3,7,6};buffer.rect1_={40,40,45,45};done(paint(),"LIU");stale_cells();
 reset();SpriteScaleNumerator=5;done(paint(),"LU");require(SpriteScalingRatio==5&&SpriteCachedScaleNumerator==-1);stale_cells();
 for(int failure=1;failure<=2;++failure)for(int enlarge=0;enlarge<2;++enlarge){
  reset();SpriteScaleNumerator=enlarge?2:2;SpriteScaleDenominator=enlarge?1:3;
  expected_original=enlarge?RECT{2,3,12,9}:RECT{2,3,5,5};buffer.rect1_=expected_original;fail_axis=failure;
  done(paint(),failure==1?"LFIJXU":"LFIJXYU");
  require(SpriteRasterXTable==(failure==1?nullptr:axes[0].steps));
  require(SpriteRasterYTable==(failure==1?stale_y:nullptr));
  if(failure==1){require(SpriteRasterDestinationColumnCount==-31&&SpriteRasterSourceColumnCount==-32);
   require(SpriteRasterDestinationRowCount==-33&&SpriteRasterSourceRowCount==-34);}
  else {require(SpriteRasterDestinationColumnCount==axes[0].destination_count&&
                SpriteRasterSourceColumnCount==axes[0].source_count);
   require(SpriteRasterDestinationRowCount==-33&&SpriteRasterSourceRowCount==-34);}
  require(SpriteRasterRemapTable.single==memory+ALTSOURCE);
  check_pitches(12,64);}
 for(int minaxis=0;minaxis<2;++minaxis){reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;expected_original={2,3,5,5};
  if(minaxis)SpriteMinimumScaledHeight=3;else SpriteMinimumScaledWidth=4;done(paint(),"LFIU");stale_cells();check_pitches(12,64);}
 reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;expected_original={2,3,5,5};buffer.rect1_=expected_original;
 SpriteMinimumScaledWidth=3;SpriteMinimumScaledHeight=2;axes[0]={0,0,5,3,{2,1,2}};axes[1]={0,0,3,2,{2,1}};
 shrink(0,OUTPUT+3*64+2,12,64,TABLE,9);done(paint(),"LFIJXYU");check_pitches(12,64);
 // Cached ratio bypasses the uncached >4 cap; actual ratio remains two.
 reset();setup_expand(3,2);SpriteScaleNumerator=SpriteCachedScaleNumerator=5;SpriteCachedScaleDenominator=1;SpriteScalingRatio=2;
 expand(0,OUTPUT+3*64+2,12,64,TABLE,9);done(paint(),"LIJXYU");check_pitches(12,64);
 // Unit pcBits is loaded after clipping; pitch captured before. Acquisition
 // changes are observed, then clip changes only the live pointer/lockedbits.
 reset();expected_original={2,3,7,6};callback=[&](char c){if(c=='L')buffer.stride_=80;
  if(c=='I'){sprite.pcBits_=memory+ALTSOURCE;sprite.iSpriteWidth2_=33;buffer.stride_=96;buffer.locked_bits_=memory+ALTOUTPUT;}};
 unit(ALTSOURCE,ALTOUTPUT+3*80+2,5,3,12,80,TABLE,9);done(paint(),"LIU");check_pitches(12,80);
 // Scaled first-pixel base uses old pitch64, traversal uses fresh80. Source
 // stays preclip, locked pixel owner is sampled after first but before second.
 reset();setup_expand(3,2);callback=[&](char c){if(c=='I'){sprite.pcBits_=memory+ALTSOURCE;buffer.stride_=80;buffer.locked_bits_=memory+ALTOUTPUT;}
  if(c=='J'){sprite.pcBits_=memory+TABLE;buffer.stride_=96;buffer.locked_bits_=memory+OUTPUT;}};
 expand(0,ALTOUTPUT+3*64+2,12,80,TABLE,9);done(paint(),"LFIJXYU");check_pitches(12,80);
 // The first intersection is discarded; a changed clip must reach both
 // helpers through the second call. Snapshotting its first result is wrong.
 reset();SpriteScaleNumerator=2;SpriteScaleDenominator=3;sprite.iSpriteWidth_=sprite.iSpriteHeight_=6;
 expected_original={2,3,6,7};buffer.rect1_=expected_original;axes[0]={2,1,3,2,{1,2}};axes[1]={2,1,3,2,{1,2}};
 callback=[&](char c){if(c=='I'){buffer.rect1_={3,4,5,6};expected_clipped=buffer.rect1_;}};
 shrink(26,OUTPUT+4*64+3,12,64,TABLE,9);done(paint(),"LFIJXYU");check_pitches(12,64);
 // Fill may publish scale operands: cache stores reload them afterward.
 reset();setup_expand(3,2);callback=[&](char c){if(c=='F'){SpriteScaleNumerator=7;SpriteScaleDenominator=4;}};
 expand(0,OUTPUT+3*64+2,12,64,TABLE,9);done(paint(),"LFIJXYU");
 require(SpriteCachedScaleNumerator==7&&SpriteCachedScaleDenominator==4&&SpriteScalingRatio==2);check_pitches(12,64);
 // Fractional offset addition rounds the whole signed coordinate toward zero.
 reset();SpriteScaleNumerator=3;SpriteScaleDenominator=2;sprite.iSpriteWidth_=sprite.iSpriteHeight_=2;
 sprite.iLeftOffset_=sprite.iTopOffset_=1;expected_original={0,0,3,3};buffer.rect1_=expected_original;
 axes[0]={0,0,2,3,{2,1}};axes[1]={0,0,2,3,{2,1}};
 expand(0,OUTPUT,12,64,TABLE,9);done(paint(9,-2,-2),"LFIJXYU");check_pitches(12,64);
 puts("source remap controls passed");}
'''


CLIPPING_STUBS = r'''
void Sprite::fill_scaling_table(){require(this==&sprite);event('F');}
#define AXIS(NAME,I,E) int* Sprite::NAME(RECT*o,RECT*c,int*a,int*b,int*d,int*e){require(this==&sprite);return axis(I,E,o,c,a,b,d,e);}
AXIS(get_clipped_x_table_shrink,0,false)
AXIS(get_clipped_y_table_shrink,1,false)
AXIS(get_clipped_x_table_expand,0,true)
AXIS(get_clipped_y_table_expand,1,true)
'''


def run(tmp_path, parts, env_extra=None):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    text = HARNESS.replace("// CLIPPING_IMPLEMENTATIONS", parts["CLIPPERS"])
    for key in sorted((key for key in parts if key != "CLIPPERS"), key=len, reverse=True):
        text = text.replace(key, parts[key])
    source, exe = tmp_path / "remap.cpp", tmp_path / "remap"
    source.write_text(text)
    compiled = subprocess.run([compiler, "-std=c++17", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(exe)], text=True, capture_output=True, timeout=45)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, timeout=20,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0", "UBSAN_OPTIONS": "halt_on_error=1",
                               **(env_extra or {})})


def test_actual_source_remap(tmp_path):
    result = run(tmp_path, actual())
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_renderer_fill_and_all_clipping_methods(tmp_path):
    result = run(tmp_path, actual(with_real_clippers=True), {"SPRITE_REAL_CLIPPERS": "1"})
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_multi_table_union_view_and_pixels(tmp_path):
    result = run(tmp_path, actual(), {"SPRITE_MULTI_RUNTIME": "1"})
    assert result.returncode == 0, result.stdout + result.stderr


DAMAGE = {
    "guard_return": ("return 3;", "return 0;"),
    "lock_polarity": ("destination->get_data() == 0", "destination->get_data() != 0"),
    "file_route": ("const unsigned char key =", "if (ppszFileName_) return 3;\n    const unsigned char key ="),
    "key_not_lowbyte": ("const unsigned char key = static_cast<unsigned char>(transparent);", "const int key = transparent;"),
    "unit_zero_skip": ("if (source_row[column] != key)", "if (source_row[column] != key && source_row[column] != 0)"),
    "unit_wrong_lut_operand": ("table[source_row[column]]", "table[destination_row[column]]"),
    "unit_source_skip_pitch": ("(clipped.top - original.top) * source_pitch", "(clipped.top - original.top) * iSpriteWidth_"),
    "unit_stale_source": ("pcBits_ +\n            (clipped.left - original.left)", "scaled_source +\n            (clipped.left - original.left)"),
    "unit_live_pitch": ("clipped.top * destination_pitch", "clipped.top * static_cast<int>(destination->stride_)"),
    "shrink_zero_skip": ("if (*source != key)", "if (*source != key && *source != 0)"),
    "shrink_wrong_lut_operand": ("table[*source]", "table[destination_row[column]]"),
    "shrink_x_lowbyte": ("source += columns[column];", "source += static_cast<unsigned char>(columns[column]);"),
    "shrink_y_fullint": ("const unsigned char row_selector =\n                static_cast<unsigned char>(*SpriteRasterYTable++);",
                         "const int row_selector = *SpriteRasterYTable++;"),
    "expansion_zero_written": ("pixel_value != key && pixel_value != 0", "pixel_value != key"),
    "default_maps_source": ("? destination_row[output_column] : pixel_value", "? pixel_value : pixel_value"),
    "all_map_destination": ("? destination_row[output_column] : pixel_value", "? destination_row[output_column] : destination_row[output_column]"),
    "selector_not_lowbyte": ("const unsigned char selector = static_cast<unsigned char>(*SpriteRasterYTable++);",
                             "const int selector = *SpriteRasterYTable++;"),
    "selector_default_three": ("selector == 3 ? 3 : 4", "selector == 3 ? 3 : 3"),
    "selector_five_rows": ("selector == 3 ? 3 : 4", "selector == 3 ? 3 : selector == 5 ? 5 : 4"),
    "row_origin": ("destination_row += repeats * destination_pitch;", "destination_row += destination_pitch;"),
    "minimum_width_equal": ("original.right - original.left < SpriteMinimumScaledWidth", "original.right - original.left <= SpriteMinimumScaledWidth"),
    "minimum_height_equal": ("original.bottom - original.top < SpriteMinimumScaledHeight", "original.bottom - original.top <= SpriteMinimumScaledHeight"),
    "ratio_cap": ("SpriteScalingRatio > 4.0", "SpriteScalingRatio > 5.0"),
    "unit_unlock": ("destination->free_data(1);\n        return 0;\n    }\n\n    // Original FILD", "return 0;\n    }\n\n    // Original FILD"),
    "scaled_stale_pitch": ("\n    destination_pitch = static_cast<int>(destination->stride_);", "\n    (void)0;"),
    "scaled_new_base_pitch": ("original.top * destination_pitch", "original.top * static_cast<int>(destination->stride_)"),
    "coordinate_scaled": ("static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio", "(static_cast<double>(x) + iLeftOffset_) * SpriteScalingRatio"),
    "coordinate_partial_truncation": ("static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio", "static_cast<double>(x) + static_cast<int>(static_cast<double>(iLeftOffset_) * SpriteScalingRatio)"),
    "remap_wrong_pointer": ("SpriteRasterRemapTable.single = table;", "SpriteRasterRemapTable.single = table + 1;"),
    "remap_store_missing": ("SpriteRasterRemapTable.single = table;", "(void)table;"),
}


@pytest.mark.parametrize("damage", [*DAMAGE, "scaled_live_source", "scaled_live_destination", "second_clip_omitted", "cache_before_fill",
                                    "lut_snapshot", "source_snapshot", "default_remap_per_pixel", "remap_early_publish"])
def test_source_remap_rejects_compiling_damage(tmp_path, damage):
    parts = actual()
    source = parts["BODY"]
    if damage == "scaled_live_source":
        good = "source_row = scaled_source +"
        assert source.count(good) == 2
        source = source.replace(good, "source_row = reinterpret_cast<const unsigned char *>(pcBits_) +")
    elif damage == "scaled_live_destination":
        good = "destination_row = destination_bits +"
        assert source.count(good) == 2
        source = source.replace(good, "destination_row = static_cast<unsigned char *>(destination->locked_bits_) +")
    elif damage == "second_clip_omitted":
        good = "IntersectRect(&clipped, &original, &destination->rect1_);"
        assert source.count(good) == 2
        source = source.replace(good, "(void)0;")
    elif damage == "cache_before_fill":
        good = "fill_scaling_table();\n        SpriteCachedScaleNumerator = SpriteScaleNumerator;\n        SpriteCachedScaleDenominator = SpriteScaleDenominator;"
        assert source.count(good) == 1
        source = source.replace(good, "SpriteCachedScaleNumerator = SpriteScaleNumerator;\n        SpriteCachedScaleDenominator = SpriteScaleDenominator;\n        fill_scaling_table();")
    elif damage in ("lut_snapshot", "source_snapshot"):
        good = "const unsigned char key = static_cast<unsigned char>(transparent);"
        assert source.count(good) == 1
        injected = ("unsigned char copied_table[256]; std::memcpy(copied_table, table, 256); table = copied_table;"
                    if damage == "lut_snapshot" else
                    "unsigned char copied_source[4096]; std::memcpy(copied_source, pcBits_, 4096); pcBits_ = copied_source;")
        source = source.replace(good, injected + "\n" + good)
    elif damage == "default_remap_per_pixel":
        good = "destination_row[repeat * destination_pitch + output_column + pixel] = mapped;"
        assert source.count(good) == 1
        source = source.replace(good, "destination_row[repeat * destination_pitch + output_column + pixel] = repeats == 4 ? table[destination_row[output_column]] : mapped;")
    elif damage == "scaled_stale_pitch":
        good, bad = DAMAGE[damage]
        assert source.count(good) == 2
        before, tail = source.rsplit(good, 1)
        source = before + bad + tail
    elif damage == "remap_early_publish":
        good = "SpriteRasterYTable = get_clipped_y_table_expand(&original, &clipped,"
        assert source.count(good) == 1
        source = source.replace(good, "SpriteRasterRemapTable.single = table;\n        " + good)
    else:
        good, bad = DAMAGE[damage]
        assert source.count(good) == 1, damage
        source = source.replace(good, bad)
    parts["BODY"] = source
    result = run(tmp_path, parts)
    assert result.returncode == 99 and "assertion line" in result.stderr, result.stdout + result.stderr
