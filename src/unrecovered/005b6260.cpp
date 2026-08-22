// ORIGINAL: 0x005B6260 ?spot_all@@YAXHH@Z 0x005B6260-0x005B8AE2 FILE
// TRIED: mechanical register/stack-slot goto transliteration (234 labels, 5638-line body): compiles, MISMATCH sim up to 0.73 on /O2 frameless. Not restructured into idiomatic control flow - next pass should re-derive loop/if shape per block before chasing byte-exactness.
// working copy - scaffold materialised by --work
// size      10370 bytes
// prototype void (__cdecl ?spot_all@@YAXHH@Z)(uint32_t vehID, BOOL tgl)
// callers   6   call targets   21
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AEF0 0x004712A0 0x004E3A50 0x004E3B80 0x004E3EF0 0x0050BA00 0x005230E0 0x0055E370 0x00579920 0x00579A30 0x00591E50 0x005B5700 0x005B5EA0 0x005B6060 0x005B8E10 0x005B9580 0x005BF010 0x005BF1F0 0x005BFE90 0x005D5A70 0x00644F3A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?spot_all@@YAXHH@Z  at 0x005B6260  (10370 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// WHAT THE BODY NEEDS, not only what the signature reaches. The unit used to
// declare exactly the types the DECODED SIGNATURE mentioned, which is correct
// for the definition head and wrong for everything inside it. Measured over
// every NO_COMPILE row in the map on 2026-08-14: 1,544 implemented pieces do
// not compile, and 899 DISTINCT undeclared identifiers cause it - led by
// `NULL` at 50 bodies, which is one line.
//
// Every name below is already defined somewhere in src/*.h. The scaffold is a
// standalone unit and cannot include those headers - they pull in the whole
// project - so the cheap, layout-free half is restated here. Constants and
// typedefs only: no class, no global address, nothing that could disagree with
// a layout the emitter computes elsewhere.
#ifndef NULL
#define NULL 0
#endif

// Windows typedefs. The brief used to tell agents these were "a fact about the
// unit, not about the body - do not rewrite the body to chase it", which is a
// scaffold gap described accurately and then accepted. 432 bodies stop on
// C2061 for want of these ten lines.
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef void *HANDLE;
typedef void *HWND;
typedef void *HDC;
typedef unsigned int UINT;

// Spliced verbatim from src/original_seam.h so the unit calls into the
// original image exactly as the build does. See seam_header().
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

/*
 * Calling a method of the ORIGINAL image, which lives at a fixed address in
 * terranx.exe rather than anywhere this DLL can link against.
 *
 * These used to be spelled as free function pointers carrying the convention
 * by hand:
 *
 *     typedef void(__thiscall func_buffer_line)(Buffer *, int, int, int, int);
 *     func_buffer_line *BufferHLine = (func_buffer_line *)0x005E1A80;
 *     BufferHLine(this, a, b, c, d);
 *
 * cl 12.00.8168 - the compiler that built the original, and the only one
 * whose output can say whether a recovered body is right - reserves the
 * `__thiscall` keyword and refuses it (C4234). Disabling that warning is a
 * TRAP: it compiles, and the call it emits is
 *
 *     push d; push c; push b; push a; call ...; add esp, 0x10
 *
 * which is __cdecl. The receiver goes on the stack instead of into ECX and
 * the caller cleans a frame the callee already cleaned. Every seam into the
 * original image would corrupt the stack, silently, at runtime.
 *
 * A pointer-to-member IS thiscall, in every compiler, without naming the
 * convention at all. The same call becomes
 *
 *     typedef void (OriginalObject::*func_buffer_line)(int, int, int, int);
 *     func_buffer_line BufferHLine = original_method<func_buffer_line>(0x005E1A80);
 *     (ORIGINAL(this)->*BufferHLine)(a, b, c, d);
 *
 * and VC6 emits `mov ecx, this; push d..a; call` - the receiver in ECX and
 * the callee cleaning up, which is what the original expects. Measured
 * against the real compiler, not inferred.
 *
 * The object is `OriginalObject` rather than the real class because the
 * receiver is frequently only known as `void *`, and because the pointer
 * value is all that is ever needed - none of these methods is resolved
 * through this type.
 */

/*
 * `__single_inheritance` pins the pointer-to-member representation to a bare
 * code address. Without it the class is incomplete, MSVC assumes the most
 * general form - virtual bases and all - and every call site grows a
 * twenty-instruction adjustment sequence around it.
 */
class __single_inheritance OriginalObject;

/*
 * An address is not convertible to a pointer-to-member by any cast, so it
 * goes through a union. Implementation-defined in principle; pinned here by
 * the representation above and verified against the compiler.
 */
template <class Method>
Method original_method(unsigned long address) {
  union {
    unsigned long address;
    Method method;
  } cast;
  cast.address = address;
  return cast.method;
}

/*
 * The same union read the other way. `reinterpret_cast<unsigned long>` on a
 * pointer-to-member is `error C2440` on VC6 - it is not a pointer as far as
 * the language is concerned, whatever the representation - so recovering the
 * bare code address needs the same pinned punning that creating one does.
 */
template <class Method>
unsigned long original_address(Method method) {
  union {
    unsigned long address;
    Method method;
  } cast;
  cast.address = 0;
  cast.method = method;
  return cast.address;
}

/*
 * A vtable slot, read as a pinned pointer-to-member. The recovered code spelt
 * this `(*reinterpret_cast<Method *>(vtable + 0x14))(object)` in seventy-odd
 * places - reading the slot AS a pointer-to-member and then calling it as a
 * free function, which is `C2064: term does not evaluate to a function`. The
 * slot holds a bare code address; this reads it as one and hands it to
 * original_method, leaving the call site an honest `->*`.
 */
template <class Method>
Method original_slot(const void *slot) {
  return original_method<Method>(*reinterpret_cast<const unsigned long *>(slot));
}

#define ORIGINAL(pointer) (reinterpret_cast<OriginalObject *>(pointer))

// Integer constants restated from src/*.h, which this standalone unit cannot include.
const int ABL_AAA = 0x100;
const int ABL_AIR_SUPERIORITY = 0x20;
const int ABL_ALGO_ENHANCEMENT = 0x10000000;
const int ABL_AMPHIBIOUS = 0x8;
const int ABL_ANTIGRAV_STRUTS = 0x400;
const int ABL_ARTILLERY = 0x8000;
const int ABL_BLINK_DISPLACER = 0x20000;
const int ABL_CARRIER = 0x80;
const int ABL_CLEAN_REACTOR = 0x10000;
const int ABL_CLOAKED = 0x4;
const int ABL_COMM_JAMMER = 0x200;
const int ABL_DEEP_PRESSURE_HULL = 0x40;
const int ABL_DEEP_RADAR = 0x2;
const int ABL_DISSOCIATIVE_WAVE = 0x2000000;
const int ABL_DROP_POD = 0x10;
const int ABL_EMPATHIC = 0x800;
const int ABL_FUEL_NANOCELLS = 0x8000000;
const int ABL_FUNGICIDAL = 0x2000;
const int ABL_HEAVY_TRANSPORT = 0x80000;
const int ABL_MARINE_DETACHMENT = 0x4000000;
const int ABL_NERVE_GAS = 0x100000;
const int ABL_POLICE_2X = 0x400000;
const int ABL_POLY_ENCRYPTION = 0x1000;
const int ABL_REPAIR = 0x200000;
const int ABL_SLOW = 0x800000;
const int ABL_SOPORIFIC_GAS = 0x1000000;
const int ABL_SUPER_TERRAFORMER = 0x1;
const int ABL_TRAINED = 0x4000;
const int ABL_TRANCE = 0x40000;
const int AFLAG_ALLOWED_AIR_UNIT = 0x4;
const int AFLAG_ALLOWED_COMBAT_UNIT = 0x8;
const int AFLAG_ALLOWED_LAND_UNIT = 0x1;
const int AFLAG_ALLOWED_NONCOMBAT_UNIT = 0x20;
const int AFLAG_ALLOWED_SEA_UNIT = 0x2;
const int AFLAG_ALLOWED_TERRAFORM_UNIT = 0x10;
const int AFLAG_COST_INC_LAND_UNIT = 0x400;
const int AFLAG_NOT_ALLOWED_FAST_UNIT = 0x200;
const int AFLAG_NOT_ALLOWED_PROBE_TEAM = 0x40;
const int AFLAG_NOT_ALLOWED_PSI_UNIT = 0x80;
const int AFLAG_ONLY_PROBE_TEAM = 0x800;
const int AFLAG_TRANSPORT_ONLY_UNIT = 0x100;
const int AI_GOAL_ATTACK = 0;
const int AI_GOAL_COLONIZE = 8;
const int AI_GOAL_CONDENSER = 73;
const int AI_GOAL_DEFEND = 2;
const int AI_GOAL_ECHELON_MIRROR = 13;
const int AI_GOAL_LANDING_SITE = 41;
const int AI_GOAL_PRIORITY_COMBAT = 16;
const int AI_GOAL_SCOUT = 3;
const int AI_GOAL_SENSOR_ARRAY = 121;
const int AI_GOAL_TERRAFORM_LAND = 9;
const int AI_GOAL_TERRAFORM_WATER = 25;
const int AI_GOAL_THERMAL_BOREHOLE = 105;
const int AI_GOAL_UNK_1 = 6;
const int AI_GOAL_UNK_2 = 7;
const int AI_GOAL_UNK_3 = 11;
const int AI_GOAL_UNUSED = -1;
const int ALT_1_LEVEL_ABOVE_SEA = 4;
const int ALT_2_LEVELS_ABOVE_SEA = 5;
const int ALT_3_LEVELS_ABOVE_SEA = 6;
const int ALT_BIT_1_LEVEL_ABOVE_SEA = 0x80;
const int ALT_BIT_2_LEVELS_ABOVE_SEA = 0xA0;
const int ALT_BIT_3_LEVELS_ABOVE_SEA = 0xC0;
const int ALT_BIT_OCEAN = 0x20;
const int ALT_BIT_OCEAN_SHELF = 0x40;
const int ALT_BIT_OCEAN_TRENCH = 0x0;
const int ALT_BIT_SHORE_LINE = 0x60;
const int ALT_OCEAN = 1;
const int ALT_OCEAN_SHELF = 2;
const int ALT_OCEAN_TRENCH = 0;
const int ALT_SHORE_LINE = 3;
const int ARM_ANTIMATTER_PLATE = 7;
const int ARM_NEUTRONIUM_ARMOR = 6;
const int ARM_NO_ARMOR = 0;
const int ARM_PHOTON_WALL = 4;
const int ARM_PLASMA_STEEL_ARMOR = 2;
const int ARM_PROBABILITY_SHEATH = 5;
const int ARM_PSI_DEFENSE = 9;
const int ARM_PULSE_3_ARMOR = 10;
const int ARM_PULSE_8_ARMOR = 12;
const int ARM_RESONANCE_3_ARMOR = 11;
const int ARM_RESONANCE_8_ARMOR = 13;
const int ARM_SILKSTEEL_ARMOR = 3;
const int ARM_STASIS_GENERATOR = 8;
const int ARM_SYNTHMETAL_ARMOR = 1;
const int BEVENT_BUMPER = 0x200;
const int BEVENT_BUST = 0x1000;
const int BEVENT_CLOUD_COVER = 0x4000;
const int BEVENT_FAMINE = 0x400;
const int BEVENT_HEAT_WAVE = 0x2000;
const int BEVENT_INDUSTRY = 0x800;
const int BEVENT_OBJECTIVE = 0x8000;
const int BEVENT_UNK_100 = 0x100;
const int BIT2_BOREHOLE = 0x1000;
const int BIT2_CANYON = 0x200;
const int BIT2_CRATER = 0x1;
const int BIT2_DUNES = 0x40;
const int BIT2_FOSSIL = 0x8000;
const int BIT2_FRESH = 0x80;
const int BIT2_GEOTHERMAL = 0x400;
const int BIT2_JUNGLE = 0x4;
const int BIT2_MESA = 0x100;
const int BIT2_NEXUS = 0x2000;
const int BIT2_RIDGE = 0x800;
const int BIT2_RUINS = 0x20;
const int BIT2_SARGASSO = 0x10;
const int BIT2_UNITY = 0x4000;
const int BIT2_UNK_80000000 = 0x80000000;
const int BIT2_URANIUM = 0x8;
const int BIT2_VOLCANO = 0x2;
const int BIT_AIRBASE = 0x40000;
const int BIT_BASE_IN_TILE = 0x1;
const int BIT_BASE_RADIUS = 0x1000;
const int BIT_BUNKER = 0x800;
const int BIT_CONDENSER = 0x400000;
const int BIT_ECH_MIRROR = 0x800000;
const int BIT_ENERGY_RSC = 0x10000;
const int BIT_FARM = 0x8000;
const int BIT_FOREST = 0x200000;
const int BIT_FUNGUS = 0x20;
const int BIT_MAGTUBE = 0x8;
const int BIT_MINE = 0x10;
const int BIT_MINERAL_RSC = 0x20000;
const int BIT_MONOLITH = 0x2000;
const int BIT_NUTRIENT_RSC = 0x20000000;
const int BIT_RIVER = 0x80;
const int BIT_RIVERBED = 0x100;
const int BIT_RIVER_LAKE = 0x200;
const int BIT_ROAD = 0x4;
const int BIT_RSC_BONUS = 0x400;
const int BIT_SENSOR_ARRAY = 0x80000000;
const int BIT_SOIL_ENRICHER = 0x80000;
const int BIT_SOLAR_TIDAL = 0x40;
const int BIT_SUPPLY_POD = 0x10000000;
const int BIT_SUPPLY_REMOVE = 0x100000;
const int BIT_THERMAL_BORE = 0x1000000;
const int BIT_UNK_2000000 = 0x2000000;
const int BIT_UNK_4000 = 0x4000;
const int BIT_UNK_4000000 = 0x4000000;
const int BIT_UNK_40000000 = 0x40000000;
const int BIT_UNK_8000000 = 0x8000000;
const int BIT_VEH_IN_TILE = 0x2;
const int BSC_ALIEN_ARTIFACT = 7;
const int BSC_BATTLE_OGRE_MK1 = 16;
const int BSC_BATTLE_OGRE_MK2 = 17;
const int BSC_BATTLE_OGRE_MK3 = 18;
const int BSC_COLONY_POD = 0;
const int BSC_FORMERS = 1;
const int BSC_FUNGAL_TOWER = 19;
const int BSC_ISLE_OF_THE_DEEP = 9;
const int BSC_LOCUSTS_OF_CHIRON = 10;
const int BSC_MIND_WORMS = 8;
const int BSC_PROBE_TEAM = 6;
const int BSC_SCOUT_PATROL = 2;
const int BSC_SEALURK = 14;
const int BSC_SEA_ESCAPE_POD = 21;
const int BSC_SEA_FORMERS = 4;
const int BSC_SPORE_LAUNCHER = 15;
const int BSC_SUPPLY_CRAWLER = 5;
const int BSC_TRANSPORT_FOIL = 3;
const int BSC_UNITY_FOIL = 13;
const int BSC_UNITY_GUNSHIP = 22;
const int BSC_UNITY_MINING_LASER = 20;
const int BSC_UNITY_ROVER = 11;
const int BSC_UNITY_SCOUT_CHOPPER = 12;
const int BSTATE_ARTIFACT_ALREADY_LINKED = 0x800;
const int BSTATE_ARTIFACT_LINKED = 0x400;
const int BSTATE_ASSISTANT_KILLER_HOME = 0x40000;
const int BSTATE_COMBAT_LOSS_LAST_TURN = 0x8;
const int BSTATE_DRONE_RIOTS_ACTIVE = 0x2;
const int BSTATE_ENERGY_RESERVES_DRAINED = 0x400000;
const int BSTATE_FACILITY_SCRAPPED = 0x200;
const int BSTATE_GENETIC_PLAGUE_INTRO = 0x20000;
const int BSTATE_GOLDEN_AGE_ACTIVE = 0x4;
const int BSTATE_NET_LOCKED = 0x10000000;
const int BSTATE_PRODUCTION_DONE = 0x800000;
const int BSTATE_PRODUCTION_HURRIED = 0x40000000;
const int BSTATE_PSI_GATE_USED = 0x20000000;
const int BSTATE_RESEARCH_DATA_STOLEN = 0x40;
const int BSTATE_UNK_1 = 0x1;
const int BSTATE_UNK_10 = 0x10;
const int BSTATE_UNK_100 = 0x100;
const int BSTATE_UNK_1000 = 0x1000;
const int BSTATE_UNK_10000 = 0x10000;
const int BSTATE_UNK_100000 = 0x100000;
const int BSTATE_UNK_1000000 = 0x1000000;
const int BSTATE_UNK_20 = 0x20;
const int BSTATE_UNK_2000 = 0x2000;
const int BSTATE_UNK_200000 = 0x200000;
const int BSTATE_UNK_2000000 = 0x2000000;
const int BSTATE_UNK_4000 = 0x4000;
const int BSTATE_UNK_4000000 = 0x4000000;
const int BSTATE_UNK_80 = 0x80;
const int BSTATE_UNK_8000 = 0x8000;
const int BSTATE_UNK_80000 = 0x80000;
const int BSTATE_UNK_8000000 = 0x8000000;
const int BSTATE_UNK_8000000000 = 0x80000000;
const int BufferSurfaceLockSlot = 0x64;
const int BufferSurfaceUnlockSlot = 0x80;
const int CHSI_COPTER = 6;
const int CHSI_CRUISER = 4;
const int CHSI_FOIL = 3;
const int CHSI_GRAVSHIP = 7;
const int CHSI_HOVERTANK = 2;
const int CHSI_INFANTRY = 0;
const int CHSI_MISSILE = 8;
const int CHSI_NEEDLEJET = 5;
const int CHSI_SPEEDER = 1;
const int CITIZEN_DOCTOR = 1;
const int CITIZEN_EMPATH = 4;
const int CITIZEN_ENGINEER = 3;
const int CITIZEN_LIBRARIAN = 2;
const int CITIZEN_TECHNICIAN = 0;
const int CITIZEN_THINKER = 5;
const int CITIZEN_TRANSCEND = 6;
const int DAGENDA_DIPLO_PERMANENT = 0x2000;
const int DAGENDA_FIGHT_TO_DEATH = 0x8;
const int DAGENDA_UNK_1 = 0x1;
const int DAGENDA_UNK_10 = 0x10;
const int DAGENDA_UNK_100 = 0x100;
const int DAGENDA_UNK_1000 = 0x1000;
const int DAGENDA_UNK_2 = 0x2;
const int DAGENDA_UNK_20 = 0x20;
const int DAGENDA_UNK_200 = 0x200;
const int DAGENDA_UNK_4 = 0x4;
const int DAGENDA_UNK_40 = 0x40;
const int DAGENDA_UNK_400 = 0x400;
const int DAGENDA_UNK_4000 = 0x4000;
const int DAGENDA_UNK_80 = 0x80;
const int DAGENDA_UNK_800 = 0x800;
const int DAGENDA_UNK_8000 = 0x8000;
const int DLVL_CITIZEN = 0;
const int DLVL_LIBRARIAN = 3;
const int DLVL_SPECIALIST = 1;
const int DLVL_TALENT = 2;
const int DLVL_THINKER = 4;
const int DLVL_TRANSCEND = 5;
const int DTREATY_ATROCITY_VICTIM = 0x40000;
const int DTREATY_COMMLINK = 0x8;
const int DTREATY_HAVE_INFILTRATOR = 0x1000;
const int DTREATY_HAVE_SURRENDERED = 0x2000000;
const int DTREATY_PACT = 0x1;
const int DTREATY_SHALL_BETRAY = 0x400;
const int DTREATY_TREATY = 0x2;
const int DTREATY_TRUCE = 0x4;
const int DTREATY_UNK_100 = 0x100;
const int DTREATY_UNK_10000 = 0x10000;
const int DTREATY_UNK_100000 = 0x100000;
const int DTREATY_UNK_1000000 = 0x1000000;
const int DTREATY_UNK_10000000 = 0x10000000;
const int DTREATY_UNK_200 = 0x200;
const int DTREATY_UNK_20000 = 0x20000;
const int DTREATY_UNK_200000 = 0x200000;
const int DTREATY_UNK_20000000 = 0x20000000;
const int DTREATY_UNK_40 = 0x40;
const int DTREATY_UNK_4000 = 0x4000;
const int DTREATY_UNK_400000 = 0x400000;
const int DTREATY_UNK_4000000 = 0x4000000;
const int DTREATY_UNK_40000000 = 0x40000000;
const int DTREATY_UNK_80 = 0x80;
const int DTREATY_UNK_800 = 0x800;
const int DTREATY_UNK_8000 = 0x8000;
const int DTREATY_UNK_80000 = 0x80000;
const int DTREATY_UNK_800000 = 0x800000;
const int DTREATY_UNK_8000000 = 0x8000000;
const int DTREATY_UNK_80000000 = 0x80000000;
const int DTREATY_VENDETTA = 0x10;
const int DTREATY_WANT_REVENGE = 0x20;
const int DTREATY_WANT_TO_TALK = 0x2000;
const int DialogsDestructorAdjustment = 0x188;
const int DisabledValue = -2;
const int FAC_AEROSPACE_COMPLEX = 29;
const int FAC_AQUAFARM = 36;
const int FAC_ASCENT_TO_TRANSCENDENCE = 102;
const int FAC_ASCETIC_VIRTUES = 83;
const int FAC_BIOENHANCEMENT_CENTER = 30;
const int FAC_BIOLOGY_LAB = 9;
const int FAC_BROOD_PIT = 35;
const int FAC_BULK_MATTER_TRANSMITTER = 99;
const int FAC_CENTAURI_PRESERVE = 31;
const int FAC_CHILDREN_CRECHE = 2;
const int FAC_CITIZENS_DEFENSE_FORCE = 75;
const int FAC_CLINICAL_IMMORTALITY = 96;
const int FAC_CLONING_VATS = 94;
const int FAC_CLOUDBASE_ACADEMY = 105;
const int FAC_COMMAND_CENTER = 27;
const int FAC_COMMAND_NEXUS = 71;
const int FAC_COVERT_OPS_CENTER = 34;
const int FAC_CYBORG_FACTORY = 87;
const int FAC_DREAM_TWISTER = 89;
const int FAC_EMPATH_GUILD = 74;
const int FAC_EMPTY_FACILITY_42 = 42;
const int FAC_EMPTY_FACILITY_43 = 43;
const int FAC_EMPTY_FACILITY_44 = 44;
const int FAC_EMPTY_FACILITY_45 = 45;
const int FAC_EMPTY_FACILITY_46 = 46;
const int FAC_EMPTY_FACILITY_47 = 47;
const int FAC_EMPTY_FACILITY_48 = 48;
const int FAC_EMPTY_FACILITY_49 = 49;
const int FAC_EMPTY_FACILITY_50 = 50;
const int FAC_EMPTY_FACILITY_51 = 51;
const int FAC_EMPTY_FACILITY_52 = 52;
const int FAC_EMPTY_FACILITY_53 = 53;
const int FAC_EMPTY_FACILITY_54 = 54;
const int FAC_EMPTY_FACILITY_55 = 55;
const int FAC_EMPTY_FACILITY_56 = 56;
const int FAC_EMPTY_FACILITY_57 = 57;
const int FAC_EMPTY_FACILITY_58 = 58;
const int FAC_EMPTY_FACILITY_59 = 59;
const int FAC_EMPTY_FACILITY_60 = 60;
const int FAC_EMPTY_FACILITY_61 = 61;
const int FAC_EMPTY_FACILITY_62 = 62;
const int FAC_EMPTY_FACILITY_63 = 63;
const int FAC_EMPTY_FACILITY_64 = 64;
const int FAC_EMPTY_SP_38 = 107;
const int FAC_EMPTY_SP_39 = 108;
const int FAC_EMPTY_SP_40 = 109;
const int FAC_EMPTY_SP_41 = 110;
const int FAC_EMPTY_SP_42 = 111;
const int FAC_EMPTY_SP_43 = 112;
const int FAC_EMPTY_SP_44 = 113;
const int FAC_EMPTY_SP_45 = 114;
const int FAC_EMPTY_SP_46 = 115;
const int FAC_EMPTY_SP_47 = 116;
const int FAC_EMPTY_SP_48 = 117;
const int FAC_EMPTY_SP_49 = 118;
const int FAC_EMPTY_SP_50 = 119;
const int FAC_EMPTY_SP_51 = 120;
const int FAC_EMPTY_SP_52 = 121;
const int FAC_EMPTY_SP_53 = 122;
const int FAC_EMPTY_SP_54 = 123;
const int FAC_EMPTY_SP_55 = 124;
const int FAC_EMPTY_SP_56 = 125;
const int FAC_EMPTY_SP_57 = 126;
const int FAC_EMPTY_SP_58 = 127;
const int FAC_EMPTY_SP_59 = 128;
const int FAC_EMPTY_SP_60 = 129;
const int FAC_EMPTY_SP_61 = 130;
const int FAC_EMPTY_SP_62 = 131;
const int FAC_EMPTY_SP_63 = 132;
const int FAC_EMPTY_SP_64 = 133;
const int FAC_ENERGY_BANK = 7;
const int FAC_FLECHETTE_DEFENSE_SYS = 39;
const int FAC_FUSION_LAB = 15;
const int FAC_GENEJACK_FACTORY = 22;
const int FAC_GEOSYNC_SURVEY_POD = 41;
const int FAC_HABITATION_DOME = 25;
const int FAC_HAB_COMPLEX = 24;
const int FAC_HEADQUARTERS = 1;
const int FAC_HOLOGRAM_THEATRE = 11;
const int FAC_HUMAN_GENOME_PROJ = 70;
const int FAC_HUNTER_SEEKER_ALGO = 85;
const int FAC_HYBRID_FOREST = 14;
const int FAC_LIVING_REFINERY = 93;
const int FAC_LONGEVITY_VACCINE = 84;
const int FAC_MANIFOLD_HARMONICS = 103;
const int FAC_MARITIME_CONTROL_CENTER = 80;
const int FAC_MERCHANT_EXCHANGE = 73;
const int FAC_NANOHOSPITAL = 18;
const int FAC_NANOREPLICATOR = 20;
const int FAC_NANO_FACTORY = 92;
const int FAC_NAVAL_YARD = 28;
const int FAC_NESSUS_MINING_STATION = 66;
const int FAC_NETHACK_TERMINUS = 104;
const int FAC_NETWORK_BACKBONE = 91;
const int FAC_NETWORK_NODE = 8;
const int FAC_NEURAL_AMPLIFIER = 79;
const int FAC_ORBITAL_DEFENSE_POD = 68;
const int FAC_ORBITAL_POWER_TRANS = 67;
const int FAC_PARADISE_GARDEN = 12;
const int FAC_PERIMETER_DEFENSE = 4;
const int FAC_PHOLUS_MUTAGEN = 86;
const int FAC_PLANETARY_DATALINKS = 81;
const int FAC_PLANETARY_ENERGY_GRID = 106;
const int FAC_PLANETARY_TRANS_SYS = 77;
const int FAC_PRESSURE_DOME = 26;
const int FAC_PSI_GATE = 33;
const int FAC_PUNISHMENT_SPHERE = 23;
const int FAC_QUANTUM_CONVERTER = 21;
const int FAC_QUANTUM_LAB = 16;
const int FAC_RECREATION_COMMONS = 6;
const int FAC_RECYCLING_TANKS = 3;
const int FAC_RESEARCH_HOSPITAL = 17;
const int FAC_ROBOTIC_ASSEMBLY_PLANT = 19;
const int FAC_SELF_AWARE_COLONY = 95;
const int FAC_SINGULARITY_INDUCTOR = 98;
const int FAC_SKUNKWORKS = 10;
const int FAC_SKY_HYDRO_LAB = 65;
const int FAC_SPACE_ELEVATOR = 97;
const int FAC_STOCKPILE_ENERGY = 69;
const int FAC_SUBSEA_TRUNKLINE = 37;
const int FAC_SUBSPACE_GENERATOR = 40;
const int FAC_SUPERCOLLIDER = 82;
const int FAC_TACHYON_FIELD = 5;
const int FAC_TELEPATHIC_MATRIX = 100;
const int FAC_TEMPLE_OF_PLANET = 32;
const int FAC_THEORY_OF_EVERYTHING = 88;
const int FAC_THERMOCLINE_TRANSDUCER = 38;
const int FAC_TREE_FARM = 13;
const int FAC_UNIVERSAL_TRANSLATOR = 90;
const int FAC_VIRTUAL_WORLD = 76;
const int FAC_VOICE_OF_PLANET = 101;
const int FAC_WEATHER_PARADIGM = 72;
const int FAC_XENOEMPATHY_DOME = 78;
const int FacilityRepStart = 65;
const int FacilitySPStart = 70;
const int FontSizeTableCount = 12;
const int GENDER_FEMALE = 1;
const int GENDER_MALE = 0;
const int GENDER_NEUTRAL = 2;
const int GOV_ACTIVE = 0x80000000;
const int GOV_MANAGES_CITIZENS_SPECS = 0x40;
const int GOV_MANAGES_PRODUCTION = 0x1;
const int GOV_MAY_HURRY_PRODUCTION = 0x20;
const int GOV_MAY_PROD_AIR_COMBAT = 0x800;
const int GOV_MAY_PROD_AIR_DEFENS = 0x2000;
const int GOV_MAY_PROD_COLONY_POD = 0x20000;
const int GOV_MAY_PROD_EXPLR_VEH = 0x400000;
const int GOV_MAY_PROD_FACILITIES = 0x10000;
const int GOV_MAY_PROD_LAND_COMBAT = 0x200;
const int GOV_MAY_PROD_LAND_DEFENS = 0x1000;
const int GOV_MAY_PROD_NAVAL_COMBAT = 0x400;
const int GOV_MAY_PROD_PROBES = 0x100000;
const int GOV_MAY_PROD_PROTOTYPE = 0x80000;
const int GOV_MAY_PROD_SP = 0x40000;
const int GOV_MAY_PROD_TERRAFORMS = 0x8000;
const int GOV_MAY_PROD_TRANSPORT = 0x800000;
const int GOV_MULTI_PRIORITIES = 0x200000;
const int GOV_NEW_VEH_FULLY_AUTO = 0x80;
const int GOV_PRIORITY_BUILD = 0x4000000;
const int GOV_PRIORITY_CONQUER = 0x8000000;
const int GOV_PRIORITY_DISCOVER = 0x2000000;
const int GOV_PRIORITY_EXPLORE = 0x1000000;
const int GOV_UNK_40000000 = 0x40000000;
const int LM_BOREHOLE = 12;
const int LM_CANYON = 9;
const int LM_CRATER = 0;
const int LM_DUNES = 6;
const int LM_FOSSIL = 15;
const int LM_FRESH = 7;
const int LM_GEOTHERMAL = 10;
const int LM_JUNGLE = 2;
const int LM_MESA = 8;
const int LM_NEXUS = 13;
const int LM_RIDGE = 11;
const int LM_RUINS = 5;
const int LM_SARGASSO = 4;
const int LM_UNITY = 14;
const int LM_URANIUM = 3;
const int LM_VOLCANO = 1;
const int ListBoxDestructorAdjustment = 0x48;
const int MOOD_AMBIVALENT = 4;
const int MOOD_BELLIGERENT = 7;
const int MOOD_COOPERATIVE = 2;
const int MOOD_MAGNANIMOUS = 0;
const int MOOD_NONCOMMITTAL = 3;
const int MOOD_OBSTINATE = 5;
const int MOOD_QUARRELSOME = 6;
const int MOOD_SEETHING = 8;
const int MOOD_SOLICITOUS = 1;
const int MORALE_COMMANDO = 5;
const int MORALE_DISCIPLINED = 2;
const int MORALE_ELITE = 6;
const int MORALE_GREEN = 1;
const int MORALE_HARDENED = 3;
const int MORALE_VERY_GREEN = 0;
const int MORALE_VETERAN = 4;
const int MPREF_ADV_CLICK_VEH_CANCELS_ORDERS = 0x40000;
const int MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING = 0x8000;
const int MPREF_ADV_DETAIL_MAIN_MENUS = 0x200000;
const int MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS = 0x400;
const int MPREF_ADV_PAUSE_AFTER_BATTLES = 0x10;
const int MPREF_ADV_QUICK_MOVE_ALL_VEH = 0x80;
const int MPREF_ADV_QUICK_MOVE_VEH_ORDERS = 0x40;
const int MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU = 0x100;
const int MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP = 0x4;
const int MPREF_AUTO_ALWAYS_INSPECT_MONOLITH = 0x800;
const int MPREF_AUTO_FORMER_BUILD_SENSORS = 0x20;
const int MPREF_AUTO_FORMER_CANT_BUILD_ROADS = 0x400000;
const int MPREF_AUTO_FORMER_REMOVE_FUNGUS = 0x8;
const int MPREF_AV_MONUMENTS_DISABLED = 0x1000000;
const int MPREF_AV_SLIDING_SCROLLBARS = 0x80000;
const int MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP = 0x20000;
const int MPREF_AV_VOICEOVER_TECH_FAC = 0x4000;
const int MPREF_AV_VOLUME_VOICE_TOGGLE = 0x4000;
const int MPREF_AV_WHOLE_VEH_BLINKS = 0x200;
const int MPREF_BSC_AUTO_PRUNE_OBS_VEH = 0x100000;
const int MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH = 0x2000000;
const int MPREF_MAP_SHOW_BASE_NAMES = 0x2000;
const int MPREF_MAP_SHOW_FLAT_TERRAIN = 0x10000;
const int MPREF_MAP_SHOW_FOG_WAR = 0x1;
const int MPREF_MAP_SHOW_GRID_OCEAN_SQ = 0x800000;
const int MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES = 0x1000;
const int MapWinActiveOffset = 0x1DD74;
const int MapWinTableSlots = 8;
const int MaxAbilityNum = 29;
const int MaxArmorNum = 14;
const int MaxBaseNum = 512;
const int MaxBonusNameNum = 41;
const int MaxChassisNum = 9;
const int MaxCitizenNum = 10;
const int MaxCompassNum = 8;
const int MaxContinentNum = 128;
const int MaxDefenseModeNum = 3;
const int MaxDiffNum = 6;
const int MaxEnergyNum = 3;
const int MaxFacilityNum = 134;
const int MaxGoalsNum = 75;
const int MaxLandmarkNum = 64;
const int MaxMandateNum = 4;
const int MaxMightNum = 7;
const int MaxMoodNum = 9;
const int MaxMoraleNum = 7;
const int MaxNaturalNum = 16;
const int MaxOffenseModeNum = 3;
const int MaxOrderNum = 30;
const int MaxPlanNum = 15;
const int MaxPlayerNum = 8;
const int MaxProposalNum = 11;
const int MaxRankingHistoryTurns = 1000;
const int MaxReactorNum = 4;
const int MaxRegionLandNum = 64;
const int MaxReputeNum = 8;
const int MaxResourceInfoNum = 9;
const int MaxResourceNum = 4;
const int MaxSecretProjectNum = 64;
const int MaxSitesNum = 25;
const int MaxSocialCatNum = 4;
const int MaxSocialEffectNum = 11;
const int MaxSocialModelNum = 4;
const int MaxSpecialistNum = 7;
const int MaxTechnologyNum = 89;
const int MaxTerrainNum = 20;
const int MaxTextIndexNum = 4;
const int MaxTimeControlNum = 6;
const int MaxTriadNum = 3;
const int MaxVehProtoFactionNum = 64;
const int MaxVehProtoNum = 512;
const int MaxWeaponNum = 26;
const int NoneValue = -1;
const int ORDERA_AUTOMATE_AIR_DEFENSE = 12;
const int ORDERA_BOMBING_RUN = 10;
const int ORDERA_ON_ALERT = 11;
const int ORDERA_TERRA_AUTOIMPROVE_BASE = 3;
const int ORDERA_TERRA_AUTOMATIC_SENSOR = 7;
const int ORDERA_TERRA_AUTO_FULL = 0;
const int ORDERA_TERRA_AUTO_FUNGUS_REM = 6;
const int ORDERA_TERRA_AUTO_MAGTUBE = 2;
const int ORDERA_TERRA_AUTO_ROAD = 1;
const int ORDERA_TERRA_FARM_MINE_ROAD = 5;
const int ORDERA_TERRA_FARM_SOLAR_ROAD = 4;
const int ORDER_AIRBASE = 12;
const int ORDER_AI_MOVE_TO = 88;
const int ORDER_BUNKER = 11;
const int ORDER_CONDENSER = 16;
const int ORDER_CONVOY = 3;
const int ORDER_DRILL_AQUIFIER = 19;
const int ORDER_ECHELON_MIRROR = 17;
const int ORDER_EXPLORE = 26;
const int ORDER_FARM = 4;
const int ORDER_HOLD = 2;
const int ORDER_MAGTUBE = 10;
const int ORDER_MAGTUBE_TO = 28;
const int ORDER_MINE = 6;
const int ORDER_MOVE = 25;
const int ORDER_MOVE_TO = 24;
const int ORDER_NONE = 0;
const int ORDER_PLACE_MONOLITH = 23;
const int ORDER_PLANT_FOREST = 8;
const int ORDER_PLANT_FUNGUS = 15;
const int ORDER_REMOVE_FUNGUS = 14;
const int ORDER_ROAD = 9;
const int ORDER_ROAD_TO = 27;
const int ORDER_SENSOR_ARRAY = 13;
const int ORDER_SENTRY_BOARD = 1;
const int ORDER_SOIL_ENRICHER = 5;
const int ORDER_SOLAR_COLLECTOR = 7;
const int ORDER_TERRAFORM_DOWN = 21;
const int ORDER_TERRAFORM_LEVEL = 22;
const int ORDER_TERRAFORM_UP = 20;
const int ORDER_THERMAL_BOREHOLE = 18;
const int PFLAGEXT_SHAMELESS_BETRAY_HUMANS = 0x20;
const int PFLAGEXT_STRAT_LOTS_ARTILLERY = 0x40;
const int PFLAGEXT_STRAT_LOTS_COLONY_PODS = 0x1;
const int PFLAGEXT_STRAT_LOTS_MISSILES = 0x10;
const int PFLAGEXT_STRAT_LOTS_PROBE_TEAMS = 0x8;
const int PFLAGEXT_STRAT_LOTS_SEA_BASES = 0x4;
const int PFLAGEXT_STRAT_LOTS_TERRAFORMERS = 0x2;
const int PFLAG_BEEN_ELECTED_GOVERNOR = 0x8000;
const int PFLAG_COMMIT_ATROCIT_WANTONLY = 0x1000000;
const int PFLAG_COOP_WITH_HUMAN = 0x400000;
const int PFLAG_EMPHASIZE_AIR_POWER = 0x10000000;
const int PFLAG_EMPHASIZE_LAND_POWER = 0x4000000;
const int PFLAG_EMPHASIZE_SEA_POWER = 0x8000000;
const int PFLAG_GENETIC_PLAGUE_INTRO = 0x400;
const int PFLAG_MAP_REVEALED = 0x200;
const int PFLAG_OBLIT_CAPTURED_BASES = 0x2000000;
const int PFLAG_SELF_AWARE_COLONY_LOST_MAINT = 0x20;
const int PFLAG_STRAT_ATK_ENEMY_HQ = 0x200000;
const int PFLAG_STRAT_ATK_OBJECTIVES = 0x80000000;
const int PFLAG_STRAT_DEF_OBJECTIVES = 0x40000000;
const int PFLAG_STRAT_SEARCH_OBJECTIVES = 0x20000000;
const int PFLAG_TEAM_UP_VS_HUMAN = 0x800000;
const int PFLAG_UNK_10000 = 0x10000;
const int PFLAG_UNK_20000 = 0x20000;
const int PLAN_AIR_SUPERIORITY = 4;
const int PLAN_ALIEN_ARTIFACT = 12;
const int PLAN_COLONIZATION = 8;
const int PLAN_COMBAT = 1;
const int PLAN_DEFENSIVE = 2;
const int PLAN_FUNGAL_MISSILE = 14;
const int PLAN_INFO_WARFARE = 11;
const int PLAN_NAVAL_SUPERIORITY = 6;
const int PLAN_NAVAL_TRANSPORT = 7;
const int PLAN_OFFENSIVE = 0;
const int PLAN_PLANET_BUSTER = 5;
const int PLAN_RECONNAISANCE = 3;
const int PLAN_SUPPLY_CONVOY = 10;
const int PLAN_TECTONIC_MISSILE = 13;
const int PLAN_TERRAFORMING = 9;
const int PRB_ACTIVATE_SABOTAGE_VIRUS = 2;
const int PRB_ASSASSINATE_PROMINENT_RESEARCHERS = 5;
const int PRB_DRAIN_ENERGY_RESERVES = 3;
const int PRB_FREE_CAPTURED_FACTION_LEADER = 8;
const int PRB_INCITE_DRONE_RIOTS = 4;
const int PRB_INFILTRATE_DATALINKS = 0;
const int PRB_INTRODUCE_GENETIC_PLAGUE = 7;
const int PRB_MIND_CONTROL_CITY = 6;
const int PRB_MIND_CONTROL_VEH = -1;
const int PRB_PROCURE_RESEARCH_DATA = 1;
const int PREF_ADV_FAST_BATTLE_RESOLUTION = 0x8;
const int PREF_ADV_NO_CENTER_VEH_ORDERS = 0x80000;
const int PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK = 0x20000000;
const int PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG = 0x10000;
const int PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD = 0x40000000;
const int PREF_AUTO_END_MOVE_SPOT_VEH_PACT = 0x100000;
const int PREF_AUTO_END_MOVE_SPOT_VEH_TREATY = 0x200000;
const int PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE = 0x400000;
const int PREF_AUTO_END_MOVE_SPOT_VEH_WAR = 0x800000;
const int PREF_AUTO_FORMER_BUILD_ADV = 0x2000000;
const int PREF_AUTO_FORMER_PLANT_FORESTS = 0x1000000;
const int PREF_AUTO_FORMER_RAISE_LWR_TERRAIN = 0x20000;
const int PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND = 0x80000000;
const int PREF_AV_BACKGROUND_MUSIC = 0x800;
const int PREF_AV_INTERLUDES_DISABLED = 0x40000;
const int PREF_AV_MAP_ANIMATIONS = 0x80;
const int PREF_AV_SECRET_PROJECT_MOVIES = 0x10000000;
const int PREF_AV_SLIDING_WINDOWS = 0x8000000;
const int PREF_AV_SOUND_EFFECTS = 0x400;
const int PREF_AV_VOLUME_MUSIC_TOGGLE = 0x800;
const int PREF_AV_VOLUME_SFX_TOGGLE = 0x400;
const int PREF_BSC_AUTOSAVE_EACH_TURN = 0x2;
const int PREF_BSC_AUTO_DESIGN_VEH = 0x4000;
const int PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH = 0x8000;
const int PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH = 0x4;
const int PREF_BSC_MOUSE_EDGE_SCROLL_VIEW = 0x1000;
const int PREF_BSC_PAUSE_END_TURN = 0x1;
const int PREF_BSC_TUTORIAL_MSGS = 0x20;
const int PREF_MAP_SHOW_BASE_GRID = 0x200;
const int PREF_MAP_SHOW_GRID = 0x100;
const int PREF_UNK_10 = 0x10;
const int PROP_ELECT_PLANETARY_GOVERNOR = 0;
const int PROP_GLOBAL_TRADE_PACT = 3;
const int PROP_INCREASE_SOLAR_SHADE = 6;
const int PROP_LAUNCH_SOLAR_SHADE = 5;
const int PROP_MELT_POLAR_CAPS = 7;
const int PROP_REINSTATE_UN_CHARTER = 9;
const int PROP_REPEAL_GLOBAL_TRADE_PACT = 4;
const int PROP_REPEAL_UN_CHARTER = 8;
const int PROP_SALVAGE_UNITY_CORE = 2;
const int PROP_UNITE_SUPREME_LEADER = 1;
const int PROTO_ACTIVE = 0x1;
const int PROTO_CUSTOM_NAME_SET = 0x2;
const int PROTO_TYPED_COMPLETE = 0x4;
const int PROTO_UNK_10 = 0x10;
const int PROTO_UNK_20 = 0x20;
const int RAINFALL_ARID = 0x0;
const int RAINFALL_MOIST = 0x8;
const int RAINFALL_RAINY = 0x10;
const int RECT_FISSION = 1;
const int RECT_FUSION = 2;
const int RECT_QUANTUM = 3;
const int RECT_SINGULARITY = 4;
const int RFLAG_ALIEN = 0x80;
const int RFLAG_AQUATIC = 0x100;
const int RFLAG_COMMFREQ = 0x1000;
const int RFLAG_FANATIC = 0x400;
const int RFLAG_FREEPROTO = 0x200;
const int RFLAG_INTEREST = 0x4000;
const int RFLAG_MINDCONTROL = 0x800;
const int RFLAG_MORALE = 0x8000;
const int RFLAG_TECHSHARE = 0x20;
const int RFLAG_TECHSTEAL = 0x10;
const int RFLAG_TERRAFORM = 0x2000;
const int RFLAG_WORMPOLICE = 0x40;
const int ROCKINESS_FLAT = 0;
const int ROCKINESS_ROCKY = 2;
const int ROCKINESS_ROLLING = 1;
const int RSCINFO_BASE_SQ = 1;
const int RSCINFO_BONUS_SQ = 2;
const int RSCINFO_BOREHOLE_SQ = 8;
const int RSCINFO_FOREST_SQ = 3;
const int RSCINFO_IMPROVED_LAND = 5;
const int RSCINFO_IMPROVED_SEA = 6;
const int RSCINFO_MONOLITH = 7;
const int RSCINFO_OCEAN_SQ = 0;
const int RSCINFO_RECYCLING_TANKS = 4;
const int RSC_ENERGY = 2;
const int RSC_MINERALS = 1;
const int RSC_NUTRIENTS = 0;
const int RSC_PSI = 3;
const int RULES_BELL_CURVE = 0x8000;
const int RULES_BLIND_RESEARCH = 0x200;
const int RULES_DO_OR_DIE = 0x1;
const int RULES_INTENSE_RIVALRY = 0x40;
const int RULES_IRONMAN = 0x400;
const int RULES_LOOK_FIRST = 0x10;
const int RULES_NO_UNITY_SCATTERING = 0x2000;
const int RULES_NO_UNITY_SURVEY = 0x100;
const int RULES_SCN_FORCE_CURRENT_DIFF_LEVEL = 0x1000000;
const int RULES_SCN_FORCE_PLAYER_PLAY_CURRENT_FACT = 0x100000;
const int RULES_SCN_NO_BUILDING_SP = 0x80000000;
const int RULES_SCN_NO_COLONY_PODS = 0x400000;
const int RULES_SCN_NO_NATIVE_LIFE = 0x200000;
const int RULES_SCN_NO_TECH_ADVANCES = 0x4000000;
const int RULES_SCN_NO_TECH_TRADING = 0x2000000;
const int RULES_SCN_NO_TERRAFORMING = 0x800000;
const int RULES_SCN_UNITY_PODS_NO_ARTIFACTS = 0x40000;
const int RULES_SCN_UNITY_PODS_NO_MONOLITHS = 0x20000;
const int RULES_SCN_UNITY_PODS_NO_RESOURCES = 0x10000;
const int RULES_SCN_VICT_ALL_BASE_COUNT_OBJ = 0x20000000;
const int RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE = 0x10000000;
const int RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE = 0x8000000;
const int RULES_SCN_VICT_SOLO_MISSION = 0x80000;
const int RULES_SCN_VICT_SP_COUNT_OBJ = 0x40000000;
const int RULES_SPOILS_OF_WAR = 0x4000;
const int RULES_TECH_STAGNATION = 0x20;
const int RULES_TIME_WARP = 0x80;
const int RULES_VICTORY_CONQUEST = 0x2;
const int RULES_VICTORY_COOPERATIVE = 0x1000;
const int RULES_VICTORY_DIPLOMATIC = 0x8;
const int RULES_VICTORY_ECONOMIC = 0x4;
const int RULES_VICTORY_TRANSCENDENCE = 0x800;
const int RULE_DEFENSE = 17;
const int RULE_FACILITY = 2;
const int RULE_FREEABIL = 15;
const int RULE_FREEFAC = 12;
const int RULE_FUNGENERGY = 9;
const int RULE_FUNGMINERALS = 8;
const int RULE_FUNGNUTRIENT = 7;
const int RULE_IMMUNITY = 4;
const int RULE_IMPUNITY = 5;
const int RULE_NODRONE = 14;
const int RULE_OFFENSE = 18;
const int RULE_PENALTY = 6;
const int RULE_PROBECOST = 16;
const int RULE_REVOLT = 13;
const int RULE_ROBUST = 10;
const int RULE_SOCIAL = 3;
const int RULE_TECH = 0;
const int RULE_UNIT = 1;
const int RULE_VOTES = 11;
const int RegionBounds = 63;
const int SE_CYBERNETIC = 1;
const int SE_DEMOCRATIC = 2;
const int SE_EUDAIMONIC = 2;
const int SE_FREE_MARKET = 1;
const int SE_FRONTIER = 0;
const int SE_FUNDAMENTALIST = 3;
const int SE_GREEN = 3;
const int SE_KNOWLEDGE = 2;
const int SE_NONE = 0;
const int SE_PLANNED = 2;
const int SE_POLICE_STATE = 1;
const int SE_POWER = 1;
const int SE_SIMPLE = 0;
const int SE_SURVIVAL = 0;
const int SE_THOUGHT_CONTROL = 3;
const int SE_WEALTH = 3;
const int SOCIAL_CAT_ECONOMICS = 1;
const int SOCIAL_CAT_FUTURE = 3;
const int SOCIAL_CAT_POLITICS = 0;
const int SOCIAL_CAT_VALUES = 2;
const int SP_ASCENT_TO_TRANSCENDENCE = 32;
const int SP_ASCETIC_VIRTUES = 13;
const int SP_BULK_MATTER_TRANSMITTER = 29;
const int SP_CITIZENS_DEFENSE_FORCE = 5;
const int SP_CLINICAL_IMMORTALITY = 26;
const int SP_CLONING_VATS = 24;
const int SP_CLOUDBASE_ACADEMY = 35;
const int SP_COMMAND_NEXUS = 1;
const int SP_CYBORG_FACTORY = 17;
const int SP_DREAM_TWISTER = 19;
const int SP_Destroyed = -2;
const int SP_EMPATH_GUILD = 4;
const int SP_EMPTY_38 = 37;
const int SP_EMPTY_39 = 38;
const int SP_EMPTY_40 = 39;
const int SP_EMPTY_41 = 40;
const int SP_EMPTY_42 = 41;
const int SP_EMPTY_43 = 42;
const int SP_EMPTY_44 = 43;
const int SP_EMPTY_45 = 44;
const int SP_EMPTY_46 = 45;
const int SP_EMPTY_47 = 46;
const int SP_EMPTY_48 = 47;
const int SP_EMPTY_49 = 48;
const int SP_EMPTY_50 = 49;
const int SP_EMPTY_51 = 50;
const int SP_EMPTY_52 = 51;
const int SP_EMPTY_53 = 52;
const int SP_EMPTY_54 = 53;
const int SP_EMPTY_55 = 54;
const int SP_EMPTY_56 = 55;
const int SP_EMPTY_57 = 56;
const int SP_EMPTY_58 = 57;
const int SP_EMPTY_59 = 58;
const int SP_EMPTY_60 = 59;
const int SP_EMPTY_61 = 60;
const int SP_EMPTY_62 = 61;
const int SP_EMPTY_63 = 62;
const int SP_EMPTY_64 = 63;
const int SP_HUMAN_GENOME_PROJ = 0;
const int SP_HUNTER_SEEKER_ALGO = 15;
const int SP_LIVING_REFINERY = 23;
const int SP_LONGEVITY_VACCINE = 14;
const int SP_MANIFOLD_HARMONICS = 33;
const int SP_MARITIME_CONTROL_CENTER = 10;
const int SP_MERCHANT_EXCHANGE = 3;
const int SP_NANO_FACTORY = 22;
const int SP_NETHACK_TERMINUS = 34;
const int SP_NETWORK_BACKBONE = 21;
const int SP_NEURAL_AMPLIFIER = 9;
const int SP_PHOLUS_MUTAGEN = 16;
const int SP_PLANETARY_DATALINKS = 11;
const int SP_PLANETARY_ENERGY_GRID = 36;
const int SP_PLANETARY_TRANS_SYS = 7;
const int SP_SELF_AWARE_COLONY = 25;
const int SP_SINGULARITY_INDUCTOR = 28;
const int SP_SPACE_ELEVATOR = 27;
const int SP_SUPERCOLLIDER = 12;
const int SP_TELEPATHIC_MATRIX = 30;
const int SP_THEORY_OF_EVERYTHING = 18;
const int SP_UNIVERSAL_TRANSLATOR = 20;
const int SP_Unbuilt = -1;
const int SP_VIRTUAL_WORLD = 6;
const int SP_VOICE_OF_PLANET = 31;
const int SP_WEATHER_PARADIGM = 2;
const int SP_XENOEMPATYH_DOME = 8;
const int STATE_COUNCIL_HAS_CONVENED = 0x10000;
const int STATE_DEBUG_MODE = 0x1000;
const int STATE_DISPLAYED_COUNCIL_AVAIL_MSG = 0x4000;
const int STATE_EDITOR_ONLY_MODE = 0x8000000;
const int STATE_IS_SCENARIO = 0x10;
const int STATE_OMNISCIENT_VIEW = 0x80;
const int STATE_PERIHELION_ACTIVE = 0x100000;
const int STATE_RAND_FAC_LEADER_PERSONALITIES = 0x800000;
const int STATE_RAND_FAC_LEADER_SOCIAL_AGENDA = 0x1000000;
const int STATE_SCENARIO_CHEATED_FLAG = 0x20;
const int STATE_SCENARIO_EDITOR = 0x40;
const int STATE_SCN_VICT_ALL_ARTIFACTS_OBJ_UNIT = 0x40000;
const int STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ = 0x4000000;
const int STATE_SCN_VICT_CREDITS_COUNT_OBJ = 0x80000000;
const int STATE_SCN_VICT_HIGHEST_AC_SCORE_WINS = 0x80000;
const int STATE_SCN_VICT_POPULATION_COUNT_OBJ = 0x20000000;
const int STATE_SCN_VICT_TECH_COUNT_OBJ = 0x40000000;
const int STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ = 0x2000000;
const int STATE_SCN_VICT_TERRITORY_COUNT_OBJ = 0x8000;
const int STATE_UNK_1 = 0x1;
const int STATE_UNK_100 = 0x100;
const int STATE_UNK_10000000 = 0x10000000;
const int STATE_UNK_2 = 0x2;
const int STATE_UNK_200 = 0x200;
const int STATE_UNK_4 = 0x4;
const int STATE_UNK_400 = 0x400;
const int STATE_UNK_8 = 0x8;
const int STATE_UNK_800 = 0x800;
const int STATE_VICTORY_CONQUER = 0x2000;
const int STATE_VICTORY_DIPLOMATIC = 0x200000;
const int STATE_VICTORY_ECONOMIC = 0x400000;
const int STATE_VOLCANO_ERUPTED = 0x20000;
const int SpyingBaseStride = 0x134;
const int SpyingFactionStride = 0x59C;
const int SpyingStatusStride = 2099;
const int StringListVirtualBaseOffset = 0x28;
const int StringStructCloseAdjustment = 0x1C;
const int StringStructDerivedCloseAdjustment = 0x28;
const int TECH_ADAPDOC = 80;
const int TECH_ADAPECO = 81;
const int TECH_AGRAV = 23;
const int TECH_ALGOR = 30;
const int TECH_ALLOYS = 11;
const int TECH_ALPHCEN = 64;
const int TECH_BFG9000 = 86;
const int TECH_BIOADAP = 82;
const int TECH_BIOENG = 50;
const int TECH_BIOGEN = 0;
const int TECH_BIOMAC = 51;
const int TECH_BRAIN = 48;
const int TECH_CENTEMP = 58;
const int TECH_CENTGEN = 74;
const int TECH_CENTMED = 47;
const int TECH_CENTPSI = 63;
const int TECH_CHAOS = 8;
const int TECH_CHEMIST = 13;
const int TECH_CONSING = 27;
const int TECH_CREATE = 65;
const int TECH_CYBER = 53;
const int TECH_DELETED_2 = 70;
const int TECH_DELETE_1 = 24;
const int TECH_DIGSENT = 33;
const int TECH_DOCAIR = 39;
const int TECH_DOCFLEX = 36;
const int TECH_DOCINIT = 35;
const int TECH_DOCLOY = 43;
const int TECH_DOCSEC = 40;
const int TECH_ECOENG = 60;
const int TECH_ECOENG2 = 62;
const int TECH_ECOLOGY = 6;
const int TECH_ENVECON = 59;
const int TECH_ETHCALC = 44;
const int TECH_EUDAIM = 54;
const int TECH_E_MC2 = 9;
const int TECH_FLDMOD = 79;
const int TECH_FOSSIL = 38;
const int TECH_FUSION = 10;
const int TECH_GENE = 49;
const int TECH_GRAVITY = 21;
const int TECH_HAL9000 = 34;
const int TECH_HOMOSUP = 67;
const int TECH_INDAUTO = 46;
const int TECH_INDECON = 45;
const int TECH_INDROB = 73;
const int TECH_INDUST = 1;
const int TECH_INFNET = 2;
const int TECH_INTEG = 37;
const int TECH_MAGNETS = 18;
const int TECH_MATCOMP = 19;
const int TECH_MATTER = 57;
const int TECH_METAL = 15;
const int TECH_MILALG = 17;
const int TECH_MINDMAC = 41;
const int TECH_MOBILE = 5;
const int TECH_NANEDIT = 71;
const int TECH_NANOMIN = 42;
const int TECH_NEURAL = 52;
const int TECH_NEWMISS = 85;
const int TECH_OPTCOMP = 72;
const int TECH_ORBITAL = 77;
const int TECH_PHYSIC = 3;
const int TECH_PLAECON = 61;
const int TECH_PLANETS = 32;
const int TECH_POLY = 22;
const int TECH_PROBMEC = 29;
const int TECH_PRPSYCH = 78;
const int TECH_PSYCH = 4;
const int TECH_QUANMAC = 69;
const int TECH_QUANTUM = 25;
const int TECH_SECMANI = 84;
const int TECH_SENTECO = 75;
const int TECH_SENTRES = 83;
const int TECH_SINGMEC = 26;
const int TECH_SOLIDS = 31;
const int TECH_SPACE = 66;
const int TECH_STRING = 16;
const int TECH_SUBAT = 12;
const int TECH_SUPER = 7;
const int TECH_SUPLUBE = 68;
const int TECH_SURFACE = 14;
const int TECH_TEMPMEC = 28;
const int TECH_THRESH = 56;
const int TECH_TRANT = 88;
const int TECH_UNIFIED = 20;
const int TECH_USER = 87;
const int TECH_VIRAL = 76;
const int TECH_WILLPOW = 55;
const int TERRAIN_BIT_ROCKY = 0x80;
const int TERRAIN_BIT_ROLLING = 0x40;
const int TERRA_AIRBASE = 8;
const int TERRA_AQUIFER = 15;
const int TERRA_BUNKER = 7;
const int TERRA_CONDENSER = 12;
const int TERRA_ECH_MIRROR = 13;
const int TERRA_FARM = 0;
const int TERRA_FOREST = 4;
const int TERRA_LEVEL_TERRAIN = 18;
const int TERRA_LOWER_LAND = 17;
const int TERRA_MAGTUBE = 6;
const int TERRA_MINE = 2;
const int TERRA_MONOLITH = 19;
const int TERRA_PLANT_FUNGUS = 11;
const int TERRA_RAISE_LAND = 16;
const int TERRA_REMOVE_FUNGUS = 10;
const int TERRA_ROAD = 5;
const int TERRA_SENSOR = 9;
const int TERRA_SOIL_ENR = 1;
const int TERRA_SOLAR_TIDAL = 3;
const int TERRA_THERMAL_BORE = 14;
const int TFLAG_ALLOW_GENE_WARFARE = 0x10;
const int TFLAG_IMPROVED_PROBES = 0x2;
const int TFLAG_INC_COMMERCE = 0x4;
const int TFLAG_INC_ENERGY_FUNGUS = 0x40;
const int TFLAG_INC_GENE_WARFARE_DEFENSE = 0x20;
const int TFLAG_INC_MINERALS_FUNGUS = 0x80;
const int TFLAG_INC_NUTRIENT_FUNGUS = 0x100;
const int TFLAG_REVEALS_MAP = 0x8;
const int TFLAG_SECRETS = 0x1;
const int TRIAD_AIR = 2;
const int TRIAD_LAND = 0;
const int TRIAD_SEA = 1;
const int TechDisabled = -2;
const int TechNone = -1;
const int VFLAG_INVISIBLE = 0x400;
const int VFLAG_IS_OBJECTIVE = 0x20;
const int VFLAG_LURKER = 0x40;
const int VFLAG_PROBE_PACT_OPERATIONS = 0x4;
const int VFLAG_START_RAND_FUNGUS = 0x200;
const int VFLAG_START_RAND_LOCATION = 0x80;
const int VFLAG_START_RAND_MONOLITH = 0x100;
const int VFLAG_UNK_1 = 0x1;
const int VFLAG_UNK_1000 = 0x1000;
const int VFLAG_UNK_2 = 0x2;
const int VSTATE_ASSISTANT_WORM = 0x800000;
const int VSTATE_CRAWLING = 0x4000000;
const int VSTATE_DESIGNATE_DEFENDER = 0x80;
const int VSTATE_EXPLORE = 0x4000;
const int VSTATE_MADE_AIRDROP = 0x20;
const int VSTATE_MONOLITH_UPGRADED = 0x100;
const int VSTATE_PACIFISM_DRONE = 0x200000;
const int VSTATE_PACIFISM_FREE_SKIP = 0x400000;
const int VSTATE_REQUIRES_SUPPORT = 0x10;
const int VSTATE_UNK_1 = 0x1;
const int VSTATE_UNK_1000 = 0x1000;
const int VSTATE_UNK_10000 = 0x10000;
const int VSTATE_UNK_100000 = 0x100000;
const int VSTATE_UNK_1000000 = 0x1000000;
const int VSTATE_UNK_10000000 = 0x10000000;
const int VSTATE_UNK_2 = 0x2;
const int VSTATE_UNK_200 = 0x200;
const int VSTATE_UNK_2000 = 0x2000;
const int VSTATE_UNK_20000 = 0x20000;
const int VSTATE_UNK_2000000 = 0x2000000;
const int VSTATE_UNK_20000000 = 0x20000000;
const int VSTATE_UNK_4 = 0x4;
const int VSTATE_UNK_40 = 0x40;
const int VSTATE_UNK_400 = 0x400;
const int VSTATE_UNK_40000 = 0x40000;
const int VSTATE_UNK_40000000 = 0x40000000;
const int VSTATE_UNK_8 = 0x8;
const int VSTATE_UNK_800 = 0x800;
const int VSTATE_UNK_8000 = 0x8000;
const int VSTATE_UNK_8000000 = 0x8000000;
const int VSTATE_UNK_80000000 = 0x80000000;
const int VSTATE_USED_NERVE_GAS = 0x80000;
const int WARN_STOP_BUILD_OUT_OF_DATE = 0x200;
const int WARN_STOP_BUILT_VIA_GOV_QUEUE = 0x2000;
const int WARN_STOP_COMBAT_VEH_BUILT = 0x400;
const int WARN_STOP_DELAY_IN_TRANSCEND = 0x1000;
const int WARN_STOP_DRONE_RIOTS = 0x8;
const int WARN_STOP_DRONE_RIOTS_END = 0x10;
const int WARN_STOP_ENERGY_SHORTAGE = 0x10000;
const int WARN_STOP_GOLDEN_AGE = 0x20;
const int WARN_STOP_GOLDEN_AGE_END = 0x40;
const int WARN_STOP_MINERAL_SHORTAGE = 0x8000;
const int WARN_STOP_NEW_FAC_BUILT = 0x1;
const int WARN_STOP_NON_COMBAT_VEH_BUILT = 0x2;
const int WARN_STOP_NUTRIENT_SHORTAGE = 0x80;
const int WARN_STOP_POP_LIMIT_REACHED = 0x800;
const int WARN_STOP_PROTOTYPE_COMPLETE = 0x4;
const int WARN_STOP_RANDOM_EVENT = 0x20000;
const int WARN_STOP_STARVATION = 0x4000;
const int WARN_STOP_UNK_100 = 0x100;
const int WPN_ALIEN_ARTIFACT = 22;
const int WPN_CHAOS_GUN = 5;
const int WPN_COLONY_MODULE = 17;
const int WPN_CONVENTIONAL_PAYLOAD = 23;
const int WPN_FUNGAL_PAYLOAD = 25;
const int WPN_FUSION_LASER = 6;
const int WPN_GATLING_LASER = 3;
const int WPN_GRAVITON_GUN = 10;
const int WPN_HAND_WEAPONS = 0;
const int WPN_LASER = 1;
const int WPN_MISSILE_LAUNCHER = 4;
const int WPN_MODE_ARTIFACT = 12;
const int WPN_MODE_COLONIST = 8;
const int WPN_MODE_CONVOY = 10;
const int WPN_MODE_ENERGY = 1;
const int WPN_MODE_INFOWAR = 11;
const int WPN_MODE_MISSILE = 2;
const int WPN_MODE_PROJECTILE = 0;
const int WPN_MODE_TERRAFORMER = 9;
const int WPN_MODE_TRANSPORT = 7;
const int WPN_PARTICLE_IMPACTOR = 2;
const int WPN_PLANET_BUSTER = 16;
const int WPN_PLASMA_SHARD = 8;
const int WPN_PROBE_TEAM = 21;
const int WPN_PSI_ATTACK = 15;
const int WPN_QUANTUM_LASER = 9;
const int WPN_RESONANCE_BOLT = 13;
const int WPN_RESONANCE_LASER = 12;
const int WPN_SINGULARITY_LASER = 11;
const int WPN_STRING_DISRUPTOR = 14;
const int WPN_SUPPLY_TRANSPORT = 20;
const int WPN_TACHYON_BOLT = 7;
const int WPN_TECTONIC_PAYLOAD = 24;
const int WPN_TERRAFORMING_UNIT = 18;
const int WPN_TROOP_TRANSPORT = 19;
const int ElevDetail[] = {0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
const int RadiusBaseX[] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
const int RadiusBaseY[] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
const int RadiusOffsetX[] = {0, 1, 2, 1, 0, -1, -2, -1, 0, 2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1, 4, -4, 0, 0, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5, -5, -4, -3, -2, -1, 0, 6, 0, -6, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1,};
const int RadiusOffsetY[] = {0, -1, 0, 1, 2, 1, 0, -1, -2, -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1, -3, 0, 0, 4, -4, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5, 6, 0, -6, 0, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15,};
const int RadiusRange[] = {1, 9, 25, 49, 81, 121, 169, 225, 289};

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class BaseButton;
class Buffer;
class ButtonGroup;
class FlatButton;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
struct MenuEntry;
typedef void * PVOID;
class PullDown;
struct PullDownItem;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Time;
typedef unsigned int UINT_PTR;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
//
// `static` ON A CALLEE IS DELIBERATE. A method whose
// mangled infix is `QAA` or `QAG` takes NO `this` -
// every argument is on the stack - so the call site is
// `Class::method(...)` with no object, and declared
// non-static that spelling is `C2352: illegal call of
// non-static member function`. It does change the
// mangling from QAA to SA, which matters only for the
// SUBJECT; a callee is reached by a relocation the
// comparison masks, so its mangling reaches nothing.
//
// THE class/struct KEY IS NOT A GUESS EITHER, and must
// not be `corrected` against the catalogue. MSVC
// mangles struct `U` and class `V`, six classes
// disagree with THEMSELVES in the catalogue, and the
// image carries no RTTI to settle it. Both objects are
// ours: `recovery_symbols.canonicalise_class_keys`
// rewrites the TARGET object with the same map this
// unit uses, so they agree by construction. Changing
// one side alone is what breaks it.
class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
    void close();
    void close2();
    void construct();
    void init();
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    LPVOID get_base();
    size_t get_base_size();
    void shutdown();
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    int UNK5();
    int get_horz_pos();
    int get_lbutton_state();
    int get_vert_pos();
    int is_dialog_focus();
    int is_visible();
    int on_query_new_palette();
    static HDC get_hdc();
    static void clear_bubble_text();
    static void release_hdc();
    void construct();
    void hide();
    void release_modal();
    void reset_window_clip();
    void sync_palette();
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    void clear();
    void shutdown();
};

class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
    HDC get_hdc();
    int get_data();
    int init_class();
    int text_height();
    int text_line_height();
    void clear_links();
    void close();
    void close_class();
    void construct();
    void destroy();
};

