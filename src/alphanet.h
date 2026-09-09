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
 * 0x00538C1B. The queues and voice members destroyed there belong to Net.
 *
 * Player records start at +0x76C, not at their process IDs at +0x78C.
 * Original on_player_added (0x0052E1DC) copies a name to +0x76C+n*0x19C;
 * its ID store (0x0052E1BA) uses +0x78C+n*0x19C. Eight records therefore
 * end at +0x144C, exactly before the ledger. The old record/ledger overlap
 * was an artifact of omitting the first 32 bytes, not evidence for a union.
 * AlphaNet construction and close initialize the count at +0x768;
 * Net construction's final ID word is +0x764. We model the count as
 * AlphaNet-owned and end Net at +0x768, removing its unsupported tail pad.
 * These ownership interpretations remain subject to wider caller evidence.
 */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
// THE LEDGER - AlphaNet's StringList-family subobject at 0x144C. A
// StringStruct-shaped base stage (vftable 0x669408)
// over a VIRTUAL StringAllocationBase at +0x38 (vbtable 0x66EAF0 places it
// there; the owner capture lands at 0x1488), then a derived list stage
// (vftable 0x66EAE8) that adds no state. The two-stage construction the old
// transcription staged by hand is the compiler's own base-chain sequence.
// The virtual surface is incomplete: original remove_all (0x402970) uses
// a second slot, and the derived table's 0x5D4890 payload operation remains
// a seed. The single declaration below is not evidence of a one-slot table.
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
  ~AlphaNetStringStruct();
};

class AlphaNetLedger : public AlphaNetStringStruct {
  // the list stage - its own vtable (0x66EAE8, slot 0x5D4890) and nothing
  // else; the base chain carries all the state.
 public:
  ~AlphaNetLedger();
};

// One complete player record, including the name before the process ID.
// Name storage ends at the ID; the meanings/types of the trailing status
// region are not yet established beyond the observed byte writes.
struct AlphaNetProcessSlot {
  // RECOVERY_MEMBER: v1 AlphaNetProcessSlot name_ 0x0 0x0052E160
  char name_[0x20];
  // RECOVERY_MEMBER: v1 AlphaNetProcessSlot pid_ 0x20 0x0052E160
  uint32_t pid_;
  // The lookup functions sign-extend this identity byte.
  // RECOVERY_MEMBER: v1 AlphaNetProcessSlot identity_ 0x24 0x004E2610
  int8_t identity_;
  uint8_t field_25_;
  uint8_t field_26_;
  int8_t field_27_;
  // Starts at original +0x794. process_message writes its first byte in
  // the indexed player record; the remaining bytes stay untyped evidence.
  // RECOVERY_MEMBER: v1 AlphaNetProcessSlot status_bytes_ 0x28 0x00534400
  uint8_t status_bytes_[0x174];

  AlphaNetProcessSlot() {
    identity_ = -1;
    field_25_ = 0;
    field_26_ = 0;
    field_27_ = -1;
    status_bytes_[0] = 2;
  }
};
static_assert(sizeof(AlphaNetProcessSlot) == 0x19C, "process slot stride");

// Original 0x004E24B7 calls Net construction on the unadjusted receiver.
// RECOVERY_BASE: v1 AlphaNet Net 0x0 0x004E2490
class AlphaNet : public Net {
 public:
  // Primary overrides from 0x0066EACC; slots 1/2 remain Net's defaults.
  // The six-slot bound follows the original primary chain described in Net.
  // RECOVERY_GENERATED: v1 vtable 0x0066EACC 0x0066EAE4 ??_7AlphaNet@@6B@ 0x004E2490
  virtual void on_net_event_0(int, int);
  virtual void on_player_name_changed(char *name, unsigned long id);
  virtual void on_session_lost();
  virtual void on_host_changed();
  AlphaNet();   // 0x004E2490, defined in alphanet.cpp
  ~AlphaNet();  // 0x00538B60; owns close before member/base destruction
  int pid_2_idx(uint32_t process_id);
  int pid_2_who(uint32_t process_id);
  int who_2_pid(int identity);
  int who_2_idx(int identity);
  void close();
  // 0x004E31E0, a pending_bodies forwarder (archived in 004e31e0.cpp). It
  // brings the transport up; NetDaemon::init (netdaemon.cpp) is the one
  // caller, and it tests the answer to decide whether the daemon came up.
  int setup(int option);

 public:
  // RECOVERY_MEMBER: v1 AlphaNet player_count_ 0x768 0x004E2490
  int player_count_;
  // Original initialization traverses eight records; callback name/ID accesses
  // establish their common start and stride independently of this declaration.
  // RECOVERY_MEMBER: v4 AlphaNet process_slots_ AlphaNetProcessSlot[8] 0x76C 0xCE0 0x004E2490
  AlphaNetProcessSlot process_slots_[8];

  // Ordinary independent members: Heap tears down first, then the ledger.
  // RECOVERY_MEMBER: v2 AlphaNet ledger_ AlphaNetLedger 0x144C 0x004E2490
  AlphaNetLedger ledger_;
  // RECOVERY_MEMBER: v2 AlphaNet heap_ Heap 0x148C 0x004E2490
  Heap heap_;
};

static_assert(sizeof(AlphaNet) == 0x14A0,
              "AlphaNet recovered layout must include its trailing Heap");



// The legacy body tail-jumps into Net::close with this unchanged; that body
// is 570 bytes with three call targets and remains an original dependency.
