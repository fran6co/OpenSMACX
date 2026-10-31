"""Actual upstream C/provider and transfer-call controls, not game acceptance."""
from pathlib import Path
from hashlib import sha256
import os
import subprocess
import tempfile
import tarfile
import pytest

ROOT=Path(os.environ.get('OPENSMACX_ROOT',Path(__file__).resolve().parents[2]))
VENDOR=ROOT/'src/vendor/zlib-1.0.2'
UPSTREAM_SHA='a57da8f714a2145ce83a93be4fa3a79bdb1472bf50bae5643864d861668ce4a9'
ARCHIVE_SHA='517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6'

C_HARNESS=r'''
#include <assert.h>
#include <string.h>
#include "zlib.h"
static int init_result,finish_result,end_result,successful_end,size_mutation;
static Bytef *expected_dest,*expected_source;
static uLongf *observed_size;
static uLong expected_capacity,expected_input;
char events[32]; int event_count;
void event(char c){assert(event_count<31);events[event_count++]=c;events[event_count]=0;}
static void configure(Bytef*d,uLongf*n,const Bytef*s,uLong capacity,uLong input,int init,int finish,int end,int mutate){
 expected_dest=d;expected_source=(Bytef*)s;observed_size=n;expected_capacity=capacity;expected_input=input;
 init_result=init;finish_result=finish;end_result=end;size_mutation=mutate;successful_end=0;event_count=0;events[0]=0;
}
int deflateInit_(z_stream *z,int level,const char*version,int size){
 event('A');assert(z->next_in==expected_source&&z->avail_in==expected_input);
 assert(z->next_out==expected_dest&&z->avail_out==expected_capacity);
 assert(z->zalloc==0&&z->zfree==0&&z->opaque==0);
 assert(level==Z_DEFAULT_COMPRESSION&&level==-1&&strcmp(version,"1.0.2")==0&&size==sizeof(z_stream));
 z->total_out=991;return init_result;
}
int deflate(z_stream *z,int flush){event('D');assert(flush==Z_FINISH);z->total_out=37;successful_end=finish_result==Z_STREAM_END;return finish_result;}
int deflateEnd(z_stream *z){
 event('E');assert(z->total_out==37);
 if(observed_size){assert(*observed_size==(successful_end?37:expected_capacity));if(size_mutation)*observed_size=91;}
 return end_result;
}
int run_controls(void){
 Bytef dest[2048],source[2048];uLongf size;int status;
 size=300;configure(dest,&size,source,300,77,Z_MEM_ERROR,1,0,0);status=compress(dest,&size,source,77);
 assert(status==Z_MEM_ERROR&&size==300&&strcmp(events,"A")==0);
 size=300;configure(dest,&size,source,300,77,0,0,-77,0);status=compress(dest,&size,source,77);
 assert(status==Z_BUF_ERROR&&size==300&&strcmp(events,"ADE")==0);
 size=300;configure(dest,&size,source,300,77,0,Z_DATA_ERROR,-77,0);status=compress(dest,&size,source,77);
 assert(status==Z_DATA_ERROR&&size==300&&strcmp(events,"ADE")==0);
 size=300;configure(dest,&size,source,300,77,0,2,-77,0);status=compress(dest,&size,source,77);
 assert(status==2&&size==300&&strcmp(events,"ADE")==0);
 size=300;configure(dest,&size,source,300,77,0,Z_STREAM_END,-77,0);status=compress(dest,&size,source,77);
 assert(status==-77&&size==37&&strcmp(events,"ADE")==0);
 size=300;configure(dest,&size,source,300,77,0,Z_STREAM_END,0,1);status=compress(dest,&size,source,77);
 assert(status==0&&size==91&&strcmp(events,"ADE")==0);
 return 0;
}
void configure_transfer(Bytef*d,Bytef*s,int fail){configure(d,0,s,1009,777,fail?Z_MEM_ERROR:0,Z_STREAM_END,0,0);}
'''
CPP_HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include "zlib.h"
extern "C" { int run_controls(void);void configure_transfer(Bytef*,Bytef*,int);void event(char);extern char events[]; }
typedef uint32_t DWORD;
static Bytef output[2048],input[2048];static int file_size_calls,alloc_calls;
struct Filemap {int get_handle(){return 7;}void*get_view(){event('V');return input;}};
struct TransferHeader {unsigned long compressed_size;};
DWORD GetFileSize(int handle,int*unused){assert(handle==7&&!unused);event('G');return ++file_size_calls==1?1000:777;}
void*mem_get(int size){assert(size==1009);event('M');++alloc_calls;return output;}
static void transfer(Filemap&filemap,TransferHeader*send_buffer){
ACTUAL_TRANSFER
}
int main(){
 int(*api)(Bytef*,uLongf*,const Bytef*,uLong)=&compress;assert(api);
 assert(run_controls()==0);
 for(int fail=0;fail<2;++fail){
  Filemap filemap;TransferHeader header={};file_size_calls=alloc_calls=0;configure_transfer(output,input,fail);
  transfer(filemap,&header);
  assert(file_size_calls==2&&alloc_calls==1);
  assert(header.compressed_size==(fail?1009UL:37UL));
  assert(std::strcmp(events,fail?"GMGVA":"GMGVADE")==0);
 }
}
'''

def caller():
 source=(ROOT/'src/netdaemon.cpp').read_text()
 start=source.index('                const DWORD original_size = GetFileSize(filemap.get_handle(), 0);')
 end=source.index('                send_buffer->compressed_size = compressed_capacity;',start)
 end+=len('                send_buffer->compressed_size = compressed_capacity;')
 return source[start:end]

def execute(source):
 with tempfile.TemporaryDirectory() as directory:
  p=Path(directory);(p/'compress.c').write_text(source);(p/'services.c').write_text(C_HARNESS);(p/'caller.cpp').write_text(CPP_HARNESS.replace('ACTUAL_TRANSFER',caller()))
  for name in ('compress','services'):
   r=subprocess.run(['gcc','-std=gnu89','-O2','-I'+str(VENDOR),'-c',str(p/(name+'.c')),'-o',str(p/(name+'.o'))],capture_output=True,text=True)
   assert r.returncode==0,r.stderr
  r=subprocess.run(['g++','-std=c++11','-O2','-I'+str(VENDOR),str(p/'caller.cpp'),str(p/'compress.o'),str(p/'services.o'),'-o',str(p/'control')],capture_output=True,text=True)
  assert r.returncode==0,r.stderr
  return subprocess.run([str(p/'control')],capture_output=True).returncode

def test_actual_unmodified_archive_source_and_C_provider():
 actual=(VENDOR/'compress.c').read_bytes();assert sha256(actual).hexdigest()==UPSTREAM_SHA
 image=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
 archive=image.parents[1]/'dependencies/zlib-1.0.2.tar.gz';assert sha256(archive.read_bytes()).hexdigest()==ARCHIVE_SHA
 with tarfile.open(archive)as tf:assert tf.extractfile('zlib-1.0.2/compress.c').read()==actual
 assert execute(actual.decode())==0

@pytest.mark.parametrize('damage',['opaque','level','cleanup','size_order'])
def test_actual_C_damage_controls(damage):
 source=(VENDOR/'compress.c').read_text()
 if damage=='opaque':old='stream.opaque = (voidpf)0;';new='stream.opaque = (voidpf)dest;'
 elif damage=='level':old='deflateInit(&stream, Z_DEFAULT_COMPRESSION)';new='deflateInit(&stream, Z_BEST_SPEED)'
 elif damage=='cleanup':old='        deflateEnd(&stream);';new='        /* deliberate omitted error cleanup */'
 else:
  old='    *destLen = stream.total_out;\n\n    err = deflateEnd(&stream);';new='    err = deflateEnd(&stream);\n    *destLen = stream.total_out;'
 assert old in source
 assert execute(source.replace(old,new,1))!=0

def test_header_x86_stream_abi_and_original_identity():
 # Host syntax-only32-bit control uses the actual unchanged zlib declarations.
 probe='#include <stddef.h>\n#include "zlib.h"\n'
 checks={'sizeof(z_stream)':56,'sizeof(uLong)':4,'sizeof(uInt)':4,'offsetof(z_stream,next_in)':0,'offsetof(z_stream,next_out)':12,'offsetof(z_stream,total_out)':20,'offsetof(z_stream,zalloc)':32,'offsetof(z_stream,zfree)':36,'offsetof(z_stream,opaque)':40,'offsetof(z_stream,reserved)':52}
 probe+='\n'.join('typedef char check%d[(%s)==%d?1:-1];'%(i,expression,value)for i,(expression,value)in enumerate(checks.items()))
 r=subprocess.run(['gcc','-std=gnu89','-m32','-fsyntax-only','-x','c','-I'+str(VENDOR),'-'],input=probe,text=True,capture_output=True);assert r.returncode==0,r.stderr
 from match_recovery.source import read_file
 from match_recovery.component_source import parse_component
 from match_recovery.source.asm import original_asm
 recs=read_file(ROOT/'src/vendor/zlib.recovery.h');rec=next(r for r in recs if r.address==0x634280)
 assert rec.symbol=='_compress'and rec.image_spans==((0x634280,0x634326),)
 assert not any(r.address==0x634280 for r in read_file(ROOT/'src/netdaemon.cpp'))
 m=parse_component((ROOT/'src/vendor/zlib.recovery.h').read_text());binding=next(b for b in m.bindings if b.origin.address==0x634280)
 assert m.language=='c'and binding.symbol=='_compress'and binding.unit=='compress.c'
 image=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
 assert sha256(image.read_bytes()).hexdigest()=='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
 listing='\n'.join(map(str,original_asm(rec,image).lines))
 assert 'push 0x38'in listing and 'push -1'in listing and all(('call '+target)in listing for target in ('0x63cf00','0x63d1a0','0x63d4d0'))
