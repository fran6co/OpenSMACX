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
#include <mmsystem.h>
#include "original_seam.h"
#include "general.h"
#include "wave.h"
#include "fx.h"
#include "dialog.h"
#include "sounddevice.h"
#include "net_class.h"
#include "basepop.h"

// The Windows multimedia clock is a system import, not storage in this build.
// RECOVERY_IMPORT: v1 __imp__timeGetTime@0 WINMM.dll name:timeGetTime

// Wave construction at 0x004C676E installs this table. Its derived slots
// 33..56 are also present in Effect's table; inherited placeholders still
// need recovery, so no complete-table acceptance is implied here.
// RECOVERY_GENERATED: v1 vtable 0x0066E44C ? ??_7Wave@@6B@ 0x004C66E0

/*
Purpose: Returns a fixed 11; the legacy implementation is a single constant load.
// ORIGINAL: 0x004C70E0 ?set_asdr@Wave@@QAEHXZ 0x004C70E0-0x004C70E6 BYTE_EXACT
// symbol    ?set_asdr@Wave@@UAEHXZ
// size      6 bytes
// prototype int (__thiscall ?set_asdr@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_asdr() {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C70D0 ?set_bufflimit@Wave@@QAEHI@Z 0x004C70D0-0x004C70D8 BYTE_EXACT
// symbol    ?set_bufflimit@Wave@@UAEHI@Z
// size      8 bytes
// prototype int (__thiscall ?set_bufflimit@Wave@@QAEHI@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_bufflimit(unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C70F0 ?set_attack@Wave@@QAEHIII@Z 0x004C70F0-0x004C70F8 BYTE_EXACT
// symbol    ?set_attack@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_attack@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_attack(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7100 ?set_sustain@Wave@@QAEHIII@Z 0x004C7100-0x004C7108 BYTE_EXACT
// symbol    ?set_sustain@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_sustain@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_sustain(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7110 ?set_decay@Wave@@QAEHIII@Z 0x004C7110-0x004C7118 BYTE_EXACT
// symbol    ?set_decay@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_decay@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_decay(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7120 ?set_release@Wave@@QAEHIII@Z 0x004C7120-0x004C7128 BYTE_EXACT
// symbol    ?set_release@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_release@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_release(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Release the loaded wave. The wrapped device, if there is one, is asked
         to unload through its own vtable slot 0x14 and its result becomes the
         return value; the device is then forgotten. Unless bit 1 of the flag
         dword at 0x54 suppresses it, the object's own vtable slot 0x80 is run,
         and the loaded bit (bit 0) of the flag dword at 0x40 is cleared.
// ORIGINAL: 0x004C6EA0 ?unload@Wave@@QAEXXZ 0x004C6EA0-0x004C6ED9 SEMANTIC
// symbol    ?unload@Wave@@UAEHXZ
// size      57 bytes
// prototype void (__thiscall ?unload@Wave@@QAEXXZ)(Wave* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6EAF 0x004C6EC6
// TRIED: reordering `flags_40_ &= ...` ahead of the trailing vtable
//   dispatch, and binding `this` into a named local first, both tried against
//   the sole remaining divergence (`mov edx,[esi]`/`call [edx+0x80]` in the
//   image vs this tree's `eax` for the same pointer) - neither changes the
//   register VC6 picks. 22/24 instructions, 1.000 similar, MNEMONIC_ONLY;
//   this is the ceiling found so far.
Return Value: whatever the device's unload returned, or 0 when none was wrapped
Status: Complete
*/
int Wave::unload() {
    // The wrapped device still has an opaque external interface; our own
    // chain operation uses the inherited Sound virtual contract.
    typedef int(__fastcall *device_unload_fn)(void *);

    int result = 0;
    if (device_) {
        // CALLED WHERE THE SLOT LIVES: the image's `call dword ptr [eax+0x14]`
        // at 0x004C6EAF is one instruction, and reading the slot into a
        // pointer-to-member first costs a `mov` before it.
        result = vtable_slot<device_unload_fn>(device_, 0x14)(device_);
    }
    // `mov al, byte ptr [esi+0x54]` at 0x004C6EB4: unload is the one place
    // that narrows the 0x54 dword, and only bit 1 is wanted.
    const uint8_t flags = static_cast<uint8_t>(flags_54_);
    device_ = nullptr;
    if (!(flags & 2)) {
        detach();
    }
    flags_40_ &= 0xFFFFFFFEu;
    return result;
}


/*
Purpose: Set the playback pitch, clamped to the range the engine accepts
         (-1200 to 1200). The clamped value is stored at 0x58 and handed to the
         wrapped device, if there is one, through its vtable slot 0x98.
// ORIGINAL: 0x004C6EE0 ?set_pitch@Wave@@QAEXH@Z 0x004C6EE0-0x004C6F19 BYTE_EXACT
// symbol    ?set_pitch@Wave@@UAEHH@Z
// LEVER: returns-int - same as Wave_Device::release/enable/disable. The
//        image's shared tail does `xor eax, eax` before `pop ebp; ret 4`,
//        the int-returning epilogue; a void body tail-jumps instead. The
//        catalogued name spells `X` (void) but the body is int-shaped, so
//        the return type changes to `int` and the `// symbol` fact carries
//        the int-mangled name the build emits. 17/20 -> 20/20.
// size      57 bytes
// prototype void (__thiscall ?set_pitch@Wave@@QAEXH@Z)(Wave* this, int)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F0D
Return Value: n/a
Status: Complete
*/
int Wave::set_pitch(int a1) {
    int pitch = a1;
    if (pitch < -1200) {
        pitch = -1200;
    } else if (pitch > 1200) {
        pitch = 1200;
    }
    pitch_ = pitch;
    if (device_) {
        // The int argument is passed ON THE STACK by the device's own
        // calling convention, not in edx as a second `__fastcall` parameter
        // would be; the pointer-to-member type keeps that thiscall shape.
        typedef void (OriginalObject::*set_pitch_fn)(int pitch);
        (ORIGINAL(device_)->*vtable_slot<set_pitch_fn>(device_, 0x98))(pitch);
    }
    return 0;
}

/*
Purpose: Load a wave. The object's own vtable slot 0x88 does the loading; unless
         bit 2 of the second argument says otherwise, its own slot 0x8C then
         runs as the follow-up and supplies the result.
// ORIGINAL: 0x004C6DB0 ?load@Wave@@QAEHPADK@Z 0x004C6DB0-0x004C6DE6 BYTE_EXACT
// symbol    ?load@Wave@@UAEHPADK@Z
// size      54 bytes
// prototype int (__thiscall ?load@Wave@@QAEHPADK@Z)(Wave* this, int8*, unsigned int)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6DC3 0x004C6DDA
// TRIED: the sole remaining divergence at best flags (/O2 /Gy /GR- /Oy-
//   /GX, 24/26, 1.000 similar, MNEMONIC_ONLY) is `mov edx,[esi]`/
//   `call [edx+0x8c]` in the image against this tree's `eax` for the same
//   pointer - the same register-choice gap `Wave::unload` has on its own
//   trailing `vtable_slot` dispatch, where reordering statements and binding
//   `this` into a named local did not move it. Not re-tried here on that
//   evidence.
Return Value: slot 0x8C's result, or 0 when bit 2 of a2 skipped it
Status: Complete
*/
int Wave::load(char *a1, unsigned long a2) {
    init(a1, a2);
    if (a2 & 4) {
        return 0;
    }
    return load();
}



/*
Purpose: Report the wave's length in milliseconds, a plain read of the field
         at 0x60.
// ORIGINAL: 0x004C6B00 ?get_ms_length@Wave@@QAEHXZ 0x004C6B00-0x004C6B04 BYTE_EXACT
// symbol    ?get_ms_length@Wave@@UAEHXZ
// size      4 bytes
// prototype int (__thiscall ?get_ms_length@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the stored length
Status: Complete
*/
int Wave::get_ms_length() {
    return ms_length_;
}


