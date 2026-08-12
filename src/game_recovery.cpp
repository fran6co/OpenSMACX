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
#include "game.h"

/*
 * Game-scoped bindings and small recovered bodies that code outside
 * game.cpp needs.
 *
 * Same split, and for the same reason, as font_recovery.cpp: game.cpp reaches
 * Players, parse_set and parse_says, so any target that links it drags the
 * whole alpha/base closure in. The leaf test suite cannot afford that, and
 * Console::focus - which only wants to compare a faction id - would otherwise
 * have to mint a third name for an address that game.cpp and netdaemon.cpp
 * already bind twice between them. Nothing here may acquire a dependency: the
 * point of the file is that it has none.
 */

// The faction the human is playing, at 0x00939284. netdaemon.cpp binds the
// same address as NetDaemonLocalFaction; that duplicate predates this file and
// is left alone rather than churned, but new callers should reach for this one.
int *LocalFaction = (int *)0x00939284;

/*
Purpose: The ceiling on a single energy allocation slider. The legacy
         implementation is a constant return.
ORIGINAL: 0x00445440 BYTE_EXACT
// name      ?energy_limit@@YAHH@Z
// size      6 bytes
// spans     0x00445440-0x00445446
// prototype int (__cdecl ?energy_limit@@YAHH@Z)(int factionID)
// callers   3   call targets   0
Return Value: 10
Status: Complete
*/
int __cdecl energy_limit(int) {
    return 10;
}
