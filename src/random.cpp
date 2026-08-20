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
#include "stdafx.h"
#include "random.h"

#include <cstring>

/*
Purpose: Update the seed value. The original code had some convoluted XORs that served no purpose.
// ORIGINAL: 0x00625750 ?reseed@Random@@QAEXK@Z 0x00625750-0x00625763
// body      src/random.h
// symbol    ?reseed@Random@@QAEXI@Z
// size      19 bytes
// prototype void (__thiscall ?reseed@Random@@QAEXK@Z)(Random* this, unsigned int)
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Get a random value between min and (max - 1).
// ORIGINAL: 0x00625770 ?get@Random@@QAEIHH@Z 0x00625770-0x006257A7
// size      55 bytes
// prototype unsigned int (__thiscall ?get@Random@@QAEIHH@Z)(Random* this, int, int)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Random unsigned integer within bounds
Status: Complete
*/
// body src/random.h

/*
Purpose: Get a random double value.
// ORIGINAL: 0x006257B0 ?get@Random@@QAENXZ 0x006257B0-0x006257DA BYTE_EXACT
// size      42 bytes
// prototype unknown float (__thiscall ?get@Random@@QAENXZ)(Random* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Random double value
Status: Complete
*/
double Random::get() {
    seed_ = seed_ * 0x19660D + 0x3C6EF35F;
    const uint32_t bits = (seed_ & 0x7FFFFF) | 0x3F800000;
    float unit;
    std::memcpy(&unit, &bits, sizeof(unit));
    return static_cast<double>(unit) - 1.0;
}

// global
// THE OBJECT, not a pointer to one. `(Random *)0x009BB568` named storage
// inside terranx.exe, so every use of it in the recovered executable reaches
// a page this process does not have - and it cost every caller the ratchet
// besides, because through a pointer VC6 emits `mov ecx, [Rand]` where the
// image passes the object's address outright. Same defect `stringTemp` had.
Random Rand;   // 0x009BB568

// ORIGINAL: 0x00625700 ??__ERand@@YAXXZ 0x00625700-0x00625716 BYTE_EXACT
// symbol    ?random_rand@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398
// notes     Staged hybrid export redirect calls the source-owned initializer
void __cdecl random_rand() { Rand.reseed(0); atexit(random_rand_exit); }

// ORIGINAL: 0x00625720 ??__FRand@@YAXXZ 0x00625720-0x0062572B BYTE_EXACT
// symbol    ?random_rand_exit@@YAXXZ
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
void __cdecl random_rand_exit() { Rand.~Random(); }

// ORIGINAL: 0x006257E0 ?random_reseed@@YAXK@Z 0x006257E0-0x006257F9
// symbol    ?random_reseed@@YAXI@Z
// size      25 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned reseed wrapper
void __cdecl random_reseed(uint32_t new_seed) { Rand.reseed(new_seed); }

// ORIGINAL: 0x00625800 ?random_get@@YAIXZ 0x00625800-0x00625806 BYTE_EXACT
// size      6 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned seed getter
uint32_t __cdecl random_get() { return Rand.get_seed(); }

// ORIGINAL: 0x00625810 ?random@@YAIHH@Z 0x00625810-0x0062584F BYTE_EXACT
// size      63 bytes
// prototype 
// callers   38   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned integer generator
uint32_t __cdecl random(int min, int max) { return Rand.get(min, max); }

// ORIGINAL: 0x00625850 ?random@@YANXZ 0x00625850-0x00625880
// size      48 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned floating generator
double __cdecl random() { return Rand.get(); }


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00625730 ??0Random@@QAE@XZ 0x00625730-0x00625739 BYTE_EXACT
// body      src/random.h
// size      9 bytes
// prototype void (__thiscall ??0Random@@QAE@XZ)(Random* this)
// callers   5   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/

/*
// ORIGINAL: 0x00625740 ??1Random@@QAE@XZ 0x00625740-0x00625747 BYTE_EXACT
// body      src/random.h
// size      7 bytes
// prototype void (__thiscall ??1Random@@QAE@XZ)(Random* this)
// callers   5   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
