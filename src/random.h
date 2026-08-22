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

#include <cstring>

 /*
  * Random class: Handles pseudo random number generator.
  */
class Random {
 public:
  MEASURED Random() : seed_(0) {
  } // 00625730
  // A PLAIN STORE. This carried a `volatile` cast to force the write - the
  // image is `mov dword ptr [ecx], 0; ret` and the fear was that VC6 would
  // elide a store to a member that is about to die. It does not: the plain
  // assignment is byte exact.
  MEASURED ~Random() { seed_ = 0; }   // 00625740

  // IN-CLASS so the free wrappers below fold them in, which is what the
  // image does: `random_reseed` is `mov ecx, [Rand]; xor ...` with no call
  // in it at all. Each still emits its own COMDAT under /Ob0, so each keeps
  // its own claim.
  // AN XOR-SWAP THROUGH THE PARAMETER'S ADDRESS. The image really does swap
  // `seed_` with the argument - four xors and two stores - and the net effect
  // is only `seed_ = new_seed`, which is why this used to be written that way.
  // Written plainly (`seed_ ^= new_seed; new_seed ^= seed_; seed_ ^= new_seed;`)
  // VC6 recognises the idiom and folds it straight back to the single store,
  // three instructions against the image's eight. Reading and writing the
  // parameter through `*b` takes the non-aliasing away from it and the whole
  // chain survives - 0x00625750 and 0x006257E0 are both byte-exact on it.
  void reseed(uint32_t new_seed) {
      uint32_t *const b = &new_seed;
      seed_ ^= *b;
      *b ^= seed_;
      seed_ ^= *b;
  }
  // IN-CLASS for the same reason: the image's `random(int, int)` at
  // 0x00625810 has this body folded into it - nineteen instructions with no
  // call - and VC6 only inlines what it can see here.
  MEASURED uint32_t get(int min, int max) {   // 00625770
      if (min > max) {
          min ^= max;
          max ^= min;
          min ^= max;
      }
      const uint32_t range = static_cast<uint32_t>(max - min);
      seed_ = seed_ * 0x19660D + 0x3C6EF35F;
      // A LOCAL MASK, NOT THE `LOWORD` MACRO. `LOWORD(seed_)` truncates to a
      // 16-bit rvalue and VC6 satisfies that by RE-READING the low word from
      // `seed_`'s memory (`mov si, word ptr [ecx]`) right after the store
      // that just wrote it, rather than masking the register already
      // holding the value. Copying `seed_` into a 32-bit local first and
      // masking THAT keeps it in the register the image's `and edx, 0xffff`
      // reuses.
      const uint32_t new_seed = seed_;
      // UNSIGNED SHIFT. The image is `shr eax, 0x10`; with `int` operands
      // the same expression shifts arithmetically - `sar` - and that one
      // byte was the last divergence in `random(int, int)`.
      return ((range * (new_seed & 0xFFFF)) >> 16) + min;
  }
  // IN-CLASS for the same reason as reseed() and get(int, int): the image's
  // free `random()` at 0x00625850 has this body FOLDED IN - twelve
  // instructions, `imul`/`and`/`or`/`fld`/`fsub`, with no call in it at all -
  // and VC6 only inlines what it can see here. It keeps its own claim because
  // /Ob0 still emits its COMDAT.
  MEASURED double get() {   // 006257B0
      seed_ = seed_ * 0x19660D + 0x3C6EF35F;
      const uint32_t bits = (seed_ & 0x7FFFFF) | 0x3F800000;
      float unit;
      std::memcpy(&unit, &bits, sizeof(unit));
      return static_cast<double>(unit) - 1.0;
  }
  // additional functions to assist with encapsulation
  uint32_t get_seed() { return seed_; }

 private:
  uint32_t seed_;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Random) == 4, "Random layout must match the legacy ABI");
#endif

// global
extern Random Rand;
void __cdecl random_rand();
void __cdecl random_rand_exit();
void __cdecl random_reseed(uint32_t new_seed);
uint32_t __cdecl random_get();
uint32_t __cdecl random(int min, int max);
double __cdecl random();
