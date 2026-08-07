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
#include "dialog.h"   // func_dialog_close
#include "graphicwin.h"


 /*
  * RadioButton class
  *
  * Virtually derives from GraphicWin, so the base is held as a member at the
  * offset MSVC's vbtable names rather than written as `: virtual GraphicWin` -
  * the Itanium ABI this toolchain follows would place it after the derived
  * object instead. The vbtable at 0x00670590 reads {0, 0x18, 0xA30}, and
  * close() resolves both of its calls through exactly those two entries.
  *
  * A Dialog follows the base at 0xA30, four bytes past where the base ends.
  * Nothing pins this sizeof: no method touches past the Dialog, but unlike
  * MapWin and Console there is no global instance to bound the object from
  * above, and "nothing touches it" would not notice a trailing field that is
  * never used.
  */
class DLLEXPORT RadioButton {
 public:
  void on_mouse_leave(int a1, int a2);
  void on_dialog_focus(int a1);
  RadioButton() { ; }
  ~RadioButton() { ; }
  void close();

 private:
  uint32_t vbtable_pointer_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  // The vbtable puts the base at 0x18; the declared fields reach
  // 0x14, so 4 bytes sit between them.
  uint8_t gap_14_[0x18 - 0x14];
  GraphicWin virtual_base_;
  uint8_t gap_A2C_[4];
  Dialog dialog_;
};

// RadioButton's Dialog::close is not recovered yet.
extern func_dialog_close RadioButtonOriginalDialogClose;
extern uint32_t *RadioButtonDefault1;
extern uint32_t *RadioButtonDefault2;

void __fastcall radio_button_close_redirect(RadioButton *self, void *);