class GraphicWin : public Win { public:
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
    uint32_t close();
    void construct();
    void redraw();
    void soft_update();
};

class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    static int __cdecl init_class();
    static void __cdecl close_class();
    uint32_t pulse();
    uint32_t start();
    void close();
    void release_modal();
    void set_modal();
    void stop();
};

class BaseButton : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    Time time1_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    Time time2_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    LPSTR name_;
    LPSTR bubble_text_;
    uint32_t color_;
    uint32_t bevel_upper_;
    uint32_t bevel_lower_;
    uint32_t bevel_thickness_;
    uint32_t field_A94_;
    uint32_t field_A98_;
    uint32_t field_A9C_;
    uint32_t field_AA0_;
    uint32_t field_AA4_;
    ButtonGroup * group_;
    uint32_t field_AAC_;
    uint32_t field_AB0_;
    uint32_t field_AB4_;
    BaseButton * destroy();
    uint32_t close();
    void construct();
};

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    void close();
};

class FlatButton : public BaseButton { public:
    uint32_t field_AB8_;
    uint32_t field_ABC_;
    uint32_t field_AC0_;
    uint32_t field_AC4_;
    uint32_t field_AC8_;
    uint32_t field_ACC_;
    uint32_t field_AD0_;
    uint32_t field_AD4_;
    uint32_t field_AD8_;
    uint32_t field_ADC_;
    Sprite * sprite0_;
    uint32_t field_AE4_;
    uint32_t field_AE8_;
    uint32_t field_AEC_;
    uint32_t field_AF0_;
    uint32_t field_AF4_;
    uint32_t field_AF8_;
    uint32_t field_AFC_;
    uint32_t field_B00_;
    uint32_t field_B04_;
    uint32_t field_B08_;
    uint32_t field_B0C_;
    Sprite * sprite1_;
    Sprite * sprite2_;
    Sprite * sprite3_;
    uint32_t field_B1C_;
    uint32_t field_B20_;
    uint32_t field_B24_;
    uint32_t field_B28_;
    uint32_t field_B2C_;
    uint32_t field_B30_;
    uint32_t field_B34_;
    uint32_t field_B38_;
    uint32_t field_B3C_;
    uint32_t field_B40_;
    uint32_t field_B44_;
    uint32_t field_B48_;
    FlatButton * destroy();
    uint32_t close();
};

