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
#include "original_seam.h"
#include "general.h"
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
    typedef int (OriginalObject::*device_unload_fn)();
    typedef void (OriginalObject::*wave_vfn)();

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
        typedef void (OriginalObject::*set_pitch_fn)(int pitch);
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
int Wave::load(int a1, uint32_t a2) {
    typedef void (OriginalObject::*load_fn)(int a1, int a2);
    typedef int (OriginalObject::*follow_fn)();

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

// With no wrapped device the wave is timed against the clock. The original
// reaches timeGetTime through `call dword ptr [0x669368]`, an indirect call
// on an import slot, so the seam is the address of that slot rather than the
// function: it reads the live pointer at run time exactly as the original
// does, and stays rebindable without linking winmm into the leaf tests.
func_time_get_time **WaveTimeGetTimeSlot =
    reinterpret_cast<func_time_get_time **>(0x00669368);

/*
Purpose: Report whether the wave is still sounding. A wrapped device answers
         for itself through its own vtable slot 0x5C; with no device the wave
         is timed against the clock instead.
Original Offset: 004C6B10
Return Value: nonzero while playing, 0 once finished or when not started
Status: Complete
*/
int Wave::is_playing() {
    // The device answers through its live vtable rather than a C++ virtual
    // call, and it is the receiver of that call - the original loads it into
    // ecx first, then dispatches on `[[ecx]+0x5C]`.
    typedef int (OriginalObject::*device_is_playing_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_is_playing_fn *>(
            device_vtable + 0x5C))(device_);
    }
    // `shr eax, 4` then `test al, 1` on the dword at 0x54: bit 4, which lives
    // in the low byte the header names.
    if (!(flags_54_ & 0x10)) {
        // The original merges here into a `test ecx, ecx` that can only fall
        // through, because ecx is the null device it already tested. Nothing
        // reachable follows it.
        return 0;
    }
    if (start_time_ == 0) {
        return 0;
    }
    const uint32_t elapsed = (*WaveTimeGetTimeSlot)() - start_time_;
    // `cmp ecx, eax` / `jbe`: the length is compared unsigned, so a negative
    // stored length reads as a very long one rather than as already finished.
    return static_cast<uint32_t>(ms_length_) > elapsed ? 1 : 0;
}

int __fastcall wave_is_playing_redirect(Wave *self, void *) {
    return self->is_playing();
}

/*
Purpose: Start the wave playing. A wrapped device answers through its own
         vtable slot 0x94, as the receiver, with the play argument passed on;
         its result is returned verbatim. With no device the answer is a
         fixed 0x14.
Original Offset: 004C68F0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::play(int a1) {
    typedef int (OriginalObject::*device_play_fn)(int a1);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_play_fn *>(device_vtable + 0x94))(
            device_, a1);
    }
    return 0x14;
}

int __fastcall wave_play_redirect(Wave *self, void *, int a1) {
    return self->play(a1);
}

// The wrapped-device forwarder family: each method lets the device at 0x3C
// answer through its own vtable slot, as the receiver, and returns a fixed
// answer when no device is wrapped. Only the slot, the argument list, and the
// no-device default vary.

/*
Purpose: Ask the wrapped device, through its vtable slot 0xC8, whether the
         buffer is in hardware.
Original Offset: 004C6AE0
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::is_hwbuffer() {
    typedef int (OriginalObject::*device_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xC8))(device_);
    }
    return 0;
}

int __fastcall wave_is_hwbuffer_redirect(Wave *self, void *) {
    return self->is_hwbuffer();
}

/*
Purpose: Ask the wrapped device for a time value through its vtable slot 0xB4,
         passing the query argument on.
Original Offset: 004C6FD0
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_time(uint32_t a1) {
    typedef int (OriginalObject::*device_fn)(uint32_t a1);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xB4))(device_,
                                                                      a1);
    }
    return 0;
}

int __fastcall wave_get_time_redirect(Wave *self, void *, uint32_t a1) {
    return self->get_time(a1);
}

/*
Purpose: Ask the wrapped device for the current marker through its vtable
         slot 0xB8.
Original Offset: 004C6FF0
Return Value: the device's answer, or -1 when no device is wrapped
Status: Complete
*/
int Wave::get_current_marker() {
    typedef int (OriginalObject::*device_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xB8))(device_);
    }
    return -1;
}

int __fastcall wave_get_current_marker_redirect(Wave *self, void *) {
    return self->get_current_marker();
}

