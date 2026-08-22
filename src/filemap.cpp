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
#include "filemap.h"
#include "general.h"

/*
Purpose: Initialize the class by opening a file with write permission. The boolean parameter toggles 
         if the file is accessed sequentially or randomly.
// ORIGINAL: 0x006283A0 ?init@Filemap@@QAEHPADH@Z 0x006283A0-0x006283BA BYTE_EXACT
// symbol    ?init@Filemap@@QAEPAV1@PBDH@Z
// size      26 bytes
// prototype int (__thiscall ?init@Filemap@@QAEHPADH@Z)(Filemap* this, int8*, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00628540
Return Value: Pointer to the initialized class
Status: Complete
*/
Filemap *Filemap::init(LPCSTR file_name, BOOL is_sequential) {
    open(file_name, is_sequential);
    return this;
}

/*
Purpose: Initialize the class by opening a random access file with write permission.
// ORIGINAL: 0x006283C0 ?init@Filemap@@QAEHPAD@Z 0x006283C0-0x006283D5 BYTE_EXACT
// symbol    ?init@Filemap@@QAEPAV1@PBD@Z
// size      21 bytes
// prototype int (__thiscall ?init@Filemap@@QAEHPAD@Z)(Filemap* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00628540
Return Value: Pointer to initialized class
Status: Complete
*/
Filemap *Filemap::init(LPCSTR file_name) {
    open(file_name, false);
    return this;
}

/*
Purpose: Open a file with read permission. The boolean parameter toggles if the file is accessed 
         sequentially or randomly.
// ORIGINAL: 0x00628430 ?open_read@Filemap@@QAEHPADH@Z 0x00628430-0x00628539 BYTE_EXACT
// symbol    ?open_read@Filemap@@QAEPAXPBDH@Z
// size      265 bytes
// prototype int (__thiscall ?open_read@Filemap@@QAEHPADH@Z)(Filemap* this, int8*, int)
// callers   8   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006005D0
// indirect  0x00628452 0x00628462 0x00628474 0x0062849D 0x006284AC 0x006284C1 0x006284D5 0x006284EC 0x00628500 0x00628512 0x00628528
Return Value: Pointer to the mapped file or NULL on error
Status: Complete
*/
LPVOID Filemap::open_read(LPCSTR file_name, BOOL is_sequential) {
    // THE FLAGS ARE COMPUTED FIRST, before the close: the image opens with
    // `neg edi; sbb edi, edi; and edi, 0xF8000000; add edi, 0x10000080`,
    // which is this ternary folded to a pair of constants and kept in a
    // callee-saved register across everything that follows. Left inside the
    // CreateFileA argument list it is computed at the call instead.
    // THE ATTRIBUTE IS INSIDE BOTH ARMS, not OR-ed on afterwards. VC6 folds
    // the two whole constants - 0x08000080 and 0x10000080 - into one `add`
    // over the sbb mask; OR-ed on outside it emits `add edi, 0x10000000`
    // followed by `or edi, 0x80`, one instruction more.
    const DWORD flags = is_sequential
        ? (FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL)
        : (FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL);
    close();
    LPCSTR file_paths = filefind_get(file_name);
    if (!file_paths) {
        file_paths = file_name;
    }
    file_ = CreateFileA(file_paths, GENERIC_READ, 0, NULL, OPEN_EXISTING, flags,
                        NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError(); // failed to read file
        return NULL;
    }
    // TWO CLOSES, ONE PER FAILURE, and the image inlines both: the
    // UnmapViewOfFile chain appears once after CreateFileMapping fails and
    // again after MapViewOfFile does. Sharing one at the bottom is a jump
    // where the image has a copy.
    file_map_ = CreateFileMapping(file_, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!file_map_) {
        close();
        return NULL;
    }
    map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_READ, 0, 0, 0);
    if (!map_view_addr_) {
        close();
        return NULL;
    }
    file_size_ = GetFileSize(file_, NULL);
    return map_view_addr_;
}

/*
Purpose: Open a file with write permission. The boolean parameter toggles if the file is accessed
         sequentially or randomly.
// ORIGINAL: 0x00628540 ?open@Filemap@@QAEHPADH@Z 0x00628540-0x0062864C BYTE_EXACT
// symbol    ?open@Filemap@@QAEPAXPBDH@Z
// size      268 bytes
// prototype int (__thiscall ?open@Filemap@@QAEHPADH@Z)(Filemap* this, int8*, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006005D0
// indirect  0x00628562 0x00628572 0x00628584 0x006285AD 0x006285BC 0x006285D1 0x006285E5 0x006285FF 0x00628613 0x00628625 0x0062863B
Return Value: Pointer to the mapped file or NULL on error
Status: Complete
*/
LPVOID Filemap::open(LPCSTR file_name, BOOL is_sequential) {
    // THE SAME THREE SHAPES AS `open_read`, which they share to the byte:
    // the flags computed first with the attribute folded into both arms, and
    // one inlined `close` per failure rather than a shared one at the end.
    const DWORD flags = is_sequential
        ? (FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL)
        : (FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL);
    close();
    LPCSTR file_paths = filefind_get(file_name);
    if (!file_paths) {
        file_paths = file_name;
    }
    file_ = CreateFileA(file_paths, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, flags, NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError(); // failed to read file
        return NULL;
    }
    file_map_ = CreateFileMapping(file_, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!file_map_) {
        close();
        return NULL;
    }
    map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!map_view_addr_) {
        close();
        return NULL;
    }
    file_size_ = GetFileSize(file_, NULL);
    return map_view_addr_;
}

