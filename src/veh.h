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

 /*
  * Vehicle/unit related objects, variables and functions.
  */
enum VehMorale {
    MORALE_VERY_GREEN = 0,
    MORALE_GREEN = 1,
    MORALE_DISCIPLINED = 2,
    MORALE_HARDENED = 3,
    MORALE_VETERAN = 4,
    MORALE_COMMANDO = 5,
    MORALE_ELITE = 6,
};

enum VehBasic {
    BSC_COLONY_POD = 0,
    BSC_FORMERS = 1,
    BSC_SCOUT_PATROL = 2,
    BSC_TRANSPORT_FOIL = 3,
    BSC_SEA_FORMERS = 4,
    BSC_SUPPLY_CRAWLER = 5,
    BSC_PROBE_TEAM = 6,
    BSC_ALIEN_ARTIFACT = 7,
    BSC_MIND_WORMS = 8,
    BSC_ISLE_OF_THE_DEEP = 9,
    BSC_LOCUSTS_OF_CHIRON = 10,
    BSC_UNITY_ROVER = 11,
    BSC_UNITY_SCOUT_CHOPPER = 12,
    BSC_UNITY_FOIL = 13,
    BSC_SEALURK = 14,
    BSC_SPORE_LAUNCHER = 15,
    BSC_BATTLE_OGRE_MK1 = 16,
    BSC_BATTLE_OGRE_MK2 = 17,
    BSC_BATTLE_OGRE_MK3 = 18,
    BSC_FUNGAL_TOWER = 19,
    BSC_UNITY_MINING_LASER = 20,
    BSC_SEA_ESCAPE_POD = 21,
    BSC_UNITY_GUNSHIP = 22,
};

enum VehTriad {
    TRIAD_LAND = 0,
    TRIAD_SEA = 1,
    TRIAD_AIR = 2,
};

enum VehAbilitiesBitfield {
    ABL_SUPER_TERRAFORMER = 0x1,
    ABL_DEEP_RADAR = 0x2,
    ABL_CLOAKED = 0x4,
    ABL_AMPHIBIOUS = 0x8,
    ABL_DROP_POD = 0x10,
    ABL_AIR_SUPERIORITY = 0x20,
    ABL_DEEP_PRESSURE_HULL = 0x40,
    ABL_CARRIER = 0x80,
    ABL_AAA = 0x100,
    ABL_COMM_JAMMER = 0x200,
    ABL_ANTIGRAV_STRUTS = 0x400,
    ABL_EMPATHIC = 0x800,
    ABL_POLY_ENCRYPTION = 0x1000,
    ABL_FUNGICIDAL = 0x2000,
    ABL_TRAINED = 0x4000,
    ABL_ARTILLERY = 0x8000,
    ABL_CLEAN_REACTOR = 0x10000,
    ABL_BLINK_DISPLACER = 0x20000,
    ABL_TRANCE = 0x40000,
    ABL_HEAVY_TRANSPORT = 0x80000,
    ABL_NERVE_GAS = 0x100000,
    ABL_REPAIR = 0x200000,
    ABL_POLICE_2X = 0x400000,
    ABL_SLOW = 0x800000,
    ABL_SOPORIFIC_GAS = 0x1000000,
    ABL_DISSOCIATIVE_WAVE = 0x2000000,
    ABL_MARINE_DETACHMENT = 0x4000000,
    ABL_FUEL_NANOCELLS = 0x8000000,
    ABL_ALGO_ENHANCEMENT = 0x10000000,
};

enum VehAbilityFlagBitfield {
    AFLAG_ALLOWED_LAND_UNIT = 0x1,
    AFLAG_ALLOWED_SEA_UNIT = 0x2,
    AFLAG_ALLOWED_AIR_UNIT = 0x4,
    AFLAG_ALLOWED_COMBAT_UNIT = 0x8,
    AFLAG_ALLOWED_TERRAFORM_UNIT = 0x10,
    AFLAG_ALLOWED_NONCOMBAT_UNIT = 0x20,
    AFLAG_NOT_ALLOWED_PROBE_TEAM = 0x40,
    AFLAG_NOT_ALLOWED_PSI_UNIT = 0x80,
    AFLAG_TRANSPORT_ONLY_UNIT = 0x100,
    AFLAG_NOT_ALLOWED_FAST_UNIT = 0x200,
    AFLAG_COST_INC_LAND_UNIT = 0x400,
    AFLAG_ONLY_PROBE_TEAM = 0x800,
};