class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
    bool is_initialized() const;
    int UNK1(int, int, int, int);
    static void __cdecl close_font_class();
    void close();
};

struct MenuEntry {
    int id;
    char * text;
    uint8_t flags;
    uint8_t padding[3];
    char * mnemonic;
    PullDown * pull_down;
};

class Menu : public GraphicWin { public:
    void * proc_;
    int count_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_A28_;
    Spot spot_;
    MenuEntry entries_[15];
    int requested_height();
};

struct PullDownItem {
    char * text;
    char * right_text;
    int id;
    uint32_t flags;
    char * mnemonic;
};

class PullDown : public GraphicWin { public:
    uint32_t field_A14_;
    PullDownItem items_[64];
    uint32_t field_F18_;
    uint32_t field_F1C_;
    int item_count_;
    int width_;
    int selected_;
    int visible_count_;
    Menu * menu_;
    uint8_t dirty_;
    uint8_t dirty_padding_[3];
    uint32_t field_F38_;
    uint32_t field_F3C_;
    int UNK5();
    int UNK6();
    int get_selected();
    void hide();
};

class Scroll : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    int border_color_;
    int range_minimum_;
    int range_maximum_;
    uint32_t range_reversed_;
    int position_;
    uint32_t field_A30_;
    uint32_t field_A34_;
    uint32_t field_A38_;
    uint32_t field_A3C_;
    int paging_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    RECT thumb_rect_;
    uint32_t color_;
    int bar_thickness_;
    Buffer * buffer_;
    uint32_t bevel_thickness_;
    uint32_t bevel_upper_;
    uint32_t bevel_lower_;
    uint32_t field_A74_;
    Sprite * field_A78_;
    Sprite * sprite_left1_;
    Sprite * sprite_left2_;
    Sprite * sprite_left3_;
    Sprite * sprite_up1_;
    Sprite * sprite_up2_;
    Sprite * sprite_up3_;
    Sprite * sprite_right1_;
    Sprite * sprite_right2_;
    Sprite * sprite_right3_;
    Sprite * sprite_down1_;
    Sprite * sprite_down2_;
    Sprite * sprite_down3_;
    FlatButton flat_button_left_;
    FlatButton flat_button_right_;
    uint32_t field_2144_;
    uint32_t field_2148_;
    Scroll * destroy();
    uint32_t close();
    uint32_t set_thumb_rect();
};

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
    void close();
};

extern "C" int __cdecl sub_5b5700();
extern "C" int abs(int);
int base_at(int, int);
int base_find(int, int);
int has_abil(int, int);
int is_sensor(int, int);
int on_map(int, int);
int stack_check(int, int, int, int, int);
int stack_fix(int);
int veh_at(int, int);
int veh_top(int);
int want_to_wake(int, int, int);
int whose_territory(int, int, int, int *, int);
void add_goal(int, int, int, int, int, int);
void bitmask(int, int *, int *);
void draw_radius(int, int, int, int);
void encounter(int, int, int, int, int, int);
void interlude(int, char *, int, int);
void synch_bit(int, int, int);
void wake_stack(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_0066ef50 = (int *)0x0066EF50;
static int *const g_0066ef74 = (int *)0x0066EF74;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_0090d91c = (int *)0x0090D91C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_0094a37a = (int *)0x0094A37A;
static int *const g_0094ae68 = (int *)0x0094AE68;
static int *const g_00952828 = (int *)0x00952828;
static int *const g_0095282a = (int *)0x0095282A;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_00952830 = (int *)0x00952830;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_00952839 = (int *)0x00952839;
static int *const g_0095283a = (int *)0x0095283A;
static int *const g_0095283b = (int *)0x0095283B;
static int *const g_0095284f = (int *)0x0095284F;
static int *const g_00952858 = (int *)0x00952858;
static int *const g_0095285a = (int *)0x0095285A;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9e1 = (int *)0x0096C9E1;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cdd0 = (int *)0x0096CDD0;
static int *const g_0096cdd4 = (int *)0x0096CDD4;
static int *const g_0097d040 = (int *)0x0097D040;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d046 = (int *)0x0097D046;
static int *const g_0097d04a = (int *)0x0097D04A;
static int *const g_0097d04b = (int *)0x0097D04B;
static int *const g_0097d0cc = (int *)0x0097D0CC;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab88d = (int *)0x009AB88D;
static int *const g_009ab892 = (int *)0x009AB892;
#define SETB0(dst,val) ((dst) = (((dst) & ~0xff) | ((val) & 0xff)))
#define SETB1(dst,val) ((dst) = (((dst) & ~0xff00) | (((val) & 0xff) << 8)))
#define SETW0(dst,val) ((dst) = (((dst) & ~0xffff) | ((val) & 0xffff)))

void __cdecl spot_all(int a1, int a2) {
    int eax=0, ebx=0, ecx=0, edx=0, esi=0, edi=0;
    int var_1, var_8, var_c, var_10, var_14, var_18, var_1c, var_20, var_24, var_28, var_2c, var_30, var_34, var_38, var_3c, var_40, var_44, var_48, var_4c, var_50, var_54, var_58, var_5c, var_60, var_64;

    // 005B6266  mov ecx, dword ptr [0x93f660]
    ecx = ((*(int*)((char*)g_0093f660)));
    // 005B626C  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B6270  cmp ecx, eax
    // 005B6273  mov dword ptr [ebp - 0x58], eax
    var_58 = (eax);
    // 005B6276  mov dword ptr [ebp - 0x60], eax
    var_60 = (eax);
    // 005B6279  mov dword ptr [ebp - 0x5c], eax
    var_5c = (eax);
    // 005B627C  mov esi, 1
    esi = (1);
    // 005B6281  jne 0x5b6286
    if ((ecx) != (eax)) goto LAB_005B6286;
    // 005B6283  mov dword ptr [ebp + 0xc], esi
    a2 = (esi);
LAB_005B6286:
    // 005B6286  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B6289  push 0x40
    // 005B628B  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B628E  lea edi, [eax + ecx*4]
    edi = (eax + ecx*4);
    // 005B6291  shl edi, 2
    edi = ((edi) << (2));
    // 005B6294  mov dword ptr [ebp - 0x20], edi
    var_20 = (edi);
    // 005B6297  movsx eax, word ptr [edi + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + edi)));
    // 005B629E  movsx edx, word ptr [edi + 0x95282a]
    edx = ((*(short*)((char*)g_0095282a + edi)));
    // 005B62A5  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B62A8  mov dword ptr [ebp - 0x24], edx
    var_24 = (edx);
    // 005B62AB  movsx ebx, word ptr [edi + 0x952828]
    ebx = ((*(short*)((char*)g_00952828 + edi)));
    // 005B62B2  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B62B5  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B62B7  push eax
    // 005B62B8  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 005B62BB  mov cl, byte ptr [edx*4 + 0x9ab88c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009ab88c + edx*4))));
    // 005B62C2  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B62C4  lea ecx, [ecx + ecx*8]
    ecx = (ecx + ecx*8);
    // 005B62C7  shl ecx, 4
    ecx = ((ecx) << (4));
    // 005B62CA  mov dl, byte ptr [ecx + 0x94a379]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094a379 + ecx))));
    // 005B62D0  mov dword ptr [ebp - 0x3c], edx
    var_3c = (edx);
    // 005B62D3  call 0x5bf1f0
    eax = has_abil(eax, 0x40);
    // 005B62D8  add esp, 8
    // 005B62DB  test eax, eax
    // 005B62DD  je 0x5b62e7
    if ((((eax) & (eax))) == 0) goto LAB_005B62E7;
    // 005B62DF  mov dword ptr [ebp - 0x34], esi
    var_34 = (esi);
    // 005B62E2  jmp 0x5b636d
    goto LAB_005B636D;
LAB_005B62E7:
    // 005B62E7  movsx eax, word ptr [edi + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + edi)));
    // 005B62EE  push 4
    // 005B62F0  push eax
    // 005B62F1  call 0x5bf1f0
    eax = has_abil(eax, 4);
    // 005B62F6  add esp, 8
    // 005B62F9  test eax, eax
    // 005B62FB  je 0x5b6302
    if ((((eax) & (eax))) == 0) goto LAB_005B6302;
    // 005B62FD  mov dword ptr [ebp - 0x34], esi
    var_34 = (esi);
    // 005B6300  jmp 0x5b636d
    goto LAB_005B636D;
LAB_005B6302:
    // 005B6302  mov cx, word ptr [edi + 0x952830]
    SETW0(ecx, ((*(unsigned short*)((char*)g_00952830 + edi))));
    // 005B6309  and ecx, 0x400
    ecx = ((ecx) & (0x400));
    // 005B630F  test cx, cx
    // 005B6312  je 0x5b6319
    if (((((ecx & 0xffff)) & ((ecx & 0xffff)))) == 0) goto LAB_005B6319;
    // 005B6314  mov dword ptr [ebp - 0x34], esi
    var_34 = (esi);
    // 005B6317  jmp 0x5b636d
    goto LAB_005B636D;
LAB_005B6319:
    // 005B6319  movsx eax, word ptr [edi + 0x95282a]
    eax = ((*(short*)((char*)g_0095282a + edi)));
    // 005B6320  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B6327  movsx edx, word ptr [edi + 0x952828]
    edx = ((*(short*)((char*)g_00952828 + edi)));
    // 005B632E  sar edx, 1
    edx = ((edx) >> (1));
    // 005B6330  add eax, edx
    eax = ((eax) + (edx));
    // 005B6332  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B6335  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6338  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B633D  mov cl, byte ptr [eax + edx*4 + 8]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + edx*4 + 0x8))));
    // 005B6341  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B6344  test cl, 0x20
    // 005B6347  je 0x5b6366
    if (((((ecx & 0xff)) & (0x20))) == 0) goto LAB_005B6366;
    // 005B6349  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B634B  mov cl, byte ptr [eax]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax)))));
    // 005B634D  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B6350  cmp ecx, 0x40
    // 005B6353  jl 0x5b6366
    if (ecx < 0x40) goto LAB_005B6366;
    // 005B6355  cmp word ptr [edi + 0x952832], 0x13
    // 005B635D  mov dword ptr [ebp - 0x34], 2
    var_34 = (2);
    // 005B6364  jne 0x5b636d
    if (((*(unsigned short*)((char*)g_00952832 + edi))) != (0x13)) goto LAB_005B636D;
LAB_005B6366:
    // 005B6366  mov dword ptr [ebp - 0x34], 0
    var_34 = (0);
LAB_005B636D:
    // 005B636D  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B636F  mov eax, 1
    eax = (1);
    // 005B6374  mov dl, byte ptr [edi + 0x952836]
    SETB0(edx, ((*(unsigned char*)((char*)g_00952836 + edi))));
    // 005B637A  push 2
    // 005B637C  mov esi, edx
    esi = (edx);
    // 005B637E  mov ecx, esi
    ecx = (esi);
    // 005B6380  mov dword ptr [ebp - 8], esi
    var_8 = (esi);
    // 005B6383  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 005B6385  mov dword ptr [ebp - 0x28], eax
    var_28 = (eax);
    // 005B6388  movsx eax, word ptr [edi + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + edi)));
    // 005B638F  push eax
    // 005B6390  call 0x5bf1f0
    eax = has_abil(eax, 2);
    // 005B6395  add esp, 8
    // 005B6398  test eax, eax
    // 005B639A  jne 0x5b63c7
    if ((((eax) & (eax))) != 0) goto LAB_005B63C7;
    // 005B639C  mov ecx, dword ptr [ebp - 0x24]
    ecx = (var_24);
    // 005B639F  push ecx
    // 005B63A0  push ebx
    // 005B63A1  call 0x5bf010
    eax = is_sensor(ebx, ecx);
    // 005B63A6  add esp, 8
    // 005B63A9  test eax, eax
    // 005B63AB  je 0x5b63c3
    if ((((eax) & (eax))) == 0) goto LAB_005B63C3;
    // 005B63AD  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 005B63B0  push 0
    // 005B63B2  push 0
    // 005B63B4  push edx
    // 005B63B5  push ebx
    // 005B63B6  push esi
    // 005B63B7  call 0x4e3ef0
    eax = whose_territory(esi, ebx, edx, 0, 0);
    // 005B63BC  add esp, 0x14
    // 005B63BF  cmp eax, esi
    // 005B63C1  je 0x5b63c7
    if ((eax) == (esi)) goto LAB_005B63C7;
LAB_005B63C3:
    // 005B63C3  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B63C5  jmp 0x5b63cc
    goto LAB_005B63CC;
LAB_005B63C7:
    // 005B63C7  mov edx, 1
    edx = (1);
LAB_005B63CC:
    // 005B63CC  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B63D1  mov ecx, ebx
    ecx = (ebx);
    // 005B63D3  imul eax, dword ptr [ebp - 0x24]
    eax = ((eax) * (var_24));
    // 005B63D7  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B63D9  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B63DB  mov dword ptr [ebp - 0x54], ecx
    var_54 = (ecx);
    // 005B63DE  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 005B63E0  mov dword ptr [ebp - 0x40], edx
    var_40 = (edx);
    // 005B63E3  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B63E6  lea eax, [eax + ecx*2]
    eax = (eax + ecx*2);
    // 005B63E9  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B63EF  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B63F2  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B63F4  mov cl, byte ptr [eax]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax)))));
    // 005B63F6  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B63F9  cmp ecx, 0x60
    // 005B63FC  setl bl
    SETB0(ebx, ((ecx < 0x60) ? 1 : 0));
    // 005B63FF  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6401  mov dword ptr [ebp - 0x44], ebx
    var_44 = (ebx);
    // 005B6404  cmp esi, ecx
    // 005B6406  mov dword ptr [ebp - 0x4c], ecx
    var_4c = (ecx);
    // 005B6409  mov dword ptr [ebp - 0x1c], ecx
    var_1c = (ecx);
    // 005B640C  jne 0x5b641f
    if ((esi) != (ecx)) goto LAB_005B641F;
    // 005B640E  mov al, byte ptr [eax + 4]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B6411  mov bl, byte ptr [edi + 0x95284f]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0095284f + edi))));
    // 005B6417  and bl, al
    SETB0(ebx, (((ebx & 0xff)) & ((eax & 0xff))));
    // 005B6419  mov byte ptr [edi + 0x95284f], bl
    (*(unsigned char*)((char*)g_0095284f + edi)) = ((ebx & 0xff));
LAB_005B641F:
    // 005B641F  test byte ptr [0x9a6494], 1
    // 005B6426  je 0x5b6963
    if (((((*(unsigned char*)((char*)g_009a6494))) & (1))) == 0) goto LAB_005B6963;
    // 005B642C  cmp esi, dword ptr [0x939284]
    // 005B6432  jne 0x5b6963
    if ((esi) != ((*(int*)((char*)g_00939284)))) goto LAB_005B6963;
    // 005B6438  test byte ptr [0x9a64c0], 0x80
    // 005B643F  jne 0x5b6963
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B6963;
    // 005B6445..005B6447  neg/sbb -> (edx!=0)?-1:0
    edx = ((edx) != 0 ? -1 : 0);
    // 005B6449  mov dword ptr [ebp - 0x18], ecx
    var_18 = (ecx);
    // 005B644C  and edx, 0x18
    edx = ((edx) & (0x18));
    // 005B644F  add edx, 0x19
    edx = ((edx) + (0x19));
    // 005B6452  cmp edx, ecx
    // 005B6454  mov dword ptr [ebp - 0x2c], edx
    var_2c = (edx);
    // 005B6457  jle 0x5b6963
    if (edx <= ecx) goto LAB_005B6963;
LAB_005B645D:
    // 005B645D  mov ebx, dword ptr [ebp - 0x18]
    ebx = (var_18);
    // 005B6460  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
    // 005B6463  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B6469  mov edx, dword ptr [0x949870]
    edx = ((*(int*)((char*)g_00949870)));
    // 005B646F  mov eax, dword ptr [ebx*4 + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ebx*4)));
    // 005B6476  add eax, esi
    eax = ((eax) + (esi));
    // 005B6478  test cl, 1
    // 005B647B  jne 0x5b648c
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B648C;
    // 005B647D  test eax, eax
    // 005B647F  jge 0x5b6486
    if (((eax) & (eax)) >= 0) goto LAB_005B6486;
    // 005B6481  lea edi, [edx + eax]
    edi = (eax);
    // 005B6484  jmp 0x5b648e
    goto LAB_005B648E;
LAB_005B6486:
    // 005B6486  cmp eax, edx
    // 005B6488  jl 0x5b648c
    if (eax < edx) goto LAB_005B648C;
    // 005B648A  sub eax, edx
    eax = ((eax) - (edx));
LAB_005B648C:
    // 005B648C  mov edi, eax
    edi = (eax);
LAB_005B648E:
    // 005B648E  mov esi, dword ptr [ebx*4 + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + ebx*4)));
    // 005B6495  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005B6498  add esi, eax
    esi = ((esi) + (eax));
    // 005B649A  mov dword ptr [ebp - 0xc], edi
    var_c = (edi);
    // 005B649D  mov dword ptr [ebp - 0x14], esi
    var_14 = (esi);
    // 005B64A0  js 0x5b6951
    if (((esi) - (0x0)) < 0) goto LAB_005B6951;
    // 005B64A6  cmp esi, dword ptr [0x949874]
    // 005B64AC  jge 0x5b6951
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B6951;
    // 005B64B2  test edi, edi
    // 005B64B4  jl 0x5b6951
    if (((edi) & (edi)) < 0) goto LAB_005B6951;
    // 005B64BA  cmp edi, edx
    // 005B64BC  jge 0x5b6951
    if (edi >= edx) goto LAB_005B6951;
    // 005B64C2  mov ecx, dword ptr [ebp - 0x40]
    ecx = (var_40);
    // 005B64C5..005B64C7  neg/sbb -> (ecx!=0)?-1:0
    ecx = ((ecx) != 0 ? -1 : 0);
    // 005B64C9  and ecx, 0x10
    ecx = ((ecx) & (0x10));
    // 005B64CC  add ecx, 9
    ecx = ((ecx) + (9));
    // 005B64CF  cmp ebx, ecx
    // 005B64D1  jl 0x5b68a6
    if (ebx < ecx) goto LAB_005B68A6;
    // 005B64D7  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B64DC  imul eax, esi
    eax = ((eax) * (esi));
    // 005B64DF  mov esi, edi
    esi = (edi);
    // 005B64E1  sar esi, 1
    esi = ((esi) >> (1));
    // 005B64E3  add eax, esi
    eax = ((eax) + (esi));
    // 005B64E5  lea esi, [eax + eax*4]
    esi = (eax + eax*4);
    // 005B64E8  lea eax, [eax + esi*2]
    eax = (eax + esi*2);
    // 005B64EB  mov esi, dword ptr [0x94a30c]
    esi = ((*(int*)((char*)g_0094a30c)));
    // 005B64F1  test byte ptr [esi + eax*4 + 0xe], 0x20
    // 005B64F6  jne 0x5b6951
    if (((((*(unsigned char*)((char*)(esi) + eax*4 + 0xe))) & (0x20))) != 0) goto LAB_005B6951;
    // 005B64FC  cmp ebx, ecx
    // 005B64FE  jl 0x5b68a6
    if (ebx < ecx) goto LAB_005B68A6;
    // 005B6504  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
    // 005B6507  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B6509  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 005B650C  mov dword ptr [ebp - 0x48], eax
    var_48 = (eax);
LAB_005B650F:
    // 005B650F  mov ecx, dword ptr [ebp - 0x48]
    ecx = (var_48);
    // 005B6512  mov eax, dword ptr [ecx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ecx)));
    // 005B6518  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B651E  add eax, edi
    eax = ((eax) + (edi));
    // 005B6520  test cl, 1
    // 005B6523  jne 0x5b6534
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B6534;
    // 005B6525  test eax, eax
    // 005B6527  jge 0x5b652e
    if (((eax) & (eax)) >= 0) goto LAB_005B652E;
    // 005B6529  lea edi, [edx + eax]
    edi = (eax);
    // 005B652C  jmp 0x5b6536
    goto LAB_005B6536;
LAB_005B652E:
    // 005B652E  cmp eax, edx
    // 005B6530  jl 0x5b6534
    if (eax < edx) goto LAB_005B6534;
    // 005B6532  sub eax, edx
    eax = ((eax) - (edx));
LAB_005B6534:
    // 005B6534  mov edi, eax
    edi = (eax);
LAB_005B6536:
    // 005B6536  mov edx, dword ptr [ebp - 0x48]
    edx = (var_48);
    // 005B6539  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005B653C  mov ebx, dword ptr [edx + 0x66f440]
    ebx = ((*(int*)((char*)g_0066f440 + edx)));
    // 005B6542  add ebx, eax
    ebx = ((ebx) + (eax));
    // 005B6544  js 0x5b688a
    if (((ebx) - (0x0)) < 0) goto LAB_005B688A;
    // 005B654A  cmp ebx, dword ptr [0x949874]
    // 005B6550  jge 0x5b688a
    if (ebx >= (*(int*)((char*)g_00949874))) goto LAB_005B688A;
    // 005B6556  test edi, edi
    // 005B6558  jl 0x5b688a
    if (((edi) & (edi)) < 0) goto LAB_005B688A;
    // 005B655E  cmp edi, dword ptr [0x949870]
    // 005B6564  jge 0x5b688a
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B688A;
    // 005B656A  mov eax, edx
    eax = (edx);
    // 005B656C  cmp eax, 0x24
    // 005B656F  jge 0x5b6607
    if (eax >= 0x24) goto LAB_005B6607;
    // 005B6575  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B657A  mov ecx, edi
    ecx = (edi);
    // 005B657C  imul eax, ebx
    eax = ((eax) * (ebx));
    // 005B657F  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B6581  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6583  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B6589  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B658C  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B658F  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B6592  test byte ptr [eax + 8], 3
    // 005B6596  je 0x5b65a3
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (3))) == 0) goto LAB_005B65A3;
    // 005B6598  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B659B  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B659E  cmp eax, 8
    // 005B65A1  jl 0x5b65a6
    if (eax < 8) goto LAB_005B65A6;
LAB_005B65A3:
    // 005B65A3  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
LAB_005B65A6:
    // 005B65A6  cmp eax, esi
    // 005B65A8  je 0x5b68a6
    if ((eax) == (esi)) goto LAB_005B68A6;
    // 005B65AE  mov edx, esi
    edx = (esi);
    // 005B65B0  add eax, esi
    eax = ((eax) + (esi));
    // 005B65B2  shl edx, 6
    edx = ((edx) << (6));
    // 005B65B5  add edx, esi
    edx = ((edx) + (esi));
    // 005B65B7  lea ecx, [esi + edx*2]
    ecx = (esi + edx*2);
    // 005B65BA  lea edx, [esi + ecx*8]
    edx = (esi + ecx*8);
    // 005B65BD  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B65C0  test byte ptr [eax*4 + 0x96c9f8], 1
    // 005B65C8  je 0x5b6607
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (1))) == 0) goto LAB_005B6607;
    // 005B65CA  push ebx
    // 005B65CB  push edi
    // 005B65CC  call 0x5bfe90
    eax = veh_at(edi, ebx);
    // 005B65D1  add esp, 8
    // 005B65D4  test eax, eax
    // 005B65D6  jl 0x5b65fc
    if (((eax) & (eax)) < 0) goto LAB_005B65FC;
LAB_005B65D8:
    // 005B65D8  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B65DB  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B65DD  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B65E0  shl eax, 2
    eax = ((eax) << (2));
    // 005B65E3  mov dl, byte ptr [eax + 0x952836]
    SETB0(edx, ((*(unsigned char*)((char*)g_00952836 + eax))));
    // 005B65E9  cmp edx, esi
    // 005B65EB  je 0x5b68a6
    if ((edx) == (esi)) goto LAB_005B68A6;
    // 005B65F1  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B65F8  test eax, eax
    // 005B65FA  jge 0x5b65d8
    if (((eax) & (eax)) >= 0) goto LAB_005B65D8;
LAB_005B65FC:
    // 005B65FC  mov eax, dword ptr [ebp - 0x30]
    eax = (var_30);
    // 005B65FF  test eax, eax
    // 005B6601  jne 0x5b68ad
    if ((((eax) & (eax))) != 0) goto LAB_005B68AD;
LAB_005B6607:
    // 005B6607  push ebx
    // 005B6608  push edi
    // 005B6609  call 0x5bf010
    eax = is_sensor(edi, ebx);
    // 005B660E  add esp, 8
    // 005B6611  test eax, eax
    // 005B6613  je 0x5b672a
    if ((((eax) & (eax))) == 0) goto LAB_005B672A;
    // 005B6619  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B661E  mov ecx, edi
    ecx = (edi);
    // 005B6620  imul eax, ebx
    eax = ((eax) * (ebx));
    // 005B6623  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B6625  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6627  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B662D  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B6630  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B6633  movsx edx, byte ptr [ecx + eax*4 + 7]
    edx = ((*(signed char*)((char*)(ecx) + eax*4 + 0x7)));
    // 005B6638  cmp edx, esi
    // 005B663A  je 0x5b68a6
    if ((edx) == (esi)) goto LAB_005B68A6;
    // 005B6640  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005B6643  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 005B6646  push eax
    // 005B6647  push ecx
    // 005B6648  call 0x4e3b80
    eax = base_find(ecx, eax);
    // 005B664D  lea edx, [eax + 1]
    edx = (eax + 0x1);
    // 005B6650  add esp, 8
    // 005B6653  test edx, edx
    // 005B6655  je 0x5b672a
    if ((((edx) & (edx))) == 0) goto LAB_005B672A;
    // 005B665B  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 005B665E  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6661  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 005B6664  lea esi, [eax + edx*4]
    esi = (eax + edx*4);
    // 005B6667  shl esi, 2
    esi = ((esi) << (2));
    // 005B666A  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 005B6671  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 005B6673  push ecx
    // 005B6674  call 0x644f3a
    eax = abs(ecx);
    // 005B6679  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B667F  add esp, 4
    // 005B6682  test cl, 1
    // 005B6685  jne 0x5b6699
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B6699;
    // 005B6687  cmp eax, dword ptr [0x68faf0]
    // 005B668D  jle 0x5b6699
    if (eax <= (*(int*)((char*)g_0068faf0))) goto LAB_005B6699;
    // 005B668F  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B6695  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 005B6697  jmp 0x5b669b
    goto LAB_005B669B;
LAB_005B6699:
    // 005B6699  mov ecx, eax
    ecx = (eax);
LAB_005B669B:
    // 005B669B  test ecx, ecx
    // 005B669D  je 0x5b66d8
    if ((((ecx) & (ecx))) == 0) goto LAB_005B66D8;
    // 005B669F  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 005B66A6  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 005B66A9  sub edx, eax
    edx = ((edx) - (eax));
    // 005B66AB  push edx
    // 005B66AC  call 0x644f3a
    eax = abs(edx);
    // 005B66B1  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B66B7  add esp, 4
    // 005B66BA  test cl, 1
    // 005B66BD  jne 0x5b66d1
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B66D1;
    // 005B66BF  cmp eax, dword ptr [0x68faf0]
    // 005B66C5  jle 0x5b66d1
    if (eax <= (*(int*)((char*)g_0068faf0))) goto LAB_005B66D1;
    // 005B66C7  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B66CD  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 005B66CF  jmp 0x5b66d3
    goto LAB_005B66D3;
LAB_005B66D1:
    // 005B66D1  mov ecx, eax
    ecx = (eax);
LAB_005B66D3:
    // 005B66D3  cmp ecx, 2
    // 005B66D6  jne 0x5b6727
    if ((ecx) != (2)) goto LAB_005B6727;
LAB_005B66D8:
    // 005B66D8  movsx eax, word ptr [esi + 0x97d042]
    eax = ((*(short*)((char*)g_0097d042 + esi)));
    // 005B66DF  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 005B66E2  mov ecx, edx
    ecx = (edx);
    // 005B66E4  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 005B66E6  je 0x5b66f7
    if ((ecx) == (0x0)) goto LAB_005B66F7;
    // 005B66E8  test ecx, ecx
    // 005B66EA  jl 0x5b66f0
    if (((ecx) & (ecx)) < 0) goto LAB_005B66F0;
    // 005B66EC  mov eax, ecx
    eax = (ecx);
    // 005B66EE  jmp 0x5b66f2
    goto LAB_005B66F2;
LAB_005B66F0:
    // 005B66F0  sub eax, edx
    eax = ((eax) - (edx));
LAB_005B66F2:
    // 005B66F2  cmp eax, 2
    // 005B66F5  jne 0x5b6727
    if ((eax) != (2)) goto LAB_005B6727;
LAB_005B66F7:
    // 005B66F7  lea eax, [ebp - 0x10]
    eax = ((int)&var_10);
    // 005B66FA  lea ecx, [ebp - 0x50]
    ecx = ((int)&var_50);
    // 005B66FD  push eax
    // 005B66FE  push ecx
    // 005B66FF  push 0x29
    // 005B6701  call 0x50ba00
    bitmask(0x29, (int*)(ecx), (int*)(eax));
    // 005B6706  mov edx, dword ptr [ebp - 0x50]
    edx = (var_50);
    // 005B6709  mov cl, byte ptr [ebp - 0x10]
    SETB0(ecx, ((var_10 & 0xff)));
    // 005B670C  add esp, 0xc
    // 005B670F  mov al, byte ptr [esi + edx + 0x97d0cc]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d0cc + edx))));
    // 005B6716  and al, cl
    SETB0(eax, (((eax & 0xff)) & ((ecx & 0xff))));
    // 005B6718..005B671C  neg8/sbb/neg -> (al!=0)?1:0
    eax = ((eax & 0xff)) != 0 ? 1 : 0;
    // 005B671E  mov dword ptr [ebp - 0x64], eax
    var_64 = (eax);
    // 005B6721  jne 0x5b68a6
    if ((eax) != (0x0)) goto LAB_005B68A6;
LAB_005B6727:
    // 005B6727  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
LAB_005B672A:
    // 005B672A  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B672F  mov ecx, edi
    ecx = (edi);
    // 005B6731  imul eax, ebx
    eax = ((eax) * (ebx));
    // 005B6734  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B6736  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6738  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B673E  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B6741  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B6744  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B6747  test byte ptr [eax + 8], 1
    // 005B674B  je 0x5b6758
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (1))) == 0) goto LAB_005B6758;
    // 005B674D  mov cl, byte ptr [eax + 2]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B6750  and ecx, 0xf
    ecx = ((ecx) & (0xf));
    // 005B6753  cmp ecx, 8
    // 005B6756  jl 0x5b675b
    if (ecx < 8) goto LAB_005B675B;
LAB_005B6758:
    // 005B6758  or ecx, 0xffffffff
    ecx = ((ecx) | (0xffffffff));
LAB_005B675B:
    // 005B675B  cmp ecx, esi
    // 005B675D  je 0x5b68a6
    if ((ecx) == (esi)) goto LAB_005B68A6;
    // 005B6763  test byte ptr [eax + 8], 2
    // 005B6767  je 0x5b6774
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (2))) == 0) goto LAB_005B6774;
    // 005B6769  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B676C  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B676F  cmp eax, 8
    // 005B6772  jl 0x5b6777
    if (eax < 8) goto LAB_005B6777;
LAB_005B6774:
    // 005B6774  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
LAB_005B6777:
    // 005B6777  cmp eax, esi
    // 005B6779  je 0x5b679b
    if ((eax) == (esi)) goto LAB_005B679B;
    // 005B677B  mov edx, eax
    edx = (eax);
    // 005B677D  shl edx, 6
    edx = ((edx) << (6));
    // 005B6780  add edx, eax
    edx = ((edx) + (eax));
    // 005B6782  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 005B6785  lea edx, [eax + ecx*8]
    edx = (eax + ecx*8);
    // 005B6788  add eax, esi
    eax = ((eax) + (esi));
    // 005B678A  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 005B678D  test byte ptr [ecx*4 + 0x96c9f8], 1
    // 005B6795  je 0x5b688a
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (1))) == 0) goto LAB_005B688A;
LAB_005B679B:
    // 005B679B  push ebx
    // 005B679C  push edi
    // 005B679D  call 0x5bfe90
    eax = veh_at(edi, ebx);
    // 005B67A2  add esp, 8
    // 005B67A5  test eax, eax
    // 005B67A7  jl 0x5b688a
    if (((eax) & (eax)) < 0) goto LAB_005B688A;
    // 005B67AD  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B67B0  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B67B3  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B67BB  test cx, cx
    // 005B67BE  jl 0x5b67d6
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B67D6;
LAB_005B67C0:
    // 005B67C0  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B67C3  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B67C6  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B67C9  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B67D1  test cx, cx
    // 005B67D4  jge 0x5b67c0
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B67C0;
LAB_005B67D6:
    // 005B67D6  test eax, eax
    // 005B67D8  jl 0x5b688a
    if (((eax) & (eax)) < 0) goto LAB_005B688A;
    // 005B67DE  mov edi, dword ptr [ebp - 0x14]
    edi = (var_14);
LAB_005B67E1:
    // 005B67E1  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B67E4  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B67E7  lea esi, [eax + edx*4]
    esi = (eax + edx*4);
    // 005B67EA  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B67EC  shl esi, 2
    esi = ((esi) << (2));
    // 005B67EF  mov al, byte ptr [esi + 0x952836]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952836 + esi))));
    // 005B67F5  cmp eax, ecx
    // 005B67F7  jne 0x5b686f
    if ((eax) != (ecx)) goto LAB_005B686F;
    // 005B67F9  movsx ecx, word ptr [esi + 0x952832]
    ecx = ((*(short*)((char*)g_00952832 + esi)));
    // 005B6800  push 2
    // 005B6802  push ecx
    // 005B6803  call 0x5bf1f0
    eax = has_abil(ecx, 2);
    // 005B6808  add esp, 8
    // 005B680B  test eax, eax
    // 005B680D  je 0x5b686f
    if ((((eax) & (eax))) == 0) goto LAB_005B686F;
    // 005B680F  movsx eax, word ptr [esi + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + esi)));
    // 005B6816  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6818  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B681B  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B681E  mov cl, byte ptr [eax*4 + 0x9ab88c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009ab88c + eax*4))));
    // 005B6825  mov eax, ecx
    eax = (ecx);
    // 005B6827  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 005B682A  shl edx, 4
    edx = ((edx) << (4));
    // 005B682D  mov cl, byte ptr [edx + 0x94a379]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094a379 + edx))));
    // 005B6833  cmp cl, 2
    // 005B6836  je 0x5b6880
    if (((ecx & 0xff)) == (2)) goto LAB_005B6880;
    // 005B6838  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B683D  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005B6840  imul eax, edi
    eax = ((eax) * (edi));
    // 005B6843  sar edx, 1
    edx = ((edx) >> (1));
    // 005B6845  mov ebx, dword ptr [0x94a30c]
    ebx = ((*(int*)((char*)g_0094a30c)));
    // 005B684B  add eax, edx
    eax = ((eax) + (edx));
    // 005B684D  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B6850  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B6853  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B6855  mov dl, byte ptr [ebx + eax*4]
    SETB0(edx, ((*(unsigned char*)((char*)(ebx) + eax*4))));
    // 005B6858  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B685A  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 005B685D  cmp edx, 0x60
    // 005B6860  setl al
    SETB0(eax, ((edx < 0x60) ? 1 : 0));
    // 005B6863  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B6865  cmp cl, 1
    // 005B6868  sete dl
    SETB0(edx, ((((ecx & 0xff)) == (1)) ? 1 : 0));
    // 005B686B  cmp eax, edx
    // 005B686D  je 0x5b6880
    if ((eax) == (edx)) goto LAB_005B6880;
