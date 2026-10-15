"""Source-extracted council_action behavior controls.

These controls execute the implementation body with explicit game-service
stubs. They check state and callback order; they are not native layout,
linked-build, multiplayer-runtime, or full-game acceptance evidence.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_definitions() -> str:
    source = (ROOT / "src/council.cpp").read_text()
    return "\n".join((
        braced((ROOT / "src/council.h").read_text(), "enum CouncilProposals") + ";",
        braced(source, "static void council_parse_leader("),
        braced(source, "void __cdecl council_action("),
    ))


HARNESS = r'''
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
using BOOL = int;
using LPSTR = char *;
constexpr BOOL FALSE = 0, TRUE = 1;
constexpr int MaxPlayerNum = 8, MaxProposalNum = 11;
constexpr uint32_t STATE_VICTORY_DIPLOMATIC=0x200000, STATE_UNK_1=1;
constexpr uint32_t RULES_VICTORY_CONQUEST=2, RULES_VICTORY_COOPERATIVE=0x1000;
constexpr uint32_t RFLAG_ALIEN=0x80, PFLAG_UNK_20000=0x20000;
constexpr uint32_t DTREATY_PACT=1, DTREATY_VENDETTA=0x10, DTREATY_HAVE_SURRENDERED=0x2000000;
constexpr uint32_t DAGENDA_FIGHT_TO_DEATH=8, DAGENDA_DIPLO_PERMANENT=0x2000;
struct Player { int is_leader_female; char title_leader[32], name_leader[32], noun_faction[32]; int noun_gender, is_noun_plural, rule_flags; };
struct PlayerData { uint32_t flags; uint32_t diplo_treaties[8]; int diplo_agenda[8]; int energy_reserves; };
struct BaseSecretProject { int project_base[64]; int empath_guild() const { return -1; } };
#include "@@GAME_DATA_HEADER@@"
GameDataStateWindow GameDataStateStorage;
struct { int sea_level_rises; } WorldBuilder;
struct Console { int faction_id_; void update_data(int); };
Player Players[MaxPlayerNum]; PlayerData PlayersData[MaxPlayerNum]; Console ConsoleGlobal;
int (&CouncilResolutionCount)[MaxProposalNum] = GameDataStateStorage.fields_.council_resolution_count_;
int (&CouncilLastResolutionTurn)[MaxProposalNum] = GameDataStateStorage.fields_.council_last_resolution_turn_;
int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
int EndgameExitReason;
int &ClimateRiseStep = GameDataStateStorage.fields_.climate_rise_step_;
int &ClimateRiseLimit = GameDataStateStorage.fields_.climate_rise_limit_;
int &ClimateRiseAccum = GameDataStateStorage.fields_.climate_rise_accum_;
int &ClimateSeaLevelDelta = GameDataStateStorage.fields_.climate_sea_level_delta_;
int MapSeaLevelCouncil; uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_; uint32_t alive_mask; BOOL IsMultiplayerNet;
int GenderDefault, PluralityDefault, popup_result, scenario, treaty_mutations, agenda_mutations;
char *ScriptPopupFile=(char*)"SCRIPT";
struct Event { std::string kind, a, b; int x, y; };
std::vector<Event> events;
void event(const char *kind,const char *a="",const char *b="",int x=0,int y=0){events.push_back({kind,a,b,x,y});}
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"council assertion line %d scenario %d\n",line,scenario);std::exit(93);}}
#include <cstdio>
#define require(x) check((x),__LINE__)
bool is_alive(int faction){return (alive_mask&(1u<<faction))!=0;}
int parse_says(int id,const char *text,int,int){event("parse",text,"",id,GenderDefault);events.back().y=PluralityDefault;return 0;}
int parse_num(int id,int value){event("number","", "",id,value);return 0;}
int popp(const char *,const char *label,int,const char *picture,int(__cdecl *callback)()){
    require(callback==nullptr);event("popup",label,picture);
    if(scenario==10&&std::strcmp(label,"DEFY")==0)ConsoleGlobal.faction_id_=3;
    return popup_result;
}
void treaty_on(int a,int b,int mask){event("treaty_on","",std::to_string(b).c_str(),a,mask);PlayersData[a].diplo_treaties[b]|=mask;if(scenario==10&&treaty_mutations++==0)ConsoleGlobal.faction_id_=4;}
void treaty_off(int a,int b,int mask){event("treaty_off","",std::to_string(b).c_str(),a,mask);PlayersData[a].diplo_treaties[b]&=~uint32_t(mask);}
void agenda_on(int a,int b,int mask){event("agenda_on","",std::to_string(b).c_str(),a,mask);PlayersData[a].diplo_agenda[b]|=mask;}
void agenda_off(int a,int b,int mask){event("agenda_off","","",a,mask);PlayersData[a].diplo_agenda[b]&=~mask;}
void set_treaty(int a,int b,int mask,BOOL set){event("set_treaty","",std::to_string(b).c_str(),a,mask);if(set)PlayersData[a].diplo_treaties[b]|=mask;else PlayersData[a].diplo_treaties[b]&=~uint32_t(mask);}
void set_agenda(int a,int b,int mask,BOOL set){event("set_agenda","",std::to_string(b).c_str(),a,mask);if(set)PlayersData[a].diplo_agenda[b]|=mask;else PlayersData[a].diplo_agenda[b]&=~mask;if(scenario==10&&agenda_mutations++==0)ConsoleGlobal.faction_id_=5;}
void Console::update_data(int value){event("update","","",value,0);}
// SOURCE
void reset(){
    std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(CouncilResolutionCount,0,sizeof(CouncilResolutionCount));std::memset(CouncilLastResolutionTurn,0,sizeof(CouncilLastResolutionTurn));
    events.clear();ConsoleGlobal.faction_id_=1;TurnCurrentNum=77;EndgameExitReason=0;ClimateRiseStep=0;ClimateRiseLimit=0;ClimateRiseAccum=9;ClimateSeaLevelDelta=0;
    MapSeaLevelCouncil=0;GameState=0;GameRules=0;alive_mask=0;IsMultiplayerNet=FALSE;WorldBuilder.sea_level_rises=4;popup_result=1;
    GenderDefault=PluralityDefault=0;treaty_mutations=agenda_mutations=0;scenario=0;
}
void counters_and_salvage(){
    reset();scenario=1;council_action(3,2);require(CouncilResolutionCount[3]==1&&CouncilLastResolutionTurn[3]==77&&events.empty());
    council_action(4,2);require(CouncilResolutionCount[4]==1&&CouncilLastResolutionTurn[4]==77&&events.empty());
    Players[1].rule_flags=0;Players[2].rule_flags=RFLAG_ALIEN;Players[3].rule_flags=0;PlayersData[1].energy_reserves=10;PlayersData[2].energy_reserves=20;PlayersData[3].energy_reserves=-5;
    council_action(2,0);require(CouncilResolutionCount[2]==1&&PlayersData[1].energy_reserves==510&&PlayersData[2].energy_reserves==20&&PlayersData[3].energy_reserves==495);
    require(events.size()==1&&events[0].kind=="update"&&events[0].x==0);
}
void climate_actions(){
    reset();scenario=2;ConsoleGlobal.faction_id_=3;Players[3].rule_flags=RFLAG_ALIEN;ClimateSeaLevelDelta=2;ClimateRiseAccum=17;MapSeaLevelCouncil=1;
    council_action(5,3);require(MapSeaLevelCouncil==-2&&ClimateSeaLevelDelta==-2&&ClimateRiseStep==2&&ClimateRiseLimit==20&&ClimateRiseAccum==0);
    require(events.size()==3&&events[0].kind=="number"&&events[0].x==0&&events[0].y==20&&events[1].kind=="number"&&events[1].x==1&&events[1].y==266&&events[2].kind=="popup"&&events[2].a=="WILLFALLALIEN");
    reset();scenario=3;ClimateSeaLevelDelta=-3;MapSeaLevelCouncil=0;council_action(7,0);
    require(MapSeaLevelCouncil==3&&ClimateSeaLevelDelta==1&&ClimateRiseStep==1&&events.size()==3&&events[2].a=="WILLRISE");
}
void local_governor(){
    reset();scenario=4;ConsoleGlobal.faction_id_=2;GameRules=RULES_VICTORY_CONQUEST;alive_mask=(1u<<1)|(1u<<2)|(1u<<3);
    Players[1].is_leader_female=1;std::strcpy(Players[1].title_leader,"Title");std::strcpy(Players[1].name_leader,"Leader");std::strcpy(Players[1].noun_faction,"Faction");Players[1].noun_gender=2;Players[1].is_noun_plural=1;
    PlayersData[1].diplo_treaties[2]=0x00040000;
    council_action(1,2);
    require(CouncilResolutionCount[1]==1&&(GameState&STATE_VICTORY_DIPLOMATIC));
    require(events.size()>=6&&events[0].kind=="parse"&&events[0].a=="Title"&&events[0].x==0&&events[0].y==FALSE);
    require(events[1].a=="Leader"&&events[2].a=="Faction"&&events[2].y==1);
    require(events[3].kind=="popup"&&events[3].a=="DEFIES"&&events[4].kind=="treaty_on"&&events[5].kind=="agenda_on");
    require(PlayersData[1].diplo_treaties[2]&DTREATY_VENDETTA);
    require(PlayersData[3].diplo_treaties[2]&0x43);
}
void governor_end_and_dispatch(){
    reset();scenario=5;ConsoleGlobal.faction_id_=1;alive_mask=1u<<1;council_action(1,1);
    require(EndgameExitReason==6&&(GameState&STATE_UNK_1)&&(PlayersData[1].flags&PFLAG_UNK_20000));
    reset();scenario=6;ConsoleGlobal.faction_id_=1;alive_mask=(1u<<1)|(1u<<2);PlayersData[2].diplo_treaties[1]=DTREATY_PACT;
    council_action(1,2);require(GameState&STATE_VICTORY_DIPLOMATIC);require(events.size()>=5&&events[0].kind=="parse"&&events[3].kind=="popup"&&events[3].a=="ACCEDE");
    require(events[4].kind=="popup"&&events[4].a=="DEFY");
    reset();scenario=7;ConsoleGlobal.faction_id_=1;alive_mask=1u<<1;popup_result=0;
    council_action(1,2);require(EndgameExitReason==11&&(GameState&STATE_UNK_1)&&(PlayersData[2].flags&PFLAG_UNK_20000));
    require(events.size()==4&&events[3].kind=="popup"&&events[3].a=="ACCEDE");
    reset();scenario=8;ConsoleGlobal.faction_id_=1;alive_mask=1u<<1;IsMultiplayerNet=TRUE;
    council_action(1,2);require(EndgameExitReason==11&&events.size()==3&&events[2].kind=="parse");
    reset();scenario=9;ConsoleGlobal.faction_id_=1;alive_mask=(1u<<1)|(1u<<2);GameRules=RULES_VICTORY_COOPERATIVE;
    PlayersData[1].diplo_treaties[2]=DTREATY_PACT;popup_result=0;council_action(1,2);
    require(EndgameExitReason==10&&events.size()==4&&events[3].a=="ACCEDECOOP");
    reset();scenario=10;ConsoleGlobal.faction_id_=1;alive_mask=(1u<<1)|(1u<<2)|(1u<<3)|(1u<<4)|(1u<<5);
    council_action(1,2);
    require(GameState&STATE_VICTORY_DIPLOMATIC);
    int defy=4;require(events[defy].kind=="popup"&&events[defy].a=="DEFY");
    require(events[defy+1].kind=="treaty_on"&&events[defy+1].x==1&&events[defy+1].b=="3"&&events[defy+1].y==0x50);
    require(events[defy+4].kind=="set_agenda"&&events[defy+4].x==1&&events[defy+4].b=="4");
    require(events[defy+5].kind=="treaty_on"&&events[defy+5].x==2&&events[defy+5].b=="5"&&events[defy+5].y==0x50);
}
int main(){counters_and_salvage();climate_actions();local_governor();governor_end_and_dispatch();}
'''


def test_council_action_actual_body_controls(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler is unavailable")
    source = HARNESS.replace("@@GAME_DATA_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
    source = source.replace("// SOURCE", actual_definitions())
    source = source.replace("#include <cstring>", "#include <cstring>\n#include <cstdio>")
    cpp = tmp_path / "council_action.cpp"
    exe = tmp_path / "council_action"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr
