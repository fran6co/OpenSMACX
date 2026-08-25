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
#include "planwin.h"
#include "mapwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "texture.h"
#include "vector_teardown.h"
#include <cstring>

// The dword immediately ahead of MapWin's virtual base. MapWin used to
// declare it as a member; VC6 owns those four bytes as MapWin's vtordisp now
// that MapWin's methods override Win's virtuals, so it has no name here any
// more - and giving PlanWin its own member at 0x21A68 measured +4 on sizeof,
// which proves the dword still belongs to the MapWin subobject. This reaches
// it the same way `virtual_base_of` reaches the base: through the parameter,
// so no body has to pun its own `this`.
static int32_t &pre_vbase_dword(void *self) {
    return *reinterpret_cast<int32_t *>(
        reinterpret_cast<uint8_t *>(self) + 0x21A68);
}


// The vbtable PlanWin stores at its own front when it is the one building
// the embedded GraphicWin, and the two hand-maintained "vtable" pointers
// `GraphicWin::construct`'s own idiom writes when GraphicWin is directly
// the most-derived object (see graphicwin.cpp) - PlanWin has to repeat the
// latter pair on its own embedded GraphicWin, because nothing in this chain
// declares a single `virtual` and so VC6 never refreshes them on its own.
static void *const g_0066d414 = reinterpret_cast<void *>(0x0066D414);
static void *const g_0066d2ac = reinterpret_cast<void *>(0x0066D2AC);
static void *const g_0066d2a4 = reinterpret_cast<void *>(0x0066D2A4);

/*
Purpose: Build a plan window - attach its embedded GraphicWin, the MapWin
         base beneath it, and its own Buffer.
// ORIGINAL: 0x0048BCD0 ??0PlanWin@@QAE@H@Z 0x0048BCD0-0x0048BD7E;0x006581D0-0x00658205
// symbol    ?construct@PlanWin@@QAEXH@Z
// size      227 bytes
// prototype void (__thiscall ??0PlanWin@@QAE@H@Z)(PlanWin* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004626E0 0x005D4CF0 0x005D7210
Return Value: this

MEASURED: not spelled as a real constructor - see the note in `planwin.h`. A
plain method never gets VC6's own most-derived-flag treatment, so the single
`a1` here is read and branched on exactly as the image's `[ebp+8]` is, with
no second, compiler-inserted flag arriving alongside it.
*/
void PlanWin::construct(int input) {
    char *const self = reinterpret_cast<char *>(this);

    if (input) {
        *reinterpret_cast<void **>(self) = g_0066d414;
        new (reinterpret_cast<GraphicWin *>(self + 0x22050)) GraphicWin();
    }

    MapWin::construct(0);
    buffer_.Buffer::Buffer();

    char *const graphic_win = self + 0x22050;
    *reinterpret_cast<void **>(graphic_win) = g_0066d2ac;
    *reinterpret_cast<void **>(graphic_win + 0x444) = g_0066d2a4;
    *reinterpret_cast<int32_t *>(graphic_win - 4) = 0;
    field_21A6C_ = 0;
    // 0x21A68 BY OFFSET: MapWin no longer names it (VC6 owns those four
    // bytes as MapWin's vtordisp now that MapWin overrides Win's virtuals),
    // and giving PlanWin a member here measured +4 on sizeof - so the dword
    // still belongs to the MapWin subobject. Reaching it by offset is the
    // honest holding position; it is 3 raw accesses of debt, to be paid when
    // the MapWin/PlanWin pass models this boundary properly.
    pre_vbase_dword(this) = 0;
}

/*
Purpose: Tear down a plan window's own buffer and the MapWin fields it holds
         directly, without a separate call into MapWin's own destructor.
// ORIGINAL: 0x0048BD80 ??1PlanWin@@QAE@XZ 0x0048BD80-0x0048BED6;0x00658210-0x006582DE
// symbol    ??1PlanWin@@UAE@XZ
// size      548 bytes
// prototype void (__thiscall ??1PlanWin@@QAE@XZ)(PlanWin* this)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x005D7410 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

`this` IS NOT PlanWin's own front: `guarded_teardowns.cpp` already calls
`reinterpret_cast<PlanWin *>(TeardownObject00856DC0)->PlanWin::~PlanWin()`
with the address of the embedded GraphicWin virtual base
(front + 0x22050 - `TeardownObject00856DC0` minus `g_PLANWIN`'s own
0x00834D70 is exactly that), matching what this body's own arithmetic reads
throughout. That is the image's own convention, not a bug reproduced here -
`??__Fg_PLANWIN` calls this and then `GraphicWin::~GraphicWin()` on the same
address, in that order.

KNOWN DIVERGENCE, not chased further here: `PlanWin` publicly inherits
`MapWin`, and `MapWin` now has a genuine `~MapWin()` (needed so
`guarded_teardowns.cpp`'s own already-matching `->MapWin::~MapWin()` keeps
compiling), so this destructor's epilogue gets an EXTRA, compiler-inserted
call into `MapWin::~MapWin()` that the image does not make. The image tears
down MapWin's own fields here directly, calling `MapWin::clear` for the
"this window is live" flag - PlanWin's destructor never calls MapWin's own
separate destructor at 0x00420F90.
*/
PlanWin::~PlanWin() {
    char *const self = reinterpret_cast<char *>(this);
    reinterpret_cast<Buffer *>(self - 0x5e0)->~Buffer();

    char *const base = self - 0x5e4;
    reinterpret_cast<MapWin *>(base - 0x21a6c)->clear(0);

    VectorDtorIterator(base - 0x2b18, 0xabc, 4,
                        reinterpret_cast<const void *>(0x00625310));
    reinterpret_cast<Font *>(base - 0x2b4c)->~Font();
    reinterpret_cast<Font *>(base - 0x2b78)->~Font();
    reinterpret_cast<Font *>(base - 0x2ba4)->~Font();
    reinterpret_cast<Buffer *>(base - 0x3134)->~Buffer();
    reinterpret_cast<Buffer *>(base - 0x36bc)->~Buffer();
    reinterpret_cast<Buffer *>(base - 0x3c44)->~Buffer();
    reinterpret_cast<TextureStore *>(base - 0x3f60)->~TextureStore();
    VectorDtorIterator(base - 0x210e0, 0x260, 0xc4,
                        reinterpret_cast<const void *>(0x006252B0));
    VectorDtorIterator(base - 0x21a60, 0x260, 4,
                        reinterpret_cast<const void *>(0x006252B0));
}

