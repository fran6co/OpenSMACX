#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/basepop.h"
#include "../src/buttongroup.h"
#include "../src/dialog.h"
#include "../src/filemap.h"
#include "../src/font.h"
#include "../src/log.h"
#include "../src/menu.h"
#include "../src/pulldown.h"
#include "../src/random.h"
#include "../src/scroll.h"
#include "../src/spot.h"
#include "../src/stringstruct.h"
#include "../src/strings.h"
#include "../src/text_recovery.h"
#include "../src/textindex.h"
#include "../src/time.h"
#include "../src/win.h"

#include <climits>
#include <cstring>
#include <float.h>
#include <new>

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

int heap_shutdown_calls = 0;
int font_close_calls = 0;
int font_init_calls = 0;
Font *font_init_this = nullptr;
LPCSTR font_init_name = nullptr;
int font_init_height = 0;
uint32_t font_init_style = 0;
int time_close_calls = 0;
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
    ++time_close_calls;
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

BOOL __cdecl in_box(int x, int y, const RECT *rect);

namespace {

int failures = 0;

void expect(bool condition) {
    if (!condition) {
        ++failures;
    }
}

void expect_tracked_free_calls(int expected) {
#if defined(__MINGW32__)
    expect(tracked_free_calls == expected);
#else
    static_cast<void>(expected);
#endif
}

void test_alpha_net_pid_to_idx() {
    AlphaNet network{};
    uint8_t *bytes = reinterpret_cast<uint8_t *>(&network);
    constexpr size_t first_pid_offset = 0x928;
    constexpr size_t pid_stride = 0x19C;

    for (int index = 1; index < 8; ++index) {
        const uint32_t process_id = 1000U + static_cast<uint32_t>(index);
        std::memcpy(bytes + first_pid_offset + (index - 1) * pid_stride,
                    &process_id, sizeof(process_id));
        expect(network.pid_2_idx(process_id) == index);
        expect(alpha_net_pid_to_idx_redirect(&network, nullptr, process_id) == index);
    }

    expect(network.pid_2_idx(999U) == 0);
    expect(alpha_net_pid_to_idx_redirect(&network, nullptr, 999U) == 0);

    const uint32_t duplicate_id = 2000U;
    std::memcpy(bytes + first_pid_offset + pid_stride, &duplicate_id,
                sizeof(duplicate_id));
    std::memcpy(bytes + first_pid_offset + 4 * pid_stride, &duplicate_id,
                sizeof(duplicate_id));
    expect(network.pid_2_idx(duplicate_id) == 2);

    AlphaNet empty_network{};
    expect(empty_network.pid_2_idx(0) == 1);
}

void test_alpha_net_identity_lookups() {
    alignas(AlphaNet) uint8_t storage[sizeof(AlphaNet) + 32];
    std::memset(storage, 0xA5, sizeof(storage));
    auto *network = reinterpret_cast<AlphaNet *>(storage + 16);
    constexpr size_t first_pid_offset = 0x928;
    constexpr size_t identity_offset = 0x92C;
    constexpr size_t slot_stride = 0x19C;
    const uint32_t process_ids[7] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 123U, 456U,
    };
    const int8_t identities[7] = {
        0, 1, 0x7F, static_cast<int8_t>(0x80),
        static_cast<int8_t>(0xFF), 42, -42,
    };
    const uint32_t lower_decoy_pid = 789U;
    const int8_t lower_decoy_identity = 2;
    const uint32_t upper_decoy_pid = 790U;
    const int8_t upper_decoy_identity = 3;

    for (int slot = 0; slot < 7; ++slot) {
        std::memcpy(reinterpret_cast<uint8_t *>(network)
                        + first_pid_offset + slot * slot_stride,
                    &process_ids[slot], sizeof(process_ids[slot]));
        std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + identity_offset + slot * slot_stride,
                    &identities[slot], sizeof(identities[slot]));
    }
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + first_pid_offset - slot_stride,
                &lower_decoy_pid, sizeof(lower_decoy_pid));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + identity_offset - slot_stride,
                &lower_decoy_identity, sizeof(lower_decoy_identity));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + first_pid_offset + 7 * slot_stride,
                &upper_decoy_pid, sizeof(upper_decoy_pid));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + identity_offset + 7 * slot_stride,
                &upper_decoy_identity, sizeof(upper_decoy_identity));
    uint8_t expected[sizeof(storage)];
    std::memcpy(expected, storage, sizeof(storage));

    for (int slot = 0; slot < 7; ++slot) {
        const int identity = static_cast<int>(identities[slot]);
        expect(network->pid_2_who(process_ids[slot]) == identity);
        expect(alpha_net_pid_to_who_redirect(
                   network, nullptr, process_ids[slot]) == identity);
        expect(static_cast<uint32_t>(network->who_2_pid(identity))
               == process_ids[slot]);
        expect(static_cast<uint32_t>(alpha_net_who_to_pid_redirect(
                   network, nullptr, identity)) == process_ids[slot]);
        expect(network->who_2_idx(identity) == slot + 1);
        expect(alpha_net_who_to_idx_redirect(
                   network, nullptr, identity) == slot + 1);
    }

    expect(network->pid_2_who(lower_decoy_pid) == 0);
    expect(network->pid_2_who(upper_decoy_pid) == 0);
    expect(alpha_net_pid_to_who_redirect(network, nullptr, lower_decoy_pid) == 0);
    expect(alpha_net_pid_to_who_redirect(network, nullptr, upper_decoy_pid) == 0);
    expect(network->who_2_pid(lower_decoy_identity) == 0);
    expect(network->who_2_pid(upper_decoy_identity) == 0);
    expect(alpha_net_who_to_pid_redirect(
               network, nullptr, lower_decoy_identity) == 0);
    expect(alpha_net_who_to_pid_redirect(
               network, nullptr, upper_decoy_identity) == 0);
    expect(network->who_2_idx(lower_decoy_identity) == 0);
    expect(network->who_2_idx(upper_decoy_identity) == 0);
    expect(network->who_2_idx(128) == 0);
    expect(network->who_2_idx(-129) == 0);
    expect(std::memcmp(storage, expected, sizeof(storage)) == 0);

    const uint32_t duplicate_pid = process_ids[1];
    const int8_t later_pid_identity = 77;
    const int8_t duplicate_identity = identities[1];
    const uint32_t later_identity_pid = process_ids[6];
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + first_pid_offset + 5 * slot_stride,
                &duplicate_pid, sizeof(duplicate_pid));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + identity_offset + 5 * slot_stride,
                &later_pid_identity, sizeof(later_pid_identity));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + first_pid_offset + 6 * slot_stride,
                &later_identity_pid, sizeof(later_identity_pid));
    std::memcpy(reinterpret_cast<uint8_t *>(network)
                    + identity_offset + 6 * slot_stride,
                &duplicate_identity, sizeof(duplicate_identity));
    std::memcpy(expected, storage, sizeof(storage));
    expect(network->pid_2_who(duplicate_pid) == identities[1]);
    expect(static_cast<uint32_t>(network->who_2_pid(duplicate_identity))
           == process_ids[1]);
    expect(network->who_2_idx(duplicate_identity) == 2);
    expect(std::memcmp(storage, expected, sizeof(storage)) == 0);
}

void test_in_box_edges() {
    const RECT rect = {10, 20, 30, 40};

    expect(in_box(10, 20, &rect));
    expect(in_box(29, 39, &rect));
    expect(!in_box(30, 39, &rect));
    expect(!in_box(29, 40, &rect));
    expect(!in_box(9, 20, &rect));
    expect(!in_box(10, 19, &rect));
}

void set_text_pointer(Text *text, size_t offset, const void *value) {
    std::memcpy(reinterpret_cast<uint8_t *>(text) + offset, value, sizeof(void *));
}

void test_text_get_and_item_number() {
    alignas(Text) uint8_t storage[sizeof(Text)]{};
    auto *text = reinterpret_cast<Text *>(storage);
    char get_buffer[512]{};
    char item_buffer[512]{};
    LPSTR get_pointer = get_buffer;
    LPSTR item_pointer = item_buffer;
    FILE *file = std::tmpfile();

    expect(file != nullptr);
    if (!file) {
        return;
    }
    set_text_pointer(text, 0x154, &file);
    set_text_pointer(text, 0x158, &get_pointer);
    set_text_pointer(text, 0x15C, &item_pointer);

    LPSTR published_get = nullptr;
    LPSTR published_item = nullptr;
    text_set_get_ptr_source(text, &published_get);
    text_set_item_ptr_source(text, &published_item);
    expect(published_get == get_buffer);
    expect(published_item == item_buffer);

    std::fputs("  first line  \n", file);
    std::rewind(file);
    expect(text_get_source(text) == get_buffer);
    expect(std::strcmp(get_buffer, "first line") == 0);
    void *current = get_buffer;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);

    expect(text_get_source(text) == get_buffer);
    expect(get_buffer[0] == 0);
    get_buffer[0] = 'x';
    expect(text_get_source(text) == nullptr);
    expect(get_buffer[0] == 0);

    char items[] = "  -42 ,ignored";
    current = items;
    set_text_pointer(text, 0x150, &current);
    std::memset(item_buffer, 0xA5, sizeof(item_buffer));
    expect(text_item_source(text) == item_buffer);
    expect(std::strcmp(item_buffer, "-42") == 0);
    current = items + 7;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);

    char binary[] = "10110,ignored";
    current = binary;
    set_text_pointer(text, 0x150, &current);
    expect(text_item_binary_source(text) == 22);
    expect(std::strcmp(item_buffer, "10110") == 0);
    current = binary + 6;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);

    char hexadecimal[] = "1aF,ignored";
    current = hexadecimal;
    set_text_pointer(text, 0x150, &current);
    expect(text_item_hex_source(text) == 431);
    expect(std::strcmp(item_buffer, "1aF") == 0);
    current = hexadecimal + 4;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);

    FILE *numbers = std::tmpfile();
    expect(numbers != nullptr);
    if (numbers) {
        std::fputs("-4\n7\n99\n", numbers);
        std::rewind(numbers);
        set_text_pointer(text, 0x154, &numbers);
        expect(text_get_number_source(text, 0, 10) == 0);
        expect(text_get_number_source(text, 0, 10) == 7);
        expect(text_get_number_source(text, 0, 10) == 10);
        text_close_source(text);
    }
    set_text_pointer(text, 0x154, &file);

    current = items;
    set_text_pointer(text, 0x150, &current);
    std::memset(item_buffer, 0xA5, sizeof(item_buffer));
    expect(text_item_number_source(text) == -42);
    expect(std::strcmp(item_buffer, "-42") == 0);
    current = items + 7;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);
    text_close_source(text);
    FILE *null_file = nullptr;
    expect(std::memcmp(storage + 0x154, &null_file, sizeof(null_file)) == 0);
    text_close_source(text);
}

void test_text_string_helpers() {
    alignas(Text) uint8_t storage[sizeof(Text)]{};
    auto *text = reinterpret_cast<Text *>(storage);
    char get_buffer[512]{};
    char item_buffer[512]{};
    LPSTR get_pointer = get_buffer;
    LPSTR item_pointer = item_buffer;
    FILE *file = std::tmpfile();
    Strings strings;

    expect(file != nullptr);
    if (!file) {
        return;
    }
    set_text_pointer(text, 0x154, &file);
    set_text_pointer(text, 0x158, &get_pointer);
    set_text_pointer(text, 0x15C, &item_pointer);

    std::fputs("  stored line  \n", file);
    std::rewind(file);
    LPSTR stored = text_string_source(text, &strings);
    expect(stored != get_buffer);
    expect(std::strcmp(stored, "stored line") == 0);
    expect(std::strcmp(get_buffer, "stored line") == 0);

    char items[] = "  item string ,ignored";
    void *current = items;
    set_text_pointer(text, 0x150, &current);
    stored = text_item_string_source(text, &strings);
    expect(stored != item_buffer);
    expect(std::strcmp(stored, "item string") == 0);
    expect(std::strcmp(item_buffer, "item string") == 0);
    current = items + 15;
    expect(std::memcmp(storage + 0x150, &current, sizeof(current)) == 0);
    std::fclose(file);
}

void expect_storage_bytes(
        const uint8_t *storage, const uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        expect(storage[index] == expected[index]);
    }
}

void report_storage_mismatch(
        const char *fixture, const uint8_t *storage, const uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        if (storage[index] != expected[index]) {
            std::fprintf(stderr, "%s mismatch at 0x%zx: 0x%02x != 0x%02x\n",
                fixture, index, storage[index], expected[index]);
            return;
        }
    }
}

void seed_storage(uint8_t *storage, uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        storage[index] = static_cast<uint8_t>(0x35U + index * 17U);
        expected[index] = storage[index];
    }
}

void expect_lifecycle_clear(uint8_t *expected) {
    std::memset(expected + 16, 0, 0x84);
    std::memset(expected + 16 + 0x8C, 0, 8);
}

