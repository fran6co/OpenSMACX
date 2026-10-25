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
#include "strings.h"
#include "alpha.h"
#include "technology.h"
#include "veh.h"
#include "base.h"
#include "faction.h"

// Keep these application formatters separate from Strings::get's definition so
// VC6's same-TU /Ob2 inliner cannot absorb the original get calls. Their shared
// storage and Strings implementation remain owned by strings.cpp.

// Append a caller-supplied string to the shared scratch buffer.
// ORIGINAL: 0x00500130 ?says@@YAXPBD@Z 0x00500130-0x00500146 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500130-0x00500146 calls strcat with 0x009B86A0", "git:0cb1eb5c:src/recovered/units/00500130.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original appends a caller string into the source-owned shared StringTemp buffer."}
void __cdecl says(const char *text) {
    strcat(StringTemp, text);
}

// The five part-name formatters share the same label table, string table,
// scratch buffer, and rules records as the neighboring say_* functions.
// Their optional suffixes deliberately append a label to StringTemp before
// continuing the destination; that side effect is present in the original.
// ORIGINAL: 0x00590510 ?say_reactor@@YAXPAXHH@Z 0x00590510-0x005905B5
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590510-0x005905B5;seed:src/unrecovered/00590510.cpp;source:src/veh.h:Reactor;source:src/strings.h:StringTable"], "reason": "This free formatter reads the real Reactor record and shared Strings and Labels objects, matching the original name, call ABI, and bounded body."}
// symbol    ?say_reactor@@YAXPADHH@Z
// TRIED: MISMATCH #4 push/mov, local buffer layout differs
// TRIED: _itoa needs a real signature -> declared as itoa_impl (own name), since the scaffold's nullary _itoa can't be redeclared (C2733)
void __cdecl say_reactor(char *text, int reactor_id, int with_detail) {
    strcat(text, StringTable.get(Reactor[reactor_id].name));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            Labels.strings_ptr[0x13C / 4]));
        strcat(text, ": ");
        char number[80];
        _itoa(reactor_id + 1, number, 10);
        strcat(text, number);
        strcat(text, ")");
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005905C0 ?say_weapon@@YAXPAXHH@Z 0x005905C0-0x005906EC
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005905C0-0x005906EC;seed:src/recovered/units/005905c0.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Weapon records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
// symbol    ?say_weapon@@YAXPADHH@Z
void __cdecl say_weapon(char *text, int weapon_id, int with_detail) {
    const RulesWeapon &weapon = Weapon[weapon_id];
    strcat(text, StringTable.get(weapon.name));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        if (weapon.offense_rating == 0) {
            strcat(text, StringTable.get(
                Labels.strings_ptr[0x284 / 4]));
        } else {
            strcat(text, StringTable.get(
                Labels.strings_ptr[0x204 / 4]));
            strcat(text, ": ");
            if (weapon.offense_rating < 0) {
                strcat(text, StringTable.get(
                    Labels.strings_ptr[0x310 / 4]));
            } else {
                char number[80];
                _itoa(weapon.offense_rating, number, 10);
                strcat(text, number);
            }
            for (int i = 0; i < weapon.mode; ++i) {
                strcat(text, "*");
            }
        }
        strcat(text, ")");
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005906F0 ?say_armor@@YAXPAXHH@Z 0x005906F0-0x005907F5
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005906F0-0x005907F5;seed:src/recovered/units/005906f0.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Armor records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
// symbol    ?say_armor@@YAXPADHH@Z
void __cdecl say_armor(char *text, int armor_id, int with_detail) {
    const RulesArmor &armor = Armor[armor_id];
    strcat(text, StringTable.get(armor.name));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            Labels.strings_ptr[0x154 / 4]));
        strcat(text, ": ");
        if (armor.defense_rating < 0) {
            strcat(text, StringTable.get(
                Labels.strings_ptr[0x310 / 4]));
        } else {
            char number[80];
            _itoa(armor.defense_rating, number, 10);
            strcat(text, number);
        }
        for (int i = 0; i < armor.mode; ++i) {
            strcat(text, "*");
        }
        strcat(text, ")");
    }
}

