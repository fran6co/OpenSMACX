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
// ORIGINAL: 0x004C5C50 sub_4c5c50 0x004C5C50-0x004C5C66 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682608 selects startup 0x4c5c50..0x4c5c66 and registers cleanup 0x4c5c70..0x4c5c7a. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?WaveDeviceGlobal@@3VWave_Device@@A is selected by physical COFF constructor/destructor relocations; current emitted initializer is identified by the adjacent symbol fact. Original receiver 0x90d978 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original init operation to the genuine C++ lifetime of ?WaveDeviceGlobal@@3VWave_Device@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E8
// ORIGINAL: 0x004C5C70 sub_4c5c70 0x004C5C70-0x004C5C7A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682608 selects startup 0x4c5c50..0x4c5c66 and registers cleanup 0x4c5c70..0x4c5c7a. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?WaveDeviceGlobal@@3VWave_Device@@A is selected by physical COFF constructor/destructor relocations; current emitted finalizer is identified by the adjacent symbol fact. Original receiver 0x90d978 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original fini operation to the genuine C++ lifetime of ?WaveDeviceGlobal@@3VWave_Device@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E6
#include "stdafx.h"
#include "wave.h"
#include "wave_device.h"
#include "dialog.h"
#include "sounddevice.h"
#include "soundx_wave_device_abi.h"
#include "basepop.h"
#include <cstring>

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004C5180 ?set_pan@Wave_Device@@QAEXH@Z 0x004C5180-0x004C5183 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?set_pan@Wave_Device@@QAEXH@Z)(Wave_Device* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_pan(int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C51A0 ?fade@Wave_Device@@QAEHI@Z 0x004C51A0-0x004C51A5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?fade@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_Device::fade(uint32_t) {
    return 0;
}

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x60.
// ORIGINAL: 0x004C51C0 ?enable@Wave_Device@@QAEXXZ 0x004C51C0-0x004C51CF BYTE_EXACT
// symbol    ?enable@Wave_Device@@QAEHXZ
// LEVER: returns-int - same as `release()`. The image's shared epilogue
// `xor eax,eax; ret` is the int-returning pattern; a void body tail-jumps
// instead. Changing to `int` with `// symbol` produces the `call` + shared
// zero-return that both the null-device and post-call paths reach.
// size      15 bytes
// prototype void (__thiscall ?enable@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C51C9
Return Value: n/a
Status: Complete
*/
int Wave_Device::enable() {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->enable();
    }
    return 0;
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x64.
// ORIGINAL: 0x004C51D0 ?disable@Wave_Device@@QAEXXZ 0x004C51D0-0x004C51DF BYTE_EXACT
// symbol    ?disable@Wave_Device@@QAEHXZ
// LEVER: returns-int - same lever as `enable()` and `release()`.
// size      15 bytes
// prototype void (__thiscall ?disable@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C51D9
Return Value: n/a
Status: Complete
*/
int Wave_Device::disable() {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->disable();
    }
    return 0;
}





/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5170 ?get_volume@Wave_Device@@QAEHXZ 0x004C5170-0x004C5173 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_volume@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_Device::get_volume() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C51B0 ?stop@Wave_Device@@QAEHXZ 0x004C51B0-0x004C51B3 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?stop@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_Device::stop() {
    return 0;
}



/*
Purpose: Ask the wrapped device to suspend, through its vtable slot 0x48.
// ORIGINAL: 0x004C4FC0 ?suspend@Wave_Device@@QAEXXZ 0x004C4FC0-0x004C4FCD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?suspend@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_Device::suspend() {
    void *device = device_14_;
    if (device) {
        static_cast<WaveDeviceProviderCallView *>(device)->suspend();
    }
}

/*
Purpose: Ask the wrapped device to restart, through its vtable slot 0x4C.
// ORIGINAL: 0x004C4FD0 ?restart@Wave_Device@@QAEXXZ 0x004C4FD0-0x004C4FDD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?restart@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_Device::restart() {
    void *device = device_14_;
    if (device) {
        static_cast<WaveDeviceProviderCallView *>(device)->restart();
    }
}

/*
Purpose: Poll the wrapped device, through its vtable slot 0x34.
// ORIGINAL: 0x004C4FE0 ?update_sound@Wave_Device@@QAEXXZ 0x004C4FE0-0x004C4FED BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?update_sound@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_Device::update_sound() {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->update_sound();
    }
}

/*
Purpose: Ask the wrapped device how many devices it offers, through its vtable
         slot 0x14.
// ORIGINAL: 0x004C4FF0 ?get_ndevices@Wave_Device@@QAEHXZ 0x004C4FF0-0x004C4FFF BYTE_EXACT
// symbol    ?get_ndevices@Wave_Device@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?get_ndevices@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_ndevices() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_ndevices();
    }
    return 0;
}

/*
Purpose: Ask the wrapped device for its hardware memory size, through its
         vtable slot 0x58.
// ORIGINAL: 0x004C5020 ?get_hw_mem_size@Wave_Device@@QAEHXZ 0x004C5020-0x004C502F BYTE_EXACT
// size      15 bytes
// prototype int (__thiscall ?get_hw_mem_size@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_hw_mem_size() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_hw_mem_size();
    }
    return 0;
}

/*
Purpose: Ask the wrapped device for its sample rate, through its vtable
         slot 0x3C.
// ORIGINAL: 0x004C5140 ?get_rate@Wave_Device@@QAEHXZ 0x004C5140-0x004C514F BYTE_EXACT
// size      15 bytes
// prototype int (__thiscall ?get_rate@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_rate() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_rate();
    }
    return 0;
}

/*
Purpose: Ask the wrapped device for its DirectSound interface, through its
         vtable slot 0x70.
// ORIGINAL: 0x004C5190 ?get_ds@Wave_Device@@QAEHXZ 0x004C5190-0x004C519F BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Carry the borrowed DirectSound interface through the provider query and wrapper API instead of an integer; preserve the historical catalogue identity and byte obligation.","evidence":["Pinned4C5190 forwards provider slot70 unchanged or returns zero; original caller4BEB76 stores EAX at UV2Player+17C, the embedded MediaDescriptor+54 DirectSound field.","MediaDescriptor+54 participates in DirectSoundCreate, SetCooperativeLevel, GetCaps, CreateSoundBuffer and Release; this query does not add a reference or change provider ownership.","Current VC6 COFF directly observed pointer-return symbol ?get_ds@Wave_Device@@QAEPAUIDirectSound@@XZ; source symbol binding names that candidate, not a changed original identity."]}
// symbol    ?get_ds@Wave_Device@@QAEPAUIDirectSound@@XZ
// size      15 bytes
// prototype int (__thiscall ?get_ds@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
IDirectSound *Wave_Device::get_ds() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_ds();
    }
    return 0;
}

/*
Purpose: Ask the wrapped device whether it supports EAX, through its vtable
         slot 0x74. With no device wrapped the original clears only al, which
         is the codegen for a false of byte width; zero is that value.
// ORIGINAL: 0x004C54B0 ?is_eax@Wave_Device@@QAEHXZ 0x004C54B0-0x004C54BF SEMANTIC
// symbol    ?is_eax@Wave_Device@@QAE_NXZ
// size      15 bytes
// prototype int (__thiscall ?is_eax@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
bool Wave_Device::is_eax() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->is_eax();
    }
    return false;
}









/*
Purpose: Ask the wrapped device whether it is disabled, through vtable slot
         0x68. With no device wrapped the answer is yes.
// ORIGINAL: 0x004C51E0 ?is_disabled@Wave_Device@@QAEHXZ 0x004C51E0-0x004C51F2 BYTE_EXACT
// size      18 bytes
// prototype int (__thiscall ?is_disabled@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 1 when none is wrapped
Status: Complete
*/
int Wave_Device::is_disabled() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->is_disabled();
    }
    return 1;
}

