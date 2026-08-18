// ORIGINAL: 0x0052DCF0 ?UNK1@DeletionList@@QAEHXZ 0x0052DCF0-0x0052DD10 BYTE_EXACT
// size      32 bytes
// prototype int (__thiscall ?UNK1@DeletionList@@QAEHXZ)(DeletionList* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0052DCF0  ?UNK1@DeletionList@@QAEHXZ  ->  ?UNK1@DeletionList@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int DeletionList::UNK1() {
    unsigned char *self = reinterpret_cast<unsigned char *>(this);
    unsigned char mask = *reinterpret_cast<unsigned char *>(g_009a64e8);
    int i;
    for (i = 0; i < 0x18; ) {
        if ((*self & mask) == mask) {
            return i;
        }
        ++i;
        self += 0x3c;
    }
    return 0;
}