void test_button_group_lifecycle() {
    alignas(ButtonGroup) uint8_t storage[sizeof(ButtonGroup) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);

    auto *group = new (storage + 16) ButtonGroup;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);
    group->close();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(ButtonGroup));
    const int group_id = 0x12345678;
    const int flags = static_cast<int>(0xA55AA55AU);
    const uint32_t unselected = 0xFFFFFFFEU;
    std::memcpy(expected + 16 + 0x84, &unselected, sizeof(unselected));
    std::memcpy(expected + 16 + 0x88, &flags, sizeof(flags));
    std::memcpy(expected + 16 + 0x8C, &group_id, sizeof(group_id));
    group->init(group_id, flags);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);
    group->~ButtonGroup();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);
    group = button_group_construct_redirect(
        reinterpret_cast<ButtonGroup *>(storage + 16), nullptr);
    expect(group == reinterpret_cast<ButtonGroup *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);
    expect(button_group_close_redirect(group, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(ButtonGroup));
    const int negative_group_id = static_cast<int>(0x89ABCDEFU);
    const int negative_flags = -1;
    std::memcpy(expected + 16 + 0x84, &unselected, sizeof(unselected));
    std::memcpy(expected + 16 + 0x88, &negative_flags, sizeof(negative_flags));
    std::memcpy(expected + 16 + 0x8C, &negative_group_id, sizeof(negative_group_id));
    expect(button_group_init_redirect(
        group, nullptr, negative_group_id, negative_flags) == negative_group_id);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_base_pop_string_font() {
    alignas(BasePop) uint8_t storage[sizeof(BasePop) + 32];
    uint8_t expected[sizeof(storage)];
    auto *popup = new (storage + 16) BasePop;

    alignas(Font) uint8_t font_storage[sizeof(Font)];
    auto *primary = new (font_storage) Font;
    auto *secondary = reinterpret_cast<Font *>(0x11111111U);
    auto *tertiary = reinterpret_cast<Font *>(0x22222222U);
    auto *quaternary = reinterpret_cast<Font *>(0x33333333U);

    seed_storage(storage, expected, sizeof(storage));
    expect(popup->set_string_font(
        nullptr, secondary, tertiary, quaternary) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected + 16 + 0x3114, &secondary, sizeof(secondary));
    std::memcpy(expected + 16 + 0x3118, &tertiary, sizeof(tertiary));
    std::memcpy(expected + 16 + 0x311C, &quaternary, sizeof(quaternary));
    expect(popup->set_string_font(
        primary, secondary, tertiary, quaternary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    *reinterpret_cast<HFONT *>(font_storage + 8) = reinterpret_cast<HFONT>(1);
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected + 16 + 0x3110, &primary, sizeof(primary));
    std::memcpy(expected + 16 + 0x3114, &secondary, sizeof(secondary));
    std::memcpy(expected + 16 + 0x3118, &tertiary, sizeof(tertiary));
    std::memcpy(expected + 16 + 0x311C, &quaternary, sizeof(quaternary));
    expect(base_pop_set_string_font_redirect(
        popup, nullptr, primary, secondary, tertiary, quaternary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

template <typename T>
void write_at(uint8_t *storage, size_t offset, const T &value) {
    std::memcpy(storage + offset, &value, sizeof(value));
}

template <typename T>
void write_at_volatile(uint8_t *storage, size_t offset, const T &value) {
    auto *target = reinterpret_cast<volatile uint8_t *>(storage + offset);
    const auto *source = reinterpret_cast<const uint8_t *>(&value);
    for (size_t index = 0; index < sizeof(value); ++index) {
        target[index] = source[index];
    }
}

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "tests require 32-bit int");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

struct ScrollInitProbeState {
    int calls;
    Scroll *self;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    Win *parent;
    uint32_t setting;
    uint32_t options;
    int nonclient;
    uint32_t result;
    bool clear_nonclient;
};

ScrollInitProbeState scroll_init_probe_state = {};

int __cdecl scroll_init_probe(Scroll *self, int x, int y, int width, int height,
                              Win *parent, int setting, int options) {
    ++scroll_init_probe_state.calls;
    scroll_init_probe_state.self = self;
    scroll_init_probe_state.x = static_cast<uint32_t>(x);
    scroll_init_probe_state.y = static_cast<uint32_t>(y);
    scroll_init_probe_state.width = static_cast<uint32_t>(width);
    scroll_init_probe_state.height = static_cast<uint32_t>(height);
    scroll_init_probe_state.parent = parent;
    scroll_init_probe_state.setting = static_cast<uint32_t>(setting);
    scroll_init_probe_state.options = static_cast<uint32_t>(options);
    scroll_init_probe_state.nonclient = ScrollNonClientInit
        ? *ScrollNonClientInit : INT_MIN;
    if (scroll_init_probe_state.clear_nonclient) {
        *ScrollNonClientInit = 0;
    }
    return int_from_bits(scroll_init_probe_state.result);
}

void reset_scroll_init_probe(uint32_t result) {
    std::memset(&scroll_init_probe_state, 0, sizeof(scroll_init_probe_state));
    scroll_init_probe_state.result = result;
}

int call_scroll_axis(Scroll *scroll, int kind, bool use_adapter, int x, int y,
                     int length, Win *parent, int setting) {
    if (kind == 0) {
        return use_adapter
            ? scroll_init_vert_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_vert(x, y, length, parent, setting);
    }
    if (kind == 1) {
        return use_adapter
            ? scroll_init_horz_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_horz(x, y, length, parent, setting);
    }
    if (kind == 2) {
        return use_adapter
            ? scroll_init_vert_nc_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_vert_nc(x, y, length, parent, setting);
    }
    return use_adapter
        ? scroll_init_horz_nc_redirect(
            scroll, nullptr, x, y, length, parent, setting)
        : scroll->init_horz_nc(x, y, length, parent, setting);
}

void test_scroll_init_wrappers() {
    ScrollPrimaryInitProc const saved_primary_init = ScrollPrimaryInit;
    int *const saved_default_thickness = ScrollDefaultThickness;
    int *const saved_nonclient_init = ScrollNonClientInit;
    int default_thickness = 0;
    int nonclient_init = 0;
    ScrollPrimaryInit = &scroll_init_probe;
    ScrollDefaultThickness = &default_thickness;
    ScrollNonClientInit = &nonclient_init;

    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
    uint8_t expected[sizeof(storage)];
    alignas(Win) uint8_t parent_storage[sizeof(Win)];
    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
    auto *parent = reinterpret_cast<Win *>(parent_storage);

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        seed_storage(storage, expected, sizeof(storage));
        reset_scroll_init_probe(0xA55AA55AU);
        const int null_rect_result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, nullptr, parent, INT_MIN, INT_MAX)
            : scroll->init(nullptr, parent, INT_MIN, INT_MAX);
        expect(null_rect_result == 3);
        expect(scroll_init_probe_state.calls == 0);
        expect_storage_bytes(storage, expected, sizeof(storage));

        reset_scroll_init_probe(0xA55AA55AU);
        auto *poison_rect = reinterpret_cast<RECT *>(1U);
        const int null_parent_result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, poison_rect, nullptr, INT_MAX, INT_MIN)
            : scroll->init(poison_rect, nullptr, INT_MAX, INT_MIN);
        expect(null_parent_result == 3);
        expect(scroll_init_probe_state.calls == 0);
        expect_storage_bytes(storage, expected, sizeof(storage));

        alignas(RECT) uint8_t rect_storage[sizeof(RECT) + 32];
        uint8_t rect_expected[sizeof(rect_storage)];
        seed_storage(rect_storage, rect_expected, sizeof(rect_storage));
        const uint32_t coordinates[] = {
            0x7FFFFFFFU, 0x80000000U, 0x80000000U, 0x7FFFFFFFU,
        };
        std::memcpy(rect_storage + 16, coordinates, sizeof(coordinates));
        std::memcpy(rect_expected, rect_storage, sizeof(rect_storage));
        auto *rect = reinterpret_cast<RECT *>(rect_storage + 16);
        nonclient_init = int_from_bits(0x2468ACE0U);
        reset_scroll_init_probe(0x89ABCDEFU);
        const int result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, rect, parent,
                int_from_bits(0x13579BDFU), int_from_bits(0xFEDCBA98U))
            : scroll->init(rect, parent,
                           int_from_bits(0x13579BDFU),
                           int_from_bits(0xFEDCBA98U));
        expect(static_cast<uint32_t>(result) == 0x89ABCDEFU);
        expect(scroll_init_probe_state.calls == 1);
        expect(scroll_init_probe_state.self == scroll);
        expect(scroll_init_probe_state.x == coordinates[0]);
        expect(scroll_init_probe_state.y == coordinates[1]);
        expect(scroll_init_probe_state.width
               == coordinates[2] - coordinates[0]);
        expect(scroll_init_probe_state.height
               == coordinates[3] - coordinates[1]);
        expect(scroll_init_probe_state.parent == parent);
        expect(scroll_init_probe_state.setting == 0x13579BDFU);
        expect(scroll_init_probe_state.options == 0xFEDCBA98U);
        expect(scroll_init_probe_state.nonclient
               == int_from_bits(0x2468ACE0U));
        expect(nonclient_init == int_from_bits(0x2468ACE0U));
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect_storage_bytes(rect_storage, rect_expected, sizeof(rect_storage));
    }

    for (int kind = 0; kind < 4; ++kind) {
        const bool nonclient = kind >= 2;
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            seed_storage(storage, expected, sizeof(storage));
            ScrollDefaultThickness = nullptr;
            nonclient_init = int_from_bits(0x2468ACE0U);
            reset_scroll_init_probe(0xA55AA55AU);
            expect(call_scroll_axis(
                       scroll, kind, use_adapter, 10, 20, 0, parent, 30) == 3);
            expect(scroll_init_probe_state.calls == 0);
            expect(nonclient_init == (nonclient
                ? 1 : int_from_bits(0x2468ACE0U)));
            expect_storage_bytes(storage, expected, sizeof(storage));

            nonclient_init = int_from_bits(0x13579BDFU);
            reset_scroll_init_probe(0x5AA55AA5U);
            expect(call_scroll_axis(
                       scroll, kind, use_adapter, INT_MIN, INT_MAX,
                       -1, nullptr, 0) == 3);
            expect(scroll_init_probe_state.calls == 0);
            expect(nonclient_init == (nonclient
                ? 1 : int_from_bits(0x13579BDFU)));
            expect_storage_bytes(storage, expected, sizeof(storage));
            ScrollDefaultThickness = &default_thickness;
        }
    }

    struct AxisCase {
        int kind;
        uint32_t x;
        uint32_t y;
        uint32_t length;
        uint32_t thickness;
        uint32_t setting;
    };
    const AxisCase axis_cases[] = {
        {0, 0x80000000U, 0x7FFFFFFFU, 0x80000000U, 0U, 0xFFFFFFFFU},
        {1, 0xFFFFFFFFU, 0U, 7U, 0xFFFFFFFFU, 0x80000000U},
        {2, 10U, 20U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x13579BDFU},
        {3, 30U, 40U, 0x7FFFFFFFU, 0x80000000U, 0xFEDCBA98U},
    };
    for (const AxisCase &test : axis_cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            seed_storage(storage, expected, sizeof(storage));
            default_thickness = int_from_bits(test.thickness);
            nonclient_init = int_from_bits(0x2468ACE0U);
            const uint32_t expected_result = 0x89ABCDEFU
                ^ (static_cast<uint32_t>(test.kind) * 0x11111111U)
                ^ static_cast<uint32_t>(use_adapter);
            reset_scroll_init_probe(expected_result);
            scroll_init_probe_state.clear_nonclient = test.kind >= 2;
            const int result = call_scroll_axis(
                scroll, test.kind, use_adapter,
                int_from_bits(test.x), int_from_bits(test.y),
                int_from_bits(test.length), parent, int_from_bits(test.setting));
            expect(static_cast<uint32_t>(result) == expected_result);
            expect(scroll_init_probe_state.calls == 1);
            expect(scroll_init_probe_state.self == scroll);
            expect(scroll_init_probe_state.x == test.x);
            expect(scroll_init_probe_state.y == test.y);
            expect(scroll_init_probe_state.width
                   == ((test.kind & 1) ? test.length : test.thickness));
            expect(scroll_init_probe_state.height
                   == ((test.kind & 1) ? test.thickness : test.length));
            expect(scroll_init_probe_state.parent == parent);
            expect(scroll_init_probe_state.setting == test.setting);
            expect(scroll_init_probe_state.options == 0U);
            expect(scroll_init_probe_state.nonclient == (test.kind >= 2
                ? 1 : int_from_bits(0x2468ACE0U)));
            expect(nonclient_init == (test.kind >= 2
                ? 0 : int_from_bits(0x2468ACE0U)));
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    ScrollPrimaryInit = saved_primary_init;
    ScrollDefaultThickness = saved_default_thickness;
    ScrollNonClientInit = saved_nonclient_init;
}

int scroll_redraw_calls = 0;
Scroll *scroll_redraw_self = nullptr;
uint32_t scroll_redraw_result = 0;
uint32_t scroll_redraw_minimum = 0;
uint32_t scroll_redraw_maximum = 0;
uint32_t scroll_redraw_reverse = 0;
uint32_t scroll_redraw_position = 0;
Win *scroll_redraw_parent = nullptr;
bool scroll_redraw_check_range = false;
bool scroll_redraw_check_current = false;

uint32_t __fastcall scroll_redraw_probe(Scroll *self, void *) {
    ++scroll_redraw_calls;
    scroll_redraw_self = self;
    const auto *bytes = reinterpret_cast<const uint8_t *>(self);
    uint32_t minimum;
    uint32_t maximum;
    uint32_t reverse;
    uint32_t position;
    std::memcpy(&minimum, bytes + 0xA20, sizeof(minimum));
    std::memcpy(&maximum, bytes + 0xA24, sizeof(maximum));
    std::memcpy(&reverse, bytes + 0xA28, sizeof(reverse));
    std::memcpy(&position, bytes + 0xA2C, sizeof(position));
    if (scroll_redraw_check_range) {
        expect(minimum == scroll_redraw_minimum);
        expect(maximum == scroll_redraw_maximum);
        expect(reverse == scroll_redraw_reverse);
        expect(position == scroll_redraw_position);
    }
    if (scroll_redraw_check_current) {
        expect(*ScrollCurrentWin == scroll_redraw_parent);
        expect(position == scroll_redraw_position);
    }
    return scroll_redraw_result;
}

void reset_scroll_redraw_probe() {
    scroll_redraw_calls = 0;
    scroll_redraw_self = nullptr;
    scroll_redraw_check_range = false;
    scroll_redraw_check_current = false;
}

void install_scroll_redraw_probe(uint8_t *storage, uintptr_t vtable[63]) {
    std::memset(vtable, 0, sizeof(uintptr_t) * 63);
    vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_redraw_probe);
    uintptr_t *vtable_pointer = vtable;
    write_at(storage, 16, vtable_pointer);
}

int scroll_style_redraw_calls = 0;
uint8_t *scroll_style_base = nullptr;
size_t scroll_style_offsets[3] = {};
uint32_t scroll_style_value = 0;
uint32_t scroll_style_results[2] = {};

void expect_scroll_style_fields() {
    for (size_t offset : scroll_style_offsets) {
        uint32_t actual;
        std::memcpy(&actual, scroll_style_base + offset, sizeof(actual));
        expect(actual == scroll_style_value);
    }
}

uint32_t __fastcall scroll_style_left_redraw_probe(void *self, void *) {
    expect(scroll_style_redraw_calls == 0);
    expect(self == scroll_style_base + 0xAAC);
    expect_scroll_style_fields();
    ++scroll_style_redraw_calls;
    return scroll_style_results[0];
}

uint32_t __fastcall scroll_style_right_redraw_probe(void *self, void *) {
    expect(scroll_style_redraw_calls == 1);
    expect(self == scroll_style_base + 0x15F8);
    expect_scroll_style_fields();
    ++scroll_style_redraw_calls;
    return scroll_style_results[1];
}

