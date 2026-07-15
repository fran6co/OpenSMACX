#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/random.h"
#include "../src/scroll.h"
#include "../src/win.h"

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <float.h>

class LegacyAlphaNet {
 public:
    int pid_2_who(uint32_t) asm("_opensmacx_legacy_004E2610");
    int who_2_pid(int) asm("_opensmacx_legacy_004E2660");
    int who_2_idx(int) asm("_opensmacx_legacy_004E26B0");
};

class LegacyRandom {
 public:
    void reseed(uint32_t) asm("_opensmacx_legacy_00625750");
    uint32_t get(uint32_t, uint32_t) asm("_opensmacx_legacy_00625770");
};

class LegacyWin {
 public:
    int move(int, int) asm("_opensmacx_legacy_005ED7D0");
    void set_vert_paging(int) asm("_opensmacx_legacy_005EE0F0");
    void set_horz_paging(int) asm("_opensmacx_legacy_005EE110");
};

class LegacyScroll {
 public:
    void set_border_color(int) asm("_opensmacx_legacy_00605B10");
};

namespace {

constexpr size_t CanarySize = 16;
constexpr size_t FirstPidOffset = 0x928;
constexpr size_t IdentityOffset = 0x92C;
constexpr size_t SlotStride = 0x19C;

struct Fixture {
    alignas(AlphaNet) uint8_t storage[sizeof(AlphaNet) + CanarySize * 2];

    AlphaNet *network() {
        return reinterpret_cast<AlphaNet *>(storage + CanarySize);
    }

    LegacyAlphaNet *legacy_network() {
        return reinterpret_cast<LegacyAlphaNet *>(storage + CanarySize);
    }
};

struct RandomFixture {
    alignas(Random) uint8_t storage[sizeof(Random) + CanarySize * 2];

    Random *source() {
        return reinterpret_cast<Random *>(storage + CanarySize);
    }

    LegacyRandom *legacy() {
        return reinterpret_cast<LegacyRandom *>(storage + CanarySize);
    }
};

struct WinFixture {
    alignas(Win) uint8_t storage[sizeof(Win) + CanarySize * 2];

    Win *source() {
        return reinterpret_cast<Win *>(storage + CanarySize);
    }

    LegacyWin *legacy() {
        return reinterpret_cast<LegacyWin *>(storage + CanarySize);
    }
};

struct ScrollFixture {
    alignas(Scroll) uint8_t storage[sizeof(Scroll) + CanarySize * 2];

    Scroll *source() {
        return reinterpret_cast<Scroll *>(storage + CanarySize);
    }

    LegacyScroll *legacy() {
        return reinterpret_cast<LegacyScroll *>(storage + CanarySize);
    }
};

void initialize_bytes(uint8_t *storage, size_t storage_size, size_t object_size) {
    std::memset(storage, 0xA5, storage_size);
    for (size_t offset = 0; offset < object_size; ++offset) {
        storage[CanarySize + offset] = static_cast<uint8_t>(0x35U + offset * 17U);
    }
}

void initialize(WinFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(Win));
}

void initialize(ScrollFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(Scroll));
}

template <typename T>
void write_object(uint8_t *storage, size_t offset, const T &value) {
    std::memcpy(storage + CanarySize + offset, &value, sizeof(value));
}

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "oracle requires 32-bit int");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

bool canaries_intact(const uint8_t *storage, size_t object_size) {
    uint8_t canary[CanarySize];
    std::memset(canary, 0xA5, sizeof(canary));
    return std::memcmp(storage, canary, sizeof(canary)) == 0
        && std::memcmp(storage + CanarySize + object_size,
                       canary, sizeof(canary)) == 0;
}

void initialize(RandomFixture &fixture, uint32_t seed) {
    std::memset(fixture.storage, 0xA5, sizeof(fixture.storage));
    std::memcpy(fixture.storage + CanarySize, &seed, sizeof(seed));
}

bool canaries_intact(const RandomFixture &fixture) {
    uint8_t canary[CanarySize];
    std::memset(canary, 0xA5, sizeof(canary));
    return std::memcmp(fixture.storage, canary, CanarySize) == 0
        && std::memcmp(fixture.storage + CanarySize + sizeof(Random),
                       canary, CanarySize) == 0;
}

