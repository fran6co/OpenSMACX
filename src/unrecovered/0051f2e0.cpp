// ORIGINAL: 0x0051F2E0 ?random_events@@YAXH@Z 0x0051F2E0-0x0052211D FILE
// RULED-OUT: mechanical register/stack-slot goto transliteration (260 labels, 39 independent per-case epilogues auto-detected from the pop/pop/ pop/mov-esp-ebp/pop/ret shape, a 22-case switch off 0x0097D044, two thiscall-via-fixed-ecx targets on Console/MapWin, one indirect GetKeyState call through g_00669330): compiles, MISMATCH sim 0.32 on /O2 /Oy- framed, divergence at instruction #33. Large negative 32-bit immediates (e.g. 0xfffffc18) need an explicit (int) cast before a signed compare, or MSVC types the literal unsigned and turns jge into jae - fixed once, may still be lurking elsewhere unverified.
// working copy - scaffold materialised by --work
// size      11837 bytes
// prototype 
// callers   1   call targets   36
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00405140 0x0046AEF0 0x0046AF40 0x0046B190 0x0046B1F0 0x0048C0A0 0x004E39D0 0x004E3A50 0x004E3EF0 0x004E5250 0x004EC3B0 0x0050BA00 0x0050DD90 0x005108A0 0x00514880 0x0055BB30 0x00591BC0 0x00591D60 0x00591E50 0x00592030 0x005AC060 0x005B9F20 0x005BE6B0 0x005C08C0 0x005C5A30 0x005C5BD0 0x005C5C70 0x005C89A0 0x006257E0 0x00625810 0x00625E30 0x00625E50 0x00625EC0 0x00644F3A 0x006453E0 0x00645470
// indirect  0x00520291 0x005202CE 0x00520313 0x00520343 0x005203D1 0x0052040E 0x00520453 0x00520938 0x005209D3 0x0052138C 0x00521422 0x005216D6 0x00521757 0x00521D53 0x00521DE2 0x0052201F 0x005220A5

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?random_events@@YAXH@Z  at 0x0051F2E0  (11837 bytes)
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
typedef char * LPSTR;
typedef void * LPVOID;
class MapWin;
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
    void UNK3();
    void close();
    void do_image_buttons();
    void main_caption();
    void on_left_double_click(int, int);
    void on_left_up(int, int);
    void on_redraw();
    void set_center(int, int, int);
};

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
    int edit_lock();
    void clear_group();
    void editor_climate();
    void editor_polar();
    void editor_redo();
    void editor_undo();
    void focus(int, int, int);
    void menu_update();
    void on_sys_close();
    void set_adv_preferences();
    void set_audiovisual();
    void set_auto_preferences();
    void set_base_preferences();
    void set_map_display();
    void set_preferences();
    void update_data(int);
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

