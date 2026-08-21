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
#include "temp.h"
#include "xpops.h"  // X_pop
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "terraforming.h"
#include "text.h"
#include "textindex.h"
#include "veh.h"

// THE INI STRINGS ARE GLOBALS, not literals at each call site. The image
// loads them - `mov ecx, [0x006900CC]; push ecx` - where a literal would
// compile to `push offset`, which is two bytes shorter per use and shifts
// every branch after it. Six call sites in this file pass them.
const char *PrefsFile = ".\\Alpha Centauri.ini";   // 0x006900CC
const char *PrefsSection = "Alpha Centauri";       // 0x006900D0

LPCSTR AlphaxFileID = "ALPHAX";
LPCSTR ScriptTxtID = "SCRIPT";
// `const`: nothing outside this file names them, so internal
// linkage folds the address into every subscript.
LPSTR *const Compass = (LPSTR *)0x00945D48;
LPSTR *const Difficulty = (LPSTR *)0x0096C85C;
// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
uint32_t Language;  // 0x009BC054

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
    if (!_stricmp(name, "None")) {
        return NoneValue;
    }
    if (!_stricmp(name, "Disable")) {
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
    X_pop("BADTECHKEY", NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return DisabledValue;
}

/*
Purpose: Convert the chassis name string to a numeric chassis id.
// ORIGINAL: 0x00584E40 ?chas_name@@YAHPAD@Z 0x00584E40-0x00584F33
// RULED-OUT: a `purge_trailing(name)` call - the image hand-inlines a NAIVE trim that recomputes `strlen` on every access (4 calls: check, loop-top compare, store index, continue condition) instead of one cached-length pointer walk. Restoring that shape took the call count from 9 to the image's 12; still MISMATCH on register choice (`mov bl,0x20` vs an immediate compare) and downstream byte offsets - not chased further, see weap_name/arm_name (identical).
// LEVER: the trim loop was `if (strlen(name) != 0) { do {...} while (strlen(name) != 0); }`,
//   which VC6 compiles as one copy of the head duplicated at the bottom (+6 instructions past the
//   image's single `jne` backedge). A plain `while (strlen(name) != 0) { if (...) break; ...; }`
//   is the image's rotated-while shape with one head. Moved 20/98 -> 55/98 agreeing.
// RULED-OUT (still open): the chas_id loop's `_stricmp(name, Chassis[chas_id].offsv_1_name)`
//   diverges at instruction 53 - the image resolves the comparison string through
//   `StringTable->get(...)` (`call 0x6169a0`, the same idiom say_tech uses for label lookups)
//   before the ecx pointer switches over; this tree compares the field directly. Not chased -
//   may mean a different field or a `Strings::get` indirection is missing here.
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
    if (!_stricmp(name, "None")) {
        return NoneValue;
    }
    if (!_stricmp(name, "Disable")) {
        return DisabledValue;
    }
    for (int chas_id = 0; chas_id < MaxChassisNum; chas_id++) {
        if (!_stricmp(name, Chassis[chas_id].offsv_1_name)) {
            return chas_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop("BADCHASKEY", NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}

/*
Purpose: Convert the weapon name string to a numeric weapon id.
// ORIGINAL: 0x00584F40 ?weap_name@@YAHPAD@Z 0x00584F40-0x00585030
// RULED-OUT: as chas_name (0x00584E40) - the naive recomputed-`strlen` trim loop, not `purge_trailing`. Call count 9 -> 12, still MISMATCH (register choice for the space literal).
// LEVER: same as chas_name - `if (strlen != 0) { do {...} while; }` duplicated the loop head;
//   a plain `while (strlen(name) != 0) { if (...) break; ...; }` matches the image's single
//   backedge. Moved 20/98 -> 55/98 agreeing.
// RULED-OUT (still open): same weap_id-loop StringTable->get() divergence as chas_name, not
//   chased further this pass.
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
    if (!_stricmp(name, "None")) {
        return NoneValue;
    }
    if (!_stricmp(name, "Disable")) {
        return DisabledValue;
    }
    for (int weap_id = 0; weap_id < MaxWeaponNum; weap_id++) {
        if (!_stricmp(name, Weapon[weap_id].name_short)) {
            return weap_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop("BADWEAPKEY", NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}

/*
Purpose: Convert the armor name string to a numeric armor id.
// ORIGINAL: 0x00585030 ?arm_name@@YAHPAD@Z 0x00585030-0x00585120
// RULED-OUT: as chas_name (0x00584E40) - the naive recomputed-`strlen` trim loop, not `purge_trailing`. Call count 9 -> 12, still MISMATCH (register choice for the space literal).
// LEVER: same as chas_name - the `if (strlen != 0) { do {...} while; }` duplicated-head shape
//   became a plain `while (strlen(name) != 0) { if (...) break; ...; }`. PLUS a second, distinct
//   defect: the final `return DisabledValue;` (-2) after X_pop("BADARMKEY",...) should be
//   `return 0;` - the image's epilogue there is `xor eax,eax` (0x00585112), matching this
//   function's own documented "error (0)" return and chas_name/weap_name's `return 0;`. Because
//   the old -2 return matched the earlier 'Disable' early return, VC6 had merged the two exits;
//   fixing the constant also restored the image's separate epilogue. Moved 24/98 -> 55/98.
// RULED-OUT (still open): same arm_id-loop StringTable->get() divergence as chas_name, not
//   chased further this pass.
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
    if (!_stricmp(name, "None")) {
        return NoneValue;
    }
    if (!_stricmp(name, "Disable")) {
        return DisabledValue;
    }
    for (int arm_id = 0; arm_id < MaxArmorNum; arm_id++) {
        if (!_stricmp(name, Armor[arm_id].name_short)) {
            return arm_id;
        }
    }
    parse_says(0, Txt.get_file_path(), -1, -1);
    parse_says(1, name, -1, -1);
    parse_says(2, TextBufferGetPtr, -1, -1);
    X_pop("BADARMKEY", NULL); // TODO: Fix crash bug if BADTECHKEY is triggered.
    return 0;
}

/*
Purpose: Parse the current tech name inside the Txt item buffer into a tech id.
// ORIGINAL: 0x00585150 ?tech_item@@YAHXZ 0x00585150-0x00585164 BYTE_EXACT
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
// ORIGINAL: 0x00585170 ?read_basic_rules@@YAHXZ 0x00585170-0x00585E26
// LEVER: the ten `Rules->tech_* = tech_item();` sites now match the image's
//        call pattern (`text_get; text_item; tech_name` per site, confirmed
//        against `osmx calls`) now that `tech_item` is `MEASURED inline` in
//        alpha.h - it was a call the image never makes (see its own
//        `callers 0`). Landed as a real structural fix; this function is
//        883 image instructions across ~40 unrelated #SECTIONs and stays
//        MISMATCH on everything else - not chased further this pass.
// LEVER: all 83 `X = text_get_number(lo, hi);` sites rewritten as the image's
//        two-statement `text_get(); X = range(text_item_number(), lo, hi);`
//        (text_get_number is `MEASURED inline` and expands to the same thing
//        through text_get_source/text_item_number_source, but the image calls
//        the zero-arg text_get()/text_item_number() wrappers, not those). Under
//        the /Ob0 flag set this moved 70/883 -> 129/883 agreeing; osmx's own
//        similarity-picked flag set switches away from /Ob0 on this larger body
//        (53/883), so `measure`'s default output understates the win - see
//        AGENT_BRIEF's MEASURING section on this exact artifact. Still deep
//        MISMATCH; not chased further this pass.
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
    text_get();
    Rules->move_rate_roads = range(text_item_number(), 1, 100);
    text_get();
    Rules->nutrient_req_citizen = range(text_item_number(), 0, 100);
    text_get();
    Rules->artillery_dmg_num = range(text_item_number(), 1, 1000);
    Rules->artillery_dmg_denom = range(text_item_number(), 1, 1000);
    text_get();
    Rules->artillery_max_rng = range(text_item_number(), 1, 8);
    text_get();
    Rules->max_airdrop_sans_orb_insert = range(text_item_number(), 1, 500);
    text_get();
    Rules->nutrient_cost_multi = range(text_item_number(), 1, 100);
    text_get();
    Rules->mineral_cost_multi = range(text_item_number(), 1, 100);
    text_get();
    Rules->tech_discovery_rate_pct_std = range(text_item_number(), 0, 1000);
    text_get();
    Rules->limit_mineral_mine_sans_road = range(text_item_number(), 0, 100);
    text_get();
    Rules->tgl_nutrient_effect_with_mine = range(text_item_number(), -1, 0); // Weird logic behind -1/0 value
    text_get();
    Rules->min_base_size_specialists = range(text_item_number(), 0, 100);
    text_get();
    Rules->drones_genejack_factory = range(text_item_number(), 0, 100);
    text_get();
    Rules->pop_limit_sans_hab_complex = range(text_item_number(), 1, 100);
    text_get();
    Rules->pop_limit_sans_hab_dome = range(text_item_number(), 1, 100);
    text_get();
    Rules->extra_pct_cost_proto_land = range(text_item_number(), 0, 500);
    text_get();
    Rules->extra_pct_cost_proto_sea = range(text_item_number(), 0, 500);
    text_get();
    Rules->extra_pct_cost_proto_air = range(text_item_number(), 0, 500);
    text_get();
    Rules->psi_combat_ratio_atk[TRIAD_LAND] = range(text_item_number(), 1, 1000);
    Rules->psi_combat_ratio_def[TRIAD_LAND] = range(text_item_number(), 1, 1000);
    text_get();
    Rules->psi_combat_ratio_atk[TRIAD_SEA] = range(text_item_number(), 1, 1000);
    Rules->psi_combat_ratio_def[TRIAD_SEA] = range(text_item_number(), 1, 1000);
    text_get();
    Rules->psi_combat_ratio_atk[TRIAD_AIR] = range(text_item_number(), 1, 1000);
    Rules->psi_combat_ratio_def[TRIAD_AIR] = range(text_item_number(), 1, 1000);
    text_get();
    Rules->player_start_energy_reserve = range(text_item_number(), 0, 1000);
    text_get();
    Rules->combat_pct_base_def = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_atk_road = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_atk_higher_elev = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pen_pct_atk_lwr_elev = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_mobile_open_ground = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_def_vs_mobile_rough = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_infantry_vs_base = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pen_pct_atk_airdrop = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_fanatic_atk_bonus = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_land_gun_vs_ship_art = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_art_bonus_lvl_alt = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_trance_def_vs_psi = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_emp_song_atk_vs_psi = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pen_pct_air_supr_vs_grnd = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_air_supr_vs_air = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pen_pct_non_cbt_def_vs_cbt = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_com_jam_def_vs_mobl = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_bonus_vs_ship_port = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_aaa_bonus_vs_air = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_def_range_sensor = range(text_item_number(), -100, 1000);
    text_get();
    Rules->combat_pct_psi_atk_bonus_planet = range(text_item_number(), -100, 1000);
    text_get();
    Rules->retool_pct_pen_prod_chg = range(text_item_number(), 0, 100);
    text_get();
    Rules->retool_strictness = range(text_item_number(), 0, 3); // Bug fix: increased max param to 3
    text_get();
    Rules->retool_exemption = range(text_item_number(), 0, 1000);
    text_get();
    Rules->min_turns_councils = range(text_item_number(), 0, 1000);
    text_get();
    Rules->minerals_harvesting_forest = range(text_item_number(), 0, 100);
    text_get();
    Rules->territory_max_dist_base = range(text_item_number(), 0, 100);
    text_get();
    Rules->turns_corner_gbl_energy_mrkt = range(text_item_number(), 1, 100);
    Rules->tech_improve_fungus_sqr = tech_item();
    Rules->tech_ease_fungus_movement = tech_item();
    Rules->tech_build_roads_fungus = tech_item();
    Rules->tech_two_spec_abilities = tech_item();
    Rules->tech_three_nutrients_sqr = tech_item();
    Rules->tech_three_minerals_sqr = tech_item();
    Rules->tech_three_energy_sqr = tech_item();
    Rules->tech_orb_insert_sans_spc_elev = tech_item();
    Rules->tech_mining_platform_bonus = tech_item();
    Rules->tech_economic_victory = tech_item();
    text_get();
    Rules->tgl_probe_can_steal_tech = range(text_item_number(), 0, 1); // Fix: Set min param to 0
    text_get();
    Rules->tgl_human_always_contact_net = range(text_item_number(), 0, 1); // Fix: Set min param to 0
    text_get();
    Rules->tgl_humans_always_contact_pbem = range(text_item_number(), 0, 1); // Fix: Set min param to 0
    text_get();
    Rules->max_pct_dmg_art_vs_unit_base_bnkr = range(text_item_number(), 10, 100);
    text_get();
    Rules->max_pct_dmg_art_vs_units_open = range(text_item_number(), 10, 100);
    text_get();
    Rules->max_pct_dmg_art_vs_units_sea = range(text_item_number(), 10, 100);
    text_get();
    Rules->freq_global_warming_num = range(text_item_number(), 0, 1000);
    Rules->freq_global_warming_denom = range(text_item_number(), 1, 1000);
    text_get();
    Rules->normal_starting_year = range(text_item_number(), 0, 999999);
    text_get();
    Rules->normal_end_year_low_three_diff = range(text_item_number(), 0, 999999);
    text_get();
    Rules->normal_end_year_high_three_diff = range(text_item_number(), 0, 999999);
    text_get();
    Rules->tgl_oblit_base_atrocity = range(text_item_number(), 0, 1); // Fix: Set min param to 0
    text_get();
    Rules->size_base_subspace_gen = range(text_item_number(), 1, 999); // SMACX only
    text_get();
    Rules->subspace_generators_needed = range(text_item_number(), 1, 999); // SMACX only
    if (text_open(AlphaxFileID, "WORLDBUILDER")) {
        return 1;
    }
    text_get();
    WorldBuilder->land_base = range(text_item_number(), 50, 4000);
    text_get();
    WorldBuilder->land_mod = range(text_item_number(), 0, 2000);
    text_get();
    WorldBuilder->continent_base = range(text_item_number(), 5, 1000);
    text_get();
    WorldBuilder->continent_mod = range(text_item_number(), 5, 1000);
    text_get();
    WorldBuilder->hills_base = range(text_item_number(), 0, 100);
    text_get();
    WorldBuilder->hills_mod = range(text_item_number(), 0, 100);
    text_get();
    WorldBuilder->plateau_base = range(text_item_number(), 0, 500);
    text_get();
    WorldBuilder->plateau_mod = range(text_item_number(), 0, 500);
    text_get();
    WorldBuilder->rivers_base = range(text_item_number(), 0, 100);
    text_get();
    WorldBuilder->rivers_rain_mod = range(text_item_number(), 0, 100);
    text_get();
    WorldBuilder->solar_energy = range(text_item_number(), 1, 64);
    text_get();
    WorldBuilder->thermal_band = range(text_item_number(), 1, 64);
    text_get();
    WorldBuilder->thermal_deviance = range(text_item_number(), 1, 64);
    text_get();
    WorldBuilder->global_warming = range(text_item_number(), 1, 64);
    text_get();
    WorldBuilder->sea_level_rises = range(text_item_number(), 1, 100);
    text_get();
    WorldBuilder->cloudmass_peaks = range(text_item_number(), 0, 20);
    text_get();
    WorldBuilder->cloudmass_hills = range(text_item_number(), 0, 20);
    text_get();
    WorldBuilder->rainfall_coeff = range(text_item_number(), 0, 8);
    text_get();
    WorldBuilder->deep_water = range(text_item_number(), -100, 100);
    text_get();
    WorldBuilder->shelf = range(text_item_number(), -100, 100);
    text_get();
    WorldBuilder->plains = range(text_item_number(), -100, 100);
    text_get();
    WorldBuilder->beach = range(text_item_number(), -100, 100);
    text_get();
    WorldBuilder->hills = range(text_item_number(), 0, 100);
    text_get();
    WorldBuilder->peaks = range(text_item_number(), -100, 100);
    text_get();
    WorldBuilder->fungus = range(text_item_number(), 0, 5);
    text_get();
    WorldBuilder->cont_size_ratio1 = text_item_number();
    WorldBuilder->cont_size_ratio2 = text_item_number();
    WorldBuilder->cont_size_ratio3 = text_item_number();
    WorldBuilder->cont_size_ratio4 = text_item_number();
    WorldBuilder->cont_size_ratio5 = text_item_number();
    text_get();
    WorldBuilder->islands = range(text_item_number(), 1, 500);
    return false;
}

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
    if (text_open(AlphaxFileID, "TECHNOLOGY")) {
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
    if (text_open(AlphaxFileID, "TECHNOLOGY")) {
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

/*
Purpose: Parse the 1st eight lines of the specified faction's file into a player structure. The 
         toggle parameter will end the function early if set to 2 (original code never uses this).
// ORIGINAL: 0x00586090 ?read_faction@@YAXPAUPlayer@@H@Z 0x00586090-0x00586F2B
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
    strcpy_s(player->formal_name_faction, 40, text_item());
    player->formal_name_faction[39] = 0;
    strcpy_s(player->desc_name_faction, 24, text_item());
    player->desc_name_faction[23] = 0;
    strcpy_s(player->noun_faction, 24, text_item());
    player->noun_faction[23] = 0;
    LPSTR gender = text_item();
    player->noun_gender = GENDER_MALE;
    if (gender[0] == 'F' || gender[0] == 'f') {
        player->noun_gender = GENDER_FEMALE;
    } else if (gender[0] == 'N' || gender[0] == 'n') {
        player->noun_gender = GENDER_NEUTRAL;
    }
    player->is_noun_plural = range(text_item_number() - 1, false, true); // original value: 1 or 2
    strcpy_s(player->name_leader, 24, text_item());
    player->name_leader[23] = 0;
    gender = text_item();
    player->is_leader_female = (gender[0] == 'F' || gender[0] == 'f') ? true : false;
    if (toggle == 2) {
        return;
    }
    player->ai_fight = text_item_number();
    player->ai_power = text_item_number();
    player->ai_tech = text_item_number();
    player->ai_wealth = text_item_number();
    player->ai_growth = text_item_number();
    text_get();
    LPSTR parse_rule_check = text_item();
    size_t len = strlen(parse_rule_check);
    while (len) {
        // NOT a heap allocation: the image copies into a fixed 256-byte LOCAL
        // buffer (`sub esp, 0x104`), same idiom as read_rules' `order_buf`.
        char parse_rule[256];
        strcpy(parse_rule, parse_rule_check);
        LPSTR parse_param = text_item();
        if (!_stricmp(parse_rule, BonusName[0].key)) { // TECH
            // will have issues if custom tech abbreviations starting with numbers are used
            int player_selected = atoi(parse_param);
            if (!player_selected && player->faction_bonus_count < 8) {
                player->faction_bonus_id[player->faction_bonus_count] = RULE_TECH;
                player->faction_bonus_val1[player->faction_bonus_count] = tech_name(parse_param);
                player->faction_bonus_count++;
            } else {
                player->rule_tech_selected = player_selected;
            }
        } else if (!_stricmp(parse_rule, BonusName[1].key)) { // MORALE
            if (parse_param[0] == '+') {
                parse_param++;
            }
            player->rule_morale = atoi(parse_param);
            // 0 indicates an exemption from negative modifiers from other sources
            if (!player->rule_morale) {
                player->rule_flags |= RFLAG_MORALE;
            }
        } else if (!_stricmp(parse_rule, BonusName[3].key) && player->faction_bonus_count < 8) {
            // FACILITY
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FACILITY;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
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
        } else if (!_stricmp(parse_rule, BonusName[12].key) && player->faction_bonus_count < 8) {
            // UNIT
            player->faction_bonus_id[player->faction_bonus_count] = RULE_UNIT;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[2].key)) { // PSI
            player->rule_psi = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[14].key)) { // SHARETECH
            player->rule_sharetech = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[15].key)) { // TERRAFORM
            player->rule_flags |= RFLAG_TERRAFORM;
        } else if ((!_stricmp(parse_rule, BonusName[16].key) // SOCIAL, ROBUST, IMMUNITY
            || !_stricmp(parse_rule, BonusName[17].key)
            || !_stricmp(parse_rule, BonusName[18].key)) && player->faction_bonus_count < 8) {
            // Moved faction_bonus_count check to start rather than inner loop
            int value = 0;
            while (parse_param[0] == '+' || parse_param[0] == '-') {
                (parse_param[0] == '+') ? value++ : value--;
                parse_param++;
            }
            if (!value) { // cannot be zero
                value = 1;
            }
            for (int j = 0; j < MaxSocialEffectNum; j++) {
                if (!_stricmp(SocialEffects[j].set1, parse_param)) {
                    if (!_stricmp(parse_rule, BonusName[17].key)) {
                        player->faction_bonus_id[player->faction_bonus_count] = RULE_ROBUST;
                    } else {
                        player->faction_bonus_id[player->faction_bonus_count] =
                            !_stricmp(parse_rule, BonusName[16].key) ? RULE_SOCIAL : RULE_IMMUNITY;
                    }
                    player->faction_bonus_val1[player->faction_bonus_count] = j; // soc effect id
                    player->faction_bonus_val2[player->faction_bonus_count] = value; // value mod
                    player->faction_bonus_count++;
                    break;
                }
            }
        } else if ((!_stricmp(parse_rule, BonusName[19].key) // IMPUNITY, PENALTY
            || !_stricmp(parse_rule, BonusName[20].key)) && player->faction_bonus_count < 8) {
            // Moved faction_bonus_count check to start rather than inner loop
            for (int j = 0; j < MaxSocialCatNum; j++) {
                for (int k = 0; k < MaxSocialModelNum; k++) {
                    if (!_stricmp(parse_param,
                        StringTable->get((int)SocialCategories[j].name[k]))) {
                        player->faction_bonus_id[player->faction_bonus_count] =
                            !_stricmp(parse_rule, BonusName[19].key) ? RULE_IMPUNITY : RULE_PENALTY;
                        player->faction_bonus_val1[player->faction_bonus_count] = j; // category id
                        player->faction_bonus_val2[player->faction_bonus_count] = k; // model id
                        player->faction_bonus_count++;
                    }
                }
            }
        } else if (!_stricmp(parse_rule, BonusName[21].key) && player->faction_bonus_count < 8) {
            // FUNGNUTRIENT
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGNUTRIENT;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[22].key) && player->faction_bonus_count < 8) {
            // FUNGMINERALS
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGMINERALS;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[23].key) && player->faction_bonus_count < 8) {
            // FUNGENERGY
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FUNGENERGY;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[24].key)) { // COMMFREQ
            player->rule_flags |= RFLAG_COMMFREQ;
        } else if (!_stricmp(parse_rule, BonusName[25].key)) { // MINDCONTROL
            player->rule_flags |= RFLAG_MINDCONTROL;
        } else if (!_stricmp(parse_rule, BonusName[26].key)) { // FANATIC
            player->rule_flags |= RFLAG_FANATIC;
        } else if (!_stricmp(parse_rule, BonusName[27].key) && player->faction_bonus_count < 8) {
            // VOTES
            player->faction_bonus_id[player->faction_bonus_count] = RULE_VOTES;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[28].key)) { // FREEPROTO
            player->rule_flags |= RFLAG_FREEPROTO;
        } else if (!_stricmp(parse_rule, BonusName[29].key)) { // AQUATIC
            player->rule_flags |= RFLAG_AQUATIC;
        } else if (!_stricmp(parse_rule, BonusName[30].key)) { // ALIEN
            player->rule_flags |= RFLAG_ALIEN;
        } else if (!_stricmp(parse_rule, BonusName[31].key) && player->faction_bonus_count < 8) {
            // FREEFAC
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FREEFAC;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[32].key) && player->faction_bonus_count < 8) {
            // REVOLT
            player->faction_bonus_id[player->faction_bonus_count] = RULE_REVOLT;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[33].key) && player->faction_bonus_count < 8) {
            // NODRONE
            player->faction_bonus_id[player->faction_bonus_count] = RULE_NODRONE;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[34].key)) { // WORMPOLICE
            player->rule_flags |= RFLAG_WORMPOLICE;
        } else if (!_stricmp(parse_rule, BonusName[35].key) && player->faction_bonus_count < 8) {
            // FREEABIL
            player->faction_bonus_id[player->faction_bonus_count] = RULE_FREEABIL;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[36].key) && player->faction_bonus_count < 8) {
            // PROBECOST
            player->faction_bonus_id[player->faction_bonus_count] = RULE_PROBECOST;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[37].key) && player->faction_bonus_count < 8) {
            // DEFENSE
            player->faction_bonus_id[player->faction_bonus_count] = RULE_DEFENSE;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[38].key) && player->faction_bonus_count < 8) {
            // OFFENSE
            player->faction_bonus_id[player->faction_bonus_count] = RULE_OFFENSE;
            player->faction_bonus_val1[player->faction_bonus_count] = atoi(parse_param);
            player->faction_bonus_count++;
        } else if (!_stricmp(parse_rule, BonusName[39].key)) { // TECHSHARE
            player->rule_flags |= RFLAG_TECHSHARE;
            player->rule_sharetech = atoi(parse_param);
        } else if (!_stricmp(parse_rule, BonusName[40].key)) { // TECHSTEAL
            player->rule_flags |= RFLAG_TECHSTEAL;
        }
        parse_rule_check = text_item();
        len = strlen(parse_rule_check);
    }
    // Societal Ideology + Anti-Ideology
    for (int i = 0; i < 2; i++) {
        *(&player->soc_ideology_category + i) = -1;
        *(&player->soc_ideology_model + i) = 0;
        *(&player->soc_ideology_effect + i) = -1;
        text_get();
        LPSTR soc_category = text_item();
        for (int j = 0; j < MaxSocialCatNum; j++) {
            LPSTR check_cat_type = StringTable->get((int)SocialCategories[j].type);
            if (Language ? !_strnicmp(soc_category, check_cat_type, 4) 
                : !_stricmp(soc_category, check_cat_type)) {
                *(&player->soc_ideology_category + i) = j;
                break;
            }
        }
        LPSTR soc_model = text_item();
        int soc_cat_num = *(&player->soc_ideology_category + i);
        if (soc_cat_num >= 0) {
            for (int j = 0; j < MaxSocialModelNum; j++) {
                LPSTR check_model = StringTable->get((int)SocialCategories[soc_cat_num].name[j]);
                if (Language ?
                    !_strnicmp(soc_model, check_model, 4) : !_stricmp(soc_model, check_model)) {
                    *(&player->soc_ideology_model + i) = j;
                    break;
                }
            }
        }
        LPSTR soc_effect = text_item();
        for (j = 0; j < MaxSocialEffectNum; j++) {
            if (!_stricmp(SocialEffects[j].set1, soc_effect)) {
                // Bug fix: Original code sets this value to -1, disabling AI faction "Emphasis"
                // value. No indication this was intentional.
                *(&player->soc_ideology_effect + i) = j;
                break;
            }
        }
    }
    // Faction and Leader related strings
    text_get(); // skips 2nd value in this line, abbreviation unused?
    strcpy_s(player->adj_name_faction, 128, text_item());
    player->adj_name_faction[127] = 0;
    text_get();
    strcpy_s(player->assistant_name, 24, text_item());
    player->assistant_name[23] = 0;
    strcpy_s(player->scientist_name, 24, text_item());
    player->scientist_name[23] = 0;
    strcpy_s(player->assistant_city, 24, text_item());
    player->assistant_city[23] = 0;
    text_get();
    strcpy_s(player->title_leader, 24, text_item());
    player->title_leader[23] = 0;
    strcpy_s(player->adj_leader, 128, text_item());
    player->adj_leader[127] = 0;
    strcpy_s(player->adj_insult_leader, 128, text_item());
    player->adj_insult_leader[127] = 0;
    strcpy_s(player->adj_faction, 128, text_item());
    player->adj_faction[127] = 0;
    strcpy_s(player->adj_insult_faction, 128, text_item());
    player->adj_insult_faction[127] = 0;
    text_get();
    strcpy_s(player->insult_leader, 24, text_item());
    player->insult_leader[23] = 0;
}

/*
Purpose: Parse the #BONUSNAMES, #FACTIONS, and #NEWFACTIONS sections inside the alpha(x).txt.
// ORIGINAL: 0x00586F30 ?read_factions@@YAHXZ 0x00586F30-0x005871C9
// RULED-OUT: fixes, call count 23 -> 28-29 (image 31): 1. `rand() % faction_count`, not `random(0, faction_count)` - the image calls the C library `rand` (0x0064601D) directly, never the game's seeded PRNG. BUG IN THE ORIGINAL, preserved. 2. `load_faction_art` was a temp.h function POINTER, compiling an indirect call where the image has `call rel32` twice; promoted to a src/pending_bodies.cpp forwarder (see there and temp.h). 3. `strcpy_s(dest, src)`, not `strncpy_s(dest, src, 24)`, for BonusName[i].key and both Players[player] fields in the first two loops - the image pushes only 2 args (`call 0x00645460`), an unbounded copy, not a 3-arg bounded one. BUG IN THE ORIGINAL, preserved. Remaining gap: `text_open`/`text_get` each short by 1 (a ternary argument the image tail-merges two of its three arms for, not chased), and `load_faction_art` merges its two call sites into one physical `call` at the object level where the image keeps two.
// LEVER: signedness at the JENN282 reroll - `uint32_t faction_count = 14;` and `uint32_t
//        faction_set = rand_val / 7;` made `rand() % faction_count` compile as an unsigned
//        div and the /7 as an unsigned magic-multiply; the image uses `cdq; idiv` for the
//        modulo and the signed magic-multiply sequence for /7. Both changed to plain `int`.
//        Moved 44/214 -> 46/214 agreeing.
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
    if (text_open(AlphaxFileID, "BONUSNAMES")) {
        return true;
    }
    for (uint32_t i = 0; i < MaxBonusNameNum; i++) {
        if (!(i % 8)) { // 8 entries per line
            text_get();
        }
        // BUG IN THE ORIGINAL (preserved): `call 0x00645460` at 0x00586F63
        // pushes only the two arguments - plain unbounded `strcpy`, not a
        // 24-byte-bounded copy. Same for both `Players[player]` fields just
        // below (0x00586FD2, 0x00586FDE); the length-checked calls only
        // start at the `strcpy_s(..., 24, ...)` further down this function.
        strcpy_s(BonusName[i].key, text_item());
    }
    if (text_open(AlphaxFileID, ExpansionEnabled ? "NEWFACTIONS" : "FACTIONS")) {
        return true;
    }
    for (int player = 1; player < MaxPlayerNum; player++) {
        text_get();
        strcpy_s(Players[player].filename, text_item());
        strcpy_s(Players[player].search_key, text_item());
    }
    // SMACX only: Will override any values parsed from alphax.txt #NEWFACTIONS if set in ini;
    prefs_fac_load(); // Removed an extra SMACX_Enabled check around call since there is one inside
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
                if (text_open(AlphaxFileID, !faction_set ? "FACTIONS" : (faction_set == 1)
                    ? "NEWFACTIONS" : "CUSTOMFACTIONS")) {
                    return true;
                }
                faction_id = rand_val % 7;
                for (int j = faction_id; j >= 0; j--) {
                    text_get();
                }
                strcpy_s(Players[player].filename, 24, text_item());
                strcpy_s(Players[player].search_key, 24, text_item()); // original copied filename twice
                for (int k = 1; k < MaxPlayerNum; k++) {
                    if (player != k) {
                        if (!strcmp(Players[player].filename, Players[k].filename)) {
                            faction_id = -1;
                            break;
                        }
                    }
                }
                if (faction_id != -1) { // skip Players[0] like below check
                    read_faction(&Players[player], 0);
                    load_faction_art(player);
                }
            } while (faction_id == -1);
        } else {
            // removed check (&Players[player] != &Players[0]) since Players[0] is already skipped
            // moved this into same loop to increase performance with random factions
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
// ORIGINAL: 0x00587240 ?read_units@@YAHXZ 0x00587240-0x005873B1
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
// RULED-OUT: not chased further - the image never reads a "reactor" field at
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
        LPSTR name = text_item();
        strcpy(VehPrototypes[proto_id].veh_name, name);
        int chas_id = chas_name(text_item());
        int weap_id = weap_name(text_item());
        int armor_id = arm_name(text_item());
        int plan = text_item_number();
        int cost = text_item_number();
        int carry = text_item_number();
        VehPrototypes[proto_id].preq_tech = (int16_t)tech_name(text_item());
        int icon = text_item_number();
        int ability = text_item_binary();
        int reactor_id = text_item_number(); // Add ability to read reactor for #UNITS
        if (!reactor_id) { // if not set or 0, default behavior
            switch (proto_id) {
              // There was a pointless explicit check for BSC_BATTLE_OGRE_MK1 to set reactor to 1
              // The parameters set by check are no different than default
              case BSC_BATTLE_OGRE_MK2:
                reactor_id = RECT_FUSION;
                break;
              case BSC_BATTLE_OGRE_MK3:
                reactor_id = RECT_QUANTUM;
                break;
              default:
                reactor_id = RECT_FISSION;
                break;
            }
        }
        make_proto(proto_id, chas_id, weap_id, armor_id, ability, reactor_id);
        // If set, override auto calculated values from make_proto()
        if (plan != -1) { // plan auto calculate: -1
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
// ORIGINAL: 0x005873C0 ?read_rules@@YAHH@Z 0x005873C0-0x0058829C
// LEVER: the #TERRAIN loop's `order_str` is copied into a 256-byte LOCAL
//        buffer (`strcpy` at 0x005874AE into `[ebp-0x114]`) before being
//        handed to `parse_string_OG` - not held as a bare `LPSTR`. That
//        local is almost the entire gap between this tree's frame and the
//        image's (`sub esp, 0x114` vs a bare-pointer version's far smaller
//        one); after adding it the frame is 0x11c against the image's
//        0x114, 8 bytes (two DWORDs) still unaccounted for somewhere else
//        in this ~40-#SECTION function. 51/1217 -> 65/1217.
// RULED-OUT: the rest of this function - it is 883 image instructions across
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
    StringTable->init(49952);
    if (labels_init()) {
        return true;
    }
    text_clear_index();
    text_make_index(ScriptTxtID);
    text_make_index(AlphaxFileID);
    if (read_tech() || read_basic_rules() || text_open(AlphaxFileID, "TERRAIN")) {
        return true;
    }
    for (int i = 0; i < MaxTerrainNum; i++) {
        text_get();
        // Land + sea terraforming
        for (int j = 0; j < 2; j++) {
            *(&Terraforming[i].name + j) = text_item_string();
            *(&Terraforming[i].preq_tech + j) = tech_name(text_item());
            // Add in bits & incompatible bits vs hard coded constant struct
            *(&Terraforming[i].bit + j) = TerraformingBits[i][j];
        }
        Terraforming[i].rate = text_item_number();
        // Land + sea orders
        // NOT held as a bare pointer: the image copies `text_item()`'s
        // result into a 256-byte LOCAL buffer first (`strcpy` at
        // 0x005874AE, into `[ebp-0x114]`) before handing it to
        // `parse_string_OG` - that local is why the image's frame is
        // 0x114 bytes against a bare-pointer version's much smaller one.
        char order_buf[256];
        strcpy(order_buf, text_item());
        for (j = 0; j < 2; j++) {
            parse_say(0, (int)*(&Terraforming[i].name + j), -1, -1);
            StringTemp[0] = 0;
            parse_string_OG(order_buf, StringTemp);
            *(&Order[i + 4].order + j) = StringTable->put(StringTemp);
        }
        Order[i + 4].letter = text_item_string();
        Terraforming[i].shortcuts = text_item_string();
    }
    if (text_open(AlphaxFileID, "RESOURCEINFO")) {
        return true;
    }
    for (i = 0; i < MaxResourceInfoNum; i++) {
        text_get();
        text_item();
        ResourceInfo[i].nutrients = text_item_number();
        ResourceInfo[i].minerals = text_item_number();
        ResourceInfo[i].energy = text_item_number();
        ResourceInfo[i].unused = text_item_number();
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
        // Bug fix/Enhancement: original function skips this value and is left as zero, isn't
        // referenced elsewhere in code. Likely because default power value is sequential.
        // This will allow future modifications.
        Reactor[i].power = (uint16_t)text_item_number();
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
            VehPrototypes[i].unk_1 = 0;
            VehPrototypes[i].flags = 0;
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
        /*
        Enhancement: The original code explicitly sets this value to disabled (-2) overriding
        alpha/x.txt.  It states in #FACILITIES alpha/x.txt: "Free  = No longer supported". The
        original AC manual in Appendix 2 and official strategy guide both list the specific
        facilities being free with certain tech.  However, this mechanic could have been removed
        for balance reasons. Or maybe was dropped due to time constraints. There is code that
        checks this value and sets the free facility only for new bases built after discovering the
        tech. It looks like existing bases do not get it. Will have to review this more. For now,
        this mechanic will be included as is. You can revert to vanilla behavior by modifying the
        four entries below in alpha/x.txt #FACILITIES with free parameter set to Disabled.

        Recycling Tanks, 4, 0, Biogen, EcoEng2,   > free with "Adv.Ecological Engineering"
        Recreation Commons, 4, 1, Psych, SentEco, > free with "Sentient Econometrics"
        Energy Bank, 8, 1, IndEcon, QuanMac,      > free with "Quantum Machinery"
        Network Node, 8, 1, InfNet, HAL9000,      > free with "Self - Aware Machines"
        */
        Facility[i].free_tech = tech_name(text_item());
        Facility[i].effect = text_item_string();
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
        if (i < 4 || i > 23) { // Skipping over orders set by #TERRAIN
            text_get();
            Order[i].order = text_item_string();
            // Potential enhancement: Have separate string for sea
            Order[i].order_sea = Order[i].order;
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
            Citizen[i].ops_bonus = text_item_number();
            Citizen[i].psych_bonus = text_item_number();
            Citizen[i].research_bonus = text_item_number();
        }
    }
    if (text_open(AlphaxFileID, "SOCIO")) {
        return true;
    }
    text_get();
    for (i = 0; i < MaxSocialEffectNum; i++) {
        strcpy_s(SocialEffects[i].set1, 24, text_item());
    }
    text_get();
    for (i = 0; i < MaxSocialEffectNum; i++) {
        strcpy_s(SocialEffects[i].set2, 24, text_item());
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
            ZeroMemory(&SocialCategories[i].model_effect[j], sizeof(SocialEffect));
            LPSTR mod_value = text_item();
            size_t mod_len = strlen(mod_value);
            while (mod_len) {
                int value = 0;
                while (mod_value[0] == '+' || mod_value[0] == '-') {
                    (mod_value[0] == '+') ? value++ : value--;
                    mod_value++;
                }
                for (int k = 0; k < MaxSocialEffectNum; k++) {
                    if (!_stricmp(mod_value, SocialEffects[k].set1)) {
                        *(&SocialCategories[i].model_effect[j].economy + k) = value;
                        break;
                    }
                }
                mod_value = text_item();
                mod_len = strlen(mod_value);
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
    // Revised original nested for loop code to be more efficient; Logic is still same.
    // Buttons used by "Edit Map" menus.
    int j = 0;
    for (i = 0; i < MaxTerrainNum; i++) {
        // excludes: Fungus (removal), Aquifer, Raise Land, Lower Land, Level Terrain
        if (Terraforming[i].bit) {
            MainInterfaceVar->set_bubble_text(j++ + 17,
                StringTable->get((int)Terraforming[i].name)); // 17-31
        }
    }
    MainInterfaceVar->set_bubble_text(32, StringTable->get((int)Natural[LM_JUNGLE].name_short));
    MainInterfaceVar->set_bubble_text(33, StringTable->get((int)Natural[LM_DUNES].name_short));
    MainInterfaceVar->set_bubble_text(34, StringTable->get((int)Natural[LM_URANIUM].name_short));
    MainInterfaceVar->set_bubble_text(35, StringTable->get((int)Natural[LM_GEOTHERMAL].name_short));
    return false;
}

/*
Purpose: Attempt to read the setting's value from the ini file.
// ORIGINAL: 0x0059D980 ?prefs_get@@YAPADPADPADH@Z 0x0059D980-0x0059DA19
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
inline LPSTR __cdecl prefs_get(LPCSTR key_name, LPCSTR default_value, BOOL use_default) {
    if (use_default ||
        (GetPrivateProfileStringA(PrefsSection, "Prefs Format", "0", TextBufferGetPtr, 256,
            PrefsFile), atoi(TextBufferGetPtr) != 12)) {
        strcpy_s(TextBufferGetPtr, 256, default_value);
    } else {
        GetPrivateProfileStringA(PrefsSection, key_name, default_value, TextBufferGetPtr, 256,
            PrefsFile);
    }
    return Txt.update();
}

/*
Purpose: Append a decimal number to the StringTemp buffer.
// ORIGINAL: 0x0050B8A0 ?say_num@@YAXH@Z 0x0050B8A0-0x0050B8CA BYTE_EXACT
// body      src/strings.h
// size      42 bytes
// prototype void (__cdecl ?say_num@@YAXH@Z)(int)
// kind      game
Return Value: n/a
Status: Complete
*/

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
    return prefs_get("Laptop", 0, false) ? base_prefs : base_prefs  // 0xA3E1DD16 : 0xBBE1DD96
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
    return prefs_get("Laptop", 0, false) ? base_prefs2 : base_prefs2 | MPREF_AV_SLIDING_SCROLLBARS;
}

