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
#include "scroll.h"
#include "worldwin.h"
#include "dialog.h"
#include "sounddevice.h"
#include "spritebox.h"
#include "net_class.h"
#include "basepop.h"
#include "general.h"

// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
int ScrollDefaultThickness;  // 0x009B8DD4
int ScrollNonClientInit;  // 0x009B8E24
uint32_t ScrollCloseStaticDefaults;  // 0x00697020
uint32_t ScrollCloseDynamicDefaults;  // 0x009B8DE0

namespace {

typedef uint32_t (OriginalObject::*func_noarg_virtual)();



inline LONG long_from_bits(uint32_t bits) {
    LONG value;
    static_assert(sizeof(value) == sizeof(bits), "Scroll geometry requires 32-bit LONG");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

inline uint32_t read_bits(const void *object, size_t offset) {
    uint32_t value;
    memcpy(&value, static_cast<const uint8_t *>(object) + offset, sizeof(value));
    return value;
}

inline void write_bits(void *object, size_t offset, uint32_t value) {
    memcpy(static_cast<uint8_t *>(object) + offset, &value, sizeof(value));
}

inline void write_sprite(void *object, size_t offset, Sprite *sprite) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
                  "Scroll sprite pointers require the 32-bit legacy ABI");
    const uintptr_t bits = reinterpret_cast<uintptr_t>(sprite);
    *reinterpret_cast<volatile uintptr_t *>(
        static_cast<uint8_t *>(object) + offset) = bits;
}

inline uint32_t read_volatile_bits(const void *object, size_t offset) {
    return *reinterpret_cast<const volatile uint32_t *>(
        static_cast<const uint8_t *>(object) + offset);
}

uint32_t redraw_from_vtable(void *self, uint32_t vtable_bits) {
    const uint32_t redraw_bits = read_volatile_bits(
        reinterpret_cast<const void *>(static_cast<uintptr_t>(vtable_bits)), 0xF8);
    uint32_t result;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov ecx, self
        mov eax, redraw_bits
        call eax
        mov result, eax
    }
#else
    typedef uint32_t (*RedrawProc)(void *);
    result = reinterpret_cast<RedrawProc>(
        static_cast<uintptr_t>(redraw_bits))(self);
#endif
    return result;
}

__forceinline void set_sprite_triplet(void *object, Sprite *volatile *primary,
                        size_t button_offset, bool horizontal, Sprite *sprite1,
                        Sprite *sprite2, Sprite *sprite3) {
    primary[0] = sprite1;
    primary[1] = sprite2;
    primary[2] = sprite3;

    const uint32_t height = 0U - read_volatile_bits(object, 0x4C8);
    const uint32_t width = read_volatile_bits(object, 0x4C4);
    const bool update_button = horizontal
        ? long_from_bits(width) > long_from_bits(height)
        : long_from_bits(width) < long_from_bits(height);
    if (update_button) {
        write_sprite(object, button_offset, sprite1);
        write_sprite(object, button_offset + 4, sprite2);
        write_sprite(object, button_offset + 8, sprite3);
    }
}

uint32_t arithmetic_shift_right_one(uint32_t value) {
    return (value >> 1U) | (value & 0x80000000U);
}

uint32_t signed_min(uint32_t left, uint32_t right) {
    return long_from_bits(left) < long_from_bits(right) ? left : right;
}

OPENSMACX_NOINLINE
uint32_t signed_divide(uint32_t dividend_bits, uint32_t divisor_bits) {
    LONG dividend = long_from_bits(dividend_bits);
    LONG divisor = long_from_bits(divisor_bits);
    LONG quotient;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, dividend
        cdq
        idiv divisor
        mov quotient, eax
    }
#else
    quotient = dividend / divisor;
#endif
    return static_cast<uint32_t>(quotient);
}

}  // namespace


