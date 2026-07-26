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
#include "../src/atexit_thunks.h"
#include "../src/fx.h"
#include "../src/battlewin.h"
#include "../src/councwin.h"
#include "../src/credits.h"
#include "../src/diplopop.h"
#include "../src/gamma.h"
#include "../src/messagewin.h"
#include "../src/newtechwin.h"
#include "../src/pickwin.h"
#include "../src/prodpicker.h"
#include "../src/texture.h"
#include "../src/reportwin.h"
#include "../src/setupwin.h"
#include "../src/multidebug.h"
#include "../src/tutwin.h"
#include "../src/popmenu.h"
#include "../src/popup.h"
#include "../src/netmsg.h"
#include "../src/radiobutton.h"
#include "../src/checkbox.h"
#include "../src/editgroup.h"
#include "../src/dialogs.h"
#include "../src/xpops.h"
#include "../src/playerlock.h"
#include "../src/netdaemon.h"
#include "../src/replaywin.h"
#include "../src/reportif.h"
#include "../src/dipedit.h"
#include "../src/sound.h"
#include "../src/wave_device.h"
#include "../src/spritebox.h"
#include "../src/listbox.h"
#include "../src/net_class.h"
#include "../src/squarelock.h"
#include "../src/deletionlist.h"
#include "../src/lock.h"
#include "../src/map.h"
#include "../src/game.h"
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

// Everything Scroll::close needs in order to run: its two default tables, the
// three seams it reaches through, and the two vtables it dispatches on. The
// Scroll suite below builds this inline for a standalone Scroll; wrappers that
// contain a Scroll need the same thing at their member's offset, so it lives
// here rather than being rebuilt per test.
struct ScrollCloseFixture {
    uint32_t fixed[11] = {};
    uint32_t dynamic[17] = {};
    uint32_t base_result = 0x7B3D19E5U;
    uintptr_t left_vtable[0x16C / sizeof(uintptr_t)] = {};
    uintptr_t right_vtable[0x16C / sizeof(uintptr_t)] = {};

    uint32_t *saved_fixed = nullptr;
    uint32_t *saved_dynamic = nullptr;
    func_subobject_close *saved_win = nullptr;
    func_subobject_close *saved_buffer = nullptr;
    uint32_t *saved_base_default = nullptr;

    void install() {
        for (size_t index = 0; index < ARRAYSIZE(fixed); ++index) {
            fixed[index] = 0x51000000U + static_cast<uint32_t>(index) * 0x010203U;
        }
        for (size_t index = 0; index < ARRAYSIZE(dynamic); ++index) {
            dynamic[index] = 0xA1000000U + static_cast<uint32_t>(index) * 0x010101U;
        }
        left_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_left_probe);
        right_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_right_probe);
        saved_fixed = ScrollCloseStaticDefaults;
        saved_dynamic = ScrollCloseDynamicDefaults;
        saved_win = WinOriginalClose;
        saved_buffer = BufferSubobjectClose;
        saved_base_default = GraphicWinFieldA0CDefault;
        ScrollCloseStaticDefaults = fixed;
        ScrollCloseDynamicDefaults = dynamic;
        WinOriginalClose = scroll_close_win_probe;
        BufferSubobjectClose = scroll_close_buffer_probe;
        GraphicWinFieldA0CDefault = &base_result;
    }

    // Point the object at this fixture's vtables and set the two fields the
    // close path reads: 0xA08 is followed as a pointer when non-null, so it
    // has to be cleared, and 0xA10 is what the probes check they can see.
    void prepare(uint8_t *scroll) {
        uintptr_t *left = left_vtable;
        uintptr_t *right = right_vtable;
        std::memcpy(scroll + 0xAAC, &left, sizeof(left));
        std::memcpy(scroll + 0x15F8, &right, sizeof(right));
        const uint32_t zero = 0;
        const uint32_t marker = 0x13579BDFU;
        std::memcpy(scroll + 0xA08, &zero, sizeof(zero));
        std::memcpy(scroll + 0xA10, &marker, sizeof(marker));
        scroll_close_probe_state = {scroll, fixed, dynamic, 0};
    }

    void restore() {
        ScrollCloseStaticDefaults = saved_fixed;
        ScrollCloseDynamicDefaults = saved_dynamic;
        WinOriginalClose = saved_win;
        BufferSubobjectClose = saved_buffer;
        GraphicWinFieldA0CDefault = saved_base_default;
    }
};

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

void test_field_store_clears() {
    // Three recoveries that zero specific fields. As with set_loc, the point
    // is which bytes move: each is checked at the absolute offset the original
    // writes, with every other byte compared against its seed, so a field
    // placed even one byte off fails both ways.
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };
    auto read8 = [](const std::vector<uint8_t> &s, size_t off) {
        return s[16 + off];
    };

    std::vector<uint8_t> md(sizeof(MultiDebug) + 32), md_want(md.size());
    auto *debug = reinterpret_cast<MultiDebug *>(md.data() + 16);
    seed_storage(md.data(), md_want.data(), md.size());
    std::memcpy(md_want.data(), md.data(), md.size());
    debug->close();
    expect(read32(md, 0xA3C) == 0);
    std::memcpy(md_want.data() + 16 + 0xA3C, md.data() + 16 + 0xA3C, 4);
    expect_storage_bytes(md.data(), md_want.data(), md.size());
    multi_debug_close_redirect(debug, nullptr);

    std::vector<uint8_t> dl(sizeof(Datalink) + 32), dl_want(dl.size());
    auto *link = reinterpret_cast<Datalink *>(dl.data() + 16);
    seed_storage(dl.data(), dl_want.data(), dl.size());
    std::memcpy(dl_want.data(), dl.data(), dl.size());
    link->close();
    expect(read32(dl, 0x29E0) == 0);
    expect(read32(dl, 0x2A34) == 0);
    expect(read32(dl, 0x2A38) == 0);
    std::memcpy(dl_want.data() + 16 + 0x29E0, dl.data() + 16 + 0x29E0, 4);
    std::memcpy(dl_want.data() + 16 + 0x2A34, dl.data() + 16 + 0x2A34, 8);
    expect_storage_bytes(dl.data(), dl_want.data(), dl.size());
    datalink_close_redirect(link, nullptr);

    // Four of these five are single bytes, so a field widened to a dword by
    // mistake would clear three neighbours the original leaves alone.
    std::vector<uint8_t> fw(sizeof(FileWin) + 32), fw_want(fw.size());
    auto *files = reinterpret_cast<FileWin *>(fw.data() + 16);
    seed_storage(fw.data(), fw_want.data(), fw.size());
    std::memcpy(fw_want.data(), fw.data(), fw.size());
    files->UNK1();
    expect(read8(fw, 0x208) == 0);
    expect(read8(fw, 0x30C) == 0);
    expect(read8(fw, 0x410) == 0);
    expect(read8(fw, 0x531) == 0);
    expect(read32(fw, 0x514) == 0);
    for (size_t offset : {0x208u, 0x30Cu, 0x410u, 0x531u}) {
        fw_want[16 + offset] = fw[16 + offset];
    }
    std::memcpy(fw_want.data() + 16 + 0x514, fw.data() + 16 + 0x514, 4);
    expect_storage_bytes(fw.data(), fw_want.data(), fw.size());
    file_win_unk1_redirect(files, nullptr);
}

void test_field_store_writes() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };
    auto keep = [](std::vector<uint8_t> &want, const std::vector<uint8_t> &got,
                   std::initializer_list<size_t> offsets) {
        for (size_t off : offsets) {
            std::memcpy(want.data() + 16 + off, got.data() + 16 + off, 4);
        }
    };

    // Caviar's fields were carved out of an opaque span that the class's
    // pinned 0x13D0 protects: if the carving shifted anything the build fails
    // before this runs. What is left to check is that each value lands where
    // the original puts it.
    std::vector<uint8_t> cv(sizeof(Caviar) + 32), cv_want(cv.size());
    auto *caviar = reinterpret_cast<Caviar *>(cv.data() + 16);
    seed_storage(cv.data(), cv_want.data(), cv.size());
    std::memcpy(cv_want.data(), cv.data(), cv.size());
    caviar->UNK10(INT_MIN, -1, INT_MAX);
    caviar->UNK8(0x5A5A5A5A);
    expect(read32(cv, 0x2C) == INT_MIN);
    expect(read32(cv, 0x30) == -1);
    expect(read32(cv, 0x34) == INT_MAX);
    expect(read32(cv, 0x108) == 0x5A5A5A5A);
    keep(cv_want, cv, {0x2C, 0x30, 0x34, 0x108});
    expect_storage_bytes(cv.data(), cv_want.data(), cv.size());
    caviar_unk10_redirect(caviar, nullptr, 1, 2, 3);
    caviar_unk8_redirect(caviar, nullptr, 4);
    expect(read32(cv, 0x2C) == 1);
    expect(read32(cv, 0x30) == 2);
    expect(read32(cv, 0x34) == 3);
    expect(read32(cv, 0x108) == 4);

    // TutWin::UNK1 splits nine fields between zero and -1, which is the part
    // a transcription most easily gets backwards, and clears a shared marker
    // that lives outside the object entirely.
    std::vector<uint8_t> tw(sizeof(TutWin) + 32), tw_want(tw.size());
    auto *tutorial = reinterpret_cast<TutWin *>(tw.data() + 16);
    seed_storage(tw.data(), tw_want.data(), tw.size());
    std::memcpy(tw_want.data(), tw.data(), tw.size());
    uint32_t marker = 0xDEADBEEF;
    uint32_t *const saved_marker = TutWinShownFlag;
    TutWinShownFlag = &marker;
    tutorial->UNK1();
    expect(marker == 0);
    expect(read32(tw, 0x537C) == -1);
    expect(read32(tw, 0x5380) == -1);
    expect(read32(tw, 0x539C) == -1);
    expect(read32(tw, 0x53AC) == -1);
    expect(read32(tw, 0x53A4) == 0);
    expect(read32(tw, 0x53A8) == 0);
    expect(read32(tw, 0x53B8) == 0);
    expect(read32(tw, 0x53C4) == 0);
    expect(read32(tw, 0x53D4) == 0);
    keep(tw_want, tw, {0x537Cu, 0x5380u, 0x539Cu, 0x53A4u, 0x53A8u,
                       0x53ACu, 0x53B8u, 0x53C4u, 0x53D4u});
    expect_storage_bytes(tw.data(), tw_want.data(), tw.size());

    seed_storage(tw.data(), tw_want.data(), tw.size());
    std::memcpy(tw_want.data(), tw.data(), tw.size());
    tutorial->UNK3(0x11223344);
    expect(read32(tw, 0x53D4) == 0x11223344);
    keep(tw_want, tw, {0x53D4u});
    expect_storage_bytes(tw.data(), tw_want.data(), tw.size());
    tut_win_unk3_redirect(tutorial, nullptr, -9);
    expect(read32(tw, 0x53D4) == -9);
    marker = 1;
    tut_win_unk1_redirect(tutorial, nullptr);
    expect(marker == 0);
    TutWinShownFlag = saved_marker;
}

namespace {
ButtonGroup *g_observed_group = nullptr;
int g_observed_button = 0;
int g_observed_flag_during_call = -1;
int g_click_calls = 0;
int g_click_result = 0;

int __thiscall observe_button_click(ButtonGroup *self, int button_id) {
    g_observed_group = self;
    g_observed_button = button_id;
    // The suppression flag must still be set while the click runs - clearing
    // it before the call rather than after would be invisible to a test that
    // only looked at the object afterwards.
    std::memcpy(&g_observed_flag_during_call,
                reinterpret_cast<const uint8_t *>(self) + 0x90, sizeof(int));
    ++g_click_calls;
    return g_click_result;
}
}  // namespace

void test_button_group_set() {
    // The highest fan-in unrecovered function in the image at 35 callers.
    // button_click itself is 471 bytes of virtual dispatch and stays original
    // for now, reached through a rebindable seam that this test replaces.
    auto *const saved = ButtonGroupOriginalButtonClick;
    ButtonGroupOriginalButtonClick = &observe_button_click;

    std::vector<uint8_t> storage(sizeof(ButtonGroup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *group = reinterpret_cast<ButtonGroup *>(storage.data() + 16);
    auto flag = [&] {
        int value = 0;
        std::memcpy(&value, storage.data() + 16 + 0x90, sizeof(value));
        return value;
    };

    // notify == 0 suppresses: the flag is raised for the duration and cleared.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_click_calls = 0;
    g_click_result = 0x1234;
    expect(group->set(7, 0) == 0x1234);
    expect(g_click_calls == 1);
    expect(g_observed_group == group);
    expect(g_observed_button == 7);
    expect(g_observed_flag_during_call == 1);
    expect(flag() == 0);
    std::memcpy(expected.data() + 16 + 0x90, storage.data() + 16 + 0x90, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // notify != 0 does not raise it, but still clears it afterwards.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_click_calls = 0;
    g_click_result = -1;
    expect(group->set(INT_MIN, 1) == -1);
    expect(g_click_calls == 1);
    expect(g_observed_button == INT_MIN);
    expect(g_observed_flag_during_call != 1);
    expect(flag() == 0);
    std::memcpy(expected.data() + 16 + 0x90, storage.data() + 16 + 0x90, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The redirect must behave identically, including passing the result back.
    g_click_calls = 0;
    g_click_result = 99;
    expect(button_group_set_redirect(group, nullptr, 3, 0) == 99);
    expect(g_click_calls == 1);
    expect(g_observed_flag_during_call == 1);
    expect(flag() == 0);

    ButtonGroupOriginalButtonClick = saved;
}

namespace {
BasePop *g_inited_base_pop = nullptr;
int g_init_a = -1;
long g_init_b = -1;
int g_base_pop_init_result = 0;
int __thiscall observe_base_pop_init(BasePop *self, int a, long b) {
    g_inited_base_pop = self;
    g_init_a = a;
    g_init_b = b;
    return g_base_pop_init_result;
}

InfoWin *g_timer_self = nullptr;
int g_timer_arg = 0;
int g_timer_calls = 0;
int g_timer_flag_during_call = -1;
void __thiscall observe_timer_proc(InfoWin *self, int arg) {
    g_timer_self = self;
    g_timer_arg = arg;
    std::memcpy(&g_timer_flag_during_call,
                reinterpret_cast<const uint8_t *>(self) + 0xA8, sizeof(int));
    ++g_timer_calls;
}
}  // namespace

namespace {
int g_wave_play_dev_calls;
void *g_wave_play_dev_self;
int g_wave_play_dev_ret;
int __thiscall observe_wave_device_is_playing(void *self) {
    g_wave_play_dev_self = self;
    ++g_wave_play_dev_calls;
    return g_wave_play_dev_ret;
}
int g_wave_time_calls;
DWORD g_wave_time_value;
DWORD __stdcall observe_wave_time_get_time(void) {
    ++g_wave_time_calls;
    return g_wave_time_value;
}

// Wave destructor doubles. The callbacks reach back into the fixture through
// g_wave_dtor_obj so they can witness which vtable was installed at call time
// and, for the release hook, re-link the wave to make the inlined base
// destructor's normally-dead unlink reachable.
uint8_t *g_wave_dtor_obj;
void *g_wave_dtor_pull_dev;
Wave *g_wave_dtor_pull_wave;
int g_wave_dtor_pull_calls;
uint32_t g_wave_dtor_pull_seen_vtable;
int __thiscall observe_wave_pull_from_group(void *device, Wave *wave) {
    g_wave_dtor_pull_dev = device;
    g_wave_dtor_pull_wave = wave;
    ++g_wave_dtor_pull_calls;
    std::memcpy(&g_wave_dtor_pull_seen_vtable, g_wave_dtor_obj, 4);
    return 3;
}
std::vector<void *> g_wave_dtor_deletes;
std::vector<void *> g_wave_dtor_delete_seen_slot;
uint32_t *g_wave_delete_watch;
uint32_t g_wave_delete_watch_seen;
void __cdecl observe_wave_operator_delete(void *block) {
    g_wave_dtor_deletes.push_back(block);
    // The original clears the buffer slot only AFTER the free, so at call
    // time the slot still names the block being freed - witness it.
    void *slot;
    std::memcpy(&slot, g_wave_dtor_obj + 0x4C, 4);
    g_wave_dtor_delete_seen_slot.push_back(slot);
    if (g_wave_delete_watch) {
        g_wave_delete_watch_seen = *g_wave_delete_watch;
    }
}
void *g_wave_playm_dev_self;
int g_wave_playm_calls;
int g_wave_playm_arg;
int g_wave_playm_ret;
int __thiscall observe_wave_device_play(void *self, int arg) {
    g_wave_playm_dev_self = self;
    g_wave_playm_arg = arg;
    ++g_wave_playm_calls;
    return g_wave_playm_ret;
}
// Shared observers for the wrapped-device forwarder family: one per arity,
// armed one vtable slot at a time so an off-slot dispatch faults on null.
void *g_wave_fam_self;
uint32_t g_wave_fam_args[3];
int g_wave_fam_calls;
int g_wave_fam_ret;
int __thiscall observe_wave_dev0(void *self) {
    g_wave_fam_self = self;
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}
std::vector<uint32_t> g_wave_fam_arg_log;
int __thiscall observe_wave_dev1(void *self, uint32_t a1) {
    g_wave_fam_self = self;
    g_wave_fam_args[0] = a1;
    g_wave_fam_arg_log.push_back(a1);
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}
int __thiscall observe_wave_dev3(void *self, uint32_t a1, uint32_t a2,
                                 uint32_t a3) {
    g_wave_fam_self = self;
    g_wave_fam_args[0] = a1;
    g_wave_fam_args[1] = a2;
    g_wave_fam_args[2] = a3;
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}
// Doubles for set_volume/set_fname/play: the game-heap allocator, the
// device singleton's group-disabled query, the original no-argument load
// (which may wrap a device as its observable effect), and the wave's own
// vtable slots 0x40/0x80.
unsigned g_wave_new_size;
int g_wave_new_calls;
char g_wave_new_arena[64];
bool g_wave_new_fails;
void *__cdecl observe_wave_operator_new(unsigned int size) {
    g_wave_new_size = size;
    ++g_wave_new_calls;
    return g_wave_new_fails ? nullptr : g_wave_new_arena;
}
void *g_wave_gd_dev;
uint32_t g_wave_gd_slot;
int g_wave_gd_calls;
int g_wave_gd_ret;
int __thiscall observe_wave_group_disabled(void *device, uint32_t slot) {
    g_wave_gd_dev = device;
    g_wave_gd_slot = slot;
    ++g_wave_gd_calls;
    return g_wave_gd_ret;
}
int g_wave_oload_calls;
void *g_wave_oload_installs;  // written into the wave's device slot when set
int __thiscall observe_wave_original_load(Wave *wave) {
    ++g_wave_oload_calls;
    std::memcpy(reinterpret_cast<uint8_t *>(wave) + 0x3C,
                &g_wave_oload_installs, 4);
    return 7;
}
Wave *g_wave_own40_self;
uint32_t g_wave_own40_arg;
int g_wave_own40_calls;
void __thiscall observe_wave_own_slot40(Wave *self, uint32_t arg) {
    g_wave_own40_self = self;
    g_wave_own40_arg = arg;
    ++g_wave_own40_calls;
}
Wave *g_wave_own80_self;
int g_wave_own80_calls;
void *g_wave_own80_seen_device;
uint32_t g_wave_own80_seen_stamp;
void __thiscall observe_wave_own_slot80(Wave *self) {
    g_wave_own80_self = self;
    ++g_wave_own80_calls;
    // The original stamps the start time and still holds the device when its
    // own slot 0x80 runs; witness both so the ordering is pinned.
    std::memcpy(&g_wave_own80_seen_device,
                reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
    std::memcpy(&g_wave_own80_seen_stamp,
                reinterpret_cast<uint8_t *>(self) + 0x64, 4);
}
// load() doubles: the device-creation hook, the base Sound::load, and the
// wave's own vtable slot 0x58.
void **g_wave_create_slot_arg;
const char *g_wave_create_name;
int g_wave_create_mode;
int g_wave_create_calls;
int g_wave_create_ret;
void *g_wave_create_installs;
int __cdecl observe_wave_device_create(void **slot, const char *name,
                                       int mode) {
    g_wave_create_slot_arg = slot;
    g_wave_create_name = name;
    g_wave_create_mode = mode;
    ++g_wave_create_calls;
    if (g_wave_create_installs) {
        *slot = g_wave_create_installs;
    }
    return g_wave_create_ret;
}
Wave *g_wave_sload_wave;
const char *g_wave_sload_name;
int g_wave_sload_calls;
int g_wave_sload_ret;
int __thiscall observe_wave_sound_load(Wave *wave, const char *name) {
    g_wave_sload_wave = wave;
    g_wave_sload_name = name;
    ++g_wave_sload_calls;
    return g_wave_sload_ret;
}
int g_wave_own58_calls;
int g_wave_own58_ret;
int __thiscall observe_wave_own_slot58(Wave *) {
    ++g_wave_own58_calls;
    return g_wave_own58_ret;
}
int g_wave_own70_calls;
int g_wave_own70_ret;
void *g_wave_own70_swaps_vtbl;
int __thiscall observe_wave_own_slot70(Wave *self) {
    ++g_wave_own70_calls;
    if (g_wave_own70_swaps_vtbl) {
        // dyna_load captures the device vtable BEFORE this query runs; a
        // swapped table here proves the capture happened first.
        void *device;
        std::memcpy(&device, reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
        std::memcpy(device, &g_wave_own70_swaps_vtbl, 4);
    }
    return g_wave_own70_ret;
}
Wave *g_wave_own48_self;
int g_wave_own48_arg;
int g_wave_own48_calls;
uint32_t g_wave_own48_seen_flags;
void __thiscall observe_wave_own_slot48(Wave *self, int arg) {
    g_wave_own48_self = self;
    g_wave_own48_arg = arg;
    ++g_wave_own48_calls;
    std::memcpy(&g_wave_own48_seen_flags,
                reinterpret_cast<uint8_t *>(self) + 0x54, 4);
}
void *g_wave_ginsert_head;
Wave *g_wave_ginsert_wave;
int g_wave_ginsert_calls;
uint32_t g_wave_ginsert_seen_slot;
void __thiscall observe_wave_group_insert(void *group_head, Wave *wave) {
    g_wave_ginsert_head = group_head;
    g_wave_ginsert_wave = wave;
    ++g_wave_ginsert_calls;
    // add_to_group stores the wave's slot AFTER the insert; witness it.
    std::memcpy(&g_wave_ginsert_seen_slot,
                reinterpret_cast<uint8_t *>(wave) + 0x68, 4);
}
Sound *g_sound_own54_self;
uint32_t g_sound_own54_arg;
int g_sound_own54_calls;
int g_sound_own54_ret;
int __thiscall observe_sound_own_slot54(Sound *self, uint32_t arg) {
    g_sound_own54_self = self;
    g_sound_own54_arg = arg;
    ++g_sound_own54_calls;
    return g_sound_own54_ret;
}
int g_wave_stype_calls;
Wave *g_wave_stype_wave;
uint32_t g_wave_stype_type;
uint32_t g_wave_stype_seen_vtable;
uint32_t g_wave_stype_seen_f40;
uint8_t g_wave_stype_seen_f54;
uint32_t g_wave_stype_seen_pitch;
void __thiscall observe_wave_set_type(Wave *wave, uint32_t type) {
    g_wave_stype_wave = wave;
    g_wave_stype_type = type;
    ++g_wave_stype_calls;
    const uint8_t *raw = reinterpret_cast<const uint8_t *>(wave);
    std::memcpy(&g_wave_stype_seen_vtable, raw, 4);
    std::memcpy(&g_wave_stype_seen_f40, raw + 0x40, 4);
    g_wave_stype_seen_f54 = raw[0x54];
    std::memcpy(&g_wave_stype_seen_pitch, raw + 0x58, 4);
}
int g_wave_own7C_calls;
uint32_t g_wave_own7C_seen_flags;
void *g_wave_own7C_restores_vtbl;
void __thiscall observe_wave_own_slot7C(Wave *self) {
    ++g_wave_own7C_calls;
    // reload sets the loaded bit BEFORE this callback; witness the word.
    std::memcpy(&g_wave_own7C_seen_flags,
                reinterpret_cast<uint8_t *>(self) + 0x40, 4);
    if (g_wave_own7C_restores_vtbl) {
        // Undo a poison swap installed by the 0x70 observer: init dispatches
        // through the LIVE device vtable after this callback, so the table
        // must be healthy again by then.
        void *device;
        std::memcpy(&device, reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
        std::memcpy(device, &g_wave_own7C_restores_vtbl, 4);
    }
}
void *g_wave_dtor_release_dev;
int g_wave_dtor_release_calls;
uint32_t g_wave_dtor_release_seen_vtable;
bool g_wave_dtor_release_relinks;
void *g_wave_dtor_release_relink_prev;
void *g_wave_dtor_release_relink_next;
void __cdecl observe_wave_release(void *device) {
    g_wave_dtor_release_dev = device;
    ++g_wave_dtor_release_calls;
    std::memcpy(&g_wave_dtor_release_seen_vtable, g_wave_dtor_obj, 4);
    if (g_wave_dtor_release_relinks) {
        uint32_t flags;
        std::memcpy(&flags, g_wave_dtor_obj + 0x40, 4);
        flags |= 2;
        std::memcpy(g_wave_dtor_obj + 0x40, &flags, 4);
        std::memcpy(g_wave_dtor_obj + 0x44, &g_wave_dtor_release_relink_prev,
                    4);
        std::memcpy(g_wave_dtor_obj + 0x48, &g_wave_dtor_release_relink_next,
                    4);
    }
}
}  // namespace

void test_wave_is_playing() {
    // is_playing lets a wrapped device at 0x3C answer for itself through the
    // device's own vtable slot 0x5C. With no device it times the wave against
    // the clock: bit 4 of the flag byte at 0x54 arms it, a zero start stamp at
    // 0x64 means it never began, and the stored length at 0x60 is compared
    // UNSIGNED against the elapsed time.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    auto *wave = reinterpret_cast<Wave *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;

    // Poison the neighbouring slots: an off-by-one dispatch faults rather than
    // silently landing on another observer.
    void *dev_vtable[32] = {};
    dev_vtable[0x5C / 4] = reinterpret_cast<void *>(&observe_wave_device_is_playing);
    dev_vtable[0x58 / 4] = nullptr;
    dev_vtable[0x60 / 4] = nullptr;
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    func_time_get_time *time_fn = &observe_wave_time_get_time;
    func_time_get_time **const saved_slot = WaveTimeGetTimeSlot;
    WaveTimeGetTimeSlot = &time_fn;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // A wrapped device answers, with itself as the receiver, and its result is
    // returned verbatim rather than normalised to 0/1.
    set_device(&fake_dev);
    obj[0x54] = 0;
    set32(0x60, 0);
    set32(0x64, 0);
    g_wave_play_dev_calls = g_wave_time_calls = 0;
    g_wave_play_dev_ret = 0x2A;
    expect(wave->is_playing() == 0x2A);
    expect(g_wave_play_dev_calls == 1 && g_wave_play_dev_self == &fake_dev);
    expect(g_wave_time_calls == 0);          // the clock is not consulted

    // The device wins over every clock input: armed flag, live stamp, long
    // length, and it still returns the device's own answer.
    obj[0x54] = 0x10;
    set32(0x60, 1000);
    set32(0x64, 1);
    g_wave_play_dev_calls = g_wave_time_calls = 0;
    g_wave_play_dev_ret = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_play_dev_calls == 1);
    expect(g_wave_time_calls == 0);

    // No device and the arming bit clear: 0, and the clock is never read.
    set_device(nullptr);
    obj[0x54] = 0xEF;                        // every bit but 4
    set32(0x60, 1000);
    set32(0x64, 1);
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_time_calls == 0);

    // Bit 4 alone arms it; the other bits are irrelevant either way.
    obj[0x54] = 0x10;
    set32(0x64, 0);                          // never started
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_time_calls == 0);          // the zero stamp short-circuits

    // Started and still inside the length: playing.
    obj[0x54] = 0xFF;
    set32(0x60, 1000);
    set32(0x64, 5000);
    g_wave_time_value = 5999;                // elapsed 999
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 1);
    expect(g_wave_time_calls == 1);

    // The boundary is `jbe`: elapsed == length is finished, one less is not.
    g_wave_time_value = 6000;                // elapsed 1000 == length
    expect(wave->is_playing() == 0);
    g_wave_time_value = 5000;                // elapsed 0
    expect(wave->is_playing() == 1);

    // The comparison is unsigned, so a negative stored length reads as an
    // enormous one rather than as already finished.
    set32(0x60, 0xFFFFFFFFu);
    g_wave_time_value = 5000u + 0x7FFFFFFFu;
    expect(wave->is_playing() == 1);

    // The subtraction is unsigned too: a clock that has wrapped below the
    // stamp yields a huge elapsed, not a negative one.
    set32(0x60, 1000);
    set32(0x64, 0xFFFFFF00u);
    g_wave_time_value = 4;                   // wrapped; elapsed 0x104
    expect(wave->is_playing() == 1);
    g_wave_time_value = 0xFFFFFEFFu;         // one tick before the stamp
    expect(wave->is_playing() == 0);         // elapsed 0xFFFFFFFF

    // Nothing above writes to the object. Seeding comes first: it fills every
    // byte, including the device pointer at 0x3C, so the fields the clock path
    // needs have to be written after it and snapshotted after that.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_device(nullptr);
    obj[0x54] = 0x10;
    set32(0x60, 1000);
    set32(0x64, 5000);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_time_value = 5500;
    wave->is_playing();
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entry, on both branches.
    set32(0x60, 1000);
    set32(0x64, 5000);
    obj[0x54] = 0x10;
    g_wave_time_value = 5500;
    expect(wave_is_playing_redirect(wave, nullptr) == 1);
    set_device(&fake_dev);
    g_wave_play_dev_ret = 0x77;
    expect(wave_is_playing_redirect(wave, nullptr) == 0x77);

    WaveTimeGetTimeSlot = saved_slot;
}

void test_wave_play() {
    // play is the wrapped-device forwarder shape: the device at 0x3C answers
    // through its own vtable slot 0x94 as the receiver, its result returned
    // verbatim; with no device the answer is a fixed 0x14.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    // Poisoned neighbours: an off-by-one dispatch faults rather than landing
    // on another observer.
    void *dev_vtable[64] = {};
    dev_vtable[0x94 / 4] = reinterpret_cast<void *>(&observe_wave_device_play);
    dev_vtable[0x90 / 4] = nullptr;
    dev_vtable[0x98 / 4] = nullptr;
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    seed_storage(storage.data(), expected.data(), storage.size());
    void *dev = &fake_dev;
    std::memcpy(obj + 0x3C, &dev, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());

    g_wave_playm_calls = 0;
    g_wave_playm_ret = 0x5A17;
    expect(wave->play(0x1234) == 0x5A17);
    expect(g_wave_playm_calls == 1);
    expect(g_wave_playm_dev_self == &fake_dev);
    expect(g_wave_playm_arg == 0x1234);

    // The result is verbatim, not normalised.
    g_wave_playm_ret = 0;
    expect(wave->play(-7) == 0);
    expect(g_wave_playm_arg == -7);

    // Nothing writes the object on the device path.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // No device: the fixed answer, no dispatch, still no writes.
    dev = nullptr;
    std::memcpy(obj + 0x3C, &dev, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_playm_calls = 0;
    expect(wave->play(77) == 0x14);
    expect(g_wave_playm_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entry, both paths.
    expect(wave_play_redirect(wave, nullptr, 5) == 0x14);
    dev = &fake_dev;
    std::memcpy(obj + 0x3C, &dev, 4);
    g_wave_playm_ret = 9;
    expect(wave_play_redirect(wave, nullptr, 6) == 9);
    expect(g_wave_playm_arg == 6);
}

void test_wave_device_forwarders() {
    // The rest of the wrapped-device family: each method dispatches through
    // one device vtable slot with the device as receiver, or answers a fixed
    // default with no device. Slots and defaults vary per method; nothing
    // here writes the object except set_reverb_mix (0x5C) and set_attrib
    // (0x30 and the 0x54 flag byte).
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
    };
    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, 4); };
    auto bits_of = [](float f) {
        uint32_t b;
        std::memcpy(&b, &f, 4);
        return b;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    set_device(&fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // --- the pure forwarders, device path: result verbatim, receiver the
    // device, argument dwords passed through ---
    arm(0xC8, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1101;
    expect(wave->is_hwbuffer() == 0x1101);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);

    arm(0xB8, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1102;
    expect(wave->get_current_marker() == 0x1102);
    expect(g_wave_fam_calls == 1);

    arm(0x3C, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1103;
    expect(wave->get_game_hwnd() == 0x1103);
    expect(g_wave_fam_calls == 1);

    arm(0xBC, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1104;
    expect(wave->get_ndevices() == 0x1104);
    expect(g_wave_fam_calls == 1);

    arm(0xDC, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1105;
    expect(wave->is_3d() == 0x1105);
    expect(g_wave_fam_calls == 1);

    arm(0xB4, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1106;
    expect(wave->get_time(0xCAFE) == 0x1106);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0xCAFE);

    arm(0xD0, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1107;
    expect(wave->set_xpos(2.5f) == 0x1107);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(2.5f));

    arm(0xD4, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1108;
    expect(wave->set_ypos(-3.25f) == 0x1108);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(-3.25f));

    arm(0xD8, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1109;
    expect(wave->set_zpos(0.5f) == 0x1109);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(0.5f));

    arm(0xCC, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x110A;
    expect(wave->set_position3d(1.0f, 2.0f, 3.0f) == 0x110A);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(1.0f) &&
           g_wave_fam_args[1] == bits_of(2.0f) &&
           g_wave_fam_args[2] == bits_of(3.0f));

    char descr_buf[4] = {'Z', 'Z', 'Z', 'Z'};
    arm(0xC0, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x110B;
    expect(wave->get_device_description(descr_buf, 0x22, 0x33) == 0x110B);
    expect(g_wave_fam_calls == 1 &&
           g_wave_fam_args[0] == reinterpret_cast<uintptr_t>(descr_buf) &&
           g_wave_fam_args[1] == 0x22 && g_wave_fam_args[2] == 0x33);
    expect(descr_buf[0] == 'Z');  // the device path never touches the buffer

    // None of the above wrote the object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- the pure forwarders, no-device defaults ---
    set_device(nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_fam_calls = 0;
    expect(wave->is_hwbuffer() == 0);
    expect(wave->get_current_marker() == -1);
    expect(wave->get_game_hwnd() == 0);
    expect(wave->get_ndevices() == 0);
    expect(wave->is_3d() == 0);
    expect(wave->get_time(1) == 0);
    expect(wave->set_xpos(1.0f) == 0x14);
    expect(wave->set_ypos(1.0f) == 0x14);
    expect(wave->set_zpos(1.0f) == 0x14);
    expect(wave->set_position3d(1.0f, 1.0f, 1.0f) == 0x14);
    expect(g_wave_fam_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // get_device_description with no device: the buffer is terminated - one
    // byte only - but only when the third argument is nonzero.
    std::memcpy(descr_buf, "ZZZ", 4);
    expect(wave->get_device_description(descr_buf, 5, 0) == 1);
    expect(descr_buf[0] == 'Z');
    expect(wave->get_device_description(descr_buf, 5, 9) == 1);
    expect(descr_buf[0] == '\0' && descr_buf[1] == 'Z');
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- UNK1: chases the +0x3C chain from the device until null, answers 1,
    // ignores its argument ---
    expect(wave->UNK1(0x7777) == 1);  // no device: immediate
    uint8_t node_a[0x40] = {};
    uint8_t node_b[0x40] = {};
    void *link = node_b;
    std::memcpy(node_a + 0x3C, &link, 4);  // a -> b -> null
    // Poison offset 0 of both nodes with self-loops: a walk that follows the
    // wrong offset never terminates instead of luckily reading a null.
    void *self_a = node_a;
    void *self_b = node_b;
    std::memcpy(node_a, &self_a, 4);
    std::memcpy(node_b, &self_b, 4);
    set_device(node_a);
    expect(wave->UNK1(-5) == 1);
    set_device(nullptr);

    // --- set_reverb_mix: stores the value at 0x5C on BOTH paths ---
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->set_reverb_mix(7.75f) == 0x14);  // no device
    {
        const uint32_t bits = bits_of(7.75f);
        std::memcpy(expected.data() + 16 + 0x5C, &bits, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    set_device(&fake_dev);
    arm(0xE0, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x110C;
    expect(wave->set_reverb_mix(-1.5f) == 0x110C);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(-1.5f));
    {
        const uint32_t bits = bits_of(-1.5f);
        std::memcpy(expected.data() + 16 + 0x5C, &bits, 4);
        void *d = &fake_dev;
        std::memcpy(expected.data() + 16 + 0x3C, &d, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- set_attrib: per-bit mapping onto the 0x54 flag byte ---
    const struct { uint32_t a1; uint8_t fl; } set_map[] = {
        {0x001, 0x01}, {0x004, 0x02}, {0x040, 0x08},
        {0x080, 0x10}, {0x010, 0x04}, {0x100, 0x20},
        // bit 2 suppresses the bit-4 and bit-8 mappings
        {0x014, 0x02}, {0x104, 0x02},
    };
    set_device(nullptr);
    for (const auto &c : set_map) {
        obj[0x54] = 0;
        wave->set_attrib(c.a1);
        expect(obj[0x54] == c.fl);
    }
    // Bits already set are never cleared, and bit 1 arms the dword at 0x30.
    obj[0x54] = 0xC0;
    uint32_t before_30;
    std::memcpy(&before_30, obj + 0x30, 4);
    wave->set_attrib(2);
    expect(obj[0x54] == 0xC0);
    uint32_t after_30;
    std::memcpy(&after_30, obj + 0x30, 4);
    expect(after_30 == 1 && before_30 != 1);
    // The device hears the raw mask through slot 0x6C.
    set_device(&fake_dev);
    arm(0x6C, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_attrib(0x181);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x181);
    expect(obj[0x54] == (0xC0 | 0x01 | 0x10 | 0x20));

    // --- get_attrib: inverse mapping, OR-ed over the device's answer ---
    const struct { uint8_t fl; int out; } get_map[] = {
        {0x01, 0x001}, {0x02, 0x004}, {0x04, 0x010},
        {0x08, 0x040}, {0x10, 0x080}, {0x20, 0x100},
    };
    set_device(nullptr);
    std::memset(obj + 0x30, 0, 4);
    for (const auto &c : get_map) {
        obj[0x54] = c.fl;
        expect(wave->get_attrib() == c.out);
    }
    obj[0x54] = 0;
    std::memcpy(obj + 0x30, "\x05\x00\x00\x00", 4);
    expect(wave->get_attrib() == 2);  // any nonzero 0x30 dword reads as bit 1
    // Device path: its answer is the base the fields OR onto.
    set_device(&fake_dev);
    arm(0x70, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x10000;
    obj[0x54] = 0x3F;
    expect(wave->get_attrib() == (0x10000 | 2 | 0x1D5));
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);

    // --- redirect entries ---
    set_device(nullptr);
    expect(wave_is_hwbuffer_redirect(wave, nullptr) == 0);
    expect(wave_get_time_redirect(wave, nullptr, 2) == 0);
    expect(wave_get_current_marker_redirect(wave, nullptr) == -1);
    expect(wave_get_game_hwnd_redirect(wave, nullptr) == 0);
    expect(wave_get_ndevices_redirect(wave, nullptr) == 0);
    expect(wave_unk1_redirect(wave, nullptr, 3) == 1);
    expect(wave_set_reverb_mix_redirect(wave, nullptr, 1.0f) == 0x14);
    expect(wave_is_3d_redirect(wave, nullptr) == 0);
    char rbuf[2] = {'Q', 'Q'};
    expect(wave_get_device_description_redirect(wave, nullptr, rbuf, 1, 1) ==
           1);
    expect(rbuf[0] == '\0');
    expect(wave_set_position3d_redirect(wave, nullptr, 1, 2, 3) == 0x14);
    expect(wave_set_xpos_redirect(wave, nullptr, 1) == 0x14);
    expect(wave_set_ypos_redirect(wave, nullptr, 1) == 0x14);
    expect(wave_set_zpos_redirect(wave, nullptr, 1) == 0x14);
    obj[0x54] = 0;
    std::memset(obj + 0x30, 0, 4);
    wave_set_attrib_redirect(wave, nullptr, 1);
    expect(obj[0x54] == 1);
    expect(wave_get_attrib_redirect(wave, nullptr) == 1);
}

void test_wave_volume_fname_play() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_new = WaveOperatorNew;
    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_gd = WaveDeviceIsGroupDisabled;
    auto *const saved_load = WaveOriginalLoad;
    uint32_t *const saved_gvol = WaveDeviceGroupVolumes;
    void *const saved_dev_global = WaveDeviceGlobal;
    func_time_get_time **const saved_time = WaveTimeGetTimeSlot;

    int fake_singleton = 0;
    func_time_get_time *time_fn = &observe_wave_time_get_time;
    uint32_t gtable[16 * 6];
    for (auto &v : gtable) v = 0xDDDDDDDDu;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveDeviceIsGroupDisabled = &observe_wave_group_disabled;
    WaveOriginalLoad = &observe_wave_original_load;
    WaveDeviceGroupVolumes = gtable;
    WaveDeviceGlobal = &fake_singleton;
    WaveTimeGetTimeSlot = &time_fn;

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm_dev = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
    };
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x40 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot40);
    own_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };

    // --- set_volume: the group-scaled level ---
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 3);
    gtable[3 * 6] = 200;
    setp(0x3C, &fake_dev);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->set_volume(0x180 | 0x7F);  // only the low seven bits survive
    expect(get32(0x04) == 127);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_fam_args[0] == 200);  // 127/127 * 200
    {
        const uint32_t vol = 127;
        std::memcpy(expected.data() + 16 + 0x04, &vol, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    gtable[3 * 6] = 100;
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(64);
    expect(get32(0x04) == 64);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 50);  // 64/127*100

    // Outside the group range the raw masked level reaches the device.
    set32(0x68, 0x10);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(90);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 90);

    // No device: the volume is still remembered, nothing is dispatched.
    setp(0x3C, nullptr);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(5);
    expect(get32(0x04) == 5 && g_wave_fam_calls == 0);

    // --- set_fname: game-heap string ownership ---
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    g_wave_new_calls = 0;
    setp(0x4C, nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->set_fname(nullptr) == 0xA);
    expect(g_wave_new_calls == 0 && g_wave_dtor_deletes.empty());
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    expect(wave->set_fname("wav/menu.wav") == 0);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "wav/menu.wav") == 0);
    expect(g_wave_dtor_deletes.empty());  // nothing to free the first time

    // A previous name goes back to the game heap first.
    char old_name[4] = {'x', 0, 0, 0};
    setp(0x4C, old_name);
    g_wave_new_calls = 0;
    expect(wave->set_fname("ok") == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == old_name);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 3);
    expect(getp(0x4C) == g_wave_new_arena &&
           std::strcmp(g_wave_new_arena, "ok") == 0);

    // --- play(): the start protocol ---
    auto reset_play = [&] {
        g_wave_gd_calls = g_wave_oload_calls = 0;
        g_wave_own40_calls = g_wave_own80_calls = 0;
        g_wave_fam_calls = 0;
        g_wave_time_calls = 0;
        g_wave_oload_installs = nullptr;
    };
    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // A: in range, "disabled" answer with a zero low byte does NOT disable;
    // unclocked wave with a device: only the device start runs.
    set32(0x68, 5);
    obj[0x54] = 0xEF;  // every bit but 4
    setp(0x3C, &fake_dev);
    set32(0x04, 77);
    arm_dev(0x1C, reinterpret_cast<void *>(&observe_wave_dev0));
    reset_play();
    g_wave_gd_ret = 0x100;
    g_wave_fam_ret = 0x2211;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->play() == 0x2211);
    expect(g_wave_gd_calls == 1 && g_wave_gd_dev == &fake_singleton &&
           g_wave_gd_slot == 5);
    expect(g_wave_own40_calls == 0);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_own80_calls == 0 && g_wave_time_calls == 0);
    expect(getp(0x3C) == &fake_dev);  // unclocked: the device is kept
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // B: a genuinely disabled group answers 0x14 before anything happens.
    reset_play();
    g_wave_gd_ret = 1;
    expect(wave->play() == 0x14);
    expect(g_wave_gd_calls == 1 && g_wave_own40_calls == 0 &&
           g_wave_fam_calls == 0 && g_wave_oload_calls == 0);

    // C: clocked wave with a device: volume replay through the wave's own
    // slot 0x40, device start, then the epilogue - timestamp, own slot 0x80,
    // device forgotten.
    obj[0x54] = 0x10;
    reset_play();
    g_wave_gd_ret = 0;
    g_wave_fam_ret = 0x3322;
    g_wave_time_value = 0xABCD1234u;
    expect(wave->play() == 0x3322);
    expect(g_wave_own40_calls == 1 && g_wave_own40_self == wave &&
           g_wave_own40_arg == 77);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_time_calls == 1 && get32(0x64) == 0xABCD1234u);
    expect(g_wave_own80_calls == 1 && g_wave_own80_self == wave);
    expect(g_wave_own80_seen_stamp == 0xABCD1234u);
    expect(g_wave_own80_seen_device == &fake_dev);
    expect(getp(0x3C) == nullptr);

    // D: out of group range (no disabled query), clocked, no device: the
    // original load runs and the device it wraps is started.
    set32(0x68, 0x10);
    reset_play();
    g_wave_oload_installs = &fake_dev;
    arm_dev(0x1C, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x4433;
    expect(wave->play() == 0x4433);
    expect(g_wave_gd_calls == 0 && g_wave_own40_calls == 0);
    expect(g_wave_oload_calls == 1);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_own80_calls == 1 && getp(0x3C) == nullptr);
    expect(g_wave_own80_seen_device == &fake_dev);

    // E: the load wraps nothing: the answer is 0, the epilogue still runs.
    reset_play();
    expect(wave->play() == 0);
    expect(g_wave_oload_calls == 1 && g_wave_fam_calls == 0);
    expect(g_wave_own80_calls == 1);

    // F: unclocked with no device: the fixed 0x14, and nothing runs at all.
    obj[0x54] = 0xEF;
    reset_play();
    expect(wave->play() == 0x14);
    expect(g_wave_gd_calls == 0 && g_wave_oload_calls == 0 &&
           g_wave_own40_calls == 0 && g_wave_own80_calls == 0 &&
           g_wave_time_calls == 0);

    // --- the scalar deleting destructor ---
    auto *const saved_pull = WaveDevicePullFromGroup;
    WaveDevicePullFromGroup = &observe_wave_pull_from_group;
    g_wave_dtor_pull_calls = 0;
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    set32(0x68, 0x10);  // no group pull
    set32(0x40, 0);     // not chained
    setp(0x4C, nullptr);
    setp(0x3C, nullptr);
    expect(wave_scalar_dtor_redirect(wave, nullptr, 0) == wave);
    expect(get32(0x00) == 0x0066E444u);  // the destructor really ran
    expect(g_wave_dtor_deletes.empty()); // mode 0: storage kept
    set32(0x68, 0x10);
    set32(0x40, 0);
    expect(wave_scalar_dtor_redirect(wave, nullptr, 3) == wave);
    expect(g_wave_dtor_deletes.size() == 1 && g_wave_dtor_deletes[0] == wave);
    expect(g_wave_dtor_pull_calls == 0);
    WaveDevicePullFromGroup = saved_pull;

    // --- redirect entries ---
    set32(0x68, 0x10);
    setp(0x3C, nullptr);
    wave_set_volume_redirect(wave, nullptr, 9);
    expect(get32(0x04) == 9);
    setp(0x4C, nullptr);
    expect(wave_set_fname_redirect(wave, nullptr, nullptr) == 0xA);
    obj[0x54] = 0xEF;
    expect(wave_play_empty_redirect(wave, nullptr) == 0x14);

    WaveOperatorNew = saved_new;
    WaveOperatorDelete = saved_delete;
    WaveDeviceIsGroupDisabled = saved_gd;
    WaveOriginalLoad = saved_load;
    WaveDeviceGroupVolumes = saved_gvol;
    WaveDeviceGlobal = saved_dev_global;
    WaveTimeGetTimeSlot = saved_time;
}

void test_wave_load_empty() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    auto *const saved_sload = SoundOriginalLoad;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 0;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;
    SoundOriginalLoad = &observe_wave_sound_load;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0xC4 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x58 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot58);
    own_vtable[0x70 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot70);
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto reset_load = [&] {
        g_wave_create_calls = g_wave_sload_calls = 0;
        g_wave_own58_calls = g_wave_fam_calls = 0;
        g_wave_own70_calls = g_wave_own7C_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
        g_wave_own58_ret = 0;
        g_wave_own70_ret = 0;
        g_wave_sload_ret = 0;
    };
    char name_buf[] = "x.wav";

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // No filename: the fixed 8, nothing runs.
    setp(0x4C, nullptr);
    reset_load();
    expect(wave->load() == 8);
    expect(g_wave_create_calls == 0 && g_wave_sload_calls == 0 &&
           g_wave_own58_calls == 0);

    // No device and a dead creation hook: 1, the hook is never entered.
    setp(0x4C, name_buf);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->load() == 1);
    expect(g_wave_create_calls == 0 && g_wave_sload_calls == 0);

    // A failing creation propagates its error before any attribute work.
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x33;
    expect(wave->load() == 0x33);
    expect(g_wave_create_calls == 1);
    expect(g_wave_create_slot_arg == reinterpret_cast<void **>(obj + 0x3C));
    expect(g_wave_create_name == reinterpret_cast<char *>(name_buf) &&
           g_wave_create_mode == 1);
    expect(g_wave_sload_calls == 0 && g_wave_own58_calls == 0);

    // Creation installs the device into 0x3C; a failing Sound::load then
    // propagates ITS error after the device heard an empty attribute mask.
    obj[0x54] = 0;
    reset_load();
    g_wave_create_installs = &fake_dev;
    g_wave_sload_ret = 0x55;
    expect(wave->load() == 0x55);
    expect(getp(0x3C) == &fake_dev);
    expect(g_wave_own58_calls == 1);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0);
    expect(g_wave_sload_calls == 1 && g_wave_sload_wave == wave &&
           g_wave_sload_name == reinterpret_cast<char *>(name_buf));
    expect(get32(0x60) != 0x7654u);  // no length query on failure

    // Success with every flag bit: attributes fold per bit (bit 5 suppresses
    // the bit-3 companion), the wave's own slot 0x58 adds bit 1, and the
    // device's slot 0xC4 answer lands in the length field.
    obj[0x54] = 0x3D;
    reset_load();
    g_wave_own58_ret = 7;
    g_wave_fam_ret = 0x7654;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->load() == 0);
    expect(g_wave_create_calls == 0);  // the device already existed
    expect(g_wave_own58_calls == 1);
    expect(g_wave_fam_calls == 2 && g_wave_fam_args[0] == 0x1D3);
    expect(g_wave_sload_calls == 1);
    expect(get32(0x60) == 0x7654u);
    {
        const uint32_t len = 0x7654;
        std::memcpy(expected.data() + 16 + 0x60, &len, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Bit 3 alone brings its companion bit 0 along.
    obj[0x54] = 0x08;
    reset_load();
    g_wave_fam_ret = 0x11;
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 0x41);

    // Bit 0 alone maps to attribute bit 0 with nothing to mask it.
    obj[0x54] = 0x01;
    reset_load();
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 1);

    // Bit 5 suppresses bit 3's companion: 0x40 and 0x100, and nothing else.
    obj[0x54] = 0x28;
    reset_load();
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 0x140);

    // --- reload(): the loaded-bit protocol ---
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x84 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);

    setp(0x4C, nullptr);
    reset_load();
    expect(wave->reload() == 8);
    setp(0x4C, name_buf);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->reload() == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x22;
    expect(wave->reload() == 0x22);
    expect(g_wave_create_calls == 1 && g_wave_fam_calls == 0);
    expect(g_wave_create_mode == 1);

    // A failing device reload propagates before the loaded-bit protocol.
    setp(0x3C, &fake_dev);
    obj[0x54] = 1;
    set32(0x40, 0xF0);
    set32(0x30, 7);
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0x66;
    expect(wave->reload() == 0x66);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3}));
    expect(g_wave_own7C_calls == 0 && get32(0x40) == 0xF0);

    // First success: the loaded bit is set before the wave's own 0x7C runs,
    // and the nonzero dword at 0x30 starts the device looping with 1.
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->reload() == 0);
    expect(get32(0x40) == 0xF1);
    expect(g_wave_own7C_calls == 1 && g_wave_own7C_seen_flags == 0xF1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3, 1}));
    {
        const uint32_t loaded_word = 0xF1;
        std::memcpy(expected.data() + 16 + 0x40, &loaded_word, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Already loaded: no bit work, no callback, no looping.
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0;
    expect(wave->reload() == 0);
    expect(g_wave_own7C_calls == 0);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3}));

    // A zero dword at 0x30 skips the looping call; with the 0x54 flag byte
    // cleared too, the attribute mask is exactly zero.
    set32(0x40, 0);
    set32(0x30, 0);
    obj[0x54] = 0;
    reset_load();
    expect(wave->reload() == 0);
    expect(g_wave_own7C_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0}));

    // --- dyna_load(): in-memory creation ---
    char data_buf[] = "DATA";
    reset_load();
    expect(wave->dyna_load(data_buf) == 0xC);  // a device already exists
    expect(g_wave_create_calls == 0);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->dyna_load(data_buf) == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x44;
    expect(wave->dyna_load(data_buf) == 0x44);
    expect(g_wave_create_name == reinterpret_cast<char *>(data_buf));
    expect(g_wave_create_mode == 1);
    expect(g_wave_own70_calls == 0);
    // Success - and the 0x70 query swaps the device's vtable for a poisoned
    // one, so a capture taken after the query would dispatch into nulls.
    void *poison_vtbl[64];
    std::memset(poison_vtbl, 0, sizeof(poison_vtbl));
    reset_load();
    g_wave_create_installs = &fake_dev;
    g_wave_own70_ret = 0x155;
    g_wave_own70_swaps_vtbl = poison_vtbl;
    expect(wave->dyna_load(data_buf) == 0);
    g_wave_own70_swaps_vtbl = nullptr;
    fake_dev.vtbl = dev_vtable;
    expect(getp(0x3C) == &fake_dev);
    expect(g_wave_own70_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0x155}));
    expect(g_wave_own7C_calls == 1);

    // --- load(const char *): the replay tail ---
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0xC4 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x40 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x98 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x44 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    char name2_buf[] = "y.wav";

    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->load(name2_buf) == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x29;
    expect(wave->load(name2_buf) == 0x29);
    expect(g_wave_create_name == reinterpret_cast<char *>(name2_buf));
    expect(g_wave_create_mode == 1);

    // The caller's name - not the remembered one - reaches Sound::load; a
    // failing load stops before the replay tail.
    setp(0x3C, &fake_dev);
    obj[0x54] = 0x19;
    reset_load();
    g_wave_sload_ret = 0x77;
    expect(wave->load(name2_buf) == 0x77);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0xC1}));
    expect(g_wave_sload_name == reinterpret_cast<char *>(name2_buf));

    // Success: length lands, then volume, pitch, and the 0x08 dword replay
    // through the device, in that order.
    obj[0x54] = 0;
    set32(0x04, 33);
    set32(0x58, 0x111);
    set32(0x08, 0x222);
    reset_load();
    g_wave_own58_ret = 9;
    g_wave_fam_ret = 0x555;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->load(name2_buf) == 0);
    expect(get32(0x60) == 0x555u);
    expect(g_wave_fam_arg_log ==
           std::vector<uint32_t>({2, 33, 0x111, 0x222}));
    {
        const uint32_t len = 0x555;
        std::memcpy(expected.data() + 16 + 0x60, &len, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entries.
    setp(0x4C, nullptr);
    expect(wave_load_empty_redirect(wave, nullptr) == 8);
    expect(wave_reload_redirect(wave, nullptr) == 8);
    expect(wave_dyna_load_redirect(wave, nullptr, data_buf) == 0xC);
    setp(0x3C, nullptr);
    guard = 0;
    expect(wave_load_fname_redirect(wave, nullptr, name2_buf) == 1);

    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
    SoundOriginalLoad = saved_sload;
}

void test_wave_init() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_new = WaveOperatorNew;
    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 1;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot48);
    own_vtable[0x70 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot70);
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto reset_init = [&] {
        filefind_get_calls = 0;
        g_wave_new_calls = g_wave_create_calls = 0;
        g_wave_own48_calls = g_wave_own70_calls = g_wave_own7C_calls = 0;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
        g_wave_own70_ret = 0;
    };
    char name_arg[] = "menu.wav";
    char resolved_buf[] = "snd/menu.wav";

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // Streaming refuses the bit-4 and bit-7 modes before resolving anything.
    reset_init();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->init(name_arg, 4 | 0x10);
    wave->init(name_arg, 4 | 0x80);
    expect(filefind_get_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // An unresolvable name changes nothing - the old filename is kept.
    reset_init();
    filefind_get_result = nullptr;
    wave->init(name_arg, 1);
    expect(filefind_get_calls == 1 &&
           filefind_get_request == static_cast<LPCSTR>(name_arg));
    expect(g_wave_dtor_deletes.empty() && g_wave_new_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Non-streaming, every folding bit at once: the old name goes back to
    // the game heap, the resolved path is copied in, the flag dword is
    // rebuilt from zero, and mode bit 1 runs the wave's own slot 0x48 with
    // the first two flag bits already placed and the high ones not yet.
    char old_name[4] = {'x', 0, 0, 0};
    setp(0x4C, old_name);
    setp(0x3C, nullptr);
    reset_init();
    filefind_get_result = resolved_buf;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->init(name_arg, 0x1D3);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == old_name);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "snd/menu.wav") == 0);
    expect(g_wave_create_calls == 0 && g_wave_fam_calls == 0);
    expect(g_wave_own48_calls == 1 && g_wave_own48_self == wave &&
           g_wave_own48_arg == 1);
    expect(g_wave_own48_seen_flags == 5);  // bits 0 and 2 placed, rest later
    expect(get32(0x54) == 0x3D);
    {
        void *arena = g_wave_new_arena;
        std::memcpy(expected.data() + 16 + 0x4C, &arena, 4);
        const uint32_t flags_word = 0x3D;
        std::memcpy(expected.data() + 16 + 0x54, &flags_word, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Streaming with no device: creation from the RESOLVED path, the
    // captured-vtable attribute round (the 0x70 query swaps the table for a
    // poisoned one to prove the capture came first), the wave's own 0x7C,
    // then the raw mode mask to the live device.
    void *poison_vtbl2[64];
    std::memset(poison_vtbl2, 0, sizeof(poison_vtbl2));
    setp(0x3C, nullptr);
    setp(0x4C, nullptr);
    reset_init();
    g_wave_create_installs = &fake_dev;
    g_wave_own70_ret = 0x2AA;
    g_wave_own70_swaps_vtbl = poison_vtbl2;
    g_wave_own7C_restores_vtbl = dev_vtable;
    wave->init(name_arg, 4);
    g_wave_own70_swaps_vtbl = nullptr;
    g_wave_own7C_restores_vtbl = nullptr;
    fake_dev.vtbl = dev_vtable;
    expect(g_wave_create_calls == 1);
    expect(g_wave_create_name == static_cast<LPCSTR>(resolved_buf));
    expect(g_wave_create_mode == 1);
    expect(g_wave_own70_calls == 1 && g_wave_own7C_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0x2AA, 4}));
    expect(get32(0x54) == 2);
    expect(g_wave_own48_calls == 0);

    // Streaming with a device already wrapped: no creation, just the raw
    // mode mask; mode bit 0 folds alongside the streaming bit.
    reset_init();
    wave->init(name_arg, 5);
    expect(g_wave_create_calls == 0);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({5}));
    expect(get32(0x54) == 3);

    // Streaming with a dead hook: no creation, no device call, and a failing
    // creation skips the attribute round but still folds the tail bits.
    setp(0x3C, nullptr);
    guard = 0;
    reset_init();
    wave->init(name_arg, 4);
    expect(g_wave_create_calls == 0 && g_wave_fam_calls == 0);
    expect(get32(0x54) == 2);
    guard = 1;
    reset_init();
    g_wave_create_ret = 0x31;
    wave->init(name_arg, 4 | 1);
    expect(g_wave_create_calls == 1);
    expect(g_wave_own70_calls == 0 && g_wave_fam_calls == 0);
    expect(get32(0x54) == 3);

    // Non-streaming keeps its bit-4 mapping (streaming suppressed it above).
    reset_init();
    wave->init(name_arg, 0x10);
    expect(get32(0x54) == 4);

    // Redirect entry.
    reset_init();
    filefind_get_result = nullptr;
    wave_init_redirect(wave, nullptr, name_arg, 1);
    expect(filefind_get_calls == 1);

    WaveOperatorDelete = saved_delete;
    WaveOperatorNew = saved_new;
    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
}

void test_wave_device_forwarder_family() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto bits_of = [](float f) {
        uint32_t b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    auto ptr_bits = [](const void *p) {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x14, &fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // Device paths: verbatim answers, per-slot dispatch, argument dwords.
    char raw_name[] = "dump.raw";
    arm(0x50, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2101;
    expect(device->start_raw_dump(raw_name) == 0x2101);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev &&
           g_wave_fam_args[0] == ptr_bits(raw_name));

    auto *eax_props = reinterpret_cast<EAX_REVERB_PROPERTIES *>(raw_name);
    arm(0x7C, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2102;
    expect(device->set_eax(eax_props) == 0x2102);
    expect(g_wave_fam_args[0] == ptr_bits(raw_name));

    arm(0x78, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2103;
    expect(device->set_eax(static_cast<unsigned long>(0xE1)) == 0x2103);
    expect(g_wave_fam_args[0] == 0xE1);

    arm(0x80, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2104;
    expect(device->set_eax_mix(0.25f) == 0x2104);
    expect(g_wave_fam_args[0] == bits_of(0.25f));

    arm(0x88, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x2105;
    expect(device->set_listener_position(1.5f, -2.5f, 3.5f) == 0x2105);
    expect(g_wave_fam_args[0] == bits_of(1.5f) &&
           g_wave_fam_args[1] == bits_of(-2.5f) &&
           g_wave_fam_args[2] == bits_of(3.5f));

    float fx = 0, fy = 0, fz = 0;
    arm(0x8C, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x2106;
    expect(device->get_listener_position(&fx, &fy, &fz) == 0x2106);
    expect(g_wave_fam_args[0] == ptr_bits(&fx) &&
           g_wave_fam_args[1] == ptr_bits(&fy) &&
           g_wave_fam_args[2] == ptr_bits(&fz));

    const struct {
        size_t slot;
        bool setter;
    } xyz[] = {
        {0x90, true}, {0x94, false}, {0x98, true},
        {0x9C, false}, {0xA0, true}, {0xA4, false},
    };
    int tag = 0x2110;
    for (const auto &c : xyz) {
        arm(c.slot, reinterpret_cast<void *>(&observe_wave_dev1));
        g_wave_fam_ret = tag;
        int result;
        float value = 4.5f + tag;
        switch (c.slot) {
        case 0x90: result = device->set_listener_xpos(value); break;
        case 0x94: result = device->get_listener_xpos(&fx); break;
        case 0x98: result = device->set_listener_ypos(value); break;
        case 0x9C: result = device->get_listener_ypos(&fy); break;
        case 0xA0: result = device->set_listener_zpos(value); break;
        default: result = device->get_listener_zpos(&fz); break;
        }
        expect(result == tag);
        expect(g_wave_fam_calls == 1);
        if (c.setter) {
            expect(g_wave_fam_args[0] == bits_of(value));
        }
        ++tag;
    }

    // get_description: void, three arguments through slot 0x1C.
    char descr[8] = {};
    arm(0x1C, reinterpret_cast<void *>(&observe_wave_dev3));
    device->get_description(2, descr, 7);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_fam_args[0] == 2 && g_wave_fam_args[1] == ptr_bits(descr) &&
           g_wave_fam_args[2] == 7);

    // Nothing above wrote the object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // No-device defaults: 0x14 everywhere except the raw dump's 3 and
    // get_description's silence.
    setp(0x14, nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_fam_calls = 0;
    expect(device->start_raw_dump(raw_name) == 3);
    expect(device->set_eax(eax_props) == 0x14);
    expect(device->set_eax(static_cast<unsigned long>(1)) == 0x14);
    expect(device->set_eax_mix(1.0f) == 0x14);
    expect(device->set_listener_position(1, 2, 3) == 0x14);
    expect(device->get_listener_position(&fx, &fy, &fz) == 0x14);
    expect(device->set_listener_xpos(1) == 0x14);
    expect(device->get_listener_xpos(&fx) == 0x14);
    expect(device->set_listener_ypos(1) == 0x14);
    expect(device->get_listener_ypos(&fy) == 0x14);
    expect(device->set_listener_zpos(1) == 0x14);
    expect(device->get_listener_zpos(&fz) == 0x14);
    device->get_description(1, descr, 1);
    expect(g_wave_fam_calls == 0);
    expect(descr[0] == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entries, one per export.
    expect(wave_device_start_raw_dump_redirect(device, nullptr, raw_name) ==
           3);
    expect(wave_device_set_eax_props_redirect(device, nullptr, eax_props) ==
           0x14);
    expect(wave_device_set_eax_redirect(device, nullptr, 1) == 0x14);
    expect(wave_device_set_eax_mix_redirect(device, nullptr, 1) == 0x14);
    expect(wave_device_set_listener_position_redirect(device, nullptr, 1, 2,
                                                      3) == 0x14);
    expect(wave_device_get_listener_position_redirect(device, nullptr, &fx,
                                                      &fy, &fz) == 0x14);
    expect(wave_device_set_listener_xpos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_xpos_redirect(device, nullptr, &fx) ==
           0x14);
    expect(wave_device_set_listener_ypos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_ypos_redirect(device, nullptr, &fy) ==
           0x14);
    expect(wave_device_set_listener_zpos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_zpos_redirect(device, nullptr, &fz) ==
           0x14);
    wave_device_get_description_redirect(device, nullptr, 1, descr, 1);
}

void test_sound_small_setters() {
    std::vector<uint8_t> storage(0xA0 + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *sound = reinterpret_cast<Sound *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm_dev = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
    };
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);
    setp(0x3C, &fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // set_volume: seven bits kept, no group rescaling, device slot 0x40.
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    sound->set_volume(0x1FF);
    expect(get32(0x04) == 0x7F);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev &&
           g_wave_fam_args[0] == 0x7F);
    {
        const uint32_t vol = 0x7F;
        std::memcpy(expected.data() + 16 + 0x04, &vol, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // set_fade: zero refused untouched; otherwise field 0x38 + device slot 0.
    arm_dev(0, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade(0) == 0xA);
    expect(g_wave_fam_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    expect(sound->set_fade(0x777) == 0);
    expect(get32(0x38) == 0x777);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x777);

    // set_fade_in: the same field, device slot 0x54.
    arm_dev(0x54, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade_in(0) == 0xA);
    expect(g_wave_fam_calls == 0);
    expect(sound->set_fade_in(0x888) == 0);
    expect(get32(0x38) == 0x888);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x888);

    // Both fade setters skip the device quietly when none is wrapped.
    setp(0x3C, nullptr);
    arm_dev(0, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade(5) == 0);
    expect(get32(0x38) == 5 && g_wave_fam_calls == 0);
    expect(sound->set_fade_in(6) == 0);
    expect(get32(0x38) == 6 && g_wave_fam_calls == 0);
    sound->set_volume(3);
    expect(get32(0x04) == 3);

    // fade_in: the own slot 0x54 answers; only zero lets slot 0x28 follow.
    own_vtable[0x54 / 4] =
        reinterpret_cast<void *>(&observe_sound_own_slot54);
    own_vtable[0x28 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);
    g_sound_own54_calls = 0;
    g_wave_own80_calls = 0;
    g_sound_own54_ret = 5;
    sound->fade_in(0xABC);
    expect(g_sound_own54_calls == 1 && g_sound_own54_self == sound &&
           g_sound_own54_arg == 0xABC);
    expect(g_wave_own80_calls == 0);
    g_sound_own54_ret = 0;
    sound->fade_in(0xDEF);
    expect(g_sound_own54_arg == 0xDEF);
    expect(g_wave_own80_calls == 1);

    // set_pan: the clamp, the field, the device slot 0x44.
    setp(0x3C, &fake_dev);
    arm_dev(0x44, reinterpret_cast<void *>(&observe_wave_dev1));
    sound->set_pan(-0x41);
    expect(get32(0x08) == 0xFFFFFFC0u);
    expect(g_wave_fam_args[0] == 0xFFFFFFC0u);
    sound->set_pan(-0x40);
    expect(get32(0x08) == 0xFFFFFFC0u);
    sound->set_pan(0x40);
    expect(get32(0x08) == 0x3F);
    sound->set_pan(0x3F);
    expect(get32(0x08) == 0x3F);
    sound->set_pan(-1);
    expect(get32(0x08) == 0xFFFFFFFFu);  // mid-range negatives pass through
    sound->set_pan(1);
    expect(get32(0x08) == 1);
    expect(g_wave_fam_calls == 6);

    // unload: the device's answer, the UNCONDITIONAL own slot 0x80 while the
    // device is still remembered, then the device and loaded bit clear.
    own_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);
    arm_dev(0x14, reinterpret_cast<void *>(&observe_wave_dev0));
    set32(0x40, 0xF1);
    g_wave_own80_calls = 0;
    g_wave_fam_ret = 0x77;
    expect(sound->unload() == 0x77);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_own80_calls == 1 && g_wave_own80_seen_device == &fake_dev);
    expect(getp(0x3C) == nullptr);
    expect(get32(0x40) == 0xF0);
    // No device: answer 0, the callback still runs.
    set32(0x40, 1);
    g_wave_own80_calls = 0;
    g_wave_fam_calls = 0;
    expect(sound->unload() == 0);
    expect(g_wave_fam_calls == 0 && g_wave_own80_calls == 1);
    expect(get32(0x40) == 0);

    // Redirect entries.
    setp(0x3C, nullptr);
    sound_set_volume_redirect(sound, nullptr, 9);
    expect(get32(0x04) == 9);
    expect(sound_set_fade_redirect(sound, nullptr, 0) == 0xA);
    expect(sound_set_fade_in_redirect(sound, nullptr, 0) == 0xA);
    g_sound_own54_ret = 5;
    g_sound_own54_calls = 0;
    sound_fade_in_arg_redirect(sound, nullptr, 2);
    expect(g_sound_own54_calls == 1);
    sound_set_pan_redirect(sound, nullptr, 1);
    expect(get32(0x08) == 1);
    g_wave_own80_calls = 0;
    expect(sound_unload_redirect(sound, nullptr) == 0);
    expect(g_wave_own80_calls == 1);
}

void test_wave_device_groups() {
    static_assert(sizeof(Wave_Device) == 0x1A4,
                  "the group table must reach slot 15's count");
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);

    auto *const saved_insert = WaveDeviceGroupInsert;
    auto *const saved_delete = WaveOperatorDelete;
    WaveDeviceGroupInsert = &observe_wave_group_insert;
    WaveOperatorDelete = &observe_wave_operator_delete;

    auto group_base = [](unsigned g) { return 0x24 + g * 0x18; };
    auto gset32 = [&](unsigned g, size_t off, uint32_t v) {
        std::memcpy(obj + group_base(g) + off, &v, 4);
    };
    auto gsetp = [&](unsigned g, size_t off, const void *ptr) {
        std::memcpy(obj + group_base(g) + off, &ptr, 4);
    };
    auto gget32 = [&](unsigned g, size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + group_base(g) + off, 4);
        return v;
    };
    auto ggetp = [&](unsigned g, size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + group_base(g) + off, 4);
        return ptr;
    };
    uint8_t wave_a[0x6C];
    uint8_t wave_b[0x6C];
    auto *wa = reinterpret_cast<Wave *>(wave_a);
    auto *wb = reinterpret_cast<Wave *>(wave_b);
    auto wave_slot = [&](uint8_t *w) {
        uint32_t v;
        std::memcpy(&v, w + 0x68, 4);
        return v;
    };
    auto set_wave_slot = [&](uint8_t *w, uint32_t v) {
        std::memcpy(w + 0x68, &v, 4);
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    std::memset(wave_a, 0, sizeof(wave_a));
    std::memset(wave_b, 0, sizeof(wave_b));

    // --- add_to_group ---
    g_wave_ginsert_calls = 0;
    expect(device->add_to_group(0x10, wa) == 0xA);
    expect(device->add_to_group(3, nullptr) == 0xA);
    expect(g_wave_ginsert_calls == 0);
    set_wave_slot(wave_a, 0xEE);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(device->add_to_group(5, wa) == 0);
    expect(g_wave_ginsert_calls == 1);
    expect(g_wave_ginsert_head == obj + group_base(5) + 8);
    expect(g_wave_ginsert_wave == wa);
    expect(g_wave_ginsert_seen_slot == 0xEE);  // stored after the insert
    expect(wave_slot(wave_a) == 5);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    // Slot 15 is the last accepted one.
    set_wave_slot(wave_a, 0xEE);
    expect(device->add_to_group(0xF, wa) == 0);
    expect(g_wave_ginsert_calls == 2 && wave_slot(wave_a) == 0xF);

    // --- is_group_disabled ---
    expect(device->is_group_disabled(0x10) == 1);
    obj[group_base(2)] = 0;
    expect(device->is_group_disabled(2) == 1);
    obj[group_base(2)] = 9;
    expect(device->is_group_disabled(2) == 0);
    obj[group_base(0xF)] = 1;
    expect(device->is_group_disabled(0xF) == 0);  // 15 is still in range
    obj[group_base(0xF)] = expected[16 + group_base(0xF)];
    obj[group_base(2)] = expected[16 + group_base(2)];
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- pull_from_group ---
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(nullptr) == 0xA);
    set_wave_slot(wave_a, 0x10);
    expect(device->pull_from_group(wa) == 0xA);
    set_wave_slot(wave_a, 0x33);
    expect(device->pull_from_group(wa) == 0xA);

    // An empty group just forgets the slot.
    set_wave_slot(wave_a, 3);
    gsetp(3, 8, nullptr);
    expect(device->pull_from_group(wa) == 0);
    expect(wave_slot(wave_a) == 0x10);
    expect(g_wave_dtor_deletes.empty());

    // A wave whose node is not on the list walks it and forgets the slot.
    WaveGroupNode n1{};
    WaveGroupNode n2{};
    WaveGroupNode n3{};
    n1.next = &n2;
    n2.prev = &n1;
    n2.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    expect(device->pull_from_group(wa) == 0);
    expect(wave_slot(wave_a) == 0x10);
    expect(g_wave_dtor_deletes.empty());
    expect(n1.next == &n2 && n2.prev == &n1);

    // Found in the middle: neighbours re-linked, cursor on the follower,
    // head and tail untouched, node freed, count down one.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n3 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.next = &n3;
    n2.wave = wa;
    n3.prev = &n2;
    n3.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n3);
    gsetp(4, 0x10, reinterpret_cast<void *>(0x7777));
    gset32(4, 0x14, 7);
    g_wave_delete_watch =
        reinterpret_cast<uint32_t *>(obj + group_base(4) + 0x14);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(device->pull_from_group(wa) == 0);
    // The node is freed BEFORE the count drops; the free observer saw 7.
    expect(g_wave_delete_watch_seen == 7);
    g_wave_delete_watch = nullptr;
    expect(n1.next == &n3 && n3.prev == &n1);
    expect(ggetp(4, 8) == &n1 && ggetp(4, 0xC) == &n3);
    expect(ggetp(4, 0x10) == &n3);
    expect(gget32(4, 0x14) == 6);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);
    expect(wave_slot(wave_a) == 0x10);
    {
        uint8_t *const eobj = expected.data() + 16;
        void *cur = &n3;
        const uint32_t cnt = 6;
        std::memcpy(eobj + group_base(4) + 0x10, &cur, 4);
        std::memcpy(eobj + group_base(4) + 0x14, &cnt, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Found at the head: the head advances, the follower loses its prev.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wa;
    n2.prev = &n1;
    n2.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(ggetp(4, 8) == &n2 && n2.prev == nullptr);
    expect(ggetp(4, 0x10) == &n2);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n1);

    // Found at the tail: the cursor clears and the tail steps back.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n2);
    gsetp(4, 0x10, reinterpret_cast<void *>(0x8888));
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(n1.next == nullptr);
    expect(ggetp(4, 0x10) == nullptr && ggetp(4, 0xC) == &n1);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);

    // A follower that IS the cursor field: the neighbour store lands on the
    // cursor first and the cursor store overwrites it second, so their order
    // is visible in which value survives.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.next = reinterpret_cast<WaveGroupNode *>(obj + group_base(4) + 0x10);
    n2.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(n1.next == n2.next);
    // next->prev wrote &n1 into the cursor field, then the cursor store
    // replaced it with the follower's address.
    expect(ggetp(4, 0x10) == obj + group_base(4) + 0x10);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);

    // The only node: head, tail, and cursor all empty out.
    n1 = WaveGroupNode{};
    n1.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(ggetp(4, 8) == nullptr && ggetp(4, 0xC) == nullptr &&
           ggetp(4, 0x10) == nullptr);
    expect(g_wave_dtor_deletes.size() == 1);

    // --- the recovered insert helper itself ---
    auto *const saved_new2 = WaveOperatorNew;
    WaveOperatorNew = &observe_wave_operator_new;
    auto *arena_node = reinterpret_cast<WaveGroupNode *>(g_wave_new_arena);

    // Empty list: the node becomes both ends.
    WaveGroupList list{};
    list.count = 5;
    list.cursor = reinterpret_cast<WaveGroupNode *>(0x9999);
    g_wave_new_calls = 0;
    g_wave_new_fails = false;
    wave_group_insert_redirect(&list, nullptr, wa);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 0xC);
    expect(list.head == arena_node && list.tail == arena_node);
    expect(list.count == 6);
    expect(list.cursor == reinterpret_cast<WaveGroupNode *>(0x9999));
    expect(arena_node->prev == nullptr && arena_node->next == nullptr &&
           arena_node->wave == wa);

    // Live tail: append after it; the new node's prev is the re-read tail.
    WaveGroupNode t1{};
    list.head = &t1;
    list.tail = &t1;
    list.count = 1;
    wave_group_insert_redirect(&list, nullptr, wb);
    expect(t1.next == arena_node);
    expect(arena_node->prev == &t1 && arena_node->next == nullptr &&
           arena_node->wave == wb);
    expect(list.head == &t1 && list.tail == arena_node && list.count == 2);

    // A tail aimed at the list itself: the old tail's next write lands ON
    // the tail field, so the re-read prev is the new node - the write
    // order is visible in which value the prev holds.
    list = WaveGroupList{};
    list.tail = reinterpret_cast<WaveGroupNode *>(&list);
    wave_group_insert_redirect(&list, nullptr, wa);
    expect(arena_node->prev == arena_node);
    expect(list.tail == arena_node);

    // A failed allocation on an empty list still counts the phantom node.
    list = WaveGroupList{};
    list.count = 9;
    g_wave_new_fails = true;
    wave_group_insert_redirect(&list, nullptr, wa);
    g_wave_new_fails = false;
    expect(list.head == nullptr && list.tail == nullptr);
    expect(list.count == 10);
    WaveOperatorNew = saved_new2;

    // Redirect entries.
    expect(wave_device_add_to_group_redirect(device, nullptr, 0x10, wa) ==
           0xA);
    expect(wave_device_is_group_disabled_redirect(device, nullptr, 0x10) ==
           1);
    expect(wave_device_pull_from_group_redirect(device, nullptr, nullptr) ==
           0xA);

    WaveDeviceGroupInsert = saved_insert;
    WaveOperatorDelete = saved_delete;
}

void test_sound_set_type_and_load() {
    std::vector<uint8_t> storage(0xA0 + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *sound = reinterpret_cast<Sound *>(obj);
    g_wave_dtor_obj = obj;

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };

    // --- set_type: the whole jump table, plus the invalid arms (3 routes
    // to the invalid arm despite being inside the table's range) ---
    seed_storage(storage.data(), expected.data(), storage.size());
    const struct { uint32_t type; uint32_t bit; } type_map[] = {
        {1, 4}, {2, 8}, {4, 0x10}, {5, 0x28}, {6, 0x100}, {7, 0x80},
    };
    const uint32_t base = 0x60000201u;
    for (const auto &c : type_map) {
        set32(0x40, base);
        set32(0x50, 0xEEEEEEEEu);
        sound->set_type(c.type);
        expect(get32(0x50) == c.type);
        expect(get32(0x40) == (base | c.bit));
    }
    const uint32_t invalid_types[] = {0, 3, 8, 0x7FFFFFFFu};
    for (const uint32_t t : invalid_types) {
        set32(0x40, base);
        set32(0x50, 0xEEEEEEEEu);
        sound->set_type(t);
        expect(get32(0x50) == 0);
        expect(get32(0x40) == base);
    }
    set32(0x40, base);
    set32(0x50, 0);
    std::memcpy(expected.data(), storage.data(), storage.size());
    sound->set_type(6);
    {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t t6 = 6;
        const uint32_t f6 = base | 0x100;
        std::memcpy(eobj + 0x50, &t6, 4);
        std::memcpy(eobj + 0x40, &f6, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    sound_set_type_redirect(sound, nullptr, 2);
    expect(get32(0x50) == 2);

    // --- load ---
    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_new = WaveOperatorNew;
    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 1;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x60 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x10 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);
    setp(0x00, own_vtable);

    auto reset_sload = [&] {
        filefind_get_calls = 0;
        g_wave_new_calls = g_wave_create_calls = 0;
        g_wave_own7C_calls = g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
    };
    char name_arg[] = "menu.wav";
    char resolved_buf[] = "snd/menu.wav";

    // Unresolvable: 0xA, and nothing else was even consulted.
    reset_sload();
    filefind_get_result = nullptr;
    expect(sound->load(name_arg) == 0xA);
    expect(filefind_get_calls == 1 &&
           filefind_get_request == static_cast<LPCSTR>(name_arg));
    expect(g_wave_create_calls == 0 && g_wave_new_calls == 0);

    // Resolvable but the hook guard is dead: 1, after the resolution.
    reset_sload();
    filefind_get_result = resolved_buf;
    guard = 0;
    expect(sound->load(name_arg) == 1);
    expect(filefind_get_calls == 1 && g_wave_create_calls == 0);
    guard = 1;

    // A failing creation propagates.
    setp(0x3C, nullptr);
    reset_sload();
    g_wave_create_ret = 0x2E;
    expect(sound->load(name_arg) == 0x2E);
    expect(g_wave_create_calls == 1 && g_wave_create_mode == 1 &&
           g_wave_create_name == static_cast<LPCSTR>(resolved_buf));

    // First successful load through a freshly created device: loaded bit
    // set BEFORE the sound's own 0x7C, loop started, name remembered.
    setp(0x4C, nullptr);
    set32(0x40, 0xF0);
    set32(0x30, 3);
    reset_sload();
    g_wave_create_installs = &fake_dev;
    g_wave_fam_ret = 0;
    // Scrub the shared arena: earlier tests left the same resolved string in
    // it, which would mask a dropped copy.
    std::memset(g_wave_new_arena, 0x5A, sizeof(g_wave_new_arena));
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(sound->load(name_arg) == 0);
    expect(getp(0x3C) == &fake_dev);
    expect(get32(0x40) == 0xF1);
    expect(g_wave_own7C_calls == 1 && g_wave_own7C_seen_flags == 0xF1);
    expect(g_wave_fam_arg_log ==
           std::vector<uint32_t>(
               {static_cast<uint32_t>(
                    reinterpret_cast<uintptr_t>(resolved_buf)),
                1}));
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "snd/menu.wav") == 0);
    expect(g_wave_dtor_deletes.empty());
    {
        uint8_t *const eobj = expected.data() + 16;
        void *dev = &fake_dev;
        const uint32_t f = 0xF1;
        void *arena = g_wave_new_arena;
        std::memcpy(eobj + 0x3C, &dev, 4);
        std::memcpy(eobj + 0x40, &f, 4);
        std::memcpy(eobj + 0x4C, &arena, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A busy existing device answers 0xF before any load.
    reset_sload();
    g_wave_fam_ret = 7;
    expect(sound->load(name_arg) == 0xF);
    expect(g_wave_fam_calls == 1 && g_wave_fam_arg_log.empty());

    // A failing load on an already-loaded sound clears the loaded bit and
    // still replaces the remembered name - new copy first, old freed after.
    reset_sload();
    g_wave_fam_ret = 0;  // not busy...
    // ...but the load answer comes from the same shared return; use the
    // arg-log to tell the calls apart and drive the answer per call through
    // the busy observer being armed to return 0 and the load to fail: the
    // shared return cannot differ per slot, so run the failing load against
    // a sound with NO device instead (fresh creation, failing load).
    setp(0x3C, nullptr);
    set32(0x40, 0xF1);
    reset_sload();
    g_wave_create_installs = &fake_dev;
    g_wave_fam_ret = 0x33;
    expect(sound->load(name_arg) == 0x33);
    expect(get32(0x40) == 0xF0);
    expect(g_wave_own7C_calls == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == g_wave_new_arena);
    expect(getp(0x4C) == g_wave_new_arena);

    // Success with the loaded bit already set: no protocol, no loop.
    set32(0x40, 1);
    reset_sload();
    g_wave_fam_ret = 0;
    expect(sound->load(name_arg) == 0);
    expect(g_wave_own7C_calls == 0);
    expect(get32(0x40) == 1);

    // Success, bit clear, but a zero loop dword: the 0x7C protocol without
    // the loop start.
    set32(0x40, 0);
    set32(0x30, 0);
    reset_sload();
    expect(sound->load(name_arg) == 0);
    expect(g_wave_own7C_calls == 1);
    // busy + load dispatched, and only the load logs an argument - the loop
    // call would have logged a second.
    expect(g_wave_fam_calls == 2 && g_wave_fam_arg_log.size() == 1);
    expect(get32(0x40) == 1);

    // Redirect entry.
    reset_sload();
    filefind_get_result = nullptr;
    expect(sound_load_redirect(sound, nullptr, name_arg) == 0xA);

    WaveOperatorDelete = saved_delete;
    WaveOperatorNew = saved_new;
    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
}

void test_wave_ctor() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    auto *const saved_stype = SoundSetType;
    SoundSetType = &observe_wave_set_type;

    seed_storage(storage.data(), expected.data(), storage.size());
    uint32_t pre_pitch;
    std::memcpy(&pre_pitch, obj + 0x58, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_stype_calls = 0;
    expect(wave_ctor_redirect(wave, nullptr) == wave);
    expect(g_wave_stype_calls == 1 && g_wave_stype_wave == wave);
    expect(g_wave_stype_type == 1);
    // At set_type time the final vtable is up, the flag dword reads 4, the
    // 0x54 byte is cleared, and the pitch dword still holds its seed - the
    // tail stores run after the call.
    expect(g_wave_stype_seen_vtable == 0x0066E44Cu);
    expect(g_wave_stype_seen_f40 == 4);
    expect(g_wave_stype_seen_f54 == 0);
    expect(g_wave_stype_seen_pitch == pre_pitch);
    // Full end state, byte for byte; only the dword at 0x34 keeps its seed.
    uint8_t *const eobj = expected.data() + 16;
    auto e32 = [&](size_t off, uint32_t v) { std::memcpy(eobj + off, &v, 4); };
    e32(0x00, 0x0066E44Cu);
    e32(0x04, 0x7F);
    e32(0x08, 0);
    std::memset(eobj + 0x0C, 0, 0x24);
    e32(0x30, 0);
    e32(0x38, 0x3E8);
    e32(0x3C, 0);
    e32(0x40, 4);
    e32(0x44, 0);
    e32(0x48, 0);
    e32(0x4C, 0);
    e32(0x50, 0);
    e32(0x54, 0);
    e32(0x58, 0);
    e32(0x5C, 0x3F800000u);
    e32(0x60, 0);
    e32(0x64, 0);
    e32(0x68, 0x10);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    SoundSetType = saved_stype;
}

void test_wave_destructor() {
    static_assert(sizeof(Wave) == 0x6C, "Wave tests require the legacy layout");
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_pull = WaveDevicePullFromGroup;
    void *const saved_dev_global = WaveDeviceGlobal;
    auto *const saved_delete = WaveOperatorDelete;
    auto **const saved_release_slot = WaveDeviceReleaseSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    Wave **const saved_head = WaveChainHead;
    Wave **const saved_tail = WaveChainTail;

    int fake_device_singleton = 0;
    func_wave_device_release *release_fn = &observe_wave_release;
    int release_guard = 1;
    Wave *chain_head = nullptr;
    Wave *chain_tail = nullptr;
    WaveDevicePullFromGroup = &observe_wave_pull_from_group;
    WaveDeviceGlobal = &fake_device_singleton;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveDeviceReleaseSlot = &release_fn;
    WaveDeviceReleaseGuard = &release_guard;
    WaveChainHead = &chain_head;
    WaveChainTail = &chain_tail;

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto node_ptr = [](uint8_t *node, size_t off) {
        void *p;
        std::memcpy(&p, node + off, 4);
        return p;
    };
    auto reset_counters = [&] {
        g_wave_dtor_pull_calls = g_wave_dtor_release_calls = 0;
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_dtor_release_relinks = false;
    };
    // The fields the destructor always leaves behind, applied to the expected
    // snapshot: the ultimate base vtable, a null device/prev/next/buffer, and
    // the final flag dword.
    auto apply_end_state = [&](uint32_t final_flags) {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t vt = 0x0066E444u;
        const uint32_t zero = 0;
        std::memcpy(eobj + 0x00, &vt, 4);
        std::memcpy(eobj + 0x3C, &zero, 4);
        std::memcpy(eobj + 0x40, &final_flags, 4);
        std::memcpy(eobj + 0x44, &zero, 4);
        std::memcpy(eobj + 0x48, &zero, 4);
        std::memcpy(eobj + 0x4C, &zero, 4);
    };

    // Scenario 1: every primary path at once. Group slot 0xF (< 0x10) pulls
    // the wave from the device singleton's group while the wave's OWN vtable
    // is installed; the buffer is freed exactly once (the unconditional clear
    // keeps the inlined base free from seeing it again); both chain
    // neighbours are real nodes, so the chain end slots stay untouched; the
    // release hook runs over the wrapped device with the MIDDLE vtable
    // installed; only bit 1 of the flag dword is cleared.
    uint8_t node_prev[0x6C];
    uint8_t node_next[0x6C];
    std::memset(node_prev, 0x66, sizeof(node_prev));
    std::memset(node_next, 0x77, sizeof(node_next));
    uint8_t buffer_block[8];
    int fake_wrapped_device = 0;
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x0F);
    setp(0x4C, buffer_block);
    set32(0x40, 0xA5A5A5F3u);
    setp(0x44, node_prev);
    setp(0x48, node_next);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 1);
    expect(g_wave_dtor_pull_dev == &fake_device_singleton);
    expect(g_wave_dtor_pull_wave == wave);
    expect(g_wave_dtor_pull_seen_vtable == 0x0066E44Cu);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == buffer_block);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == buffer_block);
    expect(node_ptr(node_prev, 0x48) == node_next);
    expect(node_ptr(node_next, 0x44) == node_prev);
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(g_wave_dtor_release_calls == 1 &&
           g_wave_dtor_release_dev == &fake_wrapped_device);
    expect(g_wave_dtor_release_seen_vtable == 0x0066E3C0u);
    apply_end_state(0xA5A5A5F1u);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    expect(get32(0x68) == 0x0F);

    // Scenario 2: the two normally-dead base-destructor arms. The chain prev
    // is aimed 4 bytes into the wave itself, so unlink #1's neighbour write
    // (prev->next at +0x48) lands on the wave's OWN buffer slot at +0x4C and
    // re-populates it with the next pointer - the inlined base free then
    // fires on that block. The release hook re-arms the chain bit with a null
    // prev, so unlink #2 takes the head-slot arm. Group slot exactly 0x10 is
    // the boundary: NOT pulled.
    uint8_t late_block[0x6C];
    uint8_t node_q[0x6C];
    std::memset(late_block, 0x33, sizeof(late_block));
    std::memset(node_q, 0x77, sizeof(node_q));
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x10);
    setp(0x4C, nullptr);
    set32(0x40, 2);
    setp(0x44, obj + 4);
    setp(0x48, late_block);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    g_wave_dtor_release_relinks = true;
    g_wave_dtor_release_relink_prev = nullptr;
    g_wave_dtor_release_relink_next = node_q;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == late_block);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == late_block);
    expect(node_ptr(late_block, 0x44) == obj + 4);  // next->prev in unlink #1
    expect(g_wave_dtor_release_calls == 1);
    expect(g_wave_dtor_release_seen_vtable == 0x0066E3C0u);
    expect(chain_head == reinterpret_cast<Wave *>(node_q));  // unlink #2 head arm
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(node_ptr(node_q, 0x44) == nullptr);  // next->prev wrote the null prev
    apply_end_state(0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 3: unlink #1's head arm. A null prev routes the next pointer
    // into the chain head slot; no buffer, no device, no pull - and the
    // buffer slot is still cleared unconditionally.
    uint8_t node_s[0x6C];
    std::memset(node_s, 0x55, sizeof(node_s));
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x45);
    setp(0x4C, nullptr);
    set32(0x40, 0x13);
    setp(0x44, nullptr);
    setp(0x48, node_s);
    setp(0x3C, nullptr);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 0);
    expect(g_wave_dtor_deletes.empty());
    expect(g_wave_dtor_release_calls == 0);
    expect(chain_head == reinterpret_cast<Wave *>(node_s));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(node_ptr(node_s, 0x44) == nullptr);
    apply_end_state(0x11);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 4: unlink #1's tail arm, and a dead release guard. A null next
    // routes the prev pointer into the chain tail slot; the guard suppresses
    // the release hook but the device is still forgotten.
    uint8_t node_t[0x6C];
    std::memset(node_t, 0x66, sizeof(node_t));
    uint8_t block_d[8];
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0);
    setp(0x4C, block_d);
    set32(0x40, 2);
    setp(0x44, node_t);
    setp(0x48, nullptr);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 0;
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 1);  // slot 0 also counts as grouped
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == block_d);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == block_d);
    expect(node_ptr(node_t, 0x48) == nullptr);
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(node_t));
    expect(g_wave_dtor_release_calls == 0);
    expect(getp(0x3C) == nullptr);
    apply_end_state(0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 5: unlink #2's tail arm, reached with the chain bit initially
    // CLEAR - unlink #1 must not run (its would-be neighbour stays pristine),
    // and only the release hook's re-link arms the second pass.
    uint8_t poison_node[0x6C];
    uint8_t poison_expected[0x6C];
    uint8_t node_r[0x6C];
    std::memset(poison_node, 0x44, sizeof(poison_node));
    std::memcpy(poison_expected, poison_node, sizeof(poison_node));
    std::memset(node_r, 0x88, sizeof(node_r));
    uint8_t block_e[8];
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x10);
    setp(0x4C, block_e);
    set32(0x40, 0x11);
    setp(0x44, poison_node);
    setp(0x48, poison_node);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    g_wave_dtor_release_relinks = true;
    g_wave_dtor_release_relink_prev = node_r;
    g_wave_dtor_release_relink_next = nullptr;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == block_e);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == block_e);
    expect(std::memcmp(poison_node, poison_expected, sizeof(poison_node)) ==
           0);
    expect(g_wave_dtor_release_calls == 1);
    expect(node_ptr(node_r, 0x48) == nullptr);  // prev->next in unlink #2
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(node_r));
    apply_end_state(0x11);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    WaveDevicePullFromGroup = saved_pull;
    WaveDeviceGlobal = saved_dev_global;
    WaveOperatorDelete = saved_delete;
    WaveDeviceReleaseSlot = saved_release_slot;
    WaveDeviceReleaseGuard = saved_guard;
    WaveChainHead = saved_head;
    WaveChainTail = saved_tail;
}

void test_delegating_closers() {
    // Popup::close has 104 callers, the most of anything left. Its Scroll sits
    // at 0x3230 - exactly sizeof(BasePop) - and Scroll::close is already
    // recovered, so only the base close is reached through a seam.
    // Popup::close is held back from this batch. It runs the real
    // Scroll::close on its member, which needs the whole Scroll fixture -
    // two default tables, three seams and two vtables - and rebuilding that
    // inside this test duplicates the Scroll suite rather than reusing it.
    // The recovery is written and correct by disassembly; it waits on that
    // fixture becoming a shared helper.

    // PopMenu::init passes `this` straight through with two zero arguments,
    // and hands back whatever the base returns.
    auto *const saved_init = BasePopOriginalInit;
    BasePopOriginalInit = &observe_base_pop_init;
    std::vector<uint8_t> pm(sizeof(PopMenu) + 32);
    std::vector<uint8_t> pm_want(pm.size());
    auto *menu = reinterpret_cast<PopMenu *>(pm.data() + 16);
    seed_storage(pm.data(), pm_want.data(), pm.size());
    std::memcpy(pm_want.data(), pm.data(), pm.size());
    g_base_pop_init_result = 0x7FFFFFFF;
    expect(menu->init() == 0x7FFFFFFF);
    expect(reinterpret_cast<void *>(g_inited_base_pop) ==
           reinterpret_cast<void *>(menu));
    expect(g_init_a == 0);
    expect(g_init_b == 0);
    g_base_pop_init_result = -1;
    expect(pop_menu_init_redirect(menu, nullptr) == -1);
    expect_storage_bytes(pm.data(), pm_want.data(), pm.size());
    BasePopOriginalInit = saved_init;

    // InfoWin::reset is guarded: it must clear the flag *before* dispatching,
    // and must not dispatch at all when the flag is already clear.
    auto *const saved_timer = InfoWinOriginalTimerProc;
    InfoWinOriginalTimerProc = &observe_timer_proc;
    std::vector<uint8_t> iw(sizeof(InfoWin) + 32);
    std::vector<uint8_t> iw_want(iw.size());
    auto *info = reinterpret_cast<InfoWin *>(iw.data() + 16);
    auto set_flag = [&](int32_t value) {
        std::memcpy(iw.data() + 16 + 0xA8, &value, sizeof(value));
    };
    auto flag = [&] {
        int32_t value = 0;
        std::memcpy(&value, iw.data() + 16 + 0xA8, sizeof(value));
        return value;
    };

    seed_storage(iw.data(), iw_want.data(), iw.size());
    set_flag(0);
    std::memcpy(iw_want.data(), iw.data(), iw.size());
    g_timer_calls = 0;
    info->reset();
    expect(g_timer_calls == 0);
    expect_storage_bytes(iw.data(), iw_want.data(), iw.size());

    seed_storage(iw.data(), iw_want.data(), iw.size());
    set_flag(0x1234);
    std::memcpy(iw_want.data(), iw.data(), iw.size());
    g_timer_calls = 0;
    info->reset();
    expect(g_timer_calls == 1);
    expect(g_timer_self == info);
    expect(g_timer_arg == 1);
    expect(g_timer_flag_during_call == 0);
    expect(flag() == 0);
    std::memcpy(iw_want.data() + 16 + 0xA8, iw.data() + 16 + 0xA8, 4);
    expect_storage_bytes(iw.data(), iw_want.data(), iw.size());

    set_flag(-1);
    g_timer_calls = 0;
    info_win_reset_redirect(info, nullptr);
    expect(g_timer_calls == 1);
    expect(flag() == 0);
    InfoWinOriginalTimerProc = saved_timer;
}

namespace {
BasePop *g_closed_base_pop = nullptr;
int g_base_pop_close_calls = 0;
int g_scroll_order_at_base_close = -1;
void __thiscall observe_base_pop_close(BasePop *self) {
    g_closed_base_pop = self;
    // The scroll must already be closed by the time the base close runs.
    g_scroll_order_at_base_close = scroll_close_probe_state.order;
    ++g_base_pop_close_calls;
}
}  // namespace

void test_popup_close() {
    // 104 callers, the highest fan-in function left. It closes the Scroll at
    // 0x3230 - exactly sizeof(BasePop) - then the popup base. Scroll::close is
    // real recovered code, so it runs for real here against the shared
    // fixture, positioned at the member rather than at the popup: if close()
    // aimed at `this` instead of the member, none of the fixture would be
    // where Scroll::close looks and the probes would not fire.
    ScrollCloseFixture scroll_fixture;
    scroll_fixture.install();
    auto *const saved_close = BasePopOriginalClose;
    BasePopOriginalClose = &observe_base_pop_close;

    std::vector<uint8_t> storage(sizeof(Popup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *popup_object = reinterpret_cast<Popup *>(storage.data() + 16);
    uint8_t *const member = storage.data() + 16 + 0x3230;

    seed_storage(storage.data(), expected.data(), storage.size());
    scroll_fixture.prepare(member);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_base_pop_close_calls = 0;
    popup_object->close();

    expect(scroll_close_probe_state.order == 4);
    expect(g_base_pop_close_calls == 1);
    expect(reinterpret_cast<void *>(g_closed_base_pop) ==
           reinterpret_cast<void *>(popup_object));
    // Ordering: the scroll is fully closed before the base close runs.
    expect(g_scroll_order_at_base_close == 4);
    // Only the Scroll member changed; the BasePop region ahead of it is
    // untouched, which is what shows close() did not aim at the wrong object.
    std::memcpy(expected.data() + 16 + 0x3230, storage.data() + 16 + 0x3230,
                sizeof(Scroll));
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    seed_storage(storage.data(), expected.data(), storage.size());
    scroll_fixture.prepare(member);
    std::memcpy(expected.data(), storage.data(), storage.size());
    popup_close_redirect(popup_object, nullptr);
    expect(scroll_close_probe_state.order == 4);
    expect(g_base_pop_close_calls == 2);
    std::memcpy(expected.data() + 16 + 0x3230, storage.data() + 16 + 0x3230,
                sizeof(Scroll));
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    BasePopOriginalClose = saved_close;
    scroll_fixture.restore();
}

namespace {
Popup *g_hidden_popup = nullptr;
int g_popup_hide_calls = 0;
int g_flag_at_hide = -1;
void __thiscall observe_popup_hide(Popup *self) {
    g_hidden_popup = self;
    std::memcpy(&g_flag_at_hide,
                reinterpret_cast<const uint8_t *>(self) + 0x5384, sizeof(int));
    ++g_popup_hide_calls;
}

void *g_released_interface = nullptr;
int g_release_calls = 0;
int g_status_flag_at_release = -1;
const uint8_t *g_status_object = nullptr;
void __thiscall observe_release_iface_mode(void *self) {
    g_released_interface = self;
    std::memcpy(&g_status_flag_at_release, g_status_object + 0x15D4, sizeof(int));
    ++g_release_calls;
}
}  // namespace

void test_guarded_delegates() {
    // NetMsg::close stops its Time then hides the popup, but only when a
    // message is actually showing.
    auto *const saved_hide = PopupOriginalHide;
    PopupOriginalHide = &observe_popup_hide;
    std::vector<uint8_t> nm(sizeof(NetMsg) + 32), nm_want(nm.size());
    auto *message = reinterpret_cast<NetMsg *>(nm.data() + 16);
    auto set_showing = [&](int32_t value) {
        std::memcpy(nm.data() + 16 + 0x5384, &value, sizeof(value));
    };

    seed_storage(nm.data(), nm_want.data(), nm.size());
    set_showing(0);
    std::memcpy(nm_want.data(), nm.data(), nm.size());
    g_popup_hide_calls = 0;
    message->close();
    expect(g_popup_hide_calls == 0);
    expect_storage_bytes(nm.data(), nm_want.data(), nm.size());

    seed_storage(nm.data(), nm_want.data(), nm.size());
    set_showing(0x1234);
    std::memcpy(nm_want.data(), nm.data(), nm.size());
    g_popup_hide_calls = 0;
    message->close();
    expect(g_popup_hide_calls == 1);
    expect(reinterpret_cast<void *>(g_hidden_popup) ==
           reinterpret_cast<void *>(message));
    // The flag is cleared before the hide, not after.
    expect(g_flag_at_hide == 0);
    // Time::close cleared its own member and nothing else moved besides the
    // flag - in particular the Popup region ahead of 0x537C is untouched.
    std::memcpy(nm_want.data() + 16 + 0x5384, nm.data() + 16 + 0x5384, 4);
    std::memcpy(nm_want.data() + 16 + 0x538C, nm.data() + 16 + 0x538C,
                sizeof(Time));
    expect_storage_bytes(nm.data(), nm_want.data(), nm.size());

    set_showing(-1);
    g_popup_hide_calls = 0;
    net_msg_close_redirect(message, nullptr);
    expect(g_popup_hide_calls == 1);
    PopupOriginalHide = saved_hide;

    // StatusWin::reset releases the interface mode on a global object, and
    // the original tail-jumps into it, so nothing may follow the call.
    auto *const saved_release = SubInterfaceOriginalReleaseIfaceMode;
    void *const saved_global = SubInterfaceGlobal;
    int fake_interface = 0;
    SubInterfaceOriginalReleaseIfaceMode = &observe_release_iface_mode;
    SubInterfaceGlobal = &fake_interface;

    std::vector<uint8_t> sw(sizeof(StatusWin) + 32), sw_want(sw.size());
    auto *status = reinterpret_cast<StatusWin *>(sw.data() + 16);
    g_status_object = sw.data() + 16;
    auto set_held = [&](int32_t value) {
        std::memcpy(sw.data() + 16 + 0x15D4, &value, sizeof(value));
    };

    seed_storage(sw.data(), sw_want.data(), sw.size());
    set_held(0);
    std::memcpy(sw_want.data(), sw.data(), sw.size());
    g_release_calls = 0;
    status->reset();
    expect(g_release_calls == 0);
    expect_storage_bytes(sw.data(), sw_want.data(), sw.size());

    seed_storage(sw.data(), sw_want.data(), sw.size());
    set_held(0x5A5A);
    std::memcpy(sw_want.data(), sw.data(), sw.size());
    g_release_calls = 0;
    status->reset();
    expect(g_release_calls == 1);
    // The global is passed, not the status window - the original loads a
    // fixed address into ecx rather than forwarding `this`.
    expect(g_released_interface == &fake_interface);
    expect(g_status_flag_at_release == 0);
    std::memcpy(sw_want.data() + 16 + 0x15D4, sw.data() + 16 + 0x15D4, 4);
    expect_storage_bytes(sw.data(), sw_want.data(), sw.size());

    set_held(INT_MIN);
    g_release_calls = 0;
    status_win_reset_redirect(status, nullptr);
    expect(g_release_calls == 1);
    SubInterfaceOriginalReleaseIfaceMode = saved_release;
    SubInterfaceGlobal = saved_global;
}

namespace {
Dialog *g_closed_dialog = nullptr;
int g_dialog_close_calls = 0;
void __thiscall observe_dialog_close(Dialog *self) {
    g_closed_dialog = self;
    ++g_dialog_close_calls;
}

const void *g_win_closed = nullptr;
const void *g_buffer_closed = nullptr;
void __thiscall observe_win_close(void *self) { g_win_closed = self; }
void __thiscall observe_buffer_close(void *self) { g_buffer_closed = self; }
}  // namespace

void test_virtual_base_closes() {
    // RadioButton and CheckBox resolve both of their calls through the
    // vbtable - [edx+4] for the virtual GraphicWin, [edx+8] for the Dialog -
    // so the whole point of these tests is which subobject each call reaches.
    // GraphicWin::close is real recovered code needing its two subobject
    // seams and its default. The Scroll fixture cannot serve here - its
    // probes assert Scroll's own call ordering against a Scroll base - so
    // these get plain observers instead.
    uint32_t base_default = 0x0BADF00DU;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_default = GraphicWinFieldA0CDefault;
    WinOriginalClose = observe_win_close;
    BufferSubobjectClose = observe_buffer_close;
    GraphicWinFieldA0CDefault = &base_default;
    auto *const saved_dialog = RadioButtonOriginalDialogClose;
    RadioButtonOriginalDialogClose = &observe_dialog_close;
    auto *const saved_check_dialog = CheckBoxOriginalDialogClose;
    CheckBoxOriginalDialogClose = &observe_dialog_close;

    uint32_t radio_one = 0x11112222U;
    uint32_t radio_two = 0x33334444U;
    uint32_t *const saved_r1 = RadioButtonDefault1;
    uint32_t *const saved_r2 = RadioButtonDefault2;
    RadioButtonDefault1 = &radio_one;
    RadioButtonDefault2 = &radio_two;

    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        uint32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };

    // Two vbtables: the one a most-derived RadioButton uses, and a second
    // placing the same subobjects elsewhere, which is what happens when the
    // class is embedded in a larger one - Dialogs holds a RadioButton at 0x44.
    // Hardcoding this class's own offsets passes the first case and fails the
    // second, which is exactly what reached the game and crashed it.
    const int32_t own_vbtable[3] = {0, 0x18, 0xA30};
    const int32_t embedded_vbtable[3] = {0, 0x30, 0xA60};

    std::vector<uint8_t> rb(sizeof(RadioButton) + 0xA0 + 32);
    auto *radio = reinterpret_cast<RadioButton *>(rb.data() + 16);
    for (size_t i = 0; i < rb.size(); ++i) {
        rb[i] = static_cast<uint8_t>(0x40 + (i * 7));
    }
    const uint32_t zero = 0;
    auto point_at = [&](const int32_t *table) {
        const int32_t *pointer = table;
        std::memcpy(rb.data() + 16, &pointer, sizeof(pointer));
        // GraphicWin::close follows 0xA08 off whichever base the table names.
        std::memcpy(rb.data() + 16 + table[1] + 0xA08, &zero, sizeof(zero));
    };
    point_at(own_vbtable);
    g_dialog_close_calls = 0;
    radio->close();
    expect(g_dialog_close_calls == 1);
    // The Dialog it closed must be the one at 0xA30, not the object.
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == rb.data() + 16 + 0xA30);
    expect(read32(rb, 0x0C) == 0);
    expect(read32(rb, 0x10) == 0);
    expect(read32(rb, 0x08) == 0x33334444U);
    expect(read32(rb, 0x04) == 0x11112222U);
    // GraphicWin::close ran against the base at 0x18, not against the object:
    // it clears 0xA10 there and its two subobject closes see that address.
    expect(read32(rb, 0x18 + 0xA10) == 0);
    expect(g_win_closed == rb.data() + 16 + 0x18);
    expect(g_buffer_closed == rb.data() + 16 + 0x18 + 0x444);

    radio_button_close_redirect(radio, nullptr);
    expect(g_dialog_close_calls == 2);

    // Now the same object described by a vbtable that puts the base at 0x30
    // and the Dialog at 0xA60. Everything must follow the table.
    point_at(embedded_vbtable);
    g_dialog_close_calls = 0;
    radio->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == rb.data() + 16 + 0xA60);
    expect(g_win_closed == rb.data() + 16 + 0x30);
    expect(g_buffer_closed == rb.data() + 16 + 0x30 + 0x444);
    expect(read32(rb, 0x30 + 0xA10) == 0);

    uint32_t check_one = 0x55556666U;
    uint32_t check_two = 0x77778888U;
    uint32_t *const saved_c1 = CheckBoxDefault1;
    uint32_t *const saved_c2 = CheckBoxDefault2;
    CheckBoxDefault1 = &check_one;
    CheckBoxDefault2 = &check_two;

    const int32_t check_own[3] = {0, 0x1C, 0xA34};
    const int32_t check_embedded[3] = {0, 0x40, 0xA70};
    std::vector<uint8_t> cb(sizeof(CheckBox) + 0xA0 + 32);
    auto *check = reinterpret_cast<CheckBox *>(cb.data() + 16);
    for (size_t i = 0; i < cb.size(); ++i) {
        cb[i] = static_cast<uint8_t>(0x90 + (i * 5));
    }
    auto point_check_at = [&](const int32_t *table) {
        const int32_t *pointer = table;
        std::memcpy(cb.data() + 16, &pointer, sizeof(pointer));
        std::memcpy(cb.data() + 16 + table[1] + 0xA08, &zero, sizeof(zero));
    };
    point_check_at(check_own);
    g_dialog_close_calls = 0;
    check->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == cb.data() + 16 + 0xA34);
    expect(read32(cb, 0x04) == 0);
    expect(read32(cb, 0x08) == 0);
    expect(read32(cb, 0x0C) == 0);
    expect(read32(cb, 0x14) == 0x77778888U);
    expect(read32(cb, 0x10) == 0x55556666U);
    expect(read32(cb, 0x1C + 0xA10) == 0);
    expect(g_win_closed == cb.data() + 16 + 0x1C);
    expect(g_buffer_closed == cb.data() + 16 + 0x1C + 0x444);
    check_box_close_redirect(check, nullptr);
    expect(g_dialog_close_calls == 2);

    point_check_at(check_embedded);
    g_dialog_close_calls = 0;
    check->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == cb.data() + 16 + 0xA70);
    expect(g_win_closed == cb.data() + 16 + 0x40);
    expect(read32(cb, 0x40 + 0xA10) == 0);

    RadioButtonDefault1 = saved_r1;
    RadioButtonDefault2 = saved_r2;
    CheckBoxDefault1 = saved_c1;
    CheckBoxDefault2 = saved_c2;
    RadioButtonOriginalDialogClose = saved_dialog;
    CheckBoxOriginalDialogClose = saved_check_dialog;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_default;
}

namespace {
void *g_edit_box = nullptr;
char *g_edit_text = nullptr;
int g_set_text_calls = 0;
void __thiscall observe_edit_box_set_text(void *self, char *text) {
    g_edit_box = self;
    g_edit_text = text;
    ++g_set_text_calls;
}
}  // namespace

void test_edit_group_text() {
    auto *const saved = EditBoxOriginalSetText;
    EditBoxOriginalSetText = &observe_edit_box_set_text;

    std::vector<uint8_t> storage(sizeof(EditGroup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *group = reinterpret_cast<EditGroup *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());

    // Two boxes present, one absent - the absent slot is what both accessors
    // guard on, and the original returns nothing rather than an offset from
    // null, which is the difference a missing check would produce.
    uint8_t first_box[0xA20] = {};
    uint8_t second_box[0xA20] = {};
    void *slots[3] = {first_box, nullptr, second_box};
    for (size_t index = 0; index < 3; ++index) {
        std::memcpy(storage.data() + 16 + 4 + index * 4, &slots[index],
                    sizeof(void *));
    }
    std::memcpy(expected.data(), storage.data(), storage.size());

    // The text buffer sits 0xA14 into the box, not at its start.
    expect(group->get_text(0) == reinterpret_cast<char *>(first_box) + 0xA14);
    expect(group->get_text(1) == nullptr);
    expect(group->get_text(2) == reinterpret_cast<char *>(second_box) + 0xA14);
    expect(edit_group_get_text_redirect(group, nullptr, 2) ==
           reinterpret_cast<char *>(second_box) + 0xA14);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    char message[] = "hello";
    g_set_text_calls = 0;
    group->set_text(message, 0);
    expect(g_set_text_calls == 1);
    expect(g_edit_box == first_box);
    expect(g_edit_text == message);
    group->set_text(message, 1);          // absent: must not dispatch
    expect(g_set_text_calls == 1);
    edit_group_set_text_redirect(group, nullptr, message, 2);
    expect(g_set_text_calls == 2);
    expect(g_edit_box == second_box);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // set_text_limits fills exactly ten dwords from 0x54 and nothing beyond.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    group->set_text_limits(0x5A5A5A5A);
    for (size_t index = 0; index < 10; ++index) {
        uint32_t value = 0;
        std::memcpy(&value, storage.data() + 16 + 0x54 + index * 4, sizeof(value));
        expect(value == 0x5A5A5A5AU);
    }
    std::memcpy(expected.data() + 16 + 0x54, storage.data() + 16 + 0x54, 10 * 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    edit_group_set_text_limits_redirect(group, nullptr, -1);
    std::memcpy(expected.data() + 16 + 0x54, storage.data() + 16 + 0x54, 10 * 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    EditBoxOriginalSetText = saved;
}

namespace {
Dialog *g_item_dialog = nullptr;
void *g_item_listbox = nullptr;
char *g_item_text = nullptr;
int g_item_index = 0;
int g_dialog_item_calls = 0;
int g_listbox_item_calls = 0;
int __thiscall observe_dialog_item(Dialog *self, char *text, int index) {
    g_item_dialog = self; g_item_text = text; g_item_index = index;
    ++g_dialog_item_calls;
    return 0x1234;
}
int __thiscall observe_list_box_item(void *self, char *text, int index) {
    g_item_listbox = self; g_item_text = text; g_item_index = index;
    ++g_listbox_item_calls;
    return 0x5678;
}
}  // namespace

void test_dialogs_dispatch() {
    // Both methods switch on the dialog kind through a sixteen entry jump
    // table, so every kind is exercised rather than a representative few -
    // the table is not regular, and a case transcribed into the wrong arm
    // would pass any test that only sampled it.
    auto *const saved_dialog = DialogOriginalItem;
    auto *const saved_listbox = ListBoxOriginalItem;
    DialogOriginalItem = &observe_dialog_item;
    ListBoxOriginalItem = &observe_list_box_item;

    std::vector<uint8_t> storage(sizeof(Dialogs) + 0x100 + 32);
    auto *dialogs = reinterpret_cast<Dialogs *>(storage.data() + 16);
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x30 + (i * 3));
    }
    // A vbtable placing the Dialog somewhere other than a most-derived
    // Dialogs would put it, so a hardcoded offset cannot pass.
    const int32_t vbtable[3] = {0, 0x188, 0xC40};
    const int32_t *pointer = vbtable;
    std::memcpy(storage.data() + 16, &pointer, sizeof(pointer));
    uint8_t *const dialog_at = storage.data() + 16 + 0xC40;
    const int32_t count = 0x0BADBEEF;
    std::memcpy(dialog_at + 0xCC, &count, sizeof(count));
    const int32_t fallback = 0x00C0FFEE;
    std::memcpy(storage.data() + 16 + 0xA4, &fallback, sizeof(fallback));

    auto set_kind = [&](int32_t kind) {
        std::memcpy(storage.data() + 16 + 0x180, &kind, sizeof(kind));
    };
    char text[] = "item";

    // item: kinds 1, 4, 8 and 16 go to the Dialog; kind 2 to the ListBox;
    // everything else, in range or not, yields zero.
    const int dialog_kinds[] = {1, 4, 8, 16};
    for (int kind : dialog_kinds) {
        set_kind(kind);
        g_dialog_item_calls = 0;
        expect(dialogs->item(text, 7) == 0x1234);
        expect(g_dialog_item_calls == 1);
        expect(reinterpret_cast<uint8_t *>(g_item_dialog) == dialog_at);
        expect(g_item_text == text);
        expect(g_item_index == 7);
    }
    set_kind(2);
    g_listbox_item_calls = 0;
    expect(dialogs->item(text, 3) == 0x5678);
    expect(g_listbox_item_calls == 1);
    // The ListBox arm passes the object itself, unadjusted.
    expect(g_item_listbox == reinterpret_cast<void *>(dialogs));
    for (int kind : {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15}) {
        set_kind(kind);
        g_dialog_item_calls = 0;
        g_listbox_item_calls = 0;
        expect(dialogs->item(text, 0) == 0);
        expect(g_dialog_item_calls == 0);
        expect(g_listbox_item_calls == 0);
    }
    for (int kind : {0, -1, 17, INT_MIN, INT_MAX}) {
        set_kind(kind);
        expect(dialogs->item(text, 0) == 0);
    }
    set_kind(1);
    expect(dialogs_item_redirect(dialogs, nullptr, text, 2) == 0x1234);

    // get_num_items: kinds 1, 2, 4 and 16 read the Dialog's count, kind 8
    // reads the object's own field, the rest are zero. Note kind 2 differs
    // between the two methods, which is the kind of asymmetry a shared
    // switch would paper over.
    for (int kind : {1, 2, 4, 16}) {
        set_kind(kind);
        expect(dialogs->get_num_items() == 0x0BADBEEF);
    }
    set_kind(8);
    expect(dialogs->get_num_items() == 0x00C0FFEE);
    for (int kind : {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 0, 17}) {
        set_kind(kind);
        expect(dialogs->get_num_items() == 0);
    }
    set_kind(4);
    expect(dialogs_get_num_items_redirect(dialogs, nullptr) == 0x0BADBEEF);

    DialogOriginalItem = saved_dialog;
    ListBoxOriginalItem = saved_listbox;
}

namespace {
struct XPopsCall {
    char *buffer; const char *label; int title; char *override_text;
    int value; Sprite *sprite; int flag1; int flag2; int (__cdecl *callback)();
    int calls;
} g_xpops = {};
int __cdecl observe_x_pops(char *buffer, const char *label, int title,
                           char *override_text, int value, Sprite *sprite,
                           int flag1, int flag2, int (__cdecl *callback)()) {
    g_xpops = {buffer, label, title, override_text, value, sprite,
               flag1, flag2, callback, g_xpops.calls + 1};
    return 0x4321;
}
int __cdecl xpops_callback() { return 0; }

void *g_date_target = nullptr;
char *g_date_text = nullptr;
int g_set_date_calls = 0;
void __thiscall observe_set_date(void *self, char *text) {
    g_date_target = self; g_date_text = text; ++g_set_date_calls;
}

float g_rotation[3] = {};
void *g_rotation_target = nullptr;
int g_rotation_calls = 0;
void __cdecl observe_apply_rotation(float *angles, void *matrix) {
    std::memcpy(g_rotation, angles, sizeof(g_rotation));
    g_rotation_target = matrix;
    ++g_rotation_calls;
}
}  // namespace

void test_fixed_argument_delegates() {
    // x_pops_short supplies five of the nine arguments itself. Their values
    // and their order are the entire content of the function, so each is
    // checked rather than just that the call happened.
    auto *const saved_full = XPopsOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopsOriginalFull = &observe_x_pops;
    XPopsCaptionBuffer = buffer;
    Sprite sprite_value;
    const char label[] = "label";
    g_xpops.calls = 0;
    expect(x_pops_short(label, 42, &sprite_value, &xpops_callback) == 0x4321);
    expect(g_xpops.calls == 1);
    expect(g_xpops.buffer == buffer);
    expect(g_xpops.label == label);
    expect(g_xpops.title == -1);
    expect(g_xpops.override_text == nullptr);
    expect(g_xpops.value == 42);
    expect(g_xpops.sprite == &sprite_value);
    expect(g_xpops.flag1 == 1);
    expect(g_xpops.flag2 == 1);
    expect(g_xpops.callback == &xpops_callback);
    XPopsOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;

    // main_caption acts on a fixed interface and caption, never on the map
    // window it is called through - passing `this` would compile and be wrong.
    auto *const saved_date = MainInterfaceOriginalSetDate;
    void *const saved_interface = MainInterfaceGlobal;
    char *const saved_caption = MapWinMainCaption;
    int fake_interface = 0;
    char caption[] = "2101.01";
    MainInterfaceOriginalSetDate = &observe_set_date;
    MainInterfaceGlobal = &fake_interface;
    MapWinMainCaption = caption;
    std::vector<uint8_t> mw(sizeof(MapWin) + 32);
    std::vector<uint8_t> mw_want(mw.size());
    auto *map_window = reinterpret_cast<MapWin *>(mw.data() + 16);
    seed_storage(mw.data(), mw_want.data(), mw.size());
    std::memcpy(mw_want.data(), mw.data(), mw.size());
    g_set_date_calls = 0;
    map_window->main_caption();
    expect(g_set_date_calls == 1);
    expect(g_date_target == &fake_interface);
    expect(g_date_target != reinterpret_cast<void *>(map_window));
    expect(g_date_text == caption);
    expect_storage_bytes(mw.data(), mw_want.data(), mw.size());
    map_win_main_caption_redirect(map_window, nullptr);
    expect(g_set_date_calls == 2);
    MainInterfaceOriginalSetDate = saved_date;
    MainInterfaceGlobal = saved_interface;
    MapWinMainCaption = saved_caption;

    // set_scene_rotation passes the three angles in order and the matrix at
    // 0x38, not the object.
    auto *const saved_rotation = CaviarOriginalApplyRotation;
    CaviarOriginalApplyRotation = &observe_apply_rotation;
    std::vector<uint8_t> cv(sizeof(Caviar) + 32);
    std::vector<uint8_t> cv_want(cv.size());
    auto *caviar = reinterpret_cast<Caviar *>(cv.data() + 16);
    seed_storage(cv.data(), cv_want.data(), cv.size());
    std::memcpy(cv_want.data(), cv.data(), cv.size());
    g_rotation_calls = 0;
    caviar->set_scene_rotation(1.5f, -2.25f, 0.125f);
    expect(g_rotation_calls == 1);
    expect(g_rotation[0] == 1.5f);
    expect(g_rotation[1] == -2.25f);
    expect(g_rotation[2] == 0.125f);
    expect(g_rotation_target == cv.data() + 16 + 0x38);
    expect_storage_bytes(cv.data(), cv_want.data(), cv.size());
    caviar_set_scene_rotation_redirect(caviar, nullptr, 3.0f, 4.0f, 5.0f);
    expect(g_rotation[0] == 3.0f);
    expect(g_rotation[2] == 5.0f);
    CaviarOriginalApplyRotation = saved_rotation;
}

namespace {
void *g_fill_target = nullptr;
int g_fill_args[5] = {};
int g_fill_calls = 0;
int __thiscall observe_buffer_fill(void *self, int a, int b, int c, int d, int e) {
    g_fill_target = self;
    g_fill_args[0]=a; g_fill_args[1]=b; g_fill_args[2]=c; g_fill_args[3]=d; g_fill_args[4]=e;
    ++g_fill_calls;
    return 0x9ABC;
}

void *g_check_target = nullptr;
long g_check_value = 0;
int g_check_calls = 0;
void __thiscall observe_set_state_flag(void *self, long value) {
    g_check_target = self; g_check_value = value; ++g_check_calls;
}

struct StartCall {
    Popup *self; char *a1; const char *a2; int a3; char *a4; int a5; void *parent;
    int calls;
} g_start = {};
void __thiscall observe_popup_start_full(Popup *self, char *a1, const char *a2,
                                         int a3, char *a4, int a5, void *parent) {
    g_start = {self, a1, a2, a3, a4, a5, parent, g_start.calls + 1};
}
}  // namespace

void test_offset_delegates() {
    // GraphicWin::fill forwards to Buffer::fill on the buffer at 0x444, with
    // the five arguments in order. The point is the target is the member, not
    // the window, and the order survives.
    auto *const saved_fill = BufferOriginalFill;
    BufferOriginalFill = &observe_buffer_fill;
    std::vector<uint8_t> gw(sizeof(GraphicWin) + 32);
    std::vector<uint8_t> gw_want(gw.size());
    auto *window = reinterpret_cast<GraphicWin *>(gw.data() + 16);
    seed_storage(gw.data(), gw_want.data(), gw.size());
    std::memcpy(gw_want.data(), gw.data(), gw.size());
    g_fill_calls = 0;
    expect(window->fill(11, 22, 33, 44, 55) == 0x9ABC);
    expect(g_fill_calls == 1);
    expect(g_fill_target == gw.data() + 16 + 0x444);
    expect(g_fill_args[0] == 11 && g_fill_args[4] == 55);
    expect(g_fill_args[1] == 22 && g_fill_args[2] == 33 && g_fill_args[3] == 44);
    expect_storage_bytes(gw.data(), gw_want.data(), gw.size());
    graphic_win_fill_redirect(window, nullptr, -1, -2, -3, -4, -5);
    expect(g_fill_args[0] == -1 && g_fill_args[4] == -5);
    BufferOriginalFill = saved_fill;

    // BasePop::write_check forwards to CheckBox::set_state_flag on the member
    // at 0x2228, passing the value through.
    auto *const saved_check = CheckBoxOriginalSetStateFlag;
    CheckBoxOriginalSetStateFlag = &observe_set_state_flag;
    std::vector<uint8_t> bp(sizeof(BasePop) + 32);
    std::vector<uint8_t> bp_want(bp.size());
    auto *popup = reinterpret_cast<BasePop *>(bp.data() + 16);
    seed_storage(bp.data(), bp_want.data(), bp.size());
    std::memcpy(bp_want.data(), bp.data(), bp.size());
    g_check_calls = 0;
    popup->write_check(0x12345678L);
    expect(g_check_calls == 1);
    expect(g_check_target == bp.data() + 16 + 0x2228);
    expect(g_check_value == 0x12345678L);
    expect_storage_bytes(bp.data(), bp_want.data(), bp.size());
    base_pop_write_check_redirect(popup, nullptr, -1L);
    expect(g_check_value == -1L);
    CheckBoxOriginalSetStateFlag = saved_check;

    // Popup::start forwards its five arguments and appends a null parent.
    auto *const saved_start = PopupOriginalStartFull;
    PopupOriginalStartFull = &observe_popup_start_full;
    std::vector<uint8_t> pu(sizeof(Popup) + 32);
    auto *pstart = reinterpret_cast<Popup *>(pu.data() + 16);
    char a1[] = "a"; const char a2[] = "b"; char a4[] = "d";
    g_start.calls = 0;
    pstart->start(a1, a2, 7, a4, 9);
    expect(g_start.calls == 1);
    expect(g_start.self == pstart);
    expect(g_start.a1 == a1 && g_start.a2 == a2 && g_start.a4 == a4);
    expect(g_start.a3 == 7 && g_start.a5 == 9);
    expect(g_start.parent == nullptr);
    popup_start_redirect(pstart, nullptr, a1, a2, -3, a4, -5);
    expect(g_start.a3 == -3 && g_start.a5 == -5 && g_start.parent == nullptr);
    PopupOriginalStartFull = saved_start;
}

void test_win_sync_palette() {
    // sync_palette caches the active palette's generation counter at 0x184 and
    // skips the update when it already matches. set_active_window happens to
    // be an empty stub, but the guard is what this checks: the counter is read
    // again after the call, so a version that cached the stale value would be
    // caught if the counter ever changed across it, and the early return must
    // touch nothing when the generations already agree.
    // Rebind the seam to a local slot rather than writing through the fixed
    // address, which is unmapped here - the recovery only reads *WinActivePalette.
    Palette **const saved = WinActivePalette;
    alignas(Palette) uint8_t palette_storage[sizeof(Palette)] = {};
    auto *palette = reinterpret_cast<Palette *>(palette_storage);
    Palette *palette_slot = palette;
    WinActivePalette = &palette_slot;
    auto set_generation = [&](uint32_t value) {
        std::memcpy(palette_storage + 0x400, &value, sizeof(value));
    };

    std::vector<uint8_t> wn(sizeof(Win) + 32);
    std::vector<uint8_t> wn_want(wn.size());
    auto *window = reinterpret_cast<Win *>(wn.data() + 16);
    auto cached = [&] {
        uint32_t value = 0;
        std::memcpy(&value, wn.data() + 16 + 0x184, sizeof(value));
        return value;
    };
    auto set_cached = [&](uint32_t value) {
        std::memcpy(wn.data() + 16 + 0x184, &value, sizeof(value));
    };

    // Already in step: nothing happens, not even a write of the same value.
    seed_storage(wn.data(), wn_want.data(), wn.size());
    set_generation(0x1111);
    set_cached(0x1111);
    std::memcpy(wn_want.data(), wn.data(), wn.size());
    window->sync_palette();
    expect(cached() == 0x1111);
    expect_storage_bytes(wn.data(), wn_want.data(), wn.size());

    // Behind: the cache is brought up to the palette's generation.
    seed_storage(wn.data(), wn_want.data(), wn.size());
    set_generation(0xABCD);
    set_cached(0x2222);
    std::memcpy(wn_want.data(), wn.data(), wn.size());
    window->sync_palette();
    expect(cached() == 0xABCD);
    std::memcpy(wn_want.data() + 16 + 0x184, wn.data() + 16 + 0x184, 4);
    expect_storage_bytes(wn.data(), wn_want.data(), wn.size());

    set_generation(0x7F7F);
    set_cached(0);
    win_sync_palette_redirect(window, nullptr);
    expect(cached() == 0x7F7F);

    WinActivePalette = saved;
}

void test_self_contained_stores() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };

    // PlanWin::clear_lines zeroes one dword at 0x21FF8, carved out of the
    // pinned derived storage. If the carve moved it, the build's static_assert
    // would have failed; here the check is only that the right dword clears
    // and nothing else does.
    std::vector<uint8_t> pw(sizeof(PlanWin) + 32);
    std::vector<uint8_t> pw_want(pw.size());
    auto *plan = reinterpret_cast<PlanWin *>(pw.data() + 16);
    seed_storage(pw.data(), pw_want.data(), pw.size());
    std::memcpy(pw_want.data(), pw.data(), pw.size());
    plan->clear_lines();
    expect(read32(pw, 0x21FF8) == 0);
    std::memcpy(pw_want.data() + 16 + 0x21FF8, pw.data() + 16 + 0x21FF8, 4);
    expect_storage_bytes(pw.data(), pw_want.data(), pw.size());
    plan_win_clear_lines_redirect(plan, nullptr);

    // PlayerLock::clear resets two three-dword entries and the flag byte at 0.
    // The two -1 sentinels and the zero within each entry are asserted per
    // field: a loop that wrote them in the wrong order, or cleared where it
    // should sentinel, would pass a check that only looked at the whole entry.
    std::vector<uint8_t> pl(sizeof(PlayerLock) + 32);
    std::vector<uint8_t> pl_want(pl.size());
    auto *lock = reinterpret_cast<PlayerLock *>(pl.data() + 16);
    seed_storage(pl.data(), pl_want.data(), pl.size());
    std::memcpy(pl_want.data(), pl.data(), pl.size());
    lock->clear();
    expect((pl[16] & 0xFF) == 0);          // active flag byte at 0
    expect(read32(pl, 0x04) == -1);
    expect(read32(pl, 0x08) == -1);
    expect(read32(pl, 0x0C) == 0);
    expect(read32(pl, 0x10) == -1);
    expect(read32(pl, 0x14) == -1);
    expect(read32(pl, 0x18) == 0);
    // Everything written sits in 0..0x1C; the byte at 0 plus the two entries.
    pl_want[16] = pl[16];
    std::memcpy(pl_want.data() + 16 + 4, pl.data() + 16 + 4, 0x18);
    expect_storage_bytes(pl.data(), pl_want.data(), pl.size());
    player_lock_clear_redirect(lock, nullptr);
}

void test_base_pop_set_width() {
    // set_width writes into the Dialog subobject of the embedded Dialogs at
    // 0x21D0, located through that Dialogs' own vbtable, and scales the width
    // by three-halves in the high-resolution layout unless the popup opts out.
    // Both the scaling condition (three independent ways to disable it) and
    // the vbtable-relative store location are what the test pins.
    int32_t screen_width = 0x400;
    int32_t *const saved_screen = BasePopScreenWidth;
    BasePopScreenWidth = &screen_width;

    std::vector<uint8_t> storage(sizeof(BasePop) + 0x40);
    std::vector<uint8_t> expected(storage.size());
    auto *popup = reinterpret_cast<BasePop *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;

    // A vbtable whose Dialog offset is not where a most-derived layout would
    // put it, so a hardcoded store offset could not pass.
    const int32_t vbtable[3] = {0, 0x188, 0x400};
    const int32_t *vbtable_pointer = vbtable;
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    const size_t store_at = 0x21D0 + 0x2C + 0x400;

    auto set_u32 = [&](size_t off, uint32_t v) {
        std::memcpy(object + off, &v, sizeof(v));
    };
    auto stored = [&] {
        int32_t v = 0;
        std::memcpy(&v, object + store_at, sizeof(v));
        return v;
    };

    // All three conditions permit scaling: field_A14 zero, style bit clear,
    // screen wide enough. 100 -> 150.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0);
    screen_width = 0x400;
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    popup->set_width(100);
    expect(stored() == 150);
    // Negative widths truncate toward zero, as the asm's cdq/sub/sar does.
    popup->set_width(-100);
    expect(stored() == -150);
    popup->set_width(3);
    expect(stored() == 4);            // (3*3)/2 = 4 truncated
    popup->set_width(-3);
    expect(stored() == -4);           // (-9)/2 = -4 toward zero

    // Each condition alone forces the raw width.
    set_u32(0xA14, 1);
    popup->set_width(100);
    expect(stored() == 100);
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0x400);
    popup->set_width(100);
    expect(stored() == 100);
    set_u32(0x30A8, 0);
    screen_width = 0x3FF;
    popup->set_width(100);
    expect(stored() == 100);
    screen_width = 0x400;

    // The store lands only at the vbtable-derived location - the rest of the
    // object, including the Dialogs vbtable pointer, is left alone.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0);
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    std::memcpy(expected.data(), storage.data(), storage.size());
    popup->set_width(42);
    expect(stored() == 63);
    std::memcpy(expected.data() + 16 + store_at, object + store_at, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    base_pop_set_width_redirect(popup, nullptr, 200);
    expect(stored() == 300);

    BasePopScreenWidth = saved_screen;
}

namespace {
void *g_mapwin_freed = nullptr;
int g_mapwin_free_calls = 0;
void *observe_map_win_free(void *pointer) {
    g_mapwin_freed = pointer;
    ++g_mapwin_free_calls;
    return nullptr;
}
}  // namespace

void test_map_win_close() {
    // close() frees the buffer at 0x4 and then closes the GraphicWin base,
    // located through the object's own vbtable. GraphicWin::close is real
    // recovered code needing its two subobject seams and its default; it does
    // not do Scroll's left/right dispatch, so it gets plain observers rather
    // than the Scroll fixture. The vbase offset installed here is not where a
    // most-derived MapWin would put it, so a hardcoded base offset cannot pass.
    uint32_t base_default = 0x0BADF00DU;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_default = GraphicWinFieldA0CDefault;
    WinOriginalClose = observe_win_close;
    BufferSubobjectClose = observe_buffer_close;
    GraphicWinFieldA0CDefault = &base_default;
    auto *const saved_free = MapWinFree;
    MapWinFree = &observe_map_win_free;

    const int32_t vbase = 0x1000;
    const int32_t vbtable[2] = {0, vbase};
    const int32_t *vbtable_pointer = vbtable;

    std::vector<uint8_t> storage(sizeof(MapWin) + 32);
    auto *window = reinterpret_cast<MapWin *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;
    auto prepare_base = [&] {
        std::memcpy(object, &vbtable_pointer, sizeof(vbtable_pointer));
        const uint32_t zero = 0;
        std::memcpy(object + vbase + 0xA08, &zero, sizeof(zero));  // release ptr
    };

    // Owned pointer present: it is freed and the slot cleared, then the base
    // close runs at the vbtable offset.
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x20 + (i * 7));
    }
    prepare_base();
    void *owned = reinterpret_cast<void *>(0xDEADBEEF);
    std::memcpy(object + 4, &owned, sizeof(owned));
    g_win_closed = nullptr;
    g_buffer_closed = nullptr;
    g_mapwin_free_calls = 0;
    window->close();
    expect(g_mapwin_free_calls == 1);
    expect(g_mapwin_freed == owned);
    void *after = reinterpret_cast<void *>(0x1);
    std::memcpy(&after, object + 4, sizeof(after));
    expect(after == nullptr);
    expect(g_win_closed == object + vbase);
    expect(g_buffer_closed == object + vbase + 0x444);

    // No owned pointer: free is skipped, base close still runs.
    prepare_base();
    void *const null_owned = nullptr;
    std::memcpy(object + 4, &null_owned, sizeof(null_owned));
    g_win_closed = nullptr;
    g_mapwin_free_calls = 0;
    window->close();
    expect(g_mapwin_free_calls == 0);
    expect(g_win_closed == object + vbase);

    std::memcpy(object + 4, &owned, sizeof(owned));
    prepare_base();
    g_mapwin_free_calls = 0;
    map_win_close_redirect(window, nullptr);
    expect(g_mapwin_free_calls == 1);

    MapWinFree = saved_free;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_default;
}

namespace {
void *g_struct_add_self = nullptr;
int g_struct_add_index = 0;
int g_struct_add_result = 0;
int g_struct_add_calls = 0;
int __thiscall observe_struct_add(void *self, int index) {
    g_struct_add_self = self; g_struct_add_index = index; ++g_struct_add_calls;
    return g_struct_add_result;
}
StringBox *g_fixup_self = nullptr;
int g_fixup_calls = 0;
void __thiscall observe_add_fixup(StringBox *self) {
    g_fixup_self = self; ++g_fixup_calls;
}
}  // namespace

void test_string_box_add() {
    auto *const saved_add = StringBoxStructAdd;
    auto *const saved_fixup = StringBoxAddFixup;
    StringBoxStructAdd = &observe_struct_add;
    StringBoxAddFixup = &observe_add_fixup;

    std::vector<uint8_t> storage(sizeof(StringBox) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *box = reinterpret_cast<StringBox *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;
    auto read_ptr = [&](size_t off) {
        void *v = nullptr;
        std::memcpy(&v, object + off, sizeof(v));
        return v;
    };
    auto read32 = [&](size_t off) {
        uint32_t v = 0;
        std::memcpy(&v, object + off, sizeof(v));
        return v;
    };

    char text[] = "entry";

    // Struct add reports success (nonzero): the three fields are staged, add
    // is called on the struct member at 0x2B70, and the fixup is skipped.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_struct_add_result = 1;
    g_struct_add_calls = 0;
    g_fixup_calls = 0;
    box->add(text, 7, 0x55);
    expect(g_struct_add_calls == 1);
    expect(g_struct_add_self == object + 0x2B70);
    expect(g_struct_add_index == 7);
    expect(g_fixup_calls == 0);
    expect(read_ptr(0x2B8C) == text);
    expect(read32(0x2B90) == 0x55);
    expect(read32(0x2B94) == 0);
    // Only the three staged fields moved.
    std::memcpy(expected.data() + 16 + 0x2B8C, object + 0x2B8C, 4);
    std::memcpy(expected.data() + 16 + 0x2B90, object + 0x2B90, 8);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Struct add reports failure (zero): the fixup runs, on the box itself.
    g_struct_add_result = 0;
    g_struct_add_calls = 0;
    g_fixup_calls = 0;
    box->add(text, -3, 0);
    expect(g_struct_add_calls == 1);
    expect(g_struct_add_index == -3);
    expect(g_fixup_calls == 1);
    expect(g_fixup_self == box);

    // Redirect: same, and the index passes through.
    g_struct_add_result = 1;
    g_fixup_calls = 0;
    string_box_add_redirect(box, nullptr, text, INT_MIN, 0);
    expect(g_struct_add_index == INT_MIN);
    expect(g_fixup_calls == 0);

    StringBoxStructAdd = saved_add;
    StringBoxAddFixup = saved_fixup;
}

namespace {
void *g_link_freed[24] = {};
int g_link_free_calls = 0;
void *observe_buffer_free(void *pointer) {
    if (g_link_free_calls < 24) g_link_freed[g_link_free_calls] = pointer;
    ++g_link_free_calls;
    return nullptr;
}
}  // namespace

void test_buffer_clear_links() {
    // clear_links reinitialises the spot list, zeroes the count at 0x4AC, and
    // frees the twenty owned link pointers at 0x4BC through the CRT boundary,
    // skipping the null ones. Spot::init is real recovered code, so its
    // allocation and its shutdown-free run for real here.
    auto *const saved_free = BufferFree;
    BufferFree = &observe_buffer_free;

    std::vector<uint8_t> storage(sizeof(Buffer) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *buffer = reinterpret_cast<Buffer *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;

    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x30 + (i * 7));
    }
    // The embedded Spot at 0x4B0 must have a null spots_ pointer so its
    // shutdown does not free seeded garbage; Spot::init allocates a fresh one.
    const void *const null_ptr = nullptr;
    std::memcpy(object + 0x4B0, &null_ptr, sizeof(null_ptr));

    // Ten of the twenty links point at real allocations, ten are null.
    void *links[20] = {};
    for (int i = 0; i < 20; ++i) {
        links[i] = (i % 2 == 0) ? std::malloc(8) : nullptr;
        std::memcpy(object + 0x4BC + i * 4, &links[i], sizeof(void *));
    }

    mem_get_calls = 0;
    g_link_free_calls = 0;
    buffer->clear_links();

    // Spot::init ran: it allocated once (the shutdown free of a null pointer
    // is a no-op).
    expect(mem_get_calls == 1);
    // The count at 0x4AC is cleared.
    uint32_t count = 0xFFFFFFFF;
    std::memcpy(&count, object + 0x4AC, sizeof(count));
    expect(count == 0);
    // Exactly the ten non-null links were freed, in order, and each slot is
    // now null; the ten null slots were never freed and stay null.
    expect(g_link_free_calls == 10);
    for (int i = 0; i < 20; ++i) {
        void *slot = reinterpret_cast<void *>(0x1);
        std::memcpy(&slot, object + 0x4BC + i * 4, sizeof(slot));
        expect(slot == nullptr);
    }
    for (int i = 0; i < 10; ++i) {
        expect(g_link_freed[i] == links[i * 2]);
    }

    // Redirect: a fresh object frees its non-null links the same way.
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x11 + (i * 3));
    }
    std::memcpy(object + 0x4B0, &null_ptr, sizeof(null_ptr));
    void *one = std::malloc(8);
    for (int i = 0; i < 20; ++i) {
        const void *v = (i == 5) ? one : nullptr;
        std::memcpy(object + 0x4BC + i * 4, &v, sizeof(v));
    }
    g_link_free_calls = 0;
    buffer_clear_links_redirect(buffer, nullptr);
    expect(g_link_free_calls == 1);
    expect(g_link_freed[0] == one);

    BufferFree = saved_free;
}

namespace {
void *g_net_get_net = nullptr;
unsigned long g_net_get_first_out = 0;
unsigned long g_net_get_second_out = 0;
int g_net_get_result = 0;
int g_net_get_calls = 0;
int __thiscall observe_net_get(void *net, unsigned long *a, unsigned long *b) {
    g_net_get_net = net;
    *a = g_net_get_first_out;
    *b = g_net_get_second_out;
    ++g_net_get_calls;
    return g_net_get_result;
}
NetDaemon *g_pm_self = nullptr;
char *g_pm_message = nullptr;
unsigned long g_pm_a = 0;
int g_pm_b = 0;
int g_pm_calls = 0;
void __thiscall observe_process_message(NetDaemon *self, char *message,
                                        unsigned long a, int b) {
    g_pm_self = self; g_pm_message = message; g_pm_a = a; g_pm_b = b;
    ++g_pm_calls;
}
}  // namespace

void test_net_daemon_receive() {
    auto *const saved_get = NetDaemonNetGet;
    auto *const saved_pm = NetDaemonProcessMessage;
    void *const saved_net = NetDaemonNet;
    int fake_net = 0;
    NetDaemonNetGet = &observe_net_get;
    NetDaemonProcessMessage = &observe_process_message;
    NetDaemonNet = &fake_net;

    std::vector<uint8_t> storage(sizeof(NetDaemon) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *daemon = reinterpret_cast<NetDaemon *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    // No message: get returns zero, process_message is not called, receive
    // reports 0, and nothing on the object moves.
    g_net_get_result = 0;
    g_net_get_calls = 0;
    g_pm_calls = 0;
    expect(daemon->receive() == 0);
    expect(g_net_get_calls == 1);
    expect(g_net_get_net == &fake_net);   // reads the global Net, not `this`
    expect(g_pm_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A message: get returns nonzero and fills the two out-parameters, which
    // flow to process_message as (result, second, first) - the order the asm
    // pushes them in, which a naive reading would swap.
    g_net_get_result = 0x00ABCDEF;
    g_net_get_first_out = 0x11111111;
    g_net_get_second_out = 0x22222222;
    g_net_get_calls = 0;
    g_pm_calls = 0;
    expect(daemon->receive() == 1);
    expect(g_net_get_calls == 1);
    expect(g_pm_calls == 1);
    expect(g_pm_self == daemon);
    expect(g_pm_message == reinterpret_cast<char *>(0x00ABCDEF));
    expect(g_pm_a == 0x22222222);     // second out-parameter
    expect(g_pm_b == 0x11111111);     // first out-parameter
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    g_pm_calls = 0;
    expect(net_daemon_receive_redirect(daemon, nullptr) == 1);
    expect(g_pm_calls == 1);

    NetDaemonNetGet = saved_get;
    NetDaemonProcessMessage = saved_pm;
    NetDaemonNet = saved_net;
}

void test_win_scroll_forwarders() {
    // The four forwarders send a position or range to the vertical Scroll at
    // 0x43C or the horizontal one at 0x440, guarding on null. Both Scroll
    // methods are real recovered code that dispatches through the redraw
    // vtable, so each member gets a full Scroll with that probe installed, and
    // the test checks the call landed on the right one - swapping vert and
    // horz would send it to the other scroll.
    Win **const saved_current_win = ScrollCurrentWin;
    Win *published = nullptr;
    ScrollCurrentWin = &published;

    alignas(Scroll) uint8_t vert[sizeof(Scroll) + 32] = {};
    alignas(Scroll) uint8_t horz[sizeof(Scroll) + 32] = {};
    uintptr_t vert_vtable[63];
    uintptr_t horz_vtable[63];
    install_scroll_redraw_probe(vert, vert_vtable);
    install_scroll_redraw_probe(horz, horz_vtable);
    auto *vert_scroll = reinterpret_cast<Scroll *>(vert + 16);
    auto *horz_scroll = reinterpret_cast<Scroll *>(horz + 16);
    // A parent so set_pos does not return early.
    Win *parent = reinterpret_cast<Win *>(0x33334444U);
    write_at(vert, 16 + 0xC4, parent);
    write_at(horz, 16 + 0xC4, parent);

    alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *window = reinterpret_cast<Win *>(win_storage + 16);
    auto point_scrolls = [&](Scroll *v, Scroll *h) {
        write_at(win_storage, 16 + 0x43C, v);
        write_at(win_storage, 16 + 0x440, h);
    };
    auto vert_range_min = [&] {
        uint32_t m = 0; std::memcpy(&m, vert + 16 + 0xA20, 4); return m;
    };
    auto horz_range_min = [&] {
        uint32_t m = 0; std::memcpy(&m, horz + 16 + 0xA20, 4); return m;
    };

    // set_vert_range hits the vertical scroll only.
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    point_scrolls(vert_scroll, horz_scroll);
    reset_scroll_redraw_probe();
    { uint32_t z = 0; std::memcpy(horz + 16 + 0xA20, &z, 4); }
    window->set_vert_range(0x1234, 0x5678);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == vert_scroll);
    expect(vert_range_min() == 0x1234);
    expect(horz_range_min() == 0);

    // set_horz_range hits the horizontal scroll only.
    reset_scroll_redraw_probe();
    { uint32_t z = 0; std::memcpy(vert + 16 + 0xA20, &z, 4); }
    window->set_horz_range(0x4321, 0x8765);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == horz_scroll);
    expect(horz_range_min() == 0x4321);
    expect(vert_range_min() == 0);

    // set_vert_pos and set_horz_pos each dispatch to their own scroll.
    reset_scroll_redraw_probe();
    published = nullptr;
    window->set_vert_pos(5);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == vert_scroll);
    reset_scroll_redraw_probe();
    window->set_horz_pos(9);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == horz_scroll);

    // Null scrollbars: every forwarder is a no-op, no dispatch, no crash.
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    point_scrolls(nullptr, nullptr);
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    reset_scroll_redraw_probe();
    window->set_vert_pos(1);
    window->set_horz_pos(1);
    window->set_vert_range(1, 2);
    window->set_horz_range(1, 2);
    expect(scroll_redraw_calls == 0);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    // Redirects reach the same scrolls.
    point_scrolls(vert_scroll, horz_scroll);
    reset_scroll_redraw_probe();
    win_set_vert_range_redirect(window, nullptr, 3, 4);
    expect(scroll_redraw_self == vert_scroll);
    reset_scroll_redraw_probe();
    win_set_horz_pos_redirect(window, nullptr, 2);
    expect(scroll_redraw_self == horz_scroll);

    ScrollCurrentWin = saved_current_win;
}

namespace {
void *g_pref_win_target = nullptr;
int g_pref_win_page = -1;
int g_pref_win_calls = 0;
void __thiscall observe_pref_win_display(void *pref_win, int page) {
    g_pref_win_target = pref_win; g_pref_win_page = page; ++g_pref_win_calls;
}
}  // namespace

void test_console_preference_openers() {
    // Five openers, each opening the one PrefWin to a different page. The page
    // number is the only thing that distinguishes them, so every one is
    // checked - a copy-paste that left two openers on the same page would pass
    // a test that only confirmed a call happened.
    auto *const saved_display = ConsolePrefWinDisplay;
    void *const saved_pref = ConsolePrefWin;
    int fake_pref_win = 0;
    ConsolePrefWinDisplay = &observe_pref_win_display;
    ConsolePrefWin = &fake_pref_win;

    std::vector<uint8_t> storage(sizeof(Console) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *console = reinterpret_cast<Console *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    struct Opener {
        void (Console::*method)();
        void (__fastcall *redirect)(Console *, void *);
        int page;
    };
    const Opener openers[] = {
        {&Console::set_preferences, &console_set_preferences_redirect, 0},
        {&Console::set_auto_preferences, &console_set_auto_preferences_redirect, 3},
        {&Console::set_base_preferences, &console_set_base_preferences_redirect, 2},
        {&Console::set_audiovisual, &console_set_audiovisual_redirect, 4},
        {&Console::set_map_display, &console_set_map_display_redirect, 5},
        {&Console::set_adv_preferences, &console_set_adv_preferences_redirect, 1},
    };

    for (const Opener &opener : openers) {
        g_pref_win_calls = 0;
        g_pref_win_page = -1;
        g_pref_win_target = nullptr;
        (console->*(opener.method))();
        expect(g_pref_win_calls == 1);
        // The shared PrefWin, not the Console it is called through.
        expect(g_pref_win_target == &fake_pref_win);
        expect(g_pref_win_target != reinterpret_cast<void *>(console));
        expect(g_pref_win_page == opener.page);

        g_pref_win_calls = 0;
        opener.redirect(console, nullptr);
        expect(g_pref_win_calls == 1);
        expect(g_pref_win_page == opener.page);
    }
    // None of them touched the Console object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    ConsolePrefWinDisplay = saved_display;
    ConsolePrefWin = saved_pref;
}

namespace {
BaseWin *g_iface_base = nullptr;
int g_iface_a1 = 0, g_iface_a2 = 0, g_iface_button = -1, g_iface_double = -1;
int g_iface_calls = 0;
void __thiscall observe_iface_click(BaseWin *self, int a1, int a2, int button,
                                    int is_double) {
    g_iface_base = self; g_iface_a1 = a1; g_iface_a2 = a2;
    g_iface_button = button; g_iface_double = is_double;
    ++g_iface_calls;
}
}  // namespace

void test_base_win_iface_clicks() {
    // Four interface-click handlers, reached through the subobject BaseWin
    // embeds at 0xA14, so their `this` points there and must be adjusted back
    // by 0xA14 before dispatching. Each carries a distinct (button, double)
    // pair, and getting the adjustment or a flag wrong is the whole risk here.
    auto *const saved = BaseWinIfaceClick;
    BaseWinIfaceClick = &observe_iface_click;

    // A BaseWin, plus room past 0xA14 so the interface pointer is in-bounds.
    std::vector<uint8_t> storage(sizeof(BaseWin) + 0xA20);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const real_base = storage.data() + 16;
    // What the interface vtable would pass as `this`: BaseWin + 0xA14.
    auto *iface = reinterpret_cast<BaseWin *>(real_base + 0xA14);

    struct Handler {
        void (BaseWin::*method)(int, int);
        void (__fastcall *redirect)(BaseWin *, void *, int, int);
        int button;
        int is_double;
    };
    const Handler handlers[] = {
        {&BaseWin::on_iface_left_click,
         &base_win_on_iface_left_click_redirect, 0, 0},
        {&BaseWin::on_iface_right_click,
         &base_win_on_iface_right_click_redirect, 1, 0},
        {&BaseWin::on_iface_left_double_click,
         &base_win_on_iface_left_double_click_redirect, 0, 1},
        {&BaseWin::on_iface_right_double_click,
         &base_win_on_iface_right_double_click_redirect, 1, 1},
    };

    for (const Handler &handler : handlers) {
        seed_storage(storage.data(), expected.data(), storage.size());
        std::memcpy(expected.data(), storage.data(), storage.size());
        g_iface_calls = 0;
        (iface->*(handler.method))(0x1111, 0x2222);
        expect(g_iface_calls == 1);
        // Adjusted back to the BaseWin, not left at the interface subobject.
        expect(reinterpret_cast<uint8_t *>(g_iface_base) == real_base);
        expect(g_iface_a1 == 0x1111);
        expect(g_iface_a2 == 0x2222);
        expect(g_iface_button == handler.button);
        expect(g_iface_double == handler.is_double);
        // Nothing on the object moves; the handler only dispatches.
        expect_storage_bytes(storage.data(), expected.data(), storage.size());

        g_iface_calls = 0;
        handler.redirect(iface, nullptr, -1, -2);
        expect(g_iface_calls == 1);
        expect(reinterpret_cast<uint8_t *>(g_iface_base) == real_base);
        expect(g_iface_a1 == -1 && g_iface_a2 == -2);
        expect(g_iface_button == handler.button);
        expect(g_iface_double == handler.is_double);
    }

    BaseWinIfaceClick = saved;
}

namespace {
BaseWin *g_draw_supported_base = nullptr;
int g_draw_supported_arg = -1;
int g_draw_supported_calls = 0;
void __thiscall observe_draw_supported(BaseWin *self, int a1) {
    g_draw_supported_base = self; g_draw_supported_arg = a1;
    ++g_draw_supported_calls;
}
}  // namespace

void test_base_win_iface_scrolled() {
    // Only scroll kind 2 does anything: it stores the position at 0x40100
    // relative to the interface subobject, then redraws on the BaseWin reached
    // by the same 0xA14 adjustment. Any other kind is a no-op.
    auto *const saved = BaseWinDrawSupported;
    BaseWinDrawSupported = &observe_draw_supported;

    // Room for the interface subobject at 0xA14 and the store at +0x40100.
    std::vector<uint8_t> storage(sizeof(BaseWin) + 0xA14 + 0x40108);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const real_base = storage.data() + 16;
    auto *iface = reinterpret_cast<BaseWin *>(real_base + 0xA14);
    auto stored = [&] {
        int v = 0;
        std::memcpy(&v, real_base + 0xA14 + 0x40100, sizeof(v));
        return v;
    };

    // Kind 2: stores the position and redraws on the adjusted base.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_draw_supported_calls = 0;
    iface->on_iface_scrolled(2, 0x1234ABCD);
    expect(g_draw_supported_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_draw_supported_base) == real_base);
    expect(g_draw_supported_arg == 1);
    expect(stored() == 0x1234ABCD);
    std::memcpy(expected.data() + 16 + 0xA14 + 0x40100,
                storage.data() + 16 + 0xA14 + 0x40100, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Any other kind: nothing happens, nothing is stored, no redraw.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    for (int kind : {0, 1, 3, -2, INT_MAX}) {
        g_draw_supported_calls = 0;
        iface->on_iface_scrolled(kind, 0x55555555);
        expect(g_draw_supported_calls == 0);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    g_draw_supported_calls = 0;
    base_win_on_iface_scrolled_redirect(iface, nullptr, 2, -1);
    expect(g_draw_supported_calls == 1);
    expect(stored() == -1);

    BaseWinDrawSupported = saved;
}

namespace {
void *g_dfwd_self = nullptr;
int g_dfwd_a1 = 0, g_dfwd_a2 = 0, g_dfwd_calls = 0;
const char *g_dfwd_which = nullptr;
void __thiscall observe_fwd2(void *self, int a1, int a2) {
    g_dfwd_self = self; g_dfwd_a1 = a1; g_dfwd_a2 = a2; ++g_dfwd_calls;
}
void __thiscall observe_fwd1(void *self, int a1) {
    g_dfwd_self = self; g_dfwd_a1 = a1; ++g_dfwd_calls;
}
}  // namespace

void test_dialogs_forwarders() {
    // Eight event forwarders. Each reads a discriminator 8 bytes before the
    // interface `this`, and only when it matches (8 for the sprite-box family,
    // 2 for the list-box one) adjusts `this` back - 0x8C or 0x140 - and calls
    // the embedded widget's handler. Every seam, both discriminator values,
    // and both adjustments are checked; a wrong adjustment sends the call to
    // the wrong subobject, and a wrong discriminator makes a live event a
    // no-op or vice versa.
    auto *const s_rd = DialogsSpriteBoxOnRightDown;
    auto *const s_rdc = DialogsSpriteBoxOnRightDoubleClick;
    auto *const s_lu = DialogsSpriteBoxOnLeftUp;
    auto *const s_ru = DialogsSpriteBoxOnRightUp;
    auto *const s_rc = DialogsSpriteBoxOnRightClick;
    auto *const l_sc = DialogsListBoxOnScrolling;
    auto *const l_mw = DialogsListBoxOnMousewheel;
    DialogsSpriteBoxOnRightDown = &observe_fwd2;
    DialogsSpriteBoxOnRightDoubleClick = &observe_fwd2;
    DialogsSpriteBoxOnLeftUp = &observe_fwd2;
    DialogsSpriteBoxOnRightUp = &observe_fwd2;
    DialogsSpriteBoxOnRightClick = &observe_fwd2;
    DialogsListBoxOnScrolling = &observe_fwd2;
    DialogsListBoxOnMousewheel = &observe_fwd1;

    std::vector<uint8_t> storage(sizeof(Dialogs) + 0x400);
    // Put the interface `this` 0x200 into the buffer so both the -8
    // discriminator and the negative adjustments stay in bounds.
    uint8_t *const iface = storage.data() + 16 + 0x200;
    auto *dialogs = reinterpret_cast<Dialogs *>(iface);
    auto set_disc = [&](int value) {
        std::memcpy(iface - 8, &value, sizeof(value));
    };

    struct Case2 {
        void (Dialogs::*method)(int, int);
        int disc;
        size_t adjust;
    };
    const Case2 sprite[] = {
        {&Dialogs::on_right_down, 8, 0x8C},
        {&Dialogs::on_right_double_click, 8, 0x8C},
        {&Dialogs::on_left_up, 8, 0x8C},
        {&Dialogs::on_right_up, 8, 0x8C},
        {&Dialogs::on_right_click, 8, 0x8C},
        {&Dialogs::on_scrolled, 2, 0x140},
        {&Dialogs::on_scrolling, 2, 0x140},
    };
    for (const Case2 &c : sprite) {
        // Matching discriminator: forwards to (this - adjust) with both args.
        set_disc(c.disc);
        g_dfwd_calls = 0;
        (dialogs->*(c.method))(0x111, 0x222);
        expect(g_dfwd_calls == 1);
        expect(g_dfwd_self == iface - c.adjust);
        expect(g_dfwd_a1 == 0x111 && g_dfwd_a2 == 0x222);
        // Wrong discriminator: no forward.
        set_disc(c.disc + 1);
        g_dfwd_calls = 0;
        (dialogs->*(c.method))(0x111, 0x222);
        expect(g_dfwd_calls == 0);
    }

    // on_mousewheel takes one argument and uses the list-box discriminator.
    set_disc(2);
    g_dfwd_calls = 0;
    dialogs->on_mousewheel(0x333);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x140);
    expect(g_dfwd_a1 == 0x333);
    set_disc(0);
    g_dfwd_calls = 0;
    dialogs->on_mousewheel(0x333);
    expect(g_dfwd_calls == 0);

    // A redirect from each family, confirming the seam wiring.
    set_disc(8);
    g_dfwd_calls = 0;
    dialogs_on_right_down_redirect(dialogs, nullptr, 1, 2);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x8C);
    set_disc(2);
    g_dfwd_calls = 0;
    dialogs_on_mousewheel_redirect(dialogs, nullptr, 7);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x140);

    DialogsSpriteBoxOnRightDown = s_rd;
    DialogsSpriteBoxOnRightDoubleClick = s_rdc;
    DialogsSpriteBoxOnLeftUp = s_lu;
    DialogsSpriteBoxOnRightUp = s_ru;
    DialogsSpriteBoxOnRightClick = s_rc;
    DialogsListBoxOnScrolling = l_sc;
    DialogsListBoxOnMousewheel = l_mw;
}

namespace {
BaseWin *g_bwclick_self = nullptr;
int g_bwclick_a1 = 0, g_bwclick_a2 = 0, g_bwclick_button = -1, g_bwclick_double = -1;
int g_bwclick_calls = 0;
void __thiscall observe_bw_click(BaseWin *self, int a1, int a2,
                                       int button, int is_double) {
    g_bwclick_self = self; g_bwclick_a1 = a1; g_bwclick_a2 = a2;
    g_bwclick_button = button; g_bwclick_double = is_double;
    ++g_bwclick_calls;
}
}  // namespace

void test_base_win_clicks() {
    // Three primary click handlers, forwarding to the shared click() with no
    // this-adjustment - `this` is the BaseWin itself. Each carries a distinct
    // (button, double) pair, and unlike the iface family the callee gets the
    // object unchanged, which is what the test confirms alongside the flags.
    auto *const saved = BaseWinClick;
    BaseWinClick = &observe_bw_click;

    std::vector<uint8_t> storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *window = reinterpret_cast<BaseWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    struct Handler {
        void (BaseWin::*method)(int, int);
        void (__fastcall *redirect)(BaseWin *, void *, int, int);
        int button;
        int is_double;
    };
    const Handler handlers[] = {
        {&BaseWin::on_left_click, &base_win_on_left_click_redirect, 0, 0},
        {&BaseWin::on_right_click, &base_win_on_right_click_redirect, 1, 0},
        {&BaseWin::on_left_double_click,
         &base_win_on_left_double_click_redirect, 0, 1},
    };

    for (const Handler &handler : handlers) {
        g_bwclick_calls = 0;
        (window->*(handler.method))(0x1111, 0x2222);
        expect(g_bwclick_calls == 1);
        expect(g_bwclick_self == window);        // the object itself, no adjust
        expect(g_bwclick_a1 == 0x1111 && g_bwclick_a2 == 0x2222);
        expect(g_bwclick_button == handler.button);
        expect(g_bwclick_double == handler.is_double);

        g_bwclick_calls = 0;
        handler.redirect(window, nullptr, -1, -2);
        expect(g_bwclick_calls == 1);
        expect(g_bwclick_self == window);
        expect(g_bwclick_a1 == -1 && g_bwclick_a2 == -2);
        expect(g_bwclick_button == handler.button);
        expect(g_bwclick_double == handler.is_double);
    }
    // The forwarders only dispatch; the object is untouched.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    BaseWinClick = saved;
}

namespace {
WorldWin *g_ww_self = nullptr;
int g_ww_button = -1, g_ww_double = -1, g_ww_calls = 0;
void __thiscall observe_world_win_click(WorldWin *self, int a1, int a2,
                                        int button, int is_double) {
    g_ww_self = self; g_ww_button = button; g_ww_double = is_double;
    ++g_ww_calls; (void)a1; (void)a2;
}
MapWin *g_mw_self = nullptr;
int g_mw_a1 = 0, g_mw_a2 = 0, g_mw_button = -1, g_mw_calls = 0;
void __thiscall observe_map_win_click(MapWin *self, int a1, int a2, int button) {
    g_mw_self = self; g_mw_a1 = a1; g_mw_a2 = a2; g_mw_button = button;
    ++g_mw_calls;
}
}  // namespace

void test_window_click_forwarders() {
    // WorldWin: a plain 4-arg click, no adjustment, no guard.
    auto *const saved_ww = WorldWinClick;
    WorldWinClick = &observe_world_win_click;
    std::vector<uint8_t> ww(sizeof(WorldWin) + 32);
    std::vector<uint8_t> ww_want(ww.size());
    auto *world = reinterpret_cast<WorldWin *>(ww.data() + 16);
    seed_storage(ww.data(), ww_want.data(), ww.size());
    std::memcpy(ww_want.data(), ww.data(), ww.size());
    g_ww_calls = 0;
    world->on_left_click(1, 2);
    expect(g_ww_calls == 1 && g_ww_self == world && g_ww_button == 0 && g_ww_double == 0);
    world->on_right_click(1, 2);
    expect(g_ww_button == 1 && g_ww_double == 0 && g_ww_self == world);
    world_win_on_left_click_redirect(world, nullptr, 3, 4);
    expect(g_ww_button == 0);
    expect_storage_bytes(ww.data(), ww_want.data(), ww.size());
    WorldWinClick = saved_ww;

    // MapWin: reached through the GraphicWin virtual base, so `this` points at
    // 0x21A6C into the object and is adjusted back; gated on an input-enable
    // flag. Both the adjustment and the guard are pinned.
    auto *const saved_mw = MapWinClick;
    int32_t *const saved_flag = MapWinInputEnabled;
    int32_t enabled = 1;
    MapWinClick = &observe_map_win_click;
    MapWinInputEnabled = &enabled;

    std::vector<uint8_t> mw(sizeof(MapWin) + 32);
    auto *real_map = reinterpret_cast<uint8_t *>(mw.data() + 16);
    // What the virtual-base vtable passes as `this`: MapWin + 0x21A6C.
    auto *vbase_this = reinterpret_cast<MapWin *>(real_map + 0x21A6C);

    enabled = 1;
    g_mw_calls = 0;
    vbase_this->on_left_click(0x55, 0x66);
    expect(g_mw_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);  // adjusted back
    expect(g_mw_a1 == 0x55 && g_mw_a2 == 0x66 && g_mw_button == 0);
    vbase_this->on_right_click(-1, -2);
    expect(g_mw_button == 1 && g_mw_a1 == -1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);

    // Disabled: the guard blocks both, no dispatch.
    enabled = 0;
    g_mw_calls = 0;
    vbase_this->on_left_click(1, 1);
    vbase_this->on_right_click(1, 1);
    expect(g_mw_calls == 0);

    enabled = 1;
    g_mw_calls = 0;
    map_win_on_right_click_redirect(vbase_this, nullptr, 9, 8);
    expect(g_mw_calls == 1 && g_mw_button == 1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);

    MapWinClick = saved_mw;
    MapWinInputEnabled = saved_flag;
}

namespace {
BasePop *g_exec_self = nullptr;
int g_exec_flag = -1;
int (__cdecl *g_exec_cb)() = nullptr;
int g_exec_result = 0, g_exec_calls = 0;
int __thiscall observe_base_pop_exec(BasePop *self, int flag, int (__cdecl *cb)()) {
    g_exec_self = self; g_exec_flag = flag; g_exec_cb = cb; ++g_exec_calls;
    return g_exec_result;
}
int __cdecl exec_probe_cb() { return 0; }

struct StartArgs {
    Popup *self; char *a; const char *b; int c; char *d; int e; void *f;
    int calls;
} g_start2 = {};
void __thiscall observe_start_full(Popup *self, char *a, const char *b, int c,
                                   char *d, int e, void *f) {
    g_start2 = {self, a, b, c, d, e, f, g_start2.calls + 1};
}
}  // namespace

void test_popup_exec_and_start_overloads() {
    // BasePop::exec: two overloads forwarding to the two-argument exec with
    // flag 0 and either no callback or the given one. The callback must pass
    // through, and the result must come back.
    auto *const saved_exec = BasePopExec;
    BasePopExec = &observe_base_pop_exec;
    std::vector<uint8_t> bp(sizeof(BasePop) + 32);
    auto *popup = reinterpret_cast<BasePop *>(bp.data() + 16);

    g_exec_result = 0x1234;
    g_exec_calls = 0;
    expect(popup->exec() == 0x1234);
    expect(g_exec_calls == 1 && g_exec_self == popup);
    expect(g_exec_flag == 0 && g_exec_cb == nullptr);

    g_exec_result = 0x5678;
    expect(popup->exec(&exec_probe_cb) == 0x5678);
    expect(g_exec_flag == 0 && g_exec_cb == &exec_probe_cb);
    expect(base_pop_exec_void_redirect(popup, nullptr) == 0x5678);
    expect(g_exec_cb == nullptr);
    expect(base_pop_exec_callback_redirect(popup, nullptr, &exec_probe_cb) == 0x5678);
    expect(g_exec_cb == &exec_probe_cb);
    BasePopExec = saved_exec;

    // Popup::start: two short forms filling the full start's defaults. The
    // caption comes from the shared buffer, c is -1, and the value lands in
    // slot e - the label-and-value form differs from the label-only form only
    // in that one argument, which the test pins.
    auto *const saved_start = PopupOriginalStartFull;
    char *const saved_caption = PopupStartCaption;
    char caption[4] = {};
    PopupOriginalStartFull = &observe_start_full;
    PopupStartCaption = caption;
    std::vector<uint8_t> pu(sizeof(Popup) + 32);
    auto *pstart = reinterpret_cast<Popup *>(pu.data() + 16);
    const char label[] = "hi";

    g_start2.calls = 0;
    pstart->start(label);
    expect(g_start2.calls == 1 && g_start2.self == pstart);
    expect(g_start2.a == caption && g_start2.b == label);
    expect(g_start2.c == -1 && g_start2.d == nullptr && g_start2.e == 0 &&
           g_start2.f == nullptr);

    pstart->start(label, 0x77);
    expect(g_start2.b == label && g_start2.e == 0x77);   // value in slot e
    expect(g_start2.c == -1 && g_start2.d == nullptr && g_start2.f == nullptr);

    popup_start_label_redirect(pstart, nullptr, label);
    expect(g_start2.e == 0);
    popup_start_label_value_redirect(pstart, nullptr, label, -9);
    expect(g_start2.e == -9);

    PopupOriginalStartFull = saved_start;
    PopupStartCaption = saved_caption;
}

void test_replay_win_stubs() {
    // Six constant-return stubs generated in bulk by tools/bulk_recover_stubs.
    // Each is a bare return that touches nothing, so the whole specification is
    // that a seeded object comes back unchanged - checked once for all six,
    // through both the method and its redirect. ReplayWin is large, so the
    // canary lives on the heap.
    std::vector<uint8_t> storage(sizeof(ReplayWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *replay = reinterpret_cast<ReplayWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    replay->on_left_double_click(1, 2);
    replay->on_right_double_click(3, 4);
    replay->on_mouse_move(5, 6);
    replay->on_right_down(7, 8);
    replay->on_left_down(9, 10);
    replay->on_left_up(11, 12);
    replay_win_on_left_double_click_redirect(replay, nullptr, -1, -2);
    replay_win_on_right_double_click_redirect(replay, nullptr, -3, -4);
    replay_win_on_mouse_move_redirect(replay, nullptr, -5, -6);
    replay_win_on_right_down_redirect(replay, nullptr, -7, -8);
    replay_win_on_left_down_redirect(replay, nullptr, -9, -10);
    replay_win_on_left_up_redirect(replay, nullptr, -11, -12);

    expect_storage_bytes(storage.data(), expected.data(), storage.size());
}

void test_bulk_generated_stubs() {
    // Eighteen constant-return stubs across seven classes, all generated by
    // tools/bulk_recover_stubs in one pass. Each touches nothing, so a seeded
    // object returning unchanged - through method and redirect - is the whole
    // check, plus the fixed return value where there is one.

    std::vector<uint8_t> report_if_b_storage(sizeof(ReportIf) + 32);
    std::vector<uint8_t> report_if_b_expected(report_if_b_storage.size());
    auto *report_if_b = reinterpret_cast<ReportIf *>(report_if_b_storage.data() + 16);
    seed_storage(report_if_b_storage.data(), report_if_b_expected.data(), report_if_b_storage.size());
    std::memcpy(report_if_b_expected.data(), report_if_b_storage.data(), report_if_b_storage.size());
    report_if_b->on_iface_right_click(1, 2);
    report_if_on_iface_right_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_left_double_click(1, 2);
    report_if_on_iface_left_double_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_right_double_click(1, 2);
    report_if_on_iface_right_double_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_left_down(1, 2);
    report_if_on_iface_left_down_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_right_down(1, 2);
    report_if_on_iface_right_down_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_mouse_move(1, 2);
    report_if_on_iface_mouse_move_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_mouse_leave(1, 2);
    report_if_on_iface_mouse_leave_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_button_toggled(1, 2);
    report_if_on_iface_button_toggled_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->close_score();
    report_if_close_score_redirect(report_if_b, nullptr);
    expect_storage_bytes(report_if_b_storage.data(), report_if_b_expected.data(),
                         report_if_b_storage.size());
    std::vector<uint8_t> dip_edit_b_storage(sizeof(DipEdit) + 32);
    std::vector<uint8_t> dip_edit_b_expected(dip_edit_b_storage.size());
    auto *dip_edit_b = reinterpret_cast<DipEdit *>(dip_edit_b_storage.data() + 16);
    seed_storage(dip_edit_b_storage.data(), dip_edit_b_expected.data(), dip_edit_b_storage.size());
    std::memcpy(dip_edit_b_expected.data(), dip_edit_b_storage.data(), dip_edit_b_storage.size());
    dip_edit_b->on_left_click(1, 2);
    dip_edit_on_left_click_redirect(dip_edit_b, nullptr, -1, -2);
    dip_edit_b->on_redraw();
    dip_edit_on_redraw_redirect(dip_edit_b, nullptr);
    expect_storage_bytes(dip_edit_b_storage.data(), dip_edit_b_expected.data(),
                         dip_edit_b_storage.size());
    std::vector<uint8_t> sound_b_storage(sizeof(Sound) + 32);
    std::vector<uint8_t> sound_b_expected(sound_b_storage.size());
    auto *sound_b = reinterpret_cast<Sound *>(sound_b_storage.data() + 16);
    seed_storage(sound_b_storage.data(), sound_b_expected.data(), sound_b_storage.size());
    std::memcpy(sound_b_expected.data(), sound_b_storage.data(), sound_b_storage.size());
    expect(sound_b->UNK1(1) == 11);
    expect(sound_unk1_redirect(sound_b, nullptr, -1) == 11);
    expect_storage_bytes(sound_b_storage.data(), sound_b_expected.data(),
                         sound_b_storage.size());
    std::vector<uint8_t> wave_device_b_storage(sizeof(Wave_Device) + 32);
    std::vector<uint8_t> wave_device_b_expected(wave_device_b_storage.size());
    auto *wave_device_b = reinterpret_cast<Wave_Device *>(wave_device_b_storage.data() + 16);
    seed_storage(wave_device_b_storage.data(), wave_device_b_expected.data(), wave_device_b_storage.size());
    std::memcpy(wave_device_b_expected.data(), wave_device_b_storage.data(), wave_device_b_storage.size());
    wave_device_b->set_pan(1);
    wave_device_set_pan_redirect(wave_device_b, nullptr, -1);
    expect(wave_device_b->fade(1) == 0);
    expect(wave_device_fade_redirect(wave_device_b, nullptr, -1) == 0);
    expect_storage_bytes(wave_device_b_storage.data(), wave_device_b_expected.data(),
                         wave_device_b_storage.size());
    std::vector<uint8_t> sprite_box_b_storage(sizeof(SpriteBox) + 32);
    std::vector<uint8_t> sprite_box_b_expected(sprite_box_b_storage.size());
    auto *sprite_box_b = reinterpret_cast<SpriteBox *>(sprite_box_b_storage.data() + 16);
    seed_storage(sprite_box_b_storage.data(), sprite_box_b_expected.data(), sprite_box_b_storage.size());
    std::memcpy(sprite_box_b_expected.data(), sprite_box_b_storage.data(), sprite_box_b_storage.size());
    sprite_box_b->on_mouse_move(1, 2);
    sprite_box_on_mouse_move_redirect(sprite_box_b, nullptr, -1, -2);
    sprite_box_b->on_mouse_leave(1, 2);
    sprite_box_on_mouse_leave_redirect(sprite_box_b, nullptr, -1, -2);
    expect_storage_bytes(sprite_box_b_storage.data(), sprite_box_b_expected.data(),
                         sprite_box_b_storage.size());
    std::vector<uint8_t> list_box_b_storage(sizeof(ListBox) + 32);
    std::vector<uint8_t> list_box_b_expected(list_box_b_storage.size());
    auto *list_box_b = reinterpret_cast<ListBox *>(list_box_b_storage.data() + 16);
    seed_storage(list_box_b_storage.data(), list_box_b_expected.data(), list_box_b_storage.size());
    std::memcpy(list_box_b_expected.data(), list_box_b_storage.data(), list_box_b_storage.size());
    list_box_b->on_dialog_focus(1);
    list_box_on_dialog_focus_redirect(list_box_b, nullptr, -1);
    expect_storage_bytes(list_box_b_storage.data(), list_box_b_expected.data(),
                         list_box_b_storage.size());
    std::vector<uint8_t> net_b_storage(sizeof(Net) + 32);
    std::vector<uint8_t> net_b_expected(net_b_storage.size());
    auto *net_b = reinterpret_cast<Net *>(net_b_storage.data() + 16);
    seed_storage(net_b_storage.data(), net_b_expected.data(), net_b_storage.size());
    std::memcpy(net_b_expected.data(), net_b_storage.data(), net_b_storage.size());
    expect(net_b->poll_players(1) == 1);
    expect(net_poll_players_redirect(net_b, nullptr, -1) == 1);
    expect_storage_bytes(net_b_storage.data(), net_b_expected.data(),
                         net_b_storage.size());
}

void test_remaining_constant_stubs() {
    // The last of the constant-return bucket - the ones the bulk tool skipped
    // for pointer or unsigned parameters, done by hand with faithful types.
    // Each touches nothing and returns its constant.
    std::vector<uint8_t> mw(sizeof(MapWin) + 32), mw_e(mw.size());
    auto *map = reinterpret_cast<MapWin *>(mw.data() + 16);
    seed_storage(mw.data(), mw_e.data(), mw.size());
    std::memcpy(mw_e.data(), mw.data(), mw.size());
    map->on_left_double_click(1, 2);
    map->on_left_up(3, 4);
    map_win_on_left_double_click_redirect(map, nullptr, -1, -2);
    map_win_on_left_up_redirect(map, nullptr, -3, -4);
    expect_storage_bytes(mw.data(), mw_e.data(), mw.size());

    std::vector<uint8_t> md(sizeof(Midi_Device) + 32), md_e(md.size());
    auto *midi = reinterpret_cast<Midi_Device *>(md.data() + 16);
    seed_storage(md.data(), md_e.data(), md.size());
    std::memcpy(md_e.data(), md.data(), md.size());
    char buf[8] = {};
    expect(midi->get_description(1u, buf, 2u) == 0);
    expect(midi_device_get_description_redirect(midi, nullptr, 1u, buf, 2u) == 0);
    expect_storage_bytes(md.data(), md_e.data(), md.size());

    std::vector<uint8_t> wd(sizeof(Wave_In_Device) + 32), wd_e(wd.size());
    auto *wave = reinterpret_cast<Wave_In_Device *>(wd.data() + 16);
    seed_storage(wd.data(), wd_e.data(), wd.size());
    std::memcpy(wd_e.data(), wd.data(), wd.size());
    expect(wave->get_description(1u, buf, 2u) == 0);
    expect(wave_in_device_get_description_redirect(wave, nullptr, 1u, buf, 2u) == 0);
    expect_storage_bytes(wd.data(), wd_e.data(), wd.size());

    std::vector<uint8_t> pu(sizeof(Popup) + 32), pu_e(pu.size());
    auto *popup = reinterpret_cast<Popup *>(pu.data() + 16);
    seed_storage(pu.data(), pu_e.data(), pu.size());
    std::memcpy(pu_e.data(), pu.data(), pu.size());
    expect(popup->on_dialog_back_draw(nullptr) == 0);
    expect(popup_on_dialog_back_draw_redirect(popup, nullptr, nullptr) == 0);
    expect_storage_bytes(pu.data(), pu_e.data(), pu.size());

    // Win::OnSetCursor and BaseWin::timer_callback are static; no instance.
    expect(Win::OnSetCursor(nullptr, nullptr, 0u, 0u) == 1);
    expect(win_onsetcursor_redirect(nullptr, nullptr, 1u, 2u) == 1);
    BaseWin::timer_callback(1, 2);
    base_win_timer_callback_redirect(-1, -2);
}

void test_field_store_batch2() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0; std::memcpy(&v, s.data() + 16 + off, sizeof(v)); return v;
    };

    // SquareLock::clear: two -1 sentinels and a zero at 0, 4, 8.
    std::vector<uint8_t> sl(sizeof(SquareLock) + 32), sl_e(sl.size());
    auto *lock = reinterpret_cast<SquareLock *>(sl.data() + 16);
    seed_storage(sl.data(), sl_e.data(), sl.size());
    std::memcpy(sl_e.data(), sl.data(), sl.size());
    lock->clear();
    expect(read32(sl, 0) == -1);
    expect(read32(sl, 4) == -1);
    expect(read32(sl, 8) == 0);
    std::memcpy(sl_e.data() + 16, sl.data() + 16, 12);
    expect_storage_bytes(sl.data(), sl_e.data(), sl.size());
    square_lock_clear_redirect(lock, nullptr);

    // DeletionList::clear(index): the marker byte and word land at index*0x3C,
    // which the test checks at two different indices - a wrong stride would put
    // the second one somewhere the check does not look.
    std::vector<uint8_t> dl(0x3C * 4 + 32), dl_e(dl.size());
    auto *list = reinterpret_cast<DeletionList *>(dl.data() + 16);
    for (int index : {0, 2}) {
        seed_storage(dl.data(), dl_e.data(), dl.size());
        std::memcpy(dl_e.data(), dl.data(), dl.size());
        list->clear(index);
        const size_t base = 16 + static_cast<size_t>(index) * 0x3C;
        expect(dl[base] == 0xFF);
        uint16_t word = 0xFFFF;
        std::memcpy(&word, dl.data() + base + 8, sizeof(word));
        expect(word == 0);
        dl_e[base] = dl[base];
        std::memcpy(dl_e.data() + base + 8, dl.data() + base + 8, 2);
        expect_storage_bytes(dl.data(), dl_e.data(), dl.size());
    }
    deletion_list_clear_redirect(list, nullptr, 1);

    // DiploWin::UNK2: zeroes 0xA24 and 0xA28.
    std::vector<uint8_t> dw(sizeof(DiploWin) + 32), dw_e(dw.size());
    auto *diplo = reinterpret_cast<DiploWin *>(dw.data() + 16);
    seed_storage(dw.data(), dw_e.data(), dw.size());
    std::memcpy(dw_e.data(), dw.data(), dw.size());
    diplo->UNK2();
    expect(read32(dw, 0xA24) == 0);
    expect(read32(dw, 0xA28) == 0);
    std::memcpy(dw_e.data() + 16 + 0xA24, dw.data() + 16 + 0xA24, 8);
    expect_storage_bytes(dw.data(), dw_e.data(), dw.size());
    diplo_win_unk2_redirect(diplo, nullptr);
}

void test_pull_down_id_to_index() {
    // A linear search over the 64 item slots comparing each item's id, at
    // 0xA18 + index*0x14 + 8. The search's three exits are all checked: a
    // match returns the index, a -1 sentinel id stops the scan and returns -1,
    // and an id absent before the sentinel also returns -1.
    std::vector<uint8_t> storage(sizeof(PullDown) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *pull = reinterpret_cast<PullDown *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    auto set_id = [&](int index, int id) {
        std::memcpy(storage.data() + 16 + 0xA18 + index * 0x14 + 8, &id,
                    sizeof(id));
    };
    // Ids 100, 200, 300 in the first three slots, then the -1 sentinel.
    set_id(0, 100);
    set_id(1, 200);
    set_id(2, 300);
    set_id(3, -1);
    std::memcpy(expected.data(), storage.data(), storage.size());

    expect(pull->id_to_index(100) == 0);
    expect(pull->id_to_index(200) == 1);
    expect(pull->id_to_index(300) == 2);
    // Absent id, scan stops at the sentinel in slot 3.
    expect(pull->id_to_index(999) == -1);
    // The id -1 is the sentinel, so it is never matched as a value.
    expect(pull->id_to_index(-1) == -1);
    expect(pull_down_id_to_index_redirect(pull, nullptr, 200) == 1);
    // A pure search writes nothing.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // First match wins when an id repeats.
    set_id(5, 200);
    expect(pull->id_to_index(200) == 1);
}

void test_loop_store_searches() {
    // Palette::get_pos: find-or-first-free over five slots. A matching value
    // returns its index, an empty (-1) slot returns its index, and a full
    // table with no match returns 5. The find and the free-slot exits both
    // return the current index, so both are checked, as is the full case.
    std::vector<uint8_t> pl(sizeof(Palette) + 32), pl_e(pl.size());
    auto *palette = reinterpret_cast<Palette *>(pl.data() + 16);
    seed_storage(pl.data(), pl_e.data(), pl.size());
    auto set_slot = [&](int i, int32_t v) {
        std::memcpy(pl.data() + 16 + 0x404 + i * 0x10, &v, sizeof(v));
    };
    set_slot(0, 0x100);
    set_slot(1, 0x200);
    set_slot(2, 0x300);
    set_slot(3, 0x400);
    set_slot(4, 0x500);
    std::memcpy(pl_e.data(), pl.data(), pl.size());
    expect(palette->get_pos(0x100) == 0);
    expect(palette->get_pos(0x300) == 2);
    expect(palette->get_pos(0x500) == 4);
    expect(palette->get_pos(0x999) == 5);
    expect(palette_get_pos_redirect(palette, nullptr, 0x400) == 3);
    expect_storage_bytes(pl.data(), pl_e.data(), pl.size());

    set_slot(2, -1);
    expect(palette->get_pos(0x999) == 2);
    expect(palette->get_pos(0x100) == 0);
    expect(palette->get_pos(0x400) == 2);

    // PlayerLock::active: 1 when either entry's flag has the low bit set.
    std::vector<uint8_t> lk(sizeof(PlayerLock) + 32), lk_e(lk.size());
    auto *lock = reinterpret_cast<PlayerLock *>(lk.data() + 16);
    auto set_flag = [&](int entry, int32_t v) {
        std::memcpy(lk.data() + 16 + 0xC + entry * 0xC, &v, sizeof(v));
    };
    seed_storage(lk.data(), lk_e.data(), lk.size());
    set_flag(0, 0);
    set_flag(1, 0);
    std::memcpy(lk_e.data(), lk.data(), lk.size());
    expect(lock->active() == 0);
    set_flag(1, 1);
    expect(lock->active() == 1);                 // second entry engaged
    set_flag(1, 0);
    set_flag(0, 3);                              // low bit set among others
    expect(lock->active() == 1);
    set_flag(0, 2);                              // low bit clear
    expect(lock->active() == 0);
    set_flag(0, 1);
    expect(player_lock_active_redirect(lock, nullptr) == 1);
    std::memcpy(lk_e.data(), lk.data(), lk.size());
    (void)lock->active();
    expect_storage_bytes(lk.data(), lk_e.data(), lk.size());
}

void test_win_unk3_contains() {
    // A linear "contains" over the id table at 0x1A4 with its count at 0x3FC.
    // All four exits are checked: a present value returns 1, an absent one 0,
    // a zero query 0 without scanning, and an empty table 0.
    std::vector<uint8_t> storage(sizeof(Win) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *window = reinterpret_cast<Win *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    auto set_count = [&](int32_t c) {
        std::memcpy(storage.data() + 16 + 0x3FC, &c, sizeof(c));
    };
    auto set_entry = [&](int i, int32_t v) {
        std::memcpy(storage.data() + 16 + 0x1A4 + i * 4, &v, sizeof(v));
    };
    set_count(3);
    set_entry(0, 0x111);
    set_entry(1, 0x222);
    set_entry(2, 0x333);
    std::memcpy(expected.data(), storage.data(), storage.size());

    expect(window->UNK3(0x111) == 1);
    expect(window->UNK3(0x333) == 1);
    expect(window->UNK3(0x444) == 0);          // absent
    expect(window->UNK3(0) == 0);              // zero query, no scan
    expect(win_unk3_redirect(window, nullptr, 0x222) == 1);
    // A pure search writes nothing.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Empty table: everything is absent, and a value that sits in the array
    // past the count must not be found.
    set_count(0);
    expect(window->UNK3(0x111) == 0);
    set_count(1);
    expect(window->UNK3(0x222) == 0);          // 0x222 is at index 1, past count 1
    expect(window->UNK3(0x111) == 1);          // 0x111 at index 0, within count
}

void test_console_clear_group() {
    // Clears the field at 0x23D1C and drops bit 27 from every entry of a group
    // table at 0x34 stride, over a count both read from fixed addresses. The
    // seams point at a local table so the masking is observable: each entry's
    // other bits survive, only 0x08000000 is cleared, and entries past the
    // count are untouched.
    int32_t count = 4;
    std::vector<uint8_t> table(0x34 * 6, 0);
    std::vector<uint8_t> table_before;
    int32_t *const saved_count = ConsoleGroupCount;
    uint8_t *const saved_table = ConsoleGroupTable;
    ConsoleGroupCount = &count;
    ConsoleGroupTable = table.data();

    auto entry = [&](int i) {
        uint32_t v = 0; std::memcpy(&v, table.data() + i * 0x34, 4); return v;
    };
    auto set_entry = [&](int i, uint32_t v) {
        std::memcpy(table.data() + i * 0x34, &v, 4);
    };
    // All bits set in the first four entries, plus one past the count.
    for (int i = 0; i < 6; ++i) set_entry(i, 0xFFFFFFFFu);
    table_before = table;

    std::vector<uint8_t> storage(sizeof(Console) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *console = reinterpret_cast<Console *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    int32_t marker = 0x1234;
    std::memcpy(storage.data() + 16 + 0x23D1C, &marker, sizeof(marker));
    std::memcpy(expected.data(), storage.data(), storage.size());

    console->clear_group();

    // Field cleared.
    int32_t field = -1;
    std::memcpy(&field, storage.data() + 16 + 0x23D1C, sizeof(field));
    expect(field == 0);
    std::memcpy(expected.data() + 16 + 0x23D1C, storage.data() + 16 + 0x23D1C, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Entries 0..3 had only bit 27 cleared; entries 4 and 5 (past count 4)
    // are untouched.
    for (int i = 0; i < 4; ++i) expect(entry(i) == 0xF7FFFFFFu);
    expect(entry(4) == 0xFFFFFFFFu);
    expect(entry(5) == 0xFFFFFFFFu);

    // Count <= 0 leaves the table alone.
    table = table_before;
    count = 0;
    console->clear_group();
    expect(table == table_before);
    count = 2;
    console_clear_group_redirect(console, nullptr);
    expect(entry(0) == 0xF7FFFFFFu);
    expect(entry(2) == 0xFFFFFFFFu);   // index 2 past count 2

    ConsoleGroupCount = saved_count;
    ConsoleGroupTable = saved_table;
}

void test_lock_reset_map() {
    // Clears bits 0x38 of the flag byte at offset 5 of each 0x2C-byte record,
    // over a count both from fixed addresses; the seams point at a local table.
    // The mask keeps bits outside 0x38, the offset-5 placement, the stride, and
    // the count bound are all checked.
    int32_t count = 4;
    std::vector<uint8_t> tbl(0x2C * 6, 0);
    int32_t *const saved_count = LockMapCount;
    uint8_t *const saved_table = LockMapTable;
    LockMapCount = &count;
    LockMapTable = tbl.data();

    // Every bit set in each record's flag byte, plus other bytes set too so a
    // wrong offset would clear the wrong byte.
    for (size_t i = 0; i < tbl.size(); ++i) tbl[i] = 0xFF;
    std::vector<uint8_t> before = tbl;

    std::vector<uint8_t> lk(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(lk.data() + 16);
    lock->reset_map();

    // Records 0..3: only the flag byte at +5 changed, to 0xFF & 0xC7 = 0xC7.
    for (int i = 0; i < 4; ++i) {
        expect(tbl[i * 0x2C + 5] == 0xC7);
        // Neighbouring bytes in the record untouched.
        expect(tbl[i * 0x2C + 4] == 0xFF);
        expect(tbl[i * 0x2C + 6] == 0xFF);
    }
    // Records 4, 5 past count 4 are whole.
    expect(tbl[4 * 0x2C + 5] == 0xFF);
    expect(tbl[5 * 0x2C + 5] == 0xFF);

    // Count <= 0 leaves the table alone.
    tbl = before;
    count = 0;
    lock->reset_map();
    expect(tbl == before);
    count = 2;
    lock_reset_map_redirect(lock, nullptr);
    expect(tbl[0 * 0x2C + 5] == 0xC7);
    expect(tbl[2 * 0x2C + 5] == 0xFF);   // index 2 past count 2

    LockMapCount = saved_count;
    LockMapTable = saved_table;
}

void test_lock_clear() {
    // Initialises eight 0x1C records to two -1 sentinels and a zero each, plus
    // three trailing dwords, then runs reset_map. The map count is set to zero
    // so the global tail is a no-op and this checks only the record init.
    int32_t map_count = 0;
    int32_t *const saved_count = LockMapCount;
    LockMapCount = &map_count;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    lock->clear();

    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, storage.data() + 16 + off, sizeof(v)); return v;
    };
    // Each record: flag byte 0 at record*0x1C, then entries {-1,-1,0} at +4 and
    // +0x10. Every one of the eight is checked, not a sample - a loop bound
    // that stopped early would leave a later record seeded.
    for (int r = 0; r < 8; ++r) {
        const size_t base = r * 0x1C;
        expect((storage[16 + base] & 0xFF) == 0);
        expect(read32(base + 0x04) == -1);
        expect(read32(base + 0x08) == -1);
        expect(read32(base + 0x0C) == 0);
        expect(read32(base + 0x10) == -1);
        expect(read32(base + 0x14) == -1);
        expect(read32(base + 0x18) == 0);
    }
    // The three trailing dwords at 0xE0.
    expect(read32(0xE0) == 0);
    expect(read32(0xE4) == 0);
    expect(read32(0xE8) == 0);

    lock_clear_redirect(lock, nullptr);
    LockMapCount = saved_count;
}

void test_lock_any_locks() {
    // any_locks: the 0xE0 dword forces a yes; otherwise records 1..7 are
    // scanned, but only those whose slot bit is set in both the low and second
    // bytes of the enable mask, and only entry flags with the low bit set
    // count. Record 0 is never scanned. Each of those conditions is exercised.
    uint32_t mask = 0;
    uint32_t *const saved_mask = LockEnableMask;
    LockEnableMask = &mask;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set_field_e0 = [&](uint32_t v) { std::memcpy(obj + 0xE0, &v, 4); };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Shortcut: field at 0xE0 non-zero returns 1 regardless of the rest.
    seed_storage(storage.data(), nullptr, 0);
    std::memset(obj, 0, sizeof(Lock));
    set_field_e0(0x99);
    expect(lock->any_locks() == 1);

    // 0xE0 clear, mask 0: nothing scanned.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0;
    // Even with an engaged entry, no bit set means no scan.
    set_flag(2, 0, 1);
    expect(lock->any_locks() == 0);

    // Bit 2 set in both mask bytes, record 2 entry engaged: yes.
    mask = 0x0404;
    expect(lock->any_locks() == 1);
    // The other entry of the same record also counts.
    set_flag(2, 0, 0);
    set_flag(2, 1, 1);
    expect(lock->any_locks() == 1);
    // Entry flag with the low bit clear does not count.
    set_flag(2, 1, 2);
    expect(lock->any_locks() == 0);

    // Bit set only in the low byte, not the second: record not scanned.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0x0004;
    set_flag(2, 0, 1);
    expect(lock->any_locks() == 0);

    // Record 0 is never scanned, even with its bit set in both bytes.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0x0101;
    set_flag(0, 0, 1);
    expect(lock->any_locks() == 0);
    expect(lock_any_locks_redirect(lock, nullptr) == 0);

    LockEnableMask = saved_mask;
}

namespace {
void *g_sq_unlock_entries[4];
int g_sq_unlock_slots[4];
int g_sq_unlock_calls;
void __thiscall observe_square_unlock(void *entry, int slot) {
    if (g_sq_unlock_calls < 4) {
        g_sq_unlock_entries[g_sq_unlock_calls] = entry;
        g_sq_unlock_slots[g_sq_unlock_calls] = slot;
    }
    ++g_sq_unlock_calls;
}
}  // namespace

void test_lock_unlock() {
    // unlock(slot) forwards both of record[slot]'s square entries to
    // SquareLock::unlock, clears the record's flag byte, and forgets the slot
    // as the active lock only when it was. The two entries are at record+4 and
    // record+0x10, which the test confirms the seam receives.
    auto *const saved = LockSquareUnlock;
    LockSquareUnlock = &observe_square_unlock;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };

    // Active lock is slot 3: unlocking it clears the 0xE0/0xE4 fields.
    std::memset(obj, 0xAB, sizeof(Lock));
    int32_t three = 3;
    std::memcpy(obj + 0xE0, &three, 4);
    g_sq_unlock_calls = 0;
    lock->unlock(3);
    expect(read32(0xE0) == 0);
    expect(read32(0xE4) == 0);
    // Both entries of record 3 forwarded, at record+4 and record+0x10.
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 3 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 3 * 0x1C + 0x10);
    expect(g_sq_unlock_slots[0] == 3 && g_sq_unlock_slots[1] == 3);
    // The record's flag byte is cleared.
    expect((obj[3 * 0x1C] & 0xFF) == 0);

    // Unlocking a different slot than the active one leaves 0xE0 alone but
    // still unlocks that slot's entries.
    std::memset(obj, 0, sizeof(Lock));
    int32_t five = 5;
    std::memcpy(obj + 0xE0, &five, 4);
    g_sq_unlock_calls = 0;
    lock->unlock(2);
    expect(read32(0xE0) == 5);          // still the active lock
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 2 * 0x1C + 4);

    g_sq_unlock_calls = 0;
    lock_unlock_redirect(lock, nullptr, 1);
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 1 * 0x1C + 4);

    LockSquareUnlock = saved;
}

void test_lock_global_lock() {
    // A try-lock on the 0xE0/0xE4 owner fields: free or already-mine succeeds
    // and records the owner, another owner fails without change. All three
    // cases are checked, including that a failed attempt leaves the fields
    // untouched.
    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_owner = [&](int32_t v) { std::memcpy(obj + 0xE0, &v, 4); };

    // Free (0): succeeds, records owner 3 and marks held.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_owner(0);
    expect(lock->global_lock(3) == 0);
    expect(read32(0xE0) == 3);
    expect(read32(0xE4) == 1);

    // Already held by 3: taking it again as 3 still succeeds.
    expect(lock->global_lock(3) == 0);
    expect(read32(0xE0) == 3);
    expect(read32(0xE4) == 1);

    // Held by 5, another owner 3 fails and changes nothing.
    set_owner(5);
    int32_t held_e4 = 0x77;
    std::memcpy(obj + 0xE4, &held_e4, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(lock->global_lock(3) == 1);
    expect(read32(0xE0) == 5);          // unchanged
    expect(read32(0xE4) == 0x77);       // unchanged
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect takes the free lock.
    set_owner(0);
    expect(lock_global_lock_redirect(lock, nullptr, 9) == 0);
    expect(read32(0xE0) == 9);
}

namespace {
int g_current_server_result = 1;
int g_current_server_calls = 0;
int __cdecl observe_current_server() {
    ++g_current_server_calls;
    return g_current_server_result;
}
}  // namespace

void test_lock_check_global_2() {
    // Reports whether the owner's global lock can be released: only on the
    // server, only when held by that owner, and only when no slot record still
    // has an active square. Every gate is exercised, and success is confirmed
    // to clear the held flag while the failing paths leave it set.
    auto *const saved = LockCurrentServer;
    LockCurrentServer = &observe_current_server;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Not the server: no regardless of the rest.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE4, 1); set32(0xE0, 3);
    g_current_server_result = 0;
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);          // held flag untouched
    g_current_server_result = 1;

    // Not marked held: no.
    set32(0xE4, 0);
    expect(lock->check_global_2(3) == 0);

    // Held, but by a different owner: no.
    set32(0xE4, 1); set32(0xE0, 5);
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);

    // Held by us, but a slot record still active: no, held flag kept.
    set32(0xE0, 3);
    set_flag(4, 1, 1);
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);

    // Held by us, nothing active: yes, and the held flag is cleared.
    set_flag(4, 1, 0);
    expect(lock->check_global_2(3) == 1);
    expect(read32(0xE4) == 0);

    // Record 0 active is ignored (scan starts at 1), so still releasable.
    set32(0xE4, 1);
    set_flag(0, 0, 1);
    expect(lock->check_global_2(3) == 1);
    set32(0xE4, 1);
    expect(lock_check_global_2_redirect(lock, nullptr, 3) == 1);

    LockCurrentServer = saved;
}

namespace {
int g_msg_args[6];
int g_msg_calls;
void __cdecl observe_message_data(int a1, int a2, int a3, int a4, int a5, int a6) {
    g_msg_args[0]=a1; g_msg_args[1]=a2; g_msg_args[2]=a3;
    g_msg_args[3]=a4; g_msg_args[4]=a5; g_msg_args[5]=a6;
    ++g_msg_calls;
}
}  // namespace

void test_lock_check_global() {
    // The void sibling of check_global_2: on a clean release it broadcasts
    // message_data(0x1205, owner, 0, 0, 0, 0) and clears the held flag rather
    // than returning a value. The same four no-op gates apply, and the message
    // arguments are pinned.
    auto *const saved_server = LockCurrentServer;
    auto *const saved_msg = LockMessageData;
    LockCurrentServer = &observe_current_server;   // reused from check_global_2
    LockMessageData = &observe_message_data;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Not the server: nothing.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE4, 1); set32(0xE0, 7);
    g_current_server_result = 0;
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);
    expect(read32(0xE4) == 1);
    g_current_server_result = 1;

    // Not held: nothing.
    set32(0xE4, 0);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);

    // Held, but a record still active: nothing, flag kept.
    set32(0xE4, 1);
    set_flag(3, 0, 1);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);
    expect(read32(0xE4) == 1);

    // Held, nothing active: broadcasts and clears the flag.
    set_flag(3, 0, 0);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 1);
    expect(g_msg_args[0] == 0x1205);
    expect(g_msg_args[1] == 7);          // the owner from 0xE0
    expect(g_msg_args[2] == 0 && g_msg_args[5] == 0);
    expect(read32(0xE4) == 0);

    set32(0xE4, 1);
    g_msg_calls = 0;
    lock_check_global_redirect(lock, nullptr);
    expect(g_msg_calls == 1);

    LockCurrentServer = saved_server;
    LockMessageData = saved_msg;
}

namespace {
void *g_sq_lock_entry;
int g_sq_lock_args[4];
int g_sq_lock_calls;
int g_sq_lock_result;
int __thiscall observe_square_lock(void *entry, int a1, int a2, int a3, int a4) {
    g_sq_lock_entry = entry;
    g_sq_lock_args[0] = a1;
    g_sq_lock_args[1] = a2;
    g_sq_lock_args[2] = a3;
    g_sq_lock_args[3] = a4;
    ++g_sq_lock_calls;
    return g_sq_lock_result;
}
}  // namespace

void test_lock_add_lock() {
    // add_lock(slot, flags, a3, a4) forwards to SquareLock::lock on the slot
    // record's second square entry - at record+0x10, i.e. records_[slot]+0x1C*slot
    // +0x10 - with mask bit 0x10 forced into the flags argument, and returns
    // whatever the seam returns. The seam receives (entry, slot, flags|0x10,
    // a3, a4); the two things a wrong recovery would get wrong are the entry
    // index (record+0x10, not record+4) and the 0x10 bit.
    auto *const saved = LockSquareLock;
    LockSquareLock = &observe_square_lock;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    std::memset(obj, 0xCD, sizeof(Lock));

    g_sq_lock_calls = 0;
    g_sq_lock_result = 0x1234;
    int rv = lock->add_lock(3, 0x21, 7, 9);
    expect(g_sq_lock_calls == 1);
    expect(g_sq_lock_entry == obj + 3 * 0x1C + 0x10);   // entries[1]
    expect(g_sq_lock_args[0] == 3);                     // slot
    expect(g_sq_lock_args[1] == (0x21 | 0x10));         // flags with 0x10 set
    expect(g_sq_lock_args[2] == 7);
    expect(g_sq_lock_args[3] == 9);
    expect(rv == 0x1234);                               // result passthrough

    // A slot-0 lock still targets record 0's second entry, and the 0x10 bit is
    // set even when the caller's flags already lack it.
    g_sq_lock_calls = 0;
    g_sq_lock_result = -1;
    rv = lock_add_lock_redirect(lock, nullptr, 0, 0, 1, 2);
    expect(g_sq_lock_calls == 1);
    expect(g_sq_lock_entry == obj + 0x10);
    expect(g_sq_lock_args[1] == 0x10);
    expect(rv == -1);

    LockSquareLock = saved;
}

namespace {
// A two-call recorder for the double-lock in Lock::lock: captures each call's
// entry pointer and four args, and returns a per-call configured result.
void *g_sq_lock2_entries[2];
int g_sq_lock2_args[2][4];
int g_sq_lock2_results[2];
int g_sq_lock2_calls;
int __thiscall observe_square_lock2(void *entry, int a1, int a2, int a3, int a4) {
    int idx = g_sq_lock2_calls;
    if (idx < 2) {
        g_sq_lock2_entries[idx] = entry;
        g_sq_lock2_args[idx][0] = a1;
        g_sq_lock2_args[idx][1] = a2;
        g_sq_lock2_args[idx][2] = a3;
        g_sq_lock2_args[idx][3] = a4;
    }
    ++g_sq_lock2_calls;
    return idx < 2 ? g_sq_lock2_results[idx] : 0;
}
}  // namespace

void test_lock_lock() {
    // Lock::lock(slot, flags, a3, a4, a5, a6, a7) locks both of a slot's square
    // entries: entries[0] with (slot, flags, a3, a4), entries[1] with (slot, a5,
    // a6, a7). It refuses when another owner holds the global lock, takes the
    // global lock when flags&2 is set, unwinds (unlocking both) when either
    // square fails, and on a global success over the server runs the release
    // broadcast (skipping its own slot) that check_global does.
    auto *const saved_lock = LockSquareLock;
    auto *const saved_unlock = LockSquareUnlock;
    auto *const saved_server = LockCurrentServer;
    auto *const saved_msg = LockMessageData;
    LockSquareLock = &observe_square_lock2;
    LockSquareUnlock = &observe_square_unlock;
    LockCurrentServer = &observe_current_server;
    LockMessageData = &observe_message_data;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Refusal: another owner (5) holds the global lock, a lock for slot 3 is
    // rejected outright - no square touched, owner unchanged.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE0, 5);
    g_sq_lock2_calls = 0;
    expect(lock->lock(3, 2, 0, 0, 0, 0, 0) == 1);
    expect(g_sq_lock2_calls == 0);
    expect(read32(0xE0) == 5);

    // Plain double-lock (flags&2 clear, no global): both squares locked with
    // the right entry pointers and arg groups, entries reset to sentinels
    // first, no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    int rv = lock->lock(3, 0, 11, 12, 13, 14, 15);
    expect(rv == 0);
    expect(g_sq_lock2_calls == 2);
    expect(g_sq_lock2_entries[0] == obj + 3 * 0x1C + 4);    // entries[0]
    expect(g_sq_lock2_args[0][0] == 3 && g_sq_lock2_args[0][1] == 0 &&
           g_sq_lock2_args[0][2] == 11 && g_sq_lock2_args[0][3] == 12);
    expect(g_sq_lock2_entries[1] == obj + 3 * 0x1C + 0x10);  // entries[1]
    expect(g_sq_lock2_args[1][0] == 3 && g_sq_lock2_args[1][1] == 13 &&
           g_sq_lock2_args[1][2] == 14 && g_sq_lock2_args[1][3] == 15);
    // Both entries were reset to {-1, -1, 0} before the locks ran.
    expect(read32(3 * 0x1C + 4) == -1 && read32(3 * 0x1C + 8) == -1);
    expect(read32(3 * 0x1C + 0x10) == -1 && read32(3 * 0x1C + 0x14) == -1);
    expect(read32(0xE0) == 0 && g_msg_calls == 0);          // no global taken

    // Global success off the server: owner and held flag are set, no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    g_current_server_result = 0;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0);
    expect(read32(0xE0) == 4 && read32(0xE4) == 1);
    expect(g_msg_calls == 0);

    // Global success on the server, nothing else active: broadcasts the
    // release and drops the held flag.
    std::memset(obj, 0, sizeof(Lock));
    g_current_server_result = 1;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0);
    expect(g_msg_calls == 1);
    expect(g_msg_args[0] == 0x1205 && g_msg_args[1] == 4);
    expect(read32(0xE4) == 0);

    // Global success on the server but another slot still has an active square:
    // no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    set_flag(2, 0, 1);                     // slot 2, first entry active
    g_current_server_result = 1;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0 && g_msg_calls == 0);

    // First square fails: only the first lock is attempted, both entries are
    // unlocked, and the global lock this slot held is dropped.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 99; g_sq_lock2_results[1] = 0;
    g_sq_unlock_calls = 0;
    rv = lock->lock(6, 2, 0, 0, 0, 0, 0);
    expect(rv == 1);
    expect(g_sq_lock2_calls == 1);         // second lock skipped
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 6 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 6 * 0x1C + 0x10);
    expect(read32(0xE0) == 0 && read32(0xE4) == 0);   // owner dropped

    // Second square fails: both locks attempted, both entries unlocked.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 7;
    g_sq_unlock_calls = 0;
    rv = lock_lock_redirect(lock, nullptr, 2, 0, 0, 0, 0, 0, 0);
    expect(rv == 1);
    expect(g_sq_lock2_calls == 2);
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 2 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 2 * 0x1C + 0x10);

    LockSquareLock = saved_lock;
    LockSquareUnlock = saved_unlock;
    LockCurrentServer = saved_server;
    LockMessageData = saved_msg;
}

// map.cpp is not linked into the leaf-test target, so SquareLock::unlock's
// direct dependencies are provided here: the two map-bound globals it reads,
// a faithful copy of the tiny xrange it wraps x through, and an observer in
// place of unlock_map that records the footprint calls the test then checks.
struct SqUnlockMapCall { int x, y, faction; };
std::vector<SqUnlockMapCall> g_sq_unlock_map_calls;

void __cdecl unlock_map(uint32_t x, uint32_t y, uint32_t faction_id) {
    g_sq_unlock_map_calls.push_back(
        {static_cast<int>(x), static_cast<int>(y), static_cast<int>(faction_id)});
}

// lock_map's observer records its calls and can be told to report one tile as
// already held (a nonzero BOOL) so the abort path can be exercised.
struct SqLockMapCall { int x, y, faction; };
std::vector<SqLockMapCall> g_sq_lock_map_calls;
int g_sq_lock_map_fail_at = -1;

BOOL __cdecl lock_map(uint32_t x, uint32_t y, uint32_t faction_id) {
    int i = static_cast<int>(g_sq_lock_map_calls.size());
    g_sq_lock_map_calls.push_back(
        {static_cast<int>(x), static_cast<int>(y), static_cast<int>(faction_id)});
    return (g_sq_lock_map_fail_at >= 0 && i == g_sq_lock_map_fail_at) ? TRUE : FALSE;
}

int __cdecl xrange(int x) {
    if (!*MapIsFlat) {
        if (x >= 0) {
            if (x >= *MapLongitudeBounds) {
                x -= *MapLongitudeBounds;
            }
        } else {
            x += *MapLongitudeBounds;
        }
    }
    return x;
}

int *MapLongitudeBounds;
int *MapLatitudeBounds;
BOOL *MapIsFlat;

// game.cpp is not linked either, so Console::edit_lock's GameState global is
// provided here, alongside a stand-in for the GetKeyState import it consults.
uint32_t *GameState;

// general.cpp is not linked here either; Console::editor_undo's only call is
// observed rather than run.
int g_load_undo_arg;
int g_load_undo_calls;
void __cdecl load_undo(int type) { g_load_undo_arg = type; ++g_load_undo_calls; }

namespace {
int g_console_key_ret;
int g_console_key_vk;
}  // namespace
SHORT __stdcall fake_console_get_key_state(int virtual_key) {
    g_console_key_vk = virtual_key;
    return static_cast<SHORT>(g_console_key_ret);
}
func_get_key_state *g_console_key_fn = &fake_console_get_key_state;

void test_square_lock_unlock() {
    // SquareLock::unlock releases the square and its footprint. When the lock
    // bit (0x1) is clear it does nothing; when set it clears the record to its
    // unset sentinels and, for an on-map coordinate, calls unlock_map over a
    // footprint whose size the flags choose - one tile, or a radius of 25 (0x4
    // set, 0x10 clear) or 81 (also 0x8 set) tiles - walked through the shared
    // RadiusOffset tables and wrapped in x by xrange. unlock_map is observed
    // (map.cpp is not linked here); the expected call sequence is recomputed the
    // same way, in order, pinning the count, the offset tables, the x-wrap, the
    // bounds skip, and the factionID passthrough at once.
    int width = 40, height = 40;
    BOOL flat = TRUE;
    MapLongitudeBounds = &width;
    MapLatitudeBounds = &height;
    MapIsFlat = &flat;

    std::vector<uint8_t> storage(sizeof(SquareLock) + 32);
    auto *sq = reinterpret_cast<SquareLock *>(storage.data() + 16);
    uint8_t *const rec = storage.data() + 16;
    auto set_rec = [&](int32_t first, int32_t second, int32_t flag) {
        std::memcpy(rec + 0, &first, 4);
        std::memcpy(rec + 4, &second, 4);
        std::memcpy(rec + 8, &flag, 4);
    };
    auto read_rec = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, rec + off, 4); return v;
    };

    // Lock bit clear: nothing happens, the record is left untouched.
    g_sq_unlock_map_calls.clear();
    set_rec(5, 7, 0);
    sq->unlock(3);
    expect(read_rec(0) == 5 && read_rec(4) == 7 && read_rec(8) == 0);
    expect(g_sq_unlock_map_calls.empty());

    // Lock set but the coordinate is off the map: the record is cleared to its
    // sentinels with no footprint work. All four out-of-bounds directions.
    for (auto coord : {std::pair<int, int>{width + 5, 20}, {-1, 20},
                       {20, height}, {20, -1}}) {
        g_sq_unlock_map_calls.clear();
        set_rec(coord.first, coord.second, 1);
        sq->unlock(3);
        expect(read_rec(0) == -1 && read_rec(4) == -1 && read_rec(8) == 0);
        expect(g_sq_unlock_map_calls.empty());
    }

    // On-map: the observed unlock_map call sequence must match, in order, the
    // footprint the flags select (recomputed via xrange + RadiusOffset), and the
    // record ends cleared to sentinels.
    auto run_case = [&](int rec_x, int rec_y, int flag, int count) {
        g_sq_unlock_map_calls.clear();
        set_rec(rec_x, rec_y, flag);
        sq->unlock(3);
        expect(read_rec(0) == -1 && read_rec(4) == -1 && read_rec(8) == 0);
        std::vector<SqUnlockMapCall> want;
        for (int i = 0; i < count; ++i) {
            int nx = xrange(rec_x + RadiusOffsetX[i]);
            int ny = rec_y + RadiusOffsetY[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                want.push_back({nx, ny, 3});
            }
        }
        expect(g_sq_unlock_map_calls.size() == want.size());
        for (size_t i = 0; i < want.size() &&
                           i < g_sq_unlock_map_calls.size(); ++i) {
            expect(g_sq_unlock_map_calls[i].x == want[i].x &&
                   g_sq_unlock_map_calls[i].y == want[i].y &&
                   g_sq_unlock_map_calls[i].faction == want[i].faction);
        }
    };

    flat = TRUE;
    run_case(20, 20, 1, 1);            // plain lock: a single tile
    run_case(20, 20, 1 | 4, 25);       // 0x4 -> radius of 25
    run_case(20, 20, 1 | 4 | 8, 81);   // 0x4 and 0x8 -> radius of 81
    run_case(20, 20, 1 | 4 | 0x10, 1); // 0x10 overrides the radius back to 1
    run_case(20, 20, 1 | 8, 1);        // 0x8 without 0x4 stays a single tile

    // Round map: a lock at the x-edge whose footprint wraps around, so xrange
    // (not a raw add) is what the recovery must use.
    flat = FALSE;
    run_case(1, 20, 1 | 4, 25);
    flat = TRUE;

    // The redirect entry drives the same footprint, and passes its faction.
    g_sq_unlock_map_calls.clear();
    set_rec(20, 20, 1);
    square_lock_unlock_redirect(sq, nullptr, 6);
    expect(g_sq_unlock_map_calls.size() == 1);
    expect(g_sq_unlock_map_calls[0].x == 20 && g_sq_unlock_map_calls[0].y == 20 &&
           g_sq_unlock_map_calls[0].faction == 6);
}

void test_square_lock_lock() {
    // SquareLock::lock stores the coordinate and flags|1 in the record
    // unconditionally, then - for an on-map coordinate - calls lock_map over the
    // same flag-sized footprint SquareLock::unlock walks, abandoning the attempt
    // (return 1) at the first tile another faction already holds. lock_map is
    // observed here; the expected call sequence is recomputed the same way.
    int width = 40, height = 40;
    BOOL flat = TRUE;
    MapLongitudeBounds = &width;
    MapLatitudeBounds = &height;
    MapIsFlat = &flat;

    std::vector<uint8_t> storage(sizeof(SquareLock) + 32);
    auto *sq = reinterpret_cast<SquareLock *>(storage.data() + 16);
    uint8_t *const rec = storage.data() + 16;
    auto read_rec = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, rec + off, 4); return v;
    };

    // Off-map: the record is still stamped with the coordinate and flags|1, but
    // no footprint work happens and the result is 0.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = -1;
    std::memset(rec, 0xEE, sizeof(SquareLock));
    expect(sq->lock(3, 4, 100, 20) == 0);   // x=100 off-map
    expect(read_rec(0) == 100 && read_rec(4) == 20 && read_rec(8) == (4 | 1));
    expect(g_sq_lock_map_calls.empty());

    // On-map: the observed lock_map sequence matches the footprint the flags
    // select, the record is stamped, and the result is 0 when nothing is held.
    auto run_case = [&](int rec_x, int rec_y, int flags, int count) {
        g_sq_lock_map_calls.clear();
        g_sq_lock_map_fail_at = -1;
        expect(sq->lock(3, flags, rec_x, rec_y) == 0);
        expect(read_rec(0) == rec_x && read_rec(4) == rec_y &&
               read_rec(8) == (flags | 1));
        std::vector<SqLockMapCall> want;
        for (int i = 0; i < count; ++i) {
            int nx = xrange(rec_x + RadiusOffsetX[i]);
            int ny = rec_y + RadiusOffsetY[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                want.push_back({nx, ny, 3});
            }
        }
        expect(g_sq_lock_map_calls.size() == want.size());
        for (size_t i = 0; i < want.size() &&
                           i < g_sq_lock_map_calls.size(); ++i) {
            expect(g_sq_lock_map_calls[i].x == want[i].x &&
                   g_sq_lock_map_calls[i].y == want[i].y &&
                   g_sq_lock_map_calls[i].faction == want[i].faction);
        }
    };

    flat = TRUE;
    run_case(20, 20, 1, 1);            // plain lock: a single tile
    run_case(20, 20, 1 | 4, 25);       // 0x4 -> radius of 25
    run_case(20, 20, 1 | 4 | 8, 81);   // 0x4 and 0x8 -> radius of 81
    run_case(20, 20, 1 | 4 | 0x10, 1); // 0x10 overrides the radius back to 1
    run_case(20, 20, 8, 1);            // 0x8 without 0x4 stays a single tile

    // Round map: a lock at the x-edge whose footprint wraps around.
    flat = FALSE;
    run_case(1, 20, 1 | 4, 25);
    flat = TRUE;

    // A contested tile: lock_map reports the 4th tile (call index 3) already
    // held, so the attempt aborts there with only four calls and a result of 1.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = 3;
    expect(sq->lock(3, 1 | 4, 20, 20) == 1);
    expect(g_sq_lock_map_calls.size() == 4);

    // The redirect entry forwards all four args, including the faction.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = -1;
    expect(square_lock_lock_redirect(sq, nullptr, 6, 1, 20, 20) == 0);
    expect(g_sq_lock_map_calls.size() == 1);
    expect(g_sq_lock_map_calls[0].faction == 6 &&
           g_sq_lock_map_calls[0].x == 20 && g_sq_lock_map_calls[0].y == 20);
}

void test_console_edit_lock() {
    // edit_lock is meaningful only in the scenario editor: outside it, always
    // unlocked. Inside it, editing locks when Scroll Lock is toggled (bit 0 of
    // GetKeyState) or the game is in editor-only mode. GetKeyState is stood in
    // for through the rebound import slot so the key state is controllable.
    uint32_t state = 0;
    auto *const saved_gs = GameState;
    auto *const saved_slot = ConsoleEditKeyStateSlot;
    GameState = &state;
    g_console_key_fn = &fake_console_get_key_state;
    ConsoleEditKeyStateSlot = &g_console_key_fn;

    std::vector<uint8_t> buf(64);   // this is never dereferenced by edit_lock
    auto *con = reinterpret_cast<Console *>(buf.data());

    // Not in the scenario editor: unlocked regardless of key or editor-only bit.
    state = 0;
    g_console_key_ret = 1;
    expect(con->edit_lock() == 0);
    state = STATE_EDITOR_ONLY_MODE;
    expect(con->edit_lock() == 0);

    // Scenario editor + Scroll Lock toggled (bit 0) locks editing, and the key
    // queried is Scroll Lock.
    state = STATE_SCENARIO_EDITOR;
    g_console_key_ret = 1;
    g_console_key_vk = 0;
    expect(con->edit_lock() == 1);
    expect(g_console_key_vk == VK_SCROLL);

    // Only bit 0 counts: the down bit (0x80) alone does not lock.
    g_console_key_ret = 0x80;
    expect(con->edit_lock() == 0);

    // Editor-only mode locks editing even with the key clear.
    g_console_key_ret = 0;
    state = STATE_SCENARIO_EDITOR | STATE_EDITOR_ONLY_MODE;
    expect(con->edit_lock() == 1);

    // Scenario editor, key clear, not editor-only: unlocked.
    state = STATE_SCENARIO_EDITOR;
    g_console_key_ret = 0;
    expect(con->edit_lock() == 0);

    // Redirect entry.
    g_console_key_ret = 1;
    expect(console_edit_lock_redirect(con, nullptr) == 1);

    GameState = saved_gs;
    ConsoleEditKeyStateSlot = saved_slot;
}

namespace {
void *g_dev_call_self;
int g_dev_enable_calls;
int g_dev_disable_calls;
void __thiscall observe_dev_enable(void *self) {
    g_dev_call_self = self; ++g_dev_enable_calls;
}
void __thiscall observe_dev_disable(void *self) {
    g_dev_call_self = self; ++g_dev_disable_calls;
}
}  // namespace

void test_wave_device_enable_disable() {
    // enable and disable drive the device wrapped at offset 0x14 through two
    // adjacent vtable slots - 0x60 for enable, 0x64 for disable, called with the
    // device as this - and do nothing when there is no wrapped device.
    std::vector<uint8_t> wd(sizeof(Wave_Device) + 16, 0);
    auto *dev = reinterpret_cast<Wave_Device *>(wd.data());

    void *vtable[32] = {};
    vtable[0x60 / 4] = reinterpret_cast<void *>(&observe_dev_enable);
    vtable[0x64 / 4] = reinterpret_cast<void *>(&observe_dev_disable);
    struct FakeDevice { void *vtbl; } fake_device;
    fake_device.vtbl = vtable;

    auto set_device = [&](void *d) { std::memcpy(wd.data() + 0x14, &d, sizeof(d)); };

    // No device: both are no-ops.
    set_device(nullptr);
    g_dev_enable_calls = g_dev_disable_calls = 0;
    dev->enable();
    dev->disable();
    expect(g_dev_enable_calls == 0 && g_dev_disable_calls == 0);

    // With a device: enable hits slot 0x60 with the device as this.
    set_device(&fake_device);
    g_dev_enable_calls = g_dev_disable_calls = 0;
    g_dev_call_self = nullptr;
    dev->enable();
    expect(g_dev_enable_calls == 1 && g_dev_disable_calls == 0);
    expect(g_dev_call_self == &fake_device);

    // disable hits the adjacent slot 0x64.
    g_dev_call_self = nullptr;
    dev->disable();
    expect(g_dev_disable_calls == 1 && g_dev_enable_calls == 1);
    expect(g_dev_call_self == &fake_device);

    // Redirect entries drive the same two slots.
    g_dev_enable_calls = g_dev_disable_calls = 0;
    wave_device_enable_redirect(dev, nullptr);
    wave_device_disable_redirect(dev, nullptr);
    expect(g_dev_enable_calls == 1 && g_dev_disable_calls == 1);
}

namespace {
void *g_sound_slot0_self;
int g_sound_slot0_arg;
int g_sound_slot0_calls;
int g_sound_slot0_ret;
void *g_sound_fallback_self;
int g_sound_fallback_calls;
// The fallback dispatch re-reads the object's vtable pointer, so slot 0 is
// given a chance to swap it; when it does, the fallback must come from the
// replacement table.
void **g_sound_swap_to;
void *g_sound_slot0_obj;

int __thiscall observe_sound_slot0(Sound *self, int a1) {
    g_sound_slot0_self = self;
    g_sound_slot0_arg = a1;
    ++g_sound_slot0_calls;
    if (g_sound_swap_to) {
        std::memcpy(g_sound_slot0_obj, &g_sound_swap_to, sizeof(g_sound_swap_to));
    }
    return g_sound_slot0_ret;
}
void __thiscall observe_sound_fallback(Sound *self) {
    g_sound_fallback_self = self;
    ++g_sound_fallback_calls;
}
int g_sound_fallback2_calls;
void __thiscall observe_sound_fallback2(Sound *) { ++g_sound_fallback2_calls; }
}  // namespace

void test_sound_fade() {
    // fade asks the object's own vtable slot 0 to fade with the argument, and
    // runs slot 0x28 as a fallback only when slot 0 declines by returning zero.
    // Both dispatches pass the Sound as this, and both read the object's live
    // vtable pointer - the original loads it twice, so a slot-0 that swaps the
    // table must redirect the fallback.
    std::vector<uint8_t> storage(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(storage.data());

    void *vtable[16] = {};
    vtable[0] = reinterpret_cast<void *>(&observe_sound_slot0);
    vtable[0x28 / 4] = reinterpret_cast<void *>(&observe_sound_fallback);
    void *vtable_ptr = vtable;
    std::memcpy(storage.data(), &vtable_ptr, sizeof(vtable_ptr));
    g_sound_slot0_obj = storage.data();

    // Slot 0 accepts (nonzero): no fallback, and it got the argument and this.
    g_sound_swap_to = nullptr;
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 1;
    sound->fade(77);
    expect(g_sound_slot0_calls == 1 && g_sound_fallback_calls == 0);
    expect(g_sound_slot0_self == sound && g_sound_slot0_arg == 77);

    // Slot 0 declines (zero): the fallback at 0x28 runs, with the same this.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 0;
    sound->fade(5);
    expect(g_sound_slot0_calls == 1 && g_sound_fallback_calls == 1);
    expect(g_sound_fallback_self == sound && g_sound_slot0_arg == 5);

    // A negative return is still nonzero, so no fallback.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = -1;
    sound->fade(0);
    expect(g_sound_fallback_calls == 0);

    // The fallback comes from the vtable as it stands after slot 0 ran: slot 0
    // swaps the object's table, and the second table's 0x28 is what runs.
    void *vtable2[16] = {};
    vtable2[0x28 / 4] = reinterpret_cast<void *>(&observe_sound_fallback2);
    g_sound_swap_to = vtable2;
    g_sound_slot0_ret = 0;
    g_sound_fallback_calls = g_sound_fallback2_calls = 0;
    sound->fade(1);
    expect(g_sound_fallback2_calls == 1 && g_sound_fallback_calls == 0);
    g_sound_swap_to = nullptr;
    std::memcpy(storage.data(), &vtable_ptr, sizeof(vtable_ptr));

    // Redirect entry forwards the argument and drives the same path.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 0;
    sound_fade_redirect(sound, nullptr, 42);
    expect(g_sound_slot0_calls == 1 && g_sound_slot0_arg == 42);
    expect(g_sound_fallback_calls == 1);
}

namespace {
void *g_wave_dev_self;
int g_wave_dev_calls;
int g_wave_dev_ret;
void *g_wave_self_self;
int g_wave_self_calls;
int __thiscall observe_wave_device_unload(void *self) {
    g_wave_dev_self = self; ++g_wave_dev_calls; return g_wave_dev_ret;
}
void __thiscall observe_wave_self_slot(Wave *self) {
    g_wave_self_self = self; ++g_wave_self_calls;
}
}  // namespace

void test_wave_unload() {
    // unload asks the wrapped device at 0x3C to unload through the device's own
    // vtable slot 0x14, returns that result, forgets the device, runs the
    // object's own vtable slot 0x80 unless bit 1 of the flag byte at 0x54
    // suppresses it, and clears bit 0 of the flag dword at 0x40.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    auto *wave = reinterpret_cast<Wave *>(storage.data());
    uint8_t *const obj = storage.data();

    void *wave_vtable[64] = {};
    wave_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_self_slot);
    void *wave_vtable_ptr = wave_vtable;
    std::memcpy(obj, &wave_vtable_ptr, sizeof(wave_vtable_ptr));

    void *dev_vtable[16] = {};
    dev_vtable[0x14 / 4] = reinterpret_cast<void *>(&observe_wave_device_unload);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto read32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // A wrapped device: its slot 0x14 runs with the device as this, its result
    // is returned, the device pointer is cleared, the self slot runs, and only
    // bit 0 of the 0x40 dword is cleared.
    set_device(&fake_dev);
    set32(0x40, 0xFFFFFFFFu);
    obj[0x54] = 0;
    g_wave_dev_calls = g_wave_self_calls = 0;
    g_wave_dev_ret = 0x5A;
    expect(wave->unload() == 0x5A);
    expect(g_wave_dev_calls == 1 && g_wave_dev_self == &fake_dev);
    expect(read32(0x3C) == 0);                 // device forgotten
    expect(g_wave_self_calls == 1 && g_wave_self_self == wave);
    expect(read32(0x40) == 0xFFFFFFFEu);       // only bit 0 cleared

    // No device: nothing is dispatched to a device and the result is 0, but the
    // self slot still runs and the bit is still cleared.
    set_device(nullptr);
    set32(0x40, 1);
    g_wave_dev_calls = g_wave_self_calls = 0;
    expect(wave->unload() == 0);
    expect(g_wave_dev_calls == 0);
    expect(g_wave_self_calls == 1);
    expect(read32(0x40) == 0);

    // Bit 1 of the 0x54 flag byte suppresses the self slot; everything else
    // still happens.
    set_device(&fake_dev);
    set32(0x40, 0xF);
    obj[0x54] = 2;
    g_wave_dev_calls = g_wave_self_calls = 0;
    g_wave_dev_ret = 7;
    expect(wave->unload() == 7);
    expect(g_wave_dev_calls == 1);
    expect(g_wave_self_calls == 0);            // suppressed
    expect(read32(0x40) == 0xE);
    expect(read32(0x3C) == 0);

    // Other bits of the 0x54 byte do not suppress it.
    set_device(nullptr);
    obj[0x54] = 0xFD;                          // everything but bit 1
    g_wave_self_calls = 0;
    wave->unload();
    expect(g_wave_self_calls == 1);

    // Redirect entry returns the device result.
    set_device(&fake_dev);
    obj[0x54] = 0;
    g_wave_dev_ret = 0x33;
    expect(wave_unload_redirect(wave, nullptr) == 0x33);
}

namespace {
int g_wave_pitch_arg;
int g_wave_pitch_calls;
void *g_wave_pitch_self;
void __thiscall observe_wave_set_pitch(void *self, int pitch) {
    g_wave_pitch_self = self; g_wave_pitch_arg = pitch; ++g_wave_pitch_calls;
}
int g_wave_load_a1, g_wave_load_a2, g_wave_load_calls;
void *g_wave_load_self;
void __thiscall observe_wave_load_slot(Wave *self, int a1, int a2) {
    g_wave_load_self = self; g_wave_load_a1 = a1; g_wave_load_a2 = a2;
    ++g_wave_load_calls;
}
int g_wave_follow_calls, g_wave_follow_ret;
void *g_wave_follow_self;
int __thiscall observe_wave_follow_slot(Wave *self) {
    g_wave_follow_self = self; ++g_wave_follow_calls; return g_wave_follow_ret;
}
}  // namespace

void test_wave_set_pitch_and_load() {
    // set_pitch clamps to [-1200, 1200], stores the clamped value at 0x58, and
    // forwards it to the wrapped device's vtable slot 0x98 when one is wrapped.
    // load runs the object's own slot 0x88 with both arguments, then - unless
    // bit 2 of the second argument skips it - its own slot 0x8C, whose result
    // it returns.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    auto *wave = reinterpret_cast<Wave *>(storage.data());
    uint8_t *const obj = storage.data();

    void *wave_vtable[64] = {};
    wave_vtable[0x88 / 4] = reinterpret_cast<void *>(&observe_wave_load_slot);
    wave_vtable[0x8C / 4] = reinterpret_cast<void *>(&observe_wave_follow_slot);
    void *wave_vtable_ptr = wave_vtable;
    std::memcpy(obj, &wave_vtable_ptr, sizeof(wave_vtable_ptr));

    void *dev_vtable[64] = {};
    dev_vtable[0x98 / 4] = reinterpret_cast<void *>(&observe_wave_set_pitch);
    struct FakeDev2 { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto pitch_field = [&]() {
        int32_t v = 0; std::memcpy(&v, obj + 0x58, 4); return v;
    };

    // In range: stored and forwarded verbatim.
    set_device(&fake_dev);
    g_wave_pitch_calls = 0;
    wave->set_pitch(500);
    expect(pitch_field() == 500);
    expect(g_wave_pitch_calls == 1 && g_wave_pitch_arg == 500);
    expect(g_wave_pitch_self == &fake_dev);

    // Above the ceiling clamps to 1200; the clamped value is what is forwarded.
    wave->set_pitch(99999);
    expect(pitch_field() == 1200 && g_wave_pitch_arg == 1200);
    wave->set_pitch(1200);          // the boundary itself is kept
    expect(pitch_field() == 1200);
    wave->set_pitch(1201);
    expect(pitch_field() == 1200);

    // Below the floor clamps to -1200.
    wave->set_pitch(-99999);
    expect(pitch_field() == -1200 && g_wave_pitch_arg == -1200);
    wave->set_pitch(-1200);         // the boundary itself is kept
    expect(pitch_field() == -1200);
    wave->set_pitch(-1201);
    expect(pitch_field() == -1200);

    // No device: still stored, nothing dispatched.
    set_device(nullptr);
    g_wave_pitch_calls = 0;
    wave->set_pitch(7);
    expect(pitch_field() == 7 && g_wave_pitch_calls == 0);

    // Redirect entry.
    set_device(&fake_dev);
    g_wave_pitch_calls = 0;
    wave_set_pitch_redirect(wave, nullptr, -30);
    expect(pitch_field() == -30 && g_wave_pitch_arg == -30);

    // load: slot 0x88 gets both arguments and this; the follow-up runs and its
    // result is returned.
    g_wave_load_calls = g_wave_follow_calls = 0;
    g_wave_follow_ret = 0x2B;
    expect(wave->load(11, 0) == 0x2B);
    expect(g_wave_load_calls == 1 && g_wave_load_a1 == 11 && g_wave_load_a2 == 0);
    expect(g_wave_load_self == wave);
    expect(g_wave_follow_calls == 1 && g_wave_follow_self == wave);

    // Bit 2 of the second argument skips the follow-up and returns 0, but the
    // first slot still runs with the argument intact.
    g_wave_load_calls = g_wave_follow_calls = 0;
    expect(wave->load(12, 4) == 0);
    expect(g_wave_load_calls == 1 && g_wave_load_a2 == 4);
    expect(g_wave_follow_calls == 0);

    // Other bits do not skip it.
    g_wave_follow_calls = 0;
    g_wave_follow_ret = 9;
    expect(wave->load(13, 0xFB) == 9);
    expect(g_wave_follow_calls == 1);

    // Redirect entry forwards both arguments.
    g_wave_load_calls = g_wave_follow_calls = 0;
    g_wave_follow_ret = 3;
    expect(wave_load_redirect(wave, nullptr, 21, 0) == 3);
    expect(g_wave_load_a1 == 21 && g_wave_follow_calls == 1);
}

void test_zeroed_constant_return_stubs() {
    // Eleven stubs whose bodies are nothing but `xor eax, eax; ret` - three
    // bytes each. They were hidden from the constant-return scan by a
    // length check that could never match that shape, so they are all
    // recovered together here: each returns zero and writes nothing.
    // The device layouts are bounded rather than established, so as elsewhere
    // the canary is sized to what is modelled; a method that should touch no
    // field writes nothing regardless of where the object really ends.
    alignas(Midi_Device) uint8_t midi_storage[sizeof(Midi_Device) + 32];
    uint8_t midi_expected[sizeof(midi_storage)];
    auto *midi = reinterpret_cast<Midi_Device *>(midi_storage + 16);
    seed_storage(midi_storage, midi_expected, sizeof(midi_storage));
    std::memcpy(midi_expected, midi_storage, sizeof(midi_storage));
    expect(midi->get_ndevices() == 0);
    expect(midi->get_volume() == 0);
    expect(midi->stop() == 0);
    expect(midi->get_rate() == 0);
    expect(midi_device_get_ndevices_redirect(midi, nullptr) == 0);
    expect(midi_device_get_volume_redirect(midi, nullptr) == 0);
    expect(midi_device_stop_redirect(midi, nullptr) == 0);
    expect(midi_device_get_rate_redirect(midi, nullptr) == 0);
    expect_storage_bytes(midi_storage, midi_expected, sizeof(midi_storage));

    alignas(Wave_In_Device) uint8_t win_storage[sizeof(Wave_In_Device) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(win_storage + 16);
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    expect(wave_in->get_ndevices() == 0);
    expect(wave_in->stop() == 0);
    expect(wave_in->get_rate() == 0);
    expect(wave_in_device_get_ndevices_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_stop_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_get_rate_redirect(wave_in, nullptr) == 0);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    alignas(Wave_Device) uint8_t wd_storage[sizeof(Wave_Device) + 32];
    uint8_t wd_expected[sizeof(wd_storage)];
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd_storage + 16);
    seed_storage(wd_storage, wd_expected, sizeof(wd_storage));
    std::memcpy(wd_expected, wd_storage, sizeof(wd_storage));
    expect(wave_dev->get_volume() == 0);
    expect(wave_dev->stop() == 0);
    expect(wave_device_get_volume_redirect(wave_dev, nullptr) == 0);
    expect(wave_device_stop_redirect(wave_dev, nullptr) == 0);
    expect_storage_bytes(wd_storage, wd_expected, sizeof(wd_storage));

    // A static with no object at all, and a MapWin method that ignores its own.
    expect(CheckButton::init_class() == 0);
    expect(check_button_init_class_redirect() == 0);

    alignas(MapWin) uint8_t mw_storage[sizeof(MapWin) + 32];
    uint8_t mw_expected[sizeof(mw_storage)];
    auto *map_win = reinterpret_cast<MapWin *>(mw_storage + 16);
    seed_storage(mw_storage, mw_expected, sizeof(mw_storage));
    std::memcpy(mw_expected, mw_storage, sizeof(mw_storage));
    expect(map_win->UNK2() == 0);
    expect(map_win_unk2_redirect(map_win, nullptr) == 0);
    expect_storage_bytes(mw_storage, mw_expected, sizeof(mw_storage));
}

namespace {
// One recorder for the whole wrapped-device forwarder family: every method
// under test reaches exactly one vtable slot, so recording which slot ran and
// what it was passed is enough to tell them apart.
int g_fwd_slot_ran;
void *g_fwd_self;
int g_fwd_calls;
int g_fwd_ret;
template <int Slot>
int __thiscall observe_fwd_slot(void *self) {
    g_fwd_slot_ran = Slot; g_fwd_self = self; ++g_fwd_calls; return g_fwd_ret;
}
}  // namespace

void test_wrapped_device_forwarders() {
    // Fourteen methods of one shape: forward to the device the object wraps -
    // at 0x14 for the two device classes, 0x3C for Sound - through one slot of
    // that device's vtable, answering zero (or doing nothing, when void) if no
    // device is wrapped. Each is checked for the slot it reaches, that the
    // device is passed as this, and that the device's answer comes back.
    void *vt[64] = {};
    vt[0x14 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x14>);
    vt[0x34 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x34>);
    vt[0x3C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x3C>);
    vt[0x48 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x48>);
    vt[0x4C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x4C>);
    vt[0x54 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x54>);
    vt[0x58 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x58>);
    vt[0x5C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x5C>);
    vt[0x70 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x70>);
    vt[0x74 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x74>);
    struct FwdDev { void *vtbl; } fwd_dev;
    fwd_dev.vtbl = vt;

    std::vector<uint8_t> wd(sizeof(Wave_Device) + 32, 0);
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd.data());
    std::vector<uint8_t> md(sizeof(Midi_Device) + 32, 0);
    auto *midi = reinterpret_cast<Midi_Device *>(md.data());
    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());

    void *dev = &fwd_dev;
    auto attach = [&](std::vector<uint8_t> &obj, size_t off, void *d) {
        std::memcpy(obj.data() + off, &d, sizeof(d));
    };

    // With a device wrapped: each method reaches its own slot, passing the
    // device as this, and each querying one returns the device's answer.
    attach(wd, 0x14, dev);
    attach(md, 0x14, dev);
    attach(sd, 0x3C, dev);
    g_fwd_ret = 0x4321;

    struct VoidCase { void (Wave_Device::*fn)(); int slot; };
    const VoidCase wd_void[] = {{&Wave_Device::suspend, 0x48},
                                {&Wave_Device::restart, 0x4C},
                                {&Wave_Device::update_sound, 0x34}};
    for (const auto &c : wd_void) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1; g_fwd_self = nullptr;
        (wave_dev->*c.fn)();
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    struct QueryCase { int (Wave_Device::*fn)(); int slot; };
    const QueryCase wd_query[] = {{&Wave_Device::get_ndevices, 0x14},
                                  {&Wave_Device::get_hw_mem_size, 0x58},
                                  {&Wave_Device::get_rate, 0x3C},
                                  {&Wave_Device::get_ds, 0x70},
                                  {&Wave_Device::is_eax, 0x74}};
    for (const auto &c : wd_query) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1;
        expect((wave_dev->*c.fn)() == 0x4321);
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    midi->enable();
    expect(g_fwd_calls == 1 && g_fwd_slot_ran == 0x54 && g_fwd_self == dev);
    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    midi->disable();
    expect(g_fwd_calls == 1 && g_fwd_slot_ran == 0x58 && g_fwd_self == dev);

    const struct { int (Sound::*fn)(); int slot; } sound_query[] = {
        {&Sound::is_playing, 0x5C}, {&Sound::is_looping, 0x58},
        {&Sound::get_time, 0x74}};
    for (const auto &c : sound_query) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1;
        expect((sound->*c.fn)() == 0x4321);
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    // With no device wrapped: nothing is dispatched and every query answers 0.
    attach(wd, 0x14, nullptr);
    attach(md, 0x14, nullptr);
    attach(sd, 0x3C, nullptr);
    g_fwd_calls = 0;
    for (const auto &c : wd_void) { (wave_dev->*c.fn)(); }
    for (const auto &c : wd_query) { expect((wave_dev->*c.fn)() == 0); }
    midi->enable();
    midi->disable();
    for (const auto &c : sound_query) { expect((sound->*c.fn)() == 0); }
    expect(g_fwd_calls == 0);

    // The redirects reach the same slots.
    attach(wd, 0x14, dev);
    attach(md, 0x14, dev);
    attach(sd, 0x3C, dev);
    g_fwd_ret = 9;
    g_fwd_slot_ran = -1;
    wave_device_suspend_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x48);
    wave_device_restart_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x4C);
    wave_device_update_sound_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x34);
    expect(wave_device_get_ndevices_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x14);
    expect(wave_device_get_hw_mem_size_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x58);
    expect(wave_device_get_rate_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x3C);
    expect(wave_device_get_ds_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x70);
    expect(wave_device_is_eax_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x74);
    midi_device_enable_redirect(midi, nullptr);
    expect(g_fwd_slot_ran == 0x54);
    midi_device_disable_redirect(midi, nullptr);
    expect(g_fwd_slot_ran == 0x58);
    expect(sound_is_playing_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x5C);
    expect(sound_is_looping_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x58);
    expect(sound_get_time_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x74);

    // Wave::get_ms_length is a plain read of the field at 0x60, not a forward.
    std::vector<uint8_t> wv(sizeof(Wave) + 32, 0);
    auto *wave_obj = reinterpret_cast<Wave *>(wv.data());
    int32_t length = 12345;
    std::memcpy(wv.data() + 0x60, &length, sizeof(length));
    int32_t decoy = -1;
    std::memcpy(wv.data() + 0x5C, &decoy, sizeof(decoy));   // neighbour differs
    std::memcpy(wv.data() + 0x64, &decoy, sizeof(decoy));
    expect(wave_obj->get_ms_length() == 12345);
    expect(wave_get_ms_length_redirect(wave_obj, nullptr) == 12345);
}

namespace {
// The one-argument members of the family need the argument recorded too.
int g_fwd_arg;
template <int Slot>
int __thiscall observe_fwd_arg_slot(void *self, int a1) {
    g_fwd_slot_ran = Slot; g_fwd_self = self; g_fwd_arg = a1;
    ++g_fwd_calls; return g_fwd_ret;
}
}  // namespace

void test_wrapped_device_forwarders_with_defaults() {
    // The rest of the family: the same wrapped-device dispatch, but with a
    // no-device answer that is not zero, an argument passed through, or a field
    // written on the way. Each method's slot, its argument, and its no-device
    // answer are all checked, because those are the three things that separate
    // these from one another.
    // Two vtables, because the wave-device side and the Sound side disagree
    // about the arity of the methods at slots 0x20 and 0x38 - one takes an
    // argument, the other does not. In the game these are different device
    // types, and calling a one-argument __thiscall with none would unbalance
    // the stack, so the test keeps them apart the same way.
    void *vt[64] = {};
    vt[0x20 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x20>);
    vt[0x38 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x38>);
    vt[0x54 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x54>);
    vt[0x58 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x58>);
    vt[0x5C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x5C>);
    vt[0x68 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x68>);
    vt[0x6C / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x6C>);
    vt[0x84 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x84>);
    struct FwdDev2 { void *vtbl; } dev_obj;
    dev_obj.vtbl = vt;
    void *dev = &dev_obj;

    void *svt[64] = {};
    svt[0x18 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x18>);
    svt[0x1C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x1C>);
    svt[0x20 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x20>);
    svt[0x38 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x38>);
    svt[0x48 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x48>);
    svt[0x4C / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x4C>);
    struct SoundDev { void *vtbl; } sound_dev_obj;
    sound_dev_obj.vtbl = svt;
    void *sound_dev = &sound_dev_obj;

    std::vector<uint8_t> wd(sizeof(Wave_Device) + 32, 0);
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd.data());
    std::vector<uint8_t> md(sizeof(Midi_Device) + 32, 0);
    auto *midi = reinterpret_cast<Midi_Device *>(md.data());
    std::vector<uint8_t> wid(sizeof(Wave_In_Device) + 32, 0);
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(wid.data());
    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());
    auto attach = [&](std::vector<uint8_t> &o, size_t off, void *d) {
        std::memcpy(o.data() + off, &d, sizeof(d));
    };
    auto read32 = [&](std::vector<uint8_t> &o, size_t off) {
        int32_t v = 0; std::memcpy(&v, o.data() + off, 4); return v;
    };

    // With a device: every method reaches its own slot and returns the answer.
    attach(wd, 0x14, dev); attach(md, 0x14, dev);
    attach(wid, 0x14, dev); attach(sd, 0x3C, sound_dev);
    g_fwd_ret = 0x777;

    g_fwd_slot_ran = -1; expect(wave_dev->is_disabled() == 0x777);
    expect(g_fwd_slot_ran == 0x68);
    g_fwd_slot_ran = -1; expect(wave_dev->stop_raw_dump() == 0x777);
    expect(g_fwd_slot_ran == 0x54);
    g_fwd_slot_ran = -1; expect(wave_dev->is_3d() == 0x777);
    expect(g_fwd_slot_ran == 0x84);
    g_fwd_slot_ran = -1; expect(midi->is_disabled() == 0x777);
    expect(g_fwd_slot_ran == 0x5C);

    // The two recorders dispatch but always answer 0 - the original throws the
    // device's answer away.
    g_fwd_slot_ran = -1; expect(wave_in->start_record() == 0);
    expect(g_fwd_slot_ran == 0x58);
    g_fwd_slot_ran = -1; expect(wave_in->end_record() == 0);
    expect(g_fwd_slot_ran == 0x5C);

    // One-argument forwards pass the argument through.
    g_fwd_slot_ran = -1; g_fwd_arg = 0;
    wave_dev->set_rate(4410u);
    expect(g_fwd_slot_ran == 0x38 && g_fwd_arg == 4410);
    g_fwd_slot_ran = -1; wave_dev->set_volume(77u);
    expect(g_fwd_slot_ran == 0x20 && g_fwd_arg == 77);
    int hwnd_marker = 0;
    g_fwd_slot_ran = -1; expect(wave_dev->set_hwnd(&hwnd_marker) == 0x777);
    expect(g_fwd_slot_ran == 0x6C);
    expect(g_fwd_arg == static_cast<int>(reinterpret_cast<intptr_t>(&hwnd_marker)));

    g_fwd_slot_ran = -1; expect(sound->play() == 0x777);
    expect(g_fwd_slot_ran == 0x1C);
    g_fwd_slot_ran = -1; expect(sound->play(9u) == 0x777);
    expect(g_fwd_slot_ran == 0x18 && g_fwd_arg == 9);
    g_fwd_slot_ran = -1; expect(sound->stop() == 0x777);
    expect(g_fwd_slot_ran == 0x20);
    g_fwd_slot_ran = -1; expect(sound->release() == 0x777);
    expect(g_fwd_slot_ran == 0x38);

    // The two setters write their field and forward the same value.
    g_fwd_slot_ran = -1; sound->set_loop_state(0x31337);
    expect(g_fwd_slot_ran == 0x48 && g_fwd_arg == 0x31337);
    expect(read32(sd, 0x30) == 0x31337);
    g_fwd_slot_ran = -1; sound->set_delay(0x4242u);
    expect(g_fwd_slot_ran == 0x4C && g_fwd_arg == 0x4242);
    expect(read32(sd, 0x34) == 0x4242);

    // With no device: nothing dispatches, and each answers its own default -
    // 1 for the disabled queries, 3 for the raw-dump stop, 0x13 for set_hwnd,
    // 0x14 for the four Sound calls, 0 for the rest. The setters still write.
    attach(wd, 0x14, nullptr); attach(md, 0x14, nullptr);
    attach(wid, 0x14, nullptr); attach(sd, 0x3C, nullptr);
    g_fwd_calls = 0;
    expect(wave_dev->is_disabled() == 1);
    expect(midi->is_disabled() == 1);
    expect(wave_dev->stop_raw_dump() == 3);
    expect(wave_dev->is_3d() == 0);
    expect(wave_dev->set_hwnd(&hwnd_marker) == 0x13);
    expect(wave_in->start_record() == 0 && wave_in->end_record() == 0);
    expect(sound->play() == 0x14 && sound->play(1u) == 0x14);
    expect(sound->stop() == 0x14 && sound->release() == 0x14);
    wave_dev->set_rate(1u);
    wave_dev->set_volume(1u);
    sound->set_loop_state(55);
    sound->set_delay(66u);
    expect(read32(sd, 0x30) == 55 && read32(sd, 0x34) == 66);
    expect(g_fwd_calls == 0);

    // get_group_volume reads the object's own table - sixteen 24-byte records
    // from 0x28 - and answers zero past the end. No device is involved.
    for (unsigned i = 0; i < 16; ++i) {
        int32_t v = static_cast<int32_t>(0x100 + i);
        std::memcpy(wd.data() + 0x28 + i * 24, &v, sizeof(v));
    }
    for (unsigned i = 0; i < 16; ++i) {
        expect(wave_dev->get_group_volume(i) == static_cast<int>(0x100 + i));
    }
    // A sentinel sits exactly where a bound that was one too generous would
    // read, so an off-by-one cannot pass by finding zeroes there.
    int32_t past_end = 0x5EEDBEEF;
    std::memcpy(wd.data() + 0x28 + 16 * 24, &past_end, sizeof(past_end));
    expect(wave_dev->get_group_volume(16) == 0);
    expect(wave_dev->get_group_volume(0xFFFFFFFFu) == 0);

    // Redirects reach the same slots and carry the same answers.
    attach(wd, 0x14, dev); attach(md, 0x14, dev);
    attach(wid, 0x14, dev); attach(sd, 0x3C, sound_dev);
    g_fwd_ret = 5;
    g_fwd_slot_ran = -1;
    expect(wave_device_is_disabled_redirect(wave_dev, nullptr) == 5);
    expect(g_fwd_slot_ran == 0x68);
    expect(wave_device_stop_raw_dump_redirect(wave_dev, nullptr) == 5);
    expect(wave_device_is_3d_redirect(wave_dev, nullptr) == 5);
    expect(midi_device_is_disabled_redirect(midi, nullptr) == 5);
    expect(wave_in_device_start_record_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_end_record_redirect(wave_in, nullptr) == 0);
    g_fwd_arg = 0;
    wave_device_set_rate_redirect(wave_dev, nullptr, 22050u);
    expect(g_fwd_arg == 22050);
    wave_device_set_volume_redirect(wave_dev, nullptr, 12u);
    expect(g_fwd_arg == 12);
    expect(wave_device_set_hwnd_redirect(wave_dev, nullptr, &hwnd_marker) == 5);
    expect(wave_device_get_group_volume_redirect(wave_dev, nullptr, 3) == 0x103);
    expect(sound_play_redirect(sound, nullptr) == 5);
    expect(sound_play_arg_redirect(sound, nullptr, 4u) == 5);
    expect(g_fwd_arg == 4);
    expect(sound_stop_redirect(sound, nullptr) == 5);
    expect(sound_release_redirect(sound, nullptr) == 5);
    sound_set_loop_state_redirect(sound, nullptr, 71);
    expect(read32(sd, 0x30) == 71 && g_fwd_arg == 71);
    sound_set_delay_redirect(sound, nullptr, 72u);
    expect(read32(sd, 0x34) == 72 && g_fwd_arg == 72);
}

namespace {
int g_ramp_a[3], g_ramp_calls;
void __thiscall observe_ramp(void *self, int a1, int a2, int a3) {
    g_fwd_self = self; g_ramp_a[0]=a1; g_ramp_a[1]=a2; g_ramp_a[2]=a3;
    ++g_ramp_calls;
}
}  // namespace

void test_sound_guarded_forwarders() {
    // fade and fade_in carry two guards, not one: the field at 0x38 must be set
    // AND a device wrapped, and either being absent answers 0x13. ramp is a
    // plain three-argument forward. Each is checked for its own slot, both
    // guards independently, and ramp's argument order.
    void *svt[64] = {};
    svt[0x28 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x28>);
    svt[0x30 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x30>);
    svt[0x34 / 4] = reinterpret_cast<void *>(&observe_ramp);
    struct GDev { void *vtbl; } gdev;
    gdev.vtbl = svt;

    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(sd.data()+off,&v,4); };
    void *dev = &gdev;
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));

    // Both guards satisfied: each reaches its own slot and returns the answer.
    set32(0x38, 1);
    g_fwd_ret = 0x2468;
    g_fwd_slot_ran = -1; expect(sound->fade() == 0x2468);
    expect(g_fwd_slot_ran == 0x28 && g_fwd_self == dev);
    g_fwd_slot_ran = -1; expect(sound->fade_in() == 0x2468);
    expect(g_fwd_slot_ran == 0x30 && g_fwd_self == dev);

    // Gate field clear, device present: refused, nothing dispatched.
    set32(0x38, 0);
    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    expect(sound->fade() == 0x13);
    expect(sound->fade_in() == 0x13);
    expect(g_fwd_calls == 0);

    // Gate field set, device absent: also refused. Each guard alone suffices.
    set32(0x38, 1);
    void *none = nullptr;
    std::memcpy(sd.data() + 0x3C, &none, sizeof(none));
    g_fwd_calls = 0;
    expect(sound->fade() == 0x13);
    expect(sound->fade_in() == 0x13);
    expect(g_fwd_calls == 0);

    // ramp forwards all three arguments in order; no gate field involved.
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));
    set32(0x38, 0);                      // deliberately clear - ramp ignores it
    g_ramp_calls = 0;
    sound->ramp(11, 22, 33u);
    expect(g_ramp_calls == 1 && g_fwd_self == dev);
    expect(g_ramp_a[0] == 11 && g_ramp_a[1] == 22 && g_ramp_a[2] == 33);

    // No device: ramp does nothing.
    std::memcpy(sd.data() + 0x3C, &none, sizeof(none));
    g_ramp_calls = 0;
    sound->ramp(1, 2, 3u);
    expect(g_ramp_calls == 0);

    // Redirects.
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));
    set32(0x38, 1);
    g_fwd_ret = 6;
    g_fwd_slot_ran = -1;
    expect(sound_fade_query_redirect(sound, nullptr) == 6);
    expect(g_fwd_slot_ran == 0x28);
    expect(sound_fade_in_redirect(sound, nullptr) == 6);
    expect(g_fwd_slot_ran == 0x30);
    g_ramp_calls = 0;
    sound_ramp_redirect(sound, nullptr, 7, 8, 9u);
    expect(g_ramp_calls == 1 && g_ramp_a[0] == 7 && g_ramp_a[2] == 9);
}

namespace {
void *g_pp_close_self;
int g_pp_close_calls;
void __thiscall observe_pp_win_close(void *self) {
    g_pp_close_self = self; ++g_pp_close_calls;
}
}  // namespace

void test_console_editor_undo_and_prod_picker_close() {
    // editor_undo's whole body is load_undo(1) - the argument is the only thing
    // it can get wrong, and it must not touch the Console.
    alignas(Console) std::vector<uint8_t> cs(sizeof(Console) + 32);
    std::vector<uint8_t> cexp(cs.size());
    auto *console = reinterpret_cast<Console *>(cs.data() + 16);
    seed_storage(cs.data(), cexp.data(), cs.size());
    std::memcpy(cexp.data(), cs.data(), cs.size());
    g_load_undo_calls = 0; g_load_undo_arg = -1;
    console->editor_undo();
    expect(g_load_undo_calls == 1 && g_load_undo_arg == 1);
    console_editor_undo_redirect(console, nullptr);
    expect(g_load_undo_calls == 2 && g_load_undo_arg == 1);
    expect_storage_bytes(cs.data(), cexp.data(), cs.size());

    // ProdPicker::close clears the dword at 0xA14 and then runs the GraphicWin
    // close it inherits - which is source-owned, so it is driven for real and
    // observed at its one seam.
    auto *const saved_win = WinOriginalClose;
    WinOriginalClose = &observe_pp_win_close;
    auto *const saved_bufclose = BufferSubobjectClose;
    BufferSubobjectClose = nullptr;   // GraphicWin::close skips it when unset
    std::vector<uint8_t> pp(sizeof(ProdPicker) + 64, 0);
    auto *picker = reinterpret_cast<ProdPicker *>(pp.data());
    int32_t live = 0x1234;
    std::memcpy(pp.data() + 0xA14, &live, sizeof(live));
    // 0xA10 is not a valid witness - GraphicWin::close clears it by design.
    // 0xA18 sits past ProdPicker's own field and nothing should touch it.
    int32_t neighbour = 0x7777;
    std::memcpy(pp.data() + 0xA18, &neighbour, sizeof(neighbour));
    g_pp_close_calls = 0;
    picker->close();
    int32_t after = -1;
    std::memcpy(&after, pp.data() + 0xA14, sizeof(after));
    expect(after == 0);                       // the field is cleared
    int32_t after_n = 0;
    std::memcpy(&after_n, pp.data() + 0xA18, sizeof(after_n));
    expect(after_n == 0x7777);                // the next dword is untouched
    expect(g_pp_close_calls == 1);            // the inherited close ran
    expect(g_pp_close_self == reinterpret_cast<void *>(picker));

    std::memcpy(pp.data() + 0xA14, &live, sizeof(live));
    g_pp_close_calls = 0;
    prod_picker_close_redirect(picker, nullptr);
    std::memcpy(&after, pp.data() + 0xA14, sizeof(after));
    expect(after == 0 && g_pp_close_calls == 1);

    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_bufclose;
}

namespace {
void *g_pw_freed;
int g_pw_free_calls;
void *observe_pw_map_win_free(void *p) { g_pw_freed = p; ++g_pw_free_calls; return nullptr; }
void *g_pw_win_close_self;
int g_pw_win_close_calls;
void __thiscall observe_pw_win_close(void *self) {
    g_pw_win_close_self = self; ++g_pw_win_close_calls;
}
}  // namespace

void test_plan_win_close() {
    // PlanWin::close clears one field and then runs MapWin::close on the very
    // same pointer - a real base-class call, since PlanWin's constructor
    // builds a MapWin at offset 0. Both halves are source-owned, so they are
    // driven for real and watched at the two seams underneath.
    auto *const saved_free = MapWinFree;
    auto *const saved_win = WinOriginalClose;
    auto *const saved_bufclose = BufferSubobjectClose;
    MapWinFree = &observe_pw_map_win_free;
    WinOriginalClose = &observe_pw_win_close;
    BufferSubobjectClose = nullptr;   // GraphicWin::close skips it when unset

    std::vector<uint8_t> pw(sizeof(PlanWin) + 64, 0);
    uint8_t *const obj = pw.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };

    // MapWin::close reaches the virtual base through the vbtable, never
    // through its own member, so the object must carry *PlanWin's* table -
    // {0, 0x22050} - and the base it closes must land at 0x22050, not at the
    // 0x21A6C a standalone MapWin would use. That difference is the whole
    // reason this call is safe to make across the two layouts.
    const int32_t vbtable[2] = {0, 0x22050};
    set32(0, reinterpret_cast<uintptr_t>(&vbtable[0]));

    int owned_block = 0;
    const uint32_t owned = reinterpret_cast<uintptr_t>(&owned_block);
    set32(4, owned);                  // MapWin's owned pointer
    set32(0x21A68, 0x1234);           // the field close clears
    set32(0x21A64, 0x11111111);       // last dword of the inherited MapWin data
    set32(0x21A6C, 0x22222222);       // PlanWin's next field - blink's toggle
    set32(sizeof(PlanWin), 0x33333333);   // past the object entirely

    g_pw_free_calls = 0; g_pw_win_close_calls = 0;
    auto *plan = reinterpret_cast<PlanWin *>(obj);
    plan->close();

    expect(get32(0x21A68) == 0);            // the field is cleared
    expect(get32(0x21A64) == 0x11111111);   // neither neighbour moves
    expect(get32(0x21A6C) == 0x22222222);
    expect(get32(sizeof(PlanWin)) == 0x33333333);
    // The inherited MapWin close ran: it released the owned block and dropped
    // the pointer.
    expect(g_pw_free_calls == 1);
    expect(g_pw_freed == reinterpret_cast<void *>(&owned_block));
    expect(get32(4) == 0);
    // ...and reached the GraphicWin base through the vbtable offset, not
    // through MapWin's own.
    expect(g_pw_win_close_calls == 1);
    expect(g_pw_win_close_self == reinterpret_cast<void *>(obj + 0x22050));
    expect(get32(0x22050 + 0xA0C) == *GraphicWinFieldA0CDefault);

    // The redirect drives the identical path.
    set32(4, owned);
    set32(0x21A68, 0x1234);
    g_pw_free_calls = 0; g_pw_win_close_calls = 0;
    plan_win_close_redirect(plan, nullptr);
    expect(get32(0x21A68) == 0);
    expect(get32(4) == 0);
    expect(g_pw_free_calls == 1 && g_pw_win_close_calls == 1);
    expect(g_pw_win_close_self == reinterpret_cast<void *>(obj + 0x22050));

    // A closed-out window frees nothing the second time round.
    g_pw_free_calls = 0;
    plan->close();
    expect(g_pw_free_calls == 0);

    MapWinFree = saved_free;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_bufclose;
}

namespace {
void *g_pw_slot_self;
int g_pw_slot_calls;
void __thiscall observe_pw_blink_slot(void *self) {
    g_pw_slot_self = self; ++g_pw_slot_calls;
}

struct CopyCall {
    void *self; Buffer *src;
    int x, y, sx, sy, w, h;
};
CopyCall g_copy;
int g_copy_calls;
int g_copy_result;
int __thiscall observe_buffer_copy_full(void *self, Buffer *src, int x, int y,
                                        int sx, int sy, int w, int h) {
    g_copy = {self, src, x, y, sx, sy, w, h};
    ++g_copy_calls;
    return g_copy_result;
}
}  // namespace

void test_buffer_copy_overload() {
    // The five-argument copy exists only to hand the destination coordinates
    // over a second time as the source coordinates. Distinct values in every
    // position are what makes a shuffled argument visible.
    auto *const saved = BufferCopyFull;
    BufferCopyFull = &observe_buffer_copy_full;

    std::vector<uint8_t> bs(sizeof(Buffer), 0);
    std::vector<uint8_t> bexp(bs.size());
    auto *buf = reinterpret_cast<Buffer *>(bs.data());
    seed_storage(bs.data(), bexp.data(), bs.size());
    std::memcpy(bexp.data(), bs.data(), bs.size());

    auto other = reinterpret_cast<Buffer *>(0x11223344);
    g_copy_calls = 0; g_copy_result = 0x5A5A;
    expect(buf->copy(other, 11, 22, 33, 44) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.self == reinterpret_cast<void *>(buf));
    expect(g_copy.src == other);
    expect(g_copy.x == 11 && g_copy.y == 22);
    expect(g_copy.sx == 11 && g_copy.sy == 22);   // the coordinates repeat
    expect(g_copy.w == 33 && g_copy.h == 44);
    // Negative extents must survive; PlanWin::UNK1 relies on passing one.
    g_copy_calls = 0;
    expect(buffer_copy_redirect(buf, nullptr, other, -1, -2, -3, -4) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.x == -1 && g_copy.sx == -1);
    expect(g_copy.y == -2 && g_copy.sy == -2);
    expect(g_copy.w == -3 && g_copy.h == -4);
    // The overload is a pure shuffle - it must not touch the buffer.
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    // The rectangle overload derives the extents instead of taking them, and
    // the rectangle is half-open: right-left, not right-left+1. Sides are
    // chosen so a swapped corner or an inclusive span shows up as a distinct
    // number rather than coinciding.
    RECT rect = {100, 200, 130, 250};   // left, top, right, bottom
    g_copy_calls = 0;
    expect(buf->copy(other, &rect) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.self == reinterpret_cast<void *>(buf));
    expect(g_copy.src == other);
    expect(g_copy.x == 100 && g_copy.y == 200);
    expect(g_copy.sx == 100 && g_copy.sy == 200);   // the corner repeats
    expect(g_copy.w == 30 && g_copy.h == 50);       // half-open extents
    // The rectangle is read, never written.
    expect(rect.left == 100 && rect.top == 200);
    expect(rect.right == 130 && rect.bottom == 250);
    g_copy_calls = 0;
    expect(buffer_copy_rect_redirect(buf, nullptr, other, &rect) == 0x5A5A);
    expect(g_copy_calls == 1 && g_copy.w == 30 && g_copy.h == 50);
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    BufferCopyFull = saved;
}

void test_plan_win_blink_and_unk1() {
    std::vector<uint8_t> pw(sizeof(PlanWin) + 64, 0);
    uint8_t *const obj = pw.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    const int32_t vbtable[2] = {0, 0x22050};
    set32(0, reinterpret_cast<uintptr_t>(&vbtable[0]));
    auto *plan = reinterpret_cast<PlanWin *>(obj);

    // blink dispatches slot 0x30 of the virtual base's own vtable, so the
    // table is planted on the base and the neighbouring slots are poisoned:
    // an off-by-one slot would jump to a null and fault rather than pass.
    void *vt[16] = {};
    vt[0x30 / sizeof(void *)] = reinterpret_cast<void *>(&observe_pw_blink_slot);
    set32(0x22050, reinterpret_cast<uintptr_t>(&vt[0]));

    // Blinking off: no toggle, no redraw.
    set32(0x21A68, 0);
    set32(0x21A6C, 7);
    g_pw_slot_calls = 0;
    plan->blink();
    expect(g_pw_slot_calls == 0);
    expect(get32(0x21A6C) == 7);

    // On, phase 0 -> 1, and the redraw runs against the virtual base.
    set32(0x21A68, 1);
    set32(0x21A6C, 0);
    plan->blink();
    expect(get32(0x21A6C) == 1);
    expect(g_pw_slot_calls == 1);
    expect(g_pw_slot_self == reinterpret_cast<void *>(obj + 0x22050));

    // 1 -> 0, and any other non-zero phase also lands on 0 rather than
    // flipping a bit or decrementing.
    plan->blink();
    expect(get32(0x21A6C) == 0);
    set32(0x21A6C, 5);
    plan_win_blink_redirect(plan, nullptr);
    expect(get32(0x21A6C) == 0);
    expect(g_pw_slot_calls == 3);
    expect(get32(0x21A68) == 1);   // the enable flag is never written

    // UNK1 blits the window's own buffer from PlanWin's, at the size the
    // window buffer records - with the height negated.
    auto *const saved_copy = BufferCopyFull;
    BufferCopyFull = &observe_buffer_copy_full;
    set32(0x22050 + 0x4C4, 640);
    set32(0x22050 + 0x4C8, 480);
    g_copy_calls = 0; g_copy_result = 0;
    plan->UNK1();
    expect(g_copy_calls == 1);
    // Destination is the *window's* buffer subobject, source is PlanWin's own.
    expect(g_copy.self == reinterpret_cast<void *>(obj + 0x22050 + 0x444));
    expect(g_copy.src == reinterpret_cast<Buffer *>(obj + 0x21A70));
    expect(g_copy.x == 0 && g_copy.y == 0);
    expect(g_copy.w == 640);
    expect(g_copy.h == -480);      // negated, as the original's `neg` does
    g_copy_calls = 0;
    plan_win_unk1_redirect(plan, nullptr);
    expect(g_copy_calls == 1 && g_copy.w == 640 && g_copy.h == -480);
    BufferCopyFull = saved_copy;
}

namespace {
// The redraw handler can retarget the button, which is how the test pins that
// set() re-reads the id and the parent link rather than caching them.
uint8_t *g_bb_obj;
int g_bb_refresh_calls;
uint32_t g_bb_retarget_id;
void *g_bb_retarget_parent;
bool g_bb_retarget;
void __thiscall observe_bb_refresh(void *self) {
    ++g_bb_refresh_calls;
    g_bb_obj = reinterpret_cast<uint8_t *>(self);
    if (g_bb_retarget) {
        std::memcpy(g_bb_obj + 0xA78, &g_bb_retarget_id, 4);
        std::memcpy(g_bb_obj + 0xC4, &g_bb_retarget_parent, 4);
    }
}
void *g_bb_notify_self;
int g_bb_notify_calls;
int g_bb_notify_id;
int g_bb_notify_value;
void __thiscall observe_bb_notify(void *self, int id, int value) {
    g_bb_notify_self = self; g_bb_notify_id = id; g_bb_notify_value = value;
    ++g_bb_notify_calls;
}
}  // namespace

void test_base_button_set() {
    // set() stores the new value, dispatches slot 0xF8 of its own live vtable,
    // then tells its parent through slot 0xB4 of the *parent's* vtable. Both
    // tables are planted with their neighbours left null so an off-by-one slot
    // faults rather than quietly passing.
    std::vector<uint8_t> bb(sizeof(BaseButton) + 64, 0);
    uint8_t *const obj = bb.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto *button = reinterpret_cast<BaseButton *>(obj);

    void *own_vt[64] = {};
    own_vt[0xF8 / sizeof(void *)] = reinterpret_cast<void *>(&observe_bb_refresh);
    set32(0, reinterpret_cast<uintptr_t>(&own_vt[0]));

    void *parent_vt[64] = {};
    parent_vt[0xB4 / sizeof(void *)] = reinterpret_cast<void *>(&observe_bb_notify);
    std::vector<uint8_t> parent(64, 0);
    const uintptr_t pvt = reinterpret_cast<uintptr_t>(&parent_vt[0]);
    std::memcpy(parent.data(), &pvt, 4);

    g_bb_retarget = false;

    // An unchanged value is inert: no store, no redraw, no notification.
    set32(0xA18, 42);
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    button->set(42);
    expect(g_bb_refresh_calls == 0 && g_bb_notify_calls == 0);
    expect(get32(0xA18) == 42);

    // A changed value stores, redraws, and notifies with (id, value).
    button->set(99);
    expect(get32(0xA18) == 99);
    expect(g_bb_refresh_calls == 1);
    expect(g_bb_obj == obj);                       // dispatched on the button
    expect(g_bb_notify_calls == 1);
    expect(g_bb_notify_self == reinterpret_cast<void *>(parent.data()));
    expect(g_bb_notify_id == 7);                   // the id, not the value
    expect(g_bb_notify_value == 99);

    // A parentless button still stores and redraws, but tells nobody.
    set32(0xC4, 0);
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    button->set(123);
    expect(get32(0xA18) == 123);
    expect(g_bb_refresh_calls == 1 && g_bb_notify_calls == 0);

    // Ordering: the redraw handler retargets the button, and the notification
    // must use the *new* id and the *new* parent - proving neither was read
    // before the dispatch.
    std::vector<uint8_t> parent2(64, 0);
    std::memcpy(parent2.data(), &pvt, 4);
    g_bb_retarget = true;
    g_bb_retarget_id = 555;
    g_bb_retarget_parent = parent2.data();
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_notify_calls = 0;
    button->set(1000);
    expect(g_bb_notify_calls == 1);
    expect(g_bb_notify_id == 555);                 // re-read after the redraw
    expect(g_bb_notify_self == reinterpret_cast<void *>(parent2.data()));
    g_bb_retarget = false;

    // The redirect drives the identical path.
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    base_button_set_redirect(button, nullptr, 2000);
    expect(get32(0xA18) == 2000);
    expect(g_bb_refresh_calls == 1 && g_bb_notify_calls == 1);
    expect(g_bb_notify_id == 7 && g_bb_notify_value == 2000);
    // Nothing past the object was disturbed.
    expect(get32(sizeof(BaseButton)) == 0);
}

namespace {
void *g_tex_freed;
int g_tex_free_calls;
void *observe_texture_free(void *p) { g_tex_freed = p; ++g_tex_free_calls; return nullptr; }
}  // namespace

void test_texture_lifecycle() {
    // The constructor and close agree on four fields - pixels at 0, two
    // descriptors at 4 and 8, a borrowed flag at 0x6C - and touch nothing
    // else. close frees the pixels only when there are pixels AND the flag is
    // clear; a borrowed texture keeps its block and even keeps its pointer,
    // which is the asymmetry worth pinning.
    auto *const saved = TextureFree;
    TextureFree = &observe_texture_free;
    g_tex_free_calls = 0;

    std::vector<uint8_t> ts(sizeof(Texture) + 32);
    std::vector<uint8_t> texp(ts.size());
    uint8_t *const obj = ts.data();
    auto read32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // Construction zeroes exactly those four and leaves the rest alone.
    seed_storage(ts.data(), texp.data(), ts.size());
    std::memcpy(texp.data(), ts.data(), ts.size());
    // Construction is driven through the redirect rather than a placement new
    // here: the constructor lives in another translation unit, so the compiler
    // cannot fold its stores against the memcpy writes this test makes into
    // the same storage afterwards.
    auto *tex = reinterpret_cast<Texture *>(obj);
    expect(texture_ctor_redirect(tex, nullptr) == tex);
    expect(read32(0) == 0 && read32(4) == 0 && read32(8) == 0);
    expect(read32(0x6C) == 0);
    // Everything between 0xC and 0x6B is untouched by construction.
    for (size_t off = 0xC; off < 0x6C; ++off) {
        expect(obj[off] == texp[off]);
    }

    // Owned pixels: freed, and the pointer cleared.
    int block = 0;
    void *pixels = &block;
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(4, 0x1111); set32(8, 0x2222); set32(0x6C, 0);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 1 && g_tex_freed == pixels);
    expect(read32(0) == 0 && read32(4) == 0 && read32(8) == 0 && read32(0x6C) == 0);

    // Borrowed pixels: not freed, and the pointer is deliberately left set.
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(4, 0x1111); set32(8, 0x2222); set32(0x6C, 1);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 0);
    void *still = nullptr;
    std::memcpy(&still, obj, sizeof(still));
    expect(still == pixels);                 // kept, not cleared
    expect(read32(4) == 0 && read32(8) == 0 && read32(0x6C) == 0);

    // No pixels at all: nothing freed, and the descriptors still clear.
    void *none = nullptr;
    std::memcpy(obj, &none, sizeof(none));
    set32(4, 0x3333); set32(0x6C, 0);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 0 && read32(4) == 0);

    // The constructor answers `this`, as MSVC constructors do.
    std::memcpy(obj, &pixels, sizeof(pixels));
    expect(texture_ctor_redirect(tex, nullptr) == tex);
    expect(read32(0) == 0);
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(0x6C, 0);
    g_tex_free_calls = 0;
    texture_close_redirect(tex, nullptr);
    expect(g_tex_free_calls == 1);

    TextureFree = saved;

    // TextureStore's destructor leaves 3 at 0 and clears 4, calling nothing.
    std::vector<uint8_t> st(sizeof(TextureStore) + 32);
    std::vector<uint8_t> sexp(st.size());
    seed_storage(st.data(), sexp.data(), st.size());
    std::memcpy(sexp.data(), st.data(), st.size());
    auto *store = reinterpret_cast<TextureStore *>(st.data());
    store->~TextureStore();
    uint32_t f0 = 0, f4 = 0;
    std::memcpy(&f0, st.data(), 4);
    std::memcpy(&f4, st.data() + 4, 4);
    expect(f0 == 3 && f4 == 0);
    // Nothing past the two fields moved.
    for (size_t off = 8; off < st.size(); ++off) {
        expect(st[off] == sexp[off]);
    }
    texture_store_dtor_redirect(store, nullptr);
    std::memcpy(&f0, st.data(), 4);
    expect(f0 == 3);
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

namespace {
void *g_datalink_exec_self = nullptr;
unsigned int g_datalink_exec_topic = 0;
int g_datalink_exec_index = 0;
int g_datalink_exec_calls = 0;
void __thiscall observe_datalink_exec(void *self, unsigned int topic,
                                      int index) {
    g_datalink_exec_self = self;
    g_datalink_exec_topic = topic;
    g_datalink_exec_index = index;
    ++g_datalink_exec_calls;
}
}  // namespace

void test_datalink_help_forwarders() {
    // Every help_* forwarder is a thin dispatch to Datalink::exec against the
    // fixed-address Datalink singleton: `DatalinkExec(DatalinkMain, topic,
    // index)`. Rebinding both seams and asserting the observed `self` equals
    // the REBOUND DatalinkMain (not the real singleton address) proves each
    // forwarder reads the seam rather than the original 0x00703EA0 literal.
    auto *const saved_exec = DatalinkExec;
    void *const saved_main = DatalinkMain;
    int fake_datalink = 0;
    DatalinkExec = &observe_datalink_exec;
    DatalinkMain = &fake_datalink;

    // help_tech -> topic 0xE
    g_datalink_exec_calls = 0;
    help_tech(101);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xE);
    expect(g_datalink_exec_index == 101);

    // help_weapon -> topic 0x6
    g_datalink_exec_calls = 0;
    help_weapon(102);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x6);
    expect(g_datalink_exec_index == 102);

    // help_armor -> topic 0x7
    g_datalink_exec_calls = 0;
    help_armor(103);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x7);
    expect(g_datalink_exec_index == 103);

    // help_chassis -> topic 0x4
    g_datalink_exec_calls = 0;
    help_chassis(104);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x4);
    expect(g_datalink_exec_index == 104);

    // help_facility -> topic 0xA
    g_datalink_exec_calls = 0;
    help_facility(105);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xA);
    expect(g_datalink_exec_index == 105);

    // help_abil -> topic 0x8
    g_datalink_exec_calls = 0;
    help_abil(106);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x8);
    expect(g_datalink_exec_index == 106);

    // help_social -> topic 0xC
    g_datalink_exec_calls = 0;
    help_social(107);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xC);
    expect(g_datalink_exec_index == 107);

    // help_faction -> topic 0xF
    g_datalink_exec_calls = 0;
    help_faction(108);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xF);
    expect(g_datalink_exec_index == 108);

    // help_veh -> topic 0x3
    g_datalink_exec_calls = 0;
    help_veh(109);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x3);
    expect(g_datalink_exec_index == 109);

    // help_topic passes its own topic argument straight through, rather than
    // a literal - so this call uses a topic value none of the fixed helpers
    // above use, to make sure it is not silently constant.
    g_datalink_exec_calls = 0;
    help_topic(0x1234, 110);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x1234);
    expect(g_datalink_exec_index == 110);

    DatalinkExec = saved_exec;
    DatalinkMain = saved_main;
}

namespace {
void *g_synch_self = nullptr;
int16_t g_synch_opcode = 0;
int g_synch_a = 0;
int g_synch_b = 0;
int g_synch_c = 0;
char *g_synch_text = nullptr;
int g_synch_d = 0;
int16_t g_synch_flags = 0;
int g_synch_calls = 0;
void __thiscall observe_synch(void *daemon, int16_t opcode, int a, int b,
                              int c, char *text, int d, int16_t flags) {
    g_synch_self = daemon;
    g_synch_opcode = opcode;
    g_synch_a = a;
    g_synch_b = b;
    g_synch_c = c;
    g_synch_text = text;
    g_synch_d = d;
    g_synch_flags = flags;
    ++g_synch_calls;
}
}  // namespace

void test_net_daemon_synch_forwarders() {
    // Fourteen forwarders, all one shape: push the four shared literals
    // (0, 0, nullptr, 1, 0x2101) plus one caller id and a fixed opcode,
    // then call NetDaemon::synch on the daemon at 0x0093CD90 - the same
    // receiver NetDaemonNet already binds for the receive path. synch_diplo
    // is the one exception: both leading arguments come from its caller.
    auto *const saved_synch = NetDaemonSynch;
    void *const saved_net = NetDaemonNet;
    int fake_net = 0;
    NetDaemonSynch = &observe_synch;
    NetDaemonNet = &fake_net;

    auto check_id_forward = [&](void (*fn)(int), int16_t opcode, int id) {
        g_synch_calls = 0;
        fn(id);
        expect(g_synch_calls == 1);
        expect(g_synch_self == &fake_net);   // rebound NetDaemonNet, not NetDaemonSynch
        expect(g_synch_opcode == opcode);
        expect(g_synch_a == id);
        expect(g_synch_b == 0);
        expect(g_synch_c == 0);
        expect(g_synch_text == nullptr);
        expect(g_synch_d == 1);
        expect(g_synch_flags == 0x2101);
    };

    check_id_forward(&synch_veh, 0x11, 1001);
    check_id_forward(&synch_base, 0x13, 1002);
    check_id_forward(&synch_energy, 0xB, 1003);
    check_id_forward(&synch_researching, 0xA, 1004);
    check_id_forward(&synch_leader, 0x6, 1005);
    check_id_forward(&synch_ai, 0x8, 1006);
    check_id_forward(&synch_research, 0x9, 1007);
    check_id_forward(&synch_alloc, 0xC, 1008);
    check_id_forward(&synch_soc, 0xD, 1009);
    check_id_forward(&synch_proto, 0xE, 1010);
    check_id_forward(&synch_obs, 0x10, 1011);
    check_id_forward(&synch_template, 0x18, 1012);
    check_id_forward(&synch_radius, 0x23, 1013);

    // synch_diplo takes two caller arguments; two distinct sentinels pin
    // that a and b are not swapped en route to NetDaemon::synch.
    g_synch_calls = 0;
    synch_diplo(2001, 2002);
    expect(g_synch_calls == 1);
    expect(g_synch_self == &fake_net);
    expect(g_synch_opcode == 0x16);
    expect(g_synch_a == 2001);
    expect(g_synch_b == 2002);
    expect(g_synch_c == 0);
    expect(g_synch_text == nullptr);
    expect(g_synch_d == 1);
    expect(g_synch_flags == 0x2101);

    NetDaemonSynch = saved_synch;
    NetDaemonNet = saved_net;
}




namespace {
// Six-argument family (target: XPopOriginalFull). Field order mirrors
// func_x_pop_full: caption, label, value, text, flags, callback.
struct XPopCall {
    char *caption; const char *label; int value; char *text; int flags;
    int (__cdecl *callback)();
    int calls;
} g_xpop6 = {};
int __cdecl observe_xpop6(char *caption, const char *label, int value,
                          char *text, int flags, int (__cdecl *callback)()) {
    g_xpop6 = {caption, label, value, text, flags, callback, g_xpop6.calls + 1};
    return 0x5AA7;
}
int __cdecl xpop6_callback() { return 0; }

// Nine-argument family (target: XPopsOriginalFull). Field order mirrors
// func_x_pops_full: caption, label, title, text, value, sprite, flag1,
// flag2, callback.
struct XPopsCall9 {
    char *caption; const char *label; int title; char *text; int value;
    Sprite *sprite; int flag1; int flag2;
    int (__cdecl *callback)();
    int calls;
} g_xpops9 = {};
int __cdecl observe_xpops9(char *caption, const char *label, int title,
                           char *text, int value, Sprite *sprite, int flag1,
                           int flag2, int (__cdecl *callback)()) {
    g_xpops9 = {caption, label, title, text, value, sprite, flag1, flag2,
               callback, g_xpops9.calls + 1};
    return 0x5AA7;
}
int __cdecl xpops9_callback() { return 0; }
}  // namespace

void test_x_pop_forwarders() {
    // XPopOriginalFull(caption, label, value, text, flags, callback) is the
    // six-argument sibling of the nine-argument popup builder (no sprite, no
    // trailing flag pair). Each of the six forwarders below defaults a
    // different subset of these six arguments; every literal and every
    // passthrough position is asserted, with distinct sentinels per
    // parameter, so a swapped argument cannot pass unnoticed.
    auto *const saved_full = XPopOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopOriginalFull = &observe_xpop6;
    XPopsCaptionBuffer = buffer;

    char caption[] = "caption";
    const char label[] = "label";
    char text[] = "text";
    const int kValue = 0x2002;
    const int kFlags = 0x3003;

    // x_pop_caption_label: caller's own caption/label/callback; value -1,
    // text nullptr and flags 0 all default.
    g_xpop6.calls = 0;
    expect(x_pop_caption_label(caption, label, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == 0);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_value_flags: shared (rebound) buffer, caller's value and flags,
    // no override text.
    g_xpop6.calls = 0;
    expect(x_pop_value_flags(label, kValue, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);  // proves the rebound seam is read
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_caption_value_flags: caller's caption/label/value/flags, no
    // override text.
    g_xpop6.calls = 0;
    expect(x_pop_caption_value_flags(caption, label, kValue, kFlags,
                                     &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_short: shared (rebound) buffer, value -1, no override text,
    // caller's flags.
    g_xpop6.calls = 0;
    expect(x_pop_short(label, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_caption_flags: caller's caption/label/flags; value -1, no
    // override text.
    g_xpop6.calls = 0;
    expect(x_pop_caption_flags(caption, label, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_default_caption: shared (rebound) buffer; caller's label, value,
    // override text and flags.
    g_xpop6.calls = 0;
    expect(x_pop_default_caption(label, kValue, text, kFlags,
                                 &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == text);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    XPopOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;
}

void test_x_pops_forwarders() {
    // XPopsOriginalFull(caption, label, title, text, value, sprite, flag1,
    // flag2, callback) is the nine-argument popup builder already exercised
    // by x_pops_short/x_pops_minimal. Each of the thirteen forwarders below
    // defaults a different subset of these nine arguments; every literal and
    // every passthrough position is asserted, with distinct sentinels per
    // parameter, so a swapped argument cannot pass unnoticed.
    auto *const saved_full = XPopsOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopsOriginalFull = &observe_xpops9;
    XPopsCaptionBuffer = buffer;

    char caption[] = "caption";
    const char label[] = "label";
    char text[] = "text";
    Sprite sprite_value;

    const int kTitle = 0x1001;
    const int kValue = 0x2002;
    const int kFlag1 = 0x4004;
    const int kFlag2 = 0x5005;

    // x_pops_flags: shared (rebound) buffer, title -1, no override text,
    // value 0; caller's sprite and both flags (unlike short/minimal, the
    // flags are not fixed at 1 here).
    g_xpops9.calls = 0;
    expect(x_pops_flags(label, &sprite_value, kFlag1, kFlag2,
                        &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);  // proves the rebound seam is read
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_minimal: caller's caption/label/sprite/callback; title
    // -1, no override text, value 0, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_minimal(caption, label, &sprite_value,
                                  &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_flags: caller's caption/label/sprite/both flags; title
    // -1, no override text, value 0.
    g_xpops9.calls = 0;
    expect(x_pops_caption_flags(caption, label, &sprite_value, kFlag1, kFlag2,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_title: shared (rebound) buffer, caller's title/value/sprite; no
    // override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_title(label, kTitle, kValue, &sprite_value,
                        &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_title_flags: shared (rebound) buffer, caller's title/value/
    // sprite/both flags; no override text.
    g_xpops9.calls = 0;
    expect(x_pops_title_flags(label, kTitle, kValue, &sprite_value, kFlag1,
                              kFlag2, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_title: caller's caption/label/title/value/sprite; no
    // override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_title(caption, label, kTitle, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_no_text: everything but the override text caller-supplied -
    // caption, label, title, value, sprite and both flags.
    g_xpops9.calls = 0;
    expect(x_pops_no_text(caption, label, kTitle, kValue, &sprite_value,
                          kFlag1, kFlag2, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_value_flags: shared (rebound) buffer, title -1, no override
    // text; caller's value/sprite/both flags.
    g_xpops9.calls = 0;
    expect(x_pops_value_flags(label, kValue, &sprite_value, kFlag1, kFlag2,
                              &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_value: caller's caption/label/value/sprite; title -1,
    // no override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_value(caption, label, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_value_flags: caller's caption/label/value/sprite/both
    // flags; title -1, no override text.
    g_xpops9.calls = 0;
    expect(x_pops_caption_value_flags(caption, label, kValue, &sprite_value,
                                      kFlag1, kFlag2,
                                      &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_override_text: shared (rebound) buffer, caller's title/override
    // text/value/sprite; both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_override_text(label, kTitle, text, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_default_caption: shared (rebound) buffer; everything else -
    // label, title, override text, value, sprite and both flags -
    // caller-supplied.
    g_xpops9.calls = 0;
    expect(x_pops_default_caption(label, kTitle, text, kValue, &sprite_value,
                                  kFlag1, kFlag2,
                                  &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_no_flags: everything but the flags caller-supplied; both flags
    // fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_no_flags(caption, label, kTitle, text, kValue,
                           &sprite_value, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    XPopsOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;
}

namespace {
void *g_caviar_freed;
int g_caviar_free_calls;
void __cdecl observe_caviar_free_record(void *record) {
    g_caviar_freed = record;
    ++g_caviar_free_calls;
}
}  // namespace

void test_caviar_data_close() {
    // close releases the record at 0x8 through the helper and forgets it; a
    // slot with no record is left entirely untouched, and the two leading
    // fields keep whatever they held either way.
    auto *const saved = CaviarDataFreeRecord;
    CaviarDataFreeRecord = &observe_caviar_free_record;

    alignas(4) uint8_t storage[sizeof(CaviarData) + 16];
    uint8_t expected[sizeof(storage)];
    auto *slot = reinterpret_cast<CaviarData *>(storage);
    int record = 0;

    // No record: nothing is called and not one byte moves.
    seed_storage(storage, expected, sizeof(storage));
    std::memset(storage + 8, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_caviar_free_calls = 0;
    slot->close();
    expect(g_caviar_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A record: the helper sees exactly that pointer, the slot forgets it,
    // and everything else survives byte for byte.
    seed_storage(storage, expected, sizeof(storage));
    void *pointer = &record;
    std::memcpy(storage + 8, &pointer, 4);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 8, 0, 4);
    g_caviar_free_calls = 0;
    g_caviar_freed = nullptr;
    slot->close();
    expect(g_caviar_free_calls == 1);
    expect(g_caviar_freed == &record);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Redirect entry.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 8, &pointer, 4);
    g_caviar_free_calls = 0;
    caviar_data_close_redirect(slot, nullptr);
    expect(g_caviar_free_calls == 1);

    CaviarDataFreeRecord = saved;
}

void test_texture_dtor() {
    // The destructor settles only the pixel block: freed and cleared when
    // there are pixels and they are ours, left alone otherwise. Unlike close
    // it resets nothing else - the descriptors and the borrowed flag keep
    // their values on every path.
    auto *const saved = TextureFree;
    TextureFree = &observe_texture_free;

    alignas(4) uint8_t storage[sizeof(Texture) + 16];
    uint8_t expected[sizeof(storage)];
    auto *texture = reinterpret_cast<Texture *>(storage);
    int pixels = 0;
    void *pointer = &pixels;

    // No pixels: nothing freed, nothing written.
    seed_storage(storage, expected, sizeof(storage));
    std::memset(storage + 0x00, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_tex_free_calls = 0;
    texture->~Texture();
    expect(g_tex_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Borrowed pixels: kept, and still nothing written.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    uint32_t borrowed = 1;
    std::memcpy(storage + 0x6C, &borrowed, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_tex_free_calls = 0;
    texture->~Texture();
    expect(g_tex_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Our pixels: freed and cleared; descriptors and flag untouched.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    std::memset(storage + 0x6C, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 0x00, 0, 4);
    g_tex_free_calls = 0;
    g_tex_freed = nullptr;
    texture->~Texture();
    expect(g_tex_free_calls == 1);
    expect(g_tex_freed == &pixels);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Redirect entry.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    std::memset(storage + 0x6C, 0, 4);
    g_tex_free_calls = 0;
    texture_dtor_redirect(texture, nullptr);
    expect(g_tex_free_calls == 1);

    TextureFree = saved;
}

namespace {
// Target: PopsOriginalFull. Field order mirrors func_pops_full: caption,
// label, value, text, title, sprite, flag_a, flag_b, callback. Note both
// caption and label are plain char* here (PAD in every mangled name), unlike
// the const char* label used by the XPopsOriginalFull family.
struct PopsCall {
    char *caption; char *label; int value; char *text; int title;
    Sprite *sprite; int flag_a; int flag_b;
    int (__cdecl *callback)();
    int calls;
} g_pops = {};

int __cdecl observe_pops(char *caption, char *label, int value, char *text,
                         int title, Sprite *sprite, int flag_a, int flag_b,
                         int (__cdecl *callback)()) {
    g_pops = {caption, label, value, text, title, sprite, flag_a, flag_b,
             callback, g_pops.calls + 1};
    return 0x7A11;
}
int __cdecl pops_test_callback() { return 0; }
}  // namespace

void test_pop_pops_forwarders() {
    // PopsOriginalFull(caption, label, value, text, title, sprite, flag_a,
    // flag_b, callback) is the nine-argument popup builder at 0x006276A0
    // that every pop_*/pops_* forwarder below reduces to. The pop_* family
    // (8 functions) always passes a null sprite; the pops_* family (15
    // functions) always forwards its own sprite argument. Every literal
    // (-1, nullptr, 0, 1) and every passthrough position is asserted, with
    // distinct sentinels per parameter, so a swapped argument cannot pass
    // unnoticed.
    auto *const saved_full = PopsOriginalFull;
    char *const saved_caption = PopupStartCaption;
    char buffer[8] = {};
    PopsOriginalFull = &observe_pops;
    PopupStartCaption = buffer;

    char caption[] = "caption";
    char label[] = "label";
    char text[] = "text";
    Sprite sprite_value;

    const int kValue = 0x2002;
    const int kTitle = 0x1001;
    const int kFlagA = 0x4004;
    const int kFlagB = 0x5005;

    // pop_label_cb (0x00627130): shared (rebound) buffer, label/callback
    // only; value -1, no override text, title 0, null sprite, both flags
    // fixed at 1.
    g_pops.calls = 0;
    expect(pop_label_cb(label, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);  // proves the rebound seam is read
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption (0x00627160): caller's own caption/label/callback; value
    // -1, no override text, title 0, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption(caption, label, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_value_title (0x006271D0): shared (rebound) buffer, caller's
    // value/title; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_value_title(label, kValue, kTitle,
                           &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption_value_title (0x00627200): caller's caption/label/value/
    // title; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption_value_title(caption, label, kValue, kTitle,
                                   &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_title (0x00627230): shared (rebound) buffer, value -1, caller's
    // title only; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_title(label, kTitle, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption_title (0x00627260): caller's caption/label/title; value
    // -1, no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption_title(caption, label, kTitle,
                             &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_value_text_title (0x00627290): shared (rebound) buffer, caller's
    // value/override text/title; null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_value_text_title(label, kValue, text, kTitle,
                                &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_full (0x006272C0): everything but the sprite and flags caller-
    // supplied - caption, value, override text, title; null sprite, both
    // flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_full(caption, label, kValue, text, kTitle,
                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_minimal (0x00627310): shared (rebound) buffer, caller's sprite
    // only; value -1, no override text, title 0, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_minimal(label, &sprite_value,
                        &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_flags (0x00627340): shared (rebound) buffer, caller's sprite and
    // both flags; value -1, no override text, title 0.
    g_pops.calls = 0;
    expect(pops_flags(label, &sprite_value, kFlagA, kFlagB,
                      &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption (0x00627370): caller's caption/label/sprite; value -1,
    // no override text, title 0, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption(caption, label, &sprite_value,
                        &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_flags (0x006273A0): caller's caption/label/sprite/both
    // flags; value -1, no override text, title 0.
    g_pops.calls = 0;
    expect(pops_caption_flags(caption, label, &sprite_value, kFlagA, kFlagB,
                              &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_title (0x006273D0): shared (rebound) buffer, caller's
    // value/title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_value_title(label, kValue, kTitle, &sprite_value,
                            &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_title_flags (0x00627400): shared (rebound) buffer,
    // caller's value/title/sprite/both flags; no override text.
    g_pops.calls = 0;
    expect(pops_value_title_flags(label, kValue, kTitle, &sprite_value,
                                  kFlagA, kFlagB,
                                  &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_value_title (0x006274D0): caller's caption/label/value/
    // title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption_value_title(caption, label, kValue, kTitle,
                                    &sprite_value,
                                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_no_text (0x00627500): everything but the override text caller-
    // supplied - caption, value, title, sprite and both flags.
    g_pops.calls = 0;
    expect(pops_no_text(caption, label, kValue, kTitle, &sprite_value,
                        kFlagA, kFlagB, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_title (0x00627540): shared (rebound) buffer, value -1, caller's
    // title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_title(label, kTitle, &sprite_value,
                      &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_title_flags (0x00627570): shared (rebound) buffer, value -1,
    // caller's title/sprite/both flags; no override text.
    g_pops.calls = 0;
    expect(pops_title_flags(label, kTitle, &sprite_value, kFlagA, kFlagB,
                            &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_title (0x006275A0): caller's caption/label/title/
    // sprite; value -1, no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption_title(caption, label, kTitle, &sprite_value,
                              &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_title_flags (0x006275D0): caller's caption/label/title/
    // sprite/both flags; value -1, no override text.
    g_pops.calls = 0;
    expect(pops_caption_title_flags(caption, label, kTitle, &sprite_value,
                                    kFlagA, kFlagB,
                                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_text_title (0x00627600): shared (rebound) buffer, caller's
    // value/override text/title/sprite; both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_value_text_title(label, kValue, text, kTitle, &sprite_value,
                                 &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_default_caption (0x00627630): shared (rebound) buffer; every
    // other argument - value, override text, title, sprite, both flags -
    // caller-supplied.
    g_pops.calls = 0;
    expect(pops_default_caption(label, kValue, text, kTitle, &sprite_value,
                                kFlagA, kFlagB,
                                &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_no_flags (0x00627670): everything but the flags caller-supplied
    // - caption, value, override text, title, sprite; both flags fixed at
    // 1.
    g_pops.calls = 0;
    expect(pops_no_flags(caption, label, kValue, text, kTitle, &sprite_value,
                         &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    PopsOriginalFull = saved_full;
    PopupStartCaption = saved_caption;
}

namespace {
struct AtexitThunkCase {
    void(__cdecl *thunk)();
    void *slot;   // the global seam to rebind
};
const AtexitThunkCase g_atexit_sprite_cases[] = {
    {&destroy_unused_sprite_var02, &g_UNUSED_SPRITE_VAR02},
    {&destroy_unused_sprite_var11, &g_UNUSED_SPRITE_VAR11},
    {&destroy_unused_sprite_var06, &g_UNUSED_SPRITE_VAR06},
    {&destroy_unused_sprite_var09, &g_UNUSED_SPRITE_VAR09},
    {&destroy_unused_sprite_var21, &g_UNUSED_SPRITE_VAR21},
    {&destroy_unused_sprite_var05, &g_UNUSED_SPRITE_VAR05},
    {&destroy_unused_sprite_var08, &g_UNUSED_SPRITE_VAR08},
    {&destroy_unused_sprite_var04, &g_UNUSED_SPRITE_VAR04},
    {&destroy_unused_sprite_var01, &g_UNUSED_SPRITE_VAR01},
    {&destroy_unused_sprite_var18, &g_UNUSED_SPRITE_VAR18},
    {&destroy_unused_sprite_var03, &g_UNUSED_SPRITE_VAR03},
    {&destroy_unused_sprite_var20, &g_UNUSED_SPRITE_VAR20},
    {&destroy_unused_sprite_var16, &g_UNUSED_SPRITE_VAR16},
    {&destroy_unused_sprite_var14, &g_UNUSED_SPRITE_VAR14},
    {&destroy_unused_sprite_var22, &g_UNUSED_SPRITE_VAR22},
    {&destroy_unused_sprite_var10, &g_UNUSED_SPRITE_VAR10},
    {&destroy_unused_sprite_var15, &g_UNUSED_SPRITE_VAR15},
    {&destroy_unused_sprite_var13, &g_UNUSED_SPRITE_VAR13},
    {&destroy_unused_sprite_var17, &g_UNUSED_SPRITE_VAR17},
    {&destroy_unused_sprite_var19, &g_UNUSED_SPRITE_VAR19},
    {&destroy_unused_sprite_var12, &g_UNUSED_SPRITE_VAR12},
    {&destroy_unused_sprite_var07, &g_UNUSED_SPRITE_VAR07},
    {&destroy_iface_std_popups_top_left_sprite, &g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE},
    {&destroy_iface_std_popups_top_right_sprite, &g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE},
    {&destroy_iface_std_popups_bot_left_sprite, &g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE},
    {&destroy_iface_std_popups_bot_right_sprite, &g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE},
    {&destroy_iface_std_popups_top_mid_sprite, &g_IFACE_STD_POPUPS_TOP_MID_SPRITE},
    {&destroy_iface_std_popups_bot_mid_sprite, &g_IFACE_STD_POPUPS_BOT_MID_SPRITE},
    {&destroy_iface_std_popups_mid_left_sprite, &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE},
    {&destroy_iface_std_popups_mid_right_sprite, &g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE},
    {&destroy_ter1_mine_sprite, &g_TER1_MINE_SPRITE},
    {&destroy_ter1_solar_collector_sprite, &g_TER1_SOLAR_COLLECTOR_SPRITE},
    {&destroy_ter1_tidal_harness_sprite, &g_TER1_TIDAL_HARNESS_SPRITE},
    {&destroy_ter1_mining_platform_sprite, &g_TER1_MINING_PLATFORM_SPRITE},
    {&destroy_ter1_tut_blank_sprite, &g_TER1_TUT_BLANK_SPRITE},
    {&destroy_ter1_kelp_farm_sprite, &g_TER1_KELP_FARM_SPRITE},
    {&destroy_ter1_condenser_sprite, &g_TER1_CONDENSER_SPRITE},
    {&destroy_ter1_echelon_mirror_sprite, &g_TER1_ECHELON_MIRROR_SPRITE},
    {&destroy_ter1_borehole_sprite, &g_TER1_BOREHOLE_SPRITE},
    {&destroy_ter1_borehole_cluster_sprite, &g_TER1_BOREHOLE_CLUSTER_SPRITE},
    {&destroy_ter1_monolith_sprite, &g_TER1_MONOLITH_SPRITE},
    {&destroy_ter1_bunker_sprite, &g_TER1_BUNKER_SPRITE},
    {&destroy_ter1_airbase_sprite, &g_TER1_AIRBASE_SPRITE},
    {&destroy_ter1_sensor_array_sprite, &g_TER1_SENSOR_ARRAY_SPRITE},
    {&destroy_red_alien_head_icon_sprite, &g_RED_ALIEN_HEAD_ICON_SPRITE},
    {&destroy_red_male_head_icon_sprite, &g_RED_MALE_HEAD_ICON_SPRITE},
    {&destroy_null_resource_icon_sprite, &g_NULL_RESOURCE_ICON_SPRITE},
    {&destroy_icon_tile_square_sprite, &g_ICON_TILE_SQUARE_SPRITE},
    {&destroy_battle_mind_worm_sprite, &g_BATTLE_MIND_WORM_SPRITE},
    {&destroy_battle_isle_deep_sprite, &g_BATTLE_ISLE_DEEP_SPRITE},
    {&destroy_battle_locusts_chiron_sprite, &g_BATTLE_LOCUSTS_CHIRON_SPRITE},
    {&destroy_battle_fungal_tower_sprite, &g_BATTLE_FUNGAL_TOWER_SPRITE},
    {&destroy_battle_spore_launcher_sprite, &g_BATTLE_SPORE_LAUNCHER_SPRITE},
    {&destroy_battle_sealurk_sprite, &g_BATTLE_SEALURK_SPRITE},
    {&destroy_scroll_bar_filler_icon_sprites, &g_SCROLL_BAR_FILLER_ICON_SPRITES},
    {&destroy_scroll_bar_small_filler_icon_sprite, &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE},
    {&destroy_iface_general_windows_top_left_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE},
    {&destroy_iface_general_windows_top_right_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE},
    {&destroy_iface_general_windows_bot_left_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE},
    {&destroy_iface_general_windows_bot_right_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE},
    {&destroy_iface_general_windows_mid_left_sprite, &g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE},
    {&destroy_iface_general_windows_mid_right_sprite, &g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE},
    {&destroy_iface_general_windows_top_mid_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE},
    {&destroy_iface_general_windows_bot_mid_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE},
    {&destroy_iface_general_windows_noncap_mid_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE},
    {&destroy_iface_general_windows_noncap_left_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE},
    {&destroy_iface_general_windows_noncap_right_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE},
    {&destroy_unused_sprite_var23, &g_UNUSED_SPRITE_VAR23},
    {&destroy_unused_sprite_var24, &g_UNUSED_SPRITE_VAR24},
    {&destroy_unused_sprite_var25, &g_UNUSED_SPRITE_VAR25},
    {&destroy_unused_sprite_var26, &g_UNUSED_SPRITE_VAR26},
    {&destroy_unused_sprite_var27, &g_UNUSED_SPRITE_VAR27},
    {&destroy_unused_sprite_var28, &g_UNUSED_SPRITE_VAR28},
    {&destroy_unused_sprite_var29, &g_UNUSED_SPRITE_VAR29},
    {&destroy_unused_sprite_var30, &g_UNUSED_SPRITE_VAR30},
    {&destroy_unused_sprite_var31, &g_UNUSED_SPRITE_VAR31},
    {&destroy_unused_sprite_var32, &g_UNUSED_SPRITE_VAR32},
    {&destroy_unused_sprite_var33, &g_UNUSED_SPRITE_VAR33},
    {&destroy_unused_sprite_var34, &g_UNUSED_SPRITE_VAR34},
    {&destroy_unused_sprite_var35, &g_UNUSED_SPRITE_VAR35},
    {&destroy_unused_sprite_var36, &g_UNUSED_SPRITE_VAR36},
    {&destroy_unused_sprite_var37, &g_UNUSED_SPRITE_VAR37},
    {&destroy_unused_sprite_var38, &g_UNUSED_SPRITE_VAR38},
    {&destroy_unused_sprite_var39, &g_UNUSED_SPRITE_VAR39},
    {&destroy_unused_sprite_var40, &g_UNUSED_SPRITE_VAR40},
    {&destroy_unused_sprite_var41, &g_UNUSED_SPRITE_VAR41},
    {&destroy_unused_sprite_var42, &g_UNUSED_SPRITE_VAR42},
    {&destroy_unused_sprite_var43, &g_UNUSED_SPRITE_VAR43},
    {&destroy_unused_sprite_var44, &g_UNUSED_SPRITE_VAR44},
    {&destroy_unused_sprite_var45, &g_UNUSED_SPRITE_VAR45},
    {&destroy_unused_sprite_var46, &g_UNUSED_SPRITE_VAR46},
    {&destroy_unused_sprite_var47, &g_UNUSED_SPRITE_VAR47},
    {&destroy_unused_sprite_var48, &g_UNUSED_SPRITE_VAR48},
    {&destroy_unused_sprite_var49, &g_UNUSED_SPRITE_VAR49},
    {&destroy_unused_sprite_var50, &g_UNUSED_SPRITE_VAR50},
    {&destroy_unused_sprite_var51, &g_UNUSED_SPRITE_VAR51},
    {&destroy_unused_sprite_var52, &g_UNUSED_SPRITE_VAR52},
    {&destroy_unused_sprite_var53, &g_UNUSED_SPRITE_VAR53},
    {&destroy_unused_sprite_var54, &g_UNUSED_SPRITE_VAR54},
    {&destroy_unused_sprite_var55, &g_UNUSED_SPRITE_VAR55},
    {&destroy_unused_sprite_var56, &g_UNUSED_SPRITE_VAR56},
    {&destroy_unused_sprite_var57, &g_UNUSED_SPRITE_VAR57},
    {&destroy_unused_sprite_var58, &g_UNUSED_SPRITE_VAR58},
    {&destroy_unused_sprite_var59, &g_UNUSED_SPRITE_VAR59},
    {&destroy_unused_sprite_var60, &g_UNUSED_SPRITE_VAR60},
    {&destroy_unused_sprite_var61, &g_UNUSED_SPRITE_VAR61},
    {&destroy_unused_sprite_var62, &g_UNUSED_SPRITE_VAR62},
    {&destroy_unused_sprite_var63, &g_UNUSED_SPRITE_VAR63},
    {&destroy_unused_sprite_var64, &g_UNUSED_SPRITE_VAR64},
    {&destroy_unused_sprite_var65, &g_UNUSED_SPRITE_VAR65},
    {&destroy_unused_sprite_var66, &g_UNUSED_SPRITE_VAR66},
    {&destroy_unused_sprite_var67, &g_UNUSED_SPRITE_VAR67},
    {&destroy_unused_sprite_var68, &g_UNUSED_SPRITE_VAR68},
    {&destroy_unused_sprite_var69, &g_UNUSED_SPRITE_VAR69},
    {&destroy_unused_sprite_var70, &g_UNUSED_SPRITE_VAR70},
    {&destroy_unused_sprite_var71, &g_UNUSED_SPRITE_VAR71},
    {&destroy_unused_sprite_var72, &g_UNUSED_SPRITE_VAR72},
    {&destroy_unused_sprite_var73, &g_UNUSED_SPRITE_VAR73},
    {&destroy_unused_sprite_var74, &g_UNUSED_SPRITE_VAR74},
    {&destroy_unused_sprite_var75, &g_UNUSED_SPRITE_VAR75},
    {&destroy_unused_sprite_var76, &g_UNUSED_SPRITE_VAR76},
    {&destroy_unused_sprite_var77, &g_UNUSED_SPRITE_VAR77},
    {&destroy_unused_sprite_var78, &g_UNUSED_SPRITE_VAR78},
    {&destroy_unused_sprite_var79, &g_UNUSED_SPRITE_VAR79},
    {&destroy_unused_sprite_var80, &g_UNUSED_SPRITE_VAR80},
    {&destroy_unused_sprite_var81, &g_UNUSED_SPRITE_VAR81},
    {&destroy_unused_sprite_var82, &g_UNUSED_SPRITE_VAR82},
    {&destroy_unused_sprite_var83, &g_UNUSED_SPRITE_VAR83},
};
const AtexitThunkCase g_atexit_caviar_cases[] = {
    {&destroy_unused_caviardata_var1, &g_UNUSED_CAVIARDATA_VAR1},
    {&destroy_ssf_caviardata, &g_SSF_CAVIARDATA},
    {&destroy_sdp_caviardata, &g_SDP_CAVIARDATA},
    {&destroy_sas_caviardata, &g_SAS_CAVIARDATA},
    {&destroy_scd_caviardata, &g_SCD_CAVIARDATA},
    {&destroy_scj_caviardata, &g_SCJ_CAVIARDATA},
    {&destroy_sags_caviardata, &g_SAGS_CAVIARDATA},
    {&destroy_sft_caviardata, &g_SFT_CAVIARDATA},
    {&destroy_vhr_caviardata1, &g_VHR_CAVIARDATA1},
    {&destroy_sht_caviardata, &g_SHT_CAVIARDATA},
    {&destroy_srb_caviardata, &g_SRB_CAVIARDATA},
    {&destroy_asas_caviardata, &g_ASAS_CAVIARDATA},
    {&destroy_reslaser_caviardata, &g_RESLASER_CAVIARDATA},
    {&destroy_resbolt_caviardata, &g_RESBOLT_CAVIARDATA},
    {&destroy_funload_caviardata, &g_FUNLOAD_CAVIARDATA},
    {&destroy_tecload_caviardata, &g_TECLOAD_CAVIARDATA},
    {&destroy_sp_disswave_caviardata, &g_SP_DISSWAVE_CAVIARDATA},
    {&destroy_sp_marined_caviardata, &g_SP_MARINED_CAVIARDATA},
    {&destroy_sp_nanoo_caviardata, &g_SP_NANOO_CAVIARDATA},
    {&destroy_sp_soporific_caviardata, &g_SP_SOPORIFIC_CAVIARDATA},
    {&destroy_aa01_caviardata, &g_AA01_CAVIARDATA},
    {&destroy_aa_rover_caviardata, &g_AA_ROVER_CAVIARDATA},
    {&destroy_ax_caviardata, &g_AX_CAVIARDATA},
    {&destroy_aa_caviardata, &g_AA_CAVIARDATA},
    {&destroy_acolpod_caviardata, &g_ACOLPOD_CAVIARDATA},
    {&destroy_at_caviardata, &g_AT_CAVIARDATA},
    {&destroy_vta_caviardata, &g_VTA_CAVIARDATA},
    {&destroy_atp_caviardata, &g_ATP_CAVIARDATA},
    {&destroy_ssfa_caviardata, &g_SSFA_CAVIARDATA},
    {&destroy_sfta_caviardata, &g_SFTA_CAVIARDATA},
    {&destroy_vw00_caviardata, &g_VW00_CAVIARDATA},
    {&destroy_viptawl_caviardata, &g_VIPTAWL_CAVIARDATA},
    {&destroy_viptasgn_caviardata, &g_VIPTASGN_CAVIARDATA},
    {&destroy_viptapsi_caviardata, &g_VIPTAPSI_CAVIARDATA},
    {&destroy_ptmod_caviardata, &g_PTMOD_CAVIARDATA},
    {&destroy_vb_caviardata, &g_VB_CAVIARDATA},
    {&destroy_vbp_caviardata, &g_VBP_CAVIARDATA},
    {&destroy_vgmc_caviardata, &g_VGMC_CAVIARDATA},
    {&destroy_vgmcp_caviardata, &g_VGMCP_CAVIARDATA},
    {&destroy_vlights_caviardata, &g_VLIGHTS_CAVIARDATA},
    {&destroy_vpt_caviardata, &g_VPT_CAVIARDATA},
    {&destroy_a_caviardata, &g_A_CAVIARDATA},
    {&destroy_apwall_caviardata, &g_APWALL_CAVIARDATA},
    {&destroy_asgen_caviardata, &g_ASGEN_CAVIARDATA},
    {&destroy_apsid_caviardata, &g_APSID_CAVIARDATA},
    {&destroy_va01_caviardata, &g_VA01_CAVIARDATA},
    {&destroy_vhr_caviardata2, &g_VHR_CAVIARDATA2},
    {&destroy_vi_caviardata, &g_VI_CAVIARDATA},
    {&destroy_vgmt_caviardata, &g_VGMT_CAVIARDATA},
    {&destroy_vgmtp_caviardata, &g_VGMTP_CAVIARDATA},
    {&destroy_unused_caviardata_var2, &g_UNUSED_CAVIARDATA_VAR2},
    {&destroy_vwntu_caviardata, &g_VWNTU_CAVIARDATA},
    {&destroy_vt_caviardata, &g_VT_CAVIARDATA},
    {&destroy_drop_caviardata, &g_DROP_CAVIARDATA},
    {&destroy_droplet_caviardata, &g_DROPLET_CAVIARDATA},
    {&destroy_vcl_caviardata, &g_VCL_CAVIARDATA},
    {&destroy_vclt00_caviardata, &g_VCLT00_CAVIARDATA},
    {&destroy_vht_vbp_caviardata, &g_VHT_VBP_CAVIARDATA},
    {&destroy_vhtp_caviardata, &g_VHTP_CAVIARDATA},
    {&destroy_vhttp_caviardata, &g_VHTTP_CAVIARDATA},
    {&destroy_vsp_caviardata, &g_VSP_CAVIARDATA},
    {&destroy_vsptf_caviardata, &g_VSPTF_CAVIARDATA},
    {&destroy_vsptb_caviardata, &g_VSPTB_CAVIARDATA},
    {&destroy_vfl_caviardata, &g_VFL_CAVIARDATA},
    {&destroy_vgs_caviardata, &g_VGS_CAVIARDATA},
    {&destroy_vgsp_caviardata, &g_VGSP_CAVIARDATA},
    {&destroy_vjtp_caviardata, &g_VJTP_CAVIARDATA},
    {&destroy_vcu_caviardata, &g_VCU_CAVIARDATA},
    {&destroy_vcup_caviardata, &g_VCUP_CAVIARDATA},
    {&destroy_vcuw_caviardata, &g_VCUW_CAVIARDATA},
    {&destroy_vct_caviardata, &g_VCT_CAVIARDATA},
    {&destroy_vctp_caviardata, &g_VCTP_CAVIARDATA},
    {&destroy_vctb_caviardata, &g_VCTB_CAVIARDATA},
    {&destroy_vwntt_caviardata, &g_VWNTT_CAVIARDATA},
    {&destroy_vwnst_caviardata, &g_VWNST_CAVIARDATA},
    {&destroy_vwnaa_caviardata, &g_VWNAA_CAVIARDATA},
    {&destroy_vm_caviardata, &g_VM_CAVIARDATA},
    {&destroy_vm13_caviardata, &g_VM13_CAVIARDATA},
    {&destroy_nw_caviardata, &g_NW_CAVIARDATA},
    {&destroy_ni_caviardata, &g_NI_CAVIARDATA},
    {&destroy_nlc_caviardata, &g_NLC_CAVIARDATA},
};
const AtexitThunkCase g_atexit_texture_cases[] = {
    {&destroy_radius1_texture, &g_RADIUS1_TEXTURE},
    {&destroy_radius2_texture, &g_RADIUS2_TEXTURE},
    {&destroy_flat_arid_land_texture, &g_FLAT_ARID_LAND_TEXTURE},
    {&destroy_dune_land_texture, &g_DUNE_LAND_TEXTURE},
    {&destroy_rainfall_single_tile_texture, &g_RAINFALL_SINGLE_TILE_TEXTURE},
};
const AtexitThunkCase g_atexit_wave_cases[] = {
    {&destroy_alphamenu_wave, &g_ALPHAMENU_WAVE},
    {&destroy_basewin_wave, &g_BASEWIN_WAVE},
    {&destroy_credits_wave, &g_CREDITS_WAVE},
    {&destroy_designwin_wave, &g_DESIGNWIN_WAVE},
    {&destroy_menu_up_wave, &g_MENU_UP_WAVE},
    {&destroy_menu_down_wave, &g_MENU_DOWN_WAVE},
    {&destroy_scoot_wave, &g_SCOOT_WAVE},
    {&destroy_ok_wave, &g_OK_WAVE},
    {&destroy_passover_wave, &g_PASSOVER_WAVE},
    {&destroy_maininterface_wave, &g_MAININTERFACE_WAVE},
    {&destroy_multiwin_wave, &g_MULTIWIN_WAVE},
    {&destroy_top_menu_wave, &g_TOP_MENU_WAVE},
    {&destroy_crash_landing_wave, &g_CRASH_LANDING_WAVE},
    {&destroy_wave_general, &g_WAVE_GENERAL},
};
const AtexitThunkCase g_atexit_buffer_cases[] = {
    {&destroy_pcx_parse_temp_buffer1, &g_PCX_PARSE_TEMP_BUFFER1},
    {&destroy_iface_std_popups_middle_buffer, &g_IFACE_STD_POPUPS_MIDDLE_BUFFER},
    {&destroy_vehdraw_buffer, &g_VEHDRAW_BUFFER},
    {&destroy_buffer, &g_BUFFER},
};
const AtexitThunkCase g_atexit_group_cases[] = {
    {&destroy_prefwin_buttongroup, &g_PREFWIN_BUTTONGROUP},
};
const AtexitThunkCase g_atexit_battlewin_cases[] = {
    {&destroy_battlewin, &g_BattleWin},
};
const AtexitThunkCase g_atexit_fx_cases[] = {
    {&destroy_fx, &g_FX},
};
const AtexitThunkCase g_atexit_fontqueue_cases[] = {
    {&destroy_fontqueue_val2, &g_FONTQUEUE_VAL2},
    {&destroy_fontqueue_val1, &g_FONTQUEUE_VAL1},
};
const AtexitThunkCase g_atexit_font_cases[] = {
    {&destroy_jackal_font, &g_JACKAL_FONT},
};
struct AtexitArrayCase {
    void(__cdecl *thunk)();
    void *slot;
    uint32_t element_size;
    int count;
    func_thiscall_teardown **teardown_slot;
};
const AtexitArrayCase g_atexit_array_cases[] = {
    {&destroy_cpu_waves, &g_CPU_WAVES, 0x6C, 45, &WaveElementTeardown},
    {&destroy_factionart, &FactionArtGlobal, 0x65C, 8, &FactionArtElementTeardown},
    {&destroy_iface_close_x_sprites, &g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_iface_box_sprites1, &g_IFACE_BOX_SPRITES1, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites2, &g_IFACE_BOX_SPRITES2, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites3, &g_IFACE_BOX_SPRITES3, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites4, &g_IFACE_BOX_SPRITES4, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites5, &g_IFACE_BOX_SPRITES5, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites6, &g_IFACE_BOX_SPRITES6, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites7, &g_IFACE_BOX_SPRITES7, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites8, &g_IFACE_BOX_SPRITES8, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprite_buffers, &g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, &BufferElementTeardown},
    {&destroy_aa_wing_caviardata, &g_AA_WING_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_acp_caviardata, &g_ACP_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_viptr_caviardata, &g_VIPTR_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_vipta_caviardata, &g_VIPTA_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vr_caviardata, &g_VR_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_vrc_caviardata, &g_VRC_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var3, &g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vhta0_caviardata, &g_VHTA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vhttpa0_caviardata, &g_VHTTPA0_CAVIARDATA, 0xC, 3, &CaviarDataElementTeardown},
    {&destroy_vspa0_caviardata, &g_VSPA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vjt0_caviardata, &g_VJT0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vcua0_caviardata, &g_VCUA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vct0_caviardata, &g_VCT0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vw_caviardata, &g_VW_CAVIARDATA, 0xC, 16, &CaviarDataElementTeardown},
    {&destroy_vpbr0_caviardata, &g_VPBR0_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var4, &g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var5, &g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, &CaviarDataElementTeardown},
    {&destroy_rocky_textures, &g_ROCKY_TEXTURES, 0x70, 4, &TextureElementTeardown},
    {&destroy_ocean_textures, &g_OCEAN_TEXTURES, 0x70, 2, &TextureElementTeardown},
    {&destroy_moist_land_textures, &g_MOIST_LAND_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_rainy_land_textures, &g_RAINY_LAND_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_jungle_land_textures, &g_JUNGLE_LAND_TEXTURES, 0x70, 15, &TextureElementTeardown},
    {&destroy_sunny_mesa_textures, &g_SUNNY_MESA_TEXTURES, 0x70, 8, &TextureElementTeardown},
    {&destroy_rainfall_single_tile_textures, &g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, &TextureElementTeardown},
    {&destroy_road_textures, &g_ROAD_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_magtube_textures, &g_MAGTUBE_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_river_textures, &g_RIVER_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_mount_planet_textures, &g_MOUNT_PLANET_TEXTURES, 0x70, 3, &TextureElementTeardown},
    {&destroy_garland_crater_textures, &g_GARLAND_CRATER_TEXTURES, 0x70, 3, &TextureElementTeardown},
    {&destroy_fungus_textures, &g_FUNGUS_TEXTURES, 0x70, 30, &TextureElementTeardown},
    {&destroy_farm_textures, &g_FARM_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_forest_textures, &g_FOREST_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_ter1_white_org_yel_tile_sprites, &g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1_bottom_left_tile_sprites, &g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, &SpriteElementTeardown},
    {&destroy_ter1_unused_sprites2, &g_TER1_UNUSED_SPRITES2, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_ter1_manifold_nexus_sprites, &g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1wreck_unity_wreckage_sprites, &g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, &SpriteElementTeardown},
    {&destroy_ter1wreck_unity_wreckage_alt_sprites, &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_fossil_field_ridge_sprites, &g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1_unused_sprites1, &g_TER1_UNUSED_SPRITES1, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_farm_sprites, &g_TER1_FARM_SPRITES, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_soil_enricher_sprites, &g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_sea_land_resource_sprites, &g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_ter1_landmark_resource_sprites, &g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_glow_sprites, &g_GLOW_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_ter1_unity_pod_sprites, &g_TER1_UNITY_POD_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_rainfall_double_tile_sprites, &g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_veh_sprites, &g_VEH_SPRITES, 0x2C, 152, &SpriteElementTeardown},
    {&destroy_flags_veh_sprites, &g_FLAGS_VEH_SPRITES, 0x2C, 112, &SpriteElementTeardown},
    {&destroy_icons_general_sprites, &g_ICONS_GENERAL_SPRITES, 0x2C, 16, &SpriteElementTeardown},
    {&destroy_resource_icon_sprites, &g_RESOURCE_ICON_SPRITES, 0x2C, 32, &SpriteElementTeardown},
    {&destroy_citizen_lg_cursor_sprites, &g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_specialist_lg_cursor_sprites, &g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_citizen_sm_cursor_sprites, &g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_specialist_sm_cursor_sprites, &g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_al_citizen_lg_cursor_sprites, &g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_al_specialist_lg_cursor_sprites, &g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_al_citizen_sm_cursor_sprites, &g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_al_specialist_sm_cursor_sprites, &g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_silver_menu_icon_sprites, &g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_silver_checkbox_icon_sprites, &g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_peace_sign_sprites, &g_PEACE_SIGN_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_xi_boom_veh_sprites, &g_XI_BOOM_VEH_SPRITES, 0x2C, 144, &SpriteElementTeardown},
    {&destroy_xf_boom_veh_sprites, &g_XF_BOOM_VEH_SPRITES, 0x2C, 64, &SpriteElementTeardown},
    {&destroy_tech_icon_sprites, &g_TECH_ICON_SPRITES, 0x2C, 89, &SpriteElementTeardown},
    {&destroy_facility_icon_sprites, &g_FACILITY_ICON_SPRITES, 0x2C, 70, &SpriteElementTeardown},
    {&destroy_secret_project_icon_sprites, &g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, &SpriteElementTeardown},
    {&destroy_iface_mp_combo_arrow_sprites, &g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_scroll_bar_arrow_icon_sprites, &g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_scroll_bar_small_arrow_icon_sprites, &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_iface_lock_sprites, &g_IFACE_LOCK_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_unused_sprites_var01, &g_UNUSED_SPRITES_VAR01, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_iface_tech_tree_arrow_sprites, &g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var02, &g_UNUSED_SPRITES_VAR02, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var03, &g_UNUSED_SPRITES_VAR03, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var04, &g_UNUSED_SPRITES_VAR04, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var05, &g_UNUSED_SPRITES_VAR05, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var06, &g_UNUSED_SPRITES_VAR06, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var07, &g_UNUSED_SPRITES_VAR07, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var08, &g_UNUSED_SPRITES_VAR08, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var09, &g_UNUSED_SPRITES_VAR09, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var10, &g_UNUSED_SPRITES_VAR10, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_basewin_sprites, &g_BASEWIN_SPRITES, 0x2C, 27, &SpriteElementTeardown},
    {&destroy_iface_green_right_arrow_sprite, &g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, &SpriteElementTeardown},
    {&destroy_cursor_sprites, &g_CURSOR_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_fonts, &g_FONTS, 0x28, 48, &FontElementTeardown},
    {&destroy_txtindex, &TxtIndexGlobal, 0x118, 4, &TextIndexElementTeardown},
};

Wave *g_atexit_wave_seen;
int g_atexit_wave_calls;
void __thiscall observe_wave_destructor(Wave *wave) {
    g_atexit_wave_seen = wave;
    ++g_atexit_wave_calls;
}
void *g_vector_array_seen;
uint32_t g_vector_size_seen;
int g_vector_count_seen;
func_thiscall_teardown *g_vector_teardown_seen;
int g_vector_calls;
void __stdcall observe_vector_dtor(void *array, unsigned int element_size,
                                   int count,
                                   func_thiscall_teardown *teardown) {
    g_vector_array_seen = array;
    g_vector_size_seen = element_size;
    g_vector_count_seen = count;
    g_vector_teardown_seen = teardown;
    ++g_vector_calls;
}
int g_vector_sentinel;
}  // namespace

void test_atexit_teardown_thunks() {
    // Every thunk is "tear down the object at this fixed address". Rebinding
    // the per-global seam to a local object and watching the teardown's own
    // observable - the free seam it calls, or the destructor dependency -
    // proves each thunk reaches its own global and the right teardown.
    auto *const saved_sprite_free = SpriteFree;
    int *const saved_sprite_memory = SpriteMemoryUsed;
    auto *const saved_caviar_free = CaviarDataFreeRecord;
    auto *const saved_texture_free = TextureFree;
    auto *const saved_wave_dtor = WaveOriginalDestructor;
    SpriteFree = &observe_texture_free;   // same shape; shared observer
    TextureFree = &observe_texture_free;
    CaviarDataFreeRecord = &observe_caviar_free_record;
    WaveOriginalDestructor = &observe_wave_destructor;
    int memory_used = 0;
    SpriteMemoryUsed = &memory_used;

    auto *const saved_iterator = VectorDtorIterator;
    for (const AtexitThunkCase &entry : g_atexit_sprite_cases) {
        alignas(4) uint8_t fake[sizeof(Sprite)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // second free branch only
        auto **slot = static_cast<Sprite **>(entry.slot);
        Sprite *const saved = *slot;
        *slot = reinterpret_cast<Sprite *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);         // its own global, freed once
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_caviar_cases) {
        alignas(4) uint8_t fake[sizeof(CaviarData)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x8, &pointer, 4);
        auto **slot = static_cast<CaviarData **>(entry.slot);
        CaviarData *const saved = *slot;
        *slot = reinterpret_cast<CaviarData *>(fake);
        g_caviar_free_calls = 0;
        g_caviar_freed = nullptr;
        entry.thunk();
        expect(g_caviar_free_calls == 1);
        expect(g_caviar_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_texture_cases) {
        alignas(4) uint8_t fake[sizeof(Texture)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // ours: borrowed_ stays 0
        auto **slot = static_cast<Texture **>(entry.slot);
        Texture *const saved = *slot;
        *slot = reinterpret_cast<Texture *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_wave_cases) {
        alignas(4) uint8_t fake[sizeof(Wave)] = {};
        auto **slot = static_cast<Wave **>(entry.slot);
        Wave *const saved = *slot;
        *slot = reinterpret_cast<Wave *>(fake);
        g_atexit_wave_calls = 0;
        g_atexit_wave_seen = nullptr;
        entry.thunk();
        expect(g_atexit_wave_calls == 1);
        expect(g_atexit_wave_seen == reinterpret_cast<Wave *>(fake));
        *slot = saved;
    }

    // Array thunks hand the whole walk to the iterator seam: the observed
    // call must carry the rebound array, the exact element size and count,
    // and the rebound per-element teardown - proving the body reads all four
    // seams rather than any baked literal.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitArrayCase &entry : g_atexit_array_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_teardown = *entry.teardown_slot;
        *slot = fake;
        *entry.teardown_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        expect(g_vector_size_seen == entry.element_size);
        expect(g_vector_count_seen == entry.count);
        expect(g_vector_teardown_seen ==
               reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
        *slot = saved_slot;
        *entry.teardown_slot = saved_teardown;
    }
    VectorDtorIterator = saved_iterator;

    // Buffer and ButtonGroup teardowns are already source-owned, so each
    // thunk is checked against ground truth directly: run the real teardown
    // on an identical twin and require the bytes to agree.
    for (const AtexitThunkCase &entry : g_atexit_buffer_cases) {
        alignas(4) uint8_t fake[sizeof(Buffer)] = {};
        alignas(4) uint8_t twin[sizeof(Buffer)] = {};
        auto **slot = static_cast<Buffer **>(entry.slot);
        Buffer *const saved = *slot;
        *slot = reinterpret_cast<Buffer *>(fake);
        entry.thunk();
        reinterpret_cast<Buffer *>(twin)->destroy();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }
    for (const AtexitThunkCase &entry : g_atexit_group_cases) {
        alignas(4) uint8_t fake[sizeof(ButtonGroup)];
        alignas(4) uint8_t twin[sizeof(ButtonGroup)];
        seed_storage(fake, twin, sizeof(fake));
        auto **slot = static_cast<ButtonGroup **>(entry.slot);
        ButtonGroup *const saved = *slot;
        *slot = reinterpret_cast<ButtonGroup *>(fake);
        entry.thunk();
        reinterpret_cast<ButtonGroup *>(twin)->close();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }

    // FX and FontQueue teardowns hand their member-array walk to the vector
    // iterator; the observed array base must be the rebound global. Sizes and
    // counts are pinned by the destructors' own suites.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitThunkCase &entry : g_atexit_fx_cases) {
        alignas(4) uint8_t fake[sizeof(FX)];
        auto **slot = static_cast<FX **>(entry.slot);
        FX *const saved = *slot;
        *slot = reinterpret_cast<FX *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    for (const AtexitThunkCase &entry : g_atexit_fontqueue_cases) {
        alignas(4) uint8_t fake[sizeof(FontQueue)];
        auto **slot = static_cast<FontQueue **>(entry.slot);
        FontQueue *const saved = *slot;
        *slot = reinterpret_cast<FontQueue *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    VectorDtorIterator = saved_iterator;

    // The Font teardown runs the suite's Font::close double, whose writes to
    // the rebound object are themselves the observation.
    for (const AtexitThunkCase &entry : g_atexit_font_cases) {
        alignas(4) uint8_t fake[sizeof(Font)];
        uint8_t untouched[sizeof(Font)];
        seed_storage(fake, untouched, sizeof(fake));
        auto **slot = static_cast<Font **>(entry.slot);
        Font *const saved = *slot;
        *slot = reinterpret_cast<Font *>(fake);
        entry.thunk();
        int32_t first = 0;
        std::memcpy(&first, fake, 4);
        expect(first == -1);               // the double's signature write
        *slot = saved;
    }

    // The BattleWin teardown is the Time member at +8, observed through the
    // suite's Time::close double recording its receiver.
    for (const AtexitThunkCase &entry : g_atexit_battlewin_cases) {
        alignas(4) uint8_t fake[sizeof(BattleWin)] = {};
        auto **slot = static_cast<BattleWin **>(entry.slot);
        BattleWin *const saved = *slot;
        *slot = reinterpret_cast<BattleWin *>(fake);
        time_close_calls = 0;
        entry.thunk();
        expect(time_close_calls == 1);
        expect(time_close_targets[0] == reinterpret_cast<Time *>(fake + 8));
        *slot = saved;
    }

    SpriteFree = saved_sprite_free;
    SpriteMemoryUsed = saved_sprite_memory;
    CaviarDataFreeRecord = saved_caviar_free;
    TextureFree = saved_texture_free;
    WaveOriginalDestructor = saved_wave_dtor;
}

void test_battle_win_dtor() {
    // The whole teardown is the Time member at +8: its close runs with that
    // member as `this`, and nothing outside that member is written. The
    // suite's Time::close double zeroes the member and stamps its resolution,
    // so the expectation models exactly that span and nothing else.
    alignas(4) uint8_t storage[sizeof(BattleWin) + 16];
    uint8_t expected[sizeof(storage)];
    auto *win = reinterpret_cast<BattleWin *>(storage);
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 8, 0, sizeof(Time));
    const uint32_t resolution = 5;
    std::memcpy(expected + 8 + 0x20, &resolution, sizeof(resolution));
    time_close_calls = 0;      // the recording window is the first 8 calls
    win->~BattleWin();
    expect(time_close_calls == 1);
    expect(time_close_targets[0] == reinterpret_cast<Time *>(storage + 8));
    expect_storage_bytes(storage, expected, sizeof(storage));
    battle_win_dtor_redirect(win, nullptr);
    expect(time_close_calls == 2);
}

void test_fx_and_font_queue_dtors() {
    // Both destructors hand a member-array walk to the vector iterator with
    // the object itself as the array base: FX walks 0x61 Wave-shaped effects
    // of 0x6C bytes, FontQueue walks its three 0x28-byte Font slots. The
    // observer pins all four arguments, and the teardown argument must be
    // whatever the element seam currently holds - rebound here - rather than
    // any baked address.
    auto *const saved_iterator = VectorDtorIterator;
    VectorDtorIterator = &observe_vector_dtor;

    alignas(4) uint8_t fx_storage[sizeof(FX)];
    auto *fx = reinterpret_cast<FX *>(fx_storage);
    func_thiscall_teardown *const saved_effect = EffectElementTeardown;
    EffectElementTeardown =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
    g_vector_calls = 0;
    fx->~FX();
    expect(g_vector_calls == 1);
    expect(g_vector_array_seen == fx_storage);
    expect(g_vector_size_seen == 0x6C);
    expect(g_vector_count_seen == 0x61);
    expect(g_vector_teardown_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
    fx_dtor_redirect(fx, nullptr);
    expect(g_vector_calls == 2);
    EffectElementTeardown = saved_effect;

    alignas(4) uint8_t queue_storage[sizeof(FontQueue)];
    auto *queue = reinterpret_cast<FontQueue *>(queue_storage);
    func_thiscall_teardown *const saved_font = FontQueueElementTeardown;
    FontQueueElementTeardown =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
    g_vector_calls = 0;
    queue->~FontQueue();
    expect(g_vector_calls == 1);
    expect(g_vector_array_seen == queue_storage);
    expect(g_vector_size_seen == 0x28);
    expect(g_vector_count_seen == 3);
    expect(g_vector_teardown_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
    font_queue_dtor_redirect(queue, nullptr);
    expect(g_vector_calls == 2);
    FontQueueElementTeardown = saved_font;

    VectorDtorIterator = saved_iterator;
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
    test_popup_close();
    test_guarded_delegates();
    test_virtual_base_closes();
    test_edit_group_text();
    test_dialogs_dispatch();
    test_fixed_argument_delegates();
    test_offset_delegates();
    test_win_sync_palette();
    test_self_contained_stores();
    test_base_pop_set_width();
    test_map_win_close();
    test_string_box_add();
    test_buffer_clear_links();
    test_net_daemon_receive();
    test_win_scroll_forwarders();
    test_console_preference_openers();
    test_base_win_iface_clicks();
    test_base_win_iface_scrolled();
    test_dialogs_forwarders();
    test_base_win_clicks();
    test_window_click_forwarders();
    test_popup_exec_and_start_overloads();
    test_replay_win_stubs();
    test_bulk_generated_stubs();
    test_remaining_constant_stubs();
    test_field_store_batch2();
    test_pull_down_id_to_index();
    test_loop_store_searches();
    test_win_unk3_contains();
    test_console_clear_group();
    test_lock_reset_map();
    test_lock_clear();
    test_lock_any_locks();
    test_lock_unlock();
    test_lock_global_lock();
    test_lock_check_global_2();
    test_lock_check_global();
    test_lock_add_lock();
    test_lock_lock();
    test_square_lock_unlock();
    test_square_lock_lock();
    test_console_edit_lock();
    test_wave_device_enable_disable();
    test_sound_fade();
    test_wave_unload();
    test_wave_set_pitch_and_load();
    test_zeroed_constant_return_stubs();
    test_wrapped_device_forwarders();
    test_wrapped_device_forwarders_with_defaults();
    test_sound_guarded_forwarders();
    test_console_editor_undo_and_prod_picker_close();
    test_plan_win_close();
    test_buffer_copy_overload();
    test_plan_win_blink_and_unk1();
    test_base_button_set();
    test_texture_lifecycle();
    test_status_win_set_loc();
    test_field_store_clears();
    test_field_store_writes();
    test_button_group_set();
    test_delegating_closers();
    test_wave_is_playing();
    test_datalink_help_forwarders();
    test_net_daemon_synch_forwarders();
    test_x_pop_forwarders();
    test_x_pops_forwarders();
    test_caviar_data_close();
    test_texture_dtor();
    test_pop_pops_forwarders();
    test_atexit_teardown_thunks();
    test_battle_win_dtor();
    test_fx_and_font_queue_dtors();
    test_wave_destructor();
    test_wave_play();
    test_wave_device_forwarders();
    test_wave_volume_fname_play();
    test_wave_load_empty();
    test_wave_ctor();
    test_wave_init();
    test_sound_set_type_and_load();
    test_wave_device_groups();
    test_sound_small_setters();
    test_wave_device_forwarder_family();
    return failures == 0 ? 0 : 1;
}
