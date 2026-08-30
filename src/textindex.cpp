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
#include "textindex.h"
#include "general.h"
#include "filemap.h"

/*
Purpose: Read the specified file and create an index of section headers ("#EXAMPLE"). Store the file 
         offset of each section for improved search time.
// ORIGINAL: 0x005FDF80 ?make_index@TextIndex@@QAEXPAD@Z 0x005FDF80-0x005FE11B;0x00662D40-0x00662D52
// symbol    ?make_index@TextIndex@@QAEXPBD@Z
// size      429 bytes
// prototype void (__thiscall ?make_index@TextIndex@@QAEXPAD@Z)(TextIndex* this, int8*)
// callers   1   call targets   17
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4580 0x005D45E0 0x005D4620 0x005D4680 0x006007B0 0x00600820 0x00628380 0x006283E0 0x00628430 0x006287C0 0x006453E0 0x00645460 0x00645470 0x00645930 0x00645DD0 0x0064697A 0x006473F0
// indirect  0x005FE007
// TRIED: the image opens a real SEH frame (`mov eax, fs:[0]; push eax;
//            mov fs:[0], esp` before `sub esp, 0x218`, and the second span
//            0x00662D40-0x00662D52 is its unwind handler) - expected, since
//            the local `Filemap txt_file_map` has a real destructor. This
//            tree's compile never emits one at any of the 10 flag sets in
//            FLAG_SETS (best 0.892 similar, /O2 /Oi- /GX, no /Oy-). Not
//            resolved this pass - the frame's absence, not the body logic,
//            is the ceiling.
Return Value: n/a
Status: Complete
*/
void TextIndex::make_index(LPCSTR source_txt) {
    Filemap txt_file_map;
    shutdown();
    strcpy_s(file_name_, 256, source_txt);
    if (!strchr(file_name_, '.')) {
        strcat_s(file_name_, 256, ".txt"); // append extension if missing
    }
    LPVOID txt_addr_base = txt_file_map.open_read(file_name_, false);
    if (txt_addr_base) {
        LPVOID seek_addr = txt_addr_base;
        LPVOID eof_addr = LPVOID(uint32_t(txt_addr_base) + txt_file_map.get_size());
        heap_.init(0x8000); // maximum amount of memory per section index
        while (seek_addr < eof_addr) {
            LPVOID found_addr = memchr(seek_addr, '\n', uint32_t(eof_addr) - uint32_t(seek_addr));
            if (!found_addr) {
                break; // invalid file, no newlines
            }
            char parse_buffer[512]; // maximum line length
            memcpy_s(parse_buffer, 512, seek_addr, uint32_t(found_addr) - uint32_t(seek_addr) + 1);
            kill_nl(parse_buffer); // bug fix; Filemap reads in raw file including carriage return
            purge_spaces(parse_buffer);
            if (parse_buffer[0] == '#' && isupper(parse_buffer[1])) {
                size_t len = strlen(&parse_buffer[1]) + 1;
                LPVOID store_addr = heap_.get(len + 4); // section name + address
                *(LPDWORD)store_addr = uint32_t(seek_addr) - uint32_t(txt_addr_base); // file offset
                strcpy_s(LPSTR(store_addr) + 4, len, &parse_buffer[1]);
                section_count_++;
            }
            seek_addr = LPVOID(uint32_t(found_addr) + 1);
        }
        heap_.squeeze(true);
    }
}

