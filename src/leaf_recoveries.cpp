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

#include <cmath>
#include "leaf_recoveries.h"
#include "win.h"
#include "field_accessors.h"
#include "buffer.h"
#include "graphicwin.h"
#include "basebutton.h"
#include "dialogs.h"
#include "listbox.h"
#include "dialog.h"

#include <cstring>

namespace {

// Every field access below goes through these rather than through a typed
// pointer, and that is load-bearing rather than fussiness.
//
// The first version of leaf_005cbbc0_redirect held the node in
// `uint8_t *const *`, read it, stored through a `uint32_t *`, and read it
// again - expecting the second read to see the store, exactly as the original
// re-reads `[ecx]`. It did not: the pointee is const and the two accesses have
// unrelated types, so the compiler is entitled to cache the first read and
// did. The re-read was in the source and absent from the object.
//
// `memcpy` has no such licence: it aliases everything, and gcc still lowers
// these to a single `mov`.
uint32_t load32(const void *base, size_t offset) {
    uint32_t value;
    std::memcpy(&value, static_cast<const uint8_t *>(base) + offset,
                sizeof(value));
    return value;
}

void store32(void *base, size_t offset, uint32_t value) {
    std::memcpy(static_cast<uint8_t *>(base) + offset, &value, sizeof(value));
}

}  // namespace

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

/*
Purpose: Add another 3x3 float matrix into this one, element by element.

             mov edx,eax / mov ebx,3 / mov esi,ecx / mov edi,3 / sub esi,eax
             fld [esi+edx] / fadd [edx] / add edx,4 / dec edi
             fstp [edx-4] / jne inner / dec ebx / jne outer / ret 4

         ESI is the BYTE DELTA between the two matrices, computed once, and
         EDX walks forward without being reset between outer iterations - so
         the two nested threes are nine consecutive floats, not three rows of
         three restarted each time.

Original Offset: 006347C0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_006347c0_redirect(void *self, void *, const float *other) {
    float *const mine = static_cast<float *>(self);
    for (int index = 0; index < 9; ++index) {
        mine[index] = other[index] + mine[index];
    }
}

/*
Purpose: Subtract another 3x3 float matrix from this one.

             fld [edx] / fsub [esi+edx] / ... / ret 4

         The same nine-element walk as 006347C0, but loading THIS first: the
         result is this - other, and subtraction does not commute.

Original Offset: 006348F0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_006348f0_redirect(void *self, void *, const float *other) {
    float *const mine = static_cast<float *>(self);
    for (int index = 0; index < 9; ++index) {
        mine[index] = mine[index] - other[index];
    }
}

/*
Purpose: Scale a 3x3 float matrix by a scalar.

             mov edi,3 / mov esi,3 / fld [esp+0xc] / fmul [edx] / add edx,4
             dec esi / fstp [edx-4] / jne inner / dec edi / jne outer / ret 4

         The scalar sits at [esp+0xc] because ESI and EDI were pushed first;
         before those pushes it is the single argument at [esp+4]. It is
         re-loaded inside the inner loop, which changes nothing - it is the
         same float32 every time.

Original Offset: 006348C0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_006348c0_redirect(void *self, void *, float scale) {
    float *const mine = static_cast<float *>(self);
    for (int index = 0; index < 9; ++index) {
        mine[index] = scale * mine[index];
    }
}

/*
Purpose: Are two three-component float vectors equal?

             fld [ecx] / fcomp [edx] / fnstsw ax / test ah,0x40 / je unequal
             ... x3 ... / mov al,1 / ret 4     unequal: xor al,al / ret 4

         `test ah,0x40` reads C3, and x87 sets C3 for EQUAL **or UNORDERED**.
         So a NaN in either vector makes this report equal, which `a == b`
         in C++ does not. The condition written out is "neither below nor
         above", which is false for an ordered mismatch and true for NaN -
         exactly what the flags say.

         The original sets AL alone and leaves the rest of EAX untouched;
         returning `bool` matches, because a caller of a byte-returning
         function reads AL.

Original Offset: 006344E0
Return Value: true when every component matches, or is unordered
Status: Complete
*/
bool __fastcall leaf_006344e0_redirect(void *self, void *, const float *other) {
    const float *const mine = static_cast<const float *>(self);
    for (int index = 0; index < 3; ++index) {
        if (mine[index] < other[index] || mine[index] > other[index]) {
            return false;
        }
    }
    return true;
}

