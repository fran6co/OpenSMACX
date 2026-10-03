"""Actual-source colony support/research controls; not game runtime proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]

SUPPORT = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifndef __cdecl
#define __cdecl
#endif

enum { MaxPlayerNum = 8 };
enum { ORDER_NONE = 0, ORDER_CONVOY = 3 };
enum { ORDERA_TERRA_AUTO_ROAD = 1, ORDERA_BOMBING_RUN = 10, ORDERA_ON_ALERT = 11 };
enum { PLAN_OFFENSIVE = 0, PLAN_COMBAT = 1, PLAN_DEFENSIVE = 2,
       PLAN_COLONIZATION = 8, PLAN_SUPPLY_CONVOY = 10 };
enum { VSTATE_REQUIRES_SUPPORT = 0x10, VSTATE_UNK_200 = 0x200,
       VSTATE_PACIFISM_DRONE = 0x200000 };
enum { ABL_CLEAN_REACTOR = 0x10000, BIT_BASE_IN_TILE = 1 };
enum { GOV_MANAGES_CITIZENS_SPECS = 0x40, GOV_ACTIVE = 0x80000000u,
       BSTATE_DRONE_RIOTS_ACTIVE = 2 };

struct Base {
    int16_t x, y;
    uint8_t faction_id_current;
    uint32_t state, governor;
    int mineral_intake_2, nutrient_surplus;
    int drone_total, talent_total;
    uint8_t facilities_built[12];
    int queue_production_id[10];
};
struct Map { uint32_t bit; uint8_t val2; };
struct Veh {
    int16_t x, y;
    uint32_t state;
    int16_t proto_id;
    uint8_t faction_id;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    uint8_t order_auto_type;
    int16_t home_base_id;
};
struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan;
};
struct SocialEffect { int support, morale; };
struct PlayerData { SocialEffect soc_effect_pending; int ai_fight; };

Base *BaseCurrent;
Base Bases[MaxPlayerNum];
Veh Vehs[32];
VehPrototype VehPrototypes[32];
PlayerData PlayersData[MaxPlayerNum];
uint32_t LockEnableMask;
uint32_t MapIsFlat = 1;
uint32_t MapLongitude = 32;
int MapLongitudeBounds = 64;
int VehCurrentCount;
int BaseIDCurrentSelected;
uint32_t BaseCurrentForcesMaintCost;
int killed_vehicle = -1;
int pop_calls;
const char *last_popup_label;
const char *last_popup_picture;
Map tile;

Map *__cdecl map_loc(int, int) { return &tile; }
void __cdecl bitmask(int id, int *offset, int *mask) {
    *offset = id / 8;
    *mask = 1 << (id % 8);
}
int __cdecl has_abil(int proto_id, int ability) {
    return proto_id == 7 && ability == ABL_CLEAN_REACTOR;
}
int __cdecl parse_says(int, char *, int, int) { return 0; }
int __cdecl popb(const char *label, int, int, const char *picture, void *) {
    ++pop_calls;
    last_popup_label = label;
    last_popup_picture = picture;
    return 0;
}
void __cdecl kill(int vehicle_id) { killed_vehicle = vehicle_id; }

HELPERS
BODY

int main() {
    Base base = {};
    BaseCurrent = &base;
    BaseIDCurrentSelected = 3;
    base.faction_id_current = 2;
    base.x = 10;
    base.y = 10;
    base.mineral_intake_2 = 0;
    BaseCurrentForcesMaintCost = 1;
    VehCurrentCount = 2;
    Vehs[0] = {};
    Vehs[0].faction_id = 2;
    Vehs[0].home_base_id = 3;
    Vehs[0].proto_id = 0;
    Vehs[0].state = VSTATE_REQUIRES_SUPPORT;
    Vehs[0].x = 12;
    Vehs[1] = Vehs[0];
    Vehs[1].x = 30;
    VehPrototypes[0].plan = PLAN_OFFENSIVE;
    tile.bit = 0;
    killed_vehicle = -1;
    base_check_support();
    assert(killed_vehicle == 1);

    // Longitude wrap may go negative, after which native first-pass code takes
    // abs again; that makes the wrapped farther unit win this discrimination.
    base.x = 0;
    MapIsFlat = 0;
    MapLongitude = 5;
    MapLongitudeBounds = 10;
    Vehs[0].x = 30;
    Vehs[1] = Vehs[0];
    Vehs[1].x = 5;
    BaseCurrentForcesMaintCost = 1;
    killed_vehicle = -1;
    base_check_support();
    assert(killed_vehicle == 0);

    // Colonization support is released, while the clean-reactor unit survives.
    MapIsFlat = 1;
    MapLongitude = 32;
    MapLongitudeBounds = 64;
    VehCurrentCount = 1;
    Vehs[0].proto_id = 7;
    Vehs[0].state = VSTATE_REQUIRES_SUPPORT;
    Vehs[0].x = 12;
    VehPrototypes[7].plan = PLAN_COLONIZATION;
    BaseCurrentForcesMaintCost = 1;
    killed_vehicle = -1;
    base_check_support();
    assert((Vehs[0].state & VSTATE_REQUIRES_SUPPORT) == 0);
    assert(killed_vehicle == -1);

    // Reset support state after that call cleared the former's support marker.
    LockEnableMask = 1u << 2;
    Vehs[0].proto_id = 0;
    Vehs[0].state = VSTATE_REQUIRES_SUPPORT;
    VehPrototypes[0].plan = PLAN_OFFENSIVE;
    BaseCurrentForcesMaintCost = 1;
    pop_calls = 0;
    base_check_support();
    assert(pop_calls == 1);
    assert(std::strcmp(last_popup_label, "NOSUPPORT") == 0);
    assert(std::strcmp(last_popup_picture, "genwarning_sm.pcx") == 0);

    // Base 3 (selected) differs from faction 2; FAC_BROOD_PIT is bit 3 of
    // byte 4. When absent, scoring distinguishes VehPrototype.cost at 0x29
    // from carry_capacity at 0x28.
    LockEnableMask = 0;
    BaseCurrentForcesMaintCost = 0;
    base.x = 10;
    base.y = 10;
    base.state = BSTATE_DRONE_RIOTS_ACTIVE;
    base.drone_total = 1;
    base.talent_total = 0;
    PlayersData[2].soc_effect_pending.morale = -3;
    PlayersData[2].ai_fight = -1;
    VehCurrentCount = 1;
    Vehs[0].faction_id = 2;
    Vehs[0].home_base_id = 3;
    Vehs[0].proto_id = 0;
    Vehs[0].state = VSTATE_PACIFISM_DRONE;
    Vehs[0].x = 30;
    VehPrototypes[0].cost = 1;
    VehPrototypes[0].carry_capacity = 100;
    VehPrototypes[0].plan = PLAN_DEFENSIVE;
    Bases[3].facilities_built[4] = 0x08;
    killed_vehicle = -1;
    base_check_support();
    assert(killed_vehicle == -1);
    assert((base.state & BSTATE_DRONE_RIOTS_ACTIVE) != 0);

    Bases[3].facilities_built[4] = 0;
    VehCurrentCount = 2;
    Vehs[0].x = 12;
    Vehs[1] = Vehs[0];
    Vehs[1].x = 30;
    Vehs[1].proto_id = 1;
    VehPrototypes[1] = VehPrototypes[0];
    VehPrototypes[1].cost = 50;
    VehPrototypes[1].carry_capacity = 1;
    killed_vehicle = -1;
    base_check_support();
    assert(killed_vehicle == 0);
    assert((base.state & BSTATE_DRONE_RIOTS_ACTIVE) == 0);
}

"""

