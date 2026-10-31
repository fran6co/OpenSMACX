"""Actual PlanWin projections and callback-visible rendering state controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

ROOT=Path(__file__).resolve().parents[2]


def definition(signature):
    text=(ROOT/'src/planwin.cpp').read_text()
    start=text.index(signature);opening=text.index('{',start);depth=0
    for end in range(opening,len(text)):
        depth+=(text[end]=='{')-(text[end]=='}')
        if depth==0:return text[start:end+1]
    raise AssertionError(signature)


HARNESS=r'''
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
static int last_color;
struct Buffer {
 struct {struct {int biWidth,biHeight;} bmiHeader;} dib_;
 void copy(Buffer*,int,int,int,int){}
 void line(int,int,int,int,int,int color,int,int,int){last_color=color;}
};
struct GraphicWin:Buffer {void on_update(){};};
struct Base {int16_t x,y;uint8_t faction_id_current,visibility;};
Base Bases[8];
struct Player {unsigned flags;};Player PlayersData[32];
struct Tile {unsigned visibility;};Tile tiles[32];Tile *MapTiles=tiles;
struct {int faction_id_;} ConsoleGlobal;
struct {struct {unsigned game_state_;int base_current_count_;}fields_;} GameDataStateStorage;
const unsigned STATE_OMNISCIENT_VIEW=128,PFLAG_MAP_REVEALED=512;
int MapLongitude=4,MapLongitudeBounds=100,MapIsFlat=0,PlanLineCounter=0;
struct PlanLineRecord {int x1,y1,x2,y2;};
static std::string scenario;
static std::vector<int> projections,drawn,factions;
struct PlanWin:GraphicWin {
 Buffer buffer_;
 int field_21A68_=1,field_21A6C_=0,line_count_=0,field_1DDC0_=0,field_1DDBC_=0,zoom_offset_=0;
 int center_x_,center_y_;PlanLineRecord line_records_[4];int line_colors_[4];
 void on_redraw();void draw_line_base(int,int,int);void find_view(int,int,int,int,int,int,int,int);
 void tile_to_pixel(int x,int y,int *px,int *py){
  projections.push_back(x);projections.push_back(y);*px=x+2;*py=y+3;
  if(scenario=="project-faction")ConsoleGlobal.faction_id_=4;
  if(scenario=="pattern" && projections.size()==6){line_records_[0].x2=90;line_records_[0].y2=91;line_colors_[0]=99;}
 }
};
static PlanWin *active;
void base_draw(Buffer*,int id,int,int,int,int){drawn.push_back(id);ConsoleGlobal.faction_id_=2;}
int veh_fake(int,int faction){factions.push_back(faction);return 7;}
void veh_draw(Buffer*,int,int,int,int,int,int){
 active->line_records_[0].x1=30;active->line_records_[0].y1=31;
 ConsoleGlobal.faction_id_=2;
}
// ACTUAL
int main(int argc,char **argv){
 assert(argc==2);scenario=argv[1];PlanWin w;active=&w;ConsoleGlobal.faction_id_=1;
 for(int i=0;i<32;++i)PlayersData[i].flags=PFLAG_MAP_REVEALED;
 if(scenario=="base-faction"){
  GameDataStateStorage.fields_.base_current_count_=2;
  Bases[0]={10,11,1,0};Bases[1]={20,21,2,0};w.on_redraw();
  assert(drawn.size()==2 && drawn[0]==0 && drawn[1]==1);
 }else if(scenario=="endpoint" || scenario=="project-faction"){
  w.line_count_=1;w.line_records_[0]={10,11,20,21};w.on_redraw();
  assert(projections.size()==8 && projections[0]==20 && projections[1]==21);
  assert(projections[2]==30 && projections[3]==31);
  assert(factions.size()==2);
  assert(factions[0]==(scenario=="project-faction" ? 4:1));
  assert(factions[1]==(scenario=="project-faction" ? 4:2));
 }else if(scenario=="negative-tile"){
  PlayersData[1].flags=0;Base b={-1,1,1,0};tiles[3].visibility=2;
  assert(planwin_base_known_to_faction(b,1));
 }else if(scenario=="pattern"){
  GameDataStateStorage.fields_.game_state_=STATE_OMNISCIENT_VIEW;
  GameDataStateStorage.fields_.base_current_count_=2;Bases[0]={10,11,1,0};Bases[1]={20,21,2,0};
  w.line_count_=1;w.line_records_[0]={10,11,20,21};w.line_colors_[0]=5;w.on_redraw();
  assert(projections.size()==8 && projections[6]==20 && projections[7]==21 && last_color==5);
 }else if(scenario=="flat-find"){
  MapIsFlat=1;w.find_view(0,2,10,4,20,6,30,8);assert(w.center_x_==15 && w.center_y_==5);
 }else if(scenario=="wrapped-find"){
  w.find_view(90,2,10,4,20,6,30,8);assert(w.center_x_==15 && w.center_y_==5);
 }else if(scenario=="negative-find"){
  w.find_view(-9,-5,-4,-4,-3,-3,0,-2);assert(w.center_x_==95 && w.center_y_==-3);
 }else if(scenario=="line"){
  Bases[0]={10,11,1,0};Bases[1]={20,21,2,0};w.draw_line_base(0,1,5);
  assert(projections==std::vector<int>({10,11,20,21}));
 }
}
'''


def program(mutant=None):
    helpers=definition('static bool planwin_base_known_to_faction(')+'\n'+definition('static bool planwin_tile_has_known_base(')
    redraw=definition('void PlanWin::on_redraw(')
    if mutant=='cached_faction':
        redraw=redraw.replace('faction = ConsoleGlobal.faction_id_;','faction = faction;',2)
        # Keep the initial declaration valid, but lose subsequent callback refreshes.
        redraw=redraw.replace('int faction = faction;','int faction = ConsoleGlobal.faction_id_;')
    if mutant=='cached_endpoints':
        redraw=redraw.replace('        int endpoint_index;','        const PlanLineRecord snapshot = record;\n        int endpoint_index;')
        redraw=redraw.replace('endpoint_index != 0 ? record.', 'endpoint_index != 0 ? snapshot.').replace(': record.y2',': snapshot.y2').replace(': record.x2',': snapshot.x2')
    if mutant=='cached_fake':redraw=redraw.replace('veh_fake(2, ConsoleGlobal.faction_id_)','veh_fake(2, faction)')
    if mutant=='late_pattern':
        redraw=redraw.replace('const PlanLineRecord record =','const PlanLineRecord &record =').replace('const int color = line_colors_[pattern_line_index];','').replace('color,','line_colors_[pattern_line_index],')
    if mutant=='truncated_tile':helpers=helpers.replace('(base.x >> 1)','(base.x / 2)')
    midpoint=definition('static int plan_horizontal_midpoint(') if 'static int plan_horizontal_midpoint(' in (ROOT/'src/planwin.cpp').read_text() else ''
    return HARNESS.replace('// ACTUAL',midpoint+'\n'+helpers+'\n'+redraw+'\n'+definition('void PlanWin::draw_line_base(')+'\n'+definition('void PlanWin::find_view('))


def run_host(tmp_path,scenario,mutant=None):
    compiler=shutil.which('clang++') or shutil.which('g++');assert compiler
    cpp=tmp_path/('control'+(mutant or '')+'.cpp');exe=cpp.with_suffix('')
    cpp.write_text(program(mutant))
    built=subprocess.run([compiler,'-std=c++11','-O1',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(exe),scenario],text=True,capture_output=True)


@pytest.mark.parametrize('scenario',['base-faction','endpoint','project-faction','negative-tile','line','flat-find','wrapped-find','negative-find','pattern'])
def test_actual_read_boundaries(tmp_path,scenario):
    result=run_host(tmp_path,scenario);assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize(('scenario','mutant'),[('base-faction','cached_faction'),('endpoint','cached_endpoints'),('project-faction','cached_fake'),('negative-tile','truncated_tile'),('pattern','late_pattern')])
def test_rejected_early_snapshots_and_truncation(tmp_path,scenario,mutant):
    result=run_host(tmp_path,scenario,mutant);assert result.returncode!=0


def test_pinned_callback_and_signed_tile_read_sites():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned original unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x48AF30,0x48B3B7)),0x48AF30)}
    assert instructions[0x48AFE5]==('sar','eax, 1')
    assert instructions[0x48B0E2]==('mov','esi, dword ptr [0x939284]')
    assert instructions[0x48B287]==('mov','esi, dword ptr [0x939284]')
    assert instructions[0x48B230]==('mov','eax, dword ptr [0x939284]')
    assert instructions[0x48B130]==('test','ecx, ecx')
    assert instructions[0x48B134]==('mov','ecx, dword ptr [eax - 4]')
    assert instructions[0x48B2D9]==('mov','eax, dword ptr [esi + 4]')
    assert instructions[0x48B2E6]==('mov','dword ptr [ebp - 0x2c], edx')
    assert instructions[0x48B306]==('mov','edx, dword ptr [ebp - 0x24]')
    assert instructions[0x48B13E]==('mov','ecx, dword ptr [eax + 4]')
