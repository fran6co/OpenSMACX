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

#include <cstddef>
#include <cstdint>

struct StringStructEntry {
  uint32_t abi_word;
  int id;
  int payload;
  StringStructEntry *next;
  StringStructEntry *previous;
  uint32_t secondary_abi_word;
  void *allocation_owner;
};

static_assert(sizeof(StringStructEntry) == 0x1C,
              "StringStructEntry layout must match the original executable");
static_assert(offsetof(StringStructEntry, id) == 0x4,
              "StringStructEntry ID offset must match the original executable");
static_assert(offsetof(StringStructEntry, payload) == 0x8,
              "StringStructEntry payload offset must match the original executable");

class DLLEXPORT StringStruct {
 public:
  int current_id();
  int current_entry();
  int next_entry();

 private:
  uint32_t primary_abi_word_;
  uint32_t virtual_base_abi_word_;
  StringStructEntry *head_;
  StringStructEntry *current_;
  int entry_count_;
  int current_position_;
  void *allocator_;
  uint32_t allocation_base_abi_word_;
  void *allocation_owner_;
};

static_assert(sizeof(StringStruct) == 0x24,
              "StringStruct layout must match the original executable");

int __fastcall string_struct_current_id_redirect(StringStruct *self, void *);
int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *);
int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *);
