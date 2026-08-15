// ORIGINAL: 0x0057F4B0 FILE
// RULED-OUT: full goto-per-branch literal transcription of all 1404
//            instructions (register vars + B/W/D/SB/SW byte-address macros);
//            compiles and matches control flow/field offsets/call order but
//            diverges at instr #0 (prologue/frame layout, MSVC's register
//            allocation for this huge frame is not reproduced by hand).
// working copy - scaffold materialised by --work
// name      ?prune_protos@@YAXHHH@Z
// size      5033 bytes
// spans     0x0057F4B0-0x00580859
// prototype 
// callers   4   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x00501500 0x0050BA30 0x00530D50 0x00530E50 0x0057D8E0 0x00593420 0x005B9F20 0x005BF310 0x006169A0 0x00625EC0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?prune_protos@@YAXHHH@Z  at 0x0057F4B0  (5033 bytes)
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

typedef int BOOL;
class Heap;
typedef void * LPVOID;
class NetDaemon;
class Strings;

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

class NetDaemon { public:
    int lock_base(int, int, int, int);
    int receive();
    uint32_t unlock_veh();
    void unlock_base(int);
};

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

bool has_tech(int, int);
extern "C" char *strcat(char *, const char *);
int __cdecl X_pop(const char *, int (__cdecl *)());
int bit_count(int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int psi_factor(int, int, int, int);
void say_stats_2(char *, int);
void synch_obs(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00408418 = (int *)0x00408418;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068f164 = (int *)0x0068F164;
static int *const g_0068f174 = (int *)0x0068F174;
static int *const g_0068f184 = (int *)0x0068F184;
static int *const g_0068f190 = (int *)0x0068F190;
static int *const g_0068f19c = (int *)0x0068F19C;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094979c = (int *)0x0094979C;
static int *const g_009497a8 = (int *)0x009497A8;
static int *const g_0094a378 = (int *)0x0094A378;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_0094a37a = (int *)0x0094A37A;
static int *const g_0094ae68 = (int *)0x0094AE68;
static int *const g_0094f280 = (int *)0x0094F280;
static int *const g_0096d20c = (int *)0x0096D20C;
static int *const g_0096d238 = (int *)0x0096D238;
static int *const g_0096d438 = (int *)0x0096D438;
static int *const g_0097d08c = (int *)0x0097D08C;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab540 = (int *)0x009AB540;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab86c = (int *)0x009AB86C;
static int *const g_009ab888 = (int *)0x009AB888;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab88d = (int *)0x009AB88D;
static int *const g_009ab88e = (int *)0x009AB88E;
static int *const g_009ab88f = (int *)0x009AB88F;
static int *const g_009ab890 = (int *)0x009AB890;
static int *const g_009ab891 = (int *)0x009AB891;
static int *const g_009ab892 = (int *)0x009AB892;
static int *const g_009ab893 = (int *)0x009AB893;
static int *const g_009ab894 = (int *)0x009AB894;
static int *const g_009ab898 = (int *)0x009AB898;
static int *const g_009ab89a = (int *)0x009AB89A;
static int *const g_009abb99 = (int *)0x009ABB99;
static int *const g_009ac58d = (int *)0x009AC58D;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
// Literal address-arithmetic helpers: the original addresses every field as
// `index_in_bytes + FIXED_ABSOLUTE_CONSTANT`, so these macros reproduce that
// exactly instead of routing through a struct type this unit does not have.
#define B(a)  (*(unsigned char *)(unsigned int)(a))
#define SB(a) (*(signed char *)(unsigned int)(a))
#define W(a)  (*(unsigned short *)(unsigned int)(a))
#define SW(a) (*(short *)(unsigned int)(a))
#define D(a)  (*(int *)(unsigned int)(a))

void __cdecl prune_protos(int a1, int a2, int a3) {
    int eax, ebx, ecx, edx, esi, edi;
    int local_8, local_c, local_10, local_14, local_18, local_1c, local_20,
        local_24, local_28, local_2c, local_30;
    unsigned char local_1;
    int _p1, _p2, _p3, _p4, _p5;
    int _pp_this, _pp_a, _pp_b, _pp_c, _pp_cb;
    int cond_fe2c;

    eax = a2;
    edi = a1;
    if (eax < 0x40) goto L_0057F4D8;
    ecx = eax + eax * 2;
    edx = eax + ecx * 4;
    if ((B(edx * 4 + 0x9AB898) & 4) == 0) goto L_0057F699;
L_0057F4D8:
    eax = edi + edi * 2;
    local_2c = 0x40;
    edi = edi + eax * 4;
    edi = edi << 8;
L_0057F4E8:
    eax = (eax & ~0xFFFF) | W(edi + 0x9AB898);
    if ((eax & 0xFF & 1) == 0) goto L_0057F689;
    if ((eax & 0xFF & 4) != 0) goto L_0057F689;
    eax = 0;
    local_1c = eax;
    local_20 = eax;
    local_24 = eax;
    local_18 = eax;
    edx = a1;
L_0057F50D:
    if (eax >= 0x40) goto L_0057F519;
    ecx = 0;
    goto L_0057F51E;
L_0057F519:
    ecx = edx;
    ecx = ecx << 6;
L_0057F51E:
    eax = eax & 0x8000003f;
    if (eax >= 0) goto L_0057F52A;
    eax = eax - 1;
    eax = eax | 0xffffffc0;
    eax = eax + 1;
L_0057F52A:
    eax = eax + ecx;
    ecx = eax + eax * 2;
    esi = eax + ecx * 4;
    esi = esi << 2;
    ecx = (ecx & ~0xFFFF) | W(esi + 0x9AB898);
    if ((ecx & 0xFF & 1) == 0) goto L_0057F659;
    if (eax >= 0x40) goto L_0057F565;
    _p1 = edx;
    edx = SW(esi + 0x9AB89A);
    _p2 = edx;
    eax = has_tech(_p2, _p1);
    if (eax == 0) goto L_0057F659;
    goto L_0057F56E;
L_0057F565:
    if ((ecx & 0xFF & 4) == 0) goto L_0057F659;
L_0057F56E:
    eax = (eax & ~0xFF) | B(esi + 0x9AB88D);
    ebx = (ebx & ~0xFF) | B(edi + 0x9AB88D);
    if ((eax & 0xFF) != (ebx & 0xFF)) goto L_0057F588;
    local_24 = 1;
L_0057F588:
    eax = (eax & ~0xFF) | B(esi + 0x9AB88E);
    edx = (edx & ~0xFF) | B(edi + 0x9AB88E);
    local_14 = (eax & 0xFF);
    local_30 = (edx & 0xFF);
    if ((eax & 0xFF) != (edx & 0xFF)) goto L_0057F5A5;
    local_20 = 1;
L_0057F5A5:
    eax = (eax & ~0xFF) | B(esi + 0x9AB88C);
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB88C);
    if ((eax & 0xFF) != (ecx & 0xFF)) goto L_0057F5BC;
    local_1c = 1;
L_0057F5BC:
    ebx = ebx & 0xff;
    ebx = ebx << 4;
    ebx = (ebx & ~0xFF) | B(ebx + 0x94AE68);
    if ((signed char)(ebx & 0xFF) <= 0) goto L_0057F5ED;
    edx = local_28;
    edx = edx & 0xff;
    edx = edx << 4;
    if (SB(edx + 0x94AE68) < (signed char)(ebx & 0xFF)) goto L_0057F5EA;
    local_24 = 1;
L_0057F5EA:
    edx = local_30;
L_0057F5ED:
    edx = edx & 0xff;
    edx = edx << 4;
    edx = (edx & ~0xFF) | B(edx + 0x94F280);
    if ((signed char)(edx & 0xFF) <= 0) goto L_0057F61B;
    esi = local_14;
    esi = esi & 0xff;
    esi = esi << 4;
    if (SB(esi + 0x94F280) < (signed char)(edx & 0xFF)) goto L_0057F61B;
    local_20 = 1;
L_0057F61B:
    eax = eax & 0xff;
    ecx = ecx & 0xff;
    eax = eax + eax * 8;
    ecx = ecx + ecx * 8;
    eax = eax << 4;
    ecx = ecx << 4;
    ebx = (ebx & ~0xFF) | B(eax + 0x94A379);
    edx = (edx & ~0xFF) | B(ecx + 0x94A379);
    if ((edx & 0xFF) != (ebx & 0xFF)) goto L_0057F659;
    eax = (eax & ~0xFF) | B(eax + 0x94A378);
    edx = (edx & ~0xFF) | B(ecx + 0x94A378);
    if ((unsigned char)(eax & 0xFF) < (unsigned char)(edx & 0xFF)) goto L_0057F659;
    local_1c = 1;
L_0057F659:
    eax = local_18;
    eax = eax + 1;
    local_18 = eax;
    if (eax < 0x80) goto L_0057F50D;
    eax = local_24;
    if (eax == 0) goto L_0057F689;
    eax = local_20;
    if (eax == 0) goto L_0057F689;
    eax = local_1c;
    if (eax == 0) goto L_0057F689;
    W(edi + 0x9AB898) |= 0x104;
L_0057F689:
    eax = local_2c;
    edi = edi + 0x34;
    eax = eax - 1;
    local_2c = eax;
    if (eax != 0) goto L_0057F4E8;
L_0057F699:
    ecx = a1;
    eax = (eax & ~0xFF) | B(0x9A64E8);
    edx = 1;
    edx = edx << (ecx & 0xFF);
    local_28 = edx;
    if (((edx & 0xFF) & (eax & 0xFF)) != 0) goto L_0057FCFA;
    ebx = ecx;
    local_1c = 0x40;
    esi = ebx;
    ecx = ebx + ebx * 2;
    esi = esi << 6;
    ebx = ebx + ecx * 4;
    local_20 = esi;
    ebx = ebx << 8;
    edi = ebx;
    local_18 = esi;
    local_24 = ebx;
    a2 = edi;
    goto L_0057F6DD;
L_0057F6DA:
    edx = local_28;
L_0057F6DD:
    eax = (eax & ~0xFFFF) | W(edi + 0x9AB898);
    if ((eax & 0xFF & 1) == 0) goto L_0057FCD9;
    if ((B(edi + 0x9AB894) & (edx & 0xFF)) != 0) goto L_0057FCD9;
    if (local_18 >= 0x40) goto L_0057F81A;
    eax = SW(edi + 0x9AB89A);
    edx = a1;
    _p1 = edx;
    _p2 = eax;
    eax = has_tech(_p2, _p1);
    if (eax == 0) goto L_0057FCD9;
L_0057F71E:
    local_30 = 1;
    local_8 = 0;
    esi = 0x9AB88D;
L_0057F731:
    if ((B(esi + 0xB) & 1) == 0) goto L_0057F9F4;
    ecx = local_8;
    eax = local_18;
    if (ecx == eax) goto L_0057F9F4;
    if (esi >= 0x9AC58D) goto L_0057F772;
    edx = 0;
    edx = (edx & ~0xFF) | B(esi);
    edx = edx << 4;
    eax = (eax & ~0xFF) | B(edx + 0x94AE68);
    if ((signed char)(eax & 0xFF) < 0) goto L_0057F9F4;
    if (esi == 0x9ABB99) goto L_0057F9F4;
L_0057F772:
    eax = (eax & ~0xFF) | B(esi + 6);
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB893);
    if ((eax & 0xFF) != (ecx & 0xFF)) goto L_0057F9F4;
    eax = 0;
    ecx = 0;
    eax = B(esi - 1);
    ecx = B(edi + 0x9AB88C);
    eax = eax + eax * 8;
    ecx = ecx + ecx * 8;
    eax = eax << 4;
    ecx = ecx << 4;
    edx = (edx & ~0xFF) | B(eax + 0x94A379);
    if ((edx & 0xFF) != B(ecx + 0x94A379)) goto L_0057F9F4;
    edx = (edx & ~0xFF) | B(esi + 5);
    ebx = (ebx & ~0xFF) | B(edi + 0x9AB892);
    if ((edx & 0xFF) != (ebx & 0xFF)) goto L_0057F9F4;
    edx = (edx & ~0xFF) | B(esi + 2);
    ebx = (ebx & ~0xFF) | B(edi + 0x9AB88F);
    if ((unsigned char)(edx & 0xFF) >= (unsigned char)(ebx & 0xFF)) goto L_0057F7E1;
    edx = 0;
    edx = (edx & ~0xFF) | B(esi);
    edx = edx << 4;
    ebx = (ebx & ~0xFF) | B(edx + 0x94AE68);
    if ((signed char)(ebx & 0xFF) >= 0) goto L_0057F9D2;
L_0057F7E1:
    eax = (eax & ~0xFF) | B(eax + 0x94A37A);
    edx = (edx & ~0xFF) | B(ecx + 0x94A37A);
    if ((eax & 0xFF) != (edx & 0xFF)) goto L_0057F9F4;
    eax = (eax & ~0xFF) | B(esi);
    ecx = eax;
    ecx = ecx & 0xff;
    ecx = ecx << 4;
    ecx = (ecx & ~0xFF) | B(ecx + 0x94AE68);
    if ((signed char)(ecx & 0xFF) > 0) goto L_0057F82E;
    if ((eax & 0xFF) != B(edi + 0x9AB88D)) goto L_0057F9F4;
    goto L_0057F845;
L_0057F81A:
    if ((eax & 0xFF & 4) != 0) goto L_0057F71E;
    local_30 = 0;
    goto L_0057FA10;
L_0057F82E:
    edx = 0;
    edx = (edx & ~0xFF) | B(edi + 0x9AB88D);
    edx = edx << 4;
    if ((signed char)(ecx & 0xFF) > SB(edx + 0x94AE68)) goto L_0057F9F4;
L_0057F845:
    eax = (eax & ~0xFF) | B(esi + 3);
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB890);
    if ((unsigned char)(eax & 0xFF) > (unsigned char)(ecx & 0xFF)) goto L_0057F9F4;
    ecx = (ecx & ~0xFF) | B(esi + 1);
    eax = ecx;
    eax = eax & 0xff;
    edx = eax;
    edx = edx << 4;
    ebx = (ebx & ~0xFF) | B(edx + 0x94F280);
    if ((signed char)(ebx & 0xFF) > 0) goto L_0057F880;
    if ((ecx & 0xFF) != B(edi + 0x9AB88E)) goto L_0057F9F4;
    goto L_0057F974;
