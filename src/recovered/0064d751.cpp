// 0x0064D751  sub_64d751  ->  _sub_64d751
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl ___strgtold12(void *, void **, void *, int, int, int, int);
int __cdecl sub_64d725(void *, void *);

int __cdecl sub_64d751(void *a1, void *a2) {
    char value[12];
    ___strgtold12(value, &a2, a2, 0, 0, 0, 0);
    return sub_64d725(value, a1);
}
