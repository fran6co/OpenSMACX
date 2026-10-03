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
#include "global_arith.h"

// PrefWin geometry constants observed in the pinned image. Roles are named
// only where the PrefWin/GraphicWin initialization call arguments establish
// them; the remaining four retain address-bearing provisional names.
// RECOVERY_OBJECT: v2 0x00686B6C 0x00686B70 ?PrefWinGeometryConstant_686B6C@@3HA 0x0048D550
int32_t PrefWinGeometryConstant_686B6C = 4;
// RECOVERY_OBJECT: v2 0x00686B70 0x00686B74 ?PrefWinGeometryConstant_686B70@@3HA 0x0048D560
int32_t PrefWinGeometryConstant_686B70 = 4;
// RECOVERY_OBJECT: v2 0x00686B74 0x00686B78 ?PrefWinGeometryConstant_686B74@@3HA 0x0048D5B0
int32_t PrefWinGeometryConstant_686B74 = 3;
// RECOVERY_OBJECT: v2 0x00686B78 0x00686B7C ?PrefWinGeometryConstant_686B78@@3HA 0x0048D5D0
int32_t PrefWinGeometryConstant_686B78 = 3;
// RECOVERY_OBJECT: v2 0x00686B7C 0x00686B80 ?PrefWinDisplayWidth@@3HA 0x0048D570
int32_t PrefWinDisplayWidth = 700;
// RECOVERY_OBJECT: v2 0x00686B80 0x00686B84 ?PrefWinDisplayHeight@@3HA 0x0048D590
int32_t PrefWinDisplayHeight = 390;
// RECOVERY_OBJECT: v2 0x00686B84 0x00686B88 ?PrefWinButtonWidth@@3HA 0x0048D610
int32_t PrefWinButtonWidth = 220;
// RECOVERY_OBJECT: v2 0x00686B88 0x00686B8C ?PrefWinTitleButtonHeight@@3HA 0x0048D840
int32_t PrefWinTitleButtonHeight = 20;
// RECOVERY_OBJECT: v2 0x00686B8C 0x00686B90 ?PrefWinToggleWidth@@3HA 0x0048D820
int32_t PrefWinToggleWidth = 120;
// RECOVERY_OBJECT: v2 0x00686B90 0x00686B94 ?PrefWinToggleHeight@@3HA 0x0048D840
int32_t PrefWinToggleHeight = 20;