enum VehPrototypeFlagBitfield {
    PROTO_ACTIVE = 0x1, // if this bit is zero, prototype has been retired
    PROTO_CUSTOM_NAME_SET = 0x2,
    PROTO_TYPED_COMPLETE = 0x4,
    // Both are set by mention_prototypes (0x0053A230) and read by
    // scan_prototypes (0x0053A4A0), which will not offer up a prototype that
    // already carries the bit its treaty state cares about. 0x10 goes onto
    // every prototype of the speaking faction once it has mentioned any of
    // them; 0x20 goes onto the one mentioned to a pact partner.
    PROTO_UNK_10 = 0x10,
    PROTO_UNK_20 = 0x20,
};

enum VehOrders {
    ORDER_NONE = 0,              //  -
    ORDER_SENTRY_BOARD = 1,      // (L)
    ORDER_HOLD = 2,              // (H); Hold (set 1st waypoint (-1, 0)), Hold 10 (-1, 10), On Alert
    ORDER_CONVOY = 3,            // (O)
    ORDER_FARM = 4,              // (f)
    ORDER_SOIL_ENRICHER = 5,     // (f)
    ORDER_MINE = 6,              // (M)
    ORDER_SOLAR_COLLECTOR = 7,   // (S)
    ORDER_PLANT_FOREST = 8,      // (F)
    ORDER_ROAD = 9,              // (R)
    ORDER_MAGTUBE = 10,          // (R)
    ORDER_BUNKER = 11,           // (K)
    ORDER_AIRBASE = 12,          // (.)
    ORDER_SENSOR_ARRAY = 13,     // (O)
    ORDER_REMOVE_FUNGUS = 14,    // (F)
    ORDER_PLANT_FUNGUS = 15,     // (F)
    ORDER_CONDENSER = 16,        // (N)
    ORDER_ECHELON_MIRROR = 17,   // (E)
    ORDER_THERMAL_BOREHOLE = 18, // (B)
    ORDER_DRILL_AQUIFIER = 19,   // (Q)
    ORDER_TERRAFORM_UP = 20,     // (])
    ORDER_TERRAFORM_DOWN = 21,   // ([)
    ORDER_TERRAFORM_LEVEL = 22,  // (_)
    ORDER_PLACE_MONOLITH = 23,   // (?)
    ORDER_MOVE_TO = 24,          // (G); Move unit to here, Go to Base, Group go to, Patrol
    ORDER_MOVE = 25,             // (>); Only used in a few places, seems to be buggy mechanic
    ORDER_EXPLORE = 26,          // (/); not set via shortcut, AI related?
    ORDER_ROAD_TO = 27,          // (r)
    ORDER_MAGTUBE_TO = 28,       // (t)
    // 29                        // max value for normal orders, seems to be unused
    ORDER_AI_MOVE_TO = 88,       //  - ; ORDER_MOVE_TO (0x18) | 0x40 > 0x58 ? only used by AI funcs
};

enum VehOrdersAutoType {
    ORDERA_TERRA_AUTO_FULL = 0,
    ORDERA_TERRA_AUTO_ROAD = 1,
    ORDERA_TERRA_AUTO_MAGTUBE = 2,
    ORDERA_TERRA_AUTOIMPROVE_BASE = 3,
    ORDERA_TERRA_FARM_SOLAR_ROAD = 4,
    ORDERA_TERRA_FARM_MINE_ROAD = 5, // displayed incorrectly as 'Mine+Solar+Road' (labels.txt:L411)
    ORDERA_TERRA_AUTO_FUNGUS_REM = 6,
    ORDERA_TERRA_AUTOMATIC_SENSOR = 7,
    // 8 unused?
    // 9 unused?
    ORDERA_BOMBING_RUN = 10, // air units only
    ORDERA_ON_ALERT = 11,
    ORDERA_AUTOMATE_AIR_DEFENSE = 12,
};

enum VehFlagsBitfield {
    // repair_phase (00526030) ages these two one unit in four each turn: bit 2 is cleared if it
    // is set, otherwise bit 1 is, so the pair is a two-step countdown rather than two independent
    // flags. What it counts down to is not recovered.
    VFLAG_UNK_1 = 0x1,
    VFLAG_UNK_2 = 0x2,
    VFLAG_PROBE_PACT_OPERATIONS = 0x4,
    VFLAG_IS_OBJECTIVE = 0x20,
    VFLAG_LURKER = 0x40,
    VFLAG_START_RAND_LOCATION = 0x80,
    VFLAG_START_RAND_MONOLITH = 0x100,
    VFLAG_START_RAND_FUNGUS = 0x200,
    VFLAG_INVISIBLE = 0x400,
    VFLAG_UNK_1000 = 0x1000, // cleared for every one of a faction's units by repair_phase
};

