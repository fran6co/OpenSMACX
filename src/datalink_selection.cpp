#include "stdafx.h"
#include "datalink.h"
#include "strings.h"
#include "faction.h"

// ORIGINAL: 0x00590D10 sub_590d10 0x00590D10-0x00590D3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d10.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append_text@DatalinkSelectionEntry@@QBEXPADH@Z
void DatalinkSelectionEntry::append_text(char *output, int extra) const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.append_text != 0) {
    callbacks.append_text(output, value, extra);
  }
}

// ORIGINAL: 0x00590D40 sub_590d40 0x00590D40-0x00590D5C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d40.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?show_help@DatalinkSelectionEntry@@QBEXXZ
void DatalinkSelectionEntry::show_help() const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.help != 0) {
    callbacks.help(value);
  }
}

// ORIGINAL: 0x00590D60 sub_590d60 0x00590D60-0x00590D6B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/field_accessors.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?clear@DatalinkSelectionQueue@@QAEXXZ
void DatalinkSelectionQueue::clear() {
  count_ = 0;
}

// ORIGINAL: 0x00590D70 sub_590d70 0x00590D70-0x00590DB3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d70.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append@DatalinkSelectionQueue@@QAEXFEE@Z
void DatalinkSelectionQueue::append(short entry_id, unsigned char category,
                                    unsigned char value) {
  if (count_ < 0x80) {
    entries_[count_].entry_id = entry_id;
    entries_[count_].category = category;
    entries_[count_].value = value;
    ++count_;
  }
}

// ORIGINAL: 0x00590DC0 sub_590dc0 0x00590DC0-0x00590E90
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/unrecovered/00590dc0.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?dispatch_matching@DatalinkSelectionQueue@@QAEXH@Z
// TRIED: `if (count > 0)` reads as `cmp;jle` here, not the original's `test;jle`; tried elsewhere didn't move it. Indexed struct field access (`g_945a60[i].fn`) collapses lea+mov into one mov; original keeps them split (address-of first).
void DatalinkSelectionQueue::dispatch_matching(int entry_id) {
  DatalinkMain.init();

  int matched_count = 0;
  int first_match_index;
  for (int index = 0; index < count_; ++index) {
    DatalinkSelectionEntry &entry = entries_[index];
    if (entry.entry_id == entry_id) {
      if (matched_count == 0) {
        first_match_index = index;
      }
      ++matched_count;
      StringTemp[0] = 0;

      if (entry.category == 5 && entry.value >= 0x46) {
        entry.category = 10;
        entry.value = static_cast<unsigned char>(entry.value + 0xBA);
      }
      entry.append_text(StringTemp, 1);
    }
  }

  if (matched_count != 0) {
    datalink_entry_selection(first_match_index);
    DatalinkMain.show(0);
    DatalinkMain.set_modal(0, pop_wait, 0);
  }
}

