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
// ORIGINAL: 0x006168A0 _$E21 0x006168A0-0x006168B6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6826b0 selects initializer0x6168a0..0x6168b6, registering exit0x6168c0..0x6168ca. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x9b90d8 bound to?StringTable@@3VStrings@@A.", "Original startup instruction evidence: [[\"0x6168a0\", \"mov\", \"ecx, 0x9b90d8\"], [\"0x6168a5\", \"call\", \"0x6168d0\"], [\"0x6168aa\", \"push\", \"0x6168c0\"], [\"0x6168af\", \"call\", \"0x645398\"], [\"0x6168b4\", \"pop\", \"ecx\"], [\"0x6168b5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x6168c0\", \"mov\", \"ecx, 0x9b90d8\"], [\"0x6168c5\", \"jmp\", \"0x6169c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/strings.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?StringTable@@3VStrings@@A\", \"??0Strings@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?StringTable@@3VStrings@@A\", \"??1Strings@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f890; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?StringTable@@3VStrings@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E8
// ORIGINAL: 0x006168C0 _$E19 0x006168C0-0x006168CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6826b0 selects initializer0x6168a0..0x6168b6, registering exit0x6168c0..0x6168ca. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x9b90d8 bound to?StringTable@@3VStrings@@A.", "Original startup instruction evidence: [[\"0x6168a0\", \"mov\", \"ecx, 0x9b90d8\"], [\"0x6168a5\", \"call\", \"0x6168d0\"], [\"0x6168aa\", \"push\", \"0x6168c0\"], [\"0x6168af\", \"call\", \"0x645398\"], [\"0x6168b4\", \"pop\", \"ecx\"], [\"0x6168b5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x6168c0\", \"mov\", \"ecx, 0x9b90d8\"], [\"0x6168c5\", \"jmp\", \"0x6169c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/strings.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?StringTable@@3VStrings@@A\", \"??0Strings@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?StringTable@@3VStrings@@A\", \"??1Strings@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f890; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?StringTable@@3VStrings@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E6
#include "stdafx.h"
#include "strings.h"
#include "buffer.h"
#include "alpha.h"
#include "technology.h"
#include "veh.h"
#include "base.h"
#include "faction.h"

// RECOVERY_OBJECT: v1 0x009B86A0 ?StringTemp@@3PADA
char StringTemp[1032];   // 0x009B86A0

// Clear the first character without disturbing the reusable buffer storage.
// ORIGINAL: 0x00500120 ?clear@@YAXXZ 0x00500120-0x00500128 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500120-0x00500128 writes 0x009B86A0", "git:0cb1eb5c:src/recovered/00500120.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original zeroes the first byte of the shared scratch buffer, whose real storage is StringTemp in this translation unit."}
void __cdecl clear() {
    StringTemp[0] = 0;
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
    // shutdown()'s live Heap reset and flag clear, and put()'s
    // strlen/Heap::get/strcpy, spelled out. Both keep their own out-of-line
    // BYTE_EXACT claims at 0x00616950 and 0x00616970.
    if (is_populated_) {
        shutdown();
        is_populated_ = false;
    }
    if (Heap::init(mem_size)) {
        return true; // allocation failed
    }
    // Both pinned strlen/strcpy arguments select 6971FC. The six literal
    // bytes precede alignment padding and the independently used labels keys.
    // RECOVERY_LITERAL: v1 0x006971FC 0x00697202 ??_C@_05BHIP@?9Nil?9?$AA@ 0x006168F0
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
int Strings::shutdown() {
    // 616950 calls the live reset at 5D4580. The base remains alive for
    // later init/put and will be destroyed normally when Strings dies.
    Heap::reset();
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
// symbol    ?get@Strings@@QAEPADPAD@Z
// size      27 bytes
// prototype int (__thiscall ?get@Strings@@QAEHH@Z)(Strings* this, int)
// callers   290   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: Address of the stored string
Status: Complete
*/
LPSTR Strings::get(LPSTR address) {
    const uint32_t address32 = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(address));
    const uint32_t base32 = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(get_base()));
    if (address32 > base32) { // checking if ptr is after base
        // The original x86 add wraps at 32 bits, and the endpoint is inclusive.
        const uint32_t end32 = base32 + static_cast<uint32_t>(get_base_size());
        return address32 <= end32 ? address : NULL;
    }
    return static_cast<LPSTR>(get_base()); // return base
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

// ORIGINAL: 0x005A58B0 ?space@@YAXXZ 0x005A58B0-0x005A58C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/recovered/units/005a58b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "strings.cpp / strings.h: append one ordinary space to the existing StringTemp via strcat. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl space() {
    strcat(StringTemp, BufferJoinSpace);
}
