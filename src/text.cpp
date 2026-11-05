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

#include "text.h"
#include "general.h"
#include "log.h"
#include "strings.h"
#include "textindex.h"
#include "filemap.h"
#include "popup.h"
#include "game.h"
#include "faction.h"
#include "console.h"

// Pinned operands share one extension object and identify two distinct modes.
// These reconstruction names and array bounds do not establish original declarations.
// RECOVERY_OBJECT: v1 0x00696E68 ?TextFileExtension@@3PADA
char TextFileExtension[] = ".txt";
namespace {
// RECOVERY_OBJECT: v1 0x00696E7C ?text_open_named_mode@?%src/text.cpp@@3PADA
char text_open_named_mode[] = "rt";
// RECOVERY_OBJECT: v1 0x00696E80 ?text_open_reopen_mode@?%src/text.cpp@@3PADA
char text_open_reopen_mode[] = "rt";
// RECOVERY_OBJECT: v1 0x00696E84 ?text_open_section_prefix@?%src/text.cpp@@3PADA
char text_open_section_prefix[] = "#";
// RECOVERY_OBJECT: v1 0x00696E88 ?text_open_seek_log_key@?%src/text.cpp@@3PADA
char text_open_seek_log_key[] = "Seeking to";
}

/*
Purpose: Initialize the class instance.
// ORIGINAL: 0x005FD8D0 ?init@Text@@QAEHH@Z 0x005FD8D0-0x005FD969 BYTE_EXACT
// LEVER: BYTE_EXACT 39/39, on two measured changes. (1) `shutdown()` was a CALL and the image WRITES IT OUT - all three guards, two `free`s and an `fclose` - which call_diff caught as 3 calls against the image's 5. Moved to `text_shutdown_source()` in text.h, the same shape the text_*_source family already uses, and reached from both here and Text::shutdown(); 0x005FD970 keeps its own BYTE_EXACT claim. (2) THE SECOND FAILURE IS BRANCHLESS. The image tests buffer_get_ with a real `test/jne` and an early `mov eax, 4`, but ends `neg eax; sbb eax, eax; and al, 0xfc; add eax, 4` - VC6's lowering of `return buffer_item_ ? 0 : 4;`. The second `if (!x) return 4; return 0;` compiles the branch instead. Only the LAST one is the ternary.
// symbol    ?init@Text@@QAEHI@Z
// size      153 bytes
// prototype int (__thiscall ?init@Text@@QAEHH@Z)(_Text* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00645598
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Text::init(size_t size) {
    text_shutdown_source(this);
    buffer_get_ = (LPSTR)mem_get(size);
    if (!buffer_get_) {
        return 4;
    }
    buffer_item_ = (LPSTR)mem_get(size);
    return buffer_item_ ? 0 : 4;
}

/*
Purpose: Shutdown the class instance.
// ORIGINAL: 0x005FD970 ?shutdown@Text@@QAEXXZ 0x005FD970-0x005FD9CC BYTE_EXACT
// size      92 bytes
// prototype void (__thiscall ?shutdown@Text@@QAEXXZ)(_Text* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645598
Return Value: n/a
Status: Complete
*/
void Text::shutdown() {
    // ONE COPY, in text.h. close() (0x005FD9D0, BYTE_EXACT as its own
    // out-of-line function) is written out rather than called - the image
    // does that here and again inside init() - and this used to be a
    // hand-inlined second copy of the same three guards. Both call sites now
    // reach text_shutdown_source(); measured BYTE_EXACT 25/25 either way.
    text_shutdown_source(this);
}

/*
Purpose: If open, close the text file.
// ORIGINAL: 0x005FD9D0 ?close@Text@@QAEXXZ 0x005FD9D0-0x005FD9F2 BYTE_EXACT
// size      34 bytes
// prototype void (__thiscall ?close@Text@@QAEXXZ)(_Text* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598
Return Value: n/a
Status: Complete
*/
void Text::close() {
    if (text_file_) {
        fclose(text_file_);
        text_file_ = 0;
    }
}

