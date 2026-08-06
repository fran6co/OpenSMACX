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
class DLLEXPORT MessageWin : Sprite {
 public:
  MessageWin() { ; }
  ~MessageWin() { ; }
  void UNK1(int a1, int a2, int a3, int a4, int a5);
  void UNK4(int a1);

  // Storage the image proves is here: its own methods reach 0x68.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 2 named; it starts a member at 0x2C, which is where src/ ends.

  uint32_t field_2C_;  // 0x2C
  uint32_t field_30_;  // 0x30
  uint32_t field_34_;  // 0x34
  uint32_t field_38_;  // 0x38
  uint8_t font_[0x28];  // 0x3C
  uint8_t field_64_[0x4];  // 0x64

  // Storage the image proves is here: its own methods reach 0x3758.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_68_[0x36F0];
};

static_assert(sizeof(MessageWin) == 0x3758,
              "MessageWin layout must match the original executable");

void __fastcall message_win_unk1_redirect(MessageWin *self, void *, int a1, int a2, int a3, int a4, int a5);
void __fastcall message_win_unk4_redirect(MessageWin *self, void *, int a1);
