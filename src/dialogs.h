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
//   EditGroup   at 0xF8   prefix 0x88
//   kind_       at 0x180; own member, not EditGroup data
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
  // 0x00613260, recovered in dialogs.cpp. The catalogued name spells the
  // parameter Scroll *; this tree spells it void * - the bytes are the same.
  int on_scroll_delete(void *scroll);

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
  //
  // WHY THE KIND-SWITCH EVENT FAMILY HERE IS STILL A FORWARDER (2026-09-06,
  //        dialogs finisher, measured on the artifacts): on_redraw
  //        (0x00612E00), on_mouse_leave (0x00612AE0), on_left_down
  //        (0x00612C20), on_left_double_click (0x00612D60), on_key_down
  //        (0x00612CC0), on_dialog_focus (0x006130E0) and on_scroll_create
  //        (0x00613220) all open on a receiver that is NOT the Dialogs
  //        object - they read a kind tag at `this - 8` and then call the
  //        widget's method at `this - 0x11C` (CheckBox), `this - 0x148`
  //        (ListBox), `this - 0x8C` (SpriteBox), `this - 0x12C`
  //        (RadioButton) and `this - 4` (EditGroup). Those are the widgets'
  //        DIALOG-PART views - the same adjusted receivers the vtordisp
  //        thunks enter on - and no member of this class names them, so the
  //        honest spelling needs a `char *self = (char *)this` walk, which
  //        class_debt counts and this tree holds at its measured ceiling.
  //        Naming the five views (a per-widget constant like
  //        StringStructCloseAdjustment, or a base-relative accessor) is the
  //        unlock; until then the forwarders stay.
  void on_mouse_leave(int a1, int a2);

 public:
  // 0x00612C20, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x00612D60, a pending_bodies forwarder.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x00612E80, recovered in dialogs.cpp.
  void on_left_click(int a, int b);

 public:
  // 0x00612CC0, a pending_bodies forwarder.
  int on_key_down(int a);

 public:
  // 0x006130E0, a pending_bodies forwarder.
  void on_dialog_focus(int a1);

 public:
  // 0x00612FE0, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  // ??0Dialogs@@QAE@H@Z (0x00612830), the bare real constructor - the H is
  // the hidden most-derived flag (the MapWin finding). The implicit
  // machinery owns all of it: the five vbtable pointers, the GraphicWin and
  // Dialog virtual-base construction, and the five base constructors each
  // run with a cleared flag - which is what the image's body spends its
  // 0x2B0 bytes on. kind_ is not initialized here; sizeof stays 0xC94.
  Dialogs();
  void close();  // hides ListBox::close; the image's body is one `ret`
  // The recovered complete-object destructor body (0x00406910), modelled on
  // the primary this (the allocation base); the original enters at
  // this + 0x188 and the redirect recovers the base, as ListBox's does. The
  // uint32_t return preserves the EAX residue (ListBox::destroy's constant 0).
  uint32_t destroy();
  int item(char *text, int index);
  int get_num_items();
  // 0x006132A0, homed in dialogs.cpp (BYTE_EXACT 24 of 24). It dispatches on
  // the Dialog kind at +0x180 to the same-named method of whichever base
  // subobject the kind selects, and zeroes the two embedded Dialog edge
  // fields when the kind is none of the five. BasePop::calculate_dimensions
  // (0x00603800) calls it; the five base declarations it tail-calls are in
  // their own headers, and their bodies are still FILE-only in
  // src/unrecovered/, so pending_bodies.cpp carries their forwarders.
  int calculate_dimensions();
  void on_right_down(int a, int b);
  void on_right_double_click(int a, int b);
  void on_left_up(int a, int b);
  void on_right_up(int a, int b);
  void on_right_click(int a, int b);
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);
  void on_scrolling(int a, int b);
  void on_mousewheel(int a);

 public:
  // Original612977 writes the kind at complete Dialogs+180;612A72 reads it.
  // The five base prefixes end here. Original constructor612830 leaves this
  // member untouched, then establishes vtordisp184/GraphicWin188/DialogBA0.
  // RECOVERY_MEMBER: v1 Dialogs kind_ 0x180 0x00612970
  int32_t kind_;

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

// ~Dialogs and ??_GDialogs are entered at B + 0x188 (the GraphicWin
// co-located subobject); both redirects recover the allocation base B first.
static const size_t DialogsDestructorAdjustment = 0x188;

uint32_t __fastcall dialogs_destructor_redirect(void *adjusted, void *);
void *__fastcall dialogs_scalar_dtor_redirect(void *adjusted, void *,
                                              unsigned int mode);
