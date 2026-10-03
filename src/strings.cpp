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

// RECOVERY_OBJECT: v1 0x009B86A0 ?StringTemp@@3PADA
char StringTemp[1032];   // 0x009B86A0

// Clear the first character without disturbing the reusable buffer storage.
// ORIGINAL: 0x00500120 ?clear@@YAXXZ 0x00500120-0x00500128 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500120-0x00500128 writes 0x009B86A0", "git:0cb1eb5c:src/recovered/00500120.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original zeroes the first byte of the shared scratch buffer, whose real storage is StringTemp in this translation unit."}
void __cdecl clear() {
    StringTemp[0] = 0;
}

// Append a caller-supplied string to the shared scratch buffer.
// ORIGINAL: 0x00500130 ?says@@YAXPBD@Z 0x00500130-0x00500146 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500130-0x00500146 calls strcat with 0x009B86A0", "git:0cb1eb5c:src/recovered/units/00500130.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original appends a caller string into the source-owned shared StringTemp buffer."}
void __cdecl says(const char *text) {
    strcat(StringTemp, text);
}

// The one game-wide string table, 0x009B90D8 in the image - REAL STORAGE,
// not the `static Strings *const` binding that named terranx.exe's address
// (see strings.h). The image's ??__E runs the real constructor against it;
// here the CRT walk does the same, and jackal_init_real's
// `StringTable.init(0x8000)` populates it.
// RECOVERY_OBJECT: v1 0x009B90D8 ?StringTable@@3VStrings@@A
Strings StringTable;  // 0x009B90D8

/*
Purpose: Initialize the class instance with a new string table of the specified size.
// ORIGINAL: 0x006168F0 ?init@Strings@@QAEHH@Z 0x006168F0-0x00616950 BYTE_EXACT
// LEVER: BYTE_EXACT 31/31, from 6/31. call_diff had it at 3 calls against the image's 5: BOTH `shutdown()` and `put("-Nil-")` are written out here by the image, and both were real calls. Hand-inlined - not moved to the header - because each has its own out-of-line body in the image (0x00616950 has 4 callers, 0x00616970 has 5) and an `inline` in strings.h would take those call sites away from them. Measured one at a time: shutdown alone 17/31, put alone 5/31, both 31/31.
// LEVER: strcpy, not strcpy_s, in the inlined put(). The image's callee is 0x00645460; the safe-CRT form scores 10/31 and compiles 35 instructions against the image's 31.
// symbol    ?init@Strings@@QAEHI@Z
// size      96 bytes
// prototype int (__thiscall ?init@Strings@@QAEHH@Z)(Strings* this, int)
// callers   4   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580 0x005D4620 0x005D4680 0x006453E0 0x00645460
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL Strings::init(size_t mem_size) {
    // BOTH HELPERS ARE HAND-INLINED. The image writes shutdown() and put()
    // out here rather than calling either - see the LEVER above - so this is
    // shutdown()'s `~Heap(); is_populated_ = false;` and put()'s
    // strlen/Heap::get/strcpy, spelled out. Both keep their own out-of-line
    // BYTE_EXACT claims at 0x00616950 and 0x00616970.
    if (is_populated_) {
        shutdown();
        is_populated_ = false;
    }
    if (Heap::init(mem_size)) {
        return true; // allocation failed
    }
    size_t len = strlen("-Nil-") + 1;
    LPSTR table_addr = LPSTR(Heap::get(len));
    strcpy(table_addr, "-Nil-");
    is_populated_ = true;
    return false; // successful
}

/*
Purpose: Shutdown the class instance.
// ORIGINAL: 0x00616950 ?shutdown@Strings@@QAEHXZ 0x00616950-0x00616963 BYTE_EXACT
// size      19 bytes
// prototype int (__thiscall ?shutdown@Strings@@QAEHXZ)(Strings* this)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580
Return Value: Always zero (the mangled name decorates an `int` return; the
    image's tail is `xor eax, eax; ret`, not a bare `ret`)
Status: Complete
*/
// Non-inline forwarder to heap.h's `MEASURED inline` ~Heap. The image CALLS
// the out-of-line ??1Heap (0x005D4580) here rather than open-coding it, and
// plain /O2 expands the destructor into this body - 21 instructions against
// the image's 7. Same idiom as faction.cpp's ascending_call: the E8 target is
// a relocation on both sides, so this symbol's own name costs nothing.
static void __fastcall strings_heap_close(Heap *self) {
    self->~Heap();
}

