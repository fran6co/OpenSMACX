// ORIGINAL: 0x0051BC70 FILE
// RULED-OUT: full transcription of every case in the ~150-target dispatch
//            (mnemonic_similarity 0.45, 834/1610 original mnemonics in
//            common); first divergence is the Win::set_cursor call through
//            the vbtable adjustor near the top loop (mov/add ordering), and
//            downstream case bodies pick up extra instructions from real
//            vtable dispatch (VCall) and MapWin/Console member calls the
//            original inlined more cheaply than /O2 does here.
// working copy - scaffold materialised by --work
// name      ?menu_proc@Console@@QAEXH@Z
// size      5582 bytes
// spans     0x0051BC70-0x0051D23E
// prototype void (__thiscall ?menu_proc@Console@@QAEXH@Z)(Console* this, int)
// callers   1   call targets   148
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00428DF0 0x00446B40 0x0044AE50 0x0044CB60 0x00458900 0x00460DD0 0x00467970 0x0046A550 0x0046AD60 0x0046AE00 0x0046B190 0x0046B1F0 0x0046B310 0x0046B510 0x0046FB10 0x00476A50 0x00477F00 0x00479970 0x00479C00 0x0047A890 0x0048C0A0 0x004AC9A0 0x004AC9F0 0x004ACA40 0x004ACA90 0x004ACAE0 0x004ACB30 0x004ACB80 0x004B9EA0 0x004B9F90 0x004BC6E0 0x004BDDE0 0x004CF7C0 0x004D0450 0x004D04A0 0x004D04E0 0x004D06C0 0x004D2220 0x004D24C0 0x004D26A0 0x004D2AC0 0x004D2DE0 0x004D2F00 0x004D3470 0x004D52A0 0x004D5D30 0x004D61A0 0x004D7BE0 0x004D7CF0 0x004D7DE0 0x004D7EE0 0x004D7F60 0x004D8A70 0x004D8F40 0x004D8FA0 0x004D9170 0x004D95F0 0x004D9970 0x004DB870 0x004DBB40 0x004DBC40 0x004DBD20 0x004DBFB0 0x004DC230 0x004DC520 0x004DC860 0x004DCCC0 0x004DD1B0 0x004DD6F0 0x004DDA50 0x004DED00 0x004DF4F0 0x004DFAD0 0x004E0120 0x004E0210 0x004E0290 0x004E05E0 0x004E09B0 0x004E09E0 0x004E0A00 0x004E0A50 0x004E0AD0 0x004E0B00 0x004E0EA0 0x004E0F30 0x004E0F80 0x004E0FA0 0x004E0FD0 0x004E10C0 0x004E1190 0x004E1200 0x004E1290 0x004E1300 0x004E1380 0x004E13E0 0x004E14A0 0x004E14D0 0x004E1F20 0x004E1F30 0x004E1F40 0x004E1F70 0x004E2000 0x0050BA30 0x0050EF50 0x00510B70 0x00511520 0x00514B30 0x00514EF0 0x00514F00 0x00514F10 0x00514F20 0x00514F30 0x00514F40 0x00515010 0x005150D0 0x00515450 0x00515770 0x00515900 0x00515F70 0x005162A0 0x00516490 0x005165B0 0x005169F0 0x00516C50 0x00517060 0x0051BBA0 0x0051DD00 0x0052E050 0x00583CB0 0x0058EE50 0x0058EE60 0x00593310 0x0059E5D0 0x005A9EB0 0x005ABD20 0x005ADBF0 0x005B5620 0x005BE530 0x005BF310 0x005BF3F0 0x005D5A70 0x005EC7C0 0x005FB760 0x005FB7C0 0x005FCA30 0x00625EC0 0x006262F0 0x006263F0
// indirect  0x0051C2A1 0x0051CEB3

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?menu_proc@Console@@QAEXH@Z  at 0x0051BC70  (5582 bytes)
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
class BasePop;
class Buffer;
class ButtonGroup;
class Console;
class FlatButton;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
class MainMenu;
class MapWin;
class Menu;
struct MenuEntry;
class MultiWin;
class NetDaemon;
class NetMsg;
typedef void * PVOID;
class Popup;
class PullDown;
struct PullDownItem;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StatusWin;
class Time;
class TutWin;
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
    int set_cursor(int);
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

