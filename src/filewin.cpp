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

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x00614B80 BYTE_EXACT
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
