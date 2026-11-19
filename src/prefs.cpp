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

// Shared separator objects the image pushes instead of unit literals.
extern const char BufferJoinSpace[];

extern char SocialWinCommaSpaceText[];

// RECOVERY_OBJECT: v1 0x00690110 _alpha_primary_laptop_key
static const char alpha_primary_laptop_key[] = "Laptop";

// RECOVERY_OBJECT: v1 0x00690118 _alpha_secondary_laptop_key
static const char alpha_secondary_laptop_key[] = "Laptop";

// RECOVERY_OBJECT: v1 0x006900FC _alpha_profile_default_zero
static const char alpha_profile_default_zero[] = "0";

// RECOVERY_OBJECT: v1 0x00690204 _alpha_binary_output_zero
static const char alpha_binary_output_zero[] = "0";

// RECOVERY_OBJECT: v1 0x00690200 _alpha_binary_output_one
static const char alpha_binary_output_one[] = "1";

// RECOVERY_OBJECT: v1 0x00690100 _alpha_profile_format_key
static const char alpha_profile_format_key[] = "Prefs Format";

// RECOVERY_OBJECT: v1 0x00690120 _alpha_faction_prefix
static const char alpha_faction_prefix[] = "Faction";

// The profile APIs are SDK imports, not recovered bodies: prefs_get (both
// overloads), prefs_fac_load, prefs_load and both prefs_put overloads reach
// them through import slots (`call dword ptr [0x669104]` at 0x0059E527 in
// prefs_put, esi loaded from 0x669108 at 0x0059D98C in prefs_get). The
// identities come from the original import directory; the pinned image
// supplies each slot address, never a copied operand.
// RECOVERY_IMPORT: v1 __imp__GetPrivateProfileStringA@24 KERNEL32.dll name:GetPrivateProfileStringA
// RECOVERY_IMPORT: v1 __imp__WritePrivateProfileStringA@16 KERNEL32.dll name:WritePrivateProfileStringA
// THE INI STRINGS ARE GLOBALS, not literals at each call site. The image
// loads them - `mov ecx, [0x006900CC]; push ecx` - where a literal would
// compile to `push offset`, which is two bytes shorter per use and shifts
// every branch after it. Six call sites in this file pass them.
// prefs_put (0x0059E510) loads both variables before its four pushes
// (`mov eax, dword ptr [0x6900CC]` / `mov eax, dword ptr [0x6900D0]`);
// the DWORDs at 0x006900CC/0x006900D0 point at the ".\Alpha Centauri.Ini"
// and "Alpha Centauri" literals.
// RECOVERY_OBJECT: v1 0x006900CC ?PrefsFile@@3PBDB
const char *PrefsFile = ".\\Alpha Centauri.Ini";   // 0x006900CC

// RECOVERY_OBJECT: v1 0x006900D0 ?PrefsSection@@3PBDB
const char *PrefsSection = "Alpha Centauri";       // 0x006900D0

// prefs_load (0x0059DCF0) stores every field in place - `mov dword ptr
// [0x94B464], eax` for preferences through custom_world at 0x0094B480
// (bounded by the `cmp esi, 0x94B49C` loop tail at 0x0059E4D9).
// Extent: compiled sizeof 0x38; prefs_load loop ends at `cmp esi, 0x94B49C` (0x0059E4D9).
// RECOVERY_OBJECT: v2 0x0094B464 0x0094B49C ?AlphaIniPrefs@@3UAlphaIniPref@@A 0x0047D4A4
AlphaIniPref AlphaIniPrefs;                         // 0x0094B464

// Extent: compiled sizeof 0x14; its last field top_menu (0x0094B360) is
// stored by prefs_load at 0x0059DE8E and read by map_menu/top_menu.
// RECOVERY_OBJECT: v2 0x0094B350 0x0094B364 ?DefaultPrefs@@3UDefaultPref@@A 0x0059DE8E
DefaultPref DefaultPrefs;                           // 0x0094B350

// WORKAROUND(noinline): expanded twin of prefs_get(LPCSTR, int, BOOL); see
// the out-of-line definition below.
inline int __cdecl prefs_get_inline(LPCSTR key_name, int default_value, BOOL use_default) {
    StringTemp[0] = 0;
    say_num(default_value);
    if (use_default) {
        strcpy(TextBufferGetPtr, StringTemp);
    } else {
        GetPrivateProfileStringA(PrefsSection, key_name, StringTemp,
            TextBufferGetPtr, 256, PrefsFile);
    }
    return atoi(Txt.update());
}

