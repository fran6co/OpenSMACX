"""Actual TutWin callouts: original wrap, alias/store timing and virtual base.

Host controls do not establish target ABI, linked references or game runtime.
"""
from pathlib import Path
import subprocess
import tempfile
import pytest
from test_netdaemon_transfer_modal import extract
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/tutwin.cpp'
NAMES=('iface','base','soc','des')

def product():
 s=SOURCE.read_text()
 return '\n'.join(extract(s,f'void TutWin::{n}_rect(') for n in NAMES)+'\n'+'\n'.join(extract(s,f'int TutWin::do_{n}(') for n in NAMES)

HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <climits>
struct RECT { int32_t left,top,right,bottom; };
struct GraphicWin { int marker; };
struct MapWin:virtual GraphicWin { int member; };
MapWin *MapWinTable[1],*next_owner;
int route,wanted_x,wanted_y,screen_calls,callouts;
const char *label; int expected_flag;
struct FixedWindow {
 int id;
 void client_to_screen(int*x,int*y){
  assert(id==route && *x==wanted_x && *y==wanted_y);
  ++screen_calls; *x=70+id; *y=80+id; MapWinTable[0]=next_owner;
 }
} MainInterfaceGlobal={1},TutWinBaseWindow={2},TutWinSocWindow={3},TutWinDesWindow={4};
struct TutWin {
 void iface_rect(RECT*,int*,int*); void base_rect(RECT*,int*,int*);
 void soc_rect(RECT*,int*,int*); void des_rect(RECT*,int*,int*);
 int do_iface(RECT*,const char*,int); int do_base(RECT*,const char*,int);
 int do_soc(RECT*,const char*,int); int do_des(RECT*,const char*,int);
 int tut_win(GraphicWin*owner,const char*text,int x,int y,void*s,int flag,int a,int b){
  assert(owner==static_cast<GraphicWin*>(next_owner));
  assert(text==label && flag==expected_flag && !s && a==-1 && b==-1);
  assert(x==70+route && y==80+route); ++callouts; return -107;
 }
};
BODY
// Independent instruction-domain oracle uses 64-bit arithmetic, avoiding the
// recovered expression and implementation-defined unsigned-to-signed casts.
int oracle(int32_t first,int32_t last){
 uint64_t diff=(uint64_t(uint32_t(last))+0x100000000ULL-uint32_t(first))&0xffffffffULL;
 int64_t signed_diff=diff<0x80000000ULL?int64_t(diff):int64_t(diff)-0x100000000LL;
 uint64_t sum=(uint64_t(uint32_t(first))+uint64_t(signed_diff/2))&0xffffffffULL;
 return int(sum<0x80000000ULL?int64_t(sum):int64_t(sum)-0x100000000LL);
}
int main(int argc,char**argv){
 TutWin t; MapWin first={},second={};
 assert(static_cast<void*>(static_cast<GraphicWin*>(&second))!=static_cast<void*>(&second));
 typedef void(TutWin::*RectFn)(RECT*,int*,int*);
 typedef int(TutWin::*DoFn)(RECT*,const char*,int);
 RectFn rects[]={&TutWin::iface_rect,&TutWin::base_rect,&TutWin::soc_rect,&TutWin::des_rect};
 DoFn wrappers[]={&TutWin::do_iface,&TutWin::do_base,&TutWin::do_soc,&TutWin::do_des};
 if(argc==2){int which=std::atoi(argv[1]);int x,y;if(which<4)(t.*rects[which])(nullptr,&x,&y);else(t.*wrappers[which-4])(nullptr,"null",0);return 0;}
 int32_t values[]={INT32_MIN,INT32_MIN+1,-101,-2,-1,0,1,2,101,INT32_MAX-1,INT32_MAX};
 label="retained-label"; expected_flag=-617;
 for(int r=0;r<4;++r)for(unsigned a=0;a<sizeof(values)/sizeof(*values);++a)for(unsigned b=0;b<sizeof(values)/sizeof(*values);++b){
  route=r+1; RECT original={values[a],values[b],values[b],values[a]};
  wanted_x=oracle(original.left,original.right);wanted_y=oracle(original.top,original.bottom);
  int x=0,y=0; RECT rect=original; next_owner=&second;MapWinTable[0]=&first;
  int before=screen_calls;(t.*rects[r])(&rect,&x,&y); assert(screen_calls==before+1&&x==70+route&&y==80+route);
  for(int null_owner=0;null_owner<2;++null_owner){
   rect=original;next_owner=null_owner?nullptr:&second;MapWinTable[0]=&first;
   before=callouts;assert((t.*wrappers[r])(&rect,label,expected_flag)==-107&&callouts==before+1);
  }
  // x aliases a field used by the subsequent y calculation. Native performs
  // the x store before reading top/bottom; caching both centres is wrong.
  rect=original;wanted_y=oracle(wanted_x,rect.bottom);
  (t.*rects[r])(&rect,&rect.top,&y);
  assert(rect.top==70+route&&y==80+route);
 }
}
'''

def run(body,argument=None):
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'c.cpp').write_text(HARNESS.replace('BODY',body))
  result=subprocess.run(['g++','-std=c++11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all',str(p/'c.cpp'),'-o',str(p/'c')],capture_output=True,text=True)
  assert result.returncode==0,result.stderr
  return subprocess.run([str(p/'c')]+([] if argument is None else [str(argument)]),capture_output=True).returncode

def test_full_native_arithmetic_alias_and_live_owner_domain():
 assert run(product())==0

@pytest.mark.parametrize('which',range(8))
def test_original_null_rect_faults(which):
 assert run(product(),which)!=0

def test_stale_owner_damage():
 body=product();old='    MapWin *const primary = MapWinTable[0];';assert body.count(old)==4
 # Move the actual snapshot before coordinate conversion in just do_base.
 start=body.index('int TutWin::do_base(');end=body.index('int TutWin::do_soc(',start)
 part=body[start:end];part=part.replace(old,'',1).replace('    TutWinBaseWindow.client_to_screen(&x, &y);',old+'\n    TutWinBaseWindow.client_to_screen(&x, &y);',1)
 assert run(body[:start]+part+body[end:])!=0

def test_unadjusted_owner_damage():
 body=product();bad=body.replace('&static_cast<GraphicWin &>(*primary)','reinterpret_cast<GraphicWin *>(primary)');assert body!=bad
 assert run(bad)!=0

def test_cached_rect_alias_damage():
 body=product();start=body.index('void TutWin::iface_rect(');end=body.index('void TutWin::base_rect(',start)
 part=body[start:end];opening=part.index('{');part=part[:opening+1]+'\n RECT copied=*rect; rect=&copied;'+part[opening+1:]
 assert run(body[:start]+part+body[end:])!=0

def test_signed_overflow_damage():
 body=product();start=body.index('void TutWin::iface_rect(');end=body.index('void TutWin::base_rect(',start)
 bad='void TutWin::iface_rect(RECT*rect,int*x,int*y){*x=rect->left+(rect->right-rect->left)/2;*y=rect->top+(rect->bottom-rect->top)/2;MainInterfaceGlobal.client_to_screen(x,y);}\n'
 assert run(body[:start]+bad+body[end:])!=0

def test_pinned_arithmetic_and_receivers():
 from hashlib import sha256
 from match_recovery.source import read_file
 from match_recovery.source.asm import original_asm
 image=Path('/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe')
 assert sha256(image.read_bytes()).hexdigest()=='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
 receivers={0x4bc5a0:'0x7ae820',0x4bc5f0:'0x6a7628',0x4bc640:'0x8a6270',0x4bc690:'0x71f2b0',0x4ba870:'0x6a7628',0x4ba8f0:'0x7ae820',0x4ba970:'0x8a6270',0x4ba9f0:'0x71f2b0'}
 records=[record for record in read_file(SOURCE) if record.address in receivers]
 assert len(records)==8
 for rec in records:
  lines='\n'.join(map(str,original_asm(rec,image).lines))
  assert lines.count('cdq')==2 and lines.count('sar ')==2
  assert 'call 0x5ed240' in lines and 'mov ecx, '+receivers[rec.address] in lines
  if rec.address<0x4bc000:
   assert '[0x7d3c3c]' in lines and 'call 0x4bdfe0' in lines
   assert 'mov eax, dword ptr [edx + 4]' in lines