/*
Purpose: Open the specified text file and copy the section into the buffer for parsing.
// ORIGINAL: 0x005FDA00 ?open@Text@@QAEHPADPAD@Z 0x005FDA00-0x005FDC04 BYTE_EXACT
// symbol    ?open@Text@@QAEHPBD0@Z
// size      516 bytes
// prototype int (__thiscall ?open@Text@@QAEHPADPAD@Z)(_Text* this, int8* srcFileID, int8* sectionID)
// callers   2   call targets   13
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FE120 0x006007B0 0x00600820 0x00626250 0x00634BB0 0x00645460 0x00645470 0x00645598 0x00645DD0 0x0064726A 0x006472CC 0x00647330 0x0064FD20
// TRIED: std::string for sect_header - it pulls an SEH frame (`mov eax,
//            fs:[0]`) the image's `sub esp, 0x54` prologue does not have;
//            replaced with a plain char buffer built by strcpy_s/strcat_s
//            (fixed at 80 bytes - matches the image's `sub esp, 0x54` frame
//            size once text_search_index is inlined, see textindex.h).
//            text_search_index() called out-of-line - the image carries the
//            whole loop inline (only 0x005FE120 shows in this function's own
//            call list); moved to textindex.h as MEASURED inline.
//            Remaining divergence: the image caches a zeroed `edi` once at
//            entry and reuses it as both the FALSE constant and later
//            accumulators (`cmp eax, edi` instead of `test eax, eax`, `mov
//            [esp+0x10], edi` instead of a literal 0 store) - a register-
//            allocation choice tied to the whole function's shape that
//            FALSE/NULL-spelling variants (BOOL/TRUE/FALSE vs bool/true/
//            false, `!= NULL` vs implicit) did not reproduce. Best reached:
//            0.848 similar, 7/170 raw instructions at /O2 /Oi- /Gy /GR- /GX.
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL Text::open(LPCSTR src_file_id, LPCSTR section_id) {
    BOOL is_file_open = false; // open, possibly tracker for reading through whole file?
    if (src_file_id) {
        strcpy(file_name_, src_file_id);
        if (!strchr(file_name_, '.')) {
            strcat(file_name_, TextFileExtension);
        }
        close();
        text_file_ = env_open(file_name_, text_open_named_mode);
        if (!text_file_) {
            return true;
        }
        strcpy(file_path_, FilefindPath.last_path);
    } else if (text_file_) {
        is_file_open = true;
    } else {
        text_file_ = env_open(file_name_, text_open_reopen_mode);
        if (!text_file_) {
            return true;
        }
        strcpy(file_path_, FilefindPath.last_path);
    }
    if (!section_id) {
        return false;
    }
    // NOT std::string: the image builds this with plain strcpy/strcat, and
    // std::string's constructor/destructor pull in an SEH frame (`mov eax,
    // fs:[0]` in the prologue) the image's `sub esp, 0x54` does not have.
    char sect_header[80];
    strcpy(sect_header, text_open_section_prefix);
    strcat(sect_header, section_id);
    int seek_addr = text_search_index(file_name_, section_id);
    if (seek_addr >= 0) {
        log_say(text_open_seek_log_key, section_id, seek_addr, 0, 0);
        fseek(text_file_, seek_addr, SEEK_SET);
        is_file_open = true;
    }
    do {
        if (feof(text_file_)) {
            if (is_file_open) {
                is_file_open = false;
                rewind(text_file_);
            } else {
                close();
                return true;
            }
        }
        if (!fgets(buffer_get_, 511, text_file_)) {
            close();
            return true;
        }
        kill_lf(buffer_get_);
        purge_spaces(buffer_get_);
    } while (_stricmp(sect_header, buffer_get_));
    current_pos_ = buffer_get_;
    return false;
}

/*
Purpose: Parse text from the opened file until a newline is reached. Copy this text into the buffer.
// ORIGINAL: 0x005FDC10 ?get@Text@@QAEHXZ 0x005FDC10-0x005FDC7E BYTE_EXACT
// LEVER: BYTE_EXACT 34/34, from 17/34, on BRANCH SHAPE alone - same instruction count either way. The body was `if (fgets(...)) { work } else { buffer_get_[0] = 0; } return buffer_get_;`, one shared tail; the image falls through to a separate `return` after EACH test. Written as the two guard clauses text_get_source() in text.h already uses - which is why the free `text_get()` wrapper over the same helper was byte-exact and this was not - it reproduces exactly. Delegating to that one copy rather than repeating it scores identically (34/34 both ways), so the shared definition wins.
// symbol    ?get@Text@@QAEPADXZ
// size      110 bytes
// prototype int (__thiscall ?get@Text@@QAEHXZ)(_Text* this)
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x0064726A
Return Value: Pointer to string
Status: Complete
*/
LPSTR Text::get() {
    return text_get_source(this);
}

