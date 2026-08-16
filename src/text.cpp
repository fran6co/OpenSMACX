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
#include "temp.h"
#include "text.h"
#include "general.h"
#include "log.h"
#include "strings.h"
#include "textindex.h"

/*
Purpose: Initialize the class instance.
ORIGINAL: 0x005FD8D0
// name      ?init@Text@@QAEHH@Z
// size      153 bytes
// spans     0x005FD8D0-0x005FD969
// prototype int (__thiscall ?init@Text@@QAEHH@Z)(_Text* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00645598
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Text::init(size_t size) {
    shutdown();
    buffer_get_ = (LPSTR)mem_get(size);
    if (!buffer_get_) {
        return 4;
    }
    buffer_item_ = (LPSTR)mem_get(size);
    if (!buffer_item_) {
        return 4;
    }
    return 0;
}

/*
Purpose: Shutdown the class instance.
ORIGINAL: 0x005FD970
// name      ?shutdown@Text@@QAEXXZ
// size      92 bytes
// spans     0x005FD970-0x005FD9CC
// prototype void (__thiscall ?shutdown@Text@@QAEXXZ)(_Text* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645598
Return Value: n/a
Status: Complete
*/
void Text::shutdown() {
    close();
    if (buffer_get_) {
        free(buffer_get_);
        buffer_get_ = 0;
    }
    if (buffer_item_) {
        free(buffer_item_);
        buffer_item_ = 0;
    }
}

/*
Purpose: If open, close the text file.
ORIGINAL: 0x005FD9D0 BYTE_EXACT
// name      ?close@Text@@QAEXXZ
// size      34 bytes
// spans     0x005FD9D0-0x005FD9F2
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
ORIGINAL: 0x005FDA00
// name      ?open@Text@@QAEHPADPAD@Z
// size      516 bytes
// spans     0x005FDA00-0x005FDC04
// prototype int (__thiscall ?open@Text@@QAEHPADPAD@Z)(_Text* this, int8* srcFileID, int8* sectionID)
// callers   2   call targets   13
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FE120 0x006007B0 0x00600820 0x00626250 0x00634BB0 0x00645460 0x00645470 0x00645598 0x00645DD0 0x0064726A 0x006472CC 0x00647330 0x0064FD20
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL Text::open(LPCSTR src_file_id, LPCSTR section_id) {
    BOOL is_file_open = false; // open, possibly tracker for reading through whole file?
    if (src_file_id) {
        strcpy_s(file_name_, 80, src_file_id);
        if (!strchr(file_name_, '.')) {
            strcat_s(file_name_, 80, ".txt");
        }
        close();
        text_file_ = env_open(file_name_, "rt");
        if (!text_file_) {
            return true;
        }
        strcpy_s(file_path_, 256, FilefindPath->last_path);
    } else if (text_file_) {
        is_file_open = true;
    } else {
        text_file_ = env_open(file_name_, "rt");
        if (!text_file_) {
            return true;
        }
        strcpy_s(file_path_, 256, FilefindPath->last_path);
    }
    if (!section_id) {
        return false;
    }
    int seek_addr = text_search_index(file_name_, section_id);
    if (seek_addr >= 0) {
        log_say("Seeking to", section_id, seek_addr, 0, 0);
        fseek(text_file_, seek_addr, SEEK_SET);
        is_file_open = true;
    }
    std::string sect_header("#");
    sect_header += section_id;
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
    } while (_stricmp(sect_header.c_str(), buffer_get_));
    current_pos_ = buffer_get_;
    return false;
}

/*
Purpose: Parse text from the opened file until a newline is reached. Copy this text into the buffer.
ORIGINAL: 0x005FDC10
// name      ?get@Text@@QAEHXZ
// size      110 bytes
// spans     0x005FDC10-0x005FDC7E
// prototype int (__thiscall ?get@Text@@QAEHXZ)(_Text* this)
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x0064726A
Return Value: Pointer to string
Status: Complete
*/
LPSTR Text::get() {
    if (feof(text_file_)) {
        buffer_get_[0] = 0;
        return NULL;
    }
    if (fgets(buffer_get_, 511, text_file_)) {
        kill_lf(buffer_get_);
        purge_spaces(buffer_get_);
        current_pos_ = buffer_get_;
    } else {
        buffer_get_[0] = 0;
    }
    return buffer_get_;
}

/*
Purpose: Get the string and put it into the string table.
ORIGINAL: 0x005FDC80
// name      ?string@Text@@QAEXXZ
// size      143 bytes
// spans     0x005FDC80-0x005FDD0F
// prototype void (__thiscall ?string@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x00616970 0x0064726A
Return Value: Pointer to string table
Status: Complete
*/
LPSTR Text::string() {
    return StringTable->put(get());
}

