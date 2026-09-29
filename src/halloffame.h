#pragma once

#include <cstdint>
#include <cstddef>

// The original save/load block at 0x009A3840 has 35 records with a 0x8C
// stride. FameWin::try_to_add_me writes these fields and copies three names
// from Player; the final 0x22 bytes still need interpretation. The first byte
// is set when an entry is added and cleared after failed file I/O.
struct HallOfFameRecord {
  uint8_t present;
  uint8_t award_flags;
  uint8_t unknown_02[2];
  int year;
  int leader_one;
  int leader_two;
  int rating;
  int field_14;
  int field_18;
  int field_1C;
  uint8_t field_20;
  uint8_t field_21;
  char leader_title[24];
  char leader_name[24];
  char faction_noun[24];
  uint8_t unknown_6A[0x22];
};

enum { HallOfFameRecordCount = 35 };
extern HallOfFameRecord HallOfFameRecords[HallOfFameRecordCount];
// Set before each hall-of-fame read or write (original 0x00759194).
extern int HallOfFameIOMode;

static_assert(sizeof(HallOfFameRecord) == 0x8C,
              "Hall-of-fame record must match the original file stride");
static_assert(offsetof(HallOfFameRecord, leader_title) == 0x22,
              "Hall-of-fame title offset must match FameWin stores");
static_assert(offsetof(HallOfFameRecord, faction_noun) == 0x52,
              "Hall-of-fame faction noun offset must match FameWin stores");
static_assert(sizeof(HallOfFameRecord) * HallOfFameRecordCount == 0x1324,
              "Hall-of-fame save block must match the original file extent");
