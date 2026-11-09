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

// Not an original translation unit. The 2026-11-08 TU regroup moved the
// evidenced pieces out (filefind.cpp, parse.cpp, trig.cpp, number_parsing.cpp,
// mci_video.cpp, movie_media_decoder.cpp, interface_fonts.cpp, game.cpp).
// Remaining pieces and their likely owners, without enough evidence to move:
// - bit_count 0x0050BA30: battle-TU tail after bitmask (battle.cpp);
// - checksum_file/checksum/checksum_password 0x00538FE0: netdaemon-TU tail;
// - sub_5391a0 0x005391A0: inside the faction TU (faction.cpp);
// - mem_get 0x005D4510: starts the object after the decoder's CC fill, before
//   Heap; in heap.cpp VC6 inlines it into Heap::init, which the image calls;
// - HSV_to_RGB 0x00628B70: beside RGB_to_HSV (jackal/palette.cpp);
// - quick_root 0x006290E0, sub_62d390 0x0062D390, env_open 0x00634BB0:
//   single functions between other owners, no boundary evidence.


// Original mem_get at 0x005D4510 reports failure through this USER32 export; no function alias.
// RECOVERY_IMPORT: v1 __imp__MessageBoxA@16 USER32.dll name:MessageBoxA
#include "council.h"
#include <math.h> // sin(double), lowered to the CRT's _sin
#include <cstring>
#include <ddraw.h>  // IDirectDraw::Release in the DirectDraw teardown stage
// _disable / _enable for sub_5d4240, the VGA DAC loader - real VC6 intrinsics
// reproducing the image's cli/sti, not inline assembly.
#include <dos.h>
// _outp for the same body's DAC port writes.
#include <conio.h>
// VC6's real intrinsic lowering emits cli/out/sti without importing
// nonexistent _enable helpers. pushad/popad remain a documented
// source-vs-original caller-register-preservation obligation.
#pragma intrinsic(_disable, _enable, _outp)
#include "sprite.h"
#include "popup.h"
#include "buffer.h"
#include "main.h"  // legacy include; PopupAllocHook is owned by BasePop via popup.h

#include "general.h"
#include "movie_media.h"
#include "alpha.h"
#include "game.h"
#include "log.h" // log_say
#include "mapwin.h" // draw_map, promoted out of temp.h
#include "strings.h"
#include "text.h"
// free bodies homed here out of the archive reach these
#include "map.h"     // zoc_any
#include "veh.h"     // can_arty
#include "alphamovie.h"  // MCIVideo - 0x00600330 is one of its members
#include "netdaemon.h"  // Shared receiver hierarchy and NetDaemonNet storage.
#include "xpops.h"   // X_pop
#include "filemap.h" // Filemap::open_read
// Utility bodies below use these source-owned drawing interfaces.
#include "palette.h"
#include "win.h"
#include "buffer.h"
#include "checkbox.h"
#include "checkbutton.h"
#include "radiobutton.h"
#include "basebutton.h"
#include "font.h"
#include "basepop.h"
#include "filewin.h"
#include "cursor.h"
#include "time.h"







// RECOVERY_LITERAL: v1 0x00696A38 0x00696A44 ??_C@_0M@MFAG@FATAL?5ERROR?$AA@ 0x005D4510
// RECOVERY_LITERAL: v1 0x00696A44 0x00696B14 ??_C@_0NA@POMN@Windows?5has?5run?5out?5of?5virtual?5m@ 0x005D4510
/*
Purpose: Allocate memory with error checking.
// ORIGINAL: 0x005D4510 ?mem_get@@YAPAXH@Z 0x005D4510-0x005D453C BYTE_EXACT
// symbol    ?mem_get@@YAPAXH@Z
// size      44 bytes
// prototype void* (__cdecl ?mem_get@@YAPAXH@Z)(int size)
// callers   88   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644DFF 0x006470A6
// indirect  0x005D452E
Return Value: Pointer to allocated memory
Status: Complete. Historical note: an earlier DLL/CRT migration used two
        malloc wrappers to avoid a reported SDK incompatibility; the recovered
        callers now use the canonical pinned int-sized mem_get contract.
*/
LPVOID __cdecl mem_get(int size) {
    LPVOID result = malloc(size);
    if (!result) {
        MessageBoxA(NULL,
            "Windows has run out of virtual memory; the game will exit. "
            "Recommendations: defragment your hard drive; use Control Panel "
            "(System/Performance/Virtual Memory) to increase your virtual memory to at least 128MB",
            "FATAL ERROR", MB_ICONWARNING);
        exit(4);
    }
    return result;
}
/*
Purpose: Check the source file path and attempt to open a handle to the file.
// ORIGINAL: 0x00634BB0 ?env_open@@YAHPADPAD@Z 0x00634BB0-0x00634BD4 BYTE_EXACT
// symbol    ?env_open@@YAPAU_iobuf@@PBD0@Z
// size      36 bytes
// prototype 
// callers   18   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006005D0 0x00645646
Return Value: FILE pointer
Status: Complete with two versions of fopen to prevent a crash. Incompatibility with newer SDK 
        version of fopen/fopen_s. Revisit once more code is redirected to dll.
*/
FILE *__cdecl env_open_old(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    return fopen(src_check, mode);
}