L_0057F880:
    ebx = a1;
    if (ebx < 0) goto L_0057F8E4;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057F89D;
    if (eax <= 9) goto L_0057F89D;
    eax = 1;
    goto L_0057F8EE;
L_0057F89D:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057F8DF;
    eax = ebx;
    _p1 = 0;
    eax = eax << 6;
    eax = eax + ebx;
    _p2 = 0;
    _p3 = ebx;
    ecx = ebx + eax * 2;
    edx = ebx + ecx * 8;
    eax = ebx + edx * 2;
    eax = D(eax * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057F8EE;
L_0057F8DF:
    eax = (signed char)(eax & 0xFF);
    goto L_0057F8EE;
L_0057F8E4:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057F8EE:
    ecx = a2;
    edi = eax + eax;
    eax = 0;
    eax = B(ecx + 0x9AB88E);
    if (ebx < 0) goto L_0057F95D;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057F916;
    if (eax <= 9) goto L_0057F916;
    eax = 1;
    goto L_0057F967;
L_0057F916:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057F958;
    edx = ebx;
    _p1 = 0;
    edx = edx << 6;
    edx = edx + ebx;
    _p2 = 0;
    _p3 = ebx;
    eax = ebx + edx * 2;
    ecx = ebx + eax * 8;
    edx = ebx + ecx * 2;
    eax = D(edx * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057F967;
L_0057F958:
    eax = (signed char)(eax & 0xFF);
    goto L_0057F967;
L_0057F95D:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057F967:
    eax = eax + eax;
    if (edi > eax) { edi = a2; goto L_0057F9F4; }
    edi = a2;
L_0057F974:
    eax = 0;
    eax = B(edi + 0x9AB88C);
    ecx = eax + eax * 8;
    eax = 0;
    eax = B(esi - 1);
    ecx = ecx << 4;
    edx = eax + eax * 8;
    edx = edx << 4;
    eax = (eax & ~0xFF) | B(edx + 0x94A378);
    edx = (edx & ~0xFF) | B(ecx + 0x94A378);
    if ((unsigned char)(eax & 0xFF) > (unsigned char)(edx & 0xFF)) goto L_0057F9F4;
    ecx = D(esi - 5);
    ebx = D(edi + 0x9AB888);
    ecx = ecx & ebx;
    _p1 = ecx;
    eax = bit_count(_p1);
    edx = D(esi - 5);
    ebx = eax;
    _p2 = edx;
    eax = bit_count(_p2);
    if (ebx < eax) goto L_0057F9F4;
    eax = D(esi - 5);
    edx = D(edi + 0x9AB888);
    eax = eax ^ edx;
    if ((eax & 0x408418) != 0) goto L_0057F9F4;
L_0057F9D2:
    ecx = a1;
    eax = a3;
    edx = (edx & ~0xFF) | 1;
    edx = (edx & ~0xFF) | (unsigned char)(1 << (ecx & 0xFF));
    ecx = (ecx & ~0xFF) | B(esi + 7);
    ecx = (ecx & ~0xFF) | ((ecx & 0xFF) | (edx & 0xFF));
    B(esi + 7) = (unsigned char)(ecx & 0xFF);
    if (eax == 0) goto L_0057F9F4;
    eax = local_8;
    _p1 = eax;
    synch_obs(_p1);
L_0057F9F4:
    edx = local_8;
    esi = esi + 0x34;
    edx = edx + 1;
    local_8 = edx;
    if (esi < 0x9AC58D) goto L_0057F731;
    ebx = local_24;
    esi = local_20;
L_0057FA10:
    local_14 = esi;
    esi = ebx + 0x9AB892;
    local_2c = 0x40;
L_0057FA20:
    edx = (edx & ~0xFFFF) | W(esi + 6);
    if ((edx & 0xFF & 1) == 0) goto L_0057FCBC;
    ecx = local_18;
    eax = local_14;
    if (eax == ecx) goto L_0057FCBC;
    eax = (eax & ~0xFF) | B(esi + 1);
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB893);
    if ((eax & 0xFF) != (ecx & 0xFF)) goto L_0057FCBC;
    eax = 0;
    ecx = 0;
    eax = B(esi - 6);
    ecx = B(edi + 0x9AB88C);
    eax = eax + eax * 8;
    ecx = ecx + ecx * 8;
    eax = eax << 4;
    ecx = ecx << 4;
    ebx = (ebx & ~0xFF) | B(eax + 0x94A379);
    if ((ebx & 0xFF) != B(ecx + 0x94A379)) goto L_0057FCBC;
    ebx = (ebx & ~0xFF) | B(esi);
    if ((ebx & 0xFF) != B(edi + 0x9AB892)) goto L_0057FCBC;
    ebx = local_30;
    if (ebx != 0) goto L_0057FA95;
    if ((edx & 0xFF & 4) != 0) goto L_0057FCBC;
L_0057FA95:
    edx = (edx & ~0xFF) | B(esi - 3);
    ebx = (ebx & ~0xFF) | B(edi + 0x9AB88F);
    if ((unsigned char)(edx & 0xFF) < (unsigned char)(ebx & 0xFF)) goto L_0057FC95;
    edx = (edx & ~0xFF) | B(esi - 2);
    ebx = (ebx & ~0xFF) | B(edi + 0x9AB890);
    if ((unsigned char)(edx & 0xFF) > (unsigned char)(ebx & 0xFF)) goto L_0057FCBC;
    eax = (eax & ~0xFF) | B(eax + 0x94A37A);
    edx = (edx & ~0xFF) | B(ecx + 0x94A37A);
    if ((eax & 0xFF) != (edx & 0xFF)) goto L_0057FCBC;
    eax = (eax & ~0xFF) | B(esi - 5);
    ecx = eax;
    ecx = ecx & 0xff;
    ecx = ecx << 4;
    ecx = (ecx & ~0xFF) | B(ecx + 0x94AE68);
    if ((signed char)(ecx & 0xFF) > 0) goto L_0057FAF1;
    if ((eax & 0xFF) != B(edi + 0x9AB88D)) goto L_0057FCBC;
    goto L_0057FB08;
L_0057FAF1:
    edx = 0;
    edx = (edx & ~0xFF) | B(edi + 0x9AB88D);
    edx = edx << 4;
    if ((signed char)(ecx & 0xFF) > SB(edx + 0x94AE68)) goto L_0057FCBC;
L_0057FB08:
    ecx = (ecx & ~0xFF) | B(esi - 4);
    eax = ecx;
    eax = eax & 0xff;
    edx = eax;
    edx = edx << 4;
    ebx = (ebx & ~0xFF) | B(edx + 0x94F280);
    if ((signed char)(ebx & 0xFF) > 0) goto L_0057FB35;
    if ((ecx & 0xFF) != B(edi + 0x9AB88E)) goto L_0057FCBC;
    ebx = a1;
    goto L_0057FC26;
L_0057FB35:
    ebx = a1;
    if (ebx < 0) goto L_0057FB99;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057FB52;
    if (eax <= 9) goto L_0057FB52;
    eax = 1;
    goto L_0057FBA3;
L_0057FB52:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057FB94;
    eax = ebx;
    _p1 = 0;
    eax = eax << 6;
    eax = eax + ebx;
    _p2 = 0;
    _p3 = ebx;
    ecx = ebx + eax * 2;
    edx = ebx + ecx * 8;
    eax = ebx + edx * 2;
    eax = D(eax * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057FBA3;
L_0057FB94:
    eax = (signed char)(eax & 0xFF);
    goto L_0057FBA3;
L_0057FB99:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057FBA3:
    ecx = a2;
    edi = eax + eax;
    eax = 0;
    eax = B(ecx + 0x9AB88E);
    if (ebx < 0) goto L_0057FC12;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057FBCB;
    if (eax <= 9) goto L_0057FBCB;
    eax = 1;
    goto L_0057FC1C;
L_0057FBCB:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057FC0D;
    edx = ebx;
    _p1 = 0;
    edx = edx << 6;
    edx = edx + ebx;
    _p2 = 0;
    _p3 = ebx;
    eax = ebx + edx * 2;
    ecx = ebx + eax * 8;
    edx = ebx + ecx * 2;
    eax = D(edx * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057FC1C;
L_0057FC0D:
    eax = (signed char)(eax & 0xFF);
    goto L_0057FC1C;
L_0057FC12:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057FC1C:
    eax = eax + eax;
    if (edi > eax) goto L_0057FCB9;
L_0057FC26:
    ecx = a2;
    eax = 0;
    eax = B(ecx + 0x9AB88C);
    edx = eax + eax * 8;
    eax = 0;
    eax = B(esi - 6);
    edx = edx << 4;
    eax = eax + eax * 8;
    eax = eax << 4;
    ecx = (ecx & ~0xFF) | B(eax + 0x94A378);
    eax = (eax & ~0xFF) | B(edx + 0x94A378);
    if ((unsigned char)(ecx & 0xFF) > (unsigned char)(eax & 0xFF)) goto L_0057FCB9;
    if ((B(esi + 6) & 4) == 0) goto L_0057FC7E;
    edx = a2;
    edi = D(esi - 0xA);
    eax = D(edx + 0x9AB888);
    eax = eax & edi;
    _p1 = eax;
    eax = bit_count(_p1);
    ecx = D(esi - 0xA);
    edi = eax;
    _p2 = ecx;
    eax = bit_count(_p2);
    if (edi < eax) goto L_0057FCB9;
L_0057FC7E:
    edx = a2;
    eax = D(edx + 0x9AB888);
    edx = D(esi - 0xA);
    eax = eax ^ edx;
    if ((eax & 0x408418) != 0) goto L_0057FCB9;
    goto L_0057FC98;
L_0057FC95:
    ebx = a1;
L_0057FC98:
    eax = a3;
    edx = (edx & ~0xFF) | 1;
    ecx = ebx;
    edx = (edx & ~0xFF) | (unsigned char)(1 << (ecx & 0xFF));
    ecx = (ecx & ~0xFF) | B(esi + 2);
    ecx = (ecx & ~0xFF) | ((ecx & 0xFF) | (edx & 0xFF));
    B(esi + 2) = (unsigned char)(ecx & 0xFF);
    if (eax == 0) goto L_0057FCB9;
    eax = local_14;
    _p1 = eax;
    synch_obs(_p1);
L_0057FCB9:
    edi = a2;
L_0057FCBC:
    ecx = local_14;
    eax = local_2c;
    esi = esi + 0x34;
    ecx = ecx + 1;
    eax = eax - 1;
    local_14 = ecx;
    local_2c = eax;
    if (eax != 0) goto L_0057FA20;
    ebx = local_24;
    esi = local_20;
L_0057FCD9:
    ecx = local_18;
    eax = local_1c;
    edi = edi + 0x34;
    ecx = ecx + 1;
    eax = eax - 1;
    a2 = edi;
    local_18 = ecx;
    local_1c = eax;
    if (eax != 0) goto L_0057F6DA;
    return;
L_0057FCFA:
    eax = D(0x93F660);
    if (eax != 0) goto L_0057FD13;
    if ((D(0x9A6494) & 0x100000) == 0) goto L_00580852;
L_0057FD13:
    eax = 0;
    local_24 = eax;
    goto L_0057FD20;
L_0057FD1A:
    ecx = a1;
    eax = local_24;
L_0057FD20:
    esi = eax;
    esi = esi & 0x8000003f;
    if (esi >= 0) goto L_0057FD2F;
    esi = esi - 1;
    esi = esi | 0xffffffc0;
    esi = esi + 1;
L_0057FD2F:
    local_14 = esi;
    if (eax < 0x40) goto L_0057FD41;
    ecx = ecx << 6;
    esi = esi + ecx;
    local_14 = esi;
    goto L_0057FD61;
L_0057FD41:
    _p1 = ecx;
    ecx = esi + esi * 2;
    edx = esi + ecx * 4;
    eax = SW(edx * 4 + 0x9AB89A);
    _p2 = eax;
    eax = has_tech(_p2, _p1);
    if (eax == 0) goto L_00580840;
L_0057FD61:
    ecx = esi + esi * 2;
    edx = (edx & ~0xFF) | (local_28 & 0xFF);
    eax = esi + ecx * 4;
    eax = eax << 2;
    local_c = eax;
    if ((B(eax + 0x9AB894) & (edx & 0xFF)) != 0) goto L_00580840;
    if ((B(eax + 0x9AB898) & 1) == 0) goto L_00580840;
    local_18 = 0;
L_0057FD90:
    eax = local_18;
    ecx = eax;
    ecx = ecx & 0x8000003f;
    if (ecx >= 0) goto L_0057FDA2;
    ecx = ecx - 1;
    ecx = ecx | 0xffffffc0;
    ecx = ecx + 1;
L_0057FDA2:
    ebx = a1;
    local_8 = ecx;
    if (eax < 0x40) goto L_0057FDB7;
    edx = ebx;
    edx = edx << 6;
    ecx = ecx + edx;
    local_8 = ecx;
L_0057FDB7:
    if (eax == local_24) goto L_0058082E;
    eax = ecx + ecx * 2;
    edi = ecx + eax * 4;
    edi = edi << 2;
    local_10 = edi;
    eax = (eax & ~0xFFFF) | W(edi + 0x9AB898);
    if ((eax & 0xFF & 1) == 0) goto L_0058082E;
    edx = (edx & ~0xFF) | (local_28 & 0xFF);
    if ((B(edi + 0x9AB894) & (edx & 0xFF)) != 0) goto L_0058082E;
    if (ecx >= 0x40) goto L_0057FE10;
    edx = 0;
    edx = (edx & ~0xFF) | B(edi + 0x9AB88D);
    edx = edx << 4;
    if (SB(edx + 0x94AE68) < 0) goto L_0058082E;
    if (ecx == 0xF) goto L_0058082E;
L_0057FE10:
    edx = eax;
    edx = edx & 0x200;
    if (edx != 0) goto L_0058082E;
    esi = local_14;
    edx = eax;
    edx = edx & 8;
    edx = edx >> 3;
    cond_fe2c = (esi < 0x40);
    esi = local_c;
    local_30 = edx;
    if (cond_fe2c) goto L_0057FE51;
    if ((B(esi + 0x9AB898) & 4) != 0) goto L_0057FE51;
    if (ecx < 0x40) goto L_0058082E;
    if ((eax & 0xFF & 4) != 0) goto L_0058082E;
L_0057FE51:
    if (edx == 0) goto L_0057FE69;
    eax = (eax & ~0xFF) | B(edi + 0x9AB88F);
    ecx = (ecx & ~0xFF) | B(esi + 0x9AB88F);
    if ((unsigned char)(eax & 0xFF) >= (unsigned char)(ecx & 0xFF)) goto L_0058082E;
L_0057FE69:
    eax = (eax & ~0xFF) | B(edi + 0x9AB88D);
    ecx = eax;
    ecx = ecx & 0xff;
    ecx = ecx << 4;
    ecx = (ecx & ~0xFF) | B(ecx + 0x94AE68);
    if ((signed char)(ecx & 0xFF) > 0) goto L_0057FE92;
    if ((eax & 0xFF) != B(esi + 0x9AB88D)) goto L_0058082E;
    goto L_0057FEA9;
L_0057FE92:
    edx = 0;
    edx = (edx & ~0xFF) | B(esi + 0x9AB88D);
    edx = edx << 4;
    if ((signed char)(ecx & 0xFF) > SB(edx + 0x94AE68)) goto L_0058082E;
L_0057FEA9:
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB88E);
    eax = ecx;
    eax = eax & 0xff;
    edx = eax;
    edx = edx << 4;
    if (SB(edx + 0x94F280) > 0) goto L_0057FED5;
    if ((ecx & 0xFF) != B(esi + 0x9AB88E)) goto L_0058082E;
    goto L_0057FFC6;
L_0057FED5:
    if (ebx < 0) goto L_0057FF36;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057FEEF;
    if (eax <= 9) goto L_0057FEEF;
    eax = 1;
    goto L_0057FF40;
L_0057FEEF:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057FF31;
    eax = ebx;
    _p1 = 0;
    eax = eax << 6;
    eax = eax + ebx;
    _p2 = 0;
    _p3 = ebx;
    ecx = ebx + eax * 2;
    edx = ebx + ecx * 8;
    eax = ebx + edx * 2;
    eax = D(eax * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057FF40;
L_0057FF31:
    eax = (signed char)(eax & 0xFF);
    goto L_0057FF40;
L_0057FF36:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057FF40:
    ecx = local_c;
    esi = eax + eax;
    eax = 0;
    eax = B(ecx + 0x9AB88E);
    if (ebx < 0) goto L_0057FFAF;
    ecx = D(0x9A6488);
    if (ecx != 0) goto L_0057FF68;
    if (eax <= 9) goto L_0057FF68;
    eax = 1;
    goto L_0057FFB9;
L_0057FF68:
    eax = eax << 4;
    eax = (eax & ~0xFF) | B(eax + 0x94F280);
    if ((signed char)(eax & 0xFF) >= 0) goto L_0057FFAA;
    edx = ebx;
    _p1 = 0;
    edx = edx << 6;
    edx = edx + ebx;
    _p2 = 0;
    _p3 = ebx;
    eax = ebx + edx * 2;
    ecx = ebx + eax * 8;
    edx = ebx + ecx * 2;
    eax = D(edx * 4 + 0x96D20C);
    eax = eax * D(0x9497A8);
    eax = eax / D(0x94979C);
    _p4 = eax;
    eax = psi_factor(_p4, _p3, _p2, _p1);
    goto L_0057FFB9;
L_0057FFAA:
    eax = (signed char)(eax & 0xFF);
    goto L_0057FFB9;
L_0057FFAF:
    eax = eax << 4;
    eax = SB(eax + 0x94F280);
L_0057FFB9:
    eax = eax + eax;
    if (esi > eax) goto L_0058082E;
    esi = local_c;
L_0057FFC6:
    if (B(edi + 0x9AB892) == 3) goto L_0058003B;
    ecx = 0;
    edx = 0;
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB88D);
    ecx = ecx << 4;
    edx = (SB(ecx + 0x94AE68) > 1) ? 1 : 0;
    eax = 0;
    ecx = 0;
    eax = B(esi + 0x9AB88D);
    eax = eax << 4;
    ecx = (SB(eax + 0x94AE68) > 1) ? 1 : 0;
    if (edx != ecx) goto L_0058082E;
    edx = 0;
    eax = 0;
    edx = B(edi + 0x9AB88E);
    edx = edx << 4;
    eax = (SB(edx + 0x94F280) > 1) ? 1 : 0;
    ecx = 0;
    edx = 0;
    ecx = B(esi + 0x9AB88E);
    ecx = ecx << 4;
    edx = (SB(ecx + 0x94F280) > 1) ? 1 : 0;
    if (eax != edx) goto L_0058082E;
L_0058003B:
    eax = (eax & ~0xFF) | B(edi + 0x9AB88F);
    ecx = (ecx & ~0xFF) | B(esi + 0x9AB88F);
    if ((unsigned char)(eax & 0xFF) > (unsigned char)(ecx & 0xFF)) goto L_0058082E;
    eax = (eax & ~0xFF) | B(edi + 0x9AB890);
    ecx = (ecx & ~0xFF) | B(esi + 0x9AB890);
    if ((unsigned char)(eax & 0xFF) > (unsigned char)(ecx & 0xFF)) goto L_0058082E;
    ecx = a2;
    eax = (eax & ~0xFF) | B(edi + 0x9AB88C);
    eax = eax & 0xff;
    edx = ecx + ecx * 2;
    eax = eax + eax * 8;
    ecx = ecx + edx * 4;
    edx = 0;
    eax = eax << 4;
    edx = (edx & ~0xFF) | B(ecx * 4 + 0x9AB88C);
    ecx = edx;
    edx = (edx & ~0xFF) | B(eax + 0x94A37A);
    ecx = ecx + ecx * 8;
    ecx = ecx << 4;
    if ((edx & 0xFF) != B(ecx + 0x94A37A)) goto L_0058082E;
    ecx = local_c;
    ebx = (ebx & ~0xFF) | B(eax + 0x94A379);
    edx = (edx & ~0xFF) | B(ecx + 0x9AB88C);
    ecx = edx;
    ecx = ecx & 0xff;
    ecx = ecx + ecx * 8;
    ecx = ecx << 4;
    if ((ebx & 0xFF) != B(ecx + 0x94A379)) goto L_0058082E;
    eax = (eax & ~0xFF) | B(eax + 0x94A378);
    ebx = (ebx & ~0xFF) | B(ecx + 0x94A378);
    if ((unsigned char)(eax & 0xFF) > (unsigned char)(ebx & 0xFF)) goto L_0058082E;
    edi = local_10;
    ebx = local_c;
    ecx = D(edi + 0x9AB888);
    esi = D(ebx + 0x9AB888);
    if (ecx == esi) goto L_005801DB;
    eax = esi;
    eax = eax ^ ecx;
    if ((eax & 0xfffbd5fc) != 0) goto L_0058082E;
    eax = (eax & ~0xFF) | B(edi + 0x9AB88F);
    if ((unsigned char)(eax & 0xFF) < B(ebx + 0x9AB88F)) goto L_005801DB;
    eax = esi;
    eax = eax & ecx;
    if (eax == ecx) goto L_005801DB;
    if (local_14 < 0x40) goto L_0058082E;
    eax = a1;
    edi = eax;
    edi = edi << 6;
    edi = edi + eax;
    edi = eax + edi * 2;
    edi = eax + edi * 8;
    eax = eax + edi * 2;
    edi = local_8;
    eax = eax << 2;
    ebx = (ebx & ~0xFF) | B(eax + edi + 0x96D238);
    if ((ebx & 0xFF) != 0) goto L_0058082E;
    ebx = (ebx & ~0xFF) | B(eax + edi + 0x96D438);
    if ((ebx & 0xFF) != 0) goto L_0058082E;
    ebx = local_c;
L_0058016B:
    edi = local_10;
    eax = (eax & ~0xFF) | B(edi + 0x9AB892);
    if ((eax & 0xFF) != B(ebx + 0x9AB892)) goto L_0058082E;
    eax = local_30;
    if (eax == 0) goto L_005801A0;
    eax = (eax & ~0xFF) | B(edi + 0x9AB893);
    if ((eax & 0xFF) != B(ebx + 0x9AB893)) goto L_0058082E;
    esi = esi & ecx;
    if (esi != ecx) goto L_0058082E;
L_005801A0:
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB891);
    eax = (eax & ~0xFF) | B(ebx + 0x9AB891);
    if ((unsigned char)(ecx & 0xFF) >= (unsigned char)(eax & 0xFF)) goto L_005801E3;
    eax = (eax & ~0xFF) | B(edi + 0x9AB88F);
    ecx = (ecx & ~0xFF) | B(ebx + 0x9AB88F);
    if ((unsigned char)(eax & 0xFF) < (unsigned char)(ecx & 0xFF)) goto L_005801E3;
    esi = local_8;
    if (esi < 0x40) goto L_0058082E;
    if ((B(edi + 0x9AB898) & 8) != 0) goto L_0058082E;
    goto L_005801E6;
L_005801DB:
    edi = local_10;
    ebx = local_c;
    goto L_0058016B;
L_005801E3:
    esi = local_8;
L_005801E6:
    eax = (eax & ~0xFF) | B(edi + 0x9AB88D);
    ecx = (ecx & ~0xFF) | B(ebx + 0x9AB88D);
    if ((eax & 0xFF) != (ecx & 0xFF)) goto L_00580231;
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB88E);
    eax = (eax & ~0xFF) | B(ebx + 0x9AB88E);
    if ((ecx & 0xFF) != (eax & 0xFF)) goto L_00580231;
    if (B(edi + 0x9AB88C) != (edx & 0xFF)) goto L_00580231;
    eax = (eax & ~0xFF) | B(edi + 0x9AB88F);
    ecx = (ecx & ~0xFF) | B(ebx + 0x9AB88F);
    if ((eax & 0xFF) != (ecx & 0xFF)) goto L_00580231;
    if (esi < 0x40) goto L_0058082E;
    if (local_14 < 0x40) goto L_0058082E;
