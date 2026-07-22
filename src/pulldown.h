/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once
#include "graphicwin.h"

class Menu;

struct PullDownItem {
  char *text;
  char *right_text;
  int id;
  uint32_t flags;
  char *mnemonic;
};

static_assert(sizeof(PullDownItem) == 0x14,
              "PullDownItem layout must match the legacy ABI");
static_assert(offsetof(PullDownItem, id) == 0x8,
              "PullDownItem ID offset must match the legacy ABI");
static_assert(offsetof(PullDownItem, flags) == 0xC,
              "PullDownItem flags offset must match the legacy ABI");

class DLLEXPORT PullDown : GraphicWin {
 public:
  int UNK2(int a);
  int UNK3(int a, int b, int c);
  int UNK5();
  int UNK6();
  PullDown() { ; }
  ~PullDown() { ; }

  int hide_item(int id);
  int show_item(int id);
  int disable_item(int id);
  int enable_item(int id);
  int check_item(int id);
  int uncheck_item(int id);
  int get_selected();

  void on_mouse_leave(int a1, int a2);
 private:
  uint32_t field_A14_;
  PullDownItem items_[64];
  uint32_t field_F18_;
  uint32_t field_F1C_;
  int item_count_;
  int width_;
  int selected_;
  int visible_count_;
  Menu *menu_;
  uint8_t dirty_;
  uint8_t dirty_padding_[3];
  uint32_t field_F38_;
  uint32_t field_F3C_;
};

static_assert(sizeof(PullDown) == 0xF40,
              "PullDown layout must match the legacy ABI");

// Item text allocations come from the executable's CRT, so the destructor
// must release them through its free rather than this module's. Tests
// outside the hybrid process rebind this.
extern func_sprite_free *PullDownFree;

// Original virtual table addresses the destructor installs before delegating
// to the GraphicWin destructor (which then overwrites both with its own).
extern const uint32_t PullDownPrimaryVtable;
extern const uint32_t PullDownBufferVtable;

// Defaults the destructor copies into the two trailing fields; the game keeps
// them at 0x009B7B58/0x009B7B5C. Tests outside the hybrid process rebind
// them at locally mapped storage.
extern uint32_t *PullDownFieldF38Default;
extern uint32_t *PullDownFieldF3CDefault;

PullDown *__fastcall pull_down_destructor_redirect(PullDown *self, void *);
int __fastcall pull_down_hide_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_show_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_disable_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_enable_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_check_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_uncheck_item_redirect(
    PullDown *self, void *, int id);
int __fastcall pull_down_get_selected_redirect(PullDown *self, void *);

int __fastcall pull_down_unk2_redirect(
    PullDown *self, void *, int a);
int __fastcall pull_down_unk3_redirect(
    PullDown *self, void *, int a, int b, int c);
int __fastcall pull_down_unk5_redirect(
    PullDown *self, void *);
int __fastcall pull_down_unk6_redirect(
    PullDown *self, void *);
void __fastcall pull_down_on_mouse_leave_redirect(PullDown *self, void *, int a1, int a2);
