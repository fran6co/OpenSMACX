#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl

enum { PLAN_ALIEN_ARTIFACT = 12, TRIAD_AIR = 2,
       ABL_AIR_SUPERIORITY = 0x20, ABL_CARRIER = 0x80,
       BIT_BASE_IN_TILE = 1, BIT_AIRBASE = 0x40000,
       MaxPlayerNum = 8 };
enum { RECT_SINGULARITY = 4 };

struct Veh { int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint16_t unk; uint8_t faction_id, lurking, dmg_incurred; int8_t order;
    uint8_t waypoint_count, patrol; int16_t wx[4], wy[4]; uint8_t morale,
    terraforming, auto_type, visibility, moves; int8_t facing; uint8_t unk6,
    move_to_ai, probe, sabotage; int16_t home_base_id, next, prev; };
struct VehPrototype { char name[32]; uint32_t abilities; uint8_t chassis_id,
    weapon_id, armor_id, reactor_id, capacity, cost, plan; int8_t unk1;
    uint8_t obsolete; int8_t combat, icon, padding; uint16_t flags;
    int16_t preq; };
struct RulesChassis { char pad[0x49]; uint8_t triad; char pad2[3];
    uint8_t cost, missile; };
struct RulesWeapon { char pad[8]; int8_t offense_rating; };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk;
    int8_t territory; uint32_t bit, bit2, visible[7]; };
struct Player { int is_leader_female; char filename[24], search_key[24],
    name_leader[24], title_leader[24], adj_leader[128], adj_insult_leader[128],
    adj_faction[128], adj_insult_faction[128]; uint8_t padding1[128];
    char noun_faction[24]; int noun_gender; int is_noun_plural;
    char adj_name_faction[128]; };
struct PlayerData { uint32_t flags; char pad0[0x858-4]; uint8_t proto_id_active[512];
    uint8_t proto_id_queue[512]; int16_t proto_id_lost[512]; char pad1[0x2084-0x1058];
    int unk_101; char pad2[0x208c-0x2088]; int cursor; };
struct Base { char pad[0x30]; uint32_t state; char pad2[0x134-0x34];
    char name_string[25]; };
struct Console { int faction_id_; int focus(int, int, int); };
struct NetMsg { int pop(const char *, int, int, const char *); };

Veh Vehs[2049]; VehPrototype VehPrototypes[512]; RulesChassis Chassis[19];
RulesWeapon Weapon[26]; Map MapTiles[32 * 64]; Player Players[8];
PlayerData PlayersData[8]; Base Bases[64]; int VehCurrentCount;
int MapLongitudeBounds = 32, MapLatitudeBounds = 64;
uint32_t MapLongitude = 16; int MapIsFlat; uint32_t GamePreferences;
int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
uint32_t FactionRankings[8]; int GenderDefault, PluralityDefault;
Console ConsoleGlobal; NetMsg NetMsgPrimary;
int lift_count, drop_count, skip_count, kill_count, scoot_count;
int last_drop_id, last_drop_x, last_drop_y, last_focus_faction;
int parse_ids[4], parse_count, interlude_id, morale_value = 3;
const char *last_popup;
int base_find_result = -1;

Map *map_loc(int x, int y) { return &MapTiles[(x >> 1) + y * MapLongitude]; }
int stack_check(int, int, int, int, int) { return 0; }
int has_abil(int proto, int ability) {
    return (VehPrototypes[proto].abilities & static_cast<uint32_t>(ability)) != 0;
}
int veh_lift(int id) { ++lift_count; return id; }
int veh_drop(int id, int x, int y) {
    ++drop_count; last_drop_id = id; last_drop_x = x; last_drop_y = y;
    Vehs[id].x = static_cast<int16_t>(x); Vehs[id].y = static_cast<int16_t>(y); return id;
}
void veh_skip(int) { ++skip_count; }
void veh_kill(int) { ++kill_count; }
int base_find(int, int, int) { return base_find_result; }
int base_find(int, int) { return base_find_result; }
int morale_veh(int, int, int) { return morale_value; }
int range(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
}
int parse_says(int id, const char *, int, int) {
    parse_ids[parse_count++] = id; return 0;
}
void interlude(int id, char *, int, int) { interlude_id = id; }
int Console::focus(int, int, int faction) { last_focus_faction = faction; return 0; }
int NetMsg::pop(const char *label, int, int, const char *) {
    last_popup = label; return 0;
}
void veh_scoot(int, int, int, int, int) { ++scoot_count; }

#include "actual.inc"

static void clear_state() {
    memset(Vehs, 0, sizeof Vehs); memset(VehPrototypes, 0, sizeof VehPrototypes);
    memset(Chassis, 0, sizeof Chassis); memset(Weapon, 0, sizeof Weapon);
    memset(MapTiles, 0, sizeof MapTiles); memset(Players, 0, sizeof Players);
    memset(PlayersData, 0, sizeof PlayersData); memset(Bases, 0, sizeof Bases);
    lift_count = drop_count = skip_count = kill_count = scoot_count = 0;
    last_drop_id = last_drop_x = last_drop_y = last_focus_faction = -1;
    parse_count = interlude_id = 0; last_popup = 0; base_find_result = -1;
    GamePreferences = 0; MapIsFlat = 1; VehCurrentCount = 0;
}

