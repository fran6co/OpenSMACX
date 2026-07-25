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
#include <cstring>

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

namespace {
// Sound wraps its device at 0x3C and asks it these questions through the
// device's own vtable; the original tail-jumps, so the device's answer is the
// caller's, and with no device wrapped the answer is zero.
typedef int(__thiscall *sound_device_query)(void *device);

int query_sound_device(Sound *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x3C);
    if (!device) {
        return 0;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (*reinterpret_cast<sound_device_query *>(vtable + vtable_offset))(
        device);
}
}  // namespace

/*
Purpose: Ask the wrapped device whether it is playing, through vtable slot 0x5C.
Original Offset: 004C64C0
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::is_playing() {
    return query_sound_device(this, 0x5C);
}

/*
Purpose: Ask the wrapped device whether it is looping, through vtable slot 0x58.
Original Offset: 004C6690
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::is_looping() {
    return query_sound_device(this, 0x58);
}

/*
Purpose: Ask the wrapped device for its play position, through vtable slot 0x74.
Original Offset: 004C66A0
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::get_time() {
    return query_sound_device(this, 0x74);
}

int __fastcall sound_is_playing_redirect(Sound *self, void *) {
    return self->is_playing();
}

int __fastcall sound_is_looping_redirect(Sound *self, void *) {
    return self->is_looping();
}

int __fastcall sound_get_time_redirect(Sound *self, void *) {
    return self->get_time();
}

namespace {
typedef int(__thiscall *sound_device_arg)(void *device, int a1);

int forward_sound_device(Sound *self, int vtable_offset, int a1,
                         int no_device_result) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x3C);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (*reinterpret_cast<sound_device_arg *>(vtable + vtable_offset))(
        device, a1);
}

int query_sound_device_default(Sound *self, int vtable_offset,
                               int no_device_result) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x3C);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (*reinterpret_cast<sound_device_query *>(vtable + vtable_offset))(
        device);
}
}  // namespace

/*
Purpose: Ask the wrapped device to play, through vtable slot 0x1C.
Original Offset: 004C6480
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play() {
    return query_sound_device_default(this, 0x1C, 0x14);
}

/*
Purpose: Ask the wrapped device to play the given sound, through vtable slot
         0x18.
Original Offset: 004C64A0
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play(unsigned int a1) {
    return forward_sound_device(this, 0x18, static_cast<int>(a1), 0x14);
}

/*
Purpose: Ask the wrapped device to stop, through vtable slot 0x20.
Original Offset: 004C64D0
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::stop() {
    return query_sound_device_default(this, 0x20, 0x14);
}

/*
Purpose: Ask the wrapped device to release, through vtable slot 0x38.
Original Offset: 004C64F0
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::release() {
    return query_sound_device_default(this, 0x38, 0x14);
}

/*
Purpose: Record the loop state at 0x30 and hand it to the wrapped device,
         through vtable slot 0x48. The field is written whether or not a device
         is wrapped.
Original Offset: 004C6540
Return Value: n/a
Status: Complete
*/
void Sound::set_loop_state(long a1) {
    const int value = static_cast<int>(a1);
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x30, &value, sizeof(value));
    forward_sound_device(this, 0x48, value, 0);
}

/*
Purpose: Record the delay at 0x34 and hand it to the wrapped device, through
         vtable slot 0x4C. The field is written whether or not a device is
         wrapped.
Original Offset: 004C6560
Return Value: n/a
Status: Complete
*/
void Sound::set_delay(unsigned int a1) {
    const int value = static_cast<int>(a1);
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x34, &value, sizeof(value));
    forward_sound_device(this, 0x4C, value, 0);
}

int __fastcall sound_play_redirect(Sound *self, void *) {
    return self->play();
}

int __fastcall sound_play_arg_redirect(Sound *self, void *, unsigned int a1) {
    return self->play(a1);
}

int __fastcall sound_stop_redirect(Sound *self, void *) {
    return self->stop();
}

int __fastcall sound_release_redirect(Sound *self, void *) {
    return self->release();
}

void __fastcall sound_set_loop_state_redirect(Sound *self, void *, long a1) {
    self->set_loop_state(a1);
}

void __fastcall sound_set_delay_redirect(Sound *self, void *, unsigned int a1) {
    self->set_delay(a1);
}
