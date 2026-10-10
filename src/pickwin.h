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
  ~PickWin();
  void on_button_passover(int a1, int a2) override;
  // Small picker actions homed from their historical seeds. Button actions
  // dispatch through Win's virtual on_button_clicked interface.
  void close();                    // 0x00489B00, qualified GraphicWin close
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
  uint32_t field_623C_;  // 0x623C
  uint32_t field_6240_;  // 0x6240
  uint32_t field_6244_;  // 0x6244
  uint32_t field_6248_;  // 0x6248
  uint32_t field_624C_;  // 0x624C
  uint32_t field_6250_;  // 0x6250
  uint32_t field_6254_;  // 0x6254
  uint32_t field_6258_;  // 0x6258
  uint32_t field_625C_;  // 0x625C
  uint32_t field_6260_;  // 0x6260
  uint32_t field_6264_;  // 0x6264
  uint32_t field_6268_;  // 0x6268
  uint32_t field_626C_;  // 0x626C
  uint32_t field_6270_;  // 0x6270
  uint32_t field_6274_;  // 0x6274
  uint32_t field_6278_;  // 0x6278
  uint32_t field_627C_;  // 0x627C
  uint32_t field_6280_;  // 0x6280
  uint32_t field_6284_;  // 0x6284
  uint32_t field_6288_;  // 0x6288
  FlatButton flatButton1_;  // 0x628C
  FlatButton flatButton2_;  // 0x6DD8
  FlatButton flatButton3_;  // 0x7924
  Sprite sprites1_[3];  // 0x8470
  Sprite sprites2_[3];  // 0x84F4
  Sprite sprites3_[3];  // 0x8578
  Sprite sprites4_[3];  // 0x85FC
  Sprite sprite5_;  // 0x8680
  uint32_t field_86AC_;  // 0x86AC
  uint32_t field_86B0_;  // 0x86B0
  uint32_t field_86B4_;  // 0x86B4
  uint32_t field_86B8_;  // 0x86B8
  ListBox listBox_;  // 0x86BC; measured 0xB54
  uint32_t field_9210_;  // 0x9210
  uint32_t field_9214_;  // 0x9214
  uint32_t field_9218_;  // 0x9218
  uint32_t field_921C_;  // 0x921C
  uint32_t field_9220_;  // 0x9220
  uint32_t field_9224_;  // 0x9224
  uint32_t field_9228_;  // 0x9228
  uint32_t field_922C_;  // 0x922C
  uint32_t field_9230_;  // 0x9230
  uint32_t field_9234_;  // 0x9234
  uint32_t field_9238_;  // 0x9238
  uint32_t field_923C_;  // 0x923C
  uint32_t field_9240_;  // 0x9240
  uint32_t field_9244_;  // 0x9244
  uint32_t field_9248_;  // 0x9248
  uint32_t field_924C_;  // 0x924C
  uint32_t field_9250_;  // 0x9250
  uint32_t field_9254_;  // 0x9254
  uint32_t field_9258_;  // 0x9258
  uint32_t field_925C_;  // 0x925C
  uint32_t field_9260_;  // 0x9260
  uint32_t field_9264_;  // 0x9264
  uint32_t field_9268_;  // 0x9268
  uint32_t field_926C_;  // 0x926C
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
