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
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#include "stdafx.h"
// WORKAROUND(decl): custom_planet 0x0058C2A0 (not exact) swings with how many
// declarations precede game.h's (guide 2.5). After the dead-helper batch
// (d64959c815) it read 373 rows with no pads; fe sweep 0-63: 11 rows at 1-7,
// 10, 13-14, 17, 20-39, 43, 46-50, 53, 56-57, 59-62 and 373-412 elsewhere, no
// other function of the unit changes, so 30. After newgame.cpp joined this
// unit (2026-11-19) the fe sweep reads 11 rows at 0-4, 11-22, 26-29, 31-38,
// 41, 44-45, 48, 53-63 and 373-412 elsewhere (404 at 30), so 31. At those
// counts VC6 itself homes &custom_world[slot] and &planet_rules_[slot]
// ([ebp-0x1C], [ebp-0x14] in the image) and lets slot die at the switch; named
// pointer locals for the two addresses read 84 rows at best, so they are not
// the original spelling. Left: i and the two address homes permuted. The
// original header declaration set is unknown; re-tune after header batches.
// After par-1's header batch (015360dbf5): 11 rows at 0-10, 12, 15-16, 21-22, 25, 27, 29-35, 38,
// 41-44, 47 and 50-63 (373-412 elsewhere), every other function of the unit exact at all counts, so 56.
// HEADER-PADS: function NewGameHeaderPad
void NewGameHeaderPad0();
void NewGameHeaderPad1();
void NewGameHeaderPad2();
void NewGameHeaderPad3();
void NewGameHeaderPad4();
void NewGameHeaderPad5();
void NewGameHeaderPad6();
void NewGameHeaderPad7();
void NewGameHeaderPad8();
void NewGameHeaderPad9();
void NewGameHeaderPad10();
void NewGameHeaderPad11();
void NewGameHeaderPad12();
void NewGameHeaderPad13();
void NewGameHeaderPad14();
void NewGameHeaderPad15();
void NewGameHeaderPad16();
void NewGameHeaderPad17();
void NewGameHeaderPad18();
void NewGameHeaderPad19();
void NewGameHeaderPad20();
void NewGameHeaderPad21();
void NewGameHeaderPad22();
void NewGameHeaderPad23();
void NewGameHeaderPad24();
void NewGameHeaderPad25();
void NewGameHeaderPad26();
void NewGameHeaderPad27();
void NewGameHeaderPad28();
void NewGameHeaderPad29();
void NewGameHeaderPad30();
void NewGameHeaderPad31();
void NewGameHeaderPad32();
void NewGameHeaderPad33();
void NewGameHeaderPad34();
void NewGameHeaderPad35();
void NewGameHeaderPad36();
void NewGameHeaderPad37();
void NewGameHeaderPad38();
void NewGameHeaderPad39();
void NewGameHeaderPad40();
void NewGameHeaderPad41();
void NewGameHeaderPad42();
void NewGameHeaderPad43();
void NewGameHeaderPad44();
void NewGameHeaderPad45();
void NewGameHeaderPad46();
void NewGameHeaderPad47();
void NewGameHeaderPad48();
void NewGameHeaderPad49();
void NewGameHeaderPad50();
void NewGameHeaderPad51();
void NewGameHeaderPad52();
void NewGameHeaderPad53();
void NewGameHeaderPad54();
void NewGameHeaderPad55();

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
#include "interface_fonts.h"
#include "popup.h"
#include "popup_fonts.h"
#include "menu.h"
#include "pulldown.h"
#include "basepop_defaults.h"
#include "messagewin.h"
#include "diplowin.h"   // DiploKnownTechColor
#include "scroll.h"
#include "sprite.h"
#include "listbox.h"
#include "font.h"
#include "faction.h"
#include "factionprofilewin.h"
#include "rectangle_operations.h"
#include "interface_palette.h"
#include "main.h"
#include "win.h"
#include "path.h"
#include "setupwin.h"
#include "pickwin.h"
#include "basepop.h"
#include "graphicwin.h"
#include "palette.h"
#include "console.h"
#include "designwin.h"
#include "tutwin.h"
#include "worldwin.h"
#include "multiwin.h"
#include "maininterface.h"
#include "netwin.h"
#include "netdaemon.h"
#include "lock.h"
#include "log.h"
#include "time.h"
#include "editgroup.h"
#include "statuswin.h"
#include "flic.h"
#include "ambience.h"  // load_music, the startup faction ambience owner.
#include "infowin.h"
#include "diplopop.h"  // DiplomacyPopup
#include "reportif.h"
#include "prefwin.h"  // ConsolePrefWin.init needs the complete owner declaration.
#include "battlewin.h"  // the standing battle window
#include "multidebug.h"  // MultiDebug, the standing debug window
#include "datalink.h"  // DatalinkMain

// timer_daemon reads this climate gate before dispatch; its wider contract remains unresolved.
// RECOVERY_OBJECT: v2 0x0068F21C 0x0068F220 ?WorldClimateSkipTerrainClear@@3IA 0x0058E360
uint32_t WorldClimateSkipTerrainClear = 1;  // 0x0068F21C
// Rank rows use one palette entry per faction, including native life.
// RECOVERY_OBJECT: v2 0x0068F9F0 0x0068FA10 ?InfoFactionTextColors@@3PAHA 0x00458420
int InfoFactionTextColors[8] = {249, 111, 252, 255, 251, 65, 102, 187};
// RECOVERY_OBJECT: v2 0x0068FA10 0x0068FA30 ?InfoFactionShadowColors@@3PAHA 0x00458420
int InfoFactionShadowColors[8] = {1, 175, 4, 248, 3, 57, 109, 192};
// Eight faction foreground colors, independently paired with eight shadows.
// load_faction_art4537D9/4537E8 writes the two banks and load_vehicles
// 45443C/454446 publishes entry0. The following 68FA70 string is NOT an
// array tail. Original chat caller537AD5 passes the packet faction index;
// malformed packet indices remain unvalidated, not a larger storage owner.
// Other original-only color users and load_faction_art still need recovery.
// RECOVERY_OBJECT: v2 0x0068FA30 0x0068FA50 ?ChatItemIdTable@@3PAHA 0x00454270
int ChatItemIdTable[8] = {
    249, 111, 252, 255, 251, 65, 102, 187,
};
// RECOVERY_OBJECT: v2 0x0068FA50 0x0068FA70 ?ChatShadowColors@@3PAHA 0x00454270
int ChatShadowColors[8] = {
    134, 177, 172, 55, 146, 51, 145, 169,
};

// Borrowed FontTable entries selected by application popup policy.
// config_popups publishes these pointers; they own no Font lifetime.
// RECOVERY_OBJECT: v1 0x009403D8 ?PopupMainFont@@3PAVFont@@A
Font *PopupMainFont;          // 0x009403D8
// RECOVERY_OBJECT: v1 0x009403DC ?PopupSmallFont@@3PAVFont@@A
Font *PopupSmallFont;         // 0x009403DC
// RECOVERY_OBJECT: v1 0x0094575C ?PopupMainBoldFont@@3PAVFont@@A
Font *PopupMainBoldFont;      // 0x0094575C
// RECOVERY_OBJECT: v1 0x00945760 ?PopupMainItalicFont@@3PAVFont@@A
Font *PopupMainItalicFont;    // 0x00945760
// RECOVERY_OBJECT: v1 0x00945764 ?PopupMenuBoldFont@@3PAVFont@@A
Font *PopupMenuBoldFont;      // 0x00945764
// RECOVERY_OBJECT: v1 0x00945768 ?PopupMenuItalicFont@@3PAVFont@@A
Font *PopupMenuItalicFont;    // 0x00945768
// RECOVERY_OBJECT: v1 0x0094576C ?PopupMediumFont@@3PAVFont@@A
Font *PopupMediumFont;        // 0x0094576C
// RECOVERY_OBJECT: v1 0x00945770 ?InterludeNormalFont@@3PAVFont@@A
Font *InterludeNormalFont;    // 0x00945770
// RECOVERY_OBJECT: v1 0x00945774 ?PopupLargeFont@@3PAVFont@@A
Font *PopupLargeFont;         // 0x00945774
// RECOVERY_OBJECT: v1 0x00945778 ?PopupMapLabelFont@@3PAVFont@@A
Font *PopupMapLabelFont;      // 0x00945778
// RECOVERY_OBJECT: v1 0x009457EC ?PopupMenuFont@@3PAVFont@@A
Font *PopupMenuFont;          // 0x009457EC
// RECOVERY_OBJECT: v1 0x009457F0 ?PopupSmallItalicFont@@3PAVFont@@A
Font *PopupSmallItalicFont;   // 0x009457F0
// RECOVERY_OBJECT: v1 0x009457F4 ?PopupSmallBoldFont@@3PAVFont@@A
Font *PopupSmallBoldFont;     // 0x009457F4
// RECOVERY_OBJECT: v1 0x009457F8 ?InterludeThirdFont@@3PAVFont@@A
Font *InterludeThirdFont;     // 0x009457F8
// RECOVERY_OBJECT: v1 0x009457FC ?InterludeSecondFont@@3PAVFont@@A
Font *InterludeSecondFont;    // 0x009457FC
// RECOVERY_OBJECT: v1 0x00945800 ?PopupLargeItalicFont@@3PAVFont@@A
Font *PopupLargeItalicFont;   // 0x00945800
// RECOVERY_OBJECT: v1 0x00945804 ?PopupLargeBoldFont@@3PAVFont@@A
Font *PopupLargeBoldFont;     // 0x00945804
// RECOVERY_OBJECT: v1 0x00945808 ?PopupMediumItalicFont@@3PAVFont@@A
Font *PopupMediumItalicFont;  // 0x00945808
// RECOVERY_OBJECT: v1 0x0094580C ?PopupMediumBoldFont@@3PAVFont@@A
Font *PopupMediumBoldFont;    // 0x0094580C
// RECOVERY_OBJECT: v1 0x00945810 ?PopupDefaultWindowFont@@3PAVFont@@A
Font *PopupDefaultWindowFont; // 0x00945810
// Signed preference: negative/default/positive select slow/normal/fast.
// The independent INI loader 0x00588AA0 names it FastUnitAnim.
// RECOVERY_OBJECT: v2 0x0094581C 0x00945820 ?FastUnitAnim@@3HA 0x0055A150
int FastUnitAnim;
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Tutorial tour's original 0x945830 gate controls WorldClimateWorldWin's
// restoration. It is source-owned, writable state with loader-zero initial
// storage; no writer or larger object owner is established.
// RECOVERY_OBJECT: v1 0x00945830 ?TutWinClimateGate945830@@3HA
int TutWinClimateGate945830;

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
// Alpha owns the parsed #COMPASS pointer IDs populated by read_rules.
// RECOVERY_OBJECT: v1 0x00945D48 ?Compass@@3PAPADA
LPSTR Compass[MaxCompassNum];      // 0x00945D48


// rankings (0x005AC690) writes the eight native dwords at
// [0x009A64EC,0x009A650C). setup_player's apparent endgame selector
// cells 0x009A6504/0x009A6508 are entries 6/7 of this same allocation;
// InfoWin::setup_text also reads entry 7, not an independent scalar.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A64EC
// ORIGINAL: 0x00584D00 _$E56 0x00584D00-0x00584D16 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E16
// ORIGINAL: 0x00584D20 _$E54 0x00584D20-0x00584D2A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E14
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

// RECOVERY_LITERAL: v1 0x0068F2FC 0x0068F30A $SG64577 0x00585E30
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


// From src/interface_fonts.cpp (unit prelude):
// ORIGINAL: 0x005882A0 _$E19 0x005882A0-0x005882C4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E21
// ORIGINAL: 0x005882D0 _$E17 0x005882D0-0x005882E4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E19



extern int TextColourTable[195];
extern uint32_t EditBoxDefaultB20;
extern uint32_t EditBoxDefaultB24;
extern uint32_t EditBoxDefaultB28;
extern uint32_t EditBoxDefaultB2C;
extern LPCSTR CreditsFontFace;

