#include "stdafx.h"
#include "base.h"

// The cached path has no result contract: its EAX is the selected id, while
// recomputation leaves the last resource pass's register residue. Callers use
// the published base fields, not a returned integer.
// ORIGINAL: 0x004EC3B0 ?base_compute@@YAHH@Z 0x004EC3B0-0x004EC3E7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004EC3B0-0x004EC3E7"], "reason": "Recover the original base-selection/cache operation with source-owned state and typed fields. The archived seed remains an obligation until this product definition is freshly remeasured."}
// symbol    ?base_compute@@YAXH@Z
void __cdecl base_compute(int force) {
    if (force || BaseComputedID != BaseIDCurrentSelected) {
        BaseComputedID = BaseIDCurrentSelected;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
}
