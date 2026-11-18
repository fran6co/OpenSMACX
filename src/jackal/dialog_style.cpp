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
#include "dialog.h"

// Inferred original member: Dialog's two style-word setters occupy their own
// contribution at 0x00605180..0x006051BB, between BasePop's member and
// Scroll's. BasePop calls both directly (0x00602758, 0x00603B16) without
// inlining them, so they were not compiled in BasePop's translation unit, and
// their address band is separate from Dialog's own member (0x00608C10..).
// The original filename is unknown.

// Both setters operate on this Dialog style word, not on AutoSound.
// ORIGINAL: 0x00605180 ?UNK3@BasePop@@QAEXH@Z 0x00605180-0x0060519B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:602758-602775 forms the embedded Dialogs virtual Dialog base before calls605180/6051A0", "pinned:603B16-603B35 forms the same Dialog receiver and writes Dialog+20 before calling6051A0", "pinned:setters605180-60519B and6051A0-6051BB update low style bits preserving upper bits", "historical:BasePop catalogue names and byte claims retained, but identical-body folding does not establish a second BasePop owner"], "reason": "Bind the preserved setter identity to the actual Dialog receiver proved by original callers; remove the unsupported BasePop alias and its erroneous AutoSound flag attribution."}
// symbol    ?set_style_bit_0@Dialog@@QAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?UNK3@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
void Dialog::set_style_bit_0(int on) {
    if (on) {
        field_20_ |= 1U;
    } else {
        field_20_ &= ~1U;
    }
}

// ORIGINAL: 0x006051A0 ?UNK4@BasePop@@QAEXH@Z 0x006051A0-0x006051BB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:602758-602775 forms the embedded Dialogs virtual Dialog base before calls605180/6051A0", "pinned:603B16-603B35 forms the same Dialog receiver and writes Dialog+20 before calling6051A0", "pinned:setters605180-60519B and6051A0-6051BB update low style bits preserving upper bits", "historical:BasePop catalogue names and byte claims retained, but identical-body folding does not establish a second BasePop owner"], "reason": "Bind the preserved setter identity to the actual Dialog receiver proved by original callers; remove the unsupported BasePop alias and its erroneous AutoSound flag attribution."}
// symbol    ?set_style_bit_1@Dialog@@QAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?UNK4@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
void Dialog::set_style_bit_1(int on) {
    if (on) {
        field_20_ |= 2U;
    } else {
        field_20_ &= ~2U;
    }
}

