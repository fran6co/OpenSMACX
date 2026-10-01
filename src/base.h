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
void __cdecl action_staple(int base_id);

#include "general.h"  // bitmask, which has_fac_built below inlines

class Buffer;

 /*
  * Base related objects, variables and functions.
  */
enum FacilityId {
    FAC_HEADQUARTERS = 1,
    FAC_CHILDREN_CRECHE = 2,
    FAC_RECYCLING_TANKS = 3,
    FAC_PERIMETER_DEFENSE = 4,
    FAC_TACHYON_FIELD = 5,
    FAC_RECREATION_COMMONS = 6,
    FAC_ENERGY_BANK = 7,
    FAC_NETWORK_NODE = 8,
    FAC_BIOLOGY_LAB = 9,
    FAC_SKUNKWORKS = 10,
    FAC_HOLOGRAM_THEATRE = 11,
    FAC_PARADISE_GARDEN = 12,
    FAC_TREE_FARM = 13,
    FAC_HYBRID_FOREST = 14,
    FAC_FUSION_LAB = 15,
    FAC_QUANTUM_LAB = 16,
    FAC_RESEARCH_HOSPITAL = 17,
    FAC_NANOHOSPITAL = 18,
    FAC_ROBOTIC_ASSEMBLY_PLANT = 19,
    FAC_NANOREPLICATOR = 20,
    FAC_QUANTUM_CONVERTER = 21,
    FAC_GENEJACK_FACTORY = 22,
    FAC_PUNISHMENT_SPHERE = 23,
    FAC_HAB_COMPLEX = 24,
    FAC_HABITATION_DOME = 25,
    FAC_PRESSURE_DOME = 26,
    FAC_COMMAND_CENTER = 27,
    FAC_NAVAL_YARD = 28,
    FAC_AEROSPACE_COMPLEX = 29,
    FAC_BIOENHANCEMENT_CENTER = 30,
    FAC_CENTAURI_PRESERVE = 31,
    FAC_TEMPLE_OF_PLANET = 32,
    FAC_PSI_GATE = 33,
    FAC_COVERT_OPS_CENTER = 34,
    FAC_BROOD_PIT = 35,
    FAC_AQUAFARM = 36,
    FAC_SUBSEA_TRUNKLINE = 37,
    FAC_THERMOCLINE_TRANSDUCER = 38,
    FAC_FLECHETTE_DEFENSE_SYS = 39,
    FAC_SUBSPACE_GENERATOR = 40,
    FAC_GEOSYNC_SURVEY_POD = 41,
    FAC_EMPTY_FACILITY_42 = 42,
    FAC_EMPTY_FACILITY_43 = 43,
    FAC_EMPTY_FACILITY_44 = 44,
    FAC_EMPTY_FACILITY_45 = 45,
    FAC_EMPTY_FACILITY_46 = 46,
    FAC_EMPTY_FACILITY_47 = 47,
    FAC_EMPTY_FACILITY_48 = 48,
    FAC_EMPTY_FACILITY_49 = 49,
    FAC_EMPTY_FACILITY_50 = 50,
    FAC_EMPTY_FACILITY_51 = 51,
    FAC_EMPTY_FACILITY_52 = 52,
    FAC_EMPTY_FACILITY_53 = 53,
    FAC_EMPTY_FACILITY_54 = 54,
    FAC_EMPTY_FACILITY_55 = 55,
    FAC_EMPTY_FACILITY_56 = 56,
    FAC_EMPTY_FACILITY_57 = 57,
    FAC_EMPTY_FACILITY_58 = 58,
    FAC_EMPTY_FACILITY_59 = 59,
    FAC_EMPTY_FACILITY_60 = 60,
    FAC_EMPTY_FACILITY_61 = 61,
    FAC_EMPTY_FACILITY_62 = 62,
    FAC_EMPTY_FACILITY_63 = 63,
    FAC_EMPTY_FACILITY_64 = 64,
    FAC_SKY_HYDRO_LAB = 65,
    FAC_NESSUS_MINING_STATION = 66,
    FAC_ORBITAL_POWER_TRANS = 67,
    FAC_ORBITAL_DEFENSE_POD = 68,
    FAC_STOCKPILE_ENERGY = 69,
    FAC_HUMAN_GENOME_PROJ = 70,
    FAC_COMMAND_NEXUS = 71,
    FAC_WEATHER_PARADIGM = 72,
    FAC_MERCHANT_EXCHANGE = 73,
    FAC_EMPATH_GUILD = 74,
    FAC_CITIZENS_DEFENSE_FORCE = 75,
    FAC_VIRTUAL_WORLD = 76,
    FAC_PLANETARY_TRANS_SYS = 77,
    FAC_XENOEMPATHY_DOME = 78,
    FAC_NEURAL_AMPLIFIER = 79,
    FAC_MARITIME_CONTROL_CENTER = 80,
    FAC_PLANETARY_DATALINKS = 81,
    FAC_SUPERCOLLIDER = 82,
    FAC_ASCETIC_VIRTUES = 83,
    FAC_LONGEVITY_VACCINE = 84,
    FAC_HUNTER_SEEKER_ALGO = 85,
    FAC_PHOLUS_MUTAGEN = 86,
    FAC_CYBORG_FACTORY = 87,
    FAC_THEORY_OF_EVERYTHING = 88,
    FAC_DREAM_TWISTER = 89,
    FAC_UNIVERSAL_TRANSLATOR = 90,
    FAC_NETWORK_BACKBONE = 91,
    FAC_NANO_FACTORY = 92,
    FAC_LIVING_REFINERY = 93,
    FAC_CLONING_VATS = 94,
    FAC_SELF_AWARE_COLONY = 95,
    FAC_CLINICAL_IMMORTALITY = 96,
    FAC_SPACE_ELEVATOR = 97,
    FAC_SINGULARITY_INDUCTOR = 98,
    FAC_BULK_MATTER_TRANSMITTER = 99,
    FAC_TELEPATHIC_MATRIX = 100,
    FAC_VOICE_OF_PLANET = 101,
    FAC_ASCENT_TO_TRANSCENDENCE = 102,
    FAC_MANIFOLD_HARMONICS = 103,
    FAC_NETHACK_TERMINUS = 104,
    FAC_CLOUDBASE_ACADEMY = 105,
    FAC_PLANETARY_ENERGY_GRID = 106,
    FAC_EMPTY_SP_38 = 107,
    FAC_EMPTY_SP_39 = 108,
    FAC_EMPTY_SP_40 = 109,
    FAC_EMPTY_SP_41 = 110,
    FAC_EMPTY_SP_42 = 111,
    FAC_EMPTY_SP_43 = 112,
    FAC_EMPTY_SP_44 = 113,
    FAC_EMPTY_SP_45 = 114,
    FAC_EMPTY_SP_46 = 115,
    FAC_EMPTY_SP_47 = 116,
    FAC_EMPTY_SP_48 = 117,
    FAC_EMPTY_SP_49 = 118,
    FAC_EMPTY_SP_50 = 119,
    FAC_EMPTY_SP_51 = 120,
    FAC_EMPTY_SP_52 = 121,
    FAC_EMPTY_SP_53 = 122,
    FAC_EMPTY_SP_54 = 123,
    FAC_EMPTY_SP_55 = 124,
    FAC_EMPTY_SP_56 = 125,
    FAC_EMPTY_SP_57 = 126,
    FAC_EMPTY_SP_58 = 127,
    FAC_EMPTY_SP_59 = 128,
    FAC_EMPTY_SP_60 = 129,
    FAC_EMPTY_SP_61 = 130,
    FAC_EMPTY_SP_62 = 131,
    FAC_EMPTY_SP_63 = 132,
    FAC_EMPTY_SP_64 = 133,
};

