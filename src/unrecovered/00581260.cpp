// ORIGINAL: 0x00581260 ?consider_designs@@YAXH@Z 0x00581260-0x00583C8F FILE
// TRIED: mechanical register/stack-slot goto transliteration (304 labels, one 7-case switch off 0x0094A37D, 6375-line body): compiles, MISMATCH sim 0.80 on /O2 /Oy- framed. Cross-branch shared-push args (26 pushes never traced to a specific call by the linear model) are a known soft spot - values landed but not re-verified by hand past the two spots that broke arg *counts* outright.
// working copy - scaffold materialised by --work
// size      10799 bytes
// prototype 
// callers   7   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA30 0x00539C00 0x00560DD0 0x0057EFF0 0x0057F0B0 0x00580860 0x00581190 0x005A5A60 0x005B9F20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?consider_designs@@YAXH@Z  at 0x00581260  (10799 bytes)
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
bool has_tech(int, int);
int bit_count(int);
int great_satan(int, int);
int pick_chassis(int, int, int);
int propose_proto(int, int, int, int, int, int, int, char *);
int proto_cost(int, int, int, int, int);
int weapon_budget(int, int, int);
void add_abil(int *, int, int);
void enemy_capabilities(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00583c90 = (int *)0x00583C90;
static int *const g_0068f0f0 = (int *)0x0068F0F0;
static int *const g_0093fc54 = (int *)0x0093FC54;
static int *const g_00946f28 = (int *)0x00946F28;
static int *const g_00946f4c = (int *)0x00946F4C;
static int *const g_009474e8 = (int *)0x009474E8;
static int *const g_00949780 = (int *)0x00949780;
static int *const g_00949c2c = (int *)0x00949C2C;
static int *const g_0094a378 = (int *)0x0094A378;
static int *const g_0094a37d = (int *)0x0094A37D;
static int *const g_0094a56e = (int *)0x0094A56E;
static int *const g_0094ae68 = (int *)0x0094AE68;
static int *const g_0094afd8 = (int *)0x0094AFD8;
static int *const g_0094afdc = (int *)0x0094AFDC;
static int *const g_0094afec = (int *)0x0094AFEC;
static int *const g_0094affc = (int *)0x0094AFFC;
static int *const g_0094b008 = (int *)0x0094B008;
static int *const g_0094f280 = (int *)0x0094F280;
static int *const g_0094f310 = (int *)0x0094F310;
static int *const g_0094f360 = (int *)0x0094F360;
static int *const g_00952800 = (int *)0x00952800;
static int *const g_00952824 = (int *)0x00952824;
static int *const g_0096c9e4 = (int *)0x0096C9E4;
static int *const g_0096c9e8 = (int *)0x0096C9E8;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cb8c = (int *)0x0096CB8C;
static int *const g_0096cc3c = (int *)0x0096CC3C;
static int *const g_0096cc50 = (int *)0x0096CC50;
static int *const g_0096cdbc = (int *)0x0096CDBC;
static int *const g_0096cdc0 = (int *)0x0096CDC0;
static int *const g_0096cdc4 = (int *)0x0096CDC4;
static int *const g_0096cdc8 = (int *)0x0096CDC8;
static int *const g_0096cdcc = (int *)0x0096CDCC;
static int *const g_0096ce44 = (int *)0x0096CE44;
static int *const g_0096d204 = (int *)0x0096D204;
static int *const g_0096d20c = (int *)0x0096D20C;
static int *const g_0096d214 = (int *)0x0096D214;
static int *const g_0096d218 = (int *)0x0096D218;
static int *const g_0096d21c = (int *)0x0096D21C;
static int *const g_0096d228 = (int *)0x0096D228;
static int *const g_0096d230 = (int *)0x0096D230;
static int *const g_0096da3c = (int *)0x0096DA3C;
static int *const g_0096ea74 = (int *)0x0096EA74;
static int *const g_0096ea78 = (int *)0x0096EA78;
static int *const g_0096eab0 = (int *)0x0096EAB0;
static int *const g_0096ec58 = (int *)0x0096EC58;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6538 = (int *)0x009A6538;
static int *const g_009a6560 = (int *)0x009A6560;
static int *const g_009a6638 = (int *)0x009A6638;
static int *const g_009a663c = (int *)0x009A663C;
static int *const g_009ab550 = (int *)0x009AB550;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab88d = (int *)0x009AB88D;
static int *const g_009ab88e = (int *)0x009AB88E;
static int *const g_009ab894 = (int *)0x009AB894;
static int *const g_009ab898 = (int *)0x009AB898;
static int *const g_009ab89a = (int *)0x009AB89A;
#define SETB0(dst,val) ((dst) = (((dst) & ~0xff) | ((val) & 0xff)))
#define SETB1(dst,val) ((dst) = (((dst) & ~0xff00) | (((val) & 0xff) << 8)))
#define SETW0(dst,val) ((dst) = (((dst) & ~0xffff) | ((val) & 0xffff)))

#define SETB0(dst,val) ((dst) = (((dst) & ~0xff) | ((val) & 0xff)))
#define SETB1(dst,val) ((dst) = (((dst) & ~0xff00) | (((val) & 0xff) << 8)))
#define SETW0(dst,val) ((dst) = (((dst) & ~0xffff) | ((val) & 0xffff)))

void __cdecl consider_designs(int a1) {
    int eax=0, ebx=0, ecx=0, edx=0, esi=0, edi=0;
    int var_4, var_8, var_c, var_10, var_14, var_18, var_1c, var_20, var_24, var_28, var_2c, var_30, var_34, var_38, var_3c;

    // 00581269  mov edi, dword ptr [ebp + 8]
    edi = (a1);
    // 0058126C  mov dword ptr [0x68f0f0], 0xffffffff
    (*(int*)((char*)g_0068f0f0)) = (0xffffffff);
    // 00581276  mov dword ptr [ebp - 0x1c], 0
    var_1c = (0);
    // 0058127D  lea eax, [edi*4 + 0x96ec58]
    eax = (edi*4 + 0x96ec58);
    // 00581284  mov ecx, 1
    ecx = (1);
    // 00581289  mov esi, 0x96eab0
    esi = (0x96eab0);
    // 0058128E  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00581291:
    // 00581291  cmp ecx, edi
    // 00581293  je 0x581349
    if ((ecx) == (edi)) goto LAB_00581349;
    // 00581299  mov edx, edi
    edx = (edi);
    // 0058129B  shl edx, 6
    edx = ((edx) << (6));
    // 0058129E  add edx, edi
    edx = ((edx) + (edi));
    // 005812A0  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 005812A3  lea edx, [edi + eax*8]
    edx = (edi + eax*8);
    // 005812A6  lea eax, [ecx + edi]
    eax = (edi);
    // 005812A9  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005812AC  shl eax, 2
    eax = ((eax) << (2));
    // 005812AF  mov edx, dword ptr [eax + 0x96c9f8]
    edx = ((*(int*)((char*)g_0096c9f8 + eax)));
    // 005812B5  test edx, 0x40000
    // 005812BB  je 0x5812de
    if ((((edx) & (0x40000))) == 0) goto LAB_005812DE;
    // 005812BD  mov edx, dword ptr [esi + 0x1d0]
    edx = ((*(int*)((char*)(esi) + 0x1d0)));
    // 005812C3  mov eax, 0x55555556
    eax = (0x55555556);
    // 005812C8  add edx, 2
    edx = ((edx) + (2));
    // 005812CB  imul edx
    { __int64 t_prod = (__int64)(int)eax * (__int64)(int)(edx); edx = (int)(t_prod >> 32); eax = (int)t_prod; }
    // 005812CD  mov eax, edx
    eax = (edx);
    // 005812CF  shr eax, 0x1f
    eax = (((unsigned int)eax) >> (0x1f));
    // 005812D2  add edx, eax
    edx = ((edx) + (eax));
    // 005812D4  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 005812D7  add eax, edx
    eax = ((eax) + (edx));
    // 005812D9  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 005812DC  jmp 0x581349
    goto LAB_00581349;
LAB_005812DE:
    // 005812DE  test dl, 0x20
    // 005812E1  je 0x581349
    if (((((edx & 0xff)) & (0x20))) == 0) goto LAB_00581349;
    // 005812E3  mov bl, byte ptr [0x9a64e8]
    SETB0(ebx, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005812E9  mov edx, 1
    edx = (1);
    // 005812EE  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005812F0  test bl, dl
    // 005812F2  je 0x581331
    if (((((ebx & 0xff)) & ((edx & 0xff)))) == 0) goto LAB_00581331;
    // 005812F4  mov edx, edi
    edx = (edi);
    // 005812F6  mov ebx, dword ptr [esi]
    ebx = ((*(int*)((char*)(esi))));
    // 005812F8  shl edx, 6
    edx = ((edx) << (6));
    // 005812FB  add edx, edi
    edx = ((edx) + (edi));
    // 005812FD  lea edx, [edi + edx*2]
    edx = (edi + edx*2);
    // 00581300  lea edx, [edi + edx*8]
    edx = (edi + edx*8);
    // 00581303  lea edx, [edi + edx*2]
    edx = (edi + edx*2);
    // 00581306  cmp ebx, dword ptr [edx*4 + 0x96c9e4]
    // 0058130D  jg 0x58131e
    if (((int)(ebx)) > ((int)((*(int*)((char*)g_0096c9e4 + edx*4))))) goto LAB_0058131E;
    // 0058130F  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00581312  mov ebx, dword ptr [eax + 0x96cb8c]
    ebx = ((*(int*)((char*)g_0096cb8c + eax)));
    // 00581318  mov edx, dword ptr [edx]
    edx = ((*(int*)((char*)(edx))));
    // 0058131A  cmp edx, ebx
    // 0058131C  jle 0x581331
    if (((int)(edx)) <= ((int)(ebx))) goto LAB_00581331;
LAB_0058131E:
    // 0058131E  mov eax, dword ptr [0x9a649c]
    eax = ((*(int*)((char*)g_009a649c)));
    // 00581323  mov edx, dword ptr [ebp - 0x1c]
    edx = (var_1c);
    // 00581326  shr eax, 6
    eax = (((unsigned int)eax) >> (6));
    // 00581329  and eax, 1
    eax = ((eax) & (1));
    // 0058132C  add edx, eax
    edx = ((edx) + (eax));
    // 0058132E  mov dword ptr [ebp - 0x1c], edx
    var_1c = (edx);
LAB_00581331:
    // 00581331  mov eax, dword ptr [esi + 0x120]
    eax = ((*(int*)((char*)(esi) + 0x120)));
    // 00581337  dec eax
    eax = ((eax) - 1);
    // 00581338  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581339  and edx, 3
    edx = ((edx) & (3));
    // 0058133C  add eax, edx
    eax = ((eax) + (edx));
    // 0058133E  mov edx, dword ptr [ebp - 0x1c]
    edx = (var_1c);
    // 00581341  sar eax, 2
    eax = ((((int)(eax))) >> (2));
    // 00581344  add edx, eax
    edx = ((edx) + (eax));
    // 00581346  mov dword ptr [ebp - 0x1c], edx
    var_1c = (edx);
LAB_00581349:
    // 00581349  mov ebx, dword ptr [ebp + 8]
    ebx = (a1);
    // 0058134C  add esi, 0x20cc
    esi = ((esi) + (0x20cc));
    // 00581352  inc ecx
    ecx = ((ecx) + 1);
    // 00581353  add ebx, 0x20cc
    ebx = ((ebx) + (0x20cc));
    // 00581359  cmp esi, 0x97d044
    // 0058135F  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 00581362  jl 0x581291
    if (((int)(esi)) < ((int)(0x97d044))) goto LAB_00581291;
    // 00581368  push edi
    // 00581369  call 0x560dd0
    enemy_capabilities(edi);
    // 0058136E  add esp, 4
    // 00581371  mov ebx, 3
    ebx = (3);
    // 00581376  mov esi, 0x952824
    esi = (0x952824);
LAB_0058137B:
    // 0058137B  movsx ecx, word ptr [esi]
    ecx = ((*(short*)((char*)(esi))));
    // 0058137E  push edi
    // 0058137F  push ecx
    // 00581380  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00581385  add esp, 8
    // 00581388  test eax, eax
    // 0058138A  jne 0x5815d2
    if ((((eax) & (eax))) != 0) goto LAB_005815D2;
    // 00581390  sub esi, 0xc
    esi = ((esi) - (0xc));
    // 00581393  dec ebx
    ebx = ((ebx) - 1);
    // 00581394  cmp esi, 0x952800
    // 0058139A  jge 0x58137b
    if (((int)(esi)) >= ((int)(0x952800))) goto LAB_0058137B;
    // 0058139C  mov ecx, 1
    ecx = (1);
LAB_005813A1:
    // 005813A1  mov eax, 2
    eax = (2);
    // 005813A6  mov dword ptr [ebp - 0x2c], 0x94ae68
    var_2c = (0x94ae68);
    // 005813AD  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 005813AF  mov esi, dword ptr [ebp - 0x2c]
    esi = (var_2c);
    // 005813B2  mov dword ptr [ebp - 8], ecx
    var_8 = (ecx);
    // 005813B5  mov dword ptr [ebp - 0xc], 0
    var_c = (0);
    // 005813BC  mov dword ptr [ebp - 0x28], eax
    var_28 = (eax);
    // 005813BF  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
    // 005813C2  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 005813C5  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
LAB_005813C8:
    // 005813C8  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 005813CC  push edi
    // 005813CD  push edx
    // 005813CE  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 005813D3  add esp, 8
    // 005813D6  test eax, eax
    // 005813D8  je 0x5814ac
    if ((((eax) & (eax))) == 0) goto LAB_005814AC;
    // 005813DE  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 005813E0  test al, al
    // 005813E2  jle 0x5814ac
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) <= 0) goto LAB_005814AC;
    // 005813E8  cmp al, 0x63
    // 005813EA  jge 0x5814ac
    if (((int)((eax & 0xff))) >= ((int)(0x63))) goto LAB_005814AC;
    // 005813F0  cmp esi, 0x94afd8
    // 005813F6  je 0x5814ac
    if ((esi) == (0x94afd8)) goto LAB_005814AC;
    // 005813FC  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 005813FF  test ecx, ecx
    // 00581401  jl 0x581412
    if (((int)(((ecx) & (ecx)))) < 0) goto LAB_00581412;
    // 00581403  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00581406  cmp byte ptr [ecx + 0x94ae68], al
    // 0058140C  jg 0x5814ac
    if (((signed char)(*(unsigned char*)((char*)g_0094ae68 + ecx))) > ((signed char)(eax & 0xff))) goto LAB_005814AC;
LAB_00581412:
    // 00581412  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
LAB_00581414:
    // 00581414  mov eax, ebx
    eax = (ebx);
    // 00581416  and eax, 0x8000003f
    eax = ((eax) & (0x8000003f));
    // 0058141B  jns 0x581422
    if (((((int)(eax))) - (((int)(0x0)))) >= 0) goto LAB_00581422;
    // 0058141D  dec eax
    eax = ((eax) - 1);
    // 0058141E  or eax, 0xffffffc0
    eax = ((eax) | (0xffffffc0));
    // 00581421  inc eax
    eax = ((eax) + 1);
LAB_00581422:
    // 00581422  cmp ebx, 0x40
    // 00581425  jl 0x58142e
    if (((int)(ebx)) < ((int)(0x40))) goto LAB_0058142E;
    // 00581427  mov ecx, edi
    ecx = (edi);
    // 00581429  shl ecx, 6
    ecx = ((ecx) << (6));
    // 0058142C  add eax, ecx
    eax = ((eax) + (ecx));
LAB_0058142E:
    // 0058142E  mov esi, eax
    esi = (eax);
    // 00581430  imul esi, esi, 0x34
    esi = ((esi) * (0x34));
    // 00581433  test byte ptr [esi + 0x9ab898], 1
    // 0058143A  je 0x58149f
    if (((((*(unsigned char*)((char*)g_009ab898 + esi))) & (1))) == 0) goto LAB_0058149F;
    // 0058143C  mov edx, 1
    edx = (1);
    // 00581441  mov ecx, edi
    ecx = (edi);
    // 00581443  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00581445  test byte ptr [esi + 0x9ab894], dl
    // 0058144B  jne 0x58149f
    if (((((*(unsigned char*)((char*)g_009ab894 + esi))) & ((edx & 0xff)))) != 0) goto LAB_0058149F;
    // 0058144D  cmp eax, 0x40
    // 00581450  jge 0x581467
    if (((int)(eax)) >= ((int)(0x40))) goto LAB_00581467;
    // 00581452  movsx eax, word ptr [esi + 0x9ab89a]
    eax = ((*(short*)((char*)g_009ab89a + esi)));
    // 00581459  push edi
    // 0058145A  push eax
    // 0058145B  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00581460  add esp, 8
    // 00581463  test eax, eax
    // 00581465  je 0x58149f
    if ((((eax) & (eax))) == 0) goto LAB_0058149F;
LAB_00581467:
    // 00581467  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581469  mov al, byte ptr [esi + 0x9ab88c]
    SETB0(eax, ((*(unsigned char*)((char*)g_009ab88c + esi))));
    // 0058146F  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 00581472  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00581475  mov al, byte ptr [ecx + 0x94a37d]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094a37d + ecx))));
    // 0058147B  test al, al
    // 0058147D  jne 0x58149f
    if (((((eax & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_0058149F;
    // 0058147F  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 00581482  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00581484  mov dl, byte ptr [esi + 0x9ab88d]
    SETB0(edx, ((*(unsigned char*)((char*)g_009ab88d + esi))));
    // 0058148A  shl edx, 4
    edx = ((edx) << (4));
    // 0058148D  mov al, byte ptr [edx + 0x94ae68]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094ae68 + edx))));
    // 00581493  mov dl, byte ptr [ecx]
    SETB0(edx, ((*(unsigned char*)((char*)(ecx)))));
    // 00581495  cmp al, dl
    // 00581497  jge 0x5814ac
    if (((signed char)(eax & 0xff)) >= ((signed char)(edx & 0xff))) goto LAB_005814AC;
    // 00581499  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 0058149C  mov dword ptr [ebp - 0x14], edx
    var_14 = (edx);
LAB_0058149F:
    // 0058149F  inc ebx
    ebx = ((ebx) + 1);
    // 005814A0  cmp ebx, 0x80
    // 005814A6  jl 0x581414
    if (((int)(ebx)) < ((int)(0x80))) goto LAB_00581414;
LAB_005814AC:
    // 005814AC  mov esi, dword ptr [ebp - 0x2c]
    esi = (var_2c);
    // 005814AF  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005814B2  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005814B5  inc edx
    edx = ((edx) + 1);
    // 005814B6  cmp esi, 0x94b008
    // 005814BC  mov dword ptr [ebp - 0xc], edx
    var_c = (edx);
    // 005814BF  mov dword ptr [ebp - 0x2c], esi
    var_2c = (esi);
    // 005814C2  jl 0x5813c8
    if (((int)(esi)) < ((int)(0x94b008))) goto LAB_005813C8;
    // 005814C8  mov esi, 0x94f280
    esi = (0x94f280);
    // 005814CD  mov dword ptr [ebp - 0x10], 0
    var_10 = (0);
    // 005814D4  mov dword ptr [ebp - 0x2c], esi
    var_2c = (esi);
LAB_005814D7:
    // 005814D7  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 005814DB  mov ebx, dword ptr [ebp - 4]
    ebx = (var_4);
    // 005814DE  push edi
    // 005814DF  push eax
    // 005814E0  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005814E5  add esp, 8
    // 005814E8  test eax, eax
    // 005814EA  je 0x581587
    if ((((eax) & (eax))) == 0) goto LAB_00581587;
    // 005814F0  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 005814F2  cmp al, 1
    // 005814F4  jle 0x581587
    if (((int)((eax & 0xff))) <= ((int)(1))) goto LAB_00581587;
    // 005814FA  test ebx, ebx
    // 005814FC  jl 0x581509
    if (((int)(((ebx) & (ebx)))) < 0) goto LAB_00581509;
    // 005814FE  shl ebx, 4
    ebx = ((ebx) << (4));
    // 00581501  cmp byte ptr [ebx + 0x94f280], al
    // 00581507  jg 0x581587
    if (((signed char)(*(unsigned char*)((char*)g_0094f280 + ebx))) > ((signed char)(eax & 0xff))) goto LAB_00581587;
LAB_00581509:
    // 00581509  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
LAB_0058150B:
    // 0058150B  mov eax, ebx
    eax = (ebx);
    // 0058150D  and eax, 0x8000003f
    eax = ((eax) & (0x8000003f));
    // 00581512  jns 0x581519
    if (((((int)(eax))) - (((int)(0x0)))) >= 0) goto LAB_00581519;
    // 00581514  dec eax
    eax = ((eax) - 1);
    // 00581515  or eax, 0xffffffc0
    eax = ((eax) | (0xffffffc0));
    // 00581518  inc eax
    eax = ((eax) + 1);
LAB_00581519:
    // 00581519  cmp ebx, 0x40
    // 0058151C  jl 0x581525
    if (((int)(ebx)) < ((int)(0x40))) goto LAB_00581525;
    // 0058151E  mov ecx, edi
    ecx = (edi);
    // 00581520  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00581523  add eax, ecx
    eax = ((eax) + (ecx));
LAB_00581525:
    // 00581525  mov esi, eax
    esi = (eax);
    // 00581527  imul esi, esi, 0x34
    esi = ((esi) * (0x34));
    // 0058152A  test byte ptr [esi + 0x9ab898], 1
    // 00581531  je 0x58157e
    if (((((*(unsigned char*)((char*)g_009ab898 + esi))) & (1))) == 0) goto LAB_0058157E;
    // 00581533  mov edx, 1
    edx = (1);
    // 00581538  mov ecx, edi
    ecx = (edi);
    // 0058153A  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 0058153C  test byte ptr [esi + 0x9ab894], dl
    // 00581542  jne 0x58157e
    if (((((*(unsigned char*)((char*)g_009ab894 + esi))) & ((edx & 0xff)))) != 0) goto LAB_0058157E;
    // 00581544  cmp eax, 0x40
    // 00581547  jge 0x58155e
    if (((int)(eax)) >= ((int)(0x40))) goto LAB_0058155E;
    // 00581549  movsx eax, word ptr [esi + 0x9ab89a]
    eax = ((*(short*)((char*)g_009ab89a + esi)));
    // 00581550  push edi
    // 00581551  push eax
    // 00581552  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00581557  add esp, 8
    // 0058155A  test eax, eax
    // 0058155C  je 0x58157e
    if ((((eax) & (eax))) == 0) goto LAB_0058157E;
LAB_0058155E:
    // 0058155E  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 00581561  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00581563  mov cl, byte ptr [esi + 0x9ab88e]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009ab88e + esi))));
    // 00581569  shl ecx, 4
    ecx = ((ecx) << (4));
    // 0058156C  mov dl, byte ptr [ecx + 0x94f280]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094f280 + ecx))));
    // 00581572  mov cl, byte ptr [eax]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax)))));
    // 00581574  cmp dl, cl
    // 00581576  jge 0x581587
    if (((signed char)(edx & 0xff)) >= ((signed char)(ecx & 0xff))) goto LAB_00581587;
    // 00581578  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 0058157B  mov dword ptr [ebp - 4], ecx
    var_4 = (ecx);
LAB_0058157E:
    // 0058157E  inc ebx
    ebx = ((ebx) + 1);
    // 0058157F  cmp ebx, 0x80
    // 00581585  jl 0x58150b
    if (((int)(ebx)) < ((int)(0x80))) goto LAB_0058150B;
LAB_00581587:
    // 00581587  mov esi, dword ptr [ebp - 0x2c]
    esi = (var_2c);
    // 0058158A  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 0058158D  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00581590  inc edx
    edx = ((edx) + 1);
    // 00581591  cmp esi, 0x94f360
    // 00581597  mov dword ptr [ebp - 0x10], edx
    var_10 = (edx);
    // 0058159A  mov dword ptr [ebp - 0x2c], esi
    var_2c = (esi);
    // 0058159D  jl 0x5814d7
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_005814D7;
    // 005815A3  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005815A6  test eax, eax
    // 005815A8  jge 0x5815b5
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005815B5;
    // 005815AA  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005815AD  test eax, eax
    // 005815AF  jl 0x5818a5
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005818A5;
LAB_005815B5:
    // 005815B5  mov esi, edi
    esi = (edi);
    // 005815B7  mov ebx, dword ptr [ebp - 8]
    ebx = (var_8);
    // 005815BA  imul esi, esi, 0xd00
    esi = ((esi) * (0xd00));
    // 005815C0  mov edx, dword ptr [ebp - 0x28]
    edx = (var_28);
    // 005815C3  add esi, 0x9ab88d
    esi = ((esi) + (0x9ab88d));
    // 005815C9  mov dword ptr [ebp - 0x34], 0x40
    var_34 = (0x40);
    // 005815D0  jmp 0x5815e0
    goto LAB_005815E0;
LAB_005815D2:
    // 005815D2  lea ecx, [ebx + 1]
    ecx = (ebx + 0x1);
    // 005815D5  jmp 0x5813a1
    goto LAB_005813A1;
LAB_005815DA:
    // 005815DA  mov edx, dword ptr [ebp - 0x28]
    edx = (var_28);
    // 005815DD  mov ebx, dword ptr [ebp - 8]
    ebx = (var_8);
LAB_005815E0:
    // 005815E0  test byte ptr [esi + 0xb], 1
    // 005815E4  je 0x581895
    if (((((*(unsigned char*)((char*)(esi) + 0xb))) & (1))) == 0) goto LAB_00581895;
    // 005815EA  mov eax, 1
    eax = (1);
    // 005815EF  mov ecx, edi
    ecx = (edi);
    // 005815F1  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 005815F3  mov cl, byte ptr [esi + 7]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + 0x7))));
    // 005815F6  test cl, al
    // 005815F8  jne 0x581895
    if (((((ecx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00581895;
    // 005815FE  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581600  mov al, byte ptr [esi - 1]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + (-0x1)))));
    // 00581603  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 00581606  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 00581609  shl eax, 4
    eax = ((eax) << (4));
    // 0058160C  mov cl, byte ptr [eax + 0x94a37d]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094a37d + eax))));
    // 00581612  test cl, cl
    // 00581614  jne 0x581895
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00581895;
    // 0058161A  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0058161C  mov al, byte ptr [esi + 5]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x5))));
    // 0058161F  cmp eax, 6
    // 00581622  ja 0x581895
    if (((unsigned int)eax) > ((unsigned int)6)) goto LAB_00581895;
    // 00581628  jmp dword ptr [eax*4 + 0x583c90]
    switch (eax) {
    case 0: goto LAB_0058162F;
    case 1: goto LAB_00581659;
    case 2: goto LAB_0058174D;
    case 3: goto LAB_00581895;
    case 4: goto LAB_0058162F;
    case 5: goto LAB_00581895;
    case 6: goto LAB_00581777;
    default: goto LAB_00581895;
    }
LAB_0058162F:
    // 0058162F  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00581632  test eax, eax
    // 00581634  jl 0x581895
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00581895;
    // 0058163A  mov edx, dword ptr [esi - 5]
    edx = ((*(int*)((char*)(esi) + (-0x5))));
    // 0058163D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0058163F  mov cl, byte ptr [esi + 6]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + 0x6))));
    // 00581642  push 0
    // 00581644  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581646  push ecx
    // 00581647  mov al, byte ptr [esi + 1]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x1))));
    // 0058164A  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 0058164D  push ebx
    // 0058164E  push edx
    // 0058164F  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 00581652  push eax
    // 00581653  push ecx
    // 00581654  jmp 0x58188b
    goto LAB_0058188B;
LAB_00581659:
    // 00581659  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 0058165C  test eax, eax
    // 0058165E  jl 0x5816dd
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005816DD;
    // 00581660  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581662  mov al, byte ptr [esi + 1]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x1))));
    // 00581665  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 00581668  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0058166B  test eax, eax
    // 0058166D  jl 0x5816bf
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005816BF;
    // 0058166F  mov eax, ebx
    eax = (ebx);
    // 00581671  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 00581674  dec eax
    eax = ((eax) - 1);
    // 00581675..00581677  neg/sbb -> (eax!=0)?-1:0
    eax = ((eax) != 0 ? -1 : 0);
    // 00581679  add eax, 2
    eax = ((eax) + (2));
    // 0058167C  mov dword ptr [ebp - 0x10], eax
    var_10 = (eax);
    // 0058167F  imul eax, edx
    eax = ((eax) * (edx));
    // 00581682  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00581685  shl eax, 2
    eax = ((eax) << (2));
    // 00581688  movsx ecx, byte ptr [ecx + 0x94ae68]
    ecx = ((*(signed char*)((char*)g_0094ae68 + ecx)));
    // 0058168F  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581690  idiv ecx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ecx)); edx = (int)(t_dividend % (int)(ecx)); }
    // 00581692  cmp eax, 2
    // 00581695  jl 0x5816a4
    if (((int)(eax)) < ((int)(2))) goto LAB_005816A4;
    // 00581697  cmp ecx, 2
    // 0058169A  jl 0x5816a4
    if (((int)(ecx)) < ((int)(2))) goto LAB_005816A4;
    // 0058169C  cmp eax, ecx
    // 0058169E  jle 0x5816a9
    if (((int)(eax)) <= ((int)(ecx))) goto LAB_005816A9;
    // 005816A0  mov eax, ecx
    eax = (ecx);
    // 005816A2  jmp 0x5816a9
    goto LAB_005816A9;
LAB_005816A4:
    // 005816A4  mov eax, 2
    eax = (2);
LAB_005816A9:
    // 005816A9  mov ecx, dword ptr [ebp - 4]
    ecx = (var_4);
    // 005816AC  mov edx, ecx
    edx = (ecx);
    // 005816AE  shl edx, 4
    edx = ((edx) << (4));
    // 005816B1  movsx edx, byte ptr [edx + 0x94f280]
    edx = ((*(signed char*)((char*)g_0094f280 + edx)));
    // 005816B8  cmp edx, eax
    // 005816BA  jg 0x5816bf
    if (((int)(edx)) > ((int)(eax))) goto LAB_005816BF;
    // 005816BC  mov dword ptr [ebp - 0x24], ecx
    var_24 = (ecx);
LAB_005816BF:
    // 005816BF  mov ecx, dword ptr [esi - 5]
    ecx = ((*(int*)((char*)(esi) + (-0x5))));
    // 005816C2  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 005816C5  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005816C7  push 0
    // 005816C9  mov al, byte ptr [esi + 6]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x6))));
    // 005816CC  push eax
    // 005816CD  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005816D0  push ebx
    // 005816D1  push ecx
    // 005816D2  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 005816D5  push edx
    // 005816D6  push eax
    // 005816D7  push ecx
    // 005816D8  jmp 0x58188c
    goto LAB_0058188C;
LAB_005816DD:
    // 005816DD  mov eax, ebx
    eax = (ebx);
    // 005816DF  dec eax
    eax = ((eax) - 1);
    // 005816E0..005816E2  neg/sbb -> (eax!=0)?-1:0
    eax = ((eax) != 0 ? -1 : 0);
    // 005816E4  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005816E6  add eax, 2
    eax = ((eax) + (2));
    // 005816E9  mov cl, byte ptr [esi]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi)))));
    // 005816EB  mov dword ptr [ebp - 0x10], eax
    var_10 = (eax);
    // 005816EE  imul eax, edx
    eax = ((eax) * (edx));
    // 005816F1  mov dword ptr [ebp - 0x3c], ecx
    var_3c = (ecx);
    // 005816F4  shl ecx, 4
    ecx = ((ecx) << (4));
    // 005816F7  shl eax, 2
    eax = ((eax) << (2));
    // 005816FA  movsx ecx, byte ptr [ecx + 0x94ae68]
    ecx = ((*(signed char*)((char*)g_0094ae68 + ecx)));
    // 00581701  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581702  idiv ecx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ecx)); edx = (int)(t_dividend % (int)(ecx)); }
    // 00581704  cmp eax, 2
    // 00581707  jl 0x581716
    if (((int)(eax)) < ((int)(2))) goto LAB_00581716;
    // 00581709  cmp ecx, 2
    // 0058170C  jl 0x581716
    if (((int)(ecx)) < ((int)(2))) goto LAB_00581716;
    // 0058170E  cmp eax, ecx
    // 00581710  jg 0x58171b
    if (((int)(eax)) > ((int)(ecx))) goto LAB_0058171B;
    // 00581712  mov ecx, eax
    ecx = (eax);
    // 00581714  jmp 0x58171b
    goto LAB_0058171B;
LAB_00581716:
    // 00581716  mov ecx, 2
    ecx = (2);
LAB_0058171B:
    // 0058171B  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 0058171E  shl edx, 4
    edx = ((edx) << (4));
    // 00581721  movsx eax, byte ptr [edx + 0x94f280]
    eax = ((*(signed char*)((char*)g_0094f280 + edx)));
    // 00581728  cmp eax, ecx
    // 0058172A  jg 0x581895
    if (((int)(eax)) > ((int)(ecx))) goto LAB_00581895;
    // 00581730  mov edx, dword ptr [esi - 5]
    edx = ((*(int*)((char*)(esi) + (-0x5))));
    // 00581733  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581736  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00581738  push 0
    // 0058173A  mov cl, byte ptr [esi + 6]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + 0x6))));
    // 0058173D  push ecx
    // 0058173E  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00581741  push ebx
    // 00581742  push edx
    // 00581743  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 00581746  push eax
    // 00581747  push ecx
    // 00581748  jmp 0x58188b
    goto LAB_0058188B;
LAB_0058174D:
    // 0058174D  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581750  test eax, eax
    // 00581752  jl 0x581895
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00581895;
    // 00581758  mov ecx, dword ptr [esi - 5]
    ecx = ((*(int*)((char*)(esi) + (-0x5))));
    // 0058175B  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 0058175E  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581760  push 0
    // 00581762  mov al, byte ptr [esi + 6]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x6))));
    // 00581765  push eax
    // 00581766  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581768  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 0058176A  push ebx
    // 0058176B  push ecx
    // 0058176C  mov ecx, dword ptr [ebp - 0x20]
    ecx = (var_20);
    // 0058176F  push edx
    // 00581770  push eax
    // 00581771  push ecx
    // 00581772  jmp 0x58188c
    goto LAB_0058188C;
LAB_00581777:
    // 00581777  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581779  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0058177B  mov al, byte ptr [esi + 1]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x1))));
    // 0058177E  mov dl, byte ptr [esi]
    SETB0(edx, ((*(unsigned char*)((char*)(esi)))));
    // 00581780  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 00581783  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00581786  test eax, eax
    // 00581788  mov dword ptr [ebp - 0x2c], edx
    var_2c = (edx);
    // 0058178B  jl 0x5817ee
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005817EE;
    // 0058178D  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00581790  mov dword ptr [ebp - 0x38], 5
    var_38 = (5);
    // 00581797  imul eax, eax
    eax = ((eax) * (eax));
    // 0058179A  cmp eax, 5
    // 0058179D  jl 0x5817a2
    if (((int)(eax)) < ((int)(5))) goto LAB_005817A2;
    // 0058179F  mov dword ptr [ebp - 0x38], eax
    var_38 = (eax);
LAB_005817A2:
    // 005817A2  mov ecx, edi
    ecx = (edi);
    // 005817A4  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005817A7  add ecx, edi
    ecx = ((ecx) + (edi));
    // 005817A9  lea edx, [edi + ecx*2]
    edx = (edi + ecx*2);
    // 005817AC  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005817AF  push ecx
    // 005817B0  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 005817B3  lea eax, [edi + edx*8]
    eax = (edi + edx*8);
    // 005817B6  mov edx, dword ptr [esi - 5]
    edx = ((*(int*)((char*)(esi) + (-0x5))));
    // 005817B9  push edx
    // 005817BA  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005817BD  lea ebx, [edi + eax*2]
    ebx = (edi + eax*2);
    // 005817C0  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005817C3  push eax
    // 005817C4  push ecx
    // 005817C5  push edx
    // 005817C6  shl ebx, 2
    ebx = ((ebx) << (2));
    // 005817C9  call 0x5a5a60
    eax = proto_cost(edx, ecx, eax, edx, ecx);
    // 005817CE  mov ecx, dword ptr [ebx + 0x96cdcc]
    ecx = ((*(int*)((char*)g_0096cdcc + ebx)));
    // 005817D4  mov edx, dword ptr [ebx + 0x96cdc0]
    edx = ((*(int*)((char*)g_0096cdc0 + ebx)));
    // 005817DA  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 005817DD  add ecx, edx
    ecx = ((ecx) + (edx));
    // 005817DF  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 005817E1  add esp, 0x14
    // 005817E4  cmp eax, ecx
    // 005817E6  jg 0x5817ee
    if (((int)(eax)) > ((int)(ecx))) goto LAB_005817EE;
    // 005817E8  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 005817EB  mov dword ptr [ebp - 0x2c], edx
    var_2c = (edx);
LAB_005817EE:
    // 005817EE  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005817F1  test eax, eax
    // 005817F3  jl 0x58185a
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0058185A;
    // 005817F5  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005817F8  mov dword ptr [ebp - 0x38], 5
    var_38 = (5);
    // 005817FF  imul eax, eax
    eax = ((eax) * (eax));
    // 00581802  cmp eax, 5
    // 00581805  jl 0x58180a
    if (((int)(eax)) < ((int)(5))) goto LAB_0058180A;
    // 00581807  mov dword ptr [ebp - 0x38], eax
    var_38 = (eax);
LAB_0058180A:
    // 0058180A  mov eax, edi
    eax = (edi);
    // 0058180C  shl eax, 6
    eax = ((eax) << (6));
    // 0058180F  add eax, edi
    eax = ((eax) + (edi));
    // 00581811  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 00581814  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00581817  push eax
    // 00581818  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 0058181B  lea edx, [edi + ecx*8]
    edx = (edi + ecx*8);
    // 0058181E  mov ecx, dword ptr [esi - 5]
    ecx = ((*(int*)((char*)(esi) + (-0x5))));
    // 00581821  push ecx
    // 00581822  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00581824  mov cl, byte ptr [esi - 1]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + (-0x1)))));
    // 00581827  lea ebx, [edi + edx*2]
    ebx = (edi + edx*2);
    // 0058182A  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 0058182D  push edx
    // 0058182E  push eax
    // 0058182F  push ecx
    // 00581830  shl ebx, 2
    ebx = ((ebx) << (2));
    // 00581833  call 0x5a5a60
    eax = proto_cost(ecx, eax, edx, ecx, eax);
    // 00581838  mov edx, dword ptr [ebx + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + ebx)));
    // 0058183E  mov ecx, dword ptr [ebx + 0x96cdc0]
    ecx = ((*(int*)((char*)g_0096cdc0 + ebx)));
    // 00581844  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 00581847  add edx, ecx
    edx = ((edx) + (ecx));
    // 00581849  add edx, ebx
    edx = ((edx) + (ebx));
    // 0058184B  add esp, 0x14
    // 0058184E  cmp eax, edx
    // 00581850  jg 0x58185a
    if (((int)(eax)) > ((int)(edx))) goto LAB_0058185A;
    // 00581852  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581855  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 00581858  jmp 0x58186e
    goto LAB_0058186E;
LAB_0058185A:
    // 0058185A  mov ecx, dword ptr [ebp - 0x24]
    ecx = (var_24);
    // 0058185D  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581860  cmp ecx, eax
    // 00581862  je 0x58186e
    if ((ecx) == (eax)) goto LAB_0058186E;
    // 00581864  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 00581867  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 0058186A  cmp eax, edx
    // 0058186C  jne 0x581895
    if ((eax) != (edx)) goto LAB_00581895;
LAB_0058186E:
    // 0058186E  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00581871  mov edx, dword ptr [esi - 5]
    edx = ((*(int*)((char*)(esi) + (-0x5))));
    // 00581874  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581876  push 0
    // 00581878  mov al, byte ptr [esi + 6]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x6))));
    // 0058187B  push eax
    // 0058187C  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 0058187F  push ecx
    // 00581880  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 00581883  push edx
    // 00581884  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00581886  mov dl, byte ptr [esi - 1]
    SETB0(edx, ((*(unsigned char*)((char*)(esi) + (-0x1)))));
    // 00581889  push eax
    // 0058188A  push ecx
LAB_0058188B:
    // 0058188B  push edx
LAB_0058188C:
    // 0058188C  push edi
    // 0058188D  call 0x580860
    eax = propose_proto(edi, edx, ecx, eax, edx, ecx, eax, 0);
    // 00581892  add esp, 0x20
LAB_00581895:
    // 00581895  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 00581898  add esi, 0x34
    esi = ((esi) + (0x34));
    // 0058189B  dec eax
    eax = ((eax) - 1);
    // 0058189C  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 0058189F  jne 0x5815da
    if ((eax) != (0x0)) goto LAB_005815DA;
LAB_005818A5:
    // 005818A5  lea eax, [edi + edi*4]
    eax = (edi + edi*4);
    // 005818A8  mov dword ptr [ebp - 0x18], 0
    var_18 = (0);
    // 005818AF  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 005818B2  shl eax, 3
    eax = ((eax) << (3));
    // 005818B5  sub eax, edi
    eax = ((eax) - (edi));
    // 005818B7  shl eax, 2
    eax = ((eax) << (2));
    // 005818BA  mov dword ptr [ebp - 0x38], eax
    var_38 = (eax);
    // 005818BD  mov ecx, dword ptr [eax + 0x946f4c]
    ecx = ((*(int*)((char*)g_00946f4c + eax)));
    // 005818C3  test ecx, ecx
    // 005818C5  jg 0x5819d2
    if (((int)(((ecx) & (ecx)))) > 0) goto LAB_005819D2;
    // 005818CB  mov eax, 1
    eax = (1);
    // 005818D0  mov ecx, edi
    ecx = (edi);
    // 005818D2  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 005818D4  test byte ptr [0x9a64e8], al
    // 005818DA  jne 0x5819d2
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((eax & 0xff)))) != 0) goto LAB_005819D2;
    // 005818E0  mov esi, 1
    esi = (1);
    // 005818E5  mov ebx, 0x9474e8
    ebx = (0x9474e8);
LAB_005818EA:
    // 005818EA  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005818EF  mov edx, 1
    edx = (1);
    // 005818F4  mov ecx, esi
    ecx = (esi);
    // 005818F6  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005818F8  test al, dl
    // 005818FA  je 0x5819bf
    if (((((eax & 0xff)) & ((edx & 0xff)))) == 0) goto LAB_005819BF;
    // 00581900  push 0
    // 00581902  push esi
    // 00581903  call 0x539c00
    eax = great_satan(esi, 0);
    // 00581908  add esp, 8
    // 0058190B  test eax, eax
    // 0058190D  jne 0x581959
    if ((((eax) & (eax))) != 0) goto LAB_00581959;
    // 0058190F  mov ecx, edi
    ecx = (edi);
    // 00581911  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00581914  add ecx, edi
    ecx = ((ecx) + (edi));
    // 00581916  lea edx, [edi + ecx*2]
    edx = (edi + ecx*2);
    // 00581919  lea eax, [edi + edx*8]
    eax = (edi + edx*8);
    // 0058191C  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 0058191F  cmp dword ptr [ecx*4 + 0x96c9e8], 4
    // 00581927  jge 0x581936
    if (((int)((*(int*)((char*)g_0096c9e8 + ecx*4)))) >= ((int)(4))) goto LAB_00581936;
    // 00581929  test byte ptr [0x9a649c], 0x40
    // 00581930  jne 0x581936
    if (((((*(unsigned char*)((char*)g_009a649c))) & (0x40))) != 0) goto LAB_00581936;
    // 00581932  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581934  jmp 0x58193b
    goto LAB_0058193B;
LAB_00581936:
    // 00581936  mov eax, 0x10
    eax = (0x10);
LAB_0058193B:
    // 0058193B  mov edx, edi
    edx = (edi);
    // 0058193D  or al, 0x20
    SETB0(eax, (((eax & 0xff)) | (0x20)));
    // 0058193F  shl edx, 6
    edx = ((edx) << (6));
    // 00581942  add edx, edi
    edx = ((edx) + (edi));
    // 00581944  lea ecx, [edi + edx*2]
    ecx = (edi + edx*2);
    // 00581947  lea edx, [edi + ecx*8]
    edx = (edi + ecx*8);
    // 0058194A  lea ecx, [esi + edi]
    ecx = (edi);
    // 0058194D  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 00581950  test dword ptr [edx*4 + 0x96c9f8], eax
    // 00581957  je 0x5819bf
    if (((((*(int*)((char*)g_0096c9f8 + edx*4))) & (eax))) == 0) goto LAB_005819BF;
LAB_00581959:
    // 00581959  mov ecx, dword ptr [ebx]
    ecx = ((*(int*)((char*)(ebx))));
    // 0058195B  test ecx, ecx
    // 0058195D  jg 0x581985
    if (((int)(((ecx) & (ecx)))) > 0) goto LAB_00581985;
    // 0058195F  mov eax, dword ptr [0x9a6560]
    eax = ((*(int*)((char*)g_009a6560)));
    // 00581964  test eax, eax
    // 00581966  jl 0x58198d
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0058198D;
    // 00581968  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 0058196B  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 0058196E  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00581971  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00581973  mov dl, byte ptr [eax*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 0058197A  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0058197C  cmp edx, esi
    // 0058197E  sete al
    SETB0(eax, (((edx) == (esi)) ? 1 : 0));
    // 00581981  test eax, eax
    // 00581983  je 0x58198d
    if ((((eax) & (eax))) == 0) goto LAB_0058198D;
LAB_00581985:
    // 00581985  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00581988  or al, 1
    SETB0(eax, (((eax & 0xff)) | (1)));
    // 0058198A  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
LAB_0058198D:
    // 0058198D  test ecx, ecx
    // 0058198F  jg 0x5819b7
    if (((int)(((ecx) & (ecx)))) > 0) goto LAB_005819B7;
    // 00581991  mov eax, dword ptr [0x9a6538]
    eax = ((*(int*)((char*)g_009a6538)));
    // 00581996  test eax, eax
    // 00581998  jl 0x5819bf
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005819BF;
    // 0058199A  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0058199D  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005819A0  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005819A2  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 005819A5  mov cl, byte ptr [eax*4 + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 005819AC  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005819AE  cmp ecx, esi
    // 005819B0  sete al
    SETB0(eax, (((ecx) == (esi)) ? 1 : 0));
    // 005819B3  test eax, eax
    // 005819B5  je 0x5819bf
    if ((((eax) & (eax))) == 0) goto LAB_005819BF;
LAB_005819B7:
    // 005819B7  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005819BA  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 005819BC  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
LAB_005819BF:
    // 005819BF  add ebx, 0x59c
    ebx = ((ebx) + (0x59c));
    // 005819C5  inc esi
    esi = ((esi) + 1);
    // 005819C6  cmp ebx, 0x949c2c
    // 005819CC  jl 0x5818ea
    if (((int)(ebx)) < ((int)(0x949c2c))) goto LAB_005818EA;
LAB_005819D2:
    // 005819D2  push 1
    // 005819D4  push 0
    // 005819D6  push edi
    // 005819D7  call 0x57eff0
    eax = pick_chassis(edi, 0, 1);
    // 005819DC  add esp, 0xc
    // 005819DF  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 005819E2  test eax, eax
    // 005819E4  jl 0x581d1e
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00581D1E;
    // 005819EA  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 005819EC  mov dword ptr [ebp - 0x34], 0xffffffff
    var_34 = (0xffffffff);
    // 005819F3  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 005819F6  mov dword ptr [ebp - 0x24], ebx
    var_24 = (ebx);
    // 005819F9  mov esi, 0x94f280
    esi = (0x94f280);
LAB_005819FE:
    // 005819FE  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00581A02  push edi
    // 00581A03  push edx
    // 00581A04  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00581A09  add esp, 8
    // 00581A0C  test eax, eax
    // 00581A0E  je 0x581a4b
    if ((((eax) & (eax))) == 0) goto LAB_00581A4B;
    // 00581A10  cmp byte ptr [esi + 2], 0x63
    // 00581A14  ja 0x581a4b
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)0x63)) goto LAB_00581A4B;
    // 00581A16  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00581A18  test al, al
    // 00581A1A  jl 0x581a4b
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00581A4B;
    // 00581A1C  test edi, edi
    // 00581A1E  jl 0x581a39
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00581A39;
    // 00581A20  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00581A26  test ecx, ecx
    // 00581A28  jne 0x581a39
    if ((((ecx) & (ecx))) != 0) goto LAB_00581A39;
    // 00581A2A  cmp esi, 0x94f310
    // 00581A30  jle 0x581a39
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00581A39;
    // 00581A32  mov eax, 1
    eax = (1);
    // 00581A37  jmp 0x581a3c
    goto LAB_00581A3C;
LAB_00581A39:
    // 00581A39  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00581A3C:
    // 00581A3C  mov ecx, dword ptr [ebp - 0x34]
    ecx = (var_34);
    // 00581A3F  add eax, eax
    eax = ((eax) + (eax));
    // 00581A41  cmp eax, ecx
    // 00581A43  jl 0x581a4b
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00581A4B;
    // 00581A45  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 00581A48  mov dword ptr [ebp - 0x24], ebx
    var_24 = (ebx);
LAB_00581A4B:
    // 00581A4B  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00581A4E  inc ebx
    ebx = ((ebx) + 1);
    // 00581A4F  cmp esi, 0x94f360
    // 00581A55  jl 0x5819fe
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_005819FE;
    // 00581A57  push edi
    // 00581A58  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00581A5B  push 0x200
    // 00581A60  push eax
    // 00581A61  call 0x581190
    add_abil((int*)(eax), 0x200, edi);
    // 00581A66  mov ecx, edi
    ecx = (edi);
    // 00581A68  add esp, 0xc
    // 00581A6B  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00581A6E  add ecx, edi
    ecx = ((ecx) + (edi));
    // 00581A70  lea edx, [edi + ecx*2]
    edx = (edi + ecx*2);
    // 00581A73  lea eax, [edi + edx*8]
    eax = (edi + edx*8);
    // 00581A76  lea esi, [edi + eax*2]
    esi = (edi + eax*2);
    // 00581A79  shl esi, 2
    esi = ((esi) << (2));
    // 00581A7C  mov dword ptr [ebp - 0x14], esi
    var_14 = (esi);
    // 00581A7F  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00581A85  test eax, eax
    // 00581A87  jl 0x581ab2
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00581AB2;
    // 00581A89  mov eax, dword ptr [esi + 0x96cc3c]
    eax = ((*(int*)((char*)g_0096cc3c + esi)));
    // 00581A8F  test eax, eax
    // 00581A91  jge 0x581aa5
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00581AA5;
    // 00581A93  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581A98  mov edx, 1
    edx = (1);
    // 00581A9D  mov ecx, edi
    ecx = (edi);
    // 00581A9F  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00581AA1  test dl, al
    // 00581AA3  je 0x581ab2
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_00581AB2;
LAB_00581AA5:
    // 00581AA5  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 00581AA8  mov ecx, dword ptr [eax + 0x946f28]
    ecx = ((*(int*)((char*)g_00946f28 + eax)));
    // 00581AAE  test ecx, ecx
    // 00581AB0  jge 0x581ac4
    if (((int)(((ecx) & (ecx)))) >= 0) goto LAB_00581AC4;
LAB_00581AB2:
    // 00581AB2  push edi
    // 00581AB3  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00581AB6  push 0x4000
    // 00581ABB  push ecx
    // 00581ABC  call 0x581190
    add_abil((int*)(ecx), 0x4000, edi);
    // 00581AC1  add esp, 0xc
LAB_00581AC4:
    // 00581AC4  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581AC9  mov ebx, 1
    ebx = (1);
    // 00581ACE  mov ecx, edi
    ecx = (edi);
    // 00581AD0  shl ebx, cl
    ebx = ((ebx) << ((ecx & 0xff)));
    // 00581AD2  test bl, al
    // 00581AD4  mov dword ptr [ebp - 0x20], ebx
    var_20 = (ebx);
    // 00581AD7  jne 0x581b03
    if (((((ebx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00581B03;
    // 00581AD9  mov edx, dword ptr [esi + 0x96d20c]
    edx = ((*(int*)((char*)g_0096d20c + esi)));
    // 00581ADF  mov eax, dword ptr [esi + 0x96d218]
    eax = ((*(int*)((char*)g_0096d218 + esi)));
    // 00581AE5  shl edx, 1
    edx = ((edx) << (1));
    // 00581AE7  cmp eax, edx
    // 00581AE9  jge 0x581af1
    if (((int)(eax)) >= ((int)(edx))) goto LAB_00581AF1;
    // 00581AEB  test byte ptr [ebp - 0x18], 1
    // 00581AEF  je 0x581b03
    if (((((var_18 & 0xff)) & (1))) == 0) goto LAB_00581B03;
LAB_00581AF1:
    // 00581AF1  push edi
    // 00581AF2  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00581AF5  push 0x40000
    // 00581AFA  push eax
    // 00581AFB  call 0x581190
    add_abil((int*)(eax), 0x40000, edi);
    // 00581B00  add esp, 0xc
LAB_00581B03:
    // 00581B03  mov eax, 0x40000
    eax = (0x40000);
    // 00581B08  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00581B0A:
    // 00581B0A  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581B0C  inc ecx
    ecx = ((ecx) + 1);
    // 00581B0D  test al, 1
    // 00581B0F  je 0x581b0a
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00581B0A;
    // 00581B11  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00581B18  push edi
    // 00581B19  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00581B1B  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00581B23  push eax
    // 00581B24  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00581B29  add esp, 8
    // 00581B2C  test eax, eax
    // 00581B2E  je 0x581b74
    if ((((eax) & (eax))) == 0) goto LAB_00581B74;
    // 00581B30  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00581B33  test eax, eax
    // 00581B35  je 0x581b4b
    if ((((eax) & (eax))) == 0) goto LAB_00581B4B;
    // 00581B37  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00581B3D  push edi
    // 00581B3E  push ecx
    // 00581B3F  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00581B44  add esp, 8
    // 00581B47  test eax, eax
    // 00581B49  je 0x581b74
    if ((((eax) & (eax))) == 0) goto LAB_00581B74;
LAB_00581B4B:
    // 00581B4B  test byte ptr [0x9a64e8], bl
    // 00581B51  jne 0x581b74
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_00581B74;
    // 00581B53  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00581B56  test eax, eax
    // 00581B58  jne 0x581b6d
    if ((((eax) & (eax))) != 0) goto LAB_00581B6D;
    // 00581B5A  cmp dword ptr [esi + 0x96ce44], 4
    // 00581B61  jl 0x581b74
    if (((int)((*(int*)((char*)g_0096ce44 + esi)))) < ((int)(4))) goto LAB_00581B74;
    // 00581B63  mov eax, dword ptr [esi + 0x96cc3c]
    eax = ((*(int*)((char*)g_0096cc3c + esi)));
    // 00581B69  test eax, eax
    // 00581B6B  jg 0x581b74
    if (((int)(((eax) & (eax)))) > 0) goto LAB_00581B74;
LAB_00581B6D:
    // 00581B6D  or dword ptr [ebp + 8], 0x40000
    a1 = ((a1) | (0x40000));
LAB_00581B74:
    // 00581B74  push 0
    // 00581B76  push 1
    // 00581B78  push edi
    // 00581B79  call 0x57f0b0
    eax = weapon_budget(edi, 1, 0);
    // 00581B7E  mov ebx, eax
    ebx = (eax);
    // 00581B80  add esp, 0xc
    // 00581B83  mov dword ptr [ebp - 0xc], ebx
    var_c = (ebx);
    // 00581B86  mov dword ptr [ebp - 0x2c], 2
    var_2c = (2);
LAB_00581B8D:
    // 00581B8D  mov edx, dword ptr [ebp - 0x2c]
    edx = (var_2c);
    // 00581B90  push 0
    // 00581B92  push edx
    // 00581B93  push edi
    // 00581B94  call 0x57f0b0
    eax = weapon_budget(edi, edx, 0);
    // 00581B99  mov esi, eax
    esi = (eax);
    // 00581B9B  add esp, 0xc
    // 00581B9E  cmp esi, ebx
    // 00581BA0  je 0x581bed
    if ((esi) == (ebx)) goto LAB_00581BED;
    // 00581BA2  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00581BA5  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00581BA8  mov edx, dword ptr [ebp - 0x24]
    edx = (var_24);
    // 00581BAB  push eax
    // 00581BAC  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581BAF  push ecx
    // 00581BB0  push edx
    // 00581BB1  push ebx
    // 00581BB2  push eax
    // 00581BB3  call 0x5a5a60
    eax = proto_cost(eax, ebx, edx, ecx, eax);
    // 00581BB8  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00581BBB  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00581BBE  mov ebx, eax
    ebx = (eax);
    // 00581BC0  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 00581BC3  push ecx
    // 00581BC4  mov ecx, dword ptr [ebp - 4]
    ecx = (var_4);
    // 00581BC7  push edx
    // 00581BC8  push eax
    // 00581BC9  push esi
    // 00581BCA  push ecx
    // 00581BCB  call 0x5a5a60
    eax = proto_cost(ecx, esi, eax, edx, ecx);
    // 00581BD0  add esp, 0x28
    // 00581BD3  cmp eax, ebx
    // 00581BD5  jg 0x581be5
    if (((int)(eax)) > ((int)(ebx))) goto LAB_00581BE5;
    // 00581BD7  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 00581BDA  mov ebx, esi
    ebx = (esi);
    // 00581BDC  inc eax
    eax = ((eax) + 1);
    // 00581BDD  mov dword ptr [ebp - 0xc], ebx
    var_c = (ebx);
    // 00581BE0  mov dword ptr [ebp - 0x2c], eax
    var_2c = (eax);
    // 00581BE3  jmp 0x581b8d
    goto LAB_00581B8D;
LAB_00581BE5:
    // 00581BE5  mov esi, dword ptr [ebp - 0x24]
    esi = (var_24);
    // 00581BE8  mov ebx, dword ptr [ebp - 0xc]
    ebx = (var_c);
    // 00581BEB  jmp 0x581bf0
    goto LAB_00581BF0;
LAB_00581BED:
    // 00581BED  mov esi, dword ptr [ebp - 0x24]
    esi = (var_24);
LAB_00581BF0:
    // 00581BF0  mov edx, esi
    edx = (esi);
    // 00581BF2  shl edx, 4
    edx = ((edx) << (4));
    // 00581BF5  cmp byte ptr [edx + 0x94f280], 1
    // 00581BFC  jg 0x581c07
    if (((int)((*(unsigned char*)((char*)g_0094f280 + edx)))) > ((int)(1))) goto LAB_00581C07;
    // 00581BFE  mov eax, dword ptr [0x93fc54]
    eax = ((*(int*)((char*)g_0093fc54)));
    // 00581C03  test eax, eax
    // 00581C05  je 0x581c22
    if ((((eax) & (eax))) == 0) goto LAB_00581C22;
LAB_00581C07:
    // 00581C07  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00581C0A  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00581C0D  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00581C10  push 0
    // 00581C12  push 2
    // 00581C14  push eax
    // 00581C15  push ecx
    // 00581C16  push esi
    // 00581C17  push ebx
    // 00581C18  push edx
    // 00581C19  push edi
    // 00581C1A  call 0x580860
    eax = propose_proto(edi, edx, ebx, esi, ecx, eax, 2, 0);
    // 00581C1F  add esp, 0x20
LAB_00581C22:
    // 00581C22  mov eax, 0x40000
    eax = (0x40000);
    // 00581C27  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00581C29:
    // 00581C29  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581C2B  inc ecx
    ecx = ((ecx) + 1);
    // 00581C2C  test al, 1
    // 00581C2E  je 0x581c29
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00581C29;
    // 00581C30  lea eax, [ecx*8]
    eax = (ecx*8);
    // 00581C37  push edi
    // 00581C38  sub eax, ecx
    eax = ((eax) - (ecx));
    // 00581C3A  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 00581C42  push ecx
    // 00581C43  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00581C48  add esp, 8
    // 00581C4B  test eax, eax
    // 00581C4D  je 0x581cab
    if ((((eax) & (eax))) == 0) goto LAB_00581CAB;
    // 00581C4F  test dword ptr [ebp + 8], 0x40000
    // 00581C56  jne 0x581cab
    if ((((a1) & (0x40000))) != 0) goto LAB_00581CAB;
    // 00581C58  mov dl, byte ptr [ebp - 0x20]
    SETB0(edx, ((var_20 & 0xff)));
    // 00581C5B  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581C60  test dl, al
    // 00581C62  jne 0x581cab
    if (((((edx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00581CAB;
    // 00581C64  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00581C67  cmp dword ptr [eax + 0x96ce44], 4
    // 00581C6E  jl 0x581c81
    if (((int)((*(int*)((char*)g_0096ce44 + eax)))) < ((int)(4))) goto LAB_00581C81;
    // 00581C70  mov ecx, dword ptr [eax + 0x96cc3c]
    ecx = ((*(int*)((char*)g_0096cc3c + eax)));
    // 00581C76  test ecx, ecx
    // 00581C78  jg 0x581c81
    if (((int)(((ecx) & (ecx)))) > 0) goto LAB_00581C81;
    // 00581C7A  mov eax, 1
    eax = (1);
    // 00581C7F  jmp 0x581c83
    goto LAB_00581C83;
LAB_00581C81:
    // 00581C81  xor eax, eax
    eax = ((eax) ^ (eax));
LAB_00581C83:
    // 00581C83  or eax, dword ptr [ebp - 0x18]
    eax = ((eax) | (var_18));
    // 00581C86  je 0x581cab
    if ((eax) == (0x0)) goto LAB_00581CAB;
    // 00581C88  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00581C8B  mov ecx, dword ptr [ebp - 4]
    ecx = (var_4);
    // 00581C8E  push 0
    // 00581C90  push 0x12
    // 00581C92  push eax
    // 00581C93  push 0x40000
    // 00581C98  push esi
    // 00581C99  push ebx
    // 00581C9A  push ecx
    // 00581C9B  push edi
    // 00581C9C  mov dword ptr [ebp + 8], 0x40000
    a1 = (0x40000);
    // 00581CA3  call 0x580860
    eax = propose_proto(edi, ecx, ebx, esi, 0x40000, eax, 0x12, 0);
    // 00581CA8  add esp, 0x20
LAB_00581CAB:
    // 00581CAB  mov eax, 0x100
    eax = (0x100);
    // 00581CB0  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00581CB2:
    // 00581CB2  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581CB4  inc ecx
    ecx = ((ecx) + 1);
    // 00581CB5  test al, 1
    // 00581CB7  je 0x581cb2
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00581CB2;
    // 00581CB9  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00581CC0  push edi
    // 00581CC1  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00581CC3  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00581CCB  push eax
    // 00581CCC  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00581CD1  add esp, 8
    // 00581CD4  test eax, eax
    // 00581CD6  je 0x581d1e
    if ((((eax) & (eax))) == 0) goto LAB_00581D1E;
    // 00581CD8  mov cl, byte ptr [ebp - 0x20]
    SETB0(ecx, ((var_20 & 0xff)));
    // 00581CDB  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581CE0  test cl, al
    // 00581CE2  jne 0x581d1e
    if (((((ecx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00581D1E;
    // 00581CE4  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00581CE7  mov ecx, dword ptr [eax + 0x96d228]
    ecx = ((*(int*)((char*)g_0096d228 + eax)));
    // 00581CED  test ecx, ecx
    // 00581CEF  jne 0x581cfb
    if ((((ecx) & (ecx))) != 0) goto LAB_00581CFB;
    // 00581CF1  mov ecx, dword ptr [eax + 0x96d230]
    ecx = ((*(int*)((char*)g_0096d230 + eax)));
    // 00581CF7  test ecx, ecx
    // 00581CF9  je 0x581d1e
    if ((((ecx) & (ecx))) == 0) goto LAB_00581D1E;
LAB_00581CFB:
    // 00581CFB  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00581CFE  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00581D01  push 0
    // 00581D03  push 0x14
    // 00581D05  push edx
    // 00581D06  push 0x100
    // 00581D0B  push esi
    // 00581D0C  push ebx
    // 00581D0D  push eax
    // 00581D0E  push edi
    // 00581D0F  mov dword ptr [ebp + 8], 0x100
    a1 = (0x100);
    // 00581D16  call 0x580860
    eax = propose_proto(edi, eax, ebx, esi, 0x100, edx, 0x14, 0);
    // 00581D1B  add esp, 0x20
LAB_00581D1E:
    // 00581D1E  push 1
    // 00581D20  push 0
    // 00581D22  push edi
    // 00581D23  call 0x57eff0
    eax = pick_chassis(edi, 0, 1);
    // 00581D28  add esp, 0xc
    // 00581D2B  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00581D2E  test eax, eax
    // 00581D30  jl 0x582218
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00582218;
    // 00581D36  mov ecx, dword ptr [ebp - 0x28]
    ecx = (var_28);
    // 00581D39  mov edx, edi
    edx = (edi);
    // 00581D3B  shl edx, 6
    edx = ((edx) << (6));
    // 00581D3E  add edx, edi
    edx = ((edx) + (edi));
    // 00581D40  lea esi, [ecx + ecx]
    esi = (ecx);
    // 00581D43  mov ebx, 1
    ebx = (1);
    // 00581D48  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00581D4F  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 00581D52  mov dword ptr [ebp - 0x2c], ebx
    var_2c = (ebx);
    // 00581D55  mov dword ptr [ebp - 0x24], esi
    var_24 = (esi);
    // 00581D58  lea ecx, [edi + eax*8]
    ecx = (edi + eax*8);
    // 00581D5B  lea eax, [edi + ecx*2]
    eax = (edi + ecx*2);
    // 00581D5E  shl eax, 2
    eax = ((eax) << (2));
    // 00581D61  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 00581D64  mov ecx, dword ptr [eax + 0x96cdcc]
    ecx = ((*(int*)((char*)g_0096cdcc + eax)));
    // 00581D6A  mov eax, 2
    eax = (2);
    // 00581D6F  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581D70  idiv esi
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(esi)); edx = (int)(t_dividend % (int)(esi)); }
    // 00581D72  add ecx, 5
    ecx = ((ecx) + (5));
    // 00581D75  cmp eax, ecx
    // 00581D77  jg 0x581d9e
    if (((int)(eax)) > ((int)(ecx))) goto LAB_00581D9E;
    // 00581D79  mov ebx, dword ptr [ebp - 0x24]
    ebx = (var_24);
    // 00581D7C  mov esi, 2
    esi = (2);
LAB_00581D81:
    // 00581D81  mov edx, dword ptr [ebp - 0x2c]
    edx = (var_2c);
    // 00581D84  inc edx
    edx = ((edx) + 1);
    // 00581D85  inc esi
    esi = ((esi) + 1);
    // 00581D86  mov dword ptr [ebp - 0x2c], edx
    var_2c = (edx);
    // 00581D89  lea eax, [esi + 1]
    eax = (esi + 0x1);
    // 00581D8C  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581D8D  sub eax, edx
    eax = ((eax) - (edx));
    // 00581D8F  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581D91  imul eax, esi
    eax = ((eax) * (esi));
    // 00581D94  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581D95  idiv ebx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ebx)); edx = (int)(t_dividend % (int)(ebx)); }
    // 00581D97  cmp eax, ecx
    // 00581D99  jle 0x581d81
    if (((int)(eax)) <= ((int)(ecx))) goto LAB_00581D81;
    // 00581D9B  mov ebx, dword ptr [ebp - 0x2c]
    ebx = (var_2c);
LAB_00581D9E:
    // 00581D9E  mov eax, 1
    eax = (1);
    // 00581DA3  mov ecx, edi
    ecx = (edi);
    // 00581DA5  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 00581DA7  mov cl, byte ptr [0x9a64e8]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581DAD  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 00581DB0  test al, cl
    // 00581DB2  mov eax, 0x63
    eax = (0x63);
    // 00581DB7  jne 0x581dbb
    if (((((eax & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00581DBB;
    // 00581DB9  mov eax, ebx
    eax = (ebx);
LAB_00581DBB:
    // 00581DBB  push 0
    // 00581DBD  push eax
    // 00581DBE  push edi
    // 00581DBF  call 0x57f0b0
    eax = weapon_budget(edi, eax, 0);
    // 00581DC4  shl eax, 4
    eax = ((eax) << (4));
    // 00581DC7  add esp, 0xc
    // 00581DCA  movsx ecx, byte ptr [eax + 0x94ae68]
    ecx = ((*(signed char*)((char*)g_0094ae68 + eax)));
    // 00581DD1  lea eax, [eax + 0x94ae68]
    eax = (eax + 0x94ae68);
    // 00581DD7  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 00581DDA  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 00581DDD  imul eax, dword ptr [ebp - 0x28]
    eax = ((eax) * (var_28));
    // 00581DE1  shl eax, 2
    eax = ((eax) << (2));
    // 00581DE4  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581DE5  idiv ecx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ecx)); edx = (int)(t_dividend % (int)(ecx)); }
    // 00581DE7  cmp eax, 2
    // 00581DEA  jl 0x581df9
    if (((int)(eax)) < ((int)(2))) goto LAB_00581DF9;
    // 00581DEC  cmp ecx, 2
    // 00581DEF  jl 0x581df9
    if (((int)(ecx)) < ((int)(2))) goto LAB_00581DF9;
    // 00581DF1  cmp eax, ecx
    // 00581DF3  jg 0x581dfe
    if (((int)(eax)) > ((int)(ecx))) goto LAB_00581DFE;
    // 00581DF5  mov ecx, eax
    ecx = (eax);
    // 00581DF7  jmp 0x581dfe
    goto LAB_00581DFE;
LAB_00581DF9:
    // 00581DF9  mov ecx, 2
    ecx = (2);
LAB_00581DFE:
    // 00581DFE  mov eax, ebx
    eax = (ebx);
    // 00581E00  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581E01  sub eax, edx
    eax = ((eax) - (edx));
    // 00581E03  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581E05  cmp ecx, eax
    // 00581E07  jg 0x581e0b
    if (((int)(ecx)) > ((int)(eax))) goto LAB_00581E0B;
    // 00581E09  mov ecx, eax
    ecx = (eax);
LAB_00581E0B:
    // 00581E0B  cmp ecx, 1
    // 00581E0E  jl 0x581e23
    if (((int)(ecx)) < ((int)(1))) goto LAB_00581E23;
    // 00581E10  cmp ecx, 0x63
    // 00581E13  jle 0x581e1e
    if (((int)(ecx)) <= ((int)(0x63))) goto LAB_00581E1E;
    // 00581E15  mov dword ptr [ebp - 0x2c], 0x63
    var_2c = (0x63);
    // 00581E1C  jmp 0x581e2a
    goto LAB_00581E2A;
LAB_00581E1E:
    // 00581E1E  mov dword ptr [ebp - 0x2c], ecx
    var_2c = (ecx);
    // 00581E21  jmp 0x581e2a
    goto LAB_00581E2A;
LAB_00581E23:
    // 00581E23  mov dword ptr [ebp - 0x2c], 1
    var_2c = (1);
LAB_00581E2A:
    // 00581E2A  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00581E2C  mov dword ptr [ebp - 0x34], 0xffffffff
    var_34 = (0xffffffff);
    // 00581E33  mov dword ptr [ebp - 0x10], ebx
    var_10 = (ebx);
    // 00581E36  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00581E3B:
    // 00581E3B  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00581E3F  push edi
    // 00581E40  push edx
    // 00581E41  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00581E46  add esp, 8
    // 00581E49  test eax, eax
    // 00581E4B  je 0x581e8e
    if ((((eax) & (eax))) == 0) goto LAB_00581E8E;
    // 00581E4D  mov ecx, dword ptr [ebp - 0x2c]
    ecx = (var_2c);
    // 00581E50  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00581E52  mov al, byte ptr [esi + 2]
    SETB0(eax, ((*(unsigned char*)((char*)(esi) + 0x2))));
    // 00581E55  cmp eax, ecx
    // 00581E57  jg 0x581e8e
    if (((int)(eax)) > ((int)(ecx))) goto LAB_00581E8E;
    // 00581E59  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00581E5B  test al, al
    // 00581E5D  jl 0x581e8e
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00581E8E;
    // 00581E5F  test edi, edi
    // 00581E61  jl 0x581e7c
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00581E7C;
    // 00581E63  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00581E69  test ecx, ecx
    // 00581E6B  jne 0x581e7c
    if ((((ecx) & (ecx))) != 0) goto LAB_00581E7C;
    // 00581E6D  cmp esi, 0x94f310
    // 00581E73  jle 0x581e7c
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00581E7C;
    // 00581E75  mov eax, 1
    eax = (1);
    // 00581E7A  jmp 0x581e7f
    goto LAB_00581E7F;
LAB_00581E7C:
    // 00581E7C  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00581E7F:
    // 00581E7F  mov ecx, dword ptr [ebp - 0x34]
    ecx = (var_34);
    // 00581E82  add eax, eax
    eax = ((eax) + (eax));
    // 00581E84  cmp eax, ecx
    // 00581E86  jl 0x581e8e
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00581E8E;
    // 00581E88  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 00581E8B  mov dword ptr [ebp - 0x10], ebx
    var_10 = (ebx);
LAB_00581E8E:
    // 00581E8E  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00581E91  inc ebx
    ebx = ((ebx) + 1);
    // 00581E92  cmp esi, 0x94f360
    // 00581E98  jl 0x581e3b
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00581E3B;
    // 00581E9A  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 00581E9D  mov ecx, dword ptr [ebp - 0x30]
    ecx = (var_30);
    // 00581EA0  shl edx, 4
    edx = ((edx) << (4));
    // 00581EA3  movsx esi, byte ptr [ecx]
    esi = ((*(signed char*)((char*)(ecx))));
    // 00581EA6  lea eax, [edx + 0x94f280]
    eax = (edx + 0x94f280);
    // 00581EAC  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 00581EAF  lea ecx, [esi + 1]
    ecx = (esi + 0x1);
    // 00581EB2  movsx eax, byte ptr [eax]
    eax = ((*(signed char*)((char*)(eax))));
    // 00581EB5  imul ecx, eax
    ecx = ((ecx) * (eax));
    // 00581EB8  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00581EBB  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00581EBE  mov eax, dword ptr [eax + 0x96cdcc]
    eax = ((*(int*)((char*)g_0096cdcc + eax)));
    // 00581EC4  add eax, 5
    eax = ((eax) + (5));
    // 00581EC7  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 00581ECA  mov eax, ecx
    eax = (ecx);
    // 00581ECC  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581ECD  idiv dword ptr [ebp - 0x24]
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(var_24)); edx = (int)(t_dividend % (int)(var_24)); }
    // 00581ED0  cmp eax, dword ptr [ebp - 0x34]
    // 00581ED3  jg 0x581ee8
    if (((int)(eax)) > ((int)(var_34))) goto LAB_00581EE8;
    // 00581ED5  mov ebx, dword ptr [ebp - 0x24]
    ebx = (var_24);
LAB_00581ED8:
    // 00581ED8  mov eax, dword ptr [ebp - 0x3c]
    eax = (var_3c);
    // 00581EDB  inc esi
    esi = ((esi) + 1);
    // 00581EDC  add ecx, eax
    ecx = ((ecx) + (eax));
    // 00581EDE  mov eax, ecx
    eax = (ecx);
    // 00581EE0  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581EE1  idiv ebx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ebx)); edx = (int)(t_dividend % (int)(ebx)); }
    // 00581EE3  cmp eax, dword ptr [ebp - 0x34]
    // 00581EE6  jle 0x581ed8
    if (((int)(eax)) <= ((int)(var_34))) goto LAB_00581ED8;
LAB_00581EE8:
    // 00581EE8  push 0
    // 00581EEA  push esi
    // 00581EEB  push edi
    // 00581EEC  call 0x57f0b0
    eax = weapon_budget(edi, esi, 0);
    // 00581EF1  mov bl, byte ptr [ebp - 0x20]
    SETB0(ebx, ((var_20 & 0xff)));
    // 00581EF4  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 00581EF7  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00581EFA  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00581EFF  add esp, 0xc
    // 00581F02  test bl, al
    // 00581F04  jne 0x581f37
    if (((((ebx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00581F37;
    // 00581F06  mov ecx, dword ptr [esi + 0x96ea74]
    ecx = ((*(int*)((char*)g_0096ea74 + esi)));
    // 00581F0C  mov eax, dword ptr [esi + 0x96d228]
    eax = ((*(int*)((char*)g_0096d228 + esi)));
    // 00581F12  shl ecx, 1
    ecx = ((ecx) << (1));
    // 00581F14  cmp eax, ecx
    // 00581F16  jle 0x581f37
    if (((int)(eax)) <= ((int)(ecx))) goto LAB_00581F37;
    // 00581F18  cmp eax, 1
    // 00581F1B  jle 0x581f37
    if (((int)(eax)) <= ((int)(1))) goto LAB_00581F37;
    // 00581F1D  cmp eax, dword ptr [esi + 0x96ea78]
    // 00581F23  jle 0x581f37
    if (((int)(eax)) <= ((int)((*(int*)((char*)g_0096ea78 + esi))))) goto LAB_00581F37;
    // 00581F25  push edi
    // 00581F26  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00581F29  push 0x100
    // 00581F2E  push edx
    // 00581F2F  call 0x581190
    add_abil((int*)(edx), 0x100, edi);
    // 00581F34  add esp, 0xc
LAB_00581F37:
    // 00581F37  mov eax, dword ptr [0x9a6638]
    eax = ((*(int*)((char*)g_009a6638)));
    // 00581F3C  mov ecx, dword ptr [0x9a663c]
    ecx = ((*(int*)((char*)g_009a663c)));
    // 00581F42  cmp eax, ecx
    // 00581F44  jle 0x581f72
    if (((int)(eax)) <= ((int)(ecx))) goto LAB_00581F72;
    // 00581F46  mov ecx, dword ptr [esi + 0x96cdbc]
    ecx = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00581F4C  mov edx, dword ptr [esi + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + esi)));
    // 00581F52  add ecx, edx
    ecx = ((ecx) + (edx));
    // 00581F54  test ecx, ecx
    // 00581F56  jle 0x581f72
    if (((int)(((ecx) & (ecx)))) <= 0) goto LAB_00581F72;
    // 00581F58  test byte ptr [0x9a64e8], bl
    // 00581F5E  jne 0x581f72
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_00581F72;
    // 00581F60  push edi
    // 00581F61  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00581F64  push 0x100000
    // 00581F69  push edx
    // 00581F6A  call 0x581190
    add_abil((int*)(edx), 0x100000, edi);
    // 00581F6F  add esp, 0xc
LAB_00581F72:
    // 00581F72  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00581F75  test ecx, ecx
    // 00581F77  je 0x581fa8
    if ((((ecx) & (ecx))) == 0) goto LAB_00581FA8;
    // 00581F79  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00581F7F  mov edx, 1
    edx = (1);
    // 00581F84  sub edx, eax
    edx = ((edx) - (eax));
    // 00581F86  cmp ecx, edx
    // 00581F88  jle 0x581fa8
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_00581FA8;
    // 00581F8A  test eax, eax
    // 00581F8C  jl 0x581fa8
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00581FA8;
    // 00581F8E  test byte ptr [0x9a64e8], bl
    // 00581F94  jne 0x581ff7
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_00581FF7;
    // 00581F96  push edi
    // 00581F97  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00581F9A  push 0x100000
    // 00581F9F  push eax
    // 00581FA0  call 0x581190
    add_abil((int*)(eax), 0x100000, edi);
    // 00581FA5  add esp, 0xc
LAB_00581FA8:
    // 00581FA8  test byte ptr [0x9a64e8], bl
    // 00581FAE  jne 0x581ff7
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_00581FF7;
    // 00581FB0  mov eax, dword ptr [esi + 0x96d204]
    eax = ((*(int*)((char*)g_0096d204 + esi)));
    // 00581FB6  mov ecx, dword ptr [esi + 0x96d21c]
    ecx = ((*(int*)((char*)g_0096d21c + esi)));
    // 00581FBC  lea eax, [eax + eax*2]
    eax = (eax + eax*2);
    // 00581FBF  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581FC0  sub eax, edx
    eax = ((eax) - (edx));
    // 00581FC2  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581FC4  cmp ecx, eax
    // 00581FC6  jl 0x581fde
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00581FDE;
    // 00581FC8  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00581FCB  shl edx, 4
    edx = ((edx) << (4));
    // 00581FCE  movsx eax, byte ptr [edx + 0x94ae68]
    eax = ((*(signed char*)((char*)g_0094ae68 + edx)));
    // 00581FD5  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00581FD6  sub eax, edx
    eax = ((eax) - (edx));
    // 00581FD8  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00581FDA  cmp ecx, eax
    // 00581FDC  jg 0x581fe5
    if (((int)(ecx)) > ((int)(eax))) goto LAB_00581FE5;
LAB_00581FDE:
    // 00581FDE  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00581FE1  test eax, eax
    // 00581FE3  je 0x581ff7
    if ((((eax) & (eax))) == 0) goto LAB_00581FF7;
LAB_00581FE5:
    // 00581FE5  push edi
    // 00581FE6  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00581FE9  push 0x800
    // 00581FEE  push eax
    // 00581FEF  call 0x581190
    add_abil((int*)(eax), 0x800, edi);
    // 00581FF4  add esp, 0xc
LAB_00581FF7:
    // 00581FF7  cmp dword ptr [esi + 0x96da3c], 8
    // 00581FFE  jl 0x58202f
    if (((int)((*(int*)((char*)g_0096da3c + esi)))) < ((int)(8))) goto LAB_0058202F;
    // 00582000  cmp dword ptr [esi + 0x96c9e4], 6
    // 00582007  jl 0x58202f
    if (((int)((*(int*)((char*)g_0096c9e4 + esi)))) < ((int)(6))) goto LAB_0058202F;
    // 00582009  mov eax, dword ptr [esi + 0x96cdc8]
    eax = ((*(int*)((char*)g_0096cdc8 + esi)));
    // 0058200F  test eax, eax
    // 00582011  jne 0x58201d
    if ((((eax) & (eax))) != 0) goto LAB_0058201D;
    // 00582013  mov eax, dword ptr [esi + 0x96cdc4]
    eax = ((*(int*)((char*)g_0096cdc4 + esi)));
    // 00582019  test eax, eax
    // 0058201B  je 0x58202f
    if ((((eax) & (eax))) == 0) goto LAB_0058202F;
LAB_0058201D:
    // 0058201D  push edi
    // 0058201E  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00582021  push 0x10000
    // 00582026  push ecx
    // 00582027  call 0x581190
    add_abil((int*)(ecx), 0x10000, edi);
    // 0058202C  add esp, 0xc
LAB_0058202F:
    // 0058202F  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 00582032  mov eax, dword ptr [edx + 0x946f28]
    eax = ((*(int*)((char*)g_00946f28 + edx)));
    // 00582038  test eax, eax
    // 0058203A  jge 0x58204e
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_0058204E;
    // 0058203C  push edi
    // 0058203D  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00582040  push 0x4000
    // 00582045  push eax
    // 00582046  call 0x581190
    add_abil((int*)(eax), 0x4000, edi);
    // 0058204B  add esp, 0xc
LAB_0058204E:
    // 0058204E  mov ebx, dword ptr [ebp - 0x30]
    ebx = (var_30);
    // 00582051  cmp byte ptr [ebx], 1
    // 00582054  jle 0x582094
    if (((int)((*(unsigned char*)((char*)(ebx))))) <= ((int)(1))) goto LAB_00582094;
    // 00582056  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582059  test eax, eax
    // 0058205B  jne 0x5820ce
    if ((((eax) & (eax))) != 0) goto LAB_005820CE;
    // 0058205D  mov eax, dword ptr [esi + 0x96cdc4]
    eax = ((*(int*)((char*)g_0096cdc4 + esi)));
    // 00582063  test eax, eax
    // 00582065  jle 0x58209b
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_0058209B;
    // 00582067  cmp dword ptr [esi + 0x96d214], 1
    // 0058206E  jle 0x58209b
    if (((int)((*(int*)((char*)g_0096d214 + esi)))) <= ((int)(1))) goto LAB_0058209B;
    // 00582070  mov ecx, dword ptr [esi + 0x96d204]
    ecx = ((*(int*)((char*)g_0096d204 + esi)));
    // 00582076  mov eax, dword ptr [esi + 0x96d20c]
    eax = ((*(int*)((char*)g_0096d20c + esi)));
    // 0058207C  shl ecx, 1
    ecx = ((ecx) << (1));
    // 0058207E  cmp eax, ecx
    // 00582080  jl 0x58209b
    if (((int)(eax)) < ((int)(ecx))) goto LAB_0058209B;
    // 00582082  push edi
    // 00582083  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00582086  push 0x200
    // 0058208B  push edx
    // 0058208C  call 0x581190
    add_abil((int*)(edx), 0x200, edi);
    // 00582091  add esp, 0xc
LAB_00582094:
    // 00582094  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582097  test eax, eax
    // 00582099  jne 0x5820ce
    if ((((eax) & (eax))) != 0) goto LAB_005820CE;
LAB_0058209B:
    // 0058209B  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 005820A1  test eax, eax
    // 005820A3  jl 0x5820bc
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005820BC;
    // 005820A5  mov eax, dword ptr [esi + 0x96cc3c]
    eax = ((*(int*)((char*)g_0096cc3c + esi)));
    // 005820AB  test eax, eax
    // 005820AD  jge 0x5820ce
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005820CE;
    // 005820AF  mov al, byte ptr [ebp - 0x20]
    SETB0(eax, ((var_20 & 0xff)));
    // 005820B2  mov cl, byte ptr [0x9a64e8]
    SETB0(ecx, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005820B8  test al, cl
    // 005820BA  jne 0x5820ce
    if (((((eax & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005820CE;
LAB_005820BC:
    // 005820BC  push edi
    // 005820BD  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005820C0  push 0x4000
    // 005820C5  push ecx
    // 005820C6  call 0x581190
    add_abil((int*)(ecx), 0x4000, edi);
    // 005820CB  add esp, 0xc
LAB_005820CE:
    // 005820CE  mov eax, 0x400
    eax = (0x400);
    // 005820D3  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005820D5:
    // 005820D5  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005820D7  inc ecx
    ecx = ((ecx) + 1);
    // 005820D8  test al, 1
    // 005820DA  je 0x5820d5
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005820D5;
    // 005820DC  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005820E3  push edi
    // 005820E4  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005820E6  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005820EE  push eax
    // 005820EF  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005820F4  add esp, 8
    // 005820F7  test eax, eax
    // 005820F9  je 0x58211f
    if ((((eax) & (eax))) == 0) goto LAB_0058211F;
    // 005820FB  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005820FE  test eax, eax
    // 00582100  je 0x582116
    if ((((eax) & (eax))) == 0) goto LAB_00582116;
    // 00582102  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00582108  push edi
    // 00582109  push ecx
    // 0058210A  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 0058210F  add esp, 8
    // 00582112  test eax, eax
    // 00582114  je 0x58211f
    if ((((eax) & (eax))) == 0) goto LAB_0058211F;
LAB_00582116:
    // 00582116  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582119  or ah, 4
    SETB1(eax, ((((eax >> 8) & 0xff)) | (4)));
    // 0058211C  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_0058211F:
    // 0058211F  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00582122  shl edx, 4
    edx = ((edx) << (4));
    // 00582125  mov al, byte ptr [edx + 0x94ae68]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094ae68 + edx))));
    // 0058212B  cmp al, 1
    // 0058212D  jle 0x582134
    if (((int)((eax & 0xff))) <= ((int)(1))) goto LAB_00582134;
    // 0058212F  cmp byte ptr [ebx], 1
    // 00582132  jg 0x582145
    if (((int)((*(unsigned char*)((char*)(ebx))))) > ((int)(1))) goto LAB_00582145;
LAB_00582134:
    // 00582134  cmp al, 2
    // 00582136  jg 0x582145
    if (((int)((eax & 0xff))) > ((int)(2))) goto LAB_00582145;
    // 00582138  mov eax, dword ptr [0x93fc54]
    eax = ((*(int*)((char*)g_0093fc54)));
    // 0058213D  test eax, eax
    // 0058213F  je 0x582218
    if ((((eax) & (eax))) == 0) goto LAB_00582218;
LAB_00582145:
    // 00582145  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00582148  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0058214B  mov esi, dword ptr [ebp - 0x10]
    esi = (var_10);
    // 0058214E  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00582151  mov ebx, dword ptr [ebp - 4]
    ebx = (var_4);
    // 00582154  push 0
    // 00582156  push 3
    // 00582158  push eax
    // 00582159  push ecx
    // 0058215A  push esi
    // 0058215B  push edx
    // 0058215C  push ebx
    // 0058215D  push edi
    // 0058215E  call 0x580860
    eax = propose_proto(edi, ebx, edx, esi, ecx, eax, 3, 0);
    // 00582163  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582166  push edi
    // 00582167  and ah, 0xfb
    SETB1(eax, ((((eax >> 8) & 0xff)) & (0xfb)));
    // 0058216A  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0058216D  mov eax, dword ptr [0x949780]
    eax = ((*(int*)((char*)g_00949780)));
    // 00582172  push eax
    // 00582173  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582178  add esp, 0x28
    // 0058217B  test eax, eax
    // 0058217D  jne 0x582182
    if ((((eax) & (eax))) != 0) goto LAB_00582182;
    // 0058217F  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00582182:
    // 00582182  mov eax, 0x10
    eax = (0x10);
    // 00582187  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582189:
    // 00582189  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0058218B  inc ecx
    ecx = ((ecx) + 1);
    // 0058218C  test al, 1
    // 0058218E  je 0x582189
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582189;
    // 00582190  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582197  push edi
    // 00582198  sub edx, ecx
    edx = ((edx) - (ecx));
    // 0058219A  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005821A2  push eax
    // 005821A3  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005821A8  add esp, 8
    // 005821AB  test eax, eax
    // 005821AD  je 0x5821cd
    if ((((eax) & (eax))) == 0) goto LAB_005821CD;
    // 005821AF  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005821B2  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005821B5  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 005821B8  push 0
    // 005821BA  push 0xb
    // 005821BC  or edx, 0x10
    edx = ((edx) | (0x10));
    // 005821BF  push ecx
    // 005821C0  push edx
    // 005821C1  push esi
    // 005821C2  push eax
    // 005821C3  push ebx
    // 005821C4  push edi
    // 005821C5  call 0x580860
    eax = propose_proto(edi, ebx, eax, esi, edx, ecx, 0xb, 0);
    // 005821CA  add esp, 0x20
LAB_005821CD:
    // 005821CD  mov eax, 8
    eax = (8);
    // 005821D2  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005821D4:
    // 005821D4  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005821D6  inc ecx
    ecx = ((ecx) + 1);
    // 005821D7  test al, 1
    // 005821D9  je 0x5821d4
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005821D4;
    // 005821DB  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005821E2  push edi
    // 005821E3  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005821E5  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005821ED  push eax
    // 005821EE  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005821F3  add esp, 8
    // 005821F6  test eax, eax
    // 005821F8  je 0x582218
    if ((((eax) & (eax))) == 0) goto LAB_00582218;
    // 005821FA  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005821FD  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00582200  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00582203  push 0
    // 00582205  push 0xc
    // 00582207  or edx, 8
    edx = ((edx) | (8));
    // 0058220A  push ecx
    // 0058220B  push edx
    // 0058220C  push esi
    // 0058220D  push eax
    // 0058220E  push ebx
    // 0058220F  push edi
    // 00582210  call 0x580860
    eax = propose_proto(edi, ebx, eax, esi, edx, ecx, 0xc, 0);
    // 00582215  add esp, 0x20
LAB_00582218:
    // 00582218  push 0
    // 0058221A  push 0x63
    // 0058221C  push edi
    // 0058221D  call 0x57f0b0
    eax = weapon_budget(edi, 0x63, 0);
    // 00582222  add esp, 0xc
    // 00582225  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 00582228  mov ecx, 0x8000
    ecx = (0x8000);
    // 0058222D  xor eax, eax
    eax = ((eax) ^ (eax));
LAB_0058222F:
    // 0058222F  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 00582231  inc eax
    eax = ((eax) + 1);
    // 00582232  test cl, 1
    // 00582235  je 0x58222f
    if (((((ecx & 0xff)) & (1))) == 0) goto LAB_0058222F;
    // 00582237  lea ecx, [eax*8]
    ecx = (eax*8);
    // 0058223E  push edi
    // 0058223F  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 00582241  movsx edx, word ptr [ecx*4 + 0x9ab550]
    edx = ((*(short*)((char*)g_009ab550 + ecx*4)));
    // 00582249  push edx
    // 0058224A  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 0058224F  add esp, 8
    // 00582252  test eax, eax
    // 00582254  je 0x5825fb
    if ((((eax) & (eax))) == 0) goto LAB_005825FB;
    // 0058225A  push 1
    // 0058225C  push 0
    // 0058225E  push edi
    // 0058225F  call 0x57eff0
    eax = pick_chassis(edi, 0, 1);
    // 00582264  add esp, 0xc
    // 00582267  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058226A  test eax, eax
    // 0058226C  jl 0x5825fb
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005825FB;
    // 00582272  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00582274  mov dword ptr [ebp - 0x34], 0xffffffff
    var_34 = (0xffffffff);
    // 0058227B  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 0058227E  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
    // 00582281  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00582286:
    // 00582286  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 0058228A  push edi
    // 0058228B  push eax
    // 0058228C  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582291  add esp, 8
    // 00582294  test eax, eax
    // 00582296  je 0x5822d3
    if ((((eax) & (eax))) == 0) goto LAB_005822D3;
    // 00582298  cmp byte ptr [esi + 2], 1
    // 0058229C  ja 0x5822d3
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_005822D3;
    // 0058229E  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 005822A0  test al, al
    // 005822A2  jl 0x5822d3
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_005822D3;
    // 005822A4  test edi, edi
    // 005822A6  jl 0x5822c1
    if (((int)(((edi) & (edi)))) < 0) goto LAB_005822C1;
    // 005822A8  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 005822AE  test ecx, ecx
    // 005822B0  jne 0x5822c1
    if ((((ecx) & (ecx))) != 0) goto LAB_005822C1;
    // 005822B2  cmp esi, 0x94f310
    // 005822B8  jle 0x5822c1
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_005822C1;
    // 005822BA  mov eax, 1
    eax = (1);
    // 005822BF  jmp 0x5822c4
    goto LAB_005822C4;
LAB_005822C1:
    // 005822C1  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_005822C4:
    // 005822C4  mov ecx, dword ptr [ebp - 0x34]
    ecx = (var_34);
    // 005822C7  add eax, eax
    eax = ((eax) + (eax));
    // 005822C9  cmp eax, ecx
    // 005822CB  jl 0x5822d3
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005822D3;
    // 005822CD  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 005822D0  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
LAB_005822D3:
    // 005822D3  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005822D6  inc ebx
    ebx = ((ebx) + 1);
    // 005822D7  cmp esi, 0x94f360
    // 005822DD  jl 0x582286
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00582286;
    // 005822DF  mov ecx, dword ptr [ebp - 0x30]
    ecx = (var_30);
    // 005822E2  mov dword ptr [ebp - 0x24], 0
    var_24 = (0);
    // 005822E9  mov dword ptr [ebp - 0x10], ecx
    var_10 = (ecx);
LAB_005822EC:
    // 005822EC  mov ebx, dword ptr [ebp - 4]
    ebx = (var_4);
    // 005822EF  mov dword ptr [ebp - 0x2c], 0
    var_2c = (0);
    // 005822F6  lea edx, [ebx + ebx*8]
    edx = (ebx + ebx*8);
    // 005822F9  shl edx, 4
    edx = ((edx) << (4));
    // 005822FC  cmp byte ptr [edx + 0x94a378], 1
    // 00582303  jne 0x5823c9
    if (((*(unsigned char*)((char*)g_0094a378 + edx))) != (1)) goto LAB_005823C9;
    // 00582309  push 2
    // 0058230B  push 0
    // 0058230D  push edi
    // 0058230E  call 0x57eff0
    eax = pick_chassis(edi, 0, 2);
    // 00582313  mov esi, eax
    esi = (eax);
    // 00582315  add esp, 0xc
    // 00582318  test esi, esi
    // 0058231A  jl 0x5823c9
    if (((int)(((esi) & (esi)))) < 0) goto LAB_005823C9;
    // 00582320  cmp esi, ebx
    // 00582322  je 0x5823c9
    if ((esi) == (ebx)) goto LAB_005823C9;
    // 00582328  mov ebx, dword ptr [ebp - 8]
    ebx = (var_8);
    // 0058232B  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0058232E  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00582331  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 00582334  push ebx
    // 00582335  push eax
    // 00582336  push ecx
    // 00582337  push edx
    // 00582338  push esi
    // 00582339  call 0x5a5a60
    eax = proto_cost(esi, edx, ecx, eax, ebx);
    // 0058233E  lea ecx, [ebx + 5]
    ecx = (ebx + 0x5);
    // 00582341  add esp, 0x14
    // 00582344  cmp eax, ecx
    // 00582346  jg 0x582387
    if (((int)(eax)) > ((int)(ecx))) goto LAB_00582387;
    // 00582348  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 0058234B  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 0058234E  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 00582351  push ebx
    // 00582352  push edx
    // 00582353  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00582356  push eax
    // 00582357  push ecx
    // 00582358  push edx
    // 00582359  call 0x5a5a60
    eax = proto_cost(edx, ecx, eax, edx, ebx);
    // 0058235E  mov ecx, edi
    ecx = (edi);
    // 00582360  push ebx
    // 00582361  and ecx, 1
    ecx = ((ecx) & (1));
    // 00582364  lea edx, [eax + ecx + 1]
    edx = (ecx + 0x1);
    // 00582368  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0058236B  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 0058236E  push eax
    // 0058236F  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 00582372  push ecx
    // 00582373  push eax
    // 00582374  push esi
    // 00582375  mov dword ptr [ebp - 0x3c], edx
    var_3c = (edx);
    // 00582378  call 0x5a5a60
    eax = proto_cost(esi, eax, ecx, eax, ebx);
    // 0058237D  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00582380  add esp, 0x28
    // 00582383  cmp eax, ecx
    // 00582385  jle 0x5823bf
    if (((int)(eax)) <= ((int)(ecx))) goto LAB_005823BF;
LAB_00582387:
    // 00582387  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 0058238A  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 0058238D  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 00582390  push ebx
    // 00582391  push edx
    // 00582392  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00582395  push eax
    // 00582396  push ecx
    // 00582397  push edx
    // 00582398  call 0x5a5a60
    eax = proto_cost(edx, ecx, eax, edx, ebx);
    // 0058239D  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 005823A0  mov edx, eax
    edx = (eax);
    // 005823A2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005823A5  push ebx
    // 005823A6  push eax
    // 005823A7  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005823AA  push ecx
    // 005823AB  push eax
    // 005823AC  push esi
    // 005823AD  mov dword ptr [ebp - 0x3c], edx
    var_3c = (edx);
    // 005823B0  call 0x5a5a60
    eax = proto_cost(esi, eax, ecx, eax, ebx);
    // 005823B5  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005823B8  add esp, 0x28
    // 005823BB  cmp eax, ecx
    // 005823BD  jg 0x5823c9
    if (((int)(eax)) > ((int)(ecx))) goto LAB_005823C9;
LAB_005823BF:
    // 005823BF  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 005823C2  mov dword ptr [ebp - 0x2c], 1
    var_2c = (1);
LAB_005823C9:
    // 005823C9  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005823CC  mov dword ptr [ebp - 0x30], 0xffffffff
    var_30 = (0xffffffff);
    // 005823D3  shl edx, 4
    edx = ((edx) << (4));
    // 005823D6  mov esi, 0x94f280
    esi = (0x94f280);
    // 005823DB  movsx ebx, byte ptr [edx + 0x94f280]
    ebx = ((*(signed char*)((char*)g_0094f280 + edx)));
    // 005823E2  lea eax, [edx + 0x94f280]
    eax = (edx + 0x94f280);
    // 005823E8  inc ebx
    ebx = ((ebx) + 1);
    // 005823E9  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 005823EC  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005823EE  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 005823F1  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
LAB_005823F4:
    // 005823F4  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 005823F8  push edi
    // 005823F9  push eax
    // 005823FA  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005823FF  add esp, 8
    // 00582402  test eax, eax
    // 00582404  je 0x582447
    if ((((eax) & (eax))) == 0) goto LAB_00582447;
    // 00582406  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00582408  mov cl, byte ptr [esi + 2]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + 0x2))));
    // 0058240B  cmp ecx, ebx
    // 0058240D  jg 0x582447
    if (((int)(ecx)) > ((int)(ebx))) goto LAB_00582447;
    // 0058240F  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00582411  test al, al
    // 00582413  jl 0x582447
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00582447;
    // 00582415  test edi, edi
    // 00582417  jl 0x582432
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00582432;
    // 00582419  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 0058241F  test ecx, ecx
    // 00582421  jne 0x582432
    if ((((ecx) & (ecx))) != 0) goto LAB_00582432;
    // 00582423  cmp esi, 0x94f310
    // 00582429  jle 0x582432
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00582432;
    // 0058242B  mov eax, 1
    eax = (1);
    // 00582430  jmp 0x582435
    goto LAB_00582435;
LAB_00582432:
    // 00582432  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00582435:
    // 00582435  mov ecx, dword ptr [ebp - 0x30]
    ecx = (var_30);
    // 00582438  add eax, eax
    eax = ((eax) + (eax));
    // 0058243A  cmp eax, ecx
    // 0058243C  jl 0x582447
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00582447;
    // 0058243E  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00582441  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 00582444  mov dword ptr [ebp - 0x20], edx
    var_20 = (edx);
LAB_00582447:
    // 00582447  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 0058244A  add esi, 0x10
    esi = ((esi) + (0x10));
    // 0058244D  inc edx
    edx = ((edx) + 1);
    // 0058244E  cmp esi, 0x94f360
    // 00582454  mov dword ptr [ebp - 0xc], edx
    var_c = (edx);
    // 00582457  jl 0x5823f4
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_005823F4;
    // 00582459  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 0058245C  mov edx, dword ptr [ebp - 0x34]
    edx = (var_34);
    // 0058245F  shl eax, 4
    eax = ((eax) << (4));
    // 00582462  mov cl, byte ptr [eax + 0x94f280]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094f280 + eax))));
    // 00582468  mov al, byte ptr [edx]
    SETB0(eax, ((*(unsigned char*)((char*)(edx)))));
    // 0058246A  cmp cl, al
    // 0058246C  jle 0x5824b1
    if (((signed char)(ecx & 0xff)) <= ((signed char)(eax & 0xff))) goto LAB_005824B1;
    // 0058246E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00582471  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00582474  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 00582477  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 0058247A  mov ebx, dword ptr [ebp - 4]
    ebx = (var_4);
    // 0058247D  push eax
    // 0058247E  push ecx
    // 0058247F  push edx
    // 00582480  push esi
    // 00582481  push ebx
    // 00582482  call 0x5a5a60
    eax = proto_cost(ebx, esi, edx, ecx, eax);
    // 00582487  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0058248A  mov edx, eax
    edx = (eax);
    // 0058248C  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 0058248F  mov dword ptr [ebp - 0x3c], edx
    var_3c = (edx);
    // 00582492  push eax
    // 00582493  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 00582496  push ecx
    // 00582497  push eax
    // 00582498  push esi
    // 00582499  push ebx
    // 0058249A  call 0x5a5a60
    eax = proto_cost(ebx, esi, eax, ecx, eax);
    // 0058249F  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005824A2  add esp, 0x28
    // 005824A5  cmp eax, ecx
    // 005824A7  jg 0x5824b4
    if (((int)(eax)) > ((int)(ecx))) goto LAB_005824B4;
    // 005824A9  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005824AC  mov dword ptr [ebp - 0x10], edx
    var_10 = (edx);
    // 005824AF  jmp 0x5824bb
    goto LAB_005824BB;
LAB_005824B1:
    // 005824B1  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
LAB_005824B4:
    // 005824B4  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005824B7  test eax, eax
    // 005824B9  je 0x5824ce
    if ((((eax) & (eax))) == 0) goto LAB_005824CE;
LAB_005824BB:
    // 005824BB  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 005824BE  inc eax
    eax = ((eax) + 1);
    // 005824BF  cmp eax, 0x64
    // 005824C2  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 005824C5  jl 0x5822ec
    if (((int)(eax)) < ((int)(0x64))) goto LAB_005822EC;
    // 005824CB  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
LAB_005824CE:
    // 005824CE  mov eax, 0x8000
    eax = (0x8000);
    // 005824D3  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005824D5:
    // 005824D5  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005824D7  inc ecx
    ecx = ((ecx) + 1);
    // 005824D8  test al, 1
    // 005824DA  je 0x5824d5
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005824D5;
    // 005824DC  lea eax, [ecx*8]
    eax = (ecx*8);
    // 005824E3  push edi
    // 005824E4  sub eax, ecx
    eax = ((eax) - (ecx));
    // 005824E6  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 005824EE  push ecx
    // 005824EF  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005824F4  add esp, 8
    // 005824F7  test eax, eax
    // 005824F9  je 0x58250f
    if ((((eax) & (eax))) == 0) goto LAB_0058250F;
    // 005824FB  mov edx, dword ptr [0x949780]
    edx = ((*(int*)((char*)g_00949780)));
    // 00582501  push edi
    // 00582502  push edx
    // 00582503  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00582508  add esp, 8
    // 0058250B  test eax, eax
    // 0058250D  je 0x582552
    if ((((eax) & (eax))) == 0) goto LAB_00582552;
LAB_0058250F:
    // 0058250F  push edi
    // 00582510  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00582513  push 0x20000
    // 00582518  push eax
    // 00582519  call 0x581190
    add_abil((int*)(eax), 0x20000, edi);
    // 0058251E  push edi
    // 0058251F  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00582522  push 0x400
    // 00582527  push ecx
    // 00582528  call 0x581190
    add_abil((int*)(ecx), 0x400, edi);
    // 0058252D  push edi
    // 0058252E  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00582531  push 2
    // 00582533  push edx
    // 00582534  call 0x581190
    add_abil((int*)(edx), 2, edi);
    // 00582539  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 0058253C  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0058253F  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 00582542  push eax
    // 00582543  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00582546  push ecx
    // 00582547  push edx
    // 00582548  push esi
    // 00582549  push eax
    // 0058254A  call 0x5a5a60
    eax = proto_cost(eax, esi, edx, ecx, eax);
    // 0058254F  add esp, 0x38
LAB_00582552:
    // 00582552  mov eax, 0x8000
    eax = (0x8000);
    // 00582557  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582559:
    // 00582559  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0058255B  inc ecx
    ecx = ((ecx) + 1);
    // 0058255C  test al, 1
    // 0058255E  je 0x582559
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582559;
    // 00582560  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582567  push edi
    // 00582568  sub edx, ecx
    edx = ((edx) - (ecx));
    // 0058256A  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582572  push eax
    // 00582573  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582578  add esp, 8
    // 0058257B  test eax, eax
    // 0058257D  je 0x582588
    if ((((eax) & (eax))) == 0) goto LAB_00582588;
    // 0058257F  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582582  or ah, 0x80
    SETB1(eax, ((((eax >> 8) & 0xff)) | (0x80)));
    // 00582585  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00582588:
    // 00582588  mov ebx, dword ptr [ebp - 0x10]
    ebx = (var_10);
    // 0058258B  mov ecx, ebx
    ecx = (ebx);
    // 0058258D  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00582590  movsx eax, byte ptr [ecx + 0x94f280]
    eax = ((*(signed char*)((char*)g_0094f280 + ecx)));
    // 00582597  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 0058259A  mov eax, esi
    eax = (esi);
    // 0058259C  shl eax, 4
    eax = ((eax) << (4));
    // 0058259F  movsx ecx, byte ptr [eax + 0x94ae68]
    ecx = ((*(signed char*)((char*)g_0094ae68 + eax)));
    // 005825A6  cmp ecx, edx
    // 005825A8  jge 0x5825e0
    if (((int)(ecx)) >= ((int)(edx))) goto LAB_005825E0;
    // 005825AA  mov eax, 0x8000
    eax = (0x8000);
    // 005825AF  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005825B1:
    // 005825B1  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005825B3  inc ecx
    ecx = ((ecx) + 1);
    // 005825B4  test al, 1
    // 005825B6  je 0x5825b1
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005825B1;
    // 005825B8  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005825BF  push edi
    // 005825C0  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005825C2  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005825CA  push eax
    // 005825CB  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005825D0  add esp, 8
    // 005825D3  test eax, eax
    // 005825D5  jne 0x5825e0
    if ((((eax) & (eax))) != 0) goto LAB_005825E0;
    // 005825D7  mov eax, dword ptr [0x93fc54]
    eax = ((*(int*)((char*)g_0093fc54)));
    // 005825DC  test eax, eax
    // 005825DE  je 0x5825fb
    if ((((eax) & (eax))) == 0) goto LAB_005825FB;
LAB_005825E0:
    // 005825E0  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005825E3  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005825E6  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005825E9  push 0
    // 005825EB  push 1
    // 005825ED  push ecx
    // 005825EE  push edx
    // 005825EF  push ebx
    // 005825F0  push esi
    // 005825F1  push eax
    // 005825F2  push edi
    // 005825F3  call 0x580860
    eax = propose_proto(edi, eax, esi, ebx, edx, ecx, 1, 0);
    // 005825F8  add esp, 0x20
LAB_005825FB:
    // 005825FB  push 3
    // 005825FD  push 0
    // 005825FF  push edi
    // 00582600  call 0x57eff0
    eax = pick_chassis(edi, 0, 3);
    // 00582605  add esp, 0xc
    // 00582608  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058260B  test eax, eax
    // 0058260D  jge 0x582627
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00582627;
    // 0058260F  push 2
    // 00582611  push 0
    // 00582613  push edi
    // 00582614  call 0x57eff0
    eax = pick_chassis(edi, 0, 2);
    // 00582619  add esp, 0xc
    // 0058261C  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058261F  test eax, eax
    // 00582621  jl 0x582969
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00582969;
LAB_00582627:
    // 00582627  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00582629  mov dword ptr [ebp - 0x34], 0xffffffff
    var_34 = (0xffffffff);
    // 00582630  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 00582633  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
    // 00582636  mov esi, 0x94f280
    esi = (0x94f280);
LAB_0058263B:
    // 0058263B  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 0058263F  push edi
    // 00582640  push ecx
    // 00582641  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582646  add esp, 8
    // 00582649  test eax, eax
    // 0058264B  je 0x582688
    if ((((eax) & (eax))) == 0) goto LAB_00582688;
    // 0058264D  cmp byte ptr [esi + 2], 1
    // 00582651  ja 0x582688
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00582688;
    // 00582653  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00582655  test al, al
    // 00582657  jl 0x582688
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00582688;
    // 00582659  test edi, edi
    // 0058265B  jl 0x582676
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00582676;
    // 0058265D  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00582663  test ecx, ecx
    // 00582665  jne 0x582676
    if ((((ecx) & (ecx))) != 0) goto LAB_00582676;
    // 00582667  cmp esi, 0x94f310
    // 0058266D  jle 0x582676
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00582676;
    // 0058266F  mov eax, 1
    eax = (1);
    // 00582674  jmp 0x582679
    goto LAB_00582679;
LAB_00582676:
    // 00582676  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00582679:
    // 00582679  mov ecx, dword ptr [ebp - 0x34]
    ecx = (var_34);
    // 0058267C  add eax, eax
    eax = ((eax) + (eax));
    // 0058267E  cmp eax, ecx
    // 00582680  jl 0x582688
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00582688;
    // 00582682  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 00582685  mov dword ptr [ebp - 0x30], ebx
    var_30 = (ebx);
LAB_00582688:
    // 00582688  add esi, 0x10
    esi = ((esi) + (0x10));
    // 0058268B  inc ebx
    ebx = ((ebx) + 1);
    // 0058268C  cmp esi, 0x94f360
    // 00582692  jl 0x58263b
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_0058263B;
    // 00582694  mov edx, dword ptr [ebp - 0x30]
    edx = (var_30);
    // 00582697  mov eax, edi
    eax = (edi);
    // 00582699  shl eax, 6
    eax = ((eax) << (6));
    // 0058269C  add eax, edi
    eax = ((eax) + (edi));
    // 0058269E  mov dword ptr [ebp - 0x10], edx
    var_10 = (edx);
    // 005826A1  xor esi, esi
    esi = ((esi) ^ (esi));
    // 005826A3  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 005826A6  mov dword ptr [ebp - 0x24], esi
    var_24 = (esi);
    // 005826A9  lea edx, [edi + ecx*8]
    edx = (edi + ecx*8);
    // 005826AC  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 005826AF  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005826B2  shl eax, 2
    eax = ((eax) << (2));
    // 005826B5  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 005826B8  mov ecx, dword ptr [eax + 0x96cdcc]
    ecx = ((*(int*)((char*)g_0096cdcc + eax)));
    // 005826BE  lea ecx, [ecx + edx*2]
    ecx = (ecx + edx*2);
    // 005826C1  mov edx, dword ptr [eax + 0x96cdbc]
    edx = ((*(int*)((char*)g_0096cdbc + eax)));
    // 005826C7  lea ebx, [ecx + edx + 3]
    ebx = (edx + 0x3);
    // 005826CB  mov ecx, 2
    ecx = (2);
LAB_005826D0:
    // 005826D0  add ecx, 2
    ecx = ((ecx) + (2));
    // 005826D3  inc esi
    esi = ((esi) + 1);
    // 005826D4  mov eax, ecx
    eax = (ecx);
    // 005826D6  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 005826D7  idiv dword ptr [ebp - 0x28]
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(var_28)); edx = (int)(t_dividend % (int)(var_28)); }
    // 005826DA  cmp eax, ebx
    // 005826DC  jle 0x5826d0
    if (((int)(eax)) <= ((int)(ebx))) goto LAB_005826D0;
    // 005826DE  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005826E3  mov ebx, 1
    ebx = (1);
    // 005826E8  mov ecx, edi
    ecx = (edi);
    // 005826EA  shl ebx, cl
    ebx = ((ebx) << ((ecx & 0xff)));
    // 005826EC  test bl, al
    // 005826EE  mov dword ptr [ebp - 0x20], ebx
    var_20 = (ebx);
    // 005826F1  je 0x5826f8
    if (((((ebx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_005826F8;
    // 005826F3  mov esi, 0x63
    esi = (0x63);
LAB_005826F8:
    // 005826F8  push 0
    // 005826FA  push esi
    // 005826FB  push edi
    // 005826FC  call 0x57f0b0
    eax = weapon_budget(edi, esi, 0);
    // 00582701  add esp, 0xc
    // 00582704  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00582707  shl eax, 4
    eax = ((eax) << (4));
    // 0058270A  lea eax, [eax + 0x94ae68]
    eax = (eax + 0x94ae68);
    // 00582710  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 00582713  jmp 0x582718
    goto LAB_00582718;
LAB_00582715:
    // 00582715  mov ebx, dword ptr [ebp - 0x20]
    ebx = (var_20);
LAB_00582718:
    // 00582718  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 0058271B  mov ecx, dword ptr [ebp - 0x30]
    ecx = (var_30);
    // 0058271E  shl edx, 4
    edx = ((edx) << (4));
    // 00582721  movsx eax, byte ptr [ecx]
    eax = ((*(signed char*)((char*)(ecx))));
    // 00582724  lea ecx, [edx + 0x94f280]
    ecx = (edx + 0x94f280);
    // 0058272A  mov dword ptr [ebp - 0x34], ecx
    var_34 = (ecx);
    // 0058272D  lea edx, [eax - 1]
    edx = (eax + (-0x1));
    // 00582730  movsx ecx, byte ptr [ecx]
    ecx = ((*(signed char*)((char*)(ecx))));
    // 00582733  cmp ecx, edx
    // 00582735  jge 0x58281e
    if (((int)(ecx)) >= ((int)(edx))) goto LAB_0058281E;
    // 0058273B  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 0058273C  sub eax, edx
    eax = ((eax) - (edx));
    // 0058273E  inc ecx
    ecx = ((ecx) + 1);
    // 0058273F  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582741  cmp eax, ecx
    // 00582743  mov dword ptr [ebp - 0x28], eax
    var_28 = (eax);
    // 00582746  jg 0x58274b
    if (((int)(eax)) > ((int)(ecx))) goto LAB_0058274B;
    // 00582748  mov dword ptr [ebp - 0x28], ecx
    var_28 = (ecx);
LAB_0058274B:
    // 0058274B  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 0058274D  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00582754  mov dword ptr [ebp - 0x2c], ebx
    var_2c = (ebx);
    // 00582757  mov esi, 0x94f280
    esi = (0x94f280);
LAB_0058275C:
    // 0058275C  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 00582760  push edi
    // 00582761  push eax
    // 00582762  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582767  add esp, 8
    // 0058276A  test eax, eax
    // 0058276C  je 0x5827af
    if ((((eax) & (eax))) == 0) goto LAB_005827AF;
    // 0058276E  mov eax, dword ptr [ebp - 0x28]
    eax = (var_28);
    // 00582771  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00582773  mov cl, byte ptr [esi + 2]
    SETB0(ecx, ((*(unsigned char*)((char*)(esi) + 0x2))));
    // 00582776  cmp ecx, eax
    // 00582778  jg 0x5827af
    if (((int)(ecx)) > ((int)(eax))) goto LAB_005827AF;
    // 0058277A  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 0058277C  test al, al
    // 0058277E  jl 0x5827af
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_005827AF;
    // 00582780  test edi, edi
    // 00582782  jl 0x58279d
    if (((int)(((edi) & (edi)))) < 0) goto LAB_0058279D;
    // 00582784  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 0058278A  test ecx, ecx
    // 0058278C  jne 0x58279d
    if ((((ecx) & (ecx))) != 0) goto LAB_0058279D;
    // 0058278E  cmp esi, 0x94f310
    // 00582794  jle 0x58279d
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_0058279D;
    // 00582796  mov eax, 1
    eax = (1);
    // 0058279B  jmp 0x5827a0
    goto LAB_005827A0;
LAB_0058279D:
    // 0058279D  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_005827A0:
    // 005827A0  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005827A3  add eax, eax
    eax = ((eax) + (eax));
    // 005827A5  cmp eax, ecx
    // 005827A7  jl 0x5827af
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005827AF;
    // 005827A9  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 005827AC  mov dword ptr [ebp - 0x2c], ebx
    var_2c = (ebx);
LAB_005827AF:
    // 005827AF  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005827B2  inc ebx
    ebx = ((ebx) + 1);
    // 005827B3  cmp esi, 0x94f360
    // 005827B9  jl 0x58275c
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_0058275C;
    // 005827BB  mov edx, dword ptr [ebp - 0x2c]
    edx = (var_2c);
    // 005827BE  mov ecx, dword ptr [ebp - 0x34]
    ecx = (var_34);
    // 005827C1  shl edx, 4
    edx = ((edx) << (4));
    // 005827C4  mov al, byte ptr [edx + 0x94f280]
    SETB0(eax, ((*(unsigned char*)((char*)g_0094f280 + edx))));
    // 005827CA  mov dl, byte ptr [ecx]
    SETB0(edx, ((*(unsigned char*)((char*)(ecx)))));
    // 005827CC  cmp al, dl
    // 005827CE  jle 0x58281b
    if (((signed char)(eax & 0xff)) <= ((signed char)(edx & 0xff))) goto LAB_0058281B;
    // 005827D0  mov esi, dword ptr [ebp - 8]
    esi = (var_8);
    // 005827D3  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005827D6  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 005827D9  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 005827DC  push esi
    // 005827DD  push edx
    // 005827DE  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 005827E1  push eax
    // 005827E2  push ecx
    // 005827E3  push edx
    // 005827E4  call 0x5a5a60
    eax = proto_cost(edx, ecx, eax, edx, esi);
    // 005827E9  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 005827EC  add esp, 0x14
    // 005827EF  mov edx, dword ptr [ecx + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + ecx)));
    // 005827F5  mov ebx, dword ptr [ecx + 0x96cdbc]
    ebx = ((*(int*)((char*)g_0096cdbc + ecx)));
    // 005827FB  add edx, ebx
    edx = ((edx) + (ebx));
    // 005827FD  lea ecx, [edx + esi + 3]
    ecx = (esi + 0x3);
    // 00582801  cmp eax, ecx
    // 00582803  jg 0x58281b
    if (((int)(eax)) > ((int)(ecx))) goto LAB_0058281B;
    // 00582805  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 00582808  mov edx, dword ptr [ebp - 0x2c]
    edx = (var_2c);
    // 0058280B  inc eax
    eax = ((eax) + 1);
    // 0058280C  mov dword ptr [ebp - 0x10], edx
    var_10 = (edx);
    // 0058280F  cmp eax, 0x64
    // 00582812  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 00582815  jl 0x582715
    if (((int)(eax)) < ((int)(0x64))) goto LAB_00582715;
LAB_0058281B:
    // 0058281B  mov ebx, dword ptr [ebp - 0x20]
    ebx = (var_20);
LAB_0058281E:
    // 0058281E  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00582821  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 00582824  test ecx, ecx
    // 00582826  je 0x582857
    if ((((ecx) & (ecx))) == 0) goto LAB_00582857;
    // 00582828  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 0058282E  mov edx, 3
    edx = (3);
    // 00582833  sub edx, eax
    edx = ((edx) - (eax));
    // 00582835  cmp ecx, edx
    // 00582837  jle 0x582857
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_00582857;
    // 00582839  test eax, eax
    // 0058283B  jl 0x582857
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00582857;
    // 0058283D  test byte ptr [0x9a64e8], bl
    // 00582843  jne 0x582857
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_00582857;
    // 00582845  push edi
    // 00582846  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00582849  push 0x100000
    // 0058284E  push eax
    // 0058284F  call 0x581190
    add_abil((int*)(eax), 0x100000, edi);
    // 00582854  add esp, 0xc
LAB_00582857:
    // 00582857  mov ecx, dword ptr [0x9a6638]
    ecx = ((*(int*)((char*)g_009a6638)));
    // 0058285D  mov eax, dword ptr [0x9a663c]
    eax = ((*(int*)((char*)g_009a663c)));
    // 00582862  cmp ecx, eax
    // 00582864  jle 0x582892
    if (((int)(ecx)) <= ((int)(eax))) goto LAB_00582892;
    // 00582866  mov edx, dword ptr [esi + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + esi)));
    // 0058286C  mov ecx, dword ptr [esi + 0x96cdbc]
    ecx = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00582872  add edx, ecx
    edx = ((edx) + (ecx));
    // 00582874  test edx, edx
    // 00582876  jle 0x582892
    if (((int)(((edx) & (edx)))) <= 0) goto LAB_00582892;
    // 00582878  test byte ptr [0x9a64e8], bl
    // 0058287E  jne 0x5828b2
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_005828B2;
    // 00582880  push edi
    // 00582881  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00582884  push 0x100000
    // 00582889  push eax
    // 0058288A  call 0x581190
    add_abil((int*)(eax), 0x100000, edi);
    // 0058288F  add esp, 0xc
LAB_00582892:
    // 00582892  test byte ptr [0x9a64e8], bl
    // 00582898  jne 0x5828b2
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_005828B2;
    // 0058289A  test byte ptr [ebp - 0x18], 2
    // 0058289E  je 0x5828b2
    if (((((var_18 & 0xff)) & (2))) == 0) goto LAB_005828B2;
    // 005828A0  push edi
    // 005828A1  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005828A4  push 0x800
    // 005828A9  push ecx
    // 005828AA  call 0x581190
    add_abil((int*)(ecx), 0x800, edi);
    // 005828AF  add esp, 0xc
LAB_005828B2:
    // 005828B2  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 005828B5  mov eax, dword ptr [edx + 0x946f28]
    eax = ((*(int*)((char*)g_00946f28 + edx)));
    // 005828BB  test eax, eax
    // 005828BD  jl 0x5828e5
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005828E5;
    // 005828BF  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 005828C5  test eax, eax
    // 005828C7  jle 0x5828d3
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_005828D3;
    // 005828C9  mov eax, dword ptr [esi + 0x96cc50]
    eax = ((*(int*)((char*)g_0096cc50 + esi)));
    // 005828CF  test eax, eax
    // 005828D1  jge 0x5828e5
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005828E5;
LAB_005828D3:
    // 005828D3  mov eax, dword ptr [esi + 0x96cc3c]
    eax = ((*(int*)((char*)g_0096cc3c + esi)));
    // 005828D9  test eax, eax
    // 005828DB  jge 0x5828f7
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005828F7;
    // 005828DD  test byte ptr [0x9a64e8], bl
    // 005828E3  jne 0x5828f7
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_005828F7;
LAB_005828E5:
    // 005828E5  push edi
    // 005828E6  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 005828E9  push 0x4000
    // 005828EE  push eax
    // 005828EF  call 0x581190
    add_abil((int*)(eax), 0x4000, edi);
    // 005828F4  add esp, 0xc
LAB_005828F7:
    // 005828F7  mov eax, 0x400
    eax = (0x400);
    // 005828FC  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005828FE:
    // 005828FE  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582900  inc ecx
    ecx = ((ecx) + 1);
    // 00582901  test al, 1
    // 00582903  je 0x5828fe
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005828FE;
    // 00582905  lea edx, [ecx*8]
    edx = (ecx*8);
    // 0058290C  push edi
    // 0058290D  sub edx, ecx
    edx = ((edx) - (ecx));
    // 0058290F  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582917  push eax
    // 00582918  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 0058291D  add esp, 8
    // 00582920  test eax, eax
    // 00582922  je 0x582948
    if ((((eax) & (eax))) == 0) goto LAB_00582948;
    // 00582924  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582927  test eax, eax
    // 00582929  je 0x58293f
    if ((((eax) & (eax))) == 0) goto LAB_0058293F;
    // 0058292B  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00582931  push edi
    // 00582932  push ecx
    // 00582933  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582938  add esp, 8
    // 0058293B  test eax, eax
    // 0058293D  je 0x582948
    if ((((eax) & (eax))) == 0) goto LAB_00582948;
LAB_0058293F:
    // 0058293F  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582942  or ah, 4
    SETB1(eax, ((((eax >> 8) & 0xff)) | (4)));
    // 00582945  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00582948:
    // 00582948  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 0058294B  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0058294E  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00582951  push 0
    // 00582953  push 4
    // 00582955  push edx
    // 00582956  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00582959  push eax
    // 0058295A  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0058295D  push ecx
    // 0058295E  push edx
    // 0058295F  push eax
    // 00582960  push edi
    // 00582961  call 0x580860
    eax = propose_proto(edi, eax, edx, ecx, eax, edx, 4, 0);
    // 00582966  add esp, 0x20
LAB_00582969:
    // 00582969  push 3
    // 0058296B  push 0
    // 0058296D  push edi
    // 0058296E  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00582975  call 0x57eff0
    eax = pick_chassis(edi, 0, 3);
    // 0058297A  add esp, 0xc
    // 0058297D  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00582980  test eax, eax
    // 00582982  jge 0x5829c1
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005829C1;
    // 00582984  push 2
    // 00582986  push 0
    // 00582988  push edi
    // 00582989  call 0x57eff0
    eax = pick_chassis(edi, 0, 2);
    // 0058298E  add esp, 0xc
    // 00582991  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00582994  test eax, eax
    // 00582996  jge 0x5829c1
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005829C1;
    // 00582998  push 1
    // 0058299A  push 0
    // 0058299C  push edi
    // 0058299D  call 0x57eff0
    eax = pick_chassis(edi, 0, 1);
    // 005829A2  push edi
    // 005829A3  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005829A6  mov esi, eax
    esi = (eax);
    // 005829A8  push 0x400
    // 005829AD  push ecx
    // 005829AE  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 005829B1  call 0x581190
    add_abil((int*)(ecx), 0x400, edi);
    // 005829B6  add esp, 0x18
    // 005829B9  test esi, esi
    // 005829BB  jl 0x582c2a
    if (((int)(((esi) & (esi)))) < 0) goto LAB_00582C2A;
LAB_005829C1:
    // 005829C1  push 0
    // 005829C3  push 2
    // 005829C5  push edi
    // 005829C6  call 0x57f0b0
    eax = weapon_budget(edi, 2, 0);
    // 005829CB  shl eax, 4
    eax = ((eax) << (4));
    // 005829CE  add esp, 0xc
    // 005829D1  cmp byte ptr [eax + 0x94ae68], 2
    // 005829D8  jge 0x582a2a
    if (((int)((*(unsigned char*)((char*)g_0094ae68 + eax)))) >= ((int)(2))) goto LAB_00582A2A;
    // 005829DA  mov eax, dword ptr [0x93fc54]
    eax = ((*(int*)((char*)g_0093fc54)));
    // 005829DF  test eax, eax
    // 005829E1  jne 0x582a2a
    if ((((eax) & (eax))) != 0) goto LAB_00582A2A;
    // 005829E3  mov eax, 0x800
    eax = (0x800);
    // 005829E8  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005829EA:
    // 005829EA  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005829EC  inc ecx
    ecx = ((ecx) + 1);
    // 005829ED  test al, 1
    // 005829EF  je 0x5829ea
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005829EA;
    // 005829F1  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005829F8  push edi
    // 005829F9  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005829FB  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582A03  push eax
    // 00582A04  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582A09  add esp, 8
    // 00582A0C  test eax, eax
    // 00582A0E  je 0x582c2a
    if ((((eax) & (eax))) == 0) goto LAB_00582C2A;
    // 00582A14  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00582A19  mov edx, 1
    edx = (1);
    // 00582A1E  mov ecx, edi
    ecx = (edi);
    // 00582A20  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00582A22  test dl, al
    // 00582A24  jne 0x582c2a
    if (((((edx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00582C2A;
LAB_00582A2A:
    // 00582A2A  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00582A2C  push ebx
    // 00582A2D  push 1
    // 00582A2F  push edi
    // 00582A30  call 0x57f0b0
    eax = weapon_budget(edi, 1, ebx);
    // 00582A35  add esp, 0xc
    // 00582A38  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00582A3B  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00582A42  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00582A45  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00582A4A:
    // 00582A4A  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 00582A4E  push edi
    // 00582A4F  push eax
    // 00582A50  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582A55  add esp, 8
    // 00582A58  test eax, eax
    // 00582A5A  je 0x582a97
    if ((((eax) & (eax))) == 0) goto LAB_00582A97;
    // 00582A5C  cmp byte ptr [esi + 2], 1
    // 00582A60  ja 0x582a97
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00582A97;
    // 00582A62  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00582A64  test al, al
    // 00582A66  jl 0x582a97
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00582A97;
    // 00582A68  test edi, edi
    // 00582A6A  jl 0x582a85
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00582A85;
    // 00582A6C  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00582A72  test ecx, ecx
    // 00582A74  jne 0x582a85
    if ((((ecx) & (ecx))) != 0) goto LAB_00582A85;
    // 00582A76  cmp esi, 0x94f310
    // 00582A7C  jle 0x582a85
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00582A85;
    // 00582A7E  mov eax, 1
    eax = (1);
    // 00582A83  jmp 0x582a88
    goto LAB_00582A88;
LAB_00582A85:
    // 00582A85  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00582A88:
    // 00582A88  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00582A8B  add eax, eax
    eax = ((eax) + (eax));
    // 00582A8D  cmp eax, ecx
    // 00582A8F  jl 0x582a97
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00582A97;
    // 00582A91  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00582A94  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00582A97:
    // 00582A97  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00582A9A  inc ebx
    ebx = ((ebx) + 1);
    // 00582A9B  cmp esi, 0x94f360
    // 00582AA1  jl 0x582a4a
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00582A4A;
    // 00582AA3  mov eax, 0x800
    eax = (0x800);
    // 00582AA8  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582AAA:
    // 00582AAA  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582AAC  inc ecx
    ecx = ((ecx) + 1);
    // 00582AAD  test al, 1
    // 00582AAF  je 0x582aaa
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582AAA;
    // 00582AB1  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582AB8  push edi
    // 00582AB9  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582ABB  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582AC3  push eax
    // 00582AC4  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582AC9  add esp, 8
    // 00582ACC  test eax, eax
    // 00582ACE  je 0x582b81
    if ((((eax) & (eax))) == 0) goto LAB_00582B81;
    // 00582AD4  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 00582AD9  mov edx, 1
    edx = (1);
    // 00582ADE  mov ecx, edi
    ecx = (edi);
    // 00582AE0  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00582AE2  test dl, al
    // 00582AE4  jne 0x582b81
    if (((((edx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00582B81;
    // 00582AEA  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582AED  push edi
    // 00582AEE  or ah, 8
    SETB1(eax, ((((eax >> 8) & 0xff)) | (8)));
    // 00582AF1  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00582AF4  mov eax, dword ptr [0x949780]
    eax = ((*(int*)((char*)g_00949780)));
    // 00582AF9  push eax
    // 00582AFA  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582AFF  add esp, 8
    // 00582B02  test eax, eax
    // 00582B04  je 0x582bf8
    if ((((eax) & (eax))) == 0) goto LAB_00582BF8;
    // 00582B0A  mov eax, 0x400
    eax = (0x400);
    // 00582B0F  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582B11:
    // 00582B11  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582B13  inc ecx
    ecx = ((ecx) + 1);
    // 00582B14  test al, 1
    // 00582B16  je 0x582b11
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582B11;
    // 00582B18  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582B1F  push edi
    // 00582B20  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582B22  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582B2A  push eax
    // 00582B2B  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582B30  add esp, 8
    // 00582B33  test eax, eax
    // 00582B35  je 0x582b45
    if ((((eax) & (eax))) == 0) goto LAB_00582B45;
    // 00582B37  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582B3A  or ah, 4
    SETB1(eax, ((((eax >> 8) & 0xff)) | (4)));
    // 00582B3D  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00582B40  jmp 0x582bf8
    goto LAB_00582BF8;
LAB_00582B45:
    // 00582B45  mov eax, 0x4000
    eax = (0x4000);
    // 00582B4A  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582B4C:
    // 00582B4C  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582B4E  inc ecx
    ecx = ((ecx) + 1);
    // 00582B4F  test al, 1
    // 00582B51  je 0x582b4c
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582B4C;
    // 00582B53  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582B5A  push edi
    // 00582B5B  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582B5D  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582B65  push eax
    // 00582B66  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582B6B  add esp, 8
    // 00582B6E  test eax, eax
    // 00582B70  je 0x582bf8
    if ((((eax) & (eax))) == 0) goto LAB_00582BF8;
    // 00582B76  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582B79  or ah, 0x40
    SETB1(eax, ((((eax >> 8) & 0xff)) | (0x40)));
    // 00582B7C  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00582B7F  jmp 0x582bf8
    goto LAB_00582BF8;
LAB_00582B81:
    // 00582B81  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582B84  test eax, eax
    // 00582B86  jne 0x582bad
    if ((((eax) & (eax))) != 0) goto LAB_00582BAD;
    // 00582B88  push edi
    // 00582B89  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00582B8C  push 2
    // 00582B8E  push ecx
    // 00582B8F  call 0x581190
    add_abil((int*)(ecx), 2, edi);
    // 00582B94  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582B97  add esp, 0xc
    // 00582B9A  test eax, eax
    // 00582B9C  jne 0x582bad
    if ((((eax) & (eax))) != 0) goto LAB_00582BAD;
    // 00582B9E  push edi
    // 00582B9F  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00582BA2  push 4
    // 00582BA4  push edx
    // 00582BA5  call 0x581190
    add_abil((int*)(edx), 4, edi);
    // 00582BAA  add esp, 0xc
LAB_00582BAD:
    // 00582BAD  mov esi, 0x400
    esi = (0x400);
    // 00582BB2  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00582BB4  mov eax, esi
    eax = (esi);
LAB_00582BB6:
    // 00582BB6  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582BB8  inc ecx
    ecx = ((ecx) + 1);
    // 00582BB9  test al, 1
    // 00582BBB  je 0x582bb6
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582BB6;
    // 00582BBD  lea eax, [ecx*8]
    eax = (ecx*8);
    // 00582BC4  push edi
    // 00582BC5  sub eax, ecx
    eax = ((eax) - (ecx));
    // 00582BC7  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 00582BCF  push ecx
    // 00582BD0  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582BD5  add esp, 8
    // 00582BD8  test eax, eax
    // 00582BDA  je 0x582bf8
    if ((((eax) & (eax))) == 0) goto LAB_00582BF8;
    // 00582BDC  mov edx, dword ptr [0x949780]
    edx = ((*(int*)((char*)g_00949780)));
    // 00582BE2  push edi
    // 00582BE3  push edx
    // 00582BE4  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00582BE9  add esp, 8
    // 00582BEC  test eax, eax
    // 00582BEE  je 0x582bf5
    if ((((eax) & (eax))) == 0) goto LAB_00582BF5;
    // 00582BF0  or dword ptr [ebp + 8], esi
    a1 = ((a1) | (esi));
    // 00582BF3  jmp 0x582bf8
    goto LAB_00582BF8;
LAB_00582BF5:
    // 00582BF5  mov dword ptr [ebp + 8], esi
    a1 = (esi);
LAB_00582BF8:
    // 00582BF8  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582BFB  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00582BFE  mov ecx, eax
    ecx = (eax);
    // 00582C00  push 0
    // 00582C02  and ecx, 0x40000
    ecx = ((ecx) & (0x40000));
    // 00582C08..00582C0A  neg/sbb -> (ecx!=0)?-1:0
    ecx = ((ecx) != 0 ? -1 : 0);
    // 00582C0C  and ecx, 9
    ecx = ((ecx) & (9));
    // 00582C0F  add ecx, 0xa
    ecx = ((ecx) + (0xa));
    // 00582C12  push ecx
    // 00582C13  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 00582C16  push edx
    // 00582C17  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00582C1A  push eax
    // 00582C1B  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 00582C1E  push eax
    // 00582C1F  push ecx
    // 00582C20  push edx
    // 00582C21  push edi
    // 00582C22  call 0x580860
    eax = propose_proto(edi, edx, ecx, eax, eax, edx, ecx, 0);
    // 00582C27  add esp, 0x20
LAB_00582C2A:
    // 00582C2A  push 0
    // 00582C2C  push 1
    // 00582C2E  push edi
    // 00582C2F  call 0x57eff0
    eax = pick_chassis(edi, 1, 0);
    // 00582C34  add esp, 0xc
    // 00582C37  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00582C3A  test eax, eax
    // 00582C3C  jl 0x58318b
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0058318B;
    // 00582C42  mov eax, edi
    eax = (edi);
    // 00582C44  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00582C4B  shl eax, 6
    eax = ((eax) << (6));
    // 00582C4E  add eax, edi
    eax = ((eax) + (edi));
    // 00582C50  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 00582C53  lea edx, [edi + ecx*8]
    edx = (edi + ecx*8);
    // 00582C56  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 00582C59  shl eax, 2
    eax = ((eax) << (2));
    // 00582C5C  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 00582C5F  mov ecx, dword ptr [eax + 0x96d228]
    ecx = ((*(int*)((char*)g_0096d228 + eax)));
    // 00582C65  mov edx, dword ptr [eax + 0x96ea78]
    edx = ((*(int*)((char*)g_0096ea78 + eax)));
    // 00582C6B  cmp ecx, edx
    // 00582C6D  jle 0x582c9a
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_00582C9A;
    // 00582C6F  cmp ecx, dword ptr [eax + 0x96ea74]
    // 00582C75  jle 0x582c9a
    if (((int)(ecx)) <= ((int)((*(int*)((char*)g_0096ea74 + eax))))) goto LAB_00582C9A;
    // 00582C77  mov eax, 1
    eax = (1);
    // 00582C7C  mov ecx, edi
    ecx = (edi);
    // 00582C7E  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 00582C80  test byte ptr [0x9a64e8], al
    // 00582C86  jne 0x582c9a
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((eax & 0xff)))) != 0) goto LAB_00582C9A;
    // 00582C88  push edi
    // 00582C89  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00582C8C  push 0x100
    // 00582C91  push ecx
    // 00582C92  call 0x581190
    add_abil((int*)(ecx), 0x100, edi);
    // 00582C97  add esp, 0xc
LAB_00582C9A:
    // 00582C9A  push edi
    // 00582C9B  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00582C9E  push 2
    // 00582CA0  push edx
    // 00582CA1  call 0x581190
    add_abil((int*)(edx), 2, edi);
    // 00582CA6  push edi
    // 00582CA7  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00582CAA  push 0x100
    // 00582CAF  push eax
    // 00582CB0  call 0x581190
    add_abil((int*)(eax), 0x100, edi);
    // 00582CB5  mov al, byte ptr [ebp - 0x18]
    SETB0(eax, ((var_18 & 0xff)));
    // 00582CB8  add esp, 0x18
    // 00582CBB  test al, 1
    // 00582CBD  je 0x582d0e
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582D0E;
    // 00582CBF  mov eax, 0x40000
    eax = (0x40000);
    // 00582CC4  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582CC6:
    // 00582CC6  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582CC8  inc ecx
    ecx = ((ecx) + 1);
    // 00582CC9  test al, 1
    // 00582CCB  je 0x582cc6
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582CC6;
    // 00582CCD  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582CD4  push edi
    // 00582CD5  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582CD7  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582CDF  push eax
    // 00582CE0  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582CE5  add esp, 8
    // 00582CE8  test eax, eax
    // 00582CEA  je 0x582d0e
    if ((((eax) & (eax))) == 0) goto LAB_00582D0E;
    // 00582CEC  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582CEF  test eax, eax
    // 00582CF1  je 0x582d07
    if ((((eax) & (eax))) == 0) goto LAB_00582D07;
    // 00582CF3  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00582CF9  push edi
    // 00582CFA  push ecx
    // 00582CFB  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582D00  add esp, 8
    // 00582D03  test eax, eax
    // 00582D05  je 0x582d0e
    if ((((eax) & (eax))) == 0) goto LAB_00582D0E;
LAB_00582D07:
    // 00582D07  or dword ptr [ebp + 8], 0x40000
    a1 = ((a1) | (0x40000));
LAB_00582D0E:
    // 00582D0E  mov eax, 0x100
    eax = (0x100);
    // 00582D13  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582D15:
    // 00582D15  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582D17  inc ecx
    ecx = ((ecx) + 1);
    // 00582D18  test al, 1
    // 00582D1A  je 0x582d15
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582D15;
    // 00582D1C  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582D23  push edi
    // 00582D24  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582D26  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582D2E  push eax
    // 00582D2F  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582D34  add esp, 8
    // 00582D37  test eax, eax
    // 00582D39  je 0x582d75
    if ((((eax) & (eax))) == 0) goto LAB_00582D75;
    // 00582D3B  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582D3E  test ah, 1
    // 00582D41  jne 0x582d75
    if ((((((eax >> 8) & 0xff)) & (1))) != 0) goto LAB_00582D75;
    // 00582D43  push eax
    // 00582D44  call 0x50ba30
    eax = bit_count(eax);
    // 00582D49  add esp, 4
    // 00582D4C  cmp eax, 2
    // 00582D4F  jge 0x582d75
    if (((int)(eax)) >= ((int)(2))) goto LAB_00582D75;
    // 00582D51  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582D54  test eax, eax
    // 00582D56  je 0x582d6c
    if ((((eax) & (eax))) == 0) goto LAB_00582D6C;
    // 00582D58  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00582D5E  push edi
    // 00582D5F  push ecx
    // 00582D60  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582D65  add esp, 8
    // 00582D68  test eax, eax
    // 00582D6A  je 0x582d75
    if ((((eax) & (eax))) == 0) goto LAB_00582D75;
LAB_00582D6C:
    // 00582D6C  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582D6F  or ah, 1
    SETB1(eax, ((((eax >> 8) & 0xff)) | (1)));
    // 00582D72  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00582D75:
    // 00582D75  mov eax, 2
    eax = (2);
    // 00582D7A  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00582D7C:
    // 00582D7C  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582D7E  inc ecx
    ecx = ((ecx) + 1);
    // 00582D7F  test al, 1
    // 00582D81  je 0x582d7c
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00582D7C;
    // 00582D83  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00582D8A  push edi
    // 00582D8B  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00582D8D  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00582D95  push eax
    // 00582D96  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00582D9B  add esp, 8
    // 00582D9E  test eax, eax
    // 00582DA0  je 0x582dda
    if ((((eax) & (eax))) == 0) goto LAB_00582DDA;
    // 00582DA2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582DA5  test al, 2
    // 00582DA7  jne 0x582dda
    if (((((eax & 0xff)) & (2))) != 0) goto LAB_00582DDA;
    // 00582DA9  push eax
    // 00582DAA  call 0x50ba30
    eax = bit_count(eax);
    // 00582DAF  add esp, 4
    // 00582DB2  cmp eax, 2
    // 00582DB5  jge 0x582dda
    if (((int)(eax)) >= ((int)(2))) goto LAB_00582DDA;
    // 00582DB7  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582DBA  test eax, eax
    // 00582DBC  je 0x582dd2
    if ((((eax) & (eax))) == 0) goto LAB_00582DD2;
    // 00582DBE  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00582DC4  push edi
    // 00582DC5  push ecx
    // 00582DC6  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582DCB  add esp, 8
    // 00582DCE  test eax, eax
    // 00582DD0  je 0x582dda
    if ((((eax) & (eax))) == 0) goto LAB_00582DDA;
LAB_00582DD2:
    // 00582DD2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00582DD5  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 00582DD7  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00582DDA:
    // 00582DDA  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00582DDC  push ebx
    // 00582DDD  push 1
    // 00582DDF  push edi
    // 00582DE0  call 0x57f0b0
    eax = weapon_budget(edi, 1, ebx);
    // 00582DE5  add esp, 0xc
    // 00582DE8  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00582DEB  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00582DF2  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00582DF5  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00582DFA:
    // 00582DFA  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00582DFE  push edi
    // 00582DFF  push edx
    // 00582E00  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00582E05  add esp, 8
    // 00582E08  test eax, eax
    // 00582E0A  je 0x582e47
    if ((((eax) & (eax))) == 0) goto LAB_00582E47;
    // 00582E0C  cmp byte ptr [esi + 2], 1
    // 00582E10  ja 0x582e47
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00582E47;
    // 00582E12  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00582E14  test al, al
    // 00582E16  jl 0x582e47
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00582E47;
    // 00582E18  test edi, edi
    // 00582E1A  jl 0x582e35
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00582E35;
    // 00582E1C  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00582E22  test ecx, ecx
    // 00582E24  jne 0x582e35
    if ((((ecx) & (ecx))) != 0) goto LAB_00582E35;
    // 00582E26  cmp esi, 0x94f310
    // 00582E2C  jle 0x582e35
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00582E35;
    // 00582E2E  mov eax, 1
    eax = (1);
    // 00582E33  jmp 0x582e38
    goto LAB_00582E38;
LAB_00582E35:
    // 00582E35  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00582E38:
    // 00582E38  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00582E3B  add eax, eax
    eax = ((eax) + (eax));
    // 00582E3D  cmp eax, ecx
    // 00582E3F  jl 0x582e47
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00582E47;
    // 00582E41  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00582E44  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00582E47:
    // 00582E47  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00582E4A  inc ebx
    ebx = ((ebx) + 1);
    // 00582E4B  cmp esi, 0x94f360
    // 00582E51  jl 0x582dfa
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00582DFA;
    // 00582E53  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 00582E56  mov dword ptr [ebp - 0x24], 0
    var_24 = (0);
    // 00582E5D  mov dword ptr [ebp - 0x10], eax
    var_10 = (eax);
LAB_00582E60:
    // 00582E60  mov ebx, dword ptr [ebp - 0xc]
    ebx = (var_c);
    // 00582E63  mov dword ptr [ebp - 0x2c], 0
    var_2c = (0);
    // 00582E6A  mov esi, 1
    esi = (1);
LAB_00582E6F:
    // 00582E6F  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00582E72  cmp ebx, eax
    // 00582E74  jne 0x582e9d
    if ((ebx) != (eax)) goto LAB_00582E9D;
    // 00582E76  mov ecx, eax
    ecx = (eax);
    // 00582E78  push 0
    // 00582E7A  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00582E7D  movsx edx, byte ptr [ecx + 0x94ae68]
    edx = ((*(signed char*)((char*)g_0094ae68 + ecx)));
    // 00582E84  add edx, esi
    edx = ((edx) + (esi));
    // 00582E86  push edx
    // 00582E87  push edi
    // 00582E88  call 0x57f0b0
    eax = weapon_budget(edi, edx, 0);
    // 00582E8D  add esp, 0xc
    // 00582E90  inc esi
    esi = ((esi) + 1);
    // 00582E91  cmp esi, 4
    // 00582E94  mov ebx, eax
    ebx = (eax);
    // 00582E96  jl 0x582e6f
    if (((int)(esi)) < ((int)(4))) goto LAB_00582E6F;
    // 00582E98  cmp ebx, dword ptr [ebp - 0xc]
    // 00582E9B  je 0x582eec
    if ((ebx) == (var_c)) goto LAB_00582EEC;
LAB_00582E9D:
    // 00582E9D  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00582EA0  mov eax, ecx
    eax = (ecx);
    // 00582EA2  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00582EA3  sub eax, edx
    eax = ((eax) - (edx));
    // 00582EA5  lea esi, [ecx + ecx]
    esi = (ecx);
    // 00582EA8  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00582EAA  sub esi, eax
    esi = ((esi) - (eax));
    // 00582EAC  cmp esi, 5
    // 00582EAF  jge 0x582eb6
    if (((int)(esi)) >= ((int)(5))) goto LAB_00582EB6;
    // 00582EB1  mov esi, 5
    esi = (5);
LAB_00582EB6:
    // 00582EB6  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00582EB9  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00582EBC  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00582EBF  push eax
    // 00582EC0  push 0
    // 00582EC2  push ecx
    // 00582EC3  push ebx
    // 00582EC4  push edx
    // 00582EC5  call 0x5a5a60
    eax = proto_cost(edx, ebx, ecx, 0, eax);
    // 00582ECA  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 00582ECD  add esp, 0x14
    // 00582ED0  mov edx, dword ptr [ecx + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + ecx)));
    // 00582ED6  add edx, dword ptr [ecx + 0x96cdc0]
    edx = ((edx) + ((*(int*)((char*)g_0096cdc0 + ecx))));
    // 00582EDC  add edx, esi
    edx = ((edx) + (esi));
    // 00582EDE  cmp eax, edx
    // 00582EE0  jg 0x582eec
    if (((int)(eax)) > ((int)(edx))) goto LAB_00582EEC;
    // 00582EE2  mov dword ptr [ebp - 0xc], ebx
    var_c = (ebx);
    // 00582EE5  mov dword ptr [ebp - 0x2c], 1
    var_2c = (1);
LAB_00582EEC:
    // 00582EEC  mov esi, dword ptr [ebp - 0x10]
    esi = (var_10);
    // 00582EEF  mov dword ptr [ebp - 0x34], 1
    var_34 = (1);
LAB_00582EF6:
    // 00582EF6  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 00582EF9  cmp esi, eax
    // 00582EFB  jne 0x582fa5
    if ((esi) != (eax)) goto LAB_00582FA5;
    // 00582F01  shl eax, 4
    eax = ((eax) << (4));
    // 00582F04  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00582F0B  mov esi, 0x94f280
    esi = (0x94f280);
    // 00582F10  movsx ebx, byte ptr [eax + 0x94f280]
    ebx = ((*(signed char*)((char*)g_0094f280 + eax)));
    // 00582F17  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 00582F1A  add ebx, eax
    ebx = ((ebx) + (eax));
    // 00582F1C  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00582F1E  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
    // 00582F21  mov dword ptr [ebp - 0x38], eax
    var_38 = (eax);
LAB_00582F24:
    // 00582F24  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 00582F28  push edi
    // 00582F29  push ecx
    // 00582F2A  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00582F2F  add esp, 8
    // 00582F32  test eax, eax
    // 00582F34  je 0x582f77
    if ((((eax) & (eax))) == 0) goto LAB_00582F77;
    // 00582F36  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00582F38  mov dl, byte ptr [esi + 2]
    SETB0(edx, ((*(unsigned char*)((char*)(esi) + 0x2))));
    // 00582F3B  cmp edx, ebx
    // 00582F3D  jg 0x582f77
    if (((int)(edx)) > ((int)(ebx))) goto LAB_00582F77;
    // 00582F3F  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00582F41  test al, al
    // 00582F43  jl 0x582f77
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00582F77;
    // 00582F45  test edi, edi
    // 00582F47  jl 0x582f62
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00582F62;
    // 00582F49  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00582F4F  test ecx, ecx
    // 00582F51  jne 0x582f62
    if ((((ecx) & (ecx))) != 0) goto LAB_00582F62;
    // 00582F53  cmp esi, 0x94f310
    // 00582F59  jle 0x582f62
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00582F62;
    // 00582F5B  mov eax, 1
    eax = (1);
    // 00582F60  jmp 0x582f65
    goto LAB_00582F65;
LAB_00582F62:
    // 00582F62  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00582F65:
    // 00582F65  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00582F68  add eax, eax
    eax = ((eax) + (eax));
    // 00582F6A  cmp eax, ecx
    // 00582F6C  jl 0x582f77
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00582F77;
    // 00582F6E  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00582F71  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 00582F74  mov dword ptr [ebp - 0x30], eax
    var_30 = (eax);
LAB_00582F77:
    // 00582F77  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 00582F7A  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00582F7D  inc edx
    edx = ((edx) + 1);
    // 00582F7E  cmp esi, 0x94f360
    // 00582F84  mov dword ptr [ebp - 0x38], edx
    var_38 = (edx);
    // 00582F87  jl 0x582f24
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00582F24;
    // 00582F89  mov eax, dword ptr [ebp - 0x34]
    eax = (var_34);
    // 00582F8C  mov esi, dword ptr [ebp - 0x30]
    esi = (var_30);
    // 00582F8F  inc eax
    eax = ((eax) + 1);
    // 00582F90  cmp eax, 4
    // 00582F93  mov dword ptr [ebp - 0x34], eax
    var_34 = (eax);
    // 00582F96  jl 0x582ef6
    if (((int)(eax)) < ((int)(4))) goto LAB_00582EF6;
    // 00582F9C  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 00582F9F  mov ecx, esi
    ecx = (esi);
    // 00582FA1  cmp ecx, eax
    // 00582FA3  je 0x58300e
    if ((ecx) == (eax)) goto LAB_0058300E;
LAB_00582FA5:
    // 00582FA5  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 00582FA8  mov eax, esi
    eax = (esi);
    // 00582FAA  shl eax, 4
    eax = ((eax) << (4));
    // 00582FAD  shl edx, 4
    edx = ((edx) << (4));
    // 00582FB0  mov cl, byte ptr [eax + 0x94f280]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094f280 + eax))));
    // 00582FB6  cmp cl, byte ptr [edx + 0x94f280]
    // 00582FBC  jle 0x58300e
    if (((signed char)(ecx & 0xff)) <= ((signed char)(*(unsigned char*)((char*)g_0094f280 + edx)))) goto LAB_0058300E;
    // 00582FBE  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00582FC1  mov ebx, 5
    ebx = (5);
    // 00582FC6  mov eax, ecx
    eax = (ecx);
    // 00582FC8  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00582FC9  sub eax, edx
    eax = ((eax) - (edx));
    // 00582FCB  mov edx, eax
    edx = (eax);
    // 00582FCD  lea eax, [ecx + ecx]
    eax = (ecx);
    // 00582FD0  sar edx, 1
    edx = ((((int)(edx))) >> (1));
    // 00582FD2  sub eax, edx
    eax = ((eax) - (edx));
    // 00582FD4  cmp eax, 5
    // 00582FD7  jl 0x582fdb
    if (((int)(eax)) < ((int)(5))) goto LAB_00582FDB;
    // 00582FD9  mov ebx, eax
    ebx = (eax);
LAB_00582FDB:
    // 00582FDB  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00582FDE  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00582FE1  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00582FE4  push eax
    // 00582FE5  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00582FE8  push ecx
    // 00582FE9  push esi
    // 00582FEA  push edx
    // 00582FEB  push eax
    // 00582FEC  call 0x5a5a60
    eax = proto_cost(eax, edx, esi, ecx, eax);
    // 00582FF1  mov ecx, dword ptr [ebp - 0x14]
    ecx = (var_14);
    // 00582FF4  add esp, 0x14
    // 00582FF7  mov edx, dword ptr [ecx + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + ecx)));
    // 00582FFD  add edx, dword ptr [ecx + 0x96cdc0]
    edx = ((edx) + ((*(int*)((char*)g_0096cdc0 + ecx))));
    // 00583003  add edx, ebx
    edx = ((edx) + (ebx));
    // 00583005  cmp eax, edx
    // 00583007  jg 0x58300e
    if (((int)(eax)) > ((int)(edx))) goto LAB_0058300E;
    // 00583009  mov dword ptr [ebp - 0x10], esi
    var_10 = (esi);
    // 0058300C  jmp 0x583015
    goto LAB_00583015;
LAB_0058300E:
    // 0058300E  mov eax, dword ptr [ebp - 0x2c]
    eax = (var_2c);
    // 00583011  test eax, eax
    // 00583013  je 0x583025
    if ((((eax) & (eax))) == 0) goto LAB_00583025;
LAB_00583015:
    // 00583015  mov eax, dword ptr [ebp - 0x24]
    eax = (var_24);
    // 00583018  inc eax
    eax = ((eax) + 1);
    // 00583019  cmp eax, 0x64
    // 0058301C  mov dword ptr [ebp - 0x24], eax
    var_24 = (eax);
    // 0058301F  jl 0x582e60
    if (((int)(eax)) < ((int)(0x64))) goto LAB_00582E60;
LAB_00583025:
    // 00583025  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0058302A  mov ebx, 1
    ebx = (1);
    // 0058302F  mov ecx, edi
    ecx = (edi);
    // 00583031  shl ebx, cl
    ebx = ((ebx) << ((ecx & 0xff)));
    // 00583033  test bl, al
    // 00583035  je 0x583046
    if (((((ebx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_00583046;
    // 00583037  push 0
    // 00583039  push 0x63
    // 0058303B  push edi
    // 0058303C  call 0x57f0b0
    eax = weapon_budget(edi, 0x63, 0);
    // 00583041  add esp, 0xc
    // 00583044  jmp 0x583049
    goto LAB_00583049;
LAB_00583046:
    // 00583046  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
LAB_00583049:
    // 00583049  mov ecx, eax
    ecx = (eax);
    // 0058304B  shl ecx, 4
    ecx = ((ecx) << (4));
    // 0058304E  cmp byte ptr [ecx + 0x94ae68], 1
    // 00583055  jl 0x583075
    if (((int)((*(unsigned char*)((char*)g_0094ae68 + ecx)))) < ((int)(1))) goto LAB_00583075;
    // 00583057  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 0058305A  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0058305D  push 0
    // 0058305F  push 6
    // 00583061  push edx
    // 00583062  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 00583065  push ecx
    // 00583066  push edx
    // 00583067  push eax
    // 00583068  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0058306B  push eax
    // 0058306C  push edi
    // 0058306D  call 0x580860
    eax = propose_proto(edi, eax, eax, edx, ecx, edx, 6, 0);
    // 00583072  add esp, 0x20
LAB_00583075:
    // 00583075  test byte ptr [0x9a64e8], bl
    // 0058307B  jne 0x58318b
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((ebx & 0xff)))) != 0) goto LAB_0058318B;
    // 00583081  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583083  push ebx
    // 00583084  push 1
    // 00583086  push edi
    // 00583087  call 0x57f0b0
    eax = weapon_budget(edi, 1, ebx);
    // 0058308C  add esp, 0xc
    // 0058308F  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00583092  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00583099  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 0058309C  mov esi, 0x94f280
    esi = (0x94f280);
LAB_005830A1:
    // 005830A1  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 005830A5  push edi
    // 005830A6  push ecx
    // 005830A7  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005830AC  add esp, 8
    // 005830AF  test eax, eax
    // 005830B1  je 0x5830ee
    if ((((eax) & (eax))) == 0) goto LAB_005830EE;
    // 005830B3  cmp byte ptr [esi + 2], 0x63
    // 005830B7  ja 0x5830ee
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)0x63)) goto LAB_005830EE;
    // 005830B9  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 005830BB  test al, al
    // 005830BD  jl 0x5830ee
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_005830EE;
    // 005830BF  test edi, edi
    // 005830C1  jl 0x5830dc
    if (((int)(((edi) & (edi)))) < 0) goto LAB_005830DC;
    // 005830C3  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 005830C9  test ecx, ecx
    // 005830CB  jne 0x5830dc
    if ((((ecx) & (ecx))) != 0) goto LAB_005830DC;
    // 005830CD  cmp esi, 0x94f310
    // 005830D3  jle 0x5830dc
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_005830DC;
    // 005830D5  mov eax, 1
    eax = (1);
    // 005830DA  jmp 0x5830df
    goto LAB_005830DF;
LAB_005830DC:
    // 005830DC  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_005830DF:
    // 005830DF  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005830E2  add eax, eax
    eax = ((eax) + (eax));
    // 005830E4  cmp eax, ecx
    // 005830E6  jl 0x5830ee
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005830EE;
    // 005830E8  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 005830EB  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_005830EE:
    // 005830EE  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005830F1  inc ebx
    ebx = ((ebx) + 1);
    // 005830F2  cmp esi, 0x94f360
    // 005830F8  jl 0x5830a1
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_005830A1;
    // 005830FA  push edi
    // 005830FB  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 005830FE  push 0x100
    // 00583103  push edx
    // 00583104  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 0058310B  call 0x581190
    add_abil((int*)(edx), 0x100, edi);
    // 00583110  add esp, 0xc
    // 00583113  mov eax, 2
    eax = (2);
    // 00583118  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_0058311A:
    // 0058311A  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0058311C  inc ecx
    ecx = ((ecx) + 1);
    // 0058311D  test al, 1
    // 0058311F  je 0x58311a
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_0058311A;
    // 00583121  lea eax, [ecx*8]
    eax = (ecx*8);
    // 00583128  push edi
    // 00583129  sub eax, ecx
    eax = ((eax) - (ecx));
    // 0058312B  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 00583133  push ecx
    // 00583134  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583139  add esp, 8
    // 0058313C  test eax, eax
    // 0058313E  je 0x58315c
    if ((((eax) & (eax))) == 0) goto LAB_0058315C;
    // 00583140  mov edx, dword ptr [0x949780]
    edx = ((*(int*)((char*)g_00949780)));
    // 00583146  push edi
    // 00583147  push edx
    // 00583148  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 0058314D  add esp, 8
    // 00583150  test eax, eax
    // 00583152  je 0x58315c
    if ((((eax) & (eax))) == 0) goto LAB_0058315C;
    // 00583154  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583157  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 00583159  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_0058315C:
    // 0058315C  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 0058315F  mov ecx, eax
    ecx = (eax);
    // 00583161  shl ecx, 4
    ecx = ((ecx) << (4));
    // 00583164  cmp byte ptr [ecx + 0x94f280], 1
    // 0058316B  jle 0x58318b
    if (((int)((*(unsigned char*)((char*)g_0094f280 + ecx)))) <= ((int)(1))) goto LAB_0058318B;
    // 0058316D  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00583170  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00583173  push 0
    // 00583175  push 0x15
    // 00583177  push edx
    // 00583178  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 0058317B  push ecx
    // 0058317C  push eax
    // 0058317D  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00583180  push edx
    // 00583181  push eax
    // 00583182  push edi
    // 00583183  call 0x580860
    eax = propose_proto(edi, eax, edx, eax, ecx, edx, 0x15, 0);
    // 00583188  add esp, 0x20
LAB_0058318B:
    // 0058318B  push 0
    // 0058318D  push 1
    // 0058318F  push edi
    // 00583190  call 0x57eff0
    eax = pick_chassis(edi, 1, 0);
    // 00583195  add esp, 0xc
    // 00583198  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058319B  test eax, eax
    // 0058319D  jl 0x58343f
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0058343F;
    // 005831A3  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 005831A5  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 005831AC  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 005831AF  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 005831B2  mov esi, 0x94f280
    esi = (0x94f280);
LAB_005831B7:
    // 005831B7  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 005831BB  push edi
    // 005831BC  push ecx
    // 005831BD  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005831C2  add esp, 8
    // 005831C5  test eax, eax
    // 005831C7  je 0x583204
    if ((((eax) & (eax))) == 0) goto LAB_00583204;
    // 005831C9  cmp byte ptr [esi + 2], 1
    // 005831CD  ja 0x583204
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00583204;
    // 005831CF  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 005831D1  test al, al
    // 005831D3  jl 0x583204
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00583204;
    // 005831D5  test edi, edi
    // 005831D7  jl 0x5831f2
    if (((int)(((edi) & (edi)))) < 0) goto LAB_005831F2;
    // 005831D9  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 005831DF  test ecx, ecx
    // 005831E1  jne 0x5831f2
    if ((((ecx) & (ecx))) != 0) goto LAB_005831F2;
    // 005831E3  cmp esi, 0x94f310
    // 005831E9  jle 0x5831f2
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_005831F2;
    // 005831EB  mov eax, 1
    eax = (1);
    // 005831F0  jmp 0x5831f5
    goto LAB_005831F5;
LAB_005831F2:
    // 005831F2  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_005831F5:
    // 005831F5  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005831F8  add eax, eax
    eax = ((eax) + (eax));
    // 005831FA  cmp eax, ecx
    // 005831FC  jl 0x583204
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00583204;
    // 005831FE  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00583201  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00583204:
    // 00583204  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583207  inc ebx
    ebx = ((ebx) + 1);
    // 00583208  cmp esi, 0x94f360
    // 0058320E  jl 0x5831b7
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_005831B7;
    // 00583210  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
    // 00583213  push 0
    // 00583215  push -7
    // 00583217  push edi
    // 00583218  call 0x57f0b0
    eax = weapon_budget(edi, -7, 0);
    // 0058321D  mov edx, edi
    edx = (edi);
    // 0058321F  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00583222  shl edx, 6
    edx = ((edx) << (6));
    // 00583225  add edx, edi
    edx = ((edx) + (edi));
    // 00583227  add esp, 0xc
    // 0058322A  lea eax, [edi + edx*2]
    eax = (edi + edx*2);
    // 0058322D  lea ecx, [edi + eax*8]
    ecx = (edi + eax*8);
    // 00583230  lea eax, [edi + ecx*2]
    eax = (edi + ecx*2);
    // 00583233  shl eax, 2
    eax = ((eax) << (2));
    // 00583236  mov ecx, dword ptr [eax + 0x96d228]
    ecx = ((*(int*)((char*)g_0096d228 + eax)));
    // 0058323C  mov edx, dword ptr [eax + 0x96ea78]
    edx = ((*(int*)((char*)g_0096ea78 + eax)));
    // 00583242  cmp ecx, edx
    // 00583244  jle 0x5832bb
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_005832BB;
    // 00583246  cmp ecx, dword ptr [eax + 0x96ea74]
    // 0058324C  jle 0x5832bb
    if (((int)(ecx)) <= ((int)((*(int*)((char*)g_0096ea74 + eax))))) goto LAB_005832BB;
    // 0058324E  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583250  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00583257  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 0058325A  mov esi, 0x94f280
    esi = (0x94f280);
LAB_0058325F:
    // 0058325F  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00583263  push edi
    // 00583264  push edx
    // 00583265  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 0058326A  add esp, 8
    // 0058326D  test eax, eax
    // 0058326F  je 0x5832ac
    if ((((eax) & (eax))) == 0) goto LAB_005832AC;
    // 00583271  cmp byte ptr [esi + 2], 0x63
    // 00583275  ja 0x5832ac
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)0x63)) goto LAB_005832AC;
    // 00583277  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583279  test al, al
    // 0058327B  jl 0x5832ac
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_005832AC;
    // 0058327D  test edi, edi
    // 0058327F  jl 0x58329a
    if (((int)(((edi) & (edi)))) < 0) goto LAB_0058329A;
    // 00583281  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00583287  test ecx, ecx
    // 00583289  jne 0x58329a
    if ((((ecx) & (ecx))) != 0) goto LAB_0058329A;
    // 0058328B  cmp esi, 0x94f310
    // 00583291  jle 0x58329a
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_0058329A;
    // 00583293  mov eax, 1
    eax = (1);
    // 00583298  jmp 0x58329d
    goto LAB_0058329D;
LAB_0058329A:
    // 0058329A  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_0058329D:
    // 0058329D  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005832A0  add eax, eax
    eax = ((eax) + (eax));
    // 005832A2  cmp eax, ecx
    // 005832A4  jl 0x5832ac
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005832AC;
    // 005832A6  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 005832A9  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_005832AC:
    // 005832AC  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005832AF  inc ebx
    ebx = ((ebx) + 1);
    // 005832B0  cmp esi, 0x94f360
    // 005832B6  jl 0x58325f
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_0058325F;
    // 005832B8  mov esi, dword ptr [ebp - 0x38]
    esi = (var_38);
LAB_005832BB:
    // 005832BB  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005832BE  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005832C1  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005832C4  push 0
    // 005832C6  push 7
    // 005832C8  push eax
    // 005832C9  push ecx
    // 005832CA  push esi
    // 005832CB  mov esi, dword ptr [ebp - 4]
    esi = (var_4);
    // 005832CE  push edx
    // 005832CF  push esi
    // 005832D0  push edi
    // 005832D1  call 0x580860
    eax = propose_proto(edi, esi, edx, esi, ecx, eax, 7, 0);
    // 005832D6  add esp, 0x20
    // 005832D9  cmp esi, 3
    // 005832DC  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 005832E3  je 0x583301
    if ((esi) == (3)) goto LAB_00583301;
    // 005832E5  movsx eax, word ptr [0x94a56e]
    eax = ((*(short*)((char*)g_0094a56e)));
    // 005832EC  push edi
    // 005832ED  push eax
    // 005832EE  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005832F3  add esp, 8
    // 005832F6  test eax, eax
    // 005832F8  je 0x583301
    if ((((eax) & (eax))) == 0) goto LAB_00583301;
    // 005832FA  mov dword ptr [ebp - 4], 3
    var_4 = (3);
LAB_00583301:
    // 00583301  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583303  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 0058330A  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 0058330D  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583312:
    // 00583312  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 00583316  push edi
    // 00583317  push ecx
    // 00583318  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 0058331D  add esp, 8
    // 00583320  test eax, eax
    // 00583322  je 0x58335f
    if ((((eax) & (eax))) == 0) goto LAB_0058335F;
    // 00583324  cmp byte ptr [esi + 2], 1
    // 00583328  ja 0x58335f
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_0058335F;
    // 0058332A  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 0058332C  test al, al
    // 0058332E  jl 0x58335f
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_0058335F;
    // 00583330  test edi, edi
    // 00583332  jl 0x58334d
    if (((int)(((edi) & (edi)))) < 0) goto LAB_0058334D;
    // 00583334  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 0058333A  test ecx, ecx
    // 0058333C  jne 0x58334d
    if ((((ecx) & (ecx))) != 0) goto LAB_0058334D;
    // 0058333E  cmp esi, 0x94f310
    // 00583344  jle 0x58334d
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_0058334D;
    // 00583346  mov eax, 1
    eax = (1);
    // 0058334B  jmp 0x583350
    goto LAB_00583350;
LAB_0058334D:
    // 0058334D  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00583350:
    // 00583350  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00583353  add eax, eax
    eax = ((eax) + (eax));
    // 00583355  cmp eax, ecx
    // 00583357  jl 0x58335f
    if (((int)(eax)) < ((int)(ecx))) goto LAB_0058335F;
    // 00583359  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 0058335C  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_0058335F:
    // 0058335F  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583362  inc ebx
    ebx = ((ebx) + 1);
    // 00583363  cmp esi, 0x94f360
    // 00583369  jl 0x583312
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583312;
    // 0058336B  push 0
    // 0058336D  push -9
    // 0058336F  push edi
    // 00583370  call 0x57f0b0
    eax = weapon_budget(edi, -9, 0);
    // 00583375  push edi
    // 00583376  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00583379  push 1
    // 0058337B  push edx
    // 0058337C  mov esi, eax
    esi = (eax);
    // 0058337E  call 0x581190
    add_abil((int*)(edx), 1, edi);
    // 00583383  push edi
    // 00583384  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 00583387  push 0x2000
    // 0058338C  push eax
    // 0058338D  call 0x581190
    add_abil((int*)(eax), 0x2000, edi);
    // 00583392  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583395  add esp, 0x24
    // 00583398  cmp eax, 1
    // 0058339B  jne 0x5833e7
    if ((eax) != (1)) goto LAB_005833E7;
    // 0058339D  mov eax, 0x2000
    eax = (0x2000);
    // 005833A2  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005833A4:
    // 005833A4  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005833A6  inc ecx
    ecx = ((ecx) + 1);
    // 005833A7  test al, 1
    // 005833A9  je 0x5833a4
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005833A4;
    // 005833AB  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005833B2  push edi
    // 005833B3  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005833B5  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005833BD  push eax
    // 005833BE  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005833C3  add esp, 8
    // 005833C6  test eax, eax
    // 005833C8  je 0x5833e7
    if ((((eax) & (eax))) == 0) goto LAB_005833E7;
    // 005833CA  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 005833D0  push edi
    // 005833D1  push ecx
    // 005833D2  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005833D7  add esp, 8
    // 005833DA  test eax, eax
    // 005833DC  je 0x5833e7
    if ((((eax) & (eax))) == 0) goto LAB_005833E7;
    // 005833DE  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005833E1  or ah, 0x20
    SETB1(eax, ((((eax >> 8) & 0xff)) | (0x20)));
    // 005833E4  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_005833E7:
    // 005833E7  test esi, esi
    // 005833E9  jl 0x583409
    if (((int)(((esi) & (esi)))) < 0) goto LAB_00583409;
    // 005833EB  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005833EE  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005833F1  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 005833F4  push 0
    // 005833F6  push 0xf
    // 005833F8  push edx
    // 005833F9  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 005833FC  push eax
    // 005833FD  push ecx
    // 005833FE  push esi
    // 005833FF  push edx
    // 00583400  push edi
    // 00583401  call 0x580860
    eax = propose_proto(edi, edx, esi, ecx, eax, edx, 0xf, 0);
    // 00583406  add esp, 0x20
LAB_00583409:
    // 00583409  push 0
    // 0058340B  push -8
    // 0058340D  push edi
    // 0058340E  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00583415  call 0x57f0b0
    eax = weapon_budget(edi, -8, 0);
    // 0058341A  add esp, 0xc
    // 0058341D  test eax, eax
    // 0058341F  jl 0x58343f
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0058343F;
    // 00583421  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00583424  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00583427  push 0
    // 00583429  push 0x10
    // 0058342B  push ecx
    // 0058342C  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 0058342F  push edx
    // 00583430  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00583433  push ecx
    // 00583434  push eax
    // 00583435  push edx
    // 00583436  push edi
    // 00583437  call 0x580860
    eax = propose_proto(edi, edx, eax, ecx, edx, ecx, 0x10, 0);
    // 0058343C  add esp, 0x20
LAB_0058343F:
    // 0058343F  push 0
    // 00583441  push 2
    // 00583443  push edi
    // 00583444  call 0x57eff0
    eax = pick_chassis(edi, 2, 0);
    // 00583449  add esp, 0xc
    // 0058344C  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058344F  test eax, eax
    // 00583451  jl 0x583701
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583701;
    // 00583457  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583459  push ebx
    // 0058345A  push 0x63
    // 0058345C  push edi
    // 0058345D  call 0x57f0b0
    eax = weapon_budget(edi, 0x63, ebx);
    // 00583462  add esp, 0xc
    // 00583465  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 00583468  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 0058346F  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583472  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583477:
    // 00583477  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 0058347B  push edi
    // 0058347C  push eax
    // 0058347D  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00583482  add esp, 8
    // 00583485  test eax, eax
    // 00583487  je 0x5834c4
    if ((((eax) & (eax))) == 0) goto LAB_005834C4;
    // 00583489  cmp byte ptr [esi + 2], 1
    // 0058348D  ja 0x5834c4
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_005834C4;
    // 0058348F  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583491  test al, al
    // 00583493  jl 0x5834c4
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_005834C4;
    // 00583495  test edi, edi
    // 00583497  jl 0x5834b2
    if (((int)(((edi) & (edi)))) < 0) goto LAB_005834B2;
    // 00583499  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 0058349F  test ecx, ecx
    // 005834A1  jne 0x5834b2
    if ((((ecx) & (ecx))) != 0) goto LAB_005834B2;
    // 005834A3  cmp esi, 0x94f310
    // 005834A9  jle 0x5834b2
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_005834B2;
    // 005834AB  mov eax, 1
    eax = (1);
    // 005834B0  jmp 0x5834b5
    goto LAB_005834B5;
LAB_005834B2:
    // 005834B2  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_005834B5:
    // 005834B5  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 005834B8  add eax, eax
    eax = ((eax) + (eax));
    // 005834BA  cmp eax, ecx
    // 005834BC  jl 0x5834c4
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005834C4;
    // 005834BE  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 005834C1  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_005834C4:
    // 005834C4  add esi, 0x10
    esi = ((esi) + (0x10));
    // 005834C7  inc ebx
    ebx = ((ebx) + 1);
    // 005834C8  cmp esi, 0x94f360
    // 005834CE  jl 0x583477
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583477;
    // 005834D0  mov ecx, edi
    ecx = (edi);
    // 005834D2  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 005834D9  shl ecx, 6
    ecx = ((ecx) << (6));
    // 005834DC  add ecx, edi
    ecx = ((ecx) + (edi));
    // 005834DE  lea edx, [edi + ecx*2]
    edx = (edi + ecx*2);
    // 005834E1  lea eax, [edi + edx*8]
    eax = (edi + edx*8);
    // 005834E4  lea esi, [edi + eax*2]
    esi = (edi + eax*2);
    // 005834E7  shl esi, 2
    esi = ((esi) << (2));
    // 005834EA  cmp dword ptr [esi + 0x96c9e4], 6
    // 005834F1  jl 0x58352c
    if (((int)((*(int*)((char*)g_0096c9e4 + esi)))) < ((int)(6))) goto LAB_0058352C;
    // 005834F3  mov eax, dword ptr [esi + 0x96cdc8]
    eax = ((*(int*)((char*)g_0096cdc8 + esi)));
    // 005834F9  test eax, eax
    // 005834FB  jne 0x583507
    if ((((eax) & (eax))) != 0) goto LAB_00583507;
    // 005834FD  mov eax, dword ptr [esi + 0x96cdc4]
    eax = ((*(int*)((char*)g_0096cdc4 + esi)));
    // 00583503  test eax, eax
    // 00583505  je 0x58352c
    if ((((eax) & (eax))) == 0) goto LAB_0058352C;
LAB_00583507:
    // 00583507  mov eax, dword ptr [esi + 0x96cdcc]
    eax = ((*(int*)((char*)g_0096cdcc + esi)));
    // 0058350D  test eax, eax
    // 0058350F  je 0x58351a
    if ((((eax) & (eax))) == 0) goto LAB_0058351A;
    // 00583511  cmp dword ptr [esi + 0x96cdbc], 1
    // 00583518  jge 0x58352c
    if (((int)((*(int*)((char*)g_0096cdbc + esi)))) >= ((int)(1))) goto LAB_0058352C;
LAB_0058351A:
    // 0058351A  push edi
    // 0058351B  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 0058351E  push 0x10000
    // 00583523  push ecx
    // 00583524  call 0x581190
    add_abil((int*)(ecx), 0x10000, edi);
    // 00583529  add esp, 0xc
LAB_0058352C:
    // 0058352C  test byte ptr [ebp - 0x18], 2
    // 00583530  je 0x583544
    if (((((var_18 & 0xff)) & (2))) == 0) goto LAB_00583544;
    // 00583532  push edi
    // 00583533  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 00583536  push 0x800
    // 0058353B  push edx
    // 0058353C  call 0x581190
    add_abil((int*)(edx), 0x800, edi);
    // 00583541  add esp, 0xc
LAB_00583544:
    // 00583544  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00583547  test ecx, ecx
    // 00583549  je 0x583583
    if ((((ecx) & (ecx))) == 0) goto LAB_00583583;
    // 0058354B  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00583551  mov edx, 5
    edx = (5);
    // 00583556  sub edx, eax
    edx = ((edx) - (eax));
    // 00583558  cmp ecx, edx
    // 0058355A  jle 0x583583
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_00583583;
    // 0058355C  test eax, eax
    // 0058355E  jl 0x583583
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583583;
    // 00583560  mov eax, 1
    eax = (1);
    // 00583565  mov ecx, edi
    ecx = (edi);
    // 00583567  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 00583569  test byte ptr [0x9a64e8], al
    // 0058356F  jne 0x583583
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((eax & 0xff)))) != 0) goto LAB_00583583;
    // 00583571  push edi
    // 00583572  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00583575  push 0x100000
    // 0058357A  push ecx
    // 0058357B  call 0x581190
    add_abil((int*)(ecx), 0x100000, edi);
    // 00583580  add esp, 0xc
LAB_00583583:
    // 00583583  mov edx, dword ptr [0x9a6638]
    edx = ((*(int*)((char*)g_009a6638)));
    // 00583589  mov eax, dword ptr [0x9a663c]
    eax = ((*(int*)((char*)g_009a663c)));
    // 0058358E  cmp edx, eax
    // 00583590  jle 0x5835c8
    if (((int)(edx)) <= ((int)(eax))) goto LAB_005835C8;
    // 00583592  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00583598  mov edx, dword ptr [esi + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + esi)));
    // 0058359E  add eax, edx
    eax = ((eax) + (edx));
    // 005835A0  test eax, eax
    // 005835A2  jle 0x5835c8
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_005835C8;
    // 005835A4  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 005835A9  mov edx, 1
    edx = (1);
    // 005835AE  mov ecx, edi
    ecx = (edi);
    // 005835B0  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 005835B2  test al, dl
    // 005835B4  jne 0x5835c8
    if (((((eax & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_005835C8;
    // 005835B6  push edi
    // 005835B7  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005835BA  push 0x100000
    // 005835BF  push ecx
    // 005835C0  call 0x581190
    add_abil((int*)(ecx), 0x100000, edi);
    // 005835C5  add esp, 0xc
LAB_005835C8:
    // 005835C8  mov eax, 2
    eax = (2);
    // 005835CD  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005835CF:
    // 005835CF  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005835D1  inc ecx
    ecx = ((ecx) + 1);
    // 005835D2  test al, 1
    // 005835D4  je 0x5835cf
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005835CF;
    // 005835D6  lea edx, [ecx*8]
    edx = (ecx*8);
    // 005835DD  push edi
    // 005835DE  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005835E0  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005835E8  push eax
    // 005835E9  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005835EE  add esp, 8
    // 005835F1  test eax, eax
    // 005835F3  je 0x583618
    if ((((eax) & (eax))) == 0) goto LAB_00583618;
    // 005835F5  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005835F8  test eax, eax
    // 005835FA  je 0x583610
    if ((((eax) & (eax))) == 0) goto LAB_00583610;
    // 005835FC  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00583602  push edi
    // 00583603  push ecx
    // 00583604  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583609  add esp, 8
    // 0058360C  test eax, eax
    // 0058360E  je 0x583618
    if ((((eax) & (eax))) == 0) goto LAB_00583618;
LAB_00583610:
    // 00583610  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583613  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 00583615  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00583618:
    // 00583618  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 0058361B  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0058361E  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 00583621  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 00583624  push 0
    // 00583626  push 8
    // 00583628  push edx
    // 00583629  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 0058362C  push eax
    // 0058362D  push ebx
    // 0058362E  push ecx
    // 0058362F  push edx
    // 00583630  push edi
    // 00583631  call 0x580860
    eax = propose_proto(edi, edx, ecx, ebx, eax, edx, 8, 0);
    // 00583636  push 0
    // 00583638  push 0x63
    // 0058363A  push edi
    // 0058363B  call 0x57f0b0
    eax = weapon_budget(edi, 0x63, 0);
    // 00583640  add esp, 0x2c
    // 00583643  mov esi, eax
    esi = (eax);
    // 00583645  mov dword ptr [ebp + 8], 0x20
    a1 = (0x20);
    // 0058364C  mov eax, 2
    eax = (2);
    // 00583651  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00583653:
    // 00583653  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00583655  inc ecx
    ecx = ((ecx) + 1);
    // 00583656  test al, 1
    // 00583658  je 0x583653
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00583653;
    // 0058365A  lea eax, [ecx*8]
    eax = (ecx*8);
    // 00583661  push edi
    // 00583662  sub eax, ecx
    eax = ((eax) - (ecx));
    // 00583664  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 0058366C  push ecx
    // 0058366D  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583672  add esp, 8
    // 00583675  test eax, eax
    // 00583677  je 0x583694
    if ((((eax) & (eax))) == 0) goto LAB_00583694;
    // 00583679  mov edx, dword ptr [0x949780]
    edx = ((*(int*)((char*)g_00949780)));
    // 0058367F  push edi
    // 00583680  push edx
    // 00583681  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00583686  add esp, 8
    // 00583689  test eax, eax
    // 0058368B  je 0x583694
    if ((((eax) & (eax))) == 0) goto LAB_00583694;
    // 0058368D  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583690  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 00583692  jmp 0x5836e3
    goto LAB_005836E3;
LAB_00583694:
    // 00583694  test byte ptr [ebp - 0x18], 1
    // 00583698  je 0x5836e6
    if (((((var_18 & 0xff)) & (1))) == 0) goto LAB_005836E6;
    // 0058369A  mov eax, 0x40000
    eax = (0x40000);
    // 0058369F  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_005836A1:
    // 005836A1  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005836A3  inc ecx
    ecx = ((ecx) + 1);
    // 005836A4  test al, 1
    // 005836A6  je 0x5836a1
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_005836A1;
    // 005836A8  lea eax, [ecx*8]
    eax = (ecx*8);
    // 005836AF  push edi
    // 005836B0  sub eax, ecx
    eax = ((eax) - (ecx));
    // 005836B2  movsx ecx, word ptr [eax*4 + 0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550 + eax*4)));
    // 005836BA  push ecx
    // 005836BB  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005836C0  add esp, 8
    // 005836C3  test eax, eax
    // 005836C5  je 0x5836e6
    if ((((eax) & (eax))) == 0) goto LAB_005836E6;
    // 005836C7  mov edx, dword ptr [0x949780]
    edx = ((*(int*)((char*)g_00949780)));
    // 005836CD  push edi
    // 005836CE  push edx
    // 005836CF  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 005836D4  add esp, 8
    // 005836D7  test eax, eax
    // 005836D9  je 0x5836e6
    if ((((eax) & (eax))) == 0) goto LAB_005836E6;
    // 005836DB  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005836DE  or eax, 0x40000
    eax = ((eax) | (0x40000));
LAB_005836E3:
    // 005836E3  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_005836E6:
    // 005836E6  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005836E9  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005836EC  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 005836EF  push 0
    // 005836F1  push 9
    // 005836F3  push eax
    // 005836F4  push ecx
    // 005836F5  push ebx
    // 005836F6  push esi
    // 005836F7  push edx
    // 005836F8  push edi
    // 005836F9  call 0x580860
    eax = propose_proto(edi, edx, esi, ebx, ecx, eax, 9, 0);
    // 005836FE  add esp, 0x20
LAB_00583701:
    // 00583701  push -2
    // 00583703  push 2
    // 00583705  push edi
    // 00583706  call 0x57eff0
    eax = pick_chassis(edi, 2, -2);
    // 0058370B  add esp, 0xc
    // 0058370E  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00583711  test eax, eax
    // 00583713  jl 0x5838fb
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005838FB;
    // 00583719  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 0058371B  push ebx
    // 0058371C  push 0x63
    // 0058371E  push edi
    // 0058371F  call 0x57f0b0
    eax = weapon_budget(edi, 0x63, ebx);
    // 00583724  add esp, 0xc
    // 00583727  mov dword ptr [ebp - 0xc], eax
    var_c = (eax);
    // 0058372A  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00583731  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583734  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583739:
    // 00583739  movsx eax, word ptr [esi + 4]
    eax = ((*(short*)((char*)(esi) + 0x4)));
    // 0058373D  push edi
    // 0058373E  push eax
    // 0058373F  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00583744  add esp, 8
    // 00583747  test eax, eax
    // 00583749  je 0x583786
    if ((((eax) & (eax))) == 0) goto LAB_00583786;
    // 0058374B  cmp byte ptr [esi + 2], 1
    // 0058374F  ja 0x583786
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00583786;
    // 00583751  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583753  test al, al
    // 00583755  jl 0x583786
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00583786;
    // 00583757  test edi, edi
    // 00583759  jl 0x583774
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00583774;
    // 0058375B  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00583761  test ecx, ecx
    // 00583763  jne 0x583774
    if ((((ecx) & (ecx))) != 0) goto LAB_00583774;
    // 00583765  cmp esi, 0x94f310
    // 0058376B  jle 0x583774
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00583774;
    // 0058376D  mov eax, 1
    eax = (1);
    // 00583772  jmp 0x583777
    goto LAB_00583777;
LAB_00583774:
    // 00583774  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00583777:
    // 00583777  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 0058377A  add eax, eax
    eax = ((eax) + (eax));
    // 0058377C  cmp eax, ecx
    // 0058377E  jl 0x583786
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00583786;
    // 00583780  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00583783  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00583786:
    // 00583786  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583789  inc ebx
    ebx = ((ebx) + 1);
    // 0058378A  cmp esi, 0x94f360
    // 00583790  jl 0x583739
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583739;
    // 00583792  mov ecx, edi
    ecx = (edi);
    // 00583794  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 0058379B  shl ecx, 6
    ecx = ((ecx) << (6));
    // 0058379E  add ecx, edi
    ecx = ((ecx) + (edi));
    // 005837A0  lea edx, [edi + ecx*2]
    edx = (edi + ecx*2);
    // 005837A3  lea eax, [edi + edx*8]
    eax = (edi + edx*8);
    // 005837A6  lea esi, [edi + eax*2]
    esi = (edi + eax*2);
    // 005837A9  shl esi, 2
    esi = ((esi) << (2));
    // 005837AC  cmp dword ptr [esi + 0x96c9e4], 6
    // 005837B3  jl 0x5837ee
    if (((int)((*(int*)((char*)g_0096c9e4 + esi)))) < ((int)(6))) goto LAB_005837EE;
    // 005837B5  mov eax, dword ptr [esi + 0x96cdc8]
    eax = ((*(int*)((char*)g_0096cdc8 + esi)));
    // 005837BB  test eax, eax
    // 005837BD  jne 0x5837c9
    if ((((eax) & (eax))) != 0) goto LAB_005837C9;
    // 005837BF  mov eax, dword ptr [esi + 0x96cdc4]
    eax = ((*(int*)((char*)g_0096cdc4 + esi)));
    // 005837C5  test eax, eax
    // 005837C7  je 0x5837ee
    if ((((eax) & (eax))) == 0) goto LAB_005837EE;
LAB_005837C9:
    // 005837C9  mov eax, dword ptr [esi + 0x96cdcc]
    eax = ((*(int*)((char*)g_0096cdcc + esi)));
    // 005837CF  test eax, eax
    // 005837D1  je 0x5837dc
    if ((((eax) & (eax))) == 0) goto LAB_005837DC;
    // 005837D3  cmp dword ptr [esi + 0x96cdbc], 1
    // 005837DA  jge 0x5837ee
    if (((int)((*(int*)((char*)g_0096cdbc + esi)))) >= ((int)(1))) goto LAB_005837EE;
LAB_005837DC:
    // 005837DC  push edi
    // 005837DD  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005837E0  push 0x10000
    // 005837E5  push ecx
    // 005837E6  call 0x581190
    add_abil((int*)(ecx), 0x10000, edi);
    // 005837EB  add esp, 0xc
LAB_005837EE:
    // 005837EE  test byte ptr [ebp - 0x18], 2
    // 005837F2  je 0x583806
    if (((((var_18 & 0xff)) & (2))) == 0) goto LAB_00583806;
    // 005837F4  push edi
    // 005837F5  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 005837F8  push 0x800
    // 005837FD  push edx
    // 005837FE  call 0x581190
    add_abil((int*)(edx), 0x800, edi);
    // 00583803  add esp, 0xc
LAB_00583806:
    // 00583806  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00583809  test ecx, ecx
    // 0058380B  je 0x583845
    if ((((ecx) & (ecx))) == 0) goto LAB_00583845;
    // 0058380D  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 00583813  mov edx, 4
    edx = (4);
    // 00583818  sub edx, eax
    edx = ((edx) - (eax));
    // 0058381A  cmp ecx, edx
    // 0058381C  jle 0x583845
    if (((int)(ecx)) <= ((int)(edx))) goto LAB_00583845;
    // 0058381E  test eax, eax
    // 00583820  jl 0x583845
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583845;
    // 00583822  mov eax, 1
    eax = (1);
    // 00583827  mov ecx, edi
    ecx = (edi);
    // 00583829  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 0058382B  test byte ptr [0x9a64e8], al
    // 00583831  jne 0x583845
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((eax & 0xff)))) != 0) goto LAB_00583845;
    // 00583833  push edi
    // 00583834  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00583837  push 0x100000
    // 0058383C  push ecx
    // 0058383D  call 0x581190
    add_abil((int*)(ecx), 0x100000, edi);
    // 00583842  add esp, 0xc
LAB_00583845:
    // 00583845  mov edx, dword ptr [0x9a6638]
    edx = ((*(int*)((char*)g_009a6638)));
    // 0058384B  mov eax, dword ptr [0x9a663c]
    eax = ((*(int*)((char*)g_009a663c)));
    // 00583850  cmp edx, eax
    // 00583852  jle 0x58388a
    if (((int)(edx)) <= ((int)(eax))) goto LAB_0058388A;
    // 00583854  mov eax, dword ptr [esi + 0x96cdbc]
    eax = ((*(int*)((char*)g_0096cdbc + esi)));
    // 0058385A  mov edx, dword ptr [esi + 0x96cdcc]
    edx = ((*(int*)((char*)g_0096cdcc + esi)));
    // 00583860  add eax, edx
    eax = ((eax) + (edx));
    // 00583862  test eax, eax
    // 00583864  jle 0x58388a
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_0058388A;
    // 00583866  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0058386B  mov edx, 1
    edx = (1);
    // 00583870  mov ecx, edi
    ecx = (edi);
    // 00583872  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00583874  test al, dl
    // 00583876  jne 0x58388a
    if (((((eax & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_0058388A;
    // 00583878  push edi
    // 00583879  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 0058387C  push 0x100000
    // 00583881  push ecx
    // 00583882  call 0x581190
    add_abil((int*)(ecx), 0x100000, edi);
    // 00583887  add esp, 0xc
LAB_0058388A:
    // 0058388A  mov eax, 2
    eax = (2);
    // 0058388F  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00583891:
    // 00583891  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00583893  inc ecx
    ecx = ((ecx) + 1);
    // 00583894  test al, 1
    // 00583896  je 0x583891
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00583891;
    // 00583898  lea edx, [ecx*8]
    edx = (ecx*8);
    // 0058389F  push edi
    // 005838A0  sub edx, ecx
    edx = ((edx) - (ecx));
    // 005838A2  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 005838AA  push eax
    // 005838AB  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 005838B0  add esp, 8
    // 005838B3  test eax, eax
    // 005838B5  je 0x5838da
    if ((((eax) & (eax))) == 0) goto LAB_005838DA;
    // 005838B7  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005838BA  test eax, eax
    // 005838BC  je 0x5838d2
    if ((((eax) & (eax))) == 0) goto LAB_005838D2;
    // 005838BE  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 005838C4  push edi
    // 005838C5  push ecx
    // 005838C6  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 005838CB  add esp, 8
    // 005838CE  test eax, eax
    // 005838D0  je 0x5838da
    if ((((eax) & (eax))) == 0) goto LAB_005838DA;
LAB_005838D2:
    // 005838D2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005838D5  or al, 2
    SETB0(eax, (((eax & 0xff)) | (2)));
    // 005838D7  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_005838DA:
    // 005838DA  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 005838DD  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005838E0  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 005838E3  push 0
    // 005838E5  push 0x11
    // 005838E7  push edx
    // 005838E8  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005838EB  push eax
    // 005838EC  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005838EF  push ecx
    // 005838F0  push edx
    // 005838F1  push eax
    // 005838F2  push edi
    // 005838F3  call 0x580860
    eax = propose_proto(edi, eax, edx, ecx, eax, edx, 0x11, 0);
    // 005838F8  add esp, 0x20
LAB_005838FB:
    // 005838FB  push -1
    // 005838FD  push 2
    // 005838FF  push edi
    // 00583900  call 0x57eff0
    eax = pick_chassis(edi, 2, -1);
    // 00583905  add esp, 0xc
    // 00583908  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0058390B  test eax, eax
    // 0058390D  jl 0x583a6c
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583A6C;
    // 00583913  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583915  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 0058391C  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 0058391F  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583922  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583927:
    // 00583927  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 0058392B  push edi
    // 0058392C  push ecx
    // 0058392D  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583932  add esp, 8
    // 00583935  test eax, eax
    // 00583937  je 0x583974
    if ((((eax) & (eax))) == 0) goto LAB_00583974;
    // 00583939  cmp byte ptr [esi + 2], 1
    // 0058393D  ja 0x583974
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00583974;
    // 0058393F  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583941  test al, al
    // 00583943  jl 0x583974
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00583974;
    // 00583945  test edi, edi
    // 00583947  jl 0x583962
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00583962;
    // 00583949  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 0058394F  test ecx, ecx
    // 00583951  jne 0x583962
    if ((((ecx) & (ecx))) != 0) goto LAB_00583962;
    // 00583953  cmp esi, 0x94f310
    // 00583959  jle 0x583962
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00583962;
    // 0058395B  mov eax, 1
    eax = (1);
    // 00583960  jmp 0x583965
    goto LAB_00583965;
LAB_00583962:
    // 00583962  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00583965:
    // 00583965  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00583968  add eax, eax
    eax = ((eax) + (eax));
    // 0058396A  cmp eax, ecx
    // 0058396C  jl 0x583974
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00583974;
    // 0058396E  mov ebx, dword ptr [ebp - 0x38]
    ebx = (var_38);
    // 00583971  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
LAB_00583974:
    // 00583974  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 00583977  add esi, 0x10
    esi = ((esi) + (0x10));
    // 0058397A  inc edx
    edx = ((edx) + 1);
    // 0058397B  cmp esi, 0x94f360
    // 00583981  mov dword ptr [ebp - 0x38], edx
    var_38 = (edx);
    // 00583984  jl 0x583927
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583927;
    // 00583986  movsx edx, word ptr [0x94afdc]
    edx = ((*(short*)((char*)g_0094afdc)));
    // 0058398D  push edi
    // 0058398E  push edx
    // 0058398F  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583992  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00583999  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 0058399E  add esp, 8
    // 005839A1  test eax, eax
    // 005839A3  je 0x5839c3
    if ((((eax) & (eax))) == 0) goto LAB_005839C3;
    // 005839A5  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005839A8  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005839AB  mov esi, dword ptr [ebp - 4]
    esi = (var_4);
    // 005839AE  push 0
    // 005839B0  push 0xd
    // 005839B2  push eax
    // 005839B3  push ecx
    // 005839B4  push ebx
    // 005839B5  push 0x17
    // 005839B7  push esi
    // 005839B8  push edi
    // 005839B9  call 0x580860
    eax = propose_proto(edi, esi, 0x17, ebx, ecx, eax, 0xd, 0);
    // 005839BE  add esp, 0x20
    // 005839C1  jmp 0x5839c6
    goto LAB_005839C6;
LAB_005839C3:
    // 005839C3  mov esi, dword ptr [ebp - 4]
    esi = (var_4);
LAB_005839C6:
    // 005839C6  movsx edx, word ptr [0x94afec]
    edx = ((*(short*)((char*)g_0094afec)));
    // 005839CD  push edi
    // 005839CE  push edx
    // 005839CF  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 005839D4  add esp, 8
    // 005839D7  test eax, eax
    // 005839D9  je 0x5839f4
    if ((((eax) & (eax))) == 0) goto LAB_005839F4;
    // 005839DB  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 005839DE  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005839E1  push 0
    // 005839E3  push 0x16
    // 005839E5  push eax
    // 005839E6  push ecx
    // 005839E7  push ebx
    // 005839E8  push 0x18
    // 005839EA  push esi
    // 005839EB  push edi
    // 005839EC  call 0x580860
    eax = propose_proto(edi, esi, 0x18, ebx, ecx, eax, 0x16, 0);
    // 005839F1  add esp, 0x20
LAB_005839F4:
    // 005839F4  movsx edx, word ptr [0x94affc]
    edx = ((*(short*)((char*)g_0094affc)));
    // 005839FB  push edi
    // 005839FC  push edx
    // 005839FD  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00583A02  add esp, 8
    // 00583A05  test eax, eax
    // 00583A07  je 0x583a22
    if ((((eax) & (eax))) == 0) goto LAB_00583A22;
    // 00583A09  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00583A0C  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00583A0F  push 0
    // 00583A11  push 0x17
    // 00583A13  push eax
    // 00583A14  push ecx
    // 00583A15  push ebx
    // 00583A16  push 0x19
    // 00583A18  push esi
    // 00583A19  push edi
    // 00583A1A  call 0x580860
    eax = propose_proto(edi, esi, 0x19, ebx, ecx, eax, 0x17, 0);
    // 00583A1F  add esp, 0x20
LAB_00583A22:
    // 00583A22  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583A24  mov esi, 0x94ae68
    esi = (0x94ae68);
LAB_00583A29:
    // 00583A29  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00583A2D  push edi
    // 00583A2E  push edx
    // 00583A2F  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00583A34  add esp, 8
    // 00583A37  test eax, eax
    // 00583A39  je 0x583a40
    if ((((eax) & (eax))) == 0) goto LAB_00583A40;
    // 00583A3B  cmp byte ptr [esi], 0x63
    // 00583A3E  jge 0x583a4e
    if (((int)((*(unsigned char*)((char*)(esi))))) >= ((int)(0x63))) goto LAB_00583A4E;
LAB_00583A40:
    // 00583A40  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583A43  inc ebx
    ebx = ((ebx) + 1);
    // 00583A44  cmp esi, 0x94b008
    // 00583A4A  jl 0x583a29
    if (((int)(esi)) < ((int)(0x94b008))) goto LAB_00583A29;
    // 00583A4C  jmp 0x583a6c
    goto LAB_00583A6C;
LAB_00583A4E:
    // 00583A4E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00583A51  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00583A54  mov edx, dword ptr [ebp - 0x38]
    edx = (var_38);
    // 00583A57  push 0
    // 00583A59  push 0xe
    // 00583A5B  push eax
    // 00583A5C  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00583A5F  push ecx
    // 00583A60  push edx
    // 00583A61  push ebx
    // 00583A62  push eax
    // 00583A63  push edi
    // 00583A64  call 0x580860
    eax = propose_proto(edi, eax, ebx, edx, ecx, eax, 0xe, 0);
    // 00583A69  add esp, 0x20
LAB_00583A6C:
    // 00583A6C  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00583A6F  mov dword ptr [ebp + 8], 0
    a1 = (0);
    // 00583A76  cmp eax, 1
    // 00583A79  jg 0x583b79
    if (((int)(eax)) > ((int)(1))) goto LAB_00583B79;
    // 00583A7F  movsx ecx, word ptr [0x9ab550]
    ecx = ((*(short*)((char*)g_009ab550)));
    // 00583A86  push edi
    // 00583A87  push ecx
    // 00583A88  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583A8D  add esp, 8
    // 00583A90  test eax, eax
    // 00583A92  jne 0x583b79
    if ((((eax) & (eax))) != 0) goto LAB_00583B79;
    // 00583A98  mov eax, 0x2000
    eax = (0x2000);
    // 00583A9D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00583A9F:
    // 00583A9F  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00583AA1  inc ecx
    ecx = ((ecx) + 1);
    // 00583AA2  test al, 1
    // 00583AA4  je 0x583a9f
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00583A9F;
    // 00583AA6  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00583AAD  push edi
    // 00583AAE  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00583AB0  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00583AB8  push eax
    // 00583AB9  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00583ABE  add esp, 8
    // 00583AC1  test eax, eax
    // 00583AC3  je 0x583c88
    if ((((eax) & (eax))) == 0) goto LAB_00583C88;
    // 00583AC9  push 1
    // 00583ACB  push 0
    // 00583ACD  push edi
    // 00583ACE  call 0x57eff0
    eax = pick_chassis(edi, 0, 1);
    // 00583AD3  add esp, 0xc
    // 00583AD6  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00583AD9  test eax, eax
    // 00583ADB  jl 0x583c88
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583C88;
    // 00583AE1  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583AE3  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00583AEA  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583AED  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583AF2:
    // 00583AF2  movsx ecx, word ptr [esi + 4]
    ecx = ((*(short*)((char*)(esi) + 0x4)));
    // 00583AF6  push edi
    // 00583AF7  push ecx
    // 00583AF8  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583AFD  add esp, 8
    // 00583B00  test eax, eax
    // 00583B02  je 0x583b3f
    if ((((eax) & (eax))) == 0) goto LAB_00583B3F;
    // 00583B04  cmp byte ptr [esi + 2], 1
    // 00583B08  ja 0x583b3f
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00583B3F;
    // 00583B0A  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583B0C  test al, al
    // 00583B0E  jl 0x583b3f
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00583B3F;
    // 00583B10  test edi, edi
    // 00583B12  jl 0x583b2d
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00583B2D;
    // 00583B14  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00583B1A  test ecx, ecx
    // 00583B1C  jne 0x583b2d
    if ((((ecx) & (ecx))) != 0) goto LAB_00583B2D;
    // 00583B1E  cmp esi, 0x94f310
    // 00583B24  jle 0x583b2d
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00583B2D;
    // 00583B26  mov eax, 1
    eax = (1);
    // 00583B2B  jmp 0x583b30
    goto LAB_00583B30;
LAB_00583B2D:
    // 00583B2D  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00583B30:
    // 00583B30  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00583B33  add eax, eax
    eax = ((eax) + (eax));
    // 00583B35  cmp eax, ecx
    // 00583B37  jl 0x583b3f
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00583B3F;
    // 00583B39  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00583B3C  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00583B3F:
    // 00583B3F  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583B42  inc ebx
    ebx = ((ebx) + 1);
    // 00583B43  cmp esi, 0x94f360
    // 00583B49  jl 0x583af2
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583AF2;
    // 00583B4B  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00583B4E  mov eax, dword ptr [ebp - 0x38]
    eax = (var_38);
    // 00583B51  mov ecx, dword ptr [ebp - 4]
    ecx = (var_4);
    // 00583B54  push 0
    // 00583B56  push -1
    // 00583B58  push edx
    // 00583B59  push 0x2000
    // 00583B5E  push eax
    // 00583B5F  push 0x12
    // 00583B61  push ecx
    // 00583B62  push edi
    // 00583B63  mov dword ptr [ebp + 8], 0x2000
    a1 = (0x2000);
    // 00583B6A  call 0x580860
    eax = propose_proto(edi, ecx, 0x12, eax, 0x2000, edx, -1, 0);
    // 00583B6F  add esp, 0x20
    return;
LAB_00583B79:
    // 00583B79  push 2
    // 00583B7B  push 0
    // 00583B7D  push edi
    // 00583B7E  call 0x57eff0
    eax = pick_chassis(edi, 0, 2);
    // 00583B83  add esp, 0xc
    // 00583B86  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 00583B89  test eax, eax
    // 00583B8B  jl 0x583c88
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00583C88;
    // 00583B91  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 00583B93  mov dword ptr [ebp - 0x3c], 0xffffffff
    var_3c = (0xffffffff);
    // 00583B9A  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
    // 00583B9D  mov esi, 0x94f280
    esi = (0x94f280);
LAB_00583BA2:
    // 00583BA2  movsx edx, word ptr [esi + 4]
    edx = ((*(short*)((char*)(esi) + 0x4)));
    // 00583BA6  push edi
    // 00583BA7  push edx
    // 00583BA8  call 0x5b9f20
    eax = has_tech(edx, edi);
    // 00583BAD  add esp, 8
    // 00583BB0  test eax, eax
    // 00583BB2  je 0x583bef
    if ((((eax) & (eax))) == 0) goto LAB_00583BEF;
    // 00583BB4  cmp byte ptr [esi + 2], 1
    // 00583BB8  ja 0x583bef
    if (((unsigned int)(*(unsigned char*)((char*)(esi) + 0x2))) > ((unsigned int)1)) goto LAB_00583BEF;
    // 00583BBA  mov al, byte ptr [esi]
    SETB0(eax, ((*(unsigned char*)((char*)(esi)))));
    // 00583BBC  test al, al
    // 00583BBE  jl 0x583bef
    if (((signed char)(((eax & 0xff)) & ((eax & 0xff)))) < 0) goto LAB_00583BEF;
    // 00583BC0  test edi, edi
    // 00583BC2  jl 0x583bdd
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00583BDD;
    // 00583BC4  mov ecx, dword ptr [0x9a6488]
    ecx = ((*(int*)((char*)g_009a6488)));
    // 00583BCA  test ecx, ecx
    // 00583BCC  jne 0x583bdd
    if ((((ecx) & (ecx))) != 0) goto LAB_00583BDD;
    // 00583BCE  cmp esi, 0x94f310
    // 00583BD4  jle 0x583bdd
    if (((int)(esi)) <= ((int)(0x94f310))) goto LAB_00583BDD;
    // 00583BD6  mov eax, 1
    eax = (1);
    // 00583BDB  jmp 0x583be0
    goto LAB_00583BE0;
LAB_00583BDD:
    // 00583BDD  movsx eax, al
    eax = (((signed char)(eax & 0xff)));
LAB_00583BE0:
    // 00583BE0  mov ecx, dword ptr [ebp - 0x3c]
    ecx = (var_3c);
    // 00583BE3  add eax, eax
    eax = ((eax) + (eax));
    // 00583BE5  cmp eax, ecx
    // 00583BE7  jl 0x583bef
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00583BEF;
    // 00583BE9  mov dword ptr [ebp - 0x3c], eax
    var_3c = (eax);
    // 00583BEC  mov dword ptr [ebp - 0x38], ebx
    var_38 = (ebx);
LAB_00583BEF:
    // 00583BEF  add esi, 0x10
    esi = ((esi) + (0x10));
    // 00583BF2  inc ebx
    ebx = ((ebx) + 1);
    // 00583BF3  cmp esi, 0x94f360
    // 00583BF9  jl 0x583ba2
    if (((int)(esi)) < ((int)(0x94f360))) goto LAB_00583BA2;
    // 00583BFB  movsx eax, word ptr [0x9ab550]
    eax = ((*(short*)((char*)g_009ab550)));
    // 00583C02  push edi
    // 00583C03  push eax
    // 00583C04  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00583C09  add esp, 8
    // 00583C0C  test eax, eax
    // 00583C0E  je 0x583c18
    if ((((eax) & (eax))) == 0) goto LAB_00583C18;
    // 00583C10  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583C13  or al, 1
    SETB0(eax, (((eax & 0xff)) | (1)));
    // 00583C15  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00583C18:
    // 00583C18  mov eax, 0x2000
    eax = (0x2000);
    // 00583C1D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
LAB_00583C1F:
    // 00583C1F  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00583C21  inc ecx
    ecx = ((ecx) + 1);
    // 00583C22  test al, 1
    // 00583C24  je 0x583c1f
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_00583C1F;
    // 00583C26  lea edx, [ecx*8]
    edx = (ecx*8);
    // 00583C2D  push edi
    // 00583C2E  sub edx, ecx
    edx = ((edx) - (ecx));
    // 00583C30  movsx eax, word ptr [edx*4 + 0x9ab550]
    eax = ((*(short*)((char*)g_009ab550 + edx*4)));
    // 00583C38  push eax
    // 00583C39  call 0x5b9f20
    eax = has_tech(eax, edi);
    // 00583C3E  add esp, 8
    // 00583C41  test eax, eax
    // 00583C43  je 0x583c69
    if ((((eax) & (eax))) == 0) goto LAB_00583C69;
    // 00583C45  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583C48  test eax, eax
    // 00583C4A  je 0x583c60
    if ((((eax) & (eax))) == 0) goto LAB_00583C60;
    // 00583C4C  mov ecx, dword ptr [0x949780]
    ecx = ((*(int*)((char*)g_00949780)));
    // 00583C52  push edi
    // 00583C53  push ecx
    // 00583C54  call 0x5b9f20
    eax = has_tech(ecx, edi);
    // 00583C59  add esp, 8
    // 00583C5C  test eax, eax
    // 00583C5E  je 0x583c69
    if ((((eax) & (eax))) == 0) goto LAB_00583C69;
LAB_00583C60:
    // 00583C60  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583C63  or ah, 0x20
    SETB1(eax, ((((eax >> 8) & 0xff)) | (0x20)));
    // 00583C66  mov dword ptr [ebp + 8], eax
    a1 = (eax);
LAB_00583C69:
    // 00583C69  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00583C6C  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00583C6F  mov ecx, dword ptr [ebp - 0x38]
    ecx = (var_38);
    // 00583C72  push 0
    // 00583C74  push -1
    // 00583C76  push edx
    // 00583C77  mov edx, dword ptr [ebp - 4]
    edx = (var_4);
    // 00583C7A  push eax
    // 00583C7B  push ecx
    // 00583C7C  push 0x12
    // 00583C7E  push edx
    // 00583C7F  push edi
    // 00583C80  call 0x580860
    eax = propose_proto(edi, edx, 0x12, ecx, eax, edx, -1, 0);
    // 00583C85  add esp, 0x20
LAB_00583C88:
    return;
}
