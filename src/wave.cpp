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

/*
Purpose: Release the loaded wave. The wrapped device, if there is one, is asked
         to unload through its own vtable slot 0x14 and its result becomes the
         return value; the device is then forgotten. Unless bit 1 of the flag
         byte at 0x54 suppresses it, the object's own vtable slot 0x80 is run,
         and the loaded bit (bit 0) of the flag dword at 0x40 is cleared.
Original Offset: 004C6EA0
Return Value: whatever the device's unload returned, or 0 when none was wrapped
Status: Complete
*/
int Wave::unload() {
    // Both dispatches read the live vtable pointer at run time rather than
    // being declared virtual, so neither can disagree with the original layout.
    typedef int(__thiscall * device_unload_fn)(void *device);
    typedef void(__thiscall * wave_vfn)(Wave *self);

    int result = 0;
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        result = (*reinterpret_cast<device_unload_fn *>(device_vtable + 0x14))(
            device_);
    }
    const uint8_t flags = flags_54_;
    device_ = nullptr;
    if (!(flags & 2)) {
        uint8_t *const vtable = *reinterpret_cast<uint8_t **>(this);
        (*reinterpret_cast<wave_vfn *>(vtable + 0x80))(this);
    }
    field_40_ &= 0xFFFFFFFEu;
    return result;
}

int __fastcall wave_unload_redirect(Wave *self, void *) {
    return self->unload();
}

/*
Purpose: Set the playback pitch, clamped to the range the engine accepts
         (-1200 to 1200). The clamped value is stored at 0x58 and handed to the
         wrapped device, if there is one, through its vtable slot 0x98.
Original Offset: 004C6EE0
Return Value: n/a
Status: Complete
*/
void Wave::set_pitch(int a1) {
    int pitch = a1;
    if (pitch < -1200) {
        pitch = -1200;
    } else if (pitch > 1200) {
        pitch = 1200;
    }
    pitch_ = pitch;
    if (device_) {
        typedef void(__thiscall * set_pitch_fn)(void *device, int pitch);
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        (*reinterpret_cast<set_pitch_fn *>(device_vtable + 0x98))(device_, pitch);
    }
}

/*
Purpose: Load a wave. The object's own vtable slot 0x88 does the loading; unless
         bit 2 of the second argument says otherwise, its own slot 0x8C then
         runs as the follow-up and supplies the result.
Original Offset: 004C6DB0
Return Value: slot 0x8C's result, or 0 when bit 2 of a2 skipped it
Status: Complete
*/
int Wave::load(int a1, int a2) {
    typedef void(__thiscall * load_fn)(Wave *self, int a1, int a2);
    typedef int(__thiscall * follow_fn)(Wave *self);

    uint8_t *const vtable = *reinterpret_cast<uint8_t **>(this);
    (*reinterpret_cast<load_fn *>(vtable + 0x88))(this, a1, a2);
    if (a2 & 4) {
        return 0;
    }
    uint8_t *const reread = *reinterpret_cast<uint8_t **>(this);
    return (*reinterpret_cast<follow_fn *>(reread + 0x8C))(this);
}

void __fastcall wave_set_pitch_redirect(Wave *self, void *, int a1) {
    self->set_pitch(a1);
}

int __fastcall wave_load_redirect(Wave *self, void *, int a1, int a2) {
    return self->load(a1, a2);
}

/*
Purpose: Report the wave's length in milliseconds, a plain read of the field
         at 0x60.
Original Offset: 004C6B00
Return Value: the stored length
Status: Complete
*/
int Wave::get_ms_length() {
    return ms_length_;
}

int __fastcall wave_get_ms_length_redirect(Wave *self, void *) {
    return self->get_ms_length();
}