/*
Purpose: Length of a three-component float vector.

             fld [eax] / ... / fmul / fmul / fmul / faddp st(2) / faddp st(1)
             fsqrt / fstp st(1) / ret

         The sum is (y*y + z*z) + x*x - components 1 and 2 first, then 0 - and
         it stays in x87 EXTENDED precision all the way into the `fsqrt`. That
         is not a detail: rounding the sum to float before taking the root
         would be a different function. The C++ below compiles to the same
         association with an inline `fsqrt`, checked with objdump.

         The original also writes the caller's argument slot as scratch. That
         is its business - __cdecl, the caller owns and re-pushes it - and
         reproducing it would be copying an implementation detail, not the
         function.

Original Offset: 006281E0
Return Value: the length, in ST(0)
Status: Complete
*/
float __cdecl leaf_006281e0_redirect(const float *vector) {
    // `long double` here is not decoration. Written with float operands this
    // compiles, UNDER THIS TREE'S FLAGS, to `fstps` and a tail call to
    // `sqrtf` - the sum rounded to float32 BEFORE the root, which is a
    // different function in the last bit. In long double it becomes an inline
    // `fsqrt` on the extended sum, which is what the original does. Checked
    // with objdump under the real compile command: a plain `-O2` on its own
    // produces the inline form and would have hidden the difference.
    const long double sum = static_cast<long double>(vector[1]) * vector[1]
                          + static_cast<long double>(vector[2]) * vector[2]
                          + static_cast<long double>(vector[0]) * vector[0];
    return static_cast<float>(std::sqrt(sum));
}

/*
Purpose: Scale a field by an argument and divide by 600.

             mov ecx,[ecx+0x3798] / mov eax,0x1b4e81b5 / imul ecx,[esp+4]
             imul ecx / mov eax,edx / sar eax,6 / mov ecx,eax / shr ecx,0x1f
             add eax,ecx / ret 4

         MSVC's magic-number division: multiply by 0x1b4e81b5, keep the high
         dword, shift right 6, then add the sign bit so it truncates toward
         zero instead of toward negative infinity. 0x1b4e81b5 is
         ceil(2^38 / 600), and the divisor was confirmed by running the
         original's exact sequence against truncating division over 206,006
         inputs - the whole of -3000..3000, 200,000 random int32s, and both
         extremes - with no disagreement.

         The multiply is an ordinary 32-bit one and WRAPS. Spelling it in
         unsigned and casting back keeps that defined, which signed overflow
         in C++ would not be.

Original Offset: 0063BEB0
Return Value: (field 0x3798 * argument) / 600
Status: Complete
*/
int32_t __fastcall leaf_0063beb0_redirect(void *self, void *, int factor) {
    const int32_t field = *reinterpret_cast<const int32_t *>(
        static_cast<const uint8_t *>(self) + 0x3798);
    const int32_t product = static_cast<int32_t>(
        static_cast<uint32_t>(field) * static_cast<uint32_t>(factor));
    return product / 600;
}

/*
Purpose: Scale a field by an argument and divide by 440.

         The same shape as 0063BEB0 with a different field, magic and shift:
         0x94f2095 is ceil(2^36 / 440). Confirmed the same way, over the same
         206,006 inputs, with no disagreement.

Original Offset: 0063BEE0
Return Value: (field 0x379c * argument) / 440
Status: Complete
*/
int32_t __fastcall leaf_0063bee0_redirect(void *self, void *, int factor) {
    const int32_t field = *reinterpret_cast<const int32_t *>(
        static_cast<const uint8_t *>(self) + 0x379c);
    const int32_t product = static_cast<int32_t>(
        static_cast<uint32_t>(field) * static_cast<uint32_t>(factor));
    return product / 440;
}

/*
Purpose: Fill the eight bytes at a pointer with 0xff.

             push ebp / mov ebp,esp / mov eax,[ebp+8] / mov cl,0xff
             mov dword [eax],0xffffffff / mov byte [eax+4],cl ... [eax+7],cl
             pop ebp / ret

         One argument and a bare `ret`, so __cdecl - the caller cleans, and
         nothing here needs to know whether it passes more. The original
         writes the first four bytes as a dword and the next four one at a
         time; the result is eight 0xff bytes either way.

Original Offset: 0057DEE0
Return Value: n/a
Status: Complete
*/
void __cdecl leaf_0057dee0_redirect(void *target) {
    uint8_t *const bytes = static_cast<uint8_t *>(target);
    for (int index = 0; index < 8; ++index) {
        bytes[index] = 0xFF;
    }
}

/*
Purpose: Hand out the next of ten 60-byte slots, wrapping back to the first.

             mov edx,[ecx+4] / lea eax,[edx+edx*2] / inc edx / cmp edx,0xa
             mov [ecx+4],edx / lea eax,[eax+eax*4] / lea eax,[ecx+eax*4+8]
             jne end / mov dword [ecx+4],0 / end: ret

         The two `lea`s multiply by 3 then by 5, and the third scales by 4:
         60 bytes a slot, the array starting at offset 8.

         The pointer handed back is the slot the counter held ON ENTRY, and
         the counter is advanced afterwards - so the wrap to zero does not
         affect what this call returns, only the next one. The `cmp` happens
         before the store either way; writing the wrap first would return the
         wrong slot on the tenth call and only then.

Original Offset: 006252C0
Return Value: a pointer to the slot that was current on entry
Status: Complete
*/
void *__fastcall leaf_006252c0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    int32_t *const counter = reinterpret_cast<int32_t *>(bytes + 4);
    const int32_t current = *counter;
    *counter = current + 1;
    if (*counter == 10) {
        *counter = 0;
    }
    return bytes + 8 + current * 60;
}