/*
Purpose: Ask the wrapped device to stop dumping raw audio, through vtable slot
         0x54.
// ORIGINAL: 0x004C5220 ?stop_raw_dump@Wave_Device@@QAEHXZ 0x004C5220-0x004C5232 BYTE_EXACT
// size      18 bytes
// prototype int (__thiscall ?stop_raw_dump@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 3 when none is wrapped
Status: Complete
*/
int Wave_Device::stop_raw_dump() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->stop_raw_dump();
    }
    return 3;
}

/*
Purpose: Ask the wrapped device whether it does 3D audio, through vtable slot
         0x84. With no device wrapped the original clears only al, the codegen
         for a false of byte width; zero is that value.
// ORIGINAL: 0x004C5530 ?is_3d@Wave_Device@@QAEHXZ 0x004C5530-0x004C5542 SEMANTIC
// symbol    ?is_3d@Wave_Device@@QAE_NXZ
// size      18 bytes
// prototype int (__thiscall ?is_3d@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
bool Wave_Device::is_3d() {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->is_3d();
    }
    return false;
}

/*
Purpose: Hand the sample rate to the wrapped device, through vtable slot 0x38.
// ORIGINAL: 0x004C5120 ?set_rate@Wave_Device@@QAEXK@Z 0x004C5120-0x004C5137 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_rate@Wave_Device@@QAEXK@Z)(Wave_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5130
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_rate(unsigned long rate) {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->set_rate(rate);
    }
}

/*
Purpose: Hand the volume to the wrapped device, through vtable slot 0x20.
// ORIGINAL: 0x004C5150 ?set_volume@Wave_Device@@QAEXK@Z 0x004C5150-0x004C5167 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_volume@Wave_Device@@QAEXK@Z)(Wave_Device* this, unsigned int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5160
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_volume(unsigned long volume) {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->set_volume(volume);
    }
}

/*
Purpose: Hand the game window to the wrapped device, through vtable slot 0x6C.
// ORIGINAL: 0x004C5000 ?set_hwnd@Wave_Device@@QAEHPAX@Z 0x004C5000-0x004C5020
// TRIED     BYTE_EXACT IS REACHABLE AND COSTS A SEAM, WHICH IS WHY THIS IS STILL UNCLAIMED. The image wants the device path FIRST and the default LAST - it `je`s forward to `mov eax, 0x13` and falls through into the call - where forward_to_wrapped_device is a guard clause. The helper CANNOT be flipped to suit: its other two users, set_rate (0x004C5120) and set_volume (0x004C5150), are BYTE_EXACT with the guard order, so this shape belongs to the body. Written out with the flip it measures BYTE_EXACT, 14 of 14 - and takes `ORIGINAL() named-pointer seam` from 94 to 95, above its ceiling. The seam-free spelling that saved the sounddevice family does not work here because this call TAKES AN ARGUMENT: `__fastcall(void *, int)` passes it in edx while the image passes it on the stack, which is thiscall - the one convention VC6 reserves (C4234). Measured 4 of 14 that way. The claim is real but costs a ratcheted site, and buying that slot by rewriting some OTHER claimed body's seam would risk a live regression to satisfy a counter. Left unclaimed deliberately, with the price written down.
// size      32 bytes
// prototype int (__thiscall ?set_hwnd@Wave_Device@@QAEHPAX@Z)(Wave_Device* this, void*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5010
Return Value: the device's answer, or 0x13 when none is wrapped
Status: Complete
*/
int Wave_Device::set_hwnd(void *hwnd) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_hwnd(static_cast<HWND>(hwnd));
    }
    return 0x13;
}

/*
Purpose: Read one group's volume from the sixteen-entry table of 24-byte
         records the object keeps at 0x28. Nothing is dispatched; an index past
         the table answers zero.
// ORIGINAL: 0x004C5380 ?get_group_volume@Wave_Device@@QAEHI@Z 0x004C5380-0x004C539C BYTE_EXACT
// size      28 bytes
// prototype int (__thiscall ?get_group_volume@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the group's volume, or 0 when the index is out of range
Status: Complete
*/
int Wave_Device::get_group_volume(unsigned int group) {
    if (group > 0xF) {
        return 0;
    }
    int value;
    std::memcpy(&value, &groups_[group].volume, sizeof(value));
    return value;
}








/*
Purpose: Put a wave into one of the sixteen groups. The list-insert helper
         threads it through the group's node list, and the wave's own group
         slot at 0x68 records the group AFTER the insert - the original
         writes the field last, so the helper still sees the old slot.
// ORIGINAL: 0x004C5240 ?add_to_group@Wave_Device@@QAEHIPAUWave@@@Z 0x004C5240-0x004C5277 BYTE_EXACT
// symbol    ?add_to_group@Wave_Device@@QAEHIPAVWave@@@Z
// size      55 bytes
// prototype int (__thiscall ?add_to_group@Wave_Device@@QAEHIPAUWave@@@Z)(Wave_Device* this, unsigned int, Wave*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5BF0
Return Value: 0, or 0xA for a bad group or a null wave
Status: Complete
*/
int Wave_Device::add_to_group(unsigned int group, Wave *wave) {
    if (group > 0xF || !wave) {
        return 0xA;
    }
    reinterpret_cast<WaveGroupList *>(&groups_[group].head)->insert(wave);
    wave->group_slot_ = group;
    return 0;
}


