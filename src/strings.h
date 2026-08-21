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
// THE OBJECT, not a pointer to one, and a plain array rather than a
// wrapper. `stringTemp` used to be `(char1032 *)0x009B86A0` - the
// injected-DLL spelling, naming storage inside a process this is no longer
// part of, so every use of it in the recovered executable wrote to unmapped
// memory. It also cost every caller the ratchet: through a pointer the
// compiler emits `mov edx, [stringTemp]` where the image pushes the
// buffer's address outright. Seven functions use it and not one of them was
// claimed. The `char1032` struct existed only to give that pointer an
// `->str`; with the object itself declared there is nothing to wrap.
extern char StringTemp[1032];    // 0x009B86A0

// APPEND A DECIMAL NUMBER TO StringTemp, and IN THE HEADER because the
// image has it BOTH WAYS: a standalone body at 0x0050B8A0 with three
// callers, and folded into `prefs_get`, whose first eight instructions are
// byte-identical to this function's and differ only in which stack slot the
// argument comes from. That is what an `inline` in a header produces - the
// out-of-line copy comes from a unit that did not fold it - and it is what
// gave `prefs_get` its otherwise baffling shape: a local buffer, an emptied
// StringTemp and a `strcat` where a single `_itoa` would do.
//
// The two claims are measured under DIFFERENT flag sets: `prefs_get` under
// /O2, which folds this in, and this one under /O2 /Ob0, which does not.
// The per-function flag search is exactly what makes that possible.
MEASURED inline void __cdecl say_num(int value) {   // 0050B8A0
  char text[0x50];
  _itoa(value, text, 10);
  strcat(StringTemp, text);
}

class Strings : public Heap {
 public:
  MEASURED Strings() : is_populated_(false) {
  } // 006168D0
  // EMPTY. The image's is a five-byte `jmp` and this emits exactly that:
  // the implicit call to `~Heap` IS the whole destructor, and the linker
  // folded `~Heap` onto `Heap::shutdown`, which is why the disassembly's
  // target reads as the latter.
  MEASURED ~Strings() { }              // 006169C0

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