/*
Purpose: Construct a Scroll: run the GraphicWin base construction, install
         the Scroll virtual tables, and load the same fixed/dynamic
         process-default fields that Scroll::close() resets, in the same
         order. The two FlatButton members construct through the compiler's
         own base-then-member sequencing rather than an explicit placement
         call.
// ORIGINAL: 0x006051D0 ??0Scroll@@QAE@XZ 0x006051D0-0x00605367;0x00662E30-0x00662E50
// LEVER: base-ctor ordering  the image runs GraphicWin::construct() BEFORE the
//   two FlatButton members, and a call in the constructor BODY can never get
//   there - member ctors always run first. Moving it into a base subobject
//   (`struct ScrollGraphicWin : GraphicWin`, scroll.h) uses C++'s own
//   base-before-members rule to put it where the image has it. No raw storage,
//   no placement-new, layout unchanged. 21 -> 22 of 83 on its own, and it is
//   what UNBLOCKS the two levers below - they are unreachable until the order
//   is right. This RETRACTS the "raw bytes and placement-new" conclusion the
//   TRIED below reached; that note read the divergence correctly and then
//   picked the wrong of two ways to fix it.
// LEVER: vtable immediates  `extern const uint32_t ScrollPrimaryVtable` with
//   the definition down in this file is invisible at the constructor, so VC6
//   emits a load-then-store where the image has one `mov [esi], 0x669d58`.
//   Defining the value IN scroll.h folds it. 22 -> 77 of 83 - by far the
//   largest single move, and it was hidden behind the ordering defect.
// LEVER: store order  0xA30 is written between 0xA2C and 0xA3C in the image;
//   this body had it three lines later. 77 -> 81 of 83.
// TRIED: the last 2 instructions are the four-store loop's INDUCTION
//   VARIABLE, not its body. The image keeps eax a plain scaled index and folds
//   each table base into the displacement (`xor eax,eax`;
//   `mov edx,[eax+0x9b8de8]`); this tree emits `mov eax,0x14` and
//   `mov edx,[eax-0xc]`, a strength-reduced moving pointer. Measured 2026-08-22:
//   `int index` for the signed `jl` - no change, still 81. Declaring the two
//   tables as real ARRAYS so each subscript folds its own base - WORSE, 76,
//   and subscripting the array by name inside the loop worse again at 75. The
//   `uint32_t *const` locals reading `&Scroll...Defaults` are the best of the
//   four spellings measured.
// TRIED: SEH frame divergence at instruction 0, same family as
//   FlatButton::FlatButton() (flatbutton.cpp) and PullDown::PullDown()
//   (pulldown.cpp) - a REAL derived-class constructor calling a base's
//   `construct()` method under `/GX` gets VC6's unwind scaffolding. Unlike
//   those two, the IMAGE here also carries an SEH prologue of its own
//   (`push -1 / push 0x662e46 / mov eax,fs:[0] / push eax / mov
//   fs:[0],esp`), so this is not the same "we have one where they have
//   none" defect - not re-derived per policy either way.
// TRIED: named FlatButton members over placement-new at a fixed offset.
//   The image calls GraphicWin::construct() BEFORE the two FlatButton
//   constructors (0x6051ee then 0x6051ff/0x60520f); ordinary C++ member
//   construction always runs the member ctors before the constructor BODY
//   starts, so with flat_button_left_/flat_button_right_ declared as real
//   members (as scroll.h already does, and as close()/destroy() already
//   rely on) the two FlatButton() calls compile BEFORE this body's
//   GraphicWin::construct() call - call TARGETS match, call ORDER does not.
//   Reordering would mean storing them as raw bytes and placement-new'ing
//   at the right point in the body instead, which moves scroll.h's layout
//   out from under close()/destroy()'s existing `flat_button_left_.close()`
//   / `.destroy()` calls; left with the named members.
// symbol    ??0Scroll@@QAE@XZ
// size      439 bytes
// prototype Scroll* (__thiscall ??0Scroll@@QAE@XZ)(Scroll* this)
// callers   13   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4CF0 0x00607CF0
Return Value: Instance pointer in EAX
Status: Complete
*/
Scroll::Scroll() {
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = ScrollPrimaryVtable;
    object[0x444 / 4] = ScrollBufferVtable;

    uint32_t *const fixed = &ScrollCloseStaticDefaults;
    uint32_t *const dynamic = &ScrollCloseDynamicDefaults;

    object[0xA14 / 4] = dynamic[0];
    object[0xA1C / 4] = fixed[3];
    object[0xA28 / 4] = 0;
    object[0xA20 / 4] = dynamic[1];
    object[0xA24 / 4] = fixed[4];
    object[0xA2C / 4] = dynamic[1];
    object[0xA30 / 4] = fixed[2];
    object[0xA3C / 4] = 0xFFFFFFFFU;
    object[0xA38 / 4] = 0;
    object[0xA34 / 4] = fixed[1];
    object[0xA40 / 4] = fixed[0];
    object[0xA44 / 4] = 0;
    object[0xA4C / 4] = 0;
    object[0xA50 / 4] = 0;
    object[0xA54 / 4] = 0;
    object[0xA58 / 4] = 0;
    object[0xA48 / 4] = fixed[5];
    object[0xA5C / 4] = fixed[6];
    object[0xA64 / 4] = fixed[7];
    object[0xA68 / 4] = fixed[8];
    object[0xA6C / 4] = fixed[9];
    object[0xA70 / 4] = fixed[10];

    for (int index = 0; index < 3; ++index) {
        object[(0xA7C / 4) + index] = dynamic[2 + index];
        object[(0xA88 / 4) + index] = dynamic[5 + index];
        object[(0xA94 / 4) + index] = dynamic[8 + index];
        object[(0xAA0 / 4) + index] = dynamic[11 + index];
    }
    object[0xA74 / 4] = dynamic[15];
    object[0xA78 / 4] = dynamic[16];
    object[0x2144 / 4] = 0;
    object[0x2148 / 4] = 0;
}

