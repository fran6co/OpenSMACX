"""Actual-owner behavior controls for the AI diplomacy turn.

The test extracts the actual faction.cpp arithmetic helpers and every helper
in the recovered enemy_diplomacy call chain. Its inline C++ harness supplies
only typed dependency mocks; it does not import or include private fragments.
"""
import pathlib
import re
import shutil
import subprocess
import tempfile

import pytest


HERE = pathlib.Path(__file__).resolve()
SOURCE_ROOT = HERE.parents[2]
FACTION_CPP = SOURCE_ROOT / "src" / "faction.cpp"


def extract_braced(source, marker):
    match = re.search(re.escape(marker) + r"[^;{]*\{", source)
    assert match is not None, "missing definition: " + marker
    start = match.start()
    brace = match.end() - 1
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unclosed function: " + marker)


FUNCTION_MARKERS = (
    "static int diplo_add(int a, int b)",
    "static int diplo_sub(int a, int b)",
    "static int diplo_mul(int a, int b)",
    "static void enemy_diplomacy_sync_agenda_pairs(",
    "static void enemy_diplomacy_pair_prefix(",
    "static void enemy_diplomacy_console_pair(",
    "static int enemy_diplomacy_territory_pair(",
    "static int enemy_diplomacy_relationship_score(",
    "static bool enemy_diplomacy_pair_relationship_finish(",
    "static void enemy_diplomacy_pair_dispatch(",
    "static int enemy_diplomacy_tail_threshold(",
    "static bool enemy_diplomacy_base_has_facility_41(",
    "static void enemy_diplomacy_satellite_attacks(",
    "void __cdecl enemy_diplomacy(",
)


def actual_source_bodies():
    source = FACTION_CPP.read_text()
    return "\n\n".join(extract_braced(source, marker)
                        for marker in FUNCTION_MARKERS)


def make_control_source(damage=None):
    bodies = actual_source_bodies()
    if damage is not None:
        before, after = DAMAGE_CASES[damage]
        assert bodies.count(before) == 1, damage
        bodies = bodies.replace(before, after, 1)
    return r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
