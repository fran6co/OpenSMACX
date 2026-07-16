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
  PullDown() { ; }
  ~PullDown() { ; }

  int hide_item(int id);
  int show_item(int id);
  int disable_item(int id);
  int enable_item(int id);
  int check_item(int id);
  int uncheck_item(int id);
  int get_selected();

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
