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
  * Cursor class
  *
  * Standalone - no base constructor runs. The constructor writes four
  * dwords and returns, which is what the storage below models; whether
  * anything uninitialised follows is not established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT Cursor {
 public:
  Cursor() { ; }
  ~Cursor() { ; }
  static void close_cursor_class();

 private:
  uint32_t field_0_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
};

void __cdecl cursor_close_cursor_class_redirect();
