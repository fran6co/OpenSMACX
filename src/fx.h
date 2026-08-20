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
#include "vector_teardown.h"

 /*
  * FX class - the sound-effect bank.
  *
  * The destructor walks 0x61 elements of 0x6C bytes starting at +0 through
  * the CRT vector iterator, and the per-element destructor it passes is a
  * jump thunk onto Wave's - so the bank is an array of Wave-shaped effects,
  * and 0x6C independently confirms the Wave extent the atexit thunk table
  * pinned. The extent here is at least 0x61 * 0x6C; nothing pins more.
  */
class FX {
 public:
  void fade(int a1);
  void release(int a1);
  void stop(int a1);
  FX();
  ~FX();

 private:
  uint8_t effects_[0x61 * 0x6C];

  // Storage the image proves is here: its own methods reach 0x28F0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 1 member(s) from the IDA database, 0 named; it starts a member at 0x28EC, which is where src/ ends.

  uint32_t field_28EC_;  // 0x28EC
};

static_assert(sizeof(FX) == 0x28F0,
              "FX layout must match the original executable");

// The per-element destructor the original passes: the Effect jump thunk at
// 0x004482C0, which lands on the Wave destructor. Held as its own rebindable
// dependency so the bank teardown retires with the Wave work.
extern func_thiscall_teardown EffectElementTeardown;

// Its construction-side twin, ??0Effect@@QAE@XZ at 0x004482D0. Still original;
// the constructor passes it and the teardown above to the same CRT iterator,
// so both retire together with the Wave work.
extern func_thiscall_teardown EffectElementCtor;

FX *__fastcall fx_ctor_redirect(FX *self, void *);
void __fastcall fx_dtor_redirect(FX *self, void *);
