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
#include "font.h"
#include "heap.h"

#include <cstddef>

struct DialogEntry {
  uint32_t vtable;
  int id;
  void *payload;
  DialogEntry *next;
  DialogEntry *previous;
  uint32_t secondary_vtable;
  void *heap;
};

static_assert(sizeof(DialogEntry) == 0x1C,
              "DialogEntry layout must match the original executable");
static_assert(offsetof(DialogEntry, id) == 0x4,
              "DialogEntry ID offset must match the original executable");
static_assert(offsetof(DialogEntry, next) == 0xC,
              "DialogEntry next offset must match the original executable");

 /*
  * Dialog class
  */
class DLLEXPORT Dialog {
 public:
  Dialog() { ; }
  ~Dialog() { ; }

  int set_dialog_font(Font *font1, Font *font2, Font *font3);
  void set_dialog_text_color(int color1, int color2, int color3, int color4);
  void set_dialog_text_color2(int color1, int color2, int color3, int color4);
  void set_dialog_text_color3(int color1, int color2, int color3, int color4);
  int id_to_pos(int id);

 private:
  LPVOID vtable_;
  Heap heap_;
  Heap *heap_ptr_;
  uint32_t field_1C_;
  uint32_t field_20_;
  uint32_t field_24_;
  uint32_t field_28_;
  uint32_t field_2C_;
  uint32_t field_30_;
  uint32_t field_34_;
  uint32_t field_38_;
  uint32_t field_3C_;
  uint32_t field_40_;
  uint32_t field_44_;
  uint32_t field_48_;
  uint32_t field_4C_;
  uint32_t field_50_;
  uint32_t field_54_;
  uint32_t field_58_;
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint32_t field_6C_;
  Font *font1_;
  Font *font2_;
  Font *font3_;
  uint32_t text_color_a_;
  uint32_t text_color_2a_;
  uint32_t text_color_3a_;
  uint32_t text_color_b_;
  uint32_t text_color_2b_;
  uint32_t text_color_3b_;
  uint32_t text_color_c_;
  uint32_t text_color_2c_;
  uint32_t text_color_3c_;
  uint32_t text_color_d_;
  uint32_t text_color_2d_;
  uint32_t text_color_3d_;
  uint32_t field_AC_;
  uint32_t field_B0_;
  uint32_t field_B4_;
  uint32_t field_B8_;
  uint32_t field_BC_;
  uint32_t field_C0_;
  DialogEntry *entry_head_;
  DialogEntry *current_entry_;
  int entry_count_;
  int entry_position_;
  uint32_t field_D4_;
  uint32_t field_D8_;
  uint32_t field_DC_;
  uint32_t field_E0_;
  uint32_t field_E4_;
  uint32_t field_E8_;
  uint32_t state_flag_;
  uint32_t field_F0_;
};

static_assert(sizeof(Dialog) == 0xF4, "Dialog layout must match the original executable");

int __fastcall dialog_set_font_redirect(
    Dialog *self, void *, Font *font1, Font *font2, Font *font3);
void __fastcall dialog_set_text_color_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
void __fastcall dialog_set_text_color2_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
void __fastcall dialog_set_text_color3_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
int __fastcall dialog_id_to_pos_redirect(Dialog *self, void *, int id);