enum  SecretProjectId {
    SP_HUMAN_GENOME_PROJ = 0,
    SP_COMMAND_NEXUS = 1,
    SP_WEATHER_PARADIGM = 2,
    SP_MERCHANT_EXCHANGE = 3,
    SP_EMPATH_GUILD = 4,
    SP_CITIZENS_DEFENSE_FORCE = 5,
    SP_VIRTUAL_WORLD = 6,
    SP_PLANETARY_TRANS_SYS = 7,
    SP_XENOEMPATYH_DOME = 8,
    SP_NEURAL_AMPLIFIER = 9,
    SP_MARITIME_CONTROL_CENTER = 10,
    SP_PLANETARY_DATALINKS = 11,
    SP_SUPERCOLLIDER = 12,
    SP_ASCETIC_VIRTUES = 13,
    SP_LONGEVITY_VACCINE = 14,
    SP_HUNTER_SEEKER_ALGO = 15,
    SP_PHOLUS_MUTAGEN = 16,
    SP_CYBORG_FACTORY = 17,
    SP_THEORY_OF_EVERYTHING = 18,
    SP_DREAM_TWISTER = 19,
    SP_UNIVERSAL_TRANSLATOR = 20,
    SP_NETWORK_BACKBONE = 21,
    SP_NANO_FACTORY = 22,
    SP_LIVING_REFINERY = 23,
    SP_CLONING_VATS = 24,
    SP_SELF_AWARE_COLONY = 25,
    SP_CLINICAL_IMMORTALITY = 26,
    SP_SPACE_ELEVATOR = 27,
    SP_SINGULARITY_INDUCTOR = 28,
    SP_BULK_MATTER_TRANSMITTER = 29,
    SP_TELEPATHIC_MATRIX = 30,
    SP_VOICE_OF_PLANET = 31,
    SP_ASCENT_TO_TRANSCENDENCE = 32,
    SP_MANIFOLD_HARMONICS = 33,
    SP_NETHACK_TERMINUS = 34,
    SP_CLOUDBASE_ACADEMY = 35,
    SP_PLANETARY_ENERGY_GRID = 36,
    SP_EMPTY_38 = 37,
    SP_EMPTY_39 = 38,
    SP_EMPTY_40 = 39,
    SP_EMPTY_41 = 40,
    SP_EMPTY_42 = 41,
    SP_EMPTY_43 = 42,
    SP_EMPTY_44 = 43,
    SP_EMPTY_45 = 44,
    SP_EMPTY_46 = 45,
    SP_EMPTY_47 = 46,
    SP_EMPTY_48 = 47,
    SP_EMPTY_49 = 48,
    SP_EMPTY_50 = 49,
    SP_EMPTY_51 = 50,
    SP_EMPTY_52 = 51,
    SP_EMPTY_53 = 52,
    SP_EMPTY_54 = 53,
    SP_EMPTY_55 = 54,
    SP_EMPTY_56 = 55,
    SP_EMPTY_57 = 56,
    SP_EMPTY_58 = 57,
    SP_EMPTY_59 = 58,
    SP_EMPTY_60 = 59,
    SP_EMPTY_61 = 60,
    SP_EMPTY_62 = 61,
    SP_EMPTY_63 = 62,
    SP_EMPTY_64 = 63,
};

