// SPDX-License-Identifier: GPL-3.0-or-later
// Rehomed from OpenSMACX's stringstruct ownership catalogue.
#include "stdafx.h"
#include "name_table.h"
#include <cstring>
#include <new>

// These original initializers call CRT functions rather than intrinsic copies.
#pragma function(memset, strlen, strcpy)

// ORIGINAL: 0x004C75A0 sub_4c75a0 0x004C75A0-0x004C75A7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x004C75A0-0x004C75A7 executes mov eax,[ecx+0x200]; ret", "source:src/name_table.h NameTable::name_ is char* at +0x200, anchored by 0x004C74B0", "source:src/name_table.cpp adjacent NameTable::set_name owns allocation/update of the same member"], "reason": "Move the exact +0x200 pointer read onto NameTable's already declared name_ member. get_name is an operation-derived source spelling; no original mangled name is asserted."}
// symbol    ?get_name@NameTable@@QAEPADXZ
// kind      game
char *NameTable::get_name() {
    return name_;
}
// ORIGINAL: 0x004C7270 sub_4c7270 0x004C7270-0x004C729E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c7270.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?initialize@NameTableEntry@@QAEPAU1@XZ
NameTableEntry *NameTableEntry::initialize() {
    memset(name_, 0, 0x104);
    low_index_ = 0;
    high_index_ = 0;
    status_ = 1;
    return this;
}
// ORIGINAL: 0x004C72A0 sub_4c72a0 0x004C72A0-0x004C72F5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/units/004c72a0.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?initialize@NameTable@@QAEPAU1@XZ
NameTable *NameTable::initialize() {
    memset(&flags_, 0, 4);
    flags_ = (flags_ & ~4u) | 2u;
    unknown_208_ = 0;
    unknown_20c_ = 0;
    name_ = 0;
    unknown_210_ = 0;
    unknown_214_ = 0xFA;
    memset(slots_, 0, sizeof(slots_));
    return this;
}
// ORIGINAL: 0x004C7300 sub_4c7300 0x004C7300-0x004C7373
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?clear@NameTable@@QAEHXZ
// TRIED: scaffold had 0 params/cdecl; disassembly reads ecx with no stack access -> changed to `__fastcall sub_4c7300(int param_1)`. Redeclared the callee fn_0064557f (operator delete@0x64557f) to take `void *` instead of the scaffold's 0-arg stub. for-loop over the outer bucket scan diverges at instr #24 ('mov' vs rebuilt 'sub').
int NameTable::clear() {
    unsigned int slot_index = 0;
    while (slot_index < 0x80) {
        NameTableEntry *entry = slots_[slot_index];
        if (entry != 0) {
            // Each installed range points at the same entry. Its +0 word is
            // the high bound; use that slot to reacquire its range metadata.
            slot_index = entry->high_index_;
            NameTableEntry *record = slots_[slot_index];
            const unsigned int low = record->low_index_;
            const unsigned int end = record->high_index_ + 1;
            delete slots_[low];
            if (low < end) {
                unsigned int count = end - low;
                NameTableEntry **slot = slots_ + low;
                while (count != 0) {
                    *slot = 0;
                    ++slot;
                    --count;
                }
            }
        }
        ++slot_index;
    }
    if (name_ != 0) {
        ::operator delete(name_);
        name_ = 0;
    }
    return 0;
}
// ORIGINAL: 0x004C7380 sub_4c7380 0x004C7380-0x004C73B3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c7380.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?remove_range@NameTable@@QAEIPBUNameTableEntry@@@Z
unsigned int NameTable::remove_range(const NameTableEntry *range) {
    unsigned int index = range->low_index_;
    unsigned int end = range->high_index_;
    void *victim = slots_[index];
    ++end;
    ::operator delete(victim);
    if (index < end) {
        for (unsigned int i = index; i < end; ++i) {
            slots_[i] = 0;
        }
    }
    return end;
}
// ORIGINAL: 0x004C73C0 sub_4c73c0 0x004C73C0-0x004C74A4
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?install@NameTable@@QAEXPBUNameTableEntry@@@Z
// TRIED: this is a __thiscall member whose `this` (ecx) is a raw `int *` array (indexed `param_1[uVar6]`, no fixed-offset fields), and param_2 is `Entry*` (3 uints + a 0x104-byte name buffer = 0x110 bytes, matching the `operator new(0x110)` allocation size exactly - `local_8` and `param_2` share this same layout). operator new/delete (mangled ??2@.../??3@...) reached via differently-named externs `op_new`/`op_delete` with the real signatures, since the scaffold's fn_0064557f/fn_0064558a are nullary and the call targets are masked from comparison anyway. strcpy is called UNCONDITIONALLY on `local_8->name` even on the `operator new` failure path (ghidra confirms this, not an artifact) - preserved as-is rather than "fixed".
void NameTable::install(const NameTableEntry *entry) {
    const unsigned int high = entry->high_index_;
    const unsigned int low = entry->low_index_;
    NameTableEntry *clone = new NameTableEntry;
    if (clone != 0) {
        clone->initialize();
    }

    // Deliberately unconditional, including the original allocation-failure
    // path where clone is null.
    strcpy(clone->name_, entry->name_);

    if (clone->high_index_ < low) {
        clone->high_index_ = low;
    }
    clone->low_index_ = low;
    if (high < low) {
        clone->low_index_ = high;
    }
    clone->high_index_ = high;
    clone->status_ = entry->status_;

    unsigned int slot_index = low;
    if (low <= high) {
        do {
            NameTableEntry *record = slots_[slot_index];
            if (record != 0) {
                const unsigned int freed_low = record->low_index_;
                slot_index = record->high_index_ + 1;
                delete slots_[freed_low];
                if (freed_low < slot_index) {
                    NameTableEntry **slot = slots_ + freed_low;
                    unsigned int count = slot_index - freed_low;
                    while (count != 0) {
                        *slot = 0;
                        ++slot;
                        --count;
                    }
                }
            }
            ++slot_index;
        } while (slot_index <= high);

        NameTableEntry **slot = slots_ + low;
        unsigned int count = high - low + 1;
        while (count != 0) {
            *slot = clone;
            ++slot;
            --count;
        }
    }
}
// ORIGINAL: 0x004C74B0 sub_4c74b0 0x004C74B0-0x004C74F0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c74b0.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?set_name@NameTable@@QAEXPAD@Z
void NameTable::set_name(char *name) {
    if (name_ != 0) {
        ::operator delete(name_);
    }
    name_ = static_cast<char *>(::operator new(strlen(name) + 1));
    strcpy(name_, name);
}
// ORIGINAL: 0x004C74F0 sub_4c74f0 0x004C74F0-0x004C75A0
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?rewrite_directory@NameTable@@QAEXPAD@Z
// TRIED: [ecx+N] reads with no matching stack slot -> a __thiscall receiver (Obj4c74f0), not the given __stdcall(int). 0.89 mnemonic sim; first divergence is prologue register/stack-slot allocation for the `this`/`a1` locals, not chased further.
void NameTable::rewrite_directory(char *directory) {
    char dir_buf[0x100];
    strcpy(dir_buf, directory);
    const unsigned int length = strlen(dir_buf);
    if (dir_buf[length - 1] != '\\') {
        dir_buf[length] = '\\';
        dir_buf[length + 1] = 0;
    }

    NameTableEntry **slot = slots_;
    for (int remaining = 0x80; remaining != 0; --remaining) {
        NameTableEntry *entry = *slot;
        if (entry != 0) {
            // The original forms (entry + 0xC) as a runtime pointer and
            // tests it, even though ordinary valid records make it non-null.
            char *name = entry->name_;
            if (name != 0) {
                char rewritten[0x100];
                strcpy(rewritten, dir_buf);
                char *slash = strrchr(name, '\\');
                char *basename = (slash != 0) ? slash + 1 : name;
                strcat(rewritten, basename);
                strcpy(name, rewritten);
            }
        }
        ++slot;
    }
}
