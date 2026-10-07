"""Actual-owner extraction v2 with readable embedded C++98 host adapters.

At runtime this module reads ONLY tracked owner sources under src/. C++ fixture
source below is embedded directly in this test; it does not open ignored packet
fixtures. It uses match_recovery.source records plus lexical C++ extraction,
explicitly composes helpers that sit before ORIGINAL markers, then compiles as
C++98 and executes. This is not native ABI, matching, or game acceptance
evidence.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]

HOST_FIXTURE = r'''// Non-product syntax harness. It compiles the actual candidate function
// bodies against minimal host declarations; it proves neither project ABI nor
// layouts, behavior, object emission, or original matching.
#include <stdint.h>

#define __cdecl
enum { MaxPlayerNum = 8 };
enum {
    PFLAG_SELF_AWARE_COLONY_LOST_MAINT = 0x20,
    DTREATY_VENDETTA = 0x10,
    RFLAG_INTEREST = 0x4000,
    RFLAG_ALIEN = 0x80,
    STATE_UNK_1 = 1,
    STATE_UNK_8 = 8,
    STATE_DISPLAYED_COUNCIL_AVAIL_MSG = 0x4000,
    STATE_COUNCIL_HAS_CONVENED = 0x10000,
    BSTATE_PRODUCTION_HURRIED = 0x40000000,
    BSTATE_DRONE_RIOTS_ACTIVE = 2,
    RULES_VICTORY_ECONOMIC = 4,
    PREF_BSC_AUTOSAVE_EACH_TURN = 2,
};

struct Goal { int16_t type, priority; int x, y, base_id; };
struct PlayerData {
    uint32_t flags;
    int diplo_treaties[8];
    int loan_balance[8];
    int loan_payment[8];
    int energy_reserves;
    int hurry_cost_total;
    int unk_13;
    int maint_cost_total;
    int turn_commerce_income;
    int unk_17, unk_18;
    int unk_28;
    int unk_38[8];
    int unk_39[8][9];
    int unk_46[9];
    int unk_47, unk_48, unk_49;
    uint32_t nutrient_surplus_total;
    int labs_total;
    uint8_t proto_id_active[512];
    uint32_t current_num_bases;
    int corner_market_turn, corner_market_active;
    int sanction_turns;
    Goal goals[75], sites[25];
};
struct Player {
    int is_leader_female;
    char name_leader[24], title_leader[24], noun_faction[24];
    int noun_gender, is_noun_plural;
    char adj_name_faction[128];
    int rule_tech_selected, rule_interest, rule_flags;
};
struct Base { uint8_t faction_id_current; uint8_t pad[0x2b]; uint32_t state; };
struct RulesBasic { int pad[40]; int turns_corner_gbl_energy_mrkt; int pad2[1]; int tech_economic_victory; };
struct Console { int faction_id_; };
struct Path { void clear_destination(); };
struct SocialCategory {};

extern PlayerData PlayersData[MaxPlayerNum];
extern Player Players[MaxPlayerNum];
extern Base Bases[256];
extern RulesBasic Rules;
extern Console ConsoleGlobal;
extern Path Paths;
extern int CurrentFactionEnergyReserves, BaseCurrentCount, DiffLevelCurrent;
extern int GameState, GameRules, GamePreferences, TurnCurrentNum;
extern int TechAchievedGate, UnkGlobal0093A934, GenderDefault, PluralityDefault;
extern int IsMultiplayerNet;

int base_upkeep(int);
void tech_effects(int);
void do_all_non_input();
void log_say(const char *, const char *, int, int, int);
int tech_advance(int);
int popp(const char *, const char *, int, const char *, int (__cdecl *)());
void social_upkeep(int);
void repair_phase(int);
void allocate_energy(int);
void enemy_diplomacy(int);
void enemy_strategy(int);
void social_ai(int, int, int, int, int, SocialCategory *);
int corner_market(int);
int has_tech_call(int, int);
int parse_says(int, const char *, int, int);
int parse_num(int, int);
uint32_t game_year(int);
int eliminate_player(int, int);
int can_call_council(int, int);
void call_council(int);
void auto_save();
int is_human(uint32_t);
int is_alive(uint32_t);

#ifdef PRODUCTION_PHASE_V2
#include "production_phase.v2.cpp"
#else
#include "production_phase.v1.cpp"
#endif
#ifdef FACTION_UPKEEP_V2
#include "faction_upkeep.v2.cpp"
#else
#include "faction_upkeep.v1.cpp"
#endif
'''

TURN_FIXTURE = r'''// Private host-only control over the actual production_phase and
// faction_upkeep bodies. Models/stubs below are test adapters, not product
// layouts, recovered helper implementations, or ABI/runtime evidence.
#include "host_syntax_control.cpp"
#include <cassert>

PlayerData PlayersData[MaxPlayerNum];
Player Players[MaxPlayerNum];
Base Bases[256];
RulesBasic Rules;
Console ConsoleGlobal;
Path Paths;
int CurrentFactionEnergyReserves, BaseCurrentCount, DiffLevelCurrent;
int GameState, GameRules, GamePreferences, TurnCurrentNum;
int TechAchievedGate, UnkGlobal0093A934, GenderDefault, PluralityDefault;
int IsMultiplayerNet;

int alive_state[MaxPlayerNum];
int human_state[MaxPlayerNum];
int upkeep_calls[16], upkeep_returns[16], upkeep_call_count;
int change_count_on_upkeep;
int callback_non_input_count, social_calls, research_advance_calls;
int path_clear_calls, elimination_calls, council_calls, can_council_calls;
int autosave_calls, popup_calls, parse_says_calls, parse_num_calls;
int observed_upkeep_latch;
int can_council_changes_human;
int corner_values[8], corner_value_count, corner_market_calls;
int has_tech_result;

void Path::clear_destination() { ++path_clear_calls; }

int __cdecl base_upkeep(int base_id) {
    const int index = upkeep_call_count++;
    upkeep_calls[index] = base_id;
    if (change_count_on_upkeep != 0) BaseCurrentCount = change_count_on_upkeep;
    return upkeep_returns[index];
}
void tech_effects(int) {}
void do_all_non_input() { ++callback_non_input_count; }
void log_say(const char *, const char *, int, int, int) {}
int tech_advance(int) { ++research_advance_calls; return 0; }
int popp(const char *, const char *, int, const char *, int (__cdecl *)()) {
    ++popup_calls; return 0;
}
void social_upkeep(int) { ++social_calls; observed_upkeep_latch = UnkGlobal0093A934; }
void repair_phase(int) {}
void allocate_energy(int) {}
void enemy_diplomacy(int) {}
void enemy_strategy(int) {}
void social_ai(int, int, int, int, int, SocialCategory *) {}
int corner_market(int) {
    const int value = corner_values[corner_market_calls];
    ++corner_market_calls;
    return value;
}
int has_tech_call(int, int) { return has_tech_result; }
int parse_says(int, const char *, int, int) { ++parse_says_calls; return 0; }
int parse_num(int, int) { ++parse_num_calls; return 0; }
uint32_t game_year(int) { return 2100; }
int eliminate_player(int faction_id, int) {
    ++elimination_calls;
    alive_state[faction_id] = 0;
    return 0;
}
int can_call_council(int faction_id, int) {
    ++can_council_calls;
    if (can_council_changes_human) human_state[faction_id] = 0;
    return 1;
}
void call_council(int) { ++council_calls; }
void auto_save() { ++autosave_calls; }
int is_human(uint32_t faction_id) { return human_state[faction_id]; }
int is_alive(uint32_t faction_id) { return alive_state[faction_id]; }

static void reset_fixture() {
    for (int i = 0; i < MaxPlayerNum; ++i) {
        PlayersData[i] = PlayerData();
        Players[i] = Player();
        alive_state[i] = 1;
        human_state[i] = 1;
    }
    for (int i = 0; i < 256; ++i) Bases[i] = Base();
    Rules = RulesBasic();
    ConsoleGlobal.faction_id_ = 1;
    CurrentFactionEnergyReserves = BaseCurrentCount = DiffLevelCurrent = 0;
    GameState = GameRules = GamePreferences = TurnCurrentNum = 0;
    TechAchievedGate = UnkGlobal0093A934 = GenderDefault = PluralityDefault = 0;
    IsMultiplayerNet = 0;
    for (int i = 0; i < 16; ++i) upkeep_calls[i] = upkeep_returns[i] = 0;
    upkeep_call_count = change_count_on_upkeep = 0;
    callback_non_input_count = social_calls = research_advance_calls = 0;
    path_clear_calls = elimination_calls = council_calls = can_council_calls = 0;
    autosave_calls = popup_calls = parse_says_calls = parse_num_calls = 0;
    observed_upkeep_latch = 0;
    can_council_changes_human = 0;
    for (int i = 0; i < 8; ++i) corner_values[i] = 0;
    corner_value_count = corner_market_calls = has_tech_result = 0;
}

static void test_production_loan_and_retry() {
    reset_fixture();
    const int faction = 1;
    const int partner = 2;
    PlayersData[faction].energy_reserves = 12;
    PlayersData[faction].loan_balance[partner] = 10;
    PlayersData[faction].loan_payment[partner] = 8;
    PlayersData[partner].energy_reserves = 20;
    PlayersData[partner].sanction_turns = 0;
    Bases[0].faction_id_current = Bases[1].faction_id_current = faction;
    BaseCurrentCount = 2;
    upkeep_returns[0] = 1;  // callback asks native loop to retry base 0
    upkeep_returns[1] = 0;
    upkeep_returns[2] = 0;
    production_phase(faction);

    assert(upkeep_call_count == 3);
    assert(upkeep_calls[0] == 0 && upkeep_calls[1] == 0 && upkeep_calls[2] == 1);
    assert(callback_non_input_count == 3);
    assert(PlayersData[faction].energy_reserves == 4);
    assert(PlayersData[faction].loan_balance[partner] == 2);
    assert(PlayersData[partner].energy_reserves == 28);
    assert(CurrentFactionEnergyReserves == 4);  // latched after loan debit
}

static void test_production_partner_sanction_blocks_loan() {
    reset_fixture();
    const int faction = 1;
    const int partner = 2;
    PlayersData[faction].energy_reserves = 12;
    PlayersData[faction].loan_balance[partner] = 10;
    PlayersData[faction].loan_payment[partner] = 8;
    PlayersData[partner].energy_reserves = 20;
    PlayersData[partner].sanction_turns = 1;
    production_phase(faction);
    assert(PlayersData[faction].energy_reserves == 12);
    assert(PlayersData[faction].loan_balance[partner] == 10);
    assert(PlayersData[partner].energy_reserves == 20);
}

static void test_production_reloads_base_count() {
    reset_fixture();
    Bases[0].faction_id_current = Bases[1].faction_id_current = 1;
    BaseCurrentCount = 2;
    change_count_on_upkeep = 1;
    production_phase(1);
    assert(upkeep_call_count == 1 && upkeep_calls[0] == 0);
    assert(callback_non_input_count == 1);
}

static void test_faction_corner_market_and_path() {
    reset_fixture();
    const int faction = 1;
    human_state[faction] = 0;
    PlayersData[faction].energy_reserves = 100;
    PlayersData[faction].hurry_cost_total = 7;
    PlayersData[faction].current_num_bases = 1;
    PlayersData[faction].proto_id_active[0] = 1;
    Rules.turns_corner_gbl_energy_mrkt = 8;
    Rules.tech_economic_victory = 5;
    GameRules = RULES_VICTORY_ECONOMIC;
    TurnCurrentNum = 12;
    IsMultiplayerNet = 1;
    has_tech_result = 1;
    corner_values[0] = 10;  // eligibility query
    corner_values[1] = 5;   // stored active amount
    corner_values[2] = 4;   // debit amount
    corner_value_count = 3;
    faction_upkeep(faction);

    assert(corner_market_calls == 3);
    assert(PlayersData[faction].corner_market_turn == 20);
    assert(PlayersData[faction].corner_market_active == 5);
    assert(PlayersData[faction].energy_reserves == 89);
    assert(PlayersData[faction].hurry_cost_total == 0);
    assert(path_clear_calls == 1);
    assert(observed_upkeep_latch == 1 && UnkGlobal0093A934 == 0);
    assert(council_calls == 1 && can_council_calls == 1);
    assert((GameState & STATE_DISPLAYED_COUNCIL_AVAIL_MSG) != 0);
}

static void test_faction_liveness_after_elimination() {
    reset_fixture();
    const int faction = 1;
    human_state[faction] = 0;
    PlayersData[faction].current_num_bases = 0;
    PlayersData[faction].proto_id_active[0] = 0;
    PlayersData[faction].proto_id_active[21] = 0;
    TurnCurrentNum = 2;
    IsMultiplayerNet = 1;
    faction_upkeep(faction);
    assert(elimination_calls == 1 && alive_state[faction] == 0);
    assert(path_clear_calls == 1);
    assert(council_calls == 1);  // nonhuman gate is independent of liveness
    assert(UnkGlobal0093A934 == 0);
}

static void test_faction_council_human_mask_reread() {
    reset_fixture();
    const int faction = 1;
    human_state[faction] = 1;
    faction_upkeep(faction);
    assert(council_calls == 0);  // human faction is skipped

    reset_fixture();
    human_state[faction] = 1;
    can_council_changes_human = 1;
    faction_upkeep(faction);
    assert(can_council_calls == 1);
    assert(human_state[faction] == 0);
    assert(council_calls == 1);  // fresh post-callback HUMAN-bit read
}

#ifdef PRODUCTION_SANCTION_ONLY
int main() {
    test_production_partner_sanction_blocks_loan();
    return 0;
}
#else
int main() {
    test_production_loan_and_retry();
    test_production_partner_sanction_blocks_loan();
    test_production_reloads_base_count();
    test_faction_corner_market_and_path();
    test_faction_liveness_after_elimination();
    test_faction_council_human_mask_reread();
    return 0;
}
#endif
'''

BASE_UPKEEP_FIXTURE = r'''// Private host-only behavioral transcription control. All declaration stubs
// below are test adapters, not recovered behavior or project ABI evidence.
#include <cassert>
#include <stdint.h>
#include <cstdlib>
#include <cstring>

#define __cdecl
enum { MaxPlayerNum = 8, MaxSecretProjectNum = 64 };
enum { STATE_UNK_1 = 1 };

struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    int energy_reserves;
    SocialEffect soc_effect_base;
    int unk_47;
    uint8_t unk_83[128];
    int sanction_turns;
    int turn_commerce_income;
};
struct Player { int rule_flags, noun_gender, is_noun_plural; char noun_faction[24]; };
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former, assimilation_turns_left,
        nerve_staple_turns_left, visibility;
    char name_string[25];
    uint32_t state;
    int minerals_accumulated, minerals_accumulated_2, production_id_last;
    int queue_production_id[10];
    int resource_sqr_radius;
    uint8_t facilities_built[12];
    int economy_total;
};
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory; uint32_t bit, bit2, bit_visible[7]; };
struct SecretProjects { int base_id[64]; };
struct TestConsole {
    int faction_id_;
    int focus(int, int, int) { ++focus_calls; return 1; }
    int focus_calls = 0;
};
struct TestBaseWin {
    int focused_base_id() { return -1; }
    void on_redraw() { ++redraws; }
    void zoom(int, int) { ++zooms; }
    int redraws = 0, zooms = 0;
};

PlayerData PlayersData[MaxPlayerNum];
Player Players[MaxPlayerNum];
Base Bases[8];
Map MapTiles[128];
SecretProjects SecretProject;
int BaseCommerceReceipts[7];
TestConsole ConsoleGlobal;
TestBaseWin TutWinBaseWindow;
int BaseIDCurrentSelected, BaseCurrentCount, BaseComputedID;
Base *BaseCurrent;
uint32_t BaseUpkeepStage;
int BaseProductionZoomRequested;
int UnkGlobal0093A934;
int TurnCurrentNum, IsMultiplayerNet, GameState;
uint32_t GamePreferences;
uint32_t MapLongitude = 16;
int GenderDefault, PluralityDefault;
int control_parse_calls, control_name_calls, control_draw_map_calls,
    control_popp_calls, control_log_calls;
const char *control_popup_label, *control_popup_picture;
int growth_result, change_owner_after_drones, request_zoom_from_production;
int calls_support, calls_yield, calls_nutrient, calls_minerals,
    calls_energy, calls_production, calls_hurry, calls_growth,
    calls_drones, calls_doctors, calls_energy_costs, calls_research,
    calls_ecology, calls_maint, calls_terraform;
int __cdecl bit_count(int value) {
#ifdef BASE_UPKEEP_DAMAGE_POPCOUNT
    (void)value;
    return 0;  // injected omission; control must distinguish it
#else
    uint32_t bits = static_cast<uint32_t>(value);
    int count = 0;
    while (bits != 0) { count += bits & 1u; bits >>= 1; }
    return count;
#endif
}
int base_project(int id) { return SecretProject.base_id[id]; }
void base_support() { ++calls_support; }
void base_yield() { ++calls_yield; }
void base_nutrient() { ++calls_nutrient; }
void base_minerals() { ++calls_minerals; }
void base_energy() { ++calls_energy; }
void base_production() {
    ++calls_production;
    if (request_zoom_from_production) BaseProductionZoomRequested = 1;
}
void base_hurry() { ++calls_hurry; }
int base_growth() { ++calls_growth; return growth_result; }
void base_check_support() {}
void base_drones() { ++calls_drones; if (change_owner_after_drones) Bases[0].faction_id_current = 2; }
void base_doctors() { ++calls_doctors; }
void base_energy_costs() { ++calls_energy_costs; }
void base_research() { ++calls_research; }
void base_ecology() { ++calls_ecology; }
void base_maint() { ++calls_maint; }
void base_terraform(int) { ++calls_terraform; }
int bit_count(int);
int parse_says(int, const char *, int, int) { ++control_parse_calls; return 0; }
void name_base(int, char *, int, int) { ++control_name_calls; }
int draw_tiles(int, int, int) { return 0; }
int draw_map(int) { ++control_draw_map_calls; return 0; }
int popp(const char *, const char *label, int, const char *picture, int (*)()) {
    ++control_popp_calls;
    control_popup_label = label;
    control_popup_picture = picture;
    return 0;
}
int log_say(const char *, int, int, int) { ++control_log_calls; return 0; }

#ifdef BASE_UPKEEP_DAMAGE_POPCOUNT
// Avoid feeding real data into the second (deliberately damaged) variant.
#define BaseUpkeepControlDamaged 1
#endif
#define rand() 0  // deterministic host control for the native random-event gate
#ifdef BASE_UPKEEP_CONTROL_V2
#include "base_upkeep.v2.cpp"
#else
#include "base_upkeep.v1.cpp"
#endif
#undef rand

static void reset_fixture() {
    for (int i = 0; i < MaxPlayerNum; ++i) {
        PlayersData[i] = PlayerData();
        Players[i] = Player();
    }
    for (int i = 0; i < 8; ++i) Bases[i] = Base();
    for (int i = 0; i < 128; ++i) MapTiles[i] = Map();
    for (int i = 0; i < MaxSecretProjectNum; ++i) SecretProject.base_id[i] = -1;
    BaseIDCurrentSelected = BaseComputedID = 0;
    BaseCurrentCount = 1;
    BaseCurrent = &Bases[0];
    BaseUpkeepStage = 0;
    BaseProductionZoomRequested = 0;
    TurnCurrentNum = 1;
    IsMultiplayerNet = GameState = 0;
    GamePreferences = 0;
    ConsoleGlobal.faction_id_ = 1;
    growth_result = change_owner_after_drones = request_zoom_from_production = 0;
    calls_support = calls_yield = calls_nutrient = calls_minerals = 0;
    calls_energy = calls_production = calls_hurry = calls_growth = 0;
    calls_drones = calls_doctors = calls_energy_costs = calls_research = 0;
    calls_ecology = calls_maint = calls_terraform = 0;
    control_parse_calls = control_name_calls = control_draw_map_calls = 0;
    control_popp_calls = control_log_calls = 0;
    control_popup_label = control_popup_picture = 0;
    ConsoleGlobal.focus_calls = 0;
    TutWinBaseWindow.redraws = TutWinBaseWindow.zooms = 0;
    Bases[0].x = 2;
    Bases[0].y = 1;
    Bases[0].faction_id_current = Bases[0].faction_id_former = 1;
    Bases[0].assimilation_turns_left = 4;
    Bases[0].nerve_staple_turns_left = 3;
    Bases[0].state = 0x20000208u;
    Bases[0].resource_sqr_radius = 0xBu;
    Bases[0].economy_total = 37;
    PlayersData[1].energy_reserves = 100;
    PlayersData[1].unk_47 = 10;
    PlayersData[1].unk_83[3] = 2;
    for (int i = 0; i < 7; ++i) BaseCommerceReceipts[i] = 10 + i;
    MapTiles[17].region = 3;
}

int main() {
    reset_fixture();
    assert(base_upkeep(0) == 0);
    assert(PlayersData[1].energy_reserves == 137);
    assert(PlayersData[1].turn_commerce_income == 81);
#ifdef BASE_UPKEEP_DAMAGE_POPCOUNT
    assert(PlayersData[1].unk_47 == 10);
#else
    assert(PlayersData[1].unk_47 == 13);
#endif
    assert(BaseUpkeepStage == 0);
    assert(Bases[0].assimilation_turns_left == 2);
    assert(Bases[0].nerve_staple_turns_left == 2);
    assert((Bases[0].state & 0x20000008u) == 0);
    assert(calls_growth == 1 && calls_ecology == 1 && calls_terraform == 1);
    assert(calls_support == 2 && calls_yield == 2 && calls_nutrient == 2);

    reset_fixture();
    Bases[0].state |= 0x10000u;
    request_zoom_from_production = 1;
    assert(base_upkeep(0) == 0);
    assert(control_parse_calls == 3 && control_name_calls == 1);
    assert(control_draw_map_calls == 1 && control_popp_calls == 1);
    assert(control_log_calls == 2 && TutWinBaseWindow.zooms == 1);
    assert(strcmp(control_popup_label, "RENAMEBASE") == 0);
    assert(strcmp(control_popup_picture, "colfoun_sm.pcx") == 0);
    assert((Bases[0].state & 0x10000u) == 0 && (Bases[0].state & 0x80u) != 0);

    reset_fixture();
    Bases[0].state |= 0x10000u;
    Players[1].rule_flags = 0x80;
    assert(base_upkeep(0) == 0);
    assert(control_popp_calls == 1);
    assert(strcmp(control_popup_label, "RENAMEBASE") == 0);
    assert(strcmp(control_popup_picture, "al_col_sm.pcx") == 0);

    reset_fixture();
    growth_result = 1;
    assert(base_upkeep(0) == 1);
    assert(BaseUpkeepStage == 0);
    assert(calls_growth == 1 && calls_ecology == 0 && calls_terraform == 0);
    assert((Bases[0].state & 0x8u) != 0);  // native early return skips final reset

    reset_fixture();
    TurnCurrentNum = 2;
    change_owner_after_drones = 1;
    assert(base_upkeep(0) == 1);
    assert(Bases[0].faction_id_current == 2);
    assert(BaseUpkeepStage == 2);
    assert(calls_drones == 1 && calls_doctors == 1);
    assert(calls_energy_costs == 0 && calls_research == 0 && calls_ecology == 0);

#ifdef BASE_UPKEEP_COMMERCE_ROLE_ASSERTIONS
    // Actor alien, partner human: native ESI-indexed actor flags suppress all
    // commerce, even though every partner is otherwise eligible.
    reset_fixture();
    Players[1].rule_flags = 0x80;
    assert(base_upkeep(0) == 0);
    assert(PlayersData[1].turn_commerce_income == 0);

    // Actor human, partner 2 alien: the walked partner pointer independently
    // suppresses only that partner; partners 3..7 still contribute 12..16.
    reset_fixture();
    Players[2].rule_flags = 0x80;
    assert(base_upkeep(0) == 0);
    assert(PlayersData[1].turn_commerce_income == 70);
#endif

    return 0;
}
'''

BASE_DRONES_FIXTURE = r'''// Private host fixture for the actual base_drones candidate body. These
// structs/callbacks model only observed fields and callback state changes;
// they are not product layout or ABI evidence.
#include <cassert>
#include <stdint.h>
#include <string.h>

#define __cdecl
enum {
    BSTATE_DRONE_RIOTS_ACTIVE = 0x2,
    BSTATE_GOLDEN_AGE_ACTIVE = 0x4,
    GOV_MANAGES_CITIZENS_SPECS = 0x40,
    GOV_ACTIVE = 0x80000000u,
    RFLAG_ALIEN = 0x80,
};
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left, nerve_staple_turns_left;
    uint8_t unk_1, visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unk_x, unk_y;
    uint32_t state;
    uint32_t event;
    uint32_t governor;
    uint8_t pad2[0xDC];
    int talent_total;
    int drone_total;
};
struct Player { int rule_flags; };
struct BaseSecretProject {
    int projects_before_telepathic_matrix[30];
    int telepathic_matrix_;
    int projects_after_telepathic_matrix[33];

 int &telepathic_matrix() { return telepathic_matrix_; }
 const int &telepathic_matrix() const { return telepathic_matrix_; }
};
class Sprite {};
Base Bases[8];
Base *BaseCurrent;
int BaseIDCurrentSelected;
uint32_t LockEnableMask;
Player Players[8];
BaseSecretProject SecretProject;
int pop_count, pop_flags, pop_value, reset_count, draw_count, riot_count;
const char *pop_caption, *pop_picture;
int draw_x, draw_y;
int reset_base_id, reset_force;
bool switch_current_on_popup;


int __cdecl popb(const char *caption, int flags, int value,
                 const char *picture, Sprite *) {
    ++pop_count;
    pop_caption = caption;
    pop_picture = picture;
    pop_flags = flags;
    pop_value = value;
    if (switch_current_on_popup) BaseCurrent = &Bases[1];
    return 0;
}
void __cdecl base_reset(int base_id, int force) {
    ++reset_count;
    reset_base_id = base_id;
    reset_force = force;
}
void __cdecl draw_tile(int x, int y, int) {
    ++draw_count;
    draw_x = x;
    draw_y = y;
}
void __cdecl drone_riot() { ++riot_count; }

#include "base_drones.v1.cpp"

static void reset_fixture() {
    for (int i = 0; i < 8; ++i) {
        Bases[i] = Base();
        Players[i] = Player();
    }
    Bases[0].faction_id_current = 1;
    Bases[0].population_size = 4;
    Bases[0].x = 12;
    Bases[0].y = 18;
    BaseCurrent = &Bases[0];
    BaseIDCurrentSelected = 0;
    LockEnableMask = 0;
    SecretProject = BaseSecretProject();
    SecretProject.telepathic_matrix() = -1;
    pop_count = pop_flags = pop_value = reset_count = draw_count = riot_count = 0;
    pop_caption = pop_picture = 0;
    draw_x = draw_y = 0;
    reset_base_id = reset_force = -1;
    switch_current_on_popup = false;
}

static void test_golden_age_callback_reload() {
    reset_fixture();
    Bases[0].talent_total = 2;
    Bases[0].drone_total = 0;
    switch_current_on_popup = true;
    base_drones();
    assert(pop_count == 1 && pop_flags == 0x20 && pop_value == -1);
    assert(strcmp(pop_caption, "GOLDENAGE") == 0);
    assert(strcmp(pop_picture, "talent_sm.pcx") == 0);
    assert((Bases[0].state & BSTATE_GOLDEN_AGE_ACTIVE) == 0);
    assert((Bases[1].state & BSTATE_GOLDEN_AGE_ACTIVE) != 0);
}

static void test_golden_age_end_and_riot_exemption() {
    reset_fixture();
    Bases[0].state = BSTATE_GOLDEN_AGE_ACTIVE;
    Bases[0].talent_total = 3;
    Bases[0].drone_total = 2;
    base_drones();
    assert(pop_count == 1 && pop_flags == 0x40);
    assert(strcmp(pop_caption, "GOLDENAGEOVER") == 0);
    assert(strcmp(pop_picture, "talent_sm.pcx") == 0);
    assert((Bases[0].state & BSTATE_GOLDEN_AGE_ACTIVE) == 0);

    reset_fixture();
    Bases[0].drone_total = 5;
    Bases[0].talent_total = 1;
    Bases[2].faction_id_current = 1;
    SecretProject.telepathic_matrix() = 2;
    Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
    base_drones();
    assert(riot_count == 0);
    assert(pop_count == 1 && pop_value == 0x10);
    assert(reset_count == 1 && draw_count == 1);

    // A nonzero selected identity distinguishes base_id from the force flag.
    reset_fixture();
    Bases[3] = Bases[0];
    Bases[3].state = BSTATE_DRONE_RIOTS_ACTIVE;
    BaseIDCurrentSelected = 3;
    BaseCurrent = &Bases[3];
    base_drones();
    assert(reset_count == 1 && reset_base_id == 3 && reset_force == 0);
}

static void test_riot_early_return_and_alien_dialog() {
    reset_fixture();
    Bases[0].drone_total = 6;
    Bases[0].talent_total = 1;
    Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
    base_drones();
    assert(riot_count == 1);
    assert(pop_count == 0 && draw_count == 0 && reset_count == 0);
    assert((Bases[0].state & BSTATE_DRONE_RIOTS_ACTIVE) != 0);

    reset_fixture();
    Players[1].rule_flags = RFLAG_ALIEN;
    Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
    base_drones();
    assert(pop_count == 1 && pop_flags == 0x33 && pop_value == 0x10);
    assert(strcmp(pop_caption, "DRONERIOTSOVER") == 0);
    assert(strcmp(pop_picture, "Alopdir.pcx") == 0);
}

int main() {
    test_golden_age_callback_reload();
    test_golden_age_end_and_riot_exemption();
    test_riot_early_return_and_alien_dialog();
    return 0;
}
'''

DRONE_API_FIXTURE = r'''#ifndef OPENSMACX_DRONE_RIOT_FIXTURE_API_H
#define OPENSMACX_DRONE_RIOT_FIXTURE_API_H

#include <stdint.h>
#include <stdlib.h>

#ifndef __cdecl
#define __cdecl
#endif

typedef int BOOL;
typedef const char *LPCSTR;
struct Sprite {};

enum {
    BSTATE_DRONE_RIOTS_ACTIVE = 0x00000002u,
    VSTATE_UNK_40000 = 0x00040000u,
    DTREATY_PACT = 0x00000001u,
    DTREATY_TREATY = 0x00000002u,
    DTREATY_TRUCE = 0x00000004u,
    ORDER_NONE = 0,
    ORDER_HOLD = 2,
    RFLAG_ALIEN = 0x80
};

static const int MaxPlayerNum = 8;
static const int MaxFacilityNum = 134;

struct Base {
    int16_t x;
    int16_t y;
    uint8_t faction_id_current;
    uint8_t faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left;
    uint8_t unused_8;
    uint8_t unused_9;
    uint8_t visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unused_xy[2];
    uint32_t state;
    uint32_t event;
    uint32_t governor;
    int nutrients_accumulated;
    int minerals_accumulated;
    int production_id_last;
    int eco_damage;
    int queue_size;
    int queue_production_id[10];
    int unused_78;
    int specialist_total;
    int specialist_unk_1;
    uint32_t spec[2];
    uint8_t facilities_built[12];
    int mineral_surplus_final;
    int minerals_accumulated_2;
    uint8_t tail[0x134 - 0xa0];
};

struct SocialCategory { uint32_t models[4]; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    uint32_t flags;
    uint32_t ranking;
    int diff_level;
    uint32_t base_name_offset;
    uint32_t base_sea_name_offset;
    int last_turn_new_base;
    uint32_t diplo_treaties[8];
    uint8_t unused_to_social_effect[0x24c - 0x38];
    SocialEffect soc_effect_pending;
    uint8_t unused_to_base_target[0x468 - 0x278];
    int base_id_atk_target;
    uint8_t padding_to_prototypes[0x858 - 0x46c];
    uint8_t proto_id_active[512];
    uint8_t padding_to_base_count[0x105c - 0xa58];
    uint32_t current_num_bases;
    uint8_t tail[0x20cc - 0x1060];
};
struct Player {
    BOOL is_leader_female;
    char filename[24];
    char search_key[24];
    char name_leader[24];
    char title_leader[24];
    char adj_leader[128];
    char adj_insult_leader[128];
    char adj_faction[128];
    char adj_insult_faction[128];
    uint8_t padding1[128];
    char noun_faction[24];
    int noun_gender;
    BOOL is_noun_plural;
    char adj_name_faction[128];
    char formal_name_faction[40];
    char insult_leader[24];
    char desc_name_faction[24];
    char assistant_name[24];
    char scientist_name[24];
    char assistant_city[24];
    uint8_t padding2[176];
    int rule_tech_selected;
    int rule_morale;
    int rule_research;
    int rule_drone;
    int rule_talent;
    int rule_energy;
    int rule_interest;
    int rule_population;
    int rule_hurry;
    int rule_techcost;
    int rule_psi;
    int rule_sharetech;
    int rule_commerce;
    int rule_flags;
    int faction_bonus_count;
    int faction_bonus_id[8];
    int faction_bonus_val1[8];
    int faction_bonus_val2[8];
    int ai_fight;
    BOOL ai_growth;
    BOOL ai_tech;
    BOOL ai_wealth;
    BOOL ai_power;
    int soc_ideology_category;
    int soc_anti_ideology_category;
    int soc_ideology_model;
    int soc_anti_ideology_model;
    int soc_ideology_effect;
    int soc_anti_ideology_effect;
};
struct RulesFacility {
    char *name;
    char *effect;
    uint32_t pad;
    int cost;
    uint32_t maint;
    int preq_tech;
    int free_tech;
    int sp_ai_fight;
    int sp_ai_growth;
    int sp_ai_tech;
    int sp_ai_wealth;
    int sp_ai_power;
};
struct Map { uint8_t padding[4]; uint8_t visibility; uint8_t tail[0x2c - 5]; };
struct RulesChassis {
    uint8_t prefix[0x4a];
    uint8_t range;
    uint8_t tail[0x90 - 0x4b];
};
struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id;
    uint8_t weapon_id;
    uint8_t armor_id;
    uint8_t reactor_id;
    uint8_t carry_capacity;
    uint8_t cost;
    uint8_t plan;
    int8_t unknown_category;
    uint8_t obsolete_factions;
    int8_t combat_factions;
    int8_t icon_offset;
    int8_t padding;
    uint16_t flags;
    int16_t preq_tech;
};
struct Veh {
    int16_t x;
    int16_t y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unused_1;
    uint8_t faction_id;
    uint8_t unused_2;
    uint8_t damage;
    int8_t order;
    uint8_t unused_waypoints[18];
    uint8_t morale;
    uint8_t terraforming_turns;
    uint8_t unused_orders[0x32 - 0x26];
    int16_t home_base_id;
};
struct Console { int faction_id_; };
struct DroneRiotSecretProject {
    int telepathic_matrix_value_;
    int telepathic_matrix() const { return telepathic_matrix_value_; }
};
struct DroneRiotLockMasks { uint32_t enable_mask_; };
struct DroneRiotGameDataStateFields {
    uint32_t game_state_;
    int vehicle_current_count_;
    int base_current_count_;
    int turn_current_num_;
    DroneRiotLockMasks lock_masks_;
    DroneRiotSecretProject secret_project_;
};
struct DroneRiotGameDataState { DroneRiotGameDataStateFields fields_; };
extern DroneRiotGameDataState GameDataStateStorage;
class MapWin { public: void set_center(int x, int y, int draw_type); };

#if __SIZEOF_POINTER__ == 4
typedef char private_base_size[(sizeof(Base) == 0x134) ? 1 : -1];
typedef char private_player_size[(sizeof(Player) == 0x59c) ? 1 : -1];
typedef char private_player_data_size[(sizeof(PlayerData) == 0x20cc) ? 1 : -1];
typedef char private_facility_size[(sizeof(RulesFacility) == 0x30) ? 1 : -1];
typedef char private_map_size[(sizeof(Map) == 0x2c) ? 1 : -1];
typedef char private_chassis_size[(sizeof(RulesChassis) == 0x90) ? 1 : -1];
typedef char private_prototype_size[(sizeof(VehPrototype) == 0x34) ? 1 : -1];
typedef char private_vehicle_size[(sizeof(Veh) == 0x34) ? 1 : -1];
typedef char private_player_rule_flags[
    (__builtin_offsetof(Player, rule_flags) == 0x508) ? 1 : -1];
typedef char private_base_state[
    (__builtin_offsetof(Base, state) == 0x30) ? 1 : -1];
typedef char private_base_facilities[
    (__builtin_offsetof(Base, facilities_built) == 0x8c) ? 1 : -1];
typedef char private_base_mineral_accumulated_2[
    (__builtin_offsetof(Base, minerals_accumulated_2) == 0x9c) ? 1 : -1];
typedef char private_treaty_offset[
    (__builtin_offsetof(PlayerData, diplo_treaties) == 0x18) ? 1 : -1];
typedef char private_attack_target_offset[
    (__builtin_offsetof(PlayerData, base_id_atk_target) == 0x468) ? 1 : -1];
typedef char private_police_offset[
    (__builtin_offsetof(PlayerData, soc_effect_pending) +
     (__builtin_offsetof(SocialEffect,values_)+sizeof(int)*SocialEffect::POLICE) == 0x260) ? 1 : -1];
typedef char private_active_proto_offset[
    (__builtin_offsetof(PlayerData, proto_id_active) == 0x858) ? 1 : -1];
typedef char private_base_count_offset[
    (__builtin_offsetof(PlayerData, current_num_bases) == 0x105c) ? 1 : -1];
typedef char private_chassis_range[
    (__builtin_offsetof(RulesChassis, range) == 0x4a) ? 1 : -1];
typedef char private_tile_visibility[
    (__builtin_offsetof(Map, visibility) == 4) ? 1 : -1];
#endif

extern Base Bases[64];
extern Base *BaseCurrent;
extern int BaseIDCurrentSelected;
extern int BaseComputedID;
extern Player Players[MaxPlayerNum];
extern PlayerData PlayersData[MaxPlayerNum];
extern RulesFacility Facility[MaxFacilityNum];
extern Map MapTiles[4096];
extern Veh Vehs[2049];
extern VehPrototype VehPrototypes[512];
extern RulesChassis Chassis[32];
extern int MapLongitudeBounds;
extern int MapLatitudeBounds;
extern uint32_t MapLongitude;
extern BOOL MapIsFlat;
extern const int RadiusOffsetX[21];
extern const int RadiusOffsetY[21];
extern int ExpansionEnabled;
extern Console ConsoleGlobal;
extern MapWin *MapWinTable[8];

inline void bitmask(int input, int *offset, int *mask) {
    *offset = input >> 3;
    *mask = 1 << (input & 7);
}
inline Map *map_loc(int x, int y) { return &MapTiles[(x >> 1) + y * MapLongitude]; }
inline BOOL is_alien_faction(uint32_t faction) {
    return ExpansionEnabled && (Players[faction].rule_flags & RFLAG_ALIEN);
}

void __cdecl draw_tile(int x, int y, int draw_type);
void __cdecl base_reset(int base_id, int force);
void __cdecl bitmask_call(int input, int *offset, int *mask);
BOOL __cdecl has_tech_call(int tech_id, int faction_id);
void __cdecl parse_say(int id, int input, int gender, int plurality);
void __cdecl parse_says(int id, LPCSTR input, int gender, int plurality);
int __cdecl popb(LPCSTR label, int x, int y, LPCSTR image, Sprite *callback);
int __cdecl popp(LPCSTR caption, LPCSTR label, int flags,
                 LPCSTR image, int (*callback)());
void __cdecl replay_base(int type, int x, int y, int faction_id);
void __cdecl using_set(int x, int y, int faction_id);
void __cdecl owner_set(int x, int y, int faction_id);
void __cdecl spot_base(int base_id, int faction_id);
void __cdecl kill(int vehicle_id);
int __cdecl propose_proto(int faction_id, int chassis, int weapon, int armor,
                          int abilities, int reactor, int category, char *name);
void __cdecl reset_territory();
void __cdecl base_nutrient();
void __cdecl base_minerals();
void __cdecl base_energy();
void __cdecl base_support();
void __cdecl base_yield();
void __cdecl draw_map(int draw_type);
extern "C" unsigned long __cdecl timeGetTime(void);

#endif
'''

DRONE_RIOT_FIXTURE = r'''#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "../drone_riot.v2.cpp"

Base Bases[64];
Base *BaseCurrent;
int BaseIDCurrentSelected;
int BaseComputedID;
Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
RulesFacility Facility[MaxFacilityNum];
Map MapTiles[4096];
Veh Vehs[2049];
VehPrototype VehPrototypes[512];
RulesChassis Chassis[32];
int MapLongitudeBounds;
int MapLatitudeBounds;
uint32_t MapLongitude;
BOOL MapIsFlat;
const int RadiusOffsetX[21] = {
    0, 1, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};
const int RadiusOffsetY[21] = {
    0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};
DroneRiotGameDataState GameDataStateStorage;
int ExpansionEnabled;
Console ConsoleGlobal;
MapWin *MapWinTable[8];

static unsigned long timer_value;
static int random_values[128];
static int random_count;
static int random_index;
static int random_fallback;
static int tech_enabled;
static int tech_calls;
static int tech_last_id;
static int tech_last_faction;
static int draw_calls;
static int popup_calls;
static char popup_label[32];
static int popup_reselect_base;
static int draw_reselect_base;
static int replay_reselect_base;
static int replay_calls;
static int replay_faction;
static int spot_calls;
static int spot_base_id;
static int spot_faction;
static int owner_calls;
static int owner_x[128];
static int owner_y[128];
static int owner_faction[128];
static int using_calls;
static int using_x[128];
static int using_y[128];
static int map_center_calls;
static int map_center_x;
static int map_center_y;
static int map_draw_calls;
static int parse_text_calls;
static int nutrient_calls;
static int mineral_calls;
static int energy_calls;
static int support_calls;
static int yield_calls;
static int resource_order[5];
static int resource_order_count;
static int reset_calls;
static int kill_calls;
static MapWin test_map_win;

extern "C" int rand(void) {
    if (random_index < random_count) return random_values[random_index++];
    return random_fallback;
}

extern "C" int abs(int value) { return value < 0 ? -value : value; }

extern "C" unsigned long timeGetTime(void) { return timer_value; }

void MapWin::set_center(int x, int y, int) {
    ++map_center_calls;
    map_center_x = x;
    map_center_y = y;
}

void __cdecl draw_tile(int, int, int) {
    ++draw_calls;
    if (draw_reselect_base >= 0) {
        BaseIDCurrentSelected = draw_reselect_base;
        BaseCurrent = &Bases[draw_reselect_base];
    }
}
void __cdecl base_reset(int, int) { ++reset_calls; }
void __cdecl bitmask_call(int input, int *offset, int *mask) {
    bitmask(input, offset, mask);
}
BOOL __cdecl has_tech_call(int tech_id, int faction_id) {
    ++tech_calls;
    tech_last_id = tech_id;
    tech_last_faction = faction_id;
    return tech_enabled;
}
void __cdecl parse_say(int, int, int, int) { ++parse_text_calls; }
void __cdecl parse_says(int, LPCSTR, int, int) { ++parse_text_calls; }
int __cdecl popb(LPCSTR label, int, int, LPCSTR, Sprite *) {
    ++popup_calls;
    strncpy(popup_label, label, sizeof(popup_label) - 1);
    popup_label[sizeof(popup_label) - 1] = 0;
    if (popup_reselect_base >= 0) {
        BaseIDCurrentSelected = popup_reselect_base;
        BaseCurrent = &Bases[popup_reselect_base];
    }
    return 0;
}
int __cdecl popp(LPCSTR, LPCSTR, int, LPCSTR, int (*)()) { return 0; }
void __cdecl replay_base(int, int, int, int faction) {
    ++replay_calls;
    replay_faction = faction;
    if (replay_reselect_base >= 0) {
        BaseIDCurrentSelected = replay_reselect_base;
        BaseCurrent = &Bases[replay_reselect_base];
    }
}
void __cdecl using_set(int x, int y, int faction) {
    using_x[using_calls] = x;
    using_y[using_calls] = y;
    ++using_calls;
    (void)faction;
}
void __cdecl owner_set(int x, int y, int faction) {
    owner_x[owner_calls] = x;
    owner_y[owner_calls] = y;
    owner_faction[owner_calls] = faction;
    ++owner_calls;
}
void __cdecl spot_base(int base_id, int faction) {
    ++spot_calls;
    spot_base_id = base_id;
    spot_faction = faction;
}
void __cdecl kill(int) { ++kill_calls; }
int __cdecl propose_proto(int, int, int, int, int, int, int, char *) {
    return -1;
}
void __cdecl reset_territory() { ++reset_calls; }
void __cdecl base_nutrient() { ++nutrient_calls; resource_order[resource_order_count++] = 3; }
void __cdecl base_minerals() { ++mineral_calls; resource_order[resource_order_count++] = 4; }
void __cdecl base_energy() { ++energy_calls; resource_order[resource_order_count++] = 5; }
void __cdecl base_support() { ++support_calls; resource_order[resource_order_count++] = 1; }
void __cdecl base_yield() { ++yield_calls; resource_order[resource_order_count++] = 2; }
void __cdecl draw_map(int) { ++map_draw_calls; }

static void set_random_values(const int *values, int count) {
    random_count = count;
    random_index = 0;
    for (int i = 0; i < count; ++i) random_values[i] = values[i];
    random_fallback = 5;
}

static void reset_fixture() {
    memset(Bases, 0, sizeof(Bases));
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Facility, 0, sizeof(Facility));
    memset(MapTiles, 0, sizeof(MapTiles));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Chassis, 0, sizeof(Chassis));
    memset(owner_x, 0, sizeof(owner_x));
    memset(owner_y, 0, sizeof(owner_y));
    memset(owner_faction, 0, sizeof(owner_faction));
    memset(using_x, 0, sizeof(using_x));
    memset(using_y, 0, sizeof(using_y));
    memset(popup_label, 0, sizeof(popup_label));
    BaseCurrent = &Bases[0];
    BaseIDCurrentSelected = 0;
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    GameDataStateStorage.fields_.base_current_count_ = 2;
    BaseComputedID = -1;
    Bases[0].x = 10;
    Bases[0].y = 10;
    Bases[0].faction_id_current = 1;
    Bases[0].faction_id_former = 2;
    Bases[0].population_size = 5;
    Bases[0].assimilation_turns_left = 0;
    Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
    Bases[0].queue_production_id[0] = -69;
    Bases[1].x = 20;
    Bases[1].y = 20;
    Bases[1].faction_id_current = 2;
    Bases[1].population_size = 4;
    Bases[1].visibility = static_cast<uint8_t>(1 << 7);
    PlayersData[1].soc_effect_pending.values_[SocialEffect::POLICE] = -3;
    PlayersData[1].current_num_bases = 2;
    PlayersData[1].proto_id_active[10] = 1;
    Players[3].faction_bonus_count = 1;
    Players[3].faction_bonus_id[0] = 13;
    Players[3].faction_bonus_val1[0] = 60;
    // Native reads all seven target slots; initialize otherwise absent
    // type-13 bonuses in this deterministic host fixture.
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if (Players[faction].faction_bonus_count == 0) {
            Players[faction].faction_bonus_count = 1;
            Players[faction].faction_bonus_id[0] = 13;
            Players[faction].faction_bonus_val1[0] = 0;
        }
    }
    Facility[5].name = const_cast<char *>("fixture specialist");
    Facility[5].preq_tech = 42;
    MapLongitudeBounds = 32;
    MapLatitudeBounds = 32;
    MapLongitude = 16;
    MapIsFlat = 1;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0x1234;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ =
        static_cast<uint32_t>((1 << 1) | (1 << 2) | (1 << 3));
    ExpansionEnabled = 0;
    ConsoleGlobal.faction_id_ = 7;
    Bases[0].visibility = static_cast<uint8_t>(1 << 7);
    MapWinTable[0] = &test_map_win;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    timer_value = 1039;
    tech_enabled = 0;
    tech_calls = 0;
    tech_last_id = -1;
    tech_last_faction = -1;
    draw_calls = popup_calls = 0;
    popup_reselect_base = draw_reselect_base = replay_reselect_base = -1;
    replay_calls = spot_calls = owner_calls = using_calls = 0;
    spot_base_id = spot_faction = -1;
    map_center_calls = map_draw_calls = parse_text_calls = 0;
    nutrient_calls = mineral_calls = energy_calls = 0;
    support_calls = yield_calls = reset_calls = kill_calls = 0;
    resource_order_count = 0;
    const int values[] = { 0 };
    set_random_values(values, 1);
}

static void test_weighted_threshold_and_radius_transfer() {
    const unsigned long ticks[] = { 1039, 1040, 1041 };
    const int expected_targets[] = { 3, 3, 2 };
    for (int test = 0; test < 3; ++test) {
        reset_fixture();
        timer_value = ticks[test];
        GameDataStateStorage.fields_.vehicle_current_count_ = 2;
        Vehs[0].x = 11;
        Vehs[0].y = 10;
        Vehs[0].proto_id = 10;
        Vehs[0].faction_id = 1;
        Vehs[0].home_base_id = 0;
        Vehs[0].order = ORDER_HOLD;
        Vehs[1].x = 11;
        Vehs[1].y = 10;
        Vehs[1].proto_id = 10;
        Vehs[1].faction_id = static_cast<uint8_t>(expected_targets[test]);
        Vehs[1].state = VSTATE_UNK_40000;
        replay_reselect_base = 1;
        drone_riot();
        const int target = expected_targets[test];
        assert(Bases[0].faction_id_current == target);
        assert(Vehs[0].faction_id == target); // noncenter radius tile
        assert((Vehs[1].state & VSTATE_UNK_40000) == 0); // already-owned unit
        assert(Vehs[0].morale == 0x34 && Vehs[0].terraforming_turns == 0x12);
        assert(spot_calls == 1 && spot_base_id == 1 && spot_faction == target);
        assert(BaseIDCurrentSelected == 1 && BaseCurrent == &Bases[1]);
        assert(BaseComputedID == 1);
        assert(owner_calls >= 2);
        assert(owner_x[owner_calls - 1] == 10 && owner_y[owner_calls - 1] == 10);
        assert(owner_faction[owner_calls - 1] == target);
        assert(map_center_calls == 1 && map_center_x == 10 && map_center_y == 10);
        assert(using_calls == 2);
        assert(nutrient_calls == 1 && mineral_calls == 1 && energy_calls == 1);
        assert(support_calls == 1 && yield_calls == 1);
        const int expected_order[] = { 1, 2, 3, 4, 5 };
        assert(resource_order_count == 5);
        for (int i = 0; i < 5; ++i) assert(resource_order[i] == expected_order[i]);
    }

    reset_fixture();
    Bases[1].visibility = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    Vehs[0].x = 11;
    Vehs[0].y = 10;
    Vehs[0].proto_id = 10;
    Vehs[0].faction_id = 1;
    replay_reselect_base = 1;
    timer_value = 1041;
    drone_riot();
    assert(replay_calls == 1 && Bases[0].faction_id_current == 2);
    assert(map_center_calls == 0); // unrelated, unseen base is not a UI gate
}

static void test_signed_former_faction_sentinel() {
    reset_fixture();
    Bases[0].faction_id_former = 0xff;
    timer_value = 1050;
    drone_riot();
    assert(Bases[0].faction_id_current == 1);
    assert(replay_calls == 0);
    assert(owner_calls == 0 && using_calls == 0);
}

static void test_nearest_fallback_treaty_weight() {
    reset_fixture();
    Bases[0].faction_id_former = 1;
    GameDataStateStorage.fields_.base_current_count_ = 3;
    Bases[1].faction_id_current = 2;
    Bases[1].x = 13;
    Bases[1].y = 10;
    Bases[2].faction_id_current = 3;
    Bases[2].x = 14;
    Bases[2].y = 10;
    PlayersData[1].diplo_treaties[2] = DTREATY_PACT;
    Players[7].faction_bonus_count = 1;
    Players[7].faction_bonus_id[0] = 13;
    Players[7].faction_bonus_val1[0] = 100;
    timer_value = 1050; // cutoff=0: retain treaty-adjusted nearest fallback
    drone_riot();
    assert(Bases[0].faction_id_current == 2);
    assert(replay_calls == 1 && replay_faction == 2);
}

static void test_initial_treaty_index_reads_last_turn_member() {
    reset_fixture();
    Bases[0].faction_id_former = 1;
    GameDataStateStorage.fields_.base_current_count_ = 3;
    Bases[1].faction_id_current = 2;
    Bases[1].x = 13;
    Bases[1].y = 10;
    Bases[2].faction_id_current = 3;
    Bases[2].x = 14;
    Bases[2].y = 10;
    PlayersData[1].last_turn_new_base = DTREATY_PACT;
    timer_value = 1050;
    drone_riot();
    // The sentinel -1 reads the preceding member's pact bit: faction 2's
    // initial score doubles, allowing faction 3's score to replace it.
    assert(Bases[0].faction_id_current == 3);
    assert(replay_calls == 1 && replay_faction == 3);
}

static void test_duplicate_bonus_uses_first_match() {
    reset_fixture();
    Bases[0].faction_id_former = 1;
    GameDataStateStorage.fields_.base_current_count_ = 4;
    Bases[1].faction_id_current = 2;
    Bases[1].x = 13;
    Bases[1].y = 10;
    Bases[2].faction_id_current = 3;
    Bases[2].x = 14;
    Bases[2].y = 10;
    Bases[3].faction_id_current = 4;
    Bases[3].x = 15;
    Bases[3].y = 10;
    // Initialize each native weight slot through a real type-13 record; the
    // pinned executable leaves slots without a match indeterminate.
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        Players[faction].faction_bonus_count = 1;
        Players[faction].faction_bonus_id[0] = 13;
        Players[faction].faction_bonus_val1[0] = 0;
    }
    Players[2].faction_bonus_count = 2;
    Players[2].faction_bonus_id[1] = 13;
    Players[2].faction_bonus_val1[1] = 100;
    Players[3].faction_bonus_val1[0] = 1;
    Bases[0].queue_production_id[1] = 0x1234;
    timer_value = 0;
    drone_riot();
    assert(Bases[0].queue_production_id[1] == 0x1234);
    assert(Bases[0].faction_id_current == 3);
    assert(replay_calls == 1 && replay_faction == 3);
}

static void test_draw_and_popup_selection_reloads() {
    reset_fixture();
    Bases[0].state = 0;
    Bases[0].governor = 0;
    Bases[1].governor = 0x80000040u;
    draw_reselect_base = 1;
    drone_riot();
    assert(draw_calls == 1);
    assert(strcmp(popup_label, "DRONERIOTS2") == 0);
    assert(BaseCurrent == &Bases[1] && BaseIDCurrentSelected == 1);
    assert(nutrient_calls == 0 && replay_calls == 0);

    reset_fixture();
    popup_reselect_base = 1;
    drone_riot();
    assert(strcmp(popup_label, "DRONERIOTS") == 0);
    assert(BaseCurrent == &Bases[1] && BaseIDCurrentSelected == 1);
    assert(nutrient_calls == 0 && replay_calls == 0);
}

static void test_visibility_gate_and_specialist_prerequisite() {
    reset_fixture();
    Bases[0].facilities_built[0] = static_cast<uint8_t>(1 << 5);
    Players[1].faction_bonus_count = 1;
    Players[1].faction_bonus_id[0] = 2;
    Players[1].faction_bonus_val1[0] = 5;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::POLICE] = 0;
    const int bonus_two_values[] = { 0, 5 };
    set_random_values(bonus_two_values, 2);
    drone_riot();
    assert((Bases[0].facilities_built[0] & (1 << 5)) != 0);
    assert(replay_calls == 0);
    assert(strcmp(popup_label, "DRONERIOTS") == 0);

    reset_fixture();
    Bases[0].facilities_built[0] = static_cast<uint8_t>(1 << 5);
    Players[1].faction_bonus_count = 1;
    Players[1].faction_bonus_id[0] = 12;
    Players[1].faction_bonus_val1[0] = 5;
    tech_enabled = 1;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::POLICE] = 0;
    timer_value = 1050;
    const int blocked_values[] = { 0, 5 };
    set_random_values(blocked_values, 2);
    drone_riot();
    assert((Bases[0].facilities_built[0] & (1 << 5)) != 0);
    assert(tech_calls > 0 && tech_last_id == 42 && tech_last_faction == 1);
    assert(replay_calls == 0);

    reset_fixture();
    Bases[0].facilities_built[0] = static_cast<uint8_t>(1 << 5);
    Players[1].faction_bonus_count = 1;
    Players[1].faction_bonus_id[0] = 12;
    Players[1].faction_bonus_val1[0] = 5;
    tech_enabled = 0;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::POLICE] = 0;
    const int converted_values[] = { 0, 5 };
    set_random_values(converted_values, 2);
    drone_riot();
    assert((Bases[0].facilities_built[0] & (1 << 5)) == 0);
    assert(strcmp(popup_label, "DRONERIOT") == 0);
    assert(replay_calls == 0);
}

static void test_bonus_facilities_come_only_from_both_participants() {
    reset_fixture();
    Bases[0].faction_id_former = 2;
    Players[1].faction_bonus_count = 2;
    Players[1].faction_bonus_id[1] = 2;
    Players[1].faction_bonus_val1[1] = 5;
    Players[2].faction_bonus_count = 2;
    Players[2].faction_bonus_id[1] = 2;
    Players[2].faction_bonus_val1[1] = 6;
    Players[3].faction_bonus_count = 2;
    Players[3].faction_bonus_id[1] = 2;
    Players[3].faction_bonus_val1[1] = 7;
    timer_value = 1041; // retain the original former-faction fallback
    drone_riot();
    assert(Bases[0].faction_id_current == 2);
    assert((Bases[0].facilities_built[0] & (1 << 5)) != 0);
    assert((Bases[0].facilities_built[0] & (1 << 6)) != 0);
    assert((Bases[0].facilities_built[0] & (1 << 7)) == 0);
}

int main() {
    test_weighted_threshold_and_radius_transfer();
    test_bonus_facilities_come_only_from_both_participants();
    test_signed_former_faction_sentinel();
    test_nearest_fallback_treaty_weight();
    test_initial_treaty_index_reads_last_turn_member();
    test_duplicate_bonus_uses_first_match();
    test_draw_and_popup_selection_reloads();
    test_visibility_gate_and_specialist_prerequisite();
    puts("drone_riot fixture controls passed");
    return 0;
}
'''


def _masked_cpp(text: str) -> str:
    """Mask comments and quoted tokens while preserving offsets and newlines."""
    out = list(text)
    i, state = 0, "code"
    while i < len(text):
        c = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
            elif c == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
            elif c in ('"', "'"):
                out[i] = " "
                state = "string" if c == '"' else "char"
                i += 1
            else:
                i += 1
            continue
        if state == "line":
            if c == "\n":
                state = "code"
            else:
                out[i] = " "
            i += 1
            continue
        if state == "block":
            if c == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
            else:
                if c != "\n":
                    out[i] = " "
                i += 1
            continue
        if c == "\\":
            out[i] = " "
            if i + 1 < len(text):
                out[i + 1] = " "
            i += 2
        elif (state == "string" and c == '"') or (state == "char" and c == "'"):
            out[i] = " "
            i += 1
            state = "code"
        else:
            if c != "\n":
                out[i] = " "
            i += 1
    assert state not in ("block", "string", "char"), "unterminated C++ token"
    return "".join(out)


def _extract_definition(text: str, signature: str) -> str:
    """Extract one typed definition with lexical C++ brace balance."""
    masked = _masked_cpp(text)
    match = re.search(re.escape(signature), masked)
    assert match is not None, "missing typed definition " + signature
    open_paren = masked.find("(", match.start())
    depth = 0
    close_paren = -1
    for pos in range(open_paren, len(masked)):
        if masked[pos] == "(":
            depth += 1
        elif masked[pos] == ")":
            depth -= 1
            if depth == 0:
                close_paren = pos
                break
    assert close_paren >= 0, "unterminated signature " + signature
    brace = close_paren + 1
    while brace < len(masked) and masked[brace].isspace():
        brace += 1
    assert brace < len(masked) and masked[brace] == "{", "expected definition " + signature
    depth = 0
    for pos in range(brace, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():pos + 1]
    raise AssertionError("unclosed definition " + signature)


def _actual_helper(owner: str, signature: str) -> str:
    return _extract_definition((ROOT / owner).read_text(encoding="utf-8"), signature)


def owner_section(address: str, signature: str, owner: str) -> str:
    parsed_address = int(address, 16)
    records = [record for record in read_file(ROOT / owner)
               if record.address == parsed_address]
    assert len(records) == 1, f"{owner}: expected one source-reader record {address}"
    record = records[0]
    assert record.region, f"{owner}: source reader could not extract {address}: {record.extract_error}"
    body = _extract_definition(record.region, signature)
    assert "BODY GOES HERE." not in body
    assert "ORIGINAL:" not in body
    helper_signatures = {
        0x00526E70: (
            "static int production_wrap_add(int",
            "static int production_wrap_subtract(int",
            "static int production_wrap_double(int",
        ),
        0x00527290: (
            "static int upkeep_wrap_add(int",
            "static int upkeep_wrap_subtract(int",
        ),
        0x004F79C0: (
            "static int base_upkeep_wrap_signed_bits(uint32_t",
            "static int base_upkeep_wrap_add(int",
        ),
        0x004F4FB0: (
            "static unsigned char faction_bit(int",
            "static void drone_riot_reassign_vehicles_on_tile(int",
        ),
    }
    helpers = "\n".join(
        _actual_helper(owner, item)
        for item in helper_signatures.get(parsed_address, ())
    )
    return helpers + "\n" + body


def compiler() -> str:
    path = shutil.which("clang++") or shutil.which("g++")
    if path is None:
        pytest.skip("host C++ compiler unavailable")
    return path


def compile_run(tmp_path: Path, name: str, source: str, *defines: str) -> None:
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    command = [compiler(), "-std=c++98", "-Wall", "-Wextra", "-Werror"]
    command.extend("-D" + item for item in defines)
    command.extend([str(cpp), "-o", str(exe)])
    built = subprocess.run(command, cwd=tmp_path, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], cwd=tmp_path, capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_turn_owner_bodies_against_embedded_retry_loan_and_upkeep_controls(tmp_path: Path) -> None:
    host = re.sub(r"(?ms)^#ifdef PRODUCTION_PHASE_V2\n.*?^#endif\n#ifdef FACTION_UPKEEP_V2\n.*?^#endif\n?$", "", HOST_FIXTURE)
    turn = TURN_FIXTURE.replace('#include "host_syntax_control.cpp"\n', "")
    source = (host + "\n" +
              owner_section("0x00526E70", "void __cdecl production_phase(int", "src/faction.cpp") +
              "\n" +
              owner_section("0x00527290", "void __cdecl faction_upkeep(int", "src/faction.cpp") +
              "\n" + turn)
    compile_run(tmp_path, "actual_turn_chain", source, "PRODUCTION_PHASE_V2",
                "FACTION_UPKEEP_V2")


def test_base_upkeep_actual_body_role_and_damage_controls(tmp_path: Path) -> None:
    harness = BASE_UPKEEP_FIXTURE
    body = owner_section("0x004F79C0", "int __cdecl base_upkeep(int", "src/base.cpp")
    harness = harness.replace("int focus_calls = 0;", "int focus_calls; TestConsole() : focus_calls(0) {}")
    harness = harness.replace("int redraws = 0, zooms = 0;", "int redraws, zooms; TestBaseWin() : redraws(0), zooms(0) {}")
    harness = re.sub(r"(?ms)^#ifdef BASE_UPKEEP_CONTROL_V2\n.*?^#endif\n", body + "\n", harness, count=1)
    compile_run(tmp_path, "actual_base_upkeep", harness,
                "BASE_UPKEEP_CONTROL_V2", "BASE_UPKEEP_COMMERCE_ROLE_ASSERTIONS")


def test_base_upkeep_actual_body_reloads_after_mutating_callbacks(tmp_path: Path) -> None:
    harness = BASE_UPKEEP_FIXTURE
    harness = harness.replace(
        "struct TestConsole {",
        """struct Base;
