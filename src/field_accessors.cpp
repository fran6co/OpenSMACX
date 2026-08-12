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
ORIGINAL: 0x00406840 BYTE_EXACT
// name      sub_406840
// size      6 bytes
// spans     0x00406840-0x00406846
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004476E0 BYTE_EXACT
// name      ?tech@SAmbience@@QAEXXZ
// size      4 bytes
// spans     0x004476E0-0x004476E4
// prototype void (__thiscall ?tech@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00447AB0 BYTE_EXACT
// name      ?design_window_show@BAmbience@@QAEXXZ
// size      5 bytes
// spans     0x00447AB0-0x00447AB5
// prototype void (__thiscall ?design_window_show@BAmbience@@QAEXXZ)(BAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00448280 BYTE_EXACT
// name      ?tech@AAmbience@@QAEXXZ
// size      4 bytes
// spans     0x00448280-0x00448284
// prototype void (__thiscall ?tech@AAmbience@@QAEXXZ)(AAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00448310 BYTE_EXACT
// name      sub_448310
// size      4 bytes
// spans     0x00448310-0x00448314
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00448320 BYTE_EXACT
// name      sub_448320
// size      4 bytes
// spans     0x00448320-0x00448324
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00448330 BYTE_EXACT
// name      sub_448330
// size      4 bytes
// spans     0x00448330-0x00448334
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004C75A0 BYTE_EXACT
// name      sub_4c75a0
// size      7 bytes
// spans     0x004C75A0-0x004C75A7
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004C8090 BYTE_EXACT
// name      sub_4c8090
// size      35 bytes
// spans     0x004C8090-0x004C80B3
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004C8100 BYTE_EXACT
// name      sub_4c8100
// size      11 bytes
// spans     0x004C8100-0x004C810B
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004C93E0 BYTE_EXACT
// name      sub_4c93e0
// size      8 bytes
// spans     0x004C93E0-0x004C93E8
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00590D60 BYTE_EXACT
// name      sub_590d60
// size      11 bytes
// spans     0x00590D60-0x00590D6B
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005DA6A0 BYTE_EXACT
// name      sub_5da6a0
// size      8 bytes
// spans     0x005DA6A0-0x005DA6A8
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005DA6B0 BYTE_EXACT
// name      sub_5da6b0
// size      8 bytes
// spans     0x005DA6B0-0x005DA6B8
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005E2460 BYTE_EXACT
// name      sub_5e2460
// size      8 bytes
// spans     0x005E2460-0x005E2468
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005E2470 BYTE_EXACT
// name      sub_5e2470
// size      8 bytes
// spans     0x005E2470-0x005E2478
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005E35D0 BYTE_EXACT
// name      sub_5e35d0
// size      31 bytes
// spans     0x005E35D0-0x005E35EF
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005E35F0 BYTE_EXACT
// name      sub_5e35f0
// size      29 bytes
// spans     0x005E35F0-0x005E360D
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00600320 BYTE_EXACT
// name      ?is_playing@MCIVideo@@QAEHXZ
// size      6 bytes
// spans     0x00600320-0x00600326
// prototype int (__thiscall ?is_playing@MCIVideo@@QAEHXZ)(MCIVideo* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00616B20 BYTE_EXACT
// name      sub_616b20
// size      13 bytes
// spans     0x00616B20-0x00616B2D
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0062D390 BYTE_EXACT
// name      sub_62d390
// size      6 bytes
// spans     0x0062D390-0x0062D396
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x006346C0 BYTE_EXACT
// name      sub_6346c0
// size      36 bytes
// spans     0x006346C0-0x006346E4
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x006346F0 BYTE_EXACT
// name      sub_6346f0
// size      34 bytes
// spans     0x006346F0-0x00634712
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00634B70 BYTE_EXACT
// name      sub_634b70
// size      36 bytes
// spans     0x00634B70-0x00634B94
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00634FB0 BYTE_EXACT
// name      ??0TexHeap@@QAE@XZ
// size      16 bytes
// spans     0x00634FB0-0x00634FC0
// prototype TexHeap* (__thiscall ??0TexHeap@@QAE@XZ)(TexHeap* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x006354D0 BYTE_EXACT
// name      sub_6354d0
// size      25 bytes
// spans     0x006354D0-0x006354E9
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x006363F0 BYTE_EXACT
// name      sub_6363f0
// size      12 bytes
// spans     0x006363F0-0x006363FC
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0063E520 BYTE_EXACT
// name      sub_63e520
// size      20 bytes
// spans     0x0063E520-0x0063E534
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00406B20 BYTE_EXACT
// name      nullsub_27
// size      3 bytes
// spans     0x00406B20-0x00406B23
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00616D80 BYTE_EXACT
// name      sub_616d80
// size      5 bytes
// spans     0x00616D80-0x00616D85
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0061F785 BYTE_EXACT
// name      nullsub_259
// size      3 bytes
// spans     0x0061F785-0x0061F788
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00634C20 BYTE_EXACT
// name      ??1FileBox@@QAE@XZ
// size      1 bytes
// spans     0x00634C20-0x00634C21
// prototype void (__thiscall ??1FileBox@@QAE@XZ)(FileBox* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00634c20_redirect(void *, void *) {

}

