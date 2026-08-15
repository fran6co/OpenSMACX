// ORIGINAL: 0x005B0E00 FILE
// RULED-OUT: full-body transcription from Ghidra hypothesis, corrected
//            against raw disasm for: tech_ai/tech_achieved and abil_index/
//            has_tech dangling-outer-call pushes (same shape as time_warp's
//            has_tech fold), veh_lift/veh_drop split (PROPOSAL: veh_lift
//            should be `int veh_lift(int)`), CharUpperA reached through the
//            IAT thunk at g_0066931c (matches src/recovered's established
//            idiom), and per-field pointer widths across the ~230 referenced
//            globals recovered from `movsx`/`test byte ptr` on the raw
//            disassembly, since the scaffold's g_ globals are declared int*
//            uniformly. All C2xxx NO_COMPILE errors from iterative
//            verify_recovered_function.py runs were fixed; the final
//            byte-tier measurement did not finish inside this session (a
//            single VC6 /O2 pass on this body ran >12 minutes without
//            completing - the same body compiles other optimisation levels'
//            passes fine, so this reads as a slow pass, not a stuck one).
// working copy - scaffold materialised by --work
// name      ?setup_player@@YAHHHH@Z
// size      9596 bytes
// spans     0x005B0E00-0x005B337C
// prototype int (__cdecl ?setup_player@@YAHHHH@Z)(int8*, int protoID, int)
// callers   4   call targets   37
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x004E25B0 0x004E3B80 0x004E3D50 0x004E49D0 0x004E4B80 0x004E5250 0x0050BA30 0x0052E050 0x0055B760 0x00579E00 0x0057D360 0x0057D480 0x0057DF00 0x0057E040 0x00581260 0x005900D0 0x00591B50 0x00591E50 0x005A5D40 0x005BA910 0x005BB000 0x005BDC10 0x005BF310 0x005C01D0 0x005C0260 0x005C03D0 0x005C0B00 0x005C4FD0 0x006169A0 0x00625EC0 0x00644F3A 0x00645460 0x00645470 0x00645660 0x0064601D 0x0064FC88
// indirect  0x005B2663 0x005B2BE2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?setup_player@@YAHHHH@Z  at 0x005B0E00  (9596 bytes)
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
typedef int BOOL;
class Heap;
typedef void * LPVOID;
class Lock;
class NetDaemon;
class Strings;
class VoiceTx;

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
    void close();
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

class Lock { public:
    int any_locks();
    void check_global();
    void clear();
    void reset_map();
};

class NetDaemon { public:
    int receive();
    uint32_t unlock_veh();
    void hang_up();
};

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
    void shutdown();
};

