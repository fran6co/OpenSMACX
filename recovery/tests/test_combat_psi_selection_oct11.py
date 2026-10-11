"""Actual initial PSI selection/display; host backing does not prove reachability."""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT=Path(__file__).resolve().parents[2]
BASE="a6dd9e351"


def psi_region(text):
    lo=text.index("    if (!is_bombard ||")
    hi=text.index("        if (veh_id_atk >= 0) {\n            offense = get_basic_offense",lo)
    return text[lo:hi]


def restore_psi_region(current,prior):
    return current.replace(psi_region(current),psi_region(prior),1)


def test_only_initial_psi_region_changes_preserving_original_obligations():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path=ROOT/"src/veh.cpp"
    prior=subprocess.check_output(["git","show",BASE+":src/veh.cpp"],cwd=ROOT,text=True)
    current=path.read_text()
    assert restore_psi_region(current,prior)==prior
    assert [(_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(prior,path)]==[
        (_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(current,path)]


HOST=r'''
#include <cassert>
#include <cstring>
typedef unsigned int uint32_t; typedef int BOOL; typedef char *LPSTR;
enum {BSC_SPORE_LAUNCHER=15,BSC_FUNGAL_TOWER=9,SP_DREAM_TWISTER=1,SP_NEURAL_AMPLIFIER=2};
struct Vehicle {short proto_id; unsigned char faction_id;};
static Vehicle backing[3]; static Vehicle *Vehs=backing+1;
struct Player {int rule_psi;}; static Player Players[8];
struct Prototype {char *veh_name;}; static Prototype VehPrototypes[32];
static int offense_rating[32],defense_rating[32],project_faction[3];
static int labels,report_count,observed_def_faction;
struct Report {int side,value,label;}; static Report reports[8];
static char label_names[1200][16];
static void battle_init(){report_count=labels=0;}
static bool is_proto_missile(int p){assert(p>=0&&p<32);return p==7;}
static int get_proto_offense_rating(int p){assert(p>=0&&p<32);return offense_rating[p];}
static int get_proto_defense_rating(int p){assert(p>=0&&p<32);return defense_rating[p];}
static bool has_project(int p,int faction){assert(faction>=0&&faction<8);return project_faction[p]==faction;}
static LPSTR label_get(int id){
 ++labels;
 return label_names[id];
}
static void add_bat(int side,int value,LPSTR label){
 if(!value)return;
 assert(report_count<8); int id=label==label_names[342]?342:label==label_names[343]?343:label==label_names[344]?344:9;
 reports[report_count++]=Report{side,value,id};
}
static uint32_t run(int veh_id_atk,int veh_id_def,int combat_type){
/*ACTUAL*/
 }
 observed_def_faction=faction_id_def;
 return psi_combat_type;
}
int main(){
 for(int atk=-1;atk<=1;++atk)for(int def=-1;def<=1;++def)
 for(int mode=0;mode<4;++mode)for(int spore=0;spore<4;++spore)
 for(int rating=0;rating<4;++rating){
  memset(Players,0,sizeof(Players));memset(offense_rating,0,sizeof(offense_rating));memset(defense_rating,0,sizeof(defense_rating));
  for(int i=0;i<3;++i){backing[i].proto_id=i+1;backing[i].faction_id=i+2;}
  if(spore&1)Vehs[atk].proto_id=15;
  if(spore&2)Vehs[def].proto_id=15;
  if(rating&1)offense_rating[Vehs[atk].proto_id]=-1;
  if(rating&2)defense_rating[Vehs[def].proto_id]=-1;
  for(int i=0;i<8;++i)Players[i].rule_psi=10+i;
  project_faction[1]=Vehs[atk].faction_id;project_faction[2]=Vehs[def].faction_id;
  bool bombard=(mode&1)||Vehs[atk].proto_id==7;
  bool selected=!bombard||Vehs[atk].proto_id==15||Vehs[def].proto_id==15;
  unsigned psi=selected?((atk>=0&&offense_rating[Vehs[atk].proto_id]<0)?1:0)|((def>=0&&defense_rating[Vehs[def].proto_id]<0)?2:0):0;
  assert(run(atk,def,mode)==psi);
  assert(observed_def_faction==(selected||def>=0?Vehs[def].faction_id:0));
  assert(report_count==(psi?4:0));
  if(psi){assert(reports[0].side==0&&reports[0].value==10+Vehs[atk].faction_id&&reports[0].label==342);
   assert(reports[1].side==0&&reports[1].label==343&&reports[1].value==50);
   assert(reports[2].side==1&&reports[2].value==10+Vehs[def].faction_id&&reports[2].label==342);
   assert(reports[3].side==1&&reports[3].label==344&&reports[3].value==50);}
 }
}
'''


@pytest.mark.parametrize("compiler",["g++","clang++"])
@pytest.mark.parametrize("mutation",[None,"cached_prototypes","cached_factions","defender_bit","guarded_defender"])
def test_actual_source_selection_and_display_controls(compiler,mutation,tmp_path):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip("host compiler unavailable")
    text=(ROOT/"src/veh.cpp").read_text()
    lo=text.index("    int offense = 8;",text.index("void __cdecl battle_compute("))
    body=text[lo:text.index("        if (veh_id_atk >= 0) {\n            offense = get_basic_offense",lo)]
    if mutation=="cached_prototypes":
        body=body.replace("Vehs[veh_id_atk].proto_id == BSC_SPORE_LAUNCHER","proto_id_atk == BSC_SPORE_LAUNCHER").replace("Vehs[veh_id_def].proto_id == BSC_SPORE_LAUNCHER","proto_id_def == BSC_SPORE_LAUNCHER")
    if mutation=="cached_factions":
        pos=body.index("    if (!is_bombard ||")
        body=body[:pos]+body[pos:].replace("            faction_id_atk = Vehs[veh_id_atk].faction_id;","").replace("        faction_id_def = Vehs[veh_id_def].faction_id;","")
    if mutation=="defender_bit":
        pos=body.index("        faction_id_def = Vehs[veh_id_def].faction_id;",body.index("    if (!is_bombard ||"))
        body=body[:pos]+body[pos:].replace("if (psi_combat_type)","if (psi_combat_type & 2)",1)
    if mutation=="guarded_defender":
        pos=body.index("    if (!is_bombard ||")
        body=body[:pos]+body[pos:].replace("        faction_id_def = Vehs[veh_id_def].faction_id;","        if (psi_combat_type) faction_id_def = Vehs[veh_id_def].faction_id;")
    source=tmp_path/"control.cpp"; source.write_text(HOST.replace("/*ACTUAL*/",body))
    binary=tmp_path/"control"
    result=subprocess.run([compiler,"-std=c++11","-O0",str(source),"-o",str(binary)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr
    result=subprocess.run([str(binary)],capture_output=True)
    assert (result.returncode==0)==(mutation is None),result.stderr


def test_native_unchecked_reads_and_rule_reload_order():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    image_path=ROOT/".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():pytest.skip("pinned image unavailable")
    image=PeImage(image_path.read_bytes(),image_pin(ROOT))
    instructions={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x501E1F,0x502102)),0x501E1F)}
    for address,expected in {
        0x501E1F:("mov","esi, 0xf"),0x501E24:("je","0x501e52"),
        0x501E2C:("cmp","word ptr [eax*4 + 0x952832], si"),
        0x501E34:("je","0x501e52"),0x501E42:("cmp","word ptr [eax + 0x952832], si"),
        0x501E49:("jne","0x502102"),0x501ED0:("mov","dl, byte ptr [ecx*4 + 0x952836]"),
        0x501F08:("call","0x6169a0"),0x501F0D:("mov","edx, dword ptr [edi]"),
        0x501FC6:("mov","al, byte ptr [ebx + 0x952836]"),0x501FD1:("test","eax, eax"),
        0x502008:("call","0x6169a0"),0x50200D:("mov","edx, dword ptr [edi]")}.items():
        assert instructions[address]==expected
