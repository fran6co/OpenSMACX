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
  // THE ENTRY'S OWN TWO VTABLE WORDS, not ints: the node's construction
  // installs them (the primary at +0 and the virtual-base one at +0x14),
  // and the tearing-down walks treat [child] as a displacement table and
  // [vtbl+4] as a this-adjustment (spritebox.cpp, leaf_recoveries.cpp).
  // Nothing in src/ reads or writes either word by name - typing them
  // honestly changes no byte.
  const void *entry_vptr;        // 0x00 - the node's primary vtable word
  int id;
  // The entry's payload: a POINTER, not an int - remove_all hands it to the
  // entry-visitor virtual and destroys it through the virtual-base teardown,
  // and Caviar's unit-list walks read record fields through it. The old int
  // spelling was the catalogue's H return leaking into the struct.
  void *payload;
  StringStructEntry *next;
  StringStructEntry *previous;
  const void *vbase_vptr;        // 0x14 - the node's virtual-base vtable word
  void *allocation_owner;
};

static_assert(sizeof(StringStructEntry) == 0x1C,
              "StringStructEntry layout must match the original executable");
static_assert(offsetof(StringStructEntry, id) == 0x4,
              "StringStructEntry ID offset must match the original executable");
static_assert(offsetof(StringStructEntry, payload) == 0x8,
              "StringStructEntry payload offset must match the original executable");

// The legacy close is entered through a virtual-base adjustor, so the redirect
// receives a pointer 0x1C bytes into the object rather than its base.
static const size_t StringStructCloseAdjustment = 0x1C;

// The hand-staged close chain is gone: the four staged table constants, the
// close_with_tables helper, and string_struct_derived_close_redirect left
// with the real ~StringStruct destructor, whose compiler-owned chain stages
// the same tables (derived pair 0x006698C4/0x006698C0, base pair
// 0x006693A4/0x006693A0, virtual base 0x006693AC) from the declarations.

/*
 * StringList - the string list whose two-stage teardown at 0x004066C0 is
 * already source-owned. 0x00406820 is its complete (non-deleting) destructor:
 * it runs that teardown and then hands the virtual base back its own vtable
 * and republishes the allocation owner the constructor captured.
 *
 * MSVC virtual inheritance, DECLARED FOR REAL. The eight-byte
 * allocation-tracking virtual base is StringAllocationBase, inherited
 * virtually through StringStruct, exactly as the image's vbtables say: the
 * derived vbtable at 0x04 (0x0066B0EC = { -4, 0x24 }) names it at 0x28. The
 * old by-layout spelling (two raw members at 0x28) could not reproduce
 * StringBox::StringBox(), whose image frame carries the two EH slots VC6
 * emits only when the constructed base really derives virtually; the
 * earlier "Itanium ABI would misplace it" worry does not apply - the whole
 * tree compiles with cl 12.00.8168, and checkbox.h/listbox.h already hold
 * real virtual bases under it. The raw-offset teardowns below are untouched
 * by the change: they read and write fixed displacements, not member names.
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
 *
 * CONSTRUCTION is entirely the compiler's, and it is what StringBox::StringBox
 * inlines: vbtable, then StringAllocationBase's vtable and owner capture (its
 * inline constructor body), then StringStruct's vtable stage and field
 * zeroing (its inline constructor body), then StringList's own vtable stage -
 * the image's exact store order, with no call anywhere.
 */

// The eight-byte allocation-tracking virtual base the vbtables name: one
// vtable slot and the saved allocation owner. REAL, not held by layout:
// StringBox::StringBox() inlines the whole StringList construction, and the
// image's frame carries the two EH slots (the object pointer and a zeroed
// vbase-state temp) that VC6 emits only when the base it is constructing
// really derives virtually - measured against cl 12.00.8168 in isolation,
// 2026-08-29. Checkbox, RadioButton and ListBox already declare real virtual
// bases under this same toolchain (see checkbox.h for the layout argument).
class Heap;  // defined in heap.h; the owner slot carries one

// THE ALLOCATOR HAND-OFF SLOT, the image's 0x009B3374 .bss: a Heap*. Heap::get
// publishes itself here on a successful allocation; the StringAllocationBase
// constructed on that block captures it and clears the slot.
extern Heap *StringAllocationHeap;

class StringAllocationBase {
 public:
  // Inline so it inlines into the implicit StringStruct/StringList
  // construction chain: the image's inlined StringList block captures the
  // pending-allocation owner between the virtual base's vtable store and
  // StringStruct's stage - which is exactly where the base's own constructor
  // body lands. The address is terranx.exe's data, unmapped in a standalone
  // build; the same raw spelling StringBox::add uses.
  // THE ALLOCATOR HAND-OFF. Heap::get publishes the allocating Heap into
  // the global slot (image 0x009B3374) and the base constructor running
  // right after captures it - "who allocated me" - then clears the slot.
  // The destructor republishes the saved Heap so the next allocation can
  // route through it, and frees with operator delete only when no Heap
  // owns the block (the saved owner is null). Measured at 0x00401100:
  // `call Heap::get; mov [0x9B3374], edi; ... mov ecx, [0x9B3374];
  // mov [obj+0x18], ecx; mov [0x9B3374], 0`.
  StringAllocationBase() {
    allocation_owner_ = StringAllocationHeap;
    StringAllocationHeap = 0;
  }
  // VIRTUAL, and the class is therefore polymorphic: the image gives this
  // base its own one-slot vftable (0x006693AC), and without the virtual the
  // vbtables that name this base do not exist - VC6 drops the vbptr and the
  // StringStruct layout loses its 0x04 slot.
  virtual ~StringAllocationBase();
  Heap *allocation_owner_;
};

