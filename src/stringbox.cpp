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

func_string_struct_add StringBoxStructAdd = original_method<func_string_struct_add>(0x00401100);

const uint32_t StringBoxPrimaryVtable = 0x0066ADC8;
const uint32_t StringBoxBufferVtable = 0x0066ADC0;

// The StringList at 0x2B70 and the eight-byte virtual base its vbtable names.
const uint32_t StringListVirtualBaseTable = 0x0066B0EC;
const uint32_t StringVirtualBaseTable = 0x006693AC;
const uint32_t StringStructTable = 0x006693A4;
const uint32_t StringStructBaseTable = 0x006693A0;
const uint32_t StringListTable = 0x006698C4;
const uint32_t StringListBaseTable = 0x006698C0;

/*
Purpose: Construct a StringBox: run the GraphicWin base construction, build
         the Scroll at 0xA20 and the StringList at 0x2B70, install this
         class's own two virtual tables, and clear the window state.
// ORIGINAL: 0x00629110 ??0StringBox@@QAE@XZ 0x00629110-0x00629208;0x00663300-0x00663312
// LEVER: PROMOTED out of src/recovered/units/00629110.cpp, where the header
//        said "measured MISMATCH", and out of the `StringBox() { ; }` inline
//        that stood in its place. Written as a REAL constructor, because the
//        image constructs its bases: it CALLS 0x005D4CF0 and 0x006051D0 and
//        carries the unwind frame that implies, and the first six instructions
//        - the whole `push -1 / push handler / fs:[0]` SEH prologue - agree.
// LEVER: WRITE THE StringList BLOCK OUT, do not factor it into a helper. The
//        0x30 bytes at 0x2B70 are a StringList and the image builds it inline.
//        Through a `static void ...(char *self, unsigned offset)` helper -
//        prodpicker.cpp's shape - VC6 declines to inline it and emits
//        `push 0x2b70 / push esi / call`: 0.683 similar. Written out in the
//        body it is 0.917, and every instruction from the block to the closing
//        vtable stores agrees.
// TRIED: BYTE_EXACT, and the wall is not in this file. call_diff says this
//        tree makes THREE calls where the image makes two:
//        ??0GraphicWin@@QAE@XZ, ??0Scroll@@QAE@XZ and, extra,
//        ?construct@GraphicWin@@QAEXXZ. The image's ONE `call 0x005D4CF0` is
//        GraphicWin's real constructor; this tree splits that into an empty
//        inline `GraphicWin() { ; }` (which the implicit base construction
//        calls) plus a separate `construct()` this body has to call by hand,
//        exactly as Scroll::Scroll() does. MEASURED: deleting the explicit
//        `GraphicWin::construct();` line leaves 46 instructions against 47 and
//        0.946 similar, with the entire remainder aligned - so the call really
//        is the whole structural gap. It is not shippable, because
//        `GraphicWin() { ; }` then leaves field_9CC_..field_A10_ unwritten. The
//        fix is `GraphicWin() { construct(); }` in src/graphicwin.h, which is
//        outside this batch's files and would move every GraphicWin-derived
//        constructor at once.
// TRIED: a `construct()` METHOD with `new (&scroll_) Scroll()` in place of
//        the real constructor - the shape that beats the SEH-frame ceiling
//        elsewhere. MEASURED WORSE: 0.627 against the real constructor's 0.917,
//        because the placement-new guard and the extra state transitions cost
//        more than the member ordering buys. The image's own `sub esp, 8` is
//        NOT a placement-new guard slot here.
// TRIED: the last two instructions of the frame, `sub esp, 8` against this
//        tree's `push ecx` and the missing `mov dword ptr [esp + 0xc], edi`.
//        The image reserves TWO dwords below its EH record and writes `this`
//        into one and zero into the other before calling the base; this tree
//        reserves one. It survives even with the extra call removed (the 46/47
//        measurement above), so it is downstream of the same base-class
//        spelling, not a body-order question.
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

    char *const self = reinterpret_cast<char *>(this);
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);

    // The 0x30 bytes at 0x2B70 are a StringList (src/stringstruct.h) and the
    // image builds it INLINE - there is no call to its constructor here. Same
    // block, same two-stage vtable install through the vbtable's second slot,
    // that prodpicker.cpp reproduces at its own three raw offsets. Written out
    // rather than factored into a helper because VC6 declines to inline the
    // helper and emits a `call` the image does not have.
    uint32_t *const pending_owner = reinterpret_cast<uint32_t *>(0x009B3374);
    field_2B74_ = StringListVirtualBaseTable;
    field_2B98_ = StringVirtualBaseTable;
    field_2B9C_ = *pending_owner;
    *pending_owner = 0;

    uint32_t vbtable = field_2B74_;
    field_2B70_ = StringStructTable;
    int adjust = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
    *reinterpret_cast<uint32_t *>(self + 0x2B74 + adjust) = StringStructBaseTable;

    vbtable = field_2B74_;
    field_2B78_ = 0;
    field_2B7C_ = 0;
    field_2B80_ = 0;
    field_2B84_ = 0;
    field_2B88_ = 0;
    field_2B70_ = StringListTable;
    adjust = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
    *reinterpret_cast<uint32_t *>(self + 0x2B74 + adjust) = StringListBaseTable;

    field_A1C_ = 0;
    field_2B6C_ = 0;
    field_A14_ = 0;
    object[0x000 / 4] = StringBoxPrimaryVtable;
    object[0x444 / 4] = StringBoxBufferVtable;
    field_A18_ = 0xFFFFFFFFU;
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
    // it - `field_2B8C_`, `field_2B90_`, `field_2B94_` and the
    // StringStruct::add call itself - off THAT register. Storing through
    // the named members instead makes VC6 re-derive each one from `this`.
    char *const base = reinterpret_cast<char *>(&field_2B70_);
    std::memcpy(base + 0x1c, &text, sizeof(text));
    *reinterpret_cast<uint32_t *>(base + 0x20) = static_cast<uint32_t>(flag);
    *reinterpret_cast<uint32_t *>(base + 0x24) = 0;
    // CALLED BY NAME, same as Dialog::item at 0x00609990: a real
    // `StringStruct::add` reaches the image's `call rel32`, where the
    // pointer-to-member `StringBoxStructAdd` compiles `call dword ptr [0]`.
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

void __fastcall string_box_add_redirect(StringBox *self, void *, char *text,
                                        int index, int flag) {
    self->add(text, index, flag);
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
void StringBox::on_scrolled(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0xa1c) = a2;
    reinterpret_cast<VCall *>(this)->slot062();
}
