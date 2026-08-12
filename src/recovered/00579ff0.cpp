// ORIGINAL: 0x00579FF0 BYTE_EXACT
// 0x00579FF0  sub_579ff0  ->  _sub_579ff0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" void __cdecl sub_579ff0(int a1, int a2) {
    int half = a1 >> 1;
    for (int i = 1; i < 8; i++) {
        int offset = i + (*g_0068faf0 * a2 + half) * 11;
        char *base = reinterpret_cast<char *>(*reinterpret_cast<int **>(g_0094a30c));
        int value = *reinterpret_cast<int *>(base + 0xc + offset * 4);
        if (value & 0x2000) {
            add_site(i, 0, 2, a1, a2);
        }
    }
}