// ORIGINAL: 0x00590800 ?say_chassis@@YAXPAXHH@Z 0x00590800-0x0059090D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590800-0x0059090D;seed:src/recovered/units/00590800.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00590800.cpp"], "reason": "This free formatter uses the established Chassis records and shared Strings and Labels objects with the original name, call ABI, and bounded body. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?say_chassis@@YAXPADHH@Z
// kind      game
void __cdecl say_chassis(char *text, int chassis_id, int with_detail) {
    const RulesChassis &chassis = Chassis[chassis_id];
    strcat(text, StringTable.get(chassis.offsv_1_name));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            Labels.strings_ptr[0x140 / 4]));
        strcat(text, ": ");
        char number[80];
        _itoa(chassis.speed, number, 10);
        strcat(text, number);
        if (chassis.triad == 1 || chassis.triad == 2) {
            strcat(text, " ");
            const int offset = chassis.triad == 1 ? 0x28C : 0x288;
            strcat(text, StringTable.get(
                Labels.strings_ptr[offset / 4]));
        }
        strcat(text, ")");
    }
}

// ORIGINAL: 0x00590B30 ?say_techs@@YAXPAXHH@Z 0x00590B30-0x00590B9A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590B30-0x00590B9A;seed:src/recovered/units/00590b30.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00590b30.cpp"], "reason": "This free formatter uses the established Technology records and shared Strings and Labels objects with the original name, call ABI, and bounded body. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?say_techs@@YAXPADHH@Z
// kind      game
void __cdecl say_techs(char *text, int tech_id, int with_detail) {
    say_tech(text, tech_id, with_detail);
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        LPSTR *label_table = Labels.strings_ptr;
        LPSTR label = label_table[0x274 / 4];
        LPSTR detail = StringTable.get(label);
        strcat(StringTemp, detail);
        strcat(text, ")");
    }
}

/*
Purpose: Append a facility's name to a scratch text: the facility label from
         base.h's Facility table (0x30-byte records, name first), and with
         the cost flag the " (" + cost-label + ")" tail whose label is read
         out of the label table through Labels.strings_ptr + 0x26C and
         parked in the shared StringTemp.

// ORIGINAL: 0x00590910 ?say_facility@@YAXPAXHH@Z 0x00590910-0x0059098E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["src/strings.h declares say_facility; strings.cpp owns StringTable and StringTemp and already includes alpha/technology data declarations.","The pinned label word at byte offset 0x26C is dword index 0x9B; existing label_get(0x9B) performs the same LPSTR-table lookup through Labels and StringTable.","The one-space/open/close strings are emitted as source-owned literals; each strcat stays in its original order."],"reason":"Move the existing free formatter into the real string-table owner, use its existing typed label_get for index 0x9B, and source-own the bounded literals without changing append order or StringTemp destination."}
// symbol    ?say_facility@@YAXPADHH@Z
// size      126 bytes
// prototype void (__cdecl ?say_facility@@YAXPAXHH@Z)(int8* text, int32 facilityID, int32 withCost)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// notes     Seed provenance (LEVER string-routines-are-calls): the string
//           routines must stay real calls - an earlier inline expansion lost
//           five bytes (121 vs 126, MISMATCH at #7) - which stdafx.h's
//           project-wide `#pragma function(strcat, ...)` provides.
Return Value: n/a
Status: Measured BYTE_EXACT here in place (wave 8 record-then-retire); the
        seed src/unrecovered/00590910.cpp is retired and its claim lives on
        the marker above. The spelling is the retained seed's own statement
        order: the receiver hoisted into a local before the Facility index,
        and the second label read split into int locals before its get -
        the original materializes StringTable's receiver in ECX ahead of the
        Facility address math, and the tail's value lands in EAX only when
        the label walk is stated stepwise (say_ability/say_society, whose
        scratch borrows ECX, keep the fused form).
*/
void __cdecl say_facility(char *text, int facility_id, int with_cost) {
    Strings *strings = &StringTable;
    LPSTR name = Facility[facility_id].name;
    strcat(text, strings->get(name));
    if (with_cost != 0) {
        strcat(text, " ");
        strcat(text, "(");
        LPSTR label = Labels.strings_ptr[0x9B];
        LPSTR detail = strings->get(label);
        strcat(StringTemp, detail);
        strcat(text, ")");
    }
}