/*
Purpose: Report whether the wave is still sounding. A wrapped device answers
         for itself through its own vtable slot 0x5C; with no device the wave
         is timed against the clock instead.
// ORIGINAL: 0x004C6B10 ?is_playing@Wave@@QAEHXZ 0x004C6B10-0x004C6B53
// symbol    ?is_playing@Wave@@UAEHXZ
// size      67 bytes
// prototype int (__thiscall ?is_playing@Wave@@QAEHXZ)(Wave* this)
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6B2B 0x004C6B4A
// TRIED: inverting the guard so the CLOCK path is the fall-through and
//   the device dispatch is the merged tail - which is the shape the image's
//   forward `jne` suggests - scores WORSE, 2 of 30 against the current 4 of
//   30, and compiles 28 instructions against the image's 30. Spelling the
//   flag test as `(flags_54_ >> 4) & 1` and returning a literal 1 instead of
//   the ternary was part of the same attempt and did not rescue it. The
//   branch-polarity lever is real elsewhere in this tree; it does not apply
//   here.
// TRIED 2026-09-03: nesting the whole clock path inside
//   `if (device_ == nullptr)` with the dispatch as the fall-through return -
//   the shape the image's `jne 0x4c6b48` to an end-of-body dispatch suggests -
//   compiles 26 instructions against the image's 30 and still 4/30, with and
//   without the `(flags_54_ >> 4) & 1` spelling for the clocked test. The
//   committed flat spelling keeps the most instructions agreeing; leave it.
Return Value: nonzero while playing, 0 once finished or when not started
Status: Complete
*/
int Wave::is_playing() {
    // The device answers through its live vtable rather than a C++ virtual
    // call, and it is the receiver of that call - the original loads it into
    // ecx first, then dispatches on `[[ecx]+0x5C]`.
    typedef int(__fastcall *device_is_playing_fn)(void *);
    if (device_) {
        return vtable_slot<device_is_playing_fn>(device_, 0x5C)(device_);
    }
    // `mov eax, dword ptr [esi+0x54]` then `shr eax, 4` / `test al, 1` at
    // 0x004C6B1A: a dword read of the whole flag field, testing bit 4.
    if (!(flags_54_ & 0x10)) {
        // The original merges here into a `test ecx, ecx` that can only fall
        // through, because ecx is the null device it already tested. Nothing
        // reachable follows it.
        return 0;
    }
    if (start_time_ == 0) {
        return 0;
    }
    const uint32_t elapsed = timeGetTime() - start_time_;
    // `cmp ecx, eax` / `jbe`: the length is compared unsigned, so a negative
    // stored length reads as a very long one rather than as already finished.
    return static_cast<uint32_t>(ms_length_) > elapsed ? 1 : 0;
}


/*
Purpose: Start the wave playing. A wrapped device answers through its own
         vtable slot 0x94, as the receiver, with the play argument passed on;
         its result is returned verbatim. With no device the answer is a
         fixed 0x14.
// ORIGINAL: 0x004C68F0 ?play@Wave@@QAEHH@Z 0x004C68F0-0x004C6913 BYTE_EXACT
// symbol    ?play@Wave@@UAEHH@Z
// size      35 bytes
// prototype int (__thiscall ?play@Wave@@QAEHH@Z)(Wave* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6900
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::play(int effect) {
    typedef int (OriginalObject::*device_play_fn)(int effect);
    if (device_) {
        return (ORIGINAL(device_)->*vtable_slot<device_play_fn>(device_, 0x94))(effect);
    }
    return 0x14;
}


// The wrapped-device forwarder family: each method lets the device at 0x3C
// answer through its own vtable slot, as the receiver, and returns a fixed
// answer when no device is wrapped. Only the slot, the argument list, and the
// no-device default vary.

/*
Purpose: Ask the wrapped device, through its vtable slot 0xC8, whether the
         buffer is in hardware.
// ORIGINAL: 0x004C6AE0 ?is_hwbuffer@Wave@@QAEHXZ 0x004C6AE0-0x004C6AF2 BYTE_EXACT
// symbol    ?is_hwbuffer@Wave@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?is_hwbuffer@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::is_hwbuffer() {
    typedef int(__fastcall *device_fn)(void *);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xC8)(device_);
    }
    return 0;
}


/*
Purpose: Ask the wrapped device for a time value through its vtable slot 0xB4,
         passing the query argument on.
// ORIGINAL: 0x004C6FD0 ?get_time@Wave@@QAEHI@Z 0x004C6FD0-0x004C6FF0 BYTE_EXACT
// symbol    ?get_time@Wave@@UAEHI@Z
// size      32 bytes
// prototype int (__thiscall ?get_time@Wave@@QAEHI@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6FE0
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_time(uint32_t a1) {
    typedef int (OriginalObject::*device_fn)(uint32_t a1);
    if (device_) {
        return (ORIGINAL(device_)->*vtable_slot<device_fn>(device_, 0xB4))(a1);
    }
    return 0;
}


/*
Purpose: Ask the wrapped device for the current marker through its vtable
         slot 0xB8.
// ORIGINAL: 0x004C6FF0 ?get_current_marker@Wave@@QAEHXZ 0x004C6FF0-0x004C7003 BYTE_EXACT
// symbol    ?get_current_marker@Wave@@UAEHXZ
// size      19 bytes
// prototype int (__thiscall ?get_current_marker@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or -1 when no device is wrapped
Status: Complete
*/
int Wave::get_current_marker() {
    typedef int(__fastcall *device_fn)(void *);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xB8)(device_);
    }
    return -1;
}


/*
Purpose: Ask the wrapped device for the game window handle through its vtable
         slot 0x3C.
// ORIGINAL: 0x004C7010 ?get_game_hwnd@Wave@@QAEHXZ 0x004C7010-0x004C701F BYTE_EXACT
// symbol    ?get_game_hwnd@Wave@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?get_game_hwnd@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_game_hwnd() {
    typedef int(__fastcall *device_fn)(void *);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0x3C)(device_);
    }
    return 0;
}


/*
Purpose: Ask the wrapped device for the device count through its vtable
         slot 0xBC.
// ORIGINAL: 0x004C7020 ?get_ndevices@Wave@@QAEHXZ 0x004C7020-0x004C7032 BYTE_EXACT
// symbol    ?get_ndevices@Wave@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_ndevices@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_ndevices() {
    typedef int(__fastcall *device_fn)(void *);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xBC)(device_);
    }
    return 0;
}


/*
Purpose: Unknown. The original chases the pointer chain that starts at the
         wrapped device and continues through each node's own 0x3C slot until
         it runs out, reads nothing else, and always answers 1. The argument
         is ignored.
// ORIGINAL: 0x004C7080 ?UNK1@Wave@@QAEHH@Z 0x004C7080-0x004C7096 BYTE_EXACT
// symbol    ?probe_device_chain@Wave@@QAEHH@Z
// size      22 bytes
// prototype int (__thiscall ?UNK1@Wave@@QAEHH@Z)(Wave* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Wave::probe_device_chain(int) {
    uint8_t *cursor = static_cast<uint8_t *>(device_);
    while (cursor) {
        cursor = *reinterpret_cast<uint8_t **>(cursor + 0x3C);
    }
    return 1;
}


/*
Purpose: Set the reverb mix. The value is stored at 0x5C first, then the
         wrapped device is told through its vtable slot 0xE0.
// ORIGINAL: 0x004C70A0 ?set_reverb_mix@Wave@@QAEHM@Z 0x004C70A0-0x004C70C6
// symbol    ?set_reverb_mix@Wave@@UAEHM@Z
// size      38 bytes
// prototype int (__thiscall ?set_reverb_mix@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C70B3
// TRIED: the sole divergence is the register holding the vtable pointer at
//   the slot 0xE0 dispatch - the image loads it into `edx` because `eax` is
//   still holding the float argument it loaded for the `reverb_mix_` store,
//   where this tree kills the value after the store and reloads it from
//   [ebp+8]. Measured: the pointer-to-member `float` typedef (kept, 6/15),
//   a named `float` local (6/15, 17 instructions), `reinterpret_cast<const
//   uint32_t &>` on the argument with a `uint32_t` typedef (11/15 - better
//   agreement, but it is a type lie about a float slot and still MISMATCH),
//   passing the field instead of the parameter (6/15, 18), and a ternary
//   (11/15). Same allocator gap `Wave::play` records; left on the honest
//   spelling.
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_reverb_mix(float mix) {
    reverb_mix_ = mix;
    if (device_) {
        typedef int(__fastcall *device_fn)(void *, uint32_t);
        return vtable_slot<device_fn>(device_, 0xE0)(
            device_, reinterpret_cast<const uint32_t &>(mix));
    }
    return 0x14;
}


/*
Purpose: Ask the wrapped device, through its vtable slot 0xDC, whether the
         wave is 3D positioned.
// ORIGINAL: 0x004C7250 ?is_3d@Wave@@QAEHXZ 0x004C7250-0x004C7262 SEMANTIC
// symbol    ?is_3d@Wave@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?is_3d@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped (the
              original defines only AL on that path; callers test the byte)
Status: Complete
*/
int Wave::is_3d() {
    // TRIED: static_cast<unsigned char>(0), a local unsigned char, and a
    // ternary all still compile the no-device path as `xor eax, eax` where
    // the image narrows to `xor al, al`; MNEMONIC_ONLY plateau, 6/7 agreeing.
    typedef int(__fastcall *device_fn)(void *);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xDC)(device_);
    }
    return 0;
}


