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
#include "original_seam.h"
#include "heap.h"

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
  void on_dialog_focus(int a1);
  void close();
  int init(RECT* a1, int a2);
  int init(int a1);
  int init(Heap *a1);
  // 0x006104D0  ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z - the five-argument
  // overload, recovered and byte-exact. It was the only one of the four with
  // no declaration, which is why its body could not be integrated.
  int init(int a1, int a2, int a3, int a4, Heap *a5);
  SpriteBox() { ; }
  ~SpriteBox() { ; }
  uint32_t id_to_pos(int id);
  void on_mouse_move(int, int);
  void on_mouse_leave(int, int);

 private:
  uint8_t unmapped_[0x2C];
  // The intrusive list `?id_to_pos@SpriteBox@@QAEIH@Z` walks, proved at
  // 0x00611600: `mov eax, [ecx + 0x2c]` (the head, tested for null first),
  // `mov [ecx + 0x30], eax` (the cursor), `mov esi, [ecx + 0x34]` compared
  // with `cmp esi, edx / jle` - so the count is SIGNED - and
  // `mov [ecx + 0x38], edx` (the position, which is also the return
  // residue). The IDB names all four field_2C..field_38 at four bytes each.
  uint32_t field_2C_;
  uint32_t field_30_;
  int32_t field_34_;
  uint32_t field_38_;
  uint8_t unmapped_3C_[0x424];

  // 0x2C + 0x10 + 0x424 == 0x460, so nothing moves. The body keeps its
  // offset casts: SpriteBox has no size assertion, so it is absent from
  // docs/recovery/verified-layouts.txt and the scaffolding a measured body
  // compiles in gives it an opaque shell where these names do not exist.
};

uint32_t __fastcall sprite_box_id_to_pos_redirect(SpriteBox *self, void *, int id);
void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2);
void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2);

// ?close@SpriteBox@@QAEXXZ - public, __thiscall, void(void) - is not
// recovered, and the three init overloads above open by calling it.
typedef void (OriginalObject::*func_sprite_box_close)();
extern func_sprite_box_close SpriteBoxClose;
