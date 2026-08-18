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
#include "uv2player.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004BF380 ?UNK4@UV2Player@@QAEHXZ 0x004BF380-0x004BF383 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK4@UV2Player@@QAEHXZ)(UV2Player* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int UV2Player::UNK4() {
    return 0;
}

int __fastcall uv2_player_unk4_redirect(UV2Player *self, void *) {
    return self->UNK4();
}
