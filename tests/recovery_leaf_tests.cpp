#include "../src/stdafx.h"
#include "../src/alphanet.h"

#include <cstring>

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

}  // namespace

int main() {
    test_alpha_net_pid_to_idx();
    test_in_box_edges();
    return failures == 0 ? 0 : 1;
}