BASE_RESEARCH = r"""
#include <cassert>
#include <cstdint>

#ifndef __cdecl
#define __cdecl
#endif
const unsigned int RULES_SCN_NO_TECH_ADVANCES = 0x04000000u;
unsigned int GameRules = 0;
int TurnCurrentNum = 0;
struct Base { int faction_id_current, labs_total; uint8_t facilities_built[12]; };
struct SocialEffect { int research; };
struct PlayerData { int diff_level; SocialEffect soc_effect_pending; int labs_total, net_random_event; };
Base *BaseCurrent = 0;
Base Bases[8] = {};
PlayerData PlayersData[8] = {};
int last_research_faction = -1, last_research_points = -1, research_calls = 0;

void __cdecl bitmask(int id, int *offset, int *mask) {
    *offset = id / 8; *mask = 1 << (id % 8);
}
void __cdecl tech_research(int faction, int points) {
    last_research_faction = faction;
    last_research_points = points;
    ++research_calls;
}

HELPERS
BODY

int main() {
    Base base = {};
    base.faction_id_current = 3;
    base.labs_total = 25;
    BaseCurrent = &base;
    PlayerData &player = PlayersData[3];
    player.diff_level = 3;
    player.soc_effect_pending.research = 0;

    Bases[3].facilities_built[2] = 0x80;
    base_research();
    assert(player.labs_total == 12 && last_research_points == 12);
    assert(research_calls == 1 && last_research_faction == 3);

    Bases[3].facilities_built[2] = 0;
    player.labs_total = 0;
    player.net_random_event = 0;
    player.soc_effect_pending.research = -2;
    TurnCurrentNum = 11;
    base_research();
    assert(player.labs_total == 25 && player.net_random_event == 0);
    assert(last_research_points == 20);

    // Wrapped threshold -5 * -2 == 10 suppresses points at turn 9.
    player.labs_total = 31;
    player.net_random_event = 0;
    TurnCurrentNum = 9;
    base_research();
    assert(player.labs_total == 31 && last_research_points == 0);

    player.diff_level = 1;
    TurnCurrentNum = 4;
    player.net_random_event = 37;
    const int calls_before_gate = research_calls;
    base_research();
    assert(player.labs_total == 31 && player.net_random_event == 37);
    assert(research_calls == calls_before_gate + 1 && last_research_points == 0);

    GameRules = RULES_SCN_NO_TECH_ADVANCES;
    const int calls_before_rule = research_calls;
    base_research();
    assert(player.labs_total == 31 && research_calls == calls_before_rule);

    GameRules = 0;
    player.diff_level = 3;
    player.soc_effect_pending.research = 0;
    player.labs_total = 0x7FFFFFFF;
    base.labs_total = 0x7FFFFFFF;
    TurnCurrentNum = 10;
    base_research();
    assert(player.labs_total == -2 && last_research_points == -1);

    // -5 * INT_MIN wraps to INT_MIN, so turn -1 is not below that threshold.
    player.labs_total = 17;
    player.net_random_event = 0;
    player.soc_effect_pending.research = (-2147483647 - 1);
    base.labs_total = 25;
    TurnCurrentNum = -1;
    base_research();
    assert(player.labs_total == 42 && last_research_points == 12);
    assert(player.net_random_event == 50);
}

"""