/*
Purpose: Get the string and put it into the string table.
// ORIGINAL: 0x005FDC80 ?string@Text@@QAEXXZ 0x005FDC80-0x005FDD0F BYTE_EXACT
// LEVER: BYTE_EXACT 43/43, from 5/43. The previous body had the right SHAPE - get() hand-inlined, put() called on all three exit paths, which the image does - but wrote the three put() calls out itself, and that put a literal in the first one: `put(nullptr)` compiles `push 0` where the image has `xor eax, eax; push eax`. Feeding put() the RETURN VALUE of the inlined get() - `strings->put(text_get_source(text))`, which is exactly text_string_source() in text.h - makes the null a value in a register, and VC6 duplicates the shared `call put; pop esi; ret` tail into each branch by itself. Delegating and spelling it out here score identically (43/43 both), so the one copy in the header wins.
// symbol    ?string@Text@@QAEPADXZ
// size      143 bytes
// prototype void (__thiscall ?string@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x00616970 0x0064726A
Return Value: Pointer to string table
Status: Complete
*/
LPSTR Text::string() {
    return text_string_source(this, &StringTable);
}

/*
Purpose: Parse the current item into the string buffer.
// ORIGINAL: 0x005FDD10 ?item@Text@@QAEXXZ 0x005FDD10-0x005FDD72 BYTE_EXACT
// symbol    ?item@Text@@QAEPADXZ
// size      98 bytes
// prototype void (__thiscall ?item@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0
Return Value: Pointer to string buffer
Status: Complete
*/
LPSTR Text::item() {
    LPSTR parse = buffer_item_;
    while (*current_pos_ != 0 && *current_pos_ != ',') {
        *parse++ = *current_pos_++;
    }
    *parse = 0;
    if (*current_pos_) {
        *current_pos_++;
    }
    purge_spaces(buffer_item_);
    return buffer_item_;
}

/*
Purpose: Parse the current string item into the buffer and add it to the string table.
// ORIGINAL: 0x005FDD80 ?item_string@Text@@QAEXXZ 0x005FDD80-0x005FDDED BYTE_EXACT
// LEVER: BYTE_EXACT 33/33, from 0/33 and 5 compiled instructions. The body called `item()`, which is defined OUT OF LINE in this .cpp, so the whole scanner loop became a `call`; the image writes it out here and calls only its own two helpers. `text_item_source(this)` - the same logic as an `inline` in text.h - reproduces it exactly. Sibling of the already-byte-exact free `text_item_*` wrappers, which reach it the same way.
// symbol    ?item_string@Text@@QAEPADXZ
// size      109 bytes
// prototype void (__thiscall ?item_string@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00616970
Return Value: Pointer to string table
Status: Complete
*/
LPSTR Text::item_string() {
    return StringTable.put(text_item_source(this));
}

