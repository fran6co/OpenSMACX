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
#include <algorithm>
#include "sound.h"
#include "general.h"
#include "wave.h"
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
void Sound::fade(uint32_t a1) {
    // The object's own vtable is read at run time rather than declaring these
    // virtual, so the dispatch cannot disagree with the original's layout.
    uint8_t *const vtable = *reinterpret_cast<uint8_t **>(this);
    typedef int (OriginalObject::*fade_fn)(int a1);
    typedef void (OriginalObject::*fallback_fn)();
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
typedef int (OriginalObject::*sound_device_query)();

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
typedef int (OriginalObject::*sound_device_arg)(int a1);

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

namespace {
// fade and fade_in are guarded twice: the field at 0x38 must be set as well as
// a device wrapped, and either being absent gives the same 0x13.
int guarded_query_sound_device(Sound *self, int vtable_offset) {
    uint8_t *const obj = reinterpret_cast<uint8_t *>(self);
    int gate;
    std::memcpy(&gate, obj + 0x38, sizeof(gate));
    if (gate == 0) {
        return 0x13;
    }
    void *device = *reinterpret_cast<void **>(obj + 0x3C);
    if (!device) {
        return 0x13;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (*reinterpret_cast<sound_device_query *>(vtable + vtable_offset))(
        device);
}
}  // namespace

/*
Purpose: Ask the wrapped device to fade, through vtable slot 0x28. Refuses
         unless the gate field at 0x38 is set and a device is wrapped.
Original Offset: 004C65E0
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade() {
    return guarded_query_sound_device(this, 0x28);
}

/*
Purpose: Ask the wrapped device to fade in, through vtable slot 0x30. Carries
         the same pair of guards fade does.
Original Offset: 004C6620
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade_in() {
    return guarded_query_sound_device(this, 0x30);
}

/*
Purpose: Hand a three-argument ramp to the wrapped device, through vtable slot
         0x34. Does nothing when no device is wrapped.
Original Offset: 004C6640
Return Value: n/a
Status: Complete
*/
void Sound::ramp(int a1, int a2, unsigned int a3) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(this) + 0x3C);
    if (!device) {
        return;
    }
    typedef void (OriginalObject::*ramp_fn)(int a1, int a2, int a3);
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    (*reinterpret_cast<ramp_fn *>(vtable + 0x34))(device, a1, a2,
                                                  static_cast<int>(a3));
}

int __fastcall sound_fade_query_redirect(Sound *self, void *) {
    return self->fade();
}

int __fastcall sound_fade_in_redirect(Sound *self, void *) {
    return self->fade_in();
}

void __fastcall sound_ramp_redirect(Sound *self, void *, int a1, int a2,
                                    unsigned int a3) {
    self->ramp(a1, a2, a3);
}

/*
Purpose: Record the sound's type. Types 1..7 - except 3, which the original's
         jump table routes to the invalid arm - store the type at 0x50 and OR
         a per-type class bit into the flag dword at 0x40: 1 -> 4, 2 -> 8,
         4 -> 0x10, 5 -> 0x28, 6 -> 0x100, 7 -> 0x80. Anything else stores
         type 0 and leaves the flags alone.
Original Offset: 004C61E0
Return Value: n/a
Status: Complete
*/
void Sound::set_type(unsigned int a1) {
    uint32_t class_bit;
    switch (a1) {
    case 1:
        class_bit = 4;
        break;
    case 2:
        class_bit = 8;
        break;
    case 4:
        class_bit = 0x10;
        break;
    case 5:
        class_bit = 0x28;
        break;
    case 6:
        class_bit = 0x100;
        break;
    case 7:
        class_bit = 0x80;
        break;
    default:
        type_ = 0;
        return;
    }
    type_ = a1;
    flags_40_ |= class_bit;
}

void __fastcall sound_set_type_redirect(Sound *self, void *, unsigned int a1) {
    self->set_type(a1);
}

