// Host-only extracted-body control. test_diplomacy_lifecycle.py inserts the
// two real faction.cpp definitions at EXTRACTED_HELPERS; this is not runtime
// or ABI evidence for the game executable.
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#ifndef __cdecl
#define __cdecl
#endif
typedef int BOOL;
static const int MaxPlayerNum = 8;
enum VehTriad { TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2 };

struct Player {
    BOOL is_leader_female;
    char noun_faction[24];
    int noun_gender;
    BOOL is_noun_plural;
    char title_leader[24];
    char name_leader[24];
};
struct PlayerData {
    uint32_t diplo_treaties[8];
    int diplo_spoke[8];
};
struct Base {
    int16_t x;
    int16_t y;
    uint8_t faction_id_current;
    uint8_t padding[0x134 - 5];
};
struct Veh {
    int16_t x;
    int16_t y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
    uint8_t faction_id;
    uint8_t year_end_lurking;
    uint8_t dmg_incurred;
    int8_t order;
    uint8_t waypoint_count;
    uint8_t patrol_current_point;
    int16_t waypoint_x[4];
    int16_t waypoint_y[4];
    uint8_t morale;
    uint8_t terraforming_turns;
    uint8_t order_auto_type;
    uint8_t visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unk_6;
    uint8_t move_to_ai_type;
    uint8_t probe_action;
    uint8_t probe_sabotage_id;
    int16_t home_base_id;
    int16_t next_veh_id_stack;
    int16_t prev_veh_id_stack;
};
struct VehPrototype {
    char name[32];
    uint32_t ability_flags;
    uint8_t chassis_id;
    uint8_t weapon_id, armor_id, reactor_id, carry_capacity, cost, plan;
    int8_t unk_1;
    uint8_t obsolete_factions;
    int8_t combat_factions;
    int8_t icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
};
struct RulesChassis {
    uint32_t names[6];
    uint32_t genders[6];
    BOOL plurals[6];
    uint8_t speed;
    uint8_t triad;
    uint8_t range, cargo, cost, missile;
    uint8_t sprite_offsets[8 * 8];
    int16_t preq_tech;
};
struct Map { uint8_t climate; uint8_t contour, val2, region; };
struct Console {
    int faction_id_;
    uint32_t cursor_active_;
    int selected_vehicle_;
    int move_updates;
    void set_move(int force) { move_updates += force; }
};
typedef char Veh_must_match_native_stride[(sizeof(Veh) == 0x34) ? 1 : -1];
typedef char Base_must_match_native_stride[(sizeof(Base) == 0x134) ? 1 : -1];
typedef char Proto_chassis_offset_must_match[(offsetof(VehPrototype, chassis_id) == 0x24) ? 1 : -1];
typedef char Chassis_triad_offset_must_match[(offsetof(RulesChassis, triad) == 0x49) ? 1 : -1];
typedef char Chassis_stride_must_match[(sizeof(RulesChassis) == 0x90) ? 1 : -1];
typedef char Proto_stride_must_match[(sizeof(VehPrototype) == 0x34) ? 1 : -1];
typedef char Veh_terraform_offset_must_match[(offsetof(Veh, terraforming_turns) == 0x25) ? 1 : -1];

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
uint32_t LockEnableMask;
int GenderDefault;
BOOL PluralityDefault;
int TurnCurrentNum;
Console ConsoleGlobal;
Veh Vehs[16];
int VehCurrentCount;
VehPrototype VehPrototypes[16];
RulesChassis Chassis[16];
Base Bases[16];
int BaseCurrentCount;
uint32_t MapLongitude;
int MapLongitudeBounds;
BOOL MapIsFlat;
Map TestTile;

struct ParseEvent { int slot; const char *text; int gender; int plural; };
ParseEvent parse_events[64];
int parse_count;
struct TreatyEvent { int first, second, value, extra; };
TreatyEvent treaty_events[8];
int treaty_count;
TreatyEvent set_treaty_events[8];
int set_treaty_count;
char popup_label[32];
int popup_count;
int base_find_result;
int territory_result;
int coast_result;
int coast_calls;
int coast_x, coast_y, coast_radius;
int sea_result;
int sea_calls, sea_base, sea_region;
int lift_result;
int lift_arg, lift_calls;
int drop_arg, drop_x, drop_y, drop_calls;
int mutate_base_on_lift, mutate_base_id, mutate_base_x, mutate_base_y;
int failures;