/*
Purpose: Take a wave out of its group. The wave's slot names the group; its
         node is searched for in the group's list and, when found, unlinked
         (head and tail maintained at the ends, the cursor left on the node
         after the removal or nulled), freed to the game heap, and counted
         out. A wave whose node is not on the list - or a group with no list
         at all - just forgets its slot. Either way the wave's slot becomes
         the out-of-range 0x10.
// ORIGINAL: 0x004C5280 ?pull_from_group@Wave_Device@@QAEHPAUWave@@@Z 0x004C5280-0x004C531C BYTE_EXACT
// symbol    ?pull_from_group@Wave_Device@@QAEHPAVWave@@@Z
// size      156 bytes
// prototype int (__thiscall ?pull_from_group@Wave_Device@@QAEHPAUWave@@@Z)(Wave_Device* this, Wave*)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
Return Value: 0, or 0xA for a null wave or an out-of-range slot
Status: Complete
*/
int Wave_Device::pull_from_group(Wave *a1) {
    // ONE SHARED TAIL, NOT TWO. The image jumps both the null-wave check
    // and the out-of-range-slot check to the SAME `mov eax, 0xa; ret`, which
    // a merged `||` condition reproduces; two separate early-return `if`s
    // each got their own inline epilogue instead.
    if (!a1 || a1->group_slot_ >= 0x10) {
        return 0xA;
    }
    const uint32_t slot = a1->group_slot_;
    // THE HEAD-RELATIVE VIEW, NOT THE WHOLE STRUCT. The image computes its
    // group pointer straight to `&groups_[slot].head`
    // (`lea esi, [ecx + eax*8 + 0x2c]`) and addresses head/tail/cursor/count
    // at +0/+4/+8/+0xc from THAT; binding a `WaveControlGroup&` to the
    // struct's own front computes the pointer 8 bytes earlier and every
    // field store lands 8 bytes further out (+8/+0xc/+0x10/+0x14) than the
    // image's.
    WaveGroupList &group =
        *reinterpret_cast<WaveGroupList *>(&groups_[slot].head);
    // ONE LOOP CONDITION, NOT A NESTED GUARD. The image tests `node` for
    // null on every iteration as PART of the loop condition (`test eax,eax;
    // je` shared by both the "list ran out" and "found" exits), then
    // retests it once after the loop to tell which exit it was - a nested
    // `if (!node) return` inside the loop body produces its own separate
    // tail instead of sharing that merge point.
    WaveGroupNode *node = group.head;
    while (node && node->wave != a1) {
        node = node->next;
    }
    if (node) {
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            group.head = node->next;
        }
        if (node->next) {
            // ONE READ OF `node->next`, reused for both the store into it
            // and the store of it into `group.cursor` - the image keeps it
            // in the register the `if` already loaded rather than
            // re-reading `node->next` a second time.
            WaveGroupNode *const next = node->next;
            next->prev = node->prev;
            group.cursor = next;
        } else {
            // `node->prev` READ FIRST, then the cursor cleared, then the
            // tail steps back - the image loads it into a register before
            // storing the cursor's zero, not after.
            WaveGroupNode *const prev = node->prev;
            group.cursor = nullptr;
            group.tail = prev;
        }
        operator delete(node);
        group.count -= 1;
    }
    a1->group_slot_ = 0x10;
    return 0;
}


/*
Purpose: Report whether a group is disabled: out-of-range groups always are,
         and a real one is disabled while its enabled byte is zero. The
         original defines only AL on the out-of-range path; callers test the
         byte.
// ORIGINAL: 0x004C5460 ?is_group_disabled@Wave_Device@@QAEHI@Z 0x004C5460-0x004C5481
// symbol    ?is_group_disabled@Wave_Device@@QAE_NI@Z
// TRIED: byte-exactness. Plateaus at 7/14, 0.875 similar (best
// `/c /O2 /Gy /GR- /Oy- /GX`). The image writes only AL on BOTH return
// paths - `mov al, 1` (2 bytes) on the out-of-range path and a bare
// `sete al` on the lookup path, each its own independent `ret`, never a
// full `mov eax`/zero-extend even though the mangled return type is `H`
// (int). This tree's compiled form always widens: `mov eax, 1` on the
// first path, and `xor edx,edx; ...; sete dl; mov eax,edx` (with a
// spurious `push ebx`/`pop ebx` for the extra register) on the second.
// Measured `return 1` vs `return true`, and `groups_[a1].enabled == 0`
// as a direct boolean return vs the `? 1 : 0` ternary - all four
// combinations compile identically (still 0.875). Whatever makes the
// image settle for AL-only appears to be a VC6 quirk this tree has not
// found the source shape for; not chased further.
// LEVER: bool-return the mangled return type is what selects AL-only stores.
//   With `bool` the out-of-range arm compiles the image's own `mov al, 1`
//   (B0 01); every int spelling measured above widened to `mov eax, 1`.
// LEVER: const-bool-local CLOSED 2026-09-18: naming the comparison result -
//   `const bool disabled = groups_[group].enabled == 0; return disabled;` -
//   drops the temp machinery entirely: the byte loads straight into dl and
//   `sete al` fires in place, 33 of 33 bytes on /O2 /Oy- (measured
//   byte-exact; the plateau above was the direct `return <expr>` spelling,
//   which kept `xor edx,edx; mov bl; sete dl; mov al,dl` plus the ebx
//   save). A branchy `if (enabled) return false; return true;` twin
//   compiles to the same 33 bytes.
// size      33 bytes
// prototype int (__thiscall ?is_group_disabled@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when disabled, 0 when enabled
Status: Complete
*/
bool Wave_Device::is_group_disabled(unsigned int group) {
    if (group > 0xF) {
        return true;
    }
    const bool disabled = groups_[group].enabled == 0;
    return disabled;
}


/*
Purpose: Append a wave to a group's node list. A fresh 12-byte node comes
         from the game heap and joins at the tail; an empty list makes it
         both ends. The original presses on when the allocation fails: a
         live tail would fault writing the null node's prev, and an empty
         list quietly "appends" the null while still counting it - both
         shapes are kept. With a live tail the new node's prev is the
         RE-READ tail field, after the old tail's next was written - an
         order the original's aliasing permits to matter.
// ORIGINAL: 0x004C5BF0 sub_4c5bf0 0x004C5BF0-0x004C5C4A BYTE_EXACT
// symbol    ?insert@WaveGroupList@@QAEXPAVWave@@@Z
// kind      game
// size      90 bytes
// callers   1   call targets   1
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064558A
// LEVER: `WaveGroupNode *const node` -> non-const with an explicit
// `else { node = 0; }`. With the pointer const-initialized from the `if
// (node) {...}` alone, this tree let the null path fall straight into the
// tail-check with `eax` already zero from the allocator's own return,
// dropping the image's own `jmp`/`xor eax,eax` merge pair (0x004C5C17-
// 0x004C5C19) - 34/36 instructions, 0.971 similar. Materializing the
// redundant zero explicitly reproduces the image's (needless) re-zero and
// closes the last two instructions.
Return Value: n/a
Status: Complete
*/
void WaveGroupList::insert(Wave *a1) {
    WaveGroupNode *node =
        static_cast<WaveGroupNode *>(operator new(0xC));
    if (node) {
        node->prev = nullptr;
        node->next = nullptr;
        node->wave = a1;
    } else {
        node = 0;
    }
    WaveGroupNode *const tail = this->tail;
    if (tail) {
        // The old tail's next is written first and the tail field re-read
        // second, exactly as the original orders its memory operations; the
        // volatile views keep the pair honest when the two locations alias
        // (the optimizer's type-based analysis would otherwise fold the
        // re-read).
        *reinterpret_cast<WaveGroupNode *volatile *>(&tail->next) = node;
        node->prev = *reinterpret_cast<WaveGroupNode *volatile *>(&this->tail);
        this->tail = node;
        this->count += 1;
    } else {
        this->head = node;
        this->tail = node;
        this->count += 1;
    }
}

