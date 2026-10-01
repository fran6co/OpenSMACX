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
typedef int (__cdecl *PullDownPoll)();
typedef int (__cdecl *PullDownFillCallback)(GraphicWin *, int, RECT *);
typedef int (__cdecl *PullDownBackfillCallback)(GraphicWin *);
static_assert(sizeof(PullDownPoll) == 4, "PullDown callback uses the original x86 ABI");
static_assert(sizeof(PullDownFillCallback) == 4,
              "PullDown fill callback uses the original x86 ABI");
static_assert(sizeof(PullDownBackfillCallback) == 4,
              "PullDown backfill callback uses the original x86 ABI");

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

class PullDown : public GraphicWin {
 public:
  // homed from 005f9100.cpp
  void UNK4();

 public:
  // homed from 005f9570.cpp
  void on_right_up(int x, int y);

 public:
  // homed from 005f9460.cpp
  void on_right_down(int x, int y);

 public:
  void hide();
  void UNK1();
  void on_mouse_move(int x, int y);
  void mouse_move(int x, int y);
  void pulldown_update(int old_selected, int new_selected);
  int UNK2(int a);
  int UNK3(int a, int b, int c);
  int UNK5();
  int UNK6();
  PullDown();
  void init(Menu *menu = 0);
  ~PullDown();
  int show(int x, int y, int (*poll)());

  int hide_item(int id);
  int show_item(int id);
  int disable_item(int id);
  int enable_item(int id);
  int check_item(int id);
  int uncheck_item(int id);
  int add_item(int id, char *text);
  int add_separator(int id);
  int rename_item(int id, char *text);
  int get_selected();
  // ReportWin::right_menu performs the original signed upper-bound check;
  // this accessor intentionally does not add a lower-bound check.
  int item_id(int index) const { return items_[index].id; }
  // get_selected returns a slot, not its action ID. Cancellation must not
  // index before items_: the old status-menu seed confused these two values.
  int get_selected_id() {
    int index = get_selected();
    return index >= 0 && index < 64 ? items_[index].id : -1;
  }
  int id_to_index(int id);

  void on_mouse_leave(int a1, int a2);

  // Resolves the widest item into width_. Only add_item (0x005F8BB6) and
  // rename_item (0x005F9028) reach it in the whole image, so it stays beside
  // them rather than in the public surface.
  int compute_length(char *text);

  // Menu's item-id probe walks items_ directly (0x005FB930 reads
  // items_[index].id at 0xa20 + index * 0x14 through the entry's PullDown),
  // and a data member has no accessor to grant it - so Menu is a friend, the
  // same grant GraphicWin gives Scroll and BaseButton. No offset moves.
  friend class Menu;

 private:
  uint32_t field_A14_;  // 0x0a14
  PullDownItem items_[64];
  uint32_t field_F18_;
  PullDownPoll poll_;  // 0xF1C, assigned by show before entering the modal loop
  int item_count_;
  int width_;
  int selected_;
  int visible_count_;
  Menu *menu_;
  uint8_t dirty_;
  uint8_t dirty_padding_[3];
  PullDownFillCallback field_F38_;
  PullDownBackfillCallback field_F3C_;
};

static_assert(sizeof(PullDown) == 0xF40,
              "PullDown layout must match the legacy ABI");

// Item text allocations come from the executable's CRT, so the destructor
// must release them through its free rather than this module's. Tests
// outside the hybrid process rebind this.

// Defaults the destructor copies into the two trailing fields; the game keeps
// them at 0x009B7B58/0x009B7B5C. Tests outside the hybrid process rebind
// them at locally mapped storage.
extern PullDownFillCallback PullDownFieldF38Default;
extern PullDownBackfillCallback PullDownFieldF3CDefault;

// The font every item string is measured with. Zero until the first PullDown
// needs it, then resolved once from `FontDefault` and cached - the same lazy
// read `PullDown::init` (0x005F8920) and `PullDown::on_redraw` (0x005F98B0)
// perform.
extern Font *PullDownDefaultFont;
extern int PullDownBorderPadding;
extern int PullDownTextPadding;
extern int PullDownDefaultTextColor;
extern int PullDownDefaultTextShadow;
extern int PullDownDefaultSelectedTextColor;
extern int PullDownDefaultSelectedTextShadow;
extern int PullDownDefaultDisabledTextColor;
extern int PullDownDefaultDisabledTextShadow;
extern int PullDownDefaultBackground;




// The image inlines this scan at every hide_item/show_item/disable_item/
// enable_item/check_item/uncheck_item call site rather than emitting a
// helper call; defined here, at the end of the header, so it can see
// PullDownItem, and inline so pulldown.cpp reproduces that.
//
// Returns an INDEX, not a pointer: the image recomputes the item's address
// from `index` at each field access (lea eax,[eax+eax*4]; lea ...+0xa24)
// rather than caching a pointer across the whole body.
inline int find_item(PullDownItem *items, int id) {
    int index = -1;
    for (int i = 0; i < 64; ++i) {
        if (items[i].id == -1) {
            break;
        }
        if (items[i].id == id) {
            index = i;
            break;
        }
    }
    return index;
}