/*
Purpose: Describe a device into the caller's buffer through the wrapped
         device's vtable slot 0xC0. With no device the buffer is terminated
         to an empty string - but only when the third argument is nonzero,
         and without any null check on the buffer - and the answer is 1.
// ORIGINAL: 0x004C7040 ?get_device_description@Wave@@QAEHPADHH@Z 0x004C7040-0x004C7078 BYTE_EXACT
// symbol    ?get_device_description@Wave@@UAEHPADHH@Z
// size      56 bytes
// prototype int (__thiscall ?get_device_description@Wave@@QAEHPADHH@Z)(Wave* this, int8*, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7058
Return Value: the device's answer, or 1 when no device is wrapped
Status: Complete
*/
int Wave::get_device_description(char *a1, int a2, int a3) {
    typedef int (OriginalObject::*device_fn)(char *a1, int a2, int a3);
    if (device_) {
        return (ORIGINAL(device_)->*vtable_slot<device_fn>(device_, 0xC0))(a1, a2, a3);
    }
    if (a3) {
        *a1 = '\0';
    }
    return 1;
}


/*
Purpose: Position the wave in 3D through the wrapped device's vtable
         slot 0xCC.
// ORIGINAL: 0x004C7190 ?set_position3d@Wave@@QAEHMMM@Z 0x004C7190-0x004C71BB BYTE_EXACT
// symbol    ?set_position3d@Wave@@UAEHMMM@Z
// size      43 bytes
// prototype int (__thiscall ?set_position3d@Wave@@QAEHMMM@Z)(Wave* this, float, float, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C71A8
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_position3d(float x, float y, float z) {
    typedef int(__fastcall *device_fn)(void *, float, float, float);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xCC)(device_, x, y, z);
    }
    return 0x14;
}


/*
Purpose: Set the wave's X position through the wrapped device's vtable
         slot 0xD0.
// ORIGINAL: 0x004C71C0 ?set_xpos@Wave@@QAEHM@Z 0x004C71C0-0x004C71E3 BYTE_EXACT
// symbol    ?set_xpos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_xpos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C71D0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_xpos(float x) {
    typedef int(__fastcall *device_fn)(void *, float);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xD0)(device_, x);
    }
    return 0x14;
}


/*
Purpose: Set the wave's Y position through the wrapped device's vtable
         slot 0xD4.
// ORIGINAL: 0x004C71F0 ?set_ypos@Wave@@QAEHM@Z 0x004C71F0-0x004C7213 BYTE_EXACT
// symbol    ?set_ypos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_ypos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7200
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_ypos(float y) {
    typedef int(__fastcall *device_fn)(void *, float);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xD4)(device_, y);
    }
    return 0x14;
}


/*
Purpose: Set the wave's Z position through the wrapped device's vtable
         slot 0xD8.
// ORIGINAL: 0x004C7220 ?set_zpos@Wave@@QAEHM@Z 0x004C7220-0x004C7243 BYTE_EXACT
// symbol    ?set_zpos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_zpos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7230
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_zpos(float z) {
    typedef int(__fastcall *device_fn)(void *, float);
    if (device_) {
        return vtable_slot<device_fn>(device_, 0xD8)(device_, z);
    }
    return 0x14;
}


/*
Purpose: Store the attribute mask into the wave's own fields, then tell the
         wrapped device through its vtable slot 0x6C. Bit 1 of the mask sets
         the dword at 0x30; the other bits map onto the flag dword at 0x54
         (bit 0 -> 1, bit 2 -> 2, bit 6 -> 8, bit 7 -> 0x10, and - only when
         bit 2 is clear - bit 4 -> 4 and bit 8 -> 0x20). Bits already set at
         0x54 are never cleared.
// ORIGINAL: 0x004C6F20 ?set_attrib@Wave@@QAEXK@Z 0x004C6F20-0x004C6F80 BYTE_EXACT
// symbol    ?set_attrib@Wave@@UAEHK@Z
// LEVER: returns-int - same as set_pitch / Wave_Device::release. The image's
//        epilogue is the int-returning `xor eax, eax` shared tail; a void body
//        tail-jumps instead. Return type becomes `int`, the `// symbol` fact
//        carries the int-mangled name the build emits.
// size      96 bytes
// prototype void (__thiscall ?set_attrib@Wave@@QAEXK@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F77
Return Value: n/a
Status: Complete
*/
int Wave::set_attrib(unsigned long attrib) {
    typedef int (OriginalObject::*device_fn)(uint32_t attrib);
    if (attrib & 2) {
        loop_flag_30_ = 1;
    }
    if (attrib & 1) {
        flags_54_ |= 1;
    }
    if (attrib & 4) {
        flags_54_ |= 2;
    }
    if (attrib & 0x40) {
        flags_54_ |= 8;
    }
    if (attrib & 0x80) {
        flags_54_ |= 0x10;
    }
    if (!(attrib & 4) && (attrib & 0x10)) {
        flags_54_ |= 4;
    }
    if (!(attrib & 4) && (attrib & 0x100)) {
        flags_54_ |= 0x20;
    }
    if (device_) {
        (ORIGINAL(device_)->*vtable_slot<device_fn>(device_, 0x6C))(attrib);
    }
    return 0;
}


