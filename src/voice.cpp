// RECOVERY_GENERATED: v1 vtable 0x66e82c 0x66e8c0 ??_7VoiceRx@@6B@ 0x4c8960
// RECOVERY_GENERATED: v1 vtable 0x66e8c4 0x66e95c ??_7VoiceTx@@6B@ 0x4c8cc0
// Slot2 is compiler weak _E with library-search mode2 and physical _G fallback;
// that observation does not establish final linked table selection.
#include "stdafx.h"
#include <cstring>
#include <new>
#include "voice.h"
#include "wave.h"
#include "sounddevice.h"
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(memset, strcpy)
#endif

// ORIGINAL: 0x004C8960 ??0VoiceRx@@QAE@XZ 0x004C8960-0x004C8A43;0x004C8450-0x004C8457;0x00659F8A-0x00659FA4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 4C8960/4C8CC0 constructors, 4C8A50/4C8DB0 destructors and 66E82C/66E8C4 tables; native VC6 emits Sound base at0, Rx extent54, Tx extent58 and own flag54.", "631A60 consumes record18/20 and returns the same record; soundx.dll kind7 slots34/35 allocate MMIOINFO48 and enforce pointer identity. Shared cleanup/EH spans remain historical obligations."], "reason": "Voice Rx and Tx genuinely derive from Sound; natural lifecycle stages and typed DLL device contracts replace raw wrappers. Tx returns and consumes the exact borrowed SDK MMIOINFO pointer, not an integer handle."}
// symbol    ??0VoiceRx@@QAE@XZ
VoiceRx::VoiceRx() {
    char name[] = "Voice Rx";
    Sound::set_type(6);
    fname_ = ::operator new(0xA);
    strcpy(static_cast<char *>(fname_), name);
}

// ORIGINAL: 0x004C8A50 ??1VoiceRx@@QAE@XZ 0x004C8A50-0x004C8B35;0x004C8450-0x004C8457;0x00659FA4-0x00659FBE FILE
// symbol    ??1VoiceRx@@UAE@XZ
VoiceRx::~VoiceRx() {
    if (device_ != 0) {
        delete static_cast<VoiceRxDeviceInterface *>(device_);
        device_ = 0;
    }
    if (fname_ != 0) {
        ::operator delete(fname_);
        fname_ = 0;
    }
}

// ORIGINAL: 0x004C8B40 ?init@VoiceRx@@QAEHKPAXK@Z 0x004C8B40-0x004C8BC5 FILE
// symbol    ?init@VoiceRx@@UAEHKPAXK@Z
// TRIED: explicit __thiscall on a plain function-pointer typedef (C4234); a member-function-pointer/union shim compiles. MISMATCH #4 'mov' vs 'push' remains open.
int VoiceRx::init(unsigned long size, void *context, unsigned long flags) {
    int result = 0;
    if (device_ == 0) {
        if (WaveDeviceReleaseGuard == 0) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, 0, 6);
        if (result != 0) {
            return result;
        }
    }
    if (device_ != 0) {
        result = static_cast<VoiceRxDeviceInterface *>(device_)->init(size, context, flags);
        if (result != 0) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device_);
            }
            device_ = 0;
        }
        // Original attaches after a device-init attempt even when that attempt
        // failed and cleared the device. Do not add a device-presence guard.
        attach();
    }
    return result;
}

// ORIGINAL: 0x004C8BD0 ?release@VoiceRx@@QAEHXZ 0x004C8BD0-0x004C8C18 FILE BYTE_EXACT
// symbol    ?release@VoiceRx@@UAEHXZ
int VoiceRx::release() {
    if (device_ != 0) {
        if (static_cast<VoiceRxDeviceInterface *>(device_)->is_playing() == 0) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device_);
            }
        } else {
            static_cast<VoiceRxDeviceInterface *>(device_)->release();
        }
        device_ = 0;
    }
    detach();
    return 0;
}

