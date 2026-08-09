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
#include "checkbutton.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x006339B0
Return Value: n/a
Status: Complete
*/
void CheckButton::close_class() {
}

void __cdecl check_button_close_class_redirect() {
    CheckButton::close_class();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x006339A0
Return Value: 0, always
Status: Complete
*/
int CheckButton::init_class() {
    return 0;
}

int __cdecl check_button_init_class_redirect() {
    return CheckButton::init_class();
}
