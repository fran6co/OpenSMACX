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
  *     by blink at 0x0048BC20 and cleared by close at 0x0048BC50.
  *     CORRECTED 2026-08-25: this passage used to end "Neither class declares
  *     a virtual function, so VC6 emits no vtordisp of its own and mapwin.h
  *     has to declare those four bytes itself". win.h declares its 88 slots
  *     now, so VC6 emits the vtordisp - but only for the MOST-DERIVED object.
  *     Inside a PlanWin the MapWin base is not most-derived, gets no
  *     vtordisp, and stops at the image's 0x21A68; PlanWin's own fields
  *     therefore start there, which is why field_21A68_ below is named and
  *     derived_tail_ gives four bytes back. sizeof was blind to the whole
  *     exchange - `clear_lines` wrote 0x21FF4 against the image's 0x21FF8.
  *
  * PlanWin's own fields are still carved out of the storage between the base
  * and 0x22050 rather than appended: appending would move the virtual base
  * and break every offset in the class.
  */
class PlanWin : public MapWin {
 public:
  // 0x0048AF30, a pending_bodies forwarder.
  void on_redraw();

 public:
  // ??0PlanWin@@QAE@H@Z, now spelled as the bare real constructor it is.
  // ATTEMPT #2, mirroring mapwin.h's: the first try built `PlanWin(int a1)`,
  // measured VC6 emitting BOTH the hidden most-derived flag at [ebp+0xc] and
  // a1 at [ebp+8] (a caller pushing 1 twice), and concluded "no constructor
  // here at all" - but the image's own `??__Eg_PLANWIN` (0x0048AE00) pushes
  // ONE 1, and the hidden flag IS that one push, arriving at [ebp+8] for a
  // bare `PlanWin()`. The catalogue's `H` is IDA's guess at that stack
  // dword (the image is stripped); the two-fact rule carries the name
  // mismatch. Zero callers for either spelling in this tree, so the
  // conversion cannot break a call site.
  PlanWin();
  ~PlanWin();
  void clear_lines();
  void close();
  void blink();
  void UNK1();

 private:
  // 0x21A68. THE FOUR BYTES MAPWIN NO LONGER NAMES. MapWin used to declare
  // them itself; VC6 emits that vtordisp now, but only while MapWin is the
  // MOST-DERIVED object. Inside PlanWin the base is not most-derived, so no
  // vtordisp is emitted there and PlanWin's own fields would start four bytes
  // early - measured: `clear_lines` wrote 0x21FF4 where the image writes
  // 0x21FF8. PlanWin's own vtordisp sits later, at 0x2204C, so sizeof is
  // unchanged by naming these.
  int32_t field_21A68_;  // 0x21A68
  int32_t field_21A6C_;  // 0x21A6C
  // PlanWin's own Buffer, constructed at 0x21A70 by the constructor
  // (`lea ecx, [esi + 0x21a70]` at 0x0048BD21). A real member now: its own
  // constructor/destructor are already recovered, and declaring it lets the
  // compiler emit that call the same way it emits every other base/member
  // subobject construction.
  Buffer buffer_;  // 0x21A70
  // No `derived_head_` here: 0x21FF8 - 0x21A70 is exactly
  // sizeof(Buffer) (0x588, pinned in buffer.h), so the array was
  // zero-length. VC6 rejects that - it is an MSVC extension - and a
  // zero-length member contributes nothing to the layout the
  // static_assert below pins.
  int32_t field_21FF8_;  // 0x21FF8
  // Four bytes shorter than the arithmetic suggests: naming the 0x21A68
  // filler above pushed every field back into its image position, so the
  // tail has to give those four back to keep the boundary at 0x22050.
  uint8_t derived_tail_[0x22050 - 0x21FFC - 4];  // 0x21FFC
};

static_assert(sizeof(PlanWin) == 0x22A64, "PlanWin layout must match terranx.exe");

