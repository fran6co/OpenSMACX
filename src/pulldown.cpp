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
#include "menu.h"
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
Purpose: Append an item, splitting its text at the first '|' into a left half
         and a right-aligned half, and isolating the mnemonic after '&'.
         A full table answers 1, a duplicate id answers 1, a missing text or a
         -1 id answers 3, an occupied slot answers 0xC, and an allocation
         failure answers 4.
// ORIGINAL: 0x005F89D0 ?add_item@PullDown@@QAEHHPAD@Z 0x005F89D0-0x005F8BDF BYTE_EXACT
// LEVER: items_[slot].field with `const int slot = new_count - 1`, NOT a
//          `PullDownItem *slot = &items_[slot]` pointer. The image keeps
//          `this + 20*(new_count-1)` in esi and carries items_'s own 0xa18 in
//          every displacement (`mov [esi+0xa20],ebp` for .id, `[esi+0xa18]`
//          for .text); a slot pointer emits `[esi+8]` / `[esi+0x10]` instead
//          and diverges from the very first store. Same shape as the
//          BYTE_EXACT add_separator beside it. `&items_[slot].mnemonic` is
//          the one address the image does fold from scratch
//          (`lea ecx,[edi+edi*4+0x285]; lea ebp,[ebx+ecx*4]`), which is what
//          spelling the mnemonic through items_[slot] twice lets VC6 CSE.
// TRIED: the duplicate scan spelled out as a local loop rather than the
//          shared find_item; the forwarder's Ghidra-derived stack-slot
//          aliasing reading. find_item is both shorter and the family's
//          established helper - and `if (duplicate != -1)` is exactly the
//          constant-propagation shape the image shows (the sentinel-break and
//          loop-exhaust edges jump straight to the append block because they
//          carry a known -1, only the dup edge gets a real `cmp eax,-1`).
// size      527 bytes
// prototype int (__thiscall ?add_item@PullDown@@QAEHHPAD@Z)(PullDown* this, int, int8*)
// callers   12   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005F9D30 0x006453E0 0x00645470 0x00645DD0
Return Value: 0 when the item was added, else the reason it was not
Status: Complete
*/
int PullDown::add_item(int id, char *text) {
    const int new_count = ++item_count_;
    if (new_count > 64) {
        --item_count_;
        return 1;
    }
    if (text == nullptr || id == -1) {
        return 3;
    }

    // A duplicate id is refused; an unused slot is only where the item lands.
    const int duplicate = find_item(items_, id);
    if (duplicate != -1) {
        return 1;
    }

    const int slot = new_count - 1;
    items_[slot].id = id;
    if (items_[slot].text != nullptr) {
        return 0xC;
    }

    // Split the text at the '|' first, so both halves are measured from it.
    char *separator = strchr(text, '|');
    if (separator != nullptr) {
        *separator = '\0';
    }

    items_[slot].text = static_cast<char *>(mem_get(strlen(text) + 1));
    if (items_[slot].text == nullptr) {
        items_[slot].id = -1;
        --item_count_;
        return 4;
    }
    items_[slot].text[0] = '\0';
    items_[slot].mnemonic = nullptr;

    // '&x' marks the mnemonic; '&&' is an escaped ampersand; '& ' is literal.
    char *ampersand = strchr(text, '&');
    if (ampersand != nullptr && ampersand[1] != ' ') {
        items_[slot].text[0] = '\0';
        *ampersand = '\0';
        strcat(items_[slot].text, text);
        *ampersand = '&';
        if (ampersand[1] != '&') {
            items_[slot].mnemonic =
                items_[slot].text + strlen(items_[slot].text);
        }
        strcat(items_[slot].text, ampersand + 1);
    } else {
        strcat(items_[slot].text, text);
    }

    if (separator != nullptr) {
        items_[slot].right_text =
            static_cast<char *>(mem_get(strlen(separator + 1) + 1));
        if (items_[slot].right_text == nullptr) {
            items_[slot].id = -1;
            --item_count_;
            return 4;
        }
        items_[slot].right_text[0] = '\0';
        strcat(items_[slot].right_text, separator + 1);
        *separator = '|';
    }

    items_[slot].flags = 1;
    dirty_ = 1;
    compute_length(text);
    ++visible_count_;
    return 0;
}


