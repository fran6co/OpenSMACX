// Fixture-only actual-body control. Not product headers/native proof.
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "game_data_state_host.h"
GameDataStateWindow GameDataStateStorage;

#ifndef __cdecl
#define __cdecl
#endif

enum { MaxPlayerNum = 8, BIT_BASE_IN_TILE = 1, BIT_AIRBASE = 0x40000,
       RFLAG_ALIEN = 0x80, FAC_BIOLOGY_LAB = 9 };
struct Veh { int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint16_t unk_1; uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order; uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4], waypoint_y[4]; uint8_t morale, terraforming_turns;
    uint8_t order_auto_type, visibility, moves_expended; int8_t facing;
    uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack; };
struct VehPrototype { char name[32]; uint32_t abilities; uint8_t chassis_id,
    weapon_id, armor_id, reactor_id, carry, cost, plan; int8_t unk;
    uint8_t obsolete; int8_t combat, icon, padding; uint16_t flags;
    int16_t preq; };
struct RulesChassis { void *names[6]; uint32_t genders[6]; int plurals[6];
    uint8_t speed, triad, range, cargo, cost, missile, offsets[64]; int16_t preq; };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk;
    int8_t territory; uint32_t bit, bit2, bit_visible[7]; };
struct Player { int is_leader_female; char filename[24], search_key[24], name_leader[24],
    title_leader[24], adj_leader[128], adj_insult_leader[128], adj_faction[128],
    adj_insult_faction[128]; uint8_t padding[128]; char noun_faction[24];
    int noun_gender, plural; uint8_t rest[0x59c - 0x1a0]; int rule_flags; };
struct PlayerData { uint32_t flags; uint32_t ranking; int diff_level;
    int base_name, sea_name, last_base; uint32_t diplo_treaties[8];
    int diplo_agenda[8]; uint32_t diplo_friction[8]; int diplo_spoke[8];
    int padding[0x20cc / 4 - 3 - 3 - 8 - 8 - 8 - 8]; int current_num_bases; };
struct Base { int16_t x, y; uint8_t faction_id_current, former;
    int8_t population; uint8_t assimilation, staple, unk, visibility, intel[8];
    char name[25]; int16_t unk_x, unk_y; uint32_t state, event, governor;
    int nutrients, minerals, production_id_last; uint8_t rest[0x134 - 0x54]; };
struct RulesFacility { void *name, *effect; uint32_t pad; int cost; uint32_t maint;
    int preq_tech, free_tech, ai[5]; };
int event_count, events[128];
struct MockMapWin { void main_caption() { events[event_count++] = 20; } };
struct MockConsole { int faction_id_; };

Veh Vehs[2049]; VehPrototype VehPrototypes[64]; RulesChassis Chassis[16];
Map MapTiles[256]; Player Players[MaxPlayerNum]; PlayerData PlayersData[MaxPlayerNum];
Base Bases[128]; RulesFacility Facility[64]; MockMapWin mapwin;
MockMapWin *MapWinTable[8] = { &mapwin };
MockConsole ConsoleGlobal;
void *FactionDiplomacyPortraits[MaxPlayerNum];
uint32_t MapLongitude = 16;
int VehTerraformStingLatch, MapSeaLevel, GenderDefault,
    PluralityDefault, BaseFindDist;
char *ScriptPopupFile = (char *)"SCRIPT";
char StringTemp[1032];
int killed[16], killed_count, setup_count,
    popup_count, treaty_count, agenda_count, random_event_count,
    interlude_count, territory_count, non_input_count, climate_count,
    redraw_count, ascending_result, spoke_12_seen_at_ordered_pair;
int random_values[8], random_index;

