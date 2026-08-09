// ORIGINAL: 0x00640A70
// 0x00640A70  sub_640a70  ->  _sub_640a70
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl _free(void *);

void __cdecl sub_640a70(int a1, void *a2) {
    _free(a2);
}