/*
Purpose: Half the sum of an argument and a span read out of a structure.

             mov ecx,[ebp+8] / mov eax,[ecx+0x14] / mov edx,[ecx+0x10]
             sub eax,edx / mov edx,[ebp+0xc] / add eax,edx
             cdq / sub eax,edx / sar eax,1 / ret

         `cdq / sub eax,edx / sar eax,1` is signed division by two that
         truncates toward zero: the `sub` adds one first when the value is
         negative, which is exactly what a bare `sar` would get wrong.

Original Offset: 00408470
Return Value: (a->[0x14] - a->[0x10] + b) / 2
Status: Complete
*/
int __cdecl leaf_00408470_redirect(const void *source, int addend) {
    const uint8_t *const bytes = static_cast<const uint8_t *>(source);
    const uint32_t high = *reinterpret_cast<const uint32_t *>(bytes + 0x14);
    const uint32_t low = *reinterpret_cast<const uint32_t *>(bytes + 0x10);
    const int32_t total = static_cast<int32_t>(
        high - low + static_cast<uint32_t>(addend));
    return total / 2;
}

/*
Purpose: Fill 1,536 dwords with -1, starting at offset 0xa20.

             lea eax,[ecx+0xa24] / mov ecx,0x200 / or edx,0xffffffff
             mov [eax-4],edx / mov [eax],edx / mov [eax+4],edx
             add eax,0xc / dec ecx / jne / ret

         Three dwords per pass and 0x200 passes, so 1,536 in all - and the
         cursor starts at 0xa24 while the first store is at [eax-4], so the
         region actually begins at 0xa20 and runs to 0x261f. Reading the
         `lea` as the start would put the whole thing four bytes too high.

Original Offset: 005AD450
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_005ad450_redirect(void *self, void *) {
    uint32_t *const slots = reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(self) + 0xA20);
    for (int index = 0; index < 0x200 * 3; ++index) {
        slots[index] = 0xFFFFFFFFU;
    }
}

/*
Purpose: Count the set bits in the low THIRTY-ONE bits of ECX.

             mov edx,ecx / xor eax,eax / xor ecx,ecx
             mov esi,1 / shl esi,cl / test edx,esi / je / inc eax
             inc ecx / cmp ecx,0x1f / jl / ret

         `cmp ecx,0x1f / jl` runs the body for bit indices 0 through 30. BIT 31
         IS NEVER TESTED, so this is not a population count - it agrees with
         one on every value with the top bit clear, which is most of them.

         ECX carries a VALUE here, not a `this` pointer, which is why this one
         does not take the (void *, void *) shape the rest of these do.

Original Offset: 005CC430
Return Value: how many of bits 0..30 are set
Status: Complete
*/
int __fastcall leaf_005cc430_redirect(uint32_t value) {
    int count = 0;
    for (int bit = 0; bit < 0x1F; ++bit) {
        if (value & (1U << bit)) {
            ++count;
        }
    }
    return count;
}

/*
Purpose: Three when there is no structure, otherwise zero or seven.

             mov eax,[esp+4] / test eax,eax / jne have / mov eax,3 / ret 0x10
       have: mov eax,[eax+0xc] / neg eax / sbb eax,eax / and al,0xf9
             add eax,7 / ret 0x10

         `neg / sbb eax,eax` is the idiom for "0 if the value was zero, -1
         otherwise". Then `and al,0xf9` masks only AL - so -1 becomes
         0xfffffff9, and adding 7 WRAPS to zero. Zero stays zero and becomes
         7. The wrap is the mechanism, not an accident, and a body that masked
         the whole register would give 7 in both cases.

         `ret 0x10` cleans four dwords while only the first is read; the other
         three are declared so the adapter cleans what the original cleans.

Original Offset: 005D7A10
Return Value: 3 with no structure, 0 when field 0xc is set, 7 when it is not
Status: Complete
*/
int __stdcall leaf_005d7a10_redirect(const void *source, int, int, int) {
    if (source == nullptr) {
        return 3;
    }
    const uint32_t flag = *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(source) + 0xC);
    return flag != 0 ? 0 : 7;
}

/*
Purpose: Forward all four arguments to 005D7A10.

             mov eax,[esp+0x10] / mov edx,[esp+0xc] / push eax
             mov eax,[esp+0xc] / push edx / mov edx,[esp+0xc]
             push eax / push edx / add ecx,0x444 / call 005D7A10 / ret 0x10

         The interleaved reads and pushes look like a shuffle and are not: each
         `[esp+0xc]` is read AFTER a push has moved ESP, so they walk backwards
         through the arguments and re-push them in the same order.

         `add ecx,0x444` computes a subobject pointer that 005D7A10 never
         reads - it takes everything from the stack. The adjustment is
         therefore unobservable, and reproducing it would be copying a leftover
         rather than the function. That is a claim about the callee, and it is
         checked: see its recovery above, which reads `[esp+4]` and nothing
         else.

Original Offset: 005D5470
Return Value: whatever 005D7A10 returns
Status: Complete
*/
int __fastcall leaf_005d5470_redirect(void *, void *, const void *source,
                                      int second, int third, int fourth) {
    return leaf_005d7a10_redirect(source, second, third, fourth);
}