int Strings::shutdown() {
    // WRONG CALLEE fix: the image's one call here targets 0x005D4580,
    // ~Heap, not Heap::shutdown() (0x005D45B0) - identical logic, different
    // symbol. The call goes through the forwarder above because the direct
    // pseudo-destructor spelling only stays a call while the expansion is
    // off; see the TRIED note on the marker above.
    strings_heap_close(this);
    is_populated_ = false;
    return 0;
}

/*
Purpose: Put the input string into the string table.
// ORIGINAL: 0x00616970 ?put@Strings@@QAEHPAD@Z 0x00616970-0x0061699D BYTE_EXACT
// LEVER (2026-09-03): strcpy, not strcpy_s, in the OUT-OF-LINE body too -
//        same callee the hand-inlined copy inside init() reaches. strcpy_s
//        is an intrinsic under /Ob2 and expanded inline here, which is why
//        this body needed the intrinsics-off set; the two-argument strcpy
//        reproduces under the plain pair and names the image's real callee
//        (0x00645460) instead of a different function whose call target the
//        comparison was discounting.
// symbol    ?put@Strings@@QAEPADPBD@Z
// size      45 bytes
// prototype int (__thiscall ?put@Strings@@QAEHPAD@Z)(Strings* this, int8*)
// callers   5   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4680 0x006453E0 0x00645460
Return Value: Address of the stored string
Status: Complete
*/
LPSTR Strings::put(LPCSTR input) {
    size_t len = strlen(input) + 1;
    LPSTR table_addr = LPSTR(Heap::get(len));
    // strcpy, NOT strcpy_s: the image's callee at 0x00645460 is the two-
    // argument CRT copy, the same one init()'s hand-inlined put() reaches.
    // strcpy_s expanded inline under /Ob2, which is why this body only
    // reproduced under the intrinsics-off flag set.
    strcpy(table_addr, input);
    return table_addr;
}

/*
Purpose: Check if the string address is within the table.
// ORIGINAL: 0x006169A0 ?get@Strings@@QAEHH@Z 0x006169A0-0x006169BB BYTE_EXACT
// symbol    ?get@Strings@@QAEPADH@Z
// size      27 bytes
// prototype int (__thiscall ?get@Strings@@QAEHH@Z)(Strings* this, int)
// callers   290   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: Address of the stored string
Status: Complete
*/
LPSTR Strings::get(int address) {
    if ((LPVOID)address > get_base()) { // checking if ptr is after base
        // checking ptr is not after end of table
        return ((LPSTR)address <= LPSTR(size_t(get_base()) + get_base_size())) 
            ? (LPSTR)address : NULL;
    }
    return LPSTR(get_base()); // return base
}