enum VehStateBitfield {
    VSTATE_UNK_1 = 0x1,
    VSTATE_UNK_2 = 0x2,
    VSTATE_UNK_4 = 0x4,
    VSTATE_UNK_8 = 0x8,
    VSTATE_REQUIRES_SUPPORT = 0x10,
    VSTATE_MADE_AIRDROP = 0x20,
    VSTATE_UNK_40 = 0x40,
    VSTATE_DESIGNATE_DEFENDER = 0x80,
    VSTATE_MONOLITH_UPGRADED = 0x100,
    VSTATE_UNK_200 = 0x200,
    VSTATE_UNK_400 = 0x400,
    VSTATE_UNK_800 = 0x800,
    VSTATE_UNK_1000 = 0x1000,
    VSTATE_UNK_2000 = 0x2000,
    VSTATE_EXPLORE = 0x4000,
    VSTATE_UNK_8000 = 0x8000,
    VSTATE_UNK_10000 = 0x10000,
    VSTATE_UNK_20000 = 0x20000,
    VSTATE_UNK_40000 = 0x40000,
    VSTATE_USED_NERVE_GAS = 0x80000, // set/reset on attacking Veh after each attack
    VSTATE_UNK_100000 = 0x100000,
    VSTATE_PACIFISM_DRONE = 0x200000,
    VSTATE_PACIFISM_FREE_SKIP = 0x400000,
    VSTATE_ASSISTANT_WORM = 0x800000, // Int: Brood Trainer; Human player's 1st spawned Mind Worm
    VSTATE_UNK_1000000 = 0x1000000,
    VSTATE_UNK_2000000 = 0x2000000,
    VSTATE_CRAWLING = 0x4000000, // more than just crawling, terraforming also?
    VSTATE_UNK_8000000 = 0x8000000,
    VSTATE_UNK_10000000 = 0x10000000,
    VSTATE_UNK_20000000 = 0x20000000,
    VSTATE_UNK_40000000 = 0x40000000,
    VSTATE_UNK_80000000 = 0x80000000,
};

enum VehWeaponMode {
    WPN_MODE_PROJECTILE = 0,
    WPN_MODE_ENERGY = 1,
    WPN_MODE_MISSILE = 2,
    WPN_MODE_TRANSPORT = 7,
    WPN_MODE_COLONIST = 8,
    WPN_MODE_TERRAFORMER = 9,
    WPN_MODE_CONVOY = 10,
    WPN_MODE_INFOWAR = 11,
    WPN_MODE_ARTIFACT = 12,
};

enum VehPlan {
    PLAN_OFFENSIVE = 0,
    PLAN_COMBAT = 1,
    PLAN_DEFENSIVE = 2,
    PLAN_RECONNAISANCE = 3,
    PLAN_AIR_SUPERIORITY = 4,
    PLAN_PLANET_BUSTER = 5,
    PLAN_NAVAL_SUPERIORITY = 6,
    PLAN_NAVAL_TRANSPORT = 7,
    PLAN_COLONIZATION = 8,
    PLAN_TERRAFORMING = 9,
    PLAN_SUPPLY_CONVOY = 10,
    PLAN_INFO_WARFARE = 11,
    PLAN_ALIEN_ARTIFACT = 12,
    PLAN_TECTONIC_MISSILE = 13,
    PLAN_FUNGAL_MISSILE = 14,
};

enum VehWeapon {
    WPN_HAND_WEAPONS = 0,
    WPN_LASER = 1,
    WPN_PARTICLE_IMPACTOR = 2,
    WPN_GATLING_LASER = 3,
    WPN_MISSILE_LAUNCHER = 4,
    WPN_CHAOS_GUN = 5,
    WPN_FUSION_LASER = 6,
    WPN_TACHYON_BOLT = 7,
    WPN_PLASMA_SHARD = 8,
    WPN_QUANTUM_LASER = 9,
    WPN_GRAVITON_GUN = 10,
    WPN_SINGULARITY_LASER = 11,
    WPN_RESONANCE_LASER = 12,
    WPN_RESONANCE_BOLT = 13,
    WPN_STRING_DISRUPTOR = 14,
    WPN_PSI_ATTACK = 15,
    WPN_PLANET_BUSTER = 16,
    WPN_COLONY_MODULE = 17,
    WPN_TERRAFORMING_UNIT = 18,
    WPN_TROOP_TRANSPORT = 19,
    WPN_SUPPLY_TRANSPORT = 20,
    WPN_PROBE_TEAM = 21,
    WPN_ALIEN_ARTIFACT = 22,
    WPN_CONVENTIONAL_PAYLOAD = 23,
    WPN_TECTONIC_PAYLOAD = 24,
    WPN_FUNGAL_PAYLOAD = 25,
};

