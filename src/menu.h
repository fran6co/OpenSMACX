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

class DLLEXPORT Menu : GraphicWin {
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
  Menu() { ; }
  ~Menu() { ; }

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
  uint32_t field_A1C_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
  Spot spot_;
  MenuEntry entries_[15];
};

static_assert(sizeof(Menu) == 0xB64, "Menu layout must match the legacy ABI");

MenuProc __fastcall menu_set_menu_proc_redirect(
    Menu *self, void *, MenuProc proc);
int __fastcall menu_id_to_index_redirect(Menu *self, void *, int id);
int __fastcall menu_unk6_redirect(Menu *self, void *, int menu_id);
int __fastcall menu_unk7_redirect(Menu *self, void *, int menu_id);
int __fastcall menu_unk8_redirect(Menu *self, void *, int menu_id);
int __fastcall menu_unk9_redirect(Menu *self, void *, int menu_id);
int __fastcall menu_unk3_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_hide_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_show_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_disable_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_enable_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_check_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);
int __fastcall menu_uncheck_menu_item_redirect(
    Menu *self, void *, int menu_id, int item_id);

int __fastcall menu_unk2_redirect(
    Menu *self, void *, int a);
int __fastcall menu_unk4_redirect(
    Menu *self, void *, int a, int b, int c);

int __fastcall menu_requested_height_redirect(Menu *self, void *);

// The menu's own font, falling back to the process default when unset.
extern Font **MenuFont;
void __fastcall menu_on_adjust_pulldown_pos_redirect(Menu *self, void *, int *a1, int *a2);