void install_scroll_style_redraw_probes(
        uint8_t *storage, uintptr_t left_vtable[63],
        uintptr_t right_vtable[63]) {
    std::memset(left_vtable, 0, sizeof(uintptr_t) * 63);
    std::memset(right_vtable, 0, sizeof(uintptr_t) * 63);
    left_vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_style_left_redraw_probe);
    right_vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_style_right_redraw_probe);
    uintptr_t *left_vtable_pointer = left_vtable;
    uintptr_t *right_vtable_pointer = right_vtable;
    write_at(storage, 16 + 0xAAC, left_vtable_pointer);
    write_at(storage, 16 + 0x15F8, right_vtable_pointer);
}

void test_win_move() {
    struct MoveCase {
        uint32_t flags;
        uint32_t x;
        uint32_t y;
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    };
    const MoveCase cases[] = {
        {0U, 100U, 200U, 10U, 20U, 50U, 80U},
        {0xFFFFFFFDU, 0xFFFFFF9CU, 0x0000012CU,
         0x00000032U, 0xFFFFFFCEU, 0x00000096U, 0x0000004BU},
        {2U, 0x80000000U, 0x7FFFFFFFU,
         0x7FFFFFFFU, 0x80000000U, 0x80000005U, 0x7FFFFFF0U},
        {0xA5A5A5A6U, 0xFFFFFFFFU, 0U,
         0x80000000U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };

    for (const MoveCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Win) uint8_t storage[sizeof(Win) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x9C, fixture.flags);
            const uint32_t decoy_rect[4] = {
                0x11223344U, 0x55667788U, 0x99AABBCCU, 0xDDEEFF00U,
            };
            const size_t target = (fixture.flags & 2U) ? 0x14C : 0x13C;
            const size_t decoy = (fixture.flags & 2U) ? 0x13C : 0x14C;
            std::memcpy(storage + 16 + decoy, decoy_rect, sizeof(decoy_rect));
            write_at(storage, 16 + target, fixture.left);
            write_at(storage, 16 + target + 4, fixture.top);
            write_at(storage, 16 + target + 8, fixture.right);
            write_at(storage, 16 + target + 12, fixture.bottom);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t dx = fixture.x - fixture.left;
            const uint32_t dy = fixture.y - fixture.top;
            write_at(expected, 16 + target, fixture.x);
            write_at(expected, 16 + target + 4, fixture.y);
            write_at(expected, 16 + target + 8, fixture.right + dx);
            write_at(expected, 16 + target + 12, fixture.bottom + dy);

            auto *window = reinterpret_cast<Win *>(storage + 16);
            const int result = use_adapter
                ? win_move_redirect(window, nullptr,
                                    int_from_bits(fixture.x), int_from_bits(fixture.y))
                : window->move(int_from_bits(fixture.x), int_from_bits(fixture.y));
            expect(result == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_win_paging() {
    const uint32_t values[] = {
        0U, 1U, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xA55AA55AU,
    };
    for (int vertical = 0; vertical < 2; ++vertical) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            for (uint32_t bits : values) {
                alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
                alignas(Scroll) uint8_t vertical_storage[sizeof(Scroll) + 32];
                alignas(Scroll) uint8_t horizontal_storage[sizeof(Scroll) + 32];
                uint8_t win_expected[sizeof(win_storage)];
                uint8_t vertical_expected[sizeof(vertical_storage)];
                uint8_t horizontal_expected[sizeof(horizontal_storage)];
                seed_storage(win_storage, win_expected, sizeof(win_storage));
                seed_storage(vertical_storage, vertical_expected,
                             sizeof(vertical_storage));
                seed_storage(horizontal_storage, horizontal_expected,
                             sizeof(horizontal_storage));
                auto *vertical_scroll = reinterpret_cast<Scroll *>(
                    vertical_storage + 16);
                auto *horizontal_scroll = reinterpret_cast<Scroll *>(
                    horizontal_storage + 16);
                write_at(win_storage, 16 + 0x43C, vertical_scroll);
                write_at(win_storage, 16 + 0x440, horizontal_scroll);
                std::memcpy(win_expected, win_storage, sizeof(win_storage));
                std::memcpy(vertical_expected, vertical_storage,
                            sizeof(vertical_storage));
                std::memcpy(horizontal_expected, horizontal_storage,
                            sizeof(horizontal_storage));
                uint8_t *selected_expected = vertical
                    ? vertical_expected : horizontal_expected;
                write_at(selected_expected, 16 + 0xA40, bits);

                auto *window = reinterpret_cast<Win *>(win_storage + 16);
                const int paging = int_from_bits(bits);
                if (vertical) {
                    if (use_adapter) {
                        win_set_vert_paging_redirect(window, nullptr, paging);
                    } else {
                        window->set_vert_paging(paging);
                    }
                } else if (use_adapter) {
                    win_set_horz_paging_redirect(window, nullptr, paging);
                } else {
                    window->set_horz_paging(paging);
                }
                expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));
                expect_storage_bytes(vertical_storage, vertical_expected,
                                     sizeof(vertical_storage));
                expect_storage_bytes(horizontal_storage, horizontal_expected,
                                     sizeof(horizontal_storage));
            }
        }
    }

    for (int vertical = 0; vertical < 2; ++vertical) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Win) uint8_t null_storage[sizeof(Win) + 32];
            uint8_t null_expected[sizeof(null_storage)];
            seed_storage(null_storage, null_expected, sizeof(null_storage));
            Scroll *null_scroll = nullptr;
            write_at(null_storage, 16 + (vertical ? 0x43C : 0x440), null_scroll);
            std::memcpy(null_expected, null_storage, sizeof(null_storage));
            auto *null_window = reinterpret_cast<Win *>(null_storage + 16);
            if (vertical) {
                if (use_adapter) {
                    win_set_vert_paging_redirect(null_window, nullptr, INT_MIN);
                } else {
                    null_window->set_vert_paging(INT_MIN);
                }
            } else if (use_adapter) {
                win_set_horz_paging_redirect(null_window, nullptr, INT_MAX);
            } else {
                null_window->set_horz_paging(INT_MAX);
            }
            expect_storage_bytes(null_storage, null_expected, sizeof(null_storage));
        }
    }

    alignas(Win) uint8_t alias_win_storage[sizeof(Win) + 32];
    alignas(Scroll) uint8_t alias_scroll_storage[sizeof(Scroll) + 32];
    uint8_t alias_win_expected[sizeof(alias_win_storage)];
    uint8_t alias_scroll_expected[sizeof(alias_scroll_storage)];
    seed_storage(alias_win_storage, alias_win_expected, sizeof(alias_win_storage));
    seed_storage(alias_scroll_storage, alias_scroll_expected,
                 sizeof(alias_scroll_storage));
    auto *alias_scroll = reinterpret_cast<Scroll *>(alias_scroll_storage + 16);
    write_at(alias_win_storage, 16 + 0x43C, alias_scroll);
    write_at(alias_win_storage, 16 + 0x440, alias_scroll);
    std::memcpy(alias_win_expected, alias_win_storage, sizeof(alias_win_storage));
    std::memcpy(alias_scroll_expected, alias_scroll_storage,
                sizeof(alias_scroll_storage));
    const uint32_t alias_value = 0x89ABCDEFU;
    write_at(alias_scroll_expected, 16 + 0xA40, alias_value);
    auto *alias_window = reinterpret_cast<Win *>(alias_win_storage + 16);
    win_set_horz_paging_redirect(
        alias_window, nullptr, int_from_bits(alias_value));
    expect_storage_bytes(alias_win_storage, alias_win_expected,
                         sizeof(alias_win_storage));
    expect_storage_bytes(alias_scroll_storage, alias_scroll_expected,
                         sizeof(alias_scroll_storage));
}

void test_scroll_border_color() {
    const uint32_t colors[] = {
        0xFFFFFFFFU, 0U, 1U, 0x80000000U, 0x7FFFFFFFU,
    };
    const uint32_t thicknesses[] = {
        0xFFFFFFFFU, 0U, 1U, 2U, 0x80000000U, 0x7FFFFFFFU,
    };
    for (uint32_t color : colors) {
        for (uint32_t thickness : thicknesses) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));
                const uint32_t old_color = 0x13579BDFU;
                const uint32_t old_rect[4] = {
                    0x11111111U, 0x22222222U, 0x33333333U, 0x44444444U,
                };
                write_at(storage, 16 + 0xA1C, old_color);
                std::memcpy(storage + 16 + 0xA4C, old_rect, sizeof(old_rect));
                write_at(storage, 16 + 0xA60, thickness);
                std::memcpy(expected, storage, sizeof(storage));
                const uint32_t inset = color == 0xFFFFFFFFU ? 0U : 1U;
                const uint32_t extent = color == 0xFFFFFFFFU
                    ? thickness : thickness - 1U;
                write_at(expected, 16 + 0xA1C, color);
                write_at(expected, 16 + 0xA4C, inset);
                write_at(expected, 16 + 0xA50, inset);
                write_at(expected, 16 + 0xA54, extent);
                write_at(expected, 16 + 0xA58, extent);

                auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                if (use_adapter) {
                    scroll_set_border_color_redirect(
                        scroll, nullptr, int_from_bits(color));
                } else {
                    scroll->set_border_color(int_from_bits(color));
                }
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