/*
Purpose: Ask the wrapped device for the game window handle through its vtable
         slot 0x3C.
Original Offset: 004C7010
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_game_hwnd() {
    typedef int (OriginalObject::*device_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0x3C))(device_);
    }
    return 0;
}

int __fastcall wave_get_game_hwnd_redirect(Wave *self, void *) {
    return self->get_game_hwnd();
}

/*
Purpose: Ask the wrapped device for the device count through its vtable
         slot 0xBC.
Original Offset: 004C7020
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_ndevices() {
    typedef int (OriginalObject::*device_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xBC))(device_);
    }
    return 0;
}

int __fastcall wave_get_ndevices_redirect(Wave *self, void *) {
    return self->get_ndevices();
}

/*
Purpose: Unknown. The original chases the pointer chain that starts at the
         wrapped device and continues through each node's own 0x3C slot until
         it runs out, reads nothing else, and always answers 1. The argument
         is ignored.
Original Offset: 004C7080
Return Value: 1, always
Status: Complete
*/
int Wave::UNK1(int) {
    uint8_t *cursor = static_cast<uint8_t *>(device_);
    while (cursor) {
        cursor = *reinterpret_cast<uint8_t **>(cursor + 0x3C);
    }
    return 1;
}

int __fastcall wave_unk1_redirect(Wave *self, void *, int a1) {
    return self->UNK1(a1);
}

/*
Purpose: Set the reverb mix. The value is stored at 0x5C first, then the
         wrapped device is told through its vtable slot 0xE0.
Original Offset: 004C70A0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_reverb_mix(float a1) {
    reverb_mix_ = a1;
    if (device_) {
        typedef int (OriginalObject::*device_fn)(float a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0xE0))(device_, a1);
    }
    return 0x14;
}

int __fastcall wave_set_reverb_mix_redirect(Wave *self, void *, float a1) {
    return self->set_reverb_mix(a1);
}

/*
Purpose: Ask the wrapped device, through its vtable slot 0xDC, whether the
         wave is 3D positioned.
Original Offset: 004C7250
Return Value: the device's answer, or 0 when no device is wrapped (the
              original defines only AL on that path; callers test the byte)
Status: Complete
*/
int Wave::is_3d() {
    typedef int (OriginalObject::*device_fn)();
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xDC))(device_);
    }
    return 0;
}

int __fastcall wave_is_3d_redirect(Wave *self, void *) {
    return self->is_3d();
}

/*
Purpose: Describe a device into the caller's buffer through the wrapped
         device's vtable slot 0xC0. With no device the buffer is terminated
         to an empty string - but only when the third argument is nonzero,
         and without any null check on the buffer - and the answer is 1.
Original Offset: 004C7040
Return Value: the device's answer, or 1 when no device is wrapped
Status: Complete
*/
int Wave::get_device_description(char *a1, int a2, int a3) {
    typedef int (OriginalObject::*device_fn)(char *a1, int a2, int a3);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xC0))(
            device_, a1, a2, a3);
    }
    if (a3) {
        *a1 = '\0';
    }
    return 1;
}

int __fastcall wave_get_device_description_redirect(Wave *self, void *,
                                                    char *a1, int a2, int a3) {
    return self->get_device_description(a1, a2, a3);
}