class StringStruct : public virtual StringAllocationBase {
 public:
  // THE OWN VIRTUAL SLOT. The image's StringStruct table at 0x006693A4 is
  // three dwords (0x006693A4..0x006693B0), so StringStruct declares virtuals
  // of its own - an override of the base's destructor would be folded into
  // the virtual base's table and leave no vfptr at offset 0, which the image
  // has. Which body owns which slot is not recovered: close() is called
  // directly (`call 0x00401060`-shaped sites and the vbase adjustor) and
  // remove_all() likewise (`call 0x402970` in ??1Net), so neither can carry
  // the virtual in this tree without changing those call sites' bytes. This
  // placeholder holds slot 0 so the layout is right; it is semantic debt
  // counted by class_debt.py until the slots are named.
  virtual void unk_slot00();
  // Zeroes the five list fields, which the implicit construction chain
  // therefore emits between StringStruct's vtable stage and StringList's - the
  // image's order. Inline for the same reason the capture is.
  StringStruct() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
  }
  // THE REAL DESTRUCTOR. Declaring it hands the derived-stage table
  // installs to the compiler: ~StringList's prologue stages the derived
  // pair, this destructor's prologue stages the base pair (the stores
  // close() and close_with_tables used to spell by hand), then the body
  // runs the entry walk. The original hand-managed these tables - 1998
  // C++ emulating virtual inheritance - and per direction the hand writes
  // go even where the image performs them; the compiled chain stages the
  // same tables from the declarations instead. Out of line in
  // stringstruct.cpp.
  ~StringStruct();
  int seek_pos(int a1);
  int current_id();
  // Both return the entry's payload pointer; the catalogue's H spelling
  // decorates the same dword return the bytes make.
  void *current_entry();
  void *next_entry();
  int seek_id(int id);
  void remove_all();
  void close();
  // 0x00401100, still a pending_bodies forwarder: allocates and links a new
  // entry, its own id, and (for the derived StringList at Dialog::item's
  // this+0xBC) a payload id node too. Dialog::item calls it BY NAME.
  int add(int id);

  // Offset 0x00 is the compiler's vftable and 0x04 its vbtable - the
  // vbtable's second dword is the virtual base's displacement, which
  // close_with_tables reads back at runtime. StringStructEntry *head_ lands
  // at 0x08, where the image's list head lives.
  //
  // PUBLIC, not private: the image pokes these fields directly from outside
  // the class - Caviar::init_class (0x006185A0) walks head_/current_/
  // entry_count_/current_position_ of the BasePop member at +0x2180 inline,
  // reading the count as its loop bound and advancing the cursor by hand.
  // No layout change: the members keep their declaration order and offsets
  // (0x08, 0x0C, 0x10, 0x14, 0x18); only the access label moved.
  StringStructEntry *head_;      // 0x08
  StringStructEntry *current_;   // 0x0C
  int entry_count_;              // 0x10
  int current_position_;         // 0x14
  void *allocator_;              // 0x18
  // 0x1C..0x24 is the virtual base: its vtable slot (compiler-managed) and
  // the inherited allocation_owner_.
};

static_assert(sizeof(StringStruct) == 0x24,
              "StringStruct layout must match the original executable");

class StringList : public StringStruct {
 public:
  // ~StringList is a void destructor in the original, but EAX at its ret
  // still holds the saved owner value; modelled as uint32_t to preserve the
  // residue, as GraphicWin::close, Scroll::destroy and ListBox::close do.
  uint32_t destroy();

  // PUBLIC, not private: the image pokes these three fields directly from
  // outside the class - Dialog::item writes field_1C_/field_20_/field_24_
  // of its embedded list (dialog.cpp) - and access specifiers do not change
  // layout, exactly as StringStruct's own list fields above.
  //
  // 0x00 is the compiler's vftable, 0x04 its vbtable, 0x08..0x18 the
  // inherited StringStruct fields; 0x1C..0x24 is this class's own storage,
  // which neither the construction above nor destroy() below touches.
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  // 0x28..0x30 is the inherited StringAllocationBase.
};

static_assert(sizeof(StringList) == 0x30,
              "StringList layout must match the original executable");

// The 0x28 displacement from a most-derived StringList to its virtual base
// (the image bakes it into `lea esi, [ecx + 0x28]`) is the compiler's own
// vbtable arithmetic now - no named constant, the same as the original,
// which does not read it from the vbtable either.

// The virtual base's own one-slot vftable (0x006693AC), reinstalled once the
// StringStruct stage has finished with the subobject.

// Pending-allocation bookkeeping. ??0StringStruct@@QAE@H@Z (0x00401000) saves
// this global into the object and clears it; the virtual base's destructors
// republish it. Rebindable so tests can substitute their own storage: the
// default address is only mapped inside the game process.