LAB_005B686F:
    // 005B686F  movsx eax, word ptr [esi + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + esi)));
    // 005B6876  test eax, eax
    // 005B6878  jge 0x5b67e1
    if (((eax) & (eax)) >= 0) goto LAB_005B67E1;
    // 005B687E  jmp 0x5b6887
    goto LAB_005B6887;
LAB_005B6880:
    // 005B6880  mov dword ptr [ebp - 0x30], 1
    var_30 = (1);
LAB_005B6887:
    // 005B6887  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
LAB_005B688A:
    // 005B688A  mov eax, dword ptr [ebp - 0x48]
    eax = (var_48);
    // 005B688D  add eax, 4
    eax = ((eax) + (4));
    // 005B6890  cmp eax, 0x64
    // 005B6893  mov dword ptr [ebp - 0x48], eax
    var_48 = (eax);
    // 005B6896  jge 0x5b68ad
    if (eax >= 0x64) goto LAB_005B68AD;
    // 005B6898  mov edx, dword ptr [0x949870]
    edx = ((*(int*)((char*)g_00949870)));
    // 005B689E  mov edi, dword ptr [ebp - 0xc]
    edi = (var_c);
    // 005B68A1  jmp 0x5b650f
    goto LAB_005B650F;
LAB_005B68A6:
    // 005B68A6  mov dword ptr [ebp - 0x30], 1
    var_30 = (1);
LAB_005B68AD:
    // 005B68AD  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B68B2  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 005B68B5  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B68B9  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B68BB  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B68BD  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B68C0  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B68C3  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B68C8  mov ecx, dword ptr [eax + edx*4 + 0xc]
    ecx = ((*(int*)((char*)(eax) + edx*4 + 0xc)));
    // 005B68CC  mov eax, dword ptr [ebp - 0x30]
    eax = (var_30);
    // 005B68CF  test ecx, 0x200000
    // 005B68D5  je 0x5b690d
    if ((((ecx) & (0x200000))) == 0) goto LAB_005B690D;
    // 005B68D7  test eax, eax
    // 005B68D9  je 0x5b6951
    if ((((eax) & (eax))) == 0) goto LAB_005B6951;
    // 005B68DB  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B68DE  cmp eax, 9
    // 005B68E1  jge 0x5b68ea
    if (eax >= 9) goto LAB_005B68EA;
    // 005B68E3  mov eax, 1
    eax = (1);
    // 005B68E8  jmp 0x5b68f7
    goto LAB_005B68F7;
LAB_005B68EA:
    // 005B68EA  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B68EC  cmp eax, 0x19
    // 005B68EF  setge cl
    SETB0(ecx, ((eax >= 0x19) ? 1 : 0));
    // 005B68F2  add ecx, 2
    ecx = ((ecx) + (2));
    // 005B68F5  mov eax, ecx
    eax = (ecx);
LAB_005B68F7:
    // 005B68F7  cmp dword ptr [ebp - 0x1c], eax
    // 005B68FA  jg 0x5b6951
    if (var_1c > eax) goto LAB_005B6951;
    // 005B68FC  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B68FF  cmp eax, 9
    // 005B6902  jge 0x5b6943
    if (eax >= 9) goto LAB_005B6943;
    // 005B6904  mov dword ptr [ebp - 0x1c], 1
    var_1c = (1);
    // 005B690B  jmp 0x5b6951
    goto LAB_005B6951;
LAB_005B690D:
    // 005B690D  test eax, eax
    // 005B690F  jne 0x5b6951
    if ((((eax) & (eax))) != 0) goto LAB_005B6951;
    // 005B6911  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B6914  cmp eax, 9
    // 005B6917  jge 0x5b6920
    if (eax >= 9) goto LAB_005B6920;
    // 005B6919  mov eax, 1
    eax = (1);
    // 005B691E  jmp 0x5b692d
    goto LAB_005B692D;
LAB_005B6920:
    // 005B6920  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6922  cmp eax, 0x19
    // 005B6925  setge cl
    SETB0(ecx, ((eax >= 0x19) ? 1 : 0));
    // 005B6928  add ecx, 2
    ecx = ((ecx) + (2));
    // 005B692B  mov eax, ecx
    eax = (ecx);
LAB_005B692D:
    // 005B692D  cmp dword ptr [ebp - 0x1c], eax
    // 005B6930  jg 0x5b6951
    if (var_1c > eax) goto LAB_005B6951;
    // 005B6932  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B6935  cmp eax, 9
    // 005B6938  jge 0x5b6943
    if (eax >= 9) goto LAB_005B6943;
    // 005B693A  mov dword ptr [ebp - 0x1c], 1
    var_1c = (1);
    // 005B6941  jmp 0x5b6951
    goto LAB_005B6951;
LAB_005B6943:
    // 005B6943  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B6945  cmp eax, 0x19
    // 005B6948  setge dl
    SETB0(edx, ((eax >= 0x19) ? 1 : 0));
    // 005B694B  add edx, 2
    edx = ((edx) + (2));
    // 005B694E  mov dword ptr [ebp - 0x1c], edx
    var_1c = (edx);
LAB_005B6951:
    // 005B6951  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B6954  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 005B6957  inc eax
    eax = ((eax) + 1);
    // 005B6958  cmp eax, ecx
    // 005B695A  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005B695D  jl 0x5b645d
    if (eax < ecx) goto LAB_005B645D;
LAB_005B6963:
    // 005B6963  mov edi, dword ptr [ebp - 0x40]
    edi = (var_40);
    // 005B6966  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B6968..005B696A  neg/sbb -> (edi!=0)?-1:0
    edi = ((edi) != 0 ? -1 : 0);
    // 005B696C  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005B696F  and edi, 0x10
    edi = ((edi) & (0x10));
    // 005B6972  add edi, 9
    edi = ((edi) + (9));
    // 005B6975  test edi, edi
    // 005B6977  jle 0x5b6aed
    if (((edi) & (edi)) <= 0) goto LAB_005B6AED;
    // 005B697D  mov ebx, dword ptr [0x949870]
    ebx = ((*(int*)((char*)g_00949870)));
LAB_005B6983:
    // 005B6983  mov ecx, dword ptr [eax*4 + 0x66efbc]
    ecx = ((*(int*)((char*)g_0066efbc + eax*4)));
    // 005B698A  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
    // 005B698D  mov dl, byte ptr [0x94988c]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B6993  add ecx, esi
    ecx = ((ecx) + (esi));
    // 005B6995  test dl, 1
    // 005B6998  jne 0x5b69a8
    if (((((edx & 0xff)) & (1))) != 0) goto LAB_005B69A8;
    // 005B699A  test ecx, ecx
    // 005B699C  jge 0x5b69a2
    if (((ecx) & (ecx)) >= 0) goto LAB_005B69A2;
    // 005B699E  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 005B69A0  jmp 0x5b69a8
    goto LAB_005B69A8;
LAB_005B69A2:
    // 005B69A2  cmp ecx, ebx
    // 005B69A4  jl 0x5b69a8
    if (ecx < ebx) goto LAB_005B69A8;
    // 005B69A6  sub ecx, ebx
    ecx = ((ecx) - (ebx));
LAB_005B69A8:
    // 005B69A8  mov edx, dword ptr [eax*4 + 0x66f440]
    edx = ((*(int*)((char*)g_0066f440 + eax*4)));
    // 005B69AF  mov esi, dword ptr [ebp - 0x24]
    esi = (var_24);
    // 005B69B2  add edx, esi
    edx = ((edx) + (esi));
    // 005B69B4  js 0x5b6ae1
    if (((edx) - (0x0)) < 0) goto LAB_005B6AE1;
    // 005B69BA  cmp edx, dword ptr [0x949874]
    // 005B69C0  jge 0x5b6ae1
    if (edx >= (*(int*)((char*)g_00949874))) goto LAB_005B6AE1;
    // 005B69C6  test ecx, ecx
    // 005B69C8  jl 0x5b6ae1
    if (((ecx) & (ecx)) < 0) goto LAB_005B6AE1;
    // 005B69CE  cmp ecx, ebx
    // 005B69D0  jge 0x5b6ae1
    if (ecx >= ebx) goto LAB_005B6AE1;
    // 005B69D6  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B69DB  imul eax, edx
    eax = ((eax) * (edx));
    // 005B69DE  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B69E0  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B69E2  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B69E5  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B69E8  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B69ED  mov ecx, dword ptr [eax + edx*4 + 0xc]
    ecx = ((*(int*)((char*)(eax) + edx*4 + 0xc)));
    // 005B69F1  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B69F4  mov edx, ecx
    edx = (ecx);
    // 005B69F6  and ecx, 0x80002000
    ecx = ((ecx) & (0x80002000));
    // 005B69FC  and edx, 0x80001000
    edx = ((edx) & (0x80001000));
    // 005B6A02  cmp ecx, 0x2000
    // 005B6A08  jne 0x5b6a0d
    if ((ecx) != (0x2000)) goto LAB_005B6A0D;
    // 005B6A0A  inc dword ptr [ebp - 0x58]
    var_58 = ((var_58) + 1);
LAB_005B6A0D:
    // 005B6A0D  cmp dword ptr [ebp - 0x18], 9
    // 005B6A11  jl 0x5b6a3a
    if (var_18 < 9) goto LAB_005B6A3A;
    // 005B6A13  mov esi, dword ptr [ebp - 0x3c]
    esi = (var_3c);
    // 005B6A16  cmp esi, 2
    // 005B6A19  je 0x5b6a3a
    if ((esi) == (2)) goto LAB_005B6A3A;
    // 005B6A1B  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6A1D  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B6A1F  mov cl, byte ptr [eax]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax)))));
    // 005B6A21  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B6A24  cmp ecx, 0x60
    // 005B6A27  setl dl
    SETB0(edx, ((ecx < 0x60) ? 1 : 0));
    // 005B6A2A  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6A2C  cmp esi, 1
    // 005B6A2F  sete cl
    SETB0(ecx, (((esi) == (1)) ? 1 : 0));
    // 005B6A32  cmp edx, ecx
    // 005B6A34  jne 0x5b6ade
    if ((edx) != (ecx)) goto LAB_005B6ADE;
LAB_005B6A3A:
    // 005B6A3A  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B6A3D  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005B6A43  cmp edx, ecx
    // 005B6A45  jne 0x5b6ac2
    if ((edx) != (ecx)) goto LAB_005B6AC2;
    // 005B6A47  test byte ptr [0x9a64c0], 0x80
    // 005B6A4E  jne 0x5b6ac2
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B6AC2;
    // 005B6A50  mov ecx, edx
    ecx = (edx);
    // 005B6A52  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B6A55  add ecx, edx
    ecx = ((ecx) + (edx));
    // 005B6A57  lea ecx, [edx + ecx*2]
    ecx = (edx + ecx*2);
    // 005B6A5A  lea ecx, [edx + ecx*8]
    ecx = (edx + ecx*8);
    // 005B6A5D  lea ecx, [edx + ecx*2]
    ecx = (edx + ecx*2);
    // 005B6A60  test dword ptr [ecx*4 + 0x96c9e0], 0x200
    // 005B6A6B  jne 0x5b6ac2
    if (((((*(int*)((char*)g_0096c9e0 + ecx*4))) & (0x200))) != 0) goto LAB_005B6AC2;
    // 005B6A6D  mov bl, byte ptr [ebp - 0x28]
    SETB0(ebx, ((var_28 & 0xff)));
    // 005B6A70  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B6A73  test bl, cl
    // 005B6A75  jne 0x5b6a92
    if (((((ebx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005B6A92;
    // 005B6A77  mov esi, dword ptr [eax + 0xc]
    esi = ((*(int*)((char*)(eax) + 0xc)));
    // 005B6A7A  or esi, 0x400000
    esi = ((esi) | (0x400000));
    // 005B6A80  mov dword ptr [eax + 0xc], esi
    (*(int*)((char*)(eax) + 0xc)) = (esi);
    // 005B6A83  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B6A89  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B6A8C  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B6A92:
    // 005B6A92  mov ecx, dword ptr [eax + edx*4 + 0xc]
    ecx = ((*(int*)((char*)(eax) + edx*4 + 0xc)));
    // 005B6A96  mov esi, dword ptr [eax + 8]
    esi = ((*(int*)((char*)(eax) + 0x8)));
    // 005B6A99  xor ecx, esi
    ecx = ((ecx) ^ (esi));
    // 005B6A9B  test ecx, 0xfffffffc
    // 005B6AA1  jne 0x5b6aa8
    if ((((ecx) & (0xfffffffc))) != 0) goto LAB_005B6AA8;
    // 005B6AA3  test byte ptr [eax + 4], bl
    // 005B6AA6  jne 0x5b6ac5
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((ebx & 0xff)))) != 0) goto LAB_005B6AC5;
LAB_005B6AA8:
    // 005B6AA8  mov esi, dword ptr [ebp - 0x18]
    esi = (var_18);
    // 005B6AAB  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B6AAD  cmp esi, 9
    // 005B6AB0  mov esi, dword ptr [ebp - 0x1c]
    esi = (var_1c);
    // 005B6AB3  setge cl
    SETB0(ecx, ((esi >= 9) ? 1 : 0));
    // 005B6AB6  add ecx, 2
    ecx = ((ecx) + (2));
    // 005B6AB9  cmp esi, ecx
    // 005B6ABB  jg 0x5b6ac5
    if (esi > ecx) goto LAB_005B6AC5;
    // 005B6ABD  mov dword ptr [ebp - 0x1c], ecx
    var_1c = (ecx);
    // 005B6AC0  jmp 0x5b6ac5
    goto LAB_005B6AC5;
LAB_005B6AC2:
    // 005B6AC2  mov bl, byte ptr [ebp - 0x28]
    SETB0(ebx, ((var_28 & 0xff)));
LAB_005B6AC5:
    // 005B6AC5  test edx, edx
    // 005B6AC7  je 0x5b6ad0
    if ((((edx) & (edx))) == 0) goto LAB_005B6AD0;
    // 005B6AC9  mov ecx, dword ptr [eax + 8]
    ecx = ((*(int*)((char*)(eax) + 0x8)));
    // 005B6ACC  mov dword ptr [eax + edx*4 + 0xc], ecx
    (*(int*)((char*)(eax) + edx*4 + 0xc)) = (ecx);
LAB_005B6AD0:
    // 005B6AD0  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B6AD3  or cl, bl
    SETB0(ecx, (((ecx & 0xff)) | ((ebx & 0xff))));
    // 005B6AD5  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B6AD8  mov ebx, dword ptr [0x949870]
    ebx = ((*(int*)((char*)g_00949870)));
LAB_005B6ADE:
    // 005B6ADE  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
LAB_005B6AE1:
    // 005B6AE1  inc eax
    eax = ((eax) + 1);
    // 005B6AE2  cmp eax, edi
    // 005B6AE4  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005B6AE7  jl 0x5b6983
    if (eax < edi) goto LAB_005B6983;
LAB_005B6AED:
    // 005B6AED  mov eax, dword ptr [ebp - 0x40]
    eax = (var_40);
    // 005B6AF0  mov ecx, edi
    ecx = (edi);
    // 005B6AF2..005B6AF4  neg/sbb -> (eax!=0)?-1:0
    eax = ((eax) != 0 ? -1 : 0);
    // 005B6AF6  mov dword ptr [ebp - 0x18], ecx
    var_18 = (ecx);
    // 005B6AF9  and eax, 0x18
    eax = ((eax) & (0x18));
    // 005B6AFC  add eax, 0x19
    eax = ((eax) + (0x19));
    // 005B6AFF  cmp ecx, eax
    // 005B6B01  mov dword ptr [ebp - 0x2c], eax
    var_2c = (eax);
    // 005B6B04  jge 0x5b6ded
    if (ecx >= eax) goto LAB_005B6DED;
LAB_005B6B0A:
    // 005B6B0A  mov esi, dword ptr [ebp - 0x18]
    esi = (var_18);
    // 005B6B0D  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 005B6B10  mov edx, dword ptr [0x94988c]
    edx = ((*(int*)((char*)g_0094988c)));
    // 005B6B16  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B6B1C  mov eax, dword ptr [esi*4 + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + esi*4)));
    // 005B6B23  add eax, ebx
    eax = ((eax) + (ebx));
    // 005B6B25  and edx, 1
    edx = ((edx) & (1));
    // 005B6B28  mov dword ptr [ebp - 0x30], edx
    var_30 = (edx);
    // 005B6B2B  jne 0x5b6b3e
    if ((edx) != (0x0)) goto LAB_005B6B3E;
    // 005B6B2D  test eax, eax
    // 005B6B2F  jge 0x5b6b38
    if (((eax) & (eax)) >= 0) goto LAB_005B6B38;
    // 005B6B31  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B6B33  mov dword ptr [ebp - 0xc], ecx
    var_c = (ecx);
    // 005B6B36  jmp 0x5b6b41
    goto LAB_005B6B41;
LAB_005B6B38:
    // 005B6B38  cmp eax, ecx
    // 005B6B3A  jl 0x5b6b3e
    if (eax < ecx) goto LAB_005B6B3E;
    // 005B6B3C  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_005B6B3E:
    // 005B6B3E  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
LAB_005B6B41:
    // 005B6B41  mov eax, dword ptr [esi*4 + 0x66f440]
    eax = ((*(int*)((char*)g_0066f440 + esi*4)));
    // 005B6B48  mov ecx, dword ptr [ebp - 0x24]
    ecx = (var_24);
    // 005B6B4B  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6B4D  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 005B6B50  js 0x5b6ddb
    if (((eax) - (0x0)) < 0) goto LAB_005B6DDB;
    // 005B6B56  mov edx, eax
    edx = (eax);
    // 005B6B58  mov eax, dword ptr [0x949874]
    eax = ((*(int*)((char*)g_00949874)));
    // 005B6B5D  cmp edx, eax
    // 005B6B5F  jge 0x5b6ddb
    if (edx >= eax) goto LAB_005B6DDB;
    // 005B6B65  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B6B68  test eax, eax
    // 005B6B6A  jl 0x5b6ddb
    if (((eax) & (eax)) < 0) goto LAB_005B6DDB;
    // 005B6B70  cmp eax, dword ptr [0x949870]
    // 005B6B76  jge 0x5b6ddb
    if (eax >= (*(int*)((char*)g_00949870))) goto LAB_005B6DDB;
    // 005B6B7C  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B6B81  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 005B6B84  imul eax, edx
    eax = ((eax) * (edx));
    // 005B6B87  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B6B89  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6B8B  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B6B91  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B6B94  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B6B97  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B6B9A  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B6B9D  mov dword ptr [ebp - 0x50], eax
    var_50 = (eax);
    // 005B6BA0  test byte ptr [eax + 4], dl
    // 005B6BA3  jne 0x5b6ddb
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B6DDB;
    // 005B6BA9  xor edi, edi
    edi = ((edi) ^ (edi));
    // 005B6BAB  mov dword ptr [ebp - 0x48], edi
    var_48 = (edi);
LAB_005B6BAE:
    // 005B6BAE  mov ebx, dword ptr [edi + 0x66ef50]
    ebx = ((*(int*)((char*)g_0066ef50 + edi)));
    // 005B6BB4  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B6BB7  lea ecx, [ebx + eax]
    ecx = (eax);
    // 005B6BBA  mov eax, dword ptr [ebp - 0x30]
    eax = (var_30);
    // 005B6BBD  test eax, eax
    // 005B6BBF  jne 0x5b6bda
    if ((((eax) & (eax))) != 0) goto LAB_005B6BDA;
    // 005B6BC1  test ecx, ecx
    // 005B6BC3  jge 0x5b6bcf
    if (((ecx) & (ecx)) >= 0) goto LAB_005B6BCF;
    // 005B6BC5  mov edx, dword ptr [0x949870]
    edx = ((*(int*)((char*)g_00949870)));
    // 005B6BCB  add ecx, edx
    ecx = ((ecx) + (edx));
    // 005B6BCD  jmp 0x5b6bda
    goto LAB_005B6BDA;
LAB_005B6BCF:
    // 005B6BCF  mov eax, dword ptr [0x949870]
    eax = ((*(int*)((char*)g_00949870)));
    // 005B6BD4  cmp ecx, eax
    // 005B6BD6  jl 0x5b6bda
    if (ecx < eax) goto LAB_005B6BDA;
    // 005B6BD8  sub ecx, eax
    ecx = ((ecx) - (eax));
LAB_005B6BDA:
    // 005B6BDA  mov esi, dword ptr [edi + 0x66ef74]
    esi = ((*(int*)((char*)g_0066ef74 + edi)));
    // 005B6BE0  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005B6BE3  lea edx, [esi + eax]
    edx = (eax);
    // 005B6BE6  test edx, edx
    // 005B6BE8  jl 0x5b6c31
    if (((edx) & (edx)) < 0) goto LAB_005B6C31;
    // 005B6BEA  cmp edx, dword ptr [0x949874]
    // 005B6BF0  jge 0x5b6c31
    if (edx >= (*(int*)((char*)g_00949874))) goto LAB_005B6C31;
    // 005B6BF2  test ecx, ecx
    // 005B6BF4  jl 0x5b6c31
    if (((ecx) & (ecx)) < 0) goto LAB_005B6C31;
    // 005B6BF6  cmp ecx, dword ptr [0x949870]
    // 005B6BFC  jge 0x5b6c31
    if (ecx >= (*(int*)((char*)g_00949870))) goto LAB_005B6C31;
    // 005B6BFE  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B6C03  imul eax, edx
    eax = ((eax) * (edx));
    // 005B6C06  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B6C08  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6C0A  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B6C0D  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6C10  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B6C16  mov al, byte ptr [ebp - 0x28]
    SETB0(eax, ((var_28 & 0xff)));
    // 005B6C19  test byte ptr [ecx + edx*4 + 4], al
    // 005B6C1D  jne 0x5b6c31
    if (((((*(unsigned char*)((char*)(ecx) + edx*4 + 0x4))) & ((eax & 0xff)))) != 0) goto LAB_005B6C31;
    // 005B6C1F  mov ecx, dword ptr [ebp - 0x48]
    ecx = (var_48);
    // 005B6C22  inc ecx
    ecx = ((ecx) + 1);
    // 005B6C23  test ebx, ebx
    // 005B6C25  mov dword ptr [ebp - 0x48], ecx
    var_48 = (ecx);
    // 005B6C28  je 0x5b6c2e
    if ((((ebx) & (ebx))) == 0) goto LAB_005B6C2E;
    // 005B6C2A  test esi, esi
    // 005B6C2C  jne 0x5b6c31
    if ((((esi) & (esi))) != 0) goto LAB_005B6C31;
LAB_005B6C2E:
    // 005B6C2E  inc dword ptr [ebp - 0x48]
    var_48 = ((var_48) + 1);
LAB_005B6C31:
    // 005B6C31  add edi, 4
    edi = ((edi) + (4));
    // 005B6C34  cmp edi, 0x20
    // 005B6C37  jl 0x5b6bae
    if (edi < 0x20) goto LAB_005B6BAE;
    // 005B6C3D  mov eax, dword ptr [ebp - 0x40]
    eax = (var_40);
    // 005B6C40  test eax, eax
    // 005B6C42  jne 0x5b6c58
    if ((((eax) & (eax))) != 0) goto LAB_005B6C58;
    // 005B6C44  mov eax, dword ptr [ebp - 0x50]
    eax = (var_50);
    // 005B6C47  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B6C49  mov dl, byte ptr [eax]
    SETB0(edx, ((*(unsigned char*)((char*)(eax)))));
    // 005B6C4B  mov eax, 4
    eax = (4);
    // 005B6C50  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 005B6C53  cmp edx, 0x60
    // 005B6C56  jge 0x5b6c5d
    if (edx >= 0x60) goto LAB_005B6C5D;
LAB_005B6C58:
    // 005B6C58  mov eax, 6
    eax = (6);
LAB_005B6C5D:
    // 005B6C5D  cmp dword ptr [ebp - 0x48], eax
    // 005B6C60  jge 0x5b6ddb
    if (var_48 >= eax) goto LAB_005B6DDB;
    // 005B6C66  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B6C69  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005B6C6F  cmp eax, ecx
    // 005B6C71  jne 0x5b6cb8
    if ((eax) != (ecx)) goto LAB_005B6CB8;
    // 005B6C73  test byte ptr [0x9a64c0], 0x80
    // 005B6C7A  jne 0x5b6cb8
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B6CB8;
    // 005B6C7C  mov ecx, eax
    ecx = (eax);
    // 005B6C7E  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B6C81  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B6C83  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6C86  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B6C89  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6C8C  mov ecx, dword ptr [edx*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx*4)));
    // 005B6C93  test ch, 2
    // 005B6C96  jne 0x5b6cb8
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B6CB8;
    // 005B6C98  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
    // 005B6C9B  mov esi, dword ptr [ecx + 0xc]
    esi = ((*(int*)((char*)(ecx) + 0xc)));
    // 005B6C9E  or esi, 0x400000
    esi = ((esi) | (0x400000));
    // 005B6CA4  mov dword ptr [ecx + 0xc], esi
    (*(int*)((char*)(ecx) + 0xc)) = (esi);
    // 005B6CA7  mov edx, dword ptr [0x90d91c]
    edx = ((*(int*)((char*)g_0090d91c)));
    // 005B6CAD  or edx, 1
    edx = ((edx) | (1));
    // 005B6CB0  mov dword ptr [0x90d91c], edx
    (*(int*)((char*)g_0090d91c)) = (edx);
    // 005B6CB6  jmp 0x5b6cbb
    goto LAB_005B6CBB;
LAB_005B6CB8:
    // 005B6CB8  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
LAB_005B6CBB:
    // 005B6CBB  test eax, eax
    // 005B6CBD  je 0x5b6cc6
    if ((((eax) & (eax))) == 0) goto LAB_005B6CC6;
    // 005B6CBF  mov edx, dword ptr [ecx + 8]
    edx = ((*(int*)((char*)(ecx) + 0x8)));
    // 005B6CC2  mov dword ptr [ecx + eax*4 + 0xc], edx
    (*(int*)((char*)(ecx) + eax*4 + 0xc)) = (edx);
LAB_005B6CC6:
    // 005B6CC6  mov eax, dword ptr [ebp - 0x50]
    eax = (var_50);
    // 005B6CC9  mov cl, byte ptr [ebp - 0x28]
    SETB0(ecx, ((var_28 & 0xff)));
    // 005B6CCC  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 005B6CCF  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B6CD2  push edx
    // 005B6CD3  or bl, cl
    SETB0(ebx, (((ebx & 0xff)) | ((ecx & 0xff))));
    // 005B6CD5  mov byte ptr [eax + 4], bl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ebx & 0xff));
    // 005B6CD8  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B6CDB  push eax
    // 005B6CDC  call 0x4e3a50
    eax = base_at(eax, edx);
    // 005B6CE1  add esp, 8
    // 005B6CE4  test eax, eax
    // 005B6CE6  jl 0x5b6dc4
    if (((eax) & (eax)) < 0) goto LAB_005B6DC4;
    // 005B6CEC  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
    // 005B6CEF  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 005B6CF2  mov bl, 1
    SETB0(ebx, (1));
    // 005B6CF4  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6CF7  mov ecx, esi
    ecx = (esi);
    // 005B6CF9  shl bl, cl
    SETB0(ebx, (((ebx & 0xff)) << ((ecx & 0xff))));
    // 005B6CFB  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B6CFE  shl eax, 2
    eax = ((eax) << (2));
    // 005B6D01  mov dl, byte ptr [eax + 0x97d04a]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d04a + eax))));
    // 005B6D07  or dl, bl
    SETB0(edx, (((edx & 0xff)) | ((ebx & 0xff))));
    // 005B6D09  mov byte ptr [eax + 0x97d04a], dl
    (*(unsigned char*)((char*)g_0097d04a + eax)) = ((edx & 0xff));
    // 005B6D0F  mov cl, byte ptr [eax + 0x97d046]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d046 + eax))));
    // 005B6D15  mov byte ptr [eax + esi + 0x97d04b], cl
    (*(unsigned char*)((char*)g_0097d04b + esi)) = ((ecx & 0xff));
    // 005B6D1C  movsx esi, word ptr [eax + 0x97d042]
    esi = ((*(short*)((char*)g_0097d042 + eax)));
    // 005B6D23  movsx edi, word ptr [eax + 0x97d040]
    edi = ((*(short*)((char*)g_0097d040 + eax)));
    // 005B6D2A  push esi
    // 005B6D2B  push edi
    // 005B6D2C  call 0x4712a0
    eax = on_map(edi, esi);
    // 005B6D31  add esp, 8
    // 005B6D34  test eax, eax
    // 005B6D36  je 0x5b6dc4
    if ((((eax) & (eax))) == 0) goto LAB_005B6DC4;
    // 005B6D3C  mov eax, esi
    eax = (esi);
    // 005B6D3E  mov edx, edi
    edx = (edi);
    // 005B6D40  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B6D47  sar edx, 1
    edx = ((edx) >> (1));
    // 005B6D49  add eax, edx
    eax = ((eax) + (edx));
    // 005B6D4B  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B6D4E  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6D51  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B6D56  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B6D59  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B6D5C  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B6D62  cmp ecx, edx
    // 005B6D64  jne 0x5b6dae
    if ((ecx) != (edx)) goto LAB_005B6DAE;
    // 005B6D66  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B6D69  test byte ptr [eax + 4], dl
    // 005B6D6C  jne 0x5b6dae
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B6DAE;
    // 005B6D6E  test byte ptr [0x9a64c0], 0x80
    // 005B6D75  jne 0x5b6dae
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B6DAE;
    // 005B6D77  mov edx, ecx
    edx = (ecx);
    // 005B6D79  shl edx, 6
    edx = ((edx) << (6));
    // 005B6D7C  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B6D7E  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B6D81  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B6D84  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005B6D87  mov edx, dword ptr [ecx*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 005B6D8E  test dh, 2
    // 005B6D91  jne 0x5b6dae
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B6DAE;
    // 005B6D93  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B6D96  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B6D9C  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B6D9F  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B6DA5  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B6DA8  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B6DAE:
    // 005B6DAE  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B6DB1  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B6DB4  push edx
    // 005B6DB5  or cl, bl
    SETB0(ecx, (((ecx & 0xff)) | ((ebx & 0xff))));
    // 005B6DB7  push esi
    // 005B6DB8  push edi
    // 005B6DB9  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B6DBC  call 0x591e50
    synch_bit(edi, esi, edx);
    // 005B6DC1  add esp, 0xc
LAB_005B6DC4:
    // 005B6DC4  mov esi, dword ptr [ebp - 0x40]
    esi = (var_40);
    // 005B6DC7  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B6DCA  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B6DCC  test esi, esi
    // 005B6DCE  setne al
    SETB0(eax, (((((esi) & (esi))) != 0) ? 1 : 0));
    // 005B6DD1  add eax, 2
    eax = ((eax) + (2));
    // 005B6DD4  cmp ecx, eax
    // 005B6DD6  jg 0x5b6ddb
    if (ecx > eax) goto LAB_005B6DDB;
    // 005B6DD8  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005B6DDB:
    // 005B6DDB  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B6DDE  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 005B6DE1  inc eax
    eax = ((eax) + 1);
    // 005B6DE2  cmp eax, ecx
    // 005B6DE4  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005B6DE7  jl 0x5b6b0a
    if (eax < ecx) goto LAB_005B6B0A;
LAB_005B6DED:
    // 005B6DED  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B6DF0  test eax, eax
    // 005B6DF2  jl 0x5b6ee2
    if (((eax) & (eax)) < 0) goto LAB_005B6EE2;
    // 005B6DF8  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B6DFB  mov cx, word ptr [ecx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx))));
    // 005B6E02  test cx, cx
    // 005B6E05  jl 0x5b6e1d
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B6E1D;
LAB_005B6E07:
    // 005B6E07  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B6E0A  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B6E0D  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B6E10  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B6E18  test cx, cx
    // 005B6E1B  jge 0x5b6e07
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B6E07;
LAB_005B6E1D:
    // 005B6E1D  test eax, eax
    // 005B6E1F  mov esi, eax
    esi = (eax);
    // 005B6E21  jl 0x5b6ee2
    if (((eax) & (eax)) < 0) goto LAB_005B6EE2;
LAB_005B6E27:
    // 005B6E27  lea edx, [esi + esi*2]
    edx = (esi + esi*2);
    // 005B6E2A  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B6E2C  lea ebx, [esi + edx*4]
    ebx = (esi + edx*4);
    // 005B6E2F  shl ebx, 2
    ebx = ((ebx) << (2));
    // 005B6E32  mov al, byte ptr [ebx + 0x952836]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952836 + ebx))));
    // 005B6E38  mov edi, eax
    edi = (eax);
    // 005B6E3A  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B6E3D  cmp edi, eax
    // 005B6E3F  je 0x5b6ed3
    if ((edi) == (eax)) goto LAB_005B6ED3;
    // 005B6E45  movsx eax, word ptr [ebx + 0x95282a]
    eax = ((*(short*)((char*)g_0095282a + ebx)));
    // 005B6E4C  movsx ecx, word ptr [ebx + 0x952828]
    ecx = ((*(short*)((char*)g_00952828 + ebx)));
    // 005B6E53  test eax, eax
    // 005B6E55  jl 0x5b6e76
    if (((eax) & (eax)) < 0) goto LAB_005B6E76;
    // 005B6E57  cmp eax, dword ptr [0x949874]
    // 005B6E5D  jge 0x5b6e76
    if (eax >= (*(int*)((char*)g_00949874))) goto LAB_005B6E76;
    // 005B6E5F  test ecx, ecx
    // 005B6E61  jl 0x5b6e76
    if (((ecx) & (ecx)) < 0) goto LAB_005B6E76;
    // 005B6E63  cmp ecx, dword ptr [0x949870]
    // 005B6E69  jge 0x5b6e76
    if (ecx >= (*(int*)((char*)g_00949870))) goto LAB_005B6E76;
    // 005B6E6B  push edi
    // 005B6E6C  push eax
    // 005B6E6D  push ecx
    // 005B6E6E  call 0x5b5700
    eax = ((int (__cdecl*)(int,int,int))sub_5b5700)(ecx, eax, edi);
    // 005B6E73  add esp, 0xc
LAB_005B6E76:
    // 005B6E76  mov cx, word ptr [ebx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ebx))));
    // 005B6E7D  mov eax, esi
    eax = (esi);
    // 005B6E7F  test cx, cx
    // 005B6E82  jl 0x5b6e9a
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B6E9A;
LAB_005B6E84:
    // 005B6E84  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B6E87  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B6E8A  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B6E8D  mov cx, word ptr [edx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edx*4))));
    // 005B6E95  test cx, cx
    // 005B6E98  jge 0x5b6e84
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B6E84;
LAB_005B6E9A:
    // 005B6E9A  test eax, eax
    // 005B6E9C  jl 0x5b6ed3
    if (((eax) & (eax)) < 0) goto LAB_005B6ED3;
    // 005B6E9E  mov dl, 1
    SETB0(edx, (1));
    // 005B6EA0  mov ecx, edi
    ecx = (edi);
    // 005B6EA2  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B6EA4:
    // 005B6EA4  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B6EA7  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B6EAA  shl eax, 2
    eax = ((eax) << (2));
    // 005B6EAD  mov cl, byte ptr [eax + 0x95284f]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B6EB3  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B6EB5  test edi, edi
    // 005B6EB7  mov byte ptr [eax + 0x95284f], cl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ecx & 0xff));
    // 005B6EBD  je 0x5b6ec8
    if ((((edi) & (edi))) == 0) goto LAB_005B6EC8;
    // 005B6EBF  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B6EC8:
    // 005B6EC8  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B6ECF  test eax, eax
    // 005B6ED1  jge 0x5b6ea4
    if (((eax) & (eax)) >= 0) goto LAB_005B6EA4;
LAB_005B6ED3:
    // 005B6ED3  movsx esi, word ptr [ebx + 0x952858]
    esi = ((*(short*)((char*)g_00952858 + ebx)));
    // 005B6EDA  test esi, esi
    // 005B6EDC  jge 0x5b6e27
    if (((esi) & (esi)) >= 0) goto LAB_005B6E27;
LAB_005B6EE2:
    // 005B6EE2  mov dword ptr [ebp - 0x18], 0
    var_18 = (0);
LAB_005B6EE9:
    // 005B6EE9  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 005B6EEC  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
    // 005B6EEF  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B6EF5  mov eax, dword ptr [edx*4 + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + edx*4)));
    // 005B6EFC  add eax, esi
    eax = ((eax) + (esi));
    // 005B6EFE  test cl, 1
    // 005B6F01  jne 0x5b6f17
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B6F17;
    // 005B6F03  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B6F09  test eax, eax
    // 005B6F0B  jge 0x5b6f11
    if (((eax) & (eax)) >= 0) goto LAB_005B6F11;
    // 005B6F0D  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6F0F  jmp 0x5b6f17
    goto LAB_005B6F17;
LAB_005B6F11:
    // 005B6F11  cmp eax, ecx
    // 005B6F13  jl 0x5b6f17
    if (eax < ecx) goto LAB_005B6F17;
    // 005B6F15  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_005B6F17:
    // 005B6F17  mov ecx, dword ptr [ebp - 0x24]
    ecx = (var_24);
    // 005B6F1A  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 005B6F1D  mov eax, dword ptr [edx*4 + 0x66f440]
    eax = ((*(int*)((char*)g_0066f440 + edx*4)));
    // 005B6F24  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B6F26  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 005B6F29  js 0x5b7ce2
    if (((eax) - (0x0)) < 0) goto LAB_005B7CE2;
    // 005B6F2F  mov edx, dword ptr [0x949874]
    edx = ((*(int*)((char*)g_00949874)));
    // 005B6F35  cmp eax, edx
    // 005B6F37  jge 0x5b7ce2
    if (eax >= edx) goto LAB_005B7CE2;
    // 005B6F3D  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B6F40  test eax, eax
    // 005B6F42  jl 0x5b7ce2
    if (((eax) & (eax)) < 0) goto LAB_005B7CE2;
    // 005B6F48  mov eax, dword ptr [0x949870]
    eax = ((*(int*)((char*)g_00949870)));
    // 005B6F4D  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 005B6F50  cmp ecx, eax
    // 005B6F52  jge 0x5b7ce2
    if (ecx >= eax) goto LAB_005B7CE2;
    // 005B6F58  mov ebx, dword ptr [ebp - 0x14]
    ebx = (var_14);
    // 005B6F5B  mov esi, ecx
    esi = (ecx);
    // 005B6F5D  push ebx
    // 005B6F5E  push esi
    // 005B6F5F  call 0x5bf010
    eax = is_sensor(esi, ebx);
    // 005B6F64  add esp, 8
    // 005B6F67  test eax, eax
    // 005B6F69  je 0x5b70b9
    if ((((eax) & (eax))) == 0) goto LAB_005B70B9;
    // 005B6F6F  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B6F74  imul eax, ebx
    eax = ((eax) * (ebx));
    // 005B6F77  sar esi, 1
    esi = ((esi) >> (1));
    // 005B6F79  add eax, esi
    eax = ((eax) + (esi));
    // 005B6F7B  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B6F7E  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B6F81  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B6F86  movsx esi, byte ptr [eax + edx*4 + 7]
    esi = ((*(signed char*)((char*)(eax) + edx*4 + 0x7)));
    // 005B6F8B  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B6F8E  test esi, esi
    // 005B6F90  jle 0x5b70b9
    if (((esi) & (esi)) <= 0) goto LAB_005B70B9;
    // 005B6F96  cmp esi, dword ptr [ebp - 8]
    // 005B6F99  je 0x5b70b9
    if ((esi) == (var_8)) goto LAB_005B70B9;
    // 005B6F9F  cmp dword ptr [ebp - 0x34], 1
    // 005B6FA3  je 0x5b70b9
    if ((var_34) == (1)) goto LAB_005B70B9;
    // 005B6FA9  test byte ptr [eax + 8], 1
    // 005B6FAD  je 0x5b6fc2
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (1))) == 0) goto LAB_005B6FC2;
    // 005B6FAF  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B6FB2  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B6FB5  cmp eax, 8
    // 005B6FB8  jge 0x5b6fc2
    if (eax >= 8) goto LAB_005B6FC2;
    // 005B6FBA  test eax, eax
    // 005B6FBC  jge 0x5b70b9
    if (((eax) & (eax)) >= 0) goto LAB_005B70B9;
