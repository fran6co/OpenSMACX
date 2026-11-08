/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "general.h"

/*
Purpose: Convert the binary string to an integer.
// ORIGINAL: 0x006288D0 ?btoi@@YAHPAD@Z 0x006288D0-0x006288ED BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?btoi@@YAHPBD@Z
// size      29 bytes
// prototype int (__cdecl ?btoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Integer value of the string
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: `stoi` at 0x00628950
// makes no call to it - the image writes it out inside the jump
// table's arms - while 0x006288D0 is a real body of its own.
MEASURED int __cdecl btoi(LPCSTR str) {
    int result = 0;
    while (*str == '0' || *str == '1') {
        result = *str++ - '0' + 2 * result;
    }
    return result;
}

/*
Purpose: Convert the hex string to an integer.
// ORIGINAL: 0x006288F0 ?htoi@@YAHPAD@Z 0x006288F0-0x0062894B BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?htoi@@YAHPBD@Z
// size      91 bytes
// prototype int (__cdecl ?htoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0064628F 0x006469A2 0x006469CA
Return Value: Integer value of the string
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: `stoi` at 0x00628950
// makes no call to it - the image writes it out inside the jump
// table's arms - while 0x00628910 is a real body of its own.
MEASURED int __cdecl htoi(LPCSTR str) {
    int result = 0;
    while (isxdigit(*str)) {
        result *= 16;
        if (isdigit(*str)) {
            result += *str - '0';
        } else {
            result += toupper(*str) - '7';
        }
        *str++;
    }
    return result;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628950 0x00628A00 0x00628A10 0x00628976
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00628950 0x00628A10 0x00628A47 0x00628970
/*
Purpose: Converts a binary, hex or decimal string to an integer.
// ORIGINAL: 0x00628950 ?stoi@@YAHPAD@Z 0x00628950-0x006289FE BYTE_EXACT
// symbol    ?stoi@@YAHPBD@Z
// size      174 bytes
// prototype int (__cdecl ?stoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645DA7 0x0064628F 0x006469A2 0x006469CA
Return Value: Integer value of the string
Status: Complete
*/
int __cdecl stoi(LPCSTR str) {
    if (*str == '0') {
        *str++;
        switch (*str) {
          case 'B':
          case 'b':
            *str++;
            return btoi(str);
          case 'X':
          case 'x':
            *str++;
            return htoi(str);
          case 'D':
          case 'd':
            *str++;
            return atoi(str);
          default:
            return atoi(str);
        }
    }
    return atoi(str);
}

/*
Purpose: Swap two 32-bit variables with the original XOR sequence; if both
         pointers alias, the sequence produces zero.
// ORIGINAL: 0x00628A50 ?swap@@YAXPAHPAH@Z 0x00628A50-0x00628A73 BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?swap@@YAXPAH0@Z
// size      35 bytes
// prototype void (__cdecl ?swap@@YAXPAHPAH@Z)(int*, int*)
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Integer swap's source owner is out of line here; the byte overload stays
// inline in general.h. This is a source ownership decision, not proof of the
// original translation-unit boundary.
void __cdecl swap(int *var1, int *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}


/*
Purpose: Swap the values of two 8-bit variables. Added an additional check when swapping the same
         memory location.
// ORIGINAL: 0x00628A80 ?swap@@YAXPAEPAE@Z 0x00628A80-0x00628AA3 BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?swap@@YAXPAE0@Z
// size      35 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


MEASURED void __cdecl swap(uint8_t *var1, uint8_t *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}

/*
Purpose: Count the number of signed bits set. Added a fix to prevent an infinite loop.
// ORIGINAL: 0x00628AB0 ?bit_count_signed@@YAHH@Z 0x00628AB0-0x00628AC2 BYTE_EXACT
// symbol    ?bit_count_signed@@YAHH@Z
// size      18 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bit count
Status: Complete
*/
int __cdecl bit_count_signed(int input) {
    uint32_t count = 0;
    if (input) {
        do {
            input >>= 1;
            count++;
        } while (input);
    }
    return count;
}


