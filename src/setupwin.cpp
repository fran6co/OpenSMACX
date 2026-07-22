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
#include "setupwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
Original Offset: 004ADAD0
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK3(int) {
    return 0;
}

int __fastcall setup_win_unk3_redirect(SetupWin *self, void *, int a1) {
    return self->UNK3(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
Original Offset: 004ADAE0
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK4(int) {
    return 0;
}

int __fastcall setup_win_unk4_redirect(SetupWin *self, void *, int a1) {
    return self->UNK4(a1);
}
