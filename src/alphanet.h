/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include "net_class.h"
#include "original_seam.h"
#include "heap.h"          // heap_ at 0x148C
#include "stringstruct.h"  // the ledger's virtual base

/*
 * AlphaNet class
 *
 * NET IS A BASE, AT ZERO. ??0AlphaNet@@QAE@XZ takes the receiver into esi
 * (`mov esi, ecx` at 0x004E24AD) and calls ??0Net@@QAE@XZ at 0x004E24B7 with
 * ecx still the entry `this` - no lea, no adjustment - and a constructor's
 * receiver is the object it constructs. ??1AlphaNet@@QAE@XZ says the same in
 * the other direction: at 0x00538C0C it republishes Net's vtable over the
 * object's first dword (`mov [esi], 0x66eb04`, the very constant ??0Net
 * writes at 0x0062D822, overwriting AlphaNet's own 0x66eacc from 0x004E2572)
 * and then calls ?close@Net@@QAEXXZ on that unadjusted receiver at
 * 0x00538C1B. The declaration below replaces `uint32_t alignment_;` and the
 * first 0x77C bytes of the old `data_` slab - exactly sizeof(Net) - so
 * nothing moves.
 *
 * WHAT THIS WITHDRAWS. tools/verify_subobjects.py reports five subobjects as
 * AlphaNet's own - VoiceTx at 0x58, JackalVoiceRx at 0xB0 and NetFifo at
 * 0xE8, 0x10C and 0x130 - because ??1AlphaNet tears them down. They are
 * Net's. ??1Net@@QAE@XZ (0x004E34D0, a standalone 525-byte function) destroys
 * the identical five at the identical offsets in the identical order, and
 * ??0Net builds them (`lea ecx, [esi+0x58]` / ??0VoiceTx at 0x0062D6C9,
 * ??0NetFifo three times from 0x0062D73E). ??1AlphaNet simply inlines ~Net
 * after the ?close@Net call; those offsets are inside the base subobject and
 * are not AlphaNet's to declare.
 *
 * WHERE THE BASE ENDS IS NOT MEASURED, and `data_` starting at 0x780 inherits
 * that. Nothing in the image pins sizeof(Net): tools/derive_class_layout.py
 * finds "no size evidence" for both Net and AlphaNet, and Net's own methods
 * reach only 0x768 (docs/recovery/access-lower-bounds.csv - which also
 * withdraws src/net_class.h's "as far as 0x770"). Against that, both
 * ??0AlphaNet (0x004E2590) and ?close@AlphaNet (0x004E25C9) write
 * [this+0x768] themselves, and AlphaNet's first unambiguously own structure
 * is the eight-entry, 0x19C-stride slot array based at 0x78C. So the real
 * boundary lies somewhere in [0x768, 0x78C]; 0x780 is only where the trailing
 * pad in src/net_class.h happens to round. The edge above is proven; this
 * offset is borrowed, and the static_assert is what will notice if
 * sizeof(Net) is ever corrected.
 */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
// THE LEDGER - AlphaNet's StringList-family subobject at 0x144C. A
// StringStruct-shaped base stage (vftable 0x669408, its one slot a leaf)
// over a VIRTUAL StringAllocationBase at +0x38 (vbtable 0x66EAF0 places it
// there; the owner capture lands at 0x1488), then a derived list stage
// (vftable 0x66EAE8) that adds no state. The two-stage construction the old
// transcription staged by hand is the compiler's own base-chain sequence.
class AlphaNetStringStruct : public virtual StringAllocationBase {
 public:
  virtual void unk_slot0();   // 0x00404250, unrecovered leaf

  uint32_t head_;              // +0x08 - the five the constructor zeroes
  uint32_t current_;           // +0x0C
  uint32_t entry_count_;       // +0x10
  uint32_t current_position_;  // +0x14
  uint32_t field_18_;          // +0x18
  uint8_t field_1C_[0x1C];     // +0x1C..+0x37, opaque to the virtual base

  AlphaNetStringStruct();
};

class AlphaNetLedger : public AlphaNetStringStruct {
  // the list stage - its own vtable (0x66EAE8, slot 0x5D4890) and nothing
  // else; the base chain carries all the state.
};

// One player's process slot: the pid dword (cleared at construction and by
// close) and the five-byte {FF,0,0,FF,2} identity pattern at +4.
struct AlphaNetProcessSlot {
  uint32_t pid_;
  uint8_t pattern_[5];
  uint8_t pad_[0x193];
};
static_assert(sizeof(AlphaNetProcessSlot) == 0x19C, "process slot stride");

class AlphaNet : public Net {
 public:
  AlphaNet();   // 0x004E2490, defined in alphanet.cpp
  int pid_2_idx(uint32_t process_id);
  int pid_2_who(uint32_t process_id);
  int who_2_pid(int identity);
  int who_2_idx(int identity);
  void close();

 private:
  uint8_t field_780_[0xC];        // 0x780..0x78B, opaque
  // THE SLOT REGION AND THE LEDGER SHARE STORAGE: eight 0x19C process
  // slots run 0x78C..0x146C, and the ledger (0x144C..0x148C) with its Heap
  // (0x148C..0x14A0) begins inside the eighth slot's unused tail - the
  // seventh slot's WRITTEN fields end at 0x12D9, far below. The original
  // models this as a union; both arms are placement-constructed (the image
  // calls Heap::Heap on 0x148C explicitly and inlines the ledger's ctor).
  union {
      AlphaNetProcessSlot process_slots_[8];  // from 0x78C
      struct {
          uint8_t slot_region_[0xCC0];        // the slot region's extent
          AlphaNetLedger ledger_;             // at 0x144C
          Heap heap_;                         // at 0x148C
      };
  };
};

static_assert(sizeof(AlphaNet) == 0x14A0,
              "AlphaNet recovered layout must include its trailing Heap");



// The legacy body tail-jumps into Net::close with this unchanged; that body
// is 570 bytes with three call targets and remains an original dependency.
