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
#include <stdlib.h>
#if !defined(_MSC_VER) || _MSC_VER > 1200
#include <cstddef>
#include <cstdint>
#endif


class StringStructPayload;
class ListPayloadBase;
struct StringStructEntry;
struct SpriteStruct;
class Popup;


// The hand-staged close chain is gone: the four staged table constants, the
// close_with_tables helper, and string_struct_derived_close_redirect left
// with the real StringList/StringStruct destructors, whose compiler-owned chain stages
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
 * real virtual bases under it. Native destruction owns both the derived/base
 * stages and the complete-object virtual-base tail; no raw-offset teardown
 * wrapper is needed.
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
 * Original StringBox construction inlines this chain: vbtable, then
 * StringAllocationBase's vtable and owner capture, StringStruct's table stage
 * and field zeroing, then StringList's own stage. Current construction is
 * genuine C++ with a standalone StringStruct constructor in stringstruct.cpp;
 * recovering its original entry also means callers may now emit a call rather
 * than this original inline sequence. Matching restoration must retain the
 * genuine virtual-base ownership, not reinstall tables manually.
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

// THE ALLOCATOR HAND-OFF SLOT, the image's 0x009B3374 .bss: a Heap*. The
// allocation wrapper publishes its selected Heap after a successful allocation;
// the StringAllocationBase
// constructed on that block captures it and clears the slot.
extern Heap *StringAllocationHeap;

class StringAllocationBase {
 public:
  // Inline so it inlines into the implicit StringStruct/StringList
  // construction chain: the image's inlined StringList block captures the
  // pending-allocation owner between the virtual base's vtable store and
  // StringStruct's stage - which is exactly where the base's own constructor
  // body lands. StringAllocationHeap is source-owned storage, not an original
  // executable address or a pointer into the old image.
  // THE ALLOCATOR HAND-OFF. The allocation wrapper publishes the Heap into
  // the global slot (image 0x009B3374) and the base constructor running
  // right after captures it - "who allocated me" - then clears the slot.
  // The destructor republishes the saved Heap for the deleting destructor's
  // deallocation decision: only an owner-null block is individually freed.
  // Original construction evidence at 0x00401100:
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
  // Scalar deleting entries 401520/4029F0/406670 and the Net-family
  // entries share this policy. Their compiler-generated adjustments supply
  // the complete allocation; the destructor publishes its captured owner.
  // Ordinary (non-deleting) destruction must leave that handoff intact.
  static void operator delete(void *memory) {
    if (StringAllocationHeap == 0) {
      free(memory);
    }
    StringAllocationHeap = 0;
  }
  Heap *allocation_owner_;
};

// Common complete-object type for the two independent list payload layouts.
// Its virtual allocation base owns dynamic deleting-destructor adjustment;
// the concrete payload fields begin after this compiler-owned header.
class ListPayloadBase : public virtual StringAllocationBase {};

// REAL VIRTUAL INHERITANCE, like StringStructPayload below it: the word
// add's construction installs at +0 is the VBTABLE 0x006693C8, whose two
// entries read {0, 0x14} - self displacement zero, the StringAllocationBase
// virtual base at +0x14 - and the base's vftable (0x006693AC, then the
// derived 0x006693C4 through the vbtable walk) plus the owner capture live
// inside that subobject. The declaration owns the vbptr and the vbase pair;
// the raw tearing-down walk (spritebox.cpp) indexes the
// words numerically and are untouched by the spelling.
struct StringStructEntry : public virtual StringAllocationBase {
  // RECOVERY_MEMBER: v1 StringStructEntry id 0x04 0x00402550
  int id;                        // 0x04
  // StringStructEntry is shared by independent string and sprite lists.
  // The allocation owner supplies the concrete payload interpretation.
  // RECOVERY_MEMBER: v3 StringStructEntry payload ListPayloadBase* 0x08 0x00402550
  ListPayloadBase *payload;  // 0x08
  StringStructPayload *string_payload();
  const StringStructPayload *string_payload() const;
  SpriteStruct *sprite_payload();
  const SpriteStruct *sprite_payload() const;
  // RECOVERY_MEMBER: v3 StringStructEntry next StringStructEntry* 0x0C 0x00402550
  StringStructEntry *next;       // 0x0C
  // RECOVERY_MEMBER: v3 StringStructEntry previous StringStructEntry* 0x10 0x00402550
  StringStructEntry *previous;   // 0x10

  // Original deleting destructor 0x00402C20 releases the complete allocation,
  // not the virtual-base address at +0x14. The compiler owns that adjustment;
  // the base destructor publishes ownership before this deallocator runs.
  static void operator delete(void *memory) {
    if (StringAllocationHeap == 0) {
      free(memory);
    }
    StringAllocationHeap = 0;
  }
};


static_assert(sizeof(StringStructEntry) == 0x1C,
              "StringStructEntry layout must match the original executable");
static_assert(offsetof(StringStructEntry, id) == 0x4,
              "StringStructEntry ID offset must match the original executable");
static_assert(offsetof(StringStructEntry, payload) == 0x8,
              "StringStructEntry payload offset must match the original executable");

