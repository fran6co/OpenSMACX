#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define __cdecl
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"failed line %d\n",__LINE__); abort(); } } while (0)
struct Base { uint8_t faction_id_current; char name_string[25]; uint32_t state;
  int minerals_accumulated, minerals_accumulated_2, production_id_last;
  int queue_production_id[10], resource_sqr_radius; };
Base Bases[8], *BaseCurrent; int BaseIDCurrentSelected;
struct PlayerData { int energy_reserves, hurry_cost_total; } PlayersData[8];
struct Player { int rule_hurry; char veh_name[32]; } Players[8];
struct RuleSet { unsigned retool_exemption; } Rules;
struct Projects { int voice_of_planet; } SecretProject;
struct FacilityRecord { int cost, name; } Facility[128];
struct Prototype { char veh_name[32]; } VehPrototypes[128];
struct StringTableType { char *get(int) { return (char*)"facility"; } } StringTable;
int IsMultiplayerNet, ConsoleControlTurnActive; uint32_t GameState;
int CurrentFactionEnergyReserves, ParseNumTable[8], ParseSays[8], ParseNums[8], ParseNumCalls;
char StringTemp[1032], *ScriptPopupFile=(char*)"SCRIPT", PopupStartCaption[256];
char *test_dialog_text=(char*)"dialog";
int exec_result, ask_result, user_payment, text_open_result;
int lock_results[8], lock_result_count, lock_calls, unlock_calls;
int callback_mode, callback_called, fail_after_exec_state, *focused_id_pointer;
int base_compute_calls, base_compute_focus_to, radius_increment_mode;
int veh_cost_switch_to, factor_switch_call, factor_switch_to, factor_calls, factor_factions[8];
int text_get_calls, text_get_mutate_call, text_get_energy_delta, parsed_text_first[4];
int veh_cost_id, veh_cost_base, synch_calls, log_calls, fx_calls, popup_start_calls;
int popup_destroy_calls, ask_calls, text_open_calls, dialog_items[3];
struct Console { int faction_id_; } ConsoleGlobal;
struct Dialogs { int item(char*,int i) { ++dialog_items[i]; return 0; } };
int pop_expire(); struct GraphicWin;
struct Popup { Dialogs d; Popup(){} ~Popup(){++popup_destroy_calls;}
  Dialogs &dialogs(){return d;} void start(char*,const char*,int,char*,int,GraphicWin*);
  void start(char*,const char*,int,char*,int,int); int exec(int,int(__cdecl*)()); };
struct NetDaemon { int lock_base(int,int,int,int); void unlock_base(int); } NetDaemonNet;
struct FX { void play(int); } g_FX; struct GraphicWin { void redraw(); };
struct MainInterface { void redraw(); } MainInterfaceGlobal;
struct InfoWin { int phase; void set_redraw_phase(int x){phase=x;} void on_redraw(); } ConsoleInfoWin;
struct BaseWin { union { int focus; int focusedBaseID_; }; int field_40B38_,redraws; BaseWin():focus(0),field_40B38_(0),redraws(0){}
  int focused_base_id()const{return focus;} void redraw(){++redraws;} void compute_selected_base(); void hurry(); };
void set_base(int id){BaseIDCurrentSelected=id;BaseCurrent=&Bases[id];}
void base_compute(int){++base_compute_calls;if(radius_increment_mode)++BaseCurrent->resource_sqr_radius;
  if(base_compute_focus_to>=0&&focused_id_pointer)*focused_id_pointer=base_compute_focus_to;}
