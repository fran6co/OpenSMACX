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
#include "matrix3.h"
#include <math.h>
#include <cmath>
#include <cstring>

// 0x0063456A calls the CRT sqrt with a double argument. Do not substitute
// the compiler's fsqrt intrinsic: it omits that argument-rounding boundary
// and the runtime's floating-point handling.

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
// Out of line, not a header inline: veh_scoot calls 0x006343C0 directly for
// its singleton `destination` (0x0055A1BB) as well as handing it to the
// array constructor, and the constructor's address lies in this unit's run
// (0x006343C0, just before close at 0x006343D0).
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

// Component-wise sum, with a separate returned value.
// ORIGINAL: 0x006343E0 ??HVector@@QAE?AV0@AAV0@@Z 0x006343E0-0x0063442B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x006343EF-0x00634417 adds right components to receiver components, rounds three float results before copying them through the separate result pointer, returns that pointer in EAX and ends with ret8 at0x00634428. Complete span agrees with src/recovered/units/006343e0.cpp.", "Vector constructor0x006343C0 and veh_scoot array construction0x0055A197-0x0055A1D9 establish the same12-byte class and destructor; its established arithmetic return convention is independently used by veh_scoot at0x0055A82F.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/recovered/units/006343e0.cpp", "Historical seed spelling sub_6343e0; current source mapping ??HVector@@QAE?AV0@AAV0@@Z; emitted symbol ??HVector@@QAE?AV0@AAV0@@Z."], "reason": "Vector addition returns a separate three-float value; the first stack argument is the hidden class-result pointer, not a source output parameter. Product ownership replaces the seed scaffold with historical provenance retained in Git. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
Vector Vector::operator+(Vector &right) {
    Vector result;
    result.x_ = right.x_ + x_;
    result.y_ = right.y_ + y_;
    result.z_ = right.z_ + z_;
    return result;
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

// Return a normalized value without modifying the receiver. The original
// comparison copies the input for either unit length or an unordered length;
// it has no zero-length guard. The original retains the length and reciprocal
// in x87 registers, rounding the sum for sqrt's double argument and the
// resulting components for their float storage.
// ORIGINAL: 0x00634520 ?normalize@Vector@@QAE?AV1@XZ 0x00634520-0x006345DD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x00634531-0x00634567 computes (x*x+y*y)+z*z with no intermediate float stores, passes a double to sqrt0x00646544, and0x0063456F-0x0063457D tests x87 C3 so equal and unordered lengths copy the input. The other path has no zero guard, multiplies x/y/z by the reciprocal and rounds each result to float. Both returns supply the result pointer in EAX. Complete span agrees with src/unrecovered/00634520.cpp; its synthetic output-parameter body is seed evidence only."], "reason": "The original computes a separate normalized three-float Vector using sqrt and a reciprocal, never mutating its receiver. Normalize is an operation-derived name, not an original debug symbol; the stack result pointer and ret4 indicate a hidden class-value result."}
// Deleted seed provenance: git:da8a4b7fe3bc3271a694c1c5144e3a52d24ad845:src/unrecovered/00634520.cpp
Vector Vector::normalize() {
    Vector result;
    float sq = x_ * x_ + y_ * y_ + z_ * z_;
    float length = sqrt(sq);
    if (length == 1.0f) {
        return *this;
    }
    float reciprocal = 1.0f / length;
    result.x_ = reciprocal * x_;
    result.y_ = reciprocal * y_;
    result.z_ = reciprocal * z_;
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

// The 3x3 numerical members follow in the same original object: the image
// places them directly after the Vector code (0x006346C0..0x00634B94), nothing
// references them, and the linker kept them only because this member was
// pulled in for Vector.

// Chosen mathematical source view for the nine-float receiver prefix used by
// this address family. This does not declare the original class or its full
// object size; application construction and call sites remain unknown.
/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / mov edx, 0x3f800000 / xor ecx, ecx / mov dword ptr [eax], edx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], edx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006346C0 sub_6346c0 0x006346C0-0x006346E4 BYTE_EXACT
// symbol    ?identity@Matrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
Matrix3 *Matrix3::identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
    return this;
}

/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov edx, 0x3f800000 / xor eax, eax / mov dword ptr [ecx], edx / mov dword ptr [ecx + 4], eax / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 0x10], edx / mov dword ptr [ecx + 0x14], eax / mov dword ptr [ecx + 0x18], eax / mov dword ptr [ecx + 0x1c], eax / mov dword ptr [ecx + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006346F0 sub_6346f0 0x006346F0-0x00634712 BYTE_EXACT
// symbol    ?reset_identity@Matrix3@@QAEXXZ
// size      34 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::reset_identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
}

