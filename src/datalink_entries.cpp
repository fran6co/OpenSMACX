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
#include "datalink.h"
#include "strings.h"
#include "faction.h"
#include "buffer.h"
#include "alpha.h"
#include "technology.h"
#include "veh.h"
#include "base.h"
#include "map.h"

// Shared punctuation storage remains owned by socialwin.cpp.
extern char SocialWinColonSpaceText[];

// Recovery names for two distinct writable star strings. The original calls
// in say_weapon and say_armor address separate two-byte strings; this does
// not establish their original allocation bounds or translation-unit home.
// RECOVERY_OBJECT: v2 0x0068FAE8 0x0068FAEA ?SayWeaponModeStarText@@3PADA 0x005905C0
char SayWeaponModeStarText[] = "*";
// RECOVERY_OBJECT: v2 0x0068FAEC 0x0068FAEE ?SayArmorModeStarText@@3PADA 0x005906F0
char SayArmorModeStarText[] = "*";

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
// ORIGINAL: 0x005905C0 ?say_weapon@@YAXPAXHH@Z 0x005905C0-0x005906EC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005905C0-0x005906EC;seed:src/recovered/units/005905c0.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Weapon records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
// symbol    ?say_weapon@@YAXPADHH@Z
void __cdecl say_weapon(char *text, int weapon_id, int with_detail) {
    char *output = text;
    Strings *strings = &StringTable;
    LPSTR name = Weapon[weapon_id].name;
    strcat(output, strings->get(name));
    if (with_detail != 0) {
        strcat(output, BufferJoinSpace);
        strcat(output, "(");
        if (Weapon[weapon_id].offense_rating != 0) {
            LPSTR label = Labels.strings_ptr[0x204 / 4];
            strcat(output, strings->get(label));
            strcat(output, SocialWinColonSpaceText);
            if (Weapon[weapon_id].offense_rating < 0) {
                label = Labels.strings_ptr[0x310 / 4];
                strcat(output, strings->get(label));
            } else {
                char number[80];
                _itoa(Weapon[weapon_id].offense_rating, number, 10);
                strcat(output, number);
            }
            for (int i = 0; i < Weapon[weapon_id].mode; ++i) {
                strcat(output, SayWeaponModeStarText);
            }
        } else {
            LPSTR label = Labels.strings_ptr[0x284 / 4];
            strcat(output, strings->get(label));
        }
        strcat(output, ")");
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005906F0 ?say_armor@@YAXPAXHH@Z 0x005906F0-0x005907F5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005906F0-0x005907F5;seed:src/recovered/units/005906f0.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Armor records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
// symbol    ?say_armor@@YAXPADHH@Z
void __cdecl say_armor(char *text, int armor_id, int with_detail) {
    char *output = text;
    Strings *strings = &StringTable;
    LPSTR name = Armor[armor_id].name;
    strcat(output, strings->get(name));
    if (with_detail != 0) {
        strcat(output, BufferJoinSpace);
        strcat(output, "(");
        LPSTR label = Labels.strings_ptr[0x154 / 4];
        LPSTR detail = strings->get(label);
        strcat(StringTemp, detail);
        strcat(output, SocialWinColonSpaceText);
        if (Armor[armor_id].defense_rating < 0) {
            LPSTR negative_label = Labels.strings_ptr[0x310 / 4];
            strcat(output, strings->get(negative_label));
        } else {
            char number[80];
            _itoa(Armor[armor_id].defense_rating, number, 10);
            strcat(output, number);
        }
        for (int i = 0; i < Armor[armor_id].mode; ++i) {
            strcat(output, SayArmorModeStarText);
        }
        strcat(output, ")");
    }
}

// ORIGINAL: 0x00590800 ?say_chassis@@YAXPAXHH@Z 0x00590800-0x0059090D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590800-0x0059090D;seed:src/recovered/units/00590800.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00590800.cpp"], "reason": "This free formatter uses the established Chassis records and shared Strings and Labels objects with the original name, call ABI, and bounded body. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?say_chassis@@YAXPADHH@Z
// kind      game
void __cdecl say_chassis(char *text, int chassis_id, int with_detail) {
    Strings *strings = &StringTable;
    LPSTR name = Chassis[chassis_id].offsv_1_name;
    strcat(text, strings->get(name));
    if (with_detail != 0) {
        strcat(text, BufferJoinSpace);
        strcat(text, "(");
        LPSTR label = Labels.strings_ptr[0x140 / 4];
        LPSTR detail = strings->get(label);
        strcat(StringTemp, detail);
        strcat(text, SocialWinColonSpaceText);
        char number[80];
        _itoa(Chassis[chassis_id].speed, number, 10);
        strcat(text, number);
        LPSTR triad_label;
        switch (Chassis[chassis_id].triad) {
        case 1:
            strcat(text, BufferJoinSpace);
            triad_label = Labels.strings_ptr[0x28C / 4];
            break;
        case 2:
            strcat(text, BufferJoinSpace);
            triad_label = Labels.strings_ptr[0x288 / 4];
            break;
        default:
            goto close_detail;
        }
        strcat(text, StringTable.get(triad_label));
close_detail:
        strcat(text, ")");
    }
}

// ORIGINAL: 0x00590B30 ?say_techs@@YAXPAXHH@Z 0x00590B30-0x00590B9A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590B30-0x00590B9A;seed:src/recovered/units/00590b30.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00590b30.cpp"], "reason": "This free formatter uses the established Technology records and shared Strings and Labels objects with the original name, call ABI, and bounded body. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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
// RECOVERY_BINDING: v1 {"evidence": ["src/strings.h declares say_facility; strings.cpp owns StringTable and StringTemp and already includes alpha/technology data declarations.", "The pinned label word at byte offset 0x26C is dword index 0x9B; existing label_get(0x9B) performs the same LPSTR-table lookup through Labels and StringTable.", "The one-space/open/close strings are emitted as source-owned literals; each strcat stays in its original order."], "reason": "Recover this formatter through the existing StringTable and typed application records beside the callback table that installs it; StringTable storage and implementation remain owned by strings.cpp. Preserve original effects and historical identity."}
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
// RECOVERY_BINDING: v1 {"evidence": ["src/strings.h declares say_ability; strings.cpp owns StringTable/StringTemp and includes veh.h for Ability.", "The pinned label word at byte offset 0x290 is dword index 0xA4; existing label_get(0xA4) performs the same LPSTR-table lookup.", "The one-space/open/close strings are emitted as source-owned literals in the original strcat order."], "reason": "Recover this formatter through the existing StringTable and typed application records beside the callback table that installs it; StringTable storage and implementation remain owned by strings.cpp. Preserve original effects and historical identity."}
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
// RECOVERY_BINDING: v1 {"evidence": ["src/strings.h declares say_society; strings.cpp owns StringTable/StringTemp; src/faction.h declares SocialCategories.", "The pinned label word at byte offset 0x114 is dword index 0x45; existing label_get(0x45) performs the same LPSTR-table lookup.", "The one-space/open/close strings are emitted as source-owned literals in the original strcat order."], "reason": "Recover this formatter through the existing StringTable and typed application records beside the callback table that installs it; StringTable storage and implementation remain owned by strings.cpp. Preserve original effects and historical identity."}
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

// RECOVERY_LITERAL: v1 0x00682E9C 0x00682E9E ??_C@_01GFI@?$CI?$AA@ 0x00590BA0
// RECOVERY_LITERAL: v1 0x00682E98 0x00682E9A ??_C@_01KMMP@?$CJ?$AA@ 0x00590BA0
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



// Native startup wrapper590500 constructs this table at945A60.
// Eleven entries is the minimum observed initialized region, not an exact
// original-capacity claim; higher category indices remain unverified.
// RECOVERY_OBJECT: v1 0x00945A60 ?DatalinkEntryCallbackTableGlobal@@3VDatalinkEntryCallbackTable@@A
DatalinkEntryCallbackTable DatalinkEntryCallbackTableGlobal;

// ORIGINAL: 0x00590500 sub_590500 0x00590500-0x0059050A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown. Combined historical spans retain both CRT wrapper590500 and constructor590C20; emitted constructor alone is not proof of the generated wrapper matching.", "historical seed:src/unrecovered/00590500.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00590500 loads 0x00945A60 and tail-jumps to 0x00590C20; the wrapper ends at that jump.", "Historical previous-owner observation: coff:src/datalink.cpp: _$E11 loads DatalinkEntryCallbackTableGlobal, tail-jumps to ??0DatalinkEntryCallbackTable, and .CRT$XCU selects _$E11.", "source:src/datalink_entries.cpp owns DatalinkEntryCallbackTableGlobal and its out-of-line constructor; the current compiler-symbol override identifies its generated startup wrapper, not an authored callback."], "reason": "Bind the original startup wrapper to the compiler-generated initializer of DatalinkEntryCallbackTableGlobal in its entry-formatting/selection owner; preserve the original wrapper identity and former combined-span evidence."}
// symbol    _$E12
// ORIGINAL: 0x00590C20 sub_590c20 0x00590C20-0x00590CAF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00590500 tail-jumps to 0x00590C20; the 143-byte constructor body ends with RET at 0x00590CAE.", "Historical previous-owner observation: coff:src/datalink.cpp: separate ??0DatalinkEntryCallbackTable@@QAE@XZ function symbol owns the typed callback-table constructor stores.", "Historical previous-owner observation: source:src/datalink.cpp:DatalinkEntryCallbackTableGlobal and DatalinkEntryCallbackTable::DatalinkEntryCallbackTable are separate source-owned lifetime/storage entities.", "history:git:ea2980d36:src/unrecovered/00590500.cpp preserves the former two-span placeholder identity; its source spans are partitioned here without dropping either range.", "source:src/datalink_entries.cpp contains both the callback-table storage definition and its separate typed constructor; the shared declaration remains in datalink.h."], "reason": "Bind the separately bounded original constructor body to DatalinkEntryCallbackTable construction beside its installed formatters and selection consumers; original TU boundaries and exact table capacity remain unproved."}
// symbol    ??0DatalinkEntryCallbackTable@@QAE@XZ
DatalinkEntryCallbackTable::DatalinkEntryCallbackTable() {
  entries[1].help = 0;
  entries[1].append_text = say_reactor;
  entries[2].help = help_weapon;
  entries[2].append_text = say_weapon;
  entries[3].help = help_armor;
  entries[3].append_text = say_armor;
  entries[4].help = help_chassis;
  entries[4].append_text = say_chassis;
  entries[5].help = help_facility;
  entries[5].append_text = say_facility;
  entries[6].help = help_social;
  entries[6].append_text = say_society;
  entries[7].help = help_abil;
  entries[7].append_text = say_ability;
  entries[8].help = help_tech;
  entries[8].append_text = say_techs;
  entries[9].help = help_veh;
  entries[9].append_text = say_vehicles;
  entries[10].help = help_project;
  entries[10].append_text = say_project;
}


// ORIGINAL: 0x00590CB0 sub_590cb0 0x00590CB0-0x00590CC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00590CB0-0x00590CC2 stores the two incoming pointer words at receiver+0 and +4", "source:src/datalink.h DatalinkEntryCallbacks exposes help and append_text in those two word positions", "pinned:0x00590CD0/0x00590CF0 consumers dispatch through those same two fields"], "reason": "Own the two exact callback-pointer stores on the existing DatalinkEntryCallbacks value type; set_callbacks is an operation-derived current name, not a recovered original API name."}
// symbol    ?set_callbacks@DatalinkEntryCallbacks@@QAEXP6AXH@ZP6AXPADHH@Z@Z
// kind      game
void DatalinkEntryCallbacks::set_callbacks(
    DatalinkHelpCallback help_callback,
    DatalinkTextCallback text_callback) {
  help = help_callback;
  append_text = text_callback;
}

// ORIGINAL: 0x00590CD0 sub_590cd0 0x00590CD0-0x00590CE6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d39c579cc8a1e25f14d68be8a37c583a32266c4c:src/recovered/00590cd0.cpp", "pinned:590CB0..590CC2 initializer;590CD0..590CE6 and590CF0..590D0F complete dispatch/stack cleanup", "source:src/datalink.h:DatalinkEntryCallbacks", "source:recovery/tests/test_datalink_callback_owner.py"], "reason": "Recover the complete guarded callback dispatch through the existing two-pointer row. Adjacent 590CB0 stores these fields at offsets0/4; the pinned member dispatches one or three stack arguments and skips null callbacks. Existing selection-entry direct dispatch remains unchanged. The native EAX survives the callback or is zero on the null path, but no observed direct call or stored pointer establishes a typed return contract; void source roles do not invent one. The old int-return scaffold interpretation remains historical; its BYTE claim and complete original span remain restoration obligations, not a fresh grant. Historical single-epilogue spelling observation: explicitly returning zero added a second return site; no such typed return is introduced."}
// symbol    ?invoke_help@DatalinkEntryCallbacks@@QBEXH@Z
void DatalinkEntryCallbacks::invoke_help(int value) const {
  if (help != 0) {
    help(value);
  }
}

// ORIGINAL: 0x00590CF0 sub_590cf0 0x00590CF0-0x00590D0F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d39c579cc8a1e25f14d68be8a37c583a32266c4c:src/recovered/00590cf0.cpp", "pinned:590CB0..590CC2 initializer;590CD0..590CE6 and590CF0..590D0F complete dispatch/stack cleanup", "source:src/datalink.h:DatalinkEntryCallbacks", "source:recovery/tests/test_datalink_callback_owner.py"], "reason": "Recover the complete guarded callback dispatch through the existing two-pointer row. Adjacent 590CB0 stores these fields at offsets0/4; the pinned member dispatches one or three stack arguments and skips null callbacks. Existing selection-entry direct dispatch remains unchanged. The native EAX survives the callback or is zero on the null path, but no observed direct call or stored pointer establishes a typed return contract; void source roles do not invent one. The old int-return scaffold interpretation remains historical; its BYTE claim and complete original span remain restoration obligations, not a fresh grant."}
// symbol    ?invoke_text@DatalinkEntryCallbacks@@QBEXPADHH@Z
void DatalinkEntryCallbacks::invoke_text(char *output, int value, int extra) const {
  if (append_text != 0) {
    append_text(output, value, extra);
  }
}


// ORIGINAL: 0x00590D10 sub_590d10 0x00590D10-0x00590D3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d10.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append_text@DatalinkSelectionEntry@@QBEXPADH@Z
void DatalinkSelectionEntry::append_text(char *output, int extra) const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.append_text != 0) {
    callbacks.append_text(output, value, extra);
  }
}

