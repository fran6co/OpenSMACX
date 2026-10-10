"""Actual diplomacy bodies with service doubles, not GUI/game acceptance."""
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition

import re

_GAME_DATA_STATE_STUB = '\n// Test-only subset of the actual nested source API; no global aliases or game ABI claim.\ntypedef unsigned int uint32_t;\nstruct GameDataLockMaskBytes { unsigned char human_mask_, alive_mask_, unassigned_[2]; };\nunion GameDataLockMaskView { uint32_t enable_mask_; GameDataLockMaskBytes bytes_; };\nstruct BaseSecretProject {\n    int project_base[64];\n    int &empath_guild() { return project_base[4]; }\n    const int &empath_guild() const { return project_base[4]; }\n};\nstruct GameDataStateWindowFields {\n    uint32_t game_rules_, game_state_;\n    int base_current_count_, turn_current_num_;\n    GameDataLockMaskView lock_masks_;\n    uint32_t faction_rankings_[8], ranking_faction_id_unk1_, ranking_faction_id_unk2_;\n    BaseSecretProject secret_project_;\n    unsigned char game_tech_achieved_[89];\n    int council_resolution_count_[11];\n};\nstruct GameDataStateWindow { GameDataStateWindowFields fields_; };\nGameDataStateWindow GameDataStateStorage;\n'
_OLD_GAME_DATA_GLOBAL_NAMES = ('GameRules', 'GameState', 'BaseCurrentCount', 'LockEnableMask', 'RankingFactionIDUnk1', 'RankingFactionIDUnk2', 'CouncilResolutionCount', 'TurnCurrentNum', 'GameTechAchieved', 'FactionRankings')

def _strip_old_storage_declarations(source: str) -> str:
    output = []
    decl = re.compile(r"^(\s*(?:unsigned(?:\s+char)?|uint32_t|int)\s+)(.*)$")
    for line in source.splitlines(keepends=True):
        raw = line.rstrip("\n")
        if not any(re.search(r"\b" + re.escape(name) + r"\b", raw)
                   for name in _OLD_GAME_DATA_GLOBAL_NAMES):
            output.append(line)
            continue
        segments = raw.split(";")
        if len(segments) == 1:
            output.append(line)
            continue
        first_decl = decl.match(segments[0])
        if (not first_decl or "=" in first_decl.group(2)
                or "(" in first_decl.group(2)):
            output.append(line)
            continue
        rewritten = []
        for segment in segments[:-1]:
            match = decl.match(segment)
            if (not match or "=" in match.group(2) or "(" in match.group(2)
                    or not any(re.search(r"\b" + re.escape(name) + r"\b", match.group(2))
                               for name in _OLD_GAME_DATA_GLOBAL_NAMES)):
                rewritten.append(segment)
                continue
            kept = []
            for item in match.group(2).split(","):
                name_match = re.match(r"\s*([A-Za-z_]\w*)", item)
                if name_match and name_match.group(1) in _OLD_GAME_DATA_GLOBAL_NAMES:
                    continue
                kept.append(item.strip())
            if kept:
                rewritten.append(match.group(1) + ", ".join(kept))
        tail = segments[-1]
        rebuilt = ";".join(rewritten) + tail
        if raw.endswith(";") and rebuilt.strip():
            rebuilt += ";"
        if rebuilt.strip():
            output.append(rebuilt + ("\n" if line.endswith("\n") else ""))
    return "".join(output)


def host_fixture(source: str) -> str:
    """Add a narrow typed owner and rewrite fixture setup to the owner fields."""
    if "GameDataStateStorage" in source or "struct GameDataStateWindowFields" in source:
        raise RuntimeError("fixture already has a game-data owner; review manually")
    source = _strip_old_storage_declarations(source)
    source, project_count = re.subn(
        r"struct Projects\s*\{int empath_guild_;.*?\}\s*SecretProject;", "", source,
        flags=re.S,
    )
    if project_count:
        source = source.replace(
            "SecretProject.empath_guild()=-1;",
            "GameDataStateStorage.fields_.secret_project_.empath_guild()=-1;",
        )

    replacements = {
        "GameRules": "GameDataStateStorage.fields_.game_rules_",
        "GameState": "GameDataStateStorage.fields_.game_state_",
        "BaseCurrentCount": "GameDataStateStorage.fields_.base_current_count_",
        "LockEnableMask": "GameDataStateStorage.fields_.lock_masks_.enable_mask_",
        "RankingFactionIDUnk1": "GameDataStateStorage.fields_.ranking_faction_id_unk1_",
        "RankingFactionIDUnk2": "GameDataStateStorage.fields_.ranking_faction_id_unk2_",
        "CouncilResolutionCount": "GameDataStateStorage.fields_.council_resolution_count_",
        "TurnCurrentNum": "GameDataStateStorage.fields_.turn_current_num_",
        "GameTechAchieved": "GameDataStateStorage.fields_.game_tech_achieved_",
        "FactionRankings": "GameDataStateStorage.fields_.faction_rankings_",
    }
    for old, new in replacements.items():
        source = re.sub(r"\b" + re.escape(old) + r"\b", new, source)

    typedef = "typedef unsigned int uint32_t;"
    if typedef in source:
        source = source.replace(typedef, typedef + "\n" + _GAME_DATA_STATE_STUB, 1)
    elif "#define __cdecl" in source:
        source = source.replace("#define __cdecl", "#define __cdecl\n" + _GAME_DATA_STATE_STUB, 1)
    else:
        raise RuntimeError("cannot locate fixture host-declaration insertion anchor")
    reset_re = re.compile(r"(\bvoid\s+(?:reset|reset_fixture)\s*\([^)]*\)\s*\{)")
    source, reset_count = reset_re.subn(
        r"\1\n    GameDataStateStorage = GameDataStateWindow();", source,
    )
    if reset_count == 0:
        raise RuntimeError("no fixture reset function found")
    return source




@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_tech_trade_gate_and_owner(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    enum_start = source.index('enum TechTradeTailEntry {')
    enum_end = source.index('};', enum_start) + 2
    gate = source[enum_start:enum_end] + '\n' + definition(
        source, r'static TechTradeTailEntry\s+tech_trade_ui_gate\s*\(')
    fixture = r'''
#include <cassert>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
struct Data {
 unsigned diplo_treaties[8],traded_maps,flags,current_num_bases,ranking;
 int diplo_agenda[8],ai_power,ai_tech,ai_fight,tech_ranking,integrity_blemishes;
} PlayersData[8];
struct Base {int faction_id_current;unsigned visibility;} Bases[8];
unsigned GameRules,RankingFactionIDUnk1,RankingFactionIDUnk2;
int FactionRankings[8],FactionRankingsUnk[8],BaseCurrentCount;
int DiploScratchFAA8,DiploScratchF800,DiploScratchFA48,DiploScratchFA3C;
int DiploScratchF7F0,DiploFriction,IsMultiplayerNet,willing,parsed,wants;
int diplo_add(int a,int b){return (int)((unsigned)a+(unsigned)b);}
int diplo_sub(int a,int b){return (int)((unsigned)a-(unsigned)b);}
int diplo_mul(int a,int b){return (int)((unsigned)a*(unsigned)b);}
int wants_to_speak(int a,int b){assert(a==1&&b==2);++wants;return willing;}
void tech_trade_parse_initial_offer_lines(){++parsed;}
// @@GATE@@
void reset(){
 std::memset(PlayersData,0,sizeof(PlayersData));std::memset(Bases,0,sizeof(Bases));
 std::memset(FactionRankings,0,sizeof(FactionRankings));
 std::memset(FactionRankingsUnk,0,sizeof(FactionRankingsUnk));
 DiploScratchFAA8=10;DiploScratchF800=20;DiploScratchFA48=DiploScratchFA3C=0;
 DiploFriction=IsMultiplayerNet=GameRules=DiploScratchF7F0=0;
 RankingFactionIDUnk1=RankingFactionIDUnk2=0;BaseCurrentCount=0;
 willing=1;parsed=wants=0;
}
int route(int context=0,int action=4){return (int)tech_trade_ui_gate(1,2,0,action,context);}
void controls(){
 reset();assert(route()==-1&&wants==1&&!parsed);
 reset();willing=0;assert(route()==TECH_TRADE_TAIL_NO_SPEAK_542E07&&wants==1);
 reset();assert(route(13)==TECH_TRADE_TAIL_COMMLINK_TECH_542E80&&parsed==1&&wants==1);
 reset();PlayersData[2].diplo_treaties[1]=1;
 assert(route(13)==TECH_TRADE_TAIL_COMMLINK_FREE_54295F&&!wants&&!parsed);
 reset();PlayersData[2].diplo_treaties[1]=0x02000001;
 assert(route()==TECH_TRADE_TAIL_TECH_SLAVE_542A78&&!wants);
 reset();PlayersData[2].flags=0x02000001;assert(route()==-1);
 reset();PlayersData[2].diplo_treaties[1]=0x20;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7&&!wants);
 reset();PlayersData[2].diplo_agenda[1]=0x200;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7);
 reset();PlayersData[2].diplo_agenda[0]=0x200;assert(route()==-1);
 reset();PlayersData[1].integrity_blemishes=4;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7);
 reset();PlayersData[2].integrity_blemishes=4;assert(route()==-1);
 reset();IsMultiplayerNet=1;PlayersData[1].diplo_agenda[2]=0x100;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7);
 reset();IsMultiplayerNet=1;PlayersData[1].diplo_agenda[0]=0x100;assert(route()==-1);
 reset();DiploScratchF800=-1;PlayersData[1].flags=0x200;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7);
 reset();DiploScratchF800=-1;PlayersData[2].flags=0x200;assert(route()==-1);
 reset();DiploScratchFAA8=-1;PlayersData[2].diplo_treaties[1]=1;
 PlayersData[1].ranking=1;PlayersData[2].ranking=2;PlayersData[2].tech_ranking=10;
 assert(route()==TECH_TRADE_TAIL_TECH_FREEBIE_542BE7);
 reset();DiploScratchFAA8=-1;PlayersData[2].diplo_treaties[1]=1;
 PlayersData[1].ranking=1;PlayersData[2].ranking=2;PlayersData[2].tech_ranking=10;
 IsMultiplayerNet=1;RankingFactionIDUnk1=1;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7);
 reset();DiploScratchFAA8=-1;DiploScratchF800=-1;PlayersData[1].current_num_bases=1;
 assert(route()==-1&&DiploScratchFAA8==9999);
 reset();DiploScratchFAA8=-1;DiploScratchF800=-1;PlayersData[2].traded_maps=2;
 PlayersData[1].current_num_bases=1;
 assert(route()==TECH_TRADE_TAIL_BUY_RESULT_5437B7&&DiploScratchFAA8==-1);
}
struct Popup{~Popup();};
enum TechTradePolicyRoute{TECH_TRADE_POLICY_REJECT_542097,
 TECH_TRADE_POLICY_REJECT_54222B,TECH_TRADE_POLICY_READY_FOR_OFFERS};
struct TechTradePolicyDecision{TechTradePolicyRoute route;};
int prefixes,phases,destructions,first_flags[4],policy_mode;
TechTradePolicyDecision tech_trade_policy_prefix(Popup&,int,int,int,int,int,bool first){
 first_flags[prefixes++]=first;TechTradePolicyDecision p;
 p.route=policy_mode?TECH_TRADE_POLICY_REJECT_542097:TECH_TRADE_POLICY_READY_FOR_OFFERS;return p;
}
void tech_trade_handle_pre_offer_rejection(Popup&,TechTradePolicyDecision,int,int,int,int,int){assert(false);}
TechTradeTailEntry tech_trade_ui_phase(Popup&,int,int,int,int,int){
 return ++phases==1?(TechTradeTailEntry)-1:TECH_TRADE_TAIL_SUCCESS_5433A0;
}
int tech_trade_run_tail(TechTradeTailEntry t,int,int,int,int,int){assert(t==TECH_TRADE_TAIL_SUCCESS_5433A0);return 1;}
Popup::~Popup(){++destructions;}
// @@OWNER@@
int main(){
 controls();assert(tech_trade(1,2,0,4,0)==1);
 assert(prefixes==2&&phases==2&&first_flags[0]&&!first_flags[1]&&destructions==1);
 policy_mode=1;assert(tech_trade(1,2,0,4,0)==0&&destructions==2);
}
'''
    owner = definition(source, r'int __cdecl tech_trade\s*\(')
    variants = {'actual': gate}
    for name, old, new in (
        ('wrong_agenda_cell', 'diplo_agenda[actor]', 'diplo_agenda[0]'),
        ('wrong_blemish_owner', 'PlayersData[actor].integrity_blemishes',
         'PlayersData[partner].integrity_blemishes'),
        ('missing_mode_parser', 'tech_trade_parse_initial_offer_lines();', ''),
    ):
        assert old in gate
        variants[name] = gate.replace(old, new)
    for name, actual in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('// @@GATE@@', actual).replace('// @@OWNER@@', owner))
        built = subprocess.run([compiler, '-std=c++98', '-pedantic-errors',
                                '-fsanitize=undefined', '-fno-sanitize-recover=all',
                                str(cpp), '-o', str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True)
        assert (ran.returncode == 0) == (name == 'actual'), ran.stderr


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_proposal_menu(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    body = definition((ROOT / 'src/faction.cpp').read_text(),
                      r'int __cdecl proposal_menu\([^)]*\)')
    fixture = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
#define nullptr 0
typedef unsigned int uint32_t;
const int MaxPlayerNum=8;
struct Sprite {} portraits[8];Sprite *FactionDiplomacyPortraits[8];
struct Player {int is_leader_female,noun_gender,is_noun_plural;
 char title_leader[24],name_leader[24],noun_faction[24];} Players[8];
struct Data {uint32_t diplo_treaties[8],diplo_agenda[8],current_num_bases;
 int loan_balance[8],energy_reserves;} PlayersData[8];
struct Base {int faction_id_current;unsigned char visibility,facilities_built[16];short x,y;} Bases[3];
struct Projects {int empath_guild_;
 int &empath_guild() { return empath_guild_; }
 const int &empath_guild() const { return empath_guild_; }
} SecretProject;
int BaseCurrentCount,DiploScratchFA30,DiploScratchFA7C,PopupDialogFactionID;
int DiplomacyProposalMode,DiploScratchFA34,DiploScratchFA5C;
unsigned LockEnableMask,GameRules;int IsMultiplayerNet,GenderDefault,PluralityDefault;
char StringTemp[128],PopupStartCaption[128],ScriptPopupFile[128],TextBufferGetPtr[128];
int answers[8],starts,execs,destructors,callbacks,shows,closes,lines,base_named;
int item_ids[8][20],item_count[8],text_lines;char keys[8][80],notice[80],fail_key[80];
int pop_wait(){return 0;}void battle_routine(int){}
void bitmask(int n,int *offset,int *mask){assert(n==1);*offset=0;*mask=2;}
void parse_num(int slot,int amount){assert(!slot&&amount==PlayersData[1].loan_balance[2]);}
void diplomacy_caption(int a,int b){assert(a==1&&b==2);}
int parse_says(int slot,char *s,int a,int b){assert(slot>=0&&slot<=2&&s&&a==-1&&b==-1);return 0;}
char *get_pact_hood(int a,int b){assert(a==1&&b==2);return Players[1].noun_faction;}
int X_text_open(char *,const char *key){return std::strcmp(fail_key,key)==0?1:0;}
void text_get(){++text_lines;std::strcpy(TextBufferGetPtr,"line");}
void parse_string(char *s,char *out){assert(s==TextBufferGetPtr&&out==StringTemp&&!*out);std::strcpy(out,s);}
char *_itoa(int n,char *out,int base){assert(base==10);std::sprintf(out,"%d",n);return out;}
void say_base(char *s,int base){assert(s==StringTemp&&!*s&&base>=0&&base<BaseCurrentCount);base_named=base;std::strcpy(s,"base");}
struct ListBox {void set_selection_callback(void(*cb)(int)){assert(cb==battle_routine);++callbacks;}};
struct Dialogs:ListBox {int stage;
 void sprite(Sprite *s,char *text,int n){assert(s==FactionDiplomacyPortraits[2]&&!text&&!n);}
 void item(char *s,int id){assert(s==StringTemp&&*s&&item_count[stage]<20);item_ids[stage][item_count[stage]++]=id;}
};
struct Popup {Dialogs d;Popup(){d.stage=0;}~Popup(){++destructors;}
 int start(char *s,const char *key,int n,char *text,int flags,int owner){assert(s==PopupStartCaption&&n==-1&&!text&&!owner&&(flags==0x100000||flags==0x100040));
 assert(starts<8);d.stage=starts;std::strcpy(keys[starts++],key);return 0;}
 bool picture_enabled(){return false;}void set_caller_picture(Sprite *s){assert(s==FactionDiplomacyPortraits[2]);}
 Dialogs &dialogs(){return d;}int exec(int n,int(*cb)()){assert(!n&&cb==pop_wait&&execs<8);return answers[execs++];}
};
struct Plan {void init(char *s){assert(!s);}void back_redraw(){}void clear_lines(){}
 void monocenter(int,int,int n){assert(!n);}void add_line(int a,int b,int c){assert(a==b&&!c);++lines;}
 void on_redraw(){}void show(int n){assert(!n);++shows;}void close(){++closes;}} PlanWindow;
int X_pops(const char *key,int flags,Sprite *s,int(*cb)()){assert(flags==0x100000&&s==FactionDiplomacyPortraits[2]&&cb==pop_wait);std::strcpy(notice,key);return 0;}
int x_pops_minimal(const char *key,Sprite *s,int(*cb)()){assert(s==FactionDiplomacyPortraits[2]&&cb==pop_wait);std::strcpy(notice,key);return 0;}
@@BODY@@
void reset(){
 std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));std::memset(Bases,0,sizeof(Bases));
 for(int i=0;i<8;++i){FactionDiplomacyPortraits[i]=&portraits[i];answers[i]=-1;}
 BaseCurrentCount=3;Bases[0].faction_id_current=2;Bases[0].visibility=2;
 Bases[1].faction_id_current=1;Bases[2].faction_id_current=1;
 PlayersData[1].current_num_bases=2;PlayersData[2].current_num_bases=1;
 LockEnableMask=0xfe00;GameRules=0;IsMultiplayerNet=0;SecretProject.empath_guild()=-1;
 DiploScratchFA30=DiploScratchFA7C=99;DiplomacyProposalMode=DiploScratchFA34=DiploScratchFA5C=99;
 starts=execs=destructors=callbacks=shows=closes=lines=text_lines=0;base_named=-1;
 std::memset(item_count,0,sizeof(item_count));notice[0]=fail_key[0]=0;
}
bool has(int stage,int id){for(int i=0;i<item_count[stage];++i)if(item_ids[stage][i]==id)return true;return false;}
int main(){
 reset();answers[0]=7;assert(proposal_menu(1,2)==7&&DiploScratchFA34==7&&DiplomacyProposalMode==1&&destructors==1);
 assert(PopupDialogFactionID==2&&has(0,13)&&has(0,8)&&has(0,9)&&!has(0,2)&&has(0,3));
 reset();answers[0]=2;answers[1]=4;assert(proposal_menu(1,2)==2&&DiplomacyProposalMode==4&&execs==2);
 assert(!std::strcmp(keys[1],"COUNTER0")&&has(1,1)&&has(1,2)&&has(1,3));
 reset();answers[0]=2;answers[1]=0;assert(proposal_menu(1,2)==0&&DiplomacyProposalMode==0);
 reset();answers[0]=1;answers[1]=7;assert(proposal_menu(1,2)==1&&DiplomacyProposalMode==7&&!std::strcmp(keys[1],"COUNTER1"));
 assert(!has(1,1)&&!has(1,2)&&has(1,7));
 reset();std::strcpy(fail_key,"PROPOSALMENU");assert(proposal_menu(1,2)==0&&destructors==1&&!execs);
 reset();answers[0]=8;answers[1]=-1;assert(proposal_menu(1,2)==0&&DiplomacyProposalMode==1&&execs==2);
 reset();answers[0]=8;answers[1]=3;answers[2]=4;PlayersData[1].diplo_treaties[3]=8;
 assert(proposal_menu(1,2)==8&&DiploScratchFA5C==3&&DiplomacyProposalMode==4&&has(1,3));
 reset();answers[0]=8;answers[1]=3;PlayersData[2].diplo_agenda[0]=16;
 assert(proposal_menu(1,2)==0&&!std::strcmp(notice,"REJWARALREADY")&&DiplomacyProposalMode==1);
 reset();answers[0]=13;answers[1]=3;assert(proposal_menu(1,2)==0&&!std::strcmp(notice,"NOCOMMLINK")&&DiplomacyProposalMode==1);
 reset();answers[0]=13;answers[1]=3;answers[2]=4;PlayersData[2].diplo_treaties[3]=8;
 assert(proposal_menu(1,2)==13&&DiplomacyProposalMode==4&&has(1,3));
 reset();answers[0]=9;answers[1]=0;answers[2]=8;answers[3]=2;
 assert(proposal_menu(1,2)==9&&DiploScratchFA7C==0&&DiploScratchFA30==2&&DiplomacyProposalMode==8);
 assert(callbacks==2&&shows==2&&closes==2&&has(1,0)&&has(3,1)&&has(3,2));
 reset();answers[0]=9;answers[1]=0;answers[2]=8;answers[3]=-1;Bases[1].facilities_built[0]=2;Bases[2].facilities_built[0]=2;
 assert(proposal_menu(1,2)==0&&execs==4&&shows==2&&closes==2&&!item_count[3]&&callbacks==2);
 reset();answers[0]=9;Bases[0].facilities_built[0]=2;assert(proposal_menu(1,2)==0&&DiploScratchFA7C==-1&&closes==1);
 reset();answers[0]=7;Bases[0].visibility=0;assert(proposal_menu(1,2)==7&&!has(0,13));
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_proposal_cell', 'DiploScratchFA34 = popup.exec(0, pop_wait)',
         'DiplomacyProposalMode = popup.exec(0, pop_wait)'),
        ('missing_selection_notification', 'set_selection_callback(battle_routine)',
         'set_selection_callback(0)'),
        ('wrong_attack_relation', 'PlayersData[faction_id_with].diplo_agenda[0]',
         'PlayersData[DiploScratchFA5C].diplo_treaties[faction_id_with]'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', candidate))
        result = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                 '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                                capture_output=True, text=True, timeout=45)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_threaten(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'void __cdecl threaten\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul'))
    fixture = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t; typedef unsigned char uint8_t;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char name_leader[24],title_leader[24],noun_faction[24],adj_name_faction[24]; } Players[8];
