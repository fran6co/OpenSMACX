// ORIGINAL: 0x0062CD10 BYTE_EXACT
// name      sub_62cd10
// size      56 bytes
// spans     0x0062CD10-0x0062CD48
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0062CD10  sub_62cd10  ->  _sub_62cd10
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __stdcall sub_62cd10(int a1, int a2) {
    char *obj = reinterpret_cast<char *>(a1);
    uint32_t density = (*reinterpret_cast<uint32_t *>(obj + 0x14) + 1) *
                        *reinterpret_cast<uint32_t *>(obj + 0x10) *
                        *reinterpret_cast<uint32_t *>(obj + 8) * 2;
    if (density < *g_009bc484) {
        uint32_t value = *reinterpret_cast<uint32_t *>(obj + 0xc);
        if (value <= 0x400) {
            uint32_t *out = reinterpret_cast<uint32_t *>(a2);
            if (value > *out) {
                *out = value;
            }
        }
    }
    return 1;
}