enum BaseStateBitfield {
    BSTATE_UNK_1 = 0x1,
    BSTATE_DRONE_RIOTS_ACTIVE = 0x2,
    BSTATE_GOLDEN_AGE_ACTIVE = 0x4,
    BSTATE_COMBAT_LOSS_LAST_TURN = 0x8, // Reset on upkeep
    BSTATE_UNK_10 = 0x10,
    BSTATE_UNK_20 = 0x20,
    BSTATE_RESEARCH_DATA_STOLEN = 0x40,
    BSTATE_UNK_80 = 0x80,
    BSTATE_UNK_100 = 0x100,
    BSTATE_FACILITY_SCRAPPED = 0x200, // Only one facility can be scrapped/recycled per turn
    BSTATE_ARTIFACT_LINKED = 0x400, // Alien Artifact linked to Network Node
    BSTATE_ARTIFACT_ALREADY_LINKED = 0x800, // Only show already linked alert once
    BSTATE_UNK_1000 = 0x1000,
    BSTATE_UNK_2000 = 0x2000,
    BSTATE_UNK_4000 = 0x4000,
    BSTATE_UNK_8000 = 0x8000,
    BSTATE_UNK_10000 = 0x10000,
    BSTATE_GENETIC_PLAGUE_INTRO = 0x20000,
    BSTATE_ASSISTANT_KILLER_HOME = 0x40000, // Veh home base (or closest) of Assistant worm killer
    BSTATE_UNK_80000 = 0x80000,
    BSTATE_UNK_100000 = 0x100000,
    BSTATE_UNK_200000 = 0x200000,
    BSTATE_ENERGY_RESERVES_DRAINED = 0x400000,
    BSTATE_PRODUCTION_DONE = 0x800000,
    BSTATE_UNK_1000000 = 0x1000000,
    BSTATE_UNK_2000000 = 0x2000000,
    BSTATE_UNK_4000000 = 0x4000000,
    BSTATE_UNK_8000000 = 0x8000000,
    BSTATE_NET_LOCKED = 0x10000000,
    BSTATE_PSI_GATE_USED = 0x20000000,
    BSTATE_PRODUCTION_HURRIED = 0x40000000,
    BSTATE_UNK_8000000000 = 0x80000000,
};

enum BaseEventBitfield {
    BEVENT_UNK_100 = 0x100,
    BEVENT_BUMPER = 0x200,
    BEVENT_FAMINE = 0x400,
    BEVENT_INDUSTRY = 0x800,
    BEVENT_BUST = 0x1000,
    BEVENT_HEAT_WAVE = 0x2000,
    BEVENT_CLOUD_COVER = 0x4000,
    BEVENT_OBJECTIVE = 0x8000,
};