/*
Purpose: Select the Font whose point size is closest to the requested one,
         then index that size's row by style.
// ORIGINAL: 0x005882F0 ?find_font@@YAHHH@Z 0x005882F0-0x0058833B BYTE_EXACT
// symbol    ?find_font@@YAPAVFont@@HH@Z
// size      75 bytes
// prototype 
// callers   31   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Font *__cdecl find_font(int size, int style) {
    int best_index = 0;
    // The seed doubles as a threshold: a requested size further than 9999 from
    // every candidate matches nothing and falls back to the first entry.
    int best_delta = 9999;
    for (size_t index = 0; index < FontSizeTableCount; ++index) {
        const int candidate = FontSizeTable[index];
        // Absolute difference as one conditional expression: in config_popups'
        // ten expansions best_index keeps ESI and candidate stays a scratch
        // register (0x00588C0D mov ecx,[edx]); the if-assigned delta promoted
        // candidate to ESI and tied main_size/interlude_size the other way.
        int delta = candidate - size < 0 ? size - candidate : candidate - size;
        if (delta < best_delta) {
            best_delta = delta;
            best_index = static_cast<int>(index);
        }
    }
    // Each size occupies four consecutive style slots in the table.
    return FontTable + (style + best_index * 4);
}

// The original setters populate the same twelve sprite slots in four
// triplets. These are source-owned sprite pointers, not addresses in the old
// executable. Index 14 is an untouched gap; index 16 is another process
// default that neither setter changes.
static inline void set_scroll_icon_defaults(Sprite *arrows,
                                                    Sprite *filler,
                                                    int thickness) {
    ScrollDefaultSpriteUp[0] = &arrows[0];
    ScrollDefaultSpriteUp[1] = &arrows[1];
    ScrollDefaultSpriteUp[2] = &arrows[2];
    ScrollDefaultSpriteDown[0] = &arrows[3];
    ScrollDefaultSpriteDown[1] = &arrows[4];
    ScrollDefaultSpriteDown[2] = &arrows[5];
    ScrollDefaultSpriteLeft[0] = &arrows[6];
    ScrollDefaultSpriteLeft[1] = &arrows[7];
    ScrollDefaultSpriteLeft[2] = &arrows[8];
    ScrollDefaultSpriteRight[0] = &arrows[9];
    ScrollDefaultSpriteRight[1] = &arrows[10];
    ScrollDefaultSpriteRight[2] = &arrows[11];
    ScrollDefaultThumb = filler;
    ScrollDefaultThickness = thickness;
}

// ORIGINAL: 0x00588340 ?scroll_normal@@YAXXZ 0x00588340-0x005883CD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00588340.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the source-owned Scroll defaults from the normal-sized sprite objects and restore the normal thickness; retain the exact historical seed as a restoration obligation."}
// LEVER: plain sequential stores of the scaffold's own fixed-address globals (`*g_dst = (int)g_src;`) - no address arithmetic here, so the `static T *const` spelling is exactly right, no `extern` rebind needed.
void __cdecl scroll_normal() {
    set_scroll_icon_defaults(g_SCROLL_BAR_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_FILLER_ICON_SPRITES, 20);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005883D0 ?scroll_small@@YAXXZ 0x005883D0-0x0058845D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/005883d0.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the same source-owned Scroll defaults from the small sprite objects and restore the small thickness; no fixed executable sprite addresses remain in this body."}
void __cdecl scroll_small() {
    set_scroll_icon_defaults(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE, 12);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00588460 ?popups_normal@@YAXXZ 0x00588460-0x005887F2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00588460.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_normal@@YAXXZ
void __cdecl popups_normal() {
    PopupModalState = 0;
    BasePopDefaultField30A4 = 0x1000080;
    BasePopDefaultField21B0[0] = 0x1A;
    BasePopDefaultField21B0[1] = 0x3A;
    BasePopDefaultField21B0[2] = 0x32;
    BasePopDefaultField21B0[3] = 3;
    BasePop::set_def_string_font(PopupMainFont, PopupMainBoldFont, PopupMainItalicFont, 0);
    Dialog::set_def_dialog_font(PopupMainFont, PopupMainBoldFont, PopupMainItalicFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    WinStaticDefaults[7] = PopupThemeWindowColor;
    BasePopDefaultField21C0[0] = 0;
    BasePopDefaultField21C0[1] = 4;
    WinStaticDefaults[0] = 0;
    WinDynamicDefaults.rest_[1] = PopupThemeTextShadow;
    WinDynamicDefaults.rest_[0] = MessageWinTextColor & 0xFF;
    WinStaticDefaults[2] = 0;
    WinStaticDefaults[3] = 1;
    WinStaticDefaults[4] = 1;
    BasePopButtonDefaults.button_color =
        static_cast<uint8_t>(PopupThemeSurfaceColor);
    BasePop::set_def_button_color(0, -1, 1, 1);
    BasePop::set_def_button_color2(0, -1, 1, 1);
    BasePop::set_def_button_color3(0, -1, 1, 1);
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color_hyper(2, -1, 1, 1);
    int bevel_upper = PopupThemeBevelUpper;
    int bevel_lower = PopupThemeBevelLower;
    BasePopDefaultField31B0[1] = PopupThemeSurfaceColor;
    MenuDefaultSelectionColor = PopupThemeSurfaceColor;
    MenuDefaultSelectionTextColor = MessageWinTextColor;
    MenuDefaultColor3 = DiploKnownTechColor;
    WinStaticDefaults[6] = InterfaceBottomBorderThickness;
    WinStaticDefaults[1] = InterfaceCaptionHeight;
    BasePopDefaultField3104[2] = 0;
    BasePopButtonDefaults.bevel_upper = bevel_upper;
    BasePopButtonDefaults.bevel_lower = bevel_lower;
    BasePopDefaultField31B0[0] = 0;
    MenuDefaultColor2 = MenuDefaultColor1 = 0;
    WinStaticDefaults[5] = InterfaceBorderThickness;
    DialogDefaults[3] = 2;
    DialogDefaults[1] = bevel_upper;
    DialogDefaults[2] = bevel_lower;
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
    BaseButtonA94Default = 0;
    BaseButton::set_def_font(0, 0, 0);
    BaseButtonA98Default = PopupThemeButtonColor;
    PullDownDefaultTextColor = TextColourTable[38];
    PullDownDefaultBackground = TextColourTable[40];
    PullDownDefaultDisabledTextShadow = TextColourTable[42];
    PullDownDefaultTextShadow = TextColourTable[44];
    PullDownDefaultSelectedTextColor = TextColourTable[46];
    PullDownDefaultSelectedTextShadow = TextColourTable[48];
    PullDownDefaultDisabledTextColor = TextColourTable[48];
    PullDownBorderPadding = 1;
    PullDownDefaultFont = PopupMenuFont;
    DialogDefaults[23] = 0;
    Dialog::set_def_dialog_text_color(0xE0, -1, 1, 1, 0);
    DialogFieldACDefault = PopupThemeBevelUpper;
    DialogFieldB0Default = PopupThemeBevelLower;
    ListBoxDefault24 = PopupThemeBevelUpper;
    ListBoxCloseDynamicDefault = PopupThemeBevelLower;
    DialogFieldB4Default = 1;
    DialogFieldB8Default = static_cast<uint32_t>(-1);
    ListBoxDefault14 = PopupThemeSurfaceColor;
    ListBoxDefault10 = 0x41;
    EditBoxDefaultB24 = PopupThemeSurfaceColor;
    EditBoxDefaultB20 = 0;
    ScrollCloseStaticDefaults.bevel_thickness = 0;
    ScrollCloseStaticDefaults.border_color = -1;
    ScrollCloseStaticDefaults.field_A64 = 0x41;
    ScrollCloseStaticDefaults.field_A48 = 0x41;
    ScrollCloseStaticDefaults.color = 0x41;
    ScrollDefaultSpriteUp[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[0];
    ScrollDefaultSpriteUp[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[1];
    ScrollDefaultSpriteUp[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[2];
    ScrollDefaultSpriteDown[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[3];
    ScrollDefaultSpriteDown[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[4];
    ScrollDefaultSpriteDown[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[5];
    ScrollDefaultSpriteLeft[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[6];
    ScrollDefaultSpriteLeft[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[7];
    ScrollDefaultSpriteLeft[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[8];
    ScrollDefaultSpriteRight[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[9];
    ScrollDefaultSpriteRight[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[10];
    ScrollDefaultSpriteRight[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[11];
    ScrollDefaultThumb = &g_SCROLL_BAR_FILLER_ICON_SPRITES;
    ScrollDefaultThickness = 20;
    SpriteBoxDefaultField8 = PopupThemeWindowColor;
    EditBoxDefaultB28 = PopupThemeWindowColor;
    EditBoxDefaultB2C = PopupThemeTextShadow;
}

// ORIGINAL: 0x00588800 sub_588800 0x00588800-0x005888DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00588800.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// TRIED: the 3 set_def_* callees needed `static` added by hand (the emitter leaves BasePop/Dialog/BaseButton members non-static); after that, MISMATCH #3 is /O2 hoisting the three unrelated global stores (6a721c/696ed0/696ed4/696ee0/697058) past the call-argument loads for scheduling - not a source-order issue, statement order already matches the Ghidra order.
// symbol    ?popups_small@@YAXXZ
void __cdecl popups_small() {
    PopupModalState = 1;
    BasePop::set_def_string_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont, 0);
    Dialog::set_def_dialog_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont);
    BasePop::set_def_button_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont);
    BasePopDefaultField21C0[0] = 3;
    BasePopDefaultField21C0[1] = 3;
    BasePopDefaultField3104[2] = 0;
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    DialogDefaults[3] = 1;
    BaseButton::set_def_font(PopupSmallFont, 0, 0);
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
}

// ORIGINAL: 0x005888E0 ?popups_tutorial@@YAXXZ 0x005888E0-0x005889BE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005888e0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// TRIED: the 8 callee declarations (set_def_string_font, set_def_dialog_font, set_def_button_font, set_def_string_color[/2/3], set_def_font, set_def_text_color) had to be marked `static` - the scaffold omitted `static` even though the mangled names (QAA, not QAE) and the disassembly (no ecx/this setup at any call site) both say static; a non-static call is C2352. Statement order matters where the original interleaves an unrelated global store between an argument load and its push/call (e.g. `*g_006a721c = 2;` sits between loading the three Font* values and the call, and `*g_00696ee0 = 0;`/`*g_00697058 = 1;` sit between the last argument load and their calls) - hoisting the load into a local first and placing the store right after reproduces it (MISMATCH #3 -> MNEMONIC_ONLY, all 61 mnemonics match).
// TRIED: nothing further found the last 1-byte size difference (rebuilt 221B vs original 222B, no masked-byte disagreement) - likely an add-esp or push-immediate encoding choice the optimizer makes independent of source form.
// symbol    ?popups_tutorial@@YAXXZ
void __cdecl popups_tutorial() {
    PopupModalState = 2;
    BasePop::set_def_string_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont, 0);
    Dialog::set_def_dialog_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 3;
    BasePopDefaultField21C0[1] = 3;
    BasePopDefaultField3104[2] = 0;
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    DialogDefaults[3] = 1;
    BaseButton::set_def_font(PopupSmallFont, 0, 0);
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005889C0 ?popups_medium@@YAXXZ 0x005889C0-0x00588A2A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005889c0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_medium@@YAXXZ
void __cdecl popups_medium() {
    PopupModalState = 3;
    BasePop::set_def_string_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont, 0);
    Dialog::set_def_dialog_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 2;
    BasePopDefaultField21C0[1] = 2;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00588A30 sub_588a30 0x00588A30-0x00588A9A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00588a30.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_interlude_medium@@YAXXZ
void __cdecl popups_interlude_medium() {
    PopupModalState = 4;
    BasePop::set_def_string_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont, 0);
    Dialog::set_def_dialog_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 2;
    BasePopDefaultField21C0[1] = 2;
}


// ORIGINAL: 0x00588AA0 ?config_popups@@YAXXZ 0x00588AA0-0x00588F8E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00588aa0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?config_popups@@YAXXZ
// Current reconstructed ownership keeps this ordinary body beside find_font.
// Shared visibility is a compiler-context hypothesis, not historical TU proof.
void __cdecl config_popups() {
    IfaceScreenWidth = WinScreenWidth;
    IfaceScreenHeight = WinScreenHeight;
    StringTemp[0] = 0;
    strcat(StringTemp, ScriptPopupFile);
    strcat(StringTemp, ".txt");
    BasePop::set_def_file(StringTemp);
    int main_size = prefs_get("MainFontSize", 16, FALSE);
    int interlude_size = prefs_get("InterludeFontSize", 16, FALSE);
    FastUnitAnim = prefs_get("FastUnitAnim", 0, FALSE);
    prefs_put("FastUnitAnim", FastUnitAnim, FALSE);
    if (!FastUnitAnim && prefs_get("SmoothUnitAnim", 0, FALSE)) FastUnitAnim = -1;
    prefs_put("SmoothUnitAnim", FastUnitAnim < 0, FALSE);
    prefs_put("MainFontSize", main_size, FALSE);
    prefs_put("InterludeFontSize", interlude_size, FALSE);
    if (WinScreenWidth == 800) { main_size -= 2; interlude_size -= 2; }
    for (int size = 0; size < 12; ++size) {
        for (int style = 0; style < 4; ++style) {
            const int height = FontSizeTable[size];
            FontTable[size * 4 + style].init(height > 12 ? DefaultLargeFontFace : DefaultFontFace,
                height, (height > 12 ? 1 : 0) | FontStyleTable[style]);
        }
    }
    // The nearest-size searches are repeated inline in the image. find_font
    // has the same strict tie rule and 9999 threshold, and returns these
    // same owned FontTable elements rather than creating another font run.
    PopupMainFont = find_font(main_size, 0);
    PopupMainItalicFont = find_font(main_size, 2);
    PopupMainBoldFont = find_font(main_size, 1);
    InterludeNormalFont = find_font(interlude_size, 0);
    InterludeThirdFont = find_font(interlude_size, 2);
    InterludeSecondFont = find_font(interlude_size, 1);
    PopupMediumFont = find_font(16, 0);
    PopupMediumItalicFont = find_font(16, 2);
    PopupMediumBoldFont = find_font(16, 1);
    PopupLargeFont = find_font(16, 0);
    PopupLargeItalicFont = find_font(16, 2);
    PopupLargeBoldFont = find_font(16, 1);
    if (WinScreenWidth == 800) {
        PopupMenuFont = find_font(12, 0);
        PopupMenuItalicFont = find_font(12, 2);
        PopupMenuBoldFont = find_font(12, 1);
    } else {
        PopupMenuFont = find_font(16, 0);
        PopupMenuItalicFont = find_font(16, 2);
        PopupMenuBoldFont = find_font(16, 1);
    }
    PopupSmallFont = find_font(12, 0);
    PopupSmallItalicFont = find_font(12, 2);
    PopupSmallBoldFont = find_font(12, 1);
    WinBubbleFont = PopupSmallFont;
    PopupDefaultWindowFont = find_font(12, 1);
    PopupMapLabelFont = find_font(10, 1);
    Menu::set_def_font(PopupMenuFont);
    PullDownDefaultFont = PopupMenuFont;
    WinDynamicDefaults.default_font_ = PopupDefaultWindowFont;
    popups_normal();
}

// ORIGINAL: 0x00588F90 ?alien_start@@YAXXZ 0x00588F90-0x0058917C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:6e78872480ef8b4a486516e322f23e9f8652665d:src/recovered/units/00588f90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover first-turn alien faction suppression through real Player filename/rules, lock masks, descending Base/Veh lifetimes and faction records."}
void __cdecl alien_start() {
    if (GameDataStateStorage.fields_.turn_current_num_ != 0 || ExpansionEnabled == 0) {
        return;
    }

    const int local_faction = ConsoleGlobal.faction_id_;
    if ((Players[local_faction].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }

    StringTemp[0] = 0;
    strcat(StringTemp, Players[local_faction].filename);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "FUNGBOY") == 0) {
        return;
    }

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        if (is_human(faction_id) || IsMultiplayerPBEM != 0 ||
            IsMultiplayerNet != 0) {
            continue;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, Players[faction_id].filename);
        CharUpperA(StringTemp);
        if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0 &&
            strcmp(StringTemp, "FUNGBOY") != 0) {
            continue;
        }

        GameDataStateStorage.fields_.lock_masks_.human_mask_ &= ~(1 << faction_id);
        GameDataStateStorage.fields_.lock_masks_.alive_mask_ &= ~(1 << faction_id);

        for (int base_id = GameDataStateStorage.fields_.base_current_count_ - 1; base_id >= 0; --base_id) {
            if (Bases[base_id].faction_id_current == faction_id) {
                base_kill(base_id);
            }
        }

        // veh_kill compacts the vehicle table; the original checks the live
        // count before dereferencing each descending id.
        for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; --veh_id) {
            if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_ &&
                Vehs[veh_id].faction_id == faction_id) {
                veh_kill(veh_id);
            }
        }

        PlayersData[faction_id].current_num_bases = 0;
        PlayersData[faction_id].flags = 0xFFFFFFFFU;
        PlayersData[faction_id].diff_level = -1;
    }
}

/*
Purpose: Handle creation of pop-up message on Planetfall.
// ORIGINAL: 0x00589180 ?planetfall@@YAXH@Z 0x00589180-0x00589329 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00589180-0x00589329 calls parse_says with slots 2, 3, 0, 1 in that order; the four text pointers are Player.title_leader (+0x4C), name_leader (+0x34), adj_name_faction (+0x304), and noun_faction (+0x2E4).", "Pinned default writes are ordered: PluralityDefault=0 before slot 2; GenderDefault=is_leader_female before slot 2; then GenderDefault and PluralityDefault=0 before slot 3; slot 0 inherits those defaults; finally noun_gender then is_noun_plural before slot 1. general.h binds those defaults to 0x009BBFEC/0x009BBFF0.", "Pinned Player records begin at 0x00946A50 with 0x59C stride; rule_flags is +0x508 and alien bit 0x80 is tested directly. The image uppercases the copied filename via CharUpperA then calls the pinned _strcmp entry at 0x00645660 for exact USURPER/FUNGBOY comparisons; this preserves the raw flag test and exact string behavior, not is_alien_faction's expansion gate or case-insensitive comparison.", "Pinned UI key construction clears and appends PLANETFALLX for an alien USURPER, PLANETFALLF for exact FUNGBOY, otherwise PLANETFALL. It uses shared StringTemp at 0x009B86A0. Nonzero TurnCurrentNum at 0x009A64D4 gates parse_num(0, turn), the 64-slot project scan, and a final appended '2' suffix before X_pop(StringTemp, 0); zero turn skips all three.", "The project scan reads the 64 dword SecretProject array at 0x009A6514, accepts nonnegative base owners matching faction_id, and calls the int-input parse_say(4, Facility[70+project].name, -1, -1) for each match. The typed sources are GameDataStateWindowFields::secret_project_, Base.faction_id_current, and RulesFacility.name; the generated body uses has_project without adding bounds or error-path changes.", "The previous TRIED finding remains verbatim as history; this rationale records the pinned call/global/UI mapping only. Existing ORIGINAL identity and complete 425-byte span remain; no exactness/semantic claim is granted."], "reason": "Recover Planetfall's ordered parser defaults, shared scratch-buffer popup key, current-turn project context and source-owned faction/project state from the pinned body."}
// size      425 bytes
// prototype void (__cdecl ?planetfall@@YAXH@Z)(int factionID)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF310 0x00625E30 0x00625E50 0x00625EC0 0x00645470 0x00645660
// indirect  0x0058923D
// TRIED: not resolved this pass. The image's FOUR `parse_says`
//        (0x00625EC0) call sites write the gender/plurality arguments to
//        FIXED GLOBALS (0x9bbfec/0x9bbff0) individually before each call
//        rather than through `parse_set`, in an ORDER that does not match
//        this body's own call order (id 2, then 3, then 0, then 1 - not
//        0, 2, 3, 1), and the calls list also names 0x00625E30 (parse_num)
//        AND 0x00625E50 (parse_say, the int-input overload) - this body
//        calls parse_num once and never parse_say, so one of its calls is
//        the wrong callee. Left at 11/125 for a pass that can trace which
//        source line the int-input `parse_say` belongs to.
Return Value: n/a
Status: Complete
*/
void __cdecl planetfall(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(2, Players[faction_id].title_leader, -1, -1);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(3, Players[faction_id].name_leader, -1, -1);
    parse_says(0, Players[faction_id].adj_name_faction, -1, -1);

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(1, Players[faction_id].noun_faction, -1, -1);

    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction_id].filename);
    CharUpperA(StringTemp);
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0
        && strcmp(StringTemp, "USURPER") == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALLX");
    } else if (strcmp(StringTemp, "FUNGBOY") == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALLF");
    } else {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALL");
    }

    const int turn = GameDataStateStorage.fields_.turn_current_num_;
    if (turn != 0) {
        parse_num(0, turn);
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            // Open-coded has_project: the image materializes the owner test.
            int base_id = GameDataStateStorage.fields_.secret_project_.project_base[project];
            BOOL owned;
            if (base_id < 0) {
                owned = false;
            } else {
                owned = (Bases[base_id].faction_id_current == faction_id);
            }
            if (owned) {
                parse_say(4, Facility[FacilitySPStart + project].name,
                          -1, -1);
            }
        }
        strcat(StringTemp, "2");
    }

    X_pop(StringTemp, 0);
}

// ORIGINAL: 0x00589330 ?time_controls_dialog@@YAXPAUPopup@@@Z 0x00589330-0x0058954A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:time-control-calculation-and-dialog", "historical seed:src/recovered/units/00589330.cpp", "Pinned original native instructions and existing typed source layout; seed aliases independently reconciled."], "reason": "Recover all six original time-control rows through Popup-owned Dialogs, StringTable and existing label_get; preserve numeric field order and punctuation."}
// symbol    ?time_controls_dialog@@YAXPAVPopup@@@Z
void __cdecl time_controls_dialog(Popup *popup) {
    popup->start(PopupStartCaption, "TIMECONTROLS", -1, nullptr, 0x40, nullptr);
    for (int index = 0; index < MaxTimeControlNum; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, table_text(TimeControl[index].name));
        strcat(StringTemp, "|");
        say_open_paren();

        if (index != 0) {
            say_num(TimeControl[index].turn);
            space();
            strcat(StringTemp, label_get(0x468 / 4));
            space();
            strcat(StringTemp, label_get(0x1094 / 4));
            space();

            say_num(TimeControl[index].base);
            space();
            strcat(StringTemp, label_get(0x46C / 4));
            strcat(StringTemp, " + ");

            say_num(TimeControl[index].unit);
            space();
            strcat(StringTemp, label_get(0x470 / 4));
        } else {
            strcat(StringTemp, label_get(0x474 / 4));
        }
        say_close_paren();
        popup->dialogs().item(StringTemp, index);
    }
}

// ORIGINAL: 0x00589550 j_GraphicWin::close 0x00589550-0x00589555 FILE BYTE_EXACT
// symbol    ?j_close@GraphicWin@@QAEXXZ
// LEVER: implicit-thiscall member forwarding; explicit __thiscall on a free fn is C4234
// working copy - scaffold materialised by --work
// size      5 bytes
// prototype 
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)


// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: j_GraphicWin::close  at 0x00589550  (5 bytes)
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
// GraphicWin is graphicwin.h's real class, which general.cpp already includes -
// the thunk is a member of it, not of a local shim.
void GraphicWin::j_close() {
    close();
}

// ORIGINAL: 0x00589560 sub_589560 0x00589560-0x00589748 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589560-0x00589748 reads receiver fields and Popup at +0xA3C, builds the faction profile content and finishes through the update slot", "source:src/faction.h Player formal_name_faction is +0x384; source:src/game.cpp owns InterfaceBoxStyles[51], populated by load_box_sprites; 0x78D690 is entry 9 at 0x28 stride"], "reason": "Carry the complete seed identity and body span into the typed show_profile member, preserving fresh owner-field reads across callback-capable Popup and SpriteBox operations. The class role is provisional and does not claim original identity or allocation registration."}
// symbol    ?show_profile@FactionProfileWin@@QAEXXZ
void FactionProfileWin::show_profile() {
  RECT local_bounds = clip_rect();
  box_sprite(&local_bounds, &InterfaceBoxStyles[9]);

  set_caption(Players[faction_id_].formal_name_faction);
  StringTemp[0] = 0;
  if (profile_mode_ != 0) {
    strcat(StringTemp, "DATALINKS1");
  } else {
    strcat(StringTemp, "BLURB");
  }
  FactionDiplomacyPortraits[faction_id_]->draw(
      static_cast<Buffer *>(this),
      FactionDiplomacyPortraits[faction_id_]->transparent_index(), 0, 2);

  const int popup_width = width_ - 0xA9;
  popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x400,
               static_cast<GraphicWin *>(this));
  if (profile_mode_ != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, "DATALINKS2");
    popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x80,
                 static_cast<GraphicWin *>(this));
  }

  Popup *const popup = &popup_;
  Sprite *const popup_portrait = FactionDiplomacyPortraits[faction_id_];
  if (!popup->picture_enabled()) {
    popup->set_caller_picture(popup_portrait);
  } else {
    popup->set_caller_picture(0);
  }
  popup->dialogs().sprite(popup_portrait, 0, 0);
  set_caption(Players[faction_id_].formal_name_faction);
  popup_.set_string_font(PopupMenuFont, PopupMenuBoldFont,
                         PopupMenuItalicFont, 0);
  popup_.set_loc(0xA2, 2);
  popup_.set_width(popup_width);
  popup_.exec(0, 0);

  on_update();
}