// Each binary preference default in pinned prefs_load 0x0059DCF0 has its own
// "1"/"0" literal pair (0x00690160/64, 0x00690174/78, 0x006901B4/B8,
// 0x006901C4/C8), so the bit loop is written out at every site rather than
// being one inline function; a macro keeps the per-site spelling. It clears
// StringTemp after `non_pad` is zeroed, which is why the image stores the
// clear from the register holding that zero (`mov [StringTemp], bl`).
#define PREFS_GET_BINARY(value) \
    { \
        int non_pad = 0; \
        StringTemp[0] = 0; \
        for (int shift = 31; shift >= 0; shift--) { \
            if ((1 << shift) & (value)) { \
                non_pad = 1; \
                strcat(StringTemp, "1"); \
            } else if (non_pad || shift < 8) { \
                strcat(StringTemp, "0"); \
            } \
        } \
    }

// WORKAROUND(noinline): prefs_load expands default_prefs/default_prefs2 but
// still calls prefs_get 0x0059DB40 for their "Laptop" queries (0x0059DF17,
// 0x0059E016), while the standalone bodies expand it. These twins call the
// out-of-line overload.
static inline uint32_t default_prefs_load() {
    uint32_t base_prefs = PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK | PREF_AUTO_FORMER_BUILD_ADV
        | PREF_AUTO_FORMER_PLANT_FORESTS | PREF_AUTO_END_MOVE_SPOT_VEH_WAR
        | PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE | PREF_AUTO_END_MOVE_SPOT_VEH_TREATY
        | PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG | PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH
        | PREF_BSC_AUTO_DESIGN_VEH | PREF_BSC_MOUSE_EDGE_SCROLL_VIEW | PREF_AV_BACKGROUND_MUSIC
        | PREF_AV_SOUND_EFFECTS | PREF_MAP_SHOW_GRID | PREF_UNK_10
        | PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH | PREF_BSC_AUTOSAVE_EACH_TURN
        | PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND;
    return prefs_get(alpha_primary_laptop_key, 0, false) ? base_prefs : base_prefs
        | PREF_AV_SECRET_PROJECT_MOVIES | PREF_AV_SLIDING_WINDOWS | PREF_AV_MAP_ANIMATIONS;
}

static inline uint32_t default_prefs2_load() {
    uint32_t base_prefs2 = MPREF_ADV_DETAIL_MAIN_MENUS | MPREF_BSC_AUTO_PRUNE_OBS_VEH
        | MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP | MPREF_AV_VOICEOVER_TECH_FAC
        | MPREF_MAP_SHOW_BASE_NAMES | MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES
        | MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU | MPREF_ADV_QUICK_MOVE_VEH_ORDERS
        | MPREF_AUTO_FORMER_BUILD_SENSORS | MPREF_AUTO_FORMER_REMOVE_FUNGUS;
    return prefs_get(alpha_secondary_laptop_key, 0, false) ? base_prefs2 : base_prefs2 | MPREF_AV_SLIDING_SCROLLBARS;
}

/*
Purpose: Attempt to read the setting's value from the ini file.
// ORIGINAL: 0x0059D980 ?prefs_get@@YAPADPADPADH@Z 0x0059D980-0x0059DA19 BYTE_EXACT
// symbol    ?prefs_get@@YAPADPBD0H@Z
// LEVER: WRONG CALLEE - `strcpy_s(TextBufferGetPtr, 256, default_value)`
//        pushed a 3rd argument the image never does; `add esp, 8` after
//        the call (2 args) confirms a plain `strcpy`.
// Historical emission experiments mistakenly classified this overload as
// uncalled. prefs_load uses it for Semaphore, Custom World and the binary
// preference defaults. Those are real source callers; absence from a linked
// symbol list does not establish absence from a compiled object.
// size      153 bytes
// prototype int8* (__cdecl ?prefs_get@@YAPADPADPADH@Z)(int8* lpKeyName, int8* lpDefault, int)
// callers   4   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645460 0x00645DA7
// indirect  0x0059D9B4 0x0059DA0A
Return Value: Key's string value from the ini or default if not set
Status: Complete
*/
LPSTR __cdecl prefs_get(LPCSTR key_name, LPCSTR default_value, BOOL use_default) {
    if (use_default) {
        strcpy(TextBufferGetPtr, default_value);
    } else {
        GetPrivateProfileStringA(PrefsSection, alpha_profile_format_key, alpha_profile_default_zero,
            TextBufferGetPtr, 256, PrefsFile);
        if (atoi(TextBufferGetPtr) != 12) {
            strcpy(TextBufferGetPtr, default_value);
        } else {
            GetPrivateProfileStringA(PrefsSection, key_name, default_value,
                TextBufferGetPtr, 256, PrefsFile);
        }
    }
    return Txt.update();
}

