/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "caviar.h"
#include <string.h>

// Shared Caviar message service. Its callers retain a real call boundary;
// the grouping and filename do not establish an original translation unit.

/*
Purpose: Stage the engine's ERROR MESSAGE: copy the caller's string into
         the shared buffer vox_message_buffer (0x9C0D60). Named
         vox_stage_error 2026-08-30 from the buffer's readers - the engine
         clears it to NUL before each use (0x0063830E), strlen-tests it and
         stages a default when empty (0x006383F1, "Error reading user data
         chunk"), and copies a staged default out of it wholesale
         (0x0063FA61) - every vox_* error path stages through here first.
         The
         catalogued nullary-int contract is wrong on both counts - the body
         reads a real stack argument, and `return 0;` would insert an
         `xor eax,eax` ahead of the expansion's `and ecx,3`.
// ORIGINAL: 0x00639390 sub_639390 0x00639390-0x006393B9 FILE BYTE_EXACT
// compile_context src/caviar/caviar_error.cpp
// symbol    _vox_stage_error
// size      41 bytes
// callers   12   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// LEVER: stdafx.h's `#pragma function(strcpy)` makes every strcpy a call,
//        but the image INLINES this one (`repne scasb` + `rep movsd/movsb`
//        is VC6's /Oi expansion). A TU-local `#pragma intrinsic(strcpy)`
//        just above the body restores the expansion at exactly this one
//        site - destination as a literal `(char *)0x009C0D60` so the
//        immediate folds - and BYTE_EXACT fell out first try. The pragma
//        sits below every other body in this file, none of which calls
//        strcpy, so nothing else can move.
//
// PROMOTED from src/recovered/00639390.cpp.
Return Value: n/a
*/
// The image INLINES this one strcpy - the `repne scasb` + `rep movsd/movsb`
// tail is VC6's own /Oi expansion, and calling strcpy while discarding the
// result is what reproduces it. Every other string op in the shipped image
// is an out-of-line call, which is why stdafx.h turns the four string
// routines into calls tree-wide - so this one TU-local pragma restores the
// expansion exactly where the image has it. Placed below every other body
// in this file, none of which calls strcpy, so nothing else can move.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma intrinsic(strcpy)
#endif
// The engine's staged-message slot, 0x009C0D60 in the image - real storage
// now. Every error arm writes it via vox_stage_error; the engine clears it,
// strlen-tests it and reads it back (0x0063830E/0x006383F1/0x0063FA61 - the
// last copies 27 bytes, so the slot holds at least that; the image's extent
// is unpinned, 0x100 is comfortable).

extern "C" void __cdecl vox_stage_error(const char *message) {
    strcpy(CaviarOutputMessageWindow.message_, message);
}
