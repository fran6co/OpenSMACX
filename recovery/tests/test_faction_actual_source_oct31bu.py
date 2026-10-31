"""Actual Faction callback reads and actor-relative capability selection.

Expectations come from 539CCA's post-call load and 560F9A..561048's actor
row, staged admissions, and five signed maxima. Host controls do not prove
native layout, full recovery, or game acceptance.
"""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual body')


HARNESS = r'''
#include <cassert>
#include <cstring>
#include <cstdint>
#define __cdecl
using BOOL = int;
const int MaxPlayerNum=8,MaxWeaponNum=1,MaxArmorNum=1,MaxChassisNum=1;
const int BSC_MIND_WORMS=8,WPN_PSI_ATTACK=15,ARM_PSI_DEFENSE=7,TRIAD_LAND=0;
const int DLVL_SPECIALIST=1,DLVL_TALENT=2,DLVL_LIBRARIAN=3,DLVL_TRANSCEND=5;
const int RULES_INTENSE_RIVALRY=0x40,RULES_VICTORY_CONQUEST=2;
const int DTREATY_PACT=1,DTREATY_TREATY=2,DTREATY_COMMLINK=8,DTREATY_VENDETTA=16;
struct State {
    struct Fields { int faction_rankings_[8],turn_current_num_,game_rules_; } fields_;
} GameDataStateStorage;
struct Scenario { int objective_req_victory; } ScenarioSave;
struct PlayerData {
    int diff_level,current_num_bases;
    uint32_t diplo_treaties[8];
    int best_psi_offense,best_weapon_value,best_psi_defense,best_armor_value,best_land_speed;
    int enemy_best_weapon_value,enemy_best_armor_value,enemy_best_land_speed;
    int enemy_best_psi_offense,enemy_best_psi_defense;
} PlayersData[8];
struct WeaponRecord { int preq_tech,offense_rating; } Weapon[1];
struct ArmorRecord { int preq_tech,defense_rating; } Armor[1];
struct ChassisRecord { int preq_tech,triad,speed; } Chassis[1];
int FactionRankingsUnk[8],human_mask;
int climax_rules,ooga_rules,climax_result,ooga_result,climax_calls,ooga_calls;
int availability_calls,technology_calls;
bool is_human(int faction) { return (human_mask & (1<<faction))!=0; }
int climactic_battle() {
    ++climax_calls;
    if(climax_rules>=0) GameDataStateStorage.fields_.game_rules_=climax_rules;
    return climax_result;
}
int aah_ooga(int,int) {
    ++ooga_calls;
    if(ooga_rules>=0) GameDataStateStorage.fields_.game_rules_=ooga_rules;
    return ooga_result;
}
int veh_avail(int prototype,int faction,int mode) {
    assert(prototype==8 && faction==6 && mode==-1);++availability_calls;return 0;
}
int has_tech(int,int faction) { assert(faction==6);++technology_calls;return 0; }
int weap_strat(int,int) { assert(false);return 0; }
int arm_strat(int,int) { assert(false);return 0; }
ACTUAL
void reset_threat(int rules) {
    std::memset(PlayersData,0,sizeof PlayersData);
    std::memset(&GameDataStateStorage,0,sizeof GameDataStateStorage);
    std::memset(FactionRankingsUnk,0,sizeof FactionRankingsUnk);
    human_mask=2;
    GameDataStateStorage.fields_.faction_rankings_[7]=1;
    GameDataStateStorage.fields_.faction_rankings_[6]=2;
    GameDataStateStorage.fields_.turn_current_num_=500;
    GameDataStateStorage.fields_.game_rules_=rules;
    PlayersData[1].current_num_bases=100;PlayersData[1].diff_level=3;
    ScenarioSave.objective_req_victory=2000;
    FactionRankingsUnk[1]=10;FactionRankingsUnk[2]=7;
    climax_rules=ooga_rules=-1;climax_result=ooga_result=0;
    climax_calls=ooga_calls=0;
}
void callback_cases() {
    // Rival arithmetic: 4*10 >=5*7; ordinary arithmetic:2*10 <3*7.
    reset_threat(0);climax_rules=0x40;
    assert(great_satan(1,0)==1 && climax_calls==1 && ooga_calls==0);
    reset_threat(0x40);climax_rules=0;
    assert(great_satan(1,0)==0 && climax_calls==1 && ooga_calls==0);
    reset_threat(0);climax_result=1;ooga_rules=0x40;
    assert(great_satan(1,0)==1 && ooga_calls==1);
    reset_threat(0x40);climax_result=1;ooga_rules=0;
    assert(great_satan(1,0)==0 && ooga_calls==1);
    reset_threat(0);climax_result=1;ooga_result=1;
    FactionRankingsUnk[1]=-100;
    assert(great_satan(1,0)==1 && ooga_calls==1);
    reset_threat(0);human_mask=0;
    assert(great_satan(1,0)==0 && climax_calls==0 && ooga_calls==0);
}
void values(int who,int w,int a,int s,int p,int d) {
    PlayersData[who].best_weapon_value=w;PlayersData[who].best_armor_value=a;
    PlayersData[who].best_land_speed=s;PlayersData[who].best_psi_offense=p;
    PlayersData[who].best_psi_defense=d;
}
void reset_capabilities() {
    std::memset(PlayersData,0,sizeof PlayersData);
    availability_calls=technology_calls=0;
    for(int row=0;row<8;++row)for(int col=0;col<8;++col)
        PlayersData[row].diplo_treaties[col]=DTREATY_PACT;
    values(1,1000,1000,1000,1000,1000);
    values(2,20,21,22,23,24);values(3,200,201,202,203,204);
}
void expect(int w,int a,int s,int p,int d) {
    const PlayerData &actor=PlayersData[6];
    assert(actor.enemy_best_weapon_value==w && actor.enemy_best_armor_value==a);
    assert(actor.enemy_best_land_speed==s && actor.enemy_best_psi_offense==p);
    assert(actor.enemy_best_psi_defense==d);
    assert(availability_calls==2 && technology_calls==3);
}
void capability_cases() {
    // Distinct actor row6; pass rows0..3 deliberately admit nobody.
    const int relationship_for_pass[]={24,8,10,0};
    for(int pass=0;pass!=4;++pass) {
        reset_capabilities();
        PlayersData[6].diplo_treaties[2]=relationship_for_pass[pass];
        enemy_capabilities(6);expect(20,21,22,23,24);
    }
    // A later-pass faction with larger scores must not replace a first-pass winner.
    reset_capabilities();PlayersData[6].diplo_treaties[2]=24;
    PlayersData[6].diplo_treaties[3]=8;
    enemy_capabilities(6);expect(20,21,22,23,24);
    reset_capabilities();PlayersData[6].diplo_treaties[2]=24;
    PlayersData[6].diplo_treaties[4]=24;values(4,-1,99,-4,23,0);
    enemy_capabilities(6);expect(20,99,22,23,24);
}
int main() { callback_cases();capability_cases(); }
'''


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_callback_reads_actor_row_and_filter_stages(tmp_path, compiler):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    bodies = '\n'.join(definition(source, signature) for signature in (
        'BOOL __cdecl great_satan(', 'void __cdecl enemy_capabilities('))

    def run(candidate, name):
        # VC6's for initializer keeps i in function scope. Project that dialect
        # into modern host C++ without changing the loop bodies or their state.
        candidate = candidate.replace('for (int i = 0;', 'int i;\n    for (i = 0;', 1)
        cpp = tmp_path / (name + '.cpp')
        binary = tmp_path / name
        cpp.write_text(HARNESS.replace('ACTUAL', candidate))
        built = subprocess.run([executable, '-std=c++11', '-O2', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(binary)], capture_output=True, text=True)

    assert run(bodies, 'actual').returncode == 0
    live = 'if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0)'
    row = 'uint32_t treaties = PlayersData[actor].diplo_treaties[j];'
    stage = 'faction_id == 0 && (treaties & DTREATY_VENDETTA) == 0'
    mutants = {
        'stale_rules': (live, 'if (has_intense_riv)'),
        'pass_as_actor': (row, 'uint32_t treaties = PlayersData[faction_id].diplo_treaties[j];'),
        'wrong_pass': (stage, 'faction_id == 3 && (treaties & DTREATY_VENDETTA) == 0'),
    }
    for name, (before, after) in mutants.items():
        assert before in bodies
        assert run(bodies.replace(before, after, 1), name).returncode != 0