/*
Purpose: Append a separator item to the end of the pull-down, and give it the
         requested ID. A full table answers 1, an unknown ID answers 3, and a
         slot whose text is still allocated answers 0xC.
// ORIGINAL: 0x005F8BE0 ?add_separator@PullDown@@QAEHH@Z 0x005F8BE0-0x005F8C81 BYTE_EXACT
// size      161 bytes
// prototype int (__thiscall ?add_separator@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   10   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0 when the separator was added, else the reason it was not
Status: Complete
*/
int PullDown::add_separator(int id) {
    const int new_count = ++item_count_;
    if (new_count > 64) {
        --item_count_;
        return 1;
    }
    if (id == -1) {
        return 3;
    }
    // Reuse the first free slot if there is one; the append is only where the
    // new item lands when the table had none.
    for (int i = 0; i < 64; ++i) {
        if (items_[i].id == -1) {
            const int slot = new_count - 1;
            if (items_[slot].text != nullptr) {
                return 0xC;
            }
            items_[slot].id = id;
            items_[slot].flags = 5;
            ++visible_count_;
            dirty_ = 1;
            return 0;
        }
    }
    return 1;
}


/*
Purpose: Replace the text of the first item carrying the requested ID,
         splitting and unescaping exactly as add_item does. An empty text
         answers 3, an unknown ID answers 0xB, and an allocation failure
         answers 4.
// ORIGINAL: 0x005F8E50 ?rename_item@PullDown@@QAEHHPAD@Z 0x005F8E50-0x005F903E BYTE_EXACT
// LEVER: items_[index].field directly, not through a slot pointer - same
//          lesson as add_item above, and here it is the difference between a
//          `lea esi,[ebp+eax*4]` with 0xa18 in the displacements and a
//          `lea esi,[ebp+eax*4]` with the struct offsets there.
// TRIED: NULL guard on `id` before the scan (add_item has one); rename_item
//          has no such guard and answers 3 on the text alone - adding it
//          costs the body its first epilogue.
// size      494 bytes
// prototype int (__thiscall ?rename_item@PullDown@@QAEHHPAD@Z)(PullDown* this, int, int8*)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005F9D30 0x00644EF2 0x006453E0 0x00645470 0x00645DD0
Return Value: 0 when the item was renamed, else the reason it was not
Status: WIP
*/
int PullDown::rename_item(int id, char *text) {
    if (text == nullptr) {
        return 3;
    }
    const int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }

    // Split the text at the '|' first, so both halves are measured from it.
    char *separator = strchr(text, '|');
    if (separator != nullptr) {
        *separator = '\0';
    }

    if (items_[index].text != nullptr) {
        free(items_[index].text);
        items_[index].text = nullptr;
    }
    if (items_[index].right_text != nullptr) {
        free(items_[index].right_text);
        items_[index].right_text = nullptr;
    }

    items_[index].text = static_cast<char *>(mem_get(strlen(text) + 1));
    if (items_[index].text == nullptr) {
        return 4;
    }
    items_[index].text[0] = '\0';
    items_[index].mnemonic = nullptr;

    // '&x' marks the mnemonic; '&&' is an escaped ampersand; '& ' is literal.
    char *ampersand = strchr(text, '&');
    if (ampersand != nullptr && ampersand[1] != ' ') {
        items_[index].text[0] = '\0';
        *ampersand = '\0';
        strcat(items_[index].text, text);
        *ampersand = '&';
        if (ampersand[1] != '&') {
            items_[index].mnemonic =
                items_[index].text + strlen(items_[index].text);
        }
        strcat(items_[index].text, ampersand + 1);
    } else {
        strcat(items_[index].text, text);
    }

    if (separator != nullptr) {
        items_[index].right_text =
            static_cast<char *>(mem_get(strlen(separator + 1) + 1));
        if (items_[index].right_text == nullptr) {
            return 4;
        }
        items_[index].right_text[0] = '\0';
        strcat(items_[index].right_text, separator + 1);
        *separator = '|';
    }

    compute_length(text);
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
    return (items_[selected_].flags & 2U) ? -1 : selected_;
}








PullDownFillCallback PullDownFieldF38Default;  // 0x009B7B58
PullDownBackfillCallback PullDownFieldF3CDefault;  // 0x009B7B5C
Font *PullDownDefaultFont;         // 0x009B7B68

