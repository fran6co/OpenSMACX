"""Actual bounded combat modifiers; lookup doubles are not whole-game proof."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

ROOT=Path(__file__).resolve().parents[2]
BASE="146be170c3e6300b829018f24e62b16653102c20"


def modifiers_region(text):
    start=text.index("                if (psi_combat_type) {\n                    if (base_id_def")
    end=text.index("                    LPSTR display_def",start)
    return text[start:end]


def terrain_block(text):
    start=text.index("                    uint32_t triad_def = get_proto_triad(proto_id_def);")
    end=text.index("                    LPSTR display_def",start)
    return text[start:end]


def psi_block(text):
    region=modifiers_region(text)
    start=region.index("                    if (base_id_def")
    end=region.index("                    defense *= 4;")+len("                    defense *= 4;")
    return region[start:end]


def test_only_modifier_region_changes_and_original_obligations_survive():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path=ROOT/"src/veh.cpp"
    old=subprocess.check_output(["git","show",BASE+":src/veh.cpp"],cwd=ROOT,text=True)
    new=path.read_text()
    assert old.replace(modifiers_region(old),"REVIEWED_MODIFIERS")==new.replace(
        modifiers_region(new),"REVIEWED_MODIFIERS")
    assert [(_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(old,path)]==[
        (_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(new,path)]


HOST=r'''
#include <cassert>
#include <cstdio>
#include <cstring>
typedef unsigned int uint32_t;
enum {TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2,TERRAIN_BIT_ROCKY=0x80,
      ABL_ANTIGRAV_STRUTS=0x400};
struct RuleState {
 int combat_pct_base_def,combat_pct_art_bonus_lvl_alt,combat_pct_mobile_open_ground;
 int combat_pct_def_vs_mobile_rough,combat_pct_atk_road,combat_pen_pct_atk_lwr_elev;
 int combat_pct_atk_higher_elev,move_rate_roads;
};
static RuleState Rules;
struct Scenario {
 int psi,base,attacker,combat,triad_atk,triad_def,terrain,rocky,speed_atk,speed_def;
 int level_atk,level_def;bool anti_atk,anti_def;RuleState rules;
 Scenario():psi(0),base(-1),attacker(0),combat(0),triad_atk(0),triad_def(0),
 terrain(2),rocky(0),speed_atk(4),speed_def(4),level_atk(2),level_def(2),
 anti_atk(false),anti_def(false) { memset(&rules,0,sizeof(rules));rules.move_rate_roads=3; }
};
static Scenario state;
struct {int x,y;} Vehs[1]={{10,20}};
static int terrain_calls;
static int get_proto_triad(int id){assert(id==2);return state.triad_def;}
static int get_triad(int id){assert(id==0);return state.triad_atk;}
static int defense_value(int,int,int,int,int){++terrain_calls;return state.terrain;}
static int rocky_at(int,int){return state.rocky;}
static int speed_proto(int id){return id==1?state.speed_atk:state.speed_def;}
static int alt_at(int x,int y){return x==10&&y==20?state.level_atk:state.level_def;}
static int altitude_at(int x,int y){return alt_at(x,y)*32;}
static bool has_abil(int id,int ability){assert(ability==ABL_ANTIGRAV_STRUTS);return id==1?state.anti_atk:state.anti_def;}
static char labels[1200][24];
static const char *label_get(int id){sprintf(labels[id],"%d",id);return labels[id];}
static char StringTemp[64];
static const char *VehBattleDisplayTerrain="mock terrain";
struct Strings {const char *get(const char *p){return p;}} StringTable;
static int *tracked_offense,*tracked_defense;
struct Note {int side,pct,offense,defense;char text[64];} notes[16];
static int note_count;
static void add_bat(int side,int pct,const char *text){
 assert(note_count<16);notes[note_count].side=side;notes[note_count].pct=pct;
 notes[note_count].offense=*tracked_offense;notes[note_count].defense=*tracked_defense;
 strcpy(notes[note_count++].text,text);
}
struct Result {int offense,defense;};
static Result run(Scenario s){
 state=s;Rules=s.rules;terrain_calls=note_count=0;
 int offense=101,defense=101;
 tracked_offense=&offense;tracked_defense=&defense;
 const int base_id_def=s.base,veh_id_atk=s.attacker,veh_id_def=1;
 const int proto_id_atk=1,proto_id_def=2,faction_id_def=2,x_def=30,y_def=40;
 const int combat_type=s.combat;
 if(s.psi){ // ACTUAL_PSI
 } else { // ACTUAL_TERRAIN
 }
 Result r={offense,defense};return r;
}
static int percent(int accumulator,int pct){return (int)((long long)accumulator*(100+pct)/100);}
static int find_note(int label){
 char wanted[24];sprintf(wanted,"%d",label);
 for(int i=0;i<note_count;++i)if(strcmp(notes[i].text,wanted)==0)return i;
 return -1;
}
int main(){
 const int pcts[]={-125,-100,-50,-25,0,25,50,100,125};
 const int report_labels[]={611,548,606,441,330,576,332};
 for(int family=0;family!=7;++family)for(unsigned i=0;i<sizeof(pcts)/sizeof(pcts[0]);++i){
  Scenario s;int pct=pcts[i],effective=pct,side=0;
  switch(family){
   case 0:s.rules.combat_pct_mobile_open_ground=pct;break;
   case 1:s.rules.combat_pct_def_vs_mobile_rough=pct;s.terrain=3;side=1;break;
   case 2:s.rules.combat_pct_atk_road=pct;break;
   case 3:s.rules.combat_pen_pct_atk_lwr_elev=pct;s.level_def=3;s.level_atk=1;side=1;break;
   case 4:s.rules.combat_pct_atk_higher_elev=pct;s.level_def=1;s.level_atk=3;break;
   case 5:s.rules.combat_pct_art_bonus_lvl_alt=pct;s.combat=1;s.level_def=1;s.level_atk=3;effective=pct*2;break;
   case 6:s.psi=1;s.base=0;s.rules.combat_pct_base_def=pct;side=1;break;
  }
  Result r=run(s);
  assert(r.offense==(side?101:percent(101,effective)));
  assert(r.defense==(side?percent(101,effective):101)*(s.psi?4:s.terrain));
  int note=find_note(report_labels[family]);
  assert(pct?(note>=0&&notes[note].side==side&&notes[note].pct==effective):note==-1);
  if(pct){
   if(family==3||family==6)assert(notes[note].defense==101);
   else if(family==4)assert(notes[note].offense==101);
   else if(side)assert(notes[note].defense==percent(101,effective));
   else assert(notes[note].offense==percent(101,effective));
  }
 }
 Scenario s;Result r;
 s.rules.combat_pct_mobile_open_ground=25;r=run(s);assert(r.offense==126);
 s.speed_def=5;r=run(s);assert(r.offense==101);
 s.speed_def=3;s.speed_atk=3;r=run(s);assert(r.offense==101);
 s.speed_atk=-1;s.speed_def=0;r=run(s);assert(r.offense==101);
 s=Scenario();s.rules.combat_pct_mobile_open_ground=25;s.base=0;r=run(s);assert(r.offense==101);
 s.base=-1;s.rocky=0x80;r=run(s);assert(r.offense==101);
 s.rocky=0;s.terrain=3;r=run(s);assert(r.offense==101);
 s=Scenario();s.rules.combat_pct_def_vs_mobile_rough=25;s.base=0;r=run(s);assert(r.defense==252);
 assert(find_note(612)>=0);
 s.base=-1;r=run(s);assert(r.defense==202);
 s.terrain=3;s.rocky=0x80;r=run(s);assert(r.defense==378);
 s=Scenario();s.rules.combat_pct_atk_road=25;s.combat=1;r=run(s);assert(r.offense==101);
 s.combat=0;s.triad_atk=1;r=run(s);assert(r.offense==101);
 s.triad_atk=0;s.attacker=-1;r=run(s);assert(r.offense==101);
 s=Scenario();s.rules.combat_pen_pct_atk_lwr_elev=25;s.level_def=3;s.level_atk=1;
 s.anti_atk=true;r=run(s);assert(r.defense==202);
 s.anti_atk=false;s.anti_def=true;r=run(s);assert(r.defense==252);
 s.level_def=1;r=run(s);assert(r.defense==202);
 s=Scenario();s.rules.combat_pct_atk_higher_elev=25;s.level_atk=3;s.level_def=1;
 s.anti_def=true;r=run(s);assert(r.offense==101);
 s.anti_def=false;s.anti_atk=true;r=run(s);assert(r.offense==126);
 s.level_atk=1;r=run(s);assert(r.offense==101);
 s=Scenario();s.rules.combat_pct_art_bonus_lvl_alt=25;s.combat=1;s.level_atk=4;s.level_def=1;
 r=run(s);assert(r.offense==176&&notes[find_note(576)].pct==75);
 s.triad_def=1;r=run(s);assert(r.offense==101);
 s.triad_def=0;s.level_atk=1;r=run(s);assert(r.offense==101);
 s=Scenario();s.psi=1;s.base=-1;s.rules.combat_pct_base_def=25;r=run(s);assert(r.defense==404&&note_count==0);
 s.base=1;r=run(s);assert(r.defense==504);
 s=Scenario();s.triad_def=2;s.terrain=3;r=run(s);assert(r.defense==202&&terrain_calls==0);
 s=Scenario();s.rules.combat_pct_mobile_open_ground=25;s.rules.combat_pct_atk_road=50;
 s.rules.combat_pct_atk_higher_elev=25;s.level_atk=3;s.level_def=1;r=run(s);
 assert(r.offense==236&&note_count==3&&strcmp(notes[0].text,"611")==0&&
        strcmp(notes[1].text,"606")==0&&strcmp(notes[2].text,"330")==0);
 return 0;
}
'''


MUTATIONS=[None,
    ("speed_proto(proto_id_def) <= speed_atk", "speed_proto(proto_id_def) < speed_atk"),
    ("int speed_atk", "uint32_t speed_atk"),
    ("add_bat(0, Rules.combat_pct_atk_higher_elev, label_get(330))",
     "add_bat(0, Rules.combat_pct_atk_road, label_get(330))"),
    ("* (alt_atk - alt_def), label_get(576)", ", label_get(576)"),
    ("* (alt_atk - alt_def) + 100) * offense / 100",
     "* (alt_atk - alt_def) + 100) / 100 * offense"),
    ("&& (terrain_def > 2 || base_id_def >= 0)",
     "&& (terrain_def > 2 || base_id_def >= 0) && rocky_at(x_def,y_def)<TERRAIN_BIT_ROCKY"),
    ("add_bat(0, Rules.combat_pct_atk_higher_elev, label_get(330)); // \"Downhill\"\n"
     "                            offense = (Rules.combat_pct_atk_higher_elev + 100) * offense / 100;",
     "offense = (Rules.combat_pct_atk_higher_elev + 100) * offense / 100;\n"
     "                            add_bat(0, Rules.combat_pct_atk_higher_elev, label_get(330));"),
]
for rule,target in [("combat_pct_base_def","defense"),
                    ("combat_pct_mobile_open_ground","offense"),
                    ("combat_pct_def_vs_mobile_rough","defense"),
                    ("combat_pct_atk_road","offense"),
                    ("combat_pen_pct_atk_lwr_elev","defense"),
                    ("combat_pct_atk_higher_elev","offense")]:
    MUTATIONS.append((f"(Rules.{rule} + 100) * {target} / 100",
                      f"((Rules.{rule} + 100) / 100) * {target}"))


@pytest.mark.parametrize("compiler",["g++","clang++"])
@pytest.mark.parametrize("mutation",MUTATIONS)
def test_actual_modifiers_and_damage_controls(compiler,mutation,tmp_path):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip("host compiler unavailable")
    text=(ROOT/"src/veh.cpp").read_text()
    psi=psi_block(text);terrain=terrain_block(text)
    actual=HOST.replace("// ACTUAL_PSI",psi).replace("// ACTUAL_TERRAIN",terrain)
    if mutation:
        assert actual.count(mutation[0])==1
        actual=actual.replace(*mutation)
    source=tmp_path/"modifiers.cpp";source.write_text(actual)
    binary=tmp_path/"modifiers"
    built=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(binary)],capture_output=True,text=True)
    assert built.returncode==0,built.stdout+built.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True,
                          preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode<0 if mutation else result.returncode==0),result.stdout+result.stderr


def test_native_modifier_products_signed_gates_and_report_rules():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.exists():pytest.skip("pinned executable unavailable")
    image=PeImage(path.read_bytes(),image_pin(ROOT))
    decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    rows={}
    for lo,hi in [(0x502C21,0x5031B3),(0x5037E9,0x503875)]:
        rows.update({i.address:(i.mnemonic,i.op_str) for i in decoder.disasm(image.read(Span(lo,hi)),lo)})
    for address,expected in {
        0x502D0F:("imul","ecx, dword ptr [ebp - 4]"),
        0x502D3B:("imul","esi, dword ptr [0x9497f0]"),
        0x502DED:("jle","0x502e8c"),0x502E17:("jg","0x502e8c"),
        0x502E27:("imul","ecx, ebx"),
        0x502EE1:("imul","ecx, dword ptr [ebp - 8]"),
        0x502F74:("imul","ecx, ebx"),
        0x503064:("call","0x6169a0"),0x5030B2:("imul","ecx, dword ptr [ebp - 8]"),
        0x503154:("call","0x6169a0"),0x503159:("mov","ecx, dword ptr [0x9497c0]"),
        0x503190:("mov","ecx, dword ptr [0x9497c0]"),
        0x5031A1:("imul","ecx, ebx"),
        0x503856:("imul","ecx, dword ptr [ebp - 8]"),
        0x503851:("mov","eax, 0x51eb851f"),
        0x50385C:("sar","edx, 5"),0x503861:("shr","eax, 0x1f"),
    }.items():assert rows[address]==expected
