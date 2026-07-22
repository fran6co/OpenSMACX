#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/autosound.h"
#include "../src/basepop.h"
#include "../src/basebutton.h"
#include "../src/buttongroup.h"
#include "../src/console.h"
#include "../src/planwin.h"
#include "../src/dialog.h"
#include "../src/ambience.h"
#include "../src/filemap.h"
#include "../src/flatbutton.h"
#include "../src/buffer.h"
#include "../src/flic.h"
#include "../src/font.h"
#include "../src/graphicwin.h"
#include "../src/font.h"
#include "../src/log.h"
#include "../src/maininterface.h"
#include "../src/mapwin.h"
#include "../src/alphamovie.h"
#include "../src/basewin.h"
#include "../src/datalink.h"
#include "../src/designwin.h"
#include "../src/diplowin.h"
#include "../src/netwin.h"
#include "../src/socialwin.h"
#include "../src/worldwin.h"
#include "../src/checkbutton.h"
#include "../src/cursor.h"
#include "../src/infowin.h"
#include "../src/statuswin.h"
#include "../src/filewin.h"
#include "../src/wave.h"
#include "../src/battlewin.h"
#include "../src/councwin.h"
#include "../src/credits.h"
#include "../src/diplopop.h"
#include "../src/gamma.h"
#include "../src/messagewin.h"
#include "../src/newtechwin.h"
#include "../src/pickwin.h"
#include "../src/prodpicker.h"
#include "../src/reportwin.h"
#include "../src/setupwin.h"
#include "../src/menu.h"
#include "../src/palette.h"
#include "../src/pulldown.h"
#include "../src/random.h"
#include "../src/scroll.h"
#include "../src/sounddevice.h"
#include "../src/spot.h"
#include "../src/stringstruct.h"
#include "../src/strings.h"
#include "../src/text_recovery.h"
#include "../src/textindex.h"
#include "../src/temp.h"
#include "../src/time.h"
#include "../src/vector.h"
#include "../src/win.h"