/*
Purpose: Position the wave in 3D through the wrapped device's vtable
         slot 0xCC.
Original Offset: 004C7190
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_position3d(float a1, float a2, float a3) {
    typedef int (OriginalObject::*device_fn)(float a1, float a2, float a3);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xCC))(
            device_, a1, a2, a3);
    }
    return 0x14;
}

int __fastcall wave_set_position3d_redirect(Wave *self, void *, float a1,
                                            float a2, float a3) {
    return self->set_position3d(a1, a2, a3);
}

/*
Purpose: Set the wave's X position through the wrapped device's vtable
         slot 0xD0.
Original Offset: 004C71C0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_xpos(float a1) {
    typedef int (OriginalObject::*device_fn)(float a1);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xD0))(device_,
                                                                      a1);
    }
    return 0x14;
}

int __fastcall wave_set_xpos_redirect(Wave *self, void *, float a1) {
    return self->set_xpos(a1);
}

/*
Purpose: Set the wave's Y position through the wrapped device's vtable
         slot 0xD4.
Original Offset: 004C71F0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_ypos(float a1) {
    typedef int (OriginalObject::*device_fn)(float a1);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xD4))(device_,
                                                                      a1);
    }
    return 0x14;
}

int __fastcall wave_set_ypos_redirect(Wave *self, void *, float a1) {
    return self->set_ypos(a1);
}

/*
Purpose: Set the wave's Z position through the wrapped device's vtable
         slot 0xD8.
Original Offset: 004C7220
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_zpos(float a1) {
    typedef int (OriginalObject::*device_fn)(float a1);
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        return (*reinterpret_cast<device_fn *>(device_vtable + 0xD8))(device_,
                                                                      a1);
    }
    return 0x14;
}

int __fastcall wave_set_zpos_redirect(Wave *self, void *, float a1) {
    return self->set_zpos(a1);
}

/*
Purpose: Store the attribute mask into the wave's own fields, then tell the
         wrapped device through its vtable slot 0x6C. Bit 1 of the mask sets
         the dword at 0x30; the other bits map onto the flag byte at 0x54
         (bit 0 -> 1, bit 2 -> 2, bit 6 -> 8, bit 7 -> 0x10, and - only when
         bit 2 is clear - bit 4 -> 4 and bit 8 -> 0x20). Bits already set at
         0x54 are never cleared.
Original Offset: 004C6F20
Return Value: n/a
Status: Complete
*/
void Wave::set_attrib(uint32_t a1) {
    typedef int (OriginalObject::*device_fn)(uint32_t a1);
    if (a1 & 2) {
        field_30_ = 1;
    }
    if (a1 & 1) {
        flags_54_ |= 1;
    }
    if (a1 & 4) {
        flags_54_ |= 2;
    }
    if (a1 & 0x40) {
        flags_54_ |= 8;
    }
    if (a1 & 0x80) {
        flags_54_ |= 0x10;
    }
    if (!(a1 & 4) && (a1 & 0x10)) {
        flags_54_ |= 4;
    }
    if (!(a1 & 4) && (a1 & 0x100)) {
        flags_54_ |= 0x20;
    }
    if (device_) {
        uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
        (*reinterpret_cast<device_fn *>(device_vtable + 0x6C))(device_, a1);
    }
}

void __fastcall wave_set_attrib_redirect(Wave *self, void *, uint32_t a1) {
    self->set_attrib(a1);
}

/*
Purpose: Compose the attribute mask back out of the wave's own fields, OR-ed
         over whatever the wrapped device answers through its vtable
         slot 0x70 (0 with no device). The mapping inverts set_attrib's:
         the dword at 0x30 -> bit 1, and the 0x54 flag byte's bits
         1 -> 0, 8 -> 6, 2 -> 2, 4 -> 4, 0x10 -> 7, 0x20 -> 8.
Original Offset: 004C6F80
Return Value: the composed attribute mask
Status: Complete
*/
int Wave::get_attrib() {
    int result = 0;
    if (device_) {
        typedef int (OriginalObject::*device_fn)();
        result = (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x70))(device_);
    }
    if (field_30_) {
        result |= 2;
    }
    const uint8_t flags = flags_54_;
    if (flags & 1) {
        result |= 1;
    }
    if (flags & 8) {
        result |= 0x40;
    }
    if (flags & 2) {
        result |= 4;
    }
    if (flags & 4) {
        result |= 0x10;
    }
    if (flags & 0x10) {
        result |= 0x80;
    }
    if (flags & 0x20) {
        result |= 0x100;
    }
    return result;
}

int __fastcall wave_get_attrib_redirect(Wave *self, void *) {
    return self->get_attrib();
}

uint32_t *WaveDeviceGroupVolumes = reinterpret_cast<uint32_t *>(0x0090D9A0);
func_wave_device_is_group_disabled WaveDeviceIsGroupDisabled = original_method<func_wave_device_is_group_disabled>(0x004C5460);
func_wave_original_load WaveOriginalLoad = original_method<func_wave_original_load>(0x004C6CE0);

/*
Purpose: Set the wave's volume. The low seven bits of the argument are stored
         at 0x04; while the wave holds a device group slot the level handed
         to the device is rescaled by that group's own volume dword (one
         every 24 bytes in the singleton's table), as level/127 * group in
         double precision truncated back to an integer. The wrapped device,
         if any, hears the result through its vtable slot 0x40.
Original Offset: 004C7130
Return Value: n/a
Status: Complete
*/
void Wave::set_volume(int a1) {
    const uint32_t vol = static_cast<uint32_t>(a1) & 0x7F;
    volume_ = vol;
    int level = static_cast<int>(vol);
    if (group_slot_ < 0x10) {
        // The original loads the group dword zero-extended through a 64-bit
        // fild, so the scale is the UNSIGNED value of the table entry.
        const double group = static_cast<double>(WaveDeviceGroupVolumes[group_slot_ * 6]);
        level = static_cast<int>(static_cast<long long>(
            static_cast<double>(static_cast<int>(vol)) * (1.0 / 127.0) *
            group));
    }
    if (device_) {
        typedef void (OriginalObject::*device_fn)(int level);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x40))(device_, level);
    }
}

