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

/*
Purpose: Follow a two-link chain, or return zero when the first link is unset.

             mov eax, [ecx+8] / test eax,eax / je zero
             mov eax, [ecx+0xc] / mov eax, [eax+8] / mov eax, [eax+4] / ret
       zero: xor eax,eax / ret

         The guard reads field 8 but the chase starts from field 0xc - two
         DIFFERENT fields, which is easy to lose when transcribing and produces
         a body that works whenever the two happen to agree.

Original Offset: 005E3630
Return Value: the dword two links away, or 0
Status: Complete
*/
uint32_t __fastcall leaf_005e3630_redirect(void *self, void *) {
    const uint8_t *const bytes = static_cast<const uint8_t *>(self);
    if (*reinterpret_cast<const uint32_t *>(bytes + 0x8) == 0) {
        return 0;
    }
    const uint8_t *const first = *reinterpret_cast<const uint8_t *const *>(
        bytes + 0xc);
    const uint8_t *const second = *reinterpret_cast<const uint8_t *const *>(
        first + 0x8);
    return *reinterpret_cast<const uint32_t *>(second + 0x4);
}

/*
Purpose: Field 0 as an offset into field 0x10, or field 0xc when it is negative.

             mov edx,[ecx] / test edx,edx / jl other
             mov eax,[ecx+0x10] / add eax,edx / ret
      other: mov eax,[ecx+0xc] / ret

         `jl` is the SIGNED test, so the negative branch is taken on the sign
         bit rather than on a large unsigned value - the difference shows up
         for any field with the top bit set.

Original Offset: 005E3650
Return Value: field 0x10 + field 0, or field 0xc
Status: Complete
*/
uint32_t __fastcall leaf_005e3650_redirect(void *self, void *) {
    const uint8_t *const bytes = static_cast<const uint8_t *>(self);
    const int32_t index = *reinterpret_cast<const int32_t *>(bytes + 0x0);
    if (index < 0) {
        return *reinterpret_cast<const uint32_t *>(bytes + 0xc);
    }
    return *reinterpret_cast<const uint32_t *>(bytes + 0x10)
           + static_cast<uint32_t>(index);
}

/*
Purpose: Store an argument into three fields, clamping anything outside 0..3
         to zero, and clear a fourth.

             mov eax,[esp+4] / test eax,eax / jl zero / cmp eax,3 / jle keep
       zero: xor eax,eax
       keep: mov [ecx+0x510],eax / mov [ecx+0x518],eax / mov [ecx+0x514],eax
             mov dword [ecx+0x51c],0 / ret 4

         Both bounds are SIGNED (`jl`, `jle`) and the upper one is inclusive:
         3 is kept, 4 is not. Two mutants of the LOWER bound survive the
         sweep and are equivalent rather than untested: at 0 the clamp target
         and the value coincide, so `< 0`, `< 1` and `<= 0` all store 0. The
         clamp VALUE is a different matter and is covered - changing it to 1
         fails the fixture. The three destinations are 0x510, 0x518 and
         0x514 - written in that order, which is not ascending, and a
         transcription that tidied it would still be correct only because the
         value stored is the same for all three.

Original Offset: 005E3660
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_005e3660_redirect(void *self, void *, int requested) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    const int32_t value = (requested < 0 || requested > 3) ? 0 : requested;
    *reinterpret_cast<int32_t *>(bytes + 0x510) = value;
    *reinterpret_cast<int32_t *>(bytes + 0x518) = value;
    *reinterpret_cast<int32_t *>(bytes + 0x514) = value;
    *reinterpret_cast<uint32_t *>(bytes + 0x51c) = 0;
}

/*
Purpose: Store a non-zero argument into field 0x38, or refuse with code 10.

             push ebp / mov ebp,esp / mov eax,[ebp+8] / test eax,eax / jne set
             mov eax,0xa / pop ebp / ret 4
        set: mov [ecx+0x38],eax / xor eax,eax / pop ebp / ret 4

         Zero is the failure case and it stores NOTHING - the field keeps
         whatever it held. Returning 10 without that being true of the object
         would be a different function.

Original Offset: 004482F0
Return Value: 0 on success, 10 when the argument is zero
Status: Complete
*/
uint32_t __fastcall leaf_004482f0_redirect(void *self, void *, int value) {
    if (value == 0) {
        return 0xA;
    }
    *reinterpret_cast<int32_t *>(static_cast<uint8_t *>(self) + 0x38) = value;
    return 0;
}

/*
Purpose: Clear field 4, set the low bit of field 8, put 0x24 in field 0.

             mov eax,ecx / mov ecx,[eax+8] / mov dword [eax+4],0
             or ecx,1 / mov dword [eax],0x24 / mov [eax+8],ecx / ret

         Field 8 is READ before field 4 is written and written back afterwards,
         but the three fields are distinct, so the interleaving cannot change
         the result. EAX still holds `this` at the `ret` - the residue this
         tree treats as a return value.

Original Offset: 004C8070
Return Value: `this`
Status: Complete
*/
void *__fastcall leaf_004c8070_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) |= 1;
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x24;
    return self;
}
