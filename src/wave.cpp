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
    typedef int(__thiscall * device_is_playing_fn)(void *device);
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
    typedef int(__thiscall * device_play_fn)(void *device, int a1);
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

// The destructor's dependencies. pull_from_group is the Wave_Device method at
// 0x004C5280 with its singleton receiver at 0x0090D978; the buffer free goes
// to the game CRT's operator delete so the block returns to the heap that
// allocated it; the release hook is an indirect call on the slot at 0x0090DB28
// guarded by the dword at 0x0090DB7C; the chain end slots are the dwords the
// unlink falls back to when a neighbour is null.
func_wave_device_pull_from_group *WaveDevicePullFromGroup = (func_wave_device_pull_from_group *)0x004C5280;
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
         holds a device group slot it is pulled from its group; the sample
         buffer goes back to the game CRT heap; a chained wave unlinks itself
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
        WaveDevicePullFromGroup(WaveDeviceGlobal, this);
    }
    void *const block = self->buffer_;
    if (block) {
        WaveOperatorDelete(block);
    }
    self->buffer_ = nullptr;
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
    void *const late_block = self->buffer_;
    if (late_block) {
        WaveOperatorDelete(late_block);
        self->buffer_ = nullptr;
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