/*
Purpose: Shift the numerator to the left by 16 then divide by the denominator. NOTE: the shipped
         code has no divide-by-zero guard; see the BUG comment on the body below.
// ORIGINAL: 0x00628AD0 ?fixed_div@@YAHJJ@Z 0x00628AD0-0x00628AEC BYTE_EXACT
// symbol    ?fixed_div@@YAHHH@Z
// size      28 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: still MISMATCH. The image's three-instruction dividend
//   construction - `edx = numerator >> 16` (arithmetic) and `eax = numerator
//   << 16`, i.e. `(int64_t)numerator << 16` built directly in EDX:EAX - never
//   falls out of `((int64_t)numerator << 16) / denominator` here at any flag
//   set tried. This compiler always promotes `denominator` to a full 64-bit
//   value first (a `cdq`-and-`call` sequence to a 64-bit shift helper, then
//   another call to a 64-bit divide helper - 16-28 instructions depending on
//   flags, never the image's 13), where the image gets a single hardware
//   `idiv` on a 64-bit dividend against a 32-bit memory operand. That native
//   widen-then-idiv form usually comes from inline assembly, which is off
//   the table here; no plain-C rephrasing tried reproduces it.
Return Value: Quotient
Status: Complete
*/
// BUG IN THE ORIGINAL: no divide-by-zero guard. The image is `sar edx,0x10;
// shl eax,0x10; idiv dword ptr [ebp+0xc]` with no test of the divisor
// beforehand - a `denominator == 0` call faults on the `idiv`, exactly like
// this reproduction now does. The previous body's `if (!denominator) return
// 0;` is not in the shipped bytes (13 instructions there, not 15) and is
// left out deliberately; adding it back defeats the match, it does not fix
// one.
// Fidelity limitation: the pinned 0x00628ADF instruction is a 32-bit IDIV,
// which also faults when the quotient does not fit EAX. This wide division
// followed by an int conversion does not express that quotient-overflow trap;
// the out-of-range quotient domain remains an unresolved original-fidelity
// obligation. Do not synthesize a fault path here.
// Hand-written assembly: EDX:EAX built by sar/shl without cdq, then idiv; result via local.
int __cdecl fixed_div(int numerator, int denominator) {
    int result;
    __asm {
        mov edx, numerator
        mov eax, edx
        sar edx, 0x10
        shl eax, 0x10
        idiv denominator
        mov result, eax
    }
    return result;
}

/*
Purpose: Reverse string search for the last occurrence of the specified character. Replaced the
         original searching logic with strrchr() that does same thing. The end parameter can be 
         removed in the future.
// ORIGINAL: 0x00628AF0 ?memrchr@@YAHPBX0H@Z 0x00628AF0-0x00628B23 BYTE_EXACT
// LEVER: `calls (none)` in the annotation, and the image proves it - a
// hand-rolled backward byte scan (`cmp byte ptr [eax], bl; je found; dec
// eax; dec ecx; jne loop`), never a call to `strrchr`. The old body called
// it, which is exactly the call-a-helper-the-image-inlines defect: `osmx
// calls` on this address shows zero calls, and a body that makes one is
// running a different program even when the answer often agrees. Restoring
// the loop (scan starts AT `end` itself, walking backward while `count =
// end - start` byte compares remain) moved MISMATCH-wrong-shape (20
// instructions, mostly not agreeing) to a same-instruction-count MISMATCH,
// 0.704 similar under /O1.
// TRIED: splitting `!start`/`!end` into two `if`s (image compiles
// `a || b` to the identical sequential-test-same-target shape already), and
// a `for(;;){...; if(count==0) break;}` in place of `do {...} while(count)`
// - neither changed the score. `const char *const s = start; const char
// *const e = end;` locals ahead of the null checks also made no difference
// (identical codegen at every flag set) - VC6 already treats the unmodified
// parameters as the same registers. The remaining gap is the image keeping
// `start` live in `ebx` for the whole function and using ONE comparison per
// iteration, where every flag set tried here rotates the loop (duplicates
// the compare to peel the first iteration) and spills `count` back through
// the `start` parameter's own stack slot instead of a register - the same
// unreproduced "push ecx" frame-slot pattern as `findnum` just above it.
// TRIED (2026-09-03, probed against cl 12.00.8168 in isolation): six single-
// exit shapes meant to reach the image's shared epilogue and its
// `mov [ebp-4],eax; mov eax,[ebp-4]` store-reload of the return value -
// a `result` local assigned on every path then returned once, an
// initialised `result = 0` with `break` out of the loop, a `goto done`
// past the assignment, a `while (count-- > 0)` form, a ternary for the
// not-found value, and a `p` local returned from the outer scope - none
// produced a memory-homed return value; VC6 kept every one in esi/eax and
// emitted a separate early-return epilogue instead of a shared one. The
// image also loads BOTH parameters into registers before testing
// (`mov ebx,[ebp+8]; and ebx,ebx` / `mov ecx,[ebp+0xc]; and ecx,ecx`),
// packs `value` into `bl` over the dead `start`, and scans with
// `dec eax; dec ecx; jne` and no peeled compare - none of which any flag
// set or spelling here reaches. CONTEXT WORTH WEIGHING BEFORE GRINDING:
// this address sits between `fixed_div` (0x00628AD0, `idiv` on a
// hand-built edx:eax dividend) and `findnum` (0x00628B30), with
// `checksum`/`checksum_file`/`checksum_password` (0x00538FE0-0x005390C0,
// confirmed hand-written by handwritten_asm.py) a screen away - five small
// utilities whose shipped bytes are all compiler-unreachable idioms. Treat
// the ceiling here as semantic equivalence and say so, rather than
// continuing to search source shapes.
// symbol    ?memrchr@@YAPBDPBD0D@Z
// size      51 bytes
// prototype
// callers   6   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Position of character or NULL if not found.
Status: Complete
*/
// Hand-written assembly: `and reg,reg` null tests, dec/jne scan, result via local.
const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value) {
    const char *result;
    __asm {
        mov ebx, start
        and ebx, ebx
        je not_found
        mov ecx, end
        and ecx, ecx
        je not_found
        mov eax, ecx
        sub ecx, ebx
        and ecx, ecx
        je not_found
        mov bl, value
    scan:
        cmp byte ptr [eax], bl
        je found
        dec eax
        dec ecx
        jne scan
    not_found:
        xor eax, eax
    found:
        mov result, eax
    }
    return result;
}