// ORIGINAL: 0x00589750 sub_589750 0x00589750-0x00589760 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00589750-0x00589760 stores its argument at receiver+0xA34", "source:src/factionprofilewin.h RECOVERY_MEMBER faction_id_ at +0xA34, anchored by 0x00589760", "pinned/current owner:0x00589760 initialize writes faction_id_ and profile_mode_ at +0xA34/+0xA38"], "reason": "Carry the one-word receiver store into the existing FactionProfileWin member that owns the anchored faction_id_ field; class role and method spelling remain provisional."}
// symbol    ?set_faction_id@FactionProfileWin@@QAEXH@Z
// kind      game
void FactionProfileWin::set_faction_id(int faction_id) {
  faction_id_ = faction_id;
}

// ORIGINAL: 0x00589760 sub_589760 0x00589760-0x0058986A;0x00660240-0x00660255 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589760-0x0058986A stores the arguments at receiver+0xA34/+0xA38, frame fields and RECT, then calls GraphicWin initialization/fill and the slot-1 show dispatch", "image:0x00660240-0x00660255 is the local Buffer unwind funclet and remains the stack lifetime in the source body"], "reason": "Carry the complete seed identity and both image spans into the typed initialize member. FactionProfileWin remains a provisional role name; original class identity and allocation/caller registration are unproved."}
// symbol    ?initialize@FactionProfileWin@@QAEXHH@Z
void FactionProfileWin::initialize(int faction_id, int profile_mode) {
  Buffer local_buffer;

  faction_id_ = faction_id;
  profile_mode_ = profile_mode;
  field_A14_ = 5;
  field_A18_ = 0x2D;
  width_ = 0x226;
  height_ = 0x12C;
  if (IfaceScreenWidth >= 1000) {
    width_ = 0x2C0;
    height_ = 0x180;
  }

  set_rectangle_dimensions(&bounds_, 5, 0x2D, width_, height_);
  init(&bounds_, "...", 0x1400000, OpeningWindow, 0,
       0);
  fill(9);
  show(0);
}


// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (dispatcher at 0x00644EEB, table at 0x00682000..0x006826CC).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Was `Font g_FONTS[48]` beside the `Font *const FontTable = (Font
// *)0x0093FC58` binding - the array is the binding's storage, so the object
// now carries the public name and find_font's `FontTable + N` indexes it
// directly. See interface_fonts.h.
// Original startup 0x005882A0 and teardown 0x005882D0 both pass this
// storage, count 48 and stride 0x28 to the Font vector lifetime helpers.
// That constructed array run establishes [0x0093FC58, 0x009403D8),
// independently of the next global or this reconstruction's sizeof.
// RECOVERY_OBJECT: v2 0x0093FC58 0x009403D8 ?FontTable@@3PAVFont@@A 0x005882A0
Font FontTable[48];  // 0x0093FC58, 0x28 stride

// 0x0068F220, twelve point sizes find_font matches against. The values are
// the image's own .data dwords (image_data.py, 0x28 0x24 0x20 0x18 0x16 0x14
// 0x12 0x10 0x0e 0x0c 0x0a 0x08), largest first.
// RECOVERY_OBJECT: v1 0x0068F220 ?FontSizeTable@@3QBHB
const int FontSizeTable[FontSizeTableCount] = {
    0x28, 0x24, 0x20, 0x18, 0x16, 0x14, 0x12, 0x10, 0x0e, 0x0c, 0x0a, 0x08};

// Original 0x0068F250..0x0068F260: config_popups indexes all four styles.
// RECOVERY_OBJECT: v1 0x0068F250 ?FontStyleTable@@3QBHB
const int FontStyleTable[4] = {0, 1, 2, 4};


void __cdecl world_build();

// New-game, menu and desktop/system lifetime unit. One original TU: the
// image keeps 0x00589870..0x0058FD6B as one code run and 0x0068F5C0..
// 0x0068FAAC as one literal run, and the bodies expand each other (see the
// note above desktop_update).
// Independent DWORD after FastUnitAnim; native map/size callers reset/set it.
// Provisional descriptive name; complete lifecycle is not established.
// RECOVERY_OBJECT: v2 0x00945820 0x00945824 ?MapSetupStatus@@3HA 0x0058D6B0
int MapSetupStatus;

// Shared opening-window owner. The pinned PE places this four-byte pointer
// in the zero-filled .data virtual tail; init_opening publishes the complete
// GraphicWin pointer. All compiled consumers use this source storage.
// RECOVERY_OBJECT: v2 0x00945824 0x00945828 ?OpeningWindow@@3PAVGraphicWin@@A 0x00589B60
GraphicWin *OpeningWindow = 0;

char PronounBuffer[256];  // 0x00945838 - see game.h for the naming evidence


// Newgame-owned shared faction preview and callback state.
// Generated initializer and destructor bindings below identify this global's startup lifetime.
// RECOVERY_OBJECT: v2 0x009403E0 0x0094575C ?ConfigFactionPreview@@3VPopup@@A 0x00589870
// ORIGINAL: 0x00589870 sub_589870 0x00589870-0x00589886 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original initializer 0x00589870-0x00589886 constructs the Popup stored at 0x009403E0 and registers original teardown 0x00589890.", "Source owner: newgame.cpp declares and uses the genuine Popup ConfigFactionPreview at 0x009403E0; the global, not a fixed-address reinterpretation, owns this lifetime.", "The emitted initializer _$E21 references ConfigFactionPreview and Popup's constructor, then registers _$E19 with atexit; _$E19 references the same global and Popup destructor.", "The archived seed's prior general.cpp placement and compiler-spelling trials are historical; the complete original mapping and claim remain live here for restoration measurement."], "reason": "Bind the original Preview startup constructor to the real global Popup in newgame.cpp and the compiler-emitted constructor/atexit lifetime pair. The prior fixed-address experiment is superseded by this typed owner."}
// symbol    _$E26
// TRIED: homed into general.cpp (its real TU) 2026-09-03. Two source forms were
// measured there and both are archived here verbatim:
//   * as written (placement new + _atexit) it does not compile in the TU -
//     libc.lib has no _atexit, and the placement new buys VC6's null guard;
//   * retyped to a qualified constructor call plus plain atexit it compiles and
//     measures 5 of 6 instructions, the one divergence being VC6 cleaning the
//     atexit push with 'add esp,4' where the image has 'pop ecx'. general.cpp
//     compiles under /Oy- and every cleanup spelling tried kept 'add esp,4';
//     'pop ecx' is the /Oy form. Reverted so the BYTE_EXACT claim stays live
//     here rather than reading REGRESSED there. The qualified-ctor + atexit
//     spelling is the right one; it needs a TU on the /Oy flag set.
Popup ConfigFactionPreview;

// ORIGINAL: 0x00589890 sub_589890 0x00589890-0x0058989A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original callback 0x00589890-0x0058989A destroys the Popup at 0x009403E0 registered by 0x00589870.", "Source owner: ConfigFactionPreview is the genuine Popup global declared and consumed by newgame.cpp; its lifetime is not independently owned by general.cpp.", "The emitted helper _$E19 references ConfigFactionPreview and Popup's destructor and is the callback registered by the same translation unit's _$E21 global initializer.", "The original identity, complete span and historical claim are preserved on the source-owned FILE mapping; the retired wrapper is not treated as independent recovered behavior."], "reason": "Bind the original ConfigFactionPreview teardown identity to the compiler-generated destructor callback of newgame.cpp's real Popup global, replacing the fixed-address wrapper in general.cpp."}
// symbol    _$E24
// RECOVERY_OBJECT: v2 0x00945818 0x0094581C ?ConfigFactionDetailMode@@3HA 0x005898A0
int ConfigFactionDetailMode;

// The image uses this zero-filled byte string only as initial prompt text.
// The adjacent PronounBuffer is independently owned; no overlay is introduced.
// RECOVERY_OBJECT: v1 0x00945834 ?ConfigRandomLeaderInitial@@3PADA
char ConfigRandomLeaderInitial[4];

// ORIGINAL: 0x005898A0 ?thumb_routine@@YAXH@Z 0x005898A0-0x00589A24 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/005898a0.cpp", "pinned:5898A0..589A24; preview9403E0; detail945818; diplomacy art78EE48 with65C faction stride", "source:Popup537C and PickWin redraw/ListBox cdecl(int) callback contract; allocation/lifetime selection still requires native qualification"], "reason": "Typed shared faction preview callback with real Popup/SpriteBox/FactionArt owners and genuine global storage; no executable-address aliases or manual vtables."}
// RECOVERY_LITERAL: v1 0x0068F5C0 0x0068F5CB $SG65309 0x005898A0
// RECOVERY_LITERAL: v1 0x0068F5CC 0x0068F5D2 $SG65311 0x005898A0
// RECOVERY_LITERAL: v1 0x0068F5D4 0x0068F5DF $SG65313 0x005898A0
void __cdecl thumb_routine(int faction) {
    if (ExpansionEnabled) {
        PickWinGlobal.GraphicWin::redraw();
// RECOVERY_LITERAL: v1 0x0068F5B8 0x0068F5C0 $SG65307 0x005898A0
        if (strcmp(Players[faction].filename, "JENN282") == 0) {
            ConfigFactionPreview.close();
            return;
        }
    }
    StringTemp[0] = 0;
    if (ConfigFactionDetailMode)
        strcat(StringTemp, "DATALINKS1");
    else
        strcat(StringTemp, "BLURB");
    ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x10500, 0);
    if (ConfigFactionDetailMode) {
        StringTemp[0] = 0;
        strcat(StringTemp, "DATALINKS2");
        ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x80, 0);
    }
    Sprite *const diplomacy = &FactionsArt[faction].diplomacy;
    ConfigFactionPreview.set_caller_picture(ConfigFactionPreview.picture_enabled()
        ? 0 : diplomacy);
    ConfigFactionPreview.dialogs().SpriteBox::sprite(diplomacy, 0, 0);
    ConfigFactionPreview.Win::set_caption(Players[faction].formal_name_faction);
    ConfigFactionPreview.set_string_font(PopupMenuFont, PopupMenuBoldFont, PopupMenuItalicFont, 0);
    PopupModalActive = 1;
    ConfigFactionPreview.set_loc(5, 0x136);
    ConfigFactionPreview.BasePop::exec(0, 0);
    PopupModalActive = 0;
}

/*
Purpose: Swap the opening logo the desktop shows: load the PCX named a1 into
         a local Buffer, blit it into the opening GraphicWin's Buffer base
         from the decoded DIB, then call the window's virtual on_update
         (Win slot 63) so it repaints.
         Returns 7 when no opening window is up.

// ORIGINAL: 0x00589A30 ?change_opening@@YAHPAD@Z 0x00589A30-0x00589B1D;0x00660255-0x0066026A BYTE_EXACT
// TRIED (seed): reproducing the SEH frame/unwind funclet for the local
//            `Buffer buf` explicitly - a real local of non-trivial type
//            (ctor/dtor declared, undefined) makes MSVC generate its own
//            construction/destruction and unwind automatically; landed
//            relying on that instead of hand-writing the funclet.
// size      258 bytes
// prototype int (__cdecl ?change_opening@@YAHPAD@Z)(int8* a1)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005D9610
Return Value: 0 when the opening swapped, 7 when there is no opening window
The second ORIGINAL span is the local Buffer's unwind funclet and scope
descriptor. OpeningWindow still reaches original storage; its producer and
lifetime consumers need a coordinated migration. The window is reloaded after
load_pcx and before on_update, as in the image. The retained seed's SEH attempt
relies on the compiler-generated unwind for the real local Buffer rather than
hand-writing a funclet.
*/
int __cdecl change_opening(char *filename) {
    Buffer buf;
    if (OpeningWindow == 0) {
        return 7;
    }
    buf.load_pcx(filename, 0, 10, 0xec);
    GraphicWin *window = OpeningWindow;
    // The dimensions use GraphicWin itself after the load callback; only the
    // Buffer copy argument conversion is null-conditional.
    buf.copy(window, 0, 0, buf.pixel_width(), buf.pixel_height(), 0, 0,
             window->pixel_width(), window->pixel_height());
    OpeningWindow->on_update();
    return 0;
}



/*
Purpose: Publish the Console's Win base for popup parenting, then destroy
         the owned opening window and clear its pointer.
// ORIGINAL: 0x00589B20 ?close_opening@@YAXXZ 0x00589B20-0x00589B57 FILE BYTE_EXACT
// size      55 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00589B4A
// The original converts Console through MapWin's virtual GraphicWin base.
// At 0x00589B4A it calls the opening object's scalar deleting destructor
// with flag one, not on_dialog_focus. GraphicWin's original primary table
// at 0x0066FC50 selects 0x005D7140, which destroys and frees the allocation.
Return Value: n/a
*/
void __cdecl close_opening() {
    Win *console_window = &ConsoleGlobal;
    GraphicWin *opening = OpeningWindow;
    // Publish the ordinary C++ conversion to the fallback parent window.
    BasePopDefaultField322C = console_window;
    if (opening != 0) {
        delete opening;
        OpeningWindow = 0;
    }
}


// WORKAROUND(inline): dead spend for init_opening's top_menu expansions;
// a constant argument folds it to no code.
static inline int init_opening_spend(int x) {
    if (x == 1) x = 3;
    if (x == 2) x = 5;
    if (x == 4) x = 7;
    return x;
}


// ORIGINAL: 0x00589B60 ?init_opening@@YAHPBD@Z 0x00589B60-0x00589D29;0x0066026A-0x0066028A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:opening-window-lifetime-owner", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589B60..589D29;66026A..66028A", "source:src/unrecovered/00589b60.cpp complete original identity/spans and seed lessons retained; Buffer::pixel_height already negates the top-down DIB height."], "reason": "Recover opening-window construction, loading, presentation and popup-parent publication through genuine GraphicWin and Buffer lifetimes. Each callback-sensitive shared-slot reload is independent; no added null guard or failure-path behavior change."}
// A header inline (MEASURED in game.h): top_menu expands it four times with
// change_opening and close_opening left as calls; this pick-any copy has no
// caller in the image.
inline int __cdecl init_opening(const char *filename) {
    // WORKAROUND(inline): top_menu calls close_opening out of line at all
    // four expansions; two dead branchy expansions here supply the depth-2
    // spend that refuses it (no natural spelling found yet).
    init_opening_spend(0);
    init_opening_spend(1);
    if (OpeningWindow == 0) {
        GraphicWin *const window = new GraphicWin();
        OpeningWindow = window;
        if (window == 0) {
            return 4;
        }

        const int height = WinScreenHeight;
        const int width = WinScreenWidth;
        const int result = window->init(0, 0, width, height,
                                        0, 0x400000, 0, 0,
                                        0);
        if (result != 0) {
            close_opening();
            return result;
        }
    }

    OpeningWindow->fill(0);
    change_opening(const_cast<char *>(filename));
    g_PALETTE1.set();
    OpeningWindow->show(0);
    do_all_draws();
    BasePopDefaultField322C = OpeningWindow;
    return 0;
}


// ORIGINAL: 0x00589D30 ?config_game@@YAHH@Z 0x00589D30-0x0058C29D;0x0066028A-0x006605A5
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/00589d30.cpp; existing original identity and useful lessons preserved", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589D30..58C29D and66028A..6605A5", "pinned:5898A0..589A24 typed cdecl(int) preview callback; ListBox dispatch retains current declaration", "source:BasePop +A44 value setter,+3100 input poll state; Popup picture/dialog/font and PickWin owned ListBox", "pinned:58A9BA passes zero-filled945834 initial string, allocation extent remains unknown", "source:recovery/tests/test_config_game_actual_source.py actual installed body, bounded API fixture and six compile-success runtime damage controls", "source:589870 initializer and589890 teardown identities now bind to the generated ConfigFactionPreview lifetime in this owner"], "reason": "Complete new-game rules, faction selection, custom naming, PBEM/network and world/setup flow with genuine Popup/SetupWin automatic local lifetimes and a source-owned shared preview. Original undefined and unchecked paths and historical main/cleanup spans remain restoration obligations; no byte or runtime acceptance is granted."}
// TRIED: mismatch #0 - only the entry (a1&1 defaults shortcut) and the first two do_menu() results (0=default_rules toggle, 1=difficulty rules-flag encode) are transcribed from the 2618-instruction original; the disassembly shows several more do_menu() results driving a PickWin faction picker, Wave/Sound playback, EditGroup name entry and a time_controls_dialog that are NOT modelled - this body loops back to do_menu() for any other result rather than fabricating their bit-flag encodings.
// WORKAROUND(inline): the 17 rule checkboxes go through one set-or-clear
// helper. The expansions compile to the same rows as the written-out
// if/else pairs (0x0058A29C-0x0058A393) and their /Ob2 spend makes both
// early `return 1` teardowns call ~FlatButton out of line as the image does
// (0x00589FC0, 0x0058A142); the helper's original name is unknown.
static inline void config_rule_check(int &checks, int bit, int on) {
    if (on) checks |= bit; else checks &= ~bit;
}

// WORKAROUND(inline): config_game_site_pad is an empty inline call site, not
// recovered code. /Ob2 gives each scope-exit teardown a share of the room
// left, divided by 1 + the inline sites after it in the same body
// (MATCHING_GUIDE 2.8 rule 8). The image calls ~StringList 0x004066C0 inside
// the expanded ~SetupWin at both early `return 1` teardowns (0x00589F1C,
// 0x0058A09E); ours expanded it there until 48..80 more inline sites follow
// the second return (46 flips only the first; dead charged spend at the top,
// 1..64 probes, moves nothing). Position after that return does not matter.
// 56 sites (the window's middle) give the image's unwind map and named call
// multiset. The original's extra sites are code-invisible free inlines that
// our spelling lacks (label_get at its 20 label sites counts as the sites
// but changes the label loads, so it is not them); the StringList and
// SetupWin declarations are not the cause.
static inline void config_game_site_pad() {}

