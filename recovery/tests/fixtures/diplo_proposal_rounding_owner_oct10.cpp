#include <cassert>
#include <cstdint>
#include <climits>
#include <initializer_list>

static int32_t signed_bits(uint32_t bits) {
    return bits <= 0x7fffffffu ? static_cast<int32_t>(bits)
                              : -1 - static_cast<int32_t>(~bits);
}

static int diplo_add(int a, int b) {
    return signed_bits(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}

static int diplo_sub(int a, int b) {
    return signed_bits(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}

static int diplo_mul(int a, int b) {
    return signed_bits(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
}

static int sar_floor(int64_t value, unsigned shift) {
    const int64_t divisor = int64_t{1} << shift;
    return static_cast<int>(value >= 0 ? value / divisor
                                       : -1 - ((-1 - value) / divisor));
}

// Literal x86 signed IMUL-high/SAR/sign-correction pipelines from the pinned
// code. The reciprocal is not itself the divisor; each shift is load-bearing.
static int pinned_div50(int value) {
    const int64_t product = static_cast<int64_t>(value) * 0x51eb851f;
    const int high = sar_floor(product, 32);
    const int quotient = sar_floor(high, 4);
    return quotient + (quotient < 0 ? 1 : 0);
}

static int pinned_div25(int value) {
    const int64_t product = static_cast<int64_t>(value) * 0x51eb851f;
    const int high = sar_floor(product, 32);
    const int quotient = sar_floor(high, 3);
    return quotient + (quotient < 0 ? 1 : 0);
}

/* ACTUAL_PACT_HELPER */

struct PlayerData {
    int energy_reserves;
    int mil_strength_1;
};
static PlayerData PlayersData[2];
static int DiploScratchFA78;

static int actual_treaty_score(int actor, int other) {
    /* ACTUAL_TREATY_INPUTS */
    /* ACTUAL_WEIGHTED_ENERGY_AND_TREATY_SCORE */
    return treaty_score;
}

static int actual_proposal_attack_threshold(int policy_class,
                                            int proposal_attack_score) {
    /* ACTUAL_PROPOSAL_ATTACK_STAGES */
    return proposal_attack_threshold;
}

static int expected_treaty_score(int actor_energy, int actor_strength,
                                 int other_strength, int scratch) {
    const int weighted_energy = diplo_mul(diplo_add(actor_energy, scratch),
                                          other_strength)
        / diplo_add(diplo_add(actor_strength, other_strength), 1);
    return diplo_mul(pinned_div50(weighted_energy), 50);
}

static int expected_proposal_threshold(int policy_class,
                                       int proposal_attack_score) {
    const int score_after_class = diplo_mul(
        diplo_sub(4, policy_class), proposal_attack_score) / 4;
    return diplo_mul(pinned_div25(diplo_add(score_after_class, 24)), 25);
}

int main() {
    const int values[] = {
        INT_MIN, INT_MIN + 1, -10001, -51, -50, -49, -26, -25, -24,
        -1, 0, 1, 24, 25, 26, 49, 50, 51, 99, 100, 101,
        INT_MAX - 1, INT_MAX
    };
    for (int value : values) {
        assert(pinned_div50(value) == value / 50);
        assert(pinned_div25(value) == value / 25);
    }
    assert(pinned_div50(50) == 1);
    assert(pinned_div25(25) == 1);

    // Exercise the actual helper's signed wrapping, max staging, output store,
    // threshold branch, and mode/offer result for boundary and counterexample
    // inputs. The helper's branch contract is independently computed below.
    for (int cap : values) {
        for (int turn : values) {
            const int comparison = diplo_mul(cap, 2) > diplo_mul(turn, 5)
                ? diplo_mul(cap, 2) : diplo_mul(turn, 5);
            const int threshold = diplo_mul(pinned_div50(comparison), 50);
            for (int mode : {1, 4, 5}) {
                for (int offer : {-1, 0}) {
                    for (int energy : {INT_MIN, threshold, INT_MAX}) {
                        int actual_cap = cap;
                        const bool actual = propose_pact_ahead_refusal(
                            mode, offer, energy, actual_cap, turn);
                        const bool expected = energy < threshold
                            ? mode == 5 || offer < 0
                            : offer < 0 && mode == 4;
                        assert(actual_cap == threshold);
                        assert(actual == expected);
                    }
                }
            }
        }
    }
    {
        int cap = 25;
        (void)propose_pact_ahead_refusal(1, 0, INT_MAX, cap, 0);
        assert(cap == 50);
    }

    // The expression is extracted from the real treaty scorer, including its
    // preceding weighted-energy declaration and wrapped numerator/denominator.
    const int strengths[] = {INT_MIN, -101, -2, -1, 0, 1, 2, 49, 50, 99,
                             100, INT_MAX - 1, INT_MAX};
    for (int actor_energy : values) {
        for (int scratch : {-1, 0, 1, INT_MIN, INT_MAX}) {
            for (int actor_strength : strengths) {
                for (int other_strength : strengths) {
                    const int denominator = diplo_add(
                        diplo_add(actor_strength, other_strength), 1);
                    const int numerator = diplo_mul(
                        diplo_add(actor_energy, scratch), other_strength);
                    if (denominator == 0 ||
                        (numerator == INT_MIN && denominator == -1)) continue;
                    PlayersData[0] = {actor_energy, actor_strength};
                    PlayersData[1] = {0, other_strength};
                    DiploScratchFA78 = scratch;
                    assert(actual_treaty_score(0, 1) == expected_treaty_score(
                        actor_energy, actor_strength, other_strength, scratch));
                }
            }
        }
    }
    PlayersData[0] = {100, 0};
    PlayersData[1] = {0, 1};
    DiploScratchFA78 = 0;
    assert(actual_treaty_score(0, 1) == 50);
    assert(actual_treaty_score(0, 1) == expected_treaty_score(100, 0, 1, 0));

    for (int policy_class : values) {
        for (int score : values) {
            assert(actual_proposal_attack_threshold(policy_class, score) ==
                   expected_proposal_threshold(policy_class, score));
        }
    }
    assert(actual_proposal_attack_threshold(0, 1) == 25);
}
