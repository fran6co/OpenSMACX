"""Actual diplomacy bodies with service doubles, not GUI/game acceptance."""
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


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
