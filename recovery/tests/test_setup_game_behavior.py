"""Host controls for the actual setup_game initializer body."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl setup_game(int reload) {"


def source_body() -> str:
    source = (ROOT / "src/game.cpp").read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated setup_game body")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
enum { MaxPlayerNum=8, MaxProposalNum=11, MaxTechnologyNum=89,
       MaxSecretProjectNum=64 };
enum { RFLAG_ALIEN=0x80 };
struct RulesBasic { uint32_t normal_starting_year, normal_end_year_low_three_diff,
                    normal_end_year_high_three_diff; } Rules;
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct RulesSocialCategory { SocialEffect model_effect[4]; };
RulesSocialCategory SocialCategories[4];
struct Player {
  int rule_flags; char search_key[24];
  char noun_faction[24], adj_name_faction[128], formal_name_faction[40];
  int soc_ideology_category, soc_anti_ideology_category, soc_ideology_model,
      soc_anti_ideology_model, soc_ideology_effect, soc_anti_ideology_effect;
} Players[8];
struct PlayerData { int diff_level; int diplo_agenda[8]; } PlayersData[8];
struct ProjectTable { int human_genome_project_; int values[63];
 int &human_genome_project() { return human_genome_project_; }
 const int &human_genome_project() const { return human_genome_project_; }
} SecretProject;
struct ReplayEventRecord { int a,b; } ReplayEvents[1024];
uint16_t FactionRankingHistory[8000];
uint32_t GameRules, GameState, GameInterludeFlags, LockEnableMask;
int DiffLevelCurrent, TurnCurrentNum, ObjectiveReqVictory;
uint32_t ObjectivesSuddenDeathVictory, ObjectiveAchievePts,
         VictoryAchieveBonusPts, MissionYearCurrent, StartingMissionYear,
         EndingMissionYear;
int SunspotDuration, GovernorFaction, ClimateRiseStep, ClimateRiseLimit,
    ClimateRiseAccum, ClimateSeaLevelDelta, BaseCurrentCount, GameRestartQueued;
int GameSetupResetWord, TurnControlDurationMilliseconds;
uint32_t MountPlanetX, MountPlanetY, ScnVictFacilityObj, NetTurnFlags;
int DustCloudDuration, SaveDaemonStateWord, GlobalEnergyEventState,
    NetTurnFaction, EcologyEventAccumulator;
int TutorialFlags, TutorialFlags2, TutWinFlags9A64BC;
int CouncilResolutionCount[11], CouncilLastResolutionTurn[11], ReplayEventBytes;
int TectonicDetonationCount[8];
uint8_t GameTechAchieved[89];
char LegacyGameSaveName[256]; // source-owned game.h storage, not a new game object
uint32_t TerraformAnnouncedMask[2];
uint32_t PendingDiplomacyFlags[8][8];
int IsMultiplayerNet;
int clear_events[16], clear_count, setup_calls[8], setup_count;
int prefs_loads, prefs_use_calls, prefs_put_calls, time_warp_calls, balance_calls;
int prefs_string_calls;
char prefs_string_keys[16][32], prefs_string_values[16][32];
int agenda_calls, agenda_first[128], agenda_second[128], agenda_flags[128];
int label_calls, label_offsets[16];
int expect_labels_before_agenda, expect_labels_before_time_warp;
int setup_rand_values[64], setup_rand_count, setup_rand_index, setup_rand_fallback=17;
extern "C" int rand(void) noexcept {
  if (setup_rand_index < setup_rand_count)
    return setup_rand_values[setup_rand_index++];
  setup_rand_fallback = setup_rand_fallback * 1103515245 + 12345;
  return (setup_rand_fallback >> 16) & 0x7fff;
}
int prefs_get(const char *key, int, int) {
  assert(!std::strcmp(key, "DontResetBeginnerPrefs")); return 0;
}
void prefs_load(int) { ++prefs_loads; clear_events[clear_count++]=1; }
void prefs_put(const char *, int, int) { ++prefs_put_calls; clear_events[clear_count++]=2; }
void prefs_put(const char *key, const char *value) {
  assert(prefs_string_calls < 16);
  std::strncpy(prefs_string_keys[prefs_string_calls],key,31);
  std::strncpy(prefs_string_values[prefs_string_calls],value,31);
  ++prefs_string_calls;
  clear_events[clear_count++]=10+prefs_string_calls;
}
void prefs_use() { ++prefs_use_calls; clear_events[clear_count++]=3; }
void clear_monuments() { clear_events[clear_count++]=4; }
void wipe_undo() { clear_events[clear_count++]=5; }
void clear_all_offers() {
  assert(GameSetupResetWord==0 && TurnControlDurationMilliseconds==4567);
  clear_events[clear_count++]=6;
}
void clear_hotseat_chat() { clear_events[clear_count++]=7; }
void clear_council_notify() { clear_events[clear_count++]=8; }
void clear_tamper() {
  assert(GameSetupResetWord==0 && TurnControlDurationMilliseconds==4567);
  clear_events[clear_count++]=9;
}
void balance() { ++balance_calls; }
void time_warp() {
  if (expect_labels_before_time_warp) {
    assert(!std::strcmp(Players[0].formal_name_faction,"Gaian Confederacy"));
    assert(!std::strcmp(Players[0].noun_faction,"Gaian Confederacy"));
    assert(!std::strcmp(Players[0].adj_name_faction,"Gaian"));
    expect_labels_before_time_warp=0;
  }
  ++time_warp_calls;
}
int setup_player(int faction, int conqueror, int retry) {
  assert((conqueror == -2 || conqueror == -1) && retry == 0);
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      assert(PendingDiplomacyFlags[row][column] != 0);
  setup_calls[setup_count++]=faction;
  return 23;
}
void agenda_on(int first, int second, int flags) {
  if (expect_labels_before_agenda) {
    assert(!std::strcmp(Players[0].formal_name_faction,"Gaian Confederacy"));
    assert(!std::strcmp(Players[0].noun_faction,"Gaian Confederacy"));
    assert(!std::strcmp(Players[0].adj_name_faction,"Gaian"));
    expect_labels_before_agenda=0;
  }
  agenda_first[agenda_calls]=first; agenda_second[agenda_calls]=second;
  agenda_flags[agenda_calls++]=flags;
}
void set_treaty(int, int, int, int) {}
void set_agenda(int, int, int, int) {}
char *label_get(int offset) {
  static char formal[] = "Gaian Confederacy";
  static char adjective[] = "Gaian";
  assert(offset == 244 || offset == 245);
  assert(setup_count == 8);
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      assert(PendingDiplomacyFlags[row][column] == 0);
  label_offsets[label_calls++] = offset;
  return offset == 245 ? formal : adjective;
}
char *CharUpperA(char *text) {
  for (char *p=text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a'-'A';
  return text;
}
BODY
int main() {
  LegacyGameSaveName[0]='X'; LegacyGameSaveName[1]='Y';
  GameTechAchieved[88]=0xA5; GameRules=0xFFFFFFFFu; GameState=0xFFFFFFFFu;
  DiffLevelCurrent=0; IsMultiplayerNet=1; LockEnableMask=0x05u;
  BaseCurrentCount=11; GameRestartQueued=0;
  GameSetupResetWord=1234; TurnControlDurationMilliseconds=4567;
  Rules.normal_starting_year=2100; Rules.normal_end_year_low_three_diff=2600;
  Rules.normal_end_year_high_three_diff=2500;
  ObjectiveAchievePts=4; ObjectiveReqVictory=3; ObjectivesSuddenDeathVictory=2;
  VictoryAchieveBonusPts=5; GameInterludeFlags=9; TutorialFlags=1;
  TutorialFlags2=2; TutWinFlags9A64BC=3; GovernorFaction=7;
  ClimateRiseStep=1; ClimateRiseLimit=2; ClimateRiseAccum=3; ClimateSeaLevelDelta=4;
  SunspotDuration=9; MountPlanetX=10; MountPlanetY=11; DustCloudDuration=12;
  SaveDaemonStateWord=13; ScnVictFacilityObj=14; GlobalEnergyEventState=15;
  NetTurnFlags=16; NetTurnFaction=17; EcologyEventAccumulator=18;
  ReplayEventBytes=88;
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      PendingDiplomacyFlags[row][column]=1 + row*8 + column;
  for (int i=0;i<11;++i) { CouncilResolutionCount[i]=1; CouncilLastResolutionTurn[i]=4; }
  for (int i=0;i<64;++i) reinterpret_cast<int *>(&SecretProject)[i]=i;
  for (int i=0;i<1024;++i) ReplayEvents[i].a=1;
  for (int i=0;i<8000;++i) FactionRankingHistory[i]=9;
  for (int i=0;i<8;++i) TectonicDetonationCount[i]=10+i;
  setup_game(1);
  assert(LegacyGameSaveName[0]==0 && LegacyGameSaveName[1]=='Y');
  assert(GameTechAchieved[88]==0 && GameRules==(0xFFFFFFFFu & ~0x20u));
  assert(ObjectiveAchievePts==0 && ObjectiveReqVictory==9999 &&
         ObjectivesSuddenDeathVictory==9999 && VictoryAchieveBonusPts==0);
  assert(StartingMissionYear==2100 && EndingMissionYear==2600);
  assert((GameState & 0x01800000u)==0x01800000u && GameInterludeFlags==0);
  assert(TutorialFlags==0 && TutorialFlags2==0 && TutWinFlags9A64BC==0);
  assert(ClimateRiseStep==0 && ClimateRiseLimit==0 && ClimateRiseAccum==0 &&
         ClimateSeaLevelDelta==0 && GovernorFaction==-1 && ReplayEventBytes==0 &&
         BaseCurrentCount==0 && TurnCurrentNum==0 &&
         GameSetupResetWord==0 && TurnControlDurationMilliseconds==0 &&
         SunspotDuration==0 && MountPlanetX==0 && MountPlanetY==0 &&
         DustCloudDuration==0 && SaveDaemonStateWord==0 && ScnVictFacilityObj==0 &&
         GlobalEnergyEventState==0 && NetTurnFlags==0 && NetTurnFaction==0 &&
         EcologyEventAccumulator==0 &&
         (LockEnableMask & 0x0000FF00u)==0x0000FF00u);
  for (int i=0;i<11;++i) assert(CouncilResolutionCount[i]==0 && CouncilLastResolutionTurn[i]==-9999);
  for (int i=0;i<64;++i) assert(reinterpret_cast<int *>(&SecretProject)[i]==-1);
  for (int i=0;i<1024;++i) assert(ReplayEvents[i].a==0 && ReplayEvents[i].b==0);
  for (int i=0;i<8000;++i) assert(FactionRankingHistory[i]==0);
  for (int i=0;i<8;++i) assert(TectonicDetonationCount[i]==0);
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      assert(PendingDiplomacyFlags[row][column] == 0);
  assert(setup_count==8);
  assert(!std::strcmp(Players[0].formal_name_faction,"Gaian Confederacy") &&
         !std::strcmp(Players[0].noun_faction,"Gaian Confederacy") &&
         !std::strcmp(Players[0].adj_name_faction,"Gaian"));
  assert(label_calls==3 && label_offsets[0]==245 && label_offsets[1]==245 &&
         label_offsets[2]==244);
  const int expected_setup_order[8]={0,2,1,3,4,5,6,7};
  for (int i=0;i<8;++i) assert(setup_calls[i]==expected_setup_order[i]);
  assert(prefs_loads==0 && prefs_use_calls==1 && prefs_put_calls==0 &&
         prefs_string_calls==2 &&
         !std::strcmp(prefs_string_keys[0],"Latest Save") &&
         !std::strcmp(prefs_string_keys[1],"Latest Scenario") &&
         !std::strcmp(prefs_string_values[0],"") &&
         !std::strcmp(prefs_string_values[1],""));
  assert(balance_calls==0 && time_warp_calls==0);
  assert(clear_count>=9 && clear_events[0]==11 && clear_events[1]==12 &&
         clear_events[2]==3 && clear_events[3]==4 && clear_events[4]==5 &&
         clear_events[5]==6 && clear_events[6]==7);

  std::memset(Players,0,sizeof Players);
  std::strcpy(Players[1].search_key,"PEACE");
  std::strcpy(Players[2].search_key,"SPARTANS");
  std::strcpy(Players[0].formal_name_faction,"STALE");
  std::strcpy(Players[0].noun_faction,"STALE");
  std::strcpy(Players[0].adj_name_faction,"STALE");
  GameState=0x01000000u; GameRules=0; LockEnableMask=0x06u;
  IsMultiplayerNet=0; DiffLevelCurrent=0; GameRestartQueued=0;
  GameSetupResetWord=1234; TurnControlDurationMilliseconds=4567;
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      PendingDiplomacyFlags[row][column]=1 + row*8 + column;
  expect_labels_before_agenda=1;
  setup_count=0; agenda_calls=0; time_warp_calls=balance_calls=0;
  setup_game(0);
  int named_agenda_found=0;
  for (int i=0;i<agenda_calls;++i) {
    if (agenda_first[i]==2 && agenda_second[i]==1 && agenda_flags[i]==0x200)
      named_agenda_found=1;
  }
  assert(named_agenda_found && balance_calls==1 && time_warp_calls==0);
  assert((Players[1].soc_ideology_category>=0 && Players[1].soc_ideology_category<3) &&
         Players[1].soc_ideology_model>=1 && Players[1].soc_ideology_model<=3);

  GameRules=0x80u; GameState=0; LockEnableMask=0; DiffLevelCurrent=1;
  std::strcpy(Players[0].formal_name_faction,"STALE");
  std::strcpy(Players[0].noun_faction,"STALE");
  std::strcpy(Players[0].adj_name_faction,"STALE");
  GameSetupResetWord=1234; TurnControlDurationMilliseconds=4567;
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      PendingDiplomacyFlags[row][column]=1 + row*8 + column;
  setup_count=0; label_calls=0;
  expect_labels_before_time_warp=1;
  setup_game(0);
  assert(time_warp_calls==1 && balance_calls==1);

  std::memset(Players,0,sizeof Players);
  std::memset(SocialCategories,0,sizeof SocialCategories);
  SocialCategories[0].model_effect[1].values_[SocialEffect::EFFICIENCY]=1;
  SocialCategories[0].model_effect[2].values_[SocialEffect::RESEARCH]=1;
  SocialCategories[1].model_effect[1].values_[SocialEffect::MORALE]=1;
  SocialCategories[1].model_effect[2].values_[SocialEffect::GROWTH]=1;
  SocialCategories[1].model_effect[3].values_[SocialEffect::PLANET]=1;
  SocialCategories[2].model_effect[1].values_[SocialEffect::PROBE]=1;
  SocialCategories[2].model_effect[2].values_[SocialEffect::INDUSTRY]=1;
  const int choices[16]={0,0, 0,0, 0,1, 1,0, 1,1, 1,2, 2,0, 2,1};
  for (int i=0;i<16;++i) setup_rand_values[i]=choices[i];
  setup_rand_count=16; setup_rand_index=0;
  GameState=0x01000000u; GameRules=0; LockEnableMask=0;
  IsMultiplayerNet=0; DiffLevelCurrent=1;
  GameSetupResetWord=1234; TurnControlDurationMilliseconds=4567;
  for (int row=0; row<8; ++row)
    for (int column=0; column<8; ++column)
      PendingDiplomacyFlags[row][column]=1 + row*8 + column;
  setup_count=0; label_calls=0; agenda_calls=0;
  setup_game(1);
  assert(setup_rand_index==16 && setup_count==8);
  const int expected_categories[8]={0,0,0,1,1,1,2,2};
  const int expected_models[8]={0,1,2,1,2,3,1,2};
  const int expected_effects[8]={0,1,10,4,6,7,8,9};
  int selected[12]={0};
  for (int faction=1;faction<8;++faction) {
    assert(Players[faction].soc_ideology_category==expected_categories[faction]);
    assert(Players[faction].soc_ideology_model==expected_models[faction]);
    assert(Players[faction].soc_ideology_effect==expected_effects[faction]);
    int selection=Players[faction].soc_ideology_category*3+
                  Players[faction].soc_ideology_model;
    assert(selection>=0 && selection<12 && selected[selection]==0);
    selected[selection]=1;
  }
}
"""


def test_setup_game_actual_typed_reset_and_player_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = HARNESS.replace("BODY", source_body())
    executable = tmp_path / "setup-game-host-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode()


def test_setup_game_has_complete_caller_body_without_claiming_callee_closure():
    body = source_body()
    assert "// BODY GOES HERE." not in body
    assert "time_warp();" in body
    assert "balance();" in body
