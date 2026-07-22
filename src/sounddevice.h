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
#pragma once

 /*
  * Sound device classes
  *
  * Two audio devices the game drives through a common interface. Their
  * constructors are byte-for-byte the same shape - the same shared vtable
  * installed at offset zero, the same four-byte memset at 0x4, 0x7F stored at
  * 0x8, zeros through 0x1C, then each swapping in its own vtable - which is
  * what places their common fields where they are here.
  *
  * The suspend, restart, and update_sound hooks recovered here do nothing in
  * the original: these devices decline to be suspended, restarted, or polled.
  *
  * Neither layout is established. The fields below are those the constructors
  * write, so the object extends at least to 0x20; g_MIDI_DEVICE's global slot
  * bounds Midi_Device above at 0x28, which is a bound and not a size. Nothing
  * pins their sizeof and no field may be appended without deriving it first.
  * The vtable pointer at offset zero is held as opaque storage so no C++
  * vtable is generated that could disagree with the original's.
  */
class DLLEXPORT Midi_Device {
 public:
  Midi_Device() { ; }
  ~Midi_Device() { ; }
  void update_sound();
  void suspend();
  void restart();

 private:
  uint32_t vtable_storage_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  uint32_t field_14_;
  uint32_t field_18_;
  uint32_t field_1C_;
};

class DLLEXPORT Wave_In_Device {
 public:
  Wave_In_Device() { ; }
  ~Wave_In_Device() { ; }
  void update_sound();
  void suspend();
  void restart();

 private:
  uint32_t vtable_storage_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  uint32_t field_14_;
  uint32_t field_18_;
  uint32_t field_1C_;
};

void __fastcall midi_device_update_sound_redirect(Midi_Device *self, void *);
void __fastcall midi_device_suspend_redirect(Midi_Device *self, void *);
void __fastcall midi_device_restart_redirect(Midi_Device *self, void *);
void __fastcall wave_in_device_update_sound_redirect(Wave_In_Device *self, void *);
void __fastcall wave_in_device_suspend_redirect(Wave_In_Device *self, void *);
void __fastcall wave_in_device_restart_redirect(Wave_In_Device *self, void *);