/*
Purpose: ?start@VoiceRx@@QAEHXZ - dispatch to slot 0x88 of the object at
         0x3c when it is present, and answer 0x0 either way: both
         paths land on the same zeroing, so the delegate's own result
         never reaches the caller.
// ORIGINAL: 0x004C8C20 ?start@VoiceRx@@QAEHXZ 0x004C8C20-0x004C8C32 BYTE_EXACT
// symbol    ?start@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?start@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C29
Return Value: 0x0, whether the delegate ran or not
Status: Complete
*/
int VoiceRx::start() {
    if (device_ != 0) {
        static_cast<VoiceRxDeviceInterface *>(device_)->start();
    }
    return 0;
}

/*
Purpose: ?stop@VoiceRx@@QAEHXZ - dispatch to slot 0x20 of the object at
         0x3c when it is present, and answer 0x0 either way: both
         paths land on the same zeroing, so the delegate's own result
         never reaches the caller.
// ORIGINAL: 0x004C8C40 ?stop@VoiceRx@@QAEHXZ 0x004C8C40-0x004C8C4F BYTE_EXACT
// symbol    ?stop@VoiceRx@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?stop@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C49
Return Value: 0x0, whether the delegate ran or not
Status: Complete
*/
int VoiceRx::stop() {
    if (device_ != 0) {
        static_cast<VoiceRxDeviceInterface *>(device_)->stop();
    }
    return 0;
}

/*
Purpose: ?unload@VoiceRx@@QAEHXZ - forward 0 argument(s) to slot 0x14 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C8C50 ?unload@VoiceRx@@QAEHXZ 0x004C8C50-0x004C8C62
// symbol    ?unload@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?unload@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceRx::unload() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->unload();
    }
    return 0x13;
}

/*
Purpose: ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C8C70 ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z 0x004C8C70-0x004C8C93
// symbol    ?add_buffer@VoiceRx@@UAEHPAU_MMIOINFO@@@Z
// size      35 bytes
// prototype int (__thiscall ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z)(VoiceRx* this, _MMIOINFO*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C80
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceRx::add_buffer(_MMIOINFO *buffer) {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->add_buffer(buffer);
    }
    return 0x13;
}

/*
Purpose: ?get_buffer_size@VoiceRx@@QAEHXZ - forward 0 argument(s) to slot 0x90 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C8CA0 ?get_buffer_size@VoiceRx@@QAEHXZ 0x004C8CA0-0x004C8CB2
// symbol    ?get_buffer_size@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_buffer_size@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int VoiceRx::get_buffer_size() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->get_buffer_size();
    }
    return 0;
}

// It forwards the device's slot 37 result verbatim, or zero when no device
// exists; this is the operation Net's receive worker uses as its level.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C9390 sub_4c9390 0x004C9390-0x004C93A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004c9390.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?get_nbuffers@VoiceRx@@UAEHXZ
int VoiceRx::get_nbuffers() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->get_nbuffers();
    }
    return 0;
}

/*
Purpose: Build a VoiceTx: full volume, centred pan, cleared regions, a
         1000ms default fade, the "Voice Tx" name on the CRT heap, and
         type 7 published through the device slot.
// ORIGINAL: 0x004C8CC0 ??0VoiceTx@@QAE@XZ 0x004C8CC0-0x004C8DAF;0x004C8450-0x004C8457;0x00659FBE-0x00659FD8
// symbol    ??0VoiceTx@@QAE@XZ
// size      239 bytes
// prototype void (__thiscall ??0VoiceTx@@QAE@XZ)(VoiceTx* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x00645460 0x0064558A 0x006465F0
// PROMOTED from src/recovered/units/004c8cc0.cpp. With VoiceTx : public Sound
// the three vtable stages (0x66E444 -> 0x66E3C0 -> 0x66E8C4) are the
// compiler's own base-construction sequence. Sound owns the inherited
// initialization and default fade setup; VoiceTx clears its own flags at
// original 0x004C8D49..0x004C8D5D before allocating the display name.
Return Value: n/a
Status: Complete
*/
VoiceTx::VoiceTx() {
    memset(&flags_54_, 0, sizeof(flags_54_));
    fname_ = ::operator new(0xA);
    strcpy(static_cast<char *>(fname_), "Voice Tx");
    Sound::set_type(7);
}

