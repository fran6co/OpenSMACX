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
#include "sound.h"
#include "general.h"
#include "wave.h"
#include "wave_device.h"    // Wave_Device in init_sound
#include "sounddevice.h"    // Midi_Device / Wave_In_Device in init_sound
#include "fx.h"
#include "dialog.h"
#include "sounddevice.h"
#include "net_class.h"
#include "basepop.h"
#include <cstring>

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 11.
// ORIGINAL: 0x004C6430 ?UNK1@Sound@@QAEHH@Z 0x004C6430-0x004C6438 BYTE_EXACT
// symbol    ?UNK1@Sound@@UAEHH@Z
// size      8 bytes
// prototype int (__thiscall ?UNK1@Sound@@QAEHH@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004C6600 ?fade@Sound@@QAEXK@Z 0x004C6600-0x004C6620 BYTE_EXACT
// symbol    ?fade@Sound@@UAEXK@Z
// size      32 bytes
// prototype void (__thiscall ?fade@Sound@@QAEXK@Z)(Sound* this, unsigned int)
// callers   11   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C660E 0x004C6618
Return Value: n/a
Status: Complete
*/
void Sound::fade(unsigned long a1) {
    // The object's own vtable is read at run time rather than declaring these
    // virtual, so the dispatch cannot disagree with the original's layout.
    // `vtable_method` leaves the call operand where it lives - the image's
    // `call dword ptr [eax]` / `call dword ptr [edx + 0x28]` - where
    // `original_slot` read the slot into a register first.
    typedef int (OriginalObject::*fade_fn)(int a1);
    typedef void (OriginalObject::*fallback_fn)();
    if ((ORIGINAL(this)->*vtable_method<fade_fn>(this, 0))(a1) == 0) {
        (ORIGINAL(this)->*vtable_method<fallback_fn>(this, 0x28))();
    }
}



namespace {
// Sound wraps its device at 0x3C and asks it these questions through the
// device's own vtable; the original tail-jumps, so the device's answer is the
// caller's, and with no device wrapped the answer is zero.
// ONE parameter, `__fastcall`: the receiver goes in ecx and edx is left
// alone. A second would cost a `xor edx, edx` the image does not have.
typedef int(__fastcall *sound_device_query)(void *);

__forceinline int query_sound_device(Sound *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x3C);
    // THE ZERO PATH LAST, as the image lays it out: `je` jumps FORWARD to
    // `xor eax, eax; ret` at 0x004C64CC. A guard clause puts it first and
    // inverts the branch.
    //
    // CALLED WHERE THE SLOT LIVES: the image tail-jumps through it -
    // `jmp dword ptr [eax+0x5c]` at 0x004C64C9 - and reading the slot into a
    // pointer-to-member first costs a `mov` before the call.
    if (device) {
        return vtable_slot<sound_device_query>(device, vtable_offset)(device);
    }
    return 0;
}
}  // namespace