/*
Purpose: Add another matrix into this one through an identity-initialized
         Matrix3 result, then publish the completed sum as a value.

         The identity fill of the result is fully overwritten by the sum
         - every one of its nine slots is stored before any read - but the
         original pays for it anyway, so it is kept rather than tidied
         away.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634720 sub_634720 0x00634720-0x006347BE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x006347A3/0x006347B2/0x006347BB: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x006347A3/0x006347B2/0x006347BB: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// TRIED: nothing yet; the nested row/column countdown pair is the original's own shape.
// symbol   ?add@Matrix3@@QAE?AU1@PBU1@@Z
// size      158 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::add(const Matrix3 *other) {
    Matrix3 result = {{1.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 1.0f}};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            result.cells[row * 3 + column] =
                other->cells[row * 3 + column] + cells[row * 3 + column];
        }
    }
    return result;
}

/*
Purpose: Add another 3x3 float matrix into this one, element by element.

             mov edx,eax / mov ebx,3 / mov esi,ecx / mov edi,3 / sub esi,eax
             fld [esi+edx] / fadd [edx] / add edx,4 / dec edi
             fstp [edx-4] / jne inner / dec ebx / jne outer / ret 4

         ESI is the BYTE DELTA between the two matrices, computed once, and
         EDX walks forward without being reset between outer iterations - so
         the two nested threes are nine consecutive floats, not three rows of
         three restarted each time.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006347C0 sub_6347c0 0x006347C0-0x006347F0 BYTE_EXACT
// symbol    ?add_assign@Matrix3@@QAEAAU1@PBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: this matrix (inferred from the pinned receiver remaining in EAX)
Status: Complete
*/
Matrix3 &Matrix3::add_assign(const Matrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = other->cells[row * 3 + column] +
                                     cells[row * 3 + column];
        }
    }
    return *this;
}

/*
Purpose: Replace this 3x3 matrix with its product with another: a
         triple-nested row/column walk into a scratch buffer, copied back
         only when every cell is settled.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006347F0 sub_6347f0 0x006347F0-0x006348BC FILE BYTE_EXACT
// symbol    ?multiply_assign@Matrix3@@QAEAAU1@PBU1@@Z
// size      204 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: this matrix (inferred from the pinned receiver remaining in EAX)
Status: Complete
*/
Matrix3 &Matrix3::multiply_assign(const Matrix3 *other) {
    float scratch[9] = {1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            scratch[row * 3 + column] = 0.0f;
            for (int depth = 0; depth < 3; ++depth) {
                scratch[row * 3 + column] =
                    other->cells[depth * 3 + column] *
                    cells[row * 3 + depth] + scratch[row * 3 + column];
            }
        }
    }
    // WORKAROUND(type): the image ends with one structure assignment
    // (`mov ecx, 9; rep movsd`, 0x006348A5); the scratch is a Matrix3 local
    // in the original, written here as its nine floats.
    *this = *reinterpret_cast<Matrix3 *>(scratch);
    return *this;
}

/*
Purpose: Scale a 3x3 float matrix by a scalar.

             mov edi,3 / mov esi,3 / fld [esp+0xc] / fmul [edx] / add edx,4
             dec esi / fstp [edx-4] / jne inner / dec edi / jne outer / ret 4

         The scalar sits at [esp+0xc] because ESI and EDI were pushed first;
         before those pushes it is the single argument at [esp+4]. It is
         re-loaded inside the inner loop, which changes nothing - it is the
         same float32 every time.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006348C0 sub_6348c0 0x006348C0-0x006348E7 BYTE_EXACT
// symbol    ?scale_assign@Matrix3@@QAEAAU1@M@Z
// size      39 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: this matrix (inferred from the pinned receiver remaining in EAX)
Status: Complete
*/
Matrix3 &Matrix3::scale_assign(float scale) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = scale * cells[row * 3 + column];
        }
    }
    return *this;
}

/*
Purpose: Subtract another 3x3 float matrix from this one.

             fld [edx] / fsub [esi+edx] / ... / ret 4

         The same nine-element walk as 006347C0, but loading THIS first: the
         result is this - other, and subtraction does not commute.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006348F0 sub_6348f0 0x006348F0-0x00634920 BYTE_EXACT
// symbol    ?subtract_assign@Matrix3@@QAEAAU1@PBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: this matrix (inferred from the pinned receiver remaining in EAX)
Status: Complete
*/
Matrix3 &Matrix3::subtract_assign(const Matrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = cells[row * 3 + column] -
                                      other->cells[row * 3 + column];
        }
    }
    return *this;
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634920 sub_634920 0x00634920-0x00634975 BYTE_EXACT
// Historical source symbol hypothesis: ?equals@Mat3@@QAE_NPBM@Z
// symbol    ?equals@Matrix3@@QAE_NPBM@Z
// LEVER: two indexed reads, not a hoisted difference. The image keeps `this`
// and `other` apart and re-derives their difference inside the outer
// loop (`sub edi, ebp` at 0x634939, one per row); spelling that
// difference out as a local lets /O2 prove it loop-invariant and hoist
// it past the loop it belongs to, which is 69 bytes against 85.
Return Value: true when all cell pairs compare equal under the native predicate.
The pinned VC6/x87 lowering accepts unordered NaN pairs; ordinary host C++ !=
rejects them, so host and native NaN behavior is characterized separately.
Status: Complete
*/
bool Matrix3::equals(const float *other) {
    const float *mine = cells;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // The pinned VC6 FCOMP lowering accepts unordered NaN pairs with
            // masked exceptions. Standard host C++ != rejects them instead.
            // Signaling-NaN, unmasked exceptions and x87 status remain unknown.
            if (mine[row * 3 + col] != other[row * 3 + col]) {
                return false;
            }
        }
    }
    return true;
}

