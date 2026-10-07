#include "stdafx.h"
#include "byte_utilities.h"
#include <cstring>

/*
Purpose: Fill the eight bytes at a pointer with 0xff.

             push ebp / mov ebp,esp / mov eax,[ebp+8] / mov cl,0xff
             mov dword [eax],0xffffffff / mov byte [eax+4],cl ... [eax+7],cl
             pop ebp / ret

         One argument and a bare `ret`, so __cdecl - the caller cleans, and
         nothing here needs to know whether it passes more. The original
         writes the first four bytes as a dword and the next four one at a
         time; the result is eight 0xff bytes either way.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x0057DEE0 sub_57dee0 0x0057DEE0-0x0057DEFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x0057DEE0-0x0057DEFC: cdecl pointer argument; writes exactly the 8-byte range [target,target+8) to 0xFF, plain ret", "Pinned instruction stream is a bounded eight-byte memory write with no receiver, global, or struct-field interpretation.", "Source family: src/byte_utilities.cpp owns a bounded byte-fill utility; no original function name or translation-unit claim is made."], "reason": "Own this complete bounded byte-fill utility. The descriptive API makes no original symbol or translation-unit claim."}
// symbol    ?fill_eight_bytes_ff@@YAXPAX@Z
// historical original marker: 0x0057DEE0 sub_57dee0 0x0057DEE0-0x0057DEFC BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_0057dee0_redirect@@YAXPAX@Z
// size      28 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

void __cdecl fill_eight_bytes_ff(void *target) {
    const uint32_t fill_word = 0xFFFFFFFFU;
    std::memcpy(target, &fill_word, sizeof(fill_word));
    uint8_t *const bytes = static_cast<uint8_t *>(target);
    const uint8_t fill = 0xFF;
    bytes[4] = fill;
    bytes[5] = fill;
    bytes[6] = fill;
    bytes[7] = fill;
}