void __fastcall wave_set_volume_redirect(Wave *self, void *, int a1) {
    self->set_volume(a1);
}

/*
Purpose: Remember the wave's filename. The previous copy, if any, goes back
         to the game CRT heap first; the new string is measured, allocated on
         that same heap, recorded at 0x4C, and copied in - in that order, so
         the field already names the block while it is still being filled.
Original Offset: 004C6B60
Return Value: 0, or 0xA when the name is null
Status: Complete
*/
int Wave::set_fname(const char *a1) {
    if (!a1) {
        return 0xA;
    }
    if (fname_) {
        WaveOperatorDelete(fname_);
    }
    fname_ = WaveOperatorNew(strlen(a1) + 1);
    strcpy(static_cast<char *>(fname_), a1);
    return 0;
}

int __fastcall wave_set_fname_redirect(Wave *self, void *, const char *a1) {
    return self->set_fname(a1);
}

/*
Purpose: Start the wave. While it holds a device group slot, a disabled group
         answers 0x14 immediately (the original trusts only the answer's low
         byte); otherwise, when bit 4 of the 0x54 flag byte marks a clocked
         wave, the wave's own vtable slot 0x40 replays the stored volume. A
         wrapped device then starts through its slot 0x1C and its answer is
         the result; with no device, a clocked wave runs the original
         no-argument load and starts the device that load may have wrapped
         (answering 0 when it wrapped none), while an unclocked one answers
         0x14. Finally a clocked wave stamps the start time from the game's
         timeGetTime import, runs its own vtable slot 0x80, and forgets the
         device.
Original Offset: 004C6920
Return Value: the device's start answer, 0x14 when disabled or unstartable,
              0 when a clocked load wrapped no device
Status: Complete
*/
int Wave::play() {
    int result = 0;
    if (group_slot_ < 0x10) {
        if (static_cast<uint8_t>(
                (ORIGINAL(WaveDeviceGlobal)->*WaveDeviceIsGroupDisabled)(group_slot_))) {
            return 0x14;
        }
        if (flags_54_ & 0x10) {
            typedef void (OriginalObject::*wave_volume_fn)(uint32_t volume);
            (*reinterpret_cast<wave_volume_fn *>(
                *reinterpret_cast<uint8_t **>(this) + 0x40))(this, volume_);
        }
    }
    if (device_) {
        typedef int (OriginalObject::*device_start_fn)();
        result = (*reinterpret_cast<device_start_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x1C))(device_);
    } else if (flags_54_ & 0x10) {
        (ORIGINAL(this)->*WaveOriginalLoad)();
        if (device_) {
            typedef int (OriginalObject::*device_start_fn)();
            result = (*reinterpret_cast<device_start_fn *>(
                *reinterpret_cast<uint8_t **>(device_) + 0x1C))(device_);
        }
    } else {
        result = 0x14;
    }
    if (flags_54_ & 0x10) {
        start_time_ = (*WaveTimeGetTimeSlot)();
        (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x80)))();
        device_ = nullptr;
    }
    return result;
}

int __fastcall wave_play_empty_redirect(Wave *self, void *) {
    return self->play();
}

func_wave_device_create **WaveDeviceCreateSlot =
    reinterpret_cast<func_wave_device_create **>(0x0090DB24);
func_sound_original_load SoundOriginalLoad = original_method<func_sound_original_load>(0x004C6280);

