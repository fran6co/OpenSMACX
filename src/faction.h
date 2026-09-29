/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "game.h"  // ExpansionEnabled, which is_alien_faction inlines
#include "general.h"  // parse_set, which get_noun below inlines
#include "sprite.h"
#include "stdafx.h"

 /*
  * Faction related objects, variables and functions.
  */
enum PlayerGender {
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_NEUTRAL = 2,
};

enum FactionSpecialRules {
    RULE_TECH = 0,
    RULE_UNIT = 1,
    RULE_FACILITY = 2,
    RULE_SOCIAL = 3,
    RULE_IMMUNITY = 4,
    RULE_IMPUNITY = 5,
    RULE_PENALTY = 6,
    RULE_FUNGNUTRIENT = 7,
    RULE_FUNGMINERALS = 8,
    RULE_FUNGENERGY = 9,
    RULE_ROBUST = 10,
    RULE_VOTES = 11,
    RULE_FREEFAC = 12,
    RULE_REVOLT = 13,
    RULE_NODRONE = 14,
    RULE_FREEABIL = 15,
    RULE_PROBECOST = 16,
    RULE_DEFENSE = 17,
    RULE_OFFENSE = 18,
};

enum PlayerRuleFlagBitfield {
    RFLAG_TECHSTEAL = 0x10,
    RFLAG_TECHSHARE = 0x20,
    RFLAG_WORMPOLICE = 0x40,
    RFLAG_ALIEN = 0x80,
    RFLAG_AQUATIC = 0x100,
    RFLAG_FREEPROTO = 0x200,
    RFLAG_FANATIC = 0x400,
    RFLAG_MINDCONTROL = 0x800,
    RFLAG_COMMFREQ = 0x1000,
    RFLAG_TERRAFORM = 0x2000,
    RFLAG_INTEREST = 0x4000,
    RFLAG_MORALE = 0x8000,
};

enum DiplomacyTreatyBitfield {
    DTREATY_PACT = 0x1,
    DTREATY_TREATY = 0x2,
    DTREATY_TRUCE = 0x4,
    DTREATY_COMMLINK = 0x8,
    DTREATY_VENDETTA = 0x10,
    DTREATY_WANT_REVENGE = 0x20,
    DTREATY_UNK_40 = 0x40,
    DTREATY_UNK_80 = 0x80,
    DTREATY_UNK_100 = 0x100,
    DTREATY_UNK_200 = 0x200,
    DTREATY_SHALL_BETRAY = 0x400,
    DTREATY_UNK_800 = 0x800,
    DTREATY_HAVE_INFILTRATOR = 0x1000,
    DTREATY_WANT_TO_TALK = 0x2000,
    DTREATY_UNK_4000 = 0x4000,
    DTREATY_UNK_8000 = 0x8000,
    DTREATY_UNK_10000 = 0x10000,
    DTREATY_UNK_20000 = 0x20000,
    DTREATY_ATROCITY_VICTIM = 0x40000,
    DTREATY_UNK_80000 = 0x80000,
    DTREATY_UNK_100000 = 0x100000,
    DTREATY_UNK_200000 = 0x200000,
    DTREATY_UNK_400000 = 0x400000,
    DTREATY_UNK_800000 = 0x800000,
    DTREATY_UNK_1000000 = 0x1000000,
    DTREATY_HAVE_SURRENDERED = 0x2000000,
    DTREATY_UNK_4000000 = 0x4000000,
    DTREATY_UNK_8000000 = 0x8000000,
    DTREATY_UNK_10000000 = 0x10000000,
    DTREATY_UNK_20000000 = 0x20000000,
    DTREATY_UNK_40000000 = 0x40000000,
    DTREATY_UNK_80000000 = 0x80000000,
};

enum DiploAgenda {
    DAGENDA_UNK_1 = 0x1,
    DAGENDA_UNK_2 = 0x2,
    DAGENDA_UNK_4 = 0x4,
    DAGENDA_FIGHT_TO_DEATH = 0x8,
    DAGENDA_UNK_10 = 0x10,
    DAGENDA_UNK_20 = 0x20,
    DAGENDA_UNK_40 = 0x40,
    DAGENDA_UNK_80 = 0x80,
    DAGENDA_UNK_100 = 0x100,
    DAGENDA_UNK_200 = 0x200,
    DAGENDA_UNK_400 = 0x400,
    DAGENDA_UNK_800 = 0x800,
    DAGENDA_UNK_1000 = 0x1000,
    DAGENDA_DIPLO_PERMANENT = 0x2000,
    DAGENDA_UNK_4000 = 0x4000,
    DAGENDA_UNK_8000 = 0x8000,
};

enum PlayerFlagsBitfield {
    PFLAG_SELF_AWARE_COLONY_LOST_MAINT = 0x20, // used to even out lossy integer division
    PFLAG_MAP_REVEALED = 0x200,
    PFLAG_GENETIC_PLAGUE_INTRO = 0x400, // +1 to defense against after 1st time faction experiences
    PFLAG_BEEN_ELECTED_GOVERNOR = 0x8000, // used to determine whether #GOVERNOR has been displayed
    PFLAG_UNK_10000 = 0x10000, // set by territory() once it has something to complain about
    PFLAG_UNK_20000 = 0x20000,
    PFLAG_STRAT_ATK_ENEMY_HQ = 0x200000,
    PFLAG_COOP_WITH_HUMAN = 0x400000,
    PFLAG_TEAM_UP_VS_HUMAN = 0x800000,
    PFLAG_COMMIT_ATROCIT_WANTONLY = 0x1000000,
    PFLAG_OBLIT_CAPTURED_BASES = 0x2000000,
    PFLAG_EMPHASIZE_LAND_POWER = 0x4000000,
    PFLAG_EMPHASIZE_SEA_POWER = 0x8000000,
    PFLAG_EMPHASIZE_AIR_POWER = 0x10000000,
    PFLAG_STRAT_SEARCH_OBJECTIVES = 0x20000000,
    PFLAG_STRAT_DEF_OBJECTIVES = 0x40000000,
    PFLAG_STRAT_ATK_OBJECTIVES = 0x80000000,
};

enum PlayerFlagsExtendedBitfield {
    PFLAGEXT_STRAT_LOTS_COLONY_PODS = 0x1,
    PFLAGEXT_STRAT_LOTS_TERRAFORMERS = 0x2,
    PFLAGEXT_STRAT_LOTS_SEA_BASES = 0x4,
    PFLAGEXT_STRAT_LOTS_PROBE_TEAMS = 0x8,
    PFLAGEXT_STRAT_LOTS_MISSILES = 0x10,
    PFLAGEXT_SHAMELESS_BETRAY_HUMANS = 0x20,
    PFLAGEXT_STRAT_LOTS_ARTILLERY = 0x40,
};

