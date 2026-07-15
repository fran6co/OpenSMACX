#include "../src/stdafx.h"
#include "../src/alphanet.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

class LegacyAlphaNet {
 public:
    int pid_2_who(uint32_t) asm("_opensmacx_legacy_004E2610");
    int who_2_pid(int) asm("_opensmacx_legacy_004E2660");
    int who_2_idx(int) asm("_opensmacx_legacy_004E26B0");
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

    return failures == 0 ? 0 : 1;
}
