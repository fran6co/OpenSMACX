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

#include "menu.h"
#include "buffer.h"
#include "sprite.h"

 /*
  * AlphaMenu class
  *
  * The layout is derived, not guessed: the IDB member table starts a member
  * called `menu` at 0x0
  * whose size, 2916, is exactly the sizeof(Menu) this tree pins, with
  * `buffer` and `sprite1` following at 0xB64 and 0x10EC - the same
  * evidence route the typing campaign uses everywhere else. Console
  * pins the rest: its own carve found the three sprites this header
  * declares at +0x1118, and sizeof(Console) leaves this object exactly
  * 0x119C bytes - where Console's next field starts.
  */
class AlphaMenu : public Menu {
 public:
  AlphaMenu() { ; }
  // Member destruction follows the original 0x0051DC10 reverse-lifetime
  // trace: three Sprites, the leading Sprite, Buffer, then Menu.
  ~AlphaMenu();
  int init(Win *parent) override;
  void adjust() override;
  // Win slot 2 and slot 12, respectively.
  void hide();
  virtual void on_redraw();
  void show();
  // The image's slot 17 is the recovered two-argument on_mouse_move override
  // (0x00403550); its four-argument router remains inherited at slot 16.
  void on_mouse_move(int x, int y) override;
  int requested_height() override;
  void compute() override;
  void on_adjust_pulldown_pos(int *x, int *y) override;
  static int __cdecl fill_func(GraphicWin *target, int mode, RECT *clip);
  static int backfill_func(GraphicWin *target);
  // 0x00460DD0 - the catalogue pins the name ?check@MainMenu@@QAEXH@Z on it,
  // but the object it runs on is THIS one (??0Console builds a Menu-shaped
  // object at this+0x22A2C and then calls 0x00460DD0 on it), so the method
  // lives here rather than on a class the tree would have to invent.
  // Declared ahead of its own body landing because Console::prev_cursor and
  // menu_update drive it every turn. Body still unrecovered - a
  // pending_bodies forwarder.
  void check(int veh_id);
  int menu_init(); // original catalogue MainMenu::menu_init, Console-owned receiver

  Buffer buffer_;      // 0xB64, IDB `buffer`, size == sizeof(Buffer)

  Sprite sprite1_;     // 0x10EC, IDB `sprite1`, size == sizeof(Sprite)

  // The three-sprite run Console's constructor walks with the vector
  // constructor iterator at 0x1118 (0x2c each) - the image's ??0Console
  // builds these three as part of building this object.
  Sprite sprites_[3];  // 0x1118
};