enum AiGoalTypes {
    AI_GOAL_UNUSED = -1,
    AI_GOAL_ATTACK = 0, // 'a', red
    AI_GOAL_DEFEND = 2, // 'd', yellow                               0000 0010
    AI_GOAL_SCOUT = 3, // 's', pink                                  0000 0011
    AI_GOAL_UNK_1 = 6, // 'n', blue ; related to tile visibility     0000 0110
    AI_GOAL_UNK_2 = 7, // related to movement                        0000 0111
    AI_GOAL_COLONIZE = 8, // 'c', teal                               0000 1000
    AI_GOAL_TERRAFORM_LAND = 9, // 'f', green ' ;                    0000 1001
    AI_GOAL_UNK_3 = 11, // related to combat                         0000 1011
    AI_GOAL_ECHELON_MIRROR = 13,                                  // 0000 1101
    AI_GOAL_PRIORITY_COMBAT = 16, // possibly artillery related      0001 0000
    AI_GOAL_TERRAFORM_WATER = 25,                                 // 0001 1001
    AI_GOAL_LANDING_SITE = 41, // '^', white                         0010 1001
    AI_GOAL_CONDENSER = 73,                                       // 0100 1001
    AI_GOAL_THERMAL_BOREHOLE = 105,                               // 0110 1001
    AI_GOAL_SENSOR_ARRAY = 121,                                   // 0111 1001
};

enum SocialCategoriesId {
    SOCIAL_CAT_POLITICS = 0,
    SOCIAL_CAT_ECONOMICS = 1,
    SOCIAL_CAT_VALUES = 2,
    SOCIAL_CAT_FUTURE = 3,
};

enum SocialEngineeringPolitics {
    SE_FRONTIER = 0,
    SE_POLICE_STATE = 1,
    SE_DEMOCRATIC = 2,
    SE_FUNDAMENTALIST = 3,
};

enum SocialEngineeringEconomics {
    SE_SIMPLE = 0,
    SE_FREE_MARKET = 1,
    SE_PLANNED = 2,
    SE_GREEN = 3,
};

enum SocialEngineeringValues {
    SE_SURVIVAL = 0,
    SE_POWER = 1,
    SE_KNOWLEDGE = 2,
    SE_WEALTH = 3,
};

enum SocialEngineeringFuture {
    SE_NONE = 0,
    SE_CYBERNETIC = 1,
    SE_EUDAIMONIC = 2,
    SE_THOUGHT_CONTROL = 3,
};

enum PlayerMood {
    MOOD_MAGNANIMOUS = 0, // also Submissive, a special case where faction is hopelessly overmatched
    MOOD_SOLICITOUS = 1,
    MOOD_COOPERATIVE = 2,
    MOOD_NONCOMMITTAL = 3,
    MOOD_AMBIVALENT = 4,
    MOOD_OBSTINATE = 5,
    MOOD_QUARRELSOME = 6,
    MOOD_BELLIGERENT = 7,
    MOOD_SEETHING = 8,
};

struct SocialCategory {
    uint32_t politics;
    uint32_t economics;
    uint32_t values;
    uint32_t future;
};

// PINNED BY THE TOTAL, NOT BY THEMSELVES, which is the difference between
// this and a self-confirming probe. `PlayerData` holds these by value and its
// size is read off the image's array stride (0x20CC, see the note there) - so
// the compiler's arithmetic over these declarations has already been checked
// against a number the image states. Asserting them adds no claim the total
// did not already carry; what it adds is SUPPLYABILITY, because the emitter
// refuses to write a layout holding a by-value member whose own type is not
// pinned, and PlayerData was verified and unusable at once for exactly that.
static_assert(sizeof(SocialCategory) == 0x10,
              "SocialCategory layout must match the original executable");

struct SocialEffect {
    int economy;
    int efficiency;
    int support;
    int talent;
    int morale;
    int police;
    int growth;
    int planet;
    int probe;
    int industry;
    int research;
};

static_assert(sizeof(SocialEffect) == 0x2C,
              "SocialEffect layout must match the original executable");

struct RulesSocialEffect {
    char set1[24];
    char set2[24];
    char padding[56];
};

struct RulesSocialCategory {
    LPSTR type;
    int preq_tech[4];
    LPSTR name[4];
    SocialEffect model_effect[4];
};

struct RulesMight {
    LPSTR adj_start;
    LPSTR adj;
};

struct RulesBonusName {
    char key[24];
};

/*
Goals overlay is visible with debug mode activated. Color is based on Goal type.
Shortcut: Shift-%
Format without omniscient view: <One letter type, see AiGoalTypes><priority>
Format with omniscient view: <1st letter of faction><One letter type, see AiGoalTypes><priority>
*/
struct Goal {
    int16_t type;
    int16_t priority;
    int x;
    int y;
    int base_id;
};

static_assert(sizeof(Goal) == 0x10,
              "Goal layout must match the original executable");

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
    char adj_name_faction[128]; // drops 2nd entry on line (abbreviation value?), non-English forms?
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
    int ai_fight; // willingness to use force to achieve goals (-1, 0, 1)
    BOOL ai_growth; // interest in population growth
    BOOL ai_tech; // interest in knowledge
    BOOL ai_wealth; // interest in wealth
    BOOL ai_power; // interest in power
    int soc_ideology_category; // priority
    int soc_anti_ideology_category; // opposition
    int soc_ideology_model; // priority
    int soc_anti_ideology_model; // opposition
    int soc_ideology_effect; // emphasis/priority
    int soc_anti_ideology_effect; // opposition ; unused, never set in default factions
};

// THE STRIDE THE IMAGE INDEXES `Players` (0x00946A50) BY, read off 1378
// call sites by tools/derive_array_strides.py - e.g. 0x00415CBC, 0x00419027. That
// number is a measurement of the binary; `sizeof` below is the compiler's
// arithmetic over the members declared above. The two were computed from
// different things and agree, which is what the assertion records.
static_assert(sizeof(Player) == 0x59C,
              "Player layout must match the original executable");


