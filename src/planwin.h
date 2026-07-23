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
#include "graphicwin.h"

 /*
  * PlanWin class
  *
  * Laid out the same way as MapWin and Console, and pinned on the same
  * evidence: the vbtable at 0x0066D414 reads {0, 0x22050}, GraphicWin is
  * pinned at 0xA14, and those sum to the 0x22A64 asserted below.
  * Independently g_PLANWIN's global slot bounds the object above at 0x22A80,
  * consistent with 0x1C to spare.
  *
  * The virtual base is a member rather than a virtual base for the ABI reason
  * described in mapwin.h, and fields must be carved out of derived_storage_
  * rather than appended.
  */
class DLLEXPORT PlanWin {
 public:
  PlanWin() { ; }
  ~PlanWin() { ; }
  void clear_lines();

 private:
  // field_21FF8_ is carved out of the derived storage rather than appended;
  // the static_assert below is what proves the carving kept the total at the
  // pinned 0x22A64.
  uint8_t derived_head_[0x21FF8];
  int32_t field_21FF8_;
  uint8_t derived_tail_[0x22050 - 0x21FFC];
  GraphicWin virtual_base_;
};

static_assert(sizeof(PlanWin) == 0x22A64, "PlanWin layout must match terranx.exe");

void __fastcall plan_win_clear_lines_redirect(PlanWin *self, void *);