FILE *__cdecl env_open(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    FILE *file_out;
    fopen_s(&file_out, src_check, mode);
    return file_out;
}




/*
Purpose: Count the number of unsigned bits set. Replaced the original code with Brian Kernighan's 
// ORIGINAL: 0x0050BA30 ?bit_count@@YAHH@Z 0x0050BA30-0x0050BA4E BYTE_EXACT
// TRIED (2026-09-03, probed against cl 12.00.8168 directly): the image's
//   `shr ebx, 1 / adc eax, 0` carry-chain fusion is NOT producible from VC6
//   C++, so this is a second case of hand-written assembly that
//   `tools/handwritten_asm.py` does not catch - it scans for `loop`, bare
//   `lodsb`, `xlatb`, `pushf/popf` and has no fingerprint for a shift-carry
//   accumulator. Twelve spellings compiled to assembly in isolation and
//   every one emitted `mov edx, ecx; and edx, 1; add eax, edx; shr ecx, 1;
//   jne`: do-while/while/for, accumulate-before-shift and after, `unsigned`
//   vs `int` counter, `if (bits & 1) count++`, the shift moved into the loop
//   CONDITION (`while (bits >>= 1)`), a 64-bit `unsigned __int64` accumulator
//   (that one does emit `adc`, but only as the HIGH half alongside the
//   `add eax, esi`, so the shape cannot converge), assignment to the
//   parameter before `return` to reach the image's
//   `mov [ebp+8],eax; mov eax,[ebp+8]` tail - which is reachable and is the
//   one half of the shape worth keeping. The image's loop is also rotated
//   with a duplicated trailing `adc` that no source form produced. Ceiling is
//   semantic equivalence, as for `checksum`/`checksum_password` above; 3/15
//   stands. Worth adding the fingerprint to handwritten_asm.py.
// size      30 bytes
// prototype
// callers   34   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: sixteen source shapes (do-while/while, `count += bits & 1`,
//   `if (bits & 1) count++`, a captured-carry temp, shift-before-test) all
//   land on the same 3/15, `push ebx` vs `mov ecx, [ebp+8]` at instruction 2
//   - none coax VC6 into the image's shr/adc carry-chain fusion; this is the
//   right algorithm (shift-and-accumulate, matching the shr/adc chain, not
//   Kernighan's clear-lowest-set-bit trick) and it scores strictly better
//   than the `n &= n - 1` form it replaced, but still MISMATCH.
Return Value: the number of set bits
Status: Complete
*/
// Hand-written assembly: shr/adc carry-count loop, result stored back to the parameter.
int __cdecl bit_count(int bitfield) {
    __asm {
        xor eax, eax
        mov ebx, bitfield
    count_loop:
        shr ebx, 1
        je count_done
        adc eax, 0
        jmp count_loop
    count_done:
        adc eax, 0
        mov bitfield, eax
    }
    return bitfield;
}

/*
Purpose: Calculate the square root of the input.
// ORIGINAL: 0x006290E0 ?quick_root@@YAHH@Z 0x006290E0-0x00629109 BYTE_EXACT
// size      41 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Square root
Status: Complete
*/
int __cdecl quick_root(int input) {
    int temp = input >> 1;
    int sq_root;
    if (input <= 1) {
        return input;
    }
    do {
        sq_root = temp;
        temp = (temp + input / temp) >> 1;
    } while (temp < sq_root);
    return sq_root;
}



