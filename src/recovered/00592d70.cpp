// 0x00592D70  sub_592d70  ->  _sub_592d70
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_592d70(int a1) {
    *g_00945aec = a1;
    if (a1 <= 0) {
        a1 = 0;
        *g_00945aec = a1;
    }
    return a1;
}
