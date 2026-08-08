// 0x005F9100  ?UNK4@PullDown@@QAEXXZ  ->  ?UNK4@PullDown@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void PullDown::UNK4() {
    // Array of 0x40 20-byte entries starting at this+0xa18; the loop
    // pointer is kept at entry+4 (this+0xa1c) to match the original's
    // register value exactly - basing it at entry+0 instead keeps the same
    // mnemonic sequence but shifts every displacement by 4 (MNEMONIC_ONLY,
    // not BYTE_EXACT).
    char *p = reinterpret_cast<char *>(this) + 0xa1c;
    int n = 0x40;
    do {
        void **a = reinterpret_cast<void **>(p - 4);
        void **b = reinterpret_cast<void **>(p);
        if (*a != 0) {
            free(*a);
            *a = 0;
        }
        if (*b != 0) {
            free(*b);
            *b = 0;
        }
        *reinterpret_cast<int *>(p + 4) = -1;
        *reinterpret_cast<int *>(p + 8) = 0;
        p += 0x14;
    } while (--n != 0);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xf18) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xf20) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xf2c) = 0;
    *reinterpret_cast<char *>(reinterpret_cast<char *>(this) + 0xf34) = 1;
}
