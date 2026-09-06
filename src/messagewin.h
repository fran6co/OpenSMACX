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
#include "sprite.h"
#include "listbox.h"
#include "stringbox.h"

 /*
  * MessageWin class
  *
  * Derives from Sprite by ordinary single inheritance - its constructor calls
  * Sprite's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. Sprite is pinned here, so the base region
  * is exact; MessageWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class MessageWin : public Sprite {
 public:
  MessageWin();
  ~MessageWin();
  void UNK1(int a1, int a2, int a3, int a4, int a5);
  void UNK4(int a1);
  // 0x004718C0 ?on_selected@MessageWin@@QAEXHH@Z. MainInterface's interface
  // events are delivered here: on_iface_selected (0x0045D140) is
  // `mov ecx, 0x7f67f8 / call 0x4718c0`, the pop-up-menu handler for a right
  // click on the map.
  void on_selected(int code, int mode);
  // 0x00471530 ?add_message@MessageWin@@QAEXPADH@Z. Appends one line to
  // listBox_, de-duplicating against the current top line when the item id is
  // 0; the id is packed with 0x10000000 into Dialog::item.
  void add_message(char *text, int item_id);
  // 0x004713B0, declared for Popup::on_exec (0x00405910), which hands the
  // desktop window the text it built from the unit-history list. The first
  // parameter is base.cpp's BaseIDCurrentSelected read at the call site.
  void add_base(int base_id, char *text, int flag);

  // Storage the image proves is here: its own methods reach 0x68.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 2 named; it starts a member at 0x2C, which is where src/ ends.

  uint32_t field_2C_;  // 0x2C
  uint32_t field_30_;  // 0x30
  uint32_t field_34_;  // 0x34
  uint32_t field_38_;  // 0x38
  Font font_;  // 0x3C
  ListBox listBox_;  // 0x64

  // Storage the image proves is here: its own methods reach 0x3758.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  StringBox stringBox_;  // 0xBB8, IDB `stringBox`, size == sizeof(StringBox)
};

static_assert(sizeof(MessageWin) == 0x3758,
              "MessageWin layout must match the original executable");

// 0x007F67F8. The desktop's MessageWin - the same object game.cpp's system_init
// builds through messagewin_init. MainInterface's interface-event handlers reach
// it by this address (`mov ecx, 0x7f67f8` at on_iface_selected), and it is a real
// object with a real constructor in the image, so it is declared as one.
extern MessageWin InterfaceMessageWin;