/*
Purpose: Get the default value for the warning pop-up preferences.
// ORIGINAL: 0x0059DB20 ?default_warn@@YAHXZ 0x0059DB20-0x0059DB26 BYTE_EXACT
// symbol    ?default_warn@@YAIXZ
// size      6 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Default warning preferences
Status: Complete
*/
// BODY IN alpha.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


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
inline int __cdecl prefs_get(LPCSTR key_name, int default_value, BOOL use_default) {
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

/*
Purpose: Read the faction filenames and search for keys from the ini file (SMACX only). This has 
         the added effect of forcing the player's search_key to be set to the filename value. 
         Rewrote almost the entire function because of how terrible the original code logic was.
// ORIGINAL: 0x0059DBD0 ?prefs_fac_load@@YAXXZ 0x0059DBD0-0x0059DCE8
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
    if (ExpansionEnabled) {
        char returned_string[256];
        GetPrivateProfileStringA(PrefsSection, "Prefs Format", "0", returned_string, 256, 
            PrefsFile);
        if (atoi(returned_string) == 12) {
            for (int i = 1; i < MaxPlayerNum; i++) {
                StringTemp[0] = 0;
                strcat(StringTemp, "Faction ");
                say_num(i);
                GetPrivateProfileStringA(PrefsSection, StringTemp, Players[i].filename,
                    returned_string, 256, PrefsFile);
                strncpy_s(Players[i].filename, returned_string, 24);
                strncpy_s(Players[i].search_key, returned_string, 24);
            }
        } else {
            // use separate loop rather than check "Prefs Format" value each time in single loop
            for (int i = 1; i < MaxPlayerNum; i++) {
                strncpy_s(Players[i].search_key, Players[i].filename, 24);
            }
        }
    }
}

/*
Purpose: Convert the specified value to a binary string for use by various preferences.
Original Offset: n/a
Return Value: Binary string
Status: Complete
*/
// MOVED BEFORE prefs_load, and `inline`: VC6 only folds a same-TU call when
// it has already seen the full definition, and prefs_load's four call sites
// (0x0059DEFB-area, 0x0059E228, 0x0059E314, 0x0059E400 - all inlined in the
// image, no `call` to a separate function at all) need the body available
// beforehand.
inline LPSTR __cdecl prefs_get_binary(int value) {
    // NOT a local buffer/std::string: the image builds this directly into
    // the global `StringTemp` (0x009B86A0) - `mov byte ptr [0x9b86a0], 0`
    // then a `strcat` per bit at, e.g., 0x0059DEF8-0x0059DF58 - and the
    // caller uses THAT buffer as the `default_value` argument to the
    // (likewise inlined) string `prefs_get` immediately after. A
    // std::string return here forces an SEH frame onto every caller that
    // the image's plain `push ebp` prologue does not have.
    StringTemp[0] = 0;
    for (int shift = 31, non_pad = 0; shift >= 0; shift--) {
        if ((1 << shift) & value) {
            non_pad = 1;
            strcat_s(StringTemp, sizeof(StringTemp), "1");
        } else if (non_pad || shift < 8) {
            strcat_s(StringTemp, sizeof(StringTemp), "0");
        }
    }
    return StringTemp;
}

/*
Purpose: Load the most common preferences from the game's ini to globals.
// ORIGINAL: 0x0059DCF0 ?prefs_load@@YAXH@Z 0x0059DCF0-0x0059E502
// RULED-OUT: `__forceinline` on the two helpers, zero measured effect under every flag set tried, reverted to plain `inline`.
// RULED-OUT: dominant defect was std::string/std::stringstream (the "Custom World" default and `prefs_get_binary`'s return), which forces an SEH frame this function's actual `push ebp; sub esp, 0x54; push ebx/esi/edi` prologue does not have - first divergence was INSTRUCTION 0 before this. `prefs_get_binary` now writes directly into the global `StringTemp` (0x009B86A0), matching the image's `mov byte ptr [0x9b86a0], 0` + per-bit `strcat`, and the "Custom World" default is the same StringTemp+strcat loop with a SIGNED `int i` (image: `cmp esi, 7; jl`, not `jb`). Fixing this alone cannot get further: `prefs_get_binary` and both `prefs_get` overloads are `inline` (moved before first use where needed) since the image inlines them at MOST call sites, but the image's exact per-site mix (e.g. int-overload: 4 inlined + 3 real calls) is a VC6 codegen heuristic no source spelling reproduced here -
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
    set_language(prefs_get("Language", 0, false));
    prefs_get("Difficulty", 0, false);
    DefaultPrefs->difficulty = text_item_number();
    prefs_get("Map Type", 0, false);
    DefaultPrefs->map_type = text_item_number();
    prefs_get("Top Menu", 0, false);
    DefaultPrefs->top_menu = text_item_number();
    prefs_get("Faction", 1, false);
    DefaultPrefs->faction_id = text_item_number();
    uint32_t prefs = default_prefs();
    if (DefaultPrefs->difficulty < DLVL_TALENT) {
        prefs |= PREF_BSC_TUTORIAL_MSGS;
    }
    prefs_get("Preferences", prefs_get_binary(prefs), use_default);
    AlphaIniPrefs->preferences = text_item_binary();
    prefs_get("More Preferences", prefs_get_binary(default_prefs2()), use_default);
    AlphaIniPrefs->more_preferences = text_item_binary();
    prefs_get("Semaphore", "00000000", use_default);
    AlphaIniPrefs->semaphore = text_item_binary();
    prefs_get("Customize", 0, false);
    AlphaIniPrefs->customize = text_item_number();
    prefs_get("Rules", prefs_get_binary(default_rules()), use_default);
    AlphaIniPrefs->rules = text_item_binary();
    prefs_get("Announce", prefs_get_binary(default_warn()), use_default);
    AlphaIniPrefs->announce = text_item_binary();
    // NOT std::stringstream: the image builds this directly into the global
    // `StringTemp`, exactly like `prefs_get_binary` above - `mov byte ptr
    // [0x9b86a0], 0` at 0x0059E405, then a `strcat` per entry at
    // 0x0059E423, `cmp esi, 7; jl` (SIGNED) at 0x0059E42C. Any std::string
    // or std::stringstream local here forces the SEH frame the image's
    // plain `push ebp` prologue does not have.
    StringTemp[0] = 0;
    for (int i = 0; i < 7; i++) {
        strcat_s(StringTemp, sizeof(StringTemp), i ? "1, " : "2, ");
    }
    prefs_get("Custom World", StringTemp, use_default);
    for (i = 0; i < 7; i++) {
        AlphaIniPrefs->custom_world[i] = text_item_number();
    }
    prefs_get("Time Controls", 1, use_default);
    AlphaIniPrefs->time_controls = text_item_number();
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
    if (tgl_binary) {
        for (int non_pad = 0, shift = 31; shift >= 0; shift--) {
            if ((1 << shift) & value) {
                non_pad = 1;
                strcat_s(StringTemp, sizeof(StringTemp), "1");
            } else if (non_pad || shift < 8) {
                strcat_s(StringTemp, sizeof(StringTemp), "0");
            }
        }
    } else {
        char num_buf[80];
        _itoa(value, num_buf, 10);
        strcat_s(StringTemp, sizeof(StringTemp), num_buf);
    }
    WritePrivateProfileStringA(PrefsSection, key_name, StringTemp, PrefsFile);
}

/*
Purpose: Save the most common preferences from memory to the game's ini. 
// ORIGINAL: 0x0059E5D0 ?prefs_save@@YAXH@Z 0x0059E5D0-0x0059E946
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
    prefs_put("Difficulty", DefaultPrefs->difficulty, false);
    prefs_put("Map Type", DefaultPrefs->map_type, false);
    prefs_put("Top Menu", DefaultPrefs->top_menu, false);
    prefs_put("Faction", DefaultPrefs->faction_id, false);
    prefs_put("Preferences", AlphaIniPrefs->preferences, true);
    prefs_put("More Preferences", AlphaIniPrefs->more_preferences, true);
    prefs_put("Semaphore", AlphaIniPrefs->semaphore, true);
    prefs_put("Announce", AlphaIniPrefs->announce, true);
    prefs_put("Rules", AlphaIniPrefs->rules, true);
    prefs_put("Customize", AlphaIniPrefs->customize, false);
    std::stringstream ss;
    for (uint32_t i = 0; i < 7; i++) {
        if (i != 0) {
            ss << ", "; // removed last trailing comma
        }
        ss << AlphaIniPrefs->custom_world[i];
    }
    std::string custom_world_str = ss.str();
    prefs_put("Custom World", custom_world_str.c_str());
    prefs_put("Time Controls", AlphaIniPrefs->time_controls, false);
    if (save_factions && ExpansionEnabled) {
        for (uint32_t i = 1; i < MaxPlayerNum; i++) {
            sprintf_s(StringTemp, sizeof(StringTemp), "Faction %d", i);
            prefs_put(StringTemp, Players[i].filename);
        }
    }
}

/*
Purpose: Set the internal game preference globals from the ini setting globals.
// ORIGINAL: 0x0059E950 ?prefs_use@@YAXXZ 0x0059E950-0x0059E973 SEMANTIC
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
    const uint32_t preferences = AlphaIniPrefs->preferences;
    const uint32_t more = AlphaIniPrefs->more_preferences;
    const uint32_t announce = AlphaIniPrefs->announce;
    GamePreferences = preferences;
    GameMorePreferences = more;
    GameWarnings = announce;
}

/*
Purpose: Parse the #LABELS section inside the labels.txt file.
// ORIGINAL: 0x00616A00 ?labels_init@@YAHXZ 0x00616A00-0x00616A93
// RULED-OUT: 40/45 plateau - the image stores `Labels->count` THEN reuses the
//            same register for `shl eax,2` (no reload); every source form
//            tried instead computes `count*4` into a second register (`lea
//            ecx,[eax*4]`) before the store: local `int count`, a separate
//            `int byte_count = count*4;` statement, `<<2` instead of `*4`,
//            and a nested `(Labels->count = text_item_number()) * 4` all
//            produced the identical `lea`+deferred-store shape. VC6
//            scheduling choice, not a source-form lever found here.
// size      147 bytes
// prototype 
// callers   3   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005FD530 0x005FD550 0x005FD570 0x005FD5E0 0x005FD740 0x00644EF2
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl labels_init() {
    labels_shutdown();
    if (text_open("labels", "labels")) {
        return true;
    }
    text_get();
    Labels->count = text_item_number();
    Labels->strings_ptr = (LPSTR)mem_get(Labels->count * 4);
    if (!Labels->strings_ptr) {
        return true;
    }
    for (int i = 0; i < Labels->count; i++) {
        *((LPSTR *)Labels->strings_ptr + i) = text_string();
    }
    text_close();
    return false;
}

/*
Purpose: Shutdown or reset the labels global variable.
// ORIGINAL: 0x006169D0 ?labels_shutdown@@YAXXZ 0x006169D0-0x006169F7 BYTE_EXACT
// size      39 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
// BODY IN alpha.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the game's language.
// ORIGINAL: 0x00627100 ?set_language@@YAXH@Z 0x00627100-0x0062710A BYTE_EXACT
// size      10 bytes
// prototype void (__cdecl ?set_language@@YAXH@Z)(int lang)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN alpha.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the label string and concatenate it to the stringTemp buffer.
// ORIGINAL: 0x005A5880 ?say_label@@YAXH@Z 0x005A5880-0x005A58AA
// size      42 bytes
// prototype void (__cdecl ?say_label@@YAXH@Z)(int labelID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
Return Value: n/a
Status: Complete
*/
void __cdecl say_label(int label_offset) {
    strcat_s(StringTemp, 1032, label_get(label_offset));
}

/*
Purpose: Get the label string from a label offset. Created to reduce code complexity.
Original Offset: n/a
Return Value: Pointer to label string
Status: Complete
*/
LPSTR __cdecl label_get(int label_offset) {
    return StringTable->get((int)*((LPSTR *)Labels->strings_ptr + label_offset));
}
