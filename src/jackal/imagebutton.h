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
  // Constructor6252E8 installs670A94: slot90 selects625330.
  virtual void close();
  int init(Sprite *sprite, LPCSTR name, int id, int x, int y, int width, int height,
           Win *parent, int style_flag);
  // 625380 borrows the same Sprite as the explicit-size overload and reads
  // its dimensions at +18/+1C. A null sprite returns 3 without a store.
  int init(Sprite *sprite, LPCSTR name, int id, int x, int y,
           Win *parent, int style_flag);
  void set_sprite(Sprite *sprite) { sprite_ = sprite; }
  // Win slot 12; the constructor-installed table selects original 6253D0.
  virtual void on_redraw();

 private:
  // init 625340/625380 publish this borrowed pointer; paint 6253D0 reads
  // Sprite transparency/source width and calls Sprite::draw. close clears
  // it before BaseButton::close. The button never owns the Sprite lifetime.
  // RECOVERY_MEMBER: v3 ImageButton sprite_ Sprite* 0xAB8 0x00625380
  Sprite *sprite_;
};
