#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/basepop.h"
#include "../src/buttongroup.h"

#include <cstring>
#include <new>

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

void expect_storage_bytes(
        const uint8_t *storage, const uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        expect(storage[index] == expected[index]);
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

}  // namespace

int main() {
    test_alpha_net_pid_to_idx();
    test_in_box_edges();
    test_button_group_lifecycle();
    test_base_pop_string_font();
    return failures == 0 ? 0 : 1;
}
