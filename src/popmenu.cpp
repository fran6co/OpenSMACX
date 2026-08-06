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
#include "popmenu.h"

func_base_pop_init BasePopOriginalInit = original_method<func_base_pop_init>(0x006015B0);

/*
Purpose: Initialise the pop-up menu with the base defaults.
Original Offset: 0059D3A0
Return Value: whatever BasePop::init returns
Status: Complete
*/
int PopMenu::init() {
    return (ORIGINAL(this)->*BasePopOriginalInit)(0, 0);
}

int __fastcall pop_menu_init_redirect(PopMenu *self, void *) {
    return self->init();
}
