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

 private:
  uint32_t field_0_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint8_t memset_region_[0x24];
};

int __fastcall wave_set_asdr_redirect(Wave *self, void *);
