// ORIGINAL: 0x004632D0 FILE
// RULED-OUT: SEH prologue vanished entirely by declaring a real `Random
//        local_rand;` local (ctor/dtor already recovered in random.h) and
//        deleting every explicit ctor/dtor call the Ghidra hypothesis
//        showed - the compiler emits the frame and every return path's
//        implicit destructor call itself, exactly as the brief predicted.
// RULED-OUT: contract typed a4/a5 as plain int; Ghidra uses both as real
//            int*/uint32_t* throughout (dereferenced, compared, walked) -
//            retyping them (and the MapWin class prototype) cleared ~70 of
//            the ~90 initial C2440/C2446 errors in one pass. `a1` (really
//            Buffer*) and the pre-fix `param_2`->`a2` (MapWin*) both get
//            reused mid-function as a plain int/byte scratch slot after
//            their last pointer use (matches the loop_base_idx pattern
//            found in base_terraform) - split into fresh locals
//            (`crop_diff`) rather than fighting the parameter's type.
//            Twelve callees (Texture-family scanline/Duff's-device
//            blitters) have no recovered name or signature anywhere in the
//            tree, and their own dedicated scaffolds are themselves
//            RULED-OUT approximations (ESP used as live data, no C
//            construct reaches that) - declared `extern "C" void
//            FUN_xxxxxxxx(...);` (ellipsis, no fixed params) so every call
//            site's argument values/order are preserved without guessing a
//            per-position type. Diverges at #1 (frame size / prologue),
//            same shape as the four-of-seven-bodies prologue note - some
//            combination of the extra `crop_diff` local and the RAII
//            frame's own layout shifts `sub esp` versus the original.
// working copy - scaffold materialised by --work
// name      ?gen_terrain_poly@MapWin@@QAAHPAUBuffer@@PAUMapWin@@HHHHHHHH@Z
// size      14804 bytes
// spans     0x004632D0-0x00466C92;0x00655920-0x00655932
// prototype 
// callers   7   call targets   38
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00462190 0x0046FE70 0x00470790 0x004E3A50 0x004E3EF0 0x004E6E50 0x0050BA00 0x00591B50 0x00592030 0x00592140 0x005BF010 0x005BF1F0 0x005BFE90 0x005C4FD0 0x005DA680 0x005DAC70 0x005DACB0 0x005DD130 0x005E15F0 0x005E4B9A 0x005E5833 0x0061A760 0x0061B1F0 0x0061C090 0x0061E540 0x00622DB0 0x006233C0 0x006239E0 0x00623F80 0x00624670 0x00625730 0x00625740 0x00625750 0x00625770 0x00644F3A 0x006453E0 0x00645470 0x0064FC88

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?gen_terrain_poly@MapWin@@QAAHPAUBuffer@@PAUMapWin@@HHHHHHHH@Z  at 0x004632D0  (14804 bytes)
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
class Random;
class Scroll;
class Spot;
class Sprite;
class Texture;
class Time;
typedef unsigned int UINT_PTR;
struct Vert;
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
    int poly(Vert *, int, int);
    int set_font(Font *, Font *, Font *, Font *);
    int text_height();
    int text_line_height();
    int write_cent_l(char *, RECT *, int);
    void clear_links();
    void close();
    void close_class();
    void construct();
    void destroy();
    void line(int, int, int, int, int);
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

class Random { public:
    uint32_t seed_;
    Random();
    uint32_t get_seed();
    unsigned int get(int, int);
    void reseed(unsigned long);
    ~Random();
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
    int draw(Buffer *, int, int, int);
    int draw_dest(Buffer *, int, int, int, unsigned char *);
    void close();
};

class Texture { public:
    void * pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    uint8_t unmapped_[0x60];
    uint32_t borrowed_;
    void close();
    void draw(Buffer *, Vert *, int *, int, Vert *);
    void draw(Buffer *, Vert *, int *, int, Vert *, int);
    void draw_coast(Buffer *, Vert *, int *, Texture *, unsigned char *, int, Vert *, int);
    void draw_mask_dest(Buffer *, Vert *, unsigned char *, unsigned char *, int, Vert *);
    void draw_trans(Buffer *, Vert *, int *, int, Vert *);
    void draw_upper_threshold_trans(Buffer *, Vert *, int *, int, Vert *, int);
};

extern "C" char *_itoa(int, char *, int);
extern "C" char *strcat(char *, const char *);
extern "C" int __stdcall fn_00622db0(int, int, int, int, int, int);
extern "C" int __stdcall fn_006239e0(int, int, int, int, int);
extern "C" int __stdcall fn_00624670(int, int, int, int, int, int, int);
extern "C" int abs(int);
extern "C" unsigned int strlen(const char *);
int alt_get_ocean_detail(int, int, int, int);
int base_at(int, int);
int bonus_at(int, int, int);
int crop_yield(int, int, int, int, int);
int goody_at(int, int);
int has_abil(int, int);
int is_sensor(int, int);
int veh_at(int, int);
int whose_territory(int, int, int, int *, int);
int world_site(int, int, int);
void bitmask(int, int *, int *);
void site_set(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_00655928 = (int *)0x00655928;
static int *const g_0066ef28 = (int *)0x0066EF28;
static int *const g_0066ef3c = (int *)0x0066EF3C;
static int *const g_0066ef50 = (int *)0x0066EF50;
static int *const g_0066ef74 = (int *)0x0066EF74;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_00675968 = (int *)0x00675968;
static int *const g_00684730 = (int *)0x00684730;
static int *const g_00685478 = (int *)0x00685478;
static int *const g_0068547c = (int *)0x0068547C;
static int *const g_00685484 = (int *)0x00685484;
static int *const g_00685485 = (int *)0x00685485;
static int *const g_00685a98 = (int *)0x00685A98;
static int *const g_00685a99 = (int *)0x00685A99;
static int *const g_00685afc = (int *)0x00685AFC;
static int *const g_00685afd = (int *)0x00685AFD;
static int *const g_00685bd4 = (int *)0x00685BD4;
static int *const g_00685bd5 = (int *)0x00685BD5;
static int *const g_00685ca8 = (int *)0x00685CA8;
static int *const g_00685cac = (int *)0x00685CAC;
static int *const g_00685ea0 = (int *)0x00685EA0;
static int *const g_00685ee0 = (int *)0x00685EE0;
static int *const g_00685f20 = (int *)0x00685F20;
static int *const g_00685f50 = (int *)0x00685F50;
static int *const g_00685f80 = (int *)0x00685F80;
static int *const g_00686090 = (int *)0x00686090;
static int *const g_00686094 = (int *)0x00686094;
static int *const g_00686098 = (int *)0x00686098;
static int *const g_0068609c = (int *)0x0068609C;
static int *const g_006860e8 = (int *)0x006860E8;
static int *const g_006860ec = (int *)0x006860EC;
static int *const g_006861f0 = (int *)0x006861F0;
static int *const g_00686354 = (int *)0x00686354;
static int *const g_0068e564 = (int *)0x0068E564;
static int *const g_0068e568 = (int *)0x0068E568;
static int *const g_0068e56c = (int *)0x0068E56C;
static int *const g_0068f9f0 = (int *)0x0068F9F0;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_00696d18 = (int *)0x00696D18;
static int *const g_00696d1c = (int *)0x00696D1C;
static int *const g_006972cc = (int *)0x006972CC;
static int *const g_006972d0 = (int *)0x006972D0;
static int *const g_006f077c = (int *)0x006F077C;
static int *const g_006f107c = (int *)0x006F107C;
static int *const g_0075b098 = (int *)0x0075B098;
static int *const g_0075b0a0 = (int *)0x0075B0A0;
static int *const g_0075b230 = (int *)0x0075B230;
static int *const g_0075b238 = (int *)0x0075B238;
static int *const g_0076e8a0 = (int *)0x0076E8A0;
static int *const g_0076e9e0 = (int *)0x0076E9E0;
static int *const g_00776758 = (int *)0x00776758;
static int *const g_00776760 = (int *)0x00776760;
static int *const g_007768b8 = (int *)0x007768B8;
static int *const g_007768c0 = (int *)0x007768C0;
static int *const g_007768e4 = (int *)0x007768E4;
static int *const g_007768ec = (int *)0x007768EC;
static int *const g_00776910 = (int *)0x00776910;
static int *const g_00776918 = (int *)0x00776918;
static int *const g_0077693c = (int *)0x0077693C;
static int *const g_00776944 = (int *)0x00776944;
static int *const g_00776968 = (int *)0x00776968;
static int *const g_00776970 = (int *)0x00776970;
static int *const g_00776994 = (int *)0x00776994;
static int *const g_0077699c = (int *)0x0077699C;
static int *const g_007769c0 = (int *)0x007769C0;
static int *const g_007769c8 = (int *)0x007769C8;
static int *const g_00776a50 = (int *)0x00776A50;
static int *const g_00776a58 = (int *)0x00776A58;
static int *const g_00776a80 = (int *)0x00776A80;
static int *const g_00779390 = (int *)0x00779390;
static int *const g_00779398 = (int *)0x00779398;
static int *const g_007797f8 = (int *)0x007797F8;
static int *const g_0077aff8 = (int *)0x0077AFF8;
static int *const g_0077b000 = (int *)0x0077B000;
static int *const g_00787e56 = (int *)0x00787E56;
static int *const g_00787e58 = (int *)0x00787E58;
static int *const g_00787e5b = (int *)0x00787E5B;
static int *const g_00788100 = (int *)0x00788100;
static int *const g_00789b68 = (int *)0x00789B68;
static int *const g_00789b70 = (int *)0x00789B70;
static int *const g_00789c28 = (int *)0x00789C28;
static int *const g_0078a340 = (int *)0x0078A340;
static int *const g_0078a5b0 = (int *)0x0078A5B0;
static int *const g_0078a5b8 = (int *)0x0078A5B8;
static int *const g_0078a700 = (int *)0x0078A700;
static int *const g_0078a708 = (int *)0x0078A708;
static int *const g_0078a72c = (int *)0x0078A72C;
static int *const g_0078a734 = (int *)0x0078A734;
static int *const g_0078a758 = (int *)0x0078A758;
static int *const g_0078dd20 = (int *)0x0078DD20;
static int *const g_0078dd28 = (int *)0x0078DD28;
static int *const g_0078dd80 = (int *)0x0078DD80;
static int *const g_00791c58 = (int *)0x00791C58;
static int *const g_00791c60 = (int *)0x00791C60;
static int *const g_00791fc8 = (int *)0x00791FC8;
static int *const g_00791fd0 = (int *)0x00791FD0;
static int *const g_00792218 = (int *)0x00792218;
static int *const g_00792288 = (int *)0x00792288;
static int *const g_00798e08 = (int *)0x00798E08;
static int *const g_00798e78 = (int *)0x00798E78;
static int *const g_007991f8 = (int *)0x007991F8;
static int *const g_00799200 = (int *)0x00799200;
static int *const g_00799238 = (int *)0x00799238;
static int *const g_00799658 = (int *)0x00799658;
static int *const g_00799660 = (int *)0x00799660;
static int *const g_00799684 = (int *)0x00799684;
static int *const g_0079968c = (int *)0x0079968C;
static int *const g_007996b0 = (int *)0x007996B0;
static int *const g_007996b8 = (int *)0x007996B8;
static int *const g_007996dc = (int *)0x007996DC;
static int *const g_007996e4 = (int *)0x007996E4;
static int *const g_00799738 = (int *)0x00799738;
static int *const g_00799e48 = (int *)0x00799E48;
static int *const g_00799eb8 = (int *)0x00799EB8;
static int *const g_0079a710 = (int *)0x0079A710;
static int *const g_0079a718 = (int *)0x0079A718;
static int *const g_007a6830 = (int *)0x007A6830;
static int *const g_007a6838 = (int *)0x007A6838;
static int *const g_007a6860 = (int *)0x007A6860;
static int *const g_007a6868 = (int *)0x007A6868;
static int *const g_007a7820 = (int *)0x007A7820;
static int *const g_007a7890 = (int *)0x007A7890;
static int *const g_007a7aa0 = (int *)0x007A7AA0;
static int *const g_007a99a0 = (int *)0x007A99A0;
static int *const g_007a99a8 = (int *)0x007A99A8;
static int *const g_007abf60 = (int *)0x007ABF60;
static int *const g_007abf68 = (int *)0x007ABF68;
static int *const g_007ac098 = (int *)0x007AC098;
static int *const g_007ac0a0 = (int *)0x007AC0A0;
static int *const g_007ac220 = (int *)0x007AC220;
static int *const g_007acb88 = (int *)0x007ACB88;
static int *const g_007acb90 = (int *)0x007ACB90;
static int *const g_007acc60 = (int *)0x007ACC60;
static int *const g_007acc68 = (int *)0x007ACC68;
static int *const g_007acd70 = (int *)0x007ACD70;
static int *const g_007acd78 = (int *)0x007ACD78;
static int *const g_007ad010 = (int *)0x007AD010;
static int *const g_007ad018 = (int *)0x007AD018;
static int *const g_007d3af0 = (int *)0x007D3AF0;
static int *const g_007d3af4 = (int *)0x007D3AF4;
static int *const g_007d3be8 = (int *)0x007D3BE8;
static int *const g_007d3c08 = (int *)0x007D3C08;
static int *const g_007d3c0c = (int *)0x007D3C0C;
static int *const g_007d3c2c = (int *)0x007D3C2C;
static int *const g_007f6644 = (int *)0x007F6644;
static int *const g_007f664c = (int *)0x007F664C;
static int *const g_007f6684 = (int *)0x007F6684;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_008eb48c = (int *)0x008EB48C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0094a379 = (int *)0x0094A379;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_00952858 = (int *)0x00952858;
static int *const g_0095285a = (int *)0x0095285A;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9e1 = (int *)0x0096C9E1;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d04a = (int *)0x0097D04A;
static int *const g_0097d0cc = (int *)0x0097D0CC;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009bb548 = (int *)0x009BB548;

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

    int get_alt(int, int, int);
    int get_point_light(int, int, int);
    void on_redraw();
    int UNK1();
    void UNK3();
    void do_image_buttons();
    void main_caption();
    void on_left_double_click(int, int);
    void on_left_up(int, int);
    void close();
    int UNK2();
    int __cdecl gen_terrain_poly(Buffer *, MapWin *, int, int *, uint32_t *, int, int, int, int, int);
};
// Two globals the emitter's own scan missed; add them alongside the rest.
static int *const g_006860f0 = (int *)0x006860F0;
static int *const g_006860f4 = (int *)0x006860F4;