struct PlayerData {
    uint32_t flags; // see PlayerFlagsBitfield
    uint32_t ranking; // 0 (lowest) to 7 (highest) rank ; used to determine #MIGHT
    int diff_level;
    uint32_t base_name_offset; // keep track which base names have been used
    uint32_t base_sea_name_offset; // keep track which sea base names have been used
    int last_turn_new_base; // turn for last built, captured or acquired (drone riot) base
    uint32_t diplo_treaties[8];
    int diplo_agenda[8];
    uint32_t diplo_friction[8]; // between 0-20
    int diplo_spoke[8]; // Turn for the last player-to-AI communication; -1 for never
    int diplo_merc[8]; // mercifulness? Possibly higher values indicate willingness for deal-making
    char diplo_patience[8]; // AI-to-player
    int sanction_turns; // Turns left for other faction imposed economic sanctions for atrocities
    int loan_balance[8]; // Loan balance remaining this faction owes another to be paid over term
    int loan_payment[8]; // The per turn payment amount this faction owes another faction
    int unk_12[8];
    int integrity_blemishes; // Blacken
    int global_reputation; // ? integrity? related to signing treaties, freeing faction
    int diplo_unk1[8]; // ? rights (vs wrongs below)? gift, bribe; Gifts and bribes we have received
    int diplo_wrongs[8]; // number of times the player has double crossed (wronged) this faction
    int diplo_betrayed[8]; // number of times the player has been double crossed by this faction
    int diplo_unk3[8]; // ? combat related
    int diplo_unk4[8]; // ? combat related
    int traded_maps; // bitfield of other factions that have traded maps with faction
    int base_governor_adv; // default advanced Governor settings; see governor_base_bitfield
    int atrocities; // count committed by faction
    int major_atrocities; // count committed by faction
    // mind control action value: mind/thought control base (+4) or successfully subvert unit (+1)
    // TODO: possible issue with thought control still increasing variables on failure
    uint32_t mind_control_total; // running sum of mind control actions above against other factions
    uint32_t diplo_mind_control[8]; // total action value against this player by each faction
    uint32_t stolen_data_count[8]; // probe procured research data (tech/map) per faction
    int energy_reserves; // current energy credits
    uint32_t hurry_cost_total; // Net MP: Total paid energy to hurry production (current turn)
    SocialCategory soc_category_pending;
    SocialCategory soc_category_active;
    int soc_upheaval_cost_paid;
    SocialEffect soc_effect_pending;
    SocialEffect soc_effect_active; // ?
    SocialEffect soc_effect_temp; // maybe previous turn?
    SocialEffect soc_effect_base; // from faction files?
    int unk_13;
    uint32_t maint_cost_total; // Total maintenance costs for all bases (current turn)
    int tech_commerce_bonus; // Increases commerce income
    int turn_commerce_income;
    int unk_17;
    int unk_18;
    int tech_fungus_nutrient; // Increases NUTRIENT production in fungus
    int tech_fungus_mineral; // Increases MINERALS production in fungus
    int tech_fungus_energy; // Increases ENERGY production in fungus
    int tech_fungus_unk; // PSI? Dropped mechanic?
    int se_alloc_psych;
    int se_alloc_labs;
    uint32_t tech_pact_shared_goals[12]; // Bitfield; Suggested techID goals from pacts (TEAMTECH)
    int tech_ranking; // Twice the number of techs achieved
    int unk_26;
    uint32_t sat_odp_deployed;
    int theory_of_everything;
    char tech_trade_source[92];
    int tech_accumulated; // Research
    int tech_id_researching; // Researching
    int tech_cost;
    int earned_techs_saved;
    int net_random_event;
    int ai_fight; // -1, 0, 1
    BOOL ai_growth;
    BOOL ai_tech;
    BOOL ai_wealth;
    BOOL ai_power;
    int x_target;
    int y_target;
    int unk_28;
    int council_call_turn;
    int unk_29[11]; // used by council code related to buying votes; vote id?
    int unk_30[11]; // used by council code related to buying votes; factionID
    uint8_t facility_announced[4]; // bitfield - used to determine one time play of fac audio blurb
    int unk_32;
    char unk_33;
    char unk_34;
    char unk_35;
    char unk_36;
    int planet_ecology;
    int base_id_atk_target; // Battle planning of base to attack, -1 if not set
    int unk_37;
    char saved_queue_name[8][24]; // queue template
    int saved_queue_size[8]; // queue template
    int saved_queue_items[8][10]; // queue template
    int unk_38[8];
    int unk_39[8][9];
    int unk_46[9];
    int unk_47;
    int unk_48;
    int unk_49;
    uint32_t nutrient_surplus_total;
    int labs_total;
    uint32_t satellites_nutrient; // +1 Nutrients per Sky Hydroponics Lab
    uint32_t satellites_mineral; // +1 Minerals per Nessus Mining Station
    uint32_t satellites_energy; // +1 Energy per Orbital Power Transmitter
    uint32_t satellites_odp_total; // Orbital Defense Pod count
    int best_weapon_value;
    int best_psi_offense;
    int best_psi_defense;
    int best_armor_value;
    int best_land_speed;
    int enemy_best_weapon_value; // Enemy refers here to any non-pact faction
    int enemy_best_armor_value;
    int enemy_best_land_speed;
    int enemy_best_psi_offense;
    int enemy_best_psi_defense;
    int unk_64;
    int unk_65;
    int unk_66;
    int unk_67;
    int unk_68;
    char unk_69[4];
    uint8_t proto_id_active[512];
    uint8_t proto_id_queue[512];
    int16_t proto_id_lost[512];
    int total_mil_veh; // total combat units
    uint32_t current_num_bases;
    int mil_strength_1;
    int mil_strength_2;
    int pop_total;
    int unk_70; // AI total TRIAD_SEA Veh?
    int planet_busters;
    int unk_71;
    int unk_72;
    /*
    * AI planning variables that relate to faction units in specific disjoint land/water areas.
    * All of these are indexed by the region value in Map struct (see for more details).
    */
    uint16_t region_total_combat_vehs[128];
    uint8_t region_total_bases[128];
    uint8_t region_total_offensive_vehs[128];
    uint16_t region_force_rating[128]; // Combined offensive/morale rating of all units in the area
    uint16_t unk_77[128]; // Movement planning flags
    uint16_t unk_78[128]; // controlled territory by region?
    uint16_t unk_79[128]; // territory related to scenario obj or visible by region?
    uint16_t unk_80[128]; // good quality controlled territory count by region?
    uint16_t unk_81[128]; // something to do with territory / base radius by region?
    uint8_t unk_82[128]; // world site territory by region?
    uint8_t unk_83[128]; // related to goody_at()?
    uint8_t region_base_plan[128]; // visible in map UI with omni view + debug mode under base name
    /* End of block */
    Goal goals[75];
    Goal sites[25]; // same struct as goals, subset of general goals?
    int unk_93;
    int unk_94;
    int unk_95;
    int unk_96;
    int unk_97;
    uint32_t tech_achieved; // count of technology faction has discovered/achieved
    int time_bonus_count; // Net MP: Each is worth amount in seconds under Time Controls Extra
    int unk_99; // unused?
    uint32_t secret_project_intel[8]; // Bitfield; News of SPs other factions are working on
    int corner_market_turn;
    int corner_market_active;
    int unk_101;
    int unk_102;
    int unk_103;
    uint32_t flags_ext;
    int unk_105;
    int unk_106;
    int unk_107;
    int unk_108;
    int unk_109;
    int unk_110;
    char unk_111[4];
    int unk_112;
    int unk_113;
    int unk_114;
    int unk_115;
    int unk_116;
    int unk_117;
    int unk_118;
};

// READ OFF THE ARRAY STRIDE, which is the one thing about this struct the
// image states outright. `PlayersData` is at 0x0096C9E0 and is indexed by
// faction, so the stride IS the size, and 0x00466E61 spells it as a lea
// chain rather than a multiply:
//
//     shl  ecx, 6              ; i*64
//     add  ecx, edi            ; i*65        (edi = i)
//     lea  eax, [edi + ecx*2]  ; i*131
//     lea  ecx, [edi + eax*8]  ; i*1049
//     lea  eax, [edi + ecx*2]  ; i*2099
//     mov  ecx, [eax*4 + 0x96c9e0]
//
// 2099 * 4 = 8396 = 0x20CC. Corroborated independently: an agent recovering
// 0x0055D430 in batch 11 reported "0x20cc-stride diplomacy tables" from that
// function's own index arithmetic, having never seen this one.
//
// WHY IT IS WORTH ASSERTING RATHER THAN NOTING. Until a size is pinned the
// emitter supplies this struct as an OPAQUE SHELL, and a body holding one by
// value fails `C2079: uses undefined class`. Measured 2026-08-14: 156
// body-mode units fail on exactly that, more than any other single cause in
// the tree. The assert is what turns the declaration below into something the
// scaffolding may use.
static_assert(sizeof(PlayerData) == 0x20CC,
              "PlayerData layout must match the original executable");

