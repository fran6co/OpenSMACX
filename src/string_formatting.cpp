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
#include "buffer.h"
#include "alpha.h"
#include "technology.h"
#include "veh.h"
#include "base.h"
#include "map.h"
#include "faction.h"

// Shared original punctuation storage, defined in socialwin.cpp.
extern char SocialWinColonSpaceText[];
extern char SocialWinCommaSpaceText[];

// Keep these application formatters separate from Strings::get's definition so
// VC6's same-TU /Ob2 inliner cannot absorb the original get calls. Their shared
// storage and Strings implementation remain owned by strings.cpp.

// The shared space is owned by BufferJoinSpace (buffer.cpp, 0x00682820).
// The existing RECOVERY_LITERAL anchors are retained as historical
// binding facts. This current source regrouping does not prove the
// original translation-unit home. Unaffected formatters independently
// corroborate all three starts: 26 original pushes across eight owners,
// including say_facility at 0x00590910. The moved functions are not
// treated as evidence of their own source home.
// These are logical one-character-plus-NUL spans, not enclosing allocations.
// The original spans are in writable .data; these bindings do not assert
// constness, runtime immutability, lifetime or linked COMDAT retention.
// Identical compiler symbols in other translation units remain independent.
// RECOVERY_LITERAL: v1 0x00682E9C 0x00682E9E ??_C@_01GFI@?$CI?$AA@ 0x00590BA0
// RECOVERY_LITERAL: v1 0x00682E98 0x00682E9A ??_C@_01KMMP@?$CJ?$AA@ 0x00590BA0

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
// ORIGINAL: 0x00590510 ?say_reactor@@YAXPAXHH@Z 0x00590510-0x005905B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590510-0x005905B5;seed:src/unrecovered/00590510.cpp;source:src/veh.h:Reactor;source:src/strings.h:StringTable"], "reason": "This free formatter reads the real Reactor record and shared Strings and Labels objects, matching the original name, call ABI, and bounded body."}
// symbol    ?say_reactor@@YAXPADHH@Z
// TRIED: MISMATCH #4 push/mov, local buffer layout differs
// TRIED: _itoa needs a real signature -> declared as itoa_impl (own name), since the scaffold's nullary _itoa can't be redeclared (C2733)
void __cdecl say_reactor(char *text, int reactor_id, int with_detail) {
    Strings *strings = &StringTable;
    LPSTR name = Reactor[reactor_id].name;
    strcat(text, strings->get(name));
    if (with_detail != 0) {
        strcat(text, BufferJoinSpace);
        strcat(text, "(");
        LPSTR label = Labels.strings_ptr[0x13C / 4];
        LPSTR detail = strings->get(label);
        strcat(StringTemp, detail);
        strcat(text, SocialWinColonSpaceText);
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
        strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            Labels.strings_ptr[0x140 / 4]));
        strcat(text, ": ");
        char number[80];
        _itoa(chassis.speed, number, 10);
        strcat(text, number);
        if (chassis.triad == 1 || chassis.triad == 2) {
            strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
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
        strcat(text, BufferJoinSpace);
        strcat(text, "(");
        char *model = label_get(0x45);
        strcat(StringTemp, model);
        strcat(text, ")");
    }
}
// Source-level formatter-family grouping proposed for the current tree.
// The original translation-unit homes of say_project and say_vehicles
// are unknown; this move preserves their mapped identities and does not
// claim historical source-file provenance.
// The legacy say_vehicles recovery note below is retained verbatim as
// history; its old generated-tool and no-build instructions do not
// describe this proposed production source owner.

