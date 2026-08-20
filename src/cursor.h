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
  // `static`, because the image's name ends in `QAA` - a public member
  // declared __cdecl, taking no receiver. jackal_init_real calls it with
  // no ecx set up, so `Class::method()` is the only legal spelling.
  // `int`, though the catalogue spells the return `X` (void). The body at
  // 0x0063B910 ends `xor eax, eax; ret` - VC6's way of returning zero -
  // and jackal_init_real tests the result before continuing. Same family
  // as the four void-over-`xor eax, eax` names corrected before it.
  static int init_cursor_class();
  Cursor() { ; }
  ~Cursor();                         // 0063B8D0
  // Spelled as a method rather than a real constructor, as Ambience's is:
  // the inline `Cursor() { ; }` above is relied on elsewhere and giving
  // the class a real one would silently change every use of it.
  void construct();
  static void close_cursor_class();

 private:
  uint32_t field_0_;
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
};

// Returns the object, preserving the legacy EAX = this residue.
Cursor *__fastcall cursor_construct_redirect(Cursor *self, void *);
void __cdecl cursor_close_cursor_class_redirect();