struct Data { uint32_t ranking,diplo_treaties[8],diplo_friction[8],diplo_agenda[8];
 int diplo_spoke[8],tech_ranking,tech_commerce_bonus,ai_fight,ai_tech,ai_power,ai_wealth;
 int energy_reserves,current_num_bases,mil_strength_1,best_weapon_value,integrity_blemishes;
 char diplo_patience[8]; unsigned char region_total_bases[128]; } PlayersData[8];
struct Base { short x,y; unsigned char faction_id_former; char name_string[24]; } Bases[1];
struct Tile { unsigned char region; } MapTiles[1]; unsigned MapLongitude=1;
int PopupDialogFactionID,DiploScratchFA70,DiploScratchFA98,DiploScratchFA34,DiploScratchF7E4;
int DiploScratchFA5C,DiploScratchF800,DiploScratchFA7C,DiploScratchFA68,DiploScratchFAA0;
int DiploScratchFA3C,DiploScratchFA50,DiploFriction,IsMultiplayerNet,TurnCurrentNum;
unsigned DiploFrictionFactionID,DiploFrictionFactionIDWith;
int GenderDefault,PluralityDefault,ExitTurnLoop; char StringTemp[128],notice[128];
int treaty_calls,treaty_a,treaty_b,treaty_bits,packets,packet_op,payload[4],await_op;
int tech_calls,technology,redraws,base_calls,set_calls,pact_calls,cross_calls,locks,unlocks;
int base_value,wants,mutate;
int pop_wait() {return 0;}
void tech_analysis(int a,int b) {assert(a==1&&b==2);}
void diplomacy_caption(int a,int b) {assert(a==1&&b==2);}
int parse_says(int slot,char *s,int a,int b) {
 assert(slot>=0&&slot<=3&&s&&a==-1&&b==-1);
 if(DiploScratchFA34==13) assert(s==Players[DiploScratchFA5C].adj_name_faction);
 return 0;
}
void parse_num(int slot,int) {assert(!slot);}
char *get_noun_phrase(int who,int kind) {assert(who==1&&kind==8);return Players[1].noun_faction;}
char *get_pact(int who) {assert(who==1);return Players[1].noun_faction;}
int random(int a,int b) {assert(!a&&b==2);return 1;}
char *_itoa(int n,char *s,int base) {assert(base==10);std::sprintf(s,"%d",n);return s;}
int X_pops(const char *key,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000&&p==FactionDiplomacyPortraits[2]&&cb==pop_wait);std::strcpy(notice,key);
 if(mutate&&!std::strcmp(key,"EXTORTCOMMLINK")) DiploScratchFA5C=4;
 return 1;
}
void log_say(const char *s,int,int,int) {assert(s);}
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z);++packets;packet_op=op;payload[0]=a;payload[1]=b;payload[2]=c;payload[3]=d;
}
struct Net {void await_diplo(int op) {await_op=op;}int add_global(){++locks;return 0;}void unlock(){++unlocks;}} NetDaemonNet;
struct Console {void update_data(int n) {redraws+=n?1:10;}} ConsoleGlobal;
struct Info {void set_redraw_phase(int n) {assert(!n);}} ConsoleInfoWin;
void treaty_on(int a,int b,int bits) {++treaty_calls;treaty_a=a;treaty_b=b;treaty_bits=bits;}
void tech_achieved(int a,int tech,int b,int z) {assert(a==1&&b==2&&!z);++tech_calls;technology=tech;}
void say_tech(char *s,int tech,int kind) {assert(!*s&&tech==DiploScratchF800&&kind==1);std::strcpy(s,"tech");}
int wants_to_attack(int a,int b,int z) {assert((a==2&&b==3)||(a==3&&b==2));assert(!z);return wants;}
void double_cross(int a,int b,int c) {assert(a==2&&b==3&&c==1);++cross_calls;}
void agenda_on(int a,int b,int bits) {assert(a==1&&b==2&&bits==0x1000);}
int value_of_base(int base,int a,int b,int rating,int) {assert(!base&&a==2&&b==1&&rating==DiploScratchFA98);return base_value;}
void give_a_base(int base,int owner) {assert(!base&&owner==1);++base_calls;}
void set_treaty(int a,int b,int bits,int on) {assert(a==2&&b==1&&bits==0x20&&on==1);++set_calls;}
void pact_ends(int a,int b) {assert(a==1&&b==2);++pact_calls;}
unsigned long timeGetTime() {return 0;}void check_spock(){}
@@BODY@@
void reset() {
 std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
 std::memset(Bases,0,sizeof(Bases));MapTiles[0].region=1;
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 DiploScratchFA70=DiploScratchFA98=DiploScratchFA34=DiploScratchF7E4=0;
 DiploScratchFA5C=3;DiploScratchF800=-1;DiploScratchFA7C=0;
 DiploScratchFA68=DiploScratchFAA0=DiploScratchFA3C=DiploScratchFA50=0;
 DiploFriction=IsMultiplayerNet=GenderDefault=PluralityDefault=ExitTurnLoop=0;TurnCurrentNum=16;
 DiploFrictionFactionID=2;DiploFrictionFactionIDWith=1;
 PlayersData[1].energy_reserves=10;PlayersData[2].energy_reserves=100;
 PlayersData[1].mil_strength_1=100;PlayersData[2].mil_strength_1=100;
 treaty_calls=treaty_a=treaty_b=treaty_bits=packets=packet_op=await_op=0;
 tech_calls=technology=redraws=base_calls=set_calls=pact_calls=cross_calls=locks=unlocks=mutate=0;
 base_value=5;wants=1;notice[0]=0;
}
int main() {
 reset();threaten(1,2);assert(!std::strcmp(notice,"IGNORE")&&PlayersData[2].diplo_friction[1]==2&&DiploFriction==2);
 reset();DiploScratchFA70=1;threaten(1,2);assert(treaty_a==2&&treaty_b==1&&treaty_bits==16);
 assert(PlayersData[1].diplo_spoke[2]==16&&!std::strcmp(notice,"PROVOKED1"));
 reset();DiploScratchFA34=6;DiploScratchFA98=2;threaten(1,2);
 assert(PlayersData[1].energy_reserves==60&&PlayersData[2].energy_reserves==50&&treaty_bits==4);
 reset();DiploScratchFA34=6;DiploScratchFA98=2;PlayersData[1].diplo_treaties[2]=2;IsMultiplayerNet=1;threaten(1,2);
 assert(packet_op==0x2447&&payload[0]==1&&payload[1]==50&&payload[2]==2&&payload[3]==-50&&await_op==0x447);
 reset();DiploScratchFA34=6;PlayersData[1].diplo_treaties[2]=1;PlayersData[1].ranking=2;PlayersData[2].ranking=6;
 PlayersData[2].tech_ranking=20;PlayersData[1].tech_ranking=4;
 PlayersData[1].tech_commerce_bonus=1;PlayersData[2].tech_commerce_bonus=4;PlayersData[2].ai_fight=1;
 PlayersData[2].diplo_friction[1]=99;threaten(1,2);assert(DiploScratchFA98==14&&PlayersData[1].energy_reserves==110&&PlayersData[2].energy_reserves==0);
 reset();DiploScratchFA34=4;DiploScratchFA98=1;DiploScratchF800=8;threaten(1,2);
 assert(tech_calls==1&&technology==8&&DiploScratchF800==-1);
 reset();DiploScratchFA34=13;DiploScratchFA98=1;mutate=1;threaten(1,2);assert(technology==93&&tech_calls==1);
 reset();DiploScratchFA34=9;DiploScratchFA98=1;threaten(1,2);
 assert(base_calls==1&&set_calls==1&&locks==1&&unlocks==1&&treaty_bits==4);
 reset();DiploScratchFA34=9;DiploScratchFA98=1;PlayersData[2].diplo_treaties[1]=1;
 threaten(1,2);assert(!base_calls&&!std::strcmp(notice,"BASENOCEDE"));
 reset();DiploScratchFA34=9;DiploScratchFA98=1;PlayersData[2].diplo_treaties[1]=0x2000001;
 PlayersData[2].current_num_bases=10;PlayersData[1].diplo_agenda[2]=0x1000;Bases[0].faction_id_former=3;
 PlayersData[1].region_total_bases[1]=1;PlayersData[2].region_total_bases[1]=3;
 threaten(1,2);assert(!base_calls&&!std::strcmp(notice,"BASENOCEDE"));
 reset();DiploScratchFA34=9;DiploScratchFA98=1;PlayersData[2].diplo_treaties[1]=0x2000001;
 PlayersData[2].current_num_bases=10;PlayersData[1].diplo_agenda[2]=0x1000;Bases[0].faction_id_former=3;
 PlayersData[1].region_total_bases[1]=4;PlayersData[2].region_total_bases[1]=3;
 threaten(1,2);assert(base_calls==1&&!set_calls);
 reset();DiploScratchFA34=8;PlayersData[1].diplo_treaties[2]=1;PlayersData[1].diplo_treaties[3]=16;
 threaten(1,2);assert(cross_calls==1&&!std::strcmp(notice,"WARPACT"));
 reset();PlayersData[2].diplo_treaties[1]=1;threaten(1,2);assert(!std::strcmp(notice,"FEND"));
 reset();PlayersData[2].diplo_treaties[1]=1;DiploScratchFA68=1;PlayersData[1].ranking=6;
 threaten(1,2);assert(pact_calls==1&&locks==1&&unlocks==1&&!std::strcmp(notice,"FEND"));
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_score_owner', 'const int actor_rank = static_cast<int>(PlayersData[faction_id].ranking)',
         'const int actor_rank = static_cast<int>(PlayersData[other_faction_id].diplo_friction[faction_id])'),
        ('wrong_region_index', 'region_total_bases[region]',
         'region_total_bases[region / 2]'),
        ('stale_commlink', 'const int technology = diplo_add(DiploScratchFA5C, 0x59)',
         'const int technology = diplo_add(3, 0x59)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        result = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                 '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                                capture_output=True, text=True, timeout=45)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_propose_treaty(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'int __cdecl propose_treaty\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul', 'diplo_clamp'))
    fixture = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char filename[24],name_leader[24],title_leader[24],noun_faction[24],adj_name_faction[24],insult_leader[24]; } Players[8];
struct Data { uint32_t ranking,flags,diplo_treaties[8],diplo_agenda[8],diplo_friction[8];
 int mil_strength_1,tech_commerce_bonus,diff_level,energy_reserves; char diplo_patience[8]; } PlayersData[8];
int PopupDialogFactionID,DiplomacyProposalGateCount,DiplomacyProposalMode;
int DiploScratchF7E8,DiploScratchFA8C,DiploScratchFA3C,DiploScratchF7FC;
int DiploScratchFA54,DiploScratchFA78,DiploScratchFA2C,DiploScratchFA98;
int DiploScratchFAA8,DiploScratchFA18,DiploScratchFA4C;
int DiploFriction,TurnCurrentNum,GenderDefault,PluralityDefault,IsMultiplayerNet,ExitTurnLoop;
unsigned DiploFrictionFactionID,DiploFrictionFactionIDWith,FactionRankings[8];
char StringTemp[128],notice[128];
const char *DiploLabelSendingTECH="tech",*DiploLabelGettingDIPLOMATICLOCK="lock";
const char *DiploLabelSendingTREATYOFF="off",*DiploLabelSendingDOUBLECROSS="cross";
const char *DiploLabelReleasingDIPLOMATICLOCK="unlock";
int response,help_response,help_once,helps,technology_calls,money_packets,tech_packets;
int money_amount,awaits,last_await,settlements,offs,crosses,locks,unlocks,redraws;
int pop_wait() { return 0; }
int parse_says(int slot,char *text,int a,int b) { assert(slot>=0&&slot<=6&&text&&a==-1&&b==-1); return 0; }
void parse_num(int slot,int amount) { assert(!slot); money_amount=amount; }
char *get_noun_phrase(int who,int kind) { assert(who==2&&kind==9); return Players[2].noun_faction; }
char *get_phrase(int who,int kind) { assert(who==1&&kind==3); return Players[1].adj_name_faction; }
int tech_val(int tech,int who,int simple) { assert(tech==DiploScratchFAA8&&who==1&&simple==1); return 40; }
void say_tech(char *s,int tech,int kind) { assert(!*s&&tech==DiploScratchFAA8&&kind==1); std::strcpy(s,"tech"); }
int random(int a,int b) { assert(!a&&b==2); return 1; }
char *_itoa(int n,char *s,int base) { assert(base==10); std::sprintf(s,"%d",n); return s; }
int X_pops(const char *key,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000&&p==FactionDiplomacyPortraits[2]&&cb==pop_wait); std::strcpy(notice,key);
 if(!std::strcmp(key,"GIMMEHELP")) return help_response;
 if(!std::strncmp(key,"GIMMETECH",9)&&help_once) { help_once=0; return 2; }
 return response;
}
void help_topic(int kind,int tech) { assert(kind==14&&tech==DiploScratchFAA8); ++helps; }
void log_say(const char *s,int,int,int) { assert(s); }
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z);
 if(op==0x2447) { assert(a==1&&b==-money_amount&&c==2&&d==money_amount); ++money_packets; }
 else if(op==0x244b) { assert(a==2&&b==DiploScratchFAA8&&c==1&&!d); ++tech_packets; }
 else if(op==0x2442) { assert(a==1&&b==2&&c==0x800000&&!d); ++offs; }
 else { assert(op==0x2440&&a==1&&b==3&&c==2&&!d); ++crosses; }
}
struct Net { void await_diplo(int op) { ++awaits;last_await=op; }
 int add_global() { ++locks; return 0; } void unlock() { ++unlocks; } } NetDaemonNet;
struct Console { void update_data(int n) { redraws+=n?1:10; } } ConsoleGlobal;
struct Info { void set_redraw_phase(int n) { assert(!n); } } ConsoleInfoWin;
void tech_achieved(int a,int tech,int b,int z) { assert(a==2&&tech==DiploScratchFAA8&&b==1&&!z); ++technology_calls; }
void diplomacy_caption(int a,int b) { assert(a==1&&b==2); }
void treaty_off(int a,int b,int bits) { assert(a==1&&b==2&&bits==0x800000); ++offs; }
void double_cross(int a,int b,int c) { assert(a==1&&b==3&&c==2); ++crosses; }
void make_treaty(int a,int b) { assert(a==1&&b==2); ++settlements; }
unsigned long timeGetTime() { return 0; } void check_spock() {}
@@BODY@@
void reset() {
 std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
 std::memset(FactionRankings,0,sizeof(FactionRankings));
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 std::strcpy(Players[2].filename,"OTHER");
 PlayersData[1].energy_reserves=1000; PlayersData[1].mil_strength_1=100;PlayersData[2].mil_strength_1=100;
 PlayersData[2].diplo_friction[1]=10;PlayersData[1].diplo_friction[2]=17;
 DiplomacyProposalGateCount=0;DiplomacyProposalMode=2;
 DiploScratchF7E8=DiploScratchFA8C=DiploScratchFA3C=DiploScratchF7FC=0;
 DiploScratchFA54=DiploScratchFA78=DiploScratchFA2C=DiploScratchFA98=0;
 DiploScratchFAA8=DiploScratchFA18=-1;DiploScratchFA4C=1;
 DiploFriction=0;TurnCurrentNum=8;GenderDefault=PluralityDefault=0;IsMultiplayerNet=ExitTurnLoop=0;
 DiploFrictionFactionID=2;DiploFrictionFactionIDWith=1;
 response=help_response=1; help_once=helps=technology_calls=money_packets=tech_packets=0;
 money_amount=awaits=last_await=settlements=offs=crosses=locks=unlocks=redraws=0;notice[0]=0;
}
int main() {
 reset();assert(propose_treaty(1,2)==1&&settlements==1&&money_amount==450);
 assert(PlayersData[1].energy_reserves==550&&PlayersData[2].energy_reserves==450);
 assert(PlayersData[2].diplo_friction[1]==7&&PlayersData[1].diplo_friction[2]==17&&DiploFriction==-3);
 reset();IsMultiplayerNet=1;assert(propose_treaty(1,2)==1&&money_packets==1&&last_await==0x447&&settlements==1);
 reset();response=0;assert(propose_treaty(1,2)==1&&!settlements&&!std::strcmp(notice,"REJTREATYBADBRIBE"));
 reset();DiplomacyProposalMode=1;DiploScratchFA78=3;response=0;assert(propose_treaty(1,2)==0&&!settlements);
 reset();DiplomacyProposalMode=4;DiploScratchFAA8=3;DiploScratchFA18=5;help_once=1;
 assert(propose_treaty(1,2)==1&&settlements==1&&technology_calls==1&&helps==1);
 assert(DiploScratchFAA8==5&&DiploScratchFA18==-1&&PlayersData[2].diplo_friction[1]==0&&DiploFriction==-20);
 reset();DiplomacyProposalMode=4;DiploScratchFAA8=3;IsMultiplayerNet=1;
 assert(propose_treaty(1,2)==1&&tech_packets==1&&last_await==0x44b&&settlements==1);
 reset();DiplomacyProposalMode=4;DiploScratchFAA8=3;response=0;
 assert(propose_treaty(1,2)==1&&!settlements&&!std::strcmp(notice,"REJTREATYNOBRIBE"));
 reset();PlayersData[1].diplo_agenda[2]=0x2000;assert(propose_treaty(1,2)==1&&!settlements&&!std::strcmp(notice,"REJTREATY"));
 reset();PlayersData[1].diplo_agenda[2]=0x2000;DiploScratchFA78=5;
 assert(propose_treaty(1,2)==1&&!std::strcmp(notice,"REJTREATYMEEK"));
 reset();PlayersData[2].diplo_treaties[1]=0x20;PlayersData[1].diplo_agenda[2]=0x2000;
 assert(propose_treaty(1,2)==1&&!std::strcmp(notice,"REJTREATYHATE"));
 reset();DiploScratchFA3C=1;assert(propose_treaty(1,2)==1&&!std::strcmp(notice,"REJTREATYAHEAD"));
 reset();DiplomacyProposalMode=3;DiploScratchF7E8=3;
 assert(propose_treaty(1,2)==1&&settlements==1&&offs==1&&crosses==1&&locks==1&&unlocks==1);
 assert(!DiploScratchF7E8&&!DiploScratchFA4C);
 reset();DiplomacyProposalMode=3;DiploScratchF7E8=3;IsMultiplayerNet=1;
 assert(propose_treaty(1,2)==1&&settlements==1&&offs==1&&crosses==1&&last_await==0x440);
 reset();DiplomacyProposalMode=1;DiploScratchF7E8=3;help_response=0;
 assert(propose_treaty(1,2)==1&&!settlements&&PlayersData[2].diplo_patience[1]==1&&!std::strcmp(notice,"REJTREATYNOHELP"));
 reset();DiplomacyProposalMode=3;DiploScratchF7E8=3;help_response=0;
 assert(propose_treaty(1,2)==1&&settlements==1&&!offs&&!crosses);
 reset();DiploFriction=16;PlayersData[2].flags=0x400000;DiplomacyProposalMode=4;
 assert(propose_treaty(1,2)==1&&!settlements&&!std::strcmp(notice,"REJTREATYBADBRIBE"));
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('reversed_settlement', 'make_treaty(actor, other)', 'make_treaty(other, actor)'),
        ('wrong_energy_delta', '0, diplo_add(friction_units, 4) / 5',
         'diplo_add(friction_units, 4) / 5, 0'),
        ('wrong_technology_direction', 'tech_achieved(other, DiploScratchFAA8, actor, 0)',
         'tech_achieved(actor, DiploScratchFAA8, other, 0)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        result = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                 '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                                capture_output=True, text=True, timeout=45)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_buy_tech(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'int __cdecl buy_tech\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul'))
    fixture = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { char filename[24],adj_name_faction[24]; } Players[8];
