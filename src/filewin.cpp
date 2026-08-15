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
#include "filewin.h"
#include "text.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x00614B80 BYTE_EXACT
// name      ?UNK6@FileWin@@QAEXXZ
// size      1 bytes
// spans     0x00614B80-0x00614B81
// prototype void (__thiscall ?UNK6@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK6() {
}

void __fastcall file_win_unk6_redirect(FileWin *self, void *) {
    self->UNK6();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00614350 BYTE_EXACT
// name      ?UNK2@FileWin@@QAEXH@Z
// size      3 bytes
// spans     0x00614350-0x00614353
// prototype void (__thiscall ?UNK2@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK2(int) {
}

void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1) {
    self->UNK2(a1);
}

/*
Purpose: Clear the file window's three name buffers, its selection index and
         one trailing flag.
ORIGINAL: 0x00614320 BYTE_EXACT
// name      ?UNK1@FileWin@@QAEXXZ
// size      33 bytes
// spans     0x00614320-0x00614341
// prototype void (__thiscall ?UNK1@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK1() {
    field_208_ = 0;
    field_30C_ = 0;
    field_410_ = 0;
    field_531_ = 0;
    field_514_ = 0;
}

void __fastcall file_win_unk1_redirect(FileWin *self, void *) {
    self->UNK1();
}

/*
ORIGINAL: 0x00614C40 BYTE_EXACT
// name      ?on_double_clicked@FileWin@@QAEXH@Z
// size      8 bytes
// spans     0x00614C40-0x00614C48
// prototype void (__thiscall ?on_double_clicked@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006146A0
Status: Complete
*/
void FileWin::on_double_clicked(int a1) {
    UNK4();
}

func_file_win_unk4 FileWinUNK4 =
    original_method<func_file_win_unk4>(0x006146A0);

/*
 * A forwarder, not a recovery. on_double_clicked above reaches UNK4 with a
 * direct `call rel32` - which is what makes that body byte-exact - so the DLL
 * must resolve the symbol, but 0x006146A0 is not decoded yet. No
 * `Original Offset:` line by design: that annotation marks a recovered body
 * and is indexed by address.
 */
void FileWin::UNK4() {
    (ORIGINAL(this)->*FileWinUNK4)();
}

/*
ORIGINAL: 0x00614D90 BYTE_EXACT
// name      ?init_class@FileWin@@QAAHXZ
// size      158 bytes
// spans     0x00614D90-0x00614E2E
// prototype
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005FD550 0x005FD570 0x00644EF2 0x006453E0 0x00645470
//
// Promoted 2026-08-15 from src/unrecovered/00614d90.cpp, where it measured
// BYTE_EXACT as a self-contained FILE unit, to retire its pending_bodies
// forwarder. The FILE unit forced the strlen/strcat calls through `_strlen`/
// `_strcat` externs - one underscore too deep to LINK, a debt FILE-mode
// measurement never sees. The original plainly called the CRT, so this does
// too; the measurement scaffold supplies the `#pragma function` that keeps
// the calls real where the image has real calls (string_routine_pragma).
Status: Complete
*/
static int *const g_006971cc = (int *)0x006971CC;
static int *const g_006971d4 = (int *)0x006971D4;
static int *const g_009b90a8 = (int *)0x009B90A8;

int __cdecl FileWin::init_class() {
    if (*g_009b90a8 != 0) {
        free(reinterpret_cast<void *>(*g_009b90a8));
        *g_009b90a8 = 0;
    }
    if (text_open(reinterpret_cast<char *>(g_006971d4), reinterpret_cast<char *>(g_006971cc)) != 0) {
        return 6;
    }
    char *str = text_get();
    if (str == 0) {
        return 1;
    }
    if (*g_009b90a8 != 0) {
        free(reinterpret_cast<void *>(*g_009b90a8));
        *g_009b90a8 = 0;
    }
    *g_009b90a8 = reinterpret_cast<int>(mem_get(strlen(str) + 1));
    if (*g_009b90a8 == 0) {
        return 4;
    }
    *reinterpret_cast<char *>(*g_009b90a8) = 0;
    strcat(reinterpret_cast<char *>(*g_009b90a8), str);
    return 0;
}
