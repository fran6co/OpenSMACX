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
