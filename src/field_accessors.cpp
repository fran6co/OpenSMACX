#include "stdafx.h"
#include "field_accessors.h"










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
// RECOVERY_HISTORY: retired redirect had emitted fact ?field_accessor_0061f785_redirect@@YIXPAX0HHHHHHH@Z
// size      3 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/


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
