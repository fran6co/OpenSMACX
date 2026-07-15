#include "stdafx.h"
#include "textindex.h"

TextIndex::TextIndex() : section_count_(0) {
    file_name_[0] = 0;
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

TextIndex::~TextIndex() {
    section_count_ = 0;
    file_name_[0] = 0;
    heap_.shutdown();
}

void text_clear_index_source(TextIndex *indexes) {
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (indexes[i].section_count_ != 0) {
            indexes[i].heap_.shutdown();
            indexes[i].section_count_ = 0;
        }
    }
}

void __cdecl text_make_index(LPCSTR source_txt) { // 005FE1F0
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (!TxtIndex[i].get_count()) {
            TxtIndex[i].make_index(source_txt);
            break;
        }
    }
}

int __cdecl text_search_index(LPCSTR source_txt, LPCSTR section_txt) { // 005FE230
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (TxtIndex[i].get_count()) {
            const int address = TxtIndex[i].search_index(source_txt, section_txt);
            if (address >= 0) {
                return address;
            }
        }
    }
    return -1;
}