/*
Purpose: Get the default value for the 1st set of preferences.
// ORIGINAL: 0x0059DA20 ?default_prefs@@YAHXZ 0x0059DA20-0x0059DA99 BYTE_EXACT
// symbol    ?default_prefs@@YAIXZ
// size      121 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00645470 0x00645DA7 0x0064FC88
// indirect  0x0059DA6E
Return Value: Default preferences
Status: Complete
*/
uint32_t __cdecl default_prefs() {
    uint32_t base_prefs = PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK | PREF_AUTO_FORMER_BUILD_ADV
        | PREF_AUTO_FORMER_PLANT_FORESTS | PREF_AUTO_END_MOVE_SPOT_VEH_WAR
        | PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE | PREF_AUTO_END_MOVE_SPOT_VEH_TREATY
        | PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG | PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH
        | PREF_BSC_AUTO_DESIGN_VEH | PREF_BSC_MOUSE_EDGE_SCROLL_VIEW | PREF_AV_BACKGROUND_MUSIC
        | PREF_AV_SOUND_EFFECTS | PREF_MAP_SHOW_GRID | PREF_UNK_10
        | PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH | PREF_BSC_AUTOSAVE_EACH_TURN
        | PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND;
    return prefs_get_inline(alpha_primary_laptop_key, 0, false) ? base_prefs : base_prefs  // 0xA3E1DD16 : 0xBBE1DD96
        | PREF_AV_SECRET_PROJECT_MOVIES | PREF_AV_SLIDING_WINDOWS | PREF_AV_MAP_ANIMATIONS;
}

/*
Purpose: Get the default value for the 2nd set of preferences.
// ORIGINAL: 0x0059DAA0 ?default_prefs2@@YAHXZ 0x0059DAA0-0x0059DB19 BYTE_EXACT
// symbol    ?default_prefs2@@YAIXZ
// size      121 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x00645DA7 0x0064FC88
// indirect  0x0059DAEE
Return Value: Default preferences 2nd set
Status: Complete
*/
uint32_t __cdecl default_prefs2() {
    uint32_t base_prefs2 = MPREF_ADV_DETAIL_MAIN_MENUS | MPREF_BSC_AUTO_PRUNE_OBS_VEH
        | MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP | MPREF_AV_VOICEOVER_TECH_FAC
        | MPREF_MAP_SHOW_BASE_NAMES | MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES
        | MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU | MPREF_ADV_QUICK_MOVE_VEH_ORDERS
        | MPREF_AUTO_FORMER_BUILD_SENSORS | MPREF_AUTO_FORMER_REMOVE_FUNGUS; // 0x327168 : 0x3A7168
    return prefs_get_inline(alpha_secondary_laptop_key, 0, false) ? base_prefs2 : base_prefs2 | MPREF_AV_SLIDING_SCROLLBARS;
}

/*
Purpose: Get the default value for the warning pop-up preferences.
// ORIGINAL: 0x0059DB20 ?default_warn@@YAHXZ 0x0059DB20-0x0059DB26 BYTE_EXACT
// symbol    ?default_warn@@YAHXZ
// size      6 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Default warning preferences
Status: Complete
*/
int __cdecl default_warn() {
    return WARN_STOP_RANDOM_EVENT | WARN_STOP_ENERGY_SHORTAGE | WARN_STOP_MINERAL_SHORTAGE
        | WARN_STOP_STARVATION | WARN_STOP_BUILD_OUT_OF_DATE | WARN_STOP_UNK_100
        | WARN_STOP_NUTRIENT_SHORTAGE | WARN_STOP_GOLDEN_AGE | WARN_STOP_DRONE_RIOTS
        | WARN_STOP_NEW_FAC_BUILT; // 0x3C3A9
}

