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

 /*
  * SpriteBox class
  *
  * Layout not established; its methods reach as far as 0x450, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class DLLEXPORT SpriteBox {
 public:
  SpriteBox() { ; }
  ~SpriteBox() { ; }
  uint32_t id_to_pos(int id);
  void on_mouse_move(int, int);
  void on_mouse_leave(int, int);

 private:
  uint8_t unmapped_[0x460];
};

uint32_t __fastcall sprite_box_id_to_pos_redirect(SpriteBox *self, void *, int id);
void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2);
void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2);