/*
Purpose: Ask the wrapped device whether it is playing, through vtable slot 0x5C.
// ORIGINAL: 0x004C64C0 ?is_playing@Sound@@QAEHXZ 0x004C64C0-0x004C64CF BYTE_EXACT
// symbol    ?is_playing@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?is_playing@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::is_playing() {
    return query_sound_device(this, 0x5C);
}

/*
Purpose: Ask the wrapped device whether it is looping, through vtable slot 0x58.
// ORIGINAL: 0x004C6690 ?is_looping@Sound@@QAEHXZ 0x004C6690-0x004C669F BYTE_EXACT
// symbol    ?is_looping@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?is_looping@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::is_looping() {
    return query_sound_device(this, 0x58);
}

/*
Purpose: Ask the wrapped device for its play position, through vtable slot 0x74.
// ORIGINAL: 0x004C66A0 ?get_time@Sound@@QAEHXZ 0x004C66A0-0x004C66AF BYTE_EXACT
// symbol    ?get_time@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?get_time@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// VIRTUAL since 2026-08-30: slot 29 of Sound's table (0x0066E3C0) is this
// function in the image, between the pure pair and unk_slot30. Virtualness
// spells U, not Q, in the mangled name - hence the symbol fact. The body is
// unchanged; the claim re-measured BYTE_EXACT under it.
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::get_time() {
    return query_sound_device(this, 0x74);
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
    return (ORIGINAL(device)->*original_slot<sound_device_arg>(vtable + vtable_offset))(a1);
}

}  // namespace

/*
Purpose: Ask the wrapped device to play, through vtable slot 0x1C.
// ORIGINAL: 0x004C6480 ?play@Sound@@QAEHXZ 0x004C6480-0x004C6492 BYTE_EXACT
// symbol    ?play@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?play@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play() {
    void *device = device_;
    int result = 0x14;
    if (device) {
        result = vtable_slot<sound_device_query>(device, 0x1C)(device);
    }
    return result;
}

/*
Purpose: Ask the wrapped device to play the given sound, through vtable slot
         0x18.
// ORIGINAL: 0x004C64A0 ?play@Sound@@QAEHI@Z 0x004C64A0-0x004C64BC
// symbol    ?play@Sound@@UAEHI@Z
// size      28 bytes
// prototype int (__thiscall ?play@Sound@@QAEHI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C64B5
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play(unsigned int effect) {
    // TRIED writing this out of forward_sound_device the way play()/stop()/
    // release() were - that is what made THOSE byte-exact - but the argument
    // is what differs: the image passes it thiscall-on-the-stack, and both
    // spellings reached for here compile 13-15 instructions against the
    // image's 12 (3/12, against 2/12 through the helper). The no-argument
    // family is fixed; this one needs its own shape.
    return forward_sound_device(this, 0x18, static_cast<int>(effect), 0x14);
}

/*
Purpose: Ask the wrapped device to stop, through vtable slot 0x20.
// ORIGINAL: 0x004C64D0 ?stop@Sound@@QAEHXZ 0x004C64D0-0x004C64E2 BYTE_EXACT
// symbol    ?stop@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?stop@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::stop() {
    void *device = device_;
    int result = 0x14;
    if (device) {
        result = vtable_slot<sound_device_query>(device, 0x20)(device);
    }
    return result;
}

/*
Purpose: Ask the wrapped device to release, through vtable slot 0x38.
// ORIGINAL: 0x004C64F0 ?release@Sound@@QAEHXZ 0x004C64F0-0x004C6502 BYTE_EXACT
// symbol    ?release@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?release@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::release() {
    void *device = device_;
    int result = 0x14;
    if (device) {
        result = vtable_slot<sound_device_query>(device, 0x38)(device);
    }
    return result;
}

/*
Purpose: Record the loop state at 0x30 and hand it to the wrapped device,
         through vtable slot 0x48. The field is written whether or not a device
         is wrapped.
// ORIGINAL: 0x004C6540 ?set_loop_state@Sound@@QAEXJ@Z 0x004C6540-0x004C655A
// symbol    ?set_loop_state@Sound@@UAEXJ@Z
// size      26 bytes
// prototype void (__thiscall ?set_loop_state@Sound@@QAEXJ@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6553
Return Value: n/a
Status: Complete
*/
void Sound::set_loop_state(long loop_state) {
    const int value = static_cast<int>(loop_state);
    loop_flag_30_ = value;
    forward_sound_device(this, 0x48, value, 0);
}

/*
Purpose: Record the delay at 0x34 and hand it to the wrapped device, through
         vtable slot 0x4C. The field is written whether or not a device is
         wrapped.
// ORIGINAL: 0x004C6560 ?set_delay@Sound@@QAEXI@Z 0x004C6560-0x004C657A
// symbol    ?set_delay@Sound@@UAEXI@Z
// size      26 bytes
// prototype void (__thiscall ?set_delay@Sound@@QAEXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6573
Return Value: n/a
Status: Complete
*/
void Sound::set_delay(unsigned int delay) {
    const int value = static_cast<int>(delay);
    delay_ = value;
    forward_sound_device(this, 0x4C, value, 0);
}







namespace {
}  // namespace