/*
Purpose: Append an ability's name, and with the flag the " (" + detail +
         ")" tail, reading the ability label from veh.h's Ability table
         (0x1C-byte records, name first) and the detail label through
         Labels.strings_ptr + 0x290.

// ORIGINAL: 0x00590AA0 ?say_ability@@YAXPAXHH@Z 0x00590AA0-0x00590B22 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["src/strings.h declares say_ability; strings.cpp owns StringTable/StringTemp and includes veh.h for Ability.","The pinned label word at byte offset 0x290 is dword index 0xA4; existing label_get(0xA4) performs the same LPSTR-table lookup.","The one-space/open/close strings are emitted as source-owned literals in the original strcat order."],"reason":"Move the existing formatter beside StringTable and express the observed label index through label_get(0xA4); preserve call order, index/flag quirks, StringTemp destination and historical claim."}
// symbol    ?say_ability@@YAXPADHH@Z
// size      130 bytes
// prototype void (__cdecl ?say_ability@@YAXPAXHH@Z)(int8* text, int32 abilityID, int32 withDetail)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
Return Value: n/a
Status: Measured BYTE_EXACT here in place (wave 8 record-then-retire); the
        seed src/unrecovered/00590aa0.cpp is retired and its claim lives on
        the marker above. The Ability index is computed first and the
        receiver materialized at the get, because the original needs ECX
        for the 7-int scale and loads StringTable after it.
*/
void __cdecl say_ability(char *text, int ability_id, int with_detail) {
    LPSTR ability_name = Ability[ability_id].name;
    LPSTR s1 = StringTable.get(ability_name);
    strcat(text, s1);
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        LPSTR *label_table = Labels.strings_ptr;
        Strings *strings = &StringTable;
        LPSTR label = label_table[0xA4];
        LPSTR detail = strings->get(label);
        strcat(StringTemp, detail);
        strcat(text, ")");
    }
}

/*
Purpose: Append a social category's name, and with the flag the " (" +
         model + ")" tail, reading the category label from faction.h's
         SocialCategories table (0xD4-byte records, type first) and the
         model label through Labels.strings_ptr + 0x114.

// ORIGINAL: 0x00590A10 ?say_society@@YAXPAXHH@Z 0x00590A10-0x00590A93 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["src/strings.h declares say_society; strings.cpp owns StringTable/StringTemp; src/faction.h declares SocialCategories.","The pinned label word at byte offset 0x114 is dword index 0x45; existing label_get(0x45) performs the same LPSTR-table lookup.","The one-space/open/close strings are emitted as source-owned literals in the original strcat order."],"reason":"Move the existing formatter beside StringTable, use label_get(0x45), and include the actual faction table declaration; preserve call order, model flag and claim."}
// symbol    ?say_society@@YAXPADHH@Z
// size      131 bytes
// prototype void (__cdecl ?say_society@@YAXPAXHH@Z)(int8* text, int32 societyID, int32 withModel)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
Return Value: n/a
Status: Measured BYTE_EXACT here in place (wave 8 record-then-retire); the
        seed src/unrecovered/00590a10.cpp is retired and its claim lives on
        the marker above. Like say_ability the 0xD4 scale borrows ECX, so
        the receiver materializes at the get despite the hoisted local.
*/
void __cdecl say_society(char *text, int society_id, int with_model) {
    Strings *strings = &StringTable;
    LPSTR type = SocialCategories[society_id].type;
    strcat(text, strings->get(type));
    if (with_model != 0) {
        strcat(text, " ");
        strcat(text, "(");
        char *model = label_get(0x45);
        strcat(StringTemp, model);
        strcat(text, ")");
    }
}
