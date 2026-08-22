#pragma once
#include "stdafx.h"

// Globals the spying check reads, all owned by the original image.
uint32_t *const SpyingStatusTable = (uint32_t *)0x0096C9F8;      // [faction * 2099 + subject]
extern int SpyingCurrentFaction;
extern int SpyingBaseIndex;
uint8_t *const SpyingBaseFactionBytes = (uint8_t *)0x0097D044;  // Bases[index] + 4
extern int SpyingObserverFaction;
uint8_t *const SpyingFactionFlagBytes = (uint8_t *)0x00946F58;  // Factions[index] + 0

static const size_t SpyingStatusStride = 2099;
static const size_t SpyingBaseStride = 0x134;
static const size_t SpyingFactionStride = 0x59C;

int __cdecl spying(int subject);

// Runtime-oracle suite for the spying check; defined in src/spying.cpp.
bool run_spying_oracle_suite();
