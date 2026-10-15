"""Actual-source bounded controls that extract only real src/faction.cpp bodies.

The host fixture checks behavior and source reachability; it does not establish
native layout, runtime behavior, or whole-image preservation. It deliberately
does not import the ignored candidate fragments.
"""

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/faction.cpp").is_file())
FACTION_CPP = ROOT / "src" / "faction.cpp"


def extract_function(text, name):
    match = re.search(
        r"(?:^|\n)[^\n;{}]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{",
        text,
    )
    assert match is not None, "missing product definition: " + name
    start = match.start() + (text[match.start()] == "\n")
    opening = text.index("{", match.start())
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n": state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state in ("string", "character"):
            if escaped: escaped = False
            elif char == "\\": escaped = True
            elif (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ): state = "code"
        elif char == "/" and next_char == "/":
            state = "line_comment"
            index += 1
        elif char == "/" and next_char == "*":
            state = "block_comment"
            index += 1
        elif char == '"': state = "string"
        elif char == "'": state = "character"
        elif char == "{": depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0: return text[start:index + 1]
        index += 1
    raise AssertionError("unterminated product definition: " + name)


def product_definitions():
    source = FACTION_CPP.read_text()
    helper = extract_function(source, "major_atrocity_add_wrapped32")
    signed_helper = extract_function(source, "atrocity_signed_from_bits")
    council = extract_function(source, "council")
    atrocity = extract_function(source, "atrocity")
    assert "void __cdecl council(int faction_id, int proposal, int target_faction)" in council
    assert "void __cdecl atrocity(int actor_id, int victim_id, int kind, int unknown)" in atrocity
    return helper, signed_helper, council, atrocity


FIXTURE = r'''#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
#define TRUE 1
#define FALSE 0
#define RFLAG_ALIEN 0x80
#define DTREATY_COMMLINK 8u
#define DTREATY_TREATY 2u
#define DTREATY_VENDETTA 0x10u
#define DTREATY_WANT_REVENGE 0x20u
#define DTREATY_UNK_40 0x40u
#define DTREATY_ATROCITY_VICTIM 0x40000u
#define DTREATY_UNK_200000 0x200000u
#define DAGENDA_UNK_4 4u
#define PFLAG_BEEN_ELECTED_GOVERNOR 0x8000u
#define PFLAG_COMMIT_ATROCIT_WANTONLY 0x1000000u
#define PROP_REPEAL_UN_CHARTER 8
#define PROP_REINSTATE_UN_CHARTER 9
static const int MaxPlayerNum=8;
static const int PROP_ELECT_PLANETARY_GOVERNOR=0;
struct Player {
    int rule_flags, is_leader_female, noun_gender, is_noun_plural;
    char *title_leader, *name_leader, *noun_faction;
};
struct PlayerData {
    uint32_t flags; int diff_level; uint32_t diplo_treaties[8];
    int diplo_agenda[8], diplo_merc[8], atrocities, integrity_blemishes;
    int sanction_turns, council_call_turn;
};
struct RulesProposal { char *name; };
struct BaseSecretProject { int project_base[64]; int empath_guild() const { return -1; } };
#include "@@GAME_DATA_HEADER@@"
GameDataStateWindow GameDataStateStorage;
struct CouncWin {
    uint32_t field_A1C_, field_A20_;
    int init_arg0, init_arg1;
    void init(int a,int b) { init_arg0=a; init_arg1=b; }
} CouncilWindow;
struct Console { int faction_id_; } ConsoleGlobal;
struct Sprite {};
struct NetMessage {
    int count; char last[256];
    void pop(const char *s,int,int,int) { ++count; strncpy(last,s,255); last[255]=0; }
} NetMsgPrimary;
static Player Players[8]; static PlayerData PlayersData[8];
static RulesProposal Proposal[11];
static int (&CouncilResolutionCount)[11] = GameDataStateStorage.fields_.council_resolution_count_;
static int (&CouncilLastResolutionTurn)[11] = GameDataStateStorage.fields_.council_last_resolution_turn_;
static Sprite *FactionDiplomacyPortraits[8];
static char StringTemp[256]; static char *ScriptPopupFile=(char*)"SCRIPT";
static int &GovernorFaction = GameDataStateStorage.fields_.governor_faction_;
static int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
static uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
static int CouncilPbemGate;
static int IsMultiplayerPBEM, IsMultiplayerNet;
static int &SunspotDuration = GameDataStateStorage.fields_.sunspot_duration_;
static int ExpansionEnabled;
static uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
static int &DiffLevelCurrent = GameDataStateStorage.fields_.diff_level_current_;
static int GenderDefault, PluralityDefault;
static uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
static int CauseFrictionMode, GreatSatanMode, ParseCount, ParseNumOrder;
static char ParseKind[32]; static int ParseSlot[32], ParseGender[32], ParsePlural[32];
static const char *ParseText[32]; static int ParseValue[32];
static int TreatyCalls[8][8], MajorAtrocityCalls;
static int __cdecl treaty_on(int a,int b,int bits) {
    ++TreatyCalls[a][b]; PlayersData[a].diplo_treaties[b] |= (uint32_t)bits; return 0;
}
static int __cdecl is_alien_faction(int f) { return (Players[f].rule_flags&RFLAG_ALIEN)!=0; }
static int __cdecl parse_says(int slot,const char *text,int,int) {
    int i=ParseCount++; ParseKind[i]='S'; ParseSlot[i]=slot; ParseText[i]=text;
    ParseGender[i]=GenderDefault; ParsePlural[i]=PluralityDefault; return 0;
}
static int __cdecl parse_say(int slot,char *text,int,int) {
    int i=ParseCount++; ParseKind[i]='s'; ParseSlot[i]=slot; ParseValue[i]=(int)(intptr_t)text;
    ParseGender[i]=GenderDefault; ParsePlural[i]=PluralityDefault; return 0;
}
static int __cdecl popp(const char *,const char *,int,const char *,int(__cdecl *)()) { return 0; }
static int __cdecl cause_friction(int a,int b,int) {
    if(CauseFrictionMode==1) PlayersData[a].diplo_treaties[b]|=DTREATY_VENDETTA;
    if(CauseFrictionMode==2) PlayersData[a].diplo_treaties[b]|=DTREATY_WANT_REVENGE;
    return 0;
}
static int __cdecl great_satan(int,int) { return GreatSatanMode; }
static int __cdecl diplomacy_caption(int,int) { return 0; }
static int __cdecl x_pops_minimal(const char *,Sprite *,int) { return 0; }
static int __cdecl parse_num(int slot,int value) {
    ParseNumOrder=ParseCount; int i=ParseCount++; ParseKind[i]='N';
    ParseSlot[i]=slot; ParseValue[i]=value; return 0;
}
static int __cdecl major_atrocity(int,int) { ++MajorAtrocityCalls; return 0; }
int __cdecl rand(void) { return 1; }
'''