class BasePop : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    Heap heap_;
    uint32_t field_A3C_;
    uint32_t field_A40_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    uint32_t field_A4C_;
    uint32_t field_A50_;
    uint32_t field_A54_;
    uint32_t field_A58_;
    FlatButton flat_button1_;
    FlatButton flat_button2_;
    uint32_t field_20F4_;
    uint32_t field_20F8_;
    uint32_t field_20FC_;
    uint32_t field_2100_;
    uint32_t field_2104_;
    uint32_t field_2108_;
    uint32_t field_210C_;
    uint32_t field_2110_;
    uint32_t field_2114_;
    Sprite sprite_;
    uint32_t field_2144_;
    uint32_t field_2148_;
    uint32_t field_214C_;
    uint32_t field_2150_;
    uint32_t field_2154_;
    uint32_t field_2158_;
    uint32_t field_215C_;
    uint32_t field_2160_;
    uint32_t field_2164_;
    uint32_t field_2168_;
    uint32_t field_216C_;
    uint32_t field_2170_;
    uint32_t field_2174_;
    uint32_t field_2178_;
    uint32_t field_217C_;
    uint32_t field_2180_;
    uint32_t field_2184_;
    uint32_t field_2188_;
    uint32_t field_218C_;
    uint32_t field_2190_;
    uint32_t field_2194_;
    uint32_t field_2198_;
    uint32_t field_219C_;
    uint32_t field_21A0_;
    uint32_t field_21A4_;
    uint32_t field_21A8_;
    uint32_t field_21AC_;
    uint32_t field_21B0_;
    uint32_t field_21B4_;
    uint32_t field_21B8_;
    uint32_t field_21BC_;
    uint32_t field_21C0_;
    uint32_t field_21C4_;
    uint32_t field_21C8_;
    uint32_t field_21CC_;
    uint8_t dialogs_[0xC94];
    uint32_t field_2E64_;
    uint32_t field_2E68_;
    uint32_t field_2E6C_;
    uint32_t field_2E70_;
    uint32_t field_2E74_;
    uint32_t field_2E78_;
    uint32_t field_2E7C_;
    uint32_t field_2E80_;
    uint32_t field_2E84_;
    uint32_t field_2E88_;
    uint32_t field_2E8C_;
    uint32_t field_2E90_;
    uint32_t field_2E94_;
    uint32_t field_2E98_;
    uint32_t field_2E9C_;
    uint32_t field_2EA0_;
    uint32_t field_2EA4_;
    uint32_t field_2EA8_;
    uint32_t field_2EAC_;
    uint32_t field_2EB0_;
    uint32_t field_2EB4_;
    uint32_t field_2EB8_;
    uint32_t field_2EBC_;
    uint32_t field_2EC0_;
    uint32_t field_2EC4_;
    uint32_t field_2EC8_;
    uint32_t field_2ECC_;
    uint32_t field_2ED0_;
    uint32_t field_2ED4_;
    uint32_t field_2ED8_;
    uint32_t field_2EDC_;
    uint32_t field_2EE0_;
    uint32_t field_2EE4_;
    uint32_t field_2EE8_;
    uint32_t field_2EEC_;
    uint32_t field_2EF0_;
    uint32_t field_2EF4_;
    uint32_t field_2EF8_;
    uint32_t field_2EFC_;
    uint32_t field_2F00_;
    uint32_t field_2F04_;
    uint32_t field_2F08_;
    uint32_t field_2F0C_;
    uint32_t field_2F10_;
    uint32_t field_2F14_;
    uint32_t field_2F18_;
    uint32_t field_2F1C_;
    uint32_t field_2F20_;
    uint32_t field_2F24_;
    uint32_t field_2F28_;
    uint32_t field_2F2C_;
    uint32_t field_2F30_;
    uint32_t field_2F34_;
    uint32_t field_2F38_;
    uint32_t field_2F3C_;
    uint32_t field_2F40_;
    uint32_t field_2F44_;
    uint32_t field_2F48_;
    uint32_t field_2F4C_;
    uint32_t field_2F50_;
    uint32_t field_2F54_;
    uint32_t field_2F58_;
    uint32_t field_2F5C_;
    uint32_t field_2F60_;
    uint32_t field_2F64_;
    uint32_t field_2F68_;
    uint32_t field_2F6C_;
    uint32_t field_2F70_;
    uint32_t field_2F74_;
    uint32_t field_2F78_;
    uint32_t field_2F7C_;
    uint32_t field_2F80_;
    uint32_t field_2F84_;
    uint32_t field_2F88_;
    uint32_t field_2F8C_;
    uint32_t field_2F90_;
    uint32_t field_2F94_;
    uint32_t field_2F98_;
    uint32_t field_2F9C_;
    uint32_t field_2FA0_;
    uint32_t field_2FA4_;
    uint32_t field_2FA8_;
    uint32_t field_2FAC_;
    uint32_t field_2FB0_;
    uint32_t field_2FB4_;
    uint32_t field_2FB8_;
    uint32_t field_2FBC_;
    uint32_t field_2FC0_;
    uint32_t field_2FC4_;
    uint32_t field_2FC8_;
    uint32_t field_2FCC_;
    uint32_t field_2FD0_;
    uint32_t field_2FD4_;
    uint32_t field_2FD8_;
    uint32_t field_2FDC_;
    uint32_t field_2FE0_;
    uint32_t field_2FE4_;
    uint32_t field_2FE8_;
    uint32_t field_2FEC_;
    uint32_t field_2FF0_;
    uint32_t field_2FF4_;
    uint32_t field_2FF8_;
    uint32_t field_2FFC_;
    uint32_t field_3000_;
    uint32_t field_3004_;
    uint32_t field_3008_;
    uint32_t field_300C_;
    uint32_t field_3010_;
    uint32_t field_3014_;
    uint32_t field_3018_;
    uint32_t field_301C_;
    uint32_t field_3020_;
    uint32_t field_3024_;
    uint32_t field_3028_;
    uint32_t field_302C_;
    uint32_t field_3030_;
    uint32_t field_3034_;
    uint32_t field_3038_;
    uint32_t field_303C_;
    uint32_t field_3040_;
    uint32_t field_3044_;
    uint32_t field_3048_;
    uint32_t field_304C_;
    uint32_t field_3050_;
    uint32_t field_3054_;
    uint32_t field_3058_;
    uint32_t field_305C_;
    uint32_t field_3060_;
    uint32_t field_3064_;
    uint32_t field_3068_;
    uint32_t field_306C_;
    uint32_t field_3070_;
    uint32_t field_3074_;
    uint32_t field_3078_;
    uint32_t field_307C_;
    uint32_t field_3080_;
    uint32_t field_3084_;
    uint32_t field_3088_;
    uint32_t field_308C_;
    uint32_t field_3090_;
    uint32_t field_3094_;
    Spot spot_;
    uint32_t field_30A4_;
    uint32_t field_30A8_;
    uint32_t field_30AC_;
    uint32_t field_30B0_;
    uint32_t field_30B4_;
    uint32_t field_30B8_;
    uint32_t field_30BC_;
    uint32_t field_30C0_;
    uint32_t field_30C4_;
    uint32_t field_30C8_;
    uint32_t field_30CC_;
    uint32_t field_30D0_;
    uint32_t field_30D4_;
    uint32_t field_30D8_;
    uint32_t field_30DC_;
    uint32_t field_30E0_;
    uint32_t field_30E4_;
    uint32_t field_30E8_;
    uint32_t field_30EC_;
    uint32_t field_30F0_;
    uint32_t field_30F4_;
    uint32_t loc_a_;
    uint32_t loc_b_;
    uint32_t field_3100_;
    uint32_t field_3104_;
    uint32_t field_3108_;
    uint32_t field_310C_;
    Font * string_font1_;
    Font * string_font2_;
    Font * string_font3_;
    Font * string_font4_;
    int string_color_a_;
    int string_color_2a_;
    int string_color_3a_;
    int string_color_hyper_a_;
    int string_color_b_;
    int string_color_2b_;
    int string_color_3b_;
    int string_color_hyper_b_;
    int string_color_c_;
    int string_color_2c_;
    int string_color_3c_;
    int string_color_hyperc_;
    int string_color_d_;
    int string_color_2d_;
    int string_color_3d_;
    int string_color_hyper_d_;
    uint32_t field_3160_;
    uint32_t field_3164_;
    uint32_t field_3168_;
    Font * button_font1_;
    Font * button_font2_;
    Font * button_font3_;
    uint8_t button_color_a_;
    uint8_t button_color_2a_;
    uint8_t button_color_3a_;
    uint8_t field_317B_;
    uint32_t button_color_b_;
    uint32_t button_color_2b_;
    uint32_t button_color_3b_;
    uint32_t button_color_c_;
    uint32_t button_color_2c_;
    uint32_t button_color_3c_;
    uint32_t button_color_d_;
    uint32_t button_color_2d_;
    uint32_t button_color_3d_;
    uint8_t field_31A0_;
    uint8_t field_31A1_;
    uint8_t field_31A2_;
    uint8_t field_31A3_;
    uint32_t field_31A4_;
    uint32_t field_31A8_;
    uint32_t field_31AC_;
    uint32_t field_31B0_;
    uint32_t field_31B4_;
    uint32_t field_31B8_;
    uint32_t field_31BC_;
    uint32_t field_31C0_;
    uint32_t field_31C4_;
    uint32_t field_31C8_;
    uint32_t field_31CC_;
    uint32_t field_31D0_;
    uint32_t field_31D4_;
    uint32_t field_31D8_;
    uint32_t field_31DC_;
    uint32_t field_31E0_;
    uint32_t field_31E4_;
    uint32_t field_31E8_;
    uint32_t field_31EC_;
    uint32_t field_31F0_;
    uint32_t field_31F4_;
    uint32_t field_31F8_;
    uint32_t field_31FC_;
    uint32_t field_3200_;
    uint32_t field_3204_;
    uint32_t field_3208_;
    uint32_t field_320C_;
    uint32_t field_3210_;
    uint32_t field_3214_;
    uint32_t field_3218_;
    uint32_t field_321C_;
    uint32_t field_3220_;
    LPSTR ok_text_;
    LPSTR cancel_text_;
    uint32_t field_322C_;
    int exec();
    static void fallout();
    uint32_t read_check();
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