/*
Purpose: Reset Scroll-owned state from the process defaults, close the two
         embedded FlatButtons through their virtual close slots, then close
         the source-owned GraphicWin base.
// ORIGINAL: 0x00605370 ?close@Scroll@@QAEXXZ 0x00605370-0x006054CE
// LEVER: `fixed`/`dynamic` were reinterpret_cast<...>(ScrollCloseStaticDefaults)
//   - casting the VALUE stored in the global to a pointer, an indirect load the
//   image never does. `&ScrollCloseStaticDefaults`/`&ScrollCloseDynamicDefaults`
//   (the object IS the table base) matches the image's direct fixed-address
//   loads and took this from 0/71 to 42/71.
// CORRECTED (2026-08-22): the note this replaces claimed the loop below
//   read the wrong windows of `dynamic[]` and called it BLOCKED. Read
//   directly off 0x605453-0x60547F: `mov edi,[eax+0x9b8de8]` before `eax`'s
//   first increment is dword offset 8 from the 0x9b8de0 table base, i.e.
//   `dynamic[2+index]`; the three reads that follow use `eax` AFTER that
//   same increment, at offsets 0x10/0x1c/0x28, i.e. `dynamic[5+index]`,
//   `dynamic[8+index]`, `dynamic[11+index]` - exactly the four windows the
//   loop below already writes. There is no data-mapping defect here; see the
//   TRIED below and the constructor's own (0x006051D0) matching note for
//   what the remaining gap actually is.
// TRIED: the remaining 6-instruction gap is two SEPARATE, already-typed
//   defects, neither of which yielded to a new spelling (try_spellings,
//   2026-08-22). (1) The loop's induction variable: the image keeps a plain
//   scaled byte offset in `eax` (`xor eax,eax` before the loop); this body's
//   compiled `eax` starts at the strength-reduced `0x14` instead - the exact
//   residual the constructor's own TRIED above documents and could not
//   move either (tried `int index` here too, no change). (2) The SECOND
//   FlatButton dispatch (`flat_button_right_`) schedules its vtable load
//   into `eax` and calls `[eax+0x168]` where the image loads `edx` and calls
//   `[edx+0x168]` with the OTHER field read (`[esi+0x15f8]`) interleaved
//   between them - the same "second call schedules differently" residual
//   already documented and abandoned in `set_button_color` and
//   `set_bevel_thickness` below. Naming the vtable pointer in a local before
//   the second call does not change it either.
// LEVER: dropped the OPENSMACX_NOINLINE call_noarg_virtual helper and
//   expanded both FlatButton close dispatches inline via
//   read_volatile_bits/original_slot<func_noarg_virtual> at slot 0x168,
//   removing both E8 calls to the helper.
// LEVER: `object`/`fixed`/`dynamic` were `volatile uint32_t *` - every one of
//   these is a straight-line copy (no aliasing hazard to guard, nothing
//   reread), so `volatile` bought nothing but forced extra register loads.
//   Plain (non-volatile) pointers, same raw-offset shape, took this from
//   47/71 to 65/71 - and the compiled instruction count now matches the
//   image's 71 exactly. The remaining 6-instruction gap is the induction
//   variable and second-dispatch scheduling the TRIED above names;
//   unaffected by this change.
// symbol    ?close@Scroll@@QAEIXZ
// size      350 bytes
// prototype void (__thiscall ?close@Scroll@@QAEXXZ)(Scroll* this)
// callers   136   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40
// indirect  0x006054AC 0x006054BE
// notes     Runtime redirect installed by DllMain after byte-signature validation; retains two embedded FlatButton virtual-close dependencies and delegates to source-owned GraphicWin close
Return Value: GraphicWin::close return residue
Status: Complete; embedded FlatButton close redirects are source-owned
*/
uint32_t Scroll::close() {
    uint32_t *const object =
        reinterpret_cast<uint32_t *>(this);
    uint32_t *const fixed = &ScrollCloseStaticDefaults;
    uint32_t *const dynamic = &ScrollCloseDynamicDefaults;

    object[0xA14 / 4] = dynamic[0];
    object[0xA1C / 4] = fixed[3];
    object[0xA28 / 4] = 0;
    object[0xA20 / 4] = dynamic[1];
    object[0xA24 / 4] = fixed[4];
    object[0xA2C / 4] = dynamic[1];
    object[0xA3C / 4] = 0xFFFFFFFFU;
    object[0xA38 / 4] = 0;
    object[0xA30 / 4] = fixed[2];
    object[0xA34 / 4] = fixed[1];
    object[0xA40 / 4] = fixed[0];
    object[0xA44 / 4] = 0;
    object[0xA4C / 4] = 0;
    object[0xA50 / 4] = 0;
    object[0xA54 / 4] = 0;
    object[0xA58 / 4] = 0;
    object[0xA48 / 4] = fixed[5];
    object[0xA5C / 4] = fixed[6];
    object[0xA64 / 4] = fixed[7];
    object[0xA68 / 4] = fixed[8];
    object[0xA6C / 4] = fixed[9];
    object[0xA70 / 4] = fixed[10];

    for (size_t index = 0; index < 3; ++index) {
        object[(0xA7C / 4) + index] = dynamic[2 + index];
        object[(0xA88 / 4) + index] = dynamic[5 + index];
        object[(0xA94 / 4) + index] = dynamic[8 + index];
        object[(0xAA0 / 4) + index] = dynamic[11 + index];
    }
    object[0xA74 / 4] = dynamic[15];
    object[0xA78 / 4] = dynamic[16];
    object[0x2144 / 4] = 0;
    object[0x2148 / 4] = 0;

    (ORIGINAL(&flat_button_left_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_left_, 0)) + 0x168))();
    (ORIGINAL(&flat_button_right_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_right_, 0)) + 0x168))();
    return GraphicWin::close();
}

