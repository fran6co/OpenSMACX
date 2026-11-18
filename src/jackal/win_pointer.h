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
#pragma once

// Win::grab_pointer's inline body lives apart from win.h: only its callers'
// TUs see it, since the extra inline definition perturbs VC6 register choice
// in unrelated TUs (world_volcano/world_ridge in world.cpp).
#include "win.h"

// Claim the pointer when nobody holds it. Inlined as a Win member, so a
// virtual-base caller converts `this` without a null check.
inline void Win::grab_pointer() {
  if (WinPointerOwner3 == 0) {
    WinPointerOwner3 = this;
    WinPointerOwner4 = 0;
  }
}