/*
Purpose: Compose the attribute mask back out of the wave's own fields, OR-ed
         over whatever the wrapped device answers through its vtable
         slot 0x70 (0 with no device). The mapping inverts set_attrib's:
         the dword at 0x30 -> bit 1, and the 0x54 flag dword's bits
         1 -> 0, 8 -> 6, 2 -> 2, 4 -> 4, 0x10 -> 7, 0x20 -> 8.
// ORIGINAL: 0x004C6F80 ?get_attrib@Wave@@QAEHXZ 0x004C6F80-0x004C6FCA BYTE_EXACT
// symbol    ?get_attrib@Wave@@UAEHXZ
// size      74 bytes
// prototype int (__thiscall ?get_attrib@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F8E
Return Value: the composed attribute mask
Status: Complete
*/
int Wave::get_attrib() {
    int result = 0;
    if (device_) {
        typedef int(__fastcall *device_fn)(void *);
        result = vtable_slot<device_fn>(device_, 0x70)(device_);
    }
    if (loop_flag_30_) {
        result |= 2;
    }
    const uint32_t flags = flags_54_;
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



/*
Purpose: Set the wave's volume. The low seven bits of the argument are stored
         at 0x04; while the wave holds a device group slot the level handed
         to the device is rescaled by that group's own volume dword (one
         every 24 bytes in the singleton's table), as level/127 * group in
         double precision truncated back to an integer. The wrapped device,
         if any, hears the result through its vtable slot 0x40.
// ORIGINAL: 0x004C7130 ?set_volume@Wave@@QAEXH@Z 0x004C7130-0x004C718D
// symbol    ?set_volume@Wave@@UAEXH@Z
// TRIED: 26/33 MISMATCH, instruction COUNT now matches the image (33/33)
//   but the `fild dword ptr [ebp+8]` + `fmul qword ptr [const]` pair that
//   materialises `scaled` still schedules LATE (right before the final
//   multiply) where the image schedules it EARLY (right after the group-slot
//   range check, before the table read). Tried `volatile double scaled`
//   (worse, back to 13/33 - forces a real store/reload and an extra
//   instruction); a separate named `group` local read after `scaled` (no
//   change, still 26/33 - same late scheduling); `static double scaled`
//   (worse, 14/33, extra relocation-backed storage). VC6's scheduler treats
//   the local's definition point as a lower bound, not a fixed position, and
//   nothing tried pinned it to the top of the block.
// size      93 bytes
// prototype void (__thiscall ?set_volume@Wave@@QAEXH@Z)(Wave* this, int)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006458C8
// indirect  0x004C7184
Return Value: n/a
Status: Complete
*/
void Wave::set_volume(int volume) {
    // MASKED IN PLACE, no `vol` local: the image masks the incoming argument
    // and masks again for the field.
    volume &= 0x7F;
    volume_ = volume & 0x7F;
    int level = volume;
    if (group_slot_ < 0x10) {
        // The group entry is the LAST factor, so the level's own
        // `fild dword ptr [ebp+8]` is emitted first and the group arrives as a
        // `fild qword` + `fmulp st(1)` rather than an `fimul`. The original
        // loads that dword zero-extended through a 64-bit fild, so the scale
        // is the UNSIGNED value of the table entry.
        // LEVER: naming the `volume * (1.0/127.0)` product as its own local -
        // `scaled` - before multiplying by the group value, rather than
        // writing the whole product as one expression, is what makes VC6
        // emit the `fild`+`fmul`-by-constant PAIR the image has instead of
        // folding volume's conversion into a single `fimul` at the end. 13/33 ->
        // 26/33 MISMATCH.
        const double scaled = static_cast<double>(volume) * (1.0 / 127.0);
        level = static_cast<int>(static_cast<int64_t>(
            scaled * static_cast<double>(WaveDeviceGroupVolumes[group_slot_ * 6])));
    }
    if (device_) {
        typedef void (OriginalObject::*device_fn)(int level);
        (ORIGINAL(device_)->*vtable_slot<device_fn>(device_, 0x40))(level);
    }
}


/*
Purpose: Remember the wave's filename. The previous copy, if any, goes back
         to the game CRT heap first; the new string is measured, allocated on
         that same heap, recorded at 0x4C, and copied in - in that order, so
         the field already names the block while it is still being filled.
// ORIGINAL: 0x004C6B60 ?set_fname@Wave@@QAEHPBD@Z 0x004C6B60-0x004C6BAB BYTE_EXACT
// size      75 bytes
// prototype int (__thiscall ?set_fname@Wave@@QAEHPBD@Z)(Wave* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// LEVER: branch polarity - `if (fname) { work; return 0; } return 0xA;`,
//        NOT the guard clause `if (!fname) return 0xA;`. The guard clause
//        compiles `test edi,edi; jne <work>` with the 0xA return INLINE at the
//        top (11/34); the positive form compiles the image's `je` to a shared
//        0xA tail at the END of the body. 11/34 -> BYTE_EXACT 34/34.
Return Value: 0, or 0xA when the name is null
Status: Complete
*/
int Wave::set_fname(const char *fname) {
    if (fname) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = WaveOperatorNew(strlen(fname) + 1);
        strcpy(static_cast<char *>(fname_), fname);
        return 0;
    }
    return 0xA;
}


/*
Purpose: Start the wave. While it holds a device group slot, a disabled group
         answers 0x14 immediately (the original trusts only the answer's low
         byte); otherwise, when bit 4 of the 0x54 flag dword marks a clocked
         wave, the wave's own vtable slot 0x40 replays the stored volume. A
         wrapped device then starts through its slot 0x1C and its answer is
         the result; with no device, a clocked wave runs the original
         no-argument load and starts the device that load may have wrapped
         (answering 0 when it wrapped none), while an unclocked one answers
         0x14. Finally a clocked wave stamps the start time from the game's
         timeGetTime import, runs its own vtable slot 0x80, and forgets the
         device.
// ORIGINAL: 0x004C6920 ?play@Wave@@QAEHXZ 0x004C6920-0x004C69AD SEMANTIC
// symbol    ?play@Wave@@UAEHXZ
// TRIED: 52/54 MNEMONIC_ONLY; sole divergence is the vtable-pointer
//   register (edx vs eax) at the FIRST `vtable_slot<device_start_fn>(device_,
//   0x1C)(device_)` call site only - the second, identical call already
//   matches. Tried: a named `device_start_fn fn = vtable_slot<...>(...)`
//   temp at the first site only (worse, 26/54); restructuring the
//   device_/flags_54_ branch as `if (!device_) {...} else {...}` with a
//   ternary on the reload path (worse, 2/54). Plain register-allocation
//   divergence between two textually-identical call expressions; no source
//   reshaping tried closed it.
// size      141 bytes
// prototype int (__thiscall ?play@Wave@@QAEHXZ)(Wave* this)
// callers   20   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C5460 0x004C6CE0
// indirect  0x004C6953 0x004C695F 0x004C697C 0x004C698E 0x004C699B
Return Value: the device's start answer, 0x14 when disabled or unstartable,
              0 when a clocked load wrapped no device
Status: Complete
*/
int Wave::play() {
    typedef int(__fastcall *device_start_fn)(void *);

    int result = 0;
    if (group_slot_ < 0x10) {
        if (static_cast<uint8_t>(
                WaveDeviceGlobal.is_group_disabled(group_slot_))) {
            return 0x14;
        }
        if (flags_54_ & 0x10) {
            set_volume(volume_);
        }
    }
    if (device_) {
        result = vtable_slot<device_start_fn>(device_, 0x1C)(device_);
    } else if (flags_54_ & 0x10) {
        Wave::load();
        if (device_) {
            result = vtable_slot<device_start_fn>(device_, 0x1C)(device_);
        }
    } else {
        result = 0x14;
    }
    if (flags_54_ & 0x10) {
        start_time_ = timeGetTime();
        detach();
        device_ = nullptr;
    }
    return result;
}



/*
Purpose: Load the wave from its remembered filename. With no wrapped device
         yet, the creation hook - the slot beside the release hook, behind
         the same guard - builds one directly into the 0x3C field; a dead
         hook answers 1 and a failed creation its own error. The 0x54 flag
         dword and the wave's own vtable slot 0x58 fold into an attribute mask
         the device hears through its slot 0x6C, the base Sound::load reads
         the file, and on success the device reports the length in
         milliseconds through its slot 0xC4 into the field at 0x60.
// ORIGINAL: 0x004C6CE0 ?load@Wave@@QAEHXZ 0x004C6CE0-0x004C6DAC
// symbol    ?load@Wave@@UAEHXZ
// LEVER: `int attribs = 0;` declared as the FIRST statement, before even the
//        `fname` null check - matches the image's `xor edi, edi` right after
//        the prologue's `push edi`, which forces edi to be pushed/popped
//        uniformly across every exit path. 7/88 -> 9/88 MISMATCH.
// TRIED: declaring `attribs` after the fname check but before the
//            device check (partway) - identical to the original placement,
//            no change (7/88); the image's shared tail between "no fname"
//            and "load failed" needs edi live from function ENTRY, not just
//            before the device branch.
// LEVER: the no-filename path spelled `if (fname != nullptr) { ...whole
//        body... } return 8;` - the same positive-branch shape reload and
//        set_fname record. The image jumps to a shared 8 tail at the END;
//        the guard clause inlined it at the top. 9/88 -> 44/88.
// TRIED: swapping the two declaration lines (fname before attribs) - 44/88
//        either way. The remaining 40+ instructions are ONE register-role
//        swap this tree cannot seem to flip: the image holds the attribute
//        accumulator in `edi` and `&device_` in `ebx` (`lea ebx,[esi+0x3c]`),
//        this tree holds them the other way round; reload() is the mirror
//        image and already agrees. Pure allocator choice, no source shape
//        tried moves it.
// size      204 bytes
// prototype int (__thiscall ?load@Wave@@QAEHXZ)(Wave* this)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C6D1D 0x004C6D39 0x004C6D74 0x004C6D8C
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/load error
Status: Complete
*/
int Wave::load() {
    int attribs = 0;
    const char *const fname = static_cast<const char *>(fname_);
    if (fname != nullptr) {
        if (!device_) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = (WaveDeviceCreateSlot())(&device_, fname, 1);
            if (created) {
                return created;
            }
        }
        if (flags_54_ & 1) {
            attribs |= 1;
        }
        if (is_looping()) {
            attribs |= 2;
        }
        const uint32_t flags = flags_54_;
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
            (ORIGINAL(device_)->*vtable_slot<device_attrib_fn>(device_, 0x6C))(attribs);
        }
        // The original calls Sound::load (0x004C6280) directly. Virtual
        // dispatch here would recurse into Wave's filename overload.
        const int loaded = Sound::load(fname);
        if (loaded) {
            return loaded;
        }
        typedef int(__fastcall *device_length_fn)(void *);
        ms_length_ = vtable_slot<device_length_fn>(device_, 0xC4)(device_);
        return 0;
    }
    return 8;
}


