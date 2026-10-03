"""Actual-source colony growth/upgrade controls; mocks are not game runtime proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
GROWTH_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>

#define __cdecl
const int MaxAbilityNum = 29;
const int RFLAG_ALIEN = 0x80;
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left, nerve_staple_turns_left, unk_1, visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unk_x, unk_y;
    uint32_t state, event, governor;
    int nutrients_accumulated, minerals_accumulated, production_id_last, eco_damage;
    uint32_t queue_size;
    int queue_production_id[10];
    int resource_sqr_radius, specialist_total;
    int specialist_unk_1;
    uint32_t spec[2];
    uint8_t facilities_built[12];
    int mineral_surplus_final, minerals_accumulated_2;
    int unk_6, unk_7, unk_8, unk_9;
    int nutrient_intake_1, mineral_intake_1, energy_intake_1, unused_intake_1;
    int nutrient_intake_2, mineral_intake_2, energy_intake_2, unused_intake_2;
    int nutrient_surplus, mineral_surplus, energy_surplus, unused_surplus;
};
struct Player { uint32_t rule_flags; int rule_population; };
struct PlayerData {
    uint32_t ranking;
    int energy_reserves;
    int ai_power, ai_tech, tech_fungus_nutrient;
    uint8_t proto_id_active[512];
};
struct Veh { int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id, unk;
    uint8_t faction_id, year_end_lurking, dmg_incurred; int8_t order;
    uint8_t waypoint_count, patrol_current_point; int16_t waypoint_x[4], waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility, moves_expended;
    int8_t facing; uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id, weapon_id,
    armor_id, reactor_id, carry_capacity, cost, plan; int8_t unk_1;
    uint8_t obsolete_factions; int8_t combat_factions, icon_offset, padding;
    uint16_t flags; int16_t preq_tech; };
struct RulesAbility { char *name, *description, *abbreviation; };
struct RulesChassis { char *padding[12]; uint8_t speed, triad; };
struct RulesBasic { uint32_t nutrient_req_citizen, padding[11], pop_limit_sans_hab_complex,
    pop_limit_sans_hab_dome; int padding2[4], tech_three_nutrients_sqr; };
struct RulesFacility { char *name; };
struct BaseSecretProject { int ascetic_virtues, cloning_vats; };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1, territory;
    uint32_t bit, bit2, bit_visible[7]; };
struct Strings { const char *get(int); };
struct NetDaemon { int receive() { return 0; } };
struct Console { int faction_id_; };

Base base_storage[512]; Base *BaseCurrent;
Base Bases[512];
Player Players[8]; PlayerData PlayersData[8];
Veh Vehs[2049]; int VehCurrentCount;
VehPrototype VehPrototypes[64];
RulesAbility Ability[MaxAbilityNum]; RulesChassis Chassis[16];
RulesFacility Facility[64]; RulesBasic Rules;
BaseSecretProject SecretProject; Map MapTiles[4096];
uint32_t MapLongitude = 32, MapIsFlat;
int MapLongitudeBounds = 64, MapLatitudeBounds = 64;
int RadiusOffsetX[289], RadiusOffsetY[289];
int GameState, BaseIDCurrentSelected, BaseCurrentGrowthRate, BaseComputedID;
uint32_t BaseCurrentConvoyFrom[4], LockEnableMask, GameWarnings;
int BasePsychDrones[5], DiffLevelCurrent, RankingFactionIDUnk1, RankingFactionIDUnk2;
int IsMultiplayerNet, UpgradePromptAnswered;
char StringTemp[1032]; char *ScriptPopupFile = const_cast<char *>("popup.txt");
Strings StringTable; NetDaemon NetDaemonNet;

int popp_label, popp_picture, popp_result, last_string_id;
const char *last_label, *last_picture;
const char *Strings::get(int id) { last_string_id = id; return "ABBR"; }
int popp(const char *, const char *label, int, const char *picture, int) {
    last_label = label; last_picture = picture; ++popp_label; ++popp_picture;
    return popp_result;
}
int popb_calls, kill_calls, objective_result, has_fac_calls, last_fac_base;
Base *popup_replacement;
int popb(const char *, int, int, const char *, void *) {
    ++popb_calls;
    if (popup_replacement) BaseCurrent = popup_replacement;
    return 0;
}
int cost_factor(int, int, int) { return 10; }
int upgrade_cost(int, int, int) { return 10; }
int has_fac(int, int base_id, int) { ++has_fac_calls; last_fac_base = base_id; return 0; }
int has_tech(int, int) { return 0; }
int has_abil(int, int ability) { return ability == 1; }
int is_objective(int) { return objective_result; }
void base_kill(int) { ++kill_calls; }
void draw_map(int) {}
void draw_tile(int, int, int) {}
void eliminate_player(int, int) {}
void base_support() {}
void base_yield() {}
void base_nutrient() {}
void base_minerals() {}
void base_energy() {}
void bitmask(int, int *byte, int *mask) { *byte = 1; *mask = 0x20; }
void say_stats_2(char *, int) {}
int parse_num(int, int) { return 0; }
int parse_says(int, const char *, int, int) { return 0; }
int parse_say(int, int, int, int) { return 0; }
int do_upgrade_calls;
void do_upgrade(int, int, int) { ++do_upgrade_calls; }
int message_data(int, int, int, int, int, int) { return 0; }


YIELD_HELPERS
BODY

static void reset_case() {
    for (int i=0; i<512; ++i) Bases[i] = Base();
    BaseIDCurrentSelected = 5;
    BaseCurrent = &Bases[5];
    BaseCurrent->faction_id_current = 2;
    BaseCurrent->population_size = 2;
    BaseCurrent->x = 2; BaseCurrent->y = 2;
    SecretProject.ascetic_virtues = SecretProject.cloning_vats = -1;
    Rules.pop_limit_sans_hab_complex = Rules.pop_limit_sans_hab_dome = 100;
    Rules.nutrient_req_citizen = 2;
    Players[2].rule_population = 0;
    GameState = BaseCurrentGrowthRate = popb_calls = kill_calls = objective_result = has_fac_calls = 0;
    popup_replacement = 0;
}
int main() {
    reset_case();
    BaseCurrent->nutrients_accumulated = -1;
    BaseCurrent->nutrient_surplus = 3;
    GameState = 1; // Native disallows starvation behavior, but resets subtotal.
    assert(base_growth() == 0);
    assert(BaseCurrent->nutrients_accumulated == 3 && popb_calls == 0);
    assert(BaseCurrent->population_size == 2);

    reset_case();
    BaseCurrent->nutrients_accumulated = -1;
    BaseCurrent->nutrient_surplus = 3;
    assert(base_growth() == 0 && popb_calls == 1);
    assert(BaseCurrent->population_size == 1 && BaseCurrent->nutrients_accumulated == 3);
    assert(kill_calls == 0);

    reset_case();
    BaseCurrent->population_size = 1;
    BaseCurrent->nutrients_accumulated = -1;
    objective_result = 1;
    assert(base_growth() == 0 && BaseCurrent->population_size == 1 && kill_calls == 0);

    reset_case();
    BaseCurrent->population_size = 1;
    BaseCurrent->nutrients_accumulated = -1;
    assert(base_growth() == 1 && BaseCurrent->population_size == 0 && kill_calls == 1);

    // Popup replaces the global current Base: subsequent state is not stale.
    reset_case();
    BaseCurrent->nutrients_accumulated = -1;
    Bases[6].population_size = 4; Bases[6].nutrient_surplus = 7;
    popup_replacement = &Bases[6];
    assert(base_growth() == 0 && BaseCurrent == &Bases[6]);
    assert(Bases[5].population_size == 2 && Bases[6].population_size == 3);
    assert(Bases[6].nutrients_accumulated == 7);

    reset_case();
    BaseCurrent->nutrients_accumulated = 100;
    BaseCurrent->nutrient_surplus = 4;
    assert(base_growth() == 0 && BaseCurrent->population_size == 3);
    assert(BaseCurrent->nutrients_accumulated == 4 && BaseComputedID == 5);

    reset_case();
    BaseCurrentGrowthRate = 6;
    BaseCurrent->nutrients_accumulated = 100;
    BaseCurrent->nutrient_surplus = 2;
    assert(base_growth() == 0 && BaseCurrent->population_size == 3);
    assert(BaseCurrent->nutrients_accumulated == 30); // Initial population cost, not new population.

    reset_case();
    Rules.pop_limit_sans_hab_complex = 0;
    BaseCurrent->nutrients_accumulated = 100;
    BaseCurrent->nutrient_surplus = 4;
    assert(base_growth() == 0 && BaseCurrent->population_size == 2);
    assert(BaseCurrent->nutrients_accumulated == 19 && popb_calls == 1);
    assert(has_fac_calls > 0 && last_fac_base == 5); // Deliberately not owner faction 2.

    reset_case();
    BaseCurrentGrowthRate = -3;
    BaseCurrent->nutrients_accumulated = 100;
    BaseCurrent->nutrient_surplus = 4;
    assert(base_growth() == 0 && BaseCurrent->population_size == 2);
    assert(BaseCurrent->nutrients_accumulated == 30 && popb_calls == 0);

    // Starvation cancels matching nutrient convoys, not another base's unit.
    reset_case();
    BaseCurrent->nutrients_accumulated = -1;
    BaseCurrentConvoyFrom[0] = 1;
    VehCurrentCount = 2;
    VehPrototypes[0].plan = 10;
    for (int i=0; i<2; ++i) {
        Vehs[i].x=2; Vehs[i].y=2; Vehs[i].proto_id=0;
        Vehs[i].home_base_id=static_cast<int16_t>(5+i);
        Vehs[i].order=3; Vehs[i].order_auto_type=0;
    }
    MapTiles[65].bit=1; MapTiles[65].val2=2;
    assert(base_growth() == 0 && Vehs[0].order == 0 && Vehs[1].order == 3);
}

"""
UPGRADE_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>

