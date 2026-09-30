"""Bounded actual-source base_yield controls, not full-game acceptance.

Real Base/PlayerData/Citizen/Rules declarations and complete recovery helpers
are compiled with the actual resource_yield dispatcher. Game services are
explicit doubles. Expected arithmetic/allocations are hand-computable original
instruction cases, not a second transcription of the allocation algorithm.
Host function-entry instrumentation observes actual dispatcher calls without
rewriting its body. Native pointer ABI and complete game-state coverage remain
outside these controls.
Slow allocation retains the original positive nutrient-divisor requirement;
zero-divisor traps and malformed packed citizen IDs are not supported inputs.
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
        if not depth:
            return source[start:end+1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    base=(ROOT/"src/base.cpp").read_text()
    headers={name:(ROOT/"src"/name).read_text() for name in
             ("base.h","map.h","faction.h","alpha.h","general.h","technology.h")}
    declarations=[]
    for filename,names in (("base.h",("Base","RulesCitizen","BaseSecretProject")),("map.h",("Map",)),
                           ("faction.h",("SocialCategory","SocialEffect","Goal","PlayerData")),
                           ("alpha.h",("RulesBasic",))):
        for name in names:
            declarations.append(braced(headers[filename],"struct "+name+" {")+";")
    for name in ("RadiusOffsetX","RadiusOffsetY"):
        table,=re.findall(r"const int "+name+r"\[\]\s*=\s*\{[^}]*\};",headers["map.h"])
        declarations.append(table)
    for name in ("BasePopulationUnassigned","BaseNutrientAdequate"):
        definition,=re.findall(r"int "+name+r";",base)
        declarations.append(definition)
    for name in ("BaseCurrentConvoyFrom","BaseCurrentConvoyTo"):
        definition,=re.findall(r"uint32_t "+name+r"\[4\];",base)
        declarations.append(definition)
    for filename,name in (("base.h","FAC_CHILDREN_CRECHE"),("base.h","BSTATE_GOLDEN_AGE_ACTIVE"),("map.h","RSC_NUTRIENTS")):
        definition,=re.findall(name+r"\s*=\s*(?:0x[0-9a-fA-F]+|\d+)",headers[filename])
        declarations.append("enum {"+definition+"};")
    for name in ("MaxSpecialistNum","FacilityRepStart"):
        definition,=re.findall(r"static const int "+name+r"\s*=\s*\d+;",headers["base.h"])
        declarations.append(definition)
    helpers=[]
    for match in re.finditer(r"^(?:static )?(?:int|bool|void)\s*&?\s*base_yield_\w+\(",base,re.M):
        helpers.append(braced(base,match.group()))
    for signature in ("MEASURED inline BOOL __cdecl on_map(","MEASURED inline int __cdecl xrange("):
        helpers.insert(0,braced(headers["map.h"],signature))
    helpers.insert(0,braced(headers["faction.h"],"inline BOOL __cdecl is_human("))
    helpers.insert(0,braced(headers["faction.h"],"inline uint8_t __cdecl factions_status_byte("))
    helpers.append(braced(headers["general.h"],"MEASURED inline int __cdecl range("))
    helpers.append(braced(headers["base.h"],"static __forceinline int has_fac_built_call("))
    helpers.append(braced(base,"void __cdecl base_nutrient()"))
    dispatcher=braced(base,"int __cdecl resource_yield(int rsc_type")
    return "\n".join(declarations),"\n".join(helpers),dispatcher,braced(base,"void __cdecl base_yield()")


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
#define __forceinline inline
typedef int BOOL;
typedef char *LPSTR;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(99);}}
#define require(x) check((x),__LINE__)
ACTUAL_DECLARATIONS
Base Bases[4],*BaseCurrent;
PlayerData PlayersData[8];RulesCitizen Citizen[10];RulesBasic Rules;BaseSecretProject SecretProject;
int BaseIDCurrentSelected,MapLongitudeBounds,MapLatitudeBounds,MapIsFlat,TurnCurrentNum,UnkGlobal0093A934;
uint32_t MapLongitude,LockEnableMask,BaseUpkeepStage,FarmStatus[25],FarmUnavailableMask;
uint32_t BaseCurrentForcesMaintCost;
int TileYieldRestricted;
int BaseCurrentGrowthRate;
Map tiles[2048];
int farm_calls,best_calls,tech_calls,resource_calls,bit_count_calls;
int yield_calls[3],tile_yield[21][3],known[16],best_answer;
int switch_on_crop,switch_on_facility;
int entry_x,entry_y,observe_allocated_type;
uint32_t availability;
std::vector<int> facility_calls;
std::vector<int> counted_values;
bool tracing;
int resource_yield(int,int,int,int,int);
extern "C" void __cyg_profile_func_enter(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_exit(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_enter(void *function,void *){
    if(tracing && function==reinterpret_cast<void *>(resource_yield))++resource_calls;
}
extern "C" void __cyg_profile_func_exit(void *,void *){}
Map *map_tiles(){return tiles;}
void farm_compute(){++farm_calls;FarmUnavailableMask=availability;}
uint32_t best_specialist(){++best_calls;return best_answer;}
BOOL has_tech(int tech,int faction){
    require(faction>=0 && faction<8 && tech>=0 && tech<16);
    if(observe_allocated_type>=0 && tech_calls==70){
        require(BaseCurrent->specialist_unk_1==5);
        for(int slot=0;slot<5;++slot)require(((BaseCurrent->spec[0]>>(slot*4))&15)==uint32_t(observe_allocated_type));
    }
    ++tech_calls;return known[tech];
}
BOOL has_tech_call(int tech,int faction){return has_tech(tech,faction);}
void bitmask(int input,int *offset,int *mask){
    facility_calls.push_back(input);*offset=input/8;*mask=1<<(input%8);
    if(switch_on_facility && input==29){switch_on_facility=0;BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}
}
void bitmask_call(int input,int *offset,int *mask){bitmask(input,offset,mask);}
int bit_count(int value){++bit_count_calls;counted_values.push_back(value);uint32_t bits=value;int count=0;while(bits){count+=bits&1;bits>>=1;}return count;}
int position_at(int x,int y){
    const int offsets[21][2]={{0,0},{1,-1},{2,0},{1,1},{0,2},{-1,1},{-2,0},{-1,-1},{0,-2},
      {2,-2},{2,2},{-2,2},{-2,-2},{1,-3},{3,-1},{3,1},{1,3},{-1,3},{-3,1},{-3,-1},{-1,-3}};
    for(int p=0;p<21;++p){
        int expected_x=entry_x+offsets[p][0];
        if(!(MapIsFlat&1))expected_x=(expected_x+MapLongitudeBounds)%MapLongitudeBounds;
        if(x==expected_x && y==entry_y+offsets[p][1])return p;
    }
    require(false);return -1;
}
int terrain_yield(int type,int faction,int base,int x,int y,int flags){
    require(faction==BaseCurrent->faction_id_current && base==BaseIDCurrentSelected && flags==0);
    ++yield_calls[type];TileYieldRestricted=77;
    if(switch_on_crop && type==0){switch_on_crop=0;BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}
    return tile_yield[position_at(x,y)][type];
}
int crop_yield(int f,int b,int x,int y,int flags){return terrain_yield(0,f,b,x,y,flags);}
int mine_yield(int f,int b,int x,int y,int flags){return terrain_yield(1,f,b,x,y,flags);}
int energy_yield(int f,int b,int x,int y,int flags){return terrain_yield(2,f,b,x,y,flags);}
ACTUAL_DISPATCHER
ACTUAL_HELPERS
ACTUAL_BODY
void reset(){
    std::memset(Bases,0,sizeof(Bases));std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Citizen,0,sizeof(Citizen));std::memset(&Rules,0,sizeof(Rules));std::memset(tiles,0,sizeof(tiles));
    std::memset(&SecretProject,0xff,sizeof(SecretProject));
    std::memset(tile_yield,0,sizeof(tile_yield));std::memset(yield_calls,0,sizeof(yield_calls));
    std::memset(BaseCurrentConvoyFrom,0,sizeof(BaseCurrentConvoyFrom));std::memset(BaseCurrentConvoyTo,0,sizeof(BaseCurrentConvoyTo));
    BaseCurrent=&Bases[0];BaseIDCurrentSelected=0;BaseCurrent->x=20;BaseCurrent->y=12;
    entry_x=20;entry_y=12;observe_allocated_type=-1;
    BaseCurrent->faction_id_current=3;BaseCurrent->population_size=1;
    BaseCurrent->specialist_total=1;BaseCurrent->resource_sqr_radius=1;
    BaseCurrent->state=0x8100;BaseCurrent->eco_damage=555;
    BaseCurrent->spec[0]=0x12345678;BaseCurrent->spec[1]=0x76543210;
    MapLongitudeBounds=64;MapLatitudeBounds=32;MapLongitude=32;MapIsFlat=0;
    LockEnableMask=8;TurnCurrentNum=10;BaseUpkeepStage=0;Rules.nutrient_req_citizen=2;
    UnkGlobal0093A934=0;BaseCurrentForcesMaintCost=0;
    Rules.min_base_size_specialists=5;
    for(auto &tile:tiles)tile.climate=0x60;
    for(int i=0;i<10;++i){Citizen[i].preq_tech=i;Citizen[i].obsol_tech=15;}
    for(int i=0;i<16;++i)known[i]=i!=15;
    farm_calls=best_calls=tech_calls=resource_calls=bit_count_calls=0;facility_calls.clear();
    counted_values.clear();
    BasePopulationUnassigned=-55;BaseNutrientAdequate=73;TileYieldRestricted=88;
    availability=0;best_answer=0;tracing=true;
    switch_on_crop=switch_on_facility=0;
}
void fast_path(){
    reset();BaseCurrent->state=0x8000;PlayersData[3].satellites_[0]=3;
    PlayersData[3].satellites_[1]=4;PlayersData[3].satellites_[2]=5;
    tile_yield[0][0]=2;tile_yield[0][1]=3;tile_yield[0][2]=4;
    base_yield();
    require(farm_calls==1 && resource_calls==4 && best_calls==0 && tech_calls==0);
    require(yield_calls[0]==1 && yield_calls[1]==1 && yield_calls[2]==1 && TileYieldRestricted==0);
    require(BaseCurrent->nutrient_intake_1==3 && BaseCurrent->mineral_intake_1==4 && BaseCurrent->energy_intake_1==5 && BaseCurrent->unused_intake_1==0);
    require(BaseCurrent->nutrient_intake_2==3 && BaseCurrent->mineral_intake_2==4 && BaseCurrent->energy_intake_2==5 && BaseCurrent->unused_intake_2==0);
    require(BasePopulationUnassigned==1 && BaseNutrientAdequate==73);
    require(BaseCurrent->resource_sqr_radius==1 && BaseCurrent->specialist_total==1 && BaseCurrent->eco_damage==0);
    require(BaseCurrent->spec[0]==0x12345678 && BaseCurrent->spec[1]==0x76543210);
    require((BaseCurrent->state&0x8100)==0 && bit_count_calls==22);
    reset();availability=1;base_yield();require(counted_values[0]==1);
}
void satellites(){
    for(int population:{1,3,7})for(int full:{0,1,2,3,4}){
        reset();BaseCurrent->state=0;BaseCurrent->population_size=population;BaseCurrent->specialist_total=population;
        PlayersData[3].satellites_[0]=3;PlayersData[3].satellites_[1]=4;PlayersData[3].satellites_[2]=5;
        if(full==1)Bases[0].facilities_built[29/8]|=1<<(29%8);
        if(full>=2){Bases[1].faction_id_current=full==4?5:3;if(full==3)SecretProject.cloudbase_academy=1;else SecretProject.space_elevator=1;}
        base_yield();
        bool doubled=full>=1 && full<=3;
        require(BaseCurrent->nutrient_intake_2==std::min(population,doubled?3:2));
        require(BaseCurrent->mineral_intake_2==std::min(population,doubled?4:2));
        require(BaseCurrent->energy_intake_2==std::min(population,doubled?5:3));
        require(BasePopulationUnassigned==population && BaseNutrientAdequate==73 && resource_calls==4);
    }
}
void forced_and_scoring(){
    for(uint32_t governor:{0u,0x40u,0x80000000u,0x80000040u}){
        reset();UnkGlobal0093A934=1;BaseCurrent->governor=governor;availability=0x1ffffe;
        BaseCurrent->spec[0]=BaseCurrent->spec[1]=0;Citizen[0].psych_bonus=1;
        base_yield();require(BaseNutrientAdequate==(governor==0x80000040u?0:73));
        require(BaseCurrent->resource_sqr_radius==1 && BaseCurrent->specialist_total==1);
    }
    for(int block:{0,1,2}){
        reset();BaseCurrent->specialist_total=0;BaseCurrent->spec[0]=BaseCurrent->spec[1]=0;
        availability=0x1ffff8u | (block==1?2u:block==2?4u:0u);
        tile_yield[0][0]=10;tile_yield[1][1]=1;tile_yield[2][2]=1;
        base_yield();
        // Original neutral scores: mineral1=768, energy1=128. No second
        // worker remains; center's nutrient10 is already adequate.
        require(BaseCurrent->resource_sqr_radius==(block==1?5:3));
        require(BaseCurrent->nutrient_intake_2==10 && BaseCurrent->specialist_total==0);
        require(BaseCurrent->mineral_intake_2==(block==1?0:1) && BaseCurrent->energy_intake_2==(block==1?1:0));
        require(BasePopulationUnassigned==0 && BaseNutrientAdequate==1 && resource_calls==8);
    }
    reset();BaseCurrent->specialist_total=0;availability=0x1ffff8;
    tile_yield[0][0]=10;tile_yield[1][1]=tile_yield[2][1]=1;base_yield();
    require(BaseCurrent->resource_sqr_radius==3); // strict tie keeps first tile
}
void fallback_specialists(){
    for(int population:{3,5,17}){
        reset();BaseCurrent->population_size=population;BaseCurrent->specialist_total=0;
        BaseCurrent->spec[0]=BaseCurrent->spec[1]=0;availability=0x1ffffe;
        Citizen[0].psych_bonus=1;Citizen[1].ops_bonus=population==5?5:20;Citizen[2].psych_bonus=2;
        Citizen[2].obsol_tech=4; // known obsolescence is ignored in final fallback
        base_yield();int type=population==17?1:2;
        require(BaseCurrent->specialist_total==population && BasePopulationUnassigned==population);
        require(BaseCurrent->resource_sqr_radius==1 && BaseNutrientAdequate==0);
        require(tech_calls==7*population && best_calls==0);
        for(int slot=0;slot<std::min(population,16);++slot)
            require(((BaseCurrent->spec[slot/8]>>((slot%8)*4))&15)==uint32_t(type));
        if(population<16)require(BaseCurrent->spec[0]==uint32_t(population==3?0x222:0x22222) && BaseCurrent->spec[1]==0);
        else require(BaseCurrent->spec[0]==0x11111111 && BaseCurrent->spec[1]==0x11111111);
    }
    // Direct genuine packed helper: first16 use nibbles; slot16 calls the
    // existing selector and refreshes BaseCurrent. No encoding is fabricated.
    reset();Base *base=BaseCurrent;best_answer=6;BaseCurrent=&Bases[1];
    require(base_yield_specialist(base,0)==8 && best_calls==0 && base==&Bases[0]);
    require(base_yield_specialist(base,15)==7 && best_calls==0);
    require(base_yield_specialist(base,16)==6 && best_calls==1 && base==&Bases[1]);
}
void ecology(){
    for(int facilities:{0,1,2,3}){
        reset();BaseCurrent->state=0;
        const uint32_t improvements[6]={4|0x8000|0x80000,4|0x8000|0x80000,0x200000,0x1000000,0x800000,0x400000};
        const int dx[6]={0,1,2,1,0,-1},dy[6]={0,-1,0,1,2,1};
        for(int p=0;p<6;++p)tiles[(12+dy[p])*32+(20+dx[p])/2].bit=improvements[p];
        if(facilities&1)Bases[0].facilities_built[13/8]|=1<<(13%8);
        if(facilities&2)Bases[0].facilities_built[14/8]|=1<<(14%8);
        base_yield();require(BaseCurrent->eco_damage==(facilities==0?29:facilities==3?0:13));
        require(bit_count_calls==22);
    }
    reset();tiles[12*32+10].bit=0x200000;base_yield();require(BaseCurrent->eco_damage==-1);
    reset();tiles[12*32+10].climate=0x40;base_yield();require((BaseCurrent->state&0x8000)!=0);
}
void specialist_candidates_and_compaction(){
    for(int winner=0;winner<7;++winner)for(bool obsolete:{false,true}){
        reset();LockEnableMask=0;UnkGlobal0093A934=1;TurnCurrentNum=101;
        BaseCurrent->population_size=5;BaseCurrent->specialist_total=0;BaseCurrent->spec[0]=BaseCurrent->spec[1]=0;
        availability=0x1ffffe;tile_yield[0][0]=20;Citizen[winner].ops_bonus=1;
        if(obsolete)Citizen[winner].obsol_tech=4;
        else observe_allocated_type=winner;
        base_yield();
        require(BaseCurrent->resource_sqr_radius==1 && BaseCurrent->specialist_total==5 && BasePopulationUnassigned==5);
        require(BaseCurrent->specialist_unk_1==(obsolete?0:5));
        // Obsolescence excludes allocation candidates, not final fallback.
        for(int slot=0;slot<5;++slot)require(((BaseCurrent->spec[0]>>(slot*4))&15)==uint32_t(winner));
        require(resource_calls==4 && tech_calls==(obsolete?49:105));
    }
    reset();LockEnableMask=0;BaseCurrent->population_size=3;BaseCurrent->specialist_total=3;
    BaseCurrent->resource_sqr_radius=3;BaseCurrent->spec[0]=0x3210;BaseCurrent->spec[1]=0;
    Citizen[2].psych_bonus=Citizen[3].psych_bonus=1;availability=0x1ffffc;tile_yield[0][0]=20;
    base_yield();
    // Original shifts old endpoint slot3 into slot2, then advances its outer
    // index: replacement nonpsych type1 in slot0 is deliberately not removed.
    require(BaseCurrent->spec[0]==0x3321 && BaseCurrent->specialist_total==2 && BaseCurrent->specialist_unk_1==1);
    require(BaseCurrent->resource_sqr_radius==3 && BasePopulationUnassigned==2 && tech_calls==7);
    reset();LockEnableMask=0;UnkGlobal0093A934=1;BaseCurrent->population_size=2;
    BaseCurrent->specialist_total=2;BaseCurrent->spec[0]=0x200;BaseCurrent->spec[1]=0;
    Citizen[2].psych_bonus=1;availability=0x1ffffe;base_yield();
    require(BaseCurrent->spec[0]==0x220 && BaseCurrent->specialist_total==2);
    require(BaseCurrent->specialist_unk_1==0 && BasePopulationUnassigned==2 && tech_calls==0);
}
void replacements(){
    for(int food:{4,1,2}){
        bool rollback=food<4;int minerals=food==1?20:100;
        reset();BaseCurrent->specialist_total=0;
        BaseCurrent->spec[0]=BaseCurrent->spec[1]=0;availability=0x1ffff8;
        tile_yield[1][0]=food;tile_yield[2][1]=minerals;
        base_yield();
        require(BaseCurrent->resource_sqr_radius==(rollback?5:3));
        require(BaseCurrent->nutrient_intake_2==(rollback?0:4) && BaseCurrent->mineral_intake_2==(rollback?minerals:0));
        require(BaseCurrent->specialist_total==0 && BasePopulationUnassigned==0);
        // Initial two tiles, removeold/addnew, optional remove-new/add-old.
        // Exactly one replacement attempt; no second scoring pass.
        require(resource_calls==(rollback?24:16));
        require(yield_calls[0]==(rollback?10:8) && yield_calls[1]==(rollback?10:8) && yield_calls[2]==(rollback?10:8));
    }
}
void callback_reloads(){
    reset();BaseCurrent->state=0;PlayersData[3].satellites_[0]=3;
    PlayersData[3].satellites_[1]=4;PlayersData[3].satellites_[2]=5;
    tile_yield[0][0]=2;tile_yield[0][1]=3;tile_yield[0][2]=4;
    Bases[1]=Bases[0];Bases[1].faction_id_current=4;
    Bases[1].nutrient_intake_2=200;Bases[1].mineral_intake_2=20;Bases[1].energy_intake_2=30;Bases[1].unused_intake_2=40;
    switch_on_crop=1;base_yield();
    require(BaseCurrent==&Bases[1] && BaseIDCurrentSelected==1 && resource_calls==4);
    require(Bases[0].nutrient_intake_2==1 && Bases[0].mineral_intake_2==1 && Bases[0].energy_intake_2==1);
    require(Bases[1].nutrient_intake_2==202 && Bases[1].mineral_intake_2==23 && Bases[1].energy_intake_2==34 && Bases[1].unused_intake_2==40);
    require(Bases[1].nutrient_intake_1==202 && Bases[1].unused_intake_1==40 && TileYieldRestricted==0);
    // A second call must reinitialize intake from satellite state rather than
    // accumulating the first call's published totals. Its faction is now4.
    resource_calls=0;base_yield();require(resource_calls==4);
    require(Bases[1].nutrient_intake_2==2 && Bases[1].mineral_intake_2==3 && Bases[1].energy_intake_2==4 && Bases[1].unused_intake_2==0);
    reset();BaseCurrent->state=0;PlayersData[3].satellites_[0]=3;PlayersData[3].satellites_[1]=4;PlayersData[3].satellites_[2]=5;
    Bases[1]=Bases[0];Bases[1].faction_id_current=4;Bases[1].population_size=7;Bases[1].nutrient_intake_2=5;
    Bases[0].facilities_built[29/8]|=1<<(29%8); // pre-callback selected base has facility
    switch_on_facility=1;base_yield();
    require(BaseCurrent==&Bases[1] && Bases[0].nutrient_intake_2==3);
    require(Bases[1].nutrient_intake_2==5); // old selected facility29 prevents halving new intake5
    require(Bases[1].mineral_intake_2==2 && Bases[1].energy_intake_2==3);
}
void convoy_consumer(){
    reset();tile_yield[0][0]=3;BaseCurrentConvoyFrom[0]=1;BaseCurrentConvoyTo[0]=2;
    base_yield();require(BaseCurrent->nutrient_intake_2==3); // yield does not consume convoys
    BaseCurrent->nutrients_accumulated=-4;BaseCurrent->state|=BSTATE_GOLDEN_AGE_ACTIVE;
    Bases[0].facilities_built[FAC_CHILDREN_CRECHE/8]|=1<<(FAC_CHILDREN_CRECHE%8);
    PlayersData[3].soc_effect_pending.growth=1;BaseUpkeepStage=1;
    base_nutrient();require(BaseCurrentGrowthRate==5 && BaseCurrent->nutrient_intake_2==5);
    require(BaseCurrent->nutrient_consumption==3 && BaseCurrent->nutrient_surplus==2 && BaseCurrent->nutrients_accumulated==0);
    require(PlayersData[3].nutrient_surplus_total==2 && BaseCurrentConvoyFrom[0]==1 && BaseCurrentConvoyTo[0]==2);
    BaseCurrentConvoyFrom[0]=20;base_nutrient();
    require(BaseCurrent->nutrient_intake_2==7 && BaseCurrent->nutrient_surplus==-15 && BaseCurrent->nutrients_accumulated==-1);
    require(PlayersData[3].nutrient_surplus_total==2);
}
void signed_arithmetic(){
    require(base_yield_add(0x7fffffff,1)==(-2147483647-1));
    require(base_yield_sub(-2147483647-1,1)==2147483647);
    require(base_yield_mul(0x40000001,4)==4 && base_yield_shift(-1,6)==-64);
    require(base_yield_denominator(-3)==1 && base_yield_denominator(100)==99);
}
void bounds_and_wrapping(){
    reset();entry_x=BaseCurrent->x=0;entry_y=BaseCurrent->y=0;
    BaseCurrent->specialist_total=0;BaseCurrent->resource_sqr_radius=3;
    tile_yield[0][0]=2;base_yield();
    require(BaseCurrent->resource_sqr_radius==3 && BasePopulationUnassigned==0 && BaseCurrent->specialist_total==0);
    require(resource_calls==4 && BaseCurrent->nutrient_intake_2==2 && BaseNutrientAdequate==73);
    for(bool flat:{false,true}){
        reset();entry_x=BaseCurrent->x=62;MapIsFlat=flat;
        BaseCurrent->specialist_total=0;BaseCurrent->resource_sqr_radius=5;
        tile_yield[0][0]=2;tile_yield[2][0]=5;base_yield();
        require(BaseCurrent->resource_sqr_radius==5 && BasePopulationUnassigned==0);
        require(resource_calls==(flat?4:8) && BaseCurrent->nutrient_intake_2==(flat?2:7));
        require(BaseNutrientAdequate==73);
    }
}
int main(){
    require(sizeof(BaseCurrentConvoyFrom)==16 && sizeof(BaseCurrentConvoyTo)==16);
    for(int i=0;i<4;++i)require(BaseCurrentConvoyFrom[i]==0 && BaseCurrentConvoyTo[i]==0);
    fast_path();satellites();forced_and_scoring();fallback_specialists();ecology();
    specialist_candidates_and_compaction();replacements();callback_reloads();convoy_consumer();signed_arithmetic();bounds_and_wrapping();
}
'''


def run_host(tmp_path,declarations,helpers,dispatcher,body):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler required")
    source=tmp_path/"yield.cpp"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS",declarations).replace("ACTUAL_HELPERS",helpers)
                      .replace("ACTUAL_DISPATCHER",dispatcher).replace("ACTUAL_BODY",body))
    binary=tmp_path/"yield"
    result=subprocess.run([compiler,"-std=c++11","-O1","-finstrument-functions","-fsanitize=address,undefined",
                           "-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert result.returncode==0,result.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_base_yield(tmp_path):
    result=run_host(tmp_path,*actual_source())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("part,before,after", [
    (3,"farm_compute();","BaseNutrientAdequate=0; farm_compute();"),
    (3,"(base->governor & 0x80000040u) == 0x80000040u","(base->governor & 0x80000000u) != 0"),
    (3,"base_yield_add(intake, 1) / 2","intake / 2"),
    (3,"if (selected->population_size < intake)","if (false)"),
    (3,"SecretProject.cloudbase_academy >= 0","false"),
    (3,"Bases[SecretProject.space_elevator].faction_id_current == faction","true"),
    (3,"if (!automatic || UnkGlobal0093A934 == 0)","if (true)"),
    (3,"BaseCurrent->resource_sqr_radius &= ~FarmUnavailableMask | 1u;",
       "BaseCurrent->resource_sqr_radius &= ~FarmUnavailableMask;"),
    (3,"shift < base->specialist_total + base->specialist_unk_1;",
       "shift + 1 < base->specialist_total + base->specialist_unk_1;"),
    (3,"if (base->specialist_unk_1 != 0)","if (false)"),
    (3,"if (score > best_score)","if (score >= best_score)"),
    (3,"best_position = -specialist - 1;","best_position = -1;"),
    (3,"!has_tech(Citizen[specialist].obsol_tech, faction)","true"),
    (3,"specialist < 7","specialist < 6"),
    (3,"base_yield_mul(Citizen[specialist].psych_bonus, 3)","Citizen[specialist].psych_bonus"),
    (3,"base->population_size >= static_cast<int>(Rules.min_base_size_specialists)","false"),
    (3,"if (base->nutrient_intake_2 < base_yield_mul(base->population_size + 1,",
       "if (base->nutrient_intake_2 < base_yield_mul(base->population_size,"),
    (3,"worked = (worked & ~new_bit) | old_bit;","/* lost restored allocation */"),
    (3,"base_yield_apply_tile(base, map_width, x, y, true);",
       "base_yield_apply_tile(base, map_width, x, y, false);"),
    (3,"base_yield_intake(*selected, resource, false) = base_yield_intake(*selected, resource, true);",
       "base_yield_intake(*selected, resource, false) = 0;"),
    (3,"if (used) damage += damage;","if (!used) damage += damage;"),
    (3,"if ((improvements & 0x200000) != 0) --damage;",
       "if ((improvements & 0x200000) != 0) ++damage;"),
    (3,"damage /= 2;","damage = static_cast<uint32_t>(damage) / 2;"),
    (3,"BaseCurrent->eco_damage = base_yield_add(BaseCurrent->eco_damage, damage);",
       "BaseCurrent->eco_damage = damage;"),
    (1,"resource < 4","resource < 3"),
    (1,"if (index < 16)","if (index < 15)"),
    (1,"const int selected = BaseIDCurrentSelected;",
       "const int &selected = BaseIDCurrentSelected;"),
    (1,"base = BaseCurrent;","/* cached stale base pointer */"),
    (1,"base.nutrient_intake_2","base.mineral_intake_2"),
    (1,"static_cast<uint32_t>(left) + static_cast<uint32_t>(right)","static_cast<uint32_t>(left + right)"),
    (2,"TileYieldRestricted = 0;","/* omitted type3 side effect */"),
    (1,"BaseCurrentConvoyTo[RSC_NUTRIENTS]","BaseCurrentConvoyFrom[RSC_NUTRIENTS]"),
    (1,"BaseCurrentConvoyFrom[RSC_NUTRIENTS]","0"),
])
def test_base_yield_damage_is_observed(tmp_path,part,before,after):
    parts=list(actual_source())
    assert before in parts[part]
    parts[part]=parts[part].replace(before,after)
    result=run_host(tmp_path,*parts)
    assert result.returncode!=0
    assert result.returncode==99 or "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr,result.stderr