int __cdecl parse_says(int id, const char *input, int gender, int plural) {
    if (parse_count >= 64) abort();
    parse_events[parse_count].slot = id;
    parse_events[parse_count].text = input;
    parse_events[parse_count].gender = GenderDefault;
    parse_events[parse_count].plural = PluralityDefault;
    ++parse_count;
    (void)gender; (void)plural;
    return 0;
}
void __cdecl treaty_on(int first, int second, int value) {
    treaty_events[treaty_count].first = first;
    treaty_events[treaty_count].second = second;
    treaty_events[treaty_count].value = value;
    treaty_events[treaty_count].extra = 0;
    ++treaty_count;
}
void __cdecl set_treaty(int first, int second, int value, int extra) {
    set_treaty_events[set_treaty_count].first = first;
    set_treaty_events[set_treaty_count].second = second;
    set_treaty_events[set_treaty_count].value = value;
    set_treaty_events[set_treaty_count].extra = extra;
    ++set_treaty_count;
}
int __cdecl X_pop(const char *label, int (__cdecl *callback)()) {
    strncpy(popup_label, label, sizeof(popup_label) - 1);
    popup_label[sizeof(popup_label) - 1] = 0;
    ++popup_count;
    (void)callback;
    return 0;
}
int __cdecl base_find(int, int) { return base_find_result; }
int __cdecl whose_territory(int, int, int, int *, BOOL) { return territory_result; }
BOOL __cdecl is_coast(int x, int y, BOOL radius) {
    ++coast_calls; coast_x = x; coast_y = y; coast_radius = radius;
    return coast_result;
}
BOOL __cdecl base_on_sea(int base_id, int region) {
    ++sea_calls; sea_base = base_id; sea_region = region;
    return sea_result;
}
Map *map_loc(int, int) { return &TestTile; }
int __cdecl veh_lift(int vehicle_id) {
    ++lift_calls; lift_arg = vehicle_id;
    if (mutate_base_on_lift) {
        Bases[mutate_base_id].x = static_cast<int16_t>(mutate_base_x);
        Bases[mutate_base_id].y = static_cast<int16_t>(mutate_base_y);
    }
    return lift_result;
}
int __cdecl veh_drop(int vehicle_id, int x, int y) {
    ++drop_calls; drop_arg = vehicle_id; drop_x = x; drop_y = y;
    return vehicle_id;
}

/*EXTRACTED_HELPERS*/

