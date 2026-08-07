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

#include "original_seam.h"
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
  void editor_polar();
  void on_sys_close();
  void menu_update();
  void editor_redo();
  void editor_climate();
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
  void update_data(int a1);
  int focus(int x_coord, int y_coord, int faction_id);

 private:
  uint8_t derived_storage_[0x23D94];
  GraphicWin virtual_base_;
};

static_assert(sizeof(Console) == 0x247A8, "Console layout must match terranx.exe");

// All five preference openers drive the one PrefWin the game keeps at a fixed
// address, opening it to a different page. PrefWin::display is not recovered.
typedef void (OriginalObject::*func_pref_win_display)(int page);
extern func_pref_win_display ConsolePrefWinDisplay;
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

// update_data drives the three fixed windows the original reaches by absolute
// address and never through `this` - ecx is dead on entry and is overwritten
// by `mov ecx, 0x7ad2a0` before anything reads it. The InfoWin at 0x007AD2A0
// and the StatusWin at 0x008C5568 are the objects themselves (loaded as
// immediates); 0x007D3C3C is a slot holding the current MapWin pointer
// (loaded with `mov ecx, dword ptr [0x7d3c3c]`), published once at startup by
// 0x0058F1C1, so it is read at call time exactly like ConsoleEditKeyStateSlot
// above. All four bindings are rebindable so tests can substitute local
// objects and probes.
//
// InfoWin::change (0x00458900) and MapWin::main_caption (0x0046FB10) are
// source_complete and are called directly; StatusWin::redraw (0x004B9EA0) is
// still original - it pulls in the 8885-byte draw_status, two indirect calls
// through the runtime slot at 0x00669328, and GraphicWin::soft_update - so it
// is reached through a rebindable seam, exactly as ListBox reaches
// Dialog::close.
typedef void (OriginalObject::*func_status_win_redraw)();
extern func_status_win_redraw ConsoleOriginalStatusWinRedraw;  // 0x004B9EA0
extern void *ConsoleInfoWin;      // 0x007AD2A0, the process-wide InfoWin
extern void *ConsoleStatusWin;    // 0x008C5568, the process-wide StatusWin
extern void **ConsoleMapWinSlot;  // 0x007D3C3C, holds the current MapWin *

void __fastcall console_update_data_redirect(Console *self, void *, int a1);

// focus walks the eight map-window slots and, for the primary window only,
// runs the console cursor and the survey-overlay latch first. Four of its
// callees are still original bodies, so each gets a rebindable seam. Nothing
// here binds an address that is already bound: the table is mapwin.h's
// MapWinTable at 0x007D3C3C (which also carries MapWinTableSlots and
// MapWinActiveOffset) and the faction id is game.h's LocalFaction.
//
// Console::cursor_next (0x005109B0, ret 8) is entered on the process-wide
// Console the original names with `mov ecx, 0x9156b0` at 0x00510921 - never on
// `this`. That object is bound separately so a test can hand focus a Console
// that is NOT the process-wide one and observe which of the two each half of
// the body touches.
typedef void (OriginalObject::*func_console_cursor_next)(int x_coord, int y_coord);
// MapWin::focus (0x0046B310, ret 8) reports whether it moved the view;
// MapWin::draw_map (0x0046A550, ret 4) repaints one. Both are unrecovered -
// focus reaches in_box and set_center, draw_map is 2049 bytes - so both stay
// original behind a seam.
typedef int (OriginalObject::*func_console_map_win_focus)(int x_coord, int y_coord);
// ?flush_input@@YAXXZ at 0x005FD120 - __cdecl, no arguments - drains the queued
// input once the focus walk actually moved the primary view. Still original; it
// pumps the message loop through check_net.
typedef void(__cdecl func_console_flush_input)(void);

extern func_console_cursor_next ConsoleOriginalCursorNext;          // 0x005109B0
extern func_console_map_win_focus ConsoleOriginalMapWinFocus;       // 0x0046B310
// 0x0046A550 is NOT bound here. It is MapWin::draw_map's own address and
// src/mapwin.h owns the seam for it (MapWinOriginalDrawMap). Seams dedupe on
// the ADDRESS, never the name: two variables for one address is not a link
// error, so a fixture rebinding one would leave the other still pointing at
// the original image - which is why tools/test_generator_support.py refuses
// a second binding, and it caught this one.
extern func_console_flush_input *ConsoleOriginalFlushInput;          // 0x005FD120
extern void *ConsoleGlobal;  // 0x009156B0, the process-wide Console

// The dword at 0x0093A938 is set while a turn is played out under program
// control rather than interactively. A linear scan of .text finds exactly four
// writers: control_turn stores 1 at 0x00527680 and 0 at 0x0052823B,
// net_end_of_turn stores 1 at 0x0052A1CB, and net_control_turn stores 1 at
// 0x0052A301 and 0 at 0x0052A4ED. Over a hundred sites read it, always as a
// plain zero/non-zero gate. focus consults it only at 0x00510910, to decide
// whether an untagged primary window still gets the cursor path.
extern int32_t *ConsoleControlTurnActive;  // 0x0093A938

// on_sys_close raises the flag the turn loop watches. The dword at 0x009B2068
// is the same one src/scenario.cpp binds as `ExitTurnLoopAddress`.
extern int32_t *ConsoleExitTurnLoop;  // 0x009B2068

// menu_update hands the console's selected-vehicle id to the MainMenu the
// original embeds at +0x22A2C. MainMenu::check (0x00460DD0, 4782 bytes) is
// unrecovered and MainMenu has no header in src/, so this is a seam rather
// than a declared type.
typedef void (OriginalObject::*func_main_menu_check)(int veh_id);
extern func_main_menu_check ConsoleOriginalMainMenuCheck;  // 0x00460DD0

int __fastcall console_focus_redirect(Console *self, void *, int x_coord,
                                      int y_coord, int faction_id);
