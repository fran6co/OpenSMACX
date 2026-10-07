#ifndef OPENSMACX_TABLE_STORAGE_H
#define OPENSMACX_TABLE_STORAGE_H

#include "vc6_compat.h"
#if !defined(_MSC_VER) || _MSC_VER > 1200
#include <stdint.h>
#endif

// Source-level view supported by the observed 0x616B40 accesses only.
// This is not asserted to be the original class or a complete allocation.
struct PointerTriplet {
    void *first_;
    void *second_;
    void *third_;
};

struct PointerTripletTable {
    PointerTriplet *table_;
    int32_t count_;
    uint32_t field_08_;

    // The mapped body and unresolved original-owner notes are retained with
    // the out-of-line definition in table_storage.cpp.
    void release_contents();
};

typedef char PointerTriplet_is_12_bytes_on_x86[
    sizeof(void *) != 4 || sizeof(PointerTriplet) == 0xC ? 1 : -1];
typedef char PointerTripletTable_is_12_bytes_on_x86[
    sizeof(void *) != 4 || sizeof(PointerTripletTable) == 0xC ? 1 : -1];

#endif