/*
Purpose: Load the wave from its remembered filename. With no wrapped device
         yet, the creation hook - the slot beside the release hook, behind
         the same guard - builds one directly into the 0x3C field; a dead
         hook answers 1 and a failed creation its own error. The 0x54 flag
         byte and the wave's own vtable slot 0x58 fold into an attribute mask
         the device hears through its slot 0x6C, the base Sound::load reads
         the file, and on success the device reports the length in
         milliseconds through its slot 0xC4 into the field at 0x60.
Original Offset: 004C6CE0
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/load error
Status: Complete
*/
int Wave::load() {
    const char *const fname = static_cast<const char *>(fname_);
    if (!fname) {
        return 8;
    }
    if (!device_) {
        if (!*WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (*WaveDeviceCreateSlot)(&device_, fname, 1);
        if (created) {
            return created;
        }
    }
    int attribs = 0;
    if (flags_54_ & 1) {
        attribs |= 1;
    }
    {
        typedef int (OriginalObject::*wave_query_fn)();
        if ((*reinterpret_cast<wave_query_fn *>(
                *reinterpret_cast<uint8_t **>(this) + 0x58))(this)) {
            attribs |= 2;
        }
    }
    const uint8_t flags = flags_54_;
    if (flags & 4) {
        attribs |= 0x11;
    }
    if (flags & 8) {
        attribs |= 0x40;
        if (!(flags & 0x20)) {
            attribs |= 1;
        }
    }
    if (flags & 0x10) {
        attribs |= 0x80;
    }
    if (flags & 0x20) {
        attribs |= 0x100;
    }
    {
        typedef void (OriginalObject::*device_attrib_fn)(int attribs);
        (*reinterpret_cast<device_attrib_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x6C))(device_, attribs);
    }
    const int loaded = (ORIGINAL(this)->*SoundOriginalLoad)(fname);
    if (loaded) {
        return loaded;
    }
    typedef int (OriginalObject::*device_length_fn)();
    ms_length_ = (*reinterpret_cast<device_length_fn *>(
        *reinterpret_cast<uint8_t **>(device_) + 0xC4))(device_);
    return 0;
}

int __fastcall wave_load_empty_redirect(Wave *self, void *) {
    return self->load();
}

/*
Purpose: Reload the wave from its remembered filename. The device is created
         through the guarded hook when missing, hears the folded attribute
         mask (only the low two bits here) through its slot 0x6C, and
         reloads through its slot 0x84. On a first successful load the
         loaded bit of the 0x40 flag dword is set BEFORE the wave's own
         vtable slot 0x7C runs, and a nonzero dword at 0x30 additionally
         starts the device looping through its slot 0x48 with argument 1.
Original Offset: 004C6DF0
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/reload error
Status: Complete
*/
int Wave::reload() {
    const char *const fname = static_cast<const char *>(fname_);
    if (!fname) {
        return 8;
    }
    if (!device_) {
        if (!*WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (*WaveDeviceCreateSlot)(&device_, fname, 1);
        if (created) {
            return created;
        }
    }
    int attribs = 0;
    if (flags_54_ & 1) {
        attribs |= 1;
    }
    {
        typedef int (OriginalObject::*wave_query_fn)();
        if ((*reinterpret_cast<wave_query_fn *>(
                *reinterpret_cast<uint8_t **>(this) + 0x58))(this)) {
            attribs |= 2;
        }
    }
    {
        typedef void (OriginalObject::*device_attrib_fn)(int attribs);
        (*reinterpret_cast<device_attrib_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x6C))(device_, attribs);
    }
    typedef int (OriginalObject::*device_reload_fn)();
    const int reloaded = (*reinterpret_cast<device_reload_fn *>(
        *reinterpret_cast<uint8_t **>(device_) + 0x84))(device_);
    if (reloaded) {
        return reloaded;
    }
    if (!(field_40_ & 1)) {
        field_40_ |= 1;
        {
            typedef void (OriginalObject::*wave_vfn)();
            (*reinterpret_cast<wave_vfn *>(
                *reinterpret_cast<uint8_t **>(this) + 0x7C))(this);
        }
        if (field_30_) {
            typedef void (OriginalObject::*device_loop_fn)(int on);
            (*reinterpret_cast<device_loop_fn *>(
                *reinterpret_cast<uint8_t **>(device_) + 0x48))(device_, 1);
        }
    }
    return reloaded;
}

int __fastcall wave_reload_redirect(Wave *self, void *) {
    return self->reload();
}

