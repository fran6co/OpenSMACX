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
  void clear_group();
  void set_preferences();
  void set_auto_preferences();
  void set_base_preferences();
  void set_audiovisual();
  void set_map_display();
  int edit_lock();
  void set_adv_preferences();
  void editor_undo();

 private:
  uint8_t derived_storage_[0x23D94];
  GraphicWin virtual_base_;
};

static_assert(sizeof(Console) == 0x247A8, "Console layout must match terranx.exe");

// All five preference openers drive the one PrefWin the game keeps at a fixed
// address, opening it to a different page. PrefWin::display is not recovered.
typedef void (__thiscall func_pref_win_display)(void *pref_win, int page);
extern func_pref_win_display *ConsolePrefWinDisplay;
extern void *ConsolePrefWin;

void __fastcall console_set_preferences_redirect(Console *self, void *);
void __fastcall console_set_auto_preferences_redirect(Console *self, void *);
void __fastcall console_set_base_preferences_redirect(Console *self, void *);
void __fastcall console_set_audiovisual_redirect(Console *self, void *);
void __fastcall console_set_map_display_redirect(Console *self, void *);

// clear_group masks a bit in each entry of a group table the game keeps at a
// fixed address, counted by another fixed-address field; both are rebindable
// so tests drive them against a local table.
extern int32_t *ConsoleGroupCount;
extern uint8_t *ConsoleGroupTable;

void __fastcall console_clear_group_redirect(Console *self, void *);

// edit_lock consults the Scroll Lock key through the game's imported
// GetKeyState. The seam is the address of that import slot (the IAT entry the
// original calls indirectly), so it reads the live pointer at run time and
// stays rebindable for tests.
typedef SHORT(__stdcall func_get_key_state)(int virtual_key);
extern func_get_key_state **ConsoleEditKeyStateSlot;

int __fastcall console_edit_lock_redirect(Console *self, void *);
void __fastcall console_set_adv_preferences_redirect(Console *self, void *);
void __fastcall console_editor_undo_redirect(Console *self, void *);
