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
#include "stdafx.h"

// Separate writable comparison strings identified by the pinned operand sites.
// These descriptive names do not claim original names or allocation bounds.
// RECOVERY_OBJECT: v1 0x0068F268 _TechNameNoneText
static char TechNameNoneText[] = "None";
// RECOVERY_OBJECT: v1 0x0068F270 _TechNameDisableText
static char TechNameDisableText[] = "Disable";
// RECOVERY_OBJECT: v1 0x0068F284 _ChassisNameNoneText
static char ChassisNameNoneText[] = "None";
// RECOVERY_OBJECT: v1 0x0068F28C _ChassisNameDisableText
static char ChassisNameDisableText[] = "Disable";
// RECOVERY_OBJECT: v1 0x0068F2A0 _WeaponNameNoneText
static char WeaponNameNoneText[] = "None";
// RECOVERY_OBJECT: v1 0x0068F2A8 _WeaponNameDisableText
static char WeaponNameDisableText[] = "Disable";
// RECOVERY_OBJECT: v1 0x0068F2BC _ArmorNameNoneText
static char ArmorNameNoneText[] = "None";
// RECOVERY_OBJECT: v1 0x0068F2C4 _ArmorNameDisableText
static char ArmorNameDisableText[] = "Disable";
// RECOVERY_OBJECT: v1 0x0068F278 _TechNameBadKey
static char TechNameBadKey[] = "BADTECHKEY";
// RECOVERY_OBJECT: v1 0x0068F294 _ChassisNameBadKey
static char ChassisNameBadKey[] = "BADCHASKEY";
// RECOVERY_OBJECT: v1 0x0068F2B0 _WeaponNameBadKey
static char WeaponNameBadKey[] = "BADWEAPKEY";
// RECOVERY_OBJECT: v1 0x0068F2CC _ArmorNameBadKey
static char ArmorNameBadKey[] = "BADARMKEY";

#include "xpops.h"  // X_pop
#include "alpha.h"
#include "base.h"
#include "buffer.h"  // BufferJoinSpace, shared by the original preference builder
#include "council.h"
#include "game.h"
#include "halloffame.h"
#include "general.h"
#include "map.h"
#include "mapwin.h"  // MainInterfaceGlobal
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "terraforming.h"
#include "text.h"
#include "textindex.h"
#include "veh.h"
#include "wave.h"

// ORIGINAL: 0x00584D30 sub_584d30 0x00584D30-0x00584D52 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f7fc8f78d783f09c46cddc646c31c2f5eb6d013:src/recovered/00584d30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x584d30 complete original body; independent source-owner review.", "source:src/general.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_584d30@@YAPADPAD0I@Z
char *__cdecl sub_584d30(char *dest, char *src, unsigned int count) {
    char *result = strncpy(dest, src, count);
    dest[count - 1] = 0;
    return result;
}

// Equal text does not imply one original object: the pinned operands below
// identify separate instances. These role names and const arrays describe the
// reconstruction; original declarations, qualifiers and full bounds are unknown.
// RECOVERY_OBJECT: v1 0x0068F2F0 _alpha_tech_identifiers_section
static const char alpha_tech_identifiers_section[] = "TECHNOLOGY";
// RECOVERY_OBJECT: v1 0x0068F30C _alpha_tech_values_section
static const char alpha_tech_values_section[] = "TECHNOLOGY";

// read_tech's prologue loads the pointer variable (`mov eax, dword ptr
// [0x691AFC]` at 0x00585E34) ahead of text_open; the DWORD there points at
// the "ALPHAX" literal at 0x00691B58.
// RECOVERY_OBJECT: v2 0x00691AFC 0x00691B00 ?AlphaxFileID@@3PBDB 0x0048207D
// The initialiser's own literal: the image DWORD at 0x00691AFC holds
// 0x00691B58, where the seven bytes "ALPHAX\0" sit.
// RECOVERY_LITERAL: v1 0x00691B58 0x00691B5F $SG52217 0x0048207D
LPCSTR AlphaxFileID = "ALPHAX";
// Alpha owns the parsed #COMPASS pointer IDs populated by read_rules.
// RECOVERY_OBJECT: v1 0x00945D48 ?Compass@@3PAPADA
LPSTR Compass[MaxCompassNum];      // 0x00945D48

LPSTR Difficulty[MaxDiffNum];      // 0x0096C85C

// ===== THE RULES OBJECTS - real storage, homed to the parser =====
// Each address is zero-initialised data in the shipped image (image_data.py:
// all sixteen sit in .data past the bytes the file carries, ZERO at run
// time), filled in by read_rules()/read_basic_rules()/read_tech() below.
// They were typed-pointer bindings into the image's address space until
// converted to the objects themselves; the folded absolute addressing of
// every use site is unchanged, the immediate now a relocation.
RulesResourceinfo ResourceInfo[MaxResourceInfoNum]; // 0x00945F50
// RECOVERY_OBJECT: v1 0x0094F1B8 ?TimeControl@@3PAURulesTimeControl@@A
RulesTimeControl TimeControl[MaxTimeControlNum];    // 0x0094F1B8
// Convoy 004D907E/004D90EF and the rules parser independently identify this
// existing resource-table start; lifetime remains open.
// Bound: the loader fills 4 x 8-byte records from +4 to `cmp esi, 0x94617c` (0x00587D75).
// RECOVERY_OBJECT: v2 0x00946158 0x00946178 ?Resource@@3PAURulesResource@@A 0x00587D75
RulesResource Resource[MaxResourceNum];             // 0x00946158
RulesEnergy Energy[MaxEnergyNum];                   // 0x0094A318
// NetDaemon::synch sizes opcode 0x81, this whole window, as 0x13C bytes
// (`mov esi, 0x13c` at 0x00533226; `mov esi, 0x949730` at 0x00533A5C).
// RECOVERY_OBJECT: v2 0x00949730 0x0094986C ?RulesStorage@@3URulesStorageWindow@@A 0x00533226
// RECOVERY_OBJECT: v3 0x00949738 ?Rules@@3URulesBasic@@A member ?RulesStorage@@3URulesStorageWindow@@A rules_
// RECOVERY_OBJECT: v3 0x00949730 ?TechValidCount@@3HA member ?RulesStorage@@3URulesStorageWindow@@A tech_valid_count_
// RECOVERY_OBJECT: v3 0x00949734 ?TechCommerceCount@@3HA member ?RulesStorage@@3URulesStorageWindow@@A tech_commerce_count_
RulesStorageWindow RulesStorage;
// The parser walks the five ratio words at +0x64..+0x74. NetDaemon::synch
// opcode 0x82 transfers 0x7C bytes from this start (0x00533230/0x00533A63).
// That transfer size bounds the object; ReplayEvents starts at its end.
// RECOVERY_OBJECT: v2 0x009502A8 0x00950324 ?WorldBuilder@@3URulesWorldbuilder@@A 0x00533230
RulesWorldbuilder WorldBuilder;

