/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include "original_seam.h"
#include "dialog.h"
#include "graphicwin.h"
#include "listbox.h"
#include "radiobutton.h"
#include "checkbox.h"
#include "spritebox.h"
#include "editgroup.h"

// FIVE BASES, ONE GraphicWin AND ONE Dialog BETWEEN THEM - which is the thing
// a base can do and a member cannot: a member of a class with virtual bases
// carries its own copies. This header has described that layout for a long
// time and could not express it, because "the coupled edit that unblocks this
// class is on GraphicWin and Dialog, not here, and all five of these bases
// need it at once". That edit is done, and the five prefixes chain onto the
// image's own offsets exactly:
//
//   ListBox     at 0x0    prefix 0x44
//   RadioButton at 0x44   prefix 0x14
//   CheckBox    at 0x58   prefix 0x18
//   SpriteBox   at 0x70   prefix 0x88
//   EditGroup   at 0xF8   prefix 0x8C
//     -> vtordisp 0x184, GraphicWin 0x188, Dialog 0xBA0, sizeof 0xC94
//
// every one of which the vbtables above name independently.
class Dialogs : public ListBox,
                public RadioButton,
                public CheckBox,
                public SpriteBox,
                public EditGroup {
 public:
  // 0x00613180, a pending_bodies forwarder.
  void pass_dialog_focus();

 public:
  // 0x00613260, a pending_bodies forwarder.
  int on_scroll_delete(void * a1);

 public:
  // 0x00613220, a pending_bodies forwarder.
  void on_scroll_create();

 public:
  // 0x00612E00, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x00612B80, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x00612AE0, a pending_bodies forwarder.
  void on_mouse_leave(int a1, int a2);

 public:
  // 0x00612C20, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x00612D60, a pending_bodies forwarder.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x00612E80, a pending_bodies forwarder.
  void on_left_click(int a1, int a2);

 public:
  // 0x00612CC0, a pending_bodies forwarder.
  int on_key_down(int a1);

 public:
  // 0x006130E0, a pending_bodies forwarder.
  void on_dialog_focus(int a1);

 public:
  // 0x00612FE0, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  void close();
  // The recovered complete-object destructor body (0x00406910), modelled on
  // the primary this (the allocation base); the original enters at
  // this + 0x188 and the redirect recovers the base, as ListBox's does. The
  // uint32_t return preserves the EAX residue (ListBox::destroy's constant 0).
  uint32_t destroy();
  int item(char *text, int index);
  int get_num_items();
  void on_right_down(int a1, int a2);
  void on_right_double_click(int a1, int a2);
  void on_left_up(int a1, int a2);
  void on_right_up(int a1, int a2);
  void on_right_click(int a1, int a2);
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);
  void on_scrolling(int a1, int a2);
  void on_mousewheel(int a1);

 private:
  // FIVE base subobjects sharing TWO virtual bases, and this slab spells none
  // of them. ??0Dialogs@@QAE@H@Z (0x00612830) writes a vbtable pointer into
  // each of 0x0, 0x44, 0x58, 0x70 and 0xF8 (0x0061285F..0x00612880),
  // constructs GraphicWin on this+0x188 and Dialog on this+0xBA0 itself, and
  // only then runs ??0ListBox, ??0RadioButton, ??0CheckBox, ??0SpriteBox and
  // ??0EditGroup on those five offsets (0x006128BE, 0x006128D0, 0x006128DF,
  // 0x006128EE, 0x00612900), each with 0 pushed for the most-derived flag.
  // That flag is the H in `??0X@@QAE@H@Z` - not a real parameter but the
  // trailing int MSVC gives every constructor of a class with virtual bases,
  // and mangles: ??0Dialogs tests its own copy at 0x00612857 and jumps over
  // the two virtual-base constructions when it is 0, which is exactly what it
  // tells its five bases to do. GraphicWin and Dialog have no virtual bases
  // and correspondingly no such parameter (`??0GraphicWin@@QAE@XZ`). The five
  // vbtables read
  //
  //   0x00670A78 {0, 0x188, 0xBA0}  ListBox      at 0x0    prefix 0x44
  //   0x00670A6C {0, 0x144, 0xB5C}  RadioButton  at 0x44   prefix 0x14
  //   0x00670A60 {0, 0x130, 0xB48}  CheckBox     at 0x58   prefix 0x18
  //   0x00670A54 {0, 0x118, 0xB30}  SpriteBox    at 0x70   prefix 0x88
  //   0x00670A48 {0, 0x90,  0xAA8}  EditGroup    at 0xF8
  //
  // - each naming the SAME GraphicWin at 0x188 and the SAME Dialog at 0xBA0.
  // Sharing a virtual base is what a base does and what a member cannot: a
  // member of a class with virtual bases carries its own copies and is
  // constructed with the flag set to 1. So vbtable_pointer_, field_44_,
  // field_58_, field_70_ and field_F8_ are five base subobjects' vbtable
  // pointers rather than fields of this class, and the gaps between them are
  // those bases' own non-virtual prefixes. ~Dialogs (0x00406910) agrees: from
  // its this = B + 0x188 entry it tears them down in reverse construction
  // order at -0x90, -0x118, -0x130 and -0x144, which is 0xF8, 0x70, 0x58 and
  // 0x44, and finishes with ??1ListBox on -0x140 = B + 0x48 (0x00406A5F).
  // That last one is the edge this class cannot spell, stated a second way:
  // 0x48 is ListBoxDestructorAdjustment, so the ListBox being destroyed
  // starts at B + 0.
  //
  // The four bytes at 0x184 and at 0xB9C are vtordisp slots, not padding. The
  // constructor writes them at 0x00612938 and 0x00612947, and ~Dialogs
  // rewrites them at 0x00406977 and 0x00406989, in both cases as (vbase
  // offset - 0x188) and (vbase offset - 0xBA0), which is 0 in a most-derived
  // Dialogs. An earlier note here called the 0xB9C one "a Dialog four bytes
  // past its end"; the two writes withdraw that.
  //
  // sizeof IS pinned, which the same note also denied: the vbtable puts Dialog
  // at 0xBA0 and sizeof(Dialog) is 0xF4, so the object ends at 0xC94 and this
  // slab is exactly that long.
  //
  // WHAT THE UNSPELLED EDGE DOES NOT BLOCK. `Dialogs dialogs;` as a stack
  // local compiles today and is used by landed units - 0047e340.cpp:972,
  // 0047f5f0.cpp:978 and 004e0b00.cpp:929 all declare one directly. Three
  // functions in one batch read the paragraph below as "a Dialogs cannot be
  // declared" and deferred on it. It says something narrower: a body that only
  // CONSTRUCTS and DESTROYS a Dialogs is fine, because the slab is the right
  // size and its ctor/dtor are the real ones; a body that must reproduce the
  // vbtable-relative stores of the inheritance itself is the blocked case.
  //
  // WHY THE EDGE IS STILL NOT DECLARED. `class Dialogs : ListBox` cannot be
  // spelled at 0xC94 today. Measured with VC6 12.00.8168 against one frozen
  // copy of src/, sizeof(Dialogs) per spelling:
  //
  //   this slab, unchanged                                     0xC94  (want)
  //   `: ListBox`, dropping [0, 0x44)                          0x17A4
  //   `: ListBox`, dropping [0, 0x48)                          0x17A0
  //       ListBox is itself a slab holding its OWN GraphicWin and Dialog, so
  //       deriving from it duplicates 0xB08 bytes instead of sharing them.
  //   ListBox respelled `: virtual GraphicWin, virtual Dialog`,
  //   then `: ListBox` with both virtual bases dropped here    REFUSED
  //       C2243: the private virtual base is unreachable from Dialogs'
  //       implicit copy constructor. Private bases are this tree's style, so
  //       the two-level chain needs `public virtual` in ListBox to compile.
  //   the same with ListBox's virtual bases public             0xC90
  //       GraphicWin lands at 0x188 exactly (0xC90 - 0xA14 - 0xF4); the four
  //       missing bytes are the 0xB9C vtordisp. This also drops
  //       sizeof(ListBox) to 0xB4C, breaking its own pin and the pinned sizes
  //       of datalink.h and messagewin.h, which embed a ListBox by value.
  //   the same plus a 4-byte pad before the virtual bases      0xC94
  //       REJECTED. It buys the number by moving both virtual bases to 0x18C
  //       and 0xBA4, which the vbtable refutes - sizeof cannot see it, so no
  //       check in this tree would have caught it.
  //
  // What the missing bytes are is settled, not open. A VC6 probe of this exact
  // shape: with the bases modelling their vtable as an opaque dword the
  // derived measures 0xC90 (and the ListBox shape 0xB4C); give both bases a
  // real virtual function IN PLACE OF that dword and let the derived override
  // it, and the same shape measures 0xC94 and 0xB54 exactly, with the bases
  // still 0xA14 and 0xF4 - and with no override it falls back to 0xB4C. That
  // conditionality is the definition of a vtordisp. See src/listbox.h: the
  // coupled edit that unblocks this class is on GraphicWin and Dialog, not
  // here, and all five of these bases need it at once.
  //
  // Both methods below dispatch on the kind at 0x180 through a sixteen entry
  // jump table, and both take the Dialog's address from the object's own
  // vbtable rather than from where it sits here: this class is used as a
  // subobject, and the embedding object's vbtable names different offsets.
  // NO OWN DATA. Everything from 0 to 0x184 is the five base subobjects and
  // the last four bytes are GraphicWin's vtordisp - all of it the compiler's
  // now. What used to be `kind_` at 0x180 is EditGroup's own field at its
  // +0x88 (0x180 - 0xF8), which is why EditGroup's prefix is 0x8C where
  // SpriteBox's is 0x88.
};

