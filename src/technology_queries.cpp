#include "stdafx.h"
#include "technology.h"

// Keep this query out of technology.cpp: VC6 /O2 implies /Ob2 and otherwise
// inlines has_tech's prerequisite walk into callers in that TU. The pinned
// callers make an out-of-line call; this separate TU preserves that boundary.
BOOL __cdecl has_tech_call(int tech_id, int faction_id) {
    return has_tech(tech_id, faction_id);
}