/*
Purpose: Ask the wrapped device to fade, through vtable slot 0x28. Refuses
         unless the gate field at 0x38 is set and a device is wrapped.
// ORIGINAL: 0x004C65E0 ?fade@Sound@@QAEHXZ 0x004C65E0-0x004C65F9 BYTE_EXACT
// symbol    ?fade@Sound@@UAEHXZ
// size      25 bytes
// prototype int (__thiscall ?fade@Sound@@QAEHXZ)(Sound* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade() {
    if (fade_38_ == 0) {
        return 0x13;
    }
    void *device = device_;
    if (!device) {
        return 0x13;
    }
    return vtable_slot<sound_device_query>(device, 0x28)(device);
}

/*
Purpose: Ask the wrapped device to fade in, through vtable slot 0x30. Carries
         the same pair of guards fade does.
// ORIGINAL: 0x004C6620 ?fade_in@Sound@@QAEHXZ 0x004C6620-0x004C6639 BYTE_EXACT
// symbol    ?fade_in@Sound@@UAEHXZ
// size      25 bytes
// prototype int (__thiscall ?fade_in@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade_in() {
    if (fade_38_ == 0) {
        return 0x13;
    }
    void *device = device_;
    if (!device) {
        return 0x13;
    }
    return vtable_slot<sound_device_query>(device, 0x30)(device);
}

/*
Purpose: Hand a three-argument ramp to the wrapped device, through vtable slot
         0x34. Does nothing when no device is wrapped.
// ORIGINAL: 0x004C6640 ?ramp@Sound@@QAEXHHI@Z 0x004C6640-0x004C665F BYTE_EXACT
// symbol    ?ramp@Sound@@UAEXHHI@Z
// size      31 bytes
// prototype void (__thiscall ?ramp@Sound@@QAEXHHI@Z)(Sound* this, int, int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6658
Return Value: n/a
Status: Complete
*/
void Sound::ramp(int a1, int a2, unsigned int a3) {
    // The wrapped device, named: it is this class's own member, not an
    // offset to be walked.
    if (!device_) {
        return;
    }
    typedef void (OriginalObject::*ramp_fn)(int a1, int a2, int a3);
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device_);
    (ORIGINAL(device_)->*original_slot<ramp_fn>(vtable + 0x34))(a1, a2, static_cast<int>(a3));
}