// THE ENTRY PAYLOAD NODE StringStruct::add allocates: what
// current_entry()/next_entry() hand back. REAL VIRTUAL INHERITANCE, the
// same shape StringStruct itself has: add's construction stores the vbtable
// 0x006693BC at +0 - whose two entries read {0, 0x10}, placing the
// StringAllocationBase virtual base at +0x10 - installs the base's vftable
// there (0x006693AC, then the derived 0x006693B8 through the vbtable walk)
// and captures the owner at +0x14, all of which the declaration below makes
// the compiler's rather than words spelled by hand. THE STRING IS AT +4:
// the image's consumers read the second word as the text (Caviar's
// unit-list walk indexes payload[1] for the display name), which is the
// string-related type the accessor family is named for.
class StringStructPayload : public ListPayloadBase {
 public:
  char *text;      // 0x04 - the string this entry carries
  uint32_t field_8_;  // 0x08 - role-dependent metadata, not universally a line count
  uint32_t field_C_;  // 0x0C

  // 0x00402C70 has the same ownership rule for the payload's virtual base
  // at +0x10. Heap-backed blocks belong to the Heap, not individual nodes.
  static void operator delete(void *memory) {
    if (StringAllocationHeap == 0) {
      free(memory);
    }
    StringAllocationHeap = 0;
  }
};

static_assert(sizeof(StringStructPayload) == 0x18,
              "payload node must match the 0x18 allocation add makes");

inline StringStructPayload *StringStructEntry::string_payload() {
  return static_cast<StringStructPayload *>(payload);
}
inline const StringStructPayload *StringStructEntry::string_payload() const {
  return static_cast<const StringStructPayload *>(payload);
}

class StringStruct : public virtual StringAllocationBase {
 public:
  // Insertion 0x004012F2 passes the newly allocated 0x18-byte payload to
  // primary slot 0. Removal 0x00402992 calls slot 1 before destroying it.
  // The primary table ends where the separate allocation-base table begins
  // at 0x006693AC; that table's destructor is not a third primary slot.
  // RECOVERY_GENERATED: v1 vtable 0x006693A4 0x006693AC ??_7StringStruct@@6B0@@ 0x006086F0
  virtual int on_insert(void *payload);
  virtual void on_remove(void *payload);
  // Zeroes the five list fields, which the implicit construction chain
  // therefore emits between StringStruct's vtable stage and StringList's - the
  // image's order. The standalone original401000 entry is source-owned in
  // stringstruct.cpp; the virtual-base initialization flag is compiler-owned,
  // not an explicit source integer parameter.
  StringStruct();
  // The compiler stages the base tables before this cleanup. StringList's
  // destructor owns the earlier derived stage, where its text-removal hook
  // must still be active. These are real C++ destructor transitions, not
  // hand-written table installs. Out of line in stringstruct.cpp.
  ~StringStruct();
  MEASURED int seek_pos(int a1);
  int current_id();
  // Both return the entry's payload node - StringStructPayload, whose +4 is
  // the string the entry carries. The catalogue spelled both returns H (int)
  // and the image's own symbols spell them PAX (void *); this tree's emitted
  // names are carried by `// symbol` facts under the markers, which is what
  // resolves the claims.
  StringStructPayload *current_entry();
  MEASURED StringStructPayload *next_entry();
  int seek_id(int id);
  void remove_all();
  // 0x00401100 appends an entry and payload, then invokes the insertion hook.
  // Allocation failures retain the original partially updated list state.
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
  // The insertion hook passes this receiver to Heap::get at 0x00608714.
  // RECOVERY_MEMBER: v3 StringStruct allocator_ Heap* 0x18 0x006086F0
  Heap *allocator_;              // 0x18
  // 0x1C..0x24 is the virtual base: its vtable slot (compiler-managed) and
  // the inherited allocation_owner_.
};

static_assert(sizeof(StringStruct) == 0x24,
              "StringStruct layout must match the original executable");

class StringList : public StringStruct {
 public:
  // 0x004066C0 removes entries while StringList's text hook is still active,
  // before the compiler enters StringStruct's base cleanup stage.
  ~StringList();
  // Original primary slots name 0x006086F0 and 0x00608770. BasePop's
  // constructor (0x00600860) installs this table in both embedded lists;
  // the next table at 0x006698CC belongs to BasePop's Buffer subobject.
  // Its 0x004070B0 deleting-destructor thunk adjusts that receiver by -0x444.
  // RECOVERY_GENERATED: v1 vtable 0x006698C4 0x006698CC ??_7StringList@@6BStringStruct@@@ 0x006086F0
  virtual int on_insert(void *payload);
  virtual void on_remove(void *payload);

  // Provisional name: 0x005E3630 returns the current StringList text.
  // Its original Buffer callers pass this list in ECX and consume a char*.
  char *current_text() {
    return head_ != 0 ? current_->string_payload()->text : 0;
  }