// ORIGINAL: 0x004C8DB0 ??1VoiceTx@@QAE@XZ 0x004C8DB0-0x004C8EA0;0x004C8450-0x004C8457;0x00659FD8-0x00659FF2 FILE
// symbol    ??1VoiceTx@@UAE@XZ
VoiceTx::~VoiceTx() {
    if (device_ != 0 && WaveDeviceReleaseGuard != 0) {
        SoundXOrdinal2DeleteSound()(device_);
    }
    device_ = 0;
    if (fname_ != 0) {
        ::operator delete(fname_);
        fname_ = 0;
    }
}

/*
Purpose: Drop the Sound-level device (vtable slot 0x38) and forget it, so a
         later init re-wraps; the answer is discarded - release always
         answers 0.
// ORIGINAL: 0x004C8EA0 ?release@VoiceTx@@QAEXXZ 0x004C8EA0-0x004C8EAF BYTE_EXACT
// symbol    ?release@VoiceTx@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?release@VoiceTx@@QAEXXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8EA9
// LEVER: the catalogue spells this `QAEXXZ` (void) - withdrawn here. Both
//        epilogues are `xor eax, eax; ret`, the shape that CORRECTED unload's
//        catalogue name to int. The dispatch differs from Sound::release only
//        in the default: the base returns 0x14 when no device is wrapped,
//        this returns 0. The ?voice_tx_release_redirect@@YIHPAX0@Z spelling
//        that held these bytes first was retired with the delegation_thunks
//        block when this member superseded it.
Return Value: 0, always
*/
int VoiceTx::release() {
    if (device_ != 0) {
        static_cast<VoiceTxDeviceInterface *>(device_)->release();
    }
    return 0;
}

/*
Purpose: Begin voice capture: disable the wave device, start the wave-in
         device, arm the Sound-level device through its own vtable slot 0x90,
         and only then light the started bit at 0x54.
// ORIGINAL: 0x004C8EB0 ?start@VoiceTx@@QAEHXZ 0x004C8EB0-0x004C8EF4 BYTE_EXACT
// symbol    ?start@VoiceTx@@UAEHXZ
// size      68 bytes
// prototype int (__thiscall ?start@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51D0 0x004C5B60
// indirect  0x004C8EDE
// LEVER: PROMOTED out of src/recovered/004c8eb0.cpp, which reached the
//        fields as `reinterpret_cast<char *>(this) + 0x54/0x3c` and the
//        device dispatch through a 37-slot local shim. The named-member
//        spelling reproduces the same bytes: flags_54_ is the dword at 0x54
//        (bit 0, and the image TESTS it as a byte and ORs bit 0 as a byte
//        inside a dword store), device_ is Sound's at 0x3C, and the armed
//        dispatch is slot 36 of the device's live vtable - 0x90, the slot
//        this class's own start occupies in VoiceTx's table 0x0066E8C4.
Return Value: 0, or the first failing stage's answer
*/
int VoiceTx::start() {
    if ((flags_54_ & 1) == 0) {
        int result = WaveDeviceGlobal.disable();
        if (result != 0) return result;
        result = g_WAVE_IN_DEVICE.start_record();
        if (result != 0) return result;
        if (device_ != 0) {
            result = static_cast<VoiceTxDeviceInterface *>(device_)->start();
            if (result != 0) return result;
        }
        flags_54_ |= 1;
    }
    return 0;
}

/*
Purpose: Stop voice capture in the reverse order of start, and tear the
         started bit down first.
// ORIGINAL: 0x004C8F00 ?stop@VoiceTx@@QAEHXZ 0x004C8F00-0x004C8F3C BYTE_EXACT
// symbol    ?stop@VoiceTx@@UAEHXZ
// size      60 bytes
// prototype int (__thiscall ?stop@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51C0 0x004C5B70
// indirect  0x004C8F33
// LEVER: PROMOTED out of src/recovered/004c8f00.cpp the same way as start
//        above. The started-bit clear is a full DWORD and-cycle
//        (`and ecx, 0xfffffffe`) while start's set is a byte `or al, 1`
//        inside a dword read-modify-write - both fall out of the one
//        `flags_54_` dword with plain `&=` / `|=`.
Return Value: 0, or the failing stage's answer
*/
int VoiceTx::stop() {
    if ((flags_54_ & 1) != 0) {
        int result = g_WAVE_IN_DEVICE.end_record();
        if (result != 0) return result;
        flags_54_ &= ~1u;
        WaveDeviceGlobal.enable();
        if (device_ != 0) {
            return static_cast<VoiceTxDeviceInterface *>(device_)->stop();
        }
    }
    return 0;
}