enum { MaxPlayerNum=8, MaxBaseCount=134, DTREATY_PACT=1 };
struct SocialCategory { uint32_t models_[4]; };
struct PlayerDataMock {
 uint32_t diplo_treaties[8]; int diplo_agenda[8],diplo_merc[8],diplo_spoke[8];
 char diplo_patience[8]; int integrity_blemishes,x_target,diff_level;
 uint32_t flags_ext,ranking; int mil_strength_1,tech_ranking,ai_power,ai_tech,ai_fight;
 int energy_reserves; SocialCategory soc_category_pending; int planet_busters;
 uint32_t satellites_[4],sat_odp_deployed;
};
struct PlayerMock { int soc_ideology_category,soc_ideology_model; const char *filename; };
struct BaseMock { uint8_t prefix[4],faction_id_current,pad[0x87],facilities_built[12]; };
struct ConsoleMock { int faction_id_; };
struct SecretProjectMock { int empath_guild; };
static PlayerDataMock PlayersData[8]; static PlayerMock Players[8];
static BaseMock Bases[134]; static ConsoleMock ConsoleGlobal;
static SecretProjectMock SecretProject;
static uint32_t LockEnableMask,GameRules,GameState,FactionDiplomacyFlags[8],PendingDiplomacyFlags[8][8];
static int SunspotDuration,TurnCurrentNum,IsMultiplayerNet,IsMultiplayerPBEM;
static int DiploFriction,DiploScratchFA60,DiploScratchFAA0,BaseCurrentCount;
static int territory_calls,territory_result,territory_base,territory_count;
static int friction_calls,friction_score,communication_calls,treaty_calls;
static int encounter_calls,trade_calls,attack_calls,attack_base[8],attack_type[8],attack_target[8];
static int deterministic_random,reputation_result,diplomacy_check_calls;
static int diplo_add(int,int); static int diplo_sub(int,int); static int diplo_mul(int,int);
static bool is_alive(int faction) { return (LockEnableMask & (1u << (8+faction))) != 0; }
static int rand() { return deterministic_random; }
static uint32_t random(int,int) { return static_cast<uint32_t>(deterministic_random); }
static int territory(int,int,int,int *base,int *count) {
 ++territory_calls; *base=territory_base; *count=territory_count; return territory_result;
}
static void cause_friction(int,int,int score) { ++friction_calls; friction_score=score; }
static void treaty_on(int actor,int peer,int flags) {
 ++treaty_calls; PlayersData[actor].diplo_treaties[peer] |= static_cast<uint32_t>(flags);
}
static void communicate(int,int,int) { ++communication_calls; }
static int reputation(int,int) { return reputation_result; }
static void diplomacy_check(int,int,int) { ++diplomacy_check_calls; }
static int great_satan(int,int) { return 0; }
static int aah_ooga(int,int) { return 0; }
static void enemies_trade_tech(int,int) { ++trade_calls; }
static void encounter(int,int,int,int,int,int) { ++encounter_calls; }
static void bitmask(int id,int *byte,int *mask) { *byte=id/8; *mask=1<<(id&7); }
static void action_sat_attack(int actor,int target,int type,int base_id) {
 const int n=attack_calls++; attack_target[n]=target; attack_type[n]=type; attack_base[n]=base_id;
 if (type==4) PlayersData[actor].diplo_treaties[target]=0;
 else PlayersData[actor].sat_odp_deployed=PlayersData[actor].satellites_[3];
}
#define __cdecl
''' + bodies + r'''
static void reset_state() {
 for(int f=0;f<8;++f) {
  Players[f].soc_ideology_category=-1; Players[f].soc_ideology_model=0; Players[f].filename="OTHER";
  FactionDiplomacyFlags[f]=0;
  for(int o=0;o<8;++o) {
   PlayersData[f].diplo_treaties[o]=0; PlayersData[f].diplo_agenda[o]=0;
   PlayersData[f].diplo_merc[o]=0; PlayersData[f].diplo_spoke[o]=0;
   PlayersData[f].diplo_patience[o]=0; PendingDiplomacyFlags[f][o]=0;
  }
  PlayersData[f].integrity_blemishes=0; PlayersData[f].x_target=-1;
  PlayersData[f].diff_level=0; PlayersData[f].flags_ext=0; PlayersData[f].ranking=0;
  PlayersData[f].mil_strength_1=0; PlayersData[f].tech_ranking=0;
  PlayersData[f].ai_power=PlayersData[f].ai_tech=PlayersData[f].ai_fight=0;
  PlayersData[f].energy_reserves=PlayersData[f].planet_busters=0;
  PlayersData[f].sat_odp_deployed=0;
  for(int m=0;m<4;++m) PlayersData[f].soc_category_pending.models_[m]=0;
  for(int s=0;s<4;++s) PlayersData[f].satellites_[s]=0;
 }
 for(int b=0;b<134;++b) { Bases[b].faction_id_current=0; for(int x=0;x<12;++x) Bases[b].facilities_built[x]=0; }
 ConsoleGlobal.faction_id_=1; SecretProject.empath_guild=-1;
 LockEnableMask=GameRules=GameState=0; SunspotDuration=TurnCurrentNum=0;
 IsMultiplayerNet=IsMultiplayerPBEM=0; DiploFriction=DiploScratchFA60=DiploScratchFAA0=0;
 BaseCurrentCount=territory_calls=friction_calls=communication_calls=treaty_calls=0;
 territory_result=territory_base=territory_count=friction_score=0;
 encounter_calls=trade_calls=attack_calls=0; deterministic_random=0; reputation_result=5;
 diplomacy_check_calls=0;
}
int main() {
 reset_state(); PlayersData[0].diplo_agenda[1]=0x20; enemy_diplomacy(0);
 assert(PlayersData[0].diplo_agenda[1]==0x20 && territory_calls==0);

 reset_state(); LockEnableMask=1u<<2; PlayersData[2].diplo_agenda[3]=0x20;
 enemy_diplomacy(2);
 assert((PlayersData[2].diplo_agenda[3]&0x40)!=0 && (PlayersData[3].diplo_agenda[2]&0x40)!=0);
 assert(territory_calls==0 && friction_calls==0);

 // Contact score uses territory output (7), not actor index (2).
 reset_state(); LockEnableMask=(1u<<(8+3))|(1u<<3); PlayersData[2].diplo_treaties[3]=8;
 PlayersData[3].diplo_spoke[2]=0; TurnCurrentNum=10;
 territory_result=7; territory_base=11; territory_count=2; enemy_diplomacy(2);
 assert(territory_calls==1 && friction_calls==1 && friction_score==2);

 // 0x20000000 alone reaches encounter through the native cleanup/dispatch gate.
 reset_state(); LockEnableMask=1u<<(8+1); PlayersData[2].diplo_treaties[1]=0x20000000u;
 TurnCurrentNum=5; enemy_diplomacy(2);
 assert(encounter_calls==1);

 // Native console-only domain rejects nonconsole-human and invalid console
 // relation-mask notification, but both still reach diplomacy_check.
 reset_state(); LockEnableMask=1u<<4; PlayersData[3].diplo_treaties[4]=8;
 PlayersData[4].diplo_spoke[3]=0; TurnCurrentNum=40;
 enemy_diplomacy_console_pair(3,4);
 assert(communication_calls==0 && diplomacy_check_calls==1);
 reset_state(); LockEnableMask=1u<<2; PlayersData[3].diplo_treaties[2]=0x10;
 PlayersData[2].diplo_spoke[3]=0; TurnCurrentNum=40;
 enemy_diplomacy_console_pair(3,2);
 assert(communication_calls==0 && diplomacy_check_calls==1);

 // Type-4 base selection is carried into a later type-3 fast path.
 reset_state(); PlayersData[1].satellites_[3]=3; PlayersData[1].planet_busters=1;
 PlayersData[1].diplo_treaties[2]=PlayersData[1].diplo_treaties[3]=0x810;
 PlayersData[3].satellites_[3]=1; BaseCurrentCount=3; Bases[1].faction_id_current=2;
 { int byte=0,mask=0; bitmask(0x29,&byte,&mask); Bases[1].facilities_built[byte]=static_cast<uint8_t>(mask); }
 enemy_diplomacy_satellite_attacks(1,19);
 assert(attack_calls==2 && attack_type[0]==4 && attack_base[0]==1);
 assert(attack_type[1]==3 && attack_base[1]==1);

 // Fourth-turn treaty bit subtracts one from the common score for either
 // relation kind; nonpact military ratio reads the existing mil_strength_1.
 reset_state(); TurnCurrentNum=4; PlayersData[2].diplo_treaties[3]=2;
 assert(enemy_diplomacy_relationship_score(2,3,0,7)==1);
 reset_state(); PlayersData[2].mil_strength_1=10; PlayersData[3].mil_strength_1=30;
 PlayersData[2].planet_busters=100;
 assert(enemy_diplomacy_relationship_score(2,3,0,7)==0);
 return 0;
}
'''


DAMAGE_CASES = {
    "territory_live_in": ("diplo_add(territory_score, turn_parity)", "diplo_add((static_cast<void>(territory_score), faction_id), turn_parity)"),
    "notification_domain": ("if (console_notification_domain && !has_reached_common_network_gate &&", "if (!has_reached_common_network_gate &&"),
    "dispatch_flag": ("(relation_before_callbacks & 0x20000000u) != 0", "(relation_before_callbacks & 0x20000000u) == 0"),
    "selected_base_carry": (
        "for (; candidate < BaseCurrentCount; ++candidate) {\n                    carried_base_scan_index = candidate;",
        "for (; candidate < BaseCurrentCount; ++candidate) {\n                    carried_base_scan_index = 0;",
    ),
}


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "territory_live_in", "notification_domain", "dispatch_flag", "selected_base_carry"])
def test_actual_enemy_diplomacy_source(compiler, damage):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    source = make_control_source(damage)
    with tempfile.TemporaryDirectory(prefix="enemy-diplomacy-source-control-") as temp:
        cpp = pathlib.Path(temp) / "control.cpp"
        binary = pathlib.Path(temp) / "control"
        cpp.write_text(source)
        subprocess.run(
            [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=undefined", "-fno-sanitize-recover=all", str(cpp),
             "-o", str(binary)], check=True,
        )
        result = subprocess.run([str(binary)], capture_output=True, text=True)
        if damage is None:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0, "damage control escaped: " + damage
