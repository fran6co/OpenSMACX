#include "stdafx.h"
#include "text_recovery.h"
#include "strings.h"

#include <cstddef>

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
