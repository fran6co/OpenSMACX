"""Complete actual-body picker controls; not full-game/runtime acceptance."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "f997ed111e2877a3cbe230667fb42338392849eb"


def control():
    body = extract((ROOT / "src/designwin.cpp").read_text(),
                   "void DesignWin::select_special(int which)")
    return r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <vector>
typedef char *LPSTR;
static const int MaxAbilityNum=29,RFLAG_ALIEN=128,pop_wait=7;
static const char *PopupStartCaption="caption",*ScriptTxtID="script",*BufferJoinSpace=" ";
struct Player {unsigned rule_flags;int faction_bonus_id[41],faction_bonus_val1[41];} Players[2];
struct Tech {char *name;} Technology[2];
struct Rule {int tech_two_spec_abilities;} Rules;
struct ChassisRow {unsigned char missile,triad,speed;} Chassis[4];
struct CostRow {unsigned char cost;} Weapon[4],Armor[4];
struct AbilityRow {int16_t preq_tech;char *name,*description;int cost_factor;unsigned flags;} Ability[29];
static char label[]="label",ability_name[]="ability",description[]="description";
static char *label_rows[0x400],*Triad[3]={label,label,label};
struct LabelTable {char **strings_ptr;} Labels={label_rows};
struct TextTable {char *get(char *s){return s;}} StringTable;
static char StringTemp[4096];
static char *_itoa(int n,char *out,int){std::sprintf(out,"%d",n);return out;}
static int chosen,technology_missing,condition_missing,execs,live,closes;
static std::vector<int> options,repaints;
static const char *warning;
struct Dialogs;
static Dialogs *expected_dialog;
struct Dialogs {
 void item(const char *,int value){assert(this==expected_dialog);options.push_back(value);}
};
struct Popup {
 Dialogs owned;
 Popup(){++live;expected_dialog=&owned;}
 ~Popup(){close();--live;}
 void close(){++closes;}
 Dialogs &dialogs(){return owned;}
 void start(const char *caption,const char *section,int v,void *p,int flags){
  assert(caption==PopupStartCaption && !std::strcmp(section,"UNITSPECIAL"));
  assert(v==-1 && p==nullptr && flags==0x200042);
 }
 int exec(int a,int b){assert(a==0 && b==pop_wait);++execs;return chosen;}
};
static bool has_tech(int tech,int faction){assert(faction==1);return tech!=technology_missing;}
static int abil_cond(int ability,int chassis,int weapon,int armor){
 assert(chassis==3 && weapon==2 && armor==1);return ability!=condition_missing;
}
static void parse_say(int slot,char *,int a,int b){assert(slot==0 && a==-1 && b==-1);}
static void popp(const char *script,const char *section,int zero,const char *picture,int wait){
 assert(script==ScriptTxtID && zero==0 && wait==pop_wait);
 assert(!std::strcmp(picture,Players[1].rule_flags?"alunit_sm.pcx":"unit_sm.pcx"));warning=section;
}
struct DesignWin {
 uint32_t factionID_,protoChassisType_,protoWeaponType_,protoArmorType_;
 uint32_t protoAbilityID1_,protoAbilityID2_,protoAbilityFlags_;
 void select_special(int which);
 void draw_special_2(int a){assert(a==0);repaints.push_back(2);}
 void draw_special_1(int a){assert(a==0);repaints.push_back(1);}
 void draw_vehicle(int a){assert(a==0);repaints.push_back(3);}
 void draw_flash(){repaints.push_back(4);}
 void draw_info(){repaints.push_back(5);}
 void on_update(){repaints.push_back(6);}
};
BODY
static DesignWin reset(){
 std::memset(Players,0,sizeof Players);std::memset(Chassis,0,sizeof Chassis);
 chosen=29;technology_missing=condition_missing=-99;execs=live=closes=0;warning=nullptr;
 options.clear();repaints.clear();Rules.tech_two_spec_abilities=1;Technology[1].name=label;
 for(auto &row:label_rows)row=label;
 for(auto &row:Ability)row={0,ability_name,description,2,0};
 Weapon[2].cost=4;Armor[1].cost=3;Chassis[3].speed=2;
 return DesignWin{1,3,2,1,4,7,0x90};
}
int main(){
 for(int which=0;which<2;++which)for(int pick: {-1,0,4,7,28,29,31}){
  DesignWin d=reset();chosen=pick;d.select_special(which);
  std::vector<int> expected{29};
  for(int ability=0;ability<29;++ability)if(ability!=(which==0?7:4))expected.push_back(ability);
  assert(options==expected && execs==1 && live==0 && closes==1);
  if(pick<0){assert(d.protoAbilityID1_==4 && d.protoAbilityID2_==7 && d.protoAbilityFlags_==0x90 && repaints.empty());}
  else {
   unsigned selected=pick>=29?~0u:unsigned(pick);
   assert((which==0?d.protoAbilityID1_:d.protoAbilityID2_)==selected);
   unsigned other=which==0?7:4;
   assert((which==0?d.protoAbilityID2_:d.protoAbilityID1_)==other);
   assert(d.protoAbilityFlags_==((selected==~0u?0u:1u<<selected)|(1u<<other)));
   assert((repaints==std::vector<int>{2,1,3,4,5,6}));
  }
 }
 for(int alien=0;alien<2;++alien)for(int kind=0;kind<2;++kind){
  DesignWin d=reset();Players[1].rule_flags=alien?128:0;
  if(kind==0)technology_missing=1;else Chassis[3].missile=1;
  d.select_special(1);assert(options.empty() && !execs && !live && closes==1 && repaints.empty());
  assert(!std::strcmp(warning,kind==0?"TWOSPECIAL":"MISSILESPECIAL"));
 }
 // Slot zero does not require the two-special technology. Filter the first,
 // middle and final bonus scan entries independently of the picked slot.
 for(int bonus_slot: {0,7,40}){
  DesignWin d=reset();technology_missing=1;Ability[8].preq_tech=1;condition_missing=9;
  Players[1].faction_bonus_id[bonus_slot]=15;Players[1].faction_bonus_val1[bonus_slot]=10;
  d.select_special(0);std::vector<int> expected{29};
  for(int ability=0;ability<29;++ability)if(ability!=7 && ability!=8 && ability!=9 && ability!=10)expected.push_back(ability);
  assert(options==expected && execs==1 && !live && closes==1 && !warning);
 }
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("Dialogs &dialogs = popup.dialogs();", "Dialogs dialogs;"),
    ("dialogs.item(StringTemp, ability);", "dialogs.item(StringTemp, static_cast<int>(protoChassisType_));"),
    ("slot != which && selected", "selected"),
    ("if (exec_result < 0)", "if (exec_result < -1)"),
    ("slot < 0x29", "slot < 8"),
    ("draw_special_2(0);", "draw_special_1(0);"),
    ("Dialogs &dialogs = popup.dialogs();", "popup.close(); Dialogs &dialogs = popup.dialogs();")))
def test_actual_picker_owner_selection_and_lifetime_controls(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)


def test_all_designwin_identities_claims_and_seed_history_remain():
    path = ROOT / "src/designwin.cpp"
    before = subprocess.check_output(["git", "show", f"{BASE}:src/designwin.cpp"], cwd=ROOT).decode()
    old = {r.address:r for r in read_text(before,path)}
    new = {r.address:r for r in read_file(path)}
    assert set(old)==set(new)
    for address,row in old.items():
        for field in fields(row):
            if field.name not in {"line","region","origin"}:
                assert getattr(row,field.name)==getattr(new[address],field.name)
        assert [(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in row.origin]==[
            (l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in new[address].origin]
