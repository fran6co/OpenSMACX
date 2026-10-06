from pathlib import Path
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_body(signature: str) -> str:
    source = (ROOT / "src/leaf_recoveries.cpp").read_text()
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated source body: {signature}")


BODY = source_body("int __cdecl wants_prop(int faction, int proposal, int option)")
ARITHMETIC_HELPERS = "\n".join(
    source_body(signature)
    for signature in (
        "static int wants_prop_signed_difference(",
        "static int wants_prop_signed_sum(",
        "static int wants_prop_signed_product(",
        "static int wants_prop_signed_add(",
    )
)


HARNESS = r'''
#include <cstdint>
#include <cassert>
#include <cstring>
#define __cdecl
const int MaxPlayerNum = 8;
const int PROP_SALVAGE_UNITY_CORE = 2;
const int PROP_GLOBAL_TRADE_PACT = 3;
const int PROP_REPEAL_GLOBAL_TRADE_PACT = 4;
const int PROP_REPEAL_UN_CHARTER = 8;
const int PROP_REINSTATE_UN_CHARTER = 9;
const int PROP_LAUNCH_SOLAR_SHADE = 5;
const int PROP_INCREASE_SOLAR_SHADE = 6;
const int PROP_MELT_POLAR_CAPS = 7;
const int PROP_UNITE_SUPREME_LEADER = 1;
const int DTREATY_PACT = 1;
const int DTREATY_ATROCITY_VICTIM = 0x40000;
const int DTREATY_VENDETTA = 0x10;
const int DTREATY_WANT_REVENGE = 0x20;
const int DTREATY_HAVE_SURRENDERED = 0x2000000;
const int RULES_VICTORY_COOPERATIVE = 0x1000;
const int RFLAG_ALIEN = 0x80;
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    int diff_level;
    uint32_t maint_cost_total;
    int ai_fight;
    int ai_power;
    int ai_wealth;
    int energy_reserves;
    int unk_13;
    int tech_commerce_bonus;
    int soc_effect_pending_value;
    int ai_tech;
    int ai_growth;
    int planet_busters;
    int atrocities;
    uint32_t diplo_treaties[8];
    SocialEffect soc_effect_pending;
    int pop_total;
    int ranking;
    int mil_strength_1;
    int mil_strength_2;
    struct Goal { int16_t type, priority; int x, y, base_id; } goals[75];
    uint8_t region_base_plan[128];
    struct PendingSocialCategory { uint32_t models_[4]; } soc_category_pending;
};
PlayerData PlayersData[MaxPlayerNum] = {};
struct RulesChassis { short preq_tech; };
struct RulesWeapon { short preq_tech; };
RulesChassis Chassis[9] = {};
RulesWeapon Weapon[17] = {};
int known_tech[MaxPlayerNum][16] = {};
int has_tech(int tech, int faction) { return known_tech[faction][tech]; }
struct Player {
    int rule_psi;
    int rule_flags;
    int soc_ideology_category, soc_anti_ideology_category;
    int soc_ideology_model, soc_anti_ideology_model;
};
struct Base {
    short x, y;
    unsigned char faction_id_current, faction_id_former;
    signed char population_size;
    unsigned char assimilation_turns_left, nerve_staple_turns_left;
    unsigned char unk_1, visibility;
    unsigned char faction_pop_size_intel[8];
    char name_string[25];
    short unk_x, unk_y;
    uint32_t state, event, governor;
    int nutrients_accumulated, minerals_accumulated, production_id_last;
    int eco_damage;
    uint32_t queue_size;
    int queue_production_id[10];
    int resource_sqr_radius, specialist_total, specialist_unk_1;
    uint32_t spec[2];
    unsigned char facilities_built[12];
};
struct Map {
    unsigned char climate, contour, val2, region;
};
struct RulesWorldbuilder { uint32_t sea_level_rises; };
Player Players[MaxPlayerNum] = {};
int ExpansionEnabled = 0;
uint32_t LockEnableMask = 0;
int GovernorFaction = -1;
int IsMultiplayerNet = 0;
uint32_t RankingFactionIDUnk1 = 0;
uint32_t FactionRankings[MaxPlayerNum] = {};
int TurnCurrentNum = 0;
Base Bases[512] = {};
Map map_storage[256] = {};
Map *MapTiles = map_storage;
Map *&map_tiles() { return MapTiles; }
int BaseCurrentCount = 0;
uint32_t MapLongitude = 16;
int MapSeaLevel = 0, MapSeaLevelCouncil = 0;
uint32_t AltNatural[11] = {};
RulesWorldbuilder WorldBuilder = {};
uint32_t GameRules = 0;
int vote_counts[MaxPlayerNum] = {};
int council_vote_calls = 0;
int council_votes(int faction) { ++council_vote_calls; return vote_counts[faction]; }
int great_satan_result[8] = {};
int wants_to_attack_result[8][8] = {};
int wants_to_attack_calls = 0;
int attack_mutates_great_satan = 0;
int attack_mutates_human_mask = 0;
int wants_to_attack(int faction, int target, int) {
    ++wants_to_attack_calls;
    if (attack_mutates_human_mask != 0 && faction == 0 && target == 1) {
        LockEnableMask |= 1u << target;
    }
    if (attack_mutates_great_satan != 0 && faction == 0 && target == 1) {
        great_satan_result[target] = 1;
    }
    return wants_to_attack_result[faction][target];
}
int great_satan_calls = 0;
int great_satan(int faction, int) {
    ++great_satan_calls;
    return great_satan_result[faction];
}
struct CouncilWindowStub { int leader_label_kind_[8]; } CouncilWindow = {};
int bit_count(int value) {
    unsigned int bits = static_cast<unsigned int>(value);
    int count = 0;
    while (bits != 0) { count += bits & 1u; bits >>= 1; }
    return count;
}
'''