static void reset_events() {
    memset(parse_events, 0, sizeof(parse_events)); parse_count = 0;
    memset(treaty_events, 0, sizeof(treaty_events)); treaty_count = 0;
    memset(set_treaty_events, 0, sizeof(set_treaty_events)); set_treaty_count = 0;
    memset(popup_label, 0, sizeof(popup_label)); popup_count = 0;
    coast_calls = sea_calls = lift_calls = drop_calls = 0;
    coast_x = coast_y = coast_radius = 0;
    sea_base = sea_region = lift_arg = drop_arg = drop_x = drop_y = 0;
    mutate_base_on_lift = 0; mutate_base_id = -1;
    mutate_base_x = mutate_base_y = 0;
    ConsoleGlobal.move_updates = 0;
}
static void reset_world() {
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Chassis, 0, sizeof(Chassis));
    memset(Bases, 0, sizeof(Bases));
    memset(&TestTile, 0, sizeof(TestTile));
    LockEnableMask = 0; GenderDefault = 0; PluralityDefault = 0;
    TurnCurrentNum = 42; ConsoleGlobal.faction_id_ = -1;
    ConsoleGlobal.cursor_active_ = 0; ConsoleGlobal.selected_vehicle_ = -1;
    VehCurrentCount = 0; BaseCurrentCount = 0;
    MapLongitude = 64; MapLongitudeBounds = 128; MapIsFlat = 1;
    base_find_result = -1; territory_result = -1;
    coast_result = 1; sea_result = 1; lift_result = 0;
    reset_events();
}
static void require(int value, const char *message) {
    if (!value) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static void test_intervention_masks_slots_and_effects() {
    reset_world();
    const int target = 1, actor = 2, candidate = 3;
    Players[candidate].noun_gender = 31;
    Players[candidate].is_noun_plural = 1;
    strcpy(Players[candidate].noun_faction, "candidate-noun");
    Players[candidate].is_leader_female = 1;
    strcpy(Players[candidate].title_leader, "candidate-title");
    strcpy(Players[candidate].name_leader, "candidate-name");
    Players[target].noun_gender = 11;
    Players[target].is_noun_plural = 1;
    strcpy(Players[target].noun_faction, "target-noun");
    Players[actor].noun_gender = 22;
    Players[actor].is_noun_plural = 0;
    strcpy(Players[actor].noun_faction, "actor-noun");
    Players[actor].is_leader_female = 1;
    strcpy(Players[actor].title_leader, "actor-title");
    strcpy(Players[actor].name_leader, "actor-name");
    PlayersData[candidate].diplo_treaties[target] = 1;
    PlayersData[candidate].diplo_treaties[actor] = 0;
    // Alive is byte 1; the separate low byte marks the actor human.
    LockEnableMask = (1u << (candidate + 8)) | (1u << actor);
    ConsoleGlobal.faction_id_ = actor;

    intervention(target, actor);
    require(parse_count == 7, "intervention parses all seven slots");
    for (int i = 0; i < 7; ++i)
        require(parse_events[i].slot == i, "intervention parser slot order");
    require(strcmp(parse_events[0].text, "candidate-noun") == 0,
            "slot 0 belongs to candidate noun");
    require(parse_events[0].gender == 31 && parse_events[0].plural == 1,
            "slot 0 receives candidate noun grammar");
    require(strcmp(parse_events[1].text, "target-noun") == 0,
            "slot 1 belongs to target noun");
    require(parse_events[1].gender == 11 && parse_events[1].plural == 1,
            "slot 1 receives target noun grammar");
    require(strcmp(parse_events[2].text, "candidate-title") == 0 &&
            parse_events[2].gender == 1 && parse_events[2].plural == 0,
            "slot 2 candidate leader title grammar");
    require(strcmp(parse_events[3].text, "candidate-name") == 0 &&
            parse_events[3].gender == 0 && parse_events[3].plural == 0,
            "slot 3 candidate leader name defaults");
    require(strcmp(parse_events[4].text, "actor-title") == 0 &&
            parse_events[4].gender == 1, "slot 4 actor leader title");
    require(strcmp(parse_events[5].text, "actor-name") == 0 &&
            parse_events[5].gender == 0, "slot 5 actor leader name");
    require(strcmp(parse_events[6].text, "actor-noun") == 0 &&
            parse_events[6].gender == 22 && parse_events[6].plural == 0,
            "slot 6 actor noun grammar");
    require(treaty_count == 1 && treaty_events[0].first == candidate &&
            treaty_events[0].second == actor &&
            treaty_events[0].value == static_cast<int>(0x80000018u),
            "intervention treaty_on order and operands");
    require(set_treaty_count == 1 && set_treaty_events[0].first == candidate &&
            set_treaty_events[0].second == actor && set_treaty_events[0].value == 0x40 &&
            set_treaty_events[0].extra == TRUE, "intervention treaty set");
    require(PlayersData[actor].diplo_spoke[candidate] == TurnCurrentNum,
            "contact timestamp is actor row, candidate column");
    require(strcmp(popup_label, "AIDSTHEM") == 0 && popup_count == 1,
            "actor-local notification uses AIDSTHEM");

    reset_events();
    PlayersData[actor].diplo_spoke[candidate] = -9;
    LockEnableMask = (1u << (candidate + 8)) | (1u << target);
    ConsoleGlobal.faction_id_ = target;
    intervention(target, actor);
    require(strcmp(popup_label, "AIDSUS") == 0 && popup_count == 1,
            "target-local notification uses AIDSUS");
    require(PlayersData[actor].diplo_spoke[candidate] == -9,
            "target human does not write actor-to-candidate contact time");

    // Clear the human byte but keep the alive byte. Parsing still occurs,
    // while treaty writes and contact bookkeeping are gated off.
    reset_events();
    PlayersData[actor].diplo_spoke[candidate] = -9;
    LockEnableMask = (1u << (candidate + 8));
    intervention(target, actor);
    require(parse_count == 7, "alive candidate still builds parser state");
    require(treaty_count == 0 && set_treaty_count == 0,
            "alive mask alone does not authorize treaty updates");
    require(PlayersData[actor].diplo_spoke[candidate] == -9,
            "alive mask alone does not write spoke time");

    // A human bit without candidate alive must not enter the candidate body.
    reset_events();
    LockEnableMask = (1u << actor);
    intervention(target, actor);
    require(parse_count == 0, "candidate alive mask is the high byte");
}

static void setup_one_vehicle() {
    VehCurrentCount = 1; BaseCurrentCount = 3;
    Vehs[0].x = 20; Vehs[0].y = 30; Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 0; Vehs[0].order = 7; Vehs[0].state = 0xFFFFFFFFu;
    Vehs[0].morale = 5;
    Vehs[0].terraforming_turns = 9;
    Vehs[0].waypoint_y[3] = 0x1234;
    VehPrototypes[0].chassis_id = 0;
    base_find_result = 0;
    Bases[0].faction_id_current = 2;
    Bases[1].faction_id_current = 1;
    Bases[2].faction_id_current = 1;
    Bases[1].x = 22; Bases[1].y = 31;
    Bases[2].x = 22; Bases[2].y = 31;
    territory_result = -1;
    ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.selected_vehicle_ = 0;
    lift_result = 11;
}
static void test_pact_withdraw_zero_result_and_tie_lift_return() {
    reset_world();
    require(pact_withdraw(1, 2) == 0, "zero vehicles returns zero");
    require(lift_calls == 0 && drop_calls == 0, "zero vehicles does no relocation");

    reset_world(); setup_one_vehicle();
    Chassis[0].triad = TRIAD_AIR;
    mutate_base_on_lift = 1; mutate_base_id = 1;
    mutate_base_x = 55; mutate_base_y = 66;
    require(pact_withdraw(1, 2) == 1, "one winning vehicle returns one");
    require(lift_calls == 1 && lift_arg == 0, "vehicle id passed to lift");
    require(drop_calls == 1 && drop_arg == 11,
            "veh_lift return, not input id, reaches veh_drop");
    require(drop_x == 22 && drop_y == 31,
            "native captures first tied destination coordinates before lift");
    require(Vehs[0].order == 0 && Vehs[0].state == (0xFFFFFFFFu & 0xFCFFBDFFu),
            "withdraw clears native order and state mask");
    require(Vehs[0].morale == 5, "air chassis leaves native morale byte unchanged");
    require(Vehs[0].terraforming_turns == 0,
            "air chassis clears native terraforming-turns field at record +0x25");
    require(ConsoleGlobal.move_updates == 1,
            "selected active vehicle refreshes console movement");
}
static void test_pact_withdraw_sea_rules_preserve_coastal_quirk() {
    reset_world(); setup_one_vehicle();
    BaseCurrentCount = 2;
    Chassis[0].triad = TRIAD_SEA;
    // The native uses candidate Base[1].x (=0) as another Bases[] index,
    // then passes candidate Base[1].y (=7) as is_coast's boolean argument.
    Bases[0].x = 11; Bases[0].y = 13;
    Bases[1].x = 0; Bases[1].y = 7;
    TestTile.climate = 0x20; TestTile.region = 9;
    coast_result = 1; sea_result = 0;
    require(pact_withdraw(1, 2) == 0,
            "sea vehicle needs base_on_sea on nonslope tile");
    require(coast_calls == 1 && coast_x == 11 && coast_y == 13 && coast_radius == 7,
            "coastal test preserves coordinate-as-index / y-as-radius behavior");
    require(sea_calls == 1 && sea_base == 1 && sea_region == 9,
            "sea-region helper receives candidate base and tile region");

    reset_events(); sea_result = 1; coast_result = 1;
    require(pact_withdraw(1, 2) == 1,
            "sea candidate accepted when region helper succeeds");
    require(drop_x == Bases[1].x && drop_y == Bases[1].y,
            "accepted sea candidate relocation destination");
}

int main() {
    failures = 0;
    test_intervention_masks_slots_and_effects();
    test_pact_withdraw_zero_result_and_tie_lift_return();
    test_pact_withdraw_sea_rules_preserve_coastal_quirk();
    if (failures != 0) return 1;
    puts("diplomacy lifecycle extracted-source controls passed");
    return 0;
}