enum GovernorBaseBitfield {
    GOV_MANAGES_PRODUCTION = 0x1,
    // 0x2
    // 0x4
    // 0x8
    // 0x10
    GOV_MAY_HURRY_PRODUCTION = 0x20,
    GOV_MANAGES_CITIZENS_SPECS = 0x40, // incl. drone riots
    GOV_NEW_VEH_FULLY_AUTO = 0x80,
    // 0x100
    GOV_MAY_PROD_LAND_COMBAT = 0x200,
    GOV_MAY_PROD_NAVAL_COMBAT = 0x400,
    GOV_MAY_PROD_AIR_COMBAT = 0x800,
    GOV_MAY_PROD_LAND_DEFENS = 0x1000,
    GOV_MAY_PROD_AIR_DEFENS = 0x2000,
    // 0x4000 ; Naval defense?
    GOV_MAY_PROD_TERRAFORMS = 0x8000,
    GOV_MAY_PROD_FACILITIES = 0x10000,
    GOV_MAY_PROD_COLONY_POD = 0x20000,
    GOV_MAY_PROD_SP = 0x40000,
    GOV_MAY_PROD_PROTOTYPE = 0x80000,
    GOV_MAY_PROD_PROBES = 0x100000,
    GOV_MULTI_PRIORITIES = 0x200000, // or no priorities
    GOV_MAY_PROD_EXPLR_VEH = 0x400000, // scout/exploration units
    GOV_MAY_PROD_TRANSPORT = 0x800000,
    GOV_PRIORITY_EXPLORE = 0x1000000,
    GOV_PRIORITY_DISCOVER = 0x2000000,
    GOV_PRIORITY_BUILD = 0x4000000,
    GOV_PRIORITY_CONQUER = 0x8000000,
    // 0x10000000
    // 0x20000000
    GOV_UNK_40000000 = 0x40000000,
    GOV_ACTIVE = 0x80000000,
};

enum CitizenSpecialists {
    CITIZEN_TECHNICIAN = 0,
    CITIZEN_DOCTOR = 1,
    CITIZEN_LIBRARIAN = 2,
    CITIZEN_ENGINEER = 3,
    CITIZEN_EMPATH = 4,
    CITIZEN_THINKER = 5,
    CITIZEN_TRANSCEND = 6,
};

