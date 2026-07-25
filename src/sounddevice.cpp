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
#include "sounddevice.h"

/*
Purpose: Midi_Device devices decline to be polled; the legacy implementation is a bare return.
Original Offset: 004C58B0
Return Value: n/a
Status: Complete
*/
void Midi_Device::update_sound() {
}

/*
Purpose: Midi_Device devices decline to be suspended; the legacy implementation is a bare return.
Original Offset: 004C58E0
Return Value: n/a
Status: Complete
*/
void Midi_Device::suspend() {
}

/*
Purpose: Midi_Device devices decline to be restarted; the legacy implementation is a bare return.
Original Offset: 004C58F0
Return Value: n/a
Status: Complete
*/
void Midi_Device::restart() {
}

/*
Purpose: Wave_In_Device devices decline to be polled; the legacy implementation is a bare return.
Original Offset: 004C5B10
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::update_sound() {
}

/*
Purpose: Wave_In_Device devices decline to be suspended; the legacy implementation is a bare return.
Original Offset: 004C5B40
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::suspend() {
}

/*
Purpose: Wave_In_Device devices decline to be restarted; the legacy implementation is a bare return.
Original Offset: 004C5B50
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
Original Offset: 004C5840
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
Original Offset: 004C5860
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
Original Offset: 004C5880
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
Original Offset: 004C5890
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
Original Offset: 004C58C0
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
Original Offset: 004C5AD0
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
Original Offset: 004C5AF0
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
Original Offset: 004C5B20
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
Original Offset: 004C5850
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
Original Offset: 004C5AE0
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
Original Offset: 004C5830
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C5870
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_volume() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C58A0
Return Value: 0, always
Status: Complete
*/
int Midi_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C58D0
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_rate() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C5AC0
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C5B00
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 004C5B30
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
typedef void(__thiscall *midi_device_vfn)(void *device);

void dispatch_midi_device(Midi_Device *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        (*reinterpret_cast<midi_device_vfn *>(vtable + vtable_offset))(device);
    }
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x54.
Original Offset: 004C5900
Return Value: n/a
Status: Complete
*/
void Midi_Device::enable() {
    dispatch_midi_device(this, 0x54);
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x58.
Original Offset: 004C5910
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
