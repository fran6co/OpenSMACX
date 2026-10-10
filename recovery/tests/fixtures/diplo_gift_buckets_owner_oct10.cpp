#include <assert.h>
#include <stdint.h>
#include <limits.h>

static int diplo_add(int a, int b) {
    return static_cast<int>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}
static int diplo_sub(int a, int b) {
    return static_cast<int>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}
static int diplo_mul(int a, int b) {
    return static_cast<int>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
}
struct TestPlayerData { int energy_reserves; };
static TestPlayerData PlayersData[1];
static int DiplomacyProposalMode;

static int pact_cap(int cap_numerator, int cap_denominator,
                    int energySnapshot, int proposalMode) {
    DiplomacyProposalMode = proposalMode;
    /* PACT_CAP_EXPRESSION */
    /* PACT_ADJUSTMENT_BLOCK */
    return cap;
}

static int treaty_bucket(int treaty_score, int reserve, int proposalMode) {
    const int actor = 0;
    PlayersData[actor].energy_reserves = reserve;
    DiplomacyProposalMode = proposalMode;
    /* TREATY_BUCKET_BLOCK */
    /* TREATY_MINIMUM_BLOCK */
    return treaty_score;
}

int main() {
    // Signed C++ division truncates toward zero, including negative buckets.
    assert(pact_cap(49, 1, INT_MAX, 0) == 0);
    assert(pact_cap(50, 1, INT_MAX, 0) == 50);
    assert(pact_cap(99, 1, INT_MAX, 0) == 50);
    assert(pact_cap(100, 1, INT_MAX, 0) == 100);
    assert(pact_cap(-49, 1, INT_MAX, 0) == 0);
    assert(pact_cap(-50, 1, INT_MAX, 0) == -50);
    assert(pact_cap(-99, 1, INT_MAX, 0) == -50);
    assert(pact_cap(-100, 1, INT_MAX, 0) == -100);
    // Division is staged: truncating by denominator precedes /50.
    assert(pact_cap(149, 2, INT_MAX, 0) == 50);
    assert(pact_cap(150, 2, INT_MAX, 0) == 50);
    // Wrapped quotient inputs retain the 32-bit signed bit pattern.
    assert(pact_cap(diplo_add(INT_MAX, 50), 1, INT_MAX, 0) == -2147483550);
    // Mode 5 skips the energy adjustment; ordinary mode follows subtract,
    // negative-to-zero, then upper clamp ordering.
    assert(pact_cap(300, 1, 100, 5) == 300);
    assert(pact_cap(300, 1, 100, 0) == 100);
    assert(pact_cap(50, 1, -1, 0) == 0);
    assert(pact_cap(100100, 1, 100000, 0) == 99999);

    // Unsigned +49 and /50 buckets, including wrap at the DWORD boundary.
    assert(treaty_bucket(1001, 1000, 0) == 951);
    assert(treaty_bucket(1050, 1000, 0) == 1000);
    assert(treaty_bucket(1051, 1000, 0) == 951);
    assert(treaty_bucket(1101, 1000, 0) == 951);
    assert(treaty_bucket(1051, 1000, 5) == 1051);
    assert(treaty_bucket(INT_MAX, -1, 0) == 0);
    assert(treaty_bucket(INT_MIN, 1, 0) == 0);
    assert(treaty_bucket(49, 0, 0) == 0);
    return 0;
}