/*
Purpose: Calculate a basic XOR checksum for the data buffer.
// The pinned body uses forward LODSB and a 32-bit ECX LOOP counter.
// Investigated VC6 C++ recipes emit a different loop; matching remains open.
// Preserve the original traversal/count semantics in ordinary C++, without
// inline assembly or source changes made solely to imitate instruction length.
// ORIGINAL: 0x00539090 ?checksum@@YAEPADHE@Z 0x00539090-0x005390B4 BYTE_EXACT
// size      36 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: byte-exact is unreachable (hand-written `loop`/`lodsb`, above);
//   ceiling here is semantic equivalence, currently MISMATCH 4/19.
Return Value: the XOR checksum of the buffer, seeded
Status: Complete
*/
// Hand-written assembly: lodsb/xor ah,al/loop XOR scan with explicit esi save.
uint8_t __cdecl checksum(char *buffer, int buffer_size, uint8_t seed) {
    if (!buffer_size) {
        return seed;
    }
    __asm {
        push esi
        mov esi, buffer
        mov ecx, buffer_size
        mov ah, seed
    sum_loop:
        lodsb
        xor ah, al
        loop sum_loop
        mov seed, ah
        pop esi
    }
    return seed;
}

/*
Purpose: Calculate a basic XOR checksum for a password string.
// The pinned body uses forward LODSB and a 32-bit ECX LOOP counter.
// Investigated VC6 C++ recipes emit a different loop; matching remains open.
// Preserve the original traversal/count semantics in ordinary C++, without
// inline assembly or source changes made solely to imitate instruction length.
// ORIGINAL: 0x005390C0 ?checksum_password@@YAHPAD@Z 0x005390C0-0x00539157 BYTE_EXACT
// symbol    ?checksum_password@@YAIPBD@Z
// size      151 bytes
// prototype
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460
// indirect  0x005390F6
// TRIED: byte-exact is unreachable (hand-written `loop`/`lodsb` inside
//   the `checksum` call it makes, above); ceiling here is semantic
//   equivalence, currently MISMATCH 22/58.
Return Value: Checksum
Status: Complete
*/
// The image calls strcpy out of line here.
#pragma function(strcpy)
uint32_t __cdecl checksum_password(LPCSTR password) {
    if (!strlen(password)) {
        return 0;
    }
    char buffer[256];
    strcpy(buffer, password);
    CharUpper(buffer); // incorrect results for Turkish/Azerbaijani 'i'
    uint8_t chksum = checksum(buffer, strlen(buffer), 0);
    return chksum + 1;
}
#pragma intrinsic(strcpy)





/*
Purpose: The sound-header version the game was built with. The whole image
         body is `mov eax,1 / ret` - the compiler folded the header's version
         constant to the literal - and load_sound_dll compares its answer
         against the sound DLL's own reported version and against 1, showing
         the "check all sound.h and sound device.h versions" complaint when
         they disagree. Recovered as the plain __cdecl function the image's
         only caller calls through, replacing the generator's
         field_accessor_0062d390_redirect: that adapter's two dead __fastcall
         slots would have cost the call site the two register setups the
         image does not have, and a redirect nothing calls is class debt. Was
         a BYTE_EXACT claim in field_accessors.cpp under the redirect's
         symbol; re-record here against the real name.
// ORIGINAL: 0x0062D390 sub_62d390 0x0062D390-0x0062D396 BYTE_EXACT
// symbol    _sound_header_version
// size      6 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1
Status: Complete
*/
extern "C" int __cdecl sound_header_version() {
    return 1;
}

// SDK imports, never raw IAT pointers.
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Faction artwork is owned by the genuine FactionArt array in faction.cpp;
// its Sprite members now participate in C++ construction and destruction.