/*
Purpose: Parse the current number item.
// ORIGINAL: 0x005FDDF0 ?item_number@Text@@QAEXXZ 0x005FDDF0-0x005FDE58 BYTE_EXACT
// LEVER: BYTE_EXACT 32/32, from 0/32 and 5 compiled instructions. The body called `item()`, which is defined OUT OF LINE in this .cpp, so the whole scanner loop became a `call`; the image writes it out here and calls only its own two helpers. `text_item_source(this)` - the same logic as an `inline` in text.h - reproduces it exactly. Sibling of the already-byte-exact free `text_item_*` wrappers, which reach it the same way.
// symbol    ?item_number@Text@@QAEHXZ
// size      104 bytes
// prototype void (__thiscall ?item_number@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00628950
Return Value: Integer value of the number item
Status: Complete
*/
int Text::item_number() {
    return stoi(text_item_source(this));
}

/*
Purpose: Parse the current binary item.
// ORIGINAL: 0x005FDE60 ?item_binary@Text@@QAEXXZ 0x005FDE60-0x005FDEC8 BYTE_EXACT
// LEVER: BYTE_EXACT 32/32, from 0/32 and 5 compiled instructions. The body called `item()`, which is defined OUT OF LINE in this .cpp, so the whole scanner loop became a `call`; the image writes it out here and calls only its own two helpers. `text_item_source(this)` - the same logic as an `inline` in text.h - reproduces it exactly. Sibling of the already-byte-exact free `text_item_*` wrappers, which reach it the same way.
// LEVER: and the btoi has to go through a LOCAL const function pointer, `int(__cdecl *const btoi_fn)(LPCSTR) = btoi;`, exactly as text_item_binary() already documents: btoi is `MEASURED inline` in general.h and VC6 writes it out where the image emits `push eax; call 0x006288D0`. Measured: the plain spelling scores 27/32 and the pointer 32/32. VC6 still folds the pointer to a direct `call rel32`, so this costs no indirection.
// symbol    ?item_binary@Text@@QAEHXZ
// size      104 bytes
// prototype void (__thiscall ?item_binary@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288D0
Return Value: Integer value of the binary item
Status: Complete
*/
int Text::item_binary() {
    int(__cdecl *const btoi_fn)(LPCSTR) = btoi;
    return btoi_fn(text_item_source(this));
}

/*
Purpose: Parse the current hex item.
// ORIGINAL: 0x005FDED0 ?item_hex@Text@@QAEXXZ 0x005FDED0-0x005FDF38 BYTE_EXACT
// LEVER: BYTE_EXACT 32/32, from 0/32 and 5 compiled instructions. The body called `item()`, which is defined OUT OF LINE in this .cpp, so the whole scanner loop became a `call`; the image writes it out here and calls only its own two helpers. `text_item_source(this)` - the same logic as an `inline` in text.h - reproduces it exactly. Sibling of the already-byte-exact free `text_item_*` wrappers, which reach it the same way.
// LEVER: and the htoi has to go through a LOCAL const function pointer, `int(__cdecl *const htoi_fn)(LPCSTR) = htoi;`, exactly as text_item_binary() already documents: htoi is `MEASURED inline` in general.h and VC6 writes it out where the image emits `push eax; call 0x006288F0`. Measured: the plain spelling scores 2/32, 62 instructions and the pointer 32/32. VC6 still folds the pointer to a direct `call rel32`, so this costs no indirection.
// symbol    ?item_hex@Text@@QAEHXZ
// size      104 bytes
// prototype void (__thiscall ?item_hex@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x006288F0
Return Value: Integer value of the hex item
Status: Complete
*/
int Text::item_hex() {
    int(__cdecl *const htoi_fn)(LPCSTR) = htoi;
    return htoi_fn(text_item_source(this));
}