#include <climits>
#include <cstring>
#include <float.h>
#include <new>
#include <type_traits>
#include <vector>

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

    // The loop covers indices 1..7 (offsets 0..6 * pid_stride); planting a
    // matching ID only at offset 7 * pid_stride distinguishes that bound from
    // an off-by-one that would read one slot further.
    const uint32_t upper_decoy_id = 3000U;
    std::memcpy(bytes + first_pid_offset + 7 * pid_stride, &upper_decoy_id,
                sizeof(upper_decoy_id));
    expect(network.pid_2_idx(upper_decoy_id) == 0);
    expect(alpha_net_pid_to_idx_redirect(&network, nullptr, upper_decoy_id) == 0);

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
    // Slot 0 must not use the "not found" sentinel (0) for either value: a
    // mutant that starts the search loop at slot 1 instead of slot 0 would
    // still report 0 for a slot-0 query - not because it found the entry, but
    // because 0 is also what "not found" returns. Distinct nonzero values at
    // slot 0 make the two cases observably different.
    const uint32_t process_ids[7] = {
        111U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 123U, 456U,
    };
    const int8_t identities[7] = {
        111, 1, 0x7F, static_cast<int8_t>(0x80),
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

void test_button_group_add() {
    // add() never calls anything on `button` beyond writing its group_ field,
    // so the target only needs to be a big-enough, uninitialized region
    // reinterpreted as BaseButton*, not a real constructed object - BaseButton
    // privately inherits the largely-unrecovered GraphicWin, and placement-new
    // would run through that whole chain for no benefit here.
    uint8_t *const button_storage[2] = {
        new uint8_t[sizeof(BaseButton) + 32],
        new uint8_t[sizeof(BaseButton) + 32],
    };
    BaseButton *const buttons[2] = {
        reinterpret_cast<BaseButton *>(button_storage[0] + 16),
        reinterpret_cast<BaseButton *>(button_storage[1] + 16),
    };

    alignas(ButtonGroup) uint8_t storage[sizeof(ButtonGroup) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    expect_lifecycle_clear(expected);
    auto *group = new (storage + 16) ButtonGroup;
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int adapter = 0; adapter < 2; ++adapter) {
        for (int call = 0; call < 2; ++call) {
            BaseButton *const button = buttons[call];
            const int slot = adapter * 2 + call;
            std::memcpy(expected + 16 + slot * 4, &button, sizeof(button));
            const uint32_t new_count = static_cast<uint32_t>(slot + 1);
            std::memcpy(expected + 16 + 0x80, &new_count, sizeof(new_count));
            if (adapter) {
                button_group_add_redirect(group, nullptr, button);
            } else {
                group->add(button);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
            // Verification note: add() also writes `button->group_ = this`,
            // but group_ is private to BaseButton (friended only to
            // ButtonGroup and Scroll) with no public accessor, and
            // BaseButton's layout past its GraphicWin base is not yet pinned
            // by a static_assert. Reading it here would mean either a
            // production-header change purely for test access or a
            // hardcoded, unverified offset - both worse than disclosing the
            // gap. The buttons_[]/count_ effects above are this class's own
            // fields and are fully verified.
        }
    }

    delete[] button_storage[0];
    delete[] button_storage[1];
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
    // An array argument deduces T as the array type, so this would copy every
    // element and leave the first one at the offset where the caller meant to
    // put a pointer. That produced two Wine crashes before it was rejected
    // here; bind an explicit pointer variable instead.
    static_assert(!std::is_array<T>::value,
                  "write_at takes a pointer, not an array");
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

void write_auto_sound_construct_expected(
        uint8_t *expected, size_t base, const uint32_t *defaults) {
    write_at(expected, base + 0x00, AutoSoundVtable);
    write_at(expected, base + 0x04, defaults[0]);
    write_at(expected, base + 0x0C, defaults[1]);
    write_at(expected, base + 0x10, defaults[2]);
    write_at(expected, base + 0x08, defaults[3]);
    for (size_t index = 4; index < 37; ++index) {
        write_at(expected, base + (index + 1) * 4, defaults[index]);
    }
}

void test_auto_sound_construct() {
    uint32_t defaults[37];
    for (size_t index = 0; index < ARRAYSIZE(defaults); ++index) {
        defaults[index] = 0x51000000U + index * 0x10203U;
    }
    uint32_t *const saved_defaults = AutoSoundDefaults;
    AutoSoundDefaults = defaults;
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(AutoSound) uint8_t storage[sizeof(AutoSound) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_auto_sound_construct_expected(expected, 16, defaults);
        auto *const self = reinterpret_cast<AutoSound *>(storage + 16);
        if (use_adapter) {
            expect(auto_sound_construct_redirect(self, nullptr) == self);
        } else {
            self->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    AutoSoundDefaults = saved_defaults;
}

void write_win_construct_expected(
        uint8_t *expected, size_t base, uint32_t self,
        const uint32_t *sound, const uint32_t *fixed,
        const uint32_t *dynamic) {
    write_auto_sound_construct_expected(expected, base, sound);
    write_at(expected, base + 0x0C8, WinSecondaryVtable);
    for (size_t offset = 0x0CC; offset <= 0x0DC; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x000, WinPrimaryVtable);
    write_at(expected, base + 0x0A8, self);
    const size_t first_zero_offsets[] = {
        0x3FC, 0x09C, 0x0A0, 0x0A4, 0x0AC, 0x0B0,
        0x134, 0x138, 0x188, 0x18C, 0x190, 0x194,
        0x198, 0x184, 0x0C4, 0x0F0, 0x19C, 0x12C,
    };
    for (size_t offset : first_zero_offsets) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x130, 1U);
    write_at(expected, base + 0x0FC, dynamic[0]);
    write_at(expected, base + 0x100, fixed[0]);
    write_at(expected, base + 0x114, fixed[1]);
    write_at(expected, base + 0x104, dynamic[2]);
    write_at(expected, base + 0x108, fixed[2]);
    write_at(expected, base + 0x10C, fixed[3]);
    write_at(expected, base + 0x110, fixed[4]);
    write_at(expected, base + 0x118, fixed[5]);
    write_at(expected, base + 0x11C, fixed[6]);
    write_at(expected, base + 0x120, fixed[7]);
    write_at(expected, base + 0x124, dynamic[3]);
    write_at(expected, base + 0x128, fixed[8]);
    write_at(expected, base + 0x0F8, dynamic[1]);
    for (size_t offset = 0x0E0; offset <= 0x0EC; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x43C, 0U);
    write_at(expected, base + 0x440, 0U);
    write_at(expected, base + 0x0F4, 0U);
    for (size_t offset = 0x15C; offset <= 0x168; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    for (size_t offset = 0x13C; offset <= 0x158; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    const size_t late_zero_offsets[] = {
        0x0B4, 0x0C0, 0x0BC, 0x0B8, 0x16C, 0x170, 0x098,
    };
    for (size_t offset : late_zero_offsets) {
        write_at(expected, base + offset, 0U);
    }
    for (size_t offset = 0x400; offset <= 0x438; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    for (size_t offset = 0x174; offset <= 0x180; offset += 4) {
        write_at(expected, base + offset, 1U);
    }
    write_at(expected, base + 0x1A0, 2U);
}

void fill_constructor_defaults(
        uint32_t *sound, uint32_t *win_fixed, uint32_t *win_dynamic) {
    for (size_t index = 0; index < 37; ++index) {
        sound[index] = 0x61000000U + index * 0x10011U;
    }
    for (size_t index = 0; index < 9; ++index) {
        win_fixed[index] = 0x62000000U + index * 0x10101U;
    }
    for (size_t index = 0; index < 4; ++index) {
        win_dynamic[index] = 0x63000000U + index * 0x11011U;
    }
}

void test_win_construct() {
    uint32_t sound[37];
    uint32_t fixed[9];
    uint32_t dynamic[4];
    fill_constructor_defaults(sound, fixed, dynamic);
    uint32_t *const saved_sound = AutoSoundDefaults;
    uint32_t *const saved_fixed = WinStaticDefaults;
    uint32_t *const saved_dynamic = WinDynamicDefaults;
    AutoSoundDefaults = sound;
    WinStaticDefaults = fixed;
    WinDynamicDefaults = dynamic;
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<Win *>(storage + 16);
        write_win_construct_expected(
            expected, 16, reinterpret_cast<uintptr_t>(self),
            sound, fixed, dynamic);
        if (use_adapter) {
            expect(win_construct_redirect(self, nullptr) == self);
        } else {
            self->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    AutoSoundDefaults = saved_sound;
    WinStaticDefaults = saved_fixed;
    WinDynamicDefaults = saved_dynamic;
}

void simulate_palette_get(
        uint8_t *bytes, size_t output, size_t source, int count) {
    for (int index = 0; index < count; ++index) {
        const size_t in = source + static_cast<size_t>(index) * 4;
        const size_t out = output + static_cast<size_t>(index) * 4;
        const uint8_t red = bytes[in + 0];
        bytes[out + 2] = red;
        const uint8_t green = bytes[in + 1];
        bytes[out + 1] = green;
        const uint8_t blue = bytes[in + 2];
        bytes[out + 0] = blue;
        bytes[out + 3] = 0;
    }
}

void test_palette_get_rgbquad() {
    int initialized = 0;
    int *const saved_initialized = PaletteInitialized;
    PaletteInitialized = &initialized;
    alignas(Palette) uint8_t palette_storage[sizeof(Palette) + 32];
    uint8_t palette_expected[sizeof(palette_storage)];
    seed_storage(palette_storage, palette_expected, sizeof(palette_storage));
    auto *const palette = reinterpret_cast<Palette *>(palette_storage + 16);

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        expect((use_adapter
            ? palette_get_rgbquad_redirect(palette, nullptr, nullptr, 0, 1)
            : palette->get_rgbquad(nullptr, 0, 1)) == 3);
        expect_storage_bytes(
            palette_storage, palette_expected, sizeof(palette_storage));

        alignas(RGBQUAD) uint8_t output[sizeof(RGBQUAD) * 6 + 32];
        uint8_t expected[sizeof(output)];
        seed_storage(output, expected, sizeof(output));
        auto *const quads = reinterpret_cast<RGBQUAD *>(output + 16);
        initialized = 0;
        expect((use_adapter
            ? palette_get_rgbquad_redirect(palette, nullptr, quads, 3, 4)
            : palette->get_rgbquad(quads, 3, 4)) == 7);
        expect_storage_bytes(output, expected, sizeof(output));

        initialized = 1;
        for (int count : {0, -1, INT_MIN}) {
            expect((use_adapter
                ? palette_get_rgbquad_redirect(
                    palette, nullptr, quads, 3, count)
                : palette->get_rgbquad(quads, 3, count)) == 0);
            expect_storage_bytes(output, expected, sizeof(output));
        }
        const size_t one_source = 16 + 3 * 4;
        expected[16 + 0] = palette_storage[one_source + 2];
        expected[16 + 1] = palette_storage[one_source + 1];
        expected[16 + 2] = palette_storage[one_source + 0];
        expected[16 + 3] = 0;
        expect((use_adapter
            ? palette_get_rgbquad_redirect(palette, nullptr, quads, 3, 1)
            : palette->get_rgbquad(quads, 3, 1)) == 0);
        expect_storage_bytes(output, expected, sizeof(output));

        seed_storage(output, expected, sizeof(output));
        for (size_t index = 0; index < 4; ++index) {
            const size_t source = 16 + (3 + index) * 4;
            const size_t target = 16 + index * 4;
            expected[target + 0] = palette_storage[source + 2];
            expected[target + 1] = palette_storage[source + 1];
            expected[target + 2] = palette_storage[source + 0];
            expected[target + 3] = 0;
        }
        expect((use_adapter
            ? palette_get_rgbquad_redirect(palette, nullptr, quads, 3, 4)
            : palette->get_rgbquad(quads, 3, 4)) == 0);
        expect_storage_bytes(output, expected, sizeof(output));

        // Exact overlap exposes the legacy read/write order: the red-byte
        // store changes the later blue-byte read from the same source entry.
        seed_storage(palette_storage, palette_expected, sizeof(palette_storage));
        simulate_palette_get(palette_expected, 16, 16, 4);
        auto *const alias = reinterpret_cast<RGBQUAD *>(palette_storage + 16);
        expect((use_adapter
            ? palette_get_rgbquad_redirect(palette, nullptr, alias, 0, 4)
            : palette->get_rgbquad(alias, 0, 4)) == 0);
        expect_storage_bytes(
            palette_storage, palette_expected, sizeof(palette_storage));
    }
    PaletteInitialized = saved_initialized;
}

void write_buffer_construct_expected(
        uint8_t *expected, size_t base, uint32_t reset_520,
        uint32_t default_font, const uint8_t *palette, int palette_state) {
    for (size_t offset = 0x4B0; offset <= 0x4B8; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x000, BufferVtable);
    for (size_t offset = 0x4BC; offset < 0x50C; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    const size_t first_zero_offsets[] = {
        0x584, 0x00C, 0x008, 0x014, 0x010, 0x018, 0x01C,
        0x050, 0x054, 0x058, 0x05C, 0x060, 0x064, 0x068,
        0x06C, 0x074, 0x078, 0x4A4, 0x4A8,
    };
    for (size_t offset : first_zero_offsets) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x50C, 0xFFFFFFFFU);
    for (size_t offset = 0x510; offset <= 0x51C; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x520, reset_520);
    write_at(expected, base + 0x524, 0U);
    write_at(expected, base + 0x52C, default_font);
    write_at(expected, base + 0x530, 0U);
    write_at(expected, base + 0x53C, 0U);
    write_at(expected, base + 0x54C, 0xFFFFFFFFU);
    write_at(expected, base + 0x55C, 2U);
    write_at(expected, base + 0x56C, 2U);
    write_at(expected, base + 0x540, 0xFFFFFFFFU);
    write_at(expected, base + 0x550, 0xFFFFFFFFU);
    write_at(expected, base + 0x560, 2U);
    write_at(expected, base + 0x570, 2U);
    write_at(expected, base + 0x534, 0U);
    write_at(expected, base + 0x544, 0xFFFFFFFFU);
    write_at(expected, base + 0x554, 0xFFFFFFFFU);
    write_at(expected, base + 0x564, 2U);
    write_at(expected, base + 0x574, 2U);
    write_at(expected, base + 0x538, 0U);
    write_at(expected, base + 0x548, 0xFFFFFFFFU);
    write_at(expected, base + 0x558, 0xFFFFFFFFU);
    write_at(expected, base + 0x568, 2U);
    write_at(expected, base + 0x578, 2U);
    write_at(expected, base + 0x57C, 0U);
    expected[base + 0x580] = 0;
    write_at(expected, base + 0x070, 0U);
    write_at(expected, base + 0x004, 0U);
    write_at(expected, base + 0x07C, 0x28U);
    write_at(expected, base + 0x080, 0U);
    write_at(expected, base + 0x084, 0U);
    write_at(expected, base + 0x088, static_cast<uint16_t>(1));
    write_at(expected, base + 0x08A, static_cast<uint16_t>(8));
    for (size_t offset = 0x08C; offset <= 0x098; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0x09C, 0x100U);
    write_at(expected, base + 0x0A0, 0U);
    if (!palette) {
        for (size_t index = 0; index < 256; ++index) {
            expected[base + 0x0A4 + index * 4] =
                static_cast<uint8_t>(index);
            expected[base + 0x0A5 + index * 4] = 0;
            expected[base + 0x0A6 + index * 4] = 0;
            expected[base + 0x0A7 + index * 4] = 0;
        }
    } else if (palette_state) {
        for (size_t index = 0; index < 256; ++index) {
            expected[base + 0x0A4 + index * 4] = palette[index * 4 + 2];
            expected[base + 0x0A5 + index * 4] = palette[index * 4 + 1];
            expected[base + 0x0A6 + index * 4] = palette[index * 4 + 0];
            expected[base + 0x0A7 + index * 4] = 0;
        }
    }
}

void test_buffer_construct() {
    uint32_t reset_520 = 0x71234567U;
    Font *default_font = reinterpret_cast<Font *>(0x76543210U);
    Palette *palette_value = nullptr;
    int palette_state = 0;
    uint32_t *const saved_reset = BufferResetValue520;
    Font **const saved_font = FontDefaultPtr;
    Palette **const saved_palette = BufferPalette;
    int *const saved_initialized = PaletteInitialized;
    BufferResetValue520 = &reset_520;
    FontDefaultPtr = &default_font;
    BufferPalette = &palette_value;
    PaletteInitialized = &palette_state;

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    for (size_t index = 0; index < sizeof(palette_storage); ++index) {
        palette_storage[index] = static_cast<uint8_t>(0x41U + index * 13U);
    }
    auto *const palette = reinterpret_cast<Palette *>(palette_storage);
    struct Case { bool has_palette; int initialized; };
    const Case cases[] = {{false, 0}, {true, 0}, {true, 1}};
    for (const Case &test : cases) {
        palette_value = test.has_palette ? palette : nullptr;
        palette_state = test.initialized;
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_buffer_construct_expected(
                expected, 16, reset_520,
                reinterpret_cast<uintptr_t>(default_font),
                test.has_palette ? palette_storage : nullptr,
                test.initialized);
            auto *const self = reinterpret_cast<Buffer *>(storage + 16);
            if (use_adapter) {
                expect(buffer_construct_redirect(self, nullptr) == self);
            } else {
                self->construct();
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
    BufferResetValue520 = saved_reset;
    FontDefaultPtr = saved_font;
    BufferPalette = saved_palette;
    PaletteInitialized = saved_initialized;
}

void write_graphic_win_construct_expected(
        uint8_t *expected, size_t base, uint32_t self,
        const uint32_t *sound, const uint32_t *win_fixed,
        const uint32_t *win_dynamic, uint32_t reset_520,
        uint32_t default_font, const uint8_t *palette, int palette_state,
        uint32_t graphic_default) {
    write_win_construct_expected(
        expected, base, self, sound, win_fixed, win_dynamic);
    write_buffer_construct_expected(
        expected, base + 0x444, reset_520, default_font,
        palette, palette_state);
    write_at(expected, base + 0x000, GraphicWinPrimaryVtable);
    write_at(expected, base + 0x444, GraphicWinBufferVtable);
    write_at(expected, base + 0xA10, 0U);
    write_at(expected, base + 0x134, 0U);
    write_at(expected, base + 0x138, 0U);
    for (size_t offset = 0x9CC; offset <= 0xA08; offset += 4) {
        write_at(expected, base + offset, 0U);
    }
    write_at(expected, base + 0xA0C, graphic_default);
}

void bind_composite_constructor_defaults(
        uint32_t *sound, uint32_t *win_fixed, uint32_t *win_dynamic,
        uint32_t *reset_520, Font **font, Palette **palette,
        int *palette_state, uint32_t *graphic_default) {
    AutoSoundDefaults = sound;
    WinStaticDefaults = win_fixed;
    WinDynamicDefaults = win_dynamic;
    BufferResetValue520 = reset_520;
    FontDefaultPtr = font;
    BufferPalette = palette;
    PaletteInitialized = palette_state;
    GraphicWinFieldA0CDefault = graphic_default;
}

void test_graphic_win_construct() {
    uint32_t sound[37], win_fixed[9], win_dynamic[4];
    fill_constructor_defaults(sound, win_fixed, win_dynamic);
    uint32_t reset_520 = 0x72112233U;
    Font *font = reinterpret_cast<Font *>(0x73334455U);
    Palette *palette = nullptr;
    int palette_state = 1;
    uint32_t graphic_default = 0x74445566U;
    uint32_t *const saved_sound = AutoSoundDefaults;
    uint32_t *const saved_win_fixed = WinStaticDefaults;
    uint32_t *const saved_win_dynamic = WinDynamicDefaults;
    uint32_t *const saved_reset = BufferResetValue520;
    Font **const saved_font = FontDefaultPtr;
    Palette **const saved_palette = BufferPalette;
    int *const saved_palette_state = PaletteInitialized;
    uint32_t *const saved_graphic = GraphicWinFieldA0CDefault;
    bind_composite_constructor_defaults(
        sound, win_fixed, win_dynamic, &reset_520, &font, &palette,
        &palette_state, &graphic_default);
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<GraphicWin *>(storage + 16);
        write_graphic_win_construct_expected(
            expected, 16,
            reinterpret_cast<uintptr_t>(self),
            sound, win_fixed, win_dynamic, reset_520,
            reinterpret_cast<uintptr_t>(font),
            nullptr, palette_state, graphic_default);
        if (use_adapter) {
            expect(graphic_win_construct_redirect(self, nullptr) == self);
        } else {
            self->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    bind_composite_constructor_defaults(
        saved_sound, saved_win_fixed, saved_win_dynamic, saved_reset,
        saved_font, saved_palette, saved_palette_state, saved_graphic);
}

void write_time_construct_expected(uint8_t *expected, size_t base) {
    for (size_t offset = 0; offset < sizeof(Time); offset += 4) {
        write_at(expected, base + offset, offset == 0x20 ? 5U : 0U);
    }
}

void test_base_button_construct() {
    uint32_t sound[37], win_fixed[9], win_dynamic[4];
    fill_constructor_defaults(sound, win_fixed, win_dynamic);
    uint32_t reset_520 = 0x75123456U;
    Font *font = reinterpret_cast<Font *>(0x76234567U);
    Palette *palette = nullptr;
    int palette_state = 1;
    uint32_t graphic_default = 0x77345678U;
    uint32_t base_fixed[5] = {
        0x78111111U, 0x78222222U, 0x78333333U, 0x78444444U,
        0x78555555U,
    };
    uint32_t base_dynamic[2] = {0x79666666U, 0x79777777U};
    uint32_t *const saved_sound = AutoSoundDefaults;
    uint32_t *const saved_win_fixed = WinStaticDefaults;
    uint32_t *const saved_win_dynamic = WinDynamicDefaults;
    uint32_t *const saved_reset = BufferResetValue520;
    Font **const saved_font = FontDefaultPtr;
    Palette **const saved_palette = BufferPalette;
    int *const saved_palette_state = PaletteInitialized;
    uint32_t *const saved_graphic = GraphicWinFieldA0CDefault;
    uint32_t *const saved_base_fixed = BaseButtonStaticDefaults;
    uint32_t *const saved_base_dynamic = BaseButtonDynamicDefaults;
    bind_composite_constructor_defaults(
        sound, win_fixed, win_dynamic, &reset_520, &font, &palette,
        &palette_state, &graphic_default);
    BaseButtonStaticDefaults = base_fixed;
    BaseButtonDynamicDefaults = base_dynamic;
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(BaseButton) uint8_t storage[sizeof(BaseButton) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<BaseButton *>(storage + 16);
        write_graphic_win_construct_expected(
            expected, 16,
            reinterpret_cast<uintptr_t>(self),
            sound, win_fixed, win_dynamic, reset_520,
            reinterpret_cast<uintptr_t>(font),
            nullptr, palette_state, graphic_default);
        write_time_construct_expected(expected, 16 + 0xA1C);
        write_time_construct_expected(expected, 16 + 0xA4C);
        write_at(expected, 16 + 0x000, BaseButtonPrimaryVtable);
        write_at(expected, 16 + 0x444, BaseButtonBufferVtable);
        write_at(expected, 16 + 0xA74, 0U);
        write_at(expected, 16 + 0xA44, 0xFFFFFFFFU);
        write_at(expected, 16 + 0xA48, 0xFFFFFFFFU);
        write_at(expected, 16 + 0xA78, 0U);
        write_at(expected, 16 + 0xA9C, 0U);
        write_at(expected, 16 + 0xA7C, 0U);
        write_at(expected, 16 + 0xA80, 0U);
        write_at(expected, 16 + 0xAA8, 0U);
        write_at(expected, 16 + 0xAAC, 0U);
        write_at(expected, 16 + 0xAB0, 0U);
        write_at(expected, 16 + 0xAB4, 0U);
        write_at(expected, 16 + 0xA94, base_dynamic[0]);
        write_at(expected, 16 + 0xA84, base_fixed[0]);
        write_at(expected, 16 + 0xA88, base_fixed[1]);
        write_at(expected, 16 + 0xA8C, base_fixed[2]);
        write_at(expected, 16 + 0xA90, base_fixed[3]);
        write_at(expected, 16 + 0xA98, base_dynamic[1]);
        write_at(expected, 16 + 0xAA4, 0U);
        write_at(expected, 16 + 0xAA0, base_fixed[4]);
        if (use_adapter) {
            expect(base_button_construct_redirect(self, nullptr) == self);
        } else {
            self->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    BaseButtonStaticDefaults = saved_base_fixed;
    BaseButtonDynamicDefaults = saved_base_dynamic;
    bind_composite_constructor_defaults(
        saved_sound, saved_win_fixed, saved_win_dynamic, saved_reset,
        saved_font, saved_palette, saved_palette_state, saved_graphic);
}

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "tests require 32-bit int");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

float float_from_bits(uint32_t bits) {
    float value;
    static_assert(sizeof(value) == sizeof(bits), "tests require binary32 float");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t midpoint_bits(uint32_t near_edge, uint32_t far_edge) {
    const uint32_t delta = far_edge - near_edge;
    const uint32_t adjusted = delta + (delta >> 31);
    const uint32_t half = (adjusted >> 1) | (adjusted & 0x80000000U);
    return near_edge + half;
}

uint32_t reference_rect_center(
        uint8_t *storage, size_t rect_offset, size_t x_offset, size_t y_offset) {
    uint32_t left;
    uint32_t right;
    std::memcpy(&left, storage + rect_offset, sizeof(left));
    std::memcpy(&right, storage + rect_offset + 8, sizeof(right));
    const uint32_t x = midpoint_bits(left, right);
    std::memcpy(storage + x_offset, &x, sizeof(x));

    uint32_t top;
    uint32_t bottom;
    std::memcpy(&top, storage + rect_offset + 4, sizeof(top));
    std::memcpy(&bottom, storage + rect_offset + 12, sizeof(bottom));
    const uint32_t y = midpoint_bits(top, bottom);
    std::memcpy(storage + y_offset, &y, sizeof(y));
    return y;
}

void test_geometry_helpers() {
    struct RectCase {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };
    const RectCase rect_cases[] = {
        {0, 0, 0, 0},
        {10, 20, 30, 40},
        {0xFFFFFFFFU, 0x80000000U, 2, 0x80000000U},
        {0x7FFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU},
    };
    for (const RectCase &test : rect_cases) {
        alignas(RECT) uint8_t storage[sizeof(RECT) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_at(expected, 16, test.x);
        write_at(expected, 20, test.y);
        write_at(expected, 24, test.x + test.width);
        write_at(expected, 28, test.y + test.height);
        auto *rect = reinterpret_cast<RECT *>(storage + 16);
        expect(make_rect(rect, int_from_bits(test.x), int_from_bits(test.y),
                         int_from_bits(test.width), int_from_bits(test.height)) == rect);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    struct BoxCase {
        uint32_t x;
        uint32_t y;
        uint32_t left;
        uint32_t top;
        uint32_t width;
        uint32_t height;
        int expected;
    };
    const BoxCase box_cases[] = {
        {10, 20, 10, 20, 30, 40, 1},
        {39, 59, 10, 20, 30, 40, 1},
        {40, 59, 10, 20, 30, 40, 0},
        {39, 60, 10, 20, 30, 40, 0},
        {9, 20, 10, 20, 30, 40, 0},
        {10, 19, 10, 20, 30, 40, 0},
        {0x80000000U, 0, 0x7FFFFFFFU, 0, 1, 1, 0},
        {0x7FFFFFFFU, 0, 0x80000000U, 0, 0xFFFFFFFFU, 1, 0},
        {0, 0, 0, 0, 0, 0, 0},
    };
    for (const BoxCase &test : box_cases) {
        expect(in_box(int_from_bits(test.x), int_from_bits(test.y),
                      int_from_bits(test.left), int_from_bits(test.top),
                      int_from_bits(test.width), int_from_bits(test.height))
               == test.expected);
    }

    struct CenterCase {
        size_t x_offset;
        size_t y_offset;
        bool use_adapter;
    };
    const CenterCase center_cases[] = {
        {40, 44, false},
        {40, 40, true},
        {20, 44, false},
        {28, 44, true},
    };
    const uint32_t rectangles[][4] = {
        {10, 20, 30, 40},
        {10, 20, 5, 13},
        {0x80000000U, 0x7FFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };
    for (const auto &rectangle : rectangles) {
        for (const CenterCase &test : center_cases) {
            alignas(RECT) uint8_t storage[64];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(storage + 16, rectangle, sizeof(rectangle));
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t expected_result = reference_rect_center(
                expected, 16, test.x_offset, test.y_offset);
            auto *rect = reinterpret_cast<RECT *>(storage + 16);
            auto *x = reinterpret_cast<int *>(storage + test.x_offset);
            auto *y = reinterpret_cast<int *>(storage + test.y_offset);
            const int result = test.use_adapter
                ? tutwin_rect_center_redirect(
                    reinterpret_cast<void *>(0x13579BDFU), nullptr, rect, x, y)
                : rect_center(rect, x, y);
            expect(static_cast<uint32_t>(result) == expected_result);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void set_vector_bits(uint8_t *object, const uint32_t (&values)[3]) {
    std::memcpy(object, values, sizeof(values));
}

void test_vector_lifecycle() {
    alignas(Vector) uint8_t storage[sizeof(Vector) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Vector));
    auto *vector = new (storage + 16) Vector;
    expect(vector == reinterpret_cast<Vector *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Vector));
    vector->close();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Vector));
    expect(vector_construct_redirect(vector, nullptr) == vector);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Vector));
    expect(vector_close_redirect(vector, nullptr) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

enum class VectorOperation {
    Subtract,
    AddAssign,
    SubtractAssign,
    Scale,
};

void call_vector_operation(VectorOperation operation, bool use_adapter,
                           Vector *left, Vector *output, Vector *right) {
    switch (operation) {
      case VectorOperation::Subtract:
        if (use_adapter) {
            expect(vector_subtract_redirect(left, nullptr, output, right) == output);
        } else {
            left->__mi(*output, *right);
        }
        return;
      case VectorOperation::AddAssign:
        if (use_adapter) {
            expect(vector_add_assign_redirect(left, nullptr, right) == left);
        } else {
            left->__apl(*right);
        }
        return;
      case VectorOperation::SubtractAssign:
        if (use_adapter) {
            expect(vector_subtract_assign_redirect(left, nullptr, right) == left);
        } else {
            left->__ami(*right);
        }
        return;
      case VectorOperation::Scale:
        if (use_adapter) {
            expect(vector_scale_redirect(left, nullptr, output, 0x40000000U) == output);
        } else {
            left->scale(*output, float_from_bits(0x40000000U));
        }
        return;
    }
}

void test_vector_arithmetic() {
    const uint32_t left_values[3] = {0x3FC00000U, 0xC0000000U, 0x40800000U};
    const uint32_t right_values[3] = {0x3F000000U, 0x40400000U, 0xC1000000U};
    const uint32_t expected_values[][3] = {
        {0x3F800000U, 0xC0A00000U, 0x41400000U},
        {0x40000000U, 0x3F800000U, 0xC0800000U},
        {0x3F800000U, 0xC0A00000U, 0x41400000U},
        {0x40400000U, 0xC0800000U, 0x41000000U},
    };
    const VectorOperation operations[] = {
        VectorOperation::Subtract,
        VectorOperation::AddAssign,
        VectorOperation::SubtractAssign,
        VectorOperation::Scale,
    };
    for (size_t operation = 0; operation < 4; ++operation) {
        for (bool use_adapter : {false, true}) {
            alignas(Vector) uint8_t left_storage[sizeof(Vector) + 32];
            alignas(Vector) uint8_t right_storage[sizeof(Vector) + 32];
            alignas(Vector) uint8_t output_storage[sizeof(Vector) + 32];
            uint8_t expected_left[sizeof(left_storage)];
            uint8_t expected_right[sizeof(right_storage)];
            uint8_t expected_output[sizeof(output_storage)];
            seed_storage(left_storage, expected_left, sizeof(left_storage));
            seed_storage(right_storage, expected_right, sizeof(right_storage));
            seed_storage(output_storage, expected_output, sizeof(output_storage));
            set_vector_bits(left_storage + 16, left_values);
            set_vector_bits(right_storage + 16, right_values);
            std::memcpy(expected_left, left_storage, sizeof(left_storage));
            std::memcpy(expected_right, right_storage, sizeof(right_storage));
            std::memcpy(expected_output, output_storage, sizeof(output_storage));
            if (operations[operation] == VectorOperation::Subtract) {
                set_vector_bits(expected_output + 16, expected_values[operation]);
            } else if (operations[operation] == VectorOperation::Scale) {
                set_vector_bits(expected_left + 16, expected_values[operation]);
                set_vector_bits(expected_output + 16, expected_values[operation]);
            } else {
                set_vector_bits(expected_left + 16, expected_values[operation]);
            }
            call_vector_operation(
                operations[operation], use_adapter,
                reinterpret_cast<Vector *>(left_storage + 16),
                reinterpret_cast<Vector *>(output_storage + 16),
                reinterpret_cast<Vector *>(right_storage + 16));
            expect_storage_bytes(left_storage, expected_left, sizeof(left_storage));
            expect_storage_bytes(right_storage, expected_right, sizeof(right_storage));
            expect_storage_bytes(output_storage, expected_output, sizeof(output_storage));
        }
    }

    alignas(Vector) uint32_t overlap[6] = {
        0x3F800000U, 0x40000000U, 0x40400000U,
        0x40800000U, 0x40A00000U, 0xA5A5A5A5U,
    };
    auto *overlap_left = reinterpret_cast<Vector *>(&overlap[1]);
    auto *overlap_right = reinterpret_cast<Vector *>(&overlap[0]);
    overlap_left->__apl(*overlap_right);
    expect(overlap[0] == 0x3F800000U);
    expect(overlap[1] == 0x40400000U);
    expect(overlap[2] == 0x40C00000U);
    expect(overlap[3] == 0x41200000U);
    expect(overlap[4] == 0x40A00000U);
    expect(overlap[5] == 0xA5A5A5A5U);

    uint32_t scale_overlap[5] = {
        0x3F800000U, 0x40000000U, 0x40400000U, 0xA5A5A5A5U, 0xA5A5A5A5U,
    };
    auto *scale_left = reinterpret_cast<Vector *>(&scale_overlap[0]);
    auto *scale_output = reinterpret_cast<Vector *>(&scale_overlap[1]);
    expect(vector_scale_redirect(
               scale_left, nullptr, scale_output, 0x40000000U) == scale_output);
    const uint32_t scale_expected[5] = {
        0x40000000U, 0x40000000U, 0x40000000U, 0x40000000U, 0xA5A5A5A5U,
    };
    expect_storage_bytes(reinterpret_cast<uint8_t *>(scale_overlap),
                         reinterpret_cast<const uint8_t *>(scale_expected),
                         sizeof(scale_overlap));
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

struct ScrollCloseProbeState {
    uint8_t *base;
    const uint32_t *static_defaults;
    const uint32_t *dynamic_defaults;
    int order;
};

ScrollCloseProbeState scroll_close_probe_state = {};

uint32_t scroll_close_read(size_t offset) {
    uint32_t value;
    std::memcpy(&value, scroll_close_probe_state.base + offset, sizeof(value));
    return value;
}

void expect_scroll_close_defaults() {
    const uint32_t *const fixed = scroll_close_probe_state.static_defaults;
    const uint32_t *const dynamic = scroll_close_probe_state.dynamic_defaults;
    expect(scroll_close_read(0xA14) == dynamic[0]);
    expect(scroll_close_read(0xA1C) == fixed[3]);
    expect(scroll_close_read(0xA20) == dynamic[1]);
    expect(scroll_close_read(0xA24) == fixed[4]);
    expect(scroll_close_read(0xA28) == 0U);
    expect(scroll_close_read(0xA2C) == dynamic[1]);
    expect(scroll_close_read(0xA30) == fixed[2]);
    expect(scroll_close_read(0xA34) == fixed[1]);
    expect(scroll_close_read(0xA38) == 0U);
    expect(scroll_close_read(0xA3C) == 0xFFFFFFFFU);
    expect(scroll_close_read(0xA40) == fixed[0]);
    expect(scroll_close_read(0xA44) == 0U);
    expect(scroll_close_read(0xA48) == fixed[5]);
    for (size_t offset = 0xA4C; offset <= 0xA58; offset += 4) {
        expect(scroll_close_read(offset) == 0U);
    }
    expect(scroll_close_read(0xA5C) == fixed[6]);
    expect(scroll_close_read(0xA64) == fixed[7]);
    expect(scroll_close_read(0xA68) == fixed[8]);
    expect(scroll_close_read(0xA6C) == fixed[9]);
    expect(scroll_close_read(0xA70) == fixed[10]);
    for (size_t index = 0; index < 3; ++index) {
        expect(scroll_close_read(0xA7C + index * 4) == dynamic[2 + index]);
        expect(scroll_close_read(0xA88 + index * 4) == dynamic[5 + index]);
        expect(scroll_close_read(0xA94 + index * 4) == dynamic[8 + index]);
        expect(scroll_close_read(0xAA0 + index * 4) == dynamic[11 + index]);
    }
    expect(scroll_close_read(0xA74) == dynamic[15]);
    expect(scroll_close_read(0xA78) == dynamic[16]);
    expect(scroll_close_read(0x2144) == 0U);
    expect(scroll_close_read(0x2148) == 0U);
}

uint32_t __fastcall scroll_close_left_probe(void *self, void *) {
    expect(scroll_close_probe_state.order == 0);
    expect(self == scroll_close_probe_state.base + 0xAAC);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 1;
    return 0x11112222U;
}

uint32_t __fastcall scroll_close_right_probe(void *self, void *) {
    expect(scroll_close_probe_state.order == 1);
    expect(self == scroll_close_probe_state.base + 0x15F8);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 2;
    return 0x33334444U;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall scroll_close_win_probe(void *self) {
    expect(scroll_close_probe_state.order == 2);
    expect(self == scroll_close_probe_state.base);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 3;
}

void __thiscall scroll_close_buffer_probe(void *self) {
    expect(scroll_close_probe_state.order == 3);
    expect(self == scroll_close_probe_state.base + 0x444);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 4;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void write_scroll_close_expected(uint8_t *expected,
                                 const uint32_t fixed[11],
                                 const uint32_t dynamic[17],
                                 uint32_t base_result) {
    write_at(expected, 16 + 0xA14, dynamic[0]);
    write_at(expected, 16 + 0xA1C, fixed[3]);
    write_at(expected, 16 + 0xA20, dynamic[1]);
    write_at(expected, 16 + 0xA24, fixed[4]);
    write_at(expected, 16 + 0xA28, 0U);
    write_at(expected, 16 + 0xA2C, dynamic[1]);
    write_at(expected, 16 + 0xA30, fixed[2]);
    write_at(expected, 16 + 0xA34, fixed[1]);
    write_at(expected, 16 + 0xA38, 0U);
    write_at(expected, 16 + 0xA3C, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA40, fixed[0]);
    write_at(expected, 16 + 0xA44, 0U);
    write_at(expected, 16 + 0xA48, fixed[5]);
    for (size_t offset = 0xA4C; offset <= 0xA58; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA5C, fixed[6]);
    write_at(expected, 16 + 0xA64, fixed[7]);
    write_at(expected, 16 + 0xA68, fixed[8]);
    write_at(expected, 16 + 0xA6C, fixed[9]);
    write_at(expected, 16 + 0xA70, fixed[10]);
    for (size_t index = 0; index < 3; ++index) {
        write_at(expected, 16 + 0xA7C + index * 4, dynamic[2 + index]);
        write_at(expected, 16 + 0xA88 + index * 4, dynamic[5 + index]);
        write_at(expected, 16 + 0xA94 + index * 4, dynamic[8 + index]);
        write_at(expected, 16 + 0xAA0 + index * 4, dynamic[11 + index]);
    }
    write_at(expected, 16 + 0xA74, dynamic[15]);
    write_at(expected, 16 + 0xA78, dynamic[16]);
    write_at(expected, 16 + 0x2144, 0U);
    write_at(expected, 16 + 0x2148, 0U);

    write_at(expected, 16 + 0x134, 0U);
    write_at(expected, 16 + 0x138, 0U);
    for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA0C, base_result);
    write_at(expected, 16 + 0xA10, 0U);
}

void test_scroll_close() {
    uint32_t fixed[11];
    uint32_t dynamic[17];
    for (size_t index = 0; index < ARRAYSIZE(fixed); ++index) {
        fixed[index] = 0x51000000U + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(dynamic); ++index) {
        dynamic[index] = 0xA1000000U + static_cast<uint32_t>(index) * 0x010101U;
    }
    dynamic[14] = 0xDEADC0DEU;
    uint32_t base_result = 0x7B3D19E5U;

    uint32_t *const saved_fixed = ScrollCloseStaticDefaults;
    uint32_t *const saved_dynamic = ScrollCloseDynamicDefaults;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_base_default = GraphicWinFieldA0CDefault;
    ScrollCloseStaticDefaults = fixed;
    ScrollCloseDynamicDefaults = dynamic;
    WinOriginalClose = scroll_close_win_probe;
    BufferSubobjectClose = scroll_close_buffer_probe;
    GraphicWinFieldA0CDefault = &base_result;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t expected[sizeof(storage)];
        uintptr_t left_vtable[0x16C / sizeof(uintptr_t)] = {};
        uintptr_t right_vtable[0x16C / sizeof(uintptr_t)] = {};
        left_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_left_probe);
        right_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_right_probe);
        seed_storage(storage, expected, sizeof(storage));
        uintptr_t *left_vtable_pointer = left_vtable;
        uintptr_t *right_vtable_pointer = right_vtable;
        write_at(storage, 16 + 0xAAC, left_vtable_pointer);
        write_at(storage, 16 + 0x15F8, right_vtable_pointer);
        write_at(storage, 16 + 0xA08, 0U);
        write_at(storage, 16 + 0xA10, 0x13579BDFU);
        std::memcpy(expected, storage, sizeof(storage));
        write_scroll_close_expected(expected, fixed, dynamic, base_result);

        auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
        scroll_close_probe_state = {
            storage + 16, fixed, dynamic, 0,
        };
        const uint32_t result = use_adapter
            ? scroll_close_redirect(scroll, nullptr)
            : scroll->close();
        expect(result == base_result);
        expect(scroll_close_probe_state.order == 4);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    ScrollCloseStaticDefaults = saved_fixed;
    ScrollCloseDynamicDefaults = saved_dynamic;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_base_default;
}

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

    // The loop bound is 15 real entries; index 15 sits just past entries_[14]
    // in the trailing canary region, which sizeof(Menu) + 32 keeps safely
    // addressable. Planting the requested ID only there distinguishes the
    // correct `< 15` bound from an off-by-one `<= 15`: the former must return
    // -1 having never read that slot, while the latter would wrongly report
    // it found.
    {
        alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        for (int index = 0; index < 15; ++index) {
            const int id = 1000 + index;
            write_at(storage, 16 + 0xA38 + index * 0x14, id);
        }
        const int requested = 55;
        write_at(storage, 16 + 0xA38 + 15 * 0x14, requested);
        std::memcpy(expected, storage, sizeof(storage));
        auto *menu = reinterpret_cast<Menu *>(storage + 16);
        expect(menu->id_to_index(requested) == -1);
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

    // A single-entry MISS is the only shape that observes the `count > 0`
    // guard: the match-at-position-zero case above leaves entry_position_ and
    // current_entry_ exactly as the pre-loop stores set them, so skipping the
    // loop entirely (guard mutated to `count > 1`) produces identical state.
    // A miss walks once - position 1, cursor advanced to the entry's next -
    // which the skipped loop cannot reproduce.
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0xC8, invalid_entry);
    write_at(expected, 16 + 0xD0, one);
    expect(dialog->id_to_pos(77) == 1);
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

    // position == 0 is the only value that distinguishes `position < 0` from
    // `position <= 0` (or `< 1`): both branches leave the cursor at the head
    // and return its ID, but the negative branch normalizes the position by
    // adding the count, so entry_position_ ends at 3 instead of 0. Only the
    // full-object byte compare sees the difference.
    seed_dialog(storage, expected);
    write_at(storage, 16 + 0xC4, head);
    write_at(storage, 16 + 0xCC, three);
    write_at(storage, 16 + 0xD0, -7);
    write_at(storage, 16 + 0xEC, zero);
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

void test_dialog_font_and_color_setters() {
    alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
    uint8_t expected[sizeof(storage)];
    auto *dialog = new (storage + 16) Dialog;

    // The twelve color slots interleave by tier - primary at 0x7C/0x88/0x94/
    // 0xA0, secondary at 0x80/0x8C/0x98/0xA4, tertiary at 0x84/0x90/0x9C/
    // 0xA8 - so each setter must stride by 0xC, not fill a contiguous block.
    // Distinct sentinels per argument catch both dropped and misrouted
    // stores; the full-object compare catches writes to a sibling tier.
    struct ColorCase {
        void (Dialog::*member)(int, int, int, int);
        void (__fastcall *redirect)(Dialog *, void *, int, int, int, int);
        size_t offsets[4];
    };
    const ColorCase color_cases[] = {
        {&Dialog::set_dialog_text_color, dialog_set_text_color_redirect,
         {0x7C, 0x88, 0x94, 0xA0}},
        {&Dialog::set_dialog_text_color2, dialog_set_text_color2_redirect,
         {0x80, 0x8C, 0x98, 0xA4}},
        {&Dialog::set_dialog_text_color3, dialog_set_text_color3_redirect,
         {0x84, 0x90, 0x9C, 0xA8}},
    };
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    for (const ColorCase &test : color_cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_dialog(storage, expected);
            std::memcpy(expected, storage, sizeof(storage));
            for (int slot = 0; slot < 4; ++slot) {
                write_at(expected, 16 + test.offsets[slot], colors[slot]);
            }
            if (adapter) {
                test.redirect(dialog, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (dialog->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // set_dialog_font reads only Font::is_initialized(), an inline check of
    // font_obj_ against null. Font's data is private with no offset pins, so
    // the fixtures are raw storage with font_obj_ written at 0x08 per the
    // declared layout (int unk_1_, BOOL is_fot_set_, HFONT font_obj_) - no
    // constructed Font is required or wanted here.
    alignas(Font) uint8_t initialized_font[sizeof(Font)];
    alignas(Font) uint8_t uninitialized_font[sizeof(Font)];
    std::memset(initialized_font, 0xA5, sizeof(initialized_font));
    std::memset(uninitialized_font, 0xA5, sizeof(uninitialized_font));
    const HFONT font_object = reinterpret_cast<HFONT>(0x1234U);
    const HFONT null_font_object = nullptr;
    write_at(initialized_font, 0x08, font_object);
    write_at(uninitialized_font, 0x08, null_font_object);
    auto *primary = reinterpret_cast<Font *>(initialized_font);
    auto *unready = reinterpret_cast<Font *>(uninitialized_font);
    auto *secondary = reinterpret_cast<Font *>(0x11111111U);
    auto *tertiary = reinterpret_cast<Font *>(0x22222222U);

    // Null primary: error 3, nothing written at all.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    expect(dialog->set_dialog_font(nullptr, secondary, tertiary) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Initialized primary: all three stored, success.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x70, primary);
    write_at(expected, 16 + 0x74, secondary);
    write_at(expected, 16 + 0x78, tertiary);
    expect(dialog->set_dialog_font(primary, secondary, tertiary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Uninitialized primary: font1_ is left alone, but the secondary and
    // tertiary slots are still stored and the call still succeeds.
    seed_dialog(storage, expected);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x74, secondary);
    write_at(expected, 16 + 0x78, tertiary);
    expect(dialog_set_font_redirect(
               dialog, nullptr, unready, secondary, tertiary) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
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

    // A single-entry list distinguishes `entry_count_ > 0` from `> 1`: with
    // three entries above, both guards evaluate true and the loop runs either
    // way, so a mutated bound survived undetected. Only count == 1 makes the
    // guard itself observable.
    struct GuardedEntry single_entry_storage;
    uint8_t single_entry_expected[sizeof(single_entry_storage)];
    seed_storage(reinterpret_cast<uint8_t *>(&single_entry_storage),
                 single_entry_expected, sizeof(single_entry_storage));
    single_entry_storage.entry.id = 42;
    single_entry_storage.entry.next = &single_entry_storage.entry;
    std::memcpy(single_entry_expected, &single_entry_storage,
                sizeof(single_entry_storage));
    StringStructEntry *single_head = &single_entry_storage.entry;

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, single_head);
    write_at(storage, 16 + 0x10, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, single_head);
    write_at(expected, 16 + 0x14, zero);
    expect(strings->seek_id(42) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(reinterpret_cast<uint8_t *>(&single_entry_storage),
                         single_entry_expected, sizeof(single_entry_storage));

    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x08, single_head);
    write_at(storage, 16 + 0x10, one);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x0C, single_head);
    write_at(expected, 16 + 0x14, one);
    expect(strings->seek_id(17) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect_storage_bytes(reinterpret_cast<uint8_t *>(&single_entry_storage),
                         single_entry_expected, sizeof(single_entry_storage));
}

}  // namespace

void test_win_is_visible() {
    // Visibility is the conjunction of this window's flag bit 0 and the
    // visibility of every ancestor, so chains exercise the recursion.
    struct WinNode {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(Win) + 32];

        Win *object() { return reinterpret_cast<Win *>(storage + 16); }
    };
    struct ChainCase {
        int depth;
        uint32_t flags[3];   // flag dword written at offset 0x9C per node
        int expected;
    };
    const ChainCase cases[] = {
        {1, {0x00000000U, 0, 0}, 0},
        {1, {0x00000001U, 0, 0}, 1},
        {1, {0xFFFFFFFEU, 0, 0}, 0},
        {1, {0xFFFFFFFFU, 0, 0}, 1},
        {1, {0xA55AA55BU, 0, 0}, 1},
        {2, {0x00000001U, 0x00000001U, 0}, 1},
        {2, {0x00000001U, 0x00000000U, 0}, 0},
        {2, {0x00000000U, 0x00000001U, 0}, 0},
        {3, {0x00000001U, 0x00000001U, 0x00000001U}, 1},
        {3, {0x00000001U, 0x00000001U, 0x00000000U}, 0},
        {3, {0x00000001U, 0x00000000U, 0x00000001U}, 0},
        {3, {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU}, 0},
    };
    for (const ChainCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[3];
            for (int index = 0; index < test.depth; ++index) {
                seed_storage(nodes[index].storage, nodes[index].expected,
                             sizeof(nodes[index].storage));
                write_at(nodes[index].storage, 16 + 0x9C, test.flags[index]);
                // Innermost node terminates the chain with a null parent.
                Win *parent = (index + 1 < test.depth)
                    ? nodes[index + 1].object() : nullptr;
                write_at(nodes[index].storage, 16 + 0xC4, parent);
                std::memcpy(nodes[index].expected, nodes[index].storage,
                            sizeof(nodes[index].storage));
            }
            const int result = use_adapter
                ? win_is_visible_redirect(nodes[0].object(), nullptr)
                : nodes[0].object()->is_visible();
            expect(result == test.expected);
            // Visibility is a pure query: no node may be modified.
            for (int index = 0; index < test.depth; ++index) {
                expect_storage_bytes(nodes[index].storage, nodes[index].expected,
                                     sizeof(nodes[index].storage));
            }
        }
    }

    struct WrappingCase {
        uint32_t flags;
        uint32_t start_x;
        uint32_t start_y;
        uint32_t child_client_x;
        uint32_t child_client_y;
        uint32_t child_outer_x;
        uint32_t child_outer_y;
        uint32_t parent_client_x;
        uint32_t parent_client_y;
        uint32_t parent_outer_x;
        uint32_t parent_outer_y;
        uint32_t expected_x;
        uint32_t expected_y;
    };
    const WrappingCase wrapping_cases[] = {
        // The local client and outer additions wrap independently for both axes.
        {0x00000000U,
         0x7FFFFFFFU, 0x80000000U,
         0x00000001U, 0xFFFFFFFFU,
         0x00000001U, 0xFFFFFFFFU,
         0, 0, 0, 0,
         0x80000001U, 0x7FFFFFFEU},
        // The parent's additions receive already adjusted child coordinates.
        {0x00000020U,
         0x7FFFFFFFU, 0x80000000U,
         0, 0, 0, 0,
         0x00000001U, 0xFFFFFFFFU,
         0x00000001U, 0xFFFFFFFFU,
         0x80000001U, 0x7FFFFFFEU},
        // Parent client/outer terms cancel during recursion, leaving the
        // bit-15 subtraction itself to cross the signed boundary.
        {0x00008020U,
         0x80000000U, 0x7FFFFFFFU,
         0, 0, 0, 0,
         0xFFFFFFFFU, 0x00000001U,
         0x00000001U, 0xFFFFFFFFU,
         0x7FFFFFFFU, 0x80000000U},
    };
    for (const WrappingCase &test : wrapping_cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[2];
            for (WinNode &node : nodes) {
                seed_storage(node.storage, node.expected, sizeof(node.storage));
            }
            write_at(nodes[0].storage, 16 + 0x98, test.flags);
            write_at(nodes[0].storage, 16 + 0x14C, test.child_client_x);
            write_at(nodes[0].storage, 16 + 0x150, test.child_client_y);
            write_at(nodes[0].storage, 16 + 0x13C, test.child_outer_x);
            write_at(nodes[0].storage, 16 + 0x140, test.child_outer_y);
            write_at(nodes[0].storage, 16 + 0xC4,
                     (test.flags & 0x20U) ? nodes[1].object() : nullptr);
            write_at(nodes[1].storage, 16 + 0x98, 0U);
            write_at(nodes[1].storage, 16 + 0x14C, test.parent_client_x);
            write_at(nodes[1].storage, 16 + 0x150, test.parent_client_y);
            write_at(nodes[1].storage, 16 + 0x13C, test.parent_outer_x);
            write_at(nodes[1].storage, 16 + 0x140, test.parent_outer_y);
            write_at(nodes[1].storage, 16 + 0xC4, static_cast<Win *>(nullptr));
            for (WinNode &node : nodes) {
                std::memcpy(node.expected, node.storage, sizeof(node.storage));
            }
            int x = int_from_bits(test.start_x);
            int y = int_from_bits(test.start_y);
            if (use_adapter) {
                win_client_to_screen_redirect(nodes[0].object(), nullptr, &x, &y);
            } else {
                nodes[0].object()->client_to_screen(&x, &y);
            }
            expect(static_cast<uint32_t>(x) == test.expected_x);
            expect(static_cast<uint32_t>(y) == test.expected_y);
            for (WinNode &node : nodes) {
                expect_storage_bytes(node.storage, node.expected,
                                     sizeof(node.storage));
            }
        }
    }
}

void test_win_client_to_screen() {
    // Each node contributes client_rect_.left/top (0x14C/0x150) plus
    // outer_rect_.left/top (0x13C/0x140); bit 5 of the dword at 0x98 continues
    // the walk to the parent at 0xC4, and bit 15 then backs the parent's outer
    // origin out again. Nodes carry distinct values so a dropped or misrouted
    // term changes the total.
    struct WinNode {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(Win) + 32];

        Win *object() { return reinterpret_cast<Win *>(storage + 16); }
    };
    struct ChainCase {
        int depth;
        uint32_t flags[3];
        int start_x;
        int start_y;
        int expected_x;
        int expected_y;
    };
    // Per-node geometry: client (100,200)*(n+1), outer (10,20)*(n+1).
    const ChainCase cases[] = {
        // No walk: bit 5 clear stops at this node regardless of a parent.
        {1, {0x00000000U, 0, 0}, 0, 0, 110, 220},
        {2, {0x00000000U, 0x00000020U, 0}, 0, 0, 110, 220},
        // The start point is accumulated, not replaced.
        {1, {0x00000000U, 0, 0}, 7, -9, 117, 211},
        // Bit 5 with a null parent still stops.
        {1, {0x00000020U, 0, 0}, 0, 0, 110, 220},
        // Two-node walk: 110+220 = 330, 220+440 = 660.
        {2, {0x00000020U, 0x00000000U, 0}, 0, 0, 330, 660},
        // Bit 15 without bit 5 is inert - no walk, so nothing to back out.
        {1, {0x00008000U, 0, 0}, 0, 0, 110, 220},
        // Bit 15 with the walk subtracts the parent's outer origin (20,40).
        {2, {0x00008020U, 0x00000000U, 0}, 0, 0, 310, 620},
        // Three-node walk: 110+220+330 = 660, 220+440+660 = 1320.
        {3, {0x00000020U, 0x00000020U, 0x00000000U}, 0, 0, 660, 1320},
        // Innermost node also backs out its own parent's outer origin (30,60).
        {3, {0x00000020U, 0x00008020U, 0x00000000U}, 0, 0, 630, 1260},
        // Both levels back out: -(20,40) at the root, -(30,60) one level in.
        {3, {0x00008020U, 0x00008020U, 0x00000000U}, 0, 0, 610, 1220},
        // Unrelated flag bits must not gate either behaviour.
        {2, {0xFFFF7FDFU, 0U, 0}, 0, 0, 110, 220},
        {2, {0xFFFFFFFFU, 0U, 0}, 0, 0, 310, 620},
    };
    for (const ChainCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[3];
            for (int index = 0; index < test.depth; ++index) {
                seed_storage(nodes[index].storage, nodes[index].expected,
                             sizeof(nodes[index].storage));
                write_at(nodes[index].storage, 16 + 0x98, test.flags[index]);
                const int scale = index + 1;
                write_at(nodes[index].storage, 16 + 0x14C, 100 * scale);
                write_at(nodes[index].storage, 16 + 0x150, 200 * scale);
                write_at(nodes[index].storage, 16 + 0x13C, 10 * scale);
                write_at(nodes[index].storage, 16 + 0x140, 20 * scale);
                Win *parent = (index + 1 < test.depth)
                    ? nodes[index + 1].object() : nullptr;
                write_at(nodes[index].storage, 16 + 0xC4, parent);
                std::memcpy(nodes[index].expected, nodes[index].storage,
                            sizeof(nodes[index].storage));
            }
            int x = test.start_x;
            int y = test.start_y;
            if (use_adapter) {
                win_client_to_screen_redirect(nodes[0].object(), nullptr, &x, &y);
            } else {
                nodes[0].object()->client_to_screen(&x, &y);
            }
            expect(x == test.expected_x);
            expect(y == test.expected_y);
            // The translation is a pure query: no node may be modified.
            for (int index = 0; index < test.depth; ++index) {
                expect_storage_bytes(nodes[index].storage, nodes[index].expected,
                                     sizeof(nodes[index].storage));
            }
        }
    }
}

void test_sprite_construct() {
    static_assert(sizeof(Sprite) == 0x2C, "Sprite tests require the legacy layout");
    int *const saved_total = SpriteMemoryUsed;
    const int32_t starting_totals[] = {
        0, 1, -1, 0x7FFFFFFF, static_cast<int32_t>(0x80000000U),
    };
    for (int32_t starting_total : starting_totals) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            // Every field the constructor writes, in legacy layout order.
            write_at(expected, 16 + 0x00, 0U);
            write_at(expected, 16 + 0x04, 0U);
            expected[16 + 0x08] = 9;
            write_at(expected, 16 + 0x0C, 0U);
            write_at(expected, 16 + 0x10, 0U);
            write_at(expected, 16 + 0x14, 0U);
            write_at(expected, 16 + 0x18, 0U);
            write_at(expected, 16 + 0x1C, 0U);
            write_at(expected, 16 + 0x20, 0U);
            write_at(expected, 16 + 0x24, 0U);
            write_at(expected, 16 + 0x28, 0U);

            int32_t total = starting_total;
            SpriteMemoryUsed = &total;
            auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
            if (use_adapter) {
                expect(sprite_construct_redirect(sprite, nullptr) == sprite);
            } else {
                new (sprite) Sprite();
            }
            // Offsets 0x09-0x0B stay untouched: only a byte is written at 0x08.
            expect_storage_bytes(storage, expected, sizeof(storage));
            // The accounting total advances by exactly one object, wrapping.
            const int32_t advanced = static_cast<int32_t>(
                static_cast<uint32_t>(starting_total) + 0x2CU);
            expect(total == advanced);
        }
    }
    SpriteMemoryUsed = saved_total;
}

// Recording stand-ins installed into the subobject-destructor seams. The
// destructor's own Probe bookkeeping records its *intent* inline before each
// delegation, so a suite that asserts only on the probe cannot tell whether
// the delegation call itself happened - dropping either call left every probe
// value intact. These stubs are the record of the calls actually being made.
struct GraphicWinStubRecord {
    int buffer_calls;
    void *buffer_target;
    int win_calls;
    void *win_target;
    int sequence;
};
GraphicWinStubRecord graphic_win_stub_record = {};

bool button_lifecycle_capture = false;
uint8_t *button_lifecycle_base = nullptr;
int button_lifecycle_close_calls = 0;
int button_lifecycle_close_time_counts[4] = {};
uint32_t button_lifecycle_close_primary_vtables[4] = {};
uint32_t button_lifecycle_close_buffer_vtables[4] = {};
uint32_t button_lifecycle_close_a14[4] = {};
uint32_t button_lifecycle_close_a18[4] = {};
uint32_t button_lifecycle_close_ab8[4] = {};
uint32_t button_lifecycle_close_abc[4] = {};
int button_lifecycle_destructor_time_count = -1;

uint32_t button_lifecycle_read(size_t offset) {
    uint32_t value;
    std::memcpy(&value, button_lifecycle_base + offset, sizeof(value));
    return value;
}

void reset_button_lifecycle_capture(uint8_t *base) {
    button_lifecycle_capture = true;
    button_lifecycle_base = base;
    button_lifecycle_close_calls = 0;
    std::memset(button_lifecycle_close_time_counts, 0,
                sizeof(button_lifecycle_close_time_counts));
    std::memset(button_lifecycle_close_primary_vtables, 0,
                sizeof(button_lifecycle_close_primary_vtables));
    std::memset(button_lifecycle_close_buffer_vtables, 0,
                sizeof(button_lifecycle_close_buffer_vtables));
    std::memset(button_lifecycle_close_a14, 0,
                sizeof(button_lifecycle_close_a14));
    std::memset(button_lifecycle_close_a18, 0,
                sizeof(button_lifecycle_close_a18));
    std::memset(button_lifecycle_close_ab8, 0,
                sizeof(button_lifecycle_close_ab8));
    std::memset(button_lifecycle_close_abc, 0,
                sizeof(button_lifecycle_close_abc));
    button_lifecycle_destructor_time_count = -1;
    time_close_calls = 0;
    std::memset(time_close_targets, 0, sizeof(time_close_targets));
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall graphic_win_stub_buffer_destructor(void *target) {
    if (button_lifecycle_capture) {
        button_lifecycle_destructor_time_count = time_close_calls;
    }
    graphic_win_stub_record.buffer_calls++;
    graphic_win_stub_record.buffer_target = target;
    graphic_win_stub_record.sequence =
        (graphic_win_stub_record.sequence << 4) | 2;
}

void __thiscall graphic_win_stub_win_destructor(void *target) {
    graphic_win_stub_record.win_calls++;
    graphic_win_stub_record.win_target = target;
    graphic_win_stub_record.sequence =
        (graphic_win_stub_record.sequence << 4) | 1;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void test_graphic_win_destructor() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "GraphicWin tests require the legacy layout");
    // The destructor stores both vtables, clears the trailing field, then
    // delegates to the Buffer and Win subobject destructors. The delegation
    // targets are original dependencies, so the source test substitutes
    // recording stubs and asserts the observable stores and call order.
    for (int null_self = 0; null_self < 2; ++null_self) {
        alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        auto *object = reinterpret_cast<GraphicWin *>(storage + 16);

        graphic_win_destructor_probe_reset();
        graphic_win_stub_record = GraphicWinStubRecord{};
        func_subobject_destructor *const saved_buffer = BufferSubobjectDestructor;
        func_subobject_destructor *const saved_win = WinOriginalDestructor;
        BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
        WinOriginalDestructor = graphic_win_stub_win_destructor;
        GraphicWin *const target = null_self ? nullptr : object;
        expect(graphic_win_destructor_redirect(target, nullptr) == target);
        BufferSubobjectDestructor = saved_buffer;
        WinOriginalDestructor = saved_win;

        if (null_self) {
            // Nothing may be written when the instance pointer is null.
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(graphic_win_destructor_probe_buffer_calls() == 0);
            expect(graphic_win_destructor_probe_win_calls() == 0);
            expect(graphic_win_stub_record.buffer_calls == 0);
            expect(graphic_win_stub_record.win_calls == 0);
            continue;
        }
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);
        write_at(expected, 16 + 0xA10, 0U);
        expect_storage_bytes(storage, expected, sizeof(storage));
        // Buffer subobject first, then the Win base, matching legacy order.
        expect(graphic_win_destructor_probe_buffer_calls() == 1);
        expect(graphic_win_destructor_probe_win_calls() == 1);
        expect(graphic_win_destructor_probe_buffer_target()
               == reinterpret_cast<void *>(storage + 16 + 0x444));
        expect(graphic_win_destructor_probe_win_target()
               == reinterpret_cast<void *>(storage + 16));
        expect(graphic_win_destructor_probe_order() == 0x21);
        // The stubs prove the delegation calls were made, not merely
        // recorded as intended by the inline probe.
        expect(graphic_win_stub_record.buffer_calls == 1);
        expect(graphic_win_stub_record.win_calls == 1);
        expect(graphic_win_stub_record.buffer_target
               == reinterpret_cast<void *>(storage + 16 + 0x444));
        expect(graphic_win_stub_record.win_target
               == reinterpret_cast<void *>(storage + 16));
        expect(graphic_win_stub_record.sequence == 0x21);
    }
}

struct GraphicWinCloseStubRecord {
    int win_calls;
    void *win_target;
    int buffer_calls;
    void *buffer_target;
    int release_calls;
    void *release_target;
    uint32_t release_flags;
    uint32_t sequence;
    uint32_t dependency_seen_a10[2];
    uint32_t dependency_seen_134[2];
    uint32_t dependency_seen_9cc[2];
    uint32_t release_seen_a10;
    uint32_t release_seen_134;
    uint32_t release_seen_9cc;
    uint32_t release_seen_a08;
    uint32_t release_seen_a0c;
    bool replace_a08;
    void *replacement_a08;
};
GraphicWinCloseStubRecord graphic_win_close_stub_record = {};

uint32_t graphic_win_field(void *self, size_t offset) {
    uint32_t value;
    std::memcpy(&value, static_cast<uint8_t *>(self) + offset, sizeof(value));
    return value;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall graphic_win_stub_win_close(void *target) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    if (button_lifecycle_capture) {
        const int call = button_lifecycle_close_calls++;
        if (call < static_cast<int>(
                ARRAYSIZE(button_lifecycle_close_time_counts))) {
            button_lifecycle_close_time_counts[call] = time_close_calls;
            button_lifecycle_close_primary_vtables[call] =
                button_lifecycle_read(0x000);
            button_lifecycle_close_buffer_vtables[call] =
                button_lifecycle_read(0x444);
            button_lifecycle_close_a14[call] =
                button_lifecycle_read(0xA14);
            button_lifecycle_close_a18[call] =
                button_lifecycle_read(0xA18);
            button_lifecycle_close_ab8[call] =
                button_lifecycle_read(0xAB8);
            button_lifecycle_close_abc[call] =
                button_lifecycle_read(0xABC);
        }
    }
    record.win_calls++;
    record.win_target = target;
    record.sequence = (record.sequence << 4) | 1U;
    record.dependency_seen_a10[0] = graphic_win_field(target, 0xA10);
    record.dependency_seen_134[0] = graphic_win_field(target, 0x134);
    record.dependency_seen_9cc[0] = graphic_win_field(target, 0x9CC);
}

void __thiscall graphic_win_stub_buffer_close(void *target) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    auto *const self = static_cast<uint8_t *>(target) - 0x444;
    record.buffer_calls++;
    record.buffer_target = target;
    record.sequence = (record.sequence << 4) | 2U;
    record.dependency_seen_a10[1] = graphic_win_field(self, 0xA10);
    record.dependency_seen_134[1] = graphic_win_field(self, 0x134);
    record.dependency_seen_9cc[1] = graphic_win_field(self, 0x9CC);
    if (record.replace_a08) {
        write_at(self, 0xA08, record.replacement_a08);
    }
}

uint32_t __thiscall graphic_win_stub_release(void *target, uint32_t flags) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    record.release_calls++;
    record.release_target = target;
    record.release_flags = flags;
    record.sequence = (record.sequence << 4) | 3U;
    record.release_seen_a10 = graphic_win_field(record.win_target, 0xA10);
    record.release_seen_134 = graphic_win_field(record.win_target, 0x134);
    record.release_seen_9cc = graphic_win_field(record.win_target, 0x9CC);
    record.release_seen_a08 = graphic_win_field(record.win_target, 0xA08);
    record.release_seen_a0c = graphic_win_field(record.win_target, 0xA0C);
    return 0x7B3D19E5U;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void test_graphic_win_close() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "GraphicWin close tests require the legacy layout");
    struct FakeReleaseObject {
        uintptr_t *vtable;
    };
    uintptr_t release_vtable[] = {
        reinterpret_cast<uintptr_t>(&graphic_win_stub_release),
    };
    FakeReleaseObject release_object = {release_vtable};

    // mode 0: null throughout; mode 1: present throughout; modes 2 and 3
    // prove field_A08_ is loaded after both dependency calls by having the
    // Buffer close stand-in install or remove the release target.
    for (int mode = 0; mode < 4; ++mode) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            auto *const self = reinterpret_cast<GraphicWin *>(storage + 16);
            void *const initial_target = (mode == 1 || mode == 3)
                ? &release_object : nullptr;
            void *const final_target = (mode == 1 || mode == 2)
                ? &release_object : nullptr;
            write_at(storage, 16 + 0xA08, initial_target);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t initial_a10 = graphic_win_field(storage + 16, 0xA10);
            const uint32_t initial_134 = graphic_win_field(storage + 16, 0x134);
            const uint32_t initial_9cc = graphic_win_field(storage + 16, 0x9CC);

            const uint32_t zero = 0;
            write_at(expected, 16 + 0xA10, zero);
            write_at(expected, 16 + 0x134, zero);
            write_at(expected, 16 + 0x138, zero);
            for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
                write_at(expected, 16 + offset, zero);
            }
            uint32_t default_a0c = 0xC6A51F73U;
            write_at(expected, 16 + 0xA08, static_cast<void *>(nullptr));
            write_at(expected, 16 + 0xA0C, default_a0c);

            func_subobject_close *const saved_win = WinOriginalClose;
            func_subobject_close *const saved_buffer = BufferSubobjectClose;
            uint32_t *const saved_default = GraphicWinFieldA0CDefault;
            WinOriginalClose = graphic_win_stub_win_close;
            BufferSubobjectClose = graphic_win_stub_buffer_close;
            GraphicWinFieldA0CDefault = &default_a0c;
            graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
            graphic_win_close_stub_record.replace_a08 = mode >= 2;
            graphic_win_close_stub_record.replacement_a08 = final_target;

            const uint32_t result = use_adapter
                ? graphic_win_close_redirect(self, nullptr)
                : self->close();

            WinOriginalClose = saved_win;
            BufferSubobjectClose = saved_buffer;
            GraphicWinFieldA0CDefault = saved_default;

            const GraphicWinCloseStubRecord &record =
                graphic_win_close_stub_record;
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(record.win_calls == 1);
            expect(record.win_target == self);
            expect(record.buffer_calls == 1);
            expect(record.buffer_target == storage + 16 + 0x444);
            expect(record.sequence == (final_target ? 0x123U : 0x12U));
            for (int dependency = 0; dependency < 2; ++dependency) {
                expect(record.dependency_seen_a10[dependency] == initial_a10);
                expect(record.dependency_seen_134[dependency] == initial_134);
                expect(record.dependency_seen_9cc[dependency] == initial_9cc);
            }
            if (final_target) {
                expect(result == 0x7B3D19E5U);
                expect(record.release_calls == 1);
                expect(record.release_target == &release_object);
                expect(record.release_flags == 1U);
                expect(record.release_seen_a10 == 0);
                expect(record.release_seen_134 == 0);
                expect(record.release_seen_9cc == 0);
                expect(record.release_seen_a08
                       == reinterpret_cast<uintptr_t>(&release_object));
                expect(record.release_seen_a0c == default_a0c);
            } else {
                expect(result == default_a0c);
                expect(record.release_calls == 0);
            }
        }
    }
}

int button_lifecycle_free_calls = 0;
void *button_lifecycle_free_targets[2] = {};
uint32_t button_lifecycle_free_a74[2] = {};
uint32_t button_lifecycle_free_a84[2] = {};
uint32_t button_lifecycle_free_a7c[2] = {};
uint32_t button_lifecycle_free_a80[2] = {};
uint32_t button_lifecycle_free_aa8[2] = {};

void reset_button_lifecycle_frees() {
    button_lifecycle_free_calls = 0;
    std::memset(button_lifecycle_free_targets, 0,
                sizeof(button_lifecycle_free_targets));
    std::memset(button_lifecycle_free_a74, 0,
                sizeof(button_lifecycle_free_a74));
    std::memset(button_lifecycle_free_a84, 0,
                sizeof(button_lifecycle_free_a84));
    std::memset(button_lifecycle_free_a7c, 0,
                sizeof(button_lifecycle_free_a7c));
    std::memset(button_lifecycle_free_a80, 0,
                sizeof(button_lifecycle_free_a80));
    std::memset(button_lifecycle_free_aa8, 0,
                sizeof(button_lifecycle_free_aa8));
}

void *button_lifecycle_free_probe(void *block) {
    const int call = button_lifecycle_free_calls++;
    if (call < static_cast<int>(ARRAYSIZE(button_lifecycle_free_targets))) {
        button_lifecycle_free_targets[call] = block;
        button_lifecycle_free_a74[call] = button_lifecycle_read(0xA74);
        button_lifecycle_free_a84[call] = button_lifecycle_read(0xA84);
        button_lifecycle_free_a7c[call] = button_lifecycle_read(0xA7C);
        button_lifecycle_free_a80[call] = button_lifecycle_read(0xA80);
        button_lifecycle_free_aa8[call] = button_lifecycle_read(0xAA8);
    }
    return reinterpret_cast<void *>(
        0xC1050000U + static_cast<uint32_t>(call + 1));
}

void write_button_graphic_close_expected(
        uint8_t *expected, uint32_t graphic_default) {
    write_at(expected, 16 + 0x134, 0U);
    write_at(expected, 16 + 0x138, 0U);
    for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA08, 0U);
    write_at(expected, 16 + 0xA0C, graphic_default);
    write_at(expected, 16 + 0xA10, 0U);
}

void write_base_button_close_expected(
        uint8_t *expected, const uint32_t fixed[5],
        const uint32_t dynamic[2], uint32_t graphic_default) {
    write_button_graphic_close_expected(expected, graphic_default);
    write_at(expected, 16 + 0xA44, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA48, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA74, 0U);
    write_at(expected, 16 + 0xA78, 0U);
    write_at(expected, 16 + 0xA7C, 0U);
    write_at(expected, 16 + 0xA80, 0U);
    for (size_t index = 0; index < 4; ++index) {
        write_at(expected, 16 + 0xA84 + index * 4, fixed[index]);
    }
    write_at(expected, 16 + 0xA94, dynamic[0]);
    write_at(expected, 16 + 0xA98, dynamic[1]);
    write_at(expected, 16 + 0xA9C, 0U);
    write_at(expected, 16 + 0xAA0, fixed[4]);
    write_at(expected, 16 + 0xAA4, 0U);
    write_at(expected, 16 + 0xAA8, 0U);
    write_at(expected, 16 + 0xAAC, 0U);
    write_at(expected, 16 + 0xAB0, 0U);
    write_at(expected, 16 + 0xAB4, 0U);
}

void write_time_close_expected(uint8_t *expected, size_t offset) {
    std::memset(expected + 16 + offset, 0, sizeof(Time));
    write_at(expected, 16 + offset + 0x20, 5U);
}

void write_flat_button_close_expected(
        uint8_t *expected, const uint32_t base_fixed[5],
        const uint32_t base_dynamic[2], const uint32_t flat_defaults[27],
        uint32_t graphic_default) {
    write_time_close_expected(expected, 0xA1C);
    write_at(expected, 16 + 0xA14, 0U);
    write_at(expected, 16 + 0xA18, 0U);
    write_at(expected, 16 + 0xAB8, 0xFFFFFFFFU);
    write_base_button_close_expected(
        expected, base_fixed, base_dynamic, graphic_default);
    for (size_t index = 0; index < 3; ++index) {
        write_at(expected, 16 + 0xABC + index * 4, 0xFFFFFFFFU);
        write_at(expected, 16 + 0xAC8 + index * 4, 0U);
        write_at(expected, 16 + 0xAD4 + index * 4, 0U);
        for (size_t group = 0; group < 9; ++group) {
            write_at(expected,
                     16 + 0xAE0 + group * 0xC + index * 4,
                     flat_defaults[group * 3 + index]);
        }
    }
}

void prepare_button_lifecycle_storage(
        uint8_t *storage, uint8_t *expected, size_t size) {
    seed_storage(storage, expected, size);
    write_at(storage, 16 + 0xA08, 0U);
    write_at(storage, 16 + 0xA7C, 0U);
    write_at(storage, 16 + 0xA80, 0U);
    std::memcpy(expected, storage, size);
}

void test_base_button_and_flat_button_lifecycle() {
    static_assert(sizeof(BaseButton) == 0xAB8,
                  "BaseButton lifecycle tests require the legacy layout");
    static_assert(sizeof(FlatButton) == 0xB4C,
                  "FlatButton lifecycle tests require the legacy layout");

    uint32_t base_fixed[5];
    uint32_t base_dynamic[2];
    uint32_t flat_defaults[27];
    for (size_t index = 0; index < ARRAYSIZE(base_fixed); ++index) {
        base_fixed[index] = 0x51000000U
            + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(base_dynamic); ++index) {
        base_dynamic[index] = 0xA1000000U
            + static_cast<uint32_t>(index) * 0x011011U;
    }
    for (size_t index = 0; index < ARRAYSIZE(flat_defaults); ++index) {
        flat_defaults[index] = 0xD1000000U
            + static_cast<uint32_t>(index) * 0x010101U;
    }
    uint32_t graphic_default = 0x7B3D19E5U;

    func2 *const saved_free = _free;
    uint32_t *const saved_base_fixed = BaseButtonStaticDefaults;
    uint32_t *const saved_base_dynamic = BaseButtonDynamicDefaults;
    uint32_t *const saved_flat_defaults = FlatButtonDefaults;
    func_subobject_close *const saved_win_close = WinOriginalClose;
    func_subobject_close *const saved_buffer_close = BufferSubobjectClose;
    uint32_t *const saved_graphic_default = GraphicWinFieldA0CDefault;
    func_subobject_destructor *const saved_buffer_destructor =
        BufferSubobjectDestructor;
    func_subobject_destructor *const saved_win_destructor =
        WinOriginalDestructor;
    _free = button_lifecycle_free_probe;
    BaseButtonStaticDefaults = base_fixed;
    BaseButtonDynamicDefaults = base_dynamic;
    FlatButtonDefaults = flat_defaults;
    WinOriginalClose = graphic_win_stub_win_close;
    BufferSubobjectClose = graphic_win_stub_buffer_close;
    GraphicWinFieldA0CDefault = &graphic_default;
    BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
    WinOriginalDestructor = graphic_win_stub_win_destructor;

    // BaseButton::close: all four allocation shapes, through both entry
    // points. The free probe observes the exact name-before-bubble order and
    // the pre-clear pointer values; the second free's return residue survives.
    for (int allocation_mask = 0; allocation_mask < 4; ++allocation_mask) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(BaseButton) uint8_t storage[sizeof(BaseButton) + 32];
            uint8_t expected[sizeof(storage)];
            prepare_button_lifecycle_storage(
                storage, expected, sizeof(storage));
            auto *const self = reinterpret_cast<BaseButton *>(storage + 16);
            void *const name = allocation_mask & 1
                ? reinterpret_cast<void *>(0x11110001U) : nullptr;
            void *const bubble = allocation_mask & 2
                ? reinterpret_cast<void *>(0x22220002U) : nullptr;
            write_at(storage, 16 + 0xA7C, name);
            write_at(storage, 16 + 0xA80, bubble);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t initial_primary = graphic_win_field(self, 0x000);
            const uint32_t initial_buffer = graphic_win_field(self, 0x444);
            const uint32_t initial_a14 = graphic_win_field(self, 0xA14);
            const uint32_t initial_a18 = graphic_win_field(self, 0xA18);
            const uint32_t initial_ab8 = graphic_win_field(self, 0xAB8);
            const uint32_t initial_abc = graphic_win_field(self, 0xABC);
            const uint32_t initial_aa8 = graphic_win_field(self, 0xAA8);
            write_base_button_close_expected(
                expected, base_fixed, base_dynamic, graphic_default);

            reset_button_lifecycle_capture(storage + 16);
            reset_button_lifecycle_frees();
            graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
            const uint32_t result = use_adapter
                ? base_button_close_redirect(self, nullptr) : self->close();

            const int expected_frees = (name ? 1 : 0) + (bubble ? 1 : 0);
            const uint32_t expected_result = bubble
                ? 0xC1050000U + static_cast<uint32_t>(expected_frees) : 0U;
            expect(result == expected_result);
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(button_lifecycle_close_calls == 1);
            expect(button_lifecycle_close_time_counts[0] == 0);
            expect(button_lifecycle_close_primary_vtables[0]
                   == initial_primary);
            expect(button_lifecycle_close_buffer_vtables[0]
                   == initial_buffer);
            expect(button_lifecycle_close_a14[0] == initial_a14);
            expect(button_lifecycle_close_a18[0] == initial_a18);
            expect(button_lifecycle_close_ab8[0] == initial_ab8);
            expect(button_lifecycle_close_abc[0] == initial_abc);
            expect(button_lifecycle_free_calls == expected_frees);
            int free_index = 0;
            if (name) {
                expect(button_lifecycle_free_targets[free_index] == name);
                expect(button_lifecycle_free_a7c[free_index]
                       == reinterpret_cast<uintptr_t>(name));
                expect(button_lifecycle_free_a80[free_index]
                       == reinterpret_cast<uintptr_t>(bubble));
                ++free_index;
            }
            if (bubble) {
                expect(button_lifecycle_free_targets[free_index] == bubble);
                expect(button_lifecycle_free_a7c[free_index] == 0U);
                expect(button_lifecycle_free_a80[free_index]
                       == reinterpret_cast<uintptr_t>(bubble));
            }
            for (int call = 0; call < expected_frees; ++call) {
                expect(button_lifecycle_free_a74[call] == 0U);
                expect(button_lifecycle_free_a84[call] == base_fixed[0]);
                expect(button_lifecycle_free_aa8[call] == initial_aa8);
            }
        }
    }

    // FlatButton::close: Time closes before the Flat fields, Base close sees
    // the orientation fields but not the later table reset, and EAX retains
    // the legacy interior pointer computed by the final loop.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(FlatButton) uint8_t storage[sizeof(FlatButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<FlatButton *>(storage + 16);
        const uint32_t initial_primary = graphic_win_field(self, 0x000);
        const uint32_t initial_buffer = graphic_win_field(self, 0x444);
        const uint32_t initial_abc = graphic_win_field(self, 0xABC);
        write_flat_button_close_expected(
            expected, base_fixed, base_dynamic, flat_defaults,
            graphic_default);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        const uint32_t result = use_adapter
            ? flat_button_close_redirect(self, nullptr) : self->close();

        expect(result == reinterpret_cast<uintptr_t>(self) + 0xAECU);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(time_close_calls == 1);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_close_calls == 1);
        expect(button_lifecycle_close_time_counts[0] == 1);
        expect(button_lifecycle_close_primary_vtables[0] == initial_primary);
        expect(button_lifecycle_close_buffer_vtables[0] == initial_buffer);
        expect(button_lifecycle_close_a14[0] == 0U);
        expect(button_lifecycle_close_a18[0] == 0U);
        expect(button_lifecycle_close_ab8[0] == 0xFFFFFFFFU);
        expect(button_lifecycle_close_abc[0] == initial_abc);
        expect(button_lifecycle_free_calls == 0);
    }

    // BaseButton destructor: install its two vtables, close, destroy Time2
    // then Time1, and delegate to GraphicWin teardown. The final GraphicWin
    // tables hide the initial stores, so the close seam snapshots them.
    {
        alignas(BaseButton) uint8_t storage[sizeof(BaseButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<BaseButton *>(storage + 16);
        write_base_button_close_expected(
            expected, base_fixed, base_dynamic, graphic_default);
        write_time_close_expected(expected, 0xA4C);
        write_time_close_expected(expected, 0xA1C);
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        graphic_win_stub_record = GraphicWinStubRecord{};
        expect(base_button_destructor_redirect(self, nullptr) == self);

        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(button_lifecycle_close_calls == 1);
        expect(button_lifecycle_close_time_counts[0] == 0);
        expect(button_lifecycle_close_primary_vtables[0]
               == BaseButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[0]
               == BaseButtonBufferVtable);
        expect(time_close_calls == 2);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA4C));
        expect(time_close_targets[1]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_destructor_time_count == 2);
        expect(graphic_win_stub_record.sequence == 0x21);
        expect(button_lifecycle_free_calls == 0);
    }

    // FlatButton destructor composes Flat close with the complete BaseButton
    // destructor. This intentionally closes Time1 twice, exactly as the
    // original two-stage destructor does.
    {
        alignas(FlatButton) uint8_t storage[sizeof(FlatButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<FlatButton *>(storage + 16);
        const uint32_t initial_abc = graphic_win_field(self, 0xABC);
        write_flat_button_close_expected(
            expected, base_fixed, base_dynamic, flat_defaults,
            graphic_default);
        write_time_close_expected(expected, 0xA4C);
        write_time_close_expected(expected, 0xA1C);
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        graphic_win_stub_record = GraphicWinStubRecord{};
        expect(flat_button_destructor_redirect(self, nullptr) == self);

        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(button_lifecycle_close_calls == 2);
        expect(button_lifecycle_close_time_counts[0] == 1);
        expect(button_lifecycle_close_time_counts[1] == 1);
        expect(button_lifecycle_close_primary_vtables[0]
               == FlatButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[0]
               == FlatButtonBufferVtable);
        expect(button_lifecycle_close_primary_vtables[1]
               == BaseButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[1]
               == BaseButtonBufferVtable);
        expect(button_lifecycle_close_abc[0] == initial_abc);
        expect(button_lifecycle_close_abc[1] == 0xFFFFFFFFU);
        expect(time_close_calls == 3);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(time_close_targets[1]
               == reinterpret_cast<Time *>(storage + 16 + 0xA4C));
        expect(time_close_targets[2]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_destructor_time_count == 3);
        expect(graphic_win_stub_record.sequence == 0x21);
        expect(button_lifecycle_free_calls == 0);
    }

    button_lifecycle_capture = false;
    button_lifecycle_base = nullptr;
    _free = saved_free;
    BaseButtonStaticDefaults = saved_base_fixed;
    BaseButtonDynamicDefaults = saved_base_dynamic;
    FlatButtonDefaults = saved_flat_defaults;
    WinOriginalClose = saved_win_close;
    BufferSubobjectClose = saved_buffer_close;
    GraphicWinFieldA0CDefault = saved_graphic_default;
    BufferSubobjectDestructor = saved_buffer_destructor;
    WinOriginalDestructor = saved_win_destructor;
    time_close_calls = 0;
    std::memset(time_close_targets, 0, sizeof(time_close_targets));
}

int sprite_close_free_calls = 0;
void *sprite_close_free_targets[4];

void *sprite_close_free_probe(void *block) {
    if (sprite_close_free_calls < 4) {
        sprite_close_free_targets[sprite_close_free_calls] = block;
    }
    ++sprite_close_free_calls;
    return nullptr;
}

void test_sprite_close() {
    struct CloseCase {
        uint32_t field_00;   // primary allocation
        uint32_t field_04;   // pixel buffer
        uint32_t field_10;   // width
        uint32_t field_14;   // height
        uint32_t field_28;   // suppresses the pixel accounting when nonzero
        int expected_frees;
        bool accounts;
    };
    const CloseCase cases[] = {
        // Nothing owned: no frees, no accounting.
        {0, 0, 7, 11, 0, 0, false},
        // Pixel buffer owned and accounted, then freed.
        {0, 0x11110000U, 7, 11, 0, 1, true},
        // field_28 set suppresses both the accounting and the pixel free.
        {0, 0x11110000U, 7, 11, 1, 0, false},
        // Primary allocation only.
        {0x22220000U, 0, 7, 11, 0, 1, false},
        // Both owned: pixel buffer freed before the primary allocation.
        {0x22220000U, 0x11110000U, 3, 5, 0, 2, true},
        // Wrapping accounting arithmetic.
        {0, 0x11110000U, 0xFFFFFFFFU, 2, 0, 1, true},
        {0, 0x11110000U, 0x80000000U, 0x80000000U, 0, 1, true},
    };
    for (const CloseCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x00, test.field_00);
            write_at(storage, 16 + 0x04, test.field_04);
            write_at(storage, 16 + 0x10, test.field_10);
            write_at(storage, 16 + 0x14, test.field_14);
            write_at(storage, 16 + 0x28, test.field_28);
            std::memcpy(expected, storage, sizeof(storage));
            // field_08 holds the type byte and is never cleared by close.
            if (test.field_28 == 0 && test.field_04 != 0) {
                write_at(expected, 16 + 0x04, 0U);
            }
            if (test.field_00 != 0) {
                write_at(expected, 16 + 0x00, 0U);
            }
            for (size_t offset = 0x0C; offset <= 0x28; offset += 4) {
                write_at(expected, 16 + offset, 0U);
            }

            const int32_t starting_total = 0x1000;
            int32_t total = starting_total;
            int *const saved_total = SpriteMemoryUsed;
            func_sprite_free *const saved_free = SpriteFree;
            SpriteMemoryUsed = &total;
            SpriteFree = &sprite_close_free_probe;
            sprite_close_free_calls = 0;

            auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
            if (use_adapter) {
                sprite_close_redirect(sprite, nullptr);
            } else {
                sprite->close();
            }
            SpriteFree = saved_free;
            SpriteMemoryUsed = saved_total;

            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(sprite_close_free_calls == test.expected_frees);
            const int32_t expected_total = test.accounts
                ? static_cast<int32_t>(
                      static_cast<uint32_t>(starting_total)
                      - test.field_14 * test.field_10)
                : starting_total;
            expect(total == expected_total);
            // The pixel buffer is released before the primary allocation.
            if (test.expected_frees == 2) {
                expect(sprite_close_free_targets[0]
                       == reinterpret_cast<void *>(test.field_04));
                expect(sprite_close_free_targets[1]
                       == reinterpret_cast<void *>(test.field_00));
            }
        }
    }
}