// The Wave_Device forwarder family over the wrapped device at 0x14: each
// method dispatches through one slot of the device's vtable with the device
// as receiver, or answers a fixed default with no device (0x14 for most,
// 3 for the raw dump, nothing at all for get_description).

/*
Purpose: Forward get_description to the wrapped device through its vtable slot
         0x1c.
// ORIGINAL: 0x004C50F0 ?get_description@Wave_Device@@QAEXKPADK@Z 0x004C50F0-0x004C5111 BYTE_EXACT
// symbol    ?get_description@Wave_Device@@UAEHKPADK@Z
// size      33 bytes
// prototype void (__thiscall ?get_description@Wave_Device@@QAEXKPADK@Z)(Wave_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5108
Return Value: n/a
Status: Complete
*/
int Wave_Device::get_description(unsigned long a1, char *a2, unsigned long a3) {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->get_description(a1, a2, a3);
    }
    // THE IMAGE RETURNS 0 ON BOTH PATHS - its `xor eax, eax` sits AFTER the
    // branch rejoins, not in the no-device arm, so the forwarded call's value
    // is discarded. Returning it instead compiles 18 instructions against the
    // image's 16.
    return 0;
}


/*
Purpose: Forward start_raw_dump to the wrapped device through its vtable slot
         0x50.
// ORIGINAL: 0x004C5200 ?start_raw_dump@Wave_Device@@QAEHPAD@Z 0x004C5200-0x004C5220 BYTE_EXACT
// size      32 bytes
// prototype int (__thiscall ?start_raw_dump@Wave_Device@@QAEHPAD@Z)(Wave_Device* this, int8*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5210
Return Value: the device's answer, or 3 when no device is wrapped
Status: Complete
*/
int Wave_Device::start_raw_dump(char *path) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->start_raw_dump(path);
    }
    return 3;
}


/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x7c.
// ORIGINAL: 0x004C54C0 ?set_eax@Wave_Device@@QAEHPAUEAX_REVERB_PROPERTIES@@@Z 0x004C54C0-0x004C54E0 BYTE_EXACT
// size      32 bytes
// prototype int (__thiscall ?set_eax@Wave_Device@@QAEHPAUEAX_REVERB_PROPERTIES@@@Z)(Wave_Device* this, EAX_REVERB_PROPERTIES*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C54D0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax(EAX_REVERB_PROPERTIES *properties) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_eax(properties);
    }
    return 0x14;
}


/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x78.
// ORIGINAL: 0x004C54E0 ?set_eax@Wave_Device@@QAEHK@Z 0x004C54E0-0x004C5500 BYTE_EXACT
// size      32 bytes
// prototype int (__thiscall ?set_eax@Wave_Device@@QAEHK@Z)(Wave_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C54F0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax(unsigned long properties) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_eax(properties);
    }
    return 0x14;
}