static void test_battle_kill_normal_and_wrap() {
    clear_state();
    Vehs[4].faction_id = 3; Vehs[4].proto_id = 7; Vehs[4].flags = 0x20;
    PlayersData[3].proto_id_lost[7] = 32767;
    PlayersData[2].unk_101 = 9;
    int attackers = 1, defenders = 2, odds = 4;
    assert(battle_kill(4, &attackers, &defenders, &odds, -1, 2) == 1);
    assert(static_cast<uint16_t>(PlayersData[3].proto_id_lost[7]) == 32768u);
    assert(PlayersData[2].unk_101 == 10);
    assert(attackers == 2 && defenders == 2 && kill_count == 1);
}

static void test_battle_kill_capture_and_event_order() {
    clear_state();
    Vehs[2].faction_id = 1; Vehs[2].proto_id = 12; Vehs[2].flags = 0;
    Vehs[3].faction_id = 4; Vehs[3].proto_id = 13;
    Vehs[3].x = 6; Vehs[3].y = 8; Vehs[3].home_base_id = -1;
    VehPrototypes[12].plan = PLAN_ALIEN_ARTIFACT;
    VehPrototypes[12].chassis_id = 2; Chassis[2].missile = 0;
    MapTiles[(6 >> 1) + 8 * MapLongitude].climate = 0x60;
    PlayersData[1].proto_id_active[12] = 1; PlayersData[4].proto_id_active[12] = 0;
    Vehs[2].state = 0xFCFFFFFFU;
    int defenders = 0;
    assert(battle_kill(2, 0, &defenders, 0, 3, -1) == 0);
    assert(Vehs[2].state == (0xFCFFFFFFU & 0xFCFFBDFFU));
    assert(Vehs[2].faction_id == 4 && Vehs[2].dmg_incurred == 0 && Vehs[2].order == 0);
    assert(PlayersData[1].proto_id_active[12] == 0 && PlayersData[4].proto_id_active[12] == 1);
    assert(defenders == 1 && lift_count == 1 && drop_count == 1 && skip_count == 1);
    assert(last_drop_id == 2 && last_drop_x == 6 && last_drop_y == 8 && kill_count == 0);

    clear_state();
    Vehs[2].faction_id = 1; Vehs[2].proto_id = 7; Vehs[2].state = 0x800000;
    Vehs[3].faction_id = 4; Vehs[3].proto_id = 13; Vehs[3].home_base_id = 5;
    PlayersData[4].proto_id_active[13] = 0x01; Players[4].is_leader_female = 1;
    Players[4].noun_gender = 2; Players[4].is_noun_plural = 1;
    strcpy(Players[4].name_leader, "Leader"); strcpy(Players[4].noun_faction, "Faction");
    strcpy(Bases[5].name_string, "Base");
    base_find_result = 5;
    int attackers = 0;
    assert(battle_kill(2, &attackers, 0, 0, 3, -1) == 1);
    assert((Bases[5].state & 0x40000) != 0);
    assert((PlayersData[4].flags & 0x1000) != 0);
    assert(parse_count == 2 && parse_ids[0] == 6 && parse_ids[1] == 7);
    assert(GenderDefault == 2 && PluralityDefault == 1 && interlude_id == 6);
}

static void test_interceptor_selection_notification_and_move() {
    clear_state();
    ConsoleGlobal.faction_id_ = 2;
    VehCurrentCount = 2;
    for (int i = 0; i < VehCurrentCount; ++i) {
        Vehs[i].faction_id = 2; Vehs[i].proto_id = i;
        Vehs[i].x = static_cast<int16_t>(i * 2); Vehs[i].y = 0;
        VehPrototypes[i].chassis_id = 0; VehPrototypes[i].weapon_id = 1;
        VehPrototypes[i].reactor_id = 2; VehPrototypes[i].abilities = ABL_AIR_SUPERIORITY;
        Chassis[0].triad = TRIAD_AIR; Weapon[1].offense_rating = 3;
        MapTiles[(Vehs[i].x >> 1)].bit = BIT_AIRBASE;
    }
    assert(interceptor(2, 4, 4, 0) == 1);
    assert(last_drop_id == 1 && last_drop_x == 4 && last_drop_y == 0);
    assert(last_focus_faction == 2 && last_popup != 0 &&
           strcmp(last_popup, "AIRSCRAMBLE") == 0);
    assert(Vehs[1].order == 0 && (Vehs[1].state & 0x400000) == 0);
    assert(scoot_count > 0);

    clear_state();
    VehCurrentCount = 1; Vehs[0].faction_id = 2; Vehs[0].proto_id = 0;
    Vehs[0].x = 10; Vehs[0].y = 10;
    VehPrototypes[0].chassis_id = 0; VehPrototypes[0].abilities = 0;
    Chassis[0].triad = TRIAD_AIR;
    assert(interceptor(2, 4, 0, 0) == 0);
    assert(drop_count == 0 && last_popup == 0);
}

int main() {
    test_battle_kill_normal_and_wrap();
    test_battle_kill_capture_and_event_order();
    test_interceptor_selection_notification_and_move();
}