/*
Purpose: Search the source text file for a section header.
// ORIGINAL: 0x005FE120 ?search_index@TextIndex@@QAEHPAD0@Z 0x005FE120-0x005FE1E1 BYTE_EXACT
// symbol    ?search_index@TextIndex@@QAEHPBD0@Z
// size      193 bytes
// prototype int (__thiscall ?search_index@TextIndex@@QAEHPAD0@Z)(TextIndex* this, int8*, int8*)
// callers   2   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645460 0x00645470 0x00645DD0 0x0064FD20
Return Value: File offset if found, otherwise -1
Status: Complete
*/
int TextIndex::search_index(LPCSTR source_txt, LPCSTR section_txt) {
    // WRONG CALLEE: the image calls plain strcpy/strchr/strcat (0x00645460,
    // 0x00645DD0, 0x00645470), not the bounded `_s` variants - an unbounded
    // copy into a 512-byte stack buffer, not MAX_PATH(260). BUG IN THE
    // ORIGINAL, preserved.
    char file_name_check[512];
    strcpy(file_name_check, source_txt);
    if (!strchr(file_name_check, '.')) {
        strcat(file_name_check, ".txt"); // append extension if missing
    }
    if (!_stricmp(file_name_check, file_name_)) {
        if (section_txt[0] == '#') {
            section_txt++;
        }
        // NO UPFRONT GUARD: the image never tests section_count_ before the
        // first comparison. `record_start` mirrors the image's own EDI - a
        // pointer to the record's leading offset field, kept alongside
        // `cmp_addr` (its ESI) rather than recomputed as `cmp_addr - 4` at
        // the return - and the advance to the NEXT record happens only
        // after the decrement-and-test, not before it.
        int i = section_count_;
        LPSTR cmp_addr = LPSTR(heap_.get_base());
        LPSTR record_start = cmp_addr;
        cmp_addr += 4;
        for (;;) {
            if (!_stricmp(section_txt, cmp_addr)) {
                return *LPDWORD(record_start);
            }
            // PRE-TEST, THEN A FINAL SKIP: the image tests `*cmp_addr`
            // before ever advancing, walking to the terminator with
            // `++cmp_addr` inside the loop and one more `++cmp_addr` after
            // it, rather than testing the post-incremented read each time.
            while (*cmp_addr) {
                ++cmp_addr;
            }
            ++cmp_addr;
            if (--i == 0) {
                break;
            }
            record_start = cmp_addr;
            cmp_addr += 4;
        }
    }
    return -1;
}

// global

// ORIGINAL: 0x005FE270 ?text_clear_index@@YAXXZ 0x005FE270-0x005FE299 BYTE_EXACT
// size      41 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_clear_index() {
    text_clear_index_source(TxtIndex);
}

// ---------------------------------------------------------------------------
// Merged from the retired src/textindex_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005FDF40 ??0TextIndex@@QAE@XZ 0x005FDF40-0x005FDF5F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ??0TextIndex@@QAE@XZ)(TextIndex* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4560
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
TextIndex::TextIndex() {
    // NOT AN INITIALISER LIST. `section_count_` is declared BEFORE
    // `heap_`, so initialising it in the list puts its store ahead
    // of the member's constructor call; the image constructs the
    // heap first and writes the fields after.
    section_count_ = 0;
    file_name_[0] = 0;
}

/*
// ORIGINAL: 0x005FDF60 ??1TextIndex@@QAE@XZ 0x005FDF60-0x005FDF78 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ??1TextIndex@@QAE@XZ)(TextIndex* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
TextIndex::~TextIndex() {
    // NO EXPLICIT SHUTDOWN. The member's own destructor runs after this
    // body and the image's tail is exactly that call; spelling it here as
    // well emitted the shutdown twice.
    section_count_ = 0;
    file_name_[0] = 0;
}

// ORIGINAL: 0x005FE1F0 ?text_make_index@@YAXPAD@Z 0x005FE1F0-0x005FE228 BYTE_EXACT
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

// ORIGINAL: 0x005FE230 ?text_search_index@@YAHPAD0@Z 0x005FE230-0x005FE26C BYTE_EXACT
// body      src/textindex.h
// symbol    ?text_search_index@@YAHPBD0@Z
// size      60 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE120
//
// MOVED TO textindex.h, INLINE: `Text::open` (0x005FDA00) carries this whole
// loop rather than calling it, same reason as `text_get_source` and
// neighbours in text.h. VC6 still emits this address as its own COMDAT for
// the claim above, which is what the comparison pulls out.

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Was `TextIndex TxtIndexGlobal[4]` beside textindex.h's
// `TextIndex *const TxtIndex = (TextIndex *)0x009B7D08` binding; the array is
// that binding's storage, so the object carries the public name now. See
// textindex.h.
TextIndex TxtIndex[MaxTextIndexNum];  // 0x009B7D08, 0x118 stride