L_00580231:
    if (esi != a2) goto L_00580256;
    eax = a3;
    B(edi + 0x9AB898) |= 8;
    if (eax == 0) goto L_0058082E;
    _p1 = esi;
    synch_obs(_p1);
    goto L_0058082E;
L_00580256:
    ecx = a1;
    eax = (eax & ~0xFF) | 1;
    eax = (eax & ~0xFF) | (unsigned char)(1 << (ecx & 0xFF));
    ecx = (ecx & ~0xFF) | B(edi + 0x9AB894);
    ebx = 0;
    local_1 = (unsigned char)(eax & 0xFF);
    ecx = (ecx & ~0xFF) | ((ecx & 0xFF) | (eax & 0xFF));
    eax = a3;
    B(edi + 0x9AB894) = (unsigned char)(ecx & 0xFF);
    if (eax == ebx) goto L_00580280;
    _p1 = esi;
    synch_obs(_p1);
L_00580280:
    eax = local_8;
    local_20 = ebx;
    local_30 = ebx;
    if (eax >= 0x40) goto L_005802AA;
    eax = SW(edi + 0x9AB89A);
    edx = a1;
    _p1 = edx;
    _p2 = eax;
    eax = has_tech(_p2, _p1);
    if (eax == 0) goto L_0058082E;