struct FactionArt {
    FactionArt();
    ~FactionArt();

    // load_bases453A20 publishes these in terrain/size/defense order;
    // base_draw55AF20 selects the same 2 x 4 x 3 elements. The original
    // constructor455F10 constructs all24 contiguous Sprites before logos.
    Sprite base[2][4][3];
    Sprite council_logo[2];
    Sprite council_small;
    Sprite council_big;
    Sprite diplomacy;
    Sprite diplomacy_logo;
    Sprite diplomacy_landscape;
    Sprite datalink;
    Sprite report_logo_small[3];
    Sprite report_logo[2];
};
static_assert(sizeof(FactionArt) == 0x65C, "FactionArt original member extent");

static const int MaxSocialCatNum = 4;
static const int MaxSocialModelNum = 4;
static const int MaxSocialEffectNum = 11;
static const int MaxMoodNum = 9;
static const int MaxReputeNum = 8;
static const int MaxMightNum = 7;
static const int MaxBonusNameNum = 41;
static const int MaxPlayerNum = 8;
static const int MaxRankingHistoryTurns = 1000;
static const int MaxGoalsNum = 75;
static const int MaxSitesNum = 25;

// THE FACTION STORAGE - real objects, not pointers into the original image.
// Every address below is uninitialised .data in the shipped binary (zero at
// load); the rules walkers in alpha.cpp and the faction code fill them.
// Definitions in alpha.cpp, except FactionsArt in faction.cpp. FactionArt
// owns its Sprite members; it is not an array of unconstructed bytes.
// FactionsStatus (0x009A64E8) is lock.h's LockEnableMask - same rule.
extern RulesSocialCategory SocialCategories[MaxSocialCatNum];   // 0x0094B000
extern RulesSocialEffect SocialEffects[MaxSocialEffectNum];     // 0x00946580
extern LPSTR Mood[MaxMoodNum];                                  // 0x0094C9E4
extern LPSTR Repute[MaxReputeNum];                              // 0x00946A30
extern RulesMight Might[MaxMightNum];                           // 0x0094C558
extern RulesBonusName BonusName[MaxBonusNameNum];               // 0x009461A8
extern Player Players[MaxPlayerNum];   // Players[0] is AI native life faction (0x00946A50)
extern PlayerData PlayersData[MaxPlayerNum];                    // 0x0096C9E0
extern uint32_t FactionRankings[MaxPlayerNum];                  // 0x009A64EC
extern uint32_t LockEnableMask;  // 0x009A64E8 - declared in lock.h too; byte 0 = is_human bytes, byte 1 = is_alive
extern FactionArt FactionsArt[MaxPlayerNum];  // 0x0078E978
// Mutable borrowed pointers, populated by load_bases. The original's other
// producer load_sprites455EB0 and content loader load_faction_art453710
// remain unrecovered; storage/lifetime alone do not supply loaded artwork.
extern Sprite *FactionBaseSprites[MaxPlayerNum][2][4][3];  // 0x0076E590
// Per-turn archive of every faction's power score, written by rankings() for the first
// MaxRankingHistoryTurns turns and indexed faction + turn * MaxPlayerNum. Sixteen bits per
// entry, which is why a score above 65535 wraps in the graph and not in the ranking.
extern uint16_t FactionRankingHistory[MaxPlayerNum * MaxRankingHistoryTurns];  // 0x009A68AC
extern uint32_t RankingFactionIDUnk1;
extern uint32_t RankingFactionIDUnk2;
uint32_t *const FactionRankingsUnk = (uint32_t *)0x00945DD8;
extern int DiploFriction;
extern uint32_t DiploFrictionFactionIDWith;
extern uint32_t DiploFrictionFactionID;
extern int BestProtoForTrade;
extern int BestProtoToMention;
extern int PopupDialogFactionID;
// Bit 0 of the diplomacy flag word at 0x0074DA9C. treaty_on sets it when a
// vendetta lands on a pair with the local faction in it and clears it again
// when the vendetta answers an earlier request - the two accesses the image
// makes through this address, and the only ones any homed body or archive
// transcription shows. Defined in faction.cpp.
extern int DiploRedrawFlag;  // 0x0074DA9C
// Nonzero while the multiplayer window is up. treaty_on redraws the window
// only through this gate; the interface handler at 0x0045D170 reads it to
// decide between MultiWindow::hide and MultiWindow::show. Defined in
// faction.cpp.
extern int MultiWindowOpen;  // 0x007FE06C
// The third per-(faction, faction) status view, 0x194 past the treaty view
// SpyingStatusTable is the front of and 0x74 past its agenda view. Same
// `faction_with + faction * SpyingStatusStride` indexing, same 2099 stride.
// treaty_on zeroes both directions of it when a vendetta lands on top of a
// live pact/treaty/truce; nothing homed reads it yet, so the name is the
// role, not a catalogue label.
uint32_t *const DiploVendettaTable = (uint32_t *)0x0096CB8C;

// THE FOUR PAIRWISE STATUS VIEWS the diplomacy code walks, all of them
// PlayerData fields read through the array's own stride. The image reaches
// each as `PlayersData[a].<field>[b]` - `[reg*4 + 0x96C9E0 + <offsetof>]`,
// where `reg` is the pair index `a * 2099 + b` and 2099 * 4 == 0x20CC ==
// sizeof(PlayerData) - and the addresses below are just the field's byte
// offset past the array front at 0x0096C9E0. They are recorded as addresses
// rather than spelled `PlayersData[a].field[b]` at every site because most
// callers compute one pair index and reuse it across several views, which
// the array spelling cannot share.
//
//   0x0096C9F8  +0x018  diplo_treaties[0]  - the treaty/agenda word; this
//                         is spying_recovery.h's SpyingStatusTable, which is
//                         the binding the bodies here use
//   0x0096CA18  +0x038  diplo_agenda[0]    - eight dwords in, per agenda_on
//   0x0096CA58  +0x078  diplo_spoke[0]     - turn of the last contact
//   0x0096CA78  +0x098  diplo_merc[0]
//   0x0096CAA4  +0x0C4  loan_balance[0]
//   0x0096CAC4  +0x0E4  loan_payment[0]
//   0x0096CC00  +0x220  energy_reserves
uint32_t *const DiploSpokeTable = (uint32_t *)0x0096CA58;
uint32_t *const DiploMercTable = (uint32_t *)0x0096CA78;
uint32_t *const DiploLoanBalanceTable = (uint32_t *)0x0096CAA4;
uint32_t *const DiploLoanPaymentTable = (uint32_t *)0x0096CAC4;