static_assert(sizeof(Dialogs) == 0xC94,
              "Dialogs ends where its vbtable puts Dialog: 0xBA0 + 0xF4");

// Neither delegate target is recovered yet.



// The SpriteBox and ListBox handlers these forward to are not recovered.
typedef void (OriginalObject::*func_dialogs_fwd2)(int a1, int a2);
typedef void (OriginalObject::*func_dialogs_fwd1)(int a1);


// ~Dialogs tears down the EditGroup, SpriteBox and CheckBox members through
// still-original destructors; each is bound rebindably. The embedded
// RadioButton, ListBox, trailing Dialog and the GraphicWin virtual base are
// torn down through their recovered bodies directly. operator delete is used
// only by the scalar deleting destructor; func_operator_delete comes from
// dialog.h.
typedef void (OriginalObject::*func_dialogs_teardown)();
extern func_dialogs_teardown DialogsEditGroupDestructor;  // 0x00611A20
extern func_dialogs_teardown DialogsSpriteBoxDestructor;  // 0x00610120
extern func_dialogs_teardown DialogsCheckBoxDestructor;   // 0x0060E740

// Virtual tables ~Dialogs stages: three into the GraphicWin/Win virtual base,
// three into the embedded RadioButton, every slot located through the
// subobject's OWN vbtable at run time. All six are written but never
// dispatched - the dispatches happen inside the recovered close/destroy
// bodies - so they are fixed constants like Scroll's.
extern const uint32_t DialogsVbaseGraphicWinVtable;  // 0x00669BE8
extern const uint32_t DialogsVbaseBufferVtable;      // 0x00669BE0
extern const uint32_t DialogsVbaseWinVtable;         // 0x00669BD4
extern const uint32_t DialogsRadioPrimaryVtable;     // 0x00669A6C
extern const uint32_t DialogsRadioBufferVtable;      // 0x00669A64
extern const uint32_t DialogsRadioWinVtable;         // 0x00669A58

// ~Dialogs and ??_GDialogs are entered at B + 0x188 (the GraphicWin
// co-located subobject); both redirects recover the allocation base B first.
static const size_t DialogsDestructorAdjustment = 0x188;

uint32_t __fastcall dialogs_destructor_redirect(void *adjusted, void *);
void *__fastcall dialogs_scalar_dtor_redirect(void *adjusted, void *,
                                              unsigned int mode);