// Full config_game candidate. Original identities/claims remain on the owning
// TU; this ignored include is a body proposal, not a measurement or annotation.
int __cdecl config_game(int mode) {
    Popup popup;
    SetupWin setup;
    int selected;       // [ebp-0x14]
    int human_mask;     // [ebp-0x10]
    int time_choice;    // register only (0x0058A5B2 -> 0x0058B98A)
    char gender_text[0x50];     // [ebp-0xA8]
    char first_special[0x18];   // [ebp-0x58]

    if (mode & 1) {
        GameDataStateStorage.fields_.diff_level_current_ = DefaultPrefs.difficulty;
        GameDataStateStorage.fields_.game_rules_ = AlphaIniPrefs.rules;
        selected = DefaultPrefs.faction_id;
        goto parse_factions;
    }

difficulty_menu:
    setup.Win::hide();
    setup.Win::show(0);
    if (!(GameDataStateStorage.fields_.game_rules_ & 0x01000000)) {
        popup.start("DIFFICULTY", 0x40);
        {
            for (int i = 0; i < 6; ++i) {
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(Difficulty[i]));
                if (i == 0) {
                    space();
                    say_open_paren();
                    strcat(StringTemp, StringTable.get(Labels.strings_ptr[113]));
                    say_close_paren();
                } else if (i == 5) {
                    space();
                    say_open_paren();
                    strcat(StringTemp, StringTable.get(Labels.strings_ptr[114]));
                    say_close_paren();
                }
                popup.item(StringTemp, i);
            }
        }
        popup.set_value(DefaultPrefs.difficulty);
        int choice = setup.do_menu(&popup, 1, 0);
        if (choice < 0) {
            return 1;
        }
        DefaultPrefs.difficulty = choice;
        GameDataStateStorage.fields_.diff_level_current_ = choice;
    }

rules_menu:
    human_mask = 0;
    if (mode == 2) goto select_faction;
    setup.Win::hide();
    setup.Win::show(0);
    popup.start("USERULES", 0x40);
    popup.set_value(AlphaIniPrefs.customize);
    {
        int choice = setup.do_menu(&popup, 1, 0);
        if (choice < 0) {
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x01000000)) goto difficulty_menu;
            return 1;
        }
        AlphaIniPrefs.customize = choice;
        // WORKAROUND(inline): 56 site pads, see config_game_site_pad.
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();
        config_game_site_pad(); config_game_site_pad(); config_game_site_pad(); config_game_site_pad();

        if (choice == 0) {
            GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
            GameDataStateStorage.fields_.game_rules_ |= default_rules();
            goto select_faction;
        }
        if (choice == 1) {
            GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
            GameDataStateStorage.fields_.game_rules_ =
                GameDataStateStorage.fields_.diff_level_current_
                    ? GameDataStateStorage.fields_.game_rules_ | (AlphaIniPrefs.rules & 0xFFFF)
                    : GameDataStateStorage.fields_.game_rules_ | ((AlphaIniPrefs.rules & 0xFDFF) | 0x200);
            GameDataStateStorage.fields_.game_state_ =
                (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFF) | AlphaIniPrefs.semaphore;
            goto select_faction;
        }
    }
    human_mask = 0;

custom_rules:
    setup.Win::hide();
    setup.Win::show(0);
    GameDataStateStorage.fields_.game_rules_ =
        (GameDataStateStorage.fields_.game_rules_ & 0xFFFF0000) |
        (GameDataStateStorage.fields_.diff_level_current_ ? AlphaIniPrefs.rules & 0xFFFF
            : (AlphaIniPrefs.rules & 0xFDFF) | 0x200);
    GameDataStateStorage.fields_.game_state_ =
        (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFF) | AlphaIniPrefs.semaphore;
    if (IsMultiplayerNet) GameDataStateStorage.fields_.game_rules_ |= 0x200;
    {
        // Rules checklist seed (0x0058A29C-0x0058A393): one checkbox bit per
        // rule, built in a register and published before the RULES popup.
        int checks = (GameDataStateStorage.fields_.game_rules_ & 0x800) != 0;
        config_rule_check(checks, 0x2, GameDataStateStorage.fields_.game_rules_ & 0x2);
        config_rule_check(checks, 0x4, GameDataStateStorage.fields_.game_rules_ & 0x8);
        config_rule_check(checks, 0x8, GameDataStateStorage.fields_.game_rules_ & 0x4);
        config_rule_check(checks, 0x10, GameDataStateStorage.fields_.game_rules_ & 0x1000);
        config_rule_check(checks, 0x20, GameDataStateStorage.fields_.game_rules_ & 0x1);
        config_rule_check(checks, 0x40, GameDataStateStorage.fields_.game_rules_ & 0x10);
        config_rule_check(checks, 0x80, GameDataStateStorage.fields_.game_rules_ & 0x20);
        config_rule_check(checks, 0x100, GameDataStateStorage.fields_.game_rules_ & 0x4000);
        config_rule_check(checks, 0x200, GameDataStateStorage.fields_.game_rules_ & 0x200);
        config_rule_check(checks, 0x400, GameDataStateStorage.fields_.game_rules_ & 0x40);
        config_rule_check(checks, 0x800, ~GameDataStateStorage.fields_.game_rules_ & 0x100);
        config_rule_check(checks, 0x1000, GameDataStateStorage.fields_.game_rules_ & 0x2000);
        config_rule_check(checks, 0x2000, GameDataStateStorage.fields_.game_rules_ & 0x8000);
        config_rule_check(checks, 0x4000, GameDataStateStorage.fields_.game_rules_ & 0x80);
        config_rule_check(checks, 0x8000, GameDataStateStorage.fields_.game_rules_ & 0x400);
        config_rule_check(checks, 0x10000, GameDataStateStorage.fields_.game_state_ & 0x00800000);
        config_rule_check(checks, 0x20000, GameDataStateStorage.fields_.game_state_ & 0x01000000);
        PopupWriteCheckState = checks;
    }
    if (x_pop_short("RULES", 0x41, 0) < 0) goto rules_menu;
    GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
    GameDataStateStorage.fields_.game_state_ &= 0xFE7FFFFF;
    if (ConsoleGroupAction) {
        GameDataStateStorage.fields_.game_rules_ |= default_rules();
        AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
        AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000;
        goto custom_rules;
    }
    if (PopupWriteCheckState & 0x1) GameDataStateStorage.fields_.game_rules_ |= 0x800;
    if (PopupWriteCheckState & 0x2) GameDataStateStorage.fields_.game_rules_ |= 0x2;
    if (PopupWriteCheckState & 0x4) GameDataStateStorage.fields_.game_rules_ |= 0x8;
    if (PopupWriteCheckState & 0x8) GameDataStateStorage.fields_.game_rules_ |= 0x4;
    if (PopupWriteCheckState & 0x10) GameDataStateStorage.fields_.game_rules_ |= 0x1000;
    if (PopupWriteCheckState & 0x20) GameDataStateStorage.fields_.game_rules_ |= 0x1;
    if (PopupWriteCheckState & 0x40) GameDataStateStorage.fields_.game_rules_ |= 0x10;
    if (PopupWriteCheckState & 0x80) GameDataStateStorage.fields_.game_rules_ |= 0x20;
    if (PopupWriteCheckState & 0x100) GameDataStateStorage.fields_.game_rules_ |= 0x4000;
    if (PopupWriteCheckState & 0x200) GameDataStateStorage.fields_.game_rules_ |= 0x200;
    if (PopupWriteCheckState & 0x400) GameDataStateStorage.fields_.game_rules_ |= 0x40;
    if (!(PopupWriteCheckState & 0x800)) GameDataStateStorage.fields_.game_rules_ |= 0x100;
    if (PopupWriteCheckState & 0x1000) GameDataStateStorage.fields_.game_rules_ |= 0x2000;
    if (PopupWriteCheckState & 0x2000) GameDataStateStorage.fields_.game_rules_ |= 0x8000;
    if (PopupWriteCheckState & 0x4000) GameDataStateStorage.fields_.game_rules_ |= 0x80;
    if (PopupWriteCheckState & 0x8000) GameDataStateStorage.fields_.game_rules_ |= 0x400;
    if (PopupWriteCheckState & 0x10000) GameDataStateStorage.fields_.game_state_ |= 0x00800000;
    if (PopupWriteCheckState & 0x20000) GameDataStateStorage.fields_.game_state_ |= 0x01000000;
    AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
    AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000;
    ConfigFactionDetailMode = 0;

select_faction:
    if ((GameDataStateStorage.fields_.game_rules_ & 0x00100000) && !IsMultiplayerPBEM) {
        selected = ConsoleGlobal.faction_id_;
    } else {
        if (!ExpansionEnabled || (mode & 2)) {
            popup.start("FACTIONX", 0x40);
            {
                for (int i = 1; i < 8; ++i) {
                    if (mode == 2 &&
                        !(GameDataStateStorage.fields_.lock_masks_.alive_mask_ & (1 << i))) continue;
                    StringTemp[0] = 0;
                    strcat(StringTemp, Players[i].formal_name_faction);
                    strcat(StringTemp, "|");
                    say_open_paren();
                    strcat(StringTemp, Players[i].desc_name_faction);
                    say_close_paren();
                    popup.item(StringTemp, i);
                }
            }
            {
                int faction;    // esi
                popup.set_value(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
                popup.dialogs().ListBox::set_selection_callback(thumb_routine);
                thumb_routine(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
                faction = popup.exec();
                if (faction < 0) {
                    if (bit_count(human_mask) > 1 && selected > 0 && (human_mask & (1 << selected))) {
                        human_mask &= ~(1 << selected);
                        for (int f = 1; f < human_mask; ++f) {
                            if (human_mask & (1 << f)) {
                                selected = f;
                                break;
                            }
                        }
                        goto pbem_selection;
                    }
                    ConfigFactionPreview.close();
                    if (mode == 2) goto difficulty_menu;
                    goto rules_menu;
                }
                if (!(mode & 2)) DefaultPrefs.faction_id = faction;
                if (IsMultiplayerPBEM && (human_mask & (1 << faction))) {
                    X_pop("FACTIONTAKEN", 0);
                    goto select_faction;
                }
                selected = faction;
                if (popup.input_poll_state() == 2) {
                    ConfigFactionDetailMode = !ConfigFactionDetailMode;
                    goto select_faction;
                }
                if (popup.input_poll_state() == 3) {
                    ConfigFactionPreview.close();
                    if (X_pop_ask("RELOADFAC", 0x50, Players[faction].filename, 0, 0)) goto select_faction;
                    strcpy(Players[faction].filename, ParseStrBuffer[0].str);
                    strcpy(Players[faction].search_key, ParseStrBuffer[0].str);
                    if (faction) {
                        read_faction(&Players[faction], 0);
                        load_faction_art(faction);
                    }
                    compute_faction_modifiers(faction);
                    goto select_faction;
                }
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000) {
                    StringTemp[0] = 0;
                    strcat(StringTemp, "voices\\");
                    strcat(StringTemp, Players[faction].filename);
                    strcat(StringTemp, ".mp3");
                    g_WAVE_GENERAL.unload();
                    g_WAVE_GENERAL.load(StringTemp, 0x100);
                    g_WAVE_GENERAL.play();
                }
                if (!popup.input_poll_state()) {
                    StringTemp[0] = 0;
                    strcat(StringTemp, "NAME");
                    if (Players[faction].is_leader_female) strcat(StringTemp, "2");
                    int answer = X_pop_ask(StringTemp, 0x17, Players[faction].name_leader, 0, 0);
                    ConfigFactionPreview.close();
                    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                        g_WAVE_GENERAL.Sound::fade(0x7D0);
                    if (answer) goto select_faction;
                    strcpy(Players[faction].name_leader, ParseStrBuffer[0].str);
                    if (!ConsoleGroupAction) goto parse_factions;
                }
                for (;;) {
                    strcpy(gender_text, table_text(Players[faction].is_leader_female
                        ? Labels.strings_ptr[739] : Labels.strings_ptr[738]));
                    popup.start("CUSTOMNAMES", 0x44);
                    {
                        char *label = StringTable.get(Labels.strings_ptr[174]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].name_leader, 0x17);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[175]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].formal_name_faction, 0x27);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[176]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].noun_faction, 0x17);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[177]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].adj_name_faction, 0x17);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[483]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].title_leader, 0x17);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[484]);
                        popup.dialogs().EditGroup::edit(label, Players[faction].adj_leader, 0x17);
                    }
                    {
                        char *label = StringTable.get(Labels.strings_ptr[485]);
                        popup.dialogs().EditGroup::edit(label, gender_text, 0x17);
                    }
                    int answer = popup.exec();
                    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                        g_WAVE_GENERAL.Sound::fade(0x7D0);
                    ConfigFactionPreview.close();
                    if (answer < 0) goto select_faction;
                    if (!popup.input_poll_state()) break;
                    if (faction) {
                        read_faction(&Players[faction], 0);
                        load_faction_art(faction);
                    }
                }
                strcpy(Players[faction].name_leader, ParseStrBuffer[0].str);
                strcpy(Players[faction].formal_name_faction, ParseStrBuffer[1].str);
                strcpy(Players[faction].noun_faction, ParseStrBuffer[2].str);
                strcpy(Players[faction].adj_name_faction, ParseStrBuffer[3].str);
                strcpy(Players[faction].title_leader, ParseStrBuffer[4].str);
                strcpy(Players[faction].adj_leader, ParseStrBuffer[5].str);
                Players[faction].is_leader_female = !_strnicmp(gender_text, StringTable.get(Labels.strings_ptr[739]), 1);
            }
        } else {
            int picked;     // [ebp-0x2C]
            PickWinGlobal.init(thumb_routine, mode, ConfigFactionDetailMode);
            {
                for (int i = 1; i < 8; ++i) {
                    if (mode == 2 &&
                        !(GameDataStateStorage.fields_.lock_masks_.alive_mask_ & (1 << i))) continue;
                    StringTemp[0] = 0;
                    if (strcmp(Players[i].filename, "JENN282")) {
                        strcat(StringTemp, Players[i].formal_name_faction);
                        strcat(StringTemp, "|");
                        say_open_paren();
                        strcat(StringTemp, Players[i].desc_name_faction);
                        say_close_paren();
                    } else {
                        strcat(StringTemp, "RANDOM");
                    }
                    PickWinGlobal.listBox_.item(StringTemp, i);
                }
            }
            PickWinGlobal.listBox_.attach(&PickWinGlobal,
                PickWinGlobal.listBox_.Dialog::window_x_,
                PickWinGlobal.listBox_.Dialog::window_y_, 0x80020);
            DefaultPrefs.faction_id = range(DefaultPrefs.faction_id, 1, 7);
            PickWinGlobal.listBox_.set_selected_id(DefaultPrefs.faction_id);
            PickWinGlobal.field_5DB4_ = PickWinGlobal.listBox_.Dialog::get_selected_id();
            PickWinGlobal.listBox_.set_selection_callback(thumb_routine);
            thumb_routine(DefaultPrefs.faction_id);
            picked = PickWinGlobal.go_girl();
            if (picked < 0) {
                if (bit_count(human_mask) > 1 && selected > 0 && (human_mask & (1 << selected))) {
                    human_mask &= ~(1 << selected);
                    for (int f = 1; f < human_mask; ++f) {
                        if (human_mask & (1 << f)) {
                            selected = f;
                            break;
                        }
                    }
                    PickWinGlobal.GraphicWin::close();
                    goto pbem_selection;
                }
                ConfigFactionPreview.close();
                PickWinGlobal.GraphicWin::close();
                do_all_draws();
                goto difficulty_menu;
            }
            DefaultPrefs.faction_id = picked;
            if (IsMultiplayerPBEM && (human_mask & (1 << picked))) {
                X_pop("FACTIONTAKEN", 0);
                goto select_faction;
            }
            selected = picked;
            if (PickWinGlobal.field_5DA4_ == 0x105) {
                picked = rand() % 7 + 1;
                selected = picked;
                ConfigFactionPreview.close();
                PickWinGlobal.GraphicWin::close();
                goto random_name;
            }
            if (PickWinGlobal.field_5DA4_ == 0x107) {
                ConfigFactionDetailMode = !ConfigFactionDetailMode;
                goto select_faction;
            }
            if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000) {
                StringTemp[0] = 0;
                strcat(StringTemp, "voices\\");
                strcat(StringTemp, Players[picked].filename);
                strcat(StringTemp, ".mp3");
                g_WAVE_GENERAL.unload();
                g_WAVE_GENERAL.load(StringTemp);
                g_WAVE_GENERAL.play();
            }
            if (PickWinGlobal.field_5DA4_) {
                if (!strcmp(Players[picked].filename, "JENN282")) goto select_faction;
                if (PickWinGlobal.field_5DA4_) goto edit_names_expansion;
            }
random_name:
            while (strcmp(Players[picked].filename, "JENN282")) {
                StringTemp[0] = 0;
                strcat(StringTemp, "NAME");
                if (Players[picked].is_leader_female && PickWinGlobal.field_5DA4_ != 0x105)
                    strcat(StringTemp, "2");
                if (PickWinGlobal.field_5DA4_ == 0x105) strcat(StringTemp, "3");
                int answer;
                if (PickWinGlobal.field_5DA4_ == 0x105)
                    answer = X_pop_ask(StringTemp, 0x17, ConfigRandomLeaderInitial, 0, 0);
                else
                    answer = X_pop_ask(StringTemp, 0x17, Players[picked].name_leader, 0, 0);
                ConfigFactionPreview.close();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                if (answer) goto select_faction;
                PickWinGlobal.GraphicWin::close();
                strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
                if (!ConsoleGroupAction) goto parse_factions;
                if (PickWinGlobal.field_5DA4_ != 0x105) goto edit_names_expansion;
                Players[picked].is_leader_female = !Players[picked].is_leader_female;
            }
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            goto parse_factions;

edit_names_expansion:
            for (;;) {
                strcpy(gender_text, table_text(Players[picked].is_leader_female
                    ? Labels.strings_ptr[739] : Labels.strings_ptr[738]));
                popup.start("CUSTOMNAMES", 0x44);
                {
                    char *label = StringTable.get(Labels.strings_ptr[174]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].name_leader, 0x17);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[175]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].formal_name_faction, 0x27);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[176]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].noun_faction, 0x17);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[177]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].adj_name_faction, 0x17);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[483]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].title_leader, 0x17);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[484]);
                    popup.dialogs().EditGroup::edit(label, Players[picked].adj_leader, 0x17);
                }
                {
                    char *label = StringTable.get(Labels.strings_ptr[485]);
                    popup.dialogs().EditGroup::edit(label, gender_text, 0x17);
                }
                int answer = popup.exec();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                if (answer < 0) {
                    popup.close();
                    goto select_faction;
                }
                if (!popup.input_poll_state()) break;
                if (picked) {
                    read_faction(&Players[picked], 0);
                    load_faction_art(picked);
                }
            }
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
            strcpy(Players[picked].formal_name_faction, ParseStrBuffer[1].str);
            strcpy(Players[picked].noun_faction, ParseStrBuffer[2].str);
            strcpy(Players[picked].adj_name_faction, ParseStrBuffer[3].str);
            strcpy(Players[picked].title_leader, ParseStrBuffer[4].str);
            strcpy(Players[picked].adj_leader, ParseStrBuffer[5].str);
            Players[picked].is_leader_female = !_strnicmp(gender_text, StringTable.get(Labels.strings_ptr[739]), 1);
            goto select_faction;
        }

