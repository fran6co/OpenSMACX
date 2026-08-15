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
#include "heap.h"

 /*
  * Strings class: Create and interact with a basic string table.
  */
class DLLEXPORT Strings : Heap {
 public:
  Strings() : is_populated_(false) {
  } // 006168D0
  ~Strings() { Heap::shutdown(); }     // 006169C0

  BOOL init(size_t mem_size);
  void shutdown();
  LPSTR put(LPCSTR input);
  LPSTR get(int address);

 private:
  BOOL is_populated_; // (+20) -> set to true when table is created
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Strings) == 0x18, "Strings layout must match the legacy ABI");
#endif

// global
// THE ADDRESS, NOT A VARIABLE HOLDING IT. `jackal_init_real` calls
// `StringTable->init(0x8000)` and the image sets the receiver up as
// `mov ecx, 0x9b90d8` - an immediate. An `extern Strings *` compiles the
// same source to `mov ecx, dword ptr [0x9b90d8]`, a load the image never
// performs, and the extra instruction was the whole gap between that
// function and BYTE_EXACT. Call sites are unchanged: `->` still works.
// Safe as a header-scope static because nothing takes `&StringTable`;
// see the note on `GenderDefault` in general.h for the two cases where
// this spelling is NOT safe.
static Strings *const StringTable = (Strings *)0x009B90D8;
