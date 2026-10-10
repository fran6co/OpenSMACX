"""Live coordinate acquisition with SDK doubles; no native ABI/allocation proof."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def control(damage):
    assert "unsigned char *get_data(int x, int y);" in (ROOT / "src/buffer.h").read_text()
    body = extract((ROOT / "src/buffer.cpp").read_text(), "unsigned char *Buffer::get_data(int x, int y)")
    assert "reinterpret_cast" not in body
    if damage:
        assert body.count(damage[0]) == 1
        body = body.replace(*damage, 1)
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
static const int DDLOCK_WAIT=1;
struct DDSURFACEDESC { unsigned dwSize; int lPitch; void *lpSurface; };
struct Surface {
    int calls=0,result=0,pitch=20;
    void *pixels=nullptr;
    int Lock(void *rect,DDSURFACEDESC *out,int flags,void *event){
        assert(!rect && !event && flags==DDLOCK_WAIT && out->dwSize==sizeof(*out));
        ++calls;out->lPitch=pitch;out->lpSurface=pixels;return result;
    }
};
struct Buffer {
    struct { struct {int biWidth=6,biHeight=-4;} bmiHeader; } dib_;
    uint32_t stride_=16;
    int surface_lock_count_=4;
    Surface *surface_=nullptr;
    void *dib_bits_=nullptr,*locked_bits_=nullptr;
    unsigned char *get_data(int,int);
};
BODY
int main(){
    unsigned char storage[256]={};
    Buffer dib;dib.dib_bits_=storage+128;dib.locked_bits_=storage;
    assert(dib.get_data(2,3)==storage+178 && dib.locked_bits_==storage+128 && dib.surface_lock_count_==5);
    assert(dib.get_data(-1,-1)==storage+111 && dib.surface_lock_count_==6);
    dib.stride_=-16;assert(dib.get_data(2,3)==storage+82 && dib.surface_lock_count_==7);
    for(int i=0;i!=2;++i){
        Buffer guarded;guarded.dib_bits_=storage+128;guarded.locked_bits_=storage;
        assert(guarded.get_data(i?0:6,i?4:0)==nullptr);
        assert(guarded.locked_bits_==storage && guarded.surface_lock_count_==4);
    }
    Buffer empty;empty.locked_bits_=storage;
    assert(empty.get_data(0,0)==nullptr && empty.locked_bits_==nullptr && empty.surface_lock_count_==4);

    Surface surface;surface.pixels=storage+128;
    Buffer locked;locked.surface_=&surface;
    assert(locked.get_data(2,3)==storage+190 && surface.calls==1);
    assert(locked.stride_==20 && locked.locked_bits_==surface.pixels && locked.surface_lock_count_==5);
    assert(locked.get_data(1,1)==storage+149 && surface.calls==1 && locked.surface_lock_count_==6);
    Buffer failed;failed.surface_=&surface;surface.result=7;
    assert(failed.get_data(1,1)==nullptr && surface.calls==2);
    assert(failed.stride_==16 && failed.surface_lock_count_==4 && failed.locked_bits_==nullptr);
    Buffer null_locked;null_locked.surface_=&surface;surface.result=0;surface.pixels=nullptr;
    assert(null_locked.get_data(1,1)==nullptr && surface.calls==3);
    assert(null_locked.stride_==20 && null_locked.surface_lock_count_==5 && null_locked.locked_bits_==nullptr);
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("if (x < dib_.bmiHeader.biWidth", "if (x >= 0 && y >= 0 && x < dib_.bmiHeader.biWidth"),
    ("stride_ = description.lPitch;", "stride_ = 16;"),
    ("static_cast<unsigned char *>(locked_bits_) + row_offset + x", "static_cast<unsigned char *>(locked_bits_) + y + x")))
def test_actual_coordinate_acquisition(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "pixel_address.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "pixel_address"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None)


def texture_control(damage):
    body = extract((ROOT / "src/texture.cpp").read_text(),
                   "int Texture::extract(Buffer *buffer, int x, int y, int width, int height,\n                     TexHeap *heap)")
    assert "const unsigned char *src = srcData;" in body
    assert "reinterpret_cast<void *>(src)" not in body
    if damage:
        assert body.count(damage[0]) == 1
        body = body.replace(*damage, 1)
    return r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
static bool fail_alloc;
static void *mem_get(int bytes){return fail_alloc?nullptr:malloc(bytes);}
struct Buffer {
    struct {struct {int biWidth=8,biHeight=-8;} bmiHeader;} dib_;
    int stride_=16,acquires=0,releases=0;
    unsigned char *pixels=nullptr;
    unsigned char *get_data(int x,int y){++acquires;return pixels?pixels+stride_*y+x:nullptr;}
    void free_data(int n){assert(n==1);++releases;}
};
struct TexHeap {
    unsigned char pixels[512];int calls=0;
    void *get_tex_mem(int bytes){assert(bytes==512);++calls;memset(pixels,0xCC,sizeof(pixels));return pixels;}
};
struct Texture {
    struct Point {int x,y;} corners_[4],cornersAndMids_[8];
    void *pixels_=nullptr;
    int iWidth_=0,iHeight_=0,borrowed_=0;
    int extract(Buffer *,int,int,int,int,TexHeap *);
};
BODY
int main(){
    unsigned char source[128];for(int i=0;i!=128;++i)source[i]=static_cast<unsigned char>(i);
    Texture texture;Buffer buffer;buffer.pixels=source;
    assert(texture.extract(nullptr,0,0,3,2,nullptr)==16);
    assert(texture.extract(&buffer,7,0,3,2,nullptr)==3 && buffer.acquires==0);
    assert(texture.extract(&buffer,0,7,3,2,nullptr)==3 && buffer.acquires==0);
    assert(texture.extract(&buffer,1,2,3,2,nullptr)==0);
    assert(texture.iWidth_==3 && texture.iHeight_==2 && texture.borrowed_==0);
    assert(buffer.acquires==1 && buffer.releases==1);
    unsigned char *out=static_cast<unsigned char *>(texture.pixels_);
    for(int row=0;row!=2;++row)for(int col=0;col!=3;++col)assert(out[row*3+col]==source[(row+2)*16+1+col]);
    assert(texture.corners_[2].x==2 && texture.corners_[2].y==1);
    assert(texture.cornersAndMids_[1].x==0 && texture.cornersAndMids_[3].y==0);
    TexHeap heap;
    assert(texture.extract(&buffer,1,2,3,2,&heap)==0 && texture.borrowed_==1 && heap.calls==1);
    assert(texture.pixels_==heap.pixels && buffer.acquires==2 && buffer.releases==2);
    for(int row=0;row!=2;++row){
        for(int col=0;col!=3;++col)assert(heap.pixels[row*256+col]==source[(row+2)*16+1+col]);
        assert(heap.pixels[row*256+3]==0xCC);
    }
    fail_alloc=true;
    assert(texture.extract(&buffer,1,2,3,2,nullptr)==4);
    assert(texture.iWidth_==0 && texture.iHeight_==0 && texture.borrowed_==0 && !texture.pixels_);
    assert(buffer.acquires==3 && buffer.releases==3);
    fail_alloc=false;buffer.pixels=nullptr;
    assert(texture.extract(&buffer,1,2,3,2,nullptr)==7);
    assert(buffer.acquires==4 && buffer.releases==3 && texture.iWidth_==3 && texture.iHeight_==2);
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("src += stride;", "src += iWidth_;"),
    ("dest += 0x100;", "dest += iWidth_;")))
def test_actual_texture_address_consumer(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "texture_address.cpp"
    source.write_text(texture_control(damage))
    binary = tmp_path / "texture_address"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None)
