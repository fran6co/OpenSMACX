"""Actual-source Buffer remapping/fill controls with explicit host doubles.

Both remappers, free_data, fill and fields are extracted unchanged from src.
The coordinate overload uses a scripted get_data double; the direct RECT path
executes its actual acquisition and cleanup with a configurable DirectDraw Lock.
Independent pixel oracles and damage cases check aliasing, pitch publication,
nested/DIB state and native no-release failure paths. Host controls and source
ordering assertions do not prove native ABI, DirectDraw runtime or game
acceptance. Host-width numeric wrapping is analogous to, not an exact 32-bit
proof of, the native zero-address check; no fabricated pointer is dereferenced
on the supported zero-address path.
"""

from pathlib import Path
import os
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
    raise AssertionError("unterminated source")


def actual_source():
    header = (ROOT / "src/buffer.h").read_text()
    fields = []
    for pattern in (r"RECT rect1_;", r"Dib dib_;", r"uint32_t stride_;",
                    r"LPVOID locked_bits_;", r"LPVOID dib_bits_;",
                    r"IDirectDrawSurface2 \*surface_;", r"int surface_lock_count_;"):
        declaration, = re.findall(pattern, header)
        fields.append(declaration)
    source = (ROOT / "src/buffer.cpp").read_text()
    bodies = [braced(source, signature) for signature in (
        "int Buffer::map_colors(int left", "int Buffer::map_colors(RECT *area",
        "void Buffer::free_data(int count)", "int Buffer::fill(RECT *area, int color)")]
    row_gap, = re.findall(r"uint32_t BufferFillRowGap;", source)
    bodies[0] = row_gap + "\n" + bodies[0]
    return "\n".join(fields), bodies


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
typedef void *LPVOID;
struct RECT {int32_t left,top,right,bottom;};
struct Dib {struct {int32_t biWidth,biHeight;} bmiHeader;};
struct DDBLTFX {uint32_t dwSize,dwFillColor;};
struct DDSURFACEDESC {uint32_t dwSize;int lPitch;void *lpSurface;};
const int DDLOCK_WAIT=1,DDBLT_COLORFILL=2,DDBLT_WAIT=4;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(95);}}
#define require(x) check((x),__LINE__)
struct IDirectDrawSurface2 {
    int Lock(void *,DDSURFACEDESC *,int,void *);
    int Unlock(void *);
    int Blt(RECT *,void *,void *,int,DDBLTFX *);
} surface;
int IntersectRect(RECT *out,const RECT *a,const RECT *b){
    RECT value={std::max(a->left,b->left),std::max(a->top,b->top),
                std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
    if(value.left>=value.right || value.top>=value.bottom){*out={0,0,0,0};return 0;}
    *out=value;return 1;
}
struct Buffer {
    ACTUAL_FIELDS
    void *acquire_pixels();void free_data(int);
    int map_colors(int,int,int,int,unsigned char *);
    int map_colors(RECT *,unsigned char *);
    int fill(RECT *,int);
} buffer;
ACTUAL_BODIES
alignas(16) unsigned char pixels[4096],expected[4096];
unsigned char lut[256];
int get_calls,unlock_calls,blt_calls,mode,published_pitch;
int lock_calls,lock_status,lock_pitch;void *lock_bits;
bool publish_pitch;
unsigned char *published_pixels;
RECT blt_rect;int blt_color;
void *Buffer::acquire_pixels(){
    require(this==&buffer);++get_calls;
    if(mode==2)return nullptr; // acquisition refused: no publication/count change
    if(publish_pitch)stride_=published_pitch;
    if(mode==3){locked_bits_=nullptr;++surface_lock_count_;return nullptr;}
    locked_bits_=published_pixels;++surface_lock_count_;return published_pixels;
}
int IDirectDrawSurface2::Lock(void *area,DDSURFACEDESC *description,int flags,void *event){
    require(this==&surface && area==nullptr && event==nullptr);
    require(flags==DDLOCK_WAIT && description->dwSize==sizeof(DDSURFACEDESC));
    ++lock_calls;
    if(mode==2 || lock_status!=0)return mode==2?1:lock_status;
    description->lPitch=mode==3?published_pitch:lock_pitch;
    description->lpSurface=mode==3?nullptr:lock_bits;
    return 0;
}
int IDirectDrawSurface2::Unlock(void *p){
    require(this==&surface && p==published_pixels);++unlock_calls;return 37;
}
int IDirectDrawSurface2::Blt(RECT *area,void *a,void *b,int flags,DDBLTFX *fx){
    require(this==&surface && a==nullptr && b==nullptr && flags==(DDBLT_COLORFILL|DDBLT_WAIT));
    require(fx->dwSize==sizeof(DDBLTFX));blt_rect=*area;blt_color=fx->dwFillColor;++blt_calls;return 41;
}
void start(){
    for(unsigned i=0;i<sizeof(pixels);++i)pixels[i]=(i*31+17)%251;
    for(int i=0;i<256;++i)lut[i]=(i*7+3)&255;
    std::memcpy(expected,pixels,sizeof(pixels));
    buffer.rect1_={0,0,20,12};buffer.dib_.bmiHeader={20,-12};buffer.stride_=24;
    buffer.dib_bits_=pixels+512;buffer.locked_bits_=nullptr;buffer.surface_=nullptr;buffer.surface_lock_count_=0;
    get_calls=unlock_calls=blt_calls=lock_calls=0;mode=0;publish_pitch=false;published_pitch=24;published_pixels=pixels+512;
    lock_status=0;lock_pitch=24;lock_bits=pixels+512;
    BufferFillRowGap=777;
}
void unchanged(){require(std::memcmp(pixels,expected,sizeof(pixels))==0);}
RECT intersection(RECT a,RECT b){return {std::max(a.left,b.left),std::max(a.top,b.top),std::min(a.right,b.right),std::min(a.bottom,b.bottom)};}
void remap_oracle(RECT area,int old_pitch,int new_pitch,bool alias){
    RECT r=intersection(area,buffer.rect1_);
    for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x){
        int offset=512+new_pitch*r.top+old_pitch*(y-r.top)+x;
        unsigned char value=expected[offset];
        expected[offset]=alias ? expected[512+value] : lut[value];
    }
}
void remaps(){
    for(bool rect_form : {false,true})for(bool acquired_surface : {false,true})
      for(bool nested : {false,true})for(bool changed_pitch : {false,true}){
        if(rect_form)continue; // direct RECT acquisition has dedicated controls below
        start();buffer.surface_=acquired_surface?&surface:nullptr;
        buffer.surface_lock_count_=nested?3:0;
        if(nested)buffer.locked_bits_=published_pixels;
        RECT area={2,2,9,6};RECT requested=area;
        if(rect_form){++requested.right;++requested.bottom;}
        publish_pitch=changed_pitch;published_pitch=changed_pitch?32:24;
        remap_oracle(area,24,published_pitch,false);
        int result=rect_form?buffer.map_colors(&requested,lut):buffer.map_colors(2,2,9,6,lut);
        require(result==0 && get_calls==1);unchanged();
        require(buffer.surface_lock_count_==(nested?3:0));
        require(buffer.locked_bits_==(nested?published_pixels:nullptr));
        require(unlock_calls==(acquired_surface && !nested));
        require(buffer.stride_==uint32_t(published_pitch));
    }
    for(bool rect_form : {false,true}){
        start();buffer.stride_=16;buffer.rect1_={0,0,8,4};
        // Destination bytes are themselves table entries. The first write
        // changes entry0, which a later pixel consults: copying LUT is wrong.
        pixels[512]=1;pixels[513]=0;pixels[514]=0;
        std::memcpy(expected,pixels,sizeof(pixels));
        remap_oracle({0,0,8,3},16,16,true);
        RECT rect={0,0,9,4};
        require((rect_form?buffer.map_colors(&rect,pixels+512):buffer.map_colors(0,0,8,3,pixels+512))==0);
        unchanged();require(get_calls==(rect_form?0:1) && buffer.surface_lock_count_==0);
    }
    for(RECT clip : {RECT{0,0,20,12},RECT{3,2,10,7},RECT{1,1,2,2}})
      for(RECT input : {RECT{-3,-2,9,6},RECT{0,0,20,12},RECT{4,3,30,20}}){
        start();buffer.rect1_=clip;RECT r=intersection(input,clip);
        remap_oracle(input,24,24,false);
        require(buffer.map_colors(input.left,input.top,input.right,input.bottom,lut)==0);
        unchanged();require(get_calls==int(r.left<r.right && r.top<r.bottom));
    }
    start();RECT rect={0,0,5,5};
    require(buffer.map_colors(nullptr,lut)==16 && buffer.map_colors(&rect,nullptr)==16);
    require(buffer.map_colors(0,0,5,5,nullptr)==16 && get_calls==0);unchanged();
    for(RECT area : {RECT{1,1,1,3},RECT{3,3,1,1},RECT{21,0,25,4},RECT{0,13,4,15}}){
        start();require(buffer.map_colors(area.left,area.top,area.right,area.bottom,lut)==0);
        require(get_calls==0);unchanged();
    }
    start();rect={2,2,3,3};require(buffer.map_colors(&rect,lut)==0 && get_calls==0);unchanged();
    for(bool vertical : {false,true}){
        start();buffer.rect1_={0,0,100,100};
        require(buffer.map_colors(vertical?1:20,vertical?12:1,vertical?5:25,vertical?15:5,lut)==7);
        require(get_calls==0);unchanged();
    }
    for(int failure : {2,3})for(bool rect_form : {false,true}){
        if(rect_form)continue;
        start();buffer.surface_=&surface;buffer.surface_lock_count_=4;mode=failure;
        publish_pitch=true;published_pitch=32;
        rect={1,1,5,5};
        require((rect_form?buffer.map_colors(&rect,lut):buffer.map_colors(1,1,4,4,lut))==7);
        require(get_calls==1 && unlock_calls==0 && buffer.surface_lock_count_==(failure==3?5:4));
        require(buffer.locked_bits_==nullptr && buffer.stride_==uint32_t(failure==3?32:24));unchanged();
    }
    start();buffer.surface_=&surface;buffer.surface_lock_count_=4;
    published_pixels=reinterpret_cast<unsigned char *>(UINTPTR_MAX);
    // Acquired nonzero base plus clipped.left wraps numerically to zero.
    // Original returns7 without decrementing count or releasing the lock.
    require(buffer.map_colors(1,0,2,1,lut)==7);
    require(get_calls==1 && unlock_calls==0 && buffer.surface_lock_count_==5);
    require(buffer.locked_bits_==published_pixels);unchanged();
}
void rect_remaps(){
    start();RECT nonnull={0,0,5,5};
    require(buffer.map_colors(nullptr,lut)==16);
    require(buffer.map_colors(&nonnull,nullptr)==16);
    require(lock_calls==0 && get_calls==0);unchanged();
    for(bool surface_path : {false,true})for(bool nested : {false,true})
      for(bool changed_pitch : {false,true}){
        start();buffer.surface_=surface_path?&surface:nullptr;
        buffer.surface_lock_count_=nested?3:0;
        if(nested)buffer.locked_bits_=published_pixels;
        lock_pitch=changed_pitch?32:24;lock_bits=published_pixels;
        int pitch=(surface_path && !nested && changed_pitch)?32:24;
        RECT area={2,2,10,7};remap_oracle({2,2,9,6},24,pitch,false);
        require(buffer.map_colors(&area,lut)==0);unchanged();
        require(get_calls==0);
        require(lock_calls==(surface_path && !nested?1:0));
        require(unlock_calls==(surface_path && !nested?1:0));
        require(buffer.surface_lock_count_==(nested?3:0));
        if(surface_path && nested)require(buffer.locked_bits_==published_pixels);
        else if(!surface_path && nested)require(buffer.locked_bits_==buffer.dib_bits_);
        else require(buffer.locked_bits_==nullptr);
        require(buffer.stride_==uint32_t(pitch));
    }
    // Direct DIB mode overwrites a nested cached pointer with dib_bits_ and
    // leaves that pointer published when the reference count remains positive.
    start();buffer.surface_lock_count_=3;buffer.locked_bits_=pixels+600;
    RECT dib_nested={1,1,5,5};
    require(buffer.map_colors(&dib_nested,lut)==0);
    require(buffer.surface_lock_count_==3 && buffer.locked_bits_==buffer.dib_bits_);
    require(lock_calls==0 && get_calls==0 && unlock_calls==0);

    // No DIB storage publishes null, but must not acquire or decrement a
    // holder. Exercise both a fresh count and stale nested state.
    for(int holders : {0,3}){
        start();buffer.dib_bits_=nullptr;buffer.locked_bits_=pixels+600;
        buffer.surface_lock_count_=holders;RECT missing_dib={1,1,5,5};
        require(buffer.map_colors(&missing_dib,lut)==7);
        require(buffer.locked_bits_==nullptr && buffer.surface_lock_count_==holders);
        require(lock_calls==0 && get_calls==0 && unlock_calls==0);unchanged();
    }

    // A successful Lock returning null increments/publishes state, then the
    // native base-pointer check returns7 before offset calculation. Exercise
    // both zero and nonzero clipped origins; no low address may be touched.
    for(RECT area : {RECT{0,0,5,5},RECT{2,2,6,6}}){
        start();buffer.surface_=&surface;lock_pitch=32;lock_bits=nullptr;
        require(buffer.map_colors(&area,lut)==7);
        require(lock_calls==1 && get_calls==0 && unlock_calls==0);
        require(buffer.surface_lock_count_==1 && buffer.locked_bits_==nullptr);
        require(buffer.stride_==32);unchanged();
    }
    start();buffer.surface_=&surface;lock_status=19;lock_pitch=32;
    RECT failure={1,1,5,5};
    require(buffer.map_colors(&failure,lut)==7);
    require(lock_calls==1 && get_calls==0 && unlock_calls==0);
    require(buffer.surface_lock_count_==0 && buffer.locked_bits_==nullptr);
    require(buffer.stride_==24);unchanged();

    // Alias the translation table into the target bytes: each lookup must
    // observe earlier writes, rather than a copied table snapshot.
    start();buffer.stride_=16;buffer.rect1_={0,0,8,4};
    pixels[512]=1;pixels[513]=0;pixels[514]=0;std::memcpy(expected,pixels,sizeof(pixels));
    remap_oracle({0,0,8,3},16,16,true);RECT alias={0,0,9,4};
    require(buffer.map_colors(&alias,pixels+512)==0);unchanged();
    require(lock_calls==0 && buffer.surface_lock_count_==0);

    start();RECT empty={2,2,3,3};
    require(buffer.map_colors(&empty,lut)==0 && lock_calls==0);unchanged();
    start();buffer.rect1_={0,0,100,100};RECT beyond_x={20,0,26,3};
    require(buffer.map_colors(&beyond_x,lut)==7 && lock_calls==0);unchanged();
    start();buffer.rect1_={0,0,100,100};RECT beyond_y={0,12,3,18};
    require(buffer.map_colors(&beyond_y,lut)==7 && lock_calls==0);unchanged();

    // Null acquired storage and numeric address wrap both retain acquired
    // state without cleanup, matching the original early-return paths.
    start();buffer.surface_=&surface;lock_bits=reinterpret_cast<void *>(UINTPTR_MAX);
    RECT wrapped={1,0,3,2};
    require(buffer.map_colors(&wrapped,lut)==7);
    require(lock_calls==1 && unlock_calls==0 && buffer.surface_lock_count_==1);
    require(buffer.locked_bits_==lock_bits);unchanged();
}
void fills(){
    for(int pitch : {8,9,12,13})for(int left : {0,1,3})for(int width=1;width<=8-left;++width)
      for(int height : {1,2,4})for(bool nested : {false,true}){
        start();buffer.stride_=pitch;buffer.dib_.bmiHeader={8,-4};buffer.rect1_={0,0,8,4};
        buffer.surface_lock_count_=nested?3:0;
        RECT area={left,0,left+width,height};
        for(int y=0;y<height;++y)for(int x=left;x<left+width;++x)expected[512+y*pitch+x]=0xAB;
        require(buffer.fill(&area,0x1AB)==0);unchanged();
        require(buffer.surface_lock_count_==(nested?3:0));
        require(buffer.locked_bits_==(nested?pixels+512:nullptr));
        require(get_calls==0 && unlock_calls==0 && BufferFillRowGap==pitch-width);
    }
    // Real allocation ends at the last pixel of the final row. Computing a
    // later row pointer is not justified merely because it is not accessed.
    start();alignas(16) unsigned char exact[16];std::memset(exact,0,sizeof(exact));
    buffer.dib_bits_=exact;buffer.stride_=8;buffer.dib_.bmiHeader={8,-2};buffer.rect1_={0,0,8,2};
    RECT area={1,0,8,2};require(buffer.fill(&area,77)==0);
    for(int i=0;i<16;++i)require(exact[i]==((i%8)==0?0:77));
    start();require(buffer.fill(nullptr,1)==3);unchanged();
    start();buffer.dib_bits_=nullptr;require(buffer.fill(nullptr,1)==7);unchanged();
    start();area={50,50,60,60};require(buffer.fill(&area,1)==0 && get_calls==0);unchanged();
    start();buffer.surface_=&surface;area={-2,-3,7,5};
    require(buffer.fill(&area,349)==0 && blt_calls==1 && blt_color==349);
    require(blt_rect.left==0 && blt_rect.top==0 && blt_rect.right==7 && blt_rect.bottom==5);
    require(get_calls==0 && buffer.surface_lock_count_==0);unchanged();
    // Null RECT + DirectDraw leaves original stack RECT uninitialized;
    // intentionally not executed or presented as supported by this control.
}
int main(int argc,char **argv){
    if(argc==1 || std::string(argv[1])=="remap")remaps();
    if(argc==1 || std::string(argv[1])=="rect")rect_remaps();
    if(argc==1 || std::string(argv[1])=="fill")fills();
}
'''


def run_host(tmp_path, fields, bodies, only=None):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler required")
    source = tmp_path / "buffer.cpp"
    source.write_text(HARNESS.replace("ACTUAL_FIELDS", fields).replace("ACTUAL_BODIES", "\n".join(bodies)))
    binary = tmp_path / "buffer"
    compiled = subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all",str(source),"-o",str(binary)],
                              capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)]+([only] if only else []),capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_buffer_remapping(tmp_path):
    result=run_host(tmp_path,*actual_source(),only="remap")
    assert result.returncode==0,result.stderr


def test_actual_direct_rectangle_remapping(tmp_path):
    result=run_host(tmp_path,*actual_source(),only="rect")
    assert result.returncode==0,result.stderr


def test_actual_source_preserves_direct_lock_publication_order():
    body = actual_source()[1][1]
    ordered = ["const int acquired_lock_count = surface_lock_count_ + 1;",
               "stride_ = static_cast<uint32_t>(description.lPitch);",
               "surface_lock_count_ = acquired_lock_count;",
               "locked_bits_ = description.lpSurface;"]
    positions = [body.index(fragment) for fragment in ordered]
    assert positions == sorted(positions)
    assert body.index("if (locked_bits_ == nullptr) {") < body.index("uintptr_t address =")


def test_actual_buffer_rectangle_fill(tmp_path):
    result=run_host(tmp_path,*actual_source(),only="fill")
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("which,before,after,only", [
    (0,"int rows = clipped.bottom - clipped.top;","int rows = clipped.bottom - clipped.top + 1;","remap"),
    (0,"const int width = clipped.right - clipped.left;","const int width = clipped.right - clipped.left + 1;","remap"),
    (0,"address += static_cast<uintptr_t>(row_gap);",
       "address += static_cast<uintptr_t>(static_cast<int>(stride_) - width);","remap"),
    (0,"static_cast<uintptr_t>(static_cast<int>(stride_))",
       "static_cast<uintptr_t>(row_gap + width)","remap"),
    (0,"clipped.left >= dib_.bmiHeader.biWidth","clipped.left > dib_.bmiHeader.biWidth","remap"),
    (0,"clipped.top >= -dib_.bmiHeader.biHeight","clipped.top > -dib_.bmiHeader.biHeight","remap"),
    (0,"if (acquire_pixels() == 0) {\n        return 7;",
       "if (acquire_pixels() == 0) {\n        free_data(1); return 7;","remap"),
    (0,"free_data(1);","free_data(2);","remap"),
    (0,"free_data(1);","/* omitted release */","remap"),
    (0,"if (address == 0)","if (false)","remap"),
    (0,"return 7;","return 0;","remap"),
    (0,"*pixel = table[*pixel];","*pixel = table[static_cast<unsigned char>(*pixel + 1)];","remap"),
    (0,"RECT clipped;","unsigned char copied[256]; memcpy(copied, table, 256); table=copied; RECT clipped;","remap"),
    (1,"--clipped.right;","/* omitted right decrement */","rect"),
    (1,"--clipped.bottom;","/* omitted bottom decrement */","rect"),
    (1,"return 16;","return 0;","rect"),
    (1,"if (locked_bits_ == nullptr) {","if (false) {","rect"),
    (1,"if (address == 0) {","if (false) {","rect"),
    (1,"surface_->Unlock(locked_bits_);","/* omitted Unlock */","rect"),
    (1,"const int row_gap = static_cast<int>(stride_) - width;","const int row_gap = 0;","rect"),
    (1,"*pixel = table[*pixel];","*pixel = table[static_cast<unsigned char>(*pixel + 1)];","rect"),
    (1,"surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr)",
       "surface_->Lock(nullptr, &description, 0, nullptr)","rect"),
    (3,"destination = reinterpret_cast<uintptr_t>(tail) + BufferFillRowGap;",
       "destination += BufferFillRowGap;","fill"),
    (3,"*tail++ = value;",
       "*reinterpret_cast<uint32_t *>(tail) = value; ++tail;","fill"),
    (3,"uint32_t count = width","uint32_t count = width & ~3u","fill"),
    (3,"uint32_t count = width","uint32_t count = width & 3u","fill"),
    (3,"BufferFillRowGap = stride_ - width;","BufferFillRowGap = stride_;","fill"),
    (3,"*tail++ = value;","*tail++ = 0;","fill"),
    (3,"--surface_lock_count_ <= 0","surface_lock_count_ <= 0","fill"),
])
def test_buffer_damage_is_observed(tmp_path,which,before,after,only):
    fields,bodies=actual_source()
    assert before in bodies[which]
    bodies[which]=bodies[which].replace(before,after)
    result=run_host(tmp_path,fields,bodies,only=only)
    assert result.returncode!=0
    assert result.returncode==95 or "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr,result.stderr
