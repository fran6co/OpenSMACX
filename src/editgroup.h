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

#include "dialog.h"
#include "graphicwin.h"

class EditBox;

// Original vbtable670A3C = {0,0x8C,0xAA4}; constructor611940 constructs
// GraphicWin at8C and Dialog atAA4. The ten controls, owned text buffers
// and limits occupy4..7B. The own prefix ends88; the compiler emits the
// GraphicWin vtordisp at88. Dialogs adds its own kind after that prefix.
// RECOVERY_BASE: v1 EditGroup GraphicWin 0x8C 0x00611940
// RECOVERY_BASE: v1 EditGroup Dialog 0xAA4 0x00611940
class EditGroup : public virtual GraphicWin, public virtual Dialog {
 public:
  // 0x006126C0, a pending_bodies forwarder.
  void pass_dialog_focus();

 public:
  // 0x00612450, a pending_bodies forwarder. One override serves both virtual
  // bases: Win slot12 through GraphicWin (table6708D4+0x30 -> 0x6127B0) and
  // Dialog slot2 (table6708C0 at complete+0xAA4 -> 0x612820). VC6 emits both
  // receiver thunks from this single declaration because Win's slot-12
  // virtual carries the same original name as Dialog's.
  virtual void on_redraw();

 public:
  // Original slot51 in table6708D4 selects adjustor6127C0 -> body612670.
  // A single real override handles both direct and virtual focus delivery.
  virtual void on_dialog_focus(int focus_id);

 public:
  // 0x00611AF0, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  // The int in the catalogue is the compiler's hidden most-derived flag.
  EditGroup();
  // Unfinished lifetime reconstruction at611A20. This legacy helper and
  // its raw receiver adjustment are not a substitute for a genuine C++
  // destructor. Complete/base destruction entries still need recovery.
  void destruct();
  // 0x00611A90, homed in editgroup.cpp; delete/free semantics remain there.
  void close();
  void set_text_limits(int limit);
  char *get_text(int index);
  void set_text(char *text, int index);
  // 0x00611F80, a pending_bodies forwarder. DipEdit::on_button_clicked drives
  // it twice on the EditGroup subobject BasePop carries at 0x22C8, once per
  // prompt field: (caption, buffer, character limit).
  int edit(char *a1, char *a2, int a3);
  // 0x006120D0, still FILE-only in src/unrecovered/006120d0.cpp. DECLARATION
  // ONLY, added because BasePop::start (0x00601BF0) calls it by name on the
  // EditGroup subobject Dialogs carries - the access costs nothing and the
  // definition stays where it is.
  int set_default(char *text, int index);
  // 0x00612150 ?calculate_dimensions@EditGroup@@QAEHXZ, still FILE-only in
  // src/unrecovered/00612150.cpp. DECLARATION ONLY, added because Dialogs
  // ::calculate_dimensions (0x006132A0) tail-calls it for kind 4 - the access
  // costs nothing and the definition stays where it is.
  int calculate_dimensions();

 private:
  // Original6119E1..6119F2 initializes these three distinct ten-entry arrays.
  // Attach611D63 constructs EditBox; close611A90 deletes controls but frees
  // the independently allocated strings (also published by edit611F80).
  // RECOVERY_MEMBER: v1 EditGroup boxes_ 0x4 0x00611940
  EditBox *boxes_[10];
  // RECOVERY_MEMBER: v1 EditGroup text_buffers_ 0x2C 0x00611940
  char *text_buffers_[10];
  // RECOVERY_MEMBER: v1 EditGroup limits_ 0x54 0x00611940
  uint32_t limits_[10];
  // calculate_dimensions612150 clears/measures these; constructor leaves
  // them untouched. The edit width includes twice Dialog's inner margin.
  // RECOVERY_MEMBER: v1 EditGroup label_width_ 0x7C 0x00612150
  int label_width_;
  // RECOVERY_MEMBER: v1 EditGroup edit_width_ 0x80 0x00612150
  int edit_width_;
  uint32_t field_84_;  // 0x84; live construction/close default
  // GraphicWin and Dialog are virtual bases, appended by the compiler.
};

static_assert(sizeof(EditGroup) == 0xB98,
              "EditGroup ends at original Dialog offset 0xAA4 plus 0xF4");
