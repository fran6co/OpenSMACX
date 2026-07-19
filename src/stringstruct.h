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
  int seek_id(int id);
  void remove_all();
  void close();
  // Shared body of every virtual-base close: installs the pair of virtual
  // tables, releases the entries, then clears the position.
  void close_with_tables(uint32_t primary, uint32_t virtual_base);

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

void __fastcall string_struct_remove_all_redirect(StringStruct *self, void *);
// The legacy close is entered through a virtual-base adjustor, so the redirect
// receives a pointer 0x1C bytes into the object rather than its base.
constexpr size_t StringStructCloseAdjustment = 0x1C;
extern const uint32_t StringStructVtable;
extern const uint32_t StringStructVirtualBaseVtable;
void __fastcall string_struct_close_redirect(void *adjusted, void *);

// A derived list closes with its own tables before closing its StringStruct
// base; its adjustor sits 0x28 bytes into the object.
constexpr size_t StringStructDerivedCloseAdjustment = 0x28;
extern const uint32_t StringStructDerivedVtable;
extern const uint32_t StringStructDerivedVirtualBaseVtable;
void __fastcall string_struct_derived_close_redirect(void *adjusted, void *);
int __fastcall string_struct_current_id_redirect(StringStruct *self, void *);
int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *);
int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *);
#ifdef __GNUC__
int __fastcall string_struct_seek_id_redirect(StringStruct *self, void *, int id)
    __attribute__((naked));
#else
int __fastcall string_struct_seek_id_redirect(StringStruct *self, void *, int id);
#endif