void test_scroll_range() {
    struct RangeCase {
        uint32_t first;
        uint32_t second;
        uint32_t minimum;
        uint32_t maximum;
        uint32_t reverse;
    };
    const RangeCase cases[] = {
        {10U, 20U, 10U, 20U, 0U},
        {20U, 10U, 10U, 20U, 1U},
        {5U, 5U, 5U, 5U, 1U},
        {0xFFFFFFFFU, 0U, 0xFFFFFFFFU, 0U, 0U},
        {0x80000000U, 0x7FFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0U},
        {0x7FFFFFFFU, 0x80000000U, 0x80000000U, 0x7FFFFFFFU, 1U},
    };
    for (const RangeCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            uintptr_t vtable[63];
            seed_storage(storage, expected, sizeof(storage));
            install_scroll_redraw_probe(storage, vtable);
            std::memcpy(expected, storage, sizeof(storage));
            write_at(expected, 16 + 0xA20, fixture.minimum);
            write_at(expected, 16 + 0xA24, fixture.maximum);
            write_at(expected, 16 + 0xA28, fixture.reverse);
            write_at(expected, 16 + 0xA2C, fixture.minimum);

            reset_scroll_redraw_probe();
            scroll_redraw_result = fixture.first ^ fixture.second ^ 0xA55AA55AU;
            scroll_redraw_minimum = fixture.minimum;
            scroll_redraw_maximum = fixture.maximum;
            scroll_redraw_reverse = fixture.reverse;
            scroll_redraw_position = fixture.minimum;
            scroll_redraw_check_range = true;
            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            const uint32_t result = use_adapter
                ? scroll_set_range_redirect(
                    scroll, nullptr, int_from_bits(fixture.first),
                    int_from_bits(fixture.second))
                : scroll->set_range(int_from_bits(fixture.first),
                                    int_from_bits(fixture.second));
            expect(result == scroll_redraw_result);
            expect(scroll_redraw_calls == 1);
            expect(scroll_redraw_self == scroll);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_scroll_style_setters() {
    struct StyleCase {
        size_t primary_offset;
        size_t left_offset;
        size_t right_offset;
    };
    const StyleCase styles[] = {
        {0xA5C, 0x1530, 0x207C},
        {0xA68, 0x153C, 0x2088},
        {0xA6C, 0x1534, 0x2080},
        {0xA70, 0x1538, 0x2084},
    };
    const uint32_t values[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };

    for (size_t style = 0; style < ARRAYSIZE(styles); ++style) {
        for (uint32_t value : values) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                uint8_t expected[sizeof(storage)];
                uintptr_t left_vtable[63];
                uintptr_t right_vtable[63];
                seed_storage(storage, expected, sizeof(storage));
                install_scroll_style_redraw_probes(
                    storage, left_vtable, right_vtable);
                std::memcpy(expected, storage, sizeof(storage));
                write_at(expected, 16 + styles[style].primary_offset, value);
                write_at(expected, 16 + styles[style].left_offset, value);
                write_at(expected, 16 + styles[style].right_offset, value);

                scroll_style_redraw_calls = 0;
                scroll_style_base = storage + 16;
                scroll_style_offsets[0] = styles[style].primary_offset;
                scroll_style_offsets[1] = styles[style].left_offset;
                scroll_style_offsets[2] = styles[style].right_offset;
                scroll_style_value = value;
                scroll_style_results[0] = value ^ 0x13579BDFU;
                scroll_style_results[1] = value ^ 0x2468ACE0U;

                auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                const int argument = int_from_bits(value);
                uint32_t result;
                switch (style) {
                  case 0:
                    result = use_adapter
                        ? scroll_set_button_color_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_button_color(argument);
                    break;
                  case 1:
                    result = use_adapter
                        ? scroll_set_bevel_thickness_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_thickness(argument);
                    break;
                  case 2:
                    result = use_adapter
                        ? scroll_set_bevel_upper_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_upper(argument);
                    break;
                  default:
                    result = use_adapter
                        ? scroll_set_bevel_lower_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_lower(argument);
                    break;
                }
                expect(result == scroll_style_results[1]);
                expect(scroll_style_redraw_calls == 2);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

void test_scroll_thumb_resetters() {
    const uint32_t colors[] = {
        0xFFFFFFFFU, 0U, 0x80000000U,
    };
    const uint32_t thicknesses[] = {
        0U, 1U, 2U, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t color : colors) {
        for (uint32_t thickness : thicknesses) {
            for (int set_bar = 0; set_bar < 2; ++set_bar) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                    uint8_t expected[sizeof(storage)];
                    seed_storage(storage, expected, sizeof(storage));
                    const uint32_t old_thickness = 0x13579BDFU;
                    const uint32_t selected_thickness = set_bar
                        ? thickness : old_thickness;
                    write_at(storage, 16 + 0xA1C, color);
                    write_at(storage, 16 + 0xA60, old_thickness);
                    std::memcpy(expected, storage, sizeof(storage));
                    if (set_bar) {
                        write_at(expected, 16 + 0xA60, thickness);
                    }
                    const uint32_t inset = color == 0xFFFFFFFFU ? 0U : 1U;
                    const uint32_t extent = color == 0xFFFFFFFFU
                        ? selected_thickness : selected_thickness - 1U;
                    write_at(expected, 16 + 0xA4C, inset);
                    write_at(expected, 16 + 0xA50, inset);
                    write_at(expected, 16 + 0xA54, extent);
                    write_at(expected, 16 + 0xA58, extent);

                    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                    uint32_t result;
                    if (set_bar && use_adapter) {
                        result = scroll_set_bar_thickness_redirect(
                            scroll, nullptr, int_from_bits(thickness));
                    } else if (set_bar) {
                        result = scroll->set_bar_thickness(
                            int_from_bits(thickness));
                    } else if (use_adapter) {
                        result = scroll_set_thumb_rect_redirect(scroll, nullptr);
                    } else {
                        result = scroll->set_thumb_rect();
                    }
                    expect(result == (color == 0xFFFFFFFFU
                        ? (set_bar ? color : selected_thickness)
                        : selected_thickness - 1U));
                    expect_storage_bytes(storage, expected, sizeof(storage));
                }
            }
        }
    }
}

void test_scroll_position() {
    Win **const saved_current_win = ScrollCurrentWin;
    Win *published = reinterpret_cast<Win *>(0x45454545U);
    ScrollCurrentWin = &published;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        Win *null_parent = nullptr;
        write_at(storage, 16 + 0xC4, null_parent);
        std::memcpy(expected, storage, sizeof(storage));
        published = reinterpret_cast<Win *>(0x45454545U);
        reset_scroll_redraw_probe();
        auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
        const uint32_t result = use_adapter
            ? scroll_set_pos_redirect(scroll, nullptr, INT_MIN)
            : scroll->set_pos(INT_MIN);
        expect(result == 0U);
        expect(scroll_redraw_calls == 0);
        expect(published == reinterpret_cast<Win *>(0x45454545U));
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    struct PositionCase {
        uint32_t minimum;
        uint32_t maximum;
        uint32_t reverse;
        uint32_t input;
        uint32_t expected;
    };
    const PositionCase cases[] = {
        {0U, 100U, 0U, 0xFFFFFFFFU, 0U},
        {0U, 100U, 0U, 101U, 100U},
        {0xFFFFFFF6U, 10U, 0U, 0xFFFFFFFDU, 0xFFFFFFFDU},
        {0U, 100U, 1U, 25U, 75U},
        {0U, 100U, 0x80000000U, 1000U, 0U},
        {0x80000000U, 0x7FFFFFFFU, 1U, 0U, 0xFFFFFFFFU},
    };
    alignas(Win) uint8_t parent_storage[sizeof(Win)];
    auto *parent = reinterpret_cast<Win *>(parent_storage);
    for (const PositionCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            uintptr_t vtable[63];
            seed_storage(storage, expected, sizeof(storage));
            install_scroll_redraw_probe(storage, vtable);
            write_at(storage, 16 + 0xC4, parent);
            write_at(storage, 16 + 0xA20, fixture.minimum);
            write_at(storage, 16 + 0xA24, fixture.maximum);
            write_at(storage, 16 + 0xA28, fixture.reverse);
            std::memcpy(expected, storage, sizeof(storage));
            write_at(expected, 16 + 0xA2C, fixture.expected);

            published = reinterpret_cast<Win *>(0x45454545U);
            reset_scroll_redraw_probe();
            scroll_redraw_result = fixture.input ^ 0x5AA55AA5U;
            scroll_redraw_position = fixture.expected;
            scroll_redraw_parent = parent;
            scroll_redraw_check_current = true;
            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            const uint32_t result = use_adapter
                ? scroll_set_pos_redirect(
                    scroll, nullptr, int_from_bits(fixture.input))
                : scroll->set_pos(int_from_bits(fixture.input));
            expect(result == scroll_redraw_result);
            expect(scroll_redraw_calls == 1);
            expect(scroll_redraw_self == scroll);
            expect(published == parent);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
    ScrollCurrentWin = saved_current_win;
}

void test_expand_rect() {
    struct ExpandRectCase {
        uint32_t rect[4];
        uint32_t horizontal;
        uint32_t vertical;
    };
    const ExpandRectCase cases[] = {
        {{0U, 0U, 0U, 0U}, 0U, 0U},
        {{10U, 20U, 30U, 40U}, 3U, 4U},
        {{10U, 20U, 30U, 40U}, 0xFFFFFFFFU, 0xFFFFFFFEU},
        {{0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0U},
         0x80000000U, 0x7FFFFFFFU},
        {{0U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
         0x7FFFFFFFU, 0x80000000U},
    };

    for (const ExpandRectCase &test : cases) {
        alignas(RECT) uint8_t storage[sizeof(RECT) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(storage + 16, test.rect, sizeof(test.rect));
        std::memcpy(expected, storage, sizeof(storage));
        uint32_t expected_rect[4];
        std::memcpy(expected_rect, test.rect, sizeof(expected_rect));
        expected_rect[0] -= test.horizontal;
        expected_rect[2] += test.horizontal;
        expected_rect[1] -= test.vertical;
        expected_rect[3] += test.vertical;
        std::memcpy(expected + 16, expected_rect, sizeof(expected_rect));

        auto *rect = reinterpret_cast<RECT *>(storage + 16);
        expect(expand_rect(rect, int_from_bits(test.horizontal),
                           int_from_bits(test.vertical)) == rect);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_scroll_sprite_setters() {
    struct Geometry {
        uint32_t width;
        uint32_t stored_height;
        bool horizontal;
        bool vertical;
    };
    const Geometry geometries[] = {
        {100U, 0xFFFFFFECU, true, false},
        {20U, 0xFFFFFF9CU, false, true},
        {20U, 0xFFFFFFECU, false, false},
        {1U, 1U, true, false},
        {0x80000001U, 0x80000000U, true, false},
        {0x80000000U, 0x80000000U, false, false},
    };
    Sprite *sprite_sets[][3] = {
        {nullptr, nullptr, nullptr},
        {reinterpret_cast<Sprite *>(0x10101010U),
         reinterpret_cast<Sprite *>(0x20202020U),
         reinterpret_cast<Sprite *>(0x30303030U)},
        {reinterpret_cast<Sprite *>(0x45454545U),
         reinterpret_cast<Sprite *>(0x56565656U),
         reinterpret_cast<Sprite *>(0x45454545U)},
    };

    for (const Geometry &geometry : geometries) {
        for (Sprite **sprites : sprite_sets) {
            for (int direction = 0; direction < 4; ++direction) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                    uint8_t expected[sizeof(storage)];
                    seed_storage(storage, expected, sizeof(storage));
                    write_at(storage, 16 + 0x4C4, geometry.width);
                    write_at(storage, 16 + 0x4C8, geometry.stored_height);
                    std::memcpy(expected, storage, sizeof(storage));

                    const size_t primary_offsets[] = {
                        0xA7C, 0xA94, 0xA88, 0xAA0,
                    };
                    const size_t primary_offset = primary_offsets[direction];
                    for (size_t index = 0; index < 3; ++index) {
                        write_at(expected, 16 + primary_offset
                            + index * sizeof(Sprite *), sprites[index]);
                    }
                    const bool update_button = direction < 2
                        ? geometry.horizontal : geometry.vertical;
                    if (update_button) {
                        const size_t button_offset = (direction & 1)
                            ? 0x2108 : 0x15BC;
                        for (size_t index = 0; index < 3; ++index) {
                            write_at(expected, 16 + button_offset
                                + index * sizeof(Sprite *), sprites[index]);
                        }
                    }

                    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                    switch (direction) {
                      case 0:
                        if (use_adapter) {
                            expect(scroll_set_sprite_left_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_left(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      case 1:
                        if (use_adapter) {
                            expect(scroll_set_sprite_right_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_right(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      case 2:
                        if (use_adapter) {
                            expect(scroll_set_sprite_up_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_up(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      default:
                        if (use_adapter) {
                            expect(scroll_set_sprite_down_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_down(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                    }
                    expect_storage_bytes(storage, expected, sizeof(storage));
                }
            }
        }
    }
}

struct ThumbRectCase {
    const char *name;
    uint32_t flags;
    uint32_t border_color;
    uint32_t width;
    uint32_t stored_height;
    uint32_t minimum;
    uint32_t maximum;
    uint32_t position;
    uint32_t drag_coordinate;
    uint32_t rect[4];
};

uint32_t arithmetic_shift_right_one(uint32_t value) {
    return (value >> 1U) | (value & 0x80000000U);
}

uint32_t signed_min_bits(uint32_t left, uint32_t right) {
    return int_from_bits(left) < int_from_bits(right) ? left : right;
}

void expected_thumb_rect(
        const ThumbRectCase &fixture, uint32_t rect[4], uint32_t &drag_coordinate) {
    rect[2] -= rect[0];
    rect[0] = 0;
    rect[3] -= rect[1];
    rect[1] = 0;
    if (fixture.border_color != 0xFFFFFFFFU) {
        for (int index = 0; index < 4; ++index) {
            ++rect[index];
        }
    }

    const uint32_t extent = rect[2] - rect[0];
    const uint32_t height = 0U - fixture.stored_height;
    const bool horizontal = int_from_bits(fixture.width) > int_from_bits(height);
    const uint32_t axis_length = horizontal ? fixture.width : height;
    const bool no_end_buttons = (fixture.flags & 2U) != 0;
    uint32_t offset;

    if (drag_coordinate == 0xFFFFFFFFU) {
        offset = no_end_buttons ? 0U : extent + 1U;
        uint32_t adjustment = 0xFFFFFFFFU - (no_end_buttons
            ? extent : extent * 3U);
        if (fixture.border_color != 0xFFFFFFFFU) {
            adjustment -= 2U;
        }
        if (fixture.maximum != fixture.minimum) {
            const uint32_t numerator = (fixture.position - fixture.minimum)
                * (axis_length + adjustment);
            const int64_t quotient = static_cast<int64_t>(int_from_bits(numerator))
                / static_cast<int64_t>(
                    int_from_bits(fixture.maximum - fixture.minimum));
            offset += static_cast<uint32_t>(quotient);
        }
    } else {
        const uint32_t far_edge = rect[0] + extent;
        const uint32_t candidate = drag_coordinate
            + arithmetic_shift_right_one(extent);
        const uint32_t upper = axis_length - (no_end_buttons
            ? far_edge : far_edge * 2U);
        const uint32_t limited = signed_min_bits(candidate, upper);
        if (no_end_buttons) {
            offset = int_from_bits(limited) < 0 ? 0U : limited;
        } else if (int_from_bits(far_edge) > int_from_bits(limited)) {
            offset = far_edge;
        } else {
            offset = limited;
        }
        drag_coordinate = 0xFFFFFFFFU;
    }

    if (horizontal) {
        rect[0] += offset;
        rect[2] += offset;
    } else {
        rect[1] += offset;
        rect[3] += offset;
    }
}

void initialize_thumb_case(uint8_t *storage, const ThumbRectCase &fixture) {
    write_at(storage, 16 + 0x4C4, fixture.width);
    write_at(storage, 16 + 0x4C8, fixture.stored_height);
    write_at(storage, 16 + 0xA14, fixture.flags);
    write_at(storage, 16 + 0xA1C, fixture.border_color);
    write_at(storage, 16 + 0xA20, fixture.minimum);
    write_at(storage, 16 + 0xA24, fixture.maximum);
    write_at(storage, 16 + 0xA2C, fixture.position);
    write_at(storage, 16 + 0xA3C, fixture.drag_coordinate);
    std::memcpy(storage + 16 + 0xA4C, fixture.rect, sizeof(fixture.rect));
}

void test_scroll_compute_thumb_rect() {
    const ThumbRectCase cases[] = {
        {"horizontal no-buttons minimum", 0xA5A50002U, 0xFFFFFFFFU,
         200U, 0xFFFFFFECU, 0U, 100U, 0U, 0xFFFFFFFFU,
         {3U, 4U, 13U, 14U}},
        {"horizontal buttons midpoint", 0xA5A50000U, 0U,
         300U, 0xFFFFFFECU, 0xFFFFFFF6U, 10U, 0U, 0xFFFFFFFFU,
         {5U, 7U, 25U, 37U}},
        {"horizontal outside range", 0U, 1U,
         100U, 0xFFFFFFF0U, 0U, 10U, 15U, 0xFFFFFFFFU,
         {0U, 0U, 8U, 12U}},
        {"vertical no-buttons truncation", 2U, 0xFFFFFFFFU,
         20U, 0xFFFFFF38U, 0U, 8U, 3U, 0xFFFFFFFFU,
         {9U, 11U, 19U, 41U}},
        {"equal dimensions choose vertical", 0U, 0U,
         20U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFFFFU,
         {2U, 3U, 12U, 23U}},
        {"equal endpoints", 0U, 0xFFFFFFFFU,
         120U, 0xFFFFFFF0U, 7U, 7U, 0x80000000U, 0xFFFFFFFFU,
         {4U, 8U, 16U, 24U}},
        {"reversed range", 2U, 0U,
         160U, 0xFFFFFFE0U, 10U, 0xFFFFFFF6U, 0U, 0xFFFFFFFFU,
         {6U, 9U, 17U, 30U}},
        {"wrapping static arithmetic", 0U, 0x80000000U,
         0x7FFFFFF0U, 0xFFFFFFE0U, 0x70000000U, 0x90000000U,
         0x80000001U, 0xFFFFFFFFU,
         {0x70000000U, 0x11111111U, 0xF0000010U, 0x33333333U}},
        {"horizontal drag below", 2U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFF80U,
         {3U, 4U, 13U, 14U}},
        {"horizontal drag above", 2U, 0U,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 200U,
         {3U, 4U, 13U, 14U}},
        {"horizontal drag buttons inside", 0U, 0xFFFFFFFFU,
         140U, 0xFFFFFFECU, 0U, 10U, 5U, 37U,
         {2U, 4U, 14U, 20U}},
        {"horizontal drag crossed bounds", 0U, 0U,
         10U, 0xFFFFFFFCU, 0U, 10U, 5U, 0U,
         {0U, 0U, 20U, 6U}},
        {"vertical drag no-buttons", 2U, 0xFFFFFFFFU,
         12U, 0xFFFFFF80U, 0U, 10U, 5U, 57U,
         {1U, 4U, 9U, 40U}},
        {"vertical drag buttons", 0U, 0U,
         12U, 0xFFFFFF80U, 0U, 10U, 5U, 90U,
         {1U, 4U, 9U, 40U}},
        {"vertical nonsquare extent", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF00U, 0U, 10U, 5U, 20U,
         {7U, 9U, 12U, 109U}},
        {"horizontal wrapped lower", 0U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
        {"vertical wrapped lower", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF9CU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
    };

    for (const ThumbRectCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            alignas(RECT) uint8_t output_storage[sizeof(RECT) + 32];
            uint8_t output_expected[sizeof(output_storage)];
            seed_storage(storage, expected, sizeof(storage));
            seed_storage(output_storage, output_expected, sizeof(output_storage));
            initialize_thumb_case(storage, fixture);
            std::memcpy(expected, storage, sizeof(storage));

            uint32_t expected_rect[4];
            std::memcpy(expected_rect, fixture.rect, sizeof(expected_rect));
            uint32_t expected_drag = fixture.drag_coordinate;
            expected_thumb_rect(fixture, expected_rect, expected_drag);
            write_at(expected, 16 + 0xA3C, expected_drag);
            std::memcpy(expected + 16 + 0xA4C,
                        expected_rect, sizeof(expected_rect));
            std::memcpy(output_expected + 16,
                        expected_rect, sizeof(expected_rect));

            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            auto *output = reinterpret_cast<RECT *>(output_storage + 16);
            const int failures_before = failures;
            if (use_adapter) {
                expect(scroll_compute_thumb_rect_redirect(
                           scroll, nullptr, output)
                       == reinterpret_cast<RECT *>(storage + 16 + 0xA4C));
            } else {
                scroll->compute_thumb_rect(output);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect_storage_bytes(
                output_storage, output_expected, sizeof(output_storage));
            if (failures != failures_before) {
                report_storage_mismatch(
                    fixture.name, storage, expected, sizeof(storage));
                report_storage_mismatch(fixture.name, output_storage,
                                        output_expected, sizeof(output_storage));
            }
        }
    }

    alignas(Scroll) uint8_t alias_storage[sizeof(Scroll) + 32];
    uint8_t alias_expected[sizeof(alias_storage)];
    seed_storage(alias_storage, alias_expected, sizeof(alias_storage));
    initialize_thumb_case(alias_storage, cases[1]);
    std::memcpy(alias_expected, alias_storage, sizeof(alias_storage));
    uint32_t alias_rect[4];
    std::memcpy(alias_rect, cases[1].rect, sizeof(alias_rect));
    uint32_t alias_drag = cases[1].drag_coordinate;
    expected_thumb_rect(cases[1], alias_rect, alias_drag);
    write_at(alias_expected, 16 + 0xA3C, alias_drag);
    std::memcpy(alias_expected + 16 + 0xA4C, alias_rect, sizeof(alias_rect));
    auto *alias_scroll = reinterpret_cast<Scroll *>(alias_storage + 16);
    auto *alias_rect_pointer = reinterpret_cast<RECT *>(
        alias_storage + 16 + 0xA4C);
    expect(scroll_compute_thumb_rect_redirect(
               alias_scroll, nullptr, alias_rect_pointer) == alias_rect_pointer);
    expect_storage_bytes(alias_storage, alias_expected, sizeof(alias_storage));
}

enum class PullDownOperation {
    Hide,
    Show,
    Disable,
    Enable,
    Check,
    Uncheck,
};

int call_pull_down_mutator(
        PullDown *pull_down, PullDownOperation operation, int id, bool adapter) {
    switch (operation) {
      case PullDownOperation::Hide:
        return adapter
            ? pull_down_hide_item_redirect(pull_down, nullptr, id)
            : pull_down->hide_item(id);
      case PullDownOperation::Show:
        return adapter
            ? pull_down_show_item_redirect(pull_down, nullptr, id)
            : pull_down->show_item(id);
      case PullDownOperation::Disable:
        return adapter
            ? pull_down_disable_item_redirect(pull_down, nullptr, id)
            : pull_down->disable_item(id);
      case PullDownOperation::Enable:
        return adapter
            ? pull_down_enable_item_redirect(pull_down, nullptr, id)
            : pull_down->enable_item(id);
      case PullDownOperation::Check:
        return adapter
            ? pull_down_check_item_redirect(pull_down, nullptr, id)
            : pull_down->check_item(id);
      case PullDownOperation::Uncheck:
        return adapter
            ? pull_down_uncheck_item_redirect(pull_down, nullptr, id)
            : pull_down->uncheck_item(id);
    }
    return -1;
}

uint32_t initial_pull_down_flags(PullDownOperation operation) {
    const uint32_t preserved = 0xA5A50004U;
    switch (operation) {
      case PullDownOperation::Hide:
      case PullDownOperation::Enable:
      case PullDownOperation::Uncheck:
        return preserved | (operation == PullDownOperation::Hide ? 1U
            : operation == PullDownOperation::Enable ? 2U : 8U);
      case PullDownOperation::Show:
      case PullDownOperation::Disable:
      case PullDownOperation::Check:
        return preserved;
    }
    return preserved;
}

uint32_t expected_pull_down_flags(
        PullDownOperation operation, uint32_t flags) {
    switch (operation) {
      case PullDownOperation::Hide:
        return flags & ~1U;
      case PullDownOperation::Show:
        return flags | 1U;
      case PullDownOperation::Disable:
        return flags | 2U;
      case PullDownOperation::Enable:
        return flags & ~2U;
      case PullDownOperation::Check:
        return flags | 8U;
      case PullDownOperation::Uncheck:
        return flags & ~8U;
    }
    return flags;
}

void fill_pull_down_ids(uint8_t *storage) {
    for (int index = 0; index < 64; ++index) {
        const int id = 1000 + index;
        write_at(storage, 16 + 0xA20 + static_cast<size_t>(index) * 0x14, id);
    }
}

void test_pull_down_item_state() {
    const PullDownOperation operations[] = {
        PullDownOperation::Hide,
        PullDownOperation::Show,
        PullDownOperation::Disable,
        PullDownOperation::Enable,
        PullDownOperation::Check,
        PullDownOperation::Uncheck,
    };
    const int target_indices[] = {0, 31, 63};
    const int target_ids[] = {INT_MIN, 17, INT_MAX};
    for (PullDownOperation operation : operations) {
        for (int target_case = 0; target_case < 3; ++target_case) {
            for (int adapter = 0; adapter < 2; ++adapter) {
                alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));
                fill_pull_down_ids(storage);
                const int target_index = target_indices[target_case];
                const int target_id = target_ids[target_case];
                const size_t item_offset = 0xA18
                    + static_cast<size_t>(target_index) * 0x14;
                write_at(storage, 16 + item_offset + 8, target_id);
                const uint32_t flags = initial_pull_down_flags(operation);
                write_at(storage, 16 + item_offset + 0xC, flags);
                const int ignored_count = INT_MIN;
                write_at(storage, 16 + 0xF20, ignored_count);
                const uint32_t visible_bits = operation == PullDownOperation::Hide
                    ? 0U : operation == PullDownOperation::Show
                    ? 0x7FFFFFFFU : 0x13579BDFU;
                write_at(storage, 16 + 0xF2C, visible_bits);
                storage[16 + 0xF34] = 0x7E;
                std::memcpy(expected, storage, sizeof(storage));
                write_at(expected, 16 + item_offset + 0xC,
                         expected_pull_down_flags(operation, flags));
                if (operation == PullDownOperation::Hide) {
                    const uint32_t decremented = visible_bits - 1U;
                    write_at(expected, 16 + 0xF2C, decremented);
                } else if (operation == PullDownOperation::Show) {
                    const uint32_t incremented = visible_bits + 1U;
                    write_at(expected, 16 + 0xF2C, incremented);
                }
                expected[16 + 0xF34] = 1;

                auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
                expect(call_pull_down_mutator(
                           pull_down, operation, target_id, adapter != 0) == 0);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }

    for (PullDownOperation operation : operations) {
        for (int miss_case = 0; miss_case < 3; ++miss_case) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            fill_pull_down_ids(storage);
            int requested_id = 77;
            if (miss_case == 0) {
                const int sentinel = -1;
                write_at(storage, 16 + 0xA20 + 0x14, sentinel);
                write_at(storage, 16 + 0xA20 + 2 * 0x14, requested_id);
            } else if (miss_case == 1) {
                requested_id = -1;
            } else {
                requested_id = 999;
            }
            const int ignored_count = INT_MAX;
            write_at(storage, 16 + 0xF20, ignored_count);
            std::memcpy(expected, storage, sizeof(storage));
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            expect(call_pull_down_mutator(
                       pull_down, operation, requested_id, false) == 11);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    for (PullDownOperation operation : {
            PullDownOperation::Hide, PullDownOperation::Show}) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            fill_pull_down_ids(storage);
            const int id = 1002;
            const uint32_t flags = operation == PullDownOperation::Hide
                ? 0xA5A50004U : 0xA5A50005U;
            write_at(storage, 16 + 0xA24 + 2 * 0x14, flags);
            const uint32_t visible = 0x89ABCDEFU;
            write_at(storage, 16 + 0xF2C, visible);
            std::memcpy(expected, storage, sizeof(storage));
            expected[16 + 0xF34] = 1;
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            expect(call_pull_down_mutator(
                       pull_down, operation, id, adapter != 0) == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    alignas(PullDown) uint8_t duplicate_storage[sizeof(PullDown) + 32];
    uint8_t duplicate_expected[sizeof(duplicate_storage)];
    seed_storage(duplicate_storage, duplicate_expected, sizeof(duplicate_storage));
    fill_pull_down_ids(duplicate_storage);
    const int duplicate_id = 12345;
    write_at(duplicate_storage, 16 + 0xA20 + 1 * 0x14, duplicate_id);
    write_at(duplicate_storage, 16 + 0xA20 + 40 * 0x14, duplicate_id);
    const uint32_t duplicate_flags = 0x40U;
    write_at(duplicate_storage, 16 + 0xA24 + 1 * 0x14, duplicate_flags);
    write_at(duplicate_storage, 16 + 0xA24 + 40 * 0x14, duplicate_flags);
    std::memcpy(duplicate_expected, duplicate_storage, sizeof(duplicate_storage));
    const uint32_t disabled_flags = duplicate_flags | 2U;
    write_at(duplicate_expected, 16 + 0xA24 + 1 * 0x14, disabled_flags);
    duplicate_expected[16 + 0xF34] = 1;
    auto *duplicate_pull_down = reinterpret_cast<PullDown *>(duplicate_storage + 16);
    expect(pull_down_disable_item_redirect(
               duplicate_pull_down, nullptr, duplicate_id) == 0);
    expect_storage_bytes(
        duplicate_storage, duplicate_expected, sizeof(duplicate_storage));
}

void test_pull_down_get_selected() {
    struct SelectionCase {
        int index;
        bool disabled;
    };
    const SelectionCase cases[] = {
        {-1, false}, {0, false}, {31, true}, {63, false},
        {-2, false}, {64, true}, {65, false}, {INT_MIN, true}, {INT_MAX, false},
    };
    for (const SelectionCase &fixture : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0xF28, fixture.index);
            if (fixture.index != -1) {
                const uint32_t offset = 0xA24U
                    + static_cast<uint32_t>(fixture.index) * 0x14U;
                const uint32_t flags = fixture.disabled ? 0xA5A50002U : 0xA5A5000CU;
                write_at(storage, 16 + offset, flags);
            }
            std::memcpy(expected, storage, sizeof(storage));
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            const int result = adapter
                ? pull_down_get_selected_redirect(pull_down, nullptr)
                : pull_down->get_selected();
            expect(result == (fixture.index == -1 || fixture.disabled
                              ? -1 : fixture.index));
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void __cdecl menu_proc_fixture(int) {}

void test_menu_accessors() {
    for (MenuProc proc : {static_cast<MenuProc>(nullptr), &menu_proc_fixture}) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(expected, 16 + 0xA14, proc);
            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            if (adapter) {
                expect(menu_set_menu_proc_redirect(menu, nullptr, proc) == proc);
            } else {
                menu->set_menu_proc(proc);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    struct LookupCase {
        int target_index;
        int id;
    };
    const LookupCase lookup_cases[] = {
        {0, INT_MIN}, {7, 17}, {14, INT_MAX},
    };
    for (const LookupCase &fixture : lookup_cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            for (int index = 0; index < 15; ++index) {
                const int id = 1000 + index;
                write_at(storage, 16 + 0xA38 + index * 0x14, id);
            }
            write_at(storage, 16 + 0xA38 + fixture.target_index * 0x14,
                     fixture.id);
            const int ignored_count = INT_MIN;
            write_at(storage, 16 + 0xA18, ignored_count);
            std::memcpy(expected, storage, sizeof(storage));
            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            const int result = adapter
                ? menu_id_to_index_redirect(menu, nullptr, fixture.id)
                : menu->id_to_index(fixture.id);
            expect(result == fixture.target_index);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    for (int miss_case = 0; miss_case < 4; ++miss_case) {
        alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        for (int index = 0; index < 15; ++index) {
            const int id = 1000 + index;
            write_at(storage, 16 + 0xA38 + index * 0x14, id);
        }
        int requested = 77;
        if (miss_case == 0) {
            const int sentinel = -1;
            write_at(storage, 16 + 0xA38 + 2 * 0x14, sentinel);
            write_at(storage, 16 + 0xA38 + 3 * 0x14, requested);
        } else if (miss_case == 1) {
            requested = -1;
        } else if (miss_case == 2) {
            write_at(storage, 16 + 0xA38 + 1 * 0x14, requested);
            write_at(storage, 16 + 0xA38 + 8 * 0x14, requested);
        } else {
            requested = 999;
        }
        std::memcpy(expected, storage, sizeof(storage));
        auto *menu = reinterpret_cast<Menu *>(storage + 16);
        const int result = menu->id_to_index(requested);
        const int expected_result = miss_case == 2 ? 1 : -1;
        expect(result == expected_result);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void expect_text_constructor_state(uint8_t *expected) {
    const uint32_t zero = 0;
    expected[16] = 0;
    write_at(expected, 16 + 0x150, zero);
    write_at(expected, 16 + 0x154, zero);
    write_at(expected, 16 + 0x158, zero);
    write_at(expected, 16 + 0x15C, zero);
}

void test_text_constructors() {
    static_assert(sizeof(Text) == 0x160, "Text fixture requires the legacy layout");
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expect_text_constructor_state(expected);
    auto *text = new (storage + 16) Text;
    expect_storage_bytes(storage, expected, sizeof(storage));
    text->~Text();

    struct AllocationScenario {
        bool first_succeeds;
        bool second_succeeds;
        int expected_calls;
    };
    const AllocationScenario scenarios[] = {
        {false, false, 1},
        {true, false, 2},
        {true, true, 2},
    };
    constexpr size_t requested_size = 0x1357;
    for (const AllocationScenario &scenario : scenarios) {
        LPVOID first = scenario.first_succeeds ? std::malloc(1) : nullptr;
        LPVOID second = scenario.second_succeeds ? std::malloc(1) : nullptr;
        mem_get_scripted = true;
        mem_get_calls = 0;
        mem_get_sizes[0] = 0;
        mem_get_sizes[1] = 0;
        mem_get_results[0] = first;
        mem_get_results[1] = second;

        seed_storage(storage, expected, sizeof(storage));
        expect_text_constructor_state(expected);
        write_at(expected, 16 + 0x158, first);
        write_at(expected, 16 + 0x15C, second);
        text = new (storage + 16) Text(requested_size);

        expect(mem_get_calls == scenario.expected_calls);
        expect(mem_get_sizes[0] == requested_size);
        if (scenario.expected_calls == 2) {
            expect(mem_get_sizes[1] == requested_size);
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
        text->~Text();
        std::free(first);
        std::free(second);
    }
    mem_get_scripted = false;
    mem_get_calls = 0;
}

void test_text_global_lifecycle() {
    alignas(Text) static uint8_t exit_storage[sizeof(Text)];
    uint8_t expected[sizeof(exit_storage)];
    seed_storage(exit_storage, expected, sizeof(exit_storage));
    expected[0] = 0;
    const uint32_t zero = 0;
    write_at(expected, 0x150, zero);
    write_at(expected, 0x154, zero);

    LPVOID first = std::malloc(1);
    LPVOID second = std::malloc(1);
    write_at(expected, 0x158, first);
    write_at(expected, 0x15C, second);
    Txt = reinterpret_cast<Text *>(exit_storage);
    mem_get_scripted = true;
    mem_get_calls = 0;
    mem_get_results[0] = first;
    mem_get_results[1] = second;
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    atexit_calls = 0;
    atexit_callback = nullptr;

    text_txt();
    expect(mem_get_calls == 2);
    expect(mem_get_sizes[0] == 512);
    expect(mem_get_sizes[1] == 512);
#if defined(__MINGW32__)
    expect(atexit_calls == 1);
    expect(atexit_callback == text_txt_exit);
#endif
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));

    text_txt_exit();
    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == Txt);
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));

    std::free(first);
    std::free(second);

    seed_storage(exit_storage, expected, sizeof(exit_storage));
    expected[0] = 0;
    write_at(expected, 0x150, zero);
    write_at(expected, 0x154, zero);
    write_at(expected, 0x158, zero);
    write_at(expected, 0x15C, zero);
    mem_get_calls = 0;
    mem_get_results[0] = nullptr;
    mem_get_results[1] = reinterpret_cast<LPVOID>(0x13572468U);
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    atexit_calls = 0;
    atexit_callback = nullptr;

    text_txt();
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 512);
#if defined(__MINGW32__)
    expect(atexit_calls == 1);
    expect(atexit_callback == text_txt_exit);
#endif
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));
    text_txt_exit();
    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == Txt);

    mem_get_scripted = false;
    mem_get_calls = 0;
    new (exit_storage) Text;
}

void test_text_destructor_thunk() {
    static_assert(sizeof(Text) == 0x160, "Text fixture requires the legacy layout");
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    auto *text = new (storage + 16) Text;
    std::memcpy(expected, storage, sizeof(storage));
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    text->~Text();

    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == text);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_text_open_wrapper() {
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    auto *text = new (storage + 16) Text;
    std::memcpy(expected, storage, sizeof(storage));
    Txt = text;
    char source[] = "fixture.txt";
    char section[] = "SECTION";
    text_open_calls = 0;
    text_open_this = nullptr;
    text_open_source_arg = nullptr;
    text_open_section_arg = nullptr;
    text_open_result = static_cast<BOOL>(0x13572468U);

    expect(text_open(source, section) == text_open_result);
    expect(text_open_calls == 1);
    expect(text_open_this == text);
    expect(text_open_source_arg == source);
    expect(text_open_section_arg == section);
    expect_storage_bytes(storage, expected, sizeof(storage));

    text_open_result = FALSE;
    expect(text_open(nullptr, nullptr) == FALSE);
    expect(text_open_calls == 2);
    expect(text_open_this == text);
    expect(text_open_source_arg == nullptr);
    expect(text_open_section_arg == nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Txt = nullptr;
    text->~Text();
}

void expect_heap_clear(uint8_t *expected, size_t heap_offset) {
    expected[heap_offset] = 0;
    LPVOID null_pointer = nullptr;
    const size_t zero = 0;
    write_at(expected, heap_offset + 4, null_pointer);
    write_at(expected, heap_offset + 8, null_pointer);
    write_at(expected, heap_offset + 12, zero);
    write_at(expected, heap_offset + 16, zero);
}

void test_text_index_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Heap) == 0x14, "Heap fixture requires the legacy layout");
    static_assert(sizeof(TextIndex) == 0x118, "TextIndex fixture requires the legacy layout");

    alignas(TextIndex) uint8_t storage[sizeof(TextIndex) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    const uint32_t zero_count = 0;
    write_at(expected, 16 + 0x100, zero_count);
    expected[16] = 0;
    expect_heap_clear(expected, 16 + 0x104);

    auto *index = new (storage + 16) TextIndex;
    int phase_failures = failures;
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != phase_failures) {
        report_storage_mismatch("TextIndex constructor", storage, expected, sizeof(storage));
    }

    seed_storage(storage, expected, sizeof(storage));
    const uint32_t section_count = 3;
    LPVOID base = std::malloc(8);
    LPVOID current = base;
    const size_t base_size = 8;
    const size_t free_size = 4;
    write_at(storage, 16 + 0x100, section_count);
    write_at(storage, 16 + 0x108, base);
    write_at(storage, 16 + 0x10C, current);
    write_at(storage, 16 + 0x110, base_size);
    write_at(storage, 16 + 0x114, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    write_at(expected, 16 + 0x100, zero_count);
    expect_heap_clear(expected, 16 + 0x104);
    heap_shutdown_calls = 0;
    index->~TextIndex();
    expect(heap_shutdown_calls == 1);
    phase_failures = failures;
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != phase_failures) {
        report_storage_mismatch("TextIndex destructor", storage, expected, sizeof(storage));
    }
    if (failures != failures_before) {
        std::fprintf(stderr, "TextIndex lifecycle fixture failed\n");
    }
}

void test_text_clear_index() {
    const int failures_before = failures;
    constexpr size_t entry_size = 0x118;
    const uint32_t zero_count = 0;
    alignas(TextIndex) uint8_t storage[MaxTextIndexNum * entry_size + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        new (storage + 16 + static_cast<size_t>(i) * entry_size) TextIndex;
    }
    const uint32_t counts[MaxTextIndexNum] = {0, 1, UINT32_MAX, 0};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        uint8_t *entry = storage + 16 + static_cast<size_t>(i) * entry_size;
        std::memset(entry, 0x41 + i, 0x100);
        write_at(entry, 0x100, counts[i]);
        entry[0x104] = static_cast<uint8_t>(0x51 + i);
        LPVOID null_pointer = nullptr;
        LPVOID poison_current = reinterpret_cast<LPVOID>(
            0x11110000U + static_cast<uint32_t>(i));
        const size_t base_size = static_cast<size_t>(0x2000 + i);
        const size_t free_size = static_cast<size_t>(0x1000 + i);
        write_at(entry, 0x108, null_pointer);
        write_at(entry, 0x10C, poison_current);
        write_at(entry, 0x110, base_size);
        write_at(entry, 0x114, free_size);
    }
    std::memcpy(expected, storage, sizeof(storage));
    for (int i = 1; i <= 2; ++i) {
        const size_t entry_offset = 16 + static_cast<size_t>(i) * entry_size;
        write_at(expected, entry_offset + 0x100, zero_count);
        expect_heap_clear(expected, entry_offset + 0x104);
    }

    heap_shutdown_calls = 0;
    text_clear_index_source(reinterpret_cast<TextIndex *>(storage + 16));
    expect(heap_shutdown_calls == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "TextIndex clear fixture failed\n");
    }
}

void test_text_index_wrappers() {
    constexpr size_t entry_size = sizeof(TextIndex);
    alignas(TextIndex) uint8_t storage[MaxTextIndexNum * entry_size + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    TxtIndex = reinterpret_cast<TextIndex *>(storage + 16);
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        new (&TxtIndex[i]) TextIndex;
    }

    const uint32_t make_counts[MaxTextIndexNum] = {1, UINT32_MAX, 0, 0};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 make_counts[i]);
    }
    std::memcpy(expected, storage, sizeof(storage));
    char source[] = "fixture";
    text_index_make_calls = 0;
    text_index_make_this = nullptr;
    text_index_make_source = nullptr;
    text_make_index(source);
    expect(text_index_make_calls == 1);
    expect(text_index_make_this == &TxtIndex[2]);
    expect(text_index_make_source == source);
    expect_storage_bytes(storage, expected, sizeof(storage));

    const uint32_t full_counts[MaxTextIndexNum] = {1, 2, 3, 4};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 full_counts[i]);
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_make_calls = 0;
    text_make_index(nullptr);
    expect(text_index_make_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    const uint32_t search_counts[MaxTextIndexNum] = {0, 1, UINT32_MAX, 1};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 search_counts[i]);
        text_index_search_results[i] = -1;
        text_index_search_this[i] = nullptr;
        text_index_search_source[i] = nullptr;
        text_index_search_section[i] = nullptr;
    }
    text_index_search_results[1] = INT_MIN;
    text_index_search_results[2] = 0;
    text_index_search_results[3] = 0x2468;
    std::memcpy(expected, storage, sizeof(storage));
    char section[] = "SECTION";
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == 0);
    expect(text_index_search_calls == 2);
    expect(text_index_search_this[0] == &TxtIndex[1]);
    expect(text_index_search_this[1] == &TxtIndex[2]);
    expect(text_index_search_source[0] == source);
    expect(text_index_search_source[1] == source);
    expect(text_index_search_section[0] == section);
    expect(text_index_search_section[1] == section);
    expect_storage_bytes(storage, expected, sizeof(storage));

    text_index_search_results[1] = 0x2468;
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == 0x2468);
    expect(text_index_search_calls == 1);
    expect(text_index_search_this[0] == &TxtIndex[1]);
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 full_counts[i]);
        text_index_search_results[i] = i == 0 ? INT_MIN : -i;
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == -1);
    expect(text_index_search_calls == MaxTextIndexNum);
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        expect(text_index_search_this[i] == &TxtIndex[i]);
        expect(text_index_search_source[i] == source);
        expect(text_index_search_section[i] == section);
    }
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        const uint32_t zero = 0;
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100, zero);
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_search_calls = 0;
    expect(text_search_index(nullptr, nullptr) == -1);
    expect(text_index_search_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        TxtIndex[i].~TextIndex();
    }
    TxtIndex = nullptr;
}

void test_spot_lifecycle() {
    static_assert(sizeof(Spot) == 0xC, "Spot fixture requires the legacy layout");
    alignas(Spot) uint8_t storage[sizeof(Spot) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    auto *spot = new (storage + 16) Spot;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    spot->clear();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID spots = std::malloc(8);
    const uint32_t max_count = 7;
    const uint32_t add_count = 5;
    write_at(storage, 16, spots);
    write_at(storage, 16 + 4, max_count);
    write_at(storage, 16 + 8, add_count);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    tracked_free_pointer = spots;
    tracked_free_calls = 0;
    spot->~Spot();
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    spot = new (storage + 16) Spot;
    write_at(storage, 16 + 4, max_count);
    write_at(storage, 16 + 8, add_count);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    tracked_free_pointer = reinterpret_cast<void *>(0x87654321U);
    tracked_free_calls = 0;
    spot->~Spot();
    expect_tracked_free_calls(0);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void expect_font_close(uint8_t *expected, size_t offset) {
    const int negative_one = -1;
    const int zero = 0;
    HGDIOBJ null_font = nullptr;
    LPSTR null_file = nullptr;
    write_at(expected, offset, negative_one);
    write_at(expected, offset + 8, null_font);
    write_at(expected, offset + 0xC, zero);
    write_at(expected, offset + 0x10, zero);
    write_at(expected, offset + 0x18, zero);
    write_at(expected, offset + 0x1C, zero);
    write_at(expected, offset + 0x24, null_file);
}

void test_font_lifecycle() {
    static_assert(sizeof(Font) == 0x28, "Font fixture requires the legacy layout");
    alignas(Font) uint8_t storage[sizeof(Font) + 32];
    uint8_t expected[sizeof(storage)];
    const int negative_one = -1;
    const int zero = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, negative_one);
    for (size_t offset : {4U, 8U, 0xCU, 0x10U, 0x18U, 0x1CU, 0x24U}) {
        write_at(expected, 16 + offset, zero);
    }
    auto *font = new (storage + 16) Font;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    char font_name[] = "Fixture Font";
    const int height = -17;
    const int style = static_cast<int>(0x80000005U);
    font_init_calls = 0;
    font_init_this = nullptr;
    font_init_name = nullptr;
    font_init_height = 0;
    font_init_style = 0;
    font = new (storage + 16) Font(font_name, height, style);
    expect(font == reinterpret_cast<Font *>(storage + 16));
    expect(font_init_calls == 1);
    expect(font_init_this == font);
    expect(font_init_name == font_name);
    expect(font_init_height == height);
    expect(font_init_style == static_cast<uint32_t>(style));
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    HGDIOBJ font_object = reinterpret_cast<HGDIOBJ>(1);
    LPSTR file_name = static_cast<LPSTR>(std::malloc(8));
    write_at(storage, 16 + 8, font_object);
    write_at(storage, 16 + 0x24, file_name);
    std::memcpy(expected, storage, sizeof(storage));
    expect_font_close(expected, 16);
    font_close_calls = 0;
    font->~Font();
    expect(font_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    font = new (storage + 16) Font;
    HGDIOBJ null_font = nullptr;
    LPSTR null_file = nullptr;
    write_at(storage, 16 + 8, null_font);
    write_at(storage, 16 + 0x24, null_file);
    std::memcpy(expected, storage, sizeof(storage));
    expect_font_close(expected, 16);
    font_close_calls = 0;
    font->~Font();
    expect(font_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void expect_time_closed(uint8_t *expected, size_t offset) {
    const uint32_t resolution = 5;
    std::memset(expected + offset, 0, sizeof(Time));
    write_at(expected, offset + 0x20, resolution);
}

void test_time_lifecycle_and_modal() {
    static_assert(sizeof(Time) == 0x28, "Time fixture requires the legacy layout");
    alignas(Time) uint8_t storage[sizeof(Time) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expect_time_closed(expected, 16);
    auto *timer = new (storage + 16) Time;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    expect_time_closed(expected, 16);
    time_close_calls = 0;
    timer->~Time();
    expect(time_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    timer = new (storage + 16) Time;
    std::memcpy(expected, storage, sizeof(storage));
    Time::TimeModal = nullptr;
    timer->set_modal();
    expect(Time::TimeModal == timer);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Time::TimeModal = reinterpret_cast<Time *>(1);
    timer->release_modal();
    expect(Time::TimeModal == nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Time::TimeInitCount = 0;
    expect(Time::init_class() == 0);
    expect(Time::TimeInitCount == 1);
    Time::close_class();
    expect(Time::TimeInitCount == 0);

    Time::TimeInitCount = INT_MAX;
    expect(Time::init_class() == 0);
    expect(Time::TimeInitCount == INT_MIN);
    Time::TimeInitCount = INT_MIN;
    Time::close_class();
    expect(Time::TimeInitCount == INT_MAX);
}

void test_filemap_lifecycle() {
    static_assert(sizeof(Filemap) == 0x10, "Filemap fixture requires the legacy layout");
    alignas(Filemap) uint8_t storage[sizeof(Filemap) + 32];
    uint8_t expected[sizeof(storage)];
    LPVOID null_view = nullptr;
    HANDLE invalid_file = INVALID_HANDLE_VALUE;
    HANDLE null_handle = nullptr;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, null_view);
    write_at(expected, 16 + 4, invalid_file);
    write_at(expected, 16 + 8, null_handle);
    auto *filemap = new (storage + 16) Filemap;
    expect_storage_bytes(storage, expected, sizeof(storage));

    HANDLE mapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 4096, nullptr);
    LPVOID view = mapping ? MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 4096) : nullptr;
    HANDLE file = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    expect(mapping != nullptr);
    expect(view != nullptr);
    expect(file != nullptr);
    if (mapping && view && file) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16, view);
        write_at(storage, 16 + 4, file);
        write_at(storage, 16 + 8, mapping);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16, null_view);
        write_at(expected, 16 + 4, null_handle);
        write_at(expected, 16 + 8, null_handle);
        filemap->~Filemap();
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(!UnmapViewOfFile(view));
        expect(!CloseHandle(file));
        expect(!CloseHandle(mapping));
    } else {
        if (view) {
            UnmapViewOfFile(view);
        }
        if (mapping) {
            CloseHandle(mapping);
        }
        if (file) {
            CloseHandle(file);
        }
    }

    seed_storage(storage, expected, sizeof(storage));
    filemap = new (storage + 16) Filemap;
    std::memcpy(expected, storage, sizeof(storage));
    filemap->~Filemap();
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_heap_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Heap) == 0x14, "Heap fixture requires the legacy layout");
    alignas(Heap) uint8_t storage[sizeof(Heap) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    auto *heap = new (storage + 16) Heap;
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap constructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID base = std::malloc(8);
    const size_t base_size = 0x12345678U;
    const size_t free_size = 0x87654321U;
    write_at(storage, 16 + 4, base);
    write_at(storage, 16 + 8, base);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = base;
    tracked_free_calls = 0;
    heap->~Heap();
    expect(heap_shutdown_calls == 0);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap allocated destructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID null_base = nullptr;
    LPVOID poison_current = reinterpret_cast<LPVOID>(0x12345678U);
    write_at(storage, 16 + 4, null_base);
    write_at(storage, 16 + 8, poison_current);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = reinterpret_cast<void *>(0x87654321U);
    tracked_free_calls = 0;
    heap->~Heap();
    expect(heap_shutdown_calls == 0);
    expect_tracked_free_calls(0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap empty destructor", storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "Heap lifecycle fixture failed\n");
    }
}

void test_strings_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Strings) == 0x18, "Strings fixture requires the legacy layout");
    alignas(Strings) uint8_t storage[sizeof(Strings) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Strings) - 4);
    auto *strings = new (storage + 16) Strings;
    expect(strings == reinterpret_cast<Strings *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Strings constructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID base = std::malloc(8);
    const size_t base_size = 0x12345678U;
    const size_t free_size = 0x87654321U;
    const BOOL is_populated = static_cast<BOOL>(0x13572468U);
    write_at(storage, 16 + 4, base);
    write_at(storage, 16 + 8, base);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    write_at(storage, 16 + 0x14, is_populated);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = base;
    tracked_free_calls = 0;
    strings->~Strings();
    expect(heap_shutdown_calls == 1);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Strings destructor", storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "Strings lifecycle fixture failed\n");
    }
}

