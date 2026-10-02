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
#include "stringstruct.h"
extern uint32_t SpriteBoxDefaultField8;

// THE SPRITE LIST, the StringStruct sibling SpriteBox embeds at 0x24. Its
// own close (0x00611730, spritebox.cpp) stages the sibling pair
// 0x0066943C/0x00669438 - StringStruct's construction shape at this class's
// own table address - and its deleting destructor (0x00402BB0, still
// unrecovered) installs the same pair, which is the evidence that this is a
// distinct class and not plain StringStruct. StringList-shaped (0x30: the
// three list-own dwords at SpriteBox+0x40 are StringList's field_1C_/20_/
// 24_). No own virtual is declared yet: the table the compiler emits for it
// is StringStruct-shaped until the sibling's virtual surface is named
// (recorded semantic debt, unk-method).
class SpriteBoxList : public StringList {
 public:
  void close();
};

// GraphicWin and Dialog are virtual bases; Spot and the item list are
// owned members. Remaining offset-named fields and virtual operations still
// need interpretation; the size assertion alone does not prove the model.
class SpriteBox : public virtual GraphicWin, public virtual Dialog {
 public:
  // Original draw_item (0x00610ECA) and BasePop::on_redraw (0x00604C37)
  // call five cdecl arguments, clean 20 bytes, and discard the result.
  // Tutorial callbacks likewise have no receiver or result contract.
  typedef void (__cdecl *DrawCallback)(Sprite *, GraphicWin *, int, int, int);
  typedef int (__cdecl *ClickCallback)(int);
  void set_redraw_callback(DrawCallback callback) { redraw_callback_ = callback; }
  // 0x00611380, row redraw entry retained in leaf_recoveries.cpp.
  void on_redraw();
  // 0x00610C50, genuine row painter used by redraw and input handlers.
  void draw_item(int position);

 public:
  // 0x00611150, recovered in spritebox.cpp.
  void on_left_down(int x_coord, int y_coord);

 public:
  // 0x006112E0, recovered in spritebox.cpp.
  void on_left_double_click(int x_coord, int y_coord);

 public:
  // 0x00611060, a pending_bodies forwarder.
  void on_left_click(int a1, int a2);

 public:
  // 0x00611400, a pending_bodies forwarder.
  int on_key_down(int a);

 public:
  // 0x00610540, a pending_bodies forwarder.
  int attach(GraphicWin * a1, int a2, int a3, int a4);

 public:
  void on_right_down(int a, int b);
  void on_right_double_click(int a, int b);
  void on_left_up(int a, int b);
  void on_right_up(int a, int b);
  void on_right_click(int a, int b);
 public:
  void on_dialog_focus(int a1);
  void close();
  // 0x006108E0, still a pending_bodies forwarder: `pops()` (popup.cpp) calls
  // it BY NAME so it emits the image's `call rel32`.
  int sprite(Sprite *a1, char *a2, int a3);
  int init(RECT* group_id, int flags);
  int init(int file_name);
  int init(Heap *file_name);
  // 0x00610440: forward the five integer arguments to the virtual Dialog base.
  int init(int a1, int a2, int a3, int a4, int a5);
  // 0x00610510: image arguments are RECT* and Heap* (catalogue's first
  // parameter spelling is wrong); forward both unchanged to Dialog.
  int init(RECT *rect, Heap *heap);
  // 0x006104D0  ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z - the five-argument
  // overload, recovered and byte-exact. It was the only one of the four with
  // no declaration, which is why its body could not be integrated.
  int init(int a1, int a2, int a3, int a4, Heap *a5);
  // ??0SpriteBox@@QAE@H@Z at 0x0060FF00. The `int` is NOT a user parameter -
  // VC6 supplies its hidden most-derived flag for the real virtual bases.
  SpriteBox();
  ~SpriteBox();
  uint32_t id_to_pos(int id);
  // 0x00611640 ?pos_to_id@SpriteBox@@QAEHH@Z - the sibling walk to
  // id_to_pos, and the same shape Dialog::pos_to_id (0x00609B50) is: set
  // the list cursor from the head, step it `position` entries in either
  // direction, and return the entry the cursor landed on. Returns int (both
  // exit paths load EAX), unlike the catalogue's void spelling.
  int pos_to_id(int position);
  void on_mouse_move(int, int);
  void on_mouse_leave(int, int);
  // 0x00610970 ?calculate_dimensions@SpriteBox@@QAEHXZ, still FILE-only in
  // src/unrecovered/00610970.cpp. DECLARATION ONLY, added because Dialogs
  // ::calculate_dimensions (0x006132A0) tail-calls it for kind 8 - the access
  // costs nothing and the definition stays where it is.
  int calculate_dimensions();

