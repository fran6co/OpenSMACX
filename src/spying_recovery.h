#pragma once
#include "stdafx.h"

// Remaining original-image treaty table view used by legacy diplomacy
// consumers. spying itself now uses PlayersData and SecretProject members.
uint32_t *const SpyingStatusTable = (uint32_t *)0x0096C9F8;      // [faction * 2099 + subject]
// GONE since 2026-08-29: SpyingFactionFlagBytes bound 0x00946F58, which is
// &Players[0].rule_flags (Players 0x00946A50 + 0x508) and bit 0x80 is
// RFLAG_ALIEN - both uses now read Players[i].rule_flags directly, which
// works in the standalone build where the raw address is unmapped.

static const size_t SpyingStatusStride = 2099;

int __cdecl spying(int subject);