// Raised by pop_wait and pop_expire around their call to wait_loop, and
// dropped again when it returns. Nothing this tree has recovered reads it -
// whatever consults it lives in a body still unrecovered - so the name is the
// observable behaviour, not a catalogue label.
extern int PopupWaitActive;  // 0x0093FAC4
int __cdecl pop_wait();      // 0x005398E0, text-dialog wait callback
int __cdecl pop_expire();    // 0x00539920, turn-expiry wait callback

// introduce enters diplomacy; diplomacy_ends clears this shared state.
extern int DiplomacyActive;

// THE SQUARE THE DIPLOMACY SCREEN CENTRES ON, both -1 in the shipped image
// (meaning "none") and both reset to -1 when the screen comes down.
// wants_to_speak bounds X against MapLongitudeBounds and Y against
// MapLatitudeBounds before handing them to Console::focus, which is what
// says which is which.
int *const DiploFocusX = (int *)0x0068D43C;
int *const DiploFocusY = (int *)0x0068D440;

// THE DIPLOMACY STRINGS, read out of the shipped image with
// tools/image_data.py. Every one is a .rdata literal the diplomacy
// bodies homed into faction.cpp hand to popup::start or NetMsg::pop as
// a KEY - the popup resolves it against the script database - or, for
// the .pcx entries, a sprite file name. Declared here rather than at
// each use because a namespace-scope `char *const` folds to the
// immediate the image pushes, and because most are shared by several
// bodies. The literal is the comment; the address is the evidence.
char *const DiploLabelSendingTREATYON = (char *)0x0068D444;  // "Sending TREATY ON"
char *const DiploLabelSendingTREATYOFF = (char *)0x0068D458;  // "Sending TREATY OFF"
char *const DiploLabelSendingSETTREATY = (char *)0x0068D46C;  // "Sending SET TREATY"
char *const DiploLabelSendingAGENDAON = (char *)0x0068D480;  // "Sending AGENDA ON"
char *const DiploLabelSendingAGENDAOFF = (char *)0x0068D494;  // "Sending AGENDA OFF"
char *const DiploLabelSendingSETAGENDA = (char *)0x0068D4A8;  // "Sending SET AGENDA"
char *const DiploLabelSendingENERGY = (char *)0x0068D4BC;  // "Sending ENERGY"
char *const DiploLabelSendingLOAN = (char *)0x0068D4CC;  // "Sending LOAN"
char *const DiploLabelSendingTRADEMAPS = (char *)0x0068D4DC;  // "Sending TRADE MAPS"
char *const DiploLabelSendingTECH = (char *)0x0068D4F0;  // "Sending TECH"
char *const DiploLabelSendingPACTENDS = (char *)0x0068D500;  // "Sending PACT ENDS"
char *const DiploLabelSendingWITHDRAWAL = (char *)0x0068D514;  // "Sending WITHDRAWAL"
char *const DiploLabelSendingCEDEBASE = (char *)0x0068D528;  // "Sending CEDE BASE"
char *const DiploLabelSendingDOUBLECROSS = (char *)0x0068D53C;  // "Sending DOUBLE CROSS"
char *const DiploLabelGettingDIPLOMATICLOCK = (char *)0x0068D554;  // "Getting DIPLOMATIC LOCK"
char *const DiploLabelNEEDGLOBAL = (char *)0x0068D56C;  // "NEEDGLOBAL"
char *const DiploLabelReleasingDIPLOMATICLOCK = (char *)0x0068D578;  // "Releasing DIPLOMATIC LOCK"
char *const DiploLabelDash = (char *)0x0068D594;  // " - "
char *const DiploLabelPROTO = (char *)0x0068D598;  // "PROTO"
char *const DiploLabelPACT = (char *)0x0068D5A0;  // "PACT"
char *const DiploLabelRIVAL = (char *)0x0068D5A8;  // "RIVAL"
char *const DiploLabelENDPACT1 = (char *)0x0068D5B0;  // "ENDPACT1"
char *const DiploLabelENDPACT2 = (char *)0x0068D5BC;  // "ENDPACT2"
char *const DiploLabelENDPACT3 = (char *)0x0068D5C8;  // "ENDPACT3"
char *const DiploLabelMAKEPACT = (char *)0x0068D5D4;  // "MAKEPACT"
char *const DiploLabelHERESTHEMAP = (char *)0x0068D5E0;  // "HERESTHEMAP"
char *const DiploLabelPACT_5EC = (char *)0x0068D5EC;  // "PACT"
char *const DiploLabelMAKETREATY = (char *)0x0068D5F4;  // "MAKETREATY"
char *const DiploLabelCOMMERCE = (char *)0x0068D600;  // "COMMERCE"
char *const DiploLabelTIMETOLEAVE = (char *)0x0068D60C;  // "TIMETOLEAVE"
char *const DiploLabelMAKETRUCE = (char *)0x0068D618;  // "MAKETRUCE"
char *const DiploLabelINTRO = (char *)0x0068D624;  // "INTRO"
char *const DiploLabelNEW = (char *)0x0068D62C;  // "NEW"
char *const DiploLabelMAD = (char *)0x0068D630;  // "MAD"
char *const DiploLabelPLANETBUSTER0 = (char *)0x0068D634;  // "PLANETBUSTER0"
char *const DiploLabelPLANETBUSTER = (char *)0x0068D644;  // "PLANETBUSTER"
char *const DiploLabelURGENTCOMM = (char *)0x0068D654;  // "URGENTCOMM"
char *const DiploLabelURGENTCOMM_660 = (char *)0x0068D660;  // "URGENTCOMM"
char *const DiploLabelCOMM = (char *)0x0068D66C;  // "COMM"
char *const DiploLabelWITHSUNWAR = (char *)0x0068D674;  // "WITHSUNWAR"
char *const DiploLabelWITHSUNNO = (char *)0x0068D680;  // "WITHSUNNO"
char *const DiploLabelWITHSUNYES = (char *)0x0068D68C;  // "WITHSUNYES"
char *const DiploLabelBROKEPACT = (char *)0x0068D698;  // "BROKEPACT"
char *const DiploLabelHIVE = (char *)0x0068D6A4;  // "HIVE"
char *const DiploLabelBUYNOTALK = (char *)0x0068D6AC;  // "BUYNOTALK"
char *const DiploLabelBUYPROMISED = (char *)0x0068D6B8;  // "BUYPROMISED"
char *const DiploLabelBUYALREADY = (char *)0x0068D6C4;  // "BUYALREADY"
char *const DiploLabelBUYNO = (char *)0x0068D6D0;  // "BUYNO"
char *const DiploLabelBUYNO_6D8 = (char *)0x0068D6D8;  // "BUYNO"
char *const DiploLabelBUYVOTE = (char *)0x0068D6E0;  // "BUYVOTE"
char *const DiploLabelYEA = (char *)0x0068D6E8;  // "YEA"
char *const DiploLabelNAY = (char *)0x0068D6EC;  // "NAY"
char *const DiploLabelABSTAIN = (char *)0x0068D6F0;  // "ABSTAIN"
char *const DiploLabelGOV = (char *)0x0068D6F8;  // "GOV"
char *const DiploLabel4 = (char *)0x0068D6FC;  // "4"
char *const DiploLabel3 = (char *)0x0068D700;  // "3"
char *const DiploLabel2 = (char *)0x0068D704;  // "2"
char *const DiploLabel1 = (char *)0x0068D708;  // "1"
char *const DiploLabelA = (char *)0x0068D70C;  // "A"
char *const DiploLabelBUYVOTEMENU = (char *)0x0068D710;  // "BUYVOTEMENU"
char *const DiploLabelUNIV = (char *)0x0068D71C;  // "UNIV"
char *const DiploLabelBUY = (char *)0x0068D724;  // "BUY"
char *const DiploLabelPROTO_728 = (char *)0x0068D728;  // "PROTO"
char *const DiploLabelCOMMLINK = (char *)0x0068D730;  // "COMMLINK"
char *const DiploLabelTECH = (char *)0x0068D73C;  // "TECH"
char *const DiploLabelHIGH = (char *)0x0068D744;  // "HIGH"
char *const DiploLabelENERGYFREEBIE = (char *)0x0068D74C;  // "ENERGYFREEBIE"
char *const DiploLabelMORGAN = (char *)0x0068D75C;  // "MORGAN"
char *const DiploLabelENERGYLOAN = (char *)0x0068D764;  // "ENERGYLOAN"
char *const DiploLabelREJENERGY = (char *)0x0068D770;  // "REJENERGY"
char *const DiploLabelBELIEVE = (char *)0x0068D77C;  // "BELIEVE"
char *const DiploLabelREJSELLNONE = (char *)0x0068D784;  // "REJSELLNONE"
char *const DiploLabelREJSELLAFFORD = (char *)0x0068D790;  // "REJSELLAFFORD"
char *const DiploLabelENERGYTECH = (char *)0x0068D7A0;  // "ENERGYTECH"
char *const DiploLabel2_7AC = (char *)0x0068D7AC;  // "2"
char *const DiploLabelHIVE_7B0 = (char *)0x0068D7B0;  // "HIVE"
char *const DiploLabelUNIV_7B8 = (char *)0x0068D7B8;  // "UNIV"
char *const DiploLabelSELLSECOND = (char *)0x0068D7C0;  // "SELLSECOND"
char *const DiploLabelREJSELLSECOND = (char *)0x0068D7CC;  // "REJSELLSECOND"
char *const DiploLabelUNIV_7DC = (char *)0x0068D7DC;  // "UNIV"
char *const DiploLabelREJTECHHATE = (char *)0x0068D7E4;  // "REJTECHHATE"
char *const DiploLabelREJTECHFEAR = (char *)0x0068D7F0;  // "REJTECHFEAR"
char *const DiploLabelREJTECHFEAR_7FC = (char *)0x0068D7FC;  // "REJTECHFEAR"
char *const DiploLabelHIVE_808 = (char *)0x0068D808;  // "HIVE"
char *const DiploLabelREJTECHFEAR1 = (char *)0x0068D810;  // "REJTECHFEAR1"
char *const DiploLabelREJTECHADVANCED = (char *)0x0068D820;  // "REJTECHADVANCED"
char *const DiploLabelREJTECHPROJ = (char *)0x0068D830;  // "REJTECHPROJ"
char *const DiploLabelCOMMLINKFREE = (char *)0x0068D83C;  // "COMMLINKFREE"
char *const DiploLabelTECHSLAVE = (char *)0x0068D84C;  // "TECHSLAVE"
char *const DiploLabelTECHFREEBIE = (char *)0x0068D858;  // "TECHFREEBIE"
char *const DiploLabelREJCOMMLINK = (char *)0x0068D864;  // "REJCOMMLINK"
char *const DiploLabelREJTECHLATER = (char *)0x0068D870;  // "REJTECHLATER"
char *const DiploLabelCOMMLINKTECH = (char *)0x0068D880;  // "COMMLINKTECH"
char *const DiploLabelTRADETECH = (char *)0x0068D890;  // "TRADETECH"
char *const DiploLabelTRADETECHOPT = (char *)0x0068D89C;  // "TRADETECHOPT"
char *const DiploLabelREJTECHINSTEAD = (char *)0x0068D8AC;  // "REJTECHINSTEAD"
char *const DiploLabelHIVE_8BC = (char *)0x0068D8BC;  // "HIVE"
char *const DiploLabelUNIV_8C4 = (char *)0x0068D8C4;  // "UNIV"
char *const DiploLabelREJTECHSECOND = (char *)0x0068D8CC;  // "REJTECHSECOND"
char *const DiploLabelNETMET = (char *)0x0068E5C4;  // "NETMET"
char *const DiploLabelNETMET_5CC = (char *)0x0068E5CC;  // "NETMET"
char *const DiploLabelPACTATTACKS = (char *)0x0068E5D4;  // "PACTATTACKS"
char *const DiploLabelENEMYWAR = (char *)0x0068E5E0;  // "ENEMYWAR"
char *const DiploLabelPACTUNPACT = (char *)0x0068E604;  // "PACTUNPACT"
char *const DiploLabelENEMYPACTENDS = (char *)0x0068E610;  // "ENEMYPACTENDS"
char *const DiploLabelPACTTRUCE = (char *)0x0068E620;  // "PACTTRUCE"
char *const DiploLabelPACTTREATY = (char *)0x0068E62C;  // "PACTTREATY"
char *const DiploLabelTEAM = (char *)0x0068E638;  // "TEAM"
char *const DiploLabelENEMYTRUCE = (char *)0x0068E668;  // "ENEMYTRUCE"
char *const DiploLabelENEMYTREATY = (char *)0x0068E674;  // "ENEMYTREATY"
char *const DiploLabelENEMIESTEAMUP = (char *)0x0068E680;  // "ENEMIESTEAMUP"
char *const DiploLabel1_690 = (char *)0x0068E690;  // "1"
char *const DiploLabel0 = (char *)0x0068E694;  // "0"
char *const DiploLabelCARETAKE = (char *)0x0068E698;  // "CARETAKE"
char *const DiploLabelGUYMEETSAL = (char *)0x0068E6A4;  // "GUYMEETSAL"
char *const DiploLabelGUYMEETSALNOPSYCH = (char *)0x0068E6B0;  // "GUYMEETSALNOPSYCH"
char *const DiploLabelALMEETSAL = (char *)0x0068E6C4;  // "ALMEETSAL"
char *const DiploLabelALMEETSGUY = (char *)0x0068E6D0;  // "ALMEETSGUY"
char *const DiploLabelALMEETSGUYNOPSYCH = (char *)0x0068E6DC;  // "ALMEETSGUYNOPSYCH"
char *const DiploLabelGUYMEETSAL_6F0 = (char *)0x0068E6F0;  // "GUYMEETSAL"
char *const DiploLabelGUYMEETSALNOPSYCH_6FC = (char *)0x0068E6FC;  // "GUYMEETSALNOPSYCH"
char *const DiploLabelALMEETSAL_710 = (char *)0x0068E710;  // "ALMEETSAL"
char *const DiploLabelALMEETSGUY_71C = (char *)0x0068E71C;  // "ALMEETSGUY"
char *const DiploLabelALMEETSGUYNOPSYCH_728 = (char *)0x0068E728;  // "ALMEETSGUYNOPSYCH"
char *const DiploLabelATROCITYSPOTS = (char *)0x0068E73C;  // "ATROCITYSPOTS"
char *const DiploLabelATROCIOUSITY = (char *)0x0068E75C;  // "ATROCIOUSITY"
char *const DiploLabelATROCITY = (char *)0x0068E76C;  // "ATROCITY"
char *const DiploLabel2_778 = (char *)0x0068E778;  // "2"
char *const DiploLabel1_77C = (char *)0x0068E77C;  // "1"
char *const DiploLabelMAJORATROCITY = (char *)0x0068E780;  // "MAJORATROCITY"
char *const DiploLabelCouncilSmPcx = (char *)0x0068E790;  // "council_sm.pcx"
char *const DiploLabelBreakSmPcx = (char *)0x0068E7A0;  // "break_sm.pcx"
char *const DiploLabelBREAKINGPACT = (char *)0x0068E7B0;  // "BREAKINGPACT"
char *const DiploLabelBreakSmPcx_7C0 = (char *)0x0068E7C0;  // "break_sm.pcx"
char *const DiploLabelBREAKINGTREATY = (char *)0x0068E7D0;  // "BREAKINGTREATY"
char *const DiploLabelBreakSmPcx_7E0 = (char *)0x0068E7E0;  // "break_sm.pcx"
char *const DiploLabelBREAKINGTRUCE = (char *)0x0068E7F0;  // "BREAKINGTRUCE"
char *const DiploLabelBreakSmPcx_800 = (char *)0x0068E800;  // "break_sm.pcx"
char *const DiploLabelBEGINVENDETTA = (char *)0x0068E810;  // "BEGINVENDETTA"
char *const DiploLabelMORGAN_82C = (char *)0x0068E82C;  // "MORGAN"
// The sound ambience an event asks for by id. Declared here because
// treaty_on is the homed caller that needs it; the body is still an
// unrecovered pending_bodies forwarder.
int __cdecl ambience(int ambience_id);

