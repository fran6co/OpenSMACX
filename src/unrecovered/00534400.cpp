// ORIGINAL: 0x00534400 ?process_message@NetDaemon@@QAEXPADKH@Z 0x00534400-0x005388B9;0x0065E0C2-0x0065E442 FILE
// TRIED: no Ghidra hypothesis was available for this one (raw asm only). The real dispatcher spans ~67 distinct message-type case bodies across several chained range checks up to type 0x1300+; only the first band (0xF05-0xF0F, 11 cases) and the shared default "fixup_message + send_message" forwarding path (the target of the large majority of jump-table slots) were read from the disassembly and modelled. Everything above type 0x1101 is unimplemented.
// working copy - scaffold materialised by --work
// size      18489 bytes
// prototype void (__thiscall ?process_message@NetDaemon@@QAEXPADKH@Z)(NetDaemon* this, MessageFactionData*, unsigned int, int)
// callers   18   call targets   133
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00424410 0x00424720 0x004271C0 0x00428350 0x004412F0 0x004413F0 0x00441490 0x004426F0 0x00442740 0x004429C0 0x00444D90 0x00446A00 0x0045C0B0 0x0046B190 0x00479330 0x0047A890 0x0047C970 0x00481AD0 0x00481DA0 0x00481FD0 0x00483420 0x0048C0A0 0x004ACBD0 0x004C96E0 0x004C9B00 0x004CA7F0 0x004CAA50 0x004CC360 0x004CD2F0 0x004CD4E0 0x004CE210 0x004CE790 0x004CEDE0 0x004CF380 0x004CF480 0x004CF740 0x004E25E0 0x004E2610 0x004E2660 0x004E26B0 0x004EFB50 0x00506A60 0x0050BA30 0x0050BCC0 0x00523C60 0x00532940 0x00532E00 0x00534330 0x00534400 0x00538FB0 0x00538FE0 0x0053C650 0x00543A20 0x00548F80 0x0054CF40 0x0055B760 0x0055B820 0x0055B870 0x0055BA80 0x0055BB30 0x0055BBA0 0x0055BC00 0x0057F1D0 0x0058FB00 0x0058FC50 0x0058FFA0 0x00590170 0x005901D0 0x00590240 0x005902C0 0x00590300 0x00590470 0x00592550 0x00592600 0x005926F0 0x00592EE0 0x005947C0 0x005B9C40 0x005BB000 0x005BF310 0x005BF420 0x005C0B00 0x005C89A0 0x005C89B0 0x005D4510 0x005D45B0 0x005D4DD0 0x005D5A70 0x005E3820 0x005EE280 0x005F7E90 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0060D1B0 0x0060E740 0x00610120 0x006108E0 0x00611A20 0x00612960 0x006169A0 0x00625810 0x00625E30 0x00625E50 0x00625EC0 0x00626250 0x006262F0 0x006263F0 0x006272F0 0x00628380 0x006283E0 0x00628650 0x006287C0 0x00630080 0x00630A00 0x00631A20 0x00634330 0x00644EF2 0x00645460 0x00645470 0x00645550 0x00645930 0x0064FC88
// indirect  0x00534EC9 0x00534ED0 0x005374DF 0x00537F83 0x00538561 0x00538573 0x00538595 0x00538611 0x00538623 0x00538645 0x005386E0 0x005386F4 0x00538715 0x00538791 0x005387A5 0x005387C6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?process_message@NetDaemon@@QAEXPADKH@Z  at 0x00534400  (18489 bytes)
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

class AlphaNet;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class BaseButton;
class BasePop;
class Buffer;
class ButtonGroup;
class CheckBox;
class CouncWin;
class Dialog;
struct DialogEntry;
class Dialogs;
class DiploWin;
class EditGroup;
class FX;
class Filemap;
class FlatButton;
class Font;
struct Goal;
class GraphicWin;
typedef void * HANDLE;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class ListBox;
class Lock;
class MainInterface;
class Menu;
struct MenuEntry;
struct MessageFactionData;
class MultiWin;
class Net;
class NetDaemon;
class NetMsg;
class NetWin;
typedef void * PVOID;
struct PlayerData;
class PlayerLock;
class Popup;
class PullDown;
struct PullDownItem;
struct RECT;
class RadioButton;
class Scroll;
struct SocialCategory;
struct SocialEffect;
class Spot;
class Sprite;
class SpriteBox;
class Strings;
class Time;
typedef unsigned int UINT_PTR;
class VoiceTx;
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
class VoiceTx { public:
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
};

