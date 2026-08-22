// ORIGINAL: 0x004A41D0 ?on_status@ReportIf@@QAEXH@Z 0x004A41D0-0x004A5ED2 FILE
// TRIED: full hand transcription from raw disassembly (not Ghidra, whose &DAT_x+idx*N pseudocode over-scales by 4x on 0x96cdac/0x96c9f8 - fixed here against the raw asm multiply chain and byte tests); covers close path (switch+jump table+loops), init path (7-button report loop, 4 button groups, single-column cef..cf8, has_tech/ bitmask channel grid, spying flags, 4-way spy-report geometry, scroll/report/slide tail). First divergence is instruction #3 (prologue 'mov'/'push' scheduling) - approximate throughout, not byte-verified block by block beyond the ones cited above.
// working copy - scaffold materialised by --work
// size      7426 bytes
// prototype void (__thiscall ?on_status@ReportIf@@QAEXH@Z)(ReportIf* this, int)
// callers   0   call targets   27
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x0045DEA0 0x0045E540 0x004A3950 0x004A9F90 0x004AB550 0x004C3A40 0x0050BA00 0x0055BC00 0x005B9F20 0x005D5250 0x005D5A70 0x005DAC70 0x005ED9D0 0x005EDCD0 0x00605840 0x00607210 0x00607550 0x00607DA0 0x00609F20 0x00616730 0x006169A0 0x0062B810 0x0062B850 0x0062B870 0x00645470
// indirect  0x004A4351 0x004A435B 0x004A448B 0x004A4544 0x004A462D 0x004A4637 0x004A46ED 0x004A46FF 0x004A4804 0x004A480E 0x004A48C4 0x004A48D6 0x004A49F6 0x004A4ACA 0x004A4BAA 0x004A4CFA 0x004A4DCF 0x004A4EC2 0x004A4FC4 0x004A4FCE 0x004A50B2 0x004A50BC 0x004A51A0 0x004A51AA 0x004A528E 0x004A5298 0x004A537C 0x004A5386 0x004A5436 0x004A5440 0x004A54F0 0x004A54FA 0x004A55C1 0x004A55CB 0x004A567B 0x004A5685 0x004A56C0 0x004A5740 0x004A574A 0x004A5903 0x004A590D 0x004A5BF5 0x004A5BFF 0x004A5C7E 0x004A5D63 0x004A5DDD 0x004A5DFB 0x004A5E30 0x004A5E3F 0x004A5E77 0x004A5EAF 0x004A5EBE 0x004A5ECD

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_status@ReportIf@@QAEXH@Z  at 0x004A41D0  (7426 bytes)
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
class Dialog;
struct DialogEntry;
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
class ListBox;
class Menu;
struct MenuEntry;
typedef void * PVOID;
class PullDown;
struct PullDownItem;
struct RECT;
class ReportIf;
class Scroll;
class Spot;
class Sprite;
class Strings;
class Time;
typedef unsigned int UINT_PTR;
class Win;
class WorldWin;

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
    void show(int);
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
    int set_font(Font *, Font *, Font *, Font *);
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
    void fill(int);
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
    int init(char *, int, int, int, int, int, Win *, int);
    int set_bubble_text(char *);
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
    int set(int, int);
    void add(BaseButton *);
    void close();
    void init(int, int);
};

class Dialog { public:
    LPVOID vtable_;
    Heap heap_;
    Heap * heap_ptr_;
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
    Font * font1_;
    Font * font2_;
    Font * font3_;
    uint32_t text_color_a_;
    uint32_t text_color_2a_;
    uint32_t text_color_3a_;
    uint32_t text_color_b_;
    uint32_t text_color_2b_;
    uint32_t text_color_3b_;
    uint32_t text_color_c_;
    uint32_t text_color_2c_;
    uint32_t text_color_3c_;
    uint32_t text_color_d_;
    uint32_t text_color_2d_;
    uint32_t text_color_3d_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    DialogEntry * entry_head_;
    DialogEntry * current_entry_;
    int entry_count_;
    int entry_position_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    int selected_position_;
    uint32_t field_F0_;
    int get_selected_id();
    void destroy();
};

