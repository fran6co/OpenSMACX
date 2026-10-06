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
#include <math.h>
#include <cmath>

// 0x0063456A calls the CRT sqrt with a double argument. Do not substitute
// the compiler's fsqrt intrinsic: it omits that argument-rounding boundary
// and the runtime's floating-point handling.
#pragma function(sqrt)

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
// ORIGINAL: 0x00634430 ?__mi@Vector@@QAEXAAVVector@@AAVVector@@@Z 0x00634430-0x0063447B BYTE_EXACT
// symbol    ??GVector@@QAE?AV0@AAV0@@Z
// size      75 bytes
// prototype void (__thiscall ?__mi@Vector@@QAEXAAVVector@@AAVVector@@@Z)(Vector* this, Vector*, Vector*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
// Historical output-parameter trials omitted the original return-object
// construction flag. Their retired scaffold could not model the destructor;
// that limitation does not constrain the product class or prove this body.
*/
Vector Vector::operator-(Vector &right) {
    Vector result;
    result.x_ = x_ - right.x_;
    result.y_ = y_ - right.y_;
    result.z_ = z_ - right.z_;
    return result;
}

// Component-wise sum, with a separate returned value.
// ORIGINAL: 0x006343E0 ??HVector@@QAE?AV0@AAV0@@Z 0x006343E0-0x0063442B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x006343EF-0x00634417 adds right components to receiver components, rounds three float results before copying them through the separate result pointer, returns that pointer in EAX and ends with ret8 at0x00634428. Complete span agrees with src/recovered/units/006343e0.cpp.", "Vector constructor0x006343C0 and veh_scoot array construction0x0055A197-0x0055A1D9 establish the same12-byte class and destructor; its established arithmetic return convention is independently used by veh_scoot at0x0055A82F.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/006343e0.cpp", "Historical seed spelling sub_6343e0; current source mapping ??HVector@@QAE?AV0@AAV0@@Z; emitted symbol ??HVector@@QAE?AV0@AAV0@@Z."], "reason": "Vector addition returns a separate three-float value; the first stack argument is the hidden class-result pointer, not a source output parameter. Product ownership replaces the seed scaffold with historical provenance retained in Git. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
Vector Vector::operator+(Vector &right) {
    Vector result;
    result.x_ = right.x_ + x_;
    result.y_ = right.y_ + y_;
    result.z_ = right.z_ + z_;
    return result;
}

// Cross product this x right; preserve the original floating-point operand order.
// ORIGINAL: 0x006345E0 ?cross@Vector@@QAE?AV1@AAV1@@Z 0x006345E0-0x00634641 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x006345EF-0x0063462D computes y*right.z-right.y*z, right.x*z-x*right.z, right.y*x-y*right.x using float receiver and argument loads. All input reads precede output copying; EAX receives the result pointer and ret8 ends at0x00634641. Span agrees with src/unrecovered/006345e0.cpp.", "The same Vector constructor/destructor and12-byte array stride are paired in veh_scoot0x0055A197-0x0055A1D9. The neighboring addition and subtraction share its receiver and hidden class-result ABI; the seed synthetic Vec3 output-parameter wrapper is not product design."], "reason": "Vector cross product returns a separate three-float value without mutating either input. Preserve the original multiplication/subtraction operand order and each component rounding; the catalogue output pointer is a hidden class-result slot."}
Vector Vector::cross(Vector &right) {
    Vector result;
    result.x_ = y_ * right.z_ - right.y_ * z_;
    result.y_ = right.x_ * z_ - x_ * right.z_;
    result.z_ = right.y_ * x_ - y_ * right.x_;
    return result;
}

// Return a normalized value without modifying the receiver. The original
// comparison copies the input for either unit length or an unordered length;
// it has no zero-length guard. The original retains the length and reciprocal
// in x87 registers, rounding the sum for sqrt's double argument and the
// resulting components for their float storage.
// ORIGINAL: 0x00634520 ?normalize@Vector@@QAE?AV1@XZ 0x00634520-0x006345DD
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x00634531-0x00634567 computes (x*x+y*y)+z*z with no intermediate float stores, passes a double to sqrt0x00646544, and0x0063456F-0x0063457D tests x87 C3 so equal and unordered lengths copy the input. The other path has no zero guard, multiplies x/y/z by the reciprocal and rounds each result to float. Both returns supply the result pointer in EAX. Complete span agrees with src/unrecovered/00634520.cpp; its synthetic output-parameter body is seed evidence only."], "reason": "The original computes a separate normalized three-float Vector using sqrt and a reciprocal, never mutating its receiver. Normalize is an operation-derived name, not an original debug symbol; the stack result pointer and ret4 indicate a hidden class-value result."}
// Deleted seed provenance: git:b18d3ac864e30cf0876946b9d54717a3d999c691:src/unrecovered/00634520.cpp
Vector Vector::normalize() {
    // Original 0x00634531 loads z, y, x before accumulating the squared length.
    const float z = z_;
    const float y = y_;
    const float x = x_;
    float length = static_cast<float>(sqrt((x * x + y * y) + z * z));
    Vector result;
    if (length != length) {
        return *this;
    }
    if (length == 1.0f) {
        return *this;
    }
    float reciprocal = 1.0f / length;
    result.x_ = static_cast<float>(reciprocal * x_);
    result.y_ = static_cast<float>(reciprocal * y_);
    result.z_ = static_cast<float>(reciprocal * z_);
    return result;
}