int veh_cost(int id,int base,int){veh_cost_id=id;veh_cost_base=base;if(veh_cost_switch_to>=0)BaseCurrent=&Bases[veh_cost_switch_to];return 100;}
int cost_factor(int f,int,int){factor_factions[factor_calls++]=f;if(factor_switch_call==factor_calls&&factor_switch_to>=0)BaseCurrent=&Bases[factor_switch_to];return f==5?2:f==6?3:1;}
int parse_says(int i,const char*s,int,int){++ParseSays[i];parsed_text_first[i]=(unsigned char)s[0];return 0;}
int parse_num(int i,int v){ParseNums[i]=v;++ParseNumCalls;return 0;}
void parse_string(char*i,char*o){strcpy(o,i);}
char *text_get(){++text_get_calls;if(text_get_calls==text_get_mutate_call)PlayersData[2].energy_reserves+=text_get_energy_delta;return test_dialog_text;}
int X_text_open(char*,const char*){++text_open_calls;return text_open_result;}
int X_pop(const char*,int(__cdecl*c)()){if(c)c();return 0;}
int X_pop_ask_number(const char*,int,int(__cdecl*c)()){++ask_calls;if(c)c();ParseNumTable[0]=user_payment;return ask_result;}
int synch_energy(int){++synch_calls;return 0;} void log_say(const char*,int,int,int){++log_calls;}
void FX::play(int){++fx_calls;} void GraphicWin::redraw(){} void MainInterface::redraw(){}
void InfoWin::on_redraw(){} int NetDaemon::lock_base(int,int,int,int){int i=lock_calls++;return i<lock_result_count?lock_results[i]:0;}
void NetDaemon::unlock_base(int){++unlock_calls;} int Popup::exec(int,int(__cdecl*c)()){if(c)c();return exec_result;}
void Popup::start(char*,const char*,int,char*,int,GraphicWin*){++popup_start_calls;}
void Popup::start(char*,const char*,int,char*,int,int){++popup_start_calls;}
int pop_expire(){++callback_called;if(callback_mode){if(focused_id_pointer)*focused_id_pointer=1;BaseIDCurrentSelected=1;BaseCurrent=&Bases[1];if(fail_after_exec_state)GameState|=0x800;}return 0;}

/* RECOVERED_SOURCE_INSERTION_POINT */

void reset(){memset(Bases,0,sizeof(Bases));memset(PlayersData,0,sizeof(PlayersData));memset(Players,0,sizeof(Players));
  memset(ParseNumTable,0,sizeof(ParseNumTable));memset(ParseSays,0,sizeof(ParseSays));memset(ParseNums,0,sizeof(ParseNums));
  IsMultiplayerNet=ConsoleControlTurnActive=0;GameState=0x80;CurrentFactionEnergyReserves=1000;SecretProject.voice_of_planet=-1;
  Rules.retool_exemption=0;lock_result_count=lock_calls=unlock_calls=0;memset(lock_results,0,sizeof(lock_results));
  exec_result=1;ask_result=user_payment=text_open_result=0;callback_mode=callback_called=fail_after_exec_state=0;focused_id_pointer=0;
  base_compute_calls=0;base_compute_focus_to=-1;radius_increment_mode=0;veh_cost_switch_to=factor_switch_call=factor_switch_to=-1;
  factor_calls=text_get_calls=text_get_mutate_call=text_get_energy_delta=0;memset(factor_factions,0,sizeof(factor_factions));
  memset(parsed_text_first,0,sizeof(parsed_text_first));veh_cost_id=veh_cost_base=synch_calls=log_calls=fx_calls=0;
  popup_start_calls=popup_destroy_calls=ask_calls=text_open_calls=ParseNumCalls=0;memset(dialog_items,0,sizeof(dialog_items));
  strcpy(Bases[0].name_string,"A");strcpy(Bases[1].name_string,"B");Bases[0].faction_id_current=2;Bases[1].faction_id_current=5;
  Bases[0].resource_sqr_radius=3;Bases[1].resource_sqr_radius=7;Bases[0].minerals_accumulated=20;Bases[1].minerals_accumulated=40;
  Bases[0].queue_production_id[0]=100;Bases[1].queue_production_id[0]=101;Players[2].rule_hurry=Players[5].rule_hurry=100;
  PlayersData[2].energy_reserves=500;PlayersData[5].energy_reserves=700;set_base(0);}
