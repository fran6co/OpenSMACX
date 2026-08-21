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
#include "mapwin.h"
#include "buffer.h"

 /*
  * PlanWin class
  *
  * The base is MapWin, and the image states it rather than suggesting it:
  * ??0PlanWin@@QAE@H@Z saves the incoming `this` in esi at 0x0048BCF2 and
  * reaches ??0MapWin@@QAE@H@Z with `mov ecx, esi` / `call` at 0x0048BD1A and
  * 0x0048BD1C - no lea, no adjustment - so a MapWin subobject opens the
  * object at offset 0. GraphicWin arrives through MapWin, not from here: the
  * same constructor stores the vbtable 0x0066D414 into [this] at 0x0048BD04
  * and builds GraphicWin at `lea ecx, [esi + 0x22050]` (0x0048BCFE) under the
  * most-derived flag. That table's two dwords read {0x00000000, 0x00022050},
  * and 0x22050 + GraphicWin's pinned 0xA14 is the 0x22A64 asserted below;
  * g_PLANWIN's global slot bounds the object above at 0x22A80 independently.
  *
  * Two claims that stood here are withdrawn by measurement:
  *
  *   - "The virtual base is a member rather than a virtual base for the ABI
  *     reason described in mapwin.h." mapwin.h no longer says that, and the
  *     reason was never true for this build: CMAKE_CXX_COMPILER is
  *     tools/vc6-cl, and VC6 places a virtual base exactly where the vbtable
  *     names it. Probed with the same `cl` verify_member_offsets uses, this
  *     declaration puts field_21A6C_ at 0x21A6C, buffer_ at 0x21A70,
  *     field_21FF8_ at 0x21FF8 and sizeof at 0x22A64 - the image's numbers,
  *     with no GraphicWin member declared at all.
  *   - "That base ends at 0x21A68 ... which is why MapWin's pinned 0x21A6C is
  *     4 more than the data PlanWin inherits." Right about the image, and not
  *     expressible here. In the image those four bytes are MSVC's vtordisp,
  *     reserved immediately ahead of the virtual base, so they are MapWin's
  *     only while MapWin is most-derived; in a PlanWin the vtordisp moves to
  *     0x2204C (`mov [eax + esi - 4], edx` at 0x0048BD5D, eax = 0x22050) and
  *     0x21A68 becomes PlanWin's own first field, written at 0x0048BD67, read
  *     by blink at 0x0048BC20 and cleared by close at 0x0048BC50. Neither
  *     class declares a virtual function, so VC6 emits no vtordisp of its own
  *     and mapwin.h has to declare those four bytes itself to hold the
  *     virtual base at 0x21A6C. The MapWin subobject here therefore covers
  *     [0, 0x21A6C), one dword past the original's boundary, and the field at
  *     0x21A68 is the protected MapWin::field_21A68_ that mapwin.h documents.
  *     Every other offset, and sizeof, are the image's.
  *
  * PlanWin's own fields are still carved out of the storage between the base
  * and 0x22050 rather than appended: appending would move the virtual base
  * and break every offset in the class.
  */
class PlanWin : public MapWin {
 public:
  PlanWin() { ; }
  ~PlanWin() { ; }
  void clear_lines();
  void close();
  void blink();
  void UNK1();

 private:
  int32_t field_21A6C_;  // 0x21A6C
  // PlanWin's own Buffer, constructed at 0x21A70 by the constructor
  // (`lea ecx, [esi + 0x21a70]` at 0x0048BD21). It is held as raw storage
  // rather than a Buffer member because nothing recovered so far needs to
  // reach into it - UNK1 only ever passes its address.
  uint8_t buffer_[sizeof(Buffer)];  // 0x21A70
  // No `derived_head_` here: 0x21FF8 - 0x21A70 is exactly
  // sizeof(Buffer) (0x588, pinned in buffer.h), so the array was
  // zero-length. VC6 rejects that - it is an MSVC extension - and a
  // zero-length member contributes nothing to the layout the
  // static_assert below pins.
  int32_t field_21FF8_;  // 0x21FF8
  uint8_t derived_tail_[0x22050 - 0x21FFC];  // 0x21FFC
};

static_assert(sizeof(PlanWin) == 0x22A64, "PlanWin layout must match terranx.exe");

void __fastcall plan_win_clear_lines_redirect(PlanWin *self, void *);
void __fastcall plan_win_close_redirect(PlanWin *self, void *);
void __fastcall plan_win_blink_redirect(PlanWin *self, void *);
void __fastcall plan_win_unk1_redirect(PlanWin *self, void *);