// THE FACTION/MAP RULES STORAGE (faction.h / map.h) - real zero-initialised
// objects; the image's copies are uninitialised .data the rules walkers fill.
// Original say_society at 0x00590A21 reads the category-row start at stride 0xD4; no full layout/extent claim.
// RECOVERY_OBJECT: v1 0x0094B000 ?SocialCategories@@3PAURulesSocialCategory@@A
RulesSocialCategory SocialCategories[MaxSocialCatNum];      // 0x0094B000
// RECOVERY_OBJECT: v1 0x00946580 ?SocialEffects@@3PAURulesSocialEffect@@A
RulesSocialEffect SocialEffects[MaxSocialEffectNum];        // 0x00946580
LPSTR Mood[MaxMoodNum];                                     // 0x0094C9E4
LPSTR Repute[MaxReputeNum];                                 // 0x00946A30
RulesMight Might[MaxMightNum];                              // 0x0094C558
RulesBonusName BonusName[MaxBonusNameNum];                  // 0x009461A8
// read_faction(int) (0x00586050) computes its Player* argument as
// `lea ecx, [eax*4 + 0x946A50]` (stride 1436); faction.h repeats the base
// beside its extern declaration.
// Bound: 8 x 0x59C records; loops end at `cmp esi, 0x949730` (0x00424A96), RulesStorage's start.
// RECOVERY_OBJECT: v2 0x00946A50 0x00949730 ?Players@@3PAUPlayer@@A 0x00424A96
Player Players[MaxPlayerNum];                               // 0x00946A50
// Eight 0x20CC-byte records end where Bases begins at 0x0097D040.
// RECOVERY_OBJECT: v2 0x0096C9E0 0x0097D040 ?PlayersData@@3PAUPlayerData@@A 0x00458DC0
PlayerData PlayersData[MaxPlayerNum];                       // 0x0096C9E0
// The original loader at 0x5881AB starts at 0x94ADE4 and fills name/name_short
// at offsets -4/0 with stride8. Terrain consumers read those same pointer slots.
// Lifetime remains open.
// Bound: 16 x 8-byte records; loop ends at `cmp esi, 0x94ae60` (0x004E155B), Weapon's start.
// RECOVERY_OBJECT: v2 0x0094ADE0 0x0094AE60 ?Natural@@3PAURulesNatural@@A 0x004E155B
RulesNatural Natural[MaxNaturalNum];                        // 0x0094ADE0
// MapLandmark at 0x00949894 is MapStorage.metadata_.landmarks_ (map.cpp).
// Bound: 128 x 0x1C records; reset loop walks +4 to `cmp eax, 0x9ab534` (0x005C55D4).
// RECOVERY_OBJECT: v2 0x009AA730 0x009AB530 ?Continents@@3PAUContinent@@A 0x005C55D4
Continent Continents[MaxContinentNum];                      // 0x009AA730
// rankings (0x005AC690) writes the eight native dwords at
// [0x009A64EC,0x009A650C). setup_player's apparent endgame selector
// cells 0x009A6504/0x009A6508 are entries 6/7 of this same allocation;
// InfoWin::setup_text also reads entry 7, not an independent scalar.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A64EC
uint16_t FactionRankingHistory[MaxPlayerNum * MaxRankingHistoryTurns];  // 0x009A68AC
// hof_save/hof_load transfer this complete zero-initialized 0x1324-byte
// block, and FameWin indexes it in 0x8C-byte records. The original address
// is evidence of identity, not a pointer used by the product build.
// RECOVERY_OBJECT: v2 0x009A3840 0x009A4B64 ?HallOfFameRecords@@3PAUHallOfFameRecord@@A 0x0044AF70
HallOfFameRecord HallOfFameRecords[HallOfFameRecordCount];  // 0x009A3840
// RECOVERY_OBJECT: v2 0x00759194 0x00759198 ?HallOfFameIOMode@@3HA 0x0044AF90
int HallOfFameIOMode;  // 0x00759194
// MapFilePath at 0x0094A2BC is MapStorage.metadata_.file_path_ (map.cpp).
// read_tech walks the array from `mov esi, 0x94F360` (&Technology[0].id,
// base+8 over flags/name) with the 0x2C stride declared in technology.h.
// Bound: 89 x 0x2C records; loops end at `cmp esi, 0x9502a4` (0x0059F7C0).
// RECOVERY_OBJECT: v2 0x0094F358 0x009502A4 ?Technology@@3PAURulesTechnology@@A 0x0059F7C0
RulesTechnology Technology[MaxTechnologyNum];       // 0x0094F358
// RECOVERY_OBJECT: v2 0x0094B4A0 0x0094B4C0 ?Mandate@@3PAURulesMandate@@A 0x005873C0
RulesMandate Mandate[MaxMandateNum];                // 0x0094B4A0
// RECOVERY_OBJECT: v2 0x00691878 0x00691AF8 ?Terraforming@@3PAURulesTerraforming@@A 0x005873C0
// The historical map word identities are live rates in this mutable array.
// 0x00691890 + row * 32 selects rate (+0x18), including road and magtube.
// RECOVERY_OBJECT: v3 0x00691930 ?MapTerrainBonusYieldRow5FirstWord@@3HB member ?Terraforming@@3PAURulesTerraforming@@A [5].rate
// RECOVERY_OBJECT: v3 0x00691950 ?MapTerrainBonusYieldRow6FirstWord@@3HB member ?Terraforming@@3PAURulesTerraforming@@A [6].rate
RulesTerraforming Terraforming[MaxTerrainNum] = {
    { 0, 0, {0, 0}, BIT_FARM,          BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_SOIL_ENRICHER, BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_MINE,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_SOLAR_TIDAL,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_FOREST,
      BIT_MINE | BIT_FUNGUS | BIT_SOLAR_TIDAL | BIT_FARM | BIT_SOIL_ENRICHER
          | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE, 0, 0 },
    { 0, 0, {0, 0}, BIT_ROAD, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_MAGTUBE, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_BUNKER, BIT_AIRBASE, 0, 0 },
    { 0, 0, {0, 0}, BIT_AIRBASE, BIT_BUNKER, 0, 0 },
    { 0, 0, {0, 0}, BIT_SENSOR_ARRAY,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE, 0, 0 },
    { 0, 0, {0, 0}, 0, BIT_FUNGUS, 0, 0 },
    { 0, 0, {0, 0}, BIT_FUNGUS,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FARM | BIT_SOIL_ENRICHER | BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_CONDENSER,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_ECH_MIRROR,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_THERMAL_BORE,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FARM | BIT_FOREST | BIT_CONDENSER
          | BIT_ECH_MIRROR | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_MONOLITH,
      BIT_SUPPLY_POD | BIT_NUTRIENT_RSC | BIT_RSC_BONUS, 0, 0 },
}; // 0x00691878
// Secret projects are the final 64 records, beginning at Facility[70]
// (original 0x009A5888). The parser and consumers share this real array;
// there is no separately allocated secret-project table.
// Bound: 134 x 0x30 records; loops end at `cmp edi, 0x9a6488` (0x0048553E).
// RECOVERY_OBJECT: v2 0x009A4B68 0x009A6488 ?Facility@@3PAURulesFacility@@A 0x0048553E
RulesFacility Facility[MaxFacilityNum];             // 0x009A4B68
// RECOVERY_OBJECT: v2 0x00946020 0x00946138 ?Citizen@@3PAURulesCitizen@@A 0x005873C0
RulesCitizen Citizen[MaxCitizenNum];                // 0x00946020
// The pinned add-base path rejects when BaseCurrentCount reaches 512
// (0x004E4B80); Base is 0x134 bytes, so the bounded array ends at 0x009A3840.
// RECOVERY_OBJECT: v2 0x0097D040 0x009A3840 ?Bases@@3PAUBase@@A 0x004E4B80
Base Bases[MaxBaseNum];                             // 0x0097D040
// ORIGINAL: 0x00584D00 _$E56 0x00584D00-0x00584D16 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E12
// ORIGINAL: 0x00584D20 _$E54 0x00584D20-0x00584D2A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E10
// RECOVERY_OBJECT: v2 0x00945780 0x009457EC ?g_TOP_MENU_WAVE@@3VWave@@A 0x0058E360
Wave g_TOP_MENU_WAVE;  // 0x00945780
/*
Purpose: Convert the tech name string to a numeric tech id.
// ORIGINAL: 0x00584D60 ?tech_name@@YAHPAD@Z 0x00584D60-0x00584E3B BYTE_EXACT
// LEVER: same defect as chas_name/weap_name/arm_name (0x00584E40 etc.) - `purge_trailing` is not in this function's own call list either; the image hand-inlines the naive recomputed-`strlen` trim loop. 8/88 -> 27/88.
// size      219 bytes
// prototype int (__cdecl ?tech_name@@YAHPAD@Z)(int8* techID)
// callers   6   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF310 0x00625EC0 0x006453E0 0x0064FD20
Return Value: Tech id; 'None' (-1); 'Disabled' (-2); or error (-2)
Status: Complete
*/
int __cdecl tech_name(LPSTR name) {
    // NOT `purge_trailing(name)`: same naive, recomputed-`strlen` trim loop
    // as chas_name/weap_name/arm_name (see chas_name's comment) - the image
    // never calls a trim helper here either (not in this function's own
    // call list).
    while (strlen(name) != 0) {
        if (name[strlen(name) - 1] != ' ') {
            break;
        }
        name[strlen(name) - 1] = 0;
    }
    if (!_stricmp(name, TechNameNoneText)) {
        return NoneValue;
    }
    if (!_stricmp(name, TechNameDisableText)) {
        return DisabledValue;
    }
    for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
        if (!_stricmp(name, Technology[tech_id].id)) {
            return tech_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop(TechNameBadKey, NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return DisabledValue;
}

/*
Purpose: Convert the chassis name string to a numeric chassis id.
// ORIGINAL: 0x00584E40 ?chas_name@@YAHPAD@Z 0x00584E40-0x00584F33 BYTE_EXACT
// LEVER: THE COMPARISON STRING GOES THROUGH StringTable.get - the image's lookup loop is
//   `_stricmp(name, StringTable.get(int(Chassis[chas_id].offsv_1_name)))`: `mov edi,<&Chassis>
//   / mov eax,[edi] / mov ecx,0x9b90d8 / push eax / call 0x6169a0 / push eax / call _stricmp`.
//   For any pointer `Strings::put` handed out `Strings::get` is an identity, so the direct
//   field compare looked equivalent - but it cost the `mov ecx,<receiver>` and the extra call,
//   and the whole tail of the function sat 43 instructions off for it. 55/98 -> 98/98, and
//   weap_name and arm_name take the same one-line fix.
// LEVER: the trim loop was `if (strlen(name) != 0) { do {...} while (strlen(name) != 0); }`,
//   which VC6 compiles as one copy of the head duplicated at the bottom (+6 instructions past the
//   image's single `jne` backedge). A plain `while (strlen(name) != 0) { if (...) break; ...; }`
//   is the image's rotated-while shape with one head. Moved 20/98 -> 55/98 agreeing.
// size      243 bytes
// prototype 
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF310 0x006169A0 0x00625EC0 0x006453E0 0x0064FD20
Return Value: Chassis id; 'None' (-1); 'Disabled' (-2); or error (0)
Status: Complete
*/
int __cdecl chas_name(LPSTR name) {
    // NOT `purge_trailing(name)`: the image inlines a NAIVE trim here that
    // recomputes `strlen(name)` on every access instead of caching it - four
    // `strlen` calls (0x00584E4A, 0x00584E59, 0x00584E6A, 0x00584E75)
    // against `purge_trailing`'s single cached-length pointer walk (compare
    // 0x006007B0's `purge_spaces`, which DOES call `purge_trailing` and
    // shows only one `strlen`). weap_name and arm_name repeat this same
    // hand-written loop verbatim.
    while (strlen(name) != 0) {
        if (name[strlen(name) - 1] != ' ') {
            break;
        }
        name[strlen(name) - 1] = 0;
    }
    if (!_stricmp(name, ChassisNameNoneText)) {
        return NoneValue;
    }
    if (!_stricmp(name, ChassisNameDisableText)) {
        return DisabledValue;
    }
    for (int chas_id = 0; chas_id < MaxChassisNum; chas_id++) {
        // The image routes the comparison string through StringTable.get - the
        // `mov ecx, 0x9b90d8` receiver - before the _stricmp. For any pointer
        // Strings::put handed out that is an identity, which is why the
        // recovery's direct compare looked equivalent and was not.
        if (!_stricmp(name, StringTable.get(Chassis[chas_id].offsv_1_name))) {
            return chas_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop(ChassisNameBadKey, NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}

/*
Purpose: Convert the weapon name string to a numeric weapon id.
// ORIGINAL: 0x00584F40 ?weap_name@@YAHPAD@Z 0x00584F40-0x00585030 BYTE_EXACT
// LEVER: same one-line fix as chas_name - the comparison string goes through
//   `StringTable.get(int(Weapon[weap_id].name_short))`, not the raw field. 55/98 -> 98/98.
// LEVER: same as chas_name (0x00584E40) - the naive recomputed-`strlen` trim loop, not `purge_trailing`. Call count 9 -> 12, still MISMATCH (register choice for the space literal).
// LEVER: same as chas_name - `if (strlen != 0) { do {...} while; }` duplicated the loop head;
//   a plain `while (strlen(name) != 0) { if (...) break; ...; }` matches the image's single
//   backedge. Moved 20/98 -> 55/98 agreeing.
// size      240 bytes
// prototype 
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF310 0x006169A0 0x00625EC0 0x006453E0 0x0064FD20
Return Value: Weapon id; 'None' (-1); 'Disabled' (-2); or error (0)
Status: Complete
*/
int __cdecl weap_name(LPSTR name) {
    // NOT `purge_trailing(name)` - see chas_name's comment; this repeats the
    // same hand-written, non-cached-length trim loop verbatim.
    while (strlen(name) != 0) {
        if (name[strlen(name) - 1] != ' ') {
            break;
        }
        name[strlen(name) - 1] = 0;
    }
    if (!_stricmp(name, WeaponNameNoneText)) {
        return NoneValue;
    }
    if (!_stricmp(name, WeaponNameDisableText)) {
        return DisabledValue;
    }
    for (int weap_id = 0; weap_id < MaxWeaponNum; weap_id++) {
        // StringTable.get on the comparison string, as chas_name's loop does.
        if (!_stricmp(name, StringTable.get(Weapon[weap_id].name_short))) {
            return weap_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop(WeaponNameBadKey, NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}

/*
Purpose: Convert the armor name string to a numeric armor id.
// ORIGINAL: 0x00585030 ?arm_name@@YAHPAD@Z 0x00585030-0x00585120 BYTE_EXACT
// LEVER: same one-line fix as chas_name - the comparison string goes through
//   `StringTable.get(int(Armor[arm_id].name_short))`, not the raw field. 55/98 -> 98/98.
// LEVER: same as chas_name (0x00584E40) - the naive recomputed-`strlen` trim loop, not `purge_trailing`. Call count 9 -> 12, still MISMATCH (register choice for the space literal).
// LEVER: same as chas_name - the `if (strlen != 0) { do {...} while; }` duplicated-head shape
//   became a plain `while (strlen(name) != 0) { if (...) break; ...; }`. PLUS a second, distinct
//   defect: the final `return DisabledValue;` (-2) after X_pop("BADARMKEY",...) should be
//   `return 0;` - the image's epilogue there is `xor eax,eax` (0x00585112), matching this
//   function's own documented "error (0)" return and chas_name/weap_name's `return 0;`. Because
//   the old -2 return matched the earlier 'Disable' early return, VC6 had merged the two exits;
//   fixing the constant also restored the image's separate epilogue. Moved 24/98 -> 55/98.
// size      240 bytes
// prototype 
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF310 0x006169A0 0x00625EC0 0x006453E0 0x0064FD20
Return Value: Armor id; 'None' (-1); 'Disabled' (-2); or error (0)
Status: Complete
*/
int __cdecl arm_name(LPSTR name) {
    // NOT `purge_trailing(name)` - see chas_name's comment; this repeats the
    // same hand-written, non-cached-length trim loop verbatim.
    while (strlen(name) != 0) {
        if (name[strlen(name) - 1] != ' ') {
            break;
        }
        name[strlen(name) - 1] = 0;
    }
    if (!_stricmp(name, ArmorNameNoneText)) {
        return NoneValue;
    }
    if (!_stricmp(name, ArmorNameDisableText)) {
        return DisabledValue;
    }
    for (int arm_id = 0; arm_id < MaxArmorNum; arm_id++) {
        // StringTable.get on the comparison string, as chas_name's loop does.
        if (!_stricmp(name, StringTable.get(Armor[arm_id].name_short))) {
            return arm_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop(ArmorNameBadKey, NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}


// body src/alpha.h
// BODY IN alpha.h, as `MEASURED inline`.

/*
Purpose: Get the label string from a label offset. Created to reduce code complexity.
Original Offset: n/a
Return Value: Pointer to label string
Status: Complete
*/
// BODY IN alpha.h, as plain `inline` (no ORIGINAL marker of its own - "Original
// Offset: n/a"): say_base (base.cpp) needs it folded to `StringTable.get(...)`
// the way the image does.

// ORIGINAL: 0x00585120 ?text_get_number@@YAHHH@Z 0x00585120-0x00585150 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned585120 calls5FD570 then5FD740 and uses an EBP frame; adjacent585170 is read_basic_rules.", "Fresh complete application COFF at421b46dbe: alpha.cpp emitted header helper matches18/18; library text.cpp emits67.", "Canonical identity, full spans, historical claim and lessons moved intact from src/text.cpp; definition remains src/text.h."], "reason": "Current application rules-parser ownership; the header definition remains shared. Library-TU emission inlines both wrappers and is not this original body. No original TU or linked selection claim."}
// body      src/jackal/text.h
// LEVER: BYTE_EXACT 18/18, from 2/18 and 71 compiled instructions against the image's 18. The body went through `text_get_number_source(&Txt, ...)`, so both parses were written out INLINE; the image CALLS the zero-arg wrappers, `call 0x005FD570` then `call 0x005FD740`, and open-codes only the range() clamp. call_diff said exactly that - 1 call against the image's 2. Rewritten in text.h as `text_get(); return range(text_item_number(), min, max);`, which is the same two-statement shape alpha.cpp's LEVER already spells out at its own 83 call sites. Nothing else calls text_get_number(), so no caller moved.
// size      48 bytes
// prototype int (__cdecl ?text_get_number@@YAHHH@Z)(int min, int max)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD570 0x005FD740
// notes     Staged hybrid export redirect calls the source-owned wrapper

/*
Purpose: Parse the current tech name inside the Txt item buffer into a tech id.
// ORIGINAL: 0x00585150 ?tech_item@@YAHXZ 0x00585150-0x00585164 BYTE_EXACT
// body      src/alpha.h
// LEVER: `osmx calls` on 0x00585170 (read_basic_rules) shows this tree
//        calling `tech_item` where the image calls `tech_name` directly at
//        all ten sites - `callers 0` above already said the image never
//        `call`s this address. Moved to alpha.h as `MEASURED inline` so
//        every call site expands in place, matching read_basic_rules.
// size      20 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00584D60 0x005FD570 0x005FD670
Return Value: Tech id
Status: Complete
*/

// BODY IN alpha.h, as `MEASURED inline`: see LEVER above.

/*
Purpose: Parse the #RULES & #WORLDBUILDER sections inside the alpha(x).txt.
// ORIGINAL: 0x00585170 ?read_basic_rules@@YAHXZ 0x00585170-0x00585E26 BYTE_EXACT
// LEVER: the ten `Rules.tech_* = tech_item();` sites now match the image's
//        call pattern (`text_get; text_item; tech_name` per site, confirmed
//        against `osmx calls`) now that `tech_item` is `MEASURED inline` in
//        alpha.h - it was a call the image never makes (see its own
//        `callers 0`). Landed as a real structural fix; this function is
//        883 image instructions across ~40 unrelated #SECTIONs.
// LEVER: every `text_get(); X = range(text_item_number(), lo, hi);` pair is
//        one `X = text_get_number(lo, hi);` call (89 sites). /Ob2 then refuses
//        the nested range at the first 47 sites (calls to 0x00422F00) and
//        expands it at the later 42, exactly as the image does; the written-out
//        pairs instead store the clamp result once per arm and leave the
//        budget to expand early sites.
// size      3254 bytes
// prototype
// callers   1   call targets   6
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00422F00 0x00584D60 0x005FD550 0x005FD570 0x005FD670 0x005FD740
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl read_basic_rules() {
    if (text_open(AlphaxFileID, "RULES")) {
        return true;
    }
    Rules.move_rate_roads = text_get_number(1, 100);
    Rules.nutrient_req_citizen = text_get_number(0, 100);
    Rules.artillery_dmg_num = text_get_number(1, 1000);
    Rules.artillery_dmg_denom = range(text_item_number(), 1, 1000);
    Rules.artillery_max_rng = text_get_number(1, 8);
    Rules.max_airdrop_sans_orb_insert = text_get_number(1, 500);
    Rules.nutrient_cost_multi = text_get_number(1, 100);
    Rules.mineral_cost_multi = text_get_number(1, 100);
    Rules.tech_discovery_rate_pct_std = text_get_number(0, 1000);
    Rules.limit_mineral_mine_sans_road = text_get_number(0, 100);
    Rules.tgl_nutrient_effect_with_mine = text_get_number(-1, 0); // Weird logic behind -1/0 value
    Rules.min_base_size_specialists = text_get_number(0, 100);
    Rules.drones_genejack_factory = text_get_number(0, 100);
    Rules.pop_limit_sans_hab_complex = text_get_number(1, 100);
    Rules.pop_limit_sans_hab_dome = text_get_number(1, 100);
    Rules.extra_pct_cost_proto_land = text_get_number(0, 500);
    Rules.extra_pct_cost_proto_sea = text_get_number(0, 500);
    Rules.extra_pct_cost_proto_air = text_get_number(0, 500);
    for (int triad = 0; triad < MaxTriadNum; triad++) {
        Rules.psi_combat_ratio_atk[triad] = text_get_number(1, 1000);
        Rules.psi_combat_ratio_def[triad] = range(text_item_number(), 1, 1000);
    }
    Rules.player_start_energy_reserve = text_get_number(0, 1000);
    Rules.combat_pct_base_def = text_get_number(-100, 1000);
    Rules.combat_pct_atk_road = text_get_number(-100, 1000);
    Rules.combat_pct_atk_higher_elev = text_get_number(-100, 1000);
    Rules.combat_pen_pct_atk_lwr_elev = text_get_number(-100, 1000);
    Rules.combat_pct_mobile_open_ground = text_get_number(-100, 1000);
    Rules.combat_pct_def_vs_mobile_rough = text_get_number(-100, 1000);
    Rules.combat_pct_infantry_vs_base = text_get_number(-100, 1000);
    Rules.combat_pen_pct_atk_airdrop = text_get_number(-100, 1000);
    Rules.combat_pct_fanatic_atk_bonus = text_get_number(-100, 1000);
    Rules.combat_pct_land_gun_vs_ship_art = text_get_number(-100, 1000);
    Rules.combat_pct_art_bonus_lvl_alt = text_get_number(-100, 1000);
    Rules.combat_pct_trance_def_vs_psi = text_get_number(-100, 1000);
    Rules.combat_pct_emp_song_atk_vs_psi = text_get_number(-100, 1000);
    Rules.combat_pen_pct_air_supr_vs_grnd = text_get_number(-100, 1000);
    Rules.combat_pct_air_supr_vs_air = text_get_number(-100, 1000);
    Rules.combat_pen_pct_non_cbt_def_vs_cbt = text_get_number(-100, 1000);
    Rules.combat_pct_com_jam_def_vs_mobl = text_get_number(-100, 1000);
    Rules.combat_pct_bonus_vs_ship_port = text_get_number(-100, 1000);
    Rules.combat_pct_aaa_bonus_vs_air = text_get_number(-100, 1000);
    Rules.combat_pct_def_range_sensor = text_get_number(-100, 1000);
    Rules.combat_pct_psi_atk_bonus_planet = text_get_number(-100, 1000);
    Rules.retool_pct_pen_prod_chg = text_get_number(0, 100);
    // Pinned 0x005856C2 passes upper bound 2; allowing 3 is a later fix.
    Rules.retool_strictness = text_get_number(0, 2);
    Rules.retool_exemption = text_get_number(0, 1000);
    Rules.min_turns_councils = text_get_number(0, 1000);
    Rules.minerals_harvesting_forest = text_get_number(0, 100);
    Rules.territory_max_dist_base = text_get_number(0, 100);
    Rules.turns_corner_gbl_energy_mrkt = text_get_number(1, 100);
    Rules.tech_improve_fungus_sqr = tech_item();
    Rules.tech_ease_fungus_movement = tech_item();
    Rules.tech_build_roads_fungus = tech_item();
    Rules.tech_two_spec_abilities = tech_item();
    Rules.tech_three_nutrients_sqr = tech_item();
    Rules.tech_three_minerals_sqr = tech_item();
    Rules.tech_three_energy_sqr = tech_item();
    Rules.tech_orb_insert_sans_spc_elev = tech_item();
    Rules.tech_mining_platform_bonus = tech_item();
    Rules.tech_economic_victory = tech_item();
    // Preserve the signed -1..1 values, not merely their boolean meaning:
    // pinned stores at 0x0058585B/7E/A1 retain -1 after signed clamping.
    Rules.tgl_probe_can_steal_tech = text_get_number(-1, 1);
    Rules.tgl_human_always_contact_net = text_get_number(-1, 1);
    Rules.tgl_humans_always_contact_pbem = text_get_number(-1, 1);
    Rules.max_pct_dmg_art_vs_unit_base_bnkr = text_get_number(10, 100);
    Rules.max_pct_dmg_art_vs_units_open = text_get_number(10, 100);
    Rules.max_pct_dmg_art_vs_units_sea = text_get_number(10, 100);
    Rules.freq_global_warming_num = text_get_number(0, 1000);
    Rules.freq_global_warming_denom = range(text_item_number(), 1, 1000);
    Rules.normal_starting_year = text_get_number(0, 999999);
    Rules.normal_end_year_low_three_diff = text_get_number(0, 999999);
    Rules.normal_end_year_high_three_diff = text_get_number(0, 999999);
    // Pinned 0x005859D9..ED uses the same signed -1..1 clamp.
    Rules.tgl_oblit_base_atrocity = text_get_number(-1, 1);
    Rules.size_base_subspace_gen = text_get_number(1, 999); // SMACX only
    Rules.subspace_generators_needed = text_get_number(1, 999); // SMACX only
    if (text_open(AlphaxFileID, "WORLDBUILDER")) {
        return 1;
    }
    WorldBuilder.land_base = text_get_number(50, 4000);
    WorldBuilder.land_mod = text_get_number(0, 2000);
    WorldBuilder.continent_base = text_get_number(5, 1000);
    WorldBuilder.continent_mod = text_get_number(5, 1000);
    WorldBuilder.hills_base = text_get_number(0, 100);
    WorldBuilder.hills_mod = text_get_number(0, 100);
    WorldBuilder.plateau_base = text_get_number(0, 500);
    WorldBuilder.plateau_mod = text_get_number(0, 500);
    WorldBuilder.rivers_base = text_get_number(0, 100);
    WorldBuilder.rivers_rain_mod = text_get_number(0, 100);
    WorldBuilder.solar_energy = text_get_number(1, 64);
    WorldBuilder.thermal_band = text_get_number(1, 64);
    WorldBuilder.thermal_deviance = text_get_number(1, 64);
    WorldBuilder.global_warming = text_get_number(1, 64);
    WorldBuilder.sea_level_rises = text_get_number(1, 100);
    WorldBuilder.cloudmass_peaks = text_get_number(0, 20);
    WorldBuilder.cloudmass_hills = text_get_number(0, 20);
    WorldBuilder.rainfall_coeff = text_get_number(0, 8);
    WorldBuilder.deep_water = text_get_number(-100, 100);
    WorldBuilder.shelf = text_get_number(-100, 100);
    WorldBuilder.plains = text_get_number(-100, 100);
    WorldBuilder.beach = text_get_number(-100, 100);
    WorldBuilder.hills = text_get_number(0, 100);
    WorldBuilder.peaks = text_get_number(-100, 100);
    WorldBuilder.fungus = text_get_number(0, 5);
    text_get();
    for (int i = 0; i < 5; i++) {
        WorldBuilder.cont_size_ratio[i] = text_item_number();
    }
    WorldBuilder.islands = text_get_number(1, 500);
    return false;
}

// RECOVERY_LITERAL: v1 0x0068F2FC 0x0068F30A $SG52315 0x00585E30
/*
Purpose: Parse the #TECHNOLOGY section inside the alpha(x).txt with a duplicate entry check.
// ORIGINAL: 0x00585E30 ?read_tech@@YAHXZ 0x00585E30-0x00585FDB BYTE_EXACT
// LEVER: the `id` copy is `strncpy(dest, TextBufferItemPtr, 8)` at 0x00585E78 (3 args, no `strlen` call in the image's own call list) - the tree had `strncpy_s(Technology[i].id, 8, TextBufferItemPtr, strlen(TextBufferItemPtr))`, an extra call and a wrong count. Fixed to a literal 8; 13/139 -> 20/139 (best across every flag set).
// LEVER: the TechValidCount guard was `preq_tech_1 != DisabledValue && preq_tech_2 != DisabledValue`
//   (an equality test against -2, which materialised -2 in a register and forced extra push/pop
//   shuffles); the image is a SIGNED `>= -1` test (`or ecx,0xffffffff; cmp; jl`). Rewriting as
//   `preq_tech_1 >= NoneValue && preq_tech_2 >= NoneValue` moved 20/139 -> 139/139, BYTE_EXACT.
// size      427 bytes
// prototype 
// callers   1   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00584D60 0x005BF310 0x005FD550 0x005FD570 0x005FD670 0x005FD6D0 0x005FD740 0x005FD7A0 0x00625E30 0x00625EC0 0x00645660 0x00645E90
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl read_tech() {
    if (text_open(AlphaxFileID, alpha_tech_identifiers_section)) {
        return true;
    }
    for (int i = 0; i < MaxTechnologyNum; i++) {
        text_get();
        text_item();
        text_item();
        strncpy_s(Technology[i].id, 8, TextBufferItemPtr, 8);
        Technology[i].id[7] = 0;
        for (int j = 0; j < i; j++) {
            if (!strcmp(Technology[i].id, Technology[j].id)) {
                parse_num(0, i);
                parse_num(1, j);
                parse_says(0, Technology[i].id, -1, -1);
                parse_says(1, FilefindPath.last_path, -1, -1);
                parse_says(2, TextBufferGetPtr, -1, -1);
                X_pop("DUPLICATETECH", NULL);
            }
        }
    }
    if (text_open(AlphaxFileID, alpha_tech_values_section)) {
        return true;
    }
    TechValidCount = 0;
    TechCommerceCount = 0;
    for (i = 0; i < MaxTechnologyNum; i++) {
        text_get();
        Technology[i].name = text_item_string();
        text_item();
        Technology[i].power_value = text_item_number();
        Technology[i].tech_value = text_item_number();
        Technology[i].wealth_value = text_item_number();
        Technology[i].growth_value = text_item_number();
        Technology[i].preq_tech_1 = tech_name(text_item());
        Technology[i].preq_tech_2 = tech_name(text_item());
        Technology[i].flags = text_item_binary();
        if (Technology[i].preq_tech_1 >= NoneValue
            && Technology[i].preq_tech_2 >= NoneValue) {
            TechValidCount += 1;
            if (Technology[i].flags & TFLAG_INC_COMMERCE) {
                TechCommerceCount += 1;
            }
        }
    }
    return false;
}

/*
Purpose: Clear the rule values for the specified player.
// ORIGINAL: 0x00585FE0 ?clear_faction@@YAXPAUPlayer@@@Z 0x00585FE0-0x00586049 BYTE_EXACT
// body      src/alpha.h
// LEVER: `MEASURED inline` in alpha.h - `callers 0` above already says the image never `call`s
//   this address; read_faction (0x00586090) writes all 15 stores out inline instead
//   ('mov dword ptr [esi+0x4d4],eax' etc., no call to 0x585fe0 anywhere in its own call list).
//   Moving the body out of this .cpp reproduces that at its one caller.
// size      105 bytes
// prototype void (__cdecl ?clear_faction@@YAXPAUPlayer@@@Z)(Player* player)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// BODY IN alpha.h, as `MEASURED inline`: see the LEVER note above.

/*
Purpose: Parse the faction's file and art for the specified player id.
// ORIGINAL: 0x00586050 ?read_faction@@YAXH@Z 0x00586050-0x00586081 BYTE_EXACT
// size      49 bytes
// prototype 
// callers   3   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00453710 0x00586090
Return Value: n/a
Status: Complete
*/
void __cdecl read_faction(int player_id) {
    if (player_id) {
        read_faction(&Players[player_id], 0);
        load_faction_art(player_id);
    }
}

// Bounded copy of one faction-file field, always terminated.
static inline void faction_copy_text(char *destination, const char *source, int size) {
    strncpy(destination, source, size);
    destination[size - 1] = 0;
}

/*
Purpose: Parse the 1st eight lines of the specified faction's file into a player structure. The 
         toggle parameter will end the function early if set to 2 (original code never uses this).
// ORIGINAL: 0x00586090 ?read_faction@@YAXPAUPlayer@@H@Z 0x00586090-0x00586F2B BYTE_EXACT
// LEVER: clear_faction moved to `MEASURED inline` in alpha.h (see its own marker) - the image
//   writes its 15 stores out inline here instead of calling it. Moved 44/985 -> 101/985.
// LEVER: the faction-bonus rule parser's `LPSTR parse_rule = new char[len + 1]; strcpy_s(...);
//   ...; delete [] parse_rule;` was a heap allocation per rule token; the image copies into a
//   fixed 256-byte LOCAL buffer instead (`sub esp, 0x104`), the same idiom as read_rules'
//   `order_buf`. Rewritten as `char parse_rule[256]; strcpy(parse_rule, parse_rule_check);` with
//   no delete. call_diff's extra `operator new`/`operator delete` edges are gone. Moved
//   101/985 -> 196/985 agreeing. Still deep MISMATCH (short 5 calls: tech_name/X_pop/text_open/
//   text_get); not chased further this pass.
// size      3739 bytes
// prototype void (__cdecl ?read_faction@@YAXPAUPlayer@@H@Z)(Player* player, int type)
// callers   4   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00584D60 0x005BF310 0x005FD550 0x005FD570 0x005FD670 0x005FD740 0x006169A0 0x00625EC0 0x006453E0 0x00645460 0x00645470 0x00645DA7 0x00645E90 0x0064FD20 0x0064FDF0
// TRIED: plateau 196/985 after the clear_faction inline and local-buffer rewrites;
//        still short 5 calls (tech_name/X_pop/text_open/text_get), deep MISMATCH
Return Value: n/a
Status: Complete
*/
void __cdecl read_faction(Player *player, int toggle) {
    clear_faction(player);
    if (text_open(player->filename, player->search_key)
        && text_open(player->filename, player->filename)) {
        parse_says(0, player->search_key, -1, -1);
        parse_says(1, player->filename, -1, -1);
        X_pop("PLAYERFILE", NULL);
        return;
    }
    text_get();
    faction_copy_text(player->formal_name_faction, text_item(), 40);
    faction_copy_text(player->desc_name_faction, text_item(), 24);
    faction_copy_text(player->noun_faction, text_item(), 24);
    text_item();
    player->noun_gender = GENDER_MALE;
    if (*TextBufferItemPtr == 'F' || *TextBufferItemPtr == 'f') {
        player->noun_gender = GENDER_FEMALE;
    }
    if (*TextBufferItemPtr == 'N' || *TextBufferItemPtr == 'n') {
        player->noun_gender = GENDER_NEUTRAL;
    }
    player->is_noun_plural = range(text_item_number() - 1, 0, 1);
    faction_copy_text(player->name_leader, text_item(), 24);
    text_item();
    player->is_leader_female = (*TextBufferItemPtr == 'F' || *TextBufferItemPtr == 'f') ? 1 : 0;
    if (toggle == 2) {
        return;
    }
    player->ai_fight = text_item_number();
    player->ai_power = text_item_number();
    player->ai_tech = text_item_number();
    player->ai_wealth = text_item_number();
    player->ai_growth = text_item_number();
    text_get();
    while (strlen(text_item())) {
        char parse_rule[256];
        strcpy(parse_rule, TextBufferItemPtr);
        LPSTR parse_param = text_item();
        int j;
        if (!_stricmp(parse_rule, BonusName[0].key)) { // TECH
            if (atoi(parse_param)) {
                player->rule_tech_selected = atoi(parse_param);
            } else if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_TECH;
                player->faction_bonus_val1[player->faction_bonus_count] = tech_name(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[1].key)) { // MORALE
            if (*parse_param == '+') {
                parse_param++;
            }
            player->rule_morale = atoi(parse_param);
            if (!player->rule_morale) {
                player->rule_flags |= RFLAG_MORALE;
            }
        } else if (!_stricmp(parse_rule, BonusName[3].key)) { // FACILITY
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FACILITY;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[4].key)) { // RESEARCH
            player->rule_research = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[5].key)) { // DRONE
            player->rule_drone = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[6].key)) { // TALENT
            player->rule_talent = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[7].key)) { // ENERGY
            player->rule_energy = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[8].key)) { // INTEREST
            player->rule_flags |= RFLAG_INTEREST;
            player->rule_interest = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[9].key)) { // COMMERCE
            player->rule_commerce = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[10].key)) { // POPULATION
            player->rule_population = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[11].key)) { // HURRY
            player->rule_hurry = range(atoi(parse_param), 1, 1000);
        } else if (!_stricmp(parse_rule, BonusName[13].key)) { // TECHCOST
            player->rule_techcost = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[12].key)) { // UNIT
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_UNIT;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[2].key)) { // PSI
            player->rule_psi = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[14].key)) { // SHARETECH
            player->rule_sharetech = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[15].key)) { // TERRAFORM
            player->rule_flags |= RFLAG_TERRAFORM;
        } else if (!_stricmp(parse_rule, BonusName[16].key) // SOCIAL, ROBUST, IMMUNITY
            || !_stricmp(parse_rule, BonusName[17].key)
            || !_stricmp(parse_rule, BonusName[18].key)) {
            int value = 0;
            while (*parse_param == '+' || *parse_param == '-') {
                if (*parse_param == '+') {
                    value++;
                } else {
                    value--;
                }
                parse_param++;
            }
            if (!value) {
                value = 1;
            }
            for (j = 0; j < MaxSocialEffectNum; j++) {
                if (!_stricmp(parse_param, SocialEffects[j].set1)) {
                    if (player->faction_bonus_count < 8) {
                        if (!_stricmp(parse_rule, BonusName[17].key)) {
                            player->faction_bonus_id[player->faction_bonus_count] = RULE_ROBUST;
                        } else {
                            player->faction_bonus_id[player->faction_bonus_count] =
                                _stricmp(parse_rule, BonusName[16].key) ? RULE_IMMUNITY : RULE_SOCIAL;
                        }
                        player->faction_bonus_val1[player->faction_bonus_count] = j;
                        player->faction_bonus_val2[player->faction_bonus_count] = value;
                        player->faction_bonus_count++;
                    }
                    break;
                }
            }
        } else if (!_stricmp(parse_rule, BonusName[19].key) // IMPUNITY, PENALTY
            || !_stricmp(parse_rule, BonusName[20].key)) {
            for (j = 0; j < MaxSocialCatNum; j++) {
                for (int k = 0; k < MaxSocialModelNum; k++) {
                    if (!_stricmp(parse_param, StringTable.get(SocialCategories[j].name[k]))
                        && player->faction_bonus_count < 8) {
                        player->faction_bonus_id[player->faction_bonus_count] =
                            _stricmp(parse_rule, BonusName[19].key) ? RULE_PENALTY : RULE_IMPUNITY;
                        player->faction_bonus_val1[player->faction_bonus_count] = j;
                        player->faction_bonus_val2[player->faction_bonus_count] = k;
                        player->faction_bonus_count++;
                    }
                }
            }
        } else if (!_stricmp(parse_rule, BonusName[21].key)) { // FUNGNUTRIENT
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGNUTRIENT;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[22].key)) { // FUNGMINERALS
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGMINERALS;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[23].key)) { // FUNGENERGY
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGENERGY;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[24].key)) { // COMMFREQ
            player->rule_flags |= RFLAG_COMMFREQ;
        } else if (!_stricmp(parse_rule, BonusName[25].key)) { // MINDCONTROL
            player->rule_flags |= RFLAG_MINDCONTROL;
        } else if (!_stricmp(parse_rule, BonusName[26].key)) { // FANATIC
            player->rule_flags |= RFLAG_FANATIC;
        } else if (!_stricmp(parse_rule, BonusName[27].key)) { // VOTES
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_VOTES;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[28].key)) { // FREEPROTO
            player->rule_flags |= RFLAG_FREEPROTO;
        } else if (!_stricmp(parse_rule, BonusName[29].key)) { // AQUATIC
            player->rule_flags |= RFLAG_AQUATIC;
        } else if (!_stricmp(parse_rule, BonusName[30].key)) { // ALIEN
            player->rule_flags |= RFLAG_ALIEN;
        } else if (!_stricmp(parse_rule, BonusName[31].key)) { // FREEFAC
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FREEFAC;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[32].key)) { // REVOLT
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_REVOLT;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[33].key)) { // NODRONE
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_NODRONE;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[34].key)) { // WORMPOLICE
            player->rule_flags |= RFLAG_WORMPOLICE;
        } else if (!_stricmp(parse_rule, BonusName[35].key)) { // FREEABIL
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_FREEABIL;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[36].key)) { // PROBECOST
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_PROBECOST;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[37].key)) { // DEFENSE
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_DEFENSE;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[38].key)) { // OFFENSE
            if (player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_OFFENSE;
                player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
                player->faction_bonus_count++;
            }
        } else if (!_stricmp(parse_rule, BonusName[39].key)) { // TECHSHARE
            player->rule_flags |= RFLAG_TECHSHARE;
            player->rule_sharetech = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[40].key)) { // TECHSTEAL
            player->rule_flags |= RFLAG_TECHSTEAL;
        }
    }
    // Societal Ideology + Anti-Ideology. The image walks one pointer over the
    // category/model/effect pairs (+0, +8, +0x10), stepping 4 per pass.
    for (int i = 0; i < 2; i++) {
        // WORKAROUND(type): one pointer per side walks category, model and
        // effect (+0, +8, +0x10); indexing the three arrays separately picks
        // another base register. The original was perhaps one [3][2] array.
        int *ideology = &player->soc_ideology_category[i];
        ideology[0] = -1;
        ideology[2] = 0;
        ideology[4] = -1;
        text_get();
        StringTemp[0] = 0;
        strcat(StringTemp, text_item());
        for (int j = 0; j < MaxSocialCatNum; j++) {
            if (!Language) {
                if (!_stricmp(StringTemp, StringTable.get(SocialCategories[j].type))) {
                    goto found_category;
                }
            } else if (!_strnicmp(StringTemp, StringTable.get(SocialCategories[j].type), 4)) {
                goto found_category;
            }
            continue;
        found_category:
            ideology[0] = j;
            break;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, text_item());
        int category = ideology[0];
        if (category >= 0) {
            for (int k = 0; k < MaxSocialModelNum; k++) {
                if (!Language) {
                    if (!_stricmp(StringTemp, StringTable.get(SocialCategories[category].name[k]))) {
                        ideology[2] = k;
                    }
                } else if (!_strnicmp(StringTemp, StringTable.get(SocialCategories[category].name[k]), 4)) {
                    ideology[2] = k;
                }
            }
        }
        StringTemp[0] = 0;
        strcat(StringTemp, text_item());
        for (int e = 0; e < MaxSocialEffectNum; e++) {
            if (!_stricmp(StringTemp, SocialEffects[e].set1)) {
                ideology[4] = -1;
                break;
            }
        }
    }
    text_get();
    faction_copy_text(player->adj_name_faction, text_item(), 128);
    text_get();
    faction_copy_text(player->assistant_name, text_item(), 24);
    faction_copy_text(player->scientist_name, text_item(), 24);
    faction_copy_text(player->assistant_city, text_item(), 24);
    text_get();
    faction_copy_text(player->title_leader, text_item(), 24);
    faction_copy_text(player->adj_leader, text_item(), 128);
    faction_copy_text(player->adj_insult_leader, text_item(), 128);
    faction_copy_text(player->adj_faction, text_item(), 128);
    faction_copy_text(player->adj_insult_faction, text_item(), 128);
    text_get();
    faction_copy_text(player->insult_leader, text_item(), 24);
}

