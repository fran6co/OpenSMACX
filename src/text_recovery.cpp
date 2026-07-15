#include "stdafx.h"
#include "text_recovery.h"
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
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Text::Text(size_t size)
    : current_pos_(nullptr), text_file_(nullptr), buffer_get_(nullptr), buffer_item_(nullptr) {
    file_name_[0] = 0;
    buffer_get_ = static_cast<LPSTR>(mem_get(size));
    if (buffer_get_) {
        buffer_item_ = static_cast<LPSTR>(mem_get(size));
    }
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

#if defined(__GNUC__) && defined(__i386__)
__attribute__((naked))
#endif
Text::~Text() noexcept(false) {
#if defined(__GNUC__) && defined(__i386__)
    __asm__("jmp __ZN4Text8shutdownEv");
#else
    shutdown();
#endif
}

void __cdecl text_txt() { // 005FD400
    new (Txt) Text(512);
    atexit(text_txt_exit);
}

void __cdecl text_txt_exit() { // 005FD460
    Txt->~Text();
}

BOOL __cdecl text_open(LPCSTR src_id, LPCSTR section_id) { // 005FD550
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