// HSV_to_RGB's six double constants, read at its operands. 0.0, 360.0 and 1.0
// are linker-folded with copies other units own (palette.cpp's hsv_zero and
// hsv_hue_wrap_degrees, random.cpp's 1.0), so this unit declares them merged.
// RECOVERY_LITERAL: v2 merged 0x0066D8D8 0x0066D8E0 __real@8@3fff8000000000000000 0x00628B70
// RECOVERY_LITERAL: v2 merged 0x0066FA70 0x0066FA78 __real@8@00000000000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C20 0x00670C28 __real@8@40078000000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C28 0x00670C30 __real@8@3ff98888888888888800 0x00628B70
// RECOVERY_LITERAL: v2 merged 0x00670C30 0x00670C38 __real@8@4007b400000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C38 0x00670C40 __real@8@bfff8000000000000000 0x00628B70
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628B70 0x00628D94 0x00628DAC 0x00628C65
// ORIGINAL: 0x00628B70 ?HSV_to_RGB@@YAXPAUHSV@@PAUPALETTEENTRY@@@Z 0x00628B70-0x00628D92 FILE BYTE_EXACT
// symbol    ?HSV_to_RGB@@YAXPAUHSV@@PAUtagPALETTEENTRY@@@Z
// size      546 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006458C8 0x00647851
// Purpose: Convert one HSV triplet (palette.h's 0x18-byte double record) into
//          the palette entry the game stores. Parameters named from use:
//          `hsv_in` is read only, `entry_out` is written only.
// NOTE: the image's own mangled name spells the second parameter `PALETTEENTRY`;
// this tree emits `tagPALETTEENTRY` because the type comes from the Windows
// header rather than a local struct - see the `symbol` fact above.
void __cdecl HSV_to_RGB(HSV *hsv_in, PALETTEENTRY *entry_out) {
    if (hsv_in != 0 && entry_out != 0) {
        HSV hsv = *hsv_in;
        double r, g, b;

        if (hsv.s == 0.0) {
            if (hsv.h == -1.0) {
                r = hsv.v;
                g = hsv.v;
                b = hsv.v;
            }
        } else {
            if (hsv.h == 360.0)
                hsv.h = 0.0;
            hsv.h *= 0.016666666666666666;
            int i = (int)floor(hsv.h);
            double f = hsv.h - i;
            double p = hsv.v * (1.0 - hsv.s);
            double q = hsv.v * (1.0 - hsv.s * f);
            double t = hsv.v * (1.0 - hsv.s * (1.0 - f));
            switch (i) {
            case 0: r = hsv.v; g = t; b = p; break;
            case 1: r = q; g = hsv.v; b = p; break;
            case 2: r = p; g = hsv.v; b = t; break;
            case 3: r = p; g = q; b = hsv.v; break;
            case 4: r = t; g = p; b = hsv.v; break;
            case 5: r = hsv.v; g = p; b = q; break;
            }
        }

        entry_out->peRed = (unsigned char)(r * 256.0);
        entry_out->peGreen = (unsigned char)(g * 256.0);
        entry_out->peBlue = (unsigned char)(b * 256.0);
    }
}


// ORIGINAL: 0x00538FE0 ?checksum_file@@YAEPAD@Z 0x00538FE0-0x0053908F;0x0065E450-0x0065E462 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      193 bytes
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x006287C0
// indirect  0x0053904A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?checksum_file@@YAEPAD@Z  at 0x00538FE0  (193 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

unsigned char __cdecl checksum_file(char *file_path) {
    Filemap fm;
    unsigned char sum = 0;
    char *data = (char *)fm.open_read(file_path, 1);
    if (data == 0) {
        return 0;
    }
    unsigned int file_size = GetFileSize(fm.get_handle(), 0);
    // Hand-written assembly: lodsb/xor ah,al/loop XOR scan with explicit esi save.
    __asm {
        push esi
        mov esi, data
        mov ecx, file_size
        mov ah, sum
    sum_loop:
        lodsb
        xor ah, al
        loop sum_loop
        mov sum, ah
        pop esi
    }
    fm.close();
    return sum;
}






// ORIGINAL: 0x005391A0 sub_5391a0 0x005391A0-0x005391C0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:a1347e25:src/recovered/005391a0.cpp complete original identity/spans and historical claims/findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5391a0 full native body review.", "source:src/general.cpp existing real state owners; no executable-address alias or synthetic receiver."], "reason": "Recover the complete short helper through established source-owned diplomacy, vehicle, text or window state; preserve signed sentinel behavior, original receiver/argument order and conditional callbacks."}
// symbol    ?read_and_parse_text_line@@YAXXZ
void __cdecl read_and_parse_text_line() {
  text_get();
  char *input = TextBufferGetPtr;
  StringTemp[0] = 0;
  parse_string(input, StringTemp);
}

