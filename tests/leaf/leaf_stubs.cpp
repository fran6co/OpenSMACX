// Stand-ins for src symbols the leaf suite deliberately does not link.
//
// Copied verbatim out of tests/recovery_leaf_tests.cpp; these were always at
// global scope, so they keep the external linkage they had.  Every family
// file sees them through tests/leaf/leaf_shared.h.
#include "leaf_shared.h"

void __cdecl purge_spaces(LPSTR input) {
    char *first = input;
    while (*first == ' ') {
        ++first;
    }
    if (first != input) {
        std::memmove(input, first, std::strlen(first) + 1);
    }
    size_t length = std::strlen(input);
    while (length != 0 && input[length - 1] == ' ') {
        input[--length] = 0;
    }
}

// Scriptable stand-in for general.cpp's filefind_get; Wave::init resolves
// its filename through it.
LPSTR filefind_get_result = nullptr;
LPCSTR filefind_get_request = nullptr;
int filefind_get_calls = 0;
LPSTR __cdecl filefind_get(LPCSTR file_name) {
    filefind_get_request = file_name;
    ++filefind_get_calls;
    return filefind_get_result;
}

void __cdecl kill_lf(LPSTR input) {
    char *newline = std::strrchr(input, '\n');
    if (newline) {
        *newline = 0;
    }
}

int __cdecl stoi(LPCSTR input) {
    return std::atoi(input);
}

int __cdecl btoi(LPCSTR input) {
    int result = 0;
    while (*input == '0' || *input == '1') {
        result = result * 2 + *input++ - '0';
    }
    return result;
}

int __cdecl htoi(LPCSTR input) {
    int result = 0;
    while ((*input >= '0' && *input <= '9')
           || (*input >= 'a' && *input <= 'f')
           || (*input >= 'A' && *input <= 'F')) {
        result *= 16;
        result += *input <= '9' ? *input - '0' : (*input & ~0x20) - '7';
        ++input;
    }
    return result;
}

int __cdecl range(int input, int min, int max) {
    return input < min ? min : (input > max ? max : input);
}

bool mem_get_scripted = false;
int mem_get_calls = 0;
size_t mem_get_sizes[2] = {};
LPVOID mem_get_results[2] = {};
int atexit_calls = 0;
void (__cdecl *atexit_callback)() = nullptr;

LPVOID __cdecl mem_get(size_t size) {
    const int call = mem_get_calls++;
    if (call < 2) {
        mem_get_sizes[call] = size;
    }
    if (mem_get_scripted) {
        return call < 2 ? mem_get_results[call] : nullptr;
    }
    return std::malloc(size);
}

LPVOID __cdecl mem_get_old(size_t size) {
    return mem_get(size);
}

int heap_shutdown_calls = 0;
int font_close_calls = 0;
int font_init_calls = 0;
Font *font_init_this = nullptr;
LPCSTR font_init_name = nullptr;
int font_init_height = 0;
uint32_t font_init_style = 0;
int time_close_calls = 0;
Time *time_close_targets[8] = {};
int text_shutdown_calls = 0;
Text *text_shutdown_this = nullptr;
int text_open_calls = 0;
Text *text_open_this = nullptr;
LPCSTR text_open_source_arg = nullptr;
LPCSTR text_open_section_arg = nullptr;
BOOL text_open_result = FALSE;
int text_index_make_calls = 0;
TextIndex *text_index_make_this = nullptr;
LPCSTR text_index_make_source = nullptr;
int text_index_search_calls = 0;
TextIndex *text_index_search_this[MaxTextIndexNum] = {};
LPCSTR text_index_search_source[MaxTextIndexNum] = {};
LPCSTR text_index_search_section[MaxTextIndexNum] = {};
int text_index_search_results[MaxTextIndexNum] = {};
int env_open_calls = 0;
#if defined(__MINGW32__)
int env_close_calls = 0;
#endif
LPCSTR env_open_source = nullptr;
LPCSTR env_open_mode = nullptr;
bool env_open_fails = false;
bool capture_closed_file = false;
char closed_file_output[256] = {};
void *tracked_free_pointer = nullptr;
int tracked_free_calls = 0;
func2 *_free = nullptr;
Time *Time::TimeModal = nullptr;
int Time::TimeInitCount = 0;
Text *Txt = nullptr;
TextIndex *TxtIndex = nullptr;

void Text::shutdown() {
    ++text_shutdown_calls;
    text_shutdown_this = this;
}

BOOL Text::open(LPCSTR source, LPCSTR section) {
    ++text_open_calls;
    text_open_this = this;
    text_open_source_arg = source;
    text_open_section_arg = section;
    return text_open_result;
}

void TextIndex::make_index(LPCSTR source) {
    ++text_index_make_calls;
    text_index_make_this = this;
    text_index_make_source = source;
}

