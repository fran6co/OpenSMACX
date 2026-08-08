// 0x0048CF00  sub_48cf00  ->  _sub_48cf00
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_48cf00(int *p1, int p2) {
    char *self = reinterpret_cast<char *>(p1);
    if (*reinterpret_cast<int *>(self + 0x2274) == 0) {
        *reinterpret_cast<int **>(self + 0x2144) = g_007777a0;
    } else {
        *reinterpret_cast<int *>(self + 0x2144) = 0;
    }
    int result = reinterpret_cast<SpriteBox *>(self + 0x2240)->sprite(
        reinterpret_cast<Sprite *>(g_007777a0), reinterpret_cast<int8 *>(0), p2);
    *reinterpret_cast<int **>(self + 0x3168) = g_0048ceb0;
    return result;
}
