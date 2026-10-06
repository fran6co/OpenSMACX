"""Actual-source cached lighting, original621870/625080 bounded host controls.

Real declarations, constructors, cache helper and complete raster bodies are
extracted unchanged. Buffer lock/release and FPU APIs are explicit service
doubles; original native ABI, allocation, x87 register/status observations,
shared scratch reentrancy and game rendering are not established. Arithmetic
fixtures use valid extrema, fitting divisions, positive pitch and allocated
sample addresses. Owned zero/inverted post-clip spans and original divide traps
are outside the valid domain; borrowed empty-clipped countdown is tested.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from test_texture_vertices import braced, x87_span_step_expected

ROOT = Path(__file__).resolve().parents[2]


def one(pattern, text):
    item, = re.findall(pattern, text, re.M)
    return item


def actual_source():
    source = (ROOT / "src/texture.cpp").read_text()
    header = (ROOT / "src/texture.h").read_text()
    buffer = (ROOT / "src/buffer.h").read_text()
    methods = {}
    for name in ("texture_fixed", "texture_difference", "texture_span_step",
                 "texture_gouraud_edge", "texture_advance_gouraud_edge",
                 "texture_cached_edge", "texture_advance_cached_edge"):
        methods[name] = braced(source, "static int " + name + "(")
    methods["texture_light_step"] = braced(source, "static unsigned short texture_light_step(")
    methods["texture_span_pixels"] = braced(source, "static unsigned int texture_span_pixels(")
    methods["texture_reset_fpu"] = braced(source, "static void texture_reset_fpu(")
    methods["ctor"] = braced(source, "TextureStore::TextureStore()")
    methods["dtor"] = braced(source, "TextureStore::~TextureStore()")
    methods["select_next"] = braced(source, "EdgeScan *TextureStore::sub_6252c0()")
    for signature in re.findall(r"void Texture::draw_cached_lit\([^)]*\)", source):
        methods["draw" if signature.count(",") == 5 else "wrapper"] = braced(source, signature)
    assert "draw" in methods and "wrapper" in methods
    declarations = "\n".join([
        braced((ROOT / "src/vert.h").read_text(), "struct Vert {") + ";",
        braced(header, "struct EdgeScan {") + ";",
        braced(header, "struct EdgePoint {") + ";",
        braced(header, "class TextureStore {") + ";",
    ])
    texture = braced(header, "class Texture {")
    fields = []
    for name in ("pixels_", "iWidth_", "iHeight_", "corners_", "cornersAndMids_", "borrowed_"):
        fields.append(one(r"^\s*((?:void\s*\*|uint32_t\s+|EdgePoint\s+)" +
                          name + r"(?:\[[^\]]+\])?\s*;)", texture))
    signatures = re.findall(r"  void draw_cached_lit\([^;]+;", header)
    assert len(signatures) == 2
    globals_ = []
    for name in ("TextureRasterKey", "TextureLightFloor", "TextureLightCeiling", "TextureRowPitchScratch"):
        globals_.append(one(r"^(?:unsigned int|int) " + name + r"(?:\s*=\s*[^;]+)?;", source))
        one(r"^extern (?:unsigned int|int) " + name + ";", header)
    buffer_fields = [one(r"^\s*(RECT rect1_;)", buffer),
                     one(r"^\s*(uint32_t stride_;)", buffer)]
    return dict(DECLARATIONS=declarations,
                TEXTURE="\n".join(fields + signatures),
                BUFFER="\n".join(buffer_fields), GLOBALS="\n".join(globals_),
                METHODS=methods)


COMMON = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>
#define __cdecl
int scenario;
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::_Exit(101);}}
#define require(x) check((x),__LINE__)
DECLARATIONS
GLOBALS
struct RECT{int32_t left,top,right,bottom;};
std::vector<int> events;
const unsigned _MCW_EM=0x3f,_MCW_PC=0x300,_MCW_RC=0xc00,_MCW_IC=0x1000;
const unsigned _PC_64=0x300,_PC_24=0,_RC_NEAR=0,_RC_CHOP=0xc00,_IC_PROJECTIVE=0;
void _fpreset(){events.push_back(2);}
unsigned _control87(unsigned value,unsigned mask){
  if(mask==(_MCW_PC|_MCW_RC)){require(value==(_PC_24|_RC_CHOP));events.push_back(4);}
  else{require(mask==(_MCW_EM|_MCW_PC|_MCW_RC|_MCW_IC));
       require(value==(_MCW_EM|_PC_64|_RC_NEAR|_IC_PROJECTIVE));events.push_back(3);}
  return 0;
}
struct Buffer{BUFFER
  intptr_t get_data();void free_data(int);
};
struct Texture{TEXTURE};
METHODS
int truncation(int64_t numerator,int denominator){return int(numerator/denominator);}
void poison(TextureStore&cache){
  cache.flags_=0x99887766u;cache.next_record_=6;
  for(int n=0;n<10;++n)std::memset(&cache.edges_[n],0x11+n,sizeof(EdgeScan));
}
void edge_same(const EdgeScan&a,const EdgeScan&b){require(std::memcmp(&a,&b,sizeof a)==0);}
void cache_same(const TextureStore&a,const TextureStore&b){
  require(a.flags_==b.flags_&&a.next_record_==b.next_record_);
  require(std::memcmp(a.edges_,b.edges_,sizeof a.edges_)==0);
}
void helper_controls(){
  scenario=1;
  static_assert(sizeof(EdgeScan)==60,"host int-edge footprint");
  static_assert(sizeof(TextureStore)==608,"host cache footprint");
  static_assert(offsetof(TextureStore,flags_)==0&&offsetof(TextureStore,next_record_)==4&&offsetof(TextureStore,edges_)==8,"cache offsets");
  static_assert(std::is_same<decltype(TextureStore::flags_),unsigned int>::value,"flag type");
  static_assert(std::is_same<decltype(TextureStore::next_record_),int>::value,"cursor type");
  alignas(TextureStore) unsigned char raw[sizeof(TextureStore)];
  unsigned char bytes[sizeof raw];std::memset(raw,0xa5,sizeof raw);std::memcpy(bytes,raw,sizeof raw);
  uint32_t three=3,zero=0;std::memcpy(bytes,&three,4);std::memcpy(bytes+4,&zero,4);
  TextureStore*object=new(raw)TextureStore;
  require(std::memcmp(raw,bytes,sizeof raw)==0);
  object->flags_=81;object->next_record_=4;object->~TextureStore();
  require(std::memcmp(raw,bytes,sizeof raw)==0); // inspect storage, not dead members
  Vert vertices[]={{2,0},{6,0},{4,3},{0,3}};
  Vert uv[]={{1,2},{5,4},{6,8},{2,7}};
  int light[]={-2,3,7,1};
  for(unsigned flags:{0u,1u,2u,3u,0x100u,0x101u,0x102u,0x103u,0xffff0003u})
  for(int record:{0,3,9})for(int direction:{-1,1}){
    TextureStore cache,expected;poison(cache);cache.flags_=flags;cache.next_record_=record;expected=cache;
    EdgeScan actual,edge;std::memset(&actual,0x53,sizeof actual);actual.f0=direction;edge=actual;
    const int start=direction<0?0:1,next=direction<0?3:2;
    const int height=3,dx=vertices[next].x-vertices[start].x;
    edge.counter=height;edge.id=next;
    edge.accumA=uv[start].x*65536+32768;edge.accumB=uv[start].y*65536+32768;
    edge.accumC=vertices[start].x;edge.extraDeltaC=dx<0?-1:1;
    edge.fracAccum=dx<0?1-height:0;edge.fracSub=height;
    EdgeScan&stored=expected.edges_[record];
    if(flags&2){
      stored.deltaA=truncation(int64_t(uv[next].x)*65536-edge.accumA,height);
      stored.deltaB=truncation(int64_t(uv[next].y)*65536-edge.accumB,height);
      stored.deltaC=dx/height;stored.fracDelta=std::abs(dx)%height;
    }
    edge.deltaA=stored.deltaA;edge.deltaB=stored.deltaB;
    edge.deltaC=stored.deltaC;edge.fracDelta=stored.fracDelta;
    if(flags&1){stored.accumD=light[start]*65536;stored.deltaD=(light[next]-light[start])*65536/height;}
    edge.accumD=stored.accumD;edge.deltaD=stored.deltaD;
    expected.next_record_=record==9?0:record+1;
    require(texture_cached_edge(actual,0,vertices,uv,light,4,2,cache)==1);
    edge_same(actual,edge);cache_same(cache,expected);
  }
  // Positive/negative X remainders independently use strict >0 correction.
  for(int sign:{-1,1})for(int direction:{-1,1}){
    Vert geometry[4];std::memcpy(geometry,vertices,sizeof geometry);
    for(Vert&v:geometry)v.x*=sign;
    TextureStore cache;poison(cache);cache.flags_=3;cache.next_record_=0;
    EdgeScan edge={};edge.f0=direction;
    require(texture_cached_edge(edge,0,geometry,uv,light,4,2,cache));
    EdgeScan initial=edge;
    for(int row=1;row<3;++row){
      require(texture_advance_cached_edge(edge,geometry,uv,light,4,2,cache));
      // dx=-2 gives start,start,start-1; dx=+2 gives start,start+1,start+2.
      require(edge.accumC==initial.accumC+(sign<0?row:-(row==2)));
      require(edge.accumA==initial.accumA+row*initial.deltaA);
      require(edge.accumB==initial.accumB+row*initial.deltaB);
      require(edge.accumD==initial.accumD+row*initial.deltaD);
      require(cache.next_record_==1);
    }
    TextureStore saved=cache;
    require(!texture_advance_cached_edge(edge,geometry,uv,light,4,2,cache));
    cache_same(cache,saved);
  }
  // Skipping an all-horizontal run and rejecting a descending edge do not
  // consume a cache record. A bottom-index input leaves the edge untouched.
  for(int mode=0;mode<3;++mode){
    TextureStore cache;poison(cache);TextureStore saved=cache;
    Vert line[]={{0,2},{1,2},{2,2}};if(mode==2)line[1].y=1;
    EdgeScan edge={};edge.f0=1;EdgeScan old=edge;
    require(!texture_cached_edge(edge,mode==0?2:0,line,uv,light,3,2,cache));
    cache_same(cache,saved);if(mode==0)edge_same(edge,old);
  }
}
'''


def run_host(tmp_path, parts, main):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = COMMON + main
    for key, value in parts.items():
        source = source.replace(key, "\n\n".join(value.values()) if key == "METHODS" else value)
    cpp, binary = tmp_path / "cached.cpp", tmp_path / "cached"
    cpp.write_text(source)
    compiled = subprocess.run([compiler,"-std=c++17","-O0","-g",
                               "-fsanitize=address,undefined","-fno-sanitize-recover=all",
                               str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=30,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


RASTER = r'''
// The first64KiB is the genuine packed-page domain. Extra poisoned backing
// keeps the wrong-full-width owned row-product mutant in allocated memory,
// so it fails the pixel oracle instead of merely causing an ASan crash.
alignas(65536) unsigned char page[8*65536],stale_page[8*65536];
unsigned char output[2048],expected[2048];
Texture texture;
Buffer buffer;
TextureStore cache;
RECT published_clip;
int published_pitch,published_floor,published_ceiling,published_mode,published_shared_pitch;
unsigned published_key;
bool null_lock,alias_pixels,mutate_release;
int seen_cursor,release_cursor;
unsigned release_flags;
intptr_t Buffer::get_data(){
  require(events.empty());events.push_back(1);
  require(cache.next_record_==0);seen_cursor=cache.next_record_;
  rect1_=published_clip;stride_=published_pitch;
  TextureRasterKey=published_key;TextureLightFloor=published_floor;TextureLightCeiling=published_ceiling;
  TextureRowPitchScratch=published_shared_pitch;
  texture.pixels_=page;texture.borrowed_=published_mode;
  texture.iWidth_=17;texture.iHeight_=256; // distinct from shared pitch and packed page
  if(null_lock)return 0;
  return reinterpret_cast<intptr_t>(alias_pixels?page:output+256);
}
void Buffer::free_data(int count){
  require(count==1&&events.front()==1);events.push_back(5);
  release_flags=cache.flags_;release_cursor=cache.next_record_;
  if(mutate_release)cache.flags_=0xfeed0103u;
}
void start(bool preserve_cache=false){
  events.clear();std::memset(output,0xee,sizeof output);std::memcpy(expected,output,sizeof output);
  std::memset(page,0x4d,sizeof page);
  for(int y=0;y<256;++y)for(int x=0;x<256;++x)page[y*256+x]=static_cast<unsigned char>(x*37+y*71+13);
  if(!preserve_cache){poison(cache);cache.flags_=3;}
  cache.next_record_=7;
  std::memset(stale_page,0x4d,sizeof stale_page);
  texture.pixels_=stale_page;texture.iWidth_=1;texture.iHeight_=1;texture.borrowed_=0;
  buffer.stride_=1;buffer.rect1_={0,0,0,0};
  published_clip={0,-4,16,12};published_pitch=32;
  published_floor=-256;published_ceiling=256;published_mode=1;
  published_shared_pitch=9;published_key=0x123400ff;
  TextureRasterKey=0;TextureLightFloor=91;TextureLightCeiling=91;TextureRowPitchScratch=1;
  null_lock=false;alias_pixels=false;mutate_release=false;seen_cursor=-1;
  release_flags=0;release_cursor=-1;
}
void finish(Vert*v,int*z,int count,Vert*uv,bool pc24=true,bool contained=true){
  const unsigned flags=cache.flags_;
  texture.draw_cached_lit(&buffer,v,z,count,uv,&cache);
  require(events==(pc24?std::vector<int>{1,2,3,4,5,2,3}:std::vector<int>{1,2,3,5,2,3}));
  require(seen_cursor==0&&release_flags==flags);
  require(cache.flags_==(contained?0u:(mutate_release?0xfeed0103u:flags)));
  require(TextureRowPitchScratch==published_shared_pitch);
  if(!alias_pixels)require(std::memcmp(output,expected,sizeof output)==0);
}
void raster_controls(){
  struct Slope{int delta,width,uv;unsigned light;};
  const Slope slopes[]={SLOPES};
  for(const auto&s:slopes){require(texture_span_step(s.delta,s.width)==s.uv);require(texture_light_step(s.delta,s.width)==s.light);}
  // Complete borrowed rectangles use independent rational edge/plane oracles.
  struct Case{int width,height,left,right,top,bottom;Vert uv[4];int light[4];unsigned key;int floor,ceiling;};
  const Case cases[]={CASES};
  const std::vector<std::vector<int> > samples={SAMPLES};
  for(size_t n=0;n<sizeof(cases)/sizeof(cases[0]);++n){
    scenario=100+int(n);start();const Case&c=cases[n];
    Vert v[]={{1,1},{1+c.width,1},{1+c.width,1+c.height},{1,1+c.height}};
    Vert uv[4];int z[4];std::memcpy(uv,c.uv,sizeof uv);std::memcpy(z,c.light,sizeof z);
    published_clip={c.left,c.top,c.right,c.bottom};published_key=c.key;
    published_floor=c.floor;published_ceiling=c.ceiling;
    const auto&s=samples[n];require(s.size()%4==0);
    for(size_t p=0;p<s.size();p+=4){
      const int x=s[p],y=s[p+1],source=s[p+2],shade=s[p+3];
      require(source>=0&&source<65536);unsigned char texel=page[source];
      if(texel!=static_cast<unsigned char>(c.key))expected[256+y*32+x]=static_cast<unsigned char>(texel+shade);
    }
    bool contained=1>=c.left&&1>=c.top&&1+c.width<=c.right&&1+c.height<=c.bottom;
    mutate_release=true;finish(v,z,4,uv,true,contained);
    for(int i=0;i<4;++i)require(z[i]==std::max(c.floor,std::min(c.ceiling,c.light[i])));
  }
  // Literal original owned-kernel coordinate traces, not an implementation of
  // the candidate's packed stepping.621E92 keeps rightX in the high dividend;
  //6212078 leaves the copied negative-U V fraction unshifted. Shared pitch9
  // differs deliberately from the acquired Texture width17.
  struct Owned{int ur,vr,skip;int source[4];};
  const Owned owned[]={
    {7,8,0,{50,60,69,79}},{7,8,1,{60,69,79}},{7,8,2,{69,79}},
    {3,8,0,{50,50,58,67}},{3,8,1,{59,58,58}},{3,8,2,{67,76}},
    {7,2,0,{50,42,42,34}},{7,2,1,{42,42,34}},{7,2,2,{42,34}},
    {3,2,0,{50,41,31,31}},{3,2,1,{41,40,31}},{3,2,2,{40,40}},
    {5,5,0,{50,50,50,50}},{5,5,1,{50,50,50}},{5,5,2,{50,50}},
    {9,5,0,{51,52,53,54}},{9,5,1,{52,53,54}},{9,5,2,{53,54}},
    {1,5,0,{50,58,57,56}},{1,5,1,{49,57,56}},{1,5,2,{48,56}},
    {5,9,0,{59,68,77,86}},{5,9,1,{68,77,86}},{5,9,2,{77,86}},
    {4,4,0,{50,50,50,50}},{4,4,1,{50,50,41}},{4,4,2,{50,41}},
    {6,4,0,{50,50,51,51}},{6,4,1,{50,51,51}},{6,4,2,{51,51}},
    {4,6,0,{50,59,59,68}},{4,6,1,{50,50,50}},{4,6,2,{59,59}},
    {6,6,0,{50,50,60,60}},{6,6,1,{50,60,60}},{6,6,2,{60,60}},
  };
  const int shades[]={0,126,253,124};
  for(const Owned&c:owned){
    scenario=500+c.ur*10+c.vr+c.skip;start();published_mode=0;
    Vert v[]={{1,1},{5,1},{5,2},{1,2}};
    Vert uv[]={{5,5},{c.ur,c.vr},{c.ur,c.vr},{5,5}};int z[]={2,-3,-3,2};
    published_clip.left=1+c.skip;published_key=page[c.source[0]]; // owned does not skip key
    for(int col=0;col<4-c.skip;++col)
      expected[256+32+1+c.skip+col]=static_cast<unsigned char>(page[c.source[col]]+shades[c.skip+col]);
    finish(v,z,4,uv,false,c.skip==0);require(release_cursor==2);
  }
  // Initial integer source-row multiplication keeps only16 bits, independent
  // of the much larger shared pitch. No fractional V correction occurs here.
  scenario=890;start();published_mode=0;published_shared_pitch=65537;
  Vert wide_v[]={{1,1},{5,1},{5,2},{1,2}},wide_uv[]={{5,5},{9,5},{9,5},{5,5}};
  int wide_z[]={2,-3,-3,2};
  for(int col=0;col<4;++col)expected[256+32+1+col]=static_cast<unsigned char>(page[11+col]+shades[col]);
  finish(wide_v,wide_z,4,wide_uv,false);
  // Signed negative rightX is the HIGH half of the owned light dividend.
  scenario=891;start();published_mode=0;published_clip.left=-8;
  Vert negative_v[]={{-5,1},{-1,1},{-1,2},{-5,2}},negative_uv[4]={};int negative_z[4]={};
  const int backwards_shades[]={0,192,128,64};
  for(int col=0;col<4;++col)expected[256+32-5+col]=static_cast<unsigned char>(page[0]+backwards_shades[col]);
  finish(negative_v,negative_z,4,negative_uv,false);
  scenario=892;start();Vert above[]={{1,-2},{5,-2},{5,0},{1,0}};
  for(int y=-2;y<0;++y)for(int x=1;x<5;++x)expected[256+y*32+x]=page[0];
  finish(above,negative_z,4,negative_uv);
  // Borrowed source/destination alias: each newly written byte becomes the
  // next source texel. A whole-row source snapshot would incorrectly diverge.
  scenario=900;start();alias_pixels=true;published_clip={0,0,16,1};published_key=255;
  Vert a[]={{1,0},{5,0},{5,1},{1,1}},au[]={{0,0},{4,0},{4,0},{0,0}};int az[]={1,1,1,1};
  unsigned char saved_page[sizeof page];std::memcpy(saved_page,page,sizeof page);
  for(int i=1;i<=4;++i)saved_page[i]=static_cast<unsigned char>(13+i);
  finish(a,az,4,au);require(std::memcmp(page,saved_page,sizeof page)==0);
  // Cached replay ignores changed light values when bit1 is clear, even
  // though the new input array is still clamped in place at entry.
  scenario=901;start();Vert r[]={{1,1},{5,1},{5,2},{1,2}},zero_uv[4]={};int light[]={2,2,2,2};
  for(int x=1;x<5;++x)expected[256+32+x]=static_cast<unsigned char>(page[0]+2);
  finish(r,light,4,zero_uv);TextureStore preserved=cache;
  start(true);for(int&z:light)z=99;published_ceiling=7;
  for(int x=1;x<5;++x)expected[256+32+x]=static_cast<unsigned char>(page[0]+2);
  finish(r,light,4,zero_uv);for(int z:light)require(z==7);
  require(std::memcmp(cache.edges_,preserved.edges_,sizeof cache.edges_)==0);
  //621C40 compares only the transparent key: non-key zero is still shaded.
  scenario=902;start();page[0]=0;published_key=255;
  for(int&z:light)z=2;
  for(int x=1;x<5;++x)expected[256+32+x]=2;
  finish(r,light,4,zero_uv);
  // Each positive-height edge consumes a cache slot only in the borrowed
  // refresh path. Twelve edges wrap the10-record ring; owned refresh uses
  // uncached61FC90 and leaves records2..9 poisoned.
  Vert many[]={{1,0},{5,0},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},
               {1,6},{1,5},{1,4},{1,3},{1,2},{1,1}};
  Vert many_uv[14]={};int many_light[14]={};
  for(int owned_mode=0;owned_mode<2;++owned_mode){
    scenario=910+owned_mode;start();published_mode=owned_mode?0:1;
    EdgeScan original_record2=cache.edges_[2];
    for(int y=0;y<6;++y)for(int x=1;x<5;++x)
      expected[256+y*32+x]=static_cast<unsigned char>(page[0]+(owned_mode?(x-1)*64:0));
    finish(many,many_light,14,many_uv,!owned_mode);require(release_cursor==2);
    require((std::memcmp(&cache.edges_[2],&original_record2,sizeof(EdgeScan))==0)==bool(owned_mode));
  }
  // A concrete descending left-chain failure leaves the right refresh
  // uncalled but still draws the saved row. This is a bounded admitted prefix
  // of a non-monotone polygon, not a general nonconvex filling claim.
  scenario=915;start();
  Vert interrupted[]={{1,0},{5,0},{5,1},{5,2},{5,6},{1,1},{1,2}};
  Vert interrupted_uv[7]={};int interrupted_light[7]={};
  for(int y=0;y<2;++y)for(int x=1;x<5;++x)expected[256+y*32+x]=page[0];
  finish(interrupted,interrupted_light,7,interrupted_uv);require(release_cursor==3);
  // No null Buffer/polygon guards are invented. Null acquired pixels can be
  // exercised safely only on a rejected zero-count polygon.
  for(int count:{-1,0}){
    scenario=920+count;start();null_lock=true;mutate_release=true;
    int z[]={-999,1,2,999};finish(r,z,count,zero_uv,false,false);
    require(z[0]==-999&&z[3]==999&&release_cursor==0);
  }
  for(bool reversed:{false,true}){
    scenario=930+reversed;start();published_floor=reversed?5:-4;published_ceiling=reversed?-3:4;
    int z[]={-2147483647,0,4,2147483647};Vert flat[]={{1,2},{5,2},{5,2},{1,2}};
    finish(flat,z,4,zero_uv,false,false);require(release_cursor==0);
    const int clamped[]={-4,0,4,4};for(int n=0;n<4;++n)require(z[n]==(reversed?5:clamped[n]));
  }
  // Non--1 sentinel does not touch cache, clamp inputs, acquire or reset FPU.
  for(int sentinel:{0,1,-2}){
    scenario=940+sentinel;start();TextureStore old=cache;int z[]={-999,0,0,999};
    texture.draw_cached_lit(&buffer,r,z,4,zero_uv,&cache,sentinel);
    require(events.empty());cache_same(cache,old);require(z[0]==-999&&z[3]==999);
  }
  scenario=950;start();
  for(int&z:light)z=0;
  for(int x=1;x<5;++x)expected[256+32+x]=page[0];
  texture.draw_cached_lit(&buffer,r,light,4,zero_uv,&cache,-1);
  require(events==std::vector<int>({1,2,3,4,5,2,3}));require(cache.flags_==0);
  require(std::memcmp(output,expected,sizeof output)==0);
  // The wrapper forwards the complete count even when geometry is rejected:
  // entry clamps every input before discovering that all vertices share a Y.
  scenario=951;start();Vert wrapper_flat[]={{1,2},{5,2},{5,2},{1,2}};
  int wrapper_light[]={0,0,0,999};
  texture.draw_cached_lit(&buffer,wrapper_flat,wrapper_light,4,zero_uv,&cache,-1);
  require(wrapper_light[3]==256);
  require(events==std::vector<int>({1,2,3,5,2,3}));
  require(std::memcmp(output,expected,sizeof output)==0);
}
int main(){helper_controls();raster_controls();return 0;}
'''


def truncate(numerator, denominator):
    result = abs(numerator) // abs(denominator)
    return -result if (numerator < 0) != (denominator < 0) else result


def light_step(delta, width):
    scaled = x87_span_step_expected(delta, width)
    value = truncate(scaled, 256)
    return value & 65535 if -32768 <= value <= 32767 else 32768


def raster_main():
    slopes = []
    for width in (1,2,3,5,7,31,127,32767):
        for delta in (0,1,-1,65536,-65536,196608,-196608,8388607,8388608,-8388609,2147483647,-2147483648):
            slopes.append("{%d,%d,%d,%d}" % (delta,width,x87_span_step_expected(delta,width),light_step(delta,width)))
    cases, samples = [], []
    uvs = [((0,0),(0,0),(0,0),(0,0)),
           ((255,3),(257,6),(260,7),(258,4)),
           ((5,6),(3,3),(1,1),(3,4)),
           ((0,0),(1,0),(1,0),(0,0))]
    lights = [(-300,-128,128,300),(-3,3,7,1),(7,-3,-7,3),(0,1,1,0)]
    for width,height in ((4,4),(3,3),(5,2)):
        for uv,values in zip(uvs,lights):
            for left,right,top,bottom in ((0,16,-4,12),(3,5,2,4),(3,3,-4,12),(0,16,4,5)):
                for key in (0,13,255):
                    floor,ceiling=-4,4
                    z=[max(floor,min(ceiling,x)) for x in values]
                    rows=[]
                    for row in range(height):
                        y=1+row
                        if not top <= y < bottom:
                            continue
                        start,end=1,1+width
                        if not(end>left and start<right):
                            continue
                        # Closed-form independently integrated vertical edges;
                        # half-texel starts approach unbiased endpoint targets.
                        channel=[]
                        for axis in (0,1):
                            a=uv[0][axis]*65536+32768;b=uv[1][axis]*65536+32768
                            a+=row*truncate(uv[3][axis]*65536-a,height)
                            b+=row*truncate(uv[2][axis]*65536-b,height)
                            channel.append((a,x87_span_step_expected(b-a,width)))
                        a=z[0]*65536+row*truncate((z[3]-z[0])*65536,height)
                        b=z[1]*65536+row*truncate((z[2]-z[1])*65536,height)
                        dz=light_step(b-a,width)
                        x=max(start,left);end=min(end,right);skip=x-start
                        brightness=((a+((dz*(skip<<16))>>8))>>8)&65535
                        # Small bounded ordinary or empty-clipped packed spans.
                        for column in range(max(1,end-x)):
                            u=(channel[0][0]+(skip+column)*channel[0][1])>>16
                            v=(channel[1][0]+(skip+column)*channel[1][1])>>16
                            shade=((brightness+column*dz)&65535)>>8
                            rows.extend([x+column,y,(v&255)*256+(u&255),shade])
                    uv_text="{"+",".join("{%d,%d}"%v for v in uv)+"}"
                    z_text="{"+",".join(map(str,values))+"}"
                    cases.append("{%d,%d,%d,%d,%d,%d,%s,%s,%d,%d,%d}"%
                                 (width,height,left,right,top,bottom,uv_text,z_text,key,floor,ceiling))
                    samples.append("{"+",".join(map(str,rows))+"}")
    return RASTER.replace("SLOPES",",".join(slopes)).replace("CASES",",".join(cases)).replace("SAMPLES",",".join(samples))


def test_actual_cached_lighting(tmp_path):
    result=run_host(tmp_path,actual_source(),raster_main())
    assert result.returncode==0,result.stderr


DAMAGE = [
    ("ctor_flags","ctor","flags_ = 3;","flags_ = 0;"),
    ("ctor_tail","ctor","flags_ = 3;","memset(edges_,0,sizeof edges_); flags_ = 3;"),
    ("dtor_cursor","dtor","(&next_record_) = 0;","(&next_record_) = 1;"),
    ("slot_selection","select_next","&edges_[next_record_]","&edges_[(next_record_+1)%10]"),
    ("slot_wrap","select_next","if (next == 10)","if (next == 9)"),

    ("uv_flag","texture_cached_edge","cache.flags_ & 2","cache.flags_ & 1"),
    ("light_flag","texture_cached_edge","cache.flags_ & 1","cache.flags_ & 2"),
    ("half_texel","texture_cached_edge","+ 0x8000u","+ 0u"),
    ("cache_v_replay","texture_cached_edge","edge.deltaB = record.deltaB;","edge.deltaB = record.deltaA;"),
    ("cache_x_replay","texture_cached_edge","edge.deltaC = record.deltaC;","edge.deltaC = 0;"),
    ("cache_light_replay","texture_cached_edge","edge.accumD = record.accumD;","edge.accumD = texture_fixed(light[index]);"),
    ("cache_footprint","texture_cached_edge","edge.id = next;","record.id = next; edge.id = next;"),
    ("advance_light","texture_advance_cached_edge","edge.accumD) + edge.deltaD","edge.accumD) + 0"),
    ("fraction_tie","texture_advance_cached_edge","edge.fracAccum > 0","edge.fracAccum >= 0"),
    ("sentinel","wrapper","sentinel == -1","sentinel != -1"),
    ("wrapper_count","wrapper","zvals, count, edgeVerts, cache","zvals, count-1, edgeVerts, cache"),
    ("initial_cursor","draw","cache->next_record_ = 0;",""),
    ("light_floor","draw","if (value <= TextureLightFloor) value = TextureLightFloor;",""),
    ("light_ceiling","draw","if (value >= TextureLightCeiling) value = TextureLightCeiling;",""),
    ("borrowed_key","draw","if (texel != key)","if (texel == key)"),
    ("borrowed_zero","draw","if (texel != key)","if (texel != key && texel != 0)"),
    ("borrowed_lighting","draw","texel + (brightness >> 8)","texel"),
    ("borrowed_preincrement","draw","texel + (brightness >> 8)","texel + (static_cast<unsigned short>(brightness + shade_step) >> 8)"),
    ("borrowed_page_pitch","draw","source_y * 256u + source_x","source_y * 17u + source_x"),
    ("borrowed_clip_light","draw","(static_cast<int64_t>(shade_step) * skip) >> 8","(static_cast<int64_t>(shade_step) * skip) >> 16"),
    ("borrowed_empty_clip","draw","texture_span_pixels(texture_difference(end, x))","static_cast<unsigned int>(texture_difference(end, x))"),
    ("left_failure_skips_right","draw","bottom, *cache) ||","bottom, *cache) |"),
    ("terminal_saved_span","draw","if (span > 0) {","if (done) break; if (span > 0) {"),
    ("owned_high_dividend","draw","static_cast<int64_t>(end) * 65536 * 65536 + low","static_cast<int64_t>(low)"),
    ("owned_light_initial","draw","unsigned int shade = static_cast<unsigned int>(left.accumD);","unsigned int shade = static_cast<unsigned int>(left.accumD >> 8);"),
    ("owned_half_step","draw","(u) + (adjusted_u >> 1)","(u) + 0"),
    ("owned_shared_pitch","draw","int signed_pitch = TextureRowPitchScratch;","int signed_pitch = static_cast<int>(iWidth_);"),
    ("owned_row_product","draw","int64_t source = static_cast<unsigned short>(","int64_t source = static_cast<unsigned int>("),
    ("owned_clip_light","draw","(static_cast<int64_t>(shade_step) * skip) >> 16","(static_cast<int64_t>(shade_step) * skip) >> 8"),
    ("owned_negative_u_fraction","draw","(static_cast<unsigned int>(~u) << 16) | static_cast<unsigned short>(v)","(static_cast<unsigned int>(~u) << 16) | (static_cast<unsigned short>(v) >> 1)"),
    ("owned_sbb","draw","reverse_u ? -int(carry) : int(carry)","int(carry)"),
    ("owned_negative_v","draw","v = ~v;","v = v;"),
    ("owned_key_filter","draw","out[column] = static_cast<unsigned char>(texel + ((shade >> 8) & 0xff));","if (texel != key) out[column] = static_cast<unsigned char>(texel + ((shade >> 8) & 0xff));"),
    ("owned_refresh_cache","draw","texture_advance_gouraud_edge(left, vertices, uv, light, count, bottom)","texture_advance_cached_edge(left, vertices, uv, light, count, bottom, *cache)"),
    ("release_count","draw","buffer->free_data(1)","buffer->free_data(2)"),
    ("final_flags_guard","draw","if (!clipped) cache->flags_ = 0;","cache->flags_ = 0;"),
    ("missing_flag_clear","draw","if (!clipped) cache->flags_ = 0;",""),
    ("entry_fpu","draw","texture_reset_fpu();",""),
    ("span_reciprocal","texture_span_step","int reciprocal_shift = 23;","int reciprocal_shift = 24;"),
    ("word_overflow","texture_light_step","return 0x8000u;","return 0u;"),
]


@pytest.mark.parametrize("name,method,before,after",DAMAGE,ids=[row[0] for row in DAMAGE])
def test_compiling_cached_lighting_damage(tmp_path,name,method,before,after):
    parts=actual_source()
    assert before in parts["METHODS"][method],name
    parts["METHODS"][method]=parts["METHODS"][method].replace(before,after,1)
    result=run_host(tmp_path,parts,raster_main())
    assert result.returncode==101 and "assertion line" in result.stderr,(name,result.returncode,result.stderr)


@pytest.mark.parametrize("declaration",[
    "const unsigned char key = static_cast<unsigned char>(TextureRasterKey);",
    "const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);",
    "const int stride = static_cast<int>(buffer->stride_);",
    "const RECT clip = buffer->rect1_;",
])
def test_acquisition_publication_cannot_be_captured_early(tmp_path,declaration):
    parts=actual_source();actual=parts["METHODS"]["draw"]
    assert actual.count(declaration)==1
    actual=actual.replace(declaration,"")
    opening=actual.index("{")+1
    parts["METHODS"]["draw"]=actual[:opening]+declaration+actual[opening:]
    result=run_host(tmp_path,parts,raster_main())
    assert result.returncode==101 and "assertion line" in result.stderr,(result.returncode,result.stderr)


def test_release_observes_flags_before_final_clear(tmp_path):
    parts=actual_source();actual=parts["METHODS"]["draw"]
    ending="if (!clipped) cache->flags_ = 0;"
    assert actual.count(ending)==1
    actual=actual.replace(ending,"").replace("buffer->free_data(1);",ending+" buffer->free_data(1);",1)
    parts["METHODS"]["draw"]=actual
    result=run_host(tmp_path,parts,raster_main())
    assert result.returncode==101 and "assertion line" in result.stderr,(result.returncode,result.stderr)