/*
Purpose: Record the sound's type. Types 1..7 - except 3, which the original's
         jump table routes to the invalid arm - store the type at 0x50 and OR
         a per-type class bit into the flag dword at 0x40: 1 -> 0x10, 2 -> 8,
         4 -> 4, 5 -> 0x28, 6 -> 0x100, 7 -> 0x80. Anything else stores
         type 0 and leaves the flags alone.
// ORIGINAL: 0x004C61E0 ?set_type@Sound@@QAEXI@Z 0x004C61E0-0x004C6260 BYTE_EXACT
// symbol    ?set_type@Sound@@UAEXI@Z
// size      128 bytes
// prototype void (__thiscall ?set_type@Sound@@QAEXI@Z)(Sound* this, SOUNDTYPE)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER, two parts. (1) each `case` inlines its own `type_ = a1;
//        flags_40_ |= N; return;` rather than sharing a `class_bit` local
//        and a common tail - the image duplicates the four-instruction
//        block per arm with no merged epilogue. (2) the CATALOGUED mapping
//        1->4 / 4->0x10 is backwards: the jump table's own byte order says
//        1->0x10 and 4->4 - `constant #9/#21 or` swapped is what measure
//        named it, and swapping the two immediates is what closed it.
Return Value: n/a
Status: Complete
*/
void Sound::set_type(unsigned int type) {
    // EACH ARM INLINE, not a shared `type_ = type; flags_40_ |= class_bit;`
    // tail: the image duplicates the store-and-OR in every case block and
    // returns from each directly, rather than merging them.
    switch (type) {
    case 1:
        flags_40_ |= 0x10;
        type_ = type;
        return;
    case 2:
        flags_40_ |= 8;
        type_ = type;
        return;
    case 4:
        flags_40_ |= 4;
        type_ = type;
        return;
    case 5:
        flags_40_ |= 0x28;
        type_ = type;
        return;
    case 6:
        flags_40_ |= 0x100;
        type_ = type;
        return;
    case 7:
        flags_40_ |= 0x80;
        type_ = type;
        return;
    default:
        type_ = 0;
        return;
    }
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
// ORIGINAL: 0x004C6280 ?load@Sound@@QAEHPBD@Z 0x004C6280-0x004C6361 SEMANTIC
// symbol    ?load@Sound@@UAEHPBD@Z
// LEVER: the two vtable dispatches (slots 0x60 and 0x10) spelled with `vtable_method<Fn>(object, offset)` instead of `original_slot<Fn>(vtable + offset)` - single indirect call, matching the image's `call dword ptr [reg+N]` where the slot-read idiom cost an extra `mov`. Took this from 44/102 to 100/102.
// TRIED: the one remaining byte is the slot-0x60 dispatch alone - image loads the vtable into EDX (`mov edx,[ecx]; call [edx+0x60]`), this tree into EAX. Naming the method pointer in its own local first made it much WORSE (47/102) rather than fixing the register; reverted. Not chased further.
// size      225 bytes
// prototype int (__thiscall ?load@Sound@@QAEHPBD@Z)(Sound* this, int8*)
// callers   5   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006005D0 0x006453E0 0x00645460 0x0064557F 0x0064558A
// indirect  0x004C62C9 0x004C62DF 0x004C62F7 0x004C6311 0x004C6321
Return Value: the device's load answer, 0xA for an unresolvable name, 1 for
              a dead creation hook, 0xF for a busy device, or the creation
              error
Status: Complete
*/
int Sound::load(const char *fname) {
    char *const resolved = filefind_get(fname);
    if (!resolved) {
        return 0xA;
    }
    if (!WaveDeviceReleaseGuard) {
        return 1;
    }
    if (!device_) {
        const int created = (WaveDeviceCreateSlot())(&device_, resolved, 1);
        if (created) {
            return created;
        }
    } else {
        typedef int (OriginalObject::*device_busy_fn)();
        if ((ORIGINAL(device_)->*vtable_method<device_busy_fn>(device_, 0x60))()) {
            return 0xF;
        }
    }
    int result;
    {
        typedef int (OriginalObject::*device_load_fn)(const char *name);
        result = (ORIGINAL(device_)->*vtable_method<device_load_fn>(device_, 0x10))(
            resolved);
    }
    if (result == 0) {
        if (!(flags_40_ & 1)) {
            flags_40_ |= 1;
            (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x7C)))();
            if (loop_flag_30_) {
                typedef void (OriginalObject::*device_loop_fn)(int on);
                (ORIGINAL(device_)->*original_slot<device_loop_fn>(*reinterpret_cast<uint8_t **>(device_) + 0x48))(
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
        operator delete(fname_);
    }
    fname_ = copy;
    return result;
}


/*
Purpose: Set the sound's volume: the low seven bits are stored at 0x04 and
         the wrapped device, if any, hears them through its vtable slot 0x40.
         Unlike Wave's override there is no group rescaling here.
// ORIGINAL: 0x004C6510 ?set_volume@Sound@@QAEXH@Z 0x004C6510-0x004C6532
// symbol    ?set_volume@Sound@@UAEXH@Z
// size      34 bytes
// prototype void (__thiscall ?set_volume@Sound@@QAEXH@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C652B
Return Value: n/a
Status: Complete
*/
void Sound::set_volume(int volume) {
    const uint32_t vol = static_cast<uint32_t>(volume) & 0x7F;
    volume_ = vol;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(uint32_t vol);
        (ORIGINAL(device_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_) + 0x40))(
            vol);
    }
}