extern "C" char *_itoa(int, char *, int);
extern "C" char *strcat(char *, const char *);
extern "C" char *strcpy(char *, const char *);
extern "C" int abs(int);
extern "C" int rand();
extern "C" int strcmp(const char *, const char *);
int __cdecl X_pop(const char *, int (__cdecl *)());
int armor_val(int, int);
int base_find(int, int);
int base_find(int, int, int, int, int, int);
int base_init(int, int, int);
int bit_count(int);
int is_coast(int, int, int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int tech_ai(int);
int veh_avail(int, int, int);
int veh_init(int, int, int, int);
int weap_val(int, int);
int world_site(int, int, int);
void base_kill(int);
void clear_bunglist(int);
void consider_designs(int);
void init_goals(int);
void kill(int);
void make_proto(int, int, int, int, int, int);
void name_proto(char *, int, int, int, int, int, int, int);
void site_set(int, int, int);
void synch_bit(int, int, int);
void tech_achieved(int, int, int, int);
void treaty_off(int, int, int);
void veh_demote(int);
void veh_promote(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_0066ef50 = (int *)0x0066EF50;
static int *const g_0066ef74 = (int *)0x0066EF74;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_0066f8c0 = (int *)0x0066F8C0;
static int *const g_0066f8c4 = (int *)0x0066F8C4;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691500 = (int *)0x00691500;
static int *const g_00691504 = (int *)0x00691504;
static int *const g_00691508 = (int *)0x00691508;
static int *const g_00691524 = (int *)0x00691524;
static int *const g_0069152c = (int *)0x0069152C;
static int *const g_00691538 = (int *)0x00691538;
static int *const g_00691544 = (int *)0x00691544;
static int *const g_00691550 = (int *)0x00691550;
static int *const g_0069155c = (int *)0x0069155C;
static int *const g_00691568 = (int *)0x00691568;
static int *const g_00691570 = (int *)0x00691570;
static int *const g_00691578 = (int *)0x00691578;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_0090ea04 = (int *)0x0090EA04;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_009393c0 = (int *)0x009393C0;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e230 = (int *)0x0093E230;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093e8c4 = (int *)0x0093E8C4;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093e8cc = (int *)0x0093E8CC;
static int *const g_0093e8d0 = (int *)0x0093E8D0;
static int *const g_0093e8d4 = (int *)0x0093E8D4;
static int *const g_0093e8d8 = (int *)0x0093E8D8;
static int *const g_0093e8dc = (int *)0x0093E8DC;
static int *const g_0093e8e0 = (int *)0x0093E8E0;
static int *const g_0093e8e4 = (int *)0x0093E8E4;
static int *const g_0093e8e8 = (int *)0x0093E8E8;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e8f0 = (int *)0x0093E8F0;
static int *const g_0093e8f8 = (int *)0x0093E8F8;
static int *const g_0093e8fc = (int *)0x0093E8FC;
static int *const g_0093e908 = (int *)0x0093E908;
static int *const g_0093e90c = (int *)0x0093E90C;
static int *const g_0093e910 = (int *)0x0093E910;
static int *const g_0093e930 = (int *)0x0093E930;
static int *const g_0093e934 = (int *)0x0093E934;
static int *const g_0093e938 = (int *)0x0093E938;
static int *const g_0093e93c = (int *)0x0093E93C;
static int *const g_0093e940 = (int *)0x0093E940;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_0093e950 = (int *)0x0093E950;
static int *const g_0093e954 = (int *)0x0093E954;
static int *const g_0093e958 = (int *)0x0093E958;
static int *const g_0093e95c = (int *)0x0093E95C;
static int *const g_0093e960 = (int *)0x0093E960;
static int *const g_0093e964 = (int *)0x0093E964;
static int *const g_0093e968 = (int *)0x0093E968;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946138 = (int *)0x00946138;
static int *const g_00946a54 = (int *)0x00946A54;
static int *const g_00946a84 = (int *)0x00946A84;
static int *const g_00946a99 = (int *)0x00946A99;
static int *const g_00946d34 = (int *)0x00946D34;
static int *const g_00946d49 = (int *)0x00946D49;
static int *const g_00946d4c = (int *)0x00946D4C;
static int *const g_00946d50 = (int *)0x00946D50;
static int *const g_00946dd4 = (int *)0x00946DD4;
static int *const g_00946df9 = (int *)0x00946DF9;
static int *const g_00946f38 = (int *)0x00946F38;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00946f59 = (int *)0x00946F59;
static int *const g_00946f5c = (int *)0x00946F5C;
static int *const g_00946f80 = (int *)0x00946F80;
static int *const g_00946fc0 = (int *)0x00946FC0;
static int *const g_00946fc4 = (int *)0x00946FC4;
static int *const g_00946fc8 = (int *)0x00946FC8;
static int *const g_00946fcc = (int *)0x00946FCC;
static int *const g_00946fd0 = (int *)0x00946FD0;
static int *const g_00947020 = (int *)0x00947020;
static int *const g_009472d0 = (int *)0x009472D0;
static int *const g_00947370 = (int *)0x00947370;
static int *const g_009497b4 = (int *)0x009497B4;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_00949888 = (int *)0x00949888;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a2a4 = (int *)0x0094A2A4;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0094a378 = (int *)0x0094A378;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_0094ae68 = (int *)0x0094AE68;
static int *const g_00952828 = (int *)0x00952828;
static int *const g_0095282a = (int *)0x0095282A;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_0095284c = (int *)0x0095284C;
static int *const g_00952856 = (int *)0x00952856;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9e8 = (int *)0x0096C9E8;
static int *const g_0096c9ec = (int *)0x0096C9EC;
static int *const g_0096c9f0 = (int *)0x0096C9F0;
static int *const g_0096c9f4 = (int *)0x0096C9F4;
static int *const g_0096ca78 = (int *)0x0096CA78;
static int *const g_0096ca98 = (int *)0x0096CA98;
static int *const g_0096caa0 = (int *)0x0096CAA0;
static int *const g_0096cb04 = (int *)0x0096CB04;
static int *const g_0096cb08 = (int *)0x0096CB08;
static int *const g_0096cbac = (int *)0x0096CBAC;
static int *const g_0096cbb0 = (int *)0x0096CBB0;
static int *const g_0096cbb4 = (int *)0x0096CBB4;
static int *const g_0096cbb8 = (int *)0x0096CBB8;
static int *const g_0096cbbc = (int *)0x0096CBBC;
static int *const g_0096cbe0 = (int *)0x0096CBE0;
static int *const g_0096cc00 = (int *)0x0096CC00;
static int *const g_0096cc04 = (int *)0x0096CC04;
static int *const g_0096cc08 = (int *)0x0096CC08;
static int *const g_0096cc28 = (int *)0x0096CC28;
static int *const g_0096cc2c = (int *)0x0096CC2C;
static int *const g_0096ccb0 = (int *)0x0096CCB0;
static int *const g_0096ccc0 = (int *)0x0096CCC0;
static int *const g_0096ccd8 = (int *)0x0096CCD8;
static int *const g_0096ccdc = (int *)0x0096CCDC;
static int *const g_0096cce0 = (int *)0x0096CCE0;
static int *const g_0096cce4 = (int *)0x0096CCE4;
static int *const g_0096cce8 = (int *)0x0096CCE8;
static int *const g_0096ccec = (int *)0x0096CCEC;
static int *const g_0096ccf0 = (int *)0x0096CCF0;
static int *const g_0096ccf4 = (int *)0x0096CCF4;
static int *const g_0096ccf8 = (int *)0x0096CCF8;
static int *const g_0096ccfc = (int *)0x0096CCFC;
static int *const g_0096cd00 = (int *)0x0096CD00;
static int *const g_0096cd04 = (int *)0x0096CD04;
static int *const g_0096cd08 = (int *)0x0096CD08;
static int *const g_0096cd0c = (int *)0x0096CD0C;
static int *const g_0096cd3c = (int *)0x0096CD3C;
static int *const g_0096cd40 = (int *)0x0096CD40;
static int *const g_0096cd44 = (int *)0x0096CD44;
static int *const g_0096cd48 = (int *)0x0096CD48;
static int *const g_0096cd4c = (int *)0x0096CD4C;
static int *const g_0096cda8 = (int *)0x0096CDA8;
static int *const g_0096cdac = (int *)0x0096CDAC;
static int *const g_0096cdb0 = (int *)0x0096CDB0;
static int *const g_0096cdb4 = (int *)0x0096CDB4;
static int *const g_0096cdb8 = (int *)0x0096CDB8;
static int *const g_0096cdbc = (int *)0x0096CDBC;
static int *const g_0096cdc0 = (int *)0x0096CDC0;
static int *const g_0096cdc4 = (int *)0x0096CDC4;
static int *const g_0096cdc8 = (int *)0x0096CDC8;
static int *const g_0096cdcc = (int *)0x0096CDCC;
static int *const g_0096cdd0 = (int *)0x0096CDD0;
static int *const g_0096cdd4 = (int *)0x0096CDD4;
static int *const g_0096cdd8 = (int *)0x0096CDD8;
static int *const g_0096cddc = (int *)0x0096CDDC;
static int *const g_0096ce0c = (int *)0x0096CE0C;
static int *const g_0096ce38 = (int *)0x0096CE38;
static int *const g_0096ce3c = (int *)0x0096CE3C;
static int *const g_0096ce40 = (int *)0x0096CE40;
static int *const g_0096ce44 = (int *)0x0096CE44;
static int *const g_0096ce48 = (int *)0x0096CE48;
static int *const g_0096ce4c = (int *)0x0096CE4C;
static int *const g_0096ce50 = (int *)0x0096CE50;
static int *const g_0096cf10 = (int *)0x0096CF10;
static int *const g_0096d1e8 = (int *)0x0096D1E8;
static int *const g_0096d1ec = (int *)0x0096D1EC;
static int *const g_0096d1f0 = (int *)0x0096D1F0;
static int *const g_0096d1f4 = (int *)0x0096D1F4;
static int *const g_0096d1f8 = (int *)0x0096D1F8;
static int *const g_0096d1fc = (int *)0x0096D1FC;
static int *const g_0096d200 = (int *)0x0096D200;
static int *const g_0096d204 = (int *)0x0096D204;
static int *const g_0096d208 = (int *)0x0096D208;
static int *const g_0096d20c = (int *)0x0096D20C;
static int *const g_0096d210 = (int *)0x0096D210;
static int *const g_0096d214 = (int *)0x0096D214;
static int *const g_0096d218 = (int *)0x0096D218;
static int *const g_0096d21c = (int *)0x0096D21C;
static int *const g_0096d238 = (int *)0x0096D238;
static int *const g_0096d438 = (int *)0x0096D438;
static int *const g_0096d638 = (int *)0x0096D638;
static int *const g_0096da38 = (int *)0x0096DA38;
static int *const g_0096da3c = (int *)0x0096DA3C;
static int *const g_0096da40 = (int *)0x0096DA40;
static int *const g_0096da44 = (int *)0x0096DA44;
static int *const g_0096da48 = (int *)0x0096DA48;
static int *const g_0096da4c = (int *)0x0096DA4C;
static int *const g_0096db5c = (int *)0x0096DB5C;
static int *const g_0096dbdc = (int *)0x0096DBDC;
static int *const g_0096dc5c = (int *)0x0096DC5C;
static int *const g_0096ea1c = (int *)0x0096EA1C;
static int *const g_0096ea20 = (int *)0x0096EA20;
static int *const g_0096ea24 = (int *)0x0096EA24;
static int *const g_0096ea28 = (int *)0x0096EA28;
static int *const g_0096ea2c = (int *)0x0096EA2C;
static int *const g_0096ea30 = (int *)0x0096EA30;
static int *const g_0096ea34 = (int *)0x0096EA34;
static int *const g_0096ea3c = (int *)0x0096EA3C;
static int *const g_0096ea5c = (int *)0x0096EA5C;
static int *const g_0096ea68 = (int *)0x0096EA68;
static int *const g_0096ea70 = (int *)0x0096EA70;
static int *const g_0096eac4 = (int *)0x0096EAC4;
static int *const g_0096fc28 = (int *)0x0096FC28;
static int *const g_0097d040 = (int *)0x0097D040;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d046 = (int *)0x0097D046;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009a6504 = (int *)0x009A6504;
static int *const g_009a6508 = (int *)0x009A6508;
static int *const g_009aa730 = (int *)0x009AA730;
static int *const g_009aa734 = (int *)0x009AA734;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab898 = (int *)0x009AB898;
static int *const g_009b2068 = (int *)0x009B2068;
static int *const g_009b206c = (int *)0x009B206C;
static int *const g_009b208c = (int *)0x009B208C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;
static int *const g_009bc054 = (int *)0x009BC054;
int __cdecl setup_player(int a1, int a2, int a3) {
  unsigned int param_1 = (unsigned int)a1;
  unsigned char *param_2 = (unsigned char *)a2;
  int param_3 = a3;
  unsigned int uVar1;
  short sVar2;
  int uVar3;
  int uVar4;
  unsigned int uVar5;
  int iVar6;
  int uVar7;
  unsigned int uVar8;
  int *puVar9;
  int iVar10;
  unsigned short *puVar11;
  int *piVar12;
  unsigned int *puVar13;
  unsigned char bVar14;
  unsigned char bVar15;
  short *puVar16;
  int iVar17;
  unsigned int uVar18;
  int iVar19;
  char *pcVar20;
  unsigned char *pbVar21;
  unsigned char *puVar22;
  short *psVar23;
  int iVar24;
  unsigned int uVar25;
  char *pcVar26;
  bool bVar27;
  unsigned char local_e0 [31];
  unsigned char local_c1;
  unsigned char local_90 [80];
  int local_40;
  int local_3c;
  unsigned int local_38;
  int local_34;
  int local_30;
  int local_2c;
  unsigned int local_28;
  unsigned int local_24;
  char *local_20;
  int local_1c;
  unsigned int local_18;
  unsigned int local_14;
  int local_10;
  unsigned int local_c;
  unsigned int local_8;
  
  uVar18 = param_1;
  local_3c = 0;
  local_10 = 0;
  if (1 < (int)param_1) {
    local_14 = param_1 - 1;
    pcVar20 = (char *)g_00947020;
    local_20 = (char *)g_00946a84 + param_1 * 0x59c;
    do {
      iVar6 = strcmp(pcVar20,local_20);
      pcVar26 = local_20;
      if (iVar6 == 0) {
        local_10 = local_10 + 1;
      }
      pcVar20 = pcVar20 + 0x59c;
      local_14 = local_14 + -1;
    } while (local_14 != 0);
    if (local_10 != 0) {
      ((char *)g_00946a99)[param_1 * 0x59c] = 0;
      strcat(local_20,(char *)g_00691500);
      _itoa(local_10 + 1,(char *)local_90,10);
      strcat(pcVar26,(char *)local_90);
    }
  }
  local_10 = 0;
  if (1 < (int)param_1) {
    local_14 = param_1 - 1;
    pcVar20 = (char *)g_009472d0;
    local_20 = (char *)g_00946d34 + param_1 * 0x59c;
    do {
      iVar6 = strcmp(pcVar20,local_20);
      pcVar26 = local_20;
      if (iVar6 == 0) {
        local_10 = local_10 + 1;
      }
      pcVar20 = pcVar20 + 0x59c;
      local_14 = local_14 + -1;
    } while (local_14 != 0);
    if (local_10 != 0) {
      ((char *)g_00946d49)[param_1 * 0x59c] = 0;
      strcat(local_20,(char *)g_00691504);
      _itoa(local_10 + 1,(char *)local_90,10);
      strcat(pcVar26,(char *)local_90);
    }
  }
  local_10 = 0;
  if (1 < (int)param_1) {
    local_14 = param_1 - 1;
    pcVar20 = (char *)g_00947370;
    local_20 = (char *)g_00946dd4 + param_1 * 0x59c;
    do {
      iVar6 = strcmp(pcVar20,local_20);
      pcVar26 = local_20;
      if (iVar6 == 0) {
        local_10 = local_10 + 1;
      }
      pcVar20 = pcVar20 + 0x59c;
      local_14 = local_14 + -1;
    } while (local_14 != 0);
    if (local_10 != 0) {
      ((char *)g_00946df9)[param_1 * 0x59c] = 0;
      strcat(local_20,(char *)g_00691508);
      _itoa(local_10 + 1,(char *)local_90,10);
      strcat(pcVar26,(char *)local_90);
    }
  }
  iVar6 = (*g_009a64cc) + -1;
  if (-1 < iVar6) {
    pbVar21 = (unsigned char *)((char *)g_0097d044 + iVar6 * 0x134);
    do {
      if (*pbVar21 == param_1) {
        base_kill(iVar6);
      }
      iVar6 = iVar6 + -1;
      pbVar21 = pbVar21 + -0x134;
    } while (-1 < iVar6);
  }
  iVar6 = (*g_009a64c8) + -1;
  if (-1 < iVar6) {
    psVar23 = (short *)((char *)g_00952832 + iVar6 * 0x1a);
    do {
      if (((iVar6 < (*g_009a64c8)) && (*(unsigned char *)(psVar23 + 2) == param_1)) &&
         ((*psVar23 != 0x13 || (param_1 != 0)))) {
        kill(iVar6);
      }
      iVar6 = iVar6 + -1;
      psVar23 = psVar23 + -0x1a;
    } while (-1 < iVar6);
  }
  iVar6 = param_1 * 0x20cc;
  *(int *)((char *)g_0096cdd4 + iVar6) = 0xffffffff;
  *(int *)((char *)g_0096cdd0 + iVar6) = 0xffffffff;
  (g_0096ce48)[param_1 * 0x833] = 0xffffffff;
  bVar14 = (unsigned char)param_1;
  local_2c = iVar6;
  if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
    init_goals(param_1);
    clear_bunglist(param_1);
    iVar24 = (*g_009a64c4);
    *(int *)((char *)g_0096c9e0 + iVar6) = 0;
    *(int *)((char *)g_0096cbb0 + iVar6) = 0xdfbe41;
    if (iVar24 == 0) {
      *(int *)((char *)g_0096cbb0 + iVar6) = 0xc0dfbe41;
    }
    iVar24 = 0;
    *(int *)((char *)g_0096c9ec + local_2c) = 0;
    *(int *)((char *)g_0096c9f0 + local_2c) = 0;
    puVar22 = (unsigned char *)((char *)g_0096ce50 + local_2c);
    iVar6 = param_1 * 0x59c;
    uVar7 = *(int *)((char *)g_00946fd0 + iVar6);
    *(int *)((char *)g_0096cdbc + local_2c) = *(int *)((char *)g_00946fc0 + iVar6);
    uVar3 = *(int *)((char *)g_00946fc8 + iVar6);
    *(int *)((char *)g_0096cdcc + local_2c) = uVar7;
    uVar7 = *(int *)((char *)g_00946fcc + iVar6);
    uVar4 = *(int *)((char *)g_00946fc4 + iVar6);
    *(int *)((char *)g_0096cdc4 + local_2c) = uVar3;
    *(int *)((char *)g_0096cdc8 + local_2c) = uVar7;
    *(int *)((char *)g_0096cdc0 + local_2c) = uVar4;
    *(int *)((int)(char *)g_0096ce4c + local_2c) = 0;
    puVar9 = (int *)((char *)g_0096cf10 + local_2c);
    do {
      iVar6 = (*g_009b90f8);
      *puVar9 = 0;
      (*(char *)g_009b86a0) = 0;
      uVar7 = reinterpret_cast<Strings *>(g_009b90d8)->get(*(int *)(iVar6 + 0x984));
      strcat((char *)g_009b86a0,(char *)uVar7);
      strcat((char *)g_009b86a0,(char *)g_00682820);
      iVar24 = iVar24 + 1;
      _itoa(iVar24,(char *)local_90,10);
      strcat((char *)g_009b86a0,(char *)local_90);
      strcpy((char *)puVar22,(char *)g_009b86a0);
      iVar6 = local_2c;
      puVar9 = puVar9 + 1;
      puVar22 = puVar22 + 0x18;
    } while (iVar24 < 8);
    iVar24 = 4;
    puVar9 = (int *)((char *)g_0096cc08 + local_2c);
    do {
      puVar9[4] = 0;
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
      iVar24 = iVar24 + -1;
    } while (iVar24 != 0);
    puVar9 = (int *)((char *)g_0096cc2c + local_2c);
    for (iVar24 = 0xb; iVar24 != 0; iVar24 = iVar24 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    *(int *)((char *)g_0096cc28 + local_2c) = 0;
    puVar9 = (int *)((int)(char *)g_0096ea3c + local_2c);
    for (iVar24 = 8; iVar24 != 0; iVar24 = iVar24 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    iVar24 = (*g_009a64c4);
    if (((unsigned char)(1 << (bVar14 & 0x1f)) & (unsigned char)(*g_009a64e8)) == 0) {
      iVar24 = 3;
    }
    *(int *)((int)(char *)g_0096c9e8 + local_2c) = iVar24;
    uVar8 = (*g_009a64c0);
    *(int *)((char *)g_0096cb04 + local_2c) = 0;
    *(int *)((char *)g_0096cb08 + local_2c) = 0;
    *(int *)((char *)g_0096cbac + local_2c) = 0;
    *(int *)((int)(char *)g_0096cc00 + local_2c) = 0;
    *(int *)((char *)g_0096cbb4 + local_2c) = 0;
    *(int *)((char *)g_0096cbb8 + local_2c) = 0;
    *(int *)((int)(char *)g_0096ce44 + local_2c) = 0;
    *(int *)((char *)g_0096caa0 + local_2c) = 0;
    (g_00946138)[param_1] = 0;
    *(int *)((char *)g_0096cddc + local_2c) = 0xfffffc19;
    if ((uVar8 & 0x800000) != 0) {
      iVar24 = rand();
      *(int *)((char *)g_0096cdbc + iVar6) = iVar24 % 3 + -1;
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000001;
      if ((int)uVar8 < 0) {
        uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
      }
      *(unsigned int *)((char *)g_0096cdcc + iVar6) = uVar8;
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000001;
      if ((int)uVar8 < 0) {
        uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
      }
      *(unsigned int *)((char *)g_0096cdc4 + iVar6) = uVar8;
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000001;
      if ((int)uVar8 < 0) {
        uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
      }
      *(unsigned int *)((char *)g_0096cdc8 + iVar6) = uVar8;
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000001;
      if ((int)uVar8 < 0) {
        uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
      }
      *(unsigned int *)((char *)g_0096cdc0 + iVar6) = uVar8;
    }
    if (((*g_009a649c) & 0x40) != 0) {
      *(int *)((char *)g_0096cdbc + iVar6) = 1;
    }
    *(int *)((int)(char *)g_0096da38 + iVar6) = 0;
    *(int *)((int)(char *)g_0096da3c + iVar6) = 0;
    *(int *)((int)(char *)g_0096da40 + iVar6) = 0;
    *(int *)((int)(char *)g_0096da44 + iVar6) = 0;
    *(int *)((int)(char *)g_0096da48 + iVar6) = 0;
    *(int *)((int)(char *)g_0096da4c + iVar6) = 0;
    *(int *)((char *)g_0096cbbc + iVar6) = 0;
    puVar9 = (int *)((char *)g_0096cbe0 + iVar6);
    iVar24 = 8;
    do {
      puVar9[-8] = 0;
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
      iVar24 = iVar24 + -1;
    } while (iVar24 != 0);
    *(int *)((char *)g_0096cdb8 + iVar6) = 0;
    iVar24 = 0;
    puVar16 = (short *)((int)(char *)g_0096dc5c + iVar6);
    do {
      puVar16[-0x100] = 0;
      ((unsigned char *)g_0096db5c)[iVar24 + iVar6] = 0;
      ((unsigned char *)g_0096dbdc)[iVar24 + iVar6] = 0;
      *puVar16 = 0;
      puVar16[0x80] = 0;
      iVar24 = iVar24 + 1;
      puVar16 = puVar16 + 1;
    } while (iVar24 < 0x80);
    puVar9 = (int *)((int)(char *)g_0096ce0c + iVar6);
    iVar24 = 0xb;
    do {
      puVar9[-0xb] = 0;
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
      iVar24 = iVar24 + -1;
    } while (iVar24 != 0);
    puVar9 = (int *)((int)(char *)g_0096ea5c + iVar6);
    for (iVar24 = 10; iVar24 != 0; iVar24 = iVar24 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    *(int *)((int)(char *)g_0096ea1c + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea20 + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea24 + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea28 + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea2c + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea30 + iVar6) = 0;
    *(int *)((int)(char *)g_0096ea34 + iVar6) = 1;
  }
  iVar24 = local_2c;
  iVar6 = (*g_009a64d4);
  iVar19 = 100;
  if ((*g_0093f660) == 0) {
    iVar19 = (*g_009497b4);
  }
  puVar9 = (int *)((char *)g_0096ca78 + local_2c);
  local_10 = param_1 * 0x59c;
  *(int *)((int)(char *)g_0096cc00 + local_2c) =
       *(int *)((int)(char *)g_0096cc00 + local_2c) + *(int *)((char *)g_00946f38 + local_10) + iVar19;
  *(int *)((char *)g_0096cc04 + local_2c) = 0;
  *(int *)((char *)g_0096cdb0 + local_2c) = 0xffffffff;
  *(int *)((char *)g_0096c9f4 + local_2c) = iVar6;
  local_14 = 0;
  do {
    uVar8 = (*g_009a649c);
    if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
      puVar9[-0x20] = 0;
      *puVar9 = 0;
      iVar6 = 5;
      if ((uVar8 & 0x40) == 0) {
        iVar6 = (*g_009a64c4);
      }
      local_30 = iVar6 * 3 + 8;
      if (local_30 == 1 || iVar6 * 3 + 7 < 0) {
LAB_005b1483:
        iVar6 = 1;
      }
      else {
        iVar6 = rand();
        iVar6 = iVar6 % local_30;
        if (iVar6 < 1) goto LAB_005b1483;
        if (0x10 < iVar6) {
          iVar6 = 0x10;
        }
      }
      puVar9[-0x10] = iVar6;
      puVar9[0x25] = 0;
      puVar9[0x2d] = 0;
      puVar9[0x35] = 0;
      ((unsigned char *)g_0096ca98)[local_14 + iVar24] = 0;
      puVar9[-8] = 0xffffffff;
      puVar9[0x45] = 0;
      puVar9[0x3d] = 0;
    }
    else {
      treaty_off(param_1,local_14,0x9000ff48);
    }
    puVar9[0x1b] = 0;
    puVar9[0xb] = 0;
    puVar9[0x13] = 0;
    local_14 = local_14 + 1;
    puVar9 = puVar9 + 1;
  } while ((int)local_14 < 8);
  iVar6 = 0;
  do {
    iVar19 = (*g_009a64d4);
    ((unsigned char *)g_0096d238)[iVar6 + iVar24] = 0;
    ((unsigned char *)g_0096d438)[iVar6 + iVar24] = 0;
    if (iVar19 == 0) {
      ((short *)g_0096d638)[param_1 * 0x1066 + iVar6] = 0;
    }
    iVar6 = iVar6 + 1;
  } while (iVar6 < 0x200);
  if ((iVar19 == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
    puVar9 = (int *)((char *)g_0096cd0c + iVar24);
    for (iVar6 = 0xc; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    puVar9 = (int *)((char *)g_0096cd4c + iVar24);
    for (iVar6 = 0x16; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    *(unsigned char *)puVar9 = 0;
    *(int *)((char *)g_0096ce38 + iVar24) = 0;
    *(int *)((char *)g_0096ce3c + iVar24) = 0;
    *(unsigned char *)((int)(char *)g_0096ce3c + iVar24 + 4) = 0;
    *(int *)((int)(char *)g_0096d1e8 + iVar24) = 0;
    *(int *)((int)(char *)g_0096d1ec + iVar24) = 0;
    *(int *)((char *)g_0096d1f0 + iVar24) = 0;
    *(int *)((int)(char *)g_0096d1f4 + iVar24) = 0;
    *(int *)((char *)g_0096cd48 + iVar24) = 0;
    *(int *)((char *)g_0096cd3c + iVar24) = 0;
    *(int *)((int)(char *)g_0096cd44 + iVar24) = 0;
    *(int *)((char *)g_0096cdac + iVar24) = 0xffffffff;
    *(int *)((char *)g_0096cda8 + iVar24) = 0;
    *(int *)((int)(char *)g_0096cdb4 + iVar24) = 0;
    *(int *)((char *)g_0096cd40 + iVar24) = 0;
    *(int *)((int)(char *)g_0096ea68 + iVar24) = 0;
  }
  *(int *)((char *)g_0096cdd8 + iVar24) = 0;
  *(int *)((char *)g_0096ccdc + iVar24) = 0;
  *(int *)((char *)g_0096cce0 + iVar24) = 0;
  *(int *)((char *)g_0096ccf0 + iVar24) = 0;
  *(int *)((char *)g_0096ccec + iVar24) = 0;
  *(int *)((char *)g_0096cce4 + iVar24) = 0;
  *(int *)((char *)g_0096ccf4 + iVar24) = 0;
  *(int *)((char *)g_0096ccf8 + iVar24) = 0;
  *(int *)((char *)g_0096ccfc + iVar24) = 0;
  *(int *)((char *)g_0096cd00 + iVar24) = 0;
  *(int *)((char *)g_0096cce8 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d1f8 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d204 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d208 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d1fc + iVar24) = 0;
  *(int *)((int)(char *)g_0096d200 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d20c + iVar24) = 0;
  *(int *)((int)(char *)g_0096d210 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d214 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d218 + iVar24) = 0;
  *(int *)((int)(char *)g_0096d21c + iVar24) = 0;
  if (param_1 == 0) {
    return 0;
  }
  local_40 = 1 << (bVar14 & 0x1f);
  if (((*(unsigned char *)g_009a64e9) & (unsigned char)local_40) == 0) {
    return 0;
  }
  if (((int)param_2 < -1) && (param_2 != (unsigned char *)0xfffffee6)) {
    (*(unsigned char *)g_009a64e9) = (*(unsigned char *)g_009a64e9) & ~('\x01' << (bVar14 & 0x1f));
    return 0;
  }
  local_18 = 0;
LAB_005b1688:
  iVar24 = 0x400 - local_18;
  iVar6 = iVar24 * (*g_00949870);
  uVar8 = (int)(iVar6 + (iVar6 >> 0x1f & 0x1fffU)) >> 0xd;
  if ((int)uVar8 < 0) {
    uVar8 = 0;
  }
  else if (9999 < (int)uVar8) {
    uVar8 = 9999;
  }
  if (((*(unsigned char *)g_0094988c) & 1) == 0) {
    iVar6 = 0;
  }
  else {
    iVar6 = uVar8 * 2;
  }
  iVar6 = (*g_00949870) - iVar6;
  if (iVar6 == 1 || iVar6 + -1 < 0) {
    iVar19 = 0;
  }
  else {
    iVar19 = rand();
    iVar19 = iVar19 % iVar6;
  }
  iVar24 = iVar24 * (*g_00949874);
  local_8 = (-(unsigned int)(((*(unsigned char *)g_0094988c) & 1) != 0) & uVar8) + iVar19;
  iVar6 = (int)(iVar24 + (iVar24 >> 0x1f & 0x1fffU)) >> 0xd;
  if (iVar6 < 0) {
    iVar6 = 0;
  }
  else if (9999 < iVar6) {
    iVar6 = 9999;
  }
  iVar24 = (*g_00949874) + iVar6 * -2;
  if (iVar24 == 1 || iVar24 + -1 < 0) {
    iVar19 = 0;
  }
  else {
    iVar19 = rand();
    iVar19 = iVar19 % iVar24;
  }
  local_c = iVar19 + iVar6;
  if ((local_8 & 1) != 0) {
    local_8 = local_8 - 1;
  }
  if ((local_c & 1) != 0) {
    local_8 = local_8 + 1;
  }
  local_34 = 0x20;
  iVar6 = 0;
  do {
    if (iVar6 == 0) {
      iVar24 = 0;
    }
    else {
      iVar24 = (g_0066f8c0)[iVar6];
    }
    iVar19 = iVar6;
    if (iVar24 < (int)(g_0066f8c4)[iVar6]) {
      do {
        iVar10 = (g_0066efbc)[iVar24] + local_8;
        if (((*(unsigned char *)g_0094988c) & 1) == 0) {
          if (iVar10 < 0) {
            iVar10 = iVar10 + (*g_00949870);
          }
          else if ((*g_00949870) <= iVar10) {
            iVar10 = iVar10 - (*g_00949870);
          }
        }
        iVar17 = (g_0066f440)[iVar24] + local_c;
        if (((((-1 < iVar17) && (iVar17 < (*g_00949874))) && (-1 < iVar10)) &&
            ((iVar10 < (*g_00949870) &&
             (uVar8 = *(unsigned int *)(((*g_0068faf0) * iVar17 + (iVar10 >> 1)) * 0x2c + 0xc + (*g_0094a30c))
             , uVar8 != 0)))) && (0 < (int)(uVar8 & 0xff000000))) {
          iVar19 = 99;
          local_34 = iVar6;
          break;
        }
        iVar24 = iVar24 + 1;
      } while (iVar24 < (int)(g_0066f8c4)[iVar6]);
    }
    iVar6 = iVar19 + 1;
  } while (iVar6 < 8);
  iVar6 = 0;
  local_24 = (*(unsigned char *)g_0094988c) & 1;
  local_1c = 0;
  do {
    iVar24 = *(int *)((int)(char *)g_0066efbc + iVar6) + local_8;
    if (local_24 == 0) {
      if (iVar24 < 0) {
        iVar24 = iVar24 + (*g_00949870);
      }
      else if ((*g_00949870) <= iVar24) {
        iVar24 = iVar24 - (*g_00949870);
      }
    }
    iVar19 = *(int *)((int)(char *)g_0066f440 + iVar6) + local_c;
    if (((-1 < iVar19) && (iVar19 < (*g_00949874))) && ((-1 < iVar24 && (iVar24 < (*g_00949870))))) {
      if ((((unsigned char *)g_00946f59)[local_10] & 1) == 0) {
        iVar24 = ((*g_0068faf0) * iVar19 + (iVar24 >> 1)) * 0x2c;
        pbVar21 = (unsigned char *)(iVar24 + (*g_0094a30c));
        if (((((*(unsigned char *)(iVar24 + 8 + (*g_0094a30c)) & 0x20) == 0) || ((*pbVar21 & 0xe0) < 0x40)) &&
            (0x5f < (*pbVar21 & 0xe0))) &&
           (((pbVar21[5] & 0xc0) < 0x41 && ((*pbVar21 & 0x18) != 0)))) goto LAB_005b1914;
      }
      else {
        iVar24 = ((*g_0068faf0) * iVar19 + (iVar24 >> 1)) * 0x2c;
        pbVar21 = (unsigned char *)(iVar24 + (*g_0094a30c));
        if (((*(unsigned char *)(iVar24 + 8 + (*g_0094a30c)) & 0x20) == 0) || ((*pbVar21 & 0xe0) < 0x40)) {
          local_1c = local_1c + (2 - (unsigned int)(*pbVar21 >> 5)) * 2;
LAB_005b1914:
          local_1c = local_1c + 1;
        }
      }
    }
    iVar6 = iVar6 + 4;
  } while (iVar6 < 0x54);
  local_14 = *(unsigned int *)((char *)g_00946f58 + local_10) & 0x100;
  if (local_14 != 0) {
    local_1c = local_1c / 3;
  }
  pcVar26 = (char *)((int)local_8 >> 1);
  iVar6 = 1;
  local_28 = (unsigned int)*(unsigned char *)((int)(pcVar26 + (*g_0068faf0) * local_c) * 0x2c + 3 + (*g_0094a30c));
  local_38 = 0;
  pcVar20 = (char *)g_0096fc28 + local_28;
  do {
    if (*pcVar20 != '\0') {
      local_38 = local_38 | 1 << ((unsigned char)iVar6 & 0x1f);
    }
    iVar6 = iVar6 + 1;
    pcVar20 = pcVar20 + 0x20cc;
  } while (iVar6 < 8);
  local_20 = pcVar26;
  if ((local_14 == 0) && (param_2 != (unsigned char *)0xfffffee6)) {
    local_30 = base_find(local_8,local_c,0xffffffff,local_28,0xffffffff,0xffffffff);
  }
  else {
    local_30 = base_find(local_8,local_c);
  }
  if (local_30 < 0) {
    (*g_0090ea04) = 0x20;
  }
  if ((((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) && (local_24 = 0, 0 < (*g_009a64c8)))
  {
    psVar23 = (short *)g_0095282a;
    do {
      sVar2 = *psVar23;
      if (*(unsigned char *)(((*g_0068faf0) * sVar2 + ((int)psVar23[-1] >> 1)) * 0x2c + 3 + (*g_0094a30c)) ==
          local_28) {
        local_38 = local_38 | 1 << (*(unsigned char *)(psVar23 + 6) & 0x1f);
        iVar6 = abs(local_8 - (int)psVar23[-1]);
        if ((((*(unsigned char *)g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar6)) {
          iVar6 = (*g_00949870) - iVar6;
        }
        uVar7 = abs(local_c - (int)sVar2);
        iVar24 = abs(iVar6);
        iVar19 = abs(uVar7);
        iVar6 = iVar24;
        if (iVar24 <= iVar19) {
          iVar6 = iVar19;
        }
        iVar10 = iVar24;
        if (iVar19 <= iVar24) {
          iVar10 = iVar19;
        }
        iVar6 = iVar6 - (((iVar24 + iVar19 >> 1) - iVar10) + 1 >> 1);
        if (iVar6 < (*g_0090ea04)) {
          (*g_0090ea04) = iVar6;
        }
      }
      local_24 = local_24 + 1;
      psVar23 = psVar23 + 0x1a;
      pcVar26 = local_20;
    } while ((int)local_24 < (*g_009a64c8));
  }
  uVar8 = local_28;
  local_18 = local_18 + 1;
  if (((int)local_18 < 0x401) ||
     ((((unsigned char)(*g_009a64e8) & (unsigned char)local_40) != 0 && ((*g_009a64d4) == 0)))) {
    local_14 = *(unsigned int *)((char *)g_00946f58 + local_10) & 0x100;
    if (local_14 == 0) {
      pbVar21 = (unsigned char *)((int)(pcVar26 + (*g_0068faf0) * local_c) * 0x2c + (*g_0094a30c));
      bVar15 = *pbVar21 & 0xe0;
      bVar27 = bVar15 < 0x60;
    }
    else {
      pbVar21 = (unsigned char *)((int)(pcVar26 + (*g_0068faf0) * local_c) * 0x2c + (*g_0094a30c));
      bVar15 = *pbVar21 & 0xe0;
      bVar27 = 0x40 < bVar15;
    }
    if (((bVar27) || (((pbVar21[8] & 0x20) != 0 && (0x3f < bVar15)))) ||
       ((local_14 == 0 && (0x40 < (pbVar21[5] & 0xc0))))) goto LAB_005b1688;
    if (((*g_009a64d4) == 0) || (iVar6 = 0xc, param_2 == (unsigned char *)0xfffffee6)) {
      iVar6 = 6;
    }
    if ((((((iVar6 * (*g_00949888)) / 0x38 <= (*g_0090ea04)) && (bVar15 < 0xa0)) &&
         (((pbVar21[8] & 3) == 0 || (7 < (pbVar21[2] & 0xf))))) &&
        ((((1 < (int)local_c && ((int)local_c < (*g_00949874) + -2)) &&
          (uVar1 = (int)local_18 >> 0x1f, 0xc - (int)local_18 / 0x50 <= local_1c)) &&
         (local_1c = (int)(local_18 + (uVar1 & 0x3f)) >> 6,
         ((0x10 - local_1c) * (*g_00949888)) / 0x38 <= (*g_0090ea04))))) &&
       ((((*g_009a64d4) != 0 && (param_2 != (unsigned char *)0xfffffee6)) ||
        ((899 < (int)local_18 || (0x24 - local_1c <= *(int *)((char *)g_009aa730 + local_28 * 0x1c)))))))
    {
      if (local_14 == 0) {
        if (((unsigned char)local_40 & (unsigned char)(*g_009a64e8)) == 0) {
          iVar6 = 8;
        }
        else {
          iVar6 = (6 - (*g_009a64c4)) * 8;
        }
        if (((g_009aa734)[local_28 * 7] * ((*g_0094a2a4) + 1)) / 2 <
            (-((int)((*g_009a64d4) + ((*g_009a64d4) >> 0x1f & 0xfU)) >> 4) -
            ((int)(local_18 + (uVar1 & 0xf)) >> 4)) + 0x18 + iVar6) goto LAB_005b1688;
      }
      iVar6 = (*g_009a64c4);
      if (((unsigned char)local_40 & (unsigned char)(*g_009a64e8)) == 0) {
        iVar6 = 3;
      }
      iVar24 = local_28 * 0x1c;
      if ((((5 - iVar6) * 0x30 - ((int)((*g_009a64d4) + ((*g_009a64d4) >> 0x1f & 0xfU)) >> 4)) -
           ((int)(local_18 + (uVar1 & 3)) >> 2) <= *(int *)((char *)g_009aa730 + iVar24)) &&
         (((local_38 == 0 || (param_2 == (unsigned char *)0xfffffee6)) ||
          (iVar6 = bit_count(local_38),
          0x40 - local_1c <= *(int *)((char *)g_009aa730 + iVar24) / (iVar6 + 1))))) {
        uVar5 = local_8;
        uVar1 = local_c;
        if ((*(unsigned int *)((int)(local_20 + (*g_0068faf0) * local_c) * 0x2c + 8 + (*g_0094a30c)) & 0x1000)
            == 0) {
          uVar25 = (unsigned int)(*(unsigned char *)((int)(local_20 + (*g_0068faf0) * local_c) * 0x2c + (*g_0094a30c) +
                                   2) >> 4);
          if (uVar25 == 0) {
            uVar25 = world_site(local_8,local_c,0);
            site_set(uVar5,uVar1,uVar25);
            uVar8 = local_28;
          }
        }
        else {
          uVar25 = 0;
        }
        if (((((0xf - local_1c <= (int)uVar25) || ((*g_009a64d4) != 0)) ||
             (((*g_009a649c) & 0x10) != 0)) &&
            (((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) != 0 ||
             (((int)((0x400 - local_18) + ((int)(0x400 - local_18) >> 0x1f & 0x7fU)) >> 7 <=
               local_34 && (local_34 != 0)))))) &&
           ((((*g_009a64d4) == 0 ||
             ((param_2 == (unsigned char *)0xfffffee6 ||
              (*(char *)((int)(local_20 + (*g_0068faf0) * uVar1) * 0x2c + 7 + (*g_0094a30c)) < '\x01')
              ))) && ((local_30 < 0 ||
                      (((unsigned char *)(unsigned int)(unsigned char)(g_0097d044)[local_30 * 0x134] != param_2 ||
                       (*(unsigned char *)(((short)(g_0097d042)[local_30 * 0x9a] * (*g_0068faf0) +
                                  ((int)(short)(g_0097d040)[local_30 * 0x9a] >> 1)) * 0x2c + 3 +
                                 (*g_0094a30c)) != uVar8)))))))) goto LAB_005b1ef1;
      }
    }
    goto LAB_005b1688;
  }
  local_3c = 1;
LAB_005b1ef1:
  iVar6 = (*g_00949870);
  if ((*(unsigned int *)((char *)g_0096c9e0 + local_2c) & 0x1000) == 0) {
    if (local_3c != 0) goto LAB_005b1f16;
LAB_005b1f1e:
    if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
      local_14 = 0;
      local_20 = (char *)0xffffffff;
      local_18 = 0;
      do {
        local_1c = (g_0066efbc)[local_18] + local_8;
        if (((*(unsigned char *)g_0094988c) & 1) == 0) {
          if (local_1c < 0) {
            local_1c = local_1c + iVar6;
          }
          else if (iVar6 <= local_1c) {
            local_1c = local_1c - iVar6;
          }
        }
        iVar24 = (g_0066f440)[local_18] + local_c;
        local_30 = iVar24;
        if ((((-1 < iVar24) && (iVar24 < (*g_00949874))) && (-1 < local_1c)) && (local_1c < iVar6)) {
          iVar19 = local_1c >> 1;
          pbVar21 = (unsigned char *)(((*g_0068faf0) * iVar24 + iVar19) * 0x2c + (*g_0094a30c));
          bVar15 = *pbVar21 & 0xe0;
          iVar6 = (*g_00949870);
          local_34 = iVar19;
          if (((((unsigned int)(bVar15 < 0x60) == (*(unsigned int *)((char *)g_00946f58 + local_10) >> 8 & 1)) &&
               (pbVar21[3] == local_28)) &&
              ((bVar15 < 0xa0 && (((pbVar21[8] & 0x20) == 0 || (bVar15 < 0x40)))))) &&
             (((pbVar21[8] & 3) == 0 || (7 < (pbVar21[2] & 0xf))))) {
            base_find(local_1c,iVar24);
            local_24 = 0;
            if (0 < (*g_009a64c8)) {
              psVar23 = (short *)g_00952828;
              do {
                sVar2 = psVar23[1];
                iVar6 = abs(local_8 - (int)*psVar23);
                if ((((*(unsigned char *)g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar6)) {
                  iVar6 = (*g_00949870) - iVar6;
                }
                uVar7 = abs(local_c - (int)sVar2);
                iVar24 = abs(iVar6);
                iVar19 = abs(uVar7);
                iVar6 = iVar24;
                if (iVar24 <= iVar19) {
                  iVar6 = iVar19;
                }
                iVar10 = iVar24;
                if (iVar19 <= iVar24) {
                  iVar10 = iVar19;
                }
                iVar6 = iVar6 - (((iVar24 + iVar19 >> 1) - iVar10) + 1 >> 1);
                if (iVar6 < (*g_0090ea04)) {
                  (*g_0090ea04) = iVar6;
                }
                local_24 = local_24 + 1;
                psVar23 = psVar23 + 0x1a;
                iVar24 = local_30;
                iVar19 = local_34;
              } while ((int)local_24 < (*g_009a64c8));
            }
            iVar10 = local_1c;
            iVar6 = (*g_00949870);
            if (5 < (*g_0090ea04)) {
              iVar6 = ((*g_0068faf0) * iVar24 + iVar19) * 0x2c;
              if ((*(unsigned int *)(iVar6 + 8 + (*g_0094a30c)) & 0x1000) == 0) {
                pcVar20 = (char *)(unsigned int)(*(unsigned char *)(iVar6 + (*g_0094a30c) + 2) >> 4);
                if (pcVar20 == (char *)0x0) {
                  pcVar20 = (char *)world_site(local_1c,iVar24,0);
                  site_set(iVar10,iVar24,(int)pcVar20);
                }
              }
              else {
                pcVar20 = (char *)0x0;
              }
              local_24 = (*(unsigned char *)g_0094988c) & 1;
              iVar6 = 0;
              do {
                iVar19 = *(int *)((int)(char *)g_0066ef50 + iVar6) + local_1c;
                if (local_24 == 0) {
                  if (iVar19 < 0) {
                    iVar19 = iVar19 + (*g_00949870);
                  }
                  else if ((*g_00949870) <= iVar19) {
                    iVar19 = iVar19 - (*g_00949870);
                  }
                }
                iVar10 = *(int *)((int)(char *)g_0066ef74 + iVar6) + iVar24;
                if ((((-1 < iVar10) && (iVar10 < (*g_00949874))) && (-1 < iVar19)) &&
                   (iVar19 < (*g_00949870))) {
                  if ((((unsigned char *)g_00946f59)[local_10] & 1) == 0) {
                    if ((*(unsigned char *)(((*g_0068faf0) * iVar10 + (iVar19 >> 1)) * 0x2c + (*g_0094a30c)) &
                        0xe0) < 0x60) {
                      pcVar20 = pcVar20 + 1;
                      break;
                    }
                  }
                  else {
                    pcVar20 = pcVar20 + (2 - (unsigned int)(*(unsigned char *)(((*g_0068faf0) * iVar10 + (iVar19 >> 1)
                                                              ) * 0x2c + (*g_0094a30c)) >> 5));
                  }
                }
                iVar6 = iVar6 + 4;
              } while (iVar6 < 0x20);
              iVar6 = (*g_00949870);
              if ((int)local_20 < (int)pcVar20) {
                local_14 = local_18;
                local_20 = pcVar20;
              }
            }
          }
        }
        local_18 = local_18 + 1;
      } while ((int)local_18 < 0x15);
      local_8 = (g_0066efbc)[local_14] + local_8;
      if (((*(unsigned char *)g_0094988c) & 1) == 0) {
        if ((int)local_8 < 0) {
          local_8 = local_8 + iVar6;
        }
        else if (iVar6 <= (int)local_8) {
          local_8 = local_8 - iVar6;
        }
      }
      local_c = local_c + (g_0066f440)[local_14];
    }
    if (param_3 == 0) goto LAB_005b227e;
    local_3c = 0;
LAB_005b2283:
    if (((((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) || (((*g_009a649c) & 1) == 0)) &&
       ((*g_009a64d4) < 0x65)) goto LAB_005b25e1;
  }
  else {
    local_3c = 1;
LAB_005b1f16:
    if (param_3 != 0) goto LAB_005b1f1e;
LAB_005b227e:
    if (local_3c == 0) goto LAB_005b2283;
  }
  uVar8 = (*g_009a6508);
  if (param_3 == 0) {
    if ((((unsigned char)(*g_009a64e8) & (unsigned char)local_40) != 0) && ((*g_0093f660) == 0)) {
      (*g_009b206c) = 7;
      (*(unsigned char *)g_009a64e9) = (*(unsigned char *)g_009a64e9) & ~('\x01' << (bVar14 & 0x1f));
      (*g_009a64c0) = (*g_009a64c0) | 1;
      return 0;
    }
    *(int *)((int)(char *)g_0096ea68 + local_2c) = 0;
    (*(unsigned char *)g_009a64e9) = (*(unsigned char *)g_009a64e9) & ~('\x01' << (bVar14 & 0x1f));
    local_14 = uVar8;
    if (param_1 == uVar8) {
      local_14 = (*g_009a6504);
    }
    iVar6 = 0;
    param_3 = 0;
    uVar18 = 1;
    pbVar21 = (unsigned char *)((char *)g_0096eac4 + local_14);
    puVar13 = (unsigned int *)((char *)g_0096eac4 + (*g_00939284));
    // INDEXED FROM THE BASE, NOT MARCHED. Ghidra rendered VC6's own strength
    // reduction back as source here: two pointers advancing by 0x833 and
    // 0x20cc, dereferenced inside a six-deep short-circuit condition. Handed
    // back to VC6, /Og re-analyses its own output and never finishes - this
    // one loop was the whole of a compile that burned 64 minutes without
    // producing an object, while /Od and #pragma optimize("g",off) both built
    // the same file in 0s. Measured: delete the loop and the 3,135-line unit
    // compiles in 1s; keep the condition but index instead of marching and it
    // also compiles in 1s. Neither the condition nor the increments alone do
    // it - only the two together.
    //
    // Indexing is also the likelier original: the optimiser produces marching
    // pointers FROM subscripts, so subscripts are what the author wrote.
    // puVar13/pbVar21 are dead after this loop (both reassigned before their
    // next use), so dropping the increments changes nothing observable.
    do {
      if (((((((unsigned int)(*(unsigned char *)g_009a64e9) & 1 << ((unsigned char)uVar18 & 0x1f)) != 0) &&
            (((iVar24 = iVar6 + 1,
               (*g_0093f660) != 0 || ((puVar13[(uVar18 - 1) * 0x833] & 1) == 0)) ||
             ((puVar13[(uVar18 - 1) * 0x833] & 0x2000000) == 0)))) &&
           ((iVar6 = iVar24,
             uVar18 != local_14 && ((pbVar21[(uVar18 - 1) * 0x20cc] & 1) != 0)))) &&
          (((*g_009a649c) & 0x1000) != 0)) && (param_3 < 2)) {
        param_3 = param_3 + 1;
      }
      uVar18 = uVar18 + 1;
    } while ((int)uVar18 < 8);
    if (param_1 == (*g_00939284)) {
      uVar18 = ((int)param_2 < 0) - 1 & (unsigned int)param_2;
      (*g_009bbfec) = *(int *)((char *)g_00946d4c + uVar18 * 0x167);
      (*g_009bbff0) = *(int *)((char *)g_00946d50 + uVar18 * 0x167);
      parse_says(0,(char *)g_00946d34 + uVar18 * 0x59c,0xffffffff,0xffffffff);
      if ((*g_0093f660) != 0) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->hang_up();
        (*g_0093e8c0) = 0;
        (*g_0093e8c4) = 0;
        (*g_0093e8c8) = 0;
        (*g_0093e8cc) = 0;
        (*g_0093e8f0) = 0;
        (*g_0093e8ec) = 0;
        (*g_0093e8fc) = 0;
        (*g_0093e8f8) = 0;
        (*g_0093e90c) = 0;
        (*g_0093e938) = 0;
        (*g_0093e930) = 0;
        (*g_0093e934) = 0;
        (*g_0093e95c) = 0;
        (*g_0093e958) = 0;
        (*g_0093e8e0) = 0;
        (*g_0093e8e4) = 0;
        (*g_0093e8e8) = 0;
        (*g_0093e954) = 0;
        (*g_0093e950) = 0;
        (*g_0093e940) = 0;
        (*g_0093e93c) = 0;
        (*g_0093e944) = 0;
        (*g_0093e8d0) = 0;
        (*g_0093e8d4) = 0;
        (*g_0093e968) = 0;
        (*g_0093e964) = 0;
        (*g_0093e8d8) = 0;
        (*g_0093e8dc) = 0;
        (*g_0093e960) = 0xff;
        (*g_0093e908) = 0;
        puVar9 = g_0093e910;
        for (iVar24 = 8; iVar24 != 0; iVar24 = iVar24 + -1) {
          *puVar9 = 0;
          puVar9 = puVar9 + 1;
        }
        reinterpret_cast<Lock *>(g_0093e230)->clear();
        reinterpret_cast<AlphaNet *>(g_0093cd90)->close();
        (*g_0093f660) = 0;
        (*g_009b2068) = 1;
      }
      if ((param_2 == (unsigned char *)0x0) && ((*g_009bc054) == 0)) {
        pcVar20 = (char *)g_0069152c;
      }
      else {
        pcVar20 = (char *)g_00691524;
      }
      X_pop(pcVar20,0);
      if (param_1 == (*g_00939284)) {
        (*g_009a64c0) = (*g_009a64c0) | 1;
        (*g_009b206c) = (-(unsigned int)(param_2 != (unsigned char *)0x0) & 0xfffffff8) + 0xf;
        return 0;
      }
    }
    if (iVar6 - param_3 < 2) {
      if (param_3 == 0) {
        if ((local_14 == (*g_00939284)) &&
           (popp((*(char **)g_00691b0c),(char *)g_0069155c,0,(char *)g_00691550,0),
           (*g_009b206c) == 0)) {
          (*g_009b206c) = 4;
        }
      }
      else {
        popp((*(char **)g_00691b0c),(char *)g_00691544,0,(char *)g_00691538,0);
        if ((*g_009b206c) == 0) {
          (*g_009b206c) = 5;
        }
      }
      (*g_009a64c0) = (*g_009a64c0) | 0x2001;
    }
    return 0;
  }
LAB_005b25e1:
  iVar19 = local_2c;
  iVar24 = (*g_009a64d4);
  *(int *)((char *)g_0096cd08 + local_2c) = 5;
  *(int *)((char *)g_0096cd04 + local_2c) = 0;
  if (((iVar24 == 0) || (param_2 == (unsigned char *)0xfffffee6)) && (1 < (*g_009a64c4))) {
    if ((*(int *)((char *)g_0096cdc8 + local_2c) != 0) || (*(int *)((char *)g_0096cdc4 + local_2c) != 0)) {
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000003;
      bVar27 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar27) {
        *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 8;
      }
    }
    (*(char *)g_009b86a0) = 0;
    strcat((char *)g_009b86a0,(char *)g_00946a54 + local_10);
    (*reinterpret_cast<char *(__stdcall **)(char *)>(g_0066931c))((char *)g_009b86a0);
    iVar6 = strcmp((char *)g_009b86a0,(char *)g_00691568);
    if (iVar6 == 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 8;
    }
    uVar8 = *(unsigned int *)((char *)g_00946f58 + local_10);
    if ((uVar8 & 0x100) != 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 4;
      *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x8000000;
    }
    if ((uVar8 & 0x80) != 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 0x10;
      *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x10000000;
    }
    iVar6 = strcmp((char *)g_009b86a0,(char *)g_00691570);
    if (iVar6 == 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) & 0xfffffff7 | 2;
    }
    iVar24 = local_28 * 0x1c;
    if ((*(int *)((char *)g_009aa730 + local_28 * 0x1c) < 0xfa) && (iVar6 = rand(), iVar6 % 3 == 0)) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 4;
    }
    if (-1 < *(int *)((char *)g_0096cdbc + iVar19)) {
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000003;
      bVar27 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar27) {
        *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 0x10;
      }
    }
    iVar6 = rand();
    if (iVar6 % 6 == 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 2;
    }
    iVar6 = rand();
    if (iVar6 % 6 == 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 1;
    }
    iVar6 = rand();
    if (iVar6 % 5 == 0) {
      *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 0x40;
    }
    if (((((*g_009a649c) & 0x40) != 0) || (3 < (*g_009a64c4))) &&
       ((int)-(unsigned int)(((*g_009a649c) & 0x40) != 0) < *(int *)((char *)g_0096cdbc + iVar19))) {
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000003;
      bVar27 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar27) {
        *(unsigned int *)((char *)g_0096ea70 + iVar19) = *(unsigned int *)((char *)g_0096ea70 + iVar19) | 0x20;
      }
    }
    if (((((*g_009a649c) & 0x40) != 0) || (3 < (*g_009a64c4))) &&
       ((int)-(unsigned int)(((*g_009a649c) & 0x40) != 0) < *(int *)((char *)g_0096cdbc + iVar19))) {
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000003;
      bVar27 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      if (bVar27) {
        *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x1000000;
        uVar8 = rand();
        uVar8 = uVar8 & 0x80000001;
        bVar27 = uVar8 == 0;
        if ((int)uVar8 < 0) {
          bVar27 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
        }
        if (bVar27) {
          *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x2000000;
        }
      }
    }
    if ((*(int *)((char *)g_0096cdcc + iVar19) != 0) ||
       (iVar6 = (*g_00949870), 0 < *(int *)((char *)g_0096cdbc + iVar19))) {
      if (399 < *(int *)((char *)g_009aa730 + iVar24)) {
        uVar8 = rand();
        uVar8 = uVar8 & 0x80000003;
        bVar27 = uVar8 == 0;
        if ((int)uVar8 < 0) {
          bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
        }
        if (bVar27) {
          *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x4000000;
        }
      }
      uVar8 = (0xfa < *(int *)((char *)g_009aa730 + iVar24)) - 1 & 0xfffffffe;
      if ((uVar8 == 0xfffffffb || (int)(uVar8 + 5) < 0) ||
         (iVar6 = rand(), iVar6 % (int)(uVar8 + 6) == 0)) {
        *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x8000000;
      }
      uVar8 = rand();
      uVar8 = uVar8 & 0x80000003;
      bVar27 = uVar8 == 0;
      if ((int)uVar8 < 0) {
        bVar27 = (uVar8 - 1 | 0xfffffffc) == 0xffffffff;
      }
      iVar6 = (*g_00949870);
      if (bVar27) {
        *(unsigned int *)((char *)g_0096c9e0 + iVar19) = *(unsigned int *)((char *)g_0096c9e0 + iVar19) | 0x10000000;
        iVar6 = (*g_00949870);
      }
    }
  }
  param_3 = 0;
  do {
    iVar24 = *(int *)((int)(char *)g_0066efbc + param_3) + local_8;
    if (((*(unsigned char *)g_0094988c) & 1) == 0) {
      if (iVar24 < 0) {
        iVar24 = iVar24 + iVar6;
      }
      else if (iVar6 <= iVar24) {
        iVar24 = iVar24 - iVar6;
      }
    }
    iVar19 = *(int *)((int)(char *)g_0066f440 + param_3) + local_c;
    if ((((-1 < iVar19) && (iVar19 < (*g_00949874))) && (-1 < iVar24)) && (iVar24 < iVar6)) {
      pbVar21 = (unsigned char *)(((*g_0068faf0) * iVar19 + (iVar24 >> 1)) * 0x2c + 4 + (*g_0094a30c));
      *pbVar21 = *pbVar21 | '\x01' << (bVar14 & 0x1f);
      synch_bit(iVar24,iVar19,param_1);
      iVar6 = (*g_00949870);
    }
    param_3 = param_3 + 4;
  } while (param_3 < 0x54);
  param_1 = 2;
  if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
    iVar6 = 0;
    if (0 < *(int *)((int)(char *)g_00946f5c + local_10)) {
      puVar9 = (int *)((int)(char *)g_00946f80 + local_10);
      do {
        if (puVar9[-8] == 0) {
          tech_achieved(uVar18,*puVar9,0,0);
        }
        iVar6 = iVar6 + 1;
        puVar9 = puVar9 + 1;
      } while (iVar6 < *(int *)((int)(char *)g_00946f5c + local_10));
    }
    if (((*g_009a6488) != 0) && (((*(int *)((char *)g_00946f58 + local_10)) & 0x80) != 0)) {
      tech_achieved(uVar18,0x4e,0,0);
      tech_achieved(uVar18,0x4f,0,0);
    }
    if ((*g_0093f660) != 0) {
      local_18 = 0;
      while( true ) {
        if (*(int *)((char *)g_0096ccd8 + local_2c) < 1) {
          iVar6 = (-1 < *(int *)((char *)g_0096ccd8 + local_2c)) - 1;
        }
        else {
          iVar6 = 1;
        }
        if (*(int *)((char *)g_0096ccc0 + local_2c) < 1) {
          iVar24 = (-1 < *(int *)((char *)g_0096ccc0 + local_2c)) - 1;
        }
        else {
          iVar24 = 1;
        }
        if ((iVar6 - iVar24) + 1 <= (int)local_18) break;
        uVar7 = tech_ai(uVar18);
        tech_achieved(uVar18,uVar7,0,0);
        local_18 = local_18 + 1;
      }
      *(int *)((int)(char *)g_0096cc00 + local_2c) =
           *(int *)((int)(char *)g_0096cc00 + local_2c) + *(int *)((char *)g_00946f38 + local_10) / 2;
    }
    consider_designs(uVar18);
    iVar6 = 0;
    puVar11 = (unsigned short *)((char *)g_009ab898 + uVar18 * 0x680);
    do {
      if (((*puVar11 & 1) != 0) && (*puVar11 = *puVar11 | 4, (char)puVar11[-3] == '\x03')) {
        param_1 = iVar6 + uVar18 * 0x40;
      }
      iVar6 = iVar6 + 1;
      puVar11 = puVar11 + 0x1a;
    } while (iVar6 < 0x40);
  }
  iVar6 = local_10;
  if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
    if ((((*g_0093f660) == 0) && (((*g_009a649c) & 0x10) == 0)) || (((*g_009a649c) & 0x80) != 0)) {
      iVar24 = base_init(uVar18,local_8,local_c);
      iVar6 = local_10;
      (*(char *)g_009b86a0) = 0;
      strcat((char *)g_009b86a0,(char *)g_00946a54 + local_10);
      (*reinterpret_cast<char *(__stdcall **)(char *)>(g_0066931c))((char *)g_009b86a0);
      iVar19 = strcmp((char *)g_009b86a0,(char *)g_00691578);
      if ((iVar19 == 0) && (param_2 == (unsigned char *)0xfffffee6)) {
        (g_0097d046)[iVar24 * 0x134] = 3;
      }
      if ((((*(int *)((char *)g_00946f58 + iVar6)) & 0x80) != 0) && (param_2 == (unsigned char *)0xfffffee6)) {
        (g_0097d046)[iVar24 * 0x134] = 3;
      }
    }
    else if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) == 0) {
      veh_init(0,uVar18,local_8,local_c);
    }
    else {
      veh_init(0x15,uVar18,local_8,local_c);
    }
    if ((*(unsigned int *)((char *)g_00946f58 + iVar6) & 0x100) == 0) {
      iVar24 = veh_init(param_1,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar24 * 0x1a) = (short)0xffff;
      veh_demote(iVar24);
    }
    if (((*g_0093f660) != 0) && ((*(unsigned int *)((char *)g_00946f58 + iVar6) & 0x100) == 0)) {
      iVar6 = veh_init(0xb,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
      iVar6 = veh_init(1,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
    }
    iVar6 = is_coast(local_8,local_c,0);
    if ((iVar6 != 0) &&
       (((1 < (*g_009a64c4) || ((*g_0093f660) != 0)) ||
        (*(int *)((char *)g_009aa730 + local_28 * 0x1c) < 0x21)))) {
      uVar8 = (unsigned int)*(unsigned char *)(((*g_0068faf0) * local_c + ((int)local_8 >> 1)) * 0x2c + 3 +
                             (*g_0094a30c));
      local_28 = uVar8;
      if ((((*g_0093f660) != 0) &&
          (iVar24 = bit_count(local_38),
          *(int *)((char *)g_009aa730 + uVar8 * 0x1c) / (iVar24 + 1) < 0x4c)) &&
         (((unsigned char)local_40 & (unsigned char)(*g_009a64e8)) != 0)) {
        iVar24 = (g_0066efbc)[iVar6] + local_8;
        if (((*(unsigned char *)g_0094988c) & 1) == 0) {
          if (iVar24 < 0) {
            iVar24 = (*g_00949870) + iVar24;
          }
          else if ((*g_00949870) <= iVar24) {
            iVar24 = iVar24 - (*g_00949870);
          }
        }
        iVar24 = veh_init(0xd,uVar18,iVar24,(g_0066f440)[iVar6] + local_c);
        (g_0095284c)[iVar24 * 0x34] = 2;
        veh_demote(iVar24);
        *(short *)((char *)g_00952856 + iVar24 * 0x1a) = (short)0xffff;
        uVar8 = local_28;
      }
      iVar24 = bit_count(local_38);
      if (*(int *)((char *)g_009aa730 + uVar8 * 0x1c) / (iVar24 + 1) < 0x33) {
        iVar24 = (g_0066efbc)[iVar6] + local_8;
        if (((*(unsigned char *)g_0094988c) & 1) == 0) {
          if (iVar24 < 0) {
            iVar24 = (*g_00949870) + iVar24;
          }
          else if ((*g_00949870) <= iVar24) {
            iVar24 = iVar24 - (*g_00949870);
          }
        }
        iVar6 = veh_init(0xd,uVar18,iVar24,(g_0066f440)[iVar6] + local_c);
        (g_0095284c)[iVar6 * 0x34] = 2;
        veh_demote(iVar6);
        *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      }
    }
  }
  else {
    if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) == 0) {
      veh_init(0,uVar18,local_8,local_c);
      uVar7 = 1;
    }
    else {
      veh_init(0x15,uVar18,local_8,local_c);
      uVar7 = 4;
    }
    uVar7 = veh_init(uVar7,uVar18,local_8,local_c);
    veh_demote(uVar7);
  }
  if (((*g_009a64d4) == 0) || (param_2 == (unsigned char *)0xfffffee6)) {
    puVar9 = (int *)((char *)g_0096ccb0 + local_2c);
    for (iVar6 = 0xb; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    iVar6 = *(int *)((int)(char *)g_00946f5c + local_10);
    if (0 < iVar6) {
      piVar12 = (int *)((int)(char *)g_00946f80 + local_10);
      iVar24 = iVar6;
      do {
        if (piVar12[-8] == 3) {
          *(int *)((char *)g_0096ccb0 + uVar18 * 0x20cc + *piVar12 * 4) =
               *(int *)((char *)g_0096ccb0 + uVar18 * 0x20cc + *piVar12 * 4) + piVar12[8];
        }
        piVar12 = piVar12 + 1;
        iVar24 = iVar24 + -1;
      } while (iVar24 != 0);
    }
    iVar24 = 0;
    if (0 < iVar6) {
      piVar12 = (int *)((int)(char *)g_00946f80 + local_10);
      do {
        if (piVar12[-8] == 1) {
          iVar6 = veh_init(*piVar12,uVar18,local_8,local_c);
          *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
          veh_demote(iVar6);
          if (*piVar12 == 0) {
            *(unsigned int *)((char *)g_0096c9e0 + local_2c) = *(unsigned int *)((char *)g_0096c9e0 + local_2c) | 0x100;
          }
        }
        iVar24 = iVar24 + 1;
        piVar12 = piVar12 + 1;
      } while (iVar24 < *(int *)((int)(char *)g_00946f5c + local_10));
    }
    if (((*g_009a6488) != 0) && (((*(int *)((char *)g_00946f58 + local_10)) & 0x80) != 0)) {
      iVar6 = veh_init(0x10,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
      iVar6 = veh_init(0,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
    }
    if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) != 0) {
      if (((unsigned char)local_40 & (unsigned char)(*g_009a64e8)) == 0) {
        iVar6 = 0;
        uVar8 = (unsigned int)param_2;
        do {
          uVar1 = uVar18 * 0x40 + iVar6;
          if ((*(unsigned char *)((char *)g_009ab898 + uVar1 * 0x1a) & 1) == 0) {
            iVar6 = 0x40;
            uVar8 = uVar1;
          }
          iVar6 = iVar6 + 1;
        } while (iVar6 < 0x40);
        make_proto(uVar8,3,0x11,0,0,1);
        (g_009ab898)[uVar8 * 0x1a] = (g_009ab898)[uVar8 * 0x1a] & 0xff3f | 4;
        name_proto((char *)local_e0,uVar8,uVar18,3,0x11,0,0,1);
        *(unsigned char *)((char *)g_009ab898 + uVar8 * 0x1a) = *(unsigned char *)((char *)g_009ab898 + uVar8 * 0x1a) & 0xfd;
        local_c1 = 0;
        strcpy((char *)g_009ab868 + uVar8 * 0x34,(char *)local_e0);
      }
      iVar6 = veh_init(0x15,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
      iVar6 = veh_init(0x16,uVar18,local_8,local_c);
      *(short *)((char *)g_00952856 + iVar6 * 0x1a) = (short)0xffff;
      veh_demote(iVar6);
    }
  }
  if ((((*g_009a64d4) != 0) && (local_3c == 0)) && (param_2 != (unsigned char *)0xfffffee6)) {
    local_20 = (char *)0xffffffff;
    local_14 = 2;
    param_3 = -1;
    param_1 = -1;
    local_30 = -1;
    local_34 = -1;
    iVar6 = 0;
    param_2 = (unsigned char *)g_009ab88c;
    do {
      iVar24 = veh_avail(iVar6,uVar18,0xffffffff);
      if ((iVar24 != 0) && (((char *)g_0094ae68)[(unsigned int)param_2[1] * 0x10] != '\0')) {
        if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) == 0) {
          if (((char *)g_0094a379)[(unsigned int)*param_2 * 0x90] == '\0') goto LAB_005b3146;
        }
        else if (((char *)g_0094a379)[(unsigned int)*param_2 * 0x90] == '\x01') {
LAB_005b3146:
          iVar24 = armor_val(iVar6,uVar18);
          iVar19 = weap_val(iVar6,uVar18);
          iVar19 = iVar24 * 8 - iVar19;
          if ((int)local_20 < iVar19) {
            local_20 = (char *)iVar19;
            local_14 = iVar6;
          }
          iVar24 = weap_val(iVar6,uVar18);
          iVar19 = armor_val(iVar6,uVar18);
          iVar24 = (unsigned int)(unsigned char)((unsigned char *)g_0094a378)[(unsigned int)*param_2 * 0x90] + (iVar19 + iVar24 * 2) * 2;
          if (param_3 < iVar24) {
            param_1 = iVar6;
            param_3 = iVar24;
          }
          iVar24 = weap_val(iVar6,uVar18);
          bVar14 = ((unsigned char *)g_0094a378)[(unsigned int)*param_2 * 0x90];
          iVar19 = armor_val(iVar6,uVar18);
          iVar19 = ((unsigned int)bVar14 * 4 - iVar24) - iVar19;
          if ((local_30 < iVar19) && (1 < (unsigned char)((unsigned char *)g_0094a378)[(unsigned int)*param_2 * 0x90])) {
            local_34 = iVar6;
            local_30 = iVar19;
          }
        }
      }
      iVar6 = iVar6 + 1;
      param_2 = param_2 + 0x34;
    } while ((int)param_2 < 0x9b208c);
    if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) == 0) {
      uVar7 = 0;
    }
    else {
      uVar7 = 0x15;
    }
    local_24 = veh_init(uVar7,uVar18,local_8,local_c);
    uVar8 = local_14;
    veh_init(local_14,uVar18,local_8,local_c);
    if ((0x14 < (*g_009a64d4)) && (-1 < local_34)) {
      veh_init(local_34,uVar18,local_8,local_c);
    }
    if ((0x28 < (*g_009a64d4)) && (-1 < (int)param_1)) {
      veh_init(param_1,uVar18,local_8,local_c);
    }
    if ((0x3c < (*g_009a64d4)) && (-1 < (int)uVar8)) {
      veh_init(uVar8,uVar18,local_8,local_c);
    }
    if ((0x50 < (*g_009a64d4)) && (-1 < (int)param_1)) {
      veh_init(param_1,uVar18,local_8,local_c);
    }
    if (100 < (*g_009a64d4)) {
      if ((*(unsigned int *)((char *)g_00946f58 + local_10) & 0x100) == 0) {
        uVar7 = 0;
      }
      else {
        uVar7 = 0x15;
      }
      veh_init(uVar7,uVar18,local_8,local_c);
    }
    veh_promote(local_24);
    *(int *)((char *)g_0096cd40 + local_2c) =
         (int)(*(int *)((char *)g_0096cd3c + local_2c) * 3 +
              (*(int *)((char *)g_0096cd3c + local_2c) * 3 >> 0x1f & 3U)) >> 2;
  }
  if (uVar18 == (*g_00939284)) {
    *(unsigned int *)((*g_007d3c3c) + 0x1dd9c) = local_8;
    *(unsigned int *)((*g_007d3c3c) + 0x1dda0) = local_c;
    puVar13 = (unsigned int *)g_00939340;
    do {
      puVar13[-0x20] = local_8;
      *puVar13 = local_c;
      puVar13 = puVar13 + 1;
    } while ((int)puVar13 < 0x9393c0);
  }
  return 1;
}