// ORIGINAL: 0x00590D40 sub_590d40 0x00590D40-0x00590D5C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d40.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?show_help@DatalinkSelectionEntry@@QBEXXZ
void DatalinkSelectionEntry::show_help() const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.help != 0) {
    callbacks.help(value);
  }
}

// ORIGINAL: 0x00590D60 sub_590d60 0x00590D60-0x00590D6B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/field_accessors.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?clear@DatalinkSelectionQueue@@QAEXXZ
void DatalinkSelectionQueue::clear() {
  count_ = 0;
}

// ORIGINAL: 0x00590D70 sub_590d70 0x00590D70-0x00590DB3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d70.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append@DatalinkSelectionQueue@@QAEXFEE@Z
void DatalinkSelectionQueue::append(short entry_id, unsigned char category,
                                    unsigned char value) {
  if (count_ < 0x80) {
    entries_[count_].entry_id = entry_id;
    entries_[count_].category = category;
    entries_[count_].value = value;
    ++count_;
  }
}

// ORIGINAL: 0x00590DC0 sub_590dc0 0x00590DC0-0x00590E90
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/unrecovered/00590dc0.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?dispatch_matching@DatalinkSelectionQueue@@QAEXH@Z
// TRIED: `if (count > 0)` reads as `cmp;jle` here, not the original's `test;jle`; tried elsewhere didn't move it. Indexed struct field access (`g_945a60[i].fn`) collapses lea+mov into one mov; original keeps them split (address-of first).
void DatalinkSelectionQueue::dispatch_matching(int entry_id) {
  DatalinkMain.init();

  int matched_count = 0;
  int first_match_index;
  for (int index = 0; index < count_; ++index) {
    DatalinkSelectionEntry &entry = entries_[index];
    if (entry.entry_id == entry_id) {
      if (matched_count == 0) {
        first_match_index = index;
      }
      ++matched_count;
      StringTemp[0] = 0;

      if (entry.category == 5 && entry.value >= 0x46) {
        entry.category = 10;
        entry.value = static_cast<unsigned char>(entry.value + 0xBA);
      }
      entry.append_text(StringTemp, 1);
    }
  }

  if (matched_count != 0) {
    datalink_entry_selection(first_match_index);
    DatalinkMain.show(0);
    Win &modal_window = DatalinkMain;
    modal_window.set_modal(0, pop_wait, 0);
  }
}

