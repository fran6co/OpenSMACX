#ifndef OPENSMACX_GLOBAL_LIST_NODE_H
#define OPENSMACX_GLOBAL_LIST_NODE_H

// Descriptive owner name only. Neither original identity proves a class name.
class GlobalListNode {
public:
    unsigned char opaque_prefix_[4];
    // Native loads/stores a DWORD here; only AL's list bit is changed.
    // RECOVERY_MEMBER: v1 GlobalListNode flags_ 0x4 0x004C4CB0
    unsigned int flags_;
    unsigned char opaque_8_17_[0x10];
    // RECOVERY_MEMBER: v3 GlobalListNode previous_ GlobalListNode* 0x18 0x004C4D10
    GlobalListNode *previous_;
    // RECOVERY_MEMBER: v3 GlobalListNode next_ GlobalListNode* 0x1C 0x004C4CB0
    GlobalListNode *next_;

    int enqueue();
    int unlink();
};

// Original pointer storage at 0x0090D944/0x0090D948. Its full allocation
// bounds and indirect alias/caller closure remain unproved. Start-only source
// bindings live beside their definitions; this declaration grants no extent.
extern GlobalListNode *GlobalListHead;
extern GlobalListNode *GlobalListTail;

#endif