L_005802AA:
    eax = D(0x9A64CC);
    edi = 0;
    local_2c = edi;
    if (eax <= 0) goto L_00580302;
    esi = 0x97D08C;
L_005802BD:
    eax = a1;
    ecx = esi + 4;
    edx = 0;
    edx = (edx & ~0xFF) | B(ecx - 0x4C);
    if (edx != eax) goto L_005802E6;
    edx = D(esi);
    eax = 0;
    if (edx < 0) goto L_005802E6;
    ebx = local_8;
L_005802D7:
    if (D(ecx) == ebx) goto L_005802F8;
    eax = eax + 1;
    ecx = ecx + 4;
    if (eax <= edx) goto L_005802D7;
L_005802E6:
    eax = D(0x9A64CC);
    edi = edi + 1;
    esi = esi + 0x134;
    if (edi >= eax) goto L_00580302;
    goto L_005802BD;
L_005802F8:
    ebx = local_20;
    local_2c = 1;
L_00580302:
    eax = D(0x9A64CC);
    local_1c = 0;
    if (eax <= 0) goto L_0058082E;
    local_20 = 0x97D08C;
L_0058031D:
    ecx = local_20;
    eax = 0;
    eax = B(ecx - 0x48);
    ecx = a1;
    if (eax != ecx) goto L_005805DD;
    eax = D(0x93F660);
    if (eax != 0) goto L_00580354;
    edx = D(0x939284);
    eax = ecx;
    if (eax != edx) goto L_00580354;
    eax = local_2c;
    if (eax == 0) goto L_005805DD;
    if (ebx == 0) goto L_0058038F;