/*
Purpose: Reload the wave from its remembered filename. The device is created
         through the guarded hook when missing, hears the folded attribute
         mask (only the low two bits here) through its slot 0x6C, and
         reloads through its slot 0x84. On a first successful load the
         loaded bit of the 0x40 flag dword is set BEFORE the wave's own
         vtable slot 0x7C runs, and a nonzero dword at 0x30 additionally
         starts the device looping through its slot 0x48 with argument 1.
// ORIGINAL: 0x004C6DF0 ?reload@Wave@@QAEHXZ 0x004C6DF0-0x004C6E95 SEMANTIC
// symbol    ?reload@Wave@@UAEHXZ
// size      165 bytes
// prototype int (__thiscall ?reload@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6E23 0x004C6E3F 0x004C6E4E 0x004C6E55 0x004C6E71 0x004C6E81
// LEVER: attribs first (image `xor ebx,ebx` at entry), ONE result cell for the
//        whole body instead of a `return` per exit. The image keeps it in ebx
//        from `mov ebx, 8` on the no-filename path through the `mov eax, ebx`
//        all five exits share; per-exit `return`s give VC6 a provably-constant
//        final return and split the tail in two. 1/72 -> 70/72 MNEMONIC_ONLY.
// TRIED: the 0x6C attribute dispatch spelled `__fastcall(void *, int)` rather
//        than the pointer-to-member form - worse, 67/72. The sole divergence
//        left is the same register-choice gap unload and load(char *) carry:
//        image `mov edx,[ecx]`/`call [edx+0x6c]`, this tree `eax`.
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/reload error
Status: Complete
*/
int Wave::reload() {
    // FIRST, before the fname check - the image's `xor ebx,ebx` at entry, the
    // same lever `load(const char *)` records below: the accumulator has to be
    // live from function entry for the shared tails to line up.
    int attribs = 0;
    // ONE result cell for the whole body. The image keeps it in ebx from the
    // `mov ebx, 8` on the no-filename path through the `mov eax, ebx` every
    // exit shares - spelling each exit its own `return` breaks that tail into
    // two epilogues.
    int result = 0;
    const char *const fname = static_cast<const char *>(fname_);
    if (fname != nullptr) {
        if (!device_) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = (WaveDeviceCreateSlot())(&device_, fname, 1);
            if (created) {
                return created;
            }
        }
        if (flags_54_ & 1) {
            attribs |= 1;
        }
        if (is_looping()) {
            attribs |= 2;
        }
        {
            typedef void (OriginalObject::*device_attrib_fn)(int attribs);
            (ORIGINAL(device_)->*vtable_slot<device_attrib_fn>(device_, 0x6C))(attribs);
        }
        typedef int(__fastcall *device_reload_fn)(void *);
        result = vtable_slot<device_reload_fn>(device_, 0x84)(device_);
        // The image's every exit - the reload error, the already-loaded bit,
        // the loop-flag jump and this fall-through - merges into ONE tail.
        // `if (result) return result;` lets VC6 prove the final return is a
        // constant zero and the shared tail disappears.
        if (result == 0) {
            if (!(flags_40_ & 1)) {
                flags_40_ |= 1;
                attach();
                if (loop_flag_30_) {
                    typedef void (OriginalObject::*device_loop_fn)(int on);
                    (ORIGINAL(device_)->*vtable_slot<device_loop_fn>(device_, 0x48))(1);
                }
            }
        }
    } else {
        result = 8;
    }
    return result;
}



/*
Purpose: Load the wave from in-memory data. Refuses (0xC) when a device is
         already wrapped; otherwise the guarded creation hook builds one from
         the data pointer. The device's vtable is captured BEFORE the wave's
         own slot 0x70 composes the attribute mask - the original dispatches
         the following slot 0x6C through that captured table on the re-read
         device - and the wave's own slot 0x7C finishes.
// ORIGINAL: 0x004C6BB0 ?dyna_load@Wave@@QAEHPAD@Z 0x004C6BB0-0x004C6C1A
// size      106 bytes
// prototype int (__thiscall ?dyna_load@Wave@@QAEHPAD@Z)(Wave* this, int8*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6BDC 0x004C6BF5 0x004C6BFB 0x004C6C02
// LEVER: the work inside the positive `if (device_ == nullptr)` branch, the
//        dead hook as an early `return 1` INSIDE it, and a guard clause on
//        `created`. Three effects, all measured: the 0xC exit becomes a tail
//        at the END of the body instead of an inline return at the top; the
//        `return 1` stays inline after the guard test; and `created` - live
//        past its own block instead of returned early - spills into the dead
//        parameter slot at [ebp+8] and is re-read for the final return, which
//        is exactly the image's `mov [ebp+8],eax` / `mov eax,[ebp+8]` pair.
//        9/49 -> 42/49.
// TRIED: the same body with the hook spelled `if (WaveDeviceReleaseGuard) {
//        ...create... } return 1;` - the 1-exit moved to a tail at the end and
//        the agreement dropped to 17/49.
// TRIED: the two remaining runs are the register swap on the 0x70/0x7C
//        dispatches (image `edx` then `eax`, this tree `eax` then `edx`) - the
//        same allocator plateau reload and load(char *) carry.
Return Value: 0 on success, 0xC with a device already wrapped, 1 with a dead
              creation hook, or the creation error
Status: Complete
*/
int Wave::dyna_load(char *fname) {
    // THE WORK IN THE POSITIVE BRANCH, and the guard clause on `created` - the
    // two shapes reload and set_fname record. The image's 0xC exit is a shared
    // tail at the END of the body, not an inline return at the top, and its
    // `created` is spilled to the dead parameter slot at [ebp+8] and re-read
    // for the final return - which only happens when the value stays live past
    // the block instead of being returned early on the nonzero path.
    if (device_ == nullptr) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (WaveDeviceCreateSlot())(&device_, fname, 1);
        if (created == 0) {
            // The device vtable is captured HERE, before the wave's own
            // slot 0x70 composes the attribute mask; the following 0x6C
            // dispatch runs through this captured table on the re-read
            // device, not a freshly re-derived one.
            uint8_t *const device_vtable =
                *reinterpret_cast<uint8_t **>(device_);
            const int attribs = get_attrib();
            {
                // The int argument is passed on the stack, not in edx; the
                // pointer-to-member type keeps the thiscall shape.
                typedef void (OriginalObject::*device_attrib_fn)(int attribs);
                (ORIGINAL(device_)->*(*reinterpret_cast<device_attrib_fn *>(
                    device_vtable + 0x6C)))(attribs);
            }
            attach();
        }
        return created;
    }
    return 0xC;
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
// ORIGINAL: 0x004C6C20 ?load@Wave@@QAEHPBD@Z 0x004C6C20-0x004C6CD8
// symbol    ?load@Wave@@UAEHPBD@Z
// LEVER: `int attribs = 0;` declared BEFORE the device-create block (not
//        after) - matches the image's `xor ebx, ebx` at function entry,
//        which is reused as the attribs accumulator only after the
//        create/guard logic. Moved 22/79 to 57/79 MISMATCH.
// TRIED: hoisting the 0xC4 length read into a named local and deferring
//            the `ms_length_ =` store past the volume dispatch, to match the
//            image's `mov edx,[esi+4]; mov [esi+0x60],eax` order - no effect
//            on codegen (still 57/79), so reverted; that ordering looks like
//            pure instruction scheduling, not source order.
// size      184 bytes
// prototype int (__thiscall ?load@Wave@@QAEHPBD@Z)(Wave* this, int8*)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C6C50 0x004C6C6C 0x004C6C8C 0x004C6CA2 0x004C6CB3 0x004C6CBE 0x004C6CCC
Return Value: 0 on success, 1 with a dead creation hook, or the
              creation/load error
