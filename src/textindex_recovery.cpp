#include "stdafx.h"
#include "textindex.h"

/*
// ORIGINAL: 0x005FDF40 ??0TextIndex@@QAE@XZ 0x005FDF40-0x005FDF5F
// size      31 bytes
// prototype void (__thiscall ??0TextIndex@@QAE@XZ)(TextIndex* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4560
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
TextIndex::TextIndex() : section_count_(0) {
    file_name_[0] = 0;
}

/*
// ORIGINAL: 0x005FDF60 ??1TextIndex@@QAE@XZ 0x005FDF60-0x005FDF78
// size      24 bytes
// prototype void (__thiscall ??1TextIndex@@QAE@XZ)(TextIndex* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
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

// ORIGINAL: 0x005FE1F0 ?text_make_index@@YAXPAD@Z 0x005FE1F0-0x005FE228
// symbol    ?text_make_index@@YAXPBD@Z
// size      56 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FDF80
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_make_index(LPCSTR source_txt) {
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (!TxtIndex[i].get_count()) {
            TxtIndex[i].make_index(source_txt);
            break;
        }
    }
}

// ORIGINAL: 0x005FE230 ?text_search_index@@YAHPAD0@Z 0x005FE230-0x005FE26C
// symbol    ?text_search_index@@YAHPBD0@Z
// size      60 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE120
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_search_index(LPCSTR source_txt, LPCSTR section_txt) {
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
