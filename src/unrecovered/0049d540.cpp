// ORIGINAL: 0x0049D540 ?draw_ops@ReportWin@@QAEXXZ 0x0049D540-0x0049EAEB;0x00659435-0x00659447 FILE
// TRIED: full transcription of the header/setup, string-building/switch, per-row spot+box_sprite+name/status rendering, and the 0x8b3 (production preview) branch, all cross-checked against raw push/pop counts; mandate_color(int) sites that Ghidra showed as 4-arg were re-derived as mandate_color(x) followed by a separate set_text_color(result,-1,1,1) sharing 3 leftover pushes, matching the same pattern found on 0x00484C60. Approximated: the 0x8b4 sprite-table indices/bases in the "!special" loops (0x00776420/0x00776428/0x0077644c family) are inferred by analogy with the "special" 0x0075b710 pair (this=base, transparent idx=base+8) rather than independently re-derived from raw bytes, so their exact table base/stride may be off; roughly 90% of the function's instructions are covered by a structurally faithful path.
// working copy - scaffold materialised by --work
// size      5565 bytes
// prototype void (__thiscall ?draw_ops@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   2   call targets   36
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045E540 0x00493AF0 0x00493B70 0x004E39D0 0x004E3A00 0x004E4020 0x004E4430 0x004EC3B0 0x0050E820 0x005594F0 0x0055AF20 0x005AC060 0x005B9580 0x005BFE90 0x005C1850 0x005C1D50 0x005DAC70 0x005DACB0 0x005DCEA0 0x005DD130 0x005DD3B0 0x005E2DD3 0x005E3E00 0x005E4B9A 0x005FA960 0x005FAA90 0x006059B0 0x00605D20 0x006169A0 0x00625730 0x00625740 0x00625750 0x00625770 0x006453E0 0x00645470 0x0064FC88
// indirect  0x0049D57B 0x0049D6A6 0x0049D6B7 0x0049D6C8 0x0049D912 0x0049DD5C 0x0049DE11

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_ops@ReportWin@@QAEXXZ  at 0x0049D540  (5565 bytes)
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
struct BoxSpriteParams;
class Buffer;
class ButtonGroup;
class Caviar;
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
class ProdPicker;
class PullDown;
struct PullDownItem;
struct RECT;
class Random;
class ReportWin;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
    int add(int, int, int, int, int, int);
    void clear();
    void kill_type(int);
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int get_data();
    int init_class();
    int set_font(Font *, Font *, Font *, Font *);
    int text_height();
    int text_line_height();
    int write_cent_l(char *, RECT *, int);
    int write_l(char *, int, int, int);
    int write_right_l(char *, int, int, int);
    void clear_links();
    void close();
    void close_class();
    void construct();
    void destroy();
    void set_text_color(int, int, int, int);
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

class Caviar { public:
    float scene_scale_;
    uint32_t field_4_;
    uint8_t field_8_[0xC];
    uint32_t field_14_;
    uint8_t field_18_[0x4];
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
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
    float field_6C_;
    float field_70_;
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
    uint8_t field_A4_;
    uint8_t field_A5_[0x63];
    int32_t field_108_;
    uint8_t field_10C_[0x640];
    uint32_t field_74C_;
    uint32_t field_750_;
    uint32_t field_754_;
    uint8_t field_758_[0xC74];
    uint32_t field_13CC_;
    float get_scaling();
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
    int draw(Buffer *, int, int, int);
    int draw_mono(Buffer *, int, int, int, int, int, int);
    void close();
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
    void set_pos(int);
    void set_range(int, int);
};