uint32_t expected_random_seed(uint32_t seed) {
    return seed * 0x19660DU + 0x3C6EF35FU;
}

uint32_t expected_random_int(uint32_t seed, uint32_t min, uint32_t max) {
    if (static_cast<int32_t>(min) > static_cast<int32_t>(max)) {
        const uint32_t temporary = min;
        min = max;
        max = temporary;
    }
    const uint32_t range = max - min;
    return ((range * (expected_random_seed(seed) & 0xFFFFU)) >> 16) + min;
}

double expected_random_double(uint32_t seed) {
    const uint32_t bits = (expected_random_seed(seed) & 0x7FFFFFU) | 0x3F800000U;
    float unit = 0.0f;
    std::memcpy(&unit, &bits, sizeof(unit));
    return static_cast<double>(unit) - 1.0;
}

void test_random_exports() {
    const int failures_before = failures;
    static_assert(sizeof(Random) == 4, "Random fixture requires the legacy layout");
    alignas(Random) uint8_t storage[sizeof(Random) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Random));
    auto *generator = new (storage + 16) Random;
    expect(generator == reinterpret_cast<Random *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Random constructor", storage, expected, sizeof(storage));

    const uint32_t destructor_seed = 0x89ABCDEFU;
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16, destructor_seed);
    std::memcpy(expected, storage, sizeof(storage));
    const uint32_t zero = 0;
    write_at(expected, 16, zero);
    generator->~Random();
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Random destructor", storage, expected, sizeof(storage));

    struct IntegerFixture {
        uint32_t seed;
        uint32_t min;
        uint32_t max;
    };
    const IntegerFixture integer_fixtures[] = {
        {0, 0, 100},
        {0xFFFFFFFFU, 100, 0},
        {0x12345678U, static_cast<uint32_t>(-10), 10},
        {0x87654321U, 10, static_cast<uint32_t>(-10)},
        {0xDEADBEEFU, static_cast<uint32_t>(INT_MIN), static_cast<uint32_t>(INT_MAX)},
        {0xCAFEBABEU, 17, 17},
    };
    for (const IntegerFixture &fixture : integer_fixtures) {
        generator->reseed(fixture.seed);
        expect(generator->get(fixture.min, fixture.max)
               == expected_random_int(fixture.seed, fixture.min, fixture.max));
        expect(generator->get_seed() == expected_random_seed(fixture.seed));
    }

    Rand = generator;
    generator->reseed(0xA5A5A5A5U);
    expect(random_get() == 0xA5A5A5A5U);
    expect(generator->get_seed() == 0xA5A5A5A5U);
    const uint32_t wrapper_min = static_cast<uint32_t>(-200);
    const uint32_t wrapper_max = 300;
    expect(random(wrapper_min, wrapper_max)
           == expected_random_int(0xA5A5A5A5U, wrapper_min, wrapper_max));
    expect(generator->get_seed() == expected_random_seed(0xA5A5A5A5U));

    const uint32_t floating_seeds[] = {
        0, 1, 0x12345678U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t seed : floating_seeds) {
        generator->reseed(seed);
        _clearfp();
        const double actual = generator->get();
        const unsigned int status = _statusfp();
        _clearfp();
        const double expected_value = expected_random_double(seed);
        expect(std::memcmp(&actual, &expected_value, sizeof(actual)) == 0);
        expect(status == 0);
        expect(generator->get_seed() == expected_random_seed(seed));

        generator->reseed(seed);
        _clearfp();
        const double wrapper_actual = random();
        const unsigned int wrapper_status = _statusfp();
        _clearfp();
        expect(std::memcmp(&wrapper_actual, &expected_value, sizeof(wrapper_actual)) == 0);
        expect(wrapper_status == 0);
        expect(generator->get_seed() == expected_random_seed(seed));
    }

    static Random exit_generator;
    Rand = &exit_generator;
    exit_generator.reseed(0x55555555U);
    atexit_calls = 0;
    atexit_callback = nullptr;
    random_rand();
    expect(exit_generator.get_seed() == 0);
    expect(atexit_calls == 1);
    expect(atexit_callback == random_rand_exit);
    random_reseed(0xABCDEF01U);
    expect(exit_generator.get_seed() == 0xABCDEF01U);
    random_rand_exit();
    expect(exit_generator.get_seed() == 0);
    if (failures != failures_before) {
        std::fprintf(stderr, "Random export fixture failed\n");
    }
}

