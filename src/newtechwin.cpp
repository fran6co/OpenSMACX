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
#include "newtechwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00483E90 ?on_left_click@NewTechWin@@QAEXHH@Z 0x00483E90-0x00483E93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_click@NewTechWin@@QAEXHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NewTechWin::on_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x00483EA0 ?on_key_click@NewTechWin@@QAEHHH@Z 0x00483EA0-0x00483EA8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@NewTechWin@@QAEHHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int NewTechWin::on_key_click(int, int) {
    return 1;
}

