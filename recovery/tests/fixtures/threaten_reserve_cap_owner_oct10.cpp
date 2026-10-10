#include <assert.h>
#include <stdint.h>
#include <limits.h>

#define __cdecl

/* SOURCE_DIPLO_MUL */

struct PlayerData {
    int energy_reserves;
};
static PlayerData PlayersData[2];

static int cap_candidate(int reserve) {
    /* SOURCE_CAP_EXPRESSION */
}

static int pinned_imul_sar3_cap(int reserve) {
    const int64_t product = static_cast<int64_t>(reserve) * 0x51eb851f;
    const int32_t high_product = static_cast<int32_t>(product >> 32);
    const int32_t sar3 = high_product >> 3;
    const int32_t sign_correction = static_cast<int32_t>(
        static_cast<uint32_t>(sar3) >> 31);
    const int32_t quotient = sar3 + sign_correction;
    return quotient * 25;
}

static int active_cap_block(int energy_delta, int other_faction_id) {
    /* SOURCE_CAP_BLOCK */
    return demand;
}

int main() {
    // Probe the signed cap expression itself, before the later demand clamp.
    assert(cap_candidate(24) == 0);
    assert(cap_candidate(25) == 25);
    assert(cap_candidate(99) == 75);
    assert(cap_candidate(100) == 100);
    assert(cap_candidate(199) == 175);
    assert(cap_candidate(200) == 200);
    assert(cap_candidate(-99) == -75);
    assert(cap_candidate(-100) == -100);
    assert(cap_candidate(-199) == -175);
    assert(cap_candidate(INT_MAX) == (INT_MAX / 25) * 25);
    assert(cap_candidate(INT_MIN) == (INT_MIN / 25) * 25);
    assert(cap_candidate(24) == pinned_imul_sar3_cap(24));
    assert(cap_candidate(25) == pinned_imul_sar3_cap(25));
    assert(cap_candidate(99) == pinned_imul_sar3_cap(99));
    assert(cap_candidate(100) == pinned_imul_sar3_cap(100));
    assert(cap_candidate(-99) == pinned_imul_sar3_cap(-99));
    assert(cap_candidate(-100) == pinned_imul_sar3_cap(-100));
    assert(cap_candidate(INT_MAX) == pinned_imul_sar3_cap(INT_MAX));
    assert(cap_candidate(INT_MIN) == pinned_imul_sar3_cap(INT_MIN));

    // Active cap branch uses that candidate; inactive branch preserves the
    // wrapped signed /16 demand unchanged.
    PlayersData[1].energy_reserves = 99;
    assert(active_cap_block(16000, 1) == 75);
    PlayersData[1].energy_reserves = 100;
    assert(active_cap_block(16000, 1) == 100);
    PlayersData[1].energy_reserves = 1000;
    assert(active_cap_block(16000, 1) == 1000);

    // The final negative-demand clamp is separate from signed cap arithmetic.
    PlayersData[1].energy_reserves = INT_MAX;
    assert(active_cap_block(-1600, 1) == 0);
    return 0;
}