/*
Purpose: Parse the #BONUSNAMES, #FACTIONS, and #NEWFACTIONS sections inside the alpha(x).txt.
// ORIGINAL: 0x00586F30 ?read_factions@@YAHXZ 0x00586F30-0x005871C9 BYTE_EXACT
// TRIED: fixes, call count 23 -> 28-29 (image 31): 1. `rand() % faction_count`, not `random(0, faction_count)` - the image calls the C library `rand` (0x0064601D) directly, never the game's seeded PRNG. BUG IN THE ORIGINAL, preserved. 2. `load_faction_art` was a temp.h function POINTER, compiling an indirect call where the image has `call rel32` twice; promoted to a src/pending_bodies.cpp forwarder (see there and temp.h). 3. `strcpy_s(dest, src)`, not `strncpy_s(dest, src, 24)`, for BonusName[i].key and both Players[player] fields in the first two loops - the image pushes only 2 args (`call 0x00645460`), an unbounded copy, not a 3-arg bounded one. BUG IN THE ORIGINAL, preserved. Remaining gap: `text_open`/`text_get` each short by 1 (a ternary argument the image tail-merges two of its three arms for, not chased), and `load_faction_art` merges its two call sites into one physical `call` at the object level where the image keeps two.
// LEVER: signedness at the JENN282 reroll - `uint32_t faction_count = 14;` and `uint32_t
//        faction_set = rand_val / 7;` made `rand() % faction_count` compile as an unsigned
//        div and the /7 as an unsigned magic-multiply; the image uses `cdq; idiv` for the
//        modulo and the signed magic-multiply sequence for /7. Both changed to plain `int`.
//        Moved 44/214 -> 46/214 agreeing.
// LEVER: the #BONUSNAMES loop is a PEEL in the image - ONE text_get hoisted ahead of it
//        (0x00586F50) and then a separate counter that resets at 8 inside the body
//        (`xor edi,edi` init, `inc edi; cmp edi,8; jl` skip, `xor edi,edi; call text_get`
//        reset, 0x00586F73 is the second call site), with the array walk on its own pointer
//        so the loop index is strength-reduced away. TRIED: spelling that out - `text_get();
//        int line_count = 0; for (int i = 0; i < MaxBonusNameNum; i++) { strcpy_s(...); if
//        (++line_count >= 8) { line_count = 0; text_get(); } }` - REGRESSED the body to
//        14/214; the modulo form stays, see the /Oi- note at the bottom for the frame.
// LEVER: THE FACTION_SET TEST IS A SWITCH, not a nested ternary. The image lowers it as
//        `sub edx,0; je` / `dec edx; je` / `dec edx; jne` (0x00587084-0x0058708D) with
//        FACTIONS+NEWFACTIONS tail-merged into the one text_open at 0x0058709B and
//        CUSTOMFACTIONS on its own at 0x005870C6 - i.e. `switch (faction_set) { case 0: ...
//        case 1: ... case 2: ... }`. The ternary this body used opened CUSTOMFACTIONS for
//        EVERY faction_set >= 2, so it also read a section the image never opens at
//        faction_set >= 3: BUG IN THE ORIGINAL, reproduced. 48/214 -> 75/214 on its own, the
//        single biggest lever in this body.
// LEVER: the JENN282 skip loop is the brief's peeled-loop shape: one UNCONDITIONAL text_get
//        (0x005870E2) then a guard-and-do-while remainder (`test edi,edi; jle` /
//        `mov esi,edi` / `loop: call; dec esi; jne`, 0x5870ED). Written out literally, which
//        is also what the image's own `test edi,edi; jle` implies: for faction_id <= 0 the
//        image still consumes one text_get where `for (j = faction_id; j >= 0; j--)`
//        consumes none - unreachable in practice because faction_id is `rand_val % 7` at
//        that point (the `faction_id = -1` assignment is always overwritten by the reroll
//        before the loop is re-entered). Measured back to back with the switch in place:
//        peel 75/214, plain for 71/214.
// TRIED (still open): the extra stack slot. Under the similarity-picked /Oi- set this body
//        compiles to `sub esp,0xc` against the image's `sub esp,8` - `faction_id` lands in
//        esi and is spilled to [ebp-0xc] where the image keeps it in edi across the whole
//        iteration - and every [ebp-N] after that diverges. Present with and without the
//        peel, so it is not either rewrite; under /O2 the frame is 8 but the body balloons
//        to 252 instructions against the image's 214 and scores 15/214, so /Oi- is the
//        better set despite the slot. Not chased further.
// size      665 bytes
// prototype 
// callers   1   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00453710 0x00586090 0x0059DBD0 0x005FD550 0x005FD570 0x005FD670 0x00645460 0x00645660 0x0064601D
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl read_factions() {
    // Both open failures share one `return true` block in the image.
    if (text_open(AlphaxFileID, "BONUSNAMES")) {
        goto failed;
    }
    // NOT `if (!(i % 8)) text_get();` at the top of the body: the image
    // hoists ONE text_get ahead of the loop and walks a separate counter
    // that resets at 8 (`xor edi,edi` ... `inc edi; cmp edi,8; jl`; the
    // array walk is a separate pointer, so the loop index is
    // strength-reduced away). 0x00586F50 / 0x00586F73 are the two call
    // sites that gives; the counter form below now matches that loop.
    text_get();
    {
        int per_line = 0;
        for (int i = 0; i < MaxBonusNameNum; i++) {
            // BUG IN THE ORIGINAL (preserved): `call 0x00645460` at 0x00586F63
            // pushes only the two arguments - plain unbounded `strcpy`, not a
            // 24-byte-bounded copy. Same for both `Players[player]` fields just
            // below (0x00586FD2, 0x00586FDE) and both copies in the JENN282 arm
            // (0x005870FC, 0x00587106). Spelled as DIRECT `strcpy` calls, not
            // `strcpy_s`: both shims live in vc6_compat.h, which /FI includes
            // BEFORE stdafx.h's `#pragma function(strcpy, ...)` takes effect, so
            // any `strcpy_s` inline-expands the intrinsic `repne scasb` walk
            // where the image makes a real call (same lever as prefs_put's
            // strcat, 0x0059E530).
            strcpy(BonusName[i].key, text_item());
            if (++per_line >= 8) { // 8 entries per line
                per_line = 0;
                text_get();
            }
        }
    }
    if (ExpansionEnabled ? text_open(AlphaxFileID, "NEWFACTIONS")
                         : text_open(AlphaxFileID, "FACTIONS")) {
failed:
        return true;
    }
    int player;
    for (player = 1; player < MaxPlayerNum; player++) {
        text_get();
        strcpy(Players[player].filename, text_item());
        strcpy(Players[player].search_key, text_item());
    }
    // SMACX only: Will override any values parsed from alphax.txt #NEWFACTIONS if set in ini;
    // The image GUARDS this call at the CALL SITE - `mov eax,[0x9a6488];
    // test eax,eax; je 0x00587002` at 0x00586FF4 - so the ExpansionEnabled
    // check that used to wrap it is restored: it is physically in the
    // original, outside prefs_fac_load's own guard.
    if (ExpansionEnabled) {
        prefs_fac_load();
    }
    int faction_count = 14;
    if (!text_open(AlphaxFileID, "CUSTOMFACTIONS")) { // get count of custom factions
        text_get();
        for (LPSTR custom = text_item(); *custom; custom = text_item()) {
            faction_count++;
            text_get();
        }
    }
    for (player = 1; player < MaxPlayerNum; player++) {
        if (!strcmp(Players[player].filename, "JENN282")) {
            int faction_id;
            do {
                // BUG IN THE ORIGINAL (preserved): this calls the C library
                // `rand()` directly - `call 0x0064601D` at 0x00587066 - and
                // takes the remainder mod `faction_count`, NOT the game's
                // seeded `random()` PRNG (0x00625810, which never appears in
                // this function's call list). That means custom-faction
                // selection here does not respect `random_reseed`.
                int rand_val = rand() % faction_count;
                int faction_set = rand_val / 7; // 0: SMAC; 1: SMACX; 2+: custom
                // NOT a nested ternary argument: the image lowers this as a
                // SWITCH - `sub edx,0; je` / `dec edx; je` / `dec edx; jne`,
                // with CUSTOMFACTIONS+NEWFACTIONS sharing the call at
                // 0x0058709B and FACTIONS on its own at 0x005870C6. So
                // `faction_set` past 2 opens NOTHING here, where the ternary
                // reading opened CUSTOMFACTIONS for every custom set: BUG IN
                // THE ORIGINAL, reproduced.
                switch (faction_set) {
              case 0:
                    if (text_open(AlphaxFileID, "FACTIONS")) {
                        return true;
                    }
                    break;
              case 1:
                    if (text_open(AlphaxFileID, "NEWFACTIONS")) {
                        return true;
                    }
                    break;
              case 2:
                    if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) {
                        return true;
                    }
                    break;
              default:
                    break;
                }
                faction_id = rand_val % 7;
                // The image PEELS this loop: one unconditional text_get
                // (0x005870E2), then the remainder as a counted do-while
                // (`test edi,edi; jle` guard, `mov esi,edi`, `loop: call;
                // dec esi; jne`). Writing the peel as `text_get(); for
                // (int j = faction_id; j > 0; j--) text_get();` is the same
                // shape but gives j a stack home and grows the frame to
                // 0xc against the image's 8 - the guard-then-do-while
                // spelling below is what keeps the counter in esi.
                // The image PEELS this loop: one UNCONDITIONAL text_get
                // (0x005870E2), then the remainder as a guard-and-do-while
                // (`test edi,edi; jle`, `mov esi,edi`,
                // `loop: call; dec esi; jne` at 0x5870ED). The image's own
                // `test edi,edi; jle` implies that for faction_id <= 0 it
                // still consumes one text_get where the plain
                // `for (j = faction_id; j >= 0; j--)` spelling below consumes
                // none - unreachable, since faction_id is `rand_val % 7`
                // here (the `faction_id = -1` assignment is always overwritten
                // by the reroll before the loop is re-entered). Peel and plain
                // for measured back to back: 75/214 against 71/214.
                text_get();
                if (faction_id > 0) {
                    int j = faction_id;
                    do {
                        text_get();
                    } while (--j != 0);
                }
                strcpy(Players[player].filename, text_item());
                // The image copies the FILENAME, not a second token:
                // `lea edx,[ebx+0x18]; push ebx; push edx; call strcpy` at
                // 0x00587101 - one text_item, two DIRECT strcpy calls (see
                // the BONUSNAMES note above), one `add esp,0x10` for both.
                // The old reading consumed a second token the image never
                // takes.
                strcpy(Players[player].search_key, Players[player].filename);
                for (int k = 1; k < MaxPlayerNum; k++) {
                    if (player != k) {
                        if (!strcmp(Players[player].filename, Players[k].filename)) {
                            faction_id = -1;
                            k = MaxPlayerNum;  // not break: the image stores 8
                        }
                    }
                }
                // TWO tests in the image: `lea edx,[edi+1]; test edx,edx;
                // je` for faction_id == -1, then `cmp ebx,0x946a54; je` -
                // `player != 0` strength-reduced onto the filename walk
                // (0x00587146-0x00587153).
                if (faction_id + 1 != 0 && player != 0) {
                    read_faction(&Players[player], 0);
                    load_faction_art(player);
                }
            } while (faction_id == -1);
        }
    }
    // TWO LOOPS IN THE IMAGE, not one merged loop: the JENN282 loop above
    // sends every other player straight to its increment (`jne 0x00587175`
    // at 0x00587060), and this second loop walks the players once more for
    // the faction parse - so the original reads each random faction twice.
    // The `cmp esi,0x946a50; je` at 0x00587198 is the `!= &Players[0]`
    // check VC6 does not fold away even though the walk starts at [1].
    for (player = 1; player < MaxPlayerNum; player++) {
        if (&Players[player] != &Players[0]) {
            read_faction(&Players[player], 0);
            load_faction_art(player);
        }
    }
    return false;
}

/*
Purpose: Parse and set the noun item's gender and plurality from the Txt buffer.
// ORIGINAL: 0x005871D0 ?noun_item@@YAXPAHPAH@Z 0x005871D0-0x0058723E BYTE_EXACT
// symbol    ?noun_item@@YAXPAH0@Z
// size      110 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD670
Return Value: n/a
Status: Complete
*/
void __cdecl noun_item(int *gender, int *plurality) {
    char *noun = text_item();
    *gender = 0; // defaults to male ('m' || 'M')
    *plurality = false; // defaults to singular ('1')
    // Four independent `if`s, not a chain: the original re-loads and re-tests
    // the character for each, and the 'M' arm redundantly re-stores the
    // default. `noun++` between the two plurality tests is load-bearing.
    if (noun[0] == 'M' || noun[0] == 'm') {
        *gender = 0;
    }
    if (noun[0] == 'F' || noun[0] == 'f') {
        *gender = 1;
    }
    if (noun[0] == 'N' || noun[0] == 'n') {
        *gender = 2;
    }
    noun++;
    if (noun[0] == '1') {
        *plurality = false;
    }
    if (noun[0] == '2') {
        *plurality = true;
    }
}

