// ORIGINAL: 0x004B2740 FILE
// RULED-OUT: full-function transcription (~1448 instructions, sim 0.57 vs
//            original at /O2 /Oy-); diverges at instr #9. Receivers for
//            member calls were traced by hand from `mov ecx,...`
//            immediately preceding each call site (VCall slots 1,2,57,62,90
//            widened to match observed arg counts); the society-name walk
//            loop (local_30 vs &DAT_009466d0) and the two multi-row
//            BaseButton/CheckButton hover-rect blocks are the least
//            independently re-checked regions - approximated by pattern
//            match against the neighbouring, verified blocks rather than a
//            second raw-disassembly pass.
// working copy - scaffold materialised by --work
// name      ?init@SocialWin@@QAEXH@Z
// size      5458 bytes
// spans     0x004B2740-0x004B3C7D;0x00659A0A-0x00659A1F
// prototype void (__thiscall ?init@SocialWin@@QAEXH@Z)(SocialWin* this, int factionID)
// callers   2   call targets   26
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D380 0x004AECE0 0x004BA650 0x004BC640 0x004BDFE0 0x005B4730 0x005D4EF0 0x005D5250 0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DAC70 0x005DFF00 0x005E2DD3 0x005E39A0 0x00607210 0x00607550 0x006169A0 0x00618F40 0x0062B7F0 0x0062B810 0x0062B850 0x0062B870 0x006337E0 0x00645470
// indirect  0x004B2784 0x004B33FA 0x004B344F 0x004B346E 0x004B355F 0x004B35AA 0x004B35C9 0x004B366A 0x004B36AB 0x004B36CA 0x004B3700 0x004B3741 0x004B375A 0x004B37AF 0x004B37F0 0x004B3809 0x004B38B6 0x004B38FE 0x004B3980 0x004B3A02 0x004B3A7E 0x004B3B00 0x004B3B7C 0x004B3BE1 0x004B3C4A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@SocialWin@@QAEXH@Z  at 0x004B2740  (5458 bytes)
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
struct BorderSizing;
struct BoxSpriteParams;
class Buffer;
class ButtonGroup;
class CheckButton;
struct ExtDirectDraw;
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
class Palette;
class Popup;
class PullDown;
struct PullDownItem;
struct RECT;
class Scroll;
class SocialWin;
class Spot;
class Sprite;
class Strings;
class SubInterface;
class TexHeap;
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
    Buffer();
    HDC get_hdc();
    int box_sprite(RECT *, BoxSpriteParams *);
    int copy(Buffer *, int, int, int, int, int, int);
    int get_data();
    int init(int, int, GraphicWin *, int);
    int init_class();
    int load_pcx(const char *, Palette *, int, int);
    int set_font(Font *, Font *, Font *, Font *);
    int text_height();
    int text_line_height();
    void clear_links();
    void close();
    void close_class();
    void construct();
    void destroy();
    ~Buffer();
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
    int init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
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
    int extract(Buffer *, int, int, int, int, int, TexHeap *);
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
    int set(int, int);
    void add(BaseButton *);
    void close();
    void init(int, int);
};

class CheckButton : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t isToggled_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_A28_;
    int init(int, int, GraphicWin *, int);
    static int init_class();
    static void close_class();
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
    int init(char *, int, int);
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

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

class SubInterface { public:
    uint32_t vtable_;
    uint32_t field_4_;
    void delete_iface_mode();
    void release_iface_mode();
    void set_iface_mode();
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
    int tut_win(GraphicWin *, const char *, int, int, Sprite *, int, int, int);
    void UNK1();
    void soc_rect(RECT *, int *, int *);
};

