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
#include "general.h"
#include "veh.h"

// Separate this mapped owner from veh_cost to retain its witnessed direct
// call boundary. The original object-file/TU assignment is unknown.
/*
Purpose: Calculates the base cost of the specified prototype.
// ORIGINAL: 0x005A5D00 ?base_cost@@YAHH@Z 0x005A5D00-0x005A5D3F BYTE_EXACT
// body      src/vehicle_cost.cpp
// size      63 bytes
// prototype int (__cdecl ?base_cost@@YAHH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A5A60
Return Value: Base cost of the prototype
Status: Complete
*/
int __cdecl base_cost(int proto_id) {
    return proto_cost(VehPrototypes[proto_id].chassis_id, VehPrototypes[proto_id].weapon_id,
        VehPrototypes[proto_id].armor_id, 0, VehPrototypes[proto_id].reactor_id);
}