#define __cdecl
const int MaxAbilityNum = 29;
const int RFLAG_ALIEN = 0x80;
struct Player { uint32_t rule_flags; };
struct PlayerData { int ranking; int energy_reserves; int ai_power, ai_tech; uint8_t proto_id_active[512]; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id, weapon_id, armor_id, reactor_id, carry_capacity, cost, plan; int8_t unk_1; uint8_t obsolete_factions; int8_t combat_factions, icon_offset, padding; uint16_t flags; int16_t preq_tech; };
struct RulesAbility { char *name, *description, *abbreviation; };
struct RulesChassis { char *padding[12]; uint8_t speed, triad; };
struct Strings { const char *get(int); };
struct NetDaemon { int receive() { return 0; } };
struct Console { int faction_id_; };

Player Players[8]; PlayerData PlayersData[8];
VehPrototype VehPrototypes[64]; RulesAbility Ability[MaxAbilityNum];
RulesChassis Chassis[16]; Strings StringTable; NetDaemon NetDaemonNet;
uint32_t LockEnableMask, GameWarnings; int GameState, DiffLevelCurrent;
int RankingFactionIDUnk1, RankingFactionIDUnk2, IsMultiplayerNet;
int UpgradePromptAnswered; Console ConsoleGlobal;
char StringTemp[1032]; char *ScriptPopupFile = const_cast<char *>("popup.txt");
int popp_result, popp_calls; const char *last_label, *last_picture; int last_string_id;
int do_upgrade_calls;

const char *Strings::get(int id) { last_string_id = id; return "ABBR"; }
int popp(const char *, const char *label, int, const char *picture, int) {
    ++popp_calls; last_label = label; last_picture = picture; return popp_result;
}
int upgrade_cost(int, int, int) { return 10; }
int cost_factor(int, int, int) { return 10; }
int has_abil(int, int ability) { return ability == 1; }
void say_stats_2(char *, int) {}
int parse_num(int, int) { return 0; }
int parse_says(int, const char *, int, int) { return 0; }
void do_upgrade(int, int, int) { ++do_upgrade_calls; }
int message_data(int, int, int, int, int, int) { return 0; }

YIELD_HELPERS
SAR_HELPER
BODY

static void set_popup_case(int flags, bool alien, const char *label, const char *picture) {
    GameState = 0;
    LockEnableMask = 1u << 2;
    GameWarnings = 4;
    ConsoleGlobal.faction_id_ = 2;
    Players[2].rule_flags = alien ? RFLAG_ALIEN : 0;
    PlayersData[2].energy_reserves = 50;
    PlayersData[2].proto_id_active[3] = 1;
    VehPrototypes[4].flags = static_cast<uint16_t>(flags);
    VehPrototypes[4].chassis_id = 2;
    Chassis[2].triad = 2;
    UpgradePromptAnswered = 0;
    popp_result = 0;
    assert(upgrade_prototype(2, 4, 3, 1) == 0);
    assert(std::strcmp(last_label, label) == 0);
    assert(std::strcmp(last_picture, picture) == 0);
}

int main() {
    LockEnableMask = 1u << 2;
    GameWarnings = 4;
    ConsoleGlobal.faction_id_ = 2;
    PlayersData[2].energy_reserves = 100;
    PlayersData[2].proto_id_active[3] = 1;
    VehPrototypes[4].chassis_id = 2;
    Chassis[2].triad = 2;
    UpgradePromptAnswered = 0;

    // Native gate: only (bit 1 set, bit 8 clear) returns before the popup.
    for (int state = 0; state < 16; ++state) {
        GameState = state;
        popp_result = 0;
        popp_calls = 0;
        const int before = do_upgrade_calls;
        const int result = upgrade_prototype(2, 4, 3, 1);
        assert(result == 0);
        assert(do_upgrade_calls == before);
        assert(popp_calls == (((state & 1) != 0 && (state & 8) == 0) ? 0 : 1));
    }

    set_popup_case(0, false, "UPGRADEPROTOCOST2", "air_sm.pcx");
    set_popup_case(4, false, "UPGRADEPROTOCOST", "air_sm.pcx");
    set_popup_case(0, true, "UPGRADEPROTOCOST2", "alair_sm.pcx");
    set_popup_case(4, true, "UPGRADEPROTOCOST", "alair_sm.pcx");

    // The free-energy path reaches the final prompt: bit 4 controls label,
    // the chassis triad/faction controls the image, and label/image stay apart.
    PlayersData[2].energy_reserves = 1000;
    LockEnableMask = 1u << 2;
    for (int flags : {0, 4}) {
        for (int alien = 0; alien < 2; ++alien) {
            VehPrototypes[4].flags = static_cast<uint16_t>(flags);
            Players[2].rule_flags = alien ? RFLAG_ALIEN : 0;
            UpgradePromptAnswered = 0;
            popp_result = 0;
            assert(upgrade_prototype(2, 4, 3, 1) == 0);
            assert(std::strcmp(last_label, flags == 0 ? "UPGRADEPROTO2" : "UPGRADEPROTO") == 0);
            assert(std::strcmp(last_picture, alien ? "alair_sm.pcx" : "air_sm.pcx") == 0);
        }
    }
}

"""

def helper(source, name):
    start = source.index(name + "(")
    # Include the return type/static declaration preceding the name.
    start = source.rfind("\n", 0, start) + 1
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if not depth:
                return source[start:end+1]
    raise AssertionError("unterminated helper " + name)

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("address,harness", [(0x4EF1C0, GROWTH_HARNESS), (0x4EFE80, UPGRADE_HARNESS)], ids=["growth", "upgrade"])
def test_actual_colony_growth_upgrade(compiler, address, harness, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source_path = ROOT / "src/base.cpp"
    source = source_path.read_text()
    records = [r for r in read_file(source_path) if r.address == address]
    assert len(records) == 1 and not records[0].extract_error
    assert "BODY GOES HERE" not in records[0].region
    assert "PENDING_BODY" not in records[0].region
    helpers = "\n".join(helper(source, n) for n in
                        ("base_yield_signed_bits", "base_yield_add", "base_yield_sub", "base_yield_mul"))
    control = harness.replace("YIELD_HELPERS", helpers).replace("BODY", records[0].region)
    if "SAR_HELPER" in control:
        control = control.replace("SAR_HELPER", helper(source, "upgrade_arithmetic_shift_three"))
    control_path = tmp_path / "control.cpp"
    control_path.write_text(control)
    binary = tmp_path / "control"
    compiled = subprocess.run([executable, "-std=c++14", "-O2", "-Wall", "-Wextra", "-Werror",
                               str(control_path), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)

VEH_INIT_HARNESS = r"""
// Expanded host-only controls; includes immutable veh-init-private-v2.inc.
// These local structs are test doubles, not ABI/layout evidence.
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl

typedef const char *LPCSTR;
typedef int BOOL;

enum {
    PLAN_INFO_WARFARE = 11,
    PLAN_ALIEN_ARTIFACT = 12,
    VFLAG_IS_OBJECTIVE = 0x20,
    VSTATE_UNK_20000 = 0x20000,
    VSTATE_UNK_40000 = 0x40000,
    DTREATY_VENDETTA = 0x10,
};

struct PlayerData {
    uint32_t diplo_treaties[8];
    int x_target, y_target;
    int total_mil_veh;
    uint8_t proto_id_active[512];
    int unk_70, unk_71, unk_72;
    uint8_t region_total_bases[128];
    uint8_t region_base_plan[128];
};
struct VehPrototype {
    uint8_t chassis_id, weapon_id, armor_id, plan;
};
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint8_t faction_id;
    int16_t home_base_id;
};
struct RulesChassis { uint8_t triad; };
struct Base { uint8_t faction_id_current; };
struct Map { uint8_t climate, region; };

static PlayerData PlayersData[8];
static VehPrototype VehPrototypes[512];
static Veh Vehs[2049];
static RulesChassis Chassis[19];
static Base Bases[128];
static Map MapStorage[128];
static Map *MapTiles = MapStorage;
static int VehCurrentCount;
static int GameState;
static int BaseIDCurrentSelected;
static int BaseComputedID;
static int BaseFindDist;
static uint32_t MapLongitude;
static int MapLongitudeBounds;
static int MapIsFlat;
static int base_find_result;
static int six_arg_base_find_calls;
static int clear_calls, drop_calls, spot_calls;
static int weapon_calls, armor_calls, ability_calls;
static int weapon_result, armor_result, ability_result, six_arg_distance;
static int six_arg_faction, six_arg_region;
static int random_result;

static int test_rand() { return random_result; }
#define rand test_rand

static Map *map_tiles() { return MapTiles; }
static void log_say(LPCSTR, int, int, int) {}
static void veh_clear(int vehicle_id, int prototype, int faction) {
    ++clear_calls;
    memset(&Vehs[vehicle_id], 0, sizeof(Vehs[vehicle_id]));
    Vehs[vehicle_id].proto_id = static_cast<int16_t>(prototype);
    Vehs[vehicle_id].faction_id = static_cast<uint8_t>(faction);
    Vehs[vehicle_id].home_base_id = -1;
}
static int base_find(int, int) { return base_find_result; }
static int base_find(int, int, int faction, int region, int, int) {
    ++six_arg_base_find_calls;
    six_arg_faction = faction;
    six_arg_region = region;
    BaseFindDist = six_arg_distance;
    return -1;
}
static int veh_drop(int vehicle_id, int x, int y) {
    ++drop_calls;
    Vehs[vehicle_id].x = static_cast<int16_t>(x);
    Vehs[vehicle_id].y = static_cast<int16_t>(y);
    return vehicle_id;
}
static void spot_all(int, int) { ++spot_calls; }
static int bit_count(int bits) {
    uint32_t value = static_cast<uint32_t>(bits);
    int count = 0;
    while (value) {
        count += static_cast<int>(value & 1);
        value >>= 1;
    }
    return count;
}
static int weap_strat(int, int) { ++weapon_calls; return weapon_result; }
static int arm_strat(int, int) { ++armor_calls; return armor_result; }
static BOOL has_abil(int, int) { ++ability_calls; return ability_result; }