int main(){
  reset();REQUIRE(hurry_mul(50000,50000)==-1794967296);REQUIRE(hurry_add(0x7fffffff,1)==(-2147483647-1));
  REQUIRE(hurry_sub((-2147483647-1),1)==0x7fffffff);
  reset();Base p=Bases[0];BaseCurrent=&p;p.faction_id_current=2;p.minerals_accumulated=10;Facility[69].cost=Facility[70].cost=100;
  p.queue_production_id[0]=-69;REQUIRE(hurry_price(0,-69,2).adjusted_cost==180);p.queue_production_id[0]=-70;
  REQUIRE(hurry_price(0,-70,2).adjusted_cost==360);p.queue_production_id[0]=100;p.minerals_accumulated=80;
  REQUIRE(hurry_price(0,100,2).adjusted_cost==60);Rules.retool_exemption=-60000;p.minerals_accumulated=-59900;
  REQUIRE(hurry_price(0,100,2).adjusted_cost==-34628364);
  reset();BaseWin full;full.focus=0;full.hurry();REQUIRE(PlayersData[2].energy_reserves==20&&CurrentFactionEnergyReserves==520);
  REQUIRE(Bases[0].minerals_accumulated==100&&Bases[0].minerals_accumulated_2==100&&full.field_40B38_==100);
  reset();BaseWin partial;partial.focus=0;exec_result=2;user_payment=50;partial.hurry();
  REQUIRE(PlayersData[2].energy_reserves==450&&CurrentFactionEnergyReserves==950&&Bases[0].minerals_accumulated==28);
  REQUIRE(Bases[0].state&0x40000000u);
  reset();BaseWin clamped;clamped.focus=0;exec_result=2;user_payment=600;clamped.hurry();
  REQUIRE(PlayersData[2].energy_reserves==0&&Bases[0].minerals_accumulated==103);
  reset();BaseWin text; text.focus=0;PlayersData[2].energy_reserves=460;text_get_mutate_call=2;text_get_energy_delta=30;text.hurry();
  REQUIRE(text_get_calls==3&&dialog_items[1]==1&&PlayersData[2].energy_reserves==10);
  reset();BaseWin drone;drone.focus=0;Bases[0].state|=2u;drone.hurry();REQUIRE(popup_start_calls==2&&text_open_calls==0);
  reset();BaseWin changed;changed.focus=0;focused_id_pointer=&changed.focus;callback_mode=1;changed.hurry();
  REQUIRE(Bases[1].minerals_accumulated==100&&Bases[0].minerals_accumulated==20);
  reset();BaseWin reloads;reloads.focus=0;veh_cost_switch_to=1;factor_switch_call=1;factor_switch_to=2;
  Bases[1].faction_id_current=5;Bases[1].queue_production_id[0]=-70;Bases[1].minerals_accumulated=10;
  Bases[2].faction_id_current=6;Bases[2].queue_production_id[0]=101;Bases[2].minerals_accumulated=20;
  strcpy(Bases[2].name_string,"C");strcpy(VehPrototypes[100].veh_name,"saved");
  PlayersData[2].energy_reserves=3000;CurrentFactionEnergyReserves=3000;reloads.hurry();
  REQUIRE(factor_calls==1&&factor_factions[0]==5&&parsed_text_first[0]=='C'&&parsed_text_first[1]=='s');
  REQUIRE(PlayersData[2].energy_reserves==1020&&PlayersData[5].energy_reserves==700);
  REQUIRE(Bases[0].minerals_accumulated==200&&Bases[2].minerals_accumulated==20);
  reset();BaseWin no_funds;no_funds.focus=0;PlayersData[2].energy_reserves=-1;no_funds.hurry();
  REQUIRE(callback_called==1&&text_open_calls==0);
  reset();IsMultiplayerNet=1;ConsoleControlTurnActive=1;BaseWin net;net.focus=0;net.hurry();
  REQUIRE(PlayersData[2].hurry_cost_total==480&&PlayersData[2].energy_reserves==500);
  REQUIRE(Bases[0].minerals_accumulated==100&&synch_calls==0);return 0;}
