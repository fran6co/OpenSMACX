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
#include "wave_device.h"
#include <cstring>

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004C5180
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_pan(int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C51A0
Return Value: 0, always
Status: Complete
*/
int Wave_Device::fade(int) {
    return 0;
}

namespace {
// The wrapped device is driven through its own vtable; enable and disable call
// two adjacent slots on it. A raw dispatch is used because the device is an
// opaque object with no source-owned type.
typedef void(__thiscall *device_vfn)(void *device);
typedef int(__thiscall *device_query_vfn)(void *device);

void *wrapped_device(Wave_Device *self) {
    return *reinterpret_cast<void **>(reinterpret_cast<uint8_t *>(self) + 0x14);
}

void dispatch_wrapped_device(Wave_Device *self, int vtable_offset) {
    void *device = wrapped_device(self);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        device_vfn fn = *reinterpret_cast<device_vfn *>(vtable + vtable_offset);
        fn(device);
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
    return fn(device);
}

// The one-argument members of the family. The original passes the argument
// straight through and, where it returns at all, answers a fixed value when no
// device is wrapped.
typedef int(__thiscall *device_arg_vfn)(void *device, int a1);

int forward_to_wrapped_device(Wave_Device *self, int vtable_offset, int a1,
                              int no_device_result = 0) {
    void *device = wrapped_device(self);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (*reinterpret_cast<device_arg_vfn *>(vtable + vtable_offset))(device,
                                                                        a1);
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x60.
Original Offset: 004C51C0
Return Value: n/a
Status: Complete
*/
void Wave_Device::enable() {
    dispatch_wrapped_device(this, 0x60);
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x64.
Original Offset: 004C51D0
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
Original Offset: 004C5170
Return Value: 0, always
Status: Complete
*/
int Wave_Device::get_volume() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C51B0
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
Original Offset: 004C4FC0
Return Value: n/a
Status: Complete
*/
void Wave_Device::suspend() {
    dispatch_wrapped_device(this, 0x48);
}

/*
Purpose: Ask the wrapped device to restart, through its vtable slot 0x4C.
Original Offset: 004C4FD0
Return Value: n/a
Status: Complete
*/
void Wave_Device::restart() {
    dispatch_wrapped_device(this, 0x4C);
}

/*
Purpose: Poll the wrapped device, through its vtable slot 0x34.
Original Offset: 004C4FE0
Return Value: n/a
Status: Complete
*/
void Wave_Device::update_sound() {
    dispatch_wrapped_device(this, 0x34);
}

/*
Purpose: Ask the wrapped device how many devices it offers, through its vtable
         slot 0x14.
Original Offset: 004C4FF0
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_ndevices() {
    return query_wrapped_device(this, 0x14);
}

/*
Purpose: Ask the wrapped device for its hardware memory size, through its
         vtable slot 0x58.
Original Offset: 004C5020
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_hw_mem_size() {
    return query_wrapped_device(this, 0x58);
}

/*
Purpose: Ask the wrapped device for its sample rate, through its vtable
         slot 0x3C.
Original Offset: 004C5140
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::get_rate() {
    return query_wrapped_device(this, 0x3C);
}

/*
Purpose: Ask the wrapped device for its DirectSound interface, through its
         vtable slot 0x70.
Original Offset: 004C5190
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
Original Offset: 004C54B0
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
Original Offset: 004C51E0
Return Value: the device's answer, or 1 when none is wrapped
Status: Complete
*/
int Wave_Device::is_disabled() {
    return query_wrapped_device(this, 0x68, 1);
}

/*
Purpose: Ask the wrapped device to stop dumping raw audio, through vtable slot
         0x54.
Original Offset: 004C5220
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
Original Offset: 004C5530
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Wave_Device::is_3d() {
    return query_wrapped_device(this, 0x84);
}

/*
Purpose: Hand the sample rate to the wrapped device, through vtable slot 0x38.
Original Offset: 004C5120
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_rate(unsigned long a1) {
    forward_to_wrapped_device(this, 0x38, static_cast<int>(a1));
}

/*
Purpose: Hand the volume to the wrapped device, through vtable slot 0x20.
Original Offset: 004C5150
Return Value: n/a
Status: Complete
*/
void Wave_Device::set_volume(unsigned long a1) {
    forward_to_wrapped_device(this, 0x20, static_cast<int>(a1));
}

/*
Purpose: Hand the game window to the wrapped device, through vtable slot 0x6C.
Original Offset: 004C5000
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
Original Offset: 004C5380
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

func_wave_group_insert *WaveDeviceGroupInsert = (func_wave_group_insert *)0x004C5BF0;

/*
Purpose: Put a wave into one of the sixteen groups. The list-insert helper
         threads it through the group's node list, and the wave's own group
         slot at 0x68 records the group AFTER the insert - the original
         writes the field last, so the helper still sees the old slot.
Original Offset: 004C5240
Return Value: 0, or 0xA for a bad group or a null wave
Status: Complete
*/
int Wave_Device::add_to_group(unsigned int a1, Wave *a2) {
    if (a1 > 0xF || !a2) {
        return 0xA;
    }
    WaveDeviceGroupInsert(&groups_[a1].head, a2);
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
Original Offset: 004C5280
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
    WaveGroup &group = groups_[slot];
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
        WaveOperatorDelete(node);
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
Original Offset: 004C5460
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
Original Offset: 004C5BF0
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
Original Offset: 004C50F0
Return Value: n/a
Status: Complete
*/
void Wave_Device::get_description(unsigned long a1, char *a2, unsigned long a3) {
    if (device_14_) {
        typedef void(__thiscall * device_fn)(void *device, unsigned long a1, char *a2, unsigned long a3);
        (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x1c))(
            device_14_, a1, a2, a3);
    }
}

void __fastcall wave_device_get_description_redirect(Wave_Device *self, void *, unsigned long a1, char *a2, unsigned long a3) {
    self->get_description(a1, a2, a3);
}

/*
Purpose: Forward start_raw_dump to the wrapped device through its vtable slot
         0x50.
Original Offset: 004C5200
Return Value: the device's answer, or 3 when no device is wrapped
Status: Complete
*/
int Wave_Device::start_raw_dump(char *a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, char *a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x50))(
            device_14_, a1);
    }
    return 3;
}

int __fastcall wave_device_start_raw_dump_redirect(Wave_Device *self, void *, char *a1) {
    return self->start_raw_dump(a1);
}

/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x7c.
Original Offset: 004C54C0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax(EAX_REVERB_PROPERTIES *a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, EAX_REVERB_PROPERTIES *a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x7c))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_props_redirect(Wave_Device *self, void *, EAX_REVERB_PROPERTIES *a1) {
    return self->set_eax(a1);
}

