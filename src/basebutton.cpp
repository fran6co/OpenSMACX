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
#include "temp.h"
#include "basebutton.h"

const uint32_t BaseButtonPrimaryVtable = 0x00670290;
const uint32_t BaseButtonBufferVtable = 0x00670288;
uint32_t *BaseButtonStaticDefaults = (uint32_t *)0x0069704C;
uint32_t *BaseButtonDynamicDefaults = (uint32_t *)0x009B8E2C;

/*
Purpose: Close the GraphicWin base, reset BaseButton-owned state from the
         process defaults, then release the owned name and bubble strings.
Original Offset: 006070C0
Return Value: Zero when no bubble string is released; otherwise the executable
              free routine's EAX residue
Status: Complete; string storage remains owned by the executable CRT
*/
uint32_t BaseButton::close() {
    GraphicWin::close();

    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile uint32_t *const fixed =
        reinterpret_cast<volatile uint32_t *>(BaseButtonStaticDefaults);
    volatile uint32_t *const dynamic =
        reinterpret_cast<volatile uint32_t *>(BaseButtonDynamicDefaults);
    object[0xA74 / 4] = 0;
    object[0xA9C / 4] = 0;
    object[0xA78 / 4] = 0;
    object[0xA44 / 4] = 0xFFFFFFFFU;
    object[0xA48 / 4] = 0xFFFFFFFFU;
    object[0xAAC / 4] = 0;
    object[0xAB0 / 4] = 0;
    object[0xAB4 / 4] = 0;
    object[0xA94 / 4] = dynamic[0];
    object[0xA84 / 4] = fixed[0];
    object[0xA88 / 4] = fixed[1];
    object[0xA8C / 4] = fixed[2];
    object[0xA90 / 4] = fixed[3];
    object[0xA98 / 4] = dynamic[1];
    object[0xAA4 / 4] = 0;
    object[0xAA0 / 4] = fixed[4];

    const uint32_t name = object[0xA7C / 4];
    if (name != 0) {
        _free(reinterpret_cast<void *>(static_cast<uintptr_t>(name)));
        object[0xA7C / 4] = 0;
    }
    uint32_t result = 0;
    const uint32_t bubble = object[0xA80 / 4];
    if (bubble != 0) {
        result = reinterpret_cast<uintptr_t>(_free(
            reinterpret_cast<void *>(static_cast<uintptr_t>(bubble))));
        object[0xA80 / 4] = 0;
    }
    object[0xAA8 / 4] = 0;
    return result;
}

uint32_t __fastcall base_button_close_redirect(BaseButton *self, void *) {
    return self->close();
}

/*
Purpose: Destroy a BaseButton by installing its two virtual tables, closing
         it, destroying Time2 then Time1, and finally destroying GraphicWin.
Original Offset: 00607040
Return Value: Instance pointer in EAX
Status: Complete
*/
BaseButton *BaseButton::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = BaseButtonPrimaryVtable;
    object[0x444 / 4] = BaseButtonBufferVtable;
    close();
    time2_.~Time();
    time1_.~Time();
    graphic_win_destructor_redirect(
        static_cast<GraphicWin *>(this), nullptr);
    return this;
}

BaseButton *__fastcall base_button_destructor_redirect(
        BaseButton *self, void *) {
    return self->destroy();
}

/*
Purpose: Set the button's bubble text.
Original Offset: 00607550
Return Value: Zero on success, non-zero on error
Status: Complete with redirect for free to prevent hang/freeze. Incompatibility between older
        version of free with newer SDK version of free CRT. Revisit once more of code is redirected
        to dll.
*/
int BaseButton::set_bubble_text(LPCSTR input) {
    if (bubble_text_) {
        _free(bubble_text_);
        bubble_text_ = 0;
    }
    if (input) {
        size_t len = strlen(input) + 1;
        bubble_text_ = (LPSTR)mem_get_old(len);
        if (!bubble_text_) {
            return 4;
        }
        strcpy_s(bubble_text_, len, input);
    }
    return 0;
}

/*
Purpose: Set the button's name string.
Original Offset: 006074E0
Return Value: Zero on success, non-zero on error
Status: Complete with redirect for free to prevent hang/freeze. Incompatibility between older
        version of free with newer SDK version of free CRT. Revisit once more of code is redirected
        to dll.
*/
int BaseButton::set_name(LPCSTR input) {
    // Bug fix: Fixed crash if input parameter was null. Original code had string copy outside last 
    // if statement causing potential write to null name variable.
    if (name_) {
        _free(name_);
        name_ = 0;
    }
    if (input) {
        size_t len = strlen(input) + 1;
        name_ = (LPSTR)mem_get_old(len);
        if (!name_) {
            return 4;
        }
        strcpy_s(name_, len, input);
    }
    return 0;
}