L_00580354:
    eax = local_2c;
    if (eax == 0) goto L_005805DD;
    eax = a3;
    if (eax == 0) goto L_00580382;
    eax = local_1c;
    _p1 = -1;
    _p2 = -1;
    _p3 = 0;
    _p4 = eax;
    ecx = 0x93CD90;
    eax = ((NetDaemon *)ecx)->lock_base(_p4, _p3, _p2, _p1);
    if (eax != 0) goto L_005805DD;
L_00580382:
    eax = D(0x93F660);
    if (eax != 0) goto L_00580599;
L_0058038F:
    ecx = D(0x939284);
    eax = a1;
    if (eax != ecx) goto L_00580599;
    if (ebx != 0) goto L_00580599;
    B(0x9B86A0) = (unsigned char)(ebx & 0xFF);
    ebx = local_10;
    edx = ebx + 0x9AB868;
    _p1 = edx;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682E9C;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    eax = local_8;
    _p1 = eax;
    _p2 = 0x9B86A0;
    say_stats_2((char *)_p2, _p1);
    eax = D(ebx + 0x9AB888);
    if (eax == 0) goto L_0058044E;
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    edi = 0;
    esi = 0x9AB540;
L_00580414:
    eax = D(ebx + 0x9AB888);
    edx = 1;
    ecx = edi;
    edx = 1 << (ecx & 0xFF);
    if ((eax & edx) == 0) goto L_00580442;
    eax = D(esi);
    ecx = 0x9B90D8;
    _p1 = eax;
    eax = ((Strings *)ecx)->get(_p1);
    _p1 = eax;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
