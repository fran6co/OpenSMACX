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
  * Wave_Device class
  *
  * Layout not established. The one field these methods name is the pointer to
  * the wrapped device at 0x14. get_group_volume is what sizes the storage
  * below: it indexes a sixteen-entry table of 24-byte records based at 0x28,
  * so the object reaches at least 0x194, well past the 0xB0 the other methods
  * suggested. That is a floor, not a size - the rest is opaque storage, an
  * object for the canary to seed rather than a modelled layout.
  */
class DLLEXPORT Wave_Device {
 public:
  Wave_Device() { ; }
  ~Wave_Device() { ; }
  void set_pan(int);
  int fade(int);
  void enable();
  void disable();
  int get_volume();
  int stop();
  void suspend();
  void restart();
  void update_sound();
  int get_ndevices();
  int get_hw_mem_size();
  int get_rate();
  int get_ds();
  int is_eax();
  int is_disabled();
  int stop_raw_dump();
  int is_3d();
  void set_rate(unsigned long a1);
  void set_volume(unsigned long a1);
  int set_hwnd(void *a1);
  int get_group_volume(unsigned int a1);

 private:
  uint8_t unmapped_[0x194];
};

void __fastcall wave_device_set_pan_redirect(Wave_Device *self, void *, int a1);
int __fastcall wave_device_fade_redirect(Wave_Device *self, void *, int a1);
void __fastcall wave_device_enable_redirect(Wave_Device *self, void *);
void __fastcall wave_device_disable_redirect(Wave_Device *self, void *);
int __fastcall wave_device_get_volume_redirect(Wave_Device *self, void *);
int __fastcall wave_device_stop_redirect(Wave_Device *self, void *);
void __fastcall wave_device_suspend_redirect(Wave_Device *self, void *);
void __fastcall wave_device_restart_redirect(Wave_Device *self, void *);
void __fastcall wave_device_update_sound_redirect(Wave_Device *self, void *);
int __fastcall wave_device_get_ndevices_redirect(Wave_Device *self, void *);
int __fastcall wave_device_get_hw_mem_size_redirect(Wave_Device *self, void *);
int __fastcall wave_device_get_rate_redirect(Wave_Device *self, void *);
int __fastcall wave_device_get_ds_redirect(Wave_Device *self, void *);
int __fastcall wave_device_is_eax_redirect(Wave_Device *self, void *);
int __fastcall wave_device_is_disabled_redirect(Wave_Device *self, void *);
int __fastcall wave_device_stop_raw_dump_redirect(Wave_Device *self, void *);
int __fastcall wave_device_is_3d_redirect(Wave_Device *self, void *);
void __fastcall wave_device_set_rate_redirect(Wave_Device *self, void *, unsigned long a1);
void __fastcall wave_device_set_volume_redirect(Wave_Device *self, void *, unsigned long a1);
int __fastcall wave_device_set_hwnd_redirect(Wave_Device *self, void *, void *a1);
int __fastcall wave_device_get_group_volume_redirect(Wave_Device *self, void *, unsigned int a1);