/*
Purpose: Load the wave from in-memory data. Refuses (0xC) when a device is
         already wrapped; otherwise the guarded creation hook builds one from
         the data pointer. The device's vtable is captured BEFORE the wave's
         own slot 0x70 composes the attribute mask - the original dispatches
         the following slot 0x6C through that captured table on the re-read
         device - and the wave's own slot 0x7C finishes.
Original Offset: 004C6BB0
Return Value: 0 on success, 0xC with a device already wrapped, 1 with a dead
              creation hook, or the creation error
Status: Complete
*/
int Wave::dyna_load(char *a1) {
    if (device_) {
        return 0xC;
    }
    if (!*WaveDeviceReleaseGuard) {
        return 1;
    }
    const int created = (*WaveDeviceCreateSlot)(&device_, a1, 1);
    if (created) {
        return created;
    }
    uint8_t *const device_vtable = *reinterpret_cast<uint8_t **>(device_);
    const int attribs = (ORIGINAL(this)->*original_method<int (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x70)))();
    {
        typedef void (OriginalObject::*device_attrib_fn)(int attribs);
        (*reinterpret_cast<device_attrib_fn *>(device_vtable + 0x6C))(
            device_, attribs);
    }
    typedef void (OriginalObject::*wave_vfn)();
    (*reinterpret_cast<wave_vfn *>(
        *reinterpret_cast<uint8_t **>(this) + 0x7C))(this);
    return created;
}

int __fastcall wave_dyna_load_redirect(Wave *self, void *, char *a1) {
    return self->dyna_load(a1);
}

/*
Purpose: Load the wave from a caller-supplied filename. The guarded creation
         hook builds a device from that name when none is wrapped; the folded
         attribute mask (bits 0 and 1 plus the 0x40/0x80 mappings, without
         the companions the no-argument load adds) goes through the device's
         slot 0x6C; the base Sound::load reads the file; and on success the
         device answers the length through slot 0xC4 and then hears the
         stored volume, pitch, and the dword at 0x08 through its slots
         0x40, 0x98, and 0x44.
Original Offset: 004C6C20
Return Value: 0 on success, 1 with a dead creation hook, or the
              creation/load error
Status: Complete
*/
int Wave::load(const char *a1) {
    if (!device_) {
        if (!*WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (*WaveDeviceCreateSlot)(&device_, a1, 1);
        if (created) {
            return created;
        }
    }
    int attribs = 0;
    if (flags_54_ & 1) {
        attribs |= 1;
    }
    {
        typedef int (OriginalObject::*wave_query_fn)();
        if ((*reinterpret_cast<wave_query_fn *>(
                *reinterpret_cast<uint8_t **>(this) + 0x58))(this)) {
            attribs |= 2;
        }
    }
    const uint8_t flags = flags_54_;
    if (flags & 8) {
        attribs |= 0x40;
    }
    if (flags & 0x10) {
        attribs |= 0x80;
    }
    {
        typedef void (OriginalObject::*device_attrib_fn)(int attribs);
        (*reinterpret_cast<device_attrib_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x6C))(device_, attribs);
    }
    const int loaded = (ORIGINAL(this)->*SoundOriginalLoad)(a1);
    if (loaded) {
        return loaded;
    }
    {
        typedef int (OriginalObject::*device_length_fn)();
        ms_length_ = (*reinterpret_cast<device_length_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0xC4))(device_);
    }
    {
        typedef void (OriginalObject::*device_level_fn)(uint32_t v);
        (*reinterpret_cast<device_level_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x40))(device_, volume_);
    }
    {
        typedef void (OriginalObject::*device_pitch_fn)(int pitch);
        (*reinterpret_cast<device_pitch_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x98))(device_, pitch_);
    }
    typedef void (OriginalObject::*device_pan_fn)(uint32_t v);
    (*reinterpret_cast<device_pan_fn *>(
        *reinterpret_cast<uint8_t **>(device_) + 0x44))(device_, field_8_);
    return 0;
}

int __fastcall wave_load_fname_redirect(Wave *self, void *, const char *a1) {
    return self->load(a1);
}

func_sound_set_type SoundSetType = original_method<func_sound_set_type>(0x004C61E0);

