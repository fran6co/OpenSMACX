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





// Binary64 one emitted by the double-valued expressions below; the original
// Random::get at 0x006257D2 reads this qword. This binds its start only.
// RECOVERY_OBJECT: v1 0x0066D8D8 __real@8@3fff8000000000000000


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
Random::Random() : seed_(0) {
}

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
Random::~Random() { seed_ = 0; }


/*
Purpose: Update the seed value, by the xor-swap with the argument that the image spells out.
// ORIGINAL: 0x00625750 ?reseed@Random@@QAEXK@Z 0x00625750-0x00625763 BYTE_EXACT
// LEVER: BYTE_EXACT 8/8. The body IS the xor-swap - `seed_ ^= n; n ^= seed_; seed_ ^= n;` - but written PLAINLY, VC6 recognises the idiom and folds the whole thing to the one `mov dword ptr [ecx], eax` the previous `seed_ = new_seed;` already emitted (measured: 3 instructions against the image's 8). Taking the ADDRESS of the parameter and xoring through it, `uint32_t *const b = &new_seed;`, denies VC6 the non-aliasing that algebra needs, and it then emits the image's four xors and two stores exactly. The same one change also took 0x006257E0 byte-exact.
// symbol    ?reseed@Random@@QAEXI@Z
// size      19 bytes
// prototype void (__thiscall ?reseed@Random@@QAEXK@Z)(Random* this, unsigned int)
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
// LEVER (2026-09-03, the two-flag-set ladder): `MEASURED` on the in-class
//        declaration in random.h. Under /Ob2 the optimiser folded reseed into
//        its one caller and /Gy dropped the COMDAT, so 0x00625750 had nothing
//        to be measured against; __declspec(dllexport) forces the
//        out-of-line copy out whatever the expansion does, and the free
//        wrapper at 0x006257E0 still folds it. The body itself is unchanged.
*/
void Random::reseed(uint32_t new_seed) {
    uint32_t *const b = &new_seed;
    seed_ ^= *b;
    *b ^= seed_;
    seed_ ^= *b;
}

/*
Purpose: Get a random value between min and (max - 1).
// ORIGINAL: 0x00625770 ?get@Random@@QAEIHH@Z 0x00625770-0x006257A7 BYTE_EXACT
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
    if (min > max) {
        min ^= max;
        max ^= min;
        min ^= max;
    }
    const uint32_t range = static_cast<uint32_t>(max) - min;
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
// Original 0x00625705/0x00625720 zero the state updated by the reseed/generator pair; no historical class claim.
// RECOVERY_OBJECT: v1 0x009BB568 ?Rand@@3VRandom@@A
Random Rand;   // 0x009BB568

// ORIGINAL: 0x00625700 ??__ERand@@YAXXZ 0x00625700-0x00625716 FILE BYTE_EXACT
// symbol    _$E4
// size      22 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398
// notes     Staged hybrid export redirect calls the source-owned initializer
// Historical substitute experiment below is not the current C++ lifetime.
// LEVER: `Rand = Random();`, NOT `Rand.reseed(0)`. This stands in for the
//        generated `??__ERand`, whose whole job is to run the constructor, and
//        the image is one `mov dword ptr [0x9bb568], 0`. `reseed(0)` used to
//        fold to the same store only because reseed was a plain assignment;
//        once reseed became the aliasing xor-swap above it stopped folding and
//        this address went BYTE_EXACT -> MISMATCH. Assigning a default-built
//        temporary says what the initialiser actually does and restores it.
// Random's global object owns startup through its default constructor.
// VC6 currently factors zero initialization and atexit registration into
// generated callees; restoration of the original combined body is pending.

// ORIGINAL: 0x00625720 ??__FRand@@YAXXZ 0x00625720-0x0062572B FILE BYTE_EXACT
// symbol    _$E2
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
// Random's global object owns cleanup through its destructor.

// ORIGINAL: 0x006257E0 ?random_reseed@@YAXK@Z 0x006257E0-0x006257F9 BYTE_EXACT
// LEVER: BYTE_EXACT 8/8, and not one character of THIS body changed. The image inlines `Rand.reseed(new_seed)` here and keeps the xor chain, dropping only the first of reseed's two stores - dead once the second follows. Blocking VC6's xor-swap algebra inside reseed - see the LEVER on 0x00625750 - is what makes those four xors survive inlining here too.
// symbol    ?random_reseed@@YAXI@Z
// size      25 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned reseed wrapper
// TRIED: the "class-wide SEH-frame ceiling" that stood here before
//        2026-08-22. There is no frame at all under `/c /O2 /Gy /GR- /GX`; the
//        `push ebp` it was read off came from the `/Oy-` in the flag set the
//        default measurement happened to report, not from `Random` carrying a
//        real ctor/dtor.
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

// ORIGINAL: 0x00625850 ?random@@YANXZ 0x00625850-0x00625880 BYTE_EXACT
// LEVER: BYTE_EXACT 12/12, from 0/12. `Random::get()` (the double overload) was defined in this .cpp, so this wrapper could only CALL it, and call_diff said so: 1 call against the image's 0. The image folds the whole generator in - imul / add / and / or / fld / fsub, no call at all. Moved in-class into random.h as `MEASURED inline`, marker left here because decomp's reader globs only *.cpp and *.c. 0x006257B0 keeps its own BYTE_EXACT claim: /Ob0 still emits its COMDAT.
// size      48 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned floating generator
double __cdecl random() { return Rand.get(); }


// ---------------------------------------------------------------------------
// Inline lifecycle mappings live beside their definitions in random.h.
// Their explicit compile contexts select this Rand-owning translation unit.
