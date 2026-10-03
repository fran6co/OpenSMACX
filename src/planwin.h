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
  *     recovery/environment/vc6-cl, and VC6 places a virtual base exactly
  *     where the vbtable names it. Probed with the same `cl` verify_member_offsets uses, this
  *     declaration puts field_21A6C_ at 0x21A6C, buffer_ at 0x21A70,
  *     line_count_ at 0x21FF8 and sizeof at 0x22A64 - the image's numbers,
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
  *     line_colors_ and line_records_ account for the exact tail. sizeof was blind to the whole
  *     exchange - `clear_lines` wrote 0x21FF4 against the image's 0x21FF8.
  *
  * PlanWin's own fields are still carved out of the storage between the base
  * and 0x22050 rather than appended: appending would move the virtual base
  * and break every offset in the class.
 */
struct PlanLineRecord {
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;
};
static_assert(sizeof(PlanLineRecord) == 0x10,
              "PlanWin line records contain four coordinates");

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
  void init(char *caption);
  void find_view(int x1, int y1, int x2, int y2,
                 int x3, int y3, int x4, int y4);
  void opticenter(int x1, int y1, int x2, int y2, int force_redraw);
  void monocenter(int x, int y, int force_redraw);
  void clear_lines();
  void add_line(int x1, int y1, int x2, int y2, int color);
  void add_line(int base_id1, int base_id2, int color);
  void draw_line_base(int base_id1, int base_id2, int color);
  void draw_line(int tile_x1, int tile_y1, int tile_x2, int tile_y2,
                 int color);
  void back_redraw();
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
  // The PlanWin line drawer has four records. The counter, color palette and
  // four-coordinate lines occupy the 0x50-byte tail before the vbase slot.
  // RECOVERY_MEMBER: v1 PlanWin line_count_ 0x21FF8 0x0048AE40
  int32_t line_count_;  // 0x21FF8
  // RECOVERY_MEMBER: v1 PlanWin line_colors_ 0x21FFC 0x0048AE40
  int32_t line_colors_[4];  // 0x21FFC, four 4-byte colors
  // RECOVERY_MEMBER: v1 PlanWin line_records_ 0x2200C 0x0048AE40
  PlanLineRecord line_records_[4];  // 0x2200C, four 16-byte line records
};

static_assert(sizeof(PlanWin) == 0x22A64, "PlanWin layout must match terranx.exe");

// The CRT constructs the complete PlanWin front at 0x00834D70; its
// GraphicWin virtual base then begins at 0x00856DC0.
extern PlanWin PlanWindow;

void __cdecl battle_routine(int base_id);
extern "C" void __cdecl sub_54dc90(int base_id);
