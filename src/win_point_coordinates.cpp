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
#include "win.h"

/*
Purpose: Translate a client-relative point into screen coordinates by walking
         the parent chain.
// ORIGINAL: 0x005ED240 ?client_to_screen@Win@@QAEXPAH0@Z 0x005ED240-0x005ED2C5 BYTE_EXACT
// LEVER: plain signed `+`/`-` instead of the `int_from_bits`/`long_bits` bit-cast chain - the image is straight `add`/`sub`, no cast dance. 21/44 MISMATCH -> BYTE_EXACT.
// size      133 bytes
// prototype void (__thiscall ?client_to_screen@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   71   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
Status: Complete
*/
void Win::client_to_screen(int *x, int *y) {
    *x = *x + client_rect_.left + outer_rect_.left;
    *y = *y + client_rect_.top + outer_rect_.top;
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((iFlags_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->client_to_screen(x, y);
    // Bit 15 additionally backs out the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each subtraction rather than caching it.
    if ((iFlags_ & 0x8000U) == 0) {
        return;
    }
    *x = *x - win_parent_->outer_rect_.left;
    *y = *y - win_parent_->outer_rect_.top;
}

/*
Purpose: The inverse of client_to_screen above.
// ORIGINAL: 0x005ED2D0 ?screen_to_client@Win@@QAEXPAHPAH@Z 0x005ED2D0-0x005ED355 BYTE_EXACT
// symbol    ?screen_to_client@Win@@QAEXPAH0@Z
// size      133 bytes
// prototype void (__thiscall ?screen_to_client@Win@@QAEXPAHPAH@Z)(Win* this, int*, int*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
Return Value: n/a
Status: Complete
*/
void Win::screen_to_client(int *x, int *y) {
    *x = *x - (client_rect_.left + outer_rect_.left);
    *y = *y - (client_rect_.top + outer_rect_.top);
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((iFlags_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->screen_to_client(x, y);
    // Bit 15 additionally adds back the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each addition rather than caching it.
    if ((iFlags_ & 0x8000U) == 0) {
        return;
    }
    *x = *x + win_parent_->outer_rect_.left;
    *y = *y + win_parent_->outer_rect_.top;
}

// ===== homed from src/unrecovered/005ed3f0.cpp =====

// ORIGINAL: 0x005ED3F0 ?nonclient_to_screen@Win@@QAEXPAH0@Z 0x005ED3F0-0x005ED48D FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      157 bytes
// prototype void (__thiscall ?nonclient_to_screen@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
void Win::nonclient_to_screen(int * x, int * y) {
    client_to_screen(x, y);
    *x = *x - outer_rect_.left;
    *y = *y - outer_rect_.top;
}

