"""Actual Sprite recolouring/copying with explicit allocator/CRT boundaries.

Original 5EAD9F..5EAF34 supplies the oracle: recolouring skips totals0..3;
copy is source.copy(destination), captures stride*height after cleanup, reads
pixels after allocation and metadata after memcpy, and never charges the new
allocation. The complete actual close body is included. Its borrowed-pointer
retention and counter subtraction are intentionally not repaired by fixtures.

Host fields and method declarations come from sprite.h; private access is a
projection, not native layout proof. Constructors/destructors and the real
allocator/CRT are outside this test. Service doubles expose call boundaries,
including deliberate callback perturbations, not a claim that CRT callbacks
exist in the game. Finite valid pixel buffers and nonoverlapping copy regions
are covered; negative recolour totals, null destinations, and overlapping
nonempty memcpy are outside the original defined host-test domain. One wrapped
allocation-size case fails allocation without touching its huge pixel range.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated actual source')


def actual():
    header = (ROOT / 'src/sprite.h').read_text()
    source = (ROOT / 'src/sprite.cpp').read_text()
    fields = header[header.index('  char *ppszFileName_;'):]
    fields = fields[:fields.index('\n};')]
    declarations = []
    for pattern in (r'void close\(\);', r'int copy\(Sprite \*destination\);',
                    r'int change_color\(int old_color, int new_color\);'):
        declaration, = re.findall(pattern, header)
        declarations.append(declaration)
    owner, = re.findall(r'^int SpriteMemoryUsed = 0;$', source, re.M)
    public, = re.findall(r'^extern int SpriteMemoryUsed;$', header, re.M)
    allocator, = re.findall(r'^LPVOID __cdecl mem_get\(int size\);$',
                           (ROOT / 'src/general.h').read_text(), re.M)
    methods = {name: braced(source, signature) for name, signature in (
        ('close', 'void Sprite::close('),
        ('change', 'int Sprite::change_color('),
        ('copy', 'int Sprite::copy('))}
    return '\n'.join((public, owner, allocator,
                       'struct Sprite {', fields, *declarations, '};')), methods


HARNESS = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vector>
#include <type_traits>
#define __cdecl
typedef void *LPVOID;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d\n",line,0);std::exit(103);}}
#define require(v) check((v),__LINE__)
DECLARATIONS
static_assert(std::is_same<decltype(Sprite::pcBits_),unsigned char*>::value,"pixel owner");
static_assert(std::is_same<decltype(Sprite::cTransparentIndex_),unsigned char>::value,"palette key");
static_assert(std::is_same<decltype(Sprite::iSpriteWidth2_),int>::value,"signed stride");
unsigned char source_pixels[256],alternate_pixels[256],old_pixels[256],new_pixels[512];
char old_name[16],replacement_name[16],source_name[16];
Sprite source,destination;
std::vector<char> calls;
std::vector<void*> frees;
size_t requested,copy_size;
void *copy_destination;
const void *copy_source;
bool allocation_failure;
std::function<void(char)> callback;
void asset_free(void *pointer){
  require(pointer==old_pixels || pointer==old_name || pointer==replacement_name || pointer==new_pixels);
  calls.push_back('F');frees.push_back(pointer);if(callback)callback('F');
}
LPVOID __cdecl mem_get(int size){
  calls.push_back('A');requested=static_cast<uint32_t>(size);if(callback)callback('A');
  return allocation_failure ? nullptr : new_pixels;
}
void *asset_memcpy(void *out,const void *in,size_t size){
  calls.push_back('C');copy_destination=out;copy_source=in;copy_size=size;
  require(size<=sizeof(source_pixels));
  require(out==new_pixels);
  require(in==source_pixels || in==alternate_pixels || (size==0 && in==new_pixels));
  if(size)std::memcpy(out,in,size);
  if(callback)callback('C');return out;
}
#define free asset_free
#define memcpy asset_memcpy
METHODS
#undef memcpy
#undef free

std::vector<intptr_t> state(const Sprite&s){
  return {reinterpret_cast<intptr_t>(s.ppszFileName_),reinterpret_cast<intptr_t>(s.pcBits_),
    s.cTransparentIndex_,s.pad1_,s.pad2_,s.pad3_,s.iSpriteWidth2_,s.iSpriteWidth_,
    s.iSpriteHeight_,s.iWidth_,s.iHeight_,s.iLeftOffset_,s.iTopOffset_,s.fObj1Exists_};
}
void start(){
  source=Sprite{};destination=Sprite{};
  source.ppszFileName_=source_name;source.pcBits_=source_pixels;
  source.cTransparentIndex_=241;source.pad1_=11;source.pad2_=12;source.pad3_=13;
  source.iSpriteWidth2_=5;source.iSpriteWidth_=3;source.iSpriteHeight_=3;
  source.iWidth_=91;source.iHeight_=92;source.iLeftOffset_=-4;source.iTopOffset_=8;source.fObj1Exists_=1;
  destination.ppszFileName_=old_name;destination.pcBits_=old_pixels;
  destination.cTransparentIndex_=203;destination.pad1_=21;destination.pad2_=22;destination.pad3_=23;
  destination.iSpriteWidth2_=11;destination.iSpriteWidth_=7;destination.iSpriteHeight_=2;
  destination.iWidth_=81;destination.iHeight_=82;destination.iLeftOffset_=3;destination.iTopOffset_=-9;
  SpriteMemoryUsed=1000;calls.clear();frees.clear();callback=nullptr;
  requested=copy_size=999;copy_source=copy_destination=nullptr;allocation_failure=false;
  for(size_t i=0;i<sizeof(source_pixels);++i){source_pixels[i]=static_cast<unsigned char>(i*13+7);alternate_pixels[i]=static_cast<unsigned char>(i*19+9);}
  std::memset(old_pixels,0x5e,sizeof old_pixels);std::memset(new_pixels,0xcc,sizeof new_pixels);
}
void cleared(const Sprite&s){
  require(s.ppszFileName_==nullptr);
  require(s.iSpriteWidth2_==0&&s.iSpriteWidth_==0&&s.iSpriteHeight_==0);
  require(s.iWidth_==0&&s.iHeight_==0&&s.iLeftOffset_==0&&s.iTopOffset_==0&&s.fObj1Exists_==0);
}
void equal_metadata(const Sprite&s,const Sprite&d){
  require(d.iSpriteWidth2_==s.iSpriteWidth2_&&d.iSpriteWidth_==s.iSpriteWidth_&&d.iSpriteHeight_==s.iSpriteHeight_);
  require(d.iWidth_==s.iWidth_&&d.iHeight_==s.iHeight_);
  require(d.iLeftOffset_==s.iLeftOffset_&&d.iTopOffset_==s.iTopOffset_);
  require(d.cTransparentIndex_==s.cTransparentIndex_);
  require(d.ppszFileName_==nullptr&&d.fObj1Exists_==0);
  require(d.pad1_==21&&d.pad2_==22&&d.pad3_==23);
}
void recolour(){
  start();source.pcBits_=nullptr;auto old=state(source);
  require(source.change_color(7,19)==7);require(state(source)==old&&calls.empty()&&SpriteMemoryUsed==1000);
  // Offsets and displayed/source widths do not restrict the full pitch*height block.
  for(int total=0;total<=65;++total)for(int oldcolor : {0,7,255,263,-1})for(int newcolor : {0,31,287,-2}){
    start();source.iSpriteWidth2_=total;source.iSpriteHeight_=1;
    source.pcBits_=source_pixels+1;source.cTransparentIndex_=static_cast<unsigned char>(oldcolor);
    unsigned char expected[sizeof source_pixels];std::memcpy(expected,source_pixels,sizeof expected);
    for(int i=0;i<total;++i)source_pixels[1+i]=expected[1+i]=static_cast<unsigned char>(i%3==0?oldcolor:i);
    if(total>=4)for(int i=0;i<total;++i)if(expected[1+i]==static_cast<unsigned char>(oldcolor))expected[1+i]=static_cast<unsigned char>(newcolor);
    auto before=state(source);
    require(source.change_color(oldcolor,newcolor)==0);
    require(std::memcmp(source_pixels,expected,sizeof expected)==0);
    require(state(source)==before&&SpriteMemoryUsed==1000&&calls.empty());
  }
  start();source.iSpriteWidth2_=3;source.iSpriteHeight_=3;
  std::memset(source_pixels,7,sizeof source_pixels);require(source.change_color(7,8)==0);
  for(int i=0;i<256;++i)require(source_pixels[i]==(i<9?8:7));
}
void copying(){
  start();source.pcBits_=nullptr;auto dest_before=state(destination),src_before=state(source);
  require(source.copy(&destination)==7);require(calls.empty()&&SpriteMemoryUsed==1000);
  require(state(destination)==dest_before&&state(source)==src_before);
  for(int borrowed : {0,1,2,-1})for(bool fail : {false,true}){
    start();destination.fObj1Exists_=borrowed;allocation_failure=fail;auto before=state(source);
    callback=[&](char kind){if(kind=='A'){
      cleared(destination);require(destination.cTransparentIndex_==203);
      require(destination.pcBits_==(borrowed?old_pixels:nullptr));
      require(SpriteMemoryUsed==(borrowed?1000:986));
    }};
    require(source.copy(&destination)==(fail?4:0));callback=nullptr;
    require(calls==(borrowed?(fail?std::vector<char>{'F','A'}:std::vector<char>{'F','A','C'}):
                           (fail?std::vector<char>{'F','F','A'}:std::vector<char>{'F','F','A','C'})));
    require(frees==(borrowed?std::vector<void*>{old_name}:std::vector<void*>{old_pixels,old_name}));
    require(requested==15&&SpriteMemoryUsed==(borrowed?1000:986)&&state(source)==before);
    if(fail){cleared(destination);require(destination.pcBits_==nullptr&&destination.cTransparentIndex_==203);}
    else{
      equal_metadata(source,destination);require(destination.pcBits_==new_pixels);
      require(copy_size==15&&copy_source==source_pixels&&copy_destination==new_pixels);
      for(int i=0;i<512;++i)require(new_pixels[i]==(i<15?source_pixels[i]:0xcc));
      destination.close();require(SpriteMemoryUsed==(borrowed?991:977));
      cleared(destination);require(destination.pcBits_==nullptr&&destination.cTransparentIndex_==241);
    }
  }
  // Empty destination, zero-size and wrapped32-bit allocation requests.
  for(int stride : {0,-1}){
    start();destination.pcBits_=nullptr;destination.ppszFileName_=nullptr;
    source.iSpriteWidth2_=stride;source.iSpriteHeight_=3;allocation_failure=true;
    require(source.copy(&destination)==4&&calls==std::vector<char>({'A'}));
    require(requested==(stride==0?0u:0xfffffffdu));
    require(SpriteMemoryUsed==1000);
  }
  // Allocator boundary sees clean destination. Source dimensions/pointer may
  // change there, but the byte count was captured before that call.
  start();callback=[](char kind){if(kind=='A'){
    cleared(destination);source.pcBits_=alternate_pixels;
    source.iSpriteWidth2_=2;source.iSpriteHeight_=4;source.iSpriteWidth_=9;
    source.iWidth_=18;source.iHeight_=19;source.iLeftOffset_=20;source.iTopOffset_=-21;source.cTransparentIndex_=199;
  }};
  require(source.copy(&destination)==0);callback=nullptr;
  require(requested==15&&copy_size==15&&copy_source==alternate_pixels);
  require(std::memcmp(new_pixels,alternate_pixels,15)==0);equal_metadata(source,destination);
  // The memcpy boundary sees zero metadata and old key, then every copied
  // source field is fresh; changed destination pixels are not overwritten.
  start();callback=[](char kind){if(kind=='C'){
    cleared(destination);require(destination.cTransparentIndex_==203);
    source.iSpriteWidth2_=17;source.iSpriteWidth_=18;source.iSpriteHeight_=19;
    source.iWidth_=20;source.iHeight_=21;source.iLeftOffset_=-22;source.iTopOffset_=23;source.cTransparentIndex_=24;
    destination.pcBits_=old_pixels;
  }};
  require(source.copy(&destination)==0);callback=nullptr;equal_metadata(source,destination);
  require(destination.pcBits_==old_pixels);
  // Pixel free precedes the filename read and all dimension resets.
  start();callback=[](char kind){if(kind=='F'&&frees.size()==1){
    require(SpriteMemoryUsed==986&&destination.pcBits_==old_pixels);
    require(destination.iSpriteWidth_==7&&destination.iSpriteHeight_==2);
    destination.ppszFileName_=replacement_name;
    source.iSpriteWidth2_=4;source.iSpriteHeight_=2;
  }};
  require(source.copy(&destination)==0);callback=nullptr;
  require(frees==std::vector<void*>({old_pixels,replacement_name}));require(requested==8&&copy_size==8);
  // Self-copy closes itself first: it reaches a zero-byte allocation and
  // copies its own newly published block, not the former image.
  start();source.ppszFileName_=old_name;source.pcBits_=old_pixels;source.fObj1Exists_=0;
  require(source.copy(&source)==0);require(requested==0&&copy_size==0&&copy_source==new_pixels);
  require(calls==std::vector<char>({'F','F','A','C'}));cleared(source);
  require(source.pcBits_==new_pixels&&source.cTransparentIndex_==241);
  require(SpriteMemoryUsed==991);
}
void close_controls(){
  for(int borrowed : {0,1,-1}){
    start();destination.fObj1Exists_=borrowed;destination.close();
    cleared(destination);require(destination.cTransparentIndex_==203);
    require(destination.pcBits_==(borrowed?old_pixels:nullptr));
    require(SpriteMemoryUsed==(borrowed?1000:986));
    require(frees==(borrowed?std::vector<void*>{old_name}:std::vector<void*>{old_pixels,old_name}));
  }
  start();SpriteMemoryUsed=-2147483647;destination.close();
  require(static_cast<uint32_t>(SpriteMemoryUsed)==0x7ffffff3u);
}
int main(){recolour();copying();close_controls();}
'''


def run_host(tmp_path, declarations, methods):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler unavailable')
    cpp = tmp_path / 'asset.cpp'
    cpp.write_text(HARNESS.replace('DECLARATIONS', declarations).replace(
        'METHODS', '\n'.join(methods.values())))
    exe = tmp_path / 'asset'
    compiled = subprocess.run([compiler, '-std=c++11', '-O1', '-g',
                               '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                               str(cpp), '-o', str(exe)], capture_output=True, text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_sprite_asset_methods(tmp_path):
    result = run_host(tmp_path, *actual())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('method,before,after', [
    ('change','return 7;', 'return 0;'),
    ('change','if (size < 4)', 'if (size == 0)'),
    ('change','iSpriteWidth2_', 'iSpriteWidth_'),
    ('change','index < size', 'index + 1 < size'),
    ('change','static_cast<unsigned char>(old_color)', 'static_cast<unsigned char>(old_color + 1)'),
    ('change','static_cast<unsigned char>(new_color)', 'static_cast<unsigned char>(new_color + 1)'),
    ('change','pixels[index] == from', 'pixels[index] != from'),
    ('copy','return 7;', 'return 0;'),
    ('copy','destination->close();', '(void)0;'),
    ('copy','return 4;', 'return 7;'),
    ('copy','* static_cast<unsigned int>(iSpriteWidth2_)', '* static_cast<unsigned int>(iSpriteWidth_)'),
    ('copy','memcpy(pixels, pcBits_, size);', 'memcpy(pixels, pcBits_, size - (size != 0));'),
    ('copy','destination->fObj1Exists_ = 0;', 'destination->fObj1Exists_ = fObj1Exists_;'),
    ('copy','destination->iSpriteWidth2_ = iSpriteWidth2_;', 'destination->iSpriteWidth2_ = iSpriteWidth_;'),
    ('copy','destination->iWidth_ = iWidth_;', 'destination->iWidth_ = iSpriteWidth_;'),
    ('copy','destination->iHeight_ = iHeight_;', 'destination->iHeight_ = iSpriteHeight_;'),
    ('copy','destination->iLeftOffset_ = iLeftOffset_;', 'destination->iLeftOffset_ = 0;'),
    ('copy','destination->iTopOffset_ = iTopOffset_;', 'destination->iTopOffset_ = 0;'),
    ('copy','destination->cTransparentIndex_ = cTransparentIndex_;', 'destination->cTransparentIndex_ = 0;'),
    ('copy','memcpy(pixels, pcBits_, size);', 'memcpy(pixels, pcBits_, size); SpriteMemoryUsed += size;'),
    ('copy','destination->iSpriteWidth2_ = iSpriteWidth2_;', 'destination->iSpriteWidth2_ = iSpriteWidth2_; destination->ppszFileName_ = ppszFileName_;'),
    ('close','fObj1Exists_ == 0', 'true'),
    ('close','static_cast<uint32_t>(iSpriteWidth_)', 'static_cast<uint32_t>(iSpriteWidth2_)'),
])
def test_asset_damage_is_observed(tmp_path, method, before, after):
    declarations, methods = actual()
    assert before in methods[method]
    methods[method] = methods[method].replace(before, after)
    result = run_host(tmp_path, declarations, methods)
    assert result.returncode == 103 and 'assertion line' in result.stderr, result.stderr


@pytest.mark.parametrize('damage', ['cached_pixels', 'cached_metadata', 'early_key', 'changed_count', 'republish_pixels'])
def test_copy_read_timing_damage_is_observed(tmp_path, damage):
    declarations, methods = actual()
    copy = methods['copy']
    anchor = 'unsigned char *pixels = static_cast<unsigned char *>(mem_get(size));'
    assert anchor in copy
    if damage == 'cached_pixels':
        copy = copy.replace(anchor, 'unsigned char *cached = pcBits_;\n' + anchor)
        copy = copy.replace('memcpy(pixels, pcBits_, size);', 'memcpy(pixels, cached, size);')
    elif damage == 'cached_metadata':
        copy = copy.replace(anchor, 'const Sprite cached = *this;\n' + anchor)
        for field in ('iSpriteWidth_', 'iSpriteHeight_', 'iWidth_', 'iHeight_',
                      'iLeftOffset_', 'iTopOffset_', 'cTransparentIndex_', 'iSpriteWidth2_'):
            copy = copy.replace(f'= {field};', f'= cached.{field};')
    elif damage == 'early_key':
        copy = copy.replace(anchor, 'destination->cTransparentIndex_ = cTransparentIndex_;\n' + anchor)
    elif damage == 'changed_count':
        copy = copy.replace('memcpy(pixels, pcBits_, size);',
                            'memcpy(pixels, pcBits_, static_cast<unsigned int>(iSpriteWidth2_) * static_cast<unsigned int>(iSpriteHeight_));')
    else:
        copy = copy.replace('memcpy(pixels, pcBits_, size);',
                            'memcpy(pixels, pcBits_, size); destination->pcBits_ = pixels;')
    methods['copy'] = copy
    result = run_host(tmp_path, declarations, methods)
    assert result.returncode == 103 and 'assertion line' in result.stderr, result.stderr
