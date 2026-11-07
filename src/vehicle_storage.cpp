#include "stdafx.h"
#include "general.h"
#include "veh.h"

// The original four 0xC-byte reactor records directly precede the 0x34-byte
// vehicle table. Keep these independent, zero-initialized arrays together;
// their original translation-unit name and any shared class remain unknown.
// The rules loader supplies reactor fields before use; vehicles are populated
// by their existing construction and game-loading owners.
// Original 0x0042A910 selects the record base; best_reactor walks prerequisite fields.
// Bound: 4 x 0xC records; loop ends at `cmp esi, 0x952828` (0x0044BAD6), Vehs' start.
// RECOVERY_OBJECT: v2 0x009527F8 0x00952828 ?Reactor@@3PAURulesReactor@@A 0x0044BAD6
RulesReactor Reactor[MaxReactorNum];  // 0x009527F8
// 2049 0x34-byte slots (veh_fake uses id 2048) end at 0x0096C85C, which the
// image indexes as a separate table base (`[edx*4+0x96c85c]` at 0x00449978).
// RECOVERY_OBJECT: v2 0x00952828 0x0096C85C ?Vehs@@3PAUVeh@@A 0x005C1D50
Veh Vehs[2049];  // 0x00952828