/*
Purpose: Load the sound from a filename. The name resolves through the
         recovered filefind_get (0xA when it cannot); a dead creation-hook
         guard answers 1; a missing device is built from the resolved path
         through the guarded hook, while an existing one is first asked
         through its vtable slot 0x60 whether it is busy (0xF if so). The
         device loads the resolved path through its slot 0x10. On a first
         success the loaded bit of the 0x40 flag dword is set BEFORE the
         sound's own vtable slot 0x7C runs, and a nonzero loop dword at 0x30
         starts the device looping through its slot 0x48; on failure the
         loaded bit is cleared. Either way the resolved path is copied onto
         the game heap - new copy first, old one freed after - and remembered
         at 0x4C.
Original Offset: 004C6280
Return Value: the device's load answer, 0xA for an unresolvable name, 1 for
              a dead creation hook, 0xF for a busy device, or the creation
              error
Status: Complete
*/
int Sound::load(const char *a1) {
    char *const resolved = filefind_get(a1);
    if (!resolved) {
        return 0xA;
    }
    if (!*WaveDeviceReleaseGuard) {
        return 1;
    }
    if (!device_) {
        const int created = (*WaveDeviceCreateSlot)(&device_, resolved, 1);
        if (created) {
            return created;
        }
    } else {
        typedef int (OriginalObject::*device_busy_fn)();
        if ((*reinterpret_cast<device_busy_fn *>(
                *reinterpret_cast<uint8_t **>(device_) + 0x60))(device_)) {
            return 0xF;
        }
    }
    int result;
    {
        typedef int (OriginalObject::*device_load_fn)(const char *name);
        result = (*reinterpret_cast<device_load_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x10))(device_, resolved);
    }
    if (result == 0) {
        if (!(flags_40_ & 1)) {
            flags_40_ |= 1;
            (*reinterpret_cast<void(__thiscall **)(Sound *)>(
                *reinterpret_cast<uint8_t **>(this) + 0x7C))(this);
            if (loop_flag_30_) {
                typedef void (OriginalObject::*device_loop_fn)(int on);
                (*reinterpret_cast<device_loop_fn *>(
                    *reinterpret_cast<uint8_t **>(device_) + 0x48))(device_,
                                                                    1);
            }
        }
    } else {
        flags_40_ &= ~1u;
    }
    char *const copy =
        static_cast<char *>(WaveOperatorNew(strlen(resolved) + 1));
    strcpy(copy, resolved);
    if (fname_) {
        WaveOperatorDelete(fname_);
    }
    fname_ = copy;
    return result;
}

int __fastcall sound_load_redirect(Sound *self, void *, const char *a1) {
    return self->load(a1);
}

/*
Purpose: Set the sound's volume: the low seven bits are stored at 0x04 and
         the wrapped device, if any, hears them through its vtable slot 0x40.
         Unlike Wave's override there is no group rescaling here.
Original Offset: 004C6510
Return Value: n/a
Status: Complete
*/
void Sound::set_volume(int a1) {
    const uint32_t vol = static_cast<uint32_t>(a1) & 0x7F;
    volume_ = vol;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(uint32_t vol);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x40))(device_, vol);
    }
}

void __fastcall sound_set_volume_redirect(Sound *self, void *, int a1) {
    self->set_volume(a1);
}

/*
Purpose: Set the fade time. Zero is refused with 0xA; otherwise the value is
         remembered at 0x38 and the wrapped device hears it through its
         vtable slot 0.
Original Offset: 004C6580
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade(unsigned long a1) {
    if (!a1) {
        return 0xA;
    }
    fade_38_ = a1;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(unsigned long t);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0))(device_, a1);
    }
    return 0;
}

int __fastcall sound_set_fade_redirect(Sound *self, void *, unsigned long a1) {
    return self->set_fade(a1);
}

/*
Purpose: Set the fade-in time. Zero is refused with 0xA; otherwise the value
         shares the 0x38 field with set_fade and the wrapped device hears it
         through its vtable slot 0x54.
Original Offset: 004C65B0
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade_in(unsigned int a1) {
    if (!a1) {
        return 0xA;
    }
    fade_38_ = a1;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(unsigned int t);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x54))(device_, a1);
    }
    return 0;
}

int __fastcall sound_set_fade_in_redirect(Sound *self, void *,
                                          unsigned int a1) {
    return self->set_fade_in(a1);
}

/*
Purpose: Fade the sound in: its own vtable slot 0x54 takes the time, and only
         a zero answer lets its own slot 0x28 follow up.
Original Offset: 004C6660
Return Value: n/a
Status: Complete
*/
void Sound::fade_in(unsigned int a1) {
    typedef int (OriginalObject::*own_time_fn)(unsigned int t);
    if ((*reinterpret_cast<own_time_fn *>(
            *reinterpret_cast<uint8_t **>(this) + 0x54))(this, a1) == 0) {
        (*reinterpret_cast<void(__thiscall **)(Sound *)>(
            *reinterpret_cast<uint8_t **>(this) + 0x28))(this);
    }
}

void __fastcall sound_fade_in_arg_redirect(Sound *self, void *,
                                           unsigned int a1) {
    self->fade_in(a1);
}

/*
Purpose: Set the pan, clamped to the range the engine accepts (-0x40 to
         0x3F), stored at 0x08 and handed to the wrapped device through its
         vtable slot 0x44.
Original Offset: 004C66B0
Return Value: n/a
Status: Complete
*/
void Sound::set_pan(int a1) {
    const int pan = std::min(std::max(a1, -0x40), 0x3F);
    pan_8_ = pan;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(int pan);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x44))(device_, pan);
    }
}

void __fastcall sound_set_pan_redirect(Sound *self, void *, int a1) {
    self->set_pan(a1);
}