LAB_005B6FC2:
    // 005B6FC2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B6FC5  mov edi, dword ptr [ebp - 0x20]
    edi = (var_20);
    // 005B6FC8  test eax, eax
    // 005B6FCA  jl 0x5b6ffd
    if (((eax) & (eax)) < 0) goto LAB_005B6FFD;
    // 005B6FCC  movsx eax, word ptr [edi + 0x95282a]
    eax = ((*(short*)((char*)g_0095282a + edi)));
    // 005B6FD3  movsx ecx, word ptr [edi + 0x952828]
    ecx = ((*(short*)((char*)g_00952828 + edi)));
    // 005B6FDA  test eax, eax
    // 005B6FDC  jl 0x5b6ffd
    if (((eax) & (eax)) < 0) goto LAB_005B6FFD;
    // 005B6FDE  cmp eax, dword ptr [0x949874]
    // 005B6FE4  jge 0x5b6ffd
    if (eax >= (*(int*)((char*)g_00949874))) goto LAB_005B6FFD;
    // 005B6FE6  test ecx, ecx
    // 005B6FE8  jl 0x5b6ffd
    if (((ecx) & (ecx)) < 0) goto LAB_005B6FFD;
    // 005B6FEA  cmp ecx, dword ptr [0x949870]
    // 005B6FF0  jge 0x5b6ffd
    if (ecx >= (*(int*)((char*)g_00949870))) goto LAB_005B6FFD;
    // 005B6FF2  push esi
    // 005B6FF3  push eax
    // 005B6FF4  push ecx
    // 005B6FF5  call 0x5b5700
    eax = ((int (__cdecl*)(int,int,int))sub_5b5700)(ecx, eax, esi);
    // 005B6FFA  add esp, 0xc
LAB_005B6FFD:
    // 005B6FFD  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B7000  test eax, eax
    // 005B7002  jl 0x5b7056
    if (((eax) & (eax)) < 0) goto LAB_005B7056;
    // 005B7004  mov cx, word ptr [edi + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edi))));
    // 005B700B  test cx, cx
    // 005B700E  jl 0x5b7026
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B7026;
LAB_005B7010:
    // 005B7010  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B7013  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B7016  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B7019  mov cx, word ptr [edx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edx*4))));
    // 005B7021  test cx, cx
    // 005B7024  jge 0x5b7010
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B7010;
LAB_005B7026:
    // 005B7026  test eax, eax
    // 005B7028  jl 0x5b7056
    if (((eax) & (eax)) < 0) goto LAB_005B7056;
    // 005B702A  mov dl, 1
    SETB0(edx, (1));
    // 005B702C  mov ecx, esi
    ecx = (esi);
    // 005B702E  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B7030  mov ecx, 0xfffffbbf
    ecx = (0xfffffbbf);
LAB_005B7035:
    // 005B7035  lea edi, [eax + eax*2]
    edi = (eax + eax*2);
    // 005B7038  lea eax, [eax + edi*4]
    eax = (eax + edi*4);
    // 005B703B  shl eax, 2
    eax = ((eax) << (2));
    // 005B703E  or byte ptr [eax + 0x95284f], dl
    (*(unsigned char*)((char*)g_0095284f + eax)) = (((*(unsigned char*)((char*)g_0095284f + eax))) | ((edx & 0xff)));
    // 005B7044  and word ptr [eax + 0x952830], cx
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & ((ecx & 0xffff)));
    // 005B704B  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B7052  test eax, eax
    // 005B7054  jge 0x5b7035
    if (((eax) & (eax)) >= 0) goto LAB_005B7035;
LAB_005B7056:
    // 005B7056  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7059  mov ecx, eax
    ecx = (eax);
    // 005B705B  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B705E  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B7060  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B7063  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B7066  lea edx, [esi + eax]
    edx = (eax);
    // 005B7069  lea eax, [edx + ecx*2]
    eax = (edx + ecx*2);
    // 005B706C  test byte ptr [eax*4 + 0x96c9f8], 7
    // 005B7074  jne 0x5b70b9
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (7))) != 0) goto LAB_005B70B9;
    // 005B7076  mov eax, dword ptr [ebp + 0xc]
    eax = (a2);
    // 005B7079  test eax, eax
    // 005B707B  je 0x5b70b9
    if ((((eax) & (eax))) == 0) goto LAB_005B70B9;
    // 005B707D  mov eax, dword ptr [ebp - 0x3c]
    eax = (var_3c);
    // 005B7080  push -1
    // 005B7082  test eax, eax
    // 005B7084  push ebx
    // 005B7085  jne 0x5b70a8
    if ((((eax) & (eax))) != 0) goto LAB_005B70A8;
    // 005B7087  mov edi, dword ptr [ebp - 0xc]
    edi = (var_c);
    // 005B708A  push edi
    // 005B708B  push 2
    // 005B708D  push 0
    // 005B708F  push esi
    // 005B7090  call 0x579a30
    add_goal(esi, 0, 2, edi, ebx, -1);
    // 005B7095  push -1
    // 005B7097  push ebx
    // 005B7098  push edi
    // 005B7099  push 1
    // 005B709B  push 2
    // 005B709D  push esi
    // 005B709E  call 0x579a30
    add_goal(esi, 2, 1, edi, ebx, -1);
    // 005B70A3  add esp, 0x30
    // 005B70A6  jmp 0x5b70b9
    goto LAB_005B70B9;
LAB_005B70A8:
    // 005B70A8  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 005B70AB  push ecx
    // 005B70AC  push 2
    // 005B70AE  push 6
    // 005B70B0  push esi
    // 005B70B1  call 0x579a30
    add_goal(esi, 6, 2, ecx, ebx, -0x1);
    // 005B70B6  add esp, 0x18
LAB_005B70B9:
    // 005B70B9  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B70BC  test eax, eax
    // 005B70BE  je 0x5b7ce2
    if ((((eax) & (eax))) == 0) goto LAB_005B7CE2;
    // 005B70C4  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005B70C7  push ebx
    // 005B70C8  push edx
    // 005B70C9  call 0x4e3a50
    eax = base_at(edx, ebx);
    // 005B70CE  add esp, 8
    // 005B70D1  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 005B70D4  test eax, eax
    // 005B70D6  jl 0x5b73e1
    if (((eax) & (eax)) < 0) goto LAB_005B73E1;
    // 005B70DC  mov esi, eax
    esi = (eax);
    // 005B70DE  lea eax, [esi + esi*8]
    eax = (esi + esi*8);
    // 005B70E1  lea ecx, [esi + eax*2]
    ecx = (esi + eax*2);
    // 005B70E4  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B70E7  lea esi, [esi + ecx*4]
    esi = (esi + ecx*4);
    // 005B70EA  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B70EC  shl esi, 2
    esi = ((esi) << (2));
    // 005B70EF  mov cl, byte ptr [esi + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 005B70F5  cmp eax, ecx
    // 005B70F7  je 0x5b73e1
    if ((eax) == (ecx)) goto LAB_005B73E1;
    // 005B70FD  mov eax, dword ptr [ebp - 0x3c]
    eax = (var_3c);
    // 005B7100  mov edx, 2
    edx = (2);
    // 005B7105  cmp eax, edx
    // 005B7107  jne 0x5b7134
    if ((eax) != (edx)) goto LAB_005B7134;
    // 005B7109  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B710C  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 005B710E  movsx eax, word ptr [eax + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + eax)));
    // 005B7115  lea edi, [eax + eax*2]
    edi = (eax + eax*2);
    // 005B7118  lea eax, [eax + edi*4]
    eax = (eax + edi*4);
    // 005B711B  mov bl, byte ptr [eax*4 + 0x9ab88c]
    SETB0(ebx, ((*(unsigned char*)((char*)g_009ab88c + eax*4))));
    // 005B7122  mov eax, ebx
    eax = (ebx);
    // 005B7124  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005B7127  shl eax, 4
    eax = ((eax) << (4));
    // 005B712A  mov bl, byte ptr [eax + 0x94a37a]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0094a37a + eax))));
    // 005B7130  test bl, bl
    // 005B7132  jne 0x5b717d
    if (((((ebx & 0xff)) & ((ebx & 0xff)))) != 0) goto LAB_005B717D;
LAB_005B7134:
    // 005B7134  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B7137  test eax, eax
    // 005B7139  jne 0x5b7142
    if ((((eax) & (eax))) != 0) goto LAB_005B7142;
    // 005B713B  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 005B713E  test eax, eax
    // 005B7140  je 0x5b716e
    if ((((eax) & (eax))) == 0) goto LAB_005B716E;
LAB_005B7142:
    // 005B7142  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7145  mov edi, eax
    edi = (eax);
    // 005B7147  shl edi, 6
    edi = ((edi) << (6));
    // 005B714A  add edi, eax
    edi = ((edi) + (eax));
    // 005B714C  lea ebx, [ecx + eax]
    ebx = (eax);
    // 005B714F  lea edi, [eax + edi*2]
    edi = (eax + edi*2);
    // 005B7152  lea edi, [eax + edi*8]
    edi = (eax + edi*8);
    // 005B7155  lea edi, [ebx + edi*2]
    edi = (ebx + edi*2);
    // 005B7158  test byte ptr [edi*4 + 0x96c9f8], 8
    // 005B7160  jne 0x5b7180
    if (((((*(unsigned char*)((char*)g_0096c9f8 + edi*4))) & (8))) != 0) goto LAB_005B7180;
    // 005B7162  mov edi, 1
    edi = (1);
    // 005B7167  shl edi, cl
    edi = ((edi) << ((ecx & 0xff)));
    // 005B7169  or dword ptr [ebp - 0x4c], edi
    var_4c = ((var_4c) | (edi));
    // 005B716C  jmp 0x5b7180
    goto LAB_005B7180;
LAB_005B716E:
    // 005B716E  mov edi, 1
    edi = (1);
    // 005B7173  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7176  shl edi, cl
    edi = ((edi) << ((ecx & 0xff)));
    // 005B7178  or dword ptr [ebp - 0x4c], edi
    var_4c = ((var_4c) | (edi));
    // 005B717B  jmp 0x5b7180
    goto LAB_005B7180;
LAB_005B717D:
    // 005B717D  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
LAB_005B7180:
    // 005B7180  cmp eax, dword ptr [0x939284]
    // 005B7186  jne 0x5b71cb
    if ((eax) != ((*(int*)((char*)g_00939284)))) goto LAB_005B71CB;
    // 005B7188  test byte ptr [0x9a64c0], 0x80
    // 005B718F  jne 0x5b71cb
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B71CB;
    // 005B7191  mov cl, byte ptr [ebp - 0x28]
    SETB0(ecx, ((var_28 & 0xff)));
    // 005B7194  mov bl, byte ptr [esi + 0x97d04a]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0097d04a + esi))));
    // 005B719A  test cl, bl
    // 005B719C  jne 0x5b71a8
    if (((((ecx & 0xff)) & ((ebx & 0xff)))) != 0) goto LAB_005B71A8;
    // 005B719E  cmp dword ptr [ebp - 0x1c], edx
    // 005B71A1  jg 0x5b71cb
    if (var_1c > edx) goto LAB_005B71CB;
    // 005B71A3  mov dword ptr [ebp - 0x1c], edx
    var_1c = (edx);
    // 005B71A6  jmp 0x5b71cb
    goto LAB_005B71CB;
LAB_005B71A8:
    // 005B71A8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B71AA  mov dl, byte ptr [esi + eax + 0x97d04b]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d04b + eax))));
    // 005B71B1  movsx eax, byte ptr [esi + 0x97d046]
    eax = ((*(signed char*)((char*)g_0097d046 + esi)));
    // 005B71B8  cmp edx, eax
    // 005B71BA  je 0x5b71cb
    if ((edx) == (eax)) goto LAB_005B71CB;
    // 005B71BC  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B71BF  mov eax, 1
    eax = (1);
    // 005B71C4  cmp ecx, eax
    // 005B71C6  jg 0x5b71cb
    if (ecx > eax) goto LAB_005B71CB;
    // 005B71C8  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005B71CB:
    // 005B71CB  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B71CE  mov dl, byte ptr [esi + 0x97d04a]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d04a + esi))));
    // 005B71D4  mov al, 1
    SETB0(eax, (1));
    // 005B71D6  shl al, cl
    SETB0(eax, (((eax & 0xff)) << ((ecx & 0xff))));
    // 005B71D8  or dl, al
    SETB0(edx, (((edx & 0xff)) | ((eax & 0xff))));
    // 005B71DA  mov byte ptr [ebp - 1], al
    var_1 = ((eax & 0xff));
    // 005B71DD  mov byte ptr [esi + 0x97d04a], dl
    (*(unsigned char*)((char*)g_0097d04a + esi)) = ((edx & 0xff));
    // 005B71E3  mov dl, byte ptr [esi + 0x97d046]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d046 + esi))));
    // 005B71E9  mov byte ptr [esi + ecx + 0x97d04b], dl
    (*(unsigned char*)((char*)g_0097d04b + ecx)) = ((edx & 0xff));
    // 005B71F0  movsx edi, word ptr [esi + 0x97d042]
    edi = ((*(short*)((char*)g_0097d042 + esi)));
    // 005B71F7  movsx ebx, word ptr [esi + 0x97d040]
    ebx = ((*(short*)((char*)g_0097d040 + esi)));
    // 005B71FE  push edi
    // 005B71FF  push ebx
    // 005B7200  call 0x4712a0
    eax = on_map(ebx, edi);
    // 005B7205  add esp, 8
    // 005B7208  test eax, eax
    // 005B720A  je 0x5b729c
    if ((((eax) & (eax))) == 0) goto LAB_005B729C;
    // 005B7210  mov eax, edi
    eax = (edi);
    // 005B7212  mov ecx, ebx
    ecx = (ebx);
    // 005B7214  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B721B  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B721D  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B721F  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B7225  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B7228  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B722B  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B7231  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B7234  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B7237  cmp ecx, edx
    // 005B7239  jne 0x5b7283
    if ((ecx) != (edx)) goto LAB_005B7283;
    // 005B723B  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B723E  test byte ptr [eax + 4], dl
    // 005B7241  jne 0x5b7283
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B7283;
    // 005B7243  test byte ptr [0x9a64c0], 0x80
    // 005B724A  jne 0x5b7283
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B7283;
    // 005B724C  mov edx, ecx
    edx = (ecx);
    // 005B724E  shl edx, 6
    edx = ((edx) << (6));
    // 005B7251  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B7253  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B7256  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B7259  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005B725C  mov edx, dword ptr [ecx*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 005B7263  test dh, 2
    // 005B7266  jne 0x5b7283
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B7283;
    // 005B7268  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B726B  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B7271  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B7274  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B727A  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B727D  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B7283:
    // 005B7283  mov dl, byte ptr [ebp - 1]
    SETB0(edx, ((var_1 & 0xff)));
    // 005B7286  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B7289  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B728B  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B728E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7291  push eax
    // 005B7292  push edi
    // 005B7293  push ebx
    // 005B7294  call 0x591e50
    synch_bit(ebx, edi, eax);
    // 005B7299  add esp, 0xc
LAB_005B729C:
    // 005B729C  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 005B729F  test eax, eax
    // 005B72A1  je 0x5b72ce
    if ((((eax) & (eax))) == 0) goto LAB_005B72CE;
    // 005B72A3  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B72A6  mov ecx, eax
    ecx = (eax);
    // 005B72A8  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B72AB  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B72AD  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B72B0  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B72B3  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B72B6  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B72B8  mov al, byte ptr [esi + 0x97d044]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 005B72BE  add edx, eax
    edx = ((edx) + (eax));
    // 005B72C0  test byte ptr [edx*4 + 0x96c9f8], 8
    // 005B72C8  jne 0x5b737f
    if (((((*(unsigned char*)((char*)g_0096c9f8 + edx*4))) & (8))) != 0) goto LAB_005B737F;
LAB_005B72CE:
    // 005B72CE  mov ebx, dword ptr [ebp + 8]
    ebx = (a1);
    // 005B72D1  mov edi, dword ptr [ebp - 0x20]
    edi = (var_20);
    // 005B72D4  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B72D6  mov cl, byte ptr [esi + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 005B72DC  test ebx, ebx
    // 005B72DE  mov dword ptr [ebp - 0x50], ecx
    var_50 = (ecx);
    // 005B72E1  jl 0x5b7317
    if (((ebx) & (ebx)) < 0) goto LAB_005B7317;
    // 005B72E3  movsx eax, word ptr [edi + 0x95282a]
    eax = ((*(short*)((char*)g_0095282a + edi)));
    // 005B72EA  movsx ecx, word ptr [edi + 0x952828]
    ecx = ((*(short*)((char*)g_00952828 + edi)));
    // 005B72F1  test eax, eax
    // 005B72F3  jl 0x5b7317
    if (((eax) & (eax)) < 0) goto LAB_005B7317;
    // 005B72F5  cmp eax, dword ptr [0x949874]
    // 005B72FB  jge 0x5b7317
    if (eax >= (*(int*)((char*)g_00949874))) goto LAB_005B7317;
    // 005B72FD  test ecx, ecx
    // 005B72FF  jl 0x5b7317
    if (((ecx) & (ecx)) < 0) goto LAB_005B7317;
    // 005B7301  cmp ecx, dword ptr [0x949870]
    // 005B7307  jge 0x5b7317
    if (ecx >= (*(int*)((char*)g_00949870))) goto LAB_005B7317;
    // 005B7309  mov edx, dword ptr [ebp - 0x50]
    edx = (var_50);
    // 005B730C  push edx
    // 005B730D  push eax
    // 005B730E  push ecx
    // 005B730F  call 0x5b5700
    eax = ((int (__cdecl*)(int,int,int))sub_5b5700)(ecx, eax, edx);
    // 005B7314  add esp, 0xc
LAB_005B7317:
    // 005B7317  test ebx, ebx
    // 005B7319  mov eax, ebx
    eax = (ebx);
    // 005B731B  jl 0x5b737f
    if (((ebx) & (ebx)) < 0) goto LAB_005B737F;
    // 005B731D  mov cx, word ptr [edi + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edi))));
    // 005B7324  test cx, cx
    // 005B7327  jl 0x5b733f
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B733F;
LAB_005B7329:
    // 005B7329  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B732C  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B732F  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B7332  mov cx, word ptr [edx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edx*4))));
    // 005B733A  test cx, cx
    // 005B733D  jge 0x5b7329
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B7329;
LAB_005B733F:
    // 005B733F  test eax, eax
    // 005B7341  jl 0x5b737f
    if (((eax) & (eax)) < 0) goto LAB_005B737F;
    // 005B7343  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
    // 005B7346  mov dl, 1
    SETB0(edx, (1));
    // 005B7348  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B734A  mov ecx, 0xfffffbbf
    ecx = (0xfffffbbf);
LAB_005B734F:
    // 005B734F  lea edi, [eax + eax*2]
    edi = (eax + eax*2);
    // 005B7352  lea eax, [eax + edi*4]
    eax = (eax + edi*4);
    // 005B7355  mov edi, dword ptr [ebp - 0x50]
    edi = (var_50);
    // 005B7358  shl eax, 2
    eax = ((eax) << (2));
    // 005B735B  mov bl, byte ptr [eax + 0x95284f]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B7361  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B7363  test edi, edi
    // 005B7365  mov byte ptr [eax + 0x95284f], bl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ebx & 0xff));
    // 005B736B  je 0x5b7374
    if ((((edi) & (edi))) == 0) goto LAB_005B7374;
    // 005B736D  and word ptr [eax + 0x952830], cx
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & ((ecx & 0xffff)));
LAB_005B7374:
    // 005B7374  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B737B  test eax, eax
    // 005B737D  jge 0x5b734f
    if (((eax) & (eax)) >= 0) goto LAB_005B734F;
LAB_005B737F:
    // 005B737F  mov eax, dword ptr [ebp - 0x3c]
    eax = (var_3c);
    // 005B7382  test eax, eax
    // 005B7384  jne 0x5b73e1
    if ((((eax) & (eax))) != 0) goto LAB_005B73E1;
    // 005B7386  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B7388  mov al, byte ptr [esi + 0x97d044]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 005B738E  mov ecx, eax
    ecx = (eax);
    // 005B7390  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B7393  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B7395  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B7398  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B739B  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B739E  add edx, eax
    edx = ((edx) + (eax));
    // 005B73A0  lea ecx, [edx + ecx*2]
    ecx = (edx + ecx*2);
    // 005B73A3  test byte ptr [ecx*4 + 0x96c9f8], 7
    // 005B73AB  jne 0x5b73e1
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (7))) != 0) goto LAB_005B73E1;
    // 005B73AD  mov ecx, dword ptr [ebp + 0xc]
    ecx = (a2);
    // 005B73B0  test ecx, ecx
    // 005B73B2  je 0x5b73e1
    if ((((ecx) & (ecx))) == 0) goto LAB_005B73E1;
    // 005B73B4  mov edi, dword ptr [ebp - 0x14]
    edi = (var_14);
    // 005B73B7  mov ebx, dword ptr [ebp - 0xc]
    ebx = (var_c);
    // 005B73BA  push -1
    // 005B73BC  push edi
    // 005B73BD  push ebx
    // 005B73BE  push 2
    // 005B73C0  push 0
    // 005B73C2  push eax
    // 005B73C3  call 0x579a30
    add_goal(eax, 0, 2, ebx, edi, -1);
    // 005B73C8  push -1
    // 005B73CA  push edi
    // 005B73CB  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B73CD  push ebx
    // 005B73CE  mov dl, byte ptr [esi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 005B73D4  push 1
    // 005B73D6  push 2
    // 005B73D8  push edx
    // 005B73D9  call 0x579a30
    add_goal(edx, 2, 1, ebx, edi, -1);
    // 005B73DE  add esp, 0x30
LAB_005B73E1:
    // 005B73E1  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B73E6  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 005B73E9  mov edi, dword ptr [ebp - 0xc]
    edi = (var_c);
    // 005B73EC  imul eax, esi
    eax = ((eax) * (esi));
    // 005B73EF  mov ecx, edi
    ecx = (edi);
    // 005B73F1  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 005B73F3  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B73F5  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B73F7  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B73F9  push 0x40
    // 005B73FB  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B73FE  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B7401  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005B7407  mov cl, byte ptr [edx + eax*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(edx) + eax*4))));
    // 005B740A  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B740D  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B7410  cmp ecx, 0x60
    // 005B7413  movsx ecx, word ptr [eax + 0x952832]
    ecx = ((*(short*)((char*)g_00952832 + eax)));
    // 005B741A  setl bl
    SETB0(ebx, ((ecx < 0x60) ? 1 : 0));
    // 005B741D  push ecx
    // 005B741E  mov dword ptr [ebp - 0x48], ebx
    var_48 = (ebx);
    // 005B7421  call 0x5bf1f0
    eax = has_abil(ecx, 0x40);
    // 005B7426  add esp, 8
    // 005B7429  test eax, eax
    // 005B742B  je 0x5b7435
    if ((((eax) & (eax))) == 0) goto LAB_005B7435;
    // 005B742D  test ebx, ebx
    // 005B742F  je 0x5b7ce2
    if ((((ebx) & (ebx))) == 0) goto LAB_005B7CE2;
LAB_005B7435:
    // 005B7435  push esi
    // 005B7436  push edi
    // 005B7437  call 0x5bfe90
    eax = veh_at(edi, esi);
    // 005B743C  push eax
    // 005B743D  call 0x5b8e10
    eax = stack_fix(eax);
    // 005B7442  add esp, 0xc
    // 005B7445  mov dword ptr [ebp - 0x2c], eax
    var_2c = (eax);
    // 005B7448  test eax, eax
    // 005B744A  jl 0x5b7ce2
    if (((eax) & (eax)) < 0) goto LAB_005B7CE2;
    // 005B7450  mov ebx, eax
    ebx = (eax);
    // 005B7452  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
    // 005B7455  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B7457  lea edx, [ebx + ebx*2]
    edx = (ebx + ebx*2);
    // 005B745A  lea ebx, [ebx + edx*4]
    ebx = (ebx + edx*4);
    // 005B745D  shl ebx, 2
    ebx = ((ebx) << (2));
    // 005B7460  mov al, byte ptr [ebx + 0x952836]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952836 + ebx))));
    // 005B7466  cmp eax, esi
    // 005B7468  mov dword ptr [ebp - 0x10], eax
    var_10 = (eax);
    // 005B746B  je 0x5b7ce2
    if ((eax) == (esi)) goto LAB_005B7CE2;
    // 005B7471  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 005B7474  test eax, eax
    // 005B7476  je 0x5b74b9
    if ((((eax) & (eax))) == 0) goto LAB_005B74B9;
    // 005B7478  movsx eax, word ptr [ebx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B747F  push 2
    // 005B7481  push eax
    // 005B7482  call 0x5bf1f0
    eax = has_abil(eax, 2);
    // 005B7487  add esp, 8
    // 005B748A  test eax, eax
    // 005B748C  jne 0x5b74b9
    if ((((eax) & (eax))) != 0) goto LAB_005B74B9;
    // 005B748E  mov ecx, esi
    ecx = (esi);
    // 005B7490  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B7493  add ecx, esi
    ecx = ((ecx) + (esi));
    // 005B7495  lea edx, [esi + ecx*2]
    edx = (esi + ecx*2);
    // 005B7498  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B749A  mov cl, byte ptr [ebx + 0x952836]
    SETB0(ecx, ((*(unsigned char*)((char*)g_00952836 + ebx))));
    // 005B74A0  lea eax, [esi + edx*8]
    eax = (esi + edx*8);
    // 005B74A3  lea eax, [esi + eax*2]
    eax = (esi + eax*2);
    // 005B74A6  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B74A8  mov dword ptr [ebp - 0x50], eax
    var_50 = (eax);
    // 005B74AB  test byte ptr [ecx*4 + 0x96c9f8], 8
    // 005B74B3  jne 0x5b777b
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (8))) != 0) goto LAB_005B777B;
LAB_005B74B9:
    // 005B74B9  mov eax, dword ptr [ebp - 0x30]
    eax = (var_30);
    // 005B74BC  test eax, eax
    // 005B74BE  jge 0x5b75f9
    if (((eax) & (eax)) >= 0) goto LAB_005B75F9;
    // 005B74C4  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B74C7  test eax, eax
    // 005B74C9  jl 0x5b75b0
    if (((eax) & (eax)) < 0) goto LAB_005B75B0;
    // 005B74CF  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B74D2  movsx esi, word ptr [eax + 0x95282a]
    esi = ((*(short*)((char*)g_0095282a + eax)));
    // 005B74D9  movsx edi, word ptr [eax + 0x952828]
    edi = ((*(short*)((char*)g_00952828 + eax)));
    // 005B74E0  test esi, esi
    // 005B74E2  jl 0x5b75b0
    if (((esi) & (esi)) < 0) goto LAB_005B75B0;
    // 005B74E8  cmp esi, dword ptr [0x949874]
    // 005B74EE  jge 0x5b75b0
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B75B0;
    // 005B74F4  test edi, edi
    // 005B74F6  jl 0x5b75b0
    if (((edi) & (edi)) < 0) goto LAB_005B75B0;
    // 005B74FC  cmp edi, dword ptr [0x949870]
    // 005B7502  jge 0x5b75b0
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B75B0;
    // 005B7508  push esi
    // 005B7509  push edi
    // 005B750A  call 0x4712a0
    eax = on_map(edi, esi);
    // 005B750F  add esp, 8
    // 005B7512  test eax, eax
    // 005B7514  je 0x5b75b0
    if ((((eax) & (eax))) == 0) goto LAB_005B75B0;
    // 005B751A  mov eax, esi
    eax = (esi);
    // 005B751C  mov edx, edi
    edx = (edi);
    // 005B751E  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B7525  sar edx, 1
    edx = ((edx) >> (1));
    // 005B7527  add eax, edx
    eax = ((eax) + (edx));
    // 005B7529  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B752C  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B752F  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B7534  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7537  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B753A  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B7540  cmp ecx, edx
    // 005B7542  jne 0x5b7593
    if ((ecx) != (edx)) goto LAB_005B7593;
    // 005B7544  mov edx, 1
    edx = (1);
    // 005B7549  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B754B  test byte ptr [eax + 4], dl
    // 005B754E  jne 0x5b7593
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B7593;
    // 005B7550  test byte ptr [0x9a64c0], 0x80
    // 005B7557  jne 0x5b7593
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B7593;
    // 005B7559  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B755C  mov edx, ecx
    edx = (ecx);
    // 005B755E  shl edx, 6
    edx = ((edx) << (6));
    // 005B7561  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B7563  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B7566  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B7569  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005B756C  mov edx, dword ptr [ecx*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 005B7573  test dh, 2
    // 005B7576  jne 0x5b7593
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B7593;
    // 005B7578  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B757B  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B7581  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B7584  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B758A  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B758D  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B7593:
    // 005B7593  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7596  mov dl, 1
    SETB0(edx, (1));
    // 005B7598  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B759A  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B759D  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B759F  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B75A2  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 005B75A5  push eax
    // 005B75A6  push esi
    // 005B75A7  push edi
    // 005B75A8  call 0x591e50
    synch_bit(edi, esi, eax);
    // 005B75AD  add esp, 0xc
LAB_005B75B0:
    // 005B75B0  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005B75B3  push ecx
    // 005B75B4  call 0x579920
    eax = veh_top(ecx);
    // 005B75B9  add esp, 4
    // 005B75BC  test eax, eax
    // 005B75BE  jl 0x5b75f9
    if (((eax) & (eax)) < 0) goto LAB_005B75F9;
    // 005B75C0  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B75C3  mov dl, 1
    SETB0(edx, (1));
    // 005B75C5  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B75C7:
    // 005B75C7  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B75CA  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B75CD  shl eax, 2
    eax = ((eax) << (2));
    // 005B75D0  mov cl, byte ptr [eax + 0x95284f]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B75D6  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B75D8  mov byte ptr [eax + 0x95284f], cl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ecx & 0xff));
    // 005B75DE  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B75E1  test ecx, ecx
    // 005B75E3  je 0x5b75ee
    if ((((ecx) & (ecx))) == 0) goto LAB_005B75EE;
    // 005B75E5  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B75EE:
    // 005B75EE  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B75F5  test eax, eax
    // 005B75F7  jge 0x5b75c7
    if (((eax) & (eax)) >= 0) goto LAB_005B75C7;
