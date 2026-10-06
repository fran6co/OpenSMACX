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

/*
 * PrefWin geometry outputs are natural dynamic initializers owned in
 * prefwin_geometry.cpp. This file retains the shared geometry inputs and the unrelated
 * fixed-pointer toggle at 0x0044C870.
 */


/*
Purpose: sub_44c870 - arithmetic over 1 fixed global(s), one
         statement per original instruction.
// RECOVERY_ROLE: SCAFFOLD Transitional address-named arithmetic/initializer adapter; preserve behavior while recovering genuine startup ownership.
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
