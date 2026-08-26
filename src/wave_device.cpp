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
#include "wave.h"
#include "wave_device.h"
#include "dialog.h"
#include "sounddevice.h"
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

namespace {
// The wrapped device is driven through its own vtable; enable and disable call
// two adjacent slots on it. A raw dispatch is used because the device is an
// opaque object with no source-owned type.
//
// CALLED WHERE THE SLOT LIVES: the image dispatches through the vtable slot
// directly (`call`/`jmp dword ptr [vtable+N]`), one instruction; reading the
// slot into a pointer-to-member first costs a `mov` before it. `__fastcall`
// with the RECEIVER AS THE ONLY PARAMETER puts it in ecx and leaves edx
// alone, matching the thiscall the image dispatches through.
typedef void(__fastcall *device_vfn)(void *);
typedef int(__fastcall *device_query_vfn)(void *);

__forceinline void *wrapped_device(Wave_Device *self) {
    return *reinterpret_cast<void **>(reinterpret_cast<uint8_t *>(self) + 0x14);
}

// `__forceinline`: the image inlines this dispatch at every call site rather
// than making a real call to it; a plain out-of-line definition compiles a
// call the image does not make.
__forceinline void dispatch_wrapped_device(Wave_Device *self, int vtable_offset) {
    void *device = wrapped_device(self);
    if (device) {
        vtable_slot<device_vfn>(device, vtable_offset)(device);
    }
}

// The querying half of the same family: the original tail-jumps into the
// device's method, so that method's result is the caller's; with no device
// wrapped the answer is whatever the original loads into eax on that path,
// which is usually but not always zero.
__forceinline int query_wrapped_device(Wave_Device *self, int vtable_offset,
                         int no_device_result = 0) {
    void *device = wrapped_device(self);
    // The zero/default path runs LAST in the image: `test ecx, ecx; je
    // <default>` falls through to the dispatch, rather than a guard clause
    // that jumps away from it.
    if (device) {
        return vtable_slot<device_query_vfn>(device, vtable_offset)(device);
    }
    return no_device_result;
}

// The one-argument members of the family. The original passes the argument
// straight through and, where it returns at all, answers a fixed value when no
// device is wrapped.
//
// The argument is passed ON THE STACK by the device's own calling convention,
// not in edx as a second `__fastcall` parameter would be; the pointer-to-
// member type keeps that thiscall shape, and `vtable_slot` still returns a
// REFERENCE to the slot so the call stays the single `call dword ptr
// [vtable+N]` the image makes.
typedef int (OriginalObject::*device_arg_vfn)(int a1);

__forceinline int forward_to_wrapped_device(Wave_Device *self, int vtable_offset, int a1,
                              int no_device_result = 0) {
    void *device = wrapped_device(self);
    if (!device) {
        return no_device_result;
    }
    return (ORIGINAL(device)->*vtable_slot<device_arg_vfn>(device, vtable_offset))(a1);
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x60.
// ORIGINAL: 0x004C51C0 ?enable@Wave_Device@@QAEXXZ 0x004C51C0-0x004C51CF
// TRIED: byte-exactness - plateaus at 4/7 agreeing (0.769 similar)
// across every flag set tried. The image keeps a real `call` on the
// device-present path and falls through into a SHARED `xor eax,eax; ret`
// epilogue the null-device path also jumps to; this tree's void wrapper
// always folds the trailing call into a tail `jmp` since nothing follows
// it, matching `disable()`'s own sibling shape rather than the image's.
// Three spellings measured, all 4/7: `dispatch_wrapped_device` as-is,
// discarding `query_wrapped_device(this, 0x60)`'s int return, and a
// guard-clause early-return before the dispatch. Contrast `get_ds()`
// (int-returning, correctly tail-jumps on the call path) - see it there.
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
void Wave_Device::enable() {
    dispatch_wrapped_device(this, 0x60);
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x64.
// ORIGINAL: 0x004C51D0 ?disable@Wave_Device@@QAEXXZ 0x004C51D0-0x004C51DF
// TRIED: byte-exactness, same plateau and same cause as `enable()` -
// see the note there (4/7 agreeing, 0.769 similar, the image's real `call`
// against this tree's tail `jmp`).
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
void Wave_Device::disable() {
    dispatch_wrapped_device(this, 0x64);
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
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        device_vfn fn = *reinterpret_cast<device_vfn *>(vtable + 0x48);
        fn(device);
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
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        device_vfn fn = *reinterpret_cast<device_vfn *>(vtable + 0x4C);
        fn(device);
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
    dispatch_wrapped_device(this, 0x34);
}

/*
Purpose: Ask the wrapped device how many devices it offers, through its vtable
         slot 0x14.
// ORIGINAL: 0x004C4FF0 ?get_ndevices@Wave_Device@@QAEHXZ 0x004C4FF0-0x004C4FFF BYTE_EXACT
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
    return query_wrapped_device(this, 0x14);
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
    return query_wrapped_device(this, 0x58);
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
    return query_wrapped_device(this, 0x3C);
}

/*
Purpose: Ask the wrapped device for its DirectSound interface, through its
         vtable slot 0x70.
// ORIGINAL: 0x004C5190 ?get_ds@Wave_Device@@QAEHXZ 0x004C5190-0x004C519F BYTE_EXACT
// size      15 bytes
// prototype int (__thiscall ?get_ds@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_ds() {
    return query_wrapped_device(this, 0x70);
}

/*
Purpose: Ask the wrapped device whether it supports EAX, through its vtable
         slot 0x74. With no device wrapped the original clears only al, which
         is the codegen for a false of byte width; zero is that value.
// ORIGINAL: 0x004C54B0 ?is_eax@Wave_Device@@QAEHXZ 0x004C54B0-0x004C54BF SEMANTIC
// size      15 bytes
// prototype int (__thiscall ?is_eax@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::is_eax() {
    return query_wrapped_device(this, 0x74);
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
    return query_wrapped_device(this, 0x68, 1);
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
    return query_wrapped_device(this, 0x54, 3);
}

/*
Purpose: Ask the wrapped device whether it does 3D audio, through vtable slot
         0x84. With no device wrapped the original clears only al, the codegen
         for a false of byte width; zero is that value.
// ORIGINAL: 0x004C5530 ?is_3d@Wave_Device@@QAEHXZ 0x004C5530-0x004C5542 SEMANTIC
// size      18 bytes
// prototype int (__thiscall ?is_3d@Wave_Device@@QAEHXZ)(Wave_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::is_3d() {
    return query_wrapped_device(this, 0x84);
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
    forward_to_wrapped_device(this, 0x38, static_cast<int>(rate));
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
    forward_to_wrapped_device(this, 0x20, static_cast<int>(volume));
}

/*
Purpose: Hand the game window to the wrapped device, through vtable slot 0x6C.
// ORIGINAL: 0x004C5000 ?set_hwnd@Wave_Device@@QAEHPAX@Z 0x004C5000-0x004C5020
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
    return forward_to_wrapped_device(this, 0x6C,
                                     static_cast<int>(
                                         reinterpret_cast<intptr_t>(hwnd)),
                                     0x13);
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
    std::memcpy(&value,
                reinterpret_cast<uint8_t *>(this) + 0x28 + group * 24,
                sizeof(value));
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
    // The slot field is private to Wave and written here by offset, exactly
    // as the original stores through [wave+0x68].
    *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(wave) + 0x68) =
        group;
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
    uint32_t *const slot_field =
        reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(a1) + 0x68);
    if (!a1 || *slot_field >= 0x10) {
        return 0xA;
    }
    const uint32_t slot = *slot_field;
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
    *slot_field = 0x10;
    return 0;
}


/*
Purpose: Report whether a group is disabled: out-of-range groups always are,
         and a real one is disabled while its enabled byte is zero. The
         original defines only AL on the out-of-range path; callers test the
         byte.
// ORIGINAL: 0x004C5460 ?is_group_disabled@Wave_Device@@QAEHI@Z 0x004C5460-0x004C5481
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
// size      33 bytes
// prototype int (__thiscall ?is_group_disabled@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when disabled, 0 when enabled
Status: Complete
*/
int Wave_Device::is_group_disabled(unsigned int group) {
    if (group > 0xF) {
        return 1;
    }
    return groups_[group].enabled == 0 ? 1 : 0;
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
// LEVER: `WaveGroupNode *const node` -> non-const with an explicit
// `else { node = 0; }`. With the pointer const-initialized from the `if
// (node) {...}` alone, this tree let the null path fall straight into the
// tail-check with `eax` already zero from the allocator's own return,
// dropping the image's own `jmp`/`xor eax,eax` merge pair (0x004C5C17-
// 0x004C5C19) - 34/36 instructions, 0.971 similar. Materializing the
// redundant zero explicitly reproduces the image's (needless) re-zero and
// closes the last two instructions.
// symbol    ?wave_group_insert_redirect@@YIXPAUWaveGroupList@@PAXPAVWave@@@Z
// size      90 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064558A
Return Value: n/a
Status: Complete
*/
void __fastcall wave_group_insert_redirect(WaveGroupList *self, void *,
                                           Wave *a1) {
    WaveGroupNode *node =
        static_cast<WaveGroupNode *>(WaveOperatorNew(0xC));
    if (node) {
        node->prev = nullptr;
        node->next = nullptr;
        node->wave = a1;
    } else {
        node = 0;
    }
    WaveGroupNode *const tail = self->tail;
    if (tail) {
        // The old tail's next is written first and the tail field re-read
        // second, exactly as the original orders its memory operations; the
        // volatile views keep the pair honest when the two locations alias
        // (the optimizer's type-based analysis would otherwise fold the
        // re-read).
        *reinterpret_cast<WaveGroupNode *volatile *>(&tail->next) = node;
        node->prev = *reinterpret_cast<WaveGroupNode *volatile *>(&self->tail);
        self->tail = node;
        self->count += 1;
    } else {
        self->head = node;
        self->tail = node;
        self->count += 1;
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
// symbol    ?get_description@Wave_Device@@QAEHKPADK@Z
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
        typedef void (OriginalObject::*device_fn)(unsigned long a1, char *a2, unsigned long a3);
        (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x1c))(a1, a2, a3);
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
        // A pointer argument beyond the receiver is passed ON THE STACK by
        // the device's own calling convention, not in edx as a second
        // `__fastcall` parameter would; the pointer-to-member type keeps
        // that thiscall shape, and `vtable_slot` still returns a REFERENCE
        // to the slot so the call is the single `call dword ptr [vtable+N]`.
        typedef int (OriginalObject::*device_fn)(char *path);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x50))(path);
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
        typedef int (OriginalObject::*device_fn)(EAX_REVERB_PROPERTIES *properties);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x7c))(properties);
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
        typedef int (OriginalObject::*device_fn)(unsigned long properties);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x78))(properties);
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
        typedef int(__fastcall *device_fn)(void *, float);
        return vtable_slot<device_fn>(device_14_, 0x80)(device_14_, mix);
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
        typedef int(__fastcall *device_fn)(void *, float, float, float);
        return vtable_slot<device_fn>(device_14_, 0x88)(device_14_, x, y, z);
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
        typedef int (OriginalObject::*device_fn)(float *x, float *y, float *z);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x8c))(x, y, z);
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
        typedef int(__fastcall *device_fn)(void *, float);
        return vtable_slot<device_fn>(device_14_, 0x90)(device_14_, x);
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
        typedef int (OriginalObject::*device_fn)(float *x);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x94))(x);
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
        typedef int(__fastcall *device_fn)(void *, float);
        return vtable_slot<device_fn>(device_14_, 0x98)(device_14_, y);
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
        typedef int (OriginalObject::*device_fn)(float *y);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x9c))(y);
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
        typedef int(__fastcall *device_fn)(void *, float);
        return vtable_slot<device_fn>(device_14_, 0xa0)(device_14_, z);
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
        typedef int (OriginalObject::*device_fn)(float *z);
        return (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0xa4))(z);
    }
    return 0x14;
}