HELPERS
BODY

static void reset_controls() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Vehs, 0, sizeof(Vehs));
    memset(Chassis, 0, sizeof(Chassis));
    memset(Bases, 0, sizeof(Bases));
    memset(MapStorage, 0, sizeof(MapStorage));
    VehCurrentCount = 0;
    GameState = 0;
    BaseIDCurrentSelected = -1;
    BaseComputedID = -1;
    BaseFindDist = 0;
    MapLongitude = 4;
    MapLongitudeBounds = 8;
    MapIsFlat = 1;
    base_find_result = -1;
    six_arg_base_find_calls = 0;
    clear_calls = drop_calls = spot_calls = 0;
    weapon_calls = armor_calls = ability_calls = 0;
    weapon_result = armor_result = ability_result = 0;
    six_arg_distance = 0;
    six_arg_faction = six_arg_region = -1;
    random_result = 0;
}

int main() {
    reset_controls();
    VehCurrentCount = 2048;
    assert(veh_init(0, 0, 0, 0) == -1);
    assert(VehCurrentCount == 2048 && clear_calls == 0 && drop_calls == 0);

    reset_controls();
    VehPrototypes[3].plan = 1;
    PlayersData[0].x_target = -1;
    base_find_result = 0;
    Bases[0].faction_id_current = 0;
    BaseIDCurrentSelected = 9;
    BaseComputedID = 0;
    assert(veh_init(3, 0, 2, 1) == 0);
    assert(VehCurrentCount == 1 && PlayersData[0].total_mil_veh == 1);
    assert(PlayersData[0].proto_id_active[3] == 1);
    assert(Vehs[0].home_base_id == 0 && BaseComputedID == -1);
    assert(BaseIDCurrentSelected == 9);
    assert(drop_calls == 1 && spot_calls == 1);

    reset_controls();
    VehPrototypes[4].plan = PLAN_ALIEN_ARTIFACT;
    PlayersData[0].x_target = -1;
    GameState = 0x40000;
    base_find_result = 0;
    Bases[0].faction_id_current = 0;
    assert(veh_init(4, 0, 2, 1) == 0);
    assert((Vehs[0].flags & VFLAG_IS_OBJECTIVE) != 0);
    assert(Vehs[0].home_base_id == -1);
    assert(PlayersData[0].total_mil_veh == 0);

    reset_controls();
    VehPrototypes[5].plan = PLAN_INFO_WARFARE;
    VehPrototypes[5].chassis_id = 2;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    Chassis[2].triad = 0;
    assert(veh_init(5, 0, 2, 1) == 0);
    assert(PlayersData[0].unk_71 == 1 && PlayersData[0].unk_72 == 2);
    assert((Vehs[0].state & VSTATE_UNK_40000) != 0);
    assert(weapon_calls == 1 && armor_calls == 1 && ability_calls == 1);

    reset_controls();
    VehPrototypes[6].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].diplo_treaties[1] = DTREATY_VENDETTA;
    PlayersData[0].region_total_bases[1] = 2;
    MapStorage[6].climate = 0x60;
    MapStorage[6].region = 1;
    MapStorage[0].region = 1;
    six_arg_distance = 0;
    assert(veh_init(6, 0, 4, 1) == 0);
    assert(six_arg_base_find_calls == 1);
    assert(weapon_calls == 0 && armor_calls == 0 && ability_calls == 0);

    reset_controls();
    VehPrototypes[7].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    PlayersData[0].region_base_plan[1] = 7;
    PlayersData[0].unk_70 = -2097152;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    Chassis[0].triad = 0;
    assert(veh_init(7, 0, 2, 1) == 0);
    assert(PlayersData[0].unk_71 == 0 && PlayersData[0].unk_72 == 0);
    assert(weapon_calls == 0 && armor_calls == 0 && ability_calls == 0);

    reset_controls();
    VehPrototypes[8].plan = PLAN_INFO_WARFARE;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    PlayersData[0].unk_71 = 1;
    PlayersData[0].unk_72 = 2;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    Chassis[0].triad = 0;
    assert(veh_init(8, 0, 2, 1) == 0);
    assert(PlayersData[0].unk_71 == 1 && PlayersData[0].unk_72 == 2);
    assert(Vehs[0].state == 0);

    reset_controls();
    VehPrototypes[9].plan = 0;
    PlayersData[0].x_target = -1;
    base_find_result = 0;
    Bases[0].faction_id_current = 1;
    assert(veh_init(9, 0, 2, 1) == 0);
    assert(Vehs[0].home_base_id == -1);

    reset_controls();
    VehPrototypes[10].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x20;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    assert(veh_init(10, 0, 2, 1) == 0);
    assert(weapon_calls == 0 && ability_calls == 0);

    reset_controls();
    VehPrototypes[11].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 2;
    assert(veh_init(11, 0, 2, 1) == 0);
    assert(weapon_calls == 0 && ability_calls == 0);

    reset_controls();
    VehPrototypes[12].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    Chassis[0].triad = 1;
    assert(veh_init(12, 0, 2, 1) == 0);
    assert(ability_calls == 0 && PlayersData[0].unk_71 == 0);

    reset_controls();
    VehPrototypes[13].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    ability_result = 1;
    assert(veh_init(13, 0, 2, 1) == 0);
    assert(ability_calls == 1 && PlayersData[0].unk_71 == 0);

    reset_controls();
    VehPrototypes[14].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    PlayersData[0].unk_72 = 1;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    weapon_result = 1;
    armor_result = 2;
    assert(veh_init(14, 0, 2, 1) == 0);
    assert(PlayersData[0].unk_71 == 0 && Vehs[0].state == 0);

    reset_controls();
    VehPrototypes[15].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    PlayersData[0].region_base_plan[1] = 7;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    weapon_result = 1;
    armor_result = 2;
    assert(veh_init(15, 0, 2, 1) == 0);
    assert(PlayersData[0].unk_71 == 0 && Vehs[0].state == 0);

    reset_controls();
    VehPrototypes[16].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    random_result = 0;
    assert(veh_init(16, 0, 2, 1) == 0);
    assert((Vehs[0].state & VSTATE_UNK_20000) != 0);
    assert(PlayersData[0].unk_71 == 0);

    reset_controls();
    VehPrototypes[17].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].region_total_bases[1] = 3;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    random_result = 1;
    assert(veh_init(17, 0, 2, 1) == 0);
    assert((Vehs[0].state & VSTATE_UNK_40000) != 0);
    assert((Vehs[0].state & VSTATE_UNK_20000) == 0);
    assert(PlayersData[0].unk_71 == 1);

    reset_controls();
    VehPrototypes[18].plan = 0;
    PlayersData[0].x_target = -1;
    PlayersData[0].total_mil_veh = INT32_MAX;
    PlayersData[0].proto_id_active[18] = 255;
    assert(veh_init(18, 0, 2, 1) == 0);
    assert(PlayersData[0].total_mil_veh == INT32_MIN);
    assert(PlayersData[0].proto_id_active[18] == 0);

    reset_controls();
    VehPrototypes[19].plan = 0;
    PlayersData[0].x_target = 0;
    PlayersData[0].y_target = 0;
    PlayersData[0].diplo_treaties[1] = DTREATY_VENDETTA;
    PlayersData[0].diplo_treaties[2] = DTREATY_VENDETTA;
    PlayersData[0].region_total_bases[1] = 2;
    PlayersData[1].region_total_bases[1] = 9;
    PlayersData[2].region_total_bases[1] = 8;
    MapStorage[5].climate = 0x60;
    MapStorage[5].region = 1;
    MapStorage[0].region = 1;
    six_arg_distance = 0;
    assert(veh_init(19, 0, 2, 1) == 0);
    assert(six_arg_base_find_calls == 1);
    assert(six_arg_faction == 1 && six_arg_region == 1);
    return 0;
}