LAB_005B75F9:
    // 005B75F9  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B75FC  mov edi, dword ptr [ebp - 0x10]
    edi = (var_10);
    // 005B75FF  mov edx, eax
    edx = (eax);
    // 005B7601  shl edx, 6
    edx = ((edx) << (6));
    // 005B7604  add edx, eax
    edx = ((edx) + (eax));
    // 005B7606  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 005B7609  lea edx, [eax + ecx*8]
    edx = (eax + ecx*8);
    // 005B760C  lea esi, [eax + edx*2]
    esi = (eax + edx*2);
    // 005B760F  mov dword ptr [ebp - 0x50], esi
    var_50 = (esi);
    // 005B7612  add esi, edi
    esi = ((esi) + (edi));
    // 005B7614  test byte ptr [esi*4 + 0x96c9f8], 1
    // 005B761C  jne 0x5b777b
    if (((((*(unsigned char*)((char*)g_0096c9f8 + esi*4))) & (1))) != 0) goto LAB_005B777B;
    // 005B7622  mov eax, dword ptr [ebp + 0xc]
    eax = (a2);
    // 005B7625  test eax, eax
    // 005B7627  je 0x5b777b
    if ((((eax) & (eax))) == 0) goto LAB_005B777B;
    // 005B762D  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005B7630  push eax
    // 005B7631  call 0x5b6060
    wake_stack(eax);
    // 005B7636  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005B763B  mov edx, 1
    edx = (1);
    // 005B7640  mov ecx, edi
    ecx = (edi);
    // 005B7642  add esp, 4
    // 005B7645  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B7647  test dl, al
    // 005B7649  je 0x5b76ed
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_005B76ED;
    // 005B764F  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B7652  mov ecx, dword ptr [ebp - 0x48]
    ecx = (var_48);
    // 005B7655  cmp eax, ecx
    // 005B7657  jne 0x5b777b
    if ((eax) != (ecx)) goto LAB_005B777B;
    // 005B765D  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005B7660  mov edx, dword ptr [ebp - 0x2c]
    edx = (var_2c);
    // 005B7663  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7666  push ecx
    // 005B7667  push edx
    // 005B7668  push eax
    // 005B7669  call 0x5b5ea0
    eax = want_to_wake(eax, edx, ecx);
    // 005B766E  add esp, 0xc
    // 005B7671  test eax, eax
    // 005B7673  je 0x5b777b
    if ((((eax) & (eax))) == 0) goto LAB_005B777B;
    // 005B7679  mov eax, dword ptr [ebx + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B767F  mov ecx, eax
    ecx = (eax);
    // 005B7681  and ecx, 0x2000200
    ecx = ((ecx) & (0x2000200));
    // 005B7687  cmp ecx, 0x2000200
    // 005B768D  jne 0x5b76aa
    if ((ecx) != (0x2000200)) goto LAB_005B76AA;
    // 005B768F  mov dl, byte ptr [ebx + 0x95283b]
    SETB0(edx, ((*(unsigned char*)((char*)g_0095283b + ebx))));
    // 005B7695  mov cl, byte ptr [ebx + 0x95283a]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095283a + ebx))));
    // 005B769B  cmp dl, cl
    // 005B769D  jae 0x5b76aa
    if (((unsigned char)(edx & 0xff)) >= ((unsigned char)(ecx & 0xff))) goto LAB_005B76AA;
    // 005B769F  and eax, 0xfdfffdff
    eax = ((eax) & (0xfdfffdff));
    // 005B76A4  mov dword ptr [ebx + 0x95282c], eax
    (*(int*)((char*)g_0095282c + ebx)) = (eax);
LAB_005B76AA:
    // 005B76AA  mov eax, dword ptr [ebx + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B76B0  mov ecx, eax
    ecx = (eax);
    // 005B76B2  and ecx, 0x1000200
    ecx = ((ecx) & (0x1000200));
    // 005B76B8  cmp ecx, 0x1000200
    // 005B76BE  jne 0x5b76cb
    if ((ecx) != (0x1000200)) goto LAB_005B76CB;
    // 005B76C0  and eax, 0xfefffdff
    eax = ((eax) & (0xfefffdff));
    // 005B76C5  mov dword ptr [ebx + 0x95282c], eax
    (*(int*)((char*)g_0095282c + ebx)) = (eax);
LAB_005B76CB:
    // 005B76CB  mov ecx, dword ptr [ebx + 0x95282c]
    ecx = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B76D1  and ch, 0xbf
    SETB1(ecx, ((((ecx >> 8) & 0xff)) & (0xbf)));
    // 005B76D4  mov dword ptr [ebx + 0x95282c], ecx
    (*(int*)((char*)g_0095282c + ebx)) = (ecx);
    // 005B76DA  mov al, byte ptr [ebx + 0x952839]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952839 + ebx))));
    // 005B76E0  cmp al, 0x18
    // 005B76E2  jne 0x5b777b
    if (((eax & 0xff)) != (0x18)) goto LAB_005B777B;
    // 005B76E8  jmp 0x5b7774
    goto LAB_005B7774;
LAB_005B76ED:
    // 005B76ED  test byte ptr [esi*4 + 0x96c9f8], 7
    // 005B76F5  jne 0x5b777b
    if (((((*(unsigned char*)((char*)g_0096c9f8 + esi*4))) & (7))) != 0) goto LAB_005B777B;
    // 005B76FB  mov eax, dword ptr [ebp - 0x48]
    eax = (var_48);
    // 005B76FE  test eax, eax
    // 005B7700  jne 0x5b7709
    if ((((eax) & (eax))) != 0) goto LAB_005B7709;
    // 005B7702  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B7705  test eax, eax
    // 005B7707  jne 0x5b777b
    if ((((eax) & (eax))) != 0) goto LAB_005B777B;
LAB_005B7709:
    // 005B7709  mov eax, dword ptr [ebx + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B770F  mov edx, eax
    edx = (eax);
    // 005B7711  and edx, 0x2000200
    edx = ((edx) & (0x2000200));
    // 005B7717  cmp edx, 0x2000200
    // 005B771D  jne 0x5b773a
    if ((edx) != (0x2000200)) goto LAB_005B773A;
    // 005B771F  mov cl, byte ptr [ebx + 0x95283b]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095283b + ebx))));
    // 005B7725  mov dl, byte ptr [ebx + 0x95283a]
    SETB0(edx, ((*(unsigned char*)((char*)g_0095283a + ebx))));
    // 005B772B  cmp cl, dl
    // 005B772D  jae 0x5b773a
    if (((unsigned char)(ecx & 0xff)) >= ((unsigned char)(edx & 0xff))) goto LAB_005B773A;
    // 005B772F  and eax, 0xfdfffdff
    eax = ((eax) & (0xfdfffdff));
    // 005B7734  mov dword ptr [ebx + 0x95282c], eax
    (*(int*)((char*)g_0095282c + ebx)) = (eax);
LAB_005B773A:
    // 005B773A  mov eax, dword ptr [ebx + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B7740  mov edx, eax
    edx = (eax);
    // 005B7742  and edx, 0x1000200
    edx = ((edx) & (0x1000200));
    // 005B7748  cmp edx, 0x1000200
    // 005B774E  jne 0x5b775b
    if ((edx) != (0x1000200)) goto LAB_005B775B;
    // 005B7750  and eax, 0xfefffdff
    eax = ((eax) & (0xfefffdff));
    // 005B7755  mov dword ptr [ebx + 0x95282c], eax
    (*(int*)((char*)g_0095282c + ebx)) = (eax);
LAB_005B775B:
    // 005B775B  mov ecx, dword ptr [ebx + 0x95282c]
    ecx = ((*(int*)((char*)g_0095282c + ebx)));
    // 005B7761  and ch, 0xbf
    SETB1(ecx, ((((ecx >> 8) & 0xff)) & (0xbf)));
    // 005B7764  mov dword ptr [ebx + 0x95282c], ecx
    (*(int*)((char*)g_0095282c + ebx)) = (ecx);
    // 005B776A  mov al, byte ptr [ebx + 0x952839]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952839 + ebx))));
    // 005B7770  cmp al, 0x18
    // 005B7772  jne 0x5b777b
    if (((eax & 0xff)) != (0x18)) goto LAB_005B777B;
LAB_005B7774:
    // 005B7774  mov byte ptr [ebx + 0x952839], 0
    (*(unsigned char*)((char*)g_00952839 + ebx)) = (0);
LAB_005B777B:
    // 005B777B  movsx eax, word ptr [ebx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B7782  push 0x40
    // 005B7784  push eax
    // 005B7785  call 0x5bf1f0
    eax = has_abil(eax, 0x40);
    // 005B778A  add esp, 8
    // 005B778D  test eax, eax
    // 005B778F  jne 0x5b77fd
    if ((((eax) & (eax))) != 0) goto LAB_005B77FD;
    // 005B7791  movsx ecx, word ptr [ebx + 0x952832]
    ecx = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B7798  push 4
    // 005B779A  push ecx
    // 005B779B  call 0x5bf1f0
    eax = has_abil(ecx, 4);
    // 005B77A0  add esp, 8
    // 005B77A3  test eax, eax
    // 005B77A5  jne 0x5b77fd
    if ((((eax) & (eax))) != 0) goto LAB_005B77FD;
    // 005B77A7  mov dx, word ptr [ebx + 0x952830]
    SETW0(edx, ((*(unsigned short*)((char*)g_00952830 + ebx))));
    // 005B77AE  and edx, 0x400
    edx = ((edx) & (0x400));
    // 005B77B4  test dx, dx
    // 005B77B7  jne 0x5b77fd
    if (((((edx & 0xffff)) & ((edx & 0xffff)))) != 0) goto LAB_005B77FD;
    // 005B77B9  movsx eax, word ptr [ebx + 0x95282a]
    eax = ((*(short*)((char*)g_0095282a + ebx)));
    // 005B77C0  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B77C7  movsx ecx, word ptr [ebx + 0x952828]
    ecx = ((*(short*)((char*)g_00952828 + ebx)));
    // 005B77CE  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B77D0  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B77D2  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B77D8  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B77DB  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B77DE  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B77E1  test byte ptr [eax + 8], 0x20
    // 005B77E5  je 0x5b7838
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (0x20))) == 0) goto LAB_005B7838;
    // 005B77E7  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B77E9  mov dl, byte ptr [eax]
    SETB0(edx, ((*(unsigned char*)((char*)(eax)))));
    // 005B77EB  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 005B77EE  cmp edx, 0x40
    // 005B77F1  jl 0x5b7838
    if (edx < 0x40) goto LAB_005B7838;
    // 005B77F3  cmp word ptr [ebx + 0x952832], 0x13
    // 005B77FB  je 0x5b7838
    if (((*(unsigned short*)((char*)g_00952832 + ebx))) == (0x13)) goto LAB_005B7838;
LAB_005B77FD:
    // 005B77FD  mov eax, dword ptr [ebp - 0x40]
    eax = (var_40);
    // 005B7800  test eax, eax
    // 005B7802  jne 0x5b7838
    if ((((eax) & (eax))) != 0) goto LAB_005B7838;
    // 005B7804  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005B7807  push -1
    // 005B7809  push -1
    // 005B780B  push -1
    // 005B780D  push 1
    // 005B780F  push eax
    // 005B7810  call 0x5b9580
    eax = stack_check(eax, 1, -1, -1, -1);
    // 005B7815  add esp, 0x14
    // 005B7818  cmp eax, 1
    // 005B781B  jne 0x5b7838
    if ((eax) != (1)) goto LAB_005B7838;
    // 005B781D  mov edx, dword ptr [ebp - 0x50]
    edx = (var_50);
    // 005B7820  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B7822  mov cl, byte ptr [ebx + 0x952836]
    SETB0(ecx, ((*(unsigned char*)((char*)g_00952836 + ebx))));
    // 005B7828  add ecx, edx
    ecx = ((ecx) + (edx));
    // 005B782A  test byte ptr [ecx*4 + 0x96c9f8], 8
    // 005B7832  jne 0x5b7ce2
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (8))) != 0) goto LAB_005B7CE2;
LAB_005B7838:
    // 005B7838  mov eax, dword ptr [ebp - 0x30]
    eax = (var_30);
    // 005B783B  test eax, eax
    // 005B783D  jge 0x5b7995
    if (((eax) & (eax)) >= 0) goto LAB_005B7995;
    // 005B7843  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B7846  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 005B784B  cmp edx, eax
    // 005B784D  jne 0x5b7874
    if ((edx) != (eax)) goto LAB_005B7874;
    // 005B784F  test byte ptr [0x9a64c0], 0x80
    // 005B7856  jne 0x5b7874
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B7874;
    // 005B7858  mov al, byte ptr [ebp - 0x28]
    SETB0(eax, ((var_28 & 0xff)));
    // 005B785B  mov cl, byte ptr [ebx + 0x95284f]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095284f + ebx))));
    // 005B7861  test al, cl
    // 005B7863  jne 0x5b7874
    if (((((eax & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005B7874;
    // 005B7865  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B7868  mov eax, 1
    eax = (1);
    // 005B786D  cmp ecx, eax
    // 005B786F  jg 0x5b7874
    if (ecx > eax) goto LAB_005B7874;
    // 005B7871  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005B7874:
    // 005B7874  movsx esi, word ptr [ebx + 0x95282a]
    esi = ((*(short*)((char*)g_0095282a + ebx)));
    // 005B787B  movsx edi, word ptr [ebx + 0x952828]
    edi = ((*(short*)((char*)g_00952828 + ebx)));
    // 005B7882  test esi, esi
    // 005B7884  jl 0x5b794c
    if (((esi) & (esi)) < 0) goto LAB_005B794C;
    // 005B788A  cmp esi, dword ptr [0x949874]
    // 005B7890  jge 0x5b794c
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B794C;
    // 005B7896  test edi, edi
    // 005B7898  jl 0x5b794c
    if (((edi) & (edi)) < 0) goto LAB_005B794C;
    // 005B789E  cmp edi, dword ptr [0x949870]
    // 005B78A4  jge 0x5b794c
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B794C;
    // 005B78AA  push esi
    // 005B78AB  push edi
    // 005B78AC  call 0x4712a0
    eax = on_map(edi, esi);
    // 005B78B1  add esp, 8
    // 005B78B4  test eax, eax
    // 005B78B6  je 0x5b794c
    if ((((eax) & (eax))) == 0) goto LAB_005B794C;
    // 005B78BC  mov eax, esi
    eax = (esi);
    // 005B78BE  mov ecx, edi
    ecx = (edi);
    // 005B78C0  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B78C7  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B78C9  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B78CB  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B78D1  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B78D4  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B78D7  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B78DD  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B78E0  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B78E3  cmp ecx, edx
    // 005B78E5  jne 0x5b792f
    if ((ecx) != (edx)) goto LAB_005B792F;
    // 005B78E7  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B78EA  test byte ptr [eax + 4], dl
    // 005B78ED  jne 0x5b792f
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B792F;
    // 005B78EF  test byte ptr [0x9a64c0], 0x80
    // 005B78F6  jne 0x5b792f
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B792F;
    // 005B78F8  mov edx, ecx
    edx = (ecx);
    // 005B78FA  shl edx, 6
    edx = ((edx) << (6));
    // 005B78FD  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B78FF  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B7902  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B7905  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005B7908  mov edx, dword ptr [ecx*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 005B790F  test dh, 2
    // 005B7912  jne 0x5b792f
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B792F;
    // 005B7914  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B7917  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B791D  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B7920  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B7926  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B7929  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B792F:
    // 005B792F  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B7932  mov dl, 1
    SETB0(edx, (1));
    // 005B7934  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B7936  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B7939  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B793B  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B793E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7941  push eax
    // 005B7942  push esi
    // 005B7943  push edi
    // 005B7944  call 0x591e50
    synch_bit(edi, esi, eax);
    // 005B7949  add esp, 0xc
LAB_005B794C:
    // 005B794C  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 005B794F  push ecx
    // 005B7950  call 0x579920
    eax = veh_top(ecx);
    // 005B7955  add esp, 4
    // 005B7958  test eax, eax
    // 005B795A  jl 0x5b7995
    if (((eax) & (eax)) < 0) goto LAB_005B7995;
    // 005B795C  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B795F  mov dl, 1
    SETB0(edx, (1));
    // 005B7961  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B7963:
    // 005B7963  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B7966  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B7969  shl eax, 2
    eax = ((eax) << (2));
    // 005B796C  mov cl, byte ptr [eax + 0x95284f]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B7972  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B7974  mov byte ptr [eax + 0x95284f], cl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ecx & 0xff));
    // 005B797A  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B797D  test ecx, ecx
    // 005B797F  je 0x5b798a
    if ((((ecx) & (ecx))) == 0) goto LAB_005B798A;
    // 005B7981  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B798A:
    // 005B798A  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B7991  test eax, eax
    // 005B7993  jge 0x5b7963
    if (((eax) & (eax)) >= 0) goto LAB_005B7963;
LAB_005B7995:
    // 005B7995  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B7998  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005B799D  test dl, al
    // 005B799F  je 0x5b7a53
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_005B7A53;
    // 005B79A5  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B79A8  mov ecx, dword ptr [ebp - 0x48]
    ecx = (var_48);
    // 005B79AB  cmp eax, ecx
    // 005B79AD  je 0x5b79b9
    if ((eax) == (ecx)) goto LAB_005B79B9;
    // 005B79AF  cmp dword ptr [ebp - 0x3c], 2
    // 005B79B3  jne 0x5b7af5
    if ((var_3c) != (2)) goto LAB_005B7AF5;
LAB_005B79B9:
    // 005B79B9  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 005B79BC  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005B79BF  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 005B79C2  push ecx
    // 005B79C3  push edx
    // 005B79C4  push eax
    // 005B79C5  call 0x5b5ea0
    eax = want_to_wake(eax, edx, ecx);
    // 005B79CA  add esp, 0xc
    // 005B79CD  test eax, eax
    // 005B79CF  je 0x5b7af5
    if ((((eax) & (eax))) == 0) goto LAB_005B7AF5;
    // 005B79D5  mov esi, dword ptr [ebp - 0x20]
    esi = (var_20);
    // 005B79D8  mov eax, dword ptr [esi + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + esi)));
    // 005B79DE  mov ecx, eax
    ecx = (eax);
    // 005B79E0  and ecx, 0x2000200
    ecx = ((ecx) & (0x2000200));
    // 005B79E6  cmp ecx, 0x2000200
    // 005B79EC  jne 0x5b7a09
    if ((ecx) != (0x2000200)) goto LAB_005B7A09;
    // 005B79EE  mov dl, byte ptr [esi + 0x95283b]
    SETB0(edx, ((*(unsigned char*)((char*)g_0095283b + esi))));
    // 005B79F4  mov cl, byte ptr [esi + 0x95283a]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095283a + esi))));
    // 005B79FA  cmp dl, cl
    // 005B79FC  jae 0x5b7a09
    if (((unsigned char)(edx & 0xff)) >= ((unsigned char)(ecx & 0xff))) goto LAB_005B7A09;
    // 005B79FE  and eax, 0xfdfffdff
    eax = ((eax) & (0xfdfffdff));
    // 005B7A03  mov dword ptr [esi + 0x95282c], eax
    (*(int*)((char*)g_0095282c + esi)) = (eax);
LAB_005B7A09:
    // 005B7A09  mov eax, dword ptr [esi + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + esi)));
    // 005B7A0F  mov ecx, eax
    ecx = (eax);
    // 005B7A11  and ecx, 0x1000200
    ecx = ((ecx) & (0x1000200));
    // 005B7A17  cmp ecx, 0x1000200
    // 005B7A1D  jne 0x5b7a2a
    if ((ecx) != (0x1000200)) goto LAB_005B7A2A;
    // 005B7A1F  and eax, 0xfefffdff
    eax = ((eax) & (0xfefffdff));
    // 005B7A24  mov dword ptr [esi + 0x95282c], eax
    (*(int*)((char*)g_0095282c + esi)) = (eax);
LAB_005B7A2A:
    // 005B7A2A  mov ecx, dword ptr [esi + 0x95282c]
    ecx = ((*(int*)((char*)g_0095282c + esi)));
    // 005B7A30  and ch, 0xbf
    SETB1(ecx, ((((ecx >> 8) & 0xff)) & (0xbf)));
    // 005B7A33  mov dword ptr [esi + 0x95282c], ecx
    (*(int*)((char*)g_0095282c + esi)) = (ecx);
    // 005B7A39  mov al, byte ptr [esi + 0x952839]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952839 + esi))));
    // 005B7A3F  cmp al, 0x18
    // 005B7A41  jne 0x5b7af5
    if (((eax & 0xff)) != (0x18)) goto LAB_005B7AF5;
    // 005B7A47  mov byte ptr [esi + 0x952839], 0
    (*(unsigned char*)((char*)g_00952839 + esi)) = (0);
    // 005B7A4E  jmp 0x5b7af5
    goto LAB_005B7AF5;
LAB_005B7A53:
    // 005B7A53  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005B7A56  mov eax, dword ptr [ebp - 0x50]
    eax = (var_50);
    // 005B7A59  add eax, edx
    eax = ((eax) + (edx));
    // 005B7A5B  test byte ptr [eax*4 + 0x96c9f8], 7
    // 005B7A63  jne 0x5b7af5
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (7))) != 0) goto LAB_005B7AF5;
    // 005B7A69  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B7A6C  test eax, eax
    // 005B7A6E  jne 0x5b7a7d
    if ((((eax) & (eax))) != 0) goto LAB_005B7A7D;
    // 005B7A70  mov eax, dword ptr [ebp - 0x48]
    eax = (var_48);
    // 005B7A73  test eax, eax
    // 005B7A75  je 0x5b7a7d
    if ((((eax) & (eax))) == 0) goto LAB_005B7A7D;
    // 005B7A77  cmp dword ptr [ebp - 0x3c], 2
    // 005B7A7B  jne 0x5b7af5
    if ((var_3c) != (2)) goto LAB_005B7AF5;
LAB_005B7A7D:
    // 005B7A7D  mov esi, dword ptr [ebp - 0x20]
    esi = (var_20);
    // 005B7A80  mov eax, dword ptr [esi + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + esi)));
    // 005B7A86  mov ecx, eax
    ecx = (eax);
    // 005B7A88  and ecx, 0x2000200
    ecx = ((ecx) & (0x2000200));
    // 005B7A8E  cmp ecx, 0x2000200
    // 005B7A94  jne 0x5b7ab1
    if ((ecx) != (0x2000200)) goto LAB_005B7AB1;
    // 005B7A96  mov dl, byte ptr [esi + 0x95283b]
    SETB0(edx, ((*(unsigned char*)((char*)g_0095283b + esi))));
    // 005B7A9C  mov cl, byte ptr [esi + 0x95283a]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095283a + esi))));
    // 005B7AA2  cmp dl, cl
    // 005B7AA4  jae 0x5b7ab1
    if (((unsigned char)(edx & 0xff)) >= ((unsigned char)(ecx & 0xff))) goto LAB_005B7AB1;
    // 005B7AA6  and eax, 0xfdfffdff
    eax = ((eax) & (0xfdfffdff));
    // 005B7AAB  mov dword ptr [esi + 0x95282c], eax
    (*(int*)((char*)g_0095282c + esi)) = (eax);
LAB_005B7AB1:
    // 005B7AB1  mov eax, dword ptr [esi + 0x95282c]
    eax = ((*(int*)((char*)g_0095282c + esi)));
    // 005B7AB7  mov ecx, eax
    ecx = (eax);
    // 005B7AB9  and ecx, 0x1000200
    ecx = ((ecx) & (0x1000200));
    // 005B7ABF  cmp ecx, 0x1000200
    // 005B7AC5  jne 0x5b7ad2
    if ((ecx) != (0x1000200)) goto LAB_005B7AD2;
    // 005B7AC7  and eax, 0xfefffdff
    eax = ((eax) & (0xfefffdff));
    // 005B7ACC  mov dword ptr [esi + 0x95282c], eax
    (*(int*)((char*)g_0095282c + esi)) = (eax);
LAB_005B7AD2:
    // 005B7AD2  mov ecx, dword ptr [esi + 0x95282c]
    ecx = ((*(int*)((char*)g_0095282c + esi)));
    // 005B7AD8  and ch, 0xbf
    SETB1(ecx, ((((ecx >> 8) & 0xff)) & (0xbf)));
    // 005B7ADB  mov dword ptr [esi + 0x95282c], ecx
    (*(int*)((char*)g_0095282c + esi)) = (ecx);
    // 005B7AE1  mov al, byte ptr [esi + 0x952839]
    SETB0(eax, ((*(unsigned char*)((char*)g_00952839 + esi))));
    // 005B7AE7  cmp al, 0x18
    // 005B7AE9  jne 0x5b7af5
    if (((eax & 0xff)) != (0x18)) goto LAB_005B7AF5;
    // 005B7AEB  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005B7AEE  mov byte ptr [edx + 0x952839], 0
    (*(unsigned char*)((char*)g_00952839 + edx)) = (0);
LAB_005B7AF5:
    // 005B7AF5  mov eax, dword ptr [ebp - 0x48]
    eax = (var_48);
    // 005B7AF8  test eax, eax
    // 005B7AFA  je 0x5b7b48
    if ((((eax) & (eax))) == 0) goto LAB_005B7B48;
    // 005B7AFC  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 005B7AFF  test eax, eax
    // 005B7B01  je 0x5b7b48
    if ((((eax) & (eax))) == 0) goto LAB_005B7B48;
    // 005B7B03  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
    // 005B7B06  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B7B08  test byte ptr [ecx*4 + 0x96c9f8], 7
    // 005B7B10  jne 0x5b7b48
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (7))) != 0) goto LAB_005B7B48;
    // 005B7B12  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 005B7B15  mov edi, dword ptr [ebp - 0xc]
    edi = (var_c);
    // 005B7B18  movsx edx, word ptr [ebx + 0x952832]
    edx = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B7B1F  push -1
    // 005B7B21  push esi
    // 005B7B22  push edi
    // 005B7B23  push 0x44
    // 005B7B25  push edx
    // 005B7B26  call 0x5bf1f0
    eax = has_abil(edx, 0x44);
    // 005B7B2B  add esp, 8
    // 005B7B2E..005B7B30  neg/sbb -> (eax!=0)?-1:0
    eax = ((eax) != 0 ? -1 : 0);
    // 005B7B32  neg eax
    eax = (-(eax));
    // 005B7B34  add eax, 2
    eax = ((eax) + (2));
    // 005B7B37  push eax
    // 005B7B38  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7B3B  push 6
    // 005B7B3D  push eax
    // 005B7B3E  call 0x579a30
    add_goal(eax, 6, eax, edi, esi, -1);
    // 005B7B43  add esp, 0x18
    // 005B7B46  jmp 0x5b7b4e
    goto LAB_005B7B4E;
LAB_005B7B48:
    // 005B7B48  mov edi, dword ptr [ebp - 0xc]
    edi = (var_c);
    // 005B7B4B  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
LAB_005B7B4E:
    // 005B7B4E  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
    // 005B7B51  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005B7B54  lea eax, [ecx + edx]
    eax = (edx);
    // 005B7B57  mov dword ptr [ebp - 0x50], eax
    var_50 = (eax);
    // 005B7B5A  test byte ptr [eax*4 + 0x96c9f8], 7
    // 005B7B62  jne 0x5b7b81
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (7))) != 0) goto LAB_005B7B81;
    // 005B7B64  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7B67  push -1
    // 005B7B69  mov ecx, eax
    ecx = (eax);
    // 005B7B6B  push esi
    // 005B7B6C..005B7B6E  neg/sbb -> (ecx!=0)?-1:0
    ecx = ((ecx) != 0 ? -1 : 0);
    // 005B7B70  push edi
    // 005B7B71  and ecx, 2
    ecx = ((ecx) & (2));
    // 005B7B74  inc ecx
    ecx = ((ecx) + 1);
    // 005B7B75  push ecx
    // 005B7B76  push 0
    // 005B7B78  push eax
    // 005B7B79  call 0x579a30
    add_goal(eax, 0, ecx, edi, esi, -1);
    // 005B7B7E  add esp, 0x18
LAB_005B7B81:
    // 005B7B81  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005B7B84  movsx eax, word ptr [edx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + edx)));
    // 005B7B8B  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B7B8E  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B7B91  cmp byte ptr [edx*4 + 0x9ab892], 8
    // 005B7B99  jb 0x5b7c0e
    if (((unsigned int)(*(unsigned char*)((char*)g_009ab892 + edx*4))) < ((unsigned int)8)) goto LAB_005B7C0E;
    // 005B7B9B  mov eax, dword ptr [ebp - 0x50]
    eax = (var_50);
    // 005B7B9E  test byte ptr [eax*4 + 0x96c9f8], 1
    // 005B7BA6  jne 0x5b7c0e
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (1))) != 0) goto LAB_005B7C0E;
    // 005B7BA8  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7BAB  push 0
    // 005B7BAD  push 0
    // 005B7BAF  push esi
    // 005B7BB0  push edi
    // 005B7BB1  push ecx
    // 005B7BB2  call 0x4e3ef0
    eax = whose_territory(ecx, edi, esi, 0, 0);
    // 005B7BB7  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7BBA  add esp, 0x14
    // 005B7BBD  cmp eax, ecx
    // 005B7BBF  jne 0x5b7c0e
    if ((eax) != (ecx)) goto LAB_005B7C0E;
    // 005B7BC1  movsx eax, word ptr [ebx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B7BC8  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B7BCB  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B7BCE  cmp byte ptr [eax*4 + 0x9ab892], 8
    // 005B7BD6  jb 0x5b7c0e
    if (((unsigned int)(*(unsigned char*)((char*)g_009ab892 + eax*4))) < ((unsigned int)8)) goto LAB_005B7C0E;
    // 005B7BD8  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7BDB  push -1
    // 005B7BDD  push esi
    // 005B7BDE  push edi
    // 005B7BDF  push 2
    // 005B7BE1  push 2
    // 005B7BE3  push ecx
    // 005B7BE4  call 0x579a30
    add_goal(ecx, 2, 2, edi, esi, -1);
    // 005B7BE9  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005B7BEC  push -1
    // 005B7BEE  push esi
    // 005B7BEF  push edi
    // 005B7BF0  push 1
    // 005B7BF2  push 0
    // 005B7BF4  push edx
    // 005B7BF5  call 0x579a30
    add_goal(edx, 0, 1, edi, esi, -1);
    // 005B7BFA  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 005B7BFD  push -1
    // 005B7BFF  push esi
    // 005B7C00  push edi
    // 005B7C01  push 1
    // 005B7C03  push 3
    // 005B7C05  push eax
    // 005B7C06  call 0x579a30
    add_goal(eax, 3, 1, edi, esi, -1);
    // 005B7C0B  add esp, 0x48
LAB_005B7C0E:
    // 005B7C0E  cmp dword ptr [ebp - 0x3c], 2
    // 005B7C12  jne 0x5b7c43
    if ((var_3c) != (2)) goto LAB_005B7C43;
    // 005B7C14  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B7C17  movsx eax, word ptr [ecx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + ecx)));
    // 005B7C1E  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B7C20  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B7C23  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B7C26  mov cl, byte ptr [eax*4 + 0x9ab88c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009ab88c + eax*4))));
    // 005B7C2D  mov eax, ecx
    eax = (ecx);
    // 005B7C2F  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 005B7C32  shl edx, 4
    edx = ((edx) << (4));
    // 005B7C35  mov al, byte ptr [edx + 0x94a37a]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094a37a + edx))));
    // 005B7C3B  test al, al
    // 005B7C3D  jne 0x5b7ce2
    if (((((eax & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_005B7CE2;
LAB_005B7C43:
    // 005B7C43  mov dx, word ptr [ebx + 0x952832]
    SETW0(edx, ((*(unsigned short*)((char*)g_00952832 + ebx))));
    // 005B7C4A  movsx eax, dx
    eax = (((short)(edx & 0xffff)));
    // 005B7C4D  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B7C50  lea ecx, [eax + ecx*4]
    ecx = (eax + ecx*4);
    // 005B7C53  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B7C55  shl ecx, 2
    ecx = ((ecx) << (2));
    // 005B7C58  mov al, byte ptr [ecx + 0x9ab88c]
    SETB0(eax, ((*(unsigned char*)((char*)g_009ab88c + ecx))));
    // 005B7C5E  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005B7C61  shl eax, 4
    eax = ((eax) << (4));
    // 005B7C64  cmp byte ptr [eax + 0x94a379], 2
    // 005B7C6B  jne 0x5b7c77
    if (((*(unsigned char*)((char*)g_0094a379 + eax))) != (2)) goto LAB_005B7C77;
    // 005B7C6D  mov bl, byte ptr [eax + 0x94a37a]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0094a37a + eax))));
    // 005B7C73  test bl, bl
    // 005B7C75  jne 0x5b7ce2
    if (((((ebx & 0xff)) & ((ebx & 0xff)))) != 0) goto LAB_005B7CE2;
LAB_005B7C77:
    // 005B7C77  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B7C7A  mov esi, dword ptr [ebp - 0x48]
    esi = (var_48);
    // 005B7C7D  cmp eax, esi
    // 005B7C7F  je 0x5b7c99
    if ((eax) == (esi)) goto LAB_005B7C99;
    // 005B7C81  mov esi, dword ptr [ebp - 0x30]
    esi = (var_30);
    // 005B7C84  test esi, esi
    // 005B7C86  jl 0x5b7c8c
    if (((esi) & (esi)) < 0) goto LAB_005B7C8C;
    // 005B7C88  test eax, eax
    // 005B7C8A  je 0x5b7c99
    if ((((eax) & (eax))) == 0) goto LAB_005B7C99;
LAB_005B7C8C:
    // 005B7C8C  mov eax, dword ptr [ebp - 0x50]
    eax = (var_50);
    // 005B7C8F  test byte ptr [eax*4 + 0x96c9f8], 8
    // 005B7C97  jne 0x5b7ce2
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (8))) != 0) goto LAB_005B7CE2;
LAB_005B7C99:
    // 005B7C99  cmp dx, 0x13
    // 005B7C9D  jne 0x5b7ca8
    if (((edx & 0xffff)) != (0x13)) goto LAB_005B7CA8;
    // 005B7C9F  mov dword ptr [ebp - 0x60], 1
    var_60 = (1);
    // 005B7CA6  jmp 0x5b7cd0
    goto LAB_005B7CD0;
LAB_005B7CA8:
    // 005B7CA8  cmp dx, 0x40
    // 005B7CAC  jge 0x5b7cd0
    if ((edx & 0xffff) >= 0x40) goto LAB_005B7CD0;
    // 005B7CAE  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B7CB0  mov al, byte ptr [ecx + 0x9ab88d]
    SETB0(eax, ((*(unsigned char*)((char*)g_009ab88d + ecx))));
    // 005B7CB6  shl eax, 4
    eax = ((eax) << (4));
    // 005B7CB9  mov cl, byte ptr [eax + 0x94ae68]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094ae68 + eax))));
    // 005B7CBF  test cl, cl
    // 005B7CC1  jl 0x5b7cc9
    if (((signed char)(((ecx & 0xff)) & ((ecx & 0xff)))) < 0) goto LAB_005B7CC9;
    // 005B7CC3  cmp dx, 0xf
    // 005B7CC7  jne 0x5b7cd0
    if (((edx & 0xffff)) != (0xf)) goto LAB_005B7CD0;
LAB_005B7CC9:
    // 005B7CC9  mov dword ptr [ebp - 0x5c], 1
    var_5c = (1);
LAB_005B7CD0:
    // 005B7CD0  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B7CD3  mov eax, dword ptr [ebp - 0x4c]
    eax = (var_4c);
    // 005B7CD6  mov edx, 1
    edx = (1);
    // 005B7CDB  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B7CDD  or eax, edx
    eax = ((eax) | (edx));
    // 005B7CDF  mov dword ptr [ebp - 0x4c], eax
    var_4c = (eax);
LAB_005B7CE2:
    // 005B7CE2  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005B7CE5  inc eax
    eax = ((eax) + 1);
    // 005B7CE6  cmp eax, 9
    // 005B7CE9  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005B7CEC  jl 0x5b6ee9
    if (eax < 9) goto LAB_005B6EE9;
    // 005B7CF2  mov edx, 9
    edx = (9);
    // 005B7CF7  mov dword ptr [ebp - 0x18], edx
    var_18 = (edx);
LAB_005B7CFA:
    // 005B7CFA  mov eax, dword ptr [edx*4 + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + edx*4)));
    // 005B7D01  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
    // 005B7D04  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 005B7D0A  add eax, esi
    eax = ((eax) + (esi));
    // 005B7D0C  test cl, 1
    // 005B7D0F  jne 0x5b7d26
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_005B7D26;
    // 005B7D11  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B7D17  test eax, eax
    // 005B7D19  jge 0x5b7d20
    if (((eax) & (eax)) >= 0) goto LAB_005B7D20;
    // 005B7D1B  lea esi, [eax + ecx]
    esi = (ecx);
    // 005B7D1E  jmp 0x5b7d28
    goto LAB_005B7D28;
LAB_005B7D20:
    // 005B7D20  cmp eax, ecx
    // 005B7D22  jl 0x5b7d26
    if (eax < ecx) goto LAB_005B7D26;
    // 005B7D24  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_005B7D26:
    // 005B7D26  mov esi, eax
    esi = (eax);
LAB_005B7D28:
    // 005B7D28  mov edi, dword ptr [edx*4 + 0x66f440]
    edi = ((*(int*)((char*)g_0066f440 + edx*4)));
    // 005B7D2F  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005B7D32  add edi, eax
    edi = ((edi) + (eax));
    // 005B7D34  mov dword ptr [ebp - 0xc], esi
    var_c = (esi);
    // 005B7D37  mov dword ptr [ebp - 0x14], edi
    var_14 = (edi);
    // 005B7D3A  js 0x5b89f0
    if (((edi) - (0x0)) < 0) goto LAB_005B89F0;
    // 005B7D40  cmp edi, dword ptr [0x949874]
    // 005B7D46  jge 0x5b89f0
    if (edi >= (*(int*)((char*)g_00949874))) goto LAB_005B89F0;
    // 005B7D4C  test esi, esi
    // 005B7D4E  jl 0x5b89f0
    if (((esi) & (esi)) < 0) goto LAB_005B89F0;
    // 005B7D54  cmp esi, dword ptr [0x949870]
    // 005B7D5A  jge 0x5b89f0
    if (esi >= (*(int*)((char*)g_00949870))) goto LAB_005B89F0;
    // 005B7D60  push edi
    // 005B7D61  push esi
    // 005B7D62  call 0x5bf010
    eax = is_sensor(esi, edi);
    // 005B7D67  add esp, 8
    // 005B7D6A  test eax, eax
    // 005B7D6C  je 0x5b7f77
    if ((((eax) & (eax))) == 0) goto LAB_005B7F77;
    // 005B7D72  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B7D77  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B7D7D  imul eax, edi
    eax = ((eax) * (edi));
    // 005B7D80  sar esi, 1
    esi = ((esi) >> (1));
    // 005B7D82  add eax, esi
    eax = ((eax) + (esi));
    // 005B7D84  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B7D87  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B7D8A  movsx esi, byte ptr [ecx + eax*4 + 7]
    esi = ((*(signed char*)((char*)(ecx) + eax*4 + 0x7)));
    // 005B7D8F  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B7D92  test esi, esi
    // 005B7D94  jle 0x5b7f77
    if (((esi) & (esi)) <= 0) goto LAB_005B7F77;
    // 005B7D9A  cmp esi, dword ptr [ebp - 8]
    // 005B7D9D  je 0x5b7f77
    if ((esi) == (var_8)) goto LAB_005B7F77;
    // 005B7DA3  cmp dword ptr [ebp - 0x34], 1
    // 005B7DA7  je 0x5b7f77
    if ((var_34) == (1)) goto LAB_005B7F77;
    // 005B7DAD  test byte ptr [eax + 8], 1
    // 005B7DB1  je 0x5b7dc6
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (1))) == 0) goto LAB_005B7DC6;
    // 005B7DB3  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B7DB6  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B7DB9  cmp eax, 8
    // 005B7DBC  jge 0x5b7dc6
    if (eax >= 8) goto LAB_005B7DC6;
    // 005B7DBE  test eax, eax
    // 005B7DC0  jge 0x5b7f77
    if (((eax) & (eax)) >= 0) goto LAB_005B7F77;