void test_log_lifecycle_and_wrappers() {
    const int failures_before = failures;
    int stage_failures = failures;
    static_assert(sizeof(Log) == 8, "Log fixture requires the legacy layout");
    alignas(Log) uint8_t storage[sizeof(Log) + 32];
    uint8_t expected[sizeof(storage)];
    LPSTR null_file = nullptr;
    const BOOL zero_state = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, null_file);
    write_at(expected, 16 + 4, zero_state);
    auto *log = new (storage + 16) Log;
    expect(log == reinterpret_cast<Log *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log default constructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log default-constructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    const BOOL preserved_state = static_cast<BOOL>(0x13572468U);
    write_at_volatile(storage, 16 + 4, preserved_state);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    env_open_calls = 0;
    log = new (storage + 16) Log(nullptr);
    expect(log == reinterpret_cast<Log *>(storage + 16));
    expect(env_open_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log null constructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log null-constructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    write_at_volatile(storage, 16 + 4, preserved_state);
    char file_name[] = "fixture.log";
    env_open_calls = 0;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    env_open_source = nullptr;
    env_open_mode = nullptr;
    log = new (storage + 16) Log(file_name);
    LPSTR allocated_file = nullptr;
    std::memcpy(&allocated_file, storage + 16, sizeof(allocated_file));
    expect(allocated_file != nullptr);
    if (allocated_file) {
        expect(std::strcmp(allocated_file, file_name) == 0);
    }
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    expect(env_open_source && std::strcmp(env_open_source, file_name) == 0);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    expect(std::memcmp(storage + 16 + 4, &preserved_state, sizeof(preserved_state)) == 0);
    if (failures != stage_failures) {
        BOOL actual_state = 0;
        std::memcpy(&actual_state, storage + 16 + 4, sizeof(actual_state));
        std::fprintf(stderr,
            "Log filename-constructor expectations failed: file=%p calls=%d source=%s mode=%s state=0x%x\n",
            allocated_file, env_open_calls, env_open_source ? env_open_source : "(null)",
            env_open_mode ? env_open_mode : "(null)", static_cast<unsigned int>(actual_state));
    }
    stage_failures = failures;

    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    tracked_free_pointer = allocated_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Log destructor", storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log destructor expectations failed\n");
    }
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    std::memcpy(expected, storage, sizeof(storage));
    mem_get_calls = 0;
    env_open_calls = 0;
    expect(log->init(nullptr) == 16);
    expect(mem_get_calls == 0);
    expect(env_open_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    char *old_file = static_cast<char *>(std::malloc(16));
    std::strcpy(old_file, "old.log");
    write_at(storage, 16, old_file);
    write_at(storage, 16 + 4, preserved_state);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16, null_file);
    mem_get_scripted = true;
    mem_get_calls = 0;
    mem_get_results[0] = nullptr;
    tracked_free_pointer = old_file;
    tracked_free_calls = 0;
    expect(log->init("new.log") == 4);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 8);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    write_at(storage, 16 + 4, preserved_state);
    char *failed_open_file = static_cast<char *>(std::malloc(9));
    mem_get_calls = 0;
    mem_get_results[0] = failed_open_file;
    env_open_calls = 0;
    env_open_fails = true;
    expect(log->init("fail.log") == 6);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 9);
    expect(std::strcmp(failed_open_file, "fail.log") == 0);
    expect(env_open_calls == 1);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    std::memcpy(expected, storage, sizeof(storage));
    tracked_free_pointer = failed_open_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    write_at(expected, 16, null_file);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    write_at(storage, 16 + 4, preserved_state);
    char *successful_file = static_cast<char *>(std::malloc(12));
    mem_get_calls = 0;
    mem_get_results[0] = successful_file;
    env_open_calls = 0;
    env_open_fails = false;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    expect(log->init("success.log") == 0);
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 12);
    expect(std::strcmp(successful_file, "success.log") == 0);
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    std::memcpy(expected, storage, sizeof(storage));
    tracked_free_pointer = successful_file;
    tracked_free_calls = 0;
    log->~Log();
    expect_tracked_free_calls(1);
    write_at(expected, 16, null_file);
    expect_storage_bytes(storage, expected, sizeof(storage));
    mem_get_scripted = false;
    mem_get_results[0] = nullptr;
    stage_failures = failures;

    seed_storage(storage, expected, sizeof(storage));
    log = new (storage + 16) Log;
    LPSTR file_pointer = file_name;
    write_at(storage, 16, file_pointer);
    std::memcpy(expected, storage, sizeof(storage));
    Logging = reinterpret_cast<Log *>(storage + 16);
    env_open_calls = 0;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    env_open_source = nullptr;
    env_open_mode = nullptr;
    log_reset();
    expect(env_open_calls == 1);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    expect(env_open_source == file_name);
    expect(env_open_mode && std::strcmp(env_open_mode, "wt") == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log reset expectations failed\n");
    }
    stage_failures = failures;

    log_set_state(FALSE);
    const BOOL disabled = TRUE;
    write_at(expected, 16 + 4, disabled);
    expect_storage_bytes(storage, expected, sizeof(storage));
    log_set_state(INT_MIN);
    write_at(expected, 16 + 4, zero_state);
    expect_storage_bytes(storage, expected, sizeof(storage));

    static BOOL logging_disabled = FALSE;
    IsLoggingDisabled = &logging_disabled;
    capture_closed_file = true;
    struct LogOutputFixture {
        bool hex;
        LPCSTR second;
        LPCSTR expected_output;
    };
    const LogOutputFixture output_fixtures[] = {
        {false, "second", "first second 1 -2 3\n"},
        {false, nullptr, "first 1 -2 3\n"},
        {true, "second", "first second 0001 abcd ffffffff\n"},
        {true, nullptr, "first 0001 abcd ffffffff\n"},
    };
    const auto invoke_log_output = [](const LogOutputFixture &fixture) {
        if (fixture.hex) {
            if (fixture.second) {
                log_say_hex("first", fixture.second, 1, 0xABCD, -1);
            } else {
                log_say_hex("first", 1, 0xABCD, -1);
            }
        } else if (fixture.second) {
            log_say("first", fixture.second, 1, -2, 3);
        } else {
            log_say("first", 1, -2, 3);
        }
    };
    for (const LogOutputFixture &fixture : output_fixtures) {
        env_open_calls = 0;
#if defined(__MINGW32__)
        env_close_calls = 0;
#endif
        env_open_source = nullptr;
        env_open_mode = nullptr;
        closed_file_output[0] = 0;
        invoke_log_output(fixture);
        expect(env_open_calls == 1);
        expect(env_open_source == file_name);
        expect(env_open_mode && std::strcmp(env_open_mode, "at") == 0);
#if defined(__MINGW32__)
        expect(env_close_calls == 1);
        expect(std::strcmp(closed_file_output, fixture.expected_output) == 0);
#endif
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    env_open_fails = true;
    for (const LogOutputFixture &fixture : output_fixtures) {
        env_open_calls = 0;
#if defined(__MINGW32__)
        env_close_calls = 0;
#endif
        env_open_source = nullptr;
        env_open_mode = nullptr;
        invoke_log_output(fixture);
        expect(env_open_calls == 1);
        expect(env_open_source == file_name);
        expect(env_open_mode && std::strcmp(env_open_mode, "at") == 0);
#if defined(__MINGW32__)
        expect(env_close_calls == 0);
#endif
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    env_open_fails = false;

    const BOOL disabled_state = TRUE;
    for (const LogOutputFixture &fixture : output_fixtures) {
        for (int gate = 0; gate < 3; ++gate) {
            write_at(storage, 16, file_pointer);
            write_at(storage, 16 + 4, zero_state);
            logging_disabled = FALSE;
            if (gate == 0) {
                write_at(storage, 16 + 4, disabled_state);
            } else if (gate == 1) {
                logging_disabled = TRUE;
            } else {
                write_at(storage, 16, null_file);
            }
            uint8_t gate_expected[sizeof(storage)];
            std::memcpy(gate_expected, storage, sizeof(storage));
            env_open_calls = 0;
            invoke_log_output(fixture);
            expect(env_open_calls == 0);
            expect_storage_bytes(storage, gate_expected, sizeof(storage));
        }
    }
    logging_disabled = FALSE;
    write_at(storage, 16, file_pointer);
    write_at(storage, 16 + 4, zero_state);
    capture_closed_file = false;
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log state expectations failed\n");
    }
    stage_failures = failures;
    write_at(storage, 16, null_file);
    log->~Log();

    alignas(Log) static uint8_t exit_storage[sizeof(Log)]{};
    Logging = reinterpret_cast<Log *>(exit_storage);
    write_at_volatile(exit_storage, 4, preserved_state);
    env_open_calls = 0;
    atexit_calls = 0;
    atexit_callback = nullptr;
#if defined(__MINGW32__)
    env_close_calls = 0;
#endif
    log_logging();
    LPSTR global_file = nullptr;
    std::memcpy(&global_file, exit_storage, sizeof(global_file));
    expect(global_file && std::strcmp(global_file, "logfile.txt") == 0);
    expect(std::memcmp(exit_storage + 4, &preserved_state, sizeof(preserved_state)) == 0);
    expect(env_open_calls == 1);
    expect(atexit_calls == 1);
    expect(atexit_callback == log_logging_exit);
#if defined(__MINGW32__)
    expect(env_close_calls == 1);
#endif
    log_logging_exit();
    std::memcpy(&global_file, exit_storage, sizeof(global_file));
    expect(global_file == nullptr);
    expect(std::memcmp(exit_storage + 4, &preserved_state, sizeof(preserved_state)) == 0);
    if (failures != stage_failures) {
        std::fprintf(stderr, "Log global-lifecycle expectations failed\n");
    }
    new (exit_storage) Log;
    if (failures != failures_before) {
        std::fprintf(stderr, "Log lifecycle fixture failed\n");
    }
}

