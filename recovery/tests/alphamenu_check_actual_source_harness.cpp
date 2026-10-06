// Host fixture for the assembled source-shaped AlphaMenu::check candidate.
// This includes the candidate definition directly (not the retired seed).
// It is intentionally ignored and uncompiled here; root must replace this
// include with its source-extraction fixture when the candidate is reviewed.
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct Op { char kind; int menu; int item; std::string text; };
static std::vector<Op> operations;
static void record(char kind, int menu, int item, const char *text = "") {
    Op op = {kind, menu, item, text}; operations.push_back(op);
}
struct AlphaMenu {
    void show_menu_item(int m, int i) { record('S', m, i); }
    void hide_menu_item(int m, int i) { record('H', m, i); }
    void enable_menu_item(int m, int i) { record('E', m, i); }
    void disable_menu_item(int m, int i) { record('D', m, i); }
    void rename_menu_item(int m, int i, char *s) { record('R', m, i, s); }
    void check(int);
};

struct GameDataStateWindowFields {
    uint32_t game_preferences_;
    uint32_t game_more_preferences_;
    uint32_t game_warnings_;
    uint32_t game_rules_;
    uint8_t before_game_state_[0x20];
    uint32_t game_state_;                  // +0x030
    uint8_t before_net_turn_flags_[0x358];
    uint32_t net_turn_flags_;              // +0x38C
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
static GameDataStateWindow GameDataStateStorage = {};
typedef char AlphaGameStateOffset[
    offsetof(GameDataStateWindowFields, game_state_) == 0x30 ? 1 : -1];
typedef char AlphaNetTurnFlagsOffset[
    offsetof(GameDataStateWindowFields, net_turn_flags_) == 0x38C ? 1 : -1];
static uint32_t &GameMorePreferences = GameDataStateStorage.fields_.game_more_preferences_;
static uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
static uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
static uint32_t &NetTurnFlags = GameDataStateStorage.fields_.net_turn_flags_;
struct ScenarioSaveState {
    int objective_req_victory;
    uint32_t objectives_sudden_death_victory;
};
static ScenarioSaveState ScenarioSave = {};
static int &ObjectiveReqVictory = ScenarioSave.objective_req_victory;
static uint32_t &ObjectivesSuddenDeathVictory = ScenarioSave.objectives_sudden_death_victory;
static int IsMultiplayerNet, IsMultiplayerPBEM, ConsoleFactionsWaitingMask;
struct ConsoleState { int cursor_active_; int faction_id_; } ConsoleGlobal;
static const uint32_t BSC_SPORE_LAUNCHER = 15;
static const uint32_t ABL_DROP_POD = 0x10, ABL_CARRIER = 0x80;
static const uint8_t TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2;
static const uint32_t DTREATY_COMMLINK = 8, DTREATY_VENDETTA = 0x10;
struct Veh { int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint16_t unk; uint8_t faction_id; };
struct VehPrototype { char name[32]; uint32_t abilities; uint8_t chassis_id;
    uint8_t weapon_id, armor_id, reactor_id, capacity, cost, plan; };
struct ChassisRecord { uint8_t triad; uint8_t bytes[0x4c-1]; uint8_t missile; };
struct WeaponRecord { int8_t offense_rating; };
struct Map { uint8_t climate, contour, val2; uint8_t rest[5]; uint32_t bit; };
struct PlayerData { uint32_t diplo_treaties[8]; };
struct RulesOrder { char *order, *order_sea, *letter; };
struct RulesTerraforming { char *name, *name_sea; int preq_tech, preq_tech_sea;
    uint32_t bit, incompatible; int rate; char *shortcuts; };
static Veh Vehs[8];
static VehPrototype VehPrototypes[64];
static ChassisRecord Chassis[19];
static WeaponRecord Weapon[26];
static PlayerData PlayersData[8];
static RulesOrder Order[30];
static RulesTerraforming Terraforming[20];
static Map MapStorage[32];
static int MapLongitude = 8;
static char StringTemp[1032];
static char order_text[] = "Order", shortcut_text[] = "Key";
static int forced_unavailable_row = -1;
static int action_calls = 0;
static int action_turns = 1;
static std::vector<int> terrain_selectors;
static int territory_owner_result = -1;
static int base_result = -1;
static int facility_result = 0;
static int cargo_result = 0;
static int can_arty_result = 0;
static int has_tech_result = 1;
struct Strings {
    int calls;
    Strings() : calls(0) {}
    char *get(int) { return (calls++ & 1) ? shortcut_text : order_text; }
} StringTable;
static Map *map_loc(int x, int y) { return &MapStorage[(x >> 1) + y * MapLongitude]; }
static int has_abil(int p, uint32_t mask) { return (VehPrototypes[p].abilities & mask) != 0; }
static int whose_territory(int, int, int, void *, int) { return territory_owner_result; }
static int get_proto_offense_rating(uint32_t p) { return Weapon[VehPrototypes[p].weapon_id].offense_rating; }
static int base_at(int, int) { return base_result; }
static bool has_fac_built(int, int) { return facility_result != 0; }
static int veh_cargo(int) { return cargo_result; }
static int can_arty(int, int) { return can_arty_result; }
static int terrain_avail(int row, int selector, int faction_id) {
    terrain_selectors.push_back(selector);
    (void)faction_id;
    record('T', 9, row);
    return row != forced_unavailable_row;
}
static int action_terraform(int, int row, int) {
    ++action_calls;
    record('A', 9, row);
    return action_turns;
}
static int has_tech(int, int) { return has_tech_result; }
static char *label_get(int n) { return n == 0x30 ? const_cast<char *>("turn") : const_cast<char *>("turns"); }
static char *_itoa(int value, char *out, int radix) { std::sprintf(out, radix == 10 ? "%d" : "%x", value); return out; }

/* ACTUAL ALPHAMENU CHECK BODY */

static bool has(char kind, int menu, int item) {
    for (size_t i = 0; i < operations.size(); ++i)
        if (operations[i].kind == kind && operations[i].menu == menu && operations[i].item == item) return true;
    return false;
}
static int find(char kind, int menu, int item, size_t from = 0) {
    for (size_t i = from; i < operations.size(); ++i)
        if (operations[i].kind == kind && operations[i].menu == menu && operations[i].item == item) return (int)i;
    return -1;
}

static void regular_plan_uses_order_rows_and_availability() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 10; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND; operations.clear(); StringTable.calls = 0;
    AlphaMenu menu; menu.check(0);
    assert(has('R', 9, 0x900));
    assert(has('E', 9, 0x912));
    assert(has('S', 3, 0x317) && has('H', 3, 0x316));
    assert(!has('D', 3, 0x313));
}