enum VehArmor {
    ARM_NO_ARMOR = 0,
    ARM_SYNTHMETAL_ARMOR = 1,
    ARM_PLASMA_STEEL_ARMOR = 2,
    ARM_SILKSTEEL_ARMOR = 3,
    ARM_PHOTON_WALL = 4,
    ARM_PROBABILITY_SHEATH = 5,
    ARM_NEUTRONIUM_ARMOR = 6,
    ARM_ANTIMATTER_PLATE = 7,
    ARM_STASIS_GENERATOR = 8,
    ARM_PSI_DEFENSE = 9,
    ARM_PULSE_3_ARMOR = 10,
    ARM_RESONANCE_3_ARMOR = 11,
    ARM_PULSE_8_ARMOR = 12,
    ARM_RESONANCE_8_ARMOR = 13,
};

enum VehChassis {
    CHSI_INFANTRY = 0,
    CHSI_SPEEDER = 1,
    CHSI_HOVERTANK = 2,
    CHSI_FOIL = 3,
    CHSI_CRUISER = 4,
    CHSI_NEEDLEJET = 5,
    CHSI_COPTER = 6,
    CHSI_GRAVSHIP = 7,
    CHSI_MISSILE = 8,
};

enum VehReactor {
    RECT_FISSION = 1,
    RECT_FUSION = 2,
    RECT_QUANTUM = 3,
    RECT_SINGULARITY = 4,
};

struct RulesChassis {
    LPSTR offsv_1_name;
    LPSTR offsv_2_name;
    LPSTR offsv_lrg_name;
    LPSTR defsv_1_name;
    LPSTR defsv_2_name;
    LPSTR defsv_lrg_name;
    uint32_t offsv_1_gender;
    uint32_t offsv_2_gender;
    uint32_t offsv_lrg_gender;
    uint32_t defsv_1_gender;
    uint32_t defsv_2_gender;
    uint32_t defsv_lrg_gender;
    BOOL offsv_1_plural;
    BOOL offsv_2_plural;
    BOOL offsv_lrg_plural;
    BOOL defsv_1_plural;
    BOOL defsv_2_plural;
    BOOL defsv_lrg_plural;
    uint8_t speed;
    uint8_t triad;
    uint8_t range;
    uint8_t cargo;
    uint8_t cost; // TODO: expand to uint32_t in future
    uint8_t missile; // treated as boolean
    uint8_t sprite_flag_x_off[8];
    uint8_t sprite_flag_y_off[8];
    uint8_t sprite_unk1_x_off[8];
    uint8_t sprite_unk1_y_off[8];
    uint8_t sprite_unk2_x_off[8];
    uint8_t sprite_unk2_y_off[8];
    uint8_t sprite_unk3_x_off[8];
    uint8_t sprite_unk3_y_off[8];
    int16_t preq_tech;
};

// THE STRIDE THE IMAGE INDEXES `Chassis` (0x0094A330) BY, read off 497
// call sites by tools/derive_array_strides.py - e.g. 0x0040B87A, 0x0040DAD3. That
// number is a measurement of the binary; `sizeof` below is the compiler's
// arithmetic over the members declared above. The two were computed from
// different things and agree, which is what the assertion records.
static_assert(sizeof(RulesChassis) == 0x90,
              "RulesChassis layout must match the original executable");


struct RulesWeapon {
    LPSTR name;
    LPSTR name_short;
    int8_t offense_rating;
    int8_t icon;
    uint8_t mode;
    uint8_t cost;
    int16_t preq_tech;
    int16_t padding;
};

// THE ARRAY EXACTLY FILLS THE SPACE BETWEEN ITS BASE AND THE NEXT NAMED
// GLOBAL, which is what makes this a measurement and not a restatement.
// `Weapon` sits at 0x0094AE60 and `SocialCategories` at 0x0094B000, a gap of
// 0x1A0. 26 elements of 0x10 is 0x1A0 - exactly that,
// with nothing left over. A wrong element size would either overflow the
// neighbour or leave a hole, and the element count is the tree's own
// (the [26] in src/veh.cpp).
//
// This is the SINGLETON-AND-ARRAY case that tools/derive_array_strides.py
// cannot reach: it reads a stride off an index computation, and code that
// walks these tables with a pointer never computes one.
static_assert(sizeof(RulesWeapon) == 0x10,
              "RulesWeapon layout must match the original executable");