Text::Text(int size) {
    file_name_[0] = 0;
    current_pos_ = 0;
    text_file_ = 0;
    buffer_get_ = 0;
    buffer_item_ = 0;
    buffer_get_ = (LPSTR)mem_get(size);
    if (buffer_get_) {
      buffer_item_ = (LPSTR)mem_get(size);
    }
}
// global
/*
Purpose: The reader every `.txt` the game parses goes through, and the two
         initialisers MSVC emits for it.
// ORIGINAL: 0x005FD400 ??__ETxt@@YAXXZ 0x005FD400-0x005FD456 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682680 selects original 0x5FD400; the fresh physical initializer probe resolves the selected callback to the actual Text global Txt.", "src/text.cpp declares Text Txt(512) as the real storage owner; the compiler-generated callback performs its dynamic construction, rather than a hand-written function body.", "The adjacent original finalizer identity 0x5FD460 is the paired generated atexit cleanup for Txt; the source retains both historical identities and spans."], "reason": "Bind the original Text startup identity to the compiler-generated dynamic initializer for the real namespace-scope Text object."}
// symbol    _$E12
// size      86 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645398
// ORIGINAL: 0x005FD460 ??__FTxt@@YAXXZ 0x005FD460-0x005FD4B5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned PE/CRT evidence pairs original 0x5FD460 with Txt initialization rooted at 0x5FD400; the fresh physical initializer probe resolves the callback to the actual Text global Txt.", "src/text.cpp owns the namespace-scope Text Txt(512); its compiler-generated lifetime callback is the current source provider, with the original identity and span retained."], "reason": "Bind the original Text finalizer identity to the compiler-generated atexit cleanup for the real namespace-scope Text object."}
// symbol    _$E10
// size      85 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645598
Return Value: n/a
Status: Complete

TWO ADDRESSES AND ONE DEFINITION, with no function to hang either on. MSVC
emits a dynamic initialiser and an atexit thunk for every namespace-scope
object with a non-trivial constructor and destructor: `??__ETxt` inlines
`Text::Text(512)` and registers `??__FTxt`, which inlines `~Text()`. The
constructor definition is above `Txt` in this translation unit; the destructor
remains defined in text.h. This line is the only source either comes from.

TRIED: hand-writing them over `Text *const Txt = (Text *)0x009B7BA0`,
           which is what stood here. It could be scored - 0x005FD400 reached
           97.8%, 87 bytes against 86 - and it could not RUN: that address is
           mapped in terranx.exe and in nothing this tree builds, so the
           first `Txt.` faults. It was also two hand-written copies of
           functions the compiler emits anyway.

TRIED: a measurement of either. VC6 names a generated initialiser
               `_$E<n>` and registers it through `.CRT$XCU`, while `??__E`
               and `??__F` are IDA's reconstruction of what they DO; the byte
               match looks its subject up by name. Nothing that is correct
               can carry those symbols. Both were NO_COMPILE before this
               change and had never been scored, so no claim is lost.
*/
// RECOVERY_OBJECT: v1 0x009B7BA0 ?Txt@@3VText@@A
Text Txt(512);  // 0x009B7BA0
// These real pointer globals are initialized directly from Txt, in declaration
// order. The pinned CRT table selects Txt construction at 0x00682680, then
// the get/item pointer initializers at 0x00682684/0x00682688; the source
// declarations below preserve that dependency without hand-written wrappers.
// RECOVERY_OBJECT: v1 0x009B7D00 ?TextBufferGetPtr@@3PADA
LPSTR TextBufferGetPtr = Txt.get_buffer_get();    // 0x009B7D00
// RECOVERY_OBJECT: v1 0x009B7D04 ?TextBufferItemPtr@@3PADA
LPSTR TextBufferItemPtr = Txt.get_buffer_item();  // 0x009B7D04

// ORIGINAL: 0x005FD4C0 ?text_set_get_ptr@@YAXXZ 0x005FD4C0-0x005FD4CB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682684 selects original 0x5FD4C0; the fresh physical initializer probe resolves its callback to the actual TextBufferGetPtr global and its Txt member read.", "src/text.cpp initializes TextBufferGetPtr directly from Txt.get_buffer_get(); this namespace-scope initializer owns the pointer value and runs after Txt construction.", "A whole-repository caller search found no calls to the former explicit text_set_get_ptr wrapper; its original marker, name, span and BYTE_EXACT claim remain on this FILE mapping."], "reason": "Bind the original get-buffer pointer initializer identity to the compiler-generated initialization of the real TextBufferGetPtr global from Txt."}
// symbol    _$E15
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned wrapper

