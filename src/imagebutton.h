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
#include "basebutton.h"

 /*
  * ImageButton class
  *
  * DERIVES FROM BaseButton, and three independent facts say so. The hypothesis
  * layout gave it `uint8_t baseButton_[0xAB8]` instead, with a note objecting
  * that "deriving one from BaseButton and giving it a field at 0xab8 would
  * assert that sizeof(BaseButton) is exactly 0xab8, and nothing here evidences
  * that". It is evidenced now:
  *
  *   * sizeof(BaseButton) IS exactly 0xAB8, bisected against the real headers.
  *     (sizeof(FlatButton) is 0xB4C, so it is not that one.)
  *   * ??0ImageButton@@QAE@XZ opens `call 0x00606F30`, ??0BaseButton@@QAE@XZ.
  *   * ??1ImageButton@@QAE@XZ ends `jmp 0x00607040`, ??1BaseButton@@QAE@XZ -
  *     a TAIL call, which is what a derived destructor with nothing after the
  *     base teardown compiles to.
  *
  * The raw-storage spelling made the destructor trivial, so VC6 emitted no
  * vtable restores and no base call at all - a bare `ret` against the image's
  * three instructions.
  */
class ImageButton : public BaseButton {
 public:
  ImageButton();
  ~ImageButton();
  void close();
  int init(int a1, LPSTR name, int id, int x, int y, int width, int height,
           Win *parent, int style_flag);

 private:
  uint32_t field_AB8_;  // 0xAB8
};
