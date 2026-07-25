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
#include "sound.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 11.
Original Offset: 004C6430
Return Value: 11, always
Status: Complete
*/
int Sound::UNK1(int) {
    return 11;
}

/*
Purpose: Fade the sound out. The work is done by two of the object's own
         virtual methods: slot 0 is asked to fade with the given argument, and
         only when it declines (returns zero) is slot 0x28 run as the fallback.
Original Offset: 004C6600
Return Value: n/a
Status: Complete
*/
void Sound::fade(int a1) {
    // The object's own vtable is read at run time rather than declaring these
    // virtual, so the dispatch cannot disagree with the original's layout.
    uint8_t *const vtable = *reinterpret_cast<uint8_t **>(this);
    typedef int(__thiscall * fade_fn)(Sound *self, int a1);
    typedef void(__thiscall * fallback_fn)(Sound *self);
    if ((*reinterpret_cast<fade_fn *>(vtable))(this, a1) == 0) {
        uint8_t *const reread = *reinterpret_cast<uint8_t **>(this);
        (*reinterpret_cast<fallback_fn *>(reread + 0x28))(this);
    }
}

int __fastcall sound_unk1_redirect(Sound *self, void *, int a1) {
    return self->UNK1(a1);
}

void __fastcall sound_fade_redirect(Sound *self, void *, int a1) {
    self->fade(a1);
}
