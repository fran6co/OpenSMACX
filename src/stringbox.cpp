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
#include "stdafx.h"
#include "original_seam.h"
#include "stringbox.h"
#include "stringstruct.h"
#include <cstring>
#include "vtable_shim.h"

/*
Purpose: Construct a StringBox: run the GraphicWin base construction, build
         the Scroll at 0xA20 and the StringList at 0x2B70, install this
         class's own two virtual tables, and clear the window state.
// ORIGINAL: 0x00629110 ??0StringBox@@QAE@XZ 0x00629110-0x00629208;0x00663300-0x00663312 BYTE_EXACT
// LEVER: THE StringList IS A REAL, VIRTUALLY-DERIVED MEMBER. The image's
//        frame reserves TWO dwords below its EH record (`sub esp, 8`) and
//        stores `this` into one and zero into the other before the base call;
//        every spelling with the StringList held by layout reserved one. The
//        two-slot frame is what VC6 emits only when the constructed hierarchy
//        really derives virtually - measured against cl 12.00.8168 in
//        isolation (2026-08-29), and landed here by making StringStruct
//        `: public virtual StringAllocationBase` (stringstruct.h) with the
//        owner capture in StringAllocationBase's inline ctor and the field
//        zeroing in StringStruct's. The whole 0x2B70 block - vbtable, vbase
//        vtable, owner capture, both vtable stages, the five zeroes - is then
//        COMPILER-GENERATED, inlined in the image's exact store order, with
//        no call anywhere: the earlier hand-written body reproduced the block
//        but never the frame.
// LEVER: DERIVE FROM GraphicWin DIRECTLY, not through ConstructedGraphicWin.
//        The shim is an empty pass-through now that GraphicWin has a real
//        constructor, but its inlined implicit constructor stores the SHIM's
//        own two vftables between the base call and the Scroll member - a
//        pair the image does not have. Dropping the layer removed exactly
//        those two instructions; the funclet (cold span) is the image's own
//        `mov ecx, [this-slot] / jmp ??1StringBox` shape because ~StringBox
//        stays declared in the header and defined only in pending_bodies.cpp.
// TRIED: BYTE_EXACT with the StringList held by layout, and the wall was the
//        frame, not the block: the hand-written staging measured 46 of 47
//        instructions with only `sub esp, 8` against `push ecx` disagreeing.
// TRIED: a `static void ...(char *self, unsigned offset)` helper for the
//        StringList staging - prodpicker.cpp's shape. VC6 declines to inline
//        it and emits `push 0x2b70 / push esi / call`: 0.683 similar.
// size      266 bytes
// prototype void (__thiscall ??0StringBox@@QAE@XZ)(StringBox* this)
// callers   6   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4CF0 0x006051D0
Return Value: Instance pointer in EAX
Status: Complete
*/
StringBox::StringBox() {
    top_line_ = 0;
    list_override_ = 0;
    per_line_draw_ = 0;
    visible_rows_ = -1;
}

/*
Purpose: Add a string to the box - stage the text, index and a cleared flag
         into the string struct at 0x2B70, add it, and run the fixup pass when
         the struct reports it did not take.
// ORIGINAL: 0x00629710 ?add@StringBox@@QAEXPADHH@Z 0x00629710-0x00629749
// size      57 bytes
// prototype void (__thiscall ?add@StringBox@@QAEXPADHH@Z)(StringBox* this, int8*, int, int)
// callers   6   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100 0x00629490
Return Value: n/a
Status: Complete
*/
void StringBox::add(char *text, int index, int flag) {
    // ONE ADDRESS, TAKEN ONCE. The image computes `ecx = &field_2B70_` a
    // single time (`lea ecx, [esi + 0x2b70]`) and reaches every field below
    // it - the StringList's field_1C_, field_20_ and field_24_ and the
    // StringStruct::add call itself - off THAT register. Storing through
    // named members instead makes VC6 re-derive each one from `this`.
    char *const base = reinterpret_cast<char *>(&stringList_);
    std::memcpy(base + 0x1c, &text, sizeof(text));
    *reinterpret_cast<uint32_t *>(base + 0x20) = static_cast<uint32_t>(flag);
    *reinterpret_cast<uint32_t *>(base + 0x24) = 0;
    // CALLED BY NAME, same as Dialog::item at 0x00609990: a real
    // `StringStruct::add` reaches the image's `call rel32`, where the
    // function-pointer seam this file used to bind compiles
    // `call dword ptr [0]`.
    if (reinterpret_cast<StringStruct *>(base)->add(index) == 0) {
        StringBox::add_fixup();
    }
    // TRIED: the image emits one further `xor eax, eax` right before the
    // epilogue, on the fixup-taken path only - MISMATCH stays at 14/18
    // (0.971 similar) without it. Tried: an early-return guard clause
    // instead of the if-block (`if (result != 0) return;`); dropping the
    // `StringBox::` qualifier on the call. Neither changes the codegen, and
    // `add` is genuinely void (`...QAEX...`), so there is no return value
    // to re-materialise. Plateaued here.
}


/*
Purpose: Record the scroll position and repaint through virtual slot 62.
// ORIGINAL: 0x00629D20 ?on_scrolled@StringBox@@QAEXHH@Z 0x00629D20-0x00629D35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?on_scrolled@StringBox@@QAEXHH@Z)(StringBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00629D2C
Return Value: n/a
Status: Complete
*/
void StringBox::on_scrolled(int code, int pos) {
    top_line_ = pos;
    reinterpret_cast<VCall *>(this)->slot062();
}

/*
Purpose: Step the receiver back to the subobject ??_GStringBox@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00432830 ??_GStringBox@@WEEE@AEPAXI@Z 0x00432830-0x0043283B BYTE_EXACT
// symbol    ??_EStringBox@@WEEE@AEPAXI@Z
// CORRECTED from ??3StringBox@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00432770` into
//   ??_GStringBox@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
