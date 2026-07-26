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
#include "fx.h"

func_thiscall_teardown *EffectElementTeardown =
    (func_thiscall_teardown *)0x004482C0;

/*
Purpose: Destroy the effect bank: hand the whole 0x61-element walk to the CRT
         vector iterator with the bank itself as the array base. Everything
         else is the per-element destructor's business.
Original Offset: 00448290
Return Value: n/a
Status: Complete
*/
FX::~FX() {
    VectorDtorIterator(this, 0x6C, 0x61, EffectElementTeardown);
}

void __fastcall fx_dtor_redirect(FX *self, void *) {
    self->~FX();
}