/*
Purpose: Add another vector into this one, component by component.
Note:    Returns `*this`, which is what leaves the receiver in EAX at `ret 4`.
         The catalogued `void` return does not reproduce these bytes; this
         does, exactly.
// ORIGINAL: 0x00634480 ?__apl@Vector@@QAEAAVVector@@AAVVector@@@Z 0x00634480-0x006344A1 BYTE_EXACT
// symbol    ??YVector@@QAEAAV0@AAV0@@Z
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
Vector &Vector::operator+=(Vector &right) {
    x_ += right.x_;
    y_ += right.y_;
    z_ += right.z_;
    return *this;
}

/*
Purpose: Subtract another vector from this one, component by component.
// ORIGINAL: 0x006344B0 ?__ami@Vector@@QAEAAVVector@@AAVVector@@@Z 0x006344B0-0x006344D1 BYTE_EXACT
// symbol    ??ZVector@@QAEAAV0@AAV0@@Z
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
Vector &Vector::operator-=(Vector &right) {
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
// ORIGINAL: 0x00634670 ?scale@Vector@@QAEXAAVVector@@M@Z 0x00634670-0x006346B1 BYTE_EXACT
// symbol    ?scale@Vector@@QAE?AV1@M@Z
// size      65 bytes
// prototype void (__thiscall ?scale@Vector@@QAEXAAVVector@@M@Z)(Vector* this, Vector*, float)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
// The historical output-parameter spelling omitted class-result construction.
// The catalogue spelling is retained as provenance, not the emitted signature.
*/
Vector Vector::scale(float scalar) {
    x_ *= scalar;
    y_ *= scalar;
    z_ *= scalar;
    return *this;
}







/*
Purpose: Add two vectors component-wise into an output vector.
// ORIGINAL: 0x00628150 sub_628150 0x00628150-0x00628175 BYTE_EXACT
// symbol    ?vector_add@@YAXPBM0PAM@Z
// size      37 bytes
// prototype 
// callers   17   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void __cdecl vector_add(const float *left, const float *right, float *output) {
    float rx = right[0];
    output[0] = rx + left[0];
    output[1] = left[1] + right[1];
    output[2] = left[2] + right[2];
}

// Historical source note: original x87 subtracts each pair and rounds only at the float store.
// ORIGINAL: 0x00628180 sub_628180 0x00628180-0x006281A5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 00628180 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_00628180_redirect@@YAXPBM0PAM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_subtract@@YAXPBM0PAM@Z
// kind      game
void __cdecl vec3_subtract(const float *left, const float *right,
                             float *result) {
    result[0] = left[0] - right[0];
    result[1] = left[1] - right[1];
    result[2] = left[2] - right[2];
}

// Historical source note: bind each source component before its multiply; this preserves
// source-before-store behavior when result aliases source and retains the original operand scheduling.
// ORIGINAL: 0x006281B0 sub_6281b0 0x006281B0-0x006281D5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 006281B0 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_006281b0_redirect@@YAXPBMMPAM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_scale@@YAXPBMMPAM@Z
// kind      game
void __cdecl vec3_scale(const float *source, float scale,
                         float *result) {
    const float x = source[0];
    result[0] = x * scale;
    const float y = source[1];
    result[1] = y * scale;
    const float z = source[2];
    result[2] = z * scale;
}

// Historical body note: the original stores y/z scratch slots before reloading them;
// the LEVER annotation below retains the measured register/spill exploration.
// ORIGINAL: 0x006281E0 sub_6281e0 0x006281E0-0x00628217
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 006281E0 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_006281e0_redirect@@YAMPBM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_length@@YAMPBM@Z
// kind      game
// LEVER: NAMED LOCALS FOR y AND z, rather than `vector[1]`/`vector[2]` read twice each: the image loads both as raw ints into ecx/edx and writes them to a two-slot scratch on the stack (reusing the argument's own slot, since __cdecl leaves it dead once the pointer is in eax) before reloading each from memory for its square - a local-variable spill, not a re-read through the pointer. `vector[1]`/`vector[2]` written out scored 0/21; these two locals reach 7/21 (0.818), the ceiling found - the two scratch SLOTS still land swapped ([esp]/[esp+8]) no matter which of y/z is declared or summed first (measured, six spellings).
float __cdecl vec3_length(const float *vector) {
    const float y = vector[1];
    const float z = vector[2];
    const long double sum = static_cast<long double>(y) * y
                          + static_cast<long double>(z) * z
                          + static_cast<long double>(vector[0]) * vector[0];
    return static_cast<float>(std::sqrt(sum));
}

// ORIGINAL: 0x00634650 sub_634650 0x00634650-0x0063466B BYTE_EXACT
// symbol    ?dot@Vector@@QBEMABV1@@Z
// kind      game
// The original accumulates z, then y, then x without intermediate stores.
// dot and const qualification describe observed behavior, not debug symbols.
float Vector::dot(const Vector &right) const {
    const float rz = right.z_;
    const float ry = right.y_;
    const float rx = right.x_;
    return (rz * z_ + ry * y_) + rx * x_;
}

// ORIGINAL: 0x006344E0 sub_6344e0 0x006344E0-0x00634513 BYTE_EXACT
// symbol    ??8Vector@@QBE_NABV0@@Z
// kind      game
// Original x87 comparisons test C3 only: each component may be equal or
// unordered, but any ordered mismatch fails. Preserve x/y/z short-circuiting.
// Explicit self-comparisons retain that relation with other C++ compilers too.
bool Vector::operator==(const Vector &right) const {
    if (!(x_ == right.x_ || x_ != x_ || right.x_ != right.x_)) goto unequal;
    if (!(y_ == right.y_ || y_ != y_ || right.y_ != right.y_)) goto unequal;
    if (z_ != z_ || right.z_ != right.z_) return true;
    if (z_ == right.z_) return true;
unequal:
    return false;
}
