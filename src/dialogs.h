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
#include "dialog.h"
#include "graphicwin.h"

class DLLEXPORT Dialogs {
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
  void on_scrolled(int a1, int a2);
  void on_scrolling(int a1, int a2);
  void on_mousewheel(int a1);

 private:
  // Virtually derives from GraphicWin - the vbtable reads {0, 0x188, 0xBA0} -
  // so the base is a member at 0x188 with a Dialog four bytes past its end.
  // Both methods below dispatch on the kind at 0x180 through a sixteen entry
  // jump table, and both take the Dialog's address from the object's own
  // vbtable rather than from where it sits here: this class is used as a
  // subobject, and the embedding object's vbtable names different offsets.
  //
  // Other dialog widgets sit between 0x4 and 0x180 at offsets that are not
  // established, so everything but the two fields these methods read is
  // opaque, and nothing pins this sizeof.
  uint32_t vbtable_pointer_;
  uint8_t unmapped_04_[0xA4 - 0x04];
  int32_t field_A4_;
  uint8_t unmapped_A8_[0x180 - 0xA8];
  int32_t kind_;
  uint8_t unmapped_184_[0x188 - 0x184];
  GraphicWin virtual_base_;
  uint8_t gap_B9C_[4];
  Dialog dialog_;
};

// Neither delegate target is recovered yet.
typedef int (__thiscall func_dialog_item)(Dialog *, char *, int);
typedef int (__thiscall func_list_box_item)(void *, char *, int);
extern func_dialog_item *DialogOriginalItem;
extern func_list_box_item *ListBoxOriginalItem;

int __fastcall dialogs_item_redirect(Dialogs *self, void *, char *text, int index);
int __fastcall dialogs_get_num_items_redirect(Dialogs *self, void *);

void __fastcall dialogs_close_redirect(Dialogs *self, void *);

// The SpriteBox and ListBox handlers these forward to are not recovered.
typedef void (__thiscall func_dialogs_fwd2)(void *self, int a1, int a2);
typedef void (__thiscall func_dialogs_fwd1)(void *self, int a1);
extern func_dialogs_fwd2 *DialogsSpriteBoxOnRightDown;
extern func_dialogs_fwd2 *DialogsSpriteBoxOnRightDoubleClick;
extern func_dialogs_fwd2 *DialogsSpriteBoxOnLeftUp;
extern func_dialogs_fwd2 *DialogsSpriteBoxOnRightUp;
extern func_dialogs_fwd2 *DialogsSpriteBoxOnRightClick;
extern func_dialogs_fwd2 *DialogsListBoxOnScrolling;
extern func_dialogs_fwd1 *DialogsListBoxOnMousewheel;

void __fastcall dialogs_on_right_down_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_right_double_click_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_left_up_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_right_up_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_right_click_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_scrolled_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_scrolling_redirect(Dialogs *self, void *, int a1, int a2);
void __fastcall dialogs_on_mousewheel_redirect(Dialogs *self, void *, int a1);

// ~Dialogs tears down the EditGroup, SpriteBox and CheckBox members through
// still-original destructors; each is bound rebindably. The embedded
// RadioButton, ListBox, trailing Dialog and the GraphicWin virtual base are
// torn down through their recovered bodies directly. operator delete is used
// only by the scalar deleting destructor; func_operator_delete comes from
// dialog.h.
typedef void (__thiscall func_dialogs_teardown)(void *self);
extern func_dialogs_teardown *DialogsEditGroupDestructor;  // 0x00611A20
extern func_dialogs_teardown *DialogsSpriteBoxDestructor;  // 0x00610120
extern func_dialogs_teardown *DialogsCheckBoxDestructor;   // 0x0060E740
extern func_operator_delete *DialogsOperatorDelete;        // 0x0064557F

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
constexpr size_t DialogsDestructorAdjustment = 0x188;

uint32_t __fastcall dialogs_destructor_redirect(void *adjusted, void *);
void *__fastcall dialogs_scalar_dtor_redirect(void *adjusted, void *,
                                              unsigned int mode);
