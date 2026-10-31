"""Actual Base distance and scan controls; host witnesses are not runtime proof."""
from pathlib import Path
import os
import shutil
import subprocess
import pytest
from test_mapwin_cache import braced
ROOT=Path(__file__).resolve().parents[2]
HOST=r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#define __cdecl
const int MaxPlayerNum=8,PLAN_SUPPLY_CONVOY=10,ORDER_CONVOY=3,ORDER_NONE=0,RSC_ENERGY=2,BIT_BASE_IN_TILE=1;
struct Base{int16_t x,y;uint8_t faction_id_current;int energy_surplus;};Base Bases[8];Base *BaseCurrent=Bases;
struct Veh{int16_t x,y,proto_id;uint8_t faction_id,order,order_auto_type;};Veh Vehs[8];
struct Proto{int plan;};Proto VehPrototypes[2];
struct Map{unsigned bit;unsigned char val2;};Map tiles[32];Map *MapTiles=tiles;
int MapLongitude=10,MapLongitudeBounds=20,MapIsFlat=0,BaseFindDist;
struct{struct{int base_current_count_,vehicle_current_count_;}fields_;}GameDataStateStorage;
static std::string scenario;
Map *map_loc(int x,int y){return MapTiles+y*MapLongitude+(x>>1);}
// ACTUAL
void initialize_scan(){
 BaseCurrent=Bases;BaseCurrent->faction_id_current=1;BaseCurrent->energy_surplus=-1;
 GameDataStateStorage.fields_.vehicle_current_count_=3;VehPrototypes[0].plan=PLAN_SUPPLY_CONVOY;
 MapLongitude=4;
 for(int i=0;i<3;++i){Vehs[i]={static_cast<int16_t>(i*2),0,0,1,ORDER_CONVOY,RSC_ENERGY};tiles[i]={BIT_BASE_IN_TILE,1};}
}
int main(){
 BaseFindDist=777;GameDataStateStorage.fields_.base_current_count_=0;
 assert(base_find(0,0)==-1 && BaseFindDist==777);
 GameDataStateStorage.fields_.base_current_count_=2;MapIsFlat=1;
 Bases[0]={0,0,1,0};Bases[1]={4,0,1,0};assert(base_find(2,0)==1 && BaseFindDist==1);
 MapIsFlat=0;Bases[0]={19,1,1,0};Bases[1]={10,1,1,0};assert(base_find(1,1)==0 && BaseFindDist==1);
 MapIsFlat=1;Bases[0]={0,-5,1,0};Bases[1]={0,2,1,0};assert(base_find(0,-4)==0 && BaseFindDist==1);
 GameDataStateStorage.fields_.base_current_count_=1;Bases[0]={30000,30000,1,0};BaseFindDist=777;
 assert(base_find(0,0)==-1 && BaseFindDist==777);
 initialize_scan();tiles[1].val2=8;Vehs[2].faction_id=2;base_energy_costs();
 assert(Vehs[0].order==ORDER_NONE && Vehs[1].order==ORDER_CONVOY && Vehs[2].order==ORDER_CONVOY);
 initialize_scan();BaseCurrent->energy_surplus=0;base_energy_costs();assert(Vehs[0].order==ORDER_CONVOY);
 initialize_scan();GameDataStateStorage.fields_.vehicle_current_count_=-1;base_energy_costs();assert(Vehs[0].order==ORDER_CONVOY);
 initialize_scan();scenario="shrink";base_energy_costs();
 assert(GameDataStateStorage.fields_.vehicle_current_count_==0);
 assert(Vehs[0].order==ORDER_NONE && Vehs[1].order==ORDER_NONE && Vehs[2].order==ORDER_NONE);
}
'''
def program(mutant=None):
    source=(ROOT/'src/base.cpp').read_text();mapping=(ROOT/'src/map.h').read_text()
    find=braced(source,'int __cdecl base_find(int x, int y)')
    energy=braced(source,'void __cdecl base_energy_costs()')
    # Observe the live-bound timing after an original store. The native scan
    # has no callback here; this is a controlled source-read witness only.
    old='vehicle->order = ORDER_NONE;';assert energy.count(old)==1
    energy=energy.replace(old,'{ vehicle->order = ORDER_NONE; if(scenario=="shrink") GameDataStateStorage.fields_.vehicle_current_count_=0; }')
    if mutant=='tie':find=find.replace('<= proximity','< proximity')
    if mutant=='far':find=find.replace('int proximity = 9999;','int proximity = 2147483647;')
    if mutant=='live':energy=energy.replace('while (remaining != 0);','while (remaining != 0 && GameDataStateStorage.fields_.vehicle_current_count_ > 0);')
    return HOST.replace('// ACTUAL',braced(mapping,'int __cdecl x_dist(')+'\n'+braced(mapping,'int __cdecl base_who(')+'\n'+find+'\n'+energy)
def execute(tmp_path,compiler,mutant=None):
    cpp=tmp_path/('actual'+(mutant or '')+'.cpp');exe=cpp.with_suffix('');cpp.write_text(program(mutant))
    result=subprocess.run([compiler,'-std=c++11','-O1',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert result.returncode==0,result.stdout+result.stderr
    return subprocess.run([str(exe)],text=True,capture_output=True)
@pytest.mark.parametrize('compiler',['g++','clang++'])
def test_actual_distance_and_count_snapshot(tmp_path,compiler):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip('host compiler unavailable')
    result=execute(tmp_path,compiler);assert result.returncode==0,result.stdout+result.stderr
@pytest.mark.parametrize('mutant',['tie','far','live'])
def test_rejected_distance_and_live_bound_changes(tmp_path,mutant):
    compiler=shutil.which('clang++') or shutil.which('g++');assert compiler
    assert execute(tmp_path,compiler,mutant).returncode!=0
def test_pinned_coordinate_capture_and_snapshot_counter():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image=PeImage(Path(os.environ['OPENSMACX_IMAGE']).read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    def ops(a,b):return {i.address:(i.mnemonic,i.op_str)for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(a,b)),a)}
    find=ops(0x4E3B80,0x4E3C5F);scan=ops(0x4F4DC0,0x4F4E73)
    vulnerable=ops(0x59E980,0x59EA75)
    assert vulnerable[0x59E9F9]==('movsx','edi, word ptr [eax + 0x97d042]')
    assert vulnerable[0x59EA0A]==('call','0x644f3a')
    assert find[0x4E3BB6]==('movsx','edi, word ptr [ebx + 2]')
    assert find[0x4E3BBD]==('call','0x644f3a')
    assert find[0x4E3C22]==('jg','0x4e3c2d')
    assert scan[0x4F4DCD]==('mov','cl, byte ptr [eax + 4]')
    assert scan[0x4F4DD3]==('mov','ecx, dword ptr [eax + 0xd8]')
    assert scan[0x4F4E01]==('mov','dword ptr [ebp - 4], eax')
    assert scan[0x4F4E61]==('mov','eax, dword ptr [ebp - 4]')
    assert scan[0x4F4E67]==('dec','eax')