extern "C" char *strcat(char *, const char *);
int society_avail(int, int, int);
int tut_check2(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters. The PARAMETERS and RETURN TYPE of
// a slot are yours to set and setting them does not move it: write
// `virtual int slot074(int, int);` if that is the call you need.
// The slots below are spelled nullary because the emitter reads the
// vtable OFFSET from the body and not the argument list.
// This body dispatches through slot(s): 1, 2, 57, 62, 90
class VCall { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used, widened
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
    virtual void slot057(int, void *, void *);  // <-- used, widened
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
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_00659a15 = (int *)0x00659A15;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00678c98 = (int *)0x00678C98;
static int *const g_006874b0 = (int *)0x006874B0;
static int *const g_006874bc = (int *)0x006874BC;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00691b30 = (int *)0x00691B30;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_007ae7f8 = (int *)0x007AE7F8;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007af59c = (int *)0x007AF59C;
static int *const g_007af5a0 = (int *)0x007AF5A0;
static int *const g_007af5a4 = (int *)0x007AF5A4;
static int *const g_007af5a8 = (int *)0x007AF5A8;
static int *const g_007af5ac = (int *)0x007AF5AC;
static int *const g_007af5b0 = (int *)0x007AF5B0;
static int *const g_007af5b4 = (int *)0x007AF5B4;
static int *const g_007af5b8 = (int *)0x007AF5B8;
static int *const g_007af5bc = (int *)0x007AF5BC;
static int *const g_007af5c0 = (int *)0x007AF5C0;
static int *const g_007af5c4 = (int *)0x007AF5C4;
static int *const g_007af5c8 = (int *)0x007AF5C8;
static int *const g_007af5cc = (int *)0x007AF5CC;
static int *const g_007af5d0 = (int *)0x007AF5D0;
static int *const g_007af5d4 = (int *)0x007AF5D4;
static int *const g_007af5d8 = (int *)0x007AF5D8;
static int *const g_007af5dc = (int *)0x007AF5DC;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_008c5380 = (int *)0x008C5380;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_008cc228 = (int *)0x008CC228;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00946598 = (int *)0x00946598;
static int *const g_009466d0 = (int *)0x009466D0;
static int *const g_00946a10 = (int *)0x00946A10;
static int *const g_0094b014 = (int *)0x0094B014;
static int *const g_0096cd04 = (int *)0x0096CD04;
static int *const g_0096cd08 = (int *)0x0096CD08;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bc054 = (int *)0x009BC054;

class SocialWin : public GraphicWin, public SubInterface { public:
    uint32_t field_A1C_;
    Spot spot_;
    uint32_t field_A2C_;
    uint32_t field_A30_;
    uint32_t field_A34_;
    uint32_t field_A38_;
    uint32_t field_A3C_;
    uint32_t field_A40_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    uint32_t field_A4C_;
    uint32_t field_A50_;
    uint32_t field_A54_;
    uint32_t field_A58_;
    uint32_t field_A5C_;
    uint32_t field_A60_;
    uint32_t field_A64_;
    uint32_t field_A68_;
    uint32_t field_A6C_;
    uint32_t field_A70_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    uint32_t field_A7C_;
    uint32_t field_A80_;
    uint32_t field_A84_;
    uint32_t field_A88_;
    uint32_t field_A8C_;
    uint32_t field_A90_;
    uint32_t field_A94_;
    uint32_t field_A98_;
    uint32_t field_A9C_;
    uint32_t field_AA0_;
    uint32_t field_AA4_;
    uint32_t field_AA8_;
    uint32_t field_AAC_;
    uint32_t field_AB0_;
    uint32_t field_AB4_;
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
    uint32_t field_AE0_;
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
    uint32_t field_B10_;
    uint32_t field_B14_;
    uint32_t field_B18_;
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
    uint32_t field_B4C_;
    uint32_t field_B50_;
    uint32_t field_B54_;
    uint32_t field_B58_;
    uint32_t field_B5C_;
    uint32_t field_B60_;
    uint32_t field_B64_;
    uint32_t field_B68_;
    uint32_t field_B6C_;
    uint32_t field_B70_;
    uint32_t field_B74_;
    uint32_t field_B78_;
    uint32_t field_B7C_;
    uint32_t field_B80_;
    uint32_t field_B84_;
    uint32_t field_B88_;
    uint32_t field_B8C_;
    uint32_t field_B90_;
    uint32_t field_B94_;
    uint32_t field_B98_;
    uint32_t field_B9C_;
    uint32_t field_BA0_;
    uint32_t field_BA4_;
    uint32_t field_BA8_;
    uint32_t field_BAC_;
    uint32_t field_BB0_;
    uint32_t field_BB4_;
    uint32_t field_BB8_;
    uint32_t field_BBC_;
    uint32_t field_BC0_;
    uint32_t field_BC4_;
    uint32_t field_BC8_;
    uint32_t field_BCC_;
    uint32_t field_BD0_;
    uint32_t field_BD4_;
    uint32_t field_BD8_;
    uint32_t field_BDC_;
    uint32_t field_BE0_;
    uint32_t field_BE4_;
    uint32_t field_BE8_;
    uint32_t field_BEC_;
    uint32_t field_BF0_;
    uint32_t field_BF4_;
    uint32_t field_BF8_;
    uint32_t field_BFC_;
    uint32_t field_C00_;
    uint32_t field_C04_;
    uint32_t field_C08_;
    uint32_t field_C0C_;
    uint32_t field_C10_;
    uint32_t field_C14_;
    uint32_t field_C18_;
    uint32_t field_C1C_;
    uint32_t field_C20_;
    uint32_t field_C24_;
    uint32_t field_C28_;
    uint32_t field_C2C_;
    uint32_t field_C30_;
    uint32_t field_C34_;
    uint32_t field_C38_;
    uint32_t field_C3C_;
    uint32_t field_C40_;
    uint32_t field_C44_;
    uint32_t field_C48_;
    uint32_t field_C4C_;
    uint32_t field_C50_;
    uint32_t field_C54_;
    uint32_t field_C58_;
    uint32_t field_C5C_;
    uint32_t field_C60_;
    uint32_t field_C64_;
    uint32_t field_C68_;
    uint32_t field_C6C_;
    uint32_t field_C70_;
    uint32_t field_C74_;
    uint32_t field_C78_;
    uint32_t field_C7C_;
    uint32_t field_C80_;
    uint32_t field_C84_;
    uint32_t field_C88_;
    uint32_t field_C8C_;
    uint32_t field_C90_;
    uint32_t field_C94_;
    uint32_t field_C98_;
    uint32_t field_C9C_;
    uint32_t field_CA0_;
    uint32_t field_CA4_;
    uint32_t field_CA8_;
    uint32_t field_CAC_;
    uint32_t field_CB0_;
    uint32_t field_CB4_;
    uint32_t field_CB8_;
    uint32_t field_CBC_;
    uint32_t field_CC0_;
    uint32_t field_CC4_;
    uint32_t field_CC8_;
    uint32_t field_CCC_;
    uint32_t field_CD0_;
    uint32_t field_CD4_;
    uint32_t field_CD8_;
    uint32_t field_CDC_;
    uint32_t field_CE0_;
    uint32_t field_CE4_;
    uint32_t field_CE8_;
    uint32_t field_CEC_;
    uint32_t field_CF0_;
    uint32_t factionID_;
    uint8_t socCategory_[0x10];
    Sprite socEffect_;
    Sprite socEffectOrig_;
    uint32_t netIncome_;
    uint32_t breakthroughTurns_;
    CheckButton energyLockButtons_[3];
    uint32_t field_2BEC_;
    uint32_t field_2BF0_;
    uint32_t field_2BF4_;
    uint32_t field_2BF8_;
    uint32_t field_2BFC_;
    uint32_t field_2C00_;
    uint32_t field_2C04_;
    uint32_t field_2C08_;
    uint32_t field_2C0C_;
    uint32_t field_2C10_;
    uint32_t field_2C14_;
    uint32_t field_2C18_;
    uint32_t field_2C1C_;
    uint32_t field_2C20_;
    uint32_t effectMaximize_;
    uint32_t field_2C28_;
    uint32_t field_2C2C_;
    uint32_t field_2C30_;
    uint32_t field_2C34_;
    uint32_t economyAllocOrig_;
    uint32_t psychAllocOrig_;
    uint32_t labsAllocOrig_;
    uint32_t economyAlloc3_;
    uint32_t psychAlloc3_;
    uint32_t labsAlloc3_;
    uint32_t economyAllocPending_;
    uint32_t psychAllocPending_;
    uint32_t labsAllocPending_;
    uint32_t field_2C5C_;
    uint8_t socEngModifierNums_[0x1B8];
    uint8_t effectIcons_[0xF78];
    uint8_t energyAllocLock_[0x58];
    uint8_t energyAllocArrow_[0x108];
    uint8_t energyAllocSlider_[0x2130];
    FlatButton flatButtons_[35];
    ButtonGroup buttonGroups_[4];
    ButtonGroup buttonGroup_;
    Font font1_;
    Font font2_;
    Font font3_;
    Font font4_;
    Font font5_;
    Font font6_;
    Font font7_;
    Font font8_;
    Font font9_;
    RECT tutRects1_[9];
    RECT tutRect1_;
    RECT tutRects2_[10];
    uint8_t tooltipSocEng_[0x10];
    RECT tutRect4_;
    RECT tutRects3_[18];
    uint8_t tooltipEffects_[0x10];
    uint8_t tooltipEconomy_[0x10];
    uint8_t tooltipLabs_[0x10];
    uint8_t tooltipPsych_[0x10];
    RECT tutRect9_;
    uint8_t tooltipFactionPower_[0x10];
    RECT tutRect11_;
    RECT tutRect12_;
    RECT tutRect13_;
    RECT tutRect14_;

    void reset();
    void hide();
    void UNK2();
    void UNK3();
    void init(int);
};
typedef char *(__stdcall *CharUpperAFn)(char *);
extern int g_0094b014x[];
extern int g_0096cd04x[];
extern int g_0096cd08x[];

void SocialWin::init(int a1) {
    int *self = reinterpret_cast<int *>(this);
    Buffer localBuffer;

    int i;
    int *btn = self + 0x1808;
    for (i = 0x23; i != 0; --i) {
        reinterpret_cast<VCall *>(btn)->slot090();
        btn += 0x2d3;
    }
    int *bg = self + 0x1eb84 / 4;
    for (i = 4; i != 0; --i) {
        reinterpret_cast<ButtonGroup *>(bg)->close();
        bg += 0x94 / 4;
    }
    reinterpret_cast<ButtonGroup *>(bg)->close();

    self[0xa2c / 4] = -1;
    self[0xa30 / 4] = -1;
    self[0x2c28 / 4] = 0;
    self[0x2c24 / 4] = -1;
    int *lockedField = self + 0xa14 / 4;
    self[0xa18 / 4] = 2;
    int wasNeg = a1 < 0 ? 1 : 0;
    if (a1 < 0) a1 = -a1;
    *g_007ae7f8 = reinterpret_cast<int>(lockedField);
    self[0xcf4 / 4] = a1;
    int notScreen800 = (*g_009b7b1c != 800) ? 1 : 0;

    int *rectA = self + 0x1f2e0 / 4;
    rectA[0] = *g_007af5c0;
    rectA[1] = *g_007af5c4;
    rectA[2] = *g_007af5c8;
    rectA[3] = *g_007af5cc;
    rectA[0] += 4;
    rectA[2] += -4;
    rectA[1] += 4;
    rectA[3] += -4;

    self[0x1f064 / 4] = 4;
    self[0x1f060 / 4] = 0x271;
    self[0x1f068 / 4] = 0x306;
    self[0x1f06c / 4] = 0xfa;

    {
        int v = 0x1d;
        int *p = self + 0x1f078 / 4;
        int cnt = 10;
        do {
            p[-2] = 0x274;
            p[-1] = v;
            p[0] = 0x303;
            p[1] = v + 0x14;
            v += 0x16;
            p += 4;
            --cnt;
        } while (cnt != 0);
    }

    self[0x1f2d0 / 4] = 0x9a; self[0x1f2d4 / 4] = 0x13a; self[0x1f2d8 / 4] = 0x13c; self[0x1f2dc / 4] = 0x14e;
    self[0x1f2c0 / 4] = 0x13f; self[0x1f2c4 / 4] = 0x13a; self[0x1f2c8 / 4] = 0x1e1; self[0x1f2cc / 4] = 0x14e;
    self[0x1f2b0 / 4] = 0x1e4; self[0x1f2b4 / 4] = 0x13a; self[0x1f2b8 / 4] = 0x286; self[0x1f2bc / 4] = 0x14e;
    self[0x1f110 / 4] = 0x1a; self[0x1f114 / 4] = 4; self[0x1f118 / 4] = 0x26e; self[0x1f11c / 4] = 0x4a;
    self[0x1f120 / 4] = 0x1a; self[0x1f124 / 4] = 0x4d; self[0x1f128 / 4] = 0x26e; self[0x1f12c / 4] = 0x93;
    self[0x1f130 / 4] = 0x1a; self[0x1f134 / 4] = 0x96; self[0x1f138 / 4] = 0x26e; self[0x1f13c / 4] = 0xdc;
    self[0x1f140 / 4] = 0x1a; self[0x1f148 / 4] = 0x26e; self[0x1f144 / 4] = 0xdf; self[0x1f14c / 4] = 0x125;
    self[0x1f150 / 4] = 0x1d; self[0x1f154 / 4] = 0x1d; self[0x1f158 / 4] = 0xaf; self[0x1f15c / 4] = 0x31;
    self[0x1f160 / 4] = 0xb1; self[0x1f164 / 4] = 0x1d; self[0x1f168 / 4] = 0x143; self[0x1f16c / 4] = 0x31;
    self[0x1f170 / 4] = 0x145; self[0x1f174 / 4] = 0x1d; self[0x1f178 / 4] = 0x1d7; self[0x1f17c / 4] = 0x31;
    self[0x1f18c / 4] = 0x31; self[0x1f180 / 4] = 0x1d9; self[0x1f184 / 4] = 0x1d; self[0x1f188 / 4] = 0x26b;
    self[0x1f190 / 4] = 0x1d; self[0x1f194 / 4] = 0x66; self[0x1f198 / 4] = 0xaf; self[0x1f19c / 4] = 0x7a;
    self[0x1f1a0 / 4] = 0xb1; self[0x1f1a4 / 4] = 0x66; self[0x1f1a8 / 4] = 0x143; self[0x1f1ac / 4] = 0x7a;
    self[0x1f1b0 / 4] = 0x145; self[0x1f1b4 / 4] = 0x66; self[0x1f1b8 / 4] = 0x1d7; self[0x1f1bc / 4] = 0x7a;
    self[0x1f1c0 / 4] = 0x1d9; self[0x1f1c4 / 4] = 0x66; self[0x1f1c8 / 4] = 0x26b; self[0x1f1cc / 4] = 0x7a;
    self[0x1f1d0 / 4] = 0x1d; self[0x1f1d4 / 4] = 0xaf; self[0x1f1d8 / 4] = 0xaf; self[0x1f1dc / 4] = 0xc3;
    self[0x1f1e0 / 4] = 0xb1; self[0x1f1e4 / 4] = 0xaf; self[0x1f1e8 / 4] = 0x143; self[0x1f1ec / 4] = 0xc3;
    self[0x1f1f0 / 4] = 0x145; self[0x1f1f4 / 4] = 0xaf; self[0x1f1f8 / 4] = 0x1d7; self[0x1f1fc / 4] = 0xc3;
    self[0x1f204 / 4] = 0xaf; self[0x1f208 / 4] = 0x26b; self[0x1f20c / 4] = 0xc3; self[0x1f200 / 4] = 0x1d9;
    self[0x1f218 / 4] = 0xaf;
    self[0x1f210 / 4] = 0x1d; self[0x1f21c / 4] = 0x10c; self[0x1f214 / 4] = 0xf8;
    self[0x1f22c / 4] = 0x10c;
    self[0x1f220 / 4] = 0xb1; self[0x1f224 / 4] = 0xf8; self[0x1f228 / 4] = 0x143;
    self[0x1f230 / 4] = 0x145; self[0x1f234 / 4] = 0xf8; self[0x1f238 / 4] = 0x1d7; self[0x1f23c / 4] = 0x10c;
    self[0x1f240 / 4] = 0x1d9; self[0x1f244 / 4] = 0xf8; self[0x1f248 / 4] = 0x26b; self[0x1f24c / 4] = 0x10c;
    self[0x1f2a0 / 4] = 0x271; self[0x1f2a4 / 4] = 0xfd; self[0x1f2a8 / 4] = 0x306; self[0x1f2ac / 4] = 0x12d;

    int iv5, iv6, iv8, iv9, iv10;
    iv8 = *g_007af5a0 + 7;
    iv5 = *g_007af5dc - 7;
    iv9 = *g_007af5d4 + 7;
    iv6 = ((*g_007af5d8 - iv8) - 4) / 3 - 3 + iv8;
    int *rectB = self + 0x1f260 / 4;
    rectB[0] = iv8;
    rectB[1] = iv9;
    rectB[2] = iv6;
    rectB[3] = iv5;
    iv10 = (iv6 - iv8) + 3;
    iv6 = iv6 + iv10;
    int *rectC = self + 0x1f280 / 4;
    rectC[0] = iv8 + iv10;
    rectC[1] = iv9;
    rectC[2] = iv6;
    rectC[3] = iv5;
    int *rectD = self + 0x1f270 / 4;
    iv8 = *g_007af5d8 - 7;
    rectD[0] = iv6 + 3;
    rectD[1] = iv9;
    rectD[2] = iv8;
    rectD[3] = iv5;
    iv8 = *g_007af5a8 - 4;
    iv6 = *g_007af5a4 + 4;
    int *rectE = self + 0x1f290 / 4;
    rectE[0] = *g_007af5a0 + 4;
    rectE[1] = iv6;
    rectE[2] = iv8;
    rectE[3] = rectD[1] - 6;
    int *rectF = self + 0x1f250 / 4;
    rectF[0] = *g_007af5b0;
    rectF[1] = *g_007af5b4;
    rectF[2] = *g_007af5b8;
    rectF[3] = *g_007af5bc;
    rectF[0] += 4;
    rectF[2] += -4;
    rectF[1] += 4;
    rectF[3] += -4;

    self[0x1f000 / 4] = rectB[0] + 3;
    self[0x1f004 / 4] = rectB[1] + 3;
    self[0x1f008 / 4] = rectB[0] + 0x17;
    self[0x1f00c / 4] = rectB[1] + 0x17;
    self[0x1f010 / 4] = rectB[2] - 0x17;
    self[0x1f014 / 4] = rectB[1] + 3;
    self[0x1f018 / 4] = rectB[2] - 3;
    self[0x1f01c / 4] = rectB[1] + 0x17;
    iv6 = rectC[0];
    self[0x1f040 / 4] = iv6 + 3;
    self[0x1f044 / 4] = rectC[1] + 3;
    self[0x1f048 / 4] = iv6 + 0x17;
    self[0x1f04c / 4] = rectC[1] + 0x17;
    self[0x1f050 / 4] = rectC[2] - 0x17;
    self[0x1f054 / 4] = rectC[1] + 3;
    self[0x1f058 / 4] = rectC[2] - 3;
    self[0x1f05c / 4] = rectC[1] + 0x17;
    self[0x1f020 / 4] = rectD[0] + 3;
    self[0x1f024 / 4] = rectD[1] + 3;
    self[0x1f028 / 4] = rectD[0] + 0x17;
    self[0x1f02c / 4] = rectD[1] + 0x17;
    self[0x1f030 / 4] = rectD[2] - 0x17;
    self[0x1f034 / 4] = rectD[1] + 3;
    self[0x1f038 / 4] = rectD[2] - 3;
    self[0x1f03c / 4] = rectD[1] + 0x17;
    self[0x1efd0 / 4] = rectB[0] + 3;
    self[0x1efd4 / 4] = rectB[3] - 0x11;
    self[0x1efd8 / 4] = rectB[0] + 0x1b;
    iv6 = rectC[0];
    self[0x1efdc / 4] = rectB[3] - 3;
    self[0x1efe0 / 4] = iv6 + 3;
    self[0x1efe4 / 4] = rectC[3] - 0x11;
    self[0x1efe8 / 4] = iv6 + 0x1b;
    self[0x1efec / 4] = rectC[3] - 3;
    int local20 = rectD[0] + 3;
    int local14 = rectD[3] - 3;
    self[0x1eff0 / 4] = local20;
    int local1c = rectD[3] - 0x11;
    int local18 = rectD[0] + 0x1b;
    self[0x1eff4 / 4] = local1c;
    self[0x1eff8 / 4] = local18;
    self[0x1effc / 4] = local14;

    reinterpret_cast<Font *>(self + 0x1ee68 / 4)->init(reinterpret_cast<char *>(g_00691b2c), 0xe, 1);
    reinterpret_cast<Font *>(self + 0x1ee90 / 4)->init(reinterpret_cast<char *>(g_00691b2c), 0xe, 1);
    reinterpret_cast<Font *>(self + 0x1eeb8 / 4)->init(reinterpret_cast<char *>(g_00691b30), 0xe, 1);
    reinterpret_cast<Font *>(self + 0x1eee0 / 4)->init(reinterpret_cast<char *>(g_00691b30), 0xe, 1);
    reinterpret_cast<Font *>(self + 0x1ef08 / 4)->init(reinterpret_cast<char *>(g_00691b2c), 0xc, 0);
    reinterpret_cast<Font *>(self + 0x1ef30 / 4)->init(reinterpret_cast<char *>(g_00691b2c), 0xb, 1);
    self[0x1ef30 / 4] = 3;
    reinterpret_cast<Font *>(self + 0x1ef58 / 4)->init(reinterpret_cast<char *>(g_00691b2c), 0xb, 0);
    self[0x1ef58 / 4] = 3;
    reinterpret_cast<Font *>(self + 0x1ef80 / 4)->init(reinterpret_cast<char *>(g_00691b30), 0xe, 1);
    reinterpret_cast<Font *>(self + 0x1efa8 / 4)->init(reinterpret_cast<char *>(g_00691b30), 0xe, 1);

    if (self[0xb87] == 0) {
        localBuffer.load_pcx(reinterpret_cast<const char *>(g_006874b0), 0, 10, 0xec);
        int *icon = self + 0x2e18 / 4;
        int row = 1;
        int rowCount = 10;
        do {
            int col = 1;
            int colCountOuter = 3;
            do {
                int colCountInner = 3;
                do {
                    reinterpret_cast<Sprite *>(icon)->extract(&localBuffer, *g_00696d14, col, row, 0x1a, 0x1a, 0);
                    icon += 0x2c / 4;
                    col += 0x1b;
                    --colCountInner;
                } while (colCountInner != 0);
                --colCountOuter;
            } while (colCountOuter != 0);
            row += 0x1b;
            --rowCount;
        } while (rowCount != 0);
        reinterpret_cast<Sprite *>(self + 0x2e18 / 4)->extract(&localBuffer, *g_00696d14, 0xf5, 1, 0x18, 0xe, 0);
        reinterpret_cast<Sprite *>(self + 0x2e18 / 4)->extract(&localBuffer, *g_00696d14, 0xf5, 0x10, 0x18, 0xe, 0);
        {
            int x = 1;
            do {
                reinterpret_cast<Sprite *>(self + 0x3d90 / 4)->extract(&localBuffer, *g_00696d14, 0x13b, x, 0x14, 0x14, 0);
                reinterpret_cast<Sprite *>(self + 0x3dbc / 4)->extract(&localBuffer, *g_00696d14, 0x150, x, 0x14, 0x14, 0);
                x += 0x15;
            } while (x < 0x40);
        }
        {
            int y = 0x10e;
            int *buf = self + 0x3ef0 / 4;
            int outer = 3;
            do {
                int x = 1;
                int inner = 2;
                do {
                    reinterpret_cast<Buffer *>(buf)->init(0xe, 0xe, 0, 0);
                    reinterpret_cast<Buffer *>(buf)->copy(reinterpret_cast<Buffer *>(0), y, x, 0, 0, 0xe, 0xe);
                    x += 0xf;
                    buf += 0x162;
                    --inner;
                } while (inner != 0);
                y += 0xf;
                --outer;
            } while (outer != 0);
        }
    }
    {
        int x = 5;
        int y = 0x115;
        do {
            reinterpret_cast<Sprite *>(self + 0x2c60 / 4)->extract(&localBuffer, *g_00696d14, y, 0x6f, 0xd, 0xd, 0);
            reinterpret_cast<Sprite *>(self + 0x2c8c / 4)->extract(&localBuffer, *g_00696d14, y, 0x7d, 0xd, 0xd, 0);
            y += 0xe;
            --x;
        } while (x != 0);
    }

    {
        int base800 = *g_009b7b1c;
        int base800b = *g_009b7b20;
        int cx = (base800 - 0x321) / 2;
        int cy = (base800b - *g_007af59c) - 0x162;
        int flags = 0x1000000 | (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009156b0) + 4) + reinterpret_cast<int>(g_009156b0));
        GraphicWin::init(cx, cy, 0x341, 0x162, 0, flags, 0, 0, 0);
    }
    GraphicWin::fill(9);
    {
        RECT boxRect;
        boxRect.left = 0x17;
        boxRect.top = 0;
        boxRect.right = 0x30a;
        boxRect.bottom = 0x162;
        reinterpret_cast<Buffer *>(self + 0x444 / 4)->box_sprite(&boxRect, reinterpret_cast<BoxSpriteParams *>(g_0078d690));
    }
    {
        int fac = self[0xcf4 / 4];
        int idxF = fac * 0x833;
        int netMin = 10 - g_0096cd08x[idxF] - g_0096cd04x[idxF];
        self[0x2c38 / 4] = netMin;
        self[0x2c50 / 4] = netMin;
        self[0x2c44 / 4] = netMin;
        int lo = g_0096cd04x[idxF];
        self[0x2c3c / 4] = lo;
        self[0x2c54 / 4] = lo;
        self[0x2c48 / 4] = lo;
        int hi = g_0096cd08x[idxF];
        self[0x2c40 / 4] = hi;
        self[0x2c58 / 4] = hi;
        self[0x2c4c / 4] = hi;
    }
    reset();

    {
        int *group = self + 0x1eb84 / 4;
        int row = 0;
        int rowAccum = 0;
        do {
            reinterpret_cast<ButtonGroup *>(group)->init(row, 0);
            int col = 0;
            do {
                int avail = society_avail(row, col, self[0xcf4 / 4]);
                if (avail != 0) {
                    char *msg = reinterpret_cast<char *>(g_009b86a0);
                    *msg = 0;
                    int strId = g_0094b014x[rowAccum + col];
                    char *text = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(strId));
                    strcat(msg, text);
                    (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(msg);
                    int idx = col + row * 4;
                    int *rect = self + (idx + 0x1f15 + row * 4) * 4;
                    int rectLeft = rect[0];
                    int *flatBtn = self + idx * 0x2d3 + 0x1808;
                    reinterpret_cast<BaseButton *>(flatBtn)->init(msg, idx, rectLeft, rect[1], rect[2] - rectLeft,
                                                                    rect[3] - rect[1], reinterpret_cast<Win *>(self), 0);
                    reinterpret_cast<VCall *>(flatBtn)->slot002();
                    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(flatBtn) + 0x444)->set_font(
                        reinterpret_cast<Font *>(self + 0x1ee90 / 4), 0, 0, 0);
                    reinterpret_cast<VCall *>(flatBtn)->slot062();
                    char *bubble = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*(g_009b90f8 + 0xcf0 / 4)));
                    reinterpret_cast<BaseButton *>(flatBtn)->set_bubble_text(bubble);
                    reinterpret_cast<ButtonGroup *>(group)->add(reinterpret_cast<BaseButton *>(flatBtn));
                }
                ++col;
            } while (col < 4);
            reinterpret_cast<ButtonGroup *>(group)->set(self[row + 0x33e], 1);
            ++row;
            rowAccum += 0x35;
            group += 0x94 / 4;
        } while (rowAccum < 0xd4);
    }

    reinterpret_cast<ButtonGroup *>(self + 0x1edd4 / 4)->init(2, 0);
    {
        char *name = reinterpret_cast<char *>(g_00946598);
        char *nameEnd = reinterpret_cast<char *>(g_009466d0);
        int row = 0;
        int fontIdx = 0;
        int *entry = 0;
        do {
            if (name != nameEnd) {
                int col;
                int rowBase;
                if (reinterpret_cast<int>(name) < 0x9466d0) {
                    col = row << 2;
                    rowBase = fontIdx;
                } else {
                    col = row * 4 - 4;
                    entry = reinterpret_cast<int *>(reinterpret_cast<char *>(entry) - 0x10);
                    rowBase = fontIdx - 0xb4c;
                }
                char *msg = reinterpret_cast<char *>(g_009b86a0);
                *msg = 0;
                strcat(msg, name);
                (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(msg);
                int y = *reinterpret_cast<int *>(reinterpret_cast<char *>(entry) + 0x1f074 + reinterpret_cast<int>(self));
                int x = *reinterpret_cast<int *>(reinterpret_cast<char *>(entry) + 0x1f070 + reinterpret_cast<int>(self));
                int *btnPtr = reinterpret_cast<int *>(rowBase + 0x1202c + reinterpret_cast<int>(self));
                reinterpret_cast<BaseButton *>(btnPtr)->init(
                    msg, col + 0x11, x, y,
                    *reinterpret_cast<int *>(reinterpret_cast<char *>(entry) + 0x1f078 + reinterpret_cast<int>(self)) - x,
                    *reinterpret_cast<int *>(reinterpret_cast<char *>(entry) + 0x1f07c + reinterpret_cast<int>(self)) - y,
                    reinterpret_cast<Win *>(self), 0);
                reinterpret_cast<VCall *>(btnPtr)->slot002();
                reinterpret_cast<Buffer *>(reinterpret_cast<char *>(btnPtr) + 0x444)->set_font(
                    reinterpret_cast<Font *>(0), 0, 0, 0);
                reinterpret_cast<VCall *>(btnPtr)->slot062();
                char *bubble = reinterpret_cast<char *>(
                    reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + col * 4 + 0xcf4)));
                reinterpret_cast<BaseButton *>(btnPtr)->set_bubble_text(bubble);
            }
            name += 0x68;
            ++row;
            entry = reinterpret_cast<int *>(reinterpret_cast<char *>(entry) + 0x10);
            fontIdx += 0xb4c;
        } while (name < reinterpret_cast<char *>(g_00946a10));
    }

    {
        char *msg = reinterpret_cast<char *>(g_009b86a0);
        char *s = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0x198)));
        *msg = 0;
        strcat(msg, s);
        if (*g_009bc054 != 2) {
            (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(msg);
        }
        int left = self[0x7cac], top = self[0x7cad], right = self[0x7cae], bottom = self[0x7caf];
        int *btnPtr = self + 0x114e0 / 4;
        reinterpret_cast<BaseButton *>(btnPtr)->init(msg, 0x10, left, top, right - left, bottom - top,
                                                       reinterpret_cast<Win *>(self), 0);
        reinterpret_cast<VCall *>(btnPtr)->slot002();
        reinterpret_cast<Buffer *>(reinterpret_cast<char *>(btnPtr) + 0x444)->set_font(
            reinterpret_cast<Font *>(0), 0, 0, 0);
        reinterpret_cast<VCall *>(btnPtr)->slot062();
    }

    reinterpret_cast<BaseButton *>(self + 0x19124 / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0x4f4))));

    reinterpret_cast<BaseButton *>(self + 0x19c70 / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd34))));

    {
        int *cb0 = self + 0xd6c / 4;
        int *cb1 = cb0 + 0xa2c / 4;
        int *cb2 = cb1 + 0xa2c / 4;
        self[0x5e4] = reinterpret_cast<int>(self + 0xf64);
        self[0x5e5] = reinterpret_cast<int>(self + 0xf6f);
        reinterpret_cast<CheckButton *>(cb0)->init(self[0x7bf4], self[0x7bf5], reinterpret_cast<GraphicWin *>(g_007ae820), 0);
        self[0x86f] = reinterpret_cast<int>(self + 0xf64);
        self[0x870] = reinterpret_cast<int>(self + 0xf6f);
        reinterpret_cast<CheckButton *>(cb1)->init(self[0x7bf8], self[0x7bf9], reinterpret_cast<GraphicWin *>(g_007ae820), 0);
        self[0xafa] = reinterpret_cast<int>(self + 0xf64);
        self[0xafb] = reinterpret_cast<int>(self + 0xf6f);
        reinterpret_cast<CheckButton *>(cb2)->init(self[0x7bfc], self[0x7bfd], reinterpret_cast<GraphicWin *>(g_007ae820), 0);

        int *p = cb0;
        int cnt = 3;
        do {
            reinterpret_cast<VCall *>(p)->slot002();
            p += 0x28b;
            --cnt;
        } while (cnt != 0);
    }

    reinterpret_cast<BaseButton *>(self + 0x69ef / 4)->init(0, 0x1d, self[0x7c00], self[0x7c01],
        self[0x7c02] - self[0x7c00], self[0x7c03] - self[0x7c01], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x69ef / 4)->slot002();
    {
        int *p68 = self + 0xf7a / 4;
        self[0x6ca5] = reinterpret_cast<int>(self + 0xf85);
        self[0x6ca4] = reinterpret_cast<int>(p68);
        self[0x6ca6] = reinterpret_cast<int>(self + 0xf90);
        reinterpret_cast<BaseButton *>(self + 0x69ef / 4)->set_bubble_text(
            reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd24))));
    }

    reinterpret_cast<BaseButton *>(self + 0x6cc2 / 4)->init(0, 0x1e, self[0x7c04], self[0x7c05],
        self[0x7c06] - self[0x7c04], self[0x7c07] - self[0x7c05], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x6cc2 / 4)->slot002();
    self[0x6f77] = reinterpret_cast<int>(self + 0xf9b);
    self[0x6f78] = reinterpret_cast<int>(self + 0xfa6);
    self[0x6f79] = reinterpret_cast<int>(self + 0xfb1);
    reinterpret_cast<BaseButton *>(self + 0x6cc2 / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd24))));

    reinterpret_cast<BaseButton *>(self + 0x6f95 / 4)->init(0, 0x1f, self[0x7c08], self[0x7c09],
        self[0x7c0a] - self[0x7c08], self[0x7c0b] - self[0x7c09], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x6f95 / 4)->slot002();
    self[0x724a] = reinterpret_cast<int>(self + 0xf7a / 4);
    self[0x724b] = reinterpret_cast<int>(self + 0xf85);
    self[0x724c] = reinterpret_cast<int>(self + 0xf90);
    reinterpret_cast<BaseButton *>(self + 0x6f95 / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd28))));

    reinterpret_cast<BaseButton *>(self + 0x7268 / 4)->init(0, 0x20, self[0x7c0c], self[0x7c0d],
        self[0x7c0e] - self[0x7c0c], self[0x7c0f] - self[0x7c0d], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x7268 / 4)->slot002();
    self[0x751d] = reinterpret_cast<int>(self + 0xf9b);
    self[0x751e] = reinterpret_cast<int>(self + 0xfa6);
    self[0x751f] = reinterpret_cast<int>(self + 0xfb1);
    reinterpret_cast<BaseButton *>(self + 0x7268 / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd28))));

    reinterpret_cast<BaseButton *>(self + 0x753b / 4)->init(0, 0x21, self[0x7c10], self[0x7c11],
        self[0x7c12] - self[0x7c10], self[0x7c13] - self[0x7c11], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x753b / 4)->slot002();
    self[0x77f0] = reinterpret_cast<int>(self + 0xf7a / 4);
    self[0x77f1] = reinterpret_cast<int>(self + 0xf85);
    self[0x77f2] = reinterpret_cast<int>(self + 0xf90);
    reinterpret_cast<BaseButton *>(self + 0x753b / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd2c))));

    reinterpret_cast<BaseButton *>(self + 0x780e / 4)->init(0, 0x22, self[0x7c14], self[0x7c15],
        self[0x7c16] - self[0x7c14], self[0x7c17] - self[0x7c15], reinterpret_cast<Win *>(g_007ae820), 0);
    reinterpret_cast<VCall *>(self + 0x780e / 4)->slot002();
    self[0x7ac3] = reinterpret_cast<int>(self + 0xf9b);
    self[0x7ac4] = reinterpret_cast<int>(self + 0xfa6);
    self[0x7ac5] = reinterpret_cast<int>(self + 0xfb1);
    reinterpret_cast<BaseButton *>(self + 0x780e / 4)->set_bubble_text(
        reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009b90f8) + 0xd2c))));

    if (wasNeg == 0) {
        if (tut_check2(8) != 0) {
            reinterpret_cast<VCall *>(this)->slot001(0);
            *g_008cc228 = 5;
            int outA = 0, outB = 0;
            reinterpret_cast<TutWin *>(g_008c6e68)->soc_rect(reinterpret_cast<RECT *>(g_008c5380), &outA, &outB);
            int *baseObj = *reinterpret_cast<int **>(g_007d3c3c);
            int gwArg = 0;
            if (baseObj != 0) {
                gwArg = *reinterpret_cast<int *>(*reinterpret_cast<int *>(baseObj) + 4) + reinterpret_cast<int>(baseObj);
            }
            reinterpret_cast<TutWin *>(g_008c6e68)->tut_win(reinterpret_cast<GraphicWin *>(gwArg),
                reinterpret_cast<const char *>(g_006874bc), outA, outB, 0, 1, -1, -1);
        }
        reinterpret_cast<VCall *>(this)->slot057(0, g_005398e0, g_007ae820);
        reinterpret_cast<SubInterface *>(this)->release_iface_mode();
    }
}
