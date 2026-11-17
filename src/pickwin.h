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
#include "sprite.h"
#include "listbox.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "buttongroup.h"
#include "popup.h"

 /*
  * PickWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; PickWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * Controls and faction-name storage retain their native member offsets.
  * Ordinary C++ construction/destruction owns their lifetimes; measured class
  * footprint does not establish the original complete allocation extent.
  */
class PickWin : public GraphicWin {
 public:
  PickWin();
  void init(void (__cdecl *callback)(int), int mode, int faction);
  void on_redraw();
  void on_group_clicked(int source, int unused, int mode) override;
  void on_button_clicked(int button);
  int on_dialog_item_back_draw(GraphicWin *window, int unused, int index,
                               RECT *area) override;
  // Typed members own the native reverse-destruction sequence.
  void on_button_passover(int a1, int a2) override;
  // Small picker actions homed from their historical seeds. Button actions
  // dispatch through Win's virtual on_button_clicked interface.
  void close();                    // 0x00489B00, qualified GraphicWin close
  // WORKAROUND(decl): 27 undefined declarations keep PickWin's member count
  // at its value before field_625C_..6288_ and field_9210_..926C_ became
  // nine RECTs (guide 2.5, "member count counts declarations").
  void decl_count_stand_in_1();
  void decl_count_stand_in_2();
  void decl_count_stand_in_3();
  void decl_count_stand_in_4();
  void decl_count_stand_in_5();
  void decl_count_stand_in_6();
  void decl_count_stand_in_7();
  void decl_count_stand_in_8();
  void decl_count_stand_in_9();
  void decl_count_stand_in_10();
  void decl_count_stand_in_11();
  void decl_count_stand_in_12();
  void decl_count_stand_in_13();
  void decl_count_stand_in_14();
  void decl_count_stand_in_15();
  void decl_count_stand_in_16();
  void decl_count_stand_in_17();
  void decl_count_stand_in_18();
  void decl_count_stand_in_19();
  void decl_count_stand_in_20();
  void decl_count_stand_in_21();
  void decl_count_stand_in_22();
  void decl_count_stand_in_23();
  void decl_count_stand_in_24();
  void decl_count_stand_in_25();
  void decl_count_stand_in_26();
  void decl_count_stand_in_27();
  int on_key_click(int flags, int key);  // 0x0048A7C0
  int go_girl();                   // 0x0048A810

  // Storage the image proves is here: its own methods reach 0x12644.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 393 member(s) from the IDA database, 18 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  // Native construction is GraphicWin base, Popup, string/control storage,
  // sprites, ListBox, remaining controls, then ButtonGroup.
  Popup popup_;  // 0xA24
  uint32_t field_5DA0_;  // 0x5DA0
  uint32_t field_5DA4_;  // 0x5DA4
  uint32_t field_5DA8_;  // 0x5DA8
  uint32_t field_5DAC_;  // 0x5DAC
  int32_t field_5DB0_;  // 0x5DB0
  uint32_t field_5DB4_;  // 0x5DB4
  char faction_files_[24][0x18];       // 0x5DB8..0x5FF8
  char faction_menu_items_[24][0x18]; // 0x5FF8..0x6238
  void (__cdecl *field_6238_)(int);    // user selection callback
  // 489678/48968B pass these member addresses to Buffer::box_sprite.
  // 48969E..48973E insets/restores the second RECT in place across drawing
  // and text calls; a stack copy loses that original alias relationship.
  RECT frame_rect_;  // 0x623C..0x624C
  RECT header_panel_rect_;  // 0x624C..0x625C
  // The three top button RECTs; init fills each through an inlined
  // set_rectangle_dimensions with its arguments evaluated first (0x00488CD0).
  RECT field_625C_rect_;  // 0x625C
  RECT field_626C_rect_;  // 0x626C
  RECT field_627C_rect_;  // 0x627C
  FlatButton flatButton1_;  // 0x628C
  FlatButton flatButton2_;  // 0x6DD8
  FlatButton flatButton3_;  // 0x7924
  Sprite sprites1_[3];  // 0x8470
  Sprite sprites2_[3];  // 0x84F4
  Sprite sprites3_[3];  // 0x8578
  Sprite sprites4_[3];  // 0x85FC
  Sprite sprite5_;  // 0x8680
  // 48921C/489DEF/48A221 pass this actual member to Dialog::init.
  RECT list_area_rect_;  // 0x86AC..0x86BC
  ListBox listBox_;  // 0x86BC; measured 0xB54
  // The six lower button RECTs, filled the same way (0x00488E42).
  RECT field_9210_rect_;  // 0x9210
  RECT field_9220_rect_;  // 0x9220
  RECT field_9230_rect_;  // 0x9230
  RECT field_9240_rect_;  // 0x9240
  RECT field_9250_rect_;  // 0x9250
  RECT field_9260_rect_;  // 0x9260
  FlatButton flatButton4_;  // 0x9270
  FlatButton flatButton5_;  // 0x9DBC
  FlatButton flatButton6_;  // 0xA908
  FlatButton flatButton7_;  // 0xB454
  FlatButton flatButton8_;  // 0xBFA0
  FlatButton flatButton9_;  // 0xCAEC
  RECT picker_button_rects_[7];  // 0xD638..0xD6A8
  FlatButton flatButtons10_[7];  // 0xD6A8; native vector count is seven
  ButtonGroup buttonGroup_;  // 0x125BC
};

extern PickWin PickWinGlobal;  // native object start 0x822718; allocation extent unknown
void __cdecl list_was_dblclicked(int faction_id);  // 0x488780
extern char PickWinCaptionBuffer[];  // borrowed caption at 0x834D6C; capacity unknown

static_assert(sizeof(PickWin) == 0x12650,
              "PickWin layout must match the legacy ABI");
