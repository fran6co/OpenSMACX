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

class DLLEXPORT AlphaNet {
 public:
  int pid_2_idx(uint32_t process_id);

 private:
  uint32_t alignment_;
  uint8_t data_[0x12D0];
};

static_assert(sizeof(AlphaNet) == 0x12D4,
              "AlphaNet recovered layout must cover all process-id slots");

int __fastcall alpha_net_pid_to_idx_redirect(
    AlphaNet *self, void *, uint32_t process_id);
