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
  int pid_2_who(uint32_t process_id);
  int who_2_pid(int identity);
  int who_2_idx(int identity);
  void close();

 private:
  uint32_t alignment_;
  uint8_t data_[0x149C];
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

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// The legacy body tail-jumps into Net::close with this unchanged; that body
// is 570 bytes with three call targets and remains an original dependency.
typedef void(__thiscall func_net_close)(void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
extern func_net_close *NetCloseOriginal;