struct DialogEntry {
    uint32_t vtable;
    int id;
    void * payload;
    DialogEntry * next;
    DialogEntry * previous;
    uint32_t secondary_vtable;
    void * heap;
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
    void close();
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

class ListBox { public:
    uint32_t vbtable_pointer_;
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
    uint32_t graphic_vbase_adjust_;
    GraphicWin virtual_base_;
    uint32_t dialog_vbase_adjust_;
    Dialog dialog_;
    uint32_t destroy();
    void close();
    void on_dialog_focus(int);
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
    int init(RECT *, Win *, int, int);
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

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

class WorldWin { public:
    void clear_terrain();
    void hide_all();
};

bool has_tech(int, int);
extern "C" char *strcat(char *, const char *);
extern "C" char *__stdcall CharUpperA(char *);
int spying(int);
void bitmask(int, int *, int *);
void scroll_hide(GraphicWin *, Scroll *);
void scroll_show(GraphicWin *, Scroll *);
void slide_hide(GraphicWin *, int);
void slide_show(GraphicWin *, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of
// a slot are yours to set and setting them does not move it: write
// `virtual int slot074(int, int);` if that is the call you need.
// The slots below are spelled nullary because the emitter reads the
// vtable OFFSET from the body and not the argument list.
// This body dispatches through slot(s): 2, 57, 62, 90
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057(int, void *, void *);  // <-- used
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004a5ed4 = (int *)0x004A5ED4;
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_006e84b4 = (int *)0x006E84B4;
static int *const g_0078dc30 = (int *)0x0078DC30;
static int *const g_0078dc58 = (int *)0x0078DC58;
static int *const g_0078dc80 = (int *)0x0078DC80;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007b39e8 = (int *)0x007B39E8;
static int *const g_007b4534 = (int *)0x007B4534;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_0087bdec = (int *)0x0087BDEC;
static int *const g_0087bdf0 = (int *)0x0087BDF0;
static int *const g_0087bdf4 = (int *)0x0087BDF4;
static int *const g_0087bdf8 = (int *)0x0087BDF8;
static int *const g_0087be24 = (int *)0x0087BE24;
static int *const g_0087be28 = (int *)0x0087BE28;
static int *const g_0087be84 = (int *)0x0087BE84;
static int *const g_0087c9d8 = (int *)0x0087C9D8;
static int *const g_0087ce1c = (int *)0x0087CE1C;
static int *const g_0087d524 = (int *)0x0087D524;
static int *const g_0087d968 = (int *)0x0087D968;
static int *const g_0087e070 = (int *)0x0087E070;
static int *const g_0087e074 = (int *)0x0087E074;
static int *const g_0087e078 = (int *)0x0087E078;
static int *const g_0087e07c = (int *)0x0087E07C;
static int *const g_0087e080 = (int *)0x0087E080;
static int *const g_0087e114 = (int *)0x0087E114;
static int *const g_0087e558 = (int *)0x0087E558;
static int *const g_0087ebb0 = (int *)0x0087EBB0;
static int *const g_0087ebbc = (int *)0x0087EBBC;
static int *const g_0087ec60 = (int *)0x0087EC60;
static int *const g_0087f0a4 = (int *)0x0087F0A4;
static int *const g_0087f6fc = (int *)0x0087F6FC;
static int *const g_0087f708 = (int *)0x0087F708;
static int *const g_0087f7ac = (int *)0x0087F7AC;
static int *const g_0087fbf0 = (int *)0x0087FBF0;
static int *const g_00880248 = (int *)0x00880248;
static int *const g_00880254 = (int *)0x00880254;
static int *const g_008802f8 = (int *)0x008802F8;
static int *const g_0088038c = (int *)0x0088038C;
static int *const g_008807d0 = (int *)0x008807D0;
static int *const g_00880e28 = (int *)0x00880E28;
static int *const g_00880e34 = (int *)0x00880E34;
static int *const g_00880ed8 = (int *)0x00880ED8;
static int *const g_0088131c = (int *)0x0088131C;
static int *const g_00881974 = (int *)0x00881974;
static int *const g_00881980 = (int *)0x00881980;
static int *const g_00881a24 = (int *)0x00881A24;
static int *const g_00881e68 = (int *)0x00881E68;
static int *const g_008824c0 = (int *)0x008824C0;
static int *const g_008824cc = (int *)0x008824CC;
static int *const g_008824ec = (int *)0x008824EC;
static int *const g_008824f0 = (int *)0x008824F0;
static int *const g_008824f4 = (int *)0x008824F4;
static int *const g_00882570 = (int *)0x00882570;
static int *const g_00882584 = (int *)0x00882584;
static int *const g_008846d0 = (int *)0x008846D0;
static int *const g_008846d4 = (int *)0x008846D4;
static int *const g_008846d8 = (int *)0x008846D8;
static int *const g_008846dc = (int *)0x008846DC;
static int *const g_008846e4 = (int *)0x008846E4;
static int *const g_008846ec = (int *)0x008846EC;
static int *const g_00885f30 = (int *)0x00885F30;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;
static int *const g_008a4164 = (int *)0x008A4164;
static int *const g_008a4170 = (int *)0x008A4170;
static int *const g_008a6240 = (int *)0x008A6240;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946fec = (int *)0x00946FEC;
static int *const g_009472ec = (int *)0x009472EC;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cdac = (int *)0x0096CDAC;
static int *const g_0096f2bc = (int *)0x0096F2BC;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d0cc = (int *)0x0097D0CC;
static int *const g_0097d850 = (int *)0x0097D850;
static int *const g_009a532c = (int *)0x009A532C;
static int *const g_009a57ac = (int *)0x009A57AC;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009b7ae0 = (int *)0x009B7AE0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class ReportIf { public:
    void report(int, int, int);
    void close_ops();
    void close_sat();
    void done();
    void close_energy();
    void close_intel();
    void on_iface_right_click(int, int);
    void on_iface_left_double_click(int, int);
    void on_iface_right_double_click(int, int);
    void on_iface_left_down(int, int);
    void on_iface_right_down(int, int);
    void on_iface_mouse_move(int, int);
    void on_iface_mouse_leave(int, int);
    void on_iface_button_toggled(int, int);
    void close_score();
    int on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *);
    void bl_anim();
    void on_status(int);
};
void ReportIf::on_status(int a1) {
    char *self = reinterpret_cast<char *>(this);

    if (a1 == 0) {
        // ---- 0x004A5D72: tear the interface down ----
        *g_008a6240 = 0;
        reinterpret_cast<Time *>(g_008846ec)->stop();
        if (*g_00882570 != 0) {
            scroll_hide(reinterpret_cast<GraphicWin *>(g_00876478), reinterpret_cast<Scroll *>(g_00882584));
            *g_00882570 = 0;
        }
        slide_hide(reinterpret_cast<GraphicWin *>(g_00876478), 0);
        reinterpret_cast<Win *>(g_00876478)->hide();
        reinterpret_cast<FlatButton *>(g_0087d524)->close();
        reinterpret_cast<FlatButton *>(g_0087c9d8)->close();

        {
            char *p = self + 0x5384;
            int n = 7;
            do {
                reinterpret_cast<VCall *>(p)->slot090();
                p += 0xB4C;
            } while (--n != 0);
        }
        {
            char *p = self + 0x80;
            int n = 7;
            do {
                reinterpret_cast<VCall *>(p)->slot090();
                p += 0xB4C;
            } while (--n != 0);
        }

        switch (*reinterpret_cast<int *>(self + 8)) {
        case 3:
            reinterpret_cast<ListBox *>(g_0087be84)->close();
            // fall through
        case 0:
            reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
            reinterpret_cast<ListBox *>(self + 0xAE24)->close();
            break;
        case 2:
            reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
            reinterpret_cast<VCall *>(self + 0xBA0C)->slot002();
            reinterpret_cast<VCall *>(self + 0xC558)->slot002();
            break;
        case 4:
            close_ops();
            break;
        case 6:
            close_sat();
            break;
        case 7:
            reinterpret_cast<VCall *>(self + 0x1BF9C)->slot002();
            reinterpret_cast<VCall *>(self + 0x1D634)->slot002();
            reinterpret_cast<VCall *>(self + 0x1CAE8)->slot002();
            break;
        default:
            break;
        }

        reinterpret_cast<WorldWin *>(g_008e9f60)->hide_all();

        {
            char *p = reinterpret_cast<char *>(g_007b39e8);
            char *stop = reinterpret_cast<char *>(g_007b4534);
            do {
                reinterpret_cast<VCall *>(p)->slot002();
                p += 0xB4C;
            } while (p <= stop);
        }
        return;
    }

    // ---- 0x004A41E8: (re)build the interface ----
    if (*g_008a6240 != 0) {
        *g_008a6240 = 0;
        return;
    }

    *reinterpret_cast<int *>(self + 0x1E238) = 1;
    *reinterpret_cast<int *>(self + 0x1E234) = 1;
    *g_00882570 = 0;
    *reinterpret_cast<int *>(self + 0x1E23C) = 1;
    reinterpret_cast<GraphicWin *>(g_00876478)->fill(9);
    *reinterpret_cast<int *>(self + 0x1E228) = *g_00939284;
    {
        int *p = reinterpret_cast<int *>(self + 0x1E1DC);
        int n = 0x13;
        do {
            *p++ = -1;
        } while (--n != 0);
    }
    *g_0087be24 = -1;
    *reinterpret_cast<int *>(self + 0x1E1D8) = 1;

    {
        int playerIdx = *reinterpret_cast<int *>(self + 0x1E228);
        if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0 ||
            (*g_009a649c & 0x200) != 0) {
            int val = *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096cdac) + playerIdx * 0x20CC);
            *g_00885f30 = val;
            if (val < 0 || val > 0x59) {
                *g_00885f30 = 0;
            }
        } else {
            *g_00885f30 = 0;
        }
    }

    // ---- 0x004A42B8: 7 report-mode buttons ----
    {
        int strOff = 0xAC4;
        int *colorArr = reinterpret_cast<int *>(self + 0xA298);
        char *textEntry = self + 0x4FC0;
        FlatButton *btn = reinterpret_cast<FlatButton *>(self + 0x5384);
        for (int i = 0; i < 7; i++) {
            *reinterpret_cast<char *>(g_009b86a0) = 0;
            int sid = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + strOff);
            int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(sid);
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));

            btn->init(0, a1 + 2, *colorArr,
                      *reinterpret_cast<int *>(self + 0xA2CC),
                      *reinterpret_cast<int *>(self + 0x4FAC),
                      *reinterpret_cast<int *>(self + 0x4FB0),
                      reinterpret_cast<Win *>(g_007ae820), 0);
            btn->field_AD8_ = reinterpret_cast<uint32_t>(textEntry);
            btn->field_AD4_ = reinterpret_cast<uint32_t>(textEntry - 0x2C);
            btn->field_ADC_ = reinterpret_cast<uint32_t>(textEntry + 0x2C);
            btn->set_bubble_text(reinterpret_cast<char *>(g_009b86a0));
            btn->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
            reinterpret_cast<VCall *>(btn)->slot062();
            reinterpret_cast<VCall *>(btn)->slot062();

            strOff += 4;
            colorArr++;
            textEntry += 0x84;
            btn = reinterpret_cast<FlatButton *>(reinterpret_cast<char *>(btn) + 0xB4C);
        }
    }

    // ---- 0x004A4397: shared caption geometry ----
    int capIVar8;
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        *g_0087e074 = *g_0087bdf8 + 7;
        *g_0087e07c = *g_0087bdf8 + 0x1B;
        *g_0087e070 = *g_0087bdec + 0x4B;
        *g_0087e078 = (*g_0087bdf4 - *g_0087bdec) + -0x96 + *g_0087e070;
        capIVar8 = (*g_0087bdf4 + *g_0087bdec) / 2;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x86C));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));

        // button id=1 (0x87d524), parent = main window
        *reinterpret_cast<int *>(self + 0x1E1C8) = *g_0087e070;
        *reinterpret_cast<int *>(self + 0x1E1CC) = *g_0087e074;
        *reinterpret_cast<int *>(self + 0x1E1D0) = capIVar8 - 1;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *g_0087e074 + 0x14;
        reinterpret_cast<BaseButton *>(g_0087d524)->init(
            reinterpret_cast<char *>(g_009b86a0), 1,
            *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
            *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
            *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0087d524)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0087d524)->slot062();
        reinterpret_cast<GraphicWin *>(g_0087d524)->redraw();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xA64));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));

        // button id=9 (0x87c9d8), parent = main window
        int rightEdge = capIVar8 + 1 + (capIVar8 - *g_0087e070) - 1;
        *reinterpret_cast<int *>(self + 0x1E1CC) = *g_0087e074;
        *reinterpret_cast<int *>(self + 0x1E1C8) = capIVar8 + 1;
        *reinterpret_cast<int *>(self + 0x1E1D0) = rightEdge;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *g_0087e074 + 0x14;
        reinterpret_cast<BaseButton *>(g_0087c9d8)->init(
            reinterpret_cast<char *>(g_009b86a0), 9,
            *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
            rightEdge - *reinterpret_cast<int *>(self + 0x1E1C8),
            *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0087c9d8)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0087c9d8)->slot062();
        reinterpret_cast<GraphicWin *>(g_0087c9d8)->redraw();
    }

    // ---- 0x004A44FD: report-mode group A (id 2), buttons 0x92/0x93 ----
    *reinterpret_cast<int *>(self + 0xC) = *reinterpret_cast<int *>(self + 0x20) + 9;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x24) + 9;
    *reinterpret_cast<int *>(self + 0x14) = (*reinterpret_cast<int *>(self + 0x28) - *reinterpret_cast<int *>(self + 0x20)) - 0x12;
    reinterpret_cast<ButtonGroup *>(self + 0xB978)->init(2, 0);
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x5D4));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14) / 2 - 1;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *b92 = reinterpret_cast<FlatButton *>(self + 0xBA0C);
        b92->init(reinterpret_cast<char *>(g_009b86a0), 0x92,
                  *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                  *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                  *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                  reinterpret_cast<Win *>(g_007ae820), 0);
        b92->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(b92)->slot062();
        reinterpret_cast<VCall *>(b92)->slot002();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x5D8));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int iVar3 = *reinterpret_cast<int *>(self + 0x14) / 2;
        int iLeft = iVar3 + 1 + *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1C8) = iLeft;
        *reinterpret_cast<int *>(self + 0x1E1D0) = iLeft + iVar3;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *b93 = reinterpret_cast<FlatButton *>(self + 0xC558);
        b93->init(reinterpret_cast<char *>(g_009b86a0), 0x93,
                  *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                  *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                  *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                  reinterpret_cast<Win *>(g_007ae820), 0);
        b93->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(b93)->slot062();
        reinterpret_cast<VCall *>(b93)->slot002();

        ButtonGroup *grpA = reinterpret_cast<ButtonGroup *>(self + 0xB978);
        grpA->add(b92);
        grpA->add(b93);
        grpA->set(0x92, 1);
    }

    // ---- 0x004A472C: report-mode group B (id 4), buttons 0xd0a/0xd0b ----
    *reinterpret_cast<int *>(self + 0xC) = *reinterpret_cast<int *>(self + 0x20) + 9;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x24) + 9;
    *reinterpret_cast<int *>(self + 0x14) = (*reinterpret_cast<int *>(self + 0x28) - *reinterpret_cast<int *>(self + 0x20)) - 0x12;
    reinterpret_cast<ButtonGroup *>(self + 0xD0A4)->init(4, 0);
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x7B8));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14) / 2 - 1;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *bA = reinterpret_cast<FlatButton *>(self + 0xD138);
        bA->init(reinterpret_cast<char *>(g_009b86a0), 0xD0A,
                 *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                 *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                 *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                 reinterpret_cast<Win *>(g_007ae820), 0);
        bA->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(bA)->slot062();
        reinterpret_cast<VCall *>(bA)->slot002();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xAA8));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int iVar3 = *reinterpret_cast<int *>(self + 0x14) / 2;
        int iLeft = iVar3 + 1 + *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        *reinterpret_cast<int *>(self + 0x1E1C8) = iLeft;
        *reinterpret_cast<int *>(self + 0x1E1D0) = iLeft + iVar3;
        FlatButton *bB = reinterpret_cast<FlatButton *>(self + 0xDC84);
        bB->init(reinterpret_cast<char *>(g_009b86a0), 0xD0B,
                 *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                 *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                 *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                 reinterpret_cast<Win *>(g_007ae820), 0);
        bB->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(bB)->slot062();
        reinterpret_cast<VCall *>(bB)->slot002();

        ButtonGroup *grpB = reinterpret_cast<ButtonGroup *>(self + 0xD0A4);
        grpB->add(bA);
        grpB->add(bB);
        grpB->set(0xD0A, 1);
    }

    // ---- 0x004A48F5: fixed group 0x87e080 (id 4), buttons 0x8b3/0x8b4/0x8b5 ----
    *reinterpret_cast<int *>(self + 0xC) = *g_0087bdec + 3;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x1C) + *g_0087bdf0 + 0xB;
    *reinterpret_cast<int *>(self + 0x14) = (*g_0087bdf4 - *g_0087bdec) - 6;
    reinterpret_cast<ButtonGroup *>(g_0087e080)->init(4, 0);
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xCB0));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int left = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1C8) = left;
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0x14) / 3 - 2 + left;
        reinterpret_cast<BaseButton *>(g_0087e114)->init(
            reinterpret_cast<char *>(g_009b86a0), 0x8B3, left, *reinterpret_cast<int *>(self + 0x1E1CC),
            *reinterpret_cast<int *>(self + 0x1E1D0) - left,
            *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0087e114)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0087e114)->slot062();
        if (*g_0087ebbc == 0) {
            *g_0087ebb0 = 0;
        }
        reinterpret_cast<Win *>(g_0087e114)->hide();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xBA4));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int iVar3 = *reinterpret_cast<int *>(self + 0x14) / 3;
        int iLeft = *reinterpret_cast<int *>(self + 0xC) + iVar3;
        int iRight = *reinterpret_cast<int *>(self + 0xC) + iVar3 * 2;
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        *reinterpret_cast<int *>(self + 0x1E1C8) = iLeft;
        *reinterpret_cast<int *>(self + 0x1E1D0) = iRight;
        reinterpret_cast<BaseButton *>(g_0087ec60)->init(
            reinterpret_cast<char *>(g_009b86a0), 0x8B4, iLeft, *reinterpret_cast<int *>(self + 0x1E1CC),
            iRight - iLeft, *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0087ec60)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0087ec60)->slot062();
        if (*g_0087f708 == 0) {
            *g_0087f6fc = 0;
        }
        reinterpret_cast<Win *>(g_0087ec60)->hide();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xFC));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int rowBottom = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D4) = rowBottom;
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
        int cLeft = *reinterpret_cast<int *>(self + 0xC) + 2 + (*reinterpret_cast<int *>(self + 0x14) / 3) * 2;
        *reinterpret_cast<int *>(self + 0x1E1C8) = cLeft;
        reinterpret_cast<BaseButton *>(g_0087f7ac)->init(
            reinterpret_cast<char *>(g_009b86a0), 0x8B5, cLeft, *reinterpret_cast<int *>(self + 0x1E1CC),
            *reinterpret_cast<int *>(self + 0x1E1D0) - cLeft, rowBottom - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0087f7ac)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0087f7ac)->slot062();
        if (*g_00880254 == 0) {
            *g_00880248 = 0;
        }
        reinterpret_cast<Win *>(g_0087f7ac)->hide();

        ButtonGroup *grpC = reinterpret_cast<ButtonGroup *>(g_0087e080);
        grpC->add(reinterpret_cast<BaseButton *>(g_0087e114));
        grpC->add(reinterpret_cast<BaseButton *>(g_0087ec60));
        grpC->add(reinterpret_cast<BaseButton *>(g_0087f7ac));
        int def = *reinterpret_cast<int *>(self + 0x1E240);
        grpC->set(def == 0 ? 0x8B3 : def, 1);
    }

    // ---- 0x004A4C13: fixed group 0x8802f8 (id 6), buttons 0xd3c/0xd3d ----
    *reinterpret_cast<int *>(self + 0xC) = *g_0087bdec + 3;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x1C) + *g_0087bdf0 + 0xB;
    *reinterpret_cast<int *>(self + 0x14) = (*g_0087bdf4 - *g_0087bdec) - 6;
    reinterpret_cast<ButtonGroup *>(g_008802f8)->init(6, 0);
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xCB0));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int left = *reinterpret_cast<int *>(self + 0xC);
        int top = *reinterpret_cast<int *>(self + 0x10);
        int right = *reinterpret_cast<int *>(self + 0x14) / 2 - 1 + left;
        *reinterpret_cast<int *>(self + 0x1E1C8) = left;
        *reinterpret_cast<int *>(self + 0x1E1CC) = top;
        *reinterpret_cast<int *>(self + 0x1E1D0) = right;
        *reinterpret_cast<int *>(self + 0x1E1D4) = top + 0x14;
        reinterpret_cast<BaseButton *>(g_0088038c)->init(
            reinterpret_cast<char *>(g_009b86a0), 0xD3C, left, top, right - left, 0x14,
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_0088038c)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_0088038c)->slot062();
        if (*g_00880e34 == 0) {
            *g_00880e28 = 0;
        }
        reinterpret_cast<Win *>(g_0088038c)->hide();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xCB4));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int local18 = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        int iLeft2 = *reinterpret_cast<int *>(self + 0x14) / 2 + 1 + *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
        *reinterpret_cast<int *>(self + 0x1E1D4) = local18;
        *reinterpret_cast<int *>(self + 0x1E1C8) = iLeft2;
        reinterpret_cast<BaseButton *>(g_00880ed8)->init(
            reinterpret_cast<char *>(g_009b86a0), 0xD3D, iLeft2, *reinterpret_cast<int *>(self + 0x1E1CC),
            *reinterpret_cast<int *>(self + 0x1E1D0) - iLeft2, local18 - *reinterpret_cast<int *>(self + 0x1E1CC),
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_00880ed8)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_00880ed8)->slot062();
        if (*g_00881980 == 0) {
            *g_00881974 = 0;
        }
        reinterpret_cast<Win *>(g_00880ed8)->hide();

        ButtonGroup *grpD = reinterpret_cast<ButtonGroup *>(g_008802f8);
        grpD->add(reinterpret_cast<BaseButton *>(g_0088038c));
        grpD->add(reinterpret_cast<BaseButton *>(g_00880ed8));
        grpD->set(0xD3C, 1);
    }

    // ---- 0x004A4E1C: single button 0xd3e (0x881a24), parent = main window ----
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xCB8));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        int right = *g_0087e078;
        int left = *g_0087e070;
        int bottom = *g_0087bdf8 - 6;
        int top = *g_0087bdf8 - 0x1A;
        *reinterpret_cast<int *>(self + 0x1E1C8) = *g_0087e070;
        *reinterpret_cast<int *>(self + 0x1E1CC) = top;
        *reinterpret_cast<int *>(self + 0x1E1D0) = right;
        *reinterpret_cast<int *>(self + 0x1E1D4) = bottom;
        reinterpret_cast<BaseButton *>(g_00881a24)->init(
            reinterpret_cast<char *>(g_009b86a0), 0xD3E, left, top, right - left, bottom - top,
            reinterpret_cast<Win *>(g_00876478), 0);
        reinterpret_cast<Buffer *>(g_00881a24)->set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(g_00881a24)->slot062();
        *g_008824ec = (int)g_0078dc80;
        *g_008824f0 = (int)g_0078dc58;
        *g_008824f4 = (int)g_0078dc30;
        if (*g_008824cc == 0) {
            *g_008824c0 = 0;
        }
        reinterpret_cast<Win *>(g_00881a24)->hide();
    }

    // ---- 0x004A4EFF: report/order rect, single column cef..cf5 ----
    *reinterpret_cast<int *>(self + 0xC) = *reinterpret_cast<int *>(self + 0x20) + 9;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x24) + 0x23;
    *reinterpret_cast<int *>(self + 0x14) = (*reinterpret_cast<int *>(self + 0x28) - *reinterpret_cast<int *>(self + 0x20)) - 0x12;
    {
        static const int strOffs[7] = {0xBA8, 0xBAC, 0xBAC, 0xBAC, 0xBAC, 0xBB0, 0xBB4};
        static const int extraOffs[7] = {-1, 0x824, 0x828, 0x82C, 0x830, -1, -1};
        static const int ids[7] = {0xCEF, 0xCF0, 0xCF1, 0xCF2, 0xCF3, 0xCF4, 0xCF5};
        char *addrs[7];
        addrs[0] = self + 0xE7D0;
        addrs[1] = self + 0xF31C;
        addrs[2] = self + 0xFE68;
        addrs[3] = self + 0x109B4;
        addrs[4] = self + 0x11500;
        addrs[5] = self + 0x1204C;
        addrs[6] = self + 0x12B98;
        for (int i = 0; i < 7; i++) {
            *reinterpret_cast<char *>(g_009b86a0) = 0;
            int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + strOffs[i]));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
            if (extraOffs[i] >= 0) {
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
                sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + extraOffs[i]));
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
            }
            *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
            *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
            *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
            *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
            FlatButton *btn = reinterpret_cast<FlatButton *>(addrs[i]);
            btn->init(reinterpret_cast<char *>(g_009b86a0), ids[i],
                      *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                      *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                      *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                      reinterpret_cast<Win *>(g_007ae820), 0);
            btn->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
            reinterpret_cast<VCall *>(btn)->slot062();
            reinterpret_cast<VCall *>(btn)->slot002();
            *reinterpret_cast<int *>(self + 0x10) += 0x16;
        }
    }

    // ---- 0x004A54FD: new column base, buttons cf6/cf8/cf7 ----
    *reinterpret_cast<int *>(self + 0xC) = *reinterpret_cast<int *>(self + 0x30) + 9;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x34) + 0x20;
    *reinterpret_cast<int *>(self + 0x14) = (*reinterpret_cast<int *>(self + 0x38) - *reinterpret_cast<int *>(self + 0x30)) - 0x12;
    {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xEEC));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *btn = reinterpret_cast<FlatButton *>(self + 0x1BF9C);
        btn->init(reinterpret_cast<char *>(g_009b86a0), 0xCF6,
                  *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                  *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                  *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                  reinterpret_cast<Win *>(g_007ae820), 0);
        btn->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(btn)->slot062();
        reinterpret_cast<VCall *>(btn)->slot002();
        *reinterpret_cast<int *>(self + 0x10) += 0x16;

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xB6C));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *btn8 = reinterpret_cast<FlatButton *>(self + 0x1D634);
        btn8->init(reinterpret_cast<char *>(g_009b86a0), 0xCF8,
                   *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                   *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                   *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                   reinterpret_cast<Win *>(g_007ae820), 0);
        btn8->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(btn8)->slot062();
        reinterpret_cast<VCall *>(btn8)->slot002();
        *reinterpret_cast<int *>(self + 0x10) += 0x16;

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        sres = reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xB6C));
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
        CharUpperA(reinterpret_cast<char *>(g_009b86a0));
        *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
        *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
        *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
        *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
        FlatButton *btn7 = reinterpret_cast<FlatButton *>(self + 0x1CAE8);
        btn7->init(reinterpret_cast<char *>(g_009b86a0), 0xCF7,
                   *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                   *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                   *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                   reinterpret_cast<Win *>(g_007ae820), 0);
        btn7->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
        reinterpret_cast<VCall *>(btn7)->slot062();
        reinterpret_cast<VCall *>(btn7)->slot002();
    }

    // ---- 0x004A5773: group E (id 0x6a) init; members added below by the faction loop ----
    *reinterpret_cast<int *>(self + 0xC) = *reinterpret_cast<int *>(self + 0x20) + 9;
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x24) + 0x20;
    *reinterpret_cast<int *>(self + 0x14) = (*reinterpret_cast<int *>(self + 0x28) - *reinterpret_cast<int *>(self + 0x20)) - 0x12;
    reinterpret_cast<ButtonGroup *>(self + 0x136E4)->init(0x6A, 0);

    // ---- 0x004A57A7: per-faction "channel" grid (has_tech / bitmask driven) ----
    {
        char *nameCursor = self + 0x13778;
        FlatButton *btn = reinterpret_cast<FlatButton *>(nameCursor);
        for (int factionIdx = 1; factionIdx <= 7; factionIdx++, btn = reinterpret_cast<FlatButton *>(reinterpret_cast<char *>(btn) + 0xB4C)) {
            if (factionIdx == *g_00939284) {
                continue;
            }
            int ownerMatches = 0;
            int baseCount = *g_009a64cc;
            if (baseCount > 0) {
                int cursorOff = 0;
                for (int b = 0; b < baseCount; b++, cursorOff += 0x134) {
                    unsigned char owner = *(reinterpret_cast<unsigned char *>(g_0097d044) + cursorOff);
                    if (owner == static_cast<unsigned char>(factionIdx)) {
                        int byteOff = 0, mask = 0;
                        bitmask(0x29, &byteOff, &mask);
                        unsigned char bits = *(reinterpret_cast<unsigned char *>(g_0097d0cc) + cursorOff + byteOff);
                        if ((bits & mask) != 0) {
                            ownerMatches++;
                        }
                    }
                }
            }
            bool techA = has_tech(*g_009a57ac, factionIdx);
            bool show = techA;
            if (!show) {
                int *stats = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096f2bc) + (factionIdx - 1) * 0x20CC);
                int sum = stats[-2] + stats[-1] + stats[1] + ownerMatches + stats[0];
                show = (sum != 0);
                if (!show) {
                    show = has_tech(*g_009a532c, factionIdx);
                }
            }
            if (!show) {
                continue;
            }

            char *nameBase = reinterpret_cast<char *>(g_009472ec) + (factionIdx - 1) * 0x59C;
            *reinterpret_cast<char *>(g_009b86a0) = 0;
            *g_009bbfec = *reinterpret_cast<int *>(nameBase - 4);
            *g_009bbff0 = *reinterpret_cast<int *>(nameBase);
            strcat(reinterpret_cast<char *>(g_009b86a0), nameBase - 0x1C);

            *reinterpret_cast<int *>(self + 0x1E1C8) = *reinterpret_cast<int *>(self + 0xC);
            *reinterpret_cast<int *>(self + 0x1E1CC) = *reinterpret_cast<int *>(self + 0x10);
            *reinterpret_cast<int *>(self + 0x1E1D0) = *reinterpret_cast<int *>(self + 0xC) + *reinterpret_cast<int *>(self + 0x14);
            *reinterpret_cast<int *>(self + 0x1E1D4) = *reinterpret_cast<int *>(self + 0x10) + 0x14;
            btn->init(reinterpret_cast<char *>(g_009b86a0), factionIdx,
                      *reinterpret_cast<int *>(self + 0x1E1C8), *reinterpret_cast<int *>(self + 0x1E1CC),
                      *reinterpret_cast<int *>(self + 0x1E1D0) - *reinterpret_cast<int *>(self + 0x1E1C8),
                      *reinterpret_cast<int *>(self + 0x1E1D4) - *reinterpret_cast<int *>(self + 0x1E1CC),
                      reinterpret_cast<Win *>(g_007ae820), 0);
            btn->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
            reinterpret_cast<VCall *>(btn)->slot062();
            reinterpret_cast<VCall *>(btn)->slot002();
            reinterpret_cast<ButtonGroup *>(self + 0x136E4)->add(btn);
            *reinterpret_cast<int *>(self + 0x10) += 0x16;
        }
    }

    // ---- 0x004A594F: per-faction visibility flags ----
    int factionFlags[7];
    *g_0087be28 = 1;
    {
        int cursor = 0x833;
        int factionIdx = 1;
        int i = 0;
        do {
            unsigned char bit = *(reinterpret_cast<unsigned char *>(g_0096c9f8) + (cursor + *g_00939284) * 4);
            if ((bit & 1) == 0 && spying(factionIdx) == 0 && factionIdx != *g_00939284 &&
                (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) == 0) {
                factionFlags[i] = 0;
            } else {
                factionFlags[i] = 1;
            }
            cursor += 0x833;
            factionIdx++;
            i++;
        } while (cursor < 0x4198);
    }

    // ---- 0x004A59CD: per-faction "spy report" grid ----
    {
        char *statsCursor = self + 0x5334;
        int *pointCursor = reinterpret_cast<int *>(self + 0x5C);
        char *nameCursor = reinterpret_cast<char *>(g_00946fec);
        FlatButton *btn = reinterpret_cast<FlatButton *>(self + 0x80);
        for (int i = 0; i < 7; i++) {
            int factionIdx = i + 1;
            if (factionFlags[i] != 0) {
                *reinterpret_cast<char *>(g_009b86a0) = 0;
                *g_009bbfec = *reinterpret_cast<int *>(nameCursor);
                *g_009bbff0 = 0;
                strcat(reinterpret_cast<char *>(g_009b86a0), nameCursor + 0x4C);
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
                *g_009bbff0 = 0;
                *g_009bbfec = *reinterpret_cast<int *>(nameCursor);
                strcat(reinterpret_cast<char *>(g_009b86a0), nameCursor + 0x34);
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
                {
                    int sid = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x2D0);
                    int sres = reinterpret_cast<Strings *>(g_009b90d8)->get(sid);
                    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(sres));
                }
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
                *g_009bbfec = *reinterpret_cast<int *>(nameCursor + 0x2FC);
                *g_009bbff0 = *reinterpret_cast<int *>(nameCursor + 0x300);
                strcat(reinterpret_cast<char *>(g_009b86a0), nameCursor + 0x2E4);

                int curPlayer = *g_00939284;
                int modeFlag = *reinterpret_cast<int *>(self + 0x1E230);
                int bonus = (modeFlag != 0) ? 5 : 0;
                int left, top, right, bottom;
                if (factionIdx < curPlayer) {
                    top = *reinterpret_cast<int *>(self + 0x7C);
                    left = pointCursor[1];
                    right = left + bonus + 0x25;
                    bottom = top + 0x1D;
                } else if (factionIdx > curPlayer) {
                    top = *reinterpret_cast<int *>(self + 0x7C);
                    left = pointCursor[0];
                    right = left + bonus + 0x25;
                    bottom = top + 0x1D;
                } else if (modeFlag != 0) {
                    left = *reinterpret_cast<int *>(self + 0x50) + 0xF;
                    top = *reinterpret_cast<int *>(self + 0x54) + 0xF;
                    right = left + 0x46;
                    bottom = top + 0x46;
                } else {
                    left = *reinterpret_cast<int *>(self + 0x50) + 9;
                    top = *reinterpret_cast<int *>(self + 0x7C);
                    right = left + 0x25;
                    bottom = top + 0x1D;
                }
                *reinterpret_cast<int *>(self + 0x1E1C8) = left;
                *reinterpret_cast<int *>(self + 0x1E1CC) = top;
                *reinterpret_cast<int *>(self + 0x1E1D0) = right;
                *reinterpret_cast<int *>(self + 0x1E1D4) = bottom;

                btn->init(0, factionIdx + 9999, left, top, right - left, bottom - top,
                          reinterpret_cast<Win *>(g_007ae820), 0);

                int *sc = reinterpret_cast<int *>(statsCursor);
                btn->field_ADC_ = static_cast<uint32_t>(sc[1]);
                btn->field_AD4_ = static_cast<uint32_t>(sc[-1]);
                btn->field_AD8_ = static_cast<uint32_t>(sc[0]);
                btn->set_bubble_text(reinterpret_cast<char *>(g_009b86a0));
                btn->buffer_.set_font(reinterpret_cast<Font *>(g_006e84b4), 0, 0, 0);
                reinterpret_cast<VCall *>(btn)->slot062();
                reinterpret_cast<VCall *>(btn)->slot062();
            }
            nameCursor += 0x59C;
            pointCursor += 1;
            statsCursor += 0xC;
            btn = reinterpret_cast<FlatButton *>(reinterpret_cast<char *>(btn) + 0xB4C);
        }
    }

    // ---- 0x004A5C45: index-selected report window, scroll, "report" event ----
    {
        int idx1 = *g_00939284;
        int off1 = idx1 * 0x20CC - 0x14;
        *reinterpret_cast<int *>(self + off1) = 2;
        int idx2 = *g_00939284;
        FlatButton *sel = reinterpret_cast<FlatButton *>(self + idx2 * 0x20CC - 0xACC);
        reinterpret_cast<VCall *>(sel)->slot062();
    }
    *g_008846d0 = 0;
    *g_008846d4 = 0;
    *g_008846d8 = 0x20;
    *g_008846dc = 0x12C;
    reinterpret_cast<Scroll *>(g_00882584)->init(reinterpret_cast<RECT *>(g_008846d0), reinterpret_cast<Win *>(g_00876478), 0, 0);
    reinterpret_cast<Win *>(g_00882584)->hide();
    *reinterpret_cast<int *>(self + 0x1E234) = 0;
    reinterpret_cast<ReportIf *>(g_00885f38)->report(*g_00885f40, *g_008a4160, *g_008a4164);
    slide_show(reinterpret_cast<GraphicWin *>(g_00876478), 0);
    reinterpret_cast<Win *>(g_00876478)->show(3);
    *g_008a4170 = 0;
    if (*g_008846e4 != 0) {
        scroll_show(reinterpret_cast<GraphicWin *>(g_00876478), reinterpret_cast<Scroll *>(g_00882584));
        *g_008846e4 = 0;
        *g_00882570 = 1;
    }

    if (*g_009b7ae0 != 0x876478) {
        *g_008a6240 = 1;
        reinterpret_cast<VCall *>(g_00876478)->slot057(0, g_005398e0, g_007ae820);
    }
}
