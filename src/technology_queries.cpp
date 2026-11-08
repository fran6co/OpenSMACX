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
#include "technology.h"
#include "strings.h"
#include "veh.h"
#include "faction.h"

// A technology-family function whose image address lies OUTSIDE the technology
// TU (0x005B9C10-0x005BEC3D): say_tech(int, BOOL) sits in the faction TU's
// tail and calls the technology entry out of line, so it must not share
// technology.cpp. It stays here until that TU is regrouped. valid_tech_leap
// moved into veh.cpp, whose image run (before study_artifact) contains it.

/*
Purpose: Get tech string for techID and store it into stringTemp buffer.
// ORIGINAL: 0x005591A0 ?say_tech@@YAXHH@Z 0x005591A0-0x005591BA BYTE_EXACT
// size      26 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9C40
Return Value: n/a
Status: Complete
*/
void __cdecl say_tech(int tech_id, BOOL category_lvl) {
    say_tech(StringTemp, tech_id, category_lvl);
}
