// Whole candidate body with compact mock owners. This is a callback-freshness
// control, not evidence for product ABI/layout or exhaustive game behavior.
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
enum { ORDER_NONE=0, ORDER_CONVOY=3, ORDER_MOVE=25, ORDER_ROAD_TO=27,
       ORDER_MAGTUBE_TO=28, ORDER_MOVE_TO=24 };
struct Veh {
    int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint16_t unk1; uint8_t faction_id,year_end_lurking,dmg_incurred;
    int8_t order; uint8_t waypoint_count,patrol_current_point;
    int16_t waypoint_x[4],waypoint_y[4]; uint8_t morale,terraforming_turns,
    order_auto_type,visibility,moves_expended; int8_t facing;
    uint8_t unk6,move_to_ai_type,probe_action,probe_sabotage_id;
    int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack;
};
struct VehPrototype { char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan;
    int8_t unk1,obsolete_factions,combat_factions,icon_offset; uint8_t padding;
    uint16_t flags; int16_t preq_tech; };
struct Base { enum AutoForwardSlot { AutoForwardLand, AutoForwardSea,
    AutoForwardAir, AutoForwardReserved, AutoForwardSlotCount };
    int16_t x,y; uint8_t faction_id_current,faction_id_former;
    int8_t population_size; uint8_t assimilation_turns_left,
    nerve_staple_turns_left,unk1,visibility,faction_pop_size_intel[8];
    char name_string[25]; int16_t unkx,unky; uint32_t state,event,governor;
    int nutrients_accumulated,minerals_accumulated,production_id_last,eco_damage;
    uint32_t queue_size; int queue_production_id[10]; int specialist_unk1;
    uint32_t spec[2]; uint8_t facilities_built[12]; int mineral_surplus_final,
    minerals_accumulated_2; int data0a8[11]; int nutrient_intake_1,
    mineral_intake_1,energy_intake_1,unused_intake_1,nutrient_intake_2,
    mineral_intake_2,energy_intake_2,unused_intake_2,nutrient_surplus,
    mineral_surplus,energy_surplus,unused_surplus,nutrient_inefficiency,
    mineral_inefficiency,energy_inefficiency,unused_inefficiency,
    nutrient_consumption,mineral_consumption,energy_consumption,
    unused_consumption,economy_total,psych_total,labs_total,unk10;
    int16_t auto_forward_slots[AutoForwardSlotCount]; int talent_total,drone_total,
    super_drone_total,random_event_turns,nerve_staple_count,unk11,location;
};
// The product's nominal LPSTR carries a StringTable index here; keep the
// index as an int in this 64-bit host double to avoid a fake pointer ABI.
struct RulesFacility { int name; int cost; };
struct PlayerData { uint32_t diplo_treaties[8]; };
struct Player { uint32_t rule_flags; };
struct ConsoleGlobalType { int faction_id_; };
int exec_flag_after=-1, selected=1, picture_bound=0;
int close_trace[32], close_trace_count;
struct LockState { uint32_t leading_words_[4]; };
struct NetDaemonMock { LockState lock_state_[2]; void await_exec(int){} };
NetDaemonMock NetDaemonNet;
struct Sprite { int init(char *,int,int){return 0;} void close(){
    ++sprite_closes; close_trace[close_trace_count++]=2;}
    static int sprite_closes; };
int Sprite::sprite_closes=0;
struct Buffer { static int get_pcx_dimensions(const char *,int *w,int *h){
    *w=20;*h=20;return 0;} };
char captured_facility_row[128];
struct Dialogs { int item(char *p,int i){ rows[i]=p; ++row_count;
    if(i==1) std::strncpy(captured_facility_row,p,sizeof(captured_facility_row)-1);
    return 0; }
    char *rows[8]; int row_count; Dialogs():row_count(0){} };
struct Popup { Sprite picture; Dialogs box; Popup():modal_result(0){}
    int modal_result;
    Sprite &owned_picture(){return picture;}
    void set_caller_picture(Sprite *p){assert(p==&picture);++picture_bound;}
    Dialogs &dialogs(){return box;}
    int start(char *,const char *s,int v,char *,int,void *){
        assert(std::strcmp(s,"SUPPLYUNIT")==0&&v==-1);return 0;}
    int exec(int,int(__cdecl *)()){
        if(exec_flag_after>=0) NetDaemonNet.lock_state_[1].leading_words_[3]=
            static_cast<uint32_t>(exec_flag_after);
        return selected;
    }
    void close(){ close_trace[close_trace_count++]=1; picture.close(); }
    ~Popup(){close();}
};
struct NetMsgMock { int last_amount, pops; NetMsgMock():last_amount(-1),pops(0){}
    void pop(const char *,int,int,int){++pops;} };
