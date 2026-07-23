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
  * MapWin class
  *
  * The original derives this from GraphicWin *virtually*, and that cannot be
  * written as `: virtual GraphicWin` here. MSVC places a virtual base at the
  * offset its vbtable names, while the Itanium ABI this toolchain follows
  * places it after the derived object entirely - so the faithful-looking
  * declaration is the one that would silently produce the wrong layout. The
  * base is therefore held as a member at the offset MSVC put it, which
  * reproduces the original addresses under either ABI.
  *
  * The size is the vbtable's, not a guess: the table at 0x0066C870 reads
  * {0, 0x21A6C}, placing the virtual base at 0x21A6C, and GraphicWin is
  * already pinned at 0xA14. Those sum to the 0x22480 asserted below.
  * Independently, g_MAPWIN's global slot bounds the object above at 0x22798,
  * which is consistent with 0x318 to spare - two readings sharing no evidence.
  *
  * The 0x21A6C bytes ahead of the base are not mapped. Fields must be carved
  * out of derived_storage_ as methods are recovered, keeping the total fixed,
  * rather than appended - appending would move the virtual base and break
  * every offset in the class.
  */
class DLLEXPORT MapWin {
 public:
  MapWin() { ; }
  ~MapWin() { ; }
  void UNK3();
  void do_image_buttons();
  void main_caption();
  void close();

 private:
  // The vbtable pointer opens the object; a heap pointer close() frees sits at
  // 0x4. Both are carved out of the derived storage, keeping the total the
  // static_assert pins.
  uint32_t vbtable_pointer_;
  void *owned_;
  uint8_t derived_tail_[0x21A6C - 0x8];
  GraphicWin virtual_base_;
};

static_assert(sizeof(MapWin) == 0x22480, "MapWin layout must match terranx.exe");

void __fastcall map_win_unk3_redirect(MapWin *self, void *);
void __fastcall map_win_do_image_buttons_redirect(MapWin *self, void *);

// MapWin::main_caption sets the date on the one MainInterface the original
// keeps at a fixed address, using a caption that also lives at one.
typedef void (__thiscall func_set_date)(void *, char *);
extern func_set_date *MainInterfaceOriginalSetDate;
extern void *MainInterfaceGlobal;
extern char *MapWinMainCaption;

void __fastcall map_win_main_caption_redirect(MapWin *self, void *);

// The heap pointer at 0x4 is freed through the game's CRT boundary, the same
// address buffer.cpp frees through. Rebindable so tests observe it.
typedef void *func_map_win_free(void *);
extern func_map_win_free *MapWinFree;

void __fastcall map_win_close_redirect(MapWin *self, void *);
