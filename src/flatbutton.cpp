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
#include "flatbutton.h"

const uint32_t FlatButtonPrimaryVtable = 0x00669754;
const uint32_t FlatButtonBufferVtable = 0x0066974C;
uint32_t *FlatButtonDefaults = (uint32_t *)0x009B8E44;

/*
Purpose: Close the primary Time member, reset FlatButton orientation state,
         close the BaseButton base, and reload all nine three-entry tables.
ORIGINAL: 0x00607DA0
Return Value: Legacy final-loop residue (this + 0xAEC)
Status: Complete
*/
uint32_t FlatButton::close() {
    time1_.close();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0xA18 / 4] = 0;
    object[0xA14 / 4] = 0;
    object[0xAB8 / 4] = 0xFFFFFFFFU;
    BaseButton::close();

    volatile uint32_t *const defaults =
        reinterpret_cast<volatile uint32_t *>(FlatButtonDefaults);
    for (size_t index = 0; index < 3; ++index) {
        object[(0xABC / 4) + index] = 0xFFFFFFFFU;
        for (size_t group = 0; group < 9; ++group) {
            object[(0xAE0 / 4) + group * 3 + index] =
                defaults[group * 3 + index];
        }
        object[(0xAD4 / 4) + index] = 0;
        object[(0xAC8 / 4) + index] = 0;
    }
    return reinterpret_cast<uintptr_t>(this) + 0xAECU;
}

uint32_t __fastcall flat_button_close_redirect(FlatButton *self, void *) {
    return self->close();
}

/*
Purpose: Destroy a FlatButton by installing its two virtual tables, closing
         the derived stage, then running the complete BaseButton destructor.
ORIGINAL: 0x00406880
Return Value: Instance pointer in EAX
Status: Complete
*/
FlatButton *FlatButton::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = FlatButtonPrimaryVtable;
    object[0x444 / 4] = FlatButtonBufferVtable;
    close();
    BaseButton::destroy();
    return this;
}

FlatButton *__fastcall flat_button_destructor_redirect(
        FlatButton *self, void *) {
    return self->destroy();
}