"""

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_vehicle_creation(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    path = ROOT / "src/veh.cpp"
    source = path.read_text()
    records = [r for r in read_file(path) if r.address == 0x5C03D0]
    assert len(records) == 1 and not records[0].extract_error
    assert "BODY GOES HERE" not in records[0].region
    assert "PENDING_BODY" not in records[0].region
    helpers = "\n".join(helper(source, name) for name in
        ("veh_init_i32", "veh_init_add32", "veh_init_sub32", "veh_init_sar32", "veh_init_abs32"))
    control = VEH_INIT_HARNESS.replace("HELPERS", helpers).replace("BODY", records[0].region)
    control_path = tmp_path / "control.cpp"
    control_path.write_text(control)
    binary = tmp_path / "control"
    compiled = subprocess.run([executable, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all", str(control_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


# Production-only behavioral harness copied from private host-v4 controls.
# Source placeholders are filled from the live, mapped C++ definitions below;
# no ignored build artifact is read by the committed test.
# Only base_production, its three arithmetic helpers, and the colony-abandon
# helper are extracted from actual source. Reporting, vehicle-finish, project
# event, and other production helpers are doubles; the dependent class shapes
# are minimal harness mocks, not original ABI/layout or full-path/game proof.
PRODUCTION_HARNESS = r"""#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define __cdecl
#define TRUE 1
#define RSC_MINERALS 1
#define FAC_STOCKPILE_ENERGY 69
#define FAC_HEADQUARTERS 1
#define FAC_CENTAURI_PRESERVE 31
#define FacilityRepStart 65
#define FacilitySPStart 70
#define DLVL_SPECIALIST 1
#define DLVL_LIBRARIAN 3
#define DLVL_TALENT 2
#define MaxPlayerNum 8
#define MapArea 8
#define FACILITY_COUNT 134
#define MaxFacilityNum 134
#define PLAN_COLONIZATION 8
#define VSTATE_UNK_200 0x200u
#define VSTATE_UNK_2000 0x2000u
#define PREF_AV_SECRET_PROJECT_MOVIES 0x10000000u
#define GOV_MANAGES_PRODUCTION 0x1u
#define GOV_NEW_VEH_FULLY_AUTO 0x80u
#define GOV_ACTIVE 0x80000000u
#define RFLAG_ALIEN 0x80u
enum {
    FAC_ASCENT_TO_TRANSCENDENCE = 102,
    SP_ASCENT_TO_TRANSCENDENCE = 32,
    SP_VOICE_OF_PLANET = 31,
    BSTATE_DRONE_RIOTS_ACTIVE = 0x2,
    BSTATE_ARTIFACT_LINKED = 0x400,
    BSTATE_ARTIFACT_ALREADY_LINKED = 0x800,
    BSTATE_PRODUCTION_DONE = 0x800000,
    BIT_VEH_IN_TILE = 0x2,
    PFLAG_MAP_REVEALED = 0x200
};
enum ProductionReportResume {
    PRODUCTION_REPORT_RESUME_274F,
    PRODUCTION_REPORT_RESUME_276B,
    PRODUCTION_REPORT_RESUME_2752
};

typedef char enum_guard_rsc_minerals[(RSC_MINERALS == 1) ? 1 : -1];
typedef char enum_guard_hq[(FAC_HEADQUARTERS == 1) ? 1 : -1];
typedef char enum_guard_facility_rep[(FacilityRepStart == 65) ? 1 : -1];
typedef char enum_guard_project_start[(FacilitySPStart == 70) ? 1 : -1];
typedef char enum_guard_stockpile[(FAC_STOCKPILE_ENERGY == 69) ? 1 : -1];
typedef char enum_guard_preserve[(FAC_CENTAURI_PRESERVE == 31) ? 1 : -1];
typedef char enum_guard_specialist[(DLVL_SPECIALIST == 1) ? 1 : -1];
typedef char enum_guard_governor_active[(GOV_ACTIVE == 0x80000000u) ? 1 : -1];
typedef char enum_guard_colonization[(PLAN_COLONIZATION == 8) ? 1 : -1];
typedef char enum_guard_production_done[(BSTATE_PRODUCTION_DONE == 0x800000) ? 1 : -1];
typedef char enum_guard_drone_riot[(BSTATE_DRONE_RIOTS_ACTIVE == 0x2) ? 1 : -1];
typedef char enum_guard_vehicle_tile[(BIT_VEH_IN_TILE == 0x2) ? 1 : -1];
typedef char enum_guard_map_revealed[(PFLAG_MAP_REVEALED == 0x200) ? 1 : -1];
typedef char enum_guard_auto_governor[(GOV_NEW_VEH_FULLY_AUTO == 0x80u) ? 1 : -1];
typedef char enum_guard_colony_defer[(VSTATE_UNK_200 == 0x200u) ? 1 : -1];
typedef char enum_guard_colony_state[(VSTATE_UNK_2000 == 0x2000u) ? 1 : -1];
typedef char enum_guard_preferences[(PREF_AV_SECRET_PROJECT_MOVIES == 0x10000000u) ? 1 : -1];
typedef char enum_guard_facility_count[(FACILITY_COUNT == 134 && MaxFacilityNum == 134) ? 1 : -1];
typedef char enum_guard_manages_production[(GOV_MANAGES_PRODUCTION == 1u) ? 1 : -1];
typedef char enum_guard_ascent_facility[(FAC_ASCENT_TO_TRANSCENDENCE == 102) ? 1 : -1];
typedef char enum_guard_ascent_project[(SP_ASCENT_TO_TRANSCENDENCE == 32) ? 1 : -1];
typedef char enum_guard_voice_project[(SP_VOICE_OF_PLANET == 31) ? 1 : -1];
typedef char enum_guard_artifact_linked[(BSTATE_ARTIFACT_LINKED == 0x400) ? 1 : -1];
typedef char enum_guard_artifact_already_linked[(BSTATE_ARTIFACT_ALREADY_LINKED == 0x800) ? 1 : -1];
typedef char enum_guard_librarian[(DLVL_LIBRARIAN == 3) ? 1 : -1];
typedef char enum_guard_talent[(DLVL_TALENT == 2) ? 1 : -1];

struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    int8_t population_size;
    uint8_t pad0[5];
    uint8_t visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unk_x, unk_y;
    uint32_t state, event, governor;
    int nutrients_accumulated, minerals_accumulated, production_id_last;
    int eco_damage;
    uint32_t queue_size;
    int queue_production_id[10];
    int resource_sqr_radius, specialist_total, specialist_unk_1;
    uint32_t spec[2];
    uint8_t facilities_built[12];
    int mineral_surplus_final, minerals_accumulated_2;
    int unk_6, unk_7, unk_8, unk_9;
    int nutrient_intake_1, mineral_intake_1, energy_intake_1, unused_intake_1;
    int nutrient_intake_2, mineral_intake_2, energy_intake_2, unused_intake_2;
    int nutrient_surplus, mineral_surplus, energy_surplus, unused_surplus;
};
struct PlayerData {
    uint32_t flags;
    int diff_level;
    int unk_28;
    int pop_total;
    int planet_ecology;
    uint32_t satellites_[4];
};
struct Player { int rule_flags; int ai_power; };
struct RulesType { int retool_exemption; } Rules;
struct RuleFacility { int cost; } Facility[FACILITY_COUNT];
struct VehPrototype { char veh_name[32]; int plan; uint16_t flags; } VehPrototypes[128];
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
    uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4], waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack;
} Vehs[128];
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
} MapTiles[MapArea];
struct Console { int faction_id_; } ConsoleGlobal;
struct GraphicWin { int redraw_calls; void redraw() { ++redraw_calls; } };
struct ClimateWorldWin : GraphicWin {} WorldClimateWorldWin;
struct Popup {
    static int constructors;
    static int destructors;
    Popup() { ++constructors; }
    ~Popup() { ++destructors; }
    Popup(const Popup &);
    Popup &operator=(const Popup &);
};
int Popup::constructors;
int Popup::destructors;

Base Bases[16];
Base *BaseCurrent = &Bases[0];
PlayerData PlayersData[8];
Player Players[8];
int BaseIDCurrentSelected, BaseCurrentCount, BaseComputedID, LockEnableMask;
int GameState;
uint32_t GameInterludeFlags, GamePreferences;
int DiffLevelCurrent, BaseProductionZoomRequested;
char StringTemp[1024];
char *ClimatePopupTitle[1];
int IsMultiplayerNet, BaseProductionUnitCreationDisabled;
char ScriptPopupFileText[] = "SCRIPT";
char *ScriptPopupFile = ScriptPopupFileText;
int production_test_vehicle_cost, production_test_cost_factor;
int production_test_vehicle_id, production_test_colony_prompt;
int production_test_objective, production_test_ascending;
int production_test_project_base, production_test_reveal;
int production_test_report_result, production_test_first_queue_result;
int production_test_second_queue_result, production_test_first_queue_replacement;
int production_test_use_first_queue;
int base_reset_calls, base_queue_calls, base_support_calls, base_yield_calls;
int base_nutrient_calls, base_minerals_calls, base_energy_calls;
int veh_init_calls, production_finish_calls, production_spend_calls;
int existing_project_notice_calls, existing_facility_notice_calls;
int set_fac_calls, production_report_calls, production_monitor_calls;
int preserve_monitor_calls, reveal_calls, popb_calls, popp_calls;
int dirty_calls, draw_map_calls, eliminated_calls, base_kill_calls;
int late_reconcile_calls, project_tail_calls, project_owner_calls;
int last_project_owner_id, last_project_owner_base;
int last_report_item, last_report_project, last_report_queue;
int last_report_orbital, last_monitor_owner;
int last_vehicle_proto, last_vehicle_faction, last_vehicle_x, last_vehicle_y;
int last_interlude_id;
int parse_says_calls, parse_num_calls, parsed_num;
const char *last_popp_label, *last_popb_text, *last_popb_image, *last_popp_image;
Map MapProbe;
std::vector<int> call_order;

PRODUCTION_ARITHMETIC_HELPERS