TECH_RESEARCH = r"""
#include <cassert>
#include <cstdint>

#ifndef __cdecl
#define __cdecl
#endif

struct RulesFacility { int preq_tech; };
struct RulesTechnology { int preq_tech_1, preq_tech_2; };
struct Player { char name_leader[24]; };
struct PlayerData { uint32_t flags; int tech_accumulated, tech_id_researching, tech_cost; };
struct Console { int faction_id_; };
RulesFacility Facility[134] = {};
RulesTechnology Technology[89] = {};
uint8_t GameTechAchieved[89] = {};
Player Players[16] = {};
PlayerData PlayersData[16] = {};
Console ConsoleGlobal = {};
uint32_t LockEnableMask = 0;
int IsMultiplayerNet = 0, IsMultiplayerPBEM = 0, TurnUpkeepPhase = 1;
int tech_rate_value = 10, tech_advance_value = -1, selection_value = -1;
int climactic_value = 1, selection_calls;

int __cdecl climactic_battle() { return climactic_value; }
int __cdecl tech_rate(int) { return tech_rate_value; }
int __cdecl tech_advance(int) { return tech_advance_value; }
int __cdecl tech_selection(int) { ++selection_calls; return selection_value; }
void __cdecl mon_tech_discovered(int, int) {}
void __cdecl log_say(const char *, const char *, int, int, int) {}

HELPERS
BODY

int main() {
    // Faction 8 is outside normal gameplay but provides a focused machine-level
    // distinction: TEST BL,AL sees the low mask byte (zero), not dword bit 8.
    // The fixture grows the state arrays only to make that operand control safe.
    const int faction = 8;
    LockEnableMask = 1u << faction;
    Facility[33].preq_tech = -1;
    Facility[102].preq_tech = 1;
    Technology[1].preq_tech_1 = -1;
    Technology[1].preq_tech_2 = -1;
    GameTechAchieved[1] = 0;
    PlayersData[faction].tech_id_researching = -1;
    TurnUpkeepPhase = 1;
    tech_research(faction, 3);
    assert(PlayersData[faction].tech_accumulated == 6);
    assert(selection_calls == 0);
}

"""

def helper(source, name):
    start = source.index(name + "(")
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
@pytest.mark.parametrize("unit,address,harness,prefix", [
    ("base.cpp", 0x4F4830, SUPPORT, "base_yield"),
    ("base.cpp", 0x4F4E80, BASE_RESEARCH, "base_yield"),
    ("technology.cpp", 0x5BE940, TECH_RESEARCH, "tech_research"),
], ids=["support", "base-research", "tech-research"])
def test_actual_colony_support_research(compiler, unit, address, harness, prefix, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    path = ROOT / "src" / unit
    source = path.read_text()
    records = [r for r in read_file(path) if r.address == address]
    assert len(records) == 1 and not records[0].extract_error
    assert "BODY GOES HERE" not in records[0].region
    assert "PENDING_BODY" not in records[0].region
    operations = ("signed_bits", "add", "sub", "mul") if prefix == "base_yield" else ("signed_bits", "add", "mul")
    helpers = "\n".join(helper(source, prefix + "_" + name) for name in operations)
    control = harness.replace("HELPERS", helpers).replace("BODY", records[0].region)
    control_path = tmp_path / "control.cpp"
    control_path.write_text(control)
    binary = tmp_path / "control"
    compiled = subprocess.run([executable, "-std=c++14", "-O2", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined", str(control_path), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True)
    assert run.returncode == 0, run.stdout + run.stderr
    assert not run.stderr, run.stderr