parse_factions:
        if (mode == 0) prefs_save(1);
        {
            int custom_count;   // [ebp-0x24]
            int special_count;  // edi
            int special_a;      // [ebp-0x18]
            int special_b;      // [ebp-0x20]
            custom_count = 14;
            special_count = 0;
            special_a = 0;
            special_b = 0;
            first_special[0] = 0;
            if (!text_open(AlphaxFileID, "CUSTOMFACTIONS")) {
                text_get();
                while (*text_item()) {
                    ++custom_count;
                    text_get();
                }
            }
            {
                for (int i = 1; i < 8; ++i) {
                    if (strcmp(Players[i].filename, "JENN282") && (Players[i].rule_flags & 0x80)) {
                        ++special_count;
                        if (special_count == 1) strcat(first_special, Players[i].filename);
                    }
                }
            }
            if (special_count == 1) {
                // The last random slot is kept in special_b itself (ebx,
                // 0x0058B40B), not in a local of its own; the pair test
                // below clears it again.
                {
                    for (int i = 1; i < 8; ++i) {
                        if (!strcmp(Players[i].filename, "JENN282")) special_b = i;
                    }
                }
                if (special_b) {
                    if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                    text_get();
                    for (int skip = 0; skip < 5; ++skip) text_get();
                    strcpy(Players[special_b].filename, text_item());
                    if (!strcmp(Players[special_b].filename, first_special)) {
                        text_get();
                        strcpy(Players[special_b].filename, text_item());
                    }
                    strcpy(Players[special_b].search_key, Players[special_b].filename);
                    read_faction(&Players[special_b], 0);
                    load_faction_art(special_b);
                }
            } else if (!special_count && rand() % 2) {
                int start = rand() % 7;
                for (int attempt = start; 1 - start + attempt < 8; ++attempt) {
                    int i = attempt % 7;
                    if (!strcmp(Players[i + 1].filename, "JENN282")) {
                        if (!special_a) special_a = i + 1;
                        else if (!special_b) special_b = i + 1;
                    }
                }
            }
            if (!special_a || !special_b) {
                special_a = 0;
                special_b = 0;
            }
            {
                for (int i = 1; i < 8; ++i) {   // [ebp-0x1C]
                    if (strcmp(Players[i].filename, "JENN282")) continue;
                    if (special_a == i) {
                        if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                        text_get();
                        for (int skip = 0; skip < 5; ++skip) text_get();
                        strcpy(Players[i].filename, text_item());
                        strcpy(Players[i].search_key, Players[i].filename);
                        if (i) {
                            read_faction(&Players[i], 0);
                            load_faction_art(i);
                        }
                    } else if (special_b == i) {
                        if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                        text_get();
                        for (int skip = 0; skip < 6; ++skip) text_get();
                        strcpy(Players[i].filename, text_item());
                        strcpy(Players[i].search_key, Players[i].filename);
                        if (i) {
                            read_faction(&Players[i], 0);
                            load_faction_art(i);
                        }
                    } else {
                        int row;    // ebx
                        do {
                            int residue;
                            do {
                                residue = rand() % custom_count;
                            } while (residue == 12 || residue == 13);
                            switch (residue / 7) {
                        case 0:
                                if (text_open(AlphaxFileID, "FACTIONS")) goto select_faction;
                                break;
                        case 1:
                                if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                                break;
                        case 2:
                                if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) goto select_faction;
                                break;
                            }
                            row = residue % 7;
                            text_get();
                            for (int skip = 0; skip < row; ++skip) text_get();
                            strcpy(Players[i].filename, text_item());
                            strcpy(Players[i].search_key, Players[i].filename);
                            for (int other = 1; other < 8; ++other) {
                                if (i != other && !strcmp(Players[i].filename, Players[other].filename)) {
                                    row = -1;
                                    other = 8;
                                }
                            }
                            if (row + 1 && i) {
                                read_faction(&Players[i], 0);
                                load_faction_art(i);
                            }
                        } while (row == -1);
                    }
                }
            }
        }
        load_flags();
        if (ExpansionEnabled && !IsMultiplayerNet && !IsMultiplayerPBEM &&
            (Players[selected].rule_flags & RFLAG_ALIEN)) {
            GameDataStateStorage.fields_.game_rules_ =
                (GameDataStateStorage.fields_.game_rules_ & ~0x200) | 0x140;
        }
        if (mode & 1) goto pbem_commit;
        do_all_draws();

pbem_selection:
        if (IsMultiplayerPBEM) {
            int bit = 1 << selected;    // edi
            human_mask |= bit;
            popup.start("HOTSEATADD0", 0x40);
            {
                for (int i = 1; i < 8; ++i) {
                    if (human_mask & (1 << i)) {
                        if (X_text_open(ScriptPopupFile, "MULTISTRING")) break;
                        text_get();
                        parse_says(0, get_title(i), -1, -1);
                        parse_says(1, get_name(i), -1, -1);
                        parse_says(2, get_noun(i), -1, -1);
                        StringTemp[0] = 0;
                        parse_string(TextBufferGetPtr, StringTemp);
                        popup.string(StringTemp);
                    }
                }
            }
            popup.start("HOTSEATADD1", 0x80);
            int added = popup.exec();
            if (added) {
                if (added < 0) human_mask &= ~bit;
                goto select_faction;
            }
        }
pbem_commit:
        if (IsMultiplayerPBEM) {
            GameDataStateStorage.fields_.lock_masks_.human_mask_ =
                static_cast<unsigned char>(human_mask);
            for (selected = 1; selected < 8; ++selected) {
                if (GameDataStateStorage.fields_.lock_masks_.human_mask_ & (1 << selected)) break;
            }
            if (selected >= 8) selected = 1;
            ConsoleGlobal.faction_id_ = selected;
            goto time_controls;
        }
    }
    ConsoleGlobal.faction_id_ = selected;
    GameDataStateStorage.fields_.lock_masks_.human_mask_ =
        static_cast<unsigned char>(1 << selected);
time_controls:
    if (IsMultiplayerNet) {
        time_controls_dialog(&popup);
        popup.set_value(AlphaIniPrefs.time_controls);
        time_choice = popup.exec();
        if (time_choice < 0) {
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x00100000)) goto select_faction;
            if (mode != 2) goto rules_menu;
            goto difficulty_menu;
        }
        AlphaIniPrefs.time_controls = time_choice;
        GameDataStateStorage.fields_.time_controls_selection_ = time_choice;
    }
    prefs_save(0);
    log_set_state(0);
    if (!(mode & 2)) GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    if (!(mode & 2) && MapSetupStatus) world_build();
    if (!(mode & 2)) setup_game(0);
    if (mode == 2) scenario_setup();
    if (IsMultiplayerNet) log_set_state(1);
    if (ExpansionEnabled) alien_start();
    if (!IsMultiplayerNet && !IsMultiplayerPBEM && !(mode & 2)) {
        planetfall(ConsoleGlobal.faction_id_);
    }
    if (IsMultiplayerPBEM) {
        clear_tamper();
        TamperState93A9AC = timeGetTime() + MapRandSeed;
        SYSTEMTIME date;    // [ebp-0x40]
        GetSystemTime(&date);
        TamperState93A9B4 = 0;
        TamperState93A9B0 = ((date.wYear << 4) + date.wMonth) * 32 + date.wDay;
    }
    return 0;
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058C2A0 0x0058CE9C 0x0058CEB4 0x0058C3C9
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058C2A0 0x0058CEB4 0x0058CECC 0x0058C715
// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code; a constant
// argument folds it away. One branchy expansion before the slot loop's
// cancel return makes the final teardown call ~StringList's second stage out
// of line as the image does (0x0058CE12); the image's EH map then matches
// exactly. The original's spend is not identified.
static inline int custom_planet_budget_probe(int x) {
    if (x == 0) x = 7; if (x == 1) x = 8; if (x == 2) x = 9; if (x == 3) x = 10;
    if (x == 4) x = 11; if (x == 5) x = 12; if (x == 6) x = 13; if (x == 7) x = 14;
    return x;
}

// ORIGINAL: 0x0058C2A0 ?custom_planet@@YAHHH@Z 0x0058C2A0-0x0058CE9C;0x006605A5-0x006607D7
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058c2a0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: exact 12-case image/message switch (jump table at 0x58ce9c) transcribed as a representative subset, not all 12 cases; Popup/SetupWin/Buffer[3] RAII locals reproduce the close cascade shape but not every StringStruct/list-walk detail.
int __cdecl custom_planet(int show_preview, int use_saved_choices) {
    Popup popup;
    Buffer previews[3];
    SetupWin setup;
    int slots[6] = {1, 3, 6, 5, 0, 0};
    char filename[64];
    custom_planet_budget_probe(1000);  // WORKAROUND(inline): see custom_planet_budget_probe

    if (show_preview) {
        setup.overlay_buffers_[0] = &previews[0];
        setup.overlay_buffers_[1] = &previews[1];
        setup.overlay_buffers_[2] = &previews[2];
    }
    for (int clear = 0; clear < 6; clear++) {
        (&MapOceanCoverage)[clear] = 0;
    }

    for (int i = 1; i < 7; i++) {
        int slot = slots[i - 1];
        if (!slot) {
            continue;
        }
        MapStorage.metadata_.planet_rules_[slot] = AlphaIniPrefs.custom_world[slot];
        if (use_saved_choices) {
            continue;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, "WORLD");
        if (show_preview) {
            switch (slot) {
            case 1:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = '1' + image;
                    filename[3] = range(MapNativeLifeForms + 1, 1, 3) + '0';
                    filename[5] = range(MapCloudCover + 1, 1, 3) + '0';
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "LAND");
                break;
            case 3:
                previews[0].load_pcx(WinScreenWidth == 800 ? "moon1_800.pcx" : "MOON1.PCX", 0, 10, 0xEC);
                previews[1].load_pcx(WinScreenWidth == 800 ? "moon2_800.pcx" : "MOON2.PCX", 0, 10, 0xEC);
                previews[2].load_pcx(WinScreenWidth == 800 ? "moon3_800.pcx" : "MOON3.PCX", 0, 10, 0xEC);
                setup.overlay_x_[0] = 0;
                setup.overlay_y_[0] = 0;
                setup.overlay_x_[1] = 0;
                setup.overlay_y_[1] = 0;
                setup.overlay_x_[2] = 0;
                setup.overlay_y_[2] = 0;
                strcat(StringTemp, "TIDES");
                break;
            case 4:
                strcat(StringTemp, "ORBIT");
                break;
            case 5:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = range(MapOceanCoverage + 1, 1, 3) + '0';
                    filename[3] = range(MapNativeLifeForms + 1, 1, 3) + '0';
                    filename[5] = '1' + image;
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "CLOUDS");
                break;
            case 6:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = range(MapOceanCoverage + 1, 1, 3) + '0';
                    filename[3] = '1' + image;
                    filename[5] = range(MapCloudCover + 1, 1, 3) + '0';
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "LIFE");
                break;
            }
        } else {
            switch (slot) {
            case 1:
                strcat(StringTemp, "LAND");
                break;
            case 3:
                strcat(StringTemp, "TIDES");
                break;
            case 4:
                strcat(StringTemp, "ORBIT");
                break;
            case 5:
                strcat(StringTemp, "CLOUDS");
                break;
            case 6:
                strcat(StringTemp, "LIFE");
                break;
            }
        }
        popup.start(StringTemp, 0x40);
        popup.set_value(AlphaIniPrefs.custom_world[slot]);
        int choice = show_preview ? setup.do_menu(&popup, 1, 0) : popup.exec();
        MapStorage.metadata_.planet_rules_[slot] = choice;
        if (choice < 0) {
            return 1;
        }
        if (popup.input_poll_state()) {
            MapStorage.metadata_.planet_rules_[slot] = -1;
        } else {
            AlphaIniPrefs.custom_world[slot] = choice;
        }
    }

    if (show_preview) {
        setup.overlay_buffers_[0] = 0;
        setup.overlay_buffers_[1] = 0;
        setup.overlay_buffers_[2] = 0;
    }
    MapLandCoverage = 2 - MapOceanCoverage;
    prefs_save(0);
    return 0;
}

// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code. The image's
// size_of_planet teardowns call ~StringList/~Scroll out of line at the menu-
// failure and success returns but expand them at the custom-map returns
// (0x0058D072, 0x0058D248, 0x0058D3F7, 0x0058D5BB); sixteen two-test
// expansions at entry reproduce that EH topology exactly (a single 32-test
// chain did too); eight do with size_of_planet_number below. The original
// spent the same budget on inline calls not yet
// identified.
static inline int size_of_planet_budget_probe(int x)
{
    if (x == 0) x = 7;
    if (x == 1) x = 8;
    return x;
}



// WORKAROUND(decl): size_of_planet's two custom-size "StringTemp = number"
// runs as one inline helper (no out-of-line copy in the image; name and home
// unknown). Its /Ob2 cost replaces six of the sixteen entry probes above
// (runscan --try; ten probes plus the helper give the same fe rows and
// teardown decisions as sixteen without it).
static inline void size_of_planet_number(int value, char *number) {
    StringTemp[0] = 0;
    _itoa(value, number, 10);
    strcat(StringTemp, number);
}

// ORIGINAL: 0x0058CED0 ?size_of_planet@@YAHH@Z 0x0058CED0-0x0058D6A4;0x006607D7-0x00660989 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058ced0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: real Popup/SetupWin RAII locals with Popup::start()'s six args re-derived from push order (p1 is the fixed 0x9b8aa8 char* buffer, not the GraphicWin* - this is backwards from what an earlier sibling function assumed), plus text_open/text_get/text_item/EditGroup::edit/ prefs_get/X_pop/Path::init argument order verified against raw disasm; compiles, MISMATCH #24 (past the prologue and into the item-list loop - likely the itemNum2/itemCount stack array layout or the loop bound).
int __cdecl size_of_planet(int mode) {
    size_of_planet_budget_probe(1001);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1002);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1003);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1004);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1005);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1006);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1007);  // WORKAROUND(inline)
    size_of_planet_budget_probe(1008);  // WORKAROUND(inline): ninth probe pays for the written-out label once rnd(5, "INIT1") expands here
    Popup popup;
    SetupWin setup;
    popup.start("WORLDSIZE", mode != 2 ? 0x40 : 0);

    int width;
    MapSetupStatus = 1;
    if (text_open(AlphaxFileID, "WORLDSIZE")) {
        MapSizePlanet = 2;
        width = 0x28;
        MapLatitudeBounds = 0x50;
    } else {
        {
            int width_for_choice[32];
            int height_for_choice[32];
            text_get();
            int item_count = text_item_number();
            if (item_count > 0x20) item_count = 0x20;
            for (int item = 0; item < item_count; ++item) {
                text_get();
                clear();
                says(text_item());
                popup.item(StringTemp, item);
                width_for_choice[item] = text_item_number();
                height_for_choice[item] = text_item_number();
            }
            clear();
            say_label(0x1104 / 4);
            popup.item(StringTemp, 0x63);

            int choice = static_cast<int>(AlphaIniPrefs.custom_world[0]);
            popup.set_value(choice);
            if (mode == 1) {
                MapSizePlanet = static_cast<uint32_t>(choice);
            } else {
                if (mode == 2) choice = popup.exec();
                else choice = setup.do_menu(&popup, 1, 0);
                MapSizePlanet = static_cast<uint32_t>(choice);
                if (choice < 0) {
                    return 1;
                }
                // 0x0058D12F: the popup's at-key poll flag (+0x3100) asks
                // for a random size; a menu choice other than "custom" is
                // remembered.
                if (popup.input_poll_state() != 0) {
                    choice = rnd(5, "INIT1");
                    MapSizePlanet = static_cast<uint32_t>(choice);
                } else if (choice != 0x63) {
                    AlphaIniPrefs.custom_world[0] = static_cast<uint32_t>(choice);
                }
            }
            if (choice == 0x63) {
            } else {
                width = width_for_choice[choice];
                MapLatitudeBounds = height_for_choice[choice];
                goto size_complete;
            }
        }

        {
            popup.start("CUSTOMMAP", 0x44);
            const int pref_x = prefs_get("Custom Map X", 0x28, 0);
            const int pref_y = prefs_get("Custom Map Y", 0x50, 0);
            char number[80];
            size_of_planet_number(pref_x, number);
            {
                LPSTR label = StringTable.get(Labels.strings_ptr[0x1108 / 4]);
                popup.dialogs().edit(label, StringTemp, 8);
            }
            size_of_planet_number(pref_y, number);
            {
                LPSTR label = StringTable.get(Labels.strings_ptr[0x110C / 4]);
                popup.dialogs().edit(label, StringTemp, 8);
            }

            if (popup.exec() < 0) {
                return 1;
            }
            int custom_width = atoi(ParseStrBuffer[0].str);
            int custom_height = atoi(ParseStrBuffer[1].str);
            if (custom_width >= 0x10) {
                if (custom_width > 0x100) custom_width = 0x100;
            } else custom_width = 0x10;
            if (custom_height >= 0x10) {
                if (custom_height > 0x100) custom_height = 0x100;
            } else custom_height = 0x10;
            if (custom_width > 0x80 || custom_height > 0x80) {
                if (X_pop("VERYLARGEMAP", 0) != 0) {
                    return 1;
                }
            }
            prefs_put("Custom Map X", custom_width, 0);
            prefs_put("Custom Map Y", custom_height, 0);
            MapLatitudeBounds = custom_height;
            width = custom_width;
        }
    }

size_complete:
    MapLongitudeBounds = width * 2;
    prefs_save(0);
    if (mode < 2) {
        map_init();
        Paths.init();
    }
    WorldClimateWorldWin.field_239BC_ |= 8;
    return 0;
}


// Random planet settings shared by map_menu and multiplayer_init; both
// images expand it inline (multiplayer_init keeps only the ocean, land and
// orbit stores plus the erosion roll's rand() call, as it overrides the rest).
static inline void random_planet_rules() {
    if (IsMultiplayerNet) {
        MapOceanCoverage = 0;
    } else {
        const int ocean_roll = rand() % 7;
        MapOceanCoverage = 2 - range((ocean_roll + 1) / 2, 0, 2);
    }
    MapLandCoverage = 2 - MapOceanCoverage;
    const int erosion_roll = rand() % 6;
    MapPlanetaryOrbit = 1;
    MapCloudCover = 1;
    MapNativeLifeForms = 1;
    MapErosiveForces = (erosion_roll + 5) / 5;
}


// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code. Two
// zero-code expansions after map_menu's menu-cancel return move the
// SetupWin list teardown's inline stage from that return (where the image
// calls ~StringList 0x004066C0) to the final return, as in the image.
static inline void map_menu_budget_probe(int x) {
    if (x == 0) x = 1;
}