static void plan_nine_uses_cost_and_turn_label_without_false_guard_fallback() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 9; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND; MapStorage[9].bit = 0x80000000u | 4u;
    operations.clear(); StringTable.calls = 0; AlphaMenu menu; menu.check(0);
    const int row = find('R', 9, 0x901);
    assert(row >= 0 && operations[row].text.find("Order (2 turns)|Key") != std::string::npos);
    assert(has('E', 3, 0x313) && !has('D', 3, 0x312));
    assert(has('S', 9, 0x9F8) && has('E', 9, 0x9F9));
}

static void inactive_cursor_or_negative_vehicle_uses_native_fallback_join() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0;
    ConsoleGlobal.faction_id_ = 0;
    AlphaMenu menu;

    ConsoleGlobal.cursor_active_ = 0;
    operations.clear();
    menu.check(0);
    assert(!has('R', 9, 0x900));
    for (int item = 0x300; item <= 0x324; ++item) {
        assert(item == 0x313 ? has('E', 3, item) : has('D', 3, item));
    }

    ConsoleGlobal.cursor_active_ = 1;
    operations.clear();
    menu.check(-1);
    assert(!has('R', 9, 0x900));
    for (int item = 0x300; item <= 0x324; ++item) {
        assert(item == 0x313 ? has('E', 3, item) : has('D', 3, item));
    }
}