void __cdecl psych_check(int faction_id, int *drones, int *talents);
// `int`, not `uint32_t`, and the catalogue is the authority: the original
// exports `?has_treaty@@YAHHHH@Z`, where each `H` is an int. The `uint32_t`
// spelling here decorated to `@@YAIII@Z` and stopped matching the export,
// while the definition in faction.cpp:113 kept the catalogued `int` form - so
// the two were separate overloads and every call was `C2666: 2 overloads have
// similar conversions`, 33 of them. AGENTS.md:85 states the rule.
BOOL __cdecl auto_contact();
BOOL __cdecl great_beelzebub(int faction_id, BOOL is_aggressive);
BOOL __cdecl great_satan(int faction_id, BOOL is_aggressive);
uint32_t __cdecl aah_ooga(int faction_id, int pact_faction_id);
BOOL __cdecl climactic_battle();
BOOL __cdecl at_climax(int faction_id);
// Non-inline forwarder to base.h's `MEASURED inline` ascending, defined in
// faction.cpp: at_climax is one of the image's four real callers of
// 0x005AC680 and needs the `call`, where base.h folds it for facility_avail.
BOOL __cdecl ascending_call(int faction_id);
void __cdecl cause_friction(int faction_id, int faction_id_with, int friction);
uint32_t __cdecl get_mood(int friction);
int __cdecl reputation(int faction_id, int faction_id_with);
int __cdecl get_patience(int faction_id_with, int faction_id);
int __cdecl energy_value(int loan_principal);
void __cdecl scan_prototypes(int faction_id, int faction_id_with);
// `int`, NOT `uint32_t`, for the reason wants_to_attack documents below: the
// image exports `?set_treaty@@YAXHHHH@Z` / `?set_agenda@@YAXHHHH@Z` - four H,
// four ints - and faction.cpp defines both that way. These spelled `uint32_t`,
// which decorated to `@@YAXIIII@Z` and declared an overload nothing defined;
// that stayed latent until read_check became the first caller outside
// faction.cpp, which would have paid for it with an LNK2001.
void __cdecl set_treaty(int faction_id, int faction_id_with, int treaty,
                                  BOOL set);
