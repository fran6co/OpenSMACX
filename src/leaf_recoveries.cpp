/*
 * Hand-recovered leaf functions that no generator should be taught.
 *
 * tools/generate_field_accessors.py earns its keep on SHAPES - a field read, a
 * store sequence, a clamp - where the same template covers several functions
 * and the tests pin the template rather than each body. What is left of the
 * leaf-testable closure is not that. These are one-offs: a float vector
 * subtract, a dot product, a round-down-to-a-multiple. Teaching a generator a
 * template with exactly one instance means the template and its only user are
 * the same thing, and the "generator" is then just a less readable way of
 * writing the body once.
 *
 * So these are written out, each with the disassembly it came from and its own
 * argument for why the C++ is the same function. They keep the address-derived
 * naming the generated families use, because the reason for it has not
 * changed: these bodies have no mangled name, so inventing one would be
 * inventing a fact about the program.
 *
 * They live in their own file rather than in src/field_accessors.cpp so that
 * regenerating that file stays a safe operation - a hand-written body in a
 * generated file is lost the first time someone regenerates it.
 */

#include "stdafx.h"
#include "leaf_recoveries.h"

/*
Purpose: Subtract two three-component float vectors into a third.

             mov eax, [esp+4] / mov ecx, [esp+8] / mov edx, [esp+0xc]
             fld [eax]   / fsub [ecx]   / fstp [edx]
             fld [eax+4] / fsub [ecx+4] / fstp [edx+4]
             fld [eax+8] / fsub [ecx+8] / fstp [edx+8]
             ret

         Three arguments read from [esp+4..0xc] and a bare `ret`: __cdecl, the
         caller cleans. Declaring exactly the slots the body reads is safe for
         __cdecl even if the real function takes more, because the callee never
         has to know the count.

         `flds / fsubs / fstps` is what the compiler emits for `o[i] = a[i] -
         b[i]` on this target - verified by objdump, not assumed. That matters
         more than it looks: the subtraction happens in x87 EXTENDED precision
         and is rounded to float only by the store, so a version that computed
         in float throughout could round differently.

Original Offset: 00628180
Return Value: n/a
Status: Complete
*/
void __cdecl leaf_00628180_redirect(const float *left, const float *right,
                                    float *result) {
    result[0] = left[0] - right[0];
    result[1] = left[1] - right[1];
    result[2] = left[2] - right[2];
}

/*
Purpose: Scale a three-component float vector by a scalar.

             mov eax, [esp+4] / mov ecx, [esp+0xc]
             fld [eax]   / fmul [esp+8] / fstp [ecx]
             fld [eax+4] / fmul [esp+8] / fstp [ecx+4]
             fld [eax+8] / fmul [esp+8] / fstp [ecx+8]
             ret

         The scalar is a float passed BY VALUE in the middle slot - [esp+8] is
         re-read as a float32 by each `fmul`, not held in a register. The
         compiler instead loads it once into st and multiplies against that;
         the value is the same extended-precision number either way, since it
         came from the same float32.

Original Offset: 006281B0
Return Value: n/a
Status: Complete
*/
void __cdecl leaf_006281b0_redirect(const float *source, float scale,
                                    float *result) {
    result[0] = source[0] * scale;
    result[1] = source[1] * scale;
    result[2] = source[2] * scale;
}

/*
Purpose: Dot product of `this` with a three-component float vector.

             mov eax, [esp+4]
             fld [eax+8] / fmul [ecx+8]
             fld [eax+4] / fmul [ecx+4] / faddp st(1)
             fld [eax]   / fmul [ecx]   / faddp st(1)
             ret 4

         ECX is `this`, one stack argument, `ret 4` - __thiscall. The ORDER is
         part of the function: components 2, then 1, then 0. Floating-point
         addition is not associative, so summing them the other way round is a
         different function that agrees on most inputs.

         The C++ below compiles to this instruction sequence exactly, `ret 4`
         included - checked with objdump rather than assumed.

Original Offset: 00634650
Return Value: the dot product, in ST(0)
Status: Complete
*/
float __fastcall leaf_00634650_redirect(void *self, void *,
                                        const float *other) {
    const float *mine = static_cast<const float *>(self);
    return other[2] * mine[2] + other[1] * mine[1] + other[0] * mine[0];
}

/*
Purpose: Round an integer DOWN to a multiple of another, toward zero.

             mov eax, [ebp+8] / mov ecx, [ebp+0xc]
             cdq / idiv ecx / imul eax, ecx
             ret

         `x / y * y` with signed division, so it truncates toward zero rather
         than toward negative infinity: -7 with y=3 gives -6, not -9.

         The compiler rewrites this as `x - x % y`, which is the same value for
         every input and traps in the same two places, because both spellings
         go through one `idiv`: y == 0 and INT_MIN / -1 raise #DE here exactly
         as they do in the original.

Original Offset: 00559210
Return Value: the largest multiple of `step` no further from zero than `value`
Status: Complete
*/
int __cdecl leaf_00559210_redirect(int value, int step) {
    return value / step * step;
}
