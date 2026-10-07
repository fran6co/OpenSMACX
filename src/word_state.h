/*
 * Bounded word-state operation views for three original receiver-shaped
 * leaves. These distinct source types name only the word ranges touched by
 * their methods; they do not assert original class identity, complete object
 * extent, a relationship between the types, producer, caller, or lifetime.
 */
#pragma once

#include "vc6_compat.h"
#if !defined(_MSC_VER) || _MSC_VER > 1200
#include <stdint.h>
#endif

struct PrefixFlagState {
    uint32_t word_00_;
    uint32_t word_04_;
    uint32_t word_08_;

    PrefixFlagState *initialize();
};

struct NineWordSentinelState {
    uint32_t word_00_;
    uint32_t word_04_;
    uint32_t word_08_;
    uint32_t word_0C_;
    uint32_t word_10_;
    uint32_t word_14_;
    uint32_t word_18_;
    uint32_t word_1C_;
    uint32_t word_20_;

    NineWordSentinelState *reset();
};

struct PairWordState {
    uint32_t word_00_;
    uint32_t word_04_;
    uint32_t word_08_;

    PairWordState *reset_tail();
};
