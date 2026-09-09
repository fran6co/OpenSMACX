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
// AlphaNet's session ledger at +0x144C. Both primary tables contain two
// relocated code pointers, immediately followed by nonrelocated vbtables:
// base 0x669408 -> {0x404250,0x404270}, vbtable {-4,0x18}; derived
// 0x66EAE8 -> {0x5D4890,0x5D4990}, vbtable {-4,0x34}. Thus the base's
// own storage ends at +0x1C, while the derived fields move its virtual
// StringAllocationBase to +0x38 (whole AlphaNet owner capture at +0x1488).
// Insertion 0x00401D80 calls slot 0 with the newly allocated NetAllocationNode;
// remove_all 0x00402970 calls slot 1 before virtual payload/node destruction.
struct AlphaNetLedgerEntry;
class AlphaNetStringStruct : public virtual StringAllocationBase {
 public:
  // RECOVERY_GENERATED: v1 vtable 0x00669408 0x00669410 ??_7AlphaNetStringStruct@@6B0@@ 0x004E2490
  virtual int on_insert(NetAllocationNode *payload);
  virtual void on_remove(NetAllocationNode *payload);

  AlphaNetLedgerEntry *head_;     // +0x08 - the five the constructor zeroes
  AlphaNetLedgerEntry *current_;  // +0x0C
  int entry_count_;              // +0x10
  int current_position_;         // +0x14
  // RECOVERY_MEMBER: v3 AlphaNetStringStruct allocator_ Heap* 0x18 0x005D4890
  Heap *allocator_;

  AlphaNetStringStruct();
  ~AlphaNetStringStruct();
};
static_assert(sizeof(AlphaNetStringStruct) == 0x24, "session ledger base and virtual allocation owner");

// RECOVERY_BASE: v1 AlphaNetLedger AlphaNetStringStruct 0x0 0x004E2490
class AlphaNetLedger : public AlphaNetStringStruct {
 public:
  // RECOVERY_GENERATED: v1 vtable 0x0066EAE8 0x0066EAF0 ??_7AlphaNetLedger@@6BAlphaNetStringStruct@@@ 0x004E2490
  virtual int on_insert(NetAllocationNode *payload);
  virtual void on_remove(NetAllocationNode *payload);
  ~AlphaNetLedger();

  // Original enumeration 0x00630C30 passes DPSESSIONDESC2's name, instance
  // GUID, flags, maximum/current player counts and password at these offsets.
  // They are borrowed inputs for the insertion hook, not owned strings.
  // RECOVERY_MEMBER: v1 AlphaNetLedger session_name_ 0x1C 0x005D4890
  const char *session_name_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger instance_guid_ 0x20 0x005D4890
  const GUID *instance_guid_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger session_flags_ 0x24 0x005D4890
  uint32_t session_flags_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger max_players_ 0x28 0x005D4890
  uint32_t max_players_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger current_players_ 0x2C 0x005D4890
  uint32_t current_players_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger password_ 0x30 0x005D4890
  const char *password_;
  // RECOVERY_MEMBER: v1 AlphaNetLedger field_34_ 0x34 0x005D4890
  uint32_t field_34_;
};
static_assert(sizeof(AlphaNetLedger) == 0x40, "session ledger fields precede its virtual allocation owner");

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
