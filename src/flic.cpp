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
#include "flic.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062A710
Return Value: n/a
Status: Complete
*/
void Flic::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062AAF0
Return Value: n/a
Status: Complete
*/
void Flic::UNK5() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062AB00
Return Value: n/a
Status: Complete
*/
void Flic::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062ACC0
Return Value: n/a
Status: Complete
*/
void Flic::UNK7() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062B5F0
Return Value: n/a
Status: Complete
*/
void Flic::UNK8() {
}

void __fastcall flic_unk4_redirect(Flic *self, void *) {
    self->UNK4();
}

void __fastcall flic_unk5_redirect(Flic *self, void *) {
    self->UNK5();
}

void __fastcall flic_unk6_redirect(Flic *self, void *) {
    self->UNK6();
}

void __fastcall flic_unk7_redirect(Flic *self, void *) {
    self->UNK7();
}

void __fastcall flic_unk8_redirect(Flic *self, void *) {
    self->UNK8();
}