/*
Purpose: Reverse the low `count` bits of a value.

             xor eax,eax / loop: mov esi,ecx / and esi,1 / or eax,esi
             shr ecx,1 / shl eax,1 / dec edx / test edx,edx / jg loop
             shr eax,1 / ret

         The loop is a DO-WHILE: the body runs once before the counter is
         tested, so a count of zero or a negative one still moves one bit
         through and the answer is bit 0 rather than nothing. Writing it as a
         `for` would return 0 for those, which is a different function.

         The trailing `shr eax,1` undoes the last of the `shl`s, which is why
         the shift happens after the OR rather than before it.

Original Offset: 00642940
Return Value: the low `count` bits, reversed
Status: Complete
*/
uint32_t __cdecl leaf_00642940_redirect(uint32_t value, int count) {
    uint32_t result = 0;
    int remaining = count;
    do {
        result |= value & 1U;
        value >>= 1;
        result <<= 1;
        --remaining;
    } while (remaining > 0);
    return result >> 1;
}

/*
Purpose: Report a node's neighbours through two optional out-parameters.

             mov eax,[ecx] / test eax,eax / jne have / ret 8
       have: mov edx,[esp+8] / test edx,edx / je skip1
             mov eax,[eax+4] / mov [edx],eax
      skip1: mov eax,[esp+4] / test eax,eax / je skip0
             mov edx,[ecx] / mov edx,[edx] / mov [eax],edx
      skip0: mov eax,[ecx] / mov eax,[eax+8] / ret 8

         With no node it returns whatever EAX held, which the `test` has just
         established is zero.

         The original RE-READS `[ecx]` after each store instead of keeping the
         node in a register. That is preserved here rather than tidied: if an
         out-parameter points at `[ecx]` itself, the second read sees the value
         just written, and a cached version would not.

Original Offset: 0063E7F0
Return Value: node->[8]
Status: Complete
*/
uint32_t __fastcall leaf_0063e7f0_redirect(void *self, void *,
                                           uint32_t *first, uint32_t *second) {
    if (load32(self, 0) == 0) {
        return 0;
    }
    if (second != nullptr) {
        store32(second, 0, load32(reinterpret_cast<void *>(load32(self, 0)), 4));
    }
    if (first != nullptr) {
        store32(first, 0, load32(reinterpret_cast<void *>(load32(self, 0)), 0));
    }
    return load32(reinterpret_cast<void *>(load32(self, 0)), 8);
}

/*
Purpose: Divide, rounding the quotient AWAY from zero when there is a
         remainder, and report both.

             mov eax,ecx / cdq / idiv esi / mov edx,eax / imul edx,esi
             sub ecx,edx / mov [edx],ecx / je zero / inc eax / mov [ecx],eax
             ret 0x10   zero: mov [edx],eax / ret 0x10

         The `je` is three instructions after the `sub` that sets it, with two
         `mov`s and a `pop` in between - none of which touch flags. It tests
         whether the REMAINDER was zero, not anything nearer to it.

         `idiv` truncates toward zero, so the +1 rounds away from zero rather
         than up: -7 by 2 gives a remainder of -1 and a quotient of -3, which
         becomes -2. Both traps of the original are kept, because both
         spellings go through one division: a zero divisor and INT_MIN / -1.

Original Offset: 00532A50
Return Value: the quotient that was stored
Status: Complete
*/
int __stdcall leaf_00532a50_redirect(int value, int *quotient_out,
                                     int *remainder_out, int divisor) {
    const int quotient = value / divisor;
    const int remainder = value - quotient * divisor;
    *remainder_out = remainder;
    const int rounded = remainder == 0 ? quotient : quotient + 1;
    *quotient_out = rounded;
    return rounded;
}

/*
Purpose: Is a point inside an inclusive rectangle?

             mov eax,[ebp+8] / mov ecx,[ebp+0x10] / cmp eax,ecx / jl no
             cmp eax,[ebp+0x18] / jg no / mov eax,[ebp+0xc]
             mov ecx,[ebp+0x14] / cmp eax,ecx / jl no / cmp eax,[ebp+0x1c]
             jg no / mov eax,1 / ret     no: xor eax,eax / ret

         Six arguments in the order x, y, left, top, right, bottom - the two
         coordinates first and the bounds interleaved after, which is not the
         (left, top, right, bottom, x, y) an eye expects.

         `jl` and `jg` are SIGNED, and both bounds are INCLUSIVE: a point
         exactly on an edge is inside.

Original Offset: 00592DB0
Return Value: 1 when inside, 0 otherwise
Status: Complete
*/
int __cdecl leaf_00592db0_redirect(int x, int y, int left, int top,
                                   int right, int bottom) {
    if (x < left || x > right) {
        return 0;
    }
    if (y < top || y > bottom) {
        return 0;
    }
    return 1;
}

