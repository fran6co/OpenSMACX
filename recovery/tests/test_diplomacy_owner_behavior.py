"""Actual-owner controls for the six faction diplomacy bodies.

The test extracts definitions from src/faction.cpp at runtime.  Private
recovery fragments are intentionally not a fallback source.  The host models
only observable calls/state; it is not a VC6 layout or game-runtime proof.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"

SIGNATURES = {
    "comm_check": r"void\s+__cdecl\s+comm_check\s*\(",
    "enemies_unpact": r"void\s+__cdecl\s+enemies_unpact\s*\(",
    "enemies_war": r"void\s+__cdecl\s+enemies_war\s*\(",
    "enemies_trade_tech": r"void\s+__cdecl\s+enemies_trade_tech\s*\(",
    "encounter": r"void\s+__cdecl\s+encounter\s*\(",
    "enemies_treaty": r"void\s+__cdecl\s+enemies_treaty\s*\(",
    "faction_treaty_signed_random16":
        r"static\s+int\s+faction_treaty_signed_random16\s*\(",
    "faction_parse_leader_fields":
        r"static\s+void\s+faction_parse_leader_fields\s*\(",
}


def _matching_brace(text: str, opening: int) -> int:
    """Find a C++ closing brace while ignoring comments and quoted strings."""
    depth = 0
    i = opening
    state = "code"
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line-comment"
            i += 1
        elif char == "/" and nxt == "*":
            state = "block-comment"
            i += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise AssertionError("unterminated function body")


def extract_definition(text: str, name: str) -> str:
    matches = list(re.finditer(SIGNATURES[name], text))
    for match in matches:
        opening = text.find("{", match.end())
        semicolon = text.find(";", match.end())
        if opening < 0 or (semicolon >= 0 and semicolon < opening):
            continue
        closing = _matching_brace(text, opening)
        body = text[match.start():closing + 1]
        assert "BODY GOES HERE" not in body and "PENDING_BODY" not in body
        assert len(body) > 100, name + " is not a complete source body"
        return body.replace("nullptr", "0")
    raise AssertionError("src/faction.cpp lacks complete actual definition: " + name)


@pytest.fixture(scope="session")
def source_bodies():
    assert SOURCE.is_file(), "missing actual faction owner source: " + str(SOURCE)
    text = SOURCE.read_text()
    return {name: extract_definition(text, name) for name in SIGNATURES}


HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
typedef int BOOL;
typedef const char *LPCSTR;
enum { FALSE = 0, TRUE = 1, MaxTechnologyNum = 89, MaxPlayerNum = 8 };
enum {
  DTREATY_PACT=1, DTREATY_TREATY=2, DTREATY_TRUCE=4,
  DTREATY_COMMLINK=8, DTREATY_VENDETTA=16,
  DTREATY_WANT_REVENGE=32, DTREATY_ATROCITY_VICTIM=0x40000,
  DAGENDA_UNK_400=0x400, DAGENDA_UNK_800=0x800,
  DAGENDA_DIPLO_PERMANENT=0x2000, RFLAG_ALIEN=0x80,
  SP_EMPATH_GUILD=4
};
struct PlayerData {
  uint32_t diplo_treaties[8];
  int diplo_agenda[8], diplo_spoke[8], diplo_merc[8], diplo_friction[8];
  int tech_trade_source[MaxTechnologyNum], tech_ranking, ai_fight;
  uint32_t ranking;
  int major_atrocities;
};
struct Player {
  int is_leader_female;
  char search_key[24], title_leader[64], name_leader[64];
  char noun_faction[64], adj_name_faction[64];
  int noun_gender, is_noun_plural, rule_flags;
};
struct Base { int faction_id_current; };
struct ProjectOwner { int empath_guild; };
struct ConsoleOwner { int faction_id_; };
struct ParseEvent { int slot, gender, plural; const char *text; };
struct TreatyEvent { int a, b, flag; };
struct TechEvent { int faction, tech, source; };
struct PopEvent { const char *label; int duration, channel; const char *target; };
struct NetMessage {
  int count;
  PopEvent events[32];
  void (*callback)(int);
  void pop(const char *label, int duration, int channel, const char *target) {
    assert(count < 32);
    events[count] = PopEvent{label, duration, channel, target};
    ++count;
    if (callback) callback(count);
  }
};
PlayerData PlayersData[MaxPlayerNum];
Player Players[MaxPlayerNum];
Base Bases[64];
ProjectOwner SecretProject;
ConsoleOwner ConsoleGlobal;
NetMessage NetMsgPrimary;
uint32_t LockEnableMask, FactionDiplomacyFlags[MaxPlayerNum];
uint32_t PendingDiplomacyFlags[MaxPlayerNum][MaxPlayerNum];
uint32_t FactionRankings[MaxPlayerNum], RankingFactionIDUnk1;
int IsMultiplayerNet, IsMultiplayerPBEM, GameRules, GameState, GovernorFaction;
int TurnCurrentNum, DiffLevelCurrent, SunspotDuration, MapLatitudeBounds;
int MapLongitudeBounds, MapLongitude, DiploFocusX, DiploFocusY;
int GenderDefault, PluralityDefault;
char StringTemp[4096];
BOOL TutorialAllowed, WantsAttack;
BOOL TechOwned[MaxPlayerNum][MaxTechnologyNum];
int TechValues[MaxTechnologyNum], GreatSatanValue, ProjectOwned;
int ParseCount, TreatyCount, NetTreatyCount, SetTreatyCount, PactCount;
int TradeMapCount, CommunicateCount, CommunicateArgs[3], TechAchievedCount;
int RandCalls, RandValues[32], RandValueCount, RandCursor;
int ParseNounFaction[64], ParseNounCount, HasTechQueries[1024][2], HasTechCount;
ParseEvent ParseEvents[256]; TreatyEvent TreatyEvents[64]; TechEvent TechEvents[64];
int InterludeCalls[64];
static int diplo_bit(int faction) { return 1u << (faction & 31); }
static int diplo_add(int a, int b) {
  return static_cast<int>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}
static int diplo_sub(int a, int b) {
  return static_cast<int>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}
int __cdecl parse_says(int slot, LPCSTR text, int gender, int plural) {
  assert(ParseCount < 256);
  ParseEvents[ParseCount++] = ParseEvent{slot, gender, plural, text};
  return 0;
}
char *Nouns[MaxPlayerNum];
char *__cdecl get_noun(int faction) {
  assert(ParseNounCount < 64);
  ParseNounFaction[ParseNounCount++] = faction;
  return Nouns[faction];
}
char *__cdecl get_he_she(int faction, int) { return Players[faction].is_leader_female ? (char*)"she" : (char*)"he"; }
char *__cdecl get_pact_hood(int, int) { return (char*)"hood"; }
int __cdecl has_project(int, int) { return ProjectOwned; }
void __cdecl treaty_on(int a, int b, int flag) {
  assert(TreatyCount < 64);
  TreatyEvents[TreatyCount++] = TreatyEvent{a,b,flag};
  PlayersData[a].diplo_treaties[b] |= static_cast<uint32_t>(flag);
}
void __cdecl net_treaty_on(int a, int b, int flag, int) {
  ++NetTreatyCount; treaty_on(a,b,flag);
}
void __cdecl set_treaty(int a, int b, int flag, int) {
  ++SetTreatyCount; treaty_on(a,b,flag);
}
void __cdecl pact_unpact(int a, int b) {
  ++PactCount;
  PlayersData[a].diplo_treaties[b] &= ~static_cast<uint32_t>(DTREATY_PACT);
  PlayersData[b].diplo_treaties[a] &= ~static_cast<uint32_t>(DTREATY_PACT);
}
void __cdecl trade_maps(int, int) { ++TradeMapCount; }
int __cdecl great_satan(int, int) { return GreatSatanValue; }
BOOL __cdecl has_tech(int tech, int faction) {
  assert(HasTechCount < 1024);
  HasTechQueries[HasTechCount][0] = tech;
  HasTechQueries[HasTechCount++][1] = faction;
  return TechOwned[faction][tech];
}
int __cdecl tech_val(int tech, int, BOOL) { return TechValues[tech]; }
void __cdecl tech_achieved(int faction, int tech, int source, int) {
  TechEvents[TechAchievedCount++] = TechEvent{faction,tech,source};
  TechOwned[faction][tech] = TRUE;
}
void __cdecl communicate(int a, int b, int mode) {
  ++CommunicateCount; CommunicateArgs[0]=a; CommunicateArgs[1]=b; CommunicateArgs[2]=mode;
}
void __cdecl interlude(int id, char *, int, int) { ++InterludeCalls[id]; }
BOOL __cdecl tut_check2(uint32_t) { return TutorialAllowed; }
BOOL __cdecl wants_to_attack(int, int, int) { return WantsAttack; }
void __cdecl enemies_team_up(int, int, int) {}
char *__stdcall CharUpperA(char *text) {
  for (char *p=text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a'-'A';
  return text;
}
struct TutWindow {
  int count; const char *text; int x,y;
  void tut_map(const char *s,int xx,int yy,int,void*,int,int,int) { ++count;text=s;x=xx;y=yy; }
} TutorialWindow;
void __cdecl comm_check(int,int);
void __cdecl enemies_unpact(int,int);
void __cdecl enemies_war(int,int);
void __cdecl enemies_trade_tech(int,int);
void __cdecl enemies_treaty(int,int,int);
void __cdecl encounter(int,int,int,int,int,int);
extern "C" int rand(void) {
  ++RandCalls;
  if (RandCursor < RandValueCount) return RandValues[RandCursor++];
  return 0;
}
@@FACTION_PARSE_LEADER_FIELDS@@
@@FACTION_TREATY_SIGNED_RANDOM16@@
@@COMM_CHECK@@
@@ENEMIES_UNPACT@@
@@ENEMIES_WAR@@
@@ENEMIES_TRADE_TECH@@
@@ENEMIES_TREATY@@
@@ENCOUNTER@@
static void reset_fixture() {
  memset(PlayersData,0,sizeof(PlayersData)); memset(Players,0,sizeof(Players));
  memset(Bases,0,sizeof(Bases)); memset(PendingDiplomacyFlags,0,sizeof(PendingDiplomacyFlags));
  memset(FactionDiplomacyFlags,0,sizeof(FactionDiplomacyFlags));
  memset(FactionRankings,0,sizeof(FactionRankings)); memset(TechOwned,0,sizeof(TechOwned));
  memset(TechValues,0,sizeof(TechValues)); memset(InterludeCalls,0,sizeof(InterludeCalls));
  memset(ParseEvents,0,sizeof(ParseEvents)); memset(TreatyEvents,0,sizeof(TreatyEvents));
  memset(TechEvents,0,sizeof(TechEvents)); memset(HasTechQueries,0,sizeof(HasTechQueries));
  memset(CommunicateArgs,0,sizeof(CommunicateArgs)); memset(RandValues,0,sizeof(RandValues));
  for (int i=0;i<MaxPlayerNum;++i) Nouns[i] = (char*)"noun";
  Nouns[1]=(char*)"actor-noun"; Nouns[2]=(char*)"peer-noun";
  SecretProject.empath_guild=-1; ConsoleGlobal.faction_id_=0; NetMsgPrimary.count=0; NetMsgPrimary.callback=0;
  LockEnableMask=0; IsMultiplayerNet=IsMultiplayerPBEM=0; GameRules=GameState=GovernorFaction=0;
  TurnCurrentNum=0; DiffLevelCurrent=2; SunspotDuration=0; MapLatitudeBounds=MapLongitudeBounds=100;
  MapLongitude=50; DiploFocusX=DiploFocusY=-1; GenderDefault=PluralityDefault=0;
  StringTemp[0]=0; TutorialWindow.count=0; TutorialWindow.text=0; TutorialAllowed=TRUE; WantsAttack=FALSE;
  ParseCount=TreatyCount=NetTreatyCount=SetTreatyCount=PactCount=0;
  TradeMapCount=CommunicateCount=TechAchievedCount=RandCalls=RandValueCount=RandCursor=0;
  ParseNounCount=HasTechCount=GreatSatanValue=ProjectOwned=0;
  for(int i=0;i<MaxPlayerNum;++i) {
    strcpy(Players[i].title_leader,"title"); strcpy(Players[i].name_leader,"leader");
    strcpy(Players[i].noun_faction,"noun"); strcpy(Players[i].adj_name_faction,"adj");
    strcpy(Players[i].search_key,"Faction");
  }
}
static void cb_change_console(int count) { if(count==1) ConsoleGlobal.faction_id_=2; }
static void test_trade_direction_and_query_order() {
  reset_fixture();
  TechOwned[2][7]=TRUE; PlayersData[2].tech_trade_source[7]=3;
  PlayersData[2].diplo_treaties[3]=DTREATY_TREATY;
  PlayersData[2].diplo_friction[3]=15;
  TechOwned[1][8]=TRUE; PlayersData[1].tech_trade_source[8]=3;
  PlayersData[1].diplo_treaties[3]=DTREATY_TREATY;
  PlayersData[2].tech_trade_source[8]=4;
  PlayersData[2].diplo_treaties[4]=DTREATY_TREATY;
  PlayersData[2].diplo_friction[3]=0; PlayersData[2].diplo_friction[4]=16;
  TechOwned[2][9]=TRUE; PlayersData[2].tech_trade_source[9]=0;
  TechValues[8]=TechValues[9]=100; RandValues[0]=11; RandValues[1]=12; RandValueCount=2;
  enemies_trade_tech(1,2);
  assert(HasTechQueries[0][0]==0 && HasTechQueries[0][1]==1);
  assert(HasTechQueries[1][0]==0 && HasTechQueries[1][1]==2);
  for(int i=0;i<TechAchievedCount;++i) assert(TechEvents[i].tech != 7);
  assert(TechAchievedCount==2);
  assert(TechEvents[0].faction==1 && TechEvents[0].tech==9);
  assert(TechEvents[1].faction==2 && TechEvents[1].tech==8);
}
static void test_comm_pbem_row_and_network_callback_reload() {
  reset_fixture(); IsMultiplayerPBEM=1; LockEnableMask=1u<<1; ConsoleGlobal.faction_id_=0;
  comm_check(1,2);
  assert(PendingDiplomacyFlags[1][2]==3u && PendingDiplomacyFlags[2][1]==0u);
  reset_fixture(); IsMultiplayerNet=1; ConsoleGlobal.faction_id_=1;
  LockEnableMask=(1u<<1)|(1u<<2); NetMsgPrimary.callback=cb_change_console;
  comm_check(1,2);
  assert(NetMsgPrimary.count==2);
  assert(strcmp(NetMsgPrimary.events[0].label,"NETMET")==0);
  assert(strcmp(NetMsgPrimary.events[1].label,"NETMET")==0);
}
static void test_war_unpact_notice_bit() {
  reset_fixture(); ConsoleGlobal.faction_id_=0; GovernorFaction=7;
  PlayersData[1].diplo_treaties[2]=DTREATY_PACT;
  PlayersData[0].diplo_treaties[1]=0x00001000u;
  enemies_unpact(1,2);
  assert(NetMsgPrimary.count==1 && strcmp(NetMsgPrimary.events[0].label,"ENEMYPACTENDS")==0);
  reset_fixture(); ConsoleGlobal.faction_id_=0; GovernorFaction=7;
  PlayersData[1].diplo_treaties[2]=DTREATY_PACT;
  PlayersData[0].diplo_treaties[1]=0x00100000u;
  enemies_unpact(1,2); assert(NetMsgPrimary.count==0);
  reset_fixture(); ConsoleGlobal.faction_id_=0; GovernorFaction=7;
  PlayersData[0].diplo_treaties[1]=0x00001000u;
  enemies_war(1,2);
  assert(NetMsgPrimary.count==1 && strcmp(NetMsgPrimary.events[0].label,"ENEMYWAR")==0);
  reset_fixture(); ConsoleGlobal.faction_id_=0; GovernorFaction=7;
  PlayersData[0].diplo_treaties[1]=0x00100000u;
  enemies_war(1,2); assert(NetMsgPrimary.count==0);
}
static void test_encounter_uses_full_map_bounds() {
  reset_fixture(); TurnCurrentNum=10; MapLatitudeBounds=50; MapLongitudeBounds=100; MapLongitude=50;
  Players[1].rule_flags=Players[2].rule_flags=RFLAG_ALIEN;
  encounter(1,2,75,49,0,0);
  assert((PlayersData[1].diplo_treaties[2]&0x40000000u)!=0);
  reset_fixture(); TurnCurrentNum=10; MapLatitudeBounds=50; MapLongitudeBounds=100; MapLongitude=50;
  Players[1].rule_flags=Players[2].rule_flags=RFLAG_ALIEN;
  encounter(1,2,100,49,0,0);
  assert((PlayersData[1].diplo_treaties[2]&0x40000000u)==0);
}
static void test_treaty_lower_labels_flags_and_nouns() {
  reset_fixture(); ConsoleGlobal.faction_id_=3;
  strcpy(Players[1].noun_faction,"actor-noun"); strcpy(Players[1].adj_name_faction,"actor-adj");
  strcpy(Players[2].noun_faction,"peer-noun"); strcpy(Players[2].adj_name_faction,"peer-adj");
  PlayersData[3].diplo_treaties[1]=DTREATY_PACT;
  enemies_treaty(1,2,0);
  assert(NetMsgPrimary.count==1 && strcmp(NetMsgPrimary.events[0].label,"PACTTREATY")==0);
  assert(TreatyCount==1 && TreatyEvents[0].a==1 && TreatyEvents[0].b==2 && TreatyEvents[0].flag==DTREATY_TREATY);
  assert(ParseEvents[2].slot==2 && strcmp(ParseEvents[2].text,"actor-noun")==0);
  assert(ParseCount==5 && ParseEvents[4].slot==4 && strcmp(ParseEvents[4].text,"peer-noun")==0);
  assert(ParseNounCount==1 && ParseNounFaction[0]==2);
  reset_fixture(); ConsoleGlobal.faction_id_=3;
  strcpy(Players[1].noun_faction,"actor-noun"); strcpy(Players[1].adj_name_faction,"actor-adj");
  strcpy(Players[2].noun_faction,"peer-noun"); strcpy(Players[2].adj_name_faction,"peer-adj");
  PlayersData[3].diplo_treaties[2]=DTREATY_PACT;
  enemies_treaty(1,2,0);
  assert(NetMsgPrimary.count==1 && strcmp(NetMsgPrimary.events[0].label,"PACTTREATY")==0);
  assert(TreatyCount==1 && TreatyEvents[0].flag==DTREATY_TREATY);
  assert(ParseEvents[2].slot==2 && strcmp(ParseEvents[2].text,"peer-noun")==0);
  assert(ParseCount==5 && ParseEvents[4].slot==4 && strcmp(ParseEvents[4].text,"actor-noun")==0);
  assert(ParseNounCount==1 && ParseNounFaction[0]==1);
}
static void test_treaty_direct_noun_slots_are_not_adjectives() {
  reset_fixture(); ConsoleGlobal.faction_id_=3;
  strcpy(Players[1].noun_faction,"actor-noun"); strcpy(Players[1].adj_name_faction,"actor-adj");
  strcpy(Players[2].noun_faction,"peer-noun"); strcpy(Players[2].adj_name_faction,"peer-adj");
  enemies_treaty(1,2,0);
  assert(NetMsgPrimary.count==1 && strcmp(NetMsgPrimary.events[0].label,"ENEMYTRUCE")==0);
  assert(ParseCount==2 && ParseEvents[0].slot==0 && ParseEvents[1].slot==1);
  assert(strcmp(ParseEvents[0].text,"actor-noun")==0);
  assert(strcmp(ParseEvents[1].text,"peer-noun")==0);
}
static void test_treaty_initial_slot_six_uses_actor_noun() {
  reset_fixture(); ConsoleGlobal.faction_id_=3; DiffLevelCurrent=2;
  PlayersData[1].diplo_treaties[2]=DTREATY_TRUCE; RandValues[0]=0; RandValueCount=1;
  PlayersData[3].diplo_treaties[1]=DTREATY_PACT;
  enemies_treaty(1,2,0);
  assert(ParseCount==7 && ParseEvents[6].slot==6);
  assert(strcmp(ParseEvents[6].text,"actor-noun")==0);
  assert(ParseNounCount==1 && ParseNounFaction[0]==1);
}
int main() {
  test_trade_direction_and_query_order();
  test_comm_pbem_row_and_network_callback_reload();
  test_war_unpact_notice_bit();
  test_encounter_uses_full_map_bounds();
  test_treaty_lower_labels_flags_and_nouns();
  test_treaty_direct_noun_slots_are_not_adjectives();
  test_treaty_initial_slot_six_uses_actor_noun();
  return 0;
}
'''



# Native-derived callback/order controls supplement the six-body baseline cases.
HARNESS = HARNESS.replace(
    "DTREATY_WANT_REVENGE=32, DTREATY_ATROCITY_VICTIM=0x40000,",
    "DTREATY_WANT_REVENGE=32, DTREATY_UNK_40=0x40, "
    "DTREATY_WANT_TO_TALK=0x2000, DTREATY_ATROCITY_VICTIM=0x40000,")
HARNESS = HARNESS.replace(
    "int InterludeCalls[64];",
    "int InterludeCalls[64];\n"
    "int InterludeMutationId, InterludeMutationFaction, InterludeMutationFlags;\n"
    "int WantsAttackCalls;\n"
    "uint32_t TutorialMasks[32]; int TutorialMaskCount;\n"
    "int ParseMutationCall, ParseMutationFaction, ParseMutationRuleFaction;\n"
    "int ParseMutationRuleFlags, ParseMutationMask;")
HARNESS = HARNESS.replace(
    "int __cdecl parse_says(int slot, LPCSTR text, int gender, int plural) {\n"
    "  assert(ParseCount < 256);\n"
    "  ParseEvents[ParseCount++] = ParseEvent{slot, gender, plural, text};\n"
    "  return 0;\n"
    "}",
    "int __cdecl parse_says(int slot, LPCSTR text, int gender, int plural) {\n"
    "  assert(ParseCount < 256);\n"
    "  ParseEvents[ParseCount++] = ParseEvent{slot, gender, plural, text};\n"
    "  if (ParseCount == ParseMutationCall) {\n"
    "    LockEnableMask = static_cast<uint32_t>(ParseMutationMask);\n"
    "    ConsoleGlobal.faction_id_ = ParseMutationFaction;\n"
    "    Players[ParseMutationRuleFaction].rule_flags |= ParseMutationRuleFlags;\n"
    "  }\n"
    "  return 0;\n"
    "}")
HARNESS = HARNESS.replace(
    "void __cdecl interlude(int id, char *, int, int) { ++InterludeCalls[id]; }",
    "void __cdecl interlude(int id, char *, int, int) {\n"
    "  ++InterludeCalls[id];\n"
    "  if (id == InterludeMutationId) {\n"
    "    ConsoleGlobal.faction_id_ = InterludeMutationFaction;\n"
    "    Players[InterludeMutationFaction].rule_flags |= InterludeMutationFlags;\n"
    "  }\n"
    "}")
HARNESS = HARNESS.replace(
    "BOOL __cdecl tut_check2(uint32_t) { return TutorialAllowed; }",
    "BOOL __cdecl tut_check2(uint32_t mask) {\n"
    "  TutorialMasks[TutorialMaskCount++] = mask;\n"
    "  return TutorialAllowed;\n"
    "}")
HARNESS = HARNESS.replace(
    "BOOL __cdecl wants_to_attack(int, int, int) { return WantsAttack; }",
    "BOOL __cdecl wants_to_attack(int, int, int) { ++WantsAttackCalls; return WantsAttack; }")
HARNESS = HARNESS.replace(
    "  ParseNounCount=HasTechCount=GreatSatanValue=ProjectOwned=0;",
    "  ParseNounCount=HasTechCount=GreatSatanValue=ProjectOwned=0;\n"
    "  InterludeMutationId=-1; InterludeMutationFaction=-1;\n"
    "  InterludeMutationFlags=0; WantsAttackCalls=0; TutorialMaskCount=0;\n"
    "  ParseMutationCall=-1; ParseMutationFaction=-1; ParseMutationRuleFaction=0;\n"
    "  ParseMutationRuleFlags=0; ParseMutationMask=0;")
_EXTRA_ENCOUNTER_TESTS = r"""
static void test_encounter_short_circuit_and_interlude_reload() {
  reset_fixture();
  TurnCurrentNum=10; ConsoleGlobal.faction_id_=1; SunspotDuration=1;
  Players[1].rule_flags=RFLAG_ALIEN;
  PlayersData[1].diplo_treaties[2]=DTREATY_COMMLINK;
  TechOwned[2][0x4E]=TRUE; TechOwned[1][0x4E]=TRUE;
  InterludeMutationId=0x23; InterludeMutationFaction=2;
  encounter(1,2,3,4,0,0);
  assert(HasTechCount==1);
  assert(HasTechQueries[0][0]==0x4E && HasTechQueries[0][1]==2);
  assert(InterludeCalls[0x23]==1 && InterludeCalls[0x17]==1);
}
static void test_encounter_peer_local_human_uses_8000_label() {
  reset_fixture();
  TurnCurrentNum=10; ConsoleGlobal.faction_id_=2;
  Players[2].rule_flags=RFLAG_ALIEN;
  TechOwned[1][0x4E]=TRUE; TechOwned[2][0x4E]=TRUE;
  strcpy(Players[2].noun_faction,"peer-noun");
  strcpy(Players[2].adj_name_faction,"peer-adj");
  encounter(1,2,3,4,0,0);
  assert(ParseCount>=1 && ParseEvents[0].slot==0 && strcmp(ParseEvents[0].text,"peer-noun")==0);
  assert(TutorialMaskCount==1 && TutorialMasks[0]==0x8000u);
  assert(TutorialWindow.count==1);
  assert(strcmp(TutorialWindow.text,"GUYMEETSAL")==0);
}
static void test_encounter_reloads_peer_alien_bit_after_parse() {
  reset_fixture();
  TurnCurrentNum=10; ConsoleGlobal.faction_id_=2;
  Players[1].rule_flags=RFLAG_ALIEN; TechOwned[1][4]=TRUE;
  strcpy(Players[2].noun_faction,"peer-noun");
  strcpy(Players[2].adj_name_faction,"peer-adj");
  ParseMutationCall=1; ParseMutationFaction=2; ParseMutationRuleFaction=2;
  ParseMutationRuleFlags=RFLAG_ALIEN;
  encounter(1,2,3,4,0,0);
  assert(ParseCount>=1 && ParseEvents[0].slot==0 && strcmp(ParseEvents[0].text,"peer-noun")==0);
  assert(TutorialMaskCount==1 && TutorialMasks[0]==0x10000u);
  assert(TutorialWindow.count==1 && strcmp(TutorialWindow.text,"ALMEETSAL")==0);
}
static void test_encounter_reloads_actor_alien_bit_after_parse() {
  reset_fixture();
  TurnCurrentNum=10; ConsoleGlobal.faction_id_=1;
  Players[2].rule_flags=RFLAG_ALIEN; TechOwned[1][0x4E]=TRUE;
  strcpy(Players[1].noun_faction,"actor-noun");
  strcpy(Players[1].adj_name_faction,"actor-adj");
  ParseMutationCall=1; ParseMutationFaction=1; ParseMutationRuleFaction=1;
  ParseMutationRuleFlags=RFLAG_ALIEN;
  encounter(1,2,3,4,0,0);
  assert(ParseCount>=1 && ParseEvents[0].slot==0 && strcmp(ParseEvents[0].text,"actor-noun")==0);
  assert(TutorialMaskCount==1 && TutorialMasks[0]==0x10000u);
  assert(TutorialWindow.count==1 && strcmp(TutorialWindow.text,"ALMEETSAL")==0);
}
static void test_encounter_aliens_gate_all_ai_callbacks_after_reload() {
  reset_fixture();
  TurnCurrentNum=9; ConsoleGlobal.faction_id_=1;
  Players[1].rule_flags=RFLAG_ALIEN;
  PlayersData[1].diplo_treaties[2]=DTREATY_COMMLINK;
  TechOwned[2][0x4E]=TRUE; TechOwned[1][0x4E]=TRUE;
  InterludeMutationId=0x23; InterludeMutationFaction=2;
  InterludeMutationFlags=RFLAG_ALIEN;
  PlayersData[1].diplo_agenda[2]=DAGENDA_DIPLO_PERMANENT;
  encounter(1,2,3,4,0,0);
  assert(WantsAttackCalls==0 && TechAchievedCount==0);
}
"""
HARNESS = HARNESS.replace("int main() {", _EXTRA_ENCOUNTER_TESTS + "\nint main() {")
HARNESS = HARNESS.replace(
    "  test_treaty_initial_slot_six_uses_actor_noun();",
    "  test_treaty_initial_slot_six_uses_actor_noun();\n"
    "  test_encounter_short_circuit_and_interlude_reload();\n"
    "  test_encounter_peer_local_human_uses_8000_label();\n"
    "  test_encounter_reloads_peer_alien_bit_after_parse();\n"
    "  test_encounter_reloads_actor_alien_bit_after_parse();\n"
    "  test_encounter_aliens_gate_all_ai_callbacks_after_reload();")

def make_source(bodies):
    source = HARNESS
    for name, body in bodies.items():
        source = source.replace("@@" + name.upper() + "@@", body)
    assert "@@" not in source, "unfilled body extraction placeholder"
    return source


def compile_and_run(tmp_path: Path, compiler: str, bodies, expect_success=True):
    source_path = tmp_path / "diplomacy_owner.cpp"
    binary = tmp_path / "diplomacy_owner"
    source_path.write_text(make_source(bodies))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-omit-frame-pointer",
         str(source_path), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if expect_success:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "host controls failed to detect deliberate body mutation"


def compilers():
    return [path for name in ("g++", "clang++") if (path := shutil.which(name))]


@pytest.mark.parametrize("compiler", compilers())
def test_actual_diplomacy_bodies(compiler, tmp_path, source_bodies):
    compile_and_run(tmp_path, compiler, source_bodies)


@pytest.mark.parametrize("compiler", compilers())
@pytest.mark.parametrize("name,old,new", [
     ("enemies_trade_tech",
     "const int recipient_source =\n                PlayersData[faction_id_with].tech_trade_source[tech_id];",
     "const int recipient_source =\n                PlayersData[faction_id].tech_trade_source[tech_id];"),
    ("enemies_unpact", "0x00001000u", "0x00100000u"),
    ("enemies_war", "0x00001000u", "0x00100000u"),
    ("encounter", "MapLongitudeBounds", "MapLongitude"),
    ("enemies_treaty", 'NetMsgPrimary.pop("PACTTREATY"',
     'NetMsgPrimary.pop("PACTTRUCE"'),
    ("comm_check", "faction_id_with == ConsoleGlobal.faction_id_",
     "faction_id == ConsoleGlobal.faction_id_"),
])
def test_deliberate_regressions_are_detected(compiler, tmp_path,
                                              source_bodies, name, old, new):
    bodies = dict(source_bodies)
    changed, count = bodies[name].replace(old, new, 1), bodies[name].count(old)
    assert count > 0, "mutation anchor missing from actual " + name
    bodies[name] = changed
    compile_and_run(tmp_path, compiler, bodies, expect_success=False)
