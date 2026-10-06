// SPDX-License-Identifier: GPL-3.0-or-later
// Rehomed declarations and structural evidence from stringstruct.h.
#pragma once

// Provisional name/range ownership family; no historical class/TU name proven.
// Explicit initialize/clear operations preserve observed lifetime boundaries;
// do not infer an automatic destructor or reset live storage implicitly.
struct NameTableEntry {
    // RECOVERY_MEMBER: v1 NameTableEntry high_index_ 0x0 0x004C7380
    unsigned int high_index_; // +0: inclusive upper slot bound
    // RECOVERY_MEMBER: v1 NameTableEntry low_index_ 0x4 0x004C7380
    unsigned int low_index_;  // +4: inclusive lower slot bound
    // RECOVERY_MEMBER: v1 NameTableEntry status_ 0x8 0x004C73C0
    unsigned int status_;     // +8
    // RECOVERY_MEMBER: v1 NameTableEntry name_ 0xC 0x004C74F0
    char name_[0x104];        // +0xC; total record size 0x110

    NameTableEntry *initialize();
};

struct NameTable {
    // RECOVERY_MEMBER: v1 NameTable slots_ 0x0 0x004C7300
    NameTableEntry *slots_[0x80]; // +0..+0x1FF
    // RECOVERY_MEMBER: v1 NameTable name_ 0x200 0x004C74B0
    char *name_;                  // +0x200; separately allocated by set_name
    // RECOVERY_MEMBER: v1 NameTable flags_ 0x204 0x004C72A0
    unsigned int flags_;          // +0x204
    // RECOVERY_MEMBER: v1 NameTable unknown_208_ 0x208 0x004C72A0
    unsigned int unknown_208_;
    // RECOVERY_MEMBER: v1 NameTable unknown_20c_ 0x20C 0x004C72A0
    unsigned int unknown_20c_;
    // RECOVERY_MEMBER: v1 NameTable unknown_210_ 0x210 0x004C72A0
    unsigned int unknown_210_;
    // RECOVERY_MEMBER: v1 NameTable unknown_214_ 0x214 0x004C72A0
    unsigned int unknown_214_;    // initialized to 0xFA

    NameTable *initialize();
    int clear();
    unsigned int remove_range(const NameTableEntry *range);
    void install(const NameTableEntry *entry);
    void rewrite_directory(char *directory);
    void set_name(char *name);
    char *get_name();
};

static_assert(sizeof(NameTableEntry) == 0x110, "Original name entry footprint");
static_assert(sizeof(NameTable) == 0x218, "Original name table footprint");