/*
Purpose: Get the default value for the rule related preferences.
// ORIGINAL: 0x0059DB30 ?default_rules@@YAHXZ 0x0059DB30-0x0059DB36 BYTE_EXACT
// symbol    ?default_rules@@YAIXZ
// size      6 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Default rule preferences
Status: Complete
*/
uint32_t __cdecl default_rules() {
    return RULES_VICTORY_COOPERATIVE | RULES_VICTORY_TRANSCENDENCE | RULES_BLIND_RESEARCH
        | RULES_VICTORY_DIPLOMATIC | RULES_VICTORY_ECONOMIC | RULES_VICTORY_CONQUEST; // 0x1A0E
}

/*
Purpose: Attempt to read the setting's value from the ini file.
// ORIGINAL: 0x0059DB40 ?prefs_get@@YAHPADHH@Z 0x0059DB40-0x0059DBC9 BYTE_EXACT
// symbol    ?prefs_get@@YAHPBDHH@Z
// size      137 bytes
// prototype int (__cdecl ?prefs_get@@YAHPADHH@Z)(LPCSTR keyName, int defaultValue, BOOL useDefault)
// callers   17   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645460 0x00645470 0x00645DA7 0x0064FC88
// indirect  0x0059DBAC
Return Value: Key's integer value from the ini or default if not set
Status: Complete
*/
// WORKAROUND(noinline): pinned prefs_load 0x0059DCF0 calls this overload
// (0x0059DB40) for both "Laptop" queries and "Time Controls" while expanding
// it at Language..Faction and Customize; default_prefs 0x0059DA20 and
// default_prefs2 0x0059DAA0 expand it too. VC6 has no noinline, so the real
// definition stays out of line here and prefs_get_inline is the expanded twin.
#pragma auto_inline(off)
int __cdecl prefs_get(LPCSTR key_name, int default_value, BOOL use_default) {
    StringTemp[0] = 0;
    say_num(default_value);
    if (use_default) {
        strcpy(TextBufferGetPtr, StringTemp);
    } else {
        GetPrivateProfileStringA(PrefsSection, key_name, StringTemp,
            TextBufferGetPtr, 256, PrefsFile);
    }
    return atoi(Txt.update());
}

#pragma auto_inline(on)