// These values occupy zero-initialized storage in the image and are filled by
// the ordered geometry initializer family below.
// RECOVERY_OBJECT: v2 0x008577D8 0x008577DC ?PrefWinGeometryValue_8577D8@@3HA 0x0048D750
int32_t PrefWinGeometryValue_8577D8;
// RECOVERY_OBJECT: v2 0x008577DC 0x008577E0 ?PrefWinGeometryValue_8577DC@@3HA 0x0048D760
int32_t PrefWinGeometryValue_8577DC;
// RECOVERY_OBJECT: v2 0x008577E0 0x008577E4 ?PrefWinGeometryValue_8577E0@@3HA 0x0048D790
int32_t PrefWinGeometryValue_8577E0;
// RECOVERY_OBJECT: v2 0x008577E4 0x008577E8 ?PrefWinGeometryValue_8577E4@@3HA 0x0048D770
int32_t PrefWinGeometryValue_8577E4;
// RECOVERY_OBJECT: v2 0x008577E8 0x008577EC ?PrefWinGeometryValue_8577E8@@3HA 0x0048D560
int32_t PrefWinGeometryValue_8577E8;
// RECOVERY_OBJECT: v2 0x008577EC 0x008577F0 ?PrefWinGeometryValue_8577EC@@3HA 0x0048D550
int32_t PrefWinGeometryValue_8577EC;
// RECOVERY_OBJECT: v2 0x00857884 0x00857888 ?PrefWinGeometryValue_857884@@3HA 0x0048D5D0
int32_t PrefWinGeometryValue_857884;
// RECOVERY_OBJECT: v2 0x00857888 0x0085788C ?PrefWinGeometryValue_857888@@3HA 0x0048D5B0
int32_t PrefWinGeometryValue_857888;
// RECOVERY_OBJECT: v2 0x0085788C 0x00857890 ?PrefWinGeometryValue_85788C@@3HA 0x0048D6F0
int32_t PrefWinGeometryValue_85788C;
// RECOVERY_OBJECT: v2 0x00857890 0x00857894 ?PrefWinGeometryValue_857890@@3HA 0x0048D710
int32_t PrefWinGeometryValue_857890;
// RECOVERY_OBJECT: v2 0x00857894 0x00857898 ?PrefWinGeometryValue_857894@@3HA 0x0048D730
int32_t PrefWinGeometryValue_857894;
// RECOVERY_OBJECT: v2 0x00857898 0x0085789C ?PrefWinGeometryValue_857898@@3HA 0x0048D630
int32_t PrefWinGeometryValue_857898;
// RECOVERY_OBJECT: v2 0x0085789C 0x008578A0 ?PrefWinGeometryValue_85789C@@3HA 0x0048D610
int32_t PrefWinGeometryValue_85789C;
// RECOVERY_OBJECT: v2 0x008578A0 0x008578A4 ?PrefWinGeometryValue_8578A0@@3HA 0x0048D6D0
int32_t PrefWinGeometryValue_8578A0;
// RECOVERY_OBJECT: v2 0x008578A4 0x008578A8 ?PrefWinGeometryValue_8578A4@@3HA 0x0048D670
int32_t PrefWinGeometryValue_8578A4;
// RECOVERY_OBJECT: v2 0x008578A8 0x008578AC ?PrefWinGeometryValue_8578A8@@3HA 0x0048D650
int32_t PrefWinGeometryValue_8578A8;
// RECOVERY_OBJECT: v2 0x008578AC 0x008578B0 ?PrefWinGeometryValue_8578AC@@3HA 0x0048D570
int32_t PrefWinGeometryValue_8578AC;
// RECOVERY_OBJECT: v2 0x008578B0 0x008578B4 ?PrefWinGeometryValue_8578B0@@3HA 0x0048D590
int32_t PrefWinGeometryValue_8578B0;
// RECOVERY_OBJECT: v2 0x008578B4 0x008578B8 ?PrefWinGeometryValue_8578B4@@3HA 0x0048D7B0
int32_t PrefWinGeometryValue_8578B4;
// RECOVERY_OBJECT: v2 0x008578B8 0x008578BC ?PrefWinGeometryValue_8578B8@@3HA 0x0048D820
int32_t PrefWinGeometryValue_8578B8;
// RECOVERY_OBJECT: v2 0x008578BC 0x008578C0 ?PrefWinGeometryValue_8578BC@@3HA 0x0048D7D0
int32_t PrefWinGeometryValue_8578BC;
// RECOVERY_OBJECT: v2 0x008578C0 0x008578C4 ?PrefWinGeometryValue_8578C0@@3HA 0x0048D5F0
int32_t PrefWinGeometryValue_8578C0;
// RECOVERY_OBJECT: v2 0x008578C4 0x008578C8 ?PrefWinGeometryValue_8578C4@@3HA 0x0048D6B0
int32_t PrefWinGeometryValue_8578C4;
// RECOVERY_OBJECT: v2 0x008578C8 0x008578CC ?PrefWinGeometryValue_8578C8@@3HA 0x0048D690
int32_t PrefWinGeometryValue_8578C8;
// RECOVERY_OBJECT: v2 0x008578CC 0x008578D0 ?PrefWinGeometryValue_8578CC@@3HA 0x0048D7F0
int32_t PrefWinGeometryValue_8578CC;
// RECOVERY_OBJECT: v2 0x008578D0 0x008578D4 ?PrefWinGeometryValue_8578D0@@3HA 0x0048D810
int32_t PrefWinGeometryValue_8578D0;