char *CharUpperA(char *s) {
    for (char *p = s; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 32;
    return s;
}
int speed(int, int) { return 2; }
int stack_check(int, int, int, int, int) { return 0; }
int range(int value, int, int) { return value; }
void kill(int id) { killed[killed_count++] = id; }
void rankings(int) { events[event_count++] = 1; }
int game_year(int turn) { return 2000 + turn; }
void draw_map(int) { ++redraw_count; }
void world_climate() { ++climate_count; }
int popp(const char *, const char *, int, const char *, int (*)()) {
    ++popup_count; events[event_count++] = 30; return 0;
}
int x_pops_short(const char *, int, void *, int (*)()) { ++popup_count; return 0; }
void alien_fauna() { events[event_count++] = 2; }
void do_fungal_towers() { events[event_count++] = 3; }
int rand() { return random_values[random_index++]; }
void setup_player(int faction, int, int) { ++setup_count; events[event_count++] = 40 + faction; }
void pact_ends(int, int) { events[event_count++] = 50; }
void treaty_on(int, int, int) { events[event_count++] = 51; }
void set_treaty(int faction, int other, int, int) {
    ++treaty_count;
    if (agenda_count == 1 && faction == 3 && other == 1) {
        spoke_12_seen_at_ordered_pair = PlayersData[1].diplo_spoke[2];
    }
    events[event_count++] = 52;
}
void set_agenda(int, int, int, int) { ++agenda_count; events[event_count++] = 53; }
void random_events(int) { ++random_event_count; events[event_count++] = 60; }
int has_tech(int, int) { return 0; }
int ascending(int) { return ascending_result; }
int pop_wait() { return 0; }
int parse_says(int, const char *, int, int) { events[event_count++] = 70; return 0; }
void interlude(int, char *, int, int) { ++interlude_count; events[event_count++] = 71; }
void reset_territory() { ++territory_count; }
void do_all_non_input() { ++non_input_count; }
void pact_ends(int, int);
void treaty_on(int, int, int);
void set_treaty(int, int, int, int);
void set_agenda(int, int, int, int);

#include "actual.inc"

static void clear_fixture() {
    memset(Vehs, 0, sizeof Vehs); memset(VehPrototypes, 0, sizeof VehPrototypes);
    memset(Chassis, 0, sizeof Chassis); memset(MapTiles, 0, sizeof MapTiles);
    memset(Players, 0, sizeof Players); memset(PlayersData, 0, sizeof PlayersData);
    memset(Bases, 0, sizeof Bases); memset(Facility, 0, sizeof Facility);
    GameDataStateStorage.fields_.vehicle_current_count_ = GameDataStateStorage.fields_.base_current_count_ = GameDataStateStorage.fields_.turn_current_num_ = GameDataStateStorage.fields_.mission_year_current_ = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0; VehTerraformStingLatch = 42;
    GameDataStateStorage.fields_.climate_sea_level_delta_ = GameDataStateStorage.fields_.climate_rise_step_ = GameDataStateStorage.fields_.climate_rise_limit_ = GameDataStateStorage.fields_.climate_rise_accum_ = 0;
    MapSeaLevel = GenderDefault = PluralityDefault = 0; ConsoleGlobal.faction_id_ = 1;
    event_count = killed_count = setup_count = popup_count = treaty_count = agenda_count = 0;
    random_event_count = interlude_count = territory_count = non_input_count = 0;
    climate_count = redraw_count = random_index = ascending_result = 0;
    spoke_12_seen_at_ordered_pair = -1;
    memset(random_values, 0, sizeof random_values);
    PlayersData[1].flags = PlayersData[1].ranking = 0xffffffffu;
    PlayersData[1].diff_level = -1;
    PlayersData[2].flags = PlayersData[2].ranking = 0xffffffffu;
    PlayersData[2].diff_level = -1;
}

int main() {
    clear_fixture();
    // A unit whose timer is below triad-1 returns immediately and must not
    // advance turn state or run any downstream callbacks.
    GameDataStateStorage.fields_.vehicle_current_count_ = 1; Vehs[0].proto_id = 0; Vehs[0].x = 2; Vehs[0].y = 2;
    Vehs[0].moves_expended = 1; Vehs[0].terraforming_turns = 0;
    VehPrototypes[0].chassis_id = 1; VehPrototypes[0].reactor_id = 1;
    Chassis[1].speed = 2; Chassis[1].triad = 2;
    turn_upkeep();
    assert(Vehs[0].terraforming_turns == 1 && GameDataStateStorage.fields_.turn_current_num_ == 0);
    assert(random_event_count == 0 && non_input_count == 0);

    clear_fixture();
    // The exceptional (triad byte 1, chassis id 6) damage path wraps the
    // byte exactly; it does not run the ordinary clamped repair calculation.
    GameDataStateStorage.fields_.vehicle_current_count_ = 1; Vehs[0].proto_id = 0; Vehs[0].x = 2; Vehs[0].y = 2;
    Vehs[0].dmg_incurred = 250; Vehs[0].moves_expended = 0;
    VehPrototypes[0].chassis_id = 6; VehPrototypes[0].reactor_id = 2;
    Chassis[6].speed = 2; Chassis[6].triad = 1;
    turn_upkeep();
    assert(Vehs[0].dmg_incurred == 0 && (Vehs[0].state & 4) != 0);
    assert(GameDataStateStorage.fields_.turn_current_num_ == 1 && GameDataStateStorage.fields_.mission_year_current_ == 2001);

    clear_fixture();
    GameDataStateStorage.fields_.vehicle_current_count_ = 1; Vehs[0].proto_id = 0; Vehs[0].x = 2; Vehs[0].y = 2;
    Vehs[0].dmg_incurred = 1; Vehs[0].moves_expended = 1;
    Vehs[0].terraforming_turns = 1;
    VehPrototypes[0].chassis_id = 1; VehPrototypes[0].reactor_id = 4;
    VehPrototypes[0].plan = 12; Chassis[1].speed = 2; Chassis[1].triad = 2;
    turn_upkeep();
    assert(killed_count == 1 && killed[0] == 0);

    clear_fixture();
    // Climate threshold crossing mutates state before invoking climate/draw/
    // popup callbacks; the source body then completes the whole upkeep.
    GameDataStateStorage.fields_.turn_current_num_ = 10; GameDataStateStorage.fields_.climate_sea_level_delta_ = 2; GameDataStateStorage.fields_.climate_rise_step_ = 3;
    GameDataStateStorage.fields_.climate_rise_limit_ = 5; GameDataStateStorage.fields_.climate_rise_accum_ = 4;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 11 && GameDataStateStorage.fields_.climate_rise_accum_ == 2);
    assert(GameDataStateStorage.fields_.climate_sea_level_delta_ == 1 && MapSeaLevel == 1);
    assert(climate_count == 1 && redraw_count == 1 && popup_count == 1);
    assert(random_event_count == 1 && non_input_count == 1);

    clear_fixture();
    GameDataStateStorage.fields_.turn_current_num_ = 3;
    random_values[0] = 1; // masked positive nonzero: no rare turn-4 setup.
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 4 && setup_count == 0 && random_index == 1);

    clear_fixture();
    GameDataStateStorage.fields_.turn_current_num_ = 3;
    random_values[0] = static_cast<int>(0x80000000u);
    strcpy(Players[1].filename, "FUNGBOY");
    strcpy(Players[1].search_key, "NOT-THE-NATIVE-NAME");
    PlayersData[1].flags = 0xfffffdffu; PlayersData[1].diff_level = -1;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 4 && setup_count == 1 && random_index == 1);

    clear_fixture();
    GameDataStateStorage.fields_.turn_current_num_ = 5;
    random_values[0] = 1; // masked positive nonzero: no rare turn-6 setup.
    strcpy(Players[1].filename, "FUNGBOY");
    strcpy(Players[1].search_key, "NOT-THE-NATIVE-NAME");
    PlayersData[1].flags = 0xfffffdffu; PlayersData[1].diff_level = -1;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 6 && setup_count == 0 && random_index == 1);

    clear_fixture();
    GameDataStateStorage.fields_.turn_current_num_ = 5;
    random_values[0] = 0; // turn 6's masked 0x80000001 test admits zero.
    strcpy(Players[1].filename, "FUNGBOY");
    strcpy(Players[1].search_key, "NOT-THE-NATIVE-NAME");
    PlayersData[1].flags = 0xfffffdffu; PlayersData[1].diff_level = -1;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 6 && setup_count == 1 && random_index == 1);

    clear_fixture();
    // Turn seven initializes eligible FUNGBOY/alien factions, sets only the
    // alive byte, informs on alien setup, then applies CARETAKE treaty writes
    // in the CARETAKE row and base-owner column.
    GameDataStateStorage.fields_.turn_current_num_ = 6;
    strcpy(Players[1].filename, "fungboy"); Players[1].rule_flags = RFLAG_ALIEN;
    strcpy(Players[1].search_key, "NOT-FUNGBOY");
    strcpy(Players[2].filename, "other"); Players[2].rule_flags = RFLAG_ALIEN;
    strcpy(Players[2].search_key, "NOT-OTHER");
    strcpy(Players[3].filename, "caretake");
    strcpy(Players[3].search_key, "NOT-CARETAKE");
    Players[3].rule_flags = RFLAG_ALIEN;
    GameDataStateStorage.fields_.base_current_count_ = 1; Bases[0].faction_id_current = 1;
    Bases[0].production_id_last = -0x66;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 7 && setup_count == 2);
    assert((reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_)[1] & 0x06) == 0x06);
    assert((reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_)[0] & 0x06) == 0);
    assert(PlayersData[3].diplo_spoke[1] == 7);
    assert(spoke_12_seen_at_ordered_pair == 0);
    assert(treaty_count >= 2 && agenda_count >= 2 && popup_count >= 2);
    assert(PlayersData[1].diplo_spoke[2] == 7 && PlayersData[2].diplo_spoke[1] == 7);
    assert(random_event_count == 1 && non_input_count == 1);

    clear_fixture();
    // The year-75 facility prerequisite is read from the live typed rules
    // table; the ascending path sets its per-faction one-shot flag and uses
    // the native title/name inputs.
    GameDataStateStorage.fields_.turn_current_num_ = 74; ConsoleGlobal.faction_id_ = 1;
    ascending_result = 1; Facility[FAC_BIOLOGY_LAB].preq_tech = 42;
    turn_upkeep();
    assert(GameDataStateStorage.fields_.turn_current_num_ == 75 && PlayersData[1].flags & 0x2000);
    assert(PlayersData[1].flags & 0x2000 && non_input_count == 1);
    assert(random_event_count == 1);
    return 0;
}