L_00580442:
    esi = esi + 0x1C;
    edi = edi + 1;
    if (esi < 0x9AB86C) goto L_00580414;
L_0058044E:
    _p1 = 0x682E98;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = -1;
    _p2 = -1;
    _p3 = 0x9B86A0;
    _p4 = 0;
    eax = parse_says(_p4, (char *)_p3, _p2, _p1);
    ebx = local_c;
    B(0x9B86A0) = 0;
    eax = ebx + 0x9AB868;
    _p1 = eax;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682E9C;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    ecx = local_14;
    _p1 = ecx;
    _p2 = 0x9B86A0;
    say_stats_2((char *)_p2, _p1);
    eax = D(ebx + 0x9AB888);
    if (eax == 0) goto L_00580514;
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    edi = 0;
    esi = 0x9AB540;
L_005804DA:
    eax = D(ebx + 0x9AB888);
    edx = 1;
    ecx = edi;
    edx = 1 << (ecx & 0xFF);
    if ((eax & edx) == 0) goto L_00580508;
    eax = D(esi);
    ecx = 0x9B90D8;
    _p1 = eax;
    eax = ((Strings *)ecx)->get(_p1);
    _p1 = eax;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
L_00580508:
    esi = esi + 0x1C;
    edi = edi + 1;
    if (esi < 0x9AB86C) goto L_005804DA;
