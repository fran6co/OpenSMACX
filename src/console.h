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
#include "graphicwin.h"

 /*
  * Console class
  *
  * The largest unrecovered class in the image - 132KB across 148 methods -
  * and the reason the layout campaign exists.
  *
  * As with MapWin, the original derives this from GraphicWin virtually and
  * that cannot be written as `: virtual GraphicWin` here: MSVC places a
  * virtual base where its vbtable says, the Itanium ABI this toolchain
  * follows places it after the derived object, so the faithful-looking
  * declaration is the one that silently produces the wrong layout. The base
  * is held as a member at the offset MSVC put it.
  *
  * The size is read rather than inferred: the vbtable at 0x0066EF04 reads
  * {0, 0x23D94}, and GraphicWin is pinned at 0xA14, summing to the 0x247A8
  * asserted below. Independently g_CONSOLE's global slot bounds the object
  * above at 0x247D8, consistent with 0x30 to spare - two readings sharing no
  * evidence.
  *
  * Console reaches GraphicWin through MapWin rather than directly: its
  * constructor calls MapWin's before anything else, so MapWin's own fields
  * occupy the front of derived_storage_. Nothing here depends on that, but it
  * is where mapping this storage should start.
  *
  * Fields must be carved out of derived_storage_ as methods are recovered,
  * keeping the total fixed. Appending would move the virtual base and break
  * every offset in the class.
  */
class DLLEXPORT Console {
 public:
  Console() { ; }
  ~Console() { ; }

 private:
  uint8_t derived_storage_[0x23D94];
  GraphicWin virtual_base_;
};

static_assert(sizeof(Console) == 0x247A8, "Console layout must match terranx.exe");
