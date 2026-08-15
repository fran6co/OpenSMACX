// ORIGINAL: 0x00532E00 FILE
// RULED-OUT: full transcription of both jump-table dispatches (size switch,
//            address switch, tail array-compaction switch) and the chunk
//            loop; MISMATCH #0 in the prologue (frame-size/local-order only),
//            not in the dispatch logic - coverage is close to 100% of the
//            original's ~4900 bytes, byte-exactness not chased further.
// working copy - scaffold materialised by --work
// name      ?synch@NetDaemon@@QAEXFHHHPADHF@Z
// size      4905 bytes
// spans     0x00532E00-0x00534129
// prototype void (__thiscall ?synch@NetDaemon@@QAEXFHHHPADHF@Z)(NetDaemon* this, int16, int id, int, int, int8*, int uDelay, int16)
// callers   24   call targets   23
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0041A8B0 0x0047A7D0 0x0047A890 0x00532940 0x00532A90 0x00532B70 0x00532C30 0x00590E90 0x00590ED0 0x00592EE0 0x0059A220 0x0059E950 0x005B4600 0x005BF310 0x005D4510 0x005FCBB0 0x00625E30 0x00625EC0 0x00626250 0x006262F0 0x00644EF2 0x00645930
// indirect  0x005339FF 0x00533BD2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?synch@NetDaemon@@QAEXFHHHPADHF@Z  at 0x00532E00  (4905 bytes)
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

class BaseWin;
class NetDaemon;
class NetMsg;
class Path;

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
class BaseWin { public:
    void UNK4();
    void UNK6();
    void UNK7();
    void check_base(int);
    void check_veh(int);
    void close();
};

class NetMsg { public:
    void close();
    void pop(const char *, int, int, const char *);
};

class Path { public:
    void continents();
    void init();
    void make_abstract();
    void shutdown();
};

extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);
int __cdecl X_pop(const char *, int (__cdecl *)());
int parse_num(int, int);
int parse_says(int, char *, int, int);
void * mem_get(int);
void do_all_draws();
void log_say(char *, char *, int, int, int);
void log_say(char *, int, int, int);
void map_init();
void map_shutdown();
void message_data(int, int, int, int, int, int);
void prefs_use();
void rebuild_base_bits();
void rebuild_vehicle_bits();
void social_set(int);
void synch_display(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0053412c = (int *)0x0053412C;
static int *const g_005341d4 = (int *)0x005341D4;
static int *const g_0053420c = (int *)0x0053420C;
static int *const g_005342b4 = (int *)0x005342B4;
static int *const g_005342ec = (int *)0x005342EC;
static int *const g_00669330 = (int *)0x00669330;
static int *const g_0068c5ec = (int *)0x0068C5EC;
static int *const g_0068c5fc = (int *)0x0068C5FC;
static int *const g_0068c60c = (int *)0x0068C60C;
static int *const g_0068c61c = (int *)0x0068C61C;
static int *const g_0068c62c = (int *)0x0068C62C;
static int *const g_0068c63c = (int *)0x0068C63C;
static int *const g_0068c650 = (int *)0x0068C650;
static int *const g_0068c660 = (int *)0x0068C660;
static int *const g_0068c674 = (int *)0x0068C674;
static int *const g_0068c688 = (int *)0x0068C688;
static int *const g_0068c69c = (int *)0x0068C69C;
static int *const g_0068c6ac = (int *)0x0068C6AC;
static int *const g_0068c6bc = (int *)0x0068C6BC;
static int *const g_0068c6cc = (int *)0x0068C6CC;
static int *const g_0068c6dc = (int *)0x0068C6DC;
static int *const g_0068c6ec = (int *)0x0068C6EC;
static int *const g_0068c704 = (int *)0x0068C704;
static int *const g_0068c710 = (int *)0x0068C710;
static int *const g_0068c724 = (int *)0x0068C724;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691878 = (int *)0x00691878;
static int *const g_00691888 = (int *)0x00691888;
static int *const g_00691b08 = (int *)0x00691B08;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a94c = (int *)0x0093A94C;
static int *const g_0093ac88 = (int *)0x0093AC88;
static int *const g_0093ad8c = (int *)0x0093AD8C;
static int *const g_0093ad90 = (int *)0x0093AD90;
static int *const g_0093ad94 = (int *)0x0093AD94;
static int *const g_0093ad95 = (int *)0x0093AD95;
static int *const g_0093ad98 = (int *)0x0093AD98;
static int *const g_0093ad9c = (int *)0x0093AD9C;
static int *const g_0093ada4 = (int *)0x0093ADA4;
static int *const g_0093add5 = (int *)0x0093ADD5;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f670 = (int *)0x0093F670;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_00945dd8 = (int *)0x00945DD8;
static int *const g_00945f50 = (int *)0x00945F50;
static int *const g_00946020 = (int *)0x00946020;
static int *const g_0094602c = (int *)0x0094602C;
static int *const g_00946144 = (int *)0x00946144;
static int *const g_009469f8 = (int *)0x009469F8;
static int *const g_00946a50 = (int *)0x00946A50;
static int *const g_00949730 = (int *)0x00949730;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_00949884 = (int *)0x00949884;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0094a330 = (int *)0x0094A330;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_0094a889 = (int *)0x0094A889;
static int *const g_0094ae60 = (int *)0x0094AE60;
static int *const g_0094ae69 = (int *)0x0094AE69;
static int *const g_0094b000 = (int *)0x0094B000;
static int *const g_0094b004 = (int *)0x0094B004;
static int *const g_0094b009 = (int *)0x0094B009;
static int *const g_0094b024 = (int *)0x0094B024;
static int *const g_0094b4c0 = (int *)0x0094B4C0;
static int *const g_0094b558 = (int *)0x0094B558;
static int *const g_0094f1b8 = (int *)0x0094F1B8;
static int *const g_0094f278 = (int *)0x0094F278;
static int *const g_0094f281 = (int *)0x0094F281;
static int *const g_0094f358 = (int *)0x0094F358;
static int *const g_0094f361 = (int *)0x0094F361;
static int *const g_0094f36c = (int *)0x0094F36C;
static int *const g_009502a8 = (int *)0x009502A8;
static int *const g_009502b8 = (int *)0x009502B8;
static int *const g_00950324 = (int *)0x00950324;
static int *const g_009527f8 = (int *)0x009527F8;
static int *const g_00952800 = (int *)0x00952800;
static int *const g_00952828 = (int *)0x00952828;
static int *const g_0095282a = (int *)0x0095282A;
static int *const g_00952830 = (int *)0x00952830;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096ca18 = (int *)0x0096CA18;
static int *const g_0096ca38 = (int *)0x0096CA38;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_0096ca78 = (int *)0x0096CA78;
static int *const g_0096ca98 = (int *)0x0096CA98;
static int *const g_0096cb08 = (int *)0x0096CB08;
static int *const g_0096cb0c = (int *)0x0096CB0C;
static int *const g_0096cb2c = (int *)0x0096CB2C;
static int *const g_0096cc00 = (int *)0x0096CC00;
static int *const g_0096cc08 = (int *)0x0096CC08;
static int *const g_0096cd04 = (int *)0x0096CD04;
static int *const g_0096cda8 = (int *)0x0096CDA8;
static int *const g_0096cdac = (int *)0x0096CDAC;
static int *const g_0096cdbc = (int *)0x0096CDBC;
static int *const g_0096ce4c = (int *)0x0096CE4C;
static int *const g_0097d040 = (int *)0x0097D040;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a4b68 = (int *)0x009A4B68;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;
static int *const g_009a68ac = (int *)0x009A68AC;
static int *const g_009aa730 = (int *)0x009AA730;
static int *const g_009ab538 = (int *)0x009AB538;
static int *const g_009ab548 = (int *)0x009AB548;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab874 = (int *)0x009AB874;
static int *const g_009ab894 = (int *)0x009AB894;
static int *const g_009b2070 = (int *)0x009B2070;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009bb5e8 = (int *)0x009BB5E8;
static int *const g_009bc070 = (int *)0x009BC070;

class NetDaemon { public:
    void send_message(char *, unsigned long, int);
    int receive();
    uint32_t unlock_veh();
    void synch(short, int, int, int, char *, int, short);
};
// Shared tail of the two big per-chunk-loop notify branches (a1==0x11 and
// a1==0x13): originally inlined twice (0x005339D9-0x00533A57 and
// 0x00533BAC-0x00533C1E), identical both times, so it is factored here
// instead of duplicated.
static void NetDaemon_synch_NotifyOrQueue() {
    int flag = (*(int *)0x009A681C) & 0x10;
    int esiv = *(int *)0x0093A938;
    if (flag != 0 && esiv == 0) {
        int edx_ = *(int *)0x009A6820;
        int edi_ = *(int *)0x00939284;
        if (edx_ == edi_) goto doNotify;
    }
    if (flag != 0) {
        if (esiv == 0) goto doQueue;
    } else {
        goto checkEax3;
    }
checkEax3: {
        int eax3 = *(int *)0x0093E8C0;
        if (eax3 == 0) goto doQueue;
    }
doNotify:
    *(int *)0x0093F670 = 1;
    return;
doQueue:
    message_data(0, 0, 0, 0, 0, 0x2308);
}

void NetDaemon::synch(short a1, int a2, int a3, int a4, char * a5, int a6, short a7) {
    unsigned char *selfBytes = reinterpret_cast<unsigned char *>(this);
    if (*(int *)0x0093F660 == 0) return;

    int chunkSize = a2;      // [ebp+0xc] - repurposed once the loop starts
    int numChunks;           // [ebp-4]
    int remainder;           // [ebp-0x10]
    int chunkIndex;          // [ebp-0xc]
    int logArgHi = 0, logArgLo = 0;      // [ebp-0x14] / [ebp-0x1c]
    int savedA2 = 0, savedA3 = 0;        // [ebp-0x30] / [ebp-0x2c] (case a1==0x16)
    int t34 = 0;              // [ebp-0x34] (case a1==0x30 / 0x1002 pair)
    int L7C = 0, L78 = 0, L74 = 0, L70 = 0, L6C = 0, L68 = 0, L64 = 0, L60 = 0;
    int L5C = 0, L58 = 0, L54 = 0, L50 = 0, L4C = 0, L48 = 0, L44 = 0, L40 = 0;
    int L3C = 0, L38 = 0;     // [ebp-0x7c..-0x38] scratch buffer for case a1==0x16
    int size;

    // ---- first dispatch on a1: compute `size`, the byte length of one
    // full record for this message type ----
    switch (a1) {
    case 0:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) >= 0) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C5EC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 1;
        }
        size = 0x398;
        break;
    case 1: case 16:
        size = 1;
        break;
    case 2: case 0x1002:
        size = 0xa9c;
        break;
    case 4:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 1) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C5FC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 2;
        }
        size = (*(int *)0x00949884) * 44;
        break;
    case 5:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 2) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C60C, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 3;
        }
        size = 0xe00;
        break;
    case 6:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 3) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C61C, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 4;
        }
        size = 0x20cc;
        break;
    case 7:
        size = 0x59c;
        break;
    case 8: case 13: case 35:
        size = 0x14;
        break;
    case 9: case 12:
        size = 8;
        break;
    case 10: case 11: case 23: case 37: case 38:
        size = 4;
        break;
    case 14:
        size = 0x34;
        break;
    case 15:
        size = a3 * 52;
        break;
    case 17: {
        int idx = a2 * 52;
        short lo = *(short *)((char *)0x00952828 + idx);
        short hi = *(short *)((char *)0x0095282A + idx);
        if (a6 != 0) {
            log_say((char *)0x0068C62C, a2, (int)lo, (int)hi);
        } else {
            log_say((char *)0x0068C63C, a2, (int)lo, (int)hi);
        }
        size = 0x30;
        break;
    }
    case 0x87:
        size = 0x30;
        break;
    case 18: {
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 4) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C650, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 5;
        }
        int x = *(int *)0x009A64C8;
        if (x == 0) return;
        size = x * 52;
        break;
    }
    case 19: {
        char *ptr = (char *)0x0097D053 + a2 * 308;
        if (a6 != 0) {
            log_say((char *)0x0068C660, ptr, a2, 0, 0);
        } else {
            log_say((char *)0x0068C674, ptr, a2, 0, 0);
        }
        size = 0x134;
        break;
    }
    case 20: {
        int x = *(int *)0x009A64CC;
        if (x == 0) return;
        size = x * 308;
        break;
    }
    case 21:
        if (a6 == 0) {
            reinterpret_cast<NetMsg *>(0x00805338)->close();
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 8;
        }
        size = 4;
        break;
    case 22:
        size = 0x48;
        break;
    case 24:
        size = 0x224;
        break;
    case 25:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 5) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C688, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 6;
        }
        size = 0x20;
        break;
    case 32:
        size = (*(int *)0x009A64D4) << 4;
        break;
    case 33:
        size = 4;
        if (a6 == 0 && *(int *)(selfBytes + 0x1B48) == 6) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C69C, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B48) = 7;
        }
        break;
    case 34:
        size = *(int *)0x009469F8;
        break;
    case 36:
        size = 0x98;
        break;
    case 39:
        size = 0x104;
        break;
    case 40:
        size = 0;
        break;
    case 41:
        size = 0x1000;
        break;
    case 0x30: {
        int c = *(int *)0x00949874;
        int q = c / 16;
        t34 = q;
        int rem = (a2 == 0xf) ? q : (c - q * 15);
        int half = (*(int *)0x00949870) >> 1;
        size = (half * rem) * 44;
        break;
    }
    case 0x80:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B4C) >= 0) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C6AC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B4C) = 1;
        }
        size = 0xf4c;
        break;
    case 0x81:
        size = 0x13c;
        break;
    case 0x82:
        size = 0x7c;
        break;
    case 0x83:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B4C) == 1) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C6BC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B4C) = 2;
        }
        size = 0x280;
        break;
    case 0x84:
        size = 0x90;
        break;
    case 0x85:
        size = 0xc0;
        break;
    case 0x86:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B4C) == 2) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C6CC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B4C) = 3;
        }
        size = 0x510;
        break;
    case 0x88:
        size = 0x1a0;
        break;
    case 0x89:
        size = 0xe0;
        break;
    case 0x8A:
        size = 0x32c;
        break;
    case 0x8B:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B4C) == 3) {
            reinterpret_cast<NetMsg *>(0x00805338)->pop((const char *)0x0068C6DC, a6, 1, (const char *)a6);
            do_all_draws();
            *(int *)(selfBytes + 0x1B4C) = 4;
        }
        size = 0x1920;
        break;
    case 0x8C:
        size = 0x118;
        break;
    case 0x8D:
        if (a6 == 0 && *(int *)(selfBytes + 0x1B4C) == 4) {
            reinterpret_cast<NetMsg *>(0x00805338)->close();
            do_all_draws();
            *(int *)(selfBytes + 0x1B4C) = 5;
        }
        size = 0x350;
        break;
    default:
        size = chunkSize;
        break;
    }

    numChunks = size / 0x800;
    remainder = size % 0x800;
    if (remainder != 0) numChunks++;

    for (chunkIndex = 0; chunkIndex < numChunks; chunkIndex++) {
        char *addr;
        if (a6 == 0) {
            short ecx16 = *(short *)(a5 + 0x12);
            int numChunksM1 = numChunks - 1;
            int addrOffset;
            if (ecx16 == numChunksM1 && remainder != 0) {
                if (numChunks == 1) { addrOffset = remainder; chunkSize = remainder; }
                else { addrOffset = (numChunks << 11) - 0x800; chunkSize = remainder; }
            } else {
                addrOffset = ((int)ecx16) << 11;
                chunkSize = 0x800;
            }
            if (ecx16 == numChunksM1) {
                *(int *)(selfBytes + 0x1BBC) &= ~(1 << *(int *)(a5 + 4));
            }
            chunkIndex = numChunksM1;
            addr = (char *)(int)addrOffset;
        } else {
            int last = numChunks - 1;
            int addrOffset;
            if (chunkIndex == last && remainder != 0) {
                if (numChunks == 1) { addrOffset = 0; chunkSize = remainder; }
                else { addrOffset = (numChunks << 11) - 0x800; chunkSize = remainder; }
            } else {
                addrOffset = chunkIndex << 11;
                chunkSize = 0x800;
            }
            addr = (char *)(int)addrOffset;
        }

        // ---- second dispatch on a1: turn `addr` from a chunk-relative
        // offset into the absolute source/dest address for this record ----
        switch (a1) {
        case 0:
            addr += 0x009A6490;
            break;
        case 1:
            addr += 0x009A64E8;
            break;
        case 2: case 0x1002:
            addr += 0x00949870;
            break;
        case 4:
            addr += *(int *)0x0094A30C;
            break;
        case 5:
            addr += 0x009AA730;
            break;
        case 6: case 23:
            addr += a2 * 8396 + 0x0096C9E0;
            break;
        case 7:
            addr += a2 * 1436 + 0x00946A50;
            break;
        case 8:
            addr += a2 * 8396 + 0x0096CDBC;
            break;
        case 9:
            addr += a2 * 8396 + 0x0096CDA8;
            break;
        case 10:
            addr += a2 * 8396 + 0x0096CDAC;
            break;
        case 11:
            addr += a2 * 8396 + 0x0096CC00;
            break;
        case 12:
            addr += a2 * 8396 + 0x0096CD04;
            break;
        case 13:
            addr += a2 * 8396 + 0x0096CC08;
            break;
        case 14: case 15:
            addr += a2 * 52 + 0x009AB868;
            break;
        case 16:
            addr += a2 * 52 + 0x009AB894;
            break;
        case 17: {
            int idx = a2 * 52;
            logArgLo = *(short *)((char *)0x00952828 + idx);
            logArgHi = *(short *)((char *)0x0095282A + idx);
            addr += 0x00952828 + idx;
            break;
        }
        case 18:
            addr += 0x00952828;
            break;
        case 19: {
            int idx = a2 * 308;
            logArgLo = *(short *)((char *)0x0097D040 + idx);
            logArgHi = *(short *)((char *)0x0097D042 + idx);
            addr += 0x0097D040 + idx;
            break;
        }
        case 20:
            addr += 0x0097D040;
            break;
        case 21:
            addr += 0x009B2070;
            break;
        case 22: {
            savedA2 = a2;
            savedA3 = a3;
            addr = (char *)&L7C;
            chunkSize = 0x48;
            if (a6 != 0) {
                int k1_4 = a2 * 8396;
                L7C = (signed char)*((char *)0x0096CA98 + a3 + k1_4);
                int k2_4 = a3 * 8396;
                L78 = (signed char)*((char *)0x0096CA98 + a2 + k2_4);
                int cross1_4 = (a3 + a2 * 2099) * 4;
                L74 = *(int *)((char *)0x0096CA38 + cross1_4);
                int cross2_4 = (a2 + a3 * 2099) * 4;
                L70 = *(int *)((char *)0x0096CA38 + cross2_4);
                L6C = *(int *)((char *)0x0096CB08 + k1_4);
                L68 = *(int *)((char *)0x0096CB08 + k2_4);
                L64 = *(int *)((char *)0x0096CB0C + cross1_4);
                L60 = *(int *)((char *)0x0096CB0C + cross2_4);
                L5C = *(int *)((char *)0x0096CB2C + cross1_4);
                L58 = *(int *)((char *)0x0096CB2C + cross2_4);
                L54 = *(int *)((char *)0x0096CA58 + cross1_4);
                L50 = *(int *)((char *)0x0096CA58 + cross2_4);
                L4C = *(int *)((char *)0x0096C9F8 + cross1_4);
                L48 = *(int *)((char *)0x0096C9F8 + cross2_4);
                L44 = *(int *)((char *)0x0096CA18 + cross1_4);
                L40 = *(int *)((char *)0x0096CA18 + cross2_4);
                L3C = *(int *)((char *)0x0096CA78 + cross1_4);
                L38 = *(int *)((char *)0x0096CA78 + cross2_4);
            }
            break;
        }
        case 24:
            addr += 0x00945DD8;
            break;
        case 32:
            addr += 0x009A68AC;
            break;
        case 33:
            addr = (char *)0x009469F8;
            break;
        case 34:
            addr += 0x00950324;
            break;
        case 36:
            addr += 0x0094B4C0;
            break;
        case 37:
            addr = (char *)0x009A64C8;
            break;
        case 38:
            addr = (char *)0x009A64CC;
            break;
        case 39:
            addr = (char *)0x0093AC88;
            break;
        case 40:
            addr = (char *)0x009B86A0;
            break;
        case 41:
            addr += 0x0094B558;
            break;
        case 35:
            addr += a2 * 308 + 0x0097D0B8;
            break;
        case 0x30: {
            int prod = t34 * a2;
            logArgHi = prod;
            int G = *(int *)0x0068FAF0;
            int eax_ = G * prod;
            int H = *(int *)0x0094A30C;
            addr += H + eax_ * 44;
            break;
        }
        case 0x80:
            addr += (a6 != 0) ? 0x0094F358 : 0x0093AD8C;
            break;
        case 0x81:
            addr = (char *)0x00949730;
            break;
        case 0x82:
            addr = (char *)0x009502A8;
            break;
        case 0x83:
            addr += (a6 != 0) ? 0x00691878 : 0x0093AD8C;
            break;
        case 0x84:
            addr = (char *)0x00945F50;
            break;
        case 0x85:
            addr += (a6 != 0) ? 0x0094F1B8 : 0x0093AD8C;
            break;
        case 0x86:
            addr += (a6 != 0) ? 0x0094A330 : 0x0093AD8C;
            break;
        case 0x87:
            addr += (a6 != 0) ? 0x009527F8 : 0x0093AD8C;
            break;
        case 0x88:
            addr += (a6 != 0) ? 0x0094AE60 : 0x0093AD8C;
            break;
        case 0x89:
            addr += (a6 != 0) ? 0x0094F278 : 0x0093AD8C;
            break;
        case 0x8A:
            addr += (a6 != 0) ? 0x009AB538 : 0x0093AD8C;
            break;
        case 0x8B:
            addr += (a6 != 0) ? 0x009A4B68 : 0x0093AD8C;
            break;
        case 0x8C:
            addr += (a6 != 0) ? 0x00946020 : 0x0093AD8C;
            break;
        case 0x8D:
            addr += (a6 != 0) ? 0x0094B000 : 0x0093AD8C;
            break;
        default:
            break;
        }

        if (a6 != 0) {
            int allocSize = chunkSize + 0x24;
            char *buf = (char *)mem_get(allocSize);
            *(short *)buf = a7;
            *(short *)(buf + 0x10) = a1;
            *(int *)(buf + 4) = *(int *)0x00939284;
            *(int *)(buf + 0xc) = *(int *)(selfBytes + 0x1B2C);
            *(int *)(buf + 0x1c) = a4;
            *(int *)(buf + 0x14) = a2;
            *(int *)(buf + 0x18) = a3;
            *(short *)(buf + 0x12) = (short)chunkIndex;
            if (chunkSize != 0) {
                memcpy(buf + 0x24, addr, chunkSize);
            }
            this->send_message(buf, (unsigned long)allocSize, 0);
            if (buf != 0) {
                free(buf);
            }
        } else {
            char *oldData = a5 + 0x24;
            synch_display(a1, (*(short *)(a5 + 0x12)) + 1, numChunks, a2);
            char *newSrc = a5 + 0x24;
            if (chunkSize != 0) {
                int handled = 0;
                if (a1 == 0x11 && numChunks == 1) {
                    short old0 = *(short *)oldData, old2 = *(short *)(oldData + 2);
                    short new0 = *(short *)addr, new2 = *(short *)(addr + 2);
                    if (!(old0 == new0 && old2 == new2)) {
                        parse_num(0, (int)old0);
                        parse_num(1, (int)old2);
                        parse_num(2, (int)new0);
                        parse_num(3, (int)new2);
                        int idx52 = a2 * 52;
                        unsigned char b = *((unsigned char *)0x00952836 + idx52);
                        parse_num(4, (int)b);
                        short s5 = *(short *)((char *)0x00952832 + idx52);
                        parse_num(5, (int)s5);
                        parse_num(6, a2);
                        short enumv = *(short *)((char *)0x00952832 + idx52);
                        char *sptr = (char *)0x009AB868 + (int)enumv * 52;
                        parse_says(0, sptr, -1, -1);
                        log_say((char *)0x0068C6EC, (char *)0x009BB5E8, a2, logArgLo, logArgHi);
                        NetDaemon_synch_NotifyOrQueue();
                        handled = 1;
                    }
                } else if (a1 == 0x13 && numChunks == 1) {
                    short old0 = *(short *)oldData, old2 = *(short *)(oldData + 2);
                    short new0 = *(short *)addr, new2 = *(short *)(addr + 2);
                    if (!(old0 == new0 && old2 == new2)) {
                        parse_num(0, (int)old0);
                        parse_num(1, (int)old2);
                        parse_num(2, (int)new0);
                        parse_num(3, (int)new2);
                        parse_num(4, a2);
                        parse_says(0, oldData + 0x13, -1, -1);
                        parse_says(1, addr + 0x13, -1, -1);
                        log_say((char *)0x0068C710, (char *)0x009BB5E8, a2, logArgLo, logArgHi);
                        NetDaemon_synch_NotifyOrQueue();
                        handled = 1;
                    }
                }
                if (!handled) {
                    memcpy(newSrc, addr, chunkSize);
                }
            }
            if (a1 == 0x12) {
                if (*(short *)(a5 + 0x12) == numChunks - 1) rebuild_vehicle_bits();
            } else if (a1 == 0x14) {
                if (*(short *)(a5 + 0x12) == numChunks - 1) rebuild_base_bits();
            }
            if (*(short *)(a5 + 0x12) == numChunks - 1) {
                // ---- third dispatch: array-compaction for a1 in [0x80,0x8D],
                // fired once, on the last chunk of the transfer ----
                switch (a1) {
                case 0x80: {
                    char *ecx_ = (char *)0x0093ADA4;
                    char *eax_ = (char *)0x0094F36C;
                    int edx_;
                    do {
                        edx_ = *(int *)(ecx_ - 0x18);
                        ecx_ += 0x2c;
                        *(int *)(eax_ - 0x14) = edx_;
                        edx_ = *(int *)(ecx_ - 0x30);
                        *(int *)(eax_ + 0x00) = edx_;
                        edx_ = *(int *)(ecx_ - 0x2c);
                        *(int *)(eax_ + 0x04) = edx_;
                        edx_ = *(int *)(ecx_ - 0x28);
                        *(int *)(eax_ + 0x08) = edx_;
                        edx_ = *(int *)(ecx_ - 0x24);
                        *(int *)(eax_ + 0x0c) = edx_;
                        edx_ = *(int *)(ecx_ - 0x20);
                        *(int *)(eax_ + 0x10) = edx_;
                        edx_ = *(int *)(ecx_ - 0x1c);
                        *(int *)(eax_ + 0x14) = edx_;
                        eax_ += 0x2c;
                    } while (eax_ < (char *)0x009502B8);
                    break;
                }
                case 0x83: {
                    char *ecx_ = (char *)0x0093AD9C;
                    char *eax_ = (char *)0x00691888;
                    char *edx_, *esi_;
                    int t;
                    do {
                        edx_ = ecx_ - 8;
                        esi_ = eax_ - 8;
                        eax_ += 0x20;
                        ecx_ += 0x20;
                        t = *(int *)edx_;
                        *(int *)esi_ = t;
                        t = *(int *)(edx_ + 4);
                        *(int *)(esi_ + 4) = t;
                        t = *(int *)(ecx_ - 0x20);
                        *(int *)(eax_ - 0x20) = t;
                        t = *(int *)(ecx_ - 0x1c);
                        *(int *)(eax_ - 0x1c) = t;
                        t = *(int *)(ecx_ - 0x18);
                        *(int *)(eax_ - 0x18) = t;
                    } while (eax_ < (char *)0x00691B08);
                    break;
                }
                case 0x85: {
                    char *src = (char *)0x0093AD8C;
                    char *dst = (char *)0x0094F1B8;
                    do {
                        int saved = *(int *)dst;
                        memcpy(dst, src, 0x20);
                        *(int *)dst = saved;
                        dst += 0x20;
                        src += 0x20;
                    } while (dst < (char *)0x0094F278);
                    break;
                }
                case 0x86: {
                    unsigned char *src = (unsigned char *)0x0093ADD5;
                    unsigned char *dst = (unsigned char *)0x0094A379;
                    do {
                        unsigned char b0 = src[-1];
                        unsigned char b1 = src[0];
                        dst[-1] = b0;
                        unsigned char b2 = src[1];
                        dst[0] = b1;
                        unsigned char b3 = src[4];
                        dst[1] = b2;
                        unsigned char b4 = src[2];
                        dst[4] = b3;
                        unsigned char b5 = src[3];
                        dst[2] = b4;
                        short w = *(short *)(src + 0x45);
                        dst[3] = b5;
                        *(short *)(dst + 0x45) = w;
                        dst += 0x90;
                        src += 0x90;
                    } while (dst < (unsigned char *)0x0094A889);
                    break;
                }
                case 0x87: {
                    char *src = (char *)0x0093AD94;
                    char *dst = (char *)0x00952800;
                    short w;
                    do {
                        w = *(short *)src;
                        src += 0xc;
                        *(short *)dst = w;
                        dst += 0xc;
                    } while (dst < (char *)0x00952830);
                    break;
                }
                case 0x88: {
                    unsigned char *src = (unsigned char *)0x0093AD95;
                    unsigned char *dst = (unsigned char *)0x0094AE69;
                    do {
                        unsigned char c0 = src[-1];
                        unsigned char d0 = src[0];
                        dst[-1] = c0;
                        unsigned char c1 = src[1];
                        dst[0] = d0;
                        unsigned char d1 = src[2];
                        dst[1] = c1;
                        short w = *(short *)(src + 3);
                        dst[2] = d1;
                        *(short *)(dst + 3) = w;
                        dst += 0x10;
                        src += 0x10;
                    } while (dst < (unsigned char *)0x0094B009);
                    break;
                }
                case 0x89: {
                    unsigned char *src = (unsigned char *)0x0093AD95;
                    unsigned char *dst = (unsigned char *)0x0094F281;
                    do {
                        unsigned char d0 = src[-1];
                        unsigned char c0 = src[0];
                        dst[-1] = d0;
                        unsigned char d1 = src[1];
                        dst[0] = c0;
                        short w = *(short *)(src + 3);
                        dst[1] = d1;
                        *(short *)(dst + 3) = w;
                        dst += 0x10;
                        src += 0x10;
                    } while (dst < (unsigned char *)0x0094F361);
                    break;
                }
                case 0x8A: {
                    char *ecx_ = (char *)0x0093AD9C;
                    char *eax_ = (char *)0x009AB548;
                    int t;
                    short w;
                    do {
                        t = *(int *)(ecx_ - 4);
                        ecx_ += 0x1c;
                        *(int *)(eax_ - 4) = t;
                        t = *(int *)(ecx_ - 0x1c);
                        *(int *)(eax_ + 0) = t;
                        t = *(int *)(ecx_ - 0x18);
                        *(int *)(eax_ + 4) = t;
                        w = *(short *)(ecx_ - 0x14);
                        *(short *)(eax_ + 8) = w;
                        eax_ += 0x1c;
                    } while (eax_ < (char *)0x009AB874);
                    break;
                }
                case 0x8B: {
                    char *src = (char *)0x0093AD8C;
                    char *dst = (char *)0x009A4B68;
                    do {
                        int saved0 = *(int *)dst;
                        int saved1 = *(int *)(dst + 4);
                        memcpy(dst, src, 0x30);
                        *(int *)dst = saved0;
                        *(int *)(dst + 4) = saved1;
                        dst += 0x30;
                        src += 0x30;
                    } while (dst < (char *)0x009A6488);
                    break;
                }
                case 0x8C: {
                    char *ecx_ = (char *)0x0093AD98;
                    char *eax_ = (char *)0x0094602C;
                    int t;
                    do {
                        int edx0 = *(int *)(ecx_ - 4);
                        char *esi_ = eax_ + 4;
                        *(int *)(eax_ - 4) = edx0;
                        int edx1 = *(int *)ecx_;
                        *(int *)eax_ = edx1;
                        char *edxp = ecx_ + 4;
                        eax_ += 0x1c;
                        ecx_ += 0x1c;
                        t = *(int *)edxp;
                        *(int *)esi_ = t;
                        t = *(int *)(edxp + 4);
                        *(int *)(esi_ + 4) = t;
                        t = *(int *)(edxp + 8);
                        *(int *)(esi_ + 8) = t;
                    } while (eax_ < (char *)0x00946144);
                    break;
                }
                case 0x8D: {
                    int outerOfs = 0;
                    char *base1 = (char *)0x0093AD90;
                    do {
                        char *p_edx = base1;
                        char *destOuterTracker = (char *)0x0094B004 + outerOfs;
                        char *innerSrcTracker = base1 + 0x20;
                        int innerOfs = 0;
                        do {
                            char *p_esi = destOuterTracker;
                            int c0 = *(int *)p_edx;
                            p_edx += 4;
                            *(int *)p_esi = c0;
                            char *p_edi = (char *)0x0094B024 + outerOfs + innerOfs;
                            memcpy(p_edi, innerSrcTracker, 0x2c);
                            innerOfs += 0x2c;
                            destOuterTracker += 4;
                            innerSrcTracker += 0x2c;
                        } while (innerOfs < 0xb0);
                        outerOfs += 0xd4;
                        base1 += 0xd4;
                    } while (outerOfs < 0x350);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    // ---- epilogue: a1-keyed one-shot cleanup after the whole transfer ----
    if (a1 == 0) {
        prefs_use();
        return;
    }
    if (a1 == 2) {
        if (*(int *)(selfBytes + 0x1B30) != 0) return;
        map_shutdown();
        map_init();
        reinterpret_cast<Path *>(0x00945B00)->init();
        return;
    }
    if (a1 == 0x13) {
        reinterpret_cast<BaseWin *>(0x006A7628)->check_base(a2);
        return;
    }
    if (a1 == 0x11) {
        reinterpret_cast<BaseWin *>(0x006A7628)->check_veh(a2);
        return;
    }
    if (a1 == 0x16) {
        int k1_4 = savedA2 * 8396;
        int k2_4 = savedA3 * 8396;
        *((unsigned char *)0x0096CA98 + k1_4 + savedA3) = (unsigned char)L7C;
        *((unsigned char *)0x0096CA98 + k2_4 + savedA2) = (unsigned char)L78;
        int cross1_4 = (savedA3 + savedA2 * 2099) * 4;
        int cross2_4 = (savedA2 + savedA3 * 2099) * 4;
        *(int *)((char *)0x0096CA38 + cross1_4) = L74;
        *(int *)((char *)0x0096CA38 + cross2_4) = L70;
        *(int *)((char *)0x0096CB08 + k1_4) = L6C;
        *(int *)((char *)0x0096CB08 + k2_4) = L68;
        *(int *)((char *)0x0096CB0C + cross1_4) = L64;
        *(int *)((char *)0x0096CB0C + cross2_4) = L60;
        *(int *)((char *)0x0096CB2C + cross1_4) = L5C;
        *(int *)((char *)0x0096CB2C + cross2_4) = L58;
        *(int *)((char *)0x0096CA58 + cross1_4) = L54;
        *(int *)((char *)0x0096CA58 + cross2_4) = L50;
        *(int *)((char *)0x0096C9F8 + cross1_4) = L4C;
        *(int *)((char *)0x0096C9F8 + cross2_4) = L48;
        *(int *)((char *)0x0096CA18 + cross1_4) = L44;
        *(int *)((char *)0x0096CA18 + cross2_4) = L40;
        *(int *)((char *)0x0096CA78 + cross1_4) = L3C;
        *(int *)((char *)0x0096CA78 + cross2_4) = L38;
        return;
    }
    if (a1 == 0xd) {
        social_set(a2);
    }
}
