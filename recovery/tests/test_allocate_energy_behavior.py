"""Actual allocator-body controls; host doubles are not target ABI proof."""
from pathlib import Path
import shutil,subprocess
import pytest

SOURCE=Path(__file__).resolve().parents[2] / 'src/faction.cpp'
SIGNATURE='void __cdecl allocate_energy(int faction_id) {'
def body():
    text=SOURCE.read_text();start=text.index(SIGNATURE);opening=text.index('{',start);depth=0
    for end in range(opening,len(text)):
        if text[end]=='{':depth+=1
        elif text[end]=='}':
            depth-=1
            if depth==0:return text[start:end+1]
    raise AssertionError('unterminated allocator')

HARNESS=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
#define nullptr 0
const int SP_TELEPATHIC_MATRIX=30;
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
typedef SocialEffect Effect;
struct PlayerData {int diff_level,pop_total,energy_reserves,se_alloc_psych,se_alloc_labs,ai_tech,ai_power,ai_wealth,tech_id_researching;Effect soc_effect_pending,soc_effect_temp;unsigned char region_total_bases[128],region_base_plan[128];};
struct Player {int rule_techcost;};
struct Base {int faction_id_current,specialist_total,specialist_unk_1,talent_total,drone_total,unk_10;uint32_t state;};
struct Prototype {short preq_tech;};
// Extended host rows permit a byte-width discriminator outside valid game IDs.
PlayerData PlayersData[32];Player Players[32];Base Bases[4];Prototype VehPrototypes[2];Base *BaseCurrent;
struct Console {int faction_id_;} ConsoleGlobal;
struct GameDataStateWindow { struct Fields { union LockMasks { struct Bytes { uint8_t human_mask_,alive_mask_,unassigned_[2]; } bytes_; uint32_t enable_mask_; } lock_masks_; int turn_current_num_,base_current_count_; } fields_; } GameDataStateStorage;
uint32_t &LockEnableMask=GameDataStateStorage.fields_.lock_masks_.enable_mask_;int &TurnCurrentNum=GameDataStateStorage.fields_.turn_current_num_,&BaseCurrentCount=GameDataStateStorage.fields_.base_current_count_;int CurrentFactionEnergyReserves;
const char *ScriptPopupFile="script";
int set_calls,compute_calls,popup_calls,social_calls,cap_calls,first_cap_labs,project_owner,popup_result;
int diplo_add(int a,int b){return static_cast<int>(static_cast<uint32_t>(a)+static_cast<uint32_t>(b));}
int diplo_sub(int a,int b){return static_cast<int>(static_cast<uint32_t>(a)-static_cast<uint32_t>(b));}
int diplo_mul(int a,int b){return static_cast<int>(static_cast<uint32_t>(a)*static_cast<uint32_t>(b));}
int popp(const char*,const char*,int,const char*,void*){++popup_calls;return popup_result;}
void social_select(int){++social_calls;}
int has_tech(int,int){return 1;}
void set_base(int index){++set_calls;BaseCurrent=&Bases[index];}
int base_compute(int){++compute_calls;return 1;}
int has_project(int,int){return project_owner;}
bool has_fac_built(int,int){return false;}
int fac_maint(int,int){return 0;}
int energy_limit(int faction){if(!cap_calls++)first_cap_labs=PlayersData[faction].se_alloc_labs;return 5;}
BODY
void reset(){
 memset(PlayersData,0,sizeof PlayersData);memset(Players,0,sizeof Players);memset(Bases,0,sizeof Bases);
 LockEnableMask=0;TurnCurrentNum=100;BaseCurrentCount=0;CurrentFactionEnergyReserves=30;BaseCurrent=&Bases[1];
 set_calls=compute_calls=popup_calls=social_calls=cap_calls=project_owner=popup_result=0;first_cap_labs=-1;
 ConsoleGlobal.faction_id_=2;Players[2].rule_techcost=5;
 PlayersData[2].diff_level=2;PlayersData[2].pop_total=5;PlayersData[2].se_alloc_labs=2;
 PlayersData[2].ai_tech=2;PlayersData[2].tech_id_researching=-1;
}
int main(){
 // Native526A00 subtracts current psych BEFORE /2, not ai_wealth twice.
 reset();PlayersData[2].soc_effect_pending.values_[SocialEffect::POLICE]=-2;PlayersData[2].se_alloc_psych=1;
 allocate_energy(2);assert(PlayersData[2].se_alloc_psych==2);assert(first_cap_labs==7);
 // First pass intentionally uses existing global current, not the owned row.
 reset();BaseCurrentCount=1;Bases[0].faction_id_current=2;Bases[0].specialist_total=2;
 Bases[1].state=2;PlayersData[2].soc_effect_pending.values_[SocialEffect::POLICE]=-1;
 allocate_energy(2);assert(PlayersData[2].se_alloc_psych==1);assert(set_calls==0&&BaseCurrent==&Bases[1]);
 // Severe police selects/recomputes, then reads the reloaded current owner.
 reset();BaseCurrentCount=1;Bases[0].faction_id_current=2;Bases[0].state=2;Bases[0].drone_total=1;
 PlayersData[2].soc_effect_pending.values_[SocialEffect::POLICE]=-3;
 allocate_energy(2);assert(set_calls==1&&compute_calls==1&&BaseCurrent==&Bases[0]);
 // An emergency refusal does not fall through into allocation.
 reset();LockEnableMask=4;PlayersData[2].energy_reserves=20;PlayersData[2].se_alloc_psych=2;PlayersData[2].se_alloc_labs=4;
 allocate_energy(2);assert(popup_calls==1&&social_calls==0&&cap_calls==0);
 // Accepted emergency opens social selection without normal recomputation.
 reset();LockEnableMask=4;PlayersData[2].energy_reserves=20;PlayersData[2].se_alloc_psych=2;PlayersData[2].se_alloc_labs=4;popup_result=1;
 allocate_energy(2);assert(popup_calls==1&&social_calls==1&&cap_calls==0);
 // Native5267C7 sees byte0, not byte1's alive bit. Synthetic extended host
 // storage avoids an out-of-bounds test; it does not legitimize game ID8.
 reset();LockEnableMask=0x100;Players[8].rule_techcost=5;PlayersData[8].tech_id_researching=-1;
 allocate_energy(8);assert(cap_calls>=2);
}
'''
@pytest.mark.parametrize('compiler',['g++','clang++'])
@pytest.mark.parametrize('damage',['none','psych_formula','human_width'])
def test_actual_allocator(compiler,damage,tmp_path):
    executable=shutil.which(compiler)
    if executable is None:pytest.skip('host compiler unavailable')
    candidate=body()
    if damage=='psych_formula':
        good='diplo_sub(faction.ai_tech, faction.ai_power), faction.se_alloc_psych);'
        assert candidate.count(good)==1
        candidate=candidate.replace(good,'diplo_sub(faction.ai_tech, faction.ai_power), faction.ai_wealth);',1)
    elif damage=='human_width':
        old='GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit'
        assert candidate.count(old)==1
        candidate=candidate.replace(old,'LockEnableMask & faction_bit',1)
        assert candidate != body()
    source=tmp_path/'allocator.cpp';binary=tmp_path/'allocator'
    source.write_text(HARNESS.replace('BODY',candidate))
    built=subprocess.run([executable,'-std=c++98','-Wall','-Wextra','-Werror',str(source),'-o',str(binary)],capture_output=True,text=True,timeout=60)
    assert built.returncode==0,built.stdout+built.stderr
    run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
    if damage!='none':
        failed='first_cap_labs' if damage=='psych_formula' else 'cap_calls'
        assert run.returncode!=0 and failed in run.stderr,run.stdout+run.stderr
    else:assert run.returncode==0,run.stdout+run.stderr
