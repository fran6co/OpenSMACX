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
#include "fx.h"
#include "vtable_shim.h"
#include "hypothesis_layouts.h"

// The FX bank's element is the hypothesis Effect (hypothesis_layouts.h): a
// Wave-derived type adding nothing - stride equals sizeof(Wave) at 0x6C. Its
// construction runs Wave::Wave and installs the type's own vftable over the
// Wave's (0x0066B7A4 in the image); its teardown is the five-byte jump into
// ~Wave that wave.cpp documents. Both are what the compiler emits for the
// placement construction and destruction of a plain Effect.

void __fastcall EffectElementTeardown(void *self) {
    reinterpret_cast<Effect *>(self)->~Effect();
}

void __fastcall EffectElementCtor(void *self) {
    new (self) Effect();
}

/*
Purpose: Build the effect bank: the mirror image of the destructor below, one
         call handing the whole 0x61-element walk to the CRT vector
         constructor iterator with the bank itself as the array base. The
         destructor argument rides along for exception unwind, which cannot
         happen here but is passed faithfully. The 0x6C stride and 0x61 count
         are the same pair the destructor uses, read off the two `push`
         immediates at 0x004482AD and 0x004482AF - note the original pushes
         them count-then-size, so they arrive at the callee in the opposite
         order from the way they are written here.
// ORIGINAL: 0x004482A0 ??0FX@@QAE@XZ 0x004482A0-0x004482BB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ??0FX@@QAE@XZ)(FX* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006457C2
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
*/
FX::FX() {
    VectorCtorIterator(this, 0x6C, 0x61, EffectElementCtor,
                       EffectElementTeardown);
}


/*
Purpose: Destroy the effect bank: hand the whole 0x61-element walk to the CRT
         vector iterator with the bank itself as the array base. Everything
         else is the per-element destructor's business.
// ORIGINAL: 0x00448290 ??1FX@@QAE@XZ 0x00448290-0x004482A0 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??1FX@@QAE@XZ)(FX* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FX::~FX() {
    VectorDtorIterator(this, 0x6C, 0x61, EffectElementTeardown);
}


/*
// ORIGINAL: 0x00446A90 ?stop@FX@@QAEXH@Z 0x00446A90-0x00446AA9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?stop@FX@@QAEXH@Z)(FX* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AA2
Status: Complete
*/
void FX::stop(int a1) {
    // The elements are Waves (0x6C = sizeof(Wave), 97 of them) - this is
    // the element's own stop() virtual, which is what this method is named.
    reinterpret_cast<Wave *>(effects_)[a1].stop();
}

/*
// ORIGINAL: 0x00446AB0 ?release@FX@@QAEXH@Z 0x00446AB0-0x00446AC9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?release@FX@@QAEXH@Z)(FX* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AC2
Status: Complete
*/
void FX::release(int a1) {
    reinterpret_cast<Wave *>(effects_)[a1].release();
}

/*
// ORIGINAL: 0x00446AD0 ?fade@FX@@QAEXH@Z 0x00446AD0-0x00446AE9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?fade@FX@@QAEXH@Z)(FX* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AE2
Status: Complete
*/
void FX::fade(int a1) {
    // The element's own fade() virtual - what this method is named after.
    reinterpret_cast<Wave *>(effects_)[a1].fade();
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
FX g_FX;  // 0x00749CF8
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