/*
Purpose: Forward set_eax_mix to the wrapped device through its vtable slot
         0x80.
// ORIGINAL: 0x004C5500 ?set_eax_mix@Wave_Device@@QAEHM@Z 0x004C5500-0x004C5523 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?set_eax_mix@Wave_Device@@QAEHM@Z)(Wave_Device* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5510
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax_mix(float mix) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_eax_mix(mix);
    }
    return 0x14;
}


/*
Purpose: Forward set_listener_position to the wrapped device through its vtable slot
         0x88.
// ORIGINAL: 0x004C5550 ?set_listener_position@Wave_Device@@QAEHMMM@Z 0x004C5550-0x004C557B BYTE_EXACT
// size      43 bytes
// prototype int (__thiscall ?set_listener_position@Wave_Device@@QAEHMMM@Z)(Wave_Device* this, float, float, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5568
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_position(float x, float y, float z) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_listener_position(x, y, z);
    }
    return 0x14;
}


/*
Purpose: Forward get_listener_position to the wrapped device through its vtable slot
         0x8c.
// ORIGINAL: 0x004C5580 ?get_listener_position@Wave_Device@@QAEHPAM00@Z 0x004C5580-0x004C55AB BYTE_EXACT
// size      43 bytes
// prototype int (__thiscall ?get_listener_position@Wave_Device@@QAEHPAM00@Z)(Wave_Device* this, float*, float*, float*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5598
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_position(float *x, float *y, float *z) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_listener_position(x, y, z);
    }
    return 0x14;
}


/*
Purpose: Forward set_listener_xpos to the wrapped device through its vtable slot
         0x90.
// ORIGINAL: 0x004C55B0 ?set_listener_xpos@Wave_Device@@QAEHM@Z 0x004C55B0-0x004C55D3 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?set_listener_xpos@Wave_Device@@QAEHM@Z)(Wave_Device* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C55C0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_xpos(float x) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_listener_xpos(x);
    }
    return 0x14;
}


/*
Purpose: Forward get_listener_xpos to the wrapped device through its vtable slot
         0x94.
// ORIGINAL: 0x004C55E0 ?get_listener_xpos@Wave_Device@@QAEHPAM@Z 0x004C55E0-0x004C5603 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?get_listener_xpos@Wave_Device@@QAEHPAM@Z)(Wave_Device* this, float*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C55F0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_xpos(float *x) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_listener_xpos(x);
    }
    return 0x14;
}


/*
Purpose: Forward set_listener_ypos to the wrapped device through its vtable slot
         0x98.
// ORIGINAL: 0x004C5610 ?set_listener_ypos@Wave_Device@@QAEHM@Z 0x004C5610-0x004C5633 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?set_listener_ypos@Wave_Device@@QAEHM@Z)(Wave_Device* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5620
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_ypos(float y) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_listener_ypos(y);
    }
    return 0x14;
}


/*
Purpose: Forward get_listener_ypos to the wrapped device through its vtable slot
         0x9c.
// ORIGINAL: 0x004C5640 ?get_listener_ypos@Wave_Device@@QAEHPAM@Z 0x004C5640-0x004C5663 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?get_listener_ypos@Wave_Device@@QAEHPAM@Z)(Wave_Device* this, float*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5650
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_ypos(float *y) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_listener_ypos(y);
    }
    return 0x14;
}


/*
Purpose: Forward set_listener_zpos to the wrapped device through its vtable slot
         0xa0.
// ORIGINAL: 0x004C5670 ?set_listener_zpos@Wave_Device@@QAEHM@Z 0x004C5670-0x004C5693 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?set_listener_zpos@Wave_Device@@QAEHM@Z)(Wave_Device* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5680
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_zpos(float z) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->set_listener_zpos(z);
    }
    return 0x14;
}


/*
Purpose: Forward get_listener_zpos to the wrapped device through its vtable slot
         0xa4.
// ORIGINAL: 0x004C56A0 ?get_listener_zpos@Wave_Device@@QAEHPAM@Z 0x004C56A0-0x004C56C3 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?get_listener_zpos@Wave_Device@@QAEHPAM@Z)(Wave_Device* this, float*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C56B0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_zpos(float *z) {
    if (device_14_) {
        return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_listener_zpos(z);
    }
    return 0x14;
}


// The removal-safe walk the three group administrators each carry INLINE:
// the cursor field is re-read after every callback (a wave's handler may
// pull waves from the group, and pull_from_group maintains the cursor, so
// the walk survives removal - and a self-removal skips the follower, exactly
// the original's advance rule), and a node with a null wave stops the walk.
// The image carries three copies of the loop with no shared helper call; the
// per-wave dispatches (volume slot 0x40, resume slot 0x8C, halt slot 0x14)
// are spelled inside each administrator's loop.

/*
Purpose: Set a group's volume scale and replay every member wave's own
         stored volume through its vtable slot 0x40, so the new scale takes
         effect. Bad slots and volumes above 0x7F answer 0xA.
// ORIGINAL: 0x004C5320 ?set_group_volume@Wave_Device@@QAEHII@Z 0x004C5320-0x004C537F BYTE_EXACT
// TRIED: inlining the walk - EARLIER CONCLUSION DISPROVEN 2026-09-18. An
//   earlier round measured spelled-out walks "hundreds of bytes further"
//   and concluded the image calls a shared walk out-of-line; decoded image
//   bytes say otherwise: all three administrators carry the cursor loop
//   INLINE (no call), and those earlier trials failed for two unrelated
//   reasons - they addressed the group through a `WaveControlGroup&`
//   (which folds the array base +0x24 into the lea, moving every field
//   disp), and their whole-file byte sums still counted the leftover
//   out-of-line helper. The spelling below is instruction-identical to
//   0x4C5320-0x4C537F on /O2 /Oy-: 40 of 40 instructions, byte-exact.
// LEVER: list-view-mixed-addressing the walk reads head/cursor through a
//   `WaveGroupList&` pinned at `&groups_[a1].head` while the volume store
//   and guard stay struct-subscripted. CSE then unifies the shared base as
//   `esi = this + 24*a1` WITHOUT folding the array base: enabled +0x24,
//   volume +0x28, head +0x2c, cursor +0x34 - the image's own disps. A pure
//   struct view (`WaveControlGroup& group`) folds +0x24 into the lea and
//   every disp lands 8 bytes off.
// LEVER: break-not-return the loop exits spell `break` out of the while,
//   reaching ONE trailing `return 0`. Early `return 0`s inside the loop
//   each grow their own epilogue (112 bytes against 95) and flip the
//   entry-test polarity (jne-to-body); breaks keep the image's je-to-exit
//   and the shared xor eax,epilogue at the end.
// size      95 bytes
// prototype int (__thiscall ?set_group_volume@Wave_Device@@QAEHII@Z)(Wave_Device* this, unsigned int, unsigned int)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5354
Return Value: 0, or 0xA for a bad slot or volume
Status: Complete
*/
int Wave_Device::set_group_volume(unsigned int a1, unsigned int a2) {
    if (a1 > 0xF || a2 > 0x7F) {
        return 0xA;
    }
    groups_[a1].volume = a2;
    WaveGroupList &list =
        *reinterpret_cast<WaveGroupList *>(&groups_[a1].head);
    WaveGroupNode *const first = list.head;
    list.cursor = first;
    if (first) {
        Wave *wave = first->wave;
        while (wave) {
            int volume;
            std::memcpy(&volume, &wave->volume_, sizeof(volume));
            wave->set_volume(volume);
            WaveGroupNode *const cursor = list.cursor;
            if (!cursor) {
                break;
            }
            WaveGroupNode *const next = cursor->next;
            list.cursor = next;
            if (!next) {
                break;
            }
            wave = next->wave;
        }
    }
    return 0;
}


/*
Purpose: Enable a group. Only a disabled one does any work: every member
         wave resumes through its vtable slot 0x8C, and the enabled byte is
         written AFTER the walk. An already-enabled group is left exactly as
         it is, nonzero byte and all.
// ORIGINAL: 0x004C53A0 ?enable_group@Wave_Device@@QAEHI@Z 0x004C53A0-0x004C5400 BYTE_EXACT
// LEVER: early-return-0 the already-enabled case is an EARLY `return 0`,
//   not the walk nested inside `if (!enabled)`: the range guard, the
//   enabled test and the walk sit at function level in sequence. That
//   layout is what makes VC6 save esi in the prologue (`mov eax,[ebp+8];
//   push esi; cmp`) and pop it inside the 0xA tail - the image's own
//   placement. With the walk nested, VC6 lays the guard fail out as a
//   distant shared tail (`ja` to the end, like set_group_volume and
//   add_to_group) and saves esi late at its first def (`lea eax; push esi;
//   lea esi`), one byte moved and 86 of 96 offsets displaced.
// LEVER: inline-break-walk same cursor loop as set_group_volume: written
//   out with `break` exits, list view for head/cursor, struct subscript
//   for the enabled byte, store AFTER the walk.
// TRIED: 2026-09-18 five-spelling deck, both flag sets: this spelling is
//   BYTE-EXACT on /O2 /Oy- (0 of 96 differing, references resolved);
//   body-in-if (`if (a1 <= 0xF) { ... }`), a goto body-label, a hoisted
//   `WaveGroupNode *first = nullptr;` above the guard (dead store,
//   optimized away - unlike select's resume_tail there is no honest read
//   site for a null here) and a hoisted `WaveControlGroup *const group =
//   &groups_[a1];` all measured worse or unchanged. Earlier rounds'
//   guard-first list-hoist and hoisted-enabled-local spellings stay
//   measured-worse (esi via edx before the guard; enabled load re-based
//   through esi).
// size      96 bytes
// prototype int (__thiscall ?enable_group@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C53D7
Return Value: 0, or 0xA for a bad slot
Status: Complete
*/
int Wave_Device::enable_group(unsigned int a1) {
    if (a1 > 0xF) {
        return 0xA;
    }
    if (groups_[a1].enabled) {
        return 0;
    }
    WaveGroupList &list =
        *reinterpret_cast<WaveGroupList *>(&groups_[a1].head);
    WaveGroupNode *const first = list.head;
    list.cursor = first;
    if (first) {
        Wave *wave = first->wave;
        while (wave) {
            wave->load();
            WaveGroupNode *const cursor = list.cursor;
            if (!cursor) {
                break;
            }
            WaveGroupNode *const next = cursor->next;
            list.cursor = next;
            if (!next) {
                break;
            }
            wave = next->wave;
        }
    }
    groups_[a1].enabled = 1;
    return 0;
}