void seed_dialog(uint8_t *storage, uint8_t *expected) {
    seed_storage(storage, expected, sizeof(Dialog) + 32);
}

void test_dialog_id_to_pos() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    seed_dialog(storage, expected);
    DialogEntry *null_entry = nullptr;
    const int stale_position = -7;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xD0, stale_position);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->id_to_pos(123) == stale_position);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    auto *invalid_entry = reinterpret_cast<DialogEntry *>(1);
    const int negative_count = INT_MIN;
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, negative_count);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    const int zero = 0;
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog_id_to_pos_redirect(dialog, nullptr, INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->id_to_pos(INT_MIN) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    seed_dialog(storage, expected);
    DialogEntry *head = &entries[0].entry;
    const int three = 3;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    DialogEntry *second = &entries[1].entry;
    const int one = 1;
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(20) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[2].entry.id = 20;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(20) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[1].entry.id = 21;
    entries[2].entry.id = 30;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    DialogEntry *third = &entries[2].entry;
    const int two = 2;
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(30) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, three);
    expect(dialog_id_to_pos_redirect(dialog, nullptr, -1) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(INT_MIN) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[1].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->id_to_pos(-1) == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[0].entry.id = INT_MAX;
    entries[0].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->id_to_pos(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_set_selected_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;
    DialogEntry *null_entry = nullptr;

    seed_dialog(storage, expected);
    const int stale_position = -17;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xD0, stale_position);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xEC, stale_position);
    dialog->set_selected_id(123);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    auto *invalid_entry = reinterpret_cast<DialogEntry *>(1);
    const int negative_count = INT_MIN;
    const int zero = 0;
    write_at(storage, 16 + 0xC4, invalid_entry);
    write_at(storage, 16 + 0xCC, negative_count);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, zero);
    write_at(expected, 16 + 0xEC, zero);
    dialog_set_selected_id_redirect(dialog, nullptr, INT_MAX);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 20;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    seed_dialog(storage, expected);
    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    const int three = 3;
    const int one = 1;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    write_at(expected, 16 + 0xEC, one);
    dialog->set_selected_id(20);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[0].entry.id = 10;
    entries[1].entry.id = 11;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = invalid_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_dialog(storage, expected);
    const int two = 2;
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, two);
    write_at(expected, 16 + 0xEC, two);
    dialog->set_selected_id(-1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_get_selected_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    entries[0].entry.previous = &entries[2].entry;
    entries[1].entry.previous = &entries[0].entry;
    entries[2].entry.previous = &entries[1].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    DialogEntry *third = &entries[2].entry;
    const int zero = 0;
    const int one = 1;
    const int two = 2;
    const int three = 3;

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, third);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->get_selected_id() == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog_get_selected_id_redirect(dialog, nullptr) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -3);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, three);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->get_selected_id() == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -4);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    DialogEntry *null_entry = nullptr;
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, zero);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, zero);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->get_selected_id() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, zero);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, null_entry);
    expect(dialog->get_selected_id() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, INT_MIN);
    std::memcpy(expected, storage, sizeof(storage));
    const int normalized_min = INT_MIN + 3;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, normalized_min);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, INT_MIN);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, INT_MIN);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->get_selected_id() == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_dialog_pos_to_id() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;
    struct GuardedEntry {
        uint8_t prefix[16];
        DialogEntry entry;
        uint8_t suffix[16];
    } entries[3] = {};
    uint8_t entries_expected[sizeof(entries)];
    entries[0].entry.id = 10;
    entries[1].entry.id = 20;
    entries[2].entry.id = 30;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    entries[0].entry.previous = &entries[2].entry;
    entries[1].entry.previous = &entries[0].entry;
    entries[2].entry.previous = &entries[1].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    DialogEntry *head = &entries[0].entry;
    DialogEntry *second = &entries[1].entry;
    DialogEntry *third = &entries[2].entry;
    const int three = 3;
    const int selected_sentinel = 0x12345678;

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int one = 1;
    write_at(expected, 16 + 0xC8, second);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->pos_to_id(1) == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int zero = 0;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(0) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int two = 2;
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog->pos_to_id(2) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, third);
    write_at(expected, 16 + 0xD0, two);
    expect(dialog_pos_to_id_redirect(dialog, nullptr, -1) == 30);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->pos_to_id(3) == 20);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xC8, second);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    expect(dialog->pos_to_id(-4) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    DialogEntry *null_entry = nullptr;
    write_at(storage, 16 + 0xC4, null_entry);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, null_entry);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(0) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    const int normalized_min = INT_MIN + 3;
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, normalized_min);
    expect(dialog->pos_to_id(INT_MIN) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, INT_MIN);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, selected_sentinel);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, head);
    write_at(expected, 16 + 0xD0, zero);
    expect(dialog->pos_to_id(INT_MIN) == 10);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_string_struct_current_accessors() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->current_id() == 0);
    expect(string_struct_current_entry_redirect(strings, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } guarded;
    uint8_t entry_expected[sizeof(guarded)];
    seed_storage(reinterpret_cast<uint8_t *>(&guarded), entry_expected, sizeof(guarded));
    guarded.entry.id = INT_MIN;
    guarded.entry.payload = INT_MAX;
    std::memcpy(entry_expected, &guarded, sizeof(guarded));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    StringStructEntry *current = &guarded.entry;
    write_at(storage, 16 + 0x0C, current);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(string_struct_current_id_redirect(strings, nullptr) == INT_MIN);
    expect(strings->current_entry() == INT_MAX);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(&guarded), entry_expected, sizeof(guarded));
}