int production_abs(int value) { return value < 0 ? -value : value; }
int production_project(int item) {
    return item <= -FacilitySPStart ? -item - FacilitySPStart : -1;
}
int cost_factor(int, int, int) { return production_test_cost_factor; }
int veh_cost(int, int, int) { return production_test_vehicle_cost; }
bool production_faction_human(int faction);
void parse_says(int, const char *, int, int) { ++parse_says_calls; }
void parse_num(int, int value) { ++parse_num_calls; parsed_num = value; }
int popp(const char *, const char *, int, const char *, int);
PRODUCTION_COLONY_PROMPT_HELPER
void prune_protos(int, int, int) { call_order.push_back(11); }
void production_refresh_base_if_needed(int) { call_order.push_back(12); }
void production_spend(Base &base, int, int cost) {
    ++production_spend_calls;
    base.minerals_accumulated = production_wrapped_subtract(
        base.minerals_accumulated, cost);
    base.mineral_surplus_final = base.minerals_accumulated;
}
int veh_init(int prototype, int faction, int x, int y) {
    ++veh_init_calls;
    last_vehicle_proto = prototype;
    last_vehicle_faction = faction;
    last_vehicle_x = x; last_vehicle_y = y;
    call_order.push_back(20);
    return production_test_vehicle_id;
}
bool production_faction_human(int faction) {
    return (LockEnableMask & (1u << faction)) != 0;
}
bool is_objective(int) { return production_test_objective != 0; }
void base_kill(int) { ++base_kill_calls; call_order.push_back(30); }
void draw_map(int) { ++draw_map_calls; call_order.push_back(31); }
void eliminate_player(int, int) { ++eliminated_calls; call_order.push_back(32); }
int production_vehicle_morale(int, int, int, bool) { return 4; }
void production_finish_vehicle(int, int, int, bool) {
    ++production_finish_calls; call_order.push_back(21);
}
bool ascending(int) { return production_test_ascending != 0; }
int base_project(int) { return production_test_project_base; }
void production_existing_project_notice(int, int, int) {
    ++existing_project_notice_calls; call_order.push_back(40);
}
bool production_facility_built(int base_id, int facility_id) {
    const int byte_offset = facility_id >> 3;
    const uint8_t mask = static_cast<uint8_t>(1u << (facility_id & 7));
    return (Bases[base_id].facilities_built[byte_offset] & mask) != 0;
}
int base_queue(int) {
    ++base_queue_calls;
    call_order.push_back(50 + base_queue_calls);
    if (production_test_use_first_queue && base_queue_calls == 1) {
        if (production_test_first_queue_replacement != 0) {
            Bases[BaseIDCurrentSelected].queue_production_id[0] =
                production_test_first_queue_replacement;
        }
        return production_test_first_queue_result;
    }
    return production_test_second_queue_result;
}
void production_existing_facility_notice(int, int) {
    ++existing_facility_notice_calls; call_order.push_back(41);
}
void bitmask(int facility_id, int *offset, int *mask) {
    *offset = facility_id >> 3;
    *mask = 1 << (facility_id & 7);
}
void set_fac(int facility_id, int base_id, int) {
    ++set_fac_calls;
    int offset, mask;
    bitmask(facility_id, &offset, &mask);
    Bases[base_id].facilities_built[offset] = static_cast<uint8_t>(
        Bases[base_id].facilities_built[offset] | mask);
    call_order.push_back(60);
}
bool production_reveal_faction(int) { ++reveal_calls; return production_test_reveal != 0; }
void production_clear_artifact_orders() { call_order.push_back(61); }
ProductionReportResume production_report_stage_v3(
    Popup &, int owner, int item, int project, int queue_result, bool orbital) {
    ++production_report_calls;
    last_monitor_owner = owner;
    last_report_item = item;
    last_report_project = project;
    last_report_queue = queue_result;
    last_report_orbital = orbital ? 1 : 0;
    call_order.push_back(70);
    return static_cast<ProductionReportResume>(production_test_report_result);
}
void mon_first_in_space(int) { ++production_monitor_calls; call_order.push_back(71); }
void mon_facility_built(int, const char *) { ++production_monitor_calls; call_order.push_back(72); }
void mon_built_preserve(int) { ++preserve_monitor_calls; call_order.push_back(73); }
void interlude(int id, char *, int, int) { last_interlude_id = id; call_order.push_back(74); }
int production_project_events_tail(int, int) { ++project_tail_calls; call_order.push_back(80); return 0; }
void production_voice_of_planet_spread() { call_order.push_back(81); }
void amovie_project(int) { call_order.push_back(82); }
void amovie_project(const char *) { call_order.push_back(83); }
void help_project(int) { call_order.push_back(84); }
void production_late_reconcile(int, int, unsigned char, int) {
    ++late_reconcile_calls; call_order.push_back(90);
}
void base_reset(int, int) { ++base_reset_calls; call_order.push_back(5); }
const Map *map_loc(int, int) { return &MapProbe; }
void popb(const char *text, int, int, const char *image, void *) {
    ++popb_calls; last_popb_text = text; last_popb_image = image;
    call_order.push_back(100);
}
int popp(const char *, const char *label, int, const char *image, int) {
    ++popp_calls; last_popp_label = label; last_popp_image = image;
    call_order.push_back(101); return 0;
}
void set_dirty() { ++dirty_calls; call_order.push_back(102); }
void base_support() { ++base_support_calls; }
void base_yield() { ++base_yield_calls; }
void base_nutrient() { ++base_nutrient_calls; }
void base_minerals() { ++base_minerals_calls; }
void base_energy() { ++base_energy_calls; }
void production_set_project_owner(int project_id, int base_id) {
    ++project_owner_calls; last_project_owner_id = project_id;
    last_project_owner_base = base_id;
}
int stack_fix(int value) { return value; }
int veh_at(int, int) { return -1; }

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"
#endif
PRODUCTION_BODY
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

static void reset_case() {
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Vehs, 0, sizeof(Vehs));
    memset(MapTiles, 0, sizeof(MapTiles));
    memset(Facility, 0, sizeof(Facility));
    memset(&MapProbe, 0, sizeof(MapProbe));
    BaseCurrent = &Bases[0]; BaseCurrentCount = 2;
    BaseIDCurrentSelected = 0; BaseComputedID = -1;
    Bases[0].faction_id_current = 1; Bases[0].queue_production_id[0] = 3;
    Bases[0].nutrients_accumulated = 10; Bases[0].minerals_accumulated = 100;
    Bases[0].population_size = 3; Bases[0].state = 0x80000000u;
    strcpy(Bases[0].name_string, "Primary"); Bases[0].x = 5; Bases[0].y = 6;
    Bases[1].faction_id_current = 1; strcpy(Bases[1].name_string, "Other");
    LockEnableMask = 1 << 1; production_test_vehicle_cost = 20;
    production_test_cost_factor = 1; production_test_vehicle_id = 0;
    production_test_colony_prompt = production_test_objective = 0;
    production_test_ascending = 1; production_test_project_base = -1;
    production_test_reveal = 0;
    production_test_report_result = PRODUCTION_REPORT_RESUME_2752;
    production_test_first_queue_result = production_test_second_queue_result = 0;
    production_test_first_queue_replacement = production_test_use_first_queue = 0;
    BaseProductionUnitCreationDisabled = 0; BaseProductionZoomRequested = 0;
    GameState = 0; DiffLevelCurrent = 0; StringTemp[0] = '\0';
    parse_says_calls = parse_num_calls = parsed_num = 0;
    GameInterludeFlags = GamePreferences = 0; IsMultiplayerNet = 0;
    Rules.retool_exemption = 999; Popup::constructors = Popup::destructors = 0;
    base_reset_calls = base_queue_calls = base_support_calls = base_yield_calls = 0;
    base_nutrient_calls = base_minerals_calls = base_energy_calls = 0;
    veh_init_calls = production_finish_calls = production_spend_calls = 0;
    existing_project_notice_calls = existing_facility_notice_calls = set_fac_calls = 0;
    production_report_calls = production_monitor_calls = preserve_monitor_calls = 0;
    reveal_calls = popb_calls = popp_calls = dirty_calls = draw_map_calls = 0;
    eliminated_calls = base_kill_calls = late_reconcile_calls = project_tail_calls = 0;
    project_owner_calls = 0; last_project_owner_id = last_project_owner_base = -1;
    last_report_item = last_report_project = last_report_queue = -1;
    last_report_orbital = last_monitor_owner = 0;
    last_vehicle_proto = last_vehicle_faction = last_vehicle_x = last_vehicle_y = -1;
    last_interlude_id = -1; last_popp_label = last_popb_text = 0;
    last_popb_image = last_popp_image = 0; call_order.clear();
}

