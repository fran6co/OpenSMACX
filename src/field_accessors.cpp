#include "stdafx.h"
#include "field_accessors.h"










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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x004C8090 sub_4c8090 0x004C8090-0x004C80B3 BYTE_EXACT
// symbol    ?field_accessor_004c8090_redirect@@YIPAXPAX0@Z
// size      35 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x004C8100 sub_4c8100 0x004C8100-0x004C810B BYTE_EXACT
// symbol    ?field_accessor_004c8100_redirect@@YIPAXPAX0@Z
// size      11 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x005DA6A0 sub_5da6a0 0x005DA6A0-0x005DA6A8 BYTE_EXACT
// symbol    ?field_accessor_005da6a0_redirect@@YIIPAX0HHH@Z
// size      8 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x005DA6B0 sub_5da6b0 0x005DA6B0-0x005DA6B8 BYTE_EXACT
// symbol    ?field_accessor_005da6b0_redirect@@YIIPAX0HHHH@Z
// size      8 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x005E2460 sub_5e2460 0x005E2460-0x005E2468 BYTE_EXACT
// symbol    ?field_accessor_005e2460_redirect@@YIIPAX0HHHHH@Z
// size      8 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x005E2470 sub_5e2470 0x005E2470-0x005E2478 BYTE_EXACT
// symbol    ?field_accessor_005e2470_redirect@@YIIPAX0HHHHH@Z
// size      8 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x00616D80 sub_616d80 0x00616D80-0x00616D85 BYTE_EXACT
// symbol    ?field_accessor_00616d80_redirect@@YIIPAX0HHH@Z
// size      5 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x0061F785 nullsub_259 0x0061F785-0x0061F788 BYTE_EXACT
// symbol    ?field_accessor_0061f785_redirect@@YIXPAX0HHHHHHH@Z
// size      3 bytes
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

             ret 4 / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop / nop

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x0050F640 nullsub_185 0x0050F640-0x0050F643 BYTE_EXACT
// symbol    ?field_accessor_0050f640_redirect@@YIXPAX0H@Z
// size      3 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 BYTE_EXACT
// symbol    ?field_accessor_004c80c0_redirect@@YIXPAX0H@Z
// size      23 bytes
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
// RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
// ORIGINAL: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 BYTE_EXACT
// symbol    ?field_accessor_004c80e0_redirect@@YIXPAX0H@Z
// size      23 bytes
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