/*
Purpose: Build the wave. The original constructs in the same three vtable
         stages the destructor tears down; between the stages only the CRT
         memset runs, so the two intermediate vtable installs are
         unobservable and - like the registered SEH frame - omitted, keeping
         only the final Wave vtable publication. A device dispatch of the
         0x3E8 default through slot 0 is provably dead (the device slot is
         zeroed a few instructions earlier with nothing between) and omitted
         under the same policy. The net field state: full volume, zeroed
         regions, a 1000ms default at 0x38, flag dword 4, Sound::set_type
         run with type 1, unit reverb mix, and the out-of-range group slot.
Original Offset: 004C66E0
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
*/
Wave::Wave() {
    volume_ = 0x7F;
    field_8_ = 0;
    for (size_t region_byte_index = 0;
         region_byte_index
             < sizeof(memset_region_) / sizeof(memset_region_[0]);
         ++region_byte_index) {
        uint8_t &region_byte = memset_region_[region_byte_index];
        region_byte = 0;
    }
    field_30_ = 0;
    chain_prev_ = nullptr;
    chain_next_ = nullptr;
    device_ = nullptr;
    fname_ = nullptr;
    // memset to zero, then the loaded bit cleared, then bit 2 set: net 4.
    field_40_ = 0;
    field_38_ = 0x3E8;
    field_50_ = 0;
    vtable_storage_ = 0x0066E44C;
    flags_54_ = 0;
    pad_55_[0] = 0;
    pad_55_[1] = 0;
    pad_55_[2] = 0;
    field_40_ |= 4;
    (ORIGINAL(this)->*SoundSetType)(1);
    pitch_ = 0;
    reverb_mix_ = 1.0f;
    group_slot_ = 0x10;
    ms_length_ = 0;
    start_time_ = 0;
}

Wave *__fastcall wave_ctor_redirect(Wave *self, void *) {
    return new (self) Wave;
}

/*
Purpose: Initialise the wave from a filename and a mode mask. Streaming waves
         (bit 2) refuse the bit-4 and bit-7 modes outright. The name resolves
         through the recovered filefind_get; an unresolvable name changes
         nothing. The resolved path replaces the remembered filename on the
         game heap and the 0x54 flag dword is cleared. A streaming wave marks
         flag bit 1 and, with no device yet and a live creation hook, builds
         one from the resolved path - capturing the device vtable before its
         own slot 0x70 composes the attribute mask, like dyna_load - then
         hands the RAW mode mask to whatever device exists through its slot
         0x6C. The mode bits then fold onto the flag byte as in set_attrib
         (bit 4 suppressed for streaming waves, bit 8 unsuppressed here), and
         bit 1 of the mode runs the wave's own vtable slot 0x48 with 1.
Original Offset: 004C69B0
Return Value: n/a
Status: Complete
*/
void Wave::init(char *a1, uint32_t a2) {
    const uint32_t streaming = a2 & 4;
    if (streaming && (a2 & 0x10)) {
        return;
    }
    if (streaming && (a2 & 0x80)) {
        return;
    }
    char *const resolved = filefind_get(a1);
    if (!resolved) {
        return;
    }
    if (fname_) {
        WaveOperatorDelete(fname_);
    }
    fname_ = WaveOperatorNew(strlen(resolved) + 1);
    strcpy(static_cast<char *>(fname_), resolved);
    for (uint8_t *flag_byte = &flags_54_; flag_byte != &flags_54_ + 4;
         ++flag_byte) {
        *flag_byte = 0;
    }
    if (streaming) {
        flags_54_ |= 2;
        if (!device_ && *WaveDeviceReleaseGuard) {
            const int created = (*WaveDeviceCreateSlot)(&device_, resolved, 1);
            if (!created) {
                uint8_t *const device_vtable =
                    *reinterpret_cast<uint8_t **>(device_);
                const int attribs =
                    (ORIGINAL(this)->*original_method<int (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x70)))();
                {
                    typedef void (OriginalObject::*device_attrib_fn)(int attribs);
                    (*reinterpret_cast<device_attrib_fn *>(
                        device_vtable + 0x6C))(device_, attribs);
                }
                (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x7C)))();
            }
        }
        if (device_) {
            typedef void (OriginalObject::*device_mode_fn)(uint32_t mode);
            (*reinterpret_cast<device_mode_fn *>(
                *reinterpret_cast<uint8_t **>(device_) + 0x6C))(device_, a2);
        }
    }
    if (a2 & 1) {
        flags_54_ |= 1;
    }
    if ((a2 & 0x10) && !streaming) {
        flags_54_ |= 4;
    }
    if (a2 & 2) {
        (ORIGINAL(this)->*original_method<void (OriginalObject::*)(int) >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x48)))(1);
    }
    if (a2 & 0x40) {
        flags_54_ |= 8;
    }
    if (a2 & 0x80) {
        flags_54_ |= 0x10;
    }
    if (a2 & 0x100) {
        flags_54_ |= 0x20;
    }
}

void __fastcall wave_init_redirect(Wave *self, void *, char *a1, uint32_t a2) {
    self->init(a1, a2);
}