/*
Purpose: Disable a group. Only an enabled one does any work: every member
         wave halts through its vtable slot 0x14, and the enabled byte is
         cleared AFTER the walk.
// ORIGINAL: 0x004C5400 ?disable_group@Wave_Device@@QAEHI@Z 0x004C5400-0x004C545D BYTE_EXACT
// LEVER: early-return-0 the halt twin of enable_group's winning spelling:
//   the not-enabled case is an EARLY `return 0`, the walk sits at function
//   level, and the enabled byte is cleared AFTER the walk. This layout is
//   what puts `push esi` in the prologue before the range compare and its
//   pop inside the 0xA tail, the image's own placement.
// LEVER: inline-break-walk the halt twin of enable_group's cursor loop
//   (vtable slot 0x14, `break` exits, list view for head/cursor, struct
//   subscript for the enabled byte).
// TRIED: 2026-09-18 three-spelling deck, both flag sets: this spelling is
//   BYTE-EXACT on /O2 /Oy- (0 of 93 differing, references resolved);
//   body-in-if measured worse, and a goto body-label spelling changed the
//   function's extent (boundary fail - bytes unmeasurable).
// size      93 bytes
// prototype int (__thiscall ?disable_group@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5437
Return Value: 0, or 0xA for a bad slot
Status: Complete
*/
int Wave_Device::disable_group(unsigned int a1) {
    if (a1 > 0xF) {
        return 0xA;
    }
    if (!groups_[a1].enabled) {
        return 0;
    }
    WaveGroupList &list =
        *reinterpret_cast<WaveGroupList *>(&groups_[a1].head);
    WaveGroupNode *const first = list.head;
    list.cursor = first;
    if (first) {
        Wave *wave = first->wave;
        while (wave) {
            wave->unload();
            WaveGroupNode *const cursor = list.cursor;
            if (!cursor) {
                break;
            }
            WaveGroupNode *const next = cursor->next;
            list.cursor = next;
            if (!next) {
                break;
            }
            wave = next->wave;
        }
    }
    groups_[a1].enabled = 0;
    return 0;
}


/*
Purpose: Switch the wrapped device to another output. Every sound on the
         global chain whose attribute word has bit 1 set and bit 4 clear
         (playing, not protected from the switch) is halted - which detaches
         it - and threaded onto a private resume list through its freed
         chain-link fields. The device then selects the new output through
         its vtable slot 0x18, and the list replays tail-first: each sound's
         links are cleared and it reloads by its remembered filename. The
         attribute word is fetched twice per chained sound, and a sound
         appended to the resume list keeps whatever its next link held until
         the replay clears it.
// ORIGINAL: 0x004C5030 ?select@Wave_Device@@QAEHK@Z 0x004C5030-0x004C50EF BYTE_EXACT
// symbol    ?select@Wave_Device@@UAEHK@Z
// LEVER: declare-before-the-guard `Wave *resume_tail = nullptr;` above the
//   `if (!device_14_) return 2;` early exit, not below it. The image zeroes edi
//   at 0x004C503B - BEFORE the guard's compare - and pops it again inside the
//   early-return epilogue at 0x004C5044, so resume_tail is live across the
//   guard. Declaring it after the guard costs the whole prologue and every
//   register after it: 14 of 88 below the guard, 62 of 88 above it.
// LEVER: guard-polarity `if (!resume_tail) { prev = 0; next = 0; } else {...}`.
//   The image FALLS THROUGH to the empty-list arm and jumps to 0x004C5096 for
//   the append, so the null test is the fall-through. Written the other way up
//   the two arms swap and the jumps invert.
// LEVER: tail-is-the-loop-variable the replay loop walks `resume_tail` itself -
//   `Wave *const replay = resume_tail; resume_tail = wave_chain_prev(...);` -
//   rather than seeding a separate `replay` cursor from it. The image keeps the
//   list head in edi and copies it to esi INSIDE the loop (0x004C50C8); a
//   separate cursor emits one extra `mov esi, edi` before the loop, 89
//   instructions against the image's 88. 62/88 -> 73/88.
// LEVER: prev-before-fname read `wave_chain_prev` first and the filename after
//   it (0x004C50CA then 0x004C50CD), not the other way round.
// TRIED: vtable-register-alternation the last 15 instructions are register
//   choice with identical shape and an identical instruction count (88 v 88,
//   0.989 similar): the image loads the vtable pointer into edx for the second
//   and fourth indirect calls where this tree reuses eax, because it issues
//   `mov edx, [esi]` BEFORE `mov ebx, eax` saves the chain-next result and this
//   tree issues them the other way round. `osmx semantic` refuses on exactly
//   that, "instruction 34: mov operand is a different KIND". Hoisting the
//   `next` local, splitting the two attribute tests into separate `if`s, and
//   assigning `resume_tail`/`sound` inside both arms were all measured: 0/88,
//   0/88 and 70/88 against the 73/88 that stands.
// LEVER: call_diff showed 14 real calls to wave_attrib/wave_chain_next/
//        halt_wave/resume_links/load_wave_by_name where the image has 0 -
//        those anonymous-namespace one-liners were not being inlined at
//        their direct call sites (this file's winning flags carry no
//        /Ob0, so it is not that lever). Marking all five `__forceinline`
//        dropped every one of those calls and moved best similarity 0.584
//        -> 0.877 (/O2 /Gy /GR- /GX). The remaining gap is the same
//        missing-ebp-frame register/scheduling plateau documented on the
//        rest of this family; not chased further.
// size      191 bytes
// prototype int (__thiscall ?select@Wave_Device@@QAEHK@Z)(Wave_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5060 0x004C506B 0x004C5076 0x004C507F 0x004C50A8 0x004C50BD 0x004C50CA 0x004C50DD
Return Value: 0, or 2 when no device is wrapped
Status: Complete
*/
int Wave_Device::select(unsigned long a1) {
    Sound *resume_tail = nullptr;
    if (!device_14_) {
        return 2;
    }
    Sound *sound = SoundChainHead;
    while (sound) {
        if ((sound->get_attrib() & 1) && !(sound->get_attrib() & 4)) {
            Sound *const next = sound->get_next();
            sound->unload();
            if (!resume_tail) {
                sound->chain_prev_ = nullptr;
                sound->chain_next_ = nullptr;
            } else {
                resume_tail->chain_next_ = sound;
                sound->chain_prev_ = resume_tail;
            }
            resume_tail = sound;
            sound = next;
        } else {
            sound = sound->get_next();
        }
    }
    static_cast<WaveDeviceProviderCallView *>(device_14_)->select(a1);
    while (resume_tail) {
        Sound *const replay = resume_tail;
        resume_tail = resume_tail->get_prev();
        const char *const fname = replay->fname_;
        replay->chain_next_ = nullptr;
        replay->chain_prev_ = nullptr;
        replay->load(fname);
    }
    return 0;
}



