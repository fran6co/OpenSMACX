// Host-only shim for compiling the actual ignored source assembly. It supplies
// table owners/callees, not an alternate consider_designs implementation.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

static const int MaxPlayerNum = 8;
static const int MaxReactorNum = 4;
static const int MaxWeaponNum = 26;
static const int MaxArmorNum = 14;
static const int MaxVehProtoFactionNum = 64;
static const int DLVL_THINKER = 4;
static const unsigned int DTREATY_ATROCITY_VICTIM = 0x40000u;
static const unsigned int DTREATY_WANT_REVENGE = 0x20u;
static const unsigned int RULES_INTENSE_RIVALRY = 0x40u;
static const int PROP_REPEAL_UN_CHARTER = 8;
static const int PROP_REINSTATE_UN_CHARTER = 9;
static const int TRIAD_SEA = 1;
static const int TRIAD_AIR = 2;
static const int TRIAD_LAND = 0;

static const int ABL_AAA = 0x100;
static const int ABL_AIR_SUPERIORITY = 0x20;
static const int ABL_AMPHIBIOUS = 0x8;
static const int ABL_ANTIGRAV_STRUTS = 0x400;
static const int ABL_ARTILLERY = 0x8000;
static const int ABL_CLEAN_REACTOR = 0x10000;
static const int ABL_CLOAKED = 0x4;
static const int ABL_COMM_JAMMER = 0x200;
static const int ABL_DEEP_RADAR = 0x2;
static const int ABL_DROP_POD = 0x10;
static const int ABL_EMPATHIC = 0x800;
static const int ABL_FUNGICIDAL = 0x2000;
static const int ABL_NERVE_GAS = 0x100000;
static const int ABL_SUPER_TERRAFORMER = 0x1000;
static const int ABL_TRAINED = 0x1000000;
static const int ABL_TRANCE = 0x4000;

struct RulesWeapon { int preq_tech; int offense_rating; };
struct RulesArmor { int preq_tech; int cost; int defense_rating; };
struct RulesChassis { int preq_tech; int speed; int missile; };
struct RulesReactor { int preq_tech; };
struct RulesAbility { int preq_tech; };
struct RulesBasic { int tech_two_spec_abilities; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    int ranking, diff_level;
    unsigned int diplo_treaties[MaxPlayerNum];
    int atrocities, integrity_blemishes;
    int diplo_unk4[MaxPlayerNum];
    int ai_fight, ai_growth, ai_tech, ai_wealth, ai_power;
    int planet_ecology;
    int best_armor_value, enemy_best_weapon_value;
    int enemy_best_land_speed, enemy_best_psi_offense, enemy_best_psi_defense;
    int unk_66, unk_68, unk_105, unk_106;
    int current_num_bases;
    SocialEffect soc_effect_pending;
};
struct Player { int rule_sharetech; int rule_morale; };
struct VehPrototype {
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id, carry_capacity, cost;
    uint8_t plan;
    int8_t unk_1;
    uint8_t obsolete_factions;
    int8_t combat_factions, icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
};
struct Base { int faction_id_current; };
struct Projects { int dream_twister_, neural_amplifier_;
 int &neural_amplifier() { return neural_amplifier_; }
 const int &neural_amplifier() const { return neural_amplifier_; }

 int &dream_twister() { return dream_twister_; }
 const int &dream_twister() const { return dream_twister_; }
};

static RulesWeapon Weapon[MaxWeaponNum];
static RulesArmor Armor[MaxArmorNum];
static RulesChassis Chassis[9];
static RulesReactor Reactor[MaxReactorNum];
static RulesAbility Ability[32];
static RulesBasic Rules;
static PlayerData PlayersData[MaxPlayerNum];
static Player Players[MaxPlayerNum];
static VehPrototype VehPrototypes[MaxPlayerNum * MaxVehProtoFactionNum];
static Base Bases[128];
static Projects SecretProject;
static int CouncilResolutionCount[16];
static int LockEnableMask;
static int GameRules;
static int ExpansionEnabled;
static int DesignConsiderArmor;
static int DesignConsiderOverride;
static int FirstProposedPrototype;

static int g_enemy_calls;
static int g_proposals;
static int g_pick_calls;
static int g_pick_args[64][3];
static int g_last_proposal[7];
static int g_proposal_log[128][7];

static int has_tech(int tech, int) { return tech == 777; }
static void enemy_capabilities(int) { ++g_enemy_calls; }
static int pick_chassis(int faction, int triad, int request) {
    if (g_pick_calls < 64) {
        g_pick_args[g_pick_calls][0] = faction;
        g_pick_args[g_pick_calls][1] = triad;
        g_pick_args[g_pick_calls][2] = request;
    }
    ++g_pick_calls;
    return -1;
}
static int weapon_budget(int, int, int) { return 0; }
static int proto_cost(int, int, int, int, int) { return 0; }
static int great_satan(int, int) { return 1; }
static int bit_count(int value) {
    int count = 0;
    while (value != 0) { count += value & 1; value >>= 1; }
    return count;
}
static void add_abil(int *flags, int ability, int) { *flags |= ability; }
static void propose_proto(int faction, int chassis, int weapon, int armor,
                          int abilities, int reactor, int category,
                          const char *) {
    ++g_proposals;
    g_last_proposal[0] = faction;
    g_last_proposal[1] = chassis;
    g_last_proposal[2] = weapon;
    g_last_proposal[3] = armor;
    g_last_proposal[4] = abilities;
    g_last_proposal[5] = reactor;
    g_last_proposal[6] = category;
    const int log_index = g_proposals - 1;
    if (log_index >= 0 && log_index < 128) {
        for (int i = 0; i < 7; ++i)
            g_proposal_log[log_index][i] = g_last_proposal[i];
    }
}

