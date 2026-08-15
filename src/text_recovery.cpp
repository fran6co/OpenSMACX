#include "stdafx.h"
#include "text_recovery.h"
#include "general.h"
#include "strings.h"

#include <cstddef>
#include <new>

namespace {

struct TextState {
    char file_name[80];
    char file_path[256];
    LPSTR current_pos;
    FILE *text_file;
    LPSTR buffer_get;
    LPSTR buffer_item;
};

static_assert(sizeof(TextState) == sizeof(Text),
              "Text recovery state must match the original object layout");
static_assert(offsetof(TextState, current_pos) == 0x150,
              "Text current position offset must match the original executable");
static_assert(offsetof(TextState, text_file) == 0x154,
              "Text file offset must match the original executable");
static_assert(offsetof(TextState, buffer_get) == 0x158,
              "Text get-buffer offset must match the original executable");
static_assert(offsetof(TextState, buffer_item) == 0x15C,
              "Text item-buffer offset must match the original executable");

TextState *state(Text *text) {
    return reinterpret_cast<TextState *>(text);
}

} // namespace

Text::Text()
    : current_pos_(nullptr), text_file_(nullptr), buffer_get_(nullptr), buffer_item_(nullptr) {
    file_name_[0] = 0;
}

// Text::Text(int) is DEFINED IN text.h now, in-class, so `??__ETxt` can
// inline it the way the image does. It also used to run its stores in the
// wrong order: an initialiser list runs in DECLARATION order - current_pos_,
// text_file_, buffer_get_, buffer_item_ - and then the body set
// file_name_[0], where the image writes the byte FIRST.

Text::~Text() OPENSMACX_NOEXCEPT_FALSE {
    shutdown();
}

// ORIGINAL: 0x005FD400
// name      ??__ETxt@@YAXXZ
// size      86 bytes
// spans     0x005FD400-0x005FD456
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645398
// notes     Staged hybrid export redirect calls the source-owned initializer
// RULED-OUT: letting VC6 GENERATE this. `Text Txt(512);` with the constructor
//            defined in-class does produce a dynamic initialiser with the
//            constructor inlined - but VC6 names it `_$E<n>` and registers it
//            through `.CRT$XCU`, and there is no `??__ETxt@@YAXXZ` symbol in
//            the object at all. That name is IDA's reconstruction, not
//            something the compiler emits, which is why every `??__E` in
//            src/init_thunks.cpp is hand-written like this one.
// RULED-OUT: `new (Txt) Text(512)`, which is what stands below. It compiles
//            in the build - text_recovery.cpp includes text.h - but the
//            measurement scaffold carries neither `Text` nor `Txt`, so this
//            body has never been scored at all: NO_COMPILE, `'Txt' :
//            undeclared identifier`. And it could not match if it were: the
//            image INLINES the constructor, writing the fields directly,
//            where this emits `call ??0Text@@QAE@H@Z` to 0x005FD880.
//
//            What the image does, read off the bytes, is Text::Text(size_t)
//            inlined onto the global: `file_name_[0] = 0`, then
//            `current_pos_`, `text_file_`, `buffer_get_`, `buffer_item_`
//            zeroed, then `buffer_get_ = mem_get(size)` and, only if that
//            succeeded, `buffer_item_ = mem_get(size)`. Written that way
//            against fixed addresses it measures 95.5%, 86 bytes against 86,
//            with TWO edits: the image hoists `push 0x200` above the five
//            field stores and this does not. Three source forms produce the
//            same ordering - the stores written out, a `const int size` held
//            first, and an `inline` helper taking the size - so the hoist is
//            the compiler's scheduling of an inlined constructor and not a
//            spelling.
//
//            Landing it needs `Text::Text(size_t)` defined IN text.h, so the
//            initialiser can inline it the way the original did; the members
//            it writes are private and 0x005FD880 is the out-of-line copy.
//            That is a change to the class, not to this body.
void __cdecl text_txt() {
    new (Txt) Text(512);
    atexit(text_txt_exit);
}

// ORIGINAL: 0x005FD460
// name      ??__FTxt@@YAXXZ
// size      85 bytes
// spans     0x005FD460-0x005FD4B5
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645598
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
void __cdecl text_txt_exit() {
    Txt->~Text();
}

// ORIGINAL: 0x005FD550
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
    return Txt->open(src_id, section_id);
}

void __cdecl text_close_source(Text *text) {
    TextState *const value = state(text);
    if (value->text_file) {
        fclose(value->text_file);
        value->text_file = nullptr;
    }
}

void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output) {
    *output = state(text)->buffer_get;
}

void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output) {
    *output = state(text)->buffer_item;
}

LPSTR __cdecl text_get_source(Text *text) {
    TextState *const value = state(text);
    if (feof(value->text_file)) {
        value->buffer_get[0] = 0;
        return nullptr;
    }
    if (!fgets(value->buffer_get, 511, value->text_file)) {
        value->buffer_get[0] = 0;
        return value->buffer_get;
    }
    kill_lf(value->buffer_get);
    purge_spaces(value->buffer_get);
    value->current_pos = value->buffer_get;
    return value->buffer_get;
}

LPSTR __cdecl text_string_source(Text *text, Strings *strings) {
    return strings->put(text_get_source(text));
}

LPSTR __cdecl text_item_source(Text *text) {
    TextState *const value = state(text);
    LPSTR parse = value->buffer_item;
    while (*value->current_pos != 0 && *value->current_pos != ',') {
        *parse++ = *value->current_pos++;
    }
    *parse = 0;
    if (*value->current_pos != 0) {
        ++value->current_pos;
    }
    purge_spaces(value->buffer_item);
    return value->buffer_item;
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
