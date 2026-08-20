#pragma once
#include "stdafx.h"

// Globals the spying check reads, all owned by the original image.
extern uint32_t *SpyingStatusTable;      // [faction * 2099 + subject]
extern int SpyingCurrentFaction;
extern int SpyingBaseIndex;
extern uint8_t *SpyingBaseFactionBytes;  // Bases[index] + 4
extern int SpyingObserverFaction;
extern uint8_t *SpyingFactionFlagBytes;  // Factions[index] + 0

static const size_t SpyingStatusStride = 2099;
static const size_t SpyingBaseStride = 0x134;
static const size_t SpyingFactionStride = 0x59C;

int __cdecl spying(int subject);

// Runtime-oracle suite for the spying check; registered in
// src/runtime_oracle.cpp.
bool run_spying_oracle_suite();