/*
Purpose: Does this window hold the dialog focus, and its partner too?

             mov esi,ecx / call Win::is_dialog_focus / test eax,eax / je no
             mov ecx,[esi+0xc4] / test ecx,ecx / je yes
             call Win::is_dialog_focus / test eax,eax / je no
             yes: mov eax,1 / ret        no: xor eax,eax / ret

         The second call is made on the window at field 0xc4 - which
         src/win.h names `win_parent_` - not on `this` again; ECX is reloaded
         from it just before. A NULL parent is not a failure: the answer is
         yes.

Original Offset: 006161A0
Return Value: 1 when both hold focus, 0 otherwise
Status: Complete
*/
int __fastcall leaf_006161a0_redirect(void *self, void *) {
    if (win_is_dialog_focus_redirect(reinterpret_cast<Win *>(self),
                                     nullptr) == 0) {
        return 0;
    }
    Win *const parent = *reinterpret_cast<Win *const *>(
        static_cast<uint8_t *>(self) + 0xC4);
    if (parent != nullptr
            && win_is_dialog_focus_redirect(parent, nullptr) == 0) {
        return 0;
    }
    return 1;
}

/*
Purpose: Find the table entry matching two keys and set its third word.

             lea edx,[ecx+0xa24] / cmp [edx-4],edi / jne next / cmp [edx],esi
             je found / next: inc eax / add edx,0xc / cmp eax,0x200 / jl
             ret 0xc      found: mov edx,[ebp+0x10] / lea eax,[eax+eax*2]
                                 mov [ecx+eax*4+0xa28],edx / ret 0xc

         The same 512 three-word entries at 0xa20 that 005AD450 fills with -1.
         The cursor is again offset by four, so `[edx-4]` is word 0 and `[edx]`
         is word 1.

         TWO THINGS THE RETURN VALUE GETS WRONG IF TIDIED. On a hit it is
         `3 * index`, not the index - the `lea eax,[eax+eax*2]` that scales for
         the store is the SAME register that is returned. On a miss it is 512,
         the loop counter's final value, which is a valid-looking index and not
         -1.

Original Offset: 005AD4C0
Return Value: 3 * index on a hit, 512 on a miss
Status: Complete
*/
int __fastcall leaf_005ad4c0_redirect(void *self, void *, int first_key,
                                      int second_key, int value) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    int32_t *const table = reinterpret_cast<int32_t *>(bytes + 0xA20);
    for (int index = 0; index < 0x200; ++index) {
        if (table[index * 3] == first_key
                && table[index * 3 + 1] == second_key) {
            table[index * 3 + 2] = value;
            return index * 3;
        }
    }
    return 0x200;
}

/*
Purpose: Swap two pairs of fields in the object this one points at.

             mov eax,[ecx] / mov edx,[eax+0x20] / test edx,edx / je second
             cmp [ecx+0x4c],1 / jne second
             mov esi,[eax+0x1c] / mov [eax+0x1c],edx / mov eax,[ecx]
             mov [eax+0x20],esi
     second: mov eax,[ecx] / mov edx,[eax+4] / test dh,8 / je end
             mov esi,[eax+0x4c] / mov edx,[eax+0x50] / mov [eax+0x50],esi
             mov ecx,[ecx] / mov [ecx+0x4c],edx / ret

         Two independent swaps on the NODE at `this->[0]`, each with its own
         guard - and the guards read different objects. The first tests the
         node's 0x20 for non-zero AND **`this`**'s 0x4c against 1; the second
         tests bit 11 of the node's field 4, which is what `test dh,8` means.

         The node pointer is re-read from `[ecx]` between the two halves of
         each swap, exactly as the original does, so a swap that wrote over
         `[ecx]` would be seen.

Original Offset: 005CBBC0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_005cbbc0_redirect(void *self, void *) {
    void *node = reinterpret_cast<void *>(load32(self, 0));
    const uint32_t moved = load32(node, 0x20);
    if (moved != 0 && load32(self, 0x4C) == 1) {
        const uint32_t previous = load32(node, 0x1C);
        store32(node, 0x1C, moved);
        node = reinterpret_cast<void *>(load32(self, 0));   // re-read
        store32(node, 0x20, previous);
    }
    node = reinterpret_cast<void *>(load32(self, 0));
    if ((load32(node, 0x4) & 0x800U) != 0) {
        const uint32_t low = load32(node, 0x4C);
        const uint32_t high = load32(node, 0x50);
        store32(node, 0x50, low);
        node = reinterpret_cast<void *>(load32(self, 0));   // re-read
        store32(node, 0x4C, high);
    }
}

/*
Purpose: Initialise 24 sixty-byte slots and the count that follows them.

             mov eax,ecx / xor ecx,ecx / mov edx,eax / mov [eax+0x5a0],ecx
             mov esi,0x18 / mov byte [edx],0xff / mov word [edx+8],cx
             add edx,0x3c / dec esi / jne / ret

         Three widths in six instructions: a DWORD at 0x5a0, then per slot a
         BYTE at +0 and a WORD at +8. Widening either of the last two would
         clear bytes the original leaves alone - 0x5a0 is exactly 24 * 60, so
         the dword sits immediately past the table and there is no slack to
         absorb a mistake.

         EAX still holds `this` at the `ret`.

Original Offset: 0052DCA0
Return Value: `this`
Status: Complete
*/
void *__fastcall leaf_0052dca0_redirect(void *self, void *) {
    store32(self, 0x5A0, 0);
    uint8_t *slot = static_cast<uint8_t *>(self);
    for (int index = 0; index < 0x18; ++index) {
        slot[0] = 0xFF;
        const uint16_t zero = 0;
        std::memcpy(slot + 8, &zero, sizeof(zero));
        slot += 0x3C;
    }
    return self;
}