/*
Purpose: Parse the current item into the string buffer.
ORIGINAL: 0x005FDD10 BYTE_EXACT
// name      ?item@Text@@QAEXXZ
// size      98 bytes
// spans     0x005FDD10-0x005FDD72
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
ORIGINAL: 0x005FDD80
// name      ?item_string@Text@@QAEXXZ
// size      109 bytes
// spans     0x005FDD80-0x005FDDED
// prototype void (__thiscall ?item_string@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00616970
Return Value: Pointer to string table
Status: Complete
*/
LPSTR Text::item_string() {
    return StringTable->put(item());
}

/*
Purpose: Parse the current number item.
ORIGINAL: 0x005FDDF0
// name      ?item_number@Text@@QAEXXZ
// size      104 bytes
// spans     0x005FDDF0-0x005FDE58
// prototype void (__thiscall ?item_number@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00628950
Return Value: Integer value of the number item
Status: Complete
*/
int Text::item_number() {
    return stoi(item());
}

/*
Purpose: Parse the current binary item.
ORIGINAL: 0x005FDE60
// name      ?item_binary@Text@@QAEXXZ
// size      104 bytes
// spans     0x005FDE60-0x005FDEC8
// prototype void (__thiscall ?item_binary@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288D0
Return Value: Integer value of the binary item
Status: Complete
*/
int Text::item_binary() {
    return btoi(item());
}

/*
Purpose: Parse the current hex item.
ORIGINAL: 0x005FDED0
// name      ?item_hex@Text@@QAEXXZ
// size      104 bytes
// spans     0x005FDED0-0x005FDF38
// prototype void (__thiscall ?item_hex@Text@@QAEXXZ)(_Text* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0 0x006288F0
Return Value: Integer value of the hex item
Status: Complete
*/
int Text::item_hex() {
    return htoi(item());
}
// global
/*
Purpose: The reader every `.txt` the game parses goes through, and the two
         initialisers MSVC emits for it.
ORIGINAL: 0x005FD400
// name      ??__ETxt@@YAXXZ
// size      86 bytes
// spans     0x005FD400-0x005FD456
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645398
ORIGINAL: 0x005FD460
// name      ??__FTxt@@YAXXZ
// size      85 bytes
// spans     0x005FD460-0x005FD4B5
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
`Text::Text(512)` and registers `??__FTxt`, which inlines `~Text()`. Both are
defined in-class in text.h so that inlining can happen. This line is the only
source either comes from.

RULED-OUT: hand-writing them over `Text *const Txt = (Text *)0x009B7BA0`,
           which is what stood here. It could be scored - 0x005FD400 reached
           97.8%, 87 bytes against 86 - and it could not RUN: that address is
           mapped in terranx.exe and in nothing this tree builds, so the
           first `Txt.` faults. It was also two hand-written copies of
           functions the compiler emits anyway.

UNRECOVERABLE: a measurement of either. VC6 names a generated initialiser
               `_$E<n>` and registers it through `.CRT$XCU`, while `??__E`
               and `??__F` are IDA's reconstruction of what they DO; the byte
               match looks its subject up by name. Nothing that is correct
               can carry those symbols. Both were NO_COMPILE before this
               change and had never been scored, so no claim is lost.
*/
Text Txt(512);  // 0x009B7BA0
LPSTR TextBufferGetPtr;   // 0x009B7D00
LPSTR TextBufferItemPtr;  // 0x009B7D04

// ORIGINAL: 0x005FD4C0
// name      ?text_set_get_ptr@@YAXXZ
// size      11 bytes
// spans     0x005FD4C0-0x005FD4CB
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_set_get_ptr() {
    text_set_get_ptr_source(&Txt, &TextBufferGetPtr);
}

// ORIGINAL: 0x005FD4D0
// name      ?text_set_item_ptr@@YAXXZ
// size      11 bytes
// spans     0x005FD4D0-0x005FD4DB
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_set_item_ptr() {
    text_set_item_ptr_source(&Txt, &TextBufferItemPtr);
}

// ORIGINAL: 0x005FD530
// name      ?text_close@@YAXXZ
// size      29 bytes
// spans     0x005FD530-0x005FD54D
// prototype 
// callers   22   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645598
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl text_close() { text_close_source(&Txt); }

// ORIGINAL: 0x005FD570
// name      ?text_get@@YAPADXZ
// size      98 bytes
// spans     0x005FD570-0x005FD5D2
// prototype 
// callers   54   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x0064726A
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_get() { return text_get_source(&Txt); }

// ORIGINAL: 0x005FD5E0
// name      ?text_string@@YAPADXZ
// size      131 bytes
// spans     0x005FD5E0-0x005FD663
// prototype 
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00600820 0x00616970 0x0064726A
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_string() { return text_string_source(&Txt, StringTable); }

// ORIGINAL: 0x005FD670
// name      ?text_item@@YAPADXZ
// size      87 bytes
// spans     0x005FD670-0x005FD6C7
// prototype 
// callers   20   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006007B0
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_item() { return text_item_source(&Txt); }