void __cdecl set_agenda(int faction_id, int faction_id_with, int agenda,
                                  BOOL set);
// The treaty and agenda writers beside set_treaty: treaty_on(faction,
// faction_with, treaty) moves `treaty` INTO the pair's status word,
// treaty_off clears it; agenda_on/agenda_off the same for the agenda word.
// DipEdit's UNK handlers and read_check drive all four from its checkboxes.
// 0x0055B760 (treaty_off), 0x0055B870 (treaty_on), 0x0055BA80 (agenda_on) and
// 0x0055B820 (agenda_off) are all defined in src/faction.cpp.
// NOTE ON THE CATALOGUE: 0x0055BA80 is `?agenda_on@@YAXHHH@Z` - three H - and
// NOT a `set_agenda`; the catalogue's `?set_agenda@@YAXHHHH@Z` is the
// four-argument set-or-clear at 0x0055BBA0, defined below. Naming 0x0055BA80
// `set_agenda` would collide with that definition, which is why the writers
// are spelled on/off here.
void __cdecl treaty_on(int faction_id, int faction_id_with, int treaty);
void __cdecl treaty_off(int faction_id, int faction_id_with, int treaty);
void __cdecl agenda_on(int faction_id, int faction_id_with, int agenda);
void __cdecl agenda_off(int faction_id, int faction_id_with, int agenda);
uint32_t __cdecl has_agenda(uint32_t faction_id, uint32_t faction_id_with, 
                                      uint32_t agenda);
// `int`, NOT `uint32_t`. The image's symbol is `?wants_to_attack@@YAHHHH@Z` -
// three H, three ints - and `faction.cpp` defines it that way. This declared a
// `uint32_t` overload nothing defined, which stayed latent only because
// nothing called it through the header; the moment technology.cpp did, it was
// LNK2001.
BOOL __cdecl wants_to_attack(int faction_id, int faction_id_tgt,
                             int faction_id_unk);
int __cdecl territory(int faction_id, int faction_id_with, int flags, int *base_id_out,
                                int *count_out);
void __cdecl rankings(int apply_ranks);
int __cdecl guard_check(int faction_id, int region);
void __cdecl add_goal(int faction_id, int type, int priority, int x, int y,
                                int base_id);
void __cdecl add_site(int faction_id, int type, int priority, int x, int y);
BOOL __cdecl at_goal(int faction_id, int type, int x, int y);
BOOL __cdecl at_site(int faction_id, int type, int x, int y);
void __cdecl wipe_goals(int faction_id);
void __cdecl init_goals(int faction_id);
void __cdecl del_site(int faction_id, int type, int x, int y, int proximity);
int __cdecl corner_market(int faction_id);
void __cdecl see_map_check();
void __cdecl compute_faction_modifiers(int faction_id);
void __cdecl social_calc(SocialCategory *category, SocialEffect *effect, 
                                   int faction_id, BOOL toggle, BOOL is_quick_calc);