extern Base Bases[8];
extern Base *BaseCurrent;
int callback_focus_switch = -1, callback_draw_switch = -1;
int callback_parse_switch = -1, callback_name_switch = -1;
int callback_popup_switch = -1, callback_focus_result = 0;
int callback_focus_x, callback_focus_y, callback_draw_x, callback_draw_y;
const char *callback_says_one, *callback_says_two, *callback_name_arg;
struct TestConsole {""",
        1,
    )
    harness = harness.replace(
        "int focus(int, int, int) { ++focus_calls; return 1; }",
        """int focus(int x, int y, int) {
        ++focus_calls; callback_focus_x=x; callback_focus_y=y;
        if (callback_focus_switch >= 0) BaseCurrent=&Bases[callback_focus_switch];
        return callback_focus_result;
    }""",
        1,
    )
    harness = harness.replace(
        "int parse_says(int, const char *, int, int) { ++control_parse_calls; return 0; }",
        """int parse_says(int what, const char *text, int, int) {
    ++control_parse_calls;
    if (what == 1) callback_says_one=text;
    if (what == 2) callback_says_two=text;
    if (what == 0 && callback_parse_switch >= 0) BaseCurrent=&Bases[callback_parse_switch];
    return 0;
}""",
        1,
    )
    harness = harness.replace(
        "void name_base(int, char *, int, int) { ++control_name_calls; }",
        """void name_base(int, char *name, int, int) {
    ++control_name_calls; callback_name_arg=name;
    if (callback_name_switch >= 0) BaseCurrent=&Bases[callback_name_switch];
}""",
        1,
    )
    harness = harness.replace(
        "int draw_tiles(int, int, int) { return 0; }",
        """int draw_tiles(int x, int y, int) {
    callback_draw_x=x; callback_draw_y=y;
    if (callback_draw_switch >= 0) BaseCurrent=&Bases[callback_draw_switch];
    return 0;
}""",
        1,
    )
    harness = harness.replace(
        "    control_popup_picture = picture;\n    return 0;",
        "    control_popup_picture = picture;\n"
        "    if (callback_popup_switch >= 0) BaseCurrent = &Bases[callback_popup_switch];\n"
        "    return 0;",
        1,
    )
    marker = "int main() {\n"
    callback_case = """int main() {
#ifdef BASE_UPKEEP_CALLBACK_RELOADS
    reset_fixture();
    TurnCurrentNum=2;
    for (int i=0;i<6;++i) { Bases[i].x=2*i+2; Bases[i].y=i+1; }
    Bases[0].faction_id_current=1;
    Bases[0].state |= 0x10000u;
    strcpy(Bases[0].name_string,"selected");
    strcpy(Bases[1].name_string,"focus");
    strcpy(Bases[2].name_string,"draw");
    strcpy(Bases[3].name_string,"parser");
    strcpy(Bases[4].name_string,"renamed");
    callback_focus_result=0; callback_focus_switch=1; callback_draw_switch=2;
    callback_parse_switch=3; callback_name_switch=4; callback_popup_switch=5;
    MapTiles[3*MapLongitude+(8>>1)].climate=0x80;
    assert(base_upkeep(0)==0);
    assert(callback_focus_x==2 && callback_focus_y==1);
    assert(callback_draw_x==4 && callback_draw_y==2);
    assert(callback_says_one==Bases[2].name_string);
    assert(callback_name_arg==Bases[3].name_string);
    assert(callback_says_two==Bases[4].name_string);
    assert(BaseCurrent==&Bases[5]);
    assert((Bases[5].state & 0x80u)!=0);
    assert((Bases[0].state & 0x10000u)!=0);
    return 0;
#endif
"""
    assert marker in harness
    harness = harness.replace(marker, callback_case, 1)
    body = owner_section("0x004F79C0", "int __cdecl base_upkeep(int", "src/base.cpp")
    harness = harness.replace("int focus_calls = 0;", "int focus_calls; TestConsole() : focus_calls(0) {}")
    harness = harness.replace("int redraws = 0, zooms = 0;", "int redraws, zooms; TestBaseWin() : redraws(0), zooms(0) {}")
    harness = re.sub(r"(?ms)^#ifdef BASE_UPKEEP_CONTROL_V2\n.*?^#endif\n", body + "\n", harness, count=1)
    compile_run(tmp_path, "actual_base_upkeep_callback_reloads", harness,
                "BASE_UPKEEP_CALLBACK_RELOADS", "BASE_UPKEEP_COMMERCE_ROLE_ASSERTIONS")


def test_base_drones_actual_body_and_callback_latches(tmp_path: Path) -> None:
    harness = BASE_DRONES_FIXTURE
    body = owner_section("0x004F5CE0", "void __cdecl base_drones()", "src/base.cpp")
    harness = harness.replace('#include "base_drones.v1.cpp"', body)
    compile_run(tmp_path, "actual_base_drones", harness)


def test_base_drones_control_rejects_reversed_reset_arguments(tmp_path: Path) -> None:
    body = owner_section("0x004F5CE0", "void __cdecl base_drones()", "src/base.cpp")
    correct = "base_reset(BaseIDCurrentSelected, 0);"
    assert body.count(correct) == 1
    damaged = body.replace(correct, "base_reset(0, BaseIDCurrentSelected);")
    harness = BASE_DRONES_FIXTURE.replace('#include "base_drones.v1.cpp"', damaged)
    with pytest.raises(AssertionError, match="reset_base_id == 3"):
        compile_run(tmp_path, "reversed_base_drones", harness)


@pytest.mark.parametrize("host_compiler", ["g++", "clang++"])
def test_drone_riot_actual_body_embedded_cpp98_controls(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, host_compiler: str
) -> None:
    executable = shutil.which(host_compiler)
    if executable is None:
        pytest.skip(f"{host_compiler} is unavailable")
    monkeypatch.setitem(globals(), "compiler", lambda: executable)
    body = owner_section("0x004F4FB0", "void __cdecl drone_riot()", "src/base.cpp")
    harness = DRONE_RIOT_FIXTURE
    harness = harness.replace('#include "api.h"\n#include "../drone_riot.v2.cpp"', "")
    harness = DRONE_API_FIXTURE + "\n" + body + "\n" + harness
    compile_run(tmp_path, "actual_drone_riot", harness)

def test_drone_riot_old_resource_order_mutation_is_rejected(tmp_path: Path) -> None:
    body = owner_section("0x004F4FB0", "void __cdecl drone_riot()", "src/base.cpp")
    expected = """base_support();
                    base_yield();
                    base_nutrient();
                    base_minerals();
                    base_energy();"""
    old_order = """base_nutrient();
                    base_minerals();
                    base_energy();
                    base_support();
                    base_yield();"""
    assert expected in body
    damaged = body.replace(expected, old_order, 1)
    harness = DRONE_RIOT_FIXTURE.replace('#include "api.h"\n#include "../drone_riot.v2.cpp"', "")
    harness = DRONE_API_FIXTURE + "\n" + damaged + "\n" + harness
    with pytest.raises(AssertionError):
        compile_run(tmp_path, "old_resource_order", harness)

def test_drone_riot_intervening_faction_bonus_mutation_is_rejected(tmp_path: Path) -> None:
    body = owner_section("0x004F4FB0", "void __cdecl drone_riot()", "src/base.cpp")
    participants = """for (int bonus_pass = 0; bonus_pass < 2; ++bonus_pass) {
                        const int bonus_player_id =
                            bonus_pass == 0 ? faction_id : best_faction;"""
    all_later_players = """for (int bonus_player_id = faction_id;
                         bonus_player_id < MaxPlayerNum; ++bonus_player_id) {"""
    assert participants in body
    damaged = body.replace(participants, all_later_players, 1)
    harness = DRONE_RIOT_FIXTURE.replace('#include "api.h"\n#include "../drone_riot.v2.cpp"', "")
    harness = DRONE_API_FIXTURE + "\n" + damaged + "\n" + harness
    with pytest.raises(AssertionError):
        compile_run(tmp_path, "intervening_bonus_owner", harness)
