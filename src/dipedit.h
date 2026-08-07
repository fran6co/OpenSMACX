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
  * DipEdit class
  *
  * Layout not established; its methods reach as far as 0x4E00, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class DLLEXPORT DipEdit {
 public:
  void on_selected(int a1);
  // Siblings on_selected calls. Bodies unrecovered - 0x004DA990 (?read_check@
  // DipEdit@@QAEXXZ) and 0x004DADA0 (?do_check@DipEdit@@QAEXXZ) - so these are
  // declarations only, resolved at link time like the other unrecovered methods.
  void read_check();
  void do_check();
  DipEdit() { ; }
  ~DipEdit() { ; }
  void on_left_click(int, int);
  void on_redraw();

 private:
  uint8_t unmapped_[0x4E10];

  // Storage the image proves is here: its own methods reach 0x5240.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_4E10_[0x430];
};

void __fastcall dip_edit_on_left_click_redirect(DipEdit *self, void *, int a1, int a2);
void __fastcall dip_edit_on_redraw_redirect(DipEdit *self, void *);