/*
Purpose: Build the wrapped device of the requested kind through the factory
         hook, straight into the 0x14 field. A device already wrapped is
         refused with 0xC; a dead factory answers 0x14.
// ORIGINAL: 0x004C4ED0 ?create_device@Wave_Device@@QAEHK@Z 0x004C4ED0-0x004C4F07 BYTE_EXACT
// symbol    ?create_device@Wave_Device@@UAEHK@Z
// LEVER: hoist-and-read-once the image hoists `lea eax, [ecx+0x14]` ahead of
//   the device test and reads the factory hook ONCE into a register
//   (`mov ecx, [0x90db34]`); the body spells both - a local slot pointer
//   first, a single hook read second - and the factory call passes that
//   pointer, not a fresh `&device_14_` at the call site.
// LEVER: inverted-guard-tail-order the image's first guard `jnz` jumps
//   FORWARD over everything, so the success arm leads and the two failure
//   values tail in fall-through order: factory answer first, 0x14 second,
//   0xC LAST. Spelling it `if (!device_14_) { factory? call : 0x14 } return
//   0xC;` reproduces that layout - measured 0 differing bytes on
//   /O2 /Oy- (the guard-first spelling kept 34). The function still
//   verifies as unknown, not pass: its one relocation reads
//   ?WaveDeviceFactoryHook@@3P6AHPAPAXK@ZA, and the shipped image carries
//   no symbol on the 0x0090DB34 dword to prove link selection against.
// size      55 bytes
// prototype int (__thiscall ?create_device@Wave_Device@@QAEHK@Z)(Wave_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C4EEC
Return Value: the factory's answer, 0xC with a device already wrapped, or
              0x14 with no factory installed
Status: Complete
*/
int Wave_Device::create_device(unsigned long a1) {
    void **const device_slot = &device_14_;
    if (!device_14_) {
        // Original 0x0090DB34 is ordinal 5 in the loader's export array.
        SoundXWaveDeviceCreateProc const factory =
            SoundXOrdinal5WaveDeviceCreate();
        if (factory) {
            return factory(device_slot, a1);
        }
        return 0x14;
    }
    return 0xC;
}


/*
Purpose: Tear the wrapped device down through the argument-less destroy hook
         and forget it. Without a device or a hook, 0x14 and nothing happens.
// ORIGINAL: 0x004C4F10 ?delete_device@Wave_Device@@QAEHXZ 0x004C4F10-0x004C4F37 BYTE_EXACT
// symbol    ?delete_device@Wave_Device@@UAEHXZ
// LEVER: shared-tail-one-store both failure paths reach ONE `mov eax, 0x14`
//   tail (nested guards, not two early returns) and the field is stored
//   once, on the success arm only; the hook is read once into a local.
// size      39 bytes
// prototype int (__thiscall ?delete_device@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C4F23
Return Value: 0, or 0x14 with no device or no hook
Status: Complete
*/
int Wave_Device::delete_device() {
    if (device_14_) {
        // Original 0x0090DB38 is ordinal 6, not independent pointer storage.
        SoundXWaveDeviceDeleteProc const hook =
            SoundXOrdinal6WaveDeviceDelete();
        if (hook) {
            hook();
            device_14_ = nullptr;
            return 0;
        }
    }
    return 0x14;
}


/*
Purpose: Initialise the device stack: the Wave_Device's OWN virtual slot 0
         takes the mode first through create_device(), then the wrapped
         device initialises through its slot
         0xC with both arguments, and a failure there runs the Wave_Device's
         own slot-1 delete_device method before the error propagates.
// ORIGINAL: 0x004C4F40 ?init@Wave_Device@@QAEHPAXK@Z 0x004C4F40-0x004C4F7D BYTE_EXACT
// symbol    ?init@Wave_Device@@UAEHPAXK@Z
// size      61 bytes
// prototype int (__thiscall ?init@Wave_Device@@QAEHPAXK@Z)(Wave_Device* this, void*, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C4F4D 0x004C4F5D 0x004C4F6A
Return Value: 0, or whichever stage's error came first
Status: Complete
*/
int Wave_Device::init(void *group_id, unsigned long flags) {
    const int staged = create_device(flags);
    if (staged) {
        return staged;
    }
    void *device = device_14_;
    const int result = static_cast<WaveDeviceProviderCallView *>(device)->init(group_id, flags);
    if (result) {
        delete_device();
        return result;
    }
    return 0;
}


/*
Purpose: Release the wrapped device: its own vtable slot 0x10 winds it down,
         and only if the device is STILL there afterwards - the callback may
         have cleared it - does the destroy hook run. Either way the field is
         forgotten.
// ORIGINAL: 0x004C4F80 ?release@Wave_Device@@QAEXXZ 0x004C4F80-0x004C4FB3 BYTE_EXACT
// LEVER: returns-int the catalogued name decodes `X` (void), but `xor eax, eax` at 0x004C4FAF is a MERGE POINT - the guard's `je 0x4c4faf` and the fall-through from the trailing store both reach it before `pop esi; ret`. That is a shared `return 0`, not a side effect. Same correction as `?release@Wave_In_Device@@QAEHXZ` (0x004C5A50) and `Midi_Device::release`. With the void head the body is 2 bytes short and ends at 17 of 19.
// LEVER: store-twice the field is cleared INSIDE the destroy-hook arm and again after it, which is why 0x004C4FA1 and 0x004C4FA8 are two identical `mov [esi+0x14], 0` that fall into one another. One store scores 17/19.
// LEVER: hook-global the destroy-hook read at 0x004C4FA6 is `mov eax,
//   dword ptr [0x90db38]` carrying an ABSOLUTE RELOCATION in the image; a raw
//   fixed-address cast compiles the same bytes with no relocation, which is
//   what failed the reference check at offset 23. The slot is a real global
//   (WaveDeviceDestroyHook, defined below with its 0x0090DB38 binding) - the
//   same treatment as WaveDeviceGlobal and the release guard. The relocation
//   itself verifies as unknown rather than pass: the shipped image has no
//   symbol on that dword, so nothing can prove link selection.
// symbol    ?release@Wave_Device@@UAEHXZ
// size      51 bytes
// prototype void (__thiscall ?release@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C4F8C 0x004C4F9F
Return Value: zero
Status: Complete
*/
int Wave_Device::release() {
    if (device_14_) {
        static_cast<WaveDeviceProviderCallView *>(device_14_)->release();
        if (device_14_) {
            SoundXWaveDeviceDeleteProc const hook =
                SoundXOrdinal6WaveDeviceDelete();
            if (hook) {
                hook();
                device_14_ = nullptr;
            }
        }
        device_14_ = nullptr;
    }
    return 0;
}


