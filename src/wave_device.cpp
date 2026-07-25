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
