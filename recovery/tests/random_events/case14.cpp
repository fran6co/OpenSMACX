#include <cassert>
#include <cstring>
#include <stddef.h>
#include <stdint.h>

enum { BIT2_VOLCANO=0x2, PFLAG_MAP_REVEALED=0x200, ROCKINESS_ROCKY=2 };
static const uint32_t BIT2_UNK_80000000=0x80000000u;
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3;
    uint8_t unk_1; int8_t territory; uint32_t bit, bit2, bit_visible[7];
};
typedef char map_bit2_is_at_0x0c[(offsetof(Map,bit2)==0x0C)?1:-1];
struct Base {
    int16_t x,y; uint8_t faction_id_current, faction_id_former;
    int8_t population_size; uint8_t assimilation_turns_left, nerve_staple_turns_left;
    uint8_t unk_1, visibility, faction_pop_size_intel[8]; char name_string[25];
    bool objective, alive; int fixture_tag;
};
typedef char base_name_is_at_0x13[(offsetof(Base,name_string)==0x13)?1:-1];
struct Veh {
    int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id,unk_1;
    uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order;
    uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4];
    uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended;
    int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id;
    int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack;
    bool alive;
};
typedef char veh_proto_and_damage_offsets[(offsetof(Veh,proto_id)==0x0A && offsetof(Veh,dmg_incurred)==0x10)?1:-1];
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; uint8_t obsolete_factions; int8_t combat_factions,icon_offset,padding; uint16_t flags; int16_t preq_tech; };
typedef char proto_reactor_plan_offsets[(offsetof(VehPrototype,reactor_id)==0x27 && offsetof(VehPrototype,plan)==0x2A)?1:-1];
struct PlayerData { uint32_t flags,ranking; unsigned char prefix_to_current_bases[0x1054]; int current_num_bases; };
typedef char current_bases_is_at_0x105c[(offsetof(PlayerData,current_num_bases)==0x105C)?1:-1];

#include "actual_case14.inc"

struct Fixture {
    bool pbem, net; int turn, local, flat, longitude, longitude_bounds, latitude_bounds;
    uint32_t event_flags, mount_x, mount_y, unknown_bitfield1; int dust_duration;
    PlayerData players[8]; Base bases[16]; Veh vehicles[16]; VehPrototype prototypes[16];
    Map tiles[128*64]; int base_count, veh_count;
    int radius_x[81], radius_y[81]; int bit_sets, syncs, rocky_calls;
    int kills[16], kill_count, base_kills, parses, focuses, draws, popps;
    char trace[2048]; int trace_count;
    bool focus_result;
    Fixture() : pbem(false),net(false),turn(75),local(2),flat(1),longitude(64),
        longitude_bounds(128),latitude_bounds(64),event_flags(0),mount_x(40),mount_y(40),unknown_bitfield1(0),
        dust_duration(0),base_count(0),veh_count(0),bit_sets(0),syncs(0),rocky_calls(0),
        kill_count(0),base_kills(0),parses(0),focuses(0),draws(0),popps(0),trace_count(0),focus_result(false) {
        for (int i=0;i<8;++i) { players[i].flags=0; players[i].ranking=0; players[i].current_num_bases=0; }
        std::memset(bases,0,sizeof(bases)); std::memset(vehicles,0,sizeof(vehicles));
        std::memset(prototypes,0,sizeof(prototypes)); std::memset(tiles,0,sizeof(tiles));
        for (int i=0;i<16;++i) { bases[i].alive=true; bases[i].fixture_tag=-1; vehicles[i].alive=true; vehicles[i].home_base_id=-1; }
        for (int i=0;i<81;++i) { radius_x[i]=1000; radius_y[i]=1000; }
        for (int i=0;i<16;++i) kills[i]=0;
    }
    bool is_multiplayer_pbem() const { return pbem; }
    int turn_current_num() const { return turn; }
    PlayerData &player_data(int f) { return players[f]; }
    uint32_t random_event_flags() const { return event_flags; }
    uint32_t mount_planet_x() const { return mount_x; }
    uint32_t mount_planet_y() const { return mount_y; }
    int map_latitude_bounds() const { return latitude_bounds; }
    int map_longitude_bounds() const { return longitude_bounds; }
    int map_longitude() const { return longitude; }
    int map_is_flat() const { return flat; }
    Map &map_tile(int x,int y) { return tiles[y*longitude+(x>>1)]; }
    const Base &base(int i) const { return bases[i]; }
    Base &base(int i) { return bases[i]; }
    int local_faction() const { return local; }
    int radius_offset_x(int i) const { return radius_x[i]; }
    int radius_offset_y(int i) const { return radius_y[i]; }
    void note(char c) { trace[trace_count++]=c; }
    int vehicle_count() const { return veh_count; }
    Veh &vehicle(int i) { return vehicles[i]; }
    const Veh &vehicle(int i) const { return vehicles[i]; }
    const VehPrototype &prototype(int i) const { return prototypes[i]; }
    void bit_set(int x,int y,int mask,bool set) { ++bit_sets; note('b'); Map &t=map_tile(x,y); if(set)t.bit|=(uint32_t)mask; else t.bit&=~(uint32_t)mask; }
    void synch_bit(int x,int y,int faction) { ++syncs; note('s'); if(faction) { Map &t=map_tile(x,y); t.bit_visible[faction-1]=t.bit; } }
    void rocky_set(int x,int y,int value) { ++rocky_calls; note('r'); Map &t=map_tile(x,y); t.val3=(t.val3&0x3F)|(value<<6); t.bit2|=0x400000; unknown_bitfield1|=1; }
    void kill_vehicle(int id) {
        note('v');
        kills[kill_count++]=id;
        for(int i=id;i<veh_count-1;++i) vehicles[i]=vehicles[i+1];
        --veh_count;
    }
    int base_at(int x,int y) const { for(int i=0;i<base_count;++i) if(bases[i].alive&&bases[i].x==x&&bases[i].y==y)return i; return -1; }
    void parse_base_name_suffix(const char *name) { assert(name!=0); note('n'); ++parses; }
    bool is_objective_base(int id) const { return bases[id].objective; }
    void kill_base(int id) {
        for(int i=id;i<base_count-1;++i) bases[i]=bases[i+1];
        note('B'); --base_count; ++base_kills;
    }
    bool focus(int x,int y,int faction) { assert(x==(int)mount_x&&y==(int)mount_y&&faction==local); note('f'); ++focuses; return focus_result; }
    void draw_map(int mode) { assert(mode==1); note('d'); ++draws; }
    void set_dust_cloud_duration(int value) { note('D'); dust_duration=value; }
    void popp_big_fat_volcano() { note('p'); ++popps; }
};