 private:
  // 0x0 is this object's own vbtable pointer, EMITTED by the compiler.
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint8_t field_C_[0x4];  // 0xC
  // Constructed at complete-object +0x10 before list_ (0x0060FF64), then
  // destroyed at GraphicWin-relative -0x7C (0x00610252). Its lifetime is
  // compiler-owned, not placement construction in the constructor body.
  // RECOVERY_MEMBER: v2 SpriteBox spots_ Spot 0x10 0x0060FF00
  Spot spots_;  // 0x10
 public:
  // PUBLIC, like CheckBox::field_10_ for DipEdit::init: BasePop
  // ::calculate_dimensions stores the popup's two text-limits straight into
  // them (Dialogs+0x225c/0x2260) with no call in between, so the access has
  // to be spelled from outside the class. Access is compile-time only; the
  // static_assert below pins the layout.
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
 private:
  // THE EMBEDDED SPRITE LIST, carved 2026-09-01 from what was field_24_ ..
  // field_50_: exactly 0x30 = sizeof(StringList) - the two ABI words at its
  // front, the four list fields (head 0x2C, cursor 0x30, signed count 0x34,
  // position 0x38 - the walk `?id_to_pos@SpriteBox@@QAEIH@Z` proves at
  // 0x00611600), the allocator, StringList's own three dwords, and the
  // virtual base (vptr 0x4C + saved owner 0x50). The ctor's field_24_/
  // field_28_/field_4C_/field_50_ stores were StringList's construction
  // sequence by hand - the StringAllocationBase capture read the hand-off
  // global and cleared it - and the compiler emits all of it from this
  // declaration now. SpriteBoxList is the sibling whose own close stages
  // the 0x0066943C/0x00669438 pair (spritebox.cpp).
  // PUBLIC, not private: Dialogs (which derives SpriteBox) returns the
  // list's count for kind 8, exactly as it did through field_34_ before.
  // Access changes no offset.
 public:
  SpriteBoxList list_;  // 0x24
 private:
  uint32_t field_54_;  // 0x54
  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
 public:
  // PUBLIC: BasePop::calculate_dimensions stores the sprite redraw callback
  // (Dialogs+0x22a4) and its two config slots (0x22a8/0x22c0) straight into
  // these, with no call in between. Access is compile-time only.
  DrawCallback redraw_callback_;  // 0x64
  ClickCallback field_68_;  // original item-click callback at 0x68
 private:
  // GraphicWin callback receivers are +0x8C; native left handlers read
  // -0x20 and -0x1C respectively, identifying these complete-object slots.
  ClickCallback left_down_callback_;  // 0x6C
  ClickCallback left_up_callback_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
 public:
  // 0x00611301 loads this slot; caller cleanup follows its one argument,
  // and the returned int decides whether to post the Enter-key message.
  // RECOVERY_MEMBER: v1 SpriteBox double_click_callback_ 0x80 0x006112E0
  ClickCallback double_click_callback_;  // 0x80
 private:
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
static_assert(sizeof(SpriteBox) == 0xB98,
              "SpriteBox layout must match the original executable");


// ?close@SpriteBox@@QAEXXZ (0x00610280) - not recovered; forwarded in
// pending_bodies.cpp. The destructor and the three init overloads above all
// call it.