/*
Purpose: Parse the #UNITS section inside the alpha(x).txt.
// ORIGINAL: 0x00587240 ?read_units@@YAHXZ 0x00587240-0x005873B1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned read_units: ability parser call at 0x00587316 proceeds directly to the prototype switch; make_proto call at 0x00587364 receives reactor 1/2/3 for prototypes 16/17/18, default 1.", "Pinned 0x0058736F..0x00587373 tests the signed plan and branches past its byte store on JL.", "Git 11a07309a introduced the optional reactor-token extension; it is preserved in Git for the later improvement milestone."], "reason": "The faithful rules reader consumes no optional reactor token after abilities, derives reactors from prototype IDs, and skips the plan override for every negative plan. Historical TRIED prose below describes the superseded reactor extension, retained as seed history only."}
// LEVER: 0x00645460 is plain `strcpy` (2 args - confirmed at its call site,
//        `push name; push &veh_name; call`, no length/size pushed anywhere
//        nearby) not `strncpy_s`; the tree's `strncpy_s(veh_name, 32, name,
//        strlen(name))` was an extra `strlen` call the image never makes.
//        17/134 -> 22/134.
// LEVER: `int total_units = text_get_number(0, MaxVehProtoFactionNum);` rewritten as the
//        image's two-statement `text_get(); int total_units = range(text_item_number(), 0,
//        MaxVehProtoFactionNum);` (same defect as read_basic_rules - text_get_number inlines
//        through text_get_source/text_item_number_source, but the image calls the zero-arg
//        text_get()/text_item_number() wrappers). Moved 22/134 -> 48/134 agreeing.
// TRIED: not chased further - the image never reads a "reactor" field at
//            all (only 4 `text_item_number` calls: plan, cost, carry, icon;
//            `ability`'s `text_item_binary` result stays live in EAX straight
//            into the proto_id switch that always computes reactor_id, no
//            gating check). This tree's `int reactor_id = text_item_number();
//            // Add ability to read reactor for #UNITS` is a DELIBERATE
//            project enhancement (own comment says so) that consumes one more
//            token per unit than the original parser ever did - the extra
//            local is why `sub esp` is 0x20 here against the image's 0x1c,
//            and everything downstream is offset by it. Left in place: this
//            is a feature the reimplementation adds on purpose, not a
//            recoverable divergence.
// size      369 bytes
// prototype 
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00584D60 0x00584E40 0x00584F40 0x00585030 0x005A5D40 0x005FD550 0x005FD570 0x005FD670 0x005FD740 0x005FD7A0 0x00645460
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl read_units() {
    if (text_open(AlphaxFileID, "UNITS")) {
        return true;
    }
    text_get();
    int total_units = range(text_item_number(), 0, MaxVehProtoFactionNum);
    for (int proto_id = 0; proto_id < total_units; proto_id++) {
        text_get();
        strcpy(VehPrototypes[proto_id].veh_name, text_item());
        int chas_id = chas_name(text_item());
        int weap_id = weap_name(text_item());
        int armor_id = arm_name(text_item());
        int plan = text_item_number();
        int cost = text_item_number();
        int carry = text_item_number();
        VehPrototypes[proto_id].preq_tech = (int16_t)tech_name(text_item());
        int icon = text_item_number();
        int ability = text_item_binary();
        if (proto_id == BSC_BATTLE_OGRE_MK1) {
            make_proto(proto_id, chas_id, weap_id, armor_id, ability, RECT_FISSION);
        } else if (proto_id == BSC_BATTLE_OGRE_MK2) {
            make_proto(proto_id, chas_id, weap_id, armor_id, ability, RECT_FUSION);
        } else if (proto_id == BSC_BATTLE_OGRE_MK3) {
            make_proto(proto_id, chas_id, weap_id, armor_id, ability, RECT_QUANTUM);
        } else {
            make_proto(proto_id, chas_id, weap_id, armor_id, ability, RECT_FISSION);
        }
        // If set, override auto calculated values from make_proto()
        if (plan >= 0) {
            VehPrototypes[proto_id].plan = (uint8_t)plan;
        }
        if (cost) { // cost auto calculate: 0
            VehPrototypes[proto_id].cost = (uint8_t)cost;
        }
        if (carry) { // carry auto calculate: 0
            VehPrototypes[proto_id].carry_capacity = (uint8_t)carry;
        }
        VehPrototypes[proto_id].icon_offset = (int8_t)icon;
    }
    return false;
}

/*
Purpose: Parse in all the game rules via alpha/x.txt. If the toggle param is set to true, parse the
         #UNITS & #FACTIONS sections. Otherwise, skip both. New game vs reload?
// ORIGINAL: 0x005873C0 ?read_rules@@YAHH@Z 0x005873C0-0x0058829C BYTE_EXACT
// LEVER: the #TERRAIN loop's `order_str` is copied into a 256-byte LOCAL
//        buffer (`strcpy` at 0x005874AE into `[ebp-0x114]`) before being
//        handed to `parse_string` - not held as a bare `LPSTR`. That
//        local is almost the entire gap between this tree's frame and the
//        image's (`sub esp, 0x114` vs a bare-pointer version's far smaller
//        one); after adding it the frame is 0x11c against the image's
//        0x114, 8 bytes (two DWORDs) still unaccounted for somewhere else
//        in this ~40-#SECTION function. 51/1217 -> 65/1217.
// TRIED: the rest of this function - it is 883 image instructions across
//            dozens of independent #SECTION loops (most already
//            array-indexed field-for-field against the shipped struct
//            layouts); several read like the read_tech pointer-walk (e.g.
//            the #CHASSIS/#TERRAIN loops advance a struct pointer + a
//            parallel int index) but confirming and rewriting each one is
//            out of scope for this pass. Left as a faithful, compiling
//            transcription; not chased to byte-exact.
// size      3804 bytes
// prototype BOOL (__cdecl ?read_rules@@YAHH@Z)(BOOL tglAllRules)
// callers   3   call targets   23
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00584D60 0x00585170 0x00585E30 0x00586F30 0x00587240 0x005FD550 0x005FD570 0x005FD670 0x005FD6D0 0x005FD740 0x005FD7A0 0x005FE1F0 0x005FE270 0x00607550 0x006168F0 0x00616970 0x006169A0 0x00616A00 0x00625880 0x00625E50 0x006453E0 0x00645460 0x0064FD20
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl read_rules(BOOL tgl_all_rules) {
    StringTable.init(49952);
    if (labels_init()) {
        return true;
    }
    text_clear_index();
    text_make_index(ScriptPopupFile);
    text_make_index(AlphaxFileID);
    if (read_tech()) {
        return true;
    }
    if (read_basic_rules()) {
        return true;
    }
    if (text_open(AlphaxFileID, "TERRAIN")) {
        return true;
    }
    for (int i = 0; i < MaxTerrainNum; i++) {
        // A named order index: the image carries (i + 4) * 12 as one byte
        // offset from Order (0x30 .. cmp 0x120); `Order[i + 4]` at each use
        // gives a pointer IV tested against the table address instead.
        int order_id = i + ORDER_FARM;
        text_get();
        // Land + sea names and prerequisites; the bit masks are static data.
        // The image walks name[] and order[] with one pointer (dec/jne at
        // 0x00587494 and 0x00587507).
        for (int j = 0; j < 2; j++) {
            Terraforming[i].name[j] = text_item_string();
            Terraforming[i].prerequisite_tech[j] = tech_name(text_item());
        }
        Terraforming[i].rate = text_item_number();
        // Land + sea orders
        // NOT held as a bare pointer: the image copies `text_item()`'s
        // result into a 256-byte LOCAL buffer first (`strcpy` at
        // 0x005874AE, into `[ebp-0x114]`) before handing it to
        // `parse_string` - that local is why the image's frame is
        // 0x114 bytes against a bare-pointer version's much smaller one.
        char order_buf[256];
        strcpy(order_buf, text_item());
        for (j = 0; j < 2; j++) {
            parse_say(0, Terraforming[i].name[j], -1, -1);
            StringTemp[0] = 0;
            parse_string(order_buf, StringTemp);
            Order[order_id].order[j] = StringTable.put(StringTemp);
        }
        Order[order_id].letter = text_item_string();
        Terraforming[i].shortcuts = text_item_string();
    }
    if (text_open(AlphaxFileID, "RESOURCEINFO")) {
        return true;
    }
    for (i = 0; i < MaxResourceInfoNum; i++) {
        text_get();
        text_item();
        // WORKAROUND(type): one int pointer runs through every row
        // (dec/jne inside, jl against the table end at 0x00587586).
        for (int j = 0; j < 4; j++) {
            ((int *)ResourceInfo)[i * 4 + j] = text_item_number();
        }
    }
    if (text_open(AlphaxFileID, "TIMECONTROLS")) {
        return true;
    }
    for (i = 0; i < MaxTimeControlNum; i++) {
        text_get();
        TimeControl[i].name = text_item_string();
        TimeControl[i].turn = text_item_number();
        TimeControl[i].base = text_item_number();
        TimeControl[i].unit = text_item_number();
        TimeControl[i].event = text_item_number();
        TimeControl[i].extra = text_item_number();
        TimeControl[i].refresh = text_item_number();
        TimeControl[i].accumulated = text_item_number();
    }
    if (text_open(AlphaxFileID, "CHASSIS")) {
        return true;
    }
    for (i = 0; i < MaxChassisNum; i++) {
        text_get();
        Chassis[i].offsv_1_name = text_item_string();
        noun_item((int *)&Chassis[i].offsv_1_gender, &Chassis[i].offsv_1_plural);
        Chassis[i].offsv_2_name = text_item_string();
        noun_item((int *)&Chassis[i].offsv_2_gender, &Chassis[i].offsv_2_plural);
        Chassis[i].defsv_1_name = text_item_string();
        noun_item((int *)&Chassis[i].defsv_1_gender, &Chassis[i].defsv_1_plural);
        Chassis[i].defsv_2_name = text_item_string();
        noun_item((int *)&Chassis[i].defsv_2_gender, &Chassis[i].defsv_2_plural);
        Chassis[i].speed = (uint8_t)text_item_number();
        Chassis[i].triad = (uint8_t)text_item_number();
        Chassis[i].range = (uint8_t)text_item_number();
        Chassis[i].missile = (uint8_t)text_item_number();
        Chassis[i].cargo = (uint8_t)text_item_number();
        Chassis[i].cost = (uint8_t)text_item_number();
        Chassis[i].preq_tech = (int16_t)tech_name(text_item());
        Chassis[i].offsv_lrg_name = text_item_string();
        noun_item((int *)&Chassis[i].offsv_lrg_gender, &Chassis[i].offsv_lrg_plural);
        Chassis[i].defsv_lrg_name = text_item_string();
        noun_item((int *)&Chassis[i].defsv_lrg_gender, &Chassis[i].defsv_lrg_plural);
    }
    if (text_open(AlphaxFileID, "REACTORS")) {
        return true;
    }
    for (i = 0; i < MaxReactorNum; i++) {
        text_get();
        Reactor[i].name = text_item_string();
        Reactor[i].name_short = text_item_string();
        // Pinned 0x005878CE consumes the power token as text, without a
        // numeric conversion or write. Keep existing power storage unchanged,
        // including across reloads; static initialization supplies its first zero.
        text_item();
        Reactor[i].preq_tech = (int16_t)tech_name(text_item());
    }
    if (text_open(AlphaxFileID, "WEAPONS")) {
        return true;
    }
    for (i = 0; i < MaxWeaponNum; i++) {
        text_get();
        Weapon[i].name = text_item_string();
        Weapon[i].name_short = text_item_string();
        Weapon[i].offense_rating = (int8_t)text_item_number();
        Weapon[i].mode = (uint8_t)text_item_number();
        Weapon[i].cost = (uint8_t)text_item_number();
        Weapon[i].icon = (int8_t)text_item_number();
        Weapon[i].preq_tech = (int16_t)tech_name(text_item());
    }
    if (text_open(AlphaxFileID, "DEFENSES")) { // Armor
        return true;
    }
    for (i = 0; i < MaxArmorNum; i++) {
        text_get();
        Armor[i].name = text_item_string();
        Armor[i].name_short = text_item_string();
        Armor[i].defense_rating = (char)text_item_number();
        Armor[i].mode = (uint8_t)text_item_number();
        Armor[i].cost = (uint8_t)text_item_number();
        Armor[i].preq_tech = (int16_t)tech_name(text_item());
    }
    if (text_open(AlphaxFileID, "ABILITIES")) {
        return true;
    }
    for (i = 0; i < MaxAbilityNum; i++) {
        text_get();
        Ability[i].name = text_item_string();
        Ability[i].cost_factor = text_item_number();
        Ability[i].preq_tech = (int16_t)tech_name(text_item());
        Ability[i].abbreviation = text_item_string();
        Ability[i].flags = text_item_binary();
        Ability[i].description = text_item_string();
    }
    if (text_open(AlphaxFileID, "MORALE")) {
        return true;
    }
    for (i = 0; i < MaxMoraleNum; i++) {
        text_get();
        Morale[i].name = text_item_string();
        Morale[i].name_lifecycle = text_item_string();
    }
    if (text_open(AlphaxFileID, "DEFENSEMODES")) {
        return true;
    }
    for (i = 0; i < MaxDefenseModeNum; i++) {
        text_get();
        DefenseModes[i].name = text_item_string();
        DefenseModes[i].hyphened = text_item_string();
        DefenseModes[i].abbrev = text_item_string();
        DefenseModes[i].letter = text_item_string();
    }
    if (text_open(AlphaxFileID, "OFFENSEMODES")) {
        return true;
    }
    for (i = 0; i < MaxOffenseModeNum; i++) {
        text_get();
        OffenseModes[i].name = text_item_string();
        OffenseModes[i].hyphened = text_item_string();
        OffenseModes[i].abbrev = text_item_string();
        OffenseModes[i].letter = text_item_string();
    }
    // Units basic prototypes (only if new game param1 boolean is set)
    // TODO: Potential bug, look into issues with Veh data persisting between loaded saved games?
    if (tgl_all_rules) {
        for (int i = 0; i < MaxVehProtoNum; i++) {
            VehPrototypes[i].veh_name[0] = 0;
            VehPrototypes[i].flags = 0;
            VehPrototypes[i].unk_1 = 0;
        }
        if (read_units()) {
            return true;
        }
    }
    if (text_open(AlphaxFileID, "FACILITIES")) {
        return true;
    }
    for (i = 1; i < MaxFacilityNum; i++) { // Facility[0] is null
        text_get();
        Facility[i].name = text_item_string();
        Facility[i].cost = text_item_number();
        Facility[i].maint = text_item_number();
        Facility[i].preq_tech = tech_name(text_item());
        // Preserve the parsed value through the effect-string callback, then
        // disable it as the original does. Enabling free facilities is a later
        // improvement, not the original alpha/x.txt interpretation.
        Facility[i].free_tech = tech_name(text_item());
        Facility[i].effect = text_item_string();
        // Pinned 0x00587BC6..D7: parsed store, effect read/store, then -2.
        Facility[i].free_tech = -2;
        if (i >= FacilitySPStart) {
            Facility[i].sp_ai_fight = text_item_number();
            Facility[i].sp_ai_power = text_item_number();
            Facility[i].sp_ai_tech = text_item_number();
            Facility[i].sp_ai_wealth = text_item_number();
            Facility[i].sp_ai_growth = text_item_number();
        }
    }
    if (text_open(AlphaxFileID, "ORDERS")) { // Basic
        return true;
    }
    for (i = 0; i < MaxOrderNum; i++) {
        if (i < 4 || i >= 24) { // Skipping over orders set by #TERRAIN
            text_get();
            Order[i].order[0] = text_item_string();
            // Potential enhancement: Have separate string for sea
            Order[i].order[1] = Order[i].order[0];
            Order[i].letter = text_item_string();
        }
    }
    if (text_open(AlphaxFileID, "COMPASS")) {
        return true;
    }
    
for (i = 0; i < MaxCompassNum; i++) {
        text_get();
        Compass[i] = text_item_string();
    }
    if (text_open(AlphaxFileID, "PLANS")) {
        return true;
    }
    for (i = 0; i < MaxPlanNum; i++) {
        text_get();
        // Future clean-up: Create structure with both short & full name vs split memory
        PlansShortName[i] = text_item_string();
        PlansFullName[i] = text_item_string();
    }
    if (text_open(AlphaxFileID, "TRIAD")) {
        return true;
    }
    for (i = 0; i < MaxTriadNum; i++) {
        text_get();
        Triad[i] = text_item_string();
    }
    if (text_open(AlphaxFileID, "RESOURCES")) {
        return true;
    }
    for (i = 0; i < MaxResourceNum; i++) {
        text_get();
        Resource[i].name_singular = text_item_string();
        Resource[i].name_plural = text_item_string();
    }
    if (text_open(AlphaxFileID, "ENERGY")) {
        return true;
    }
    for (i = 0; i < MaxEnergyNum; i++) {
        text_get();
        Energy[i].abbrev = text_item_string();
        Energy[i].name = text_item_string();
    }
    if (text_open(AlphaxFileID, "CITIZENS")) {
        return true;
    }
    for (i = 0; i < MaxCitizenNum; i++) {
        text_get();
        Citizen[i].singular_name = text_item_string();
        Citizen[i].plural_name = text_item_string();
        if (i < 7) {
            Citizen[i].preq_tech = tech_name(text_item());
            Citizen[i].obsol_tech = tech_name(text_item());
            // The three bonuses are filled by a dec/jne loop (0x00587E30).
            for (int j = 0; j < 3; j++) {
                Citizen[i].bonus[j] = text_item_number();
            }
        }
    }
    if (text_open(AlphaxFileID, "SOCIO")) {
        return true;
    }
    text_get();
    for (i = 0; i < MaxSocialEffectNum; i++) {
        strcpy(SocialEffects[i].set1, text_item());
    }
    text_get();
    for (i = 0; i < MaxSocialEffectNum; i++) {
        strcpy(SocialEffects[i].set2, text_item());
    }
    text_get();
    for (i = 0; i < MaxSocialCatNum; i++) {
        SocialCategories[i].type = text_item_string();
    }
    for (i = 0; i < MaxSocialCatNum; i++) {
        for (int j = 0; j < MaxSocialModelNum; j++) {
            text_get();
            SocialCategories[i].name[j] = text_item_string();
            SocialCategories[i].preq_tech[j] = tech_name(text_item());
            for (int clear = 0; clear < 11; clear++) {
                SocialCategories[i].model_effect[j].values_[clear] = 0;
            }
            // The image adds the value to every matching effect: no break.
            while (strlen(text_item())) {
                LPSTR mod_value = TextBufferItemPtr;
                int value = 0;
                while (mod_value[0] == '+' || mod_value[0] == '-') {
                    (mod_value[0] == '+') ? value++ : value--;
                    mod_value++;
                }
                for (int k = 0; k < MaxSocialEffectNum; k++) {
                    if (!_stricmp(mod_value, SocialEffects[k].set1)) {
                        SocialCategories[i].model_effect[j].values_[k] += value;
                    }
                }
            }
        }
    }
    if (text_open(AlphaxFileID, "DIFF")) { // Difficulty
        return true;
    }
    for (i = 0; i < MaxDiffNum; i++) {
        text_get();
        Difficulty[i] = text_item_string();
    }
    if (tgl_all_rules && read_factions()) {
        return true;
    }
    if (text_open(AlphaxFileID, "MANDATE")) {
        return true;
    }
    for (i = 0; i < MaxMandateNum; i++) {
        text_get();
        Mandate[i].name = text_item_string();
        Mandate[i].name_caps = text_item_string();
    }
    if (text_open(AlphaxFileID, "MOOD")) {
        return true;
    }
    for (i = 0; i < MaxMoodNum; i++) {
        text_get();
        Mood[i] = text_item_string();
    }
    if (text_open(AlphaxFileID, "REPUTE")) {
        return true;
    }
    for (i = 0; i < MaxReputeNum; i++) {
        text_get();
        Repute[i] = text_item_string();
    }
    if (text_open(AlphaxFileID, "MIGHT")) {
        return true;
    }
    for (i = 0; i < MaxMightNum; i++) {
        text_get();
        Might[i].adj_start = text_item_string();
        Might[i].adj = text_item_string();
    }
    if (text_open(AlphaxFileID, "PROPOSALS")) {
        return true;
    }
    for (i = 0; i < MaxProposalNum; i++) {
        text_get();
        Proposal[i].name = text_item_string();
        Proposal[i].preq_tech = tech_name(text_item());
        Proposal[i].description = text_item_string();
    }
    if (text_open(AlphaxFileID, "NATURAL")) {
        return true;
    }
    for (i = 0; i < MaxNaturalNum; i++) {
        text_get();
        Natural[i].name = text_item_string();
        Natural[i].name_short = text_item_string();
    }
    // Superseded rationale: "Revised original nested for loop code to be more
    // efficient; Logic is still same." The original translates each of 15 fixed
    // button ordinals through all 20 rows anew, including after button callbacks.
    // Buttons used by "Edit Map" menus.
    for (int button = 17; button <= 31; ++button) {
        int terrain = button - 17;
        for (int entry = 0; entry < MaxTerrainNum; ++entry) {
            if (Terraforming[entry].bit == 0 && terrain >= entry) ++terrain;
        }
        MainInterfaceGlobal.flat_button_[button].set_bubble_text(
            StringTable.get(Terraforming[terrain].name[0]));
    }
    MainInterfaceGlobal.flat_button(32).set_bubble_text(StringTable.get(Natural[LM_JUNGLE].name_short));
    MainInterfaceGlobal.flat_button(33).set_bubble_text(StringTable.get(Natural[LM_DUNES].name_short));
    MainInterfaceGlobal.flat_button(34).set_bubble_text(StringTable.get(Natural[LM_URANIUM].name_short));
    MainInterfaceGlobal.flat_button(35).set_bubble_text(StringTable.get(Natural[LM_GEOTHERMAL].name_short));
    return false;
}

