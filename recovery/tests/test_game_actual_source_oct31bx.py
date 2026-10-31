"""Actual Game opening lifetime/read order and climate callback controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

ROOT=Path(__file__).resolve().parents[2]


def definition(signature):
    text=(ROOT/'src/game.cpp').read_text();start=text.index(signature)
    opening=text.index('{',start);depth=0
    for end in range(opening,len(text)):
        depth+=(text[end]=='{')-(text[end]=='}')
        if depth==0:return text[start:end+1]
    raise AssertionError(signature)


HARNESS=r'''
#include <cassert>
#include <string>
#include <vector>
#define __cdecl
static std::string scenario,events,popup;
static bool tracking;
struct GraphicWin;
static GraphicWin *current,*first,*second;
static int copied,updated,released,selection,probes,drawn_frame;
struct RECT {long left,top,right,bottom;};
struct Buffer {
 struct {struct {int biWidth,biHeight;} bmiHeader;}dib_;
 Buffer(){dib_.bmiHeader.biWidth=2;dib_.bmiHeader.biHeight=-3;if(tracking)events+='C';}
 ~Buffer(){if(tracking)events+='D';}
 int load_pcx(char*,int,int,int);
 void copy(Buffer*,int,int,int,int,int,int,int,int);
};
struct Prefix{int unused[7];};
struct GraphicWin:Prefix,Buffer {
 int identity;
 void release_modal();
 void soft_update(RECT*){}
 virtual void on_update(){events+='U';updated=identity;}
};

struct Sprite {
 int index;
 int transparent_index(){return 9;}
 void draw(Buffer *buffer,int transparent,int left,int top,int one,int other){
  assert(buffer && transparent==9 && left==10 && top==20 && one==1 && other==1);
  drawn_frame=index;
 }
};
struct Demo:GraphicWin {
 int selected_link_,timer_phase_;
 RECT splash_rects_[3];Sprite splash_sprites_[2];
 void on_left_down(int,int);void redraw();
};
static Demo *demo_active;
RECT DemoFourthHotspot={4,0,0,0};
void GraphicWin::release_modal(){++released;selection=demo_active->selected_link_;}
int in_box(int x,int y,RECT *area){
 assert(x==7 && y==9);++probes;
 if(probes==1)assert(demo_active->selected_link_==0);
 if(scenario=="demo-live" && probes==1)demo_active->splash_rects_[1].left=9;
 if(scenario=="demo-live")return area->left==9;
 return scenario=="demo-all" || scenario=="demo-"+std::to_string(area->left);
}

static GraphicWin *opening_window(){return current;}
int Buffer::load_pcx(char*,int zero,int ten,int ec){
 assert(zero==0 && ten==10 && ec==0xec);events+='L';
 if(scenario=="load-swap")current=second;
 if(scenario=="load-null")current=0;
 return scenario=="load-error" ? 1:0;
}
void Buffer::copy(Buffer *destination,int x,int y,int w,int h,int dx,int dy,int dw,int dh){
 events+='P';++copied;
 GraphicWin *expected=scenario=="load-swap" ? second:first;
 assert(destination==static_cast<Buffer*>(expected));
 assert(this!=destination && x==0 && y==0 && dx==0 && dy==0 && w==2 && h==3);
 assert(dw==expected->dib_.bmiHeader.biWidth && dh==-expected->dib_.bmiHeader.biHeight);
 if(scenario=="copy-swap")current=second;
}
struct State {int climate_sea_level_delta_,climate_rise_accum_,climate_rise_step_,climate_rise_limit_;};
struct {State fields_;}GameDataStateStorage;
int MapSeaLevel;
const char *ScriptPopupFile="script";
void world_climate(){events+='W';GameDataStateStorage.fields_.climate_sea_level_delta_=99;}
void draw_map(int x){assert(x==1);events+='M';}
void popp(const char*,const char *key,int zero,const char*,int z){assert(zero==0 && z==0);popup=key;events+='P';}
// ACTUAL
int main(int argc,char **argv){
 assert(argc==2);scenario=argv[1];
 if(scenario.find("demo")==0){
  Demo w;demo_active=&w;w.selected_link_=77;
  for(int i=0;i<3;++i)w.splash_rects_[i]={i+1,0,0,0};
  if(scenario=="demo-draw0" || scenario=="demo-draw1"){
   w.timer_phase_=scenario=="demo-draw0" ? 0:1;w.splash_rects_[0]={10,20,30,40};
   w.splash_sprites_[0].index=0;w.splash_sprites_[1].index=1;w.redraw();
   assert(drawn_frame==(scenario=="demo-draw0" ? 1:0));return 0;
  }
  w.on_left_down(7,9);assert(released==1);
  int expected=scenario=="demo-1" || scenario=="demo-all" ? 1:scenario=="demo-2" || scenario=="demo-live" ? 2:scenario=="demo-none" ? 0:3;
  assert(selection==expected);
  assert(probes==(expected==1 ? 1:expected==2 ? 2:scenario=="demo-3" ? 3:4));
  return 0;
 }
 if(scenario.find("climate")==0){
  auto &s=GameDataStateStorage.fields_;s={scenario=="climate-down" ? -2:2,3,2,5};MapSeaLevel=10;
  if(scenario=="climate-zero")s.climate_sea_level_delta_=0;
  if(scenario=="climate-wait")s.climate_rise_step_=1;
  climactic_changes();
  if(scenario=="climate-zero")assert(events.empty() && s.climate_rise_accum_==3 && MapSeaLevel==10);
  else if(scenario=="climate-wait")assert(events.empty() && s.climate_rise_accum_==4 && MapSeaLevel==10);
  else {assert(events=="WMP" && s.climate_rise_accum_==0 && s.climate_sea_level_delta_==99);
   assert(MapSeaLevel==(scenario=="climate-down" ? 9:11));
   assert(popup==(scenario=="climate-down" ? "SEAFALLING":"SEARISING"));}
  return 0;
 }
 GraphicWin one,two;first=&one;second=&two;one.identity=1;two.identity=2;
 one.dib_.bmiHeader.biWidth=40;one.dib_.bmiHeader.biHeight=-50;
 two.dib_.bmiHeader.biWidth=60;two.dib_.bmiHeader.biHeight=-70;
 current=scenario=="initial-null" ? 0:first;tracking=true;
 char filename[]="opening.pcx";int result=change_opening(filename);
 if(scenario=="initial-null"){assert(result==7 && events=="CD" && copied==0);}
 else {assert(result==0 && events=="CLPUD" && copied==1);
 assert(updated==((scenario=="copy-swap" || scenario=="load-swap") ? 2:1));}
 tracking=false;
}
'''


def run_host(tmp_path,scenario,mutant=None):
    opening=definition('int __cdecl change_opening(')
    climate=definition('void __cdecl climactic_changes(')
    if mutant=='stale_update':opening=opening.replace('opening_window()->on_update();','window->on_update();')
    if mutant=='early_null_guard':opening=opening.replace('GraphicWin *window = opening_window();','GraphicWin *window = opening_window();\n    if (!window) return 7;')
    if mutant=='popup_from_mutated_delta':climate=climate.replace('if (sign > 0)','if (GameDataStateStorage.fields_.climate_sea_level_delta_ > 0)')
    compiler=shutil.which('clang++') or shutil.which('g++');assert compiler
    cpp=tmp_path/('control'+(mutant or '')+'.cpp');exe=cpp.with_suffix('')
    cpp.write_text(HARNESS.replace('// ACTUAL',climate+'\n'+opening+'\n'+definition('void Demo::redraw(')+'\n'+definition('void Demo::on_left_down(')))
    built=subprocess.run([compiler,'-std=c++11','-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(exe),scenario],text=True,capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})


@pytest.mark.parametrize('scenario',['normal','initial-null','load-swap','copy-swap','load-error','climate-up','climate-down','climate-zero','climate-wait','demo-1','demo-2','demo-3','demo-4','demo-none','demo-all','demo-live','demo-draw0','demo-draw1'])
def test_actual_lifetime_callbacks_and_climate(tmp_path,scenario):
    result=run_host(tmp_path,scenario);assert result.returncode==0,result.stdout+result.stderr


def test_original_unchecked_window_after_load(tmp_path):
    result=run_host(tmp_path,'load-null');assert result.returncode!=0
    assert 'null pointer' in result.stderr or 'AddressSanitizer' in result.stderr


@pytest.mark.parametrize(('scenario','mutant'),[('copy-swap','stale_update'),('climate-down','popup_from_mutated_delta')])
def test_rejected_callback_snapshot_changes(tmp_path,scenario,mutant):
    result=run_host(tmp_path,scenario,mutant);assert result.returncode!=0


def test_pinned_window_receiver_and_climate_order():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned original unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    def ops(a,b):return {i.address:(i.mnemonic,i.op_str)for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(a,b)),a)}
    opening=ops(0x589AA6,0x589AFB)
    assert opening[0x589AAF]==('lea','edx, [eax + 0x444]')
    assert opening[0x589AB7]==('xor','edx, edx')
    assert opening[0x589AB9]==('mov','ecx, dword ptr [eax + 0x4c4]')
    assert opening[0x589ABF]==('mov','eax, dword ptr [eax + 0x4c8]')
    assert opening[0x589AED]==('mov','ecx, dword ptr [0x945824]')
    climate=ops(0x524259,0x524280)
    assert climate[0x524259]==('mov','edx, dword ptr [0x94987c]')
    assert climate[0x524263]==('mov','dword ptr [0x9a67d8], eax')
    assert climate[0x524268]==('mov','dword ptr [0x94987c], edx')
    assert climate[0x52427D]==('test','esi, esi')


def test_pinned_demo_distinct_hit_paths():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned original unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions={i.address:(i.mnemonic,i.op_str)for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x5243A0,0x52445B)),0x5243A0)}
    assert instructions[0x5243C1]==('call','0x5fa7e0')
    assert instructions[0x5243F1]==('call','0x5fa7e0')
    assert instructions[0x524421]==('call','0x5fa7e0')
    assert instructions[0x524434]==('call','0x5fa7e0')
    assert instructions[0x52442B]==('jne','0x524440')
    assert instructions[0x524440]==('mov','dword ptr [esi + 0xa14], 3')
