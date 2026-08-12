// ORIGINAL: 0x0063E0F0 BYTE_EXACT
// 0x0063E0F0  sub_63e0f0  ->  _sub_63e0f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl sub_63dfe0(int, int, int, int);

int __cdecl sub_63e0f0(int a1, int a2, int a3) {
    return sub_63dfe0(a1, 0xf, a2, a3);
}