/*
Purpose: Release the loaded sound. The wrapped device, if any, is asked to
         unload through its own vtable slot 0x14 and its answer is the
         result; the sound's own vtable slot 0x80 then runs UNCONDITIONALLY
         (Wave's override suppresses it by flag; the base does not), the
         device is forgotten, and the loaded bit at 0x40 is cleared.
Original Offset: 004C6440
Return Value: whatever the device's unload returned, or 0 with none wrapped
Status: Complete
*/
int Sound::unload() {
    int result = 0;
    if (device_) {
        typedef int (OriginalObject::*device_fn)();
        result = (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_) + 0x14))(device_);
    }
    (*reinterpret_cast<void(__thiscall **)(Sound *)>(
        *reinterpret_cast<uint8_t **>(this) + 0x80))(this);
    const uint32_t cleared = flags_40_ & ~1u;
    device_ = nullptr;
    flags_40_ = cleared;
    return result;
}

int __fastcall sound_unload_redirect(Sound *self, void *) {
    return self->unload();
}

/*
Purpose: Destroy the sound. This is exactly the body ~Wave inlines as its
         base stage: publish the Sound vtable, return the filename copy to
         the game heap (cleared only when there was one), put the wrapped
         device through the guarded release hook and forget it, unlink from
         the sound chain, and publish the ultimate base's vtable on the way
         out. The registered SEH frame is omitted as unreachable.
Original Offset: 004C6120
Return Value: n/a
Status: Complete
*/
Sound::~Sound() {
    // The whole body runs through a volatile view: a destructor's trailing
    // member stores are otherwise dead to the optimizer (the Wave precedent).
    Sound volatile *const self = this;
    self->vtable_storage_ = 0x0066E3C0;
    {
        void *const name = self->fname_;
        if (name) {
            WaveOperatorDelete(name);
            self->fname_ = nullptr;
        }
    }
    void *const device = self->device_;
    if (device) {
        if (*WaveDeviceReleaseGuard) {
            (*WaveDeviceReleaseSlot)(device);
        }
        self->device_ = nullptr;
    }
    if (self->flags_40_ & 2) {
        Sound *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<Sound volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            *WaveChainHead = reinterpret_cast<Wave *>(self->chain_next_);
        }
        Sound *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<Sound volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            *WaveChainTail = reinterpret_cast<Wave *>(self->chain_prev_);
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->flags_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E444;
}

void __fastcall sound_dtor_redirect(Sound *self, void *) {
    self->~Sound();
}

/*
Purpose: The compiler-generated scalar deleting destructor: destroy the
         sound and, when bit 0 of the mode asks, free the storage to the
         game heap.
Original Offset: 004C92D0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall sound_scalar_dtor_redirect(Sound *self, void *,
                                            unsigned int mode) {
    self->~Sound();
    if (mode & 1) {
        WaveOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Join the sound chain at the tail. A sound with either neighbour
         already set is left alone. An empty chain makes it both ends;
         otherwise the old tail becomes its prev - re-read after the tail
         slot advances, as the original orders it - and learns its next.
         Either way the chained bit sets.
Original Offset: 004C6370
Return Value: 0, always
Status: Complete
*/
int Sound::attach() {
    if (chain_next_ || chain_prev_) {
        return 0;
    }
    // The original re-zeroes both links in each arm; the guard above proves
    // they are already null, so those stores are omitted as unobservable.
    if (!*WaveChainTail) {
        *WaveChainHead = reinterpret_cast<Wave *>(this);
        *WaveChainTail = reinterpret_cast<Wave *>(this);
        flags_40_ |= 2;
        return 0;
    }
    chain_prev_ = reinterpret_cast<Sound *>(*WaveChainTail);
    *WaveChainTail = reinterpret_cast<Wave *>(this);
    chain_prev_->chain_next_ = this;
    flags_40_ |= 2;
    return 0;
}

int __fastcall sound_attach_redirect(Sound *self, void *) {
    return self->attach();
}

/*
Purpose: Leave the sound chain: nothing at all for an unchained sound;
         otherwise the standard unlink with the head and tail slots
         maintained at the ends, both links cleared, and the chained bit
         dropped.
Original Offset: 004C63D0
Return Value: 0, always
Status: Complete
*/
int Sound::detach() {
    if (!(flags_40_ & 2)) {
        return 0;
    }
    Sound *const prev = chain_prev_;
    if (prev) {
        prev->chain_next_ = chain_next_;
    } else {
        *WaveChainHead = reinterpret_cast<Wave *>(chain_next_);
    }
    Sound *const next = chain_next_;
    if (next) {
        next->chain_prev_ = chain_prev_;
    } else {
        *WaveChainTail = reinterpret_cast<Wave *>(chain_prev_);
    }
    chain_next_ = nullptr;
    flags_40_ &= ~2u;
    chain_prev_ = nullptr;
    return 0;
}

int __fastcall sound_detach_redirect(Sound *self, void *) {
    return self->detach();
}
