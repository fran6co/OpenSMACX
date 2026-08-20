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
class AlphaNet : Net {
 public:
  int pid_2_idx(uint32_t process_id);
  int pid_2_who(uint32_t process_id);
  int who_2_pid(int identity);
  int who_2_idx(int identity);
  void close();

 private:
  uint8_t data_[0xD20];  // 0x780, immediately after the Net base subobject
};

static_assert(sizeof(AlphaNet) == 0x14A0,
              "AlphaNet recovered layout must include its trailing Heap");

int __fastcall alpha_net_pid_to_idx_redirect(
    AlphaNet *self, void *, uint32_t process_id);
int __fastcall alpha_net_pid_to_who_redirect(
    AlphaNet *self, void *, uint32_t process_id);
int __fastcall alpha_net_who_to_pid_redirect(
    AlphaNet *self, void *, int identity);
int __fastcall alpha_net_who_to_idx_redirect(
    AlphaNet *self, void *, int identity);

void __fastcall alpha_net_close_redirect(AlphaNet *self, void *);

// The legacy body tail-jumps into Net::close with this unchanged; that body
// is 570 bytes with three call targets and remains an original dependency.
typedef void (OriginalObject::*func_net_close)();
extern func_net_close NetCloseOriginal;