class InfoWin { public:
    void UNK1();
    void change(int);
    void on_redraw();
    int reset();
};

class MainMenu { public:
    void check(int);
};

class MapWin { public:
    void * __vbptr;
    void * owned_;
    uint8_t field_8_[0x1DD64];
    uint32_t field_1DD6C_;
    uint32_t field_1DD70_;
    uint32_t field_1DD74_;
    uint32_t field_1DD78_;
    uint8_t field_1DD7C_[0x4];
    uint32_t field_1DD80_;
    uint32_t field_1DD84_;
    uint32_t field_1DD88_;
    uint32_t field_1DD8C_;
    uint32_t field_1DD90_;
    uint32_t field_1DD94_;
    uint32_t field_1DD98_;
    uint32_t field_1DD9C_;
    uint32_t field_1DDA0_;
    uint32_t field_1DDA4_;
    uint32_t field_1DDA8_;
    uint32_t field_1DDAC_;
    uint32_t field_1DDB0_;
    uint32_t field_1DDB4_;
    uint32_t field_1DDB8_;
    uint32_t field_1DDBC_;
    uint32_t field_1DDC0_;
    uint32_t field_1DDC4_;
    uint32_t field_1DDC8_;
    uint32_t field_1DDCC_;
    uint32_t field_1DDD0_;
    uint32_t field_1DDD4_;
    uint32_t field_1DDD8_;
    uint32_t field_1DDDC_;
    uint32_t field_1DDE0_;
    uint8_t field_1DDE4_[0x40];
    uint32_t field_1DE24_;
    uint8_t field_1DE28_[0x1098];
    uint32_t field_1EEC0_;
    uint32_t field_1EEC4_;
    uint8_t field_1EEC8_[0x28];
    uint32_t field_1EEF0_;
    uint32_t field_1EEF4_;
    uint8_t field_1EEF8_[0x8];
    uint32_t field_1EF00_;
    uint32_t field_1EF04_;
    uint8_t field_1EF08_[0x14];
    uint32_t field_1EF1C_;
    uint8_t field_1EF20_[0x28];
    uint32_t field_1EF48_;
    uint32_t field_1EF4C_;
    uint32_t field_1EF50_;
    uint8_t field_1EF54_[0x2AF0];
    uint32_t field_21A44_;
    uint32_t field_21A48_;
    uint32_t field_21A4C_;
    uint32_t field_21A50_;
    uint32_t field_21A54_;
    uint32_t field_21A58_;
    uint32_t field_21A5C_;
    uint32_t field_21A60_;
    uint32_t field_21A64_;
    int32_t field_21A68_;
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
    int UNK1();
    int UNK2();
    int focus(int, int);
    void UNK3();
    void close();
    void do_image_buttons();
    void draw_base_dest(int);
    void draw_cursor();
    void draw_map(int);
    void editor(int, int, int);
    void main_caption();
    void on_left_double_click(int, int);
    void on_left_up(int, int);
    void on_redraw();
    void set_center(int, int, int);
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
    int check_menu_item(int, int);
    int requested_height();
    int uncheck_menu_item(int, int);
};

class MultiWin { public:
    void hide();
    void on_button_clicked(int);
    void show();
};

class NetDaemon { public:
    int receive();
    uint32_t unlock_veh();
    void hang_up();
};

