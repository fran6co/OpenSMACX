#include "stdafx.h"
#include "field_accessors.h"

/*
Purpose: Return the constant 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00406840
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00406840_redirect(void *, void *) {
    return 0x1U;
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 004476E0
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_004476e0_redirect(void *self, void *) {
    ++*reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(self) + 0x58);
}

/*
Purpose: Store 0x1 in the byte at 0x6d.
         Emitted by tools/generate_field_accessors.py from

             mov byte ptr [ecx + 0x6d], 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00447AB0
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00447ab0_redirect(void *self, void *) {
    *(static_cast<uint8_t *>(self) + 0x6d) = 0x1;
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00448280
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00448280_redirect(void *self, void *) {
    ++*reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(self) + 0x58);
}

/*
Purpose: Read the dword field at 0x48.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x48] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00448310
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00448310_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x48);
}

/*
Purpose: Read the dword field at 0x44.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x44] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00448320
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00448320_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x44);
}

/*
Purpose: Read the dword field at 0x50.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x50] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00448330
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00448330_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x50);
}

/*
Purpose: Read the dword field at 0x200.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x200] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 004C75A0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_004c75a0_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x200);
}

/*
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], 0xffffffff / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 004C8090
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_004c8090_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0xffffffffU;
    return self;
}

/*
Purpose: Set 2 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 004C8100
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_004c8100_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    return self;
}

/*
Purpose: Return the constant 0xb.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 0xb / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 004C93E0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_004c93e0_redirect(void *, void *, int stack0) {
    return 0xbU;
}

/*
Purpose: Set 1 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov dword ptr [ecx + 0x200], 0 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00590D60
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00590d60_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x200) = 0x0U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0xc

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 3 stack dword(s) so it
         cleans 12 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005DA6A0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005da6a0_redirect(void *, void *, int stack0, int stack1, int stack2) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x10

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 4 stack dword(s) so it
         cleans 16 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005DA6B0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005da6b0_redirect(void *, void *, int stack0, int stack1, int stack2, int stack3) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x14

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 5 stack dword(s) so it
         cleans 20 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005E2460
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005e2460_redirect(void *, void *, int stack0, int stack1, int stack2, int stack3, int stack4) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x14

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 5 stack dword(s) so it
         cleans 20 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005E2470
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005e2470_redirect(void *, void *, int stack0, int stack1, int stack2, int stack3, int stack4) {
    return 0x8U;
}

/*
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], ecx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], ecx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005E35D0
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_005e35d0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0x0U;
    return self;
}

/*
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             xor eax, eax / mov dword ptr [ecx], eax / mov dword ptr [ecx + 4], eax / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 0x10], eax / mov dword ptr [ecx + 0x14], eax / mov dword ptr [ecx + 0x18], eax / mov dword ptr [ecx + 0x1c], eax / mov dword ptr [ecx + 0x20], eax / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 005E35F0
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_005e35f0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0x0U;
}

/*
Purpose: Read the dword field at 0x0, masked to 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx] / and eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00600320
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00600320_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x0)
        & 0x1U;
}

/*
Purpose: Set 3 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00616B20
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_00616b20_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    return self;
}

/*
Purpose: Return the constant 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 0062D390
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_0062d390_redirect(void *, void *) {
    return 0x1U;
}

/*
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
Original Offset: 006346C0
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_006346c0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0x3f800000U;
    return self;
}

/*
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
Original Offset: 006346F0
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_006346f0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0x3f800000U;
}

/*
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
Original Offset: 00634B70
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_00634b70_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x3f800000U;
    *reinterpret_cast<uint32_t *>(bytes + 0x14) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x20) = 0x3f800000U;
    return self;
}

/*
Purpose: Set 4 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 4], ecx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00634FB0
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_00634fb0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    return self;
}

/*
Purpose: Set 7 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 006354D0
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_006354d0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x18) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x1c) = 0x0U;
    return self;
}

/*
Purpose: Read the dword field at 0x4ac, masked to 0x8000.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x4ac] / and eax, 0x8000 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 006363F0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_006363f0_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x4ac)
        & 0x8000U;
}

/*
Purpose: Set 4 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor ecx, ecx / mov dword ptr [eax], ecx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], 0xffffffff / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 0063E520
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_0063e520_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0xffffffffU;
    return self;
}

/*
Purpose: Do nothing; the original body is only its `ret`.
         Emitted by tools/generate_field_accessors.py from

             ret 0xc

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 3 stack dword(s) so it
         cleans 12 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00406B20
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00406b20_redirect(void *, void *, int stack0, int stack1, int stack2) {

}

/*
Purpose: Return the constant 0x0.
         Emitted by tools/generate_field_accessors.py from

             xor eax, eax / ret 0xc

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 3 stack dword(s) so it
         cleans 12 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00616D80
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_00616d80_redirect(void *, void *, int stack0, int stack1, int stack2) {
    return 0x0U;
}

/*
Purpose: Do nothing; the original body is only its `ret`.
         Emitted by tools/generate_field_accessors.py from

             ret 0x1c

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 7 stack dword(s) so it
         cleans 28 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 0061F785
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_0061f785_redirect(void *, void *, int stack0, int stack1, int stack2, int stack3, int stack4, int stack5, int stack6) {

}

/*
Purpose: Do nothing; the original body is only its `ret`.
         Emitted by tools/generate_field_accessors.py from

             ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: 00634C20
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00634c20_redirect(void *, void *) {

}