def compile_and_run(body: str) -> None:
    program = HARNESS + "\n" + ARITHMETIC_HELPERS + "\n" + body + r'''
int main() {
    std::memset(PlayersData, 0, sizeof(PlayersData));
    PlayersData[1].ai_power = 1;
    PlayersData[1].ai_fight = -1;
    PlayersData[1].energy_reserves = 150;
    for (int faction = 2; faction < MaxPlayerNum; ++faction) {
        PlayersData[faction].energy_reserves = 200;
        PlayersData[faction].maint_cost_total = 100;
        PlayersData[faction].unk_13 = 200;
    }
    assert(wants_prop(1, PROP_SALVAGE_UNITY_CORE, 0) == 7);

    for (int faction = 2; faction < MaxPlayerNum; ++faction) {
        PlayersData[faction].energy_reserves = 100;
        PlayersData[faction].maint_cost_total = 0;
    }
    PlayersData[1].unk_13 = 100;
    assert(wants_prop(1, PROP_SALVAGE_UNITY_CORE, 0) == 1);

    assert(wants_prop_signed_difference(0, 1u) == -1);
    assert(wants_prop_signed_difference(0x7fffffff, 0xffffffffu) == (int)0x80000000u);

    std::memset(PlayersData, 0, sizeof(PlayersData));
    PlayersData[0].tech_commerce_bonus = 2;
    PlayersData[0].ai_wealth = 1;
    PlayersData[0].ai_power = 1;
    PlayersData[0].ai_fight = 1;
    PlayersData[0].soc_effect_pending.values_[SocialEffect::ECONOMY] = 2;
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        PlayersData[faction].tech_commerce_bonus = 3;
        PlayersData[0].diplo_treaties[faction] = 3;
    }
    assert(wants_prop(0, PROP_GLOBAL_TRADE_PACT, 0) == 7);
    assert(wants_prop(0, PROP_REPEAL_GLOBAL_TRADE_PACT, 0) == -7);

    // Reversing the treaty direction must remove the contribution: native
    // indexes actor-row + peer, not peer-row + actor.
    std::memset(PlayersData[0].diplo_treaties, 0,
                sizeof(PlayersData[0].diplo_treaties));
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        PlayersData[faction].diplo_treaties[0] = 3;
    }
    assert(wants_prop(0, PROP_GLOBAL_TRADE_PACT, 0) == -7);
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        PlayersData[faction].diplo_treaties[0] = 0;
        PlayersData[0].diplo_treaties[faction] = 3;
    }

    // Proposal 0's early native gates reject before relationship scoring.
    std::memset(vote_counts, 0, sizeof(vote_counts));
    LockEnableMask = 0;
    council_vote_calls = 0;
    assert(wants_prop(0, 0, 1) == -9);
    assert(council_vote_calls == 0);

    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8;
    ExpansionEnabled = 1;
    Players[1].rule_flags = RFLAG_ALIEN;
    council_vote_calls = 0;
    assert(wants_prop(0, 0, 1) == -9);
    assert(council_vote_calls == 0);
    ExpansionEnabled = 0;

    // With expansion disabled the native branch skips the alien test and
    // proceeds to council_votes; it does not compare the flag to option id.
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8;
    council_vote_calls = 0;
    (void)wants_prop(0, 0, 1);
    assert(council_vote_calls == 17);

    ExpansionEnabled = 1;
    vote_counts[1] = 1;
    vote_counts[2] = 1;
    vote_counts[3] = 2;
    council_vote_calls = 0;
    assert(wants_prop(0, 0, 1) == -9);
    // Native 0x52AEF4 tests only ExpansionEnabled. With it enabled, an
    // alien option is rejected before council_votes, even if its id is 1.
    assert(council_vote_calls == 0);
    ExpansionEnabled = 0;
    Players[1].rule_flags = 0;

    // Native 0x52AF52..5A rejects when at least two living factions have
    // more votes than the option; zero higher-vote factions proceeds.
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8 | (1u << 1);
    vote_counts[1] = 5;
    vote_counts[2] = 1;
    vote_counts[3] = 1;
    GovernorFaction = -1;
    assert(wants_prop(1, 0, 1) == 13);
    vote_counts[1] = 1;
    vote_counts[2] = 2;
    vote_counts[3] = 3;
    council_vote_calls = 0;
    assert(wants_prop(1, 0, 1) == -9);
    assert(council_vote_calls == 3);

    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(vote_counts, 0, sizeof(vote_counts));
    std::memset(CouncilWindow.leader_label_kind_, 0,
                sizeof(CouncilWindow.leader_label_kind_));
    LockEnableMask = (1u << 1) << 8 | (1u << 1);
    vote_counts[1] = 1;
    vote_counts[2] = 2;
    vote_counts[3] = 3;
    GovernorFaction = -1;
    assert(wants_prop(1, 0, 1) == 9); // self-option: five, then native 2*x-1
    CouncilWindow.leader_label_kind_[1] = 2;
    assert(wants_prop(1, 0, 1) == 9); // self leader returns before decrement
    CouncilWindow.leader_label_kind_[1] = 0;

    // Native computes this actor-relative count before the self/nonself split.
    // Candidate screening costs three vote calls here; actor comparison then
    // reads both votes for all seven numbered factions because actor is 0.
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8 | 1u;
    vote_counts[0] = 1;
    vote_counts[1] = 0;
    vote_counts[2] = 0;
    vote_counts[3] = 0;
    Players[0].soc_ideology_category = -1;
    Players[0].soc_anti_ideology_category = -1;
    council_vote_calls = 0;
    (void)wants_prop(0, 0, 1);
    assert(council_vote_calls == 17);

    // A reverse-only actor/option treaty flag is not read by native here.
    std::memset(PlayersData, 0, sizeof(PlayersData));
    Players[0].soc_ideology_category = -1;
    Players[0].soc_anti_ideology_category = -1;
    Players[1].soc_ideology_category = -1;
    Players[1].soc_anti_ideology_category = 1;
    PlayersData[0].ai_fight = 0;
    PlayersData[1].ai_fight = 0;
    PlayersData[1].diplo_treaties[0] = 2u;
    great_satan_result[1] = 1;
    wants_to_attack_calls = 0;
    assert(wants_prop(0, 0, 1) == -2);
    assert(wants_to_attack_calls == 0);
    great_satan_result[1] = 0;

    // Case 0's difficulty penalties are gated by the option's live human bit.
    // The top-ranked human option at high difficulty skips its +1 preference,
    // then the score and difficulty gates independently decrement it.
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(vote_counts, 0, sizeof(vote_counts));
    Players[0].soc_ideology_category = -1;
    Players[0].soc_anti_ideology_category = -1;
    Players[1].soc_ideology_category = -1;
    Players[1].soc_anti_ideology_category = 1;
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8 | (1u << 1);
    vote_counts[0] = 1;
    PlayersData[0].ai_fight = 0;
    PlayersData[1].ai_fight = 0;
    PlayersData[1].diff_level = 3;
    RankingFactionIDUnk1 = 1;
    GameRules = 0;
    assert(wants_prop(0, 0, 1) == -2);
    PlayersData[1].diff_level = 0;
    GameRules = 0x40;
    assert(wants_prop(0, 0, 1) == 0);
    GameRules = 0;
    RankingFactionIDUnk1 = 0;

    // The pact score begins at one. A nonzero first attack result jumps past
    // the non-pact great_satan check; the callback mutation must not be read.
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8;
    PlayersData[0].diplo_treaties[1] = DTREATY_PACT;
    PlayersData[1].diplo_treaties[0] = 0;
    great_satan_result[1] = 0;
    wants_to_attack_result[0][1] = 1;
    attack_mutates_great_satan = 0;
    const int pact_score_without_mutation = wants_prop(0, 0, 1);
    wants_to_attack_result[0][1] = 1;
    attack_mutates_great_satan = 1;
    wants_to_attack_calls = 0;
    great_satan_calls = 0;
    assert(wants_prop(0, 0, 1) == pact_score_without_mutation);
    assert(wants_to_attack_calls == 1);
    assert(great_satan_calls == 0);
    attack_mutates_great_satan = 0;
    great_satan_result[1] = 0;
    wants_to_attack_result[0][1] = 0;
    assert(wants_prop(0, 0, 1) == pact_score_without_mutation + 1);
    // Native 0x52B437 reloads the human mask after attack callbacks. An
    // option made human during pact scoring reaches the positive-score
    // clamp instead of the both-AI early return.
    attack_mutates_human_mask = 1;
    assert(wants_prop(0, 0, 1) == 0);
    attack_mutates_human_mask = 0;
    LockEnableMask &= ~(1u << 1);
    PlayersData[0].diplo_treaties[1] = 0;

    // 0x52AF85 increments on other_votes >= actor_votes, including ties.
    for (int peer = 1; peer < MaxPlayerNum; ++peer) {
        vote_counts[peer] = 1;
    }
    assert(wants_prop(1, 0, 1) == 9);

    std::memset(PlayersData, 0, sizeof(PlayersData));
    Players[0].soc_ideology_category = -1;
    Players[0].soc_anti_ideology_category = -1;
    LockEnableMask = ((1u << 1) | (1u << 2) | (1u << 3)) << 8 | 1u;
    vote_counts[0] = 0;
    vote_counts[1] = 5;
    vote_counts[2] = 2;
    vote_counts[3] = 3;
    PlayersData[1].ai_fight = 1;
    great_satan_result[1] = 1;
    great_satan_calls = 0;
    assert(wants_prop(0, 0, 1) == -1);
    assert(great_satan_calls == 1);
    great_satan_result[1] = 0;

    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(FactionRankings, 0, sizeof(FactionRankings));
    for (int rank = 0; rank < MaxPlayerNum; ++rank) {
        FactionRankings[rank] = static_cast<uint32_t>(rank);
    }
    Players[0].soc_ideology_category = -1;
    Players[0].soc_anti_ideology_category = -1;
    PlayersData[0].ai_fight = -1;
    PlayersData[6].ai_fight = 1;
    LockEnableMask = ((1u << 2) | (1u << 3) | (1u << 6)) << 8 | 1u;
    vote_counts[6] = 5;
    vote_counts[2] = 2;
    vote_counts[3] = 3;
    const int reverse_rank_initial = wants_prop(0, 0, 6);
    PlayersData[0].diplo_treaties[7] = 0x70;
    assert(wants_prop(0, 0, 6) == reverse_rank_initial + 1);
    PlayersData[0].diplo_treaties[7] = 0;

    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(vote_counts, 0, sizeof(vote_counts));
    LockEnableMask = (1u << 1 | 1u << 2 | 1u << 3) << 8;
    vote_counts[1] = 1; vote_counts[2] = 2; vote_counts[3] = 3;
    PlayersData[0].ranking = 0;
    PlayersData[0].pop_total = 100;
    PlayersData[0].mil_strength_1 = 10;
    PlayersData[0].mil_strength_2 = 5;
    PlayersData[1].ranking = 7;
    PlayersData[1].pop_total = 200;
    PlayersData[1].mil_strength_1 = 20;
    PlayersData[1].mil_strength_2 = 10;
    TurnCurrentNum = 200;
    // 0x52AF??'s second vote-count gate likewise rejects two or more
    // living factions above the option before relationship scoring.
    council_vote_calls = 0;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == -9);
    assert(council_vote_calls == 3);
    vote_counts[1] = 5;
    vote_counts[2] = 1;
    vote_counts[3] = 1;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == -2);
    PlayersData[0].diplo_treaties[1] = DTREATY_PACT | DTREATY_HAVE_SURRENDERED;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == 2);
    PlayersData[0].diplo_treaties[1] = DTREATY_PACT;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == 0);
    GameRules = RULES_VICTORY_COOPERATIVE;
    TurnCurrentNum = 200;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == -1);
    TurnCurrentNum = 201;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == 1);
    PlayersData[1].diplo_treaties[0] = 0;
    assert(wants_prop(1, PROP_UNITE_SUPREME_LEADER, 1) == 5);
    vote_counts[2] = 1; vote_counts[3] = 1;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == 1);
    ExpansionEnabled = 1;
    Players[1].rule_flags = RFLAG_ALIEN;
    council_vote_calls = 0;
    assert(wants_prop(0, PROP_UNITE_SUPREME_LEADER, 1) == -9);
    assert(council_vote_calls == 0);
    ExpansionEnabled = 0;
    Players[1].rule_flags = 0;
    GameRules = 0;

    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(known_tech, 0, sizeof(known_tech));
    Chassis[8].preq_tech = 3;
    Weapon[16].preq_tech = 4;
    PlayersData[0].ai_power = 1;
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        known_tech[faction][3] = 1;
        known_tech[faction][4] = 1;
    }
    assert(wants_prop(0, PROP_REPEAL_UN_CHARTER, 0) == -3);
    assert(wants_prop(0, PROP_REINSTATE_UN_CHARTER, 0) == 3);
    known_tech[3][4] = 0;
    assert(wants_prop(0, PROP_REPEAL_UN_CHARTER, 0) == -3);
    assert(wants_prop(0, PROP_REINSTATE_UN_CHARTER, 0) == 3);

    // Owner busters skip the peer-buster adjustment; with no starting tech
    // signal and ai_power zero, the native tail returns its exact zero score.
    std::memset(known_tech, 0, sizeof(known_tech));
    PlayersData[0].planet_busters = 1;
    PlayersData[0].ai_power = 0;
    assert(wants_prop(0, PROP_REPEAL_UN_CHARTER, 0) == 0);
    assert(wants_prop(0, PROP_REINSTATE_UN_CHARTER, 0) == 0);
    PlayersData[0].planet_busters = 0;

    std::memset(PlayersData, 0, sizeof(PlayersData));
    Players[0].rule_psi = 0;
    MapSeaLevel = 0;
    MapSeaLevelCouncil = 3;
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -2);
    // Opposite council direction increments when MapSeaLevel >= 0 at
    // native 0x52B9A4..0x52B9A8; a negative level skips that increment.
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == 1);
    assert(wants_prop(0, PROP_INCREASE_SOLAR_SHADE, 0) == 1);
    MapSeaLevel = -1;
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == 0);
    MapSeaLevel = 0;

    PlayersData[0].pop_total = 2;
    AltNatural[3] = 10;
    WorldBuilder.sea_level_rises = 5;
    MapSeaLevelCouncil = 0;
    BaseCurrentCount = 1;
    Bases[0].population_size = 3;
    Bases[0].faction_id_current = 0;
    MapTiles[0].climate = 0;
    MapTiles[0].contour = 0;
    MapTiles[0].val2 = 1; // deliberately differs from the base's owner
    // Council direction zero takes 0x52B99C -> 0x52B9EF: minus one
    // before adding the base contribution, even with no growth/psi rule.
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == 5);
    MapTiles[0].val2 = 0;
    Bases[0].faction_id_current = 1;
    PlayersData[0].diplo_treaties[1] = 0x000C0030u;
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == -13);
    PlayersData[0].diplo_treaties[1] = 0;
    MapTiles[0].val2 = 1;
    Bases[0].faction_id_current = 0;
    MapTiles[0].contour = 5;  // equality takes the native weight-2 branch
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == 11);
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -7);

    // Growth zero with no psi setting skips the first direction score; a
    // nonzero growth setting adds the separate initial directional score.
    MapSeaLevelCouncil = 1;
    PlayersData[0].ai_growth = 0;
    Players[0].rule_psi = 0;
    BaseCurrentCount = 0;
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -2);
    PlayersData[0].ai_growth = 1;
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -4);
    PlayersData[0].ai_growth = 0;
    MapSeaLevelCouncil = 0;
    BaseCurrentCount = 1;

    MapTiles[0].climate = 0x60;
    MapTiles[0].contour = 0;
    Bases[0].facilities_built[3] = 0;
    PlayersData[0].pop_total = 1;
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -37);
    Bases[0].facilities_built[3] = 4; // native base bit for facility 26
    assert(wants_prop(0, PROP_MELT_POLAR_CAPS, 0) == -1);

    // Negative direction still assigns -1 above the shoreline; this base's
    // population 3 contributes 3 * -1 * 4, divided by total population 1.
    Bases[0].facilities_built[3] = 0;
    MapTiles[0].climate = 0x80;
    MapTiles[0].contour = 0;
    PlayersData[0].pop_total = 1;
    assert(wants_prop(0, PROP_LAUNCH_SOLAR_SHADE, 0) == -13);

    // The native unsigned jump-table default is proposal > 10 -> original
    // score (initialized to zero), while entry 10 targets the -9 block.
    assert(wants_prop(0, 11, 0) == 0);
    assert(wants_prop(0, -1, 0) == 0);
    assert(wants_prop(0, 10, 0) == -9);
    return 0;
}
'''
    with tempfile.TemporaryDirectory() as tmp:
        source = Path(tmp) / "wants_prop.cpp"
        executable = Path(tmp) / "wants_prop"
        source.write_text(program)
        subprocess.run(
            ["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", "-Wno-unused-parameter", str(source), "-o", str(executable)],
            check=True,
        )
        subprocess.run([str(executable)], check=True)


def test_native_proposal_scoring_phases():
    compile_and_run(BODY)


def test_controls_reject_stale_callback_state():
    damaged = BODY.replace("LockEnableMask", "frozen_mask").replace(
        "    int score = 0;",
        "    const uint32_t frozen_mask = LockEnableMask;\n    int score = 0;",
        1,
    )
    with pytest.raises(subprocess.CalledProcessError) as rejected:
        compile_and_run(damaged)
    # A compilation failure cannot satisfy the runtime damage control.
    assert rejected.value.returncode < 0
