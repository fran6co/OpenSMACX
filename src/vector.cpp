/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "vector.h"

#include <cstring>
#include <new>

/*
Purpose: Initialize a zero vector.
// ORIGINAL: 0x006343C0 ??0Vector@@QAE@XZ 0x006343C0-0x006343CD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ??0Vector@@QAE@XZ)(Vector* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Vector::Vector() {
    x_ = 0.0f;
    y_ = 0.0f;
    z_ = 0.0f;
}

/*
Purpose: Clear all vector components.
// ORIGINAL: 0x006343D0 ?close@Vector@@QAEXXZ 0x006343D0-0x006343DB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?close@Vector@@QAEXXZ)(Vector* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Vector::close() {
    x_ = 0.0f;
    y_ = 0.0f;
    z_ = 0.0f;
}

/*
Purpose: Subtract another vector into a separate output vector.
Note:    `output` is the hidden return-object slot of `Vector operator-`, not
         a declared parameter - see the signature note in vector.h. The
         difference is computed into a temporary first because the original
         does, and because the caller may pass the same object twice.
// ORIGINAL: 0x00634430 ?__mi@Vector@@QAEXAAVVector@@AAVVector@@@Z 0x00634430-0x0063447B
// symbol    ?__mi@Vector@@QAEXAAV1@0@Z
// size      75 bytes
// prototype void (__thiscall ?__mi@Vector@@QAEXAAVVector@@AAVVector@@@Z)(Vector* this, Vector*, Vector*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
// TRIED: this catalogued `void __mi(Vector&, Vector&)` signature, tried
//            as-is - MISMATCH from instruction 0 (`sub esp, 0x10` against
//            this tree's `sub esp, 0xc`). The image's extra 4 bytes and the
//            `mov dword ptr [esp], 0` right after are the /GX return-object
//            constructed flag for a BY-VALUE `Vector operator-` return - see
//            "why two are adopted and two are not" in vector.h, which names
//            this a REAL CEILING in `tools/emit_translation_unit.py` (never
//            declares the destructor a rebuilt verification unit would need
//            to reproduce the flag), not a source-form question for this body.
*/
void Vector::__mi(Vector &output, Vector &right) {
    Vector result;
    result.x_ = x_ - right.x_;
    result.y_ = y_ - right.y_;
    result.z_ = z_ - right.z_;
    output = result;
}

/*
Purpose: Add another vector into this one, component by component.
Note:    Returns `*this`, which is what leaves the receiver in EAX at `ret 4`.
         The catalogued `void` return does not reproduce these bytes; this
         does, exactly.
// ORIGINAL: 0x00634480 ?__apl@Vector@@QAEAAVVector@@AAVVector@@@Z 0x00634480-0x006344A1 BYTE_EXACT
// symbol    ?__apl@Vector@@QAEAAV1@AAV1@@Z
// CORRECTED from ?__apl@Vector@@QAEXAAVVector@@@Z
//   opens `mov eax, ecx` and leaves `this` in EAX at `ret 4`; with a
//   `Vector &` return and `return *this` the body is byte-exact, with
//   the catalogued `void` it is a different 29-byte sequence
// size      33 bytes
// prototype Vector* (__thiscall ?__apl@Vector@@QAEAAVVector@@AAVVector@@@Z)(Vector* this, Vector*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: this vector
Status: Complete
*/
Vector &Vector::__apl(Vector &right) {
    x_ += right.x_;
    y_ += right.y_;
    z_ += right.z_;
    return *this;
}

/*
Purpose: Subtract another vector from this one, component by component.
// ORIGINAL: 0x006344B0 ?__ami@Vector@@QAEAAVVector@@AAVVector@@@Z 0x006344B0-0x006344D1 BYTE_EXACT
// symbol    ?__ami@Vector@@QAEAAV1@AAV1@@Z
// CORRECTED from ?__ami@Vector@@QAEXAAVVector@@@Z
//   the same shape with fsub for fadd, and the same two verdicts
// size      33 bytes
// prototype Vector* (__thiscall ?__ami@Vector@@QAEAAVVector@@AAVVector@@@Z)(Vector* this, Vector*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: this vector
Status: Complete
*/
Vector &Vector::__ami(Vector &right) {
    x_ -= right.x_;
    y_ -= right.y_;
    z_ -= right.z_;
    return *this;
}

/*
Purpose: Scale this vector and copy the updated components into an output
         vector.
Note:    The original scales `this` IN PLACE and then copies it out; `output`
         is the hidden return slot of `Vector scale(float)`. Both effects are
         load-bearing - a caller that ignores the result still sees `this`
         scaled.
// ORIGINAL: 0x00634670 ?scale@Vector@@QAEXAAVVector@@M@Z 0x00634670-0x006346B1
// symbol    ?scale@Vector@@QAEXAAV1@M@Z
// size      65 bytes
// prototype void (__thiscall ?scale@Vector@@QAEXAAVVector@@M@Z)(Vector* this, Vector*, float)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
// TRIED: this catalogued `void scale(Vector&, float)` signature, tried
//            as-is - MISMATCH from instruction 0 (image opens `push ecx`
//            with no frame; this tree's `push ebp; mov ebp, esp`). Same
//            /GX return-object-flag ceiling as __mi above: `scale` is really
//            `Vector scale(float)` returning by value, and
//            `tools/emit_translation_unit.py` cannot rebuild a verification
//            unit that declares the destructor the flag depends on. See
//            "why two are adopted and two are not" in vector.h.
*/
void Vector::scale(Vector &output, float scalar) {
    x_ *= scalar;
    y_ *= scalar;
    z_ *= scalar;
    output = *this;
}







/*
Purpose: Add two vectors component-wise into an output vector.
// ORIGINAL: 0x00628150 sub_628150 0x00628150-0x00628175 BYTE_EXACT
// symbol    ?vector_add@@YAXPAVVector@@00@Z
// size      37 bytes
// prototype 
// callers   17   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void __cdecl vector_add(Vector *left, Vector *right, Vector *output) {
    float rx = right->x_;
    output->x_ = rx + left->x_;
    output->y_ = left->y_ + right->y_;
    output->z_ = left->z_ + right->z_;
}
