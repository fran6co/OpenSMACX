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
#include "heap.h"
#include "general.h"
#include "strings.h"  // StringTemp, say_num() - Heap::get()'s error message idiom

/*
// ORIGINAL: 0x005D4560 ??0Heap@@QAE@XZ 0x005D4560-0x005D4573 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ??0Heap@@QAE@XZ)(Heap* this)
// callers   6   call targets   0
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Heap::Heap() {
    // IMAGE ORDER: current_ before base_, free_size_ before base_size_.
    err_flags_ = 0;
    current_ = nullptr;
    base_ = nullptr;
    free_size_ = 0;
    base_size_ = 0;
}

/*
Purpose: Shutdown the class instance.
// ORIGINAL: 0x005D45B0 ?shutdown@Heap@@QAEXXZ 0x005D45B0-0x005D45DA BYTE_EXACT
// size      42 bytes
// prototype void (__thiscall ?shutdown@Heap@@QAEXXZ)(Heap* this)
// callers   93   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void Heap::shutdown() {
    // SAME SHAPE AS ~Heap() (heap.h): the null goes INSIDE the guard as well
    // as after it, and the store order is err_flags_, current_, base_,
    // free_size_, base_size_ - not declaration order.
    if (base_) {
        free(base_);
        base_ = nullptr;
    }
    err_flags_ = 0;
    current_ = nullptr;
    base_ = nullptr;
    free_size_ = 0;
    base_size_ = 0;
}

/*
Purpose: Deflate the heap of any free memory.
// ORIGINAL: 0x005D45E0 ?squeeze@Heap@@QAEXH@Z 0x005D45E0-0x005D4615 BYTE_EXACT
// size      53 bytes
// prototype void (__thiscall ?squeeze@Heap@@QAEXH@Z)(Heap* this, int toggle)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00647132
Return Value: n/a
Status: Complete
*/
void Heap::squeeze(int UNUSED(toggle)) {
    // NOT CACHED. The image RE-READS base_size_ and free_size_ after the
    // call instead of reusing the value it computed for realloc's argument.
    size_t const size = base_size_ - free_size_;
    LPVOID const old_base = base_;
    LPVOID new_addr = realloc(old_base, size);
    if (new_addr) {
        base_ = new_addr;
        base_size_ -= free_size_;
        free_size_ = 0;
    }
}

/*
Purpose: Initialize the class instance and allocate the requested memory size.
// ORIGINAL: 0x005D4620 ?init@Heap@@QAE_NH@Z 0x005D4620-0x005D4675 BYTE_EXACT
// symbol    ?init@Heap@@QAEHI@Z
// size      85 bytes
// prototype bool (__thiscall ?init@Heap@@QAE_NH@Z)(Heap* this, int)
// callers   7   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL Heap::init(size_t req_size) {
    // shutdown()'S BODY, INLINED AND WHOLLY GUARDED: `osmx calls` names
    // 0x005D4510 and 0x00644EF2 (free) only, no call to 0x005D45B0
    // (Heap::shutdown) - and unlike shutdown() itself, the image's `je`
    // skips ALL FIVE stores, not just the free() call, when base_ is
    // already null.
    if (base_) {
        free(base_);
        err_flags_ = 0;
        current_ = nullptr;
        base_ = nullptr;
        free_size_ = 0;
        base_size_ = 0;
    }
    base_ = mem_get(req_size);
    if (!base_) {
        return true; // error: failed to allocate memory
    }
    current_ = base_;
    free_size_ = req_size;
    base_size_ = req_size;
    return false;
}

/*
Purpose: Get the requested memory size. If there currently isn't enough memory to meet the request, 
         allocate additional memory in blocks of 1024 bytes until request is met.
// ORIGINAL: 0x005D4680 ?get@Heap@@QAEPAXH@Z 0x005D4680-0x005D47CB
// LEVER: call_diff showed FEWER (3 vs 15) - this body called sprintf_s,
//        MessageBoxA and exit only. `osmx calls --all` on the image gives
//        realloc, 7x strcat, 3x add_lf, 3x _itoa, MessageBoxA, exit, and
//        `mov byte ptr [0x9b86a0], 0` - the same strcat/_itoa-into-a-GLOBAL
//        idiom as name_base() (base.cpp) and say_num() (strings.h), not
//        sprintf_s into a stack local. Rewrote to strcat/add_lf/say_num()
//        into StringTemp (0x9B86A0, strings.h). Best similarity 0.896 ->
//        0.943 (/O2 /Gy /GR- /GX) and call_diff now agrees exactly.
// TRIED: hand-inlining say_num's _itoa+strcat body at each of the three
//        call sites instead of calling it - identical 0.943/15/104, so
//        VC6 already folds say_num() the same way here. Reverted to the
//        say_num() spelling as the more idiomatic match to its own lever.
// symbol    ?get@Heap@@QAEPAXI@Z
// size      331 bytes
// prototype void* (__thiscall ?get@Heap@@QAEPAXH@Z)(Heap* this, int reqSize)
// callers   17   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00600840 0x00644DFF 0x00645470 0x00647132 0x0064FC88
// indirect  0x005D47BE
Return Value: Memory pointer with address to requested size
Status: Complete
*/
LPVOID Heap::get(size_t req_size) {
    while (static_cast<int>(free_size_) < static_cast<int>(req_size)) {
        if (err_flags_ & 1) {
            return NULL; // error
        }
        LPVOID new_addr = realloc(base_, base_size_ + 1024);
        if (!new_addr) {
            // LEVER: image builds this message with strcat/say_num()/add_lf
            // into the GLOBAL StringTemp buffer (0x9B86A0 -
            // `mov byte ptr [0x9b86a0], 0`), not sprintf_s into a stack
            // local - `osmx calls --all` shows realloc, then 7x strcat, 3x
            // add_lf (general.h, appends '\n'), 3x _itoa, MessageBoxA, exit,
            // and the disassembly's `_itoa` calls write into a stack buffer
            // then strcat that onto StringTemp - exactly say_num()
            // (strings.h). Same idiom as name_base() in base.cpp.
            StringTemp[0] = 0;
            strcat(StringTemp, "Aborting due to a heap shortage!");
            add_lf(StringTemp);
            strcat(StringTemp, "Base size: ");
            say_num((int)base_size_);
            add_lf(StringTemp);
            strcat(StringTemp, "Free size: ");
            say_num((int)free_size_);
            add_lf(StringTemp);
            strcat(StringTemp, "Requested size: ");
            say_num((int)req_size);
            MessageBoxA(NULL, StringTemp, "Heap Notice!!", MB_OK);
            exit(3);
        }
        base_ = new_addr;
        base_size_ += 1024;
        free_size_ += 1024;
    }
    LPVOID free_mem_addr = current_;
    free_size_ -= req_size;
    current_ = LPVOID(size_t(current_) + req_size);
    return free_mem_addr;
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005D4580 ??1Heap@@QAE@XZ 0x005D4580-0x005D45AA BYTE_EXACT
// body      src/heap.h
// size      42 bytes
// prototype void (__thiscall ??1Heap@@QAE@XZ)(Heap* this)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