/*
Purpose: Set the fade time. Zero is refused with 0xA; otherwise the value is
         remembered at 0x38 and the wrapped device hears it through its
         vtable slot 0.
// ORIGINAL: 0x004C6580 ?set_fade@Sound@@QAEHK@Z 0x004C6580-0x004C65A8 BYTE_EXACT
// symbol    ?set_fade@Sound@@UAEHK@Z
// size      40 bytes
// prototype int (__thiscall ?set_fade@Sound@@QAEHK@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C65A0
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade(unsigned long fade) {
    if (!fade) {
        return 0xA;
    }
    fade_38_ = fade;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(unsigned long t);
        (ORIGINAL(device_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_) + 0))(
            fade);
    }
    return 0;
}


/*
Purpose: Set the fade-in time. Zero is refused with 0xA; otherwise the value
         shares the 0x38 field with set_fade and the wrapped device hears it
         through its vtable slot 0x54.
// ORIGINAL: 0x004C65B0 ?set_fade_in@Sound@@QAEHI@Z 0x004C65B0-0x004C65D9 BYTE_EXACT
// symbol    ?set_fade_in@Sound@@UAEHI@Z
// size      41 bytes
// prototype int (__thiscall ?set_fade_in@Sound@@QAEHI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C65D0
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade_in(unsigned int fade_in) {
    if (!fade_in) {
        return 0xA;
    }
    fade_38_ = fade_in;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(unsigned int t);
        (ORIGINAL(device_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_) + 0x54))(
            fade_in);
    }
    return 0;
}


/*
Purpose: Fade the sound in: its own vtable slot 0x54 takes the time, and only
         a zero answer lets its own slot 0x28 follow up.
// ORIGINAL: 0x004C6660 ?fade_in@Sound@@QAEXI@Z 0x004C6660-0x004C6681
// symbol    ?fade_in@Sound@@UAEXI@Z
// size      33 bytes
// prototype void (__thiscall ?fade_in@Sound@@QAEXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C666E 0x004C6679
Return Value: n/a
Status: Complete
*/
void Sound::fade_in(unsigned int a1) {
    typedef int (OriginalObject::*own_time_fn)(unsigned int t);
    if ((ORIGINAL(this)->*original_slot<own_time_fn>(*reinterpret_cast<uint8_t **>(this) + 0x54))(
        a1) == 0) {
        (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x28)))();
    }
}


/*
Purpose: Set the pan, clamped to the range the engine accepts (-0x40 to
         0x3F), stored at 0x08 and handed to the wrapped device through its
         vtable slot 0x44.
// ORIGINAL: 0x004C66B0 ?set_pan@Sound@@QAEXH@Z 0x004C66B0-0x004C66E0 BYTE_EXACT
// symbol    ?set_pan@Sound@@UAEXH@Z
// size      48 bytes
// prototype void (__thiscall ?set_pan@Sound@@QAEXH@Z)(Sound* this, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C66D9
Return Value: n/a
Status: Complete
*/
void Sound::set_pan(int a1) {
    // Written out rather than with std::min/std::max: <windows.h> defines min
    // and max as function-like macros, and VC6's STL spells the templates
    // _cpp_min/_cpp_max, so neither name is usable here.
    const int pan = a1 < -0x40 ? -0x40 : (a1 > 0x3F ? 0x3F : a1);
    pan_8_ = pan;
    if (device_) {
        typedef void (OriginalObject::*device_fn)(int pan);
        (ORIGINAL(device_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_) + 0x44))(
            pan);
    }
}


/*
Purpose: Release the loaded sound. The wrapped device, if any, is asked to
         unload through its own vtable slot 0x14 and its answer is the
         result; the sound's own vtable slot 0x80 then runs UNCONDITIONALLY
         (Wave's override suppresses it by flag; the base does not), the
         device is forgotten, and the loaded bit at 0x40 is cleared.
// ORIGINAL: 0x004C6440 ?unload@Sound@@QAEHXZ 0x004C6440-0x004C6472
// symbol    ?unload@Sound@@UAEHXZ
// size      50 bytes
// prototype int (__thiscall ?unload@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C644F 0x004C6458
Return Value: whatever the device's unload returned, or 0 with none wrapped
Status: Complete
*/
int Sound::unload() {
    int result = 0;
    if (device_) {
        typedef int (OriginalObject::*device_fn)();
        result = (ORIGINAL(device_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_) + 0x14))();
    }
    (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 0x80)))();
    const uint32_t cleared = flags_40_ & ~1u;
    device_ = nullptr;
    flags_40_ = cleared;
    return result;
}