// ORIGINAL: 0x005FD4D0 ?text_set_item_ptr@@YAXXZ 0x005FD4D0-0x005FD4DB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682688 selects original 0x5FD4D0; the fresh physical initializer probe resolves its callback to the actual TextBufferItemPtr global and its Txt member read.", "src/text.cpp initializes TextBufferItemPtr directly from Txt.get_buffer_item(); this namespace-scope initializer owns the pointer value and runs after Txt construction.", "A whole-repository caller search found no calls to the former explicit text_set_item_ptr wrapper; its original marker, name, span and BYTE_EXACT claim remain on this FILE mapping."], "reason": "Bind the original item-buffer pointer initializer identity to the compiler-generated initialization of the real TextBufferItemPtr global from Txt."}
// symbol    _$E18
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned wrapper

// ORIGINAL: 0x005FD530 ?text_close@@YAXXZ 0x005FD530-0x005FD54D BYTE_EXACT
// size      29 bytes
// prototype 
// callers   22   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645598
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_close() { text_close_source(&Txt); }

// ORIGINAL: 0x005FD570 ?text_get@@YAPADXZ 0x005FD570-0x005FD5D2 BYTE_EXACT
// size      98 bytes
// prototype 
// callers   54   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x0064726A
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_get() { return text_get_source(&Txt); }

// ORIGINAL: 0x005FD5E0 ?text_string@@YAPADXZ 0x005FD5E0-0x005FD663 BYTE_EXACT
// size      131 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x00616970 0x0064726A
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_string() { return text_string_source(&Txt, &StringTable); }

// ORIGINAL: 0x005FD670 ?text_item@@YAPADXZ 0x005FD670-0x005FD6C7 BYTE_EXACT
// size      87 bytes
// prototype 
// callers   20   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_item() { return text_item_source(&Txt); }

// ORIGINAL: 0x005FD6D0 ?text_item_string@@YAPADXZ 0x005FD6D0-0x005FD732 BYTE_EXACT
// size      98 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00616970
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_item_string() {
    return text_item_string_source(&Txt, &StringTable);
}

// ORIGINAL: 0x005FD740 ?text_item_number@@YAHXZ 0x005FD740-0x005FD79D BYTE_EXACT
// size      93 bytes
// prototype 
// callers   18   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00628950
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_item_number() { return text_item_number_source(&Txt); }

// ORIGINAL: 0x005FD7A0 ?text_item_binary@@YAHXZ 0x005FD7A0-0x005FD7FD BYTE_EXACT
// LEVER: `btoi(text_item_source(&Txt))` inlines BOTH - two inlines stacked
// size      93 bytes
// prototype 
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288D0
// notes     Staged hybrid export redirect calls the source-owned wrapper
// on this call site folded the second `buffer_item_` read to an absolute
// `[0x15c]` rather than `Txt`'s real address (25/30, plateaued across every
// /O flag set). A function pointer through `btoi` forces the real
// `call 0x6288d0` the image has, while `text_item_source` still inlines -
// 30/30.
int __cdecl text_item_binary() {
    int(__cdecl *const btoi_fn)(LPCSTR) = btoi;
    return btoi_fn(text_item_source(&Txt));
}

// ORIGINAL: 0x005FD800 ?text_item_hex@@YAHXZ 0x005FD800-0x005FD85D BYTE_EXACT
// LEVER: BYTE_EXACT 30/30, from 0/30 and 61 compiled instructions. The scanner half already matched through `text_item_hex_source`; what diverged was `htoi`, which is `MEASURED inline` in general.h and got written out where the image emits `push eax; call 0x006288F0`. A LOCAL const function pointer blocks the inline and still compiles a direct `call rel32` - the idiom text_item_binary() above already uses for btoi, now applied to htoi here and in Text::item_hex().
// size      93 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288F0
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_item_hex() {
    int(__cdecl *const htoi_fn)(LPCSTR) = htoi;
    return htoi_fn(text_item_source(&Txt));
}


