/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "spying_recovery.h"
#include "faction.h"  // Players, RFLAG_ALIEN

int SpyingCurrentFaction;  // 0x00939284
int SpyingBaseIndex;  // 0x009A6524
int SpyingObserverFaction;  // 0x009A6614

/*
Purpose: Determine whether the current faction has intelligence visibility of
         the given subject faction.
// ORIGINAL: 0x0055BC00 ?spying@@YAHH@Z 0x0055BC00-0x0055BC7C BYTE_EXACT
// LEVER: ONE `||` CHAIN IN A `return`, not three early-return `if`s. The image reaches its `mov eax, 1` epilogue from three different jumps and its `xor eax, eax` epilogue from two - shared tails, which is the opposite of the usual split-the-guards lever and is what a single `return A || B || C;` produces. Separate `if (...) return 1;` statements emit an inline epilogue after each test: 9 of 44, similarity 0.632. The chain alone took it to 0.860.
// LEVER: the MIDDLE operand is a TERNARY, not an `&&`. `base >= 0 && byte == faction` short-circuits with a plain `je`, where the image materialises the comparison - `xor eax, eax / cmp edx, ecx / sete al / test eax, eax / jne` - which is what `(base >= 0 ? (byte == faction) : 0) != 0` emits. That was the last three instructions: 42 compiled against 44, then 44 of 44. `!!(...)`, `static_cast<int>(...) != 0`, an extra `(...) != 0` and the comparison written the other way round all stay at 42.
// size      124 bytes
// prototype 
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete

Verification note: a mutation sweep of this function against the default
`recovery-leaf-tests` target reports 10 survivors out of 10 - every constant,
not merely some. That is the suite split, not a coverage hole. There is no
leaf test for spying and there is not meant to be one: its gate is
run_spying_oracle_suite() below, an in-process differential oracle that runs
the recovered body against the original at 0x0055BC00 across a swept matrix
of faction, base index, observer and subject, with the status and base tables
seeded so that a wrong stride cannot read another zero and compare equal.
That is strictly stronger than any leaf fixture, and it runs under the hybrid
smoke target rather than under ctest. To sweep this function meaningfully,
point the harness at that gate; measured against the leaf suite the result is
vacuous by construction.
*/
int __cdecl spying(int subject) {
    const int faction = SpyingCurrentFaction;
    return (SpyingStatusTable[static_cast<size_t>(subject)
                              + static_cast<size_t>(faction)
                                    * SpyingStatusStride]
            & 0x1000U) != 0
           || (SpyingBaseIndex >= 0
               ? (SpyingBaseFactionBytes[static_cast<size_t>(SpyingBaseIndex)
                                         * SpyingBaseStride] == faction)
               : 0) != 0
           || (faction == SpyingObserverFaction
               && (Players[static_cast<size_t>(subject)].rule_flags &
                   RFLAG_ALIEN) == 0);
}

namespace {

typedef int(__cdecl *OriginalSpying)(int);

}  // namespace

bool run_spying_oracle_suite() {
    // A pure read over original-image tables: both sides observe identical
    // globals, so every branch is compared by driving the two selector globals
    // across their interesting values and sweeping the subject index.
    OriginalSpying original = reinterpret_cast<OriginalSpying>(0x0055BC00U);
    const int saved_base = SpyingBaseIndex;
    const int saved_observer = SpyingObserverFaction;
    const int saved_faction = SpyingCurrentFaction;
    bool passed = true;

    // The current faction scales the status-table index, so it must be swept:
    // leaving it at its startup value of zero makes the stride unobservable
    // and the comparison vacuous.
    // The status table is empty this early in startup, so a wrong faction
    // stride would read another zero and compare equal. Seed distinctive
    // entries at the exact indices the legacy body addresses, then restore.
    const size_t SeedFactions = 8;
    const size_t SeedSubjects = 8;
    uint32_t saved_entries[SeedFactions][SeedSubjects];
    for (size_t f = 0; f < SeedFactions; ++f) {
        for (size_t s = 0; s < SeedSubjects; ++s) {
            const size_t index = s + f * SpyingStatusStride;
            saved_entries[f][s] = SpyingStatusTable[index];
            // Alternate the visibility bit so both branches are taken.
            SpyingStatusTable[index] = ((f + s) & 1U) ? 0x1000U : 0x0020U;
        }
    }

    // The base table is likewise empty this early, so its stride would also be
    // unobservable. Seed the faction byte of each base the sweep addresses.
    const int base_indices[] = {-1, 0, 1, 7};
    uint8_t saved_base_bytes[8];
    for (size_t b = 0; b < 8; ++b) {
        uint8_t &cell = SpyingBaseFactionBytes[b * SpyingBaseStride];
        saved_base_bytes[b] = cell;
        cell = static_cast<uint8_t>(b);
    }
    for (int faction = 0; faction < 8 && passed; ++faction) {
        SpyingCurrentFaction = faction;
        const int observers[] = {faction, faction + 1, -1};
        for (size_t base_index_index = 0;
             base_index_index < sizeof(base_indices) / sizeof(base_indices[0]);
             ++base_index_index) {
            int base_index = base_indices[base_index_index];
            for (size_t observer_index = 0;
                 observer_index < sizeof(observers) / sizeof(observers[0]);
                 ++observer_index) {
                int observer = observers[observer_index];
                SpyingBaseIndex = base_index;
                SpyingObserverFaction = observer;
                for (int subject = 0; subject < 8 && passed; ++subject) {
                    if (original(subject) != spying(subject)) {
                        passed = false;
                    }
                }
                if (!passed) {
                    break;
                }
            }
            if (!passed) {
                break;
            }
        }
    }
    for (f = 0; f < SeedFactions; ++f) {
        for (size_t s = 0; s < SeedSubjects; ++s) {
            SpyingStatusTable[s + f * SpyingStatusStride] = saved_entries[f][s];
        }
    }
    for (b = 0; b < 8; ++b) {
        SpyingBaseFactionBytes[b * SpyingBaseStride] = saved_base_bytes[b];
    }
    SpyingCurrentFaction = saved_faction;
    SpyingBaseIndex = saved_base;
    SpyingObserverFaction = saved_observer;
    return passed;
}
