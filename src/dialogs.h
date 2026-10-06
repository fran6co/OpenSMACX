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
  // 0x00613180: kinds1/2/8/16 use shared GraphicWin; kind4 uses EditGroup.
  void pass_dialog_focus();

 public:
  // Explicit Win slot-7/slot-6 overrides. Dialogs' kind/parent behavior
  // remains distinct from ListBox's AutoSound-recipient implementation.
  virtual int on_scroll_delete(Scroll *scroll);

 public:
  virtual int on_scroll_create();

 public:
  // 0x00612E00, kind-dispatched through typed widget base views in dialogs.cpp.
  void on_redraw();

 public:
  // 0x00612B80, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // HISTORICAL RECEIVER NOTE (2026-09-06): the archived event bodies read
  // kind at this-8 and dispatch through adjusted widget/Dialog views (the
  // observed offsets were -0x11C CheckBox, -0x148 ListBox, -0x8C SpriteBox,
  // -0x12C RadioButton and -4 EditGroup). Dialogs' dispatch thunks document
  // the separate +0x188 GraphicWin receiver adjustment. The source bodies
  // below now express the same case maps from a complete Dialogs object and
  // use its five declared direct bases; no executable or byte-offset view is
  // used. on_scroll_create and pass_dialog_focus now use the real shared
  // GraphicWin state and source-owned parent operations in dialogs.cpp.
  void on_mouse_leave(int a1, int a2);

 public:
  // 0x00612C20, kind-dispatched through typed base views in dialogs.cpp.
  void on_left_down(int a1, int a2);

 public:
  // 0x00612D60, kind-dispatched through typed base views in dialogs.cpp.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x00612E80, recovered in dialogs.cpp.
  void on_left_click(int a, int b) override;

 public:
  // 0x00612CC0, kind-dispatched through typed base views in dialogs.cpp.
  int on_key_down(int a) override;

 public:
  // 0x006130E0, kind-dispatched through typed base views in dialogs.cpp.
  void on_dialog_focus(int a1);

 public:
  // 0x00612FE0, kind-dispatched through typed base views in dialogs.cpp.
  int attach(GraphicWin *a1, int a2, int a3, int a4);
  // Kind-selecting attachment entry at 0x00612970.
  int attach(int kind, GraphicWin *parent, int x, int y, int flags);

 public:
  // ??0Dialogs@@QAE@H@Z (0x00612830), the bare real constructor - the H is
  // the hidden most-derived flag (the MapWin finding). The implicit
  // machinery owns all of it: the five vbtable pointers, the GraphicWin and
  // Dialog virtual-base construction, and the five base constructors each
  // run with a cleared flag - which is what the image's body spends its
  // 0x2B0 bytes on. kind_ is not initialized here; sizeof stays 0xC94.
  Dialogs();
  void close();  // hides ListBox::close; the image's body is one `ret`
  // Four recovered initialization overloads. Each initializes the same
  // shared virtual Dialog after closing the widget-specific state.
  int init(RECT *group_id, int flags);
  int init(int file_name);
  int init(int left, int top, int right, int bottom, int heap_size);
  int init(RECT *rect, Heap *heap);
  int init(int x, int y, int width, int height, Heap *heap);
  int init(Heap *heap);
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
  void on_right_click(int a, int b) override;
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);
  void on_scrolling(int a, int b);
  void on_mousewheel(int a) override;

 public:
  // Original612977 writes the kind at complete Dialogs+180;612A72 reads it.
  // The five base prefixes end here. Original constructor612830 leaves this
  // member untouched, then establishes vtordisp184/GraphicWin188/DialogBA0.
  // RECOVERY_MEMBER: v1 Dialogs kind_ 0x180 0x00612970
  int32_t kind_;

 private:
  // The archived dispatch bodies address these widget receivers by byte
  // offsets. The real multiple-inheritance graph already names them; use
  // C++ base conversions so the compiler follows the declared subobjects.
  ListBox &list_box_view() { return static_cast<ListBox &>(*this); }
  RadioButton &radio_button_view() { return static_cast<RadioButton &>(*this); }
  CheckBox &check_box_view() { return static_cast<CheckBox &>(*this); }
  SpriteBox &sprite_box_view() { return static_cast<SpriteBox &>(*this); }
  EditGroup &edit_group_view() { return static_cast<EditGroup &>(*this); }

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

void *__fastcall dialogs_scalar_dtor_redirect(void *adjusted, void *,
                                              unsigned int mode);