namespace {

// The removal-safe walk the three group administrators share. The cursor
// field is re-read after every callback: a wave's handler may pull waves
// from the group, and pull_from_group maintains the cursor, so the walk
// survives removal - and a self-removal skips the follower, exactly the
// original's advance rule. A node with a null wave stops the walk.
void walk_group_waves(WaveControlGroup &group, void (*visit)(Wave *wave)) {
    WaveGroupNode *const first = group.head;
    group.cursor = first;
    if (!first) {
        return;
    }
    Wave *wave = first->wave;
    if (!wave) {
        return;
    }
    for (;;) {
        visit(wave);
        WaveGroupNode *const cursor = group.cursor;
        if (!cursor) {
            return;
        }
        WaveGroupNode *const next = cursor->next;
        group.cursor = next;
        if (!next) {
            return;
        }
        wave = next->wave;
        if (!wave) {
            return;
        }
    }
}

// Per-wave handlers: replay the wave's own stored volume through its vtable
// slot 0x40, resume through slot 0x8C, halt through slot 0x14. The volume
// field is private to Wave and read by offset, as the original does.
void replay_wave_volume(Wave *wave) {
    typedef void (OriginalObject::*wave_volume_fn)(uint32_t volume);
    uint32_t vol;
    std::memcpy(&vol, reinterpret_cast<uint8_t *>(wave) + 4, 4);
    (ORIGINAL(wave)->*vtable_slot<wave_volume_fn>(wave, 0x40))(vol);
}

void resume_wave(Wave *wave) {
    typedef void(__fastcall *wave_vfn)(void *);
    vtable_slot<wave_vfn>(wave, 0x8C)(wave);
}

__forceinline void halt_wave(Wave *wave) {
    typedef void(__fastcall *wave_vfn)(void *);
    vtable_slot<wave_vfn>(wave, 0x14)(wave);
}

// The chain-link view select repurposes: once a sound is halted off the
// global chain, its link fields at 0x44/0x48 thread the private resume
// list instead, and 0x4C remembers the filename to reload by. All three
// fields are private to the sound classes and reached by offset, as the
// original does.
struct WaveResumeLinks {
    Wave *prev;   // +0x44
    Wave *next;   // +0x48
    char *fname;  // +0x4C
};

__forceinline WaveResumeLinks *resume_links(Wave *wave) {
    return reinterpret_cast<WaveResumeLinks *>(
        reinterpret_cast<uint8_t *>(wave) + 0x44);
}

// The sound-side virtuals select dispatches through: the attribute word
// (slot 0x70), the chain-next and chain-prev accessors (slots 0x64/0x68),
// and load-by-name (slot 0x10).
__forceinline int wave_attrib(Wave *wave) {
    typedef int(__fastcall *wave_query_fn)(void *);
    return vtable_slot<wave_query_fn>(wave, 0x70)(wave);
}

__forceinline Wave *wave_chain_next(Wave *wave) {
    typedef Wave *(__fastcall *wave_chain_fn)(void *);
    return vtable_slot<wave_chain_fn>(wave, 0x64)(wave);
}

__forceinline Wave *wave_chain_prev(Wave *wave) {
    typedef Wave *(__fastcall *wave_chain_fn)(void *);
    return vtable_slot<wave_chain_fn>(wave, 0x68)(wave);
}

__forceinline void load_wave_by_name(Wave *wave, char *fname) {
    typedef int (OriginalObject::*wave_load_fn)(char *fname);
    (ORIGINAL(wave)->*vtable_slot<wave_load_fn>(wave, 0x10))(fname);
}

}  // namespace