static int order_index(int value) {
    for (size_t i = 0; i < call_order.size(); ++i) {
        if (call_order[i] == value) return static_cast<int>(i);
    }
    return -1;
}
static void check_popup_lifetime() {
    assert(Popup::constructors == 1 && Popup::destructors == 1);
}
static void test_unit_paths() {
    reset_case();
    Bases[0].queue_production_id[0] = 3;
    base_production();
    check_popup_lifetime();
    assert(veh_init_calls == 1 && production_finish_calls == 1);
    assert(last_vehicle_proto == 3 && last_vehicle_faction == 1);
    assert(last_vehicle_x == 5 && last_vehicle_y == 6);
    assert(Vehs[0].morale == 4 && (Vehs[0].state & VSTATE_UNK_2000) != 0);
    assert((Vehs[0].flags & 0x1000u) != 0);
    assert(production_spend_calls == 1 && Bases[0].minerals_accumulated == 80);
    assert(order_index(20) >= 0 && order_index(21) > order_index(20));

    reset_case();
    Bases[0].queue_production_id[0] = 3;
    Bases[0].minerals_accumulated = 1;
    base_production();
    check_popup_lifetime();
    assert(veh_init_calls == 0 && production_spend_calls == 0);

    reset_case();
    Bases[0].queue_production_id[0] = 3;
    BaseProductionUnitCreationDisabled = 1;
    base_production();
    check_popup_lifetime();
    assert(production_spend_calls == 1 && veh_init_calls == 0);
}
static void test_wrapped_overflow_helpers() {
    reset_case();
    LockEnableMask = 1 << 1;
    DiffLevelCurrent = INT_MAX;
    Bases[0].nutrients_accumulated = INT_MAX;
    assert(!production_colony_abandon_prompt(2, 3));
    assert(Bases[0].nutrients_accumulated == -4);

    reset_case();
    Bases[0].queue_production_id[0] = -32;
    Facility[32].cost = 20;
    PlayersData[1].planet_ecology = INT_MAX;
    base_production();
    check_popup_lifetime();
    assert(PlayersData[1].planet_ecology == INT_MIN);
}
static void test_stockpile_and_native_bitmap_hq() {
    reset_case();
    Bases[0].queue_production_id[0] = -FAC_STOCKPILE_ENERGY;
    Bases[0].mineral_surplus = 9;
    const int before_minerals = Bases[0].minerals_accumulated;
    base_production();
    check_popup_lifetime();
    assert(Bases[0].minerals_accumulated == before_minerals);
    assert(production_report_calls == 0 && production_spend_calls == 0);

    reset_case();
    Bases[0].queue_production_id[0] = -FAC_HEADQUARTERS;
    Facility[FAC_HEADQUARTERS].cost = 20;
    Bases[1].facilities_built[0] = static_cast<uint8_t>(1u << FAC_HEADQUARTERS);
    base_production();
    check_popup_lifetime();
    assert(Bases[1].facilities_built[0] == 0);
    assert((Bases[0].facilities_built[0]
            & static_cast<uint8_t>(1u << FAC_HEADQUARTERS)) != 0);
    assert(set_fac_calls == 1 && production_report_calls == 1);
}
static void test_orbital_project_and_duplicate() {
    reset_case();
    Bases[0].queue_production_id[0] = -65;
    Facility[65].cost = 20;
    base_production();
    check_popup_lifetime();
    assert(PlayersData[1].satellites_[0] == 1);
    assert(set_fac_calls == 0 && production_report_calls == 1);
    assert(last_report_orbital == 1 && production_monitor_calls == 1);

    reset_case();
    Bases[0].queue_production_id[0] = -72;
    Facility[72].cost = 20;
    base_production();
    check_popup_lifetime();
    assert(project_owner_calls == 1 && last_project_owner_id == 2);
    assert(last_project_owner_base == 0);
    assert(last_report_project == 2 && last_report_item == 72);
    assert(project_tail_calls == 1 && late_reconcile_calls == 1);
    assert(order_index(70) < order_index(80) && order_index(80) < order_index(90));

    reset_case();
    Bases[0].queue_production_id[0] = -72;
    Facility[72].cost = 20;
    production_test_project_base = 4;
    base_production();
    check_popup_lifetime();
    assert(existing_project_notice_calls == 1);
    assert(production_spend_calls == 0 && production_report_calls == 0);
}
static void test_queue_redispatch() {
    reset_case();
    Bases[0].queue_production_id[0] = -42;
    Bases[0].facilities_built[42 >> 3] = static_cast<uint8_t>(1u << (42 & 7));
    Facility[42].cost = 20;
    Facility[43].cost = 25;
    production_test_use_first_queue = 1;
    production_test_first_queue_result = 1;
    production_test_first_queue_replacement = -43;
    production_test_second_queue_result = 2;
    base_production();
    check_popup_lifetime();
    assert(base_queue_calls == 2);
    assert((Bases[0].facilities_built[43 >> 3]
            & static_cast<uint8_t>(1u << (43 & 7))) != 0);
    assert(last_report_item == 43 && last_report_queue == 2);
    assert(existing_facility_notice_calls == 0);
}
int main() {
    test_wrapped_overflow_helpers();
    test_unit_paths();
    test_stockpile_and_native_bitmap_hq();
    test_orbital_project_and_duplicate();
    test_queue_redispatch();
    puts("actual base_production body corrected-mock controls: PASS");
    return 0;
}
"""


def _assert_production_harness_constants():
    headers = {
        "alpha.h": ("PREF_AV_SECRET_PROJECT_MOVIES", "0x10000000"),
        "base.h": (
            ("FAC_HEADQUARTERS", "1"),
            ("FAC_CENTAURI_PRESERVE", "31"),
            ("FAC_STOCKPILE_ENERGY", "69"),
            ("SP_VOICE_OF_PLANET", "31"),
            ("SP_ASCENT_TO_TRANSCENDENCE", "32"),
            ("BSTATE_DRONE_RIOTS_ACTIVE", "0x2"),
            ("BSTATE_ARTIFACT_LINKED", "0x400"),
            ("BSTATE_ARTIFACT_ALREADY_LINKED", "0x800"),
            ("BSTATE_PRODUCTION_DONE", "0x800000"),
            ("GOV_MANAGES_PRODUCTION", "0x1"),
            ("GOV_NEW_VEH_FULLY_AUTO", "0x80"),
            ("GOV_ACTIVE", "0x80000000"),
            ("MaxFacilityNum", "134"),
            ("FacilitySPStart", "70"),
            ("FacilityRepStart", "65"),
        ),
        "faction.h": (
            ("RFLAG_ALIEN", "0x80"),
            ("PFLAG_MAP_REVEALED", "0x200"),
            ("MaxPlayerNum", "8"),
        ),
        "game.h": (
            ("DLVL_SPECIALIST", "1"),
            ("DLVL_TALENT", "2"),
            ("DLVL_LIBRARIAN", "3"),
        ),
        "map.h": (("RSC_MINERALS", "1"), ("BIT_VEH_IN_TILE", "0x2")),
        "veh.h": (
            ("PLAN_COLONIZATION", "8"),
            ("VSTATE_UNK_200", "0x200"),
            ("VSTATE_UNK_2000", "0x2000"),
            ("MaxAbilityNum", "29"),
        ),
    }
    for filename, entries in headers.items():
        if isinstance(entries[0], str):
            entries = (entries,)
        text = (ROOT / "src" / filename).read_text()
        for name, value in entries:
            assert re.search(r"\b" + re.escape(name) + r"\s*=\s*"
                             + re.escape(value) + r"\s*[,;]", text), (
                "host-v4 constant diverges from src/" + filename + ": " + name)
    # FACILITY_COUNT is only a mock array-size alias; the native declaration is
    # MaxFacilityNum and the C++ scaffold retains a compile-time equality guard.
    assert "#define FACILITY_COUNT 134" in PRODUCTION_HARNESS
    assert "enum_guard_facility_count[(FACILITY_COUNT == 134 && MaxFacilityNum == 134)" in PRODUCTION_HARNESS


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_source_complete_base_production(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    _assert_production_harness_constants()

    source_path = ROOT / "src/base.cpp"
    source = source_path.read_text()
    records = [record for record in read_file(source_path)
               if record.address == 0x4F07E0]
    assert len(records) == 1 and not records[0].extract_error
    body = records[0].region
    assert "void __cdecl base_production()" in body
    assert "BODY GOES HERE" not in body and "PENDING_BODY" not in body

    arithmetic = "\n".join(helper(source, name) for name in (
        "production_wrapped_subtract",
        "production_wrapped_add",
        "production_wrapped_multiply",
    ))
    colony_prompt = helper(source, "production_colony_abandon_prompt")
    control = PRODUCTION_HARNESS.replace("PRODUCTION_ARITHMETIC_HELPERS", arithmetic)
    control = control.replace("PRODUCTION_COLONY_PROMPT_HELPER", colony_prompt)
    control = control.replace("PRODUCTION_BODY", body)
    for marker in ("PRODUCTION_ARITHMETIC_HELPERS",
                   "PRODUCTION_COLONY_PROMPT_HELPER", "PRODUCTION_BODY"):
        assert marker not in control

    control_path = tmp_path / "base_production_actual.cpp"
    control_path.write_text(control)
    binary = tmp_path / "base_production_actual"
    compiled = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all",
         str(control_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    assert ran.returncode == 0, ran.stdout + ran.stderr
    assert "actual base_production body corrected-mock controls: PASS" in ran.stdout

ECOLOGY_HARNESS = r"""
#include <cassert>
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#define __cdecl
#ifndef nullptr
#define nullptr 0
#endif
enum { BIT_FUNGUS=0x20, BIT_BASE_IN_TILE=1, BIT_VEH_IN_TILE=2,
       PFLAG_MAP_REVEALED=0x200, ALT_OCEAN_SHELF=2, ALT_BIT_OCEAN_SHELF=0x40,
       SP_VOICE_OF_PLANET=31, SP_Unbuilt=-1 };