/*
Purpose: The compiler-generated scalar deleting destructor: destroy the wave
         and, when bit 0 of the mode argument asks for it, return the storage
         to the game CRT heap.
Original Offset: 004C9300
Return Value: the object pointer
Status: Complete
*/
void *__fastcall wave_scalar_dtor_redirect(Wave *self, void *,
                                           unsigned int mode) {
    self->~Wave();
    if (mode & 1) {
        WaveOperatorDelete(self);
    }
    return self;
}

// The destructor's dependencies. pull_from_group is the Wave_Device method at
// 0x004C5280 with its singleton receiver at 0x0090D978; the buffer free goes
// to the game CRT's operator delete so the block returns to the heap that
// allocated it; the release hook is an indirect call on the slot at 0x0090DB28
// guarded by the dword at 0x0090DB7C; the chain end slots are the dwords the
// unlink falls back to when a neighbour is null.
func_wave_device_pull_from_group WaveDevicePullFromGroup = original_method<func_wave_device_pull_from_group>(0x004C5280);
void *WaveDeviceGlobal = reinterpret_cast<void *>(0x0090D978);
func_operator_delete *WaveOperatorDelete = (func_operator_delete *)0x0064557F;
func_wave_device_release **WaveDeviceReleaseSlot =
    reinterpret_cast<func_wave_device_release **>(0x0090DB28);
int *WaveDeviceReleaseGuard = reinterpret_cast<int *>(0x0090DB7C);
Wave **WaveChainHead = reinterpret_cast<Wave **>(0x0090DB20);
Wave **WaveChainTail = reinterpret_cast<Wave **>(0x0090DB1C);

/*
Purpose: Destroy the wave. The original is a three-stage teardown of an
         inlined hierarchy, republishing a vtable pointer at each stage: its
         own (0x0066E44C) at entry, the base's (0x0066E3C0) midway, and the
         ultimate base's (0x0066E444) on the way out. While the wave still
         holds a device group slot it is pulled from its group; the filename
         copy goes back to the game CRT heap; a chained wave unlinks itself
         from the global wave chain; the wrapped device is put through the
         release hook when the hook is live, then forgotten. The inlined base
         destructor then repeats the buffer free and the unlink - normally
         dead, but kept because an aliased chain neighbour or a re-linking
         release hook re-arms them, exactly as in the original. The registered
         SEH frame is omitted: the binary has no throw entry point, so it is
         unreachable.
Original Offset: 004C67C0
Return Value: n/a
Status: Complete
*/
Wave::~Wave() {
    // A destructor's member stores are dead to the optimizer once the object's
    // lifetime ends, so every access goes through a volatile view (the Texture
    // precedent, extended to the whole body): each store happens, in the
    // original's order, and each guard re-reads memory where the original does.
    Wave volatile *const self = this;
    self->vtable_storage_ = 0x0066E44C;
    if (self->group_slot_ < 0x10) {
        (ORIGINAL(WaveDeviceGlobal)->*WaveDevicePullFromGroup)(this);
    }
    void *const block = self->fname_;
    if (block) {
        WaveOperatorDelete(block);
    }
    self->fname_ = nullptr;
    if (self->field_40_ & 2) {
        // Unlink from the wave chain. The neighbour writes go through volatile
        // views too, and the second neighbour is re-read after the first write
        // because a neighbour pointer may alias this very object - the
        // original re-reads it the same way. The flag update folds the
        // original's early read of the word into the final store; the two
        // null stores between them cannot touch it.
        Wave *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<Wave volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            *WaveChainHead = self->chain_next_;
        }
        Wave *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<Wave volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            *WaveChainTail = self->chain_prev_;
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->field_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E3C0;
    // The inlined base destructor's copy of the free: reachable only when the
    // unlink above re-populated the slot through an aliased neighbour.
    void *const late_block = self->fname_;
    if (late_block) {
        WaveOperatorDelete(late_block);
        self->fname_ = nullptr;
    }
    void *const device = self->device_;
    if (device) {
        if (*WaveDeviceReleaseGuard) {
            (*WaveDeviceReleaseSlot)(device);
        }
        self->device_ = nullptr;
    }
    if (self->field_40_ & 2) {
        // The inlined base destructor's copy of the unlink: reachable only
        // when the release hook re-armed the chain bit.
        Wave *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<Wave volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            *WaveChainHead = self->chain_next_;
        }
        Wave *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<Wave volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            *WaveChainTail = self->chain_prev_;
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->field_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E444;
}

void __fastcall wave_dtor_redirect(Wave *self, void *) {
    self->~Wave();
}
