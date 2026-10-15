#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __cdecl
enum { VSTATE_MONOLITH_UPGRADED=0x100 };
struct Veh { int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id; uint16_t unk_1; uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order; uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4]; uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended; int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id; int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; uint8_t obsolete_factions; int8_t combat_factions,icon_offset,padding; uint16_t flags; int16_t preq_tech; };
struct RulesWeapon { const char *name,*name_short; int8_t offense_rating,icon; uint8_t mode,cost; int16_t preq_tech,padding; };
struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk_1; int8_t territory; uint32_t bit,bit2,bit_visible[7]; };
struct Char256 { char str[256]; };
struct Effect { int is_playing(){return 0;} };
struct FX { int plays[100],stops[100]; Effect e; FX(){memset(plays,0,sizeof plays);memset(stops,0,sizeof stops);} void play(int i){++plays[i];} void stop(int i){++stops[i];} Effect &effect(int){return e;} };
struct Console { int faction_id_; };
Veh Vehs[8]; VehPrototype VehPrototypes[8]; RulesWeapon Weapon[26]; Map tile; Map *MapTiles=&tile; uint32_t MapLongitude=4; Console ConsoleGlobal={0};
int callback_console_faction=-1;
void on_ui_callback() { if(callback_console_faction>=0) ConsoleGlobal.faction_id_=callback_console_faction; }
struct NetMsg { int calls,last_timeout; const char *last_label,*last_picture; NetMsg():calls(0),last_timeout(0),last_label(0),last_picture(0){} int pop(const char *label,int timeout,int,const char *picture){++calls;last_timeout=timeout;last_label=label;last_picture=picture;on_ui_callback();return 0;} };
FX g_FX; NetMsg NetMsgPrimary;
struct GameDataStateWindow { struct Fields { uint32_t game_more_preferences_,game_state_; int turn_current_num_; union LockMasks { struct Bytes { uint8_t human_mask_,alive_mask_,unassigned_[2]; } bytes_; uint32_t enable_mask_; } lock_masks_; } fields_; } GameDataStateStorage;
int IsMultiplayerNet=0; uint32_t &GameMorePreferences=GameDataStateStorage.fields_.game_more_preferences_; uint32_t &GameState=GameDataStateStorage.fields_.game_state_; int &TurnCurrentNum=GameDataStateStorage.fields_.turn_current_num_; uint32_t &LockEnableMask=GameDataStateStorage.fields_.lock_masks_.enable_mask_; int prefs=0,choice=0,skips=0,draws=0,del_count=0,site_count=0,site_ids[16],bit_calls=0,sync_calls=0,morale_calls=0,proto_after_morale=-1,morale_result=2,morale_after_first=-1;
char StringTemp[1032]; Char256 ParseStrBuffer[10]; int GenderDefault=0,PluralityDefault=0; char *ScriptPopupFile=(char*)"title";
void del_site(int,int,int,int,int){++del_count;} void add_site(int id,int,int,int,int){site_ids[site_count++]=id;} void draw_tile(int,int,int){++draws;} void veh_skip(int){++skips;} int morale_veh(int,int,int){++morale_calls;if(proto_after_morale>=0)Vehs[0].proto_id=static_cast<int16_t>(proto_after_morale);if(morale_calls==2&&morale_after_first>=0)Vehs[0].morale=static_cast<uint8_t>(morale_after_first);return morale_result;} void prefs_save(int){++prefs;} int popp(const char*,const char*,int,const char*,int(__cdecl*)()){on_ui_callback();return choice;}
Map *map_loc(int,int){return MapTiles;} void bit_set(int,int,int bit,int set){++bit_calls;if(set)tile.bit|=bit;else tile.bit&=~static_cast<uint32_t>(bit);} void synch_bit(int,int,int){++sync_calls;} void say_morale(int,char*,int){} void say_morale(int,int){} int parse_says(int,const char*,int,int){return 0;} char *CharLowerA(char *s){return s;} char *_itoa(int n,char *out,int){strcpy(out,n?"1":"0");return out;}
#include "actual.inc"
static void reset(){
 memset(Vehs,0,sizeof Vehs); memset(VehPrototypes,0,sizeof VehPrototypes);
 memset(Weapon,0,sizeof Weapon); memset(&tile,0,sizeof tile);
 ConsoleGlobal.faction_id_=0;
 IsMultiplayerNet=GameMorePreferences=GameState=LockEnableMask=prefs=choice=skips=draws=del_count=site_count=bit_calls=sync_calls=morale_calls=0; TurnCurrentNum=1;
 memset(site_ids,0,sizeof site_ids);
 callback_console_faction=-1; proto_after_morale=-1; morale_result=2; morale_after_first=-1;
 Vehs[0].x=2; Vehs[0].y=1; Vehs[0].proto_id=0; Vehs[0].faction_id=0;
 VehPrototypes[0].weapon_id=1; VehPrototypes[0].reactor_id=2;
 VehPrototypes[0].chassis_id=1; Weapon[1].offense_rating=0; Vehs[0].morale=2;
 g_FX=FX(); NetMsgPrimary=NetMsg();
}
int main(){
 reset();
 Vehs[0].proto_id=0x10;
 monolith(0); assert(Vehs[0].state==0&&Vehs[0].flags==0&&NetMsgPrimary.calls==0);

 reset(); monolith(0);
 assert((Vehs[0].state&0x100)!=0 && Vehs[0].flags==0);
 assert(NetMsgPrimary.calls==1&&strcmp(NetMsgPrimary.last_label,"SEEMONOLITH")==0);

 reset(); VehPrototypes[0].weapon_id=0x10; monolith(0);
 assert((Vehs[0].state&0x100)!=0 && NetMsgPrimary.calls==1);

 reset(); Vehs[0].dmg_incurred=7; monolith(0);
 assert(Vehs[0].dmg_incurred==0&&skips==1&&draws==1&&NetMsgPrimary.calls==1);

 reset(); Weapon[1].offense_rating=1; Vehs[0].state=0x100; LockEnableMask=0;
 tile.bit_visible[0]=0x2000; tile.bit_visible[2]=0x2000;
 monolith(0);
 assert(del_count==7&&site_count==2&&site_ids[0]==1&&site_ids[1]==3);

 reset(); Weapon[1].offense_rating=1; LockEnableMask=2; choice=1;
 Vehs[0].faction_id=1; Vehs[0].state=0x100; Vehs[0].dmg_incurred=1;
 callback_console_faction=1; monolith(0);
 assert(ConsoleGlobal.faction_id_==1);
 assert(NetMsgPrimary.calls==1&&strcmp(NetMsgPrimary.last_label,"MONOLITHHEAL")==0);

 reset(); Weapon[1].offense_rating=1; Weapon[2].offense_rating=0;
 VehPrototypes[1].weapon_id=2; Weapon[2].offense_rating=1; LockEnableMask=0;
 tile.bit_visible[0]=tile.bit_visible[1]=tile.bit_visible[2]=tile.bit_visible[3]=
 tile.bit_visible[4]=tile.bit_visible[5]=tile.bit_visible[6]=0x2000;
 proto_after_morale=1; Vehs[0].morale=2; morale_result=2; morale_after_first=9; monolith(0);
 assert(Vehs[0].proto_id==1&&site_count==7&&Vehs[0].morale==6);
}