/*
Purpose: Destroy the sound. This is exactly the body ~Wave inlines as its
         base stage: publish the Sound vtable, return the filename copy to
         the game heap (cleared only when there was one), put the wrapped
         device through the guarded release hook and forget it, unlink from
         the sound chain, and publish the ultimate base's vtable on the way
         out. The registered SEH frame is omitted as unreachable.
// ORIGINAL: 0x004C6120 ??1Sound@@QAE@XZ 0x004C6120-0x004C61D4;0x004C8450-0x004C8457;0x00659EE2-0x00659EF4
// symbol    ??1Sound@@UAE@XZ
// size      205 bytes
// prototype void (__thiscall ??1Sound@@QAE@XZ)(Sound* this)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C616E
Return Value: n/a
Status: Complete
*/
Sound::~Sound() {
    // The whole body runs through a volatile view: a destructor's trailing
    // member stores are otherwise dead to the optimizer (the Wave precedent).
    Sound volatile *const self = this;
    {
        void *const name = self->fname_;
        if (name) {
            operator delete(name);
            self->fname_ = nullptr;
        }
    }
    void *const device = self->device_;
    if (device) {
        if (WaveDeviceReleaseGuard) {
            (WaveDeviceReleaseSlot())(device);
        }
        self->device_ = nullptr;
    }
    if (self->flags_40_ & 2) {
        Sound *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<Sound volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            WaveChainHead() = reinterpret_cast<Wave *>(self->chain_next_);
        }
        Sound *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<Sound volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            WaveChainTail() = reinterpret_cast<Wave *>(self->chain_prev_);
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->flags_40_ &= ~2u;
    }
}


/*
Purpose: The compiler-generated scalar deleting destructor: destroy the
         sound and, when bit 0 of the mode asks, free the storage to the
         game heap.
// ORIGINAL: 0x004C92D0 ??_GSound@@UAEPAXI@Z 0x004C92D0-0x004C92F1
// symbol    ?sound_scalar_dtor_redirect@@YIPAXPAVSound@@PAXI@Z
// size      33 bytes
// prototype void* (__thiscall ??_GSound@@UAEPAXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6120 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall sound_scalar_dtor_redirect(Sound *self, void *,
                                            unsigned int mode) {
    self->~Sound();
    if (mode & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Join the sound chain at the tail. A sound with either neighbour
         already set is left alone. An empty chain makes it both ends;
         otherwise the old tail becomes its prev - re-read after the tail
         slot advances, as the original orders it - and learns its next.
         Either way the chained bit sets.
// ORIGINAL: 0x004C6370 ?attach@Sound@@QAEHXZ 0x004C6370-0x004C63C1
// symbol    ?attach@Sound@@UAEHXZ
// TRIED 2026-08-30, after the slot reconciliation (the image's vftable slot
//   29 is get_time 0x4C66A0 and slot 31 is attach - the old header had them
//   swapped with detach at 31, and detach 0x4C63D0 is not in the vftable at
//   all): now MEASURABLE, this body shows the image's persistent zero
//   register - `xor edx,edx` materialised once, then `cmp eax,edx` and
//   `cmp [ecx+0x44],edx` - where every source spelling emits `test`. Same
//   wall class as sub_63f9b0 (vox2 batch): 5/28, the virtualization made it
//   measurable, the zero-register plan awaits the flag-axis/emitter-level
//   investigation.
// size      81 bytes
// prototype int (__thiscall ?attach@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Sound::attach() {
    if (chain_next_ || chain_prev_) {
        return 0;
    }
    // The original re-zeroes both links in each arm; the guard above proves
    // they are already null, so those stores are omitted as unobservable.
    if (!WaveChainTail()) {
        WaveChainHead() = reinterpret_cast<Wave *>(this);
        WaveChainTail() = reinterpret_cast<Wave *>(this);
        flags_40_ |= 2;
        return 0;
    }
    chain_prev_ = reinterpret_cast<Sound *>(WaveChainTail());
    WaveChainTail() = reinterpret_cast<Wave *>(this);
    chain_prev_->chain_next_ = this;
    flags_40_ |= 2;
    return 0;
}


/*
Purpose: Leave the sound chain: nothing at all for an unchained sound;
         otherwise the standard unlink with the head and tail slots
         maintained at the ends, both links cleared, and the chained bit
         dropped.
// ORIGINAL: 0x004C63D0 ?detach@Sound@@QAEHXZ 0x004C63D0-0x004C642C BYTE_EXACT
// symbol    ?detach@Sound@@UAEHXZ
// size      92 bytes
// prototype int (__thiscall ?detach@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
        WaveChainHead() = reinterpret_cast<Wave *>(chain_next_);
    }
    Sound *const next = chain_next_;
    if (next) {
        next->chain_prev_ = chain_prev_;
    } else {
        WaveChainTail() = reinterpret_cast<Wave *>(chain_prev_);
    }
    chain_next_ = nullptr;
    flags_40_ &= ~2u;
    chain_prev_ = nullptr;
    return 0;
}



/*
// ORIGINAL: 0x004C5CE0 ?init_sound@@YAHPAXK@Z 0x004C5CE0-0x004C5D8E
// size      174 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C4F40 0x004C57A0 0x004C5A10 0x004C5E50
//
// Promoted 2026-08-15 from src/unrecovered/004c5ce0.cpp to retire its
// pending_bodies forwarder. Brings up the wave device, then the midi and
// wave-in devices per the backend bitmask. The driver-unload and FreeLibrary
// calls are read out of their fixed slots, as the original does.
Status: Complete
*/
typedef void(__cdecl *ZeroArgFn)(int, int);

