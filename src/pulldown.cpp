/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "temp.h"  // do_all_draws
#include "pulldown.h"
#include "vtable_shim.h"
#include <stdlib.h>

namespace {

static const int ItemNotFound = 11;

__forceinline int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "PullDown requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

/*
Purpose: Hide the first item with the requested ID.
// ORIGINAL: 0x005F8CB0 ?hide_item@PullDown@@QAEHH@Z 0x005F8CB0-0x005F8D1F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      111 bytes
// prototype int (__thiscall ?hide_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::hide_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    if (items_[index].flags & 1U) {
        items_[index].flags &= ~1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) - 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Show the first item with the requested ID.
// ORIGINAL: 0x005F8D20 ?show_item@PullDown@@QAEHH@Z 0x005F8D20-0x005F8D8F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      111 bytes
// prototype int (__thiscall ?show_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::show_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    if (!(items_[index].flags & 1U)) {
        items_[index].flags |= 1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) + 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Disable the first item with the requested ID.
// ORIGINAL: 0x005F8D90 ?disable_item@PullDown@@QAEHH@Z 0x005F8D90-0x005F8DEF BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?disable_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::disable_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags |= 2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Enable the first item with the requested ID.
// ORIGINAL: 0x005F8DF0 ?enable_item@PullDown@@QAEHH@Z 0x005F8DF0-0x005F8E4F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?enable_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::enable_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags &= ~2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Check the first item with the requested ID.
// ORIGINAL: 0x005F9040 ?check_item@PullDown@@QAEHH@Z 0x005F9040-0x005F909F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?check_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::check_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags |= 8U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Uncheck the first item with the requested ID.
// ORIGINAL: 0x005F90A0 ?uncheck_item@PullDown@@QAEHH@Z 0x005F90A0-0x005F90FF BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?uncheck_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::uncheck_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags &= ~8U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Return the selected item index unless that item is disabled.
// ORIGINAL: 0x005F9F40 ?get_selected@PullDown@@QAEHXZ 0x005F9F40-0x005F9F5C BYTE_EXACT
// size      28 bytes
// prototype int (__thiscall ?get_selected@PullDown@@QAEHXZ)(PullDown* this)
// callers   13   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::get_selected() {
    if (selected_ == -1) {
        return -1;
    }
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
                  "PullDown unchecked indexing requires PE32");
    uintptr_t flags_address = reinterpret_cast<uintptr_t>(this);
    flags_address += 0xA24U + static_cast<uint32_t>(selected_) * 0x14U;
    uint32_t flags;
    memcpy(&flags, reinterpret_cast<const void *>(flags_address), sizeof(flags));
    return (flags & 2U) ? -1 : selected_;
}








const uint32_t PullDownPrimaryVtable = 0x0066FF40;
const uint32_t PullDownBufferVtable = 0x0066FF38;
uint32_t PullDownFieldF38Default;  // 0x009B7B58
uint32_t PullDownFieldF3CDefault;  // 0x009B7B5C

/*
Purpose: Destroy a PullDown by releasing every item's text pair, resetting
         the trailing fields from their global defaults, and delegating to
         the GraphicWin destructor.
// ORIGINAL: 0x005F88A0 ??1PullDown@@QAE@XZ 0x005F88A0-0x005F891B
// symbol    ?destroy@PullDown@@QAEPAV1@XZ
// LEVER: void trailing-call tail-jmps every time (VC6 always sibling-calls
//          a trailing call needing no stack cleanup, verified in isolation
//          even for a compiler-generated base-dtor call); returning `this`
//          (matching BaseButton::destroy/Scroll::destroy) forces a real
//          call+epilogue and mirrors the image's `call 0x5d4dd0` / pop* /
//          ret shape instead of pop* / jmp.
// size      123 bytes
// prototype void (__thiscall ??1PullDown@@QAE@XZ)(PullDown* this)
// callers   27   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00644EF2
Status: Complete
Verification note: the two virtual-table stores are dead - the GraphicWin
delegation unconditionally overwrites both slots with its own tables - so
they mirror the original's transient writes and no suite can observe them.
*/
PullDown *PullDown::destroy() {
    // 0x000 and 0x444 are the Win and Buffer vtable slots GraphicWin
    // installs; they are compiler-managed, not ordinary members, so they
    // stay at their raw offset. They are also dead stores here - the
    // GraphicWin delegation below unconditionally overwrites both.
    uint32_t *const ordered = reinterpret_cast<uint32_t *>(this);
    ordered[0x000 / 4] = PullDownPrimaryVtable;
    ordered[0x444 / 4] = PullDownBufferVtable;

    // Sixty-four items with two owned strings each, released directly
    // through items_'s own named fields.
    for (size_t index = 0; index < 64; ++index) {
        if (items_[index].text != nullptr) {
            free(items_[index].text);
            items_[index].text = nullptr;
        }
        if (items_[index].right_text != nullptr) {
            free(items_[index].right_text);
            items_[index].right_text = nullptr;
        }
    }

    dirty_ = 1;
    field_F38_ = PullDownFieldF38Default;
    field_F3C_ = PullDownFieldF3CDefault;
    reinterpret_cast<GraphicWin *>(this)->~GraphicWin();
    return this;
}

PullDown *__fastcall pull_down_destructor_redirect(PullDown *self, void *) {
    self->destroy();
    return self;
}

/*
Purpose: Legacy stub; the original body returns 8 without reading its
         arguments.
// ORIGINAL: 0x005F8C90 ?UNK2@PullDown@@QAEHH@Z 0x005F8C90-0x005F8C98 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK2@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK2(int) {
    return 8;
}


/*
Purpose: Legacy stub; the original body returns 8 without reading its
         arguments.
// ORIGINAL: 0x005F8CA0 ?UNK3@PullDown@@QAEHHHH@Z 0x005F8CA0-0x005F8CA8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK3@PullDown@@QAEHHHH@Z)(PullDown* this, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK3(int, int, int) {
    return 8;
}


/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x005FA750 ?UNK5@PullDown@@QAEHXZ 0x005FA750-0x005FA756 BYTE_EXACT
// size      6 bytes
// prototype int (__thiscall ?UNK5@PullDown@@QAEHXZ)(PullDown* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK5() {
    return 1;
}


/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x005FA760 ?UNK6@PullDown@@QAEHXZ 0x005FA760-0x005FA766 BYTE_EXACT
// size      6 bytes
// prototype int (__thiscall ?UNK6@PullDown@@QAEHXZ)(PullDown* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK6() {
    return 1;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005FA740 ?on_mouse_leave@PullDown@@QAEXHH@Z 0x005FA740-0x005FA743 BYTE_EXACT
// symbol    ?on_mouse_leave@PullDown@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PullDown::on_mouse_leave(int, int) {
}


/*
Purpose: Find a menu item by its id and return its index, scanning up to the
         64 item slots. Stops at the first slot whose id is the -1 sentinel,
         and returns -1 when the id is not found or that sentinel is reached.
// ORIGINAL: 0x005F9D00 ?id_to_index@PullDown@@QAEHH@Z 0x005F9D00-0x005F9D28 BYTE_EXACT
// size      40 bytes
// prototype int (__thiscall ?id_to_index@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the item's index, or -1 when absent
Status: Complete
*/
int PullDown::id_to_index(int id) {
    for (int index = 0; index < 64; ++index) {
        const int slot_id = items_[index].id;
        if (slot_id == -1) {
            return -1;
        }
        if (slot_id == id) {
            return index;
        }
    }
    return -1;
}


/*
// ORIGINAL: 0x005F93A0 ?hide@PullDown@@QAEXXZ 0x005F93A0-0x005F93B9 BYTE_EXACT
// symbol    ?hide@PullDown@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@PullDown@@QAEXXZ)(PullDown* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x005FCBB0
// indirect  0x005F93A5
Status: Complete
*/
void PullDown::hide() {
    reinterpret_cast<VCall *>(this)->slot058();
    reinterpret_cast<Win *>(this)->Win::hide();
    do_all_draws();
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005F8800 ??0PullDown@@QAE@XZ 0x005F8800-0x005F8893
// size      147 bytes
// prototype void (__thiscall ??0PullDown@@QAE@XZ)(PullDown* this)
// callers   26   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0
*/

// TRIED: SEH frame - same symptom as FlatButton::FlatButton() (see
// flatbutton.cpp for the full measurement) - PullDown : public GraphicWin
// is a REAL constructor calling `GraphicWin::construct()`, and this tree's
// compiled body gets an unwind frame (`push -1 / push handler / mov eax,
// fs:[0] / push eax / mov fs:[0], esp`) the image does not have (flags
// carry no `frame`).
//
// MEASURED 2026-08-21: this is NOT `GraphicWin`'s `Buffer buffer_` etc.
// being held by value - swapping every by-value member up the whole
// GraphicWin/Win/BaseButton chain for raw storage of the same size does
// not drop the frame. Only compiling with `/GX-` does (confirmed on
// FlatButton's identical shape), and `/GX-` is not reachable from `src/`:
// it is a whole-translation-unit flag, and `Buffer::~Buffer()`
// (0x005D7410, `buffer.h`) is currently BYTE_EXACT specifically because
// the image's own destructor carries this same SEH prologue - turning
// `/GX` off would regress that claim. `__declspec(nothrow)` / `throw()`
// on `GraphicWin::construct()`'s declaration does not suppress the frame
// either. Left as a MISMATCH; see flatbutton.cpp for the full note.
PullDown::PullDown() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();
    for (size_t index = 0; index < 64; ++index) {
        items_[index].text = nullptr;
        items_[index].right_text = nullptr;
        items_[index].id = -1;
        items_[index].flags = 0;
        items_[index].mnemonic = nullptr;
    }

    // STORE ORDER, NOT DECLARATION ORDER: the image writes menu_ (0xF30)
    // before visible_count_ (0xF2C), and field_A14_ (0xA14, the class's
    // FIRST declared field) dead last, after dirty_.
    field_F18_ = 0;
    field_F1C_ = 0;
    item_count_ = 0;
    width_ = 0;
    selected_ = -1;
    menu_ = nullptr;
    visible_count_ = 0;
    dirty_ = 1;
    field_A14_ = 0;
    field_F38_ = PullDownFieldF38Default;
    field_F3C_ = PullDownFieldF3CDefault;
}

/*
Purpose: Step the receiver back to the subobject ??_GPullDown@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005FA790 ??_GPullDown@@WEEE@AEPAXI@Z 0x005FA790-0x005FA79B BYTE_EXACT
// symbol    ??_EPullDown@@WEEE@AEPAXI@Z
// CORRECTED from ??3PullDown@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FA770` into
//   ??_GPullDown@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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