  // Sort the circular list by payload text, retaining each ID with its
  // payload. The int argument is unused by the original implementation.
  // 0x006087A0 is the source-owned StringList::sort body in stringlist.cpp.
  void sort(int key);
  // 0x00608BA0 parses the caret prefix into pending-text metadata and inserts
  // ID zero through the inherited StringStruct::add operation.
  int add_special(char *text);
  // Load a text section into this list. The callback remains part of the
  // original interface; the original body does not invoke it.
  int load(LPSTR file_name, LPSTR section_id, int clear_existing,
           void (__cdecl *callback)(LPSTR));
  // PUBLIC, not private: the image pokes these three fields directly from
  // outside the class - Dialog::item writes pending_text_/field_20_/field_24_
  // of its embedded list (dialog.cpp) - and access specifiers do not change
  // layout, exactly as StringStruct's own list fields above.
  //
  // 0x00 is the compiler's vftable, 0x04 its vbtable, 0x08..0x18 the
  // inherited StringStruct fields; 0x1C..0x24 is this class's own storage,
  // which the default construction does not initialize.
  // Borrowed text supplied by Dialog::item and BasePop's string/button paths;
  // on_insert copies it into payload-owned storage. Null leaves text untouched.
  // RECOVERY_MEMBER: v1 StringList pending_text_ 0x1C 0x006086F0
  char *pending_text_;  // 0x1C
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

// THE TWO LIST WALKS THE IMAGE BOTH CALLS AND INLINES. Real out-of-line
// functions at 0x004015B0 (seek_pos, BYTE_EXACT, 2 callers) and 0x00402500
// (next_entry, BYTE_EXACT, 9 callers), AND expanded whole at other sites -
// BasePop::exec inlines both (0x00602FC5 and 0x0060305C), Caviar::init_class
// inlines the pair into its own list walk. A .cpp definition can only ever be
// one of those, which is why they live here: `inline` is what lets VC6 expand
// them, `MEASURED` is what keeps the out-of-line COMDAT emitted so their own
// claims stay measurable. Markers stay in stringstruct.cpp - decomp's reader
// globs only *.cpp.
//
// `seek_pos` calls abs() through a pointer for the same reason vector_dist
// spells its own two: `abs` of a variable is a real call here and the image
// makes two, while `abs` of a compile-time constant folds away and the calls
// vanish.

typedef int (__cdecl *AbsFnT)(int);

MEASURED inline int StringStruct::seek_pos(int a1) {
    AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
    if (a1 > entry_count_ - 1) {
        return 0;
    }
    current_ = head_;
    if (a1 < 0) {
        if (abs_fn(a1) > entry_count_) {
            return 0;
        }
        int i = abs_fn(a1);
        if (i > 0) {
            do {
                i--;
                current_ = current_->previous;
            } while (i != 0);
        }
        current_position_ = a1 + entry_count_;
        return 1;
    } else {
        if (a1 > 0) {
            int i = a1;
            do {
                i--;
                current_ = current_->next;
            } while (i != 0);
        }
        current_position_ = a1;
        return 1;
    }
}

MEASURED inline StringStructPayload *StringStruct::next_entry() {
    if (!head_) {
        return 0;
    }

    StringStructEntry *next = current_->next;
    int position = current_position_;
    current_ = next;
    int count = entry_count_;
    ++position;
    current_position_ = position;
    if (position == count) {
        current_position_ = 0;
    }
    return current_->string_payload();
}

#include "name_table.h"

// Provisional path-string owner, distinct from NameTable and StringStruct.
// Native lifetime callers remain unproved; initialize/close are explicit
// operations, and close deliberately does not reclaim directory storage.
struct PathRecord {
    // 0x004C8150 initializes this layout and returns this. The available
    // caller evidence does not establish constructor-only use.
    PathRecord *initialize();
    void close();
    void set_directory(char *directory);
    void set_path(char *path);
    void set_name(char *name);

    // RECOVERY_MEMBER: v1 PathRecord unknown_00_ 0x0 0x004C8150
    int unknown_00_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_04_ 0x4 0x004C8150
    unsigned int unknown_04_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_08_ 0x8 0x004C8150
    int unknown_08_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_0C_ 0xC 0x004C8150
    int unknown_0C_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_10_ 0x10 0x004C8150
    int unknown_10_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_14_ 0x14 0x004C8150
    int unknown_14_;
    // RECOVERY_MEMBER: v1 PathRecord unknown_18_ 0x18 0x004C8150
    int unknown_18_;
    // RECOVERY_MEMBER: v1 PathRecord path_ 0x1C 0x004C8290
    char *path_;
    // RECOVERY_MEMBER: v1 PathRecord directory_ 0x20 0x004C81C0
    char *directory_;
    // RECOVERY_MEMBER: v1 PathRecord name_ 0x24 0x004C83A0
    char *name_;
};

static_assert(sizeof(PathRecord) == 0x28, "Original path record footprint");

// Native entry 0x005BDE20 has five stack arguments and uses two Popup
// receivers. No direct source caller is currently proven; this typed
// declaration records the proposed source API without claiming caller ABI
// provenance beyond the pinned argument count/convention.
extern "C" void __cdecl tech_wizard_unk1(Popup *, Popup *, int, int, int);