class AlphaNet { public:
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
    uint8_t field_2C_[0x18];
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    VoiceTx voice_tx_;
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
    uint8_t field_E8_[0x6C];
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint8_t field_160_[0x4];
    uint32_t field_164_;
    uint8_t field_168_;
    uint8_t field_169_[0x56B];
    uint32_t field_6D4_;
    uint8_t field_6D8_[0x4];
    uint32_t field_6DC_;
    uint32_t field_6E0_;
    uint32_t field_6E4_;
    uint32_t field_6E8_;
    uint8_t field_6EC_[0x10];
    uint32_t field_6FC_;
    uint32_t field_700_;
    uint32_t field_704_;
    uint32_t field_708_;
    uint32_t field_70C_;
    uint32_t field_710_;
    uint32_t field_714_;
    uint32_t field_718_;
    uint32_t field_71C_;
    uint32_t field_720_;
    uint8_t field_724_[0x8];
    uint32_t field_72C_;
    uint32_t field_730_;
    uint32_t field_734_;
    uint32_t field_738_;
    uint32_t field_73C_;
    uint32_t field_740_;
    uint32_t field_744_;
    uint8_t field_748_[0x8];
    uint32_t field_750_;
    uint32_t field_754_;
    uint32_t field_758_;
    uint32_t field_75C_;
    uint32_t field_760_;
    uint32_t field_764_;
    uint8_t field_768_[0x18];
    uint8_t data_[0xD20];
    int pid_2_idx(unsigned long);
    int pid_2_who(unsigned long);
    int who_2_pid(int);
    void close();
    void who_2_idx(int);
};

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
    ~Spot();
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
    ~GraphicWin();
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
    ~BaseButton();
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
    ~FlatButton();
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
    int exec(int, int (__cdecl *)());
    static void fallout();
    uint32_t read_check();
    void close();
    ~BasePop();
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

class CheckBox { public:
    void close();
    ~CheckBox();
};

class CouncWin { public:
    int make_resolution(int);
    int on_button_text_draw(::GraphicWin *, char *, int);
    void arrived(int);
    void draw_all_leaders();
    void set_vote(int, int);
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
    ~Dialog();
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

class Dialogs { public:
    uint32_t vbtable_pointer_;
    uint8_t field_4_[0x40];
    uint32_t field_44_;
    uint8_t field_48_[0x10];
    uint32_t field_58_;
    uint8_t field_5C_[0x14];
    uint32_t field_70_;
    uint8_t field_74_[0x30];
    int32_t field_A4_;
    uint8_t field_A8_[0x50];
    uint32_t field_F8_;
    uint8_t field_FC_[0x84];
    int32_t kind_;
    uint8_t unmapped_184_[0x188 - 0x184];
    GraphicWin virtual_base_;
    uint8_t gap_B9C_[4];
    Dialog dialog_;
    int get_num_items();
    uint32_t destroy();
    void close();
};

class DiploWin { public:
    void UNK2();
    void UNK5();
    void add_offer(int, int, int);
    void check_done();
    void do_it();
    void done_deal();
    void hide();
    void kill_offer(int, int);
    void new_chat(char *, int);
};

class EditGroup { public:
    ~EditGroup();
};

class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
};

