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
#include "original_seam.h"
#include "popmenu.h"
#include "dialog.h"
#include "sounddevice.h"
#include "basepop.h"


/*
Purpose: Initialise the pop-up menu with the base defaults.
// ORIGINAL: 0x0059D3A0 ?init@PopMenu@@QAEHXZ 0x0059D3A0-0x0059D3AA BYTE_EXACT
// size      10 bytes
// prototype int (__thiscall ?init@PopMenu@@QAEHXZ)(PopMenu* this)
// callers   14   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006015B0
Return Value: whatever BasePop::init returns
Status: Complete
*/
int PopMenu::init() {
    return BasePop::init(0, 0);
}