struct FakeSurface {
    void **vtable;
};

int surface_lock_calls = 0;
int surface_unlock_calls = 0;
void *surface_unlock_data = nullptr;
uint32_t surface_lock_pitch = 0;
uint32_t surface_lock_data = 0;
long surface_lock_result = 0;
uint32_t surface_lock_seen_size = 0;

long __stdcall fake_surface_lock(
        void *, void *rect, void *descriptor, uint32_t flags, void *event) {
    ++surface_lock_calls;
    auto *bytes = static_cast<uint8_t *>(descriptor);
    std::memcpy(&surface_lock_seen_size, bytes, sizeof(surface_lock_seen_size));
    expect(rect == nullptr && flags == 1U && event == nullptr);
    if (surface_lock_result == 0) {
        std::memcpy(bytes + 0x10, &surface_lock_pitch, sizeof(surface_lock_pitch));
        std::memcpy(bytes + 0x24, &surface_lock_data, sizeof(surface_lock_data));
    }
    return surface_lock_result;
}

long __stdcall fake_surface_unlock(void *, void *data) {
    ++surface_unlock_calls;
    surface_unlock_data = data;
    return 0;
}

void reset_surface_probes() {
    surface_lock_calls = 0;
    surface_unlock_calls = 0;
    surface_unlock_data = nullptr;
    surface_lock_seen_size = 0;
}