/*
Purpose: Is the movie still playing?

             add ecx,0xa14 / call MCIVideo::is_playing
             neg eax / sbb eax,eax / neg eax / ret

         `neg / sbb eax,eax / neg` is the idiom for "normalise to 0 or 1":
         the first pair gives 0 or -1, the second `neg` flips -1 to 1.

         It is kept because the original does it, NOT because it changes
         anything here - the recovered MCIVideo::is_playing masks with 1 and
         so already returns 0 or 1. Dropping the normalisation passes the
         fixture; that mutant is equivalent through this callee, and would
         stop being so if is_playing were ever recovered as returning some
         other non-zero value.

         MCIVideo sits at 0xa14, and its is_playing is the bit-0 read
         recovered as field_accessor_00600320_redirect.

Original Offset: 004041C0
Return Value: 1 when playing, 0 otherwise
Status: Complete
*/
int __fastcall leaf_004041c0_redirect(void *self, void *) {
    return field_accessor_00600320_redirect(
               static_cast<uint8_t *>(self) + 0xA14, nullptr) != 0 ? 1 : 0;
}

/*
Purpose: Construct the Buffer subobject, then clear one field.

             mov esi,ecx / lea ecx,[esi+0x8dc] / call ??0Buffer
             mov dword [esi+0x10c],0 / mov eax,esi / ret

         The Buffer lives at 0x8dc and the cleared field at 0x10c - which is
         BELOW the Buffer, so the store is in this object's own space and not
         inside the subobject. It happens after the constructor, which is the
         order that would matter if the constructor ever reached that far back.

Original Offset: 004BEA30
Return Value: `this`
Status: Complete
*/
void *__fastcall leaf_004bea30_redirect(void *self, void *) {
    buffer_construct_redirect(
        reinterpret_cast<Buffer *>(static_cast<uint8_t *>(self) + 0x8DC),
        nullptr);
    store32(self, 0x10C, 0);
    return self;
}

/*
Purpose: Reset six fields, then close the window and its Buffer.

             xor eax,eax / mov [esi+0x2b60],eax / mov [esi+0x2b6c],eax
             mov [esi+0x2b70],eax / mov [esi+0x2b80],eax
             mov [esi+0x2b68],0xffffffff / mov [esi+0x4648],eax
             call GraphicWin::close / lea ecx,[esi+0x406c]
             call Buffer::close / ret

         Five of the six are cleared and ONE - 0x2b68 - is set to -1. It is
         written FIFTH, between 0x2b80 and 0x4648, so the write order is not
         ascending and the odd value is not at the end either.

         GraphicWin::close is called on `this` itself; Buffer::close on the
         subobject at 0x406c.

Original Offset: 00432970
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_00432970_redirect(void *self, void *) {
    store32(self, 0x2B60, 0);
    store32(self, 0x2B6C, 0);
    store32(self, 0x2B70, 0);
    store32(self, 0x2B80, 0);
    store32(self, 0x2B68, 0xFFFFFFFFU);
    store32(self, 0x4648, 0);
    graphic_win_close_redirect(reinterpret_cast<GraphicWin *>(self), nullptr);
    buffer_close_redirect(
        reinterpret_cast<Buffer *>(static_cast<uint8_t *>(self) + 0x406C),
        nullptr);
}

/*
Purpose: Append a 16-bit value to a byte stream, most significant byte first.

             mov esi,[eax+8] / mov edi,[eax+0x10] / mov edx,ecx / shr edx,8
             mov byte [esi+edi],dl / mov edx,[eax+0x10] / mov esi,[eax+8]
             inc edx / mov [eax+0x10],edx / mov byte [esi+edx],cl
             mov ecx,[eax+0x10] / inc ecx / mov [eax+0x10],ecx / ret

         BIG-endian: the high byte goes first. The cursor at 0x10 is advanced
         between the two stores and again after the second, so it ends two
         past where it started, and the second byte lands at the position the
         FIRST advance produced.

         Both the base at 0x8 and the cursor at 0x10 are RE-READ from the
         stream between the stores rather than kept in registers, so a stream
         that overlapped its own descriptor would see the updated values.
         Preserved for the same reason as in 005CBBC0 - and reached through
         load32/store32 so the compiler cannot cache what the original
         re-reads.

         The CURSOR re-read is pinned by a fixture that points the stream at
         its own descriptor. The BASE re-read is not: separating it needs the
         first byte store to land on the descriptor's own pointer field at
         0x8, and what the corrupted pointer then addresses depends on the
         arena's absolute address, so the outcome could not be asserted
         portably. Two mutants of that line survive the sweep for that reason
         and are left alone rather than papered over.

Original Offset: 0063D420
Return Value: n/a
Status: Complete
*/
void __cdecl leaf_0063d420_redirect(void *stream, uint32_t value) {
    uint8_t *base = reinterpret_cast<uint8_t *>(load32(stream, 0x8));
    uint32_t cursor = load32(stream, 0x10);
    base[cursor] = static_cast<uint8_t>((value >> 8) & 0xFFU);

    cursor = load32(stream, 0x10) + 1;
    base = reinterpret_cast<uint8_t *>(load32(stream, 0x8));
    store32(stream, 0x10, cursor);
    base[cursor] = static_cast<uint8_t>(value & 0xFFU);

    store32(stream, 0x10, load32(stream, 0x10) + 1);
}