/*
Purpose: Forward set_eax to the wrapped device through its vtable slot
         0x78.
Original Offset: 004C54E0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax(unsigned long a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, unsigned long a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x78))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_redirect(Wave_Device *self, void *, unsigned long a1) {
    return self->set_eax(a1);
}

/*
Purpose: Forward set_eax_mix to the wrapped device through its vtable slot
         0x80.
Original Offset: 004C5500
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_eax_mix(float a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x80))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_eax_mix_redirect(Wave_Device *self, void *, float a1) {
    return self->set_eax_mix(a1);
}

/*
Purpose: Forward set_listener_position to the wrapped device through its vtable slot
         0x88.
Original Offset: 004C5550
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_position(float a1, float a2, float a3) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float a1, float a2, float a3);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x88))(
            device_14_, a1, a2, a3);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_position_redirect(Wave_Device *self, void *, float a1, float a2, float a3) {
    return self->set_listener_position(a1, a2, a3);
}

/*
Purpose: Forward get_listener_position to the wrapped device through its vtable slot
         0x8c.
Original Offset: 004C5580
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_position(float *a1, float *a2, float *a3) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float *a1, float *a2, float *a3);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x8c))(
            device_14_, a1, a2, a3);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_position_redirect(Wave_Device *self, void *, float *a1, float *a2, float *a3) {
    return self->get_listener_position(a1, a2, a3);
}

/*
Purpose: Forward set_listener_xpos to the wrapped device through its vtable slot
         0x90.
Original Offset: 004C55B0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_xpos(float a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x90))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_xpos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_xpos(a1);
}

/*
Purpose: Forward get_listener_xpos to the wrapped device through its vtable slot
         0x94.
Original Offset: 004C55E0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_xpos(float *a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float *a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x94))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_xpos_redirect(Wave_Device *self, void *, float *a1) {
    return self->get_listener_xpos(a1);
}

/*
Purpose: Forward set_listener_ypos to the wrapped device through its vtable slot
         0x98.
Original Offset: 004C5610
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_ypos(float a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x98))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_ypos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_ypos(a1);
}

/*
Purpose: Forward get_listener_ypos to the wrapped device through its vtable slot
         0x9c.
Original Offset: 004C5640
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_ypos(float *a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float *a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0x9c))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_get_listener_ypos_redirect(Wave_Device *self, void *, float *a1) {
    return self->get_listener_ypos(a1);
}

/*
Purpose: Forward set_listener_zpos to the wrapped device through its vtable slot
         0xa0.
Original Offset: 004C5670
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::set_listener_zpos(float a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0xa0))(
            device_14_, a1);
    }
    return 0x14;
}

int __fastcall wave_device_set_listener_zpos_redirect(Wave_Device *self, void *, float a1) {
    return self->set_listener_zpos(a1);
}

/*
Purpose: Forward get_listener_zpos to the wrapped device through its vtable slot
         0xa4.
Original Offset: 004C56A0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave_Device::get_listener_zpos(float *a1) {
    if (device_14_) {
        typedef int(__thiscall * device_fn)(void *device, float *a1);
        return (*reinterpret_cast<device_fn *>(
            *reinterpret_cast<uint8_t **>(device_14_) + 0xa4))(
            device_14_, a1);
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
void walk_group_waves(WaveGroup &group, void (*visit)(Wave *wave)) {
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
    (*reinterpret_cast<void(__thiscall **)(Wave *, uint32_t)>(
        *reinterpret_cast<uint8_t **>(wave) + 0x40))(wave, vol);
}

void resume_wave(Wave *wave) {
    (*reinterpret_cast<void(__thiscall **)(Wave *)>(
        *reinterpret_cast<uint8_t **>(wave) + 0x8C))(wave);
}

void halt_wave(Wave *wave) {
    (*reinterpret_cast<void(__thiscall **)(Wave *)>(
        *reinterpret_cast<uint8_t **>(wave) + 0x14))(wave);
}

}  // namespace

/*
Purpose: Set a group's volume scale and replay every member wave's own
         stored volume through its vtable slot 0x40, so the new scale takes
         effect. Bad slots and volumes above 0x7F answer 0xA.
Original Offset: 004C5320
Return Value: 0, or 0xA for a bad slot or volume
Status: Complete
*/
int Wave_Device::set_group_volume(unsigned int a1, unsigned int a2) {
    if (a1 > 0xF || a2 > 0x7F) {
        return 0xA;
    }
    WaveGroup &group = groups_[a1];
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
Original Offset: 004C53A0
Return Value: 0, or 0xA for a bad slot
Status: Complete
*/
int Wave_Device::enable_group(unsigned int a1) {
    if (a1 > 0xF) {
        return 0xA;
    }
    WaveGroup &group = groups_[a1];
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
Original Offset: 004C5400
Return Value: 0, or 0xA for a bad slot
Status: Complete
*/
int Wave_Device::disable_group(unsigned int a1) {
    if (a1 > 0xF) {
        return 0xA;
    }
    WaveGroup &group = groups_[a1];
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