void test_buffer_get_data() {
    void *vtable[(0x80 / sizeof(void *)) + 1] = {};
    vtable[BufferSurfaceLockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_lock);
    vtable[BufferSurfaceUnlockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_unlock);
    FakeSurface surface = {vtable};

    struct GetCase {
        bool has_surface;
        uint32_t field_50;      // published data pointer
        uint32_t field_54;      // owned storage used without a surface
        uint32_t references;
        long lock_result;
        uint32_t lock_pitch;
        uint32_t lock_data;
        int expected_result;
        int expected_locks;
        uint32_t expected_references;
    };
    const GetCase cases[] = {
        // No surface, no storage: publishes zero and counts nothing.
        {false, 0x9999U, 0, 4, 0, 0, 0, 0, 0, 4},
        // No surface with storage: publishes it and counts one reference.
        {false, 0x9999U, 0x4444U, 4, 0, 0, 0, 0x4444, 0, 5},
        // Surface already published: counts without locking.
        {true, 0x5555U, 0, 7, 0, 0, 0, 0x5555, 0, 8},
        // Surface locks successfully and publishes pitch plus data.
        {true, 0, 0, 0, 0, 0x1234U, 0x8888U, 0x8888, 1, 1},
        // A failed lock publishes nothing and counts nothing.
        {true, 0, 0, 3, 1, 0x1234U, 0x8888U, 0, 1, 3},
    };
    for (const GetCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x50, test.field_50);
            write_at(storage, 16 + 0x54, test.field_54);
            write_at(storage, 16 + 0x58,
                     test.has_surface ? reinterpret_cast<uint32_t>(&surface) : 0U);
            write_at(storage, 16 + 0x6C, test.references);
            std::memcpy(expected, storage, sizeof(storage));

            reset_surface_probes();
            surface_lock_result = test.lock_result;
            surface_lock_pitch = test.lock_pitch;
            surface_lock_data = test.lock_data;

            auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
            const int result = use_adapter
                ? buffer_get_data_redirect(buffer, nullptr)
                : buffer->get_data();

            if (!test.has_surface) {
                write_at(expected, 16 + 0x50, test.field_54);
            } else if (test.lock_result == 0 && test.field_50 == 0) {
                write_at(expected, 16 + 0x4A8, test.lock_pitch);
                write_at(expected, 16 + 0x50, test.lock_data);
            }
            write_at(expected, 16 + 0x6C, test.expected_references);

            expect(result == test.expected_result);
            expect(surface_lock_calls == test.expected_locks);
            expect_storage_bytes(storage, expected, sizeof(storage));
            // The descriptor always announces its own size to the surface.
            if (test.expected_locks) {
                expect(surface_lock_seen_size == 0x6CU);
            }
        }
    }
}

void test_buffer_free_data() {
    void *vtable[(0x80 / sizeof(void *)) + 1] = {};
    vtable[BufferSurfaceLockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_lock);
    vtable[BufferSurfaceUnlockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_unlock);
    FakeSurface surface = {vtable};

    struct FreeCase {
        bool has_surface;
        uint32_t field_50;
        uint32_t references;
        int count;
        int expected_unlocks;
        bool clears;
        uint32_t expected_references;
    };
    const FreeCase cases[] = {
        // No surface: the release clears once the count reaches zero.
        {false, 0x5555U, 3, 1, 0, false, 2},
        {false, 0x5555U, 1, 1, 0, true, 0},
        {false, 0x5555U, 1, 4, 0, true, 0},
        {false, 0, 1, 1, 0, true, 0},
        // Surface: unlocking additionally requires published data.
        {true, 0x5555U, 3, 1, 0, false, 2},
        {true, 0x5555U, 1, 1, 1, true, 0},
        {true, 0, 1, 1, 0, false, 0},
        // Release counts are compared as signed, so a wrapping subtraction
        // that lands negative still releases.
        {true, 0x5555U, 0, static_cast<int>(0x80000000U), 1, true, 0},
        {false, 0x5555U, 0, static_cast<int>(0x80000000U), 0, true, 0},
        // A large positive remainder keeps the data published.
        {true, 0x5555U, 0x7FFFFFFFU, 1, 0, false, 0x7FFFFFFEU},
    };
    for (const FreeCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x50, test.field_50);
            write_at(storage, 16 + 0x58,
                     test.has_surface ? reinterpret_cast<uint32_t>(&surface) : 0U);
            write_at(storage, 16 + 0x6C, test.references);
            std::memcpy(expected, storage, sizeof(storage));

            reset_surface_probes();
            auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
            if (use_adapter) {
                buffer_free_data_redirect(buffer, nullptr, test.count);
            } else {
                buffer->free_data(test.count);
            }

            if (test.clears) {
                write_at(expected, 16 + 0x50, 0U);
                write_at(expected, 16 + 0x6C, 0U);
            } else {
                write_at(expected, 16 + 0x6C, test.expected_references);
            }
            expect(surface_unlock_calls == test.expected_unlocks);
            if (test.expected_unlocks) {
                expect(surface_unlock_data
                       == reinterpret_cast<void *>(test.field_50));
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

struct RemoveAllProbe {
    int visits;
    int payload_destroys;
    int entry_destroys;
    void *last_payload;
    int last_flags;
};

RemoveAllProbe remove_all_probe = {};

void __thiscall remove_all_visitor(void *, void *payload) {
    ++remove_all_probe.visits;
    remove_all_probe.last_payload = payload;
}

void __thiscall remove_all_payload_destroy(void *, int flags) {
    ++remove_all_probe.payload_destroys;
    remove_all_probe.last_flags = flags;
}

void __thiscall remove_all_entry_destroy(void *, int flags) {
    ++remove_all_probe.entry_destroys;
    remove_all_probe.last_flags = flags;
}

// A destructible stand-in: the object's vtable slot 1 holds the virtual-base
// displacement to itself, so slot 0 of the same vtable is the destructor.
struct Destructible {
    uint32_t *vptr;
    uint32_t vtable[2];
};

void arm_destructible(Destructible &object, void *destructor) {
    object.vtable[0] = reinterpret_cast<uint32_t>(destructor);
    object.vtable[1] = 0;   // displacement selects the object itself
    object.vptr = object.vtable;
}

void test_string_struct_remove_all() {
    struct RemoveCase {
        int entries;
        int count;
        bool with_payloads;
    };
    const RemoveCase cases[] = {
        {0, 0, false},      // empty list returns immediately
        {3, 0, true},       // non-positive count clears without walking
        {3, -1, true},
        {1, 1, true},
        {3, 3, true},
        {3, 3, false},      // null payloads skip the payload destructor
        {3, 2, true},       // count shorter than the list stops early
    };
    for (const RemoveCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));

            StringStructEntry entries[3] = {};
            Destructible entry_objects[3] = {};
            Destructible payloads[3] = {};
            uint32_t owner_vtable[2] = {};
            owner_vtable[1] = reinterpret_cast<uint32_t>(&remove_all_visitor);

            for (int index = 0; index < test.entries; ++index) {
                arm_destructible(entry_objects[index],
                                 reinterpret_cast<void *>(&remove_all_entry_destroy));
                arm_destructible(payloads[index],
                                 reinterpret_cast<void *>(&remove_all_payload_destroy));
                // Each entry is prefixed by its own vptr so the destructor
                // dispatch finds the stand-in vtable.
                entries[index].abi_word =
                    reinterpret_cast<uint32_t>(entry_objects[index].vtable);
                entries[index].payload = test.with_payloads
                    ? static_cast<int>(reinterpret_cast<uintptr_t>(&payloads[index]))
                    : 0;
                entries[index].next = (index + 1 < test.entries)
                    ? &entries[index + 1] : nullptr;
            }

            write_at(storage, 16 + 0x00, reinterpret_cast<uint32_t>(owner_vtable));
            write_at(storage, 16 + 0x08,
                     test.entries ? reinterpret_cast<uint32_t>(&entries[0]) : 0U);
            write_at(storage, 16 + 0x10, static_cast<uint32_t>(test.count));
            std::memcpy(expected, storage, sizeof(storage));

            remove_all_probe = RemoveAllProbe();
            auto *list = reinterpret_cast<StringStruct *>(storage + 16);
            if (use_adapter) {
                string_struct_remove_all_redirect(list, nullptr);
            } else {
                list->remove_all();
            }

            const int walked = (test.entries == 0) ? 0
                : (test.count > 0 ? (test.count < test.entries
                                     ? test.count : test.entries) : 0);
            if (test.entries != 0) {
                write_at(expected, 16 + 0x08, 0U);
                write_at(expected, 16 + 0x10, 0U);
                write_at(expected, 16 + 0x14, 0U);
                if (walked) {
                    // current_ trails the last entry the walk advanced past.
                    write_at(expected, 16 + 0x0C,
                             walked < test.entries
                                 ? reinterpret_cast<uint32_t>(&entries[walked])
                                 : 0U);
                }
            }
            expect(remove_all_probe.visits == walked);
            expect(remove_all_probe.entry_destroys == walked);
            expect(remove_all_probe.payload_destroys
                   == (test.with_payloads ? walked : 0));
            if (walked && test.with_payloads) {
                expect(remove_all_probe.last_flags == 1);
            }
            // Every walked entry has its payload field cleared.
            for (int index = 0; index < walked; ++index) {
                expect(entries[index].payload == 0);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_find_font() {
    // sizes and table each carry one entry past FontSizeTableCount: a decoy
    // the function must never read, sized so its style-slot pointer
    // arithmetic (index * 4) still lands inside table.
    int sizes[FontSizeTableCount + 1] = {
        8, 10, 12, 14, 16, 18, 20, 24, 28, 32, 40, 48, 9000,
    };
    Font table[(FontSizeTableCount + 1) * 4];
    int *const saved_sizes = FontSizeTable;
    Font *const saved_table = FontTable;
    FontSizeTable = sizes;
    FontTable = table;

    struct FontCase { int size; int style; int expected_index; };
    const FontCase cases[] = {
        {8, 0, 0}, {48, 3, 11},
        {13, 0, 2},    // ties resolve to the earlier entry: |12-13| < |14-13| is false
        {11, 1, 1},    // |10-11| == |12-11|, strict less-than keeps the first
        {0, 2, 0}, {-5, 3, 0},
        // The 9999 seed is a threshold: only when every candidate is at
        // least that far away does the lookup fall back to the first entry
        // instead of the nearest. The largest size is 48, so the boundary
        // sits at 48 + 9999.
        {100000, 0, 0}, {-100000, 1, 0},
        {10047, 0, 0},    // farthest delta is exactly 9999, strict < fails
        {10046, 1, 11},   // one closer, so the nearest entry wins
        {22, 0, 6},    // |20-22| == |24-22|, first wins
        {26, 1, 7},
        // Every real entry is 8952+ away from 9000, so the true bound must
        // stop at index 11 (size 48). Reading one past it would find the
        // decoy at index 12 with delta 0 and wrongly report that instead.
        {9000, 1, 11},
    };
    for (const FontCase &test : cases) {
        const Font *const result = find_font(test.size, test.style);
        expect(result == table + (test.style + test.expected_index * 4));
    }
    // The table is never modified by a lookup.
    FontSizeTable = saved_sizes;
    FontTable = saved_table;
}

void test_buffer_text_line_height() {
    struct HeightCase { int override_value; int line_height; int height; int expected; };
    const HeightCase cases[] = {
        {0, 111, 222, 222},          // zero is non-negative: height + 0
        {5, 111, 222, 227},
        {-1, 111, 222, 111},         // negative selects the natural line height
        {INT_MIN, 111, 222, 111},
        {INT_MAX, 111, 1, static_cast<int>(0x80000000U)},   // wraps
    };
    for (const HeightCase &test : cases) {
        for (int preset_font = 0; preset_font < 2; ++preset_font) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));

                alignas(Font) uint8_t font_storage[sizeof(Font)] = {};
                int *const fields = reinterpret_cast<int *>(font_storage);
                fields[0x00 / 4] = test.override_value;
                fields[0x0C / 4] = test.line_height;
                fields[0x10 / 4] = test.height;
                auto *font = reinterpret_cast<Font *>(font_storage);

                // The default-font global lives at a fixed address that is
                // only mapped inside the hybrid process, so the binding is
                // pointed at test storage.
                Font **const saved_default_ptr = FontDefaultPtr;
                Font *default_font = font;
                FontDefaultPtr = &default_font;
                if (preset_font) {
                    write_at(storage, 16 + 0x52C, font);
                } else {
                    write_at(storage, 16 + 0x52C, 0U);
                }
                std::memcpy(expected, storage, sizeof(storage));
                // A missing font is filled in from the global default.
                write_at(expected, 16 + 0x52C, font);

                auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
                const int result = use_adapter
                    ? buffer_text_line_height_redirect(buffer, nullptr)
                    : buffer->text_line_height();
                FontDefaultPtr = saved_default_ptr;

                expect(result == test.expected);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

int pull_down_free_calls = 0;
void *pull_down_free_targets[128];

void *pull_down_free_probe(void *block) {
    if (pull_down_free_calls < 128) {
        pull_down_free_targets[pull_down_free_calls] = block;
    }
    ++pull_down_free_calls;
    return nullptr;
}

void test_pull_down_destructor() {
    alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    auto *self = reinterpret_cast<PullDown *>(storage + 16);

    // The seed pattern would make every text slot look allocated; zero all
    // 128 owned-string slots first, then plant sentinels in a shape that
    // distinguishes the two slots per item and proves the walk reaches the
    // final item: text only, right only, both, and both at index 63.
    const uint32_t zero = 0;
    for (size_t index = 0; index < 64; ++index) {
        write_at(storage, 16 + 0xA18 + index * 0x14, zero);
        write_at(storage, 16 + 0xA1C + index * 0x14, zero);
    }
    const uint32_t text_0 = 0x11110001U;
    const uint32_t right_1 = 0x22220002U;
    const uint32_t text_2 = 0x33330003U;
    const uint32_t right_2 = 0x44440004U;
    const uint32_t text_63 = 0x55550005U;
    const uint32_t right_63 = 0x66660006U;
    write_at(storage, 16 + 0xA18 + 0 * 0x14, text_0);
    write_at(storage, 16 + 0xA1C + 1 * 0x14, right_1);
    write_at(storage, 16 + 0xA18 + 2 * 0x14, text_2);
    write_at(storage, 16 + 0xA1C + 2 * 0x14, right_2);
    write_at(storage, 16 + 0xA18 + 63 * 0x14, text_63);
    write_at(storage, 16 + 0xA1C + 63 * 0x14, right_63);
    std::memcpy(expected, storage, sizeof(storage));

    // Final state: the GraphicWin delegation overwrites both vtable slots
    // with its own tables and clears 0xA10; the planted strings are freed
    // and nulled; dirty_ is set and the trailing pair reloads from the
    // rebindable defaults. Mnemonic slots keep their seeds untouched.
    write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
    write_at(expected, 16 + 0x444, GraphicWinBufferVtable);
    write_at(expected, 16 + 0xA10, zero);
    write_at(expected, 16 + 0xA18 + 0 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 1 * 0x14, zero);
    write_at(expected, 16 + 0xA18 + 2 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 2 * 0x14, zero);
    write_at(expected, 16 + 0xA18 + 63 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 63 * 0x14, zero);
    const uint8_t dirty = 1;
    write_at(expected, 16 + 0xF34, dirty);
    uint32_t default_f38 = 0xCAFE0001U;
    uint32_t default_f3c = 0xCAFE0002U;
    write_at(expected, 16 + 0xF38, default_f38);
    write_at(expected, 16 + 0xF3C, default_f3c);

    func_sprite_free *const saved_free = PullDownFree;
    uint32_t *const saved_f38 = PullDownFieldF38Default;
    uint32_t *const saved_f3c = PullDownFieldF3CDefault;
    func_subobject_destructor *const saved_buffer = BufferSubobjectDestructor;
    func_subobject_destructor *const saved_win = WinOriginalDestructor;
    PullDownFree = pull_down_free_probe;
    PullDownFieldF38Default = &default_f38;
    PullDownFieldF3CDefault = &default_f3c;
    BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
    WinOriginalDestructor = graphic_win_stub_win_destructor;
    pull_down_free_calls = 0;
    graphic_win_destructor_probe_reset();
    graphic_win_stub_record = GraphicWinStubRecord{};

    expect(pull_down_destructor_redirect(self, nullptr) == self);

    PullDownFree = saved_free;
    PullDownFieldF38Default = saved_f38;
    PullDownFieldF3CDefault = saved_f3c;
    BufferSubobjectDestructor = saved_buffer;
    WinOriginalDestructor = saved_win;

    expect_storage_bytes(storage, expected, sizeof(storage));
    // Six frees in walk order, text before right-hand text within an item.
    expect(pull_down_free_calls == 6);
    expect(pull_down_free_targets[0] == reinterpret_cast<void *>(text_0));
    expect(pull_down_free_targets[1] == reinterpret_cast<void *>(right_1));
    expect(pull_down_free_targets[2] == reinterpret_cast<void *>(text_2));
    expect(pull_down_free_targets[3] == reinterpret_cast<void *>(right_2));
    expect(pull_down_free_targets[4] == reinterpret_cast<void *>(text_63));
    expect(pull_down_free_targets[5] == reinterpret_cast<void *>(right_63));
    // The GraphicWin delegation actually ran, against this object.
    expect(graphic_win_stub_record.buffer_calls == 1);
    expect(graphic_win_stub_record.win_calls == 1);
    expect(graphic_win_stub_record.buffer_target
           == reinterpret_cast<void *>(storage + 16 + 0x444));
    expect(graphic_win_stub_record.win_target
           == reinterpret_cast<void *>(storage + 16));
    expect(graphic_win_stub_record.sequence == 0x21);
}

void test_base_button_default_setters() {
    // The three colour tiers interleave inside one table: slot s, tier t lives
    // at base + s * 0xC + t * 4. A local table stands in for the game's so the
    // test observes exact placement, and every one of the twelve cells is
    // compared after each call - a setter that strides wrongly, or writes a
    // sibling tier, moves a cell the caller never named.
    uint32_t table[12];
    uint32_t *const saved_table = BaseButtonDefaultTextColors;
    BaseButtonDefaultTextColors = table;

    struct TierCase {
        size_t tier;
        void (*member)(int, int, int, int);
        void (__cdecl *redirect)(int, int, int, int);
    };
    const TierCase tiers[] = {
        {0, &BaseButton::set_def_text_color,
         base_button_set_def_text_color_redirect},
        {1, &BaseButton::set_def_text_color2,
         base_button_set_def_text_color2_redirect},
        {2, &BaseButton::set_def_text_color3,
         base_button_set_def_text_color3_redirect},
    };
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    for (const TierCase &test : tiers) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            uint32_t expected[12];
            for (size_t index = 0; index < 12; ++index) {
                table[index] = 0xA5000000U ^ static_cast<uint32_t>(index);
                expected[index] = table[index];
            }
            for (size_t slot = 0; slot < 4; ++slot) {
                expected[(slot * 0xC + test.tier * 4) / 4] =
                    static_cast<uint32_t>(colors[slot]);
            }
            if (adapter) {
                test.redirect(colors[0], colors[1], colors[2], colors[3]);
            } else {
                test.member(colors[0], colors[1], colors[2], colors[3]);
            }
            for (size_t index = 0; index < 12; ++index) {
                expect(table[index] == expected[index]);
            }
        }
    }
    BaseButtonDefaultTextColors = saved_table;

    // set_def_font reads only Font::is_initialized(), an inline null check of
    // font_obj_ at offset 8, so raw storage stands in for constructed Fonts.
    alignas(Font) uint8_t ready_font[sizeof(Font)];
    alignas(Font) uint8_t unready_font[sizeof(Font)];
    std::memset(ready_font, 0xA5, sizeof(ready_font));
    std::memset(unready_font, 0xA5, sizeof(unready_font));
    const HFONT font_object = reinterpret_cast<HFONT>(0x1234U);
    const HFONT null_font_object = nullptr;
    write_at(ready_font, 0x08, font_object);
    write_at(unready_font, 0x08, null_font_object);
    auto *ready = reinterpret_cast<Font *>(ready_font);
    auto *unready = reinterpret_cast<Font *>(unready_font);
    auto *second = reinterpret_cast<Font *>(0x11111111U);
    auto *third = reinterpret_cast<Font *>(0x22222222U);

    Font *fonts[3];
    Font **const saved_fonts = BaseButtonDefaultFonts;
    BaseButtonDefaultFonts = fonts;

    // A null primary is rejected before anything is written.
    Font *const untouched = reinterpret_cast<Font *>(0x33333333U);
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(BaseButton::set_def_font(nullptr, second, third) == 3);
    for (int index = 0; index < 3; ++index) {
        expect(fonts[index] == untouched);
    }

    // An initialized primary publishes all three slots.
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(BaseButton::set_def_font(ready, second, third) == 0);
    expect(fonts[0] == ready);
    expect(fonts[1] == second);
    expect(fonts[2] == third);

    // An uninitialized primary leaves slot zero alone, still stores the other
    // two, and still reports success.
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(base_button_set_def_font_redirect(unready, second, third) == 0);
    expect(fonts[0] == untouched);
    expect(fonts[1] == second);
    expect(fonts[2] == third);

    BaseButtonDefaultFonts = saved_fonts;
}

// Stand-in DirectDraw surface and Buffer vtables for the device-context
// protocol. The surface's GetDC/ReleaseDC slots record their arguments and
// return a configurable status; the Buffer's slot 4 records that the error
// path fired.
struct HdcProbe {
    int get_calls;
    int release_calls;
    int error_calls;
    void *get_surface;
    void *release_surface;
    HDC released_handle;
    long get_status;
    long release_status;
    HDC produced_handle;
};
HdcProbe hdc_probe = {};

long __stdcall hdc_probe_get_dc(void *surface, void *out) {
    ++hdc_probe.get_calls;
    hdc_probe.get_surface = surface;
    std::memcpy(out, &hdc_probe.produced_handle, sizeof(HDC));
    return hdc_probe.get_status;
}

long __stdcall hdc_probe_release_dc(void *surface, void *handle) {
    ++hdc_probe.release_calls;
    hdc_probe.release_surface = surface;
    hdc_probe.released_handle = reinterpret_cast<HDC>(handle);
    return hdc_probe.release_status;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall hdc_probe_buffer_virtual(void *) {
    ++hdc_probe.error_calls;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void test_buffer_hdc_protocol() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_object = surface_vtable;
    void **surface = &surface_object;

    const HDC direct_handle = reinterpret_cast<HDC>(0x11110000U);
    const HDC surface_handle = reinterpret_cast<HDC>(0x22220000U);
    const uint32_t zero = 0;

    // Acquire with no surface: the stored handle is published, counted, and
    // returned without any surface traffic.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x60, direct_handle);
    write_at(expected, 16 + 0x68, 1U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == direct_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);
    expect(hdc_probe.error_calls == 0);

    // field_50_ set routes through the buffer's own slot 4 first.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, 1U);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x60, direct_handle);
    write_at(expected, 16 + 0x68, 1U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == direct_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.error_calls == 1);

    // Surface present and no cached handle: GetDC runs once, its out-parameter
    // becomes the cached handle, and a nonzero status trips the error path.
    for (int failing = 0; failing < 2; ++failing) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x00, buffer_vtable_ptr);
        write_at(storage, 16 + 0x50, zero);
        write_at(storage, 16 + 0x58, surface);
        write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
        write_at(storage, 16 + 0x64, direct_handle);
        write_at(storage, 16 + 0x68, zero);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16 + 0x60, surface_handle);
        write_at(expected, 16 + 0x68, 1U);
        hdc_probe = HdcProbe{};
        hdc_probe.produced_handle = surface_handle;
        hdc_probe.get_status = failing ? 1 : 0;
        expect(buffer->get_hdc() == surface_handle);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(hdc_probe.get_calls == 1);
        expect(hdc_probe.get_surface == surface);
        expect(hdc_probe.error_calls == (failing ? 1 : 0));
    }

    // A cached handle short-circuits: no GetDC, just another reference.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, surface_handle);
    write_at(storage, 16 + 0x68, 4U);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x68, 5U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == surface_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);

    // Release without a surface clears the handle only at or below zero.
    struct DirectCase { uint32_t start; int count; bool cleared; };
    const DirectCase direct_cases[] = {
        {5, 1, false}, {1, 1, true}, {3, 3, true}, {2, 5, true}, {9, 4, false},
        // remaining == 1 is the boundary: one reference is still out, so
        // the handle must survive. Without it, `< 1` and `<= 1` agree.
        {2, 1, false},
    };
    for (const DirectCase &test : direct_cases) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x00, buffer_vtable_ptr);
        write_at(storage, 16 + 0x58, zero);
        write_at(storage, 16 + 0x60, direct_handle);
        write_at(storage, 16 + 0x68, test.start);
        std::memcpy(expected, storage, sizeof(storage));
        if (test.cleared) {
            write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
            write_at(expected, 16 + 0x68, 0U);
        } else {
            write_at(expected, 16 + 0x68,
                     static_cast<uint32_t>(static_cast<int>(test.start)
                                           - test.count));
        }
        hdc_probe = HdcProbe{};
        buffer->release_hdc(test.count);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(hdc_probe.release_calls == 0);
    }

    // With a surface, the last reference calls ReleaseDC with the cached
    // handle; a surviving reference must not.
    struct SurfaceCase { uint32_t start; int count; bool released; };
    const SurfaceCase surface_cases[] = {
        {1, 1, true}, {6, 2, false}, {2, 7, true},
        {2, 1, false},   // same boundary on the surface path
    };
    for (const SurfaceCase &test : surface_cases) {
        for (int failing = 0; failing < 2; ++failing) {
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x00, buffer_vtable_ptr);
            write_at(storage, 16 + 0x58, surface);
            write_at(storage, 16 + 0x60, surface_handle);
            write_at(storage, 16 + 0x68, test.start);
            std::memcpy(expected, storage, sizeof(storage));
            if (test.released) {
                write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
                write_at(expected, 16 + 0x68, 0U);
            } else {
                write_at(expected, 16 + 0x68,
                         static_cast<uint32_t>(static_cast<int>(test.start)
                                               - test.count));
            }
            hdc_probe = HdcProbe{};
            hdc_probe.release_status = failing ? 1 : 0;
            buffer_release_hdc_redirect(buffer, nullptr, test.count);
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(hdc_probe.release_calls == (test.released ? 1 : 0));
            if (test.released) {
                expect(hdc_probe.release_surface == surface);
                expect(hdc_probe.released_handle == surface_handle);
                expect(hdc_probe.error_calls == (failing ? 1 : 0));
            }
        }
    }

    // A null cached handle with a surface releases nothing but still counts.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, 1U);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x68, 0U);
    hdc_probe = HdcProbe{};
    buffer->release_hdc(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.release_calls == 0);
}