// ImageButton's two vtables, named so the image addresses live in one place -
// the same treatment BufferVtable and BaseButtonBufferVtable already get, and
// the reason the two candidates carrying them stayed in the queue when the
// seven that DEREFERENCE image addresses left it.
const uint32_t ImageButtonPrimaryVtable = 0x00670A94;
const uint32_t ImageButtonBufferVtable = 0x00670A8C;

/*
Purpose: Construct the BaseButton base, publish ImageButton's vtables, clear
         one field.

             mov esi,ecx / call ??0BaseButton / mov [esi],0x670a94
             mov [esi+0x444],0x670a8c / mov [esi+0xab8],0 / mov eax,esi / ret

         Two vtables, at 0 and at 0x444 - the primary and the Buffer
         subobject's, the same pair GraphicWin and BaseButton each publish.
         Both are written AFTER the base constructor and overwrite what it
         left.

         No ImageButton class is declared for these three. Deriving one from
         BaseButton and giving it a field at 0xab8 would assert that
         sizeof(BaseButton) is exactly 0xab8, and nothing here evidences that;
         raw offsets claim only what the instructions show, which is the same
         rule src/net_class.h states for Net.

Original Offset: 006252E0
Return Value: `this`
Status: Complete
*/
void *__fastcall image_button_construct_redirect(void *self, void *) {
    base_button_construct_redirect(reinterpret_cast<BaseButton *>(self),
                                   nullptr);
    store32(self, 0x0, ImageButtonPrimaryVtable);
    store32(self, 0x444, ImageButtonBufferVtable);
    store32(self, 0xAB8, 0);
    return self;
}

/*
Purpose: Clear the field, then close as a BaseButton.

             mov dword [ecx+0xab8],0 / jmp ?close@BaseButton@@QAEXXZ

         A TAIL jump, so BaseButton::close's return value is this function's -
         but the mangled name says `X`, void, so nothing reads it. Declared
         void to match the name, and the value is discarded rather than
         quietly re-typed.

Original Offset: 00625330
Return Value: n/a
Status: Complete
*/
void __fastcall image_button_close_redirect(void *self, void *) {
    store32(self, 0xAB8, 0);
    base_button_close_redirect(reinterpret_cast<BaseButton *>(self), nullptr);
}

/*
Purpose: Keep the first argument, hand the other eight to BaseButton::init.

             mov eax,[esp+4] / mov edx,[esp+0x24] / mov [ecx+0xab8],eax
             ...eight interleaved reads and pushes... / call BaseButton::init
             ret 0x24

         The reads and pushes alternate because each `[esp+0x20]` is taken
         AFTER a push has moved ESP - so they walk backwards through the
         arguments and re-push them in order. Nine arguments in (`ret 0x24`),
         the first kept at 0xab8 and the remaining eight passed straight on.

Original Offset: 00625340
Return Value: whatever BaseButton::init returns
Status: Complete
*/
int __fastcall image_button_init_redirect(
        void *self, void *, int image, LPCSTR name, int id, int x, int y,
        int width, int height, Win *parent, int style_flag) {
    store32(self, 0xAB8, static_cast<uint32_t>(image));
    return base_button_init_redirect(
        reinterpret_cast<BaseButton *>(self), nullptr, name, id, x, y, width,
        height, parent, style_flag);
}

// The text sub_59d230 hands to Dialogs::item. It is a REBINDABLE pointer, not
// a constant: the callee dereferences it, so a fixture that could not swap it
// would fault on the original image's address rather than test anything.
char *DialogsItemText6900C4 = reinterpret_cast<char *>(0x006900C4);