class NetMsg { public:
    void close();
    void pop(const char *, int, int, const char *);
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

class Popup : public BasePop { public:
    Scroll scroll_;
    void close();
    void on_adjust_button_width();
    void on_redraw_nc(RECT *, int);
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

class StatusWin { public:
    void close();
    void redraw();
    void reset();
    void set_loc(int, int);
};

class TutWin : public Popup { public:
    int32_t field_537C_;
    int32_t field_5380_;
    uint32_t field_5384_;
    uint32_t field_5388_;
    uint32_t field_538C_;
    uint32_t field_5390_;
    uint32_t field_5394_;
    uint32_t field_5398_;
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t field_53B0_[0x4];
    uint32_t field_53B4_;
    int32_t field_53B8_;
    uint32_t field_53BC_;
    uint32_t field_53C0_;
    int32_t field_53C4_;
    uint32_t field_53C8_;
    uint32_t field_53CC_;
    uint32_t field_53D0_;
    int32_t field_53D4_;
    int reset();
    void UNK1();
    void tour();
};

extern "C" int __cdecl sub_51dd00();
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl X_pop(const char *, int, int (__cdecl *)());
int ambience(int);
int bit_count(int);
int desktop_init(int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int save_game(int);
int tech_advance(int);
void auto_save();
void design_new_veh(int, int);
void desktop_update();
void do_non_input();
void __cdecl draw_cursor();
void __cdecl draw_map(int);
void go_reset();
void hall_of_fame(int);
void help_topic(unsigned int, int);
void log_say(char *, int, int, int);
void log_say_hex(char *, int, int, int);
void monument(int);
void prefs_save(int);
void report_energy();
void report_labs();
void report_ops();
void report_projects();
void report_sat();
void report_score(int);
void report_security();
void show_credits();
void show_replay();
void social_select(int);
void synch_ai(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of
// a slot are yours to set and setting them does not move it: write
// `virtual int slot074(int, int);` if that is the call you need.
// The slots below are spelled nullary because the emitter reads the
// vtable OFFSET from the body and not the argument list.
// This body dispatches through slot(s): 1, 2
class VCall { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_0051d240 = (int *)0x0051D240;
static int *const g_0051d274 = (int *)0x0051D274;
static int *const g_0051d2b8 = (int *)0x0051D2B8;
static int *const g_0051d324 = (int *)0x0051D324;
static int *const g_0051d368 = (int *)0x0051D368;
static int *const g_0051d394 = (int *)0x0051D394;
static int *const g_0051d488 = (int *)0x0051D488;
static int *const g_0051d4f4 = (int *)0x0051D4F4;
static int *const g_0051d504 = (int *)0x0051D504;
static int *const g_0051d530 = (int *)0x0051D530;
static int *const g_0051d54c = (int *)0x0051D54C;
static int *const g_0051d578 = (int *)0x0051D578;
static int *const g_0051d634 = (int *)0x0051D634;
static int *const g_0051d658 = (int *)0x0051D658;
static int *const g_0051d6c8 = (int *)0x0051D6C8;
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_0068a7d4 = (int *)0x0068A7D4;
static int *const g_0068a7e4 = (int *)0x0068A7E4;
static int *const g_0068a7f0 = (int *)0x0068A7F0;
static int *const g_0068a8c8 = (int *)0x0068A8C8;
static int *const g_0068a9c0 = (int *)0x0068A9C0;
static int *const g_0068a9cc = (int *)0x0068A9CC;
static int *const g_0068aa7c = (int *)0x0068AA7C;
static int *const g_0068aa8c = (int *)0x0068AA8C;
static int *const g_0068ad54 = (int *)0x0068AD54;
static int *const g_0068ad60 = (int *)0x0068AD60;
static int *const g_0068ad70 = (int *)0x0068AD70;
static int *const g_0068ad7c = (int *)0x0068AD7C;
static int *const g_00691870 = (int *)0x00691870;
static int *const g_00691874 = (int *)0x00691874;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_007ad2a0 = (int *)0x007AD2A0;
static int *const g_007b0cb8 = (int *)0x007B0CB8;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_007fd648 = (int *)0x007FD648;
static int *const g_007fe06c = (int *)0x007FE06C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_008cc228 = (int *)0x008CC228;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a940 = (int *)0x0093A940;
static int *const g_0093a944 = (int *)0x0093A944;
static int *const g_0093a948 = (int *)0x0093A948;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_0094ae68 = (int *)0x0094AE68;
static int *const g_0094b464 = (int *)0x0094B464;
static int *const g_0094b468 = (int *)0x0094B468;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_0096cdc0 = (int *)0x0096CDC0;
static int *const g_0096cdc4 = (int *)0x0096CDC4;
static int *const g_0096cdc8 = (int *)0x0096CDC8;
static int *const g_0096cdcc = (int *)0x0096CDCC;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab88d = (int *)0x009AB88D;
static int *const g_009b2068 = (int *)0x009B2068;
static int *const g_009bc06c = (int *)0x009BC06C;

class Console : public MapWin { public:
    uint8_t field_21A6C_[0xFC0];
    uint32_t field_22A2C_;
    uint8_t field_22A30_[0x440];
    uint32_t field_22E70_;
    uint8_t field_22E74_[0xD54];
    uint32_t field_23BC8_;
    uint32_t field_23BCC_;
    uint32_t field_23BD0_;
    uint32_t field_23BD4_;
    uint32_t field_23BD8_;
    uint32_t field_23BDC_;
    uint32_t field_23BE0_;
    uint32_t field_23BE4_;
    uint32_t field_23BE8_;
    uint32_t field_23BEC_;
    uint32_t field_23BF0_;
    uint32_t field_23BF4_;
    uint32_t field_23BF8_;
    uint32_t field_23BFC_;
    uint32_t field_23C00_;
    uint32_t field_23C04_;
    uint32_t field_23C08_;
    uint32_t field_23C0C_;
    uint8_t field_23C10_[0x100];
    uint32_t field_23D10_;
    uint32_t field_23D14_;
    uint32_t field_23D18_;
    uint32_t field_23D1C_;
    uint8_t field_23D20_[0x4];
    uint32_t field_23D24_;
    uint8_t field_23D28_[0x58];
    uint32_t field_23D80_;
    uint32_t field_23D84_;
    uint32_t field_23D88_;
    uint32_t field_23D8C_;
    int32_t field_23D90_;

    void new_base(int);
    void skip(int);
    void wait(int);
    void unload(int);
    int upgrade(int);
    void sentry(int, int);
    void hold(int, int);
    void give_veh(int);
    void disband(int);
    void disband(int, int);
    void activate(int, int, int);
    void terraform(int, int);
    void destroy(int);
    void oblit(int);
    void go_to(int, int, int);
    void road_to(int, int);
    void designate(int);
    void go_home(int);
    void set_patrol(int);
    void use_ability(int);
    void automate(int, int);
    void explore(int);
    void convoy(int);
    void arty(int);
    void assemble();
    void editor_on();
    void editor_diplomacy();
    void editor_reset_faction();
    void editor_reset_tech();
    void editor_personality();
    void editor_strategy();
    void editor_rules();
    void editor_scen_rules();
    void editor_scen_victory();
    void editor_scen_param();
    void custom_my_faction();
    void editor_diff();
    void editor_edit_veh(int);
    void editor_veh();
    void editor_view();
    void editor_tech();
    void editor_energy();
    void editor_date();
    void editor_reload();
    void editor_eliminate();
    void editor_kill_vehicles();
    void editor_save();
    void editor_load();
    void editor_paints(int);
    void editor_minor();
    void editor_terrain();
    void editor_set_brush();
    void editor_flat();
    void editor_polar();
    void editor_climate();
    void editor_generate();
    void editor_fast();
    void editor_randomize();
    void editor_clear_random();
    void editor_clear();
    void editor_clear_vehicles();
    void editor_clear_terrain();
    void editor_fungus();
    void editor_rockiness();
    void editor_natural(int);
    void editor_undo();
    void editor_redo();
    int edit_lock();
    void editor_lock2();
    void corner_the_market();
    void next_cursor();
    void set_move(int);
    void retire_game();
    void set_preferences();
    void set_adv_preferences();
    void set_auto_preferences();
    void set_base_preferences();
    void set_audiovisual();
    void set_map_display();
    void set_tech(int);
    void zoom(int, int);
    void survey();
    void chat(int);
    void time_controls();
    void show_movies();
    void name_landmark(int, int);
    void erase_landmark(int, int);
    void zoom_to_upkeep_base();
    void end_my_turn();
    void show_scen_objectives();
    void use_time_bonus();
    void load_proc(int, int);
    void on_sys_close();
    void menu_update();
    void clear_group();
    void menu_proc(int);
};
static void Console_reset_turn_view(Console *self) {
    ambience(0x1e);
    self->field_23BD8_ = 0;
    self->field_23BF8_ = 1;
    self->field_23BFC_ = 1;
    if (self->field_23C00_ != 0) {
        self->field_23C00_ = 0;
        ((MapWin *)*g_007d3c3c)->draw_map(1);
    }
    draw_cursor();
    ((InfoWin *)g_007ad2a0)->change(0);
    ((StatusWin *)g_008c5568)->redraw();
    ((MapWin *)*g_007d3c3c)->main_caption();
    ((MainMenu *)&self->field_22A2C_)->check(self->field_23BDC_);
}

void Console::menu_proc(int a1) {
    if (*g_0093a940 == 0) {
        *g_0093a944 = 1;
        return;
    }
    if (*g_009b2068 != 0) {
        return;
    }
    ((TutWin *)g_008c6e68)->reset();
    go_reset();
    this->field_23D10_ = 0;

    {
        MapWin **table = (MapWin **)g_007d3c3c;
        for (MapWin **p = table; (int)p < (int)g_007d3c5c; ++p) {
            MapWin *mw = *p;
            if (mw != 0 && (p == table || mw->field_1DD74_ != 0)) {
                if (mw->field_1EF4C_ != 0 || mw->field_1EF50_ != 0x7f00) {
                    void *vbtable = *(void **)mw;
                    int adjustor = *(int *)((char *)vbtable + 4);
                    typedef int (OriginalObject::*SetCursorFn)(int);
                    SetCursorFn set_cursor = original_method<SetCursorFn>(0x005EC7C0);
                    (ORIGINAL((char *)mw + adjustor)->*set_cursor)(0x7f00);
                    mw->field_1EF50_ = 0x7f00;
                    mw->field_1EF4C_ = 0;
                }
                uint32_t flags = mw->field_1DD70_;
                if (flags & 0xc40000) {
                    mw->field_1DD70_ = flags & 0xff3bffff;
                    mw->draw_map(1);
                }
                do_non_input();
            }
        }
    }

    if (*g_0093f660 != 0) {
        int hi = (a1 >> 8) & 0xff;
        if ((((*g_0093a938 != 0) || (*g_0093e944 != 0)) || (*g_0093e8c8 != 0)) &&
            hi != 0 && hi != 7 && hi != 8) {
            return;
        }
        log_say_hex((char *)g_0068ad54, a1, 0, 0);
    }

    if (*g_00939288 != 0) {
        if (this->edit_lock() != 0) {
            Console_reset_turn_view(this);
            return;
        }
        if (this->field_23BDC_ < 0 && a1 != 0x5d) {
            return;
        }
    }

    if (0x8ff < a1 && a1 < 0x913) {
        this->terraform(this->field_23BDC_, a1 - 0x900);
    }

    if (0x103 < a1) {
        if (a1 < 0x201) {
            if (a1 == 0x200) {
                if (this->edit_lock() == 0) {
                    this->set_move(0);
                } else {
                    ((NetMsg *)g_00805338)->pop((const char *)g_0068ad70, 5000, 0, 0);
                }
                goto switchD_0051be9e_default;
            }
            switch (a1) {
            case 0x105:
                social_select(*g_00939284);
                break;
            case 0x106: {
                if ((*g_009a649c & 0x200) != 0) {
                    int off = *g_00939284 * 0x20cc;
                    int flags = (*(int *)((char *)g_0096cdc0 + off) != 0) ? 1 : 0;
                    if (*(int *)((char *)g_0096cdc4 + off) != 0) flags |= 2;
                    if (*(int *)((char *)g_0096cdc8 + off) != 0) flags |= 4;
                    if (*(int *)((char *)g_0096cdcc + off) != 0) flags |= 8;
                    *g_009bc06c = flags;
                    int r = X_pop((const char *)g_0068a8c8, 1, 0);
                    if (r >= 0) {
                        unsigned char c = (unsigned char)*g_009bc06c;
                        *(int *)((char *)g_0096cdc0 + off) = c & 1;
                        *(int *)((char *)g_0096cdc4 + off) = (c & 2) >> 1;
                        *(int *)((char *)g_0096cdc8 + off) = (c & 4) >> 2;
                        *(int *)((char *)g_0096cdcc + off) = (c & 8) >> 3;
                        synch_ai(*g_00939284);
                    }
                }
                break;
            }
            case 0x107:
                if ((*g_009a649c & 0x200) == 0) {
                    this->set_tech(*g_00939284);
                }
                break;
            case 0x108:
                goto switchD_0051c15a_caseD_108;
            case 0x109:
                report_labs();
                break;
            case 0x10a:
                report_energy();
                break;
            case 0x10b:
                report_ops();
                break;
            case 0x10c:
                report_projects();
                break;
            case 0x10d:
                report_sat();
                break;
            case 0x10e:
                report_security();
                break;
            case 0x10f:
                report_score(0);
                break;
            case 0x110:
                monument(0);
                break;
            case 0x111:
                hall_of_fame(0);
                break;
            case 0x113:
                if (*g_007fe06c == 0) {
                    ((MultiWin *)g_007fd648)->show();
                } else {
                    ((MultiWin *)g_007fd648)->hide();
                }
                break;
            case 0x114:
                this->corner_the_market();
                break;
            case 0x115:
                this->show_scen_objectives();
                break;
            }
        } else {
            if (a1 < 0x324) {
                if (a1 == 0x323) {
                    this->skip(this->field_23BDC_);
                } else if (a1 < 0x308) {
                    if (a1 == 0x307) {
                        this->automate(this->field_23BDC_, 0xb);
                    } else if (a1 < 0x213) {
                        if (a1 == 0x212) {
                            this->erase_landmark(-1, -1);
                        } else {
                            switch (a1) {
                            case 0x201:
                                if (*g_00939288 != 0) {
                                    Console_reset_turn_view(this);
                                }
                                break;
                            case 0x203:
                            case 0x204:
                            case 0x205:
                            case 0x206:
                            case 0x207:
                            case 0x208:
                                this->zoom(a1, 0);
                                break;
                            case 0x209: {
                                *g_009a6494 ^= 0x10000;
                                Menu *menu = (Menu *)&this->field_22A2C_;
                                if ((*g_009a6494 & 0x10000) == 0) menu->uncheck_menu_item(2, 0x209);
                                else menu->check_menu_item(2, 0x209);
                                *g_0094b468 = *g_009a6494;
                                prefs_save(0);
                                draw_map(1);
                                break;
                            }
                            case 0x20a: {
                                *g_009a6490 ^= 0x100;
                                Menu *menu = (Menu *)&this->field_22A2C_;
                                if ((*g_009a6490 & 0x100) == 0) menu->uncheck_menu_item(2, 0x20a);
                                else menu->check_menu_item(2, 0x20a);
                                *g_0094b464 = *g_009a6490;
                                prefs_save(0);
                                draw_map(1);
                                break;
                            }
                            case 0x20b: {
                                *g_009a6490 ^= 0x200;
                                Menu *menu = (Menu *)&this->field_22A2C_;
                                if ((*g_009a6490 & 0x200) == 0) menu->uncheck_menu_item(2, 0x20b);
                                else menu->check_menu_item(2, 0x20b);
                                *g_0094b464 = *g_009a6490;
                                prefs_save(0);
                                draw_map(1);
                                break;
                            }
                            case 0x20c: {
                                MapWin *mw0 = (MapWin *)*g_007d3c3c;
                                mw0->field_1DD70_ ^= 0x40000;
                                draw_map(1);
                                break;
                            }
                            case 0x20d:
                                *g_00939ed8 = 0;
                                desktop_init(1);
                                break;
                            case 0x20f:
                                this->survey();
                                break;
                            case 0x210: {
                                int v;
                                if ((*g_009a6494 & 0x1000) == 0) {
                                    if ((*g_009a6494 & 0x2000) == 0) v = *g_009a6494 | 0x2000;
                                    else v = *g_009a6494 | 0x1000;
                                } else {
                                    v = *g_009a6494 & 0xffffcfff;
                                }
                                *g_0094b468 = v;
                                *g_009a6494 = v;
                                prefs_save(0);
                                draw_map(1);
                                break;
                            }
                            case 0x211:
                                this->name_landmark(-1, -1);
                                break;
                            }
                        }
                    } else {
                        switch (a1) {
                        case 0x213:
                            this->go_to(-1, -2, 0);
                            break;
                        case 0x214:
                            ((MapWin *)*g_007d3c3c)->set_center(g_009392c0[*g_009392b8], g_00939340[*g_009392b8], 1);
                            break;
                        case 0x215: {
                            MapWin **tbl = (MapWin **)g_007d3c3c;
                            MapWin *first = tbl[0];
                            unsigned int base_flag = first->field_1DD70_ & 0x400000;
                            for (MapWin **q = tbl; (int)q < (int)g_007d3c5c; ++q) {
                                MapWin *mw = *q;
                                if (mw != 0 && (q == tbl || mw->field_1DD74_ != 0)) {
                                    unsigned int flags = mw->field_1DD70_;
                                    if (base_flag == 0) {
                                        mw->field_1DD70_ = flags | 0x400000;
                                        mw->draw_base_dest(1);
                                    } else {
                                        mw->field_1DD70_ = flags & 0xffbfffff;
                                        mw->draw_map(1);
                                    }
                                }
                            }
                            break;
                        }
                        case 0x216:
                            this->field_23BF8_ = 0;
                            if (this->field_23C00_ != 0) {
                                this->field_23C00_ = 0;
                                ((MapWin *)*g_007d3c3c)->draw_map(1);
                            }
                            ((MapWin *)*g_007d3c3c)->draw_cursor();
                            if (*g_00939288 != 0) {
                                Console_reset_turn_view(this);
                            }
                            {
                                int v = this->field_23C08_ - 1;
                                this->field_23C08_ = v;
                                if (v < 0) this->field_23C08_ = 0x1f;
                            }
                            {
                                int idx = *g_009392b8;
                                int x = g_00939340[idx];
                                int y = g_009392c0[idx];
                                ((StatusWin *)g_008c5568)->set_loc(x, y);
                            }
                            {
                                int idx = *g_009392b8;
                                int x = g_00939340[idx];
                                int y = g_009392c0[idx];
                                ((MapWin *)*g_007d3c3c)->focus(x, y);
                            }
                            break;
                        case 0x217:
                            this->next_cursor();
                            break;
                        case 0x300:
                        case 0x301:
                            this->new_base(this->field_23BDC_);
                            break;
                        case 0x302:
                            this->oblit(this->field_23BDC_);
                            break;
                        case 0x304: {
                            int veh_id = this->field_23BDC_;
                            int x = *(short *)((char *)g_00952832 + veh_id * 0x34);
                            int base = x * 0x34;
                            unsigned char armor_idx = *(unsigned char *)((char *)g_009ab88c + base);
                            int row = armor_idx * 144;
                            if (*(unsigned char *)((char *)g_0094a379 + row) == 2) {
                                unsigned char weapon_idx = *(unsigned char *)((char *)g_009ab88d + base);
                                if (*(unsigned char *)((char *)g_0094ae68 + weapon_idx * 16) != 0) {
                                    this->automate(veh_id, 0xc);
                                    break;
                                }
                            }
                            this->automate(veh_id, 3);
                            break;
                        }
                        case 0x305:
                            this->arty(this->field_23BDC_);
                            break;
                        case 0x306:
                            this->automate(this->field_23BDC_, 10);
                            break;
                        }
                    }
                } else {
                    switch (a1) {
                    case 0x308:
                        this->convoy(this->field_23BDC_);
                        break;
                    case 0x309:
                        this->use_ability(this->field_23BDC_);
                        break;
                    case 0x30a:
                        this->use_ability(this->field_23BDC_);
                        break;
                    case 0x310:
                        this->go_to(this->field_23BDC_, -3, 0);
                        break;
                    case 0x311:
                        this->destroy(this->field_23BDC_);
                        break;
                    case 0x312:
                        if (*g_00939288 == 0) {
                            this->disband(g_00939340[*g_009392b8], g_009392c0[*g_009392b8]);
                        } else {
                            this->disband(this->field_23BDC_);
                        }
                        break;
                    case 0x313:
                        if (this->edit_lock() == 0) {
                            this->activate(g_00939340[*g_009392b8], g_009392c0[*g_009392b8], 1);
                        } else {
                            ((NetMsg *)g_00805338)->pop((const char *)g_0068ad7c, 5000, 0, 0);
                        }
                        break;
                    case 0x314:
                        this->automate(this->field_23BDC_, 0);
                        break;
                    case 0x315:
                        this->hold(this->field_23BDC_, 0);
                        break;
                    case 0x316:
                    case 0x317:
                        this->go_home(this->field_23BDC_);
                        break;
                    case 0x318:
                        this->go_to(this->field_23BDC_, 0x18, 0);
                        break;
                    case 0x319:
                        this->go_to(this->field_23BDC_, 0x18, 1);
                        break;
                    case 0x31a:
                        this->assemble();
                        break;
                    case 0x31b:
                        this->explore(this->field_23BDC_);
                        break;
                    case 0x31c:
                        this->wait(this->field_23BDC_);
                        break;
                    case 0x31d:
                        this->unload(this->field_23BDC_);
                        break;
                    case 0x31e:
                        this->upgrade(this->field_23BDC_);
                        break;
                    case 799:
                        this->give_veh(this->field_23BDC_);
                        break;
                    case 800:
                        this->designate(this->field_23BDC_);
                        break;
                    case 0x321:
                        this->sentry(this->field_23BDC_, 0);
                        break;
                    case 0x322:
                        this->sentry(this->field_23BDC_, 10);
                        break;
                    }
                }
            } else {
                if (0x725 < a1) {
                    if (a1 < 0x9f1) {
                        if (a1 == 0x9f0) {
                            this->road_to(this->field_23BDC_, 0x1b);
                        } else {
                            switch (a1) {
                            case 0x750:
                                show_credits();
                                break;
                            case 0x751:
                                parse_says(0, (char *)g_00691870, -1, -1);
                                parse_says(1, (char *)g_00691874, -1, -1);
                                popp((char *)g_00691b0c, (const char *)g_0068aa8c, 0, (const char *)g_0068aa7c, 0);
                                break;
                            case 0x800:
                                this->chat(0);
                                break;
                            case 0x801:
                                X_pop((const char *)g_0068ad60, (int(__cdecl *)())g_005398e0);
                                break;
                            case 0x802:
                                ((MultiWin *)g_007fd648)->on_button_clicked(2);
                                break;
                            case 0x803:
                                this->custom_my_faction();
                                break;
                            case 0x804:
                                this->time_controls();
                                break;
                            case 0x805:
                                this->zoom_to_upkeep_base();
                                break;
                            case 0x806:
                                this->use_time_bonus();
                                break;
                            case 0x807:
                            case 0x808:
                                this->end_my_turn();
                                break;
                            }
                        }
                    } else {
                        switch (a1) {
                        case 0x9f1:
                            this->road_to(this->field_23BDC_, 0x1c);
                            break;
                        case 0x9f2:
                            this->automate(this->field_23BDC_, 0);
                            break;
                        case 0x9f3:
                            this->automate(this->field_23BDC_, 1);
                            break;
                        case 0x9f4:
                            this->automate(this->field_23BDC_, 2);
                            break;
                        case 0x9f5:
                            this->automate(this->field_23BDC_, 7);
                            break;
                        case 0x9f6:
                            this->automate(this->field_23BDC_, 6);
                            break;
                        case 0x9f7:
                            this->automate(this->field_23BDC_, 3);
                            break;
                        case 0x9f8:
                            this->automate(this->field_23BDC_, 4);
                            break;
                        case 0x9f9:
                            this->automate(this->field_23BDC_, 5);
                            break;
                        }
                    }
                    goto switchD_0051be9e_default;
                }
                if (a1 == 0x725) {
                    *g_008cc228 = 6;
                    ((TutWin *)g_008c6e68)->tour();
                    goto switchD_0051be9e_default;
                }
                if (0x709 < a1) {
                    switch (a1) {
                    case 0x70a:
                        help_topic(3, 0);
                        break;
                    case 0x70b:
                        help_topic(10, 0);
                        break;
                    case 0x70c:
                        help_topic(0xb, 0);
                        break;
                    case 0x70d:
                        help_topic(0xc, 0);
                        break;
                    case 0x70e:
                        help_topic(0xf, 0);
                        break;
                    case 0x720:
                        *g_008cc228 = 0;
                        ((TutWin *)g_008c6e68)->tour();
                        break;
                    case 0x721:
                        *g_008cc228 = 3;
                        ((TutWin *)g_008c6e68)->tour();
                        break;
                    case 0x722:
                        *g_008cc228 = 1;
                        ((TutWin *)g_008c6e68)->tour();
                        break;
                    case 0x723:
                        *g_008cc228 = 5;
                        ((TutWin *)g_008c6e68)->tour();
                        break;
                    case 0x724:
                        *g_008cc228 = 7;
                        ((TutWin *)g_008c6e68)->tour();
                        break;
                    }
                    goto switchD_0051be9e_default;
                }
                if (a1 == 0x709) {
                    help_topic(8, 0);
                    goto switchD_0051be9e_default;
                }
                if (0x704 < a1) {
                    switch (a1) {
                    case 0x705:
                        help_topic(4, 0);
                        break;
                    case 0x706:
                        help_topic(6, 0);
                        break;
                    case 0x707:
                        help_topic(7, 0);
                        break;
                    case 0x708:
                        help_topic(5, 0);
                        break;
                    }
                    goto switchD_0051be9e_default;
                }
                if (a1 == 0x704) {
                    help_topic(0xe, 0);
                    goto switchD_0051be9e_default;
                }
                if (0x701 < a1) {
                    if (a1 == 0x702) help_topic(0, 0);
                    else if (a1 == 0x703) help_topic(1, 0);
                    goto switchD_0051be9e_default;
                }
                if (a1 != 0x701) {
                    if (a1 == 0x324) this->set_patrol(this->field_23BDC_);
                    else if (a1 == 0x600) this->editor_on();
                    goto switchD_0051be9e_default;
                }
switchD_0051c15a_caseD_108:
                help_topic(2, 0);
            }
        }
        goto switchD_0051be9e_default;
    }
    if (a1 == 0x103) {
        design_new_veh(*g_00939284, -1);
        goto switchD_0051be9e_default;
    }
    switch (a1) {
    case 0x51:
        *g_009a6494 |= 0x200000;
        ((MainMenu *)&this->field_22A2C_)->check(this->field_23BDC_);
        X_pop((const char *)g_0068a9c0, 0);
        break;
    case 0x52:
        *g_009a6494 &= 0xffdfffff;
        ((MainMenu *)&this->field_22A2C_)->check(this->field_23BDC_);
        X_pop((const char *)g_0068a9cc, 0);
        break;
    case 0x53:
        this->set_preferences();
        break;
    case 0x54:
        this->set_base_preferences();
        break;
    case 0x55:
        this->set_adv_preferences();
        break;
    case 0x56:
        this->set_auto_preferences();
        break;
    case 0x57:
        this->set_audiovisual();
        break;
    case 0x58:
        this->set_map_display();
        break;
    case 0x59:
        save_game(0);
        break;
    case 0x5a:
        this->load_proc(0, 0);
        break;
    case 0x5b:
        this->retire_game();
        break;
    case 0x5c: {
        if (*g_0093f660 != 0) break;
        const char *msg;
        if (*g_0093a95c == 0) {
            msg = (const char *)g_0068a7e4;
        } else {
            int n = bit_count(*(unsigned char *)g_009a64e8);
            msg = (n > 1) ? (const char *)g_0068a7d4 : (const char *)g_0068a7e4;
        }
        if (X_pop(msg, 0) != 0) {
            if (*g_0093a95c != 0) {
                *(unsigned char *)g_009a64e8 &= ~(1 << (*g_00939284 & 0x1f));
                save_game(0);
            }
            if ((*g_009a649c & 0x400) != 0 && *g_0093a95c == 0) {
                auto_save();
            }
            if (*g_0093f660 != 0) {
                ((NetDaemon *)g_0093cd90)->hang_up();
                sub_51dd00();
                *g_0093f660 = 0;
            }
            *g_009b2068 = 1;
            log_say((char *)g_0068a7f0, 0, 0, 0);
        }
        if (*g_009b2068 != 0) {
            *g_0093a948 = 1;
        }
        break;
    }
    case 0x5d: {
        const char *msg;
        if (*g_0093f660 == 0 && *g_0093a95c == 0) {
            msg = (const char *)g_0068a7e4;
        } else {
            int n = bit_count(*(unsigned char *)g_009a64e8);
            msg = (n > 1) ? (const char *)g_0068a7d4 : (const char *)g_0068a7e4;
        }
        if (X_pop(msg, 0) != 0) {
            if (*g_0093a95c != 0) {
                *(unsigned char *)g_009a64e8 &= ~(1 << (*g_00939284 & 0x1f));
                save_game(0);
            }
            if ((*g_009a649c & 0x400) != 0 && *g_0093a95c == 0) {
                auto_save();
            }
            if (*g_0093f660 != 0) {
                ((NetDaemon *)g_0093cd90)->hang_up();
                sub_51dd00();
                *g_0093f660 = 0;
            }
            *g_009b2068 = 1;
            log_say((char *)g_0068a7f0, 0, 0, 0);
        }
        break;
    }
    }

switchD_0051be9e_default:
    if ((*g_009a64c0 & 0x40) == 0) {
        return;
    }
    if (0xa00 < a1) {
        int has_zoom = 0;
        switch (a1) {
        case 0xa01:
            this->editor_load();
            return;
        default:
            return;
        case 0xa10:
            this->editor_paints(0);
            return;
        case 0xa11:
            this->editor_paints(1);
            return;
        case 0xa12:
            this->editor_paints(2);
            return;
        case 0xa13:
            this->editor_paints(3);
            return;
        case 0xa14:
            this->editor_paints(4);
            return;
        case 0xa15:
            this->editor_terrain();
            return;
        case 0xa16:
            this->editor_paints(5);
            return;
        case 0xa17:
            this->editor_natural(1);
            return;
        case 0xa18:
            this->editor_natural(0);
            return;
        case 0xa19:
            this->editor_minor();
            return;
        case 0xa20:
            has_zoom = 0;
            break;
        case 0xa21:
            has_zoom = 1;
            break;
        case 0xa24:
            this->editor_set_brush();
            return;
        case 0xa30:
            this->editor_climate();
            return;
        case 0xa31:
            this->editor_generate();
            return;
        case 0xa32:
            this->editor_fast();
            return;
        case 0xa36:
            this->editor_fungus();
            return;
        case 0xa37:
            this->editor_rockiness();
            return;
        case 0xa40:
            this->editor_randomize();
            return;
        case 0xa41:
            this->editor_clear_random();
            return;
        case 0xa48:
            this->editor_flat();
            return;
        case 0xa50:
            this->editor_clear();
            return;
        case 0xa51:
            this->editor_polar();
            return;
        case 0xa52:
            this->editor_clear_vehicles();
            return;
        case 0xa53:
            this->editor_clear_terrain();
            return;
        case 0xa54:
            this->editor_lock2();
            return;
        }
        ((MapWin *)*g_007d3c3c)->editor(g_00939340[*g_009392b8], g_009392c0[*g_009392b8], has_zoom);
        return;
    }
    if (a1 != 0xa00) {
        switch (a1) {
        case 0x601:
            goto switchD_0051c269_caseD_601;
        case 0x602:
            this->editor_veh();
            return;
        case 0x603:
            this->editor_edit_veh(-1);
            return;
        case 0x604:
            tech_advance(*g_00939284);
            return;
        case 0x605:
            this->editor_tech();
            return;
        case 0x606:
            this->editor_view();
            return;
        case 0x607:
            this->editor_diff();
            return;
        case 0x608:
            this->editor_energy();
            return;
        case 0x609:
            this->editor_date();
            return;
        case 0x60a:
            this->editor_eliminate();
            return;
        case 0x60b:
            this->editor_reload();
            return;
        case 0x60c:
            this->editor_kill_vehicles();
            return;
        case 0x60d:
            show_replay();
            return;
        case 0x60e:
            this->show_movies();
            return;
        case 0x60f:
            this->editor_reset_tech();
            return;
        case 0x610:
            this->editor_reset_faction();
            return;
        case 0x611:
            this->editor_diplomacy();
            return;
        case 0x612:
            this->editor_personality();
            return;
        case 0x613:
            this->editor_strategy();
            return;
        case 0x614:
            this->editor_rules();
            return;
        case 0x615:
            this->editor_scen_rules();
            return;
        case 0x616:
            this->editor_scen_param();
            return;
        case 0x617:
            this->editor_scen_victory();
            return;
        case 0x618:
            this->editor_undo();
            return;
        case 0x619:
            this->editor_redo();
            return;
        case 0x61a:
            this->load_proc(1, 0);
            return;
        case 0x61b:
            save_game(1);
            return;
        default:
            return;
        }
    }
    this->editor_save();
    return;

switchD_0051c269_caseD_601:
    *g_009a64c0 ^= 0x80;
    if (((*g_009a64c0 & 0x80) == 0) || ((*g_009a64c0 & 0x40) == 0)) {
        reinterpret_cast<VCall *>(g_007b0cb8)->slot002();
    } else {
        reinterpret_cast<VCall *>(g_007b0cb8)->slot001(0);
    }
    draw_map(1);
    ((GraphicWin *)g_008e9f60)->redraw();
    desktop_update();
    ((InfoWin *)g_007ad2a0)->change(0);
    ((StatusWin *)g_008c5568)->redraw();
    ((MapWin *)*g_007d3c3c)->main_caption();
    return;
}