/* __SOURCE_EXTRACTED_FROM_MAPPED_VEH_OWNER__ */

int main(int argc, char **argv) {
    const bool prototype_plan0 = argc > 1 && strcmp(argv[1], "plan0") == 0;
    for (int i = 0; i < MaxWeaponNum; ++i) {
        Weapon[i].preq_tech = 10;
        Weapon[i].offense_rating = i;
    }
    for (int i = 0; i < MaxArmorNum; ++i) {
        Armor[i].preq_tech = 10;
        Armor[i].cost = 1;
        Armor[i].defense_rating = i;
    }
    for (int i = 0; i < MaxReactorNum; ++i) Reactor[i].preq_tech = 10;
    for (int i = 0; i < 32; ++i) Ability[i].preq_tech = 10;
    for (int i = 0; i < MaxPlayerNum; ++i) {
        PlayersData[i].integrity_blemishes = 1;
        PlayersData[i].unk_105 = PlayersData[i].unk_106 = 0;
    }
    SecretProject.dream_twister() = -1;
    SecretProject.neural_amplifier() = -1;

    if (prototype_plan0) {
        Weapon[2].preq_tech = 777;
        Weapon[2].offense_rating = 4;
        Armor[3].preq_tech = 777;
        Armor[3].defense_rating = 3;
        Reactor[3].preq_tech = 777;
        VehPrototype &native_plan = VehPrototypes[MaxVehProtoFactionNum];
        memset(&native_plan, 0, sizeof(native_plan));
        native_plan.flags = 1;
        native_plan.chassis_id = 1;
        native_plan.weapon_id = 0;
        native_plan.armor_id = 0;
        native_plan.plan = 0;
        native_plan.unk_1 = 7;
    }

    // Native plan-1's 0x581659 arm checks the armor-selection sentinel before
    // any Armor[] access. When no candidate armor exists, it retains the
    // current prototype armor while still proposing the selected weapon.
    VehPrototype sentinel_prototype;
    memset(&sentinel_prototype, 0, sizeof(sentinel_prototype));
    sentinel_prototype.chassis_id = 3;
    sentinel_prototype.weapon_id = 4;
    sentinel_prototype.armor_id = 5;
    sentinel_prototype.unk_1 = 12;
    int shared_armor_scale = 0;
    ConsiderDesignsProposalState proposal_state(
        sentinel_prototype, 2, -1, 1, shared_armor_scale);
    if (!consider_designs_plan_proposal(1, 1, proposal_state)) return 20;
    if (g_proposals != 1 || g_last_proposal[0] != 1
        || g_last_proposal[1] != 3 || g_last_proposal[2] != 2
        || g_last_proposal[3] != 5 || g_last_proposal[5] != 1
        || g_last_proposal[6] != 12 || shared_armor_scale != 2) {
        return 21;
    }
    g_proposals = 0;

    consider_designs(1);

    if (prototype_plan0) {
        int saw_native_plan0_proposal = 0;
        for (int i = 0; i < g_proposals && i < 128; ++i) {
            if (g_proposal_log[i][0] == 1
                && g_proposal_log[i][1] == 1
                && g_proposal_log[i][2] == 2
                && g_proposal_log[i][3] == 0
                && g_proposal_log[i][5] == 4
                && g_proposal_log[i][6] == 7) {
                saw_native_plan0_proposal = 1;
            }
        }
        printf("PROPOSALS %d\n", g_proposals);
        for (int i = 0; i < g_proposals && i < 128; ++i) {
            for (int j = 0; j < 7; ++j) {
                printf("%s%d", j == 0 ? "" : ",", g_proposal_log[i][j]);
            }
            printf("\n");
        }
        return saw_native_plan0_proposal ? 0 : 13;
    }

    if (g_enemy_calls != 1 || g_proposals != 0) return 10;
    // Both values are requested by the suffix/early strategy, then each real
    // continuation is observable as a source call rather than a no-op.
    if (g_pick_calls < 6) return 11;
    int saw_land_fallback = 0;
    for (int i = 0; i < g_pick_calls && i < 64; ++i) {
        if (g_pick_args[i][1] == TRIAD_LAND && g_pick_args[i][2] == 3)
            saw_land_fallback = 1;
    }
    if (!saw_land_fallback) return 12;
    printf("PICKS %d\n", g_pick_calls);
    for (int i = 0; i < g_pick_calls && i < 64; ++i) {
        printf("%d,%d,%d\n", g_pick_args[i][0], g_pick_args[i][1],
               g_pick_args[i][2]);
    }
    return 0;
}
