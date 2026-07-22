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
#include "wave.h"

/*
Purpose: Returns a fixed 11; the legacy implementation is a single constant load.
Original Offset: 004C70E0
Return Value: 11, always
Status: Complete
*/
int Wave::set_asdr() {
    return 11;
}

int __fastcall wave_set_asdr_redirect(Wave *self, void *) {
    return self->set_asdr();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
Original Offset: 004C70D0
Return Value: 11, always
Status: Complete
*/
int Wave::set_bufflimit(unsigned int) {
    return 11;
}

int __fastcall wave_set_bufflimit_redirect(Wave *self, void *, unsigned int a1) {
    return self->set_bufflimit(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
Original Offset: 004C70F0
Return Value: 11, always
Status: Complete
*/
int Wave::set_attack(unsigned int, unsigned int, unsigned int) {
    return 11;
}

int __fastcall wave_set_attack_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3) {
    return self->set_attack(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
Original Offset: 004C7100
Return Value: 11, always
Status: Complete
*/
int Wave::set_sustain(unsigned int, unsigned int, unsigned int) {
    return 11;
}

int __fastcall wave_set_sustain_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3) {
    return self->set_sustain(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
Original Offset: 004C7110
Return Value: 11, always
Status: Complete
*/
int Wave::set_decay(unsigned int, unsigned int, unsigned int) {
    return 11;
}

int __fastcall wave_set_decay_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3) {
    return self->set_decay(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
Original Offset: 004C7120
Return Value: 11, always
Status: Complete
*/
int Wave::set_release(unsigned int, unsigned int, unsigned int) {
    return 11;
}

int __fastcall wave_set_release_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3) {
    return self->set_release(a1, a2, a3);
}