/*
Purpose: Initialize one control group: clear head, tail, cursor and count,
         then set full volume and enable the group. Native4C54A0/4C54A7
         establish the last two stores; the old mapped wrapper omitted them.
// ORIGINAL: 0x004C5490 ??0WaveControlGroup@@QAE@XZ 0x004C5490-0x004C54AB
// Historical project redirect symbol: ?wave_control_group_ctor_redirect@@YIXPAUWaveControlGroup@@PAX@Z
// No candidate emitted symbol is asserted before fresh object inspection.
// size      27 bytes
// prototype void (__thiscall ??0WaveControlGroup@@QAE@XZ)(WaveControlGroup* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a (the constructor ABI returns this in eax)
Status: Complete
*/
WaveControlGroup::WaveControlGroup() {
    this->head = nullptr;
    this->tail = nullptr;
    this->cursor = nullptr;
    this->count = 0;
    this->volume = 0x7f;
    this->enabled = 1;
}

/*
Purpose: Destroy one control group by draining its node list from the head:
         each node's follower loses its back link and becomes the head (the
         last one empties both ends), the node goes back to the game heap,
         and the count drops - but a node carrying a NULL wave stops the
         drain right after it is freed, leaving the rest of the list in
         place. The cursor is never touched.
// ORIGINAL: 0x004C5B80 ??1WaveControlGroup@@QAE@XZ 0x004C5B80-0x004C5BEF
// Historical project redirect symbol: ?wave_control_group_dtor_redirect@@YIXPAUWaveControlGroup@@PAX@Z
// No candidate emitted symbol is asserted before fresh object inspection.
// size      111 bytes
// prototype void (__thiscall ??1WaveControlGroup@@QAE@XZ)(WaveControlGroup* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0064557F
Return Value: n/a
Status: Complete
*/
WaveControlGroup::~WaveControlGroup() {
    WaveGroupNode *node = this->head;
    while (node) {
        WaveGroupNode *const next = node->next;
        if (next) {
            next->prev = nullptr;
            this->head = next;
        } else {
            // One statement: the tail clears first, the head second, the
            // original's store order.
            this->head = (this->tail = nullptr);
        }
        Wave *const wave = node->wave;
        operator delete(node);
        this->count -= 1;
        if (!wave) {
            break;
        }
        node = this->head;
    }
}

/*
Purpose: Build the device singleton in two vtable stages: the base's vtable
         (0x0066E098) up first while the scalar fields settle - full master
         volume at 0x08, everything else zeroed, no wrapped device - then
         the sixteen control groups construct through the CRT vector
         constructor iterator (their destructor rides along for the
         unreachable unwind path), the device's own vtable (0x0066E0E8)
         publishes, and the trailing dword clears. The SEH frame is omitted.
// ORIGINAL: 0x004C4DD0 ??0Wave_Device@@QAE@XZ 0x004C4DD0-0x004C4E5E;0x004C4DB0-0x004C4DC3;0x00659EA0-0x00659EB2
// size      179 bytes
// prototype void (__thiscall ??0Wave_Device@@QAE@XZ)(Wave_Device* this)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006457C2 0x006465F0
Return Value: n/a (the redirect answers the object pointer)
Status: Complete
*/
Wave_Device::Wave_Device() {
    field_20_ = 0;
}


/*
Purpose: Tear the device singleton down: its own vtable republishes, the
         destructor body is empty, then C++ destroys the sixteen typed
         WaveControlGroup members in reverse order before destroying the
         SoundDeviceBase. A wrapped device is wound down through its slot
         0x10 - kept in the field, since the object is dying anyway. The
         compiler owns array iteration and destructor-failure unwinding.
// ORIGINAL: 0x004C4E60 ??1Wave_Device@@QAE@XZ 0x004C4E60-0x004C4EC6;0x004C4DB0-0x004C4DC3;0x00659EB2-0x00659EC4
// size      139 bytes
// prototype void (__thiscall ??1Wave_Device@@QAE@XZ)(Wave_Device* this)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006456E4
// indirect  0x004C4EB5
Return Value: n/a
Status: Complete
*/
Wave_Device::~Wave_Device() {
}

// ====================
// The process wave device, 0x0090D978 in the image - REAL STORAGE carrying
// the public name wave.h's consumers read. It was `Wave_Device g_WAVE_DEVICE`
// beside the `Wave_Device *const WaveDeviceGlobal = (Wave_Device *)0x0090D978`
// binding in wave.h naming terranx.exe data that is unmapped in a standalone
// build; the object and the binding were the same address, so the object now
// carries the public name and every `WaveDeviceGlobal->` call site reaches it
// directly, the folded `mov ecx, imm32` receivers relocated.
// RECOVERY_OBJECT: v1 0x0090D978 ?WaveDeviceGlobal@@3VWave_Device@@A
Wave_Device WaveDeviceGlobal;  // 0x0090D978

// The release-hook guard, 0x0090DB7C in the image: init_sound zeroes it when
// the device fails to come up and sets it once the hook is live, and the
// wave teardown paths run the release slot only while it reads nonzero. Was
// `int *const WaveDeviceReleaseGuard = (int *)0x0090DB7C` (wave.h) and a
// `g_0090db7c` static in sound.cpp - the same terranx.exe dword twice.
int WaveDeviceReleaseGuard;  // 0x0090DB7C

// Factory/destroy storage belongs to SoundDllProcs in sound_loader.cpp:
// original 0x0090DB34/0x0090DB38 are its slots 4/5. The old separately
// allocated WaveDeviceFactoryHook/WaveDeviceDestroyHook variables described
// those same addresses but were never populated by the loader. Preserve the
// slot identities through the array's bounded source mapping and the reads
// above; there must not be a second pair of pointer objects here.

// Slot 2 of Wave_Device's image vftable: a bare `ret` stub at 0x00404280.
void Wave_Device::unk_slot2() {}
