"""Actual TexHeap count domains and first-fit behavior under host UBSan.

The original increments the 32-bit count and then uses a signed JLE guard.
This fixture does not establish original class spelling, native ABI or runtime.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    source = (ROOT / 'src/sprite.cpp').read_text()
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(signature)


PREFIX = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
struct Heap {
    void *current_;
    size_t free_size_;
    char token;
    void *get(int size) { assert(size<=static_cast<int>(free_size_)); return &token; }
};
struct TexHeap {
    unsigned char *base_;
    uint32_t capacity_;
    Heap **blocks_;
    uint32_t count_;
    void *get_mem(int size);
    void *get_tex_mem(int size);
    void release_allocations();
    void shutdown();
};
int releases;
void release(void *) { ++releases; }
#define free release
'''
SUFFIX = r'''
int main() {
    unsigned char arena;
    Heap first={reinterpret_cast<void *>(0x10001),4,1};
    Heap second={reinterpret_cast<void *>(0x20000),20,2};
    Heap *blocks[]={&first,&second};
    TexHeap heap={&arena,0,blocks,1};
    assert(heap.get_mem(3)==&first.token);
    assert(heap.get_mem(10)==&second.token);
    assert(heap.get_tex_mem(3)==&second.token);
    assert(heap.get_mem(21)==0 && heap.get_tex_mem(21)==0);
    heap.count_=0;
    assert(heap.get_mem(3)==&first.token && heap.get_mem(10)==0);
    assert(heap.get_tex_mem(3)==0);
    // The pinned INC followed by signed JLE skips the table in these cases.
    const uint32_t skipped[]={0x7fffffffU,0x80000000U,0xfffffffeU,0xffffffffU};
    heap.blocks_=0;
    for(unsigned i=0;i<4;++i) {
        heap.count_=skipped[i];
        assert(heap.get_mem(1)==0 && heap.get_tex_mem(1)==0);
    }
    heap.base_=0; heap.count_=1;
    assert(heap.get_mem(1)==0 && heap.get_tex_mem(1)==0);
    for(unsigned i=0;i<4;++i) {
        heap.base_=&arena; heap.blocks_=blocks; heap.count_=skipped[i];
        releases=0; heap.release_allocations();
        assert(releases==2 && heap.base_==0 && heap.blocks_==0 && heap.count_==skipped[i]);
        heap.base_=&arena; heap.blocks_=blocks; heap.count_=skipped[i]; heap.capacity_=12;
        releases=0; heap.shutdown();
        assert(releases==2 && heap.base_==0 && heap.blocks_==0 && heap.count_==0 && heap.capacity_==0);
    }
}
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'signed_overflow', 'alignment'])
def test_original_texheap_count_and_first_fit(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = '\n'.join(body(signature) for signature in (
        'void *TexHeap::get_mem(', 'void *TexHeap::get_tex_mem(',
        'void TexHeap::release_allocations()', 'void TexHeap::shutdown()'))
    if damage == 'signed_overflow':
        before='static_cast<int>(count_ + 1u)'
        assert source.count(before)==4
        source=source.replace(before,'static_cast<int>(count_) + 1')
    elif damage == 'alignment':
        before='& 0xffff) == 0'
        assert source.count(before)==1
        source=source.replace(before,'& 0xffff) != 0')
    cpp=tmp_path/'count.cpp'; binary=tmp_path/'count'
    cpp.write_text(PREFIX + source + SUFFIX)
    built=subprocess.run([compiler,'-std=c++11','-O1','-fsanitize=undefined',
                          '-fno-sanitize-recover=all',str(cpp),'-o',str(binary)],
                         capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True)
    assert (result.returncode==0)==(damage is None),result.stderr


INIT_PREFIX = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
int SpriteMemoryUsed;
void *mem_get(size_t size) { return std::malloc(size); }
struct Sprite {
    char *ppszFileName_;
    unsigned char *pcBits_;
    unsigned char cTransparentIndex_;
    int iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_, iWidth_, iHeight_;
    int iLeftOffset_, iTopOffset_, fObj1Exists_;
    int init(char *filename,int width,int height);
};
'''
INIT_SUFFIX = r'''
int main() {
    const int dimensions[][3]={{4,3,42},{INT_MAX,2,17},{1,1,INT_MIN},{INT_MIN,-1,5}};
    for(unsigned i=0;i<4;++i) {
        Sprite sprite={};
        sprite.pcBits_=static_cast<unsigned char *>(std::malloc(1));
        assert(sprite.pcBits_);
        sprite.iSpriteWidth_=dimensions[i][0]; sprite.iSpriteHeight_=dimensions[i][1];
        sprite.cTransparentIndex_=0xb3; SpriteMemoryUsed=dimensions[i][2];
        uint32_t expected=static_cast<uint32_t>(SpriteMemoryUsed)
            - static_cast<uint32_t>(sprite.iSpriteWidth_)*static_cast<uint32_t>(sprite.iSpriteHeight_);
        char filename[]="image.pcx";
        assert(sprite.init(filename,7,9)==0);
        assert(static_cast<uint32_t>(SpriteMemoryUsed)==expected);
        assert(sprite.pcBits_==0 && std::strcmp(sprite.ppszFileName_,filename)==0);
        assert(sprite.iSpriteWidth2_==7 && sprite.iSpriteWidth_==7 && sprite.iWidth_==7);
        assert(sprite.iSpriteHeight_==9 && sprite.iHeight_==9 && sprite.fObj1Exists_==0);
        assert(sprite.cTransparentIndex_==0xb3);
        std::free(sprite.ppszFileName_);
    }
}
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'signed_accounting', 'ownership'])
def test_init_retains_wrapped_accounting(tmp_path, compiler_name, damage):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source=body('int Sprite::init(char *filename, int width, int height)')
    if damage=='signed_accounting':
        before='''SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(SpriteMemoryUsed)
            - static_cast<uint32_t>(iSpriteHeight_)
                  * static_cast<uint32_t>(iSpriteWidth_));'''
        assert source.count(before)==1
        source=source.replace(before,'SpriteMemoryUsed -= iSpriteHeight_ * iSpriteWidth_;')
    elif damage=='ownership':
        before='fObj1Exists_ == 0'
        assert source.count(before)==1
        source=source.replace(before,'fObj1Exists_ != 0')
    cpp=tmp_path/'init.cpp';binary=tmp_path/'init'
    cpp.write_text(INIT_PREFIX + source + INIT_SUFFIX)
    built=subprocess.run([compiler,'-std=c++11','-O1','-fsanitize=undefined',
                          '-fno-sanitize-recover=all',str(cpp),'-o',str(binary)],
                         capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True)
    assert (result.returncode==0)==(damage is None),result.stderr
