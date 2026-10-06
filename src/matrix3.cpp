#include <cstring>
#include "matrix3.h"

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
// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006346C0 sub_6346c0 0x006346C0-0x006346E4 BYTE_EXACT
// symbol    ?sub_6346c0@FloatMatrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
FloatMatrix3 *FloatMatrix3::sub_6346c0() {
    values[0][0] = 1.0f; values[0][1] = 0.0f; values[0][2] = 0.0f;
    values[1][0] = 0.0f; values[1][1] = 1.0f; values[1][2] = 0.0f;
    values[2][0] = 0.0f; values[2][1] = 0.0f; values[2][2] = 1.0f;
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
// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006346F0 sub_6346f0 0x006346F0-0x00634712 BYTE_EXACT
// symbol    ?sub_6346f0@FloatMatrix3@@QAEXXZ
// size      34 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_6346f0() {
    values[0][0] = 1.0f; values[0][1] = 0.0f; values[0][2] = 0.0f;
    values[1][0] = 0.0f; values[1][1] = 1.0f; values[1][2] = 0.0f;
    values[2][0] = 0.0f; values[2][1] = 0.0f; values[2][2] = 1.0f;
}

/*
Purpose: Add another matrix into this one through an identity scratch
         buffer, then copy the sum out to a destination.

         The identity fill of the scratch is fully overwritten by the sum
         - every one of its nine slots is stored before any read - but the
         original pays for it anyway, so it is kept rather than tidied
         away.

// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634720 sub_634720 0x00634720-0x006347BE FILE
// TRIED: nothing yet; the nested row/column countdown pair is the original's own shape.
// symbol   ?sub_634720@FloatMatrix3@@QAEXPAU1@PBU1@@Z
// size      158 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_634720(FloatMatrix3 *destination,
                              const FloatMatrix3 *other) {
    float scratch[3][3] = {{1.0f, 0.0f, 0.0f},
                           {0.0f, 1.0f, 0.0f},
                           {0.0f, 0.0f, 1.0f}};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            scratch[row][column] = values[row][column] +
                                   other->values[row][column];
        }
    }
    std::memcpy(destination->values, scratch, sizeof(scratch));
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

// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006347C0 sub_6347c0 0x006347C0-0x006347F0
// symbol    ?sub_6347c0@FloatMatrix3@@QAEXPBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_6347c0(const FloatMatrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            values[row][column] = other->values[row][column] +
                                  values[row][column];
        }
    }
}

/*
Purpose: Replace this 3x3 matrix with its product with another: a
         triple-nested row/column walk into a scratch buffer, copied back
         only when every cell is settled.

// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006347F0 sub_6347f0 0x006347F0-0x006348BC FILE
// symbol   ?sub_6347f0@FloatMatrix3@@QAEXPBU1@@Z
// size      204 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_6347f0(const FloatMatrix3 *other) {
    float scratch[3][3] = {{1.0f, 0.0f, 0.0f},
                           {0.0f, 1.0f, 0.0f},
                           {0.0f, 0.0f, 1.0f}};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            float sum = 0.0f;
            for (int depth = 0; depth < 3; ++depth) {
                sum = other->values[depth][column] * values[row][depth] + sum;
            }
            scratch[row][column] = sum;
        }
    }
    std::memcpy(values, scratch, sizeof(scratch));
}

/*
Purpose: Scale a 3x3 float matrix by a scalar.

             mov edi,3 / mov esi,3 / fld [esp+0xc] / fmul [edx] / add edx,4
             dec esi / fstp [edx-4] / jne inner / dec edi / jne outer / ret 4

         The scalar sits at [esp+0xc] because ESI and EDI were pushed first;
         before those pushes it is the single argument at [esp+4]. It is
         re-loaded inside the inner loop, which changes nothing - it is the
         same float32 every time.

// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006348C0 sub_6348c0 0x006348C0-0x006348E7
// symbol    ?sub_6348c0@FloatMatrix3@@QAEXM@Z
// size      39 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_6348c0(float scale) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            values[row][column] = scale * values[row][column];
        }
    }
}

/*
Purpose: Subtract another 3x3 float matrix from this one.

             fld [edx] / fsub [esi+edx] / ... / ret 4

         The same nine-element walk as 006347C0, but loading THIS first: the
         result is this - other, and subtraction does not commute.

// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x006348F0 sub_6348f0 0x006348F0-0x00634920
// symbol    ?sub_6348f0@FloatMatrix3@@QAEXPBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FloatMatrix3::sub_6348f0(const FloatMatrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            values[row][column] = values[row][column] -
                                  other->values[row][column];
        }
    }
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
// RECOVERY_ROLE: SCAFFOLD FloatMatrix3 is a chosen numerical representation; original class, complete extent, translation unit, callers, lifecycle and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634B70 sub_634b70 0x00634B70-0x00634B94 BYTE_EXACT
// symbol    ?sub_634b70@FloatMatrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
FloatMatrix3 *FloatMatrix3::sub_634b70() {
    values[0][0] = 1.0f; values[0][1] = 0.0f; values[0][2] = 0.0f;
    values[1][0] = 0.0f; values[1][1] = 1.0f; values[1][2] = 0.0f;
    values[2][0] = 0.0f; values[2][1] = 0.0f; values[2][2] = 1.0f;
    return this;
}