struct RulesArmor {
    LPSTR name;
    LPSTR name_short;
    int8_t defense_rating;
    uint8_t mode;
    uint8_t cost;
    uint8_t padding1;
    int16_t preq_tech;
    int16_t padding2;
};

// THE ARRAY EXACTLY FILLS THE SPACE BETWEEN ITS BASE AND THE NEXT NAMED
// GLOBAL, which is what makes this a measurement and not a restatement.
// `Armor` sits at 0x0094F278 and `Technology` at 0x0094F358, a gap of
// 0xE0. 14 elements of 0x10 is 0xE0 - exactly that,
// with nothing left over. A wrong element size would either overflow the
// neighbour or leave a hole, and the element count is the tree's own
// (the [14] in src/veh.cpp).
//
// This is the SINGLETON-AND-ARRAY case that tools/derive_array_strides.py
// cannot reach: it reads a stride off an index computation, and code that
// walks these tables with a pointer never computes one.
static_assert(sizeof(RulesArmor) == 0x10,
              "RulesArmor layout must match the original executable");


struct Veh {
    int16_t x;
    int16_t y;
    uint32_t state; // see VehStateBitfield
    uint16_t flags; // see VehFlagsBitfield
    int16_t proto_id;
    uint16_t unk_1; // seems to only be set by veh_clear()
    uint8_t faction_id;
    uint8_t year_end_lurking;
    uint8_t dmg_incurred; // damaged taken
    int8_t order; // see VehOrders
    uint8_t waypoint_count;
    uint8_t patrol_current_point;
    int16_t waypoint_x[4]; // [0] duals as transport veh id if unit is sentry/board
    int16_t waypoint_y[4];
    uint8_t morale;
    uint8_t terraforming_turns;
    uint8_t order_auto_type; // see VehOrdersAutoType
    uint8_t visibility; // faction bitfield of who can currently see Veh excluding owner
    uint8_t moves_expended; // stored as road moves (x3)
    int8_t unk_5; // related to movement
    uint8_t unk_6; // automated action iterator count
    uint8_t move_to_ai_type; // shown as status icon with debug mode active
    // 000 00 000 : framed faction : secondary options : primary action id (0-7)
    // secondary options: THOUGHTMENU, ADVVIRUS, DECIPHER, SUBVERTMENU
    // 000 0 0000 : framed faction : n/a : probe action id (8) ; Freeing Captured Leaders only
    uint8_t probe_action; // see above and ProbePrimaryAction, last action taken by probe team
    uint8_t probe_sabotage_id; // for targeted sabotage: production: 0, abort: 99, or facility id
    int16_t home_base_id;
    int16_t next_veh_id_stack;
    int16_t prev_veh_id_stack;
};

static_assert(sizeof(Veh) == 0x34,  // yitzi 0x34
              "Veh layout must match the original executable");

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
    int8_t unk_1; // some kind of internal prototype category?
    uint8_t obsolete_factions; // faction bitfield of those who marked this prototype obsolete
    int8_t combat_factions; // faction bitfield for those that have seen it in combat (atk/def)
    int8_t icon_offset;
    int8_t padding; // unused
    uint16_t flags; // see veh_prototype_flag_bitfield
    int16_t preq_tech; // only set for predefined alpha/x.txt units
};

static_assert(sizeof(VehPrototype) == 0x34,  // yitzi 0x34
              "VehPrototype layout must match the original executable");

struct RulesReactor {
    LPSTR name;
    LPSTR name_short;
    int16_t preq_tech;
    uint16_t power; // Bug fix: this value isn't originally set
};

struct RulesAbility {
    LPSTR name;
    LPSTR description;
    LPSTR abbreviation;
    int cost_factor;
    int unk_1; // only referenced in NetDaemon::synch
    uint32_t flags;
    int16_t preq_tech;
    uint16_t padding;
};

struct RulesMorale {
    LPSTR name;
    LPSTR name_lifecycle;
};

struct RulesCombatMode {
    LPSTR name;
    LPSTR hyphened;
    LPSTR abbrev;
    LPSTR letter;
};

struct RulesOrder {
    LPSTR order;
    LPSTR order_sea;
    LPSTR letter;
};

