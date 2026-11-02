#include "stdafx.h"
#include "general.h"
#include "veh.h"

// The original four 0xC-byte reactor records directly precede the 0x34-byte
// vehicle table. Keep these independent, zero-initialized arrays together;
// their original translation-unit name and any shared class remain unknown.
// The rules loader supplies reactor fields before use; vehicles are populated
// by their existing construction and game-loading owners.
// Original 0x0042A910 selects the record base; best_reactor walks prerequisite fields; no extent claim.
// RECOVERY_OBJECT: v1 0x009527F8 ?Reactor@@3PAURulesReactor@@A
RulesReactor Reactor[MaxReactorNum];  // 0x009527F8
// RECOVERY_OBJECT: v1 0x00952828 ?Vehs@@3PAUVeh@@A
Veh Vehs[2049];  // 0x00952828
