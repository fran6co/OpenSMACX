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
#include "buffer.h"

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
  *
  * PlanWin derives from MapWin: its constructor at 0x0048BCD0 calls
  * ??0MapWin@@QAE@H@Z with `this` unchanged, so the MapWin base subobject
  * opens the object at offset 0. That base ends at 0x21A68 - the offset the
  * constructor's own writes start at - and 0x21A68 is exactly where a
  * standalone MapWin puts the 4-byte vtordisp MSVC reserves ahead of a
  * virtual base, which is why MapWin's pinned 0x21A6C is 4 more than the data
  * PlanWin inherits. PlanWin's own vtordisp sits at 0x2204C, written by the
  * same constructor as `[vbase_offset + this - 4] = 0`.
  */
class DLLEXPORT PlanWin {
 public:
  PlanWin() { ; }
  ~PlanWin() { ; }
  void clear_lines();
  void close();
  void blink();
  void UNK1();

 private:
  // The inherited MapWin data, then PlanWin's own fields carved out of the
  // derived storage rather than appended; the static_assert below is what
  // proves the carving kept the total at the pinned 0x22A64.
  uint8_t map_win_base_[0x21A68];
  int32_t field_21A68_;
  int32_t field_21A6C_;
  // PlanWin's own Buffer, constructed at 0x21A70 by the constructor. It is
  // held as raw storage rather than a Buffer member because nothing recovered
  // so far needs to reach into it - UNK1 only ever passes its address.
  uint8_t buffer_[sizeof(Buffer)];
  // No `derived_head_` here: 0x21FF8 - 0x21A70 is exactly
  // sizeof(Buffer) (0x588, pinned in buffer.h), so the array was
  // zero-length. VC6 rejects that - it is an MSVC extension - and a
  // zero-length member contributes nothing to the layout the
  // static_assert below pins.
  int32_t field_21FF8_;
  uint8_t derived_tail_[0x22050 - 0x21FFC];
  GraphicWin virtual_base_;
};

static_assert(sizeof(PlanWin) == 0x22A64, "PlanWin layout must match terranx.exe");

void __fastcall plan_win_clear_lines_redirect(PlanWin *self, void *);
void __fastcall plan_win_close_redirect(PlanWin *self, void *);
void __fastcall plan_win_blink_redirect(PlanWin *self, void *);
void __fastcall plan_win_unk1_redirect(PlanWin *self, void *);