// ORIGINAL: 0x005FD6D0
// name      ?text_item_string@@YAPADXZ
// size      98 bytes
// spans     0x005FD6D0-0x005FD732
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00616970
// notes     Staged hybrid export redirect calls the source-owned wrapper
LPSTR __cdecl text_item_string() {
    return text_item_string_source(&Txt, StringTable);
}

// ORIGINAL: 0x005FD740
// name      ?text_item_number@@YAHXZ
// size      93 bytes
// spans     0x005FD740-0x005FD79D
// prototype 
// callers   18   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00628950
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_item_number() { return text_item_number_source(&Txt); }

// ORIGINAL: 0x005FD7A0
// name      ?text_item_binary@@YAHXZ
// size      93 bytes
// spans     0x005FD7A0-0x005FD7FD
// prototype 
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288D0
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_item_binary() { return text_item_binary_source(&Txt); }

// ORIGINAL: 0x005FD800
// name      ?text_item_hex@@YAHXZ
// size      93 bytes
// spans     0x005FD800-0x005FD85D
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x006288F0
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_item_hex() { return text_item_hex_source(&Txt); }

// ORIGINAL: 0x00585120
// name      ?text_get_number@@YAHHH@Z
// size      48 bytes
// spans     0x00585120-0x00585150
// prototype int (__cdecl ?text_get_number@@YAHHH@Z)(int min, int max)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD570 0x005FD740
// notes     Staged hybrid export redirect calls the source-owned wrapper
int __cdecl text_get_number(int min, int max) {
    return text_get_number_source(&Txt, min, max);
}

// ---------------------------------------------------------------------------
// Merged from the retired src/text_recovery.cpp on 2026-08-15. It held the
// class's own constructors and destructor while text.cpp held every other
// method, and it reached the private members through a hand-mirrored
// `struct TextState`; both are gone.
// ---------------------------------------------------------------------------
Text::Text()
    : current_pos_(nullptr), text_file_(nullptr), buffer_get_(nullptr), buffer_item_(nullptr) {
    file_name_[0] = 0;
}

// Text::Text(int) is DEFINED IN text.h now, in-class, so `??__ETxt` can
// inline it the way the image does. It also used to run its stores in the
// wrong order: an initialiser list runs in DECLARATION order - current_pos_,
// text_file_, buffer_get_, buffer_item_ - and then the body set
// file_name_[0], where the image writes the byte FIRST.


// ORIGINAL: 0x005FD550 BYTE_EXACT
// name      ?text_open@@YAHPADPAD@Z
// size      21 bytes
// spans     0x005FD550-0x005FD565
// prototype int (__cdecl ?text_open@@YAHPADPAD@Z)(int8* srcFileID, int8* sectionID)
// callers   34   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FDA00
// notes     Staged hybrid export redirect calls the source-owned wrapper
BOOL __cdecl text_open(LPCSTR src_id, LPCSTR section_id) {
    return Txt.open(src_id, section_id);
}

void __cdecl text_close_source(Text *text) {
    Text *const value = text;
    if (value->text_file_) {
        fclose(value->text_file_);
        value->text_file_ = nullptr;
    }
}

void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output) {
    *output = text->buffer_get_;
}

void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output) {
    *output = text->buffer_item_;
}

LPSTR __cdecl text_get_source(Text *text) {
    Text *const value = text;
    if (feof(value->text_file_)) {
        value->buffer_get_[0] = 0;
        return nullptr;
    }
    if (!fgets(value->buffer_get_, 511, value->text_file_)) {
        value->buffer_get_[0] = 0;
        return value->buffer_get_;
    }
    kill_lf(value->buffer_get_);
    purge_spaces(value->buffer_get_);
    value->current_pos_ = value->buffer_get_;
    return value->buffer_get_;
}

LPSTR __cdecl text_string_source(Text *text, Strings *strings) {
    return strings->put(text_get_source(text));
}

LPSTR __cdecl text_item_source(Text *text) {
    Text *const value = text;
    LPSTR parse = value->buffer_item_;
    while (*value->current_pos_ != 0 && *value->current_pos_ != ',') {
        *parse++ = *value->current_pos_++;
    }
    *parse = 0;
    if (*value->current_pos_ != 0) {
        ++value->current_pos_;
    }
    purge_spaces(value->buffer_item_);
    return value->buffer_item_;
}

LPSTR __cdecl text_item_string_source(Text *text, Strings *strings) {
    return strings->put(text_item_source(text));
}

int __cdecl text_item_number_source(Text *text) {
    return stoi(text_item_source(text));
}

int __cdecl text_item_binary_source(Text *text) {
    return btoi(text_item_source(text));
}

int __cdecl text_item_hex_source(Text *text) {
    return htoi(text_item_source(text));
}

int __cdecl text_get_number_source(Text *text, int min, int max) {
    text_get_source(text);
    return range(text_item_number_source(text), min, max);
}