bool has_tech(int, int);
extern "C" char *strcat(char *, const char *);
extern "C" int abs(int);
extern "C" unsigned int strlen(const char *);
int POP2(const char *, const char *, int);
int base_at(int, int);
int base_compute(int);
int bonus_at(int, int, int);
int game_year(int);
int is_objective(int);
int near_landmark(int, int);
int parse_num(int, int);
int parse_say(int, int, int, int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int tech_rate(int);
int whose_territory(int, int, int, int *, int);
unsigned int random(int, int);
void base_kill(int);
void bit_set(int, int, int, int);
void bitmask(int, int *, int *);
void clock_wait(int);
void draw_map(int);
void draw_radius(int, int, int, int);
void draw_tile(int, int, int);
void random_reseed(unsigned long);
void rocky_set(int, int, int);
void set_base(int);
void set_treaty(int, int, int, int);
void synch_bit(int, int, int);
void veh_kill(int);
void world_climate();
void world_crater(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00522120 = (int *)0x00522120;
static int *const g_00669330 = (int *)0x00669330;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_0068b084 = (int *)0x0068B084;
static int *const g_0068b094 = (int *)0x0068B094;
static int *const g_0068b0a0 = (int *)0x0068B0A0;
static int *const g_0068b0b0 = (int *)0x0068B0B0;
static int *const g_0068b0bc = (int *)0x0068B0BC;
static int *const g_0068b0cc = (int *)0x0068B0CC;
static int *const g_0068b0dc = (int *)0x0068B0DC;
static int *const g_0068b0e8 = (int *)0x0068B0E8;
static int *const g_0068b0f4 = (int *)0x0068B0F4;
static int *const g_0068b0fc = (int *)0x0068B0FC;
static int *const g_0068b108 = (int *)0x0068B108;
static int *const g_0068b110 = (int *)0x0068B110;
static int *const g_0068b120 = (int *)0x0068B120;
static int *const g_0068b130 = (int *)0x0068B130;
static int *const g_0068b13c = (int *)0x0068B13C;
static int *const g_0068b14c = (int *)0x0068B14C;
static int *const g_0068b154 = (int *)0x0068B154;
static int *const g_0068b168 = (int *)0x0068B168;
static int *const g_0068b174 = (int *)0x0068B174;
static int *const g_0068b180 = (int *)0x0068B180;
static int *const g_0068b18c = (int *)0x0068B18C;
static int *const g_0068b19c = (int *)0x0068B19C;
static int *const g_0068b1ac = (int *)0x0068B1AC;
static int *const g_0068b1b8 = (int *)0x0068B1B8;
static int *const g_0068b1c8 = (int *)0x0068B1C8;
static int *const g_0068b1d4 = (int *)0x0068B1D4;
static int *const g_0068b1e4 = (int *)0x0068B1E4;
static int *const g_0068b1f0 = (int *)0x0068B1F0;
static int *const g_0068b200 = (int *)0x0068B200;
static int *const g_0068b20c = (int *)0x0068B20C;
static int *const g_0068b21c = (int *)0x0068B21C;
static int *const g_0068b228 = (int *)0x0068B228;
static int *const g_0068b23c = (int *)0x0068B23C;
static int *const g_0068b248 = (int *)0x0068B248;
static int *const g_0068b258 = (int *)0x0068B258;
static int *const g_0068b268 = (int *)0x0068B268;
static int *const g_0068b278 = (int *)0x0068B278;
static int *const g_0068b2a8 = (int *)0x0068B2A8;
static int *const g_0068b2b4 = (int *)0x0068B2B4;
static int *const g_0068b2c0 = (int *)0x0068B2C0;
static int *const g_0068b2cc = (int *)0x0068B2CC;
static int *const g_0068b2d8 = (int *)0x0068B2D8;
static int *const g_0068b2e4 = (int *)0x0068B2E4;
static int *const g_0068b2f0 = (int *)0x0068B2F0;
static int *const g_0068b2fc = (int *)0x0068B2FC;
static int *const g_0068b30c = (int *)0x0068B30C;
static int *const g_0068b318 = (int *)0x0068B318;
static int *const g_0068b328 = (int *)0x0068B328;
static int *const g_0068b334 = (int *)0x0068B334;
static int *const g_0068b344 = (int *)0x0068B344;
static int *const g_0068b350 = (int *)0x0068B350;
static int *const g_0068b360 = (int *)0x0068B360;
static int *const g_0068b36c = (int *)0x0068B36C;
static int *const g_0068b378 = (int *)0x0068B378;
static int *const g_0068b380 = (int *)0x0068B380;
static int *const g_0068b38c = (int *)0x0068B38C;
static int *const g_0068b39c = (int *)0x0068B39C;
static int *const g_0068b3a8 = (int *)0x0068B3A8;
static int *const g_0068b3b0 = (int *)0x0068B3B0;
static int *const g_0068b3c0 = (int *)0x0068B3C0;
static int *const g_0068b3c8 = (int *)0x0068B3C8;
static int *const g_0068b3d0 = (int *)0x0068B3D0;
static int *const g_0068b3e0 = (int *)0x0068B3E0;
static int *const g_0068b3e8 = (int *)0x0068B3E8;
static int *const g_0068b3f0 = (int *)0x0068B3F0;
static int *const g_0068b400 = (int *)0x0068B400;
static int *const g_0068b40c = (int *)0x0068B40C;
static int *const g_0068b41c = (int *)0x0068B41C;
static int *const g_0068b428 = (int *)0x0068B428;
static int *const g_0068b438 = (int *)0x0068B438;
static int *const g_0068b440 = (int *)0x0068B440;
static int *const g_0068b44c = (int *)0x0068B44C;
static int *const g_0068b454 = (int *)0x0068B454;
static int *const g_0068b464 = (int *)0x0068B464;
static int *const g_0068b46c = (int *)0x0068B46C;
static int *const g_0068b478 = (int *)0x0068B478;
static int *const g_0068b480 = (int *)0x0068B480;
static int *const g_0068b48c = (int *)0x0068B48C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0094615c = (int *)0x0094615C;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_00949878 = (int *)0x00949878;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095282a = (int *)0x0095282A;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9e4 = (int *)0x0096C9E4;
static int *const g_0096c9e8 = (int *)0x0096C9E8;
static int *const g_0096cc00 = (int *)0x0096CC00;
static int *const g_0096cda8 = (int *)0x0096CDA8;
static int *const g_0096cdb8 = (int *)0x0096CDB8;
static int *const g_0096da3c = (int *)0x0096DA3C;
static int *const g_0096eac8 = (int *)0x0096EAC8;
static int *const g_0096eae4 = (int *)0x0096EAE4;
static int *const g_0096f2b8 = (int *)0x0096F2B8;
static int *const g_0096f2c0 = (int *)0x0096F2C0;
static int *const g_0097d040 = (int *)0x0097D040;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d046 = (int *)0x0097D046;
static int *const g_0097d047 = (int *)0x0097D047;
static int *const g_0097d04a = (int *)0x0097D04A;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_0097d074 = (int *)0x0097D074;
static int *const g_0097d0cc = (int *)0x0097D0CC;
static int *const g_0097d110 = (int *)0x0097D110;
static int *const g_0097d164 = (int *)0x0097D164;
static int *const g_0097d84c = (int *)0x0097D84C;
static int *const g_0097d854 = (int *)0x0097D854;
static int *const g_009a4bdc = (int *)0x009A4BDC;
static int *const g_009a4ccc = (int *)0x009A4CCC;
static int *const g_009a4cfc = (int *)0x009A4CFC;
static int *const g_009a4d2c = (int *)0x009A4D2C;
static int *const g_009a4e98 = (int *)0x009A4E98;
static int *const g_009a4ec8 = (int *)0x009A4EC8;
static int *const g_009a5888 = (int *)0x009A5888;
static int *const g_009a5b28 = (int *)0x009A5B28;
static int *const g_009a5d68 = (int *)0x009A5D68;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6508 = (int *)0x009A6508;
static int *const g_009a6514 = (int *)0x009A6514;
static int *const g_009a654c = (int *)0x009A654C;
static int *const g_009a657c = (int *)0x009A657C;
static int *const g_009a65a4 = (int *)0x009A65A4;
static int *const g_009a6800 = (int *)0x009A6800;
static int *const g_009a6804 = (int *)0x009A6804;
static int *const g_009a6808 = (int *)0x009A6808;
static int *const g_009a680c = (int *)0x009A680C;
static int *const g_009a6818 = (int *)0x009A6818;
static int *const g_009ab88f = (int *)0x009AB88F;
static int *const g_009ab892 = (int *)0x009AB892;
static int *const g_009b86a0 = (int *)0x009B86A0;
#pragma function(strcat, strlen)
#define SETB0(dst,val) ((dst) = (((dst) & ~0xff) | ((val) & 0xff)))
#define SETB1(dst,val) ((dst) = (((dst) & ~0xff00) | (((val) & 0xff) << 8)))
#define SETW0(dst,val) ((dst) = (((dst) & ~0xffff) | ((val) & 0xffff)))

void __cdecl random_events(int a1) {
    int eax=0, ebx=0, ecx=0, edx=0, esi=0, edi=0;
    int var_4, var_8, var_c, var_10, var_14, var_18, var_1c, var_20;

    // 0051F2E6  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0051F2EC  test eax, eax
    // 0051F2EE  jne 0x51f479
    if ((((eax) & (eax))) != 0) goto LAB_0051F479;
    // 0051F2F4  mov eax, dword ptr [0x9a6818]
    eax = ((*(int*)((char*)g_009a6818)));
    // 0051F2F9  test al, 2
    // 0051F2FB  je 0x51f304
    if (((((eax & 0xff)) & (2))) == 0) goto LAB_0051F304;
    // 0051F2FD  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051F2FF  mov dword ptr [0x9a6818], eax
    (*(int*)((char*)g_009a6818)) = (eax);
LAB_0051F304:
    // 0051F304  test al, 1
    // 0051F306  je 0x51f312
    if (((((eax & 0xff)) & (1))) == 0) goto LAB_0051F312;
    // 0051F308  mov dword ptr [0x9a6818], 2
    (*(int*)((char*)g_009a6818)) = (2);
LAB_0051F312:
    // 0051F312  mov eax, dword ptr [0x9a6800]
    eax = ((*(int*)((char*)g_009a6800)));
    // 0051F317  cmp eax, 0xfffffc18
    // 0051F31C  jge 0x51f323
    if (((int)(eax)) >= ((int)(0xfffffc18))) goto LAB_0051F323;
    // 0051F31E  mov eax, 0xfffffc18
    eax = (0xfffffc18);
LAB_0051F323:
    // 0051F323  mov ecx, dword ptr [0x9a680c]
    ecx = ((*(int*)((char*)g_009a680c)));
    // 0051F329  test ecx, ecx
    // 0051F32B  jge 0x51f337
    if (((int)(((ecx) & (ecx)))) >= 0) goto LAB_0051F337;
    // 0051F32D  mov dword ptr [0x9a680c], 0
    (*(int*)((char*)g_009a680c)) = (0);
LAB_0051F337:
    // 0051F337  dec eax
    eax = ((eax) - 1);
    // 0051F338  mov dword ptr [0x9a6800], eax
    (*(int*)((char*)g_009a6800)) = (eax);
    // 0051F33D  jne 0x51f353
    if ((eax) != (0x0)) goto LAB_0051F353;
    // 0051F33F  push -1
    // 0051F341  push 0x68b084
    // 0051F346  push 0x68b094
    // 0051F34B  call 0x405140
    eax = POP2((const char*)(0x68b094), (const char*)(0x68b084), -1);
    // 0051F350  add esp, 0xc
LAB_0051F353:
    // 0051F353  mov dl, byte ptr [0x9a64e8]
    SETB0(edx, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0051F359  mov edi, 1
    edi = (1);
    // 0051F35E  mov dword ptr [ebp - 0x1c], 0x96eac8
    var_1c = (0x96eac8);
LAB_0051F365:
    // 0051F365  mov eax, 1
    eax = (1);
    // 0051F36A  mov ecx, edi
    ecx = (edi);
    // 0051F36C  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 0051F36E  test dl, al
    // 0051F370  je 0x51f3af
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_0051F3AF;
    // 0051F372  mov ebx, dword ptr [ebp - 0x1c]
    ebx = (var_1c);
    // 0051F375  mov esi, 1
    esi = (1);
LAB_0051F37A:
    // 0051F37A  mov eax, 1
    eax = (1);
    // 0051F37F  mov ecx, esi
    ecx = (esi);
    // 0051F381  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 0051F383  test dl, al
    // 0051F385  jne 0x51f3a3
    if (((((edx & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_0051F3A3;
    // 0051F387  test byte ptr [ebx], 0x30
    // 0051F38A  jne 0x51f3a3
    if (((((*(unsigned char*)((char*)(ebx)))) & (0x30))) != 0) goto LAB_0051F3A3;
    // 0051F38C  push 0
    // 0051F38E  push 0xc00
    // 0051F393  push edi
    // 0051F394  push esi
    // 0051F395  call 0x55bb30
    set_treaty(esi, edi, 0xc00, 0);
    // 0051F39A  mov dl, byte ptr [0x9a64e8]
    SETB0(edx, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0051F3A0  add esp, 0x10
LAB_0051F3A3:
    // 0051F3A3  inc esi
    esi = ((esi) + 1);
    // 0051F3A4  add ebx, 0x20cc
    ebx = ((ebx) + (0x20cc));
    // 0051F3AA  cmp esi, 8
    // 0051F3AD  jl 0x51f37a
    if (((int)(esi)) < ((int)(8))) goto LAB_0051F37A;
LAB_0051F3AF:
    // 0051F3AF  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 0051F3B2  inc edi
    edi = ((edi) + 1);
    // 0051F3B3  add eax, 4
    eax = ((eax) + (4));
    // 0051F3B6  cmp eax, 0x96eae4
    // 0051F3BB  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 0051F3BE  jl 0x51f365
    if (((int)(eax)) < ((int)(0x96eae4))) goto LAB_0051F365;
    // 0051F3C0  mov eax, dword ptr [0x9a680c]
    eax = ((*(int*)((char*)g_009a680c)));
    // 0051F3C5  test eax, eax
    // 0051F3C7  jle 0x51f3e5
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_0051F3E5;
    // 0051F3C9  dec eax
    eax = ((eax) - 1);
    // 0051F3CA  mov dword ptr [0x9a680c], eax
    (*(int*)((char*)g_009a680c)) = (eax);
    // 0051F3CF  jne 0x51f3e5
    if ((eax) != (0x0)) goto LAB_0051F3E5;
    // 0051F3D1  push -1
    // 0051F3D3  push 0x68b0a0
    // 0051F3D8  push 0x68b0b0
    // 0051F3DD  call 0x405140
    eax = POP2((const char*)(0x68b0b0), (const char*)(0x68b0a0), -1);
    // 0051F3E2  add esp, 0xc
LAB_0051F3E5:
    // 0051F3E5  mov edx, dword ptr [0x9a64cc]
    edx = ((*(int*)((char*)g_009a64cc)));
    // 0051F3EB  test edx, edx
    // 0051F3ED  jle 0x51f420
    if (((int)(((edx) & (edx)))) <= 0) goto LAB_0051F420;
    // 0051F3EF  mov eax, 0x97d164
    eax = (0x97d164);
    // 0051F3F4  mov esi, 0xffff81ff
    esi = (0xffff81ff);
LAB_0051F3F9:
    // 0051F3F9  mov ecx, dword ptr [eax - 0xf0]
    ecx = ((*(int*)((char*)(eax) + (-0xf0))));
    // 0051F3FF  test ch, 0x7e
    // 0051F402  je 0x51f418
    if ((((((ecx >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F418;
    // 0051F404  mov ecx, dword ptr [eax]
    ecx = ((*(int*)((char*)(eax))));
    // 0051F406  test ecx, ecx
    // 0051F408  jl 0x51f40d
    if (((int)(((ecx) & (ecx)))) < 0) goto LAB_0051F40D;
    // 0051F40A  dec ecx
    ecx = ((ecx) - 1);
    // 0051F40B  mov dword ptr [eax], ecx
    (*(int*)((char*)(eax))) = (ecx);
LAB_0051F40D:
    // 0051F40D  cmp dword ptr [eax], 0
    // 0051F410  jne 0x51f418
    if (((*(int*)((char*)(eax)))) != (0)) goto LAB_0051F418;
    // 0051F412  and dword ptr [eax - 0xf0], esi
    (*(int*)((char*)(eax) + (-0xf0))) = (((*(int*)((char*)(eax) + (-0xf0)))) & (esi));
LAB_0051F418:
    // 0051F418  add eax, 0x134
    eax = ((eax) + (0x134));
    // 0051F41D  dec edx
    edx = ((edx) - 1);
    // 0051F41E  jne 0x51f3f9
    if ((edx) != (0x0)) goto LAB_0051F3F9;
LAB_0051F420:
    // 0051F420  test dword ptr [0x9a64c0], 0x100000
    // 0051F42A  je 0x51f479
    if (((((*(int*)((char*)g_009a64c0))) & (0x100000))) == 0) goto LAB_0051F479;
    // 0051F42C  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 0051F432  push ecx
    // 0051F433  call 0x5c89a0
    eax = game_year(ecx);
    // 0051F438  sub eax, 0x8a2
    eax = ((eax) - (0x8a2));
    // 0051F43D  mov ecx, 0x50
    ecx = (0x50);
    // 0051F442  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 0051F443  idiv ecx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ecx)); edx = (int)(t_dividend % (int)(ecx)); }
    // 0051F445  add esp, 4
    // 0051F448  test edx, edx
    // 0051F44A  jne 0x51f479
    if ((((edx) & (edx))) != 0) goto LAB_0051F479;
    // 0051F44C  mov edi, dword ptr [0x9a64c0]
    edi = ((*(int*)((char*)g_009a64c0)));
    // 0051F452  push -1
    // 0051F454  and edi, 0xffefffff
    edi = ((edi) & (0xffefffff));
    // 0051F45A  push 0x68b0bc
    // 0051F45F  push 0x68b0cc
    // 0051F464  mov dword ptr [0x9a64c0], edi
    (*(int*)((char*)g_009a64c0)) = (edi);
    // 0051F46A  call 0x405140
    eax = POP2((const char*)(0x68b0cc), (const char*)(0x68b0bc), -1);
    // 0051F46F  add esp, 0xc
    return;
LAB_0051F479:
    // 0051F479  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
    // 0051F47E  cmp eax, 0x32
    // 0051F481  jle 0x51f4d6
    if (((int)(eax)) <= ((int)(0x32))) goto LAB_0051F4D6;
    // 0051F483  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0051F486  test ecx, ecx
    // 0051F488  jne 0x51f4d6
    if ((((ecx) & (ecx))) != 0) goto LAB_0051F4D6;
    // 0051F48A  push eax
    // 0051F48B  call 0x5c89a0
    eax = game_year(eax);
    // 0051F490  sub eax, 0x88e
    eax = ((eax) - (0x88e));
    // 0051F495  mov ecx, 0x50
    ecx = (0x50);
    // 0051F49A  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 0051F49B  idiv ecx
    { __int64 t_dividend = (((__int64)edx) << 32) | (unsigned int)eax; eax = (int)(t_dividend / (int)(ecx)); edx = (int)(t_dividend % (int)(ecx)); }
    // 0051F49D  add esp, 4
    // 0051F4A0  test edx, edx
    // 0051F4A2  jne 0x51f4d1
    if ((((edx) & (edx))) != 0) goto LAB_0051F4D1;
    // 0051F4A4  mov edi, dword ptr [0x9a64c0]
    edi = ((*(int*)((char*)g_009a64c0)));
    // 0051F4AA  push -1
    // 0051F4AC  or edi, 0x100000
    edi = ((edi) | (0x100000));
    // 0051F4B2  push 0x68b0dc
    // 0051F4B7  push 0x68b0e8
    // 0051F4BC  mov dword ptr [0x9a64c0], edi
    (*(int*)((char*)g_009a64c0)) = (edi);
    // 0051F4C2  call 0x405140
    eax = POP2((const char*)(0x68b0e8), (const char*)(0x68b0dc), -1);
    // 0051F4C7  add esp, 0xc
    return;
LAB_0051F4D1:
    // 0051F4D1  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
LAB_0051F4D6:
    // 0051F4D6  mov ecx, dword ptr [0x9a649c]
    ecx = ((*(int*)((char*)g_009a649c)));
    // 0051F4DC  test ch, 0x80
    // 0051F4DF  jne 0x522116
    if ((((((ecx >> 8) & 0xff)) & (0x80))) != 0) goto LAB_00522116;
    // 0051F4E5  mov ecx, dword ptr [0x9a64c4]
    ecx = ((*(int*)((char*)g_009a64c4)));
    // 0051F4EB  lea edx, [ecx + ecx*4]
    edx = (ecx + ecx*4);
    // 0051F4EE  mov ecx, 0x4b
    ecx = (0x4b);
    // 0051F4F3  shl edx, 1
    edx = ((edx) << (1));
    // 0051F4F5  sub ecx, edx
    ecx = ((ecx) - (edx));
    // 0051F4F7  cmp eax, ecx
    // 0051F4F9  jl 0x522116
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00522116;
    // 0051F4FF  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0051F502  test ecx, ecx
    // 0051F504  jne 0x51f51d
    if ((((ecx) & (ecx))) != 0) goto LAB_0051F51D;
    // 0051F506  mov ecx, dword ptr [0x949878]
    ecx = ((*(int*)((char*)g_00949878)));
    // 0051F50C  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 0051F50F  add ecx, eax
    ecx = ((ecx) + (eax));
    // 0051F511  lea edx, [ecx + edx*4]
    edx = (ecx + edx*4);
    // 0051F514  push edx
    // 0051F515  call 0x6257e0
    random_reseed(edx);
    // 0051F51A  add esp, 4
LAB_0051F51D:
    // 0051F51D  mov eax, dword ptr [0x9a64cc]
    eax = ((*(int*)((char*)g_009a64cc)));
    // 0051F522  cmp eax, 0x64
    // 0051F525  jg 0x51f52c
    if (((int)(eax)) > ((int)(0x64))) goto LAB_0051F52C;
    // 0051F527  mov eax, 0x64
    eax = (0x64);
LAB_0051F52C:
    // 0051F52C  push eax
    // 0051F52D  push 0
    // 0051F52F  call 0x625810
    eax = random(0, eax);
    // 0051F534  mov ecx, dword ptr [0x9a64cc]
    ecx = ((*(int*)((char*)g_009a64cc)));
    // 0051F53A  add esp, 8
    // 0051F53D  cmp eax, ecx
    // 0051F53F  mov dword ptr [ebp - 8], eax
    var_8 = (eax);
    // 0051F542  jge 0x522116
    if (((int)(eax)) >= ((int)(ecx))) goto LAB_00522116;
    // 0051F548  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0051F54B  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051F54E  lea edi, [eax + edx*4]
    edi = (eax + edx*4);
    // 0051F551  shl edi, 2
    edi = ((edi) << (2));
    // 0051F554  mov dword ptr [ebp - 0xc], edi
    var_c = (edi);
    // 0051F557  cmp byte ptr [edi + 0x97d046], 3
    // 0051F55E  jle 0x522116
    if (((int)((*(unsigned char*)((char*)g_0097d046 + edi)))) <= ((int)(3))) goto LAB_00522116;
    // 0051F564  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051F566  mov al, byte ptr [edi + 0x97d044]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d044 + edi))));
    // 0051F56C  mov ecx, eax
    ecx = (eax);
    // 0051F56E  shl ecx, 6
    ecx = ((ecx) << (6));
    // 0051F571  add ecx, eax
    ecx = ((ecx) + (eax));
    // 0051F573  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051F576  lea ecx, [eax + edx*8]
    ecx = (eax + edx*8);
    // 0051F579  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051F57C  cmp dword ptr [edx*4 + 0x96da3c], 1
    // 0051F584  jle 0x522116
    if (((int)((*(int*)((char*)g_0096da3c + edx*4)))) <= ((int)(1))) goto LAB_00522116;
    // 0051F58A  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F590  test ah, 0x7e
    // 0051F593  jne 0x522116
    if ((((((eax >> 8) & 0xff)) & (0x7e))) != 0) goto LAB_00522116;
    // 0051F599  push 0x16
    // 0051F59B  push 0
    // 0051F59D  call 0x625810
    eax = random(0, 0x16);
    // 0051F5A2  movsx ecx, word ptr [edi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + edi)));
    // 0051F5A9  mov esi, eax
    esi = (eax);
    // 0051F5AB  xor ebx, ebx
    ebx = ((ebx) ^ (ebx));
    // 0051F5AD  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 0051F5B4  mov bl, byte ptr [edi + 0x97d044]
    SETB0(ebx, ((*(unsigned char*)((char*)g_0097d044 + edi))));
    // 0051F5BA  push 0xa
    // 0051F5BC  push 0
    // 0051F5BE  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 0051F5C1  mov dword ptr [ebp - 0x14], eax
    var_14 = (eax);
    // 0051F5C4  mov dword ptr [ebp - 0x10], ecx
    var_10 = (ecx);
    // 0051F5C7  call 0x625e30
    eax = parse_num(0, 0xa);
    // 0051F5CC  lea eax, [edi + 0x97d053]
    eax = (edi + 0x97d053);
    // 0051F5D2  push -1
    // 0051F5D4  push -1
    // 0051F5D6  push eax
    // 0051F5D7  push 0
    // 0051F5D9  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 0051F5DC  call 0x625ec0
    eax = parse_says(0, (char*)(eax), -1, -1);
    // 0051F5E1  add esp, 0x20
    // 0051F5E4  cmp esi, 0x15
    // 0051F5E7  mov byte ptr [0x9b86a0], 0
    (*(unsigned char*)((char*)g_009b86a0)) = (0);
    // 0051F5EE  ja 0x522116
    if (((unsigned int)esi) > ((unsigned int)0x15)) goto LAB_00522116;
    // 0051F5F4  jmp dword ptr [esi*4 + 0x522120]
    switch (eax) {
    case 0: goto LAB_0051F5FB;
    case 1: goto LAB_0051F6C0;
    case 2: goto LAB_0051F7A2;
    case 3: goto LAB_0051F867;
    case 4: goto LAB_0051F917;
    case 5: goto LAB_0051F9DC;
    case 6: goto LAB_0051FA8C;
    case 7: goto LAB_0051FFB1;
    case 8: goto LAB_0051FFB1;
    case 9: goto LAB_00520529;
    case 10: goto LAB_005205B3;
    case 11: goto LAB_0052060D;
    case 12: goto LAB_0052067D;
    case 13: goto LAB_0052079F;
    case 14: goto LAB_00520A37;
    case 15: goto LAB_00520ED4;
    case 16: goto LAB_005211FF;
    case 17: goto LAB_00521515;
    case 18: goto LAB_00521846;
    case 19: goto LAB_005219A5;
    case 20: goto LAB_00521BAB;
    case 21: goto LAB_00521E6F;
    default: goto LAB_0051FFB1;
    }
LAB_0051F5FB:
    // 0051F5FB  cmp dword ptr [0x9a6508], ebx
    // 0051F601  jne 0x51f643
    if (((*(int*)((char*)g_009a6508))) != (ebx)) goto LAB_0051F643;
    // 0051F603  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0051F608  mov edx, 1
    edx = (1);
    // 0051F60D  mov ecx, ebx
    ecx = (ebx);
    // 0051F60F  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 0051F611  test dl, al
    // 0051F613  je 0x51f62f
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_0051F62F;
    // 0051F615  mov eax, ebx
    eax = (ebx);
    // 0051F617  shl eax, 6
    eax = ((eax) << (6));
    // 0051F61A  add eax, ebx
    eax = ((eax) + (ebx));
    // 0051F61C  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 0051F61F  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0051F622  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F625  cmp dword ptr [eax*4 + 0x96c9e8], 1
    // 0051F62D  jle 0x51f643
    if (((int)((*(int*)((char*)g_0096c9e8 + eax*4)))) <= ((int)(1))) goto LAB_0051F643;
LAB_0051F62F:
    // 0051F62F  push 4
    // 0051F631  push 0
    // 0051F633  call 0x625810
    eax = random(0, 4);
    // 0051F638  add esp, 8
    // 0051F63B  test eax, eax
    // 0051F63D  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
LAB_0051F643:
    // 0051F643  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F649  push 0x68b0f4
    // 0051F64E  or esi, 0x200
    esi = ((esi) | (0x200));
    // 0051F654  push 0x9b86a0
    // 0051F659  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051F65F  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b0f4));
    // 0051F664  push 0x9b86a0
    // 0051F669  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051F66E  add esp, 0xc
    // 0051F671  test eax, eax
    // 0051F673  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051F679  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F67F  test ah, 0x7e
    // 0051F682  je 0x51f68e
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F68E;
    // 0051F684  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051F68E:
    // 0051F68E  cmp ebx, dword ptr [0x939284]
    // 0051F694  je 0x51f6a3
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051F6A3;
    // 0051F696  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051F69B  test eax, eax
    // 0051F69D  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051F6A3:
    // 0051F6A3  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051F6A6  push ecx
    // 0051F6A7  push 0x68b0fc
    // 0051F6AC  push 0x9b86a0
    // 0051F6B1  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b0fc), ecx);
    // 0051F6B6  add esp, 0xc
    return;
LAB_0051F6C0:
    // 0051F6C0  mov edx, ebx
    edx = (ebx);
    // 0051F6C2  shl edx, 6
    edx = ((edx) << (6));
    // 0051F6C5  add edx, ebx
    edx = ((edx) + (ebx));
    // 0051F6C7  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F6CA  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 0051F6CD  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051F6CF  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0051F6D2  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 0051F6D8  cmp ecx, 0x96
    // 0051F6DE  mov ecx, dword ptr [edx*4 + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + edx*4)));
    // 0051F6E5  setge al
    SETB0(eax, ((((int)(ecx)) >= ((int)(0x96))) ? 1 : 0));
    // 0051F6E8  add eax, 4
    eax = ((eax) + (4));
    // 0051F6EB  cmp ecx, eax
    // 0051F6ED  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
    // 0051F6F3  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F6F9  push 0x68b108
    // 0051F6FE  or esi, 0x400
    esi = ((esi) | (0x400));
    // 0051F704  push 0x9b86a0
    // 0051F709  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051F70F  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b108));
    // 0051F714  push 0x9b86a0
    // 0051F719  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051F71E  add esp, 0xc
    // 0051F721  test eax, eax
    // 0051F723  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051F729  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F72F  test ah, 0x7e
    // 0051F732  je 0x51f73e
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F73E;
    // 0051F734  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051F73E:
    // 0051F73E  cmp ebx, dword ptr [0x939284]
    // 0051F744  je 0x51f753
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051F753;
    // 0051F746  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051F74B  test eax, eax
    // 0051F74D  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051F753:
    // 0051F753  lea eax, [ebx + ebx*4]
    eax = (ebx + ebx*4);
    // 0051F756  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0051F759  shl ecx, 3
    ecx = ((ecx) << (3));
    // 0051F75C  sub ecx, ebx
    ecx = ((ecx) - (ebx));
    // 0051F75E  test byte ptr [ecx*4 + 0x946f58], 0x80
    // 0051F766  je 0x51f785
    if (((((*(unsigned char*)((char*)g_00946f58 + ecx*4))) & (0x80))) == 0) goto LAB_0051F785;
    // 0051F768  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 0051F76B  push edx
    // 0051F76C  push 0x68b110
    // 0051F771  push 0x9b86a0
    // 0051F776  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b110), edx);
    // 0051F77B  add esp, 0xc
    return;
LAB_0051F785:
    // 0051F785  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 0051F788  push eax
    // 0051F789  push 0x68b120
    // 0051F78E  push 0x9b86a0
    // 0051F793  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b120), eax);
    // 0051F798  add esp, 0xc
    return;
LAB_0051F7A2:
    // 0051F7A2  cmp dword ptr [0x9a6508], ebx
    // 0051F7A8  jne 0x51f7ea
    if (((*(int*)((char*)g_009a6508))) != (ebx)) goto LAB_0051F7EA;
    // 0051F7AA  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0051F7AF  mov edx, 1
    edx = (1);
    // 0051F7B4  mov ecx, ebx
    ecx = (ebx);
    // 0051F7B6  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 0051F7B8  test dl, al
    // 0051F7BA  je 0x51f7d6
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_0051F7D6;
    // 0051F7BC  mov eax, ebx
    eax = (ebx);
    // 0051F7BE  shl eax, 6
    eax = ((eax) << (6));
    // 0051F7C1  add eax, ebx
    eax = ((eax) + (ebx));
    // 0051F7C3  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 0051F7C6  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0051F7C9  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F7CC  cmp dword ptr [eax*4 + 0x96c9e8], 1
    // 0051F7D4  jle 0x51f7ea
    if (((int)((*(int*)((char*)g_0096c9e8 + eax*4)))) <= ((int)(1))) goto LAB_0051F7EA;
LAB_0051F7D6:
    // 0051F7D6  push 4
    // 0051F7D8  push 0
    // 0051F7DA  call 0x625810
    eax = random(0, 4);
    // 0051F7DF  add esp, 8
    // 0051F7E2  test eax, eax
    // 0051F7E4  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
LAB_0051F7EA:
    // 0051F7EA  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F7F0  push 0x68b130
    // 0051F7F5  or esi, 0x800
    esi = ((esi) | (0x800));
    // 0051F7FB  push 0x9b86a0
    // 0051F800  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051F806  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b130));
    // 0051F80B  push 0x9b86a0
    // 0051F810  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051F815  add esp, 0xc
    // 0051F818  test eax, eax
    // 0051F81A  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051F820  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F826  test ah, 0x7e
    // 0051F829  je 0x51f835
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F835;
    // 0051F82B  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051F835:
    // 0051F835  cmp ebx, dword ptr [0x939284]
    // 0051F83B  je 0x51f84a
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051F84A;
    // 0051F83D  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051F842  test eax, eax
    // 0051F844  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051F84A:
    // 0051F84A  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051F84D  push ecx
    // 0051F84E  push 0x68b13c
    // 0051F853  push 0x9b86a0
    // 0051F858  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b13c), ecx);
    // 0051F85D  add esp, 0xc
    return;
LAB_0051F867:
    // 0051F867  mov edx, ebx
    edx = (ebx);
    // 0051F869  shl edx, 6
    edx = ((edx) << (6));
    // 0051F86C  add edx, ebx
    edx = ((edx) + (ebx));
    // 0051F86E  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F871  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 0051F874  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051F876  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0051F879  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 0051F87F  cmp ecx, 0x96
    // 0051F885  mov ecx, dword ptr [edx*4 + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + edx*4)));
    // 0051F88C  setge al
    SETB0(eax, ((((int)(ecx)) >= ((int)(0x96))) ? 1 : 0));
    // 0051F88F  add eax, 4
    eax = ((eax) + (4));
    // 0051F892  cmp ecx, eax
    // 0051F894  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
    // 0051F89A  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F8A0  push 0x68b14c
    // 0051F8A5  or esi, 0x1000
    esi = ((esi) | (0x1000));
    // 0051F8AB  push 0x9b86a0
    // 0051F8B0  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051F8B6  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b14c));
    // 0051F8BB  push 0x9b86a0
    // 0051F8C0  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051F8C5  add esp, 0xc
    // 0051F8C8  test eax, eax
    // 0051F8CA  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051F8D0  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F8D6  test ah, 0x7e
    // 0051F8D9  je 0x51f8e5
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F8E5;
    // 0051F8DB  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051F8E5:
    // 0051F8E5  cmp ebx, dword ptr [0x939284]
    // 0051F8EB  je 0x51f8fa
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051F8FA;
    // 0051F8ED  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051F8F2  test eax, eax
    // 0051F8F4  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051F8FA:
    // 0051F8FA  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051F8FD  push ecx
    // 0051F8FE  push 0x68b154
    // 0051F903  push 0x9b86a0
    // 0051F908  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b154), ecx);
    // 0051F90D  add esp, 0xc
    return;
LAB_0051F917:
    // 0051F917  cmp dword ptr [0x9a6508], ebx
    // 0051F91D  jne 0x51f95f
    if (((*(int*)((char*)g_009a6508))) != (ebx)) goto LAB_0051F95F;
    // 0051F91F  mov al, byte ptr [0x9a64e8]
    SETB0(eax, ((*(unsigned char*)((char*)g_009a64e8))));
    // 0051F924  mov edx, 1
    edx = (1);
    // 0051F929  mov ecx, ebx
    ecx = (ebx);
    // 0051F92B  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 0051F92D  test dl, al
    // 0051F92F  je 0x51f94b
    if (((((edx & 0xff)) & ((eax & 0xff)))) == 0) goto LAB_0051F94B;
    // 0051F931  mov eax, ebx
    eax = (ebx);
    // 0051F933  shl eax, 6
    eax = ((eax) << (6));
    // 0051F936  add eax, ebx
    eax = ((eax) + (ebx));
    // 0051F938  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 0051F93B  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0051F93E  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F941  cmp dword ptr [eax*4 + 0x96c9e8], 1
    // 0051F949  jle 0x51f95f
    if (((int)((*(int*)((char*)g_0096c9e8 + eax*4)))) <= ((int)(1))) goto LAB_0051F95F;
LAB_0051F94B:
    // 0051F94B  push 4
    // 0051F94D  push 0
    // 0051F94F  call 0x625810
    eax = random(0, 4);
    // 0051F954  add esp, 8
    // 0051F957  test eax, eax
    // 0051F959  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
LAB_0051F95F:
    // 0051F95F  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F965  push 0x68b168
    // 0051F96A  or esi, 0x2000
    esi = ((esi) | (0x2000));
    // 0051F970  push 0x9b86a0
    // 0051F975  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051F97B  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b168));
    // 0051F980  push 0x9b86a0
    // 0051F985  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051F98A  add esp, 0xc
    // 0051F98D  test eax, eax
    // 0051F98F  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051F995  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051F99B  test ah, 0x7e
    // 0051F99E  je 0x51f9aa
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051F9AA;
    // 0051F9A0  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051F9AA:
    // 0051F9AA  cmp ebx, dword ptr [0x939284]
    // 0051F9B0  je 0x51f9bf
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051F9BF;
    // 0051F9B2  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051F9B7  test eax, eax
    // 0051F9B9  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051F9BF:
    // 0051F9BF  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051F9C2  push ecx
    // 0051F9C3  push 0x68b174
    // 0051F9C8  push 0x9b86a0
    // 0051F9CD  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b174), ecx);
    // 0051F9D2  add esp, 0xc
    return;
LAB_0051F9DC:
    // 0051F9DC  mov edx, ebx
    edx = (ebx);
    // 0051F9DE  shl edx, 6
    edx = ((edx) << (6));
    // 0051F9E1  add edx, ebx
    edx = ((edx) + (ebx));
    // 0051F9E3  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051F9E6  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 0051F9E9  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051F9EB  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0051F9EE  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 0051F9F4  cmp ecx, 0x96
    // 0051F9FA  mov ecx, dword ptr [edx*4 + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + edx*4)));
    // 0051FA01  setge al
    SETB0(eax, ((((int)(ecx)) >= ((int)(0x96))) ? 1 : 0));
    // 0051FA04  add eax, 4
    eax = ((eax) + (4));
    // 0051FA07  cmp ecx, eax
    // 0051FA09  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
    // 0051FA0F  mov esi, dword ptr [edi + 0x97d074]
    esi = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051FA15  push 0x68b180
    // 0051FA1A  or esi, 0x4000
    esi = ((esi) | (0x4000));
    // 0051FA20  push 0x9b86a0
    // 0051FA25  mov dword ptr [edi + 0x97d074], esi
    (*(int*)((char*)g_0097d074 + edi)) = (esi);
    // 0051FA2B  call 0x645470
    eax = (int)strcat((char*)(0x9b86a0), (const char*)(0x68b180));
    // 0051FA30  push 0x9b86a0
    // 0051FA35  call 0x6453e0
    eax = strlen((const char*)(0x9b86a0));
    // 0051FA3A  add esp, 0xc
    // 0051FA3D  test eax, eax
    // 0051FA3F  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051FA45  mov eax, dword ptr [edi + 0x97d074]
    eax = ((*(int*)((char*)g_0097d074 + edi)));
    // 0051FA4B  test ah, 0x7e
    // 0051FA4E  je 0x51fa5a
    if ((((((eax >> 8) & 0xff)) & (0x7e))) == 0) goto LAB_0051FA5A;
    // 0051FA50  mov dword ptr [edi + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + edi)) = (0xa);
LAB_0051FA5A:
    // 0051FA5A  cmp ebx, dword ptr [0x939284]
    // 0051FA60  je 0x51fa6f
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0051FA6F;
    // 0051FA62  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051FA67  test eax, eax
    // 0051FA69  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0051FA6F:
    // 0051FA6F  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051FA72  push ecx
    // 0051FA73  push 0x68b18c
    // 0051FA78  push 0x9b86a0
    // 0051FA7D  call 0x405140
    eax = POP2((const char*)(0x9b86a0), (const char*)(0x68b18c), ecx);
    // 0051FA82  add esp, 0xc
    return;
LAB_0051FA8C:
    // 0051FA8C  lea edx, [ebp - 0x20]
    edx = ((int)&var_20);
    // 0051FA8F  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 0051FA92  push edx
    // 0051FA93  push eax
    // 0051FA94  push 0x11
    // 0051FA96  call 0x50ba00
    bitmask(0x11, (int*)(eax), (int*)(edx));
    // 0051FA9B  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0051FA9E  mov al, byte ptr [ebp - 0x20]
    SETB0(eax, ((var_20 & 0xff)));
    // 0051FAA1  add esp, 0xc
    // 0051FAA4  mov dl, byte ptr [edi + ecx + 0x97d0cc]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d0cc + ecx))));
    // 0051FAAB  and dl, al
    SETB0(edx, (((edx & 0xff)) & ((eax & 0xff))));
    // 0051FAAD..0051FAB1  neg8/sbb/neg -> (dl!=0)?1:0
    edx = ((edx & 0xff)) != 0 ? 1 : 0;
    // 0051FAB3  mov dword ptr [ebp + 8], edx
    a1 = (edx);
    // 0051FAB6  jne 0x51fefd
    if ((edx) != (0x0)) goto LAB_0051FEFD;
    // 0051FABC  lea eax, [ebp - 0x20]
    eax = ((int)&var_20);
    // 0051FABF  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 0051FAC2  push eax
    // 0051FAC3  push ecx
    // 0051FAC4  push 0x12
    // 0051FAC6  call 0x50ba00
    bitmask(0x12, (int*)(ecx), (int*)(eax));
    // 0051FACB  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 0051FACE  mov cl, byte ptr [ebp - 0x20]
    SETB0(ecx, ((var_20 & 0xff)));
    // 0051FAD1  add esp, 0xc
    // 0051FAD4  mov al, byte ptr [edi + edx + 0x97d0cc]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d0cc + edx))));
    // 0051FADB  and al, cl
    SETB0(eax, (((eax & 0xff)) & ((ecx & 0xff))));
    // 0051FADD..0051FAE1  neg8/sbb/neg -> (al!=0)?1:0
    eax = ((eax & 0xff)) != 0 ? 1 : 0;
    // 0051FAE3  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0051FAE6  jne 0x51fefd
    if ((eax) != (0x0)) goto LAB_0051FEFD;
    // 0051FAEC  mov esi, dword ptr [0x9a6514]
    esi = ((*(int*)((char*)g_009a6514)));
    // 0051FAF2  mov eax, dword ptr [0x9a654c]
    eax = ((*(int*)((char*)g_009a654c)));
    // 0051FAF7  test esi, esi
    // 0051FAF9  jl 0x51fb1c
    if (((int)(((esi) & (esi)))) < 0) goto LAB_0051FB1C;
    // 0051FAFB  lea ecx, [esi + esi*8]
    ecx = (esi + esi*8);
    // 0051FAFE  lea edx, [esi + ecx*2]
    edx = (esi + ecx*2);
    // 0051FB01  lea ecx, [esi + edx*4]
    ecx = (esi + edx*4);
    // 0051FB04  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0051FB06  mov dl, byte ptr [ecx*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + ecx*4))));
    // 0051FB0D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FB0F  cmp edx, ebx
    // 0051FB11  sete cl
    SETB0(ecx, (((edx) == (ebx)) ? 1 : 0));
    // 0051FB14  test ecx, ecx
    // 0051FB16  jne 0x51fe24
    if ((((ecx) & (ecx))) != 0) goto LAB_0051FE24;
LAB_0051FB1C:
    // 0051FB1C  test eax, eax
    // 0051FB1E  jl 0x51fb41
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0051FB41;
    // 0051FB20  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0051FB23  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051FB26  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 0051FB29  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0051FB2B  mov dl, byte ptr [ecx*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + ecx*4))));
    // 0051FB32  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FB34  cmp edx, ebx
    // 0051FB36  sete cl
    SETB0(ecx, (((edx) == (ebx)) ? 1 : 0));
    // 0051FB39  test ecx, ecx
    // 0051FB3B  jne 0x51fe24
    if ((((ecx) & (ecx))) != 0) goto LAB_0051FE24;
LAB_0051FB41:
    // 0051FB41  mov ecx, dword ptr [0x9a657c]
    ecx = ((*(int*)((char*)g_009a657c)));
    // 0051FB47  test ecx, ecx
    // 0051FB49  jl 0x51fb6c
    if (((int)(((ecx) & (ecx)))) < 0) goto LAB_0051FB6C;
    // 0051FB4B  lea edx, [ecx + ecx*8]
    edx = (ecx + ecx*8);
    // 0051FB4E  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 0051FB51  lea ecx, [ecx + edx*4]
    ecx = (ecx + edx*4);
    // 0051FB54  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0051FB56  mov dl, byte ptr [ecx*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + ecx*4))));
    // 0051FB5D  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FB5F  cmp edx, ebx
    // 0051FB61  sete cl
    SETB0(ecx, (((edx) == (ebx)) ? 1 : 0));
    // 0051FB64  test ecx, ecx
    // 0051FB66  jne 0x51fe24
    if ((((ecx) & (ecx))) != 0) goto LAB_0051FE24;
LAB_0051FB6C:
    // 0051FB6C  mov eax, ebx
    eax = (ebx);
    // 0051FB6E  shl eax, 6
    eax = ((eax) << (6));
    // 0051FB71  add eax, ebx
    eax = ((eax) + (ebx));
    // 0051FB73  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 0051FB76  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0051FB79  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FB7B  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 0051FB7E  mov ebx, dword ptr [0x9a64d4]
    ebx = ((*(int*)((char*)g_009a64d4)));
    // 0051FB84  cmp ebx, 0x96
    // 0051FB8A  mov edx, dword ptr [eax*4 + 0x96c9e4]
    edx = ((*(int*)((char*)g_0096c9e4 + eax*4)));
    // 0051FB91  setge cl
    SETB0(ecx, ((((int)(ebx)) >= ((int)(0x96))) ? 1 : 0));
    // 0051FB94  add ecx, 4
    ecx = ((ecx) + (4));
    // 0051FB97  cmp edx, ecx
    // 0051FB99  jl 0x522116
    if (((int)(edx)) < ((int)(ecx))) goto LAB_00522116;
    // 0051FB9F  movsx eax, byte ptr [edi + 0x97d046]
    eax = ((*(signed char*)((char*)g_0097d046 + edi)));
    // 0051FBA6  inc eax
    eax = ((eax) + 1);
    // 0051FBA7  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 0051FBA8  sub eax, edx
    eax = ((eax) - (edx));
    // 0051FBAA  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0051FBAC  push eax
    // 0051FBAD  push 1
    // 0051FBAF  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 0051FBB2  call 0x625e30
    eax = parse_num(1, eax);
    // 0051FBB7  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 0051FBBD  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0051FBBF  mov dl, byte ptr [edi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + edi))));
    // 0051FBC5  add esp, 8
    // 0051FBC8  cmp edx, ecx
    // 0051FBCA  je 0x51fbe4
    if ((edx) == (ecx)) goto LAB_0051FBE4;
    // 0051FBCC  mov dl, byte ptr [edi + 0x97d04a]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d04a + edi))));
    // 0051FBD2  mov eax, 1
    eax = (1);
    // 0051FBD7  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 0051FBD9  test al, dl
    // 0051FBDB  jne 0x51fbe4
    if (((((eax & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_0051FBE4;
    // 0051FBDD  xor esi, esi
    esi = ((esi) ^ (esi));
    // 0051FBDF  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 0051FBE2  jmp 0x51fc07
    goto LAB_0051FC07;
LAB_0051FBE4:
    // 0051FBE4  movsx edx, word ptr [edi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + edi)));
    // 0051FBEB  push ecx
    // 0051FBEC  mov esi, 1
    esi = (1);
    // 0051FBF1  movsx ecx, word ptr [edi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + edi)));
    // 0051FBF8  push ecx
    // 0051FBF9  push edx
    // 0051FBFA  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 0051FBFF  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 0051FC02  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, ecx);
LAB_0051FC07:
    // 0051FC07  mov eax, dword ptr [0x9a64cc]
    eax = ((*(int*)((char*)g_009a64cc)));
    // 0051FC0C  test eax, eax
    // 0051FC0E  jle 0x51fe01
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_0051FE01;
    // 0051FC14  mov dword ptr [ebp + 8], 1
    a1 = (1);
    // 0051FC1B  xor esi, esi
    esi = ((esi) ^ (esi));
LAB_0051FC1D:
    // 0051FC1D  movsx eax, word ptr [edi + 0x97d042]
    eax = ((*(short*)((char*)g_0097d042 + edi)));
    // 0051FC24  movsx ecx, word ptr [edi + 0x97d040]
    ecx = ((*(short*)((char*)g_0097d040 + edi)));
    // 0051FC2B  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 0051FC2E  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 0051FC35  movsx ebx, word ptr [esi + 0x97d042]
    ebx = ((*(short*)((char*)g_0097d042 + esi)));
    // 0051FC3C  sub eax, ecx
    eax = ((eax) - (ecx));
    // 0051FC3E  push eax
    // 0051FC3F  call 0x644f3a
    eax = abs(eax);
    // 0051FC44  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 0051FC4A  add esp, 4
    // 0051FC4D  test cl, 1
    // 0051FC50  jne 0x51fc67
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_0051FC67;
    // 0051FC52  cmp eax, dword ptr [0x68faf0]
    // 0051FC58  jle 0x51fc67
    if (((int)(eax)) <= ((int)((*(int*)((char*)g_0068faf0))))) goto LAB_0051FC67;
    // 0051FC5A  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 0051FC60  sub ecx, eax
    ecx = ((ecx) - (eax));
    // 0051FC62  mov dword ptr [ebp - 0x1c], ecx
    var_1c = (ecx);
    // 0051FC65  jmp 0x51fc6a
    goto LAB_0051FC6A;
LAB_0051FC67:
    // 0051FC67  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_0051FC6A:
    // 0051FC6A  sub ebx, dword ptr [ebp - 0x20]
    ebx = ((ebx) - (var_20));
    // 0051FC6D  push ebx
    // 0051FC6E  call 0x644f3a
    eax = abs(ebx);
    // 0051FC73  mov ebx, dword ptr [ebp - 0x1c]
    ebx = (var_1c);
    // 0051FC76  mov ecx, dword ptr [ebp - 0x18]
    ecx = (var_18);
    // 0051FC79  add eax, ebx
    eax = ((eax) + (ebx));
    // 0051FC7B  add esp, 4
    // 0051FC7E  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0051FC80  cmp eax, ecx
    // 0051FC82  jg 0x51fde2
    if (((int)(eax)) > ((int)(ecx))) goto LAB_0051FDE2;
    // 0051FC88  lea ecx, [ebp - 0x10]
    ecx = ((int)&var_10);
    // 0051FC8B  lea edx, [ebp - 0x14]
    edx = ((int)&var_14);
    // 0051FC8E  push ecx
    // 0051FC8F  push edx
    // 0051FC90  push 0x11
    // 0051FC92  call 0x50ba00
    bitmask(0x11, (int*)(edx), (int*)(ecx));
    // 0051FC97  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 0051FC9A  mov bl, byte ptr [ebp - 0x10]
    SETB0(ebx, ((var_10 & 0xff)));
    // 0051FC9D  add esp, 0xc
    // 0051FCA0  mov cl, byte ptr [esi + eax + 0x97d0cc]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d0cc + eax))));
    // 0051FCA7  and cl, bl
    SETB0(ecx, (((ecx & 0xff)) & ((ebx & 0xff))));
    // 0051FCA9..0051FCAD  neg8/sbb/neg -> (cl!=0)?1:0
    ecx = ((ecx & 0xff)) != 0 ? 1 : 0;
    // 0051FCAF  mov dword ptr [ebp - 0x20], ecx
    var_20 = (ecx);
    // 0051FCB2  jne 0x51fde2
    if ((ecx) != (0x0)) goto LAB_0051FDE2;
    // 0051FCB8  lea edx, [ebp - 8]
    edx = ((int)&var_8);
    // 0051FCBB  lea eax, [ebp - 0xc]
    eax = ((int)&var_c);
    // 0051FCBE  push edx
    // 0051FCBF  push eax
    // 0051FCC0  push 0x12
    // 0051FCC2  call 0x50ba00
    bitmask(0x12, (int*)(eax), (int*)(edx));
    // 0051FCC7  mov ecx, dword ptr [ebp - 0xc]
    ecx = (var_c);
    // 0051FCCA  mov bl, byte ptr [ebp - 8]
    SETB0(ebx, ((var_8 & 0xff)));
    // 0051FCCD  add esp, 0xc
    // 0051FCD0  mov dl, byte ptr [esi + ecx + 0x97d0cc]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d0cc + ecx))));
    // 0051FCD7  and dl, bl
    SETB0(edx, (((edx & 0xff)) & ((ebx & 0xff))));
    // 0051FCD9..0051FCDD  neg8/sbb/neg -> (dl!=0)?1:0
    edx = ((edx & 0xff)) != 0 ? 1 : 0;
    // 0051FCDF  mov dword ptr [ebp - 0x20], edx
    var_20 = (edx);
    // 0051FCE2  jne 0x51fde2
    if ((edx) != (0x0)) goto LAB_0051FDE2;
    // 0051FCE8  mov eax, dword ptr [0x9a6514]
    eax = ((*(int*)((char*)g_009a6514)));
    // 0051FCED  test eax, eax
    // 0051FCEF  jl 0x51fd16
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0051FD16;
    // 0051FCF1  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0051FCF4  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051FCF7  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 0051FCFA  mov dl, byte ptr [esi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 0051FD00  mov cl, byte ptr [eax*4 + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 0051FD07  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051FD09  cmp cl, dl
    // 0051FD0B  sete al
    SETB0(eax, ((((ecx & 0xff)) == ((edx & 0xff))) ? 1 : 0));
    // 0051FD0E  test eax, eax
    // 0051FD10  jne 0x51fde2
    if ((((eax) & (eax))) != 0) goto LAB_0051FDE2;
LAB_0051FD16:
    // 0051FD16  mov eax, dword ptr [0x9a654c]
    eax = ((*(int*)((char*)g_009a654c)));
    // 0051FD1B  test eax, eax
    // 0051FD1D  jl 0x51fd46
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0051FD46;
    // 0051FD1F  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 0051FD22  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 0051FD25  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 0051FD28  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FD2A  mov al, byte ptr [edx*4 + 0x97d044]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d044 + edx*4))));
    // 0051FD31  mov dl, byte ptr [esi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 0051FD37  cmp al, dl
    // 0051FD39  sete cl
    SETB0(ecx, ((((eax & 0xff)) == ((edx & 0xff))) ? 1 : 0));
    // 0051FD3C  mov eax, ecx
    eax = (ecx);
    // 0051FD3E  test eax, eax
    // 0051FD40  jne 0x51fde2
    if ((((eax) & (eax))) != 0) goto LAB_0051FDE2;
LAB_0051FD46:
    // 0051FD46  mov eax, dword ptr [0x9a657c]
    eax = ((*(int*)((char*)g_009a657c)));
    // 0051FD4B  test eax, eax
    // 0051FD4D  jl 0x51fd72
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0051FD72;
    // 0051FD4F  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 0051FD52  lea ecx, [eax + edx*2]
    ecx = (eax + edx*2);
    // 0051FD55  lea edx, [eax + ecx*4]
    edx = (eax + ecx*4);
    // 0051FD58  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FD5A  mov al, byte ptr [edx*4 + 0x97d044]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d044 + edx*4))));
    // 0051FD61  mov dl, byte ptr [esi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 0051FD67  cmp al, dl
    // 0051FD69  sete cl
    SETB0(ecx, ((((eax & 0xff)) == ((edx & 0xff))) ? 1 : 0));
    // 0051FD6C  mov eax, ecx
    eax = (ecx);
    // 0051FD6E  test eax, eax
    // 0051FD70  jne 0x51fde2
    if ((((eax) & (eax))) != 0) goto LAB_0051FDE2;
LAB_0051FD72:
    // 0051FD72  movsx eax, byte ptr [esi + 0x97d046]
    eax = ((*(signed char*)((char*)g_0097d046 + esi)));
    // 0051FD79  inc eax
    eax = ((eax) + 1);
    // 0051FD7A  push 2
    // 0051FD7C  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 0051FD7D  sub eax, edx
    eax = ((eax) - (edx));
    // 0051FD7F  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 0051FD81  mov byte ptr [esi + 0x97d046], al
    (*(unsigned char*)((char*)g_0097d046 + esi)) = ((eax & 0xff));
    // 0051FD87  movsx edx, word ptr [esi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + esi)));
    // 0051FD8E  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 0051FD95  push edx
    // 0051FD96  push eax
    // 0051FD97  call 0x46af40
    draw_tile(eax, edx, 2);
    // 0051FD9C  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 0051FDA1  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FDA3  mov cl, byte ptr [esi + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 0051FDA9  add esp, 0xc
    // 0051FDAC  cmp ecx, eax
    // 0051FDAE  jne 0x51fdd3
    if ((ecx) != (eax)) goto LAB_0051FDD3;
    // 0051FDB0  movsx edx, word ptr [esi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + esi)));
    // 0051FDB7  push eax
    // 0051FDB8  push edx
    // 0051FDB9  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 0051FDC0  push eax
    // 0051FDC1  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 0051FDC6  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, eax);
    // 0051FDCB  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0051FDCE  mov dword ptr [ebp - 4], ecx
    var_4 = (ecx);
    // 0051FDD1  jmp 0x51fde2
    goto LAB_0051FDE2;
LAB_0051FDD3:
    // 0051FDD3  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051FDD8  test eax, eax
    // 0051FDDA  je 0x51fde2
    if ((((eax) & (eax))) == 0) goto LAB_0051FDE2;
    // 0051FDDC  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 0051FDDF  mov dword ptr [ebp - 4], edx
    var_4 = (edx);
LAB_0051FDE2:
    // 0051FDE2  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0051FDE5  mov ecx, dword ptr [0x9a64cc]
    ecx = ((*(int*)((char*)g_009a64cc)));
    // 0051FDEB  add esi, 0x134
    esi = ((esi) + (0x134));
    // 0051FDF1  inc eax
    eax = ((eax) + 1);
    // 0051FDF2  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0051FDF5  dec eax
    eax = ((eax) - 1);
    // 0051FDF6  cmp eax, ecx
    // 0051FDF8  jl 0x51fc1d
    if (((int)(eax)) < ((int)(ecx))) goto LAB_0051FC1D;
    // 0051FDFE  mov esi, dword ptr [ebp - 4]
    esi = (var_4);
LAB_0051FE01:
    // 0051FE01  test esi, esi
    // 0051FE03  je 0x522116
    if ((((esi) & (esi))) == 0) goto LAB_00522116;
    // 0051FE09  dec esi
    esi = ((esi) - 1);
    // 0051FE0A  push esi
    // 0051FE0B  push 0x68b20c
    // 0051FE10  push 0x68b21c
    // 0051FE15  call 0x405140
    eax = POP2((const char*)(0x68b21c), (const char*)(0x68b20c), esi);
    // 0051FE1A  add esp, 0xc
    return;
LAB_0051FE24:
    // 0051FE24  test esi, esi
    // 0051FE26  jl 0x51fe50
    if (((int)(((esi) & (esi)))) < 0) goto LAB_0051FE50;
    // 0051FE28  lea ecx, [esi + esi*8]
    ecx = (esi + esi*8);
    // 0051FE2B  lea edx, [esi + ecx*2]
    edx = (esi + ecx*2);
    // 0051FE2E  lea ecx, [esi + edx*4]
    ecx = (esi + edx*4);
    // 0051FE31  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0051FE33  mov dl, byte ptr [ecx*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + ecx*4))));
    // 0051FE3A  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FE3C  cmp edx, ebx
    // 0051FE3E  sete cl
    SETB0(ecx, (((edx) == (ebx)) ? 1 : 0));
    // 0051FE41  test ecx, ecx
    // 0051FE43  je 0x51fe50
    if ((((ecx) & (ecx))) == 0) goto LAB_0051FE50;
    // 0051FE45  mov eax, dword ptr [0x9a5888]
    eax = ((*(int*)((char*)g_009a5888)));
    // 0051FE4A  push -1
    // 0051FE4C  push -1
    // 0051FE4E  jmp 0x51fe87
    goto LAB_0051FE87;
LAB_0051FE50:
    // 0051FE50  test eax, eax
    // 0051FE52  jl 0x51fe7e
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0051FE7E;
    // 0051FE54  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0051FE57  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 0051FE5A  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0051FE5C  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 0051FE5F  mov cl, byte ptr [eax*4 + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 0051FE66  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0051FE68  cmp ecx, ebx
    // 0051FE6A  sete al
    SETB0(eax, (((ecx) == (ebx)) ? 1 : 0));
    // 0051FE6D  test eax, eax
    // 0051FE6F  je 0x51fe7e
    if ((((eax) & (eax))) == 0) goto LAB_0051FE7E;
    // 0051FE71  mov edx, dword ptr [0x9a5b28]
    edx = ((*(int*)((char*)g_009a5b28)));
    // 0051FE77  push -1
    // 0051FE79  push -1
    // 0051FE7B  push edx
    // 0051FE7C  jmp 0x51fe88
    goto LAB_0051FE88;
LAB_0051FE7E:
    // 0051FE7E  mov eax, dword ptr [0x9a5d68]
    eax = ((*(int*)((char*)g_009a5d68)));
    // 0051FE83  push -1
    // 0051FE85  push -1
LAB_0051FE87:
    // 0051FE87  push eax
LAB_0051FE88:
    // 0051FE88  push 1
    // 0051FE8A  call 0x625e50
    eax = parse_say(1, eax, -1, -1);
    // 0051FE8F  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 0051FE94  add esp, 0x10
    // 0051FE97  cmp ebx, eax
    // 0051FE99  jne 0x51fed3
    if ((ebx) != (eax)) goto LAB_0051FED3;
    // 0051FE9B  movsx ecx, word ptr [edi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + edi)));
    // 0051FEA2  movsx edx, word ptr [edi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + edi)));
    // 0051FEA9  push eax
    // 0051FEAA  push ecx
    // 0051FEAB  push edx
    // 0051FEAC  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 0051FEB1  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, eax);
    // 0051FEB6  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 0051FEB9  push eax
    // 0051FEBA  push 0x68b1d4
    // 0051FEBF  push 0x68b1e4
    // 0051FEC4  call 0x405140
    eax = POP2((const char*)(0x68b1e4), (const char*)(0x68b1d4), eax);
    // 0051FEC9  add esp, 0xc
    return;
LAB_0051FED3:
    // 0051FED3  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051FED8  test eax, eax
    // 0051FEDA  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051FEE0  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051FEE3  push ecx
    // 0051FEE4  push 0x68b1f0
    // 0051FEE9  push 0x68b200
    // 0051FEEE  call 0x405140
    eax = POP2((const char*)(0x68b200), (const char*)(0x68b1f0), ecx);
    // 0051FEF3  add esp, 0xc
    return;
LAB_0051FEFD:
    // 0051FEFD  lea edx, [ebp - 0x20]
    edx = ((int)&var_20);
    // 0051FF00  lea eax, [ebp + 8]
    eax = ((int)&a1);
    // 0051FF03  push edx
    // 0051FF04  push eax
    // 0051FF05  push 0x12
    // 0051FF07  call 0x50ba00
    bitmask(0x12, (int*)(eax), (int*)(edx));
    // 0051FF0C  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 0051FF0F  mov al, byte ptr [ebp - 0x20]
    SETB0(eax, ((var_20 & 0xff)));
    // 0051FF12  add esp, 0xc
    // 0051FF15  mov dl, byte ptr [edi + ecx + 0x97d0cc]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d0cc + ecx))));
    // 0051FF1C  and dl, al
    SETB0(edx, (((edx & 0xff)) & ((eax & 0xff))));
    // 0051FF1E  push -1
    // 0051FF20..0051FF26  neg8/sbb/neg -> (dl!=0)?1:0
    edx = ((edx & 0xff)) != 0 ? 1 : 0;
    // 0051FF24  push -1
    // 0051FF28  mov dword ptr [ebp + 8], edx
    a1 = (edx);
    // 0051FF2B  je 0x51ff35
    if ((edx) == (0x0)) goto LAB_0051FF35;
    // 0051FF2D  mov eax, dword ptr [0x9a4ec8]
    eax = ((*(int*)((char*)g_009a4ec8)));
    // 0051FF32  push eax
    // 0051FF33  jmp 0x51ff3c
    goto LAB_0051FF3C;
LAB_0051FF35:
    // 0051FF35  mov ecx, dword ptr [0x9a4e98]
    ecx = ((*(int*)((char*)g_009a4e98)));
    // 0051FF3B  push ecx
LAB_0051FF3C:
    // 0051FF3C  push 1
    // 0051FF3E  call 0x625e50
    eax = parse_say(1, ecx, eax, -1);
    // 0051FF43  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 0051FF48  add esp, 0x10
    // 0051FF4B  cmp ebx, eax
    // 0051FF4D  jne 0x51ff87
    if ((ebx) != (eax)) goto LAB_0051FF87;
    // 0051FF4F  movsx edx, word ptr [edi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + edi)));
    // 0051FF56  push eax
    // 0051FF57  push edx
    // 0051FF58  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 0051FF5F  push eax
    // 0051FF60  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 0051FF65  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, eax);
    // 0051FF6A  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0051FF6D  push ecx
    // 0051FF6E  push 0x68b19c
    // 0051FF73  push 0x68b1ac
    // 0051FF78  call 0x405140
    eax = POP2((const char*)(0x68b1ac), (const char*)(0x68b19c), ecx);
    // 0051FF7D  add esp, 0xc
    return;
LAB_0051FF87:
    // 0051FF87  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051FF8C  test eax, eax
    // 0051FF8E  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0051FF94  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 0051FF97  push edx
    // 0051FF98  push 0x68b1b8
    // 0051FF9D  push 0x68b1c8
    // 0051FFA2  call 0x405140
    eax = POP2((const char*)(0x68b1c8), (const char*)(0x68b1b8), edx);
    // 0051FFA7  add esp, 0xc
    return;
LAB_0051FFB1:
    // 0051FFB1  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 0051FFB6  test eax, eax
    // 0051FFB8  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 0051FFBE  xor esi, esi
    esi = ((esi) ^ (esi));
    // 0051FFC0  push 0x14
    // 0051FFC2  push esi
    // 0051FFC3  call 0x625810
    eax = random(esi, 0x14);
    // 0051FFC8  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 0051FFCE  add esp, 8
    // 0051FFD1  inc eax
    eax = ((eax) + 1);
    // 0051FFD2  mov dword ptr [ebp - 0x1c], esi
    var_1c = (esi);
    // 0051FFD5  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 0051FFD8  mov dword ptr [ebp + 8], esi
    a1 = (esi);
LAB_0051FFDB:
    // 0051FFDB  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 0051FFDE  movsx edx, word ptr [eax + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + eax)));
    // 0051FFE5  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0051FFE8  mov eax, dword ptr [eax*4 + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + eax*4)));
    // 0051FFEF  add eax, edx
    eax = ((eax) + (edx));
    // 0051FFF1  mov dl, byte ptr [0x94988c]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094988c))));
    // 0051FFF7  test dl, 1
    // 0051FFFA  jne 0x52000b
    if (((((edx & 0xff)) & (1))) != 0) goto LAB_0052000B;
    // 0051FFFC  test eax, eax
    // 0051FFFE  jge 0x520005
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00520005;
    // 00520000  lea esi, [eax + ecx]
    esi = (ecx);
    // 00520003  jmp 0x52000d
    goto LAB_0052000D;
LAB_00520005:
    // 00520005  cmp eax, ecx
    // 00520007  jl 0x52000b
    if (((int)(eax)) < ((int)(ecx))) goto LAB_0052000B;
    // 00520009  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_0052000B:
    // 0052000B  mov esi, eax
    esi = (eax);
LAB_0052000D:
    // 0052000D  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00520010  movsx eax, word ptr [edx + 0x97d042]
    eax = ((*(short*)((char*)g_0097d042 + edx)));
    // 00520017  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 0052001A  mov edi, dword ptr [edx*4 + 0x66f440]
    edi = ((*(int*)((char*)g_0066f440 + edx*4)));
    // 00520021  add edi, eax
    edi = ((edi) + (eax));
    // 00520023  js 0x5200e1
    if (((((int)(edi))) - (((int)(0x0)))) < 0) goto LAB_005200E1;
    // 00520029  cmp edi, dword ptr [0x949874]
    // 0052002F  jge 0x5200e1
    if (((int)(edi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_005200E1;
    // 00520035  test esi, esi
    // 00520037  jl 0x5200e1
    if (((int)(((esi) & (esi)))) < 0) goto LAB_005200E1;
    // 0052003D  cmp esi, ecx
    // 0052003F  jge 0x5200e1
    if (((int)(esi)) >= ((int)(ecx))) goto LAB_005200E1;
    // 00520045  push 0
    // 00520047  push edi
    // 00520048  push esi
    // 00520049  call 0x592030
    eax = bonus_at(esi, edi, 0);
    // 0052004E  add esp, 0xc
    // 00520051  test eax, eax
    // 00520053  je 0x5200db
    if ((((eax) & (eax))) == 0) goto LAB_005200DB;
    // 00520059  mov eax, ebx
    eax = (ebx);
    // 0052005B  shl eax, 6
    eax = ((eax) << (6));
    // 0052005E  add eax, ebx
    eax = ((eax) + (ebx));
    // 00520060  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 00520063  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 00520066  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00520069  mov ecx, dword ptr [eax*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + eax*4)));
    // 00520070  test ch, 2
    // 00520073  jne 0x52009e
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_0052009E;
    // 00520075  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 0052007A  mov ecx, esi
    ecx = (esi);
    // 0052007C  imul eax, edi
    eax = ((eax) * (edi));
    // 0052007F  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 00520081  add eax, ecx
    eax = ((eax) + (ecx));
    // 00520083  mov ecx, ebx
    ecx = (ebx);
    // 00520085  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00520088  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 0052008B  mov edx, 1
    edx = (1);
    // 00520090  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00520092  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 00520098  test byte ptr [ecx + eax*4 + 4], dl
    // 0052009C  je 0x5200db
    if (((((*(unsigned char*)((char*)(ecx) + eax*4 + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_005200DB;
LAB_0052009E:
    // 0052009E  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005200A3  mov edx, esi
    edx = (esi);
    // 005200A5  imul eax, edi
    eax = ((eax) * (edi));
    // 005200A8  sar edx, 1
    edx = ((((int)(edx))) >> (1));
    // 005200AA  add eax, edx
    eax = ((eax) + (edx));
    // 005200AC  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005200AF  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005200B2  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005200B8  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005200BA  mov al, byte ptr [ecx + edx*4]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + edx*4))));
    // 005200BD  and al, 0xe0
    SETB0(eax, (((eax & 0xff)) & (0xe0)));
    // 005200BF  cmp eax, 0x60
    // 005200C2  jl 0x5200f6
    if (((int)(eax)) < ((int)(0x60))) goto LAB_005200F6;
    // 005200C4  push 0
    // 005200C6  push 0
    // 005200C8  push edi
    // 005200C9  push esi
    // 005200CA  push ebx
    // 005200CB  call 0x4e3ef0
    eax = whose_territory(ebx, esi, edi, 0, 0);
    // 005200D0  add esp, 0x14
    // 005200D3  test eax, eax
    // 005200D5  jl 0x5200f6
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005200F6;
    // 005200D7  cmp eax, ebx
    // 005200D9  je 0x5200f6
    if ((eax) == (ebx)) goto LAB_005200F6;
LAB_005200DB:
    // 005200DB  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
LAB_005200E1:
    // 005200E1  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005200E4  inc eax
    eax = ((eax) + 1);
    // 005200E5  cmp eax, 9
    // 005200E8  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 005200EB  jl 0x51ffdb
    if (((int)(eax)) < ((int)(9))) goto LAB_0051FFDB;
    // 005200F1  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 005200F4  jmp 0x520106
    goto LAB_00520106;
LAB_005200F6:
    // 005200F6  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005200F9  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005200FF  mov dword ptr [ebp - 0x1c], 1
    var_1c = (1);
LAB_00520106:
    // 00520106  mov esi, dword ptr [ebp - 0xc]
    esi = (var_c);
    // 00520109  mov edi, dword ptr [eax*4 + 0x66efbc]
    edi = ((*(int*)((char*)g_0066efbc + eax*4)));
    // 00520110  movsx edx, word ptr [esi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + esi)));
    // 00520117  add edi, edx
    edi = ((edi) + (edx));
    // 00520119  mov dl, byte ptr [0x94988c]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094988c))));
    // 0052011F  test dl, 1
    // 00520122  jne 0x520132
    if (((((edx & 0xff)) & (1))) != 0) goto LAB_00520132;
    // 00520124  test edi, edi
    // 00520126  jge 0x52012c
    if (((int)(((edi) & (edi)))) >= 0) goto LAB_0052012C;
    // 00520128  add edi, ecx
    edi = ((edi) + (ecx));
    // 0052012A  jmp 0x520132
    goto LAB_00520132;
LAB_0052012C:
    // 0052012C  cmp edi, ecx
    // 0052012E  jl 0x520132
    if (((int)(edi)) < ((int)(ecx))) goto LAB_00520132;
    // 00520130  sub edi, ecx
    edi = ((edi) - (ecx));
LAB_00520132:
    // 00520132  movsx edx, word ptr [esi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + esi)));
    // 00520139  mov esi, dword ptr [eax*4 + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + eax*4)));
    // 00520140  add esi, edx
    esi = ((esi) + (edx));
    // 00520142  js 0x522116
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_00522116;
    // 00520148  cmp esi, dword ptr [0x949874]
    // 0052014E  jge 0x522116
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00522116;
    // 00520154  test edi, edi
    // 00520156  jl 0x522116
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00522116;
    // 0052015C  cmp edi, ecx
    // 0052015E  jge 0x522116
    if (((int)(edi)) >= ((int)(ecx))) goto LAB_00522116;
    // 00520164  mov eax, ebx
    eax = (ebx);
    // 00520166  shl eax, 6
    eax = ((eax) << (6));
    // 00520169  add eax, ebx
    eax = ((eax) + (ebx));
    // 0052016B  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 0052016E  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 00520171  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00520174  mov ecx, dword ptr [eax*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + eax*4)));
    // 0052017B  test ch, 2
    // 0052017E  jne 0x5201ad
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_005201AD;
    // 00520180  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00520185  mov ecx, edi
    ecx = (edi);
    // 00520187  imul eax, esi
    eax = ((eax) * (esi));
    // 0052018A  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 0052018C  add eax, ecx
    eax = ((eax) + (ecx));
    // 0052018E  mov ecx, ebx
    ecx = (ebx);
    // 00520190  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00520193  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 00520196  mov edx, 1
    edx = (1);
    // 0052019B  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 0052019D  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005201A3  test byte ptr [ecx + eax*4 + 4], dl
    // 005201A7  je 0x522116
    if (((((*(unsigned char*)((char*)(ecx) + eax*4 + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_00522116;
LAB_005201AD:
    // 005201AD  mov ecx, dword ptr [0x68faf0]
    ecx = ((*(int*)((char*)g_0068faf0)));
    // 005201B3  mov eax, edi
    eax = (edi);
    // 005201B5  imul ecx, esi
    ecx = ((ecx) * (esi));
    // 005201B8  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 005201BA  add ecx, eax
    ecx = ((ecx) + (eax));
    // 005201BC  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005201BF  mov eax, ecx
    eax = (ecx);
    // 005201C1  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005201C3  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005201C6  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005201C9  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005201CF  mov cl, byte ptr [edx + eax*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(edx) + eax*4))));
    // 005201D2  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 005201D5  cmp ecx, 0x60
    // 005201D8  jl 0x5201f5
    if (((int)(ecx)) < ((int)(0x60))) goto LAB_005201F5;
    // 005201DA  push 0
    // 005201DC  push 0
    // 005201DE  push esi
    // 005201DF  push edi
    // 005201E0  push ebx
    // 005201E1  call 0x4e3ef0
    eax = whose_territory(ebx, edi, esi, 0, 0);
    // 005201E6  add esp, 0x14
    // 005201E9  test eax, eax
    // 005201EB  jl 0x5201f5
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005201F5;
    // 005201ED  cmp eax, ebx
    // 005201EF  jne 0x522116
    if ((eax) != (ebx)) goto LAB_00522116;
LAB_005201F5:
    // 005201F5  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 005201F8  test eax, eax
    // 005201FA  je 0x52037f
    if ((((eax) & (eax))) == 0) goto LAB_0052037F;
    // 00520200  push 0
    // 00520202  push esi
    // 00520203  push edi
    // 00520204  call 0x592030
    eax = bonus_at(edi, esi, 0);
    // 00520209  add esp, 0xc
    // 0052020C  test eax, eax
    // 0052020E  je 0x52037f
    if ((((eax) & (eax))) == 0) goto LAB_0052037F;
    // 00520214  cmp dword ptr [0x9a64d4], 0x4b
    // 0052021B  jl 0x522116
    if (((int)((*(int*)((char*)g_009a64d4)))) < ((int)(0x4b))) goto LAB_00522116;
    // 00520221  push 0
    // 00520223  push esi
    // 00520224  push edi
    // 00520225  call 0x592030
    eax = bonus_at(edi, esi, 0);
    // 0052022A  dec eax
    eax = ((eax) - 1);
    // 0052022B  push -1
    // 0052022D  push -1
    // 0052022F  mov eax, dword ptr [eax*8 + 0x94615c]
    eax = ((*(int*)((char*)g_0094615c + eax*8)));
    // 00520236  push eax
    // 00520237  push 1
    // 00520239  call 0x625e50
    eax = parse_say(1, eax, -1, -1);
    // 0052023E  push 0
    // 00520240  push 0x400
    // 00520245  push esi
    // 00520246  push edi
    // 00520247  call 0x591d60
    bit_set(edi, esi, 0x400, 0);
    // 0052024C  push ebx
    // 0052024D  push esi
    // 0052024E  push edi
    // 0052024F  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 00520254  push 2
    // 00520256  push esi
    // 00520257  push edi
    // 00520258  call 0x46af40
    draw_tile(edi, esi, 2);
    // 0052025D  add esp, 0x44
    // 00520260  push 0
    // 00520262  push esi
    // 00520263  push edi
    // 00520264  call 0x592030
    eax = bonus_at(edi, esi, 0);
    // 00520269  add esp, 0xc
    // 0052026C  test eax, eax
    // 0052026E  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 00520274  cmp ebx, dword ptr [0x939284]
    // 0052027A  jne 0x522116
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_00522116;
    // 00520280  mov ecx, dword ptr [0x7d3c3c]
    ecx = ((*(int*)((char*)g_007d3c3c)));
    // 00520286  push 1
    // 00520288  push esi
    // 00520289  push edi
    // 0052028A  call 0x46b1f0
    reinterpret_cast<MapWin*>(ecx)->set_center(edi, esi, 1);
    // 0052028F  push 0x10
    // 00520291  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00520297  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00520299  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 0052029B  test cl, cl
    // 0052029D  jne 0x520341
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00520341;
    // 005202A3  mov dword ptr [ebp + 8], 0xa
    a1 = (0xa);
LAB_005202AA:
    // 005202AA  push 1
    // 005202AC  push 0x400
    // 005202B1  push esi
    // 005202B2  push edi
    // 005202B3  call 0x591d60
    bit_set(edi, esi, 0x400, 1);
    // 005202B8  push ebx
    // 005202B9  push esi
    // 005202BA  push edi
    // 005202BB  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 005202C0  push 2
    // 005202C2  push esi
    // 005202C3  push edi
    // 005202C4  call 0x46af40
    draw_tile(edi, esi, 2);
    // 005202C9  add esp, 0x28
    // 005202CC  push 0x10
    // 005202CE  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 005202D4  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005202D6  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 005202D8  test dl, dl
    // 005202DA  jne 0x5202ef
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_005202EF;
    // 005202DC  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 005202E1  test eax, eax
    // 005202E3  jne 0x5202ef
    if ((((eax) & (eax))) != 0) goto LAB_005202EF;
    // 005202E5  push 0x14
    // 005202E7  call 0x50dd90
    clock_wait(0x14);
    // 005202EC  add esp, 4
LAB_005202EF:
    // 005202EF  push 0
    // 005202F1  push 0x400
    // 005202F6  push esi
    // 005202F7  push edi
    // 005202F8  call 0x591d60
    bit_set(edi, esi, 0x400, 0);
    // 005202FD  push ebx
    // 005202FE  push esi
    // 005202FF  push edi
    // 00520300  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 00520305  push 2
    // 00520307  push esi
    // 00520308  push edi
    // 00520309  call 0x46af40
    draw_tile(edi, esi, 2);
    // 0052030E  add esp, 0x28
    // 00520311  push 0x10
    // 00520313  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00520319  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0052031B  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 0052031D  test cl, cl
    // 0052031F  jne 0x520334
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00520334;
    // 00520321  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00520326  test eax, eax
    // 00520328  jne 0x520334
    if ((((eax) & (eax))) != 0) goto LAB_00520334;
    // 0052032A  push 0x14
    // 0052032C  call 0x50dd90
    clock_wait(0x14);
    // 00520331  add esp, 4
LAB_00520334:
    // 00520334  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00520337  dec eax
    eax = ((eax) - 1);
    // 00520338  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0052033B  jne 0x5202aa
    if ((eax) != (0x0)) goto LAB_005202AA;
LAB_00520341:
    // 00520341  push 0x10
    // 00520343  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00520349  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0052034B  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 0052034D  test dl, dl
    // 0052034F  jne 0x520364
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_00520364;
    // 00520351  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00520356  test eax, eax
    // 00520358  jne 0x520364
    if ((((eax) & (eax))) != 0) goto LAB_00520364;
    // 0052035A  push 0x64
    // 0052035C  call 0x50dd90
    clock_wait(0x64);
    // 00520361  add esp, 4
LAB_00520364:
    // 00520364  push -1
    // 00520366  push 0x68b228
    // 0052036B  push 0x68b23c
    // 00520370  call 0x405140
    eax = POP2((const char*)(0x68b23c), (const char*)(0x68b228), -1);
    // 00520375  add esp, 0xc
    return;
LAB_0052037F:
    // 0052037F  push 1
    // 00520381  push 0x400
    // 00520386  push esi
    // 00520387  push edi
    // 00520388  call 0x591d60
    bit_set(edi, esi, 0x400, 1);
    // 0052038D  push ebx
    // 0052038E  push esi
    // 0052038F  push edi
    // 00520390  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 00520395  push 2
    // 00520397  push esi
    // 00520398  push edi
    // 00520399  call 0x46af40
    draw_tile(edi, esi, 2);
    // 0052039E  push 0
    // 005203A0  push esi
    // 005203A1  push edi
    // 005203A2  call 0x592030
    eax = bonus_at(edi, esi, 0);
    // 005203A7  add esp, 0x34
    // 005203AA  dec eax
    eax = ((eax) - 1);
    // 005203AB  mov dword ptr [ebp - 0x20], eax
    var_20 = (eax);
    // 005203AE  js 0x522116
    if (((((int)(eax))) - (((int)(0x0)))) < 0) goto LAB_00522116;
    // 005203B4  cmp ebx, dword ptr [0x939284]
    // 005203BA  jne 0x522116
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_00522116;
    // 005203C0  mov ecx, dword ptr [0x7d3c3c]
    ecx = ((*(int*)((char*)g_007d3c3c)));
    // 005203C6  push 1
    // 005203C8  push esi
    // 005203C9  push edi
    // 005203CA  call 0x46b1f0
    reinterpret_cast<MapWin*>(ecx)->set_center(edi, esi, 1);
    // 005203CF  push 0x10
    // 005203D1  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 005203D7  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005203D9  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 005203DB  test cl, cl
    // 005203DD  jne 0x520481
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00520481;
    // 005203E3  mov dword ptr [ebp + 8], 0xa
    a1 = (0xa);
LAB_005203EA:
    // 005203EA  push 0
    // 005203EC  push 0x400
    // 005203F1  push esi
    // 005203F2  push edi
    // 005203F3  call 0x591d60
    bit_set(edi, esi, 0x400, 0);
    // 005203F8  push ebx
    // 005203F9  push esi
    // 005203FA  push edi
    // 005203FB  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 00520400  push 2
    // 00520402  push esi
    // 00520403  push edi
    // 00520404  call 0x46af40
    draw_tile(edi, esi, 2);
    // 00520409  add esp, 0x28
    // 0052040C  push 0x10
    // 0052040E  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00520414  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520416  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 00520418  test dl, dl
    // 0052041A  jne 0x52042f
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_0052042F;
    // 0052041C  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00520421  test eax, eax
    // 00520423  jne 0x52042f
    if ((((eax) & (eax))) != 0) goto LAB_0052042F;
    // 00520425  push 0x14
    // 00520427  call 0x50dd90
    clock_wait(0x14);
    // 0052042C  add esp, 4
LAB_0052042F:
    // 0052042F  push 1
    // 00520431  push 0x400
    // 00520436  push esi
    // 00520437  push edi
    // 00520438  call 0x591d60
    bit_set(edi, esi, 0x400, 1);
    // 0052043D  push ebx
    // 0052043E  push esi
    // 0052043F  push edi
    // 00520440  call 0x591e50
    synch_bit(edi, esi, ebx);
    // 00520445  push 2
    // 00520447  push esi
    // 00520448  push edi
    // 00520449  call 0x46af40
    draw_tile(edi, esi, 2);
    // 0052044E  add esp, 0x28
    // 00520451  push 0x10
    // 00520453  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00520459  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0052045B  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 0052045D  test cl, cl
    // 0052045F  jne 0x520474
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00520474;
    // 00520461  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00520466  test eax, eax
    // 00520468  jne 0x520474
    if ((((eax) & (eax))) != 0) goto LAB_00520474;
    // 0052046A  push 0x14
    // 0052046C  call 0x50dd90
    clock_wait(0x14);
    // 00520471  add esp, 4
LAB_00520474:
    // 00520474  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00520477  dec eax
    eax = ((eax) - 1);
    // 00520478  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0052047B  jne 0x5203ea
    if ((eax) != (0x0)) goto LAB_005203EA;
LAB_00520481:
    // 00520481  mov edi, dword ptr [ebp - 0x20]
    edi = (var_20);
    // 00520484  push -1
    // 00520486  push -1
    // 00520488  mov edx, dword ptr [edi*8 + 0x94615c]
    edx = ((*(int*)((char*)g_0094615c + edi*8)));
    // 0052048F  push edx
    // 00520490  push 1
    // 00520492  call 0x625e50
    eax = parse_say(1, edx, -1, -1);
    // 00520497  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 0052049C  mov ecx, dword ptr [ebp - 0x18]
    ecx = (var_18);
    // 0052049F  imul eax, esi
    eax = ((eax) * (esi));
    // 005204A2  add eax, ecx
    eax = ((eax) + (ecx));
    // 005204A4  add esp, 0x10
    // 005204A7  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005204AA  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005204AD  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005204B3  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005204B5  mov al, byte ptr [ecx + edx*4]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + edx*4))));
    // 005204B8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005204BA  and al, 0xe0
    SETB0(eax, (((eax & 0xff)) & (0xe0)));
    // 005204BC  cmp eax, 0x60
    // 005204BF  setl dl
    SETB0(edx, ((((int)(eax)) < ((int)(0x60))) ? 1 : 0));
    // 005204C2  test edi, edi
    // 005204C4  mov eax, edx
    eax = (edx);
    // 005204C6  jne 0x520504
    if ((((edi) & (edi))) != 0) goto LAB_00520504;
    // 005204C8  test eax, eax
    // 005204CA  je 0x5204e8
    if ((((eax) & (eax))) == 0) goto LAB_005204E8;
    // 005204CC  mov eax, 0x68b248
    eax = (0x68b248);
    // 005204D1  push -1
    // 005204D3  push eax
    // 005204D4  push 0x68b2a8
    // 005204D9  call 0x405140
    eax = POP2((const char*)(0x68b2a8), (const char*)(eax), -1);
    // 005204DE  add esp, 0xc
    return;
LAB_005204E8:
    // 005204E8  mov eax, 0x68b258
    eax = (0x68b258);
    // 005204ED  push -1
    // 005204EF  push eax
    // 005204F0  push 0x68b2a8
    // 005204F5  call 0x405140
    eax = POP2((const char*)(0x68b2a8), (const char*)(eax), -1);
    // 005204FA  add esp, 0xc
    return;
LAB_00520504:
    // 00520504  test eax, eax
    // 00520506  mov eax, 0x68b268
    eax = (0x68b268);
    // 0052050B  jne 0x520512
    if ((((eax) & (eax))) != 0) goto LAB_00520512;
    // 0052050D  mov eax, 0x68b278
    eax = (0x68b278);
LAB_00520512:
    // 00520512  push -1
    // 00520514  push eax
    // 00520515  push 0x68b2a8
    // 0052051A  call 0x405140
    eax = POP2((const char*)(0x68b2a8), (const char*)(eax), -1);
    // 0052051F  add esp, 0xc
    return;
LAB_00520529:
    // 00520529  mov eax, dword ptr [0x9a6818]
    eax = ((*(int*)((char*)g_009a6818)));
    // 0052052E  test eax, eax
    // 00520530  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 00520536  xor esi, esi
    esi = ((esi) ^ (esi));
    // 00520538  push 5
    // 0052053A  push esi
    // 0052053B  call 0x625810
    eax = random(esi, 5);
    // 00520540  add esp, 8
    // 00520543  test eax, eax
    // 00520545  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 0052054B  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0052054D  mov eax, 0x96f2c0
    eax = (0x96f2c0);
LAB_00520552:
    // 00520552  cmp dword ptr [eax - 4], esi
    // 00520555  jne 0x52055b
    if (((*(int*)((char*)(eax) + (-0x4)))) != (esi)) goto LAB_0052055B;
    // 00520557  cmp dword ptr [eax], esi
    // 00520559  je 0x520565
    if (((*(int*)((char*)(eax)))) == (esi)) goto LAB_00520565;
LAB_0052055B:
    // 0052055B  mov dword ptr [eax - 4], esi
    (*(int*)((char*)(eax) + (-0x4))) = (esi);
    // 0052055E  mov dword ptr [eax], esi
    (*(int*)((char*)(eax))) = (esi);
    // 00520560  mov ecx, 1
    ecx = (1);
LAB_00520565:
    // 00520565  add eax, 0x20cc
    eax = ((eax) + (0x20cc));
    // 0052056A  cmp eax, 0x97d854
    // 0052056F  jl 0x520552
    if (((int)(eax)) < ((int)(0x97d854))) goto LAB_00520552;
    // 00520571  cmp ecx, esi
    // 00520573  mov dword ptr [0x9a6818], 1
    (*(int*)((char*)g_009a6818)) = (1);
    // 0052057D  push -1
    // 0052057F  je 0x52059a
    if ((ecx) == (esi)) goto LAB_0052059A;
    // 00520581  push 0x68b2b4
    // 00520586  push 0x68b2c0
    // 0052058B  call 0x405140
    eax = POP2((const char*)(0x68b2c0), (const char*)(0x68b2b4), -1);
    // 00520590  add esp, 0xc
    return;
LAB_0052059A:
    // 0052059A  push 0x68b2cc
    // 0052059F  push 0x68b2d8
    // 005205A4  call 0x405140
    eax = POP2((const char*)(0x68b2d8), (const char*)(0x68b2cc), -1);
    // 005205A9  add esp, 0xc
    return;
LAB_005205B3:
    // 005205B3  push 5
    // 005205B5  push 0
    // 005205B7  call 0x625810
    eax = random(0, 5);
    // 005205BC  add esp, 8
    // 005205BF  test eax, eax
    // 005205C1  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 005205C7  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005205C9  mov eax, 0x96f2b8
    eax = (0x96f2b8);
LAB_005205CE:
    // 005205CE  cmp dword ptr [eax], 0
    // 005205D1  je 0x5205de
    if (((*(int*)((char*)(eax)))) == (0)) goto LAB_005205DE;
    // 005205D3  mov dword ptr [eax], 0
    (*(int*)((char*)(eax))) = (0);
    // 005205D9  mov ecx, 1
    ecx = (1);
LAB_005205DE:
    // 005205DE  add eax, 0x20cc
    eax = ((eax) + (0x20cc));
    // 005205E3  cmp eax, 0x97d84c
    // 005205E8  jl 0x5205ce
    if (((int)(eax)) < ((int)(0x97d84c))) goto LAB_005205CE;
    // 005205EA  test ecx, ecx
    // 005205EC  je 0x522116
    if ((((ecx) & (ecx))) == 0) goto LAB_00522116;
    // 005205F2  push -1
    // 005205F4  push 0x68b2e4
    // 005205F9  push 0x68b2f0
    // 005205FE  call 0x405140
    eax = POP2((const char*)(0x68b2f0), (const char*)(0x68b2e4), -1);
    // 00520603  add esp, 0xc
    return;
LAB_0052060D:
    // 0052060D  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
    // 00520612  cmp eax, 0x32
    // 00520615  jl 0x522116
    if (((int)(eax)) < ((int)(0x32))) goto LAB_00522116;
    // 0052061B  mov ecx, dword ptr [0x9a6800]
    ecx = ((*(int*)((char*)g_009a6800)));
    // 00520621  cmp ecx, -0x28
    // 00520624  jg 0x522116
    if (((int)(ecx)) > ((int)(-0x28))) goto LAB_00522116;
    // 0052062A  cmp ecx, -0x50
    // 0052062D  jle 0x520648
    if (((int)(ecx)) <= ((int)(-0x50))) goto LAB_00520648;
    // 0052062F  cmp eax, 0x50
    // 00520632  jl 0x520648
    if (((int)(eax)) < ((int)(0x50))) goto LAB_00520648;
    // 00520634  push 3
    // 00520636  push 0
    // 00520638  call 0x625810
    eax = random(0, 3);
    // 0052063D  add esp, 8
    // 00520640  test eax, eax
    // 00520642  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
LAB_00520648:
    // 00520648  push 0xb
    // 0052064A  push 0
    // 0052064C  call 0x625810
    eax = random(0, 0xb);
    // 00520651  add eax, 0xa
    eax = ((eax) + (0xa));
    // 00520654  push 0x14
    // 00520656  push 0
    // 00520658  mov dword ptr [0x9a6800], eax
    (*(int*)((char*)g_009a6800)) = (eax);
    // 0052065D  call 0x625e30
    eax = parse_num(0, 0x14);
    // 00520662  push -1
    // 00520664  push 0x68b2fc
    // 00520669  push 0x68b30c
    // 0052066E  call 0x405140
    eax = POP2((const char*)(0x68b30c), (const char*)(0x68b2fc), -1);
    // 00520673  add esp, 0x1c
    return;
LAB_0052067D:
    // 0052067D  mov eax, ebx
    eax = (ebx);
    // 0052067F  shl eax, 6
    eax = ((eax) << (6));
    // 00520682  add eax, ebx
    eax = ((eax) + (ebx));
    // 00520684  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 00520687  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0052068A  lea edi, [ebx + edx*2]
    edi = (ebx + edx*2);
    // 0052068D  shl edi, 2
    edi = ((edi) << (2));
    // 00520690  mov esi, dword ptr [edi + 0x96cc00]
    esi = ((*(int*)((char*)g_0096cc00 + edi)));
    // 00520696  cmp esi, 0x3e8
    // 0052069C  jg 0x520700
    if (((int)(esi)) > ((int)(0x3e8))) goto LAB_00520700;
    // 0052069E  cmp dword ptr [edi + 0x96c9e4], 4
    // 005206A5  jg 0x522116
    if (((int)((*(int*)((char*)g_0096c9e4 + edi)))) > ((int)(4))) goto LAB_00522116;
    // 005206AB  cmp esi, 0x1f4
    // 005206B1  jg 0x522116
    if (((int)(esi)) > ((int)(0x1f4))) goto LAB_00522116;
    // 005206B7  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005206BD  lea eax, [esi + esi]
    eax = (esi);
    // 005206C0  cmp ebx, ecx
    // 005206C2  mov dword ptr [edi + 0x96cc00], eax
    (*(int*)((char*)g_0096cc00 + edi)) = (eax);
    // 005206C8  jne 0x522116
    if ((ebx) != (ecx)) goto LAB_00522116;
    // 005206CE  push eax
    // 005206CF  push 0
    // 005206D1  call 0x625e30
    eax = parse_num(0, eax);
    // 005206D6  add esp, 8
    // 005206D9  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005206DE  push 0
    // 005206E0  call 0x514880
    reinterpret_cast<Console*>(ecx)->update_data(0);
    // 005206E5  push -1
    // 005206E7  push 0x68b318
    // 005206EC  push 0x68b328
    // 005206F1  call 0x405140
    eax = POP2((const char*)(0x68b328), (const char*)(0x68b318), -1);
    // 005206F6  add esp, 0xc
    return;
LAB_00520700:
    // 00520700  mov eax, 1
    eax = (1);
    // 00520705  mov ecx, ebx
    ecx = (ebx);
    // 00520707  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 00520709  test byte ptr [0x9a64e8], al
    // 0052070F  jne 0x52071d
    if (((((*(unsigned char*)((char*)g_009a64e8))) & ((eax & 0xff)))) != 0) goto LAB_0052071D;
    // 00520711  cmp esi, 0x7d0
    // 00520717  jle 0x522116
    if (((int)(esi)) <= ((int)(0x7d0))) goto LAB_00522116;
LAB_0052071D:
    // 0052071D  mov eax, esi
    eax = (esi);
    // 0052071F  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00520725  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00520726  and edx, 3
    edx = ((edx) & (3));
    // 00520729  add eax, edx
    eax = ((eax) + (edx));
    // 0052072B  sar eax, 2
    eax = ((((int)(eax))) >> (2));
    // 0052072E  cmp ebx, ecx
    // 00520730  mov dword ptr [edi + 0x96cc00], eax
    (*(int*)((char*)g_0096cc00 + edi)) = (eax);
    // 00520736  jne 0x52076a
    if ((ebx) != (ecx)) goto LAB_0052076A;
    // 00520738  push eax
    // 00520739  push 0
    // 0052073B  call 0x625e30
    eax = parse_num(0, eax);
    // 00520740  add esp, 8
    // 00520743  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00520748  push 0
    // 0052074A  call 0x514880
    reinterpret_cast<Console*>(ecx)->update_data(0);
    // 0052074F  push -1
    // 00520751  push 0x68b334
    // 00520756  push 0x68b344
    // 0052075B  call 0x405140
    eax = POP2((const char*)(0x68b344), (const char*)(0x68b334), -1);
    // 00520760  add esp, 0xc
    return;
LAB_0052076A:
    // 0052076A  mov ecx, dword ptr [0x93a95c]
    ecx = ((*(int*)((char*)g_0093a95c)));
    // 00520770  test ecx, ecx
    // 00520772  je 0x522116
    if ((((ecx) & (ecx))) == 0) goto LAB_00522116;
    // 00520778  push eax
    // 00520779  push 0
    // 0052077B  call 0x625e30
    eax = parse_num(0, eax);
    // 00520780  or ecx, 0xffffffff
    ecx = ((ecx) | (0xffffffff));
    // 00520783  sub ecx, ebx
    ecx = ((ecx) - (ebx));
    // 00520785  push ecx
    // 00520786  push 0x68b350
    // 0052078B  push 0x68b360
    // 00520790  call 0x405140
    eax = POP2((const char*)(0x68b360), (const char*)(0x68b350), ecx);
    // 00520795  add esp, 0x14
    return;
LAB_0052079F:
    // 0052079F  mov esi, dword ptr [0x9a64d4]
    esi = ((*(int*)((char*)g_009a64d4)));
    // 005207A5  cmp esi, 0x4b
    // 005207A8  jl 0x522116
    if (((int)(esi)) < ((int)(0x4b))) goto LAB_00522116;
    // 005207AE  mov edx, ebx
    edx = (ebx);
    // 005207B0  shl edx, 6
    edx = ((edx) << (6));
    // 005207B3  add edx, ebx
    edx = ((edx) + (ebx));
    // 005207B5  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 005207B8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005207BA  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 005207BD  lea eax, [ebx + ecx*2]
    eax = (ebx + ecx*2);
    // 005207C0  shl eax, 2
    eax = ((eax) << (2));
    // 005207C3  cmp esi, 0x96
    // 005207C9  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 005207CC  mov ecx, dword ptr [eax + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + eax)));
    // 005207D2  setge dl
    SETB0(edx, ((((int)(esi)) >= ((int)(0x96))) ? 1 : 0));
    // 005207D5  add edx, 4
    edx = ((edx) + (4));
    // 005207D8  cmp ecx, edx
    // 005207DA  jl 0x522116
    if (((int)(ecx)) < ((int)(edx))) goto LAB_00522116;
    // 005207E0  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005207E2  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 005207E5  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
LAB_005207E8:
    // 005207E8  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 005207EB  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 005207EE  movsx esi, word ptr [edx + 0x97d040]
    esi = ((*(short*)((char*)g_0097d040 + edx)));
    // 005207F5  mov eax, dword ptr [ecx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ecx)));
    // 005207FB  add eax, esi
    eax = ((eax) + (esi));
    // 005207FD  test byte ptr [0x94988c], 1
    // 00520804  jne 0x52081a
    if (((((*(unsigned char*)((char*)g_0094988c))) & (1))) != 0) goto LAB_0052081A;
    // 00520806  mov esi, dword ptr [0x949870]
    esi = ((*(int*)((char*)g_00949870)));
    // 0052080C  test eax, eax
    // 0052080E  jge 0x520814
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00520814;
    // 00520810  add eax, esi
    eax = ((eax) + (esi));
    // 00520812  jmp 0x52081a
    goto LAB_0052081A;
LAB_00520814:
    // 00520814  cmp eax, esi
    // 00520816  jl 0x52081a
    if (((int)(eax)) < ((int)(esi))) goto LAB_0052081A;
    // 00520818  sub eax, esi
    eax = ((eax) - (esi));
LAB_0052081A:
    // 0052081A  mov esi, dword ptr [ecx + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + ecx)));
    // 00520820  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00520823  movsx eax, word ptr [edx + 0x97d042]
    eax = ((*(short*)((char*)g_0097d042 + edx)));
    // 0052082A  add esi, eax
    esi = ((esi) + (eax));
    // 0052082C  js 0x5209fd
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_005209FD;
    // 00520832  cmp esi, dword ptr [0x949874]
    // 00520838  jge 0x5209fd
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_005209FD;
    // 0052083E  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00520841  test eax, eax
    // 00520843  jl 0x5209fd
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005209FD;
    // 00520849  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 0052084F  cmp eax, ecx
    // 00520851  jge 0x5209fd
    if (((int)(eax)) >= ((int)(ecx))) goto LAB_005209FD;
    // 00520857  mov edi, eax
    edi = (eax);
    // 00520859  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 0052085E  imul eax, esi
    eax = ((eax) * (esi));
    // 00520861  sar edi, 1
    edi = ((((int)(edi))) >> (1));
    // 00520863  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 00520869  add eax, edi
    eax = ((eax) + (edi));
    // 0052086B  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 0052086E  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 00520871  lea eax, [ecx + eax*4]
    eax = (ecx + eax*4);
    // 00520874  test byte ptr [eax + 8], 0x40
    // 00520878  je 0x5209fd
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (0x40))) == 0) goto LAB_005209FD;
    // 0052087E  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 00520881  mov ecx, dword ptr [edx + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx)));
    // 00520887  test ch, 2
    // 0052088A  jne 0x52089e
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_0052089E;
    // 0052088C  mov edx, 1
    edx = (1);
    // 00520891  mov ecx, ebx
    ecx = (ebx);
    // 00520893  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00520895  test byte ptr [eax + 4], dl
    // 00520898  je 0x5209fd
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_005209FD;
LAB_0052089E:
    // 0052089E  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005208A1  push 0
    // 005208A3  push 0
    // 005208A5  push esi
    // 005208A6  push eax
    // 005208A7  push ebx
    // 005208A8  call 0x4e3ef0
    eax = whose_territory(ebx, eax, esi, 0, 0);
    // 005208AD  add esp, 0x14
    // 005208B0  test eax, eax
    // 005208B2  jl 0x5208bc
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005208BC;
    // 005208B4  cmp eax, ebx
    // 005208B6  jne 0x5209fd
    if ((eax) != (ebx)) goto LAB_005209FD;
LAB_005208BC:
    // 005208BC  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005208C1  imul eax, esi
    eax = ((eax) * (esi));
    // 005208C4  add eax, edi
    eax = ((eax) + (edi));
    // 005208C6  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005208C9  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 005208CC  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005208D2  xor eax, eax
    eax = ((eax) ^ (eax));
    // 005208D4  mov al, byte ptr [ecx + edx*4]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + edx*4))));
    // 005208D7  and al, 0xe0
    SETB0(eax, (((eax & 0xff)) & (0xe0)));
    // 005208D9  cmp eax, 0x60
    // 005208DC  jl 0x5209fd
    if (((int)(eax)) < ((int)(0x60))) goto LAB_005209FD;
    // 005208E2  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 005208E7  test eax, eax
    // 005208E9  je 0x5208f4
    if ((((eax) & (eax))) == 0) goto LAB_005208F4;
    // 005208EB  mov dword ptr [ebp - 4], 1
    var_4 = (1);
    // 005208F2  jmp 0x520972
    goto LAB_00520972;
LAB_005208F4:
    // 005208F4  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005208FA  cmp ebx, ecx
    // 005208FC  je 0x52091c
    if ((ebx) == (ecx)) goto LAB_0052091C;
    // 005208FE  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00520901  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520903  mov dl, byte ptr [eax + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax))));
    // 00520909  cmp edx, ecx
    // 0052090B  je 0x52091c
    if ((edx) == (ecx)) goto LAB_0052091C;
    // 0052090D  mov edx, 1
    edx = (1);
    // 00520912  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00520914  test byte ptr [eax + 0x97d04a], dl
    // 0052091A  je 0x520972
    if (((((*(unsigned char*)((char*)g_0097d04a + eax))) & ((edx & 0xff)))) == 0) goto LAB_00520972;
LAB_0052091C:
    // 0052091C  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0052091F  test eax, eax
    // 00520921  jne 0x520962
    if ((((eax) & (eax))) != 0) goto LAB_00520962;
    // 00520923  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 00520926  push ecx
    // 00520927  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 0052092A  push ecx
    // 0052092B  push edx
    // 0052092C  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00520931  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, ecx);
    // 00520936  push 0x10
    // 00520938  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 0052093E  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00520940  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 00520942  test cl, cl
    // 00520944  jne 0x52095c
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_0052095C;
    // 00520946  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 0052094B  test eax, eax
    // 0052094D  jne 0x52095c
    if ((((eax) & (eax))) != 0) goto LAB_0052095C;
    // 0052094F  push 0xc8
    // 00520954  call 0x50dd90
    clock_wait(0xc8);
    // 00520959  add esp, 4
LAB_0052095C:
    // 0052095C  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
LAB_00520962:
    // 00520962  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00520965  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520967  cmp ebx, ecx
    // 00520969  sete dl
    SETB0(edx, (((ebx) == (ecx)) ? 1 : 0));
    // 0052096C  inc edx
    edx = ((edx) + 1);
    // 0052096D  or eax, edx
    eax = ((eax) | (edx));
    // 0052096F  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
LAB_00520972:
    // 00520972  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00520975  push 0
    // 00520977  push 0x40
    // 00520979  push esi
    // 0052097A  push eax
    // 0052097B  call 0x591d60
    bit_set(eax, esi, 0x40, 0);
    // 00520980  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00520983  push ebx
    // 00520984  push esi
    // 00520985  push ecx
    // 00520986  call 0x591e50
    synch_bit(ecx, esi, ebx);
    // 0052098B  add esp, 0x1c
    // 0052098E  mov ecx, 1
    ecx = (1);
LAB_00520993:
    // 00520993  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00520998  imul eax, esi
    eax = ((eax) * (esi));
    // 0052099B  add eax, edi
    eax = ((eax) + (edi));
    // 0052099D  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005209A0  add eax, ecx
    eax = ((eax) + (ecx));
    // 005209A2  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 005209A5  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 005209AA  lea eax, [eax + edx*4 + 0xc]
    eax = (eax + edx*4 + 0xc);
    // 005209AE  mov edx, dword ptr [eax]
    edx = ((*(int*)((char*)(eax))));
    // 005209B0  and edx, 0xffffffbf
    edx = ((edx) & (0xffffffbf));
    // 005209B3  inc ecx
    ecx = ((ecx) + 1);
    // 005209B4  cmp ecx, 8
    // 005209B7  mov dword ptr [eax], edx
    (*(int*)((char*)(eax))) = (edx);
    // 005209B9  jl 0x520993
    if (((int)(ecx)) < ((int)(8))) goto LAB_00520993;
    // 005209BB  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005209BE  test eax, eax
    // 005209C0  je 0x5209fd
    if ((((eax) & (eax))) == 0) goto LAB_005209FD;
    // 005209C2  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 005209C5  push 2
    // 005209C7  push esi
    // 005209C8  push ecx
    // 005209C9  call 0x46af40
    draw_tile(ecx, esi, 2);
    // 005209CE  add esp, 0xc
    // 005209D1  push 0x10
    // 005209D3  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 005209D9  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005209DB  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 005209DD  test dl, dl
    // 005209DF  jne 0x5209fd
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_005209FD;
    // 005209E1  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 005209E6  test eax, eax
    // 005209E8  jne 0x5209fd
    if ((((eax) & (eax))) != 0) goto LAB_005209FD;
    // 005209EA  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 005209EF  test eax, eax
    // 005209F1  jne 0x5209fd
    if ((((eax) & (eax))) != 0) goto LAB_005209FD;
    // 005209F3  push 0x32
    // 005209F5  call 0x50dd90
    clock_wait(0x32);
    // 005209FA  add esp, 4
LAB_005209FD:
    // 005209FD  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00520A00  add eax, 4
    eax = ((eax) + (4));
    // 00520A03  cmp eax, 0x54
    // 00520A06  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 00520A09  jl 0x5207e8
    if (((int)(eax)) < ((int)(0x54))) goto LAB_005207E8;
    // 00520A0F  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00520A12  test eax, eax
    // 00520A14  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 00520A1A  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00520A1D  push eax
    // 00520A1E  push 0x68b36c
    // 00520A23  push 0x68b378
    // 00520A28  call 0x405140
    eax = POP2((const char*)(0x68b378), (const char*)(0x68b36c), eax);
    // 00520A2D  add esp, 0xc
    return;
LAB_00520A37:
    // 00520A37  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00520A3C  test eax, eax
    // 00520A3E  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 00520A44  cmp dword ptr [0x9a64d4], 0x4b
    // 00520A4B  jl 0x522116
    if (((int)((*(int*)((char*)g_009a64d4)))) < ((int)(0x4b))) goto LAB_00522116;
    // 00520A51  mov ecx, ebx
    ecx = (ebx);
    // 00520A53  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00520A56  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 00520A58  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00520A5B  lea eax, [ebx + edx*8]
    eax = (ebx + edx*8);
    // 00520A5E  lea eax, [ebx + eax*2]
    eax = (ebx + eax*2);
    // 00520A61  shl eax, 2
    eax = ((eax) << (2));
    // 00520A64  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 00520A67  cmp dword ptr [eax + 0x96da3c], 8
    // 00520A6E  jl 0x522116
    if (((int)((*(int*)((char*)g_0096da3c + eax)))) < ((int)(8))) goto LAB_00522116;
    // 00520A74  test byte ptr [0x9a64c0], 0x10
    // 00520A7B  jne 0x522116
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x10))) != 0) goto LAB_00522116;
    // 00520A81  mov edx, dword ptr [0x9a6808]
    edx = ((*(int*)((char*)g_009a6808)));
    // 00520A87  test edx, edx
    // 00520A89  jl 0x522116
    if (((int)(((edx) & (edx)))) < 0) goto LAB_00522116;
    // 00520A8F  cmp edx, dword ptr [0x949874]
    // 00520A95  jge 0x522116
    if (((int)(edx)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00522116;
    // 00520A9B  mov ecx, dword ptr [0x9a6804]
    ecx = ((*(int*)((char*)g_009a6804)));
    // 00520AA1  test ecx, ecx
    // 00520AA3  jl 0x522116
    if (((int)(((ecx) & (ecx)))) < 0) goto LAB_00522116;
    // 00520AA9  cmp ecx, dword ptr [0x949870]
    // 00520AAF  jge 0x522116
    if (((int)(ecx)) >= ((int)((*(int*)((char*)g_00949870))))) goto LAB_00522116;
    // 00520AB5  mov eax, edx
    eax = (edx);
    // 00520AB7  mov esi, ecx
    esi = (ecx);
    // 00520AB9  imul eax, dword ptr [0x68faf0]
    eax = ((eax) * ((*(int*)((char*)g_0068faf0))));
    // 00520AC0  sar esi, 1
    esi = ((((int)(esi))) >> (1));
    // 00520AC2  add eax, esi
    eax = ((eax) + (esi));
    // 00520AC4  lea esi, [eax + eax*4]
    esi = (eax + eax*4);
    // 00520AC7  lea eax, [eax + esi*2]
    eax = (eax + esi*2);
    // 00520ACA  mov esi, dword ptr [0x94a30c]
    esi = ((*(int*)((char*)g_0094a30c)));
    // 00520AD0  mov eax, dword ptr [esi + eax*4 + 0xc]
    eax = ((*(int*)((char*)(esi) + eax*4 + 0xc)));
    // 00520AD4  and eax, 0x80000002
    eax = ((eax) & (0x80000002));
    // 00520AD9  cmp eax, 2
    // 00520ADC  jne 0x522116
    if ((eax) != (2)) goto LAB_00522116;
    // 00520AE2  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 00520AE9  movsx esi, word ptr [edi + 0x97d042]
    esi = ((*(short*)((char*)g_0097d042 + edi)));
    // 00520AF0  sub eax, ecx
    eax = ((eax) - (ecx));
    // 00520AF2  mov dword ptr [ebp + 8], edx
    a1 = (edx);
    // 00520AF5  push eax
    // 00520AF6  call 0x644f3a
    eax = abs(eax);
    // 00520AFB  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 00520B01  add esp, 4
    // 00520B04  test cl, 1
    // 00520B07  jne 0x520b1b
    if (((((ecx & 0xff)) & (1))) != 0) goto LAB_00520B1B;
    // 00520B09  cmp eax, dword ptr [0x68faf0]
    // 00520B0F  jle 0x520b1b
    if (((int)(eax)) <= ((int)((*(int*)((char*)g_0068faf0))))) goto LAB_00520B1B;
    // 00520B11  mov edi, dword ptr [0x949870]
    edi = ((*(int*)((char*)g_00949870)));
    // 00520B17  sub edi, eax
    edi = ((edi) - (eax));
    // 00520B19  jmp 0x520b1d
    goto LAB_00520B1D;
LAB_00520B1B:
    // 00520B1B  mov edi, eax
    edi = (eax);
LAB_00520B1D:
    // 00520B1D  sub esi, dword ptr [ebp + 8]
    esi = ((esi) - (a1));
    // 00520B20  push esi
    // 00520B21  call 0x644f3a
    eax = abs(esi);
    // 00520B26  add eax, edi
    eax = ((eax) + (edi));
    // 00520B28  add esp, 4
    // 00520B2B  and al, 0xfe
    SETB0(eax, (((eax & 0xff)) & (0xfe)));
    // 00520B2D  cmp eax, 0xa
    // 00520B30  jg 0x522116
    if (((int)(eax)) > ((int)(0xa))) goto LAB_00522116;
    // 00520B36  mov edi, dword ptr [ebp - 0x18]
    edi = (var_18);
    // 00520B39  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 00520B3F  mov esi, dword ptr [0x68faf0]
    esi = ((*(int*)((char*)g_0068faf0)));
    // 00520B45  mov eax, dword ptr [edi + 0x96c9e0]
    eax = ((*(int*)((char*)g_0096c9e0 + edi)));
    // 00520B4B  test ah, 2
    // 00520B4E  jne 0x520b81
    if ((((((eax >> 8) & 0xff)) & (2))) != 0) goto LAB_00520B81;
    // 00520B50  mov eax, dword ptr [0x9a6808]
    eax = ((*(int*)((char*)g_009a6808)));
    // 00520B55  mov ecx, dword ptr [0x9a6804]
    ecx = ((*(int*)((char*)g_009a6804)));
    // 00520B5B  imul eax, esi
    eax = ((eax) * (esi));
    // 00520B5E  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 00520B60  add eax, ecx
    eax = ((eax) + (ecx));
    // 00520B62  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00520B65  lea eax, [eax + ecx*2]
    eax = (eax + ecx*2);
    // 00520B68  mov ecx, ebx
    ecx = (ebx);
    // 00520B6A  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00520B6D  mov eax, 1
    eax = (1);
    // 00520B72  shl eax, cl
    eax = ((eax) << ((ecx & 0xff)));
    // 00520B74  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00520B77  test byte ptr [edx + ecx*4 + 4], al
    // 00520B7B  je 0x522116
    if (((((*(unsigned char*)((char*)(edx) + ecx*4 + 0x4))) & ((eax & 0xff)))) == 0) goto LAB_00522116;
LAB_00520B81:
    // 00520B81  cmp dword ptr [edi + 0x96c9e4], 6
    // 00520B88  jl 0x522116
    if (((int)((*(int*)((char*)g_0096c9e4 + edi)))) < ((int)(6))) goto LAB_00522116;
    // 00520B8E  mov edi, dword ptr [0x939284]
    edi = ((*(int*)((char*)g_00939284)));
    // 00520B94  mov eax, edi
    eax = (edi);
    // 00520B96  shl eax, 6
    eax = ((eax) << (6));
    // 00520B99  add eax, edi
    eax = ((eax) + (edi));
    // 00520B9B  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 00520B9E  lea eax, [edi + ecx*8]
    eax = (edi + ecx*8);
    // 00520BA1  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 00520BA4  mov eax, dword ptr [ecx*4 + 0x96c9e0]
    eax = ((*(int*)((char*)g_0096c9e0 + ecx*4)));
    // 00520BAB  test ah, 2
    // 00520BAE  jne 0x520bde
    if ((((((eax >> 8) & 0xff)) & (2))) != 0) goto LAB_00520BDE;
    // 00520BB0  mov eax, dword ptr [0x9a6808]
    eax = ((*(int*)((char*)g_009a6808)));
    // 00520BB5  mov ecx, dword ptr [0x9a6804]
    ecx = ((*(int*)((char*)g_009a6804)));
    // 00520BBB  imul eax, esi
    eax = ((eax) * (esi));
    // 00520BBE  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 00520BC0  add eax, ecx
    eax = ((eax) + (ecx));
    // 00520BC2  mov ebx, 1
    ebx = (1);
    // 00520BC7  mov dword ptr [ebp - 4], 0
    var_4 = (0);
    // 00520BCE  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00520BD1  lea eax, [eax + ecx*2]
    eax = (eax + ecx*2);
    // 00520BD4  mov ecx, edi
    ecx = (edi);
    // 00520BD6  shl ebx, cl
    ebx = ((ebx) << ((ecx & 0xff)));
    // 00520BD8  test byte ptr [edx + eax*4 + 4], bl
    // 00520BDC  je 0x520be5
    if (((((*(unsigned char*)((char*)(edx) + eax*4 + 0x4))) & ((ebx & 0xff)))) == 0) goto LAB_00520BE5;
LAB_00520BDE:
    // 00520BDE  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_00520BE5:
    // 00520BE5  mov dword ptr [ebp - 0x1c], 0
    var_1c = (0);
LAB_00520BEC:
    // 00520BEC  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00520BEF  mov esi, dword ptr [0x9a6804]
    esi = ((*(int*)((char*)g_009a6804)));
    // 00520BF5  mov dl, byte ptr [0x94988c]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094988c))));
    // 00520BFB  mov eax, dword ptr [ecx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ecx)));
    // 00520C01  add eax, esi
    eax = ((eax) + (esi));
    // 00520C03  test dl, 1
    // 00520C06  je 0x520c12
    if (((((edx & 0xff)) & (1))) == 0) goto LAB_00520C12;
    // 00520C08  mov edx, dword ptr [0x949870]
    edx = ((*(int*)((char*)g_00949870)));
    // 00520C0E  mov edi, eax
    edi = (eax);
    // 00520C10  jmp 0x520c29
    goto LAB_00520C29;
LAB_00520C12:
    // 00520C12  mov edx, dword ptr [0x949870]
    edx = ((*(int*)((char*)g_00949870)));
    // 00520C18  test eax, eax
    // 00520C1A  jge 0x520c21
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00520C21;
    // 00520C1C  lea edi, [eax + edx]
    edi = (edx);
    // 00520C1F  jmp 0x520c29
    goto LAB_00520C29;
LAB_00520C21:
    // 00520C21  cmp eax, edx
    // 00520C23  jl 0x520c27
    if (((int)(eax)) < ((int)(edx))) goto LAB_00520C27;
    // 00520C25  sub eax, edx
    eax = ((eax) - (edx));
LAB_00520C27:
    // 00520C27  mov edi, eax
    edi = (eax);
LAB_00520C29:
    // 00520C29  mov ebx, dword ptr [ecx + 0x66f440]
    ebx = ((*(int*)((char*)g_0066f440 + ecx)));
    // 00520C2F  mov eax, dword ptr [0x9a6808]
    eax = ((*(int*)((char*)g_009a6808)));
    // 00520C34  add ebx, eax
    ebx = ((ebx) + (eax));
    // 00520C36  mov dword ptr [ebp + 8], ebx
    a1 = (ebx);
    // 00520C39  js 0x520e5f
    if (((((int)(ebx))) - (((int)(0x0)))) < 0) goto LAB_00520E5F;
    // 00520C3F  cmp ebx, dword ptr [0x949874]
    // 00520C45  jge 0x520e5f
    if (((int)(ebx)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00520E5F;
    // 00520C4B  test edi, edi
    // 00520C4D  jl 0x520e5f
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00520E5F;
    // 00520C53  cmp edi, edx
    // 00520C55  jge 0x520e5f
    if (((int)(edi)) >= ((int)(edx))) goto LAB_00520E5F;
    // 00520C5B  push 0
    // 00520C5D  push 0x81e8887c
    // 00520C62  push ebx
    // 00520C63  push edi
    // 00520C64  call 0x591d60
    bit_set(edi, ebx, 0x81e8887c, 0);
    // 00520C69  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00520C6F  push ecx
    // 00520C70  push ebx
    // 00520C71  push edi
    // 00520C72  call 0x591e50
    synch_bit(edi, ebx, ecx);
    // 00520C77  push 2
    // 00520C79  push ebx
    // 00520C7A  push edi
    // 00520C7B  call 0x591bc0
    rocky_set(edi, ebx, 2);
    // 00520C80  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00520C85  mov edx, edi
    edx = (edi);
    // 00520C87  imul eax, ebx
    eax = ((eax) * (ebx));
    // 00520C8A  sar edx, 1
    edx = ((((int)(edx))) >> (1));
    // 00520C8C  add eax, edx
    eax = ((eax) + (edx));
    // 00520C8E  add esp, 0x28
    // 00520C91  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00520C94  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00520C97  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 00520C9C  mov ecx, dword ptr [eax + edx*4 + 0xc]
    ecx = ((*(int*)((char*)(eax) + edx*4 + 0xc)));
    // 00520CA0  and ecx, 0x80000002
    ecx = ((ecx) & (0x80000002));
    // 00520CA6  cmp ecx, 2
    // 00520CA9  jne 0x520cf2
    if ((ecx) != (2)) goto LAB_00520CF2;
    // 00520CAB  mov edx, dword ptr [0x9a64c8]
    edx = ((*(int*)((char*)g_009a64c8)));
    // 00520CB1  lea esi, [edx - 1]
    esi = (edx + (-0x1));
    // 00520CB4  test esi, esi
    // 00520CB6  jl 0x520d9a
    if (((int)(((esi) & (esi)))) < 0) goto LAB_00520D9A;
    // 00520CBC  lea eax, [esi + esi*2]
    eax = (esi + esi*2);
    // 00520CBF  lea ecx, [esi + eax*4]
    ecx = (esi + eax*4);
    // 00520CC2  lea ebx, [ecx*4 + 0x95282a]
    ebx = (ecx*4 + 0x95282a);
LAB_00520CC9:
    // 00520CC9  movsx edx, word ptr [ebx - 2]
    edx = ((*(short*)((char*)(ebx) + (-0x2))));
    // 00520CCD  cmp edx, edi
    // 00520CCF  jne 0x520ce2
    if ((edx) != (edi)) goto LAB_00520CE2;
    // 00520CD1  movsx eax, word ptr [ebx]
    eax = ((*(short*)((char*)(ebx))));
    // 00520CD4  cmp eax, dword ptr [ebp + 8]
    // 00520CD7  jne 0x520ce2
    if ((eax) != (a1)) goto LAB_00520CE2;
    // 00520CD9  push esi
    // 00520CDA  call 0x5c08c0
    veh_kill(esi);
    // 00520CDF  add esp, 4
LAB_00520CE2:
    // 00520CE2  dec esi
    esi = ((esi) - 1);
    // 00520CE3  sub ebx, 0x34
    ebx = ((ebx) - (0x34));
    // 00520CE6  test esi, esi
    // 00520CE8  jge 0x520cc9
    if (((int)(((esi) & (esi)))) >= 0) goto LAB_00520CC9;
    // 00520CEA  mov ebx, dword ptr [ebp + 8]
    ebx = (a1);
    // 00520CED  jmp 0x520d9a
    goto LAB_00520D9A;
LAB_00520CF2:
    // 00520CF2  mov ecx, dword ptr [0x9a64c8]
    ecx = ((*(int*)((char*)g_009a64c8)));
    // 00520CF8  lea eax, [ecx - 1]
    eax = (ecx + (-0x1));
    // 00520CFB  test eax, eax
    // 00520CFD  jl 0x520d9a
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00520D9A;
    // 00520D03  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 00520D06  lea esi, [eax + 1]
    esi = (eax + 0x1);
    // 00520D09  lea ecx, [eax + edx*4]
    ecx = (eax + edx*4);
    // 00520D0C  lea ecx, [ecx*4 + 0x952832]
    ecx = (ecx*4 + 0x952832);
LAB_00520D13:
    // 00520D13  movsx edx, word ptr [ecx - 0xa]
    edx = ((*(short*)((char*)(ecx) + (-0xa))));
    // 00520D17  cmp edx, edi
    // 00520D19  jne 0x520d90
    if ((edx) != (edi)) goto LAB_00520D90;
    // 00520D1B  movsx eax, word ptr [ecx - 8]
    eax = ((*(short*)((char*)(ecx) + (-0x8))));
    // 00520D1F  cmp eax, ebx
    // 00520D21  jne 0x520d90
    if ((eax) != (ebx)) goto LAB_00520D90;
    // 00520D23  movsx eax, word ptr [ecx]
    eax = ((*(short*)((char*)(ecx))));
    // 00520D26  lea edx, [eax + eax*2]
    edx = (eax + eax*2);
    // 00520D29  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00520D2C  shl eax, 2
    eax = ((eax) << (2));
    // 00520D2F  cmp byte ptr [eax + 0x9ab892], 0xc
    // 00520D36  jne 0x520d3f
    if (((*(unsigned char*)((char*)g_009ab892 + eax))) != (0xc)) goto LAB_00520D3F;
    // 00520D38  mov eax, 1
    eax = (1);
    // 00520D3D  jmp 0x520d64
    goto LAB_00520D64;
LAB_00520D3F:
    // 00520D3F  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520D41  mov dl, byte ptr [eax + 0x9ab88f]
    SETB0(edx, ((*(unsigned char*)((char*)g_009ab88f + eax))));
    // 00520D47  mov eax, edx
    eax = (edx);
    // 00520D49  cmp eax, 1
    // 00520D4C  jl 0x520d5a
    if (((int)(eax)) < ((int)(1))) goto LAB_00520D5A;
    // 00520D4E  cmp eax, 0x64
    // 00520D51  jle 0x520d5f
    if (((int)(eax)) <= ((int)(0x64))) goto LAB_00520D5F;
    // 00520D53  mov eax, 0x64
    eax = (0x64);
    // 00520D58  jmp 0x520d5f
    goto LAB_00520D5F;
LAB_00520D5A:
    // 00520D5A  mov eax, 1
    eax = (1);
LAB_00520D5F:
    // 00520D5F  lea eax, [eax + eax*4]
    eax = (eax + eax*4);
    // 00520D62  shl eax, 1
    eax = ((eax) << (1));
LAB_00520D64:
    // 00520D64  mov bl, byte ptr [ecx + 6]
    SETB0(ebx, ((*(unsigned char*)((char*)(ecx) + 0x6))));
    // 00520D67  mov edx, ebx
    edx = (ebx);
    // 00520D69  and edx, 0xff
    edx = ((edx) & (0xff));
    // 00520D6F  sub eax, edx
    eax = ((eax) - (edx));
    // 00520D71  js 0x520d81
    if (((((int)(eax))) - (((int)(0x0)))) < 0) goto LAB_00520D81;
    // 00520D73  cmp eax, 0x270f
    // 00520D78  jle 0x520d83
    if (((int)(eax)) <= ((int)(0x270f))) goto LAB_00520D83;
    // 00520D7A  mov eax, 0x270f
    eax = (0x270f);
    // 00520D7F  jmp 0x520d83
    goto LAB_00520D83;
LAB_00520D81:
    // 00520D81  xor eax, eax
    eax = ((eax) ^ (eax));
LAB_00520D83:
    // 00520D83  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00520D84  sub eax, edx
    eax = ((eax) - (edx));
    // 00520D86  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00520D88  add al, bl
    SETB0(eax, (((eax & 0xff)) + ((ebx & 0xff))));
    // 00520D8A  mov ebx, dword ptr [ebp + 8]
    ebx = (a1);
    // 00520D8D  mov byte ptr [ecx + 6], al
    (*(unsigned char*)((char*)(ecx) + 0x6)) = ((eax & 0xff));
LAB_00520D90:
    // 00520D90  sub ecx, 0x34
    ecx = ((ecx) - (0x34));
    // 00520D93  dec esi
    esi = ((esi) - 1);
    // 00520D94  jne 0x520d13
    if ((esi) != (0x0)) goto LAB_00520D13;
LAB_00520D9A:
    // 00520D9A  push ebx
    // 00520D9B  push edi
    // 00520D9C  call 0x4e3a50
    eax = base_at(edi, ebx);
    // 00520DA1  mov edi, eax
    edi = (eax);
    // 00520DA3  add esp, 8
    // 00520DA6  test edi, edi
    // 00520DA8  jl 0x520e5f
    if (((int)(((edi) & (edi)))) < 0) goto LAB_00520E5F;
    // 00520DAE  lea eax, [edi + edi*8]
    eax = (edi + edi*8);
    // 00520DB1  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520DB3  lea ecx, [edi + eax*2]
    ecx = (edi + eax*2);
    // 00520DB6  lea esi, [edi + ecx*4]
    esi = (edi + ecx*4);
    // 00520DB9  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00520DBF  shl esi, 2
    esi = ((esi) << (2));
    // 00520DC2  mov dl, byte ptr [esi + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + esi))));
    // 00520DC8  cmp edx, ecx
    // 00520DCA  jne 0x520e04
    if ((edx) != (ecx)) goto LAB_00520E04;
    // 00520DCC  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00520DCF  lea edx, [eax + eax*8]
    edx = (eax + eax*8);
    // 00520DD2  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 00520DD5  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00520DD8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00520DDA  mov dl, byte ptr [eax*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 00520DE1  cmp edx, ecx
    // 00520DE3  je 0x520e04
    if ((edx) == (ecx)) goto LAB_00520E04;
    // 00520DE5  push -1
    // 00520DE7  lea eax, [esi + 0x97d053]
    eax = (esi + 0x97d053);
    // 00520DED  push -1
    // 00520DEF  push eax
    // 00520DF0  push 0
    // 00520DF2  mov dword ptr [ebp - 8], edi
    var_8 = (edi);
    // 00520DF5  mov dword ptr [ebp - 4], 1
    var_4 = (1);
    // 00520DFC  call 0x625ec0
    eax = parse_says(0, (char*)(eax), -1, -1);
    // 00520E01  add esp, 0x10
LAB_00520E04:
    // 00520E04  mov ebx, dword ptr [ebp - 0x1c]
    ebx = (var_1c);
    // 00520E07  cmp ebx, 0x24
    // 00520E0A  jge 0x520e24
    if (((int)(ebx)) >= ((int)(0x24))) goto LAB_00520E24;
    // 00520E0C  push edi
    // 00520E0D  call 0x5ac060
    eax = is_objective(edi);
    // 00520E12  add esp, 4
    // 00520E15  test eax, eax
    // 00520E17  jne 0x520e24
    if ((((eax) & (eax))) != 0) goto LAB_00520E24;
    // 00520E19  push edi
    // 00520E1A  call 0x4e5250
    base_kill(edi);
    // 00520E1F  add esp, 4
    // 00520E22  jmp 0x520e5f
    goto LAB_00520E5F;
LAB_00520E24:
    // 00520E24  cmp ebx, 0x64
    // 00520E27  jge 0x520e35
    if (((int)(ebx)) >= ((int)(0x64))) goto LAB_00520E35;
    // 00520E29  movsx eax, byte ptr [esi + 0x97d046]
    eax = ((*(signed char*)((char*)g_0097d046 + esi)));
    // 00520E30  add eax, 3
    eax = ((eax) + (3));
    // 00520E33  jmp 0x520e50
    goto LAB_00520E50;
LAB_00520E35:
    // 00520E35  movsx eax, byte ptr [esi + 0x97d046]
    eax = ((*(signed char*)((char*)g_0097d046 + esi)));
    // 00520E3C  cmp ebx, 0xc4
    // 00520E42  jge 0x520e4c
    if (((int)(ebx)) >= ((int)(0xc4))) goto LAB_00520E4C;
    // 00520E44  inc eax
    eax = ((eax) + 1);
    // 00520E45  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00520E46  sub eax, edx
    eax = ((eax) - (edx));
    // 00520E48  sar eax, 1
    eax = ((((int)(eax))) >> (1));
    // 00520E4A  jmp 0x520e59
    goto LAB_00520E59;
LAB_00520E4C:
    // 00520E4C  inc eax
    eax = ((eax) + 1);
    // 00520E4D  lea eax, [eax + eax*2]
    eax = (eax + eax*2);
LAB_00520E50:
    // 00520E50  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00520E51  and edx, 3
    edx = ((edx) & (3));
    // 00520E54  add eax, edx
    eax = ((eax) + (edx));
    // 00520E56  sar eax, 2
    eax = ((((int)(eax))) >> (2));
LAB_00520E59:
    // 00520E59  mov byte ptr [esi + 0x97d046], al
    (*(unsigned char*)((char*)g_0097d046 + esi)) = ((eax & 0xff));
LAB_00520E5F:
    // 00520E5F  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00520E62  add eax, 4
    eax = ((eax) + (4));
    // 00520E65  cmp eax, 0x144
    // 00520E6A  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 00520E6D  jl 0x520bec
    if (((int)(eax)) < ((int)(0x144))) goto LAB_00520BEC;
    // 00520E73  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00520E76  test eax, eax
    // 00520E78  je 0x520e9c
    if ((((eax) & (eax))) == 0) goto LAB_00520E9C;
    // 00520E7A  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00520E80  mov edx, dword ptr [0x9a6808]
    edx = ((*(int*)((char*)g_009a6808)));
    // 00520E86  mov eax, dword ptr [0x9a6804]
    eax = ((*(int*)((char*)g_009a6804)));
    // 00520E8B  push ecx
    // 00520E8C  push edx
    // 00520E8D  push eax
    // 00520E8E  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00520E93  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ecx);
    // 00520E98  test eax, eax
    // 00520E9A  jne 0x520ea6
    if ((((eax) & (eax))) != 0) goto LAB_00520EA6;
LAB_00520E9C:
    // 00520E9C  push 1
    // 00520E9E  call 0x46b190
    draw_map(1);
    // 00520EA3  add esp, 4
LAB_00520EA6:
    // 00520EA6  mov ecx, dword ptr [0x691b0c]
    ecx = ((*(int*)((char*)g_00691b0c)));
    // 00520EAC  push 0
    // 00520EAE  push 0x68b380
    // 00520EB3  push 0
    // 00520EB5  push 0x68b38c
    // 00520EBA  push ecx
    // 00520EBB  mov dword ptr [0x9a680c], 0xa
    (*(int*)((char*)g_009a680c)) = (0xa);
    // 00520EC5  call 0x48c0a0
    eax = popp((char*)(ecx), (const char*)(0x68b38c), 0, (const char*)(0x68b380), 0);
    // 00520ECA  add esp, 0x14
    return;
LAB_00520ED4:
    // 00520ED4  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00520ED9  test eax, eax
    // 00520EDB  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 00520EE1  cmp dword ptr [0x9a64d4], 0x4b
    // 00520EE8  jl 0x522116
    if (((int)((*(int*)((char*)g_009a64d4)))) < ((int)(0x4b))) goto LAB_00522116;
    // 00520EEE  mov edx, ebx
    edx = (ebx);
    // 00520EF0  shl edx, 6
    edx = ((edx) << (6));
    // 00520EF3  add edx, ebx
    edx = ((edx) + (ebx));
    // 00520EF5  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00520EF8  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 00520EFB  lea ebx, [ebx + ecx*2]
    ebx = (ebx + ecx*2);
    // 00520EFE  shl ebx, 2
    ebx = ((ebx) << (2));
    // 00520F01  mov dword ptr [ebp - 0x18], ebx
    var_18 = (ebx);
    // 00520F04  cmp dword ptr [ebx + 0x96da3c], 8
    // 00520F0B  jl 0x522116
    if (((int)((*(int*)((char*)g_0096da3c + ebx)))) < ((int)(8))) goto LAB_00522116;
    // 00520F11  test byte ptr [0x9a64c0], 0x10
    // 00520F18  jne 0x522116
    if (((((*(unsigned char*)((char*)g_009a64c0))) & (0x10))) != 0) goto LAB_00522116;
    // 00520F1E  mov edi, dword ptr [ebp - 0x10]
    edi = (var_10);
    // 00520F21  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 00520F24  push edi
    // 00520F25  push esi
    // 00520F26  call 0x5c5bd0
    eax = near_landmark(esi, edi);
    // 00520F2B  add esp, 8
    // 00520F2E  test eax, eax
    // 00520F30  jne 0x522116
    if ((((eax) & (eax))) != 0) goto LAB_00522116;
    // 00520F36  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00520F3C  mov edx, ecx
    edx = (ecx);
    // 00520F3E  shl edx, 6
    edx = ((edx) << (6));
    // 00520F41  add edx, ecx
    edx = ((edx) + (ecx));
    // 00520F43  lea eax, [ecx + edx*2]
    eax = (ecx + edx*2);
    // 00520F46  lea edx, [ecx + eax*8]
    edx = (ecx + eax*8);
    // 00520F49  lea eax, [ecx + edx*2]
    eax = (ecx + edx*2);
    // 00520F4C  mov edx, dword ptr [eax*4 + 0x96c9e0]
    edx = ((*(int*)((char*)g_0096c9e0 + eax*4)));
    // 00520F53  test dh, 2
    // 00520F56  jne 0x520f84
    if ((((((edx >> 8) & 0xff)) & (2))) != 0) goto LAB_00520F84;
    // 00520F58  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00520F5D  mov dword ptr [ebp - 4], 0
    var_4 = (0);
    // 00520F64  imul eax, edi
    eax = ((eax) * (edi));
    // 00520F67  sar esi, 1
    esi = ((((int)(esi))) >> (1));
    // 00520F69  add eax, esi
    eax = ((eax) + (esi));
    // 00520F6B  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00520F6E  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 00520F71  mov edx, 1
    edx = (1);
    // 00520F76  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00520F78  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 00520F7E  test byte ptr [ecx + eax*4 + 4], dl
    // 00520F82  je 0x520f8b
    if (((((*(unsigned char*)((char*)(ecx) + eax*4 + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_00520F8B;
LAB_00520F84:
    // 00520F84  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_00520F8B:
    // 00520F8B  mov ebx, dword ptr [0x94988c]
    ebx = ((*(int*)((char*)g_0094988c)));
    // 00520F91  mov edi, dword ptr [0x949870]
    edi = ((*(int*)((char*)g_00949870)));
    // 00520F97  xor esi, esi
    esi = ((esi) ^ (esi));
    // 00520F99  and ebx, 1
    ebx = ((ebx) & (1));
    // 00520F9C  mov dword ptr [ebp - 0x1c], esi
    var_1c = (esi);
LAB_00520F9F:
    // 00520F9F  mov eax, dword ptr [esi + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + esi)));
    // 00520FA5  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 00520FA8  add eax, edx
    eax = ((eax) + (edx));
    // 00520FAA  test ebx, ebx
    // 00520FAC  jne 0x520fbd
    if ((((ebx) & (ebx))) != 0) goto LAB_00520FBD;
    // 00520FAE  test eax, eax
    // 00520FB0  jge 0x520fb7
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00520FB7;
    // 00520FB2  lea ecx, [eax + edi]
    ecx = (edi);
    // 00520FB5  jmp 0x520fbf
    goto LAB_00520FBF;
LAB_00520FB7:
    // 00520FB7  cmp eax, edi
    // 00520FB9  jl 0x520fbd
    if (((int)(eax)) < ((int)(edi))) goto LAB_00520FBD;
    // 00520FBB  sub eax, edi
    eax = ((eax) - (edi));
LAB_00520FBD:
    // 00520FBD  mov ecx, eax
    ecx = (eax);
LAB_00520FBF:
    // 00520FBF  mov edx, dword ptr [esi + 0x66f440]
    edx = ((*(int*)((char*)g_0066f440 + esi)));
    // 00520FC5  mov eax, dword ptr [ebp - 0x10]
    eax = (var_10);
    // 00520FC8  add edx, eax
    edx = ((edx) + (eax));
    // 00520FCA  js 0x52101a
    if (((((int)(edx))) - (((int)(0x0)))) < 0) goto LAB_0052101A;
    // 00520FCC  cmp edx, dword ptr [0x949874]
    // 00520FD2  jge 0x52101a
    if (((int)(edx)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_0052101A;
    // 00520FD4  test ecx, ecx
    // 00520FD6  jl 0x52101a
    if (((int)(((ecx) & (ecx)))) < 0) goto LAB_0052101A;
    // 00520FD8  cmp ecx, edi
    // 00520FDA  jge 0x52101a
    if (((int)(ecx)) >= ((int)(edi))) goto LAB_0052101A;
    // 00520FDC  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00520FE1  imul eax, edx
    eax = ((eax) * (edx));
    // 00520FE4  sar ecx, 1
    ecx = ((((int)(ecx))) >> (1));
    // 00520FE6  add eax, ecx
    eax = ((eax) + (ecx));
    // 00520FE8  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00520FEA  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00520FED  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 00520FF0  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 00520FF6  mov cl, byte ptr [edx + eax*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(edx) + eax*4))));
    // 00520FF9  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 00520FFC  cmp ecx, 0x60
    // 00520FFF  jge 0x52101a
    if (((int)(ecx)) >= ((int)(0x60))) goto LAB_0052101A;
    // 00521001  cmp esi, 0x64
    // 00521004  jl 0x522116
    if (((int)(esi)) < ((int)(0x64))) goto LAB_00522116;
    // 0052100A  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 0052100D  inc eax
    eax = ((eax) + 1);
    // 0052100E  cmp eax, 5
    // 00521011  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 00521014  jg 0x522116
    if (((int)(eax)) > ((int)(5))) goto LAB_00522116;
LAB_0052101A:
    // 0052101A  add esi, 4
    esi = ((esi) + (4));
    // 0052101D  cmp esi, 0xc4
    // 00521023  jl 0x520f9f
    if (((int)(esi)) < ((int)(0xc4))) goto LAB_00520F9F;
    // 00521029  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 0052102C  cmp dword ptr [eax + 0x96c9e4], 7
    // 00521033  jl 0x522116
    if (((int)((*(int*)((char*)g_0096c9e4 + eax)))) < ((int)(7))) goto LAB_00522116;
    // 00521039  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0052103B  mov dword ptr [ebp - 0x1c], ecx
    var_1c = (ecx);
    // 0052103E  jmp 0x521043
    goto LAB_00521043;
LAB_00521040:
    // 00521040  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
LAB_00521043:
    // 00521043  mov eax, dword ptr [ecx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ecx)));
    // 00521049  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 0052104C  mov dl, byte ptr [0x94988c]
    SETB0(edx, ((*(unsigned char*)((char*)g_0094988c))));
    // 00521052  add eax, esi
    eax = ((eax) + (esi));
    // 00521054  test dl, 1
    // 00521057  jne 0x521068
    if (((((edx & 0xff)) & (1))) != 0) goto LAB_00521068;
    // 00521059  test eax, eax
    // 0052105B  jge 0x521062
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00521062;
    // 0052105D  lea ebx, [eax + edi]
    ebx = (edi);
    // 00521060  jmp 0x52106a
    goto LAB_0052106A;
LAB_00521062:
    // 00521062  cmp eax, edi
    // 00521064  jl 0x521068
    if (((int)(eax)) < ((int)(edi))) goto LAB_00521068;
    // 00521066  sub eax, edi
    eax = ((eax) - (edi));
LAB_00521068:
    // 00521068  mov ebx, eax
    ebx = (eax);
LAB_0052106A:
    // 0052106A  mov eax, dword ptr [ecx + 0x66f440]
    eax = ((*(int*)((char*)g_0066f440 + ecx)));
    // 00521070  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00521073  add eax, ecx
    eax = ((eax) + (ecx));
    // 00521075  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00521078  js 0x521180
    if (((((int)(eax))) - (((int)(0x0)))) < 0) goto LAB_00521180;
    // 0052107E  mov ecx, dword ptr [0x949874]
    ecx = ((*(int*)((char*)g_00949874)));
    // 00521084  cmp eax, ecx
    // 00521086  jge 0x521180
    if (((int)(eax)) >= ((int)(ecx))) goto LAB_00521180;
    // 0052108C  test ebx, ebx
    // 0052108E  jl 0x521180
    if (((int)(((ebx) & (ebx)))) < 0) goto LAB_00521180;
    // 00521094  cmp ebx, edi
    // 00521096  jge 0x521180
    if (((int)(ebx)) >= ((int)(edi))) goto LAB_00521180;
    // 0052109C  push 0
    // 0052109E  mov esi, eax
    esi = (eax);
    // 005210A0  push 0x81e8887c
    // 005210A5  push esi
    // 005210A6  push ebx
    // 005210A7  call 0x591d60
    bit_set(ebx, esi, 0x81e8887c, 0);
    // 005210AC  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005210B2  push edx
    // 005210B3  push esi
    // 005210B4  push ebx
    // 005210B5  call 0x591e50
    synch_bit(ebx, esi, edx);
    // 005210BA  push esi
    // 005210BB  push ebx
    // 005210BC  call 0x4e3a50
    eax = base_at(ebx, esi);
    // 005210C1  mov esi, eax
    esi = (eax);
    // 005210C3  add esp, 0x24
    // 005210C6  test esi, esi
    // 005210C8  jl 0x521129
    if (((int)(((esi) & (esi)))) < 0) goto LAB_00521129;
    // 005210CA  lea eax, [esi + esi*8]
    eax = (esi + esi*8);
    // 005210CD  mov edi, dword ptr [0x939284]
    edi = ((*(int*)((char*)g_00939284)));
    // 005210D3  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005210D5  lea ecx, [esi + eax*2]
    ecx = (esi + eax*2);
    // 005210D8  lea eax, [esi + ecx*4]
    eax = (esi + ecx*4);
    // 005210DB  shl eax, 2
    eax = ((eax) << (2));
    // 005210DE  mov dl, byte ptr [eax + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax))));
    // 005210E4  cmp edx, edi
    // 005210E6  jne 0x521120
    if ((edx) != (edi)) goto LAB_00521120;
    // 005210E8  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005210EB  lea edx, [ecx + ecx*8]
    edx = (ecx + ecx*8);
    // 005210EE  lea edx, [ecx + edx*2]
    edx = (ecx + edx*2);
    // 005210F1  lea ecx, [ecx + edx*4]
    ecx = (ecx + edx*4);
    // 005210F4  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005210F6  mov dl, byte ptr [ecx*4 + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + ecx*4))));
    // 005210FD  cmp edx, edi
    // 005210FF  je 0x521120
    if ((edx) == (edi)) goto LAB_00521120;
    // 00521101  push -1
    // 00521103  lea eax, [eax + 0x97d053]
    eax = (eax + 0x97d053);
    // 00521109  push -1
    // 0052110B  push eax
    // 0052110C  push 0
    // 0052110E  mov dword ptr [ebp - 8], esi
    var_8 = (esi);
    // 00521111  call 0x625ec0
    eax = parse_says(0, (char*)(eax), -1, -1);
    // 00521116  add esp, 0x10
    // 00521119  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_00521120:
    // 00521120  push esi
    // 00521121  call 0x4e5250
    base_kill(esi);
    // 00521126  add esp, 4
LAB_00521129:
    // 00521129  mov ecx, dword ptr [0x9a64c8]
    ecx = ((*(int*)((char*)g_009a64c8)));
    // 0052112F  lea esi, [ecx - 1]
    esi = (ecx + (-0x1));
    // 00521132  test esi, esi
    // 00521134  jl 0x52117a
    if (((int)(((esi) & (esi)))) < 0) goto LAB_0052117A;
    // 00521136  lea edx, [esi + esi*2]
    edx = (esi + esi*2);
    // 00521139  lea eax, [esi + edx*4]
    eax = (esi + edx*4);
    // 0052113C  lea edi, [eax*4 + 0x95282a]
    edi = (eax*4 + 0x95282a);
LAB_00521143:
    // 00521143  movsx ecx, word ptr [edi - 2]
    ecx = ((*(short*)((char*)(edi) + (-0x2))));
    // 00521147  cmp ecx, ebx
    // 00521149  jne 0x521172
    if ((ecx) != (ebx)) goto LAB_00521172;
    // 0052114B  movsx edx, word ptr [edi]
    edx = ((*(short*)((char*)(edi))));
    // 0052114E  cmp edx, dword ptr [ebp + 8]
    // 00521151  jne 0x521172
    if ((edx) != (a1)) goto LAB_00521172;
    // 00521153  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00521159  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0052115B  mov al, byte ptr [edi + 0xc]
    SETB0(eax, ((*(unsigned char*)((char*)(edi) + 0xc))));
    // 0052115E  cmp eax, ecx
    // 00521160  jne 0x521169
    if ((eax) != (ecx)) goto LAB_00521169;
    // 00521162  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_00521169:
    // 00521169  push esi
    // 0052116A  call 0x5c08c0
    veh_kill(esi);
    // 0052116F  add esp, 4
LAB_00521172:
    // 00521172  dec esi
    esi = ((esi) - 1);
    // 00521173  sub edi, 0x34
    edi = ((edi) - (0x34));
    // 00521176  test esi, esi
    // 00521178  jge 0x521143
    if (((int)(((esi) & (esi)))) >= 0) goto LAB_00521143;
LAB_0052117A:
    // 0052117A  mov edi, dword ptr [0x949870]
    edi = ((*(int*)((char*)g_00949870)));
LAB_00521180:
    // 00521180  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00521183  add eax, 4
    eax = ((eax) + (4));
    // 00521186  cmp eax, 0xc4
    // 0052118B  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 0052118E  jl 0x521040
    if (((int)(eax)) < ((int)(0xc4))) goto LAB_00521040;
    // 00521194  mov edi, dword ptr [ebp - 0x10]
    edi = (var_10);
    // 00521197  mov esi, dword ptr [ebp - 0x14]
    esi = (var_14);
    // 0052119A  push edi
    // 0052119B  push esi
    // 0052119C  call 0x5c5c70
    world_crater(esi, edi);
    // 005211A1  add esp, 8
    // 005211A4  call 0x5c5a30
    world_climate();
    // 005211A9  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005211AC  test eax, eax
    // 005211AE  je 0x5211c7
    if ((((eax) & (eax))) == 0) goto LAB_005211C7;
    // 005211B0  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005211B6  push ecx
    // 005211B7  push edi
    // 005211B8  push esi
    // 005211B9  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005211BE  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(esi, edi, ecx);
    // 005211C3  test eax, eax
    // 005211C5  jne 0x5211d1
    if ((((eax) & (eax))) != 0) goto LAB_005211D1;
LAB_005211C7:
    // 005211C7  push 1
    // 005211C9  call 0x46b190
    draw_map(1);
    // 005211CE  add esp, 4
LAB_005211D1:
    // 005211D1  mov edx, dword ptr [0x691b0c]
    edx = ((*(int*)((char*)g_00691b0c)));
    // 005211D7  push 0
    // 005211D9  push 0x68b39c
    // 005211DE  push 0
    // 005211E0  push 0x68b3a8
    // 005211E5  push edx
    // 005211E6  mov dword ptr [0x9a680c], 0xa
    (*(int*)((char*)g_009a680c)) = (0xa);
    // 005211F0  call 0x48c0a0
    eax = popp((char*)(edx), (const char*)(0x68b3a8), 0, (const char*)(0x68b39c), 0);
    // 005211F5  add esp, 0x14
    return;
LAB_005211FF:
    // 005211FF  mov eax, dword ptr [0x9a4d2c]
    eax = ((*(int*)((char*)g_009a4d2c)));
    // 00521204  push ebx
    // 00521205  push eax
    // 00521206  call 0x5b9f20
    eax = has_tech(eax, ebx);
    // 0052120B  add esp, 8
    // 0052120E  test eax, eax
    // 00521210  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 00521216  lea ecx, [ebp - 0x20]
    ecx = ((int)&var_20);
    // 00521219  lea edx, [ebp + 8]
    edx = ((int)&a1);
    // 0052121C  push ecx
    // 0052121D  xor esi, esi
    esi = ((esi) ^ (esi));
    // 0052121F  push edx
    // 00521220  push 9
    // 00521222  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 00521225  call 0x50ba00
    bitmask(9, (int*)(edx), (int*)(ecx));
    // 0052122A  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0052122D  add esp, 0xc
    // 00521230  mov cl, byte ptr [edi + eax + 0x97d0cc]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d0cc + eax))));
    // 00521237  mov al, byte ptr [ebp - 0x20]
    SETB0(eax, ((var_20 & 0xff)));
    // 0052123A  and cl, al
    SETB0(ecx, (((ecx & 0xff)) & ((eax & 0xff))));
    // 0052123C..00521240  neg8/sbb/neg -> (cl!=0)?1:0
    ecx = ((ecx & 0xff)) != 0 ? 1 : 0;
    // 00521242  mov dword ptr [ebp - 0x18], ecx
    var_18 = (ecx);
    // 00521245  jne 0x52127a
    if ((ecx) != (0x0)) goto LAB_0052127A;
    // 00521247  mov edx, ebx
    edx = (ebx);
    // 00521249  shl edx, 6
    edx = ((edx) << (6));
    // 0052124C  add edx, ebx
    edx = ((edx) + (ebx));
    // 0052124E  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00521251  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 00521254  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00521256  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00521259  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 0052125F  cmp ecx, 0x96
    // 00521265  mov ecx, dword ptr [edx*4 + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + edx*4)));
    // 0052126C  setge al
    SETB0(eax, ((((int)(ecx)) >= ((int)(0x96))) ? 1 : 0));
    // 0052126F  add eax, 4
    eax = ((eax) + (4));
    // 00521272  cmp ecx, eax
    // 00521274  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
LAB_0052127A:
    // 0052127A  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 00521280  mov dword ptr [ebp - 0x1c], esi
    var_1c = (esi);
LAB_00521283:
    // 00521283  mov edx, dword ptr [ebp - 0x1c]
    edx = (var_1c);
    // 00521286  mov edi, dword ptr [ebp - 0x14]
    edi = (var_14);
    // 00521289  mov eax, dword ptr [edx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + edx)));
    // 0052128F  add eax, edi
    eax = ((eax) + (edi));
    // 00521291  test byte ptr [0x94988c], 1
    // 00521298  jne 0x5212a8
    if (((((*(unsigned char*)((char*)g_0094988c))) & (1))) != 0) goto LAB_005212A8;
    // 0052129A  test eax, eax
    // 0052129C  jge 0x5212a2
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005212A2;
    // 0052129E  add eax, ecx
    eax = ((eax) + (ecx));
    // 005212A0  jmp 0x5212a8
    goto LAB_005212A8;
LAB_005212A2:
    // 005212A2  cmp eax, ecx
    // 005212A4  jl 0x5212a8
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005212A8;
    // 005212A6  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_005212A8:
    // 005212A8  mov esi, dword ptr [edx + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + edx)));
    // 005212AE  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 005212B1  add esi, dword ptr [ebp - 0x10]
    esi = ((esi) + (var_10));
    // 005212B4  js 0x521450
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_00521450;
    // 005212BA  cmp esi, dword ptr [0x949874]
    // 005212C0  jge 0x521450
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00521450;
    // 005212C6  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005212C9  test eax, eax
    // 005212CB  jl 0x521450
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00521450;
    // 005212D1  cmp eax, ecx
    // 005212D3  jge 0x521450
    if (((int)(eax)) >= ((int)(ecx))) goto LAB_00521450;
    // 005212D9  mov edi, eax
    edi = (eax);
    // 005212DB  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005212E0  imul eax, esi
    eax = ((eax) * (esi));
    // 005212E3  sar edi, 1
    edi = ((((int)(edi))) >> (1));
    // 005212E5  add eax, edi
    eax = ((eax) + (edi));
    // 005212E7  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 005212EA  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 005212ED  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 005212F3  lea eax, [edx + eax*4]
    eax = (edx + eax*4);
    // 005212F6  xor edx, edx
    edx = ((edx) ^ (edx));
    // 005212F8  mov dl, byte ptr [eax]
    SETB0(edx, ((*(unsigned char*)((char*)(eax)))));
    // 005212FA  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 005212FD  cmp edx, 0x60
    // 00521300  jl 0x521450
    if (((int)(edx)) < ((int)(0x60))) goto LAB_00521450;
    // 00521306  test dword ptr [eax + 8], 0x208000
    // 0052130D  je 0x521450
    if (((((*(int*)((char*)(eax) + 0x8))) & (0x208000))) == 0) goto LAB_00521450;
    // 00521313  mov ecx, ebx
    ecx = (ebx);
    // 00521315  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00521318  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 0052131A  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0052131D  lea ecx, [ebx + edx*8]
    ecx = (ebx + edx*8);
    // 00521320  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00521323  mov ecx, dword ptr [edx*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx*4)));
    // 0052132A  test ch, 2
    // 0052132D  jne 0x521341
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_00521341;
    // 0052132F  mov edx, 1
    edx = (1);
    // 00521334  mov ecx, ebx
    ecx = (ebx);
    // 00521336  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521338  test byte ptr [eax + 4], dl
    // 0052133B  je 0x52144a
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_0052144A;
LAB_00521341:
    // 00521341  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521344  push 0
    // 00521346  push 0
    // 00521348  push esi
    // 00521349  push eax
    // 0052134A  push ebx
    // 0052134B  call 0x4e3ef0
    eax = whose_territory(ebx, eax, esi, 0, 0);
    // 00521350  add esp, 0x14
    // 00521353  test eax, eax
    // 00521355  jl 0x52135f
    if (((int)(((eax) & (eax)))) < 0) goto LAB_0052135F;
    // 00521357  cmp eax, ebx
    // 00521359  jne 0x52144a
    if ((eax) != (ebx)) goto LAB_0052144A;
LAB_0052135F:
    // 0052135F  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00521362  test eax, eax
    // 00521364  jne 0x521443
    if ((((eax) & (eax))) != 0) goto LAB_00521443;
    // 0052136A  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0052136D  test eax, eax
    // 0052136F  jne 0x5213b0
    if ((((eax) & (eax))) != 0) goto LAB_005213B0;
    // 00521371  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00521377  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 0052137A  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 0052137D  push ecx
    // 0052137E  push edx
    // 0052137F  push eax
    // 00521380  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00521385  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ecx);
    // 0052138A  push 0x10
    // 0052138C  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00521392  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521394  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 00521396  test cl, cl
    // 00521398  jne 0x5213b0
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005213B0;
    // 0052139A  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 0052139F  test eax, eax
    // 005213A1  jne 0x5213b0
    if ((((eax) & (eax))) != 0) goto LAB_005213B0;
    // 005213A3  push 0xc8
    // 005213A8  call 0x50dd90
    clock_wait(0xc8);
    // 005213AD  add esp, 4
LAB_005213B0:
    // 005213B0  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005213B3  push 0
    // 005213B5  push 0x288000
    // 005213BA  push esi
    // 005213BB  push edx
    // 005213BC  call 0x591d60
    bit_set(edx, esi, 0x288000, 0);
    // 005213C1  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005213C4  push ebx
    // 005213C5  push esi
    // 005213C6  push eax
    // 005213C7  call 0x591e50
    synch_bit(eax, esi, ebx);
    // 005213CC  add esp, 0x1c
    // 005213CF  mov dword ptr [ebp - 4], 7
    var_4 = (7);
LAB_005213D6:
    // 005213D6  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 005213DB  mov edx, dword ptr [0x939284]
    edx = ((*(int*)((char*)g_00939284)));
    // 005213E1  imul eax, esi
    eax = ((eax) * (esi));
    // 005213E4  add eax, edi
    eax = ((eax) + (edi));
    // 005213E6  add edx, eax
    edx = ((edx) + (eax));
    // 005213E8  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 005213EB  lea eax, [edx + ecx*2]
    eax = (edx + ecx*2);
    // 005213EE  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 005213F4  lea ecx, [ecx + eax*4 + 0xc]
    ecx = (ecx + eax*4 + 0xc);
    // 005213F8  mov eax, dword ptr [ecx]
    eax = ((*(int*)((char*)(ecx))));
    // 005213FA  test eax, 0x288000
    // 005213FF  je 0x521408
    if ((((eax) & (0x288000))) == 0) goto LAB_00521408;
    // 00521401  and eax, 0xffd77fff
    eax = ((eax) & (0xffd77fff));
    // 00521406  mov dword ptr [ecx], eax
    (*(int*)((char*)(ecx))) = (eax);
LAB_00521408:
    // 00521408  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0052140B  dec eax
    eax = ((eax) - 1);
    // 0052140C  mov dword ptr [ebp - 4], eax
    var_4 = (eax);
    // 0052140F  jne 0x5213d6
    if ((eax) != (0x0)) goto LAB_005213D6;
    // 00521411  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00521414  push 2
    // 00521416  push esi
    // 00521417  push edx
    // 00521418  call 0x46af40
    draw_tile(edx, esi, 2);
    // 0052141D  add esp, 0xc
    // 00521420  push 0x10
    // 00521422  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00521428  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 0052142A  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 0052142C  test cl, cl
    // 0052142E  jne 0x521443
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00521443;
    // 00521430  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00521435  test eax, eax
    // 00521437  jne 0x521443
    if ((((eax) & (eax))) != 0) goto LAB_00521443;
    // 00521439  push 0x32
    // 0052143B  call 0x50dd90
    clock_wait(0x32);
    // 00521440  add esp, 4
LAB_00521443:
    // 00521443  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_0052144A:
    // 0052144A  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
LAB_00521450:
    // 00521450  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00521453  add eax, 4
    eax = ((eax) + (4));
    // 00521456  cmp eax, 0x54
    // 00521459  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 0052145C  jl 0x521283
    if (((int)(eax)) < ((int)(0x54))) goto LAB_00521283;
    // 00521462  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00521465  test eax, eax
    // 00521467  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0052146D  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00521470  test eax, eax
    // 00521472  je 0x521490
    if ((((eax) & (eax))) == 0) goto LAB_00521490;
    // 00521474  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00521477  mov ecx, dword ptr [eax + 0x97d074]
    ecx = ((*(int*)((char*)g_0097d074 + eax)));
    // 0052147D  or ch, 2
    SETB1(ecx, ((((ecx >> 8) & 0xff)) | (2)));
    // 00521480  mov dword ptr [eax + 0x97d074], ecx
    (*(int*)((char*)g_0097d074 + eax)) = (ecx);
    // 00521486  mov dword ptr [eax + 0x97d164], 0xa
    (*(int*)((char*)g_0097d164 + eax)) = (0xa);
LAB_00521490:
    // 00521490  cmp ebx, dword ptr [0x939284]
    // 00521496  jne 0x5214de
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_005214DE;
    // 00521498  mov esi, dword ptr [ebp - 0xc]
    esi = (var_c);
    // 0052149B  push ebx
    // 0052149C  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005214A1  movsx edx, word ptr [esi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + esi)));
    // 005214A8  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 005214AF  push edx
    // 005214B0  push eax
    // 005214B1  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ebx);
    // 005214B6  test eax, eax
    // 005214B8  jne 0x5214d6
    if ((((eax) & (eax))) != 0) goto LAB_005214D6;
    // 005214BA  movsx ecx, word ptr [esi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + esi)));
    // 005214C1  movsx edx, word ptr [esi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + esi)));
    // 005214C8  push 2
    // 005214CA  push 2
    // 005214CC  push ecx
    // 005214CD  push edx
    // 005214CE  call 0x46aef0
    draw_radius(edx, ecx, 2, 2);
    // 005214D3  add esp, 0x10
LAB_005214D6:
    // 005214D6  cmp ebx, dword ptr [0x939284]
    // 005214DC  je 0x5214eb
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_005214EB;
LAB_005214DE:
    // 005214DE  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 005214E3  test eax, eax
    // 005214E5  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_005214EB:
    // 005214EB  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 005214EE  test eax, eax
    // 005214F0  mov eax, 0x68b3c0
    eax = (0x68b3c0);
    // 005214F5  jne 0x5214fc
    if ((((eax) & (eax))) != 0) goto LAB_005214FC;
    // 005214F7  mov eax, 0x68b3c8
    eax = (0x68b3c8);
LAB_005214FC:
    // 005214FC  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 005214FF  push ecx
    // 00521500  push 0x68b3b0
    // 00521505  push eax
    // 00521506  call 0x405140
    eax = POP2((const char*)(eax), (const char*)(0x68b3b0), ecx);
    // 0052150B  add esp, 0xc
    return;
LAB_00521515:
    // 00521515  mov edx, dword ptr [0x9a4ccc]
    edx = ((*(int*)((char*)g_009a4ccc)));
    // 0052151B  push ebx
    // 0052151C  push edx
    // 0052151D  call 0x5b9f20
    eax = has_tech(edx, ebx);
    // 00521522  add esp, 8
    // 00521525  test eax, eax
    // 00521527  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0052152D  lea eax, [ebp - 0x20]
    eax = ((int)&var_20);
    // 00521530  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00521533  push eax
    // 00521534  xor esi, esi
    esi = ((esi) ^ (esi));
    // 00521536  push ecx
    // 00521537  push 7
    // 00521539  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 0052153C  call 0x50ba00
    bitmask(7, (int*)(ecx), (int*)(eax));
    // 00521541  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00521544  mov cl, byte ptr [ebp - 0x20]
    SETB0(ecx, ((var_20 & 0xff)));
    // 00521547  add esp, 0xc
    // 0052154A  mov al, byte ptr [edi + edx + 0x97d0cc]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d0cc + edx))));
    // 00521551  and al, cl
    SETB0(eax, (((eax & 0xff)) & ((ecx & 0xff))));
    // 00521553..00521557  neg8/sbb/neg -> (al!=0)?1:0
    eax = ((eax & 0xff)) != 0 ? 1 : 0;
    // 00521559  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 0052155C  jne 0x5215e6
    if ((eax) != (0x0)) goto LAB_005215E6;
    // 00521562  mov eax, dword ptr [0x9a65a4]
    eax = ((*(int*)((char*)g_009a65a4)));
    // 00521567  cmp eax, esi
    // 00521569  jl 0x521588
    if (((int)(eax)) < ((int)(esi))) goto LAB_00521588;
    // 0052156B  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 0052156E  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00521571  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521573  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00521576  mov cl, byte ptr [eax*4 + 0x97d044]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0097d044 + eax*4))));
    // 0052157D  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0052157F  cmp ecx, ebx
    // 00521581  sete al
    SETB0(eax, (((ecx) == (ebx)) ? 1 : 0));
    // 00521584  cmp eax, esi
    // 00521586  jne 0x5215e6
    if ((eax) != (esi)) goto LAB_005215E6;
LAB_00521588:
    // 00521588  mov edx, ebx
    edx = (ebx);
    // 0052158A  mov dword ptr [ebp - 0x1c], esi
    var_1c = (esi);
    // 0052158D  shl edx, 6
    edx = ((edx) << (6));
    // 00521590  add edx, ebx
    edx = ((edx) + (ebx));
    // 00521592  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00521595  lea ecx, [ebx + eax*8]
    ecx = (ebx + eax*8);
    // 00521598  xor eax, eax
    eax = ((eax) ^ (eax));
    // 0052159A  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0052159D  mov ecx, dword ptr [0x9a64d4]
    ecx = ((*(int*)((char*)g_009a64d4)));
    // 005215A3  cmp ecx, 0x96
    // 005215A9  mov ecx, dword ptr [edx*4 + 0x96c9e4]
    ecx = ((*(int*)((char*)g_0096c9e4 + edx*4)));
    // 005215B0  setge al
    SETB0(eax, ((((int)(ecx)) >= ((int)(0x96))) ? 1 : 0));
    // 005215B3  add eax, 4
    eax = ((eax) + (4));
    // 005215B6  cmp ecx, eax
    // 005215B8  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
LAB_005215BE:
    // 005215BE  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 005215C4  mov dword ptr [ebp - 0x18], esi
    var_18 = (esi);
LAB_005215C7:
    // 005215C7  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 005215CA  mov edi, dword ptr [ebp - 0x14]
    edi = (var_14);
    // 005215CD  mov eax, dword ptr [edx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + edx)));
    // 005215D3  add eax, edi
    eax = ((eax) + (edi));
    // 005215D5  test byte ptr [0x94988c], 1
    // 005215DC  jne 0x5215f5
    if (((((*(unsigned char*)((char*)g_0094988c))) & (1))) != 0) goto LAB_005215F5;
    // 005215DE  test eax, eax
    // 005215E0  jge 0x5215ef
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_005215EF;
    // 005215E2  add eax, ecx
    eax = ((eax) + (ecx));
    // 005215E4  jmp 0x5215f5
    goto LAB_005215F5;
LAB_005215E6:
    // 005215E6  mov dword ptr [ebp - 0x1c], 1
    var_1c = (1);
    // 005215ED  jmp 0x5215be
    goto LAB_005215BE;
LAB_005215EF:
    // 005215EF  cmp eax, ecx
    // 005215F1  jl 0x5215f5
    if (((int)(eax)) < ((int)(ecx))) goto LAB_005215F5;
    // 005215F3  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_005215F5:
    // 005215F5  mov esi, dword ptr [edx + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + edx)));
    // 005215FB  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 005215FE  add esi, dword ptr [ebp - 0x10]
    esi = ((esi) + (var_10));
    // 00521601  js 0x521785
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_00521785;
    // 00521607  cmp esi, dword ptr [0x949874]
    // 0052160D  jge 0x521785
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00521785;
    // 00521613  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521616  test eax, eax
    // 00521618  jl 0x521785
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00521785;
    // 0052161E  cmp eax, ecx
    // 00521620  jge 0x521785
    if (((int)(eax)) >= ((int)(ecx))) goto LAB_00521785;
    // 00521626  mov edi, eax
    edi = (eax);
    // 00521628  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 0052162D  imul eax, esi
    eax = ((eax) * (esi));
    // 00521630  sar edi, 1
    edi = ((((int)(edi))) >> (1));
    // 00521632  add eax, edi
    eax = ((eax) + (edi));
    // 00521634  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00521637  lea eax, [eax + edx*2]
    eax = (eax + edx*2);
    // 0052163A  mov edx, dword ptr [0x94a30c]
    edx = ((*(int*)((char*)g_0094a30c)));
    // 00521640  lea eax, [edx + eax*4]
    eax = (edx + eax*4);
    // 00521643  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00521645  mov dl, byte ptr [eax]
    SETB0(edx, ((*(unsigned char*)((char*)(eax)))));
    // 00521647  and edx, 0xffffffe0
    edx = ((edx) & (0xffffffe0));
    // 0052164A  cmp edx, 0x60
    // 0052164D  jl 0x521785
    if (((int)(edx)) < ((int)(0x60))) goto LAB_00521785;
    // 00521653  test byte ptr [eax + 8], 0x10
    // 00521657  je 0x521785
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (0x10))) == 0) goto LAB_00521785;
    // 0052165D  mov ecx, ebx
    ecx = (ebx);
    // 0052165F  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00521662  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 00521664  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00521667  lea ecx, [ebx + edx*8]
    ecx = (ebx + edx*8);
    // 0052166A  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 0052166D  mov ecx, dword ptr [edx*4 + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx*4)));
    // 00521674  test ch, 2
    // 00521677  jne 0x52168b
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_0052168B;
    // 00521679  mov edx, 1
    edx = (1);
    // 0052167E  mov ecx, ebx
    ecx = (ebx);
    // 00521680  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521682  test byte ptr [eax + 4], dl
    // 00521685  je 0x52177f
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_0052177F;
LAB_0052168B:
    // 0052168B  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0052168E  push 0
    // 00521690  push 0
    // 00521692  push esi
    // 00521693  push eax
    // 00521694  push ebx
    // 00521695  call 0x4e3ef0
    eax = whose_territory(ebx, eax, esi, 0, 0);
    // 0052169A  add esp, 0x14
    // 0052169D  test eax, eax
    // 0052169F  jl 0x5216a9
    if (((int)(((eax) & (eax)))) < 0) goto LAB_005216A9;
    // 005216A1  cmp eax, ebx
    // 005216A3  jne 0x52177f
    if ((eax) != (ebx)) goto LAB_0052177F;
LAB_005216A9:
    // 005216A9  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 005216AC  test eax, eax
    // 005216AE  jne 0x521778
    if ((((eax) & (eax))) != 0) goto LAB_00521778;
    // 005216B4  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005216B7  test eax, eax
    // 005216B9  jne 0x5216fa
    if ((((eax) & (eax))) != 0) goto LAB_005216FA;
    // 005216BB  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005216C1  mov edx, dword ptr [ebp - 0x10]
    edx = (var_10);
    // 005216C4  mov eax, dword ptr [ebp - 0x14]
    eax = (var_14);
    // 005216C7  push ecx
    // 005216C8  push edx
    // 005216C9  push eax
    // 005216CA  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005216CF  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ecx);
    // 005216D4  push 0x10
    // 005216D6  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 005216DC  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005216DE  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 005216E0  test cl, cl
    // 005216E2  jne 0x5216fa
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005216FA;
    // 005216E4  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 005216E9  test eax, eax
    // 005216EB  jne 0x5216fa
    if ((((eax) & (eax))) != 0) goto LAB_005216FA;
    // 005216ED  push 0xc8
    // 005216F2  call 0x50dd90
    clock_wait(0xc8);
    // 005216F7  add esp, 4
LAB_005216FA:
    // 005216FA  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005216FD  push 0
    // 005216FF  push 0x10
    // 00521701  push esi
    // 00521702  push edx
    // 00521703  call 0x591d60
    bit_set(edx, esi, 0x10, 0);
    // 00521708  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 0052170B  push ebx
    // 0052170C  push esi
    // 0052170D  push eax
    // 0052170E  call 0x591e50
    synch_bit(eax, esi, ebx);
    // 00521713  add esp, 0x1c
    // 00521716  mov edx, 1
    edx = (1);
LAB_0052171B:
    // 0052171B  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521720  imul eax, esi
    eax = ((eax) * (esi));
    // 00521723  add eax, edi
    eax = ((eax) + (edi));
    // 00521725  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00521728  add eax, edx
    eax = ((eax) + (edx));
    // 0052172A  lea ecx, [eax + ecx*2]
    ecx = (eax + ecx*2);
    // 0052172D  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 00521732  lea ecx, [eax + ecx*4 + 0xc]
    ecx = (eax + ecx*4 + 0xc);
    // 00521736  mov eax, dword ptr [ecx]
    eax = ((*(int*)((char*)(ecx))));
    // 00521738  test al, 0x10
    // 0052173A  je 0x521740
    if (((((eax & 0xff)) & (0x10))) == 0) goto LAB_00521740;
    // 0052173C  and al, 0xef
    SETB0(eax, (((eax & 0xff)) & (0xef)));
    // 0052173E  mov dword ptr [ecx], eax
    (*(int*)((char*)(ecx))) = (eax);
LAB_00521740:
    // 00521740  inc edx
    edx = ((edx) + 1);
    // 00521741  cmp edx, 8
    // 00521744  jl 0x52171b
    if (((int)(edx)) < ((int)(8))) goto LAB_0052171B;
    // 00521746  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00521749  push 2
    // 0052174B  push esi
    // 0052174C  push ecx
    // 0052174D  call 0x46af40
    draw_tile(ecx, esi, 2);
    // 00521752  add esp, 0xc
    // 00521755  push 0x10
    // 00521757  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 0052175D  xor edx, edx
    edx = ((edx) ^ (edx));
    // 0052175F  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 00521761  test dl, dl
    // 00521763  jne 0x521778
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_00521778;
    // 00521765  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 0052176A  test eax, eax
    // 0052176C  jne 0x521778
    if ((((eax) & (eax))) != 0) goto LAB_00521778;
    // 0052176E  push 0x32
    // 00521770  call 0x50dd90
    clock_wait(0x32);
    // 00521775  add esp, 4
LAB_00521778:
    // 00521778  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_0052177F:
    // 0052177F  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
LAB_00521785:
    // 00521785  mov eax, dword ptr [ebp - 0x18]
    eax = (var_18);
    // 00521788  add eax, 4
    eax = ((eax) + (4));
    // 0052178B  cmp eax, 0x54
    // 0052178E  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 00521791  jl 0x5215c7
    if (((int)(eax)) < ((int)(0x54))) goto LAB_005215C7;
    // 00521797  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 0052179A  test eax, eax
    // 0052179C  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 005217A2  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 005217A5  test eax, eax
    // 005217A7  je 0x5217c1
    if ((((eax) & (eax))) == 0) goto LAB_005217C1;
    // 005217A9  mov eax, ebx
    eax = (ebx);
    // 005217AB  shl eax, 6
    eax = ((eax) << (6));
    // 005217AE  add eax, ebx
    eax = ((eax) + (ebx));
    // 005217B0  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 005217B3  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 005217B6  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 005217B9  add dword ptr [eax*4 + 0x96cc00], 0x32
    (*(int*)((char*)g_0096cc00 + eax*4)) = (((*(int*)((char*)g_0096cc00 + eax*4))) + (0x32));
LAB_005217C1:
    // 005217C1  cmp ebx, dword ptr [0x939284]
    // 005217C7  jne 0x52180f
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_0052180F;
    // 005217C9  mov esi, dword ptr [ebp - 0xc]
    esi = (var_c);
    // 005217CC  push ebx
    // 005217CD  movsx eax, word ptr [esi + 0x97d042]
    eax = ((*(short*)((char*)g_0097d042 + esi)));
    // 005217D4  movsx ecx, word ptr [esi + 0x97d040]
    ecx = ((*(short*)((char*)g_0097d040 + esi)));
    // 005217DB  push eax
    // 005217DC  push ecx
    // 005217DD  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005217E2  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(ecx, eax, ebx);
    // 005217E7  test eax, eax
    // 005217E9  jne 0x521807
    if ((((eax) & (eax))) != 0) goto LAB_00521807;
    // 005217EB  movsx edx, word ptr [esi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + esi)));
    // 005217F2  movsx eax, word ptr [esi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + esi)));
    // 005217F9  push 2
    // 005217FB  push 2
    // 005217FD  push edx
    // 005217FE  push eax
    // 005217FF  call 0x46aef0
    draw_radius(eax, edx, 2, 2);
    // 00521804  add esp, 0x10
LAB_00521807:
    // 00521807  cmp ebx, dword ptr [0x939284]
    // 0052180D  je 0x52181c
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_0052181C;
LAB_0052180F:
    // 0052180F  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521814  test eax, eax
    // 00521816  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_0052181C:
    // 0052181C  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 0052181F  test eax, eax
    // 00521821  mov eax, 0x68b3e0
    eax = (0x68b3e0);
    // 00521826  jne 0x52182d
    if ((((eax) & (eax))) != 0) goto LAB_0052182D;
    // 00521828  mov eax, 0x68b3e8
    eax = (0x68b3e8);
LAB_0052182D:
    // 0052182D  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00521830  push ecx
    // 00521831  push 0x68b3d0
    // 00521836  push eax
    // 00521837  call 0x405140
    eax = POP2((const char*)(eax), (const char*)(0x68b3d0), ecx);
    // 0052183C  add esp, 0xc
    return;
LAB_00521846:
    // 00521846  mov edx, dword ptr [0x9a4cfc]
    edx = ((*(int*)((char*)g_009a4cfc)));
    // 0052184C  push ebx
    // 0052184D  push edx
    // 0052184E  call 0x5b9f20
    eax = has_tech(edx, ebx);
    // 00521853  add esp, 8
    // 00521856  test eax, eax
    // 00521858  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 0052185E  mov eax, ebx
    eax = (ebx);
    // 00521860  push ebx
    // 00521861  shl eax, 6
    eax = ((eax) << (6));
    // 00521864  add eax, ebx
    eax = ((eax) + (ebx));
    // 00521866  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 00521869  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 0052186C  lea esi, [ebx + edx*2]
    esi = (ebx + edx*2);
    // 0052186F  shl esi, 2
    esi = ((esi) << (2));
    // 00521872  call 0x5be6b0
    eax = tech_rate(ebx);
    // 00521877  mov ecx, dword ptr [esi + 0x96cda8]
    ecx = ((*(int*)((char*)g_0096cda8 + esi)));
    // 0052187D  add esp, 4
    // 00521880  cdq 
    edx = ((int)eax < 0) ? -1 : 0;
    // 00521881  and edx, 3
    edx = ((edx) & (3));
    // 00521884  add eax, edx
    eax = ((eax) + (edx));
    // 00521886  sar eax, 2
    eax = ((((int)(eax))) >> (2));
    // 00521889  cmp ecx, eax
    // 0052188B  jl 0x522116
    if (((int)(ecx)) < ((int)(eax))) goto LAB_00522116;
    // 00521891  lea eax, [ebp - 0x20]
    eax = ((int)&var_20);
    // 00521894  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 00521897  push eax
    // 00521898  push ecx
    // 00521899  push 8
    // 0052189B  call 0x50ba00
    bitmask(8, (int*)(ecx), (int*)(eax));
    // 005218A0  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005218A3  mov cl, byte ptr [ebp - 0x20]
    SETB0(ecx, ((var_20 & 0xff)));
    // 005218A6  add esp, 0xc
    // 005218A9  mov al, byte ptr [edi + edx + 0x97d0cc]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d0cc + edx))));
    // 005218B0  and al, cl
    SETB0(eax, (((eax & 0xff)) & ((ecx & 0xff))));
    // 005218B2..005218B6  neg8/sbb/neg -> (al!=0)?1:0
    eax = ((eax & 0xff)) != 0 ? 1 : 0;
    // 005218B8  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 005218BB  je 0x521922
    if ((eax) == (0x0)) goto LAB_00521922;
    // 005218BD  push ebx
    // 005218BE  call 0x5be6b0
    eax = tech_rate(ebx);
    // 005218C3  mov dword ptr [esi + 0x96cda8], eax
    (*(int*)((char*)g_0096cda8 + esi)) = (eax);
    // 005218C9  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 005218CE  add esp, 4
    // 005218D1  cmp ebx, eax
    // 005218D3  jne 0x5218f8
    if ((ebx) != (eax)) goto LAB_005218F8;
    // 005218D5  movsx ecx, word ptr [edi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + edi)));
    // 005218DC  movsx edx, word ptr [edi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + edi)));
    // 005218E3  push ebx
    // 005218E4  push ecx
    // 005218E5  push edx
    // 005218E6  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 005218EB  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, ebx);
    // 005218F0  cmp ebx, dword ptr [0x939284]
    // 005218F6  je 0x521905
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_00521905;
LAB_005218F8:
    // 005218F8  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 005218FD  test eax, eax
    // 005218FF  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_00521905:
    // 00521905  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00521908  push eax
    // 00521909  push 0x68b3f0
    // 0052190E  push 0x68b400
    // 00521913  call 0x405140
    eax = POP2((const char*)(0x68b400), (const char*)(0x68b3f0), eax);
    // 00521918  add esp, 0xc
    return;
LAB_00521922:
    // 00521922  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
    // 00521927  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521929  cmp eax, 0x96
    // 0052192E  mov eax, dword ptr [esi + 0x96c9e4]
    eax = ((*(int*)((char*)g_0096c9e4 + esi)));
    // 00521934  setge cl
    SETB0(ecx, ((((int)(eax)) >= ((int)(0x96))) ? 1 : 0));
    // 00521937  add ecx, 4
    ecx = ((ecx) + (4));
    // 0052193A  cmp eax, ecx
    // 0052193C  jl 0x522116
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00522116;
    // 00521942  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00521944  mov dword ptr [esi + 0x96cda8], eax
    (*(int*)((char*)g_0096cda8 + esi)) = (eax);
    // 0052194A  mov dword ptr [esi + 0x96cdb8], eax
    (*(int*)((char*)g_0096cdb8 + esi)) = (eax);
    // 00521950  cmp ebx, dword ptr [0x939284]
    // 00521956  jne 0x52197b
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_0052197B;
    // 00521958  movsx edx, word ptr [edi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + edi)));
    // 0052195F  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 00521966  push ebx
    // 00521967  push edx
    // 00521968  push eax
    // 00521969  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 0052196E  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ebx);
    // 00521973  cmp ebx, dword ptr [0x939284]
    // 00521979  je 0x521988
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_00521988;
LAB_0052197B:
    // 0052197B  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521980  test eax, eax
    // 00521982  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_00521988:
    // 00521988  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 0052198B  push ecx
    // 0052198C  push 0x68b40c
    // 00521991  push 0x68b41c
    // 00521996  call 0x405140
    eax = POP2((const char*)(0x68b41c), (const char*)(0x68b40c), ecx);
    // 0052199B  add esp, 0xc
    return;
LAB_005219A5:
    // 005219A5  mov edx, dword ptr [0x9a4bdc]
    edx = ((*(int*)((char*)g_009a4bdc)));
    // 005219AB  push ebx
    // 005219AC  push edx
    // 005219AD  call 0x5b9f20
    eax = has_tech(edx, ebx);
    // 005219B2  add esp, 8
    // 005219B5  test eax, eax
    // 005219B7  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 005219BD  lea eax, [ebp - 0x20]
    eax = ((int)&var_20);
    // 005219C0  lea ecx, [ebp + 8]
    ecx = ((int)&a1);
    // 005219C3  push eax
    // 005219C4  push ecx
    // 005219C5  push 2
    // 005219C7  call 0x50ba00
    bitmask(2, (int*)(ecx), (int*)(eax));
    // 005219CC  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 005219CF  mov cl, byte ptr [ebp - 0x20]
    SETB0(ecx, ((var_20 & 0xff)));
    // 005219D2  add esp, 0xc
    // 005219D5  mov al, byte ptr [edi + edx + 0x97d0cc]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d0cc + edx))));
    // 005219DC  and al, cl
    SETB0(eax, (((eax & 0xff)) & ((ecx & 0xff))));
    // 005219DE..005219E2  neg8/sbb/neg -> (al!=0)?1:0
    eax = ((eax & 0xff)) != 0 ? 1 : 0;
    // 005219E4  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 005219E7  je 0x521adb
    if ((eax) == (0x0)) goto LAB_00521ADB;
    // 005219ED  mov eax, dword ptr [edi + 0x97d110]
    eax = ((*(int*)((char*)g_0097d110 + edi)));
    // 005219F3  xor esi, esi
    esi = ((esi) ^ (esi));
    // 005219F5  test eax, eax
    // 005219F7  jle 0x522116
    if (((int)(((eax) & (eax)))) <= 0) goto LAB_00522116;
LAB_005219FD:
    // 005219FD  inc esi
    esi = ((esi) + 1);
    // 005219FE  cmp esi, 3
    // 00521A01  jge 0x521a2e
    if (((int)(esi)) >= ((int)(3))) goto LAB_00521A2E;
    // 00521A03  mov al, byte ptr [edi + 0x97d046]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d046 + edi))));
    // 00521A09  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00521A0C  inc al
    SETB0(eax, (((eax & 0xff)) + 1));
    // 00521A0E  push ecx
    // 00521A0F  mov byte ptr [edi + 0x97d046], al
    (*(unsigned char*)((char*)g_0097d046 + edi)) = ((eax & 0xff));
    // 00521A15  call 0x4e39d0
    set_base(ecx);
    // 00521A1A  push 1
    // 00521A1C  call 0x4ec3b0
    eax = base_compute(1);
    // 00521A21  mov eax, dword ptr [edi + 0x97d110]
    eax = ((*(int*)((char*)g_0097d110 + edi)));
    // 00521A27  add esp, 8
    // 00521A2A  test eax, eax
    // 00521A2C  jg 0x5219fd
    if (((int)(((eax) & (eax)))) > 0) goto LAB_005219FD;
LAB_00521A2E:
    // 00521A2E  test esi, esi
    // 00521A30  je 0x522116
    if ((((esi) & (esi))) == 0) goto LAB_00522116;
    // 00521A36  cmp ebx, dword ptr [0x939284]
    // 00521A3C  jne 0x521a7f
    if ((ebx) != ((*(int*)((char*)g_00939284)))) goto LAB_00521A7F;
    // 00521A3E  movsx edx, word ptr [edi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + edi)));
    // 00521A45  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 00521A4C  push ebx
    // 00521A4D  push edx
    // 00521A4E  push eax
    // 00521A4F  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00521A54  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ebx);
    // 00521A59  test eax, eax
    // 00521A5B  jne 0x521a77
    if ((((eax) & (eax))) != 0) goto LAB_00521A77;
    // 00521A5D  movsx ecx, word ptr [edi + 0x97d042]
    ecx = ((*(short*)((char*)g_0097d042 + edi)));
    // 00521A64  movsx edx, word ptr [edi + 0x97d040]
    edx = ((*(short*)((char*)g_0097d040 + edi)));
    // 00521A6B  push 2
    // 00521A6D  push ecx
    // 00521A6E  push edx
    // 00521A6F  call 0x46af40
    draw_tile(edx, ecx, 2);
    // 00521A74  add esp, 0xc
LAB_00521A77:
    // 00521A77  cmp ebx, dword ptr [0x939284]
    // 00521A7D  je 0x521a8c
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_00521A8C;
LAB_00521A7F:
    // 00521A7F  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521A84  test eax, eax
    // 00521A86  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_00521A8C:
    // 00521A8C  lea eax, [ebx + ebx*4]
    eax = (ebx + ebx*4);
    // 00521A8F  lea eax, [eax + eax*8]
    eax = (eax + eax*8);
    // 00521A92  shl eax, 3
    eax = ((eax) << (3));
    // 00521A95  sub eax, ebx
    eax = ((eax) - (ebx));
    // 00521A97  test byte ptr [eax*4 + 0x946f58], 0x80
    // 00521A9F  je 0x521abe
    if (((((*(unsigned char*)((char*)g_00946f58 + eax*4))) & (0x80))) == 0) goto LAB_00521ABE;
    // 00521AA1  mov ecx, dword ptr [ebp - 8]
    ecx = (var_8);
    // 00521AA4  push ecx
    // 00521AA5  push 0x68b428
    // 00521AAA  push 0x68b438
    // 00521AAF  call 0x405140
    eax = POP2((const char*)(0x68b438), (const char*)(0x68b428), ecx);
    // 00521AB4  add esp, 0xc
    return;
LAB_00521ABE:
    // 00521ABE  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00521AC1  push edx
    // 00521AC2  push 0x68b440
    // 00521AC7  push 0x68b44c
    // 00521ACC  call 0x405140
    eax = POP2((const char*)(0x68b44c), (const char*)(0x68b440), edx);
    // 00521AD1  add esp, 0xc
    return;
LAB_00521ADB:
    // 00521ADB  mov eax, ebx
    eax = (ebx);
    // 00521ADD  shl eax, 6
    eax = ((eax) << (6));
    // 00521AE0  add eax, ebx
    eax = ((eax) + (ebx));
    // 00521AE2  lea ecx, [ebx + eax*2]
    ecx = (ebx + eax*2);
    // 00521AE5  lea edx, [ebx + ecx*8]
    edx = (ebx + ecx*8);
    // 00521AE8  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521AEA  lea eax, [ebx + edx*2]
    eax = (ebx + edx*2);
    // 00521AED  mov edx, dword ptr [0x9a64d4]
    edx = ((*(int*)((char*)g_009a64d4)));
    // 00521AF3  cmp edx, 0x96
    // 00521AF9  mov edx, dword ptr [eax*4 + 0x96c9e4]
    edx = ((*(int*)((char*)g_0096c9e4 + eax*4)));
    // 00521B00  setge cl
    SETB0(ecx, ((((int)(edx)) >= ((int)(0x96))) ? 1 : 0));
    // 00521B03  add ecx, 4
    ecx = ((ecx) + (4));
    // 00521B06  cmp edx, ecx
    // 00521B08  jl 0x522116
    if (((int)(edx)) < ((int)(ecx))) goto LAB_00522116;
    // 00521B0E  mov al, byte ptr [edi + 0x97d047]
    SETB0(eax, ((*(unsigned char*)((char*)g_0097d047 + edi))));
    // 00521B14  test al, al
    // 00521B16  jne 0x522116
    if (((((eax & 0xff)) & ((eax & 0xff)))) != 0) goto LAB_00522116;
    // 00521B1C  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 00521B21  mov byte ptr [edi + 0x97d047], 5
    (*(unsigned char*)((char*)g_0097d047 + edi)) = (5);
    // 00521B28  cmp ebx, eax
    // 00521B2A  jne 0x521b4f
    if ((ebx) != (eax)) goto LAB_00521B4F;
    // 00521B2C  movsx edx, word ptr [edi + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + edi)));
    // 00521B33  movsx eax, word ptr [edi + 0x97d040]
    eax = ((*(short*)((char*)g_0097d040 + edi)));
    // 00521B3A  push ebx
    // 00521B3B  push edx
    // 00521B3C  push eax
    // 00521B3D  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00521B42  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(eax, edx, ebx);
    // 00521B47  cmp ebx, dword ptr [0x939284]
    // 00521B4D  je 0x521b5c
    if ((ebx) == ((*(int*)((char*)g_00939284)))) goto LAB_00521B5C;
LAB_00521B4F:
    // 00521B4F  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521B54  test eax, eax
    // 00521B56  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
LAB_00521B5C:
    // 00521B5C  lea eax, [ebx + ebx*4]
    eax = (ebx + ebx*4);
    // 00521B5F  lea ecx, [eax + eax*8]
    ecx = (eax + eax*8);
    // 00521B62  shl ecx, 3
    ecx = ((ecx) << (3));
    // 00521B65  sub ecx, ebx
    ecx = ((ecx) - (ebx));
    // 00521B67  test byte ptr [ecx*4 + 0x946f58], 0x80
    // 00521B6F  je 0x521b8e
    if (((((*(unsigned char*)((char*)g_00946f58 + ecx*4))) & (0x80))) == 0) goto LAB_00521B8E;
    // 00521B71  mov edx, dword ptr [ebp - 8]
    edx = (var_8);
    // 00521B74  push edx
    // 00521B75  push 0x68b454
    // 00521B7A  push 0x68b464
    // 00521B7F  call 0x405140
    eax = POP2((const char*)(0x68b464), (const char*)(0x68b454), edx);
    // 00521B84  add esp, 0xc
    return;
LAB_00521B8E:
    // 00521B8E  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00521B91  push eax
    // 00521B92  push 0x68b46c
    // 00521B97  push 0x68b478
    // 00521B9C  call 0x405140
    eax = POP2((const char*)(0x68b478), (const char*)(0x68b46c), eax);
    // 00521BA1  add esp, 0xc
    return;
LAB_00521BAB:
    // 00521BAB  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521BB0  xor edi, edi
    edi = ((edi) ^ (edi));
    // 00521BB2  cmp eax, edi
    // 00521BB4  jne 0x522116
    if ((eax) != (edi)) goto LAB_00522116;
    // 00521BBA  mov esi, dword ptr [0x9a64d4]
    esi = ((*(int*)((char*)g_009a64d4)));
    // 00521BC0  cmp esi, 0x4b
    // 00521BC3  jl 0x522116
    if (((int)(esi)) < ((int)(0x4b))) goto LAB_00522116;
    // 00521BC9  mov ecx, ebx
    ecx = (ebx);
    // 00521BCB  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00521BCE  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 00521BD0  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00521BD3  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521BD5  lea eax, [ebx + edx*8]
    eax = (ebx + edx*8);
    // 00521BD8  lea eax, [ebx + eax*2]
    eax = (ebx + eax*2);
    // 00521BDB  shl eax, 2
    eax = ((eax) << (2));
    // 00521BDE  cmp esi, 0x96
    // 00521BE4  mov dword ptr [ebp - 0x18], eax
    var_18 = (eax);
    // 00521BE7  mov edx, dword ptr [eax + 0x96c9e4]
    edx = ((*(int*)((char*)g_0096c9e4 + eax)));
    // 00521BED  setge cl
    SETB0(ecx, ((((int)(esi)) >= ((int)(0x96))) ? 1 : 0));
    // 00521BF0  add ecx, 4
    ecx = ((ecx) + (4));
    // 00521BF3  cmp edx, ecx
    // 00521BF5  jl 0x522116
    if (((int)(edx)) < ((int)(ecx))) goto LAB_00522116;
    // 00521BFB  mov dword ptr [ebp - 4], edi
    var_4 = (edi);
    // 00521BFE  mov dword ptr [ebp - 0x1c], edi
    var_1c = (edi);
LAB_00521C01:
    // 00521C01  mov edx, dword ptr [ebp - 0xc]
    edx = (var_c);
    // 00521C04  mov ecx, dword ptr [ebp - 0x1c]
    ecx = (var_1c);
    // 00521C07  movsx esi, word ptr [edx + 0x97d040]
    esi = ((*(short*)((char*)g_0097d040 + edx)));
    // 00521C0E  mov eax, dword ptr [ecx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + ecx)));
    // 00521C14  add eax, esi
    eax = ((eax) + (esi));
    // 00521C16  test byte ptr [0x94988c], 1
    // 00521C1D  jne 0x521c33
    if (((((*(unsigned char*)((char*)g_0094988c))) & (1))) != 0) goto LAB_00521C33;
    // 00521C1F  mov esi, dword ptr [0x949870]
    esi = ((*(int*)((char*)g_00949870)));
    // 00521C25  test eax, eax
    // 00521C27  jge 0x521c2d
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00521C2D;
    // 00521C29  add eax, esi
    eax = ((eax) + (esi));
    // 00521C2B  jmp 0x521c33
    goto LAB_00521C33;
LAB_00521C2D:
    // 00521C2D  cmp eax, esi
    // 00521C2F  jl 0x521c33
    if (((int)(eax)) < ((int)(esi))) goto LAB_00521C33;
    // 00521C31  sub eax, esi
    eax = ((eax) - (esi));
LAB_00521C33:
    // 00521C33  movsx edx, word ptr [edx + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + edx)));
    // 00521C3A  mov esi, dword ptr [ecx + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + ecx)));
    // 00521C40  mov dword ptr [ebp + 8], eax
    a1 = (eax);
    // 00521C43  add esi, edx
    esi = ((esi) + (edx));
    // 00521C45  js 0x521e03
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_00521E03;
    // 00521C4B  cmp esi, dword ptr [0x949874]
    // 00521C51  jge 0x521e03
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_00521E03;
    // 00521C57  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521C5A  test eax, eax
    // 00521C5C  jl 0x521e03
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00521E03;
    // 00521C62  mov eax, dword ptr [0x949870]
    eax = ((*(int*)((char*)g_00949870)));
    // 00521C67  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00521C6A  cmp ecx, eax
    // 00521C6C  jge 0x521e03
    if (((int)(ecx)) >= ((int)(eax))) goto LAB_00521E03;
    // 00521C72  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521C77  mov edi, ecx
    edi = (ecx);
    // 00521C79  imul eax, esi
    eax = ((eax) * (esi));
    // 00521C7C  sar edi, 1
    edi = ((((int)(edi))) >> (1));
    // 00521C7E  add eax, edi
    eax = ((eax) + (edi));
    // 00521C80  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00521C83  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00521C86  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 00521C8B  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521C8D  mov cl, byte ptr [eax + edx*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + edx*4))));
    // 00521C90  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00521C93  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 00521C96  cmp ecx, 0x60
    // 00521C99  jge 0x521e03
    if (((int)(ecx)) >= ((int)(0x60))) goto LAB_00521E03;
    // 00521C9F  mov ecx, dword ptr [eax + 8]
    ecx = ((*(int*)((char*)(eax) + 0x8)));
    // 00521CA2  test ch, 0x80
    // 00521CA5  je 0x521e03
    if ((((((ecx >> 8) & 0xff)) & (0x80))) == 0) goto LAB_00521E03;
    // 00521CAB  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 00521CAE  mov ecx, dword ptr [edx + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx)));
    // 00521CB4  test ch, 2
    // 00521CB7  jne 0x521ccb
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_00521CCB;
    // 00521CB9  mov edx, 1
    edx = (1);
    // 00521CBE  mov ecx, ebx
    ecx = (ebx);
    // 00521CC0  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521CC2  test byte ptr [eax + 4], dl
    // 00521CC5  je 0x521e03
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_00521E03;
LAB_00521CCB:
    // 00521CCB  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521CCE  push 0
    // 00521CD0  push 0
    // 00521CD2  push esi
    // 00521CD3  push eax
    // 00521CD4  push ebx
    // 00521CD5  call 0x4e3ef0
    eax = whose_territory(ebx, eax, esi, 0, 0);
    // 00521CDA  add esp, 0x14
    // 00521CDD  test eax, eax
    // 00521CDF  jl 0x521ce9
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00521CE9;
    // 00521CE1  cmp eax, ebx
    // 00521CE3  jne 0x521e03
    if ((eax) != (ebx)) goto LAB_00521E03;
LAB_00521CE9:
    // 00521CE9  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521CEE  imul eax, esi
    eax = ((eax) * (esi));
    // 00521CF1  add eax, edi
    eax = ((eax) + (edi));
    // 00521CF3  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00521CF6  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00521CF9  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 00521CFF  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00521D01  mov al, byte ptr [ecx + edx*4]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + edx*4))));
    // 00521D04  and al, 0xe0
    SETB0(eax, (((eax & 0xff)) & (0xe0)));
    // 00521D06  cmp eax, 0x60
    // 00521D09  jl 0x521e03
    if (((int)(eax)) < ((int)(0x60))) goto LAB_00521E03;
    // 00521D0F  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00521D15  cmp ebx, ecx
    // 00521D17  je 0x521d37
    if ((ebx) == (ecx)) goto LAB_00521D37;
    // 00521D19  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00521D1C  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00521D1E  mov dl, byte ptr [eax + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax))));
    // 00521D24  cmp edx, ecx
    // 00521D26  je 0x521d37
    if ((edx) == (ecx)) goto LAB_00521D37;
    // 00521D28  mov edx, 1
    edx = (1);
    // 00521D2D  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521D2F  test byte ptr [eax + 0x97d04a], dl
    // 00521D35  je 0x521d7e
    if (((((*(unsigned char*)((char*)g_0097d04a + eax))) & ((edx & 0xff)))) == 0) goto LAB_00521D7E;
LAB_00521D37:
    // 00521D37  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00521D3A  test eax, eax
    // 00521D3C  jne 0x521d77
    if ((((eax) & (eax))) != 0) goto LAB_00521D77;
    // 00521D3E  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 00521D41  push ecx
    // 00521D42  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00521D45  push ecx
    // 00521D46  push edx
    // 00521D47  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00521D4C  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, ecx);
    // 00521D51  push 0x10
    // 00521D53  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00521D59  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521D5B  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 00521D5D  test cl, cl
    // 00521D5F  jne 0x521d77
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00521D77;
    // 00521D61  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00521D66  test eax, eax
    // 00521D68  jne 0x521d77
    if ((((eax) & (eax))) != 0) goto LAB_00521D77;
    // 00521D6A  push 0xc8
    // 00521D6F  call 0x50dd90
    clock_wait(0xc8);
    // 00521D74  add esp, 4
LAB_00521D77:
    // 00521D77  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_00521D7E:
    // 00521D7E  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00521D81  push 0
    // 00521D83  push 0x8000
    // 00521D88  push esi
    // 00521D89  push edx
    // 00521D8A  call 0x591d60
    bit_set(edx, esi, 0x8000, 0);
    // 00521D8F  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521D92  push ebx
    // 00521D93  push esi
    // 00521D94  push eax
    // 00521D95  call 0x591e50
    synch_bit(eax, esi, ebx);
    // 00521D9A  add esp, 0x1c
    // 00521D9D  mov ecx, 1
    ecx = (1);
LAB_00521DA2:
    // 00521DA2  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521DA7  imul eax, esi
    eax = ((eax) * (esi));
    // 00521DAA  add eax, edi
    eax = ((eax) + (edi));
    // 00521DAC  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00521DAF  add eax, ecx
    eax = ((eax) + (ecx));
    // 00521DB1  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 00521DB4  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 00521DB9  lea eax, [eax + edx*4 + 0xc]
    eax = (eax + edx*4 + 0xc);
    // 00521DBD  mov edx, dword ptr [eax]
    edx = ((*(int*)((char*)(eax))));
    // 00521DBF  and dh, 0x7f
    SETB1(edx, ((((edx >> 8) & 0xff)) & (0x7f)));
    // 00521DC2  inc ecx
    ecx = ((ecx) + 1);
    // 00521DC3  cmp ecx, 8
    // 00521DC6  mov dword ptr [eax], edx
    (*(int*)((char*)(eax))) = (edx);
    // 00521DC8  jl 0x521da2
    if (((int)(ecx)) < ((int)(8))) goto LAB_00521DA2;
    // 00521DCA  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00521DCD  test eax, eax
    // 00521DCF  je 0x521e03
    if ((((eax) & (eax))) == 0) goto LAB_00521E03;
    // 00521DD1  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00521DD4  push 2
    // 00521DD6  push esi
    // 00521DD7  push ecx
    // 00521DD8  call 0x46af40
    draw_tile(ecx, esi, 2);
    // 00521DDD  add esp, 0xc
    // 00521DE0  push 0x10
    // 00521DE2  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00521DE8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00521DEA  mov dl, ah
    SETB0(edx, (((eax >> 8) & 0xff)));
    // 00521DEC  test dl, dl
    // 00521DEE  jne 0x521e03
    if (((((edx & 0xff)) & ((edx & 0xff)))) != 0) goto LAB_00521E03;
    // 00521DF0  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00521DF5  test eax, eax
    // 00521DF7  jne 0x521e03
    if ((((eax) & (eax))) != 0) goto LAB_00521E03;
    // 00521DF9  push 0x32
    // 00521DFB  call 0x50dd90
    clock_wait(0x32);
    // 00521E00  add esp, 4
LAB_00521E03:
    // 00521E03  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00521E06  add eax, 4
    eax = ((eax) + (4));
    // 00521E09  cmp eax, 0x54
    // 00521E0C  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 00521E0F  jl 0x521c01
    if (((int)(eax)) < ((int)(0x54))) goto LAB_00521C01;
    // 00521E15  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00521E18  test eax, eax
    // 00521E1A  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 00521E20  mov eax, dword ptr [ebp - 0x20]
    eax = (var_20);
    // 00521E23  push -1
    // 00521E25  push -1
    // 00521E27  push eax
    // 00521E28  push 0
    // 00521E2A  call 0x625ec0
    eax = parse_says(0, (char*)(eax), -1, -1);
    // 00521E2F  mov eax, dword ptr [0x939284]
    eax = ((*(int*)((char*)g_00939284)));
    // 00521E34  add esp, 0x10
    // 00521E37  cmp ebx, eax
    // 00521E39  jne 0x521e55
    if ((ebx) != (eax)) goto LAB_00521E55;
    // 00521E3B  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00521E3E  push eax
    // 00521E3F  push 0
    // 00521E41  push 0x68b480
    // 00521E46  call 0x405140
    eax = POP2((const char*)(0x68b480), (const char*)(0), eax);
    // 00521E4B  add esp, 0xc
    return;
LAB_00521E55:
    // 00521E55  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
    // 00521E58  push eax
    // 00521E59  push 0
    // 00521E5B  push 0x68b480
    // 00521E60  call 0x405140
    eax = POP2((const char*)(0x68b480), (const char*)(0), eax);
    // 00521E65  add esp, 0xc
    return;
LAB_00521E6F:
    // 00521E6F  mov eax, dword ptr [0x93a95c]
    eax = ((*(int*)((char*)g_0093a95c)));
    // 00521E74  xor esi, esi
    esi = ((esi) ^ (esi));
    // 00521E76  cmp eax, esi
    // 00521E78  jne 0x522116
    if ((eax) != (esi)) goto LAB_00522116;
    // 00521E7E  mov eax, dword ptr [0x9a64d4]
    eax = ((*(int*)((char*)g_009a64d4)));
    // 00521E83  cmp eax, 0x4b
    // 00521E86  jl 0x522116
    if (((int)(eax)) < ((int)(0x4b))) goto LAB_00522116;
    // 00521E8C  mov ecx, ebx
    ecx = (ebx);
    // 00521E8E  shl ecx, 6
    ecx = ((ecx) << (6));
    // 00521E91  add ecx, ebx
    ecx = ((ecx) + (ebx));
    // 00521E93  lea edx, [ebx + ecx*2]
    edx = (ebx + ecx*2);
    // 00521E96  lea ecx, [ebx + edx*8]
    ecx = (ebx + edx*8);
    // 00521E99  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00521E9B  lea ebx, [ebx + ecx*2]
    ebx = (ebx + ecx*2);
    // 00521E9E  shl ebx, 2
    ebx = ((ebx) << (2));
    // 00521EA1  cmp eax, 0x96
    // 00521EA6  mov dword ptr [ebp - 0x18], ebx
    var_18 = (ebx);
    // 00521EA9  mov eax, dword ptr [ebx + 0x96c9e4]
    eax = ((*(int*)((char*)g_0096c9e4 + ebx)));
    // 00521EAF  setge dl
    SETB0(edx, ((((int)(eax)) >= ((int)(0x96))) ? 1 : 0));
    // 00521EB2  add edx, 4
    edx = ((edx) + (4));
    // 00521EB5  cmp eax, edx
    // 00521EB7  jl 0x522116
    if (((int)(eax)) < ((int)(edx))) goto LAB_00522116;
    // 00521EBD  mov dword ptr [ebp - 4], esi
    var_4 = (esi);
    // 00521EC0  mov dword ptr [ebp - 0x1c], esi
    var_1c = (esi);
LAB_00521EC3:
    // 00521EC3  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00521EC6  mov edx, dword ptr [ebp - 0x1c]
    edx = (var_1c);
    // 00521EC9  movsx ecx, word ptr [eax + 0x97d040]
    ecx = ((*(short*)((char*)g_0097d040 + eax)));
    // 00521ED0  mov eax, dword ptr [edx + 0x66efbc]
    eax = ((*(int*)((char*)g_0066efbc + edx)));
    // 00521ED6  add eax, ecx
    eax = ((eax) + (ecx));
    // 00521ED8  mov cl, byte ptr [0x94988c]
    SETB0(ecx, ((*(unsigned char*)((char*)g_0094988c))));
    // 00521EDE  test cl, 1
    // 00521EE1  je 0x521eed
    if (((((ecx & 0xff)) & (1))) == 0) goto LAB_00521EED;
    // 00521EE3  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 00521EE9  mov ebx, eax
    ebx = (eax);
    // 00521EEB  jmp 0x521f04
    goto LAB_00521F04;
LAB_00521EED:
    // 00521EED  mov ecx, dword ptr [0x949870]
    ecx = ((*(int*)((char*)g_00949870)));
    // 00521EF3  test eax, eax
    // 00521EF5  jge 0x521efc
    if (((int)(((eax) & (eax)))) >= 0) goto LAB_00521EFC;
    // 00521EF7  lea ebx, [eax + ecx]
    ebx = (ecx);
    // 00521EFA  jmp 0x521f04
    goto LAB_00521F04;
LAB_00521EFC:
    // 00521EFC  cmp eax, ecx
    // 00521EFE  jl 0x521f02
    if (((int)(eax)) < ((int)(ecx))) goto LAB_00521F02;
    // 00521F00  sub eax, ecx
    eax = ((eax) - (ecx));
LAB_00521F02:
    // 00521F02  mov ebx, eax
    ebx = (eax);
LAB_00521F04:
    // 00521F04  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00521F07  movsx edx, word ptr [eax + 0x97d042]
    edx = ((*(short*)((char*)g_0097d042 + eax)));
    // 00521F0E  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 00521F11  mov esi, dword ptr [eax + 0x66f440]
    esi = ((*(int*)((char*)g_0066f440 + eax)));
    // 00521F17  add esi, edx
    esi = ((esi) + (edx));
    // 00521F19  js 0x5220c6
    if (((((int)(esi))) - (((int)(0x0)))) < 0) goto LAB_005220C6;
    // 00521F1F  cmp esi, dword ptr [0x949874]
    // 00521F25  jge 0x5220c6
    if (((int)(esi)) >= ((int)((*(int*)((char*)g_00949874))))) goto LAB_005220C6;
    // 00521F2B  test ebx, ebx
    // 00521F2D  jl 0x5220c6
    if (((int)(((ebx) & (ebx)))) < 0) goto LAB_005220C6;
    // 00521F33  cmp ebx, ecx
    // 00521F35  jge 0x5220c6
    if (((int)(ebx)) >= ((int)(ecx))) goto LAB_005220C6;
    // 00521F3B  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521F40  mov edi, ebx
    edi = (ebx);
    // 00521F42  imul eax, esi
    eax = ((eax) * (esi));
    // 00521F45  sar edi, 1
    edi = ((((int)(edi))) >> (1));
    // 00521F47  add eax, edi
    eax = ((eax) + (edi));
    // 00521F49  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00521F4C  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00521F4F  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 00521F54  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00521F56  mov cl, byte ptr [eax + edx*4]
    SETB0(ecx, ((*(unsigned char*)((char*)(eax) + edx*4))));
    // 00521F59  lea eax, [eax + edx*4]
    eax = (eax + edx*4);
    // 00521F5C  and ecx, 0xffffffe0
    ecx = ((ecx) & (0xffffffe0));
    // 00521F5F  cmp ecx, 0x60
    // 00521F62  jge 0x5220c6
    if (((int)(ecx)) >= ((int)(0x60))) goto LAB_005220C6;
    // 00521F68  test byte ptr [eax + 8], 0x10
    // 00521F6C  je 0x5220c6
    if (((((*(unsigned char*)((char*)(eax) + 0x8))) & (0x10))) == 0) goto LAB_005220C6;
    // 00521F72  mov edx, dword ptr [ebp - 0x18]
    edx = (var_18);
    // 00521F75  mov ecx, dword ptr [edx + 0x96c9e0]
    ecx = ((*(int*)((char*)g_0096c9e0 + edx)));
    // 00521F7B  test ch, 2
    // 00521F7E  jne 0x521f93
    if ((((((ecx >> 8) & 0xff)) & (2))) != 0) goto LAB_00521F93;
    // 00521F80  mov ecx, dword ptr [ebp + 8]
    ecx = (a1);
    // 00521F83  mov edx, 1
    edx = (1);
    // 00521F88  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521F8A  test byte ptr [eax + 4], dl
    // 00521F8D  je 0x5220c6
    if (((((*(unsigned char*)((char*)(eax) + 0x4))) & ((edx & 0xff)))) == 0) goto LAB_005220C6;
LAB_00521F93:
    // 00521F93  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521F96  push 0
    // 00521F98  push 0
    // 00521F9A  push esi
    // 00521F9B  push ebx
    // 00521F9C  push eax
    // 00521F9D  call 0x4e3ef0
    eax = whose_territory(eax, ebx, esi, 0, 0);
    // 00521FA2  add esp, 0x14
    // 00521FA5  test eax, eax
    // 00521FA7  jl 0x521fb2
    if (((int)(((eax) & (eax)))) < 0) goto LAB_00521FB2;
    // 00521FA9  cmp eax, dword ptr [ebp + 8]
    // 00521FAC  jne 0x5220c6
    if ((eax) != (a1)) goto LAB_005220C6;
LAB_00521FB2:
    // 00521FB2  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 00521FB7  imul eax, esi
    eax = ((eax) * (esi));
    // 00521FBA  add eax, edi
    eax = ((eax) + (edi));
    // 00521FBC  lea ecx, [eax + eax*4]
    ecx = (eax + eax*4);
    // 00521FBF  lea edx, [eax + ecx*2]
    edx = (eax + ecx*2);
    // 00521FC2  mov ecx, dword ptr [0x94a30c]
    ecx = ((*(int*)((char*)g_0094a30c)));
    // 00521FC8  xor eax, eax
    eax = ((eax) ^ (eax));
    // 00521FCA  mov al, byte ptr [ecx + edx*4]
    SETB0(eax, ((*(unsigned char*)((char*)(ecx) + edx*4))));
    // 00521FCD  and al, 0xe0
    SETB0(eax, (((eax & 0xff)) & (0xe0)));
    // 00521FCF  cmp eax, 0x60
    // 00521FD2  jl 0x5220c6
    if (((int)(eax)) < ((int)(0x60))) goto LAB_005220C6;
    // 00521FD8  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 00521FDE  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 00521FE1  cmp eax, ecx
    // 00521FE3  je 0x522003
    if ((eax) == (ecx)) goto LAB_00522003;
    // 00521FE5  mov eax, dword ptr [ebp - 0xc]
    eax = (var_c);
    // 00521FE8  xor edx, edx
    edx = ((edx) ^ (edx));
    // 00521FEA  mov dl, byte ptr [eax + 0x97d044]
    SETB0(edx, ((*(unsigned char*)((char*)g_0097d044 + eax))));
    // 00521FF0  cmp edx, ecx
    // 00521FF2  je 0x522003
    if ((edx) == (ecx)) goto LAB_00522003;
    // 00521FF4  mov edx, 1
    edx = (1);
    // 00521FF9  shl edx, cl
    edx = ((edx) << ((ecx & 0xff)));
    // 00521FFB  test byte ptr [eax + 0x97d04a], dl
    // 00522001  je 0x52204a
    if (((((*(unsigned char*)((char*)g_0097d04a + eax))) & ((edx & 0xff)))) == 0) goto LAB_0052204A;
LAB_00522003:
    // 00522003  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00522006  test eax, eax
    // 00522008  jne 0x522043
    if ((((eax) & (eax))) != 0) goto LAB_00522043;
    // 0052200A  mov edx, dword ptr [ebp - 0x14]
    edx = (var_14);
    // 0052200D  push ecx
    // 0052200E  mov ecx, dword ptr [ebp - 0x10]
    ecx = (var_10);
    // 00522011  push ecx
    // 00522012  push edx
    // 00522013  mov ecx, 0x9156b0
    ecx = (0x9156b0);
    // 00522018  call 0x5108a0
    reinterpret_cast<Console*>(ecx)->focus(edx, ecx, ecx);
    // 0052201D  push 0x10
    // 0052201F  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 00522025  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 00522027  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 00522029  test cl, cl
    // 0052202B  jne 0x522043
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_00522043;
    // 0052202D  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 00522032  test eax, eax
    // 00522034  jne 0x522043
    if ((((eax) & (eax))) != 0) goto LAB_00522043;
    // 00522036  push 0xc8
    // 0052203B  call 0x50dd90
    clock_wait(0xc8);
    // 00522040  add esp, 4
LAB_00522043:
    // 00522043  mov dword ptr [ebp - 4], 1
    var_4 = (1);
LAB_0052204A:
    // 0052204A  push 0
    // 0052204C  push 0x10
    // 0052204E  push esi
    // 0052204F  push ebx
    // 00522050  call 0x591d60
    bit_set(ebx, esi, 0x10, 0);
    // 00522055  mov edx, dword ptr [ebp + 8]
    edx = (a1);
    // 00522058  push edx
    // 00522059  push esi
    // 0052205A  push ebx
    // 0052205B  call 0x591e50
    synch_bit(ebx, esi, edx);
    // 00522060  add esp, 0x1c
    // 00522063  mov ecx, 1
    ecx = (1);
LAB_00522068:
    // 00522068  mov eax, dword ptr [0x68faf0]
    eax = ((*(int*)((char*)g_0068faf0)));
    // 0052206D  imul eax, esi
    eax = ((eax) * (esi));
    // 00522070  add eax, edi
    eax = ((eax) + (edi));
    // 00522072  lea edx, [eax + eax*4]
    edx = (eax + eax*4);
    // 00522075  add eax, ecx
    eax = ((eax) + (ecx));
    // 00522077  lea edx, [eax + edx*2]
    edx = (eax + edx*2);
    // 0052207A  mov eax, dword ptr [0x94a30c]
    eax = ((*(int*)((char*)g_0094a30c)));
    // 0052207F  lea eax, [eax + edx*4 + 0xc]
    eax = (eax + edx*4 + 0xc);
    // 00522083  mov edx, dword ptr [eax]
    edx = ((*(int*)((char*)(eax))));
    // 00522085  and edx, 0xffffffef
    edx = ((edx) & (0xffffffef));
    // 00522088  inc ecx
    ecx = ((ecx) + 1);
    // 00522089  cmp ecx, 8
    // 0052208C  mov dword ptr [eax], edx
    (*(int*)((char*)(eax))) = (edx);
    // 0052208E  jl 0x522068
    if (((int)(ecx)) < ((int)(8))) goto LAB_00522068;
    // 00522090  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 00522093  test eax, eax
    // 00522095  je 0x5220c6
    if ((((eax) & (eax))) == 0) goto LAB_005220C6;
    // 00522097  push 2
    // 00522099  push esi
    // 0052209A  push ebx
    // 0052209B  call 0x46af40
    draw_tile(ebx, esi, 2);
    // 005220A0  add esp, 0xc
    // 005220A3  push 0x10
    // 005220A5  call dword ptr [0x669330]
    eax = (int)((*(short (__stdcall*)(int))(*g_00669330))(0x10));
    // 005220AB  xor ecx, ecx
    ecx = ((ecx) ^ (ecx));
    // 005220AD  mov cl, ah
    SETB0(ecx, (((eax >> 8) & 0xff)));
    // 005220AF  test cl, cl
    // 005220B1  jne 0x5220c6
    if (((((ecx & 0xff)) & ((ecx & 0xff)))) != 0) goto LAB_005220C6;
    // 005220B3  mov eax, dword ptr [0x93f660]
    eax = ((*(int*)((char*)g_0093f660)));
    // 005220B8  test eax, eax
    // 005220BA  jne 0x5220c6
    if ((((eax) & (eax))) != 0) goto LAB_005220C6;
    // 005220BC  push 0x32
    // 005220BE  call 0x50dd90
    clock_wait(0x32);
    // 005220C3  add esp, 4
LAB_005220C6:
    // 005220C6  mov eax, dword ptr [ebp - 0x1c]
    eax = (var_1c);
    // 005220C9  add eax, 4
    eax = ((eax) + (4));
    // 005220CC  cmp eax, 0x54
    // 005220CF  mov dword ptr [ebp - 0x1c], eax
    var_1c = (eax);
    // 005220D2  jl 0x521ec3
    if (((int)(eax)) < ((int)(0x54))) goto LAB_00521EC3;
    // 005220D8  mov eax, dword ptr [ebp - 4]
    eax = (var_4);
    // 005220DB  test eax, eax
    // 005220DD  je 0x522116
    if ((((eax) & (eax))) == 0) goto LAB_00522116;
    // 005220DF  mov edx, dword ptr [ebp - 0x20]
    edx = (var_20);
    // 005220E2  push -1
    // 005220E4  push -1
    // 005220E6  push edx
    // 005220E7  push 0
    // 005220E9  call 0x625ec0
    eax = parse_says(0, (char*)(edx), -1, -1);
    // 005220EE  mov eax, dword ptr [ebp + 8]
    eax = (a1);
    // 005220F1  mov ecx, dword ptr [0x939284]
    ecx = ((*(int*)((char*)g_00939284)));
    // 005220F7  add esp, 0x10
    // 005220FA  cmp eax, ecx
    // 005220FC  jne 0x522103
    if ((eax) != (ecx)) goto LAB_00522103;
    // 005220FE  mov eax, dword ptr [ebp - 8]
    eax = (var_8);
    // 00522101  jmp 0x522106
    goto LAB_00522106;
LAB_00522103:
    // 00522103  or eax, 0xffffffff
    eax = ((eax) | (0xffffffff));
LAB_00522106:
    // 00522106  push eax
    // 00522107  push 0
    // 00522109  push 0x68b48c
    // 0052210E  call 0x405140
    eax = POP2((const char*)(0x68b48c), (const char*)(0), eax);
    // 00522113  add esp, 0xc
LAB_00522116:
    return;
}