struct Base {
    // farm_compute loads these through BaseCurrent at 0x004E68BE..004E68CA.
    // RECOVERY_MEMBER: v1 Base x 0x0 0x004E68B0
    int16_t x;
    // RECOVERY_MEMBER: v1 Base y 0x2 0x004E68B0
    int16_t y;
    // RECOVERY_MEMBER: v1 Base faction_id_current 0x4 0x004E68B0
    uint8_t faction_id_current;
    uint8_t faction_id_former;
    // RECOVERY_MEMBER: v1 Base population_size 0x6 0x004E80B0
    int8_t population_size;
    // RECOVERY_MEMBER: v1 Base assimilation_turns_left 0x7 0x004EA540
    uint8_t assimilation_turns_left;
    // RECOVERY_MEMBER: v1 Base nerve_staple_turns_left 0x8 0x004EA540
    uint8_t nerve_staple_turns_left;
    uint8_t unk_1; // AI plan state?
    uint8_t visibility; // faction bitfield of those who can see base (mapped: dim/bright)
    uint8_t faction_pop_size_intel[8]; // last know population size for each faction
    char name_string[25];
    int16_t unk_x; // terraforming related
    int16_t unk_y; // terraforming related
    // RECOVERY_MEMBER: v1 Base state 0x30 0x004E80B0
    uint32_t state; // see base_state_bitfield
    uint32_t event; // see base_event_bitfield
    // RECOVERY_MEMBER: v1 Base governor 0x38 0x004E80B0
    uint32_t governor; // see governor_base_bitfield
    int nutrients_accumulated;
    int minerals_accumulated;
    int production_id_last;
    // RECOVERY_MEMBER: v1 Base eco_damage 0x48 0x004E80B0
    int eco_damage;
    uint32_t queue_size;
    // RECOVERY_MEMBER: v1 Base queue_production_id 0x50 0x004E80B0
    int queue_production_id[10]; // items in the production queue
    // Original 0x004E6C7A tests neighboring bases' worked-square bits here.
    // RECOVERY_MEMBER: v1 Base resource_sqr_radius 0x78 0x004E68B0
    int resource_sqr_radius; // worked tiles
    // RECOVERY_MEMBER: v1 Base specialist_total 0x7C 0x004E80B0
    int specialist_total;
    // RECOVERY_MEMBER: v1 Base specialist_unk_1 0x80 0x004E80B0
    // Allocation scratch count: initially the existing non-psych specialists,
    // then the slots assigned by the weighted specialist search. Those new
    // slots may have psych bonuses too; this is not a permanent non-psych total.
    int specialist_unk_1;
    /*
    * Specialist types for the first 16 (see citizen_specialists, 4 bits per specialist)
    * Stored from left to right in base view, with left most represented by lower bits of spec
    * Specialists after 16th in base are auto assigned based on best_specialist()
    * Isn't correct when specialist_total is 0, not reset? Or Governor actively controlling specs?
    */
    // RECOVERY_MEMBER: v1 Base spec 0x84 0x004E80B0
    uint32_t spec[2]; // specialist types
    // RECOVERY_MEMBER: v1 Base facilities_built 0x8C 0x004E80B0
    uint8_t facilities_built[12];
    int mineral_surplus_final;
    int minerals_accumulated_2; // original minerals before retooling?
    int unk_6; // unused?
    int unk_7; // unused?
    int unk_8; // unused?
    int unk_9; // unused?
    // RECOVERY_MEMBER: v1 Base nutrient_intake_1 0xB0 0x004E80B0
    int nutrient_intake_1;
    // RECOVERY_MEMBER: v1 Base mineral_intake_1 0xB4 0x004E80B0
    int mineral_intake_1;
    // RECOVERY_MEMBER: v1 Base energy_intake_1 0xB8 0x004E80B0
    int energy_intake_1;
    // RECOVERY_MEMBER: v1 Base unused_intake_1 0xBC 0x004E80B0
    int unused_intake_1;
    // RECOVERY_MEMBER: v1 Base nutrient_intake_2 0xC0 0x004E80B0
    int nutrient_intake_2;
    // RECOVERY_MEMBER: v1 Base mineral_intake_2 0xC4 0x004E80B0
    int mineral_intake_2;
    // RECOVERY_MEMBER: v1 Base energy_intake_2 0xC8 0x004E80B0
    int energy_intake_2;
    // RECOVERY_MEMBER: v1 Base unused_intake_2 0xCC 0x004E80B0
    int unused_intake_2;
    int nutrient_surplus;
    int mineral_surplus;
    int energy_surplus;
    int unused_surplus;
    int nutrient_inefficiency;
    int mineral_inefficiency;
    int energy_inefficiency;
    int unused_inefficiency;
    int nutrient_consumption;
    int mineral_consumption;
    int energy_consumption;
    int unused_consumption;
    int economy_total;
    // RECOVERY_MEMBER: v1 Base psych_total 0x104 0x004EA540
    int psych_total;
    int labs_total;
    int unk_10;
    int16_t auto_forward_land_base_id;
    int16_t auto_forward_sea_base_id;
    int16_t auto_forward_air_base_id;
    int16_t padding_0;
    // RECOVERY_MEMBER: v1 Base talent_total 0x118 0x004EA540
    int talent_total;
    // RECOVERY_MEMBER: v1 Base drone_total 0x11C 0x004EA540
    int drone_total;
    // RECOVERY_MEMBER: v1 Base super_drone_total 0x120 0x004EA540
    int super_drone_total;
    int random_event_turns;
    int nerve_staple_count;
    int unk_11; // effectively unused, only set to 0 by base_init()
    // base_reset reads this stored site value at 0x004E5FE0.
    // RECOVERY_MEMBER: v1 Base location 0x130 0x004E5FE0
    int location;
};

static_assert(sizeof(Base) == 0x134, "Base layout must match terranx.exe");  // yitzi 0x134