struct Data { uint32_t diplo_treaties[8],ranking; int tech_ranking,energy_reserves; } PlayersData[8];
int PopupDialogFactionID,BestProtoForTrade,DiploScratchF800,DiploScratchFAA8;
int DiploScratchFA3C,DiploScratchFA5C,TurnCurrentNum,IsMultiplayerNet;
unsigned char GameTechAchieved[89]; char StringTemp[128],key[128];
int scans,scan_result,unit_value,satan,popup_answer,pops,randoms,price;
int money_messages,tech_messages,tech_calls,purchased,awaits,redraws,mutate;
int pop_wait() { return 0; }
void scan_prototypes(int a,int b) { assert(a==1 && b==2); ++scans; BestProtoForTrade=scan_result; }
int tech_val(int t,int b,int simple) { assert(t==DiploScratchF800 && b==2 && simple==1); return unit_value; }
int great_satan(int a,int z) { assert(a==1 && !z); return satan; }
int bit_count(unsigned n) { int result=0; while(n) { result+=n&1; n>>=1; } return result; }
void say_tech(char *s,int t,int cat) { assert(s==StringTemp && !*s && t==DiploScratchF800 && cat==1); std::strcpy(s,"technology"); }
int parse_says(int n,char *s,int a,int b) { assert(!n && s && a==-1 && b==-1); return 0; }
void parse_num(int n,int v) { assert(!n); price=v; }
int random(int a,int b) { assert(!a && b==2); ++randoms; return 1; }
char *_itoa(int n,char *s,int base) { assert(base==10); std::sprintf(s,"%d",n); return s; }
int X_pops(const char *s,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000 && p==FactionDiplomacyPortraits[2] && cb==pop_wait);
 std::strcpy(key,s); ++pops; return popup_answer;
}
void log_say(const char *s,int a,int b,int c) { assert(s && a==1 && (b==2 || b==DiploScratchF800)); }
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z && a==1 && c==2);
 if(op==0x2447) { assert(b==-price && d==price); ++money_messages; if(mutate) { IsMultiplayerNet=0; DiploScratchF800=6; } }
 else { assert(op==0x244b && b==DiploScratchF800 && !d); ++tech_messages; purchased=b; }
}
struct Net { void await_diplo(int op) { assert(op==0x44b); ++awaits; } } NetDaemonNet;
struct Info { void set_redraw_phase(int n) { assert(!n); } } ConsoleInfoWin;
struct Console { void update_data(int n) { redraws+=n?1:10; if(n && mutate) { IsMultiplayerNet=1; DiploScratchF800=7; } } } ConsoleGlobal;
void tech_achieved(int a,int t,int b,int z) { assert(a==1 && b==2 && !z); ++tech_calls; purchased=t; }
void diplomacy_caption(int a,int b) { assert(a==1 && b==2); }
@@BODY@@
void reset() {
 std::memset(Players,0,sizeof(Players)); std::memset(PlayersData,0,sizeof(PlayersData));
 std::memset(GameTechAchieved,0,sizeof(GameTechAchieved));
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 BestProtoForTrade=-1; scan_result=0; DiploScratchF800=3; DiploScratchFAA8=-1;
 DiploScratchFA3C=0; DiploScratchFA5C=4; TurnCurrentNum=8; IsMultiplayerNet=0;
 scans=satan=pops=randoms=price=money_messages=tech_messages=tech_calls=awaits=redraws=mutate=0;
 unit_value=401; popup_answer=1; PlayersData[1].energy_reserves=10000; key[0]=0; purchased=-1;
 std::strcpy(Players[2].filename,"OTHER");
}
int run(int mode=4,int action=0,int contact=0) { return buy_tech(1,2,action,contact,mode); }
int main() {
 reset(); DiploScratchF800=-1; assert(!run() && !pops);
 reset(); std::strcpy(Players[2].filename,"UNIV"); DiploScratchFAA8=4; assert(!run() && !pops);
 reset(); assert(!run(4,1) && !pops);
 reset(); scan_result=-1; assert(!run(5) && scans==1 && !pops);
 // Scan writes pseudo-tech F800=97; action==1 exits before native bitmap load.
 reset(); scan_result=0; assert(!run(5,1) && scans==1 && DiploScratchF800==97 && !pops);
 reset(); assert(run() && price==425 && tech_calls==1 && purchased==3 && PlayersData[1].energy_reserves==9575 && PlayersData[2].energy_reserves==425 && redraws==11 && !std::strcmp(key,"BUYTECH1"));
 reset(); GameTechAchieved[3]=15; assert(run() && price==150);
 reset(); PlayersData[1].diplo_treaties[2]=1; PlayersData[1].ranking=1; PlayersData[2].ranking=2; PlayersData[2].tech_ranking=9; assert(run() && price==125);
 reset(); unit_value=999; PlayersData[1].diplo_treaties[2]=1; PlayersData[1].ranking=1; PlayersData[2].ranking=2; PlayersData[2].tech_ranking=9; assert(run() && price==250);
 reset(); PlayersData[1].energy_reserves=0; assert(!run() && !pops);
 reset(); PlayersData[1].energy_reserves=0; assert(run(4,5) && !tech_calls && !std::strcmp(key,"BUYTECHHIGH1"));
 // Keep the commlink/key branch on a valid ordinary technology identifier.
 reset(); DiploScratchF800=3; unit_value=25; assert(run(13) && price==50 && !randoms && !std::strcmp(key,"BUYCOMMLINK0"));
 reset(); popup_answer=0; assert(run() && !tech_calls && !redraws);
 reset(); IsMultiplayerNet=1; assert(run() && money_messages==1 && tech_messages==1 && awaits==1 && !tech_calls && redraws==10);
 reset(); IsMultiplayerNet=1; mutate=1; assert(run() && money_messages==1 && !tech_messages && tech_calls==1 && purchased==6 && !awaits);
 reset(); mutate=1; assert(run() && !money_messages && tech_messages==1 && purchased==7 && awaits==1);
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_discount', 'value /= 4;', 'value = diplo_add(value, 3) / 4;'),
        ('wrong_bitmap_divisor',
         'game_tech_achieved_[DiploScratchF800]) - 1;',
         'game_tech_achieved_[DiploScratchF800]);'),
        ('wrong_tech_receiver', 'tech_achieved(faction_id, purchased_tech, other_faction_id, 0)',
         'tech_achieved(other_faction_id, purchased_tech, faction_id, 0)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


_MAKE_PROPOSAL_FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
#define nullptr 0
typedef unsigned int uint32_t;
int diplo_add(int a,int b){return (int)((unsigned)a+(unsigned)b);}
int diplo_sub(int a,int b){return (int)((unsigned)a-(unsigned)b);}
typedef unsigned long intptr_t;
const int MaxPlayerNum = 8;
const int MaxProposalNum = 11;
const uint32_t DTREATY_PACT = 1u;
const uint32_t DTREATY_HAVE_SURRENDERED = 0x02000000u;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female; char title_leader[24], name_leader[24],
    adj_name_faction[24]; } Players[8];
struct Data { uint32_t diplo_treaties[8], diplo_agenda[8]; int loan_balance[8],
    loan_payment[8], energy_reserves; char diplo_patience[8]; } PlayersData[8];
struct ProposalRecord { int preq_tech; const char *name, *description; };
ProposalRecord Proposal[MaxProposalNum];
int CouncilResolutionCount[10];
int PopupDialogFactionID, DiplomacyProposalMode, DiploScratchFA34,
    DiploScratchFAD0, DiploScratchFAD4, DiploScratchFA5C, DiploScratchFA7C;
int GenderDefault, PluralityDefault, IsMultiplayerNet, DiploFriction;
int GameState, GameRules, DiplomacyMapTransferChanged;
int BaseCurrentCount, LockEnableMask;
char StringTemp[256], PopupStartCaption[64];
const char *DiploLabelSendingENERGY = "energy";
int proposal_menu_result, menu_calls, popup_constructs, popup_destroys;
int pact_calls, treaty_calls, attack_calls, energy_calls, tech_calls, threat_calls;
int gift_calls, swap_calls, vote_calls, map_calls, map_order, modal_order;
int council_execs, council_reply, council_choice, governor_exec, governor_choice, governor_item_count;
int first_choice, reply_value, eligible_mask, proposal_set_value, proposal_selected_id;
int value_arg, item_ids[32], item_count, dialog_stage, popup_exec_answer[4];
int net_mutation, await_mutation, update_calls, update_mutation, sent_messages[8];
int message_count, awaits, local_map, popup_reply_calls, callback_count;
int event_counter, map_popup_event, map_transfer_event, update_event;
int fail_map_refusal, log_count, parse_count;
char last_popup[64];
int pop_wait() { return 0; }
int proposal_menu(int actor, int other) {
    assert(actor == 1 && other == 2); ++menu_calls; return proposal_menu_result;
}
int propose_pact(int a, int b) { assert(a==1&&b==2); ++pact_calls; return 0; }
int propose_treaty(int a, int b) { assert(a==1&&b==2); ++treaty_calls; return 0; }
int propose_attack(int a, int b) { assert(a==1&&b==2); ++attack_calls; return 0; }
int energy_trade(int a, int b) { assert(a==1&&b==2); ++energy_calls; return 6; }
int tech_trade(int a,int b,int energy,int mode,int kind) {
    assert(a==1&&b==2&&energy==0&&kind==4); value_arg=mode; ++tech_calls; return 9;
}
void threaten(int a,int b) { assert(a==1&&b==2); ++threat_calls; DiplomacyProposalMode=99; }
void make_gift(int a,int b) { assert(a==1&&b==2); ++gift_calls; }
void base_swap(int a,int b) { assert(a==1&&b==2); ++swap_calls; }
void buy_council_vote(int a,int b,int choice,int count) {
    assert(a==1&&b==2); ++vote_calls; council_choice=choice; value_arg=count;
}
int has_tech(int,int faction) { assert(faction>=0&&faction<8); return 1; }
int eligible(int faction) { return faction>=0&&faction<8 && (eligible_mask&(1<<faction)); }
unsigned int factions_status_byte(int n) { assert(n==1); return 0xff; }
int parse_says(int slot,char *s,int a,int b) {
    assert(slot>=0&&slot<=1&&s&&a==-1&&b==-1); ++parse_count; return 0;
}
void log_say(const char *s,int,int,int) { assert(s); ++log_count; }
void message_data(int op,int z,int a,int b,int c,int d) {
    assert(!z); sent_messages[message_count++]=op;
    if (op==0x2447) { assert(a==1&&b==-40&&c==2&&d==40); }
    else if (op==0x244e) { assert(a==1&&b==2&&!c&&!d); }
    else assert(false);
}
struct Net { void await_diplo(int op) { assert(op==0x447); ++awaits;
    if(await_mutation) IsMultiplayerNet=0; } } NetDaemonNet;
struct Console { void update_data(int n) { assert(n==1); ++update_calls;
    update_event=++event_counter; if(update_mutation) IsMultiplayerNet=1; }
} ConsoleGlobal;
int net_maps(int a,int b,int mode) {
    assert(a==1&&b==2&&mode==1); ++map_calls; map_transfer_event=++event_counter;
    if(local_map) ++PlayersData[2].energy_reserves;
    return 0;
}
int X_pops(const char *key,int flags,Sprite *sprite,int(*cb)()) {
    assert(key&&sprite==FactionDiplomacyPortraits[2]&&cb==pop_wait);
    std::strncpy(last_popup,key,sizeof(last_popup)-1); last_popup[sizeof(last_popup)-1]=0;
    if(!std::strcmp(key,"TRADEMAPS")) { map_popup_event=++event_counter; }
    if(!std::strcmp(key,"COUNCILVOTE")) { ++popup_reply_calls; return reply_value; }
    if(!std::strcmp(key,"REPAIDLOAN") && modal_order) DiploScratchFA34=modal_order;
    if(!std::strcmp(key,"REJMAPS")) assert(flags==0x100000);
    return 0;
}
int x_pops_minimal(const char *,Sprite *,int(*)()) { return 0; }
char *get_pact_hood(int,int) { return Players[1].adj_name_faction; }
struct StringTableType { const char *get(int) { return "label"; } } StringTable;
struct Dialogs {
    void sprite(Sprite *s,char *text,int n) { assert(s==FactionDiplomacyPortraits[2]&&!text&&!n); }
    void item(char *,int id) {
        if(dialog_stage==1) { assert(item_count<32); item_ids[item_count++]=id; }
        else if(dialog_stage==2) { ++governor_item_count; }
    }
    void set_value(int id) { ++proposal_set_value; value_arg=id; }
    void set_selected_id(int id) { ++proposal_selected_id; value_arg=id; }
};
struct Popup {
    Dialogs dlg; int start(const char *,const char *key,int,char *,int,int) {
        if(!std::strcmp(key,"COUNCILMATTER")) dialog_stage=1;
        else if(!std::strcmp(key,"COUNCILVOTEGOV")) dialog_stage=2;
        return 0;
    }
    Popup() { ++popup_constructs; } ~Popup() { ++popup_destroys; }
    bool picture_enabled() { return false; }
    void set_caller_picture(Sprite *s) { assert(s==FactionDiplomacyPortraits[2]); }
    void set_value(int id) { ++proposal_set_value; value_arg=id; }
    void set_selected_id(int id) { ++proposal_selected_id; value_arg=id; }
    Dialogs &dialogs() { return dlg; }
    int exec(int n,int(*cb)()) {
        assert(n==0&&cb==pop_wait);
        if(dialog_stage==1) { ++council_execs; return first_choice; }
        if(dialog_stage==2) { ++governor_exec; return governor_choice; }
        return -1;
    }
};
// @@BODY@@
void reset() {
    std::memset(Players,0,sizeof(Players)); std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Proposal,0,sizeof(Proposal)); std::memset(CouncilResolutionCount,0,sizeof(CouncilResolutionCount));
    std::memset(sent_messages,0,sizeof(sent_messages));
    for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
    std::strcpy(Players[1].title_leader,"Leader"); std::strcpy(Players[1].name_leader,"Name");
    PlayersData[1].loan_balance[2]=40; PlayersData[1].loan_payment[2]=4;
    for(int i=0;i<MaxProposalNum;++i) { Proposal[i].preq_tech=0; Proposal[i].name="n"; Proposal[i].description="d"; }
    PopupDialogFactionID=DiplomacyProposalMode=DiploScratchFA34=0;
    DiploScratchFAD0=DiploScratchFAD4=DiploScratchFA5C=DiploScratchFA7C=0;
    GenderDefault=PluralityDefault=IsMultiplayerNet=DiploFriction=0;
    GameState=0; GameRules=8; DiplomacyMapTransferChanged=0;
    proposal_menu_result=menu_calls=popup_constructs=popup_destroys=0;
    pact_calls=treaty_calls=attack_calls=energy_calls=tech_calls=threat_calls=0;
    gift_calls=swap_calls=vote_calls=map_calls=map_order=modal_order=0;
    council_execs=council_reply=council_choice=governor_exec=governor_choice=governor_item_count=0;
    first_choice=reply_value=eligible_mask=proposal_set_value=proposal_selected_id=0;
    value_arg=item_count=dialog_stage=0; std::memset(item_ids,0,sizeof(item_ids));
    update_calls=update_mutation=await_mutation=net_mutation=0;
    message_count=awaits=local_map=popup_reply_calls=callback_count=0;
    event_counter=map_popup_event=map_transfer_event=update_event=0;
    fail_map_refusal=log_count=parse_count=0; last_popup[0]=0;
}
bool has_item(int id) { for(int i=0;i<item_count;++i) if(item_ids[i]==id)return true; return false; }
int main() {
    // Menu cancellation returns with the local Popup still destructed exactly once.
    reset(); proposal_menu_result=0; make_a_proposal(1,2,0);
    assert(menu_calls==1&&popup_constructs==1&&popup_destroys==1&&PopupDialogFactionID==2);
    assert(!pact_calls&&!threat_calls&&!vote_calls);

    // The saved counteroffer mode drives downstream dispatch even if threat changes the global.
    reset(); DiplomacyProposalMode=2; DiploScratchFA34=2; make_a_proposal(1,2,1);
    assert(pact_calls==1&&threat_calls==0&&popup_destroys==1);
    reset(); DiplomacyProposalMode=3; DiploScratchFA34=2; make_a_proposal(1,2,1);
    assert(pact_calls==1&&threat_calls==1&&DiplomacyProposalMode==99);
    reset(); DiplomacyProposalMode=4; DiploScratchFA34=4; make_a_proposal(1,2,1);
    assert(tech_calls==1&&value_arg==4&&DiploScratchFAD0==9);
    reset(); DiplomacyProposalMode=4; DiploScratchFA34=6; make_a_proposal(1,2,1);
    assert(energy_calls==1&&DiploScratchFAD0==6);
    reset(); DiplomacyProposalMode=4; DiploScratchFA34=1; make_a_proposal(1,2,1);
    assert(gift_calls==1);
    reset(); DiplomacyProposalMode=4; DiploScratchFA34=3; make_a_proposal(1,2,1);
    assert(treaty_calls==1&&threat_calls==0);
    reset(); DiplomacyProposalMode=3; DiploScratchFA34=3; make_a_proposal(1,2,1);
    assert(treaty_calls==1&&threat_calls==1);
    reset(); DiplomacyProposalMode=3; DiploScratchFA34=8;
    PlayersData[2].diplo_treaties[1]=DTREATY_PACT|DTREATY_HAVE_SURRENDERED;
    make_a_proposal(1,2,1); assert(attack_calls==1&&!threat_calls);
    reset(); DiplomacyProposalMode=3; DiploScratchFA34=8; make_a_proposal(1,2,1);
    assert(!attack_calls&&threat_calls==1);
    reset(); DiplomacyProposalMode=4; DiploScratchFA34=9; make_a_proposal(1,2,1);
    assert(swap_calls==1);

    // Map refusal has no transfer or patience effect; success displays first, then transfers.
    reset(); DiploScratchFA34=7; DiploFriction=1; make_a_proposal(1,2,1);
    assert(!map_calls&&!PlayersData[2].diplo_patience[1]&&!DiplomacyMapTransferChanged);
    assert(!std::strcmp(last_popup,"REJMAPS"));
    reset(); DiploScratchFA34=7; PlayersData[1].diplo_treaties[2]=DTREATY_PACT;
    make_a_proposal(1,2,1);
    assert(map_calls==1&&map_popup_event>0&&map_popup_event<map_transfer_event);
    assert(PlayersData[2].diplo_patience[1]==2&&DiplomacyMapTransferChanged==1);

    // Local repayment redraw can switch networking on before the second live check.
    reset(); DiploScratchFA34=11; update_mutation=1; make_a_proposal(1,2,1);
    assert(update_calls==1&&message_count==1&&sent_messages[0]==0x244e);
    assert(PlayersData[1].energy_reserves==-40&&PlayersData[2].energy_reserves==40);
    assert(PlayersData[1].loan_balance[2]==40&&PlayersData[1].loan_payment[2]==4);
    assert(!std::strcmp(last_popup,"REPAIDLOAN"));
    // A network await may switch networking off before the second live check.
    reset(); DiploScratchFA34=11; IsMultiplayerNet=1; await_mutation=1; make_a_proposal(1,2,1);
    assert(awaits==1&&message_count==1&&sent_messages[0]==0x2447);
    assert(PlayersData[1].loan_balance[2]==0&&PlayersData[1].loan_payment[2]==0);
    reset(); DiploScratchFA34=11; modal_order=12; first_choice=-1; make_a_proposal(1,2,1);
    assert(council_execs==1&&proposal_set_value==0);
    reset(); DiploScratchFA34=11; make_a_proposal(1,2,1);
    assert(!council_execs);

    // Council case ids retain their recovered predicates (1..9 jump-table order).
    // Addresses: FA82, FA6E, FADA, FAF0, FA6E, FB03, FB0C, FAAD, FAC4.
    reset(); eligible_mask=1<<1; GameRules=9; GameState=0;
    CouncilResolutionCount[1]=CouncilResolutionCount[2]=CouncilResolutionCount[5]=0;
    CouncilResolutionCount[3]=CouncilResolutionCount[4]=0;
    CouncilResolutionCount[8]=CouncilResolutionCount[9]=0;
    first_choice=-1; DiploScratchFA34=12; make_a_proposal(1,2,1);
    assert(has_item(1)&&has_item(2)&&has_item(3)&&!has_item(4)&&has_item(5));
    assert(!has_item(6)&&has_item(7)&&has_item(8)&&!has_item(9));
    reset(); eligible_mask=1<<1; GameRules=8; GameState=1; DiploScratchFA34=12;
    make_a_proposal(1,2,1); assert(!has_item(1));
    reset(); eligible_mask=1<<1; GameRules=0; GameState=0; DiploScratchFA34=12;
    first_choice=-1; make_a_proposal(1,2,1); assert(!has_item(1));
    reset(); eligible_mask=1<<1; GameRules=9; GameState=0;
    CouncilResolutionCount[1]=CouncilResolutionCount[2]=0;
    CouncilResolutionCount[3]=2; CouncilResolutionCount[4]=1;
    CouncilResolutionCount[5]=1; CouncilResolutionCount[8]=2; CouncilResolutionCount[9]=1;
    DiploScratchFA34=12; make_a_proposal(1,2,1);
    assert(!has_item(3)&&has_item(4)&&!has_item(5)&&has_item(6));
    assert(!has_item(8)&&has_item(9));

    // Confirmation reply 0 yields vote_count -1; zero result suppresses purchase.
    reset(); DiploScratchFA34=12; first_choice=3; reply_value=0;
    make_a_proposal(1,2,1); assert(vote_calls==1&&council_choice==3&&value_arg==-1);
    reset(); DiploScratchFA34=12; first_choice=3; reply_value=-1;
    make_a_proposal(1,2,1); assert(vote_calls==0&&popup_reply_calls==1);

    // Governor's selected faction is a default value, not a selected id.
    reset(); DiploScratchFA34=12; first_choice=0; eligible_mask=(1<<1)|(1<<2);
    governor_choice=-1; make_a_proposal(1,2,1);
    assert(proposal_set_value==1&&proposal_selected_id==0&&governor_exec==1);
    assert(!vote_calls&&value_arg==1&&governor_item_count==2);
    reset(); DiploScratchFA34=12; first_choice=0; eligible_mask=(1<<1)|(1<<2);
    governor_choice=2; make_a_proposal(1,2,1);
    assert(vote_calls==1&&council_choice==0&&value_arg==2);
    return 0;
}
'''


def _run_make_proposal_controls():
    body = definition((ROOT / 'src/faction.cpp').read_text(), r"void\s+__cdecl\s+make_a_proposal\s*\(")
    assert "proposal_menu" in body and "buy_council_vote" in body
    mutations = {
        "network_timing": body.replace(
            'if (IsMultiplayerNet != 0) {\n            log_say("Sending LOAN"',
            'if (IsMultiplayerNet != 0 || awaits != 0) {\n            log_say("Sending LOAN"'),
        "missing_repayment_modal": body.replace(
            'X_pops("REPAIDLOAN", 0x100000,\n'
            '                     FactionDiplomacyPortraits[other_faction_id], pop_wait);',
            '(void)0;'),
        "game_state_predicate": body.replace('(GameDataStateStorage.fields_.game_state_ & 1)', '(GameDataStateStorage.fields_.game_rules_ & 1)'),
        "governor_default_field": body.replace('popup.set_value(candidate)',
                                                'popup.set_selected_id(candidate)'),
        "lost_pact_friction_guard": body.replace(
            '(((actor_treaty & DTREATY_PACT) == 0) && DiploFriction > 0) ||',
            'false ||'),
        "lost_council_rule_guard": body.replace('(GameDataStateStorage.fields_.game_rules_ & 8) != 0', 'true'),
    }
    assert all(mutated != body for mutated in mutations.values()), "mutation anchor drift"
    failures = []
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if not compiler:
            print("SKIP", compiler_name, "not installed")
            continue
        with tempfile.TemporaryDirectory(prefix="proposal-behavior-", dir=None) as temp:
            scratch = Path(temp)
            candidates = {"actual": body}
            candidates.update(mutations)
            for name, candidate in candidates.items():
                cpp, binary = scratch / (name + ".cpp"), scratch / name
                cpp.write_text(host_fixture(_MAKE_PROPOSAL_FIXTURE).replace("// @@BODY@@", candidate, 1))
                built = subprocess.run(
                    [compiler, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra",
                     "-Wno-unused-parameter", "-fsanitize=undefined",
                     "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                    capture_output=True, text=True, timeout=45)
                if built.returncode:
                    failures.append(compiler_name + "/" + name + " compile:\n" +
                                    built.stdout + built.stderr)
                    continue
                result = subprocess.run([str(binary)], capture_output=True,
                                        text=True, timeout=15)
                expected_pass = name == "actual"
                if (result.returncode == 0) != expected_pass:
                    failures.append(compiler_name + "/" + name + " unexpected result:\n" +
                                    result.stdout + result.stderr)
            if not failures:
                print("PASS", compiler_name,
                      "actual body and", len(mutations), "detected behavior mutations")
    if failures:
        raise SystemExit("\n".join(failures))
    if not any(shutil.which(c) for c in ("g++", "clang++")):
        raise SystemExit("No local GCC or Clang compiler is available")
    print("Controls cover a C++98 fixture only; no native/game acceptance is implied.")



def test_actual_make_a_proposal_controls():
    _run_make_proposal_controls()


_tech_modal_fixture = r'''#include <cassert>
#include <cstring>
#include <stdint.h>
#define FALSE 0
#define TRUE 1
typedef unsigned int uint32_t;
typedef int BOOL;

