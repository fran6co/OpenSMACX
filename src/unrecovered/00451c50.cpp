// ORIGINAL: 0x00451C50 ?load_terrain@@YAXXZ 0x00451C50-0x0045370C;0x006550F9-0x0065513A FILE
// RULED-OUT: sim 0.94, 1704/1715 mnemonics (99.4%); first divergence is the missing `mov byte ptr[ebp-4],N` SEH local-unwind state byte the compiler writes between constructing locals and the first call - not reproduced since it falls out of the compiler's own bookkeeping for the `Sprite arr1[4]; Sprite arr2[4]; Buffer buf804;` RAII locals (matching the original's `??_L`/`??_M` array-constructor-iterator calls) rather than anything we write by hand. Every Texture::extract/Sprite::extract/ change_color loop (including the two 4x4 grids with the `ebx==3&&esi==3` skip, the register-relative `esi±0x2c` sprite-table walks, and the get_pixel/palette-remap blocks) is transcribed from the disassembly, not approximated.
// working copy - scaffold materialised by --work
// size      6909 bytes
// prototype 
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7470 0x005D7670 0x005D7DE0 0x005DE580 0x005DFB50 0x005E2210 0x005E39A0 0x005E9D44 0x00619710 0x006456E4 0x006457C2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_terrain@@YAXXZ  at 0x00451C50  (6909 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
struct ExtDirectDraw;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
class Palette;
struct RECT;
class Spot;
class Sprite;
class TexHeap;
class Texture;
class Time;
typedef unsigned int UINT_PTR;

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
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
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
    Buffer();
    HDC get_hdc();
    int change_color(int, int);
    int fill(int);
    int get_data();
    int get_pixel(int, int);
    int init(int, int, int, ExtDirectDraw *);
    int init_class();
    int load_pcx(const char *, Palette *, int, int);
    int text_height();
    int text_line_height();
    void clear_links();
    void close();
    void close_class();
    void construct();
    void destroy();
    ~Buffer();
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

class Palette { public:
    struct PaletteInternal {
        uint32_t field_0;
        Time * time;
        uint8_t field_8;
        uint8_t field_9;
        uint8_t field_A;
        uint8_t field_B;
        void * field_C;
    };
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    uint32_t field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t field_A8_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    uint32_t field_EC_;
    uint32_t field_F0_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    uint32_t field_114_;
    uint32_t field_118_;
    uint32_t field_11C_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    uint32_t field_13C_;
    uint32_t field_140_;
    uint32_t field_144_;
    uint32_t field_148_;
    uint32_t field_14C_;
    uint32_t field_150_;
    uint32_t field_154_;
    uint32_t field_158_;
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
    uint32_t field_188_;
    uint32_t field_18C_;
    uint32_t field_190_;
    uint32_t field_194_;
    uint32_t field_198_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    uint32_t field_1A4_;
    uint32_t field_1A8_;
    uint32_t field_1AC_;
    uint32_t field_1B0_;
    uint32_t field_1B4_;
    uint32_t field_1B8_;
    uint32_t field_1BC_;
    uint32_t field_1C0_;
    uint32_t field_1C4_;
    uint32_t field_1C8_;
    uint32_t field_1CC_;
    uint32_t field_1D0_;
    uint32_t field_1D4_;
    uint32_t field_1D8_;
    uint32_t field_1DC_;
    uint32_t field_1E0_;
    uint32_t field_1E4_;
    uint32_t field_1E8_;
    uint32_t field_1EC_;
    uint32_t field_1F0_;
    uint32_t field_1F4_;
    uint32_t field_1F8_;
    uint32_t field_1FC_;
    uint32_t field_200_;
    uint32_t field_204_;
    uint32_t field_208_;
    uint32_t field_20C_;
    uint32_t field_210_;
    uint32_t field_214_;
    uint32_t field_218_;
    uint32_t field_21C_;
    uint32_t field_220_;
    uint32_t field_224_;
    uint32_t field_228_;
    uint32_t field_22C_;
    uint32_t field_230_;
    uint32_t field_234_;
    uint32_t field_238_;
    uint32_t field_23C_;
    uint32_t field_240_;
    uint32_t field_244_;
    uint32_t field_248_;
    uint32_t field_24C_;
    uint32_t field_250_;
    uint32_t field_254_;
    uint32_t field_258_;
    uint32_t field_25C_;
    uint32_t field_260_;
    uint32_t field_264_;
    uint32_t field_268_;
    uint32_t field_26C_;
    uint32_t field_270_;
    uint32_t field_274_;
    uint32_t field_278_;
    uint32_t field_27C_;
    uint32_t field_280_;
    uint32_t field_284_;
    uint32_t field_288_;
    uint32_t field_28C_;
    uint32_t field_290_;
    uint32_t field_294_;
    uint32_t field_298_;
    uint32_t field_29C_;
    uint32_t field_2A0_;
    uint32_t field_2A4_;
    uint32_t field_2A8_;
    uint32_t field_2AC_;
    uint32_t field_2B0_;
    uint32_t field_2B4_;
    uint32_t field_2B8_;
    uint32_t field_2BC_;
    uint32_t field_2C0_;
    uint32_t field_2C4_;
    uint32_t field_2C8_;
    uint32_t field_2CC_;
    uint32_t field_2D0_;
    uint32_t field_2D4_;
    uint32_t field_2D8_;
    uint32_t field_2DC_;
    uint32_t field_2E0_;
    uint32_t field_2E4_;
    uint32_t field_2E8_;
    uint32_t field_2EC_;
    uint32_t field_2F0_;
    uint32_t field_2F4_;
    uint32_t field_2F8_;
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint32_t field_304_;
    uint32_t field_308_;
    uint32_t field_30C_;
    uint32_t field_310_;
    uint32_t field_314_;
    uint32_t field_318_;
    uint32_t field_31C_;
    uint32_t field_320_;
    uint32_t field_324_;
    uint32_t field_328_;
    uint32_t field_32C_;
    uint32_t field_330_;
    uint32_t field_334_;
    uint32_t field_338_;
    uint32_t field_33C_;
    uint32_t field_340_;
    uint32_t field_344_;
    uint32_t field_348_;
    uint32_t field_34C_;
    uint32_t field_350_;
    uint32_t field_354_;
    uint32_t field_358_;
    uint32_t field_35C_;
    uint32_t field_360_;
    uint32_t field_364_;
    uint32_t field_368_;
    uint32_t field_36C_;
    uint32_t field_370_;
    uint32_t field_374_;
    uint32_t field_378_;
    uint32_t field_37C_;
    uint32_t field_380_;
    uint32_t field_384_;
    uint32_t field_388_;
    uint32_t field_38C_;
    uint32_t field_390_;
    uint32_t field_394_;
    uint32_t field_398_;
    uint32_t field_39C_;
    uint32_t field_3A0_;
    uint32_t field_3A4_;
    uint32_t field_3A8_;
    uint32_t field_3AC_;
    uint32_t field_3B0_;
    uint32_t field_3B4_;
    uint32_t field_3B8_;
    uint32_t field_3BC_;
    uint32_t field_3C0_;
    uint32_t field_3C4_;
    uint32_t field_3C8_;
    uint32_t field_3CC_;
    uint32_t field_3D0_;
    uint32_t field_3D4_;
    uint32_t field_3D8_;
    uint32_t field_3DC_;
    uint32_t field_3E0_;
    uint32_t field_3E4_;
    uint32_t field_3E8_;
    uint32_t field_3EC_;
    uint32_t field_3F0_;
    uint32_t field_3F4_;
    uint32_t field_3F8_;
    uint32_t field_3FC_;
    uint32_t seed_;
    PaletteInternal internal_[5];
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
    Sprite();
    ~Sprite();
    int draw(Buffer *, int, int, int, unsigned char *);
    int extract(Buffer *, int, int, int, int, int, TexHeap *);
    void close();
};

class Texture { public:
    void * pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    uint8_t unmapped_[0x60];
    uint32_t borrowed_;
    int extract(Buffer *, int, int, int, int, TexHeap *);
    void close();
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

void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));
void __stdcall fn_006457c2(void*, unsigned int, int, void (*)(void*), void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_005e37e0 = (int *)0x005E37E0;
static int *const g_00655130 = (int *)0x00655130;
static int *const g_00675368 = (int *)0x00675368;
static int *const g_006846f8 = (int *)0x006846F8;
static int *const g_00684fbc = (int *)0x00684FBC;
static int *const g_00684fc8 = (int *)0x00684FC8;
static int *const g_00684fd8 = (int *)0x00684FD8;
static int *const g_00684fe4 = (int *)0x00684FE4;
static int *const g_00684ff4 = (int *)0x00684FF4;
static int *const g_0075b098 = (int *)0x0075B098;
static int *const g_0075b2b4 = (int *)0x0075B2B4;
static int *const g_0075b4c4 = (int *)0x0075B4C4;
static int *const g_0075b858 = (int *)0x0075B858;
static int *const g_0076e8a0 = (int *)0x0076E8A0;
static int *const g_0076e910 = (int *)0x0076E910;
static int *const g_0076e9e0 = (int *)0x0076E9E0;
static int *const g_0076eac0 = (int *)0x0076EAC0;
static int *const g_00776758 = (int *)0x00776758;
static int *const g_00776784 = (int *)0x00776784;
static int *const g_007767b0 = (int *)0x007767B0;
static int *const g_007767dc = (int *)0x007767DC;
static int *const g_00776808 = (int *)0x00776808;
static int *const g_00776834 = (int *)0x00776834;
static int *const g_00776860 = (int *)0x00776860;
static int *const g_0077688c = (int *)0x0077688C;
static int *const g_007768b8 = (int *)0x007768B8;
static int *const g_007768e4 = (int *)0x007768E4;
static int *const g_00776910 = (int *)0x00776910;
static int *const g_0077693c = (int *)0x0077693C;
static int *const g_00776968 = (int *)0x00776968;
static int *const g_00776994 = (int *)0x00776994;
static int *const g_007769c0 = (int *)0x007769C0;
static int *const g_00776a50 = (int *)0x00776A50;
static int *const g_00776a80 = (int *)0x00776A80;
static int *const g_00777110 = (int *)0x00777110;
static int *const g_007792b0 = (int *)0x007792B0;
static int *const g_00779390 = (int *)0x00779390;
static int *const g_0077944c = (int *)0x0077944C;
static int *const g_00779770 = (int *)0x00779770;
static int *const g_0077979c = (int *)0x0077979C;
static int *const g_007797f8 = (int *)0x007797F8;
static int *const g_00779b78 = (int *)0x00779B78;
static int *const g_0077aff8 = (int *)0x0077AFF8;
static int *const g_00787e54 = (int *)0x00787E54;
static int *const g_00787fb8 = (int *)0x00787FB8;
static int *const g_00788100 = (int *)0x00788100;
static int *const g_00788250 = (int *)0x00788250;
static int *const g_00789b68 = (int *)0x00789B68;
static int *const g_00789c28 = (int *)0x00789C28;
static int *const g_0078a340 = (int *)0x0078A340;
static int *const g_0078a490 = (int *)0x0078A490;
static int *const g_0078a5b0 = (int *)0x0078A5B0;
static int *const g_0078a700 = (int *)0x0078A700;
static int *const g_0078a72c = (int *)0x0078A72C;
static int *const g_0078a758 = (int *)0x0078A758;
static int *const g_0078ae58 = (int *)0x0078AE58;
static int *const g_0078dd20 = (int *)0x0078DD20;
static int *const g_0078dd80 = (int *)0x0078DD80;
static int *const g_00791c84 = (int *)0x00791C84;
static int *const g_00791d8c = (int *)0x00791D8C;
static int *const g_00791fc8 = (int *)0x00791FC8;
static int *const g_00792218 = (int *)0x00792218;
static int *const g_00792608 = (int *)0x00792608;
static int *const g_00798668 = (int *)0x00798668;
static int *const g_00798c40 = (int *)0x00798C40;
static int *const g_00798e08 = (int *)0x00798E08;
static int *const g_007991f8 = (int *)0x007991F8;
static int *const g_00799238 = (int *)0x00799238;
static int *const g_00799628 = (int *)0x00799628;
static int *const g_00799658 = (int *)0x00799658;
static int *const g_00799684 = (int *)0x00799684;
static int *const g_007996b0 = (int *)0x007996B0;
static int *const g_007996dc = (int *)0x007996DC;
static int *const g_00799738 = (int *)0x00799738;
static int *const g_00799e38 = (int *)0x00799E38;
static int *const g_00799e48 = (int *)0x00799E48;
static int *const g_00799eb8 = (int *)0x00799EB8;
static int *const g_0079a5b8 = (int *)0x0079A5B8;
static int *const g_0079a710 = (int *)0x0079A710;
static int *const g_007a6830 = (int *)0x007A6830;
static int *const g_007a6860 = (int *)0x007A6860;
static int *const g_007a7820 = (int *)0x007A7820;
static int *const g_007a7900 = (int *)0x007A7900;
static int *const g_007a7aa0 = (int *)0x007A7AA0;
static int *const g_007a81a0 = (int *)0x007A81A0;
static int *const g_007a99cc = (int *)0x007A99CC;
static int *const g_007a9b58 = (int *)0x007A9B58;
static int *const g_007abf60 = (int *)0x007ABF60;
static int *const g_007ac098 = (int *)0x007AC098;
static int *const g_007ac0c4 = (int *)0x007AC0C4;
static int *const g_007ac0f0 = (int *)0x007AC0F0;
static int *const g_007ac11c = (int *)0x007AC11C;
static int *const g_007ac148 = (int *)0x007AC148;
static int *const g_007ac174 = (int *)0x007AC174;
static int *const g_007ac220 = (int *)0x007AC220;
static int *const g_007acb88 = (int *)0x007ACB88;
static int *const g_007acbb8 = (int *)0x007ACBB8;
static int *const g_007acc60 = (int *)0x007ACC60;
static int *const g_007acd70 = (int *)0x007ACD70;
static int *const g_007acd9c = (int *)0x007ACD9C;
static int *const g_007acdc8 = (int *)0x007ACDC8;
static int *const g_007acdf4 = (int *)0x007ACDF4;
static int *const g_007ace20 = (int *)0x007ACE20;
static int *const g_007ace4c = (int *)0x007ACE4C;
static int *const g_007ad010 = (int *)0x007AD010;
static int *const g_009bb548 = (int *)0x009BB548;
void __cdecl load_terrain() {
    Sprite arr1[4];
    Sprite arr2[4];
    Buffer buf804;

    ((Buffer *)g_00798668)->load_pcx((char *)g_00684fbc, (Palette *)0, 0xa, 0xec);
    *(int *)g_009bb548 = 9;
    buf804.init(0x1c, 0x1c, 0, (ExtDirectDraw *)0);

    ((Texture *)g_00787fb8)->extract((Buffer *)g_00798668, 1, 0x23b, 0x38, 0x38, (TexHeap *)0);  // 0x00451D0B
    ((Texture *)g_0075b858)->extract((Buffer *)g_00798668, 0x3a, 0x23b, 0x38, 0x38, (TexHeap *)0);  // 0x00451D27

    {
        unsigned char *texBase = (unsigned char *)0x76e9e0;
        int scaleVal = 1;
        do {
            unsigned char *tex = texBase;
            for (int k = 0; k < 2; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, 1, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0xe0;
            }
            texBase += 0x70;
        } while (texBase < (unsigned char *)0x76eac0);
    }

    {
        int scaleVal = 0x4f;
        unsigned char *tex = (unsigned char *)0x7a7820;
        while (tex < (unsigned char *)0x7a7900) {
            ((Texture *)tex)->extract((Buffer *)g_00798668, 0x118, scaleVal, 0x38, 0x38, (TexHeap *)0);
            tex += 0x70;
            scaleVal += 0x39;
        }
    }
    ((Texture *)g_00799e48)->extract((Buffer *)g_00798668, 1, 0x3a, 0x38, 0x38, (TexHeap *)0);  // 0x00451DB6

    {
        int rowVal = 0x73;
        unsigned char *tex = (unsigned char *)0x799eb8;
        while (tex < (unsigned char *)0x79a5b8) {
            int scaleVal = 1;
            for (int k = 0; k < 4; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x157;
        unsigned char *tex = (unsigned char *)0x799738;
        while (tex < (unsigned char *)0x799e38) {
            int scaleVal = 1;
            for (int k = 0; k < 4; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x103;
        for (int row = 0; row < 4; row++) {
            int scaleVal = 0x20e;
            for (int col = 0; col < 4; col++) {
                if (row == 3 && col == row) continue;
                int a = col + row * 4;
                unsigned char *tex = (unsigned char *)0x789c28 + (a * 7) * 16;
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
            }
            rowVal += 0x39;
        }
    }
    ((Texture *)g_007ac220)->extract((Buffer *)g_00798668, 0xe5, 1, 0x38, 0x38, (TexHeap *)0);  // 0x00451EBF

    {
        int rowVal = 0xf;
        unsigned char *tex = (unsigned char *)0x7797f8;
        while (tex < (unsigned char *)0x779b78) {
            int scaleVal = 0x300;
            for (int k = 0; k < 4; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x18b;
        unsigned char *tex = (unsigned char *)0x792218;
        while (tex < (unsigned char *)0x792608) {
            int scaleVal = 0x307;
            for (int k = 0; k < 3; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x236;
        unsigned char *tex = (unsigned char *)0x798e08;
        while (tex < (unsigned char *)0x7991f8) {
            int scaleVal = 0x307;
            for (int k = 0; k < 3; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x103;
        unsigned char *tex = (unsigned char *)0x7a7aa0;
        while (tex < (unsigned char *)0x7a81a0) {
            int scaleVal = 0x118;
            for (int k = 0; k < 4; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        unsigned char *tex = (unsigned char *)0x78a340;
        int scaleVal = 1;
        while (tex < (unsigned char *)0x78a490) {
            ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, 0x274, 0x38, 0x38, (TexHeap *)0);
            tex += 0x70;
            scaleVal += 0x39;
        }
    }

    {
        unsigned char *tex = (unsigned char *)0x788100;
        int scaleVal = 1;
        while (tex < (unsigned char *)0x788250) {
            ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, 0x2ad, 0x38, 0x38, (TexHeap *)0);
            tex += 0x70;
            scaleVal += 0x39;
        }
    }

    {
        int rowVal = 0x204;
        for (int row = 0; row < 4; row++) {
            int scaleVal = 0x118;
            for (int col = 0; col < 4; col++) {
                if (row == 3 && col == row) continue;
                int a = col + row * 4;
                unsigned char *tex = (unsigned char *)0x776a80 + (a * 7) * 16;
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0x204;
        for (int row = 0; row < 4; row++) {
            int scaleVal = 0x1fc;
            for (int col = 0; col < 4; col++) {
                if (row == 3 && col == row) continue;
                int a = col + row * 4;
                unsigned char *tex = (unsigned char *)0x777110 + (a * 7) * 16;
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 0xdb;
        unsigned char *tex = (unsigned char *)0x799238;
        while (tex < (unsigned char *)0x799628) {
            int scaleVal = 0x307;
            for (int k = 0; k < 3; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }

    {
        int rowVal = 6;
        unsigned char *tex = (unsigned char *)0x78a758;
        while (tex < (unsigned char *)0x78ae58) {
            int scaleVal = 0x20e;
            for (int k = 0; k < 4; k++) {
                ((Texture *)tex)->extract((Buffer *)g_00798668, scaleVal, rowVal, 0x38, 0x38, (TexHeap *)0);
                scaleVal += 0x39;
                tex += 0x70;
            }
            rowVal += 0x39;
        }
    }
    ((Buffer *)g_00798668)->load_pcx((char *)g_00684fc8, (Palette *)0, 0xa, 0xec);  // 0x004521A8
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004521BC
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004521D0
    ((Sprite *)g_007acbb8)->extract((Buffer *)g_00798668, 0x109, 1, 0x34, 0x64, 0x64, (TexHeap *)0);  // 0x004521EE

    arr2[0].extract((Buffer *)g_00798668, 0x109, 2, 0x14d, 0x71, 0x38, (TexHeap *)0);

    {
        unsigned char *dst = (unsigned char *)0x787e54;
        for (int rowVal = 0x158; rowVal < 0x18b; rowVal += 0x11) {
            for (int colVal = 0x82; colVal < 0xba; colVal += 0xe) {
                *dst = (unsigned char)((Buffer *)g_00798668)->get_pixel(colVal, rowVal);
                dst++;
            }
        }
    }

    unsigned char remapTable[0x100];
    for (int zi = 0; zi < 0x100; zi++) remapTable[zi] = 0;
    for (int i = 0; i < 0x35; i++) {
        unsigned char idx = (unsigned char)(*(unsigned char *)(0x6846f8 + i) + 0xa);
        remapTable[idx] = (unsigned char)i;
    }

    buf804.init(0xaa, 0x3c, 0, (ExtDirectDraw *)0);
    {
        unsigned char transByte = (unsigned char)arr2[0].cTransparentIndex_;
        arr2[0].draw(&buf804, transByte, 0, 0, remapTable);
    }
    ((Texture *)0x76e8a0)->extract(&buf804, 0, 0, 0x38, 0x38, (TexHeap *)0);
    ((Texture *)0x76e910)->extract(&buf804, 0x39, 0, 0x38, 0x38, (TexHeap *)0);

    arr2[0].extract((Buffer *)g_00798668, 0x109, 2, 0xfe, 0x3c, 0x3c, (TexHeap *)0);
    {
        static const unsigned char kRemapSrc[9] = { 0xd3, 0xd0, 0x29, 0x8f, 0x74, 0x97, 0xce, 0x5f, 0xc0 };
        for (int zi = 0; zi < 0x100; zi++) remapTable[zi] = 0;
        for (int i = 0; i < 9; i++) {
            unsigned char idx = (unsigned char)(kRemapSrc[i] + 0xa);
            remapTable[idx] = (unsigned char)i;
        }
    }
    buf804.fill(9);
    {
        unsigned char transByte2 = (unsigned char)arr2[0].cTransparentIndex_;
        arr2[0].draw(&buf804, transByte2, 0, 0, remapTable);
    }
    ((Texture *)0x78dd80)->extract(&buf804, 0, 0, 0x3c, 0x3c, (TexHeap *)0);
    ((Buffer *)g_00798668)->load_pcx((char *)g_00684fd8, (Palette *)0, 0, 0x100);  // 0x004523BE
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004523D2
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004523E6
    ((Sprite *)g_00779770)->extract((Buffer *)g_00798668, 0x109, 1, 1, 0x64, 0x3e, (TexHeap *)0);  // 0x00452404
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452418
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045242C
    ((Sprite *)g_0077979c)->extract((Buffer *)g_00798668, 0x109, 0x66, 1, 0x64, 0x3e, (TexHeap *)0);  // 0x0045244A
    ((Buffer *)g_00798668)->load_pcx((char *)g_00684fe4, (Palette *)0, 0xa, 0xec);  // 0x00452462
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452476
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045248A
    ((Sprite *)g_00776994)->extract((Buffer *)g_00798668, 0x109, 0x1a, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x004524A8
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004524BC
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004524D0
    ((Sprite *)g_00776910)->extract((Buffer *)g_00798668, 0x109, 0x7f, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x004524EE
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452502
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452516
    ((Sprite *)g_007769c0)->extract((Buffer *)g_00798668, 0x109, 0xe4, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x00452537
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045254B
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045255F
    ((Sprite *)g_00776860)->extract((Buffer *)g_00798668, 0x109, 0x1a, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x0045257D
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452591
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004525A5
    ((Sprite *)g_0077688c)->extract((Buffer *)g_00798668, 0x109, 0x7f, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x004525C3
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004525D7
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004525EB
    ((Sprite *)g_007768b8)->extract((Buffer *)g_00798668, 0x109, 0xe4, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x0045260C
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452620
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452634
    ((Sprite *)g_00776834)->extract((Buffer *)g_00798668, 0x109, 0x1a, 0x93, 0x64, 0x3e, (TexHeap *)0);  // 0x00452655
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452669
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045267D
    ((Sprite *)g_00776758)->extract((Buffer *)g_00798668, 0x109, 0x7f, 0x93, 0x64, 0x3e, (TexHeap *)0);  // 0x0045269E
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004526B2
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004526C6
    ((Sprite *)g_00776784)->extract((Buffer *)g_00798668, 0x109, 0xe4, 0x93, 0x64, 0x3e, (TexHeap *)0);  // 0x004526EA
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004526FE
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452712
    ((Sprite *)g_00776808)->extract((Buffer *)g_00798668, 0x109, 0x1a, 0xd2, 0x64, 0x3e, (TexHeap *)0);  // 0x00452733
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452747
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045275B
    ((Sprite *)g_007767dc)->extract((Buffer *)g_00798668, 0x109, 0x7f, 0xd2, 0x64, 0x3e, (TexHeap *)0);  // 0x0045277C
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452790
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004527A4
    ((Sprite *)g_007767b0)->extract((Buffer *)g_00798668, 0x109, 0xe4, 0xd2, 0x64, 0x3e, (TexHeap *)0);  // 0x004527C8
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004527DC
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004527F0
    ((Sprite *)g_00776968)->extract((Buffer *)g_00798668, 0x109, 0x1a, 0x111, 0x64, 0x3e, (TexHeap *)0);  // 0x00452811
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452825
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452839
    ((Sprite *)g_007768e4)->extract((Buffer *)g_00798668, 0x109, 0x7f, 0x111, 0x64, 0x3e, (TexHeap *)0);  // 0x0045285A
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045286E
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452882
    ((Sprite *)g_0077693c)->extract((Buffer *)g_00798668, 0x109, 0xe4, 0x111, 0x64, 0x3e, (TexHeap *)0);  // 0x004528A6
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004528BA
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004528CE
    ((Sprite *)g_00799658)->extract((Buffer *)g_00798668, 0x109, 0x149, 0x93, 0x64, 0x3e, (TexHeap *)0);  // 0x004528F2
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452906
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045291A
    ((Sprite *)g_00799684)->extract((Buffer *)g_00798668, 0x109, 0x1ae, 0x93, 0x64, 0x3e, (TexHeap *)0);  // 0x0045293E
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452952
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452966
    ((Sprite *)g_007996b0)->extract((Buffer *)g_00798668, 0x109, 0x149, 0xd2, 0x64, 0x3e, (TexHeap *)0);  // 0x0045298A
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045299E
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004529B2
    ((Sprite *)g_007996dc)->extract((Buffer *)g_00798668, 0x109, 0x1ae, 0xd2, 0x64, 0x3e, (TexHeap *)0);  // 0x004529D6
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004529EA
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004529FE
    ((Sprite *)g_007ace4c)->extract((Buffer *)g_00798668, 0x109, 0x149, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x00452A1F
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452A33
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452A47
    ((Sprite *)g_007acd70)->extract((Buffer *)g_00798668, 0x109, 0x1ae, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x00452A68
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452A7C
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452A90
    ((Sprite *)g_007acd9c)->extract((Buffer *)g_00798668, 0x109, 0x213, 0x15, 0x64, 0x3e, (TexHeap *)0);  // 0x00452AB1
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452AC5
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452AD9
    ((Sprite *)g_007ace20)->extract((Buffer *)g_00798668, 0x109, 0x149, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x00452AFA
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452B0E
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452B22
    ((Sprite *)g_007acdf4)->extract((Buffer *)g_00798668, 0x109, 0x1ae, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x00452B43
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452B57
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452B6B
    ((Sprite *)g_007acdc8)->extract((Buffer *)g_00798668, 0x109, 0x213, 0x54, 0x64, 0x3e, (TexHeap *)0);  // 0x00452B8C
    ((Buffer *)g_00798668)->load_pcx((char *)g_00684ff4, (Palette *)0, 0xa, 0xec);  // 0x00452BA4

    {
        int scaleVal = 1;
        unsigned char *tex = (unsigned char *)0x791c84;
        do {
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex - 0x2c))->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x1bf, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)tex)->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x243, 0x64, 0x3e, (TexHeap *)0);
            tex += 0x58;
            scaleVal += 0x65;
        } while (tex < (unsigned char *)0x791d8c);
    }

    {
        int scaleVal = 1;
        unsigned char *tex = (unsigned char *)0x7a99cc;
        do {
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex - 0x2c))->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x1fe, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)tex)->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x282, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex + 0x2c))->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x2c1, 0x64, 0x3e, (TexHeap *)0);
            tex += 0x84;
            scaleVal += 0x65;
        } while (tex < (unsigned char *)0x7a9b58);
    }
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452D62
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452D76
    ((Sprite *)g_0077944c)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 1, 0x64, 0x3e, (TexHeap *)0);  // 0x00452D97
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452DAB
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452DBF
    ((Sprite *)g_00776a50)->extract((Buffer *)g_00798668, 0x109, 0x25f, 0x40, 0x64, 0x3e, (TexHeap *)0);  // 0x00452DE0
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452DF4
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452E08
    ((Sprite *)g_00789b68)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 0x40, 0x64, 0x3e, (TexHeap *)0);  // 0x00452E29
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452E3D
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452E51
    ((Sprite *)g_007991f8)->extract((Buffer *)g_00798668, 0x109, 0x25f, 0x7f, 0x64, 0x3e, (TexHeap *)0);  // 0x00452E72
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452E86
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452E9A
    ((Sprite *)g_0078a5b0)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 0x7f, 0x64, 0x3e, (TexHeap *)0);  // 0x00452EBB
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452ECF
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452EE3
    ((Sprite *)g_00798c40)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 0xbe, 0x64, 0x3e, (TexHeap *)0);  // 0x00452F07
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452F1B
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452F2F
    ((Sprite *)g_007a6860)->extract((Buffer *)g_00798668, 0x109, 0x25f, 0xbe, 0x64, 0x3e, (TexHeap *)0);  // 0x00452F53
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452F67
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452F7B
    ((Sprite *)g_007ad010)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 0x13c, 0x64, 0x3e, (TexHeap *)0);  // 0x00452F9F
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452FB3
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00452FC7
    ((Sprite *)g_007acc60)->extract((Buffer *)g_00798668, 0x109, 0x25f, 0x13c, 0x64, 0x3e, (TexHeap *)0);  // 0x00452FEB
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00452FFF
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453013
    ((Sprite *)g_007a6830)->extract((Buffer *)g_00798668, 0x109, 0x2c4, 0x13c, 0x64, 0x3e, (TexHeap *)0);  // 0x00453037

    {
        int scaleVal = 0x1c5;
        for (unsigned char *off = (unsigned char *)0; off < (unsigned char *)0xdc; off += 0x2c) {
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(off + 0x7792b0))->extract((Buffer *)g_00798668, 0x109, 0x26c, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(off + 0x79a710))->extract((Buffer *)g_00798668, 0x109, 0x336, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(off + 0x791fc8))->extract((Buffer *)g_00798668, 0x109, 0x39b, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            scaleVal += 0x3f;
        }
    }
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045313F
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453153
    ((Sprite *)g_00779390)->extract((Buffer *)g_00798668, 0x109, 0x1fa, 0xfd, 0x64, 0x3e, (TexHeap *)0);  // 0x00453177
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045318B
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045319F
    ((Sprite *)g_007abf60)->extract((Buffer *)g_00798668, 0x109, 0x25f, 0xfd, 0x64, 0x3e, (TexHeap *)0);  // 0x004531C3
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004531D7
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004531EB
    ((Sprite *)g_0078dd20)->extract((Buffer *)g_00798668, 0x109, 0x2c4, 0xfd, 0x64, 0x3e, (TexHeap *)0);  // 0x0045320F
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453223
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453237
    ((Sprite *)g_007acb88)->extract((Buffer *)g_00798668, 0x109, 0x2c4, 0xbe, 0x64, 0x3e, (TexHeap *)0);  // 0x0045325B
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045326F
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453283
    ((Sprite *)g_0078a700)->extract((Buffer *)g_00798668, 0x109, 0x336, 0xfd, 0x64, 0x3e, (TexHeap *)0);  // 0x004532A7
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004532BB
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004532CF
    ((Sprite *)g_0078a72c)->extract((Buffer *)g_00798668, 0x109, 0x336, 0xbe, 0x64, 0x3e, (TexHeap *)0);  // 0x004532F3
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453307
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045331B
    ((Sprite *)g_007ac11c)->extract((Buffer *)g_00798668, 0x109, 0x1de, 0x278, 0x64, 0x3e, (TexHeap *)0);  // 0x0045333F
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453353
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453367
    ((Sprite *)g_007ac0f0)->extract((Buffer *)g_00798668, 0x109, 0x1de, 0x2b7, 0x64, 0x3e, (TexHeap *)0);  // 0x0045338B
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x0045339F
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004533B3
    ((Sprite *)g_007ac148)->extract((Buffer *)g_00798668, 0x109, 0x243, 0x278, 0x64, 0x3e, (TexHeap *)0);  // 0x004533D7
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x004533EB
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x004533FF
    ((Sprite *)g_007ac098)->extract((Buffer *)g_00798668, 0x109, 0x243, 0x2b7, 0x64, 0x3e, (TexHeap *)0);  // 0x00453423
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453437
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x0045344B
    ((Sprite *)g_007ac174)->extract((Buffer *)g_00798668, 0x109, 0x2a8, 0x278, 0x64, 0x3e, (TexHeap *)0);  // 0x0045346F
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453483
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453497
    ((Sprite *)g_007ac0c4)->extract((Buffer *)g_00798668, 0x109, 0x2a8, 0x2b7, 0x64, 0x3e, (TexHeap *)0);  // 0x004534BB

    {
        int scaleVal = 0xfd;
        unsigned char *tex = (unsigned char *)0x75b2b4;
        do {
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex - 0x2c))->extract((Buffer *)g_00798668, 0x109, 1, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)tex)->extract((Buffer *)g_00798668, 0x109, 0x66, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex - 0x84))->extract((Buffer *)g_00798668, 0x109, 0xcb, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            ((Sprite *)(tex - 0x58))->extract((Buffer *)g_00798668, 0x109, 0x130, scaleVal, 0x64, 0x3e, (TexHeap *)0);
            tex += 0xb0;
            scaleVal += 0x3f;
        } while (tex < (unsigned char *)0x75b4c4);
    }

    {
        int scaleVal = 0x1a2;
        for (int n = 0; n < 6; n++) {
            ((Buffer *)g_00798668)->change_color(0x107, 0x109);
            ((Buffer *)g_00798668)->change_color(0x106, 0x109);
            int half = n / 2;
            int parity = (~n) & 1;
            int idx = (parity + half * 2) * 11;
            unsigned char *tex = (unsigned char *)0x77aff8 + idx * 4;
            ((Sprite *)tex)->extract((Buffer *)g_00798668, 0x109, scaleVal, 0x17b, 0x64, 0x3e, (TexHeap *)0);
            scaleVal += 0x65;
        }
    }
    ((Buffer *)g_00798668)->change_color(0x107, 0x109);  // 0x00453673
    ((Buffer *)g_00798668)->change_color(0x106, 0x109);  // 0x00453687
    ((Sprite *)g_0075b098)->extract((Buffer *)g_00798668, 0x109, 0x130, 1, 0x64, 0x3e, (TexHeap *)0);  // 0x004536A8

    ((Buffer *)g_00798668)->close();
}
