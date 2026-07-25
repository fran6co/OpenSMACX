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
  * Layout not established; its methods reach as far as 0xB0, so the
  * object is at least that large. The one field these methods establish is a
  * pointer to the wrapped device at offset 0x14; the rest is opaque storage,
  * only an object for the canary to seed, not a modelled layout.
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

 private:
  uint8_t unmapped_[0xC0];
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
