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
Original Offset: 00614B80
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
Original Offset: 00614350
Return Value: n/a
Status: Complete
*/
void FileWin::UNK2(int) {
}

void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1) {
    self->UNK2(a1);
}