/*
Purpose: Set a group's volume scale and replay every member wave's own
         stored volume through its vtable slot 0x40, so the new scale takes
         effect. Bad slots and volumes above 0x7F answer 0xA.
// ORIGINAL: 0x004C5320 ?set_group_volume@Wave_Device@@QAEHII@Z 0x004C5320-0x004C537F
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
    WaveControlGroup &group = groups_[a1];
    group.volume = a2;
    walk_group_waves(group, &replay_wave_volume);
    return 0;
}


/*
Purpose: Enable a group. Only a disabled one does any work: every member
         wave resumes through its vtable slot 0x8C, and the enabled byte is
         written AFTER the walk. An already-enabled group is left exactly as
         it is, nonzero byte and all.
// ORIGINAL: 0x004C53A0 ?enable_group@Wave_Device@@QAEHI@Z 0x004C53A0-0x004C5400
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
    WaveControlGroup &group = groups_[a1];
    if (!group.enabled) {
        walk_group_waves(group, &resume_wave);
        group.enabled = 1;
    }
    return 0;
}


/*
Purpose: Disable a group. Only an enabled one does any work: every member
         wave halts through its vtable slot 0x14, and the enabled byte is
         cleared AFTER the walk.
// ORIGINAL: 0x004C5400 ?disable_group@Wave_Device@@QAEHI@Z 0x004C5400-0x004C545D
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
    WaveControlGroup &group = groups_[a1];
    if (group.enabled) {
        walk_group_waves(group, &halt_wave);
        group.enabled = 0;
    }
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
// ORIGINAL: 0x004C5030 ?select@Wave_Device@@QAEHK@Z 0x004C5030-0x004C50EF
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
    Wave *resume_tail = nullptr;
    if (!device_14_) {
        return 2;
    }
    Wave *sound = WaveChainHead();
    while (sound) {
        if ((wave_attrib(sound) & 1) && !(wave_attrib(sound) & 4)) {
            Wave *const next = wave_chain_next(sound);
            halt_wave(sound);
            if (!resume_tail) {
                resume_links(sound)->prev = nullptr;
                resume_links(sound)->next = nullptr;
            } else {
                resume_links(resume_tail)->next = sound;
                resume_links(sound)->prev = resume_tail;
            }
            resume_tail = sound;
            sound = next;
        } else {
            sound = wave_chain_next(sound);
        }
    }
    {
        typedef int (OriginalObject::*device_fn)(unsigned long a1);
        (ORIGINAL(device_14_)->*vtable_slot<device_fn>(device_14_, 0x18))(a1);
    }
    while (resume_tail) {
        Wave *const replay = resume_tail;
        resume_tail = wave_chain_prev(resume_tail);
        char *const fname = resume_links(replay)->fname;
        resume_links(replay)->next = nullptr;
        resume_links(replay)->prev = nullptr;
        load_wave_by_name(replay, fname);
    }
    return 0;
}



/*
Purpose: Build the wrapped device of the requested kind through the factory
         hook, straight into the 0x14 field. A device already wrapped is
         refused with 0xC; a dead factory answers 0x14.
// ORIGINAL: 0x004C4ED0 ?create_device@Wave_Device@@QAEHK@Z 0x004C4ED0-0x004C4F07
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
    if (device_14_) {
        return 0xC;
    }
    if (!WaveDeviceFactorySlot()) {
        return 0x14;
    }
    return (WaveDeviceFactorySlot())(&device_14_, a1);
}


/*
Purpose: Tear the wrapped device down through the argument-less destroy hook
         and forget it. Without a device or a hook, 0x14 and nothing happens.
// ORIGINAL: 0x004C4F10 ?delete_device@Wave_Device@@QAEHXZ 0x004C4F10-0x004C4F37
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
    if (!device_14_) {
        return 0x14;
    }
    if (!WaveDeviceDestroySlot()) {
        return 0x14;
    }
    (WaveDeviceDestroySlot())();
    device_14_ = nullptr;
    return 0;
}


/*
Purpose: Initialise the device stack: the Wave_Device's OWN virtual slot 0
         takes the mode first (the class does have a vtable, held opaque at
         offset zero), then the wrapped device initialises through its slot
         0xC with both arguments, and a failure there runs the Wave_Device's
         own virtual slot 4 before the error propagates.
// ORIGINAL: 0x004C4F40 ?init@Wave_Device@@QAEHPAXK@Z 0x004C4F40-0x004C4F7D BYTE_EXACT
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
    typedef int (OriginalObject::*own_mode_fn)(unsigned long mode);
    const int staged = (ORIGINAL(this)->*(*reinterpret_cast<own_mode_fn **>(this))[0])(flags);
    if (staged) {
        return staged;
    }
    typedef int (OriginalObject::*device_init_fn)(void *group_id, unsigned long flags);
    void *device = device_14_;
    const int result = (ORIGINAL(device)->*(*reinterpret_cast<device_init_fn **>(device))[3])(group_id, flags);
    if (result) {
        typedef void (OriginalObject::*own_fail_fn)();
        (ORIGINAL(this)->*(*reinterpret_cast<own_fail_fn **>(this))[1])();
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
// symbol    ?release@Wave_Device@@QAEHXZ
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
        {
            typedef void(__fastcall *device_down_fn)(void *);
            vtable_slot<device_down_fn>(device_14_, 0x10)(device_14_);
        }
        if (device_14_) {
            func_wave_device_destroy *const hook = WaveDeviceDestroySlot();
            if (hook) {
                hook();
                device_14_ = nullptr;
            }
        }
        device_14_ = nullptr;
    }
    return 0;
}


const void *const WaveControlGroupOriginalCtor = (const void *)0x004C5490;
const void *const WaveControlGroupOriginalDtor = (const void *)0x004C5B80;

/*
Purpose: Construct one control group: the list fields - head, tail, cursor,
         count - zero out; the enabled byte and volume are left untouched.
// ORIGINAL: 0x004C5490 ??0WaveControlGroup@@QAE@XZ 0x004C5490-0x004C54AB
// symbol    ?wave_control_group_ctor_redirect@@YIXPAUWaveControlGroup@@PAX@Z
// size      27 bytes
// prototype void (__thiscall ??0WaveControlGroup@@QAE@XZ)(WaveControlGroup* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a (the redirect leaves the object pointer where the original
              does)
Status: Complete
*/
void __fastcall wave_control_group_ctor_redirect(WaveControlGroup *self,
                                                 void *) {
    self->head = nullptr;
    self->tail = nullptr;
    self->cursor = nullptr;
    self->count = 0;
}

