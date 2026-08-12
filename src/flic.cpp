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
ORIGINAL: 0x0062A710 BYTE_EXACT
// name      ?UNK4@Flic@@QAEXXZ
// size      1 bytes
// spans     0x0062A710-0x0062A711
// prototype void (__thiscall ?UNK4@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void Flic::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062AAF0 BYTE_EXACT
// name      ?UNK5@Flic@@QAEXXZ
// size      1 bytes
// spans     0x0062AAF0-0x0062AAF1
// prototype void (__thiscall ?UNK5@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void Flic::UNK5() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062AB00 BYTE_EXACT
// name      ?UNK6@Flic@@QAEXXZ
// size      1 bytes
// spans     0x0062AB00-0x0062AB01
// prototype void (__thiscall ?UNK6@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void Flic::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062ACC0 BYTE_EXACT
// name      ?UNK7@Flic@@QAEXXZ
// size      1 bytes
// spans     0x0062ACC0-0x0062ACC1
// prototype void (__thiscall ?UNK7@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void Flic::UNK7() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0062B5F0 BYTE_EXACT
// name      ?UNK8@Flic@@QAEXXZ
// size      1 bytes
// spans     0x0062B5F0-0x0062B5F1
// prototype void (__thiscall ?UNK8@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
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
