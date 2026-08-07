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

func_thiscall_teardown EffectElementTeardown =
    original_method<func_thiscall_teardown>(0x004482C0);

func_thiscall_teardown EffectElementCtor =
    original_method<func_thiscall_teardown>(0x004482D0);

/*
Purpose: Build the effect bank: the mirror image of the destructor below, one
         call handing the whole 0x61-element walk to the CRT vector
         constructor iterator with the bank itself as the array base. The
         destructor argument rides along for exception unwind, which cannot
         happen here but is passed faithfully. The 0x6C stride and 0x61 count
         are the same pair the destructor uses, read off the two `push`
         immediates at 0x004482AD and 0x004482AF - note the original pushes
         them count-then-size, so they arrive at the callee in the opposite
         order from the way they are written here.
Original Offset: 004482A0
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
*/
FX::FX() {
    VectorCtorIterator(this, 0x6C, 0x61, EffectElementCtor,
                       EffectElementTeardown);
}

FX *__fastcall fx_ctor_redirect(FX *self, void *) {
    return new (self) FX;
}

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

/*
Original Offset: 00446A90
Status: Complete
*/
void FX::stop(int a1) {
    reinterpret_cast<VCall *>(reinterpret_cast<char *>(this) + a1 * 0x6C)->slot008();
}

/*
Original Offset: 00446AB0
Status: Complete
*/
void FX::release(int a1) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self + a1 * 108)->slot014();
}

/*
Original Offset: 00446AD0
Status: Complete
*/
void FX::fade(int a1) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self)[a1 * 27].slot010();
}
