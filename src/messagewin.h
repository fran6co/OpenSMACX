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
};

void __fastcall message_win_unk1_redirect(MessageWin *self, void *, int a1, int a2, int a3, int a4, int a5);
void __fastcall message_win_unk4_redirect(MessageWin *self, void *, int a1);
