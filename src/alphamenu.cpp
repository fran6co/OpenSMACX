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
#include "alphamenu.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x00403610 BYTE_EXACT
Return Value: 0
Status: Complete
*/
int AlphaMenu::requested_height() {
    return 0;
}

int __fastcall alpha_menu_requested_height_redirect(AlphaMenu *self, void *) {
    return self->requested_height();
}
