#include "node.h"

// These are the source-owned counterparts of the pinned zero-initialized
// endpoint words. Each original body reads/writes these four-byte cells;
// full enclosing allocations and indirect aliases remain unknown. No executable
// address aliases or manual initialization are introduced.
// RECOVERY_OBJECT: v1 0x0090D944 ?GlobalListHead@@3PAVGlobalListNode@@A
GlobalListNode *GlobalListHead;
// RECOVERY_OBJECT: v1 0x0090D948 ?GlobalListTail@@3PAVGlobalListNode@@A
GlobalListNode *GlobalListTail;

// ORIGINAL: 0x004C4CB0 sub_4c4cb0 0x004C4CB0-0x004C4D01 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:ef28ddf0:src/recovered/units/004c4cb0.cpp", "git:ef28ddf0:src/unrecovered/004c4d10.cpp", "pinned:terranx_original:0x4c4cb0"], "reason": "Both complete original bodies use the same head/tail pointer cells and intrusive links at+18/+1C, with DWORD flags at+4. Recover actual pointer-member operations under a descriptive provisional owner name; preserve the original singleton repeat-enqueue defect and unflagged stale-link no-op. Original class name, allocation extent and full hierarchy remain unproved."}
// symbol    ?enqueue@GlobalListNode@@QAEHXZ
int GlobalListNode::enqueue() {
    if (next_ == 0 && previous_ == 0) {
        if (GlobalListTail == 0) {
            GlobalListHead = this;
            GlobalListTail = this;
            next_ = 0;
            flags_ |= 2u;
            previous_ = 0;
            return 0;
        }
        previous_ = GlobalListTail;
        GlobalListTail = this;
        previous_->next_ = this;
        flags_ |= 2u;
        next_ = 0;
    }
    return 0;
}

// ORIGINAL: 0x004C4D10 sub_4c4d10 0x004C4D10-0x004C4D6C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:ef28ddf0:src/recovered/units/004c4cb0.cpp", "git:ef28ddf0:src/unrecovered/004c4d10.cpp", "pinned:terranx_original:0x4c4d10"], "reason": "Both complete original bodies use the same head/tail pointer cells and intrusive links at+18/+1C, with DWORD flags at+4. Recover actual pointer-member operations under a descriptive provisional owner name; preserve the original singleton repeat-enqueue defect and unflagged stale-link no-op. Original class name, allocation extent and full hierarchy remain unproved."}
// symbol    ?unlink@GlobalListNode@@QAEHXZ
int GlobalListNode::unlink() {
    if ((flags_ & 2u) == 0) {
        return 0;
    }
    if (previous_ != 0) {
        previous_->next_ = next_;
    } else {
        GlobalListHead = next_;
    }
    if (next_ != 0) {
        next_->previous_ = previous_;
    } else {
        GlobalListTail = previous_;
    }
    flags_ &= ~2u;
    next_ = 0;
    previous_ = 0;
    return 0;
}