/*
 * Absolute-operand arithmetic leaves.
 * Was generated by tools/generate_global_arith.py, WHICH NO LONGER EXISTS:
 * commit 271114cf retired the bulk generators as "9,262 lines that nobody
 * could run". EDIT THIS FILE DIRECTLY. The old instruction - edit the
 * generator, not the file - outlived the generator by months and told every
 * reader not to touch this file in the only way it can now be touched.
 *
 *
 * The geometry initializer family contains the 24 historical arithmetic
 * helpers below plus the two formerly seed-only initializers. Their former
 * fixed-address operands now resolve to ordinary named objects; the source
 * bodies and claims remain available for coordinator rebinding/review.
 *
 * SIXTEEN of the 26 bodies RETURN THE FIRST GLOBAL THEY LOAD. That is not a
 * claim about the image's signature - no caller exists to observe a return
 * value - it is the one source shape VC6 lowers to the shipped bytes. Spelled
 * `void`, the first global's register is free to clobber once its last use is
 * past, so the compiler computes into it and the image's `add ecx, eax` /
 * `lea ecx,[eax+eax]` comes out as `add eax, ecx` / `shl eax, 1`. The image
 * keeps that register live to the ret, and the only way to make VC6 see a use
 * after the last store is to add one that emits nothing: returning a value
 * already sitting in eax. Every statement spelling, `register` storage and
 * all twenty flag sets in the ladder were measured against this and produce
 * the identical clobbering form - the LEVER notes under each marker carry the
 * detail. IDA 9.4's guess_type for all 26 is `int()`, which is consistent
 * with it and cannot discriminate.
 *
 * Geometry startup is driven by the compiler-generated initializer for
 * PrefWinGeometryBootstrap at the end of this translation unit. The separate
 * 0x0044C870 toggle still uses its historical 0x007591C4 pointer alias.
 */


