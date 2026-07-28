#include "stdafx.h"
#include "field_accessors.h"

/*
Purpose: Return the constant 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 004C75A0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_004c75a0_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x200);
}

/*
Purpose: Return the constant 0xb.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 0xb / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 4 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 004C93E0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_004c93e0_redirect(void *, void *) {
    return 0xbU;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0xc

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 12 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 005DA6A0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005da6a0_redirect(void *, void *) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x10

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 16 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 005DA6B0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005da6b0_redirect(void *, void *) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x14

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 20 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 005E2460
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005e2460_redirect(void *, void *) {
    return 0x8U;
}

/*
Purpose: Return the constant 0x8.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 8 / ret 0x14

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 20 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 005E2470
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_005e2470_redirect(void *, void *) {
    return 0x8U;
}

/*
Purpose: Read the dword field at 0x0, masked to 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx] / and eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
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
Purpose: Return the constant 0x1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, 1 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 0062D390
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_0062d390_redirect(void *, void *) {
    return 0x1U;
}

/*
Purpose: Read the dword field at 0x4ac, masked to 0x8000.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x4ac] / and eax, 0x8000 / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans 0 argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: 006363F0
Return Value: the value described above
Status: Complete
*/
uint32_t __fastcall field_accessor_006363f0_redirect(void *self, void *) {
    return *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(self) + 0x4ac)
        & 0x8000U;
}