// The five part-name formatters share the same label table, string table,
// scratch buffer, and rules records as the neighboring say_* functions.
// Their optional suffixes deliberately append a label to StringTemp before
// continuing the destination; that side effect is present in the original.
// ORIGINAL: 0x00590510 ?say_reactor@@YAXPAXHH@Z 0x00590510-0x005905B5
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590510-0x005905B5;seed:src/unrecovered/00590510.cpp;source:src/veh.h:Reactor;source:src/strings.h:StringTable"], "reason": "This free formatter reads the real Reactor record and shared Strings and Labels objects, matching the original name, call ABI, and bounded body."}
// TRIED: MISMATCH #4 push/mov, local buffer layout differs
// TRIED: _itoa needs a real signature -> declared as itoa_impl (own name), since the scaffold's nullary _itoa can't be redeclared (C2733)
void __cdecl say_reactor(void *output, int reactor_id, int with_detail) {
    char *text = static_cast<char *>(output);
    strcat(text, StringTable.get(reinterpret_cast<int>(Reactor[reactor_id].name)));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            *reinterpret_cast<int *>(Labels.strings_ptr + 0x13C)));
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
void __cdecl say_weapon(void *output, int weapon_id, int with_detail) {
    char *text = static_cast<char *>(output);
    const RulesWeapon &weapon = Weapon[weapon_id];
    strcat(text, StringTable.get(reinterpret_cast<int>(weapon.name)));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        if (weapon.offense_rating == 0) {
            strcat(text, StringTable.get(
                *reinterpret_cast<int *>(Labels.strings_ptr + 0x284)));
        } else {
            strcat(text, StringTable.get(
                *reinterpret_cast<int *>(Labels.strings_ptr + 0x204)));
            strcat(text, ": ");
            if (weapon.offense_rating < 0) {
                strcat(text, StringTable.get(
                    *reinterpret_cast<int *>(Labels.strings_ptr + 0x310)));
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
void __cdecl say_armor(void *output, int armor_id, int with_detail) {
    char *text = static_cast<char *>(output);
    const RulesArmor &armor = Armor[armor_id];
    strcat(text, StringTable.get(reinterpret_cast<int>(armor.name)));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            *reinterpret_cast<int *>(Labels.strings_ptr + 0x154)));
        strcat(text, ": ");
        if (armor.defense_rating < 0) {
            strcat(text, StringTable.get(
                *reinterpret_cast<int *>(Labels.strings_ptr + 0x310)));
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

// ORIGINAL: 0x00590800 ?say_chassis@@YAXPAXHH@Z 0x00590800-0x0059090D
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590800-0x0059090D;seed:src/recovered/units/00590800.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Chassis records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
void __cdecl say_chassis(void *output, int chassis_id, int with_detail) {
    char *text = static_cast<char *>(output);
    const RulesChassis &chassis = Chassis[chassis_id];
    strcat(text, StringTable.get(reinterpret_cast<int>(chassis.offsv_1_name)));
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            *reinterpret_cast<int *>(Labels.strings_ptr + 0x140)));
        strcat(text, ": ");
        char number[80];
        _itoa(chassis.speed, number, 10);
        strcat(text, number);
        if (chassis.triad == 1 || chassis.triad == 2) {
            strcat(text, " ");
            const int offset = chassis.triad == 1 ? 0x28C : 0x288;
            strcat(text, StringTable.get(
                *reinterpret_cast<int *>(Labels.strings_ptr + offset)));
        }
        strcat(text, ")");
    }
}

// ORIGINAL: 0x00590B30 ?say_techs@@YAXPAXHH@Z 0x00590B30-0x00590B9A
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00590B30-0x00590B9A;seed:src/recovered/units/00590b30.cpp;source:src/strings.h:StringTable;source:src/alpha.h:Labels"], "reason": "This free formatter uses the established Technology records and shared Strings and Labels objects with the original name, call ABI, and bounded body."}
void __cdecl say_techs(void *output, int tech_id, int with_detail) {
    char *text = static_cast<char *>(output);
    say_tech(text, tech_id, with_detail);
    if (with_detail != 0) {
        strcat(text, " ");
        strcat(text, "(");
        strcat(StringTemp, StringTable.get(
            *reinterpret_cast<int *>(Labels.strings_ptr + 0x274)));
        strcat(text, ")");
    }
}

// global


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x006168D0 ??0Strings@@QAE@XZ 0x006168D0-0x006168E3 BYTE_EXACT
// body      src/strings.h
// size      19 bytes
// prototype void (__thiscall ??0Strings@@QAE@XZ)(Strings* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4560
// notes     Staged hybrid export redirect calls the source-owned constructor
*/

/*
// ORIGINAL: 0x006169C0 ??1Strings@@QAE@XZ 0x006169C0-0x006169C5 BYTE_EXACT
// LEVER (2026-09-03): taking `~Heap` out of heap.h and into heap.cpp - the
//        same edit the constructor already had - is what lands this. In-class
//        it was EXPANDED at this implicit base teardown by /Ob2: nineteen
//        instructions where the image is the five-byte `jmp` the comment on
//        strings.h's `~Strings` describes. Out of line, the implicit call is
//        a real call and VC6 tail-jumps it, exactly the image's shape. The
//        earlier TRIED note here ("no source spelling can ask a void
//        function to tail-call") was right about the mechanism and wrong
//        about the reach: the spelling that changes is the callee's, not
//        this body's.
// body      src/strings.h
// size      5 bytes
// prototype void (__thiscall ??1Strings@@QAE@XZ)(Strings* this)
// callers   0   call targets   0
// kind      thunk
// flags     thunk;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
