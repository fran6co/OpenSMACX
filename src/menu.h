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
#include "font.h"
#include "graphicwin.h"
#include "spot.h"

class PullDown;

typedef void (__cdecl *MenuProc)(int);

struct MenuEntry {
  int id;
  char *text;
  uint8_t flags;
  uint8_t padding[3];
  char *mnemonic;
  PullDown *pull_down;
};

static_assert(sizeof(MenuEntry) == 0x14,
              "MenuEntry layout must match the legacy ABI");
static_assert(offsetof(MenuEntry, flags) == 0x8,
              "MenuEntry flags offset must match the legacy ABI");

 /*
  * Menu class
  */
// Slot 0xF8 of the Menu's OWN vtable: the repaint the flag operations end on.
// It takes no arguments and its result is discarded.
typedef void (OriginalObject::*func_menu_repaint)();

class Menu : public GraphicWin {
 public:
  void on_mouse_move(int a1, int a2);
  // ?mouse_move@Menu@@QAEXHH@Z (0x005FBDB0, 333 bytes) is unrecovered;
  // on_mouse_move is an 18-byte tail-forward to it. Declared so that
  // forward names a real member - it still needs a body before the DLL links.
  void mouse_move(int a1, int a2);
  int requested_height();
  int UNK2(int a);
  int UNK6(int menu_id);
  int UNK7(int menu_id);
  int UNK8(int menu_id);
  int UNK9(int menu_id);
  int UNK4(int a, int b, int c);
  Menu();
  // NOT a constructor: see the "NOT a constructor" note in log.h - an
  // ordinary method drops the SEH frame a real derived GraphicWin
  // constructor picks up under /GX (FlatButton's/PullDown's own notes).
  Menu *construct();
  // 0x005FAD00 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~Menu();

  void on_adjust_pulldown_pos(int *a1, int *a2);
  void set_menu_proc(MenuProc proc);
  int id_to_index(int id);
  int UNK3(int menu_id, int item_id);
  int hide_menu_item(int menu_id, int item_id);
  int show_menu_item(int menu_id, int item_id);
  int disable_menu_item(int menu_id, int item_id);
  int enable_menu_item(int menu_id, int item_id);
  int check_menu_item(int menu_id, int item_id);
  int uncheck_menu_item(int menu_id, int item_id);

 private:
  MenuProc proc_;
  int count_;
  uint32_t field_A1C_;  // 0x0a1c
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
  Spot spot_;
  MenuEntry entries_[15];

 public:
  // MENU'S OWN VTABLE SLOTS, derived from a dispatch the image spells out.
  // Win declares 0..87 and GraphicWin appends on_dialog_focus and
  // on_mouse_leave at 88 and 89, so Menu's first addition is 90.
  // `Win::adjust_menus` (0x005F0540, BYTE_EXACT) calls this object at
  // `[vtable + 0x174]` = slot 93, which is what fixes the count at four:
  // three unnamed slots and the one the dispatch proves exists. Naming any
  // of them is a later measurement; the declaration is what lets the
  // compiler emit the dispatch instead of a fake-class shim.
  virtual void vslot_90() {}
  virtual void vslot_91() {}
  virtual void vslot_92() {}
  virtual void vslot_93() {}
};

static_assert(sizeof(Menu) == 0xB64, "Menu layout must match the legacy ABI");




// The menu's own font, falling back to the process default when unset.
inline Font *&MenuFont() { return *reinterpret_cast<Font **>(0x009B7B94); }

// mouse_move is declared on the class because the recovered on_mouse_move is
// an 18-byte tail-forward to it, but its 333-byte body is not recovered, so
// the DLL has nothing to link that call against. It stands in with a forward
// to the original image until the body lands.
//   ?mouse_move@Menu@@QAEXHH@Z  0x005FBDB0  public, __thiscall, void(int, int)