LAB_005B7DC6:
    // 005B7DC6  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B7DC9  test eax, eax
    // 005B7DCB  jl 0x5b7eaa
    if (((eax) & (eax)) < 0) goto LAB_005B7EAA;
    // 005B7DD1  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B7DD4  movsx edi, word ptr [eax + 0x95282a]
    edi = ((*(short*)((char*)g_0095282a + eax)));
    // 005B7DDB  movsx ebx, word ptr [eax + 0x952828]
    ebx = ((*(short*)((char*)g_00952828 + eax)));
    // 005B7DE2  test edi, edi
    // 005B7DE4  jl 0x5b7eaa
    if (((edi) & (edi)) < 0) goto LAB_005B7EAA;
    // 005B7DEA  cmp edi, dword ptr [0x949874]
    // 005B7DF0  jge 0x5b7eaa
    if (edi >= (*(int*)((char*)g_00949874))) goto LAB_005B7EAA;
    // 005B7DF6  test ebx, ebx
    // 005B7DF8  jl 0x5b7eaa
    if (((ebx) & (ebx)) < 0) goto LAB_005B7EAA;
    // 005B7DFE  cmp ebx, dword ptr [0x949870]
    // 005B7E04  jge 0x5b7eaa
    if (ebx >= (*(int*)((char*)g_00949870))) goto LAB_005B7EAA;
    // 005B7E0A  push edi
    // 005B7E0B  push ebx
    // 005B7E0C  call 0x4712a0
    eax = on_map(ebx, edi);
    // 005B7E11  add esp, 8
    // 005B7E14  test eax, eax
    // 005B7E16  je 0x5b7eaa
    if ((((eax) & (eax))) == 0) goto LAB_005B7EAA;
    // 005B7E1C  mov eax, edi
    eax = (edi);
    // 005B7E1E  mov edx, ebx
    edx = (ebx);
    // 005B7E20  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B7E27  sar edx, 1
    edx = ((edx) >> (1));
    // 005B7E29  add eax, edx
    eax = ((eax) + (edx));
    // 005B7E2B  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B7E2E  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B7E31  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B7E36  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005B7E3C  cmp esi, ecx
    // 005B7E3E  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B7E41  jne 0x5b7e91
    if ((esi) != (ecx)) goto LAB_005B7E91;
    // 005B7E43  mov edx, 1
    edx = (1);
    // 005B7E48  mov ecx, esi
    ecx = (esi);
    // 005B7E4A  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B7E4C  test byte ptr [eax + 4], dl
    // 005B7E4F  jne 0x5b7e91
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B7E91;
    // 005B7E51  test byte ptr [0x9a64c0], 0x80
    // 005B7E58  jne 0x5b7e91
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B7E91;
    // 005B7E5A  mov ecx, esi
    ecx = (esi);
    // 005B7E5C  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B7E5F  add ecx, esi
    ecx = ((ecx) + (esi));
    // 005B7E61  lea edx, [esi + ecx*2]
    edx = (esi + ecx*2);
    // 005B7E64  lea ecx, [esi + edx*8]
    ecx = (esi + edx*8);
    // 005B7E67  lea edx, [esi + ecx*2]
    edx = (esi + ecx*2);
    // 005B7E6A  mov ecx, dword ptr [edx*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx*4)));
    // 005B7E71  test ch, 2
    // 005B7E74  jne 0x5b7e91
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B7E91;
    // 005B7E76  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B7E79  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B7E7F  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B7E82  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B7E88  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B7E8B  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B7E91:
    // 005B7E91  mov dl, 1
    SETB0(edx, (1));
    // 005B7E93  mov ecx, esi
    ecx = (esi);
    // 005B7E95  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B7E97  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B7E9A  push esi
    // 005B7E9B  push edi
    // 005B7E9C  push ebx
    // 005B7E9D  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B7E9F  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B7EA2  call 0x591e50
    synch_bit(ebx, edi, esi);
    // 005B7EA7  add esp, 0xc
LAB_005B7EAA:
    // 005B7EAA  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005B7EAD  test eax, eax
    // 005B7EAF  jl 0x5b7f0e
    if (((eax) & (eax)) < 0) goto LAB_005B7F0E;
    // 005B7EB1  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B7EB4  mov cx, word ptr [ecx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx))));
    // 005B7EBB  test cx, cx
    // 005B7EBE  jl 0x5b7ed6
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B7ED6;
LAB_005B7EC0:
    // 005B7EC0  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B7EC3  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B7EC6  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B7EC9  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B7ED1  test cx, cx
    // 005B7ED4  jge 0x5b7ec0
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B7EC0;
LAB_005B7ED6:
    // 005B7ED6  test eax, eax
    // 005B7ED8  jl 0x5b7f0e
    if (((eax) & (eax)) < 0) goto LAB_005B7F0E;
    // 005B7EDA  mov dl, 1
    SETB0(edx, (1));
    // 005B7EDC  mov ecx, esi
    ecx = (esi);
    // 005B7EDE  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B7EE0  mov ecx, 0xfffffbbf
    ecx = (0xfffffbbf);
LAB_005B7EE5:
    // 005B7EE5  lea edi, [eax + eax*2]
    edi = (eax + eax*2);
    // 005B7EE8  lea eax, [eax + edi*4]
    eax = (eax + edi*4);
    // 005B7EEB  shl eax, 2
    eax = ((eax) << (2));
    // 005B7EEE  mov bl, byte ptr [eax + 0x95284f]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B7EF4  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B7EF6  mov byte ptr [eax + 0x95284f], bl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ebx & 0xff));
    // 005B7EFC  and word ptr [eax + 0x952830], cx
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & ((ecx & 0xffff)));
    // 005B7F03  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B7F0A  test eax, eax
    // 005B7F0C  jge 0x5b7ee5
    if (((eax) & (eax)) >= 0) goto LAB_005B7EE5;
LAB_005B7F0E:
    // 005B7F0E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B7F11  mov edx, eax
    edx = (eax);
    // 005B7F13  shl edx, 6
    edx = ((edx) << (6));
    // 005B7F16  add edx, eax
    edx = ((edx) + (eax));
    // 005B7F18  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 005B7F1B  lea edx, [eax + ecx*8]
    edx = (eax + ecx*8);
    // 005B7F1E  add eax, esi
    eax = ((eax) + (esi));
    // 005B7F20  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 005B7F23  test byte ptr [ecx*4 + 0x96c9f8], 7
    // 005B7F2B  jne 0x5b7f77
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (7))) != 0) goto LAB_005B7F77;
    // 005B7F2D  mov eax, dword ptr [ebp + 0xc]
    eax = (a2);
    // 005B7F30  test eax, eax
    // 005B7F32  je 0x5b7f77
    if ((((eax) & (eax))) == 0) goto LAB_005B7F77;
    // 005B7F34  mov eax, dword ptr [ebp - 0x3c]
    eax = (var_3c);
    // 005B7F37  push -1
    // 005B7F39  test eax, eax
    // 005B7F3B  jne 0x5b7f62
    if ((((eax) & (eax))) != 0) goto LAB_005B7F62;
    // 005B7F3D  mov edi, dword ptr [ebp - 0x14]
    edi = (var_14);
    // 005B7F40  mov ebx, dword ptr [ebp - 0xc]
    ebx = (var_c);
    // 005B7F43  push edi
    // 005B7F44  push ebx
    // 005B7F45  push 2
    // 005B7F47  push 0
    // 005B7F49  push esi
    // 005B7F4A  call 0x579a30
    add_goal(esi, 0, 2, ebx, edi, -1);
    // 005B7F4F  push -1
    // 005B7F51  push edi
    // 005B7F52  push ebx
    // 005B7F53  push 1
    // 005B7F55  push 2
    // 005B7F57  push esi
    // 005B7F58  call 0x579a30
    add_goal(esi, 2, 1, ebx, edi, -1);
    // 005B7F5D  add esp, 0x30
    // 005B7F60  jmp 0x5b7f77
    goto LAB_005B7F77;
LAB_005B7F62:
    // 005B7F62  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 005B7F65  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B7F68  push edx
    // 005B7F69  push eax
    // 005B7F6A  push 2
    // 005B7F6C  push 6
    // 005B7F6E  push esi
    // 005B7F6F  call 0x579a30
    add_goal(esi, 6, 2, eax, edx, -0x1);
    // 005B7F74  add esp, 0x18
LAB_005B7F77:
    // 005B7F77  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B7F7C  mov esi, dword ptr [ebp - 0xc]
    esi = (var_c);
    // 005B7F7F  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B7F83  mov ecx, esi
    ecx = (esi);
    // 005B7F85  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B7F87  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B7F89  mov dword ptr [ebp - 0x50], ecx
    var_50 = (ecx);
    // 005B7F8C  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B7F8F  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B7F92  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B7F97  mov cl, byte ptr [eax + edx*4 + 8]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + edx*4 + 0x8))));
    // 005B7F9B  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B7F9E  test cl, 3
    // 005B7FA1  je 0x5b7fae
    if (((((ecx & 0xff)) & (3))) == 0) goto LAB_005B7FAE;
    // 005B7FA3  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B7FA6  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B7FA9  cmp eax, 8
    // 005B7FAC  jl 0x5b7fb1
    if (eax < 8) goto LAB_005B7FB1;
LAB_005B7FAE:
    // 005B7FAE  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
LAB_005B7FB1:
    // 005B7FB1  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
    // 005B7FB4  mov dword ptr [ebp - 0x10], eax
    var_10 = (eax);
    // 005B7FB7  cmp eax, edi
    // 005B7FB9  je 0x5b89f0
    if ((eax) == (edi)) goto LAB_005B89F0;
    // 005B7FBF  mov ebx, dword ptr [ebp - 0x14]
    ebx = (var_14);
    // 005B7FC2  push ebx
    // 005B7FC3  push esi
    // 005B7FC4  call 0x5bfe90
    eax = veh_at(esi, ebx);
    // 005B7FC9  push eax
    // 005B7FCA  call 0x5b8e10
    eax = stack_fix(eax);
    // 005B7FCF  push ebx
    // 005B7FD0  push esi
    // 005B7FD1  mov dword ptr [ebp - 0x2c], eax
    var_2c = (eax);
    // 005B7FD4  call 0x4e3a50
    eax = base_at(esi, ebx);
    // 005B7FD9  mov ebx, eax
    ebx = (eax);
    // 005B7FDB  mov eax, dword ptr [ebp - 0x40]
    eax = (var_40);
    // 005B7FDE  add esp, 0x14
    // 005B7FE1  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
    // 005B7FE4  test eax, eax
    // 005B7FE6  je 0x5b8400
    if ((((eax) & (eax))) == 0) goto LAB_005B8400;
    // 005B7FEC  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005B7FEF  cmp ecx, 2
    // 005B7FF2  je 0x5b802e
    if ((ecx) == (2)) goto LAB_005B802E;
    // 005B7FF4  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B7FF9  mov edx, dword ptr [ebp - 0x50]
    edx = (var_50);
    // 005B7FFC  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B8000  mov esi, dword ptr [0x94a30c]
    esi = ((*(int*)((char*)g_0094a30c)));
    // 005B8006  add eax, edx
    eax = ((eax) + (edx));
    // 005B8008  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B800B  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B800E  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B8010  mov dl, byte ptr [esi + eax*4]
    SETB0(edx, ((*(unsigned char*)((char*)(esi) + eax*4))));
    // 005B8013  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B8015  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 005B8018  cmp edx, 0x60
    // 005B801B  setl al
    SETB0(eax, ((edx < 0x60) ? 1 : 0));
    // 005B801E  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005B8020  cmp ecx, 1
    // 005B8023  sete dl
    SETB0(edx, (((ecx) == (1)) ? 1 : 0));
    // 005B8026  cmp eax, edx
    // 005B8028  jne 0x5b8403
    if ((eax) != (edx)) goto LAB_005B8403;
LAB_005B802E:
    // 005B802E  test ebx, ebx
    // 005B8030  jl 0x5b8183
    if (((ebx) & (ebx)) < 0) goto LAB_005B8183;
    // 005B8036  cmp edi, dword ptr [0x939284]
    // 005B803C  jne 0x5b808f
    if ((edi) != ((*(int*)((char*)g_00939284)))) goto LAB_005B808F;
    // 005B803E  test byte ptr [0x9a64c0], 0x80
    // 005B8045  jne 0x5b808f
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B808F;
    // 005B8047  lea eax, [ebx + ebx*8]
    eax = (ebx + ebx*8);
    // 005B804A  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B804D  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 005B8050  lea eax, [ebx + ecx*4]
    eax = (ebx + ecx*4);
    // 005B8053  shl eax, 2
    eax = ((eax) << (2));
    // 005B8056  test byte ptr [eax + 0x97d04a], dl
    // 005B805C  jne 0x5b806c
    if (((((*(unsigned char*)((char*)g_0097d04a + eax))) & ((edx & 0xff)))) != 0) goto LAB_005B806C;
    // 005B805E  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B8061  mov eax, 3
    eax = (3);
    // 005B8066  cmp ecx, eax
    // 005B8068  jg 0x5b808f
    if (ecx > eax) goto LAB_005B808F;
    // 005B806A  jmp 0x5b808c
    goto LAB_005B808C;
LAB_005B806C:
    // 005B806C  movsx edx, byte ptr [eax + 0x97d046]
    edx = ((*(signed char*)((char*)g_0097d046 + eax)));
    // 005B8073  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B8075  mov cl, byte ptr [eax + edi + 0x97d04b]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d04b + edi))));
    // 005B807C  cmp ecx, edx
    // 005B807E  je 0x5b808f
    if ((ecx) == (edx)) goto LAB_005B808F;
    // 005B8080  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B8083  mov eax, 2
    eax = (2);
    // 005B8088  cmp ecx, eax
    // 005B808A  jg 0x5b808f
    if (ecx > eax) goto LAB_005B808F;
LAB_005B808C:
    // 005B808C  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005B808F:
    // 005B808F  lea eax, [ebx + ebx*8]
    eax = (ebx + ebx*8);
    // 005B8092  mov dl, 1
    SETB0(edx, (1));
    // 005B8094  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 005B8097  lea eax, [ebx + ecx*4]
    eax = (ebx + ecx*4);
    // 005B809A  mov ecx, edi
    ecx = (edi);
    // 005B809C  shl eax, 2
    eax = ((eax) << (2));
    // 005B809F  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B80A1  mov cl, byte ptr [eax + 0x97d04a]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d04a + eax))));
    // 005B80A7  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B80A9  mov byte ptr [eax + 0x97d04a], cl
    (*(unsigned char*)((char*)g_0097d04a + eax)) = ((ecx & 0xff));
    // 005B80AF  mov cl, byte ptr [eax + 0x97d046]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d046 + eax))));
    // 005B80B5  mov byte ptr [eax + edi + 0x97d04b], cl
    (*(unsigned char*)((char*)g_0097d04b + edi)) = ((ecx & 0xff));
    // 005B80BC  movsx ecx, word ptr [eax + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + eax)));
    // 005B80C3  movsx esi, word ptr [eax + 0x97d040]
    esi = ((*(short*)((char*)g_0097d040 + eax)));
    // 005B80CA  test ecx, ecx
    // 005B80CC  jl 0x5b8183
    if (((ecx) & (ecx)) < 0) goto LAB_005B8183;
    // 005B80D2  cmp ecx, dword ptr [0x949874]
    // 005B80D8  jge 0x5b8183
    if (ecx >= (*(int*)((char*)g_00949874))) goto LAB_005B8183;
    // 005B80DE  test esi, esi
    // 005B80E0  jl 0x5b8183
    if (((esi) & (esi)) < 0) goto LAB_005B8183;
    // 005B80E6  cmp esi, dword ptr [0x949870]
    // 005B80EC  jge 0x5b8183
    if (esi >= (*(int*)((char*)g_00949870))) goto LAB_005B8183;
    // 005B80F2  mov eax, ecx
    eax = (ecx);
    // 005B80F4  mov ebx, esi
    ebx = (esi);
    // 005B80F6  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B80FD  sar ebx, 1
    ebx = ((ebx) >> (1));
    // 005B80FF  add eax, ebx
    eax = ((eax) + (ebx));
    // 005B8101  lea ebx, [eax + eax*4]
    ebx = (eax + eax*4);
    // 005B8104  lea eax, [eax + ebx*2]
    eax = (eax + ebx*2);
    // 005B8107  mov ebx, dword ptr [0x94a30c]
    ebx = ((*(int*)((char*)g_0094a30c)));
    // 005B810D  lea eax, [ebx + eax*4]
    eax = (ebx + eax*4);
    // 005B8110  mov ebx, dword ptr [0x939284]
    ebx = ((*(int*)((char*)g_00939284)));
    // 005B8116  cmp edi, ebx
    // 005B8118  jne 0x5b8167
    if ((edi) != (ebx)) goto LAB_005B8167;
    // 005B811A  mov bl, byte ptr [ebp - 0x28]
    SETB0(ebx, ((var_28 & 0xff)));
    // 005B811D  test byte ptr [eax + 4], bl
    // 005B8120  jne 0x5b8167
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((ebx & 0xff)))) != 0) goto LAB_005B8167;
    // 005B8122  mov bl, byte ptr [0x9a64c0]
    SETB0(ebx, ((*(unsigned char*)((char*)g_009a64c0))));
    // 005B8128  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
    // 005B812B  test bl, 0x80
    // 005B812E  jne 0x5b816a
    if (((((ebx & 0xff)) & (0x80))) != 0) goto LAB_005B816A;
    // 005B8130  mov ebx, edi
    ebx = (edi);
    // 005B8132  shl ebx, 6
    ebx = ((ebx) << (6));
    // 005B8135  add ebx, edi
    ebx = ((ebx) + (edi));
    // 005B8137  lea ebx, [edi + ebx*2]
    ebx = (edi + ebx*2);
    // 005B813A  lea ebx, [edi + ebx*8]
    ebx = (edi + ebx*8);
    // 005B813D  lea ebx, [edi + ebx*2]
    ebx = (edi + ebx*2);
    // 005B8140  test byte ptr [ebx*4 + 0x96c9e1], 2
    // 005B8148  jne 0x5b816a
    if (((((*(unsigned char*)((char*)g_0096c9e1 + ebx*4))) & (2))) != 0) goto LAB_005B816A;
    // 005B814A  mov ebx, dword ptr [eax + 0xc]
    ebx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B814D  or ebx, 0x400000
    ebx = ((ebx) | (0x400000));
    // 005B8153  mov dword ptr [eax + 0xc], ebx
    (*(int*)((char*)(eax) + 0xc)) = (ebx);
    // 005B8156  mov ebx, dword ptr [0x90d91c]
    ebx = ((*(int*)((char*)g_0090d91c)));
    // 005B815C  or ebx, 1
    ebx = ((ebx) | (1));
    // 005B815F  mov dword ptr [0x90d91c], ebx
    (*(int*)((char*)g_0090d91c)) = (ebx);
    // 005B8165  jmp 0x5b816a
    goto LAB_005B816A;
LAB_005B8167:
    // 005B8167  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
LAB_005B816A:
    // 005B816A  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B816D  push edi
    // 005B816E  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B8170  push ecx
    // 005B8171  push esi
    // 005B8172  mov byte ptr [eax + 4], bl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ebx & 0xff));
    // 005B8175  call 0x591e50
    synch_bit(esi, ecx, edi);
    // 005B817A  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
    // 005B817D  mov ebx, dword ptr [ebp - 0x30]
    ebx = (var_30);
    // 005B8180  add esp, 0xc
LAB_005B8183:
    // 005B8183  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005B8186  test eax, eax
    // 005B8188  jl 0x5b86a6
    if (((eax) & (eax)) < 0) goto LAB_005B86A6;
    // 005B818E  test ebx, ebx
    // 005B8190  jge 0x5b8403
    if (((ebx) & (ebx)) >= 0) goto LAB_005B8403;
    // 005B8196  mov ebx, eax
    ebx = (eax);
    // 005B8198  push 0x40
    // 005B819A  lea ecx, [ebx + ebx*2]
    ecx = (ebx + ebx*2);
    // 005B819D  lea ebx, [ebx + ecx*4]
    ebx = (ebx + ecx*4);
    // 005B81A0  shl ebx, 2
    ebx = ((ebx) << (2));
    // 005B81A3  movsx edx, word ptr [ebx + 0x952832]
    edx = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B81AA  push edx
    // 005B81AB  call 0x5bf1f0
    eax = has_abil(edx, 0x40);
    // 005B81B0  add esp, 8
    // 005B81B3  test eax, eax
    // 005B81B5  jne 0x5b8400
    if ((((eax) & (eax))) != 0) goto LAB_005B8400;
    // 005B81BB  movsx eax, word ptr [ebx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + ebx)));
    // 005B81C2  push 4
    // 005B81C4  push eax
    // 005B81C5  call 0x5bf1f0
    eax = has_abil(eax, 4);
    // 005B81CA  add esp, 8
    // 005B81CD  test eax, eax
    // 005B81CF  jne 0x5b8400
    if ((((eax) & (eax))) != 0) goto LAB_005B8400;
    // 005B81D5  mov cx, word ptr [ebx + 0x952830]
    SETW0(ecx, ((*(unsigned short*)((char*)g_00952830 + ebx))));
    // 005B81DC  and ecx, 0x400
    ecx = ((ecx) & (0x400));
    // 005B81E2  test cx, cx
    // 005B81E5  jne 0x5b8400
    if (((((ecx & 0xffff)) & ((ecx & 0xffff)))) != 0) goto LAB_005B8400;
    // 005B81EB  movsx esi, word ptr [ebx + 0x95282a]
    esi = ((*(short*)((char*)g_0095282a + ebx)));
    // 005B81F2  movsx edi, word ptr [ebx + 0x952828]
    edi = ((*(short*)((char*)g_00952828 + ebx)));
    // 005B81F9  mov eax, esi
    eax = (esi);
    // 005B81FB  mov edx, edi
    edx = (edi);
    // 005B81FD  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B8204  sar edx, 1
    edx = ((edx) >> (1));
    // 005B8206  add eax, edx
    eax = ((eax) + (edx));
    // 005B8208  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B820B  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B820E  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B8213  mov cl, byte ptr [eax + edx*4 + 8]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + edx*4 + 0x8))));
    // 005B8217  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B821A  test cl, 0x20
    // 005B821D  je 0x5b8239
    if (((((ecx & 0xff)) & (0x20))) == 0) goto LAB_005B8239;
    // 005B821F  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B8221  mov cl, byte ptr [eax]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax)))));
    // 005B8223  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B8226  cmp ecx, 0x40
    // 005B8229  jl 0x5b8239
    if (ecx < 0x40) goto LAB_005B8239;
    // 005B822B  cmp word ptr [ebx + 0x952832], 0x13
    // 005B8233  jne 0x5b83fd
    if (((*(unsigned short*)((char*)g_00952832 + ebx))) != (0x13)) goto LAB_005B83FD;
LAB_005B8239:
    // 005B8239  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B823C  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 005B8241  cmp edx, eax
    // 005B8243  jne 0x5b826b
    if ((edx) != (eax)) goto LAB_005B826B;
    // 005B8245  test byte ptr [0x9a64c0], 0x80
    // 005B824C  jne 0x5b826b
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B826B;
    // 005B824E  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B8251  mov al, byte ptr [ebp - 0x28]
    SETB0(eax, ((var_28 & 0xff)));
    // 005B8254  test byte ptr [ecx + 0x95284f], al
    // 005B825A  jne 0x5b826b
    if (((((*(unsigned char*)((char*)g_0095284f + ecx))) & ((eax & 0xff)))) != 0) goto LAB_005B826B;
    // 005B825C  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005B825F  mov eax, 2
    eax = (2);
    // 005B8264  cmp ecx, eax
    // 005B8266  jg 0x5b826b
    if (ecx > eax) goto LAB_005B826B;
    // 005B8268  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005B826B:
    // 005B826B  test esi, esi
    // 005B826D  jl 0x5b8334
    if (((esi) & (esi)) < 0) goto LAB_005B8334;
    // 005B8273  cmp esi, dword ptr [0x949874]
    // 005B8279  jge 0x5b8334
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B8334;
    // 005B827F  test edi, edi
    // 005B8281  jl 0x5b8334
    if (((edi) & (edi)) < 0) goto LAB_005B8334;
    // 005B8287  cmp edi, dword ptr [0x949870]
    // 005B828D  jge 0x5b8334
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B8334;
    // 005B8293  push esi
    // 005B8294  push edi
    // 005B8295  call 0x4712a0
    eax = on_map(edi, esi);
    // 005B829A  add esp, 8
    // 005B829D  test eax, eax
    // 005B829F  je 0x5b8334
    if ((((eax) & (eax))) == 0) goto LAB_005B8334;
    // 005B82A5  mov eax, esi
    eax = (esi);
    // 005B82A7  mov edx, edi
    edx = (edi);
    // 005B82A9  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B82B0  sar edx, 1
    edx = ((edx) >> (1));
    // 005B82B2  add eax, edx
    eax = ((eax) + (edx));
    // 005B82B4  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B82B7  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B82BA  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B82BF  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B82C2  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B82C5  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B82CB  cmp ecx, edx
    // 005B82CD  jne 0x5b8317
    if ((ecx) != (edx)) goto LAB_005B8317;
    // 005B82CF  mov dl, byte ptr [ebp - 0x28]
    SETB0(edx, ((var_28 & 0xff)));
    // 005B82D2  test byte ptr [eax + 4], dl
    // 005B82D5  jne 0x5b8317
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) != 0) goto LAB_005B8317;
    // 005B82D7  test byte ptr [0x9a64c0], 0x80
    // 005B82DE  jne 0x5b8317
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B8317;
    // 005B82E0  mov edx, ecx
    edx = (ecx);
    // 005B82E2  shl edx, 6
    edx = ((edx) << (6));
    // 005B82E5  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B82E7  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B82EA  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B82ED  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005B82F0  mov edx, dword ptr [ecx*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 005B82F7  test dh, 2
    // 005B82FA  jne 0x5b8317
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B8317;
    // 005B82FC  mov edx, dword ptr [eax + 0xc]
    edx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B82FF  or edx, 0x400000
    edx = ((edx) | (0x400000));
    // 005B8305  mov dword ptr [eax + 0xc], edx
    (*(int*)((char*)(eax) + 0xc)) = (edx);
    // 005B8308  mov ecx, dword ptr [0x90d91c]
    ecx = ((*(int*)((char*)g_0090d91c)));
    // 005B830E  or ecx, 1
    ecx = ((ecx) | (1));
    // 005B8311  mov dword ptr [0x90d91c], ecx
    (*(int*)((char*)g_0090d91c)) = (ecx);
LAB_005B8317:
    // 005B8317  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B831A  mov dl, 1
    SETB0(edx, (1));
    // 005B831C  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B831E  mov cl, byte ptr [eax + 4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B8321  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B8323  mov byte ptr [eax + 4], cl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ecx & 0xff));
    // 005B8326  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B8329  push eax
    // 005B832A  push esi
    // 005B832B  push edi
    // 005B832C  call 0x591e50
    synch_bit(edi, esi, eax);
    // 005B8331  add esp, 0xc
LAB_005B8334:
    // 005B8334  mov cx, word ptr [ebx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ebx))));
    // 005B833B  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005B833E  test cx, cx
    // 005B8341  jl 0x5b8359
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B8359;
LAB_005B8343:
    // 005B8343  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B8346  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B8349  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B834C  mov cx, word ptr [edx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + edx*4))));
    // 005B8354  test cx, cx
    // 005B8357  jge 0x5b8343
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B8343;
LAB_005B8359:
    // 005B8359  test eax, eax
    // 005B835B  jl 0x5b8396
    if (((eax) & (eax)) < 0) goto LAB_005B8396;
    // 005B835D  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B8360  mov dl, 1
    SETB0(edx, (1));
    // 005B8362  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B8364:
    // 005B8364  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B8367  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B836A  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005B836D  shl eax, 2
    eax = ((eax) << (2));
    // 005B8370  mov bl, byte ptr [eax + 0x95284f]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B8376  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B8378  test ecx, ecx
    // 005B837A  mov byte ptr [eax + 0x95284f], bl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ebx & 0xff));
    // 005B8380  je 0x5b838b
    if ((((ecx) & (ecx))) == 0) goto LAB_005B838B;
    // 005B8382  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B838B:
    // 005B838B  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B8392  test eax, eax
    // 005B8394  jge 0x5b8364
    if (((eax) & (eax)) >= 0) goto LAB_005B8364;
LAB_005B8396:
    // 005B8396  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B839B  mov esi, dword ptr [ebp - 0x50]
    esi = (var_50);
    // 005B839E  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B83A2  add eax, esi
    eax = ((eax) + (esi));
    // 005B83A4  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B83A6  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B83A9  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B83AC  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005B83B2  mov cl, byte ptr [edx + eax*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(edx) + eax*4))));
    // 005B83B5  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B83B8  cmp ecx, 0x60
    // 005B83BB  jge 0x5b83fd
    if (ecx >= 0x60) goto LAB_005B83FD;
    // 005B83BD  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B83C0  test ecx, ecx
    // 005B83C2  je 0x5b83fd
    if ((((ecx) & (ecx))) == 0) goto LAB_005B83FD;
    // 005B83C4  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B83C7  mov edx, eax
    edx = (eax);
    // 005B83C9  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B83CB  shl edx, 6
    edx = ((edx) << (6));
    // 005B83CE  add edx, eax
    edx = ((edx) + (eax));
    // 005B83D0  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 005B83D3  lea edx, [eax + edx*8]
    edx = (eax + edx*8);
    // 005B83D6  lea eax, [ecx + edx*2]
    eax = (ecx + edx*2);
    // 005B83D9  test byte ptr [eax*4 + 0x96c9f8], 2
    // 005B83E1  jne 0x5b83fd
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (2))) != 0) goto LAB_005B83FD;
    // 005B83E3  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 005B83E6  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005B83E9  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B83EC  push -1
    // 005B83EE  push ecx
    // 005B83EF  push edx
    // 005B83F0  push 2
    // 005B83F2  push 6
    // 005B83F4  push eax
    // 005B83F5  call 0x579a30
    add_goal(eax, 6, 2, edx, ecx, -1);
    // 005B83FA  add esp, 0x18
LAB_005B83FD:
    // 005B83FD  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
LAB_005B8400:
    // 005B8400  mov ebx, dword ptr [ebp - 0x30]
    ebx = (var_30);
LAB_005B8403:
    // 005B8403  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005B8406  test eax, eax
    // 005B8408  jl 0x5b86a6
    if (((eax) & (eax)) < 0) goto LAB_005B86A6;
    // 005B840E  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 005B8411  test eax, eax
    // 005B8413  jne 0x5b86a6
    if ((((eax) & (eax))) != 0) goto LAB_005B86A6;
    // 005B8419  mov esi, dword ptr [ebp - 0x2c]
    esi = (var_2c);
    // 005B841C  push 2
    // 005B841E  lea ecx, [esi + esi*2]
    ecx = (esi + esi*2);
    // 005B8421  lea esi, [esi + ecx*4]
    esi = (esi + ecx*4);
    // 005B8424  movsx edx, word ptr [esi*4 + 0x952832]
    edx = ((*(short*)((char*)g_00952832 + esi*4)));
    // 005B842C  push edx
    // 005B842D  call 0x5bf1f0
    eax = has_abil(edx, 2);
    // 005B8432  add esp, 8
    // 005B8435  test eax, eax
    // 005B8437  je 0x5b86a6
    if ((((eax) & (eax))) == 0) goto LAB_005B86A6;
    // 005B843D  movsx eax, word ptr [esi*4 + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + esi*4)));
    // 005B8445  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B8448  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B844B  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B844D  mov al, byte ptr [edx*4 + 0x9ab88c]
    SETB0(eax, ((*(unsigned char*)((char*)g_009ab88c + edx*4))));
    // 005B8454  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005B8457  shl eax, 4
    eax = ((eax) << (4));
    // 005B845A  mov al, byte ptr [eax + 0x94a379]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094a379 + eax))));
    // 005B8460  cmp al, 2
    // 005B8462  je 0x5b8476
    if (((eax & 0xff)) == (2)) goto LAB_005B8476;
    // 005B8464  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B8466  cmp al, 1
    // 005B8468  mov eax, dword ptr [ebp - 0x44]
    eax = (var_44);
    // 005B846B  sete cl
    SETB0(ecx, ((((eax & 0xff)) == (1)) ? 1 : 0));
    // 005B846E  cmp eax, ecx
    // 005B8470  jne 0x5b86a6
    if ((eax) != (ecx)) goto LAB_005B86A6;
