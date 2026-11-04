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
/*
// ORIGINAL: 0x00625730 ??0Random@@QAE@XZ 0x00625730-0x00625739 BYTE_EXACT
// compile_context src/random.cpp
// size      9 bytes
// prototype void (__thiscall ??0Random@@QAE@XZ)(Random* this)
// callers   5   call targets   0
// kind
// flags
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
  MEASURED Random() : seed_(0) {
  } // 00625730
  // A PLAIN STORE. This carried a `volatile` cast to force the write - the
  // image is `mov dword ptr [ecx], 0; ret` and the fear was that VC6 would
  // elide a store to a member that is about to die. It does not: the plain
  // assignment is byte exact.
/*
// ORIGINAL: 0x00625740 ??1Random@@QAE@XZ 0x00625740-0x00625747 BYTE_EXACT
// compile_context src/random.cpp
// size      7 bytes
// prototype void (__thiscall ??1Random@@QAE@XZ)(Random* this)
// callers   5   call targets   0
// kind
// flags
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
  MEASURED ~Random() { seed_ = 0; }   // 00625740

  // The optimization notes below record historical inline trials. These
  // methods now have ordinary declaration-only interfaces here and sole
  // definitions in random.cpp; no body is exposed to callers.
  // HISTORICAL IN-CLASS rationale: the free wrappers folded these methods in,
  // as the image does: `random_reseed` is `mov ecx, [Rand]; xor ...` with no call
  // in it at all. Historically MEASURED was load-bearing here rather than
  // inherited style: under /Ob2 the optimiser folded reseed into its caller
  // and /Gy drops the COMDAT, so 0x00625750 was left with nothing to be
  // measured against. __declspec(dllexport) on the declaration forces the
  // out-of-line copy out whatever the expansion does - the pin the other
  // members here already carry.
  // AN XOR-SWAP THROUGH THE PARAMETER'S ADDRESS. The image really does swap
  // `seed_` with the argument - four xors and two stores - and the net effect
  // is only `seed_ = new_seed`, which is why this used to be written that way.
  // Written plainly (`seed_ ^= new_seed; new_seed ^= seed_; seed_ ^= new_seed;`)
  // VC6 recognises the idiom and folds it straight back to the single store,
  // three instructions against the image's eight. Reading and writing the
  // parameter through `*b` takes the non-aliasing away from it and the whole
  // chain survives - 0x00625750 and 0x006257E0 are both byte-exact on it.
  // These methods now use ordinary external linkage, not reconstruction-only
  // exports. Their sole definitions remain in random.cpp.
  void reseed(uint32_t new_seed);
  // HISTORICAL IN-CLASS rationale: the image's `random(int, int)` at
  // 0x00625810 has this body folded into it - nineteen instructions with no
  // call - and VC6 only inlines what it can see here.
  uint32_t get(int min, int max);   // 00625770
  // HISTORICAL IN-CLASS rationale, also applying to reseed() and get(int, int): the image's
  // free `random()` at 0x00625850 has this body FOLDED IN - twelve
  // instructions, `imul`/`and`/`or`/`fld`/`fsub`, with no call in it at all -
  // and VC6 only inlines what it can see here. It keeps its own claim because
  // /Ob0 still emits its COMDAT.
  double get();   // 006257B0
  // additional functions to assist with encapsulation
  uint32_t get_seed() { return seed_; }

 private:
  uint32_t seed_;  // 0x0000
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Random) == 4, "Random layout must match the legacy ABI");
#endif

// global
extern Random Rand;
void __cdecl random_reseed(uint32_t new_seed);
uint32_t __cdecl random_get();
uint32_t __cdecl random(int min, int max);
double __cdecl random();
