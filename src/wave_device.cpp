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

void dispatch_wrapped_device(Wave_Device *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        device_vfn fn = *reinterpret_cast<device_vfn *>(vtable + vtable_offset);
        fn(device);
    }
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