L_00580514:
    _p1 = 0x682E98;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = -1;
    _p2 = -1;
    _p3 = 0x9B86A0;
    _p4 = 1;
    eax = parse_says(_p4, (char *)_p3, _p2, _p1);
    esi = local_10;
    eax = (eax & ~0xFFFF) | W(esi + 0x9AB898);
    eax = eax & 0x200;
    if (eax != 0) goto L_0058056E;
    eax = local_30;
    if (eax != 0) goto L_0058056E;
    _p1 = 0;
    _p2 = 0x68F164;
    eax = X_pop((const char *)_p2, (int (__cdecl *)())_p1);
    if (eax <= 1) goto L_0058057A;
    local_30 = 1;
    goto L_00580599;
L_0058056E:
    ecx = local_30;
    eax = 1;
    if (ecx != 0) goto L_00580599;
L_0058057A:
    ecx = (ecx & ~0xFFFF) | W(esi + 0x9AB898);
    ecx = ecx & 0x200;
    if (ecx != 0) goto L_005807E2;
    if (eax == 1) goto L_005807E2;
L_00580599:
    edx = local_20;
    ecx = 0;
    ebx = 1;
    if (D(edx) < 0) goto L_005805BF;
    esi = local_8;
    edi = local_14;
    eax = edx + 4;