// map_menu's string operands, each read from the image's push operand at
// 0x0058D710 ("MAPMENU"), 0x0058D840 ("maps\\xplanet.mp") and 0x0058D847
// ("maps\\xplanetx.mp") and checked against the literal text in the body.
// RECOVERY_LITERAL: v1 0x0068F8A0 0x0068F8A8 $SG65886 0x0058D6B0
// RECOVERY_LITERAL: v1 0x0068F8A8 0x0068F8B8 $SG65904 0x0058D6B0
// RECOVERY_LITERAL: v1 0x0068F8B8 0x0068F8C9 $SG65907 0x0058D6B0
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058D6B0 0x0058DB18 0x0058DB2C 0x0058D778
// ORIGINAL: 0x0058D6B0 ?map_menu@@YAHH@Z 0x0058D6B0-0x0058DB17;0x00660989-0x00660A96 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058d6b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: MISMATCH #11. Real SetupWin/Popup/Buffer/Spot/StringStruct locals (Popup embeds Scroll+2 FlatButtons per the scaffold) do produce the SEH frame and destructor-chain shape, but local declaration order/count likely differs from the original's stack layout past instruction 11.
int __cdecl map_menu(int quick_start) {
    SetupWin setup;
    Popup popup;
    setup.opening_window_snapshot_ = OpeningWindow;
    int choice;

map_menu_retry:
    MapSetupStatus = 0;
    popup.start(PopupStartCaption, "MAPMENU", -1, 0, 0, 0);
    choice = static_cast<int>(DefaultPrefs.map_type);
    popup.set_value(choice);
    if (quick_start == 0) choice = setup.do_menu(&popup, 1, quick_start);
    if (choice < 0) return 1;
    map_menu_budget_probe(0); map_menu_budget_probe(1);  // WORKAROUND(inline)

    DefaultPrefs.map_type = static_cast<uint32_t>(choice);
    prefs_save(0);
map_menu_dispatch:
    switch (choice) {
    case 0:
    case 1:
        if (size_of_planet(quick_start) != 0) goto map_menu_retry;
        if (MapSetupStatus != 0) {
            if (choice == 1) {
                // A cancelled custom planet re-runs the size choice, not the menu.
                if (custom_planet(1, quick_start) != 0) {
                    goto map_menu_dispatch;
                }
                break;
            }
            random_planet_rules();
        }
        break;
    case 2:
        if (load_map_daemon("maps\\xplanet.mp") != 0)
            goto map_menu_retry;
        world_climate();
        break;
    case 3:
        if (load_map_daemon("maps\\xplanetx.mp") != 0)
            goto map_menu_retry;
        world_climate();
        break;
    case 4:
        if (load_map() == 0) {
            world_climate();
            break;
        }
        quick_start = 0;
        goto map_menu_retry;
    }

    if (config_game(quick_start) == 0) return 0;
    goto map_menu_retry;
}



// ORIGINAL: 0x0058DB30 ?multiplayer_init@@YAHH@Z 0x0058DB30-0x0058E35E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058db30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: prologue diverges at #7 - original threads the literal 1 through esi across the whole prologue (mov esi,1; ...; mov [addr],esi; ...; push esi for the later top_menu(1) call); rebuilt just stores/pushes the literal 1 directly at each site. Tried binding a local `int one=1` reused at both sites - similarity unchanged (0.74). Body is otherwise a full structural transcription (322/~420 mnemonics in common), including the two literal duplicated cleanup blocks the original also duplicates.
int __cdecl multiplayer_init(int startup_mode) {
    int result;

    IsMultiplayerNet = 1;
    log_set_state(0);
    NetWinDialog.config_players();

    if (startup_mode != 0) {
        const uint32_t initial_voice_flags = NetDaemonNet.voice_flags_D8_;
        NetDaemonNet.voice_hotkey_48_ = 0xDC;
        NetDaemonNet.voice_hotkey_4c_ = 0x100DC;
        NetDaemonNet.voice_flags_D8_ = initial_voice_flags | 1;
    } else {
        if (NetDaemonNet.init(0) != 0) {
            result = 1;
            goto do_cleanup;
        }
    }

    if (IsMultiplayerPBEM != 0) {
        IsMultiplayerNet = 0;
        int menu = top_menu(1);
        if (menu != 0) IsMultiplayerPBEM = 0;
        return menu;
    }

    NetDaemonNet.primary_state_.trailing_words_[1] = 0;
    if (ExitTurnLoop == 0 && NetWinDialog.exec() >= 1) {
        GameDataStateStorage.fields_.diff_level_current_ =
            NetWinSessionOptions.fields_.difficulty_;
        GameDataStateStorage.fields_.game_rules_ = NetWinSessionOptions.fields_.rule_bits_a_;
        GameDataStateStorage.fields_.game_state_ |= NetWinSessionOptions.fields_.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.fields_.rule_bits_c_ & 0x10u) != 0)
            GameDataStateStorage.fields_.net_turn_flags_ |= 0x10u;
        else
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x10u;

        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            GameDataStateStorage.fields_.lock_masks_.human_mask_ = 0;
            for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
                GameDataStateStorage.fields_.lock_masks_.human_mask_ |=
                    1 << NetDaemonNet.process_slots_[i].state_.identity_;

            if (NetWinSessionData.fields_.mode_ == 0) {
                if (text_open(AlphaxFileID, "WORLDSIZE") != 0) {
                    MapSizePlanet = 2;
                    MapLongitudeBounds = 0x28;
                    MapLatitudeBounds = 0x50;
                } else {
                    int longitude[32];
                    int latitude[32];
                    int count;
                    text_get();
                    count = text_item_number();
                    if (count > 0x20) count = 0x20;
                    for (int i = 0; i < count; ++i) {
                        text_get();
                        text_item();
                        longitude[i] = text_item_number();
                        latitude[i] = text_item_number();
                    }
                    const int8_t selected =
                        NetWinSessionOptions.fields_.planet_size_;
                    const int index = selected == -1 ? (int)rnd(5, "INIT1") : selected;
                    MapSizePlanet = static_cast<uint32_t>(index);
                    MapLongitudeBounds = longitude[index] << 1;
                    MapLatitudeBounds = latitude[index];
                }

                map_init();
                Paths.init();
                WorldClimateWorldWin.field_239BC_ |= 8;

                random_planet_rules();
                MapOceanCoverage = NetWinSessionOptions.fields_.ocean_coverage_;
                MapErosiveForces = NetWinSessionOptions.fields_.erosion_;
                MapCloudCover = NetWinSessionOptions.fields_.cloud_cover_;
                MapNativeLifeForms = NetWinSessionOptions.fields_.native_life_;
                world_build();
            }

            if (NetWinSessionData.fields_.mode_ != 3 && NetWinSessionData.fields_.mode_ != 2) {
                GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                setup_game(0);
            }
        }

        // Native 0x58DE3D stores PlayerData +0x2088 (unk_102).
        for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
            PlayersData[NetDaemonNet.process_slots_[i].state_.identity_].unk_102 =
                NetWinLobbyRows.rows_[i].owner_;

        GameDataStateStorage.fields_.game_state_ |= NetWinSessionOptions.fields_.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.fields_.rule_bits_c_ & 0x10u) != 0)
            GameDataStateStorage.fields_.net_turn_flags_ |= 0x10u;
        else
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x10u;
        GameDataStateStorage.fields_.time_controls_selection_ = static_cast<uint8_t>(
            NetWinSessionOptions.fields_.time_control_);
        ConsoleGlobal.faction_id_ =
            NetDaemonNet.pid_2_who(static_cast<unsigned long>(NetDaemonNet.local_player_id()));

        if (NetWinSessionData.fields_.mode_ == 3) {
            GameDataStateStorage.fields_.game_state_ |= 0x10u;
            if (NetDaemonNet.send_files() != 0) {
                pop_close();
                result = 1;
                goto do_cleanup;
            }
            if (NetDaemonNet.primary_state_.leading_words_[0] != 0)
                scenario_setup();
        } else {
            GameDataStateStorage.fields_.game_state_ &= ~0x10u;
        }

        log_reset();
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            GameDataStateStorage.fields_.lock_masks_.human_mask_ = 0;
            for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
                GameDataStateStorage.fields_.lock_masks_.human_mask_ |=
                    1 << NetDaemonNet.process_slots_[i].state_.identity_;
            NetDaemonNet.send_rules();
            NetDaemonNet.send_game();
        } else {
            for (int i = 0; i != 11; ++i)
                AltNatural[i] = AltContour[i];
            while (static_cast<int32_t>(
                       NetDaemonNet.primary_state_.trailing_words_[1]) < 8 &&
                   ExitTurnLoop == 0) {
                NetDaemonNet.receive();
                do_all_tasks();
            }
        }

        if (NetWinSessionData.fields_.mode_ == 3) {
            game_reload(1, 0);
        }

        NetDaemonNet.locks_.clear();
        if (ControlTurnPhase == 0) planetfall(ConsoleGlobal.faction_id_);

        if (NetWinSessionData.fields_.mode_ == 3) {
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
            parse_says(1, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
            parse_says(2, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
            parse_set(Players[ConsoleGlobal.faction_id_].noun_gender,
                      Players[ConsoleGlobal.faction_id_].is_noun_plural);
            parse_says(3, Players[ConsoleGlobal.faction_id_].noun_faction, -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0));
            space();
            say_year(StringTemp);
            parse_says(4, StringTemp, -1, -1);
            parse_num(0, ScenarioSave.objective_req_victory);
            parse_num(1, ScenarioSave.objectives_sudden_death_victory);
            parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000u));
            parse_num(3, ScenarioSave.ending_mission_year);
            X_pop("SCENARIO", "INTRO", 0);
        }

        result = ExitTurnLoop;
    } else {
        result = 1;
        NetDaemonNet.reset();
    }

    if (result != 0) {
do_cleanup:
        log_set_state(0);
        NetDaemonNet.reset();
        IsMultiplayerNet = 0;
    }
    return result;
}




// top_menu's string operands, each read from the image's push operand and
// checked against the literal text in the body; the repeated file names are
// separate unpooled copies at distinct addresses.
// RECOVERY_LITERAL: v1 0x0068F8F4 0x0068F908 $SG65994 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F908 0x0068F916 $SG65998 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F918 0x0068F926 $SG65999 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F928 0x0068F935 $SG66001 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F938 0x0068F945 $SG66002 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F948 0x0068F956 $SG66007 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F958 0x0068F966 $SG66008 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F968 0x0068F975 $SG66010 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F978 0x0068F985 $SG66011 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F988 0x0068F990 $SG66012 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F990 0x0068F998 $SG66013 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F998 0x0068F9A5 $SG66038 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F9A8 0x0068F9AE $SG66056 0x0058E360
// RECOVERY_LITERAL: v1 0x0068F9B0 0x0068F9B9 $SG66057 0x0058E360
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058E360 0x0058EE34 0x0058EE4C 0x0058E758

// ORIGINAL: 0x0058E360 ?top_menu@@YAHH@Z 0x0058E360-0x0058EE31;0x00660A96-0x00660BD7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058e360.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: `SetupWin setup; Popup popup;` real locals (RAII for their destructor cascades) with the full main-menu retry loop transcribed via goto+labels matching original addresses (window-creation blocks x3, do_menu switch on 6 cases, new-game wizard parse_says/X_pop block); diverges at instr #14, not chased further given size.
int __cdecl top_menu(int mode) {
    SetupWin setup;
    Popup popup;
    int esi = 0;
    int edi = 0;

    if (mode == 0) {
        IsMultiplayerPBEM = 0;
        clear_all_player_messages();
    }
    prefs_load(0);
    prefs_use();
    if (mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0) {
        g_TOP_MENU_WAVE.load("fx\\opening menu.wav", 2);
        g_TOP_MENU_WAVE.play();
    }
    if (~AlphaIniPrefs.preferences & 0x20000000u)
        DialogDefaults[36] |= 4u;
    else
        DialogDefaults[36] &= ~4u;

    if (ExpansionEnabled != 0) {
        init_opening(WinScreenWidth == 0x320
            ? "xopeningb.pcx" : "xopeninga.pcx");
    } else {
        init_opening(WinScreenWidth == 0x320
            ? "openingb.pcx" : "openinga.pcx");
    }
    setup.opening_window_snapshot_ = OpeningWindow;
    TurnUpkeepPhase = 0;

    // The menu loop is left only through gotos to the shared exit below; the
    // function ends in a top-level return, so it has no closing-brace
    // teardown (the image has two teardown sites, not three).
    do {
        if (ExpansionEnabled != 0) {
            init_opening(WinScreenWidth == 0x320
                ? "xopeningb.pcx" : "xopeninga.pcx");
        } else {
            init_opening(WinScreenWidth == 0x320
                ? "openingb.pcx" : "openinga.pcx");
        }
        setup.opening_window_snapshot_ = OpeningWindow;
        prefs_load(0);
        prefs_use();

        edi = mode;
        popup.start(PopupStartCaption, edi != 0 ? "HOTSEAT" : "TOPMENU",
                    -1, 0, 0, 0);
        if (edi == 0) popup.set_value(static_cast<int>(DefaultPrefs.top_menu));

        esi = setup.do_menu(&popup, 1, 0);
        if (esi < 0 || esi == 6) {
            return 1;
        }

        if (edi == 0) {
            DefaultPrefs.top_menu = static_cast<uint32_t>(esi);
            prefs_save(0);
        } else if (esi > 0) {
            ++esi;
        }

        if (static_cast<unsigned int>(esi) > 5) goto top_menu_normal_return;
        switch (esi) {
        case 0:
        case 1:
            esi = map_menu(esi == 1 ? 1 : 0);
            break;

        case 3:
            GameDataLoadRequest = mode;
            {
                const int loaded = load_game(0, 0);
                esi = loaded;
                GameDataLoadRequest = 0;
                if (loaded != 0) continue;
            }
            ControlTurnPhase = ConsoleGlobal.faction_id_;
            if (GameSessionActive != 0) {
                game_reload(0, 0);
            }
            break;

        case 2:
            if (mode == 0) {
                edi = 1;
                esi = setup.do_menu("SCENARIOMENU", edi, 0);
                if (esi < 0) continue;
                if (esi == edi) goto top_menu_new_game;
                if (esi == 2) {
                    const int loaded = load_map();
                    esi = loaded;
                    if (loaded != 0) continue;
                    config_game(3);
                    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                    setup_game(edi);
                    ControlTurnPhase = ConsoleGlobal.faction_id_;
                    GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
                    break;
                }
            } else {
                esi = 0;
            }
            edi = IsMultiplayerPBEM;
            {
                const int loaded = load_game(1, 0);
                if (edi != 0) {
                    IsMultiplayerPBEM = 1;
                    if (IsMultiplayerNet != 0) IsMultiplayerPBEM = 0;
                } else {
                    IsMultiplayerPBEM = 0;
                }
                if (loaded != 0) continue;
            }
            if (GameSessionActive != 0) {
                game_reload(0, 0);
            }
            if (esi != 0) {
                ControlTurnPhase = ConsoleGlobal.faction_id_;
                GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
                goto top_menu_normal_return;
            }
            esi = config_game(2);
            if (esi != 0) {
                read_rules(1);
                break;
            }
            {
                ControlTurnPhase = ConsoleGlobal.faction_id_;
                parse_says(1, get_title(ConsoleGlobal.faction_id_), -1, -1);
                parse_says(2, get_name(ConsoleGlobal.faction_id_), -1, -1);
                parse_says(3, get_noun(ConsoleGlobal.faction_id_), -1, -1);
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0));
                strcat(StringTemp, BufferJoinSpace);
                say_year(StringTemp);
                parse_says(4, StringTemp, -1, -1);
                parse_says(0, StringTemp, -1, -1);
                parse_num(0, ScenarioSave.objective_req_victory);
                parse_num(1, ScenarioSave.objectives_sudden_death_victory);
                parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000));
                parse_num(3, ScenarioSave.ending_mission_year);
                X_pop("SCENARIO", "INTRO", 0);
                ConsoleGlobal.set_move(1);
            }
            goto top_menu_normal_return;

        case 4:
            esi = multiplayer_init(0);
            break;

        case 5:
            show_credits();
            continue;
        }
        if (esi == 0) break;
    } while (ExitTurnLoop == 0);
    goto top_menu_normal_return;

    top_menu_new_game:
        size_of_planet(edi);
        MapOceanCoverage = static_cast<uint32_t>(edi);
        MapLandCoverage = static_cast<uint32_t>(edi);
        MapErosiveForces = static_cast<uint32_t>(edi);
        MapPlanetaryOrbit = static_cast<uint32_t>(edi);
        MapCloudCover = static_cast<uint32_t>(edi);
        MapNativeLifeForms = static_cast<uint32_t>(edi);
        map_wipe();
        config_game(3);
        GameDataStateStorage.fields_.vehicle_current_count_ = 0;
        setup_game(edi);
        ControlTurnPhase = ConsoleGlobal.faction_id_;
        GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;

top_menu_normal_return:
    if ((mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0)
        || g_TOP_MENU_WAVE.is_playing() != 0) {
        g_TOP_MENU_WAVE.fade(0xBB8);
        g_TOP_MENU_WAVE.unload();
    }
    return ExitTurnLoop;
}


// ---------------------------------------------------------------------------
// Desktop, system and game lifetime, and the faction phrase helpers. These
// were game.cpp, game_desktop.cpp, maininterface.cpp and stringstruct.cpp
// bodies; the image lays them out in this unit's code and literal runs, and
// top_menu, multiplayer_init, desktop_init and system_close expand its
// game_reload, desktop_close, game_close and close_opening.
// ---------------------------------------------------------------------------

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0058EE50 ?desktop_update@@YAXXZ 0x0058EE50-0x0058EE51 BYTE_EXACT
// size      1 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}


