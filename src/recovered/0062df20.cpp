// ORIGINAL: 0x0062DF20 BYTE_EXACT
// 0x0062DF20  ?start_voice@Net@@QAEHK@Z  ->  ?start_voice@Net@@QAEHK@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Net::start_voice(unsigned long a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*g_009be600 == 0) {
        return 7;
    }
    if ((*reinterpret_cast<unsigned int *>(self + 0xd8) & 0x60000000) == 0) {
        *reinterpret_cast<int *>(self + 0x54) = 0;
        *reinterpret_cast<unsigned long *>(self + 0xd4) = a1;
        reinterpret_cast<VCall *>(self + 0x58)->slot036();
        *reinterpret_cast<unsigned int *>(self + 0xd8) |= 0x20000000;
    }
    return 0;
}