static const int MaxChassisNum = 9;
static const int MaxWeaponNum = 26;
static const int MaxArmorNum = 14;
static const int MaxVehProtoNum = 512;
static const int MaxVehProtoFactionNum = 64; // Limit per faction + max predefined from alpha/x.txt
static const int MaxReactorNum = 4;
static const int MaxAbilityNum = 29;
static const int MaxMoraleNum = 7;
static const int MaxDefenseModeNum = 3;
static const int MaxOffenseModeNum = 3;
static const int MaxOrderNum = 30;
static const int MaxPlanNum = 15;
static const int MaxTriadNum = 3;

VehPrototype *const VehPrototypes = (VehPrototype *)0x009AB868;
Veh *const Vehs = (Veh *)0x00952828;
RulesChassis *const Chassis = (RulesChassis *)0x0094A330;
RulesWeapon *const Weapon = (RulesWeapon *)0x0094AE60;
RulesArmor *const Armor = (RulesArmor *)0x0094F278;
RulesReactor *const Reactor = (RulesReactor *)0x009527F8;
RulesAbility *const Ability = (RulesAbility *)0x009AB538;
RulesMorale *const Morale = (RulesMorale *)0x00952328;
RulesCombatMode *const DefenseModes = (RulesCombatMode *)0x00946A00;
RulesCombatMode *const OffenseModes = (RulesCombatMode *)0x00946178;
RulesOrder *const Order = (RulesOrder *)0x0096C878;
LPSTR *const PlansShortName = (LPSTR *)0x00945FE0;
LPSTR *const PlansFullName = (LPSTR *)0x00952360;
LPSTR *const Triad = (LPSTR *)0x0094F1A8;

extern int VehCurrentCount;
extern int VehDropLiftVehID;
extern int VehLiftX;
extern int VehLiftY;
extern BOOL VehBitError;
uint32_t *const VehBasicBattleMorale = (uint32_t *)0x00912420;
extern int VehMoraleModifierCount;
// Battle related globals
uint32_t *const VehBattleModCount = (uint32_t *)0x00915614;
BOOL *const VehBattleUnkTgl = (BOOL *)0x0091561C;
int *const VehBattleModifier = (int *)0x009155F0;
LPSTR *const VehBattleDisplay = (LPSTR *)0x0090F554;
extern LPSTR VehBattleDisplayTerrain;

void __cdecl say_morale(LPSTR morale_output, uint32_t veh_id, int faction_id_vs_native);
void __cdecl say_morale(uint32_t veh_id, int faction_id_vs_native);
int __cdecl drop_range(int faction_id);
// `int` throughout, per the catalogue: the original exports this as
// `?defense_value@@YAHHHHHH@Z` - all `H`, no `I`.
int __cdecl defense_value(int faction_id, int x, int y, int veh_id_def,
                                    int veh_id_atk);
uint32_t __cdecl morale_alien(int veh_id, int faction_id_vs_native);
// ?psi_factor@@YAHHHHH@Z - the faction id is `int`, as the definition has it.
int __cdecl psi_factor(int combat_ratio, int faction_id, BOOL is_attack,
                                 BOOL is_fungal_twr);
// `int` throughout, per the catalogue: the original exports these as
// `?get_basic_offense@@YAHHHHHH@Z` and `?get_basic_defense@@YAHHHHH@Z`, all
// `H`. The `uint32_t` parameters here decorated to `I` and made each of these
// a second overload alongside the catalogued definition in veh.cpp, which is
// what the C2666 ambiguities at every call site were.
int __cdecl get_basic_offense(int veh_id_atk, int veh_id_def,
                                        int psi_combat_type, BOOL is_bombard, BOOL unk_tgl);
int __cdecl get_basic_defense(int veh_id_def, int veh_id_atk,
                                        int psi_combat_type, BOOL is_bombard);
void __cdecl add_bat(int type, int modifier, LPCSTR display_str);
void __cdecl battle_compute(int veh_id_atk, int veh_id_def, int *offense_out, 
                                      int *defense_out, int combat_type);
uint32_t __cdecl best_defender(int veh_id_def, int veh_id_atk, BOOL check_artillery);
void __cdecl invasions(int base_id);
void __cdecl set_course(int veh_id, char type, int x, int y);
int __cdecl veh_top(int veh_id);
BOOL __cdecl want_monolith(int veh_id);
int __cdecl arm_strat(int armor_id, int faction_id);
int __cdecl weap_strat(int weapon_id, int faction_id);
int __cdecl weap_val(int proto_id, int faction_id);
int __cdecl arm_val(int armor_id, int faction_id);
int __cdecl armor_val(int proto_id, int faction_id);
uint32_t __cdecl transport_val(uint32_t chassis_id, uint32_t ability, 
                                         uint32_t reactor_id);