struct RulesFacility {
    LPSTR name;
    LPSTR effect;
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

static_assert(sizeof(RulesFacility) == 0x30,  // yitzi 0x30
              "RulesFacility layout must match the original executable");

struct RulesCitizen {
    LPSTR singular_name;
    LPSTR plural_name;
    // RECOVERY_MEMBER: v1 RulesCitizen preq_tech 0x8 0x004E80B0
    int preq_tech;
    // RECOVERY_MEMBER: v1 RulesCitizen obsol_tech 0xC 0x004E80B0
    int obsol_tech;
    // RECOVERY_MEMBER: v1 RulesCitizen ops_bonus 0x10 0x004E80B0
    int ops_bonus;
    // RECOVERY_MEMBER: v1 RulesCitizen psych_bonus 0x14 0x004E80B0
    int psych_bonus;
    // RECOVERY_MEMBER: v1 RulesCitizen research_bonus 0x18 0x004E80B0
    int research_bonus;
};

struct BaseSecretProject {
    int human_genome_project;
    int command_nexus;
    int weather_paradigm;
    int merchant_exchange;
    int empath_guild;
    int citizens_defense_force;
    int virtual_world;
    int planetary_transit_system;
    int xenoempathy_dome;
    int neural_amplifier;
    int maritime_control_center;
    int planetary_datalinks;
    int supercollider;
    int ascetic_virtues;
    int longevity_vaccine;
    int hunter_seeker_algorithm;
    int pholus_mutagen;
    int cyborg_factory;
    int theory_of_everything;
    int dream_twister;
    int universal_translator;
    int network_backbone;
    int nano_factory;
    int living_refinery;
    int cloning_vats;
    int self_aware_colony;
    int clinical_immortality;
    // RECOVERY_MEMBER: v1 BaseSecretProject space_elevator 0x6C 0x004E80B0
    int space_elevator;
    int singularity_inductor;
    int bulk_matter_transmitter;
    int telepathic_matrix;
    int voice_of_planet;
    int ascent_to_transcendence;
    int manifold_harmonics; // SMACX Only
    int nethack_terminus; // SMACX Only
    // RECOVERY_MEMBER: v1 BaseSecretProject cloudbase_academy 0x8C 0x004E80B0
    int cloudbase_academy; // SMACX Only
    int planetary_energy_grid; // SMACX Only
    int user_defined_38; // no direct ref in code logic?
    int user_defined_39;
    int user_defined_40;
    int user_defined_41;
    int user_defined_42;
    int user_defined_43;
    int user_defined_44;
    int user_defined_45;
    int user_defined_46;
    int user_defined_47;
    int user_defined_48;
    int user_defined_49;
    int user_defined_50;
    int user_defined_51;
    int user_defined_52;
    int user_defined_53;
    int user_defined_54;
    int user_defined_55;
    int user_defined_56;
    int user_defined_57;
    int user_defined_58;
    int user_defined_59;
    int user_defined_60;
    int user_defined_61;
    int user_defined_62;
    int user_defined_63;
    int user_defined_64;
};

static const int MaxBaseNum = 512;
static const int MaxFacilityNum = 134; // 0 slot unused
static const int MaxSecretProjectNum = 64;
static const int FacilitySPStart = 70; // Special Project start offset
static const int FacilityRepStart = 65; // Sky Hydro Lab; Nessus Mining Stat; OPT; ODP; Stockpile Eng
static const int MaxCitizenNum = 10;
static const int MaxSpecialistNum = 7;
static const int SP_Unbuilt = -1;
static const int SP_Destroyed = -2;

// Real storage in the image (.bss, filled by read_rules()/read_basic_rules()),
// defined in alpha.cpp.
extern RulesFacility Facility[MaxFacilityNum];      // 0x009A4B68
extern RulesCitizen Citizen[MaxCitizenNum];         // 0x00946020
extern Base Bases[MaxBaseNum];                      // 0x0097D040
extern BaseSecretProject SecretProject;             // 0x009A6514
extern int BaseIDCurrentSelected;
extern int BaseCurrentCount;
extern int BaseFindDist;
extern Base *BaseCurrent;
extern int BaseComputedID;
extern uint32_t ScnVictFacilityObj;
extern uint32_t BaseUpkeepStage;
extern uint32_t BaseCurrentConvoyFrom[4]; // resources convoyed from base
extern uint32_t BaseCurrentConvoyTo[4]; // resources convoyed/extracted to base
// Entries correspond to other-faction slots 1..7. base_energy publishes both
// sides of each commerce exchange; BaseWin::draw_commerce reads them.
extern int BaseCommerceReceipts[7];
extern int BaseCommercePartnerReceipts[7];
extern int BaseCurrentGrowthRate;
extern uint32_t BaseCurrentVehPacifismCount;
extern uint32_t BaseCurrentForcesSupported; // Forces Supported by Base
extern uint32_t BaseCurrentForcesMaintCost; // Minerals expended for Forces Supported maintenance
extern uint32_t BaseCurrentForcesMaintCount; // Forces past the free allowance; never read back
extern int TileYieldRestricted; // resource a square lost to the per-square restriction
extern uint32_t FarmStatus[25]; // per-position status; final four positions excluded
extern uint32_t FarmUnavailableMask; // nonzero status among the first 21 positions
extern int BasePopulationUnassigned; // remaining worker/specialist allocation count
extern int BaseNutrientAdequate; // allocation-path result, retained on successful reuse
extern int BasePsychTalents[5]; // social, spending, facilities, police, suppression
extern int BasePsychDrones[5];
extern int BasePsychSuperDrones[5];
extern int BaseSquareEnergy; // energy_yield's publication for a base square
extern int GovernorFaction; // elected Planetary Governor; see the note in base.cpp
extern int GlobalEnergyEventState; // energy tripling state driven by random_events
extern int UnkGlobal0093A934; // TODO: identify global; selects the bit above

BOOL __cdecl has_fac(int facility_id, int base_id, int queue_count);
void __cdecl farm_compute();
void __cdecl say_base(LPSTR base_str, int base_id);
int __cdecl base_at(int x, int y);
void __cdecl base_draw(Buffer *target, int base_id, int pixel_x, int pixel_y,
                       int style, int flags);
int __cdecl base_find(int x, int y);
// ?base_find@@YAHHHH@Z - three H.
int __cdecl base_find(int x, int y, int faction_id);
int __cdecl base_find(int x, int y, int faction_id, int region, int faction_id_2, 
                                int faction_id_3);
uint32_t  __cdecl best_specialist();
void __cdecl name_base(int faction_id, LPSTR name_out, BOOL is_final,
                                 BOOL is_sea_base);
void __cdecl base_mark(int base_id);
int __cdecl cost_factor(int faction_id, int rsc_type, int base_id);
int __cdecl base_making(int production_id, int base_id);
int __cdecl base_lose_minerals(int base_id, int UNUSED(production_id) production_id);
void __cdecl set_fac(int facility_id, int base_id, BOOL set);
BOOL __cdecl has_fac_announced(uint32_t faction_id, uint32_t facility_id);
void __cdecl set_fac_announced(uint32_t faction_id, uint32_t facility_id, BOOL set);
void __cdecl base_first(int base_id);
int __cdecl base_init(int faction_id, int x, int y);
void __cdecl base_kill(int base_id);
int __cdecl morale_mod(int base_id, int faction_id, int triad);
int __cdecl worm_mod(int base_id, int faction_id);
int __cdecl crop_yield(int faction_id, int base_id, int x, int y,
                                 BOOL assume_improved);
// ?resource_yield@@YAHHHHHH@Z - the three-terrain-yield dispatcher; RSC_PSI
// (3) falls through to clearing TileYieldRestricted.
int __cdecl resource_yield(int rsc_type, int faction_id, int base_id, int x,
                           int y);
int __cdecl mine_yield(int faction_id, int base_id, int x, int y,
                                 BOOL assume_improved);
int __cdecl energy_yield(int faction_id, int base_id, int x, int y,
                                   BOOL assume_improved);
void __cdecl base_support();
void __cdecl base_yield();
void __cdecl base_nutrient();
void __cdecl base_minerals();
void __cdecl base_energy();
void __cdecl base_compute(int force);
int __cdecl base_build(int base_id, int *arg2, int *arg3, int *arg4); // 0x004F81A0
void __cdecl base_change(int base_id, int production_id); // 0x004E5A60
void __cdecl base_reset(int base_id, int force);  // 0x004E5FE0
void __cdecl base_doctors();                     // 0x004F5F30
uint32_t __cdecl black_market(int energy);
void __cdecl base_psych();
int __cdecl base_rank(int faction_id, int rank_position);
int __cdecl pop_goal_fac(int base_id);
int __cdecl pop_goal(int base_id);
BOOL __cdecl base_queue(int base_id);
void __cdecl base_energy_costs();
int __cdecl fac_maint(int facility_id, int faction_id);
void __cdecl base_maint();
void __cdecl make_base_unique(int base_id);
// ?has_fac_built@@YA_NH@Z. `_N` is bool, not BOOL: the catalogue spells a
// genuine one-byte bool, and per tools/catalogue_corrections.py that width is
// byte-visible - a bool return clears with `xor al, al` (32 C0) where an int
// uses `xor eax, eax` (33 C0). The two-argument overload is not catalogued.
int __cdecl attack_from(int base_id_to_atk, int faction_id);
// The prototype-upgrade family: do_upgrade drives the queue, upgrade_cost
// prices one upgrade, upgrade_prototypes runs a faction's whole table and
// upgrade_any_prototypes gates it on the preferences bit.
void __cdecl do_upgrade(int base_id, int faction_id, int proto_id);
int __cdecl upgrade_cost(int faction_id, int proto_id_init, int proto_id_upg);
void __cdecl upgrade_prototypes(int faction_id, int proto_id);
// ?upgrade_prototype@@YAHHHHH@Z. The image's own only caller passes four
// arguments; see pending_bodies.cpp before adding a fifth parameter here.
int __cdecl upgrade_prototype(int base_id, int faction_id, int proto_id,
                              int set_flag);
void __cdecl upgrade_any_prototypes(int faction_id);
int __cdecl value_of_base(int base_id, uint32_t faction_id_req, uint32_t faction_id_res, 
                                    uint32_t overmatch_deg, BOOL tgl);
int __cdecl garrison_check(int base_id);
// ?get_rating@@YAHHH@Z - a faction's score weight from its difficulty level.
int __cdecl get_rating(int faction_id, int arg2);
int __cdecl defensive_check(int base_id);
BOOL __cdecl is_port(int base_id, BOOL is_base_radius);
int __cdecl vulnerable(int faction_id, int x, int y);
BOOL __cdecl is_objective(int base_id);
int __cdecl num_objectives(int faction_id, BOOL count_pact_factions);
BOOL __cdecl transcending(int faction_id);
BOOL __cdecl redundant(int facility_id, int faction_id);
// ?facility_avail@@YAHHHHH@Z - the definition already takes int.
BOOL __cdecl facility_avail(int facility_id, int faction_id, int base_id,
                                      int queue_count);
int __cdecl facility_offset(LPCSTR facil_search);
int __cdecl suggest_plan(int faction_id, int faction_id_2);

void __cdecl set_base(int base_id);

MEASURED inline int __cdecl base_project(int project_id) {
    return *(&SecretProject.human_genome_project + project_id);
}

MEASURED inline BOOL __cdecl has_project(int project_id, int faction_id) {
    int base_id = base_project(project_id);
    return (base_id < 0) ? false : (Bases[base_id].faction_id_current == faction_id);
}

inline bool __cdecl has_fac_built(int facility_id, int base_id) {
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    return (Bases[base_id].facilities_built[offset] & mask) != 0;
}

inline bool __cdecl has_fac_built(int facility_id) {
    int base_id = BaseIDCurrentSelected;
    return (facility_id >= FacilityRepStart) ? false : has_fac_built(facility_id, base_id);
}

// has_fac_built(), open-coded with a REAL bitmask() call via `bitmask_call`
// (general.h) rather than the folded shift/and `MEASURED inline bitmask`
// gives everywhere else - moved here (from base.cpp, where the marker note
// for it still lives) so breed_mod below can reach it too.
static __forceinline int has_fac_built_call(int facility_id, int base_id) {
    int offset;
    int mask;
    bitmask_call(facility_id, &offset, &mask);
    return (Bases[base_id].facilities_built[offset] & mask) != 0;
}

// BODY IN base.h, as `MEASURED inline` (marker stays in base.cpp): worm_mod
// (base.cpp) is the caller that needs it folded in place - the image writes
// breed_mod's whole body out there rather than calling 0x004E65C0.
// LEVER: breed_mod's OWN standalone body (0x004E65C0) calls bitmask() FOUR
// times, directly, with LITERAL facility ids (push 0x1f/0x20/9/0x1e) - the
// has_fac(FAC_X, base_id, 0) wrapper is never called there, where worm_mod's
// inlined copy of this same source DOES keep has_fac() as four real calls
// to 0x00421670 (its own marker note). Since has_fac(facility_id, base_id, 0)
// is exactly has_fac_built(facility_id, base_id) when facility_id is a
// compile-time constant below FacilityRepStart and queue_count is the
// literal 0, spelling the CENTAURI_PRESERVE/TEMPLE_OF_PLANET/BIOLOGY_LAB/
// BIOENHANCEMENT_CENTER checks with has_fac_built_call() (general.h
// bitmask_call, forcing the real E8) here fixes breed_mod's call count
// (0 -> 4 real bitmask calls, matching call_diff) and moves its best
// similarity to 0.749. TRADE-OFF, measured: since breed_mod is a SINGLE
// shared inline body, this same edit reaches worm_mod's inlined copy too -
// its best similarity drops 0.826 -> 0.706 (worm_mod is 0x004E6740, not in
// this pass's list and not a recorded match, so this is not a tracked
// regression, but a future pass on worm_mod needs to know the source of
// its drop is here, not there).
MEASURED inline int __cdecl breed_mod(int base_id, int faction_id) {
    uint32_t lifecycle_modifier = has_project(SP_XENOEMPATYH_DOME, faction_id) ? 1 : 0;
    if (has_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_project(SP_VOICE_OF_PLANET, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built_call(FAC_CENTAURI_PRESERVE, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built_call(FAC_TEMPLE_OF_PLANET, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built_call(FAC_BIOLOGY_LAB, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built_call(FAC_BIOENHANCEMENT_CENTER, base_id)
        || has_project(SP_CYBORG_FACTORY, faction_id)) {
        lifecycle_modifier++;
    }
    return lifecycle_modifier;
}

// BODY IN base.h, as `MEASURED inline` (marker stays in base.cpp, where it
// is already recorded BYTE_EXACT as a standalone function): facility_avail
// (base.cpp) is the caller that needs it folded in place instead of calling
// out to 0x005AC680.
void __cdecl compute_score(int faction, int *total, int *items, int recursive);
// Unfinished implementation in base.cpp; retained original seed owns behavior.
void __cdecl capture_base(int base_id, int faction_id, int mode);

MEASURED inline BOOL __cdecl ascending(int UNUSED(faction_id) faction_id) {
    return base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt;
}