static int *const g_0090d950 = (int *)0x0090D950;
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db2c = (int *)0x0090DB2C;

int __cdecl init_sound(void *window, unsigned long backends) {
    int loadResult = load_sound_dll();
    if (loadResult != 0) {
        return loadResult;
    }
    if (SoundDllModule() != 0) {
        (*reinterpret_cast<ZeroArgFn *>(g_0090db2c))(0, 0);
    }
    int result = WaveDeviceGlobal.init(window, backends);
    if (result != 0) {
        // THE GUARD IS READ BEFORE THE STORE. The image loads [0x90db78]
        // at 0x004C5D21 and only then writes 0 to [0x90db7c]; the two are
        // different addresses, so the order is free and the image picked one.
        const HMODULE module = SoundDllModule();
        WaveDeviceReleaseGuard = 0;
        if (module != 0) {
            FreeLibrary(module);
            SoundDllModule() = 0;
        }
        memset(g_0090db24, 0, 0xb * 4);
        return result;
    }
    if ((backends & 2) != 0) {
        Midi_Device *midiDevice = reinterpret_cast<Midi_Device *>(g_0090d950);
        midiDevice->init(window, 2);
    }
    if ((backends & 8) != 0) {
        Wave_In_Device *waveInDevice = WaveInDeviceGlobal;
        waveInDevice->init(window, backends);
    }
    WaveDeviceReleaseGuard = 1;
    return 0;
}

// The unrecovered Sound vftable slots (0x004482F0 and the 0x004483xx run):
// implemented in the image's effect region, bodies not yet recovered. The
// declarations keep the vftable slots true; the bodies are semantic debt.
void Sound::unk_slot1() {}
void Sound::unk_slot24() {}
void Sound::unk_slot25() {}
void Sound::unk_slot26() {}
void Sound::unk_slot27() {}
void Sound::unk_slot28() {}
void Sound::unk_slot30() {}

// Slot 15: pure in the image's Sound vftable, overridden per derived class
// (Wave's override is get_game_hwnd). This tree defines the base slot so the
// vftable links; the per-class overrides carry the real behavior.
int Sound::get_game_hwnd() { return 0; }
