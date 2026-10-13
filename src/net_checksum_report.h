#pragma once

#include <stddef.h>

// Observed output contract of NetDaemon::game_checksum and its mapped caller.
// The diagnostic groups are written only when the debug popup flag is set.
struct NetChecksumReport {
  int common[5];
  int map_sections[16];
  int diagnostic_aggregates[3];
  int faction_values[7];
  int map_values[7];
  int vehicles_by_faction[8];
  int base_value;
};

static_assert(sizeof(int) == 4, "checksum report uses signed dwords");
static_assert(offsetof(NetChecksumReport, common) == 0x00,
              "checksum common group offset");
static_assert(offsetof(NetChecksumReport, map_sections) == 0x14,
              "checksum map-section group offset");
static_assert(offsetof(NetChecksumReport, diagnostic_aggregates) == 0x54,
              "checksum diagnostics begin after 21 common words");
static_assert(offsetof(NetChecksumReport, faction_values) == 0x60,
              "checksum faction group offset");
static_assert(offsetof(NetChecksumReport, map_values) == 0x7C,
              "checksum map-component group offset");
static_assert(offsetof(NetChecksumReport, vehicles_by_faction) == 0x98,
              "checksum vehicle-faction group offset");
static_assert(offsetof(NetChecksumReport, base_value) == 0xB8,
              "checksum base diagnostic offset");
static_assert(sizeof(NetChecksumReport) == 0xBC,
              "observed checksum report output extent");