struct Player { char filename[32]; } Players[8];
struct PlayerData {
    uint32_t diplo_treaties[8];
    uint32_t diplo_friction[8];
    int tech_ranking;
} PlayersData[8];

int DiploScratchF814, DiploScratchFAA8, DiploScratchF800;
int DiploScratchFA18, DiploScratchF80C, DiploScratchFA44;
int DiploScratchFA48, DiploScratchFA60;
uint32_t RankingFactionIDUnk1;
int IsMultiplayerNet, DiploFriction;
uint32_t DiploFrictionFactionIDWith, DiploFrictionFactionID;
int GenderDefault, PluralityDefault;
char StringTemp[256];
struct Sprite {};
Sprite *FactionDiplomacyPortraits[8];
int pop_result, help_calls, help_id, help_topic_id;
int tech_value[100], tech_val_calls;
int transfer_count, transferred_sender[8], transferred_tech[8];
int transferred_receiver[8], mutate_f800_on_first_transfer;
int agenda_count, agenda_actor[4], agenda_partner[4], agenda_mask[4];
int treaty_count, treaty_actor[4], treaty_partner[4], treaty_mask[4];

const char *DiploLabelSendingTECH = "Sending TECH";
const char *DiploLabelSendingTREATYON = "Sending TREATYON";
const char *DiploLabelSendingAGENDAON = "Sending AGENDAON";

int diplo_add(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) +
                            static_cast<uint32_t>(right));
}
int diplo_sub(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) -
                            static_cast<uint32_t>(right));
}
void swap(int *left, int *right) {
    const int temporary = *left;
    *left = *right;
    *right = temporary;
}
int tech_val(int technology, int, int) {
    ++tech_val_calls;
    return technology >= 0 && technology < 100 ? tech_value[technology]
                                               : technology;
}
void say_tech(char *, int, int) {}
int parse_says(int, const char *, int, int) { return 0; }
void help_topic(unsigned int topic, int technology) {
    ++help_calls; help_id = static_cast<int>(topic); help_topic_id = technology;
}
int X_pops(const char *, int, Sprite *, int (*)()) { return pop_result; }
int pop_wait() { return 0; }
int great_satan(int, int) { return 0; }

void log_say(const char *, int, int, int) {}
int message_data(int, int, int, int, int, int) { return 0; }
void treaty_on(int actor, int partner, int mask) {
    treaty_actor[treaty_count] = actor;
    treaty_partner[treaty_count] = partner;
    treaty_mask[treaty_count++] = mask;
}
void agenda_on(int actor, int partner, int mask) {
    agenda_actor[agenda_count] = actor;
    agenda_partner[agenda_count] = partner;
    agenda_mask[agenda_count++] = mask;
}
void tech_achieved(int sender, int technology, int receiver, int) {
    transferred_sender[transfer_count] = sender;
    transferred_tech[transfer_count] = technology;
    transferred_receiver[transfer_count] = receiver;
    ++transfer_count;
    if (mutate_f800_on_first_transfer && transfer_count == 1)
        DiploScratchF800 = 37;
}
struct NetDaemonStub { void await_diplo(int) {} } NetDaemonNet;

// @@CANDIDATE@@

void reset_fixture() {
    std::memset(Players, 0, sizeof(Players));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(tech_value, 0, sizeof(tech_value));
    DiploScratchF814 = DiploScratchFAA8 = DiploScratchF800 = -1;
    DiploScratchFA18 = DiploScratchF80C = DiploScratchFA44 = -1;
    DiploScratchFA48 = DiploScratchFA60 = 0;
    RankingFactionIDUnk1 = 0; IsMultiplayerNet = 0; DiploFriction = 0;
    DiploFrictionFactionIDWith = DiploFrictionFactionID = 0;
    GenderDefault = PluralityDefault = 0; StringTemp[0] = '\0';
    pop_result = 1; help_calls = help_id = help_topic_id = 0;
    tech_val_calls = transfer_count = mutate_f800_on_first_transfer = 0;
    agenda_count = treaty_count = 0;
    for (int i = 0; i < 8; ++i) {
        transferred_sender[i] = transferred_tech[i] = -1;
        transferred_receiver[i] = -1;
        agenda_actor[i < 4 ? i : 3] = 0;
        agenda_partner[i < 4 ? i : 3] = 0;
        agenda_mask[i < 4 ? i : 3] = 0;
        treaty_actor[i < 4 ? i : 3] = 0;
        treaty_partner[i < 4 ? i : 3] = 0;
        treaty_mask[i < 4 ? i : 3] = 0;
    }
}