/*
Purpose: Detach from the Sound-level device, stopping voice first, and hand
         the device to the release hook while the guard dword says the hook
         is live - the same hook and guard ~Sound consults.
// ORIGINAL: 0x004C8F40 ?unload@VoiceTx@@QAEHXZ 0x004C8F40-0x004C8F70 BYTE_EXACT
// symbol    ?unload@VoiceTx@@UAEHXZ
// size      48 bytes
// prototype int (__thiscall ?unload@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8F45 0x004C8F4C 0x004C8F5C
// LEVER: PROMOTED out of src/recovered/004c8f40.cpp. The first two
//        dispatches are VIRTUAL SELF-CALLS of slots 8 and 14 - spelled
//        `stop(); release();`, which is why the class model matters here:
//        the shim spelling could not see that they are this class's own
//        stop and release.
Return Value: 0, always
*/
int VoiceTx::unload() {
    stop();
    release();
    if (WaveDeviceReleaseGuard != 0) {
        SoundXOrdinal2DeleteSound()(device_);
    }
    device_ = 0;
    return 0;
}

// ORIGINAL: 0x004C8F70 ?init@VoiceTx@@QAEHKPAXK@Z 0x004C8F70-0x004C9007 FILE
// symbol    ?init@VoiceTx@@UAEHKPAXK@Z
int VoiceTx::init(unsigned long size, void *context, unsigned long flags) {
    unload();
    int result = 0;
    if (device_ == 0) {
        if (WaveDeviceReleaseGuard == 0) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, 0, 7);
        if (result != 0) return result;
    }
    if (device_ != 0) {
        result = static_cast<VoiceTxDeviceInterface *>(device_)->init(size, context, flags);
        if (result != 0 && result != 0x10) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device_);
            }
            device_ = 0;
        }
        if (device_ != 0) {
            attach();
        }
    }
    return result;
}

/*
Purpose: ?get_nbuffers@VoiceTx@@QAEHXZ - forward 0 argument(s) to slot 0x94 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C9010 ?get_nbuffers@VoiceTx@@QAEHXZ 0x004C9010-0x004C9022
// symbol    ?get_nbuffers@VoiceTx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_nbuffers@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int VoiceTx::get_nbuffers() {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->get_nbuffers();
    }
    return 0;
}

/*
Purpose: ?get_next_buffer@VoiceTx@@QAEHXZ - forward 0 argument(s) to slot 0x88 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C9030 ?get_next_buffer@VoiceTx@@QAEHXZ 0x004C9030-0x004C9042
// symbol    ?get_next_buffer@VoiceTx@@UAEPAU_MMIOINFO@@XZ
// size      18 bytes
// prototype int (__thiscall ?get_next_buffer@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
_MMIOINFO *VoiceTx::get_next_buffer() {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->get_next_buffer();
    }
    return 0;
}

/*
Purpose: ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C9050 ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z 0x004C9050-0x004C9073
// symbol    ?return_buffer@VoiceTx@@UAEHPAU_MMIOINFO@@@Z
// size      35 bytes
// prototype int (__thiscall ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z)(VoiceTx* this, _MMIOINFO*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C9060
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceTx::return_buffer(_MMIOINFO *buffer) {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->return_buffer(buffer);
    }
    return 0x13;
}

// ORIGINAL: 0x004C93B0 ??_GVoiceRx@@UAEPAXI@Z 0x004C93B0-0x004C93D1 FILE BYTE_EXACT
// symbol    ??_GVoiceRx@@UAEPAXI@Z
// Compiler-owned VoiceRx scalar deletion mapping.
// ORIGINAL: 0x004C93F0 ??_GVoiceTx@@UAEPAXI@Z 0x004C93F0-0x004C9411 FILE BYTE_EXACT
// symbol    ??_GVoiceTx@@UAEPAXI@Z
// Compiler-owned VoiceTx scalar deletion mapping.