/*
Purpose: Initialize a scrollbar from a rectangle.
// ORIGINAL: 0x00605840 ?init@Scroll@@QAEHPAURECT@@PAUWin@@HH@Z 0x00605840-0x00605885 BYTE_EXACT
// symbol    ?init@Scroll@@QAEHPAUtagRECT@@PAVWin@@HH@Z
// size      69 bytes
// prototype int (__thiscall ?init@Scroll@@QAEHPAURECT@@PAUWin@@HH@Z)(Scroll* this, RECT*, Win*, int, int)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init(RECT *rect, Win *parent, int setting, int options) {
    if (!rect || !parent) {
        return 3;
    }

    // PLAIN ARITHMETIC. `long_from_bits` is a memcpy round trip VC6 will not
    // inline, and there are four of them here - four calls the image does not
    // make. The wrap it was protecting against is what the image's `sub` does
    // anyway.
    return init(rect->left, rect->top, rect->right - rect->left,
                rect->bottom - rect->top, parent, setting, options);
}

/*
Purpose: Initialize a vertical scrollbar using the process-default thickness.
// ORIGINAL: 0x00605890 ?init_vert@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605890-0x006058CB BYTE_EXACT
// symbol    ?init_vert@Scroll@@QAEHHHHPAVWin@@H@Z
// size      59 bytes
// prototype int (__thiscall ?init_vert@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_vert(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal scrollbar using the process-default thickness.
// ORIGINAL: 0x006058D0 ?init_horz@Scroll@@QAEHHHHPAUWin@@H@Z 0x006058D0-0x0060590C BYTE_EXACT
// symbol    ?init_horz@Scroll@@QAEHHHHPAVWin@@H@Z
// size      60 bytes
// prototype int (__thiscall ?init_horz@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_horz(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, length, ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Initialize a vertical nonclient scrollbar.
// ORIGINAL: 0x00605910 ?init_vert_nc@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605910-0x00605955 BYTE_EXACT
// LEVER: ScrollNonClientInit is the object itself, not a pointer to it; drop the reinterpret_cast indirection
// symbol    ?init_vert_nc@Scroll@@QAEHHHHPAVWin@@H@Z
// size      69 bytes
// prototype int (__thiscall ?init_vert_nc@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_vert_nc(
        int x, int y, int length, Win *parent, int setting) {
    ScrollNonClientInit = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal nonclient scrollbar.
// ORIGINAL: 0x00605960 ?init_horz_nc@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605960-0x006059A6 BYTE_EXACT
// LEVER: ScrollNonClientInit is the object itself, not a pointer to it; drop the reinterpret_cast indirection
// symbol    ?init_horz_nc@Scroll@@QAEHHHHPAVWin@@H@Z
// size      70 bytes
// prototype int (__thiscall ?init_horz_nc@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_horz_nc(
        int x, int y, int length, Win *parent, int setting) {
    ScrollNonClientInit = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, length, ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Set the signed scrollbar range and redraw it at the lower endpoint.
// ORIGINAL: 0x006059B0 ?set_range@Scroll@@QAEXHH@Z 0x006059B0-0x00605A0D
// TRIED: manual temp-swap (3/24 agreeing); swap() by name matches structure
//   through the branch and both stores (15/24) but the call target itself
//   doesn't resolve to 0x628A50 in this harness - store order/offsets confirmed
//   correct via tools/store_order.py.
// LEVER: replaced the cached-vtable-bits/redraw_from_vtable(__asm helper)
//   tail with the inline read_volatile_bits/original_slot<func_noarg_virtual>
//   dispatch 0x00605A10 uses - 15/24 to 17/24 agreeing (best flags /Ob0).
//   The swap defect above is the remaining structural cause.
// TRIED: at /Ob0 the named `swap()` call now DOES resolve to 0x628A50 up to
//   instruction 16 - the earlier note is stale - but /Ob0 also stops
//   `read_volatile_bits`/`original_slot` from inlining, so the vtable
//   dispatch tail becomes two real out-of-line calls instead of the image's
//   `mov eax,[esi]; call [eax+0xf8]`. Marking `read_volatile_bits`
//   `__forceinline` did not change this - VC6 12.00.8168 lets /Ob0 win over
//   `__forceinline` too, so no flag set inlines the tail while leaving
//   `swap` a real call. 17/24 (0.815 similar) is the ceiling under every
//   flag combination tried.
// symbol    ?set_range@Scroll@@QAEIHH@Z
// size      93 bytes
// prototype void (__thiscall ?set_range@Scroll@@QAEXHH@Z)(Scroll* this, int, int)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628A50
// indirect  0x00605A03
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_range(int minimum, int maximum) {
    if (minimum < maximum) {
        range_reversed_ = 0U;
    } else {
        range_reversed_ = 1U;
        swap(&minimum, &maximum);
    }
    range_minimum_ = minimum;
    position_ = minimum;
    range_maximum_ = maximum;
    return (ORIGINAL(this)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(this, 0)) + 0xF8))();
}

/*
Purpose: Set the color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A10 ?set_button_color@Scroll@@QAEXH@Z 0x00605A10-0x00605A4D SEMANTIC
// LEVER: vtable_method for the SECOND dispatch, `original_slot` for the first.
//   The two are not interchangeable here: the first dispatch already matches
//   the image's single `call dword ptr [reg+0xf8]` through
//   read_volatile_bits/original_slot, while the second scheduled as
//   `mov eax,[obj]; mov eax,[eax+0xf8]; call eax` - two instructions the image
//   does not have. `vtable_method<Fn>(&obj, 0xF8)` emits the single call.
//   MISMATCH 7/14 -> 12 of 14, granted SEMANTIC. Identical to what
//   set_bevel_thickness, set_bevel_upper and set_bevel_lower each needed, so
//   this is the fourth site of one shape rather than four coincidences.
// LEVER: read_volatile_bits/original_slot inline (not the redraw_from_vtable __asm helper, and the shared helpers marked `inline`) gets the vtable dispatch inlined in place, matching the image's direct load+call; a volatile store on the first field write fixes its scheduling relative to the following `lea`.
// TRIED: the second call's vtable-load/call still schedules differently (edx direct-load + call [edx+0xf8] vs our eax materialised then call eax); tried this+0x15F8 addressing and evaluation-order changes, no effect.
// symbol    ?set_button_color@Scroll@@QAEIH@Z
// size      61 bytes
// prototype void (__thiscall ?set_button_color@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605A31 0x00605A43
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_button_color(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    *reinterpret_cast<volatile uint32_t *>(&color_) = value;
    flat_button_left_.color_ = value;
    flat_button_right_.color_ = value;

    (ORIGINAL(&flat_button_left_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_left_, 0)) + 0xF8))();

    return (ORIGINAL(&flat_button_right_)->*vtable_method<func_noarg_virtual>(
        &flat_button_right_, 0xF8))();
}

/*
Purpose: Set the bevel thickness shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A50 ?set_bevel_thickness@Scroll@@QAEXH@Z 0x00605A50-0x00605A8D SEMANTIC
// LEVER: replaced both redraw_from_vtable(__asm helper) calls with the inline read_volatile_bits/original_slot<func_noarg_virtual> dispatch 0x00605A10 already uses - 4/14 to 7/14 agreeing. First dispatch now matches the image's single `call dword ptr [reg+0xf8]`; the second still schedules as `mov eax,[obj]; mov eax,[eax+0xf8]; call eax` - the same residual 0x00605A10's own TRIED note already gives up on.
// symbol    ?set_bevel_thickness@Scroll@@QAEIH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_thickness@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605A71 0x00605A83
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_bevel_thickness(int thickness) {
    const uint32_t value = static_cast<uint32_t>(thickness);
    *reinterpret_cast<volatile uint32_t *>(&bevel_thickness_) = value;
    flat_button_left_.bevel_thickness_ = value;
    flat_button_right_.bevel_thickness_ = value;
    (ORIGINAL(&flat_button_left_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_left_, 0)) + 0xF8))();
    return (ORIGINAL(&flat_button_right_)->*vtable_method<func_noarg_virtual>(
        &flat_button_right_, 0xF8))();
}

/*
Purpose: Set the upper bevel color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A90 ?set_bevel_upper@Scroll@@QAEXH@Z 0x00605A90-0x00605ACD SEMANTIC
// LEVER: same substitution as 0x00605A50 - 4/14 to 7/14 agreeing, same second-call residual.
// symbol    ?set_bevel_upper@Scroll@@QAEIH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_upper@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605AB1 0x00605AC3
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_bevel_upper(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    *reinterpret_cast<volatile uint32_t *>(&bevel_upper_) = value;
    flat_button_left_.bevel_upper_ = value;
    flat_button_right_.bevel_upper_ = value;
    (ORIGINAL(&flat_button_left_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_left_, 0)) + 0xF8))();
    return (ORIGINAL(&flat_button_right_)->*vtable_method<func_noarg_virtual>(
        &flat_button_right_, 0xF8))();
}

/*
Purpose: Set the lower bevel color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605AD0 ?set_bevel_lower@Scroll@@QAEXH@Z 0x00605AD0-0x00605B0D SEMANTIC
// LEVER: same substitution as 0x00605A50 - 4/14 to 7/14 agreeing, same second-call residual.
// symbol    ?set_bevel_lower@Scroll@@QAEIH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_lower@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605AF1 0x00605B03
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_bevel_lower(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    *reinterpret_cast<volatile uint32_t *>(&bevel_lower_) = value;
    flat_button_left_.bevel_lower_ = value;
    flat_button_right_.bevel_lower_ = value;
    (ORIGINAL(&flat_button_left_)->*original_slot<func_noarg_virtual>(
        reinterpret_cast<const uint8_t *>(read_volatile_bits(&flat_button_left_, 0)) + 0xF8))();
    return (ORIGINAL(&flat_button_right_)->*vtable_method<func_noarg_virtual>(
        &flat_button_right_, 0xF8))();
}

/*
Purpose: Set the scrollbar thickness and reset its thumb rectangle.
// ORIGINAL: 0x00605B80 ?set_bar_thickness@Scroll@@QAEXH@Z 0x00605B80-0x00605BE0
// TRIED: dropping the `volatile` casts on `bar_thickness_`, `border_color_`
//   and the `thumb` RECT alias (plain assignments/reads to the named fields
//   instead): 7/22 -> 8/22 but the compiler now folds the border-color guard
//   into an `or eax, edx` the image never has and drops 4 instructions the
//   image keeps (23 -> 19 compiled) - same dead-store-elimination hazard
//   `set_border_color` below already measured and rejected. Reverted.
// symbol    ?set_bar_thickness@Scroll@@QAEIH@Z
// size      96 bytes
// prototype void (__thiscall ?set_bar_thickness@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_bar_thickness(int thickness) {
    const uint32_t thickness_bits = static_cast<uint32_t>(thickness);
    *reinterpret_cast<volatile int *>(&bar_thickness_) = thickness;
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness_bits);
    thumb->bottom = long_from_bits(thickness_bits);
    const uint32_t color = static_cast<uint32_t>(
        *reinterpret_cast<volatile int *>(&border_color_));
    if (color == 0xFFFFFFFFU) {
        return color;
    }

    const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
    thumb->left = 1;
    const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
    thumb->right = long_from_bits(right);
    const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
    thumb->top = long_from_bits(top);
    thumb->bottom = long_from_bits(bottom);
    return bottom;
}

/*
Purpose: Set the border color and reset the scrollbar thumb rectangle.
// ORIGINAL: 0x00605B10 ?set_border_color@Scroll@@QAEXH@Z 0x00605B10-0x00605B74
// LEVER: image writes defaults unconditionally then overwrites in the branch;
//   ternary/direct-assign forms let the optimizer prove the first write dead and
//   drop it. A `volatile RECT*` keeps both writes, and re-reading `border_color_`
//   (not the `color` parameter) for the branch condition matches the image, which
//   reloads from the just-stored field instead of keeping color live (1/22 -> 9/22).
// TRIED: forcing the thickness read through a volatile cast, no further gain.
// TRIED: `thumb->left = 1` re-spelled as `thumb->left + 1` (mirroring the
//   already-matching `top`/`right`/`bottom` idiom, on the theory that the
//   image's `inc edx` reuses the register the unconditional zero-init left
//   at 0 rather than materialising a fresh literal) - WORSE, 9/22 -> 7/22
//   and the compiled body grows from 21 to 23 instructions. Reverted.
// TRIED: hoisting the RECT zero-init through a named `const uint32_t zero`
//   local, on the theory that a value with no dependency on the
//   `border_color_` store would let VC6 schedule the `xor edx, edx` before
//   it, matching the image's instruction 1. No effect - still 9/22, same
//   schedule. Reverted. The `xor edx, edx` vs its later position is the
//   remaining divergence and is pure instruction scheduling, not source
//   shape.
// size      100 bytes
// prototype void (__thiscall ?set_border_color@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_border_color(int color) {
    *reinterpret_cast<volatile int *>(&border_color_) = color;
    const uint32_t thickness = static_cast<uint32_t>(bar_thickness_);
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness);
    thumb->bottom = long_from_bits(thickness);
    if (border_color_ != -1) {
        const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
        thumb->left = 1;
        const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
        thumb->right = long_from_bits(right);
        const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
        thumb->top = long_from_bits(top);
        thumb->bottom = long_from_bits(bottom);
    }
}

/*
Purpose: Expand a rectangle horizontally and vertically.
// ORIGINAL: 0x00606F00 sub_606f00 0x00606F00-0x00606F2B BYTE_EXACT
// LEVER: the long_from_bits/uint32_t round trip on plain LONG subtraction was dead weight the image never does (no overflow-defined behavior needed for these fields); direct `RECT` member arithmetic on the `volatile RECT*` matches the mnemonics exactly (2/16 -> 11/16).
// LEVER: the `volatile RECT *ordered_rect` alias was itself the remaining wall, not a fix for anything read twice - every field here is written once and read once. Reading `rect->field` directly (compound `-=`/`+=`, no volatile handle) let VC6 fold the param-then-field load order the image has instead of forcing a `mov edx, [eax]` ahead of the `ecx` load through the alias. 11/16 -> BYTE_EXACT 16/16.
// symbol    ?expand_rect@@YAPAUtagRECT@@PAU1@HH@Z
// size      43 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

/*
Purpose: Set the left scrollbar sprites and horizontal button sprites.
// ORIGINAL: 0x00605BE0 ?set_sprite_left@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605BE0-0x00605C2B
// LEVER: marking the shared set_sprite_triplet helper __forceinline removed
//   the 7-argument push sequence and the E8 call, taking this from 0/21 to
//   5/21 agreeing (0.905 similar) - same fix as the other three sprite
//   setters, which share the one helper.
// symbol    ?set_sprite_left@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?set_sprite_left@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_left(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_left1_, 0x15BC, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the right scrollbar sprites and horizontal button sprites.
// ORIGINAL: 0x00605C30 ?set_sprite_right@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605C30-0x00605C7B
// LEVER: same __forceinline on set_sprite_triplet as 0x00605BE0 - 0/21 to
//   5/21 agreeing (0.905 similar).
// symbol    ?set_sprite_right@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?set_sprite_right@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_right(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_right1_, 0x2108, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the upper scrollbar sprites and vertical upper-button sprites.
// ORIGINAL: 0x00605C80 ?UNK1@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605C80-0x00605CCB
// symbol    ?set_sprite_up@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?UNK1@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_up(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_up1_, 0x15BC, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the lower scrollbar sprites and vertical lower-button sprites.
// ORIGINAL: 0x00605CD0 ?UNK2@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605CD0-0x00605D1B
// symbol    ?set_sprite_down@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?UNK2@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_down(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_down1_, 0x2108, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Clamp, optionally reverse, and redraw the scrollbar position.
// ORIGINAL: 0x00605D20 ?set_pos@Scroll@@QAEXH@Z 0x00605D20-0x00605D8A
// TRIED: the guard clause's branch polarity (image falls through to the
//   continue path and jumps forward to a shared `ret` for the early return;
//   this tree's `if(!parent) return 0;` gets compiled the other way around,
//   `jne`+immediate `ret` first). Tried if/else, nested-if without else, and
//   goto-to-label; all three either reproduce the same layout or add a second
//   epilogue. Everything past the guard already matches in shape (8/29).
// TRIED: replacing the redraw_from_vtable(__asm helper) tail with the
//   inline read_volatile_bits/original_slot<func_noarg_virtual> dispatch -
//   still 8/29, no measured improvement, so reverted. The inline form emits
//   `mov eax,[ecx]; mov eax,[eax+0xf8]; call eax` (3 insns) against the
//   image's `mov edx,[ecx]; call dword ptr [edx+0xf8]` (2), the same
//   second-dispatch scheduling gap 0x00605A10's own note gives up on.
// LEVER: `parent`/`minimum`/`current`/`maximum`/`range_reversed_` reads were
//   each cast through `volatile` to a single already-named field - no raw
//   offsets, so nothing to rename, just the qualifier to drop. Plain reads
//   took this from 8/29 to 11/29. The branch-polarity gap above is
//   unaffected: it is still the first divergence, at instruction 2.
// symbol    ?set_pos@Scroll@@QAEIH@Z
// size      106 bytes
// prototype void (__thiscall ?set_pos@Scroll@@QAEXH@Z)(Scroll* this, int position)
// callers   17   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605D80
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_pos(int position) {
    ::Win *const parent = win_parent_;
    if (!parent) {
        return 0U;
    }

    ScrollCurrentWin() = parent;

    const int minimum = range_minimum_;
    int *const current = &position_;
    if (position < minimum) {
        *current = minimum;
    } else {
        const int maximum = range_maximum_;
        *current = position > maximum ? maximum : position;
    }
    if (range_reversed_ != 0U) {
        const uint32_t maximum = static_cast<uint32_t>(range_maximum_);
        const uint32_t clamped = static_cast<uint32_t>(*current);
        *current = long_from_bits(
            maximum - clamped + static_cast<uint32_t>(minimum));
    }
    const uint32_t vtable_bits = read_volatile_bits(this, 0);
    return redraw_from_vtable(this, vtable_bits);
}

/*
Purpose: Compute and publish the scrollbar thumb rectangle.
// ORIGINAL: 0x00606C50 ?compute_thumb_rect@Scroll@@QAEXPAURECT@@@Z 0x00606C50-0x00606E9E
// symbol    ?compute_thumb_rect@Scroll@@QAEXPAUtagRECT@@@Z
// size      590 bytes
// prototype void (__thiscall ?compute_thumb_rect@Scroll@@QAEXPAURECT@@@Z)(Scroll* this, RECT*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
Verification note: several mutation-harness survivors in this body are
equivalent by construction, not coverage gaps. The two interim stores to
0xA3C in the drag branch are dead - the branch ends by unconditionally
storing 0xFFFFFFFF over them, faithfully mirroring the original's transient
writes, so no suite can observe them. The clamp idioms are strictness-
insensitive: `x < 0 ? 0U : x` equals `x < 1 ? 0U : x` and `x <= 0 ? 0U : x`
everywhere (the only inputs where the branches differ select equal values),
and likewise `a > b ? a : b` equals `a >= b ? a : b`. The early read_bits
offsets over the just-normalized rect scan bytes that were all zeroed two
statements earlier, so off-by-small-constant offset mutants read the same
zeros.
*/
void Scroll::compute_thumb_rect(RECT *rect) {
    uint8_t *thumb = reinterpret_cast<uint8_t *>(this) + 0xA4C;
    for (size_t offset = 0; offset < sizeof(RECT); offset += sizeof(uint32_t)) {
        write_bits(rect, offset, read_bits(thumb, offset));
    }

    uint32_t left = read_bits(rect, 0);
    uint32_t right = read_bits(rect, 8) - left;
    const uint32_t top = read_bits(rect, 4);
    write_bits(rect, 8, right);
    uint32_t bottom = read_bits(rect, 12) - top;
    left = 0;
    write_bits(rect, 0, left);
    write_bits(rect, 12, bottom);
    write_bits(rect, 4, 0);
    if (read_bits(this, 0xA1C) != 0xFFFFFFFFU) {
        left = read_bits(rect, 0) + 1U;
        right = read_bits(rect, 8) + 1U;
        const uint32_t inset_top = read_bits(rect, 4) + 1U;
        bottom = read_bits(rect, 12) + 1U;
        write_bits(rect, 0, left);
        write_bits(rect, 8, right);
        write_bits(rect, 4, inset_top);
        write_bits(rect, 12, bottom);
    }

    right = read_bits(rect, 8);
    left = read_bits(rect, 0);
    const uint32_t extent = right - left;
    const uint32_t drag_coordinate = read_bits(this, 0xA3C);
    uint32_t thumb_offset;

    if (drag_coordinate == 0xFFFFFFFFU) {
        const bool no_end_buttons = (read_bits(this, 0xA14) & 2U) != 0;
        thumb_offset = no_end_buttons ? 0U : extent + 1U;
        uint32_t adjustment = 0xFFFFFFFFU - (no_end_buttons
            ? extent : extent * 3U);
        if (read_bits(this, 0xA1C) != 0xFFFFFFFFU) {
            adjustment -= 2U;
        }
        const uint32_t height = 0U - read_bits(this, 0x4C8);
        const uint32_t width = read_bits(this, 0x4C4);
        const bool horizontal = long_from_bits(width) > long_from_bits(height);
        const uint32_t axis_length = horizontal ? width : height;
        const uint32_t maximum = read_bits(this, 0xA24);
        const uint32_t minimum = read_bits(this, 0xA20);
        if (maximum != minimum) {
            const uint32_t numerator = (read_bits(this, 0xA2C) - minimum)
                * (axis_length + adjustment);
            // LEVER: the image makes NO calls at all in this body -
            // signed_divide/arithmetic_shift_right_one/signed_min are
            // hand-inlined here rather than called, matching call_diff's
            // "this tree 3, image 0" gap.
            thumb_offset += static_cast<uint32_t>(
                long_from_bits(numerator) / long_from_bits(maximum - minimum));
        }
        if (horizontal) {
            write_bits(rect, 0, read_bits(rect, 0) + thumb_offset);
            write_bits(rect, 8, read_bits(rect, 8) + thumb_offset);
        } else {
            write_bits(rect, 4, read_bits(rect, 4) + thumb_offset);
            write_bits(rect, 12, read_bits(rect, 12) + thumb_offset);
        }
    } else {
        const bool no_end_buttons = (read_bits(this, 0xA14) & 2U) != 0;
        const uint32_t far_edge = left + extent;
        const uint32_t candidate = drag_coordinate
            + ((extent >> 1U) | (extent & 0x80000000U));
        write_bits(this, 0xA3C, candidate);
        const uint32_t height = 0U - read_bits(this, 0x4C8);
        const uint32_t width = read_bits(this, 0x4C4);
        const bool horizontal = long_from_bits(width) > long_from_bits(height);
        const uint32_t axis_length = horizontal ? width : height;
        const uint32_t upper = axis_length - (no_end_buttons
            ? far_edge : far_edge * 2U);
        const uint32_t limited =
            (long_from_bits(candidate) < long_from_bits(upper)) ? candidate : upper;
        if (no_end_buttons) {
            thumb_offset = long_from_bits(limited) < 0 ? 0U : limited;
        } else if (long_from_bits(far_edge) > long_from_bits(limited)) {
            thumb_offset = far_edge;
        } else {
            thumb_offset = limited;
        }
        write_bits(this, 0xA3C, thumb_offset);
        if (horizontal) {
            write_bits(rect, 0, read_bits(rect, 0) + thumb_offset);
            write_bits(rect, 8, read_bits(rect, 8) + thumb_offset);
        } else {
            write_bits(rect, 4, read_bits(rect, 4) + thumb_offset);
            write_bits(rect, 12, read_bits(rect, 12) + thumb_offset);
        }
        write_bits(this, 0xA3C, 0xFFFFFFFFU);
    }
    for (size_t coordinate = 0; coordinate < sizeof(RECT);
            coordinate += sizeof(uint32_t)) {
        write_bits(thumb, coordinate, read_bits(rect, coordinate));
    }
}