void __cdecl social_upkeep(int faction_id);
BOOL __cdecl society_avail(int soc_category, int soc_model, int faction_id);
void __cdecl social_ai(int faction_id, int growth_val, int tech_val, int wealth_val,
                                 int power_val, SocialCategory *output);
void __cdecl enemy_capabilities(int faction_id);
void __cdecl enemy_capabilities_t(uint32_t faction_id);

// THE AI DIPLOMACY MOVES. Each one is a `?name@@YAXHH@Z`-shaped free
// function that reads the pair's treaty/agenda state, opens a popup speaking
// in one faction's voice, and calls the treaty/agenda writers above to make
// the move stick. Homed into faction.cpp one at a time from the
// recovered-units archives; the declarations arrive with each body, so this
// block grows with the homing rather than speculating about arguments the
// image's own symbols have not pinned yet.
void __cdecl pact_attacks(int faction_id, int faction_id_with);
void __cdecl pact_truce(int faction_id, int faction_id_with);
void __cdecl pact_unpact(int faction_id, int faction_id_with);
void __cdecl enemies_team_up(int faction_id, int faction_id_with,
                             int turncoat_faction);

// The pronoun the diplomacy text needs for a faction's leader - "he", "she"
// or "they" by the faction's noun gender. 0x0058FCE0, still unrecovered and
// forwarded in pending_bodies.cpp.
char *__cdecl get_he_she(int faction_id, int sentence_form);
char *__cdecl get_his_her(int faction_id, int sentence_form);
char *__cdecl get_pact_hood(int faction_id, int faction_id_with);

// THE NETWORK MOVES. Every one is a wrapper over the local writer of the same
// name (treaty_on, set_treaty, agenda_on, ...): in a network game it logs the
// "Sending <MOVE>" line, broadcasts the move with message_data, and - when the
// caller's last argument says to - waits on NetDaemon::await_diplo for the
// receiving side's ack. Out of a network game it just calls the writer, so
// the two halves of a diplomacy move stay in step whatever the transport.
void __cdecl net_treaty_on(int faction_id, int faction_id_with, int treaty,
                           int wait);
void __cdecl net_treaty_off(int faction_id, int faction_id_with, int treaty,
                            int wait);
void __cdecl net_set_treaty(int faction_id, int faction_id_with, int treaty,
                            int set, int wait);
void __cdecl net_agenda_on(int faction_id, int faction_id_with, int agenda,
                           int wait);
void __cdecl net_agenda_off(int faction_id, int faction_id_with, int agenda,
                            int wait);
void __cdecl net_set_agenda(int faction_id, int faction_id_with, int agenda,
                            int set, int wait);
void __cdecl net_energy(int faction_id, int amount, int faction_id_with,
                        int amount_with, int wait);
void __cdecl net_loan(int faction_id, int faction_id_with, int balance,
                      int payment);
void __cdecl net_maps(int faction_id, int faction_id_with, int wait);
void __cdecl net_tech(int faction_id, int faction_id_with, int tech_id,
                      int wait);
void __cdecl net_pact_ends(int faction_id, int faction_id_with, int wait);
int __cdecl net_withdrawal(int faction_id, int faction_id_with, int wait);
void __cdecl net_cede_base(int faction_id, int faction_id_with, int wait);
void __cdecl net_double_cross(int faction_id, int faction_id_with,
                              int victim,
                              int wait);
void __cdecl diplomacy_ends(int faction_id, int faction_id_with);

// Cross-TU helpers the diplomacy bodies reach with the image's `call rel32`
// and that this tree has not recovered yet - each one is a pending_bodies
// forwarder.
void __cdecl trade_maps(int faction_id, int faction_id_with);
void __cdecl tech_achieved(int faction_id, int faction_id_with, int tech_id,
                           int give_freebie);
int __cdecl do_withdrawal(int faction_id, int faction_id_with);
void __cdecl give_a_base(int faction_id, int faction_id_with);
void __cdecl double_cross(int faction_id, int faction_id_with,
                          int victim);
void __cdecl pact_ends(int faction_id, int faction_id_with);
void __cdecl wait_loop();

MEASURED inline int __cdecl has_treaty(int faction_id, int faction_id_with, int treaty) {
    return PlayersData[faction_id].diplo_treaties[faction_id_with] & treaty;
}

MEASURED inline LPSTR __cdecl get_adjective(int faction_id) {
    return Players[faction_id].adj_name_faction;
}

// Faction dialogue forms. The leader-name/title/insult forms also select
// singular grammar and the leader's gender for the next parse_says call.
char *__cdecl get_name(int faction_id);
char *__cdecl get_title(int faction_id);
char *__cdecl get_adjective_leader(int faction_id);
char *__cdecl get_adjective_insult_leader(int faction_id);
char *__cdecl get_adjective_faction(int faction_id);
char *__cdecl get_adjective_insult_faction(int faction_id);
char *__cdecl get_insult_leader(int faction_id);

// INLINE: the image has no is_human - it open-codes what this names.
inline uint8_t __cdecl factions_status_byte(int index) {
    // the image's FactionsStatus byte view of the LockEnableMask dword
    return reinterpret_cast<const uint8_t *>(&LockEnableMask)[index];
}

inline BOOL __cdecl is_human(uint32_t faction_id) {
    return factions_status_byte(0) & (1u << faction_id);
}

// INLINE: the image has no is_alive - it open-codes what this names.
inline BOOL __cdecl is_alive(uint32_t faction_id) {
    return factions_status_byte(1) & (1u << faction_id);
}

inline BOOL __cdecl is_alien_faction(uint32_t faction_id) {
    return ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN);
}

// MEASURED inline: social_ai's two call sites do not reach a `call` to
// social_upheaval in the image - it is inlined there, even though it also
// keeps its own out-of-line body (0x005B4550, in faction.cpp) for other
// callers. See faction.cpp for the ORIGINAL marker; this definition is only
// what lets social_ai's own translation unit fold the two call sites.
MEASURED inline uint32_t __cdecl social_upheaval(int faction_id, SocialCategory *category_new) {
    uint32_t change_count = 0;
    for (int i = 0; i < MaxSocialCatNum; i++) {
        if (*(&category_new->politics + i) !=
            *(&PlayersData[faction_id].soc_category_active.politics + i)) {
            change_count++;
        }
    }
    if (!change_count) {
        return 0;
    }
    change_count++;
    uint32_t diff_lvl = is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN;
    uint32_t cost = change_count * change_count * change_count * diff_lvl;
    if (is_alien_faction(faction_id)) {
        cost += cost / 2;
    }
    return cost;
}

// BODY IN faction.h, as `MEASURED inline` (marker stays in faction.cpp):
// name_base (base.cpp) needs this folded in place - the image never calls
// get_noun out of line there.
MEASURED inline LPSTR __cdecl get_noun(int faction_id) {
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    return Players[faction_id].noun_faction;
}