/*
// Structural debt: the chosen Matrix3 source view covers the observed
// nine-float prefix only. Original class/extent/lifetime/callers/TU and x87
// precision are not established. The pointer-to-int scratch delta below the prior body was
// a source-form experiment; it has been replaced by equivalent bounded index
// traversal, not promoted as an ABI or matching claim.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634980 sub_634980 0x00634980-0x00634A50 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634A35/0x00634A44/0x00634A4D: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634A35/0x00634A44/0x00634A4D: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// Historical source symbol hypothesis: ?multiply@Mat3@@QAEXPAM0@Z
// symbol    ?multiply@Matrix3@@QAE?AU1@PBU1@@Z
// TRIED: `extern "C" int __stdcall sub_634980(int a1, int a2)` per the given def head - disasm reads *ecx directly as a float* with no matching stack slot (a 3x3 matrix "this"), so the receiver is a __thiscall member, not a 2-arg __stdcall free function; changed to a `Mat3::multiply` member. `dst`/`a` tracked as independent incrementing pointers gives a MISMATCH at the prologue (#4, 87% similarity); an `int off = dst - a` delta recomputed each outer iteration (matching the original's spill of that delta to `[esp+0x14]` and its `add esi, ebx` re-derivation) gets to 89.5% similarity, 7 edits, all register-allocation choices (which callee-saved reg holds which pointer) rather than control-flow shape. Pointer-to-int delta form is retired here; the observation remains historical, not an implementation requirement.
// size      208 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::multiply(const Matrix3 *other) {
    Matrix3 result = {{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    }};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            result.cells[row * 3 + column] = 0.0f;
            for (int depth = 0; depth < 3; ++depth) {
                result.cells[row * 3 + column] =
                    other->cells[depth * 3 + column] *
                    cells[row * 3 + depth] + result.cells[row * 3 + column];
            }
        }
    }
    return result;
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634A50 sub_634a50 0x00634A50-0x00634AC9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned0x00634A50..0x00634AC9 stages three float results then copies them to the first stack argument, retains that pointer in EAX and ends ret8; the second stack argument supplies the three input floats.", "Matched Vector::operator+0x006343E0 independently establishes the same hidden-result pointer/copy/EAX convention. This does not establish that transform returns the existing Vector class or its destructor lifetime.", "No in-tree source callers require migration; preserve original sub_634a50 identity and full121-byte span."], "reason": "Model the observed three-float hidden class result instead of an explicit output parameter. Matrix3TransformResult is a bounded mathematical value view; original type name, complete receiver ownership, construction/lifetime details and native x87 matching remain unresolved."}
// Historical source symbol hypothesis: ?transform@Matrix3@@QAEXPAMPBM@Z
// symbol    ?transform@Matrix3@@QAE?AUMatrix3TransformResult@@PBM@Z
// size      121 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// The pin supports a hidden three-float value result, not this type's
// historical name or ownership.
Matrix3TransformResult Matrix3::transform(const float *in) {
    Matrix3TransformResult result;
    result.x_ = cells[0] * in[0] + cells[1] * in[1] + cells[2] * in[2];
    result.y_ = cells[3] * in[0] + cells[4] * in[1] + cells[5] * in[2];
    result.z_ = cells[6] * in[0] + cells[7] * in[1] + cells[8] * in[2];
    return result;
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted. Replaced the old
// pointer-to-int scratch delta with flat indexing for host-width safety.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634AD0 sub_634ad0 0x00634AD0-0x00634B67 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634B4E/0x00634B5D/0x00634B64: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634B4E/0x00634B5D/0x00634B64: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// Historical source symbol hypothesis: ?scale_into@Mat3x3@@QAEXPAMM@Z
// symbol    ?scaled@Matrix3@@QAE?AU1@M@Z
// TRIED: extern "C" __stdcall(int,int) contract head - disasm reads ecx as an implicit receiver (thiscall) into a 3x3 float matrix, not a stack int; rewrote as Mat3x3::scale_into(float*,float). MISMATCH #1 push/lea, prologue layout only, structure otherwise matches Ghidra's nested-loop scale+copy. The old pointer-to-int delta is retired; preserve this as historical source-form evidence.
// size      151 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::scaled(float scale) {
    Matrix3 result = {{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    }};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            const int index = row * 3 + column;
            result.cells[index] = scale * cells[index];
        }
    }
    return result;
}

/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / mov edx, 0x3f800000 / xor ecx, ecx / mov dword ptr [eax], edx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], edx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634B70 sub_634b70 0x00634B70-0x00634B94 BYTE_EXACT
// symbol    ?write_identity@Matrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
Matrix3 *Matrix3::write_identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
    return this;
}