/*
Purpose: Add one fixed item to the Dialogs at 0x21d0.

             push -2 / push 0x6900c4 / add ecx,0x21d0
             call ?item@Dialogs@@QAEHPADH@Z / ret

         A tail call in everything but name: the callee cleans its own two
         arguments and this function cleans none, so its `ret` carries the
         callee's result straight out.

         -2 is the index. It is not a count or a flag, and it is negative,
         which is the sort of thing that gets "tidied" to 2 or 0 by someone
         reading quickly.

Original Offset: 0059D230
Return Value: whatever Dialogs::item returns
Status: Complete
*/
int __fastcall leaf_0059d230_redirect(void *self, void *) {
    Dialogs *const dialogs = reinterpret_cast<Dialogs *>(
        static_cast<uint8_t *>(self) + 0x21D0);
    return dialogs->item(DialogsItemText6900C4, -2);
}

/*
Purpose: Clear four fields, then close as a GraphicWin.

             xor eax,eax / mov [ecx+0xa24],eax / mov [ecx+0xa28],eax
             mov [ecx+0xa38],eax / mov [ecx+0xa3c],eax
             jmp ?close@GraphicWin@@QAEXXZ

         Four fields in two PAIRS - 0xa24/0xa28 and 0xa38/0xa3c - with a gap
         between them that is not written. The tail jump means GraphicWin's
         return is this function's, and the mangled name says void, so nothing
         reads it.

Original Offset: 00484B60
Return Value: n/a
Status: Complete
*/
void __fastcall pick_tech_close_redirect(void *self, void *) {
    store32(self, 0xA24, 0);
    store32(self, 0xA28, 0);
    store32(self, 0xA38, 0);
    store32(self, 0xA3C, 0);
    graphic_win_close_redirect(reinterpret_cast<GraphicWin *>(self), nullptr);
}




// The two destructor chains below reach four recovered destructors. Running
// those for real needs three class teardown fixtures composed, and three
// attempts at that produced a fixture that faulted, then faulted further on,
// and finally passed only one run in three - a green result that did not
// reproduce.
//
// So the callees are SEAMS, which is this tree's established answer to exactly
// this problem: WinOriginalClose, DialogOriginalClose and BufferFree are all
// the same idea. Each defaults to the real redirect, so nothing about the
// shipped behaviour changes; a fixture can bind them and check the one thing
// these functions actually do, which is call three of them with three
// particular pointers in a particular order.
func_leaf_adjusted_dtor *LeafListBoxDestructor = &list_box_destructor_redirect;
func_leaf_adjusted_dtor *LeafDialogsDestructor = &dialogs_destructor_redirect;
func_leaf_dialog_dtor *LeafDialogDestructor = &dialog_destructor_redirect;
func_leaf_graphic_dtor *LeafGraphicWinDestructor =
    &graphic_win_destructor_redirect;

/*
Purpose: Destroy the ListBox at 0x48, the Dialog at 0xa60, and the GraphicWin
         base the ListBox shares.

             mov esi,ecx / lea edi,[esi+0x48] / mov ecx,edi
             call ??1ListBox@@QAE@XZ / lea ecx,[esi+0xa60]
             call ??1Dialog@@QAE@XZ / mov ecx,edi
             call ??1GraphicWin@@QAE@XZ / ret

         EDI is computed ONCE and used twice - for the ListBox and again for
         the GraphicWin. They are the same subobject at 0x48, destroyed at two
         levels, with an unrelated Dialog at 0xa60 in between. A body that gave
         GraphicWin the outer object would be wrong only where 0x48 matters.

         0x48 is ListBoxDestructorAdjustment, and list_box_destructor_redirect
         takes exactly that already-adjusted pointer, so it is passed through
         rather than recomputed.

Original Offset: 004080B0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_004080b0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    uint8_t *const inner = bytes + ListBoxDestructorAdjustment;
    LeafListBoxDestructor(inner, nullptr);
    LeafDialogDestructor(reinterpret_cast<Dialog *>(bytes + 0xA60), nullptr);
    LeafGraphicWinDestructor(reinterpret_cast<GraphicWin *>(inner), nullptr);
}

/*
Purpose: Destroy the Dialogs at 0x188, the Dialog at 0xba0, and the GraphicWin
         base the Dialogs shares.

         The same shape as 004080B0 with different offsets and a Dialogs in
         place of the ListBox. 0x188 is DialogsDestructorAdjustment, and is
         likewise computed once and used twice.

Original Offset: 00406AF0
Return Value: n/a
Status: Complete
*/
void __fastcall leaf_00406af0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    uint8_t *const inner = bytes + DialogsDestructorAdjustment;
    LeafDialogsDestructor(inner, nullptr);
    LeafDialogDestructor(reinterpret_cast<Dialog *>(bytes + 0xBA0), nullptr);
    LeafGraphicWinDestructor(reinterpret_cast<GraphicWin *>(inner), nullptr);
}