int main() {
    // Result 1 preserves the initial signed F814/4 share, including negative
    // truncation toward zero as emitted by the native add/sign-correct/shift.
    reset_fixture();
    DiploScratchF814 = -5;
    TechTradeDialogStep result1 =
        tech_trade_handle_dialog_result(1, 1, 2, 0);
    assert(!result1.rebuild_dialog && result1.friction_eighths == -1);

    // Result 4 swaps the selectors first, then uses the live F814/8 share.
    reset_fixture();
    DiploScratchF814 = 13;
    DiploScratchF800 = 10; DiploScratchF80C = 20;
    DiploScratchFA18 = 0;
    PlayersData[2].diplo_treaties[1] = 1;
    TechTradeDialogStep result4 =
        tech_trade_handle_dialog_result(4, 1, 2, 0);
    assert(DiploScratchF800 == 20 && DiploScratchF80C == 10);
    assert(result4.friction_eighths == 1);

    // Result 2 swaps FAA8/FA18 before taking the live FA44/8 share.
    reset_fixture();
    std::strcpy(Players[2].filename, "OTHER");
    DiploScratchFAA8 = 10; DiploScratchFA18 = 20;
    DiploScratchFA44 = -9;
    TechTradeDialogStep result2 =
        tech_trade_handle_dialog_result(2, 1, 2, 0);
    assert(DiploScratchFAA8 == 20 && DiploScratchFA18 == 10);
    assert(result2.friction_eighths == -1);

    // Agenda always uses partner->actor; treaty is request-mode-only and has
    // that same direction. The friction IDs are partner in FAC0, actor in FABC.
    reset_fixture();
    DiploScratchFAA8 = 11; DiploScratchF800 = 22; DiploScratchFA18 = 33;
    PlayersData[2].diplo_friction[1] = 10;
    DiploFriction = 8;
    DiploFrictionFactionID = 2;
    DiploFrictionFactionIDWith = 1;
    mutate_f800_on_first_transfer = 1;
    TechTradeTailEntry request0 =
        tech_trade_commit_dialog_state(1, 2, 0, 2);
    assert(request0 == TECH_TRADE_TAIL_SUCCESS_5433A0);
    assert(treaty_count == 0);
    assert(agenda_count == 1 && agenda_actor[0] == 2 &&
           agenda_partner[0] == 1 && agenda_mask[0] == 0x180);
    assert(transfer_count == 2);
    assert(transferred_sender[0] == 2 && transferred_tech[0] == 11 &&
           transferred_receiver[0] == 1);
    assert(transferred_sender[1] == 1 && transferred_tech[1] == 37 &&
           transferred_receiver[1] == 2);
    assert(PlayersData[2].diplo_friction[1] == 8 && DiploFriction == 6);
    assert(DiploScratchFAA8 == 33 && DiploScratchFA18 == -1 &&
           DiploScratchFA60 == 0);

    // Request mode adds exactly the reversed treaty callback, while agenda
    // still occurs in the local callback path.
    reset_fixture();
    DiploScratchFAA8 = 4; DiploScratchF800 = 5;
    IsMultiplayerNet = 0;
    TechTradeTailEntry request1 =
        tech_trade_commit_dialog_state(1, 2, 1, 0);
    assert(static_cast<int>(request1) == -1);
    assert(treaty_count == 1 && treaty_actor[0] == 2 &&
           treaty_partner[0] == 1 && treaty_mask[0] == 0x1000000);
    assert(agenda_count == 1 && agenda_actor[0] == 2 &&
           agenda_partner[0] == 1 && agenda_mask[0] == 0x180);
    return 0;
}
'''


def _tech_modal_named_struct(source, marker):
    start = source.index(marker)
    brace = source.index("{", start)
    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise ValueError("unclosed definition: " + marker)


def _tech_modal_candidate_text(source):
    enum = _tech_modal_named_struct(source, "enum TechTradeTailEntry")
    struct = _tech_modal_named_struct(source, "struct TechTradeDialogStep")
    functions = (
        r"static int\s+tech_trade_signed_shift_divide\s*\(",
        r"static TechTradeDialogStep\s+tech_trade_handle_dialog_result\s*\(",
        r"static TechTradeTailEntry\s+tech_trade_commit_dialog_state\s*\(",
    )
    return enum + ";\n" + struct + ";\n" + "\n".join(
        definition(source, pattern) for pattern in functions)


def _run_tech_modal_controls():
    source = (ROOT / 'src/faction.cpp').read_text()
    actual = _tech_modal_candidate_text(source)
    required = (
        "tech_trade_signed_shift_divide(\n        DiploScratchF814, 3, 2)",
        "tech_trade_signed_shift_divide(\n            DiploScratchF814, 7, 3)",
        "tech_trade_signed_shift_divide(\n            DiploScratchFA44, 7, 3)",
        "selected = DiploScratchF800",
        "treaty_on(partner, actor, 0x1000000)",
        "agenda_on(partner, actor, 0x180)",
        "DiploFrictionFactionID == static_cast<uint32_t>(partner)",
        "DiploFrictionFactionIDWith == static_cast<uint32_t>(actor)",
    )
    for anchor in required:
        assert anchor in actual, "native control anchor drift: " + anchor

    mutations = {
        "wrong_result4_divisor": actual.replace(
            "DiploScratchF814, 7, 3", "DiploScratchF814, 3, 2", 1),
        "wrong_treaty_direction": actual.replace(
            "treaty_on(partner, actor, 0x1000000)",
            "treaty_on(actor, partner, 0x1000000)", 1),
        "conditional_agenda": actual.replace(
            "agenda_on(partner, actor, 0x180);",
            "if (request_mode != 0) agenda_on(partner, actor, 0x180);", 1),
        "cached_first_offer": actual.replace(
            "selected = DiploScratchF800;", "selected = DiploScratchFAA8;", 1),
    }
    for name, mutated in mutations.items():
        assert mutated != actual, "mutation anchor drift: " + name

    failures = []
    ran = 0
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if not compiler:
            print("SKIP", compiler_name, "not installed")
            continue
        ran += 1
        with tempfile.TemporaryDirectory(
                prefix="tech-ui-",
                dir=None) as temp:
            scratch = Path(temp)
            candidates = {"actual": actual}
            candidates.update(mutations)
            for name, candidate in candidates.items():
                cpp, binary = scratch / (name + ".cpp"), scratch / name
                cpp.write_text(host_fixture(_tech_modal_fixture).replace("// @@CANDIDATE@@", candidate, 1))
                compiled = subprocess.run(
                    [compiler, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra",
                     "-Wno-unused-parameter", "-fsanitize=undefined",
                     "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                    capture_output=True, text=True, timeout=45)
                if compiled.returncode:
                    failures.append(compiler_name + "/" + name + " compile:\n" +
                                    compiled.stdout + compiled.stderr)
                    continue
                result = subprocess.run([str(binary)], capture_output=True,
                                        text=True, timeout=15)
                if name == "actual":
                    if result.returncode:
                        failures.append(compiler_name + "/actual controls:\n" +
                                        result.stdout + result.stderr)
                elif result.returncode == 0:
                    failures.append(compiler_name + "/" + name +
                                    " mutation unexpectedly passed")

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    if not ran:
        print("No host C++98 compiler available; fixture not run.",
              file=sys.stderr)
        return 3
    print("Actual modal/commit definitions passed C++98 host controls.")
    return 0



def test_actual_tech_trade_modal_controls():
    assert _run_tech_modal_controls() == 0

_tech_policy_fixture = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#define __cdecl
typedef unsigned int uint32_t;
struct Player {
    char filename[32], search_key[32], title_leader[24], name_leader[24];
    char insult_leader[24], adj_insult_faction[24];
    char assistant_city[24];
    int is_leader_female, rule_techcost;
} Players[8];
struct PlayerData { uint32_t diplo_treaties[8], ranking, flags; int tech_ranking; };
const uint32_t PFLAG_COOP_WITH_HUMAN = 0x00400000u;
PlayerData PlayersData[8];
struct ReactorRecord { int preq_tech; } Reactor[2];
uint32_t FactionRankingsUnk[8];
int RankingFactionIDUnk2;
int PopupDialogFactionID, DiploScratchF7C4, DiploScratchF800;
int DiploScratchFA5C, DiploScratchFA2C, DiploScratchFA60, DiploScratchFA6C;
int DiploFriction, IsMultiplayerNet, GameRules, GenderDefault, PluralityDefault;
char DiploLabelUNIV_7DC[] = "UNIV";
char DiploLabelHIVE_808[] = "HIVE";
char DiploLabelREJTECHHATE[] = "REJTECHHATE";
int SatanResult, tech_available[8][128], close_calls, popup_calls;
int tech_analysis_event, caption_event, sequence, rejection_calls;
int parse_channels[32], parse_count, parse_num_calls, popup_short_calls;
const char *parse_values[32];
int parse_num_result, buy_tech_result;
char StringTemp[256];
struct FacilityRecord { char name[24]; } Facility[128];
struct Sprite {} sprites[8];
Sprite *FactionDiplomacyPortraits[8];
const char *DiploLabelREJTECHFEAR="REJTECHFEAR";
const char *DiploLabelREJTECHFEAR_7FC="REJTECHFEARALT";
const char *DiploLabelREJTECHADVANCED="REJTECHADVANCED";
const char *DiploLabelREJTECHPROJ="REJTECHPROJ";
int pop_wait(){return 0;}
int parse_says(int channel, const char *value, int, int) {
    assert(parse_count<32);parse_channels[parse_count]=channel;
    parse_values[parse_count++]=value;return 0;
}
char *get_phrase(int faction, int phrase){assert(faction==1&&phrase==3);return Players[faction].assistant_city;}
int parse_num(int channel, int arg){assert(arg==0);++parse_num_calls;return parse_num_result;}
char *_itoa(int value,char *buffer,int radix){assert(radix==10);std::sprintf(buffer,"%d",value);return buffer;}
int X_pops(const char *text,int flags,Sprite *sprite,int(*callback)()) {
    assert(flags==0x100000&&callback==pop_wait&&sprite==FactionDiplomacyPortraits[2]);
    assert(text&&std::strlen(text)<sizeof(StringTemp));++popup_short_calls;
    std::strcpy(StringTemp,text);return 0;
}
int buy_tech(int actor,int partner,int action,int mode,int context) {
    assert(actor==1&&partner==2&&action>=0&&mode==1&&(context==0||context==13));
    return buy_tech_result;
}
int diplo_add(int a,int b){return (int)((uint32_t)a+(uint32_t)b);}
int diplo_sub(int a,int b){return (int)((uint32_t)a-(uint32_t)b);}
int diplo_mul(int a,int b){return (int)((uint32_t)a*(uint32_t)b);}
struct Popup { void close(); };
int has_tech(int tech, int faction);
int great_satan(int faction, int unused);
void tech_analysis(int actor, int partner);
void diplomacy_caption(int actor, int partner);
// @@CANDIDATE@@
int has_tech(int tech, int faction) {
    assert(faction >= 0 && faction < 8 && tech >= 0 && tech < 128);
    return tech_available[faction][tech];
}
int great_satan(int faction, int unused) {
    assert(faction == 1 && unused == 0); return SatanResult;
}
void tech_analysis(int actor, int partner) {
    assert(actor == 1 && partner == 2); tech_analysis_event = ++sequence;
}
void diplomacy_caption(int actor, int partner) {
    assert(actor == 1 && partner == 2); caption_event = ++sequence;
}
void Popup::close() { ++close_calls; }
void reset() {
    std::memset(Players, 0, sizeof(Players));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Reactor, 0, sizeof(Reactor));
    std::memset(FactionRankingsUnk, 0, sizeof(FactionRankingsUnk));
    for (int f=0; f<8; ++f) for (int t=0; t<128; ++t) tech_available[f][t]=1;
    Players[2].rule_techcost = 100;
    RankingFactionIDUnk2=2;
    PopupDialogFactionID=DiploScratchF7C4=DiploScratchF800=0;
    DiploScratchFA5C=0; DiploScratchFA2C=0; DiploScratchFA60=0;
    DiploScratchFA6C=-1; DiploFriction=0; IsMultiplayerNet=0; GameRules=0;
    GenderDefault=PluralityDefault=0; SatanResult=0;
    close_calls=popup_calls=tech_analysis_event=caption_event=sequence=0;
    rejection_calls=0;
    parse_count=parse_num_calls=popup_short_calls=0;
    parse_num_result=0;buy_tech_result=0;StringTemp[0]='\0';
    std::memset(parse_channels,0,sizeof(parse_channels));
    std::memset(parse_values,0,sizeof(parse_values));
    for(int i=0;i<8;++i)FactionDiplomacyPortraits[i]=&sprites[i];
}
void assert_contact(TechTradeContactResult expected) {
    assert(tech_trade_contact_policy(1,2)==expected);
}
int main() {
    // Actor-row commlink bypass is checked before reverse-row vetoes.
    reset(); PlayersData[1].diplo_treaties[2]=1;
    PlayersData[2].diplo_treaties[1]=0x8020; DiploScratchFA2C=1;
    DiploFriction=18; DiploScratchFA60=1; std::strcpy(Players[2].filename,"OTHER");
    assert_contact(TECH_TRADE_CONTACT_ALLOWED);

    // Each reverse-row veto is independent; the threshold is strictly > 17.
    reset(); PlayersData[2].diplo_treaties[1]=0x20;
    assert_contact(TECH_TRADE_CONTACT_REJECTED);
    reset(); PlayersData[2].diplo_treaties[1]=0x8000;
    assert_contact(TECH_TRADE_CONTACT_REJECTED);
    reset(); DiploScratchFA2C=1; assert_contact(TECH_TRADE_CONTACT_REJECTED);
    reset(); DiploFriction=18; assert_contact(TECH_TRADE_CONTACT_REJECTED);
    reset(); DiploFriction=17; assert_contact(TECH_TRADE_CONTACT_ALLOWED);

    // FA60 zero falls through; when nonzero, only partner filename controls
    // the UNIV exception. search_key intentionally disagrees in both cases.
    reset(); DiploScratchFA60=0; std::strcpy(Players[2].filename,"OTHER");
    std::strcpy(Players[2].search_key,"UNIV");
    assert_contact(TECH_TRADE_CONTACT_ALLOWED);
    reset(); DiploScratchFA60=1; std::strcpy(Players[2].filename,"UNIV");
    std::strcpy(Players[2].search_key,"OTHER");
    assert_contact(TECH_TRADE_CONTACT_ALLOWED);
    reset(); DiploScratchFA60=1; std::strcpy(Players[2].filename,"OTHER");
    std::strcpy(Players[2].search_key,"UNIV");
    assert_contact(TECH_TRADE_CONTACT_REJECTED);

    // Entry order and context-13 state capture; lexical Popup destruction
    // remains with the owner, so the policy helper does not call close().
    reset(); Popup popup; DiploScratchFA5C=7;
    TechTradePolicyDecision allowed = tech_trade_policy_prefix(popup,1,2,1,4,13,true);
    assert(PopupDialogFactionID==2&&DiploScratchF7C4==0&&DiploScratchF800==0x60);
    assert(tech_analysis_event==1&&caption_event==2&&close_calls==0);
    assert(allowed.route==TECH_TRADE_POLICY_READY_FOR_OFFERS&&allowed.special_action);
    reset(); Popup rejected_popup; DiploScratchFA60=1;
    std::strcpy(Players[2].filename,"OTHER");
    TechTradePolicyDecision rejected = tech_trade_policy_prefix(rejected_popup,1,2,1,1,0,true);
    assert(rejected.route==TECH_TRADE_POLICY_REJECT_542097&&close_calls==0);
    assert(rejection_calls==0&&PopupDialogFactionID==2);

    // Rejection presentation uses assistant_city on parser channel 2, then
    // the completed text is passed directly to the short-popup boundary.
    reset();std::strcpy(Players[1].assistant_city,"Assistant City");
    parse_num_result=7;tech_trade_show_contact_rejection(1,2,0);
    assert(parse_count==5&&parse_channels[0]==0&&parse_channels[1]==1&&
           parse_channels[2]==2&&parse_channels[3]==3&&parse_channels[4]==4);
    assert(parse_values[2]==Players[1].assistant_city&&parse_num_calls==1);
    assert(popup_short_calls==1&&!std::strcmp(StringTemp,"REJTECHHATE7"));
    reset();tech_trade_show_contact_rejection(1,2,13);
    assert(parse_num_calls==0&&popup_short_calls==1&&
           !std::strcmp(StringTemp,"REJTECHHATE0"));

    // Pre-offer friction threshold and rank/power exits use isolated predicates.
    reset(); SatanResult=1; DiploFriction=3;
    TechTradePolicyDecision d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS);
    reset(); SatanResult=1; DiploFriction=4;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_54222B);
    reset(); SatanResult=1; IsMultiplayerNet=1; PlayersData[1].ranking=7;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_5423D8);
    reset(); IsMultiplayerNet=1; PlayersData[1].ranking=7;
    FactionRankingsUnk[1]=5; FactionRankingsUnk[2]=3;
    PlayersData[1].tech_ranking=5; PlayersData[2].tech_ranking=3;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_5423D8);

    // HIVE adjusts the special threshold from partner.filename (not search_key).
    reset(); PlayersData[1].tech_ranking=0; PlayersData[2].tech_ranking=1;
    DiploFriction=6; std::strcpy(Players[2].filename,"HIVE");
    std::strcpy(Players[2].search_key,"OTHER");
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_5425BF);
    reset(); PlayersData[1].tech_ranking=0; PlayersData[2].tech_ranking=1;
    DiploFriction=6; std::strcpy(Players[2].filename,"OTHER");
    std::strcpy(Players[2].search_key,"HIVE");
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS);
    reset(); PlayersData[1].tech_ranking=0; PlayersData[2].tech_ranking=1;
    PlayersData[2].diplo_treaties[1]=2; DiploFriction=8;
    std::strcpy(Players[2].filename,"HIVE");
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS);
    DiploFriction=9;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_5425BF);

    // Human-cooperation flags suppress the otherwise active rejection checks.
    reset(); SatanResult=1; DiploFriction=4;
    PlayersData[2].flags=PFLAG_COOP_WITH_HUMAN;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS);

    // Ranked prerequisite refusal and the partner self-row bypass.
    reset(); Reactor[0].preq_tech=7; tech_available[2][7]=1; tech_available[1][7]=0;
    PlayersData[1].tech_ranking=PlayersData[2].tech_ranking=4;
    d=tech_trade_pre_offer_policy(1,2,1,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_542585&&d.partner_has_missing_prerequisite);
    reset(); Reactor[0].preq_tech=7; tech_available[2][7]=1; tech_available[1][7]=0;
    PlayersData[2].diplo_treaties[1]=0x02000001u;
    PlayersData[1].tech_ranking=PlayersData[2].tech_ranking=4;
    d=tech_trade_pre_offer_policy(1,2,1,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS&&!d.partner_has_missing_prerequisite);

    // Project slot refusal is suppressed only for native context 13.
    reset(); DiploScratchFA6C=0;
    d=tech_trade_pre_offer_policy(1,2,0,1,0,false);
    assert(d.route==TECH_TRADE_POLICY_REJECT_5427FA);
    reset(); DiploScratchFA6C=0;
    d=tech_trade_pre_offer_policy(1,2,0,1,13,false);
    assert(d.route==TECH_TRADE_POLICY_READY_FOR_OFFERS);
    return 0;
}
'''


def _tech_policy_candidate_text(source):
    marker = "static TechTradeContactResult tech_trade_contact_policy"
    header_end = source.index(marker)
    header = source[source.index('enum TechTradeContactResult {'):header_end]
    signatures = (
        r"static TechTradeContactResult\s+tech_trade_contact_policy\s*\(",
        r"static void\s+tech_trade_show_contact_rejection\s*\(",
        r"static TechTradePolicyDecision\s+tech_trade_pre_offer_policy\s*\(",
        r"static TechTradePolicyDecision\s+tech_trade_policy_prefix\s*\(",
    )
    bodies = [definition(source, signature) for signature in signatures]
    return header + "\n".join(bodies)


def _run_tech_policy_controls():
    source = (ROOT / 'src/faction.cpp').read_text()
    actual = _tech_policy_candidate_text(source)
    contact = definition(source, r"static TechTradeContactResult\s+tech_trade_contact_policy\s*\(")
    prefix = definition(source, r"static TechTradePolicyDecision\s+tech_trade_policy_prefix\s*\(")
    mutations = {
        "wrong_identity_field": actual.replace("Players[partner].filename",
                                                "Players[partner].search_key", 1),
        "wrong_FA60_polarity": actual.replace("DiploScratchFA60 != 0",
                                               "DiploScratchFA60 == 0", 1),
        "wrong_treaty_row": actual.replace(
            "PlayersData[partner].diplo_treaties[actor]",
            "PlayersData[actor].diplo_treaties[partner]", 1),
        "inclusive_friction_boundary": actual.replace("DiploFriction > 0x11",
                                                       "DiploFriction >= 0x11", 1),
        "cxx11_aggregate_return": actual.replace(
            "TechTradePolicyDecision decision = {\n"
            "            TECH_TRADE_POLICY_REJECT_542097, false, special_action};\n"
            "        return decision;",
            "return {TECH_TRADE_POLICY_REJECT_542097, false, special_action};", 1),
    }
    assert contact in actual and prefix in actual
    for name, mutated in mutations.items():
        assert mutated != actual, "mutation anchor drift: " + name

    failures = []
    ran = 0
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if not compiler:
            print("SKIP", compiler_name, "not installed")
            continue
        ran += 1
        with tempfile.TemporaryDirectory(prefix="tech-policy-", dir=None) as temp:
            scratch = Path(temp)
            candidates = {"actual": actual}
            candidates.update(mutations)
            for name, candidate in candidates.items():
                cpp, binary = scratch / (name + ".cpp"), scratch / name
                cpp.write_text(host_fixture(_tech_policy_fixture).replace("// @@CANDIDATE@@", candidate, 1))
                compiled = subprocess.run(
                    [compiler, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra",
                     "-Wno-unused-parameter",
                     "-fsanitize=undefined", "-fno-sanitize-recover=all",
                     str(cpp), "-o", str(binary)],
                    capture_output=True, text=True, timeout=45)
                if name == "cxx11_aggregate_return":
                    if compiled.returncode == 0:
                        failures.append(compiler_name + "/" + name +
                                        " unexpectedly compiled in C++98")
                    continue
                if compiled.returncode:
                    failures.append(compiler_name + "/" + name + " compile:\n" +
                                    compiled.stdout + compiled.stderr)
                    continue
                result = subprocess.run([str(binary)], capture_output=True,
                                        text=True, timeout=15)
                expected_pass = name == "actual"
                if (result.returncode == 0) != expected_pass:
                    failures.append(compiler_name + "/" + name + " unexpected result:\n" +
                                    result.stdout + result.stderr)
            if not failures:
                print("PASS", compiler_name,
                      "actual extracted helpers and four detected behavior mutations; C++11 return rejected")
    if ran == 0:
        raise SystemExit("No local GCC or Clang compiler is available")
    if failures:
        raise SystemExit("\n".join(failures))
    print("C++98 fake-boundary controls only; native inspection is evidence input, not acceptance.")
    return 0



def test_actual_tech_trade_policy_controls():
    assert _run_tech_policy_controls() == 0