void __cdecl say_offense(LPSTR stat, int proto_id);
void __cdecl say_defense(LPSTR stat, int proto_id);
void __cdecl say_stats_3(LPSTR stat, uint32_t proto_id);
void __cdecl say_stats_3(uint32_t proto_id);
void __cdecl say_stats_2(LPSTR stat, int proto_id);
void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer);
uint32_t __cdecl best_reactor(int faction_id);
int __cdecl pick_chassis(int faction_id, int triad_chk, int speed_chk);
int __cdecl weapon_budget(int faction_id, int condition, BOOL check_mode);
int __cdecl armor_budget(int faction_id, int max_cost);
// ?hex_cost@@YAHHHHHHHH@Z - eight H. The coordinates are int.
int __cdecl hex_cost(int proto_id, int faction_id, int x_src, int y_src,
                               int x_dst, int y_dst, BOOL toggle);
int __cdecl veh_health(int veh_id);
// `int` throughout, matching the definition in src/veh.cpp. The `uint32_t`
// spelling that stood here declared an overload NOTHING defines - harmless
// while every caller lived in veh.cpp, and an unresolved symbol the moment
// `base_cost` moved into this header and resolved to it.
uint32_t __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id,
                            int ability, int reactor_id);
// `int` throughout: the definition in src/veh.cpp already is, and the
// catalogue exports ?make_proto@@YAXHHHHHH@Z - all H, no I.
void __cdecl make_proto(int proto_id, int chassis_id, int weapon_id,
                                  int armor_id, int ability, int reactor_id);
int __cdecl get_plan(int faction_id, int plan);
void __cdecl spot_tile(int x, int y, int faction_id);
void __cdecl spot_base(int base_id, int faction_id);
void __cdecl spot_stack(int veh_id, int faction_id);
void __cdecl spot_loc(int x, int y, int faction_id);
BOOL __cdecl want_to_wake(int faction_id, int veh_id, int spotted_veh_id);
void __cdecl wake_stack(int veh_id);
void __cdecl stack_put(int veh_id, int x, int y);
void __cdecl stack_sort(int veh_id);
void __cdecl stack_sort_2(int veh_id);
int __cdecl stack_fix(int veh_id);
int __cdecl stack_veh(int veh_id, int mode);
int __cdecl stack_check(int veh_id, int type, int cond1, int cond2, int cond3);
BOOL __cdecl veh_avail(int proto_id, int faction_id, int base_id);
BOOL __cdecl wants_prototype(int proto_id, int faction_id);
int __cdecl veh_at(int x, int y);
BOOL __cdecl has_abil(int proto_id, int ability_id);
int __cdecl veh_lift(int veh_id);
int __cdecl veh_drop(int veh_id, int x, int y);
void __cdecl veh_demote(int veh_id);
void __cdecl veh_promote(int veh_id);
void __cdecl veh_clear(int veh_id, int proto_id, int faction_id);
BOOL __cdecl can_arty(int proto_id, BOOL sea_triad_retn);
// ?morale_veh@@YAHHHH@Z - returns int and takes int, as the definition does.
int __cdecl morale_veh(int veh_id, BOOL check_drone_riot,
                                 int faction_id_vs_native);
int __cdecl offense_proto(int proto_id, int veh_id_def, BOOL is_bombard);
int __cdecl armor_proto(int proto_id, int veh_id_atk, BOOL is_bombard);
int __cdecl speed_proto(int proto_id);
int __cdecl valid_patrol(int veh_id, int x, int y);
int __cdecl speed(int veh_id, BOOL skip_morale);
int __cdecl veh_cargo(int veh_id);
// A REAL, out-of-line forwarder to the `MEASURED inline sleep()` below - its
// one caller (stack_veh, 0x005B8EE0) is a genuine `call` in the image, not
// the folded field writes every other spot gets from `sleep` inlining.
void __cdecl sleep_call(int veh_id);
int __cdecl prototype_factor(int proto_id);
int __cdecl veh_cost(int proto_id, int base_id, BOOL *has_proto_cost);
BOOL __cdecl veh_jail(int veh_id);
int __cdecl veh_wake(int veh_id);
int __cdecl action_home(int veh_id, int flags);
int __cdecl compute_odds(int odds, int faction_id, int veh_id_atk, int veh_id_def,
                                  int base_id);
int __cdecl alien_base(int veh_id, int x, int y);

MEASURED inline uint32_t __cdecl planet_buster2(int proto_id) {
    return VehPrototypes[proto_id].plan == PLAN_PLANET_BUSTER 
        ? VehPrototypes[proto_id].reactor_id : 0;
}