// Palette indices shared by the popup policy and PullDown rendering.
// RECOVERY_OBJECT: v1 0x00696E08 ?PullDownDefaultTextColor@@3HA
int PullDownDefaultTextColor = 247;        // 0x00696E08
// RECOVERY_OBJECT: v1 0x00696E0C ?PullDownDefaultTextShadow@@3HA
int PullDownDefaultTextShadow = 255;       // 0x00696E0C
// RECOVERY_OBJECT: v1 0x00696E10 ?PullDownDefaultSelectedTextColor@@3HA
int PullDownDefaultSelectedTextColor = 7;  // 0x00696E10
// RECOVERY_OBJECT: v1 0x00696E14 ?PullDownDefaultSelectedTextShadow@@3HA
int PullDownDefaultSelectedTextShadow = 7; // 0x00696E14
// RECOVERY_OBJECT: v1 0x00696E18 ?PullDownDefaultDisabledTextColor@@3HA
int PullDownDefaultDisabledTextColor = 248;// 0x00696E18
// RECOVERY_OBJECT: v1 0x009B7B64 ?PullDownDefaultDisabledTextShadow@@3HA
int PullDownDefaultDisabledTextShadow;     // 0x009B7B64
// RECOVERY_OBJECT: v1 0x009B7B60 ?PullDownDefaultBackground@@3HA
int PullDownDefaultBackground;             // 0x009B7B60

// Shared menu geometry, initialized in the original image's .data.
// RECOVERY_OBJECT: v2 0x00696E04 0x00696E08 ?PullDownTextPadding@@3HA 0x005F9170
int PullDownTextPadding = 10;
// RECOVERY_OBJECT: v2 0x00696E1C 0x00696E20 ?PullDownBorderPadding@@3HA 0x005F9170
int PullDownBorderPadding = 2;
// These two reset words have no established event interpretation yet.
// RECOVERY_OBJECT: v2 0x009B7B70 0x009B7B74 ?PullDownState009B7B70@@3HA 0x005F9170
int PullDownState009B7B70;
// RECOVERY_OBJECT: v2 0x009B7B74 0x009B7B78 ?PullDownState009B7B74@@3HA 0x005F9170
int PullDownState009B7B74;
// Published only for the duration of the modal call at 0x005F929C.
// RECOVERY_OBJECT: v2 0x009B7B78 0x009B7B7C ?ActivePullDown@@3PAVPullDown@@A 0x005F9170
PullDown *ActivePullDown;

// ORIGINAL: 0x005F9170 ?show@PullDown@@QAEHHHP6AHXZ@Z 0x005F9170-0x005F9397
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005F9170-0x005F9397"], "reason": "Original modal menu behavior expressed through the GraphicWin base, item members and typed callback."}
int PullDown::show(int x, int y, int (*poll)()) {
    poll_ = poll;
    if (dirty_) {
        int width = width_ + 2 * (PullDownBorderPadding + PullDownTextPadding);
        int height = visible_count_ * PullDownDefaultFont->line_advance()
                   + 2 * PullDownBorderPadding;
        if (width != dib_.bmiHeader.biWidth || height != -dib_.bmiHeader.biHeight) {
            GraphicWin::resize(width, height, 0);
        }
    }
    selected_ = -1;
    if (win_parent_) {
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
        if (x + dib_.bmiHeader.biWidth > WinScreenWidth)
            x = WinScreenWidth - dib_.bmiHeader.biWidth;
        if (y - dib_.bmiHeader.biHeight > WinScreenHeight)
            y = WinScreenHeight + dib_.bmiHeader.biHeight;
    } else {
        ++x;
        ++y;
        if (x + dib_.bmiHeader.biWidth > WinScreenWidth)
            x -= dib_.bmiHeader.biWidth;
        if (y - dib_.bmiHeader.biHeight > WinScreenHeight)
            y += dib_.bmiHeader.biHeight;
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
    }
    PullDownState009B7B70 = 0;
    PullDownState009B7B74 = 0;
    Win::move(x, y);
    if (!win_parent_ && !WinPointerOwner3) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }
    ActivePullDown = this;
    set_modal(1, poll, 0);
    ActivePullDown = 0;
    if (!win_parent_) {
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) WinPointerOwner4 = 0;
    }
    Win::hide();
    do_all_draws();
    if (selected_ != -1 && menu_ && !(items_[selected_].flags & 2)) {
        menu_->menu_event(selected_ < 64 ? items_[selected_].id : -1);
    }
    return selected_;
}

// ORIGINAL: 0x005F8920 ?init@PullDown@@QAEXPAUMenu@@@Z 0x005F8920-0x005F8967 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005F8920-0x005F8967", "git:7c6c86d9:src/recovered/005f8920.cpp"], "reason": "Initialize the real GraphicWin base and disable markup through its Buffer subobject; Menu is a class in product source, not the retired seed struct spelling."}
// symbol    ?init@PullDown@@QAEXPAVMenu@@@Z
void PullDown::init(Menu *menu) {
    if (PullDownDefaultFont == 0) {
        PullDownDefaultFont = FontDefault;
    }
    menu_ = menu;
    GraphicWin::init(0, 0, 0, 0, 0, 0, 0, 0, 0);
    set_markup_enabled(0);
}