/*
Purpose: sub_44c870 - arithmetic over 1 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0044C870 sub_44c870 0x0044C870-0x0044C878
// TRIED 2026-09-03: five spellings for the single statement, scored with
// tools/try_spellings.py - `*p ^= 1`, `*p = *p ^ 1` (as committed), a
// `uint32_t *const p` local, `volatile int32_t *const p` read-modify-write
// and `volatile ... ^= 1`, plus `|= 1`. Every one still compiles the
// load/xor/store triple (3 or 4 instructions against the image's 2); none
// reaches the memory-form `xor dword ptr [0x7591c4], 1`. VC6 will not emit
// the in-memory read-modify-write through a const-pointer lvalue, only
// through a named global object, and these globals are bound rather than
// placed so no named object can live at 0x7591c4.
// TRIED 2026-09-03, second pass: the reference lvalue is the one new shape -
// `int32_t &r = *GlobalSlot007591C4; r = r ^ 1;` compiles `mov eax,<ptr>;
// xor [eax],1` (9 bytes against the image's 8), still not the memory form.
// Compiled the same statement against a REAL object in the same run as the
// control: `extern int x; x ^= 1` and `x = x ^ 1` BOTH compile to
// `xor dword ptr [<object>],1` - the image's exact 7 bytes, displacement
// relocated. So the memory form IS reachable from C++, and the one thing this
// body lacks is an object at 0x7591c4. That is an EVIDENCE lever, not a
// spelling lever: leaf_recoveries.cpp documents 0x7591C4 as evidence-free
// ("a named global here would be invention"), so it stays bound.
// symbol    ?global_arith_0044c870_redirect@@YIXPAX0@Z
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0044c870_redirect(void *, void *) {
    *GlobalSlot007591C4 = *GlobalSlot007591C4 ^ 1;
}

/*
Purpose: sub_48d550 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D550 sub_48d550 0x0048D550-0x0048D55E BYTE_EXACT
// symbol    ?global_arith_0048d550_redirect@@YIXPAX0@Z
// size      14 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d550_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B6C;
    eax = eax + 23;
    PrefWinGeometryValue_8577EC = eax;
}

/*
Purpose: sub_48d560 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D560 sub_48d560 0x0048D560-0x0048D56B BYTE_EXACT
// symbol    ?global_arith_0048d560_redirect@@YIXPAX0@Z
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d560_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B70;
    PrefWinGeometryValue_8577E8 = eax;
}

/*
Purpose: sub_48d570 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D570 sub_48d570 0x0048D570-0x0048D587 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   Committed as `void`, VC6 folds `2*a` into a's own register (`shl eax, 1`)
//   and gives the second load ecx; the image has `lea ecx, [eax + eax]` with
//   the second load in edx, which VC6 emits ONLY when a is still live - four
//   spellings of the statement, `register` storage and every /O flag set
//   produce the identical shl form. Returning the first global is the one
//   shape that keeps it live, and it costs no instruction: the value is
//   already in eax at the ret. No caller in the image, so nothing observes
//   the choice either way.
//   PROOF OF THE MECHANISM: keeping the first global's value alive with a
//   conditional second store compiles the image's exact `mov edx / lea
//   ecx,[eax+eax] / sub edx,ecx` plus that store - liveness is the only thing
//   the `void` spelling was missing.
// symbol    ?global_arith_0048d570_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d570_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B6C;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinDisplayWidth;
    edx = edx - ecx;
    PrefWinGeometryValue_8578AC = edx;
    return eax;
}

/*
Purpose: sub_48d590 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D590 sub_48d590 0x0048D590-0x0048D5A7 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d590_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d590_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B70;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinDisplayHeight;
    edx = edx - ecx;
    PrefWinGeometryValue_8578B0 = edx;
    return eax;
}

/*
Purpose: sub_48d5b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5B0 sub_48d5b0 0x0048D5B0-0x0048D5C4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d5b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d5b0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577EC;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    PrefWinGeometryValue_857888 = ecx;
    return eax;
}

/*
Purpose: sub_48d5d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5D0 sub_48d5d0 0x0048D5D0-0x0048D5E4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d5d0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d5d0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577E8;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    ecx = ecx + eax;
    PrefWinGeometryValue_857884 = ecx;
    return eax;
}

/*
Purpose: sub_48d5f0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5F0 sub_48d5f0 0x0048D5F0-0x0048D607 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d5f0_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d5f0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B78;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinGeometryValue_8578B0;
    edx = edx - ecx;
    PrefWinGeometryValue_8578C0 = edx;
    return eax;
}

/*
Purpose: sub_48d610 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D610 sub_48d610 0x0048D610-0x0048D62B SEMANTIC
// TRIED 2026-09-03: six shapes for the four-global chain, including the
//   register-liveness lever that landed the other sixteen bodies in this
//   file. Closest is declaring the accumulator before the first global,
//   which puts all three loads in the image's order and agrees 4 of 6 - but
//   the first add still lands in eax and the reload of the third global in
//   ecx, the exact mirror of the image's `add ecx,eax; mov eax,[<3rd>]`.
//   The first add only moves into ecx when the first global is still live,
//   and a live first global cannot share its register with the reload. The
//   image has both at once, so this pair is as far as source form goes.
// symbol    ?global_arith_0048d610_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d610_redirect(void *, void *) {
    int32_t eax = PrefWinButtonWidth;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    eax = PrefWinGeometryValue_857888;
    ecx = ecx + eax;
    PrefWinGeometryValue_85789C = ecx;
}

/*
Purpose: sub_48d630 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D630 sub_48d630 0x0048D630-0x0048D644 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d630_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d630_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577E8;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    ecx = ecx + eax;
    PrefWinGeometryValue_857898 = ecx;
    return eax;
}

// Original ordered initializer at 0x0048D650, between the 0x48D630 and
// 0x48D670 entries in the pinned CRT initializer array.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048D650 sub_48d650 0x0048D650-0x0048D66E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT table entry selects 0048D650, whose typed arithmetic and output store were independently decoded from the pinned executable.", "recovery/tasks/prefwin-geometry-initializers.json", "src/global_arith.h"], "reason": "Recover the missing scalar initializer in the ordered PrefWin geometry startup family through genuine source-owned integer storage; retain the compatible historical exact seed and complete original span."}
// symbol    _sub_48d650
extern "C" void __cdecl sub_48d650() {
    PrefWinGeometryValue_8578A8 =
        PrefWinGeometryValue_8578AC - PrefWinGeometryConstant_686B74 * 3 -
        PrefWinButtonWidth;
}

/*
Purpose: sub_48d670 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D670 sub_48d670 0x0048D670-0x0048D687 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d670_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d670_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B78;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinGeometryValue_8578B0;
    edx = edx - ecx;
    PrefWinGeometryValue_8578A4 = edx;
    return eax;
}

/*
Purpose: sub_48d690 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D690 sub_48d690 0x0048D690-0x0048D6A4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d690_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d690_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_85789C;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    PrefWinGeometryValue_8578C8 = ecx;
    return eax;
}

/*
Purpose: sub_48d6b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6B0 sub_48d6b0 0x0048D6B0-0x0048D6C4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d6b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d6b0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_857898;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    ecx = ecx + eax;
    PrefWinGeometryValue_8578C4 = ecx;
    return eax;
}

/*
Purpose: sub_48d6d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6D0 sub_48d6d0 0x0048D6D0-0x0048D6E7 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d6d0_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d6d0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B74;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinGeometryValue_8578A8;
    edx = edx - ecx;
    PrefWinGeometryValue_8578A0 = edx;
    return eax;
}

/*
Purpose: sub_48d6f0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6F0 sub_48d6f0 0x0048D6F0-0x0048D704 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d6f0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d6f0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_857888;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    PrefWinGeometryValue_85788C = ecx;
    return eax;
}

/*
Purpose: sub_48d710 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D710 sub_48d710 0x0048D710-0x0048D724 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d710_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d710_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_857884;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    ecx = ecx + eax;
    PrefWinGeometryValue_857890 = ecx;
    return eax;
}

/*
Purpose: sub_48d730 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D730 sub_48d730 0x0048D730-0x0048D747 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d730_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d730_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B74;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinButtonWidth;
    edx = edx - ecx;
    PrefWinGeometryValue_857894 = edx;
    return eax;
}

/*
Purpose: sub_48d750 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D750 sub_48d750 0x0048D750-0x0048D75B BYTE_EXACT
// symbol    ?global_arith_0048d750_redirect@@YIXPAX0@Z
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d750_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8578C8;
    PrefWinGeometryValue_8577D8 = eax;
}

/*
Purpose: sub_48d760 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D760 sub_48d760 0x0048D760-0x0048D770 BYTE_EXACT
// symbol    ?global_arith_0048d760_redirect@@YIXPAX0@Z
// size      16 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d760_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577E8;
    eax = eax + 283;
    PrefWinGeometryValue_8577DC = eax;
}

/*
Purpose: sub_48d770 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D770 sub_48d770 0x0048D770-0x0048D787 BYTE_EXACT
// LEVER: the first global's register must stay live across the doubling.
//   VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second
//   load ecx; the image has `lea ecx, [eax + eax]` with the second load in
//   edx, which VC6 emits ONLY when the first global is still live - four
//   statement spellings, `register` storage and every /O flag produce the
//   identical shl form. Returning the first global is the one shape that
//   keeps it live, and it costs no instruction: the value is already in eax
//   at the ret. No caller in the image, so nothing observes the choice.
// symbol    ?global_arith_0048d770_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d770_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryConstant_686B74;
    int32_t ecx = eax + eax;
    int32_t edx = PrefWinGeometryValue_8578A8;
    edx = edx - ecx;
    PrefWinGeometryValue_8577E4 = edx;
    return eax;
}

/*
Purpose: sub_48d790 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D790 sub_48d790 0x0048D790-0x0048D7AB BYTE_EXACT
// symbol    ?global_arith_0048d790_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d790_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8578B0;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    eax = eax - ecx;
    ecx = PrefWinGeometryValue_8577DC;
    eax = eax - ecx;
    PrefWinGeometryValue_8577E0 = eax;
}

/*
Purpose: sub_48d7b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7B0 sub_48d7b0 0x0048D7B0-0x0048D7C4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d7b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d7b0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577D8;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    PrefWinGeometryValue_8578B4 = ecx;
    return eax;
}

/*
Purpose: sub_48d7d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7D0 sub_48d7d0 0x0048D7D0-0x0048D7E4 BYTE_EXACT
// LEVER: the first global's register must stay live across the add.
//   As `void`, VC6 computes the sum into the FIRST global's register
//   (`add eax, ecx`, so the image's `add ecx, eax` reads as a different
//   program). Six statement spellings scored identically - the destination is
//   chosen by liveness, not by which operand is written first. Returning the
//   first global keeps its register live, VC6 has to add into the second
//   one's register, and the bytes match with no extra instruction: the value
//   is already in eax at the ret. No caller in the image observes the choice.
// symbol    ?global_arith_0048d7d0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int32_t __fastcall global_arith_0048d7d0_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8577DC;
    int32_t ecx = PrefWinGeometryConstant_686B78;
    ecx = ecx + eax;
    PrefWinGeometryValue_8578BC = ecx;
    return eax;
}

/*
Purpose: sub_48d7f0 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7F0 sub_48d7f0 0x0048D7F0-0x0048D80B SEMANTIC
// TRIED 2026-09-03: same six shapes as 0x0048D610, same wall - see that
//   marker. The image's `add ecx,eax` needs the first global live past the
//   first add, and its register is the one the third global's reload wants.
// symbol    ?global_arith_0048d7f0_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall global_arith_0048d7f0_redirect(void *, void *) {
    int32_t eax = PrefWinToggleWidth;
    int32_t ecx = PrefWinGeometryConstant_686B74;
    ecx = ecx + eax;
    eax = PrefWinGeometryValue_8578B4;
    ecx = ecx + eax;
    PrefWinGeometryValue_8578CC = ecx;
}

/*
Purpose: sub_48d810 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D810 sub_48d810 0x0048D810-0x0048D81E
// TRIED: the last of four instructions. The image has `add eax, -2` (83 C0 FE); VC6 emits `sub eax, 2` (83 E8 02) and will not be talked out of it. Six spellings measured with `tools/try_spellings.py` - `eax + -2`, `eax += -2`, unsigned `+ 0xFFFFFFFEu`, an int cast around the unsigned add, and both one-liner forms - all score 3 of 4, identically. All ten flag sets score 3 of 4 or worse. The two are semantically the same instruction; the encoding is not reachable from C++ with this compiler.
// symbol    ?global_arith_0048d810_redirect@@YIXPAX0@Z
// size      14 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
// TRIED: the one divergence is an ENCODING choice VC6 will not make - the
//   image has `add eax, -2` (83 C0 FE), this tree emits `sub eax, 2`
//   (83 E8 02), same semantics and same length. `eax = eax + -2` is
//   already the source form and VC6 canonicalises it to a subtract;
//   so does the unsigned spelling `eax + 0xFFFFFFFEu`, measured. A
//   literal negative addend cannot reach that encoding, so whatever
//   the original wrote, the -2 did not arrive as a constant here.
// TRIED 2026-09-03, second pass, still 3 of 4: `eax -= 2`, `eax = eax - 2`,
//   `eax + ~1` (~1 is -2), `eax - 1 - 1`, `eax + 0xFFFFFFFE` and the unsigned
//   wrap `uint32_t eax + 0xFFFFFFFEu` all compile the identical
//   `mov/sub/store/ret`; `eax - (-2)` compiles `add eax, 2`, which is the
//   proof that the sign of the immediate is what VC6 canonicalises away.
//   /G6 measured too, in the tree's own ladder and directly: identical.
//   The register-liveness lever that landed the other 24 bodies in this file
//   does not apply - there is one variable and it is live to the store either
//   way, so no spelling of the statement changes the encoder's choice.
*/
void __fastcall global_arith_0048d810_redirect(void *, void *) {
    int32_t eax = PrefWinGeometryValue_8578BC;
    eax = eax + -2;
    PrefWinGeometryValue_8578D0 = eax;
}