// The application text_get_number mapping lives in alpha.cpp; its body is in text.h.









// ---------------------------------------------------------------------------
// Merged from the retired src/text_recovery.cpp on 2026-08-15. It held the
// class's own constructors and destructor while text.cpp held every other
// method, and it reached the private members through a hand-mirrored
// `struct TextState`; both are gone.
// ---------------------------------------------------------------------------
/*
Purpose: Construct an empty Text - no file open, no buffers allocated.
// ORIGINAL: 0x005FD860 ??0Text@@QAE@XZ 0x005FD860-0x005FD87F BYTE_EXACT
// LEVER: field stores in disassembly order - the byte at 0x0 FIRST, then the four dwords - not in declaration order
// size      31 bytes
// prototype Text* (__thiscall ??0Text@@QAE@XZ)(Text* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
Text::Text() {
    // NOT AN INITIALISER LIST: one runs in DECLARATION order, which puts the
    // four pointers before `file_name_[0]`, and the image writes the byte at
    // offset 0 FIRST. Same shape as `Text(int)` above, for the same
    // reason.
    file_name_[0] = 0;
    current_pos_ = nullptr;
    text_file_ = nullptr;
    buffer_get_ = nullptr;
    buffer_item_ = nullptr;
}

/*
Purpose: Construct a Text with `size` bytes of parse buffer.
// ORIGINAL: 0x005FD880 ??0Text@@QAE@H@Z 0x005FD880-0x005FD8CF BYTE_EXACT
// body      src/text.cpp
// size      79 bytes
// prototype Text* (__thiscall ??0Text@@QAE@H@Z)(Text* this, int size)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510

Historical placement note: this body previously lived in text.h, in-class.
`??__ETxt` at 0x005FD400 inlines the constructor's stores and allocations
rather than calling it, so the definition had to be visible where the object
was constructed. The marker was kept here because the native measurement
compiles a `*.cpp` translation unit; VC6 also emits `??0Text@@QAE@H@Z` into
this unit's object as its own COMDAT. The current candidate places the
identical definition earlier in this translation unit, before `Txt`, so it
remains visible to that initializer. Its body-location fact is
coordinator-maintained.

It also used to run its stores in the wrong order: an initialiser list runs
in DECLARATION order - current_pos_, text_file_, buffer_get_, buffer_item_ -
and then the body set file_name_[0], where the image writes the byte FIRST.
Return Value: n/a
Status: Complete
*/


// ORIGINAL: 0x005FD550 ?text_open@@YAHPADPAD@Z 0x005FD550-0x005FD565 BYTE_EXACT
// symbol    ?text_open@@YAHPBD0@Z
// size      21 bytes
// prototype int (__cdecl ?text_open@@YAHPADPAD@Z)(int8* srcFileID, int8* sectionID)
// callers   34   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FDA00
// notes     Staged hybrid export redirect calls the source-owned wrapper
BOOL __cdecl text_open(LPCSTR src_id, LPCSTR section_id) {
    return Txt.open(src_id, section_id);
}

// text_close_source, text_get_source, text_string_source, text_item_source,
// text_item_number_source and text_item_string_source moved to text.h, INLINE:
// the image folds each into every one of its callers rather than sharing one
// out-of-line copy (see LEVER on text_get() and neighbours below). None of
// them carries its own ORIGINAL marker - they are this tree's own shared
// shape for logic the image duplicates at each call site, not an image
// function - so there is nothing to claim here; the header definitions are
// still reached by this unit's own `#include "text.h"`.

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