void test_string_struct_next_entry() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.payload = INT_MIN;
    entries[1].entry.payload = 0;
    entries[2].entry.payload = INT_MAX;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));

    StringStructEntry *head = &entries[0].entry;
    StringStructEntry *second = &entries[1].entry;
    StringStructEntry *third = &entries[2].entry;
    const int three = 3;
    const int one = 1;
    const int two = 2;
    const int zero = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, one);
    expect(string_struct_next_entry_redirect(strings, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, third);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, two);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == INT_MIN);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, -1);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, INT_MIN);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, head);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->next_entry() == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

void test_string_struct_seek_id() {
    alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
    uint8_t expected[sizeof(storage)];
    auto *strings = new (storage + 16) StringStruct;
    StringStructEntry *null_entry = nullptr;
    auto *invalid_entry = reinterpret_cast<StringStructEntry *>(1);

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, null_entry);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    expect(strings->seek_id(INT_MIN) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, invalid_entry);
    write_at(storage, 16 + 0x0C, null_entry);
    write_at(storage, 16 + 0x10, INT_MIN);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, invalid_entry);
    const int zero = 0;
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    struct GuardedEntry {
        uint8_t prefix[16];
        StringStructEntry entry;
        uint8_t suffix[16];
    } entries[3];
    uint8_t entries_expected[sizeof(entries)];
    seed_storage(reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
    entries[0].entry.id = INT_MIN;
    entries[1].entry.id = 0;
    entries[2].entry.id = INT_MAX;
    entries[0].entry.next = &entries[1].entry;
    entries[1].entry.next = &entries[2].entry;
    entries[2].entry.next = &entries[0].entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    StringStructEntry *head = &entries[0].entry;
    StringStructEntry *second = &entries[1].entry;
    const int one = 1;
    const int three = 3;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, zero);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MAX) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x0C, invalid_entry);
    write_at(storage, 16 + 0x10, three);
    write_at(storage, 16 + 0x14, INT_MAX);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC, second);
    write_at(expected, 16 + 0x14, one);
    expect(string_struct_seek_id_redirect(strings, nullptr, zero) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(INT_MIN) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    StringStructEntry *third = &entries[2].entry;
    const int two = 2;
    write_at(expected, 16 + 0x0C, third);
    write_at(expected, 16 + 0x14, two);
    expect(strings->seek_id(INT_MAX) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    entries[2].entry.id = zero;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, second);
    write_at(expected, 16 + 0x14, one);
    expect(strings->seek_id(zero) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    entries[2].entry.id = INT_MAX;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, head);
    write_at(expected, 16 + 0x14, three);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));

    entries[2].entry.next = null_entry;
    std::memcpy(entries_expected, entries, sizeof(entries));
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, head);
    write_at(storage, 16 + 0x10, three);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, null_entry);
    write_at(expected, 16 + 0x14, three);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(
        reinterpret_cast<uint8_t *>(entries), entries_expected, sizeof(entries));
}

}  // namespace

int main() {
    test_alpha_net_pid_to_idx();
    test_alpha_net_identity_lookups();
    test_in_box_edges();
    test_win_move();
    test_win_paging();
    test_scroll_init_wrappers();
    test_scroll_range();
    test_scroll_style_setters();
    test_scroll_thumb_resetters();
    test_scroll_border_color();
    test_scroll_position();
    test_expand_rect();
    test_scroll_sprite_setters();
    test_scroll_compute_thumb_rect();
    test_pull_down_item_state();
    test_pull_down_get_selected();
    test_menu_accessors();
    test_text_get_and_item_number();
    test_text_string_helpers();
    test_text_constructors();
    test_text_global_lifecycle();
    test_text_destructor_thunk();
    test_text_open_wrapper();
    test_text_index_lifecycle();
    test_text_clear_index();
    test_text_index_wrappers();
    test_spot_lifecycle();
    test_font_lifecycle();
    test_time_lifecycle_and_modal();
    test_filemap_lifecycle();
    test_heap_lifecycle();
    test_strings_lifecycle();
    test_random_exports();
    test_log_lifecycle_and_wrappers();
    test_button_group_lifecycle();
    test_base_pop_string_font();
    test_dialog_id_to_pos();
    test_dialog_set_selected_id();
    test_dialog_get_selected_id();
    test_dialog_pos_to_id();
    test_string_struct_current_accessors();
    test_string_struct_next_entry();
    test_string_struct_seek_id();
    return failures == 0 ? 0 : 1;
}
