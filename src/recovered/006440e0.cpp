// 0x006440E0  sub_6440e0  ->  _sub_6440e0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_6440e0(int *a1, int a2) {
    return (*a1 -= a2) * 8 + reinterpret_cast<int>(g_009bf410);
}