/*
Purpose: Create a new file or overwrite an existing one with the specified size filled with zeros.
// ORIGINAL: 0x00628650 ?create@Filemap@@QAEHPADKH@Z 0x00628650-0x006287B3 BYTE_EXACT
// symbol    ?create@Filemap@@QAEPAXPBDIH@Z
// size      355 bytes
// prototype int (__thiscall ?create@Filemap@@QAEHPADKH@Z)(Filemap* this, int8* lpFileName, ULONG lDistanceToMove, int)
// callers   6   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x00628673 0x00628683 0x00628695 0x006286B5 0x006286C3 0x006286DC 0x006286EA 0x00628703 0x00628710 0x0062871E 0x00628739 0x0062874D 0x00628767 0x0062877B 0x0062878D
Return Value: Pointer to the mapped file or NULL on error
Status: Complete
*/
LPVOID Filemap::create(LPCSTR file_name, uint32_t size, BOOL is_sequential) {
    // THE SAME FLAGS SHAPE AS open_read/open, computed first before close().
    const DWORD flags = is_sequential
        ? (FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL)
        : (FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL);
    close();
    file_size_ = size;
    file_ = CreateFileA(file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, flags, NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError();
        return NULL;
    }
    // AN EARLY OUT AT EACH FAILURE, not a single trailing cleanup: the image
    // inlines `close()` separately at every one of these four points (VC6
    // proves the `map_view_addr_` check dead only at the last of them, where
    // it was just stored 0 by MapViewOfFile's own failure in the same basic
    // block, and folds the shared CloseHandle(file_map_)/CloseHandle(file_)
    // tail across all four via cross-jumping).
    if (SetFilePointer(file_, size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        close();
        return NULL;
    }
    SetEndOfFile(file_);
    if (SetFilePointer(file_, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        close();
        return NULL;
    }
    file_map_ = CreateFileMapping(file_, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!file_map_) {
        close();
        return NULL;
    }
    map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!map_view_addr_) {
        close();
        return NULL;
    }
    memset(map_view_addr_, 0, size);
    return map_view_addr_;
}

/*
Purpose: Close the map and all handles to the file.
// ORIGINAL: 0x006287C0 ?close@Filemap@@QAEXXZ 0x006287C0-0x00628803 BYTE_EXACT
// size      67 bytes
// prototype void (__thiscall ?close@Filemap@@QAEXXZ)(Filemap* this)
// callers   10   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006287CB 0x006287E5 0x006287F7
Return Value: n/a
Status: Complete
*/
// body src/filemap.h

/*
Purpose: Close and set the end of the file. This can be used to truncate existing files. It also 
         assumes the file has write permission.
// ORIGINAL: 0x00628810 ?close@Filemap@@QAEXPAE@Z 0x00628810-0x006288C4 BYTE_EXACT
// symbol    ?close@Filemap@@QAEXPAX@Z
// size      180 bytes
// prototype void (__thiscall ?close@Filemap@@QAEXPAE@Z)(Filemap* this, unsigned int8*)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00628826 0x00628849 0x0062885F 0x0062886B 0x0062887E 0x0062888E 0x0062889C 0x006288AB 0x006288B9
Return Value: n/a
Status: Complete
*/
void Filemap::close(LPVOID new_addr) {
    // AN EARLY OUT, NOT AN `if`/`else`. The image's `jae` at 0x0062881D
    // jumps TO the truncation work and falls into the plain close, which is
    // what `if (new_addr < map_view_addr_) { close(); return; }` emits;
    // written the other way round VC6 lays the body inline and the close
    // after it, and every branch in between inverts.
    // LEVER: `new_size` NOT scoped inside a trailing `{ }` block, and the
    // SetFilePointer failure returns EARLY (`close(); return;`) instead of
    // falling through to one `close()` call at the very end. Both spellings
    // are semantically identical, but the block scope stopped VC6 from
    // folding this `close()`'s tail (CloseHandle(file_map_)/CloseHandle
    // (file_)) into the SAME physical code the later failure branch also
    // reaches by `jmp` - the image has ONE copy of that tail, this tree had
    // two before the reshape.
    if (new_addr < map_view_addr_) {
        close();
        return;
    }
    LONG new_size = LONG(new_addr) - LONG(map_view_addr_);
    if (map_view_addr_) {
        UnmapViewOfFile(map_view_addr_);
        map_view_addr_ = 0;
    }
    if (file_map_) {
        CloseHandle(file_map_);
        file_map_ = 0;
    }
    if (SetFilePointer(file_, new_size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        close();
        return;
    }
    SetEndOfFile(file_);
    if (file_) {
        CloseHandle(file_);
        file_ = 0;
    }
}

// ---------------------------------------------------------------------------
// Merged from the retired src/filemap_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00628380 ??0Filemap@@QAE@XZ 0x00628380-0x00628397 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ??0Filemap@@QAE@XZ)(Filemap* this)
// callers   15   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Filemap::Filemap()
    : map_view_addr_(nullptr),
      file_(INVALID_HANDLE_VALUE),
      file_map_(nullptr) {
}

/*
// ORIGINAL: 0x006283E0 ??1Filemap@@QAE@XZ 0x006283E0-0x00628423 BYTE_EXACT
// size      67 bytes
// prototype void (__thiscall ??1Filemap@@QAE@XZ)(Filemap* this)
// callers   15   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x006283EB 0x00628405 0x00628417
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
Filemap::~Filemap() {
    if (map_view_addr_) {
        UnmapViewOfFile(map_view_addr_);
        map_view_addr_ = nullptr;
    }
    if (file_map_) {
        CloseHandle(file_map_);
        file_map_ = nullptr;
    }
    if (file_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_);
        file_ = nullptr;
    }
}