static void plan_nine_gates_special_rows_before_cost_and_labels() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 9; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND;
    MapStorage[9].bit = 0; // neither row-specific feature flag is present
    forced_unavailable_row = 18;
    action_calls = 0; action_turns = 2; operations.clear();
    StringTable.calls = 0; AlphaMenu menu; menu.check(0);
    assert(find('T', 9, 1) < 0 && find('T', 9, 6) < 0);
    assert(find('A', 9, 1) < 0 && find('A', 9, 6) < 0);
    assert(action_calls == 16); // rows 1 and 6 are blocked; row 18 unavailable
    assert(has('H', 9, 1));
    assert(has('D', 9, 0x912) && has('H', 9, 0x912));
    forced_unavailable_row = -1;
}

static void unit_zero_is_admitted_and_negative_or_inactive_uses_fallback() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0;
    ConsoleGlobal.cursor_active_ = 1; ConsoleGlobal.faction_id_ = 0;
    Vehs[0].x = 2; Vehs[0].y = 1; Vehs[0].proto_id = 0;
    VehPrototypes[0].plan = 10; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND; operations.clear();
    AlphaMenu menu; menu.check(0);
    assert(find('T', 9, 0) >= 0 && has('R', 9, 0x900));
    operations.clear(); menu.check(-1);
    assert(find('T', 9, 0) < 0 && !has('R', 9, 0x900));
    assert(has('E', 3, 0x313) && has('D', 3, 0x312));
}

static void independent_prefix_gates_preserve_ordered_mutations() {
    AlphaMenu menu;
    GameMorePreferences = 0; GameRules = 0; GameState = 0;
    IsMultiplayerNet = 0; IsMultiplayerPBEM = 1;
    ConsoleGlobal.cursor_active_ = 0; ConsoleGlobal.faction_id_ = 0;
    operations.clear(); menu.check(-1);
    assert(find('H', 0, 0x5C) >= 0);
    assert(find('H', 0, 0x5A) > find('H', 0, 0x5C));

    IsMultiplayerPBEM = 0; GameRules = 0; GameState = 0;
    GameMorePreferences = 0x200000;
    operations.clear(); menu.check(-1);
    assert(has('S', 0, 0x5C) && has('S', 0, 0x5A));
    assert(has('S', 2, 0x205) && has('S', 3, 0x31B));

    GameMorePreferences = 0; GameRules = 0x400; GameState = 0;
    operations.clear(); menu.check(-1);
    assert(!has('S', 0, 0x5C) && has('H', 0, 0x5A));

    GameRules = 0; GameState = 0x1000;
    operations.clear(); menu.check(-1);
    assert(has('S', 0, 0x5A) && !has('H', 0, 0x5A));

    GameState = 0; ObjectiveReqVictory = 1000;
    ObjectivesSuddenDeathVictory = 1000;
    operations.clear(); menu.check(-1);
    assert(has('H', 1, 0x115));
    ObjectivesSuddenDeathVictory = -1;
    operations.clear(); menu.check(-1);
    assert(has('S', 1, 0x115) && !has('H', 1, 0x115));
    ObjectivesSuddenDeathVictory = 1000; GameState = 0x10;
    operations.clear(); menu.check(-1);
    assert(has('S', 1, 0x115) && !has('H', 1, 0x115));
    ObjectiveReqVictory = 999; GameState = 0;
    operations.clear(); menu.check(-1);
    assert(has('S', 1, 0x115) && !has('H', 1, 0x115));

    IsMultiplayerNet = 1; GameState = 2; ConsoleFactionsWaitingMask = 1;
    operations.clear(); menu.check(-1);
    assert(find('H', 8, 0x807) < find('S', 8, 0x808));
    ConsoleFactionsWaitingMask = 0xFF;
    operations.clear(); menu.check(-1);
    assert(has('S', 8, 0x807) && has('H', 8, 0x808));

    IsMultiplayerNet = 0; GameState = 0xC0; NetTurnFlags = 8;
    GameRules = 0x200; operations.clear(); menu.check(-1);
    assert(has('H', 1, 0x105));
    assert(find('S', 1, 0x106) < find('S', 1, 0x107));
    assert(find('S', 1, 0x107) < find('H', 1, 0x106));
    GameRules = 0; operations.clear(); menu.check(-1);
    assert(find('S', 1, 0x107) < find('S', 1, 0x106));
    assert(find('S', 1, 0x106) < find('H', 1, 0x106));
}

