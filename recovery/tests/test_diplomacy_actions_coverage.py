"""Actual diplomacy bodies with service doubles, not GUI/game acceptance."""
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


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
struct Projects {int empath_guild;} SecretProject;
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
int x_pops_short(const char *key,int flags,Sprite *s,int(*cb)()){assert(flags==0x100000&&s==FactionDiplomacyPortraits[2]&&cb==pop_wait);std::strcpy(notice,key);return 0;}
int x_pops_minimal(const char *key,Sprite *s,int(*cb)()){assert(s==FactionDiplomacyPortraits[2]&&cb==pop_wait);std::strcpy(notice,key);return 0;}
@@BODY@@
void reset(){
 std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));std::memset(Bases,0,sizeof(Bases));
 for(int i=0;i<8;++i){FactionDiplomacyPortraits[i]=&portraits[i];answers[i]=-1;}
 BaseCurrentCount=3;Bases[0].faction_id_current=2;Bases[0].visibility=2;
 Bases[1].faction_id_current=1;Bases[2].faction_id_current=1;
 PlayersData[1].current_num_bases=2;PlayersData[2].current_num_bases=1;
 LockEnableMask=0xfe00;GameRules=0;IsMultiplayerNet=0;SecretProject.empath_guild=-1;
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
        cpp.write_text(fixture.replace('@@BODY@@', candidate))
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
int x_pops_short(const char *key,int flags,Sprite *p,int(*cb)()) {
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
 PlayersData[2].diplo_friction[1]=99;threaten(1,2);assert(DiploScratchFA98==14&&PlayersData[1].energy_reserves==110);
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
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
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
int x_pops_short(const char *key,int flags,Sprite *p,int(*cb)()) {
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
 reset();assert(propose_treaty(1,2)==1&&settlements==1&&money_amount==200);
 assert(PlayersData[1].energy_reserves==800&&PlayersData[2].energy_reserves==200);
 assert(PlayersData[2].diplo_friction[1]==8&&PlayersData[1].diplo_friction[2]==17&&DiploFriction==-2);
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
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
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
int x_pops_short(const char *s,int flags,Sprite *p,int(*cb)()) {
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
 reset(); assert(run() && price==425 && tech_calls==1 && purchased==3 && PlayersData[1].energy_reserves==9575 && PlayersData[2].energy_reserves==425 && redraws==11 && !std::strcmp(key,"BUYTECH1"));
 reset(); GameTechAchieved[3]=15; assert(run() && price==150);
 reset(); PlayersData[1].diplo_treaties[2]=1; PlayersData[1].ranking=1; PlayersData[2].ranking=2; PlayersData[2].tech_ranking=9; assert(run() && price==125);
 reset(); unit_value=999; PlayersData[1].diplo_treaties[2]=1; PlayersData[1].ranking=1; PlayersData[2].ranking=2; PlayersData[2].tech_ranking=9; assert(run() && price==250);
 reset(); PlayersData[1].energy_reserves=0; assert(!run() && !pops);
 reset(); PlayersData[1].energy_reserves=0; assert(run(4,5) && !tech_calls && !std::strcmp(key,"BUYTECHHIGH1"));
 reset(); PlayersData[1].energy_reserves=0; assert(run(5) && scans==1 && DiploScratchF800==97 && !tech_calls && !std::strcmp(key,"BUYPROTOHIGH0"));
 reset(); assert(run(5) && scans==1 && purchased==97 && BestProtoForTrade==-1 && !randoms && price==425);
 reset(); DiploScratchF800=96; unit_value=25; assert(run(13) && price==50 && !randoms && !std::strcmp(key,"BUYCOMMLINK0"));
 reset(); popup_answer=0; assert(run() && !tech_calls && !redraws);
 reset(); IsMultiplayerNet=1; assert(run() && money_messages==1 && tech_messages==1 && awaits==1 && !tech_calls && redraws==10);
 reset(); IsMultiplayerNet=1; mutate=1; assert(run() && money_messages==1 && !tech_messages && tech_calls==1 && purchased==6 && !awaits);
 reset(); mutate=1; assert(run() && !money_messages && tech_messages==1 && purchased==7 && awaits==1);
}
'''
    variants = {'actual': body}
    for name, old, new in (
        ('wrong_discount', 'value /= 4;', 'value = diplo_add(value, 3) / 4;'),
        ('unsafe_pseudo_lookup', 'DiploScratchF800 < 89', 'DiploScratchF800 < 700'),
        ('wrong_tech_receiver', 'tech_achieved(faction_id, purchased_tech, other_faction_id, 0)',
         'tech_achieved(other_faction_id, purchased_tech, faction_id, 0)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)


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
int x_pops_short(const char *s,int flags,Sprite *p,int(*cb)()) {
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
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
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
int x_pops_short(const char *s,int flags,Sprite *p,int(*cb)()) {
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
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
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
int x_pops_short(const char *s,int f,Sprite *p,int(*cb)()) {
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
        cpp.write_text(fixture.replace('@@BODY@@', candidate))
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
        cpp.write_text(fixture.replace('@@BODIES@@',helper+'\n'+candidate))
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
int x_pops_short(const char *s,int flags,Sprite *p,int(*cb)()) {
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
 DiplomacyProposalMode=mode; DiploScratchFAA8=3; DiploScratchFA18=5; DiploScratchF800=-1;
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
        ('unsafe_bitmap', 'DiploScratchF800 >= 0 && DiploScratchF800 < 89', 'DiploScratchF800 < 89'),
        ('unsigned_repayment', 'diplo_mul(diplo_add(surcharge, 100), principal) / 100',
         'static_cast<int>(static_cast<uint32_t>(diplo_mul(diplo_add(surcharge, 100), principal)) / 100u)'),
    ):
        assert old in body
        variants[name] = body.replace(old, new)
    for name, candidate in variants.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(fixture.replace('@@BODY@@', helpers + '\n' + candidate))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=15)
        assert (result.returncode == 0) == (name == 'actual'), (name, result.stderr)