class Filemap { public:
    LPVOID map_view_addr_;
    HANDLE file_;
    HANDLE file_map_;
    uint32_t file_size_;
    Filemap();
    int create(char *, unsigned long, int);
    uint32_t get_size();
    void close();
    ~Filemap();
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

struct Goal {
    int16_t type;
    int16_t priority;
    int x;
    int y;
    int base_id;
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
    uint32_t close();
    uint32_t destroy();
    void on_dialog_focus(int);
    ~ListBox();
};

class Lock { public:
    int add_lock(int, int, int, int);
    int any_locks();
    int check_global_2(int);
    int global_lock(int);
    int lock(int, int, int, int, int, int, int);
    void check_global();
    void clear();
    void reset_map();
    void unlock(int);
};

class MainInterface { public:
    void UNK2();
    void UNK3();
    void UNK4();
    void chat(char *, int);
    void clear_message();
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

class MultiWin { public:
    void draw(int);
};

class Net { public:
    char * get_player_name(unsigned long);
    int get(unsigned long *, unsigned long *);
    int poll_players(int);
    int send(void *, int, unsigned long, int);
};

class NetMsg { public:
    void close();
    void pop(const char *, int, int, const char *);
};

class NetWin { public:
    int pick_pass(int);
    void UNK1();
    void UNK2();
    void UNK5();
    void alloc_slots();
    void fill_player(PlayerData *, int);
    void new_chat(char *, int);
    void on_redraw();
    void setup_faction_data(MessageFactionData *);
};

struct SocialCategory {
    uint32_t politics;
    uint32_t economics;
    uint32_t values;
    uint32_t future;
};

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

struct PlayerData {
    uint32_t flags;
    uint32_t ranking;
    int diff_level;
    uint32_t base_name_offset;
    uint32_t base_sea_name_offset;
    int last_turn_new_base;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8];
    uint32_t diplo_friction[8];
    int diplo_spoke[8];
    int diplo_merc[8];
    char diplo_patience[8];
    int sanction_turns;
    int loan_balance[8];
    int loan_payment[8];
    int unk_12[8];
    int integrity_blemishes;
    int global_reputation;
    int diplo_unk1[8];
    int diplo_wrongs[8];
    int diplo_betrayed[8];
    int diplo_unk3[8];
    int diplo_unk4[8];
    int traded_maps;
    int base_governor_adv;
    int atrocities;
    int major_atrocities;
    uint32_t mind_control_total;
    uint32_t diplo_mind_control[8];
    uint32_t stolen_data_count[8];
    int energy_reserves;
    uint32_t hurry_cost_total;
    SocialCategory soc_category_pending;
    SocialCategory soc_category_active;
    int soc_upheaval_cost_paid;
    SocialEffect soc_effect_pending;
    SocialEffect soc_effect_active;
    SocialEffect soc_effect_temp;
    SocialEffect soc_effect_base;
    int unk_13;
    uint32_t maint_cost_total;
    int tech_commerce_bonus;
    int turn_commerce_income;
    int unk_17;
    int unk_18;
    int tech_fungus_nutrient;
    int tech_fungus_mineral;
    int tech_fungus_energy;
    int tech_fungus_unk;
    int se_alloc_psych;
    int se_alloc_labs;
    uint32_t tech_pact_shared_goals[12];
    int tech_ranking;
    int unk_26;
    uint32_t sat_odp_deployed;
    int theory_of_everything;
    char tech_trade_source[92];
    int tech_accumulated;
    int tech_id_researching;
    int tech_cost;
    int earned_techs_saved;
    int net_random_event;
    int ai_fight;
    BOOL ai_growth;
    BOOL ai_tech;
    BOOL ai_wealth;
    BOOL ai_power;
    int x_target;
    int y_target;
    int unk_28;
    int council_call_turn;
    int unk_29[11];
    int unk_30[11];
    uint8_t facility_announced[4];
    int unk_32;
    char unk_33;
    char unk_34;
    char unk_35;
    char unk_36;
    int planet_ecology;
    int base_id_atk_target;
    int unk_37;
    char saved_queue_name[8][24];
    int saved_queue_size[8];
    int saved_queue_items[8][10];
    int unk_38[8];
    int unk_39[8][9];
    int unk_46[9];
    int unk_47;
    int unk_48;
    int unk_49;
    uint32_t nutrient_surplus_total;
    int labs_total;
    uint32_t satellites_nutrient;
    uint32_t satellites_mineral;
    uint32_t satellites_energy;
    uint32_t satellites_odp_total;
    int best_weapon_value;
    int best_psi_offense;
    int best_psi_defense;
    int best_armor_value;
    int best_land_speed;
    int enemy_best_weapon_value;
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
    int total_mil_veh;
    uint32_t current_num_bases;
    int mil_strength_1;
    int mil_strength_2;
    int pop_total;
    int unk_70;
    int planet_busters;
    int unk_71;
    int unk_72;
    uint16_t region_total_combat_vehs[128];
    uint8_t region_total_bases[128];
    uint8_t region_total_offensive_vehs[128];
    uint16_t region_force_rating[128];
    uint16_t unk_77[128];
    uint16_t unk_78[128];
    uint16_t unk_79[128];
    uint16_t unk_80[128];
    uint16_t unk_81[128];
    uint8_t unk_82[128];
    uint8_t unk_83[128];
    uint8_t region_base_plan[128];
    Goal goals[75];
    Goal sites[25];
    int unk_93;
    int unk_94;
    int unk_95;
    int unk_96;
    int unk_97;
    uint32_t tech_achieved;
    int time_bonus_count;
    int unk_99;
    uint32_t secret_project_intel[8];
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

class PlayerLock { public:
    void active();
    void clear();
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
    uint32_t set_thumb_rect();
    void close();
};

class Popup : public BasePop { public:
    Scroll scroll_;
    Popup();
    void close();
    void on_adjust_button_width();
    void on_redraw_nc(RECT *, int);
    void start(char *, const char *, int, char *, int, GraphicWin *);
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

class RadioButton { public:
    void close();
};

class SpriteBox { public:
    int sprite(Sprite *, char *, int);
    void close();
    void on_mouse_leave(int, int);
    void on_mouse_move(int, int);
    ~SpriteBox();
};

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

char * get_him_her(int, int);
char * get_pact_hood(int, int);
extern "C" char *_itoa(int, char *, int);
extern "C" char *strcat(char *, const char *);
extern "C" char *strcpy(char *, const char *);
extern "C" int __cdecl _alloca_probe();
extern "C" int __cdecl sub_634330();
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);
int __cdecl X_pop(char *, const char *, int, int (__cdecl *)());
int __cdecl X_pop(const char *, int (__cdecl *)());
int action(int);
int action_airdrop(int, int, int, int);
int action_build(int, char *);
int action_terraform(int, int, int);
int battle_fight(int, int, int, int, int *);
int bit_count(int);
int do_withdrawal(int, int);
int find_landmark(int, int, int);
int game_year(int);
int my_srand(int);
int new_landmark(int, int, char *);
int order_veh(int, int, int);
int parse_num(int, int);
int parse_say(int, int, int, int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int spying(int);
unsigned char checksum_file(char *);
unsigned int random(int, int);
void * mem_get(int);
void action_destroy(int, int, int, int);
void action_destruct(int);
void action_fungal(int, int, int);
void action_gate(int, int);
void action_give(int, int);
void action_oblit(int, int);
void action_sat_attack(int, int, int, int);
void action_staple(int);
void action_tectonic(int, int, int);
void agenda_off(int, int, int);
void agenda_on(int, int, int);
void council(int, int, int);
void diplo(int, int);
void do_upgrade(int, int, int);
void double_cross(int, int, int);
void draw_map(int);
void give_a_base(int, int);
void kill(int);
void kill_landmark(int, int);
void log_say(char *, char *, int, int, int);
void log_say(char *, int, int, int);
void log_say_hex(char *, int, int, int);
void message_data(int, int, int, int, int, int);
void pact_ends(int, int);
void pop_close();
void report_intel(int);
void retire_proto(int, int);
void say_tech(char *, int, int);
void say_year(char *);
void set_agenda(int, int, int, int);
void set_time_controls();
void set_treaty(int, int, int, int);
void tech_achieved(int, int, int, int);
void trade_maps(int, int);
void treaty_off(int, int, int);
void treaty_on(int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of
// a slot are yours to set and setting them does not move it: write
// `virtual int slot074(int, int);` if that is the call you need.
// The slots below are spelled nullary because the emitter reads the
// vtable OFFSET from the body and not the argument list.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005388bc = (int *)0x005388BC;
static int *const g_005388e8 = (int *)0x005388E8;
static int *const g_00538908 = (int *)0x00538908;
static int *const g_0053891c = (int *)0x0053891C;
static int *const g_00538930 = (int *)0x00538930;
static int *const g_00538944 = (int *)0x00538944;
static int *const g_0053895c = (int *)0x0053895C;
static int *const g_005389ac = (int *)0x005389AC;
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_00539920 = (int *)0x00539920;
static int *const g_0065e438 = (int *)0x0065E438;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_006698cc = (int *)0x006698CC;
static int *const g_006698d4 = (int *)0x006698D4;
static int *const g_00669a58 = (int *)0x00669A58;
static int *const g_00669a64 = (int *)0x00669A64;
static int *const g_00669a6c = (int *)0x00669A6C;
static int *const g_00669bd4 = (int *)0x00669BD4;
static int *const g_00669be0 = (int *)0x00669BE0;
static int *const g_00669be8 = (int *)0x00669BE8;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0067c388 = (int *)0x0067C388;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068c11c = (int *)0x0068C11C;
static int *const g_0068c570 = (int *)0x0068C570;
static int *const g_0068c730 = (int *)0x0068C730;
static int *const g_0068c740 = (int *)0x0068C740;
static int *const g_0068c750 = (int *)0x0068C750;
static int *const g_0068c764 = (int *)0x0068C764;
static int *const g_0068c770 = (int *)0x0068C770;
static int *const g_0068c780 = (int *)0x0068C780;
static int *const g_0068c790 = (int *)0x0068C790;
static int *const g_0068c7a8 = (int *)0x0068C7A8;
static int *const g_0068c7c4 = (int *)0x0068C7C4;
static int *const g_0068c7e0 = (int *)0x0068C7E0;
static int *const g_0068c800 = (int *)0x0068C800;
static int *const g_0068c820 = (int *)0x0068C820;
static int *const g_0068c840 = (int *)0x0068C840;
static int *const g_0068c858 = (int *)0x0068C858;
static int *const g_0068c86c = (int *)0x0068C86C;
static int *const g_0068c880 = (int *)0x0068C880;
static int *const g_0068c8a4 = (int *)0x0068C8A4;
static int *const g_0068c8c8 = (int *)0x0068C8C8;
static int *const g_0068c8e0 = (int *)0x0068C8E0;
static int *const g_0068c8f4 = (int *)0x0068C8F4;
static int *const g_0068c910 = (int *)0x0068C910;
static int *const g_0068c930 = (int *)0x0068C930;
static int *const g_0068c94c = (int *)0x0068C94C;
static int *const g_0068c964 = (int *)0x0068C964;
static int *const g_0068c978 = (int *)0x0068C978;
static int *const g_0068c99c = (int *)0x0068C99C;
static int *const g_0068c9b4 = (int *)0x0068C9B4;
static int *const g_0068c9cc = (int *)0x0068C9CC;
static int *const g_0068c9e0 = (int *)0x0068C9E0;
static int *const g_0068c9fc = (int *)0x0068C9FC;
static int *const g_0068ca18 = (int *)0x0068CA18;
static int *const g_0068ca38 = (int *)0x0068CA38;
static int *const g_0068ca44 = (int *)0x0068CA44;
static int *const g_0068ca50 = (int *)0x0068CA50;
static int *const g_0068ca68 = (int *)0x0068CA68;
static int *const g_0068ca74 = (int *)0x0068CA74;
static int *const g_0068ca8c = (int *)0x0068CA8C;
static int *const g_0068ca98 = (int *)0x0068CA98;
static int *const g_0068cab0 = (int *)0x0068CAB0;
static int *const g_0068cadc = (int *)0x0068CADC;
static int *const g_0068cafc = (int *)0x0068CAFC;
static int *const g_0068cb18 = (int *)0x0068CB18;
static int *const g_0068cb34 = (int *)0x0068CB34;
static int *const g_0068cb50 = (int *)0x0068CB50;
static int *const g_0068cb78 = (int *)0x0068CB78;
static int *const g_0068cba0 = (int *)0x0068CBA0;
static int *const g_0068cbbc = (int *)0x0068CBBC;
static int *const g_0068cbd4 = (int *)0x0068CBD4;
static int *const g_0068cbf0 = (int *)0x0068CBF0;
static int *const g_0068cbfc = (int *)0x0068CBFC;
static int *const g_0068cc08 = (int *)0x0068CC08;
static int *const g_0068cc24 = (int *)0x0068CC24;
static int *const g_0068cc48 = (int *)0x0068CC48;
static int *const g_0068cc68 = (int *)0x0068CC68;
static int *const g_0068cc80 = (int *)0x0068CC80;
static int *const g_0068cc98 = (int *)0x0068CC98;
static int *const g_0068ccac = (int *)0x0068CCAC;
static int *const g_0068ccc0 = (int *)0x0068CCC0;
static int *const g_0068ccd8 = (int *)0x0068CCD8;
static int *const g_0068ccf0 = (int *)0x0068CCF0;
static int *const g_0068cd08 = (int *)0x0068CD08;
static int *const g_0068cd1c = (int *)0x0068CD1C;
static int *const g_0068cd3c = (int *)0x0068CD3C;
static int *const g_0068cd5c = (int *)0x0068CD5C;
static int *const g_0068cd7c = (int *)0x0068CD7C;
static int *const g_0068cd94 = (int *)0x0068CD94;
static int *const g_0068cdac = (int *)0x0068CDAC;
static int *const g_0068cdc4 = (int *)0x0068CDC4;
static int *const g_0068cdd8 = (int *)0x0068CDD8;
static int *const g_0068cdec = (int *)0x0068CDEC;
static int *const g_0068ce00 = (int *)0x0068CE00;
static int *const g_0068ce14 = (int *)0x0068CE14;
static int *const g_0068ce30 = (int *)0x0068CE30;
static int *const g_0068ce4c = (int *)0x0068CE4C;
static int *const g_0068ce6c = (int *)0x0068CE6C;
static int *const g_0068ce9c = (int *)0x0068CE9C;
static int *const g_0068cebc = (int *)0x0068CEBC;
static int *const g_0068ced8 = (int *)0x0068CED8;
static int *const g_0068cef4 = (int *)0x0068CEF4;
static int *const g_0068cf0c = (int *)0x0068CF0C;
static int *const g_0068cf14 = (int *)0x0068CF14;
static int *const g_0068cf30 = (int *)0x0068CF30;
static int *const g_0068cf3c = (int *)0x0068CF3C;
static int *const g_0068cf5c = (int *)0x0068CF5C;
static int *const g_0068cf6c = (int *)0x0068CF6C;
static int *const g_0068cf7c = (int *)0x0068CF7C;
static int *const g_0068cf88 = (int *)0x0068CF88;
static int *const g_0068cfa8 = (int *)0x0068CFA8;
static int *const g_0068cfb8 = (int *)0x0068CFB8;
static int *const g_0068cfd8 = (int *)0x0068CFD8;
static int *const g_0068cff8 = (int *)0x0068CFF8;
static int *const g_0068d018 = (int *)0x0068D018;
static int *const g_0068d030 = (int *)0x0068D030;
static int *const g_0068d04c = (int *)0x0068D04C;
static int *const g_0068d060 = (int *)0x0068D060;
static int *const g_0068d070 = (int *)0x0068D070;
static int *const g_0068d07c = (int *)0x0068D07C;
static int *const g_0068d088 = (int *)0x0068D088;
static int *const g_0068d094 = (int *)0x0068D094;
static int *const g_0068d0a8 = (int *)0x0068D0A8;
static int *const g_0068d0bc = (int *)0x0068D0BC;
static int *const g_0068d0d0 = (int *)0x0068D0D0;
static int *const g_0068d0e4 = (int *)0x0068D0E4;
static int *const g_0068d0f8 = (int *)0x0068D0F8;
static int *const g_0068d108 = (int *)0x0068D108;
static int *const g_0068d118 = (int *)0x0068D118;
static int *const g_0068d12c = (int *)0x0068D12C;
static int *const g_0068d140 = (int *)0x0068D140;
static int *const g_0068d154 = (int *)0x0068D154;
static int *const g_0068d168 = (int *)0x0068D168;
static int *const g_0068d174 = (int *)0x0068D174;
static int *const g_0068d188 = (int *)0x0068D188;
static int *const g_0068d19c = (int *)0x0068D19C;
static int *const g_0068d1ac = (int *)0x0068D1AC;
static int *const g_0068d1bc = (int *)0x0068D1BC;
static int *const g_0068d1d8 = (int *)0x0068D1D8;
static int *const g_0068d1f0 = (int *)0x0068D1F0;
static int *const g_0068d208 = (int *)0x0068D208;
static int *const g_0068d224 = (int *)0x0068D224;
static int *const g_0068d238 = (int *)0x0068D238;
static int *const g_0068d244 = (int *)0x0068D244;
static int *const g_0068d258 = (int *)0x0068D258;
static int *const g_0068d264 = (int *)0x0068D264;
static int *const g_0068d280 = (int *)0x0068D280;
static int *const g_0068d298 = (int *)0x0068D298;
static int *const g_0068d2a4 = (int *)0x0068D2A4;
static int *const g_0068d2b4 = (int *)0x0068D2B4;
static int *const g_0068d2c8 = (int *)0x0068D2C8;
static int *const g_0068d2d4 = (int *)0x0068D2D4;
static int *const g_0068d2e0 = (int *)0x0068D2E0;
static int *const g_0068d2f4 = (int *)0x0068D2F4;
static int *const g_0068d308 = (int *)0x0068D308;
static int *const g_0068d320 = (int *)0x0068D320;
static int *const g_0068d338 = (int *)0x0068D338;
static int *const g_0068d34c = (int *)0x0068D34C;
static int *const g_0068d360 = (int *)0x0068D360;
static int *const g_0068d374 = (int *)0x0068D374;
static int *const g_0068d388 = (int *)0x0068D388;
static int *const g_0068d3a4 = (int *)0x0068D3A4;
static int *const g_0068d3c0 = (int *)0x0068D3C0;
static int *const g_0068d3d0 = (int *)0x0068D3D0;
static int *const g_0068d3e4 = (int *)0x0068D3E4;
static int *const g_0068d3fc = (int *)0x0068D3FC;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_006fec80 = (int *)0x006FEC80;
static int *const g_0073acd8 = (int *)0x0073ACD8;
static int *const g_0073b6f8 = (int *)0x0073B6F8;
static int *const g_0073b6fc = (int *)0x0073B6FC;
static int *const g_0073b700 = (int *)0x0073B700;
static int *const g_0073b790 = (int *)0x0073B790;
static int *const g_007492cc = (int *)0x007492CC;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007fd648 = (int *)0x007FD648;
static int *const g_007fff74 = (int *)0x007FFF74;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0080a6f8 = (int *)0x0080A6F8;
static int *const g_00811e24 = (int *)0x00811E24;
static int *const g_0090db98 = (int *)0x0090DB98;
static int *const g_0090db9b = (int *)0x0090DB9B;
static int *const g_0090dd10 = (int *)0x0090DD10;
static int *const g_0090dd14 = (int *)0x0090DD14;
static int *const g_0090dd17 = (int *)0x0090DD17;
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a510 = (int *)0x0093A510;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a93c = (int *)0x0093A93C;
static int *const g_0093a950 = (int *)0x0093A950;
static int *const g_0093a954 = (int *)0x0093A954;
static int *const g_0093a960 = (int *)0x0093A960;
static int *const g_0093ac88 = (int *)0x0093AC88;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f8 = (int *)0x0093D4F8;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e8f8 = (int *)0x0093E8F8;
static int *const g_0093e904 = (int *)0x0093E904;
static int *const g_0093e968 = (int *)0x0093E968;
static int *const g_0093e970 = (int *)0x0093E970;
static int *const g_0093f668 = (int *)0x0093F668;
static int *const g_0093f66c = (int *)0x0093F66C;
static int *const g_0093f670 = (int *)0x0093F670;
static int *const g_0093f674 = (int *)0x0093F674;
static int *const g_0093f79c = (int *)0x0093F79C;
static int *const g_0093f7a0 = (int *)0x0093F7A0;
static int *const g_0093f7a4 = (int *)0x0093F7A4;
static int *const g_0093f7a8 = (int *)0x0093F7A8;
static int *const g_0093f7ac = (int *)0x0093F7AC;
static int *const g_0093f7b0 = (int *)0x0093F7B0;
static int *const g_0093fab4 = (int *)0x0093FAB4;
static int *const g_0093fac4 = (int *)0x0093FAC4;
static int *const g_00946a50 = (int *)0x00946A50;
static int *const g_00946a84 = (int *)0x00946A84;
static int *const g_00946a9c = (int *)0x00946A9C;
static int *const g_00946d34 = (int *)0x00946D34;
static int *const g_00946d4c = (int *)0x00946D4C;
static int *const g_00946d50 = (int *)0x00946D50;
static int *const g_00946d54 = (int *)0x00946D54;
static int *const g_0094989c = (int *)0x0094989C;
static int *const g_0094f1b8 = (int *)0x0094F1B8;
static int *const g_0094f1bc = (int *)0x0094F1BC;
static int *const g_0094f1c0 = (int *)0x0094F1C0;
static int *const g_0094f1c4 = (int *)0x0094F1C4;
static int *const g_0094f1cc = (int *)0x0094F1CC;
static int *const g_0094f1d0 = (int *)0x0094F1D0;
static int *const g_0094f1d4 = (int *)0x0094F1D4;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_00952839 = (int *)0x00952839;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096caa4 = (int *)0x0096CAA4;
static int *const g_0096cac4 = (int *)0x0096CAC4;
static int *const g_0096cc00 = (int *)0x0096CC00;
static int *const g_0096ea34 = (int *)0x0096EA34;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a4b68 = (int *)0x009A4B68;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e0 = (int *)0x009A64E0;
static int *const g_009a64e4 = (int *)0x009A64E4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009a6514 = (int *)0x009A6514;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b8aa8 = (int *)0x009B8AA8;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;
static int *const g_009bc054 = (int *)0x009BC054;

class NetDaemon { public:
    void send_message(char *, unsigned long, int);
    void synch(short, int, int, int, char *, int, short);
    int fixup_message(char *);
    int receive();
    uint32_t unlock_veh();
    void process_message(char *, unsigned long, int);
};
void NetDaemon::process_message(char * a1, unsigned long a2, int a3) {
    Popup popup;
    Filemap filemap;
    char *self = reinterpret_cast<char *>(this);

    unsigned char *msg = reinterpret_cast<unsigned char *>(a1);
    unsigned short type = *reinterpret_cast<unsigned short *>(a1);

    // Special-cased "file transfer" message-type band (0x1500-0x15FF).
    bool skip500 = ((type & 0xf00) != 0x500) || (type == 0x1501);
    if (!skip500 && *g_007492cc != 0 && *g_0073b790 == (int)a2) {
        skip500 = true;
    }
    if (!skip500) {
        if (type == 0x1505) {
            filemap.~Filemap();
            popup.close();
            return;
        }
        message_data(0x1505, (int)a2, 0, 0, 0, 0);
    }

    type = *reinterpret_cast<unsigned short *>(a1);

    // First dispatch band: message types 0xF05-0xF0F. Bodies below are a
    // faithful-shape but incomplete reading of what is a much larger
    // (roughly 67-case) dispatcher spanning the rest of the type space;
    // see the TRIED note on the file header.
    if (type <= 0x1101) {
        if (type == 0x1101) {
            // TODO: 0x53624a - not reconstructed (schedule/ready-state update).
            return;
        }
        unsigned int idx = (unsigned int)(type - 0xf05);
        if (idx <= 0xa) {
            switch (idx) {
                case 0: {
                    typedef void (OriginalObject::*FnV1)(void *);
                    FnV1 fn = original_method<FnV1>(0x481fd0);
                    (ORIGINAL((void *)0x80a6f8)->*fn)(a1);
                    return;
                }
                case 1: {
                    typedef int (OriginalObject::*FnVisible)();
                    typedef int (OriginalObject::*FnPick)(int);
                    FnVisible isVisible = original_method<FnVisible>(0x5f7e90);
                    if ((ORIGINAL((void *)0x80a6f8)->*isVisible)()) {
                        FnPick pickPass = original_method<FnPick>(0x47c970);
                        (ORIGINAL((void *)0x80a6f8)->*pickPass)(1);
                    }
                    return;
                }
                case 2:
                case 3:
                case 4:
                case 7:
                case 8:
                case 9:
                case 10: {
                    // Modal-dialog guard: when no popup is currently active
                    // (self+0x1b30 == 0), reject and unwind the locals this
                    // function opened; otherwise the original shows a
                    // reason-specific popup this pass does not reconstruct.
                    if (*reinterpret_cast<int *>(self + 0x1b30) == 0) {
                        filemap.~Filemap();
                        popup.close();
                        return;
                    }
                    return;
                }
                case 5:
                case 6: {
                    int field4 = *reinterpret_cast<int *>(a1 + 4);
                    int slot = field4 * 103;
                    *reinterpret_cast<unsigned char *>(self + slot * 4 + 0x794) =
                        (idx == 5) ? 1 : 2;
                    return;
                }
            }
        }
    }

    // Default path, reached by the overwhelming majority of message types
    // (the vast majority of jump-table slots point here in the original):
    // validate and re-broadcast the message.
    if ((msg[1] & 0xa0) != 0) {
        int fixResult = fixup_message(a1);
        if (fixResult == 0) {
            unsigned short masked = (unsigned short)((type & 0x1fff) | 0x4000);
            *reinterpret_cast<unsigned short *>(a1) = masked;
            int *seq = reinterpret_cast<int *>(self + 0x1b2c);
            *seq = *seq + 1;
            *reinterpret_cast<int *>(a1 + 0xc) = *seq;
            send_message(a1, a2, 0);
        }
    }

}
