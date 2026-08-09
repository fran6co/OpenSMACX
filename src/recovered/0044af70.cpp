// ORIGINAL: 0x0044AF70
// 0x0044AF70  sub_44af70  ->  _sub_44af70
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_44af70() {
    int p = reinterpret_cast<int>(g_009a3840);
    do {
        int i = 5;
        do {
            *reinterpret_cast<char *>(p) = 0;
            p += 0x8c;
            i--;
        } while (i != 0);
    } while (p < reinterpret_cast<int>(g_009a4b64));
}