/*
Purpose: Do nothing; the original body is only its `ret`.
         Emitted by tools/generate_field_accessors.py from

             ret 4 / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x0050F640 BYTE_EXACT
// name      nullsub_185
// size      16 bytes
// spans     0x0050F640-0x0050F650
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_0050f640_redirect(void *, void *, int stack0) {

}

/*
Purpose: Copy 1 argument(s) into field(s) of `this`.
         Emitted by tools/generate_field_accessors.py from

             push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov dword ptr [ecx + 0xa34], eax / pop ebp / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x00589750 BYTE_EXACT
// name      sub_589750
// size      16 bytes
// spans     0x00589750-0x00589760
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00589750_redirect(void *self, void *, int stack0) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0xa34) = static_cast<uint32_t>(stack0);
}

/*
Purpose: Copy 2 argument(s) into field(s) of `this`.
         Emitted by tools/generate_field_accessors.py from

             push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ebp + 0xc] / mov dword ptr [ecx], eax / mov dword ptr [ecx + 4], edx / pop ebp / ret 8

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 2 stack dword(s) so it
         cleans 8 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x00590CB0 BYTE_EXACT
// name      sub_590cb0
// size      18 bytes
// spans     0x00590CB0-0x00590CC2
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00590cb0_redirect(void *self, void *, int stack0, int stack1) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x0) = static_cast<uint32_t>(stack0);
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = static_cast<uint32_t>(stack1);
}

/*
Purpose: Copy 1 argument(s) into field(s) of `this`.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [esp + 4] / mov dword ptr [ecx + 0x128], eax / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x005F05C0 BYTE_EXACT
// name      sub_5f05c0
// size      13 bytes
// spans     0x005F05C0-0x005F05CD
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_005f05c0_redirect(void *self, void *, int stack0) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint32_t *>(bytes + 0x128) = static_cast<uint32_t>(stack0);
}

/*
Purpose: Set 5 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             xor eax, eax / mov byte ptr [ecx], al / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 4], eax / mov dword ptr [ecx + 0x10], eax / mov dword ptr [ecx + 0xc], eax / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x005D4540 BYTE_EXACT
// name      Heap::Heap2
// size      17 bytes
// spans     0x005D4540-0x005D4551
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_005d4540_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *(bytes + 0x0) = 0x0;
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x4) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0x10) = 0x0U;
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = 0x0U;
}

/*
Purpose: Set 8 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / xor dl, dl / lea ecx, [eax + 0x30c] / mov byte ptr [eax + 0x30d], dl / mov dword ptr [eax + 0x410], ecx / mov byte ptr [eax], dl / mov byte ptr [ecx], dl / mov byte ptr [eax + 0x104], dl / mov byte ptr [eax + 0x208], dl / mov byte ptr [eax + 0x414], dl / mov byte ptr [eax + 0x418], dl / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x00634BE0 BYTE_EXACT
// name      ??0FileBox@@QAE@XZ
// size      51 bytes
// spans     0x00634BE0-0x00634C13
// prototype void (__thiscall ??0FileBox@@QAE@XZ)(FileBox* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
void * __fastcall field_accessor_00634be0_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *(bytes + 0x30d) = 0x0;
    *reinterpret_cast<uint8_t **>(bytes + 0x410) = bytes + 0x30c;
    *(bytes + 0x0) = 0x0;
    *(bytes + 0x30c) = 0x0;
    *(bytes + 0x104) = 0x0;
    *(bytes + 0x208) = 0x0;
    *(bytes + 0x414) = 0x0;
    *(bytes + 0x418) = 0x0;
    return self;
}

/*
Purpose: Set 8 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             lea eax, [ecx + 0x30c] / xor dl, dl / mov dword ptr [ecx + 0x410], eax / mov byte ptr [ecx + 0x30d], dl / mov byte ptr [eax], dl / mov byte ptr [ecx], dl / mov byte ptr [ecx + 0x104], dl / mov byte ptr [ecx + 0x208], dl / mov byte ptr [ecx + 0x414], dl / mov byte ptr [ecx + 0x418], dl / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x00634F70 BYTE_EXACT
// name      ?UNK2@FileBox@@QAEXXZ
// size      49 bytes
// spans     0x00634F70-0x00634FA1
// prototype void (__thiscall ?UNK2@FileBox@@QAEXXZ)(FileBox* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_00634f70_redirect(void *self, void *) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    *reinterpret_cast<uint8_t **>(bytes + 0x410) = bytes + 0x30c;
    *(bytes + 0x30d) = 0x0;
    *(bytes + 0x30c) = 0x0;
    *(bytes + 0x0) = 0x0;
    *(bytes + 0x104) = 0x0;
    *(bytes + 0x208) = 0x0;
    *(bytes + 0x414) = 0x0;
    *(bytes + 0x418) = 0x0;
}

/*
Purpose: Clamp field 0xc against an argument, then store unconditionally.
         Emitted by tools/generate_field_accessors.py from

             push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 0xc] / cmp edx, eax / jae 0x4c80d0 / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax / pop ebp / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x004C80C0 BYTE_EXACT
// name      sub_4c80c0
// size      23 bytes
// spans     0x004C80C0-0x004C80D7
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_004c80c0_redirect(void *self, void *, int stack0) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    if (*reinterpret_cast<uint32_t *>(bytes + 0xc)
            < static_cast<uint32_t>(stack0)) {
        *reinterpret_cast<uint32_t *>(bytes + 0xc) = static_cast<uint32_t>(stack0);
    }
    *reinterpret_cast<uint32_t *>(bytes + 0x8) = static_cast<uint32_t>(stack0);
}

/*
Purpose: Clamp field 0x8 against an argument, then store unconditionally.
         Emitted by tools/generate_field_accessors.py from

             push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 8] / cmp edx, eax / jbe 0x4c80f0 / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / pop ebp / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x004C80E0 BYTE_EXACT
// name      sub_4c80e0
// size      23 bytes
// spans     0x004C80E0-0x004C80F7
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall field_accessor_004c80e0_redirect(void *self, void *, int stack0) {
    uint8_t *const bytes = static_cast<uint8_t *>(self);
    if (*reinterpret_cast<uint32_t *>(bytes + 0x8)
            > static_cast<uint32_t>(stack0)) {
        *reinterpret_cast<uint32_t *>(bytes + 0x8) = static_cast<uint32_t>(stack0);
    }
    *reinterpret_cast<uint32_t *>(bytes + 0xc) = static_cast<uint32_t>(stack0);
}

/*
Purpose: Sign-extend bit 0 of field 0x40: 0 or -1.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [ecx + 0x40] / shl eax, 0x1f / sar eax, 0x1f / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
ORIGINAL: 0x00448380 BYTE_EXACT
// name      sub_448380
// size      10 bytes
// spans     0x00448380-0x0044838A
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
/*
ORIGINAL: 0x00448380
// name      sub_448380
// size      10 bytes
// spans     0x00448380-0x0044838A
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: bit 0 of the field at +0x40, sign-extended to a full mask
Status: Complete
*/
uint32_t __fastcall field_accessor_00448380_redirect(void *self, void *) {
    // Sign-extend bit 0 to 0xFFFFFFFF or 0: `shl 31` puts it in the sign
    // position and `sar 31` smears it back, which is what the original does.
    const int32_t value = *reinterpret_cast<const int32_t *>(
        static_cast<const uint8_t *>(self) + 0x40);
    return static_cast<uint32_t>((value << 31) >> 31);
}
