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
#include "voice.h"
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

// Independent zero-initialized pointer objects used by the common chain.
// Original attach writes head at 0x004C6387 and tail at 0x004C638D;
// Sound::detach and Wave_Device::select use the same storage identities.
// RECOVERY_OBJECT: v2 0x0090DB20 0x0090DB24 ?SoundChainHead@@3PAVSound@@A 0x004C6370
Sound *SoundChainHead;
// RECOVERY_OBJECT: v2 0x0090DB1C 0x0090DB20 ?SoundChainTail@@3PAVSound@@A 0x004C6370
Sound *SoundChainTail;

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

// Sound destruction publishes this table at original 0x004C6140. Its
// inherited overload positions are independently consumed by derived tables;
// unfinished placeholder slots still prevent complete-table verification.
// RECOVERY_GENERATED: v1 vtable 0x0066E3C0 ? ??_7Sound@@6B@ 0x004C6120

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
    // The source-owned virtual APIs occupy original slots 0 and 10.
    // Historical seam spelling used vtable_method to emit the pinned calls;
    // the source calls retain virtual dispatch, with matching still owed.
    if (set_fade(a1) == 0) {
        fade();
    }
}



namespace {
// Historical LEVER/TRIED notes below document manual dispatch spellings that
// predate this typed external call view; their compiler observations remain
// useful history, not descriptions of the current call expressions.
// Sound wraps its device at 0x3C and asks it these questions through the
// device's own vtable; the original tail-jumps, so the device's answer is the
// caller's, and with no device wrapped the answer is zero.
__forceinline SoundDeviceInterface *sound_device(void *device) {
    return static_cast<SoundDeviceInterface *>(device);
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
    if (device_) {
        return sound_device(device_)->is_playing();
    }
    return 0;
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
    if (device_) {
        return sound_device(device_)->is_looping();
    }
    return 0;
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
// function in the image, between the pure pair and get_type. Virtualness
// spells U, not Q, in the mangled name - hence the symbol fact. The body is
// unchanged; the claim re-measured BYTE_EXACT under it.
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::get_time() {
    if (device_) {
        return sound_device(device_)->get_time();
    }
    return 0;
}




// `forward_sound_device` stood here until 2026-09-03. It carried every
// argument-forwarding dispatch through `original_slot` - a slot-read-then-call
// - and every body that reached it is byte-exact only now that each one spells
// the dispatch itself with `vtable_method`, which leaves the call operand on
// the slot.

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
        result = sound_device(device)->play();
    }
    return result;
}