Veh Vehs[2]; VehPrototype VehPrototypes[128]; Base Bases[2];
RulesFacility Facility[134]; PlayerData PlayersData[8]; Player Players[8];
ConsoleGlobalType ConsoleGlobal; NetMsgMock NetMsgPrimary;
uint32_t LockEnableMask=0xFF; BOOL MapIsFlat=1; uint32_t MapLongitude=80,
    MapLongitudeBounds=160; int PopupModalState=0,IsMultiplayerNet=0;
char PopupStartCaption[32]; char *ScriptPopupFile=PopupStartCaption;
char StringTemp[1032], localized[64], parsed_template[64];
LPSTR TextBufferGetPtr=localized;
int facility_parse_calls=0,text_reads=0,cost_calls=0,mutate_cost_proto=0;
int cost_trace[16],cost_trace_count=0;
int text_open_result=0, mutate_parse=0, mutate_parse_num=0, sync_calls=0, kills=0, messages=0,
    awaits=0, parse_num_calls=0, last_parse_num=-999;
int __cdecl cost_factor(int,int,int){cost_trace[cost_trace_count++]=6;
    ++cost_calls;if(cost_calls==3&&mutate_cost_proto) Vehs[0].proto_id=1;
    return 1;}
int __cdecl veh_cost(int,int,BOOL *){cost_trace[cost_trace_count++]=5;return 3;}
BOOL __cdecl convoy(int,int){return 1;}
void __cdecl synch_base(int){++sync_calls; Bases[0].minerals_accumulated=1;}
void __cdecl message_veh(int,int,int,int){++messages;}
void __cdecl kill(int){++kills;}
void __cdecl parse_num(int,int value){++parse_num_calls;last_parse_num=value;
    if(parse_num_calls==1&&mutate_parse_num){Bases[0].minerals_accumulated=10;
        Facility[70].cost=11;}}
void __cdecl parse_say(int slot,int input,int gender,int plural){
    assert(slot==0&&input==Facility[70].name&&gender==-1&&plural==-1);
    ++facility_parse_calls;std::strcpy(parsed_template,"parser-state-live");
    TextBufferGetPtr=parsed_template;
}
void __cdecl parse_says(int slot,char *,int,int){
    if(slot==1&&mutate_parse){Vehs[0].proto_id=1;Bases[0].queue_production_id[0]=-70;}
    if(slot==0){std::strcpy(parsed_template,"prototype-state-live");
        TextBufferGetPtr=parsed_template;}
}
LPSTR __cdecl text_get(){++text_reads;std::strcpy(localized,"row");
    TextBufferGetPtr=localized;return localized;}
int __cdecl parse_string(LPSTR in,LPSTR out){std::strcpy(out,in);return 0;}
int __cdecl X_text_open(char *,const char *){return text_open_result;}

// ACTUAL_SOURCE_BODY