/*
Purpose: Clear the plan window's line count.
// ORIGINAL: 0x0048AF20 ?clear_lines@PlanWin@@QAEXXZ 0x0048AF20-0x0048AF2B BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?clear_lines@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PlanWin::clear_lines() {
    field_21FF8_ = 0;
}


/*
Purpose: Close the plan window by dropping its blink state and running the
         close it inherits from MapWin.
// ORIGINAL: 0x0048BC50 ?close@PlanWin@@QAEXXZ 0x0048BC50-0x0048BC5F BYTE_EXACT
// size      15 bytes
// prototype void (__thiscall ?close@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete

The original clears the field and tail-jumps into MapWin::close with `this`
untouched, which is a plain base-class call: the MapWin subobject opens a
PlanWin at offset 0 - and it is spelled as one now. The claim that stood
here, that inheritance was impossible because MapWin held its virtual base
as a member, went with the member: VC6 places a virtual base where the
vbtable names it, so PlanWin reaches GraphicWin at its own 0x22050.
*/
void PlanWin::close() {
    // WRITTEN INLINE, not through `pre_vbase_dword`. The image emits
    // `mov [ecx + 0x21a68], 0` then TAIL-JUMPS to MapWin::close; routing the
    // store through a helper call blocks that tail call and costs the claim.
    // The pun of `this` is the price of the tail jump, and it is measured.
    field_21A68_ = 0;
    MapWin::close();
}


namespace {

// blink dispatches through the virtual base's own vtable rather than a C++
// virtual call, so no vtable this toolchain lays out can disagree with the
// original's. Slot 0x30 is read from whatever object is actually there.
typedef void (OriginalObject::*func_base_vtable_slot)();

static const size_t PlanWinBlinkSlot = 0x30;

// Both methods reach the virtual base the way the original does - through the
// vbtable rather than through the member - so they stay correct for any class
// that inherits this one and moves the base.
uint8_t *virtual_base_of(void *self) {
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    return reinterpret_cast<uint8_t *>(self) + vbtable[1];
}

}  // namespace

/*
Purpose: Toggle the plan window's blink phase and redraw, when blinking is on.
// ORIGINAL: 0x0048BC20 ?blink@PlanWin@@QAEXXZ 0x0048BC20-0x0048BC4B
// size      43 bytes
// prototype void (__thiscall ?blink@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PlanWin::blink() {
    if (!pre_vbase_dword(this)) {
        return;
    }
    field_21A6C_ = (field_21A6C_ == 0) ? 1 : 0;
    uint8_t *const base = virtual_base_of(this);
    void **const vtable = *reinterpret_cast<void ***>(base);
    (ORIGINAL(base)->*original_method<func_base_vtable_slot>(reinterpret_cast<unsigned long>(vtable[PlanWinBlinkSlot / sizeof(void *)])))();
}


/*
Purpose: Restore the window's backing image from the plan window's own buffer.
// ORIGINAL: 0x0048B3C0 ?UNK1@PlanWin@@QAEXXZ 0x0048B3C0-0x0048B3EF
// size      47 bytes
// prototype void (__thiscall ?UNK1@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D95B0
Return Value: n/a
Status: Complete

The height is passed negated. That is the original's, not a transcription
slip: it loads the buffer's field at 0x84 and runs `neg edx` before pushing.
*/
void PlanWin::UNK1() {
    uint8_t *const base = virtual_base_of(this);
    Buffer *const window_buffer = reinterpret_cast<Buffer *>(base + 0x444);
    int32_t width;
    int32_t height;
    std::memcpy(&width, base + 0x4C4, sizeof(width));
    std::memcpy(&height, base + 0x4C8, sizeof(height));
    window_buffer->copy(&buffer_, 0, 0, width,
                        -height);
}