@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_propose_attack(tmp_path, compiler_name):
    """Real owning body, with service doubles; not game/native acceptance."""
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    attack = definition(source, r'int\s+__cdecl\s+propose_attack\s*\(')
    helpers = '\n'.join(
        definition(source, r'static int\s+diplo_' + name + r'\s*\([^)]*\)(?=\s*\{)')
        for name in ('add', 'sub', 'mul'))
    fixture = r'''#include <cassert>
    #include <cstdio>
    #include <cstring>
    #include <stdint.h>
    #define __cdecl
    typedef unsigned char uint8_t;
    struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
    struct Player {
        int is_leader_female,noun_gender,is_noun_plural;
        char title_leader[24],name_leader[24],noun_faction[24],adj_name_faction[24];
        char adj_insult_faction[24],adj_name_faction_unused[24],filename[24];
    } Players[8];
    struct PlayerData {
        uint32_t diplo_treaties[8],flags,ranking;
        int integrity_blemishes,mil_strength_1,ai_fight,energy_reserves,diff_level;
        int diplo_unk1[8],diplo_wrongs[8];
        uint8_t region_total_bases[128];
        int diplo_friction[8];
    } PlayersData[8];
    const uint32_t DTREATY_PACT=0x1u, DTREATY_COMMLINK=0x8u;
    const uint32_t DTREATY_VENDETTA=0x10u, DTREATY_WANT_REVENGE=0x20u;
    const uint32_t DTREATY_HAVE_SURRENDERED=0x02000000u;
    uint32_t FactionRankings[8],FactionRankingsUnk[8];
    int DiploScratchFA5C,DiploScratchFA78,DiploScratchF7FC,DiploScratchF7F0;
    int DiploScratchFA54,DiploScratchFA3C,DiploScratchFA98;
    int DiploScratchFAA8,DiploScratchFA18,DiploScratchFA1C,DiploScratchFA28;
    int DiploFriction,DiploFrictionFactionID,DiploFrictionFactionIDWith;
    int TurnCurrentNum,DiplomacyProposalMode,IsMultiplayerNet,ExitTurnLoop;
    int GenderDefault,PluralityDefault,PopupDialogFactionID,LockEnableMask;
    char StringTemp[256],PopupStartCaption[64];
    const char *DiploLabelSendingTREATYON="Sending TREATY ON";
    const char *DiploLabelReleasingDIPLOMATICLOCK="Releasing DIPLOMATIC LOCK";
    int contiguous_result=1, short_response, tech_offer_response, random_result;
    int energy_popup_answer, tech_values[128], tech_value_calls, tech_transfer_count;
    int transferred_tech[8],transfer_to[8],transfer_from[8],mutate_live_ids;
    int wants_results[4],wants_calls,wants_a[4],wants_b[4];
    int lock_add_calls,lock_unlock_calls,treaty_on_calls,treaty_a,treaty_b,treaty_bits;
    int message_count,message_op[16],message_a[16],message_b[16],message_c[16],message_d[16];
    int await_count,await_op[16],double_cross_calls,net_set_calls,net_set_a[16],net_set_b[16],net_set_bits[16];
    int popup_start_count,popup_exec_count,popup_destroy_count,short_popup_count,price_modal_count;
    int console_update_one,console_update_zero,energy_price,parse_num_calls,tech_offer_calls;
    int contiguous_calls,random_calls,incited_calls,pact_end_calls,set_treaty_calls;
    int short_keys_count; char short_keys[24][64];
    int get_him_her_calls;
    int pop_wait(){return 0;}
    struct Dialogs { void sprite(Sprite *s,char *text,int n) {
        assert(s==FactionDiplomacyPortraits[2]&&!text&&!n); } };
    struct Popup {
        Dialogs d;
        Popup(){} ~Popup(){++popup_destroy_count;}
        void close(){}
        bool picture_enabled(){return false;}
        void set_caller_picture(Sprite *s){assert(s==FactionDiplomacyPortraits[2]);}
        Dialogs &dialogs(){return d;}
        void start(char *,const char *key,int,char *,int,int) {
            ++popup_start_count; if(key && !std::strcmp(key,"MAYBEWARPRICE")) ++price_modal_count;
        }
        int exec(int n,int(*cb)()) { assert(n==0&&cb==pop_wait); ++popup_exec_count; return energy_popup_answer; }
    };
    int contiguous(int a,int b,int region,int mode) {
        assert(a==2&&b==3&&region==0&&mode==1); ++contiguous_calls; return contiguous_result;
    }
    int wants_to_attack(int a,int b,int z) {
        assert(z==0&&wants_calls<4); wants_a[wants_calls]=a; wants_b[wants_calls]=b;
        return wants_results[wants_calls++];
    }
    void parse_says(int,char *,int,int){}
    void parse_num(int slot,int value) { assert(slot==0); ++parse_num_calls; energy_price=value; }
    char *get_pact(int){return Players[3].noun_faction;}
    char *get_pacts(int){return Players[3].noun_faction;}
    char *get_phrase(int,int){return Players[3].noun_faction;}
    char *get_him_her(int who,int form){assert(who==3&&form==0);++get_him_her_calls;return Players[3].noun_faction;}
    int X_pops(const char *key,int flags,Sprite *s,int(*cb)()) {
        assert(key&&flags==0x100000&&s==FactionDiplomacyPortraits[2]&&cb==pop_wait);
        if(short_keys_count<24) std::strncpy(short_keys[short_keys_count++],key,63);
        if(!std::strncmp(key,"MAYBEWARTECH",12)) { ++tech_offer_calls; return tech_offer_response; }
        ++short_popup_count; return short_response;
    }
    void x_pop_short(const char *,int(*)()){++incited_calls;}
    void log_say(const char *,int,int,int){}
    int great_satan(int,int){return 0;}
    void X_pop(const char *,int(*)()){++incited_calls;}
    void message_data(int op,int z,int a,int b,int c,int d) {
        assert(!z&&message_count<16); int i=message_count++;
        message_op[i]=op;message_a[i]=a;message_b[i]=b;message_c[i]=c;message_d[i]=d;
    }
    struct Net {
        int add_global(){++lock_add_calls;return 0;}
        void unlock(){++lock_unlock_calls;}
        void await_diplo(int op){assert(await_count<16);await_op[await_count++]=op;
            if(mutate_live_ids&&op==0x44b&&await_count==1) DiploScratchFA1C=51;}
    } NetDaemonNet;
    struct Console { void update_data(int n){if(n)++console_update_one;else ++console_update_zero;} } ConsoleGlobal;
    struct Info { void set_redraw_phase(int n){assert(!n);} } ConsoleInfoWin;
    int tech_val(int tech,int faction,int simple) {
        assert(faction>=0&&faction<8&&simple==1&&tech>=0&&tech<128);
        ++tech_value_calls;return tech_values[tech];
    }
    void say_tech(char *buffer,int tech,int){std::sprintf(buffer,"T%d",tech);}
    void tech_achieved(int who,int tech,int other,int z) {
        assert(z==0&&tech_transfer_count<8); int i=tech_transfer_count++;
        transfer_to[i]=who;transferred_tech[i]=tech;transfer_from[i]=other;
        if(mutate_live_ids&&i==0) DiploScratchFA1C=51;
    }
    int random(int a,int b){assert(a==0&&b==2);++random_calls;return random_result;}
    char *_itoa(int n,char *s,int base){assert(base==10);std::sprintf(s,"%d",n);return s;}
    unsigned long timeGetTime(){return 0;}
    void check_spock(){}
    void treaty_on(int a,int b,int bits){++treaty_on_calls;treaty_a=a;treaty_b=b;treaty_bits=bits;}
    void net_set_treaty(int a,int b,int bits,int on,int wait){assert(on==1&&(wait==0||wait==1));
        assert(net_set_calls<16);int i=net_set_calls++;net_set_a[i]=a;net_set_b[i]=b;net_set_bits[i]=bits;}
    void net_pact_ends(int,int,int){++pact_end_calls;}
    void set_treaty(int,int,int,int){++set_treaty_calls;}
    void double_cross(int a,int b,int c){assert(a==2&&b==3&&c==1);++double_cross_calls;}
    @@HELPERS@@
    @@BODY@@
    void reset(){
        std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
        std::memset(FactionRankings,0,sizeof(FactionRankings));std::memset(FactionRankingsUnk,0,sizeof(FactionRankingsUnk));
        std::memset(tech_values,0,sizeof(tech_values));std::memset(wants_results,0,sizeof(wants_results));
        std::memset(wants_a,0,sizeof(wants_a));std::memset(wants_b,0,sizeof(wants_b));
        std::memset(message_op,0,sizeof(message_op));std::memset(message_a,0,sizeof(message_a));
        std::memset(message_b,0,sizeof(message_b));std::memset(message_c,0,sizeof(message_c));std::memset(message_d,0,sizeof(message_d));
        std::memset(await_op,0,sizeof(await_op));std::memset(net_set_a,0,sizeof(net_set_a));
        std::memset(net_set_b,0,sizeof(net_set_b));std::memset(net_set_bits,0,sizeof(net_set_bits));
        std::memset(short_keys,0,sizeof(short_keys));
        for(int i=0;i<8;++i)FactionDiplomacyPortraits[i]=&portraits[i];
        std::strcpy(Players[1].title_leader,"Actor");std::strcpy(Players[2].title_leader,"Other");
        std::strcpy(Players[3].title_leader,"Target");std::strcpy(Players[3].noun_faction,"Faction");
        std::strcpy(Players[3].adj_name_faction,"TargetName");std::strcpy(Players[3].adj_insult_faction,"Insult");
        std::strcpy(Players[2].filename,"OTHER");
        DiploScratchFA5C=3;DiploScratchFA78=2000;DiploScratchF7FC=DiploScratchF7F0=0;
        DiploScratchFA54=DiploScratchFA3C=DiploScratchFA98=0;
        DiploScratchFAA8=DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1;
        DiploFriction=DiploFrictionFactionID=DiploFrictionFactionIDWith=0;
        TurnCurrentNum=10;DiplomacyProposalMode=2;IsMultiplayerNet=0;ExitTurnLoop=0;
        GenderDefault=PluralityDefault=PopupDialogFactionID=0;LockEnableMask=0x8;
        PlayersData[1].energy_reserves=1000;PlayersData[0].energy_reserves=1000;
        PlayersData[2].energy_reserves=0;PlayersData[2].ai_fight=1;
        PlayersData[3].mil_strength_1=0;PlayersData[3].ranking=0;
        PlayersData[2].diplo_treaties[1]=DTREATY_PACT|DTREATY_WANT_REVENGE;
        PlayersData[3].diplo_treaties[1]=DTREATY_WANT_REVENGE;
        contiguous_result=1;short_response=0;tech_offer_response=1;random_result=1;
        energy_popup_answer=0;mutate_live_ids=0;
        tech_value_calls=tech_transfer_count=0;std::memset(transferred_tech,0,sizeof(transferred_tech));
        std::memset(transfer_to,0,sizeof(transfer_to));std::memset(transfer_from,0,sizeof(transfer_from));
        wants_calls=lock_add_calls=lock_unlock_calls=treaty_on_calls=message_count=await_count=0;
        double_cross_calls=net_set_calls=popup_start_count=popup_exec_count=popup_destroy_count=0;
        short_popup_count=price_modal_count=console_update_one=console_update_zero=energy_price=parse_num_calls=tech_offer_calls=0;
        contiguous_calls=random_calls=incited_calls=pact_end_calls=set_treaty_calls=0;
        short_keys_count=get_him_her_calls=0;
    }
    int main(){
        // Declining the energy offer must not transfer reserves or enter treaty lock.
        reset(); energy_popup_answer=0;
        assert(propose_attack(1,2)==0);
        assert(parse_num_calls==1&&energy_price==1000&&popup_exec_count==1);
        assert(price_modal_count==1);
        assert(PlayersData[1].energy_reserves==1000&&PlayersData[2].energy_reserves==0);
        assert(!lock_add_calls&&!lock_unlock_calls&&!treaty_on_calls);

        // Class 2 has no common region; one matching region changes it to class 1.
        reset(); contiguous_result=0; PlayersData[2].diplo_treaties[1]|=DTREATY_PACT;
        PlayersData[2].diplo_treaties[3]=0; PlayersData[2].ai_fight=1;
        PlayersData[2].diplo_treaties[1]|=DTREATY_WANT_REVENGE;
        energy_popup_answer=0; assert(propose_attack(1,2)==0);
        assert(parse_num_calls==1&&energy_price==500);
        reset(); contiguous_result=0; PlayersData[2].diplo_treaties[1]|=DTREATY_PACT;
        PlayersData[2].diplo_treaties[3]=0; PlayersData[2].ai_fight=1;
        PlayersData[2].region_total_bases[7]=1;PlayersData[3].region_total_bases[7]=1;
        energy_popup_answer=0; assert(propose_attack(1,2)==0);
        assert(parse_num_calls==1&&energy_price==750);

        // Equality in the target-power / next-power comparison takes the /4 path.
        reset(); DiploScratchFA78=1000; FactionRankings[7]=3;FactionRankings[6]=4;
        FactionRankingsUnk[3]=10;FactionRankingsUnk[4]=5;energy_popup_answer=0;
        assert(propose_attack(1,2)==0);int equality_price=energy_price;
        reset(); DiploScratchFA78=1000;FactionRankings[7]=3;FactionRankings[6]=4;
        FactionRankingsUnk[3]=9;FactionRankingsUnk[4]=5;energy_popup_answer=0;
        assert(propose_attack(1,2)==0);assert(energy_price>equality_price);

        // Actor faction zero is not charged on an accepted local energy offer.
        reset(); DiplomacyProposalMode=2;DiploScratchFA78=1000;
        PlayersData[2].diplo_treaties[0]=DTREATY_PACT|DTREATY_WANT_REVENGE;
        energy_popup_answer=1;assert(propose_attack(0,2)==1);
        assert(PlayersData[0].energy_reserves==1000&&PlayersData[2].energy_reserves>0);
        assert(console_update_one==1&&lock_add_calls==1&&lock_unlock_calls==1);

        // The original small tech values do not meet the corrected 1000 offer
        // price: 4 * (35 * ((10 + 7) / 8)) = 280. A positive popup response
        // cannot accept an offer that was never shown.
        reset(); DiplomacyProposalMode=4;DiploScratchFA78=2000;
        DiploScratchFA28=40;DiploScratchFA1C=30;DiploScratchFA18=20;DiploScratchFAA8=10;
        tech_values[40]=tech_values[30]=tech_values[20]=tech_values[10]=35;
        tech_offer_response=1;mutate_live_ids=1;assert(propose_attack(1,2)==0);
        assert(tech_value_calls==4&&tech_transfer_count==0&&tech_offer_calls==0);
        assert(short_keys_count==1&&!std::strcmp(short_keys[0],"REJWAR"));

        // A funded complete tech offer is transferred in reverse selector order
        // and each selector is re-read after the preceding award changes state.
        // Four 140-value selectors total 1120, enough for price 1000 but below
        // the 1333 cutoff, so none is pruned before transfer.
        reset(); DiplomacyProposalMode=4;DiploScratchFA78=2000;
        DiploScratchFA28=40;DiploScratchFA1C=30;DiploScratchFA18=20;DiploScratchFAA8=10;
        tech_values[40]=tech_values[30]=tech_values[20]=tech_values[10]=140;
        tech_offer_response=1;mutate_live_ids=1;assert(propose_attack(1,2)==1);
        assert(tech_transfer_count==4);
        assert(transferred_tech[0]==40&&transferred_tech[1]==51&&
               transferred_tech[2]==20&&transferred_tech[3]==10);
        assert(transfer_to[0]==2&&transfer_from[0]==1);
        assert(DiploScratchFA28==-1&&DiploScratchFA1C==-1&&
               DiploScratchFA18==-1&&DiploScratchFAA8==-1);

        // PEACE uses 3/2 of the updated total for each of the three cutoffs.
        // With total 1680, PEACE updates the total to 2520 then uses 3780 for all
        // three cutoffs; the ordinary cutoff is 1333 and all three selectors prune.
        // A per-checkpoint cutoff would leave the final two selectors visible.
        reset(); DiplomacyProposalMode=4;DiploScratchFA78=2000;
        DiploScratchFA28=40;DiploScratchFA1C=30;DiploScratchFA18=20;DiploScratchFAA8=10;
        tech_values[40]=tech_values[30]=tech_values[20]=200;tech_values[10]=240;
        std::strcpy(Players[2].filename,"PEACE");tech_offer_response=0;
        assert(propose_attack(1,2)==0);
        assert(tech_value_calls==4&&DiploScratchFA28==-1&&DiploScratchFA1C==-1&&
               DiploScratchFA18==-1&&DiploScratchFAA8==10);
        assert(short_keys_count==1&&!std::strcmp(short_keys[0],"MAYBEWARTECH"));

        // Existing surrendered pact accepts the initial treaty differently offline
        // and online; this branch must not perform the separate double-cross action.
        reset(); DiplomacyProposalMode=3;
        PlayersData[2].diplo_treaties[1]=DTREATY_PACT|DTREATY_HAVE_SURRENDERED;
        PlayersData[3].diplo_treaties[2]=DTREATY_PACT;
        PlayersData[1].diplo_treaties[3]=DTREATY_VENDETTA;
        energy_popup_answer=1; PlayersData[1].diplo_treaties[3]=DTREATY_PACT;
        PlayersData[1].diff_level=2; random_result=0;
        DiploFrictionFactionID=3;DiploFrictionFactionIDWith=1;DiploFriction=7;
        assert(propose_attack(1,2)==1);
        assert(treaty_on_calls==1&&treaty_a==2&&treaty_b==3&&treaty_bits==0x80000018);
        assert(message_count==0&&double_cross_calls==0&&lock_add_calls==1&&lock_unlock_calls==1);
        assert(net_set_calls==2&&net_set_a[0]==2&&net_set_b[0]==3&&net_set_bits[0]==0x40);
        assert(net_set_a[1]==2&&net_set_b[1]==1&&net_set_bits[1]==0x80);
        assert(incited_calls==1&&pact_end_calls==1&&PlayersData[3].diplo_friction[1]==4);
        assert(DiploFriction==11);
        reset(); DiplomacyProposalMode=3;IsMultiplayerNet=1;
        PlayersData[2].diplo_treaties[1]=DTREATY_PACT|DTREATY_HAVE_SURRENDERED;
        PlayersData[3].diplo_treaties[2]=DTREATY_PACT;
        PlayersData[1].diplo_treaties[3]=DTREATY_VENDETTA;
        energy_popup_answer=1; PlayersData[1].diplo_treaties[3]=DTREATY_PACT;
        PlayersData[1].diff_level=2; random_result=0;
        assert(propose_attack(1,2)==1);
        assert(treaty_on_calls==0&&message_count==1&&message_op[0]==0x2441);
        assert(message_a[0]==2&&message_b[0]==3&&message_c[0]==0x80000018);
        assert(double_cross_calls==0&&lock_add_calls==1&&lock_unlock_calls==1);
        assert(net_set_calls==2&&net_set_a[1]==2&&net_set_b[1]==1&&net_set_bits[1]==0x80);

        // The directed actor->target treaty value selects the treaty-on result;
        // the second staged update is target->actor with the native 0x20/wait=1.
        reset(); DiplomacyProposalMode=3;PlayersData[2].diplo_treaties[1]=DTREATY_PACT|DTREATY_HAVE_SURRENDERED;
        PlayersData[1].diplo_treaties[3]=2;PlayersData[1].diff_level=2;random_result=0;
        PlayersData[3].diplo_friction[1]=18;DiploFrictionFactionID=3;DiploFrictionFactionIDWith=1;
        DiploFriction=19;energy_popup_answer=1;
        assert(propose_attack(1,2)==1);
        assert(treaty_on_calls==2&&treaty_a==1&&treaty_b==3&&treaty_bits==0x10);
        assert(incited_calls==1&&net_set_calls==3);
        assert(net_set_a[2]==3&&net_set_b[2]==1&&net_set_bits[2]==0x20);
        assert(PlayersData[3].diplo_friction[1]==20&&DiploFriction==23);

        // With no tech selector and no eligible energy offer, the no-offer
        // rejection branch uses get_him_her(target, 0), not the pact formatter.
        reset();DiplomacyProposalMode=4;assert(propose_attack(1,2)==0);
        assert(get_him_her_calls==1&&short_keys_count==1&&
               !std::strcmp(short_keys[0],"REJWAR"));

        // Mode 3's FA78 refusal is a silent zero-return cleanup edge.
        reset(); DiplomacyProposalMode=3;DiploScratchFA78=6;
        PlayersData[2].diplo_treaties[1]=DTREATY_WANT_REVENGE;
        assert(propose_attack(1,2)==0);
        assert(short_popup_count==0&&lock_add_calls==0&&lock_unlock_calls==0);
        return 0;
    }
    '''
    anchors = {
            "per_checkpoint_PEACE_cutoff": (
                'first_value = second_value = third_value =\n'
                '            diplo_mul(technology_value, 3) / 2;',
                'first_value = diplo_mul(first_value, 3) / 2;\n'
                '        second_value = diplo_mul(second_value, 3) / 2;\n'
                '        third_value = diplo_mul(third_value, 3) / 2;'),
            "offline_lock_double_cross": (
                '    goto attack_accept_treaty;\n}',
                '    double_cross(other, target, actor);\n'
                '    goto attack_accept_treaty;\n}'),
            "wrong_incited_direction": (
                'PlayersData[actor].diplo_treaties[target];',
                'PlayersData[target].diplo_treaties[actor];'),
            "decline_is_accept": ('popup.exec(0, pop_wait) > 0',
                                   'popup.exec(0, pop_wait) >= 0'),
            "wrong_tech_order": (
                'const int tech = selection == 0 ? DiploScratchFA28\n'
                '                    : selection == 1 ? DiploScratchFA1C\n'
                '                    : selection == 2 ? DiploScratchFA18 : DiploScratchFAA8;',
                'const int tech = selection == 0 ? DiploScratchFAA8\n'
                '                    : selection == 1 ? DiploScratchFA1C\n'
                '                    : selection == 2 ? DiploScratchFA18 : DiploScratchFA28;'),
            "charge_actor_zero": ('if (actor != 0) PlayersData[actor].energy_reserves =',
                                  'if (actor == 0) PlayersData[actor].energy_reserves ='),
            "mode3_refusal_not_silent": (
                'if (attack_cached_mode == 3) {\n'
                '            goto attack_return_zero;\n'
                '        }',
                'if (attack_cached_mode == 3) {\n'
                '            goto attack_trade_offer;\n'
                '        }'),
            "power_equality_strict": ('if (target_power >= diplo_mul(next_ranked_power, 2))',
                                      'if (target_power > diplo_mul(next_ranked_power, 2))'),
        }

    variants = {'actual': attack}
    for name, (old, new) in anchors.items():
        assert old in attack, 'mutation anchor drift: ' + name
        variants[name] = attack.replace(old, new, 1)
    for name, body in variants.items():
        cpp = tmp_path / (name + '.cpp')
        binary = tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@HELPERS@@', helpers, 1)
                       .replace('@@BODY@@', body, 1))
        built = subprocess.run(
            [compiler, '-std=c++98', '-pedantic-errors', '-Wall', '-Wextra',
             '-Wno-unused-parameter', '-fsanitize=undefined',
             '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
            capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, name + '\n' + built.stdout + built.stderr
        result = subprocess.run([str(binary)], capture_output=True,
                                text=True, timeout=20)
        assert (result.returncode == 0) == (name == 'actual'), (
            name + '\n' + result.stdout + result.stderr)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_make_gift(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'void __cdecl make_gift\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul', 'diplo_clamp'))
    fixture = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char name_leader[24],title_leader[24],noun_faction[24],filename[24],search_key[24]; } Players[8];
struct Data { int energy_reserves,integrity_blemishes,diplo_unk1[8];
 unsigned diplo_treaties[8],diplo_friction[8]; char diplo_patience[8]; } PlayersData[8];
