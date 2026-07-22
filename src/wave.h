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
  * Wave class
  *
  * Standalone. The constructor writes fields at 0, 4, and 8 then memsets
  * 0x24 bytes from 0xC, which places the extent at 0x30 or beyond;
  * g_ALPHAMENU_WAVE's global slot bounds it above at 0x70. A bound is not a
  * size, so nothing pins this and the storage below is the evidenced
  * minimum rather than the object.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT Wave {
 public:
  Wave() { ; }
  ~Wave() { ; }
  int set_asdr();

  int set_bufflimit(unsigned int a1);
  int set_attack(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_sustain(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_decay(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_release(unsigned int a1, unsigned int a2, unsigned int a3);
 private:
  uint32_t field_0_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint8_t memset_region_[0x24];
};

int __fastcall wave_set_asdr_redirect(Wave *self, void *);
int __fastcall wave_set_bufflimit_redirect(Wave *self, void *, unsigned int a1);
int __fastcall wave_set_attack_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_sustain_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_decay_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_release_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