Status: Complete
*/
int Wave::load(const char *fname) {
    int attribs = 0;
    if (!device_) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (WaveDeviceCreateSlot())(&device_, fname, 1);
        if (created) {
            return created;
        }
    }
    if (flags_54_ & 1) {
        attribs |= 1;
    }
    if (is_looping()) {
        attribs |= 2;
    }
    const uint32_t flags = flags_54_;
    if (flags & 8) {
        attribs |= 0x40;
    }
    if (flags & 0x10) {
        attribs |= 0x80;
    }
    {
        typedef void (OriginalObject::*device_attrib_fn)(int attribs);
        (ORIGINAL(device_)->*vtable_slot<device_attrib_fn>(device_, 0x6C))(attribs);
    }
    // The original calls Sound::load (0x004C6280) directly, not this
    // Wave overload. Qualification preserves the non-recursive base call.
    const int loaded = Sound::load(fname);
    if (loaded) {
        return loaded;
    }
    {
        typedef int(__fastcall *device_length_fn)(void *);
        ms_length_ = vtable_slot<device_length_fn>(device_, 0xC4)(device_);
    }
    {
        typedef void (OriginalObject::*device_level_fn)(uint32_t v);
        (ORIGINAL(device_)->*vtable_slot<device_level_fn>(device_, 0x40))(volume_);
    }
    {
        typedef void (OriginalObject::*device_pitch_fn)(int pitch);
        (ORIGINAL(device_)->*vtable_slot<device_pitch_fn>(device_, 0x98))(pitch_);
    }
    typedef void (OriginalObject::*device_pan_fn)(uint32_t v);
    (ORIGINAL(device_)->*vtable_slot<device_pan_fn>(device_, 0x44))(pan_8_);
    return 0;
}