// The desktop coordinator calls each real source-owned window and publishes
// its four bounds in the existing Console storage used by blink_timer and
// the Console splitter. It sits in this unit's code run and expands the
// unit's close_opening.
// ORIGINAL: 0x0058EE60 ?desktop_init@@YAHH@Z 0x0058EE60-0x0058EFEB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:818efd04347d6f83e98e66d75a443fb646d75aaa:src/unrecovered/0058ee60.cpp", "Pinned58EE60..58EFEB bounds939430..43C are Console+23D80..8C; DIB width/height come from the Console virtual Buffer base, not detached globals.", "Original startup interfaces, multiplayer/PBEM branches and two non-input pumps; OpeningWindow is deleted through its actual GraphicWin owner.", "recovery/tasks/startup-desktop-owner-oct5.json", "recovery/tests/test_startup_actual_source.py"], "reason": "Complete startup coordination through real owned windows and Console bounds; preserve the original calls, stores, redraw/cover ordering and OpeningWindow lifetime. Current source home is game.cpp, not a claim about original translation-unit ownership."}
// kind      game
// TRIED: none confirmed byte-exact; MISMATCH #5 push-count differs - the original sets up an ebp frame (push ebp/mov ebp,esp) that this body's register pressure does not need, so /O2 drops it.
int __cdecl desktop_init(int fresh) {
    load_music(ConsoleGlobal.faction_id_);
    ConsoleInfoWin.init();
    WorldClimateWorldWin.init(fresh);
    ConsoleStatusWin.init();
    mapwin_system_init(fresh);
    MultiWindow.init();
    TutWinBaseWindow.init();
    TutWinDesWindow.init();
    DiplomacyPopup.init();
    ReportIfGlobal.init();
    ConsolePrefWin.init();

    // Both native stores survive: the virtual-base width read sits between
    // them, so VC6 cannot drop the first one.
    ConsoleGlobal.field_23D80_ = 10;
    ConsoleGlobal.field_23D84_ = ConsoleGlobal.dib_.bmiHeader.biWidth - 10;
    ConsoleGlobal.field_23D80_ = ConsoleGlobal.alpha_menu_.client_rect().right
        - ConsoleGlobal.alpha_menu_.client_rect().left + 10;
    if (IsMultiplayerNet) {
        ConsoleGlobal.field_23D84_ += MultiWindow.client_rect().left
            - MultiWindow.client_rect().right;
    }
    ConsoleGlobal.field_23D88_ = 10;
    ConsoleGlobal.field_23D8C_ = -10 - ConsoleGlobal.dib_.bmiHeader.biHeight;

    if (GameDataStateStorage.fields_.turn_current_num_) {
        draw_map(1);
    }

    MainInterfaceGlobal.Win::show(0);
    ConsoleGlobal.Win::show(0);
    if (IsMultiplayerPBEM) {
        ConsoleGlobal.cover_window_.GraphicWin::fill(0);
        ConsoleGlobal.cover_window_.Win::show(0);
    }
    do_all_non_input();
    close_opening();
    do_all_non_input();
    return 0;
}


/*
Purpose: Tear the desktop down at the end of each main-loop pass.
// ORIGINAL: 0x0058EFF0 ?desktop_close@@YAXXZ 0x0058EFF0-0x0058F031 FILE BYTE_EXACT
// size      65 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x005D4E40
// LEVER: byte-exact on promotion. The fixed windows are REAL OBJECTS now
//   (TutWinDesWindow, TutWinBaseWindow in their domain .cpps; ConsoleStatusWin
//   in statuswin.cpp; MainInterfaceGlobal in mapwin.cpp for TutWinIfaceWindow)
//   and still fold each receiver to `mov ecx, imm32`, the displacement
//   relocated; `Win::hide` is qualified to keep the slot-2 virtual a direct
//   tail `jmp`, and `GraphicWin::close` on the WorldWin and on MultiWindow's
//   front is qualified so a later base-class declaration cannot steal the
//   call from the direct E8 the image has. WorldClimateWorldWin is declared
//   in worldwin.h and defined in map.cpp, so both bodies name the object -
//   same fold as the old by-value world_climate_window() helper, one binding
//   name instead of two.
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_close() {
    TutWinDesWindow.close();
    TutWinBaseWindow.close();
    mapwin_system_shutdown();
    ConsoleStatusWin.close();
    WorldClimateWorldWin.GraphicWin::close();
    // The MultiWin object's GraphicWin front - the image closes that base
    // directly (`call 0x5d4e40` on `mov ecx, 0x7fd648`).
    MultiWindow.GraphicWin::close();
    MainInterfaceGlobal.Win::hide();
}


/*
Purpose: Bring the shared system up, once per process: create the four data
         directories, read the language preference and load the string and
         label tables, upper-case the two default button captions out of them,
         load the fixed art, then build the eight map-window slots and the
         desktop's standing windows.
// ORIGINAL: 0x0058F040 ?system_init@@YAHXZ 0x0058F040-0x0058F245;0x00660BD7-0x00660BEC BYTE_EXACT
// size      538 bytes
// prototype int (__cdecl ?system_init@@YAHXZ)(void)
// callers   1   call targets   23
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004219E0 0x00429300 0x00455E80 0x004595C0 0x00459680 0x004626E0 0x00471D50 0x004BA220 0x0051D890 0x00588AA0 0x0059DB40 0x005C9860 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x00626450 0x00627100 0x00645470 0x0064557F 0x0064558A 0x00646829
// indirect  0x0058F11A 0x0058F159
// LEVER: the SEH frame the archive unit could not reproduce is the
//   `new MapWin` below it. MapWin has a real destructor (mapwin.h), so /GX
//   makes VC6 wrap the whole function - `push -1 / push handler / fs:[0]` at
//   instruction 0, the `[ebp-4]` state toggle around the constructor and the
//   `operator delete` funclet at 0x660BD7 - which is exactly the frame the
//   artifact's plain operator-new-plus-placement-new spelling was missing.
//   `new MapWin` with no argument is also what produces the image's
//   `push 1` before the ctor call: that 1 is the hidden most-derived flag,
//   not the catalogue's `H`.
// LEVER: the two captions are `label_get` (alpha.h's forceinline of
//   Strings::get over Labels.strings_ptr) at offsets 102 and 101, which is the
//   image's `[strings_ptr + 0x198]` and `[+ 0x194]`.
// LEVER: slot 0 of the map-window table is published last, as the Console -
//   the image's single `mov dword ptr [0x7d3c3c], 0x9156b0` - and the seven
//   slots above it are the ones the allocation loop walks (0x7D3C40 to
//   0x7D3C5C exclusive), so the loop is `&MapWinTable[1] .. &MapWinTable[8]`
//   and not the whole table.
// Historical constraint below is superseded: InterfaceMessageWin now owns
// original 0x7F67F8, and WinBubbleWindow has its genuine MultiDebug owner.
// system_init uses those source objects; no fixed receiver is needed here.
// TRIED: `static_cast<MultiDebug *>(&WinBubbleWindow)` and a named extern for
//   the MessageWin storage - the image has no MessageWin object in this tree
//   (messagewin.cpp defines nothing at 0x7F67F8) and WinBubbleWindow is a
//   `Win`, so both stay address spellings, the way game_reload reaches
//   0x008E9F60.
// The shared graphics-language API is separately compiled: the original
// calls set_language at 0x0058F0A1, rather than inlining its global store.
// TRIED: the two caption builds spelled `label_get(102)` / `label_get(101)`
//   instead of the local `LPSTR *labels` plus `StringTable.get(labels[n])` now
//   in the body - that spelling scores 32/135 against this one's 34, and
//   game_init measures the same either way, so the two bodies legitimately
//   spell it differently. What is left is the register the label value lands
//   in and the register holding `Labels.strings_ptr` across the StringTemp
//   store (image eax/eax, ours eax/ecx on the first caption), plus the
//   `SystemInterfaceFlags` read-modify-write, which the image interleaves with
//   maininterface_init's argument setup and runs in esi while ours runs it as
//   a block in edx and re-derives the zero twice. All register assignment;
//   /Ob0 scores worse (53/135) because map_win_table and friends turn into
//   real calls.
Return Value: 1 when the label table fails to load, else 0.
*/
int __cdecl system_init() {
    _mkdir("saves");
    _mkdir("saves\\auto");
    _mkdir("maps");
    _mkdir("scenarios");
    WorldClimateSkipTerrainClear = 1;
    log_set_state(0);
    set_language(prefs_get("Language", 0, 0));
    textcolor_init();
    StringTable.init(0xC320);
    if (labels_init() != 0) {
        return 1;
    }

    // The Cancel caption, label 102: upper-cased unless the language is 2.
    LPSTR *labels = Labels.strings_ptr;
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(labels[102]));
    LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
    if (Language != 2) {
        string_upper(StringTemp);
    }
    BasePop::set_def_cancel_text(StringTemp);

    // The OK caption, label 101: upper-cased whatever the language.
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(101));
    string_upper(StringTemp);
    BasePop::set_def_ok_text(StringTemp);

    load_fixed_sprites();
    config_popups();

    // Construct only slots 1..7, publishing each after construction. Keep
    // the genuine new-expression so failed construction releases its storage;
    // slot zero is published separately below as the existing Console.
    for (int slot = 1; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWinTable[slot] = new MapWin;
    }

    MapWinTable[0] = &ConsoleGlobal;
    SystemInterfaceFlags &= ~0x8C0u;

    MainInterfaceGlobal.init(0);
    g_BattleWin.init();
    ConsoleGlobal.init(0);

    // The language performs the Console-to-Win virtual-base conversion.
    Win *console_window = &ConsoleGlobal;
    WinBubbleWindow.init(&NetDaemonNet, console_window, ShadowColorTable);

    InterfaceMessageWin.init();
    DatalinkMain.init();
    bung_momma();
    return 0;
}


// ORIGINAL: 0x0058F250 ?system_close@@YAXXZ 0x0058F250-0x0058F2E6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:edbf268ff962712fadc389b8283e4c795b6d27a2:src/unrecovered/0058f250.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json", "Pinned58F27B..58F281 and58F2C4..58F2D9: virtual slot0 with deleting flag1, followed by clearing the respective global slot.", "init_opening and system_init allocate GraphicWin and secondary MapWin owners with new; Win/GraphicWin/MapWin virtual destructors and the separate close_opening path establish typed destruction/free semantics.", "recovery/tasks/game-shutdown-owner-oct26dn.json", "recovery/tests/test_system_close_actual_source.py"], "reason": "Recover system shutdown through real heap-owned GraphicWin and MapWin lifetimes: delete the opening window and secondary MapWin slots1..7, preserving each null guard and shared-slot visibility until destruction/free returns. Keep primary slot0 and the original sound, parent, window, Paths and map cleanup order. This replaces the prior mistaken show(1) translation; source recovery alone does not establish current byte exactness or game acceptance."}
void __cdecl system_close() {
    release_sound();
    close_opening();

    TutWinDesWindow.close();
    WinBubbleWindow.close();
    ConsoleGlobal.close();
    Paths.shutdown();
    map_shutdown();

    for (int slot = 1; slot < 8; ++slot) {
        if (MapWinTable[slot] != 0) {
            delete MapWinTable[slot];
            MapWinTable[slot] = 0;
        }
    }
}


/*
Purpose: Bring a game up: reseed both random chains off the clock, then either
         just turn the log off (mode 0) or reload the string table and
         re-read the two default button captions before the art and the rules
         load. mode 0 is the first boot, anything else a reload into a live
         system.
// ORIGINAL: 0x0058F2F0 ?game_init@@YAHHH@Z 0x0058F2F0-0x0058F421 BYTE_EXACT
// size      305 bytes
// prototype BOOL (__cdecl ?game_init@@YAHHH@Z)(BOOL tglParseStrings, BOOL tglAllRules)
// callers   4   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00455EB0 0x0050F3D0 0x005873C0 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x006257E0 0x00626450 0x00645470
// indirect  0x0058F304 0x0058F30F 0x0058F38D 0x0058F3CD
// LEVER: the clock slot is read ONCE into a register and called twice, the
//   same shape control_game's GetAsyncKeyState lever records - holding
//   WaveTimeGetTimeSlot in a local is what keeps the load from happening
//   twice.
// LEVER: the `add esp,4` at 0x58F3D9 is one instruction serving BOTH arms -
//   log_set_state's argument on the mode==0 arm, set_def_ok_text's on the
//   other - so the source converges the two arms into one `if/else` whose
//   join the compiler cleans once.
// TRIED: naming the three caption-icon stores through sprite.cpp's array -
//   sprite.h does not declare it, and game.h cannot add a member to a header
//   it does not own, so the three store their addresses (see WinCloseIcon in
//   game.h for the identification).
// TRIED: the two caption builds spelled `LPSTR *labels` and
//   `StringTable.get(labels[n])` instead of label_get - byte-identical output,
//   so it is a free spelling, not a lever. The six runs that remain are the
//   register the label value lands in (image eax, ours ecx on the first
//   caption), the register holding `Labels.strings_ptr` for the second caption
//   (image ecx, ours edx), and the register `reload` is read into for
//   read_rules (image edx, ours ecx). Register naming only; the instruction
//   count and the call set agree exactly.
Return Value: 1 when the label table or the rules fail to load, else 0.
Status: homed from src/unrecovered/0058f2f0.cpp on this pass; its pending
        forwarder is gone. 74/82 agreeing, best set /O2 /Gy /GR- /Oy- /GX,
        156 bytes discounted as relocations.
*/
int __cdecl game_init(int mode, int reload) {
    DWORD (WINAPI *const time_get_time)() = timeGetTime;
    WorldClimateSkipTerrainClear = 1;
    my_srand(time_get_time());
    random_reseed(time_get_time());

    if (mode == 0) {
        log_set_state(0);
    } else {
        StringTable.init(0xC320);
        if (labels_init() != 0) {
            return 1;
        }
        // The same two captions system_init builds, for the same two buttons.
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(102));
        LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
        if (Language != 2) {
            string_upper(StringTemp);
        }
        BasePop::set_def_cancel_text(StringTemp);

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(101));
        string_upper(StringTemp);
        BasePop::set_def_ok_text(StringTemp);
    }

    load_sprites();
    if (read_rules(reload) != 0) {
        return 1;
    }
    start_timers();

    // The three caption-button icons: sprite.cpp's array at 0x0075AE20, a 0x2c
    // stride, so these are its [1], [2] and [0]. WinRestoreIcon, the slot
    // between the last two, is not set here.
    WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1];
    WinMaximizeIcon = &g_SILVER_MENU_ICON_SPRITES[2];
    WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0];
    return 0;
}


/*
Purpose: Shut the game state down: timers, label strings, then the string
         table. The mode parameter is read by none of it.
// ORIGINAL: 0x0058F430 ?game_close@@YAXH@Z 0x0058F430-0x0058F444 FILE BYTE_EXACT
// size      20 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0050F440 0x006169D0
// LEVER: byte-exact on promotion. StringTable is strings.h's header binding,
//   so the receiver folds to `mov ecx, 0x9b90d8`; the image's closing `jmp`
//   into Strings::shutdown comes free from the call being the last statement
//   of a void function, and the discarded int result costs nothing.
// Historical LEVER (2026-09-03): temporary stop_timers_call and
//   labels_shutdown_call forwarders once restored these call boundaries.
Return Value: n/a
Status: Complete
*/
void __cdecl game_close(int mode) {
    stop_timers();
    labels_shutdown();
    StringTable.shutdown();
}


/*
Purpose: Restart the game state: tear the desktop down (mode-gated, the same
         closer sequence desktop_close runs), stop the timers, drop the label
         and string tables, re-init the game, and re-init the desktop when the
         caller asked for it. Always clears the climate skip flag.
// ORIGINAL: 0x0058F450 ?game_reload@@YAHHH@Z 0x0058F450-0x0058F4E7 BYTE_EXACT
// LEVER (2026-09-03, pair normalisation): the same two forwarders game_close
//   uses - the image CALLS 0x0050F440 and 0x006169D0 in this body too, and
//   with the `MEASURED inline` helpers folded in it was 107 compiled
//   instructions against the image's 44, 12 of 44. 44 of 44 after the switch.
// LEVER: byte-exact on promotion from the 0058f450 archive unit. The
//   closer block is desktop_close's own sequence spelled the same way, so
//   every receiver folds to its `mov ecx, imm32` and every call lands on the
//   image's E8 target; `mode` is read into esi once at entry (the image's
//   `mov esi, [ebp + 8]`) because the source uses it twice - the opening
//   guard and the desktop_init guard - exactly as the image does.
// LEVER (2026-11-16, TU merge): with desktop_close and game_close defined in
//   this unit, the hand-written closer and shutdown sequences are plain
//   calls again; /Ob2 expands both here and game_reload in top_menu and
//   multiplayer_init, all exact.
// size      151 bytes
// prototype int (__cdecl ?game_reload@@YAHHH@Z)(int, BOOL tglAllRules)
// callers   2   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x0050F440 0x0058EE60 0x0058F2F0 0x005D4E40 0x005EDCD0 0x00616950 0x006169D0
Return Value: 1 when game_init fails, else 0.
Status: Complete
*/
int __cdecl game_reload(int mode, int reload) {
    if (mode != 0) {
        desktop_close();
    }
    game_close(mode);
    if (game_init(1, reload) != 0) {
        return 1;
    }
    if (mode != 0) {
        desktop_init(1);
    }
    WorldClimateSkipTerrainClear = 0;
    return 0;
}


// ORIGINAL: 0x0058F4F0 sub_58f4f0 0x0058F4F0-0x0058F609 FILE BYTE_EXACT
// symbol    ?sub_58f4f0@@YAPADPADHHH@Z
// LEVER: string-routines-are-calls the body was already right; the scaffold was compiling a string routine the image CALLS into an `/Oi` inline expansion. What was written down here as a tried source form - "VC6 lowers `local_104[0]=0;` as `and byte ptr [buf],0` rather than `mov`, and each branch shares one `lea` instead of recomputing it, both compiler instruction-selection choices at /O1 that plain restatement did not move" - was that expansion changing the register pressure around them. The if/else-if chain matching Ghidra's structure is the right shape and always was.
// working copy - scaffold materialised by --work
// size      281 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00625880 0x00645470


char * __cdecl sub_58f4f0(char *output, int faction_id, int plurality, int gender) {
    char parsed[256];
    char buffer[256];

    if (plurality < 0) {
        plurality = PluralityDefault;
    }
    if (gender < 0) {
        gender = GenderDefault;
    }
    buffer[0] = 0;
// RECOVERY_LITERAL: v1 0x0068FA70 0x0068FA73 $SG66146 0x0058F4F0
    strcat(buffer, "$<");
    if (gender == 0) {
// RECOVERY_LITERAL: v1 0x0068FA74 0x0068FA76 $SG66148 0x0058F4F0
        strcat(buffer, "M");
    } else if (gender == 1) {
// RECOVERY_LITERAL: v1 0x0068FA78 0x0068FA7A $SG66151 0x0058F4F0
        strcat(buffer, "F");
    } else if (gender == 2) {
// RECOVERY_LITERAL: v1 0x0068FA7C 0x0068FA7E $SG66154 0x0058F4F0
        strcat(buffer, "N");
    }
    if (plurality == 0) {
// RECOVERY_LITERAL: v1 0x0068FA80 0x0068FA82 $SG66156 0x0058F4F0
        strcat(buffer, "1");
    } else if (plurality == 1) {
// RECOVERY_LITERAL: v1 0x0068FA84 0x0068FA86 $SG66159 0x0058F4F0
        strcat(buffer, "2");
    }
// RECOVERY_LITERAL: v1 0x0068FA88 0x0068FA8A $SG66160 0x0058F4F0
    strcat(buffer, ":");
    strcat(buffer, Players[faction_id].adj_name_faction);
// RECOVERY_LITERAL: v1 0x0068FA8C 0x0068FA8E $SG66161 0x0058F4F0
    strcat(buffer, ">");
    parse_string(buffer, parsed);
    return strcat(output, parsed);
}


