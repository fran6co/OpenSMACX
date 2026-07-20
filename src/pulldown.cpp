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
#include "pulldown.h"

namespace {

constexpr int ItemNotFound = 11;

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
Original Offset: 005F8CB0
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
Original Offset: 005F8D20
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
Original Offset: 005F8D90
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
Original Offset: 005F8DF0
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
Original Offset: 005F9040
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
Original Offset: 005F90A0
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
Original Offset: 005F9F40
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
Original Offset: 005F88A0
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
