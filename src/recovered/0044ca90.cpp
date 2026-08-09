// ORIGINAL: 0x0044CA90
// 0x0044CA90  sub_44ca90  ->  _sub_44ca90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_44ca90(int a1) {
    return X_pop(reinterpret_cast<char *>(g_00946a54) + a1 * 0x59c,
                 reinterpret_cast<const char *>(g_0068469c), 0);
}