struct SocialEffect { int planet; };
struct PlayerData {
    uint32_t flags;
    int ranking;
    int diff_level;
    uint32_t diplo_treaties[8];
    SocialEffect soc_effect_pending;
    int unk_49;
    int pop_total;
    int planet_ecology;
    int region_total_bases[128];
};
struct Base {
    int x, y;
    uint8_t faction_id_current, faction_id_former, population_size;
    uint8_t padding[3];
    uint32_t bit_state;
    uint32_t event;
    uint32_t governor;
    int nutrient1, mineral1, production_last, eco_damage;
    uint32_t queue_size;
    int queue[10];
    int resource_sqr_radius, specialist_total, specialist_extra, spec[2];
    uint8_t facilities[12];
    int tail[23];
    char name_string[25];
};
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk; int8_t territory; uint32_t bit, bit2; uint32_t visible[7]; };
struct Player { int is_leader_female; char filename[24], search_key[24], name_leader[24], title_leader[24]; char adj_leader[128], adj_insult_leader[128], adj_faction[128], adj_insult_faction[128]; uint8_t pad[128]; char noun_faction[24]; int noun_gender, is_noun_plural; char adj_name_faction[128], formal_name_faction[40]; char insult_leader[24], desc_name_faction[24], assistant_name[24], scientist_name[24], assistant_city[24]; uint8_t pad2[176]; int rule_flags; };
struct RulesTerraforming { const char *name; const char *name_sea; int preq_tech, preq_tech_sea; uint32_t bit, incompatible; int rate; const char *shortcuts; };
struct WorldRules { uint32_t sea_level_rises; };
struct BasicRules { uint32_t freq_global_warming_num, freq_global_warming_denom; };
struct Console { int faction_id_; int focus(int,int,int); };
struct Window { int set_center(int,int,int); };
Base Bases[16], *BaseCurrent;
Player Players[8]; PlayerData PlayersData[8]; Map Tiles[128];
RulesTerraforming Terraforming[20]; WorldRules WorldBuilder; BasicRules Rules;
Console ConsoleGlobal; Window MainMap; Window *MapWinTable[8];
uint32_t GameState, GameRules, LockEnableMask, MapTileFlagClearMask;
int BaseIDCurrentSelected, BaseCurrentCount, BaseFindDist, MapLongitudeBounds, MapLatitudeBounds, MapSeaLevel, TurnCurrentNum, DiffLevelCurrent, IsMultiplayerNet;
int MapSeaLevelCouncil;
int MapWinSelectedSlot=0;
uint32_t MapLongitude; int MapIsFlat, ExpansionEnabled;
int EcologyEventAccumulator, ClimateRiseStep, ClimateRiseLimit, ClimateRiseAccum, ClimateSeaLevelDelta;
int GenderDefault, PluralityDefault; int ParseNum[2];
char climate_title[]="CLIMATE"; char *ClimatePopupTitle[1]={climate_title};
int VehCurrentCount;
struct Vehicle { uint8_t faction_id; uint8_t pad[25]; uint8_t order_auto_type; } Vehs[32];
int radius_x[289], radius_y[289];
const int *RadiusOffsetX = radius_x, *RadiusOffsetY = radius_y;
int RiverStepX[4] = {1, 1, -1, -1}, RiverStepY[4] = {-1, 1, 1, -1};
std::vector<std::string> calls;
std::vector<int> vehicle_kinds;
std::vector<int> interlude_ids;
std::vector<int> sync_viewers;
int random_value = 1;
int focus_ecology_mutation = -1;
int focus_viewer_mutation = -1;
int interlude_8_ecology_mutation = -1;
int report_ecology_mutation = -1;
int spore_vehicle_ecology_mutation = -1;
int rand_calls = 0;
int rand_calls_at_report = -1;
int test_rand() { calls.push_back("rand"); ++rand_calls; return random_value; }
#define rand test_rand
Map *map_loc(int x, int y) { return &Tiles[y * MapLongitude + (x >> 1)]; }
int bit_count(int value) { return __builtin_popcount(static_cast<unsigned>(value)); }
int vector_dist(int x, int y) { const int hi=x>y?x:y, lo=x<y?x:y; return hi-((hi+lo)/2-lo+1)/2; }
void bitmask(int value, int *byte, int *bit) { *byte=value/8; *bit=1<<(value%8); }
void bit_set(int x,int y,int bit,int set) { Map *t=map_loc(x,y); if(set)t->bit|=bit;else t->bit&=~static_cast<uint32_t>(bit); calls.push_back("bit_set"); }
void world_alt_set(int x,int y,int,int) { (void)x;(void)y; calls.push_back("alt"); }
void synch_bit(int,int,int viewer) { calls.push_back("sync"); sync_viewers.push_back(viewer); }
void draw_tile(int,int,int) { calls.push_back("draw_tile"); }
void draw_map(int) { calls.push_back("draw_map"); }
void world_climate() { calls.push_back("world_climate"); }
void world_volcano(int,int,int) { calls.push_back("world_volcano"); }
void interlude(int event, char *, int, int) {
    calls.push_back("interlude"); interlude_ids.push_back(event);
    if(event==8 && interlude_8_ecology_mutation>=0) {
        PlayersData[BaseCurrent->faction_id_current].planet_ecology=interlude_8_ecology_mutation;
        interlude_8_ecology_mutation=-1;
    }
}
int boom(int,int,int) { calls.push_back("boom"); return 0; }
void clock_wait(int n) { calls.push_back(std::string("wait:")+char('0'+(n==20?2:1))); }
int parse_say(int,int,int,int) { calls.push_back("parse_say"); return 0; }
int parse_says(int,char *,int,int) { calls.push_back("parse_says"); return 0; }
int popp(char *,const char *,int,const char *,int(__cdecl *)()) { calls.push_back("popp"); return 0; }
int POP2(const char *,const char *,int) {
    calls.push_back("POP2");
    if(report_ecology_mutation>=0) {
        PlayersData[BaseCurrent->faction_id_current].planet_ecology=report_ecology_mutation;
        rand_calls_at_report=rand_calls;
        report_ecology_mutation=-1;
    }
    return 0;
}
int parse_num(int slot,int value) { ParseNum[slot]=value; calls.push_back("parse_num"); return value; }
int veh_init(int kind,int,int,int) {
    calls.push_back(std::string("veh:")+char('0'+kind%10)); vehicle_kinds.push_back(kind);
    if(kind!=19 && spore_vehicle_ecology_mutation>=0) {
        PlayersData[BaseCurrent->faction_id_current].planet_ecology=spore_vehicle_ecology_mutation;
        spore_vehicle_ecology_mutation=-1;
    }
    return VehCurrentCount++;
}
int GetAsyncKeyState(int) { calls.push_back("shift"); return 0; }
int base_project(int project) { assert(project==31); calls.push_back("project"); return -1; }
int Console::focus(int,int,int faction) {
    calls.push_back("focus");
    if(focus_ecology_mutation>=0) {
        PlayersData[faction].planet_ecology=focus_ecology_mutation;
        focus_ecology_mutation=-1;
    }
    if(focus_viewer_mutation>=0) {
        ConsoleGlobal.faction_id_=focus_viewer_mutation;
        focus_viewer_mutation=-1;
    }
    return 0;
}
int Window::set_center(int,int,int) { calls.push_back("center"); return 0; }
int is_human(uint32_t faction) { return (LockEnableMask & (1u<<faction)) != 0; }
int is_alive(uint32_t faction) { return (LockEnableMask & (1u<<(8+faction))) != 0; }

HELPERS
BODY

static void reset_case() {
    std::memset(Bases,0,sizeof(Bases)); std::memset(Players,0,sizeof(Players));
    std::memset(PlayersData,0,sizeof(PlayersData)); std::memset(Tiles,0,sizeof(Tiles));
    std::memset(radius_x,0,sizeof(radius_x)); std::memset(radius_y,0,sizeof(radius_y));
    std::memset(&WorldBuilder,0,sizeof(WorldBuilder)); std::memset(&Rules,0,sizeof(Rules));
    BaseCurrent=&Bases[0]; BaseCurrent->faction_id_current=1; BaseCurrent->x=2; BaseCurrent->y=2;
    BaseCurrent->eco_damage=120; BaseCurrent->name_string[0]='B'; BaseCurrent->name_string[1]=0;
    BaseIDCurrentSelected=0; BaseCurrentCount=1; BaseFindDist=0; MapLongitudeBounds=8; MapLatitudeBounds=8; MapLongitude=4; MapIsFlat=1;
    GameState=0; GameRules=0; LockEnableMask=1u<<1; TurnCurrentNum=5; DiffLevelCurrent=0; IsMultiplayerNet=0; ExpansionEnabled=0;
    MapTileFlagClearMask=4; MapSeaLevel=0; EcologyEventAccumulator=0; ClimateRiseStep=0; ClimateRiseLimit=0; ClimateRiseAccum=0; ClimateSeaLevelDelta=0;
    PlayersData[1].ranking=1; PlayersData[1].region_total_bases[0]=1; ConsoleGlobal.faction_id_=1;
    Rules.freq_global_warming_num=1; Rules.freq_global_warming_denom=1; WorldBuilder.sea_level_rises=1; MapSeaLevelCouncil=0;
    Terraforming[5].bit=4; Terraforming[5].name="ROAD"; MapWinTable[0]=&MainMap;
    VehCurrentCount=0; random_value=1; calls.clear(); vehicle_kinds.clear(); interlude_ids.clear(); sync_viewers.clear();
    focus_ecology_mutation=-1; focus_viewer_mutation=-1; interlude_8_ecology_mutation=-1;
    report_ecology_mutation=-1; spore_vehicle_ecology_mutation=-1;
    rand_calls=0; rand_calls_at_report=-1;
}

static void setup_visible_event(int initial_ecology) {
    reset_case();
    PlayersData[1].planet_ecology=initial_ecology;
    Map *center=map_loc(2,2); center->climate=0x80; center->bit=4; center->visibility=2;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
}