def compile_run(tmp_path, bodies, main, compiler):
    executable = shutil.which(compiler)
    if executable is None: pytest.skip(compiler + " is unavailable")
    source = tmp_path / (compiler + ".cpp")
    binary = tmp_path / compiler
    fixture = FIXTURE.replace("@@GAME_DATA_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
    source.write_text(fixture + "\n" + "\n".join(bodies) +
                      "\nint main(){\n" + main + "\n}\n")
    subprocess.run([executable, "-std=c++98", "-w", str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_council_product_source_parser_order_and_owner_arguments(tmp_path, compiler):
    helper, signed, council, atrocity = product_definitions()
    main = r'''
        TurnCurrentNum=17; GovernorFaction=4; ConsoleGlobal.faction_id_=0;
        IsMultiplayerPBEM=0; IsMultiplayerNet=0; CouncilPbemGate=0;
        Proposal[1].name=(char*)"VOTE"; Players[2].is_leader_female=1;
        Players[2].title_leader=(char*)"TITLE"; Players[2].name_leader=(char*)"LEADER";
        council(2,1,3);
        assert(PlayersData[3].council_call_turn==17 && CouncilLastResolutionTurn[1]==17);
        assert(CouncilWindow.init_arg0==1 && CouncilWindow.init_arg1==3);
        assert((GameState&0x10000)!=0 && ParseCount==3);
        assert(ParseKind[0]=='S'&&ParseSlot[0]==0&&ParseText[0]==Players[2].title_leader);
        assert(ParseKind[1]=='S'&&ParseSlot[1]==1&&ParseText[1]==Players[2].name_leader);
        assert(ParseKind[2]=='s'&&ParseSlot[2]==2&&ParseValue[2]==(int)(intptr_t)Proposal[1].name);
        assert(ParseGender[0]==1&&ParsePlural[0]==FALSE&&ParseGender[1]==1&&ParsePlural[1]==FALSE);
        assert(ParseGender[2]==1&&ParsePlural[2]==FALSE);
        ParseCount=0; IsMultiplayerPBEM=1; CouncilPbemGate=2;
        CouncilWindow.field_A1C_=5; CouncilWindow.field_A20_=6;
        ConsoleGlobal.faction_id_=2; LockEnableMask=1u<<2;
        council(7,3,4);
        assert(CouncilWindow.init_arg0==5 && CouncilWindow.init_arg1==6);
    '''
    result = compile_run(tmp_path, [helper,signed,council,atrocity], main, compiler)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_atrocity_product_source_parser_trace_and_wrapping(tmp_path, compiler):
    helper, signed, council, atrocity = product_definitions()
    main = r'''
        CouncilResolutionCount[8]=CouncilResolutionCount[9]=0;
        SunspotDuration=0; ExpansionEnabled=0; ConsoleGlobal.faction_id_=0;
        PlayersData[2].diff_level=7; PlayersData[2].atrocities=1;
        PlayersData[2].sanction_turns=4; Players[2].is_leader_female=1;
        Players[2].title_leader=(char*)"ACTOR_TITLE"; Players[2].name_leader=(char*)"ACTOR_NAME";
        Players[1].is_leader_female=0; Players[1].title_leader=(char*)"VICTIM_TITLE";
        Players[1].name_leader=(char*)"VICTIM_NAME";
        atrocity(2,1,1,0);
        assert(PlayersData[2].atrocities==2 && ParseCount==5 && ParseNumOrder==4);
        assert(ParseKind[0]=='S'&&ParseSlot[0]==0&&ParseText[0]==Players[2].title_leader);
        assert(ParseKind[1]=='S'&&ParseSlot[1]==1&&ParseText[1]==Players[2].name_leader);
        assert(ParseKind[2]=='S'&&ParseSlot[2]==2&&ParseText[2]==Players[1].title_leader);
        assert(ParseKind[3]=='S'&&ParseSlot[3]==3&&ParseText[3]==Players[1].name_leader);
        assert(ParseKind[4]=='N'&&ParseSlot[4]==0&&ParseValue[4]==20);
        assert(ParseGender[0]==1&&ParsePlural[0]==FALSE&&ParseGender[1]==1&&ParsePlural[1]==FALSE);
        assert(ParseGender[2]==0&&ParsePlural[2]==FALSE&&ParseGender[3]==0&&ParsePlural[3]==FALSE);
        assert(GenderDefault==0&&PluralityDefault==FALSE);
        PlayersData[2].atrocities=INT_MAX; PlayersData[2].sanction_turns=INT_MAX;
        ParseCount=0; atrocity(2,1,1,0);
        assert(PlayersData[2].atrocities==INT_MIN);
        // The native shift-left computes INT_MIN * 10 modulo 2^32 as zero.
        assert(PlayersData[2].sanction_turns==INT_MAX);
    '''
    result=compile_run(tmp_path,[helper,signed,council,atrocity],main,compiler)
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_atrocity_product_source_directed_rows_and_live_reload(tmp_path, compiler):
    helper, signed, council, atrocity = product_definitions()
    main = r'''
        CouncilResolutionCount[8]=CouncilResolutionCount[9]=0;
        SunspotDuration=0; ExpansionEnabled=0; GameRules=0x40;
        ConsoleGlobal.faction_id_=0; LockEnableMask=1u<<2; CauseFrictionMode=1;
        PlayersData[3].diplo_treaties[2]=DTREATY_COMMLINK|DTREATY_UNK_200000;
        PlayersData[1].diplo_treaties[2]=0x1234;
        atrocity(2,1,1,0);
        assert((PlayersData[1].diplo_treaties[2]&0x40020)==0x40020);
        assert((PlayersData[1].diplo_agenda[2]&DAGENDA_UNK_4)!=0);
        assert(PlayersData[1].diplo_treaties[2] != PlayersData[2].diplo_treaties[1]);
        assert((PlayersData[3].diplo_treaties[2]&DTREATY_VENDETTA)!=0);
        assert((PlayersData[3].diplo_treaties[2]&DTREATY_UNK_40)==0);
        assert(PlayersData[3].diplo_merc[2]==0);
        assert((PlayersData[2].diplo_treaties[3]&DTREATY_VENDETTA)==0);
        CauseFrictionMode=2; PlayersData[1].diplo_treaties[2]=0;
        PlayersData[3].diplo_treaties[2]=DTREATY_COMMLINK|DTREATY_UNK_200000;
        atrocity(2,1,1,0);
        assert((PlayersData[3].diplo_treaties[2]&DTREATY_UNK_40)!=0);
        assert(PlayersData[3].diplo_merc[2]==0x32);
        assert((PlayersData[2].diplo_treaties[3]&DTREATY_VENDETTA)!=0);
    '''
    result=compile_run(tmp_path,[helper,signed,council,atrocity],main,compiler)
    assert result.returncode==0,result.stderr


def test_product_source_has_original_function_identities_and_spans():
    source=FACTION_CPP.read_text()
    for identity in (
        "0x00428350 ?council@@YAXHHH@Z 0x00428350-0x0042854E",
        "0x0055EEE0 ?atrocity@@YAXHHHH@Z 0x0055EEE0-0x0055F44F",
    ):
        assert identity in source
