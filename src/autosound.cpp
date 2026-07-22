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
#include "autosound.h"

const uint32_t AutoSoundVtable = 0x0066FF34;
uint32_t *AutoSoundDefaults = reinterpret_cast<uint32_t *>(0x009BC080);

/*
Purpose: Construct an AutoSound by installing its virtual table and copying
         the complete process-default block in legacy store order.
Original Offset: 0062BA80
Status: Complete
*/
void AutoSound::construct() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const defaults = AutoSoundDefaults;
    object[0x00 / 4] = AutoSoundVtable;
    object[0x04 / 4] = defaults[0];
    object[0x0C / 4] = defaults[1];
    object[0x10 / 4] = defaults[2];
    object[0x08 / 4] = defaults[3];
    for (size_t index = 4; index < 37; ++index) {
        object[index + 1] = defaults[index];
    }
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

AutoSound *__fastcall auto_sound_construct_redirect(AutoSound *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: Legacy sound processing hook retained as a no-op.
Original Offset: 005FD2B0
Return Value: n/a
Status: Complete
*/
void __cdecl do_sound() {
}

int __cdecl do_sound_redirect() {
    do_sound();
    return 0;
}