/*
Purpose: Destroy one control group by draining its node list from the head:
         each node's follower loses its back link and becomes the head (the
         last one empties both ends), the node goes back to the game heap,
         and the count drops - but a node carrying a NULL wave stops the
         drain right after it is freed, leaving the rest of the list in
         place. The cursor is never touched.
// ORIGINAL: 0x004C5B80 ??1WaveControlGroup@@QAE@XZ 0x004C5B80-0x004C5BEF
// symbol    ?wave_control_group_dtor_redirect@@YIXPAUWaveControlGroup@@PAX@Z
// size      111 bytes
// prototype void (__thiscall ??1WaveControlGroup@@QAE@XZ)(WaveControlGroup* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0064557F
Return Value: n/a
Status: Complete
*/
void __fastcall wave_control_group_dtor_redirect(WaveControlGroup *self,
                                                 void *) {
    WaveGroupNode *node = self->head;
    while (node) {
        WaveGroupNode *const next = node->next;
        if (next) {
            next->prev = nullptr;
            self->head = next;
        } else {
            // One statement: the tail clears first, the head second, the
            // original's store order.
            self->head = (self->tail = nullptr);
        }
        Wave *const wave = node->wave;
        operator delete(node);
        self->count -= 1;
        if (!wave) {
            break;
        }
        node = self->head;
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
    vtable_storage_ = 0x0066E098;
    field_4_ = 0;
    volume_8_ = 0x7F;
    field_C_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_10_ = 0;
    device_14_ = nullptr;
    (*VectorCtorIterator)(groups_, 0x18, 0x10, WaveControlGroupOriginalCtor,
                          WaveControlGroupOriginalDtor);
    vtable_storage_ = 0x0066E0E8;
    field_20_ = 0;
}


/*
Purpose: Tear the device singleton down: its own vtable republishes, the
         sixteen control groups drain through the CRT vector destructor
         iterator, the base vtable takes over, and a wrapped device is wound
         down through its slot 0x10 - kept in the field, since the object is
         dying anyway. The SEH frame is omitted.
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
    Wave_Device volatile *const self = this;
    self->vtable_storage_ = 0x0066E0E8;
    VectorDtorIterator(const_cast<WaveControlGroup *>(self->groups_), 0x18,
                       0x10, WaveControlGroupOriginalDtor);
    self->vtable_storage_ = 0x0066E098;
    {
        void *const device = self->device_14_;
        if (device) {
            typedef void(__fastcall *device_down_fn)(void *);
            vtable_slot<device_down_fn>(device, 0x10)(device);
        }
    }
}

