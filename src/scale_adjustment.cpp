#include "stdafx.h"
#include "general.h"

#include <cstring>

// Native592D70/592D90 prove four-byte accesses only. This behavior-derived
// scalar home does not assert the complete original allocation or lifetime.
// RECOVERY_OBJECT: v1 0x00945AEC ?StoredScaleAdjustment@@3HA
int StoredScaleAdjustment = 0;

// ORIGINAL: 0x00592D70 sub_592d70 0x00592D70-0x00592D88 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/00592d70.cpp", "task:owned-helper-coverage-batch", "Native592D70 writes945AEC, clamps nonpositive inputs to zero and returns the stored value. Names describe observed behavior only; containing allocation and lifecycle remain unknown."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?set_scale_adjustment@@YAHH@Z
int __cdecl set_scale_adjustment(int value) {
    StoredScaleAdjustment = value;
    if (value <= 0) {
        value = 0;
        StoredScaleAdjustment = value;
    }
    return value;
}

// ORIGINAL: 0x00592D90 sub_592d90 0x00592D90-0x00592DAA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/00592d90.cpp", "task:owned-helper-coverage-batch", "Native592D90 reads945AEC, wraps(add16)*input at32bits and divides the signed product by16 toward zero. memcpy preserves signed representation without undefined C++ signed-overflow arithmetic. Seed zero-argument catalogue conflicts with observed [ebp+8] input; retain finding."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?apply_scale_adjustment@@YAHH@Z
int __cdecl apply_scale_adjustment(int value) {
    // Original add/imul wrap at32bits before signed division truncates to zero.
    const uint32_t bits = (static_cast<uint32_t>(StoredScaleAdjustment) + 16u)
        * static_cast<uint32_t>(value);
    int product;
    memcpy(&product, &bits, sizeof(product));
    return product / 16;
}
