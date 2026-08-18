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

namespace {

static const int ItemNotFound = 11;

PullDownItem *find_item(PullDownItem *items, int id) {
    for (int index = 0; index < 64; ++index) {
        if (items[index].id == -1) {
            break;
        }
        if (items[index].id == id) {
            return &items[index];
        }
    }
    return nullptr;
}

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "PullDown requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

/*
Purpose: Hide the first item with the requested ID.
// ORIGINAL: 0x005F8CB0 ?hide_item@PullDown@@QAEHH@Z 0x005F8CB0-0x005F8D1F
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    if (item->flags & 1U) {
        item->flags &= ~1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) - 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Show the first item with the requested ID.
// ORIGINAL: 0x005F8D20 ?show_item@PullDown@@QAEHH@Z 0x005F8D20-0x005F8D8F
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    if (!(item->flags & 1U)) {
        item->flags |= 1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) + 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Disable the first item with the requested ID.
// ORIGINAL: 0x005F8D90 ?disable_item@PullDown@@QAEHH@Z 0x005F8D90-0x005F8DEF
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    item->flags |= 2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Enable the first item with the requested ID.
// ORIGINAL: 0x005F8DF0 ?enable_item@PullDown@@QAEHH@Z 0x005F8DF0-0x005F8E4F
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    item->flags &= ~2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Check the first item with the requested ID.
// ORIGINAL: 0x005F9040 ?check_item@PullDown@@QAEHH@Z 0x005F9040-0x005F909F
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    item->flags |= 8U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Uncheck the first item with the requested ID.
// ORIGINAL: 0x005F90A0 ?uncheck_item@PullDown@@QAEHH@Z 0x005F90A0-0x005F90FF
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
    PullDownItem *item = find_item(items_, id);
    if (!item) {
        return ItemNotFound;
    }
    item->flags &= ~8U;
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

int __fastcall pull_down_hide_item_redirect(
        PullDown *self, void *, int id) {
    return self->hide_item(id);
}

int __fastcall pull_down_show_item_redirect(
        PullDown *self, void *, int id) {
    return self->show_item(id);
}

int __fastcall pull_down_disable_item_redirect(
        PullDown *self, void *, int id) {
    return self->disable_item(id);
}

int __fastcall pull_down_enable_item_redirect(
        PullDown *self, void *, int id) {
    return self->enable_item(id);
}

int __fastcall pull_down_check_item_redirect(
        PullDown *self, void *, int id) {
    return self->check_item(id);
}

int __fastcall pull_down_uncheck_item_redirect(
        PullDown *self, void *, int id) {
    return self->uncheck_item(id);
}

int __fastcall pull_down_get_selected_redirect(PullDown *self, void *) {
    return self->get_selected();
}

func_sprite_free *PullDownFree = (func_sprite_free *)0x00644EF2;
const uint32_t PullDownPrimaryVtable = 0x0066FF40;
const uint32_t PullDownBufferVtable = 0x0066FF38;
uint32_t *PullDownFieldF38Default = (uint32_t *)0x009B7B58;
uint32_t *PullDownFieldF3CDefault = (uint32_t *)0x009B7B5C;

/*
Purpose: Destroy a PullDown by releasing every item's text pair, resetting
         the trailing fields from their global defaults, and delegating to
         the GraphicWin destructor.
// ORIGINAL: 0x005F88A0 ??1PullDown@@QAE@XZ 0x005F88A0-0x005F891B
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
PullDown *__fastcall pull_down_destructor_redirect(PullDown *self, void *) {
    volatile uint32_t *const ordered = reinterpret_cast<volatile uint32_t *>(self);
    ordered[0x000 / 4] = PullDownPrimaryVtable;
    ordered[0x444 / 4] = PullDownBufferVtable;

    // Sixty-four items with two owned strings each: the text at 0xA18 and the
    // right-hand text at 0xA1C, stride 0x14. The mnemonic pointer at 0xA28 is
    // deliberately left alone, exactly as the legacy body leaves it.
    for (size_t index = 0; index < 64; ++index) {
        volatile uint32_t *const text = ordered + (0xA18 / 4) + index * 5;
        volatile uint32_t *const right_text = text + 1;
        if (*text != 0) {
            PullDownFree(reinterpret_cast<void *>(*text));
            *text = 0;
        }
        if (*right_text != 0) {
            PullDownFree(reinterpret_cast<void *>(*right_text));
            *right_text = 0;
        }
    }

    *reinterpret_cast<volatile uint8_t *>(
        reinterpret_cast<uint8_t *>(self) + 0xF34) = 1;
    ordered[0xF38 / 4] = *PullDownFieldF38Default;
    ordered[0xF3C / 4] = *PullDownFieldF3CDefault;
    graphic_win_destructor_redirect(
        reinterpret_cast<GraphicWin *>(self), nullptr);
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

int __fastcall pull_down_unk2_redirect(
        PullDown *self, void *, int a) {
    return self->UNK2(a);
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

int __fastcall pull_down_unk3_redirect(
        PullDown *self, void *, int a, int b, int c) {
    return self->UNK3(a, b, c);
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

int __fastcall pull_down_unk5_redirect(
        PullDown *self, void *) {
    return self->UNK5();
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

int __fastcall pull_down_unk6_redirect(
        PullDown *self, void *) {
    return self->UNK6();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005FA740 ?on_mouse_leave@PullDown@@QAEXHH@Z 0x005FA740-0x005FA743 BYTE_EXACT
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

void __fastcall pull_down_on_mouse_leave_redirect(PullDown *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
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

int __fastcall pull_down_id_to_index_redirect(PullDown *self, void *, int id) {
    return self->id_to_index(id);
}

/*
// ORIGINAL: 0x005F93A0 ?hide@PullDown@@QAEXXZ 0x005F93A0-0x005F93B9 BYTE_EXACT
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
    reinterpret_cast<Win *>(this)->hide();
    do_all_draws();
}
