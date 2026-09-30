"""Actual five-stage psych calculation with bounded original-derived controls.

Uses complete recovered helpers/body and real game field declarations. Facility,
vehicle lookup and ability APIs are explicit doubles; they do not establish
native layout or real game services. Function-entry instrumentation observes
the actual publication helper after it runs, without substituting stage logic.
Original valid-index/divisor domain only: malformed stacks and signed-division
traps are not repaired or certified. No host test proves original x87/ABI/game
integration or exhaustively covers overflowing full-calculation inputs.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT=Path(__file__).resolve().parents[2]


def braced(source,signature):
    start=source.index(signature)
    opening=source.index("{",start)
    depth=0
    for end in range(opening,len(source)):
        depth+=(source[end]=="{")-(source[end]=="}")
        if not depth:return source[start:end+1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    source=(ROOT/"src/base.cpp").read_text()
    headers={name:(ROOT/"src"/name).read_text() for name in
             ("base.h","map.h","faction.h","alpha.h","general.h","veh.h","game.h")}
    declarations=[]
    for filename,names in (("base.h",("Base","BaseSecretProject")),
                           ("faction.h",("SocialCategory","SocialEffect","Goal","Player","PlayerData")),
                           ("alpha.h",("RulesBasic",)),
                           ("veh.h",("Veh","VehPrototype","RulesWeapon","RulesChassis"))):
        for name in names:declarations.append(braced(headers[filename],"struct "+name+" {")+";")
    for filename,names in (("base.h",("FacilityId",)),("faction.h",("FactionSpecialRules","PlayerRuleFlagBitfield")),
                           ("veh.h",("VehAbilitiesBitfield","VehTriad")),("game.h",("GameDifficultyLevel",))):
        for name in names:declarations.append(braced(headers[filename],"enum "+name+" {")+";")
    for name in ("BasePsychTalents","BasePsychDrones","BasePsychSuperDrones"):
        definition,=re.findall(r"int "+name+r"\[5\];",source)
        declarations.append(definition)
    for filename,names in (("base.h",("BaseUpkeepStage","BaseCurrentVehPacifismCount")),
                           ("map.h",("MapAreaSqRoot",)),("faction.h",("LockEnableMask",))):
        for name in names:
            declaration,=re.findall(r"extern uint32_t "+name+r";",headers[filename])
            declarations.append(declaration)
    for filename,names in (("base.h",("has_fac",)),("veh.h",("veh_at","stack_fix","has_abil"))):
        for name in names:
            declaration,=re.findall(r"^(?:BOOL|int) __cdecl "+name+r"\([^;]+\);",headers[filename],re.M)
            declarations.append(declaration)
    declarations.append(re.search(r"MEASURED inline void __cdecl bitmask\([^)]*\)",headers["general.h"]).group().replace("MEASURED inline ","")+";")
    helpers=[]
    for name in ("signed_bits","add","sub","mul","has_facility"):
        signature=re.search(r"^(?:int|bool) base_yield_"+name+r"\(",source,re.M)
        assert signature is not None
        helpers.append(braced(source,signature.group()))
    for match in re.finditer(r"^(?:static )?(?:int|bool|void)\s*&?\s*base_psych_\w+\(",source,re.M):
        helpers.append(braced(source,match.group()))
    helpers.insert(0,braced(headers["general.h"],"MEASURED inline int __cdecl range("))
    return "\n".join(declarations),"\n".join(helpers),braced(source,"void __cdecl base_psych()")


HARNESS=r'''
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>
#define __cdecl
#define MEASURED
typedef int BOOL;typedef char *LPSTR;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(101);}}
#define require(x) check((x),__LINE__)
ACTUAL_DECLARATIONS
Base Bases[8],*BaseCurrent;
Player Players[8];PlayerData PlayersData[8];RulesBasic Rules;BaseSecretProject SecretProject;
Veh Vehs[64];VehPrototype VehPrototypes[512];RulesWeapon Weapon[26];RulesChassis Chassis[9];
int BaseIDCurrentSelected;
uint32_t LockEnableMask,MapAreaSqRoot,BaseUpkeepStage,BaseCurrentVehPacifismCount;
bool facilities[128],abilities[512];
int raw_vehicle,fixed_vehicle,veh_calls,fix_calls,ability_calls;
std::vector<int> facility_calls;
std::vector<int> ability_answers;
int ability_position,change_proto,change_selected,change_base;
bool tracing;
int published[400][3],publish_count;
void base_psych_publish(int);
extern "C" void __cyg_profile_func_enter(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_exit(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_enter(void *,void *){}
extern "C" void __cyg_profile_func_exit(void *function,void *){
    if(tracing && function==reinterpret_cast<void *>(base_psych_publish) && publish_count<400){
        int stage=publish_count%5;
        published[publish_count][0]=BasePsychTalents[stage];
        published[publish_count][1]=BasePsychDrones[stage];
        published[publish_count][2]=BasePsychSuperDrones[stage];++publish_count;
    }
}
BOOL has_fac(int facility,int selected,int queue){
    require(facility>=0 && facility<128 && selected==BaseIDCurrentSelected && queue==0);
    facility_calls.push_back(facility);return facilities[facility];
}
void bitmask(int facility,int *offset,int *mask){
    require(facility>=0 && facility<96);facility_calls.push_back(facility);*offset=facility/8;*mask=1<<(facility%8);
    if(change_selected>=0){BaseIDCurrentSelected=change_selected;change_selected=-1;}
}
int veh_at(int x,int y){require(x==BaseCurrent->x && y==BaseCurrent->y);++veh_calls;return raw_vehicle;}
int stack_fix(int id){require(id==raw_vehicle);++fix_calls;if(change_base>=0){BaseCurrent=&Bases[change_base];change_base=-1;}return fixed_vehicle;}
BOOL has_abil(int proto,int ability){
    require(proto>=0 && proto<512 && ability==ABL_POLICE_2X);++ability_calls;
    if(change_proto>=0){Vehs[0].proto_id=change_proto;change_proto=-1;}
    if(ability_position<int(ability_answers.size()))return ability_answers[ability_position++];
    return abilities[proto];
}
ACTUAL_HELPERS
ACTUAL_BODY
void reset(){
    tracing=false;std::memset(Bases,0,sizeof(Bases));std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(&Rules,0,sizeof(Rules));std::memset(&SecretProject,0xff,sizeof(SecretProject));
    std::memset(Vehs,0,sizeof(Vehs));std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Weapon,0,sizeof(Weapon));std::memset(Chassis,0,sizeof(Chassis));
    std::memset(facilities,0,sizeof(facilities));std::memset(abilities,0,sizeof(abilities));
    BaseCurrent=&Bases[0];BaseIDCurrentSelected=0;BaseCurrent->x=20;BaseCurrent->y=12;BaseCurrent->population_size=5;BaseCurrent->faction_id_current=3;
    PlayersData[3].diff_level=3;PlayersData[3].current_num_bases=1;MapAreaSqRoot=0;LockEnableMask=8;
    PlayersData[3].soc_effect_pending.police=-2;
    BaseUpkeepStage=0;BaseCurrentVehPacifismCount=0;
    raw_vehicle=fixed_vehicle=-1;veh_calls=fix_calls=ability_calls=0;facility_calls.clear();
    ability_answers.clear();ability_position=0;change_proto=change_selected=change_base=-1;
    for(auto &v:Vehs){v.prev_veh_id_stack=v.next_veh_id_stack=-1;v.faction_id=3;}
    for(auto &player:PlayersData)for(auto &row:player.unk_39)for(auto &value:row)value=-777;
    for(int i=0;i<5;++i)BasePsychTalents[i]=BasePsychDrones[i]=BasePsychSuperDrones[i]=-555;
    publish_count=0;std::memset(published,0,sizeof(published));tracing=true;
}
void expect_stage(int stage,int t,int d,int s){
    require(BasePsychTalents[stage]==t && BasePsychDrones[stage]==d && BasePsychSuperDrones[stage]==s);
}
void baseline(){
    reset();base_psych();require(publish_count==5);
    for(int stage=0;stage<5;++stage)expect_stage(stage,0,2,0);
    require(BaseCurrent->talent_total==0 && BaseCurrent->drone_total==2 && BaseCurrent->super_drone_total==0);
    require(facility_calls==std::vector<int>({22,6,8,11,12,17,18,23,35}));
    require(veh_calls==0 && ability_calls==0);
}
void stage_controls(){
    reset();BaseCurrent->psych_total=10;base_psych();expect_stage(0,0,2,0);
    for(int s=1;s<5;++s)expect_stage(s,4,1,0);
    reset();Players[3].faction_bonus_count=2;Players[3].faction_bonus_id[0]=Players[3].faction_bonus_id[1]=RULE_NODRONE;
    Players[3].faction_bonus_val1[0]=10;Players[3].faction_bonus_val1[1]=-100;PlayersData[3].soc_effect_pending.talent=-2;
    base_psych();for(int s=0;s<5;++s)expect_stage(s,0,2,0);
    reset();Players[3].rule_talent=1;PlayersData[3].soc_effect_pending.talent=4;
    base_psych();for(int s=0;s<5;++s)expect_stage(s,5,0,0);
    reset();MapAreaSqRoot=28;PlayersData[3].current_num_bases=10;PlayersData[3].soc_effect_pending.talent=-5;
    base_psych();for(int s=0;s<5;++s)expect_stage(s,0,5,1);
    BaseCurrent->nerve_staple_turns_left=1;publish_count=0;base_psych();expect_stage(4,0,0,1);
    reset();BaseCurrent->specialist_total=2;BaseCurrent->psych_total=10;
    base_psych();expect_stage(0,0,2,0);for(int s=1;s<5;++s)expect_stage(s,3,0,0);
}
void matrix_controls(){
    // Original 4EB44E..4EB45E stores producer case-1, not social_ai's shifted row.
    const int scores[8]={-20,-18,-16,-14,-12,0,0,1};
    const int talents[8]={0,0,0,0,1,2,3,3},drones[8]={5,4,3,2,2,2,2,1};
    reset();BaseUpkeepStage=1;base_psych();require(publish_count==365);
    require(std::count(facility_calls.begin(),facility_calls.end(),35)==9);
    int observed=0;
    for(int row=7;row>=0;--row)for(int col=8;col>=0;--col){
        require(PlayersData[3].unk_39[row][col]==-777+scores[row]);
        for(int stage=0;stage<5;++stage,++observed)
            require(published[observed][0]==talents[row] && published[observed][1]==drones[row] && published[observed][2]==0);
    }
    for(int stage=0;stage<5;++stage)expect_stage(stage,0,2,0);
    publish_count=0;base_psych();require(publish_count==365);
    for(int r=0;r<8;++r)for(int c=0;c<9;++c){
        require(PlayersData[3].unk_39[r][c]==-777+2*scores[r]);
        require(PlayersData[2].unk_39[r][c]==-777);
    }
    // Independent nonuniform column oracle: talent +4 gives stage2 (3,1,0).
    // Police -5/-4/-3 then produces (1,3,0)/(2,3,0)/(2,2,0).
    const int police_scores[9]={-14,-12,0,1,1,1,1,1,1};
    reset();BaseUpkeepStage=1;BaseCurrentVehPacifismCount=3;base_psych();
    require(publish_count==365);
    for(int c=0;c<9;++c)require(PlayersData[3].unk_39[7][c]==-777+police_scores[c]);
    for(int stage=0;stage<5;++stage)expect_stage(stage,0,2,0);
}
void unit(){raw_vehicle=4;fixed_vehicle=0;Vehs[0].proto_id=1;Vehs[0].faction_id=7;}
void police_controls(){
    for(int p=-3;p>=-5;--p){reset();PlayersData[3].soc_effect_pending.police=p;BaseCurrentVehPacifismCount=3;
        base_psych();expect_stage(2,0,2,0);expect_stage(3,0,p==-3?4:5,0);}
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;base_psych();
    expect_stage(3,0,1,0);require(veh_calls==2 && fix_calls==2 && ability_calls==3);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;abilities[1]=true;base_psych();
    expect_stage(3,0,0,0);require(veh_calls==1 && fix_calls==1 && ability_calls==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;ability_answers={1,0};base_psych();
    expect_stage(3,0,1,0);require(ability_calls==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;Weapon[0].offense_rating=-1;
    base_psych();expect_stage(3,0,0,0);require(veh_calls==1);
    for(int p=0;p<4;++p){reset();unit();PlayersData[3].soc_effect_pending.police=p;
        PlayersData[3].soc_effect_pending.talent=-3;
        for(int id=0;id<4;++id){Vehs[id].proto_id=1;Vehs[id].next_veh_id_stack=id==3?-1:id+1;}
        base_psych();expect_stage(3,0,p==0?4:p==1?3:p==2?2:0,0);}
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;change_base=1;Bases[1]=Bases[0];
    Bases[1].faction_id_current=7;Bases[1].drone_total=4;Bases[1].x=18;
    Players[3].rule_flags=0x40;Weapon[0].offense_rating=-1;
    base_psych();expect_stage(3,0,2,0);require(BaseCurrent==&Bases[1] && Bases[0].drone_total==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;Vehs[0].proto_id=15;
    base_psych();expect_stage(3,0,0,0);require(veh_calls==1);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;
    base_psych();expect_stage(3,0,1,0);require(veh_calls==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;Weapon[0].offense_rating=-1;Chassis[0].triad=1;
    base_psych();expect_stage(3,0,1,0);require(veh_calls==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;Weapon[0].offense_rating=-1;Vehs[0].proto_id=64;
    base_psych();expect_stage(3,0,1,0);require(veh_calls==2);
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;VehPrototypes[1].plan=4;
    base_psych();expect_stage(3,0,2,0);require(ability_calls==0 && veh_calls==2);
    // has_abil changes prototype; the native-trait check must reread the vehicle.
    reset();unit();PlayersData[3].soc_effect_pending.police=-1;Players[3].rule_flags=0x40;
    VehPrototypes[2].weapon_id=1;Weapon[1].offense_rating=-1;change_proto=2;
    base_psych();expect_stage(3,0,0,0);require(veh_calls==1);
}
void built(int id){Bases[0].facilities_built[id/8]|=1u<<(id%8);}
void project_controls(){
    reset();facilities[6]=true;base_psych();expect_stage(1,0,2,0);expect_stage(2,0,0,0);
    reset();facilities[12]=true;base_psych();expect_stage(2,2,2,0);
    reset();built(17);built(18);base_psych();expect_stage(2,0,0,0);
    reset();facilities[22]=true;Rules.drones_genejack_factory=2;base_psych();expect_stage(0,0,2,0);expect_stage(2,0,4,0);
    reset();SecretProject.human_genome_project=0;SecretProject.clinical_immortality=0;
    base_psych();expect_stage(3,0,2,0);expect_stage(4,3,2,0);
    for(int economy=0;economy<3;++economy){reset();SecretProject.longevity_vaccine=0;
        PlayersData[3].soc_category_pending.models_[SocialCategory::ECONOMICS]=economy;base_psych();expect_stage(4,0,economy==1?2:economy==2?0:1,0);}
    reset();built(23);PlayersData[3].soc_effect_pending.talent=1;base_psych();expect_stage(3,1,2,0);expect_stage(4,0,0,0);
    reset();PlayersData[3].soc_effect_pending.police=-1;SecretProject.self_aware_colony=0;
    base_psych();expect_stage(3,0,1,0);require(veh_calls==0);
    reset();built(35);PlayersData[3].soc_effect_pending.police=-3;unit();base_psych();expect_stage(3,0,1,0);
    reset();facilities[8]=true;SecretProject.virtual_world=0;base_psych();expect_stage(2,0,0,0);
    require(std::count(facility_calls.begin(),facility_calls.end(),11)==0);
    reset();facilities[8]=true;SecretProject.virtual_world=1;Bases[1].faction_id_current=7;
    base_psych();expect_stage(2,0,2,0);
    // bitmask may move the selected global; the current lookup keeps its prior ID.
    reset();built(17);change_selected=1;base_psych();expect_stage(2,0,1,0);
}
void initial_controls(){
    reset();LockEnableMask=0;PlayersData[3].diff_level=0;base_psych();expect_stage(0,0,2,0);
    reset();PlayersData[3].diff_level=0;base_psych();expect_stage(0,0,0,0);
    reset();BaseCurrent->assimilation_turns_left=255;base_psych();expect_stage(0,0,3,0);
    reset();BaseCurrent->assimilation_turns_left=1;base_psych();expect_stage(0,0,3,0);
    reset();Players[3].rule_drone=2;base_psych();expect_stage(0,0,4,0);
    reset();Players[3].rule_talent=2;base_psych();expect_stage(0,3,2,0);
    reset();PlayersData[3].soc_effect_pending.police=-3;BaseCurrentVehPacifismCount=0x80000000u;
    base_psych();expect_stage(3,0,2,0);
    reset();PlayersData[3].soc_effect_pending.police=-3;BaseCurrentVehPacifismCount=0xffffffffu;
    base_psych();expect_stage(3,0,2,0);
    reset();PlayersData[3].soc_effect_pending.police=-5;BaseCurrentVehPacifismCount=0x7fffffffu;
    base_psych();expect_stage(3,0,0,0);
    reset();PlayersData[3].soc_effect_pending.police=-5;BaseCurrentVehPacifismCount=0x80000000u;
    base_psych();expect_stage(3,0,2,0);
    reset();BaseCurrent->population_size=3;PlayersData[3].soc_effect_pending.talent=-2;
    SecretProject.planetary_transit_system=0;base_psych();expect_stage(0,0,2,0);expect_stage(2,0,1,0);
    reset();MapAreaSqRoot=28;PlayersData[3].current_num_bases=10;PlayersData[3].soc_effect_pending.talent=-5;
    built(23);base_psych();expect_stage(3,0,5,1);expect_stage(4,0,0,1);
}
int main(){baseline();stage_controls();matrix_controls();police_controls();project_controls();initial_controls();}
'''


def run_host(tmp_path,declarations,helpers,body):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host compiler required")
    source=tmp_path/"psych.cpp"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS",declarations).replace("ACTUAL_HELPERS",helpers).replace("ACTUAL_BODY",body))
    binary=tmp_path/"psych"
    built=subprocess.run([compiler,"-std=c++11","-O1","-finstrument-functions","-fsanitize=address,undefined",
                          "-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert built.returncode==0,built.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_base_psych(tmp_path):
    result=run_host(tmp_path,*actual_source())
    assert result.returncode==0,result.stderr


# Every damage variant must compile and fail an actual body observation. The
# mutations alter only extracted product code, never fixtures or expected data.
MUTANTS=[
    ("talent-not-police", "body", "talent_case - 4 : PlayersData[faction].soc_effect_pending.talent", "talent_case - 4 : PlayersData[faction].soc_effect_pending.police"),
    ("no-drone-immediate", "body", "if (initial_drones < 0) initial_drones = 0;", ""),
    ("no-drone-first", "body", "            break;", "            continue;"),
    ("stage-zero-talent-clamp", "body", "            base_psych_balance();", "            base_psych_clamp_totals();"),
    ("psych-half", "body", "BaseCurrent->psych_total / 2", "BaseCurrent->psych_total / 3"),
    ("commons-not-creche", "body", "has_fac(FAC_RECREATION_COMMONS,", "has_fac(FAC_CHILDREN_CRECHE,"),
    ("hypothetical-row", "body", "talent_case - 4", "talent_case - 3"),
    ("matrix-overwrite", "body", "weight = base_yield_add(weight, score);", "weight = score;"),
    ("matrix-score-scale", "body", "if (base->population_size < 6)", "if (base->population_size < 5)"),
    ("matrix-positive-half", "body", "score /= 2;", "score /= 3;"),
    ("all-hypothetical-columns", "body", "police_case = 9", "police_case = 8"),
    ("hypothetical-police-column", "body", "police = police_case - 6;", "police = 4 - police_case;"),
    ("brood-after-skip", "body", "const bool brood_pit = base_yield_has_facility(FAC_BROOD_PIT);", "const bool brood_pit = talent_case == 0 && base_yield_has_facility(FAC_BROOD_PIT);"),
    ("facility-cached-selection", "helpers", "Bases[selected].facilities_built", "Bases[BaseIDCurrentSelected].facilities_built"),
    ("normalization-capacity", "helpers", "base_yield_sub(base->population_size, base->specialist_total)", "base->population_size"),
    ("normalization-super-exit", "helpers", "base->super_drone_total = base_psych_clamp(base->super_drone_total, base->drone_total);", "base->super_drone_total = 0;"),
    ("publish-stage-talents", "helpers", "BasePsychTalents[stage] = base->talent_total;", "BasePsychTalents[stage] = base->drone_total;"),
    ("publish-stage-super", "helpers", "BasePsychSuperDrones[stage] = base->super_drone_total;", "BasePsychSuperDrones[stage] = 0;"),
    ("police-priority", "body", "enhanced == (pass == 0)", "enhanced == (pass != 0)"),
    ("police-second-pass", "body", "pass < 2 && used < police_limit", "pass < 1 && used < police_limit"),
    ("police-plan", "body", "VehPrototypes[prototype].plan <= 3", "VehPrototypes[prototype].plan <= 4"),
    ("invented-faction-filter", "body", "VehPrototypes[prototype].plan <= 3", "VehPrototypes[prototype].plan <= 3 && Vehs[vehicle].faction_id == faction"),
    ("police-ability-cached", "body", "if (has_abil(Vehs[vehicle].proto_id, 0x400000))", "if (enhanced)"),
    ("police-quota", "body", "if (police_limit > 1) --police_limit;", ""),
    ("police-high-bonus", "body", "if (police >= 3) base_psych_reduce_drone();", "if (police >= 4) base_psych_reduce_drone();"),
    ("native-prototype-bound", "helpers", "if (prototype >= 64)", "if (prototype > 64)"),
    ("native-triad", "helpers", "Chassis[VehPrototypes[prototype].chassis_id].triad == 0", "Chassis[VehPrototypes[prototype].chassis_id].triad != 0"),
    ("native-prototype-fifteen", "helpers", "&& prototype != 15", "&& prototype != 14"),
    ("native-weapon-signed", "helpers", "].offense_rating >= 0", "].offense_rating > 0"),
    ("pacifism-minus-one", "body", "base_yield_sub(away, 1)", "away"),
    ("pacifism-signed", "body", "away > 1 ?", "static_cast<uint32_t>(away) > 1 ?"),
    ("pacifism-double", "body", "penalty = base_yield_add(away, away);", "penalty = away;"),
    ("punishment-super-preservation", "body", "BaseCurrent->talent_total = 0;", "BaseCurrent->talent_total = 0; BaseCurrent->super_drone_total = 0;"),
    ("longevity-economy", "body", "soc_category_pending.models_[SocialCategory::ECONOMICS] != 1", "soc_category_pending.models_[SocialCategory::ECONOMICS] == 1"),
    ("project-faction", "helpers", "Bases[project_base].faction_id_current == faction", "Bases[project_base].faction_id_current != faction"),
    ("ai-difficulty", "body", "human ? PlayersData[faction].diff_level : 3", "PlayersData[faction].diff_level"),
    ("assimilation-ceiling", "body", "assimilation_turns_left + 9", "assimilation_turns_left + 0"),
    ("rule-talent-ceiling", "body", "base_yield_add(Players[faction].rule_talent, population)", "base_yield_add(Players[faction].rule_talent, population - 1)"),
]


@pytest.mark.parametrize("name,part,before,after",MUTANTS,ids=[item[0] for item in MUTANTS])
def test_compiling_psych_damage(tmp_path,name,part,before,after):
    declarations,helpers,body=actual_source()
    original=helpers if part=="helpers" else body
    assert before in original,(name,"mutation anchor absent")
    damaged=original.replace(before,after)
    result=run_host(tmp_path,declarations,damaged if part=="helpers" else helpers,
                    damaged if part=="body" else body)
    assert result.returncode!=0,(name,"behavior damage escaped controls")