static void reset(){
    std::memset(Vehs,0,sizeof(Vehs));std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Bases,0,sizeof(Bases));std::memset(Facility,0,sizeof(Facility));
    std::memset(PlayersData,0,sizeof(PlayersData));std::memset(Players,0,sizeof(Players));
    std::memset(&NetDaemonNet,0,sizeof(NetDaemonNet)); Sprite::sprite_closes=0;
    LockEnableMask=0xFF;MapIsFlat=1;MapLongitude=80;MapLongitudeBounds=160;
    PopupModalState=IsMultiplayerNet=sync_calls=kills=messages=awaits=0;
    parse_num_calls=0;last_parse_num=-999;text_open_result=0;mutate_parse=mutate_parse_num=0;
    close_trace_count=0;std::memset(close_trace,0,sizeof(close_trace));
    exec_flag_after=-1;selected=1;NetMsgPrimary.last_amount=-1;NetMsgPrimary.pops=0;
    picture_bound=facility_parse_calls=text_reads=0;TextBufferGetPtr=localized;
    cost_calls=mutate_cost_proto=cost_trace_count=0;
    captured_facility_row[0]='\0';
    Vehs[0].faction_id=0;Vehs[0].proto_id=0;Vehs[0].order=ORDER_NONE;
    Bases[0].faction_id_current=0;Bases[0].queue_production_id[0]=0;
    VehPrototypes[0].cost=1;VehPrototypes[1].cost=7;
    Facility[70].cost=2;
    std::strcpy(VehPrototypes[0].veh_name,"old-unit");
    std::strcpy(VehPrototypes[1].veh_name,"new-unit");
    std::strcpy(Bases[0].name_string,"base");Facility[70].name=37;
}
int main(){
    // Native modal zero/decline result returns zero after automatic teardown,
    // without applying a supply/convoy action or storing a probe action.
    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=1;selected=0;
    assert(supply_options(0,0)==0);
    assert(Vehs[0].order==ORDER_NONE&&Vehs[0].probe_action==0);
    assert(sync_calls==0&&kills==0&&PopupModalState==0);
    assert(close_trace_count==2&&close_trace[0]==1&&close_trace[1]==2);
    assert(picture_bound==1);
    // Early text-open failure relies only on the automatic Popup lifetime.
    reset();text_open_result=1;
    assert(supply_options(0,0)==0);
    assert(close_trace_count==2&&close_trace[0]==1&&close_trace[1]==2);

    // parse_says callbacks mutate the vehicle prototype and base queue; later
    // native reads must see both changes. parse_num mutates current minerals,
    // then synch_base changes them again before the amount-needed calculation.
    reset();mutate_parse=1;mutate_parse_num=1;
    NetDaemonNet.lock_state_[1].leading_words_[3]=1;
    selected=1;exec_flag_after=2;Vehs[0].proto_id=0;Bases[0].queue_production_id[0]=0;
    assert(supply_options(0,0)==1);
    assert(Vehs[0].proto_id==1&&Bases[0].queue_production_id[0]==-70);
    assert(Bases[0].minerals_accumulated==1&&Bases[0].mineral_surplus_final==17);
    assert(sync_calls==1&&kills==1&&last_parse_num==10);
    assert(NetMsgPrimary.pops==1);
    assert(std::strcmp(captured_facility_row,"parser-state-live")==0);
    assert(facility_parse_calls==1&&text_reads==6&&picture_bound==1);
    assert(close_trace_count>=5&&close_trace[0]==1&&close_trace[1]==2&&
           close_trace[2]==2); // Popup::close, then explicit picture close.

    // The native path rereads lock flags after modal exec. Bit 1 set selects
    // action dispatch even when bit 0 was cleared by the callback; the stored
    // probe action must therefore still be applied.
    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=1;
    PopupModalState=0;selected=1;exec_flag_after=2;
    assert(supply_options(0,0)==1);
    assert(Bases[0].minerals_accumulated==1&&sync_calls==1&&kills==1);
    assert(close_trace_count>=5&&close_trace[0]==1&&close_trace[1]==2&&
           close_trace[2]==2);

    // Each native veh_cost call precedes cost_factor. The final cost factor
    // callback runs before the live prototype cost is read for the action.
    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=1;
    selected=1;exec_flag_after=2;mutate_cost_proto=1;
    assert(supply_options(0,0)==1);
    assert(cost_trace_count==5&&cost_trace[0]==5&&cost_trace[1]==6&&
           cost_trace[2]==5&&cost_trace[3]==6&&cost_trace[4]==6);
    assert(Vehs[0].proto_id==1&&Bases[0].mineral_surplus_final==7);

    // With both action bits clear, the native path does not take the simple
    // return; it dispatches the default (non-supply) action and changes order.
    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=0;
    PopupModalState=0;selected=0;
    assert(supply_options(0,0)==0);
    assert(Vehs[0].order==ORDER_CONVOY&&Vehs[0].probe_sabotage_id==254);
    assert(Bases[0].minerals_accumulated==0&&sync_calls==0&&kills==0);
    assert(close_trace_count>=5&&close_trace[0]==1&&close_trace[1]==2&&
           close_trace[2]==2);

    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=2;
    PopupModalState=0;selected=0;
    assert(supply_options(0,0)==0);
    assert(Vehs[0].order==ORDER_CONVOY&&Vehs[0].probe_sabotage_id==254);
    assert(close_trace_count>=5&&close_trace[0]==1&&close_trace[1]==2&&
           close_trace[2]==2);

    // Bit 0 without bit 1 is the sole simple-return case after modal exec.
    reset();NetDaemonNet.lock_state_[1].leading_words_[3]=1;
    PopupModalState=0;selected=1;exec_flag_after=1;
    assert(supply_options(0,0)==1);
    assert(Bases[0].minerals_accumulated==0&&sync_calls==0&&kills==0);
    assert(close_trace_count==2&&close_trace[0]==1&&close_trace[1]==2);
    return 0;
}
