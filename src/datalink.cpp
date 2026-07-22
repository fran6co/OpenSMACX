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
#include "datalink.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BEA0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BF00
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00431DB0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK9() {
}

void __fastcall datalink_unk6_redirect(Datalink *self, void *) {
    self->UNK6();
}

void __fastcall datalink_unk8_redirect(Datalink *self, void *) {
    self->UNK8();
}

void __fastcall datalink_unk9_redirect(Datalink *self, void *) {
    self->UNK9();
}