static void set_offset(Fixture &f,int index,int dx,int dy) {
    f.radius_x[index]=dx; f.radius_y[index]=dy;
}
static void prepare(Fixture &f) {
    f.players[1].current_num_bases=8; f.players[1].ranking=6;
    f.players[1].flags=PFLAG_MAP_REVEALED;
    f.players[2].flags=PFLAG_MAP_REVEALED;
    f.base_count=5; f.veh_count=3;
    f.bases[0].x=40; f.bases[0].y=40; f.bases[0].faction_id_current=1;
    f.bases[0].population_size=8; f.bases[0].objective=true; f.bases[0].fixture_tag=0;
    f.bases[1].x=42; f.bases[1].y=42; f.bases[1].faction_id_current=2;
    f.bases[1].population_size=8; f.bases[1].fixture_tag=1;
    f.bases[2].x=41; f.bases[2].y=39; f.bases[2].faction_id_current=1;
    f.bases[2].population_size=8; f.bases[2].fixture_tag=2;
    f.bases[3].x=44; f.bases[3].y=44; f.bases[3].faction_id_current=1;
    f.bases[3].population_size=8; f.bases[3].fixture_tag=3;
    f.bases[4].x=46; f.bases[4].y=46; f.bases[4].faction_id_current=1;
    f.bases[4].population_size=8; f.bases[4].fixture_tag=4;
    f.prototypes[0].plan=0xC; f.prototypes[0].reactor_id=4;
    f.prototypes[1].plan=1; f.prototypes[1].reactor_id=4;
    f.vehicles[0].x=40; f.vehicles[0].y=40; f.vehicles[0].proto_id=1; f.vehicles[0].dmg_incurred=10;
    f.vehicles[1].x=41; f.vehicles[1].y=39; f.vehicles[1].proto_id=0; f.vehicles[1].dmg_incurred=0;
    f.vehicles[2].x=41; f.vehicles[2].y=39; f.vehicles[2].proto_id=1; f.vehicles[2].dmg_incurred=10;
    f.map_tile(40,40).bit2=BIT2_VOLCANO;
    f.map_tile(40,40).visibility=(1u<<1)|(1u<<2);
    set_offset(f,0,0,0); set_offset(f,1,1,-1);
    set_offset(f,10,2,2); set_offset(f,30,4,4); set_offset(f,60,6,6);
}