/*
Purpose: Write a secret project's name, and its cost bracket when asked.
// ORIGINAL: 0x00590990 ?say_project@@YAXPAXHH@Z 0x00590990-0x00590A0E FILE BYTE_EXACT
// size      126 bytes
// prototype void (__cdecl ?say_project@@YAXPAXHH@Z)(void*, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// ONE image reference outside its own body, as a raw function pointer in the
//        file's .data, so the catalogue gives no parameter names; they are
//        read off the body (an output buffer, a project index, a flag).
//        SecretProjectRules is the 0x30-stride rules record whose first field
//        is the original LPSTR name, matching the Facility[] representation;
//        0x2CC into the label block is label_get(179).
// TRIED: (as 00590990.cpp) the artifact's `strcat(StringTemp, ...)`,
//        i.e. no append of StringTemp to the output - it is what the image
//        does, and it is kept here.
// symbol    ?say_project@@YAXPADHH@Z
// TRIED: 32/40. Everything left is evaluation order around the first
//        `StringTable.get`: the image hoists the receiver
//        (`mov ecx, 0x9b90d8`) to the top of the body, before the
//        `project_id * 0x30` index arithmetic, and reads the name into EAX;
//        this tree computes the argument first and reads the name into ECX,
//        which also costs the receiver load its own instruction. Spelling the
//        receiver through a local `Strings &`, or the name through a local
//        `LPSTR`, only adds the store the image does not have. Not chased
//        further this pass.
Return Value: n/a
Status: Complete
*/
void __cdecl say_project(char *output, int project_id, int with_cost) {
    Strings &string_table = StringTable;
    LPSTR project_name = Facility[FacilitySPStart + project_id].name;
    strcat(output, string_table.get(project_name));
    if (with_cost != 0) {
        strcat(output, BufferJoinSpace);
        strcat(output, "(");
        LPSTR cost_label = Labels.strings_ptr[179];
        strcat(StringTemp, StringTable.get(cost_label));
        strcat(output, ")");
    }
}

// ORIGINAL: 0x00590BA0 ?say_vehicles@@YAXPAXHH@Z 0x00590BA0-0x00590C14 FILE BYTE_EXACT
// symbol    ?say_vehicles@@YAXPADHH@Z
// size      116 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00590BA0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00590ba0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_vehicles@@YAXPAXHH@Z  at 0x00590BA0  (116 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl say_vehicles(char *text, int chassis_id, int with_height) {
    strcat(text, VehPrototypes[chassis_id].veh_name);
    if (with_height != 0) {
        strcat(text, BufferJoinSpace);
        strcat(text, "(");
        LPSTR label = Labels.strings_ptr[0x314 / 4];
        strcat(StringTemp, StringTable.get(label));
        strcat(text, ")");
    }
}


// ORIGINAL: 0x00592250 ?say_loc@@YAXPADHHHHH@Z 0x00592250-0x005923FD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Group the free location formatter with the other say_* formatters and their independently evidenced punctuation; share the existing source-owned space and comma-space storage. This is a current semantic home, not proof of the original translation unit.", "evidence": ["src/veh.h:say_loc;src/map.h:map_loc", "src/string_formatting.cpp:independently corroborated parenthesis RECOVERY_LITERAL bindings", "src/buffer.cpp:BufferJoinSpace;src/socialwin.cpp:SocialWinCommaSpaceText", "image:0x00592250-0x005923FD uses 0x00682820,0x00682E90,0x00682E9C,0x00682E98"]}
// size      429 bytes
// prototype void (__cdecl ?say_loc@@YAXPADHHHHH@Z)(int8*, int xCoord, int yCoord, int, int, int)
// callers   4   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592250
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_loc@@YAXPADHHHHH@Z  at 0x00592250  (429 bytes)
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
void __cdecl say_loc(char *text, int x, int y, int want_name, int trailer_mode,
                     int want_coords) {
    char number[80];
    int base_id = base_at(x, y);
    if (want_name != 0) {
        if (base_id < 0) {
            want_coords = 0;
            base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (base_id < 0) {
                goto trailer;
            }
            strcat(text, label_get(0xf8 / 4));
            strcat(text, BufferJoinSpace);
        } else {
            goto skip_first_check;
        }
    }
    if (base_id < 0) {
        goto trailer;
    }
skip_first_check:
    if (want_coords != 0) {
        strcat(text, label_get(0x20 / 4));
        strcat(text, BufferJoinSpace);
    }
    strcat(text, Bases[base_id].name_string);
    if (trailer_mode != 0) {
        strcat(text, BufferJoinSpace);
    }

trailer:
    if (trailer_mode == 1 || (trailer_mode == 2 && base_id < 0)) {
        strcat(text, "(");
        _itoa(x, number, 10);
        strcat(text, number);
        strcat(text, SocialWinCommaSpaceText);
        _itoa(y, number, 10);
        strcat(text, number);
        strcat(text, ")");

        int flags = GameDataStateStorage.fields_.game_state_;
        if ((flags & 0x40) != 0 && (flags & 0x1000) != 0) {
            strcat(StringTemp, BufferJoinSpace);
            Map *tile = map_loc(x, y);
            _itoa(tile->region, number, 10);
            strcat(StringTemp, number);
        }
    }
}
