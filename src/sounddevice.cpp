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
#include "sounddevice.h"

/*
Purpose: Midi_Device devices decline to be polled; the legacy implementation is a bare return.
ORIGINAL: 0x004C58B0 BYTE_EXACT
// name      ?update_sound@Midi_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C58B0-0x004C58B1
// prototype void (__thiscall ?update_sound@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::update_sound() {
}

/*
Purpose: Midi_Device devices decline to be suspended; the legacy implementation is a bare return.
ORIGINAL: 0x004C58E0 BYTE_EXACT
// name      ?suspend@Midi_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C58E0-0x004C58E1
// prototype void (__thiscall ?suspend@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::suspend() {
}

/*
Purpose: Midi_Device devices decline to be restarted; the legacy implementation is a bare return.
ORIGINAL: 0x004C58F0 BYTE_EXACT
// name      ?restart@Midi_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C58F0-0x004C58F1
// prototype void (__thiscall ?restart@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::restart() {
}

/*
Purpose: Wave_In_Device devices decline to be polled; the legacy implementation is a bare return.
ORIGINAL: 0x004C5B10 BYTE_EXACT
// name      ?update_sound@Wave_In_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C5B10-0x004C5B11
// prototype void (__thiscall ?update_sound@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::update_sound() {
}

/*
Purpose: Wave_In_Device devices decline to be suspended; the legacy implementation is a bare return.
ORIGINAL: 0x004C5B40 BYTE_EXACT
// name      ?suspend@Wave_In_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C5B40-0x004C5B41
// prototype void (__thiscall ?suspend@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::suspend() {
}

/*
Purpose: Wave_In_Device devices decline to be restarted; the legacy implementation is a bare return.
ORIGINAL: 0x004C5B50 BYTE_EXACT
// name      ?restart@Wave_In_Device@@QAEXXZ
// size      1 bytes
// spans     0x004C5B50-0x004C5B51
// prototype void (__thiscall ?restart@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::restart() {
}

void __fastcall midi_device_update_sound_redirect(Midi_Device *self, void *) {
    self->update_sound();
}

void __fastcall midi_device_suspend_redirect(Midi_Device *self, void *) {
    self->suspend();
}

void __fastcall midi_device_restart_redirect(Midi_Device *self, void *) {
    self->restart();
}

void __fastcall wave_in_device_update_sound_redirect(Wave_In_Device *self, void *) {
    self->update_sound();
}

void __fastcall wave_in_device_suspend_redirect(Wave_In_Device *self, void *) {
    self->suspend();
}

void __fastcall wave_in_device_restart_redirect(Wave_In_Device *self, void *) {
    self->restart();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
ORIGINAL: 0x004C5840 BYTE_EXACT
// name      ?select@Midi_Device@@QAEHI@Z
// size      5 bytes
// spans     0x004C5840-0x004C5845
// prototype int (__thiscall ?select@Midi_Device@@QAEHI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::select(unsigned int) {
    return 0;
}

int __fastcall midi_device_select_redirect(Midi_Device *self, void *, unsigned int a1) {
    return self->select(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004C5860 BYTE_EXACT
// name      ?set_volume@Midi_Device@@QAEXI@Z
// size      3 bytes
// spans     0x004C5860-0x004C5863
// prototype void (__thiscall ?set_volume@Midi_Device@@QAEXI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_volume(unsigned int) {
}

void __fastcall midi_device_set_volume_redirect(Midi_Device *self, void *, unsigned int a1) {
    self->set_volume(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004C5880 BYTE_EXACT
// name      ?set_pan@Midi_Device@@QAEXH@Z
// size      3 bytes
// spans     0x004C5880-0x004C5883
// prototype void (__thiscall ?set_pan@Midi_Device@@QAEXH@Z)(Midi_Device* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_pan(int) {
}

void __fastcall midi_device_set_pan_redirect(Midi_Device *self, void *, int a1) {
    self->set_pan(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
ORIGINAL: 0x004C5890 BYTE_EXACT
// name      ?fade@Midi_Device@@QAEHI@Z
// size      5 bytes
// spans     0x004C5890-0x004C5895
// prototype int (__thiscall ?fade@Midi_Device@@QAEHI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::fade(unsigned int) {
    return 0;
}

int __fastcall midi_device_fade_redirect(Midi_Device *self, void *, unsigned int a1) {
    return self->fade(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004C58C0 BYTE_EXACT
// name      ?set_rate@Midi_Device@@QAEXI@Z
// size      3 bytes
// spans     0x004C58C0-0x004C58C3
// prototype void (__thiscall ?set_rate@Midi_Device@@QAEXI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_rate(unsigned int) {
}

void __fastcall midi_device_set_rate_redirect(Midi_Device *self, void *, unsigned int a1) {
    self->set_rate(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
ORIGINAL: 0x004C5AD0 BYTE_EXACT
// name      ?select@Wave_In_Device@@QAEHI@Z
// size      5 bytes
// spans     0x004C5AD0-0x004C5AD5
// prototype int (__thiscall ?select@Wave_In_Device@@QAEHI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::select(unsigned int) {
    return 0;
}

int __fastcall wave_in_device_select_redirect(Wave_In_Device *self, void *, unsigned int a1) {
    return self->select(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
ORIGINAL: 0x004C5AF0 BYTE_EXACT
// name      ?get_caps@Wave_In_Device@@QAEHI@Z
// size      5 bytes
// spans     0x004C5AF0-0x004C5AF5
// prototype int (__thiscall ?get_caps@Wave_In_Device@@QAEHI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_caps(unsigned int) {
    return 0;
}

int __fastcall wave_in_device_get_caps_redirect(Wave_In_Device *self, void *, unsigned int a1) {
    return self->get_caps(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004C5B20 BYTE_EXACT
// name      ?set_rate@Wave_In_Device@@QAEXI@Z
// size      3 bytes
// spans     0x004C5B20-0x004C5B23
// prototype void (__thiscall ?set_rate@Wave_In_Device@@QAEXI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::set_rate(unsigned int) {
}

void __fastcall wave_in_device_set_rate_redirect(Wave_In_Device *self, void *, unsigned int a1) {
    self->set_rate(a1);
}

/*
Purpose: Report no device description; the legacy implementation returns 0.
ORIGINAL: 0x004C5850 BYTE_EXACT
// name      ?get_description@Midi_Device@@QAEHIPADI@Z
// size      5 bytes
// spans     0x004C5850-0x004C5855
// prototype int (__thiscall ?get_description@Midi_Device@@QAEHIPADI@Z)(Midi_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_description(unsigned int, char *, unsigned int) {
    return 0;
}

int __fastcall midi_device_get_description_redirect(Midi_Device *self, void *, unsigned int a1, char *a2, unsigned int a3) {
    return self->get_description(a1, a2, a3);
}

/*
Purpose: Report no device description; the legacy implementation returns 0.
ORIGINAL: 0x004C5AE0 BYTE_EXACT
// name      ?get_description@Wave_In_Device@@QAEHIPADI@Z
// size      5 bytes
// spans     0x004C5AE0-0x004C5AE5
// prototype int (__thiscall ?get_description@Wave_In_Device@@QAEHIPADI@Z)(Wave_In_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_description(unsigned int, char *, unsigned int) {
    return 0;
}

int __fastcall wave_in_device_get_description_redirect(Wave_In_Device *self, void *, unsigned int a1, char *a2, unsigned int a3) {
    return self->get_description(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C5830 BYTE_EXACT
// name      ?get_ndevices@Midi_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C5830-0x004C5833
// prototype int (__thiscall ?get_ndevices@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C5870 BYTE_EXACT
// name      ?get_volume@Midi_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C5870-0x004C5873
// prototype int (__thiscall ?get_volume@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_volume() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C58A0 BYTE_EXACT
// name      ?stop@Midi_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C58A0-0x004C58A3
// prototype int (__thiscall ?stop@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C58D0 BYTE_EXACT
// name      ?get_rate@Midi_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C58D0-0x004C58D3
// prototype int (__thiscall ?get_rate@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_rate() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C5AC0 BYTE_EXACT
// name      ?get_ndevices@Wave_In_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C5AC0-0x004C5AC3
// prototype int (__thiscall ?get_ndevices@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C5B00 BYTE_EXACT
// name      ?stop@Wave_In_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C5B00-0x004C5B03
// prototype int (__thiscall ?stop@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x004C5B30 BYTE_EXACT
// name      ?get_rate@Wave_In_Device@@QAEHXZ
// size      3 bytes
// spans     0x004C5B30-0x004C5B33
// prototype int (__thiscall ?get_rate@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_rate() {
    return 0;
}

int __fastcall midi_device_get_ndevices_redirect(Midi_Device *self, void *) {
    return self->get_ndevices();
}

int __fastcall midi_device_get_volume_redirect(Midi_Device *self, void *) {
    return self->get_volume();
}

int __fastcall midi_device_stop_redirect(Midi_Device *self, void *) {
    return self->stop();
}

int __fastcall midi_device_get_rate_redirect(Midi_Device *self, void *) {
    return self->get_rate();
}

int __fastcall wave_in_device_get_ndevices_redirect(Wave_In_Device *self, void *) {
    return self->get_ndevices();
}

int __fastcall wave_in_device_stop_redirect(Wave_In_Device *self, void *) {
    return self->stop();
}

int __fastcall wave_in_device_get_rate_redirect(Wave_In_Device *self, void *) {
    return self->get_rate();
}

namespace {
// Midi_Device wraps its device at 0x14, the same offset Wave_Device uses, and
// drives it through the device's own vtable.
typedef void (OriginalObject::*midi_device_vfn)();

void dispatch_midi_device(Midi_Device *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        (ORIGINAL(device)->*original_slot<midi_device_vfn>(vtable + vtable_offset))();
    }
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x54.
ORIGINAL: 0x004C5900
// name      ?enable@Midi_Device@@QAEXXZ
// size      15 bytes
// spans     0x004C5900-0x004C590F
// prototype void (__thiscall ?enable@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::enable() {
    dispatch_midi_device(this, 0x54);
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x58.
ORIGINAL: 0x004C5910
// name      ?disable@Midi_Device@@QAEXXZ
// size      15 bytes
// spans     0x004C5910-0x004C591F
// prototype void (__thiscall ?disable@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::disable() {
    dispatch_midi_device(this, 0x58);
}

void __fastcall midi_device_enable_redirect(Midi_Device *self, void *) {
    self->enable();
}

void __fastcall midi_device_disable_redirect(Midi_Device *self, void *) {
    self->disable();
}

namespace {
typedef int (OriginalObject::*device_query_vfn)();

// The querying form of the same dispatch, shared by both device classes: the
// original tail-jumps or calls into the device's method, and answers a fixed
// value when no device is wrapped.
int query_device_at_14(void *self, int vtable_offset, int no_device_result) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (ORIGINAL(device)->*original_slot<device_query_vfn>(vtable + vtable_offset))();
}
}  // namespace

/*
Purpose: Ask the wrapped device whether it is disabled, through vtable slot
         0x5C. With no device wrapped the answer is yes.
ORIGINAL: 0x004C5920
// name      ?is_disabled@Midi_Device@@QAEHXZ
// size      18 bytes
// spans     0x004C5920-0x004C5932
// prototype int (__thiscall ?is_disabled@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 1 when none is wrapped
Status: Complete
*/
int Midi_Device::is_disabled() {
    return query_device_at_14(this, 0x5C, 1);
}

/*
Purpose: Ask the wrapped device to start recording, through vtable slot 0x58.
         The original discards its answer.
ORIGINAL: 0x004C5B60
// name      ?start_record@Wave_In_Device@@QAEHXZ
// size      15 bytes
// spans     0x004C5B60-0x004C5B6F
// prototype int (__thiscall ?start_record@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::start_record() {
    query_device_at_14(this, 0x58, 0);
    return 0;
}

/*
Purpose: Ask the wrapped device to stop recording, through vtable slot 0x5C.
         The original discards its answer.
ORIGINAL: 0x004C5B70
// name      ?end_record@Wave_In_Device@@QAEHXZ
// size      15 bytes
// spans     0x004C5B70-0x004C5B7F
// prototype int (__thiscall ?end_record@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::end_record() {
    query_device_at_14(this, 0x5C, 0);
    return 0;
}

int __fastcall midi_device_is_disabled_redirect(Midi_Device *self, void *) {
    return self->is_disabled();
}

int __fastcall wave_in_device_start_record_redirect(Wave_In_Device *self, void *) {
    return self->start_record();
}

int __fastcall wave_in_device_end_record_redirect(Wave_In_Device *self, void *) {
    return self->end_record();
}