MEASURED inline uint32_t __cdecl planet_buster(int veh_id) {
    return planet_buster2(Vehs[veh_id].proto_id);
}

MEASURED inline void __cdecl battle_init() {
    VehBattleModCount[0] = 0;
    VehBattleModCount[1] = 0;
    VehBattleUnkTgl[0] = false;
    VehBattleUnkTgl[1] = false;
}

MEASURED inline int __cdecl veh_moves(int veh_id) {
    return range(speed(veh_id, false) - Vehs[veh_id].moves_expended, 0, 999);
}

MEASURED inline int __cdecl abil_index(int ability_id) {
    int index = 0;
    for (int check = ability_id; !(check & 1); index++) {
        check >>= 1;
    }
    return index;
}

MEASURED inline void __cdecl veh_put(int veh_id, int x, int y) {
    veh_drop(veh_lift(veh_id), x, y);
}

MEASURED inline int __cdecl base_cost(int proto_id) {
    return proto_cost(VehPrototypes[proto_id].chassis_id, VehPrototypes[proto_id].weapon_id,
        VehPrototypes[proto_id].armor_id, 0, VehPrototypes[proto_id].reactor_id);
}

MEASURED inline void __cdecl go_to(int veh_id, char type, int x, int y) {
    Vehs[veh_id].order = ORDER_MOVE_TO;
    Vehs[veh_id].move_to_ai_type = type;
    Vehs[veh_id].waypoint_x[0] = (int16_t)x;
    Vehs[veh_id].waypoint_y[0] = (int16_t)y;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_COLONIZATION) {
        Vehs[veh_id].terraforming_turns = 0;
    }
}

MEASURED inline void __cdecl sleep(int veh_id) {
    Vehs[veh_id].order = ORDER_SENTRY_BOARD;
    Vehs[veh_id].waypoint_x[0] = -1;
    Vehs[veh_id].waypoint_y[0] = 0;
}

MEASURED inline void __cdecl veh_skip(int veh_id) {
    // TODO Bug: Due to size of moves_expended, speeds over 255 will be incorrect. The speed()
    //           function can return a value from 1-999. Eventually increase size to 16 bits.
    Vehs[veh_id].moves_expended = (uint8_t)speed(veh_id, false);
}

MEASURED inline int __cdecl veh_fake(int proto_id, int faction_id) {
    veh_clear(2048, proto_id, faction_id);
    return 2048;
}

// BODY HERE (was a plain, non-`inline` function in veh.cpp): neither of its
// two callers (best_defender, action_home) has it in their `calls` list -
// both inline it - and a plain function is never an /Ob1 inline candidate.
// The `// ORIGINAL:` marker stays in veh.cpp.
MEASURED inline int __cdecl proto_power(int veh_id) {
    int proto_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    return range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
}

// INLINE: the image has no get_proto_triad - it open-codes what this names.
inline uint8_t __cdecl get_proto_triad(uint32_t proto_id) {
    return Chassis[VehPrototypes[proto_id].chassis_id].triad;
}

// INLINE: the image has no get_triad - it open-codes what this names.
inline uint8_t __cdecl get_triad(uint32_t veh_id) {
    return get_proto_triad(Vehs[veh_id].proto_id);
}

// INLINE: the image has no get_proto_offense_rating - it open-codes what this names.
inline int8_t __cdecl get_proto_offense_rating(uint32_t proto_id) {
    return Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
}

// INLINE: the image has no get_offense_rating - it open-codes what this names.
inline int8_t __cdecl get_offense_rating(uint32_t veh_id) {
    return get_proto_offense_rating(Vehs[veh_id].proto_id);
}

// INLINE: the image has no get_proto_defense_rating - it open-codes what this names.
inline int8_t __cdecl get_proto_defense_rating(uint32_t proto_id) {
    return Armor[VehPrototypes[proto_id].armor_id].defense_rating;
}

// INLINE: the image has no get_defense_rating - it open-codes what this names.
inline int8_t __cdecl get_defense_rating(uint32_t veh_id) {
    return get_proto_defense_rating(Vehs[veh_id].proto_id);
}

// INLINE: the image has no is_proto_missile - it open-codes what this names.
inline BOOL __cdecl is_proto_missile(uint32_t proto_id) {
    return Chassis[VehPrototypes[proto_id].chassis_id].missile;
}

// INLINE: the image has no is_missile - it open-codes what this names.
inline BOOL __cdecl is_missile(uint32_t veh_id) {
    return is_proto_missile(Vehs[veh_id].proto_id);
}
