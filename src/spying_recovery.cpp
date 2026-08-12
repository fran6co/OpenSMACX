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

uint32_t *SpyingStatusTable = (uint32_t *)0x0096C9F8;
int *SpyingCurrentFaction = (int *)0x00939284;
int *SpyingBaseIndex = (int *)0x009A6524;
uint8_t *SpyingBaseFactionBytes = (uint8_t *)0x0097D044;
int *SpyingObserverFaction = (int *)0x009A6614;
uint8_t *SpyingFactionFlagBytes = (uint8_t *)0x00946F58;

/*
Purpose: Determine whether the current faction has intelligence visibility of
         the given subject faction.
ORIGINAL: 0x0055BC00
// name      ?spying@@YAHH@Z
// size      124 bytes
// spans     0x0055BC00-0x0055BC7C
// prototype 
// callers   19   call targets   0
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
    const int faction = *SpyingCurrentFaction;
    // Status word for this faction/subject pair; bit 12 grants visibility
    // outright.
    const size_t status_index =
        static_cast<size_t>(subject)
        + static_cast<size_t>(faction) * SpyingStatusStride;
    if ((SpyingStatusTable[status_index] & 0x1000U) != 0) {
        return 1;
    }
    // A tracked base belonging to the current faction also grants it.
    const int base_index = *SpyingBaseIndex;
    if (base_index >= 0) {
        const uint8_t base_faction =
            SpyingBaseFactionBytes[static_cast<size_t>(base_index)
                                   * SpyingBaseStride];
        if (static_cast<int>(base_faction) == faction) {
            return 1;
        }
    }
    // Otherwise the observing faction sees every subject whose high flag bit
    // is clear.
    if (faction != *SpyingObserverFaction) {
        return 0;
    }
    const uint8_t flags =
        SpyingFactionFlagBytes[static_cast<size_t>(subject)
                               * SpyingFactionStride];
    return (flags & 0x80U) == 0 ? 1 : 0;
}

namespace {

typedef int(__cdecl *OriginalSpying)(int);

}  // namespace

bool run_spying_oracle_suite() {
    // A pure read over original-image tables: both sides observe identical
    // globals, so every branch is compared by driving the two selector globals
    // across their interesting values and sweeping the subject index.
    OriginalSpying original = reinterpret_cast<OriginalSpying>(0x0055BC00U);
    const int saved_base = *SpyingBaseIndex;
    const int saved_observer = *SpyingObserverFaction;
    const int saved_faction = *SpyingCurrentFaction;
    bool passed = true;

    // The current faction scales the status-table index, so it must be swept:
    // leaving it at its startup value of zero makes the stride unobservable
    // and the comparison vacuous.
    // The status table is empty this early in startup, so a wrong faction
    // stride would read another zero and compare equal. Seed distinctive
    // entries at the exact indices the legacy body addresses, then restore.
    constexpr size_t SeedFactions = 8;
    constexpr size_t SeedSubjects = 8;
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
        *SpyingCurrentFaction = faction;
        const int observers[] = {faction, faction + 1, -1};
        for (size_t base_index_index = 0;
             base_index_index < sizeof(base_indices) / sizeof(base_indices[0]);
             ++base_index_index) {
            int base_index = base_indices[base_index_index];
            for (size_t observer_index = 0;
                 observer_index < sizeof(observers) / sizeof(observers[0]);
                 ++observer_index) {
                int observer = observers[observer_index];
                *SpyingBaseIndex = base_index;
                *SpyingObserverFaction = observer;
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
    for (size_t f = 0; f < SeedFactions; ++f) {
        for (size_t s = 0; s < SeedSubjects; ++s) {
            SpyingStatusTable[s + f * SpyingStatusStride] = saved_entries[f][s];
        }
    }
    for (size_t b = 0; b < 8; ++b) {
        SpyingBaseFactionBytes[b * SpyingBaseStride] = saved_base_bytes[b];
    }
    *SpyingCurrentFaction = saved_faction;
    *SpyingBaseIndex = saved_base;
    *SpyingObserverFaction = saved_observer;
    return passed;
}