/*
Purpose: Destroy a PullDown by releasing every item's text pair, resetting
         the trailing fields from their global defaults, and delegating to
         the GraphicWin destructor.
// ORIGINAL: 0x005F88A0 ??1PullDown@@QAE@XZ 0x005F88A0-0x005F891B
// symbol    ??1PullDown@@UAE@XZ
// size      123 bytes
// prototype void (__thiscall ??1PullDown@@QAE@XZ)(PullDown* this)
// callers   27   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00644EF2
The compiler owns table staging and base teardown. Earlier destroy()-helper
experiments are retained in Git; they do not describe this real destructor.
*/
PullDown::~PullDown() {
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
    // GraphicWin is destroyed once by C++ after the owned strings.
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
Purpose: Resolve width_ to the widest item string, measuring the mnemonically
         stripped text plus twice the two-space separator plus the right half.
         A null text re-measures the whole table; a named text measures only
         that one.
// ORIGINAL: 0x005F9D30 ?compute_length@PullDown@@QAEHPAD@Z 0x005F9D30-0x005F9F34
// size      516 bytes
// prototype int (__thiscall ?compute_length@PullDown@@QAEHPAD@Z)(PullDown* this, int8*)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619280 0x006192F0 0x00645DD0
// WHAT IT IS: the lazy `PullDownDefaultFont` slot (0x009B7B68, resolved once
//          from FontDefault) plus the width of the widest item string. Each
//          arm is the same `width_ = max(width_, <measurement>)` with the max
//          written OUT as an if that re-evaluates its second operand - 16
//          calls to Font::width for 8 measurements, because the macro
//          re-evaluates exactly as Font::width(LPSTR, size_t)'s own documented
//          min does. The four `push imm32` string arguments are four distinct
//          instances of the same "  " literal at 0x696E20/24/28/2C, one per
//          macro expansion in this function.
// MISMATCH 7 of 162 (compiled 144) - RECEIVER-SPILL WALL, recognised by the
//          brief's fingerprint and not reached in four measured spellings.
//          The image opens `push ecx / push ebx / push ebp / mov ebp,ecx /
//          push esi / push edi / test / mov [esp+0x10],ebp`, so `this` lives
//          in a reserved stack slot and ebp is free as the accumulator in the
//          whole-table arm; it reloads it at 0x005F9EE6. This tree keeps
//          `this` in ebp for the whole body and never spills, so it is 18
//          instructions short and the accumulator lands in ebx instead.
// TRIED: `PullDownItem *item = &items_[i]` in the loop -> `items_[i].field`
//          (the add_item lesson), 5/162 -> 9/162.
// TRIED: hoisting `width_` into `const int current` before the width() calls
//          in both arms, matching the image's `mov edi,[ebp+0xf24]` before the
//          call rather than a folded `cmp [ebp+0xf24],eax` - moved the loop
//          but not the prologue.
// TRIED: reordering the summand to `2 * width("  ") + width(text) +
//          width(right)` to match the image's evaluation order (pipe, text,
//          right; ours was pipe, right, text) - no movement at all.
// TRIED: renaming via `PullDown::rename_item`'s shape, which DOES reproduce
//          the identical `push ecx` + `mov [esp+0x10],ebp` spill and is
//          BYTE_EXACT - so the spill is reachable from source, this body just
//          needs one more live register than this spelling supplies. Left
//          here rather than hand-spelled; see the note on rename_item.
Status: WIP
*/
int PullDown::compute_length(char *text) {
    if (PullDownDefaultFont == nullptr) {
        PullDownDefaultFont = FontDefault;
    }

    if (text != nullptr) {
        char *separator = strchr(text, '|');
        if (separator == nullptr) {
            const int current = width_;
            int measured = PullDownDefaultFont->width(text);
            if (current <= measured) {
                measured = PullDownDefaultFont->width(text);
            }
            width_ = measured;
            return width_;
        }

        const int previous = width_;
        if (previous <= 2 * PullDownDefaultFont->width("  ")
                            + PullDownDefaultFont->width(text, separator - text)
                            + PullDownDefaultFont->width(separator + 1)) {
            width_ = 2 * PullDownDefaultFont->width("  ")
                     + PullDownDefaultFont->width(text, separator - text)
                     + PullDownDefaultFont->width(separator + 1);
        } else {
            width_ = previous;
            return width_;
        }
    } else {
        width_ = 0;
        for (int i = 0; i < 64; ++i) {
            if (items_[i].text == nullptr) {
                continue;
            }
            const int current = width_;
            if (items_[i].right_text == nullptr) {
                int measured = PullDownDefaultFont->width(items_[i].text);
                if (current <= measured) {
                    measured = PullDownDefaultFont->width(items_[i].text);
                }
                width_ = measured;
            } else {
                int total = 2 * PullDownDefaultFont->width("  ")
                            + PullDownDefaultFont->width(items_[i].text)
                            + PullDownDefaultFont->width(items_[i].right_text);
                if (current > total) {
                    total = current;
                } else {
                    total = 2 * PullDownDefaultFont->width("  ")
                            + PullDownDefaultFont->width(items_[i].text)
                            + PullDownDefaultFont->width(items_[i].right_text);
                }
                width_ = total;
            }
        }
    }

    // BUG IN THE ORIGINAL: neither the whole-table walk above nor the
    // split-text branch falls through to a `return`, though the function is
    // declared int. The shipped bytes confirm it - the walk's epilogue leaves
    // EAX holding the exhausted loop counter (so it answers 0) and the split
    // branch's leaves EAX holding the last width() result - while the two
    // early exits do return width_ properly. Reproduced as written; adding
    // the missing return would be a different program.
    // (add_item and rename_item, its only callers, discard the result.)
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
    release_modal();
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

// The one base-constructor call in the image belongs to implicit C++ base
// initialization. Placement-constructing GraphicWin here ran it twice.
PullDown::PullDown() {
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
    poll_ = 0;
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

// ORIGINAL: 0x005F9460 ?on_right_down@PullDown@@QAEXHH@Z 0x005F9460-0x005F9472 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_right_down@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F946C
// 0x005F9460  ?on_right_down@PullDown@@QAEXHH@Z  ->  ?on_right_down@PullDown@@QAEXHH@Z
//
// Purpose: Pass a right-button press straight through to Win's virtual slot
//          20 - the image's `call dword ptr [eax + 0x50]` - where the
//          pull-down's owner decides what a right click means.


void PullDown::on_right_down(int x, int y) {
    vslot_20(x, y);
}

// ORIGINAL: 0x005F9570 ?on_right_up@PullDown@@QAEXHH@Z 0x005F9570-0x005F9582 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_right_up@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F957C
// 0x005F9570  ?on_right_up@PullDown@@QAEXHH@Z  ->  ?on_right_up@PullDown@@QAEXHH@Z
//
// Purpose: Pass a right-button release straight through to Win's virtual slot
//          21 - the image's `call dword ptr [eax + 0x54]` - the counterpart
//          of on_right_down one slot up.


void PullDown::on_right_up(int x, int y) {
    vslot_21(x, y);
}

// ORIGINAL: 0x005F9100 ?UNK4@PullDown@@QAEXXZ 0x005F9100-0x005F9165 BYTE_EXACT
// size      101 bytes
// prototype void (__thiscall ?UNK4@PullDown@@QAEXXZ)(PullDown* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x005F9100  ?UNK4@PullDown@@QAEXXZ  ->  ?UNK4@PullDown@@QAEXXZ
//
// Purpose: release every item's text pair and clear the item table, leaving
//          the pull-down with no items and the redraw latch set.


void PullDown::UNK4() {
    // THE IMAGE BASES ITS WALK POINTER ON THE ITEM'S SECOND MEMBER, not on
    // the item: `lea esi,[ebp+0xa1c]` is &items_[0].right_text, and every
    // access in the loop is a displacement off that pointer - [-4] text,
    // [0] right_text, [+4] id, [+8] flags. Basing a PullDownItem* on the
    // item instead shifts all four displacements by four bytes (same
    // mnemonics, wrong bytes), so the walk is spelled over the text slot.
    // cursor[1] and cursor[2] are the id and flags words that follow the two
    // owned strings in PullDownItem.
    char **cursor = &items_->right_text;
    int count = 0x40;
    do {
        if (cursor[-1] != nullptr) {
            free(cursor[-1]);
            cursor[-1] = nullptr;
        }
        if (cursor[0] != nullptr) {
            free(cursor[0]);
            cursor[0] = nullptr;
        }
        cursor[1] = reinterpret_cast<char *>(-1);
        cursor[2] = nullptr;
        cursor += sizeof(PullDownItem) / sizeof(char *);
    } while (--count != 0);
    field_F18_ = 0;
    item_count_ = 0;
    visible_count_ = 0;
    dirty_ = 1;
}