void initialize(Fixture &fixture) {
    std::memset(fixture.storage, 0xA5, sizeof(fixture.storage));
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
    auto *bytes = reinterpret_cast<uint8_t *>(fixture.network());
    for (int slot = 0; slot < 7; ++slot) {
        std::memcpy(bytes + FirstPidOffset + slot * SlotStride,
                    &process_ids[slot], sizeof(process_ids[slot]));
        std::memcpy(bytes + IdentityOffset + slot * SlotStride,
                    &identities[slot], sizeof(identities[slot]));
    }
    std::memcpy(bytes + FirstPidOffset - SlotStride,
                &lower_decoy_pid, sizeof(lower_decoy_pid));
    std::memcpy(bytes + IdentityOffset - SlotStride,
                &lower_decoy_identity, sizeof(lower_decoy_identity));
    std::memcpy(bytes + FirstPidOffset + 7 * SlotStride,
                &upper_decoy_pid, sizeof(upper_decoy_pid));
    std::memcpy(bytes + IdentityOffset + 7 * SlotStride,
                &upper_decoy_identity, sizeof(upper_decoy_identity));
}

bool unchanged(const Fixture &before, const Fixture &after) {
    return std::memcmp(before.storage, after.storage, sizeof(before.storage)) == 0;
}

}  // namespace