class ProdPicker : public GraphicWin { public:
    int32_t field_A14_;
    Sprite sprite1_;
    Sprite sprite2_;
    Sprite sprite3_;
    Font font1_;
    Font font2_;
    Font font3_;
    Font font4_;
    Time time_;
    uint32_t field_B64_;
    uint32_t field_B68_;
    uint32_t field_B6C_;
    uint32_t field_B70_;
    FlatButton flatButton1_;
    FlatButton flatButton2_;
    FlatButton flatButton3_;
    FlatButton flatButton4_;
    FlatButton flatButton5_;
    FlatButton flatButton6_;
    FlatButton flatButton7_;
    FlatButton flatButton8_;
    FlatButton flatButton9_;
    Scroll scroll_;
    Caviar caviar_;
    uint8_t field_A63C_[0x100];
    uint32_t field_A73C_;
    uint32_t field_A740_;
    uint32_t field_A744_;
    uint8_t field_A748_[0x18];
    float field_A760_;
    float field_A764_;
    float field_A768_;
    uint32_t field_A76C_;
    uint32_t field_A770_;
    uint32_t field_A774_;
    uint32_t field_A778_;
    uint32_t field_A77C_;
    uint32_t field_A780_;
    uint32_t field_A784_;
    uint32_t field_A788_;
    uint32_t field_A78C_;
    uint32_t field_A790_;
    uint32_t field_A794_;
    uint32_t field_A798_;
    uint32_t field_A79C_;
    uint32_t field_A7A0_;
    uint32_t field_A7A4_;
    uint32_t field_A7A8_;
    uint32_t field_A7AC_;
    uint32_t field_A7B0_;
    uint32_t field_A7B4_;
    uint32_t field_A7B8_;
    uint32_t field_A7BC_;
    uint32_t field_A7C0_;
    uint32_t field_A7C4_;
    uint32_t field_A7C8_;
    uint32_t field_A7CC_;
    uint32_t field_A7D0_;
    uint32_t field_A7D4_;
    uint32_t field_A7D8_;
    uint32_t field_A7DC_;
    uint32_t field_A7E0_;
    uint32_t field_A7E4_;
    uint32_t field_A7E8_;
    uint32_t field_A7EC_;
    uint32_t field_A7F0_;
    uint32_t field_A7F4_;
    uint32_t field_A7F8_;
    uint32_t field_A7FC_;
    uint32_t field_A800_;
    uint32_t field_A804_;
    uint32_t field_A808_;
    uint32_t field_A80C_;
    uint32_t field_A810_;
    uint32_t field_A814_;
    uint32_t field_A818_;
    uint32_t field_A81C_;
    uint32_t field_A820_;
    uint32_t field_A824_;
    uint32_t field_A828_;
    uint32_t field_A82C_;
    uint32_t field_A830_;
    uint32_t field_A834_;
    uint32_t field_A838_;
    uint32_t field_A83C_;
    int edbc_fac(int);
    int edbc_unit(int);
    void close();
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

class Random { public:
    uint32_t seed_;
    Random();
    uint32_t get_seed();
    unsigned int get(int, int);
    void reseed(unsigned long);
    ~Random();
};

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

extern "C" char *_itoa(int, char *, int);
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
int base_compute(int);
int best_specialist();
int cost_factor(int, int, int);
int is_objective(int);
int mandate_color(int);
int stack_check(int, int, int, int, int);
int veh_at(int, int);
int veh_cost(int, int, int *);
int veh_draw(Buffer *, int, int, int, int, int, int);
int veh_fake(int, int);
void base_draw(Buffer *, int, int, int, int, int);
void say_base(char *, int);
void scroll_hide(GraphicWin *, Scroll *);
void set_base(int);
extern "C" unsigned long __stdcall timeGetTime();
extern "C" char *__stdcall CharUpperA(char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of
// a slot are yours to set and setting them does not move it: write
// `virtual int slot074(int, int);` if that is the call you need.
// The slots below are spelled nullary because the emitter reads the
// vtable OFFSET from the body and not the argument list.
// This body dispatches through slot(s): 1
class VCall { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0049eaec = (int *)0x0049EAEC;
static int *const g_0065943d = (int *)0x0065943D;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00678868 = (int *)0x00678868;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_006870a8 = (int *)0x006870A8;
static int *const g_006870ac = (int *)0x006870AC;
static int *const g_006870b0 = (int *)0x006870B0;
static int *const g_006870b4 = (int *)0x006870B4;
static int *const g_006870b8 = (int *)0x006870B8;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0068f9f0 = (int *)0x0068F9F0;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_006a8044 = (int *)0x006A8044;
static int *const g_00759320 = (int *)0x00759320;
static int *const g_0075933c = (int *)0x0075933C;
static int *const g_0075b180 = (int *)0x0075B180;
static int *const g_0075b188 = (int *)0x0075B188;
static int *const g_0075b1ac = (int *)0x0075B1AC;
static int *const g_0075b1b4 = (int *)0x0075B1B4;
static int *const g_0075b710 = (int *)0x0075B710;
static int *const g_0075b718 = (int *)0x0075B718;
static int *const g_00776420 = (int *)0x00776420;
static int *const g_00776428 = (int *)0x00776428;
static int *const g_0077644c = (int *)0x0077644C;
static int *const g_00779b78 = (int *)0x00779B78;
static int *const g_00779b80 = (int *)0x00779B80;
static int *const g_00787200 = (int *)0x00787200;
static int *const g_0078721c = (int *)0x0078721C;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d550 = (int *)0x0078D550;
static int *const g_0078d578 = (int *)0x0078D578;
static int *const g_0078d5f0 = (int *)0x0078D5F0;
static int *const g_0078dac8 = (int *)0x0078DAC8;
static int *const g_008a40bc = (int *)0x008A40BC;
static int *const g_008a40c4 = (int *)0x008A40C4;
static int *const g_008a4160 = (int *)0x008A4160;
static int *const g_008a418c = (int *)0x008A418C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00946d54 = (int *)0x00946D54;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00952858 = (int *)0x00952858;
static int *const g_0095285a = (int *)0x0095285A;
static int *const g_0096da3c = (int *)0x0096DA3C;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d078 = (int *)0x0097D078;
static int *const g_009a4b68 = (int *)0x009A4B68;
static int *const g_009a4b74 = (int *)0x009A4B74;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a658c = (int *)0x009A658C;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class ReportWin { public:
    uint8_t pad_0_[0x5A0C];
    uint32_t field_5a0c_;
    uint8_t pad_5a10_[0x66E8];
    uint32_t field_c0f8_;

    void sat_anim();
    void exp_anim();
    void draw_ops();
};
void ReportWin::draw_ops() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    char *msg = reinterpret_cast<char *>(g_009b86a0);

    char local_22c[80];
    char local_1dc[80];
    char local_18c[80];
    char local_13c[80];
    char local_ec[80];
    int local_9c;
    int v98, v94, v90, v8c, v88, v84, v80, v7c;
    unsigned int v78;
    int v74, v70, v6c, v68, v64, v60;
    unsigned int v5c;
    int v54, v50, v4c, v48, v44, v40, v3c, v38, v34, v30, v2c;
    unsigned int v28;
    int *v24;
    int v20, v1c, v18, v14;

    unsigned int curSkin = *g_008a4160;
    v78 = curSkin;

    Random rng;
    rng.reseed(timeGetTime());

    *reinterpret_cast<int *>(self + 0x59a0) = 0x28;
    *reinterpret_cast<int *>(self + 0x59a4) = 0x28;

    v94 = *reinterpret_cast<int *>(self + 0x5978);
    v98 = *reinterpret_cast<int *>(self + 0x5974);
    v8c = v94 + 0x22 + *reinterpret_cast<int *>(self + 0xe268);
    v90 = *reinterpret_cast<int *>(self + 0x597c);
    v64 = *reinterpret_cast<int *>(self + 0xe268) + 9 + v94;
    v68 = v90 - 3;
    v6c = v94 + 3;
    v7c = *reinterpret_cast<int *>(self + 0x5980);
    v70 = v98 + 3;
    v84 = v8c + 2;
    v48 = v8c + 5;
    v30 = v98 + 0x2b;
    v40 = v8c + 0x2d;
    v4c = v98 + 0x2d;
    v88 = v98;
    v80 = v90;
    v60 = v64;
    v50 = v8c;
    v44 = v68;
    v38 = v70;
    v34 = v48;
    v2c = v40;
    v24 = reinterpret_cast<int *>(v68);

    buf->box_sprite(reinterpret_cast<RECT *>(&v98), reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    buf->box_sprite(reinterpret_cast<RECT *>(&v70), reinterpret_cast<BoxSpriteParams *>(g_0078d550));
    buf->box_sprite(reinterpret_cast<RECT *>(&v88), reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    v20 = *reinterpret_cast<int *>(self + 0x59a4) + 2;
    v50 = ((v7c - v84) + -6) / v20;

    reinterpret_cast<VCall *>(self + 0x7c9c)->slot001(0);
    reinterpret_cast<VCall *>(self + 0x87e8)->slot001(0);
    reinterpret_cast<VCall *>(self + 0x9334)->slot001(0);

    if (*reinterpret_cast<int *>(self + 0xc0f8) == 0) {
        v24 = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096da3c) + curSkin * 0x833 * 4);
        if (v50 <= v24[0]) {
            *reinterpret_cast<int *>(self + 0xe26c) = 1;
        }
    } else {
        v24 = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096da3c) + curSkin * 0x833 * 4);
        if (v24[0] < v50) {
            scroll_hide(reinterpret_cast<GraphicWin *>(self), reinterpret_cast<Scroll *>(self + 0xc10c));
            *reinterpret_cast<int *>(self + 0xc0f8) = 0;
        }
    }