// TextIndex bodies, lifetime mappings and storage share this module cohort.
// ORIGINAL: 0x005FD4E0 sub_5fd4e0 0x005FD4E0-0x005FD507 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x68268c selects startup 0x5fd4e0..0x5fd507 and registers cleanup 0x5fd510..0x5fd527. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?TxtIndex@@3PAVTextIndex@@A is selected by physical COFF constructor/destructor relocations; current emitted initializer is identified by the adjacent symbol fact. Original array construction and reverse destruction use four TextIndex elements at 9B7D08 with stride118.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original init operation to the genuine C++ lifetime of ?TxtIndex@@3PAVTextIndex@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E23
// ORIGINAL: 0x005FD510 sub_5fd510 0x005FD510-0x005FD527 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x68268c selects startup 0x5fd4e0..0x5fd507 and registers cleanup 0x5fd510..0x5fd527. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?TxtIndex@@3PAVTextIndex@@A is selected by physical COFF constructor/destructor relocations; current emitted finalizer is identified by the adjacent symbol fact. Original array construction and reverse destruction use four TextIndex elements at 9B7D08 with stride118.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original fini operation to the genuine C++ lifetime of ?TxtIndex@@3PAVTextIndex@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E21
#if defined(_MSC_VER) && _MSC_VER <= 1200
// This copy is the large, called form in the original image.  memcpy is
// intentionally not project-wide forced out of line because small copies in
// other translation units are known to inline.
#pragma function(memcpy)
#endif

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
    // Original 0x005FDFB9 calls the shared Heap cleanup at 0x005D4580
    // before clearing the count; the embedded Heap remains alive.
    heap_.reset();
    section_count_ = 0;
    strcpy(file_name_, source_txt);
    if (!strchr(file_name_, '.')) {
        strcat(file_name_, TextFileExtension); // append extension if missing
    }
    LPVOID txt_addr_base = txt_file_map.open_read(file_name_, false);
    if (txt_addr_base) {
        LPVOID seek_addr = txt_addr_base;
        LPVOID eof_addr = LPVOID(uint32_t(txt_addr_base) + GetFileSize(txt_file_map.get_handle(), NULL));
        heap_.init(0x8000); // maximum amount of memory per section index
        while (seek_addr < eof_addr) {
            LPVOID found_addr = memchr(seek_addr, '\n', uint32_t(eof_addr) - uint32_t(seek_addr));
            if (!found_addr) {
                break; // invalid file, no newlines
            }
            memcpy(TextBufferGetPtr, seek_addr, uint32_t(found_addr) - uint32_t(seek_addr) + 1);
            kill_lf(TextBufferGetPtr);
            purge_spaces(TextBufferGetPtr);
            if (TextBufferGetPtr[0] == '#' && isupper(TextBufferGetPtr[1])) {
                size_t len = strlen(&TextBufferGetPtr[1]) + 1;
                LPVOID store_addr = heap_.get(len + 4); // section name + address
                *(LPDWORD)store_addr = uint32_t(seek_addr) - uint32_t(txt_addr_base); // file offset
                strcpy(LPSTR(store_addr) + 4, &TextBufferGetPtr[1]);
                section_count_++;
            }
            seek_addr = LPVOID(uint32_t(found_addr) + 1);
        }
        heap_.squeeze(true);
        txt_file_map.close();
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
        strcat(file_name_check, TextFileExtension); // append extension if missing
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
// LEVER (2026-09-03): taking `~Heap` out of heap.h and into heap.cpp. The
//        in-class `MEASURED ~Heap` was EXPANDED into the implicit
//        `heap_.~Heap()` here - 21 instructions plus an SEH unwind frame the
//        image does not pay for, because the expansion contains a
//        non-intrinsic call. Out of line, the implicit teardown is a real
//        call: 4/4, and the frame goes. The earlier TRIED note predicted
//        exactly this and named heap.h as outside its own pass's files;
//        this is that edit, made from the heap side.
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
// Original init 0x005FD4E0 and teardown 0x005FD510 both name
// this constructed array run: 4 elements, stride 0x118; names remain provisional.
// RECOVERY_OBJECT: v2 0x009B7D08 0x009B8168 ?TxtIndex@@3PAVTextIndex@@A 0x005FD4E0
TextIndex TxtIndex[MaxTextIndexNum];  // 0x009B7D08, 0x118 stride