static void native_terrain_plan_technology_and_final_gates_are_independent() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0x40;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 9; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND; MapStorage[9].bit = 0x80000000u | 4u;
    has_tech_result = 0; forced_unavailable_row = 2;
    operations.clear(); StringTable.calls = 0; AlphaMenu menu; menu.check(0);
    assert(has('E', 6, 0x601) && has('E', 6, 0x61B));
    assert(has('E', 10, 0xA00) && has('E', 10, 0xA54));
    assert(has('D', 9, 0x9F6) && has('D', 9, 0x9F0));
    assert(has('H', 9, 0x9F3) && has('H', 9, 0x9F1));
    assert(has('H', 9, 0x9F4) && has('H', 9, 0x9F5));
    assert(find('T', 9, 2) < find('R', 9, 0x902));
    assert(find('R', 9, 0x902) < find('D', 9, 0x902));

    has_tech_result = 1; GameState = 0; GameMorePreferences = 0;
    operations.clear(); menu.check(0);
    assert(has('E', 9, 0x9F6) && has('E', 9, 0x9F0));
    assert(has('S', 9, 0x9F3) && has('E', 9, 0x9F4));
    assert(has('E', 9, 0x9F5));
    assert(has('H', 2, 0x205) && has('H', 3, 0x31F));
    forced_unavailable_row = -1; has_tech_result = 1;
}

static void unit_features_use_typed_owners_and_preserve_overwrites() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 10; VehPrototypes[0].chassis_id = 0;
    VehPrototypes[0].weapon_id = 0x18; Chassis[0].triad = TRIAD_SEA;
    VehPrototypes[0].abilities = ABL_DROP_POD;
    Chassis[0].missile = 1; Weapon[0x18].offense_rating = -1;
    territory_owner_result = 1; PlayersData[0].diplo_treaties[1] = DTREATY_COMMLINK;
    base_result = 0; facility_result = 1; cargo_result = 1; can_arty_result = 1;
    operations.clear(); AlphaMenu menu; menu.check(0);
    const int treaty_show = find('S', 3, 0x31F);
    assert(treaty_show >= 0 && find('S', 3, 0x31F, treaty_show + 1) > treaty_show);
    assert(has('S', 3, 0x30A) && has('S', 3, 0x310));
    assert(has('S', 3, 0x31D) && has('S', 3, 0x306) && has('S', 3, 0x304));
    assert(has('S', 3, 0x305));
    const int action_plan_show = find('S', 3, 0x308);
    assert(action_plan_show >= 0 &&
           find('E', 3, 0x308, action_plan_show + 1) > action_plan_show);
    assert(has('S', 3, 0x309));
    MapStorage[9].bit = 1; MapStorage[9].val2 = 0;
    operations.clear(); menu.check(0);
    const int action_show = find('S', 3, 0x302);
    assert(action_show >= 0 && find('E', 3, 0x302, action_show + 1) > action_show);
    Chassis[0].triad = TRIAD_AIR; MapStorage[9].val2 = 1;
    operations.clear(); menu.check(0);
    assert(has('D', 3, 0x302) && has('H', 3, 0x302));
    assert(has('H', 3, 0x306) && has('H', 3, 0x304));

    Chassis[0].triad = TRIAD_LAND; MapStorage[9].bit = 0;
    MapStorage[9].val2 = 0; VehPrototypes[0].weapon_id = 0;
    Weapon[0].offense_rating = 0; territory_owner_result = 1;
    PlayersData[0].diplo_treaties[1] = DTREATY_COMMLINK | DTREATY_VENDETTA;
    GameMorePreferences = 0;
    operations.clear(); menu.check(0);
    assert(!has('S', 3, 0x31F));

    VehPrototypes[0].abilities = ABL_DROP_POD | ABL_CARRIER;
    cargo_result = 0; territory_owner_result = -1;
    operations.clear(); menu.check(0);
    assert(has('S', 3, 0x309) && has('S', 3, 0x31D));
    VehPrototypes[0].abilities = 0;

    territory_owner_result = -1; base_result = -1; facility_result = 0;
    cargo_result = 0; can_arty_result = 0; Chassis[0].missile = 0;
    GameMorePreferences = 0x200000;
    Chassis[0].triad = TRIAD_LAND; Weapon[0x18].offense_rating = 0;
}

