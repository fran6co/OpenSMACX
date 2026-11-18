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
  * Initialization owns the Windows cursor and its copied Sprite. The
  * construction clears all four fields before the object owns resources.
  */
class Sprite;

class Cursor {
 public:
  // `static`, because the image's name ends in `QAA` - a public member
  // declared __cdecl, taking no receiver. jackal_init_real calls it with
  // no ecx set up, so `Class::method()` is the only legal spelling.
  // `int`, though the catalogue spells the return `X` (void). The body at
  // 0x0063B910 ends `xor eax, eax; ret` - VC6's way of returning zero -
  // and jackal_init_real tests the result before continuing. Same family
  // as the four void-over-`xor eax, eax` names corrected before it.
  static int init_cursor_class();
  Cursor();
  int init(Sprite *sprite, int hotspot_x, int hotspot_y);
  ~Cursor();                         // 0063B8D0
  static void close_cursor_class();

 private:
  HCURSOR cursor_handle_;  // 0x0000: CreateIconIndirect / DestroyCursor
  Sprite *owned_sprite_;  // 0x0004: constructed Sprite, closed and deleted
  int hotspot_x_;         // 0x0008: original caller coordinate
  int hotspot_y_;         // 0x000C: original caller coordinate
};


// Cached results of GetSystemMetrics(SM_CXCURSOR)/(SM_CYCURSOR), filled in
// by Cursor::init_cursor_class().
extern int CursorWidth;   // 0x009BE6D4
extern int CursorHeight;  // 0x009BE6D8
