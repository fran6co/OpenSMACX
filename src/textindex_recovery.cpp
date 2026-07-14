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
}

void text_clear_index_source(TextIndex *indexes) {
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (indexes[i].section_count_ != 0) {
            indexes[i].heap_.shutdown();
            indexes[i].section_count_ = 0;
        }
    }
}