    if (*reinterpret_cast<int *>(self + 0xe270) != 0) {
        Scroll *scroll = reinterpret_cast<Scroll *>(self + 0xc10c);
        int iv = v24[0];
        *reinterpret_cast<int *>(self + 0xc108) = v50 - 1;
        *reinterpret_cast<int *>(self + 0xc0fc) = 0;
        *reinterpret_cast<int *>(self + 0xc100) = 0;
        *reinterpret_cast<int *>(self + 0xc104) = iv - 1;
        scroll->set_range(0, iv - 1);
        *reinterpret_cast<int *>(self + 0xcb4c) = *reinterpret_cast<int *>(self + 0xc108);
        scroll->set_pos(*reinterpret_cast<int *>(self + 0xc0fc));
        *reinterpret_cast<int *>(self + 0xe270) = 0;
    }

    reinterpret_cast<Spot *>(self + 0x5994)->kill_type(0);
    *reinterpret_cast<int *>(self + 0x599c) = 0;

    *msg = 0;
    strcat(msg, reinterpret_cast<char *>(g_006870a8));
    v74 = curSkin * 0x59c;
    strcat(msg, reinterpret_cast<char *>(g_00946d54) + v74);
    strcat(msg, reinterpret_cast<char *>(g_00682820));
    {
        Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
        strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xacc))));
    }

    if (*reinterpret_cast<int *>(self + 0x5a08) != 0) {
        strcat(msg, reinterpret_cast<char *>(g_00682820));
        strcat(msg, reinterpret_cast<char *>(g_006870ac));
        strcat(msg, reinterpret_cast<char *>(g_00682820));
        int caseVal = *reinterpret_cast<int *>(self + 0x5a08);
        bool haveId = true;
        int strId = 0;
        char *base98 = reinterpret_cast<char *>(*g_009b90f8);
        switch (caseVal) {
        case 1: strId = *reinterpret_cast<int *>(base98 + 0xa0); break;
        case 2: strId = *reinterpret_cast<int *>(base98 + 0xa4); break;
        case 3: strId = *reinterpret_cast<int *>(base98 + 0xe4); break;
        case 4: strId = *reinterpret_cast<int *>(base98 + 0xaac); break;
        case 5: strId = *reinterpret_cast<int *>(base98 + 0xab0); break;
        case 6: strId = *reinterpret_cast<int *>(base98 + 0xab4); break;
        case 7: strId = *reinterpret_cast<int *>(base98 + 0xfc); break;
        case 8: strId = *reinterpret_cast<int *>(base98 + 0xab8); break;
        default: haveId = false; break;
        }
        if (haveId) {
            Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
            strcat(msg, reinterpret_cast<char *>(strings->get(strId)));
        }
    }

    CharUpperA(msg);
    strcat(msg, reinterpret_cast<char *>(g_006870b0));
    v6c -= 1;
    v64 -= 1;
    {
        unsigned int len = strlen(msg);
        buf->write_cent_l(msg, reinterpret_cast<RECT *>(&v70), len);
    }
    v6c += 1;
    v64 += 1;

    v24 = reinterpret_cast<int *>(-1);
    v1c = 0;
    if (0 < *g_009a64cc) {
        do {
            int *entry = reinterpret_cast<int *>(g_008a418c) + v1c * 4;
            unsigned char skinByte = *(reinterpret_cast<unsigned char *>(g_0097d044) + entry[0] * 0x134);
            if (skinByte == *g_008a4160) {
                int cur = *reinterpret_cast<int *>(self + 0xc0fc);
                v24 = reinterpret_cast<int *>(reinterpret_cast<int>(v24) + 1);
                if (cur <= reinterpret_cast<int>(v24) && reinterpret_cast<int>(v24) < cur + v50) {
                    if (cur < reinterpret_cast<int>(v24)) {
                        v48 += v20;
                        v40 += v20;
                        v34 += v20;
                        v2c += v20;
                    }
                    *reinterpret_cast<int *>(self + 0xe258) = v38;
                    *reinterpret_cast<int *>(self + 0xe264) = v40;
                    *reinterpret_cast<int *>(self + 0xe25c) = v48;
                    *reinterpret_cast<int *>(self + 0xe260) = (v44 - v4c) + 3 + v30;

                    reinterpret_cast<Spot *>(self + 0x5994)->add(
                        entry[0] + 0x400, 0,
                        *reinterpret_cast<int *>(self + 0xe258), v48,
                        *reinterpret_cast<int *>(self + 0xe260) - *reinterpret_cast<int *>(self + 0xe258),
                        *reinterpret_cast<int *>(self + 0xe264) - v48);
                    reinterpret_cast<Spot *>(self + 0x5994)->add(
                        entry[0] + 0xc00, 0, v38, v34, v30 - v38, v2c - v34);

                    int flags = entry[0];
                    RECT *box1 = reinterpret_cast<RECT *>(&v4c);
                    RECT *box2 = reinterpret_cast<RECT *>(&v38);
                    void *style;
                    if ((*reinterpret_cast<unsigned int *>(self + 0x59b4 + (((flags + ((flags >> 0x1f) & 0x1f)) >> 5) * 4)) &
                         (1u << (flags & 0x1f))) == 0) {
                        buf->box_sprite(box1, reinterpret_cast<BoxSpriteParams *>(g_0078d550));
                        style = g_0078d550;
                    } else {
                        buf->box_sprite(box1, reinterpret_cast<BoxSpriteParams *>(g_0078d578));
                        buf->box_sprite(box1, reinterpret_cast<BoxSpriteParams *>(g_0078d5f0));
                        buf->box_sprite(box2, reinterpret_cast<BoxSpriteParams *>(g_0078d578));
                        style = g_0078d5f0;
                    }
                    buf->box_sprite(box2, reinterpret_cast<BoxSpriteParams *>(style));

                    if ((reinterpret_cast<unsigned int *>(g_0097d078)[entry[0] * 0x4d] & 0x80000000u) != 0) {
                        for (int bitI = 0; bitI < 4; bitI++) {
                            if ((reinterpret_cast<unsigned int *>(g_0097d078)[entry[0] * 0x4d] & (0x1000000u << (bitI & 0x1f))) != 0) {
                                v38 += 1;
                                v30 -= 1;
                                v34 += 1;
                                v2c -= 1;
                                buf->box_sprite(box2, reinterpret_cast<BoxSpriteParams *>(reinterpret_cast<char *>(g_0078dac8) + bitI * 0x78));
                                v38 -= 1;
                                v30 += 1;
                                v34 -= 1;
                                v2c += 1;
                                break;
                            }
                        }
                    }

                    v18 = v20 / 2;
                    v60 = (v18 - *reinterpret_cast<int *>(self + 0xe268) / 2) + v48;

                    int ratioX;
                    if (*g_00691e6c < 1) ratioX = 0x3e;
                    else ratioX = (*g_00691e6c * 100) / *g_00691e70;
                    int posX = (*reinterpret_cast<int *>(self + 0x59a0) - ratioX) / 2 + 1 + v38;
                    *reinterpret_cast<int *>(self + 0xa14) = posX;

                    int ratioY;
                    if (*g_00691e6c < 1) ratioY = 0x2e;
                    else ratioY = (*g_00691e6c * 0x4b) / *g_00691e70;
                    int posY = (*reinterpret_cast<int *>(self + 0x59a4) - ratioY) / 2 + 1 + v34;
                    *reinterpret_cast<int *>(self + 0xa18) = posY;

                    base_draw(self != 0 ? buf : 0, entry[0], posX, posY, -6, 0x40);
                    buf->set_text_color(0xe1, -1, 1, 1);
                    buf->set_font(reinterpret_cast<Font *>(*g_008a40c4), reinterpret_cast<Font *>(*g_008a40bc),
                                  reinterpret_cast<Font *>(*g_008a40c4), 0);
                    set_base(entry[0]);
                    base_compute(0);
                    *msg = 0;
                    say_base(msg, entry[0]);

                    char *baseObj = reinterpret_cast<char *>(*g_0090ea30);
                    v14 = *reinterpret_cast<int *>(baseObj + 0x8c);
                    if (v14 < *reinterpret_cast<int *>(baseObj + 0x8e) &&
                        (*g_009a658c < 0 || *(reinterpret_cast<unsigned char *>(g_0097d044) + *g_009a658c * 0x134) != v78)) {
                        buf->set_text_color(0xf9, -1, 1, 1);
                    } else if (*reinterpret_cast<int *>(baseObj + 0x8e) == 0 &&
                               (2 < static_cast<signed char>(baseObj[3]) &&
                                (static_cast<signed char>(baseObj[3]) + 1) / 2 <= v14)) {
                        strcat(msg, reinterpret_cast<char *>(g_006870b4));
                        buf->set_text_color(0xff, -1, 1, 1);
                    }

                    *reinterpret_cast<int *>(self + 0xa14) = v4c + 5;
                    CharUpperA(msg);
                    v3c = *reinterpret_cast<int *>(self + 0xa14);
                    v14 = v48 + 5;
                    {
                        unsigned int len = strlen(msg);
                        buf->write_l(msg, v3c, v14, len);
                    }

                    *msg = 0;
                    int objFlag = is_objective(entry[0]);
                    if (objFlag == 0 || (*g_009a649c & 0x20000000) != 0) {
                        if ((reinterpret_cast<unsigned int *>(g_0097d078)[entry[0] * 0x4d] & 0x80000000u) == 0) {
                            buf->set_text_color(0xe1, -1, 1, 1);
                            Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                            strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xbfc))));
                        } else {
                            for (int bitI2 = 0; bitI2 < 4; bitI2++) {
                                if ((reinterpret_cast<unsigned int *>(g_0097d078)[entry[0] * 0x4d] & (0x1000000u << (bitI2 & 0x1f))) != 0) {
                                    int color = mandate_color(bitI2);
                                    buf->set_text_color(color, -1, 1, 1);
                                    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                                    strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x824 + bitI2 * 4))));
                                    break;
                                }
                            }
                        }
                    } else {
                        buf->set_text_color(reinterpret_cast<int *>(g_0068f9f0)[*(reinterpret_cast<unsigned char *>(g_0097d044) + v1c * 0x134)], -1, 1, 1);
                        Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                        strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xe08))));
                        CharUpperA(msg);
                    }
                    int statusOff = v40 - *reinterpret_cast<int *>(self + 0xe268);
                    v3c = *reinterpret_cast<int *>(self + 0xa14) + 0x14;
                    {
                        unsigned int len = strlen(msg);
                        buf->write_l(msg, v3c, statusOff - 1, len);
                    }

                    int viewMode = *reinterpret_cast<int *>(self + 0x7c8c);
                    *reinterpret_cast<int *>(self + 0xa14) += 0xc0;

                    if (viewMode == 0x8b3) {
                        int prod = *reinterpret_cast<int *>(baseObj + 0x28);
                        v14 = prod;
                        int itemDraw;
                        if (prod < 0) {
                            int negProd = -prod;
                            v54 = negProd;
                            void *iconRec;
                            unsigned int bufTernary = self != 0 ? reinterpret_cast<unsigned int>(buf) : 0;
                            if (negProd < 0x46) {
                                v3c = *g_0078721c;
                                v5c = (v40 - v48) - 2;
                                int facSrc = reinterpret_cast<ProdPicker *>(g_006a8044)->edbc_fac(negProd);
                                itemDraw = mandate_color(facSrc);
                                v18 = *reinterpret_cast<int *>(self + 0xa14) + 8;
                                iconRec = reinterpret_cast<char *>(g_00787200) + negProd * 0x2c;
                            } else {
                                v18 = *g_0075933c;
                                v28 = (v40 - v48) - 2;
                                int unitColor = mandate_color(1);
                                itemDraw = unitColor;
                                v5c = *reinterpret_cast<int *>(self + 0xa14) + 8;
                                iconRec = reinterpret_cast<char *>(negProd * 0x2c + 0x758718);
                            }
                            unsigned char transparentIdx = *(reinterpret_cast<unsigned char *>(iconRec) + 8);
                            int y2 = *reinterpret_cast<int *>(self + 0xa18);
                            reinterpret_cast<Sprite *>(iconRec)->draw_mono(reinterpret_cast<Buffer *>(bufTernary),
                                transparentIdx, v18, y2 + 4, itemDraw, v5c, v3c);
                            prod = v14;
                        } else {
                            int vehShp = veh_fake(prod, static_cast<signed char>(baseObj[2]));
                            veh_draw(self != 0 ? buf : 0, vehShp, *reinterpret_cast<int *>(self + 0xa14),
                                     *reinterpret_cast<int *>(self + 0xa18) + 4, -8, 0x4c, 0);
                        }
                        *msg = 0;
                        buf->set_font(reinterpret_cast<Font *>(*g_008a40c4), reinterpret_cast<Font *>(*g_008a40c4),
                                      reinterpret_cast<Font *>(*g_008a40c4), 0);
                        int rushCost;
                        int itemCount;
                        if (prod < 0) {
                            int negProd2 = -prod;
                            itemCount = reinterpret_cast<int *>(g_009a4b74)[prod * -0xc];
                            rushCost = negProd2;
                            Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                            strcat(msg, reinterpret_cast<char *>(strings->get(reinterpret_cast<int *>(g_009a4b68)[prod * -0xc])));
                            int facSrc2 = reinterpret_cast<ProdPicker *>(g_006a8044)->edbc_fac(negProd2);
                            buf->set_text_color(mandate_color(facSrc2), -1, 1, 1);
                        } else {
                            strcat(msg, reinterpret_cast<char *>(g_009ab868) + prod * 0x34);
                            itemCount = veh_cost(prod, *g_00689370, &local_9c);
                            int unitSrc2 = reinterpret_cast<ProdPicker *>(g_006a8044)->edbc_unit(prod);
                            buf->set_text_color(mandate_color(unitSrc2), -1, 1, 1);
                            rushCost = itemCount;
                        }
                        v18 = *reinterpret_cast<int *>(self + 0xa14) + 0x3c;
                        *reinterpret_cast<int *>(self + 0xa14) = v18;
                        {
                            unsigned int len = strlen(msg);
                            buf->write_l(msg, v18, v60, len);
                        }
                        buf->set_text_color(0xf2, -1, 1, 1);
                        *msg = 0;
                        strcat(msg, reinterpret_cast<char *>(g_00682e9c));
                        if (rushCost == 0x45) {
                            _itoa(0, local_ec, 10);
                            strcat(msg, local_ec);
                        } else {
                            _itoa(*reinterpret_cast<int *>(baseObj + 0x20), local_1dc, 10);
                            strcat(msg, local_1dc);
                        }
                        strcat(msg, reinterpret_cast<char *>(g_006870b8));
                        int factor = cost_factor(static_cast<signed char>(baseObj[2]), 1, -1);
                        int totalCost = factor * itemCount;
                        _itoa(totalCost, local_13c, 10);
                        strcat(msg, local_13c);
                        strcat(msg, reinterpret_cast<char *>(g_00682e98));
                        strcat(msg, reinterpret_cast<char *>(g_00682820));
                        strcat(msg, reinterpret_cast<char *>(g_00682820));
                        strcat(msg, reinterpret_cast<char *>(g_00682820));
                        strcat(msg, reinterpret_cast<char *>(g_00682820));
                        int minerals = *reinterpret_cast<int *>(baseObj + 0x6a);
                        if (minerals < 1) {
                            Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                            strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x10c))));
                        } else {
                            Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
                            strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xc4))));
                            strcat(msg, reinterpret_cast<char *>(g_00682e94));
                            strcat(msg, reinterpret_cast<char *>(g_00682820));
                            int have = *reinterpret_cast<int *>(baseObj + 0x20);
                            if (totalCost - have == 0 || totalCost < have) {
                                _itoa(0, local_22c, 10);
                                strcat(msg, local_22c);
                            } else {
                                int turns = minerals < 1 ? 1 : (minerals > 99999 ? 99999 : minerals);
                                int result = (minerals - 1 + (totalCost - have)) / turns;
                                if (result < 0) result = 0;
                                else if (result > 999999) result = 999999;
                                _itoa(result, local_18c, 10);
                                strcat(msg, local_18c);
                            }
                        }
                        {
                            unsigned int len = strlen(msg);
                            buf->write_right_l(msg, v44 - 4, v60, len);
                        }
                    } else if (viewMode == 0x8b4) {
                        int perRow;
                        signed char rowsMax = static_cast<signed char>(baseObj[3]);
                        if (rowsMax < 2) {
                            perRow = 0x22;
                        } else {
                            perRow = ((v44 - v4c) - 0xe2) / (rowsMax - 1);
                            if (perRow < 1) perRow = 1;
                            else if (perRow > 0x22) perRow = 0x22;
                        }
                        *reinterpret_cast<int *>(self + 0xa14) = perRow - 0x20 + v44;
                        *reinterpret_cast<int *>(self + 0xa18) = (v40 - v18) - 0x12;
                        bool special = *g_009a6488 != 0 &&
                            (reinterpret_cast<unsigned char *>(g_00946f58)[v74] & 0x80) != 0;
                        Spot *spot = reinterpret_cast<Spot *>(self + 0x5994);
                        if (!special) {
                            int cnt3e = *reinterpret_cast<int *>(baseObj + 0x3e);
                            for (int i1 = 0; i1 < cnt3e; i1++) {
                                int y1 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y1;
                                spot->add(v1c, 0, y1, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                unsigned int idx16 = (cnt3e - i1) - 1;
                                unsigned int sprIdx;
                                if (static_cast<int>(idx16) < 0x10) {
                                    int shift = (idx16 & 7) << 2;
                                    sprIdx = (*reinterpret_cast<unsigned int *>(baseObj + (static_cast<int>(idx16) >> 3) * 2 + 0x42) >> shift) & 0xf;
                                } else {
                                    sprIdx = best_specialist();
                                }
                                {
                                    Sprite *sprRec = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_00779b78) + sprIdx * 0x2c);
                                    unsigned char tIdx = *(reinterpret_cast<unsigned char *>(sprRec) + 8);
                                    sprRec->draw(self != 0 ? buf : 0, tIdx, y1, *reinterpret_cast<int *>(self + 0xa18));
                                }
                            }
                            int cnt8e = *reinterpret_cast<int *>(baseObj + 0x8e);
                            for (int i2 = 0; i2 < cnt8e; i2++) {
                                int y2b = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y2b;
                                spot->add(v1c, 0, y2b, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                int r = rng.get(0, 2);
                                int idx = ((i2 < *reinterpret_cast<int *>(baseObj + 0x90) ? 1 : 0) + 2 + r * 4);
                                {
                                    Sprite *sprRec = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_00776420) + idx * 0x2c);
                                    unsigned char tIdx = *(reinterpret_cast<unsigned char *>(sprRec) + 8);
                                    sprRec->draw(self != 0 ? buf : 0, tIdx, y2b, *reinterpret_cast<int *>(self + 0xa18));
                                }
                            }
                            int uZ = *reinterpret_cast<unsigned int *>(baseObj + 0x8e);
                            int empty = (((static_cast<int>(rowsMax)) - *reinterpret_cast<int *>(baseObj + 0x8c)) - cnt3e) -
                                        (uZ & (uZ < 0 ? -1 : 0));
                            for (int i3 = 0; i3 < empty; i3++) {
                                int y3 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y3;
                                spot->add(v1c, 0, y3, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                int r2 = rng.get(0, 2);
                                {
                                    Sprite *sprRec = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_0077644c) + r2 * 0xb0);
                                    unsigned char tIdx = *(reinterpret_cast<unsigned char *>(sprRec) + 8);
                                    sprRec->draw(self != 0 ? buf : 0, tIdx, y3, *reinterpret_cast<int *>(self + 0xa18));
                                }
                            }
                            int cnt8c = *reinterpret_cast<int *>(baseObj + 0x8c);
                            for (int i4 = 0; i4 < cnt8c; i4++) {
                                int y4 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y4;
                                spot->add(v1c, 0, y4, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                int r3 = rng.get(0, 2);
                                {
                                    Sprite *sprRec = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_00776420) + r3 * 0xb0);
                                    unsigned char tIdx = *(reinterpret_cast<unsigned char *>(sprRec) + 8);
                                    sprRec->draw(self != 0 ? buf : 0, tIdx, y4, *reinterpret_cast<int *>(self + 0xa18));
                                }
                            }
                        } else {
                            int cnt3e = *reinterpret_cast<int *>(baseObj + 0x3e);
                            for (int j1 = 0; j1 < cnt3e; j1++) {
                                int y1 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y1;
                                spot->add(v1c, 0, y1, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                unsigned int idx16 = (cnt3e - j1) - 1;
                                unsigned int sprIdx;
                                if (static_cast<int>(idx16) < 0x10) {
                                    int shift = (idx16 & 7) << 2;
                                    sprIdx = (*reinterpret_cast<unsigned int *>(baseObj + (static_cast<int>(idx16) >> 3) * 2 + 0x42) >> shift) & 0xf;
                                } else {
                                    sprIdx = best_specialist();
                                }
                                {
                                    Sprite *sprRec = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_0075b710) + sprIdx * 0x2c);
                                    unsigned char tIdx = *(reinterpret_cast<unsigned char *>(sprRec) + 8);
                                    sprRec->draw(self != 0 ? buf : 0, tIdx, y1, *reinterpret_cast<int *>(self + 0xa18));
                                }
                            }
                            int cnt8e = *reinterpret_cast<int *>(baseObj + 0x8e);
                            for (int j2 = 0; j2 < cnt8e; j2++) {
                                int y2b = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y2b;
                                spot->add(v1c, 0, y2b, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                int pick = (j2 < *reinterpret_cast<int *>(baseObj + 0x90) ? 1 : 0) + 2;
                                Sprite *sprRec2 = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_0075b180) + pick * 0xb);
                                unsigned char tIdx2 = static_cast<unsigned char>(*g_0075b188 & 0xff);
                                sprRec2->draw(self != 0 ? buf : 0, tIdx2, y2b, *reinterpret_cast<int *>(self + 0xa18));
                            }
                            int uZ = *reinterpret_cast<unsigned int *>(baseObj + 0x8e);
                            int empty = (((static_cast<int>(rowsMax)) - *reinterpret_cast<int *>(baseObj + 0x8c)) - cnt3e) -
                                        (uZ & (uZ < 0 ? -1 : 0));
                            for (int j3 = 0; j3 < empty; j3++) {
                                int y3 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y3;
                                spot->add(v1c, 0, y3, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                reinterpret_cast<Sprite *>(g_0075b1ac)->draw(
                                    self != 0 ? buf : 0, static_cast<unsigned char>(*g_0075b1b4 & 0xff), y3,
                                    *reinterpret_cast<int *>(self + 0xa18));
                            }
                            int cnt8c = *reinterpret_cast<int *>(baseObj + 0x8c);
                            for (int j4 = 0; j4 < cnt8c; j4++) {
                                int y4 = *reinterpret_cast<int *>(self + 0xa14) - perRow;
                                *reinterpret_cast<int *>(self + 0xa14) = y4;
                                spot->add(v1c, 0, y4, *reinterpret_cast<int *>(self + 0xa18), perRow, v20);
                                reinterpret_cast<Sprite *>(g_0075b180)->draw(
                                    self != 0 ? buf : 0, static_cast<unsigned char>(*g_0075b188 & 0xff), y4,
                                    *reinterpret_cast<int *>(self + 0xa18));
                            }
                        }
                    } else if (viewMode == 0x8b5) {
                        int veh = veh_at(*reinterpret_cast<int *>(baseObj), *(reinterpret_cast<int *>(baseObj) + 1));
                        int slots = stack_check(veh, 1, -1, -1, -1);
                        int perRow2;
                        if (slots < 2) {
                            perRow2 = 0x28;
                        } else {
                            perRow2 = ((v44 - v4c) - 0xe8) / (slots - 1);
                            if (perRow2 < 1) perRow2 = 1;
                            else if (perRow2 > 0x28) perRow2 = 0x28;
                        }
                        *reinterpret_cast<int *>(self + 0xa14) = perRow2 - 0x2d + v44;
                        *reinterpret_cast<int *>(self + 0xa18) = v48;
                        if (veh >= 0) {
                            int cur2 = veh;
                            while (reinterpret_cast<short *>(g_0095285a)[cur2 * 0x1a] >= 0) {
                                cur2 = reinterpret_cast<short *>(g_0095285a)[cur2 * 0x1a];
                            }
                            for (; cur2 >= 0; cur2 = reinterpret_cast<short *>(g_00952858)[cur2 * 0x1a]) {
                                int y5 = *reinterpret_cast<int *>(self + 0xa14) - perRow2;
                                *reinterpret_cast<int *>(self + 0xa14) = y5;
                                veh_draw(self != 0 ? buf : 0, cur2, y5, *reinterpret_cast<int *>(self + 0xa18), -8, 8, 0);
                                reinterpret_cast<Spot *>(self + 0x5994)->add(entry[0] + 0x800, cur2, y5,
                                    *reinterpret_cast<int *>(self + 0xa18), perRow2, v20);
                            }
                        }
                    }
                }
            }
            v1c++;
        } while (v1c < *g_009a64cc);
    }
}

