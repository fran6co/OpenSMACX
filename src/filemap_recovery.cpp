#include "stdafx.h"
#include "filemap.h"

Filemap::Filemap()
    : map_view_addr_(nullptr),
      file_(INVALID_HANDLE_VALUE),
      file_map_(nullptr) {
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

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
#if defined(__GNUC__) && defined(__i386__)
        *reinterpret_cast<HANDLE volatile *>(&file_) = nullptr;
#else
        file_ = nullptr;
#endif
    }
}