LAB_005B8476:
    // 005B8476  mov edx, edi
    edx = (edi);
    // 005B8478  shl edx, 6
    edx = ((edx) << (6));
    // 005B847B  add edx, edi
    edx = ((edx) + (edi));
    // 005B847D  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 005B8480  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005B8483  add edx, edi
    edx = ((edx) + (edi));
    // 005B8485  lea ecx, [edi + eax*8]
    ecx = (edi + eax*8);
    // 005B8488  lea eax, [edx + ecx*2]
    eax = (edx + ecx*2);
    // 005B848B  test byte ptr [eax*4 + 0x96c9f8], 1
    // 005B8493  jne 0x5b84a8
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (1))) != 0) goto LAB_005B84A8;
    // 005B8495  mov eax, dword ptr [ebp + 0xc]
    eax = (a2);
    // 005B8498  test eax, eax
    // 005B849A  je 0x5b84a8
    if ((((eax) & (eax))) == 0) goto LAB_005B84A8;
    // 005B849C  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 005B849F  push ecx
    // 005B84A0  call 0x5b6060
    wake_stack(ecx);
    // 005B84A5  add esp, 4
LAB_005B84A8:
    // 005B84A8  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B84AD  mov edx, dword ptr [ebp - 0x54]
    edx = (var_54);
    // 005B84B0  imul eax, dword ptr [ebp - 0x24]
    eax = ((eax) * (var_24));
    // 005B84B4  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B84BA  add eax, edx
    eax = ((eax) + (edx));
    // 005B84BC  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B84BF  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B84C2  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B84C5  test byte ptr [eax + 8], 1
    // 005B84C9  je 0x5b84de
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (1))) == 0) goto LAB_005B84DE;
    // 005B84CB  mov al, byte ptr [eax + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(eax) + 0x2))));
    // 005B84CE  and eax, 0xf
    eax = ((eax) & (0xf));
    // 005B84D1  cmp eax, 8
    // 005B84D4  jge 0x5b84de
    if (eax >= 8) goto LAB_005B84DE;
    // 005B84D6  test eax, eax
    // 005B84D8  jge 0x5b863f
    if (((eax) & (eax)) >= 0) goto LAB_005B863F;
LAB_005B84DE:
    // 005B84DE  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005B84E1  test edx, edx
    // 005B84E3  jl 0x5b85d7
    if (((edx) & (edx)) < 0) goto LAB_005B85D7;
    // 005B84E9  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B84EC  movsx esi, word ptr [eax + 0x95282a]
    esi = ((*(short*)((char*)g_0095282a + eax)));
    // 005B84F3  movsx edi, word ptr [eax + 0x952828]
    edi = ((*(short*)((char*)g_00952828 + eax)));
    // 005B84FA  test esi, esi
    // 005B84FC  jl 0x5b85d7
    if (((esi) & (esi)) < 0) goto LAB_005B85D7;
    // 005B8502  mov eax, dword ptr [0x949874]
    eax = ((*(int*)((char*)g_00949874)));
    // 005B8507  cmp esi, eax
    // 005B8509  jge 0x5b85d7
    if (esi >= eax) goto LAB_005B85D7;
    // 005B850F  test edi, edi
    // 005B8511  jl 0x5b85d7
    if (((edi) & (edi)) < 0) goto LAB_005B85D7;
    // 005B8517  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005B851D  cmp edi, ecx
    // 005B851F  jge 0x5b85d7
    if (edi >= ecx) goto LAB_005B85D7;
    // 005B8525  test esi, esi
    // 005B8527  jl 0x5b85d7
    if (((esi) & (esi)) < 0) goto LAB_005B85D7;
    // 005B852D  cmp esi, eax
    // 005B852F  jge 0x5b85d7
    if (esi >= eax) goto LAB_005B85D7;
    // 005B8535  test edi, edi
    // 005B8537  jl 0x5b85d7
    if (((edi) & (edi)) < 0) goto LAB_005B85D7;
    // 005B853D  cmp edi, ecx
    // 005B853F  jge 0x5b85d7
    if (edi >= ecx) goto LAB_005B85D7;
    // 005B8545  mov eax, esi
    eax = (esi);
    // 005B8547  mov edx, edi
    edx = (edi);
    // 005B8549  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B8550  sar edx, 1
    edx = ((edx) >> (1));
    // 005B8552  add eax, edx
    eax = ((eax) + (edx));
    // 005B8554  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B8557  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B855A  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B855F  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B8562  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B8565  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B856B  cmp ecx, edx
    // 005B856D  jne 0x5b85bd
    if ((ecx) != (edx)) goto LAB_005B85BD;
    // 005B856F  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B8572  mov edx, 1
    edx = (1);
    // 005B8577  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B8579  test dl, bl
    // 005B857B  jne 0x5b85bd
    if (((((edx & 0xff)) & ((ebx & 0xff)))) != 0) goto LAB_005B85BD;
    // 005B857D  test byte ptr [0x9a64c0], 0x80
    // 005B8584  jne 0x5b85bd
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B85BD;
    // 005B8586  mov edx, ecx
    edx = (ecx);
    // 005B8588  shl edx, 6
    edx = ((edx) << (6));
    // 005B858B  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B858D  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B8590  lea edx, [ecx + edx*8]
    edx = (ecx + edx*8);
    // 005B8593  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005B8596  mov ebx, dword ptr [edx*4 + 0x96c9e0]
    ebx = ((*(int*)((char*)g_0096c9e0 + edx*4)));
    // 005B859D  test bh, 2
    // 005B85A0  jne 0x5b85bd
    if ((((((ebx >> 8) & 0xff)) & (2))) != 0) goto LAB_005B85BD;
    // 005B85A2  mov ebx, dword ptr [eax + 0xc]
    ebx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B85A5  or ebx, 0x400000
    ebx = ((ebx) | (0x400000));
    // 005B85AB  mov dword ptr [eax + 0xc], ebx
    (*(int*)((char*)(eax) + 0xc)) = (ebx);
    // 005B85AE  mov edx, dword ptr [0x90d91c]
    edx = ((*(int*)((char*)g_0090d91c)));
    // 005B85B4  or edx, 1
    edx = ((edx) | (1));
    // 005B85B7  mov dword ptr [0x90d91c], edx
    (*(int*)((char*)g_0090d91c)) = (edx);
LAB_005B85BD:
    // 005B85BD  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B85C0  mov dl, 1
    SETB0(edx, (1));
    // 005B85C2  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B85C4  push ecx
    // 005B85C5  push esi
    // 005B85C6  push edi
    // 005B85C7  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B85C9  mov byte ptr [eax + 4], bl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ebx & 0xff));
    // 005B85CC  call 0x591e50
    synch_bit(edi, esi, ecx);
    // 005B85D1  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005B85D4  add esp, 0xc
LAB_005B85D7:
    // 005B85D7  test edx, edx
    // 005B85D9  mov eax, edx
    eax = (edx);
    // 005B85DB  jl 0x5b863f
    if (((edx) & (edx)) < 0) goto LAB_005B863F;
    // 005B85DD  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B85E0  mov cx, word ptr [ecx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx))));
    // 005B85E7  test cx, cx
    // 005B85EA  jl 0x5b8602
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B8602;
LAB_005B85EC:
    // 005B85EC  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B85EF  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B85F2  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B85F5  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B85FD  test cx, cx
    // 005B8600  jge 0x5b85ec
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B85EC;
LAB_005B8602:
    // 005B8602  test eax, eax
    // 005B8604  jl 0x5b863f
    if (((eax) & (eax)) < 0) goto LAB_005B863F;
    // 005B8606  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B8609  mov dl, 1
    SETB0(edx, (1));
    // 005B860B  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B860D:
    // 005B860D  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B8610  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B8613  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B8616  shl eax, 2
    eax = ((eax) << (2));
    // 005B8619  mov bl, byte ptr [eax + 0x95284f]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B861F  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B8621  test ecx, ecx
    // 005B8623  mov byte ptr [eax + 0x95284f], bl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ebx & 0xff));
    // 005B8629  je 0x5b8634
    if ((((ecx) & (ecx))) == 0) goto LAB_005B8634;
    // 005B862B  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B8634:
    // 005B8634  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B863B  test eax, eax
    // 005B863D  jge 0x5b860d
    if (((eax) & (eax)) >= 0) goto LAB_005B860D;
LAB_005B863F:
    // 005B863F  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005B8644  mov esi, dword ptr [ebp - 0x50]
    esi = (var_50);
    // 005B8647  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B864B  add eax, esi
    eax = ((eax) + (esi));
    // 005B864D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B864F  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B8652  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B8655  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005B865B  mov cl, byte ptr [edx + eax*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(edx) + eax*4))));
    // 005B865E  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005B8661  cmp ecx, 0x60
    // 005B8664  jl 0x5b86a3
    if (ecx < 0x60) goto LAB_005B86A3;
    // 005B8666  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 005B8669  test eax, eax
    // 005B866B  je 0x5b86a3
    if ((((eax) & (eax))) == 0) goto LAB_005B86A3;
    // 005B866D  mov ecx, eax
    ecx = (eax);
    // 005B866F  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B8672  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B8674  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B8677  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B867A  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005B867D  add edx, eax
    edx = ((edx) + (eax));
    // 005B867F  lea ecx, [edx + ecx*2]
    ecx = (edx + ecx*2);
    // 005B8682  test byte ptr [ecx*4 + 0x96c9f8], 2
    // 005B868A  jne 0x5b86a3
    if (((((*(unsigned char*)((char*)g_0096c9f8 + ecx*4))) & (2))) != 0) goto LAB_005B86A3;
    // 005B868C  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 005B868F  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 005B8692  push -1
    // 005B8694  push edx
    // 005B8695  push ecx
    // 005B8696  push 2
    // 005B8698  push 6
    // 005B869A  push eax
    // 005B869B  call 0x579a30
    add_goal(eax, 6, 2, ecx, edx, -1);
    // 005B86A0  add esp, 0x18
LAB_005B86A3:
    // 005B86A3  mov ebx, dword ptr [ebp - 0x30]
    ebx = (var_30);
LAB_005B86A6:
    // 005B86A6  test ebx, ebx
    // 005B86A8  jl 0x5b89f0
    if (((ebx) & (ebx)) < 0) goto LAB_005B89F0;
    // 005B86AE  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B86B1  mov esi, dword ptr [0x68faf0]
    esi = ((*(int*)((char*)g_0068faf0)));
    // 005B86B7  mov edx, ecx
    edx = (ecx);
    // 005B86B9  shl edx, 6
    edx = ((edx) << (6));
    // 005B86BC  add edx, ecx
    edx = ((edx) + (ecx));
    // 005B86BE  lea eax, [ecx + edx*2]
    eax = (ecx + edx*2);
    // 005B86C1  lea edx, [ecx + eax*8]
    edx = (ecx + eax*8);
    // 005B86C4  lea ebx, [ecx + edx*2]
    ebx = (ecx + edx*2);
    // 005B86C7  shl ebx, 2
    ebx = ((ebx) << (2));
    // 005B86CA  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
    // 005B86CD  mov eax, dword ptr [ebx + 0x96c9e0]
    eax = ((*(int*)((char*)g_0096c9e0 + ebx)));
    // 005B86D3  and eax, 0x200
    eax = ((eax) & (0x200));
    // 005B86D8  mov dword ptr [ebp - 0x48], eax
    var_48 = (eax);
    // 005B86DB  jne 0x5b8707
    if ((eax) != (0x0)) goto LAB_005B8707;
    // 005B86DD  mov eax, esi
    eax = (esi);
    // 005B86DF  mov edi, dword ptr [ebp - 0x54]
    edi = (var_54);
    // 005B86E2  imul eax, dword ptr [ebp - 0x24]
    eax = ((eax) * (var_24));
    // 005B86E6  add eax, edi
    eax = ((eax) + (edi));
    // 005B86E8  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B86EB  lea edi, [eax + edx*2]
    edi = (eax + edx*2);
    // 005B86EE  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005B86F4  mov eax, 1
    eax = (1);
    // 005B86F9  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 005B86FB  test byte ptr [edx + edi*4 + 4], al
    // 005B86FF  je 0x5b89f0
    if (((((*(unsigned char*)((char*)(edx) + edi*4 + 0x4))) & ((eax & 0xff)))) == 0) goto LAB_005B89F0;
    // 005B8705  jmp 0x5b870d
    goto LAB_005B870D;
LAB_005B8707:
    // 005B8707  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
LAB_005B870D:
    // 005B870D  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 005B8710  test eax, eax
    // 005B8712  jne 0x5b89f0
    if ((((eax) & (eax))) != 0) goto LAB_005B89F0;
    // 005B8718  mov eax, esi
    eax = (esi);
    // 005B871A  mov ecx, dword ptr [ebp - 0x50]
    ecx = (var_50);
    // 005B871D  imul eax, dword ptr [ebp - 0x14]
    eax = ((eax) * (var_14));
    // 005B8721  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B8723  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B8726  lea eax, [eax + ecx*2]
    eax = (eax + ecx*2);
    // 005B8729  lea ecx, [edx + eax*4]
    ecx = (edx + eax*4);
    // 005B872C  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B872E  mov al, byte ptr [ecx]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx)))));
    // 005B8730  mov edi, eax
    edi = (eax);
    // 005B8732  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B8734  and edi, 0xffffffe0
    edi = ((edi) & (0xffffffe0));
    // 005B8737  cmp edi, 0x60
    // 005B873A  mov edi, dword ptr [ebp - 0x44]
    edi = (var_44);
    // 005B873D  setl al
    SETB0(eax, ((edi < 0x60) ? 1 : 0));
    // 005B8740  cmp edi, eax
    // 005B8742  jne 0x5b8764
    if ((edi) != (eax)) goto LAB_005B8764;
    // 005B8744  mov eax, esi
    eax = (esi);
    // 005B8746  mov esi, dword ptr [ebp - 0x54]
    esi = (var_54);
    // 005B8749  imul eax, dword ptr [ebp - 0x24]
    eax = ((eax) * (var_24));
    // 005B874D  add eax, esi
    eax = ((eax) + (esi));
    // 005B874F  lea esi, [eax + eax*4]
    esi = (eax + eax*4);
    // 005B8752  lea eax, [eax + esi*2]
    eax = (eax + esi*2);
    // 005B8755  mov dl, byte ptr [edx + eax*4 + 3]
    SETB0(edx, ((*(unsigned char*)((char*)(edx) + eax*4 + 0x3))));
    // 005B8759  mov al, byte ptr [ecx + 3]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + 0x3))));
    // 005B875C  cmp dl, al
    // 005B875E  jne 0x5b89f0
    if (((edx & 0xff)) != ((eax & 0xff))) goto LAB_005B89F0;
LAB_005B8764:
    // 005B8764  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005B8767  test ecx, ecx
    // 005B8769  jl 0x5b8851
    if (((ecx) & (ecx)) < 0) goto LAB_005B8851;
    // 005B876F  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B8772  movsx esi, word ptr [eax + 0x95282a]
    esi = ((*(short*)((char*)g_0095282a + eax)));
    // 005B8779  movsx edi, word ptr [eax + 0x952828]
    edi = ((*(short*)((char*)g_00952828 + eax)));
    // 005B8780  test esi, esi
    // 005B8782  jl 0x5b8851
    if (((esi) & (esi)) < 0) goto LAB_005B8851;
    // 005B8788  cmp esi, dword ptr [0x949874]
    // 005B878E  jge 0x5b8851
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B8851;
    // 005B8794  test edi, edi
    // 005B8796  jl 0x5b8851
    if (((edi) & (edi)) < 0) goto LAB_005B8851;
    // 005B879C  cmp edi, dword ptr [0x949870]
    // 005B87A2  jge 0x5b8851
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B8851;
    // 005B87A8  test esi, esi
    // 005B87AA  jl 0x5b8851
    if (((esi) & (esi)) < 0) goto LAB_005B8851;
    // 005B87B0  cmp esi, dword ptr [0x949874]
    // 005B87B6  jge 0x5b8851
    if (esi >= (*(int*)((char*)g_00949874))) goto LAB_005B8851;
    // 005B87BC  test edi, edi
    // 005B87BE  jl 0x5b8851
    if (((edi) & (edi)) < 0) goto LAB_005B8851;
    // 005B87C4  cmp edi, dword ptr [0x949870]
    // 005B87CA  jge 0x5b8851
    if (edi >= (*(int*)((char*)g_00949870))) goto LAB_005B8851;
    // 005B87D0  mov eax, esi
    eax = (esi);
    // 005B87D2  mov ecx, edi
    ecx = (edi);
    // 005B87D4  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 005B87DB  sar ecx, 1
    ecx = ((ecx) >> (1));
    // 005B87DD  add eax, ecx
    eax = ((eax) + (ecx));
    // 005B87DF  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005B87E5  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005B87E8  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005B87EB  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005B87F1  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 005B87F4  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005B87F7  cmp ecx, edx
    // 005B87F9  jne 0x5b8834
    if ((ecx) != (edx)) goto LAB_005B8834;
    // 005B87FB  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B87FE  mov edx, 1
    edx = (1);
    // 005B8803  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B8805  test dl, bl
    // 005B8807  jne 0x5b8834
    if (((((edx & 0xff)) & ((ebx & 0xff)))) != 0) goto LAB_005B8834;
    // 005B8809  test byte ptr [0x9a64c0], 0x80
    // 005B8810  jne 0x5b8834
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x80))) != 0) goto LAB_005B8834;
    // 005B8812  mov edx, dword ptr [ebp - 0x48]
    edx = (var_48);
    // 005B8815  test edx, edx
    // 005B8817  jne 0x5b8834
    if ((((edx) & (edx))) != 0) goto LAB_005B8834;
    // 005B8819  mov ebx, dword ptr [eax + 0xc]
    ebx = ((*(int*)((char*)(eax) + 0xc)));
    // 005B881C  or ebx, 0x400000
    ebx = ((ebx) | (0x400000));
    // 005B8822  mov dword ptr [eax + 0xc], ebx
    (*(int*)((char*)(eax) + 0xc)) = (ebx);
    // 005B8825  mov edx, dword ptr [0x90d91c]
    edx = ((*(int*)((char*)g_0090d91c)));
    // 005B882B  or edx, 1
    edx = ((edx) | (1));
    // 005B882E  mov dword ptr [0x90d91c], edx
    (*(int*)((char*)g_0090d91c)) = (edx);
LAB_005B8834:
    // 005B8834  mov bl, byte ptr [eax + 4]
    SETB0(ebx, ((*(unsigned char*)((char*)(eax) + 0x4))));
    // 005B8837  mov dl, 1
    SETB0(edx, (1));
    // 005B8839  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
    // 005B883B  push ecx
    // 005B883C  push esi
    // 005B883D  push edi
    // 005B883E  or bl, dl
    SETB0(ebx, (((ebx & 0xff)) | ((edx & 0xff))));
    // 005B8840  mov byte ptr [eax + 4], bl
    (*(unsigned char*)((char*)(eax) + 0x4)) = ((ebx & 0xff));
    // 005B8843  call 0x591e50
    synch_bit(edi, esi, ecx);
    // 005B8848  mov ebx, dword ptr [ebp - 0x30]
    ebx = (var_30);
    // 005B884B  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005B884E  add esp, 0xc
LAB_005B8851:
    // 005B8851  test ecx, ecx
    // 005B8853  mov eax, ecx
    eax = (ecx);
    // 005B8855  jl 0x5b88ba
    if (((ecx) & (ecx)) < 0) goto LAB_005B88BA;
    // 005B8857  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005B885A  mov cx, word ptr [ecx + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx))));
    // 005B8861  test cx, cx
    // 005B8864  jl 0x5b887c
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) < 0) goto LAB_005B887C;
LAB_005B8866:
    // 005B8866  movsx eax, cx
    eax = (((short)(ecx & 0xffff)));
    // 005B8869  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B886C  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 005B886F  mov cx, word ptr [ecx*4 + 0x95285a]
    SETW0(ecx, ((*(unsigned short*)((char*)g_0095285a + ecx*4))));
    // 005B8877  test cx, cx
    // 005B887A  jge 0x5b8866
    if (((short)(((ecx & 0xffff)) & ((ecx & 0xffff)))) >= 0) goto LAB_005B8866;
LAB_005B887C:
    // 005B887C  test eax, eax
    // 005B887E  jl 0x5b88ba
    if (((eax) & (eax)) < 0) goto LAB_005B88BA;
    // 005B8880  mov esi, dword ptr [ebp - 0x10]
    esi = (var_10);
    // 005B8883  mov dl, 1
    SETB0(edx, (1));
    // 005B8885  mov ecx, esi
    ecx = (esi);
    // 005B8887  shl dl, cl
    SETB0(edx, (((edx & 0xff)) << ((ecx & 0xff))));
LAB_005B8889:
    // 005B8889  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B888C  lea eax, [eax + ecx*4]
    eax = (eax + ecx*4);
    // 005B888F  shl eax, 2
    eax = ((eax) << (2));
    // 005B8892  mov cl, byte ptr [eax + 0x95284f]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0095284f + eax))));
    // 005B8898  or cl, dl
    SETB0(ecx, (((ecx & 0xff)) | ((edx & 0xff))));
    // 005B889A  test esi, esi
    // 005B889C  mov byte ptr [eax + 0x95284f], cl
    (*(unsigned char*)((char*)g_0095284f + eax)) = ((ecx & 0xff));
    // 005B88A2  je 0x5b88ad
    if ((((esi) & (esi))) == 0) goto LAB_005B88AD;
    // 005B88A4  and word ptr [eax + 0x952830], 0xfbbf
    (*(unsigned short*)((char*)g_00952830 + eax)) = (((*(unsigned short*)((char*)g_00952830 + eax))) & (0xfbbf));
LAB_005B88AD:
    // 005B88AD  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B88B4  test eax, eax
    // 005B88B6  jge 0x5b8889
    if (((eax) & (eax)) >= 0) goto LAB_005B8889;
    // 005B88B8  jmp 0x5b88bd
    goto LAB_005B88BD;
LAB_005B88BA:
    // 005B88BA  mov esi, dword ptr [ebp - 0x10]
    esi = (var_10);
LAB_005B88BD:
    // 005B88BD  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005B88C0  movsx eax, word ptr [edx + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + edx)));
    // 005B88C7  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B88CA  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B88CD  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005B88CF  mov al, byte ptr [edx*4 + 0x9ab88c]
    SETB0(eax, ((*(unsigned char*)((char*)g_009ab88c + edx*4))));
    // 005B88D6  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005B88D9  shl eax, 4
    eax = ((eax) << (4));
    // 005B88DC  mov cl, byte ptr [eax + 0x94a379]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094a379 + eax))));
    // 005B88E2  test cl, cl
    // 005B88E4  jne 0x5b8983
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005B8983;
    // 005B88EA  mov ecx, dword ptr [0x68faf0]
    ecx = ((*(int*)((char*)g_0068faf0)));
    // 005B88F0  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 005B88F3  mov eax, ecx
    eax = (ecx);
    // 005B88F5  mov edi, dword ptr [ebp - 0x50]
    edi = (var_50);
    // 005B88F8  imul eax, edx
    eax = ((eax) * (edx));
    // 005B88FB  imul ecx, dword ptr [ebp - 0x24]
    ecx = ((ecx) * (var_24));
    // 005B88FF  add eax, edi
    eax = ((eax) + (edi));
    // 005B8901  lea edi, [eax + eax*4]
    edi = (eax + eax*4);
    // 005B8904  lea edi, [eax + edi*2]
    edi = (eax + edi*2);
    // 005B8907  mov eax, dword ptr [ebp - 0x54]
    eax = (var_54);
    // 005B890A  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B890C  mov eax, ecx
    eax = (ecx);
    // 005B890E  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005B8911  lea ecx, [eax + ecx*2]
    ecx = (eax + ecx*2);
    // 005B8914  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005B8919  mov cl, byte ptr [eax + ecx*4 + 3]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + ecx*4 + 0x3))));
    // 005B891D  cmp cl, byte ptr [eax + edi*4 + 3]
    // 005B8921  jne 0x5b8983
    if (((ecx & 0xff)) != ((*(unsigned char*)((char*)(eax) + edi*4 + 0x3)))) goto LAB_005B8983;
    // 005B8923  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005B8926  mov ecx, dword ptr [ebx + 0x96cdd0]
    ecx = ((*(int*)((char*)g_0096cdd0 + ebx)));
    // 005B892C  cmp eax, ecx
    // 005B892E  jne 0x5b8983
    if ((eax) != (ecx)) goto LAB_005B8983;
    // 005B8930  cmp edx, dword ptr [ebx + 0x96cdd4]
    // 005B8936  jne 0x5b8983
    if ((edx) != ((*(int*)((char*)g_0096cdd4 + ebx)))) goto LAB_005B8983;
    // 005B8938  or ecx, 0xffffffff
    ecx = ((ecx) | (0xffffffff));
    // 005B893B  push edx
    // 005B893C  mov dword ptr [ebx + 0x96cdd4], ecx
    (*(int*)((char*)g_0096cdd4 + ebx)) = (ecx);
    // 005B8942  push eax
    // 005B8943  mov dword ptr [ebx + 0x96cdd0], ecx
    (*(int*)((char*)g_0096cdd0 + ebx)) = (ecx);
    // 005B8949  call 0x5bfe90
    eax = veh_at(eax, edx);
    // 005B894E  add esp, 8
    // 005B8951  test eax, eax
    // 005B8953  jl 0x5b8983
    if (((eax) & (eax)) < 0) goto LAB_005B8983;
LAB_005B8955:
    // 005B8955  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 005B8958  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005B895B  shl eax, 2
    eax = ((eax) << (2));
    // 005B895E  mov ecx, dword ptr [eax + 0x95282c]
    ecx = ((*(int*)((char*)g_0095282c + eax)));
    // 005B8964  test ecx, 0x40000
    // 005B896A  je 0x5b8978
    if ((((ecx) & (0x40000))) == 0) goto LAB_005B8978;
    // 005B896C  or ecx, 0x60000
    ecx = ((ecx) | (0x60000));
    // 005B8972  mov dword ptr [eax + 0x95282c], ecx
    (*(int*)((char*)g_0095282c + eax)) = (ecx);
LAB_005B8978:
    // 005B8978  movsx eax, word ptr [eax + 0x952858]
    eax = ((*(short*)((char*)g_00952858 + eax)));
    // 005B897F  test eax, eax
    // 005B8981  jge 0x5b8955
    if (((eax) & (eax)) >= 0) goto LAB_005B8955;
LAB_005B8983:
    // 005B8983  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005B8986  movsx eax, word ptr [eax + 0x952832]
    eax = ((*(short*)((char*)g_00952832 + eax)));
    // 005B898D  lea ecx, [eax + eax*2]
    ecx = (eax + eax*2);
    // 005B8990  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 005B8993  cmp byte ptr [edx*4 + 0x9ab892], 0xb
    // 005B899B  jne 0x5b89f0
    if (((*(unsigned char*)((char*)g_009ab892 + edx*4))) != (0xb)) goto LAB_005B89F0;
    // 005B899D  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005B89A0  mov ecx, eax
    ecx = (eax);
    // 005B89A2  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005B89A5  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005B89A7  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005B89AA  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 005B89AD  lea edx, [esi + eax]
    edx = (eax);
    // 005B89B0  lea eax, [edx + ecx*2]
    eax = (edx + ecx*2);
    // 005B89B3  test byte ptr [eax*4 + 0x96c9f8], 1
    // 005B89BB  jne 0x5b89f0
    if (((((*(unsigned char*)((char*)g_0096c9f8 + eax*4))) & (1))) != 0) goto LAB_005B89F0;
    // 005B89BD  mov ebx, dword ptr [ebp - 0x24]
    ebx = (var_24);
    // 005B89C0  mov edi, dword ptr [ebp - 0x38]
    edi = (var_38);
    // 005B89C3  push -1
    // 005B89C5  push ebx
    // 005B89C6  push edi
    // 005B89C7  push 2
    // 005B89C9  push 0
    // 005B89CB  push esi
    // 005B89CC  call 0x579a30
    add_goal(esi, 0, 2, edi, ebx, -1);
    // 005B89D1  push -1
    // 005B89D3  push ebx
    // 005B89D4  push edi
    // 005B89D5  push 1
    // 005B89D7  push 2
    // 005B89D9  push esi
    // 005B89DA  call 0x579a30
    add_goal(esi, 2, 1, edi, ebx, -1);
    // 005B89DF  push -1
    // 005B89E1  push ebx
    // 005B89E2  push edi
    // 005B89E3  push 1
    // 005B89E5  push 3
    // 005B89E7  push esi
    // 005B89E8  call 0x579a30
    add_goal(esi, 3, 1, edi, ebx, -1);
    // 005B89ED  add esp, 0x48
LAB_005B89F0:
    // 005B89F0  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 005B89F3  inc edx
    edx = ((edx) + 1);
    // 005B89F4  cmp edx, 0x19
    // 005B89F7  mov dword ptr [ebp - 0x18], edx
    var_18 = (edx);
    // 005B89FA  jl 0x5b7cfa
    if (edx < 0x19) goto LAB_005B7CFA;
    // 005B8A00  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
    // 005B8A05  test eax, eax
    // 005B8A07  je 0x5b8adb
    if ((((eax) & (eax))) == 0) goto LAB_005B8ADB;
    // 005B8A0D  mov edi, dword ptr [ebp - 8]
    edi = (var_8);
    // 005B8A10  mov ebx, dword ptr [0x939284]
    ebx = ((*(int*)((char*)g_00939284)));
    // 005B8A16  mov esi, dword ptr [ebp - 0x1c]
    esi = (var_1c);
    // 005B8A19  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005B8A1B  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 005B8A1E  cmp edi, ebx
    // 005B8A20  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 005B8A23  push 2
    // 005B8A25  setne cl
    SETB0(ecx, (((edi) != (ebx)) ? 1 : 0));
    // 005B8A28  dec ecx
    ecx = ((ecx) - 1);
    // 005B8A29  and ecx, esi
    ecx = ((ecx) & (esi));
    // 005B8A2B  push ecx
    // 005B8A2C  push edx
    // 005B8A2D  push ebx
    // 005B8A2E  call 0x46aef0
    draw_radius(ebx, edx, ecx, 2);
    // 005B8A33  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 005B8A38  add esp, 0x10
    // 005B8A3B  cmp edi, eax
    // 005B8A3D  jne 0x5b8a4d
    if ((edi) != (eax)) goto LAB_005B8A4D;
    // 005B8A3F  test esi, esi
    // 005B8A41  je 0x5b8a4d
    if ((((esi) & (esi))) == 0) goto LAB_005B8A4D;
    // 005B8A43  mov ecx, 0x8e9f60
    ecx = (0x8e9f60);
    // 005B8A48  call 0x5d5a70
    reinterpret_cast<GraphicWin*>(0x8e9f60)->redraw();
LAB_005B8A4D:
    // 005B8A4D  cmp dword ptr [ebp - 0x58], 8
    // 005B8A51  jle 0x5b8a80
    if (var_58 <= 8) goto LAB_005B8A80;
    // 005B8A53  cmp edi, dword ptr [0x939284]
    // 005B8A59  jne 0x5b8a80
    if ((edi) != ((*(int*)((char*)g_00939284)))) goto LAB_005B8A80;
    // 005B8A5B  lea eax, [edi + edi*4]
    eax = (edi + edi*4);
    // 005B8A5E  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005B8A61  shl eax, 3
    eax = ((eax) << (3));
    // 005B8A64  sub eax, edi
    eax = ((eax) - (edi));
    // 005B8A66  test byte ptr [eax*4 + 0x946f58], 0x80
    // 005B8A6E  je 0x5b8a80
    if (((((*(unsigned char*)((char*)g_00946f58 + eax*4))) & (0x80))) == 0) goto LAB_005B8A80;
    // 005B8A70  push 0
    // 005B8A72  push 1
    // 005B8A74  push 0
    // 005B8A76  push 0x29
    // 005B8A78  call 0x5230e0
    interlude(0x29, 0, 1, 0);
    // 005B8A7D  add esp, 0x10
LAB_005B8A80:
    // 005B8A80  test byte ptr [ebp - 0x4c], 1
    // 005B8A84  je 0x5b8a9e
    if (((((var_4c & 0xff)) & (1))) == 0) goto LAB_005B8A9E;
    // 005B8A86  mov ecx, dword ptr [ebp - 0x5c]
    ecx = (var_5c);
    // 005B8A89  mov edx, dword ptr [ebp - 0x60]
    edx = (var_60);
    // 005B8A8C  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005B8A8F  push ecx
    // 005B8A90  push edx
    // 005B8A91  push eax
    // 005B8A92  push ebx
    // 005B8A93  push 0
    // 005B8A95  push edi
    // 005B8A96  call 0x55e370
    encounter(edi, 0, ebx, eax, edx, ecx);
    // 005B8A9B  add esp, 0x18
LAB_005B8A9E:
    // 005B8A9E  test edi, edi
    // 005B8AA0  je 0x5b8adb
    if ((((edi) & (edi))) == 0) goto LAB_005B8ADB;
    // 005B8AA2  mov eax, dword ptr [ebp + 0xc]
    eax = (a2);
    // 005B8AA5  test eax, eax
    // 005B8AA7  je 0x5b8adb
    if ((((eax) & (eax))) == 0) goto LAB_005B8ADB;
    // 005B8AA9  mov esi, 1
    esi = (1);
LAB_005B8AAE:
    // 005B8AAE  cmp esi, edi
    // 005B8AB0  je 0x5b8ad5
    if ((esi) == (edi)) goto LAB_005B8AD5;
    // 005B8AB2  mov eax, dword ptr [ebp - 0x4c]
    eax = (var_4c);
    // 005B8AB5  mov edx, 1
    edx = (1);
    // 005B8ABA  mov ecx, esi
    ecx = (esi);
    // 005B8ABC  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005B8ABE  test eax, edx
    // 005B8AC0  je 0x5b8ad5
    if ((((eax) & (edx))) == 0) goto LAB_005B8AD5;
    // 005B8AC2  mov ecx, dword ptr [ebp - 0x24]
    ecx = (var_24);
    // 005B8AC5  push 0
    // 005B8AC7  push 0
    // 005B8AC9  push ecx
    // 005B8ACA  push ebx
    // 005B8ACB  push esi
    // 005B8ACC  push edi
    // 005B8ACD  call 0x55e370
    encounter(edi, esi, ebx, ecx, 0, 0);
    // 005B8AD2  add esp, 0x18
LAB_005B8AD5:
    // 005B8AD5  inc esi
    esi = ((esi) + 1);
    // 005B8AD6  cmp esi, 8
    // 005B8AD9  jl 0x5b8aae
    if (esi < 8) goto LAB_005B8AAE;
LAB_005B8ADB:
    return;
}