/*
Purpose: Read the faction filenames and search for keys from the ini file (SMACX only). This has 
         the added effect of forcing the player's search_key to be set to the filename value. 
         Rewrote almost the entire function because of how terrible the original code logic was.
// ORIGINAL: 0x0059DBD0 ?prefs_fac_load@@YAXXZ 0x0059DBD0-0x0059DCE8 BYTE_EXACT
// LEVER: THE IMAGE'S SHAPE IS ONE LOOP, not the "separate loop for the old ini format" this
//        body had. Reading 0x0059DBD0-0x0059DCE8 off the image: the loop head is the
//        ExpansionEnabled test (`mov eax,[0x9a6488]; test; je <increment>`), so the toggle is
//        re-tested every iteration and a toggle-off pass costs nothing but the increment; the
//        "Faction" + " " + say_num(i) build sits inside it; the "Prefs Format" query uses the
//        GLOBAL TextBufferGetPtr (0x9b7d00) as its buffer rather than a local
//        `returned_string[256]`; and `Txt.update()` - which is only
//        `mov ecx,[Txt.buffer_get_]; mov [Txt.current_pos_],ecx` - sits at the branch join
//        before the two strcpy's. Everything the old two-loop spelling needed was therefore
//        the wrong control flow, not a detail. 8/83 -> 83/83.
// LEVER: THE STRCPY IS REVERSED, and the polarity is load-bearing twice. `atoi != 12` pushes
//        ebx (the Player field) FIRST and TextBufferGetPtr LAST, so arg1 is the BUFFER - the
//        image stages the existing filename through TextBufferGetPtr so that the two
//        strcpy's at the join then copy it into filename (no-op) and search_key (the real
//        copy). Spelled the intuitive way round it costs `jne` instead of `je`, two extra
//        reloads, and 81/83 instead of 83/83.
// TRIED: `if (atoi(...) == 12) GetPrivateProfileStringA(...) else strcpy(...)` - branch
//        polarity backwards (`jne 0x59dc94` against the image's `je 0x59dc82`); inverting the
//        condition to `!= 12` is what puts the `je` on the query arm and the strcpy on the
//        fall-through.
// LEVER: dropped `std::string faction = "Faction "; faction += std::to_string(i);` for the
//        image's own idiom - `StringTemp[0] = 0; strcat(StringTemp, "Faction "); say_num(i);`
//        (say_num is the existing `char text[0x50]; _itoa(...); strcat(StringTemp, text);`
//        helper this same file already uses in prefs_get_binary/prefs_get; 0x50 == 80, so it
//        reproduces the image's `sub esp, 0x50` local exactly). Removes the __EH_prolog and the
//        four basic_string edges; call_diff moved from 10 calls (image 8, MORE) to 7 (image 8,
//        FEWER - short by one). Moved 3/83 -> 8/83 agreeing. NOT attempted: the image's larger
//        restructure - ExpansionEnabled and the "Prefs Format" GetPrivateProfileStringA query
//        both re-run INSIDE a single per-player loop (`je` to the loop increment when
//        ExpansionEnabled is false), sharing an unnamed global 256-byte buffer at 0x9b7d00
//        across both queries instead of a local `returned_string[256]` - not chased this pass.
// size      280 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645460 0x00645470 0x00645DA7 0x0064FC88
// indirect  0x0059DC5B 0x0059DCA1
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_fac_load() {
    // The image keeps ONE loop and re-runs both the ExpansionEnabled test and
    // the "Prefs Format" query inside it (`je` straight to the increment when
    // the toggle is off), and it uses the global TextBufferGetPtr as the query
    // buffer - not a local `returned_string[256]`. The `Txt.update()` at the
    // join is `mov ecx,[Txt.buffer_get_]; mov [Txt.current_pos_],ecx`.
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (!ExpansionEnabled) {
            continue;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, alpha_faction_prefix);
        strcat(StringTemp, BufferJoinSpace);
        say_num(i);
        GetPrivateProfileStringA(PrefsSection, alpha_profile_format_key, alpha_profile_default_zero, TextBufferGetPtr, 256,
            PrefsFile);
        if (atoi(TextBufferGetPtr) != 12) {
            // NOT `strcpy(Players[i].filename, ...)`: the image pushes ebx
            // (the Player field) FIRST and TextBufferGetPtr LAST, so arg1 is
            // the buffer - it stages the existing filename through
            // TextBufferGetPtr so the two strcpy's at the join copy it into
            // search_key. The unbounded copy into a char[24] field that
            // read_factions documents is still here, one call later.
            strcpy(TextBufferGetPtr, Players[i].filename);
        } else {
            GetPrivateProfileStringA(PrefsSection, StringTemp, Players[i].filename,
                TextBufferGetPtr, 256, PrefsFile);
        }
        Txt.update();
        strcpy(Players[i].filename, TextBufferGetPtr);
        strcpy(Players[i].search_key, TextBufferGetPtr);
    }
}

/*
Purpose: Load the most common preferences from the game's ini to globals.
// ORIGINAL: 0x0059DCF0 ?prefs_load@@YAXH@Z 0x0059DCF0-0x0059E502 BYTE_EXACT
// TRIED: `__forceinline` on the two helpers, zero measured effect under every flag set tried, reverted to plain `inline`.
// TRIED: dominant defect was std::string/std::stringstream (the "Custom World" default and `prefs_get_binary`'s return), which forces an SEH frame this function's actual `push ebp; sub esp, 0x54; push ebx/esi/edi` prologue does not have - first divergence was INSTRUCTION 0 before this. `prefs_get_binary` now writes directly into the global `StringTemp` (0x009B86A0), matching the image's `mov byte ptr [0x9b86a0], 0` + per-bit `strcat`, and the "Custom World" default is the same StringTemp+strcat loop with a SIGNED `int i` (image: `cmp esi, 7; jl`, not `jb`). Fixing this alone cannot get further: `prefs_get_binary` and both `prefs_get` overloads are `inline` (moved before first use where needed) since the image inlines them at MOST call sites, but the image's exact per-site mix (e.g. int-overload: 4 inlined + 3 real calls) is a VC6 codegen heuristic no source spelling reproduced here -
// size      2066 bytes
// prototype void (__cdecl ?prefs_load@@YAXH@Z)(BOOL useDefault)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0059DB40 0x005FD740 0x005FD7A0 0x00627100 0x00645460 0x00645470 0x00645DA7 0x0064FC88
// indirect  0x0059DD47 0x0059DDA7 0x0059DE0B 0x0059DE6F 0x0059DED3 0x0059DFA8 0x0059DFF7 0x0059E0BE 0x0059E10D 0x0059E160 0x0059E1AE 0x0059E20E 0x0059E2B4 0x0059E303 0x0059E3A0 0x0059E3EF 0x0059E46E 0x0059E4BC
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_load(BOOL use_default) {
    set_language(prefs_get_inline("Language", 0, false));
    prefs_get_inline("Difficulty", 0, false);
    DefaultPrefs.difficulty = text_item_number();
    prefs_get_inline("Map Type", 0, false);
    DefaultPrefs.map_type = text_item_number();
    prefs_get_inline("Top Menu", 0, false);
    DefaultPrefs.top_menu = text_item_number();
    prefs_get_inline("Faction", 1, false);
    DefaultPrefs.faction_id = text_item_number();
    // Original (0x59DEF8): StringTemp is cleared before default_prefs()'s
    // Laptop query, whose int prefs_get leaves "0" there; the bits append.
    // `non_pad` is zeroed first: the image keeps it in ebx and pushes that
    // zero for the Laptop query's two zero arguments.
    StringTemp[0] = 0;
    int non_pad = 0;
    uint32_t prefs = default_prefs_load();
    // Pinned 0x0059DF21 compares the difficulty signed (`cmp eax, 2; jge`).
    if (DefaultPrefs.difficulty < DLVL_TALENT) {
        prefs |= PREF_BSC_TUTORIAL_MSGS;
    }
    for (int shift = 31; shift >= 0; shift--) {
        if ((1 << shift) & prefs) {
            non_pad = 1;
            strcat(StringTemp, "1");
        } else if (non_pad || shift < 8) {
            strcat(StringTemp, "0");
        }
    }
    prefs_get("Preferences", StringTemp, use_default);
    AlphaIniPrefs.preferences = text_item_binary();
    prefs = default_prefs2_load();
    PREFS_GET_BINARY(prefs);
    prefs_get("More Preferences", StringTemp, use_default);
    AlphaIniPrefs.more_preferences = text_item_binary();
    prefs_get("Semaphore", "00000000", use_default);
    AlphaIniPrefs.semaphore = text_item_binary();
    prefs_get_inline("Customize", 0, false);
    AlphaIniPrefs.customize = text_item_number();
    prefs = default_rules();
    PREFS_GET_BINARY(prefs);
    prefs_get("Rules", StringTemp, use_default);
    AlphaIniPrefs.rules = text_item_binary();
    prefs = default_warn();
    PREFS_GET_BINARY(prefs);
    prefs_get("Announce", StringTemp, use_default);
    AlphaIniPrefs.announce = text_item_binary();
    // The image builds this directly into the global StringTemp: `mov byte
    // ptr [0x9b86a0], 0` at 0x0059E405, a `strcat` per entry, `cmp esi, 7;
    // jl` (SIGNED) at 0x0059E42C. The "2, " arm falls through first.
    StringTemp[0] = 0;
    for (int i = 0; i < 7; i++) {
        if (!i) {
            strcat(StringTemp, "2, ");
        } else {
            strcat(StringTemp, "1, ");
        }
    }
    prefs_get("Custom World", StringTemp, use_default);
    for (i = 0; i < 7; i++) {
        AlphaIniPrefs.custom_world[i] = text_item_number();
    }
    prefs_get("Time Controls", 1, use_default);
    AlphaIniPrefs.time_controls = text_item_number();
}

/*
Purpose: Write the string value to the pref key of the ini.
// ORIGINAL: 0x0059E510 ?prefs_put@@YAXPADPAD@Z 0x0059E510-0x0059E52F BYTE_EXACT
// symbol    ?prefs_put@@YAXPBD0@Z
// size      31 bytes
// prototype void (__cdecl ?prefs_put@@YAXPADPAD@Z)(int8* lpKeyName, int8* lpString)
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0059E527
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_put(LPCSTR key_name, LPCSTR value) {
    WritePrivateProfileStringA(PrefsSection, key_name, value, PrefsFile);
}

/*
Purpose: Write the value as either an integer or a binary string to the pref key inside the ini.
// ORIGINAL: 0x0059E530 ?prefs_put@@YAXPADHH@Z 0x0059E530-0x0059E5CD BYTE_EXACT
// symbol    ?prefs_put@@YAXPBDHH@Z
// size      157 bytes
// prototype void (__cdecl ?prefs_put@@YAXPADHH@Z)(int8* lpKeyName, int, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x0064FC88
// indirect  0x0059E5C3
// LEVER (2026-09-03): plain `strcat`, not `strcat_s`. 2 of 57 agreeing,
//   83 instructions against the image's 57: the `strcat_s` shim lives in
//   vc6_compat.h, which /FI force-includes BEFORE stdafx.h's
//   `#pragma function(strcat, ...)` line, so inside the shim `strcat` is
//   still an intrinsic and every one of the three sites expanded into the
//   29-instruction `repne scasb`/`rep movsd` walk. The image makes three
//   plain `call strcat` (0x00645470). Writing `strcat` directly in this
//   .cpp - after the pragma - emits the call. Second effect: with the bloat
//   gone the winning set flipped to `/O2 /Oy- /GX`, which supplies the
//   image's `push ebp / mov ebp, esp / sub esp, 0x50` frame. 57 of 57,
//   BYTE_EXACT. Same lever as say_label 0x005A5880.
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_put(LPCSTR key_name, int value, BOOL tgl_binary) {
    // NOT a local buffer: like `prefs_get_binary`, the image builds the
    // value string directly into the global `StringTemp` (0x009B86A0) -
    // `mov byte ptr [0x9b86a0], 0` at 0x0059E539 runs UNCONDITIONALLY
    // before the branch on `tgl_binary` - and passes that buffer straight
    // to WritePrivateProfileStringA, not a copy. The binary loop is the
    // same shape as `prefs_get_binary`, hand-inlined here rather than
    // called (no third call target besides strcat/itoa in the image).
    StringTemp[0] = 0;
    // PLAIN `strcat`, never `strcat_s`: the shim is defined in
    // vc6_compat.h, which /FI force-includes BEFORE stdafx.h's
    // `#pragma function(strcat, ...)` takes effect, so inside the shim
    // `strcat` is still an intrinsic and expands into the 29-instruction
    // `repne scasb` walk. The image makes a plain `call strcat`. Same lever
    // as say_label 0x005A5880.
    if (tgl_binary) {
        for (int non_pad = 0, shift = 31; shift >= 0; shift--) {
            if ((1 << shift) & value) {
                non_pad = 1;
                strcat(StringTemp, alpha_binary_output_one);
            } else if (non_pad || shift < 8) {
                strcat(StringTemp, alpha_binary_output_zero);
            }
        }
    } else {
        char num_buf[80];
        _itoa(value, num_buf, 10);
        strcat(StringTemp, num_buf);
    }
    WritePrivateProfileStringA(PrefsSection, key_name, StringTemp, PrefsFile);
}

// WORKAROUND(inline): pinned prefs_save 0x0059E5D0 expands prefs_put(LPCSTR,int,BOOL)
// inline at its first seven sites (through "More Preferences", 0x0059E7xx) and calls
// 0x0059E530 from "Semaphore" on; the /Ob2 budget here stops one site earlier. This
// forced twin of prefs_put's body covers the seventh site without spending budget.
static __forceinline void prefs_put_inline(LPCSTR key_name, int value, BOOL tgl_binary) {
    StringTemp[0] = 0;
    if (tgl_binary) {
        for (int non_pad = 0, shift = 31; shift >= 0; shift--) {
            if ((1 << shift) & value) {
                non_pad = 1;
                strcat(StringTemp, alpha_binary_output_one);
            } else if (non_pad || shift < 8) {
                strcat(StringTemp, alpha_binary_output_zero);
            }
        }
    } else {
        char num_buf[80];
        _itoa(value, num_buf, 10);
        strcat(StringTemp, num_buf);
    }
    WritePrivateProfileStringA(PrefsSection, key_name, StringTemp, PrefsFile);
}

// RECOVERY_LITERAL: v1 0x00690208 0x00690215 $SG51428 0x0059E5D0
/*
Purpose: Save the most common preferences from memory to the game's ini. 
// ORIGINAL: 0x0059E5D0 ?prefs_save@@YAXH@Z 0x0059E5D0-0x0059E946 BYTE_EXACT
// LEVER: THE IMAGE HAS NO C++ LIBRARY - `std::stringstream` was pulling
//        in a genuine SEH unwind frame (its own destructor is
//        non-trivial), which the image does not carry at all; replaced
//        the whole "Custom World" join and the "Faction %d" key with the
//        `StringTemp`/`strcat`/say_num idiom, and dropped the `sprintf_s`
//        the same way. 140 -> 110 instructions, frame gone.
// TRIED: not byte-exact - the image fully INLINES `prefs_put(LPCSTR,
//            int, BOOL)` (0x0059E530, itself separately claimed
//            BYTE_EXACT) at every one of its ~11 call sites here, each
//            with its own local scratch buffer and direct
//            WritePrivateProfileStringA call, where this body calls the
//            real function. Same "MEASURED inline ceiling" shape the
//            brief documents elsewhere - reproducing it means inlining
//            prefs_put's entire body ~11 times by hand, not attempted at
//            this budget for an 886-byte function.
// size      886 bytes
// prototype void (__cdecl ?prefs_save@@YAXH@Z)(BOOL saveFactions)
// callers   17   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0059E530 0x00645470 0x0064FC88
// indirect  0x0059E61B 0x0059E65F 0x0059E6A2 0x0059E6E4 0x0059E728 0x0059E792 0x0059E7FC 0x0059E8A8 0x0059E92E
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_save(BOOL save_factions) {
    prefs_put("Prefs Format", 12, false);
    prefs_put("Difficulty", DefaultPrefs.difficulty, false);
    prefs_put("Map Type", DefaultPrefs.map_type, false);
    prefs_put("Top Menu", DefaultPrefs.top_menu, false);
    prefs_put("Faction", DefaultPrefs.faction_id, false);
    prefs_put("Preferences", AlphaIniPrefs.preferences, true);
    prefs_put_inline("More Preferences", AlphaIniPrefs.more_preferences, true);
    prefs_put("Semaphore", AlphaIniPrefs.semaphore, true);
    prefs_put("Announce", AlphaIniPrefs.announce, true);
    prefs_put("Rules", AlphaIniPrefs.rules, true);
    prefs_put("Customize", AlphaIniPrefs.customize, false);
    // THE IMAGE HAS NO C++ LIBRARY: no stringstream, no sprintf - the
    // call list is just `strcat`/`_itoa` (say_num's own idiom) into the
    // global StringTemp buffer.
    StringTemp[0] = 0;
    for (uint32_t i = 0; i < 7; i++) {
        say_num(AlphaIniPrefs.custom_world[i]);
        strcat(StringTemp, SocialWinCommaSpaceText);
    }
    prefs_put("Custom World", StringTemp);
    prefs_put("Time Controls", AlphaIniPrefs.time_controls, false);
    if (save_factions) {
        for (uint32_t i = 1; i < MaxPlayerNum; i++) {
            if (ExpansionEnabled) {
                StringTemp[0] = 0;
                strcat(StringTemp, "Faction");
                strcat(StringTemp, BufferJoinSpace);
                say_num(i);
                prefs_put(StringTemp, Players[i].filename);
            }
        }
    }
}

/*
Purpose: Set the internal game preference globals from the ini setting globals.
// ORIGINAL: 0x0059E950 ?prefs_use@@YAXXZ 0x0059E950-0x0059E973 BYTE_EXACT
// TRIED: 5/7 plateau, MNEMONIC_ONLY (register swap on the `more`/
//            `announce` stores only). Tried: declaring `announce` before
//            `more`; storing `announce` before `more`; both reversed
//            together; and direct field-to-field assignment with no
//            locals at all. All four score WORSE (3-4/7) than the
//            committed three-loads-then-three-stores body. VC6 register
//            allocation quirk, not a source-form lever found here.
// size      35 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl prefs_use() {
    // ALL THREE LOADS BEFORE ANY STORE, which is what the image does at
    // 0x0059E950 - three `mov reg, [...]` then three `mov [...], reg`.
    // Written as three assignments the compiler interleaves them.
    const uint32_t preferences = AlphaIniPrefs.preferences;
    const uint32_t more = AlphaIniPrefs.more_preferences;
    const uint32_t announce = AlphaIniPrefs.announce;
    GameDataStateStorage.fields_.game_preferences_ = preferences;
    GameDataStateStorage.fields_.game_more_preferences_ = more;
    GameDataStateStorage.fields_.game_warnings_ = announce;
}
