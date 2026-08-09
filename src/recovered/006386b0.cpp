// ORIGINAL: 0x006386B0
// 0x006386B0  sub_6386b0  ->  _sub_6386b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl sub_638650(int, int, int);

int __cdecl sub_6386b0(int a1, int a2) {
    return sub_638650(a1, a2 + 8, 4) ? -1 : 0;
}