/*
Purpose: Reset the scrollbar thumb rectangle from its stored thickness.
// ORIGINAL: 0x00606EA0 ?set_thumb_rect@Scroll@@QAEXXZ 0x00606EA0-0x00606EF8
// TRIED: dropping the `volatile` casts on `bar_thickness_`, `border_color_`
//   and the `thumb` RECT alias: 12/20 -> 7/20, a clear regression. Without
//   `volatile` the optimizer proves the unconditional 0/0/thickness/thickness
//   rect writes are dead when the branch below overwrites them and elides
//   them, which the image does not do - same shape as `set_border_color`'s
//   already-documented dead-store hazard. Reverted.
// symbol    ?set_thumb_rect@Scroll@@QAEIXZ
// size      88 bytes
// prototype void (__thiscall ?set_thumb_rect@Scroll@@QAEXXZ)(Scroll* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_thumb_rect() {
    const uint32_t thickness = static_cast<uint32_t>(
        *reinterpret_cast<volatile int *>(&bar_thickness_));
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness);
    thumb->bottom = long_from_bits(thickness);
    if (*reinterpret_cast<volatile int *>(&border_color_) == -1) {
        return thickness;
    }

    const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
    thumb->left = 1;
    const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
    thumb->right = long_from_bits(right);
    const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
    thumb->top = long_from_bits(top);
    thumb->bottom = long_from_bits(bottom);
    return bottom;
}

int __fastcall scroll_init_rect_redirect(
        Scroll *self, void *, RECT *rect, Win *parent, int setting, int options) {
    return self->init(rect, parent, setting, options);
}

int __fastcall scroll_init_vert_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_vert(x, y, length, parent, setting);
}

int __fastcall scroll_init_horz_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_horz(x, y, length, parent, setting);
}

int __fastcall scroll_init_vert_nc_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_vert_nc(x, y, length, parent, setting);
}

int __fastcall scroll_init_horz_nc_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_horz_nc(x, y, length, parent, setting);
}

uint32_t __fastcall scroll_set_range_redirect(
        Scroll *self, void *, int minimum, int maximum) {
    return self->set_range(minimum, maximum);
}

uint32_t __fastcall scroll_set_button_color_redirect(
        Scroll *self, void *, int color) {
    return self->set_button_color(color);
}

uint32_t __fastcall scroll_set_bevel_thickness_redirect(
        Scroll *self, void *, int thickness) {
    return self->set_bevel_thickness(thickness);
}

uint32_t __fastcall scroll_set_bevel_upper_redirect(
        Scroll *self, void *, int color) {
    return self->set_bevel_upper(color);
}

uint32_t __fastcall scroll_set_bevel_lower_redirect(
        Scroll *self, void *, int color) {
    return self->set_bevel_lower(color);
}

uint32_t __fastcall scroll_set_bar_thickness_redirect(
        Scroll *self, void *, int thickness) {
    return self->set_bar_thickness(thickness);
}

void __fastcall scroll_set_border_color_redirect(
        Scroll *self, void *, int color) {
    self->set_border_color(color);
}

Sprite *__fastcall scroll_set_sprite_left_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_left(sprite1, sprite2, sprite3);
    return sprite1;
}

Sprite *__fastcall scroll_set_sprite_right_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_right(sprite1, sprite2, sprite3);
    return sprite1;
}

Sprite *__fastcall scroll_set_sprite_up_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_up(sprite1, sprite2, sprite3);
    return sprite1;
}

Sprite *__fastcall scroll_set_sprite_down_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_down(sprite1, sprite2, sprite3);
    return sprite1;
}

uint32_t __fastcall scroll_set_pos_redirect(
        Scroll *self, void *, int position) {
    return self->set_pos(position);
}

RECT *__fastcall scroll_compute_thumb_rect_redirect(
        Scroll *self, void *, RECT *rect) {
    self->compute_thumb_rect(rect);
    return reinterpret_cast<RECT *>(
        reinterpret_cast<uint8_t *>(self) + 0xA4C);
}

uint32_t __fastcall scroll_set_thumb_rect_redirect(Scroll *self, void *) {
    return self->set_thumb_rect();
}

uint32_t __fastcall scroll_close_redirect(Scroll *self, void *) {
    return self->close();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00606310 ?on_left_click@Scroll@@QAEHHH@Z 0x00606310-0x00606313 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?on_left_click@Scroll@@QAEHHH@Z)(Scroll* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
#pragma warning(push)
#pragma warning(disable : 4716)
// C4716 IS SUPPRESSED HERE ON PURPOSE. The mangled name says the function
// returns `int`, and the original body is three bytes - `ret 8` at 0x00606310
// and `ret 0xc` at 0x005F6A40 - which never sets EAX. A `return 0;` would emit
// `xor eax, eax` and make the function five bytes, so falling off the end is
// not a defect to fix but the behaviour to reproduce.
int Scroll::on_left_click(int, int) {
}
#pragma warning(pop)

int __fastcall scroll_on_left_click_redirect(Scroll *self, void *, int a1, int a2) {
    return self->on_left_click(a1, a2);
}


/*
Purpose: Destroy a Scroll: stage its two virtual tables, run close, destroy
         the embedded right then left FlatButtons, and finish with the
         GraphicWin base teardown. The original's exception frame is omitted
         as unreachable per policy.
// ORIGINAL: 0x00406E60 ??1Scroll@@QAE@XZ 0x00406E60-0x00406F1A;0x00650BB0-0x00650BEE
// TRIED: dropping `volatile` from `object` (plain `uint32_t *`, same raw
//   offsets): no change, still 0/46. The image's prologue is an SEH frame
//   (`push ebp; mov ebp,esp; push -1; push handler; ...`) that this body's
//   policy-omitted frame (see Purpose above) never reproduces, so every
//   instruction is misaligned from #0 - the divergence is the missing frame,
//   not the volatile.
// symbol    ?destroy@Scroll@@QAEPAV1@XZ
// size      248 bytes
// prototype void (__thiscall ??1Scroll@@QAE@XZ)(Scroll* this)
// callers   41   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00605370 0x00607040 0x00607DA0
Return Value: Instance pointer in EAX
Status: Complete
*/
Scroll *Scroll::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = ScrollPrimaryVtable;
    object[0x444 / 4] = ScrollBufferVtable;
    close();
    flat_button_right_.destroy();
    flat_button_left_.destroy();
    graphic_win_destructor_redirect(
        static_cast<GraphicWin *>(this), nullptr);
    return this;
}

Scroll *__fastcall scroll_destructor_redirect(Scroll *self, void *) {
    return self->destroy();
}

/*
Purpose: The compiler-generated scalar deleting destructor: run the complete
         destructor and, when bit 0 of the mode asks, free the storage to the
         game heap.
// ORIGINAL: 0x00406F20 ??_GScroll@@UAEPAXI@Z 0x00406F20-0x00406F41 BYTE_EXACT
// symbol    ?scroll_scalar_dtor_redirect@@YIPAXPAVScroll@@PAXI@Z
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00406E60 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scroll_scalar_dtor_redirect(Scroll *self, void *,
                                             unsigned int mode) {
    self->destroy();
    if (mode & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GScroll@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070C0 ??_GScroll@@WEEE@AEPAXI@Z 0x004070C0-0x004070CB
// symbol    ??_EScroll@@WEEE@AEPAXI@Z
// CORRECTED from ??3Scroll@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406F20` into
//   ??_GScroll@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