int main() {
    reset_case();
    // The selected tile is clear, adjacent fungus makes the native candidate
    // weight nonzero, and a worked improvement tests the owned Terraforming
    // table/clear-mask path.
    Map *center=map_loc(2,2); center->climate=0x80; center->bit=4; center->visibility=2;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    base_ecology();
    fprintf(stderr,"unk=%d count=%d calls=%u bit=%u\n",PlayersData[1].unk_49,PlayersData[1].planet_ecology,(unsigned)calls.size(),(unsigned)center->bit);
    assert(PlayersData[1].unk_49==120);
    assert(PlayersData[1].planet_ecology==1);
    assert((center->bit & BIT_FUNGUS)!=0 && (center->bit & 4)==0);
    assert(calls.front()=="rand");
    assert(std::find(calls.begin(),calls.end(),"sync")!=calls.end());
    assert(std::find(calls.begin(),calls.end(),"draw_tile")!=calls.end());

    reset_case(); BaseCurrent->eco_damage=0;
    base_ecology();
    assert(PlayersData[1].unk_49==0 && PlayersData[1].planet_ecology==0);
    assert(calls.size()==1 && calls[0]=="rand");

    reset_case(); GameRules=0x200000;
    base_ecology();
    assert(PlayersData[1].unk_49==120 && PlayersData[1].planet_ecology==0);
    assert(calls.size()==1 && calls[0]=="rand");

    reset_case();
    PlayersData[1].planet_ecology=2;
    PlayersData[1].diff_level=1;
    PlayersData[1].ranking=100;
    Map *threshold_center=map_loc(2,2);
    threshold_center->climate=0x40; threshold_center->bit=4; threshold_center->visibility=2;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    base_ecology();
    const int before_threshold_rand_count=static_cast<int>(
        std::count(calls.begin(),calls.end(),"rand"));
    assert(PlayersData[1].planet_ecology==3);

    reset_case();
    PlayersData[1].planet_ecology=3;
    PlayersData[1].diff_level=1;
    PlayersData[1].ranking=100;
    threshold_center=map_loc(2,2);
    threshold_center->climate=0x40; threshold_center->bit=4; threshold_center->visibility=2;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    base_ecology();
    assert(PlayersData[1].planet_ecology==4);
    assert(static_cast<int>(std::count(calls.begin(),calls.end(),"rand"))
        == before_threshold_rand_count+1);

    reset_case();
    BaseCurrent->population_size=8;
    PlayersData[1].planet_ecology=17;
    PlayersData[1].pop_total=30;
    radius_x[1]=1; radius_y[1]=0;
    Map *event_tile=map_loc(3,2);
    event_tile->climate=0x80; event_tile->visibility=2; event_tile->region=0;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    map_loc(4,1)->climate=0x80; map_loc(4,1)->bit=BIT_FUNGUS;
    Map *sea_tile=map_loc(4,2);
    sea_tile->climate=0x80; sea_tile->bit=BIT_FUNGUS; sea_tile->region=0x40;
    PlayersData[1].region_total_bases[0]=1;
    base_ecology();
    assert(PlayersData[1].planet_ecology==18);
    assert(vehicle_kinds.size()>=2);
    assert(vehicle_kinds[0]==8); // a land-region spore precedes the sea-region pick
    assert(vehicle_kinds[1]==10); // triad uses non-worm gate count, still zero here

    // focus() is an intervening callback before the native ecology < 2 test.
    // Mutating 1 -> 2 must suppress interlude(3) and enable the later >=2 path.
    setup_visible_event(0);
    focus_ecology_mutation=2;
    base_ecology();
    assert(std::find(interlude_ids.begin(),interlude_ids.end(),3)==interlude_ids.end());
    assert(std::find(interlude_ids.begin(),interlude_ids.end(),5)!=interlude_ids.end());

    // Native reloads the viewer after focus(). A callback that switches 1 -> 2
    // must pass viewer 2 to synch_bit, not the entry viewer cached before focus.
    setup_visible_event(0);
    focus_viewer_mutation=2;
    base_ecology();
    assert(!sync_viewers.empty() && sync_viewers.front()==2);

    // The report popup can change planet_ecology after the initial increment;
    // the subsequent native seven-event admission rereads that live value.
    setup_visible_event(8);
    report_ecology_mutation=6;
    base_ecology();
    assert(rand_calls_at_report>=0 && PlayersData[1].planet_ecology==6);
    assert(std::find(interlude_ids.begin(),interlude_ids.end(),8)==interlude_ids.end());
    assert(std::find(calls.begin(),calls.end(),"project")==calls.end());

    // The native <7 admission occurs before interlude(8), then the repeated
    // spawn-loop head observes the callback's 7 -> 9 mutation.
    setup_visible_event(6);
    PlayersData[1].pop_total=100;
    PlayersData[1].region_total_bases[0]=1;
    interlude_8_ecology_mutation=9;
    base_ecology();
    assert(std::find(interlude_ids.begin(),interlude_ids.end(),8)!=interlude_ids.end());
    assert(PlayersData[1].planet_ecology==9);
    assert(vehicle_kinds.size()==9);

    // Spawn callbacks can lower the live event count.  The outer spawn cursor
    // and each quota comparison must use the updated PlayerData member.
    reset_case();
    BaseCurrent->population_size=8;
    PlayersData[1].planet_ecology=9;
    PlayersData[1].pop_total=30;
    radius_x[1]=1; radius_y[1]=0;
    Map *spawn_event=map_loc(3,2);
    spawn_event->climate=0x80; spawn_event->visibility=2; spawn_event->region=0;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    map_loc(4,1)->climate=0x80; map_loc(4,1)->bit=BIT_FUNGUS;
    Map *spawn_sea=map_loc(4,2);
    spawn_sea->climate=0x80; spawn_sea->bit=BIT_FUNGUS; spawn_sea->region=0x40;
    PlayersData[1].region_total_bases[0]=0;
    spore_vehicle_ecology_mutation=3;
    base_ecology();
    assert(PlayersData[1].planet_ecology==3);
    assert(vehicle_kinds.size()==1);

    // Native `SHL pop_total,1` wraps the 32-bit value before signed division.
    reset_case();
    BaseCurrent->population_size=0;
    PlayersData[1].planet_ecology=9;
    PlayersData[1].pop_total=0x40000000;
    radius_x[1]=1; radius_y[1]=0;
    spawn_event=map_loc(3,2);
    spawn_event->climate=0x80; spawn_event->visibility=2; spawn_event->region=0;
    map_loc(3,1)->climate=0x80; map_loc(3,1)->bit=BIT_FUNGUS;
    PlayersData[1].region_total_bases[0]=0;
    base_ecology();
    assert(vehicle_kinds.empty());

    // A report callback lowering ecology 10 -> 9 must block the final volcano
    // roll; after the report only the ordinary pre-spawn random roll is consumed.
    setup_visible_event(9);
    PlayersData[1].region_total_bases[0]=0;
    PlayersData[1].pop_total=0;
    BaseCurrent->population_size=0;
    Rules.freq_global_warming_num=0;
    report_ecology_mutation=9;
    base_ecology();
    assert(rand_calls_at_report>=0 && PlayersData[1].planet_ecology==9);
    assert(rand_calls-rand_calls_at_report==1);

    // Rules counters are UINT32 storage, but native IMUL/IDIV uses signed
    // 32-bit bit patterns. A high-bit product must therefore compare negative.
    setup_visible_event(12);
    PlayersData[1].pop_total=0; PlayersData[1].region_total_bases[0]=0;
    BaseCurrent->population_size=0;
    Rules.freq_global_warming_num=0x80000000u;
    Rules.freq_global_warming_denom=1u;
    EcologyEventAccumulator=2;
    base_ecology();
    assert(EcologyEventAccumulator==3);
    assert(MapSeaLevelCouncil==0 && ClimateRiseLimit==0);

    // A negative signed denominator and negative product yield +10 natively;
    // unsigned C++ division would instead produce zero and miss warming.
    setup_visible_event(12);
    PlayersData[1].pop_total=0; PlayersData[1].region_total_bases[0]=0;
    BaseCurrent->population_size=0;
    Rules.freq_global_warming_num=0xFFFFFFFFu;
    Rules.freq_global_warming_denom=0xFFFFFFFEu;
    EcologyEventAccumulator=19;
    base_ecology();
    assert(EcologyEventAccumulator==0);
    assert(ClimateRiseLimit==20 && MapSeaLevelCouncil==1);

    // The native accumulator increment wraps at 32 bits rather than relying
    // on C++ signed-overflow behavior.
    setup_visible_event(12);
    PlayersData[1].pop_total=0; PlayersData[1].region_total_bases[0]=0;
    BaseCurrent->population_size=0;
    Rules.freq_global_warming_num=0u;
    Rules.freq_global_warming_denom=1u;
    EcologyEventAccumulator=0x7FFFFFFF;
    base_ecology();
    assert(EcologyEventAccumulator==static_cast<int32_t>(0x80000000u));
    assert(ClimateRiseLimit==0);

    // Council, raw rise multiplication, delta addition, wrapped absolute
    // value, and the later x1000 scaling all exercise native 32-bit arithmetic.
    setup_visible_event(23);
    PlayersData[1].pop_total=0; PlayersData[1].region_total_bases[0]=0;
    BaseCurrent->population_size=0;
    Rules.freq_global_warming_num=6u;
    Rules.freq_global_warming_denom=1u;
    WorldBuilder.sea_level_rises=0x7FFFFFFFu;
    EcologyEventAccumulator=0;
    MapSeaLevelCouncil=0x7FFFFFFE;
    ClimateSeaLevelDelta=2000000000;
    base_ecology();
    assert(EcologyEventAccumulator==0);
    assert(MapSeaLevelCouncil==static_cast<int32_t>(0x80000001u));
    assert(ClimateSeaLevelDelta==-1579139415);
    assert(ClimateRiseStep==1579139415);
    assert(ClimateRiseLimit==20 && ClimateRiseAccum==0);
    assert(ParseNum[0]==20 && ParseNum[1]==-93903328);
}

"""

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_colony_ecology(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    path = ROOT / "src/base.cpp"
    source = path.read_text()
    records = [r for r in read_file(path) if r.address == 0x4F67F0]
    assert len(records) == 1 and not records[0].extract_error
    assert "BODY GOES HERE" not in records[0].region
    assert "PENDING_BODY" not in records[0].region
    helpers = "\n".join(helper(source, name) for name in ('ecology_wrap_int', 'ecology_wrapped_add', 'ecology_wrapped_shift_two', 'ecology_wrapped_double', 'ecology_wrapped_multiply', 'ecology_native_abs', 'ecology_wrapped_neg_sub'))
    control = ECOLOGY_HARNESS.replace("HELPERS", helpers).replace("BODY", records[0].region)
    control_path = tmp_path / "control.cpp"
    control_path.write_text(control)
    binary = tmp_path / "control"
    compiled = subprocess.run([executable, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all", str(control_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)