// Original final initializer in the pinned geometry family.
// ORIGINAL: 0x0048D820 sub_48d820 0x0048D820-0x0048D83E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT table entry selects 0048D820, whose typed arithmetic and output store were independently decoded from the pinned executable.", "recovery/tasks/prefwin-geometry-initializers.json", "src/global_arith.h"], "reason": "Recover the missing scalar initializer in the ordered PrefWin geometry startup family through genuine source-owned integer storage; retain the compatible historical exact seed and complete original span."}
// symbol    _sub_48d820
extern "C" void __cdecl sub_48d820() {
    PrefWinGeometryValue_8578B8 =
        PrefWinGeometryValue_8577E4 - PrefWinGeometryConstant_686B74 * 3 -
        PrefWinToggleWidth;
}

// Let the compiler place this constructor in its ordinary startup initializer
// section. Its helper-call sequence follows the pinned CRT slot order
// 0x68257C..0x6825E0; this is one compiler-generated entry, not a claim that
// the original had the same per-helper initializer shape. 0x0044C870 is an
// unrelated toggle and is deliberately not part of it.
namespace {
struct PrefWinGeometryBootstrap {
    PrefWinGeometryBootstrap() {
        global_arith_0048d550_redirect(nullptr, nullptr);
        global_arith_0048d560_redirect(nullptr, nullptr);
        global_arith_0048d570_redirect(nullptr, nullptr);
        global_arith_0048d590_redirect(nullptr, nullptr);
        global_arith_0048d5b0_redirect(nullptr, nullptr);
        global_arith_0048d5d0_redirect(nullptr, nullptr);
        global_arith_0048d5f0_redirect(nullptr, nullptr);
        global_arith_0048d610_redirect(nullptr, nullptr);
        global_arith_0048d630_redirect(nullptr, nullptr);
        sub_48d650();
        global_arith_0048d670_redirect(nullptr, nullptr);
        global_arith_0048d690_redirect(nullptr, nullptr);
        global_arith_0048d6b0_redirect(nullptr, nullptr);
        global_arith_0048d6d0_redirect(nullptr, nullptr);
        global_arith_0048d6f0_redirect(nullptr, nullptr);
        global_arith_0048d710_redirect(nullptr, nullptr);
        global_arith_0048d730_redirect(nullptr, nullptr);
        global_arith_0048d750_redirect(nullptr, nullptr);
        global_arith_0048d760_redirect(nullptr, nullptr);
        global_arith_0048d770_redirect(nullptr, nullptr);
        global_arith_0048d790_redirect(nullptr, nullptr);
        global_arith_0048d7b0_redirect(nullptr, nullptr);
        global_arith_0048d7d0_redirect(nullptr, nullptr);
        global_arith_0048d7f0_redirect(nullptr, nullptr);
        global_arith_0048d810_redirect(nullptr, nullptr);
        sub_48d820();
    }
};

PrefWinGeometryBootstrap prefWinGeometryBootstrap;
}  // namespace