static void plan_eight_uses_owner_nibble_boundary_and_ordered_transitions() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 8; VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND; AlphaMenu menu;
    MapStorage[9].bit = 1; MapStorage[9].val2 = 7;
    operations.clear(); menu.check(0);
    assert(find('E', 3, 0x300) < find('S', 3, 0x301));
    assert(find('S', 3, 0x301) < find('H', 3, 0x300));
    MapStorage[9].val2 = 8; operations.clear(); menu.check(0);
    assert(find('H', 3, 0x301) < find('S', 3, 0x300));
    MapStorage[9].bit = 0; operations.clear(); menu.check(0);
    assert(find('H', 3, 0x301) < find('S', 3, 0x300));
    assert(!has('D', 3, 0x300)); // plan 8's enable survives its later visibility choice
}

static void terrain_label_selector_uses_sea_and_strict_air_climate_boundary() {
    GameMorePreferences = 0x200000; GameRules = 0x400; GameState = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0; ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.faction_id_ = 0; Vehs[0].x = 2; Vehs[0].y = 1;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 0;
    VehPrototypes[0].plan = 10; VehPrototypes[0].chassis_id = 0;
    AlphaMenu menu;

    Chassis[0].triad = TRIAD_SEA; MapStorage[9].climate = 0xE0;
    terrain_selectors.clear(); operations.clear(); menu.check(0);
    assert(terrain_selectors.size() == 19);
    for (size_t i = 0; i != terrain_selectors.size(); ++i) assert(terrain_selectors[i] == 1);

    Chassis[0].triad = TRIAD_AIR; MapStorage[9].climate = 0x40;
    terrain_selectors.clear(); operations.clear(); menu.check(0);
    assert(terrain_selectors.size() == 19);
    for (size_t i = 0; i != terrain_selectors.size(); ++i) assert(terrain_selectors[i] == 1);

    MapStorage[9].climate = 0x60;
    terrain_selectors.clear(); operations.clear(); menu.check(0);
    assert(terrain_selectors.size() == 19);
    for (size_t i = 0; i != terrain_selectors.size(); ++i) assert(terrain_selectors[i] == 0);
    Chassis[0].triad = TRIAD_LAND; MapStorage[9].climate = 0;
}

int main() {
    Order[4].order = order_text; Order[4].order_sea = order_text;
    Terraforming[0].shortcuts = shortcut_text;
    regular_plan_uses_order_rows_and_availability();
    plan_nine_uses_cost_and_turn_label_without_false_guard_fallback();
    inactive_cursor_or_negative_vehicle_uses_native_fallback_join();
    plan_nine_gates_special_rows_before_cost_and_labels();
    unit_zero_is_admitted_and_negative_or_inactive_uses_fallback();
    independent_prefix_gates_preserve_ordered_mutations();
    native_terrain_plan_technology_and_final_gates_are_independent();
    unit_features_use_typed_owners_and_preserve_overwrites();
    plan_eight_uses_owner_nibble_boundary_and_ordered_transitions();
    terrain_label_selector_uses_sea_and_strict_air_climate_boundary();
}
