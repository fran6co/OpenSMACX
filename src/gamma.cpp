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
#include "gamma.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
ORIGINAL: 0x005C9410
Return Value: 1, always
Status: Complete
*/
int Gamma::on_key_click(int, int) {
    return 1;
}

int __fastcall gamma_on_key_click_redirect(Gamma *self, void *, int a1, int a2) {
    return self->on_key_click(a1, a2);
}

func_gamma_adjust_palette GammaOriginalAdjustPalette =
    original_method<func_gamma_adjust_palette>(0x005C9520);

/*
ORIGINAL: 0x005C9390
Status: Complete
*/
void Gamma::on_scrolled(int a1, int a2) {
    (ORIGINAL(this)->*GammaOriginalAdjustPalette)();
}