int DiplomacyProposalMode,PopupDialogFactionID,DiploScratchFAA8,DiploScratchFA18;
int DiploScratchFA30,DiploScratchFA98,DiploScratchFA68,DiploScratchFAA0;
unsigned DiploFrictionFactionID,DiploFrictionFactionIDWith;
int DiploFriction,GenderDefault,PluralityDefault,TurnCurrentNum,ExitTurnLoop,IsMultiplayerNet;
char StringTemp[128],PopupStartCaption[80];
int selection,response,destructors,items[20],item_count,redraws,awaits,last_await,picture_calls;
int techs[90],tech_count,base_count,satan,base_value,mutate_mode,mutate_choice,transfer_amount;
char notice[80];
int pop_wait() { return 0; }
struct Dialogs {
 void sprite(Sprite *s,char *t,int n) { assert(s==FactionDiplomacyPortraits[2] && !t && !n); }
 void item(char *s,int n) { assert(item_count<20 && s[0]); items[item_count++]=n; }
};
struct Popup {
 Dialogs menu; ~Popup() { ++destructors; }
 void start(char *s,const char *key,int n,char *t,int flags,int z=0) { assert(s==PopupStartCaption && !std::strcmp(key,"OFFERENERGY") && n==-1 && !t && flags==0x100000 && !z); }
 bool picture_enabled() { return false; }
 void set_caller_picture(Sprite *s) { assert(s==FactionDiplomacyPortraits[2]); ++picture_calls; }
 Dialogs &dialogs() { return menu; }
 int exec(int n,int(*cb)()) { assert(!n && cb==pop_wait); return selection; }
};
char *_itoa(int n,char *d,int base) { assert(base==10); std::sprintf(d,"%d",n); return d; }
void diplomacy_caption(int a,int b) { assert(a==1 && b==2); }
int parse_says(int n,char *s,int a,int b) { assert(a==-1 && b==-1 && n>=0 && n<=2 && s); return 0; }
int X_pops(const char *s,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000 && p==FactionDiplomacyPortraits[2] && cb==pop_wait); std::strcpy(notice,s);
 if(mutate_choice && (!std::strcmp(s,"OFFERTECH") || !std::strcmp(s,"OFFERTECH2"))) DiploScratchFAA8=5;
 return response;
}
struct Console { void update_data(int n) { redraws+=n ? 1 : 10; } } ConsoleGlobal;
struct Info { void set_redraw_phase(int n) { assert(!n); } } ConsoleInfoWin;
int great_satan(int a,int z) { assert(a==1 && !z); return satan; }
void treaty_off(int a,int b,int flags) { assert((a==1 && b==2)||(a==2 && b==1)); assert(flags==0x60); }
void log_say(const char *s,int a,int b,int c) { assert(s && a>=0 && b>=0); }
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z);
 if(op==0x2447) { assert(a==1 && c==2 && b==-selection && d==selection); transfer_amount=d; }
 else if(op==0x244b) { assert(a==2 && c==1 && !d); techs[tech_count++]=b; }
 else if(op==0x2449) { assert(a==DiploScratchFA30 && b==2 && !c && !d); ++base_count; }
 else assert(op==0x2442 && c==0x60 && !d);
}
struct Net { int add_global() { return 0; } void unlock() {} void await_diplo(int op) { ++awaits; last_await=op; } } NetDaemonNet;
unsigned timeGetTime() { return 0; }
void check_spock() { assert(false); }
void tech_analysis(int a,int b) { assert(a==1 && b==2); if(mutate_mode) DiplomacyProposalMode=7; }
int has_tech(int tech,int owner) { return owner==1 && (tech==3 || tech==5); }
int tech_val(int tech,int owner,int simple) { assert(owner==2 && simple==1); return tech==3 ? 10 : 20; }
void tech_achieved(int owner,int tech,int giver,int z) { assert(owner==2 && giver==1 && !z); techs[tech_count++]=tech; }
void say_tech(char *s,int tech,int cat) { assert(s==StringTemp && !s[0] && cat==1); std::strcpy(s,"technology"); }
char *get_noun_phrase(int owner,int n) { assert(owner==2 && n==9); static char phrase[]="phrase"; return phrase; }
int value_of_base(int base,int req,int res,int counter,int contact) { assert(base==DiploScratchFA30 && req==2 && res==1); return base_value; }
void give_a_base(int base,int owner) { assert(base==DiploScratchFA30 && owner==2); ++base_count; }
@@BODY@@
void reset(int mode) {
 std::memset(Players,0,sizeof(Players)); std::memset(PlayersData,0,sizeof(PlayersData));
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 DiplomacyProposalMode=mode; DiploScratchFAA8=3; DiploScratchFA18=-1;
 DiploScratchFA30=DiploScratchFA98=DiploScratchFA68=DiploScratchFAA0=0;
 selection=50; response=1; destructors=item_count=redraws=awaits=tech_count=base_count=satan=mutate_mode=mutate_choice=transfer_amount=picture_calls=0;
 DiploFrictionFactionID=2; DiploFrictionFactionIDWith=1; DiploFriction=10;
 PlayersData[2].diplo_friction[1]=10; PlayersData[1].energy_reserves=100; PlayersData[2].energy_reserves=0;
 TurnCurrentNum=1; ExitTurnLoop=IsMultiplayerNet=0; base_value=40;
 PlayersData[1].integrity_blemishes=3; PlayersData[2].diplo_patience[1]=5; notice[0]=0;
}
void run() { const int mode=DiplomacyProposalMode; make_gift(1,2); assert(destructors==1 && PopupDialogFactionID==2 && picture_calls==(mode==5 ? 1 : 0)); }
int main() {
 reset(1); run(); assert(!notice[0] && !item_count);
 reset(5); selection=0; run(); assert(item_count==4 && items[0]==100 && items[1]==50 && items[2]==25 && items[3]==0 && !redraws);
 reset(5); std::strcpy(Players[1].search_key,"MORGAN"); run(); assert(PlayersData[1].energy_reserves==50 && PlayersData[2].energy_reserves==50 && redraws==11 && PlayersData[2].diplo_friction[1]==7 && DiploFriction==7 && !std::strcmp(notice,"GAVEENERGY1") && PlayersData[2].diplo_patience[1]==2 && PlayersData[1].diplo_unk1[2]==2);
 reset(5); IsMultiplayerNet=1; std::strcpy(Players[1].search_key,"MORGAN"); run(); assert(transfer_amount==50 && redraws==10 && PlayersData[1].energy_reserves==100 && awaits==2);
 reset(4); DiploScratchFAA8=-1; run(); assert(!std::strcmp(notice,"REJTECHNOTHING") && !tech_count);
 reset(4); response=0; run(); assert(!tech_count);
 reset(4); mutate_choice=1; run(); assert(tech_count==1 && techs[0]==5 && !std::strcmp(notice,"GAVETECH1") && PlayersData[2].diplo_patience[1]==3);
 reset(4); DiploScratchFA18=5; response=2; IsMultiplayerNet=1; run(); assert(tech_count==1 && techs[0]==5 && awaits==2);
 reset(4); mutate_mode=1; run(); assert(tech_count==2 && techs[0]==3 && techs[1]==5);
 reset(7); std::strcpy(Players[2].search_key,"BELIEVE"); run(); assert(tech_count==2 && PlayersData[2].diplo_friction[1]==0 && DiploFriction==-5 && !std::strcmp(notice,"GAVETECH"));
 reset(8); run(); assert(base_count==1 && !std::strcmp(notice,"BASEGIFT") && PlayersData[2].diplo_patience[1]==2 && PlayersData[1].diplo_unk1[2]==2);
 reset(8); base_value=0; run(); assert(base_count==1 && PlayersData[1].diplo_unk1[2]==1 && PlayersData[2].diplo_patience[1]==3);
 reset(8); IsMultiplayerNet=1; run(); assert(base_count==1 && awaits==1 && last_await==0x449);
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_menu', '(offered_energy / 2) / 25', '(offered_energy / 4) / 25'),
        ('wrong_money', 'diplo_sub(PlayersData[giver].energy_reserves, selection)', 'diplo_sub(PlayersData[giver].energy_reserves, amount)'),
        ('wrong_bucket', 'amount / 5', 'amount / 10'),
        ('wrong_live_mode', 'if (DiplomacyProposalMode == 7)', 'if (mode == 7)'),
        ('wrong_recipient_modifier', 'strcmp(recipient_key, "BELIEVE")', 'strcmp(recipient_key, "SPARTANS")'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_base_swap(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'void __cdecl base_swap\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul'))
    fixture = r'''
#include <cassert>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
const int MaxTechnologyNum=89;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female; char name_leader[24]; } Players[8];
struct Data { int ranking,tech_ranking,energy_reserves,current_num_bases; unsigned diplo_treaties[8]; } PlayersData[8];
struct Base { char name_string[24]; int faction_id_former; unsigned char facilities_built[12]; } Bases[4];
int PopupDialogFactionID,GenderDefault,PluralityDefault,IsMultiplayerNet;
int DiploScratchFA68,DiploScratchFAA0,DiploScratchFA98,DiploScratchFA30,DiploScratchFA7C;
int DiplomacyProposalMode,TurnCurrentNum,ExitTurnLoop;
unsigned FactionRankings[8];
int values[4],response,transfer_count,tech_count,await_count,lock_count,unlock_count;
int random_count,random_values[2],redraw_count,numeric,valuations,tech_scans,mutate_network;
const char *notice;
int transfer_bases[4],transfer_owners[4];
int value_of_base(int base,int req,int res,int counter,int contact) {
 assert(req==2 && res==1);
 if(!valuations) assert(base==DiploScratchFA7C && counter==DiploScratchFA98 && contact==(DiploScratchFA68!=0 || DiploScratchFAA0!=0));
 else assert(base==DiploScratchFA30 && !counter && !contact);
 ++valuations; return base<0 ? -1 : values[base];
}
int pop_wait() { return 0; }
int parse_says(int n,char *s,int a,int b) {
 assert(a==-1 && b==-1);
 if(n==0) assert(s==Bases[0].name_string);
 else if(n==1) assert(s==Bases[1].name_string);
 else assert(n==2 && s==Players[2].name_leader && !PluralityDefault && GenderDefault==1);
 return 0;
}
void bitmask(int n,int *offset,int *mask) { assert(n==1); *offset=0; *mask=2; }
int parse_num(int n,int value) { assert(!n); numeric=value; return 0; }
int X_pops(const char *s,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000 && p==FactionDiplomacyPortraits[2] && cb==pop_wait); notice=s;
 return response;
}
void log_say(const char *s,int a,int b,int v) {
 if(!std::strcmp(s,"Sending ENERGY")) assert(a==1 && b==2 && v==-numeric);
 else if(!std::strcmp(s,"Sending TECH")) assert(a==2 && b==3 && v==1);
 else if(!std::strcmp(s,"Sending CEDE BASE")) assert(a==0 || a==1);
 else assert(!a && !b && !v);
}
unsigned timeGetTime() { return 0; }
void check_spock() { assert(false); }
struct Net {
 int add_global() { ++lock_count; return 0; }
 void unlock() { ++unlock_count; }
 void await_diplo(int op) { assert(op==0x449); ++await_count; }
} NetDaemonNet;
struct Console { void update_data(int n) { assert(n==1); ++redraw_count; } } ConsoleGlobal;
void give_a_base(int b,int owner) {
 transfer_bases[transfer_count]=b; transfer_owners[transfer_count++]=owner;
 if(mutate_network) IsMultiplayerNet=1;
}
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z);
 if(op==0x2447) assert(a==1 && b==-numeric && c==2 && d==numeric);
 else if(op==0x244b) { assert(a==2 && b==3 && c==1 && !d); ++tech_count; }
 else { assert(op==0x2449 && !c && !d); transfer_bases[transfer_count]=a; transfer_owners[transfer_count++]=b; }
}
int has_tech(int tech,int owner) { ++tech_scans; return owner==1 && tech==3; }
int tech_val(int tech,int owner,int simple) { assert(tech==3 && owner==2 && simple==1); return 100; }
void tech_achieved(int owner,int tech,int giver,int z) { assert(owner==2 && tech==3 && giver==1 && !z); ++tech_count; }
int random(int a,int b) { assert(!a && b==100 && random_count<2); return random_values[random_count++]; }
@@BODY@@
void reset() {
 std::memset(PlayersData,0,sizeof(PlayersData)); std::memset(Bases,0,sizeof(Bases)); std::memset(FactionRankings,0,sizeof(FactionRankings));
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 DiploScratchFA68=DiploScratchFAA0=DiploScratchFA98=IsMultiplayerNet=ExitTurnLoop=0;
 DiploScratchFA7C=0; DiploScratchFA30=-1; DiplomacyProposalMode=5; TurnCurrentNum=8;
 response=1; transfer_count=tech_count=await_count=lock_count=unlock_count=random_count=redraw_count=numeric=valuations=tech_scans=mutate_network=0; notice=0;
 values[0]=20; values[1]=10; random_values[0]=random_values[1]=0;
 PlayersData[1].ranking=PlayersData[2].ranking=2;
 PlayersData[1].energy_reserves=100; PlayersData[2].energy_reserves=5;
 PlayersData[1].current_num_bases=PlayersData[2].current_num_bases=2;
 Players[2].is_leader_female=1;
}
void run() { base_swap(1,2); assert(PopupDialogFactionID==2 && valuations==2); }
void label(const char *s) { assert(notice && !std::strcmp(notice,s)); }
int main() {
 reset(); Bases[0].facilities_built[0]=2; run(); label("NOMYHQ"); assert(!transfer_count);
 reset(); DiploScratchFA30=1; PlayersData[1].current_num_bases=1; run(); label("NOHQ");
 reset(); PlayersData[2].diplo_treaties[1]=0x20; run(); label("NEVERBASESWAP");
 reset(); response=0; run(); label("PAYBASESWAP"); assert(!transfer_count && numeric==20);
 reset(); run(); label("BASEOK"); assert(transfer_count==1 && transfer_bases[0]==0 && transfer_owners[0]==1 && PlayersData[1].energy_reserves==80 && PlayersData[2].energy_reserves==25 && redraw_count==1 && unlock_count==1);
 reset(); IsMultiplayerNet=1; run(); label("BASEOK"); assert(transfer_count==1 && await_count==1 && !redraw_count && PlayersData[1].energy_reserves==100);
 reset(); Bases[0].faction_id_former=1; PlayersData[2].diplo_treaties[1]=0x2000001; run(); assert(numeric==5);
 reset(); FactionRankings[7]=1; run(); assert(numeric==40);
 reset(); PlayersData[1].energy_reserves=19; run(); label("NOBASESWAP"); assert(!transfer_count);
 reset(); DiplomacyProposalMode=1; run(); label("NOBASESWAP");
 reset(); DiplomacyProposalMode=4; PlayersData[2].tech_ranking=6; run(); label("NOBASESWAP2");
 reset(); DiplomacyProposalMode=4; response=0; run(); label("TECHBASESWAP"); assert(!transfer_count && !tech_count);
 reset(); DiplomacyProposalMode=4; IsMultiplayerNet=1; run(); label("BASEOK"); assert(tech_count==1 && transfer_count==1 && await_count==1 && unlock_count==1);
 reset(); DiplomacyProposalMode=2; PlayersData[1].energy_reserves=0; run(); label("BASEOK"); assert(tech_count==1 && !redraw_count);
 reset(); DiploScratchFA30=1; run(); label("YESBASESWAP"); assert(transfer_count==2 && transfer_bases[0]==0 && transfer_owners[0]==1 && transfer_bases[1]==1 && transfer_owners[1]==2);
 reset(); DiploScratchFA30=1; mutate_network=1; run(); assert(transfer_count==2 && await_count==1);
 reset(); DiploScratchFA30=1; random_values[0]=10; run(); label("NOBASESWAP3"); assert(!transfer_count);
 reset(); DiploScratchFA30=DiploScratchFA7C=-1; run(); assert(!notice && !transfer_count);
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_hq', 'facilities_built[mask] & bit) == 0', 'facilities_built[mask] & bit) != 0'),
        ('wrong_name', '.name_string, -1, -1', '.name_string + 7, -1, -1'),
        ('wrong_former', '.faction_id_former == faction_id_res', '.faction_id_former == faction_id_req'),
        ('wrong_energy', 'diplo_sub(0, amount)', 'amount'),
        ('wrong_tech', 'tech_achieved(faction_id_req, tech_id, faction_id_res, 0)', 'tech_achieved(faction_id_res, tech_id, faction_id_req, 0)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), name


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_call_off_vendetta(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    body = definition((ROOT / 'src/faction.cpp').read_text(),
                      r'void __cdecl call_off_vendetta\([^)]*\)')
    fixture = r'''
#include <cassert>
#include <cstring>
#define __cdecl
const int MaxPlayerNum=8;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char title_leader[24],name_leader[24],adj_insult_faction[24],noun_faction[24]; } Players[8];
struct Data { unsigned diplo_treaties[8]; int major_atrocities; } PlayersData[8];
int PopupDialogFactionID,GenderDefault,PluralityDefault,IsMultiplayerNet;
char StringTemp[80],PopupStartCaption[80],line[]="line",script[]="script";
char *TextBufferGetPtr=line,*ScriptPopupFile=script;
int selected,available,executions,destructors,opens,fail_open,parsed,attack,picture;
int items[8],item_count,local_count,network_count,await_count,log_count;
const char *notice;
int is_alive(int n) { return n==3 || n==4; }
int pop_wait() { return 0; }
int X_text_open(char *s,const char *key) {
 assert(s==script && !std::strcmp(key,"ATTACKSTRING")); ++opens;
 return opens==fail_open;
}
char *text_get() { return line; }
int parse_string(char *s,char *d) { assert(s==line && d==StringTemp && !d[0]); std::strcpy(d,s); return 0; }
int parse_says(int n,char *s,int a,int b) {
 assert(a==-1 && b==-1);
 int who=-1; for(int i=3;i<=4;++i)
  if(s==Players[i].title_leader || s==Players[i].name_leader ||
     s==Players[i].adj_insult_faction || s==Players[i].noun_faction) who=i;
 assert(who>=0);
 if(n==0) assert(s==Players[who].title_leader && GenderDefault==who && !PluralityDefault);
 else if(n==1) { assert(s==Players[who].name_leader && GenderDefault==who && !PluralityDefault); GenderDefault=17; PluralityDefault=19; }
 else if(n==2 && s==Players[who].adj_insult_faction) assert(GenderDefault==17 && PluralityDefault==19);
 else { assert(s==Players[who].noun_faction && GenderDefault==who+8 && PluralityDefault==who+16); assert(n==2 || n==3); }
 ++parsed; return 0;
}
struct Dialogs {
 void sprite(Sprite *s,char *t,int who) { assert(s==FactionDiplomacyPortraits[2] && !t && who==2); }
 void item(char *s,int n) { assert(s==StringTemp && !std::strcmp(s,"line")); items[item_count++]=n; }
};
struct Popup {
 Dialogs menu;
 ~Popup() { ++destructors; }
 void start(char *s,const char *key,int n,char *t,int f,int z) {
  assert(s==PopupStartCaption && !std::strcmp(key,"CALLOFF") && n==-1 && !t && f==0x100042 && !z);
 }
 bool picture_enabled() { return picture!=0; }
 void set_caller_picture(Sprite *s) { assert(s==(picture ? 0 : FactionDiplomacyPortraits[2])); }
 Dialogs &dialogs() { return menu; }
 int exec(int n,int(*cb)()) { assert(!n && cb==pop_wait && item_count>0); ++executions; return selected; }
};
int X_pops(const char *s,int f,Sprite *p,int(*cb)()) {
 assert(f==0x100000 && p==FactionDiplomacyPortraits[2] && !cb); notice=s; return 0;
}
void set_treaty(int a,int b,int bit,int state) {
 assert(a==2 && b==3); assert((local_count==0 && bit==0x4000000 && state==1) || (local_count==1 && bit==0x40 && !state)); ++local_count;
}
void log_say(const char *s,int a,int b,int v) {
 assert(a==2 && b==3);
 assert(log_count<2 ? (!std::strcmp(s,"Sending SET TREATY") && v==(log_count ? 0x40 : 0x4000000)) : (!std::strcmp(s,"Sending TREATY ON") && v==2)); ++log_count;
}
void message_data(int op,int z,int a,int b,int bit,int state) {
 assert(!z && a==2 && b==3);
 assert(network_count<2 ? (op==0x2443 && bit==(network_count ? 0x40 : 0x4000000) && state==(network_count ? 0 : 1)) : (op==0x2441 && bit==2 && !state)); ++network_count;
}
int wants_to_attack(int a,int b,int z) { assert(a==3 && b==2 && !z); return attack; }
void enemies_treaty(int a,int b,int z) { assert(a==2 && b==3 && !z && local_count==2); ++local_count; }
struct Net { void await_diplo(int op) { assert(op==0x441 && network_count==3); ++await_count; } } NetDaemonNet;
@@BODY@@
void reset() {
 std::memset(PlayersData,0,sizeof(PlayersData)); std::memset(Players,0,sizeof(Players));
 for(int i=0;i<8;++i) { FactionDiplomacyPortraits[i]=&portraits[i]; Players[i].is_leader_female=i; Players[i].noun_gender=i+8; Players[i].is_noun_plural=i+16; }
 selected=3; available=executions=destructors=opens=fail_open=parsed=attack=picture=0;
 item_count=local_count=network_count=await_count=log_count=IsMultiplayerNet=0; notice=0;
 PlayersData[2].diplo_treaties[3]=0x10; PlayersData[1].diplo_treaties[3]=9;
}
void run() { call_off_vendetta(1,2); assert(destructors==1); }
int main() {
 reset(); PlayersData[2].diplo_treaties[3]=0; run(); assert(!opens && !executions);
 reset(); PlayersData[1].diplo_treaties[3]=8; run(); assert(!opens);
 reset(); fail_open=1; run(); assert(opens==1 && !executions);
 reset(); selected=-1; run(); assert(item_count==1 && items[0]==3 && executions==1 && !notice);
 reset(); fail_open=2; PlayersData[2].diplo_treaties[4]=0x10; PlayersData[1].diplo_treaties[4]=9;
 run(); assert(opens==2 && item_count==1 && parsed==7 && local_count==3);
 reset(); PlayersData[2].diplo_treaties[3]|=0x20; run(); assert(!std::strcmp(notice,"TOTHEDEATH") && !local_count);
 reset(); PlayersData[3].major_atrocities=1; run(); assert(!std::strcmp(notice,"TOTHEDEATH"));
 reset(); PlayersData[2].diplo_treaties[3]|=0x40; run(); assert(!std::strcmp(notice,"NOPEHIRED"));
 reset(); PlayersData[2].diplo_treaties[3]|=0x40; PlayersData[2].diplo_treaties[1]=0x80; run(); assert(local_count==3);
 reset(); attack=1; picture=1; run(); assert(local_count==2 && !std::strcmp(notice,"THEIRFAULT"));
 reset(); run(); assert(local_count==3 && !std::strcmp(notice,"CALLEDOFF"));
 reset(); PlayersData[3].major_atrocities=PlayersData[2].major_atrocities=1; IsMultiplayerNet=1;
 run(); assert(!local_count && network_count==3 && log_count==3 && await_count==1);
 reset(); IsMultiplayerNet=attack=1; run(); assert(network_count==2 && !await_count);
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_candidate_row', 'PlayersData[faction_id_with].diplo_treaties[candidate]', 'PlayersData[candidate].diplo_treaties[faction_id_with]'),
        ('wrong_actor_row', 'PlayersData[faction_id].diplo_treaties[candidate]', 'PlayersData[faction_id_with].diplo_treaties[candidate]'),
        ('wrong_atrocity', 'PlayersData[chosen].major_atrocities != 0', 'PlayersData[chosen].major_atrocities == 0'),
        ('wrong_ally_row', 'PlayersData[faction_id_with].diplo_treaties[faction_id]', 'PlayersData[faction_id].diplo_treaties[faction_id_with]'),
        ('wrong_local_tuple', 'enemies_treaty(faction_id_with, chosen, 0)', 'enemies_treaty(chosen, faction_id_with, 0)'),
        ('wrong_network_tuple', 'message_data(0x2441, 0, faction_id_with, chosen, 2, 0)', 'message_data(0x2441, 0, chosen, faction_id_with, 2, 0)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), name


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_diplomacy_menu(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    helper = definition(source, r'void read_diplomacy_menu_line\(\)')
    body = definition(source, r'int __cdecl diplomacy_menu\([^)]*\)')
    fixture = r'''
#include <cassert>
#include <cstring>
#define __cdecl
#define nullptr 0
struct Sprite {} portraits[8];
Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female, rule_flags; char name_leader[24]; } Players[8];
struct Data { unsigned int diplo_treaties[8]; } PlayersData[8];
int PopupDialogFactionID, PluralityDefault, GenderDefault;
int DiploScratchFAD0, DiploScratchFAD4, DiploScratchFA94, DiploScratchF7DC;
int IsMultiplayerNet;
char StringTemp[80], PopupStartCaption[80], line[]="line", script[]="script";
char *TextBufferGetPtr=line, *ScriptPopupFile=script;
int lines, parses, closes, destructors, executions, open_result, council, picture;
int ids[16], id_count, mutation;
char *text_get() { ++lines; TextBufferGetPtr=line; return line; }
int parse_string(const char *s,char *d) {
  assert(s==TextBufferGetPtr && d==StringTemp && !d[0]);
  std::strcpy(d,s); ++parses;
  if (mutation && lines==4) DiploScratchFAD4=1;
  return 0;
}
int parse_says(int n,const char *s,int a,int b) {
  assert(a==-1 && b==-1);
  if (!n) { assert(s==Players[1].name_leader); assert(!PluralityDefault && GenderDefault==1); }
  else assert(n==1 && !std::strcmp(s,"pact"));
  return 0;
}
char *get_pact_hood(int a,int b) { assert(a==1 && b==2); static char s[]="pact"; return s; }
void diplomacy_caption(int a,int b) { assert(a==1 && b==2 && PopupDialogFactionID==2); }
int X_text_open(char *s,const char *section) { assert(s==script && !std::strcmp(section,"DIPLOMENU")); return open_result; }
void text_close() { assert(lines==9 && parses==9); ++closes; }
int can_call_council(int a,int b) { assert(a==1 && b==0); return council; }
int pop_wait() { return 0; }
struct Dialogs {
 void sprite(Sprite *s,char *t,int n) { assert(s==FactionDiplomacyPortraits[2] && !t && !n); }
 void item(char *s,int id) { assert(s==StringTemp && !std::strcmp(s,"line")); ids[id_count++]=id; }
};
struct Popup {
 Dialogs menu;
 ~Popup() { ++destructors; }
 void start(char *s,const char *label,int n,char *t,int flags) {
  assert(s==PopupStartCaption && !std::strcmp(label,"DIPLO") && n==-1 && !t && flags==0x100000);
 }
 bool picture_enabled() { return picture!=0; }
 void set_caller_picture(Sprite *s) { assert(s==(picture ? 0 : FactionDiplomacyPortraits[2])); }
 Dialogs &dialogs() { return menu; }
 int exec(int n,int(*callback)()) { assert(!n && callback==pop_wait && closes==1); ++executions; return 23; }
};
@@BODIES@@
void reset() {
 lines=parses=closes=destructors=executions=id_count=mutation=0;
 open_result=council=picture=IsMultiplayerNet=0;
 DiploScratchFAD0=DiploScratchFAD4=DiploScratchFA94=DiploScratchF7DC=0;
 std::memset(PlayersData,0,sizeof(PlayersData)); std::memset(Players,0,sizeof(Players));
 Players[1].is_leader_female=1;
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 StringTemp[0]='x';
}
void expect(const int *values,int count) {
 assert(id_count==count); for(int i=0;i<count;++i) assert(ids[i]==values[i]);
 assert(lines==9 && parses==9 && closes==1 && destructors==1 && executions==1);
}
int main() {
 reset(); open_result=1; assert(diplomacy_menu(1,2)==0);
 assert(!lines && !closes && !executions && destructors==1);
 reset(); assert(diplomacy_menu(1,2)==23); const int basic[]={0,2}; expect(basic,2);
 reset(); picture=1; council=1; DiploScratchFAD0=DiploScratchFAD4=DiploScratchFA94=DiploScratchF7DC=1;
 PlayersData[2].diplo_treaties[1]=1; PlayersData[1].diplo_treaties[2]=7;
 assert(diplomacy_menu(1,2)==23); const int all[]={0,1,2,5,6,7,3,4}; expect(all,8);
 reset(); IsMultiplayerNet=1; council=1; DiploScratchFA94=DiploScratchF7DC=1;
 Players[2].rule_flags=0x80; PlayersData[1].diplo_treaties[2]=1;
 assert(diplomacy_menu(1,2)==23); const int net[]={0,2,4}; expect(net,3);
 reset(); mutation=1; assert(diplomacy_menu(1,2)==23); const int reread[]={0,2,2}; expect(reread,3);
}
'''
    variants = {'actual': body}
    replacements = {
        'wrong_forward_pair': ('PlayersData[faction_id_with].diplo_treaties[faction_id] & 1', 'PlayersData[faction_id].diplo_treaties[faction_id_with] & 1'),
        'wrong_reverse_pair': ('PlayersData[faction_id].diplo_treaties[faction_id_with] & 7', 'PlayersData[faction_id_with].diplo_treaties[faction_id] & 7'),
        'wrong_positive_gate': ('DiploScratchFA94 > 0', 'DiploScratchFA94 < 0'),
        'wrong_mode_line': ('DiploScratchFAD4 != 0', 'DiploScratchFAD4 == 0'),
        'lost_network_gate': ('IsMultiplayerNet == 0 && DiploScratchF7DC > 0', 'DiploScratchF7DC > 0'),
        'lost_alien_gate': ('(Players[faction_id_with].rule_flags & 0x80) == 0', 'true'),
        'lost_text_close': ('text_close();', '(void)0;'),
    }
    for name,(old,new) in replacements.items():
        assert body.count(old)==1
        variants[name]=body.replace(old,new)
    for name,candidate in variants.items():
        cpp,binary=tmp_path/(name+'.cpp'),tmp_path/name
        cpp.write_text(host_fixture(fixture).replace('@@BODIES@@',helper+'\n'+candidate))
        build=subprocess.run([compiler,'-std=c++98','-fsanitize=undefined','-fno-sanitize-recover=all',str(cpp),'-o',str(binary)],capture_output=True,text=True,timeout=45)
        assert build.returncode==0,build.stderr
        run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
        assert (run.returncode==0)==(name=='actual'),name+run.stderr

@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_energy_trade(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    body = definition(source, r'int __cdecl energy_trade\([^)]*\)')
    helpers = '\n'.join(definition(source, r'static int ' + name + r'\([^)]*\)(?=\s*\{)')
                        for name in ('diplo_add', 'diplo_sub', 'diplo_mul', 'diplo_clamp'))
    fixture = r"""
#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int uint32_t;
struct Sprite {} portraits[8]; Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female; char title_leader[24],name_leader[24],filename[24]; } Players[8];
struct Data { uint32_t diplo_treaties[8],ranking; int energy_reserves,tech_ranking,tech_commerce_bonus;
 char diplo_patience[8]; int loan_balance[8],loan_payment[8]; } PlayersData[8];
int PopupDialogFactionID,DiplomacyProposalMode,DiploScratchFAA8,DiploScratchFA18;
int DiploScratchF800,DiploScratchFA78,TurnCurrentNum,IsMultiplayerNet,GenderDefault,PluralityDefault;
unsigned char GameTechAchieved[89]; char StringTemp[128],key[128];
int responses[8],response_count,pops,helps,first_value,second_value,satan;
int money_messages,tech_messages,loan_messages,tech_calls,awaits,redraws,nums[3],mutate;
int purchased,loan_balance,loan_periods,transferred;
int pop_wait() { return 0; }
void tech_analysis(int a,int b) { assert(a==1 && b==2); }
int bit_count(unsigned n) { int r=0; while(n) { r+=n&1; n>>=1; } return r; }
int great_satan(int a,int z) { assert(a==1 && !z); return satan; }
int tech_val(int t,int owner,int simple) { assert(owner==2 && simple==1); return t==3?first_value:second_value; }
void say_tech(char *s,int t,int cat) { assert(s==StringTemp && !*s && cat==1); std::sprintf(s,"tech%d",t); }
int parse_says(int n,char *s,int a,int b) { assert(n>=0 && n<2 && s && a==-1 && b==-1); return 0; }
void parse_num(int n,int v) { assert(n>=0 && n<3); nums[n]=v; }
int random(int a,int b) { assert(!a && b==2); return 1; }
char *_itoa(int n,char *s,int base) { assert(base==10); std::sprintf(s,"%d",n); return s; }
void help_topic(int cat,int tech) { assert(cat==14 && tech==3); ++helps; }
int X_pops(const char *s,int flags,Sprite *p,int(*cb)()) {
 assert(flags==0x100000 && p==FactionDiplomacyPortraits[2] && cb==pop_wait);
 std::strcpy(key,s); assert(pops<8); int result=responses[pops++];
 if(mutate && !std::strcmp(s,"SELLSECOND")) { DiploScratchFA18=6; IsMultiplayerNet=1; }
 return result;
}
void log_say(const char *s,int a,int b,int c) { assert(s && a>=0 && b>=0); }
void message_data(int op,int z,int a,int b,int c,int d) {
 assert(!z);
 if(op==0x2447) { assert(a==2 && c==1 && b==-d); ++money_messages; transferred=d; }
 else if(op==0x244b) { assert(a==2 && c==1 && !d); ++tech_messages; purchased=b; }
 else { assert(op==0x244e && a==1 && b==2); ++loan_messages; loan_balance=c; loan_periods=d; }
}
struct Net { void await_diplo(int op) { assert(op==0x447); ++awaits; } } NetDaemonNet;
struct Info { void set_redraw_phase(int n) { assert(!n); } } ConsoleInfoWin;
struct Console { void update_data(int n) { redraws+=n?1:10; } } ConsoleGlobal;
void tech_achieved(int a,int t,int b,int z) { assert(a==2 && b==1 && !z); ++tech_calls; purchased=t; }
@@BODY@@
void reset(int mode=4) {
 std::memset(Players,0,sizeof(Players)); std::memset(PlayersData,0,sizeof(PlayersData));
 std::memset(GameTechAchieved,0,sizeof(GameTechAchieved));
 for(int i=0;i<8;++i) { FactionDiplomacyPortraits[i]=&portraits[i]; responses[i]=1; }
 DiplomacyProposalMode=mode; DiploScratchFAA8=3; DiploScratchFA18=5; DiploScratchF800=3;
 DiploScratchFA78=2; TurnCurrentNum=8; IsMultiplayerNet=0;
 response_count=pops=helps=satan=money_messages=tech_messages=loan_messages=tech_calls=awaits=redraws=mutate=0;
 first_value=300; second_value=200; purchased=-1; transferred=loan_balance=loan_periods=0;
 PlayersData[1].energy_reserves=100; PlayersData[2].energy_reserves=500;
 std::strcpy(Players[2].filename,"OTHER"); key[0]=0;
}
int run() { return energy_trade(1,2); }
int main() {
 reset(3); assert(!run() && !tech_calls && !std::strcmp(key,"REJENERGY1"));
 reset(1); PlayersData[2].diplo_treaties[1]=1; PlayersData[2].ranking=3; PlayersData[1].ranking=1; PlayersData[2].tech_ranking=10;
 assert(!run() && PlayersData[1].energy_reserves==600 && PlayersData[2].energy_reserves==0 && PlayersData[2].diplo_patience[1]==2 && !std::strcmp(key,"ENERGYFREEBIE1"));
 reset(1); PlayersData[1].diplo_treaties[2]=1; PlayersData[2].ranking=3; PlayersData[2].tech_ranking=10;
 assert(!run() && PlayersData[1].energy_reserves==100 && !std::strcmp(key,"REJENERGY1"));
 reset(); assert(run() && purchased==3 && tech_calls==1 && nums[0]==250 && PlayersData[1].energy_reserves==350 && PlayersData[2].energy_reserves==250 && DiploScratchFAA8==5 && DiploScratchFA18==-1 && redraws==11);
 reset(); DiploScratchFA18=-1; assert(run() && !std::strcmp(key,"ENERGYTECH1"));
 reset(); first_value=120; second_value=80; satan=1; assert(run() && nums[0]==75);
 reset(); GameTechAchieved[3]=15; first_value=800;
 assert(run() && nums[0]==250); // valid-index bit_count-minus-one divisor (3)
 reset(); responses[0]=3; assert(run() && helps==1 && pops==2);
 reset(); responses[0]=0; assert(!run() && !tech_calls && !redraws);
 reset(); std::strcpy(Players[2].filename,"BELIEVE"); assert(!run() && !std::strcmp(key,"REJSELLNONE"));
 reset(); std::strcpy(Players[2].filename,"HIVE"); responses[0]=2; assert(!run() && !tech_calls && !std::strcmp(key,"REJSELLSECOND"));
 reset(); first_value=80; responses[0]=2; assert(!run() && !std::strcmp(key,"REJSELLSECOND"));
 reset(); first_value=80; std::strcpy(Players[2].filename,"UNIV"); responses[0]=2; mutate=1;
 assert(run() && purchased==6 && !tech_calls && tech_messages==1 && money_messages==1 && transferred==75 && awaits==1 && DiploScratchFAA8==3);
 reset(); IsMultiplayerNet=1; assert(run() && tech_messages==1 && money_messages==1 && awaits==1 && !tech_calls && redraws==10);
 reset(6); PlayersData[2].energy_reserves=600; PlayersData[2].diplo_treaties[1]=2;
 assert(run() && nums[0]==200 && nums[1]==4 && nums[2]==80 && PlayersData[1].loan_balance[2]==320 && PlayersData[1].loan_payment[2]==4 && PlayersData[1].energy_reserves==300 && PlayersData[2].energy_reserves==400 && !std::strcmp(key,"ENERGYLOAN2"));
 reset(6); PlayersData[2].energy_reserves=600; PlayersData[2].diplo_treaties[1]=2; IsMultiplayerNet=1;
 assert(run() && loan_balance==320 && loan_periods==4 && transferred==200 && loan_messages==1 && money_messages==1 && awaits==1);
 reset(6); PlayersData[1].energy_reserves=1000; PlayersData[2].diplo_treaties[1]=2;
 assert(!run() && !std::strcmp(key,"REJENERGY1"));
 reset(6); PlayersData[1].energy_reserves=1000; PlayersData[2].diplo_treaties[1]=2; std::strcpy(Players[2].filename,"MORGAN");
 assert(run() && PlayersData[1].energy_reserves==1166);
 reset(6); PlayersData[2].diplo_treaties[1]=2; PlayersData[1].loan_balance[2]=1; assert(!run());
 reset(6); PlayersData[2].energy_reserves=600; PlayersData[2].diplo_treaties[1]=2; PlayersData[2].tech_commerce_bonus=1;
 assert(run() && nums[0]==300 && nums[1]==7 && nums[2]==80 && PlayersData[1].loan_balance[2]==560 && PlayersData[1].loan_payment[2]==7);
 reset(6); PlayersData[2].energy_reserves=60000000; PlayersData[2].diplo_treaties[1]=2;
 assert(run() && nums[0]==20000000 && nums[1]==-1094966 && nums[2]==10 && PlayersData[1].loan_balance[2]==-10949660 && PlayersData[1].loan_payment[2]==-1094966);
}
"""
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_freebie_row', 'PlayersData[other].diplo_treaties[faction] & 1u', 'PlayersData[faction].diplo_treaties[other] & 1u'),
        ('wrong_sale_scale', 'diplo_mul(scale, 3) / 4', 'diplo_mul(scale, 3) / 32'),
        ('wrong_loan_field', 'loan_payment[other] = periods', 'loan_payment[other] = payment'),
        ('wrong_bitmap_divisor',
         'game_tech_achieved_[DiploScratchF800]) - 1, 1, 99)',
         'game_tech_achieved_[DiploScratchF800]), 1, 99)'),
        ('unsigned_repayment', 'diplo_mul(diplo_add(surcharge, 100), principal) / 100',
         'static_cast<int>(static_cast<uint32_t>(diplo_mul(diplo_add(surcharge, 100), principal)) / 100u)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(host_fixture(fixture).replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)