L_005805B1:
    if (D(eax) != esi) goto L_005805B7;
    D(eax) = edi;
L_005805B7:
    ecx = ecx + 1;
    eax = eax + 4;
    if (ecx <= D(edx)) goto L_005805B1;
L_005805BF:
    eax = D(0x93F660);
    if (eax == 0) goto L_005805DD;
    eax = a3;
    if (eax == 0) goto L_005805DD;
    edx = local_1c;
    ecx = 0x93CD90;
    _p1 = edx;
    ((NetDaemon *)ecx)->unlock_base(_p1);
L_005805DD:
    eax = local_1c;
    edx = local_20;
    ecx = D(0x9A64CC);
    eax = eax + 1;
    edx = edx + 0x134;
    local_1c = eax;
    local_20 = edx;
    if (eax < ecx) goto L_0058031D;
    if (ebx == 0) goto L_0058082E;
    eax = D(0x93F660);
    if (eax == 0) goto L_0058082E;
    eax = D(0x939284);
    ecx = a1;
    if (ecx != eax) goto L_0058082E;
    esi = local_10;
    B(0x9B86A0) = 0;
    ecx = esi + 0x9AB868;
    _p1 = ecx;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682E9C;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    edx = local_8;
    _p1 = edx;
    _p2 = 0x9B86A0;
    say_stats_2((char *)_p2, _p1);
    eax = D(esi + 0x9AB888);
    if (eax == 0) goto L_005806CB;
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    edi = 0;
    esi = 0x9AB540;
L_00580690:
    eax = 1;
    ecx = edi;
    eax = 1 << (ecx & 0xFF);
    ecx = local_10;
    if ((D(ecx + 0x9AB888) & eax) == 0) goto L_005806BF;
    eax = D(esi);
    ecx = 0x9B90D8;
    _p1 = eax;
    eax = ((Strings *)ecx)->get(_p1);
    _p1 = eax;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
L_005806BF:
    esi = esi + 0x1C;
    edi = edi + 1;
    if (esi < 0x9AB86C) goto L_00580690;
L_005806CB:
    _p1 = 0x682E98;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = -1;
    _p2 = -1;
    _p3 = 0x9B86A0;
    _p4 = 0;
    eax = parse_says(_p4, (char *)_p3, _p2, _p1);
    esi = local_c;
    B(0x9B86A0) = 0;
    edx = esi + 0x9AB868;
    _p1 = edx;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = 0x682E9C;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    eax = local_14;
    _p1 = eax;
    _p2 = 0x9B86A0;
    say_stats_2((char *)_p2, _p1);
    eax = D(esi + 0x9AB888);
    if (eax == 0) goto L_00580792;
    _p1 = 0x682820;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    edi = 0;
    esi = 0x9AB540;
L_00580757:
    eax = local_c;
    edx = 1;
    ecx = edi;
    edx = 1 << (ecx & 0xFF);
    if ((D(eax + 0x9AB888) & edx) == 0) goto L_00580786;
    eax = D(esi);
    ecx = 0x9B90D8;
    _p1 = eax;
    eax = ((Strings *)ecx)->get(_p1);
    _p1 = eax;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
L_00580786:
    esi = esi + 0x1C;
    edi = edi + 1;
    if (esi < 0x9AB86C) goto L_00580757;
L_00580792:
    _p1 = 0x682E98;
    _p2 = 0x9B86A0;
    strcat((char *)_p2, (const char *)_p1);
    _p1 = -1;
    _p2 = -1;
    _p3 = 0x9B86A0;
    _p4 = 1;
    eax = parse_says(_p4, (char *)_p3, _p2, _p1);
    ecx = a1;
    eax = ecx + ecx * 4;
    _pp_cb = 0;
    edx = eax + eax * 8;
    edx = edx << 3;
    edx = edx - ecx;
    if ((B(edx * 4 + 0x946F58) & 0x80) == 0) goto L_00580813;
    eax = D(0x691B0C);
    _pp_c = 0x68F174;
    _pp_b = 0;
    _pp_a = 0x68F184;
    _pp_this = eax;
    goto L_00580826;
L_00580813:
    ecx = D(0x691B0C);
    _pp_c = 0x68F190;
    _pp_b = 0;
    _pp_a = 0x68F19C;
    _pp_this = ecx;
L_00580826:
    eax = popp((char *)_pp_this, (const char *)_pp_a, _pp_b, (const char *)_pp_c,
               (int (__cdecl *)())_pp_cb);
    goto L_0058082E;
L_005807E2:
    eax = (eax & ~0xFF) | local_1;
    edx = (edx & ~0xFF) | B(esi + 0x9AB894);
    eax = (eax & ~0xFF) | ((~eax) & 0xFF);
    edx = (edx & ~0xFF) | ((edx & 0xFF) & (eax & 0xFF));
    B(esi + 0x9AB894) = (unsigned char)(edx & 0xFF);
    eax = (eax & ~0xFFFF) | W(esi + 0x9AB898);
    if ((eax & 0xFF & 8) == 0) goto L_0058080A;
    eax = eax | 0x200;
    W(esi + 0x9AB898) = (unsigned short)(eax & 0xFFFF);
L_0058080A:
    B(esi + 0x9AB898) |= 8;
    goto L_0058082E;
L_0058082E:
    eax = local_18;
    eax = eax + 1;
    local_18 = eax;
    if (eax < 0x80) goto L_0057FD90;
L_00580840:
    eax = local_24;
    eax = eax + 1;
    local_24 = eax;
    if (eax < 0x80) goto L_0057FD1A;
L_00580852:
    return;
}

#undef B
#undef SB
#undef W
#undef SW
#undef D
