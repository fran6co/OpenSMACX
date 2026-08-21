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

/*
Purpose: Tear down a UV2Player: close it, then destroy its Buffer member.
         buffer_ is a real member, so the compiler installs its own SEH
         unwind protection around close() automatically - matching the
         image, which protects the same call so the Buffer subobject is
         still destroyed if close() throws.
// ORIGINAL: 0x004043D0 ??1UV2Player@@QAE@XZ 0x004043D0-0x0040441C;0x00650740-0x00650758 BYTE_EXACT
// LEVER: MEASURED 22/22 agreeing. `Buffer buffer_` being a REAL typed member (not raw storage) is what does it - the compiler installs the exact SEH-wrapped-close()-then-destroy-Buffer shape on its own once the member is real, so the body is just `close();` with no explicit try/catch and no explicit `~Buffer()` call at all.
// size      100 bytes
// prototype void (__thiscall ??1UV2Player@@QAE@XZ)(UV2Player* this)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF400 0x005D7410
Return Value: n/a
Status: Complete
*/
UV2Player::~UV2Player() {
    close();
}
