/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

// Writable formatter strings at distinct pinned operand addresses.
// These names describe their uses; original allocation bounds remain unknown.
// RECOVERY_OBJECT: v1 0x0068F118 _DefenseAAAOpenText
static char DefenseAAAOpenText[] = "<";
// RECOVERY_OBJECT: v1 0x0068F11C _DefenseCommJammerText
static char DefenseCommJammerText[] = "+";
// RECOVERY_OBJECT: v1 0x0068F120 _DefenseTranceText
static char DefenseTranceText[] = "t";
// RECOVERY_OBJECT: v1 0x0068F124 _DefenseAAACloseText
static char DefenseAAACloseText[] = ">";
// RECOVERY_OBJECT: v1 0x0068F128 _DefensePulseText
static char DefensePulseText[] = "p";
// RECOVERY_OBJECT: v1 0x0068F12C _DefenseResonanceText
static char DefenseResonanceText[] = "r";
// RECOVERY_OBJECT: v1 0x0068F130 _Stats3OffenseUnknownText
static char Stats3OffenseUnknownText[] = "?";
// RECOVERY_OBJECT: v1 0x0068F134 _Stats3OffenseExtremeText
static char Stats3OffenseExtremeText[] = "*";
// RECOVERY_OBJECT: v1 0x0068F138 _Stats3OffenseDefenseSeparatorText
static char Stats3OffenseDefenseSeparatorText[] = "-";
// RECOVERY_OBJECT: v1 0x0068F13C _Stats3DefenseUnknownText
static char Stats3DefenseUnknownText[] = "?";
// RECOVERY_OBJECT: v1 0x0068F140 _Stats3DefenseSpeedSeparatorText
static char Stats3DefenseSpeedSeparatorText[] = "-";
// RECOVERY_OBJECT: v1 0x0068F144 _Stats3ReactorPrefixText
static char Stats3ReactorPrefixText[] = "*";
#include "general.h"
#include "veh.h"
#include "alpha.h"
/*
Purpose: Generate offensive display string for the specified prototype. Replaced existing non-safe
         strcat with string. Original function took a 2nd parameter with char buffer and didn't
         return anything.
// ORIGINAL: 0x0057D560 ?say_offense@@YAXPADH@Z 0x0057D560-0x0057D6C5 BYTE_EXACT
// size      357 bytes
// prototype void (__cdecl ?say_offense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C0DB0 0x005C1150 0x00645470 0x0064FC88
Return Value: Offensive display string
Status: Complete
*/
void __cdecl say_offense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, "<");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, "(");
    }
    int off_rating = offense_proto(proto_id, -1, 0);
    _itoa(off_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_DROP_POD)) {
        strcat(stat, "^");
    }
    if (has_abil(proto_id, ABL_AMPHIBIOUS)) {
        strcat(stat, "~");
    }
    if (has_abil(proto_id, ABL_NERVE_GAS)) {
        strcat(stat, "x");
    }
    if (has_abil(proto_id, ABL_EMPATHIC)) {
        strcat(stat, "e");
    }
    if (has_abil(proto_id, ABL_BLINK_DISPLACER)) {
        strcat(stat, "!");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, ")");
    }
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, ">");
    }
    uint8_t weapon_id = VehPrototypes[proto_id].weapon_id;
    if (weapon_id == WPN_RESONANCE_LASER || weapon_id == WPN_RESONANCE_BOLT) {
        strcat(stat, "r");
    }
}
/*
Purpose: Generate defense display string for the specified prototype. Replaced existing non-safe
         strcat with string. Original function took a 2nd parameter with char buffer and didn't
         return anything.
// ORIGINAL: 0x0057D6D0 ?say_defense@@YAXPADH@Z 0x0057D6D0-0x0057D7C4 BYTE_EXACT
// size      244 bytes
// prototype void (__cdecl ?say_defense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C1290 0x00645470 0x0064FC88
Return Value: Defense display string
Status: Complete
*/
void __cdecl say_defense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, DefenseAAAOpenText);
    }
    int def_rating = armor_proto(proto_id, -1, 0);
    _itoa(def_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_COMM_JAMMER)) {
        strcat(stat, DefenseCommJammerText);
    }
    if (has_abil(proto_id, ABL_TRANCE)) {
        strcat(stat, DefenseTranceText);
    }
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, DefenseAAACloseText);
    }
    if (VehPrototypes[proto_id].armor_id == ARM_PULSE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_PULSE_8_ARMOR) {
        strcat(stat, DefensePulseText);
    }
    if (VehPrototypes[proto_id].armor_id == ARM_RESONANCE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_RESONANCE_8_ARMOR) {
        strcat(stat, DefenseResonanceText);
    }
}
/*
Purpose: Generate stats string for specified prototype. Replaced existing non-safe strcat with
         string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D7D0 ?say_stats_3@@YAXPADH@Z 0x0057D7D0-0x0057D8D3 BYTE_EXACT
// TRIED: else-if branch order off_rating<0/off_rating<99/else folded the "?"
//            and "*" strcat tails together (29/92); reordering to
//            off_rating<0/off_rating>=99/else (matching the disasm's own branch
//            order) got to 51/92. def_rating stored to a named int8_t local, or
//            inlined as Armor[...].armor_id].defense_rating, made no further
//            difference; plateaus on the def_rating address computation
//            (shl 4 + disp vs add const + shl 4), 4 spellings tried.
// LEVER 2026-09-03: the speed division is SIGNED - `speed_proto(proto_id) /
//            Rules.move_rate_roads` gives the image's `cdq / idiv`,
//            where the uncast `uint32_t` field gave `xor edx,edx / div`.
//            Same cast now at all three say_stats* sites (0x57D7D0,
//            0x57D8E0, 0x57DAA0).
// size      259 bytes
// prototype void (__cdecl ?say_stats_3@@YAXPADH@Z)(int8*, int protoID)
// callers   13   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_3(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        strcat(stat, Stats3OffenseUnknownText); // PSI
    } else if (off_rating >= 99) {
        strcat(stat, Stats3OffenseExtremeText);
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, Stats3OffenseDefenseSeparatorText);
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, Stats3DefenseUnknownText);
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, Stats3DefenseSpeedSeparatorText);
    int speed = speed_proto(proto_id) / Rules.move_rate_roads;
    _itoa(speed, num_buf, 10);
    strcat(stat, num_buf);
    if (VehPrototypes[proto_id].reactor_id > RECT_FISSION) {
        strcat(stat, Stats3ReactorPrefixText);
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
    }
}
/*
Purpose: Generate stats string for specified prototype. List whether prototype is psi, sea or air.
         Replaced existing non-safe strcat with string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D8E0 ?say_stats_2@@YAXPADH@Z 0x0057D8E0-0x0057DA94 BYTE_EXACT
// TRIED: label_get(196/163/162) calls (25/140); open-coding
//            StringTable.get((int)*((LPSTR*)Labels.strings_ptr + N)) plus the
//            off_rating<0/>=99/else branch order got to 58/140. def_rating via
//            get_proto_defense_rating() vs inlined Armor[...] made no further
//            difference; plateaus on the same shl-vs-add-const address split
//            seen at 0x0057D7D0. Call count is 19/21 (call_diff): same
//            get(Labels[196])+strcat 'Psi' CSE across the off_rating<0 and
//            def_rating<0 arms as 0x0057DAA0 - the compiler shares the pair,
//            the image keeps two call sites.
// size      436 bytes
// prototype void (__cdecl ?say_stats_2@@YAXPADH@Z)(int8*, int protoID)
// callers   4   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_2(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        strcat(stat, label_get(196)); // 'Psi'
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, label_get(196)); // 'Psi'
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    int speed = speed_proto(proto_id) / Rules.move_rate_roads;
    _itoa(speed, num_buf, 10);
    strcat(stat, num_buf);
    if (get_proto_triad(proto_id) == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, label_get(163)); // 'Sea'
    }
    if (get_proto_triad(proto_id) == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, label_get(162)); // 'Air'
    }
    if (VehPrototypes[proto_id].reactor_id > 1) {
        strcat(stat, "*");
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
    }
}
/*
Purpose: Generate verbose stats string for specified prototype. Used by Design Workshop and Military
         Command Nexus. Replaced existing non-safe strcat with string. Reworked to integrate with
         existing C code.
// ORIGINAL: 0x0057DAA0 ?say_stats@@YAXPADHPAD@Z 0x0057DAA0-0x0057DED8
// RECOVERY_BINDING: v1 {"evidence": ["pinned:57DCEA-57DCF0 strcat references682E90 comma-space-NUL", "pinned:57DDF0 triad arms separately append682820 space-NUL before localized Sea/Air labels"], "reason": "Retain the original comma-space separator followed by the separate triad space. The visible doubled space is original formatting, not a new output correction. Complete formatter matching and runtime remain obligations."}
// symbol    ?say_stats@@YAXPADH0@Z
// TRIED: std::string body called 82 vs the image's 44 (call_diff); rebuilt
//            as char pointer + strcat/sprintf in the image's own branch shape (goto
//            shared "defense+spacer+speed" tail, matching the two physical
//            jumps into 0x57dd70) got the call count to 42/44. label_get(N)
//            calls had to become the open-coded
//            StringTable.get((int)*((LPSTR*)Labels.strings_ptr + N)) - the
//            image never calls a separate label helper here. The
//            `custom_spacer ? custom_spacer : "/"` after the offense append
//            is two strcat call sites in the image (if/else, not a ternary
//            merged into one call); the same spacer after the defense append
//            IS one call site (a shared "push ebx or literal, then push+call"
//            tail) - only the first needed splitting.
//            Still 2 calls short: the compiler CSEs the two identical
//            `StringTable.get(Labels[196])` + strcat 'Psi' pairs (one on the
//            off_rating<0 arm, one on the def_rating<0 arm) into one shared
//            block, where the image keeps them as two separate call sites.
//            No source spelling tried defeated that merge. Also plateaus on
//            prologue shape (push ebp/mov ebp,esp/sub esp,0x50 vs an
//            esp-relative frame) picked by the flag search's own similarity
//            metric; forcing /Oy- narrows but does not close it.
// size      1080 bytes
// prototype void (__cdecl ?say_stats@@YAXPADHPAD@Z)(int8*, int protoID, int8*)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer) {
    char num_buf[80];
    if (VehPrototypes[proto_id].plan == PLAN_RECONNAISANCE && get_proto_triad(proto_id) == TRIAD_LAND && get_proto_offense_rating(proto_id) == 1 && get_proto_defense_rating(proto_id) == 1
        && !VehPrototypes[proto_id].ability_flags) {
        strcat(stat, StringTable.get(PlansFullName[3])); // 'Explore/Defense'
        strcat(stat, ", ");
    } else if (Weapon[VehPrototypes[proto_id].weapon_id].mode < 3) { // Projectile, energy, missile
        strcat(stat, StringTable.get((VehPrototypes[proto_id].plan != PLAN_DEFENSIVE || (get_proto_offense_rating(proto_id) >= 0 && get_proto_offense_rating(proto_id) <= get_proto_defense_rating(proto_id)))
            ? PlansShortName[VehPrototypes[proto_id].plan] : Labels.strings_ptr[312])); // 'Combat'
        strcat(stat, ", ");
    }
    if (get_proto_offense_rating(proto_id) < 0 || Weapon[VehPrototypes[proto_id].weapon_id].mode < 3) {
        if (get_proto_offense_rating(proto_id) < 0) {
            strcat(stat, StringTable.get(Labels.strings_ptr[196])); // 'Psi'
        } else {
            say_offense(stat, proto_id);
        }
        if (custom_spacer) {
            strcat(stat, custom_spacer);
        } else {
            strcat(stat, "/");
        }
        goto append_defense_tail;
    } else if (get_proto_defense_rating(proto_id) != 1 || VehPrototypes[proto_id].ability_flags || (Chassis[VehPrototypes[proto_id].chassis_id].speed != 1
        && (Weapon[VehPrototypes[proto_id].weapon_id].mode != WPN_MODE_TRANSPORT || VehPrototypes[proto_id].chassis_id != CHSI_FOIL))) {
        strcat(stat, StringTable.get(PlansShortName[Weapon[VehPrototypes[proto_id].weapon_id].mode]));
        if (VehPrototypes[proto_id].plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        strcat(stat, ", ");
append_defense_tail:
        if (get_proto_defense_rating(proto_id) < 0) {
            strcat(stat, StringTable.get(Labels.strings_ptr[196])); // 'Psi'
        } else {
            say_defense(stat, proto_id);
        }
        strcat(stat, custom_spacer ? custom_spacer : "/");
        int speed = speed_proto(proto_id) / Rules.move_rate_roads;
        _itoa(speed, num_buf, 10);
        strcat(stat, num_buf);
    } else {
        strcat(stat, StringTable.get(PlansFullName[Weapon[VehPrototypes[proto_id].weapon_id].mode]));
        if (VehPrototypes[proto_id].plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        if (get_proto_triad(proto_id)) { // sea, air
            strcat(stat, ", "); // Original separator; the later triad space remains.
        }
    }
    if (get_proto_triad(proto_id) == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable.get(Labels.strings_ptr[163])); // 'Sea'
    }
    if (get_proto_triad(proto_id) == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable.get(Labels.strings_ptr[162])); // 'Air'
    }
    if (VehPrototypes[proto_id].reactor_id > 1) {
        strcat(stat, " ");
        strcat(stat, "(");
        strcat(stat, "*");
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
        strcat(stat, ")");
    }
}
