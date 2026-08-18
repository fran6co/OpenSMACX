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
// ORIGINAL: 0x00628430 ?open_read@Filemap@@QAEHPADH@Z 0x00628430-0x00628539
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
    close();
    LPCSTR file_paths = filefind_get(file_name);
    if (!file_paths) {
        file_paths = file_name;
    }
    file_ = CreateFileA(file_paths, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
        | (is_sequential ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS), NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError(); // failed to read file
        return NULL;
    }
    file_map_ = CreateFileMapping(file_, NULL, PAGE_READONLY, 0, 0, NULL);
    if (file_map_) {
        map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_READ, 0, 0, 0);
        if (map_view_addr_) {
            file_size_ = GetFileSize(file_, NULL);
            return map_view_addr_;
        }
    }
    close(); // clear everything on error
    return NULL;
}

/*
Purpose: Open a file with write permission. The boolean parameter toggles if the file is accessed
         sequentially or randomly.
// ORIGINAL: 0x00628540 ?open@Filemap@@QAEHPADH@Z 0x00628540-0x0062864C
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
    close();
    LPCSTR file_paths = filefind_get(file_name);
    if (!file_paths) {
        file_paths = file_name;
    }
    file_ = CreateFileA(file_paths, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL |
        (is_sequential ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS), NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError(); // failed to read file
        return NULL;
    }
    file_map_ = CreateFileMapping(file_, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (file_map_) {
        map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (map_view_addr_) {
            file_size_ = GetFileSize(file_, NULL);
            return map_view_addr_;
        }
    }
    close(); // clear everything on error
    return NULL;
}

/*
Purpose: Create a new file or overwrite an existing one with the specified size filled with zeros.
// ORIGINAL: 0x00628650 ?create@Filemap@@QAEHPADKH@Z 0x00628650-0x006287B3
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
    close();
    file_size_ = size;
    file_ = CreateFileA(file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL |
        (is_sequential ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS), NULL);
    if (file_ == INVALID_HANDLE_VALUE) {
        GetLastError();
        return NULL;
    }
    if (SetFilePointer(file_, size, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
        SetEndOfFile(file_);
        if (SetFilePointer(file_, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
            file_map_ = CreateFileMapping(file_, NULL, PAGE_READWRITE, 0, 0, NULL);
            if (file_map_) {
                map_view_addr_ = MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
                if (map_view_addr_) {
                    ZeroMemory(map_view_addr_, size);
                    return map_view_addr_;
                }
            }
        }
    }
    close(); // clear everything on error
    return NULL;
}

/*
Purpose: Close the map and all handles to the file.
// ORIGINAL: 0x006287C0 ?close@Filemap@@QAEXXZ 0x006287C0-0x00628803
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
void Filemap::close() {
    if (map_view_addr_) {
        UnmapViewOfFile(map_view_addr_);
        map_view_addr_ = 0;
    }
    if (file_map_) {
        CloseHandle(file_map_);
        file_map_ = 0;
    }
    if (file_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_);
        file_ = 0;
    }
    file_size_ = 0;
}

/*
Purpose: Close and set the end of the file. This can be used to truncate existing files. It also 
         assumes the file has write permission.
// ORIGINAL: 0x00628810 ?close@Filemap@@QAEXPAE@Z 0x00628810-0x006288C4
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
    if (new_addr >= map_view_addr_) {
        LONG new_size = LONG(new_addr) - LONG(map_view_addr_);
        if (map_view_addr_) {
            UnmapViewOfFile(map_view_addr_);
            map_view_addr_ = 0;
        }
        if (file_map_) {
            CloseHandle(file_map_);
            file_map_ = 0;
        }
        if (SetFilePointer(file_, new_size, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
            SetEndOfFile(file_);
            if (file_) {
                CloseHandle(file_);
                file_ = 0;
            }
            return;
        }
    }
    close(); // clear everything on error
}