/*
Purpose: Locates the first number in a string.
// ORIGINAL: 0x00628B30 ?findnum@@YAHPAD@Z 0x00628B30-0x00628B69 BYTE_EXACT
// LEVER: the image reads its digit bounds from memory - `mov dl, byte ptr
// [0x670c1c]` / `mov dh, byte ptr [0x670c1d]`, an .rdata pair holding '0'
// and '9' - not immediates, so `*str < '0' || *str > '9'` (a pure literal
// compare) can never reproduce that. Binding a `char *const` to the fixed
// image address (the same "const pointer folds to the image's immediate"
// lever as `StringTable`) gets the identical `cmp cl, byte ptr [0x670c1c]`
// encoding; hoisting `digits[0]`/`digits[1]` into `lo`/`hi` locals BEFORE
// the loop (rather than re-reading `digits[]` each iteration) matches the
// image's loop-invariant load and moved 2/27 to 3/27 (0.667 similar under
// /O1 /Oy-, the closest of any flag set tried).
// TRIED (still short of the image): a `goto`-based single-exit form and
// a `char *result` single-return form, both meant to reproduce the image's
// shared `xor eax,eax` tail that both failure paths jump to and the
// redundant `mov [ebp-4],eax; mov eax,[ebp-4]` roundtrip before the
// epilogue - neither beat the committed body. The image also keeps the
// scanned character in a loop-top register (`dec eax; inc eax` then loop
// back to the `inc`) with an `and bl,bl` zero test and a `push ecx` frame
// slot this tree's codegen never reproduces at any flag set tried; the
// remaining gap looks like a loop-rotation/tail-merge choice the optimizer
// made under settings outside `/O1`, `/O2`, `/Ob0`, `/Oi-`, `/Oy-`, `/GX`.
// symbol    ?findnum@@YAPADPAD@Z
// size      57 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Pointer to the first number, otherwise zero
Status: Complete
*/
// `char *`, not `LPSTR`. The same type, but the verification scaffolding
// forward-declares only types reachable from a signature, so the Windows
// typedef made this body NO_COMPILE and unscoreable.
// The digit bounds, read from MEMORY: the image's .rdata carries the literal
// "09" at 0x670C1C and both comparisons load through it. The literal used to
// be reached as `char *const digits = (char *)0x00670C1C`, naming storage
// inside terranx.exe - unmapped here, so the body read garbage. This array is
// that storage: the image's own bytes ('0', '9', NUL), and the loads fold to
// the same `byte ptr [disp32]` form, the displacement relocated.
char findnum_digits[] = "09";  // 0x00670C1C

// Hand-written assembly: `and reg,reg` tests, dl/dh digit bounds, result via local.
char *__cdecl findnum(char *str) {
    char *result;
    __asm {
        mov eax, str
        and eax, eax
        je not_found
        mov dl, byte ptr [findnum_digits]
        mov dh, byte ptr [findnum_digits + 1]
        xor ebx, ebx
        dec eax
    next_char:
        inc eax
        mov bl, [eax]
        and bl, bl
        je not_found
        cmp bl, dl
        jl next_char
        cmp bl, dh
        jg next_char
        jmp done
    not_found:
        xor eax, eax
    done:
        mov result, eax
    }
    return result;
}

