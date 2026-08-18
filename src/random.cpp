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
// size      19 bytes
// prototype void (__thiscall ?reseed@Random@@QAEXK@Z)(Random* this, unsigned int)
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Random::reseed(uint32_t new_seed) { seed_ = new_seed; }

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
uint32_t Random::get(int min, int max) {
    if (static_cast<int32_t>(min) > static_cast<int32_t>(max)) {
        uint32_t temp = min;
        min = max;
        max = temp;
    }
    seed_ = seed_ * 0x19660D + 0x3C6EF35F;
    return (((max - min) * LOWORD(seed_)) >> 16) + min;
}

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
Random *Rand = (Random *)0x009BB568;

// ORIGINAL: 0x00625700 ??__ERand@@YAXXZ 0x00625700-0x00625716
// size      22 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398
// notes     Staged hybrid export redirect calls the source-owned initializer
void __cdecl random_rand() { Rand->reseed(0); atexit(random_rand_exit); }

// ORIGINAL: 0x00625720 ??__FRand@@YAXXZ 0x00625720-0x0062572B
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
void __cdecl random_rand_exit() { Rand->~Random(); }

// ORIGINAL: 0x006257E0 ?random_reseed@@YAXK@Z 0x006257E0-0x006257F9
// size      25 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned reseed wrapper
void __cdecl random_reseed(uint32_t new_seed) { Rand->reseed(new_seed); }

// ORIGINAL: 0x00625800 ?random_get@@YAIXZ 0x00625800-0x00625806
// size      6 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned seed getter
uint32_t __cdecl random_get() { return Rand->get_seed(); }

// ORIGINAL: 0x00625810 ?random@@YAIHH@Z 0x00625810-0x0062584F
// size      63 bytes
// prototype 
// callers   38   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned integer generator
uint32_t __cdecl random(uint32_t min, uint32_t max) { return Rand->get(min, max); }

// ORIGINAL: 0x00625850 ?random@@YANXZ 0x00625850-0x00625880
// size      48 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned floating generator
double __cdecl random() { return Rand->get(); }