int main() {
    const uint32_t process_ids[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 123U, 456U, 789U, 790U,
    };
    const int identities[] = {0, 1, 0x7F, -128, -1, 42, -42, 2, 3, 128, -129};
    int failures = 0;

    for (uint32_t process_id : process_ids) {
        Fixture source{};
        Fixture legacy{};
        initialize(source);
        initialize(legacy);
        Fixture source_before = source;
        Fixture legacy_before = legacy;
        const int source_result = source.network()->pid_2_who(process_id);
        const int legacy_result = legacy.legacy_network()->pid_2_who(process_id);
        const bool boundary_decoy = process_id == 789U || process_id == 790U;
        if (source_result != legacy_result
                || (boundary_decoy && source_result != 0)
                || !unchanged(source_before, source)
                || !unchanged(legacy_before, legacy)) {
            std::fprintf(stderr, "pid_2_who mismatch for 0x%08x\n", process_id);
            ++failures;
        }
    }

    for (int identity : identities) {
        Fixture source{};
        Fixture legacy{};
        initialize(source);
        initialize(legacy);
        Fixture source_before = source;
        Fixture legacy_before = legacy;
        const int source_pid = source.network()->who_2_pid(identity);
        const int legacy_pid = legacy.legacy_network()->who_2_pid(identity);
        const int source_index = source.network()->who_2_idx(identity);
        const int legacy_index = legacy.legacy_network()->who_2_idx(identity);
        const bool boundary_decoy = identity == 2 || identity == 3;
        if (source_pid != legacy_pid || source_index != legacy_index
                || (boundary_decoy && (source_pid != 0 || source_index != 0))
                || !unchanged(source_before, source)
                || !unchanged(legacy_before, legacy)) {
            std::fprintf(stderr, "identity lookup mismatch for %d\n", identity);
            ++failures;
        }
    }

    Fixture source{};
    Fixture legacy{};
    initialize(source);
    initialize(legacy);
    const uint32_t duplicate_pid = 1U;
    const int8_t later_pid_identity = 77;
    const int8_t duplicate_identity = 1;
    const uint32_t later_identity_pid = 456U;
    for (Fixture *fixture : {&source, &legacy}) {
        auto *bytes = reinterpret_cast<uint8_t *>(fixture->network());
        std::memcpy(bytes + FirstPidOffset + 5 * SlotStride,
                    &duplicate_pid, sizeof(duplicate_pid));
        std::memcpy(bytes + IdentityOffset + 5 * SlotStride,
                    &later_pid_identity, sizeof(later_pid_identity));
        std::memcpy(bytes + FirstPidOffset + 6 * SlotStride,
                    &later_identity_pid, sizeof(later_identity_pid));
        std::memcpy(bytes + IdentityOffset + 6 * SlotStride,
                    &duplicate_identity, sizeof(duplicate_identity));
    }
    Fixture source_before = source;
    Fixture legacy_before = legacy;
    const int source_identity = source.network()->pid_2_who(duplicate_pid);
    const int legacy_identity = legacy.legacy_network()->pid_2_who(duplicate_pid);
    const int source_pid = source.network()->who_2_pid(duplicate_identity);
    const int legacy_pid = legacy.legacy_network()->who_2_pid(duplicate_identity);
    const int source_index = source.network()->who_2_idx(duplicate_identity);
    const int legacy_index = legacy.legacy_network()->who_2_idx(duplicate_identity);
    if (source_identity != 1 || legacy_identity != 1
            || source_pid != 1 || legacy_pid != 1
            || source_index != 2 || legacy_index != 2
            || !unchanged(source_before, source)
            || !unchanged(legacy_before, legacy)) {
        std::fprintf(stderr, "first-match duplicate behavior mismatch\n");
        ++failures;
    }

    const uint32_t reseed_values[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t seed : reseed_values) {
        RandomFixture source_fixture{};
        RandomFixture legacy_fixture{};
        initialize(source_fixture, 0xA55AA55AU);
        initialize(legacy_fixture, 0xA55AA55AU);
        source_fixture.source()->reseed(seed);
        legacy_fixture.legacy()->reseed(seed);
        if (std::memcmp(source_fixture.storage, legacy_fixture.storage,
                        sizeof(source_fixture.storage)) != 0
                || !canaries_intact(source_fixture)
                || !canaries_intact(legacy_fixture)) {
            std::fprintf(stderr, "Random reseed mismatch for 0x%08x\n", seed);
            ++failures;
        }
    }

    struct IntegerFixture {
        uint32_t seed;
        uint32_t min;
        uint32_t max;
    };
    const IntegerFixture integer_fixtures[] = {
        {0U, 0U, 0U},
        {1U, 0U, 1U},
        {0xFFFFFFFFU, 1U, 0xFFFFFFFFU},
        {0x12345678U, 0x80000000U, 0x7FFFFFFFU},
        {0x87654321U, 0xFFFFFFFFU, 0x80000000U},
        {0xA5A5A5A5U, 123U, 456U},
    };
    for (const IntegerFixture &fixture : integer_fixtures) {
        RandomFixture source_fixture{};
        RandomFixture legacy_fixture{};
        initialize(source_fixture, fixture.seed);
        initialize(legacy_fixture, fixture.seed);
        _clearfp();
        const uint32_t source_result = source_fixture.source()->get(
            fixture.min, fixture.max);
        const unsigned int source_status = _statusfp();
        _clearfp();
        const uint32_t legacy_result = legacy_fixture.legacy()->get(
            fixture.min, fixture.max);
        const unsigned int legacy_status = _statusfp();
        _clearfp();
        if (source_result != legacy_result || source_status != legacy_status
                || std::memcmp(source_fixture.storage, legacy_fixture.storage,
                               sizeof(source_fixture.storage)) != 0
                || !canaries_intact(source_fixture)
                || !canaries_intact(legacy_fixture)) {
            std::fprintf(stderr,
                "Random integer mismatch for seed 0x%08x, bounds 0x%08x..0x%08x\n",
                fixture.seed, fixture.min, fixture.max);
            ++failures;
        }
    }

    struct MoveFixture {
        uint32_t flags;
        uint32_t x;
        uint32_t y;
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    };
    const MoveFixture move_fixtures[] = {
        {0U, 100U, 200U, 10U, 20U, 50U, 80U},
        {0xFFFFFFFDU, 0xFFFFFF9CU, 0x0000012CU,
         0x00000032U, 0xFFFFFFCEU, 0x00000096U, 0x0000004BU},
        {2U, 0x80000000U, 0x7FFFFFFFU,
         0x7FFFFFFFU, 0x80000000U, 0x80000005U, 0x7FFFFFF0U},
        {0xA5A5A5A6U, 0xFFFFFFFFU, 0U,
         0x80000000U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };
    for (const MoveFixture &fixture : move_fixtures) {
        WinFixture source_window{};
        WinFixture legacy_window{};
        initialize(source_window);
        initialize(legacy_window);
        const size_t target = (fixture.flags & 2U) ? 0x14C : 0x13C;
        for (WinFixture *window : {&source_window, &legacy_window}) {
            write_object(window->storage, 0x9C, fixture.flags);
            write_object(window->storage, target, fixture.left);
            write_object(window->storage, target + 4, fixture.top);
            write_object(window->storage, target + 8, fixture.right);
            write_object(window->storage, target + 12, fixture.bottom);
        }
        const int source_result = source_window.source()->move(
            int_from_bits(fixture.x), int_from_bits(fixture.y));
        const int legacy_result = legacy_window.legacy()->move(
            int_from_bits(fixture.x), int_from_bits(fixture.y));
        if (source_result != legacy_result || source_result != 0
                || std::memcmp(source_window.storage, legacy_window.storage,
                               sizeof(source_window.storage)) != 0
                || !canaries_intact(source_window.storage, sizeof(Win))
                || !canaries_intact(legacy_window.storage, sizeof(Win))) {
            std::fprintf(stderr, "Win::move mismatch for flags 0x%08x\n",
                         fixture.flags);
            ++failures;
        }
    }

    const uint32_t paging_values[] = {
        0U, 1U, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xA55AA55AU,
    };
    for (int vertical = 0; vertical < 2; ++vertical) {
        for (uint32_t paging_bits : paging_values) {
            WinFixture source_window{};
            WinFixture legacy_window{};
            ScrollFixture source_scroll{};
            ScrollFixture legacy_scroll{};
            initialize(source_window);
            initialize(legacy_window);
            initialize(source_scroll);
            initialize(legacy_scroll);
            const size_t pointer_offset = vertical ? 0x43C : 0x440;
            Scroll *source_pointer = source_scroll.source();
            Scroll *legacy_pointer = reinterpret_cast<Scroll *>(
                legacy_scroll.storage + CanarySize);
            write_object(source_window.storage, pointer_offset, source_pointer);
            write_object(legacy_window.storage, pointer_offset, legacy_pointer);
            WinFixture source_window_before = source_window;
            WinFixture legacy_window_before = legacy_window;
            const int paging = int_from_bits(paging_bits);
            if (vertical) {
                source_window.source()->set_vert_paging(paging);
                legacy_window.legacy()->set_vert_paging(paging);
            } else {
                source_window.source()->set_horz_paging(paging);
                legacy_window.legacy()->set_horz_paging(paging);
            }
            if (std::memcmp(source_scroll.storage, legacy_scroll.storage,
                            sizeof(source_scroll.storage)) != 0
                    || std::memcmp(source_window.storage, source_window_before.storage,
                                   sizeof(source_window.storage)) != 0
                    || std::memcmp(legacy_window.storage, legacy_window_before.storage,
                                   sizeof(legacy_window.storage)) != 0
                    || !canaries_intact(source_scroll.storage, sizeof(Scroll))
                    || !canaries_intact(legacy_scroll.storage, sizeof(Scroll))) {
                std::fprintf(stderr, "Win paging mismatch for %s value 0x%08x\n",
                             vertical ? "vertical" : "horizontal", paging_bits);
                ++failures;
            }
        }
    }

    for (int vertical = 0; vertical < 2; ++vertical) {
        WinFixture null_source{};
        WinFixture null_legacy{};
        initialize(null_source);
        initialize(null_legacy);
        Scroll *null_scroll = nullptr;
        const size_t pointer_offset = vertical ? 0x43C : 0x440;
        write_object(null_source.storage, pointer_offset, null_scroll);
        write_object(null_legacy.storage, pointer_offset, null_scroll);
        WinFixture null_source_before = null_source;
        WinFixture null_legacy_before = null_legacy;
        if (vertical) {
            null_source.source()->set_vert_paging(INT_MIN);
            null_legacy.legacy()->set_vert_paging(INT_MIN);
        } else {
            null_source.source()->set_horz_paging(INT_MAX);
            null_legacy.legacy()->set_horz_paging(INT_MAX);
        }
        if (std::memcmp(null_source.storage, null_source_before.storage,
                        sizeof(null_source.storage)) != 0
                || std::memcmp(null_legacy.storage, null_legacy_before.storage,
                               sizeof(null_legacy.storage)) != 0) {
            std::fprintf(stderr, "Win null %s paging mismatch\n",
                         vertical ? "vertical" : "horizontal");
            ++failures;
        }
    }

    const uint32_t border_colors[] = {
        0xFFFFFFFFU, 0U, 1U, 0x80000000U, 0x7FFFFFFFU,
    };
    const uint32_t thicknesses[] = {
        0xFFFFFFFFU, 0U, 1U, 2U, 0x80000000U, 0x7FFFFFFFU,
    };
    for (uint32_t color : border_colors) {
        for (uint32_t thickness : thicknesses) {
            ScrollFixture source_scroll{};
            ScrollFixture legacy_scroll{};
            initialize(source_scroll);
            initialize(legacy_scroll);
            write_object(source_scroll.storage, 0xA60, thickness);
            write_object(legacy_scroll.storage, 0xA60, thickness);
            source_scroll.source()->set_border_color(int_from_bits(color));
            legacy_scroll.legacy()->set_border_color(int_from_bits(color));
            if (std::memcmp(source_scroll.storage, legacy_scroll.storage,
                            sizeof(source_scroll.storage)) != 0
                    || !canaries_intact(source_scroll.storage, sizeof(Scroll))
                    || !canaries_intact(legacy_scroll.storage, sizeof(Scroll))) {
                std::fprintf(stderr,
                    "Scroll border mismatch for color 0x%08x, thickness 0x%08x\n",
                    color, thickness);
                ++failures;
            }
        }
    }

    return failures == 0 ? 0 : 1;
}