/*
Purpose: Ask the wrapped device to play the given sound, through vtable slot
         0x18.
// ORIGINAL: 0x004C64A0 ?play@Sound@@QAEHI@Z 0x004C64A0-0x004C64BC BYTE_EXACT
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
    // LEVER: the same `device / result = 0x14 / if` shape the no-argument
    // play(), stop() and release() use, with `vtable_method` for the argument
    // dispatch - the image passes the effect thiscall-on-the-stack. The old
    // forward_sound_device helper read the slot into a register first and
    // compiled 3/12 here; the byte-exact no-argument family goes through
    // `vtable_slot` because its slot needs no argument.
    void *device = device_;
    int result = 0x14;
    if (device) {
        result = sound_device(device)->play(effect);
    }
    return result;
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
        result = sound_device(device)->stop();
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
        result = sound_device(device)->release();
    }
    return result;
}

/*
Purpose: Record the loop state at 0x30 and hand it to the wrapped device,
         through vtable slot 0x48. The field is written whether or not a device
         is wrapped.
// ORIGINAL: 0x004C6540 ?set_loop_state@Sound@@QAEXJ@Z 0x004C6540-0x004C655A BYTE_EXACT
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
    int value = loop_state;
    loop_flag_30_ = value;
    void *device = device_;
    if (device) {
        sound_device(device)->set_loop_state(value);
    }
}

/*
Purpose: Record the delay at 0x34 and hand it to the wrapped device, through
         vtable slot 0x4C. The field is written whether or not a device is
         wrapped.
// ORIGINAL: 0x004C6560 ?set_delay@Sound@@QAEXI@Z 0x004C6560-0x004C657A BYTE_EXACT
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
    int value = static_cast<int>(delay);
    delay_ = value;
    void *device = device_;
    if (device) {
        sound_device(device)->set_delay(static_cast<unsigned int>(value));
    }
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
    return sound_device(device)->fade();
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
    return sound_device(device)->fade_in();
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
    sound_device(device_)->ramp(a1, a2, a3);
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
        if (sound_device(device_)->unk_slot24()) {
            return 0xF;
        }
    }
    int result;
    {
        result = sound_device(device_)->load(resolved);
    }
    if (result == 0) {
        if (!(flags_40_ & 1)) {
            flags_40_ |= 1;
            attach();
            if (loop_flag_30_) {
                sound_device(device_)->set_loop_state(1);
            }
        }
    } else {
        flags_40_ &= ~1u;
    }
    char *const copy =
        static_cast<char *>(operator new(strlen(resolved) + 1));
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
// ORIGINAL: 0x004C6510 ?set_volume@Sound@@QAEXH@Z 0x004C6510-0x004C6532 BYTE_EXACT
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
    // MASKED IN PLACE, the same shape Wave::set_volume documents: the argument
    // is masked first and the field masks AGAIN - so the image hands the
    // device the once-masked value (eax) and stores the twice-masked one
    // (edx). Masking once into a local lets VC6 merge the two and stores the
    // same register it pushes.
    volume &= 0x7F;
    volume_ = volume & 0x7F;
    if (device_) {
        sound_device(device_)->set_volume(volume);
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
        sound_device(device_)->set_fade(fade);
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
        sound_device(device_)->set_fade_in(fade_in);
    }
    return 0;
}


/*
Purpose: Fade the sound in: its own vtable slot 0x54 takes the time, and only
         a zero answer lets its own slot 0x28 follow up.
// ORIGINAL: 0x004C6660 ?fade_in@Sound@@QAEXI@Z 0x004C6660-0x004C6681 BYTE_EXACT
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
    // Both dispatches go to the slot WHERE IT LIVES - `call [eax+0x54]` and
    // `call [edx+0x28]` - the same shape the byte-exact fade(unsigned long)
    // uses, not the slot-read-then-call the original_slot idiom emits.
    if (set_fade_in(a1) == 0) {
        // Slot 0x28 is this class's own fade() - a REAL virtual call, which
        // reads the vtable into edx exactly as the hand dispatch does.
        fade();
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
        sound_device(device_)->set_pan(pan);
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
    // BOTH dispatches call the slot WHERE IT LIVES - `call [eax+0x14]` on the
    // device, `call [reg+0x80]` on the sound - not the slot-read-then-call the
    // original_slot / original_method idioms emit.
    // The loaded bit is cleared with a plain read-modify-write, as detach's
    // byte-exact `flags_40_ &= ~2u` is: the byte-form `and al, 0xfe` comes
    // from that spelling, not from masking into a local first.
    // TRIED for the 0x80 dispatch, which the image reads into EDX and every
    // spelling here reads into EAX: a real virtual `detach();` call, and the
    // __fastcall `vtable_slot` free-function form. Two instructions of 21
    // differ on the register name alone - fade_in(unsigned) puts two
    // own-vtable dispatches in eax then edx, but here a device dispatch and a
    // branch merge sit between them and the allocator restarts at eax.
    int result = 0;
    void *device = device_;
    if (device) {
        // The no-argument device dispatch goes through the __fastcall slot
        // form, as play()/stop()/release() do - the receiver is already in
        // ecx, so there is no receiver pointer to name at the call site.
        result = sound_device(device)->unload();
    }
    detach();
    device_ = nullptr;
    flags_40_ &= ~1u;
    return result;
}


/*
Purpose: Destroy the sound. This is exactly the body ~Wave inlines as its
         base stage: publish the Sound vtable, return the filename copy to
         the game heap (cleared only when there was one), put the wrapped
         device through the guarded release hook and forget it, unlink from
         the sound chain, and publish the ultimate base's vtable on the way
         out through its real SoundFadeState base. The compiler owns the
         exception cleanup; remaining external hooks are not lifetime proof.
// ORIGINAL: 0x004C6120 ??1Sound@@QAE@XZ 0x004C6120-0x004C61D4;0x004C8450-0x004C8457;0x00659EE2-0x00659EF4 BYTE_EXACT
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
    // The original closes with the root table at 0x004C61C1 and has an
    // exception cleanup at 0x004C8450. SoundFadeState owns both lifetime
    // effects; do not install a table by hand or flatten this base to alter
    // the destructor's code shape. Remaining hooks/chain globals still need
    // independent recovery before the whole sound lifetime is accepted.
    if (fname_) {
        operator delete(fname_);
        fname_ = nullptr;
    }
    if (device_) {
        if (WaveDeviceReleaseGuard) {
            (WaveDeviceReleaseSlot())(device_);
        }
        device_ = nullptr;
    }
    if (flags_40_ & 2) {
        Sound *const prev = chain_prev_;
        if (prev) {
            prev->chain_next_ = chain_next_;
        } else {
            SoundChainHead = chain_next_;
        }
        Sound *const next = chain_next_;
        if (next) {
            next->chain_prev_ = chain_prev_;
        } else {
            SoundChainTail = chain_prev_;
        }
        chain_next_ = nullptr;
        chain_prev_ = nullptr;
        flags_40_ &= ~2u;
    }
}


/*
Purpose: The compiler-generated scalar deleting destructor: destroy the
         sound and, when bit 0 of the mode asks, free the storage to the
         game heap.
// ORIGINAL: 0x004C92D0 ??_GSound@@UAEPAXI@Z 0x004C92D0-0x004C92F1 BYTE_EXACT
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
    // QUALIFIED, not `self->~Sound()`: the bare spelling makes VC6 dispatch
    // the DELETING destructor through slot 8 (`push 0; mov eax,[esi]; call
    // [eax+8]`, re-reading the mode off [esp+8]) where the image calls
    // ??1Sound@@QAE@XZ directly.
    self->Sound::~Sound();
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
// ORIGINAL: 0x004C6370 ?attach@Sound@@QAEHXZ 0x004C6370-0x004C63C1 BYTE_EXACT
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
    // BUG IN THE ORIGINAL: the empty-chain arm re-zeroes both links the guard
    // above has just proved null. The stores are unobservable in the running
    // program - but the image performs them, and they are what put the zero in
    // the register the image's `cmp eax,edx` / `cmp [ecx+0x44],edx` reuse, so
    // they are written here and left alone deliberately.
    // TRIED 2026-08-30 with the stores omitted: the image's persistent zero
    // register never materialises and the compares fall back to `test`.
    if (!SoundChainTail) {
        SoundChainHead = this;
        SoundChainTail = this;
        chain_next_ = nullptr;
        chain_prev_ = nullptr;
        flags_40_ |= 2;
        return 0;
    }
    chain_prev_ = SoundChainTail;
    SoundChainTail = this;
    chain_prev_->chain_next_ = this;
    chain_next_ = nullptr;
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
        SoundChainHead = chain_next_;
    }
    Sound *const next = chain_next_;
    if (next) {
        next->chain_prev_ = chain_prev_;
    } else {
        SoundChainTail = chain_prev_;
    }
    chain_next_ = nullptr;
    flags_40_ &= ~2u;
    chain_prev_ = nullptr;
    return 0;
}



/*
// ORIGINAL: 0x004C5CE0 ?init_sound@@YAHPAXK@Z 0x004C5CE0-0x004C5D8E BYTE_EXACT
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
int __cdecl init_sound(void *window, unsigned long backends) {
    int loadResult = load_sound_dll();
    if (loadResult != 0) {
        return loadResult;
    }
    if (SoundDllModule != 0) {
        (*reinterpret_cast<func_sound_dll_init *>(&SoundDllProcs[2]))(0, 0);
    }
    int result = WaveDeviceGlobal.init(window, backends);
    if (result != 0) {
        // THE GUARD IS READ BEFORE THE STORE. The image loads [0x90db78]
        // at 0x004C5D21 and only then writes 0 to [0x90db7c]; the two are
        // different addresses, so the order is free and the image picked one.
        const HMODULE module = SoundDllModule;
        WaveDeviceReleaseGuard = 0;
        if (module != 0) {
            FreeLibrary(module);
            SoundDllModule = 0;
        }
        memset(SoundDllProcs, 0, sizeof(SoundDllProcsTable));
        return result;
    }
    if ((backends & 2) != 0) {
        // QUALIFIED CALLS: the image calls both device inits directly
        // (`call 0x4c57a0` / `call 0x4c5a10`, receiver in ecx), not through
        // the vtables - an unqualified call virtual-dispatches and the
        // shapes diverge.
        g_MIDI_DEVICE.Midi_Device::init(window, 2);
    }
    if ((backends & 8) != 0) {
        g_WAVE_IN_DEVICE.Wave_In_Device::init(window, backends);
    }
    WaveDeviceReleaseGuard = 1;
    return 0;
}

// ORIGINAL: 0x004C8410 ??0Patch@@QAE@XZ 0x004C8410-0x004C8442 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:4C8410..4C8442: stores root table66E444, volume+4=7F, pan+8=0, zeroes+C..+2F and clears+30; +34/+38 remain untouched", "pinned:Sound constructor4C6080 publishes66E444 before Sound table66E3C0; Sound destructor4C6120 and shared unwind path4C8450 restore the same root; Ambience/Wave/Voice constructors stage the same prefix", "src/sound.h SoundFadeState actual members and RECOVERY_MEMBER bindings account for fields+4..+38; the root's two fade slots select the shared typed SoundFadeState behavior", "promoted historical seed src/recovered/units/004c8410.cpp retains original catalog name ??0Patch@@QAE@XZ, full50-byte span, and BYTE_EXACT claim"], "reason": "Own the complete initialized 0x3C SoundFadeState prefix through its existing real base class. The old Patch catalog name is retained only as historical function identity; it does not establish a Patch class or hierarchy. Leave delay_ and fade_38_ untouched. Externalizing this formerly inline constructor changes compiler call placement, so native matching/restoration across Sound-derived constructors remains required."}
// symbol    ??0SoundFadeState@@QAE@XZ
SoundFadeState::SoundFadeState() {
  volume_ = 0x7F;
  pan_8_ = 0;
  memset(state_C_, 0, sizeof(state_C_));
  loop_flag_30_ = 0;
  // The original base does not initialize delay_ or fade_38_. Sound sets
  // the default interval after constructing this prefix.
}

// Original family tables share slot 1 at 0x004482F0. Unlike set_fade,
// this changes the remembered interval without notifying the wrapped device.
// The behavioral name is provisional; zero refuses without changing state.
// ORIGINAL: 0x004482F0 sub_4482f0 0x004482F0-0x0044830C BYTE_EXACT
// symbol    ?set_fade_interval@SoundFadeState@@UAEHK@Z
int SoundFadeState::set_fade_interval(unsigned long interval) {
    if (interval == 0) {
        return 10;
    }
    fade_38_ = interval;
    return 0;
}

// Root slot 0 has the same original body as root slot 1. Sound overrides
// only this slot to notify its wrapped device; the root only remembers it.
// RECOVERY_SHARED: v1 0x004482F0 0x0044830C ?set_fade@SoundFadeState@@UAEHK@Z 0x004C6080
int SoundFadeState::set_fade(unsigned long interval) {
    if (interval == 0) {
        return 10;
    }
    fade_38_ = interval;
    return 0;
}

// Construct the Sound-owned fields after the fade prefix. The original
// flags clear covers one complete dword and does not initialize delay_.
// The first table publication in 0x004C6080 and final publication in
// 0x004C6120 identify the base table. Its complete original extent remains
// unclaimed; two observed slots are not a full linked-table proof.
// RECOVERY_GENERATED: v1 vtable 0x0066E444 ? ??_7SoundFadeState@@6B@ 0x004C6080
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: no tier is reachable for ANY body - SHARED_TAIL, the helper at 0x004C8450 (mov [ecx],0x66e444; ret) is COMDAT-folded with another catalogued function, same wall as VoiceRx::VoiceRx() at 0x004C8960. Straight-line field reconstruction below, no attempt at the SEH try/catch unwind frame (bare try/catch with rethrow produced no fs:[0] frame at all under this compiler's flags; the scaffold gives Sound no base class to hang the automatic base/derived vtable-swap unwind on, same as the already-tried ~UV2Player shape).
// ORIGINAL: 0x004C6080 ??0Sound@@QAE@XZ 0x004C6080-0x004C611A;0x004C8450-0x004C8457;0x00659ED0-0x00659EE2
Sound::Sound() {
    chain_prev_ = 0;
    chain_next_ = 0;
    device_ = 0;
    fname_ = 0;
    memset(&flags_40_, 0, sizeof(flags_40_));
    flags_40_ &= ~1u;
    Sound::set_fade(1000);
    type_ = 0;
}

// Purpose: Return the next Sound in the common chain (family slot 25).
// Ownership is also witnessed by attach/detach's typed chain manipulation.
// ORIGINAL: 0x00448310 sub_448310 0x00448310-0x00448314 BYTE_EXACT
// symbol    ?get_next@Sound@@UAEPAV1@XZ
Sound *Sound::get_next() {
    return chain_next_;
}

// Purpose: Return the previous Sound in the common chain (family slot 26).
// ORIGINAL: 0x00448320 sub_448320 0x00448320-0x00448324 BYTE_EXACT
// symbol    ?get_prev@Sound@@UAEPAV1@XZ
Sound *Sound::get_prev() {
    return chain_prev_;
}

// Purpose: Return the type field written by Sound::set_type (family slot 30).
// ORIGINAL: 0x00448330 sub_448330 0x00448330-0x00448334 BYTE_EXACT
// symbol    ?get_type@Sound@@UAEIXZ
uint32_t Sound::get_type() {
    return type_;
}

// Historical seed catalog (inactive): /*
// Historical seed catalog (inactive): Purpose: Sign-extend bit 0 of field 0x40: 0 or -1.
// Historical seed catalog (inactive):          Emitted by tools/generate_field_accessors.py from
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):              mov eax, dword ptr [ecx + 0x40] / shl eax, 0x1f / sar eax, 0x1f / ret
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):          The name is the ADDRESS, not an invention, and `self` is a void
// Historical seed catalog (inactive):          pointer because this body needs `this` to be nothing more than a
// Historical seed catalog (inactive):          pointer and an offset - the class it belongs to is not established.
// Historical seed catalog (inactive):          The adapter declares 0 stack dword(s) so it
// Historical seed catalog (inactive):          cleans 0 bytes, taken from the body's own `ret` -
// Historical seed catalog (inactive):          the only statement of this function's arity that exists, since it has
// Historical seed catalog (inactive):          no mangled name. Declaring fewer would leave them on the caller's
// Historical seed catalog (inactive):          stack.
// Historical seed catalog (inactive): // prior-original: 0x00448380 sub_448380 0x00448380-0x0044838A historical-byte-exact
// Historical seed catalog (inactive): // symbol    ?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): // size      10 bytes
// Historical seed catalog (inactive): // prototype
// Historical seed catalog (inactive): // callers   0   call targets   0
// Historical seed catalog (inactive): // kind      game
// Historical seed catalog (inactive): // flags     hidden;sp_ready;purged_ok
// Historical seed catalog (inactive): // calls     (none)
// Historical seed catalog (inactive): Return Value: the value described above
// Historical seed catalog (inactive): Status: Complete
// Historical seed catalog (inactive): */
// Historical seed catalog (inactive): address=0x00448380
// Historical seed catalog (inactive): mode=body; state=implemented
// Historical seed catalog (inactive): name=sub_448380; symbol=?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): spans=((4490112, 4490122),)
// Historical seed catalog (inactive): kind='game'; exclusion=''
// Historical seed catalog (inactive): byte_exact=True; semantic=False
// Historical seed catalog (inactive): recipe=census
// Historical seed catalog (inactive): levers=()
// Historical seed catalog (inactive): ruled_out=()
// Historical seed catalog (inactive): unrecoverable=()
// Historical seed catalog (inactive): deferred=()
// ORIGINAL: 0x00448380 sub_448380 0x00448380-0x0044838A BYTE_EXACT
// symbol    ?unk_slot24@Sound@@UAEHXZ
// The original Sound table selects 0x00448380 at slot 24. This
// inherited integer contract sign-extends flags_40_ bit zero.
int Sound::unk_slot24() {
    return (flags_40_ & 1) ? -1 : 0;
}

// ORIGINAL: 0x004C83F0 sub_4c83f0 0x004C83F0-0x004C840F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned4C83F0..4C840F loads source-owned SoundChainHead90DB20, dispatches Sound virtual unload slot5 and reloads the shared head after callbacks; preserve return11.", "historical seed:src/recovered/004c83f0.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?unload_active_sound_chain@@YAHXZ
int __cdecl unload_active_sound_chain() {
    while (SoundChainHead != 0) {
        SoundChainHead->unload();
    }
    return 0x0B;
}