// Twelve callees this function reaches whose real names and calling
// convention are not recovered anywhere else in the tree (their own
// scaffolds, where they exist, are themselves RULED-OUT approximations
// of hand-tuned Duff's-device blitters and textured scanline fillers
// that use the live ESP as data - no portable C reaches them). Some
// call sites pass what is really a Buffer* here and a plain shading
// int there (the same slot is reused for both across the function, as
// `iVar24` shows by being assigned first from `a1` and later from
// literal color constants). Declaring them ellipsis-only sidesteps
// guessing a fixed signature per position while keeping every call's
// argument values and order faithful to the decompilation.
extern "C" void FUN_005e15f0(...);
extern "C" void FUN_005e4b9a(...);
extern "C" void FUN_005e5833(...);
extern "C" void FUN_0061a760(...);
extern "C" void FUN_0061b1f0(...);
extern "C" void FUN_0061c090(...);
extern "C" void FUN_0061e540(...);
extern "C" void FUN_00622db0(...);
extern "C" void FUN_006233c0(...);
extern "C" void FUN_006239e0(...);
extern "C" void FUN_00623f80(...);
extern "C" void FUN_00624670(...);

int __cdecl MapWin::gen_terrain_poly(Buffer * a1, MapWin * a2, int a3, int *a4, uint32_t *a5, int a6, int a7, int a8, int a9, int a10)
{
  uint8_t uVar1;
  short sVar2;
  int32_t uVar3;
  char cVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint32_t *puVar8;
  unsigned int sVar9;
  uint8_t bVar10;
  uint8_t bVar11;
  uint32_t uVar12;
  int iVar13;
  int *piVar14;
  int32_t uVar15;
  uint32_t uVar16;
  uint32_t uVar17;
  uint8_t *pbVar18;
  int iVar19;
  int32_t *puVar20;
  int32_t *puVar21;
  int *piVar22;
  bool bVar23;
  int iVar24;
  uint8_t local_5cc [256];
  char local_4cc [256];
  int32_t local_3cc [24];
  int32_t local_36c [32];
  int local_2ec [32];
  int32_t auStack_26c [12];
  uint8_t local_23c [80];
  int32_t local_1ec [24];
  int local_18c [24];
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  uint32_t local_11c;
  int local_118;
  int32_t local_114;
  uint32_t local_108 [6];
  int32_t local_f0;
  int32_t local_ec;
  int32_t local_e8;
  int32_t local_e4;
  int32_t local_e0;
  int32_t local_dc;
  int32_t local_d8;
  int32_t local_d4;
  int32_t local_d0;
  int32_t local_cc;
  int32_t local_c8;
  int32_t local_c4;
  uint32_t local_c0;
  uint32_t local_bc;
  int32_t local_b8 [4];
  int local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  int local_98;
  int local_94;
  int local_90;
  uint32_t local_8c;
  int *local_88;
  uint32_t local_84;
  int32_t *local_80;
  int32_t *local_7c;
  int local_78;
  int local_70;
  uint32_t local_6c [9];
  uint8_t *local_48;
  uint32_t local_44;
  uint32_t local_40;
  int32_t *local_3c;
  uint32_t local_38;
  uint32_t local_34;
  uint32_t local_30;
  uint32_t *local_2c;
  int *local_28;
  uint32_t local_24;
  uint32_t local_20;
  int local_1c;
  uint32_t local_18;
  int local_14;
  Random local_rand;
  int crop_diff = 0;

  iVar24 = reinterpret_cast<int>(a1);
  local_108[4] = 0;
  if ((((a8 < 0) || ((*g_00949874) <= a8)) || ((int)a7 < 0)) ||
     ((*g_00949870) <= (int)a7)) {
    return 0;
  }
  iVar19 = (int)a7 >> 1;
  iVar5 = (*g_0094a30c) + ((*g_0068faf0) * a8 + iVar19) * 0x2c;
  local_2c = (uint32_t *)(iVar5 + 0xc);
  local_34 = *local_2c;
  local_14 = iVar19;
  if (((a2 != 0) && (reinterpret_cast<char *>(a2) == reinterpret_cast<char *>(g_008eb48c))) &&
     (((a6 & 0x80000000) == 0 && ((local_34 & 0x400000) == 0)))) goto LAB_00464756;
  local_rand.reseed(iVar5);
  pbVar18 = (uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + iVar19) * 0x2c);
  local_18 = (uint32_t)((*pbVar18 & 0xe0) < 0x60);
  if (local_18 == 0) {
    (*g_006972cc) = 0xfffffffc;
    (*g_006972d0) = 4;
  }
  if ((local_34 & 0x103) != 0) {
    local_94 = *(int *)(pbVar18 + 0xc) >> 0x18;
  }
  if ((*g_00691e6c) < 1) {
    iVar5 = (a3 * 5 + 0x50) * 10;
    iVar5 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfU)) >> 4;
  }
  else {
    iVar5 = ((*g_00691e6c) * 0x32) / (*g_00691e70);
  }
  local_98 = (int)(iVar5 + 1 + (iVar5 + 1 >> 0x1f & 3U)) >> 2;
  local_78 = local_98 * 2;
  if (a2 == 0) {
    local_44 = 0;
  }
  else {
    iVar5 = a2->get_alt(a7,a8,4);
    iVar19 = a2->get_alt(a7,a8,3);
    iVar6 = a2->get_alt(a7,a8,2);
    iVar7 = a2->get_alt(a7,a8,1);
    local_44 = iVar5 + iVar19 + iVar6 + iVar7 >> 2;
  }
  a10 = a10 + local_44;
  local_6c[8] = (uint32_t)a4;
  if (((int)a4 < 0) &&
     (bVar10 = *(uint8_t *)((*g_0094a30c) + 5 + ((*g_0068faf0) * a8 + local_14) * 0x2c),
     local_6c[8] = bVar10 & 7, (bVar10 & 7) == 0)) {
    local_6c[8] = (*g_00939284);
  }
  iVar5 = (*g_0068faf0) * a8 + local_14;
  pbVar18 = (uint8_t *)((*g_0094a30c) + iVar5 * 0x2c);
  local_84 = (*pbVar18 & 0x18) >> 3;
  local_8c = (uint32_t)(*pbVar18 >> 5);
  local_38 = (uint32_t)(pbVar18[5] >> 6);
  if (((int)a4 < 0) || ((*(uint32_t *)(g_0096c9e0 + (int)a4 * 0x20cc) & 0x200) != 0)) {
    local_24 = *(uint32_t *)(pbVar18 + 8);
  }
  else {
    local_24 = *(uint32_t *)((*g_0094a30c) + 0xc + ((int)a4 + iVar5 * 0xb) * 4);
  }
  if ((a3 < -6) || ((a6 & 0x80000000) != 0)) {
    local_90 = 1;
  }
  else {
    local_90 = 0;
  }
  local_11c = a6 & 0x1000000;
  bVar10 = (uint8_t)a4;
  if ((a6 & 0x1000000) == 0) {
    if ((((((*g_009a649c) >> 8 & 1) != 0) && (((uint8_t)(*g_009a64c0) & 0x80) == 0)) &&
        (((g_0096c9e1)[(int)a4 * 0x20cc] & 2) == 0)) &&
       ((pbVar18[4] & (uint8_t)(1 << (bVar10 & 0x1f))) == 0)) {
      local_84 = 0;
      local_38 = 0;
      local_24 = local_24 & 0xffffffdf;
      local_70 = 1;
      local_1c = 0;
LAB_004635f8:
      uVar12 = *local_2c | 0x200000;
      goto LAB_00463603;
    }
    local_70 = 0;
    if (((((uint8_t)(*g_009a64c0) & 0x80) != 0) || ((int)a4 < 1)) ||
       ((((*g_009a6494) & 1) == 0 || (a3 < -7)))) goto LAB_00463589;
    local_1c = 0;
    local_40 = 0;
    do {
      local_20 = *(int *)((int)g_0066efbc + local_40) + a7;
      if (((*g_0094988c) & 1) == 0) {
        if ((int)local_20 < 0) {
          local_20 = local_20 + (*g_00949870);
        }
        else if ((*g_00949870) <= (int)local_20) {
          local_20 = local_20 - (*g_00949870);
        }
      }
      iVar5 = *(int *)((int)g_0066f440 + local_40) + a8;
      if (((-1 < iVar5) && (iVar5 < (*g_00949874))) &&
         ((-1 < (int)local_20 && ((int)local_20 < (*g_00949870))))) {
        if ((int)local_40 < 0x24) {
          iVar19 = (*g_0094a30c) + ((*g_0068faf0) * iVar5 + ((int)local_20 >> 1)) * 0x2c;
          if (((*(uint8_t *)(iVar19 + 8) & 3) == 0) ||
             (uVar12 = *(uint8_t *)(iVar19 + 2) & 0xf, 7 < uVar12)) {
            uVar12 = 0xffffffff;
          }
          if (((int *)uVar12 == a4) ||
             ((((-1 < (*g_0068e564) &&
                ((int *)(uint32_t)(uint8_t)(g_00952836)[(*g_0068e56c) * 0x34] == a4)) &&
               ((*g_0068e564) == local_20)) && ((*g_0068e568) == iVar5)))) goto LAB_00463b55;
          if (((g_0096c9f8)[uVar12 * 4 + (int)a4 * 0x20cc] & 1) != 0) {
            for (iVar19 = veh_at(local_20,iVar5); -1 < iVar19;
                iVar19 = (int)(short)(g_00952858)[iVar19 * 0x1a]) {
              if ((int *)(uint32_t)(uint8_t)(g_00952836)[iVar19 * 0x34] == a4) goto LAB_00463b55;
              if (iVar19 < 0) break;
            }
            if (local_1c != 0) goto LAB_0046358c;
          }
        }
        uVar12 = local_20;
        iVar6 = is_sensor(local_20,iVar5);
        iVar19 = (*g_0068e56c);
        iVar7 = (int)uVar12 >> 1;
        if ((iVar6 != 0) &&
           ((int *)(int)*(char *)((*g_0094a30c) + 7 + ((*g_0068faf0) * iVar5 + iVar7) * 0x2c) ==
            a4)) goto LAB_00463b55;
        iVar6 = (*g_0094a30c) + ((*g_0068faf0) * iVar5 + iVar7) * 0x2c;
        if (((*(uint8_t *)(iVar6 + 8) & 1) == 0) || (uVar16 = *(uint8_t *)(iVar6 + 2) & 0xf, 7 < uVar16))
        {
          uVar16 = 0xffffffff;
        }
        if (((int *)uVar16 == a4) ||
           (((((*g_0068e564) == uVar12 && ((*g_0068e568) == iVar5)) &&
             (((int *)(uint32_t)(uint8_t)(g_00952836)[(*g_0068e56c) * 0x34] == a4 &&
              (iVar6 = has_abil((int)(short)(g_00952832)[(*g_0068e56c) * 0x1a],2), iVar6 != 0)
              ))) && (((g_0094a379)
                       [(uint32_t)(uint8_t)(g_009ab88c)[(short)(g_00952832)[iVar19 * 0x1a] * 0x34] *
                        0x90] == '\x02' ||
                      (local_18 ==
                       ((g_0094a379)
                        [(uint32_t)(uint8_t)(g_009ab88c)[(short)(g_00952832)[iVar19 * 0x1a] * 0x34] *
                         0x90] == '\x01'))))))) goto LAB_00463b55;
        iVar19 = (*g_0094a30c) + ((*g_0068faf0) * iVar5 + iVar7) * 0x2c;
        if (((*(uint8_t *)(iVar19 + 8) & 2) == 0) || (uVar12 = *(uint8_t *)(iVar19 + 2) & 0xf, 7 < uVar12)
           ) {
          uVar12 = 0xffffffff;
        }
        if ((((int *)uVar12 == a4) ||
            (((g_0096c9f8)[((int)a4 + uVar12 * 0x833) * 4] & 1) != 0)) &&
           (iVar5 = veh_at(local_20,iVar5), -1 < iVar5)) {
          sVar2 = (g_0095285a)[iVar5 * 0x1a];
          while (-1 < sVar2) {
            iVar5 = (int)sVar2;
            sVar2 = (g_0095285a)[iVar5 * 0x1a];
          }
          if (-1 < iVar5) {
            while ((((int *)(uint32_t)(uint8_t)(g_00952836)[iVar5 * 0x34] != a4 ||
                    (iVar19 = has_abil((int)(short)(g_00952832)[iVar5 * 0x1a],2), iVar19 == 0
                    )) || (((g_0094a379)
                            [(uint32_t)(uint8_t)(g_009ab88c)
                                         [(short)(g_00952832)[iVar5 * 0x1a] * 0x34] * 0x90] !=
                            '\x02' &&
                           (local_18 !=
                            ((g_0094a379)
                             [(uint32_t)(uint8_t)(g_009ab88c)
                                          [(short)(g_00952832)[iVar5 * 0x1a] * 0x34] * 0x90] ==
                            '\x01')))))) {
              if ((iVar5 < 0) || (iVar5 = (int)(short)(g_00952858)[iVar5 * 0x1a], iVar5 < 0))
              goto LAB_00463a11;
            }
            local_1c = 1;
          }
        }
      }
LAB_00463a11:
      local_40 = local_40 + 4;
    } while ((int)local_40 < 100);
    if (local_1c == 0) {
      if ((*g_009a6488) != 0) {
        iVar5 = 0;
        do {
          iVar6 = *(int *)((int)g_0066f440 + iVar5);
          iVar19 = *(int *)((int)g_0066efbc + iVar5);
          iVar7 = iVar6 - iVar19 >> 1;
          iVar13 = iVar19 + iVar6 >> 1;
          if (iVar7 * iVar7 + iVar13 * iVar13 < 0x19) {
            iVar19 = iVar19 + a7;
            if (((*g_0094988c) & 1) == 0) {
              if (iVar19 < 0) {
                iVar19 = iVar19 + (*g_00949870);
              }
              else if ((*g_00949870) <= iVar19) {
                iVar19 = iVar19 - (*g_00949870);
              }
            }
            iVar6 = iVar6 + a8;
            if ((((-1 < iVar6) && (iVar6 < (*g_00949874))) && (-1 < iVar19)) &&
               (iVar19 < (*g_00949870))) {
              iVar7 = (*g_0068faf0) * iVar6 + (iVar19 >> 1);
              if (((*(uint8_t *)((*g_0094a30c) + 8 + iVar7 * 0x2c) & 1) == 0) ||
                 (uVar12 = *(uint8_t *)((*g_0094a30c) + iVar7 * 0x2c + 2) & 0xf, 7 < uVar12)) {
                uVar12 = 0xffffffff;
              }
              if ((int *)uVar12 == a4) {
                iVar19 = base_at(iVar19,iVar6);
                bitmask(0x29,(int *)&local_88,(int *)&local_30);
                local_48 = (uint8_t *)(uint32_t)((*(uint8_t *)(reinterpret_cast<char *>(local_88) + iVar19 * 0x4d + 0x25f433) &
                                          (uint8_t)local_30) != 0);
                if (local_48 != (uint8_t *)0x0) goto LAB_00463b55;
              }
            }
          }
          iVar5 = iVar5 + 4;
        } while (iVar5 < 0x144);
      }
      goto LAB_004635f8;
    }
  }
  else {
    local_84 = 0;
    local_38 = 0;
    local_24 = 0;
    local_70 = 1;
LAB_00463589:
    local_1c = 1;
  }
