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
typedef void (OriginalObject::*device_vfn)();
typedef int (OriginalObject::*device_query_vfn)();

void *wrapped_device(Wave_Device *self) {
    return *reinterpret_cast<void **>(reinterpret_cast<uint8_t *>(self) + 0x14);
}

void dispatch_wrapped_device(Wave_Device *self, int vtable_offset) {
    void *device = wrapped_device(self);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        device_vfn fn = *reinterpret_cast<device_vfn *>(vtable + vtable_offset);
        (ORIGINAL(device)->*fn)();
    }
}

// The querying half of the same family: the original tail-jumps into the
// device's method, so that method's result is the caller's; with no device
// wrapped the answer is whatever the original loads into eax on that path,
// which is usually but not always zero.
int query_wrapped_device(Wave_Device *self, int vtable_offset,
                         int no_device_result = 0) {
    void *device = wrapped_device(self);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    device_query_vfn fn =
        *reinterpret_cast<device_query_vfn *>(vtable + vtable_offset);
    return (ORIGINAL(device)->*fn)();
}

// The one-argument members of the family. The original passes the argument
// straight through and, where it returns at all, answers a fixed value when no
// device is wrapped.
typedef int (OriginalObject::*device_arg_vfn)(int a1);

int forward_to_wrapped_device(Wave_Device *self, int vtable_offset, int a1,
                              int no_device_result = 0) {
    void *device = wrapped_device(self);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (ORIGINAL(device)->*original_slot<device_arg_vfn>(vtable + vtable_offset))(a1);
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x60.
// ORIGINAL: 0x004C51C0 ?enable@Wave_Device@@QAEXXZ 0x004C51C0-0x004C51CF
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

void __fastcall wave_device_set_pan_redirect(Wave_Device *self, void *, int a1) {
    self->set_pan(a1);
}

int __fastcall wave_device_fade_redirect(Wave_Device *self, void *, int a1) {
    return self->fade(a1);
}

void __fastcall wave_device_enable_redirect(Wave_Device *self, void *) {
    self->enable();
}

void __fastcall wave_device_disable_redirect(Wave_Device *self, void *) {
    self->disable();
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

int __fastcall wave_device_get_volume_redirect(Wave_Device *self, void *) {
    return self->get_volume();
}

int __fastcall wave_device_stop_redirect(Wave_Device *self, void *) {
    return self->stop();
}

/*
Purpose: Ask the wrapped device to suspend, through its vtable slot 0x48.
// ORIGINAL: 0x004C4FC0 ?suspend@Wave_Device@@QAEXXZ 0x004C4FC0-0x004C4FCD
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
    dispatch_wrapped_device(this, 0x48);
}

/*
Purpose: Ask the wrapped device to restart, through its vtable slot 0x4C.
// ORIGINAL: 0x004C4FD0 ?restart@Wave_Device@@QAEXXZ 0x004C4FD0-0x004C4FDD
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
    dispatch_wrapped_device(this, 0x4C);
}

/*
Purpose: Poll the wrapped device, through its vtable slot 0x34.
// ORIGINAL: 0x004C4FE0 ?update_sound@Wave_Device@@QAEXXZ 0x004C4FE0-0x004C4FED
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
// ORIGINAL: 0x004C4FF0 ?get_ndevices@Wave_Device@@QAEHXZ 0x004C4FF0-0x004C4FFF
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
// ORIGINAL: 0x004C5020 ?get_hw_mem_size@Wave_Device@@QAEHXZ 0x004C5020-0x004C502F
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
// ORIGINAL: 0x004C5140 ?get_rate@Wave_Device@@QAEHXZ 0x004C5140-0x004C514F
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
// ORIGINAL: 0x004C5190 ?get_ds@Wave_Device@@QAEHXZ 0x004C5190-0x004C519F
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
// ORIGINAL: 0x004C54B0 ?is_eax@Wave_Device@@QAEHXZ 0x004C54B0-0x004C54BF
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

void __fastcall wave_device_suspend_redirect(Wave_Device *self, void *) {
    self->suspend();
}

void __fastcall wave_device_restart_redirect(Wave_Device *self, void *) {
    self->restart();
}

void __fastcall wave_device_update_sound_redirect(Wave_Device *self, void *) {
    self->update_sound();
}

int __fastcall wave_device_get_ndevices_redirect(Wave_Device *self, void *) {
    return self->get_ndevices();
}

int __fastcall wave_device_get_hw_mem_size_redirect(Wave_Device *self, void *) {
    return self->get_hw_mem_size();
}

int __fastcall wave_device_get_rate_redirect(Wave_Device *self, void *) {
    return self->get_rate();
}

int __fastcall wave_device_get_ds_redirect(Wave_Device *self, void *) {
    return self->get_ds();
}

int __fastcall wave_device_is_eax_redirect(Wave_Device *self, void *) {
    return self->is_eax();
}

/*
Purpose: Ask the wrapped device whether it is disabled, through vtable slot
         0x68. With no device wrapped the answer is yes.
// ORIGINAL: 0x004C51E0 ?is_disabled@Wave_Device@@QAEHXZ 0x004C51E0-0x004C51F2
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
// ORIGINAL: 0x004C5220 ?stop_raw_dump@Wave_Device@@QAEHXZ 0x004C5220-0x004C5232
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
// ORIGINAL: 0x004C5530 ?is_3d@Wave_Device@@QAEHXZ 0x004C5530-0x004C5542
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
// ORIGINAL: 0x004C5120 ?set_rate@Wave_Device@@QAEXK@Z 0x004C5120-0x004C5137
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
void Wave_Device::set_rate(unsigned long a1) {
    forward_to_wrapped_device(this, 0x38, static_cast<int>(a1));
}

/*
Purpose: Hand the volume to the wrapped device, through vtable slot 0x20.
// ORIGINAL: 0x004C5150 ?set_volume@Wave_Device@@QAEXK@Z 0x004C5150-0x004C5167
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
void Wave_Device::set_volume(unsigned long a1) {
    forward_to_wrapped_device(this, 0x20, static_cast<int>(a1));
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
int Wave_Device::set_hwnd(void *a1) {
    return forward_to_wrapped_device(this, 0x6C,
                                     static_cast<int>(
                                         reinterpret_cast<intptr_t>(a1)),
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
int Wave_Device::get_group_volume(unsigned int a1) {
    if (a1 > 0xF) {
        return 0;
    }
    int value;
    std::memcpy(&value,
                reinterpret_cast<uint8_t *>(this) + 0x28 + a1 * 24,
                sizeof(value));
    return value;
}

int __fastcall wave_device_is_disabled_redirect(Wave_Device *self, void *) {
    return self->is_disabled();
}

int __fastcall wave_device_stop_raw_dump_redirect(Wave_Device *self, void *) {
    return self->stop_raw_dump();
}

int __fastcall wave_device_is_3d_redirect(Wave_Device *self, void *) {
    return self->is_3d();
}

void __fastcall wave_device_set_rate_redirect(Wave_Device *self, void *,
                                              unsigned long a1) {
    self->set_rate(a1);
}

void __fastcall wave_device_set_volume_redirect(Wave_Device *self, void *,
                                                unsigned long a1) {
    self->set_volume(a1);
}

int __fastcall wave_device_set_hwnd_redirect(Wave_Device *self, void *,
                                             void *a1) {
    return self->set_hwnd(a1);
}

int __fastcall wave_device_get_group_volume_redirect(Wave_Device *self, void *,
                                                     unsigned int a1) {
    return self->get_group_volume(a1);
}

func_wave_group_insert WaveDeviceGroupInsert = original_method<func_wave_group_insert>(0x004C5BF0);

/*
Purpose: Put a wave into one of the sixteen groups. The list-insert helper
         threads it through the group's node list, and the wave's own group
         slot at 0x68 records the group AFTER the insert - the original
         writes the field last, so the helper still sees the old slot.
// ORIGINAL: 0x004C5240 ?add_to_group@Wave_Device@@QAEHIPAUWave@@@Z 0x004C5240-0x004C5277
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
int Wave_Device::add_to_group(unsigned int a1, Wave *a2) {
    if (a1 > 0xF || !a2) {
        return 0xA;
    }
    (ORIGINAL(&groups_[a1].head)->*WaveDeviceGroupInsert)(a2);
    // The slot field is private to Wave and written here by offset, exactly
    // as the original stores through [wave+0x68].
    *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(a2) + 0x68) =
        a1;
    return 0;
}

int __fastcall wave_device_add_to_group_redirect(Wave_Device *self, void *,
                                                 unsigned int a1, Wave *a2) {
    return self->add_to_group(a1, a2);
}

/*
Purpose: Take a wave out of its group. The wave's slot names the group; its
         node is searched for in the group's list and, when found, unlinked
         (head and tail maintained at the ends, the cursor left on the node
         after the removal or nulled), freed to the game heap, and counted
         out. A wave whose node is not on the list - or a group with no list
         at all - just forgets its slot. Either way the wave's slot becomes
         the out-of-range 0x10.
// ORIGINAL: 0x004C5280 ?pull_from_group@Wave_Device@@QAEHPAUWave@@@Z 0x004C5280-0x004C531C
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
    if (!a1) {
        return 0xA;
    }
    uint32_t *const slot_field =
        reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(a1) + 0x68);
    const uint32_t slot = *slot_field;
    if (slot >= 0x10) {
        return 0xA;
    }
    WaveControlGroup &group = groups_[slot];
    WaveGroupNode *node = group.head;
    if (node) {
        while (node->wave != a1) {
            node = node->next;
            if (!node) {
                *slot_field = 0x10;
                return 0;
            }
        }
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            group.head = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
            group.cursor = node->next;
        } else {
            // One statement, comma-sequenced: the cursor clears first and
            // the tail steps back second, exactly the original's store order.
            group.tail = (group.cursor = nullptr, node->prev);
        }
        operator delete(node);
        group.count -= 1;
    }
    *slot_field = 0x10;
    return 0;
}

int __fastcall wave_device_pull_from_group_redirect(Wave_Device *self, void *,
                                                    Wave *a1) {
    return self->pull_from_group(a1);
}

/*
Purpose: Report whether a group is disabled: out-of-range groups always are,
         and a real one is disabled while its enabled byte is zero. The
         original defines only AL on the out-of-range path; callers test the
         byte.
// ORIGINAL: 0x004C5460 ?is_group_disabled@Wave_Device@@QAEHI@Z 0x004C5460-0x004C5481
// size      33 bytes
// prototype int (__thiscall ?is_group_disabled@Wave_Device@@QAEHI@Z)(Wave_Device* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when disabled, 0 when enabled
Status: Complete
*/
int Wave_Device::is_group_disabled(unsigned int a1) {
    if (a1 > 0xF) {
        return 1;
    }
    return groups_[a1].enabled == 0 ? 1 : 0;
}

int __fastcall wave_device_is_group_disabled_redirect(Wave_Device *self,
                                                      void *,
                                                      unsigned int a1) {
    return self->is_group_disabled(a1);
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
// ORIGINAL: 0x004C5BF0 sub_4c5bf0 0x004C5BF0-0x004C5C4A
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
    WaveGroupNode *const node =
        static_cast<WaveGroupNode *>(WaveOperatorNew(0xC));
    if (node) {
        node->prev = nullptr;
        node->next = nullptr;
        node->wave = a1;
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
// ORIGINAL: 0x004C50F0 ?get_description@Wave_Device@@QAEXKPADK@Z 0x004C50F0-0x004C5111
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
void Wave_Device::get_description(unsigned long a1, char *a2, unsigned long a3) {
    if (device_14_) {
        typedef void (OriginalObject::*device_fn)(unsigned long a1, char *a2, unsigned long a3);
        (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x1c))(
            a1, a2, a3);
    }
}

void __fastcall wave_device_get_description_redirect(Wave_Device *self, void *, unsigned long a1, char *a2, unsigned long a3) {
    self->get_description(a1, a2, a3);
}

/*
Purpose: Forward start_raw_dump to the wrapped device through its vtable slot
         0x50.
// ORIGINAL: 0x004C5200 ?start_raw_dump@Wave_Device@@QAEHPAD@Z 0x004C5200-0x004C5220
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
int Wave_Device::start_raw_dump(char *a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(char *a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x50))(
            a1);
    }
    return 3;
}

int __fastcall wave_device_start_raw_dump_redirect(Wave_Device *self, void *, char *a1) {
    return self->start_raw_dump(a1);
}

/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x7c.
// ORIGINAL: 0x004C54C0 ?set_eax@Wave_Device@@QAEHPAUEAX_REVERB_PROPERTIES@@@Z 0x004C54C0-0x004C54E0
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
int Wave_Device::set_eax(EAX_REVERB_PROPERTIES *a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(EAX_REVERB_PROPERTIES *a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x7c))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_props_redirect(Wave_Device *self, void *, EAX_REVERB_PROPERTIES *a1) {
    return self->set_eax(a1);
}

/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x78.
// ORIGINAL: 0x004C54E0 ?set_eax@Wave_Device@@QAEHK@Z 0x004C54E0-0x004C5500
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
int Wave_Device::set_eax(unsigned long a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(unsigned long a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x78))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_redirect(Wave_Device *self, void *, unsigned long a1) {
    return self->set_eax(a1);
}

/*
Purpose: Forward set_eax_mix to the wrapped device through its vtable slot
         0x80.
// ORIGINAL: 0x004C5500 ?set_eax_mix@Wave_Device@@QAEHM@Z 0x004C5500-0x004C5523
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
int Wave_Device::set_eax_mix(float a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x80))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_mix_redirect(Wave_Device *self, void *, float a1) {
    return self->set_eax_mix(a1);
}

/*
Purpose: Forward set_listener_position to the wrapped device through its vtable slot
         0x88.
// ORIGINAL: 0x004C5550 ?set_listener_position@Wave_Device@@QAEHMMM@Z 0x004C5550-0x004C557B
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
int Wave_Device::set_listener_position(float a1, float a2, float a3) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float a1, float a2, float a3);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x88))(
            a1, a2, a3);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_position_redirect(Wave_Device *self, void *, float a1, float a2, float a3) {
    return self->set_listener_position(a1, a2, a3);
}

/*
Purpose: Forward get_listener_position to the wrapped device through its vtable slot
         0x8c.
// ORIGINAL: 0x004C5580 ?get_listener_position@Wave_Device@@QAEHPAM00@Z 0x004C5580-0x004C55AB
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
int Wave_Device::get_listener_position(float *a1, float *a2, float *a3) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float *a1, float *a2, float *a3);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x8c))(
            a1, a2, a3);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_position_redirect(Wave_Device *self, void *, float *a1, float *a2, float *a3) {
    return self->get_listener_position(a1, a2, a3);
}

/*
Purpose: Forward set_listener_xpos to the wrapped device through its vtable slot
         0x90.
// ORIGINAL: 0x004C55B0 ?set_listener_xpos@Wave_Device@@QAEHM@Z 0x004C55B0-0x004C55D3
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
int Wave_Device::set_listener_xpos(float a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x90))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_xpos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_xpos(a1);
}

/*
Purpose: Forward get_listener_xpos to the wrapped device through its vtable slot
         0x94.
// ORIGINAL: 0x004C55E0 ?get_listener_xpos@Wave_Device@@QAEHPAM@Z 0x004C55E0-0x004C5603
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
int Wave_Device::get_listener_xpos(float *a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float *a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x94))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_xpos_redirect(Wave_Device *self, void *, float *a1) {
    return self->get_listener_xpos(a1);
}

/*
Purpose: Forward set_listener_ypos to the wrapped device through its vtable slot
         0x98.
// ORIGINAL: 0x004C5610 ?set_listener_ypos@Wave_Device@@QAEHM@Z 0x004C5610-0x004C5633
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
int Wave_Device::set_listener_ypos(float a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x98))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_ypos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_ypos(a1);
}

/*
Purpose: Forward get_listener_ypos to the wrapped device through its vtable slot
         0x9c.
// ORIGINAL: 0x004C5640 ?get_listener_ypos@Wave_Device@@QAEHPAM@Z 0x004C5640-0x004C5663
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
int Wave_Device::get_listener_ypos(float *a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float *a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x9c))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_ypos_redirect(Wave_Device *self, void *, float *a1) {
    return self->get_listener_ypos(a1);
}

/*
Purpose: Forward set_listener_zpos to the wrapped device through its vtable slot
         0xa0.
// ORIGINAL: 0x004C5670 ?set_listener_zpos@Wave_Device@@QAEHM@Z 0x004C5670-0x004C5693
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
int Wave_Device::set_listener_zpos(float a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0xa0))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_zpos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_zpos(a1);
}

/*
Purpose: Forward get_listener_zpos to the wrapped device through its vtable slot
         0xa4.
// ORIGINAL: 0x004C56A0 ?get_listener_zpos@Wave_Device@@QAEHPAM@Z 0x004C56A0-0x004C56C3
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
int Wave_Device::get_listener_zpos(float *a1) {
    if (device_14_) {
        typedef int (OriginalObject::*device_fn)(float *a1);
        return (ORIGINAL(device_14_)->*original_slot<device_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0xa4))(
            a1);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_zpos_redirect(Wave_Device *self, void *, float *a1) {
    return self->get_listener_zpos(a1);
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
    uint32_t vol;
    std::memcpy(&vol, reinterpret_cast<uint8_t *>(wave) + 4, 4);
    (ORIGINAL(wave)->*original_method<void (OriginalObject::*)(uint32_t) >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x40)))(
        vol);
}

void resume_wave(Wave *wave) {
    (ORIGINAL(wave)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x8C)))();
}

void halt_wave(Wave *wave) {
    (ORIGINAL(wave)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x14)))();
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

WaveResumeLinks *resume_links(Wave *wave) {
    return reinterpret_cast<WaveResumeLinks *>(
        reinterpret_cast<uint8_t *>(wave) + 0x44);
}

// The sound-side virtuals select dispatches through: the attribute word
// (slot 0x70), the chain-next and chain-prev accessors (slots 0x64/0x68),
// and load-by-name (slot 0x10).
int wave_attrib(Wave *wave) {
    return (ORIGINAL(wave)->*original_method<int (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x70)))();
}

Wave *wave_chain_next(Wave *wave) {
    return (ORIGINAL(wave)->*original_method<Wave * (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x64)))();
}

Wave *wave_chain_prev(Wave *wave) {
    return (ORIGINAL(wave)->*original_method<Wave * (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x68)))();
}

void load_wave_by_name(Wave *wave, char *fname) {
    (ORIGINAL(wave)->*original_method<int (OriginalObject::*)(char *) >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(wave) + 0x10)))(
        fname);
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

int __fastcall wave_device_set_group_volume_redirect(Wave_Device *self,
                                                     void *, unsigned int a1,
                                                     unsigned int a2) {
    return self->set_group_volume(a1, a2);
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

int __fastcall wave_device_enable_group_redirect(Wave_Device *self, void *,
                                                 unsigned int a1) {
    return self->enable_group(a1);
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

int __fastcall wave_device_disable_group_redirect(Wave_Device *self, void *,
                                                  unsigned int a1) {
    return self->disable_group(a1);
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
    if (!device_14_) {
        return 2;
    }
    Wave *resume_tail = nullptr;
    Wave *sound = WaveChainHead();
    while (sound) {
        if ((wave_attrib(sound) & 1) && !(wave_attrib(sound) & 4)) {
            Wave *const next = wave_chain_next(sound);
            halt_wave(sound);
            if (resume_tail) {
                resume_links(resume_tail)->next = sound;
                resume_links(sound)->prev = resume_tail;
            } else {
                resume_links(sound)->prev = nullptr;
                resume_links(sound)->next = nullptr;
            }
            resume_tail = sound;
            sound = next;
        } else {
            sound = wave_chain_next(sound);
        }
    }
    (ORIGINAL(device_14_)->*original_method<int (OriginalObject::*)(unsigned long) >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(device_14_) + 0x18)))(
        a1);
    for (Wave *replay = resume_tail; replay;) {
        char *const fname = resume_links(replay)->fname;
        Wave *const prev = wave_chain_prev(replay);
        resume_links(replay)->next = nullptr;
        resume_links(replay)->prev = nullptr;
        load_wave_by_name(replay, fname);
        replay = prev;
    }
    return 0;
}

int __fastcall wave_device_select_redirect(Wave_Device *self, void *,
                                           unsigned long a1) {
    return self->select(a1);
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

int __fastcall wave_device_create_device_redirect(Wave_Device *self, void *,
                                                  unsigned long a1) {
    return self->create_device(a1);
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

int __fastcall wave_device_delete_device_redirect(Wave_Device *self,
                                                  void *) {
    return self->delete_device();
}

/*
Purpose: Initialise the device stack: the Wave_Device's OWN virtual slot 0
         takes the mode first (the class does have a vtable, held opaque at
         offset zero), then the wrapped device initialises through its slot
         0xC with both arguments, and a failure there runs the Wave_Device's
         own virtual slot 4 before the error propagates.
// ORIGINAL: 0x004C4F40 ?init@Wave_Device@@QAEHPAXK@Z 0x004C4F40-0x004C4F7D
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
int Wave_Device::init(void *a1, unsigned long a2) {
    typedef int (OriginalObject::*own_mode_fn)(unsigned long mode);
    const int staged = (ORIGINAL(this)->*original_slot<own_mode_fn>(*reinterpret_cast<uint8_t **>(this) + 0))(
        a2);
    if (staged) {
        return staged;
    }
    typedef int (OriginalObject::*device_init_fn)(void *a1, unsigned long a2);
    const int result = (ORIGINAL(device_14_)->*original_slot<device_init_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0xC))(
        a1, a2);
    if (result) {
        (ORIGINAL(this)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(this) + 4)))();
        return result;
    }
    return 0;
}

int __fastcall wave_device_init_redirect(Wave_Device *self, void *, void *a1,
                                         unsigned long a2) {
    return self->init(a1, a2);
}

/*
Purpose: Release the wrapped device: its own vtable slot 0x10 winds it down,
         and only if the device is STILL there afterwards - the callback may
         have cleared it - does the destroy hook run. Either way the field is
         forgotten.
// ORIGINAL: 0x004C4F80 ?release@Wave_Device@@QAEXXZ 0x004C4F80-0x004C4FB3
// size      51 bytes
// prototype void (__thiscall ?release@Wave_Device@@QAEXXZ)(Wave_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C4F8C 0x004C4F9F
Return Value: n/a
Status: Complete
*/
void Wave_Device::release() {
    if (!device_14_) {
        return;
    }
    {
        typedef void (OriginalObject::*device_down_fn)();
        (ORIGINAL(device_14_)->*original_slot<device_down_fn>(*reinterpret_cast<uint8_t **>(device_14_) + 0x10))();
    }
    if (device_14_ && WaveDeviceDestroySlot()) {
        (WaveDeviceDestroySlot())();
    }
    device_14_ = nullptr;
}

void __fastcall wave_device_release_redirect(Wave_Device *self, void *) {
    self->release();
}

func_thiscall_teardown WaveControlGroupOriginalCtor =
    original_method<func_thiscall_teardown>(0x004C5490);
func_thiscall_teardown WaveControlGroupOriginalDtor =
    original_method<func_thiscall_teardown>(0x004C5B80);

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

Wave_Device *__fastcall wave_device_ctor_redirect(Wave_Device *self, void *) {
    return new (self) Wave_Device;
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
            typedef void (OriginalObject::*device_down_fn)();
            (ORIGINAL(device)->*original_slot<device_down_fn>(*reinterpret_cast<uint8_t **>(device) + 0x10))();
        }
    }
}

void __fastcall wave_device_dtor_redirect(Wave_Device *self, void *) {
    self->~Wave_Device();
}
