"""Actual terrain query and two-slot prerequisite controls; bounded host evidence."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import function_line

ROOT = Path(__file__).resolve().parents[2]


def definition(text, name):
    line = function_line(text, name)
    start = sum(len(s) for s in text.splitlines(keepends=True)[:line-1])
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end+1]
    raise AssertionError(name)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'swap_domains', 'disabled', 'terraform_ban'])
def test_actual_terrain_prerequisite_domains(tmp_path, compiler_name, damage):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name+' unavailable')
    source=(ROOT/'src/technology_queries.cpp').read_text()
    terrain=definition(source,'terrain_avail')
    assert 'prerequisite_tech[is_sea]' in terrain
    mutations={
        'swap_domains': ('prerequisite_tech[is_sea]','prerequisite_tech[1-is_sea]'),
        'disabled': ('preq_tech < TechNone','preq_tech < -2'),
        'terraform_ban': ('& RULES_SCN_NO_TERRAFORMING','& 0'),
    }
    if damage:
        before,after=mutations[damage]
        assert terrain.count(before)==1
        terrain=terrain.replace(before,after)
    header=(ROOT/'src/terraforming.h').read_text()
    row=re.search(r'struct RulesTerraforming \{.*?\n\};',header,re.S).group()
    actions=re.search(r'enum TerraformingAction \{.*?\n\};',header,re.S).group()
    base=(ROOT/'src/base.h').read_text()
    helpers=definition(base,'base_project')+'\n'+definition(base,'has_project')
    text=r'''
#include <cassert>
#include <cstring>
#include <cstdint>
#define __cdecl
#define MEASURED
#define inline
using LPSTR=char*;
using BOOL=int;
const int TechNone=-1,MaxTechnologyNum=89,SP_WEATHER_PARADIGM=2;
const unsigned RULES_SCN_NO_TERRAFORMING=0x800000;
struct TechnologyRow { int preq_tech_1,preq_tech_2; } Technology[89];
struct GameFields { unsigned game_rules_; unsigned char game_tech_achieved_[89]; int secret_project_[64]; };
struct GameStorage { GameFields fields_; } GameDataStateStorage;
struct Base { int faction_id_current; } Bases[2];
''' + row+'\n'+actions+r'''
RulesTerraforming Terraforming[20];
''' + helpers+'\n'+definition(source,'has_tech')+'\n'+terrain+r'''
int main() {
    static_assert(sizeof(Terraforming[0].prerequisite_tech)==2*sizeof(int),"two prerequisites");
    const int prerequisites[]={-2,-1,4,8,88,89};
    for(int i=0;i<89;++i) {Technology[i].preq_tech_1=-1;Technology[i].preq_tech_2=-1;}
    GameDataStateStorage.fields_.game_tech_achieved_[4]=2;
    GameDataStateStorage.fields_.game_tech_achieved_[8]=4;
    for(int action=0;action<20;++action) for(int sea=0;sea<2;++sea)
    for(int land=0;land<6;++land) for(int water=0;water<6;++water)
    for(int faction=0;faction<4;++faction) for(int owner=-1;owner<4;++owner)
    for(int ban=0;ban<2;++ban) {
        Terraforming[action].prerequisite_tech[0]=prerequisites[land];
        Terraforming[action].prerequisite_tech[1]=prerequisites[water];
        GameDataStateStorage.fields_.game_rules_=ban ? 0x800000 : 0;
        GameDataStateStorage.fields_.secret_project_[2]=owner<0 ? -1 : 0;
        Bases[0].faction_id_current=owner;
        int technology=prerequisites[sea ? water : land];
        bool allowed=false;
        if(technology>=-1 && !(ban && (action==16 || action==17))) {
            if(action>=12 && action<=18 && owner>=0 && owner==faction) allowed=true;
            else if(faction>0) allowed=technology==-1 || (technology==4 && faction==1) || (technology==8 && faction==2);
        }
        assert((terrain_avail(action,sea,faction)!=0)==allowed);
    }
}
'''
    cpp=tmp_path/'terrain.cpp';binary=tmp_path/'terrain'
    cpp.write_text(text)
    built=subprocess.run([compiler,'-std=c++11','-O1','-fsanitize=undefined',
                          '-fno-sanitize-recover=all',str(cpp),'-o',str(binary)],
                         capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True)
    assert (result.returncode==0)==(damage is None),result.stderr
