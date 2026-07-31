// Recovery leaf tests: core_construction.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "core_construction"

namespace {

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

void test_mandate_color() {
    // Four colours, one dword every eight bytes; index 3 is the default the
    // switch falls through to for any selector outside 0..2.
    uint32_t table[7] = {
        0x11111111U, 0xDEADDEADU, 0x22222222U, 0xDEADDEADU,
        0x33333333U, 0xDEADDEADU, 0x44444444U,
    };
    uint32_t *const saved = MandateColors;
    MandateColors = table;

    // The three explicit selectors read entries 0, 1, 2.
    expect(mandate_color(0) == static_cast<int>(0x11111111U));
    expect(mandate_color(1) == static_cast<int>(0x22222222U));
    expect(mandate_color(2) == static_cast<int>(0x33333333U));
    // Everything else - above the range, and negative - returns the fourth.
    expect(mandate_color(3) == static_cast<int>(0x44444444U));
    expect(mandate_color(100) == static_cast<int>(0x44444444U));
    expect(mandate_color(-1) == static_cast<int>(0x44444444U));
    // The redirect forwards verbatim.
    expect(mandate_color_redirect(1) == static_cast<int>(0x22222222U));
    expect(mandate_color_redirect(-7) == static_cast<int>(0x44444444U));

    MandateColors = saved;
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

}  // namespace

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

namespace {

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

}  // namespace

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

namespace {

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

}  // namespace

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "tests require 32-bit int");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

namespace {

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
    // offset_rect slides both x fields by the first delta and both y fields by
    // the second. Every case uses dx != dy and four distinct field values, so
    // a delta applied to the wrong axis moves the wrong pair and shows up in
    // the byte compare; the wrapping cases pin that the adds are modular
    // rather than saturating, the way `add` is.
    struct OffsetCase {
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
        uint32_t dx;
        uint32_t dy;
    };
    const OffsetCase offset_cases[] = {
        {10, 20, 30, 40, 0, 0},
        {10, 20, 30, 40, 3, 7},
        {10, 20, 30, 40, 0xFFFFFFFDU, 0xFFFFFFF9U},
        {0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U, 0x80000000U, 1, 1},
        {0, 0, 0, 0, 0x80000000U, 0x7FFFFFFFU},
    };
    for (const OffsetCase &test : offset_cases) {
        alignas(RECT) uint8_t storage[sizeof(RECT) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16, test.left);
        write_at(storage, 20, test.top);
        write_at(storage, 24, test.right);
        write_at(storage, 28, test.bottom);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16, test.left + test.dx);
        write_at(expected, 20, test.top + test.dy);
        write_at(expected, 24, test.right + test.dx);
        write_at(expected, 28, test.bottom + test.dy);
        offset_rect(reinterpret_cast<RECT *>(storage + 16),
                    int_from_bits(test.dx), int_from_bits(test.dy));
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

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

}  // namespace

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(6, test_alpha_net_pid_to_idx);
LEAF_CASE(7, test_alpha_net_identity_lookups);
LEAF_CASE(8, test_in_box_edges);
LEAF_CASE(60, test_mandate_color);
LEAF_CASE(73, test_text_get_and_item_number);
LEAF_CASE(74, test_text_string_helpers);
LEAF_CASE(90, test_button_group_lifecycle);
LEAF_CASE(99, test_button_group_add);
LEAF_CASE(91, test_base_pop_string_font);
LEAF_CASE(0, test_auto_sound_construct);
LEAF_CASE(1, test_win_construct);
LEAF_CASE(2, test_palette_get_rgbquad);
LEAF_CASE(3, test_buffer_construct);
LEAF_CASE(4, test_graphic_win_construct);
LEAF_CASE(5, test_base_button_construct);
LEAF_CASE(9, test_geometry_helpers);
LEAF_CASE(10, test_vector_lifecycle);
LEAF_CASE(11, test_vector_arithmetic);
}  // namespace