int main() {
    // Failure gates have no terrain, vehicle, base, display, or popup effects.
    Fixture gate; prepare(gate); gate.pbem=true;
    random_events_case14(gate,1,0);
    assert(gate.bit_sets==0 && gate.veh_count==3 && gate.base_kills==0 && gate.popps==0);
    gate.pbem=false; gate.turn=74;
    random_events_case14(gate,1,0);
    assert(gate.bit_sets==0);
    gate.turn=75; gate.event_flags=0x10;
    random_events_case14(gate,1,0);
    assert(gate.bit_sets==0);

    Fixture unseen; prepare(unseen); unseen.players[1].flags=0;
    unseen.map_tile(40,40).visibility &= ~(1u<<1);
    random_events_case14(unseen,1,0);
    assert(unseen.bit_sets==0 && unseen.popps==0);
    Fixture low_rank; prepare(low_rank); low_rank.players[1].ranking=5;
    random_events_case14(low_rank,1,0);
    assert(low_rank.bit_sets==0 && low_rank.popps==0);

    // The exact body mutates each in-bounds radius tile in bit/sync/rocky order,
    // kills units on the live volcano, damages other units, kills a non-objective
    // inner base, scales protected/farther populations by ring band, and then
    // drives focus/redraw and the volcano popup.
    Fixture f; prepare(f);
    random_events_case14(f,1,0);
    assert(f.bit_sets==5 && f.syncs==5 && f.rocky_calls==5);
    assert(f.unknown_bitfield1==1);
    assert(f.trace[0]=='b' && f.trace[1]=='s' && f.trace[2]=='r' && f.trace[3]=='v');
    assert(f.kills[0]==0 && f.veh_count==2 && f.vehicles[0].x==41);
    assert(f.vehicles[0].dmg_incurred==0); // one-point reactor exception halves/truncates
    assert(f.vehicles[1].dmg_incurred==25); // (40 - 10) >> 1, then add old damage
    assert(f.base_kills==1 && f.base_count==4);
    assert(f.bases[0].population_size==2);
    assert(f.bases[1].population_size==2 && f.parses==1);
    assert(f.bases[2].fixture_tag==3 && f.bases[2].population_size==4);
    assert(f.bases[3].fixture_tag==4 && f.bases[3].population_size==6);
    assert(f.focuses==1 && f.draws==1 && f.popps==1 && f.dust_duration==10);
    assert(f.trace[f.trace_count-4]=='f' && f.trace[f.trace_count-3]=='d' &&
           f.trace[f.trace_count-2]=='D' && f.trace[f.trace_count-1]=='p');

    // Focus success suppresses draw_map but not state mutation or popup.
    Fixture focused; prepare(focused); focused.focus_result=true;
    random_events_case14(focused,1,0);
    assert(focused.focuses==1 && focused.draws==0 && focused.popps==1);

    // Dormant/marked center and out-of-range selected base reject before effects.
    Fixture dormant; prepare(dormant); dormant.map_tile(40,40).bit2|=BIT2_UNK_80000000;
    random_events_case14(dormant,1,0);
    assert(dormant.bit_sets==0 && dormant.popps==0);
    Fixture far; prepare(far); far.bases[0].x=100;
    random_events_case14(far,1,0);
    assert(far.bit_sets==0 && far.popps==0);

    // Round-map wrapping takes the single native add/subtract branch. A base
    // at longitude 120 is eight map-distance units from event x=0.
    Fixture wrap; prepare(wrap); wrap.flat=0; wrap.mount_x=0;
    wrap.bases[0].x=120; wrap.vehicles[0].x=0;
    for (int i=1;i<81;++i) { wrap.radius_x[i]=1000; wrap.radius_y[i]=1000; }
    set_offset(wrap,0,0,0);
    wrap.map_tile(0,40).bit2=BIT2_VOLCANO;
    wrap.map_tile(0,40).visibility=(1u<<1)|(1u<<2);
    random_events_case14(wrap,1,0);
    assert(wrap.bit_sets==1 && wrap.kills[0]==0 && wrap.popps==1);
    return 0;
}

