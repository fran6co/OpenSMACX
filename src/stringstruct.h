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

#include "vc6_compat.h"
#if !defined(_MSC_VER) || _MSC_VER > 1200
#include <cstddef>
#include <cstdint>
#endif

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

class StringStruct {
 public:
  int seek_pos(int a1);
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
static const size_t StringStructCloseAdjustment = 0x1C;
extern const uint32_t StringStructVtable;
extern const uint32_t StringStructVirtualBaseVtable;
void __fastcall string_struct_close_redirect(void *adjusted, void *);

// A derived list closes with its own tables before closing its StringStruct
// base; its adjustor sits 0x28 bytes into the object.
static const size_t StringStructDerivedCloseAdjustment = 0x28;
extern const uint32_t StringStructDerivedVtable;
extern const uint32_t StringStructDerivedVirtualBaseVtable;
void __fastcall string_struct_derived_close_redirect(void *adjusted, void *);
int __fastcall string_struct_current_id_redirect(StringStruct *self, void *);
int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *);
int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *);
int __fastcall string_struct_seek_id_redirect(StringStruct *self, void *, int id);

/*
 * StringList - the string list whose two-stage teardown at 0x004066C0 is
 * already source-owned. 0x00406820 is its complete (non-deleting) destructor:
 * it runs that teardown and then hands the virtual base back its own vtable
 * and republishes the allocation owner the constructor captured.
 *
 * MSVC virtual inheritance: the eight-byte allocation-tracking virtual base is
 * held as the two MEMBERS at 0x28 that this class's own vbtable names
 * (0x0066B0EC = { -4, 0x24 }, so 4 + 0x24 = 0x28). It is deliberately NOT
 * written as ": virtual <base>" - the Itanium ABI this toolchain follows
 * would place it after the derived object and silently produce the wrong
 * layout. src/checkbox.h and src/listbox.h hold their virtual bases the same
 * way.
 *
 * StringStruct is likewise held by layout rather than by inheritance: a
 * StringStruct member at offset 0 would claim 0x1C/0x20 for its own virtual
 * base, which in a StringList is StringList-owned storage instead. destroy()
 * therefore reaches the recovered teardown through
 * string_struct_derived_close_redirect(), the entry point that performs the
 * 0x28 adjustment itself, exactly as the original's `call 0x004066C0` does.
 *
 * Layout evidence, all read from the pinned executable:
 *   0x00  vftable      - 0x006698C4 (StringList) then 0x006693A4
 *                        (StringStruct), both installed by the delegated close
 *   0x04  vbtable ptr  - 0x0066B0EC = { -4, 0x24 }   (ctor site 0x0042C9F3)
 *   0x08..0x18          the StringStruct list fields the delegated close and
 *                       remove_all() operate on; unchanged by this function
 *   0x1C..0x24          StringList-owned storage; untouched here
 *   0x28  virtual base vftable          (ctor 0x0042C9FA, dtor 0x0040682E)
 *   0x2C  virtual base saved owner      (ctor 0x0042CA01, dtor 0x0040682B)
 *   0x30  size - consecutive StringLists are 0x30 apart at 0x0048CD01 and at
 *         the inlined destructors 0x00406540 / 0x00406585
 */
class StringList {
 public:
  // ~StringList is a void destructor in the original, but EAX at its ret
  // still holds the saved owner value; modelled as uint32_t to preserve the
  // residue, as GraphicWin::close, Scroll::destroy and ListBox::close do.
  uint32_t destroy();

 private:
  uint32_t primary_abi_word_;          // 0x00
  uint32_t virtual_base_abi_word_;     // 0x04  -> 0x0066B0EC
  StringStructEntry *head_;            // 0x08
  StringStructEntry *current_;         // 0x0C
  int entry_count_;                    // 0x10
  int current_position_;               // 0x14
  void *allocator_;                    // 0x18
  uint32_t field_1C_;                  // 0x1C
  uint32_t field_20_;                  // 0x20
  uint32_t field_24_;                  // 0x24
  uint32_t allocation_base_abi_word_;  // 0x28
  void *allocation_owner_;             // 0x2C
};

static_assert(sizeof(StringList) == 0x30,
              "StringList layout must match the original executable");

// Displacement from a most-derived StringList to its virtual base. The
// original bakes this into `lea esi, [ecx + 0x28]`; it is deliberately NOT
// read from the vbtable, because the original does not read it either. The
// vbtable IS honoured where the original honours it, inside the delegated
// StringStruct::close_with_tables.
static const size_t StringListVirtualBaseOffset = 0x28;

// The virtual base's own one-slot vftable (0x006693AC), reinstalled once the
// StringStruct stage has finished with the subobject.
extern const uint32_t StringVirtualBaseVtable;

// Pending-allocation bookkeeping. ??0StringStruct@@QAE@H@Z (0x00401000) saves
// this global into the object and clears it; the virtual base's destructors
// republish it. Rebindable so tests can substitute their own storage: the
// default address is only mapped inside the game process.
extern uint32_t *StringVirtualBaseOwner;   // default 0x009B3374

// ~StringList is entered on the UNADJUSTED object, so unlike
// ListBoxDestructorAdjustment there is nothing to undo in the adapter; the
// 0x28 belongs to the callee at 0x004066C0.
uint32_t __fastcall string_list_destructor_redirect(StringList *self, void *);
