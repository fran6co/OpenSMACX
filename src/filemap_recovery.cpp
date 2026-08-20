#include "stdafx.h"
#include "filemap.h"

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