void test_buffer_sync_to_palette() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    auto *palette = reinterpret_cast<Palette *>(palette_storage);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void *surface_object = surface_vtable;
    void **surface = &surface_object;

    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;
    const LPVOID *const no_pixel_storage = nullptr;
    const uint32_t zero = 0;
    const uint32_t tag = 0x1234ABCDU;

    int *const saved_initialized = PaletteInitialized;
    int initialized = 1;
    PaletteInitialized = &initialized;

    // No pixel storage: error 7 before anything at all is written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, no_pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->sync_to_palette(palette) == 7);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Null palette: error 3, still before any write, even with storage.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->sync_to_palette(nullptr) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Matching generation tag: the republish is skipped entirely - the colour
    // table is untouched - but the trailing pair is still published.
    std::memset(palette_storage, 0x5A, sizeof(palette_storage));
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x4A4, tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    hdc_probe = HdcProbe{};
    expect(buffer_sync_to_palette_redirect(buffer, nullptr, palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);

    // Differing tag with a surface that yields a null context: the tag is
    // cached, the table is republished from the palette, the context is
    // acquired and found null, so neither SetDIBColorTable nor the release
    // runs - but get_hdc still counted its reference.
    std::memset(palette_storage, 0, sizeof(palette_storage));
    for (uint32_t index = 0; index < 0x100; ++index) {
        // Palette entries are three bytes; get_rgbquad swaps red and blue
        // into the RGBQUAD it publishes.
        palette_storage[index * 4 + 0] = static_cast<uint8_t>(index);
        palette_storage[index * 4 + 1] = static_cast<uint8_t>(0xFF - index);
        palette_storage[index * 4 + 2] = static_cast<uint8_t>(index ^ 0x5AU);
    }
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x68, 1U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    for (uint32_t index = 0; index < 0x100; ++index) {
        const size_t cell = 16 + 0xA4 + index * 4;
        expected[cell + 2] = static_cast<uint8_t>(index);
        expected[cell + 1] = static_cast<uint8_t>(0xFF - index);
        expected[cell + 0] = static_cast<uint8_t>(index ^ 0x5AU);
        expected[cell + 3] = 0;
    }
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.release_calls == 0);

    // A non-null context reaches the publish branch. SetDIBColorTable is a
    // real GDI import given a synthetic handle, so it fails harmlessly and its
    // effect is unobservable - but the release that follows is not: the
    // reference get_hdc took is handed back through the surface's ReleaseDC
    // and the count returns to zero.
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    for (uint32_t index = 0; index < 0x100; ++index) {
        const size_t cell = 16 + 0xA4 + index * 4;
        expected[cell + 2] = static_cast<uint8_t>(index);
        expected[cell + 1] = static_cast<uint8_t>(0xFF - index);
        expected[cell + 0] = static_cast<uint8_t>(index ^ 0x5AU);
        expected[cell + 3] = 0;
    }
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = reinterpret_cast<HDC>(0x22220000U);
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.release_calls == 1);
    expect(hdc_probe.released_handle == reinterpret_cast<HDC>(0x22220000U));

    // An uninitialized palette makes get_rgbquad a no-op, so the table keeps
    // its seed while the tag and trailing pair are still published.
    initialized = 0;
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x68, 1U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    hdc_probe = HdcProbe{};
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    PaletteInitialized = saved_initialized;
}

void test_buffer_text_height() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    alignas(Font) uint8_t default_font[sizeof(Font)];
    alignas(Font) uint8_t cached_font[sizeof(Font)];
    std::memset(default_font, 0xA5, sizeof(default_font));
    std::memset(cached_font, 0xA5, sizeof(cached_font));
    const int default_height = 0x1234;
    const int cached_height = -99;
    write_at(default_font, 0x10, default_height);
    write_at(cached_font, 0x10, cached_height);
    auto *fallback = reinterpret_cast<Font *>(default_font);
    auto *cached = reinterpret_cast<Font *>(cached_font);

    Font **const saved_default = FontDefaultPtr;
    Font *default_slot = fallback;
    FontDefaultPtr = &default_slot;

    // No cached font: the process default is adopted and its height returned.
    for (int adapter = 0; adapter < 2; ++adapter) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x52C, static_cast<Font *>(nullptr));
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16 + 0x52C, fallback);
        const int result = adapter
            ? buffer_text_height_redirect(buffer, nullptr)
            : buffer->text_height();
        expect(result == default_height);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // A cached font is used as-is and must not be replaced, even though the
    // default is loaded first.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x52C, cached);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->text_height() == cached_height);
    expect_storage_bytes(storage, expected, sizeof(storage));

    FontDefaultPtr = saved_default;
}

void test_win_is_dialog_focus() {
    alignas(Win) uint8_t storage[sizeof(Win) + 32];
    alignas(Win) uint8_t parent_storage[sizeof(Win) + 32];
    uint8_t expected[sizeof(storage)];
    uint8_t parent_expected[sizeof(parent_storage)];
    auto *self = reinterpret_cast<Win *>(storage + 16);
    auto *parent = reinterpret_cast<Win *>(parent_storage + 16);

    // The focus list's entry at +4 names the focused window.
    uintptr_t focus_list[2] = {};
    // Bind an explicit pointer: passing the array to write_at deduces
    // uintptr_t[2] under `const T&` and copies both elements.
    uintptr_t *const focus_list_ptr = focus_list;
    const uint32_t zero = 0;
    const uint32_t nonzero = 1;

    struct FocusCase {
        uint32_t flags;
        bool has_parent;
        uint32_t list_count;
        int focus_target;   // 0 none, 1 self, 2 parent
        int expected;
    };
    const FocusCase cases[] = {
        // Bit 12 short-circuits before the parent is even consulted.
        {0x00001000U, false, 0, 0, 1},
        {0xFFFFFFFFU, false, 0, 0, 1},
        // Without the bit and without a parent there is no focus.
        {0x00000000U, false, 0, 0, 0},
        {0xFFFFEFFFU, false, 0, 0, 0},
        // An empty focus list reads as no focus and never dereferences it.
        {0x00000000U, true, 0, 1, 0},
        // A populated list naming this window grants focus.
        {0x00000000U, true, 1, 1, 1},
        // Naming a different window does not.
        {0x00000000U, true, 1, 2, 0},
        {0x00000000U, true, 1, 0, 0},
    };
    for (const FocusCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            seed_storage(parent_storage, parent_expected,
                         sizeof(parent_storage));
            write_at(storage, 16 + 0x98, test.flags);
            write_at(storage, 16 + 0xC4,
                     test.has_parent ? parent : static_cast<Win *>(nullptr));
            focus_list[0] = 0;
            focus_list[1] = test.focus_target == 1
                ? reinterpret_cast<uintptr_t>(self)
                : (test.focus_target == 2
                       ? reinterpret_cast<uintptr_t>(parent) : 0U);
            write_at(parent_storage, 16 + 0xCC,
                     test.list_count ? nonzero : zero);
            write_at(parent_storage, 16 + 0xD0, focus_list_ptr);
            std::memcpy(expected, storage, sizeof(storage));
            std::memcpy(parent_expected, parent_storage,
                        sizeof(parent_storage));
            const int result = adapter
                ? win_is_dialog_focus_redirect(self, nullptr)
                : self->is_dialog_focus();
            expect(result == test.expected);
            // The query must not modify either window.
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect_storage_bytes(parent_storage, parent_expected,
                                 sizeof(parent_storage));
        }
    }
}

struct ClipProbe {
    int set_list_calls;
    int set_clipper_calls;
    void *set_list_this;
    void *set_clipper_this;
    void *set_clipper_arg;
    unsigned long set_list_flags;
    RGNDATAHEADER header;
    RECT rects[1];
};
ClipProbe clip_probe = {};

long __stdcall clip_probe_set_list(void *self, void *data, unsigned long flags) {
    ++clip_probe.set_list_calls;
    clip_probe.set_list_this = self;
    clip_probe.set_list_flags = flags;
    std::memcpy(&clip_probe.header, data, sizeof(RGNDATAHEADER));
    std::memcpy(clip_probe.rects,
                static_cast<const uint8_t *>(data) + sizeof(RGNDATAHEADER),
                sizeof(RECT));
    return 0;
}

long __stdcall clip_probe_set_clipper(void *self, void *clipper) {
    ++clip_probe.set_clipper_calls;
    clip_probe.set_clipper_this = self;
    clip_probe.set_clipper_arg = clipper;
    return 0;
}

void test_buffer_set_clip() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_vtable[0x74 / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    surface_vtable[0x70 / sizeof(void *)] =
        reinterpret_cast<void *>(&clip_probe_set_clipper);
    void *surface_object = surface_vtable;
    void **surface = &surface_object;
    void *clipper_vtable[0x20 / sizeof(void *)] = {};
    clipper_vtable[0x1C / sizeof(void *)] =
        reinterpret_cast<void *>(&clip_probe_set_list);
    void *clipper_object = clipper_vtable;
    void **clipper = &clipper_object;

    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;
    const LPVOID *const no_pixels = nullptr;
    const uint32_t zero = 0;
    const RECT extent = {0, 0, 100, 50};

    // Neither pixel storage nor a surface: error 7, nothing written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, no_pixels);
    write_at(storage, 16 + 0x58, zero);
    std::memcpy(expected, storage, sizeof(storage));
    RECT request = {10, 10, 20, 20};
    expect(buffer->set_clip(&request) == 7);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Null rectangle: error 3, still nothing written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->set_clip(nullptr) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A request disjoint from the extent yields an empty intersection: error 1
    // and, per IntersectRect, a zeroed destination rectangle.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    std::memcpy(expected, storage, sizeof(storage));
    const RECT empty_rect = {0, 0, 0, 0};
    write_at(expected, 16 + 0x20, empty_rect);
    RECT disjoint = {500, 500, 600, 600};
    expect(buffer->set_clip(&disjoint) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A partial overlap clips to the intersection. With no surface the buffer
    // owns its context directly, so get_hdc adopts it and release_hdc gives it
    // back, leaving the count at zero.
    const HDC direct_handle = reinterpret_cast<HDC>(0x33330000U);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, static_cast<HRGN>(nullptr));
    std::memcpy(expected, storage, sizeof(storage));
    const RECT clipped = {10, 10, 20, 20};
    write_at(expected, 16 + 0x20, clipped);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    RECT overlap = {10, 10, 20, 20};
    expect(buffer->set_clip(&overlap) == 0);
    // field_70_ now holds a real region handle, so compare around it.
    HRGN produced_region;
    std::memcpy(&produced_region, storage + 16 + 0x70, sizeof(produced_region));
    expect(produced_region != nullptr);
    write_at(expected, 16 + 0x70, produced_region);
    expect_storage_bytes(storage, expected, sizeof(storage));
    // No surface means no clipper traffic at all.
    expect(clip_probe.set_list_calls == 0);
    expect(clip_probe.set_clipper_calls == 0);
    DeleteObject(produced_region);

    // A clip equal to the full extent needs no region: the previously held
    // region is deleted and the slot cleared, and no replacement is created.
    // Every earlier case starts with an empty slot, so this is the only one
    // that exercises the cleanup branch.
    const RECT prior = {1, 2, 3, 4};
    HRGN existing = CreateRectRgnIndirect(&prior);
    expect(existing != nullptr);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, existing);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x20, extent);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    write_at(expected, 16 + 0x70, static_cast<HRGN>(nullptr));
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    RECT full = {0, 0, 100, 50};
    expect(buffer->set_clip(&full) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // With a surface, the clipper receives a single-rectangle RGNDATA whose
    // bound and only entry are both the clipped rectangle, and the surface is
    // then handed that clipper.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x5C, clipper);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, static_cast<HRGN>(nullptr));
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;   // GetDC yields nothing: skip the GDI work
    RECT overlap2 = {5, 5, 40, 30};
    expect(buffer->set_clip(&overlap2) == 0);
    RECT stored;
    std::memcpy(&stored, storage + 16 + 0x20, sizeof(stored));
    expect(stored.left == 5 && stored.top == 5);
    expect(stored.right == 40 && stored.bottom == 30);
    expect(clip_probe.set_list_calls == 1);
    expect(clip_probe.set_list_this == clipper);
    expect(clip_probe.set_list_flags == 0);
    expect(clip_probe.header.dwSize == sizeof(RGNDATAHEADER));
    expect(clip_probe.header.iType == RDH_RECTANGLES);
    expect(clip_probe.header.nCount == 1);
    expect(clip_probe.header.nRgnSize == sizeof(RECT));
    expect(clip_probe.header.rcBound.left == 5);
    expect(clip_probe.header.rcBound.top == 5);
    expect(clip_probe.header.rcBound.right == 40);
    expect(clip_probe.header.rcBound.bottom == 30);
    expect(clip_probe.rects[0].left == 5);
    expect(clip_probe.rects[0].top == 5);
    expect(clip_probe.rects[0].right == 40);
    expect(clip_probe.rects[0].bottom == 30);
    expect(clip_probe.set_clipper_calls == 1);
    expect(clip_probe.set_clipper_this == surface);
    expect(clip_probe.set_clipper_arg == clipper);
}