LAB_0046358c:
  uVar12 = *local_2c & 0xffdfffff;
LAB_00463603:
  *local_2c = uVar12;
  local_108[0] = 0;
  local_108[1] = 0;
  iVar5 = 0;
  local_108[2] = 0;
  local_28 = (int *)0x0;
  local_40 = 0;
  local_2c = (uint32_t *)0x0;
  local_30 = 0;
  local_108[3] = 0;
  local_20 = a6 & 0x80000000;
  if ((a6 & 0x80000000) == 0) {
    if (local_18 == 0) {
      uVar12 = 0;
      local_3c = (int32_t *)((*g_0094988c) & 1);
      do {
        iVar5 = (g_0066ef50)[uVar12] + a7;
        if (local_3c == (int32_t *)0x0) {
          if (iVar5 < 0) {
            iVar5 = (*g_00949870) + iVar5;
          }
          else if ((*g_00949870) <= iVar5) {
            iVar5 = iVar5 - (*g_00949870);
          }
        }
        iVar19 = (g_0066ef74)[uVar12] + a8;
        if (((-1 < iVar19) && (iVar19 < (*g_00949874))) &&
           ((-1 < iVar5 &&
            ((iVar5 < (*g_00949870) &&
             ((*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * iVar19 + (iVar5 >> 1)) * 0x2c) & 0xe0) <
              0x60)))))) {
          iVar5 = (int)uVar12 >> 1;
          if ((uVar12 & 1) == 0) {
            uVar16 = iVar5 + 1U & 3;
            puVar8 = local_108 + (iVar5 - 2U & 3);
            local_108[uVar16] = local_108[uVar16] | 4;
            uVar16 = *puVar8 | 1;
          }
          else {
            uVar16 = iVar5 - 2U & 3;
            puVar8 = local_108 + uVar16;
            uVar16 = local_108[uVar16] | 2;
          }
          *puVar8 = uVar16;
        }
        uVar12 = uVar12 + 1;
      } while ((int)uVar12 < 8);
      local_30 = (uint32_t)((local_34 & 0x80000004) == 4);
      if (((local_84 != 0) || ((local_24 & 0x20) != 0)) || (local_30 != 0)) {
        iVar5 = 0;
        do {
          uVar12 = iVar5 - 1U & 7;
          iVar19 = (g_0066ef50)[uVar12] + a7;
          if (local_3c == (int32_t *)0x0) {
            if (iVar19 < 0) {
              iVar19 = iVar19 + (*g_00949870);
            }
            else if ((*g_00949870) <= iVar19) {
              iVar19 = iVar19 - (*g_00949870);
            }
          }
          iVar6 = (g_0066ef74)[uVar12] + a8;
          if (((-1 < iVar6) && (iVar6 < (*g_00949874))) && ((-1 < iVar19 && (iVar19 < (*g_00949870))))
             ) {
            pbVar18 = (uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * iVar6 + (iVar19 >> 1)) * 0x2c);
            bVar11 = (uint8_t)iVar5;
            if (((pbVar18[8] & 0x20) != 0) && (0x3f < (*pbVar18 & 0xe0))) {
              local_40 = local_40 | 1 << (bVar11 & 0x1f);
            }
            if (((local_30 != 0) && ((*(uint32_t *)(pbVar18 + 0xc) & 0x80000004) == 4)) &&
               (0x5f < (*pbVar18 & 0xe0))) {
              local_2c = (uint32_t *)((uint32_t)local_2c | 1 << (bVar11 & 0x1f));
            }
            if ((local_84 != 0) &&
               (((*pbVar18 & 0xe0) < 0x60 || ((int)local_84 <= (int)(*pbVar18 >> 3 & 3))))) {
              local_28 = (int *)((uint32_t)local_28 | 1 << (bVar11 & 0x1f));
            }
          }
          iVar5 = iVar5 + 1;
        } while (iVar5 < 8);
      }
    }
    else if ((local_24 & 0x20) != 0) {
      do {
        uVar12 = iVar5 - 1U & 7;
        iVar19 = (g_0066ef50)[uVar12] + a7;
        if (((*g_0094988c) & 1) == 0) {
          if (iVar19 < 0) {
            iVar19 = iVar19 + (*g_00949870);
          }
          else if ((*g_00949870) <= iVar19) {
            iVar19 = iVar19 - (*g_00949870);
          }
        }
        iVar6 = (g_0066ef74)[uVar12] + a8;
        if (((-1 < iVar6) && (iVar6 < (*g_00949874))) &&
           ((-1 < iVar19 &&
            (((iVar19 < (*g_00949870) &&
              (pbVar18 = (uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * iVar6 + (iVar19 >> 1)) * 0x2c),
              (pbVar18[8] & 0x20) != 0)) && (0x3f < (*pbVar18 & 0xe0))))))) {
          local_40 = local_40 | 1 << ((uint8_t)iVar5 & 0x1f);
        }
        iVar5 = iVar5 + 1;
      } while (iVar5 < 8);
    }
  }
  a10 = a10 + local_78;
  a9 = a9 + local_78 * 2;
  if ((0x15 < (int)a5) ||
     (((((*g_009a649c) & 0x100) == 0 && (-1 < (int)a4)) &&
      (((*(uint32_t *)(g_0096c9e0 + (int)a4 * 0x20cc) & 0x200) == 0 &&
       ((*(uint8_t *)((*g_0094a30c) + 4 + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
        (uint8_t)(1 << (bVar10 & 0x1f))) == 0)))))) {
    if (local_20 == 0) {
      iVar24 = 0;
      a4 = g_007d3c0c;
      piVar14 = local_18c + 2;
      do {
        piVar22 = piVar14 + -1;
        a6 = 3;
        do {
          iVar5 = *(int *)((int)g_00685f50 + iVar24) * local_98;
          piVar22[-1] = *(int *)((int)g_00685f20 + iVar24) * local_78 + a9;
          *piVar22 = iVar5 + a10;
          if (a2 != 0) {
            iVar5 = *(int *)((int)g_00685f80 + iVar24);
            if ((a8 < (*g_00949874)) && ((int)a7 < (*g_00949870))) {
              a2->get_alt(a7,a8,iVar5);
              if (*(int *)(reinterpret_cast<char *>(a2) + 4) == 0) {
                cVar4 = '\0';
              }
              else {
                cVar4 = *(char *)(*(int *)(reinterpret_cast<char *>(a2) + 4) +
                                  (((*g_00949870) >> 1) * a8 + local_14) * 0xc + 4 + iVar5);
              }
              iVar5 = (int)cVar4;
            }
            else {
              iVar5 = 0;
            }
            *piVar22 = *piVar22 - iVar5;
          }
          iVar24 = iVar24 + 4;
          piVar22 = piVar22 + 2;
          a6 = a6 + -1;
        } while (a6 != 0);
        a4[-1] = *piVar14;
        *a4 = piVar14[1];
        a4 = a4 + 2;
        piVar14 = piVar14 + 6;
      } while ((int)a4 < 0x7d3c2c);
      puVar21 = g_007d3c08;
      uVar12 = 1;
      do {
        FUN_005e15f0(*puVar21,puVar21[1],(g_007d3c08)[(uVar12 & 3) * 2],
                     (g_007d3c0c)[(uVar12 & 3) * 2],0xc2);
        puVar21 = puVar21 + 2;
        bVar23 = (int)uVar12 < 4;
        uVar12 = uVar12 + 1;
      } while (bVar23);
      if (((g_009392c0)[(*g_009392b8)] == a7) && ((g_00939340)[(*g_009392b8)] == a8))
      {
        puVar21 = g_007d3c08;
        puVar20 = g_007d3be8;
        for (iVar24 = 8; iVar24 != 0; iVar24 = iVar24 + -1) {
          *puVar20 = *puVar21;
          puVar21 = puVar21 + 1;
          puVar20 = puVar20 + 1;
        }
      }
    }
    goto LAB_00466c32;
  }
  iVar5 = 0;
  local_bc = 0;
  local_c4 = local_rand.get(0,2);
  local_48 = (uint8_t *)(a6 & 0x10000000);
  if ((uint8_t *)(a6 & 0x10000000) == (uint8_t *)0x0) {
    a5 = (uint32_t *)0x0;
    iVar19 = 0;
    do {
      iVar6 = 0;
      do {
        iVar7 = *(int *)((int)g_00685f50 + iVar5) * local_98;
        *(int *)((int)local_18c + iVar19) =
             *(int *)((int)g_00685f20 + iVar5) * local_78 + a9;
        bVar23 = local_18 == 0;
        *(int *)((int)local_18c + iVar19 + 4) = iVar7 + a10;
        if ((((bVar23) && ((local_24 & 0x8000) == 0)) && (local_84 != 0)) && (local_20 == 0)) {
          iVar7 = iVar6 + ((uint32_t)(uint8_t)(g_00685485)[(int)local_28 * 2] + (int)a5 & 3) * 3;
          uVar15 = (g_00686094)[iVar7 * 2];
          *(int32_t *)((int)local_1ec + iVar19) = (g_00686090)[iVar7 * 2];
        }
        else {
          uVar15 = *(int32_t *)((int)g_00686094 + iVar19);
          *(int32_t *)((int)local_1ec + iVar19) = *(int32_t *)((int)g_00686090 + iVar19);
        }
        *(int32_t *)((int)local_1ec + iVar19 + 4) = uVar15;
        if (local_38 != 0) {
          uVar15 = *(int32_t *)((int)g_00686094 + iVar19);
          *(int32_t *)((int)local_3cc + iVar19) = *(int32_t *)((int)g_00686090 + iVar19);
          *(int32_t *)((int)local_3cc + iVar19 + 4) = uVar15;
        }
        if ((2 < (int)local_8c) && (local_84 != 0)) {
          local_bc = (uint32_t)(uint8_t)(g_00685484)[(int)local_28 * 2];
        }
        iVar6 = iVar6 + 1;
        iVar5 = iVar5 + 4;
        iVar19 = iVar19 + 8;
      } while (iVar6 < 3);
      a5 = (uint32_t *)((int)a5 + 1);
    } while (iVar5 < 0x30);
    local_3c = (int32_t *)0x2;
    a5 = (uint32_t *)0x0;
    iVar5 = 0;
    do {
      uVar12 = (int)a5 - 2;
      iVar19 = iVar5;
      do {
        iVar5 = iVar19 + 8;
        iVar6 = ((uVar12 & 3) + (int)a5 * 4) * 4;
        iVar7 = *(int *)(g_00685ee0 + iVar6) * local_98;
        *(int *)((int)local_2ec + iVar19) = *(int *)(g_00685ea0 + iVar6) * local_78 + a9;
        uVar15 = *(int32_t *)((int)g_006860f0 + iVar19);
        *(int *)((int)local_2ec + iVar19 + 4) = iVar7 + a10;
        uVar3 = *(int32_t *)((int)g_006860f4 + iVar19);
        *(int32_t *)((int)auStack_26c + iVar19) = uVar15;
        puVar21 = local_3c;
        *(int32_t *)((int)auStack_26c + iVar19 + 4) = uVar3;
        uVar12 = uVar12 + 1;
        iVar19 = iVar5;
      } while ((int)((int)puVar21 + uVar12) < 4);
      a5 = (uint32_t *)((int)a5 + 1);
      local_3c = (int32_t *)((int)local_3c + -1);
    } while (iVar5 < 0x80);
    iVar5 = 0;
    local_80 = (int32_t *)0x0;
    local_c0 = a6 & 0x10000;
    if (((a6 & 0x10000) != 0) &&
       (((((*(uint32_t *)(g_0096c9e0 + (*g_00939284) * 0x20cc) & 0x200) != 0 ||
          ((*(uint8_t *)((*g_0094a30c) + 4 + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
           (uint8_t)(1 << ((uint8_t)(*g_00939284) & 0x1f))) != 0)) || (((uint8_t)(*g_009a64c0) & 0x80) != 0))
        && (local_20 != 0)))) {
      local_80 = (int32_t *)0x1;
      local_6c[8] = whose_territory((*g_00939284),a7,a8,0,0);
    }
    a5 = (uint32_t *)0x0;
    local_28 = (int *)0x0;
    local_7c = local_b8;
    local_3c = g_007d3c0c;
    do {
      piVar14 = &local_118;
      iVar19 = 0;
      do {
        if ((int)local_8c < 3) {
          *piVar14 = (int)a5;
        }
        else {
          if (a2 != 0) {
            iVar6 = (int)local_28 + iVar19;
            local_88 = piVar14;
            iVar7 = a2->get_alt(a7,a8,(g_00685f80)[iVar6]);
            local_18c[iVar6 * 2 + 1] = local_18c[iVar6 * 2 + 1] - iVar7;
            piVar14 = local_88;
            if (a2 != 0) {
              if (local_1c == 0) {
                *local_88 = (*g_00686354);
              }
              else {
                iVar6 = a2->get_point_light(a7,a8,(g_00685f80)[(int)local_28 + iVar19]);
                *piVar14 = iVar6;
              }
              goto LAB_00464158;
            }
          }
          *piVar14 = 0;
        }
LAB_00464158:
        iVar19 = iVar19 + 1;
        piVar14 = piVar14 + 1;
        local_88 = piVar14;
      } while (iVar19 < 3);
      uVar15 = *(int32_t *)((int)local_18c + iVar5 + 0xc);
      local_3c[-1] = *(int32_t *)((int)local_18c + iVar5 + 8);
      *local_3c = uVar15;
      *local_7c = local_114;
      if ((local_18 == 0) &&
         (((local_80 == (int32_t *)0x0 || ((int)local_6c[8] < 1)) && (local_20 == 0)))) {
        if (((local_24 & 0x8000) == 0) || ((a6 & 1) == 0)) {
          if ((local_34 & 0x40) == 0) {
            if (local_84 == 0) {
              iVar19 = (int)local_1ec + iVar5;
              iVar7 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
              iVar6 = (int)local_18c + iVar5;
              goto LAB_00464353;
            }
            if (local_84 == 1) {
              iVar19 = (int)local_1ec + iVar5;
              iVar7 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
              iVar6 = (int)local_18c + iVar5;
              goto LAB_00464353;
            }
            if (local_84 == 2) {
              iVar19 = (int)local_1ec + iVar5;
              iVar7 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
              iVar6 = (int)local_18c + iVar5;
              goto LAB_00464353;
            }
          }
          else {
            iVar19 = (int)g_00686090 + iVar5;
            iVar7 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
            iVar6 = (int)local_18c + iVar5;
LAB_00464353:
            FUN_006233c0(a1,iVar6,&local_118,3,iVar19,iVar7);
          }
          if (local_38 == 0) goto LAB_004643ae;
          iVar6 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
          iVar19 = (int)local_18c + iVar5;
        }
        else {
          iVar6 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
          iVar19 = (int)local_18c + iVar5;
          FUN_006233c0(a1,iVar19,&local_118,3,(int)local_1ec + iVar5,iVar6);
          if (local_38 == 0) goto LAB_004643ae;
        }
        FUN_00622db0(a1,iVar19,&local_118,3,(int)local_3cc + iVar5,iVar6);
      }
LAB_004643ae:
      local_28 = (int *)((int)local_28 + 3);
      local_3c = local_3c + 2;
      a5 = (uint32_t *)((int)a5 + 1);
      iVar5 = iVar5 + 0x18;
      local_7c = local_7c + 1;
    } while ((int)local_3c < 0x7d3c2c);
    if (local_11c == 0) {
      if (local_18 == 0) {
        if ((local_80 != (int32_t *)0x0) && (0 < (int)local_6c[8])) goto LAB_0046441f;
        if (local_20 != 0) {
          if ((a6 & 0x10) == 0) {
            iVar24 = (0xe - local_8c) * 2;
          }
          else {
            iVar24 = ((*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x18)
                     >> 3) * 2 + 0x95;
          }
          goto LAB_00464733;
        }
      }
      uVar12 = 0;
      piVar14 = local_18c + 1;
      do {
        uVar12 = uVar12 + 1;
        iVar5 = piVar14[1];
        uVar16 = uVar12 & 3;
        local_2ec[uVar16 * 8] = piVar14[-1];
        local_2ec[uVar16 * 8 + 1] = *piVar14;
        iVar19 = piVar14[4];
        local_2ec[uVar16 * 8 + 2] = iVar5 + piVar14[3] >> 1;
        local_2ec[uVar16 * 8 + 3] = piVar14[2] + iVar19 >> 1;
        local_2ec[uVar16 * 8 + 4] = piVar14[3];
        local_2ec[uVar16 * 8 + 5] = iVar19;
        iVar5 = local_18c[uVar16 * 6 + 3];
        local_2ec[uVar16 * 8 + 6] = local_18c[uVar16 * 6 + 4] + local_18c[uVar16 * 6 + 2] >> 1;
        local_2ec[uVar16 * 8 + 7] = local_18c[uVar16 * 6 + 5] + iVar5 >> 1;
        piVar14 = piVar14 + 6;
      } while ((int)uVar12 < 4);
      if ((a6 & 0x2000000) == 0) {
        local_28 = g_007f664c;
        a5 = (uint32_t *)0x0;
        local_7c = (int32_t *)0x0;
        local_80 = g_007f6644;
        local_38 = 0;
        do {
          if ((local_18 != 0) ||
             (uVar12 = local_38, piVar14 = local_28, local_108[(int)a5] != 0)) {
            iVar5 = 0;
            puVar21 = local_80;
            puVar20 = local_7c;
            do {
              *puVar21 = 0;
              iVar6 = (((int)a5 + -2 + iVar5 & 3U) + (int)a5 * 4) * 4;
              iVar19 = *(int *)(g_00685ee0 + iVar6);
              puVar20[0x1f4ebc] = *(int *)(g_00685ea0 + iVar6) * local_78 + a9;
              puVar20[0x1f4ebd] = iVar19 * local_98 + a10;
              iVar19 = alt_get_ocean_detail(a7,a8,(int)a5,iVar5);
              uVar15 = (g_006861f0)[iVar19];
              uVar3 = puVar20[0x1a172b];
              *(int32_t *)((int)local_36c + (int)puVar20) = puVar20[0x1a172a];
              *puVar21 = uVar15;
              *(int32_t *)((int)(local_36c + 1) + (int)puVar20) = uVar3;
              piVar14 = local_28;
              uVar12 = local_38;
              iVar5 = iVar5 + 1;
              puVar21 = puVar21 + 1;
              puVar20 = puVar20 + 2;
            } while (iVar5 < 4);
            if ((local_18 != 0) && (local_20 == 0)) {
              piVar22 = local_28 + -2;
              if ((0xf < local_28[-2]) ||
                 (((0xf < local_28[-1] || (0xf < *local_28)) || (0xf < local_28[1])))) {
                local_12c = local_28[-2] + -0x10;
                local_128 = local_28[-1] + -0x10;
                local_124 = *local_28 + -0x10;
                local_120 = local_28[1] + -0x10;
                if (local_1c == 0) {
                  FUN_006233c0(a1,g_007d3af0 + local_38,&local_12c,4,
                               (int)local_36c + local_38,0x41);
                }
                else {
                  FUN_0061b1f0(a1,g_007d3af0 + local_38,&local_12c,4,
                               (int)local_36c + local_38);
                }
              }
              FUN_00624670(a1,g_007d3af0 + uVar12,piVar22,4,(int)local_36c + uVar12,0x10,
                           (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
            }
          }
          local_80 = local_80 + 4;
          a5 = (uint32_t *)((int)a5 + 1);
          local_7c = local_7c + 8;
          local_28 = piVar14 + 4;
          local_38 = uVar12 + 0x20;
        } while ((int)local_80 < 0x7f6684);
      }
      iVar5 = local_94;
      if ((local_18 == 0) || (local_20 == 0)) {
        if (((local_70 == 0) && (((local_34 & 2) != 0 && (-1 < local_94)))) &&
           ((g_00685a98)[local_94 * 2] != -1)) {
          uVar12 = (uint32_t)(char)(g_00685a99)[local_94 * 2];
          uVar16 = uVar12 & 3;
          local_108[5] = (g_007d3c08)[uVar16 * 2];
          local_f0 = (g_007d3c0c)[uVar16 * 2];
          uVar17 = uVar12 + 1 & 3;
          local_d4 = local_b8[uVar16];
          local_ec = (g_007d3c08)[uVar17 * 2];
          local_e8 = (g_007d3c0c)[uVar17 * 2];
          uVar16 = uVar12 - 2 & 3;
          local_d0 = local_b8[uVar17];
          local_e4 = (g_007d3c08)[uVar16 * 2];
          uVar12 = uVar12 - 1 & 3;
          local_e0 = (g_007d3c0c)[uVar16 * 2];
          local_cc = local_b8[uVar16];
          local_dc = (g_007d3c08)[uVar12 * 2];
          local_c8 = local_b8[uVar12];
          local_d8 = (g_007d3c0c)[uVar12 * 2];
          local_6c[0] = 0;
          local_6c[1] = 0;
          local_6c[2] = 0x37;
          local_6c[3] = 0;
          local_6c[4] = 0x37;
          local_6c[5] = 0x37;
          local_6c[6] = 0;
          local_6c[7] = 0x37;
          if (local_94 == 0) {
            FUN_0061a760(a1,local_108 + 5,4,local_6c);
          }
          else {
            FUN_0061c090(a1,local_108 + 5,&local_d4,4,local_6c);
          }
        }
        if (((((local_34 & 0x100) != 0) && (local_70 == 0)) && (-1 < iVar5)) &&
           ((g_00685afc)[iVar5 * 2] != -1)) {
          uVar12 = (uint32_t)(char)(g_00685afd)[iVar5 * 2];
          uVar16 = uVar12 & 3;
          local_108[5] = (g_007d3c08)[uVar16 * 2];
          local_f0 = (g_007d3c0c)[uVar16 * 2];
          uVar17 = uVar12 + 1 & 3;
          local_d4 = local_b8[uVar16];
          local_ec = (g_007d3c08)[uVar17 * 2];
          local_e8 = (g_007d3c0c)[uVar17 * 2];
          uVar16 = uVar12 - 2 & 3;
          local_d0 = local_b8[uVar17];
          local_e4 = (g_007d3c08)[uVar16 * 2];
          uVar12 = uVar12 - 1 & 3;
          local_e0 = (g_007d3c0c)[uVar16 * 2];
          local_cc = local_b8[uVar16];
          local_dc = (g_007d3c08)[uVar12 * 2];
          local_c8 = local_b8[uVar12];
          local_d8 = (g_007d3c0c)[uVar12 * 2];
          local_6c[4] = 0x37;
          local_6c[2] = 0;
          local_6c[3] = 0;
          local_6c[5] = 0;
          local_6c[0] = 0;
          local_6c[6] = 0x37;
          local_6c[7] = 0x37;
          local_6c[1] = 0x37;
          FUN_0061c090(a1,local_108 + 5,&local_d4,4,local_6c);
        }
        if ((((local_34 & 1) != 0) && (local_70 == 0)) &&
           ((-1 < iVar5 && ((g_00685bd4)[iVar5 * 2] != -1)))) {
          uVar12 = (uint32_t)(char)(g_00685bd5)[iVar5 * 2];
          uVar16 = uVar12 & 3;
          local_108[5] = (g_007d3c08)[uVar16 * 2];
          local_f0 = (g_007d3c0c)[uVar16 * 2];
          uVar17 = uVar12 + 1 & 3;
          local_d4 = local_b8[uVar16];
          local_ec = (g_007d3c08)[uVar17 * 2];
          local_e8 = (g_007d3c0c)[uVar17 * 2];
          uVar16 = uVar12 - 2 & 3;
          local_d0 = local_b8[uVar17];
          local_e4 = (g_007d3c08)[uVar16 * 2];
          uVar12 = uVar12 - 1 & 3;
          local_e0 = (g_007d3c0c)[uVar16 * 2];
          local_cc = local_b8[uVar16];
          local_dc = (g_007d3c08)[uVar12 * 2];
          local_c8 = local_b8[uVar12];
          local_d8 = (g_007d3c0c)[uVar12 * 2];
          local_6c[4] = 0x37;
          local_6c[2] = 0;
          local_6c[3] = 0;
          local_6c[5] = 0;
          local_6c[0] = 0;
          local_6c[6] = 0x37;
          local_6c[7] = 0x37;
          local_6c[1] = 0x37;
          FUN_0061c090(a1,local_108 + 5,&local_d4,4,local_6c);
        }
        if ((((((*g_009a6490) & 0x100) != 0) && (local_20 == 0)) || ((a6 & 0x4000000) != 0)) &&
           (((0x5f < (*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0xe0)
             || (local_20 != 0)) || (((*g_009a6494) & 0x800000) != 0)))) {
          iVar5 = a7 + -1;
          if (((*g_0094988c) & 1) == 0) {
            if (iVar5 < 0) {
              iVar5 = iVar5 + (*g_00949870);
            }
            else if ((*g_00949870) <= iVar5) {
              iVar5 = iVar5 - (*g_00949870);
            }
          }
          puVar21 = g_007d3c08;
          uVar12 = 1;
          do {
            iVar19 = a8 + -1;
            if (((-1 < iVar19) && (iVar19 < (*g_00949874))) &&
               ((-1 < iVar5 && (iVar5 < (*g_00949870))))) {
              bVar11 = *(uint8_t *)((*g_0094a30c) + (iVar19 * (*g_0068faf0) + (iVar5 >> 1)) * 0x2c) &
                       0xe0;
              if (bVar11 < 0x60) {
                if ((local_20 != 0) || (((*g_009a6494) & 0x800000) != 0)) {
                  if (0x5f < bVar11) goto LAB_00464c67;
                  goto LAB_00464c8d;
                }
              }
              else {
LAB_00464c67:
                uVar15 = 0x25;
                if ((*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0xe0) <
                    0x60) {
LAB_00464c8d:
                  uVar15 = 0xbe;
                }
                FUN_005e15f0(*puVar21,puVar21[1],(g_007d3c08)[(uVar12 & 3) * 2],
                             (g_007d3c0c)[(uVar12 & 3) * 2],uVar15);
              }
            }
            iVar5 = iVar5 + 2;
            if (((*g_0094988c) & 1) == 0) {
              if (iVar5 < 0) {
                iVar5 = iVar5 + (*g_00949870);
              }
              else if ((*g_00949870) <= iVar5) {
                iVar5 = iVar5 - (*g_00949870);
              }
            }
            puVar21 = puVar21 + 2;
            bVar23 = (int)uVar12 < 2;
            uVar12 = uVar12 + 1;
          } while (bVar23);
        }
        if (((g_009392c0)[(*g_009392b8)] == a7) && ((g_00939340)[(*g_009392b8)] == a8)
           ) {
          puVar21 = g_007d3c08;
          puVar20 = g_007d3be8;
          for (iVar5 = 8; iVar5 != 0; iVar5 = iVar5 + -1) {
            *puVar20 = *puVar21;
            puVar21 = puVar21 + 1;
            puVar20 = puVar20 + 1;
          }
        }
        a9 = a9 + local_78 * -2;
        a10 = a10 - (local_44 + local_78);
        if ((*g_00691e6c) < 1) {
          (*g_00696d1c) = 0x10;
          (*g_00696d18) = a3 + 0x10;
        }
        else {
          (*g_00696d18) = (*g_00691e6c);
          (*g_00696d1c) = (*g_00691e70);
        }
        if ((((a6 & 0x200) != 0) && (local_70 == 0)) &&
           ((uVar12 = *(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c),
            uVar16 = (*g_0078a708), (uVar12 & 0x80000008) == 8 ||
            (uVar16 = (*g_0078a734), (uVar12 & 0x80000400) == 0x400)))) {
          FUN_005e4b9a(a1,uVar16 & 0xff,a9,a10);
        }
        if (((local_30 != 0) && (local_18 == 0)) && (local_70 == 0)) {
          iVar5 = 0;
          do {
            iVar5 = iVar5 + 1;
          } while (iVar5 < 4);
          (*g_009bb548) = 9;
          uVar12 = (uint32_t)(uint8_t)(g_00685485)[(int)local_2c * 2];
          iVar5 = 4;
          puVar8 = local_6c + 1;
          do {
            uVar17 = uVar12 & 3;
            uVar12 = uVar12 + 1;
            iVar5 = iVar5 + -1;
            uVar16 = (g_0068609c)[uVar17 * 6];
            puVar8[-1] = (g_00686098)[uVar17 * 6];
            *puVar8 = uVar16;
            puVar8 = puVar8 + 2;
          } while (iVar5 != 0);
          FUN_0061a760(a1,g_007d3c08,4,local_6c);
        }
        if (local_20 == 0) {
          if ((((local_24 & 0x200000) != 0) && (2 < (int)local_8c)) &&
             (((a6 & 1) != 0 && (local_70 == 0)))) {
            iVar5 = 0;
            do {
              iVar5 = iVar5 + 1;
            } while (iVar5 < 4);
            local_6c[2] = 0x37;
            local_6c[4] = 0x37;
            local_6c[5] = 0x37;
            local_6c[7] = 0x37;
            local_6c[0] = 0;
            local_6c[1] = 0;
            local_6c[3] = 0;
            local_6c[6] = 0;
            (*g_009bb548) = 9;
            FUN_0061c090(a1,g_007d3c08,local_b8,4,local_6c);
          }
          if ((((local_24 & 0x20) != 0) && (1 < (int)local_8c)) &&
             ((local_70 == 0 && (-10 < a3)))) {
            (*g_009bb548) = 9;
            uVar12 = (uint32_t)(uint8_t)(g_00685485)[local_40 * 2];
            iVar5 = 4;
            puVar8 = local_6c + 1;
            do {
              uVar17 = uVar12 & 3;
              uVar12 = uVar12 + 1;
              iVar5 = iVar5 + -1;
              uVar16 = (g_0068609c)[uVar17 * 6];
              puVar8[-1] = (g_00686098)[uVar17 * 6];
              *puVar8 = uVar16;
              puVar8 = puVar8 + 2;
            } while (iVar5 != 0);
            FUN_006239e0(a1,g_007d3c08,4,local_6c,
                         (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
          }
        }
        if ((local_c0 != 0) &&
           (((((*(uint32_t *)(g_0096c9e0 + (*g_00939284) * 0x20cc) & 0x200) != 0 ||
              ((*(uint8_t *)((*g_0094a30c) + 4 + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
               (uint8_t)(1 << ((uint8_t)(*g_00939284) & 0x1f))) != 0)) ||
             (((uint8_t)(*g_009a64c0) & 0x80) != 0)) && (local_20 == 0)))) {
          local_6c[8] = whose_territory((*g_00939284),a7,a8,0,0);
          if (-1 < (int)local_6c[8]) {
            local_4cc[1] = '\0';
            local_4cc[2] = '\0';
            local_4cc[3] = '\0';
            local_4cc[4] = '\0';
            local_4cc[0] = '\0';
            local_4cc[5] = '\0';
            local_4cc[6] = '\0';
            local_4cc[7] = '\0';
            local_4cc[8] = '\0';
            local_44 = 0;
            local_4cc[9] = 0;
            local_c0 = (uint32_t)*(uint8_t *)((*g_0094a30c) + 3 + ((*g_0068faf0) * a8 + local_14) * 0x2c
                                      );
            a5 = (uint32_t *)0x0;
            do {
              iVar5 = (g_0066ef50)[(int)a5] + a7;
              if (((*g_0094988c) & 1) == 0) {
                if (iVar5 < 0) {
                  iVar5 = (*g_00949870) + iVar5;
                }
                else if ((*g_00949870) <= iVar5) {
                  iVar5 = iVar5 - (*g_00949870);
                }
              }
              iVar19 = (g_0066ef74)[(int)a5] + a8;
              if (((-1 < iVar19) && (iVar19 < (*g_00949874))) &&
                 ((((-1 < iVar5 &&
                    ((iVar5 < (*g_00949870) &&
                     (pbVar18 = (uint8_t *)((*g_0094a30c) +
                                        (iVar19 * (*g_0068faf0) + (iVar5 >> 1)) * 0x2c),
                     (*pbVar18 & 0xe0) < 0x60 ==
                     (*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0xe0) <
                     0x60)))) && (pbVar18[3] == local_c0)) &&
                  (uVar12 = whose_territory((*g_00939284),iVar5,iVar19,0,0), local_6c[8] != uVar12)))) {
                local_44 = 1;
                cVar4 = (char)local_6c[8];
                if (((uint32_t)a5 & 1) == 0) {
                  if (a5 == (uint32_t *)0x0) {
                    local_4cc[8] = cVar4 + '\x01';
                    local_4cc[2] = cVar4 + '\x01';
                    /* WARNING: Ignoring partial resolution of indirect */
                    local_4cc[1] = cVar4 + '\x01';
                    goto LAB_004652ed;
                  }
                  local_4cc[(int)a5] = cVar4 + '\x01';
                  local_4cc[(int)a5 + 2] = cVar4 + '\x01';
                }
                local_4cc[(int)a5 + 1] = cVar4 + '\x01';
              }
LAB_004652ed:
              a5 = (uint32_t *)((int)a5 + 1);
            } while ((int)a5 < 8);
            if (local_44 != 0) {
              local_6c[2] = 0x3b;
              local_6c[4] = 0x3b;
              local_6c[5] = 0x3b;
              local_6c[7] = 0x3b;
              local_6c[0] = 0;
              local_6c[1] = 0;
              local_6c[3] = 0;
              local_6c[6] = 0;
              FUN_0061e540(a1,g_007d3c08,local_4cc,g_006f077c,4,local_6c);
            }
          }
        }
        if ((-8 < a3) && (local_18 == 0)) {
          a5 = local_108;
          iVar5 = 0;
          puVar21 = g_007f6644;
          do {
            uVar12 = *a5;
            if (uVar12 != 0) {
              iVar19 = 0;
              pbVar18 = (uint8_t *)((char *)g_00684730 + uVar12);
              do {
                bVar11 = *pbVar18;
                local_5cc[iVar19] = bVar11;
                if (1 < bVar11) {
                  local_5cc[iVar19] = (g_00787e56)[bVar11];
                }
                iVar19 = iVar19 + 1;
                pbVar18 = pbVar18 + 8;
              } while (iVar19 < 0x35);
              if ((uVar12 == 7) && ((a8 - a7 & 2) != 0)) {
                local_5cc[5] = 0;
                local_5cc[0xe] = 0;
                local_5cc[0xf] = 0;
                local_5cc[0x10] = 0;
                local_5cc[0x11] = 0;
                iVar19 = 0x12;
                pbVar18 = (uint8_t *)g_00787e5b;
                do {
                  local_5cc[iVar19] = *pbVar18;
                  iVar19 = iVar19 + 1;
                  pbVar18 = pbVar18 + -1;
                } while (0x787e57 < (int)pbVar18);
                iVar19 = 0x16;
                pbVar18 = (uint8_t *)g_00787e5b;
                do {
                  local_5cc[iVar19] = *pbVar18;
                  iVar19 = iVar19 + 1;
                  pbVar18 = pbVar18 + -1;
                } while (0x787e57 < (int)pbVar18);
                iVar19 = 0x1a;
                pbVar18 = (uint8_t *)g_00787e5b;
                do {
                  local_5cc[iVar19] = *pbVar18;
                  iVar19 = iVar19 + 1;
                  pbVar18 = pbVar18 + -1;
                } while (0x787e57 < (int)pbVar18);
              }
              iVar6 = 4;
              iVar19 = iVar5;
              do {
                *(int32_t *)((int)local_36c + iVar19) =
                     *(int32_t *)((int)auStack_26c + iVar19);
                *(int32_t *)((int)local_36c + iVar19 + 4) =
                     *(int32_t *)((int)auStack_26c + iVar19 + 4);
                iVar19 = iVar19 + 8;
                iVar6 = iVar6 + -1;
              } while (iVar6 != 0);
              FUN_00623f80(a1,(int)local_2ec + iVar5,puVar21,g_0076e8a0,local_5cc,4,
                           (int)local_36c + iVar5,(-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
            }
            puVar21 = puVar21 + 4;
            a5 = a5 + 1;
            iVar5 = iVar5 + 0x20;
          } while ((int)puVar21 < 0x7f6684);
        }
        if ((((a6 & 0x400) != 0) &&
            ((*(uint8_t *)((*g_0094a30c) + 8 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x80) != 0)
            ) && (local_18 == 0)) {
          iVar5 = 0;
          local_3c = (int32_t *)((*g_0094988c) & 1);
          do {
            iVar5 = iVar5 + 1;
          } while (iVar5 < 4);
          local_6c[0] = 0;
          local_6c[2] = 0x37;
          local_6c[4] = 0x37;
          local_6c[5] = 0x37;
          local_6c[7] = 0x37;
          local_6c[1] = 0;
          local_6c[3] = 0;
          local_6c[6] = 0;
          iVar5 = (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
          (*g_009bb548) = 9;
          FUN_00622db0(a1,g_007d3c08,local_b8,4,local_6c,iVar5);
          if ((*(uint32_t *)((*g_0094a30c) + 8 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x200) !=
              0) {
            FUN_00622db0(a1,g_007d3c08,local_b8,4,local_6c,iVar5);
          }
        }
        goto LAB_00465605;
      }
      iVar24 = 0xbe;
    }
    else if (local_18 == 0) {
      if (local_6c[8] == 0xffffffff) {
        iVar24 = 0x11;
      }
      else {
LAB_0046441f:
        iVar24 = (g_0068f9f0)[local_6c[8]];
      }
    }
    else {
      iVar24 = 0xa1;
    }
LAB_00464733:
    a1->poly(reinterpret_cast<Vert *>(g_007d3c08),4,iVar24);
    (*g_006972cc) = 0xffffff00;
    (*g_006972d0) = 0x100;
LAB_00464756:
    return 1;
  }
LAB_00465605:
  pbVar18 = (uint8_t *)(*g_0068547c);
  if ((*g_00691e6c) < 1) {
    (*g_00696d1c) = 0x10;
    (*g_00696d18) = a3 + 0x10;
  }
  else {
    (*g_00696d18) = (*g_00691e6c);
    (*g_00696d1c) = (*g_00691e70);
  }
  if (((local_20 != 0) && (local_48 == (uint8_t *)0x0)) ||
     ((-1 < (int)a4 &&
      (((*(uint32_t *)(g_0096c9e0 + (int)a4 * 0x20cc) & 0x200) == 0 &&
       ((*(uint8_t *)((*g_0094a30c) + 4 + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
        (uint8_t)(1 << (bVar10 & 0x1f))) == 0)))))) goto LAB_00466c32;
  if ((((*g_009a6490) & 0x200) != 0) && (local_20 == 0)) {
    uVar12 = *(uint8_t *)((*g_0094a30c) + 5 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 7;
    if (((-1 < (int)a4) && ((int *)uVar12 != a4)) &&
       (((g_0096c9f8)[(int)a4 * 0x20cc + uVar12 * 4] & 8) == 0)) {
      uVar12 = 0;
    }
    iVar5 = abs(a7 - (*g_00685478));
    if ((((*g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar5)) {
      iVar5 = (*g_00949870) - iVar5;
    }
    uVar15 = abs(a8 - (int)pbVar18);
    iVar19 = abs(iVar5);
    iVar6 = abs(uVar15);
    pbVar18 = (uint8_t *)(*g_0068547c);
    iVar5 = iVar19;
    if (iVar19 <= iVar6) {
      iVar5 = iVar6;
    }
    iVar7 = iVar19;
    if (iVar6 <= iVar19) {
      iVar7 = iVar6;
    }
    if ((iVar5 - (((iVar19 + iVar6 >> 1) - iVar7) + 1 >> 1) < 3) + uVar12 == 0) {
      iVar5 = abs(a7 - (*g_00685478));
      if ((((*g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar5)) {
        iVar5 = (*g_00949870) - iVar5;
      }
      uVar15 = abs(a8 - (int)pbVar18);
      iVar19 = abs(iVar5);
      iVar6 = abs(uVar15);
      iVar5 = iVar19;
      if (iVar19 <= iVar6) {
        iVar5 = iVar6;
      }
      iVar7 = iVar19;
      if (iVar6 <= iVar19) {
        iVar7 = iVar6;
      }
      local_44 = (-1 < (int)((iVar5 - (((iVar19 + iVar6 >> 1) - iVar7) + 1 >> 1) < 3) + uVar12)) - 1
      ;
    }
    else {
      local_44 = 1;
    }
    local_30 = 1;
    local_2c = (uint32_t *)0x0;
    do {
      iVar5 = local_2c[0x19bbca] + a7;
      if (((*g_0094988c) & 1) == 0) {
        if (iVar5 < 0) {
          iVar5 = iVar5 + (*g_00949870);
        }
        else if ((*g_00949870) <= iVar5) {
          iVar5 = iVar5 - (*g_00949870);
        }
      }
      local_34 = local_2c[0x19bbcf] + a8;
      if ((((-1 < (int)local_34) && ((int)local_34 < (*g_00949874))) && (-1 < iVar5)) &&
         (iVar5 < (*g_00949870))) {
        a5 = (uint32_t *)(*(uint8_t *)((*g_0094a30c) + 5 +
                                    (local_34 * (*g_0068faf0) + (iVar5 >> 1)) * 0x2c) & 7);
        if (((-1 < (int)a4) && (a5 != (uint32_t *)a4)) &&
           (((g_0096c9f8)[(int)a5 * 4 + (int)a4 * 0x20cc] & 8) == 0)) {
          a5 = (uint32_t *)0x0;
        }
        local_48 = (uint8_t *)(*g_0068547c);
        iVar19 = abs(iVar5 - (*g_00685478));
        if ((((*g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar19)) {
          iVar19 = (*g_00949870) - iVar19;
        }
        local_48 = (uint8_t *)abs(local_34 - (int)local_48);
        iVar6 = abs(iVar19);
        iVar7 = abs((int)local_48);
        iVar19 = iVar6;
        if (iVar6 <= iVar7) {
          iVar19 = iVar7;
        }
        iVar13 = iVar6;
        if (iVar7 <= iVar6) {
          iVar13 = iVar7;
        }
        if ((uint32_t)(iVar19 - (((iVar6 + iVar7 >> 1) - iVar13) + 1 >> 1) < 3) + (int)a5 == 0) {
          local_48 = (uint8_t *)(*g_0068547c);
          iVar5 = abs(iVar5 - (*g_00685478));
          if ((((*g_0094988c) & 1) == 0) && ((*g_0068faf0) < iVar5)) {
            iVar5 = (*g_00949870) - iVar5;
          }
          uVar15 = abs(local_34 - (int)local_48);
          iVar19 = abs(iVar5);
          iVar6 = abs(uVar15);
          iVar5 = iVar19;
          if (iVar19 <= iVar6) {
            iVar5 = iVar6;
          }
          iVar7 = iVar19;
          if (iVar6 <= iVar19) {
            iVar7 = iVar6;
          }
          uVar12 = (-1 < (int)((uint32_t)(iVar5 - (((iVar19 + iVar6 >> 1) - iVar7) + 1 >> 1) < 3) +
                              (int)a5)) - 1;
        }
        else {
          uVar12 = 1;
        }
        if (uVar12 != local_44) {
          uVar12 = local_30 + 1 & 3;
          FUN_005e15f0((g_007d3c08)[(local_30 & 3) * 2],(g_007d3c0c)[(local_30 & 3) * 2],
                       (g_007d3c08)[uVar12 * 2],(g_007d3c0c)[uVar12 * 2],0xf9);
        }
      }
      local_2c = local_2c + 1;
      uVar12 = local_30 + 1;
      bVar23 = (int)local_30 < 4;
      local_30 = uVar12;
    } while (bVar23);
  }
  if ((((a6 & 1) != 0) && ((local_24 & 0x8000) != 0)) && (local_18 != 0)) {
    FUN_005e4b9a(a1,(*g_007a6868) & 0xff,a9,a10);
  }
  local_30 = a6 & 0x20;
  if (local_30 != 0) {
    local_88 = (int *)base_at(a7,a8);
    if (((-1 < (int)local_88) && (-1 < (int)a4)) &&
       (((int *)(uint32_t)(uint8_t)(g_0097d044)[(int)local_88 * 0x134] != a4 &&
        (((g_0097d04a)[(int)local_88 * 0x134] & (uint8_t)(1 << (bVar10 & 0x1f))) == 0)))) {
      local_88 = (int *)0xffffffff;
    }
    if ((local_18 == 0) && (((local_24 & 4) != 0 || (-1 < (int)local_88)))) {
      if (((local_24 & 8) != 0) || (bVar23 = false, -1 < (int)local_88)) {
        bVar23 = true;
      }
      local_94 = 0;
      local_2c = (uint32_t *)0x0;
      a5 = (uint32_t *)0x0;
      do {
        iVar5 = (g_0066ef50)[(int)a5] + a7;
        if (((*g_0094988c) & 1) == 0) {
          if (iVar5 < 0) {
            iVar5 = (*g_00949870) + iVar5;
          }
          else if ((*g_00949870) <= iVar5) {
            iVar5 = iVar5 - (*g_00949870);
          }
        }
        local_34 = (g_0066ef74)[(int)a5] + a8;
        if ((((-1 < (int)local_34) && ((int)local_34 < (*g_00949874))) && (-1 < iVar5)) &&
           (iVar5 < (*g_00949870))) {
          iVar19 = local_34 * (*g_0068faf0) + (iVar5 >> 1);
          local_48 = (uint8_t *)((*g_0094a30c) + iVar19 * 0x2c);
          if (0x5f < (*local_48 & 0xe0)) {
            if ((int)a4 < 0) {
              local_44 = *(uint32_t *)(local_48 + 8);
            }
            else {
              local_44 = *(uint32_t *)((*g_0094a30c) + 0xc + ((int)a4 + iVar19 * 0xb) * 4);
            }
            iVar5 = base_at(iVar5,local_34);
            if (((-1 < iVar5) && (-1 < (int)a4)) &&
               (((int *)(uint32_t)(uint8_t)(g_0097d044)[iVar5 * 0x134] != a4 &&
                (((g_0097d04a)[iVar5 * 0x134] & (uint8_t)(1 << (bVar10 & 0x1f))) == 0)))) {
              iVar5 = -1;
            }
            if (((local_44 & 4) != 0) || (-1 < iVar5)) {
              local_94 = 1;
              local_6c[0] = 0;
              local_6c[1] = 0;
              local_6c[3] = 0;
              local_6c[2] = 0x37;
              local_6c[4] = 0x37;
              local_6c[5] = 0x37;
              local_6c[7] = 0x37;
              local_6c[6] = 0;
              if ((bVar23) && (((local_44 & 8) != 0 || (-1 < iVar5)))) {
                FUN_0061c090(a1,g_007d3c08,local_b8,4,local_6c);
                local_2c = (uint32_t *)0x1;
              }
              else {
                FUN_0061c090(a1,g_007d3c08,local_b8,4,local_6c);
              }
            }
          }
        }
        a5 = (uint32_t *)((int)a5 + 1);
      } while ((int)a5 < 8);
      if (((int)local_88 < 0) && (((bVar23 && (local_2c == (uint32_t *)0x0)) || (local_94 == 0)))) {
        local_6c[0] = 0;
        local_6c[1] = 0;
        local_6c[2] = 0x37;
        local_6c[3] = 0;
        local_6c[4] = 0x37;
        local_6c[5] = 0x37;
        local_6c[6] = 0;
        local_6c[7] = 0x37;
        FUN_0061c090(a1,g_007d3c08,local_b8,4,local_6c);
      }
    }
  }
  if ((((local_30 != 0) && ((local_24 & 0x8000) != 0)) && (-8 < a3)) ||
     ((a6 & 0x1000) != 0)) {
    crop_diff = crop_yield(local_6c[8],0xffffffff,a7,a8,0);
  }
  if ((((a6 & 0x200) != 0) && (iVar5 = bonus_at(a7,a8,0), iVar5 != 0)) &&
     (-8 < a3)) {
    if (iVar5 == 4) {
      FUN_005e4b9a(iVar24,*(uint8_t *)(g_0078a708 + (uint32_t)(local_18 != 0) * 0xb),a9,
                   a10);
    }
    else {
      FUN_005e5833(iVar24,(g_0075b238)
                          [(((local_14 + a8 & 1U) - 4) +
                           ((uint32_t)(local_18 != 0) + iVar5 * 2) * 2) * 0x2c],a9,a10,
                   g_006f107c);
    }
  }
  if (local_30 != 0) {
    if ((((local_24 & 0x800) != 0) && (-8 < a3)) && (local_18 == 0)) {
      FUN_005e4b9a(iVar24,(*g_007ad018) & 0xff,a9,a10);
    }
    if ((((local_24 & 0x40000) != 0) && (-8 < a3)) && (local_18 == 0)) {
      FUN_005e4b9a(iVar24,(*g_007acc68) & 0xff,a9,a10);
    }
    if (((local_24 & 0x80000000) != 0) && (-8 < a3)) {
      FUN_005e5833(iVar24,(*g_007a6838) & 0xff,a9,a10,g_006f107c);
    }
    if (local_30 != 0) {
      uVar12 = local_24 & 0x8000;
      if (((uVar12 != 0) && (-8 < a3)) && (local_18 == 0)) {
        if ((local_24 & 0x80000) == 0) {
          iVar5 = crop_diff + -1;
          if (iVar5 < 0) {
            iVar5 = 0;
          }
          else if (3 < iVar5) {
            iVar5 = 3;
          }
          uVar1 = (g_00791fd0)[iVar5 * 0x2c];
        }
        else {
          iVar5 = crop_diff + -1;
          if (iVar5 < 0) {
            iVar5 = 0;
          }
          else if (3 < iVar5) {
            iVar5 = 3;
          }
          uVar1 = *(uint8_t *)(g_0079a718 + iVar5 * 0xb);
        }
        FUN_005e4b9a(iVar24,uVar1,a9,a10);
      }
      if ((((local_24 & 0x10) != 0) && (-8 < a3)) &&
         ((uVar16 = (*g_00776a58), local_18 == 0 ||
          ((uVar16 = (*g_00789b70), (*g_009a6488) != 0 || (local_8c == 2)))))) {
        FUN_005e4b9a(iVar24,uVar16 & 0xff,a9,a10);
      }
      if ((((local_24 & 0x40) != 0) && (-8 < a3)) &&
         ((uVar16 = (*g_00799200), local_18 == 0 ||
          ((uVar16 = (*g_0078a5b8), (*g_009a6488) != 0 || (local_8c == 2)))))) {
        FUN_005e4b9a(iVar24,uVar16 & 0xff,a9,a10);
      }
      if (((local_24 & 0x400000) != 0) && (-8 < a3)) {
        FUN_005e4b9a(iVar24,(*g_00779398) & 0xff,a9,a10);
      }
      if (((local_24 & 0x800000) != 0) && (-8 < a3)) {
        FUN_005e4b9a(iVar24,(*g_007abf68) & 0xff,a9,a10);
      }
      if (((local_24 & 0x1000000) != 0) && (-8 < a3)) {
        uVar16 = (*g_0078dd28);
        if ((*(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x80001000
            ) == 0x1000) {
          uVar16 = (*g_007acb90);
        }
        FUN_005e4b9a(iVar24,uVar16 & 0xff,a9,a10);
      }
      if ((((uVar12 != 0) && (-8 < a3)) && (local_18 == 0)) &&
         ((4 < crop_diff && ((local_24 & 0x80000) != 0)))) {
        FUN_005e4b9a(iVar24,(*g_0079a718) & 0xff,a9,a10);
      }
      uVar12 = *(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c);
      if ((uVar12 & 0x80002000) == 0x2000) {
        iVar5 = (int)uVar12 >> 0x18;
        if (0x1ffffff < (int)(uVar12 & 0xff000000)) {
          if ((int)(uVar12 & 0xff000000) < 0x4000001) goto LAB_004661b9;
          iVar5 = iVar5 + -3;
        }
        FUN_005e4b9a(iVar24,(g_007ac0a0)[iVar5 * 0x2c],a9,a10);
      }
LAB_004661b9:
      uVar12 = *(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c);
      if (((uVar12 & 0x80004000) == 0x4000) && ((*g_009a6488) != 0)) {
        uVar16 = uVar12 & 0xff000000;
        if (uVar16 == 0) {
          iVar5 = goody_at(a7,a8);
          if (iVar5 == 0) {
            uVar12 = (*g_00799660) & 0xff;
          }
          else {
            uVar12 = (*g_00776760) & 0xff;
          }
        }
        else if ((int)uVar16 < 0x8000000) {
          uVar12 = (uint32_t)*(uint8_t *)(g_00776760 + ((int)uVar12 >> 0x18) * 0xb);
        }
        else if (uVar16 == 0x8000000) {
          iVar5 = goody_at(a7,a8);
          if (iVar5 == 0) {
            uVar12 = (*g_0079968c) & 0xff;
          }
          else {
            uVar12 = (*g_007768c0) & 0xff;
          }
        }
        else if (uVar16 == 0xa000000) {
          iVar5 = goody_at(a7,a8);
          if (iVar5 == 0) {
            uVar12 = (*g_007996b8) & 0xff;
          }
          else {
            uVar12 = (*g_007768ec) & 0xff;
          }
        }
        else if (uVar16 == 0xc000000) {
          uVar12 = (*g_00776918) & 0xff;
        }
        else if (uVar16 == 0xf000000) {
          uVar12 = (*g_00776944) & 0xff;
        }
        else if (uVar16 == 0x10000000) {
          uVar12 = (*g_00776970) & 0xff;
        }
        else if (uVar16 == 0x13000000) {
          iVar5 = goody_at(a7,a8);
          if (iVar5 == 0) {
            uVar12 = (*g_007996e4) & 0xff;
          }
          else {
            uVar12 = (*g_0077699c) & 0xff;
          }
        }
        else {
          if (uVar16 != 0x14000000) goto LAB_004663cf;
          uVar12 = (*g_007769c8) & 0xff;
        }
        FUN_005e4b9a(iVar24,uVar12,a9,a10);
      }
LAB_004663cf:
      uVar12 = *(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c);
      if ((((uVar12 & 0x80008000) == 0x8000) && ((*g_009a6488) != 0)) &&
         ((int)(uVar12 & 0xff000000) < 0x6000000)) {
        FUN_005e4b9a(iVar24,(g_007acd78)[((int)uVar12 >> 0x18) * 0x2c],a9,a10);
      }
    }
  }
  if (((a6 & 0x200) != 0) && (-8 < a3)) {
    iVar5 = goody_at(a7,a8);
    if ((iVar5 != 0) &&
       ((uVar12 = *(uint32_t *)((*g_0094a30c) + 0xc + ((*g_0068faf0) * a8 + local_14) * 0x2c),
        (uVar12 & 0x80004000) != 0x4000 ||
        (((uVar12 = uVar12 & 0xff000000, uVar12 != 0 && (uVar12 != 0x8000000)) &&
         ((uVar12 != 0xa000000 && (uVar12 != 0x13000000)))))))) {
      FUN_005e4b9a(iVar24,(g_0077b000)
                          [(local_18 + ((int)(a7 * 7 + a8 * 5) % 3) * 2) * 0x2c],a9,
                   a10);
    }
    if ((*(uint32_t *)((*g_0094a30c) + 8 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x2000) != 0) {
      FUN_005e5833(iVar24,(*g_0075b0a0) & 0xff,a9,a10,g_006f107c);
    }
  }
  if (local_18 == 0) {
    if ((a6 & 8) != 0) {
      FUN_005e4b9a(iVar24,(g_00791c60)
                          [(local_90 +
                           (*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 7
                           ) * 2) * 0x2c],a9,a10);
    }
    if ((local_20 == 0) && ((a6 & 0x10) != 0)) {
      if ((local_90 != 0) && ((a6 & 8) != 0)) {
        local_90 = local_90 + 1;
      }
      FUN_005e4b9a(iVar24,(g_007a99a8)
                          [(local_90 +
                           ((*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
                            0x18) >> 3) * 3) * 0x2c],a9,a10);
    }
  }
  if (a2 != 0) {
    iVar24 = local_78;
    if ((a6 & 4) != 0) {
      (*g_009b86a0) = 0;
      iVar24 = (*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c;
      if ((*(uint32_t *)(iVar24 + 8) & 0x1000) == 0) {
        uVar12 = (uint32_t)(*(uint8_t *)(iVar24 + 2) >> 4);
        if (uVar12 == 0) {
          uVar12 = world_site(a7,a8,0);
          site_set(a7,a8,uVar12);
        }
      }
      else {
        a7 = 0;
        uVar12 = a7;
      }
      a7 = uVar12;
      _itoa(a7,(char *)local_23c,10);
      strcat((char *)g_009b86a0,(char *)local_23c);
      iVar24 = local_78;
      local_a8 = a9;
      local_a4 = a10;
      local_9c = a10 + local_78 * 2;
      local_a0 = a9 + local_78 * 4;
      a1->set_text_color(((0x5f < (*(uint8_t *)((*g_0094a30c) + ((*g_0068faf0) * a8 + local_14) * 0x2c) &
                            0xe0)) - 1 & 0xfffffffd) + 0xff,0,1,1);
      a1->set_font(reinterpret_cast<Font *>(reinterpret_cast<char *>(a2) + 0x1eef4),0,0,0);
      if ((a7 != 0) && (local_18 == 0)) {
        sVar9 = strlen((char *)g_009b86a0);
        a1->write_cent_l(reinterpret_cast<char *>(g_009b86a0),reinterpret_cast<RECT *>(&local_a8),sVar9);
      }
    }
    if ((a6 & 0x4000) != 0) {
      (*g_009b86a0) = 0;
      _itoa(*(int32_t *)((*g_00945b00) + (((*g_00949870) >> 1) * a8 + local_14) * 4),
                   (char *)local_23c,10);
      strcat((char *)g_009b86a0,(char *)local_23c);
      a1->set_text_color(0xff,0,1,1);
      a1->set_font(reinterpret_cast<Font *>(reinterpret_cast<char *>(a2) + 0x1eef4),0,0,0);
      local_a8 = a9;
      local_a4 = a10;
      local_a0 = a9 + iVar24 * 4;
      local_9c = a10 + iVar24 * 2;
      sVar9 = strlen((char *)g_009b86a0);
      a1->write_cent_l(reinterpret_cast<char *>(g_009b86a0),reinterpret_cast<RECT *>(&local_a8),sVar9);
    }
    if (((a6 & 0x80000) != 0) &&
       (iVar5 = (*g_0068faf0) * a8 + local_14,
       (*(uint8_t *)((*g_0094a30c) + 5 + iVar5 * 0x2c) & 0x38) != 0)) {
      local_a8 = a9;
      local_a4 = a10;
      local_a0 = a9 + iVar24 * 4;
      local_9c = a10 + iVar24 * 2;
      a1->set_text_color((g_0068f9f0)[(*(uint8_t *)((*g_0094a30c) + 5 + iVar5 * 0x2c) & 0x38) >> 3],0,1,1);
      a1->set_font(reinterpret_cast<Font *>(reinterpret_cast<char *>(a2) + 0x1eef4),0,0,0);
      (*g_009b86a0) = 0;
      _itoa((*(uint8_t *)((*g_0094a30c) + 5 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x38)
                   >> 3,(char *)local_23c,10);
      strcat((char *)g_009b86a0,(char *)local_23c);
      sVar9 = strlen((char *)g_009b86a0);
      a1->write_cent_l(reinterpret_cast<char *>(g_009b86a0),reinterpret_cast<RECT *>(&local_a8),sVar9);
    }
    if (((a6 & 0x80000) != 0) &&
       (iVar5 = (*g_0068faf0) * a8 + local_14,
       (*(uint8_t *)((*g_0094a30c) + 5 + iVar5 * 0x2c) & 0x38) != 0)) {
      local_a8 = a9;
      local_a4 = a10;
      local_a0 = a9 + iVar24 * 4;
      local_9c = a10 + iVar24 * 2;
      a1->set_text_color((g_0068f9f0)[(*(uint8_t *)((*g_0094a30c) + 5 + iVar5 * 0x2c) & 0x38) >> 3],0,1,1);
      a1->set_font(reinterpret_cast<Font *>(reinterpret_cast<char *>(a2) + 0x1eef4),0,0,0);
      (*g_009b86a0) = 0;
      _itoa((*(uint8_t *)((*g_0094a30c) + 5 + ((*g_0068faf0) * a8 + local_14) * 0x2c) & 0x38)
                   >> 3,(char *)local_23c,10);
      strcat((char *)g_009b86a0,(char *)local_23c);
      sVar9 = strlen((char *)g_009b86a0);
      a1->write_cent_l(reinterpret_cast<char *>(g_009b86a0),reinterpret_cast<RECT *>(&local_a8),sVar9);
    }
  }
  local_108[4] = 1;
LAB_00466c32:
  if ((0 < (*g_00691e6c)) && (a2 != 0)) {
    *(int32_t *)(reinterpret_cast<char *>(a2) + 8) = 99999;
  }
  (*g_00696d18) = 1;
  (*g_00696d1c) = 1;
  (*g_006972cc) = 0xffffff00;
  (*g_006972d0) = 0x100;
  return local_108[4];
LAB_00463b55:
  local_1c = 1;
  uVar12 = *local_2c & 0xffdfffff;
  goto LAB_00463603;
}