int TextIndex::search_index(LPCSTR source, LPCSTR section) {
    const int call = text_index_search_calls++;
    if (call < MaxTextIndexNum) {
        text_index_search_this[call] = this;
        text_index_search_source[call] = source;
        text_index_search_section[call] = section;
    }
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (this == &TxtIndex[i]) {
            return text_index_search_results[i];
        }
    }
    return -1;
}

void Heap::shutdown() {
    ++heap_shutdown_calls;
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    LPVOID base = nullptr;
    std::memcpy(&base, bytes + 4, sizeof(base));
    if (base) {
        std::free(base);
    }
    bytes[0] = 0;
    LPVOID null_pointer = nullptr;
    const size_t zero = 0;
    std::memcpy(bytes + 4, &null_pointer, sizeof(null_pointer));
    std::memcpy(bytes + 8, &null_pointer, sizeof(null_pointer));
    std::memcpy(bytes + 12, &zero, sizeof(zero));
    std::memcpy(bytes + 16, &zero, sizeof(zero));
}

void Spot::shutdown() {
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    LPVOID spots = nullptr;
    std::memcpy(&spots, bytes, sizeof(spots));
    if (spots) {
        std::free(spots);
    }
    clear();
}

void Time::close() {
    const int call = time_close_calls++;
    if (call < static_cast<int>(ARRAYSIZE(time_close_targets))) {
        time_close_targets[call] = this;
    }
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    const int zero = 0;
    const uint32_t resolution = 5;
    for (size_t offset = 0; offset < sizeof(Time); offset += 4) {
        std::memcpy(bytes + offset, &zero, sizeof(zero));
    }
    std::memcpy(bytes + 0x20, &resolution, sizeof(resolution));
}

int Font::init(LPCSTR font_name, int height, uint32_t style) {
    ++font_init_calls;
    font_init_this = this;
    font_init_name = font_name;
    font_init_height = height;
    font_init_style = style;
    return 13;
}

void Font::close() {
    ++font_close_calls;
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    const int negative_one = -1;
    const int zero = 0;
    std::memcpy(bytes, &negative_one, sizeof(negative_one));
    std::memcpy(bytes + 0xC, &zero, sizeof(zero));
    std::memcpy(bytes + 0x10, &zero, sizeof(zero));
    std::memcpy(bytes + 0x18, &zero, sizeof(zero));
    std::memcpy(bytes + 0x1C, &zero, sizeof(zero));
    HGDIOBJ font_object = nullptr;
    std::memcpy(&font_object, bytes + 8, sizeof(font_object));
    if (font_object) {
        font_object = nullptr;
        std::memcpy(bytes + 8, &font_object, sizeof(font_object));
    }
    LPSTR file_name = nullptr;
    std::memcpy(&file_name, bytes + 0x24, sizeof(file_name));
    if (file_name) {
        std::free(file_name);
        file_name = nullptr;
        std::memcpy(bytes + 0x24, &file_name, sizeof(file_name));
    }
}

FILE *__cdecl env_open(LPCSTR source, LPCSTR mode) {
    ++env_open_calls;
    env_open_source = source;
    env_open_mode = mode;
    return env_open_fails ? nullptr : std::tmpfile();
}

#if defined(__MINGW32__)
extern "C" int __real_fclose(FILE *file);
extern "C" void __real_free(void *pointer);

extern "C" int __wrap_atexit(void (__cdecl *callback)()) {
    ++atexit_calls;
    atexit_callback = callback;
    return 0;
}

extern "C" int __wrap_fclose(FILE *file) {
    ++env_close_calls;
    if (capture_closed_file) {
        std::fflush(file);
        std::fseek(file, 0, SEEK_SET);
        const size_t size = std::fread(
            closed_file_output, 1, sizeof(closed_file_output) - 1, file);
        closed_file_output[size] = 0;
    }
    return __real_fclose(file);
}

extern "C" void __wrap_free(void *pointer) {
    if (pointer == tracked_free_pointer) {
        ++tracked_free_calls;
    }
    __real_free(pointer);
}
#endif

LPSTR Strings::put(LPCSTR input) {
    const size_t size = std::strlen(input) + 1;
    auto *const output = static_cast<LPSTR>(std::malloc(size));
    std::memcpy(output, input, size);
    return output;
}

// src/strings.cpp owns this global; src/init_thunks.cpp names it but no longer
// defines it, because a second definition of one object only ever linked
// through -Wl,--allow-multiple-definition. This suite cannot link strings.cpp
// - it stands in for Strings::put just above - so it supplies the pointer
// itself, at the address the executable puts the object. The init-thunk case
// table below reads the pointer VALUE through its `slot` field and never
// dereferences it, so no storage has to exist here.
Strings *StringTable = reinterpret_cast<Strings *>(0x009B90D8);
