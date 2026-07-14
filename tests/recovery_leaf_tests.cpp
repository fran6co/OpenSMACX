#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/basepop.h"
#include "../src/buttongroup.h"
#include "../src/dialog.h"
#include "../src/font.h"
#include "../src/spot.h"
#include "../src/stringstruct.h"
#include "../src/strings.h"
#include "../src/text_recovery.h"
#include "../src/textindex.h"
#include "../src/time.h"

#include <climits>
#include <cstring>
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

LPVOID __cdecl mem_get(size_t size) {
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
Time *Time::TimeModal = nullptr;
int Time::TimeInitCount = 0;

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
    spot->~Spot();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    spot = new (storage + 16) Spot;
    write_at(storage, 16 + 4, max_count);
    write_at(storage, 16 + 8, add_count);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    spot->~Spot();
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
    test_in_box_edges();
    test_text_get_and_item_number();
    test_text_string_helpers();
    test_text_index_lifecycle();
    test_text_clear_index();
    test_spot_lifecycle();
    test_font_lifecycle();
    test_time_lifecycle_and_modal();
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