void test_base_button_text_colors() {
    // The buffer member sits at +0x444 inside the GraphicWin base. Buffer
    // carries four interleaved colour tiers - the three these setters reach
    // plus the hyper tier - so slot s of tier t is at 0x53C + s * 0x10 + t * 4,
    // a 0x10 stride rather than the 0xC of a three-tier table.
    std::vector<uint8_t> button_bytes(sizeof(BaseButton) + 32);
    std::vector<uint8_t> expected_bytes(sizeof(BaseButton) + 32);
    uint8_t *const storage = button_bytes.data();
    uint8_t *const expected = expected_bytes.data();
    auto *button = reinterpret_cast<BaseButton *>(storage + 16);

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    std::memset(palette_storage, 0, sizeof(palette_storage));
    auto *palette = reinterpret_cast<Palette *>(palette_storage);
    Palette *palette_slot = palette;
    Palette **const saved_palette = BaseButtonActivePalette;
    BaseButtonActivePalette = &palette_slot;
    int *const saved_initialized = PaletteInitialized;
    int initialized = 0;   // get_rgbquad becomes a no-op
    PaletteInitialized = &initialized;

    constexpr size_t kBuffer = 0x444;
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    const uint32_t zero = 0;
    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;

    struct TierCase {
        size_t tier;
        void (BaseButton::*member)(int, int, int, int);
        void (__fastcall *redirect)(BaseButton *, void *, int, int, int, int);
    };
    const TierCase tiers[] = {
        {0, &BaseButton::set_text_color, base_button_set_text_color_redirect},
        {1, &BaseButton::set_text_color2, base_button_set_text_color2_redirect},
        {2, &BaseButton::set_text_color3, base_button_set_text_color3_redirect},
    };

    for (const TierCase &test : tiers) {
        // No parent: the whole body is skipped, so not even the palette is
        // published and the object is untouched.
        seed_storage(storage, expected, button_bytes.size());
        write_at(storage, 16 + 0xC4, static_cast<Win *>(nullptr));
        std::memcpy(expected, storage, button_bytes.size());
        (button->*test.member)(colors[0], colors[1], colors[2], colors[3]);
        expect_storage_bytes(storage, expected, button_bytes.size());

        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, button_bytes.size());
            // A parent enables the body; the buffer needs pixel storage so
            // sync_to_palette gets past its own guard.
            write_at(storage, 16 + 0xC4, button);
            write_at(storage, 16 + kBuffer + 0x54, pixel_storage);
            write_at(storage, 16 + kBuffer + 0x58, zero);
            write_at(storage, 16 + kBuffer + 0x4A4, zero);
            std::memcpy(expected, storage, button_bytes.size());
            // sync_to_palette publishes the palette and its generation tag,
            // then the tier's four colours land at stride 0xC.
            write_at(expected, 16 + kBuffer + 0x4A4, zero);
            write_at(expected, 16 + kBuffer + 0x57C, 1U);
            write_at(expected, 16 + kBuffer + 0x584, palette);
            for (size_t slot = 0; slot < 4; ++slot) {
                write_at(expected,
                         16 + kBuffer + 0x53C + slot * 0x10 + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(button, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (button->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, button_bytes.size());
        }
    }

    PaletteInitialized = saved_initialized;
    BaseButtonActivePalette = saved_palette;
}

void test_default_font_setters() {
    // Every default-font setter in the codebase shares one shape: reject a
    // null primary with 3, publish the primary only when it is initialized,
    // store the remaining slots unconditionally, and return 0 either way.
    alignas(Font) uint8_t ready_font[sizeof(Font)];
    alignas(Font) uint8_t unready_font[sizeof(Font)];
    std::memset(ready_font, 0xA5, sizeof(ready_font));
    std::memset(unready_font, 0xA5, sizeof(unready_font));
    const HFONT font_object = reinterpret_cast<HFONT>(0x1234U);
    const HFONT null_font_object = nullptr;
    write_at(ready_font, 0x08, font_object);
    write_at(unready_font, 0x08, null_font_object);
    auto *ready = reinterpret_cast<Font *>(ready_font);
    auto *unready = reinterpret_cast<Font *>(unready_font);
    auto *second = reinterpret_cast<Font *>(0x11111111U);
    auto *third = reinterpret_cast<Font *>(0x22222222U);
    auto *fourth = reinterpret_cast<Font *>(0x44444444U);
    Font *const untouched = reinterpret_cast<Font *>(0x33333333U);

    Font *slots[4];
    auto reset = [&slots]() {
        for (int index = 0; index < 4; ++index) {
            slots[index] = reinterpret_cast<Font *>(0x33333333U);
        }
    };

    // The four-slot string setter.
    Font **const saved_string = BasePopDefaultStringFonts;
    BasePopDefaultStringFonts = slots;
    reset();
    expect(BasePop::set_def_string_font(nullptr, second, third, fourth) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(BasePop::set_def_string_font(ready, second, third, fourth) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == fourth);
    reset();
    expect(base_pop_set_def_string_font_redirect(
               unready, second, third, fourth) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == fourth);
    BasePopDefaultStringFonts = saved_string;

    // The three-slot button setter must not touch a fourth slot.
    Font **const saved_button = BasePopDefaultButtonFonts;
    BasePopDefaultButtonFonts = slots;
    reset();
    expect(BasePop::set_def_button_font(nullptr, second, third) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(BasePop::set_def_button_font(ready, second, third) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    reset();
    expect(base_pop_set_def_button_font_redirect(unready, second, third) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    BasePopDefaultButtonFonts = saved_button;

    // The dialog setter, which writes its own separate table.
    Font **const saved_dialog = DialogDefaultFonts;
    DialogDefaultFonts = slots;
    reset();
    expect(Dialog::set_def_dialog_font(nullptr, second, third) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(Dialog::set_def_dialog_font(ready, second, third) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    reset();
    expect(dialog_set_def_dialog_font_redirect(unready, second, third) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    DialogDefaultFonts = saved_dialog;
}

void test_win_shared_hdc() {
    // The shared context lives in three process globals rather than on an
    // instance, so the fixture rebinds all of them plus the window handle.
    int refcount = 0;
    HDC shared = nullptr;
    void *surface_object = nullptr;
    HWND window = nullptr;

    int *const saved_count = WinHdcRefCount;
    HDC *const saved_shared = WinSharedHdc;
    void **const saved_surface = WinHdcSurface;
    HWND *const saved_window = WinHdcWindow;
    WinHdcRefCount = &refcount;
    WinSharedHdc = &shared;
    WinHdcSurface = &surface_object;
    WinHdcWindow = &window;

    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void *surface_vtable_object = surface_vtable;
    void **surface = &surface_vtable_object;
    const HDC surface_handle = reinterpret_cast<HDC>(0x77770000U);

    // First acquire through a surface: GetDC runs, the handle is published,
    // and the count starts at one.
    surface_object = surface;
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = surface_handle;
    expect(Win::get_hdc() == surface_handle);
    expect(refcount == 1);
    expect(shared == surface_handle);
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.get_surface == surface);

    // A held context is counted again without touching the surface.
    hdc_probe = HdcProbe{};
    expect(win_get_hdc_redirect() == surface_handle);
    expect(refcount == 2);
    expect(hdc_probe.get_calls == 0);

    // Releasing back to one reference must not release the handle.
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == 1);
    expect(shared == surface_handle);
    expect(hdc_probe.release_calls == 0);

    // The last reference releases through the surface and clears the handle.
    hdc_probe = HdcProbe{};
    win_release_hdc_redirect();
    expect(refcount == 0);
    expect(shared == nullptr);
    expect(hdc_probe.release_calls == 1);
    expect(hdc_probe.release_surface == surface);
    expect(hdc_probe.released_handle == surface_handle);

    // A surface that yields nothing leaves the count at zero, so the next
    // call retries the acquire rather than handing back a null handle as
    // though it were held.
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;
    expect(Win::get_hdc() == nullptr);
    expect(refcount == 0);
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = surface_handle;
    expect(Win::get_hdc() == surface_handle);
    expect(refcount == 1);
    expect(hdc_probe.get_calls == 1);
    // Leave the count balanced.
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == 0);

    // Over-releasing drives the count negative and, because the legacy body
    // tests for exactly zero, performs no release at all.
    refcount = 0;
    shared = surface_handle;
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == -1);
    expect(shared == surface_handle);
    expect(hdc_probe.release_calls == 0);

    // Without a surface the context comes from the window. GetDC(nullptr)
    // yields a real screen context under Wine, which the release returns.
    surface_object = nullptr;
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    const HDC window_context = Win::get_hdc();
    expect(hdc_probe.get_calls == 0);
    // Asserted unconditionally: GetDC(nullptr) yields the screen context, so
    // a conditional check here would simply follow a mutant that dropped the
    // acquire into its own else branch and pass.
    expect(window_context != nullptr);
    expect(refcount == 1);
    expect(shared == window_context);
    Win::release_hdc();
    expect(refcount == 0);
    expect(shared == nullptr);

    WinHdcWindow = saved_window;
    WinHdcSurface = saved_surface;
    WinSharedHdc = saved_shared;
    WinHdcRefCount = saved_count;
}

struct SpriteDrawProbe {
    int calls;
    Sprite *self;
    Buffer *buffer;
    int a, b, c;
    int origin_x_during;
    int origin_y_during;
    int result;
};
SpriteDrawProbe sprite_draw_probe = {};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
int __thiscall sprite_draw_probe_body(
        Sprite *self, Buffer *buffer, int a, int b, int c) {
    ++sprite_draw_probe.calls;
    sprite_draw_probe.self = self;
    sprite_draw_probe.buffer = buffer;
    sprite_draw_probe.a = a;
    sprite_draw_probe.b = b;
    sprite_draw_probe.c = c;
    // The substituted origin must be visible from inside the call.
    sprite_draw_probe.origin_x_during = *SpriteDrawOriginX;
    sprite_draw_probe.origin_y_during = *SpriteDrawOriginY;
    return sprite_draw_probe.result;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void test_sprite_draw_origin() {
    alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
    uint8_t expected[sizeof(storage)];
    auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
    auto *buffer = reinterpret_cast<Buffer *>(0x12340000U);

    int origin_x = 0;
    int origin_y = 0;
    int *const saved_x_ptr = SpriteDrawOriginX;
    int *const saved_y_ptr = SpriteDrawOriginY;
    func_sprite_draw_original *const saved_body = SpriteDrawOriginal;
    SpriteDrawOriginX = &origin_x;
    SpriteDrawOriginY = &origin_y;
    SpriteDrawOriginal = &sprite_draw_probe_body;

    struct DrawCase { int prior_x, prior_y, x, y, result; };
    const DrawCase cases[] = {
        {0, 0, 5, 7, 0},
        {111, 222, -1, -2, 42},
        {INT_MIN, INT_MAX, INT_MAX, INT_MIN, -7},
        {9, 9, 9, 9, 1},
    };
    for (const DrawCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            origin_x = test.prior_x;
            origin_y = test.prior_y;
            sprite_draw_probe = SpriteDrawProbe{};
            sprite_draw_probe.result = test.result;
            const int returned = adapter
                ? sprite_draw_redirect(sprite, nullptr, buffer,
                                       1, 2, 3, test.x, test.y)
                : sprite->draw(buffer, 1, 2, 3, test.x, test.y);
            // The overload sees the substituted origin and this sprite.
            expect(sprite_draw_probe.calls == 1);
            expect(sprite_draw_probe.self == sprite);
            expect(sprite_draw_probe.buffer == buffer);
            expect(sprite_draw_probe.a == 1);
            expect(sprite_draw_probe.b == 2);
            expect(sprite_draw_probe.c == 3);
            expect(sprite_draw_probe.origin_x_during == test.x);
            expect(sprite_draw_probe.origin_y_during == test.y);
            // The origin is restored exactly, and the result passes through.
            expect(origin_x == test.prior_x);
            expect(origin_y == test.prior_y);
            expect(returned == test.result);
            // The sprite itself is never touched.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    SpriteDrawOriginal = saved_body;
    SpriteDrawOriginY = saved_y_ptr;
    SpriteDrawOriginX = saved_x_ptr;
}

struct WrapperProbe {
    int cursor_calls;
    Win *cursor_window;
    int cursor_flag;
    int width_calls;
    Buffer *width_self;
    LPSTR width_text;
    size_t width_length;
    int width_result;
};
WrapperProbe wrapper_probe = {};

int __cdecl wrapper_probe_update_cursor(Win *window, int flag) {
    ++wrapper_probe.cursor_calls;
    wrapper_probe.cursor_window = window;
    wrapper_probe.cursor_flag = flag;
    return 0;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
int __thiscall wrapper_probe_text_width(Buffer *self, LPSTR text, size_t length) {
    ++wrapper_probe.width_calls;
    wrapper_probe.width_self = self;
    wrapper_probe.width_text = text;
    wrapper_probe.width_length = length;
    return wrapper_probe.width_result;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void test_win_set_cursor() {
    alignas(Win) uint8_t storage[sizeof(Win) + 32];
    uint8_t expected[sizeof(storage)];
    auto *win = reinterpret_cast<Win *>(storage + 16);

    func_win_update_cursor *const saved = WinUpdateCursorOriginal;
    WinUpdateCursorOriginal = &wrapper_probe_update_cursor;

    auto *const sentinel_sprite = reinterpret_cast<Sprite *>(0x11110000U);
    auto *const sentinel_handle = reinterpret_cast<HCURSOR *>(0x22220000U);

    // The accepted range is 0x7F00 through 0x7F8A inclusive; the boundaries
    // on both sides are the only values that distinguish the comparisons.
    struct CursorCase { int name; int expected; };
    const CursorCase cases[] = {
        {0x7EFF, 3},   // one below the range
        {0x7F00, 0},   // first accepted
        {0x7F45, 0},
        {0x7F8A, 0},   // last accepted
        {0x7F8B, 3},   // one above the range
        {0, 3}, {-1, 3}, {INT_MAX, 3}, {INT_MIN, 3},
    };
    for (const CursorCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x188, sentinel_sprite);
            write_at(storage, 16 + 0x194, sentinel_handle);
            std::memcpy(expected, storage, sizeof(storage));
            if (test.expected == 0) {
                write_at(expected, 16 + 0x188, static_cast<Sprite *>(nullptr));
                write_at(expected, 16 + 0x198, test.name);
                write_at(expected, 16 + 0x194, static_cast<HCURSOR *>(nullptr));
            }
            wrapper_probe = WrapperProbe{};
            const int result = adapter
                ? win_set_cursor_redirect(win, nullptr, test.name)
                : win->set_cursor(test.name);
            expect(result == test.expected);
            expect_storage_bytes(storage, expected, sizeof(storage));
            // A rejected name must not trigger the refresh either.
            expect(wrapper_probe.cursor_calls == (test.expected == 0 ? 1 : 0));
            if (test.expected == 0) {
                expect(wrapper_probe.cursor_window == nullptr);
                expect(wrapper_probe.cursor_flag == 1);
            }
        }
    }

    WinUpdateCursorOriginal = saved;
}

void test_buffer_text_width() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    func_buffer_text_width_measured *const saved = BufferTextWidthMeasured;
    BufferTextWidthMeasured = &wrapper_probe_text_width;

    // A null string is rejected without measuring anything.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    wrapper_probe = WrapperProbe{};
    expect(buffer->text_width(nullptr) == 0);
    expect(wrapper_probe.width_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Otherwise the string and its measured length are forwarded and the
    // result passes through. An empty string still measures.
    char empty[] = "";
    char text[] = "hello world";
    struct WidthCase { LPSTR text; size_t length; int result; };
    const WidthCase cases[] = {
        {empty, 0, 0},
        {text, 11, 4242},
        {text, 11, -1},
    };
    for (const WidthCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            wrapper_probe = WrapperProbe{};
            wrapper_probe.width_result = test.result;
            const int result = adapter
                ? buffer_text_width_redirect(buffer, nullptr, test.text)
                : buffer->text_width(test.text);
            expect(result == test.result);
            expect(wrapper_probe.width_calls == 1);
            expect(wrapper_probe.width_self == buffer);
            expect(wrapper_probe.width_text == test.text);
            expect(wrapper_probe.width_length == test.length);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    BufferTextWidthMeasured = saved;
}

struct TeardownProbe {
    int net_close_calls;
    void *net_close_self;
    int update_calls;
    RECT *update_rect;
    Win *update_window;
    int flip_calls;
    RECT *flip_rect;
    int active_during_update;
    int companion_during_update;
    int sequence;
};
TeardownProbe teardown_probe = {};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall teardown_probe_net_close(void *self) {
    ++teardown_probe.net_close_calls;
    teardown_probe.net_close_self = self;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

int __cdecl teardown_probe_update_screen(RECT *rect, Win *window) {
    ++teardown_probe.update_calls;
    teardown_probe.update_rect = rect;
    teardown_probe.update_window = window;
    // Both flags must already be cleared by the time the refresh runs.
    teardown_probe.active_during_update = *WinBubbleActive;
    teardown_probe.companion_during_update = *WinBubbleCompanion;
    teardown_probe.sequence = (teardown_probe.sequence << 4) | 1;
    return 0;
}

void __cdecl teardown_probe_flip(RECT *rect) {
    ++teardown_probe.flip_calls;
    teardown_probe.flip_rect = rect;
    teardown_probe.sequence = (teardown_probe.sequence << 4) | 2;
}

void test_alpha_net_close() {
    alignas(AlphaNet) uint8_t storage[sizeof(AlphaNet) + 32];
    uint8_t expected[sizeof(storage)];
    auto *network = reinterpret_cast<AlphaNet *>(storage + 16);

    func_net_close *const saved = NetCloseOriginal;
    NetCloseOriginal = &teardown_probe_net_close;

    for (int adapter = 0; adapter < 2; ++adapter) {
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(expected, storage, sizeof(storage));
        const uint32_t zero = 0;
        // Eight process slots at 0x78C, stride 0x19C, plus the slot at 0x768.
        for (size_t slot = 0; slot < 8; ++slot) {
            write_at(expected, 16 + 0x78C + slot * 0x19C, zero);
        }
        write_at(expected, 16 + 0x768, zero);
        teardown_probe = TeardownProbe{};
        if (adapter) {
            alpha_net_close_redirect(network, nullptr);
        } else {
            network->close();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
        // The tail jump hands the same object to the network close.
        expect(teardown_probe.net_close_calls == 1);
        expect(teardown_probe.net_close_self == network);
    }

    NetCloseOriginal = saved;
}

void test_win_clear_bubble_text() {
    int active = 0;
    int companion = 0;
    RECT rect = {1, 2, 3, 4};
    int *const saved_active = WinBubbleActive;
    int *const saved_companion = WinBubbleCompanion;
    RECT *const saved_rect = WinBubbleRect;
    func_win_update_screen *const saved_update = WinUpdateScreenOriginal;
    func_win_flip *const saved_flip = WinFlipOriginal;
    WinBubbleActive = &active;
    WinBubbleCompanion = &companion;
    WinBubbleRect = &rect;
    WinUpdateScreenOriginal = &teardown_probe_update_screen;
    WinFlipOriginal = &teardown_probe_flip;

    // No bubble pending: nothing is cleared and neither refresh runs, even
    // with a stale companion value sitting there.
    active = 0;
    companion = 0x5A5A5A5A;
    teardown_probe = TeardownProbe{};
    Win::clear_bubble_text();
    expect(active == 0);
    expect(companion == 0x5A5A5A5A);
    expect(teardown_probe.update_calls == 0);
    expect(teardown_probe.flip_calls == 0);

    // A pending bubble clears both slots, then refreshes and flips the same
    // rectangle. The flags must already be clear when the refresh observes
    // them, which is why the probe samples them.
    for (int adapter = 0; adapter < 2; ++adapter) {
        active = 1;
        companion = 0x5A5A5A5A;
        teardown_probe = TeardownProbe{};
        if (adapter) {
            win_clear_bubble_text_redirect();
        } else {
            Win::clear_bubble_text();
        }
        expect(active == 0);
        expect(companion == 0);
        expect(teardown_probe.update_calls == 1);
        expect(teardown_probe.update_rect == &rect);
        expect(teardown_probe.update_window == nullptr);
        expect(teardown_probe.active_during_update == 0);
        expect(teardown_probe.companion_during_update == 0);
        expect(teardown_probe.flip_calls == 1);
        expect(teardown_probe.flip_rect == &rect);
        // The refresh must precede the flip, not merely both happen.
        expect(teardown_probe.sequence == 0x12);
    }

    // Any nonzero pending value counts, not just one.
    active = -1;
    teardown_probe = TeardownProbe{};
    Win::clear_bubble_text();
    expect(active == 0);
    expect(teardown_probe.update_calls == 1);

    WinFlipOriginal = saved_flip;
    WinUpdateScreenOriginal = saved_update;
    WinBubbleRect = saved_rect;
    WinBubbleCompanion = saved_companion;
    WinBubbleActive = saved_active;
}

void test_constant_return_stubs() {
    // Fifteen legacy stubs whose entire body sets a constant and returns. The
    // shape classifier proposed them; each was confirmed against its own
    // instruction bytes, including that the `ret N` cleanup matches the
    // arity its mangled name implies. Win::on_redraw was rejected from this
    // batch on exactly that check - its name declares no parameters while its
    // body cleans eight bytes.
    //
    // The object must come back untouched: these read nothing and write
    // nothing, so a full byte comparison is the whole specification.
    alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *win = reinterpret_cast<Win *>(win_storage + 16);
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    expect(win->UNK1(1, 2, 3, 4, 5, 6, 7, 8, 9) == 0);
    expect(win->UNK5() == 0);
    expect(win->UNK6(INT_MIN) == 0);
    expect(win->on_set_cursor(nullptr, 0U, 0U) == 1);
    expect(win_unk1_redirect(win, nullptr, -1, -2, -3, -4, -5, -6, -7, -8, -9) == 0);
    expect(win_unk5_redirect(win, nullptr) == 0);
    expect(win_unk6_redirect(win, nullptr, INT_MAX) == 0);
    expect(win_on_set_cursor_redirect(
               win, nullptr, win_storage, 0xFFFFFFFFU, 0xFFFFFFFFU) == 1);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    alignas(Sprite) uint8_t sprite_storage[sizeof(Sprite) + 32];
    uint8_t sprite_expected[sizeof(sprite_storage)];
    auto *sprite = reinterpret_cast<Sprite *>(sprite_storage + 16);
    seed_storage(sprite_storage, sprite_expected, sizeof(sprite_storage));
    std::memcpy(sprite_expected, sprite_storage, sizeof(sprite_storage));
    expect(sprite->UNK1(1, 2, 3, 4, 5, 6, 7) == 0);
    expect(sprite->UNK2(1, 2, 3, 4, 5) == 0);
    expect(sprite_unk1_redirect(sprite, nullptr, -1, -2, -3, -4, -5, -6, -7) == 0);
    expect(sprite_unk2_redirect(sprite, nullptr, -1, -2, -3, -4, -5) == 0);
    expect_storage_bytes(sprite_storage, sprite_expected, sizeof(sprite_storage));

    alignas(PullDown) uint8_t pd_storage[sizeof(PullDown) + 32];
    uint8_t pd_expected[sizeof(pd_storage)];
    auto *pull_down = reinterpret_cast<PullDown *>(pd_storage + 16);
    seed_storage(pd_storage, pd_expected, sizeof(pd_storage));
    std::memcpy(pd_expected, pd_storage, sizeof(pd_storage));
    // These two return 8 rather than zero, which is the distinction a
    // constant-return stub most easily gets wrong.
    expect(pull_down->UNK2(0) == 8);
    expect(pull_down->UNK3(0, 0, 0) == 8);
    expect(pull_down->UNK5() == 1);
    expect(pull_down->UNK6() == 1);
    expect(pull_down_unk2_redirect(pull_down, nullptr, INT_MIN) == 8);
    expect(pull_down_unk3_redirect(pull_down, nullptr, 1, 2, 3) == 8);
    expect(pull_down_unk5_redirect(pull_down, nullptr) == 1);
    expect(pull_down_unk6_redirect(pull_down, nullptr) == 1);
    expect_storage_bytes(pd_storage, pd_expected, sizeof(pd_storage));

    alignas(Menu) uint8_t menu_storage[sizeof(Menu) + 32];
    uint8_t menu_expected[sizeof(menu_storage)];
    auto *menu = reinterpret_cast<Menu *>(menu_storage + 16);
    seed_storage(menu_storage, menu_expected, sizeof(menu_storage));
    std::memcpy(menu_expected, menu_storage, sizeof(menu_storage));
    expect(menu->UNK2(0) == 0);
    expect(menu->UNK4(0, 0, 0) == 0);
    expect(menu_unk2_redirect(menu, nullptr, INT_MAX) == 0);
    expect(menu_unk4_redirect(menu, nullptr, 1, 2, 3) == 0);

    alignas(Font) uint8_t font_storage[sizeof(Font) + 32];
    uint8_t font_expected[sizeof(font_storage)];
    auto *font = reinterpret_cast<Font *>(font_storage + 16);
    seed_storage(font_storage, font_expected, sizeof(font_storage));
    std::memcpy(font_expected, font_storage, sizeof(font_storage));
    expect(font->UNK1(1, 2, 3, 4) == 1);
    expect(font_unk1_redirect(font, nullptr, -1, -2, -3, -4) == 1);
    expect_storage_bytes(font_storage, font_expected, sizeof(font_storage));
    expect_storage_bytes(menu_storage, menu_expected, sizeof(menu_storage));

    alignas(BaseButton) uint8_t bb_storage[sizeof(BaseButton) + 32];
    uint8_t bb_expected[sizeof(bb_storage)];
    auto *button = reinterpret_cast<BaseButton *>(bb_storage + 16);
    seed_storage(bb_storage, bb_expected, sizeof(bb_storage));
    std::memcpy(bb_expected, bb_storage, sizeof(bb_storage));
    button->on_key_click(1, 2);
    button->on_key_down(3);
    button->on_key_up(4);
    base_button_on_key_click_redirect(button, nullptr, -1, -2);
    base_button_on_key_down_redirect(button, nullptr, -3);
    base_button_on_key_up_redirect(button, nullptr, -4);
    expect_storage_bytes(bb_storage, bb_expected, sizeof(bb_storage));

    // MainInterface is far too large to seed on the stack, so its canary
    // lives on the heap. These three are bare returns rather than constant
    // returns - the original bodies are a single `ret` - so leaving every
    // byte alone is the entire behaviour there is to check.
    std::vector<uint8_t> mi_storage(sizeof(MainInterface) + 32);
    std::vector<uint8_t> mi_expected(mi_storage.size());
    auto *interface = reinterpret_cast<MainInterface *>(mi_storage.data() + 16);
    seed_storage(mi_storage.data(), mi_expected.data(), mi_storage.size());
    std::memcpy(mi_expected.data(), mi_storage.data(), mi_storage.size());
    interface->UNK2();
    interface->UNK3();
    interface->UNK4();
    main_interface_unk2_redirect(interface, nullptr);
    main_interface_unk3_redirect(interface, nullptr);
    main_interface_unk4_redirect(interface, nullptr);
    expect_storage_bytes(mi_storage.data(), mi_expected.data(), mi_storage.size());

    alignas(Flic) uint8_t flic_storage[sizeof(Flic) + 32];
    uint8_t flic_expected[sizeof(flic_storage)];
    auto *flic = reinterpret_cast<Flic *>(flic_storage + 16);
    seed_storage(flic_storage, flic_expected, sizeof(flic_storage));
    std::memcpy(flic_expected, flic_storage, sizeof(flic_storage));
    flic->UNK4();
    flic->UNK5();
    flic->UNK6();
    flic->UNK7();
    flic->UNK8();
    flic_unk4_redirect(flic, nullptr);
    flic_unk5_redirect(flic, nullptr);
    flic_unk6_redirect(flic, nullptr);
    flic_unk7_redirect(flic, nullptr);
    flic_unk8_redirect(flic, nullptr);
    expect_storage_bytes(flic_storage, flic_expected, sizeof(flic_storage));

    // Eighteen ambience event hooks across three faction flavours, none of
    // which responds to anything. Where these objects end is not established,
    // so the canary is sized to the layout modelled here rather than to a
    // proven extent; what it checks is that a hook which should do nothing
    // writes nothing, which holds whatever the true trailing size turns out
    // to be.
    alignas(FactionAmbience) uint8_t fa_storage[sizeof(FactionAmbience) + 32];
    uint8_t fa_expected[sizeof(fa_storage)];
    auto *faction_ambience = reinterpret_cast<FactionAmbience *>(fa_storage + 16);
    seed_storage(fa_storage, fa_expected, sizeof(fa_storage));
    std::memcpy(fa_expected, fa_storage, sizeof(fa_storage));
    faction_ambience->begin();
    faction_ambience->tech();
    faction_ambience->terraform();
    faction_ambience->production();
    faction_ambience->general();
    faction_ambience->new_base();
    faction_ambience->popup1();
    faction_ambience->eot();
    faction_ambience->hostility();
    faction_ambience->energy_resources();
    faction_ambience->base_liberated();
    faction_ambience_begin_redirect(faction_ambience, nullptr);
    faction_ambience_tech_redirect(faction_ambience, nullptr);
    faction_ambience_terraform_redirect(faction_ambience, nullptr);
    faction_ambience_production_redirect(faction_ambience, nullptr);
    faction_ambience_general_redirect(faction_ambience, nullptr);
    faction_ambience_new_base_redirect(faction_ambience, nullptr);
    faction_ambience_popup1_redirect(faction_ambience, nullptr);
    faction_ambience_eot_redirect(faction_ambience, nullptr);
    faction_ambience_hostility_redirect(faction_ambience, nullptr);
    faction_ambience_energy_resources_redirect(faction_ambience, nullptr);
    faction_ambience_base_liberated_redirect(faction_ambience, nullptr);
    expect_storage_bytes(fa_storage, fa_expected, sizeof(fa_storage));

    alignas(UAmbience) uint8_t ua_storage[sizeof(UAmbience) + 32];
    uint8_t ua_expected[sizeof(ua_storage)];
    auto *u_ambience = reinterpret_cast<UAmbience *>(ua_storage + 16);
    seed_storage(ua_storage, ua_expected, sizeof(ua_storage));
    std::memcpy(ua_expected, ua_storage, sizeof(ua_storage));
    u_ambience->tech();
    u_ambience->popup1();
    u_ambience->eot();
    u_ambience_tech_redirect(u_ambience, nullptr);
    u_ambience_popup1_redirect(u_ambience, nullptr);
    u_ambience_eot_redirect(u_ambience, nullptr);
    expect_storage_bytes(ua_storage, ua_expected, sizeof(ua_storage));

    alignas(GAmbience) uint8_t ga_storage[sizeof(GAmbience) + 32];
    uint8_t ga_expected[sizeof(ga_storage)];
    auto *g_ambience = reinterpret_cast<GAmbience *>(ga_storage + 16);
    seed_storage(ga_storage, ga_expected, sizeof(ga_storage));
    std::memcpy(ga_expected, ga_storage, sizeof(ga_storage));
    g_ambience->tech();
    g_ambience->production();
    g_ambience->popup1();
    g_ambience->eot();
    g_ambience_tech_redirect(g_ambience, nullptr);
    g_ambience_production_redirect(g_ambience, nullptr);
    g_ambience_popup1_redirect(g_ambience, nullptr);
    g_ambience_eot_redirect(g_ambience, nullptr);
    expect_storage_bytes(ga_storage, ga_expected, sizeof(ga_storage));

    // Two sound devices that decline to be polled, suspended, or restarted,
    // and one Win clip reset that resets nothing. Their layouts are bounded
    // rather than established, so as with the ambience hooks the canary is
    // sized to what is modelled here; a method that should do nothing writes
    // nothing regardless of where the object really ends.
    alignas(Midi_Device) uint8_t midi_storage[sizeof(Midi_Device) + 32];
    uint8_t midi_expected[sizeof(midi_storage)];
    auto *midi = reinterpret_cast<Midi_Device *>(midi_storage + 16);
    seed_storage(midi_storage, midi_expected, sizeof(midi_storage));
    std::memcpy(midi_expected, midi_storage, sizeof(midi_storage));
    midi->update_sound();
    midi->suspend();
    midi->restart();
    midi_device_update_sound_redirect(midi, nullptr);
    midi_device_suspend_redirect(midi, nullptr);
    midi_device_restart_redirect(midi, nullptr);
    expect_storage_bytes(midi_storage, midi_expected, sizeof(midi_storage));

    alignas(Wave_In_Device) uint8_t wave_storage[sizeof(Wave_In_Device) + 32];
    uint8_t wave_expected[sizeof(wave_storage)];
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(wave_storage + 16);
    seed_storage(wave_storage, wave_expected, sizeof(wave_storage));
    std::memcpy(wave_expected, wave_storage, sizeof(wave_storage));
    wave_in->update_sound();
    wave_in->suspend();
    wave_in->restart();
    wave_in_device_update_sound_redirect(wave_in, nullptr);
    wave_in_device_suspend_redirect(wave_in, nullptr);
    wave_in_device_restart_redirect(wave_in, nullptr);
    expect_storage_bytes(wave_storage, wave_expected, sizeof(wave_storage));

    alignas(Win) uint8_t clip_storage[sizeof(Win) + 32];
    uint8_t clip_expected[sizeof(clip_storage)];
    auto *clip_win = reinterpret_cast<Win *>(clip_storage + 16);
    seed_storage(clip_storage, clip_expected, sizeof(clip_storage));
    std::memcpy(clip_expected, clip_storage, sizeof(clip_storage));
    clip_win->reset_window_clip();
    win_reset_window_clip_redirect(clip_win, nullptr);
    expect_storage_bytes(clip_storage, clip_expected, sizeof(clip_storage));

    // MapWin is the first class here whose size is pinned rather than
    // bounded, so unlike the ambience and sound-device canaries this one
    // spans the whole object the original allocates - 0x22480 bytes, far too
    // large for the stack.
    std::vector<uint8_t> mw_storage(sizeof(MapWin) + 32);
    std::vector<uint8_t> mw_expected(mw_storage.size());
    auto *map_win = reinterpret_cast<MapWin *>(mw_storage.data() + 16);
    seed_storage(mw_storage.data(), mw_expected.data(), mw_storage.size());
    std::memcpy(mw_expected.data(), mw_storage.data(), mw_storage.size());
    map_win->UNK3();
    map_win->do_image_buttons();
    map_win_unk3_redirect(map_win, nullptr);
    map_win_do_image_buttons_redirect(map_win, nullptr);
    expect_storage_bytes(mw_storage.data(), mw_expected.data(), mw_storage.size());

    // Sixteen bare returns across eight window classes. None of these layouts
    // is established, so each canary spans only the GraphicWin base the class
    // is declared with rather than the object's true extent - which is all
    // these methods could reach anyway, since they touch nothing.
    std::vector<uint8_t> base_win_storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> base_win_expected(base_win_storage.size());
    auto *base_win = reinterpret_cast<BaseWin *>(base_win_storage.data() + 16);
    seed_storage(base_win_storage.data(), base_win_expected.data(), base_win_storage.size());
    std::memcpy(base_win_expected.data(), base_win_storage.data(), base_win_storage.size());
    base_win->close();
    base_win->UNK4();
    base_win->UNK6();
    base_win->UNK7();
    base_win_close_redirect(base_win, nullptr);
    base_win_unk4_redirect(base_win, nullptr);
    base_win_unk6_redirect(base_win, nullptr);
    base_win_unk7_redirect(base_win, nullptr);
    expect_storage_bytes(base_win_storage.data(), base_win_expected.data(),
                         base_win_storage.size());
    std::vector<uint8_t> datalink_storage(sizeof(Datalink) + 32);
    std::vector<uint8_t> datalink_expected(datalink_storage.size());
    auto *datalink = reinterpret_cast<Datalink *>(datalink_storage.data() + 16);
    seed_storage(datalink_storage.data(), datalink_expected.data(), datalink_storage.size());
    std::memcpy(datalink_expected.data(), datalink_storage.data(), datalink_storage.size());
    datalink->UNK6();
    datalink->UNK8();
    datalink->UNK9();
    datalink_unk6_redirect(datalink, nullptr);
    datalink_unk8_redirect(datalink, nullptr);
    datalink_unk9_redirect(datalink, nullptr);
    expect_storage_bytes(datalink_storage.data(), datalink_expected.data(),
                         datalink_storage.size());
    std::vector<uint8_t> net_win_storage(sizeof(NetWin) + 32);
    std::vector<uint8_t> net_win_expected(net_win_storage.size());
    auto *net_win = reinterpret_cast<NetWin *>(net_win_storage.data() + 16);
    seed_storage(net_win_storage.data(), net_win_expected.data(), net_win_storage.size());
    std::memcpy(net_win_expected.data(), net_win_storage.data(), net_win_storage.size());
    net_win->UNK1();
    net_win->UNK2();
    net_win->alloc_slots();
    net_win_unk1_redirect(net_win, nullptr);
    net_win_unk2_redirect(net_win, nullptr);
    net_win_alloc_slots_redirect(net_win, nullptr);
    expect_storage_bytes(net_win_storage.data(), net_win_expected.data(),
                         net_win_storage.size());
    std::vector<uint8_t> social_win_storage(sizeof(SocialWin) + 32);
    std::vector<uint8_t> social_win_expected(social_win_storage.size());
    auto *social_win = reinterpret_cast<SocialWin *>(social_win_storage.data() + 16);
    seed_storage(social_win_storage.data(), social_win_expected.data(), social_win_storage.size());
    std::memcpy(social_win_expected.data(), social_win_storage.data(), social_win_storage.size());
    social_win->UNK2();
    social_win->UNK3();
    social_win_unk2_redirect(social_win, nullptr);
    social_win_unk3_redirect(social_win, nullptr);
    expect_storage_bytes(social_win_storage.data(), social_win_expected.data(),
                         social_win_storage.size());
    std::vector<uint8_t> design_win_storage(sizeof(DesignWin) + 32);
    std::vector<uint8_t> design_win_expected(design_win_storage.size());
    auto *design_win = reinterpret_cast<DesignWin *>(design_win_storage.data() + 16);
    seed_storage(design_win_storage.data(), design_win_expected.data(), design_win_storage.size());
    std::memcpy(design_win_expected.data(), design_win_storage.data(), design_win_storage.size());
    design_win->UNK1();
    design_win_unk1_redirect(design_win, nullptr);
    expect_storage_bytes(design_win_storage.data(), design_win_expected.data(),
                         design_win_storage.size());
    std::vector<uint8_t> world_win_storage(sizeof(WorldWin) + 32);
    std::vector<uint8_t> world_win_expected(world_win_storage.size());
    auto *world_win = reinterpret_cast<WorldWin *>(world_win_storage.data() + 16);
    seed_storage(world_win_storage.data(), world_win_expected.data(), world_win_storage.size());
    std::memcpy(world_win_expected.data(), world_win_storage.data(), world_win_storage.size());
    world_win->clear_terrain();
    world_win_clear_terrain_redirect(world_win, nullptr);
    expect_storage_bytes(world_win_storage.data(), world_win_expected.data(),
                         world_win_storage.size());
    std::vector<uint8_t> diplo_win_storage(sizeof(DiploWin) + 32);
    std::vector<uint8_t> diplo_win_expected(diplo_win_storage.size());
    auto *diplo_win = reinterpret_cast<DiploWin *>(diplo_win_storage.data() + 16);
    seed_storage(diplo_win_storage.data(), diplo_win_expected.data(), diplo_win_storage.size());
    std::memcpy(diplo_win_expected.data(), diplo_win_storage.data(), diplo_win_storage.size());
    diplo_win->UNK5();
    diplo_win_unk5_redirect(diplo_win, nullptr);
    expect_storage_bytes(diplo_win_storage.data(), diplo_win_expected.data(),
                         diplo_win_storage.size());
    std::vector<uint8_t> alpha_movie_storage(sizeof(AlphaMovie) + 32);
    std::vector<uint8_t> alpha_movie_expected(alpha_movie_storage.size());
    auto *alpha_movie = reinterpret_cast<AlphaMovie *>(alpha_movie_storage.data() + 16);
    seed_storage(alpha_movie_storage.data(), alpha_movie_expected.data(), alpha_movie_storage.size());
    std::memcpy(alpha_movie_expected.data(), alpha_movie_storage.data(), alpha_movie_storage.size());
    alpha_movie->UNK7();
    alpha_movie_unk7_redirect(alpha_movie, nullptr);
    expect_storage_bytes(alpha_movie_storage.data(), alpha_movie_expected.data(),
                         alpha_movie_storage.size());

    // Three more window classes, each over a base this repository has already
    // pinned - Time, Caviar, FlatButton - so the canary covers an exact base
    // region even though the derived extent is still unknown.
    std::vector<uint8_t> info_win_storage(sizeof(InfoWin) + 32);
    std::vector<uint8_t> info_win_expected(info_win_storage.size());
    auto *info_win = reinterpret_cast<InfoWin *>(info_win_storage.data() + 16);
    seed_storage(info_win_storage.data(), info_win_expected.data(), info_win_storage.size());
    std::memcpy(info_win_expected.data(), info_win_storage.data(), info_win_storage.size());
    info_win->UNK1();
    info_win->on_redraw();
    info_win_unk1_redirect(info_win, nullptr);
    info_win_on_redraw_redirect(info_win, nullptr);
    expect_storage_bytes(info_win_storage.data(), info_win_expected.data(),
                         info_win_storage.size());
    std::vector<uint8_t> status_win_storage(sizeof(StatusWin) + 32);
    std::vector<uint8_t> status_win_expected(status_win_storage.size());
    auto *status_win = reinterpret_cast<StatusWin *>(status_win_storage.data() + 16);
    seed_storage(status_win_storage.data(), status_win_expected.data(), status_win_storage.size());
    std::memcpy(status_win_expected.data(), status_win_storage.data(), status_win_storage.size());
    status_win->close();
    status_win_close_redirect(status_win, nullptr);
    expect_storage_bytes(status_win_storage.data(), status_win_expected.data(),
                         status_win_storage.size());
    std::vector<uint8_t> file_win_storage(sizeof(FileWin) + 32);
    std::vector<uint8_t> file_win_expected(file_win_storage.size());
    auto *file_win = reinterpret_cast<FileWin *>(file_win_storage.data() + 16);
    seed_storage(file_win_storage.data(), file_win_expected.data(), file_win_storage.size());
    std::memcpy(file_win_expected.data(), file_win_storage.data(), file_win_storage.size());
    file_win->UNK6();
    file_win_unk6_redirect(file_win, nullptr);
    expect_storage_bytes(file_win_storage.data(), file_win_expected.data(),
                         file_win_storage.size());

    // Wave::set_asdr is the only member of this sweep that returns a value
    // rather than nothing: a single constant load of 11, which is the part a
    // constant-return stub most easily gets wrong.
    std::vector<uint8_t> asdr_storage(sizeof(Wave) + 32);
    std::vector<uint8_t> asdr_expected(asdr_storage.size());
    auto *wave = reinterpret_cast<Wave *>(asdr_storage.data() + 16);
    seed_storage(asdr_storage.data(), asdr_expected.data(), asdr_storage.size());
    std::memcpy(asdr_expected.data(), asdr_storage.data(), asdr_storage.size());
    expect(wave->set_asdr() == 11);
    expect(wave_set_asdr_redirect(wave, nullptr) == 11);
    expect_storage_bytes(asdr_storage.data(), asdr_expected.data(),
                         asdr_storage.size());

    // Both class-level closers are static and __cdecl in the original, so
    // they take no instance at all and there is nothing to canary.
    CheckButton::close_class();
    Cursor::close_cursor_class();
    check_button_close_class_redirect();
    cursor_close_cursor_class_redirect();

    // Fifty-six stubs that take arguments and ignore them. Every call below
    // passes deliberately hostile values - INT_MIN, INT_MAX, negatives - so a
    // body that actually read an argument, or that returned one rather than
    // its constant, would show up rather than coincide with the expected
    // answer. The canary catches anything written; the return check catches
    // anything computed.
    std::vector<uint8_t> alpha_movie_c_storage(sizeof(AlphaMovie) + 32);
    std::vector<uint8_t> alpha_movie_c_expected(alpha_movie_c_storage.size());
    auto *alpha_movie_c = reinterpret_cast<AlphaMovie *>(alpha_movie_c_storage.data() + 16);
    seed_storage(alpha_movie_c_storage.data(), alpha_movie_c_expected.data(), alpha_movie_c_storage.size());
    std::memcpy(alpha_movie_c_expected.data(), alpha_movie_c_storage.data(), alpha_movie_c_storage.size());
    expect(alpha_movie_c->UNK2(-1, 2147483647, -2147483648) == 0);
    expect(alpha_movie_unk2_redirect(alpha_movie_c, nullptr, -1, 2147483647, -2147483648) == 0);
    expect(alpha_movie_c->UNK4(-1, 2147483647, -2147483648, 3) == 0);
    expect(alpha_movie_unk4_redirect(alpha_movie_c, nullptr, -1, 2147483647, -2147483648, 3) == 0);
    expect(alpha_movie_c->UNK5(-1) == 0);
    expect(alpha_movie_unk5_redirect(alpha_movie_c, nullptr, -1) == 0);
    alpha_movie_c->UNK6(-1, 2147483647);
    alpha_movie_unk6_00404260_redirect(alpha_movie_c, nullptr, -1, 2147483647);
    alpha_movie_c->UNK6(-1);
    alpha_movie_unk6_00404270_redirect(alpha_movie_c, nullptr, -1);
    expect(alpha_movie_c->UNK8(-1, 2147483647) == 1);
    expect(alpha_movie_unk8_redirect(alpha_movie_c, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(alpha_movie_c_storage.data(), alpha_movie_c_expected.data(),
                         alpha_movie_c_storage.size());
    std::vector<uint8_t> base_win_c_storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> base_win_c_expected(base_win_c_storage.size());
    auto *base_win_c = reinterpret_cast<BaseWin *>(base_win_c_storage.data() + 16);
    seed_storage(base_win_c_storage.data(), base_win_c_expected.data(), base_win_c_storage.size());
    std::memcpy(base_win_c_expected.data(), base_win_c_storage.data(), base_win_c_storage.size());
    base_win_c->UNK5(-1);
    base_win_unk5_redirect(base_win_c, nullptr, -1);
    base_win_c->on_button_toggled(-1, 2147483647);
    base_win_on_button_toggled_redirect(base_win_c, nullptr, -1, 2147483647);
    base_win_c->on_iface_right_down(-1, 2147483647);
    base_win_on_iface_right_down_redirect(base_win_c, nullptr, -1, 2147483647);
    base_win_c->on_iface_selected(-1, 2147483647);
    base_win_on_iface_selected_redirect(base_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(base_win_c_storage.data(), base_win_c_expected.data(),
                         base_win_c_storage.size());
    std::vector<uint8_t> datalink_c_storage(sizeof(Datalink) + 32);
    std::vector<uint8_t> datalink_c_expected(datalink_c_storage.size());
    auto *datalink_c = reinterpret_cast<Datalink *>(datalink_c_storage.data() + 16);
    seed_storage(datalink_c_storage.data(), datalink_c_expected.data(), datalink_c_storage.size());
    std::memcpy(datalink_c_expected.data(), datalink_c_storage.data(), datalink_c_storage.size());
    datalink_c->on_left_click(-1, 2147483647);
    datalink_on_left_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_group_clicked(-1, 2147483647);
    datalink_on_group_clicked_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_mouse_move(-1, 2147483647);
    datalink_on_mouse_move_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_click(-1, 2147483647);
    datalink_on_iface_left_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_click(-1, 2147483647);
    datalink_on_iface_right_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_down(-1, 2147483647);
    datalink_on_iface_left_down_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_down(-1, 2147483647);
    datalink_on_iface_right_down_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_mouse_move(-1, 2147483647);
    datalink_on_iface_mouse_move_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_mouse_leave(-1, 2147483647);
    datalink_on_iface_mouse_leave_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_selected(-1, 2147483647);
    datalink_on_iface_selected_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_double_click(-1, 2147483647);
    datalink_on_iface_left_double_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_double_click(-1, 2147483647);
    datalink_on_iface_right_double_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_button_clicked(-1);
    datalink_on_iface_button_clicked_redirect(datalink_c, nullptr, -1);
    datalink_c->on_iface_button_toggled(-1, 2147483647);
    datalink_on_iface_button_toggled_redirect(datalink_c, nullptr, -1, 2147483647);
    expect_storage_bytes(datalink_c_storage.data(), datalink_c_expected.data(),
                         datalink_c_storage.size());
    std::vector<uint8_t> design_win_c_storage(sizeof(DesignWin) + 32);
    std::vector<uint8_t> design_win_c_expected(design_win_c_storage.size());
    auto *design_win_c = reinterpret_cast<DesignWin *>(design_win_c_storage.data() + 16);
    seed_storage(design_win_c_storage.data(), design_win_c_expected.data(), design_win_c_storage.size());
    std::memcpy(design_win_c_expected.data(), design_win_c_storage.data(), design_win_c_storage.size());
    design_win_c->on_iface_left_down(-1, 2147483647);
    design_win_on_iface_left_down_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_right_down(-1, 2147483647);
    design_win_on_iface_right_down_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_selected(-1, 2147483647);
    design_win_on_iface_selected_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_left_double_click(-1, 2147483647);
    design_win_on_iface_left_double_click_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_right_double_click(-1, 2147483647);
    design_win_on_iface_right_double_click_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_button_toggled(-1, 2147483647);
    design_win_on_iface_button_toggled_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_group_clicked(-1, 2147483647, -2147483648);
    design_win_on_iface_group_clicked_redirect(design_win_c, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(design_win_c_storage.data(), design_win_c_expected.data(),
                         design_win_c_storage.size());
    std::vector<uint8_t> diplo_win_c_storage(sizeof(DiploWin) + 32);
    std::vector<uint8_t> diplo_win_c_expected(diplo_win_c_storage.size());
    auto *diplo_win_c = reinterpret_cast<DiploWin *>(diplo_win_c_storage.data() + 16);
    seed_storage(diplo_win_c_storage.data(), diplo_win_c_expected.data(), diplo_win_c_storage.size());
    std::memcpy(diplo_win_c_expected.data(), diplo_win_c_storage.data(), diplo_win_c_storage.size());
    diplo_win_c->UNK4(-1);
    diplo_win_unk4_redirect(diplo_win_c, nullptr, -1);
    diplo_win_c->on_iface_left_click(-1, 2147483647);
    diplo_win_on_iface_left_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_up(-1, 2147483647);
    diplo_win_on_iface_left_up_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_click(-1, 2147483647);
    diplo_win_on_iface_right_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_down(-1, 2147483647);
    diplo_win_on_iface_left_down_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_down(-1, 2147483647);
    diplo_win_on_iface_right_down_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_mouse_move(-1, 2147483647);
    diplo_win_on_iface_mouse_move_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_mouse_leave(-1, 2147483647);
    diplo_win_on_iface_mouse_leave_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_selected(-1, 2147483647);
    diplo_win_on_iface_selected_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_double_click(-1, 2147483647);
    diplo_win_on_iface_left_double_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_double_click(-1, 2147483647);
    diplo_win_on_iface_right_double_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_button_clicked(-1);
    diplo_win_on_iface_button_clicked_redirect(diplo_win_c, nullptr, -1);
    diplo_win_c->on_iface_button_toggled(-1, 2147483647);
    diplo_win_on_iface_button_toggled_redirect(diplo_win_c, nullptr, -1, 2147483647);
    expect(diplo_win_c->on_iface_dialog_back_draw(-1) == 0);
    expect(diplo_win_on_iface_dialog_back_draw_redirect(diplo_win_c, nullptr, -1) == 0);
    expect_storage_bytes(diplo_win_c_storage.data(), diplo_win_c_expected.data(),
                         diplo_win_c_storage.size());
    std::vector<uint8_t> net_win_c_storage(sizeof(NetWin) + 32);
    std::vector<uint8_t> net_win_c_expected(net_win_c_storage.size());
    auto *net_win_c = reinterpret_cast<NetWin *>(net_win_c_storage.data() + 16);
    seed_storage(net_win_c_storage.data(), net_win_c_expected.data(), net_win_c_storage.size());
    std::memcpy(net_win_c_expected.data(), net_win_c_storage.data(), net_win_c_storage.size());
    expect(net_win_c->on_key_click(-1, 2147483647) == 1);
    expect(net_win_on_key_click_redirect(net_win_c, nullptr, -1, 2147483647) == 1);
    net_win_c->on_mouse_move(-1, 2147483647);
    net_win_on_mouse_move_redirect(net_win_c, nullptr, -1, 2147483647);
    net_win_c->on_mouse_leave(-1, 2147483647);
    net_win_on_mouse_leave_redirect(net_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(net_win_c_storage.data(), net_win_c_expected.data(),
                         net_win_c_storage.size());
    std::vector<uint8_t> social_win_c_storage(sizeof(SocialWin) + 32);
    std::vector<uint8_t> social_win_c_expected(social_win_c_storage.size());
    auto *social_win_c = reinterpret_cast<SocialWin *>(social_win_c_storage.data() + 16);
    seed_storage(social_win_c_storage.data(), social_win_c_expected.data(), social_win_c_storage.size());
    std::memcpy(social_win_c_expected.data(), social_win_c_storage.data(), social_win_c_storage.size());
    social_win_c->on_iface_left_down(-1, 2147483647);
    social_win_on_iface_left_down_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_right_down(-1, 2147483647);
    social_win_on_iface_right_down_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_selected(-1, 2147483647);
    social_win_on_iface_selected_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_left_double_click(-1, 2147483647);
    social_win_on_iface_left_double_click_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_right_double_click(-1, 2147483647);
    social_win_on_iface_right_double_click_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_button_toggled(-1, 2147483647);
    social_win_on_iface_button_toggled_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_group_clicked(-1, 2147483647, -2147483648);
    social_win_on_iface_group_clicked_redirect(social_win_c, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(social_win_c_storage.data(), social_win_c_expected.data(),
                         social_win_c_storage.size());
    std::vector<uint8_t> world_win_c_storage(sizeof(WorldWin) + 32);
    std::vector<uint8_t> world_win_c_expected(world_win_c_storage.size());
    auto *world_win_c = reinterpret_cast<WorldWin *>(world_win_c_storage.data() + 16);
    seed_storage(world_win_c_storage.data(), world_win_c_expected.data(), world_win_c_storage.size());
    std::memcpy(world_win_c_expected.data(), world_win_c_storage.data(), world_win_c_storage.size());
    world_win_c->on_left_double_click(-1, 2147483647);
    world_win_on_left_double_click_redirect(world_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(world_win_c_storage.data(), world_win_c_expected.data(),
                         world_win_c_storage.size());

    // Eleven more classes, each over a base already pinned here, so every
    // canary below covers an exact base region. Same hostile arguments as
    // above.
    std::vector<uint8_t> battle_win_k_storage(sizeof(BattleWin) + 32);
    std::vector<uint8_t> battle_win_k_expected(battle_win_k_storage.size());
    auto *battle_win_k = reinterpret_cast<BattleWin *>(battle_win_k_storage.data() + 16);
    seed_storage(battle_win_k_storage.data(), battle_win_k_expected.data(), battle_win_k_storage.size());
    std::memcpy(battle_win_k_expected.data(), battle_win_k_storage.data(), battle_win_k_storage.size());
    battle_win_k->on_iface_left_click(-1, 2147483647);
    battle_win_on_iface_left_click_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_right_click(-1, 2147483647);
    battle_win_on_iface_right_click_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_right_down(-1, 2147483647);
    battle_win_on_iface_right_down_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_mouse_move(-1, 2147483647);
    battle_win_on_iface_mouse_move_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_mouse_leave(-1, 2147483647);
    battle_win_on_iface_mouse_leave_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_selected(-1, 2147483647);
    battle_win_on_iface_selected_redirect(battle_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(battle_win_k_storage.data(), battle_win_k_expected.data(),
                         battle_win_k_storage.size());
    std::vector<uint8_t> counc_win_k_storage(sizeof(CouncWin) + 32);
    std::vector<uint8_t> counc_win_k_expected(counc_win_k_storage.size());
    auto *counc_win_k = reinterpret_cast<CouncWin *>(counc_win_k_storage.data() + 16);
    seed_storage(counc_win_k_storage.data(), counc_win_k_expected.data(), counc_win_k_storage.size());
    std::memcpy(counc_win_k_expected.data(), counc_win_k_storage.data(), counc_win_k_storage.size());
    counc_win_k->on_group_clicked(-1, 2147483647);
    counc_win_on_group_clicked_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_button_passover(-1, 2147483647);
    counc_win_on_button_passover_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_click(-1, 2147483647);
    counc_win_on_iface_left_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_click(-1, 2147483647);
    counc_win_on_iface_right_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_down(-1, 2147483647);
    counc_win_on_iface_left_down_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_down(-1, 2147483647);
    counc_win_on_iface_right_down_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_mouse_move(-1, 2147483647);
    counc_win_on_iface_mouse_move_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_mouse_leave(-1, 2147483647);
    counc_win_on_iface_mouse_leave_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_selected(-1, 2147483647);
    counc_win_on_iface_selected_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_double_click(-1, 2147483647);
    counc_win_on_iface_left_double_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_double_click(-1, 2147483647);
    counc_win_on_iface_right_double_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_button_toggled(-1, 2147483647);
    counc_win_on_iface_button_toggled_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_group_clicked(-1, 2147483647, -2147483648);
    counc_win_on_iface_group_clicked_redirect(counc_win_k, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(counc_win_k_storage.data(), counc_win_k_expected.data(),
                         counc_win_k_storage.size());
    std::vector<uint8_t> credits_k_storage(sizeof(Credits) + 32);
    std::vector<uint8_t> credits_k_expected(credits_k_storage.size());
    auto *credits_k = reinterpret_cast<Credits *>(credits_k_storage.data() + 16);
    seed_storage(credits_k_storage.data(), credits_k_expected.data(), credits_k_storage.size());
    std::memcpy(credits_k_expected.data(), credits_k_storage.data(), credits_k_storage.size());
    credits_k->on_left_down(-1, 2147483647);
    credits_on_left_down_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_right_down(-1, 2147483647);
    credits_on_right_down_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_mouse_move(-1, 2147483647);
    credits_on_mouse_move_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_button_clicked(-1);
    credits_on_button_clicked_redirect(credits_k, nullptr, -1);
    expect_storage_bytes(credits_k_storage.data(), credits_k_expected.data(),
                         credits_k_storage.size());
    std::vector<uint8_t> diplo_pop_k_storage(sizeof(DiploPop) + 32);
    std::vector<uint8_t> diplo_pop_k_expected(diplo_pop_k_storage.size());
    auto *diplo_pop_k = reinterpret_cast<DiploPop *>(diplo_pop_k_storage.data() + 16);
    seed_storage(diplo_pop_k_storage.data(), diplo_pop_k_expected.data(), diplo_pop_k_storage.size());
    std::memcpy(diplo_pop_k_expected.data(), diplo_pop_k_storage.data(), diplo_pop_k_storage.size());
    diplo_pop_k->on_iface_left_click(-1, 2147483647);
    diplo_pop_on_iface_left_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_click(-1, 2147483647);
    diplo_pop_on_iface_right_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_left_down(-1, 2147483647);
    diplo_pop_on_iface_left_down_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_down(-1, 2147483647);
    diplo_pop_on_iface_right_down_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_mouse_move(-1, 2147483647);
    diplo_pop_on_iface_mouse_move_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_mouse_leave(-1, 2147483647);
    diplo_pop_on_iface_mouse_leave_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_selected(-1, 2147483647);
    diplo_pop_on_iface_selected_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_left_double_click(-1, 2147483647);
    diplo_pop_on_iface_left_double_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_double_click(-1, 2147483647);
    diplo_pop_on_iface_right_double_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_button_toggled(-1, 2147483647);
    diplo_pop_on_iface_button_toggled_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    expect_storage_bytes(diplo_pop_k_storage.data(), diplo_pop_k_expected.data(),
                         diplo_pop_k_storage.size());
    std::vector<uint8_t> gamma_k_storage(sizeof(Gamma) + 32);
    std::vector<uint8_t> gamma_k_expected(gamma_k_storage.size());
    auto *gamma_k = reinterpret_cast<Gamma *>(gamma_k_storage.data() + 16);
    seed_storage(gamma_k_storage.data(), gamma_k_expected.data(), gamma_k_storage.size());
    std::memcpy(gamma_k_expected.data(), gamma_k_storage.data(), gamma_k_storage.size());
    expect(gamma_k->on_key_click(-1, 2147483647) == 1);
    expect(gamma_on_key_click_redirect(gamma_k, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(gamma_k_storage.data(), gamma_k_expected.data(),
                         gamma_k_storage.size());
    std::vector<uint8_t> message_win_k_storage(sizeof(MessageWin) + 32);
    std::vector<uint8_t> message_win_k_expected(message_win_k_storage.size());
    auto *message_win_k = reinterpret_cast<MessageWin *>(message_win_k_storage.data() + 16);
    seed_storage(message_win_k_storage.data(), message_win_k_expected.data(), message_win_k_storage.size());
    std::memcpy(message_win_k_expected.data(), message_win_k_storage.data(), message_win_k_storage.size());
    message_win_k->UNK1(-1, 2147483647, -2147483648, 3, -4);
    message_win_unk1_redirect(message_win_k, nullptr, -1, 2147483647, -2147483648, 3, -4);
    message_win_k->UNK4(-1);
    message_win_unk4_redirect(message_win_k, nullptr, -1);
    expect_storage_bytes(message_win_k_storage.data(), message_win_k_expected.data(),
                         message_win_k_storage.size());
    std::vector<uint8_t> new_tech_win_k_storage(sizeof(NewTechWin) + 32);
    std::vector<uint8_t> new_tech_win_k_expected(new_tech_win_k_storage.size());
    auto *new_tech_win_k = reinterpret_cast<NewTechWin *>(new_tech_win_k_storage.data() + 16);
    seed_storage(new_tech_win_k_storage.data(), new_tech_win_k_expected.data(), new_tech_win_k_storage.size());
    std::memcpy(new_tech_win_k_expected.data(), new_tech_win_k_storage.data(), new_tech_win_k_storage.size());
    new_tech_win_k->on_left_click(-1, 2147483647);
    new_tech_win_on_left_click_redirect(new_tech_win_k, nullptr, -1, 2147483647);
    expect(new_tech_win_k->on_key_click(-1, 2147483647) == 1);
    expect(new_tech_win_on_key_click_redirect(new_tech_win_k, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(new_tech_win_k_storage.data(), new_tech_win_k_expected.data(),
                         new_tech_win_k_storage.size());
    std::vector<uint8_t> pick_win_k_storage(sizeof(PickWin) + 32);
    std::vector<uint8_t> pick_win_k_expected(pick_win_k_storage.size());
    auto *pick_win_k = reinterpret_cast<PickWin *>(pick_win_k_storage.data() + 16);
    seed_storage(pick_win_k_storage.data(), pick_win_k_expected.data(), pick_win_k_storage.size());
    std::memcpy(pick_win_k_expected.data(), pick_win_k_storage.data(), pick_win_k_storage.size());
    pick_win_k->on_button_passover(-1, 2147483647);
    pick_win_on_button_passover_redirect(pick_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(pick_win_k_storage.data(), pick_win_k_expected.data(),
                         pick_win_k_storage.size());
    std::vector<uint8_t> prod_picker_k_storage(sizeof(ProdPicker) + 32);
    std::vector<uint8_t> prod_picker_k_expected(prod_picker_k_storage.size());
    auto *prod_picker_k = reinterpret_cast<ProdPicker *>(prod_picker_k_storage.data() + 16);
    seed_storage(prod_picker_k_storage.data(), prod_picker_k_expected.data(), prod_picker_k_storage.size());
    std::memcpy(prod_picker_k_expected.data(), prod_picker_k_storage.data(), prod_picker_k_storage.size());
    expect(prod_picker_k->UNK1(-1) == 1);
    expect(prod_picker_unk1_redirect(prod_picker_k, nullptr, -1) == 1);
    prod_picker_k->on_mouse_move(-1, 2147483647);
    prod_picker_on_mouse_move_redirect(prod_picker_k, nullptr, -1, 2147483647);
    prod_picker_k->UNK3(-1);
    prod_picker_unk3_redirect(prod_picker_k, nullptr, -1);
    expect_storage_bytes(prod_picker_k_storage.data(), prod_picker_k_expected.data(),
                         prod_picker_k_storage.size());
    std::vector<uint8_t> report_win_k_storage(sizeof(ReportWin) + 32);
    std::vector<uint8_t> report_win_k_expected(report_win_k_storage.size());
    auto *report_win_k = reinterpret_cast<ReportWin *>(report_win_k_storage.data() + 16);
    seed_storage(report_win_k_storage.data(), report_win_k_expected.data(), report_win_k_storage.size());
    std::memcpy(report_win_k_expected.data(), report_win_k_storage.data(), report_win_k_storage.size());
    report_win_k->on_mouse_move(-1, 2147483647);
    report_win_on_mouse_move_redirect(report_win_k, nullptr, -1, 2147483647);
    report_win_k->on_mouse_leave(-1, 2147483647);
    report_win_on_mouse_leave_redirect(report_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(report_win_k_storage.data(), report_win_k_expected.data(),
                         report_win_k_storage.size());
    std::vector<uint8_t> setup_win_k_storage(sizeof(SetupWin) + 32);
    std::vector<uint8_t> setup_win_k_expected(setup_win_k_storage.size());
    auto *setup_win_k = reinterpret_cast<SetupWin *>(setup_win_k_storage.data() + 16);
    seed_storage(setup_win_k_storage.data(), setup_win_k_expected.data(), setup_win_k_storage.size());
    std::memcpy(setup_win_k_expected.data(), setup_win_k_storage.data(), setup_win_k_storage.size());
    expect(setup_win_k->UNK3(-1) == 0);
    expect(setup_win_unk3_redirect(setup_win_k, nullptr, -1) == 0);
    expect(setup_win_k->UNK4(-1) == 0);
    expect(setup_win_unk4_redirect(setup_win_k, nullptr, -1) == 0);
    expect_storage_bytes(setup_win_k_storage.data(), setup_win_k_expected.data(),
                         setup_win_k_storage.size());

    // Thirty-seven more, into classes that already had headers, so these went
    // into existing public sections rather than generated files. The calls are
    // generated from the emitted definitions themselves. Deriving them instead
    // by matching redirect names back to methods, as a first attempt did,
    // matches case-insensitively onto pre-existing methods of the same name -
    // which crashed under Wine calling a real one with a seeded `this`.
    // Several take unsigned parameters, occupying the same four-byte stack
    // slot as an int and fed 0xFFFFFFFF for the same reason.
    std::vector<uint8_t> caviar_z_storage(sizeof(Caviar) + 32);
    std::vector<uint8_t> caviar_z_expected(caviar_z_storage.size());
    auto *caviar_z = reinterpret_cast<Caviar *>(caviar_z_storage.data() + 16);
    seed_storage(caviar_z_storage.data(), caviar_z_expected.data(), caviar_z_storage.size());
    std::memcpy(caviar_z_expected.data(), caviar_z_storage.data(), caviar_z_storage.size());
    caviar_z->UNK12(-1, 2147483647, -2147483648);
    caviar_unk12_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648);
    expect(caviar_z->UNK3(-1, 2147483647, -2147483648) == 0);
    expect(caviar_unk3_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648) == 0);
    expect(caviar_z->UNK4(-1, 2147483647, -2147483648, 3) == 0);
    expect(caviar_unk4_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648, 3) == 0);
    caviar_z->UNK5(-1, 2147483647);
    caviar_unk5_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK6(-1, 2147483647);
    caviar_unk6_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK7(-1, 2147483647);
    caviar_unk7_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK9(-1, 2147483647, -2147483648);
    caviar_unk9_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(caviar_z_storage.data(), caviar_z_expected.data(),
                         caviar_z_storage.size());
    std::vector<uint8_t> file_win_z_storage(sizeof(FileWin) + 32);
    std::vector<uint8_t> file_win_z_expected(file_win_z_storage.size());
    auto *file_win_z = reinterpret_cast<FileWin *>(file_win_z_storage.data() + 16);
    seed_storage(file_win_z_storage.data(), file_win_z_expected.data(), file_win_z_storage.size());
    std::memcpy(file_win_z_expected.data(), file_win_z_storage.data(), file_win_z_storage.size());
    file_win_z->UNK2(-1);
    file_win_unk2_redirect(file_win_z, nullptr, -1);
    expect_storage_bytes(file_win_z_storage.data(), file_win_z_expected.data(),
                         file_win_z_storage.size());
    std::vector<uint8_t> info_win_z_storage(sizeof(InfoWin) + 32);
    std::vector<uint8_t> info_win_z_expected(info_win_z_storage.size());
    auto *info_win_z = reinterpret_cast<InfoWin *>(info_win_z_storage.data() + 16);
    seed_storage(info_win_z_storage.data(), info_win_z_expected.data(), info_win_z_storage.size());
    std::memcpy(info_win_z_expected.data(), info_win_z_storage.data(), info_win_z_storage.size());
    info_win_z->UNK3(-1, 2147483647);
    info_win_unk3_redirect(info_win_z, nullptr, -1, 2147483647);
    info_win_z->change(-1);
    info_win_change_redirect(info_win_z, nullptr, -1);
    expect_storage_bytes(info_win_z_storage.data(), info_win_z_expected.data(),
                         info_win_z_storage.size());
    std::vector<uint8_t> main_interface_z_storage(sizeof(MainInterface) + 32);
    std::vector<uint8_t> main_interface_z_expected(main_interface_z_storage.size());
    auto *main_interface_z = reinterpret_cast<MainInterface *>(main_interface_z_storage.data() + 16);
    seed_storage(main_interface_z_storage.data(), main_interface_z_expected.data(), main_interface_z_storage.size());
    std::memcpy(main_interface_z_expected.data(), main_interface_z_storage.data(), main_interface_z_storage.size());
    main_interface_z->UNK1(-1);
    main_interface_unk1_redirect(main_interface_z, nullptr, -1);
    expect(main_interface_z->on_iface_button_pressed(-1) == 0);
    expect(main_interface_on_iface_button_pressed_redirect(main_interface_z, nullptr, -1) == 0);
    main_interface_z->on_iface_button_toggled(-1, 2147483647);
    main_interface_on_iface_button_toggled_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_right_double_click(-1, 2147483647);
    main_interface_on_iface_right_double_click_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_scrolled(-1, 2147483647);
    main_interface_on_iface_scrolled_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_scrolling(-1, 2147483647);
    main_interface_on_iface_scrolling_redirect(main_interface_z, nullptr, -1, 2147483647);
    expect_storage_bytes(main_interface_z_storage.data(), main_interface_z_expected.data(),
                         main_interface_z_storage.size());
    std::vector<uint8_t> midi_device_z_storage(sizeof(Midi_Device) + 32);
    std::vector<uint8_t> midi_device_z_expected(midi_device_z_storage.size());
    auto *midi_device_z = reinterpret_cast<Midi_Device *>(midi_device_z_storage.data() + 16);
    seed_storage(midi_device_z_storage.data(), midi_device_z_expected.data(), midi_device_z_storage.size());
    std::memcpy(midi_device_z_expected.data(), midi_device_z_storage.data(), midi_device_z_storage.size());
    expect(midi_device_z->fade(4294967295u) == 0);
    expect(midi_device_fade_redirect(midi_device_z, nullptr, 4294967295u) == 0);
    expect(midi_device_z->select(4294967295u) == 0);
    expect(midi_device_select_redirect(midi_device_z, nullptr, 4294967295u) == 0);
    midi_device_z->set_pan(-1);
    midi_device_set_pan_redirect(midi_device_z, nullptr, -1);
    midi_device_z->set_rate(4294967295u);
    midi_device_set_rate_redirect(midi_device_z, nullptr, 4294967295u);
    midi_device_z->set_volume(4294967295u);
    midi_device_set_volume_redirect(midi_device_z, nullptr, 4294967295u);
    expect_storage_bytes(midi_device_z_storage.data(), midi_device_z_expected.data(),
                         midi_device_z_storage.size());
    std::vector<uint8_t> pull_down_z_storage(sizeof(PullDown) + 32);
    std::vector<uint8_t> pull_down_z_expected(pull_down_z_storage.size());
    auto *pull_down_z = reinterpret_cast<PullDown *>(pull_down_z_storage.data() + 16);
    seed_storage(pull_down_z_storage.data(), pull_down_z_expected.data(), pull_down_z_storage.size());
    std::memcpy(pull_down_z_expected.data(), pull_down_z_storage.data(), pull_down_z_storage.size());
    pull_down_z->on_mouse_leave(-1, 2147483647);
    pull_down_on_mouse_leave_redirect(pull_down_z, nullptr, -1, 2147483647);
    expect_storage_bytes(pull_down_z_storage.data(), pull_down_z_expected.data(),
                         pull_down_z_storage.size());
    std::vector<uint8_t> scroll_z_storage(sizeof(Scroll) + 32);
    std::vector<uint8_t> scroll_z_expected(scroll_z_storage.size());
    auto *scroll_z = reinterpret_cast<Scroll *>(scroll_z_storage.data() + 16);
    seed_storage(scroll_z_storage.data(), scroll_z_expected.data(), scroll_z_storage.size());
    std::memcpy(scroll_z_expected.data(), scroll_z_storage.data(), scroll_z_storage.size());
    scroll_z->on_left_click(-1, 2147483647);
    scroll_on_left_click_redirect(scroll_z, nullptr, -1, 2147483647);
    expect_storage_bytes(scroll_z_storage.data(), scroll_z_expected.data(),
                         scroll_z_storage.size());
    std::vector<uint8_t> sprite_z_storage(sizeof(Sprite) + 32);
    std::vector<uint8_t> sprite_z_expected(sprite_z_storage.size());
    auto *sprite_z = reinterpret_cast<Sprite *>(sprite_z_storage.data() + 16);
    seed_storage(sprite_z_storage.data(), sprite_z_expected.data(), sprite_z_storage.size());
    std::memcpy(sprite_z_expected.data(), sprite_z_storage.data(), sprite_z_storage.size());
    sprite_z->UNK3(-1, 2147483647);
    sprite_unk3_redirect(sprite_z, nullptr, -1, 2147483647);
    sprite_z->UNK4(-1, 2147483647);
    sprite_unk4_redirect(sprite_z, nullptr, -1, 2147483647);
    expect_storage_bytes(sprite_z_storage.data(), sprite_z_expected.data(),
                         sprite_z_storage.size());
    std::vector<uint8_t> wave_z_storage(sizeof(Wave) + 32);
    std::vector<uint8_t> wave_z_expected(wave_z_storage.size());
    auto *wave_z = reinterpret_cast<Wave *>(wave_z_storage.data() + 16);
    seed_storage(wave_z_storage.data(), wave_z_expected.data(), wave_z_storage.size());
    std::memcpy(wave_z_expected.data(), wave_z_storage.data(), wave_z_storage.size());
    expect(wave_z->set_attack(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_attack_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_bufflimit(4294967295u) == 11);
    expect(wave_set_bufflimit_redirect(wave_z, nullptr, 4294967295u) == 11);
    expect(wave_z->set_decay(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_decay_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_release(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_release_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_sustain(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_sustain_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect_storage_bytes(wave_z_storage.data(), wave_z_expected.data(),
                         wave_z_storage.size());
    std::vector<uint8_t> wave_in_device_z_storage(sizeof(Wave_In_Device) + 32);
    std::vector<uint8_t> wave_in_device_z_expected(wave_in_device_z_storage.size());
    auto *wave_in_device_z = reinterpret_cast<Wave_In_Device *>(wave_in_device_z_storage.data() + 16);
    seed_storage(wave_in_device_z_storage.data(), wave_in_device_z_expected.data(), wave_in_device_z_storage.size());
    std::memcpy(wave_in_device_z_expected.data(), wave_in_device_z_storage.data(), wave_in_device_z_storage.size());
    expect(wave_in_device_z->get_caps(4294967295u) == 0);
    expect(wave_in_device_get_caps_redirect(wave_in_device_z, nullptr, 4294967295u) == 0);
    expect(wave_in_device_z->select(4294967295u) == 0);
    expect(wave_in_device_select_redirect(wave_in_device_z, nullptr, 4294967295u) == 0);
    wave_in_device_z->set_rate(4294967295u);
    wave_in_device_set_rate_redirect(wave_in_device_z, nullptr, 4294967295u);
    expect_storage_bytes(wave_in_device_z_storage.data(), wave_in_device_z_expected.data(),
                         wave_in_device_z_storage.size());
    std::vector<uint8_t> win_z_storage(sizeof(Win) + 32);
    std::vector<uint8_t> win_z_expected(win_z_storage.size());
    auto *win_z = reinterpret_cast<Win *>(win_z_storage.data() + 16);
    seed_storage(win_z_storage.data(), win_z_expected.data(), win_z_storage.size());
    std::memcpy(win_z_expected.data(), win_z_storage.data(), win_z_storage.size());
    win_z->on_move(-1, 2147483647);
    win_on_move_redirect(win_z, nullptr, -1, 2147483647);
    win_z->on_size(4294967295u, 2147483647, -2147483648);
    win_on_size_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    win_z->on_size_nc(4294967295u, 2147483647, -2147483648);
    win_on_size_nc_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    win_z->on_sys_command(4294967295u, 2147483647, -2147483648);
    win_on_sys_command_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    expect_storage_bytes(win_z_storage.data(), win_z_expected.data(),
                         win_z_storage.size());
}

void test_status_win_set_loc() {
    // The first recovery here that writes fields rather than nothing, so the
    // question is no longer whether memory moved but whether the right bytes
    // moved. Caviar is pinned at 0x13D0 and these four dwords sit at 0x15B4
    // through 0x15C0; the check reads them back at those absolute offsets and
    // compares every other byte of the object against its seed, so a field
    // declared at the wrong offset fails twice over - the value is missing
    // where it belongs and present where it does not.
    std::vector<uint8_t> storage(sizeof(StatusWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *status = reinterpret_cast<StatusWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    status->set_loc(0x11223344, -2);

    auto read_at = [&](size_t offset) {
        int32_t value = 0;
        std::memcpy(&value, storage.data() + 16 + offset, sizeof(value));
        return value;
    };
    expect(read_at(0x15B4) == 0x11223344);
    expect(read_at(0x15B8) == -2);
    expect(read_at(0x15BC) == -1);
    expect(read_at(0x15C0) == -1);

    // Everything outside those sixteen bytes must be untouched.
    std::memcpy(expected.data() + 16 + 0x15B4, storage.data() + 16 + 0x15B4, 16);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The redirect must land identically, including the two constants.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    status_win_set_loc_redirect(status, nullptr, INT_MIN, INT_MAX);
    expect(read_at(0x15B4) == INT_MIN);
    expect(read_at(0x15B8) == INT_MAX);
    expect(read_at(0x15BC) == -1);
    expect(read_at(0x15C0) == -1);
    std::memcpy(expected.data() + 16 + 0x15B4, storage.data() + 16 + 0x15B4, 16);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
}

void test_base_pop_default_colors() {
    // Two interleaved tables with different geometry: the string table has
    // four tiers so its slots are 0x10 apart, the button table three at 0xC.
    // Both are compared whole after every call, because the failure mode a
    // wrong stride produces is writing a sibling slot rather than a sibling
    // tier - which a check of only the four intended cells would miss.
    uint32_t string_table[16];
    uint32_t button_table[12];
    uint32_t *const saved_string = BasePopDefaultStringColors;
    uint32_t *const saved_button = BasePopDefaultButtonColors;
    BasePopDefaultStringColors = string_table;
    BasePopDefaultButtonColors = button_table;

    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};

    struct TableCase {
        uint32_t *table;
        size_t cells;
        size_t stride;
        size_t tier;
        void (*member)(int, int, int, int);
        void (__cdecl *redirect)(int, int, int, int);
    };
    const TableCase cases[] = {
        {string_table, 16, 0x10, 0, &BasePop::set_def_string_color,
         base_pop_set_def_string_color_redirect},
        {string_table, 16, 0x10, 1, &BasePop::set_def_string_color2,
         base_pop_set_def_string_color2_redirect},
        {string_table, 16, 0x10, 2, &BasePop::set_def_string_color3,
         base_pop_set_def_string_color3_redirect},
        {string_table, 16, 0x10, 3, &BasePop::set_def_string_color_hyper,
         base_pop_set_def_string_color_hyper_redirect},
        {button_table, 12, 0x0C, 0, &BasePop::set_def_button_color,
         base_pop_set_def_button_color_redirect},
        {button_table, 12, 0x0C, 1, &BasePop::set_def_button_color2,
         base_pop_set_def_button_color2_redirect},
        {button_table, 12, 0x0C, 2, &BasePop::set_def_button_color3,
         base_pop_set_def_button_color3_redirect},
    };
    for (const TableCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            uint32_t expected[16];
            for (size_t index = 0; index < test.cells; ++index) {
                test.table[index] = 0xA5000000U ^ static_cast<uint32_t>(index);
                expected[index] = test.table[index];
            }
            for (size_t slot = 0; slot < 4; ++slot) {
                expected[(slot * test.stride + test.tier * 4) / 4] =
                    static_cast<uint32_t>(colors[slot]);
            }
            if (adapter) {
                test.redirect(colors[0], colors[1], colors[2], colors[3]);
            } else {
                test.member(colors[0], colors[1], colors[2], colors[3]);
            }
            for (size_t index = 0; index < test.cells; ++index) {
                expect(test.table[index] == expected[index]);
            }
        }
    }

    BasePopDefaultButtonColors = saved_button;
    BasePopDefaultStringColors = saved_string;
}

void test_win_set_def_focus() {
    int focus = 0x5A5A5A5A;
    int *const saved = WinDefaultFocus;
    WinDefaultFocus = &focus;
    const int values[] = {0, 1, -1, INT_MIN, INT_MAX};
    for (int value : values) {
        Win::set_def_focus(value);
        expect(focus == value);
        win_set_def_focus_redirect(~value);
        expect(focus == ~value);
    }
    WinDefaultFocus = saved;
}

void test_base_pop_instance_colors() {
    // The two families have different storage: string slots are four dwords
    // at 0x3120 with a 0x10 stride, while button slot one is a *byte* at
    // 0x3178 + tier and the remaining three are dwords at 0x317C stride 0xC.
    // A dword written to that byte would overwrite the neighbouring tiers, so
    // the whole object is compared after every call.
    std::vector<uint8_t> bytes(sizeof(BasePop) + 32);
    std::vector<uint8_t> expected_bytes(sizeof(BasePop) + 32);
    uint8_t *const storage = bytes.data();
    uint8_t *const expected = expected_bytes.data();
    auto *popup = reinterpret_cast<BasePop *>(storage + 16);

    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};

    struct StringCase {
        size_t tier;
        void (BasePop::*member)(int, int, int, int);
        void (__fastcall *redirect)(BasePop *, void *, int, int, int, int);
    };
    const StringCase strings[] = {
        {0, &BasePop::set_string_color, base_pop_set_string_color_redirect},
        {1, &BasePop::set_string_color2, base_pop_set_string_color2_redirect},
        {2, &BasePop::set_string_color3, base_pop_set_string_color3_redirect},
        {3, &BasePop::set_string_color_hyper,
         base_pop_set_string_color_hyper_redirect},
    };
    for (const StringCase &test : strings) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, bytes.size());
            std::memcpy(expected, storage, bytes.size());
            for (size_t slot = 0; slot < 4; ++slot) {
                write_at(expected, 16 + 0x3120 + slot * 0x10 + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(popup, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (popup->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, bytes.size());
        }
    }

    const StringCase buttons[] = {
        {0, &BasePop::set_button_color, base_pop_set_button_color_redirect},
        {1, &BasePop::set_button_color2, base_pop_set_button_color2_redirect},
        {2, &BasePop::set_button_color3, base_pop_set_button_color3_redirect},
    };
    for (const StringCase &test : buttons) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, bytes.size());
            std::memcpy(expected, storage, bytes.size());
            // Slot one is a single byte; the truncation is part of the
            // contract, not an accident of the fixture's values.
            expected[16 + 0x3178 + test.tier] =
                static_cast<uint8_t>(colors[0]);
            for (size_t slot = 1; slot < 4; ++slot) {
                write_at(expected,
                         16 + 0x317C + (slot - 1) * 0xC + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(popup, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (popup->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, bytes.size());
        }
    }
}

void test_guarded_store_recoveries() {
    // Win::UNK8/UNK9 publish into two optional scrollbars, each tested
    // separately, so an attached bar is still updated when the other is null.
    std::vector<uint8_t> win_bytes(sizeof(Win) + 32);
    std::vector<uint8_t> win_expected(sizeof(Win) + 32);
    uint8_t *const ws = win_bytes.data();
    uint8_t *const we = win_expected.data();
    auto *win = reinterpret_cast<Win *>(ws + 16);
    std::vector<uint8_t> vert(sizeof(Scroll));
    std::vector<uint8_t> horz(sizeof(Scroll));
    auto *scroll_v = reinterpret_cast<Scroll *>(vert.data());
    auto *scroll_h = reinterpret_cast<Scroll *>(horz.data());

    struct SlotCase { size_t slot; void (Win::*member)(int);
                      void (__fastcall *redirect)(Win *, void *, int); };
    const SlotCase slots[] = {
        {0x2144, &Win::UNK8, win_unk8_redirect},
        {0x2148, &Win::UNK9, win_unk9_redirect},
    };
    // Both attached, only vertical, only horizontal, neither.
    const int attach[][2] = {{1, 1}, {1, 0}, {0, 1}, {0, 0}};
    for (const SlotCase &test : slots) {
        for (const auto &which : attach) {
            for (int adapter = 0; adapter < 2; ++adapter) {
                seed_storage(ws, we, win_bytes.size());
                std::memset(vert.data(), 0xA5, vert.size());
                std::memset(horz.data(), 0xA5, horz.size());
                write_at(ws, 16 + 0x43C,
                         which[0] ? scroll_v : static_cast<Scroll *>(nullptr));
                write_at(ws, 16 + 0x440,
                         which[1] ? scroll_h : static_cast<Scroll *>(nullptr));
                std::memcpy(we, ws, win_bytes.size());
                const int value = 0x1234ABCD;
                if (adapter) {
                    test.redirect(win, nullptr, value);
                } else {
                    (win->*test.member)(value);
                }
                // The window itself is never modified.
                expect_storage_bytes(ws, we, win_bytes.size());
                uint32_t stored;
                if (which[0]) {
                    std::memcpy(&stored, vert.data() + test.slot, sizeof(stored));
                    expect(stored == 0x1234ABCDU);
                }
                if (which[1]) {
                    std::memcpy(&stored, horz.data() + test.slot, sizeof(stored));
                    expect(stored == 0x1234ABCDU);
                }
            }
        }
    }

    // Menu::requested_height doubles the resolved font's height, preferring
    // the menu's own font and falling back to the process default.
    alignas(Font) uint8_t menu_font[sizeof(Font)];
    alignas(Font) uint8_t default_font[sizeof(Font)];
    std::memset(menu_font, 0xA5, sizeof(menu_font));
    std::memset(default_font, 0xA5, sizeof(default_font));
    write_at(menu_font, 0x10, 21);
    write_at(default_font, 0x10, -3);
    Font *menu_slot = reinterpret_cast<Font *>(menu_font);
    Font *default_slot = reinterpret_cast<Font *>(default_font);
    Font **const saved_menu = MenuFont;
    Font **const saved_default = FontDefaultPtr;
    MenuFont = &menu_slot;
    FontDefaultPtr = &default_slot;
    alignas(Menu) uint8_t menu_storage[sizeof(Menu) + 32];
    uint8_t menu_expected[sizeof(menu_storage)];
    auto *menu = reinterpret_cast<Menu *>(menu_storage + 16);
    seed_storage(menu_storage, menu_expected, sizeof(menu_storage));
    std::memcpy(menu_expected, menu_storage, sizeof(menu_storage));
    expect(menu->requested_height() == 42);
    expect(menu_requested_height_redirect(menu, nullptr) == 42);
    // With no menu font the default is resolved instead; the doubling is
    // signed, so a negative height doubles rather than saturating.
    menu_slot = nullptr;
    expect(menu->requested_height() == -6);
    expect_storage_bytes(menu_storage, menu_expected, sizeof(menu_storage));
    FontDefaultPtr = saved_default;
    MenuFont = saved_menu;

    // BasePop::fallout raises its flag only while the gate is set.
    int gate = 0;
    int flag = 0;
    int *const saved_gate = BasePopFalloutGate;
    int *const saved_flag = BasePopFalloutFlag;
    BasePopFalloutGate = &gate;
    BasePopFalloutFlag = &flag;
    BasePop::fallout();
    expect(flag == 0);
    gate = 1;
    BasePop::fallout();
    expect(flag == 1);
    flag = 0;
    gate = -1;   // any nonzero gate, not just one
    base_pop_fallout_redirect();
    expect(flag == 1);
    BasePopFalloutFlag = saved_flag;
    BasePopFalloutGate = saved_gate;
}

int main() {
    // Sprite's constructor charges a fixed-address accounting global that is
    // only mapped inside the hybrid process. Objects embedding Sprite by value
    // are constructed throughout these tests, so bind the counter to process
    // storage for the whole run; individual tests rebind it as needed.
    static int sprite_memory_sink = 0;
    SpriteMemoryUsed = &sprite_memory_sink;

    test_auto_sound_construct();
    test_win_construct();
    test_palette_get_rgbquad();
    test_buffer_construct();
    test_graphic_win_construct();
    test_base_button_construct();
    test_alpha_net_pid_to_idx();
    test_alpha_net_identity_lookups();
    test_in_box_edges();
    test_geometry_helpers();
    test_vector_lifecycle();
    test_vector_arithmetic();
    test_win_move();
    test_win_is_visible();
    test_sprite_construct();
    test_graphic_win_destructor();
    test_graphic_win_close();
    test_base_button_and_flat_button_lifecycle();
    test_sprite_close();
    test_buffer_get_data();
    test_buffer_free_data();
    test_string_struct_remove_all();
    test_find_font();
    test_buffer_text_line_height();
    test_win_paging();
    test_scroll_close();
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
    // test_button_group_add() is deliberately called last, not adjacent to
    // test_button_group_lifecycle() where it logically belongs. Placing any
    // extra call - even an empty one, even a second call to an
    // already-passing existing test - immediately before
    // test_base_pop_string_font() reproducibly crashes the whole suite
    // (confirmed by duplicating test_button_group_lifecycle() itself with no
    // other change). This is a pre-existing stack/Wine-environment fragility
    // around that call site, not a bug in add() or in this test; moving the
    // call site to the end of main() avoids it without masking it.
    test_button_group_add();
    test_dialog_font_and_color_setters();
    test_pull_down_destructor();
    test_base_button_default_setters();
    test_buffer_hdc_protocol();
    test_buffer_sync_to_palette();
    test_buffer_text_height();
    test_win_is_dialog_focus();
    test_buffer_set_clip();
    test_base_button_text_colors();
    test_default_font_setters();
    test_win_shared_hdc();
    test_sprite_draw_origin();
    test_win_set_cursor();
    test_buffer_text_width();
    test_alpha_net_close();
    test_win_clear_bubble_text();
    test_constant_return_stubs();
    test_base_pop_default_colors();
    test_win_set_def_focus();
    test_base_pop_instance_colors();
    test_guarded_store_recoveries();
    test_win_client_to_screen();
    test_status_win_set_loc();
    return failures == 0 ? 0 : 1;
}
