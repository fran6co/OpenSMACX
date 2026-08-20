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
class Midi_Device {
 public:
  Midi_Device() { ; }
  ~Midi_Device() { ; }
  int init(void *window, unsigned long backends);  // 004C57A0
  void update_sound();
  void suspend();
  void restart();

  int select(unsigned int a1);
  void set_volume(unsigned int a1);
  void set_pan(int a1);
  int fade(unsigned int a1);
  void set_rate(unsigned int a1);
  int get_description(unsigned int a1, char *a2, unsigned int a3);
  int get_ndevices();
  int get_volume();
  int stop();
  int get_rate();
  void enable();
  void disable();
  int is_disabled();
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

static_assert(sizeof(Midi_Device) == 0x20,
              "Midi_Device layout must match the original executable");

class Wave_In_Device {
 public:
  Wave_In_Device() { ; }
  ~Wave_In_Device() { ; }
  int init(void *window, unsigned long backends);  // 004C5A10
  void update_sound();
  void suspend();
  void restart();

  int select(unsigned int a1);
  int get_caps(unsigned int a1);
  void set_rate(unsigned int a1);
  int get_description(unsigned int a1, char *a2, unsigned int a3);
  int get_ndevices();
  int stop();
  int get_rate();
  int start_record();
  int end_record();
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

static_assert(sizeof(Wave_In_Device) == 0x20,
              "Wave_In_Device layout must match the original executable");

void __fastcall midi_device_update_sound_redirect(Midi_Device *self, void *);
void __fastcall midi_device_suspend_redirect(Midi_Device *self, void *);
void __fastcall midi_device_restart_redirect(Midi_Device *self, void *);
void __fastcall wave_in_device_update_sound_redirect(Wave_In_Device *self, void *);
void __fastcall wave_in_device_suspend_redirect(Wave_In_Device *self, void *);
void __fastcall wave_in_device_restart_redirect(Wave_In_Device *self, void *);
int __fastcall midi_device_select_redirect(Midi_Device *self, void *, unsigned int a1);
void __fastcall midi_device_set_volume_redirect(Midi_Device *self, void *, unsigned int a1);
void __fastcall midi_device_set_pan_redirect(Midi_Device *self, void *, int a1);
int __fastcall midi_device_fade_redirect(Midi_Device *self, void *, unsigned int a1);
void __fastcall midi_device_set_rate_redirect(Midi_Device *self, void *, unsigned int a1);
int __fastcall wave_in_device_select_redirect(Wave_In_Device *self, void *, unsigned int a1);
int __fastcall wave_in_device_get_caps_redirect(Wave_In_Device *self, void *, unsigned int a1);
void __fastcall wave_in_device_set_rate_redirect(Wave_In_Device *self, void *, unsigned int a1);
int __fastcall midi_device_get_description_redirect(Midi_Device *self, void *, unsigned int a1, char *a2, unsigned int a3);
int __fastcall wave_in_device_get_description_redirect(Wave_In_Device *self, void *, unsigned int a1, char *a2, unsigned int a3);
int __fastcall midi_device_get_ndevices_redirect(Midi_Device *self, void *);
int __fastcall midi_device_get_volume_redirect(Midi_Device *self, void *);
int __fastcall midi_device_stop_redirect(Midi_Device *self, void *);
int __fastcall midi_device_get_rate_redirect(Midi_Device *self, void *);
int __fastcall wave_in_device_get_ndevices_redirect(Wave_In_Device *self, void *);
int __fastcall wave_in_device_stop_redirect(Wave_In_Device *self, void *);
int __fastcall wave_in_device_get_rate_redirect(Wave_In_Device *self, void *);
void __fastcall midi_device_enable_redirect(Midi_Device *self, void *);
void __fastcall midi_device_disable_redirect(Midi_Device *self, void *);
int __fastcall midi_device_is_disabled_redirect(Midi_Device *self, void *);
int __fastcall wave_in_device_start_record_redirect(Wave_In_Device *self, void *);
int __fastcall wave_in_device_end_record_redirect(Wave_In_Device *self, void *);
