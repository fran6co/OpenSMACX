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
#include "graphicwin.h"
#include "dialog.h"
#include "spot.h"

 /*
  * SpriteBox class
  *
  * Layout not established; its methods reach as far as 0x450, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class SpriteBox : public virtual GraphicWin, public virtual Dialog {
 public:
  // 0x00611380, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x00611150, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x006112E0, a pending_bodies forwarder.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x00611060, a pending_bodies forwarder.
  void on_left_click(int a1, int a2);

 public:
  // 0x00611400, a pending_bodies forwarder.
  int on_key_down(int a1);

 public:
  // 0x00610540, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  void on_right_down(int a, int b);
  void on_right_double_click(int a, int b);
  void on_left_up(int a, int b);
  void on_right_up(int a, int b);
  void on_right_click(int a, int b);
 public:
  void on_dialog_focus(int a1);
  uint32_t close();
  // 0x006108E0, still a pending_bodies forwarder: `pops()` (popup.cpp) calls
  // it BY NAME so it emits the image's `call rel32`.
  int sprite(Sprite *a1, char *a2, int a3);
  int init(RECT* a1, int a2);
  int init(int a1);
  int init(Heap *a1);
  // 0x006104D0  ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z - the five-argument
  // overload, recovered and byte-exact. It was the only one of the four with
  // no declaration, which is why its body could not be integrated.
  int init(int a1, int a2, int a3, int a4, Heap *a5);
  // ??0SpriteBox@@QAE@H@Z at 0x0060FF00. The `int` is NOT a user parameter -
  // SpriteBox's layout is composed by hand (see the class comment above), so
  // VC6 cannot inject the most-derived flag the way it would for a real
  // `: virtual GraphicWin, virtual Dialog` declaration; this constructor
  // takes an explicit flag instead and mirrors what the image's own guard
  // does with it.
  SpriteBox(int a1);
  ~SpriteBox();
  uint32_t id_to_pos(int id);
  void on_mouse_move(int, int);
  void on_mouse_leave(int, int);

 private:
  // 0x0 is this object's own vbtable pointer, EMITTED by the compiler.
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint8_t field_C_[0x4];  // 0xC
  // Placement-new'd unconditionally by the constructor (`new (self+0x10)
  // Spot();`), raw storage rather than a real member so its construction
  // stays exactly where the disassembly puts it rather than moving into
  // the compiler's own implicit member-init prologue.
  uint8_t spot_storage_[0xC];  // 0x10, sizeof(Spot)
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  // The intrusive list `?id_to_pos@SpriteBox@@QAEIH@Z` walks, proved at
  // 0x00611600: `mov eax, [ecx + 0x2c]` (the head, tested for null first),
  // `mov [ecx + 0x30], eax` (the cursor), `mov esi, [ecx + 0x34]` compared
  // with `cmp esi, edx / jle` - so the count is SIGNED - and
  // `mov [ecx + 0x38], edx` (the position, which is also the return
  // residue). The IDB names all four field_2C..field_38 at four bytes each.
  uint32_t field_2C_;
  uint32_t field_30_;
  // 0x34. Dialogs returns this for kind 8 - at 0x70 + 0x34 == 0xA4, which that
  // header called `field_A4_`.
 protected:
  int32_t field_34_;
 private:
  uint32_t field_38_;
  uint32_t field_3C_;  // 0x3C
  uint8_t field_40_[0xC];  // 0x40
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  uint32_t field_6C_;  // 0x6C
  uint32_t field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  // 0x88 IS GRAPHICWIN'S VTORDISP, not a field. The compiler emits it because
  // this class overrides GraphicWin's on_dialog_focus and on_mouse_leave;
  // declaring it as well put the base at 0x90 where the image has it at 0x8C,
  // which `mov eax, [ecx - 0x8c]` in 0x006115E0 is what caught. A size
  // assertion could not: the four bytes total the same wherever they sit.

  // The constructor placement-news a GraphicWin at +0x8C and a Dialog at
  // +0xAA4 (`new (self+0x8c) GraphicWin(); new (self+0xaa4) Dialog();`),
  // the same two-virtual-base shape CheckBox/EditGroup/RadioButton use -
  // field_0_ above IS this object's own vbtable pointer, read at run time
  // by id_to_pos's siblings (`*reinterpret_cast<char**>(this)+8` reaches
  // Dialog through vtbl[2], matching `init`'s own existing bodies). Sizing
  // these as real GraphicWin/Dialog subobjects is what makes the previous
  // field_88_[0x3D8] wrong: SpriteBox has no size assertion, so nothing else
  // in this file depended on the old (too-small) extent.
  // GraphicWin and Dialog are VIRTUAL BASES, appended by the compiler.
};

// PINNED BEFORE CHANGING THE DECLARATION, so that replacing the hand-composed
// vbtable pointer and embedded subobjects with a real
// `: public virtual GraphicWin, public virtual Dialog` cannot move the layout
// without failing here. 0x8C of own data, then GraphicWin 0xA14, then the
// 4-byte vtordisp this header already carries as a gap, then Dialog 0xF4.
static_assert(sizeof(SpriteBox) == 0xB94,
              "SpriteBox layout must match the original executable");

uint32_t __fastcall sprite_box_id_to_pos_redirect(SpriteBox *self, void *, int id);
void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2);
void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2);

// ?close@SpriteBox@@QAEXXZ (0x00610280) - not recovered; forwarded in
// pending_bodies.cpp. The destructor and the three init overloads above all
// call it.