// RECOVERY_LITERAL: v1 0x0068FAAC 0x0068FAAE $SG66186 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FAA0 0x0068FAA2 $SG66181 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA98 0x0068FA9A $SG66175 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA9C 0x0068FA9E $SG66178 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA94 0x0068FA96 $SG66172 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FAA8 0x0068FAAA $SG66185 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA90 0x0068FA93 $SG66170 0x0058F610
// ORIGINAL: 0x0058F610 ?say_special@@YAXPADPADH@Z 0x0058F610-0x0058F6F6 BYTE_EXACT
// LEVER: the eight fixed-address bindings are .rdata STRING LITERALS, read
//         out of the image with image_data.py and spelled as the text, the
//         same way netdaemon.cpp's captions went: 0x0068FA90 "$<",
//         0x0068FA94 "M", 0x0068FA98 "F", 0x0068FA9C "N", 0x0068FAA0 "1",
//         0x0068FAA8 ":" and 0x0068FAAC ">". 0x0068FAA4 "2" is not read by
//         this body and is left for whoever homes its caller. The eight
//         together are the markup the parser consumes: a faction noun of
//         M/F/N, a noun form digit, then the substituted text.
// LEVER: the ninth binding is `Players` (faction.h, 0x00946A50); the image
//         indexes it `a3 * 0x167` dwords, which is `Players[a3]` - the
//         stride IS sizeof(Player).
// symbol   ?say_special@@YAXPAD0H@Z
//         - this tree's name for the same (char *, char *, int) signature.
//         VC6 folds the two identical `char *` parameters into a
//         back-reference, where the catalogued name spells the second `PAD`
//         longhand; `measure` matches by address, and this fact is what lets
//         it resolve the symbol.
void __cdecl say_special(char *output, char *text, int faction_id) {
    char buffer[256];
    char parsed[256];

    buffer[0] = 0;
    int noun = Players[faction_id].is_leader_female;
    strcat(buffer, "$<");

    if (noun == 0) {
        strcat(buffer, "M");
    } else if (noun == 1) {
        strcat(buffer, "F");
    } else if (noun == 2) {
        strcat(buffer, "N");
    }

    // 0x0068FAA4 "2" is an unreferenced literal between "1" and ":": the
    // plural arm of the noun-form test, folded away for the constant form.
    int plural = 0;
    if (plural == 0) {
        strcat(buffer, "1");
    } else if (plural == 1) {
        strcat(buffer, "2");
    }
    strcat(buffer, ":");
    strcat(buffer, text);
    strcat(buffer, ">");

    parse_string(buffer, parsed);
    strcat(output, parsed);
}


// ORIGINAL: 0x0058F700 ?say_fac_special@@YAXPADPADH@Z 0x0058F700-0x0058F80A FILE BYTE_EXACT
// symbol    ?say_fac_special@@YAXPAD0H@Z
// size      266 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00625880 0x00645470
// PRESERVED UNIT - measured BYTE_EXACT.
//
// The following PRESERVED UNIT / GENERATED SKELETON prose records an earlier
// address-bound verification artifact, not current product-source placement.
// Keep its historical compiler and measurement findings intact.
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058F700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_fac_special@@YAXPADPADH@Z  at 0x0058F700  (266 bytes)
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



void __cdecl say_fac_special(char *output, char *text, int faction_id) {
    char buffer[256];
    char parsed[256];

    int plurality = Players[faction_id].is_noun_plural;
    int gender = Players[faction_id].noun_gender;

    buffer[0] = 0;
// RECOVERY_LITERAL: v1 0x0068FAB0 0x0068FAB3 $SG66196 0x0058F700
    strcat(buffer, "$<");

    if (gender == 0) {
// RECOVERY_LITERAL: v1 0x0068FAB4 0x0068FAB6 $SG66198 0x0058F700
        strcat(buffer, "M");
    } else if (gender == 1) {
// RECOVERY_LITERAL: v1 0x0068FAB8 0x0068FABA $SG66201 0x0058F700
        strcat(buffer, "F");
    } else if (gender == 2) {
// RECOVERY_LITERAL: v1 0x0068FABC 0x0068FABE $SG66204 0x0058F700
        strcat(buffer, "N");
    }

    if (plurality == 0) {
// RECOVERY_LITERAL: v1 0x0068FAC0 0x0068FAC2 $SG66206 0x0058F700
        strcat(buffer, "1");
    } else if (plurality == 1) {
// RECOVERY_LITERAL: v1 0x0068FAC4 0x0068FAC6 $SG66209 0x0058F700
        strcat(buffer, "2");
    }

// RECOVERY_LITERAL: v1 0x0068FAC8 0x0068FACA $SG66210 0x0058F700
    strcat(buffer, ":");
    strcat(buffer, text);
// RECOVERY_LITERAL: v1 0x0068FACC 0x0068FACE $SG66211 0x0058F700
    strcat(buffer, ">");

    parse_string(buffer, parsed);
    strcat(output, parsed);
}


// ORIGINAL: 0x0058F810 ?get_phrase@@YAPADHH@Z 0x0058F810-0x0058F8A2 BYTE_EXACT
// LEVER: the faction-table bases at 0x00946A54 and 0x00946A6C are
//         `Players[a1].filename` and `Players[a1].search_key` - the image
//         computes `a1 * 0x59c` and adds those two constants, and 0x00946A50
//         (Players) plus 4 and plus 0x1c are exactly those two fields of
//         faction.h's Player.
// LEVER: 0x0068FAD0 is the .rdata literal "PLAYERFILE" (image_data.py), the
//         popup caption for the section that failed to open.
// LEVER: 0x009B7D00 is text.h's `TextBufferGetPtr`, already modelled; the
//         image returns the POINTER stored there (`mov eax, [0x9b7d00]`), so
//         the binding is a real global, not a buffer.
char *__cdecl get_phrase(int faction_id, int phrase_id) {
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;
    int ok = text_open(filename, search_key);
    if (ok != 0) {
        ok = text_open(filename, filename);
        if (ok != 0) {
            parse_says(0, search_key, -1, -1);
            parse_says(1, filename, -1, -1);
// RECOVERY_LITERAL: v1 0x0068FAD0 0x0068FADB $SG66221 0x0058F810
            X_pop("PLAYERFILE", 0);
            return StringTable.get(0);
        }
    }
    int remaining = phrase_id + 9;
    while (remaining > 0) {
        text_get();
        --remaining;
    }
    // TRIED: the counter as one local driving both the guard and the loop
    //         (`int n = phrase_id + 9; if (n > 0) { do { text_get(); n--; }`)
    //         scores 46 of 58 - VC6 folds the add into `lea esi,[eax+9]` and
    //         tests esi, where the image tests eax and then copies it to esi
    //         for the loop. Modifying the parameter instead (`phrase_id =
    //         phrase_id + 9;` with a separate `int i`) moves the whole test
    //         to esi too: 45 of 58. The image's shape needs the guard value
    //         and the counter in DIFFERENT registers, which VC6 merges under
    //         both spellings.
    return TextBufferGetPtr;
}


// ORIGINAL: 0x0058F8B0 ?get_noun_phrase@@YAPADHH@Z 0x0058F8B0-0x0058F9C9 BYTE_EXACT
// LEVER: same three bindings as get_phrase - Players' filename/search_key at
//         0x00946A54/0x00946A6C, the "PLAYERFILE" caption at 0x0068FADC, and
//         text.h's TextBufferGetPtr at 0x009B7D00.
// BUG IN THE ARCHIVED TRANSCRIPTION, not in the shipped image: the artifact
//         returned the ADDRESS of the TextBufferGetPtr variable rather than
//         the pointer it holds. The image loads `[0x9b7d00]` into eax twice,
//         once as strcpy's destination and once as the return value, so it
//         returns the buffer. Fixed here; the two loads are kept separate on
//         purpose - caching one local would delete the second
//         `mov eax, [0x9b7d00]`.
char *__cdecl get_noun_phrase(int faction_id, int phrase_id) {
    char buffer[256];
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;

    if ((text_open(filename, search_key) != 0)
        && (text_open(filename, filename) != 0)) {
        parse_says(0, search_key, -1, -1);
        parse_says(1, filename, -1, -1);
// RECOVERY_LITERAL: v1 0x0068FADC 0x0068FAE7 $SG66234 0x0058F8B0
        X_pop("PLAYERFILE", 0);
        return StringTable.get(0);
    }

    for (int i = 0; i < phrase_id + 9; i++) {
        text_get();
    }
    strcpy(buffer, text_item());
    char *item = text_item();
    char c = item[0];
    int number = 0;
    int gender = 0;
    if (c == 'M' || c == 'm') {
        gender = 0;
    }
    if (c == 'F' || c == 'f') {
        gender = 1;
    }
    if (c == 'N' || c == 'n') {
        gender = 2;
    }
    if (item[1] == '1') {
        number = 0;
    }
    if (item[1] == '2') {
        number = 1;
    }
    parse_set(gender, number);
    strcpy(TextBufferGetPtr, buffer);
    return TextBufferGetPtr;
}


// ORIGINAL: 0x0058F9D0 ?get_pact@@YAPADH@Z 0x0058F9D0-0x0058FA19 BYTE_EXACT
// MISMATCH #3 'xor' vs 'mov': the original zeroes edx BEFORE storing
// PluralityDefault = 0, and every source order tried compiles that store
// first. A second divergence follows: the original keeps `add edx, 0xc9` as
// its own instruction before `mov ecx, [eax+edx*4]` where every spelling
// tried folds the 0xc9 into the addressing displacement. Tried: inline
// expression, a named boolean `flag`, a named `off` combining the constant
// with the flag, and a `char *` base with a manual `*4` multiply - all four
// byte-identical at the divergence, so it reads as an optimiser
// canonicalisation this call shape always hits.
// TRIED: homed out of the archive once its five fixed-address bindings were
//         all nameable from models the tree already holds (see get_pacts).
char *__cdecl get_pact(int faction_id) {
    PluralityDefault = 0;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    return StringTable.get(Labels.strings_ptr[noun != 0 ? 0xca : 0xc9]);
}


// ORIGINAL: 0x0058FA20 ?get_pacts@@YAPADH@Z 0x0058FA20-0x0058FA69 BYTE_EXACT
// LEVER: the carried bindings renamed to the tree's own models - Players
//         (faction.h, 0x00946A50), Labels (alpha.h, 0x009B90F8), StringTable
//         (strings.h, 0x009B90D8) and general.h's noun defaults - so no
//         anonymous g_ binding arrived with the body.
// TRIED: as `Players[a1].is_leader_female` for the load; the image reads it
//         as `[players + a1*0x59c]`, so the member form is the same operand.
// MISMATCH #10 'mov' vs 'mov': the image keeps `add edx, 0xcb` as its own
//         instruction before `mov ecx, [eax+edx*4]`, where every spelling
//         tried folds the 0xcb into the addressing displacement
//         (`[ecx+edx*4+0x32c]`) and hoists the `Labels.strings_ptr` load
//         above the store. Tried (as 0x0058F9D0, four spellings): inline
//         expression, a named boolean, a named `off` combining the constant
//         with the flag, and a `char *` base with a manual `*4` multiply -
//         all four byte-identical at this divergence.
char *__cdecl get_pacts(int faction_id) {
    PluralityDefault = 1;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    return StringTable.get(Labels.strings_ptr[noun != 0 ? 0xcc : 0xcb]);
}


// ORIGINAL: 0x0058FA70 ?get_pacts@@YAPADHH@Z 0x0058FA70-0x0058FAFA BYTE_EXACT
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// LEVER: the three fixed-address bindings renamed to the tree's own models -
//         Players (faction.h), Labels (alpha.h) and StringTable (strings.h), so
//         no anonymous g_ binding arrived with the body. 0x009BBFEC is
//         GenderDefault and 0x009BBFF0 is PluralityDefault (general.h).
// LEVER: `int flag; if (a && b) flag = 1; else flag = 0;` - the two else arms
//         SHARE one `xor edx,edx` and the true arm jumps over it
//         (`mov edx,1 / jmp / xor edx,edx`). The ternary and the bare `&&`
//         both compile the other way (`jne` straight to the store).
// TRIED: `code` as a ternary, as a nested ternary and with `labels` declared
//         ahead of it - all three move `mine` off ECX (22/43, 4/43, 12/43)
//         against 27/43 for the plain `int code = 0xcb; if (...) code = 0xcc;`.
//         What is left is register ASSIGNMENT: the image keeps `code` in EAX
//         (reusing the dead `Players[other_id]` load) and `labels` in EDX, and
//         hoists the third `test ecx,ecx` above the two global stores.

char *__cdecl get_pacts(int faction_id, int other_id) {
    int mine = Players[faction_id].is_leader_female;
    int flag;
    if (mine != 0 && Players[other_id].is_leader_female != 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    GenderDefault = flag;
    PluralityDefault = 1;
    return label_get((mine != 0 && Players[other_id].is_leader_female != 0) ? 0xcc : 0xcb);
}


// ORIGINAL: 0x0058FB00 ?get_pact_hood@@YAPADHH@Z 0x0058FB00-0x0058FB61 BYTE_EXACT
// LEVER: the five fixed-address bindings renamed to the tree's own models - see
//         get_pacts above for the address map. No anonymous g_ binding arrived.
// TRIED: `int code = 0xcd; if (...) code = 0xce;`, the ternary form, and a
//         named `other_female` local for the second read - all three score the
//         same 22/29 as the shared-else `if/else`. The image has no upfront
//         constant store (both arms materialise into EAX), which the
//         shared-else form is the only one to reproduce. What is left is
//         register ASSIGNMENT: the image reuses the dead `Players[other_id]`
//         register in EAX for `code`, this tree puts it in ECX.

char *__cdecl get_pact_hood(int faction_id, int other_id) {
    GenderDefault = 0;
    PluralityDefault = 0;
    int code;
    if (Players[faction_id].is_leader_female != 0
        && Players[other_id].is_leader_female != 0) {
        code = 0xce;
    } else {
        code = 0xcd;
    }
    return label_get(code);
}


// ORIGINAL: 0x0058FB70 sub_58fb70 0x0058FB70-0x0058FBB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/0058fb70.cpp", "task:owned-helper-coverage-batch", "Native58FB70 mode0 returns unchanged, mode1 passes a two-byte terminated buffer and writes the first byte back; other nonzero modes call USER32 CharUpperA on the whole text. No mapped-caller claim."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?uppercase_text@@YAPADPADH@Z
char *__cdecl uppercase_text(char *text, int mode) {
    if (mode == 0) {
        return text;
    }
    if (mode == 1) {
        char first_character[2];
        first_character[0] = *text;
        first_character[1] = '\0';
        CharUpperA(first_character);
        *text = first_character[0];
        return text;
    }
    CharUpperA(text);
    return text;
}


// ORIGINAL: 0x0058FBC0 ?get_his_her@@YAPADHH@Z 0x0058FBC0-0x0058FC4B BYTE_EXACT
// LEVER: the fixed-address bindings renamed to the tree's own models - Players
//         (faction.h), Labels (alpha.h), StringTable (strings.h), the CharUpperA
//         import slot (windows.h) and 0x00945838 as PronounBuffer below. The
//         use-site census names that buffer: get_his_her (this body),
//         get_him_her (0x0058FC50) and get_he_she (0x0058FCE0) are the only
//         three sites in the image that touch it, and each strcpy's its fetched
//         pronoun into it and hands back its address.
// TRIED: `LPSTR *labels` read inline in the call instead of as a named local -
//         20/42 against 21/42, so the local stays. Naming the flag `female`
//         first, swapping the `+` operands, and the `(x != 0) + 0xcf` form all
//         score the same 21/42 as what is committed. What is left is register
//         ASSIGNMENT: the image zeroes the `0xcf +` accumulator into EDX before
//         the index chain and keeps `Labels.strings_ptr` in EAX; this tree puts
//         the accumulator in EAX and the label table in ECX.

char *__cdecl get_his_her(int faction_id, int sentence_form) {
    strcpy(PronounBuffer, StringTable.get(Labels.strings_ptr[Players[faction_id].is_leader_female != 0 ? 0xd0 : 0xcf]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}


/*
Purpose: Fetch the "him/her" pronoun into the shared game pronoun buffer,
         optionally upper-casing it, beside the sibling get_his_her body.
// ORIGINAL: 0x0058FC50 ?get_him_her@@YAPADHH@Z 0x0058FC50-0x0058FCDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original58FC50..58FCDB; retired seed git:da8a4b7f:src/unrecovered/0058fc50.cpp records the same identity and spans.", "The fixed-address bindings rename to the tree's own models exactly as game.cpp's sibling get_his_her (0x0058FBC0) already does: Players (faction.h, the 0x167-stride first word is is_leader_female), Labels (alpha.h), StringTable (strings.h), PronounBuffer (game.h, 0x00945838) and the CharUpperA import the header supplies.", "Label code 0xC7/0xC8 and the sentence-form handling are the seed's measured behaviour."], "reason": "Home the sibling pronoun fetcher beside its shared buffer and get_his_her in the game source."}
Return Value: PronounBuffer
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_him_her(int faction_id, int sentence_form) {
    strcpy(PronounBuffer, StringTable.get(Labels.strings_ptr[Players[faction_id].is_leader_female != 0 ? 0xC8 : 0xC7]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}


/*
Purpose: The third pronoun fetcher beside game.cpp's get_his_her/get_him_her:
         pick the he/she label by the faction leader's gender, copy the string
         table's text into the shared pronoun buffer, and uppercase it in
         place when the sentence form asks.

// ORIGINAL: 0x0058FCE0 ?get_he_she@@YAPADHH@Z 0x0058FCE0-0x0058FD6B FILE BYTE_EXACT
// size      139 bytes
// prototype char* (__cdecl ?get_he_she@@YAPADHH@Z)(int, int)
// callers   6   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645460
// indirect  0x0058FD43 0x0058FD5E
// TRIED (seed): caching the CharUpperA pointer in a local vs calling through
//         the import slot at each site - both land 18 bytes over (157-160 vs
//         139), extra push/push in the prologue before the a1*0x167 multiply;
//         root cause not found. The fixed-address bindings are renamed to the
//         tree's own models: Players (faction.h), Labels (alpha.h),
//         StringTable (strings.h), the CharUpperA import slot (windows.h) and
//         0x00945838 as PronounBuffer (game.h) - the same census-named buffer
//         get_his_her and get_him_her fill.
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_he_she(int faction_id, int sentence_form) {
    strcpy(PronounBuffer, StringTable.get(Labels.strings_ptr[Players[faction_id].is_leader_female != 0 ? 0xd2 : 0xd1]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}