/*
Purpose: Build the inherited Sound/Fade state once, then initialize Wave's
         own flags, pitch, reverb, timing and group state. Sound::set_type(1)
         is a direct base-method call in the original. The compiler owns
         construction tables and exception cleanup; the historical spelling
         experiments below predate recovery of the fade base.
// ORIGINAL: 0x004C66E0 ??0Wave@@QAE@XZ 0x004C66E0-0x004C67B6;0x004C8450-0x004C8457;0x00659EF4-0x00659F06
// TRIED: 3/73 - the image carries an SEH unwind frame here (`push -1 /
//            push 0x659efc / mov eax,fs:[0] / ...`), the same symptom as
//            FlatButton::FlatButton() (flatbutton.cpp) and
//            GraphicWin::~GraphicWin() (graphicwin.cpp): a real
//            constructor calling something VC6 assumes can throw
//            (`Sound::set_type`) alongside a non-trivial `~Wave()`
//            (0x004C67C0, this file). Not attempted: this body already
//            deliberately elides two intermediate vtable-install stages
//            the image performs before the SEH frame, on the documented
//            theory that a CRT memset between them makes both
//            unobservable - reworking the frame risks that already-
//            reasoned simplification for an uncertain payoff, given
//            FlatButton's own SEH-frame recovery needed no vtable-stage
//            changes and this body's does. Left as is.
// size      239 bytes
// prototype void (__thiscall ??0Wave@@QAE@XZ)(Wave* this)
// callers   16   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C6762
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
*/
// LEVER: the image's call list is memset x3 + Sound::set_type - the region
// loop and the two single-dword field clears (flags_40_, flags_54_) are all
// real `call memset` in the image, not stores; writing them as memset() calls
// (matching set_fname's own `memset(&flags_54_, 0, 4)` lever below) reaches
// that count. Store order also follows the image: an early temporary vtable
// write (0x66E444), then the region memset, then a SECOND temporary vtable
// write (0x66E3C0) ahead of the pointer-field clears and the flags_40_
// memset, then the FINAL vtable write (0x66E44C) ahead of the flags_54_
// memset.
Wave::Wave() {
    // Sound and SoundFadeState construct their own fields exactly once.
    // The original's `memset(this + 0x54, 0, 4)` at 0x004C6774 - same lever
    // as set_fname's own `memset(&flags_54_, 0, 4)` below.
    memset(&flags_54_, 0, sizeof(flags_54_));
    flags_40_ |= 4;
    Sound::set_type(1);
    pitch_ = 0;
    reverb_mix_ = 1.0f;
    group_slot_ = 0x10;
    ms_length_ = 0;
    start_time_ = 0;
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
         0x6C. The mode bits then fold onto the flag dword as in set_attrib
         (bit 4 suppressed for streaming waves, bit 8 unsuppressed here), and
         bit 1 of the mode runs the wave's own vtable slot 0x48 with 1.
// ORIGINAL: 0x004C69B0 ?init@Wave@@QAEXPADK@Z 0x004C69B0-0x004C6AD5
// symbol    ?init@Wave@@UAEXPADK@Z
// TRIED: best flag set /c /O2 /Oi- /Gy /GR- /Oy- /GX reaches 13/122
//   (0.975 similar); `listing_diff` at those flags shows the structure and
//   control flow already match - 23 differing runs, nearly all the SAME
//   global register swap: the image keeps `this` in edi and the resolved
//   filename / later `&flags_54_` pointer in esi (reused once the filename
//   is done with), while this tree keeps `this` in esi and the other value
//   in edi throughout. Tried reassigning the resolved path back onto the
//   `a1` parameter slot itself (`a1 = filefind_get(a1);`, matching the
//   image's `mov [ebp+8], esi` reuse of the a1 stack slot) instead of a
//   separate `resolved` local - byte-identical output, no effect on the
//   allocator's choice. Also present: the image issues the `a2 & 0x10` and
//   `a2 & 0x80` streaming guards as two SEPARATE `test al, N` probes re-
//   testing `streaming` each time, where this tree's compiled output merges
//   them into one `test al, 0x90`, saving 4 instructions (118 vs image's
//   122) - a legal optimization of the identical-`return;` arms that no
//   restructuring tried suppressed.
// size      293 bytes
// prototype void (__thiscall ?init@Wave@@QAEXPADK@Z)(Wave* this, int8*, unsigned int)
// callers   5   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006005D0 0x006453E0 0x00645460 0x0064557F 0x0064558A 0x006465F0
// indirect  0x004C6A4F 0x004C6A64 0x004C6A70 0x004C6A77 0x004C6A88 0x004C6AAD
Return Value: n/a
Status: Complete
*/
void Wave::init(char *group_id, unsigned long flags) {
    const uint32_t streaming = flags & 4;
    if (streaming && (flags & 0x10)) {
        return;
    }
    if (streaming && (flags & 0x80)) {
        return;
    }
    char *const resolved = filefind_get(group_id);
    if (!resolved) {
        return;
    }
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = WaveOperatorNew(strlen(resolved) + 1);
    strcpy(static_cast<char *>(fname_), resolved);
    // The original's `memset(this + 0x54, 0, 4)` at 0x004C6A1A. This used to be
    // a byte loop, the only way to spell a four-byte clear while the header
    // declared a byte and three pad bytes at 0x54.
    // `memset(&flags_54_, 0, 4)`, not a store. The image clears it that
    // way - `lea esi, [edi+0x54]; push 4; push 0; push esi; call` - and
    // keeps `&flags_54_` in esi afterwards, so every later flag update is
    // one `or dword ptr [esi], imm`. A plain store costs three
    // instructions at each of those six sites.
    memset(&flags_54_, 0, 4);
    if (streaming) {
        flags_54_ |= 2;
        if (!device_ && WaveDeviceReleaseGuard) {
            const int created = (WaveDeviceCreateSlot())(&device_, resolved, 1);
            if (!created) {
                // The device vtable is captured HERE, before the wave's own
                // slot 0x70 composes the attribute mask; the following 0x6C
                // dispatch runs through this captured table on the re-read
                // device, not a freshly re-derived one - as in dyna_load.
                uint8_t *const device_vtable =
                    *reinterpret_cast<uint8_t **>(device_);
                const int attribs = get_attrib();
                {
                    // The int argument is passed on the stack, not in edx;
                    // the pointer-to-member type keeps the thiscall shape.
                    typedef void (OriginalObject::*device_attrib_fn)(int attribs);
                    (ORIGINAL(device_)->*(*reinterpret_cast<device_attrib_fn *>(device_vtable + 0x6C)))(
                        attribs);
                }
                attach();
            }
        }
        if (device_) {
            typedef void (OriginalObject::*device_mode_fn)(uint32_t mode);
            (ORIGINAL(device_)->*vtable_slot<device_mode_fn>(device_, 0x6C))(flags);
        }
    }
    if (flags & 1) {
        flags_54_ |= 1;
    }
    if ((flags & 0x10) && !streaming) {
        flags_54_ |= 4;
    }
    if (flags & 2) {
        set_loop_state(1);
    }
    if (flags & 0x40) {
        flags_54_ |= 8;
    }
    if (flags & 0x80) {
        flags_54_ |= 0x10;
    }
    if (flags & 0x100) {
        flags_54_ |= 0x20;
    }
}


/*
Purpose: The compiler-generated scalar deleting destructor: destroy the wave
         and, when bit 0 of the mode argument asks for it, return the storage
         to the game CRT heap.
// ORIGINAL: 0x004C9300 ??_GWave@@UAEPAXI@Z 0x004C9300-0x004C9321 BYTE_EXACT
// symbol    ?wave_scalar_dtor_redirect@@YIPAXPAVWave@@PAXI@Z
// size      33 bytes
// prototype void* (__thiscall ??_GWave@@UAEPAXI@Z)(Wave* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C67C0 0x0064557F
// LEVER: the qualified destructor call. ~Wave is virtual through ~Sound, so
//        `self->~Wave()` dispatched through the live vtable
//        (`mov eax,[esi]; call [eax+8]`) and dropped the frame pointer - 6 of
//        14. `self->Wave::~Wave()` names the class and emits the direct
//        `call` the image makes; 6/14 -> BYTE_EXACT 14/14.
Return Value: the object pointer
Status: Complete
*/
void *__fastcall wave_scalar_dtor_redirect(Wave *self, void *,
                                           unsigned int mode) {
    // QUALIFIED, not `self->~Wave()`. ~Wave is virtual through ~Sound, so the
    // unqualified form dispatches through the live vtable
    // (`mov eax,[esi]; call [eax+8]`) where the image calls ??1Wave directly.
    self->Wave::~Wave();
    if (mode & 1) {
        operator delete(self);
    }
    return self;
}

// The destructor's dependencies. pull_from_group is the Wave_Device method at
// 0x004C5280 with its singleton receiver at 0x0090D978; the buffer free goes
// to the game CRT's operator delete so the block returns to the heap that
// allocated it; the release hook is an indirect call on the slot at 0x0090DB28
// guarded by the dword at 0x0090DB7C; the chain end slots are the dwords the
// unlink falls back to when a neighbour is null.

/*
Purpose: Destroy the wave. The original is a three-stage teardown of an
         inlined hierarchy, republishing a vtable pointer at each stage: its
         own (0x0066E44C) at entry, the base's (0x0066E3C0) midway, and the
         ultimate base's (0x0066E444) on the way out. While the wave still
         holds a device group slot it is pulled from its group; the filename
         copy goes back to the game CRT heap; a chained wave unlinks itself
         from the global wave chain; the wrapped device is put through the
         release hook when the hook is live, then forgotten. The inlined base
         destructor owns the later filename/device/chain cleanup. The real
         Sound base performs that stage once, with Sound virtual dispatch
         already active. The compiler owns the exception cleanup and final
         SoundFadeState destruction; do not duplicate those stages here.
// ORIGINAL: 0x004C67C0 ??1Wave@@QAE@XZ 0x004C67C0-0x004C68EC;0x004C8450-0x004C8457;0x00659F06-0x00659F20
// symbol    ??1Wave@@UAE@XZ
// LEVER: plain member access, not the `Wave volatile *const self` view. The
//        view predates the real `Sound` base: once the SEH frame is in the
//        body for real, VC6 keeps every store anyway, and the volatile only
//        blocked the hoist of the `flags_40_` read above the two null
//        neighbour stores. 49/101 -> 58/101, instruction count 101/101.
// WALL (unchanged, 2026-09-03): the ONLY four instructions left are the
//        base-destructor stage - the image republishes the mid vtable
//        (0x0066E3C0) at instruction 52, moves the SEH state to 1 at 54, and
//        ends `mov [esi], 0x66e444`; this tree emits `mov ecx,esi /
//        mov [ebp-4],-1 / call` - the OUT-OF-LINE base destructor call the
//        `#pragma auto_inline(off)` below exists to keep. Inlining the base
//        stage would break ~Effect's five-byte tail jump (see that marker and
//        the ~Sound stale-TU note there). Not re-derived here.
// TRIED: 0.888 similar, 2/101 agreeing - already a real destructor,
//            and everything BUT the SEH prologue/epilogue is close; the
//            gap is the missing unwind frame, matching Wave::Wave()'s own
//            ceiling above (same file). The comment above claims the
//            frame is "unreachable, so omitted", which is the SAME
//            reasoning that write_l's own font1_ rebind note (buffer.cpp)
//            turned out to be WRONG about elsewhere in this pass -
//            flagged here rather than trusted, but not re-investigated:
//            unlike FlatButton's destructor (flatbutton.cpp, where the
//            fix was straightforward - just stop routing through a
//            `destroy()` method), this is ALREADY a real destructor, has
//            no base class to call into, and is wrapped in heavy
//            `volatile` reads/writes throughout; reproducing the frame
//            here would need isolating which of those is responsible,
//            which this budget does not cover.
// size      333 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5280 0x0064557F
// indirect  0x004C6886
Return Value: n/a
Status: Complete
*/
Wave::~Wave() {
    // Original 0x004C6856 changes to the Sound table before the release
    // callback at 0x004C6886. Only the Wave stage belongs in this body;
    // the automatic base destructor owns that transition and cleanup.
    if (group_slot_ < 0x10) {
        WaveDeviceGlobal.pull_from_group(this);
    }
    void *const block = fname_;
    if (block) {
        operator delete(block);
    }
    fname_ = nullptr;
    if (flags_40_ & 2) {
        // Unlink from the wave chain. The second neighbour is re-read after
        // the first write because a neighbour pointer may alias this very
        // object - the original re-reads it the same way.
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
Purpose: Tear down an Effect, which is a Wave and nothing more.
// ORIGINAL: 0x004482C0 ??1Effect@@QAE@XZ 0x004482C0-0x004482C5
// symbol    ??1Effect@@UAE@XZ
// CORRECTED from 0x004482C0-0x004482C4, which the artifact carried: the body
//   is one `E9 rel32`, five bytes, and a four-byte span cannot decode a single
//   instruction - `measure` read "0 of 0 instructions, 4 discounted as
//   relocations", which is what a span one byte short looks like.
// TRIED 2026-08-30, post-virtualization sweep (the claim regressed when
//   ~Sound went virtual): empty user dtor measures `mov [ecx],0; ret` - the
//   base-dtor call is ELIDED outright; qualified `this->Wave::~Wave()` gives
//   23 instructions of SEH frame + vptr store + TWO base calls; a no-user-
//   dtor implicit compiles the dtor straight into the fx.cpp caller and the
//   symbol disappears; `#pragma auto_inline(off)` on ~Wave changes nothing.
//   Evidence for the class pass: the image's Sound vtable slot 2 holds
//   ??_GSound (0x004C92D0, verified in the image), so ~Sound IS virtual and
//   the catalogue's QAE spelling hides UAE - yet the original's ??1Effect
//   still folds to a 5-byte tail-jmp, which our VC6 refuses for a virtual
//   base dtor. Something about the original Effect model (a friend-side
//   emission, a different TU split, or /GX flags) remains unmodelled.
//   MEASURED 2026-08-30, VoiceTx class pass: the regression reproduces on the
//   PRISTINE tree - `git checkout HEAD` of every file this pass touched and
//   this body still measures 0/1 (`mov [ecx], <vtable>` + the jump) - so the
//   red claim was inherited, not caused here. The structural reading the
//   evidence supports: the shipped Effect.obj was compiled against a sound.h
//   that did not yet have `virtual ~Sound` (a stale-TU build), which is why
//   the SAME 12.00.8168 compiler both puts ??_GSound in slot 2 of every
//   Sound-family vtable AND emits this storeless tail-jmp - the two shapes
//   cannot come from one consistent header, and no source this tree can hold
//   reproduces both. The model choice stands with the virtual ~Sound, because
//   slot 2 is measured in the image's tables and ??1Effect is one 5-byte
//   thorn; leave the claim red and the TRIED wall standing.
// size      5 bytes
// prototype void (__thiscall ??1Effect@@QAE@XZ)(Effect* this)
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6920
//
// PROMOTED from src/recovered/units/004482c0.cpp. The whole body is the tail
// jump into `??1Wave@@QAE@XZ` that a trivial derived destructor emits, which
// is why `class Effect : public Wave` is the fix and an empty inline stub is
// not - that compiles to nothing at all.
Return Value: n/a
Status: Complete
*/


Effect::~Effect() {
}


/*
Purpose: Initialise an Effect wave. Registers the Effect family's three
         callbacks into the Win hook slots, resolves the filename against the
         fx\\ directory (appending .wav when the name carries no extension),
         and hands the resolved path and the adjusted mode to Wave::init. A
         mode with neither bit 0 nor bit 7 gains bit 4 - the streamed-and-
         looped combination - and a mode with neither bit 2 nor bit 7 runs the
         wave's own vtable slot 0x8C once Wave::init returns.
// ORIGINAL: 0x00445BB0 ?init@Effect@@QAEXPADJ@Z 0x00445BB0-0x00445CC6 FILE
// size      278 bytes
// prototype void (__thiscall ?init@Effect@@QAEXPADJ@Z)(Effect* this, int8*, long)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C69B0 0x00645470 0x00645598 0x00645646 0x00645DD0
// indirect  0x00445CB9
// LEVER: the three callback stores must be `#define`d constants, not const
//   pointer variables - caviar.cpp's lever, re-measured here. A header-defined
//   `const func_effect_callback X = (func_effect_callback)0xADDR;` compiles
//   `mov eax, dword ptr [X]` + `mov [hook], eax` (87 instructions, 6/84);
//   the macro folds the immediate (`mov dword ptr [hook], 0x4457c0`, 84/84
//   instruction count, 79/84 agreeing).
// LEVER: `if (file) { fclose } else { rebuild }` vs the image's arm order.
//   The image falls THROUGH the fopen test into the rebuild and JUMPS to the
//   fclose, so the guard is spelled `if (!file) { rebuild } else { fclose }`.
// TRIED: the mode block ahead of the three stores (72/84), a separate
//   `long mode` local for the adjusted flags (5/84), and the three stores in
//   reverse order (79/84, but lower similarity). Best is stores, then the
//   mode block, then the path build - the image's own order.
// TRIED: /G6 (Pentium-Pro scheduling) on this body, 4/84 - the remaining 5
//   instructions are the three hook stores interleaved into the push/test
//   sequence where the image keeps them contiguous after `mov edi, ecx`.
//   No source ordering tried moved them; scheduling plateau.
// PROMOTED from src/recovered/units/00445bb0.cpp.
Return Value: n/a
Status: Complete
*/
void Effect::init(char *fname, long flags) {
    WinMessageHook = EffectInputCallback;
    EffectOkHookSlot() = ok_callback;
    EffectDoneHookSlot() = EffectDoneCallback;
    if ((static_cast<uint8_t>(flags) & 0x81) == 0) {
        flags |= 4;
    }
    char path[0x100];
    path[0] = '\0';
    // The literals the image pushes are its own data at 0x00683CA4 ("fx\\"),
    // 0x00683CA8 and 0x00683CB4 (".wav" each) and 0x00683CB0 (the "rb" mode).
    // A string literal pushes a relocated address on both sides, so the bytes
    // agree with the image's fixed copies.
    strcat(path, "fx\\");
    strcat(path, fname);
    if (!strchr(path, '.')) {
        strcat(path, ".wav");
    }
    FILE *const file = fopen(path, "rb");
    if (!file) {
        // No such file under fx\\: rebuild the path from the raw argument,
        // extension pass included, and let Wave::init report what it finds.
        path[0] = '\0';
        strcat(path, fname);
        if (!strchr(path, '.')) {
            strcat(path, ".wav");
        }
    } else {
        fclose(file);
    }
    Wave::init(path, flags);
    if ((static_cast<uint8_t>(flags) & 0x84) == 0) {
        load();
    }
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// FX::init supplies these receivers; array teardown independently witnesses
// the CPU bank's 45 elements and 0x6C stride.
// RECOVERY_OBJECT: v1 0x006A7090 ?g_ALPHAMENU_WAVE@@3VWave@@A
Wave g_ALPHAMENU_WAVE;  // 0x006A7090
// RECOVERY_OBJECT: v2 0x0074C5F0 0x0074D8EC ?g_CPU_WAVES@@3PAVWave@@A 0x00445CD0
Wave g_CPU_WAVES[45];  // 0x0074C5F0, 0x6c stride
// RECOVERY_OBJECT: v1 0x00749C18 ?g_SCOOT_WAVE@@3VWave@@A
Wave g_SCOOT_WAVE;  // 0x00749C18
// RECOVERY_OBJECT: v1 0x0074D8F0 ?g_OK_WAVE@@3VWave@@A
Wave g_OK_WAVE;  // 0x0074D8F0

// The signed pitch cycle is loaded and stored as a dword by ok_callback.
// This start binding does not infer the extent of neighboring audio storage.
// RECOVERY_OBJECT: v1 0x0074DAA8 ?EffectOkPitch@@3HA
int EffectOkPitch;

// ORIGINAL: 0x00445650 ?ok_callback@@YAXXZ 0x00445650-0x0044569D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bca75cb0:src/recovered/00445650.cpp", "image:original complete function instructions", "source:src/wave.cpp"], "reason": "Home the original behavioral dependency in its base/audio domain using typed state and existing named callees; retained seed provenance and original spans remain restoration evidence, not runtime acceptance."}
void __cdecl ok_callback() {
    g_OK_WAVE.Wave::set_pitch(EffectOkPitch);
    g_OK_WAVE.Wave::play();
    int pitch = EffectOkPitch;
    if (pitch >= 0) {
        pitch -= 300;
    } else {
        pitch += 200;
    }
    EffectOkPitch = pitch;
    if (pitch <= -1200 || pitch >= 1200) {
        EffectOkPitch = 0;
    }
}
// RECOVERY_OBJECT: v1 0x0074D9D0 ?g_PASSOVER_WAVE@@3VWave@@A
Wave g_PASSOVER_WAVE;  // 0x0074D9D0
Wave g_MAININTERFACE_WAVE;  // 0x007D38B8
Wave g_MULTIWIN_WAVE;  // 0x007FFF00
Wave g_TOP_MENU_WAVE;  // 0x00945780
Wave g_CRASH_LANDING_WAVE;  // 0x00945E08
Wave g_WAVE_GENERAL;  // 0x00945ED0
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
